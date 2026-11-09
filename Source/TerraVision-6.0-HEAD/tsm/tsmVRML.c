/*
 * Name: tsmVRML.c
 *
 * Description: 
 *   Contains user-level routines for writing out terrain datasets in
 *   VRML format with the SRI VRML 2.0 parse/build library. For details
 *   on this facility, please refer to the tsmApi documention at:
 *
 *      http://www.ai.sri.com/TerraVision/tsmApi/vrml
 *
 * Function List:
 *   int tsmWriteVtile( TsmConnection oiConnect, TsmConnection demConnect, 
 *	       int tx, int ty, int iLevel, TsmConnectionParams params )
 *   int tsmWriteAllVtiles( TsmConnection oiConnect, TsmConnection demConnect, 
 *                          TsmConnectionParams params )
 *   int tsmWriteAllTrees( TsmConnection oiConn, TsmConnection demConn, 
 *                         TsmConnectionParams params )
 *
 * Revision History:
 *   Created 27 April 1998 by Nat Bletter
 *
 *   $Id: tsmVRML.c,v 1.34.2.2 2001/08/17 18:54:48 leei Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#include "tsmInternal.h"
#include "tsmVRMLUtils.h"
#include "tsmConnectionInt.h"
#include "tsmConnectionParamsInt.h"

#include <vrml2/c_nodes.h>
#include <vrml2/vrml2.h>
#include "orient.h"

#define TREE_FILE_VERSION   "1.0"      /* Written to tree's WorldInfo node */
#define VTILE_FILE_VERSION  "1.0"      /* Written to vtile's WorldInfo node */

/* interface declaration for the vtile proto */

static char const vtileStr[] = 
"#VRML V2.0 utf8						\n\
EXTERNPROTO vtileProto [					\n\
  field SFNode   vrmlCoord					\n\
  field MFInt32  vrmlCoordIndex					\n\
  field SFNode   vrmlNorm					\n\
  field MFInt32  vrmlNormIndex					\n\
  field MFString vrmlTexUrl					\n\
  field SFNode   vrmlTexCoord					\n\
  field MFInt32  vrmlTexCoordIndex				\n\
  field SFNode   vrmlTexTransform				\n\
  field SFVec3f  upVector					\n\
  field MFString terrainTexUrn					\n\
  field MFString info						\n\
  field SFString title						\n\
  field SFInt32  tileX						\n\
  field SFInt32  tileY						\n\
  field SFInt32  polysPerSide					\n\
] [ \"../../protos/VTile/vtile.wrl\" 				\n\
 \"http://www.ai.sri.com/VRMLSets/v1.0/protos/VTile/vtile.wrl\" \n\
]								\n\
Group {                                                         \n\
  children DEF vtile vtileProto {				\n\
    vrmlCoord Coordinate {}					\n\
    vrmlTexCoord TextureCoordinate {}				\n\
    upVector 0 0 1						\n\
  }                                                             \n\
}";

/* EXTERNPROTO declaration and instance for the geotile file */

static char const geoTileStr[] = "		                    \n\
#VRML V2.0 utf8					                    \n\
						                    \n\
EXTERNPROTO GeoTile [				                    \n\
  field MFString terrainURL			                    \n\
  field MFString terrainName			                    \n\
  field MFString featureURL			                    \n\
  field MFString featureName			                    \n\
] [ \"../../protos/GeoTile/GeoTile.wrl\"	                    \n\
 \"http://www.ai.sri.com/VRMLSets/v1.0/protos/GeoTile/GeoTile.wrl\" \n\
]						                    \n";

static char const geoTileInstStr[] = "		                    \n\
Group {                                                             \n\
  children DEF geoTile GeoTile {}                                   \n\
}";

/* This is the EXTERNPROTO definition for the QuadLOD node */

static char const quadLODStr[] = "		                    \n\
EXTERNPROTO QuadLOD [                                               \n\
  field MFNode   parentNode                                         \n\
  field MFString parentUrl                                          \n\
  field MFString child1Url                                          \n\
  field MFString child2Url                                          \n\
  field MFString child3Url                                          \n\
  field MFString child4Url                                          \n\
  field SFVec3f  child1Center                                       \n\
  field SFVec3f  child1Size                                         \n\
  field SFVec3f  child2Center                                       \n\
  field SFVec3f  child2Size                                         \n\
  field SFVec3f  child3Center                                       \n\
  field SFVec3f  child3Size                                         \n\
  field SFVec3f  child4Center                                       \n\
  field SFVec3f  child4Size                                         \n\
  field SFVec3f parentSize                                          \n\
  field SFVec3f parentCenter                                        \n\
  field SFVec3f switchSize                                          \n\
  field SFBool enabled                                              \n\
  eventIn SFBool set_enabled                                        \n\
  eventOut SFBool enabled_changed                                   \n\
] [ \"../../protos/QuadLOD/QuadLOD.wrl#QuadLOD\"                    \n\
 \"http://www.ai.sri.com/VRMLSets/v1.0/protos/QuadLOD/QuadLOD.wrl#QuadLOD\" \n\
]                                                                   \n\
";

/* The GlobalState and Cache EXTERNPROTOs for the root QuadLOD tree file */

static char const globalStateStr[] = "		                    \n\
EXTERNPROTO GlobalState [                                           \n\
  field        MFString allTerrains                                 \n\
  eventOut     MFString allTerrains_changed                         \n\
  field        MFString allFeatures                                 \n\
  eventOut     MFString allFeatures_changed                         \n\
  field        SFString whichTerrain                                \n\
  eventIn      SFString set_whichTerrain                            \n\
  eventOut     SFString whichTerrain_changed                        \n\
  field        MFString whichFeatures                               \n\
  eventIn      MFString set_whichFeatures                           \n\
  eventOut     MFString whichFeatures_changed                       \n\
  eventOut     MFNode   children                                    \n\
  field        SFBool   debug                                       \n\
] [ \"../../protos/GeoTile/GlobalState.wrl\"                        \n\
 \"http://www.ai.sri.com/VRMLSets/v1.0/protos/GeoTile/GlobalState.wrl\" \n\
]                                                                   \n";

static char const cacheStr[] = "		                    \n\
EXTERNPROTO Cache [                                                 \n\
  field        SFInt32  cacheSize                                   \n\
  field        SFBool   debug                                       \n\
] [ \"../../protos/QuadLOD/Cache.wrl\"                              \n\
 \"http://www.ai.sri.com/VRMLSets/v1.0/protos/QuadLOD/Cache.wrl\"   \n\
]                                                                   \n";

static void
tsmAppendVRML( char **string, char const *vrml )
{
  static const char *header = "#VRML V2.0 utf8\n";

  if ( string == NULL || vrml == NULL ) return;

  /* If the vrml string has a VRML header in it, then skip over it */

  if ( *vrml == '#' ) 
    while ( *vrml != '\n' && *vrml != '\0' )
      vrml++;

  /* If we haven't got a string yet, then just copy the vrml string */

  if ( *string == NULL ) {
    *string = (char *) tsmMemMalloc( strlen(header) + strlen(vrml) + 1 );
    if ( *string == NULL ) return;
    strcpy( *string, header );
    strcat( *string, vrml );
    return;
  }

  /* Otherwise, we grow the string and append the new vrml string */

  *string = (char *) tsmMemRealloc( *string, strlen(*string)+strlen(vrml)+2 );
  if ( *string == NULL ) return;
  strcat( *string, "\n" );
  strcat( *string, vrml );
}

/*
 * Synopsis:
 *      void *tsmreateVtileProto( vsgWorld **wrlp, TsmConnectionParams params )
 *
 * Description:
 *      Functionally creates the appropriate EXTERNPROTO 
 *
 * Externals: None
 *
 * Returns: VRML_info = a pointer to the vtile VRML file (for scenegraph,etc.)
 *
 * Author: Nat Bletter
 * 
 * Date: 1/13/98
 *
 */

static void *
tsmCreateVtileProto( vsgWorld **wrlp, TsmConnectionParams params )
{
  const char *funcName = "tsmCreateVtileProto";
  vsgWorld   *wrl;
  char const *vtileProto_name, *vtile_name;

  wrl = *wrlp = vsgNewWorld (0);
  vsgParseString (wrl, vtileStr);

  vtileProto_name = vsgIntern (wrl, "vtileProto" );
  vtile_name = vsgIntern (wrl, "vtile" );

  /* If the user supplied a new VRML_URL, then change the default one */

  if (tsmIsParamSet(params, TSM_PARAM_VRML_URL)) {
    char                url[512];
    vsgExternProtoClass *vtileProto;

    vtileProto = (vsgExternProtoClass *) vsgGetClass( wrl, vtileProto_name );
    if ( vtileProto == NULL ) 
      tsmError( funcName, "Couldn't get EXTERNPROTO url for vtile!" );
    else {
      sprintf( url, "%s/protos/VTile/vtile.wrl", params->vrml_url );
      vtileProto->extern_urls->data[0] = vsgStrdup( wrl, url );
    }
  }

  return vsgUSE (wrl, vtile_name );
}

/*
 * Synopsis:
 *   void tsmDestroyProtoWrl( TsmConnection conn )
 *
 * Description:
 *   Destroys the VRML world context that is allocated for the
 *   vtile proto definition. This is called by tsmDisconnect.
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 6 February 1998
 *
 */

void
tsmDestroyProtoWrl( TsmConnection conn )
{
  if ( conn->protoWrl )
    vsgDestroyWorld( conn->protoWrl );
}

/*
 * Synopsis:
 *    tsmCreateTestVtile( double x1, double y1, double x2, double y2, FILE * )
 *
 * Description:
 *    Creates a VRML Shape node with a random colour and an IndexedFaceSet
 *    containing a single square polygon. You specify the coordinates of
 *    this polygon by providing the lower-left and upper-right coordinates.
 *
 * Parameters: 
 *    x1, y1 - the lower-left coordinates of the tile
 *    x2, y2 - the upper-right coordinates of the tile
 *
 * Returns: nothing
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

static void
tsmCreateTestVtile( double x1, double y1, double x2, double y2, FILE *fp )
{
  vsgWorld *wrl;
  SFNode shape, app, mat, ifs, coord;

  wrl = vsgNewWorld (0);
  vsgSetFiletype(wrl, "#VRML V2.0 utf8");

  vsgStartWorld(wrl);
  shape = vsgNewShape (wrl);
  vsgWorldAppend (wrl, shape);

  /* Set up the appearance field of the Shape node */
  app = vsgNewAppearance(wrl);
  mat = vsgNewMaterial(wrl);
  vsgSet (wrl, mat, vsgNdiffuseColor,
	  vsgMakeSFColor(( rand() % 255 ) / 256.0,
			 ( rand() % 255 ) / 256.0, 
			 ( rand() % 255 ) / 256.0));
  vsgSetAppearance_material(wrl, app, mat);
  vsgSetShape_appearance(wrl, shape, app);

  /* Set up the geometry field of the Shape node */
  ifs = vsgNewIndexedFaceSet(wrl);
  vsgSetShape_geometry(wrl, shape, ifs);

  coord = vsgNewCoordinate(wrl);
  vsgSetIndexedFaceSet_coord(wrl, ifs, coord);
  vsgSetIndexedFaceSet_solid(wrl, ifs, FALSE);

  {
    MFVec3f *pts = vsgStartMFVec3f(wrl);
    vsgAppendSFVec3f( wrl, pts, vsgMakeSFVec3f(x1, y1, 0.0 ) );
    vsgAppendSFVec3f( wrl, pts, vsgMakeSFVec3f(x2, y1, 0.0 ) );
    vsgAppendSFVec3f( wrl, pts, vsgMakeSFVec3f(x2, y2, 0.0 ) );
    vsgAppendSFVec3f( wrl, pts, vsgMakeSFVec3f(x1, y2, 0.0 ) );
    vsgEndMFVec3f(wrl, pts);
    vsgSetCoordinate_point(wrl, coord, pts);
  }

  {
    MFInt32 *coordIdx = vsgStartMFInt32 (wrl);
    vsgAppendSFInt32( wrl, coordIdx, 0 );
    vsgAppendSFInt32( wrl, coordIdx, 1 );
    vsgAppendSFInt32( wrl, coordIdx, 2 );
    vsgAppendSFInt32( wrl, coordIdx, 3 );
    vsgAppendSFInt32( wrl, coordIdx, -1 );
    vsgEndMFInt32(wrl, coordIdx);
    vsgSetIndexedFaceSet_coordIndex(wrl, ifs, coordIdx);
  }

  vsgEndWorld(wrl);
  vsgPrint(wrl, fp);
}

/*
 * Synopsis:
 *    void tsmCrossProduct( double const a[3], double const b[3], double c[3] )
 *    void tsmSubVector( double const a[3], double const b[3], double c[3] )
 *    void tsmNormalizeVector( double const *a, double *n, int dim )
 *
 * Description:
 *    A few vector math functions for computing the cross product and
 *    sub vector, as well as normalising a vector.
 *
 * Returns: void
 *
 */

