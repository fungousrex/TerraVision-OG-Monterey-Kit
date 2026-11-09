/* -*- tab-width: 2; -*-

  Name: ThreeDCoreI.h

  Description: Internal header file for Core TerraVision functions.

  Revision History:
      $Id: ThreeDCoreI.h,v 2.58.2.22 2002/10/31 05:26:36 reddy Exp $

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

#ifndef THREEDCOREI_H
#define THREEDCOREI_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdarg.h>

#include <glt/glt.h>
#include <glt/glm.h>

#include <prtypes.h>
#include <prcvar.h>

#ifdef HAVE_LIBGEO
#include <libGeo_DiscClient/DiscoverClientCAPI.h>
//#include <libGeo_DiscClient/PublicationClientCAPI.h>
#endif

#ifndef TVGLX_H
typedef struct RenderPrimitiveCache RenderPrimitiveCache;
#endif

typedef struct RenderPrimitive RenderPrimitive;
typedef struct GeoElement GeoElement;
typedef struct tvBaseGeo tvBaseGeo;

#ifndef ThreeDWidget_typedef
#define ThreeDWidget_typedef
typedef struct ThreeDWidget ThreeDWidget;
#endif

#include "ThreeDCore.h"
#include "Features.h"
#include "OAA.h"
#include "Paths.h"
#include "TVGLX.h"
#include "Visible.h"
#include "matrix.h"

#include <pthr/istack.h>
#include <pthr/eqhash.h>

#define THREED_INIT_FOV   20               /* Initial field of view */
#define THREED_INIT_NEAR  10               /* Initial near plane value */
#define THREED_INIT_FAR   200000           /* Initial far plane value */

#define MIN_NEAR_PLANE 5.0

#define MAX_STARS 300

#define VIS_THREAD

