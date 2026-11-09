/*
  tsGeoPyramidCmds.c
 

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Rick Brownrigg, Martin Reddy, Lee Iverson, Yvan Leclerc.
  ----- END LICENSE BLOCK -----

*/
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <tcl.h>
#include <TerraVision/ThreeDCore.h>
#include <TerraVision/Trace.h>

#include "tsGeoPyramidCmds.h"
#include "tvBaseCmds.h"

#if 0
/* stuff used to generate unique instance names... */
static char baseName[] = "tSgEoSeT_";
static int  nextNameIncr = 0;
#endif

/* forward decls... */
int tsGeoPyramidInstProc(ClientData, Tcl_Interp*, int, Tcl_Obj*const[]);
int tsInfoSubCommand(ClientData, Tcl_Interp*, int, Tcl_Obj*const[]);
int tsCoordinateConversions(ClientData, Tcl_Interp*, int, Tcl_Obj*const[]);
int geosetNotFound(Tcl_Interp* interp, char* geosetName);

typedef struct {
  Tcl_Interp *interp;
  char   *imageUrl;
  char   *imageOptions;
  char   *demUrl;
  char   *demOptions;
} TerrainLoadClosure;

static void
tvSplashScreen( tvBase *base, void *message )
{
  if ( message)
    tvSetString( base, TV_SPLASH_MESSAGE, (char *) message );
}

static void
tvLoadTerrainThread( void *closure )
{
  TerrainLoadClosure *info = (TerrainLoadClosure *) closure;
  TsGeoPyramidInst* inst;
  const char* geoName;

  if ( ! info ) return;

  inst = (TsGeoPyramidInst*)calloc(1, sizeof(TsGeoPyramidInst));
  if ( ! inst ) return;

  tvConsoleMsg( "Loading terrain - %s", info->imageUrl );
  
  inst->geoset = TsGeoCreate_OIDEM (info->imageUrl, NULL, info->demUrl, NULL);
  if (!inst->geoset) {
    tvConsoleMsg( "Failed loading terrain - %s", info->imageUrl );
    tvForallWindows( tvSplashScreen, "Failed to load dataset!" );
    return;
  }

#if 0          
  /* create a unique name... */
  nextNameIncr++;
  sprintf(buff, "%s%04d", baseName, nextNameIncr);
  inst->name = (char*)malloc(strlen(buff)+1);
  strcpy(inst->name, buff);
#endif
  
  geoName = TsGeoName(inst->geoset);
  inst->name = (char*)malloc(strlen(geoName)+1);
  strcpy(inst->name, geoName);
    
  tvConsoleMsg( "Loaded terrain successfully - %s from %s",
		geoName, info->imageUrl );

    /* make a command associated with this instance... */
  Tcl_CreateObjCommand(info->interp, inst->name, tsGeoPyramidInstProc, 
		       (ClientData)inst, NULL);

  /* also want to relate the new instance name with the data in an
   * associative array, so that we can get at the data merely by "name"
   * later.
   */
  Tcl_SetAssocData(info->interp, inst->name, NULL, (ClientData)inst);

  /* signal to the Tcl/Tk side that we have completed the load */
  tvMsgListAdd( "TERRAIN_ADD", "", geoName, NULL );

  /* free up the strings */

  if ( info->imageUrl ) free( info->imageUrl );
  if ( info->imageOptions ) free( info->imageOptions );
  if ( info->demUrl ) free( info->demUrl );
  if ( info->demOptions ) free( info->demOptions );
  free( info );
}

int
tvRegisterGeoWebTerrain( Tcl_Interp* interp, char *geoName )
{
  TsGeoPyramidInst* inst;
  TsGeoPyramid *geo = TsGeoFind( geoName );

  if ( ! geo ) return FALSE;

  inst = (TsGeoPyramidInst*)calloc(1, sizeof(TsGeoPyramidInst));
  if ( ! inst ) return FALSE;

  inst->geoset = geo;
  inst->name = strdup(geoName);
    
  /* make a command associated with this instance... */
  Tcl_CreateObjCommand(interp, inst->name, tsGeoPyramidInstProc, 
		       (ClientData)inst, NULL);

  /* also want to relate the new instance name with the data in an
   * associative array, so that we can get at the data merely by "name"
   * later.
   */
  Tcl_SetAssocData(interp, inst->name, NULL, (ClientData)inst);

  return TRUE;
}

/*
 * t s G e o P y r a m i d P r o c ( )
 *
 */
