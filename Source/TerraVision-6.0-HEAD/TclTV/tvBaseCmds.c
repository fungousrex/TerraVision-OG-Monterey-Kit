/*
  tvBaseCmds.c
 

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
#include <stdarg.h>
#include <togl.h>

#include <TerraVision/Trace.h>
#include <TerraVision/ThreeDCore.h>
#include <TerraVision/OAA.h>
#include <TerraVision/agentlib.h>
#include <TerraVision/Features.h>
#include <glt/glt.h>
#include <glt/glmTcl.h>
#include <tvutils/tvutils.h>

#include "tsGeoPyramidCmds.h"
#include "tvBaseCmdsP.h"
#include "tvBaseCmds.h"


FILE* fp;  /***DEBUGGING ONLY****/

static Tcl_Obj* genericErrorMessage(int argc, char* argv[]);
static void tvlibMessageHandler(void*, tvMessageType, char const*, va_list);

TsmDatabase *getDBase( void );     /* from tsmDatabaseCmds.c */

/*
 * t v T o g l C r e a t e C B ( )
 *
 * Called when a new Togl widget is instantiated.  Here we create and 
 * initialize a new tvBase instance.
 * 
 */
void
tvToglCreateCB(struct Togl *togl)
{
    const char *funcName = "tvToglCreateCB";
    static int init = 1;

    glmContext glmCtx;
    glmWindow  glmw;

    tvBase* tvBaseInst;
    TvBasePacket* tvPacket;
    char* ident;

    if (init) {
        char *magicHome = tvuGetString( TVU_INSTALL_DIR );
	char suffix[] = "/etc/textures";
	char *buff = (char*)malloc(strlen(magicHome) + strlen(suffix) + 1);
	strcpy(buff, magicHome);
	strcat(buff, suffix);
	TextureSetPath(buff);
	free(buff);

        CreateTsLog();

        init = 0;
    }

    TSMTRACE(( INIT_MASK|1, funcName, "Entering" ));

    /* create a glmContext... */
    glmCtx = glmTclCreateContext(togl);

    /* create a glmWindow from our context... */
    glmw = glmTclCreateWindow(togl, glmCtx, True, False);

    /* now we can create a tvBase instance... */
    tvBaseInst = tvBaseCreate(NULL, "TerraVision II", glmw);

    /* tie the tvBase and glmw instances to the togl instance... */
    tvPacket = (TvBasePacket*)calloc(1, sizeof(TvBasePacket));
    tvPacket->base = tvBaseInst;
    tvPacket->glmw = glmw;
    tvPacket->mapped = FALSE;
    tvPacket->togl = togl;
    Togl_SetClientData(togl, (ClientData)tvPacket) ;

    /* set initial state for the new window */

    tvSetBoolean( tvBaseInst, TV_USE_ZBUFFER_HACK, TRUE );
 
    /* register a message handler */
    tvSetMessageHandler(tvBaseInst, tvlibMessageHandler, (void*)tvPacket);

   /* connect to .geo schema if requested */
    
    tvuInitCore( tvBaseInst );

    /* We'll also register an association between the script-provided 
     * ident-string and the tvPacket.  This is of use later when we need
     * to look up the tvBase given an "instID"; for example, in the Links
     * code.
     */
    ident = Togl_Ident(togl);
    if (ident) {
        Tcl_SetAssocData(Togl_Interp(togl), ident, NULL, (ClientData)tvPacket);
    }
}

/*
 * t v T o g l D e s t r o y C B ( )
 *
 */
void
tvToglDestroyCB(struct Togl* togl)
{
#if 0 /* THIS NOW TAKES PLACE IN THE "tvbase destroy" COMMAND. */
    TvBasePacket* tvPacket;

    tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    if (!tvPacket) return;

    tvBaseDestroy(tvPacket->base);
#endif
}

/*
 * t v T o g l D i s p l a y C B ( ) 
 * 
 */
void
tvToglDisplayCB(struct Togl* togl)
{
    const char   *funcName = "tvToglDisplayCB";
    TvBasePacket *tvPacket;
    Tk_Window    tkwin = Togl_TkWin(togl);

    TSMTRACE(( THREED_MASK|1, funcName, "Entering" ));

    /* retrieve the clientdata to pick up our tvBase and window... */
    tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    if (!tvPacket) return;

    /* This appears to be the only viable place we can trap on "map"
     * events.  Tk will however, issue a redraw/post-display when the
     * underlying window is first mapped. Thus...
     * On first time through, dispatch to register mapNotifiers.
     * 
     * NOTE: this probably not right -- there's no provision here to notice
     * if the window ever becomes subsequently unmapped/remapped. --RLB
     */

    if (!tvPacket->mapped && Tk_IsMapped(tkwin)) {
        int size[2];
        size[0] = Togl_Width(togl);
        size[1] = Togl_Height(togl);
        tvSetIntegerv(tvPacket->base, TV_SIZE, size);
        glmTclMapNotifyDispatch(tvPacket->glmw, (void*)tvPacket->base);
        /* NOTE: the ordering of these next two commands *seems* important;
         * tvBaseRedoVisibleList() signals on the threeD->vis_cond condition
         * variable. However, the thread spawned by tvBaseStart() waits on 
         * that condVar.  Previously, these two calls were swapped, and often
         * times, the display would hang, with the new thread waiting
         * indefinitely on the condVar.    --RLB 11/2000.
         */
        tvBaseStart(tvPacket->base);
//        tvBaseWaitRunnable(tvPacket->base); //????
        tvBaseRedoVisibleList(tvPacket->base, 0);
        tvPacket->mapped = TRUE;
    }
    else {
        glmPostRedraw(tvPacket->glmw);
    }
}


/*
 * t v T o g l R e s h a p e C B ( ) 
 * 
 */
void
tvToglReshapeCB(struct Togl* togl)
{
    const char    *funcName = "tvToglReshapeCB";
    int           size[2];
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);

    if (!tvPacket) return;

    size[0] = Togl_Width(togl);
    size[1] = Togl_Height(togl);
    tvSetIntegerv(tvPacket->base, TV_SIZE, size);

    TSMTRACE(( THREED_MASK|1, funcName, "size = %d x %d", size[0], size[1] ));
}
    


/*
 * We'll keep a hash table of the various "tvVariable" keywords to the set/get
 * commands.  The hash table will contain the keyword string along with an
 * index into a secondary table. This other table encodes the numbers and types
 * of arguments associated with each set/get variable.
 *
 */
enum TvVarsType {END = -1, TV_INT, TV_INTV, TV_FLOAT, TV_FLOATV,
		 TV_BOOLEAN, TV_STRING, TV_STRINGV};

static Tcl_HashTable tvVarsTable;
typedef struct TvVarsElements {
    tvVariable      key;
    enum TvVarsType type;
    int             numArgs;
    char*           keyStr;
} TvVarsElements;