void
tsmCrossProduct( double const a[3], double const b[3], double c[3] )
{
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

void
tsmSubVector( double const a[3], double const b[3], double c[3] )
{
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

void
tsmNormalizeVector( double const *a, double *n, int dim )
{
  double len = 0;
  double len2 = 0;
  int i;

  for (i = 0; i < dim; i++) 
    len2 += a[i] * a[i];

  len = sqrt(len2);
  for(i = 0; i < dim; i++)
    n[i] = a[i] / len;
}

int 
tsmOILevel2DEMLevel( Pyramid *oiPyramid, Pyramid *demPyramid, 
		     TsmConnectionParams params, int oiLevel, int *demLevel )
{
  const char    *funcName = "tsmOILevel2DEMLevel";
  int 		gaussPyr, pointPyr;
  int		i;
  int		dLevel;
  TileTspec     *tile = &oiPyramid->tileTspec;

  /* what type of pyramid is this? */
  gaussPyr = pointPyr = FALSE;
  for (i = 0; i < demPyramid->identityTspec.operations.len; i++) {
    if (demPyramid->identityTspec.operations.val[i] == tspGaussReduce)
      gaussPyr = TRUE;
    if (demPyramid->identityTspec.operations.val[i] == tspPointSample)
      pointPyr = TRUE;
  }

  if (gaussPyr && pointPyr) {
    /* wha? */
    tsmError( funcName, "Can't have gaussian and point reduced pyramid at "
	      "the same time!" );
    tsmVRMLCacheFree();
    return(0);
  }

  /* Work out the DEM level to use */
  if ( ((params->gaussian && gaussPyr) || (!params->gaussian && pointPyr)) 
       && demPyramid->projectionTspec.projectionType == 
       oiPyramid->projectionTspec.projectionType) {

    /* use appropriately filtered level and make sure it's in range */

    dLevel = floor(log((double)(tile->numPixelsX) * oiPyramid->minResolution 
			 / (params->num_polys * demPyramid->minResolution))
		     /log(2) + demPyramid->minLevel + oiLevel -
		     oiPyramid->minLevel);
    dLevel = max( demPyramid->minLevel, dLevel );
    dLevel = min( demPyramid->maxLevel, dLevel );

  }  
  else if (!params->gaussian)
    dLevel = demPyramid->minLevel; /* We'll down sample it manually */
  else {
    tsmError( funcName, "Can't do gaussian VRML of non-gaussian pyramid!" );
    tsmVRMLCacheFree();
    return(0);
  }

  *demLevel = dLevel;
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteVtile( TsmConnection oiConnect, TsmConnection demConnect, 
 *	       int tx, int ty, int iLevel, TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Creates a single VRML vtile file.
 *
 *   Writes a single VRML vtile file at level iLevel and location (tx,ty).
 *   A vtile file contains all of the
 *   geometry and links to the texture map imagery for one tile in a 
 *   multi-resolution pyramid. You supply a DEM and OI connection to 
 *   provide the elevation and imagery data.  
 *
 *   You can control the generation of the vtile through a number of 
 *   optional parameters in the TsmConnectionParams structure. You can
 *   set these parameters with the tsmSetParam call.
 *   The following parameters are currently supported:
 *
 *   \begin{itemize}
 *   \item TSM_PARAMS_VRML_DIR: 
 *   The vtile file will be written to the vtile directory inside this
 *   localdisk directory. This parameter is required - you should always
 *   set this with before calling this function.
 *   \item TSM_PARAMS_VRML_URL:
 *   By default, all of the URLs that are inserted into the vtile files
 *   will be relative. However, you can specify an absolute URL for all
 *   vtiles to use (i.e. for the image file) by setting this parameter.
 *   The URL should be for the root directory of the dataset, e.g.
 *   http://www.ai.sri.com/VRMLSets/Yosemite.
 *   \item TSM_PARAMS_NUM_POLYS:
 *   This setting specifies the number of polygons on one side of a tile.
 *   Increasing this value will give smoother geometric models, but at
 *   the computational expense of more polygons per scene.
 *   \item TSM_PARAMS_GAUSSIAN:
 *   Use gaussian interpolation instead of linear when interpolating
 *   the elevation data.
 *   \item TSM_PARAMS_TEST_VTILES:
 *   Ignore the DEM and OI datasets and just create a flat vtile that is
 *   randomly coloured. This can be useful for seeing the level of detail
 *   mechanism in action.
 *   \item TSM_PARAMS_FLAT:
 *   By default, all coordinates will be projected into geocentric coordinates
 *   i.e. an (x,y,z) offset from the center of the earth. This will take
 *   into account the earth's curvature. If you just want to produce a flat
 *   dataset where all elevations are relative to a plane, then use this
 *   option. N.B. in flat worlds, all coordinates are specified in a Z-up
 *   coordinate system with embedded Transform nodes to convert these into
 *   VRML's Y-up coordinate system.
 *   \item TSM_PARAMS_FAST_BBOXES:
 *   By default, all bounding boxes will be calculated by taking the min 
 *   and max of all coordinates in a tile. This can be a time consuming
 *   process and so we offer this alternative method which produces an
 *   approximation to a tile's bounding box but works much faster.
 *   \item TSM_PARAMS_NORMALS:
 *   If this parameter is set, then normals will be generated for all of
 *   the polyons in the vtile. This will produce smoother lighting effects.
 *   \item TSM_PARAMS_WORLD_SCALE:
 *   Every coordinate that is sent to the vtile file will be scaled (divided)
 *   by this value. A value of 1.0 means no scaling. This facility is
 *   useful because geocentric coordinates can be very large and some VRML
 *   browsers have problems with these large coordinates. Scaling the 
 *   coordinates down to the 10-100 m range gives good results.
 *   \item TSM_PARAMS_ELEV_SCALE:
 *   Every elevation value will be multiplied by this value before projecting
 *   into the display coordinate system (e.g. geocentric). This allows you
 *   to introduce a vertical exaggeration into your models. A value of 1.0
 *   means no vertical exaggeration.
 *   \end{itemize}
 *
 *
 * @param    oiConnect is a connection to a OI pyramid dataset.
 * @param    demConnect is a connection to the corresponding DEM pyramid 
 *		dataset.
 * @param    tx is the X position of the tile at this level
 * @param    ty is the Y position of the tile at this level
 * @param    level is the TerraVision level of the OI pyramid for this tile.
 * @param    params is a pointer to a TsmConnectionParams structure.
 *
 * @return   TRUE if all went just peachy, FALSE if it went pear shaped
 *
 * @author   Nat Bletter
 * @version  8 May 1998
 *
 * @see      tsmConnect, tsmSetParam, tsmWriteAllVtiles
 *
 */

int
tsmWriteVtile( TsmConnection oiConnect, TsmConnection demConnect, 
	       int tx, int ty, int iLevel, TsmConnectionParams params )
{
  const char *funcName = "tsmWriteVtile";
  Pyramid *oiPyramid, *demPyramid;
  vsgWorld *wrl1;
  SFNode coord, texCoord;
  MFVec3f *pts, *norms;
  MFVec2f *texPts;
  MFInt32 *coordIdx, *texIdx, *normIdx;
  SFVec3f upVector;
  MFString *jpg_url;

  double     p[3];
  double     pp[4][3], pn[4][3], cross[3], avg[3];
  int	     valid[4];
  int 	     nvalid;
  double     n[3];
  double     t[2];
  int        polyX, polyY;
  double     u, v, maxv, maxu;
  char       texurl[512], buffer1[512], buffer2[512], buffer3[512], buffer4[512];
  int	     demLevel;
  int        polyx, polyy;
  int        vrml_level;
  int        cUrn;
  double     ll[3], ur[3];	/* tile bounds */
  int 		i;
  SimpleTileSet    *tileset, *tileset0;
  float            *ixiytouv;
  TileTspec        *tile;
  char             filename[512];
  FILE             *filep;

  tsmVRMLCacheInit();

  /* make sure the prototype has been created once before */
  if (!oiConnect->protoWrl) {
    oiConnect->proto = tsmCreateVtileProto(&(oiConnect->protoWrl), params);
  }

  /* Take some convenience pointers to commonly used structures */
  oiPyramid = (Pyramid *)tsmGet(oiConnect, TSM_DATASET_TSPEC);
  demPyramid = (Pyramid *)tsmGet(demConnect, TSM_DATASET_TSPEC);
  tileset  = &oiPyramid->tspecLevel.val[iLevel];
  tileset0 = &oiPyramid->tspecLevel.val[oiPyramid->minLevel];
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  tile     = &oiPyramid->tileTspec;
  vrml_level = tsmTVToVRMLLevel( oiPyramid, iLevel );

  /* create directory and file for it */
  tsmMakeVtileFilename( oiPyramid, tx, ty, iLevel, params, filename );
  tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );
  if ( ( filep = fopen( filename, "w" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write vtile file: %s", filename );
    tsmVRMLCacheFree();
    return FALSE;
  }

  /* make real vtiles or test ones? */
  if (params->test_vtiles) {
    tsmTileToVRMLBoundingBox( oiPyramid, tx, ty, iLevel, ll, ur );
    tsmCreateTestVtile( ll[0], ll[1], ur[0], ur[1], filep );
    fclose(filep);
    tsmVRMLCacheFree();
    return TRUE;
  }

  if (!tsmOILevel2DEMLevel(oiPyramid, demPyramid, params, iLevel, &demLevel))
    return FALSE;

  /* Calculate the total size of the texture map, without the   */
  /* padding bytes around it - we don't want to create geometry */
  /* or map imagery for the padded region.                      */

  maxu = oiPyramid->minResolution * (tileset0->maxValidPixel[0]+1) + ixiytouv[2];
  maxv = oiPyramid->minResolution * (tileset0->maxValidPixel[1]+1) + ixiytouv[5];

  /* Now work out how many polygons we will split this tile into   */
  /* This will be the default setting for most tiles, but polygons */
  /* that contain only pad bytes are not generated.                */

  polyx = polyy = params->num_polys;

  if ( tx >= tileset->maxTile[0] )
    polyx = (int) ceil( params->num_polys * (double)( tileset->numValidPixel[0]
			- tx * tile->numPixelsX) /
			tile->numPixelsX );

  if ( ty >= tileset->maxTile[1] )
    polyy = (int) ceil( params->num_polys * (double)( tileset->numValidPixel[1]
			- ty * tile->numPixelsY) /
			tile->numPixelsY );
  
  /* put in all the coordinates first */
  wrl1 = vsgNewWorld (0);

  pts = vsgStartMFVec3f(wrl1);
  texPts = vsgStartMFVec2f(wrl1);
  if (params->normals)
    norms = vsgStartMFVec3f(wrl1);

  for ( polyY = 0; polyY <= polyy; polyY++ ) {
    for ( polyX = 0; polyX <= polyx; polyX++ ) {

      /* Work out the (x,y,z) coordinates for the geometry */

      u = tile->numPixelsX * ( tx + (double) polyX / params->num_polys ) * ixiytouv[0] + ixiytouv[2];
      v = tile->numPixelsY * ( ty + (double) polyY / params->num_polys ) * ixiytouv[4] + ixiytouv[5];

      /* make the ends of the world meet */
      if (tx == tileset->maxTile[0] && polyX == polyx && oiPyramid->wrap_lon)
	u = maxu;
      if (ty == tileset->maxTile[1] && polyY == polyy && oiPyramid->wrap_lat)
	v = maxv;

      /* Clip the coordinates to valid texture map pixels */

      u = min( u, maxu );
      v = min( v, maxv );
      
      tsmGetCoords( oiConnect, demConnect, u, v, demLevel, params, p,
		     TSM_ELEV_MODE_EXACT );
      vsgAppendSFVec3f(wrl1, pts, vsgMakeSFVec3f (p[0], p[1], p[2]));

      if (params->normals) {
	/* Do normals as well */
	/* get four surrounding points */
	valid[0] = tsmGetCoords( oiConnect, demConnect, min(tile->numPixelsX * ( tx + (double) (polyX + 1) / params->num_polys ) * ixiytouv[0] + ixiytouv[2], maxu), min(tile->numPixelsY * ( ty + (double) polyY / params->num_polys ) * ixiytouv[4] + ixiytouv[5], maxv), demLevel, params, pp[0], TSM_ELEV_MODE_EXACT );
	valid[1] = tsmGetCoords( oiConnect, demConnect, min(tile->numPixelsX * ( tx + (double) polyX / params->num_polys ) * ixiytouv[0] + ixiytouv[2], maxu), min(tile->numPixelsY * ( ty + (double) (polyY + 1)/ params->num_polys ) * ixiytouv[4] + ixiytouv[5], maxv), demLevel, params, pp[1], TSM_ELEV_MODE_EXACT );
	valid[2] = tsmGetCoords( oiConnect, demConnect, min(tile->numPixelsX * ( tx + (double) (polyX - 1) / params->num_polys ) * ixiytouv[0] + ixiytouv[2], maxu), min(tile->numPixelsY * ( ty + (double) polyY / params->num_polys ) * ixiytouv[4] + ixiytouv[5], maxv), demLevel, params, pp[2], TSM_ELEV_MODE_EXACT );
	valid[3] = tsmGetCoords( oiConnect, demConnect, min(tile->numPixelsX * ( tx + (double) polyX / params->num_polys ) * ixiytouv[0] + ixiytouv[2], maxu), min(tile->numPixelsY * ( ty + (double) (polyY - 1)/ params->num_polys ) * ixiytouv[4] + ixiytouv[5], maxv), demLevel, params, pp[3], TSM_ELEV_MODE_EXACT );
	/* and their normals */
	tsmSubVector(pp[0], p, pn[0]);
	tsmSubVector(pp[1], p, pn[1]);
	tsmSubVector(pp[2], p, pn[2]);
	tsmSubVector(pp[3], p, pn[3]);

	for (i = 0; i < 4; i++)
	  if (pn[i][0] == 0.0 && pn[i][1] == 0.0 && pn[i][2] == 0.0)
	    valid[i] = FALSE;

	/* now average the valid ones */
	nvalid =0;
	avg[0] = avg[1] = avg[2] = 0.0;
	for (i = 0; i < 4; i++)
	  if (valid[i] && valid[(i+1)% 4]) {
	    nvalid++;
	    tsmCrossProduct(pn[i], pn[(i+1)%4], cross);
	    avg[0] += cross[0];
	    avg[1] += cross[1];
	    avg[2] += cross[2];
	  }
	tsmNormalizeVector(avg, n, 3);
	vsgAppendSFVec3f(wrl1, norms, vsgMakeSFVec3f (n[0], n[1], n[2]));
      }

      /* And the image (u,v) texture coordinates too */

      tsmGetTexCoords( polyX, polyY, tx, ty, iLevel, oiPyramid, t,
		       u==maxu, v==maxv, params );
      vsgAppendSFVec2f( wrl1, texPts, vsgMakeSFVec2f (t[0], t[1]) );
    }
  }

  /* stick the coordinates into the proto scene graph */
  coord = vsgNewCoordinate (wrl1);
  vsgSetCoordinate_point(wrl1, coord, vsgEndMFVec3f (wrl1, pts));
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "vrmlCoord"), coord);

  texCoord = vsgNewTextureCoordinate (wrl1);
  vsgSetTextureCoordinate_point(wrl1, texCoord, vsgEndMFVec2f (wrl1, texPts));
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "vrmlTexCoord"), texCoord);

  if (params->normals) {
    coord = vsgNewCoordinate (wrl1);
    vsgSetCoordinate_point(wrl1, coord, vsgEndMFVec3f (wrl1, norms));
    vsgSet (oiConnect->protoWrl, oiConnect->proto,
	    vsgIntern(oiConnect->protoWrl, "vrmlNorm"), coord);
  }

  /* now set up the indices into the above coordinate lists */
  coordIdx = vsgStartMFInt32(wrl1);
  texIdx = vsgStartMFInt32(wrl1);
  if (params->normals) 
    normIdx = vsgStartMFInt32(wrl1);

  for ( polyY = 0; polyY < polyy; polyY++ ) {
    for ( polyX = 0; polyX < polyx; polyX++ ) {

      /* first the spatial coordinate indices */

      vsgAppendSFInt32( wrl1, coordIdx, polyY * (polyx + 1) + polyX );
      vsgAppendSFInt32( wrl1, coordIdx, polyY * (polyx + 1) + polyX + 1 );
      vsgAppendSFInt32( wrl1, coordIdx, (polyY + 1) * (polyx + 1) + polyX+1);
      vsgAppendSFInt32( wrl1, coordIdx, (polyY + 1) * (polyx + 1) + polyX );
      vsgAppendSFInt32( wrl1, coordIdx, -1 );

      /* then the texture coordinate indices */

      vsgAppendSFInt32( wrl1, texIdx, polyY * (polyx + 1) + polyX );
      vsgAppendSFInt32( wrl1, texIdx, polyY * (polyx + 1) + polyX + 1 );
      vsgAppendSFInt32( wrl1, texIdx, (polyY+1) * (polyx+1) + polyX+1 );
      vsgAppendSFInt32( wrl1, texIdx, (polyY+1) * (polyx+1) + polyX );
      vsgAppendSFInt32( wrl1, texIdx, -1 );

      if (params->normals) {
	/* then the normal indices */

	vsgAppendSFInt32( wrl1, normIdx, polyY * (polyx + 1) + polyX );
	vsgAppendSFInt32( wrl1, normIdx, polyY * (polyx + 1) + polyX + 1 );
	vsgAppendSFInt32( wrl1, normIdx, (polyY+1) * (polyx+1) + polyX+1 );
	vsgAppendSFInt32( wrl1, normIdx, (polyY+1) * (polyx+1) + polyX );
	vsgAppendSFInt32( wrl1, normIdx, -1 );
      }
    }
  }

  vsgEndMFInt32 (wrl1, coordIdx);
  vsgEndMFInt32 (wrl1, texIdx);

  /* stick indices into proto graph */
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "vrmlCoordIndex"), coordIdx);
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "vrmlTexCoordIndex"), texIdx);

  if (params->normals) {
    vsgEndMFInt32 (wrl1, normIdx);
    vsgSet (oiConnect->protoWrl, oiConnect->proto,
	    vsgIntern(oiConnect->protoWrl, "vrmlNormIndex"), normIdx);
  }

  /* Set the URL for the JPG texture map image */

  sprintf( texurl, "%s/images/%d/p%dp%d.jpg", params->vrml_url, vrml_level,
	   tx, ty );

  jpg_url = vsgStartMFString( wrl1 );
  vsgAppendSFString( wrl1, jpg_url, texurl );
  vsgEndMFString( wrl1, jpg_url );
  vsgSet( oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "vrmlTexUrl"), jpg_url );
  

#ifdef TEXTRANSFORM
  vrmlTexTransform->scale =
    vsgMakeSFVec2f( (double) tileset->numValidPixel[0] / tile->numPixelsX,
		    (double) tileset->numValidPixel[1] / tile->numPixelsY );
  vrmlTexTransform->translation =
    vsgMakeSFVec2f( -tx * ((double)tile->numPixelsX - 1.0) / tileset->numValidPixel[0],
		    -ty  * ((double)tile->numPixelsY - 1.0)/ tileset->numValidPixel[1]);
#endif
  
  /* vector perpendicular to terrain centroid? */
  tsmTileToUpVector(oiPyramid, tx, ty, iLevel, params, upVector.v);
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "upVector"), upVector);

  /* add all the terrain tex URNs to the list */
  {
    MFString *urns = vsgStartMFString(wrl1);
    for (cUrn = 0; cUrn < params->nTexUrns; cUrn++)
      vsgAppendSFString( wrl1, urns, params->texUrns[cUrn]);
    vsgEndMFString (wrl1, urns);
    vsgSet (oiConnect->protoWrl, oiConnect->proto,
	    vsgIntern(oiConnect->protoWrl, "terrainTexUrn"), urns);
  }
  
  /* info node */
  {
    MFString *info = vsgStartMFString(wrl1);
    sprintf( buffer1, "TerraVision II vtile file V%s", VTILE_FILE_VERSION );
    vsgAppendSFString( wrl1, info, buffer1 );

    sprintf( buffer2, "Index: %d %d", tx, ty );
    vsgAppendSFString( wrl1, info, buffer2 );

    sprintf( buffer3, "Level: %d", vrml_level );
    vsgAppendSFString( wrl1, info, buffer3 );

    sprintf( buffer4, "PolysPerSide: %d", params->num_polys );
    vsgAppendSFString( wrl1, info, buffer4 );

    vsgAppendSFString( wrl1, info, oiPyramid->description );

    vsgEndMFString (wrl1, info);
    vsgSet (oiConnect->protoWrl, oiConnect->proto,
	    vsgIntern(oiConnect->protoWrl, "info"), info);
  }

  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "title"), oiPyramid->description);

  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "tileX"), tx);
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "tileY"), ty);
  vsgSet (oiConnect->protoWrl, oiConnect->proto,
	  vsgIntern(oiConnect->protoWrl, "polysPerSide"), params->num_polys);

  /* ready to print it */
  vsgPrint(oiConnect->protoWrl, filep);
  fclose( filep );

  /* delete all the nodes for the next time around */
  vsgDestroyWorld (wrl1);
  tsmVRMLCacheFree();

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteAllVtiles( TsmConnection oiConnect, TsmConnection demConnect, 
 *                          TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Creates a pyramid of VRML vtile files.
 *
 *   Creates part or all of a pyramid of VRML vtile files. This function
 *   calls tsmWriteVtile in order to write out multiple vtile files for
 *   a dataset. By default, the entire pyramid of vtile files will be
 *   generated. However, you can specify the start and end levels of the
 *   pyramid to produce if you wish.
 *
 *   This function supports all of TsmConnectionParams settings that the
 *   tsmWriteVtile function supports. In addition, the following settings
 *   can be defined:
 *
 *   \begin{itemize}
 *   \item TSM_PARAMS_START_LEVEL:
 *    This value specifies the lowest resolution (TerraVision style)
 *    level number at which the process should begin, e.g. if the
 *    TerraVision levels are 0..9, then a start level of 8 will begin
 *    at the second lowest resolution level. If you do not specify a
 *    value, then the lowest level will be assumed.
 *   \item TSM_PARAMS_END_LEVEL:
 *    This value specifies the highest resolution (TerraVision style)
 *    level number at which the process should stop, e.g. if the
 *    TerraVision levels are 0..9, then a start level of 1 will stop
 *    at the second highest resolution level. If you do not specify a
 *    value, then the highest level (probably 0) will be assumed.
 *   \end{itemize}
 *
 * @param    oiConnect is a connection to a OI pyramid dataset.
 * @param    demConnect is a connection to the corresponding DEM pyramid 
 *		dataset.
 * @param    params is a pointer to a TsmConnectionParams structure.
 *
 * @return   TRUE if all went just peachy, FALSE if it went pear shaped
 *
 * @author   Nat Bletter
 * @version  8 May 1998
 *
 * @see      tsmConnect, tsmSetParam, tsmWriteAllVtiles
 *
 */

int
tsmWriteAllVtiles( TsmConnection oiConnect, TsmConnection demConnect,
		   TsmConnectionParams params )
{
  const char    *funcName = "CreateAllVtiles";
  SimpleTileSet *tileset;            /* pointer to a level's simple tileset */
  char          filename[512];       /* filename used to write stuff to disk */
  int           lev, tx, ty;         /* loop variables */
  int           vrml_lev;            /* the VRML version of TV's level */
  Pyramid       *oi_tspec, *demPyramid;
  int		start_level;
  int		end_level;
  int 		i;
  TileTspec	*tile;
  int		demLevel;
  int		width;
  int 		gaussPyr, pointPyr;

  tsmVRMLCacheInit();

  oi_tspec = (Pyramid *)tsmGet(oiConnect, TSM_DATASET_TSPEC);
  demPyramid = (Pyramid *)tsmGet(demConnect, TSM_DATASET_TSPEC);
  tile     = &oi_tspec->tileTspec;

  /* Set up the start and end levels for the main loop */

  if ( params->start_level < 0 ) 
    start_level = oi_tspec->maxLevel;
  else
    start_level = params->start_level;

  if ( params->end_level < 0 )
    end_level = oi_tspec->minLevel;
  else
    end_level = params->end_level;
  
  /* Ensure that these are in range, and in the correct order */

  if ( start_level < oi_tspec->minLevel || start_level > oi_tspec->maxLevel ) {
    tsmError( funcName, "Start level %d not in valid range (%d..%d)",
	      start_level, oi_tspec->minLevel, oi_tspec->maxLevel );
    start_level = min( max( start_level, oi_tspec->minLevel ),
		       oi_tspec->maxLevel );
  }

  if ( end_level < oi_tspec->minLevel || end_level > oi_tspec->maxLevel ) {
    tsmError( funcName, "End level %d not in valid range (%d..%d)",
	      end_level, oi_tspec->minLevel, oi_tspec->maxLevel );
    end_level = min( max( end_level, oi_tspec->minLevel ), oi_tspec->maxLevel);
  }

  if ( start_level < end_level ) {
    tsmError( funcName, "start level must be higher then end level" );
    return FALSE;
  }

  gaussPyr = pointPyr = FALSE;
  for (i = 0; i < demPyramid->identityTspec.operations.len; i++) {
    if (demPyramid->identityTspec.operations.val[i] == tspGaussReduce)
      gaussPyr = TRUE;
    if (demPyramid->identityTspec.operations.val[i] == tspPointSample)
      pointPyr = TRUE;
  }

  if (gaussPyr && pointPyr) {
    /* wha? */
    tsmError( funcName, "Can't have gaussian and point reduced pyramid at "
	      "the same time!" );
    tsmVRMLCacheFree();
    return FALSE;
  }

  /* Work out the DEM level to use */

  if ( (params->gaussian && gaussPyr) || (!params->gaussian && pointPyr)) {
				/* OK for now */
  } 
  else if (!params->gaussian && gaussPyr) {
    tsmError( funcName, "Can't create Gaussian VRML vtiles from non-gaussian "
	      "reduced DEM Pyramid. \nTry creating a new gaussian reduced "
	      "Pyramid from the original data." );
    tsmVRMLCacheFree();
    return FALSE;
  }
  else {			/* point sampled DEM needed */
    tsmError( funcName, "Creating point sampled VRML vtiles from a gaussian "
	      "reduced dataset may be slow.\nTry creating a point sampled "
	      "Pyramid instead to make it faster." );
  }

  fprintf( stderr, "Processing %s (%d levels):\n", oi_tspec->description,
	   start_level - end_level + 1 );

  /* Create the vtiles for each level of the pyramid... */
  for ( lev = start_level; lev >= end_level; --lev ) {

    /* Create the directory for this level of VRML vtile files */

    tileset = &oi_tspec->tspecLevel.val[lev];
    tsmMakeVtileFilename( oi_tspec, 0, 0, lev, params, filename );
    tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );

    vrml_lev = tsmTVToVRMLLevel( oi_tspec, lev );
    fprintf( stderr, "Creating level %d vtiles...\n", vrml_lev );

    if (!tsmOILevel2DEMLevel(oi_tspec, demPyramid, params, lev, &demLevel))
      return FALSE;

    width = ceil(pow(2, lev - oi_tspec->minLevel) * oi_tspec->minResolution *
	       tile->numPixelsX / ( pow(2, demLevel - demPyramid->minLevel) *
	       demPyramid->minResolution * demPyramid->tileTspec.numPixelsX));

    /* Create vtile files for all of the tiles in this level */

    for ( ty = tileset->minTile[1]; ty <= tileset->maxTile[1]; ty += width ) {
      for ( tx = tileset->minTile[0]; tx <= tileset->maxTile[0]; tx += width) {
	int ix,iy;
	for ( ix = 0; ix < width && ix + tx <= tileset->maxTile[0]; ix++)
	  for ( iy = 0; iy < width && iy + ty <= tileset->maxTile[1]; iy++)
	  
	    /* Build the entire vtile structure for this tile. The  */
	    /* CreateVtileProto call initialise the scene graph for */
	    /* the vtile. This can be retrieved with Get_VRML_Info. */
	    tsmWriteVtile( oiConnect, demConnect, tx+ix, ty+iy, lev, params );

      }
    }
  }

  tsmVRMLCacheFree();

  return TRUE;
}