int
tsGeoPyramidProc(ClientData data, Tcl_Interp* interp, int objc, 
    Tcl_Obj *const objv[])
{
    char* cmd;
    if (objc < 2) return TCL_ERROR;

    cmd = Tcl_GetStringFromObj(objv[1], NULL);

    /*
     * c r e a t e 
     */
    if (!strcmp(cmd, "create")) {
      TerrainLoadClosure *info = malloc( sizeof(TerrainLoadClosure) );

      if (objc < 6) return TCL_ERROR;

      /* do the actual terrain loading in a separate thread */
      /* so that we don't hang up the user interface.       */

      if ( info ) {
      	PRThread *newthread;
        char const *demURL = Tcl_GetStringFromObj(objv[4], NULL);

#if defined( _WIN32_ ) || defined( WIN32 ) || defined( __WIN32__ )
        /* check to see if we have already loaded this DEM URL before  */
        /* unfortunately, the brain-dead Windows operating system will */
        /* not let us load data from the same open file in different   */
        /* threads, so in the meantime we need to stop the user from   */
        /* selecting this otherwise lots of TIFF errors get generated  */

        Purl *purl = tsmParseUrl( demURL );
        int  is_local = ( purl && purl->protocol == tsmLocalFile );
        if ( purl ) tsmFreePurl( purl );

        if ( is_local && TsGeoFindByURL( demURL ) != NULL ) {
          tvMsgListAdd( "ERROR", NULL, "Cannot open multiple versions of the same\nDEM from local disk under Windows. Sorry!\nTry copying the dataset under another name.", NULL );
          return TCL_OK;
        }
#endif

        /* unpack the arguments... */
        info->interp       = interp;
      	info->imageUrl     = strdup( Tcl_GetStringFromObj(objv[2], NULL) );
      	info->imageOptions = strdup( Tcl_GetStringFromObj(objv[3], NULL) );
	      info->demUrl       = strdup( demURL );
      	info->demOptions   = strdup( Tcl_GetStringFromObj(objv[5], NULL) );


        /* load the terrain dataset in a new thread */

	      newthread = PR_CreateThread( PR_USER_THREAD, tvLoadTerrainThread,
				                             info, PR_PRIORITY_HIGH,
				                             PR_GLOBAL_THREAD, PR_JOINABLE_THREAD,
				                             1<<16 );
      	pthr_set_name( newthread, "Load Terrain" );
      }

      return TCL_OK;
    }

    /*
     * l i s t 
     */
    if (!strcmp(cmd, "list")) {
        /* We extract this info from the global variable "geoList" maintained
         * by libTV2. This is either a violation of the library's interface,
         * or more likely, the interface needs to incorporate a cleaner access
         * path.
         */
        list_iter_t i;
        Tcl_Obj* retList = Tcl_NewListObj(0, NULL);

        for (i=list_first(geoList); !list_done(i); i=list_next(i)) {
            TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
            Tcl_Obj* elem = Tcl_NewStringObj((char*)TsGeoName(geo), -1);
            Tcl_ListObjAppendElement(interp, retList, elem);
        }

        Tcl_SetObjResult(interp, retList);
        return TCL_OK;
    }

    /*
     * d e s t r o y 
     */
    if (!strcmp(cmd, "destroy")) {
        char* geosetName;
        list_iter_t i;
        TsGeoPyramid* theGeoset = NULL;

        if (objc != 3) return TCL_ERROR;

        
        geosetName = Tcl_GetStringFromObj(objv[2], NULL);
        for (i = list_first(geoList); !list_done(i); i=list_next(i)) {
            TsGeoPyramid* geo = (TsGeoPyramid*) list_value(i);
            if (!geo) continue;
            if (!strcmp(geosetName, TsGeoName(geo))) {
                theGeoset = geo;
                break;
            }
        }
        if (!theGeoset) return geosetNotFound(interp, geosetName);

	tvConsoleMsg( "Unloaded terrain - %s", geosetName );

        /* TsCacheClearAll(); */

        /* remove the geoset from all opened viewers... */
        tvForallWindows ((void (*)(tvBase*,void*)) tvRemoveDataset, theGeoset);

        /* go back through the global list of geos and remove this entry */
        for ( i = list_first( geoList ); ! list_done( i ); ) {
            TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );

	    if (geo && geo == theGeoset) {
	      TsGeoDestroy(theGeoset);
	      i = list_delete( i );
	      break;
	    }

		i = list_next( i );
        }

        return TCL_OK;
    }

    /*
     * i n f o 
     */
    if (!strcmp(cmd, "info")) {
        return tsInfoSubCommand(data, interp, objc, objv);
    }

    /* 
     * c o o r d s 
     */
    if (!strcmp(cmd, "coords")) {
        return tsCoordinateConversions(data, interp, objc, objv);
    }

    return TCL_ERROR;
}