static TvVarsElements tvVarsData[] = {
    /* dummy entry, to avoid zero as an index */
    {(tvVariable)-1, TV_BOOLEAN, 0, "- - -"},

    {TV_SHOW_LIGHTING, TV_BOOLEAN, 1, "TV_SHOW_LIGHTING"},
    {TV_SHOW_HEADS_UP, TV_BOOLEAN, 1, "TV_SHOW_HEADS_UP"},
    {TV_SHOW_WIREFRAME,TV_BOOLEAN, 1, "TV_SHOW_WIREFRAME"},
    {TV_SHOW_TEXTURES, TV_BOOLEAN, 1, "TV_SHOW_TEXTURES"}, 
    {TV_SHOW_MODELS,   TV_BOOLEAN, 1, "TV_SHOW_MODELS"},
    {TV_SHOW_SKY,      TV_BOOLEAN, 1, "TV_SHOW_SKY"},
    {TV_SHOW_GROUND,   TV_BOOLEAN, 1, "TV_SHOW_GROUND"},
    {TV_SHOW_DATASETS, TV_BOOLEAN, 1, "TV_SHOW_DATASETS"},
    {TV_SHOW_FRUSTUMS, TV_BOOLEAN, 1, "TV_SHOW_FRUSTUMS"},
    {TV_SHOW_TILES,    TV_BOOLEAN, 1, "TV_SHOW_TILES"},
    {TV_SHOW_PATHS,    TV_BOOLEAN, 1, "TV_SHOW_PATHS"},
    {TV_SHOW_LOGO,     TV_BOOLEAN, 1, "TV_SHOW_LOGO"},
    {TV_SHOW_CROSSHAIR,TV_BOOLEAN, 1, "TV_SHOW_CROSSHAIR"},
    {TV_CROSSHAIR,     TV_INTV, 2, "TV_CROSSHAIR"},
    {TV_FEATURES_WIREFRAME, TV_BOOLEAN, 1, "TV_FEATURES_WIREFRAME"},
    {TV_FEATURES_TEXTURED,  TV_BOOLEAN, 1, "TV_FEATURES_TEXTURED"},
    {TV_FEATURES_LIGHTING,  TV_BOOLEAN, 1, "TV_FEATURES_LIGHTING"},
    {TV_FEATURES_LOAD_TEXTURES, TV_BOOLEAN, 1, "TV_FEATURES_LOAD_TEXTURES"},
    {TV_FEATURES_BACKFACE_CULL, TV_BOOLEAN, 1, "TV_FEATURES_BACKFACE_CULL"},
    {TV_TIME_OF_DAY,       TV_FLOAT, 1, "TV_TIME_OF_DAY"},
    {TV_RESOLUTION_ADJUST, TV_FLOAT, 1, "TV_RESOLUTION_ADJUST"},
    {TV_ELEVATION_SCALE,   TV_FLOAT, 1, "TV_ELEVATION_SCALE"},
    {TV_BACKFACE_CULL,     TV_BOOLEAN,  1, "TV_BACKFACE_CULL"},
    {TV_HORIZON_CULL,      TV_BOOLEAN,  1, "TV_HORIZON_CULL"},
    {TV_USE_ZBUFFER_HACK,  TV_BOOLEAN,  1, "TV_USE_ZBUFFER_HACK"},
    {TV_TWOD_CONSTRAIN,    TV_BOOLEAN,  1, "TV_TWOD_CONSTRAIN"},
    {TV_MIPMAP_TEXTURES,   TV_BOOLEAN,  1, "TV_MIPMAP_TEXTURES"},
    {TV_FOG_DENSITY,       TV_FLOAT, 1, "TV_FOG_DENSITY"},
    {TV_HEIGHT_BASED_NAVIGATION, TV_BOOLEAN, 1, "TV_HEIGHT_BASED_NAVIGATION"},
    {TV_DOODLE_ON_TERRAIN, TV_BOOLEAN,  1, "TV_DOODLE_ON_TERRAIN"},
    {TV_FREEZE_TILES,      TV_BOOLEAN,  1, "TV_FREEZE_TILES"},
    {TV_SHOW_TILE_DEBUG,   TV_BOOLEAN,  1, "TV_SHOW_TILE_DEBUG"},
    {TV_STEREO,            TV_BOOLEAN,  1, "TV_STEREO"},
    {TV_SCALE_IPD,         TV_BOOLEAN,  1, "TV_SCALE_IPD"},
    {TV_ABSOLUTE_IPD,      TV_FLOAT, 1, "TV_ABSOLUTE_IPD"},
    {TV_VISIBLE_CYCLE,     TV_INTV,  2, "TV_VISIBLE_CYCLE"},
    {TV_DIS_SEND,          TV_BOOLEAN,  1, "TV_DIS_SEND"},
    {TV_DIS_MONITOR,       TV_BOOLEAN,  1, "TV_DIS_MONITOR"},
    {TV_DIS_ID,            TV_INTV,  3, "TV_DIS_ID"},
    {TV_DIS_SITE_ID,       TV_INT,   1, "TV_DIS_SITE_ID"},
    {TV_DIS_FORCE_ID,      TV_INT,   1, "TV_DIS_FORCE_ID"},
    {TV_DIS_ENTITY_ID,     TV_INT,   1, "TV_DIS_ENTITY_ID"},
    {TV_SIZE,              TV_INTV,  2, "TV_SIZE"},
    {TV_VELOCITY,          TV_FLOAT, 1, "TV_VELOCITY"},
    {TV_ACCELERATION,      TV_FLOAT, 1, "TV_ACCELERATION"},
    {TV_IMAGE_QUALITY,     TV_FLOAT, 1, "TV_IMAGE_QUALITY"},
    {TV_SLIDE_DELTA,       TV_FLOATV, 2, "TV_SLIDE_DELTA"},
    {TV_ATTITUDE_DELTA,    TV_FLOATV, 2, "TV_ATTITUDE_DELTA"},
    {TV_SPLASH_MESSAGE,    TV_STRING, 1, "TV_SPLASH_MESSAGE"},
    {TV_MOTION_THRESHOLD,  TV_FLOAT,  1, "TV_MOTION_THRESHOLD"},
    {TV_TILE_BY_TILE,      TV_BOOLEAN, 1, "TV_TILE_BY_TILE"},
    {TV_DOTGEO_PAUSE,      TV_INT,     1, "TV_DOTGEO_PAUSE"},
    {TV_AUTO_POP_FRONT,    TV_BOOLEAN, 1, "TV_AUTO_POP_FRONT"},
    {TV_SCHEMA_URL,        TV_STRING,  1, "TV_SCHEMA_URL"},
    {TV_HYPERLINK_URL,     TV_STRING,  1, "TV_HYPERLINK_URL"},
    {TV_HYPERLINK_DESC,    TV_STRING,  1, "TV_HYPERLINK_DESC"},
    {TV_MOVING,            TV_BOOLEAN, 1, "TV_MOVING"},
    {TV_POPPING_STRATEGY,  TV_INT,     1, "TV_POPPING_STRATEGY"},
    {TV_SHOW_HEADS_UP_DEBUG,TV_BOOLEAN,1, "TV_SHOW_HEADS_UP_DEBUG"},
    {TV_SHOW_LATLONG,      TV_BOOLEAN,  1, "TV_SHOW_LATLONG"},
    {TV_OAA_ID,            TV_STRING,  1, "TV_OAA_ID"},
    {TV_DOTGEO_COMPRESS,   TV_BOOLEAN, 1, "TV_DOTGEO_COMPRESS"},
    {TV_DOTGEO_QUERY_TO_TOP, TV_BOOLEAN, 1, "TV_DOTGEO_QUERY_TO_TOP"},
    {TV_DOTGEO_MAX_RESULTS, TV_INT, 1, "TV_DOTGEO_MAX_RESULTS"},
    {TV_DOTGEO_CACHE_CELLS, TV_BOOLEAN, 1, "TV_DOTGEO_CACHE_CELLS"},
    {TV_DOTGEO_PROFILE_URL, TV_STRING, 1, "TV_DOTGEO_PROFILE_URL"},

    {(tvVariable)-1, END, 0, ""}
};

static Tcl_HashTable tvuVarsTable;
typedef struct TvuVarsElements {
    int             key;
    enum TvVarsType type;
    int             numArgs;
    char*           keyStr;
} TvuVarsElements;

static TvuVarsElements tvuVarsData[] = {
    /* dummy entry, to avoid zero as an index */
    {(tvVariable)-1, TV_BOOLEAN, 0, "- - -"},

    {TVU_ARGS_PARSED,       TV_INT,     1, "TVU_ARGS_PARSED"},
    {TVU_DROP_TO_WIREFRAME, TV_FLOAT,   1, "TVU_DROP_TO_WIREFRAME"},
    {TVU_SCHEMA_URL,        TV_STRING,  1, "TVU_SCHEMA_URL"},
    {TVU_TRACE_MASK,        TV_INT,     1, "TVU_TRACE_MASK"}, 
    {TVU_NUM_DATASETS,      TV_INT,     1, "TVU_NUM_DATASETS"},
    {TVU_OI_URL,            TV_STRINGV, 2, "TVU_OI_URL"},
    {TVU_DEM_URL,           TV_STRINGV, 2, "TVU_DEM_URL"},
    {TVU_INITIAL_VIEWER,    TV_INT,     1, "TVU_INITIAL_VIEWER"},
    {TVU_TV_VERSION,        TV_STRING,  1, "TVU_TV_VERSION"},
    {TVU_STEREO,            TV_INT,     1, "TVU_STEREO"},
    {TVU_SINGLE_BUFFER,     TV_INT,     1, "TVU_SINGLE_BUFFER"},
    {TVU_TILE_BY_TILE,      TV_INT,     1, "TVU_TILE_BY_TILE"},
    {TVU_NUM_COMMANDS,      TV_INT,     1, "TVU_NUM_COMMANDS"},
    {TVU_COMMAND,           TV_STRINGV, 2, "TVU_COMMAND"},
    {TVU_CONFIG_URL,        TV_STRING,  1, "TVU_CONFIG_URL"},
    {TVU_SHOW_LICENSE,      TV_INT,     1, "TVU_SHOW_LICENSE"},
    {TVU_LOG,               TV_INT,     1, "TVU_LOG"},
    {TVU_NO_IPC,            TV_INT,     1, "TVU_NO_IPC"},
    {TVU_WIDTH,             TV_INT,     1, "TVU_WIDTH"},
    {TVU_HEIGHT,            TV_INT,     1, "TVU_HEIGHT"},
    {TVU_CAVE,              TV_INT,     1, "TVU_CAVE"},
    {TVU_INSTALL_DIR,       TV_STRING,  1, "TVU_INSTALL_DIR"},
    {TVU_USER_DIR,          TV_STRING,  1, "TVU_USER_DIR"},
    {TVU_MODE,              TV_INT,     1, "TVU_MODE"},
    {TVU_PATH_URL,          TV_STRING,  1, "TVU_PATH_URL"},
    {TVU_VIEWPOINT_URL,     TV_STRING,  1, "TVU_VIEWPOINT_URL"},
    {TVU_DATABASE_URL,      TV_STRING,  1, "TVU_DATABASE_URL"},
    {TVU_LICENSE_ACCEPTED,  TV_STRING,  1, "TVU_LICENSE_ACCEPTED"},
    {TVU_VERSION,           TV_FLOAT,   1, "TVU_VERSION"},
    {TVU_TOTAL_ARGS,        TV_INT,     1, "TVU_TOTAL_ARGS"},
    {TVU_PATHS_DIR,         TV_STRING,  1, "TVU_PATHS_DIR"},
    {TVU_TRACE_FILE,        TV_STRING,  1, "TVU_TRACE_FILE"},
    {TVU_PROFILE_URL,       TV_STRING,  1, "TVU_PROFILE_URL"},
    {TVU_TCL_FILENAME,      TV_STRING,  1, "TVU_TCL_FILENAME"},
    {TVU_VISUAL_ID,         TV_INT,     1, "TVU_VISUAL_ID"},
    
    {(tvVariable)-1, END, 0, ""}
};