#ifdef __cplusplus
extern "C" {
#endif

extern int use_antialiasing;       /* USE OPENGL ANTIALIASING */
extern int use_texture_objects;    /* USE OPENGL TEXTURE OBJECTS */
extern int use_vertex_array;       /* USE OPENGL VERTEX ARRAYS */

struct tvBaseGeo {
  tvBase *threeD;
  TsGeoPyramid *geo;
};

/*
 * Doc++ Description: *//** 
 *   A description of the rendering geometry of a TsTile.
 *
 * @param len_coordIndex The length of the coordIndex vector
 * @param coordIndex     The sequence of coordinates to render
 * @param len_coords     The length of the two coordinate vectors
 * @param coords         The positional coordinates indexed by coordIndex
 * @param texCoords      The texture coordinates indexed by coordIndex
 *
 * @see TsGeoPyramidType, TsCache, TsGeoCalls
 *
 */

typedef struct TileGeometry {
  PRUint16 len_coordIndex;
  PRUint16 *coordIndex;
  PRUint16 len_triStrips;
  PRUint16 *triStrips;
  PRUint16 len_coords;
  PRUint16 const *neighIndex;
} TileGeometry;

struct RenderPrimitive {
  unsigned char  sig;
  GLuint         disp_list;

  TileGeometry const *geometry;
  TsCoord *coords;
};

struct GeoElement {
  tvBase *           threeD;
  TsGeoPyramid *     geo;
  TsGeoView *        geoView;
  QuadTile *         quad;
  TextureImage *     texImage;
};

typedef struct VisibleDrawGeoList {
  PRUint16 n;
  PRUint16 prepared;
  TextureImage *texim;
  TsGeoPyramid *geo;
  TsGeoView *geoView;
  VisibleDraw *vis;
} VisibleDrawGeoList;

typedef struct VisibleDrawList {
  PRUint32 id;
  PRUint16 n, ntiles, maxTiles;
  eqhash_t in_list;
  VisibleDrawGeoList *geo_list;
  tvViewpoint view;
  TsBBox bbox; /* This is in the GCC coords */
} VisibleDrawList;

extern void VisibleDrawInit (VisibleDrawList *list);
extern void VisibleDrawGeoClear (VisibleDrawGeoList *list);
extern void VisibleDrawClear (VisibleDrawList *list);
extern void VisibleDrawDestroy (VisibleDrawList *list);

typedef void (*PickCB)(tvBase *threeD, int res, int tx, int ty, 
		       double p[], void *data); 

typedef enum {
  TVC_SLIDE,
  TVC_ATTITUDE
} tvBaseMotion;

struct tvBase {
  ThreeDWidget *     widget;
  char const *       name;

  glmWindow          glmw;
  tvGLXContext *     tvGLX;

  float              zfar;
  GLmatrix	     viewMatrix;
  float	             resAdjust;

  tvViewpoint const *initView;
  /* True if the current viewpoint is considered to be valid. */
  PRUintn            viewValid:1;
  /* True if the movement to this viewpoint was a "jump". */
  PRUintn            viewJumped:1;

  /* True if graphics have been initialized. */
  PRUintn            graphics:1;
  /* True if OpenGL viewport needs update. */
  PRUintn            update_viewport:1;

  PRUintn            runnable:1;
  PRUintn	     mapped:1;
  PRLock             *lock;
  PRCondVar          *run_cond;

  PRInt32	     destroying;

  tvViewpoint        view;
  PRIntervalTime     moveTime;
  PRCondVar *        jumpReady;

  tvViewpoint        freezeView;
  GLmatrix           freezeViewMatrix;

  /* These fields are used for calculating the dynamic near/far planes. */
  int                computedNearFar;
  TsTile const *     nearTile;
  double             nearPoint[3];
  int                inside;
  TsTile const *     farTile;
  double             farPoint[3];

  char const *       splash_message;

  int		     renderTime;
  short	             weather;
  float              fog_params[5];
  float              timeOfDay;
  float              stars[MAX_STARS][3];
  int                frame;
  float              frame_rate;       /* instantaneous frame rate (Hz) */
  float              frame_rate_av;    /* averaged frame rate (Hz) */
  float  	     ipd;	       /* inter-pupillary distance in meters */
  float              elevScaleFactor;

  GLuint             bgList;
  PRBool	     newBgList;
  double             bgEyeZ;
  GLboolean          bgTextured;

#ifdef VIS_THREAD
  PRThread          *visible_thread;
  PRCondVar         *vis_cond;
  PRLock            *visible_lock;
#endif

  PRInt32            redo_quad;
  PRInt32            redo_vislist;

#ifdef HAVE_LIBGEO
  PRThread           *dotgeo_thread;
  int                dotgeo_pause;
  PRLock             *dotgeo_lock;
#endif

  istack *	     is0;
  istack *	     is1;
  eqhash_t           treehash0;
  eqhash_t           treehash1;
  int		     rerequest;

  VisiblePoppingStrategy poppingStrategy;
  VisibleList        visibleList[2];
  VisibleList *      workingList0;
  VisibleList *      workingList1;

  TextureCache *     textureCache;
  
  RenderPrimitiveCache *rp_cache;

  int sunLight_list;

  PRBool	     cave;		/* is this a CAVE widget? */
  PRBool	     selecting;	        /* we're trying to pick a point now */
  PRBool             select_features_only; /* only pick features not terrain */
  int		     pickScreen[2];
  double	     pickPoint[3];
  PickCB 	     pickCB;		/* called when a point is selected */
  void               *pickData;

  /* feature picking (hyperlink) support */
  
  int                f_pick_id;         /* picked feature ID */
  int                f_pick_anchor;     /* no. of anchor picked in feature */
  char               *f_pick_url;       /* url of hyperlink */
  char               *f_pick_desc;      /* description of hyperlink */

  /* feature display fields */

  list_t *           fselIDs;          /* list of feature IDs to render */

  list_t *           geoList;	       /* List of active GeoElements */

  int    	     width;
  int    	     height;
  
  /* Feature display settings */
  PRUintn            started:1;

  PRUintn            f_wireframe:1;/* display features in wireframe? */
  PRUintn            f_textured:1; /* texture map features? */
  PRUintn            f_lighting:1; /* put lighting on for features? */
  PRUintn            f_loadtextures:1; /* load textures for models? */
  PRUintn            f_backfacecull:1; /* do backface culling on all objects */

  PRUintn            useZBufferHack:1; /* z-buffer hack for dataset prior. */
  PRUintn            showLighting:1;
  PRUintn            headsUp:1;
  PRUintn            headsUpDebug:1;
  PRUintn            mipmapTextures:1;
  PRUintn            horizonCull:1;
  PRUintn            backfaceCull:1;
  PRUintn            showWireframe:1;
  PRUintn            showTextures:1;
  PRUintn            showVehicles:1;
  PRUintn            showTileDebug:1;
  PRUintn            freezeTiles:1;
  PRUintn            showModels:1;
  PRUintn            showSky:1;
  PRUintn            showGround:1;
  PRUintn            showDatasets:1;

  PRUintn     	     showFrustums:1;
  PRUintn     	     showTiles:1;
  PRUintn     	     showPaths:1;
  PRUintn            showLogo:1;
  PRUintn            twoDConstrain:1;
  PRUintn            showCrossHair:1;

  PRUintn            projectToTerrain:1;
  PRUintn            heightBasedNav:1;
  PRUintn            disSending:1;
  PRUintn            disMonitoring:1;

  PRUintn            tileByTile:1;
  PRUintn            stereoVisual:1;
  PRUintn            stereo:1;
  PRUintn            scaleIPD:1;
  PRUintn            auto_pop_to_top:1; /* auto order with hires on top */
  PRUintn            showLatLong:1;

  float              absoluteIPD;
  int                crossHair[2];

  float              motionThreshold;  /* when to drop to wireframe (Hz) */
  int                motionDegraded;   /* are we dropped to wireframe now? */

  VisibleDrawList    drawList[3];
  int                currDrawList;
  int                nextDrawList;
  int                currWorkList;
  int                nextWorkList;

  /* info for DIS interaction */
  int                disSiteID;
  int                disForceID;
  int                disEntityID;
  PRTime             disLastSend;

   /* info for paths */
  Path               *path;	/* the current path definition */
  Path               *old_path;	/* used to save value of path */
  int                playing;	/* TRUE if currently animating */
  int                flying;	/* TRUE if flying to a new loc */
  double             curr_index; /* normalised time index [0-1] */
  double             index_inc;	/* time increment */
  void *             pathTimer; /* Timer id of current path timer. */
  void *             drawTimer; /* Timer id for sentence drawing time out */
  void *             moveTimer; /* Timer id for movement */
  PRIntervalTime     timestamp;	/* timestamp of last render */

  tvBaseMotion       delta_motion;
  float              delta_x;
  float              delta_y;

  /* OAA related state */
  char               *oaa_id;   /* this viewer's OAA ID string */

  /* post draw callback function */
  tvPostDrawCallback post_draw_callback;  /* callback to issue after tiles */
  void               *post_draw_closure;  /* closure for post draw callback */

  /* handles a message to be delivered to user. */
  tvMessageHandler   msg_handler;
  void               *msg_closure;

  /* Reports that a (possibly) linked item has been updated. */
  void               (*update) (void *update_closure,
				tvLinkType link, tvBase *threeD);
  void               *update_closure;

  SentenceType       sentence;


  list_t         *linkedBases;	/* widgets needing a redraw when we move */
  list_t         *frustumLinks;	/* widgets whose frustum we display */
  list_t         *tileLinks;    /* widgets whose tiles we display */
  list_t         *moveLinks;	/* widgets whose view we control */
  list_t         *pathLinks;    /* widgets whose paths we display */

  float	         image_quality;  /* 1.0 = best image quality */

#ifdef HAVE_LIBGEO
	GeoCellNameSpecificationHandle	geoCellSpecHandle;
	GeoDiscoverClientHandle					geoDiscClientHandle;
	GeoFetchMetadataHandle					geoFetchMetadataHandle;
	GeoSearchProfileHandle					geoSearchProfileHandle;

	GeoCacheHandle									geoCellNameCache;
	char														geoCellNameCacheEnabled;
	
	GeoCacheHandle									geoGeoDataCache;
	char														geoGeoDataCacheEnabled;
  
	char														*geoSchemaURI;
  char                            *geoProfileURI;
  
	int                             geoCompress; /* gzip compress .geo results */
  int                             geoQueryToTop; /* query to top of hierarch */
  int                             geoMaxResults; /* max # results returned */
#endif /* HAVE_LIBGEO */
};

/* These invoke callbacks to the higher level */
extern void tvBaseAddTimer (tvBase *threeD, void **idp,
			    unsigned int msecs,
			    void (*alarm) (void *closure),
			    void *closure);
extern void tvBaseDelTimer (tvBase *threeD, void **idp);

extern void tvBaseError (tvBase const *threeD, char const *fmt, ...);
extern void tvBaseBusy (tvBase const *threeD, char const *fmt, ...);
extern void tvBaseStatus (tvBase const *threeD, char const *fmt, ...);
extern void tvBaseConsole (tvBase const *threeD, char const *fmt, ...);
extern void tvBaseMessage (tvBase const *threeD, tvMessageType msg, char const *fmt, ...);
extern void tvBaseUpdate (tvBase *threeD, tvLinkType link);

extern void tvBaseDraw(glmWindow glmw, void *closure);
extern void tvBaseDrawFrustumCB(tvBase *threeD, PRBool wireframe);
extern void tvBaseDrawFrustum(tvViewpoint *view, PRBool wireframe);
extern int tvBaseDrawPrimitive (VisibleDraw const *visible,
				VisiblePoppingStrategy poppingStrategy,
				PRBool selecting, PRBool selectModelsOnly);
extern void tvBaseFlyToPickPoint(tvBase *threeD,
				 int res, int tx, int ty, 
				 double p[], void *data);
extern void tvBaseMapNotify (glmWindow glmw, void *closure);
extern void tvBaseRedoVisibility (tvBase * threeD, int locked);
extern void tvBaseRedoVisibleList (tvBase * threeD, int locked);
extern void tvBasePostRedraw (tvBase *threeD, int locked);
extern void tvBaseNextDrawList (tvBase * threeD);
extern int tvBaseGrabDrawList (tvBase * threeD);
extern PRStatus tvBaseWaitRunnable (tvBase *threeD);
extern void tvBaseRegisterPickCB(tvBase*, int, int, PickCB, void*);
extern void tvBaseDrawProcessPick(tvBase *threeD,
				  int x, int y, double pick[3]);
extern tvViewpoint *tvBaseView(tvBase const *);
extern int tvBaseMoveView (tvBase *threeD, 
			   tvViewpoint *view,
			   double forward, double right, double up);
extern void tvBaseCacheCB (TsCache const *cache, 
			   void *arg, void *closure);
extern void tvBaseStartThreads (tvBase *);
extern void tvBaseStopThreads (tvBase *);
extern void tvBaseGeoViewMatrix (tvBase *threeD,
				 TsGeoPyramid const *geo,
				 GLmatrix M, GLmatrix P, 
				 GLmatrix PM);
extern void tvBaseGLViewpointLVCS (tvViewpoint const *view,
				   TsGeoPyramid const *geo);
extern void tvBaseGLViewpointCustomLVCS (tvViewpoint const *view,
					 double const geoOrigin[3],
					 float const geoOrient[4]);
extern float tvBasePointAltitude (tvBase * threeD, double *eye);
extern void tvBaseReset (tvBase *threeD);
extern void tvBaseVisibleThread (void *generic);
extern void tvBaseDotGeoThread (void *generic);
extern TsGeoPyramid * tvBaseTop (tvBase *threeD);

extern int  tvBaseGenerateVisible(tvBase * threeD,
				  tvViewpoint const *view,
				  int *count);
extern void tvBaseSetVisibleCycle (int min, int max);
extern void tvBaseVisibleCycle (int *min, int *max);
extern int tvBaseDISEnable (tvBase *threeD, tvVariable which, int val);

extern char *GetNextOAAID( void );

/*
  0  => Blue
  1  => Green
  2  => Cyan
  3  => Red
  4  => Magenta
  5  => Yellow
  6  => White
  7  => 50% Blue
  8  => 50% Green
  9  => 50% Cyan
  10 => 50% Red
  11 => 50% Magenta
  12 => 50% Yellow
  13 => 50% White
 */
extern void tvBaseTileColor (int idx);

#if HAVE_TEXSUBIMAGE
extern TextureImage * GetTextureImage (tvGLXContext *tvGLX,
				       TsGeoPyramid const *geo);
#endif

/* view-related functions... */
extern void tvBaseGetView (tvBase *, tvViewpoint *);
extern void tvBaseGetView_nolock (tvBase * threeD, tvViewpoint *view);
extern void tvBaseSetView (tvBase *, tvViewpoint const *, PRBool jump);
extern void tvBaseSetView_nolock (tvBase *, tvViewpoint const *, PRBool jump);
extern void tvBaseUpdateView (tvBase *, PRBool jump);
extern void tvBaseSetFrozenView (tvBase * threeD);
extern void tvBaseInitView(tvBase *, TsGeoPyramid const *);
extern void tvBaseCentralView(tvBase * threeD, TsGeoPyramid const *);

extern void tvBaseSetDISView (tvBase *threeD, int send_forced,
			      int forceID, int entityID);

extern void tvBaseSetSlideDelta (tvBase *, float dx, float dy);
extern void tvBaseSetAttitudeDelta (tvBase *, float dx, float dy);

extern RenderPrimitiveCache * RenderPrimitiveCacheCreate (void);
extern void CreateRenderPrimitive (VisibleDraw *vis,
				   VisibleDrawList *list,
				   VisibleDrawGeoList *geo_list,
				   RenderPrimitiveCache *rp_cache);

extern void CreateDrawList (tvBase * threeD,
			    VisibleList *visList,
			    VisibleDrawList *drawList,
			    tvViewpoint const *view);
extern void ClearWorkList(tvBase * threeD, VisibleList * list);
extern void ClearDrawList(tvBase * threeD, VisibleDrawList * list);

#define tvBaseLock(threeD) (\
  TSMTRACE((THREED_MASK|3,funcName,"lock")),\
  PR_Lock ((threeD)->lock),\
  TSMTRACE((THREED_MASK|3,funcName,"locked"))\
)