/*
 * Synopsis:
 *   void InfoMakeSize( double bytes, char *buffer )
 *
 * Description:
 *   A simple routine that will create a string given a number of bytes.
 *   It will try to convert the byte count into suitable units, e.g.
 *   KB, MB, of GB, depending upon its magnitude.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoMakeSize( double bytes, char *buffer )
{
  static const float KB = 1024.0;
  static const float MB = 1024.0 * 1024.0;
  static const float GB = 1024.0 * 1024.0 * 1024.0;

  if ( bytes < KB )
    sprintf( buffer, "%1.0f bytes", bytes );
  else if ( bytes < MB )
    sprintf( buffer, "%1.1f KB", bytes / KB );
  else if ( bytes < GB )
    sprintf( buffer, "%1.1f MB", bytes / MB );
  else
    sprintf( buffer, "%1.1f GB", bytes / GB );
}

/*
 * Synopsis:
 *   static void printDeg( float res1, char *buffer )
 *
 * Description:
 *   Takes a decimal resolution value in units of degrees and returns
 *   a string describing this value in terms of degrees, minutes, and
 *   seconds.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 April 2001
 *
 */

static void
printDeg( float res1, char *buffer )
{
  double res = floor( res1 * 60.0*60.0 + 0.5 ) /60.0/60.0;
  double deg = floor( res );
  double min = ( res - deg ) * 60.0;
  double sec = ( min - floor( min ) ) * 60.0;
  sprintf( buffer, "%1.0lf deg %1.0lf min %1.0lf sec",
           deg, floor(min), fabs(sec) );
}

/*
 * t s I n f o S u b C o m m a n d ( )
 *
 * Returns a string describing various details about the connection for a 
 * given tsGeoPyramid, or aggregate statistics if no argument is given.
 *
 * Much of this code was adapted directly from the Info.c code found in 
 * XmTV.
 *
 */