static int
tvTclError( Tcl_Interp *interp, int argc, char *argv[] )
{
  Tcl_SetObjResult( interp, genericErrorMessage(argc, argv) );
  return TCL_ERROR;  /* eventually provide more helpful message... */
}

/*
 * t v S e t C o m m a n d ( )
 *
 */
int
tvSetCommand(struct Togl* togl, int argc, char* argv[])
{
    Tcl_HashEntry* entry;
    int index;
    Tcl_Interp* interp = Togl_Interp(togl);
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    int argi;
    double argd;

    if (argc <= 2) return tvTclError( interp, argc, argv );

    /* lookup the keyword in our hashtable... */
    entry = Tcl_FindHashEntry(&tvVarsTable, argv[2]);
    if (!entry) return tvTclError( interp, argc, argv );

    /* retrieve the index into the secondary table... */
    index = (int)Tcl_GetHashValue(entry);
    if (argc != (3+tvVarsData[index].numArgs))
      return tvTclError( interp, argc, argv );

    switch (tvVarsData[index].type) {
    case TV_BOOLEAN:
        if (Tcl_GetBoolean(interp, argv[3], &argi) == TCL_ERROR)
            return tvTclError( interp, argc, argv );
        tvSetBoolean(tvPacket->base, tvVarsData[index].key, argi);
        break;
    case TV_INT:
        if (Tcl_GetInt(interp, argv[3], &argi) == TCL_ERROR)
            return tvTclError( interp, argc, argv );
        tvSetInteger(tvPacket->base, tvVarsData[index].key, argi);
        break;
    case TV_FLOAT:
        if (Tcl_GetDouble(interp, argv[3], &argd) == TCL_ERROR)
            return tvTclError( interp, argc, argv );
        tvSetFloat(tvPacket->base, tvVarsData[index].key, (float)argd);
        break;
    case TV_STRING:
        tvSetString(tvPacket->base, tvVarsData[index].key, argv[3] );
	break;
    case TV_INTV:
    {
        /* cannot have variable init: int args[tvVarsData[index].numArgs]; */
        int args[5];
        int i;
        for (i=0; i<tvVarsData[index].numArgs; i++) 
            if (Tcl_GetInt(interp, argv[3+i], &args[i]) == TCL_ERROR)
                return tvTclError( interp, argc, argv );
        tvSetIntegerv(tvPacket->base, tvVarsData[index].key, args);
        break;
    }
    case TV_FLOATV:
    {
        /* cannot have variable init: int args[tvVarsData[index].numArgs]; */
        float args[5];
        int i;
        for (i=0; i<tvVarsData[index].numArgs; i++) {
            double d;
            if (Tcl_GetDouble(interp, argv[3+i], &d) == TCL_ERROR)
                return tvTclError( interp, argc, argv );
            args[i] = (float)d;
        }
        tvSetFloatv(tvPacket->base, tvVarsData[index].key, args);
        break;
    }
    default:
        return tvTclError( interp, argc, argv );
    }

    return TCL_OK;
}

/*
 * t v G e t C o m m a n d ( )
 *
 */
int
tvGetCommand(struct Togl* togl, int argc, char* argv[])
{
    Tcl_HashEntry* entry;
    int index;
    Tcl_Interp* interp = Togl_Interp(togl);
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    Tcl_Obj* retObj;

    if (argc != 3) return tvTclError( interp, argc, argv );

    /* lookup the keyword in our hashtable... */
    entry = Tcl_FindHashEntry(&tvVarsTable, argv[2]);
    if (!entry) return tvTclError( interp, argc, argv );

    /* get the index into the secondary table... */
    index = (int)Tcl_GetHashValue(entry);

    retObj = Tcl_NewObj();
    switch (tvVarsData[index].type) {
    case TV_BOOLEAN:
    {
        PRBool val = tvGetBoolean(tvPacket->base, tvVarsData[index].key);
        Tcl_SetBooleanObj(retObj, val);
        break;
    }
    case TV_INT:
    {
        int val = tvGetInteger(tvPacket->base, tvVarsData[index].key);
        Tcl_SetIntObj(retObj, val);
        break;
    }
    case TV_FLOAT:
    {
        double val = tvGetFloat(tvPacket->base, tvVarsData[index].key);
        Tcl_SetDoubleObj(retObj, val);
        break;
    }
    case TV_INTV:
    {
        /* cannot have variable init: int args[tvVarsData[index].numArgs]; */
        int vals[5];
        int i;
        tvGetIntegerv(tvPacket->base, tvVarsData[index].key, vals);
        for (i=0; i<tvVarsData[index].numArgs; i++) {
            Tcl_Obj* obj = Tcl_NewObj();
            Tcl_SetIntObj(obj, vals[i]);
            Tcl_ListObjAppendList(interp, retObj, obj);
        }
        break;
    }
    case TV_FLOATV:
    {
        /* cannot have variable init: int args[tvVarsData[index].numArgs]; */
        float vals[5];
        int i;
        tvGetFloatv(tvPacket->base, tvVarsData[index].key, vals);
        for (i=0; i<tvVarsData[index].numArgs; i++) {
            Tcl_Obj* obj = Tcl_NewObj();
            Tcl_SetDoubleObj(obj, vals[i]);
            Tcl_ListObjAppendList(interp, retObj, obj);
        }
        break;
    }
    case TV_STRING:
    {
        char *val = (char*) tvGetString(tvPacket->base, tvVarsData[index].key);
        Tcl_SetStringObj(retObj, (val) ? val : "", -1);
	if ( val ) free( val );
        break;
    }
    default:
        return tvTclError( interp, argc, argv );
    }

    Tcl_SetObjResult(interp, retObj);
    return TCL_OK;
}

static void
AddToModelList( Tcl_Interp *interp, TsmModel *model, Tcl_Obj **obj )
{
  int      j;
  char     numurls[32];
  Tcl_Obj  *this = Tcl_NewListObj( 0, NULL );
  Tcl_Obj  *string;

  /* append the model name to the list: use title if given, otherwise name */

  string = Tcl_NewObj();
  if ( model->title )
    Tcl_SetStringObj( string, model->title, -1 );
  else
    Tcl_SetStringObj( string, model->name, -1 );
  Tcl_ListObjAppendElement( interp, this, string );
  
  /* append the model geoname to the list */

  string = Tcl_NewObj();
  Tcl_SetStringObj( string, model->geoname, -1 );
  Tcl_ListObjAppendElement( interp, this, string );

  /* append the model description to the list */

  string = Tcl_NewObj();
  Tcl_SetStringObj( string, model->description, -1 );
  Tcl_ListObjAppendElement( interp, this, string );

  /* append the count of model URLs to the list */

  string = Tcl_NewObj();
  sprintf( numurls, "%d", model->numLocations );
  Tcl_SetStringObj( string, numurls, -1 );
  Tcl_ListObjAppendElement( interp, this, string );
  
  /* append all of the model URLs to the list */

  for ( j = 0; j < model->numLocations; j++ ) {
    string = Tcl_NewObj();
    Tcl_SetStringObj( string, model->location[j]->url, -1 );
    Tcl_ListObjAppendElement( interp, this, string );
  }

  Tcl_ListObjAppendElement( interp, *obj, this );
}

typedef struct {
  Tcl_Interp *interp;
  Tcl_Obj    *obj;
} MakeModelCl;

static void
MakeModelList( TsGeoPyramid *geo, void *closure )
{
  MakeModelCl *modelclosure = (MakeModelCl *) closure;
  TsmDatabase *database;
  TsmGeoSet   *geoset = NULL;
  TsmModel    *model;
  char const  *geoset_name = ( geo ) ? TsGeoName( geo ) : NULL;
  int         i;

  /* Get the list of models for the named geoset */

  if ( ( database = getDBase() ) == NULL ) return;
  if ( geoset_name ) {
    if ( ( geoset = tsmGetGeoSet( database, geoset_name ) ) == NULL ) return;
    if ( geoset->numModels == 0 ) return;
  }

  /* now go through the list and create button widgets from them */

  if ( geoset ) {
    for ( i = 0; i < geoset->numModels; i++ ) {
      model = tsmGetModel( database, geoset->model_name[i] );
      if ( model != NULL )
	AddToModelList( modelclosure->interp, model, &modelclosure->obj );
    }
  } else {
    for ( i = 0; i < database->numModels; i++ ) {
      model = &database->model[i];
      if ( model->geoname == NULL )
	AddToModelList( modelclosure->interp, model, &modelclosure->obj );
    }
  }
}