/*
 * Synopsis:
 *    SFVec3f tsmBBoxSize( double const *ll, double const *ur )
 *    SFVec3f tsmBBoxCenter( double const *ll, double const *ur )
 *
 * Description:
 *    Takes two coordinates (provided as a vector of 3 doubles in each
 *    case) which represent the lower-left and upper-right corners of
 *    a bounding box. The functions then return an SFVec3f containing
 *    the size of that bounding box and it's center, respectively.
 *
 * Returns: An SFVec3f, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 28 January 1998
 *
 */

static SFVec3f
tsmBBoxSize( double const ll[3], double const ur[3] )
{
  return vsgMakeSFVec3f( (SFFloat) (ur[0] - ll[0]),
			 (SFFloat) (ur[1] - ll[1]),
			 (SFFloat) (ur[2] - ll[2]) );
}

static SFVec3f
tsmBBoxCenter( double const ll[3], double const ur[3] )
{
  return vsgMakeSFVec3f( (SFFloat) (ll[0] + ( ur[0] - ll[0] ) / 2.0),
			 (SFFloat) (ll[1] + ( ur[1] - ll[1] ) / 2.0),
			 (SFFloat) (ll[2] + ( ur[2] - ll[2] ) / 2.0) );
}

/*
 * Synopsis:
 *   TsmCachedTile *tsmEstimateBoundingBox( TsmConnection oiConn,
 *                    TsmConnection demConn, int tx, int ty, int level,
 *                    TsmConnectionParams params )
 *
 * Description:
 *    Calculates the bounding box for the specified tile and returns this
 *    as a TsmCachedTile structure which contains both a min/max vertex
 *    format as well as a center/size format. 
 *
 *    This function only *approximates* the bounding box - it does not
 *    calculate a tightest possible box. It works by taking the (u,v)
 *    coordiante for all four corners of the tile and the center (u,v).
 *    We then simply use the min and max elevation of the entire dataset
 *    to estimate the elevation. This means that we do not need to read
 *    any DEM tiles or do any interpolation of multiple elevation points
 *    per DEM tile. We simply do 10 transformations into the final 
 *    coordinate system and take the min/max of these.
 *
 *    N.B. for geocentric datasets which cover a large area (deemed to
 *    be 45 deg) we increase the number of samples that are made in
 *    order to account for the earth's curvature - i.e. 5 samples is
 *    not enough for a full earth dataset wrapped around a sphere.
 *
 *    N.B. you will not generally call this function directly. Use either
 *    of the tsmSetBoundingBox or tsmSetVRMLBoundingBox functions.
 *
 *    See also: tsmCalcBoundingBox
 *
 * Returns: the bounding box information in a TsmCachedTile structure
 *
 * Author: Martin Reddy
 *
 * Date: 11 August 1998
 *
 */

static TsmCachedTile *
tsmEstimateBoundingBox( TsmConnection oiConn, TsmConnection demConn,
			int tx, int ty, int level, TsmConnectionParams params )
{
  int           first_time = TRUE;
  double        p[3];
  double        polyX, polyY;
  double        x, y;
  double        maxix, maxiy;
  float         coverage, increment;
  int 		demLevel;
  float         *ixiytouv;
  TileTspec     *tile;
  SimpleTileSet *tileset, *tileset0;
  Pyramid       *oiPyramid, *demPyramid;
  static TsmCachedTile cached_bbox;

  /* Otherwise, we're going to have to calculate the bounding box */
  /* by looking at every single coordinate in the tile...         */

  oiPyramid = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  demPyramid = (Pyramid *) tsmGet( demConn, TSM_DATASET_TSPEC );

  /* Take some convenience pointers to commonly used structures */

  tileset  = &oiPyramid->tspecLevel.val[level];
  tileset0 = &oiPyramid->tspecLevel.val[oiPyramid->minLevel];
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  tile     = &oiPyramid->tileTspec;

  /* Work out the DEM level to use */

  if ( ! tsmOILevel2DEMLevel( oiPyramid, demPyramid, params, level, &demLevel))
    return FALSE;

  /* See CreateVtile func for description of maxix/polyx vars */

  maxix = oiPyramid->minResolution * tileset0->maxValidPixel[0];
  maxiy = oiPyramid->minResolution * tileset0->maxValidPixel[1];

  /* For geocentric lat/long pyramids, we will need more samples */
  /* because the datasets is curved along the earth's surface.   */
  /* Therefore, increase the sample number for res > 45 deg.     */

  coverage  = 0.0;
  increment = 0.5;

  if ( params->flat == TSM_GEOCNTRC_VTILES )
    if ( demPyramid->coordTspec.componentUnits == tspDegrees )
      coverage = demPyramid->minResolution * 
        demPyramid->tspecLevel.val[demPyramid->minLevel].numValidPixel[0];

  if ( coverage > 45.0 ) increment = 0.1;

  /* read each of the sampled coordinates first */

  for ( polyY = 0; polyY <= 1.0; polyY += increment ) {
    for ( polyX = 0; polyX <= 1.0; polyX += increment ) {

      /* only do the four corners and the center point, */
      /* unless the dataset is geocentric, > 45 deg.    */

      if ( coverage < 45.0 && 
	   ( ( polyX == 0.5 && polyY != 0.5 ) ||
	     ( polyY == 0.5 && polyX != 0.5 ) ) ) 
	continue;

      /* Work out the (x,y,z) coordinates for the geometry */

      x = tile->numPixelsX * ( tx + polyX ) * ixiytouv[0];
      y = tile->numPixelsY * ( ty + polyY ) * ixiytouv[4];

      /* Clip the coordinates to valid texture map pixels */

      x = min( x, maxix ) + ixiytouv[2];
      y = min( y, maxiy ) + ixiytouv[5];
      
      /* Get the minimum coordinate for this point */

      tsmGetCoords( oiConn, demConn, x, y, demLevel, params, p,
		     TSM_ELEV_MODE_MIN );

      /* Check to see if we need to update the min or max elevation */

      if ( first_time == TRUE ) {
	cached_bbox.ll[0] = cached_bbox.ur[0] = p[0];
	cached_bbox.ll[1] = cached_bbox.ur[1] = p[1];
	cached_bbox.ll[2] = cached_bbox.ur[2] = p[2];
	first_time = FALSE;
      } else {
	cached_bbox.ll[0] = min( cached_bbox.ll[0], p[0]);
	cached_bbox.ll[1] = min( cached_bbox.ll[1], p[1]);
	cached_bbox.ll[2] = min( cached_bbox.ll[2], p[2]);
	cached_bbox.ur[0] = max( cached_bbox.ur[0], p[0]);
	cached_bbox.ur[1] = max( cached_bbox.ur[1], p[1]);
	cached_bbox.ur[2] = max( cached_bbox.ur[2], p[2]);
      }

      /* Get the maximum coordinate for this point */

      tsmGetCoords( oiConn, demConn, x, y, demLevel, params, p,
		    TSM_ELEV_MODE_MAX );

      cached_bbox.ll[0] = min( cached_bbox.ll[0], p[0]);
      cached_bbox.ll[1] = min( cached_bbox.ll[1], p[1]);
      cached_bbox.ll[2] = min( cached_bbox.ll[2], p[2]);
      cached_bbox.ur[0] = max( cached_bbox.ur[0], p[0]);
      cached_bbox.ur[1] = max( cached_bbox.ur[1], p[1]);
      cached_bbox.ur[2] = max( cached_bbox.ur[2], p[2]);
    }
  }

  /* okay, so now we have filled in the min and max coordinates of the */
  /* tile, let's fill in the other fields of the BoundingBox structure */

  cached_bbox.tx     = tx;
  cached_bbox.ty     = ty;
  cached_bbox.level  = level;
  cached_bbox.buffer = NULL;
  
  cached_bbox.size[0] = cached_bbox.ur[0] - cached_bbox.ll[0];
  cached_bbox.size[1] = cached_bbox.ur[1] - cached_bbox.ll[1];
  cached_bbox.size[2] = cached_bbox.ur[2] - cached_bbox.ll[2];

  cached_bbox.mid[0] = cached_bbox.ll[0] + cached_bbox.size[0] / 2.0;
  cached_bbox.mid[1] = cached_bbox.ll[1] + cached_bbox.size[1] / 2.0;
  cached_bbox.mid[2] = cached_bbox.ll[2] + cached_bbox.size[2] / 2.0;

  return &cached_bbox;
}

/*
 * Synopsis:
 *    TsmCachedTile *tsmCalcBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
 *                     TsmConnection demConn, int tx, int ty, int level, 
 *                     TsmConnectionParams params )
 *
 * Description:
 *    Calculates the bounding box for the specified tile and returns this
 *    as a TsmCachedTile structure which contains both a min/max vertex
 *    format as well as a center/size format.
 *
 *    This function will explicitly perform a projection into geocentric
 *    coordinates for every point in the tile and take the min and max
 *    of these (x,y,z) tuples. This function will therefore return the
 *    tightest possible bounding box for a tile.
 *
 *    N.B. you will not generally call this function directly. Use either
 *    of the tsmSetBoundingBox or tsmSetVRMLBoundingBox functions.
 *
 *    See also: tsmEstimateBoundingBox
 *
 * Returns: the bounding box information in a TsmCachedTile structure
 *
 * Author: Martin Reddy
 *
 * Date: 27 January 1998
 *
 */

static TsmCachedTile *
tsmCalcBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
		    TsmConnection demConn, int tx, int ty, int level, 
		    TsmConnectionParams params )
{
  int           first_time = TRUE;
  double        p[3];
  int           polyX, polyY /*, polyx, polyy*/;
  double        x, y;
  double        maxix, maxiy;
  int 		demLevel;
  float         *ixiytouv;
  TileTspec     *tile;
  SimpleTileSet *tileset, *tileset0;
  TsmCachedTile *cached_bbox;
  Pyramid       *oiPyramid, *demPyramid;

  /* If -fastbbox has been set, then use the fast approx to bounding box */

  if ( tsmIsParamSet( params, TSM_PARAM_FAST_BBOXES ) && params->fast_bbox )
    return tsmEstimateBoundingBox( oiConn, demConn, tx, ty, level, params );

  /* Check to see if the bounding box information for this tile */
  /* has already been cached. If so, then just return the info. */
  
  if ( ( cached_bbox = tsmVRMLCacheGetBBox( tx, ty, level ) ) != NULL )
    return cached_bbox;

  /* allocate a new bounding box structure to fill in */

  cached_bbox = (TsmCachedTile *) tsmMemMalloc( sizeof(TsmCachedTile) );
  if ( cached_bbox == NULL )
    return NULL;

  /* Otherwise, we're going to have to calculate the bounding box */
  /* by looking at every single coordinate in the tile...         */

  oiPyramid = (Pyramid *)tsmGet(oiConn, TSM_DATASET_TSPEC);
  demPyramid = (Pyramid *)tsmGet(demConn, TSM_DATASET_TSPEC);

  /* Take some convenience pointers to commonly used structures */

  tileset  = &oiPyramid->tspecLevel.val[level];
  tileset0 = &oiPyramid->tspecLevel.val[oiPyramid->minLevel];
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  tile     = &oiPyramid->tileTspec;

  /* Work out the DEM level to use */
  if (! tsmOILevel2DEMLevel(oiPyramid, demPyramid, params, level, &demLevel))
    return FALSE;

  /* See CreateVtile func for description of maxix/polyx vars */

  maxix = oiPyramid->minResolution * tileset0->maxValidPixel[0];
  maxiy = oiPyramid->minResolution * tileset0->maxValidPixel[1];

  /*
  polyx = polyy = params->num_polys;

  if ( tx >= tileset->maxTile[0] )
    polyx = (int) ceil( (double) tileset->numValidPixel[0] *
			params->num_polys / tileset->numPixels[0] );

  if ( ty >= tileset->maxTile[1] )
    polyy = (int) ceil( (double) tileset->numValidPixel[1] *
			params->num_polys / tileset->numPixels[1] );
  */
  
  /* read each of the sampled coordinates first */

  for ( polyY = 0; polyY <= params->num_polys; polyY++ ) {
    for ( polyX = 0; polyX <= params->num_polys; polyX++ ) {

      /* Work out the (x,y,z) coordinates for the geometry */

      x = tile->numPixelsX * ( tx + (double) polyX / params->num_polys )
	  * ixiytouv[0];
      y = tile->numPixelsY * ( ty + (double) polyY / params->num_polys )
	  * ixiytouv[4];

      /* Clip the coordinates to valid texture map pixels */

      x = min( x, maxix );
      y = min( y, maxiy );
      
      x += ixiytouv[2];
      y += ixiytouv[5];

      tsmGetCoords( oiConn, demConn, x, y, demLevel, params, p,
		    TSM_ELEV_MODE_EXACT );

      /* Check to see if we need to update the min or max elevation */
      if ( first_time == TRUE ) {
	cached_bbox->ll[0] = cached_bbox->ur[0] = p[0];
	cached_bbox->ll[1] = cached_bbox->ur[1] = p[1];
	cached_bbox->ll[2] = cached_bbox->ur[2] = p[2];
	first_time = FALSE;
      } else {
	cached_bbox->ll[0] = min( cached_bbox->ll[0], p[0]);
	cached_bbox->ll[1] = min( cached_bbox->ll[1], p[1]);
	cached_bbox->ll[2] = min( cached_bbox->ll[2], p[2]);
	cached_bbox->ur[0] = max( cached_bbox->ur[0], p[0]);
	cached_bbox->ur[1] = max( cached_bbox->ur[1], p[1]);
	cached_bbox->ur[2] = max( cached_bbox->ur[2], p[2]);
      }

    }
  }

  /* okay, so now we have filled in the min and max coordinates of the */
  /* tile, let's fill in the other fields of the BoundingBox structure */

  cached_bbox->tx     = tx;
  cached_bbox->ty     = ty;
  cached_bbox->level  = level;
  cached_bbox->buffer = NULL;
  
  cached_bbox->size[0] = cached_bbox->ur[0] - cached_bbox->ll[0];
  cached_bbox->size[1] = cached_bbox->ur[1] - cached_bbox->ll[1];
  cached_bbox->size[2] = cached_bbox->ur[2] - cached_bbox->ll[2];

  cached_bbox->mid[0] = cached_bbox->ll[0] + cached_bbox->size[0] / 2.0;
  cached_bbox->mid[1] = cached_bbox->ll[1] + cached_bbox->size[1] / 2.0;
  cached_bbox->mid[2] = cached_bbox->ll[2] + cached_bbox->size[2] / 2.0;

  /* Add this tile to the cache */

  tsmVRMLCacheAddBBox( cached_bbox );

  /*printf( "Calculated bbox %d %d %d\n", tx, ty, level );*/

  return cached_bbox;
}