int
tsInfoSubCommand(ClientData data, Tcl_Interp* interp, int objc, 
    Tcl_Obj *const objv[])
{
    Tcl_DString theMsg;
    Tcl_DString* theMsgP = &theMsg;
    list_iter_t i;

    Tcl_DStringInit(theMsgP);

    /* aggregate or individual report? */
    if (objc < 3) {
        TsmConnection connection[2];
        Pyramid       *tspec;
        TileTspec     *tile;
        char          buff[1024], size[128];
        int           *pixels;
        double        imgsize, pixs;
        double        totalSize = 0.0;
        double        totalPixels = 0.0;
        double        totalDemPostings = 0.0;
        int           numConnections;
        int           k;

        /* iterate over the geosets... */
        for (i = list_first( geoList ); ! list_done( i ); i=list_next(i)) {
            TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
            if (!geo) continue;

            /* obtain some convenience pointers... */
            numConnections = TsGeoConnections(geo, connection);

            for (k = 0; k < numConnections; k++ ) {
                tspec   = (Pyramid *)tsmGet(connection[k], TSM_DATASET_TSPEC);
                tile    = &tspec->tileTspec;
                pixels  = tspec->tspecLevel.val[tspec->minLevel].numValidPixel;
		pixs    = ((double) pixels[0]) * ((double) pixels[1]); 
                imgsize = pixs * ((double) tile->numComponentsPixel) *
		  ((double) tile->numBytesPerComponent);

                /* add to the total aggregate size of all loaded datasets */
                totalSize += imgsize * 4.0 / 3.0;

                /* add to the total aggregate number of pixels/DEM postings */

                switch ((TsmDataSetType)tsmGet(connection[k], TSM_DATASET_TYPE)) {
		case TSM_DEM_DATASET:
                    totalPixels += pixs;
                    break;
                case TSM_IMAGE_DATASET:
                    totalDemPostings += pixs;
                    break;
                }
            }
        }

        Tcl_DStringAppend(theMsgP, "Aggregate Report:\n\n", -1);

        sprintf(buff, "  Datasets Loaded = %d\n",
            ( geoList ) ? list_count( geoList ) : 0);
        Tcl_DStringAppend(theMsgP, buff, -1);

	InfoMakeSize( totalSize, size );
        sprintf(buff, "  Aggregate Dataset Size = %s\n", size);
        Tcl_DStringAppend(theMsgP, buff, -1);

        sprintf(buff, "  Aggregate Pixels = %1.1f million\n",
            totalPixels / 1e6 );
        Tcl_DStringAppend(theMsgP, buff, -1);

        sprintf(buff, "  Aggregate DEM postings = %1.1f million\n",
            totalDemPostings / 1e6 );
        Tcl_DStringAppend(theMsgP, buff, -1);

        {
            Tcl_Obj* retVal = Tcl_NewStringObj(Tcl_DStringValue(theMsgP), -1);
            Tcl_SetObjResult(interp, retVal);
        }
        return TCL_OK;
    }


    /* 
     * individual report... this be ugly, so hang on... 
     */

    else {
        char*          geosetName;
        TsGeoPyramid*  theGeoset = NULL;
        TsmDataSet     dset;
        TsmConnection  connection[2];
        Pyramid*       tspec;
        TileTspec      *tile;
        Purl*          purl;
        char           *units;
        int            *pixels;
        float          *ixiytouv;
        float          x, y;
        double         imgSize;
        char           buff[1024]; /* damn-well better be big enough */
	char           size[64];
        int            numConnections;
        int            j;

        geosetName = Tcl_GetStringFromObj(objv[2], NULL);

        /* first find the geoset... */
        for ( i = list_first( geoList ); ! list_done( i ); ) {
            TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
            if (geo && !strcmp(geosetName, TsGeoName(geo))){
                theGeoset = geo;
                break;
            }
            i = list_next( i );
        }
        if (!theGeoset) return geosetNotFound(interp, geosetName);

        numConnections = TsGeoConnections(theGeoset, connection);

        /* 
         * OK, lets bang out the report; iterate over both "connections" --
         * this is kinda evil, but we "know" there are only two connections.
         */

        sprintf(buff, "Geoset:  %s\n", geosetName);
        Tcl_DStringAppend(theMsgP, buff, -1);

        for (j=0; j<numConnections; j++) {
            /* some convenience pointers... */
            tspec = (Pyramid*) tsmGet(connection[j], TSM_DATASET_TSPEC);
            tile    = &tspec->tileTspec;
            units   = (tspec->coordTspec.componentUnits == tspDegrees)
                ? "deg" 
                : "m";
            pixels  = tspec->tspecLevel.val[tspec->minLevel].numValidPixel;
            ixiytouv = 
                tspec->tspecLevel.val[tspec->minLevel].imageTransformationTspec.ixiytoUV;
            imgSize =
	      ((double) pixels[0]) *
	      ((double) pixels[1]) *
	      ((double) tile->numComponentsPixel) *
	      ((double) tile->numBytesPerComponent);

            sprintf(buff, "\n  Dataset = %s\n", (char *)
                tsmGet(connection[j], TSM_DATASET_NAME));
            Tcl_DStringAppend(theMsgP, buff, -1);
        
            sprintf(buff, "  Type = %s\n", (char *)
                tsmGet(connection[j], TSM_DATASET_TYPE_STRING));
            Tcl_DStringAppend(theMsgP, buff, -1);

            /* use the 4/3 factor to approximate the size of entire pyramid */
	    InfoMakeSize( imgSize * 4.0 / 3.0, size );
            sprintf(buff, "  Total Size = %s\n", size );
            Tcl_DStringAppend(theMsgP, buff, -1);

            if ( tspec->coordTspec.componentUnits == tspDegrees ) {
              char b1[64], b2[64];
              printDeg( tspec->minResolution, b1 );
              printDeg( tspec->maxResolution, b2 );
              sprintf(buff, "  Resolution:\n    Ground Res = %s to %s\n",
                      b1, b2 );
            } else
              sprintf(buff, "  Resolution:\n    Ground Res = %1.10g m to %1.10g m\n",
                  tspec->minResolution, tspec->maxResolution );

            Tcl_DStringAppend(theMsgP, buff, -1);

            x = tspec->minResolution * pixels[0];
            y = tspec->minResolution * pixels[1];
            if ( ( x > 1000.0 || y > 1000.0 ) && strcmp( units, "m" ) == 0 ) {
                x /= 1000.0;
                y /= 1000.0;
                units = "km";
            }
            sprintf(buff, "    Ground Cover = %1.10g x %1.10g %s\n", x, y, units);
            Tcl_DStringAppend(theMsgP, buff, -1);

	    InfoMakeSize( imgSize, size );
            sprintf(buff, "    Pixel Res = %d x %d (%s)\n", pixels[0],
		    pixels[1], size);
            Tcl_DStringAppend(theMsgP, buff, -1);

            sprintf(buff, "    Tile size = %d x %d\n",
		    tspec->tileTspec.numPixelsX, tspec->tileTspec.numPixelsY );
            Tcl_DStringAppend(theMsgP, buff, -1);
        
            sprintf(buff, "    Levels = %d (%d..%d)\n",
                tspec->maxLevel - tspec->minLevel + 1,
                tspec->minLevel, tspec->maxLevel);
            Tcl_DStringAppend(theMsgP, buff, -1);
        
            /* Where is the data coming from? */
            Tcl_DStringAppend(theMsgP, "  Location:\n", -1);

            purl = tsmParseUrl((char *)tsmGet(connection[j], TSM_DATASET_URL));
            switch (tsmGet(connection[j], TSM_CONNECT_PROTOCOL)) {
            case TSM_DPSS:
                sprintf(buff, "    DPSS = %s\n", purl->hostName);
                Tcl_DStringAppend(theMsgP, buff, -1);

                sprintf(buff, "    Servers = %s\n",
                    (char *) tsmGet(connection[j], TSM_DPSS_SERVERS));
                Tcl_DStringAppend(theMsgP, buff, -1);

                sprintf(buff, "    Set ID = %d\n", 
                    tsmGet(connection[j], TSM_DPSS_SETID));
                Tcl_DStringAppend(theMsgP, buff, -1);

                sprintf(buff, "    Tspec = %s\n", 
                    (char*)tsmGet(connection[j], TSM_TSPEC_URL));
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;

            case TSM_LOCAL:
                sprintf(buff, "    Localhost = %s\n", purl->uri);
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;

            case TSM_HTTP:
                sprintf(buff, "    HTTP = %s\n", purl->url);
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;
            }

            /* What format is the tiled data in? tiff/raw jpeg/raw */
            sprintf(buff, "  Format:\n    Image Format = %s\n",
                (char *) tsmGet(connection[j], TSM_IMAGE_FORMAT_STRING));
            Tcl_DStringAppend(theMsgP, buff, -1);

            sprintf(buff, "    Tile Format = %s\n",
                (char *) tsmGet(connection[j], TSM_TILE_FORMAT_STRING));
            Tcl_DStringAppend(theMsgP, buff, -1);

            /* Where on the planet is this dataset? */

            Tcl_DStringAppend(theMsgP, "  Georeference:\n", -1);
            tsmCalcFootprintFromXform(tspec, &dset);

            switch ( tspec->projectionTspec.projectionType ) {
            case tspUtmProjection:
                sprintf(buff, "    Coordinate System = UTM\n");
                Tcl_DStringAppend(theMsgP, buff, -1);
                sprintf(buff, "    Lower-Left = %1.10g easting %1.10g northing %d zone\n", 
                    dset.footprint.utm.c1.e, dset.footprint.utm.c1.n,
                    dset.footprint.utm.c1.zone);
                Tcl_DStringAppend(theMsgP, buff, -1);
                sprintf(buff, "    Upper-Right = %1.10g easting %1.10g northing %d zone\n", 
                    dset.footprint.utm.c3.e, dset.footprint.utm.c3.n,
                    dset.footprint.utm.c3.zone );
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;
            
            case tspAffineProjection:
                sprintf(buff, "    Coordinate System = LVCS\n");
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;
            
            case tspLatLongProjection:
                sprintf(buff, "    Coordinate System = Lat/Long\n");
                Tcl_DStringAppend(theMsgP, buff, -1);
                sprintf(buff, "    Lower-Left = %1.10g latitude %1.10g longitude\n", 
                    ixiytouv[5], ixiytouv[2]);
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;
            
            default:
                sprintf(buff, "    Coordinate System = Unknown<%d>\n",
                    tspec->projectionTspec.projectionType);
                Tcl_DStringAppend(theMsgP, buff, -1);
                break;
            }

            sprintf(buff, "    Planet = %s\n", tsmPlanetToString(tspec->planet));
            Tcl_DStringAppend(theMsgP, buff, -1);

	    /* output the description string if given */
	    
	    if ( tspec->description && strlen(tspec->description) > 0 ) {
	      Tcl_DStringAppend(theMsgP, "  Description:\n    ", -1);
	      Tcl_DStringAppend(theMsgP, tspec->description, -1);
	      Tcl_DStringAppend(theMsgP, "\n", -1);
	    }

        }
    }

    /* Contrary to what the docs suggest, the following, presumably more 
     * efficient call doesn't work, and causes a SEGV in the interpreter 
     * downstream:
     *     Tcl_DStringResult(interp, theMsgP);
     */
    {
        Tcl_Obj* retVal = Tcl_NewStringObj(Tcl_DStringValue(theMsgP), -1);
        Tcl_SetObjResult(interp, retVal);
    }
    return TCL_OK;
}