typedef struct {
  tvBase *base;
  char   *url;
} ModelLoadClosure;

static void
tvLoadModelThread( void *closure )
{
  ModelLoadClosure *info = (ModelLoadClosure *) closure;

  if ( info && info->base && info->url ) {
    if ( FeatureRefCountLoad( info->base, info->url ) != NULL ) {
      
      /* tell the GUI layer that we have loaded a model for it */
      tvMsgListAdd( "MODEL_ADD", "", info->url, NULL );

    } else {

      /* display a failure message on the dataset console.  N.B. */
      /* a success message is handled within FeatureRefCountLoad */
      tvConsoleMsg( "Failed to load model - %s", info->url );
      
    }
    tsmMemFree( info->url );
    tsmMemFree( info );
  }
}

int
tvFeatureCommand(struct Togl* togl, int argc, char* argv[])
{
  TvBasePacket *tvPacket;
  Tcl_Interp   *interp = Togl_Interp(togl);

  if ( argc < 2 )
    return TCL_ERROR;

  /* get the tvBase instance... */

  if ( ! ( tvPacket = (TvBasePacket*) Togl_GetClientData(togl) ) )
    return TCL_ERROR;

  /* check for the supported commands */

  if ( strcmp( argv[2], "load" ) == 0 ) {
    if ( argc < 3 ) return TCL_ERROR;

    /* do the actual model loading in a separate thread */
    /* so that we don't hang up the user interface.     */

    if ( tvPacket->base ) {
      PRThread *newthread;
      ModelLoadClosure *closure = tsmMemMalloc( sizeof(ModelLoadClosure) );
      if ( closure ) {
	closure->base = tvPacket->base;
	closure->url = tsmMemStrdup( argv[3] );
	newthread = PR_CreateThread( PR_USER_THREAD, tvLoadModelThread,
				     closure, PR_PRIORITY_HIGH,
				     PR_GLOBAL_THREAD, PR_JOINABLE_THREAD,
				     1<<16 );
	pthr_set_name( newthread, "Load Model" );
      }
    }

  } else if ( strcmp( argv[2], "unload" ) == 0 ) {
    if ( argc < 3 ) return TCL_ERROR;
    if ( tvPacket->base ) FeatureUnload( tvPacket->base, argv[3] );

  } else if ( strcmp( argv[2], "unloadall" ) == 0 ) {
    if ( tvPacket->base )
      FeatureUnloadAll( tvPacket->base );

  } else if ( strcmp( argv[2], "info" ) == 0 ) {
    Tcl_DString theMsg;
    Tcl_DString *theMsgP = &theMsg;
    FeatureItem *item;
    Tcl_Obj     *retVal;
    
    if ( argc < 3 ) return TCL_ERROR;
    if ( ( item = FeatureQuery( argv[3] ) ) ) {
      char buffer[512];
      char *na = "Not Specified";

      Tcl_DStringInit( theMsgP );

      Tcl_DStringAppend( theMsgP, "Author: ", -1 );
      Tcl_DStringAppend( theMsgP, item->author ? item->author : na, -1 );
      Tcl_DStringAppend( theMsgP, "\n", -1 );

      Tcl_DStringAppend( theMsgP, "Copyright: ", -1 );
      Tcl_DStringAppend( theMsgP, item->copyright ? item->copyright:na , -1 );
      Tcl_DStringAppend( theMsgP, "\n", -1 );

      Tcl_DStringAppend( theMsgP, "Description: ", -1 );
      Tcl_DStringAppend( theMsgP, item->description ? item->description:na,-1);
      Tcl_DStringAppend( theMsgP, "\n", -1 );

      Tcl_DStringAppend( theMsgP, "Associated Terrain Model: ", -1 );
      Tcl_DStringAppend( theMsgP, item->geoset ? item->geoset : "None", -1 );
      Tcl_DStringAppend( theMsgP, "\n", -1 );

      sprintf( buffer, "Filesize: %d KB\n", item->filesize / 1024 );
      Tcl_DStringAppend( theMsgP, buffer, -1 );

      sprintf( buffer, "Anchor Nodes: %d\n", item->anchors );
      Tcl_DStringAppend( theMsgP, buffer, -1 );

      sprintf( buffer, "Texture Maps: %d\n", item->textures );
      Tcl_DStringAppend( theMsgP, buffer, -1 );

      sprintf( buffer, "Inline Nodes: %d", item->inlines );
      Tcl_DStringAppend( theMsgP, buffer, -1 );

      retVal = Tcl_NewStringObj( Tcl_DStringValue(theMsgP), -1 );
      Tcl_SetObjResult( interp, retVal );
      
      FeatureFree( item );
    }

  } else if ( strcmp( argv[2], "list" ) == 0 ) {
    Tcl_Obj *obj = Tcl_NewListObj( 0, NULL );
    MakeModelCl closure;

    closure.interp = interp;
    closure.obj = obj;

    /* models associated with a particular terrain model */
    tvForallDatasets( tvPacket->base, MakeModelList, &closure );
    /* global models associated with no terrain dataset */
    MakeModelList( NULL, &closure );

    /* return list of [name geoname description numurls url1 ...] */
    Tcl_SetObjResult( interp, obj );
    
  } else
    return TCL_ERROR;

  return TCL_OK;
}


int
tvLogCommand(struct Togl* togl, int argc, char* argv[])
{
  return TRUE;
}


int
tvOAACommand(struct Togl* togl, int argc, char* argv[])
{
  TvBasePacket     *tvPacket;
  Tcl_Interp       *interp = Togl_Interp(togl);

  if ( argc < 3 )
    return TCL_ERROR;

  /* get the tvBase instance... */

  if ( ! ( tvPacket = (TvBasePacket*) Togl_GetClientData(togl) ) )
    return TCL_ERROR;

  /* check for the supported commands */

  if ( strcmp( argv[2], "interpret" ) == 0 ) {
    if ( tvPacket->base && argc > 3 )
      tvInterpret( tvPacket->base, argv[3] );

  } else if ( strcmp( argv[2], "get" ) == 0 ) {
    char *hostname = GetOAAHost();
    Tcl_Obj *obj = Tcl_NewListObj( 0, NULL );
    Tcl_Obj *string;

    string = Tcl_NewObj();
    Tcl_SetStringObj( string, (hostname)?hostname:"", -1 );
    Tcl_ListObjAppendElement( interp, obj, string );

    string = Tcl_NewObj();
    Tcl_SetIntObj( string, GetOAAPort() );
    Tcl_ListObjAppendElement( interp, obj, string );

    Tcl_SetObjResult( interp, obj );
    
    if ( hostname ) free( hostname );

  } else if ( strcmp( argv[2], "set" ) == 0 ) {
    if ( argc < 5 ) return TCL_ERROR;
    
    SetOAAHost( argv[3] );
    SetOAAPort( atoi(argv[4]) );

  } else if ( strcmp( argv[2], "connect" ) == 0 ) {
    if ( tvPacket->base )
      OAAConnect( tvPacket->base );
    
  } else if ( strcmp( argv[2], "disconnect" ) == 0 ) {
    if ( tvPacket->base )
      OAADisconnect( tvPacket->base );

  } else if ( strcmp( argv[2], "getconnected" ) == 0 ) {
    char *host = GetOAAConnected();

    Tcl_Obj *retVal = Tcl_NewObj();
    Tcl_SetStringObj( retVal, (host)?host:"", -1 );
    
    Tcl_SetObjResult( interp, retVal );
    
  } else if ( strcmp( argv[2], "solve" ) == 0 ) {
    Tcl_Obj *obj;
    char    *ans;
    if ( argc < 6 ) return TCL_ERROR;
    ans = OAASolve( argv[3], argv[4], argv[5] );
    if ( ! ans ) ans = "[]";
    obj = Tcl_NewStringObj( ans, -1 );
    Tcl_SetObjResult( interp, obj );

  } else if ( strcmp( argv[2], "list" ) == 0 ) {
    Tcl_Obj *obj = Tcl_NewListObj( 0, NULL ), *curval;
    /* possible solve options might be [blocking(true),time_limit(10)] */
    char *ids = OAASolve( "getinfo(Id,Host,Os,Arch,Procs,Res)", NULL, NULL );

    if ( ids != NULL ) {
      int listlen, i, j;
      char *function, *func_name, *func_args, *string;

      list_to_terms( ids );
      listlen = list_len( ids );

      for ( i = 0; i < listlen; i++ ) {
	/* strip the terrain3d() bit */
	NthElt( ids, i+1, &function );
	Functor( function, &func_name, &func_args );
	OAA_FREE( function );
	OAA_FREE( func_name );
	function = func_args;
	/* strip the getinfo(),[] bit */
	Functor( function, &func_name, &func_args );
	OAA_FREE( function );
	OAA_FREE( func_name );
	/* extract each of the first 5 strings of the getinfo() result */
	curval = Tcl_NewListObj( 0, NULL );
	for ( j = 1; j <= 5; j++ ) {
	  NthElt( func_args, j, &string );
	  remove_quotes( string );
	  Tcl_ListObjAppendElement(interp,curval,Tcl_NewStringObj(string,-1));
	}
	/* add this list of 5 strings to our list result */
	Tcl_ListObjAppendElement( interp, obj, curval );
	OAA_FREE( func_args );
      }

      free( ids );
    }

    Tcl_SetObjResult( interp, obj );

  } else
    return TCL_ERROR;

  return TCL_OK;
}