/*
 * Synopsis:
 *    void tsmSetBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
 *                 TsmConnection demConn, int tx, int ty, int level, 
 *                 TsmConnectionParams params,
 *                 double *ll, double *ur )
 *
 * Description:
 *    Calculates the bounding box for the specified tile and returns this
 *    as two points in 3 space: one for the lower-left (x,y,z) and one
 *    for the upper-right (x,y,z).
 *
 *    If you have set TSM_PARAM_FAST_BBOXES to TRUE, then an fast
 *    approximation to the bounding box will be calculated, otherwise
 *    the tightest possible bounding box is calculated by checking every
 *    coordinate in the tile.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 August 1998
 *
 */

static void
tsmSetBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
		   TsmConnection demConn, int tx, int ty, int level, 
		   TsmConnectionParams params, double *ll, double *ur )
{
  TsmCachedTile *bbox;

  bbox = tsmCalcBoundingBox( wrl, oiConn, demConn, tx, ty, level, params );

  ll[0] = bbox->ll[0];
  ll[1] = bbox->ll[1];
  ll[2] = bbox->ll[2];

  ur[0] = bbox->ur[0];
  ur[1] = bbox->ur[1];
  ur[2] = bbox->ur[2];
}

/*
 * Synopsis:
 *   void tsmSetVRMLBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
 *                     TsmConnection demConn, int tx, int ty, int level, 
 *                     TsmConnectionParams params,
 *                     SFVec3f *size, SFVec3f *center )
 *
 * Description:
 *    Calculates the bounding box for the specified tile and returns this
 *    as two vectors: one giving the size of the bounding box (w,h,d) and
 *    the giving the center point of the box (x,y,z). This is the 
 *    representation that VRML uses for bounding boxes.
 *
 *    If you have set TSM_PARAM_FAST_BBOXES to TRUE, then an fast
 *    approximation to the bounding box will be calculated, otherwise
 *    the tightest possible bounding box is calculated by checking every
 *    coordinate in the tile.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 August 1998
 *
 */

static void
tsmSetVRMLBoundingBox( vsgWorld *wrl, TsmConnection oiConn, 
		       TsmConnection demConn, int tx, int ty, int level, 
		       TsmConnectionParams params,
		       SFVec3f *size, SFVec3f *center )
{
  TsmCachedTile *bbox;

  bbox = tsmCalcBoundingBox( wrl, oiConn, demConn, tx, ty, level, params );

  /* Now put this information into a center/size format */

  *size   = vsgMakeSFVec3f( bbox->size[0], bbox->size[1], bbox->size[2] );
  *center = vsgMakeSFVec3f( bbox->mid[0], bbox->mid[1], bbox->mid[2] );
}

/*
 * Synopsis:
 *    void tsmMakeGeotileRedirectFilename( Pyramid *tspec, int tx, int ty,
 *              int level, TsmConnectionParams params, char *filename )
 *
 * Description:
 *    Fills in the filename string buffer with the name of the url
 *    that is linked to from a TSM_TREE_TYPE_ANCHOR tree file in order
 *    to display the terrain.  If we are using geotiles, then this
 *    will be a geotile file, otherwise it will be a vtile file.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 1 October 1998
 * */

static void
tsmMakeGeotileRedirectFilename( Pyramid *tspec, int tx, int ty, int level,
				TsmConnectionParams params, char *filename )
{

  /* The URL that the Inline node of an old-style tree file links to */

  if ( params->geotiles )
    sprintf( filename, "%s/geotiles/%d/p%dp%d.wrl", params->vrml_url,
	     tsmTVToVRMLLevel( tspec, level ), tx, ty ); 
  else
    sprintf( filename, "%s/vtiles/%d/p%dp%d.wrl", params->vrml_url,
	     tsmTVToVRMLLevel( tspec, level ), tx, ty ); 
}

/*
 * Synopsis:
 *    void tsmMakeTreeRedirectFilename( Pyramid *tspec, int tx, int ty,
 *                     int level, TsmConnectionParams params, char *filename )
 *
 * Description:
 *    Fills in the filename string buffer with the name of the url that
 *    is linked to for the children of a QuadLOD node.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 1 October 1998
 *
 */

static void
tsmMakeTreeRedirectFilename( Pyramid *tspec, int tx, int ty, int level,
			     TsmConnectionParams params, char *filename )
{
  
  /* The URL that each childNUrl field of a QuadLOD links to */

  sprintf( filename, "%s/trees/%d/p%dp%d.wrl", params->vrml_url,
	   tsmTVToVRMLLevel( tspec, level ), tx, ty );
}

/*
 * Synopsis:
 *   SFNode tsmCreateInline( vsgWorld *wrl, TsmConnection oiConn, 
 *               TsmConnection demConn, int tx, int ty, int level, int tree
 *               TsmConnectionParams params )
 *
 * Description:
 *   Creates a VRML 2.0 Inline structure, with the url field filled out
 *   to point to the (tx,ty) vtile/geotile in (TerraVision) level, level.
 *   If the params structure has the TSM_PARAMS_GEOTILES value set,
 *   then the inline will link to a geotile directory, otherwise it
 *   will link to a vtile directory.
 *
 * Parameters:
 *   tspec  - the Pyramid tspec structure for the OI dataset
 *   tx, ty - the vtile (x,y) coordinates to inline
 *   level  - the (TerraVision) level number for the vtile to inline
 *   tree   - create an inlined tree or vtile?
 *
 * Returns: An allocated Inline structure, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

static SFNode
tsmCreateInline( vsgWorld *wrl, TsmConnection oiConn, 
		 TsmConnection demConn, int tx, int ty, int level, int tree,
		 TsmConnectionParams params )
{
  char     vtile_url[512];
  Pyramid  *oi_tspec = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  SFVec3f  bboxCenter;
  SFVec3f  bboxSize;  
  SFNode   inl = vsgNewInline(wrl);
  MFString *urns = vsgStartMFString(wrl);

  /* Create the URL for the vtile/geotile that is to be inlined */
  
  if (tree)
    tsmMakeTreeRedirectFilename( oi_tspec, tx, ty, level, params, vtile_url );
  else
    tsmMakeGeotileRedirectFilename( oi_tspec, tx, ty, level, params, vtile_url );

  vsgAppendSFString( wrl, urns, vsgStrdup ( wrl, vtile_url ) );
  vsgEndMFString(wrl, urns);
  vsgSetInline_url(wrl, inl, urns);

  /* Calculate the bounding box for the inline */
  tsmSetVRMLBoundingBox( wrl, oiConn, demConn, tx, ty, level, params,
			 &bboxSize, &bboxCenter );
  vsgSetInline_bboxSize(wrl, inl, bboxSize);
  vsgSetInline_bboxCenter(wrl, inl, bboxCenter);
  
  return inl;
}

/*
 * Synopsis:
 *   SFNode tsmCreateAnchor( vsgWorld *wrl, TsmConnection oiConn, 
 *               TsmConnection demConn, int tx, int ty, int level,
 *               TsmConnectionParams params )
 *
 * Description:
 *   Creates a VRML 2.0 Anchor structure, with the url field filled out
 *   to link to the (tx,ty) tree in (TerraVision) level, level. The
 *   children field of the anchor is filled out to contain an Inline to
 *   the vtile for the (tx,ty,level) tile, i.e. this vtile is displayed
 *   and if you click on it you go to the relevant tree file.
 *
 *   If the level of the tile is the highest resolution level of the 
 *   pyramid, then the function will return an Inline rather than an
 *   Anchor node because there is nothing worth linking to.
 *
 * Parameters:
 *   tspec  - the Pyramid tspec structure for the OI dataset
 *   tx, ty - the vtile (x,y) coordinates to inline (also the tree to link)
 *   level  - the (TerraVision) level number for the vtile to inline
 *
 * Returns: An allocated Anchor or Inline structure, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

static SFNode
tsmCreateAnchor( vsgWorld *wrl, TsmConnection oiConn, 
		 TsmConnection demConn, int tx, int ty, int level,
		 TsmConnectionParams params )
{
  const char *funcName = "tsmCreateAnchor";
  char       link_url[512], description[128];
  Pyramid    *tspec =  (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  int        vrml_level = tsmTVToVRMLLevel( tspec, level );
  SFVec3f    bboxCenter;
  SFVec3f    bboxSize;
  SFNode     anchor;
  SFNode     inl;
  MFString   *urns;
  MFNode     *children;

  /* If this is the highest resolution level, then there is nothing */
  /* worth linking to, so just output an Inline rather than Anchor  */

  if ( level <= tspec->minLevel )
    return tsmCreateInline( wrl, oiConn, demConn, tx, ty, level, FALSE, params );
  else if ( params->tree_type == TSM_TREE_TYPE_LOD )
    return tsmCreateInline( wrl, oiConn, demConn, tx, ty, level, TRUE, params );

  /* Otherwise, we're creating an anchor node... */

  anchor = vsgNewAnchor (wrl);

  /* Create the URL that is loaded when the anchor is selected */

  urns = vsgStartMFString(wrl);

  sprintf( link_url, "%s/trees/%d/p%dp%d.wrl", params->vrml_url, vrml_level,
	   tx, ty );
  vsgAppendSFString( wrl, urns, vsgStrdup( wrl, link_url ) );
  vsgEndMFString (wrl, urns);
  vsgSetAnchor_url(wrl, anchor, urns);

  /* Create a description field that may be displayed by the  */
  /* browser when the user moves the pointer over this anchor */

  sprintf( description, "Warp To Level %d (%d,%d)", vrml_level, tx, ty );
  vsgSetAnchor_description(wrl, anchor, vsgStrdup( wrl, description ) );

  /* An an inline for the vtile file to the children field of the anchor */

  inl = tsmCreateInline( wrl, oiConn, demConn, tx, ty, level, FALSE, params );
  if ( inl == NULL ) {
    tsmError( funcName, "Couldn't create inline for anchor" );
    return NULL;
  }

  children = vsgStartMFNode(wrl);
  vsgAppendSFNode( wrl, children, inl );
  vsgSetAnchor_children(wrl, anchor, vsgEndMFNode(wrl, children));

  tsmSetVRMLBoundingBox( wrl, oiConn, demConn, tx, ty, level, params,
			 &bboxSize, &bboxCenter );
  vsgSetAnchor_bboxCenter(wrl, anchor, bboxCenter);
  vsgSetAnchor_bboxSize(wrl, anchor, bboxSize);

  return anchor;
}

/*
 * Synopsis:
 *    SFNode tsmCreateLODStructure( vsgWorld *wrl, TsmConnection oiConn, 
 *                     TsmConnection demConn, int tx, int ty, int depth, 
 *                     int end_level, TsmConnectionParams params )
 *
 * Description:
 *    This is the routine that does all of the leg work for creating
 *    Anchor-LOD type tree files. It is a recursive function that
 *    generates an LOD structure for a range of levels of a pyramid,
 *    starting from a seed tile. 
 *
 * Parameters:
 *    tspec     - the tpsec structure for the OI pyramid
 *    tx, ty    - the tile (x,y) coordinates
 *    depth     - the (TerraVision) level number for the tree file
 *    end_level - the (TerraVision) level where the LOD hierarchy should stop
 *
 * Returns: An allocated LOD or Anchor structure, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

static SFNode
tsmCreateLODStructure( vsgWorld *wrl, TsmConnection oiConn, 
		       TsmConnection demConn, int tx, int ty, int depth, 
		       int end_level, TsmConnectionParams params )
{
  SFNode        new_node;        /* holds create LOD structure */
  SimpleTileSet *tileset;        /* pointer to *next* level's simple tileset */
  void          *inlineNode;     /* The inline for the lowest-res geom. */
  float         mag;             /* the scale factor between successive levs */
  int           x1, y1;          /* the lower-left (tx,ty) in next level */
  int           x2, y2;          /* the upper-right (tx,ty) in next level */
  int           x, y;            /* loop variables to create next level LODs */
  SFVec3f       bboxCenter;
  SFVec3f       bboxSize;
  Pyramid 	*tspec = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  double        ll[3], ur[3];    /* geometry bounding box */
  SFNode        lod, grp;
  MFNode        *nodes, *children;
  MFFloat       *dists;
  char          *protos = NULL;

  /* Check to see if we should break out of the recursion yet. If */
  /* this is the last level to add to the LOD hierarchy, then we  */
  /* dump out an anchor to the tree file that will continue the   */
  /* LOD hierarchy.                                               */

  if ( depth <= end_level )
    return tsmCreateAnchor( wrl, oiConn, demConn, tx, ty, depth, params );

  /* Allocate an LOD node to hold the entire tree structure for this */
  /* file, and a group node to contain all of the LODs in the first  */
  /* level of this node.                                             */
  /* Each LOD has two levels, the first one can recursively contain */
  /* further LOD hierarchies or Anchor nodes (contained in a Group  */
  /* node so that they are all the first node of the LOD level),    */
  /* while the second one is just a single inline for the lowest    */
  /* detail geometry.                                               */

  lod = vsgNewLOD(wrl);

  nodes = vsgStartMFNode(wrl); /* holds differnt LOD level nodes */

  grp = vsgNewGroup(wrl);

  /* We should allow GeoTiles to work with anchor tree files also.  */
  /* In which case, we need to include the appropriate EXTERNPROTOs */

  if ( params->geotiles ) {
    tsmAppendVRML( &protos, globalStateStr );
    vsgParseString( wrl, protos );
    tsmMemFree( protos );
    
    /* If a new VRML_URL has been specified, then change the default */
    /* URL for both the GlobalState and Cache definitions            */

    if ( tsmIsParamSet( params, TSM_PARAM_VRML_URL ) ) {
      vsgExternProtoClass *extdef;
      symbol_t            extdef_name;
      char                url[512];

      /* Change the GlobalState EXTERNPROTO default URL */

      extdef_name = vsgIntern( wrl, "GlobalState" );
      extdef = (vsgExternProtoClass *) vsgGetClass( wrl, extdef_name );
      if ( extdef != NULL ) {
	sprintf( url, "%s/protos/GeoTile/GlobalState.wrl", params->vrml_url);
	extdef->extern_urls->data[0] = vsgStrdup( wrl, url );
      }
    }
  }
  
  /* We have now added the two level nodes to the LOD structure, */
  /* but now we have to fill in all of the nodes in the Group    */
  /* node which makes up the first (highest) LOD level. This     */
  /* means that we have to find out all of the tiles in the next */
  /* level of the pyramid that are covered by this tile (tx,ty). */

  mag     = tspec->invResScaleFactor;
  tileset = &tspec->tspecLevel.val[depth-1];

  /* Tile (tx,ty) in level n will project onto a block of tiles */
  /* in level n-1 (using TerraVision level lingo). That block   */
  /* will be, e.g., a 2x2 block of tiles if mag = 2.0 (although */
  /* some clipping of this block size may be required at edges) */

  x1 = tx * mag;
  y1 = ty * mag;
  
  x2 = min( x1 + mag - 1, tileset->maxTile[0] );
  y2 = min( y1 + mag - 1, tileset->maxTile[1] );

  /* So now we have figured out the block of tiles that we want */
  /* the current tile to be replaced with in the next level, so */
  /* let's add them to the Group node in the first LOD level    */

  children = vsgStartMFNode(wrl);
  for ( y = y1; y <= y2; ++y ) {
    for ( x = x1; x <= x2; ++x ) {

      new_node = tsmCreateLODStructure( wrl, oiConn, demConn, x, y, depth-1,
					end_level, params );
      if ( new_node == NULL ) {
	vsgEndMFNode(wrl, children);
	vsgEndMFNode(wrl, nodes);
	return NULL;
      }

      vsgAppendSFNode( wrl, children, new_node );
      
    }
  }
  vsgSetGroup_children(wrl, grp, vsgEndMFNode(wrl, children));

  /* Work out the bounding box for the group of tiles we just generated, */

  tsmSetVRMLBoundingBox( wrl, oiConn, demConn, tx, ty, depth, params,
			 &bboxSize, &bboxCenter );
  vsgSetGroup_bboxCenter(wrl, grp, bboxCenter);
  vsgSetGroup_bboxSize(wrl, grp, bboxSize);

  vsgAppendSFNode( wrl, nodes, grp ); /* added to list of LOD levels */

  if ( depth < end_level + params->num_lods - 1 && params->tree_skip == 1 )
    inlineNode = tsmCreateAnchor( wrl, oiConn, demConn, tx, ty, depth, params);
  else
    inlineNode = tsmCreateInline( wrl, oiConn, demConn, tx, ty, depth, FALSE, params);

  if ( inlineNode == NULL ) return NULL;

  vsgAppendSFNode( wrl, nodes, inlineNode ); /* second set of LOD levels */
  vsgSetLOD_level( wrl, lod, vsgEndMFNode(wrl, nodes) );

  /* Work out the distance at which the LOD should switch, and  */
  /* the center of the tile group to be used by the LOD routine */
  dists = vsgStartMFFloat(wrl);

  tsmSetBoundingBox( wrl, oiConn, demConn, tx, ty, depth, params, ll, ur );
  vsgAppendSFFloat( wrl, dists, tsmCalcRange( tspec, tx, ty, depth, end_level,
					      params, ll, ur ) );

  vsgSetLOD_range(wrl, lod, vsgEndMFFloat(wrl, dists));
  vsgSetLOD_center(wrl, lod, vsgMakeSFVec3f( ( ur[0] - ll[0] ) / 2.0 + ll[0],
					     ( ur[1] - ll[1] ) / 2.0 + ll[1],
					     ( ur[2] - ll[2] ) / 2.0 + ll[2]));

  return lod;
}

/*
 * Synopsis:
 *    void tsmSetNodeMFString( vsgWorld *wrl, SFNode *node, char const
 *                             *field_name, char const *string )
 *
 * Description:
 *    This function simplifies the setting of a URL field for an
 *    existing node. You just need pass the world context, the SFNode
 *    instance, the field name (e.g. "parentUrl"), and the string that
 *    this field should be set to.
 *
 *    The string that you pass will be duplicated.
 *
 *    Limitation: note that and MFString can take multiple strings, but
 *    this function only lets you set one string.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 October 1998
 *
 */