#define tvBaseUnlock(threeD) (\
  TSMTRACE((THREED_MASK|3,funcName,"unlock")),\
  PR_Unlock ((threeD)->lock)\
)

extern void trace_matrix (int trace_level,
			  char const *funcName, char const *name,
			  const_GLmatrix m);

#define TRACE_MATRIX(level,funcName,name,m) 			\
  (TSMTRACEP ((level))						\
   ? trace_matrix(level,funcName,name,(const_GLmatrix)m)	\
   : (void) 0)

extern TsGeoPyramid * tvBaseTop (tvBase *threeD);
extern float tvBaseAltitude (tvBase * threeD);

extern void tvHeading (m4f const *view,
		       float x, float y, float z,
		       v3f *h);

/* Vertex arrays were introduced in OGL 1.1 */
#ifdef GL_VERSION_1_1
#define HAVE_VERTEX_ARRAY  1
#endif

/* Under OGL 1.2, Vertex Arrays are no longer extensions */
#ifndef GL_VERTEX_ARRAY_EXT
#define GL_VERTEX_ARRAY_EXT GL_VERTEX_ARRAY
#endif

#ifndef GL_NORMAL_ARRAY_EXT
#define GL_NORMAL_ARRAY_EXT GL_NORMAL_ARRAY
#endif

#ifndef GL_TEXTURE_COORD_ARRAY_EXT
#define GL_TEXTURE_COORD_ARRAY_EXT GL_TEXTURE_COORD_ARRAY
#endif


#define tvHeadingForward(v,h,d) tvHeading(v,  0 ,  0 , -(d), h)
#define tvHeadingRight(v,h,d)   tvHeading(v, (d),  0 ,   0 , h)
#define tvHeadingUp(v,h,d)      tvHeading(v,  0 , (d),   0 , h)

#if defined(HAVE_INLINE) && !defined(NO_ThreeD_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "ThreeDIL.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* THREEDCORE_H */