/*
 * t v A n c h o r C o m m a n d
 *
 * Implements the "anchor" commands for supporting hyperlinks
 *
 */
int
tvAnchorCommand(struct Togl* togl, int argc, char* argv[])
{
  TvBasePacket*  tvPacket;
  Tcl_Interp*    interp = Togl_Interp(togl);
  int            result = 0;
  Tcl_Obj        *retObj;

  if (argc < 3) {
    Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
    return TCL_ERROR;  /* eventually provide more helpful message... */
  }

  if ( !(tvPacket = (TvBasePacket*)Togl_GetClientData(togl)))
    return TCL_ERROR;

  /* have we clicked on a hyperlink? */
  if (!strcmp(argv[2], "click")) {
    const char *link_url = tvGetString( tvPacket->base, TV_HYPERLINK_URL );
    if ( link_url ) {
      if ( strlen(link_url) > 0 ) tsmShowUrl( link_url );
      free( (char *) link_url );
      result = 1;
    }
  }

  /* find the hyperlink under the cursor position, only if moving */
  else if (!strcmp(argv[2], "move")) {
    const char *link_desc = NULL;

    if (argc < 5) {
      Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
      return TCL_ERROR;  /* eventually provide more helpful message... */
    }

    if ( ! tvGetBoolean( tvPacket->base, TV_MOVING ) ) {
      int  mx = atoi( argv[3] ), my = atoi( argv[4] );

      tvBaseCheckForHyperlink( tvPacket->base, mx, my );

      /* get a description for the hyperlink under cursor, if any */
      
      link_desc = tvGetString( tvPacket->base, TV_HYPERLINK_DESC );
      if ( link_desc == NULL )
	link_desc = tvGetString( tvPacket->base, TV_HYPERLINK_URL );
    }

    /* return the status bar description */

    retObj = Tcl_NewObj();
    if ( link_desc == NULL )
      Tcl_SetStringObj(retObj, " ", -1);
    else {
      Tcl_SetStringObj(retObj, (char *) link_desc, -1);
      free( (char *) link_desc );
    }
    Tcl_SetObjResult(interp, retObj);
    return TCL_OK;
  }
  
  /* did we clicked on a hyperlink? If so, unhighlight the link  */
  else if (!strcmp(argv[2], "release")) {

    const char *link_url = tvGetString( tvPacket->base, TV_HYPERLINK_URL );
    if ( link_url ) {
      free( (char *) link_url );
      tvSetString( tvPacket->base, TV_HYPERLINK_URL, NULL );
      tvBasePostRedraw( tvPacket->base, FALSE );
      result = 1;
    }

  }

  retObj = Tcl_NewObj();
  Tcl_SetIntObj(retObj, result);
  Tcl_SetObjResult(interp, retObj);
  return TCL_OK;
}

/*
 * t v S t r o k e C o m m a n d
 *
 * Implements the various "stroke" commands for annotating on the screen.
 *
 */
int
tvStrokeCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket*  tvPacket;
    Tcl_Interp*    interp = Togl_Interp(togl);
    int x, y;
    float timeOut;

    if (argc < 3) {
        Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
        return TCL_ERROR;  /* eventually provide more helpful message... */
    }

    if ( !(tvPacket = (TvBasePacket*)Togl_GetClientData(togl)))
        return TCL_ERROR;

    if (!strcmp(argv[2], "clear")) {
        if (argc != 3) {
            Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
            return TCL_ERROR;  /* eventually provide more helpful message... */
        }
        ClearSentence(tvPacket->base);
        tvBasePostRedraw(tvPacket->base, FALSE);
        return TCL_OK;
    }

    if (!strcmp(argv[2], "timeOut")) {
        if (argc != 4 || (sscanf(argv[3], "%f", &timeOut) != 1)) {
            Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
            return TCL_ERROR;  /* eventually provide more helpful message... */
        }
        SetDrawTimeout(timeOut);
        return TCL_OK;
    }

    /* the remaining possibilities have X/Y arguments;  
     * parse those out... 
     */
    if (argc != 5 || (sscanf(argv[3], "%d", &x) != 1) ||
        (sscanf(argv[4], "%d", &y) != 1))
    {
        Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
        return TCL_ERROR;  /* eventually provide more helpful message... */
    }


    if (!strcmp(argv[2], "begin")) {
        tvBeginStroke(tvPacket->base, x, y);
    }
    else if (!strcmp(argv[2], "continue")) {
        tvContinueStroke(tvPacket->base, x, y);
    }
    else if (!strcmp(argv[2], "end")) {
        tvContinueStroke(tvPacket->base, x, y);
        tvEndStroke(tvPacket->base);
    }
    else {
          Tcl_SetObjResult(interp, genericErrorMessage(argc, argv));
          return TCL_ERROR;  /* eventually provide more helpful message... */
    }

    return TCL_OK;
}

extern void populateGeosetsList(TsGeoPyramid*, void*);
typedef struct ClosurePacket {
    Tcl_Interp* interp;
    Tcl_Obj*    theList;
} ClosurePacket;

static int
tvTclGeoError( Tcl_Interp *interp, int argc, char *argv[] )
{
  char buff[100];

  strcpy(buff, "unknown tsGeoPyramid instance: ");
  strcat(buff, argv[3]);
  Tcl_AppendResult(interp, buff, NULL);
  return TCL_ERROR;
}

static int
tvTclGeoUsage( Tcl_Interp *interp, int argc, char *argv[] )
{
  char buff[100];

  strcpy(buff, "usage: ");
  strcat(buff, argv[0]);
  strcat(buff, " geoset add|remove|list|priority tsGeoPyramid_name");
  Tcl_AppendResult(interp, buff, NULL);
  return TCL_ERROR;
}

/*
 * t v G e o s e t C o m m a n d ( )
 *
 */
int
tvGeosetCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket* tvPacket;
    TsGeoPyramidInst* dataset;
    Tcl_Interp* interp = Togl_Interp(togl);

    if (argc < 3) {
        return tvTclGeoUsage( interp, argc, argv );
    }
    
    /* get the tvBase instance... */
    tvPacket = (TvBasePacket*)Togl_GetClientData(togl);

    /*
     * a d d 
     */
    if (!strcmp(argv[2], "add")) {
        if (argc != 4) return tvTclGeoUsage( interp, argc, argv );

        /* the 4th arg represents the "name" of the TsGeoPyramid instance...
         * find it in the assocative array...
         */
        dataset = (TsGeoPyramidInst*)Tcl_GetAssocData(Togl_Interp(togl), 
            argv[3], NULL);
        if (!dataset) return tvTclGeoError( interp, argc, argv );

        /* should have everything; add this geoset to our rendering thread.. */
        tvAddDataset(tvPacket->base, dataset->geoset);
        tvBaseRedoVisibleList(tvPacket->base, 0);
        return TCL_OK;
    }

    /*
     * r e m o v e 
     */
    if (!strcmp(argv[2], "remove")) {
        if (argc != 4) return tvTclGeoUsage( interp, argc, argv );

        /* the 4th arg represents the "name" of the TsGeoPyramid instance...
         * find it in the assocative array...
         */
        dataset = (TsGeoPyramidInst*)Tcl_GetAssocData(Togl_Interp(togl), 
            argv[3], NULL);
        if (!dataset) return tvTclGeoError( interp, argc, argv );

        /* should have everything; add this geoset to our rendering thread.. */
        tvRemoveDataset(tvPacket->base, dataset->geoset);
        tvBaseRedoVisibleList(tvPacket->base, 0);
        return TCL_OK;
    }

    /*
     * l i s t 
     */
    if (!strcmp(argv[2], "list")) {
        ClosurePacket closure;
        closure.theList = Tcl_NewListObj(0, NULL);

        tvForallDatasets(tvPacket->base, populateGeosetsList, (void*)&closure);

        Tcl_SetObjResult(Togl_Interp(togl), closure.theList);
        return TCL_OK;
    }

    /*
     * l i s t 
     */
    if (!strcmp(argv[2], "priority")) {
        int prio;

        if (argc != 5) return tvTclGeoUsage( interp, argc, argv );

        /* the 4th arg represents the "name" of the TsGeoPyramid instance...
         * find it in the assocative array...
         */
        dataset = (TsGeoPyramidInst*)Tcl_GetAssocData(Togl_Interp(togl), 
						      argv[3], NULL);
        if (!dataset) return tvTclGeoError( interp, argc, argv );

	prio = tvDatasetPriority (tvPacket->base, dataset->geoset);

	if ( strcmp( argv[4], "up" ) == 0 )
	  prio++;
	else if ( strcmp( argv[4], "down" ) == 0 )
	  prio--;
	else if ( strcmp( argv[4], "top" ) == 0 )
	  prio = -1;
	else if ( strcmp( argv[4], "bottom" ) == 0 )
	  prio = 0;
	else
	  prio = atoi(argv[4]);

	tvDatasetSetPriority (tvPacket->base, dataset->geoset, prio);
        tvBaseRedoVisibleList(tvPacket->base, 0);
	tvBasePostRedraw( tvPacket->base, FALSE );
        return TCL_OK;
    }
    
    return tvTclGeoUsage( interp, argc, argv );
}