static void
tsmSetNodeMFString( vsgWorld *wrl, SFNode node, char const *field_name,
		    char const *string )
{
  const char     *funcName = "tsmSetNodeMFString";
  vsgEventInDesc evin;
  MFString       *url;
  symbol_t       field_intern;

  if ( wrl == NULL || node == NULL || field_name == NULL || string == NULL ) {
    tsmError( funcName, "NULL value(s) passed" );
    return;
  }

  /* Make a new MFString instance for the string */

  if ( ( url = vsgMakeMFString( wrl ) ) == NULL ) {
    tsmError( funcName, "Couldn't make an MFString structure" );
    return;
  }
  vsgAppendSFString( wrl, url, vsgStrdup( wrl, string ) );

  /* Intern the field name to get its internal pointer */

  if ( ( field_intern = vsgIntern( wrl, field_name ) ) == NULL ) {
    tsmError( funcName, "Couldn't find field %s in world", field_name );
    return;
  }

  /* Get the field and then set the new MFString value */

  evin = vsgSetFunc( node, field_intern );
  vsgSetMFString( wrl, node, evin, url );
}

/*
 * Synopsis:
 *    void tsmSetNodeMFNode( vsgWorld *wrl, SFNode *node, char const
 *                           *field_name, MFNode *nodes )
 *
 * Description:
 *    This function simplifies the setting of an MFNode field for an
 *    existing node. You just need pass the world context, the SFNode
 *    instance, the field name (e.g. "parentNode"), and the MFNode that
 *    this field should be set to.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 December 1998
 *
 */

static void
tsmSetNodeMFNode( vsgWorld *wrl, SFNode node, char const *field_name,
		  MFNode *nodes )
{
  const char     *funcName = "tsmSetNodeMFNode";
  vsgEventInDesc evin;
  symbol_t       field_intern;

  if ( wrl == NULL || node == NULL || field_name == NULL || nodes == NULL ) {
    tsmError( funcName, "NULL value(s) passed" );
    return;
  }

  /* Intern the field name to get its internal pointer */

  if ( ( field_intern = vsgIntern( wrl, field_name ) ) == NULL ) {
    tsmError( funcName, "Couldn't find field %s in world", field_name );
    return;
  }

  /* Get the field and then set the new MFString value */

  evin = vsgSetFunc( node, field_intern );
  vsgSetMFNode( wrl, node, evin, nodes );
}

/*
 * Synopsis:
 *    void tsmSetNodeSFVec3f( vsgWorld *wrl, SFNode node, char const
 *                            *field_name, SFVec3f *value )
 *
 * Description:
 *    This function simplifies the setting of a 3-D vector field for an
 *    existing node. You just need pass the world context, the node
 *    instance, the field name (e.g. "child1Center"), and the SFVec3f value.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 October 1998
 *
 */

static void
tsmSetNodeSFVec3f( vsgWorld *wrl, SFNode node, char const *field_name,
		   SFVec3f *value )
{
  const char     *funcName = "tsmSetNodeSFVec3f";
  vsgEventInDesc evin;
  SFVec3f        vector;
  symbol_t       field_intern;

  if ( wrl == NULL || node == NULL || field_name == NULL || value == NULL ) {
    tsmError( funcName, "NULL value(s) passed" );
    return;
  }

  /* Make a new SFVec3f instance for the array */

  vector = vsgMakeSFVec3f( value->v[0], value->v[1], value->v[2] );

  /* Intern the field name to get an internal pointer */

  if ( ( field_intern = vsgIntern( wrl, field_name ) ) == NULL ) {
    tsmError( funcName, "Couldn't find field %s in scene", field_name );
    return;
  }

  /* Get the field and then set the new SFVec3f value */

  evin = vsgSetFunc( node, field_intern );
  vsgSetSFVec3f( wrl, node, evin, vector );
}

/*
 * Synopsis:
 *    void tsmSetNodeMFString( vsgWorld *wrl, SFNode *node, char const
 *                             *field_name, char const *string )
 *
 * Description:
 *    This function simplifies the adding of a URL to a list of URLs for
 *    an existing node. You just need pass the world context, the SFNode
 *    instance, the field name (e.g. "featureURL"), and the string that
 *    is to be appended to the URL list. 
 *
 *    The string that you pass will be duplicated. It will not be appended
 *    to the URL list if it already exists in that list.
 *
 * Returns: TRUE if the string was appended to the node's field
 *
 * Author: Martin Reddy
 *
 * Date: 27 October 1998
 *
 */

