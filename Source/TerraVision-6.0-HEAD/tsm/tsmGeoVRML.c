/*
 * Name: tsmGeoVRML.c
 *
 * Description: 
 *   This file contains functions to converts TerraVision datasets from DEM
 *   and OI format into GeoVRML 1.0 compliant files.  The output of this
 *   utility includes .wrl or .wrz files that contains GeoLOD, GeoElevationGrid,
 *   and GeoViewpoint nodes.
 *
 *   The utility will put the generated files into a user-specified
 *   directory.  Inside this directory, two subdirectories will be create:
 *   "trees" and "images".  Inside each of these two directories there are
 *   more subdirectories that group files in the same level of detail
 *   together.  As the names suggest, all VRML files will be put under the
 *   "trees" directory, and all the texture image files are put under the
 *   "images" directory.
 *
 *   Under the "trees" and "images" subdirectories, there will be further
 *   subdirectories that group files of different level of detail together,
 *   starting from "0", until the maximum level of detail.  In this context,
 *   0 is the lowest resolution, and the higher the number, the higher the
 *   resolution.
 *
 *   The tsmGeoVRML module is written in a way similar to the C++ notion of
 *   class.  In order to use the functions in this module, an instance
 *   of the TsmGeoVRML struct has to be created using the TsmGeoVRML_new
 *   function.  After the structure is successfully created, under normal
 *   circumstances, you would call the TsmGeoVRML_writeAllTiles function,
 *   and all the hassles of generating the VRML and texture image files
 *   would be handled for you.  However, before terminating the program,
 *   TsmGeoVRML_destroy has to be called.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmGeoVRML.c,v 1.12.2.11 2002/07/30 21:51:28 reddy Exp $
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
#include "tsmConnectionParamsInt.h"
#include "tsmVRMLUtils.h"
#include "tsmGeoVRML.h"
#include "tsmTileCoord.h"
#include "tsmGeoCoordBuffer.h"
#include "tsmStringBuffer.h"

#include <stdio.h>
#include <prnetdb.h>
/*#include <sys/stat.h>*/
#include <limits.h>
#include <float.h>
#include <vrml2/vrml2.h>
#include <vrml2/c_nodes.h>
#include <zlib/zlib.h>

#define HELP    "http://www.tvgeo.com/tsmApi/examples/Docs/make_geovrml.html"
#define APP_VERSION tsmCvsToString( "$Revision: 1.12.2.11 $" )

/* Constants used to calculate range of GeoLOD node */
#define PROJ_F              (double) 2.41421356237309490   /* 1 / tan(45/2) */
#define PROJ_SCR            (double) 512.0             /* min(width,height) */

static void TsmGeoVRML_setNodeMFString0(vsgWorld *wrl, SFNode node, char const *field_name, char const *string);
static void TsmGeoVRML_setNodeMFString(vsgWorld *wrl, SFNode node, char const *field_name, MFString *strings);
static void TsmGeoVRML_setNodeMFNode0(vsgWorld *wrl, SFNode node, const char *field_name, SFNode value);
static void TsmGeoVRML_setNodeMFNode(vsgWorld *wrl, SFNode node, const char *field_name, MFNode *value);
static void TsmGeoVRML_setNodeMFFloat(vsgWorld *wrl, SFNode node, char const *field_name, MFFloat *value);
static void TsmGeoVRML_setNodeMFInt32(vsgWorld *wrl, SFNode node, char const *field_name, MFInt32 *value);
static void TsmGeoVRML_setNodeMFVec2f(vsgWorld *wrl, SFNode node, const char *field_name, MFVec2f *value);
static void TsmGeoVRML_setNodeMFVec3f(vsgWorld *wrl, SFNode node, const char *field_name, MFVec3f *value);
static void TsmGeoVRML_setNodeSFVec3f(vsgWorld *wrl, SFNode node, const char *field_name, SFVec3f value);
static void TsmGeoVRML_setNodeSFNode(vsgWorld *wrl, SFNode node, const char *field_name, SFNode node2);
static void TsmGeoVRML_setNodeSFString(vsgWorld *wrl, SFNode node, char const *field_name, char *string);
static void TsmGeoVRML_setNodeSFFloat(vsgWorld *wrl, SFNode node, const char *field_name, float value);
static void TsmGeoVRML_setNodeSFBool(vsgWorld *wrl, SFNode node, const char *field_name, int value);
static void TsmGeoVRML_setNodeSFInt32(vsgWorld *wrl, SFNode node, const char *field_name, int value);
static void TsmGeoVRML_setNodeSFColor(vsgWorld *wrl, SFNode node, const char *field_name, float r, float g, float b);
static void TsmGeoVRML_setNodeSFRotation(vsgWorld *wrl, SFNode node, const char *field_name, double x, double y, double z, double v);
static void TsmGeoVRML_appendVRML(char **string, char const *vrml);
static void TsmGeoVRML_setNodeMFColor(vsgWorld *wrl, SFNode node, const char *field_name, float r, float g, float b);


double lonOrigin;
double latOrigin;
int touch1 = -1;
int touch2 = -1;
int touch3 = -1;
int noviewFlag = FALSE;
int hudFlag = FALSE;
int maxRangeFlag = FALSE;
int wrzFlag = FALSE;
int whitebgFlag = FALSE;
int numTouchLevels;


/*
 * EXTERNPROTO Locations
    \"urn:web3d:geovrml:1.0/protos/GeoTouchSensor.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoTouchSensor.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/%s.wrl\"\n\
    \"http://www.parallelgraphics.com/vrml/proto/GeoVRML/1.0/GeoViewpoint.wrl\"\n\
 */

static const char *GeoTouchSensorProto = "\
EXTERNPROTO GeoTouchSensor [\n\
  exposedField SFBool enabled \n\
  field SFNode geoOrigin \n\
  field MFString geoSystem \n\
  eventOut SFVec3f hitNormal_changed\n\
  eventOut SFVec3f hitPoint_changed\n\
  eventOut SFVec2f hitTexCoord_changed\n\
  eventOut SFBool isActive\n\
  eventOut SFBool isOver\n\
  eventOut SFTime touchTime\n\
  eventOut SFString hitGeoCoord_changed\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoTouchSensor.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoTouchSensor.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoTouchSensor.wrl\"\n\
]";

static const char *GeoOriginProto = "\
EXTERNPROTO GeoOrigin [\n\
  exposedField  MFString  geoSystem\n\
  exposedField  SFString  geoCoords\n\
  field         SFBool    rotateYUp\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoOrigin.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoOrigin.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoOrigin.wrl\"\n\
]";

static const char *GeoLODProto = "\
EXTERNPROTO GeoLOD [\n\
  field     SFNode   geoOrigin\n\
  field     MFString geoSystem\n\
  field     MFString rootUrl\n\
  field     MFNode   rootNode\n\
  field     MFString child1Url\n\
  field     MFString child2Url\n\
  field     MFString child3Url\n\
  field     MFString child4Url\n\
  field     SFString center\n\
  field     SFFloat  range\n\
  eventOut  MFNode   children\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoLOD.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoLOD.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoLOD.wrl\"\n\
]";

static const char *GeoElevationGridProto = "\
EXTERNPROTO GeoElevationGrid [\n\
  eventIn       MFFloat   set_height\n\
  eventIn       SFFloat   set_yScale\n\
  exposedField  SFNode    color\n\
  exposedField  SFNode    normal\n\
  exposedField  SFNode    texCoord\n\
  field         SFNode    geoOrigin\n\
  field         MFString  geoSystem\n\
  field         SFString  geoGridOrigin\n\
  field         MFFloat   height\n\
  field         SFBool    ccw\n\
  field         SFBool    colorPerVertex\n\
  field         SFFloat   creaseAngle\n\
  field         SFBool    normalPerVertex\n\
  field         SFBool    solid\n\
  field         SFInt32   xDimension\n\
  field         SFString  xSpacing\n\
  field         SFInt32   zDimension\n\
  field         SFString  zSpacing\n\
  field         SFFloat   yScale\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoElevationGrid.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoElevationGrid.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoElevationGrid.wrl\"\n\
]";

static const char *GeoViewpointProto = "\
EXTERNPROTO GeoViewpoint [\n\
  eventIn       SFBool      set_bind\n\
  eventIn       SFString    set_position\n\
  eventIn       SFString    set_orientation\n\
  field         SFNode      geoOrigin\n\
  field         MFString    geoSystem\n\
  exposedField  SFFloat     fieldOfView\n\
  exposedField  SFBool      jump\n\
  field         SFRotation  orientation\n\
  field         SFString    position\n\
  field         SFString    description\n\
  exposedField  SFBool      headlight\n\
  field         SFFloat     speed\n\
  exposedField  MFString    navType\n\
  eventOut      SFTime      bindTime\n\
  eventOut      SFBool      isBound\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoViewpoint.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoViewpoint.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoViewpoint.wrl\"\n\
]";

static const char *GeoCoordinateProto = "\
EXTERNPROTO GeoCoordinate [\n\
   field  SFNode    geoOrigin\n\
   field  MFString  geoSystem\n\
   field  MFString  point\n\
] [ \
    \"urn:web3d:geovrml:1.0/protos/GeoCoordinate.wrl\"\n\
    \"file:///C:/Program\%20Files/GeoVRML/1.0/protos/GeoCoordinate.wrl\"\n\
    \"http://www.geovrml.org/1.0/protos/GeoCoordinate.wrl\"\n\
]";


/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_new                                                         | *
 * | Description:                                                                 | *
 * |   Creates a new instance of TsmGeoVRML object, and sets up connections       | *
 * |   to the DEM and OI dataset.  This function has to be called before          | *
 * |   any other functions can be used.  Caller shoule pass in the parameters     | *
 * |   of the main function in order to parse the command line options.           | *
 * | Parameters:                                                                  | *
 * |   int argc     Number of arguments passed in.                                | *
 * |   char **argv  Array of strings containing the command line parameters.      | *
 * | Return value:                                                                | *
 * |   On successful initialization, the new TsmGeoVRML instance is returned.     | *
 * |   Otherwise NULL is returned.                                                | *
 * +------------------------------------------------------------------------------+ */