/*
 * t s C o o r d i n a t e C o n v e r s i o n s ( )
 *
 * Performs coordinate conversions:  GCC <==> LVCS,  using a TsGeoPyramid
 * as reference.
 *
 */
int 
tsCoordinateConversions(ClientData data, Tcl_Interp* interp, int objc,
    Tcl_Obj *const objv[])
{
    char*         convDir;
    char*         geosetName;
    list_iter_t   i;
    TsGeoPyramid* theGeoset = NULL;
    Tcl_Obj*      inCoords;
    int           listLen;
    int           j;
    Tcl_Obj**      elems;
    double        x, y, z, newCoords[3];
    Tcl_Obj*      retList;

    if (objc != 5) return TCL_ERROR;

    /* third argument tells us which way the conversion goes... */
    convDir = Tcl_GetStringFromObj(objv[2], NULL);

    /* 4th argument better be a TsGeoPyramid... */
    geosetName = Tcl_GetStringFromObj(objv[2], NULL);
    for (i = list_first(geoList); !list_done(i); i=list_next(i)) {
        TsGeoPyramid* geo = (TsGeoPyramid*) list_value(i);
        if (!geo) continue;
        if (!strcmp(geosetName, TsGeoName(geo))) {
            theGeoset = geo;
            break;
        }
    }
    if (!theGeoset) return geosetNotFound(interp, geosetName);

    /* last argument better be a list of 3 doubles... */
    inCoords = objv[4];
    if (Tcl_ListObjGetElements(interp, inCoords, &listLen, &elems)==TCL_ERROR) 
    {
        return TCL_ERROR;  /* Tcl_ListObjGetElements sets "Result" */
    }

    if (listLen != 3 || 
        Tcl_GetDoubleFromObj(interp, elems[0], &x) == TCL_ERROR ||
        Tcl_GetDoubleFromObj(interp, elems[1], &y) == TCL_ERROR ||
        Tcl_GetDoubleFromObj(interp, elems[2], &z) == TCL_ERROR)
    {
        Tcl_Obj* errReturn = Tcl_NewStringObj("invalid coordinate list", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    /* if we made it here, should be in pretty good shape... */
    if (!strcmp(convDir, "LCVStoGCC")) 
        TsGeoLVCStoGCC(theGeoset, x, y, z, newCoords);
    else if (!strcmp(convDir, "GCCtoLVCS")) {
        float coords[3];
        TsGeoGCCtoLVCS(theGeoset, x, y, z, coords);
        newCoords[0] = coords[0];
        newCoords[1] = coords[1];
        newCoords[2] = coords[2];
    }
    else {
        Tcl_Obj* errReturn = 
            Tcl_NewStringObj("invalid conversion keyword\n", -1);
        Tcl_AppendToObj(errReturn, 
            "Should be either \"LVCStoGCC\" or \"GCCtoLVCS\"", -1);
        Tcl_SetObjResult(interp, errReturn);
        return TCL_ERROR;
    }

    retList = Tcl_NewListObj(0, NULL);
    for (j=0; j<3; j++) {
        Tcl_Obj* elem = Tcl_NewDoubleObj(newCoords[j]);
        Tcl_ListObjAppendElement(interp, retList, elem);
    }

    Tcl_SetObjResult(interp, retList);
    return TCL_OK;
}


/*
 * g e o s e t N o t F o u n d ( )
 *
 * Returns TCL_ERROR, after setting "result" to an appropriate error message.
 *
 */
int
geosetNotFound(Tcl_Interp* interp, char* geosetName)
{
    Tcl_Obj* errReturn = Tcl_NewStringObj("Could not find geoset: ", -1);
    Tcl_AppendToObj(errReturn, geosetName, -1);
    Tcl_SetObjResult(interp, errReturn);
    return TCL_ERROR;
}




/*
 * t s G e o P y r a m i d I n s t P r o c ( )
 *
 */
int
tsGeoPyramidInstProc(ClientData data, Tcl_Interp* interp, int objc, 
    Tcl_Obj *const objv[])
{
  return 0;
}