/*
 * p o p u l a t e G e o s e t s L i s t 
 *
 * Companion utility to the "geoset list" command above; this function serves
 * as a callback to the libtv2 function "tvForallDatasets".  It adds the name
 * of each tsGeoPyramid to the Tcl-list that is passed in as closure.
 *
 */
void
populateGeosetsList(TsGeoPyramid* geo, void* closure)
{
    ClosurePacket* packet = (ClosurePacket*)closure;

    if (geo && packet) {
        Tcl_Obj* element = Tcl_NewStringObj((char*)TsGeoName(geo), -1);
        Tcl_ListObjAppendElement(packet->interp, packet->theList, element);
    }

    return;
}

/*
 * t v D e s t r o y C o m m a n d ( )
 *
 * Implements a means to shutdown and clean up a tvBase instance before closing
 * the associated window.
 *
 * NOTE: before we were relying on the Togl destroy callback to clean up
 * things. However, apparently by the time that callback is invoked, the Tk
 * window data has already been destroyed; this messes up the call to 
 * tvBaseDestroy(), as downstream from it, reposts, etc., take place as part
 * of the shutdown.
 *
 * Thus, the model we impose is to explicitly "destroy" the tvBase instance
 * before destroying the togl window.  I don't like this, as there's nothing 
 * to enforce this policy, are even make it apparent. But I don't have a good
 * solution at this point.  --RLB 1/31/2001.
 *
 */
int
tvDestroyCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket* tvPacket;
    Tcl_Interp* interp = Togl_Interp(togl);
    /* TsGeoPyramidInst* dataset; */

    /* get the tvBase instance... */
    tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    if (!tvPacket) return TCL_ERROR;

    /* call into libtv2 to do the shutdown.... */
    tvBaseDestroy(tvPacket->base);
    return TCL_OK;
}


/*
 * t v B a s e C m d s I n i t ( )
 *
 * Perform various initializations required for operation of tvBase commands.
 *
 */
void
tvBaseCmdsInit()
{
  int i = 0;

  /* initialise the features module */
  FeatureInit();

  /* build out the "tvVariable" get/set hash table... */
  Tcl_InitHashTable(&tvVarsTable, TCL_STRING_KEYS);
  for (i = 0; ; i++) {
    Tcl_HashEntry* entry;
    int new;
    
    if (tvVarsData[i].type == END) break;
    entry = Tcl_CreateHashEntry(&tvVarsTable, tvVarsData[i].keyStr, &new);
    Tcl_SetHashValue(entry, (ClientData)i);
  }
  
  /* build out the "tvuVariable" get/set hash table... */
  Tcl_InitHashTable(&tvuVarsTable, TCL_STRING_KEYS);
  for (i = 0; ; i++) {
    Tcl_HashEntry* entry;
    int new;
    
    if (tvuVarsData[i].type == END) break;
    entry = Tcl_CreateHashEntry(&tvuVarsTable, tvuVarsData[i].keyStr, &new);
    Tcl_SetHashValue(entry, (ClientData)i);
  }
}

Tcl_Obj* 
genericErrorMessage(int argc, char* argv[])
{
    int i=0;
    Tcl_Obj* retObj = Tcl_NewObj();
    static char prefix[] = "Unknown Command or wrong # args: ";
    int len = strlen(prefix);

    for (i=0; i<argc; i++) {
        len += strlen(argv[i]) + 1;  /* include spaces and last NULL */
    }
    
    {
        char *buff;
        if ( ( buff = malloc(len) ) ) {
          strcpy(buff, prefix);
          for (i=0; i<argc; i++) {
              strcat(buff, " ");
              strcat(buff, argv[i]);
          }
          free( buff );
        }

        Tcl_SetStringObj(retObj, buff, -1);
    }

    return retObj;
}

/*
 * t v u t i l s G a z ()
 *
 * Entry point for TCL to access the gazetteer facility of tvutils.
 * That is, to translate an address to a lat/long location. This
 * returns a list of strings as a result, these can be:
 *
 * error:     [ "error" ] or []
 * success:   [ "location", "<latitude>", "<longitude>" ]
 * conflicts: [ "choice", <citydesc1>, <citydesc2> [, <citydesc3>] ]
 *
 */

static int
tvutilsGaz( ClientData clientdata, Tcl_Interp *interp, int objc,
	    Tcl_Obj *const objv[] )
{
  char           *street, *city, *province, *postcode, *country;
  tvuGazResponse *response;
  Tcl_Obj        *list = Tcl_NewListObj( 0, NULL );

  if ( objc < 7 ) {
    Tcl_SetObjResult(interp, list);
    return TCL_ERROR;
  }

  street = Tcl_GetStringFromObj( objv[2], NULL );
  city = Tcl_GetStringFromObj( objv[3], NULL );
  province = Tcl_GetStringFromObj( objv[4], NULL );
  postcode = Tcl_GetStringFromObj( objv[5], NULL );
  country = Tcl_GetStringFromObj( objv[6], NULL );

  response = tvuGazetteer( street, city, province, postcode, country );

  if ( response != NULL ){
    char buffer[2048];

    if ( response->type == TVU_GAZ_LATLONG ) {

      /* return the "location" array, which just contains the lat and long */

      strcpy( buffer, "location" );
      Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj(buffer, -1) );
      sprintf( buffer, "%1lf", response->latitude );
      Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj(buffer, -1) );
      sprintf( buffer, "%1lf", response->longitude );
      Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj(buffer, -1) );

    } else {
      int i;

      /* return a list of potential matches */

      strcpy( buffer, "choice" );
      Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj(buffer, -1) );

      for ( i = 0; i < response->num_choices; i++ ) {
	sprintf( buffer, "%s, %s, %s", response->choice[i].city,
		response->choice[i].province, response->choice[i].country );
	Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj(buffer, -1) );
      }
    } 

    tvuFreeGazResponse( response );

  } else {

    /* could not locate the specified address */

    Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj("error", -1) );

  }

  Tcl_SetObjResult(interp, list);
  return TCL_OK;
}

/*
 * t v u t i l s A D L ()
 *
 * Entry point for TCL to access the ADL gazetteer facility of tvutils.
 * That is, to translate a feature name to a lat/long location. This
 * returns a list of strings as a result, these can be:
 *
 * error:     [ "error" ] or []
 * success:   [ "<matches>", "<total-matches>", 
 *              [ "<name>", "<type">, "<latitude>", "<longitude>" ],
 *              [ "<name>", "<type">, "<latitude>", "<longitude>" ],
 *              ... ]
 *
 */

static int
tvutilsADL( ClientData clientdata, Tcl_Interp *interp, int objc,
	    Tcl_Obj *const objv[] )
{
  char           *adl_name, *adl_type;
  tvuADLResponse *response;
  Tcl_Obj        *list = Tcl_NewListObj( 0, NULL );
  int            i;

  if ( objc < 4 ) {
    Tcl_SetObjResult(interp, list);
    return TCL_ERROR;
  }

  adl_name = Tcl_GetStringFromObj( objv[2], NULL );
  adl_type = Tcl_GetStringFromObj( objv[3], NULL );

  response = tvuADLQuery( adl_name, adl_type, NULL, -1 );

  if ( response != NULL ){
    Tcl_Obj *x;

    x = Tcl_NewIntObj( response->matches );
    Tcl_ListObjAppendElement( interp, list, x );

    x = Tcl_NewIntObj( response->total_matches );
    Tcl_ListObjAppendElement( interp, list, x );

    for ( i = 0; i < response->matches; i++ ) {
      double  coord;
      char    buffer[2048];
      Tcl_Obj *match;

      match = Tcl_NewListObj( 0, NULL );
      
      sprintf( buffer, "\"%s\"", response->title[i] );
      Tcl_ListObjAppendElement( interp, match, Tcl_NewStringObj(buffer, -1) );
      sprintf( buffer, "\"%s\"", response->type[i] );
      Tcl_ListObjAppendElement( interp, match, Tcl_NewStringObj(buffer, -1) );

      coord = response->south[i] + 
	( response->north[i] - response->south[i] ) / 2.0;
      Tcl_ListObjAppendElement( interp, match, Tcl_NewDoubleObj( coord ) );

      coord = response->west[i] + 
	( response->east[i] - response->west[i] ) / 2.0;
      Tcl_ListObjAppendElement( interp, match, Tcl_NewDoubleObj( coord ) );

      Tcl_ListObjAppendList( interp, list, match );
    }

    tvuADLFreeResponse( response );

  } else {

    /* could not locate the specified address */

    Tcl_ListObjAppendElement( interp, list, Tcl_NewStringObj("error", -1) );

  }

  Tcl_SetObjResult(interp, list);
  return TCL_OK;
}