static int
tsmAppendNodeMFString( vsgWorld *wrl, SFNode node, char const *field_name,
		       char const *string )
{
  const char      *funcName = "tsmAppendNodeMFString";
  vsgEventOutDesc evout;
  MFString        *url, *url1;
  symbol_t        field_intern;
  int             i;

  if ( wrl == NULL || node == NULL || field_name == NULL || string == NULL ) {
    tsmError( funcName, "NULL value(s) passed" );
    return FALSE;
  }

  /* Intern the field name to get its internal pointer */

  if ( ( field_intern = vsgIntern( wrl, field_name ) ) == NULL ) {
    tsmError( funcName, "Couldn't find field %s in world", field_name );
    return FALSE;
  }

  /* Get the MFString field from the specified node */

  evout = vsgGetFunc( node, field_intern );

  if ( ( url = vsgGetMFString( wrl, node, evout ) ) == NULL ) {
    tsmError( funcName, "Couldn't find the MFString field %s", field_name );
    return FALSE;
  }
  
  /* If the string is empty, then just set rather than append */

  if ( url->data == NULL || url->mf.len == 0 ) {
    tsmSetNodeMFString( wrl, node, field_name, string );
    return TRUE;
  }

  /* Check to see if the string already exists in the field */
  /* If it does, then we don't do anything - return FALSE.  */

  for ( i = 0; i < url->mf.len; i++ )
    if ( ((unsigned int) url->data[i]) < 100 )
      tsmError( funcName, "Invalid URL data field!" );
    else if ( strcmp( string, url->data[i] ) == 0 )
      return FALSE;

  /* Add the string to the MFString and set the new field value */

  url1 = vsgAppendSFString( wrl, url, vsgStrdup( wrl, string ) );
  if ( url1 != url )
    vsgSet( wrl, node, field_intern, url1 );

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmIsRootTree( int vrml_level, TsmConnectionParams params )
 *
 * Description:
 *   Quickie to work out whether a tree file is to be considered a
 *   "root" tree file. A root tree file is one that is used as an
 *   entry point for viewing the hierarchy. For the old anchor-lod
 *   style tree files, every tree file can be an entry point, but
 *   for the QuadLOD trees, only files at a certain level are.
 *
 * Returns: TRUE if the tree file is a root tree.
 *
 * Author: Martin Reddy
 *
 * Date: 29 October 1998
 *
 */

static int
tsmIsRootTree( int vrml_level, TsmConnectionParams params )
{
  if ( params == NULL ) return TRUE;
  if ( params->tree_type != TSM_TREE_TYPE_QUADLOD ) return TRUE;
  return ( vrml_level == params->root_level );
}

/*
 * Synopsis:
 *   int tsmMakeGeoTile( vsgWorld *wrl )
 *
 * Description:
 *   This function will create a DEF'd instance of a GeoTile node. The
 *   GeoTile node is returned as the sole node inside of an MFNode. None
 *   of the GeoTile fields are filled in by this function.
 *
 * Returns: The MFNode structure containing the GeoTile node (NULL=fail)
 *
 * Author: Martin Reddy
 *
 * Date: 11 December 1998
 *
 */

static MFNode *
tsmMakeGeoTile( vsgWorld *wrl )
{
  const char *funcName = "tsmMakeGeoTile";
  vsgClass   *class;
  SFNode     geotile;
  MFNode     *wrapper;

  /* Create a GeoTile node instance */
  
  if ( ( class = vsgGetClass( wrl, vsgIntern( wrl, "GeoTile" ) ) ) == NULL ) {
    tsmError( funcName, "Couldn't find the GeoTile definition" );
    return NULL;
  }
    
  if ( ( geotile = vsgCreateNode( wrl, class ) ) == NULL ) {
    tsmError( funcName, "Couldn't create the GeoTile node!" );
    return NULL;
  }

  /* DEF the GeoTile node so that we can grab it later */
  
  vsgDEF( wrl, vsgIntern( wrl, "geoTile" ), geotile );

  /* Wrap up the GeoTile SFNode inside an MFNode structure */

  if ( ( wrapper = vsgMakeMFNode( wrl ) ) == NULL ) {
    tsmError( funcName, "Couldn't make an MFNode structure" );
    return NULL;
  }
  
  return vsgAppendSFNode( wrl, wrapper, geotile );
}

/*
 * Synopsis:
 *    MFNode *tsmCreateQuadLODStructure( vsgWorld *wrl, TsmConnection oiConn, 
 *                         TsmConnection demConn, int tx, int ty, int lev,
 *                         TsmConnectionParams params )
 *
 * Description:
 *    This function will create a QuadLOD instance for the specified
 *    (x, y, lev) tree file. It will also add the appropriate QuadLOD
 *    EXTERNPROTO to the vsgWorld structure, and GlobalState/Cache
 *    EXTERNPROTOs if this tree file is a "root" tree (entry point).
 *
 * Returns: An MFNode structure containing the QuadLOD node.
 *
 * Author: Martin Reddy
 *
 * Date: 27 October 1998
 *
 */

static MFNode *
tsmCreateQuadLODStructure( vsgWorld *wrl, TsmConnection oiConn, 
			   TsmConnection demConn, int tx, int ty, int lev,
			   TsmConnectionParams params )
{
  const char     *funcName = "tsmCreateQuadLODStructure";
  MFNode         *lod_nodes, *geotile;
  SFNode         node;
  vsgClass       *class;
  SFVec3f        bboxCenter;
  SFVec3f        bboxSize;
  float          range;
  double         ll[3], ur[3];
  Pyramid 	 *tspec = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  SimpleTileSet  *next = &tspec->tspecLevel.val[lev - 1];
  int            ix, iy, itx, ity, ichild;
  char           url[512];
  char           fieldname[32], *protos = NULL;

  /* Start of the MFNode structure to hold of the nodes we make here */

  if ( ( lod_nodes = vsgMakeMFNode( wrl ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate an MFNode!" );
    return NULL;
  }
  
  /* Parse the QuadLOD EXTERNPROTO and node instance  */
  /* If this is the root tree file, then we also need */
  /* to put in GlobalState and Cache EXTERNPROTOs too */
  
  if ( tsmIsRootTree( tsmTVToVRMLLevel( tspec, lev ), params ) ) {

    /* Make a string with the GlobalState, Cache, and QuadLOD */
    /* EXTERNPROTOs, and optionally the GeoTile EXTERNPROTO.  */

    tsmAppendVRML( &protos, cacheStr );
    tsmAppendVRML( &protos, quadLODStr );
    if ( params->geotiles ) {
      tsmAppendVRML( &protos, globalStateStr );
      tsmAppendVRML( &protos, geoTileStr );
    }

    if ( protos == NULL ) {
      tsmError( funcName, "Couldn't allocate a string for the EXTERNPROTOs" );
      return NULL;
    }

    vsgParseString( wrl, protos );
    tsmMemFree( protos );

    /* If a new VRML_URL has been specified, then change the default URL */
    /* for both the GlobalState and Cache definitions (QuadLOD below).   */

    if ( tsmIsParamSet( params, TSM_PARAM_VRML_URL ) ) {
      vsgExternProtoClass *extdef;
      symbol_t            extdef_name;

      /* Change the GlobalState EXTERNPROTO default URL */

      extdef_name = vsgIntern( wrl, "GlobalState" );
      extdef = (vsgExternProtoClass *) vsgGetClass( wrl, extdef_name );
      if ( extdef != NULL ) {
	sprintf( url, "%s/protos/GeoTile/GlobalState.wrl", params->vrml_url);
	extdef->extern_urls->data[0] = vsgStrdup( wrl, url );
      }

      /* Change the Cache EXTERNPROTO default URL */

      extdef_name = vsgIntern( wrl, "Cache" );
      extdef = (vsgExternProtoClass *) vsgGetClass( wrl, extdef_name );
      if ( extdef != NULL ) {
	sprintf( url, "%s/protos/QuadLOD/Cache.wrl", params->vrml_url );
	extdef->extern_urls->data[0] = vsgStrdup( wrl, url );
      }
    }
    
  } else {

    /* Just make a QuadLOD EXTERNPROTO, and optionally a GeoTile one */

    tsmAppendVRML( &protos, quadLODStr );
    if ( params->geotiles ) tsmAppendVRML( &protos, geoTileStr );

    if ( protos == NULL ) {
      tsmError( funcName, "Couldn't allocate a string for the EXTERNPROTOs" );
      return NULL;
    }

    vsgParseString( wrl, protos );
    tsmMemFree( protos );
  }

  /* If the user supplied a new VRML_URL, change the default */
  /* one for QuadLOD, and also for GeoTile if we've got one. */

  if ( tsmIsParamSet( params, TSM_PARAM_VRML_URL ) ) {
    vsgExternProtoClass *node;
    symbol_t            node_name;

    node_name = vsgIntern( wrl, "QuadLOD" );
    node = (vsgExternProtoClass *) vsgGetClass( wrl, node_name );
    if ( node != NULL ) {
      sprintf( url, "%s/protos/QuadLOD/QuadLOD.wrl#QuadLOD", params->vrml_url);
      node->extern_urls->data[0] = vsgStrdup( wrl, url );
    }

    if ( params->geotiles ) {
      node_name = vsgIntern( wrl, "GeoTile" );
      node = (vsgExternProtoClass *) vsgGetClass( wrl, node_name );
      if ( node != NULL ) {
	sprintf( url, "%s/protos/GeoTile/GeoTile.wrl", params->vrml_url );
	node->extern_urls->data[0] = vsgStrdup( wrl, url );
      }
    }
  }
  
  /* Now let's create an instance of the QuadLOD node. We have already */
  /* parsed the EXTERNPROTO definition so we know all about the new    */
  /* node and can use vsgGetClass to get the class definition.         */

  if ( ( class = vsgGetClass( wrl, vsgIntern( wrl, "QuadLOD" ) ) ) == NULL ) {
    tsmError( funcName, "Couldn't find the QuadLOD class I just made!" );
    return NULL;
  }
    
  if ( ( node = vsgCreateNode( wrl, class ) ) == NULL ) {
    tsmError( funcName, "Couldn't create the QuadLOD node!" );
    return NULL;
  }

  /* Set the URL and bounding box for the QuadLOD's parent tile */

  if ( params->geotiles ) {
    geotile = tsmMakeGeoTile( wrl );
    tsmSetNodeMFNode( wrl, node, "parentNode", geotile );
  } else {
    tsmMakeGeotileRedirectFilename( tspec, tx, ty, lev, params, url );
    tsmSetNodeMFString( wrl, node, "parentUrl", url );
  }

  /* Set the bounding box information for the parent tile */

  tsmSetVRMLBoundingBox( wrl, oiConn, demConn, tx, ty, lev, params, 
			 &bboxSize, &bboxCenter );
  tsmSetNodeSFVec3f( wrl, node, "parentCenter", &bboxCenter );
  tsmSetNodeSFVec3f( wrl, node, "parentSize", &bboxSize );

  /* Set the URL and bounding box for each of the child tiles */
  
  for( ix = 0, ichild = 1; ix < 2; ix++ ) {
    for ( iy = 0; iy < 2; iy++, ichild++ ) {
      itx = tx * 2 + ix;
      ity = ty * 2 + iy;
      if (itx >= next->minTile[0] && ity <= next->maxTile[1]) {

	/* Set the URL for this child tile */

	tsmMakeTreeRedirectFilename( tspec, itx, ity, lev - 1, params, url );
	sprintf( fieldname, "child%dUrl", ichild );
	tsmSetNodeMFString( wrl, node, fieldname, url );
	
	/* Set the bounding box for this child tile */
	
	tsmSetVRMLBoundingBox( wrl, oiConn, demConn, itx, ity, lev-1, 
			       params, &bboxSize, &bboxCenter );
	
	sprintf( fieldname, "child%dCenter", ichild );
	tsmSetNodeSFVec3f( wrl, node, fieldname, &bboxCenter );
	
	sprintf( fieldname, "child%dSize", ichild );
	tsmSetNodeSFVec3f( wrl, node, fieldname, &bboxSize );
	
      }
    }
  }
  
  /* Set the LOD switching volume for the QuadLOD node */
  
  tsmSetBoundingBox( wrl, oiConn, demConn, tx, ty, lev, params, ll, ur );
  range = tsmCalcRange( tspec, tx, ty, lev, 0, params, ll, ur );
  
  bboxSize.v[0] = bboxSize.v[1] = bboxSize.v[2] = range;
  
  tsmSetNodeSFVec3f( wrl, node, "switchSize", &bboxSize );

  /* Add the QuadLOD node to the MFNode to be returned */
  
  vsgAppendSFNode( wrl, lod_nodes, node );
  return lod_nodes;
}

/*
 * Synopsis:
 *    int tsmAppendViewpoint( vsgWorld *wrl, MFNode *nodes, Vector location,
 *                            Vector lookat, Vector up, char const *name,
 *                            TsmConnectionParams params )
 *
 * Description:
 *    Creates a single viewpoint node in the current VRML scene graph.
 *    You pass in where you want the viewpoint to be located in (x,y,z)
 *    and where you want to be looking at, and this function will work
 *    out the correct quaternion orientation.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 17 August 1998
 *
 */

static void
tsmAppendViewpoint( vsgWorld *wrl, MFNode *nodes, Vector location,
		    Vector lookat, Vector up, char const *name,
		    TsmConnectionParams params )
{
  Vector axis;
  double angle;
  SFNode view;

  /* transform from z-up to y-up coords if producing flat world */

  if ( params->flat != TSM_GEOCNTRC_VTILES ) { 
    TSM_MAKE_Y_UP( location.x, location.y, location.z );
    TSM_MAKE_Y_UP( lookat.x, lookat.y, lookat.z );
    TSM_MAKE_Y_UP( up.x, up.y, up.z );
  }

  /* work out the orientation quaterion */

  Convert_Camera_Model( &location, &lookat, &up, &axis, &angle );

  /* create the Viewpoint node */
  
  view = vsgNewViewpoint( wrl );
  vsgSetViewpoint_orientation( wrl, view,
     vsgMakeSFRotation(axis.x, axis.y, axis.z, angle ) );
  vsgSetViewpoint_position(wrl, view,
     vsgMakeSFVec3f(location.x, location.y, location.z ) );
  vsgSetViewpoint_description( wrl, view, vsgStrdup (wrl, name) );

  vsgAppendSFNode( wrl, nodes, view );
}

/*
 * Synopsis:
 *    int tsmAppendViewpoints( MFNode *root, MFNode *nodes, double *ll,
 *                             double *ur, TsmConnectionParams params )
 *
 * Description:
 *    Takes the bounding box for the tile and creates a number of
 *    viewpoints around the tile. Currently this includes the initial
 *    overhead view as well as a view above each of the four corners of
 *    the tile, looking back towards the center for the tile.
 *
 * Returns: TRUE if all went okay, FALSE if couldn't alloc structs
 *
 * Author: Martin Reddy
 *
 * Date: 28 January 1998
 *
 */

static int
tsmAppendViewpoints( vsgWorld *wrl, MFNode *nodes, double *ll, double *ur,
		     TsmConnectionParams params )
{
  Vector location, lookat, up;
  double offset[3];
  char   *desc0 = "Overhead View";
  char   *desc1 = "View From SW";
  char   *desc2 = "View From NW";
  char   *desc3 = "View From NE";
  char   *desc4 = "View From SE";

  /* create the initial "overhead" viewpoint */

  location.x = ll[0] + ( ur[0] - ll[0] ) / 2.0;
  location.y = ll[1] + ( ur[1] - ll[1] ) / 2.0;
  location.z = tsmSetInitDistance( ll, ur );

  lookat.x = location.x;
  lookat.y = location.y;
  lookat.z = -location.z;

  up.x = 0.0;  up.y = 1.0;  up.z = 0.0;

  tsmAppendViewpoint( wrl, nodes, location, lookat, up, desc0, params );

  /* Set up the look-at vector, the up vector, the viewing height,  */
  /* and a vector of offsets which are used to displace the viewpnt */
  /* from the corner of the bounding box by a small fraction.       */

  lookat.x = ll[0] + ( ur[0] - ll[0] ) / 2.0;
  lookat.y = ll[1] + ( ur[1] - ll[1] ) / 2.0;
  lookat.z = ll[2] + ( ur[2] - ll[2] ) / 2.0;

  up.x = 0.0;  up.y = 0.0;  up.z = 1.0;

  offset[0] = ( ur[0] - ll[0] ) / 10.0;
  offset[1] = offset[0] * ( ur[1] - ll[1] ) / ( ur[0] - ll[0] );
  offset[2] = offset[0];

  if ( ur[2] - ll[2] < offset[0] ) offset[2] *= 2.0;  /* up if real flat */

  /* Create a viewpoint for the SE corner */

  location.x = ll[0] - offset[0];
  location.y = ll[1] - offset[1];
  location.z = ur[2] + offset[2];
  tsmAppendViewpoint( wrl, nodes, location, lookat, up, desc1, params );

  /* Create a viewpoint for the NW corner */

  location.x = ll[0] - offset[0];
  location.y = ur[1] + offset[1];
  location.z = ur[2] + offset[2];
  tsmAppendViewpoint( wrl, nodes, location, lookat, up, desc2, params );

  /* Create a viewpoint for the NE corner */

  location.x = ur[0] + offset[0];
  location.y = ur[1] + offset[1];
  location.z = ur[2] + offset[2];
  tsmAppendViewpoint( wrl, nodes, location, lookat, up, desc3, params );

  /* Create a viewpoint for the SE corner */

  location.x = ur[0] + offset[0];
  location.y = ll[1] - offset[1];
  location.z = ur[2] + offset[2];
  tsmAppendViewpoint( wrl, nodes, location, lookat, up, desc4, params );

  return TRUE;
}

/*
 * Synopsis:
 *    SFNode tsmCreateTreeGraph( vsgWorld *wrl, TsmConnection oiConn,
 *                  TsmConnection demConn, int tx, int ty, int level,
 *                  MFNode *lod_nodes, TsmConnectionParams params )
 *
 * Description:
 *    This function will create the entire scene graph for a tree
 *    file. It will create a Group node which has various children
 *    added to it, such as a WorldInfo node, NavigationInfo, initial
 *    Viewpoint, etc. Also, you pass in the LOD hierarchy that has
 *    been generated and this will also be added to the Group node.
 *
 *    This is the structure that is output to the VRML tree file.
 *
 * Parameters:
 *    tspec    - the Pyramid tspec structure for the OI pyramid
 *    tx, ty   - the (x,y) coordinates for the tree file
 *    level    - the (TerraVision) level for the tree file
 *    lod_node - the LOD hierarchy for the tree file
 *
 * Returns: An allocated Group node, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

static SFNode
tsmCreateTreeGraph( vsgWorld *wrl, TsmConnection oiConn,
		    TsmConnection demConn, int tx, int ty, int level,
		    MFNode *lod_nodes, TsmConnectionParams params )
{
  char           buffer[512];
  double         ll[3], ur[3];
  SFFloat        speed;
  Pyramid 	 *tspec = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  int            i, vrml_level = tsmTVToVRMLLevel( tspec, level );
  int            do_flat = ( params->flat != TSM_GEOCNTRC_VTILES );
  SFNode         grp, collision, info, navinfo, light;
  SFNode         globalstate, cache;
  SFVec3f        direction;
  MFNode         *children;
  MFString       *strs;
  MFFloat        *avatar;
  vsgClass       *class;

  /* If we are doing flat tiles, and this is a root tree file, then */
  /* we want to rotate all of the imagery round to a Y-up frame.    */

  do_flat = ( params->flat != TSM_GEOCNTRC_VTILES &&
	      tsmIsRootTree( vrml_level, params ) );

  /* Allocate a Group node to contain the entire scene graph first */

  grp = vsgNewGroup(wrl);
  vsgDEF( wrl, vsgIntern( wrl, "root_node" ), grp );
  children = vsgStartMFNode(wrl);

  /* Do we want to add a Collision node to the scene graph? If so, */
  /* then we add this as a child of the Group node we allocated    */

  if ( params->collision == TRUE )
    collision = vsgNewCollision(wrl);

  tsmSetBoundingBox( wrl, oiConn, demConn, tx, ty, level, params, ll, ur );

  /* If this is a root tree file, then we add in a load of nodes  */
  /* like a WorldInfo, NavigationInfo, and a number of Viewpoints */

  if ( tsmIsRootTree( tsmTVToVRMLLevel( tspec, level ), params ) ) {

    /* If this is a root QuadLOD tree, then the first thing */
    /* that we add are GlobalState and Cache nodes.         */

    if ( params->geotiles ) {
  
      /* Add a "DEF GLOBAL_STATE GlobalState {}" instance */

      class = vsgGetClass( wrl, vsgIntern( wrl, "GlobalState" ) );
      if ( class != NULL ) {
	if ( ( globalstate = vsgCreateNode( wrl, class ) ) != NULL ) {
	  vsgDEF( wrl, vsgIntern( wrl, "GLOBAL_STATE" ), globalstate );
	  vsgAppendSFNode( wrl, children, globalstate );
	}
      }
    }

    if ( params->tree_type == TSM_TREE_TYPE_QUADLOD ) {

      /* Add a Cache node instance and set the default cacheSize */
    
      class = vsgGetClass( wrl, vsgIntern( wrl, "Cache" ) );
      if ( class != NULL ) {
	if ( ( cache = vsgCreateNode( wrl, class ) ) != NULL ) {
	  vsgEventInDesc evin;
	  evin = vsgSetFunc( cache, vsgIntern( wrl, "cacheSize" ) );
	  vsgSetSFInt32( wrl, cache, evin, 64 );
	  vsgAppendSFNode( wrl, children, cache );
	}
      }

    }

    /* Create a WorldInfo node to give some info about this tree file */
    info = vsgNewWorldInfo(wrl);

    sprintf( buffer, "TerraVision II: %s", tspec->identityTspec.name );
    if ( vrml_level > 0 )
      sprintf( buffer + strlen(buffer), " (Level %d)", vrml_level );
    vsgSetWorldInfo_title(wrl, info, vsgStrdup(wrl, buffer));
    
    strs = vsgStartMFString(wrl);
    sprintf( buffer, "TerraVision II tree file V%s", TREE_FILE_VERSION );
    vsgAppendSFString(wrl, strs, vsgStrdup(wrl, buffer));

    sprintf( buffer, "Index: %d %d", tx, ty );
    vsgAppendSFString(wrl, strs, vsgStrdup(wrl, buffer));

    sprintf( buffer, "Level: %d", vrml_level );
    vsgAppendSFString(wrl, strs, vsgStrdup(wrl, buffer));

    sprintf( buffer, "LODs: %d", params->num_lods );
    vsgAppendSFString(wrl, strs, vsgStrdup(wrl, buffer));

    sprintf( buffer, "PixelsPerTexel: %1.4g", params->texel_scale );
    vsgAppendSFString(wrl, strs, vsgStrdup(wrl, buffer));

    vsgSetWorldInfo_info(wrl, info, vsgEndMFString(wrl, strs));

    vsgAppendSFNode(wrl, children, info); /* for groups children */

    /* Create a NavigationInfo node so we can use the Examiner interface */

    navinfo = vsgNewNavigationInfo(wrl);
    strs = vsgStartMFString(wrl);
    vsgAppendSFString( wrl, strs, "EXAMINE" );
    vsgAppendSFString( wrl, strs, "ANY" );
    vsgSetNavigationInfo_type(wrl, navinfo, vsgEndMFString(wrl, strs));
    
    speed = max( ur[0] - ll[0], ur[1] - ll[1] );
    vsgSetNavigationInfo_speed(wrl, navinfo, speed );

    avatar = vsgStartMFFloat(wrl);
    /* distance between user and collision geom. Half = near clipping plane */
    vsgAppendSFFloat(wrl, avatar, tsmSetNearPlane( ll, ur ) * 2.0 );
    /* maintained height about terrain */
    vsgAppendSFFloat(wrl, avatar, speed / 10.0);
    /* tallest object that can be stepped over */
    vsgAppendSFFloat(wrl, avatar, speed / 20.0);
    vsgSetNavigationInfo_avatarSize(wrl, navinfo, vsgEndMFFloat(wrl, avatar));
    vsgSetNavigationInfo_headlight(wrl, navinfo, FALSE);

    vsgAppendSFNode(wrl, children, navinfo);

    /* Create a directional light source that shines down on the terrain */

    light = vsgNewDirectionalLight( wrl );
    direction = vsgMakeSFVec3f( 0.0, -0.707, -0.707 );
    vsgSetDirectionalLight_direction( wrl, light, direction );

    vsgAppendSFNode( wrl, children, light );

    /* Add the list of viewpoints for this tile */

    tsmAppendViewpoints( wrl, children, ll, ur, params );

  }

  /* Now add all these nodes to the Group or Collision node we */
  /* created and include the LOD hierarchy that we generated.  */
  /* If we are creating a flat world then we transform all of  */
  /* the coordinates from a z-up coordinate sys to VRML's y-up */

  if ( do_flat && !params->tree_type == TSM_TREE_TYPE_LOD) {
    SFNode tfm = vsgNewTransform(wrl);
    MFNode *tfmChildren = vsgStartMFNode(wrl);

    for ( i = 0; lod_nodes && i < lod_nodes->mf.len; i++ )
      vsgAppendSFNode( wrl, tfmChildren, lod_nodes->data[i] );

    vsgSetTransform_children(wrl, tfm, vsgEndMFNode(wrl, tfmChildren));
    vsgSetTransform_rotation(wrl, tfm, 
       vsgMakeSFRotation( 1.0, 0.0, 0.0, -1.5707963 ) );
    vsgSetTransform_bboxSize(wrl, tfm, tsmBBoxSize(ll, ur ));
    vsgSetTransform_bboxCenter(wrl, tfm, tsmBBoxCenter(ll, ur ));
    vsgAppendSFNode( wrl, children, tfm );
  } else {

    for ( i = 0; lod_nodes && i < lod_nodes->mf.len; i++ )
      vsgAppendSFNode( wrl, children, lod_nodes->data[i] );

  }

  /* Do we want to add a Collision node to the scene graph? If so, */
  /* then we add this as a child of the Group node we allocated    */

  if ( params->collision ) {
    vsgSetCollision_children(wrl, collision, vsgEndMFNode(wrl, children));
    vsgSetCollision_collide(wrl, collision, TRUE);

    vsgSetCollision_bboxSize(wrl, collision, tsmBBoxSize(ll, ur ));
    vsgSetCollision_bboxCenter(wrl, collision, tsmBBoxCenter(ll, ur ));

    children = vsgStartMFNode (wrl);
    vsgAppendSFNode( wrl, children, collision);
  }

  vsgSetGroup_children(wrl, grp, vsgEndMFNode(wrl, children));

  if ( ! do_flat || params->tree_type == TSM_TREE_TYPE_LOD) {
    /* Don't add this if we are doing a flat model because    */
    /* these values will be outside of the z-up -> y-up xform */
    /* and besides, we add these to the Transform node anyway */

    vsgSetGroup_bboxSize(wrl, grp, tsmBBoxSize(ll, ur ));
    vsgSetGroup_bboxCenter(wrl, grp, tsmBBoxCenter(ll, ur ));
  }

  return grp;
}

/*
 * Synopsis:
 *    void tsmWriteIcons( vsgWorld *wrl, TsmConnection oiConn, TsmConnection
 *                        demConn, int tx, int ty, int level, FILE *handle,
 *                        TsmConnectionParams params )
 *
 * Description:
 *    Outputs the EXTERNPROTO declaration and calls in order to adorn the
 *    display with movement arrows. An arrow to move in a particular 
 *    direction is only added if there is a tile in that direction.
 *
 *    This is only done for TSM_TREE_TYPE_ANCHOR type tree files as this
 *    is not necessary for QuadLOD trees.
 *
 *    Currently this is done with fprintf's - we should probably update
 *    this to use the VRML parser. It would be nice if the parser had a
 *    way to parse a string and append it to the world though, as if
 *    we use vsgParseString it will overwrite our current scene.
 *
 * Parameters: 
 *    tspec  - the OI pyramid's tspec structure
 *    tx, ty - the (x,y) coordinates for the tree tile
 *    level  - the (TerraVision) level for the tree file
 *    handle - the stream to write the EXTERNPROTO stuff out to
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 16 January 1998
 *
 */

static void
tsmWriteIcons( vsgWorld *wrl, TsmConnection oiConn, TsmConnection demConn,
	       int tx, int ty, int level, FILE *handle,
	       TsmConnectionParams params )
{
  SimpleTileSet *tileset;
  int           vrml_lev, mag;
  int           show_up, show_down, show_left, show_right, show_zoom;
  double        ll[3], ur[3], x1, y1, x2, y2, xm, ym;
  double        z;
  double        scale, spc;
  Pyramid       *tspec;

  /* For the anchor-lod tree files we drape a number   */
  /* of icons (e.g. arrows) around the tile to allow   */
  /* inter-tile navigation when zoomed in a few levels */

  if ( params->tree_type != TSM_TREE_TYPE_ANCHOR ) return;

  tspec    = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );
  tileset  = &tspec->tspecLevel.val[level];
  vrml_lev = tsmTVToVRMLLevel( tspec, level );

  /* If the tile is the top one with no adjacent tiles, then exit now */

  if ( tileset->numTile[0] == 1 && tileset->numTile[1] == 1 && vrml_lev == 0 ) 
    return;

  /* pre-calculate which of the arrow/zoom icons we will display */

  show_up    = ( ty < tileset->maxTile[1] );
  show_down  = ( ty > tileset->minTile[1] );
  show_left  = ( tx > tileset->minTile[0] );
  show_right = ( tx < tileset->maxTile[0] );
  show_zoom  = ( level < tspec->maxLevel );

  /* Otherwise we're going to have to create an EXTERNPROTO node. We */
  /* do this by just dumping the node and the calls to it directly   */
  /* out to a stream. Setting up VRML structures to do this would be */
  /* overly verbose and there is no easy way to call a proto either. */
  
  fprintf( handle, "\nEXTERNPROTO DrawArrows [\n" );

  if ( show_up == TRUE ) {
    fprintf( handle, "   field MFString up_url\n" );
    fprintf( handle, "   field SFVec3f  up_trans\n" );
    fprintf( handle, "   field SFVec3f  up_scale\n" );
  } else
    fprintf( handle, "   field SFInt32  up_hide\n" );

  if ( show_down == TRUE ) {
    fprintf( handle, "   field MFString down_url\n" );
    fprintf( handle, "   field SFVec3f  down_trans\n" );
    fprintf( handle, "   field SFVec3f  down_scale\n" );
  } else
    fprintf( handle, "   field SFInt32  down_hide\n" );

  if ( show_left == TRUE ) {
    fprintf( handle, "   field MFString left_url\n" );
    fprintf( handle, "   field SFVec3f  left_trans\n" );
    fprintf( handle, "   field SFVec3f  left_scale\n" );
  } else
    fprintf( handle, "   field SFInt32  left_hide\n" );

  if ( show_right == TRUE ) {
    fprintf( handle, "   field MFString right_url\n" );
    fprintf( handle, "   field SFVec3f  right_trans\n" );
    fprintf( handle, "   field SFVec3f  right_scale\n" );
  } else
    fprintf( handle, "   field SFInt32  right_hide\n" );

  if ( show_zoom == TRUE ) {
    fprintf( handle, "   field MFString out_url\n" );
    fprintf( handle, "   field SFVec3f  out_trans\n" );
    fprintf( handle, "   field SFVec3f  out_scale\n" );
  } else
    fprintf( handle, "   field SFInt32  out_hide\n" );

  fprintf( handle, "] [ \"../../protos/Arrows/arrows.wrl#DrawArrows\"\n" );
  fprintf( handle, " \"http://www.ai.sri.com/VRMLSets/v1.0/protos/Arrows/arrows.wrl#DrawArrows\"\n" );
  fprintf( handle, "]\n" );

  /* For flat worlds, we apply a -pi/2 rotation around the x-axis */
  /* in order to transform from z-up to VRML's y-up coord system. */

  if ( params->flat != TSM_GEOCNTRC_VTILES )
    fprintf( handle, "\nTransform { rotation 1 0 0 -1.5708 children [" );

  fprintf( handle, "\nDrawArrows {\n" );

  /* Calculate the lower-left (x1,y1), the upper-right (x2,y2) */
  /* and the mid-point (xm,ym) for the current tile.           */

  tsmSetBoundingBox( wrl, oiConn, demConn, tx, ty, level, params, ll, ur );

  /* Take some convenience variables for the lower-left (x1,y1) */
  /* and upper-right (x2,y2) coordinates of the tile.           */

  x1 = ll[0];  x2 = ur[0];
  y1 = ll[1];  y2 = ur[1];

  /* From the bounding box coordinates, work out the center (x,y) */

  xm = x1 + ( x2 - x1 ) / 2.0;
  ym = y1 + ( y2 - y1 ) / 2.0;

  /* calculate how much to scale the icons for this tile size, */
  /* and also how much space between icons and the terrain.    */

  scale = max( x2 - x1, y2 - y1 );
  spc   = scale * 0.05;

  /* Now calculate the average height of the terrain in the tile */
  /* The center of the icons are positioned at this height.      */

  z = ll[2] + ( ur[2] - ll[2] ) / 2.0;
  
  /* Output the fields to create or hide the UP arrow */

  if ( show_up == TRUE ) {
    fprintf( handle, "  up_url \"p%dp%d.wrl\"\n", tx, ty+1 );
    fprintf( handle, "  up_trans %g %g %g\n", xm, y2 + spc, z );
    fprintf( handle, "  up_scale %g %g %g\n", scale, scale, scale );
  } else
    fprintf( handle, "  up_hide -1\n" );

  /* Output the fields to create or hide the DOWN arrow */

  if ( show_down == TRUE ) {
    fprintf( handle, "  down_url \"p%dp%d.wrl\"\n", tx, ty-1 );
    fprintf( handle, "  down_trans %g %g %g\n", xm, y1 - spc, z );
    fprintf( handle, "  down_scale %g %g %g\n", scale, scale, scale );
  } else
    fprintf( handle, "  down_hide -1\n" );

  /* Output the fields to create or hide the LEFT arrow */

  if ( show_left == TRUE ) {
    fprintf( handle, "  left_url \"p%dp%d.wrl\"\n", tx-1, ty );
    fprintf( handle, "  left_trans %g %g %g\n", x1 - spc, ym, z );
    fprintf( handle, "  left_scale %g %g %g\n", scale, scale, scale );
  } else
    fprintf( handle, "  left_hide -1\n" );

  /* Output the fields to create or hide the RIGHT arrow */

  if ( show_right == TRUE ) {
    fprintf( handle, "  right_url \"p%dp%d.wrl\"\n", tx+1, ty );
    fprintf( handle, "  right_trans %g %g %g\n", x2 + spc, ym, z );
    fprintf( handle, "  right_scale %g %g %g\n", scale, scale, scale );
  } else
    fprintf( handle, "  right_hide -1\n" );

  /* Output the fields to create or hide the ZOOM OUT button */

  if ( show_zoom == TRUE ) {
    mag = tspec->invResScaleFactor * params->tree_skip;
    fprintf( handle, "  out_url \"../%d/p%dp%d.wrl\"\n",
	     vrml_lev - params->tree_skip, tx / mag, ty / mag );
    fprintf( handle, "  out_trans %g %g %g\n", x1 + (x2-x1) * 0.7, y2+spc, z );
    fprintf( handle, "  out_scale %g %g %g\n", scale, scale, scale );
  } else
    fprintf( handle, "  out_hide -1\n" );

  /* Finally, output the closing brace for the DrawArrows call. */
  /* And close the Transform node for flat worlds if created.   */

  fprintf( handle, "}\n" );
  if ( params->flat != TSM_GEOCNTRC_VTILES )
    fprintf( handle, "]}\n" );
}

/*
 * Synopsis:
 *   void tsmWriteVRMLIndex( TsmConnectionParams params )
 *
 * Description:
 *   Creates an index.html file in the vrml_dir directory. This HTML
 *   file performs an automatic redirection to the root tree file.
 *   For example, the http://www.foo.bar/foo/index.html file might
 *   redirect the web browser to www.foo.bar/foo/trees/2/p0p0.wrl.
 *   The level number of the root tree is determined by the value
 *   of params->root_level (set via command line with -rootlevel)
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 October 1998
 *
 */

static void
tsmWriteVRMLIndex( TsmConnectionParams params )
{
  const char    *funcName = "tsmWriteVRMLIndex";
  char          filename[512];
  FILE          *handle;

  if ( params == NULL ) return;

  /* Create the index.html file */

  sprintf( filename, "%s/index.html", params->vrml_dir );
  tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );

  if ( ( handle = fopen( filename, "w" ) ) == NULL ) {
    tsmError( funcName, "Couldn't create %s", filename );
    return;
  }

  /* Output the HTML code that redirects to the root tree file */

  fprintf( handle, 
"<HTML>\n\
<HEAD>\n\
<META HTTP-EQUIV=\"refresh\" CONTENT=\"0; URL=trees/%d/p0p0.wrl\">\n\
<TITLE>Redirect Page</TITLE>\n\
</HEAD>\n\
<BODY BGCOLOR=\"#000000\" TEXT=\"#AAAAAA\">\n\
<BASEFONT SIZE=\"3\">\n\
<H1>VRML Dataset</H1>\n\
This page will automatically redirect you to the root VRML\n\
scene of the terrain dataset. If your browser does not\n\
support redirection, then you can\n\
<A HREF=\"trees/%d/p0p0.wrl\">Click Here</A> to go directly\n\
to the VRML scene.\n\
</BODY>\n\
</HTML>\n", 
  params->root_level, params->root_level );

  fclose( handle );
}

