/* -*- tab-width: 2; -*-

  Name: ThreeDCore.h

  Description: Internal header file for Core TerraVision functions.

  Revision History:
      $Id: ThreeDCore.h,v 2.38.2.9 2002/10/31 05:26:35 reddy Exp $

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
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#ifndef THREEDCORE_H
#define THREEDCORE_H

#include <prtypes.h>

typedef struct tvBase tvBase;
typedef struct tvViewpoint tvViewpoint;

#include "Trace.h"
#include "TsGeoPyramid.h"
#include "Paths.h"

#include <pthr/list.h>
#include <glt/glm.h>

/* Doc++ Documentation: *//**
 *   Encapsulates viewing parameters. Members:
 * <pre>
 * struct tvViewpoint {
 *   double eye[3];
 *   float altitude;
 *
 *   double reference[3];
 *   float reference_plane[4];
 *
 *   float nearPlane;
 *   float farPlane;
 *   int   fov;
 *   float aspect;
 *
 *   float yaw, pitch, roll;
 *   float vel, accel, base_accel;
 * };
 * </pre>
 */
struct tvViewpoint {
  double eye[3];
  float altitude;

  double reference[3];
  float reference_plane[4];

  float nearPlane;
  float farPlane;
  int   fov;
  float aspect;

  float yaw, pitch, roll;
  float vel, accel, base_accel;
};