/*
 * t v u t i l s B o o k m a r k s ()
 *
 * Entry point for TCL to load or save the viewpoint bookmark file
 * Called from the main tvutils interface, tvutilsInterface().
 *
 */

static int
tvutilsBookmarks( ClientData clientData, Tcl_Interp *interp, int objc,
		  Tcl_Obj *const objv[])
{
  char* cmd;

  if (objc < 3) return TCL_ERROR;

  cmd = Tcl_GetStringFromObj(objv[2], NULL);

  if (!strcmp(cmd, "open")) {
    tvuLoadViewpoints();
  }

  else if (!strcmp(cmd, "write")) {
    tvuSaveViewpoints();
  }

  else if (!strcmp(cmd, "size")) {
    Tcl_Obj* retObj = Tcl_NewObj();
    int size = tvuViewpointCount();
    Tcl_SetIntObj(retObj, size);
    Tcl_SetObjResult(interp, retObj);
  }

  else if (!strcmp(cmd, "entry")) {
    ViewpointItem *item;
    char          *opt1;
    Tcl_Obj       *listObj;
    Tcl_Obj       *list2Obj;
    Tcl_Obj       *obj;
 
    if ( objc < 3 ) return TCL_ERROR;

    opt1 = Tcl_GetStringFromObj(objv[3], NULL);
    if ( ( item = tvuViewpoint( atoi( opt1 ) ) ) == NULL )
      return TCL_ERROR;

    /* create the list to be returned */

    listObj = Tcl_NewListObj( 0, NULL );

    /* append the dataset name to the list */

    obj = Tcl_NewObj();
    Tcl_SetStringObj(obj, (char *) item->dataset, -1);
    Tcl_ListObjAppendElement( interp, listObj, obj );

    /* append the bookmark name to the list */
    
    obj = Tcl_NewObj();
    Tcl_SetStringObj(obj, (char *) item->name, -1);
    Tcl_ListObjAppendElement( interp, listObj, obj );
    
    /* append a list containing the location (x,y,z) to the list */
    
    list2Obj = Tcl_NewListObj( 0, NULL );
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->x);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->y);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->z);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    Tcl_ListObjAppendElement( interp, listObj, list2Obj );
    
    /* append a list containing the (yaw, pitch, roll) to the list */
    
    list2Obj = Tcl_NewListObj( 0, NULL );
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->yaw);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->pitch);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    obj = Tcl_NewObj();
    Tcl_SetDoubleObj(obj, item->roll);
    Tcl_ListObjAppendElement( interp, list2Obj, obj );
    
    Tcl_ListObjAppendElement( interp, listObj, list2Obj );
    
    /* append the coordsys designator */
    
    obj = Tcl_NewObj();
    Tcl_SetIntObj(obj, item->coordsys);
    Tcl_ListObjAppendElement( interp, listObj, obj );
    
    /* return [$name $dataset [$x $y $z] [$yaw $pitch $roll] $coordsys] */
    
    Tcl_SetObjResult(interp, listObj);
    return TCL_OK;

  } else if (!strcmp(cmd, "add")) {
    ViewpointItem *item;
    char          *string;
    Tcl_Obj       *listObj, *list2Obj, *obj;
    double        yaw = 0.0, pitch = 0.0, roll = 0.0;

    if ( objc < 3 )
      return TCL_ERROR;

    if ( ( item = tsmMemCalloc( sizeof(ViewpointItem), 1 ) ) == NULL )
      return TCL_ERROR;

    listObj = objv[3];

    /* get the dataset name */

    Tcl_ListObjIndex( interp, listObj, 0, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      item->dataset = tsmMemStrdup( string );

    /* get the bookmark name */

    Tcl_ListObjIndex( interp, listObj, 1, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      item->name = tsmMemStrdup( string );

    /* get the location list */

    Tcl_ListObjIndex( interp, listObj, 2, &list2Obj );

    Tcl_ListObjIndex( interp, list2Obj, 0, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &item->x );

    Tcl_ListObjIndex( interp, list2Obj, 1, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &item->y );

    Tcl_ListObjIndex( interp, list2Obj, 2, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &item->z );

    /* get the orientation list */

    Tcl_ListObjIndex( interp, listObj, 3, &list2Obj );

    Tcl_ListObjIndex( interp, list2Obj, 0, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &yaw );

    Tcl_ListObjIndex( interp, list2Obj, 1, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &pitch );

    Tcl_ListObjIndex( interp, list2Obj, 2, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetDouble( interp, string, &roll );

    item->yaw   = (float) yaw;
    item->pitch = (float) pitch;
    item->roll  = (float) roll;

    /* get the coordsys designator */

    Tcl_ListObjIndex( interp, listObj, 4, &obj );
    if ( ( string = Tcl_GetStringFromObj( obj, NULL ) ) )
      Tcl_GetInt( interp, string, &item->coordsys );

    /* add this new bookmark */

    tvuAddViewpoint( item );

  } else if (!strcmp(cmd, "remove")) {
    char          *name, *dataset;

    if ( objc < 4 )
      return TCL_ERROR;

    /* get the dataset name and the bookmark name */

    dataset = Tcl_GetStringFromObj( objv[3], NULL );
    name    = Tcl_GetStringFromObj( objv[4], NULL );

    /* remove this bookmark */

    tvuDeleteViewpoint( dataset, name );

  }

  return TCL_OK;
}

/*
 * This following routines are used to maintain a list of
 * events to send to the Tcl front end. There is an idle
 * event callback on the Tcl side that asks us every noe
 * and then what messages we have pending. We define 
 * each message with a mode type, where this can be one
 * of the following strings:
 *
 *    NONE        = no message
 *    STATUS      = update the status bar
 *    MESSAGE     = display a message 
 *    ERROR       = display an error
 *    BUSY        = dataset loading display
 *    CONSOLE     = message to output directly to the dataset console
 *    TERRAIN_ADD = notify that a terrain model has been loaded by .geo
 *    MODEL_ADD   = notify that a 3D model has been loaded by .geo
 *    TERRAIN_DEL = notify that a terrain model has been unloaded by .geo
 *    MODEL_DEL   = notify that a 3D model has been unloaded by .geo
 *
 * The list is queried on the Tcl side by calling the
 * "tvutils msg" command.
 *
 */

typedef struct {
  char *mode; /* message type, see above for possible values */
  char *id;   /* Togl ID string, e.g. .tvinst1 */
  char *msg;  /* Message string */
  char *opt;  /* Optional addendum string */
} MsgElem;

static list_t *msgElemList = NULL;

void
tvMsgListClear( void )
{
  msgElemList = list_create( TRUE );
}

int
tvMsgListAdd( char const *mode, char const *id,
	      char const *msg, char const *opt )
{
  MsgElem *elem = calloc( 1, sizeof( MsgElem ) );
  if ( ! elem ) return FALSE;
  elem->mode = strdup( ( mode ) ? mode : "" );
  elem->id   = strdup( ( id ) ? id : "" );
  elem->msg  = strdup( ( msg ) ? msg : "" );
  elem->opt  = strdup( ( opt ) ? opt : "" );
  list_shift( msgElemList, elem );
  return TRUE;
}

static void
tvMsgElemFree( MsgElem *elem )
{
  if ( ! elem ) return;
  if ( elem->mode ) free( elem->mode );
  if ( elem->id ) free( elem->id );
  if ( elem->msg ) free( elem->msg );
  if ( elem->opt ) free( elem->opt );
  free( elem );
}

/* a specific wrapper to let anyone send messages to the console */

void
tvConsoleMsg( char const *fmt, ... )
{
  char *buff = malloc( 10 * 1024 );
  if ( buff ) {
    va_list ap;
    va_start (ap, fmt);
    vsprintf (buff, fmt, ap);
    va_end (ap);
    tvMsgListAdd( "CONSOLE", "", buff, NULL );
    free( buff );
  }
}

/*
 * t v u t i l s I n t e r f a c e
 *
 * This function is registered as the Tcl object "tvutils". It
 * provides an access point to all of the tvutils library functions.
 * For example, get/set/config/bookmarks.
 *
 */

int
tvutilsInterface(ClientData clientData, Tcl_Interp *interp, int objc,
		 Tcl_Obj *const objv[])
{
  Tcl_HashEntry *entry;
  int           index, subindex = 0;
  Tcl_Obj       *retObj;
  char          *cmd, *option;

  if (objc < 2) return TCL_ERROR;

  cmd = Tcl_GetStringFromObj( objv[1], NULL );

  if ( strcmp( cmd, "bookmarks" ) == 0 ) {

    if (objc < 3) return TCL_ERROR;
    return tvutilsBookmarks( clientData, interp, objc, objv );

  } else if ( strcmp( cmd, "gazetteer" ) == 0 ) {

    if (objc < 3) return TCL_ERROR;
    return tvutilsGaz( clientData, interp, objc, objv );

  } else if ( strcmp( cmd, "adl" ) == 0 ) {

    if (objc < 3) return TCL_ERROR;
    return tvutilsADL( clientData, interp, objc, objv );

  } else if ( strcmp( cmd, "config" ) == 0 ) {

    if (objc < 3) return TCL_ERROR;
    option = Tcl_GetStringFromObj( objv[2], NULL );

    if ( strcmp( option, "load" ) == 0 ) {
      tvuLoadConfig();
    } else if ( strcmp( option, "save" ) == 0 ) {
      tvuSaveConfig();
    } else if ( strcmp( option, "add1" ) == 0 ) {
      char *opt = Tcl_GetStringFromObj( objv[3], NULL );
      char *val = Tcl_GetStringFromObj( objv[4], NULL );
      tvuAddConfig1( opt, val );
    } else if ( strcmp( option, "add2" ) == 0 ) {
      char *opt = Tcl_GetStringFromObj( objv[3], NULL );
      char *val1 = Tcl_GetStringFromObj( objv[4], NULL );
      char *val2 = Tcl_GetStringFromObj( objv[4], NULL );
      tvuAddConfig2( opt, val1, val2 );
    } else 
      return TCL_ERROR;
    
  } else if ( strcmp( cmd, "get" ) == 0 ) {
  
    /* lookup the keyword in our hashtable... */

    if (objc < 3) return TCL_ERROR;
    option = Tcl_GetStringFromObj( objv[2], NULL );
    entry = Tcl_FindHashEntry(&tvuVarsTable, option);
    if (!entry) return TCL_ERROR;

    /* get the index into the secondary table... */

    index = (int)Tcl_GetHashValue(entry);

    retObj = Tcl_NewObj();
    switch (tvuVarsData[index].type) {
    case TV_BOOLEAN:
    case TV_INT:
      Tcl_SetIntObj(retObj, tvuGetInt(tvuVarsData[index].key));
      break;

    case TV_FLOAT:
      Tcl_SetDoubleObj(retObj, (double)tvuGetFloat(tvuVarsData[index].key));
      break;

    case TV_STRING:
      Tcl_SetStringObj(retObj, tvuGetString(tvuVarsData[index].key), -1);
      break;

    case TV_STRINGV:
      if (objc < 4) return TCL_ERROR;
      Tcl_GetIntFromObj( interp, objv[3], &subindex );
      Tcl_SetStringObj(retObj, tvuGetStringN(tvuVarsData[index].key,
					     subindex), -1);
      break;
	
    default:
      return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, retObj);

  } else if ( strcmp( cmd, "set" ) == 0 ) {
  
    int    argi;
    double argd;
    char   *args;

    /* lookup the keyword in our hashtable... */

    if (objc < 3) return TCL_ERROR;
    option = Tcl_GetStringFromObj( objv[2], NULL );
    entry = Tcl_FindHashEntry(&tvuVarsTable, option);
    if (!entry) return TCL_ERROR;

    /* get the index into the secondary table... */

    index = (int)Tcl_GetHashValue(entry);

    switch (tvuVarsData[index].type) {
    case TV_BOOLEAN:
    case TV_INT:
      if ( Tcl_GetIntFromObj(interp, objv[3], &argi) == TCL_ERROR )
	return TCL_ERROR;
      tvuSetInt( tvuVarsData[index].key, argi );
      break;

    case TV_FLOAT:
      if ( Tcl_GetDoubleFromObj(interp, objv[3], &argd) == TCL_ERROR )
	return TCL_ERROR;
      tvuSetFloat( tvuVarsData[index].key, (float)argd );
      break;

    case TV_STRING:
      args = Tcl_GetStringFromObj( objv[3], NULL);
      tvuSetString( tvuVarsData[index].key, args );
      break;
	
    default:
      return TCL_ERROR;
    }

  } else if ( strcmp( cmd, "msg" ) == 0 ) {
    Tcl_Obj  *string;

    if ( msgElemList == NULL ) return TCL_ERROR;

    retObj = Tcl_NewListObj( 0, NULL );
    if ( list_empty( msgElemList ) ) {
      /* return the "no message" result  */
      string = Tcl_NewObj();
      Tcl_SetStringObj( string, "NONE", -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );
      string = Tcl_NewObj();
      Tcl_SetStringObj( string, "", -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );
      Tcl_ListObjAppendElement( interp, retObj, string );
      Tcl_ListObjAppendElement( interp, retObj, string );

    } else {
      /* return the top message on the stack */
      MsgElem *elem = list_pop( msgElemList );
      if ( elem == NULL ) return TCL_ERROR;

      string = Tcl_NewObj();
      Tcl_SetStringObj( string, elem->mode, -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );

      string = Tcl_NewObj();
      Tcl_SetStringObj( string, elem->id, -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );

      string = Tcl_NewObj();
      Tcl_SetStringObj( string, elem->msg, -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );

      string = Tcl_NewObj();
      Tcl_SetStringObj( string, elem->opt, -1 );
      Tcl_ListObjAppendElement( interp, retObj, string );

      tvMsgElemFree( elem );      
    }

    Tcl_SetObjResult( interp, retObj );
    return TCL_OK;
    
  } else
    return TCL_ERROR;

  return TCL_OK;
}

/*
 * t v L o c a l M e s s a g e H a n d l e r ( )
 *
 * Registered with libtv2 to handle messages reported back from the library
 * at runtime.
 *
 */
static void
tvlibMessageHandler(void *closure, tvMessageType type, char const *format, 
    va_list ap)
{
    TvBasePacket* tvPacket = (TvBasePacket*)closure;
    Tcl_Interp* interp = Togl_Interp(tvPacket->togl);
    char buff[1024], *buffptr;
    int BUFFSIZE = 1024;
    char *ident, *instID, *suffix;

    if ( format == NULL ) format = "";

    if ( strchr( format, '%' ) == NULL ) {
      /* no printf style formatting - don't need to use vsprintf() */
      /* we want to avoid this as it means guessing at how big a   */
      /* string buffer we need (if we don't have vsnprintf()).     */
      buffptr = (char *) format;
    } else {
#ifdef _MSVC
      /* don't seem to have vsnprintf() under MSVC on Windows */
      vsprintf (buff, format, ap);
#else
      vsnprintf (buff, BUFFSIZE, format, ap);
#endif
      buffptr = buff;
    }
    
    /* need to pull out the instance ID.  The "ident" string is of
     * the form ".tvinstX.tvbase", where X is an integer. We need
     * to pull off the ".tvbase" suffix.
     */

    ident = Togl_Ident(tvPacket->togl);
    instID = strdup( ( ident ) ? ident : "" );
    suffix = strstr( instID, ".tvbase" );
    if (suffix) *suffix = '\0';

    /* now store up the appropriate message for Tcl to query */

    switch (type) {
    case TV_ERROR_MSG: 
      tvMsgListAdd( "ERROR", instID, buffptr, NULL );
      break; 

    case TV_STATUS_MSG:
      tvMsgListAdd( "STATUS", instID, buffptr, NULL );
      break; 

    case TV_MESSAGE_MSG: 
      tvMsgListAdd( "MESSAGE", instID, buffptr, NULL );
      break; 

    case TV_BUSY_MSG:
      tvMsgListAdd( "BUSY", instID, buffptr, NULL );
      break;

    case TV_CONSOLE_MSG:
      tvMsgListAdd( "CONSOLE", instID, buffptr, NULL );
      break;

    case TV_TERRAIN_ADD_MSG: 
      /* this can only be a GeoWeb discovered datasets */
      /* in which case, we need to register it with us */
      if ( tvRegisterGeoWebTerrain( interp, buffptr ) )
        tvMsgListAdd( "TERRAIN_ADD", instID, buffptr, NULL );
      break; 

    case TV_MODEL_ADD_MSG: 
      tvMsgListAdd( "MODEL_ADD", instID, buffptr, NULL );
      break; 

    case TV_TERRAIN_DEL_MSG: 
      tvMsgListAdd( "TERRAIN_DEL", instID, buffptr, NULL );
      break; 

    case TV_MODEL_DEL_MSG: 
      tvMsgListAdd( "MODEL_DEL", instID, buffptr, NULL );
      break; 

    }

    /* free the ID string that we allocated */
    free( instID );
}