/*
 * Synopsis:
 *   int tsmWriteAllTrees( TsmConnection oiConn, TsmConnection demConn, 
 *                         TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Creates a pyramid VRML tree and geotile files.
 *
 *   Create a hierarchy of VRML tree files. Tree files are used to control
 *   the level of detail (LOD) structure of a multi-resolution dataset.
 *   They are the glue that hold the vtiles together.
 *
 *   This function controls the generation of an entire hierarchy of
 *   VRML tree files from a TerraVision pyramid. It will open a
 *   connection to the dataset to get the tspec metadata, then it will
 *   create all of the relevant tree files and directories for that
 *   dataset.
 *
 *   The function will also generate an index.html redirect file that
 *   will send a web browser to the root tree file of the dataset.
 *
 *   The TsmConnectionParams structure lets you specify a number of
 *   parameters to control this process. Currently, the following 
 *   settings are supported:

 *   \begin{itemize}
 *   \item TSM_PARAMS_VRML_DIR: 
 *   The tree file will be written to the vtile directory inside this
 *   localdisk directory. This parameter is required - you should always
 *   set this with before calling this function.
 *   \item TSM_PARAMS_VRML_URL:
 *   By default, all of the URLs that are inserted into the VRML files
 *   will be relative. However, you can specify an absolute URL for all
 *   trees to use (i.e. for the vtile URL) by setting this parameter.
 *   The URL should be for the root directory of the dataset, e.g.
 *   http://www.ai.sri.com/VRMLSets/Yosemite.
 *   \item TSM_PARAMS_TREE_TYPE:
 *   If this is set to TSM_TREE_TYPE_QUADLOD, then the function will generate
 *   tree files using the custom Java-implemented QuadLOD node.
 *   If this value is set to
 *   TSM_TREE_TYPE_ANCHOR, then the function will generate tree files
 *   containing a sub-hierarchy of the terrain using standard VRML LOD
 *   and Anchor nodes.
 *   TSM_TREE_TYPE_LOD, then the function will generate tree files
 *   containing a sub-hierarchy of the terrain using standard VRML LOD
 *   node with NO Anchor nodes, just inlines.
 *   \item TSM_PARAMS_TEXEL_SCALE:
 *   This setting influences the level of detail (LOD) switching range for the
 *   multi-resolution terrain. This value is essentially an approximate
 *   screen-space error value. The default of 1 means switch LOD when the
 *   screen error between texels in adjacent LODs for a tile is >= 1.
 *   Therefore, using values
 *   greater than 1 will effectively make the LOD switching happen
 *   closer to the viewpoint.
 *   \item TSM_PARAMS_GEOTILES:
 *   When this setting is defined, for every tree file we generate a geotile
 *   file. The geotile file is inserted between the tree and vtile files
 *   and enables the specification of multiple terrain imageries as well
 *   as arbitary cultural features such as buildings, roads, etc.
 *   \item TSM_PARAMS_ROOT_LEVEL:
 *   When generating QuadLOD trees (see TSM_PARAMS_TREE_TYPE), there is
 *   a particular entry tree file which should be the file that you
 *   initially direct the web browser to. By default this will be the
 *   tree/2/p0p0.wrl file. However, if you set this value to 3, then then
 *   the root tree file becomes tree/3/p0p0.wrl. Additional nodes are 
 *   inserted into the QuadLOD root file, and also the index.html
 *   file will be generated to point to this root file.
 *   \item TSM_PARAMS_NUM_POLYS:
 *   This setting specifies the number of polygons on one side of a tile.
 *   This value should be the same as the value you used to create the
 *   vtiles that this tree will be pointing to. It is used to correctly
 *   calculate the bounding box for the vtile file.
 *   \item TSM_PARAMS_GAUSSIAN:
 *   Use gaussian interpolation instead of linear when interpolating
 *   the elevation data.
 *   This value should be the same as the value you used to create the
 *   vtiles that this tree will be pointing to. It is used to correctly
 *   calculate the bounding box for the vtile file.
 *   \item TSM_PARAMS_FLAT:
 *   By default, all coordinates will be projected into geocentric coordinates
 *   i.e. an (x,y,z) offset from the center of the earth. This will take
 *   into account the earth's curvature. If you just want to produce a flat
 *   dataset where all elevations are relative to a plane, then use this
 *   option. N.B. in flat worlds, all coordinates are specified in a Z-up
 *   coordinate system with embedded Transform nodes to convert these into
 *   VRML's Y-up coordinate system.
 *   This value should be the same as the value you used to create the
 *   vtiles that this tree will be pointing to. It is used to correctly
 *   calculate the bounding box for the vtile file.
 *   \item TSM_PARAMS_FAST_BBOXES:
 *   By default, all bounding boxes will be calculated by taking the min 
 *   and max of all coordinates in a tile. This can be a time consuming
 *   process and so we offer this alternative method which produces an
 *   approximation to a tile's bounding box but works much faster.
 *   \item TSM_PARAMS_WORLD_SCALE:
 *   Every coordinate that is sent to the vtile file will be scaled (divided)
 *   by this value. A value of 1.0 means no scaling. This facility is
 *   useful because geocentric coordinates can be very large and some VRML
 *   browsers have problems with these large coordinates. Scaling the 
 *   coordinates down to the 10-100 m range gives good results.
 *   This value should be the same as the value you used to create the
 *   vtiles that this tree will be pointing to. It is used to correctly
 *   calculate the bounding box for the vtile file.
 *   \item TSM_PARAMS_ELEV_SCALE:
 *   Every elevation value will be multiplied by this value before projecting
 *   into the display coordinate system (e.g. geocentric). This allows you
 *   to introduce a vertical exaggeration into your models. A value of 1.0
 *   means no vertical exaggeration.
 *   This value should be the same as the value you used to create the
 *   vtiles that this tree will be pointing to. It is used to correctly
 *   calculate the bounding box for the vtile file.
 *   \end{itemize}
 *
 * @param    oiConn is a connection to a OI pyramid dataset.
 * @param    demConn is a connection to the co-rectified DEM pyramid dataset.
 * @param    params is a pointer to a TsmConnectionParams structure.
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  18 June 1998
 *
 * @see      tsmWriteAllImages, tsmWriteAllVtiles
 *
 * */

int
tsmWriteAllTrees( TsmConnection oiConn, TsmConnection demConn, 
		  TsmConnectionParams params )
{
  const char    *funcName = "tsmWriteAllTrees";
  Pyramid       *tspec;              /* tspec structure for the OI dataset */
  SimpleTileSet *tileset;            /* pointer to a level's simple tileset */
  FILE          *handle;             /* used to write out the tree files */
  char          filename[512];       /* filename used to write stuff to disk */
  int           lev, tx, ty;         /* loop variables */
  int           end_lev;             /* the last level in a LOD hierarchy */
  SFNode        lod;                 /* The LOD hierarchy for a tree file */
  SFNode        scene;               /* The entire scene graph for tree */
  MFNode        *lod_nodes;          /* All of the LOD related VRML nodes */
  vsgWorld      *wrl;                /* state context */
  Pyramid 	*oi_tspec = (Pyramid *)tsmGet(oiConn, TSM_DATASET_TSPEC);
  Pyramid 	*dem_tspec = (Pyramid *)tsmGet(demConn, TSM_DATASET_TSPEC);

  /* Check that we have been given a valid OI and DEM pyramid */

  if ( ( tspec = oi_tspec ) == NULL ) {
    tsmError( funcName, "No OI pyramid was specified" );
    return FALSE;
  }

  if ( dem_tspec == NULL ) {
    tsmError( funcName, "No DEM pyramid was specified" );
    return FALSE;
  }

  /* Let's check that we have a params structure - this is needed */

  if ( params == NULL ) {
    tsmError( funcName, "No connection params structure specified!" );
    return FALSE;
  }

  /* Set start and end levels to the full pyramid if none are specified */

  if ( params->start_level == -1 )
    params->start_level = oi_tspec->maxLevel;

  if ( params->end_level == -1 )
    params->end_level = oi_tspec->minLevel;

  /* Display a warning message if the start/end levels are not valid */
  /* and then make them valid. Also assert that start < end level.   */
  
  if ( params->start_level < oi_tspec->minLevel || 
       params->start_level > oi_tspec->maxLevel ) {
    tsmError( funcName, "Start level %d not in valid range (%d..%d)",
	      params->start_level, oi_tspec->minLevel, oi_tspec->maxLevel );
    params->start_level = min( max( params->start_level, oi_tspec->minLevel ),
			       oi_tspec->maxLevel );
  }

  if ( params->end_level < oi_tspec->minLevel || 
       params->end_level > oi_tspec->maxLevel ) {
    tsmError( funcName, "End level %d not in valid range (%d..%d)",
	      params->end_level, oi_tspec->minLevel, oi_tspec->maxLevel );
    params->end_level   = min( max( params->end_level, oi_tspec->minLevel ),
			       oi_tspec->maxLevel );
  }

  if ( params->start_level < params->end_level ) {
    tsmError( funcName, "Start level must be higher then end level!" );
    return FALSE;
  }

  /* Create an index.html file to redirect the web browser */
  /* to the root tree file of the hierarchy */

  tsmWriteVRMLIndex( params );

  /* output a little message to the user and also set the VRML */
  /* writer's format string for writing SFFloats so that we    */
  /* can cut out all of those trailing zeros.                  */

  fprintf( stderr, "Processing %s (%d levels):\n",oi_tspec->identityTspec.name,
	   params->start_level - params->end_level + 1 );

  tsmVRMLCacheInit();

  /*Set_SFFloat_Output_Format( FLOAT_FORMAT_STRING );*/

  vsgcSetup();
  wrl = vsgNewWorld (0);
  vsgSetFiletype(wrl, "#VRML V2.0 utf8");

  /* The outer loop progresses through each VRML level tree directory */
  /* This starts with the lowest resolution TV level & decrements by  */
  /* a value defined by tree_skip, i.e. if this value is 3, then tree */
  /* files are only created for every third level of vtiles           */

  for ( lev = params->start_level;
	lev > params->end_level;
	lev -= params->tree_skip ){

    /* Create the directory for this level of VRML tree files */

    tileset = &tspec->tspecLevel.val[lev];
    tsmMakeTreeFilename( tspec, 0, 0, lev, params, filename );
    tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );

    fprintf( stderr, "Creating level %d tree...\n",
	     tsmTVToVRMLLevel(tspec,lev) );

    /* Create tree files for all of the vtiles in this level */

    for ( ty = tileset->minTile[1]; ty <= tileset->maxTile[1]; ++ty ) {
      for ( tx = tileset->minTile[0]; tx <= tileset->maxTile[0]; ++tx ) {

	/* Build the entire LOD structure for this tree file */

	vsgStartWorld(wrl);

	/* Are we creating a QuadLOD tree file, or one of those */
	/* old multi-lod-with-anchor type tree files?           */

	if ( params->tree_type == TSM_TREE_TYPE_QUADLOD ) {

	  if ( ! ( lod_nodes = tsmCreateQuadLODStructure( wrl, oiConn, demConn,
							  tx, ty, lev,
							  params ) ) ) {
	    tsmError( funcName, "Couldn't make QuadLOD (%d,%d,%d)",tx,ty,lev );
	    tsmVRMLCacheFree();
	    return FALSE;
	  }

	} else {

	  end_lev = max( lev - params->num_lods + 1, tspec->minLevel );

	  if ( ! ( lod = tsmCreateLODStructure( wrl, oiConn, demConn, tx, ty,
						lev, end_lev, params ) ) ) {
	    tsmError( funcName, "Couldn't make LOD (%d,%d,%d)", tx, ty, lev );
	    tsmVRMLCacheFree();
	    return FALSE;
	  }

	  /* Now just wrap a few other nodes around this LOD hierarchy */
	  /* in order to complete the VRML definition (e.g. we add a   */
	  /* WorldInfo node, NavigationInfo node, etc.).               */
	  
	  lod_nodes = vsgMakeMFNode( wrl );
	  vsgAppendSFNode( wrl, lod_nodes, lod );
	}
	
	if ( ! ( scene = tsmCreateTreeGraph( wrl, oiConn, demConn, tx, ty,
					     lev, lod_nodes, params ) ) ) {
	  tsmError( funcName, "Couldn't make scene for (%d,%d)", tx, ty );
	  tsmVRMLCacheFree();
	  return FALSE;
	}

	vsgWorldAppend( wrl, scene );
	vsgEndWorld( wrl );

	/* Write out the final scene graph to the tree file */
	
	tsmMakeTreeFilename( tspec, tx, ty, lev, params, filename );

	if ( ( handle = fopen( filename, "w" ) ) == NULL ) {
	  tsmError( funcName, "Couldn't write tree file: %s", filename );
	  tsmVRMLCacheFree();
	  return FALSE;
	}
	
	vsgPrint( wrl, handle );
	tsmWriteIcons( wrl, oiConn, demConn, tx, ty, lev, handle, params );
	fclose( handle );

	/* Write out the geotile for this tree if we have been */
	/* asked to produce geotile files (the default)        */

	if ( params->geotiles ) {
	  const char *vtile_url, *vtile_name;

	  sprintf( filename, "%s/vtiles/%d/p%dp%d.wrl", params->vrml_url, 
		   tsmTVToVRMLLevel( oi_tspec, lev ), tx, ty );

	  vtile_url  = &filename[0];
	  vtile_name = oi_tspec->geoName;

	  tsmWriteGeotile( oiConn, demConn, tx, ty, lev, params,
			   &vtile_url, &vtile_name, 1, NULL, NULL, 0 );

	}

      }
    }
    
  }

  /* All done, so finish up */

  vsgDestroyWorld( wrl );
  tsmVRMLCacheFree();

  return TRUE;
}

/*
 * Synopsis:
 *    uchar *tsmConvertToRGBBuffer( TsmConnection connect, uchar *tileBuffer )
 *
 * Description:
 *    Takes a tile buffer for a particular connection. If this connection
 *    is for a dataset with 1-byte 1-component images (e.g. grayscale),
 *    then it will create a 1-byte 3-component version (e.g. RGB).
 *
 * Returns: pointer to the (potentially) converted tile buffer
 *
 * Author: Martin Reddy
 *
 * Date: 6 February 1998
 *
 */