/* Doc++ Documentation: *//**
 *   Enumeration of the names of state variables for Core behavior, which may 
 *   be set/retrieved from/by external clients.  The names are:
 *
 * <pre>
 * NAME                          TYPE   DESCRIPTION
 * ----                          ----   -----------
 * TV_SHOW_LIGHTING              bool   show lighting effects
 * TV_SHOW_HEADS_UP              bool   show heads-up display   
 * TV_SHOW_WIREFRAME             bool   show as wireframe models
 * TV_SHOW_TEXTURES              bool   apply texture mapping 
 * TV_SHOW_MODELS                bool   show VRML models 
 * TV_SHOW_SKY                   bool   render sky 
 * TV_SHOW_GROUND                bool   render ground not covered by datasets
 * TV_SHOW_DATASETS              bool   render datasets
 * TV_SHOW_FRUSTUMS              bool   show viewer's frustrum 
 * TV_SHOW_TILES                 bool   show tile bounds
 * TV_SHOW_PATHS                 bool   show path trajectories 
 * TV_SHOW_LOGO                  bool   show SRI logo
 * TV_SHOW_CROSSHAIR             bool   show crosshair for flight controls
 * TV_CROSSHAIR                  int[2] crosshair position
 * TV_FEATURES_WIREFRAME         bool   show VRML features as wireframe
 * TV_FEATURES_TEXTURED          bool   apply textures to VRML features
 * TV_FEATURES_LIGHTING          bool   apply lighting models to VRML features
 * TV_FEATURES_LOAD_TEXTURES     bool   load textures assoc. w/VRML features
 * TV_FEATURES_BACKFACE_CULL     bool   enable backface culling from VRML features
 * TV_TIME_OF_DAY                float  set time of day (in hours, [0..24])
 * TV_RESOLUTION_ADJUST          float  resolution adjust (?????)
 * TV_ELEVATION_SCALE            float  scale factor for elevation
 * TV_BACKFACE_CULL              bool   enable backface culling for geodata
 * TV_HORIZON_CULL               bool   enable horizon culling (????)
 * TV_USE_ZBUFFER_HACK           bool   enable z-buffer hack (????)
 * TV_TWOD_CONSTRAIN             bool   constrain view to down-looking
 * TV_MIPMAP_TEXTURES            bool   make use of MIPMAPS in texture ops
 * TV_FOG_DENSITY                float  set the fog density [0..1]
 * TV_HEIGHT_BASED_NAVIGATION    bool   enable height-contrained navigation
 * TV_DOODLE_ON_TERRAIN          bool   permit annotations on top of terrain
 * TV_FREEZE_TILES               bool   freeze tiles (????)
 * TV_SHOW_TILE_DEBUG            bool   show tile debugging information
 * TV_STEREO                     bool   calculate stereo views
 * TV_SCALE_IPD                  bool   ??????
 * TV_ABSOLUTE_IPD               float  ??????
 * TV_VISIBLE_CYCLE              int[?] huh????
 * TV_DIS_SEND                   bool   ?????
 * TV_DIS_MONITOR                bool   ?????
 * TV_DIS_ID                     int[?] ?????
 * TV_DIS_SITE_ID                int    ?????
 * TV_DIS_FORCE_ID               int    ?????
 * TV_DIS_ENTITY_ID              int    ?????
 * TV_SIZE                       int[2] width and height of viewport
 * TV_VELOCITY                   float  the current motion velocity (m/s)
 * TV_ACCELERATION               float  the current acceleration
 * TV_IMAGE_QUALITY              float  1.0 = best image quality
 * TV_SLIDE_DELTA                float[2] "slide" parameters for movement
 * TV_ATTITUDE_DELTA             float[2] "attitude" parameters for movement
 * TV_SPLASH_MESSAGE             string message display on splash screen
 * TV_MOTION_THRESHOLD           float  threshold to drop to wireframe (Hz)
 * TV_TILE_BY_TILE               bool   tile by tile display
 * TV_DOTGEO_PAUSE               int    ms pause between .geo requests
 * TV_AUTO_POP_FRONT             bool   auto pop hires dataset to top
 * TV_SCHEMA_URL                 string url for the .geo schema
 * TV_HYPERLINK_URL              string url for hyperlink under cursor
 * TV_HYPERLINK_DESC             string description for hyperlink
 * TV_MOVING                     bool   is viewpoint moving?
 * TV_POPPING_STRATEGY           int    enum-val of type VisiblePoppingStrategy
 * TV_SHOW_HEADS_UP_DEBUG        bool   showHeadsUp (debug info)
 * TV_SHOW_LATLONG               bool   Draw latitude/longitude lines
 * TV_OAA_ID                     string this viewer's OAA ID string
 * TV_DOTGEO_COMPRESS            bool   gzip compress .geo results
 * TV_DOTGEO_QUERY_TO_TOP        bool   query to top of cell hierarch
 * TV_DOTGEO_MAX_RESULTS         int    max number of results returned
 * TV_DOTGEO_CACHE_CELLS         bool   cache cell requests 
 * TV_DOTGEO_PROFILE_URL         string URL to .geo profile 
 * </pre>
 * @see  tvGetBoolean, tvSetBoolean, tvGetFloat, tvSetFloat, tvGetFloatv,
 *       tvSetFloatv, tvGetInteger, tvSetInteger, tvGetIntegerv, tvSetIntegerv,
 *       tvGetDouble, tvSetDouble, tvGetDoublev, tvSetDoublev, 
 *       VisiblePoppingStrategy
 */
enum tvVariable {
  TV_SHOW_LIGHTING,		/* Boolean => showLighting */
  TV_SHOW_HEADS_UP,		/* Boolean => showHeadsUp */
  TV_SHOW_WIREFRAME,		/* Boolean => showWireframe */
  TV_SHOW_TEXTURES,		/* Boolean => showTextures */
  TV_SHOW_MODELS,		/* Boolean => showModels */
  TV_SHOW_SKY,			/* Boolean => showSky */
  TV_SHOW_GROUND,		/* Boolean => showGround */
  TV_SHOW_DATASETS,		/* Boolean => showDatasets */
  TV_SHOW_FRUSTUMS,		/* Boolean => showFrustums */
  TV_SHOW_TILES,		/* Boolean => showTiles */
  TV_SHOW_PATHS,		/* Boolean => showPaths  */
  TV_SHOW_LOGO,			/* Boolean => showLogo */

  TV_SHOW_CROSSHAIR,		/* Boolean => showCrossHair */
  TV_CROSSHAIR,			/* Integerv => crossHair[2] */