TsmGeoVRML *TsmGeoVRML_new(int argc, char **argv) {
  static const char *funcName = "TsmGeoVRML_new";
  TsmGeoVRML *this;
  TsmConnectionParams params;
  char *oiURL = NULL;
  char *demURL = NULL;
  SimpleTileSet *tileset;
  char *levels;

  if ((this = (TsmGeoVRML *) tsmMemMalloc(sizeof(TsmGeoVRML))) == NULL) return NULL;

  tsmParseInitArgs(argc, argv);

  if (tsmGetArg(argc, argv, "-help") == TRUE) {
    puts( "Attempting to display help page using netscape..." );
    tsmShowUrl( HELP );
    exit( 0 );
  }

  if ( argc < 2 ) TsmGeoVRML_usage();

  params = tsmParseReadArgs(argc, argv);
  this->params = tsmParseVRMLArgs(argc, argv);

  CHECK_NOT_NULL(this->params, NULL);

  tsmGetArgString(argc, argv, "-oi", &oiURL);
  if (oiURL == NULL) {
    tsmError(funcName, "Must supply OI URL");
    return NULL;
  }

  tsmGetArgString(argc, argv, "-dem", &demURL);
  if (demURL == NULL) {
    tsmError(funcName, "Must supply DEM URL");
    return NULL;
  }

  if ( tsmGetArgFloat(argc, argv, "-olon", &lonOrigin) == FALSE ) {
    tsmError(funcName, "Must supply -olon <longitude> for geoOrigin");
    return NULL;
  }

  if ( tsmGetArgFloat(argc, argv, "-olat", &latOrigin) == FALSE ) {
    tsmError(funcName, "Must supply -olat <latitude> for geoOrigin");
    return NULL;
  }

  if ( tsmGetArgString(argc, argv, "-touch", &levels) == TRUE ) {
    if (levels == NULL) {
      tsmError(funcName, "Must supply level(s) for GeoTouchSensor");
      return NULL;
    }
    else {
      printf ("\nInserting GeoTouchSensors at TerraVision levels = %s\n\n", levels);
      numTouchLevels = sscanf ( levels, "%d,%d,%d", &touch1, &touch2, &touch3 );
    }
  }


  if ((this->oiConnection = tsmConnect(oiURL, "r", params)) == NULL) {
    tsmError(funcName, "ERROR: couldn't connect to %s\n", oiURL);
    return NULL;
  }

  if ((this->demConnection = tsmConnect(demURL, "r", params)) == NULL) {
    tsmError(funcName, "ERROR: couldn't connect to %s\n", demURL);
    return NULL;
  }

  tsmParseConnectArgs(argc, argv, this->oiConnection);
  tsmParseConnectArgs(argc, argv, this->demConnection);

  this->oiPyramid = (Pyramid *) tsmGet(this->oiConnection, TSM_DATASET_TSPEC);
  this->demPyramid = (Pyramid *) tsmGet(this->demConnection, TSM_DATASET_TSPEC);

 
  if (this->oiPyramid->projectionTspec.projectionType != this->demPyramid->projectionTspec.projectionType ||
      this->oiPyramid->projectionTspec.projectionType == tspAffineProjection) {
    tsmError(funcName, "Dataset has unsupported projection type");
    return NULL;
  }

  tsmVRMLCacheInit();

  /* Initialize VRML library */
  vsgcSetup();
  vsgdInit();

  /* Initialize VRML world struct */
  this->wrl = NULL;

  if (tsmGetArg(argc, argv, "-noview") == TRUE) {
    noviewFlag = TRUE;  
  }

  if (tsmGetArg(argc, argv, "-hud") == TRUE) {
    hudFlag = TRUE;  
  }

  if (tsmGetArg(argc, argv, "-maxrange") == TRUE) {
    maxRangeFlag = TRUE;  
  }

  if (tsmGetArg(argc, argv, "-wrz") == TRUE) {
    wrzFlag = TRUE;  
  }

  if (tsmGetArg(argc, argv, "-whitebg") == TRUE) {
    whitebgFlag = TRUE;  
  }

  if (tsmGetArg(argc, argv, "-info") == TRUE) {
    tsmDescribeConnection(this->oiConnection);
  }

  this->ferror = NULL;
  /*
  this->ferror = fopen("error.txt", "wb");
  if (this->ferror == NULL) {
    printf("Couldn't create error file!\n");
  }
  */

  /* Calculate maximum u and v */
  tileset = &this->demPyramid->tspecLevel.val[this->demPyramid->minLevel];
  TsmGeoVRML_ixiy2uv(this->demPyramid, this->demPyramid->minLevel,
    tileset->maxValidPixel[0], tileset->maxValidPixel[1], &this->maxu, &this->maxv);

  return this;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_destroy                                                     | *
 * | Description:                                                                 | *
 * |   Destroy the TsmGeoVRML instance passed in, and set the instance's pointer  | *
 * |   to NULL.  Also, it will disconnect all the connections to DEM and OI       | *
 * |   datasets.                                                                  | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML **this   Reference to TsmGeoVRML instance pointer.              | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_destroy(TsmGeoVRML **this) {

  if ( this == NULL || *this == NULL ) return;

  if ( (*this)->ferror ) fclose((*this)->ferror);

  tsmFreeParams((*this)->params);
  tsmDisconnect((*this)->oiConnection);
  tsmDisconnect((*this)->demConnection);

  tsmVRMLCacheFree();

  tsmMemFree( *this );
  if (this!=NULL) *this = NULL;
}


/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_usage                                                       | *
 * | Description:                                                                 | *
 * |   Prints out some help information about the command line parameters.        | *
 * | Parameters:                                                                  | *
 * |   None.                                                                      | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_usage() {
  printf( "make_geovrml - release %s\n\n", APP_VERSION );
  puts( "usage: make_geovrml [-dem <dem-pyramid>] [-oi <oi-pyramid>] [...]" );
  puts( "\nThis utility copies all the tile and tspec data from a oi and" );
  puts( "associated dem pyramid to a GeoVRML hierarchy of GeoLOD nodes." );
  puts( "\nParameters:" );
  puts( " <oi-pyramid> = URL/filename of the oi pyramid to be read" );
  puts( " <dem-pyramid> = URL/filename of the dem pyramid to use" );
  tsmDescribeArgs( TSM_ARG_READ | TSM_ARG_VRML );
  puts( " -olon <lon> = GDC origin longitude for geoOrigin node" );
  puts( " -olat <lat> = GDC origin latitude for geoOrigin node" );
  puts( " -noview = Do not include GeoViewpoints in the tiles" );
  puts( " -touch <level>s = apply GeoTouchSensors at up to 3 comma separated TV pyramid levels" );
  puts( " -hud = add a Heads Up Display for pos info. Must use -touch <n>" );
  puts( " -maxrange = for use with -touch <n>, sets all LOD ranes to maximum for the tile set" );
  puts( " -wrz = compress tile files and save with .wrz extension" );
  puts( " -whitebg = build files with white background instead of black" );
  puts( " -info = display info for the dataset that was created" );
  puts( " -help = display web documentation for this utility using netscape" );
  puts( "" );
  exit( 0 );
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_progress                                                    | *
 * | Description:                                                                 | *
 * |   Increments progress count by the specified value.  This function is called | *
 * |   whenever a tile is generated.  Supposedly will print something on the      | *
 * |   screen so that the user will know the program is running.                  | *
 * | Parameters:                                                                  | *
 * |   int value    Indicate progress count should increment by how much.         | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_progress(int value) {
  static const char *sequenceChars = "-/|\\";
  static int sequenceCurrent = 0;
  
  sequenceCurrent = (sequenceCurrent + value) % 4;
  putchar(sequenceChars[sequenceCurrent]);
  putchar('\x8');
  fflush(stdout);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_initWorld                                                   | *
 * | Description:                                                                 | *
 * |   Initializes the VRML world structure.  Upon return, the VRML world will    | *
 * |   have the necessary EXTERNPROTO definitions.                                | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this   Reference to the TsmGeoVRML object.                     | *
 * | Return value:                                                                | *
 * |   vsgWorld *         Reference to the VRML world structure.                  | *
 * +------------------------------------------------------------------------------+ */
vsgWorld *TsmGeoVRML_initWorld(TsmGeoVRML *this) {
  static const char *funcName = "TsmGeoVRML_initWorld";
  char *protos = NULL;

  CHECK_NOT_NULL(this, NULL);

  if (this->wrl != NULL) vsgDestroyWorld(this->wrl);
  this->wrl = vsgNewWorld(0);
  vsgStartWorld(this->wrl);
  vsgSetFiletype(this->wrl, "#VRML V2.0 utf8");

  /* Parse GeoLOD and GeoElevationGrid EXTERNPROTOs */
  TsmGeoVRML_appendVRML(&protos, GeoTouchSensorProto);
  TsmGeoVRML_appendVRML(&protos, GeoOriginProto);
  TsmGeoVRML_appendVRML(&protos, GeoLODProto);
  TsmGeoVRML_appendVRML(&protos, GeoElevationGridProto);
  if ( ! noviewFlag ) {
    TsmGeoVRML_appendVRML(&protos, GeoViewpointProto);
  }
  if (this->params->test_vtiles) {
    TsmGeoVRML_appendVRML(&protos, GeoCoordinateProto);
  }

  vsgParseString(this->wrl, protos);
  tsmMemFree(protos);

  /* create geoOrigin node  -mpm 10/17/01*/

  if ((this->geoOriginClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoOrigin"))) == NULL) {
    tsmError(funcName, "Couldn't create GeoOrigin class");
    vsgDestroyWorld(this->wrl);
    return NULL;
  }


  if ((this->geoLODClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoLOD"))) == NULL) {
    tsmError(funcName, "Couldn't create GeoLOD class");
    vsgDestroyWorld(this->wrl);
    return NULL;
  }
  if ((this->geoElevationGridClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoElevationGrid"))) == NULL) {
    tsmError(funcName, "Couldn't create GeoElevationGrid class");
    vsgDestroyWorld(this->wrl);
    return NULL;
  }
  if ( ! noviewFlag ) {
    if ((this->geoViewpointClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoViewpoint"))) == NULL) {
      tsmError(funcName, "Couldn't create GeoViewpoint class");
      vsgDestroyWorld(this->wrl);
      return NULL;
    }
  }
  if (this->params->test_vtiles) {
    if ((this->geoCoordinateClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoCoordinate"))) == NULL) {
      tsmError(funcName, "Couldn't create GeoCoordinate class");
      vsgDestroyWorld(this->wrl);
      return NULL;
    }
  }
  if ((this->geoTouchSensorClass = vsgGetClass(this->wrl, vsgIntern(this->wrl, "GeoTouchSensor"))) == NULL) {
    tsmError(funcName, "Couldn't create GeoTouchSensor class");
    vsgDestroyWorld(this->wrl);
    return FALSE;
  }

  return this->wrl;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_destroyWorld                                                | *
 * | Description:                                                                 | *
 * |   Destroys the VRML world.  This function is called whenever a tile is       | *
 * |   generated, and we need to move on to the next one.                         | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    Reference to the TsmGeoVRML object.                    | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_destroyWorld(TsmGeoVRML *this) {
  static const char *funcName = "TsmGeoVRML_destroyWorld";
  CHECK_NOT_NULL0(this);
  if (this->wrl != NULL) vsgDestroyWorld(this->wrl);
  this->wrl = NULL;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_oi2DemCoord                                                 | *
 * | Description:                                                                 | *
 * |   Converts coordinate specified in OI space to DEM space.  This function is  | *
 * |   called by the TsmGeoVRML_getElevation function so that the corresponding   | *
 * |   DEM tile can be retrieved, given an OI coordinate.                         | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this      Reference to the TsmGeoVRML object.                  | *
 * |   TileCoord  *oiCoord   OI coordinate.  Only ix and iy is used.              | *
 * |   TileCoord  *demCoord  The calculated DEM coordinate.                       | *
 * | Return value:                                                                | *
 * |   TRUE if successful, FALSE otherwise.                                       | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_oi2DemCoord(TsmGeoVRML *this, TileCoord *oiCoord, TileCoord *demCoord) {
  static const char *funcName = "TsmGeoVRML_oi2DemCoord";
  TileTspec *demTileTspec;
  float *ixiytouv;
  float *uvtoixiy;

  CHECK_NOT_NULL(this, FALSE);

  if ((this->oiPyramid->projectionTspec.projectionType
        != this->demPyramid->projectionTspec.projectionType)
      || (this->oiPyramid->projectionTspec.projectionType==tspAffineProjection)) {
    return FALSE;
  }
  if (oiCoord->level<this->oiPyramid->minLevel || oiCoord->level>this->oiPyramid->maxLevel) {
    return FALSE;
  }

  demCoord->level = this->demPyramid->minLevel;
  ixiytouv = this->oiPyramid->tspecLevel.val[oiCoord->level].imageTransformationTspec.ixiytoUV;
  uvtoixiy = this->demPyramid->tspecLevel.val[demCoord->level].imageTransformationTspec.uvtoIXIY;
  demCoord->u = (oiCoord->ix * ixiytouv[0] + ixiytouv[2]);
  demCoord->v = (oiCoord->iy * ixiytouv[4] + ixiytouv[5]);
  if (demCoord->u > this->maxu) demCoord->u = this->maxu;
  if (demCoord->v > this->maxv) demCoord->v = this->maxv;
  demCoord->ix = demCoord->u * uvtoixiy[0] + uvtoixiy[2];
  demCoord->iy = demCoord->v * uvtoixiy[4] + uvtoixiy[5];

  demTileTspec = &this->demPyramid->tspecLevel.val[demCoord->level].tileTspec;
  
  demCoord->tx = (int) (demCoord->ix / demTileTspec->numPixelsX);
  demCoord->dx = demCoord->ix - (demCoord->tx * demTileTspec->numPixelsX);

  demCoord->ty = (int) (demCoord->iy / demTileTspec->numPixelsY);
  demCoord->dy = demCoord->iy - (demCoord->ty * demTileTspec->numPixelsY);

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getDEMTile                                                  | *
 * | Description:                                                                 | *
 * |   Returns the DEM tile specified by the given DEM coordinate.                | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this      Reference to the TsmGeoVRML object.                  | *
 * |   TileCoord *tileCoord  The DEM coordinate.                                  | *
 * | Return value:                                                                | *
 * |   The DEM buffer.                                                            | *
 * +------------------------------------------------------------------------------+ */
uchar *TsmGeoVRML_getDEMTile(TsmGeoVRML *this, TileCoord *tileCoord) {
  static const char *funcName = "TsmGeoVRML_getDEMTile";
  TsmCachedTile *tile;
  SimpleTileSet *tileset;
  uchar *buffer;

  CHECK_NOT_NULL(this, NULL);
  CHECK_NOT_NULL(tileCoord, NULL);

  tileset = &this->demPyramid->tspecLevel.val[tileCoord->level];
  if (tileCoord->tx > tileset->maxTile[0] || tileCoord->ty > tileset->maxTile[1]) {
    return NULL;
  }

  if ((tile = tsmVRMLCacheGetBuffer(tileCoord->tx, tileCoord->ty, tileCoord->level)) != NULL)
    return tile->buffer;

  if ((tile = tsmMemMalloc(sizeof(TsmCachedTile))) == NULL) {
    tsmError(funcName, "Couldn't allocate tile buffer");
    return NULL;
  }

  if ((buffer = tsmAllocTileBuffer(this->demConnection)) == NULL) {
    tsmError(funcName, "Couldn't allocate tile buffer");
    return NULL;
  }

  if (!tsmReadTile(this->demConnection, tileCoord->tx, tileCoord->ty, tileCoord->level, buffer)) {
    tsmError( funcName, "Couldn't get the tile!" );
    return NULL;
  }

  /* Add this tile to the cache and return it */
  tile->tx      = tileCoord->tx;
  tile->ty      = tileCoord->ty;
  tile->level   = tileCoord->level;
  tile->size[0] = 0.0;
  tile->buffer  = buffer;
  tsmVRMLCacheAddBBox(tile);

  return buffer;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getElevationDEM                                             | *
 * | Description:                                                                 | *
 * |   Gets elevation from DEM buffer.  This function doesn't perform any         | *
 * |   interpolation.  Instead it truncates the decimal part of the coordinates   | *
 * |   and return the elevation at that pixel in the DEM buffer.                  | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *oiCoord  OI coordinate.  This coordinate is converted to the    | *
 * |                       corresponding DEM coordinate before determining the    | *
 * |                       elevation.                                             | *
 * | Return value:                                                                | *
 * |   The elevation.                                                             | *
 * +------------------------------------------------------------------------------+ */
double TsmGeoVRML_getElevationDEM(TsmGeoVRML *this, TileCoord *oiCoord) {
  static const char *funcName = "TsmGeoVRML_getElevationDEM";
  uchar *buffer;
  TileCoord demCoord;
  double height;

  CHECK_NOT_NULL(this, 0);
  CHECK_NOT_NULL(oiCoord, 0);

  if (TsmGeoVRML_oi2DemCoord(this, oiCoord, &demCoord) == FALSE) {
    return 0;
  }

  if ((buffer = TsmGeoVRML_getDEMTile(this, &demCoord)) == NULL) {
    /*    fprintf(this->ferror, "Couldn't get tile (%f, %f, %d, %d, %d)::(%f, %f, %d, %d, %d)\n",
      oiCoord->ix, oiCoord->iy, oiCoord->tx, oiCoord->ty, oiCoord->level,
      demCoord.ix, demCoord.iy, demCoord.tx, demCoord.ty, demCoord.level); */
    tsmError(funcName, "Couldn't get the tile buffer (%f, %f, %d)", oiCoord->ix, oiCoord->iy, oiCoord->level);
    return 0;
  }

  height = PR_ntohl(((long*) buffer)[((int)demCoord.dy)
    * (this->demPyramid->tileTspec.totalPixels[0]) + ((int)demCoord.dx)]);
  height = height * this->demPyramid->pixelTspec.scale + this->demPyramid->pixelTspec.offset;
  return height;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getElevation                                                | *
 * | Description:                                                                 | *
 * |   Returns the elevation at the specified OI coordinate.  Linear              | *
 * |   interpolation on four neighbouring points to determine the approximate     | *
 * |   elevation at the specified coordinate.  Calls TsmGeoVRML_getElevationDEM   | *
 * |   to get elevation from DEM buffer.                                          | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *oiCoord  The OI coordinate at which the elevation is determined.| *
 * | Return value:                                                                | *
 * |   The elevation.                                                             | *
 * +------------------------------------------------------------------------------+ */
double TsmGeoVRML_getElevation(TsmGeoVRML *this, TileCoord *oiCoord) {
  static const char *funcName = "TsmGeoVRML_getElevation";
  double pxf, pyf;
  double h[4];
  long hy1, hy2;

  TileCoord *coord, *demCoord;
  SimpleTileSet *tileset;

  CHECK_NOT_NULL(this, 0);
  CHECK_NOT_NULL(oiCoord, 0);

  tileset = &this->oiPyramid->tspecLevel.val[oiCoord->level];

  /*
   *  2---3
   *  |   |
   *  0---1
   */
  coord = TileCoord_new1(oiCoord);
  h[0] = TsmGeoVRML_getElevationDEM(this, coord);

  return h[0];

  coord->ix = oiCoord->ix+1;
  if (coord->ix <= tileset->maxValidPixel[0]) h[1] = TsmGeoVRML_getElevationDEM(this, coord);
  else h[1] = 0;

  coord->ix = oiCoord->ix;
  coord->iy = oiCoord->iy+1;
  if (coord->iy <= tileset->maxValidPixel[1]) h[2] = TsmGeoVRML_getElevationDEM(this, coord);
  else h[2] = 0;

  coord->ix = oiCoord->ix+1;
  if (coord->ix <= tileset->maxValidPixel[0] && coord->iy <= tileset->maxValidPixel[1])
    h[3] = TsmGeoVRML_getElevationDEM(this, coord);
  else h[3] = 0;

  demCoord = TileCoord_new();
  TsmGeoVRML_oi2DemCoord(this, oiCoord, demCoord);
  pxf = demCoord->dx - (int)demCoord->dx;
  pyf = demCoord->dy - (int)demCoord->dy;

  hy1 = (h[2]-h[0]) * pyf + h[0];
  hy2 = (h[3]-h[1]) * pyf + h[1];

  TileCoord_destroy(&coord);
  TileCoord_destroy(&demCoord);

  return (hy2-hy1) * pxf + hy1;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setGeoSystem                                                | *
 * | Description:                                                                 | *
 * |   Convenience function to the the geoSystem field of a GeoVRML node.  The    | *
 * |   value in the Pyramid structure is used to set the field value.             | *
 * | Parameters:                                                                  | *
 * |   Pyramid  *pyramid    The Pyramid structure.                                | *
 * |   vsgWorld *wrl        The VRML world.                                       | *
 * |   SFNode   node        The node which we need the geoSystem field set.       | *
 * | Return value:                                                                | *
 * |   TRUE if successful, FALSE otherwise.                                       | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_setGeoSystem(Pyramid *pyramid, vsgWorld *wrl, SFNode node) {
  const char *funcName = "TsmGeoVRML_setGeoSystem";
  MFString *geoSystem = NULL;
  char buffer[256];
  symbol_t field;
  vsgEventInDesc evin;

  CHECK_NOT_NULL(pyramid, FALSE);
  CHECK_NOT_NULL(wrl, FALSE);
  CHECK_NOT_NULL(node, FALSE);

  if ((field = vsgIntern(wrl, "geoSystem")) == NULL) {
    tsmError(funcName, "Couldn't find field: geoSystem");
    return FALSE;
  }

  if ((geoSystem = vsgMakeMFString(wrl)) == NULL) {
    tsmError(funcName, "Couldn't create MFString structure");
    return FALSE;
  }
  if (pyramid->projectionTspec.projectionType == tspUtmProjection) {
    vsgAppendSFString(wrl, geoSystem, vsgStrdup(wrl, "UTM"));
    sprintf(buffer, "Z%d", pyramid->projectionTspec.utmZoneNumber);
    vsgAppendSFString(wrl, geoSystem, vsgStrdup(wrl, buffer));
  }
  else if (pyramid->projectionTspec.projectionType == tspLatLongProjection) {
    vsgAppendSFString(wrl, geoSystem, vsgStrdup(wrl, "GDC"));
  }
  else return FALSE;

  evin = vsgSetFunc(node, field);
  vsgSetMFString(wrl, node, evin, geoSystem);

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getProjectionName                                           | *
 * | Description:                                                                 | *
 * |   Returns the projection type associated with the given Pyramid in string.   | *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid      The Pyramid structure.                               | *
 * |   char    *buffer       Buffer to hold the projection type.                  | *
 * | Return value:                                                                | *
 * |   On success, returns a reference to the buffer (same as the parameter       | *
 * |   buffer).  Otherwise, returns NULL.                                         | *
 * +------------------------------------------------------------------------------+ */
char *TsmGeoVRML_getProjectionName(Pyramid *pyramid, char *buffer) {
  if (pyramid->projectionTspec.projectionType==tspUtmProjection) strcpy(buffer, "UTM");
  else if (pyramid->projectionTspec.projectionType==tspLatLongProjection) strcpy(buffer, "GDC");
  else return NULL;
  return buffer;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getOrigin                                                   | *
 * | Description:                                                                 | *
 * |   Gets the origin of a give tile, specified by coord.                        | *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid      The Pyramid structure.                               | *
 * |   TileCoord *coord      Which tile are we talking about.  Only the tx and ty | *
 * |                         fields are used.                                     | *
 * |   char *buffer          Buffer to hold the geo-referenced coordinate.        | *
 * | Return value:                                                                | *
 * |   On success, returns a reference to the buffer, NULL otherwise.             | *
 * +------------------------------------------------------------------------------+ */
char *TsmGeoVRML_getOrigin(Pyramid *pyramid, TileCoord *coord, char *buffer) {
  const char *funcName = "TsmGeoVRML_getOrigin";
  SimpleTileSet *tileset = &pyramid->tspecLevel.val[coord->level];
  TileTspec *tile = &pyramid->tileTspec;
  float *ixiytouv = tileset->imageTransformationTspec.ixiytoUV;

  if (pyramid->projectionTspec.projectionType == tspUtmProjection
      || pyramid->projectionTspec.projectionType == tspLatLongProjection) {
    sprintf(buffer, "%f %f 0", (coord->ty * tile->numPixelsY * ixiytouv[4]) + ixiytouv[5],
      (coord->tx * tile->numPixelsX * ixiytouv[0]) + ixiytouv[2]);
  }
  else {
    tsmError(funcName, "Projection type not supported");
    return NULL;
  }

  return buffer;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getCenter                                                   | *
 * | Description:                                                                 | *
 * |   Determines the ceter coordinate of the specified tile.                     | *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid     The Pyramid structure.                                | *
 * |   TileCoord *coord     The tile we are talking about.  Only the tx and ty    | *
 * |                        fields are used.                                      | *
 * |   double *u, *v,       Geo-referenced coordinate of the center.              | *
 * |     *elevation                                                               | *
 * | Return value:                                                                | *
 * |   On success, returns reference to the buffer, NULL otherwise.               | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_getCenter(Pyramid *pyramid, double *ll, double *ur, double *u, double*v, double *elevation) {
  static const char *funcName = "TsmGeoVMRL_getCenter";
  double x, y, z;

  if (pyramid->projectionTspec.projectionType == tspUtmProjection
      || pyramid->projectionTspec.projectionType == tspLatLongProjection) {
    x = (ur[0] + ll[0]) * 0.5;
    y = (ur[1] + ll[1]) * 0.5;
    z = (ur[2] + ll[2]) * 0.5;
    TsmGeoVRML_gcc2uv(pyramid, x, y, z, u, v, elevation);
  }
  else {
    tsmError(funcName, "Projection type not supported");
    return FALSE;
  }

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createDirectory                                             | *
 * | Description:                                                                 | *
 * |   Prepares the directory needed for the creation of the given filename.  The | *
 * |   filename will be scanned to create all the subdirectories, if they don't   | *
 * |   already exist.                                                             | *
 * | Parameters:                                                                  | *
 * |   char *filename    Path to the file.                                        | *
 * | Return value:                                                                | *
 * |   TRUE if successful, and FALSE otherwise.                                   | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_createDirectory(char *filename) {

  if ( filename == NULL || filename[0] == '\0' )
    return FALSE;

  if ( filename[strlen(filename)-1] == '/' )
    tsmMakeDirIfNotExist( filename, TSM_PATH_ONLY );
  else
    tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createTreeFile                                              | *
 * | Description:                                                                 | *
 * |   Creates a file where the VRML model is to be contained.  The function will | *
 * |   automatically determines the filename from the specified level, and tile   | *
 * |   coordinate.                                                                | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The coordinate of the current tile.  Only level, tx,   | *
 * |                       and ty fields are used.                                | *
 * |   char *filename      Buffer to hold the filename.                           | *
 * | Return value:                                                                | *
 * |   Returns the file handle if everything goes well, NULL otherwise.           | *
 * +------------------------------------------------------------------------------+ */
FILE *TsmGeoVRML_createTreeFile(TsmGeoVRML *this, TileCoord *coord, char *filename) {
  const char *funcName = "TsmGeoVRML_createTreeFile";
  int vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level);
  FILE *fout;
  
  sprintf(filename, "%s/trees/%d/p%dp%d.wrl", this->params->vrml_dir,
     vrmlLevel, coord->tx, coord->ty);
  if (TsmGeoVRML_createDirectory(filename)) {
    if ((fout = fopen(filename, "wb")) == NULL) {
      tsmError(funcName, "Error creating file: %s\n", filename);
      return NULL;
    }
    return fout;
  }

  return NULL;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createCompressedTreeFile                                              | *
 * | Description:                                                                 | *
 * |   Creates a file where the VRML model is to be contained.  The function will | *
 * |   automatically determines the filename from the specified level, and tile   | *
 * |   coordinate.                                                                | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The coordinate of the current tile.  Only level, tx,   | *
 * |                       and ty fields are used.                                | *
 * |   char *filename      Buffer to hold the filename.                           | *
 * | Return value:                                                                | *
 * |   Returns the gz file handle if everything goes well, NULL otherwise.           | *
 * +------------------------------------------------------------------------------+ */
gzFile *TsmGeoVRML_createCompressedTreeFile(TsmGeoVRML *this, TileCoord *coord, char *filename) {
  const char *funcName = "TsmGeoVRML_createCompressedTreeFile";
  int vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level);
  gzFile *gzout;
  
  sprintf(filename, "%s/trees/%d/p%dp%d.wrz", this->params->vrml_dir,
     vrmlLevel, coord->tx, coord->ty);
   if (TsmGeoVRML_createDirectory(filename)) {
     if ((gzout = gzopen(filename, "wb")) == NULL) {
       tsmError(funcName, "Error creating file: %s\n", filename);
       return NULL;
     }
     return gzout;
  }

  return NULL;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createImageFile                                             | *
 * | Description:                                                                 | *
 * |   Only generates the filename needed for the texture image file of the       | *
 * |   specified level and tile coordinate.                                       | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this     The TsmGeoVRML object.                                | *
 * |   TileCoord *coord     The tile coord.  Only level, tx, and ty fields are    | *
 * |                        used.                                                 | *
 * |   char *filename       Buffer to hold the generated filename.                | *
 * | Return value:                                                                | *
 * |   TRUE on success, FALSE otherwise.                                          | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_createImageFile(TsmGeoVRML *this, TileCoord *coord, char *filename) {
  int vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level);

  sprintf(filename, "%s/images/%d/p%dp%d.jpg", this->params->vrml_dir,
    vrmlLevel, coord->tx, coord->ty);
  return TsmGeoVRML_createDirectory(filename);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createGeoElevationGrid                                      | *
 * | Description:                                                                 | *
 * |   Creates a Shape that contains a GeoElevationGrid node.  An Appearance node | *
 * |   is also attached to the Shape node that includes the corresponding texture.| *
 * |   Also returns the min, max, and average of the heights, so that the bounding| *
 * |   can be calculated later.                                                   | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this     The TsmGeoVRML object.                                | *
 * |   TileCoord *oiCoord   The OI coordinate.  Only level, tx, and ty fields are | *
 * |                        used.                                                 | *
 * |   double *min          On return, this parameter will contain the minimum    | *
 * |                        height.                                               | *
 * |   double *max          On return, this parameter will contain the maximum.   | *
 * |   double *average      On return, this parameter will contain the average.   | *
 * | Return value:                                                                | *
 * |   Returns reference to the Shape node created on success, NULL otherwise.    | *
 * +------------------------------------------------------------------------------+ */
SFNode TsmGeoVRML_createGeoElevationGrid(TsmGeoVRML *this, TileCoord *oiCoord, double *ll, double *ur, SFNode geoOriginNode) {
  const char *funcName = "TsmGeoVRML_createGeoElevationGrid";
  SFNode node;
  SFNode shapeNode;
  SFNode appearanceNode;
  SFNode normalNode;
  SFNode textureNode;
  SFNode textureCoord;
  MFFloat *height;
  MFVec2f *texPts;
  MFVec3f *normal;

  char buffer[128];
  long count;
  int vrmlLevel;
  long p;
  double h;
  double N[3];

  SimpleTileSet *tileset;
  TileTspec *tile;
  float *ixiytouv;

  int polyx;
  int polyy;
  double dix, diy;
  double diu, div;
  double iu, iv;
  int pi, pj;
  TileCoord *coord;
  double xSpacing, zSpacing;
  int numPixelsX, numPixelsY;
  double x, y, z;

  CHECK_NOT_NULL(this, NULL);
  CHECK_NOT_NULL(oiCoord, NULL);

  tileset = &this->oiPyramid->tspecLevel.val[oiCoord->level];
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  tile = &tileset->tileTspec;

  /* Calculate the total size of the texture map, without the   */
  /* padding bytes around it - we don't want to create geometry */
  /* or map imagery for the padded region.                      */
  /* maxu = oiPyramid->minResolution * (tileset0->maxValidPixel[0]+1) + ixiytouv[2]; */
  /* maxv = oiPyramid->minResolution * (tileset0->maxValidPixel[1]+1) + ixiytouv[5]; */

  polyx = polyy = this->params->num_polys;

  if (oiCoord->tx >= tileset->maxTile[0]) {
    numPixelsX = tileset->maxValidPixel[0] - oiCoord->tx*tile->numPixelsX + 1;
    polyx = max( 2, (int) ceil(this->params->num_polys * (double)numPixelsX /
			       tile->numPixelsX) );
  }
  else {
    numPixelsX = tile->numPixelsX;
  }
  dix = numPixelsX / (double) (polyx-1);
  xSpacing = dix * ixiytouv[0];
  diu = dix / numPixelsX;

  if (oiCoord->ty >= tileset->maxTile[1]) {
    numPixelsY = tileset->maxValidPixel[1] - oiCoord->ty*tile->numPixelsY + 1;
    polyy = max( 2, (int) ceil(this->params->num_polys * (double)numPixelsY /
			       tile->numPixelsY) );
  }
  else {
    numPixelsY = tile->numPixelsY;
  }
  diy = numPixelsY / (double) (polyy-1);
  zSpacing = diy * ixiytouv[4];
  div = diy / numPixelsY;

  /* Create a GeoElevationGrid node */
  if ((node = vsgCreateNode(this->wrl, this->geoElevationGridClass)) == NULL) {
    tsmError(funcName, "Couldn't create GeoElevatinoGrid node.");
    return NULL;
  }

  /* Set the geOrigin node */
  TsmGeoVRML_setNodeSFNode(this->wrl, node, "geoOrigin", geoOriginNode );

  TsmGeoVRML_setNodeSFBool(this->wrl, node, "solid", this->params->solid);
  TsmGeoVRML_setNodeSFBool(this->wrl, node, "normalPerVertex", TRUE);
  TsmGeoVRML_setNodeSFInt32(this->wrl, node, "xDimension", polyx);
  TsmGeoVRML_setNodeSFInt32(this->wrl, node, "zDimension", polyy);
  sprintf(buffer, "%.10f", xSpacing);
  TsmGeoVRML_setNodeSFString(this->wrl, node, "xSpacing", vsgStrdup(this->wrl, buffer));
  sprintf(buffer, "%.10f", zSpacing);
  TsmGeoVRML_setNodeSFString(this->wrl, node, "zSpacing", vsgStrdup(this->wrl, buffer));
  TsmGeoVRML_setNodeSFFloat(this->wrl, node, "yScale", this->params->elev_scale);
  TsmGeoVRML_setGeoSystem(this->oiPyramid, this->wrl, node);
  TsmGeoVRML_getOrigin(this->oiPyramid, oiCoord, buffer);
  TsmGeoVRML_setNodeSFString(this->wrl, node, "geoGridOrigin", vsgStrdup(this->wrl, buffer));

  coord = TileCoord_new1(oiCoord);

  ll[0] = ll[1] = ll[2] = DBL_MAX;
  ur[0] = ur[1] = ur[2] = -DBL_MAX;

  height = vsgStartMFFloat(this->wrl);
  texPts = vsgStartMFVec2f(this->wrl);
  if (this->params->normals) normal = vsgStartMFVec3f(this->wrl);
  for (pj=0, coord->iy=oiCoord->ty * tile->numPixelsY, count=0, iv=0, p=0; pj<polyy; pj++, coord->iy+=diy, iv+=div) {
    for (pi=0, coord->ix=oiCoord->tx * tile->numPixelsX, iu=0; pi<polyx; pi++, coord->ix+=dix, count++, iu+=diu, p++) {
      h = TsmGeoVRML_getElevation(this, coord);
      vsgAppendSFFloat(this->wrl, height, h);
      vsgAppendSFVec2f(this->wrl, texPts, vsgMakeSFVec2f(iu, iv));

      TsmGeoVRML_ixiy2uv(this->oiPyramid, coord->level, coord->ix, coord->iy, &coord->u, &coord->v);
      TsmGeoVRML_uv2gcc(this->oiPyramid, coord->u, coord->v, h, &x, &y, &z);
      if (x<ll[0]) ll[0] = x;
      if (y<ll[1]) ll[1] = y;
      if (z<ll[2]) ll[2] = z;
      if (x>ur[0]) ur[0] = x;
      if (y>ur[1]) ur[1] = y;
      if (z>ur[2]) ur[2] = z;

      if (this->params->normals) {
        TsmGeoVRML_calcNormal(this, coord, h, dix, diy, N);
        vsgAppendSFVec3f(this->wrl, normal, vsgMakeSFVec3f(N[0], N[1], N[2]));
      }
    }
  }
  vsgEndMFFloat(this->wrl, height);
  vsgEndMFVec2f(this->wrl, texPts);

  TsmGeoVRML_setNodeMFFloat(this->wrl, node, "height", height);

  if (this->params->normals) {
    vsgEndMFVec3f(this->wrl, normal);

    normalNode = vsgNewNormal(this->wrl);
    TsmGeoVRML_setNodeMFVec3f(this->wrl, normalNode, "vector", normal);
    TsmGeoVRML_setNodeSFNode(this->wrl, node, "normal", normalNode);
  }

  textureCoord = vsgNewTextureCoordinate(this->wrl);
  TsmGeoVRML_setNodeMFVec2f(this->wrl, textureCoord, "point", texPts);
  TsmGeoVRML_setNodeSFNode(this->wrl, node, "texCoord", textureCoord);

  textureNode = vsgNewImageTexture(this->wrl);
  vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level);
  if ( this->params->vrml_url ) {
    sprintf(buffer, "%s/images/%d/p%dp%d.jpg", this->params->vrml_url, vrmlLevel, coord->tx, coord->ty);
  }
  else {
    sprintf(buffer, "../../images/%d/p%dp%d.jpg", vrmlLevel, coord->tx, coord->ty);
  }
  TsmGeoVRML_setNodeMFString0(this->wrl, textureNode, "url", vsgStrdup(this->wrl, buffer));

  /* Appearance settings */
  appearanceNode = vsgNewAppearance(this->wrl);
  TsmGeoVRML_setNodeSFNode(this->wrl, appearanceNode, "texture", textureNode);

  /* Create a Shape node to hold the GeoElevationGrid node */
  if ((shapeNode = vsgNewShape(this->wrl)) == NULL) {
    tsmError(funcName, "Couldn't create Shape node");
    return FALSE;
  }

  TsmGeoVRML_setNodeSFNode(this->wrl, shapeNode, "appearance", appearanceNode);
  TsmGeoVRML_setNodeSFNode(this->wrl, shapeNode, "geometry", node);

  return shapeNode;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_createTestTileModel                                         | *
 * | Description:                                                                 | *
 * |   Create a Shape node that contains a flat square covering the area of the   | *
 * |   specified tile.  A random color will be assigned to the flat square.  This | *
 * |   is used only for debugging purposes.                                       | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The coordinate of the tile being created.  Only level, | *
 * |                       tx, and ty are used.                                   | *
 * | Return value:                                                                | *
 * |   On success, returns reference to the Shape node created, NULL otherwise.   | *
 * +------------------------------------------------------------------------------+ */
SFNode TsmGeoVRML_createTestTileModel(TsmGeoVRML *this, TileCoord *coord, double *ll, double *ur) {
  static const char *funcName = "TsmGeoVRML_writeTestTileModel";
  SimpleTileSet *tileset;
  TileTspec *tile;
  float *ixiytouv;

  SFNode shapeNode;
  SFNode appearanceNode;
  SFNode materialNode;
  SFNode indexedFaceSet;
  SFNode geoCoordinateNode;
  MFString *point;
  MFInt32 *coordIndex;
  GeoCoordBuffer *geoCoordBuffer;
  int ix0, iy0;
  double u, v;
  int polyx, polyy;
  double xSpacing, zSpacing;
  int numPixelsX, numPixelsY;
  int pi, pj;
  double x, y, z;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(coord, FALSE);

  tileset = &this->oiPyramid->tspecLevel.val[coord->level];
  tile = &tileset->tileTspec;
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;

  if ((geoCoordinateNode = vsgCreateNode(this->wrl, this->geoCoordinateClass)) == NULL) {
    tsmError(funcName, "Couldn't create GeoCoordinate node");
    return NULL;
  }

  if ((geoCoordBuffer = GeoCoordBuffer_new()) == NULL) {
    tsmError(funcName, "Couldn't allocate memory for GeoCoordinate structure");
    return NULL;
  }

  ix0 = coord->tx * tile->numPixelsX;
  iy0 = coord->ty * tile->numPixelsY;
  /* width = (coord->tx==tileset->maxTile[0]) ? tileset->maxValidPixel[0] - ix0 + 1 : tile->numPixelsX;  */
  /* height = (coord->ty==tileset->maxTile[1]) ? tileset->maxValidPixel[1] - iy0 + 1 : tile->numPixelsY; */

  polyx = polyy = this->params->num_polys;

  if (coord->tx >= tileset->maxTile[0]) {
    numPixelsX = tileset->maxValidPixel[0] - coord->tx*tile->numPixelsX + 1;
    polyx = (int) ceil(this->params->num_polys * (double)numPixelsX / tile->numPixelsX);
    xSpacing = numPixelsX * ixiytouv[0] / (double) (polyx-1);
  }
  else {
    numPixelsX = tile->numPixelsX;
    xSpacing = numPixelsX * ixiytouv[0] / (double) (polyx-1);
  }

  if (coord->ty >= tileset->maxTile[1]) {
    numPixelsY = tileset->maxValidPixel[1] - coord->ty*tile->numPixelsY + 1;
    polyy = (int) ceil(this->params->num_polys * (double)numPixelsY / tile->numPixelsY);
    zSpacing = numPixelsY * ixiytouv[4] / (double) (polyy-1);
  }
  else {
    numPixelsY = tile->numPixelsY;
    zSpacing = numPixelsY * ixiytouv[4] / (double) (polyy-1);
  }

  ll[0] = ll[1] = ll[2] = DBL_MAX;
  ur[0] = ur[1] = ur[2] = -DBL_MAX;

  for (pj=0, v = iy0 * ixiytouv[4] + ixiytouv[5]; pj<polyy; pj++, v+=zSpacing) {
    for (pi=0, u = ix0 * ixiytouv[0] + ixiytouv[2]; pi<polyx; pi++, u+=xSpacing) {
      GeoCoordBuffer_insert(geoCoordBuffer, u, v, 0);
      TsmGeoVRML_uv2gcc(this->oiPyramid, u, v, 0, &x, &y, &z);
      if (x<ll[0]) ll[0] = x;
      if (y<ll[1]) ll[1] = y;
      if (z<ll[2]) ll[2] = z;
      if (x>ur[0]) ur[0] = x;
      if (y>ur[1]) ur[1] = y;
      if (z>ur[2]) ur[2] = z;
    }
  }
  if ((point = GeoCoordBuffer_getMFString(geoCoordBuffer, this->wrl)) == NULL) {
    GeoCoordBuffer_destroy(&geoCoordBuffer);
    return NULL;
  }
  TsmGeoVRML_setNodeMFString(this->wrl, geoCoordinateNode, "point", point);
  TsmGeoVRML_setGeoSystem(this->oiPyramid, this->wrl, geoCoordinateNode);

  /* Create coordIndex array */
  coordIndex = vsgStartMFInt32(this->wrl);
  for (pj=0; pj<polyy-1; pj++) {
    for(pi=0; pi<polyx-1; pi++) {
      vsgAppendSFInt32(this->wrl, coordIndex, pj * polyx + pi);
      vsgAppendSFInt32(this->wrl, coordIndex, pj * polyx + pi+1);
      vsgAppendSFInt32(this->wrl, coordIndex, (pj+1)*polyx + pi+1);
      vsgAppendSFInt32(this->wrl, coordIndex, (pj+1)*polyx + pi);
      vsgAppendSFInt32(this->wrl, coordIndex, -1);
    }
  }
  vsgEndMFInt32(this->wrl, coordIndex);

  /* Create an IndexedFaceSet node */
  indexedFaceSet = vsgNewIndexedFaceSet(this->wrl);
  TsmGeoVRML_setNodeSFNode(this->wrl, indexedFaceSet, "coord", geoCoordinateNode);
  TsmGeoVRML_setNodeMFInt32(this->wrl, indexedFaceSet, "coordIndex", coordIndex);
  TsmGeoVRML_setNodeSFBool(this->wrl, indexedFaceSet, "colorPerVertex", FALSE);
  
  materialNode = vsgNewMaterial(this->wrl);
  TsmGeoVRML_setNodeSFColor(this->wrl, materialNode, "diffuseColor",
    rand() / (float) RAND_MAX, rand() / (float) RAND_MAX, rand() / (float) RAND_MAX);

  appearanceNode = vsgNewAppearance(this->wrl);
  TsmGeoVRML_setNodeSFNode(this->wrl, appearanceNode, "material", materialNode);

  shapeNode = vsgNewShape(this->wrl);
  TsmGeoVRML_setNodeSFNode(this->wrl, shapeNode, "appearance", appearanceNode);
  TsmGeoVRML_setNodeSFNode(this->wrl, shapeNode, "geometry", indexedFaceSet);

  GeoCoordBuffer_destroy(&geoCoordBuffer);

  return shapeNode;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_writeTileModel                                              | *
 * | Description:                                                                 | *
 * |   Generates VRML model for a single give tile, specified by the coord        | *
 * |   parameter.                                                                 | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The coordinate of the tile being generated.            | *
 * | Return value:                                                                | *
 * |   Returns TRUE if everything is ok, FALSE otherwise.                         | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_writeTileModel(TsmGeoVRML *this, TileCoord *coord) {
  const char *funcName = "TsmGeoVRML_writeTileModel";
  char buffer[256];
  int vrmlLevel;
  double ll[3], ur[3];
  FILE *fout;
  gzFile *gzout;
  char filename[512];
  char wrlFilename[512];
  double centerU, centerV, centerElev;
  double range;
  static double maxRange = 0.0;
  int overx, overy;
  char urlbuf[128];

  SFNode groupNode;
  SFNode navigationInfoNode;
  SFNode backgroundNode;
  SFNode geoLODNode;
  SFNode geoElevationGridNode;
  SFNode indexedFaceSetNode;
  MFNode *children;
  SFNode geoViewpointNode;
  SFNode geoOriginNode;
  MFFloat *avatar;
  MFString *navTypeStr;
  SFNode switchNode;
  MFNode *switchChildren;
  SFNode switchGroupNode0;
  SFNode switchGroupNode1;
  SFNode geoTouchSensorNode0;
  SFNode geoTouchSensorNode1;
  MFNode *switchGroupChildren;
  SFNode proximitySensorNode;
  SFNode touchScriptNode;
  SFFloat proxDist;
  SFVec3f proxSize;
  SFNode hudTransformNode;
  MFNode *hudChildren;
  SFNode textTransformNode;
  SFNode textNode;
  SFNode shapeNode;
  MFNode *textTransformChildren;
  SFFloat visibilityLimit = 1.e7;

/* Scripts and Routes to get added at ane of TileModel */
static const char *TouchScriptAndRoutes = "\
DEF TS Script {\n\
     eventIn SFTime set_time\n\
     eventOut SFInt32 changed_choice\n\
     url \"javascript:\n\
      function set_time (value, timestamp)\n\
      {\n\
        changed_choice = 1;\n\
      }\n\
      \"\n\
}\n\
ROUTE LGTS.touchTime TO TS.set_time\n\
ROUTE TS.changed_choice TO SW.whichChoice\n\
";

static const char *TextScriptAndRoutes = "\
DEF TEXTSCRIPT Script {\n\
     eventIn SFBool isOver\n\
     eventIn SFString set_value\n\
     eventOut MFString LatLonString\n\
     eventOut MFNode child\n\
     field MFNode noText WorldInfo {}\n\
     field MFNode posText Transform {\n\
       children Shape { geometry DEF TEXT Text { fontStyle FontStyle { family \"SANSERIF\" } } }\n\
       translation 5 -10 -30\n\
     }\n\
     url \"javascript:\n\
      //  Format lat lon depth. Divide depth by vertical exaggeration.\n\
      function set_value (value, timestamp)\n\
      {\n\
        var lld = value.split(\\\" \\\");\n\
        LatLonString[0] = Math.round(lld[0]*10000)/10000 + \\\"   \\\"\n\
        + Math.round(lld[1]*10000)/10000 + \\\"   \\\"\n\
        + Math.round(lld[2]*10)/10/%f + \\\" m\\\";\n\
      }\n\
      //  Display text only if over the tile\n\
      function isOver ( value, timestamp)\n\
      {\n\
        if ( value ) {\n\
          child = posText;\n\
        }\n\
        else {\n\
          child = noText;\n\
        }\n\
      }\n\
      \"\n\
}\n\
ROUTE PROX.position_changed TO HUD.set_translation\n\
ROUTE PROX.orientation_changed TO HUD.set_rotation\n\
ROUTE LGTS.hitGeoCoord_changed TO TEXTSCRIPT.set_value\n\
ROUTE HGTS.hitGeoCoord_changed TO TEXTSCRIPT.set_value\n\
ROUTE LGTS.isOver TO TEXTSCRIPT.isOver\n\
ROUTE HGTS.isOver TO TEXTSCRIPT.isOver\n\
ROUTE TEXTSCRIPT.child TO HUD.children\n\
ROUTE TEXTSCRIPT.LatLonString TO TEXT.string\n\
";



  /* TileTspec *tile; */
  SimpleTileSet *tileset;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(coord, FALSE);

  /* printf("Create tree file... "); */
  /* Create directory if needed, and open the .wrl file for writing */
  if ((fout = TsmGeoVRML_createTreeFile(this, coord, filename)) == NULL) {
    tsmError(funcName, "Couldn't create tree file: %s", filename);
    return FALSE;
  }
  TsmGeoVRML_progress(1);

  tileset = &this->oiPyramid->tspecLevel.val[coord->level];

  /* Figure out URL's to the higher resolution children */
  vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level);

  /* Initialize the VRML world */
  TsmGeoVRML_initWorld(this);

  if (this->params->collision && vrmlLevel == this->params->root_level) {
    groupNode = vsgNewCollision(this->wrl);
  }
  else {
    groupNode = vsgNewGroup(this->wrl);
  }
  children = vsgStartMFNode(this->wrl);

  /* Create a NavigationInfo with small avatar distance, but not too small*/
  // Commented out 1 May 2002 - it conflicts with GeoViewpoint Navinfo settings
  //navigationInfoNode = vsgNewNavigationInfo( this->wrl );
  //avatar = vsgStartMFFloat(this->wrl);
  //vsgAppendSFFloat(this->wrl, avatar, 3.0 );
  //vsgAppendSFFloat(this->wrl, avatar, 1.6 );
  //vsgAppendSFFloat(this->wrl, avatar, 0.75 );
  //vsgEndMFFloat(this->wrl, avatar);
  //TsmGeoVRML_setNodeMFFloat( this->wrl, navigationInfoNode, "avatarSize", avatar ); 
  //TsmGeoVRML_setNodeSFFloat( this->wrl, navigationInfoNode, "speed", 1000 ); 
  //navTypeStr = vsgStartMFString(this->wrl);
  //vsgAppendSFString(this->wrl, navTypeStr, vsgStrdup(this->wrl, "EXAMINE") );
  //vsgAppendSFString(this->wrl, navTypeStr, vsgStrdup(this->wrl, "FLY") );
  //vsgEndMFString(this->wrl, navTypeStr);
  //TsmGeoVRML_setNodeMFString( this->wrl, navigationInfoNode, "type", navTypeStr ); 
  //TsmGeoVRML_setNodeSFFloat( this->wrl, navigationInfoNode, "visibilityLimit", visibilityLimit ); 
  //vsgAppendSFNode(this->wrl, children, navigationInfoNode);


  /* create a background node with a black colour, unless whitebgFlag */

  backgroundNode = vsgNewBackground( this->wrl );
  if ( whitebgFlag ) {
    TsmGeoVRML_setNodeMFColor( this->wrl, backgroundNode, "skyColor", 1, 1, 1 ); 
    TsmGeoVRML_setNodeMFColor( this->wrl, backgroundNode, "groundColor", 1, 1, 1 ); 
  }
  else {
    TsmGeoVRML_setNodeMFColor( this->wrl, backgroundNode, "skyColor", 0.0, 0.0, 0.0 ); 
    TsmGeoVRML_setNodeMFColor( this->wrl, backgroundNode, "groundColor", 0.0, 0.0, 0.0 ); 
  }
  vsgAppendSFNode(this->wrl, children, backgroundNode);


  /* Create the GeoOrigin node  mpm  10/17/01 */
  {
    if ((geoOriginNode = vsgCreateNode(this->wrl, this->geoOriginClass)) == NULL) {
      tsmError(funcName, "Couldn't create GeoOrigin node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);
      
      return FALSE;
    }

	vsgDEF( this->wrl, vsgIntern( this->wrl, "ORIGIN" ), geoOriginNode );

    /* Set the geoSystem field */
    TsmGeoVRML_setGeoSystem(this->oiPyramid, this->wrl, geoOriginNode);

    TsmGeoVRML_setNodeMFString0(this->wrl, geoOriginNode, "geoSystem", vsgStrdup(this->wrl, "GDC"));
    sprintf(buffer, "%f %f %f", latOrigin, lonOrigin, 0.0);
    TsmGeoVRML_setNodeSFString(this->wrl, geoOriginNode, "geoCoords", vsgStrdup(this->wrl, buffer));
    vsgAppendSFNode(this->wrl, children, geoOriginNode);
  }


  /* Create the GeoElevationGrid node */
  if (this->params->test_vtiles) {
    if ((indexedFaceSetNode = TsmGeoVRML_createTestTileModel(this, coord, ll, ur)) == NULL) {
      tsmError(funcName, "Couldn't create test tile model");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }
  }
  else {
    if ((geoElevationGridNode = TsmGeoVRML_createGeoElevationGrid(this, coord, ll, ur, geoOriginNode)) == NULL) {
      tsmError(funcName, "Couldn't create GeoElevationGrid node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }
  }

  TsmGeoVRML_getCenter(this->oiPyramid, ll, ur, &centerU, &centerV, &centerElev);

  if (coord->level > this->params->end_level) {

    /* If it's not the end level, create a GeoLOD node */
    if ((geoLODNode = vsgCreateNode(this->wrl, this->geoLODClass)) == NULL) {
      tsmError(funcName, "Couldn't create GeoLOD node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }

    /* Figure out URL's to the higher resolution children */
    vrmlLevel = tsmTVToVRMLLevel(this->oiPyramid, coord->level-1);

    if (this->params->test_vtiles) {
      TsmGeoVRML_setNodeMFNode0(this->wrl, geoLODNode, "rootNode", indexedFaceSetNode);
    }
    else {
      /* Assign the Shape node that holds the GeoElevationGrid node to rootNode field of GeoLOD */
      TsmGeoVRML_setNodeMFNode0(this->wrl, geoLODNode, "rootNode", geoElevationGridNode);
    }

    if ( this->params->vrml_url ) {
      sprintf(urlbuf, "%s/trees", this->params->vrml_url);
    }
	else {
      sprintf(urlbuf, "..");
	}

	if ( wrzFlag ) {
      sprintf(buffer, "%s/%d/p%dp%d.wrz", urlbuf, vrmlLevel, coord->tx*2, coord->ty*2);
	}
	else {
	  sprintf(buffer, "%s/%d/p%dp%d.wrl", urlbuf, vrmlLevel, coord->tx*2, coord->ty*2);
    }
    TsmGeoVRML_setNodeMFString0(this->wrl, geoLODNode, "child1Url", buffer);

    if (overx = (coord->tx < tileset->maxTile[0]
        || (tileset->numValidPixel[0] - coord->tx * tileset->tileTspec.numPixelsX) > tileset->tileTspec.numPixelsX * 0.5)) {
      if ( wrzFlag ) {
        sprintf(buffer, "%s/%d/p%dp%d.wrz", urlbuf, vrmlLevel, coord->tx*2+1, coord->ty*2);
      }
      else {
        sprintf(buffer, "%s/%d/p%dp%d.wrl", urlbuf, vrmlLevel, coord->tx*2+1, coord->ty*2);
      }
      TsmGeoVRML_setNodeMFString0(this->wrl, geoLODNode, "child2Url", buffer);
    }

    if (overy = (coord->ty < tileset->maxTile[1]
        || (tileset->numValidPixel[1] - coord->ty * tileset->tileTspec.numPixelsY) > tileset->tileTspec.numPixelsY * 0.5)) {
      if ( wrzFlag ) {
        sprintf(buffer, "%s/%d/p%dp%d.wrz", urlbuf, vrmlLevel, coord->tx*2, coord->ty*2+1);
      }
      else {
        sprintf(buffer, "%s/%d/p%dp%d.wrl", urlbuf, vrmlLevel, coord->tx*2, coord->ty*2+1);
      }
      TsmGeoVRML_setNodeMFString0(this->wrl, geoLODNode, "child3Url", buffer);
    }

    if (overx && overy) {
      if ( wrzFlag ) {
        sprintf(buffer, "%s/%d/p%dp%d.wrz", urlbuf, vrmlLevel, coord->tx*2+1, coord->ty*2+1);
      }
      else {
        sprintf(buffer, "%s/%d/p%dp%d.wrl", urlbuf, vrmlLevel, coord->tx*2+1, coord->ty*2+1);
      }
      TsmGeoVRML_setNodeMFString0(this->wrl, geoLODNode, "child4Url", buffer);
    }

    /* Set the geOrigin node */
    TsmGeoVRML_setNodeSFNode(this->wrl, geoLODNode, "geoOrigin", geoOriginNode );

    /* Set the geoSystem field */
    TsmGeoVRML_setGeoSystem(this->oiPyramid, this->wrl, geoLODNode);

    /* Set center and range */
    sprintf(buffer, "%f %f %f", centerV, centerU, centerElev);
    TsmGeoVRML_setNodeSFString(this->wrl, geoLODNode, "center", vsgStrdup(this->wrl, buffer));

	/* Set range to max of tile set if so specified */
   	range = TsmGeoVRML_calcRange(this, coord, ll, ur);
	if ( maxRangeFlag ) {
		maxRange = (range > maxRange) ? range : maxRange;
		range = maxRange;	// For large areas need to get way back
		//printf ( "\nlevel %d, range = %lf\n", coord->level, range );
	}
	else {
    	range = TsmGeoVRML_calcRange(this, coord, ll, ur);
	}
    /* Peg range at 90% of the visibilityLimit */
	if ( range >= visibilityLimit ) {
	  range = 0.9 * visibilityLimit;
    } 

    TsmGeoVRML_setNodeSFFloat(this->wrl, geoLODNode, "range", range);

    /* Finally add the GeoLOD node to the VRML world */
    /* If at a touchLevel (1 of possible 3) then it's added inside the Switch node below */
    if (coord->level != touch1 && coord->level != touch2 && coord->level != touch3 ) {
      vsgAppendSFNode(this->wrl, children, geoLODNode);
    }
    /* vsgWorldAppend(this->wrl, geoLODNode); */
  }
  else {
    /* If it's the end level, just add the Shape node that holds the GeoElevationGrid node */
    /* to the VRML world */
    if (this->params->test_vtiles) {
      vsgAppendSFNode(this->wrl, children, indexedFaceSetNode);
    }
    else {
      /* If at a touchLevel then it's added inside the Switch node below */
      if (coord->level != touch1 && coord->level != touch2 && coord->level != touch3 ) {
        vsgAppendSFNode(this->wrl, children, geoElevationGridNode);
      }
    }
  }

  if ( ! noviewFlag ) {
    /* Create a default GeoViewpoint node that looks at the center of the tile */
    if ((geoViewpointNode = vsgCreateNode(this->wrl, this->geoViewpointClass)) == NULL) {
      tsmError(funcName, "Couldn't create GeoViewpoint node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }

    /* Set the geOrigin node */
    TsmGeoVRML_setNodeSFNode(this->wrl, geoViewpointNode, "geoOrigin", geoOriginNode );

    /* Set GeoViewpoint node's fields */
    TsmGeoVRML_setGeoSystem(this->oiPyramid, this->wrl, geoViewpointNode);
    sprintf(buffer, "%f %f %f", centerV, centerU, range + 100);
    TsmGeoVRML_setNodeSFString(this->wrl, geoViewpointNode, "position", vsgStrdup(this->wrl, buffer));
    TsmGeoVRML_setNodeSFRotation(this->wrl, geoViewpointNode, "orientation", 1, 0, 0, -1.57);
    TsmGeoVRML_setNodeSFString(this->wrl, geoViewpointNode, "description", vsgStrdup(this->wrl, "Home"));
    vsgAppendSFNode(this->wrl, children, geoViewpointNode);
  }



  //printf ("\n%s(): Level = %d, touch1 = %d\n", funcName, coord->level, touch1 );
  if (coord->level == touch1 || coord->level == touch2 || coord->level == touch3) {

    /* If this is a "touch" level then add HUD (if asked for) */
    if ( hudFlag ) {
      /* ProximitySensor  - provides transl & rotation for HUD transform */
	  proxDist = 1.e7;		// 100000 km ~ to view from near Earth orbit
      proximitySensorNode = vsgNewProximitySensor(this->wrl);
      vsgDEF( this->wrl, vsgIntern( this->wrl, "PROX" ), proximitySensorNode );
      proxSize = vsgMakeSFVec3f(proxDist, proxDist, proxDist);
      TsmGeoVRML_setNodeSFVec3f(this->wrl, proximitySensorNode, "size", proxSize);
	  /* HUD transform with text display */
      hudTransformNode = vsgNewTransform(this->wrl);
      vsgDEF( this->wrl, vsgIntern( this->wrl, "HUD" ), hudTransformNode );
    }

	/* wrap a Switch node around choices */
    //printf ("\n%s(): Creating Switch node at level %d..\n", funcName, coord->level );
    switchNode = vsgNewSwitch(this->wrl);
    vsgDEF( this->wrl, vsgIntern( this->wrl, "SW" ), switchNode );

    /* Choices for the Switch node */
    switchChildren = vsgStartMFNode(this->wrl);

    /* First child (choice 0) is Lo-res GeoTouchSensor with GeoElevationGrid */
    switchGroupNode0 = vsgNewGroup(this->wrl);
    switchGroupChildren = vsgStartMFNode(this->wrl);

    if ((geoTouchSensorNode0 = vsgCreateNode(this->wrl, this->geoTouchSensorClass)) == NULL) {
      tsmError(funcName, "Couldn't create GeoTouchSensor node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }
    vsgDEF( this->wrl, vsgIntern( this->wrl, "LGTS" ), geoTouchSensorNode0 );
    TsmGeoVRML_setNodeSFNode(this->wrl, geoTouchSensorNode0, "geoOrigin", geoOriginNode );

    TsmGeoVRML_setNodeSFInt32(this->wrl, switchNode, "whichChoice", 0);

    vsgAppendSFNode(this->wrl, switchGroupChildren, geoTouchSensorNode0 );
    vsgAppendSFNode(this->wrl, switchGroupChildren, geoElevationGridNode );
    vsgEndMFNode(this->wrl, switchGroupChildren);

    TsmGeoVRML_setNodeMFNode(this->wrl, switchGroupNode0, "children", switchGroupChildren);

    /* Second child (choice 1) is GeoLOD group with Hires GeoTouchSensor */
    switchGroupNode1 = vsgNewGroup(this->wrl);
    switchGroupChildren = vsgStartMFNode(this->wrl);

    if ((geoTouchSensorNode1 = vsgCreateNode(this->wrl, this->geoTouchSensorClass)) == NULL) {
      tsmError(funcName, "Couldn't create GeoTouchSensor node");
      TsmGeoVRML_destroyWorld(this);
      fclose(fout);

      return FALSE;
    }
    TsmGeoVRML_setNodeSFNode(this->wrl, geoTouchSensorNode1, "geoOrigin", geoOriginNode );
    vsgDEF( this->wrl, vsgIntern( this->wrl, "HGTS" ), geoTouchSensorNode1 );

    vsgAppendSFNode(this->wrl, switchGroupChildren, geoTouchSensorNode1 );
    vsgAppendSFNode(this->wrl, switchGroupChildren, geoLODNode );
    vsgEndMFNode(this->wrl, switchGroupChildren);

    TsmGeoVRML_setNodeMFNode(this->wrl, switchGroupNode1, "children", switchGroupChildren);


    /* Build up all the children & add to world */

    vsgEndMFNode(this->wrl, switchChildren);
    vsgAppendSFNode(this->wrl, switchChildren, switchGroupNode0 );
    vsgAppendSFNode(this->wrl, switchChildren, switchGroupNode1 );
    TsmGeoVRML_setNodeMFNode(this->wrl, switchNode, "choice", switchChildren);

    if ( hudFlag ) {
      vsgAppendSFNode(this->wrl, children, proximitySensorNode);
      vsgAppendSFNode(this->wrl, children, hudTransformNode);
    }
    vsgAppendSFNode(this->wrl, children, switchNode);

    vsgEndMFNode(this->wrl, children);
    TsmGeoVRML_setNodeMFNode(this->wrl, groupNode, "children", children);

    vsgWorldAppend(this->wrl, groupNode);

  }
  else {
    vsgEndMFNode(this->wrl, children);
    TsmGeoVRML_setNodeMFNode(this->wrl, groupNode, "children", children);
    vsgWorldAppend(this->wrl, groupNode);
  }


  /* End of the world */
  vsgEndWorld(this->wrl);

  /* Dump VRML world to file */
  vsgPrint(this->wrl, fout);



  if (coord->level == touch1 || coord->level == touch2 || coord->level == touch3) {

    /* Add Scripts & Routes to the file, use elevscale from command line */
    fprintf(fout, "%s", TouchScriptAndRoutes);
    if ( hudFlag ) {
      fprintf(fout, TextScriptAndRoutes, this->params->elev_scale);
    }
  }
  fclose(fout);


  /* If to be gzipped, then read in the .wrl & write back out as .wrz */

  if ( wrzFlag ) {
	/* Open .wrl file for input */
    //printf ("\nOpening %s\n", filename);
    strcpy(wrlFilename, filename);
    if ((fout = fopen(filename, "rb")) == NULL) {
      tsmError(funcName, "Error opening file: %s\n", filename);
    }

    /* Open .wrz file for output */
    if ((gzout = TsmGeoVRML_createCompressedTreeFile(this, coord, filename)) == NULL) {
      tsmError(funcName, "Couldn't create tree file: %s", filename);
    }
    //printf ("\nOpened %s\n", filename);

    while ( fgets(buffer, 256, fout) != NULL ) {
		gzputs(gzout, buffer);
    }

    gzclose(gzout);
    fclose(fout);
	remove(wrlFilename);
  }


  /* And then, destroy the world */
  TsmGeoVRML_destroyWorld(this);
  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_writeTileImage                                              | *
 * | Description:                                                                 | *
 * |   Generates the texture image file for a given tile, specified by coord.     | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The coordinate of the tile being generated. Only level,| *
 * |                       tx, and ty fields are used.                            | *
 * | Return value:                                                                | *
 * |   TRUE if everything's ok, FALSE if there's an error.                        | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_writeTileImage(TsmGeoVRML *this, TileCoord *coord) {
  static const char *funcName = "TsmGeoVRML_writeTileImage";
  TsmConnection out_connect;
  Pyramid       *out_pyramid;
  SimpleTileSet *tileset;
  TileTspec     *tile;
  unsigned char *buffer;
  char          filename[512];

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(coord, FALSE);

  /* Make a dummy output connection to use for tsmSaveImage */
  /* do this in case we need to convert dataset image type */
  if ((out_connect = tsmConnect("dummy", "w", NULL)) == NULL) {
    tsmError(funcName, "Couldn't connect to dummy dataset");
    return FALSE;
  }

  /* Copy the tspec structure from the input to the output connection */
  tsmSetTspecField(out_connect, this->oiPyramid);

  out_pyramid = (Pyramid *) tsmGet(out_connect, TSM_DATASET_TSPEC);
  if (out_pyramid == NULL) {
    tsmError(funcName, "Couldn't copy the tspec structure!");
    return FALSE;
  }

  /* Set the image format to always be colour (RGB) */
  if (out_pyramid->pixelTspec.componentNames.val != NULL)
    tsmMemFree(out_pyramid->pixelTspec.componentNames.val);

  out_pyramid->tileTspec.numComponentsPixel = 3;
  tsmArrayMalloc(tspSymbol, out_pyramid->pixelTspec.componentNames, 3);
  out_pyramid->pixelTspec.componentNames.val[0] = tspRed;
  out_pyramid->pixelTspec.componentNames.val[1] = tspGreen;
  out_pyramid->pixelTspec.componentNames.val[2] = tspBlue;
  
  /* allocate a buffer for the tiles to be read into */
  if ((buffer = tsmAllocTileBuffer(this->oiConnection)) == NULL) {
    tsmError(funcName, "couldn't allocate a tile buffer");
    return FALSE;
  }

  if (!tsmReadTile(this->oiConnection, coord->tx, coord->ty, coord->level, buffer)) {
    tsmError(funcName, "Couldn't read tile (%d, %d) of level %d", coord->tx, coord->ty, coord->level);
    tsmFreeTileBuffer(buffer);
    return FALSE;
  }

  if (!TsmGeoVRML_createImageFile(this, coord, filename)) {
    tsmFreeTileBuffer(buffer);
    return FALSE;
  }

  tileset = &this->oiPyramid->tspecLevel.val[coord->level];
  tile = &tileset->tileTspec;

  if (coord->tx == tileset->maxTile[0] || coord->ty == tileset->maxTile[1]) {
    int width, height;
    long p1, p2;
    int i, j;
    uchar *newBuffer;

    width = (coord->tx==tileset->maxTile[0])
      ? (tileset->maxValidPixel[0] - coord->tx * tile->numPixelsX + 1)
      : tile->numPixelsX;
    height = (coord->ty==tileset->maxTile[1])
      ? (tileset->maxValidPixel[1] - coord->ty * tile->numPixelsY + 1)
      : tile->numPixelsY;

    newBuffer = tsmMemMalloc(width * height * 3);
    for (i=0, p2=0; i<height; i++) {
      for (j=0, p1=i*tile->numPixelsX*tile->numComponentsPixel; j<width; j++) {
        newBuffer[p2++] = buffer[p1++];
        if ( tile->numComponentsPixel == 3 ) {
          newBuffer[p2++] = buffer[p1++];
          newBuffer[p2++] = buffer[p1++];
        }
      }
    }
    tsmFreeTileBuffer(buffer);
    buffer = newBuffer;

    out_pyramid->tileTspec.numPixelsX = width;
    out_pyramid->tileTspec.numPixelsY = height;
    out_pyramid->tileTspec.totalPixels[0] = width;
    out_pyramid->tileTspec.totalPixels[1] = height;
  }

  /* convert 1-component images to 3-component */

  if ( tile->numComponentsPixel == 1 ) {
    int   width = out_pyramid->tileTspec.numPixelsX;
    int   height = out_pyramid->tileTspec.numPixelsY;
    uchar *newBuffer = tsmMemMalloc( width * height * 3 );
    uchar *src = buffer, *dest = newBuffer;
    int   i, len = width * height;

    for ( i = 0; i < len; i++ ) {
      *(dest++) = *src;
      *(dest++) = *src;
      *(dest++) = *(src++);
    }

    tsmFreeTileBuffer( buffer );
    buffer = newBuffer;
  }

  /* finally, save out the image to a JPEG/JFIF file */

  if (!tsmSaveImage(out_connect, buffer, filename, TSM_JFIF)) {
    tsmError(funcName, "Error writing image file for tile (%d, %d) of level %d", coord->tx, coord->ty, coord->level);
    tsmFreeTileBuffer(buffer);
    return FALSE;
  }

  tsmFreeTileBuffer(buffer);

  tsmDisconnect(out_connect);

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_writeHTMLFile                                               | *
 * | Description:                                                                 | *
 * |   Outputs the HTML file that would load up the level 0 VRML file.            | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * | Return value:                                                                | *
 * |   Returns TRUE if everything's ok, FALSE otherwise.                          | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_writeHTMLFile(TsmGeoVRML *this) {
  char buffer[256];
  FILE *fout;

  sprintf(buffer, "%s/index.html", this->params->vrml_dir);

  TsmGeoVRML_createDirectory( buffer );

  if ((fout = fopen(buffer, "wb")) == NULL) {
    return FALSE;
  }

  fprintf(fout, "<html>\n");
  fprintf(fout, "<title>GeoVRML Terrain Model</title>\n");
  fprintf(fout, "<body bgcolor=\"black\">\n");
  fprintf(fout, "  <!-- Generated by tsmApi from SRI International -->\n" );
  if ( wrzFlag ) {
    fprintf(fout, "  <embed width=\"100%%\" height=\"100%%\" src=\"trees/0/p0p0.wrz\"></embed>\n" );
  }
  else {
  
    fprintf(fout, "  <embed width=\"100%%\" height=\"100%%\" src=\"trees/0/p0p0.wrl\"></embed>\n" );
  }
  fprintf(fout, "</body>\n");
  fprintf(fout, "</html>\n");

  fclose(fout);

  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_writeAllTiles                                               | *
 * | Description:                                                                 | *
 * |   This is the entry point of the program.  It iterates through all the tiles | *
 * |   in all the levels, generating the VRML model along with its corresponding  | *
 * |   texture image file.                                                        | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    The TsmGeoVRML object.                                 | *
 * | Return value:                                                                | *
 * |   Returns TRUE if everything's fine, FALSE otherwise.                        | *
 * +------------------------------------------------------------------------------+ */
int TsmGeoVRML_writeAllTiles(TsmGeoVRML *this) {
  const char *funcName = "tsmWriteAllTiles";

  int startLevel;
  int endLevel;
  SimpleTileSet *tileset;
  int temp;
  TileCoord *coord;

  CHECK_NOT_NULL(this, FALSE);

  coord = TileCoord_new();

  if (this->params) {
    if (this->params->start_level < 0) this->params->start_level = this->oiPyramid->maxLevel;
    if (this->params->end_level < 0) this->params->end_level = this->oiPyramid->minLevel;

    if (this->params->start_level < this->oiPyramid->minLevel || this->params->start_level > this->oiPyramid->maxLevel) {
      tsmError(funcName, "Start level %d not in valid range (%d..%d)",
        this->params->start_level, this->oiPyramid->minLevel, this->oiPyramid->maxLevel);
      this->params->start_level = min(max(this->params->start_level, this->oiPyramid->minLevel), this->oiPyramid->maxLevel);
    }
    if (this->params->end_level < this->oiPyramid->minLevel || this->params->end_level > this->oiPyramid->maxLevel) {
      tsmError(funcName, "End level %d not in valid range (%d..%d)",
        this->params->end_level, this->oiPyramid->minLevel, this->oiPyramid->maxLevel);
      this->params->end_level = min(max(this->params->end_level, this->oiPyramid->minLevel), this->oiPyramid->maxLevel);
    }
    if (this->params->start_level < this->params->end_level) {
      tsmError(funcName, "Start level must be higher then end level");
      temp = this->params->start_level;
      this->params->start_level = this->params->end_level;
      this->params->end_level = temp;
    }
    startLevel = this->params->start_level;
    endLevel = this->params->end_level;
  }
  else {
    startLevel = this->oiPyramid->maxLevel;
    endLevel = this->oiPyramid->minLevel;
  }

  TsmGeoVRML_writeHTMLFile(this);

  printf("\rNumber of levels to process: %d (last one is %d cause we start at 0)\n",
     startLevel-endLevel+1, startLevel-endLevel);

  for (coord->level=startLevel; coord->level>=endLevel; coord->level--) {
    tileset = &this->oiPyramid->tspecLevel.val[coord->level];

    printf("\rProcessing geoVRML level %d ... ",
	   tsmTVToVRMLLevel( this->oiPyramid, coord->level ) );
    fflush(stdout);

    for (coord->ty=tileset->minTile[1]; coord->ty<=tileset->maxTile[1]; coord->ty++) {
      for (coord->tx=tileset->minTile[0]; coord->tx<=tileset->maxTile[0]; coord->tx++) {

        if (this->params->vrml_models
            && !TsmGeoVRML_writeTileModel(this, coord))
          return FALSE;

        if (this->params->vrml_textures
            && !this->params->test_vtiles
            && !TsmGeoVRML_writeTileImage(this, coord))
          return FALSE;

      }
    }

  }
  printf( "\rCreated GeoVRML model in %s\n", this->params->vrml_dir );

  tsmVRMLCacheFree();

  TileCoord_destroy(&coord);
  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFString0                                            | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFString field with only a single| *
 * |   string.                                                                    | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name    Field name.                                      | *
 * |   char const *string  The string value to be set.                            | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFString0(vsgWorld *wrl, SFNode node, char const *field_name, char const *string) {
  const char     *funcName = "TsmGeoVRML_setNodeMFString";
  vsgEventInDesc evin;
  MFString       *value;
  symbol_t       field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  /* Make a new MFString instance for the string */
  if ((value = vsgMakeMFString(wrl)) == NULL ) {
    tsmError(funcName, "Couldn't make an MFString structure");
    return;
  }
  vsgAppendSFString(wrl, value, vsgStrdup(wrl, string));

  /* Intern the field name to get its internal pointer */
  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in world", field_name);
    return;
  }

  /* Get the field and then set the new MFString value */
  evin = vsgSetFunc(node, field_intern);
  vsgSetMFString(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFString                                             | *
 * | Description:                                                                 | *
 * |   Convenience function to set a node's MFString field.                       | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl      The VRML world.                                         | *
 * |   SFNode node        The concerned node.                                     | *
 * |   char const *field_name   Name of the MFString field to be set.             | *
 * |   MFString *strings  The MFstring value.                                     | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFString(vsgWorld *wrl, SFNode node, char const *field_name, MFString *strings) {
  const char     *funcName = "TsmGeoVRML_setNodeMFString";
  vsgEventInDesc evin;
  symbol_t       field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  /* Intern the field name to get its internal pointer */
  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in world", field_name);
    return;
  }

  /* Get the field and then set the new MFString value */
  evin = vsgSetFunc(node, field_intern);
  vsgSetMFString(wrl, node, evin, strings);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFNode0                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a node's MFNode field.  However, this function | *
 * |   accepts a SFNode parameter, and sets the MFNode with only a single SFNode. | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl    The VRML world.                                           | *
 * |   SFNode node      The converned node.                                       | *
 * |   const char *field_name    Name of the field.                               | *
 * |   SFNode value     The SFNode value.                                         | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFNode0(vsgWorld *wrl, SFNode node, const char *field_name, SFNode value) {
  const char *funcName = "TsmGeoVRML_setNodeMFNode0";
  vsgEventInDesc evin;
  MFNode *nodes;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in world", field_name);
    return;
  }

  if ((nodes = vsgStartMFNode(wrl)) == NULL) {
    tsmError(funcName, "Couldn't crete MFNode array");
    return;
  }
  vsgAppendSFNode(wrl, nodes, value);

  evin = vsgSetFunc(node, field_intern);
  vsgEndMFNode(wrl, nodes);
  vsgSetMFNode(wrl, node, evin, nodes);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFNode                                               | *
 * | Description:                                                                 | *
 * |   Sets a VRML node's MFNode field with a MFNode value.                       | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl     The VRML world.                                          | *
 * |   SFNode node       The converned node.                                      | *
 * |   const char *field_name   Field name.                                       | *
 * |   MFNode *value     MFNode value.                                            | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFNode(vsgWorld *wrl, SFNode node, const char *field_name, MFNode *value) {
  const char *funcName = "TsmGeoVRML_setNodeMFNode";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in world", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFNode(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFString                                             | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFString field.                  | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   char const *string  The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFString(vsgWorld *wrl, SFNode node, char const *field_name, char *string) {
  const char     *funcName = "TsmGeoVRML_setNodeSFString";
  vsgEventInDesc evin;
  symbol_t       field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  /* Intern the field name to get its internal pointer */
  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in world", field_name);
    return;
  }

  /* Get the field and then set the new SFString value */
  evin = vsgSetFunc(node, field_intern);
  vsgSetSFString(wrl, node, evin, (char*)string);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFFloat                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFFloat field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   MFFloat *value      The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFFloat(vsgWorld *wrl, SFNode node, char const *field_name, MFFloat *value) {
  const char *funcName = "TsmGeoVRML_setNodeMFFloat";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFFloat(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFInt32                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFInt32 field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   MFInt32 *value      The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFInt32(vsgWorld *wrl, SFNode node, char const *field_name, MFInt32 *value) {
  const char *funcName = "TsmGeoVRML_setNodeMFInt32";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFInt32(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFNode                                               | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFNode field.                    | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   SFNode *value       The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFNode(vsgWorld *wrl, SFNode node, const char *field_name, SFNode value) {
  const char *funcName = "TsmGeoVRML_setNodeSFNode";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);
  CHECK_NOT_NULL0(value);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't finde field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFNode(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFVec2f                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFVec2f field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   MFVec2f *value      The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFVec2f(vsgWorld *wrl, SFNode node, const char *field_name, MFVec2f *value) {
  const char *funcName = "TsmGeoVRML_setNodeMFVec2f";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFVec2f(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFColor                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFColor field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   r, g, b             The first colour's value.                              | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFColor(vsgWorld *wrl, SFNode node, const char *field_name, 
                                      float r, float g, float b) {
  const char *funcName = "TsmGeoVRML_setNodeMFColor";
  vsgEventInDesc evin;
  symbol_t field_intern;
  MFColor *value;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  value = vsgMakeMFColor( wrl );
  vsgAppendSFColor( wrl, value, vsgMakeSFColor( r, g, b ) );

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFColor(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeMFVec3f                                             | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's MFVec3f field.                  | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   char const *string  The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeMFVec3f(vsgWorld *wrl, SFNode node, const char *field_name, MFVec3f *value) {
  const char *funcName = "TsmGeoVRML_setNodeMFVec3f";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetMFVec3f(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFVec3f                                             | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFVec3f field.                  | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   char const *string  The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFVec3f(vsgWorld *wrl, SFNode node, const char *field_name, SFVec3f value) {
  const char *funcName = "TsmGeoVRML_setNodeSFVec3f";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFVec3f(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFFloat                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFFloat field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   float value         The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFFloat(vsgWorld *wrl, SFNode node, const char *field_name, float value) {
  const char *funcName = "TsmGeoVRML_setNodeSFFloat";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFFloat(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFBool                                               | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFBool field.                    | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   int value           The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFBool(vsgWorld *wrl, SFNode node, const char *field_name, int value) {
  const char *funcName = "TsmGeoVRML_setNodeSFBool";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFBool(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFInt32                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFInt32 field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   int value           The value.                                             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFInt32(vsgWorld *wrl, SFNode node, const char *field_name, int value) {
  const char *funcName = "TsmGeoVRML_setNodeSFInt32";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFInt32(wrl, node, evin, value);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFColor                                              | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFColor field.                   | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   float r, g, b       R, G, B of the SFColor value                           | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFColor(vsgWorld *wrl, SFNode node, const char *field_name, float r, float g, float b) {
  const char *funcName = "TsmGeoVRML_setNodeSFColor";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFColor(wrl, node, evin, vsgMakeSFColor(r, g, b));
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_setNodeSFRotation                                           | *
 * | Description:                                                                 | *
 * |   Convenience function to set a VRML node's SFRotation field.                | *
 * | Parameters:                                                                  | *
 * |   vsgWorld *wrl       The VRML world.                                        | *
 * |   SFNode node         The concerned node.                                    | *
 * |   char const *field_name   Field name.                                       | *
 * |   float x, y, z, v    The SFRotation value.                                  | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_setNodeSFRotation(vsgWorld *wrl, SFNode node, const char *field_name, double x, double y, double z, double v) {
  const char *funcName = "TsmGeoVRML_setNodeSFRotation";
  vsgEventInDesc evin;
  symbol_t field_intern;

  CHECK_NOT_NULL0(wrl);
  CHECK_NOT_NULL0(node);

  if ((field_intern = vsgIntern(wrl, field_name)) == NULL) {
    tsmError(funcName, "Couldn't find field %s in scene", field_name);
    return;
  }

  evin = vsgSetFunc(node, field_intern);
  vsgSetSFRotation(wrl, node, evin, vsgMakeSFRotation(x, y, z, v));
}


/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_appendVRML                                                  | *
 * | Description:                                                                 | *
 * |   Appends the second string to the end of the first.  The function will      | *
 * |   allocate enough memory to hold all the contents of the two strings.        | *
 * | Parameters:                                                                  | *
 * |   char **string     Reference to a char array.                               | *
 * |   char const *vrml  The string to be padded to the end of the first.         | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
static void TsmGeoVRML_appendVRML(char **string, char const *vrml) {
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

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_ixiy2uv                                                     | *
 * | Description:                                                                 | *
 * |   Converts coordinate specified in image space (ix, iy) to geo-referenced    | *
 * |   coordinate (u, v).  The conversion is done by multiplying the (ix, iy)     | *
 * |   by the appropriate ixiytoUV matrix.                                        | *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid     The Pyramid structure.                                | *
 * |   int level            The level the coordinate is specified in.             | *
 * |   int ix, iy           The image space coordinate.                           | *
 * |   double *u, *v        Reference to the variable that will hold the          | *
 * |                        calculated geo-referenced coordinate.                 | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_ixiy2uv(Pyramid *pyramid, int level, double ix, double iy, double *u, double *v) {
  float *ixiytoUV = pyramid->tspecLevel.val[level].imageTransformationTspec.ixiytoUV;

  *u = ix * ixiytoUV[0] + ixiytoUV[2];
  *v = iy * ixiytoUV[4] + ixiytoUV[5];

  return;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_uv2ixiy                                                     | *
 * | Description:                                                                 | *
 * |   Converts geo-referenced coordinate (u, v) into image space coordinate (ix, | *
 * |   iy).  Uses the appropriate uvtoIXIY matrix to perform the conversion.      | *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid     The Pyramid structure.                                | *
 * |   int level            The level the coordinate is specified in.             | *
 * |   double u, v          Geo-referenced coordinate.                            | *
 * |   int *ix, *iy         Reference to the variable that will hold the          | *
 * |                        calculated image space coordinate.                    | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_uv2ixiy(Pyramid *pyramid, int level, double u, double v, double *ix, double *iy) {
  float *uv2ixiy = pyramid->tspecLevel.val[level].imageTransformationTspec.uvtoIXIY;

  *ix = u * uv2ixiy[0] + uv2ixiy[2];
  *iy = v * uv2ixiy[4] + uv2ixiy[5];

  return;
}

int TsmGeoVRML_uv2gcc(Pyramid *pyramid, double u, double v, double elevation,
    double *x, double *y, double *z) {

  double lat, lon;

  if (pyramid->projectionTspec.projectionType == tspUtmProjection) {
    tsmUTMToLatLong(pyramid->projectionTspec.utmZoneNumber, u, v, &lat, &lon);
    return tsmLatLongToGeocentric(lat, lon, elevation, x, y, z);
  }
  else if (pyramid->projectionTspec.projectionType == tspLatLongProjection) {
    return tsmLatLongToGeocentric(v, u, elevation, x, y, z);
  }
  return FALSE;
}

int TsmGeoVRML_gcc2uv(Pyramid *pyramid, double x, double y, double z,
    double *u, double *v, double *elevation) {

  double lat, lon;
  int zone;

  tsmGeocentricToLatLong(x, y, z, &lat, &lon, elevation);
  if (pyramid->projectionTspec.projectionType == tspUtmProjection) {
    if (!tsmLatLongToUTM(lat, lon, &zone, u, v)
        || zone != pyramid->projectionTspec.utmZoneNumber) {
      return FALSE;
    }
    return TRUE;
  }

  *u = lon;
  *v = lat;
  return TRUE;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_getBoundingBox                                              | *
 * | Description:                                                                 | *
 * |   Determines the bounding box of a tile by using it's corner coordinates and | *
 * |   its minimum height and maximum height.  This method works well for a tile  | *
 * |   that covers only a small geographic area.  However, for tiles that cover   | *
 * |   large geographic area, the approximated bounding box will be less accurate.| *
 * | Parameters:                                                                  | *
 * |   Pyramid *pyramid        The Pyramid structure.                             | *
 * |   TileCoord *coord        Specifies which tile we are processing.            | *
 * |   double min              Minimum height.                                    | *
 * |   double max              Maximum height.                                    | *
 * |   double *ll              On return, ll will contain the lower left          | *
 * |                           corner's coordinate (u, v, elevation).             | *
 * |   double *ur              On return, ur will contain the upper right         | *
 * |                           corner's coordinate (u, v, elevation).             | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_getBoundingBox(Pyramid *pyramid, TileCoord *coord, double min, double max, double *ll, double *ur) {
  SimpleTileSet *tileset = &pyramid->tspecLevel.val[coord->level];
  TileTspec *tile = &tileset->tileTspec;
  double ix, iy;

  ix = coord->tx * tile->numPixelsX;
  ll[1] = min;
  iy = coord->ty * tile->numPixelsY;
  TsmGeoVRML_ixiy2uv(pyramid, coord->level, ix, iy, &ll[0], &ll[2]);

  ix = (coord->tx < tileset->maxTile[0])
    ? (coord->tx+1) * tile->numPixelsX
    : tileset->maxValidPixel[0];
  ur[1] = max;
  iy = (coord->ty < tileset->maxTile[1])
    ? (coord->ty+1) * tile->numPixelsY
    : tileset->maxValidPixel[1];
  TsmGeoVRML_ixiy2uv(pyramid, coord->level, ix, iy, &ur[0], &ur[2]);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_calcRange                                                   | *
 * | Description:                                                                 | *
 * |   Calculate the range field of a GeoLOD node, based on the bounding box      | *
 * |   of the geometry passed in. The calculated range value would cause the      | *
 * |   GeoLOD to switch to a higher resolution data at the right time. This       | *
 * |   function is modified from Martin's tsmCalcRange function in tsmVRMLUtils.  | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRML *this    TsmGeoVRML instance.                                   | *
 * |   TileCoord *coord    Coordinate of the tile. Only tx and ty are used.       | *
 * |   double *ll          Lower left corner of the bouding box. Contains 3       | *
 * |                       elements (x, y, z).                                    | *
 * |   double *ur          Upper right corner of the bounding box (x, y, z).      | *
 * | Return value:                                                                | *
 * |   Value to be set in the range field of the GeoLOD node.                     | *
 * +------------------------------------------------------------------------------+ */
double TsmGeoVRML_calcRange(TsmGeoVRML *this, TileCoord *coord, double *ll, double *ur) {
  double          geo_size;       /* geom dimensions */
  double          tex_width, tex_height, tex_size;       /* texture dimensions */
  SimpleTileSet   *tileset;
  TileTspec       *tile;

  /* We use the following constants for the calculation:              */
  /*                                                                  */  
  /* 1. "PROJ_F" equals the distance from the tip of the perspective  */
  /* pyramid to the viewport. This can be calculated from the display */
  /* field of view by the equation 1 / tan( fov / 2.0 ). We assume a  */
  /* default field of view of 45 degrees.                             */
  /*                                                                  */  
  /* 2. "PROJ_SCR" represents the smallest value of the display's     */
  /* width and height, in pixels. It is this value that the field of  */
  /* view setting refers to in VRML.                                  */

  tileset = &this->oiPyramid->tspecLevel.val[coord->level];
  tile    = &this->oiPyramid->tileTspec;

  /* Now we want to be able to calculate the size of the geometry and */
  /* the number of pixels in the texture map that is applied to that  */
  /* geometry. This is done for both the X and Y axes.                */

  geo_size = max( max( ur[0] - ll[0], ur[1] - ll[1] ), ur[2] - ll[2] );

  /* Find the valid number of pixels in the texture map. This is  */
  /* the number of pixels that are mapped onto the above geometry */

  if ( coord->tx < tileset->maxTile[0] )
    tex_width = tile->numPixelsX;
  else {
    tex_width = tileset->numValidPixel[0] % tile->numPixelsX;
    if ( tex_width == 0 ) tex_width = tile->numPixelsX;
  }

  if ( coord->ty < tileset->maxTile[1] )
    tex_height = tile->numPixelsY;
  else {
    tex_height = tileset->numValidPixel[1] % tile->numPixelsY;
    if ( tex_height == 0 ) tex_height = tile->numPixelsY;
  }

  tex_size = max( tex_height, tex_width );
  
  /* Finally we can do the reverse perspective calculation. Normally */
  /* we can find the (x',y') screen coordinates from an (x,y,x)      */
  /* world coordinate by computing the equation:                     */
  /*       x' = ( screen_width * x * f ) / z                         */
  /*       y' = ( screen_height * y * f ) / z                        */
  /* Here we turn this round in terms of the distance variable, z.   */

  return ( geo_size * 0.5 * PROJ_F * PROJ_SCR ) /
         ( tex_size * this->params->texel_scale );
}

uchar *TsmGeoVRML_convertToRGBBuffer(Pyramid *pyramid, uchar *buffer) {
  static uchar *newBuffer = NULL;
  TileTspec *tile = &pyramid->tileTspec;
  int x, y;
  long p1, p2;

  if (newBuffer != NULL) tsmMemFree(buffer);

  if (tile->numComponentsPixel == 1 && tile->numBytesPerComponent == 1) {
    newBuffer = (uchar *) tsmMemMalloc(tile->numPixelsX * tile->numPixelsY * 3);
    for (y=0, p1=0, p2=0; y<tile->numPixelsY; y++) {
      for (x=0; x<tile->numPixelsX; x++, p1++) {
        newBuffer[p2++] = newBuffer[p2++] = newBuffer[p2++] = buffer[p1];
      }
    }
    return newBuffer;
  }
  else return buffer;
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_calcNormal                                                  | *
 * | Description:                                                                 | *
 * |   Calculates the surface normal at the specified position.  Neighbouring     | *
 * |   elevation are queried and used to calculate the normal.                    | *
 * | Parameters:                                                                  | *
 * |   TsmGeoVRMl *this    The TsmGeoVRML object.                                 | *
 * |   TileCoord *coord    The position at which we are calculating the normal.   | *
 * |   double height       Elevation at that given position.  Instead of having   | *
 * |                       the function query the elevation another time, it is   | *
 * |                       passed in from the TsmGeoVRML_createGeoElevationGrid   | *
 * |                       function to slightly improve performance.              | *
 * |   double *normal      The array that will hold the calculated normal.        | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_calcNormal(TsmGeoVRML *this, TileCoord *coord, double height, double dx, double dy, double *normal) {
  int valid[4];
  double d[4][3];
  double n[3];
  TileCoord *coord2 = TileCoord_new1(coord);
  SimpleTileSet *tileset = &this->oiPyramid->tspecLevel.val[coord->level];
  int i;

  valid[0] = valid[1] = valid[2] = valid[3] = 1;

  coord2->ix = coord->ix + dx;
  coord2->iy = coord->iy;
  if (coord2->ix <= tileset->maxValidPixel[0]) {
    d[0][0] = 1;
    d[0][1] = TsmGeoVRML_getElevation(this, coord2) - height;
    d[0][2] = 0;
  }
  else valid[0] = 0;

  coord2->ix = coord->ix;
  coord2->iy = coord->iy + dy;
  if (coord2->iy <= tileset->maxValidPixel[1]) {
    d[1][0] = 0;
    d[1][1] = TsmGeoVRML_getElevation(this, coord2) - height;
    d[1][2] = 1;
  }
  else valid[1] = 0;

  coord2->ix = coord->ix - dx;
  coord2->iy = coord->iy;
  if (coord2->ix >= tileset->minValidPixel[0]) {
    d[2][0] = -1;
    d[2][1] = TsmGeoVRML_getElevation(this, coord2) - height;
    d[2][2] = 0;
  }
  else valid[2] = 0;

  coord2->ix = coord->ix;
  coord2->iy = coord->iy - dy;
  if (coord2->iy >= tileset->minValidPixel[1]) {
    d[3][0] = 0;
    d[3][1] = TsmGeoVRML_getElevation(this, coord2) - height;
    d[3][2] = -1;
  }
  else valid[3] = 0;

  TileCoord_destroy(&coord2);

  normal[0] = normal[1] = normal[2] = 0;
  for (i=0; i<4; i++) {
    if (valid[i] && valid[(i+1)%4]) {
      TsmGeoVRML_crossProduct(d[i], d[(i+1)%4], n);
      normal[0] += n[0];
      normal[1] += n[1];
      normal[2] += n[2];
    }
  }
  TsmGeoVRML_normalize(normal);
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_crossProduct                                                | *
 * | Description:                                                                 | *
 * |   Calculate cross product of two 3-space vectors.                            | *
 * | Parameters:                                                                  | *
 * |   double *v1      First vector.                                              | *
 * |   double *v2      Second vector.                                             | *
 * |   double *result  The result, obviously.                                     | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_crossProduct(double *v1, double *v2, double *result) {
  result[0] = v1[1] * v2[2] - v1[2] * v2[1];
  result[1] = v1[2] * v2[0] - v1[0] * v2[2];
  result[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/* +------------------------------------------------------------------------------+ *
 * | Name: TsmGeoVRML_normalize                                                   | *
 * | Description:                                                                 | *
 * |   Normalizes a 3-space vector.                                               | *
 * | Parameters:                                                                  | *
 * |   double *v   The vector to be normalized.  On return, the array will hold   | *
 * |               the normalized vector.                                         | *
 * | Return value:                                                                | *
 * |   None.                                                                      | *
 * +------------------------------------------------------------------------------+ */
void TsmGeoVRML_normalize(double *v) {
  double len;

  len = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  if ( len != 0.0 ) {
    v[0] /= len;
    v[1] /= len;
    v[2] /= len;
  }
}