static uchar *
tsmConvertToRGBBuffer( TsmConnection connect, uchar *tileBuffer )
{
  Pyramid      *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  TileTspec    *tile = &tspec->tileTspec;
  static uchar newBuffer[128*128*3];
  int          ix, iy, in_pos, out_pos;

  if ( tile->numComponentsPixel == 1 && tile->numBytesPerComponent == 1 ) {
    for ( iy = 0; iy < 128; ++iy ) {
      for ( ix = 0; ix < 128; ++ix ) {
        in_pos = ( iy * 128 ) + ix;
        out_pos = ( iy * 128 * 3 ) + ( ix * 3 );
        newBuffer[out_pos]   = tileBuffer[in_pos];
        newBuffer[out_pos+1] = tileBuffer[in_pos];
        newBuffer[out_pos+2] = tileBuffer[in_pos];
      }
    }
    return newBuffer;
  } else
    return tileBuffer;
}

/*
 * Synopsis:
 *   int tsmWriteAllImages( TsmConnection oiConn, TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Create an image pyramid of JFIF/JPEG files.
 *
 *   This function can be used to convert an entire pyramid of tiles
 *   for a TerraVision ortho-image dataset into a pyramid of individual
 *   tiles in JFIF/JPEG format. These JPEG versions of the image tiles
 *   are then displayable by a standard VRML browser.
 *
 *   The params structure lets you specify a number of additional parameters
 *   to this function. The important parameters in params are:
 *
 *   \begin{itemize}
 *   \item TSM_PARAMS_VRML_DIR:
 *   The image files will be written to the image directory inside this
 *   localdisk directory. This parameter is required - you should always
 *   set this with before calling this function.
 *   \item TSM_PARAMS_START_LEVEL:
 *    This value specifies the lowest resolution (TerraVision style)
 *    level number at which the process should begin, e.g. if the
 *    TerraVision levels are 0..9, then a start level of 8 will begin
 *    at the second lowest resolution level. If you do not specify a
 *    value, then the lowest level will be assumed.
 *   \item TSM_PARAMS_END_LEVEL:
 *    This value specifies the highest resolution (TerraVision style)
 *    level number at which the process should stop, e.g. if the
 *    TerraVision levels are 0..9, then a start level of 1 will stop
 *    at the second highest resolution level. If you do not specify a
 *    value, then the highest level (probably 0) will be assumed.
 *   \end{itemize}
 *
 * @param    oiConn is a connection to a OI pyramid dataset.
 * @param    params is a pointer to a TsmConnectionParams structure.
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  18 June 1998
 *
 * @see      tsmWriteAllTrees, tsmWriteAllVtiles
 *
 */

int
tsmWriteAllImages( TsmConnection oiConn, TsmConnectionParams params )
{
  const char    *funcName = "tsmWriteAllImages";
  Pyramid       *pyramid, *out_pyramid;
  SimpleTileSet *tileset;
  unsigned char *tileBuffer;
  int           x, y, level, res, tiles = 0;
  char          message[100];
  int		start_level;
  int		end_level;
  TsmConnection out_connect;

  if ( oiConn == NULL || params == NULL ) {
    tsmError( funcName, "You must specify a connection and parameters" );
    return FALSE;
  }

  pyramid = (Pyramid *) tsmGet( oiConn, TSM_DATASET_TSPEC );

  if ( pyramid == NULL ) {
    tsmError( funcName, "no tspec information available for connection" );
    return FALSE;
  }

  /* Set up the start and end levels for the main loop */

  if ( params->start_level < 0 ) 
    start_level = pyramid->maxLevel;
  else
    start_level = params->start_level;

  if ( params->end_level < 0 )
    end_level = pyramid->minLevel;
  else
    end_level = params->end_level;
  
  /* Ensure that these are in range, and in the correct order */

  if ( start_level < pyramid->minLevel || start_level > pyramid->maxLevel ) {
    tsmError( funcName, "Start level %d not in valid range (%d..%d)",
	      start_level, pyramid->minLevel, pyramid->maxLevel );
    start_level = min( max( start_level, pyramid->minLevel ),
		       pyramid->maxLevel );
  }

  if ( end_level < pyramid->minLevel || end_level > pyramid->maxLevel ) {
    tsmError( funcName, "End level %d not in valid range (%d..%d)",
	      end_level, pyramid->minLevel, pyramid->maxLevel );
    end_level = min( max( end_level, pyramid->minLevel ), pyramid->maxLevel);
  }

  if ( start_level < end_level ) {
    tsmError( funcName, "start level must be higher then end level" );
    return FALSE;
  }

  /* make a dummy output connection to use for tsmSaveImage */
  /* do this in case we need to convert dataset image type */

  if ( ( out_connect = tsmConnect( "dummy", "w", NULL ) ) == NULL ) {
    tsmError( funcName, "Couldn't connect to dummy dataset" );
    return FALSE;
  }

  /* Copy the tspec structure from the input to the output connection */

  tsmSetTspecField( out_connect, pyramid );

  out_pyramid = (Pyramid *) tsmGet( out_connect, TSM_DATASET_TSPEC );
  if ( out_pyramid == NULL ) {
    tsmError( funcName, "Couldn't copy the tspec structure!" );
    return FALSE;
  }

  /* Set the image format to always be colour (RGB) */

  if ( out_pyramid->pixelTspec.componentNames.val != NULL )
    tsmMemFree( out_pyramid->pixelTspec.componentNames.val );

  out_pyramid->tileTspec.numComponentsPixel = 3;
  tsmArrayMalloc( tspSymbol, out_pyramid->pixelTspec.componentNames, 3 );
  out_pyramid->pixelTspec.componentNames.val[0] = tspRed;
  out_pyramid->pixelTspec.componentNames.val[1] = tspGreen;
  out_pyramid->pixelTspec.componentNames.val[2] = tspBlue;
  
  /* allocate a buffer for the tiles to be read into */

  if ( ( tileBuffer = tsmAllocTileBuffer( oiConn ) ) == NULL ) {
    tsmError( funcName, "couldn't allocate a tile buffer" );
    return FALSE;
  }

  /* now go through every tile in each level and read it in */

  for ( level = start_level; level >= end_level; level-- ) {

    tileset = &pyramid->tspecLevel.val[level];

    /* Make sure the directory exists that we're going to write to */

    tsmMakeImageFilename( pyramid, 0, 0, level, message, params );
    tsmMakeDirIfNotExist( message, TSM_PATH_AND_FILE );

    printf( "Generate TV level %d images...\n", level );
    
    for ( y = tileset->minTile[1]; y <= tileset->maxTile[1]; y++ ) {
      for ( x = tileset->minTile[0]; x <= tileset->maxTile[0]; x++ ) {
        
        res = tsmReadTile( oiConn, x, y, level, tileBuffer );

        if ( res == FALSE ) {
          tsmError( funcName, "reading tile (%d,%d), level %d", x, y, level );
          return FALSE;
        } else {
          tsmMakeImageFilename( pyramid, x, y, level, message, params );

          if ( tsmSaveImage( out_connect,
                             tsmConvertToRGBBuffer( oiConn, tileBuffer ),
                             message, TSM_JFIF ) )
            tiles++;
        }

      }
    }
  }

  printf( "%d JPEG tiles written\n", tiles );

  tsmFreeTileBuffer( tileBuffer );
  /* tsmFreeTspec( out_pyramid ); */
  tsmDisconnect( out_connect );

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteGeotile( TsmConnection oiConnect, TsmConnection demConnect, 
 *               int tx, int ty, int iLevel, TsmConnectionParams params,
 *               char const **terrainUrls, char const **terrainNames,
 *               int numTerrains, char const **featureUrls,
 *               char const **featureNames, int numFeatures )
 *
 * Doc++ Description: *//** 
 *   Creates a single VRML geotile file.
 *
 *   This function creates a single GeoTile file. A GeoTile file contains
 *   a list of alternative terrain URLs for a tile, as well as a list of
 *   cultural features (e.g. roads, buildings, etc.) that extend into
 *   the tile. These files are normally linked to from a tree file and
 *   they contain links to the appropriate vtile (or other) files.
 *
 *   This function will take a list of terrain URLs and feature URLs.
 *   It also takes a corresponding list of terrain names and feature names,
 *   i.e. terrainName[0] is the name for the dataset at terrainUrl[0].
 *   Of course, you should have the same number of names as you have URLs
 *   in each case.
 *
 *   The function will check to see if the specified GeoTile file already
 *   exists. If it does, then it will read in that GeoTile file and add
 *   any unique entries to the GeoTile instance. Otherwise, a new GeoTile
 *   file will be created with the specified terrain and feature information.
 *
 *   The following TsmConnectionParams settings are currently supported
 *   by this function:
 *
 *   \begin{itemize}
 *   \item TSM_PARAMS_VRML_DIR: 
 *   The geotile file will be written to the geotile directory inside this
 *   localdisk directory. This parameter is required - you should always
 *   set this with before calling this function.
 *   \item TSM_PARAMS_VRML_URL:
 *   By default, the EXTERNPROTO URL that are inserted into the geotile file
 *   will be relative. However, you can specify an absolute URL for all
 *   geotiles to use by setting this parameter.
 *   The URL should be for the root directory of the dataset, e.g.
 *   http://www.ai.sri.com/VRMLSets/Yosemite.
 *   \end{itemize}
 *
 * @param    oiConnect is a connection to a OI pyramid dataset.
 * @param    demConnect is a connection to the corresponding DEM pyramid 
 *		dataset.
 * @param    tx is the X position of the tile at this level
 * @param    ty is the Y position of the tile at this level
 * @param    level is the TerraVision level of the OI pyramid for this tile.
 * @param    params is a pointer to a TsmConnectionParams structure.
 * @param    terrainUrls an array of URL strings for the terrains
 * @param    terrainNames an array of identifier strings for each terrain URL
 * @param    numTerrains the number of entries in the terrain URL and
 *                       name arrays
 * @param    featureUrls an array of URL strings for the features
 * @param    featureNames an array of identifier strings for each feature URL
 * @param    numFeatures the number of entries in the feature URL and
 *                       name arrays
 *
 * @return   TRUE if all went just peachy, FALSE if it went pear shaped
 *
 * @author   Martin Reddy
 * @version  1 July 1998
 *
 * @see      tsmConnect, tsmSetParam, tsmWriteAllTrees, tsmWriteVtile
 *
 */

int
tsmWriteGeotile( TsmConnection oiConnect, TsmConnection demConnect, 
		 int tx, int ty, int iLevel, TsmConnectionParams params,
		 char const **terrainUrls, char const **terrainNames,
		 int numTerrains, char const **featureUrls,
		 char const **featureNames, int numFeatures )
{
  static const char funcName[] = "tsmWriteGeotile";
  Pyramid             *oiPyramid;
  vsgWorld            *wrl;
  int 		      i, filesize;
  char                filename[512], url[512], *buffer, *protos = NULL;
  FILE                *filep;
  PRFileInfo          stat_struct;
  vsgExternProtoClass *geoTileProto;
  SFNode              geotile;
  const char          *geoTileProto_name, *geoTile_name;

  /* Take some convenience pointers to commonly used structures */

  oiPyramid = (Pyramid *) tsmGet( oiConnect, TSM_DATASET_TSPEC );

  /* create directory and filename for the geotile */

  tsmMakeGeotileFilename( oiPyramid, tx, ty, iLevel, params, filename );
  tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );

  /* Do the vrml generator initialisation */

  vsgcSetup();
  wrl = vsgNewWorld ( 0 );

  /* test to see if the geotile file already exists */

  if ( PR_GetFileInfo ( filename, &stat_struct ) == PR_FAILURE ) {
    filesize = stat_struct.size;

    /* Read in the current GeoTile file and parse its contents */

    if ( ( filep = fopen( filename, "r" ) ) == NULL ) {
      tsmError( funcName, "Couldn't read existing geotile: %s", filename );
      return FALSE;
    }

    if ( ! ( buffer = (char *) tsmMemMalloc( filesize + 1 ) ) ) {
      tsmError( funcName, "Couldn't allocate %d bytes", filesize + 1 );
      return FALSE;
    }

    fread( buffer, 1, filesize, filep );
    buffer[filesize] = '\0';
    vsgParseString( wrl, buffer );
    tsmMemFree( buffer );

  } else {

    /* No current GeoTile file, so just use the default one */

    tsmAppendVRML( &protos, geoTileStr );
    tsmAppendVRML( &protos, geoTileInstStr );
    vsgParseString( wrl, protos );
    tsmMemFree( protos );

  }

  /* Intern the node names and update the default EXTERNPROTO URL */

  geoTileProto_name = vsgIntern( wrl, "GeoTile" );
  geoTile_name = vsgIntern( wrl, "geoTile" );
  geoTileProto = (vsgExternProtoClass *) vsgGetClass( wrl, geoTileProto_name );

  if ( geoTileProto == NULL ) {
    tsmError( funcName, "Corrupted GeoTile file, using default node" );
    vsgParseString( wrl, geoTileStr );

    geoTileProto_name = vsgIntern( wrl, "GeoTile" );
    geoTile_name = vsgIntern( wrl, "geoTile" );
    geoTileProto = (vsgExternProtoClass *) vsgGetClass(wrl, geoTileProto_name);
  }
  
  if ( geoTileProto != NULL ) {
    sprintf( url, "%s/protos/GeoTile/GeoTile.wrl", params->vrml_url );
    geoTileProto->extern_urls->data[0] = vsgStrdup( wrl, url );
  }

  /* Now get a hold of the GeoTile instance in this file */
  /* and add all of the new terrain/feature URLs/names.  */

  geotile = vsgUSE( wrl, geoTile_name );

  for ( i = 0; i < numTerrains; ++i )
    if ( tsmAppendNodeMFString( wrl, geotile, "terrainURL", terrainUrls[i] ) )
      tsmAppendNodeMFString( wrl, geotile, "terrainName", terrainNames[i] );
    
  for ( i = 0; i < numFeatures; ++i )
    if ( tsmAppendNodeMFString( wrl, geotile, "featureURL", featureUrls[i] ) )
      tsmAppendNodeMFString( wrl, geotile, "featureName", featureNames[i] );

  /* finish up the VRML file, write it out, and clean up */

  if ( ( filep = fopen( filename, "w" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write geotile file: %s", filename );
    return FALSE;
  }

  vsgEndWorld( wrl );
  vsgPrint( wrl, filep );
  vsgDestroyWorld ( wrl );
  fclose( filep );

  return TRUE;
}

/*
 * Synopsis:
 *   MFString *tsmGetMetadataNode( vsgWorld *world )
 *
 * Doc++ Description: *//** 
 *   Finds the info field of a METADATA node.
 *
 *   This function takes a VRML scene graph and searches for a node
 *   that has been DEF named METADATA, e.g. DEF METADATA Metdata {}.
 *   It then returns the MFString field of that node that is called
 *   "info".
 *
 *   If there is no node named METADATA, or if that node does not
 *   have an exposedField called info, then this function will
 *   return NULL.
 *
 * @param    world (vsgWorld *) - a valid VRML scene graph structure
 *
 * @return   A valid MFString node, or NULL if no METADATA node found
 *
 * @author   Martin Reddy
 * @version  11 April 1999
 *
 * @see      tsmParseMetadataInfo
 *
 */

MFString *
tsmGetMetadataNode( vsgWorld *world )
{
  char const      *funcName = "tsmGetMetadataNode()";
  char const      *name;
  SFNode          node;
  MFString        *mfstring;
  vsgEventOutDesc event_out;

  if ( world == NULL ) return NULL;

  /* Find the node that has been DEF'd as METADATA */

  name = vsgIntern( world, "METADATA" );
  if ( ( node =  vsgUSE( world, name ) ) == NULL ) {
    TSMTRACE(( TSMMASK|2, funcName, "No node called METADATA" ));
    return NULL;
  }

  /* Get the info field of that node */

  name = vsgIntern( world, "info" );
  event_out = vsgGetFunc( node, name );

  if ( event_out.f.MFString == NULL ) {
    TSMTRACE(( TSMMASK|2, funcName, "Event out %s not found in METADATA" ));
    return NULL;
  }

  if ( ( mfstring = vsgGetMFString( world, node, event_out ) ) == NULL ) {
    TSMTRACE(( TSMMASK|2, funcName, "Could not get the MFString %s", name ));
    return NULL;
  }

  /* return the valid MFString definition for the METADATA node info field */

  return mfstring;
}


/*
 * Synopsis:
 *   int tsmParseMetadataInfo( MFString *, char *token_match, char result[] )
 *
 * Doc++ Description: *//** 
 *   Extract a Metadata token value.
 *
 *   This function takes an MFString, as returned by tsmGetMetadataNode(),
 *   and then checks to see if the specified token is defined in this
 *   string array. If it is, then the value of that token is copied into
 *   the result buffer and a value of TRUE is returned. Otherwise, a
 *   value of FALSE is returned and the contents of result is undefined.
 *
 *   For example, if the mfstring array contains the string
 *   "fred: hello world", then calling this function with token_match
 *   equal to "fred" will produce the string "hello world" in result.
 *
 * @param    mfstring (MFString *) - the info field of a METADATA node
 * @param    token_match (char *) - the token string to search for
 * @param    result (char[]) - the buffer to copy the token value into
 *
 * @return   TRUE if the result buffer has been modified. FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  11 April 1999
 *
 * @see      tsmGetMetadataNode
 *
 */

int
tsmParseMetadataInfo( MFString *mfstring, char *token_match, char result[] )
{
  SFString token, value;
  int      i;

  if ( mfstring == NULL ) return FALSE;

  /* Now parse the MFString and extract the supported metadata tokens */

  for ( i = 0; i < mfstring->mf.len; i++ ) {

    /* extract the token string and the value string, separated by ':' */

    if ( mfstring->data[i] == NULL ) continue;
    if ( ( token = tsmMemStrdup( mfstring->data[i] ) ) == NULL ) continue;
    if ( ( value = strchr( token, ':' ) ) == NULL ) {
      tsmMemFree( token );
      continue;
    }

    /* null-terminate the token string, and move to start of value */

    *(value++) = '\0';
    while ( isspace(*value) ) value++;

    /* Now we can work out if this is a supported token and implement it */

    if ( strcmp( token, token_match ) == 0 ) {
      strcpy( result, value );
      tsmMemFree( token );
      return TRUE;
    }

    tsmMemFree( token );
  }  

  return FALSE;
}