  TV_FEATURES_WIREFRAME,
  TV_FEATURES_TEXTURED,
  TV_FEATURES_LIGHTING,
  TV_FEATURES_LOAD_TEXTURES,
  TV_FEATURES_BACKFACE_CULL,

  TV_TIME_OF_DAY,
  TV_RESOLUTION_ADJUST,
  TV_ELEVATION_SCALE,
  TV_BACKFACE_CULL,
  TV_HORIZON_CULL,
  TV_USE_ZBUFFER_HACK,
  TV_TWOD_CONSTRAIN,
  TV_MIPMAP_TEXTURES,
  TV_FOG_DENSITY,

  TV_HEIGHT_BASED_NAVIGATION,
  TV_DOODLE_ON_TERRAIN,

  TV_FREEZE_TILES,
  TV_SHOW_TILE_DEBUG,

  TV_STEREO,
  TV_SCALE_IPD,
  TV_ABSOLUTE_IPD,

  TV_VISIBLE_CYCLE,

  TV_DIS_SEND,
  TV_DIS_MONITOR,
  TV_DIS_ID,
  TV_DIS_SITE_ID,
  TV_DIS_FORCE_ID,
  TV_DIS_ENTITY_ID,

  TV_SIZE,                      /* Integerv => width, height */

  TV_VELOCITY,                  /* Float => current motion velocity */
  TV_ACCELERATION,              /* Float => current acceleration */

  TV_IMAGE_QUALITY,             /* Float => current image quality (1.0 best) */

  TV_SLIDE_DELTA,
  TV_ATTITUDE_DELTA,

  TV_SPLASH_MESSAGE,		/* String => message displayed on splash */

  TV_MOTION_THRESHOLD,          /* Float => threshold to drop to wireframe */

  TV_TILE_BY_TILE,		/* Boolean => tileByTile display */

  TV_DOTGEO_PAUSE,		/* Integer => ms pause between .geo requests */

  TV_AUTO_POP_FRONT,            /* Boolean => auto pop hires sets to front */

  TV_SCHEMA_URL,                /* String => set and load new schema file */

  TV_HYPERLINK_URL,             /* String => url for hyperlink under cursor */
  TV_HYPERLINK_DESC,            /* String => description for hyperlink */

  TV_MOVING,                    /* Boolean => is viewpoint moving or still? */

  TV_POPPING_STRATEGY,          /* Integer, of type VisiblePoppingStrategy */

  TV_SHOW_HEADS_UP_DEBUG,	/* Boolean => showHeadsUp (debug info) */

  TV_SHOW_LATLONG,              /* Boolean => Draw latitude/longitude lines. */

  TV_OAA_ID,                    /* String => this viewer's OAA ID string */

  TV_DOTGEO_COMPRESS,           /* Boolean => gzip compress .geo results */
  TV_DOTGEO_QUERY_TO_TOP,       /* Boolean => query to top of cell hierarch */
  TV_DOTGEO_MAX_RESULTS,        /* Integer => max number of results returned */
  TV_DOTGEO_CACHE_CELLS,        /* Boolean => cache cell requests */
  TV_DOTGEO_PROFILE_URL,        /* String => URL to .geo profile */

  /* This is just a placeholder */
  TV_VARIABLE_END
};
typedef enum tvVariable tvVariable;

#define tvEnable(core,var) tvSetBoolean(core, var, True)
#define tvDisable(core,var) tvSetBoolean(core, var, False)

#ifdef __cplusplus
extern "C" {
#endif

extern PRBool tvSetBoolean (tvBase *, tvVariable, PRBool);
extern PRBool tvGetBoolean (tvBase const *, tvVariable);

extern int tvSetInteger (tvBase *, tvVariable, int);
extern int tvGetInteger (tvBase const *, tvVariable);
extern int tvSetIntegerv (tvBase *, tvVariable, int *);
extern int tvGetIntegerv (tvBase const *, tvVariable, int *);

extern float tvSetFloat (tvBase *, tvVariable, float);
extern float tvGetFloat (tvBase const *, tvVariable);
extern int tvSetFloatv (tvBase *, tvVariable, float *);
extern int tvGetFloatv (tvBase const *, tvVariable, float *);

extern double tvSetDouble (tvBase *, tvVariable, double);
extern double tvGetDouble (tvBase const *, tvVariable);
extern int tvSetDoublev (tvBase *, tvVariable, double *);
extern int tvGetDoublev (tvBase const *, tvVariable, double *);

extern char const * tvSetString (tvBase *, tvVariable, char const *);
extern char const * tvGetString (tvBase const *, tvVariable);

extern char const * tvName (tvBase const *);

extern Path * tvGetPath (tvBase const *);

/* These are exposed for use elsewhere... */
extern tvBase* tvBaseCreate(void *, char const *, glmWindow);
extern void tvBaseStart (tvBase *threeD);
extern void tvBaseStop (tvBase *threeD);
extern void tvBaseDestroy(tvBase*);
extern void tvStartMovement (tvBase *);
extern void tvStopMovement (tvBase *);
extern void tvStartAccelerate (tvBase *);
extern void tvStartDeccelerate (tvBase *);

extern void tvDrawLoadScreen( tvBase * );

extern tvBase ** tvWindows(int *n);
extern void tvForallWindows (void (*f) (tvBase *, void *), void *closure);
extern tvBase *tvFindWindow (int (*f) (tvBase *, void *),
				 void *closure);

extern void tvAddDataset (tvBase *, TsGeoPyramid *);
extern void tvRemoveDataset (tvBase *, TsGeoPyramid *);

extern int tvDatasetPriority (tvBase *, TsGeoPyramid *);
extern int tvDatasetSetPriority (tvBase *, TsGeoPyramid *, int prio);

extern int tvNumDatasets (tvBase const *threeD);
extern TsGeoPyramid * tvGetNthDataset (tvBase const *threeD, int order);
extern TsGeoPyramid * tvGetDataset (tvBase const *threeDCore,
				    char const *geoname);
extern TsGeoPyramid * tvFindDataset (tvBase const *threeD,
				     int (*f) (TsGeoPyramid *, void *),
				     void *closure);
extern void tvForallDatasets (tvBase const *threeD,
			      void (*f) (TsGeoPyramid *, void *),
			      void *closure);

extern void tvBeginStroke (tvBase *threeD, int x, int y);
extern void tvContinueStroke (tvBase *threeD, int x, int y);
extern void tvEndStroke (tvBase *threeD);

/* Doc++ Documentation: *//**
 *   Used in Update events to inform caller of type of update.  Values are:
 * <pre>
 * TV_FRUSTUM
 * TV_TILES
 * TV_MOVEMENT
 * TV_PATHS
 * TV_DATASETS
 * </pre>
 * 
 * @see tvSetUpdateHandler
 *
 */
enum tvLinkType {
  TV_FRUSTUM,
  TV_TILES,
  TV_MOVEMENT,
  TV_PATHS,
  TV_DATASETS
};
typedef enum tvLinkType tvLinkType;

extern int tvAddLink (tvBase *from, tvLinkType t, tvBase *to);
extern int tvRemoveLink (tvBase *from, tvLinkType t, tvBase *to);
extern list_t *tvGetLinks (tvBase *from, tvLinkType t);

extern void tvSetUpdateHandler (tvBase *threeD, void (*f) (void *, tvLinkType, tvBase *), void *closure);

extern void tvUpdateLinkHandler (tvBase *threeD,
				 void (*f) (void *, tvLinkType),
				 void *closure);

/* Doc++ Documentation: *//**
 *   Enumeration of message-types that may be passed back to client from 
 *   a rendering thread.  Values are:
 * <pre>
 *   TV_ERROR_MSG
 *   TV_STATUS_MSG
 *   TV_BUSY_MSG
 *   TV_MESSAGE_MSG,
 *   TV_CONSOLE_MSG,
 *   TV_TERRAIN_ADD_MSG,
 *   TV_MODEL_ADD_MSG,
 *   TV_TERRAIN_DEL_MSG,
 *   TV_MODEL_DEL_MSG,
 * </pre>
 */

enum tvMessageType {
  TV_ERROR_MSG,       /* an error occurred */
  TV_STATUS_MSG,      /* update the status bar */
  TV_BUSY_MSG,        /* dataset loading message (not used?) */
  TV_MESSAGE_MSG,     /* display a message */
  TV_TERRAIN_ADD_MSG, /* notify that .geo has loaded a terrain model */
  TV_MODEL_ADD_MSG,   /* notify that .geo has loaded a 3D model */
  TV_TERRAIN_DEL_MSG, /* notify that .geo has unloaded a terrain model */
  TV_MODEL_DEL_MSG,   /* notify that .geo has unloaded a 3D model */
  TV_CONSOLE_MSG     /* dataset console message */
}; 
typedef enum tvMessageType tvMessageType;

/* Doc++ Documentation: *//**
 *   It is possible to register a callback function that is called
 *   during the render process, after TerraVision has drawn all of
 *   its data. This allows the user to insert their own drawings
 *   into the frame buffer.
 *
 *   The call to do this is tvSetPostDrawCallback, and you pass it
 *   a tvPostDrawCallback function pointer along with a closure 
 *   pointer (may be NULL). The closure is passed back to you 
 *   when the callback is made.
 *   
 */

typedef void (*tvPostDrawCallback) (tvBase *, void *);

extern tvPostDrawCallback tvSetPostDrawCallback( tvBase *, tvPostDrawCallback,
						 void * );


/* Doc++ Documentation: *//**
 *   Callback function to receive warning or error messages from a rendering
 *   thread.
 *
 * @param closure opaque pointer passed in.
 * @param which   classification of message. Values may be:
 * <pre>
 *   TV_ERROR_MSG
 *   TV_STATUS_MSG
 *   TV_BUSY_MSG
 * </pre>
 * @param fmt     message text  (???)
 * @param args    ????
 *
 * @see tvSetMessageHandler, tvMessageType
 *
 */
typedef void (*tvMessageHandler) (void *, tvMessageType which,
				  char const *fmt, va_list args);

extern void tvSetMessageHandler (tvBase *threeD,
				 tvMessageHandler f, void *closure);

extern void tvSetInitialView (tvBase *threeD, tvViewpoint const *view);

extern void tvCurrentView (tvBase *threeD, tvViewpoint *view);
extern void tvDeltaView (tvBase *threeD,
			 double forw, double right, double up,
			 tvViewpoint *viewp);

extern int tvAdjustView (tvBase *, tvViewpoint *view,
			 double forward, double right, double up);

extern void tvSetView (tvBase *threeD, tvViewpoint const *view, PRBool jump);
extern void tvSetView3d (tvBase *threeD,
			 double x, double y, double z,
			 double yaw, double pitch, double roll);
extern void tvSetViewGDC3d (tvBase *threeD, 
			    double lat, double lon, double elev,
			    double yaw, double pitch, double roll);


extern void tvGetView3d (tvBase *threeD,
			 double *x, double *y, double *z,
			 double *yaw, double *pitch, double *roll);
extern void tvGetViewGDC3d (tvBase *threeD,
			    double *lat, double *lon, double *elev);

extern void tvScreenToGCC( tvBase *, int x, int y, double gcc[3] );
extern void tvLookAtGCC( tvBase *threeDCore, double gcc[3] );

extern void tvSaveScreen( tvBase *, int width, int height, char const * );

extern void tvBaseCheckForHyperlink(tvBase *threeD, int x, int y);

extern void tvFlyToPickPoint(tvBase *threeD, int mx, int my );

extern float tvBasePointElevation( tvBase *threeD, double eye[3] );
extern float tvBasePointElevationGDC( tvBase *threeD, double gdc[3] );


#ifdef __cplusplus
}
#endif

#endif /* THREEDCORE_H */
