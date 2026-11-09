/* -*- tab-width: 8; -*-

  Name: ThreeDCore.c

  Description: Core TerraVision functionality;  rendering and management of 
      3D views.

  Revision History:
      $Id: ThreeDCore.c,v 2.119.2.40 2002/10/31 05:26:33 reddy Exp $

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

#include "TerraVisionInc.h"
#include "ThreeDCoreI.h"
#include "TsGeoPyramid.h"
#include "Texture.h"
#include "DIS.h"
#include "TsLogging.h"
#include "sri_logo.c"   /*RGBA bitmap for SRI logo */

#include <prinit.h>

#include <pthr/list.h>

#ifdef HAVE_LIBGEO
#include <libGeo_DiscClient/DiscoverClientCAPI.h>
//#include <libGeo_DiscClient/PublicationClientCAPI.h>
#endif

#include <limits.h>

#define AV_FRAME_COUNT   10           /* no. frames to avg frame rate over */

static float fogColorLight[3] = {0.75, 0.75, 0.75};
static float fogColorDark[3] = {0.0, 0.0, 0.0};

int white_list;
gltParamDef white_material[] =
{ { GL_DIFFUSE,   {1.0, 1.0, 1.0, 1.0} },
  { GL_SPECULAR,  {0.0, 0.0, 0.0, 1.0} },
  { GL_SHININESS, {0.0} },
  { GL_AMBIENT,   {0.1, 0.1, 0.1, 1.0} },
  GLT_PARAMDEF_END
};

int ground_list;
gltParamDef ground_material[] = 
{
  { GL_DIFFUSE, {1.0, 1.0, 1.0, 1.0} },
  { GL_AMBIENT, {0.4, 0.4, 0.4, 1.0} },
  GLT_PARAMDEF_END
};

int gray_list;
gltParamDef gray_material[] =
{
  { GL_DIFFUSE,   {0.5, 0.5, 0.5, 1.0} },
  { GL_SPECULAR,  {0.0, 0.0, 0.0, 1.0} },
  { GL_SHININESS, {0.0} },
  { GL_AMBIENT,   {0.4, 0.4, 0.4, 1.0} },
  GLT_PARAMDEF_END
};

int yellow_list;
gltParamDef yellow_material[] =
{
  { GL_DIFFUSE,   {1.0, 1.0, 0.0, 1.0} },
  { GL_SPECULAR,  {1.0, 1.0, 1.0, 1.0} },
  { GL_SHININESS, {128.0} },
  { GL_AMBIENT,   {0.8, 0.8, 0.8, 1.0} },
  GLT_PARAMDEF_END
};

gltParamDef sunLight[] =
{
  { GL_POSITION,  {1.0, -1.0, 1.0, 0.0} },
  { GL_DIFFUSE,   {1.0,  1.0, 1.0, 1.0} },
  { GL_AMBIENT,   {0.3,  0.3, 0.3, 1.0} },
  GLT_PARAMDEF_END
};

int use_antialiasing;       /* USE OPENGL ANTIALIASING */
int use_texture_objects;    /* USE OPENGL TEXTURE OBJECTS */
int use_vertex_array;       /* USE OPENGL VERTEX ARRAYS */

#define DRAWTIMES (0)  /*RLB: If this is "1", there are supporting structs.
                        * also required to support the functionality.
                        */

/* forward declarations... */
static void tvBaseInitializeGraphics(tvBase *base);
static void tvBaseSetRunnable(tvBase*, int);

static void tvGeoElementDestroy (GeoElement *elem);

static void checkVarDescs (void);

static void tvBaseMessageHandlerDefault (void *w, tvMessageType,
					 char const *fmt, va_list ap);

static void tvDrawOverlays (tvBase *threeD,
			    int newTextures, int totalTextures);
static void tvDrawCrossHair (tvBase const *threeD);
static void tvDrawHeadsUp (tvBase const * threeD,
			   int newTiles, int numTiles);

static void tvBaseTiming(int             numDrawnPolys,
			 int             newTextures,
			 int             totalTextures,
			 PRIntervalTime  t_start);
static void tvBaseDrawSphere (void);
/*static*/ void tvBaseDrawBackGround(tvBase * threeD, VisibleDrawList *drawList,
				 int *numPolys, int stereo, int stereoVisual,
				 int eye, PRIntervalTime t_start,
				 PRIntervalTime t_prev);
static void tvBaseDrawBoundingBoxes(tvBase *threeD, int id);
static void tvBaseOutlineDatasets(tvBase *threeD);
static void tvBaseDrawTiles (tvBase * threeD,
			     VisibleDrawList *  drawList,
			     int *          numPolys,
			     PRIntervalTime   t_start,
			     PRIntervalTime   t_prev);
static void tvBasePrepareTiles (tvBase * threeD,
				VisibleDrawList *  drawList,
				int *          numPolys,
				int *          newTextures,
				int *          totalTextures);
static void tvBaseDrawGeoTiles(tvBase * threeD,
			       VisibleDrawGeoList *  geoList,
			       int *          numPolys,
			       PRIntervalTime t_start,
			       PRIntervalTime t_prev);
static void tvBaseDebugTile (tvBase const *threeD,
			     VisibleDraw *vis);
void tvBaseFrameGrab(tvBase * threeD, unsigned long * imageBuffer);
static void tvBaseDrawFog (tvBase const * threeD);
static void tvBaseSetFogColor (tvBase * threeD, float const *color);
void tvBaseViewMatrix (tvBase *threeD, TsGeoPyramid const *geo,
		       GLmatrix M, GLmatrix P);
static void tvBaseGLPerspective(tvBase * threeD, 
				float nearPlane, float farPlane,
				int stereo, int leftEye);
static void tvBaseGLAttitude(tvBase * threeD);
void tvBaseEnableVArray (void);
static void tvBaseSetClipPlanes( tvBase *threeD );
static void tvBaseSRILogo( tvBase * threeD );
static void tvBaseCreateStars(tvBase *);
static void tvBaseProcessPick(tvBase *threeD, int hits, 
			      unsigned int selectBuf[]);
static void tvBaseSetSize( tvBase *threeD,
			   int width, int height);
static void tvBaseCalculateLighting(tvBase*);
static int tvDatasetSetPriority_nolock (tvBase *threeD, TsGeoPyramid *geo,
                                        int prio);

static void MultVecMatrix3(float const v[3], const_GLmatrixd m, float o[3]);

static list_t *allWindows;

/*
 * Synopsis: tvBase* tvBaseCreate(void *w, char const *name,
 *                                glmWindow glmw);
 *
 * Doc++ Description: *//**
 *   Creates a new instance of a tvBase struct, used as a handle to a
 *   rendering thread and its state.
 *
 * @param  w      opaque pointer back to an "owner", or containing object.
 * @param  name   an arbitrary string used as a title in various windows 
 *                created by the rendering thread.      
 * @param  glmw   an instance of an OpenGL/Motif/Xt window structure; see
 *                API for libglt.a.
 * 
 * @return new instance of a tvBase structure.
 *
 * @author
 *
 */
tvBase *
tvBaseCreate (void *w, char const *name, glmWindow glmw)
{
  const char* funcName = "tvBaseCreate";
  ThreeDWidget *widget = w;
  tvBase* base;
  glmContext glmCtx = glmWindowContext (glmw);
  glmContext baseCtx = glmBaseContext (glmCtx);
  tvGLXContext * tvGLX = tvGLX_FindContext (baseCtx);

  TSMTRACE ((GLT_MASK|THREED_MASK|1, funcName, "baseCtx %p tvGLX %p",
	     baseCtx, tvGLX));

  checkVarDescs();

  base = (tvBase*) calloc(sizeof(tvBase), 1);
  if (!base) {
    tsmError(funcName, "Can't initialize Base functions!");
    return NULL;
  }

  /* An opaque pointer to embedded widget. */
  base->widget = widget;
  base->name = name;

  /* this is hardcoded off for now... */
  base->cave = FALSE;

  /* create the lock for the renderer */
  if ( !(base->lock = PR_NewLock()) ) {
    tsmError(funcName, "Unable to create a mutex!");
    return NULL;
  }

#ifdef VIS_THREAD
  /* create the lock for the visibility threadr */
  if ( !(base->visible_lock = PR_NewLock()) ) {
    tsmError(funcName, "Unable to create a mutex!");
    return NULL;
  }
#endif

  if ( !(base->run_cond = PR_NewCondVar(base->lock))) {
    tsmError(funcName, "Unable to create run condition!");
    return NULL;
  }

#ifdef HAVE_LIBGEO
  /* create the lock for the dotgeo process */
  if ( !(base->dotgeo_lock = PR_NewLock()) ) {
    tsmError(funcName, "Unable to create a dotgeo mutex!");
    return NULL;
  }
#endif /* HAVE_LIBGEO */

  /* create the conditions... */
#ifdef VIS_THREAD
  base->vis_cond = PR_NewCondVar(base->lock);
  pthr_set_name(base->vis_cond, "C<visibility-3D>");
  base->visible_thread = 0;
#endif
  base->redo_quad = 1;
  base->redo_vislist = 0;

  /* The first view is always a jump. */
  base->viewJumped = TRUE;
  base->jumpReady = PR_NewCondVar(base->lock);

  base->started = FALSE;
  base->width = base->height = 0;
  base->ipd = 2.0;
  base->timeOfDay = 12;
  base->elevScaleFactor = 1.0;
  base->resAdjust = 1.0;
  base->horizonCull = TRUE;
  base->backfaceCull = TRUE;
  base->renderTime = 10;
#ifdef HAVE_LIBGEO
  base->dotgeo_thread = 0;
  base->dotgeo_pause = 1000;  /* ms pause between .geo discovery requests */
#endif
  base->bgList = 0;
  base->newBgList = FALSE;
  base->selecting = FALSE;
  base->select_features_only = FALSE;
  base->frame = 0;
  base->frame_rate = 0.0;
  base->frame_rate_av = 0.0;
  base->mapped = FALSE;
  base->heightBasedNav = TRUE;

#ifdef HAVE_VERTEX_ARRAY
  base->poppingStrategy = VisiblePopping_LowFrag;
#else
  /* popping strategy LowFrag/Res broken for non-vertex arrays */
  base->poppingStrategy = VisiblePopping_None;
#endif

  base->showFrustums = TRUE;
  base->showPaths = TRUE;
  base->showTiles = TRUE;
  base->showLogo = TRUE;
  base->showCrossHair = FALSE;
  base->disSending = FALSE;
  base->disMonitoring = FALSE;
  base->disLastSend = LL_INIT (0, 0);
  base->auto_pop_to_top = TRUE;
  base->showLatLong = FALSE;
  base->image_quality = 0.0;
  base->showModels = TRUE;

  base->motionThreshold = 3.0;   /* switch to wireframe when < 3.0 Hz */
  base->motionDegraded = FALSE;  /* have we dropped to wireframe? */

  base->splash_message = tsmMemStrdup( "Waiting for datasets..." );

  base->geoList = list_create (TRUE);

  base->f_pick_id     = -1;
  base->f_pick_anchor = -1;
  base->f_pick_url    = NULL;
  base->f_pick_desc   = NULL;

  /* Initialize istack. */
  base->is0 = istack_create (256 * sizeof (QuadTile));
  base->is1 = istack_create (256 * sizeof (QuadTile));
  base->treehash0 = eqhash_create (512);
  base->treehash1 = eqhash_create (512);
  TSMTRACE ((VIS_MASK|1, funcName, "treehash0 = %p", base->treehash0));
  TSMTRACE ((VIS_MASK|1, funcName, "treehash1 = %p", base->treehash1));

  base->path = NULL;
  base->old_path = NULL;
  base->initView = NULL;
  base->viewValid = FALSE;

  base->linkedBases = list_create(TRUE);
  base->frustumLinks = list_create(TRUE);
  base->tileLinks = list_create(TRUE);
  base->moveLinks = list_create(TRUE);
  base->pathLinks = list_create(TRUE);

  /* feature selection stuff... */
  base->f_wireframe = FALSE;
  base->f_textured = TRUE;
  base->f_lighting = TRUE;
  base->f_loadtextures = TRUE;
  base->f_backfacecull = FALSE;
  base->fselIDs = list_create(FALSE);

  base->drawTimer = NULL;
  base->msg_handler = tvBaseMessageHandlerDefault;
  base->update = NULL;

  base->post_draw_callback = NULL;
  base->post_draw_closure = NULL;

  /* set the OAA state for this viewer */
  base->oaa_id = GetNextOAAID();

  /* Create an OpenGL context for this one. */
  base->glmw = glmw;
  base->tvGLX = tvGLX;
  base->graphics = FALSE;
  base->update_viewport = TRUE;
  base->destroying = FALSE;

#ifdef HAVE_LIBGEO
	// DiscoverClientInitialize must be called before other calls to dotgeo 
	GeoDiscoverClientInitialize();

	GeoInitLogger( 1 );
	GeoSetLoggerOutput( "c:/TerraVision.log" );

	base->geoCellSpecHandle = NULL;
  base->geoDiscClientHandle = NULL;
	base->geoFetchMetadataHandle = NULL;
  base->geoSearchProfileHandle = NULL;

	base->geoCellNameCache = NULL;
	base->geoCellNameCacheEnabled = TRUE;

	base->geoGeoDataCache = NULL;
	base->geoGeoDataCacheEnabled = TRUE;

	base->geoSchemaURI = NULL;
  base->geoProfileURI = NULL;

  base->geoCompress = FALSE;
  base->geoQueryToTop = TRUE;
  base->geoMaxResults = 50;
#endif

  /* Check for a stereo visual */
  base->stereoVisual = glmContextStereo (glmWindowContext (glmw));

  glmRegisterRedraw (glmw, tvBaseDraw, base);
  glmRegisterMapNotify (glmw, tvBaseMapNotify, base);

  tvBaseCreateStars(base);

  InitSentence (base);

  PathCoreInit (base);

  /* Add this to global list. */
  if (! allWindows)
    allWindows = list_create (TRUE);
  list_push (allWindows, base);

  /* Initially the widget is not runnable. */
  tvBaseSetRunnable (base, FALSE);

  return base;
}

/*
 * Synopsis:
 *   void tvBaseStart (tvBase *base)
 *
 * Doc++ Description: *//**
 *	Spawns the rendering thread associated with the given tvBase 
 *      instance. Called by front-end when renderable.
 *
 * @param  base  handle to rendering state
 *
 * @return void
 *
 * @author  Lee Iverson
 *
 * @version  Nov 1, 1999
 *
 */
void
tvBaseStart (tvBase *threeD)
{
  static char * funcName = "tvBaseStart";

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  if (! threeD->graphics)
    tvBaseInitializeGraphics (threeD);

  /* Add all currently active geos */
  {
    list_iter_t i;
    for (i = list_first (geoList); ! list_done (i); i = list_next (i)) {
      TsGeoPyramid *geo = list_value (i);
      tvAddDataset (threeD, geo);
    }
  }

  /* PUT THE VIEWPOINT IN THE DATA SET */
  tvBaseInitView (threeD, tvBaseTop (threeD));

  /* Establish an OAA connection */
  OAAConnect (threeD);

#ifdef HAVE_LIBGEO
  /* Open the libgeo schema structure */
  PR_Lock (threeD->dotgeo_lock); 
  
	// init geoCellSpecHandle
	if ( threeD->geoCellSpecHandle == NULL && threeD->geoSchemaURI != NULL){
		threeD->geoCellSpecHandle = GeoCreateCellNameSpecification( threeD->geoSchemaURI );

    TSMTRACE ((DOTGEO_MASK|2, funcName, "CreateCellNameSpecification %p", threeD->geoCellSpecHandle));
    if ( threeD->geoCellSpecHandle == NULL ) {
			tsmError (funcName, "Unable to open .geo schema: %s", threeD->geoSchemaURI );
    }

		// tvBaseConsole( threeD, "Connected to GeoWeb - schema %s", threeD->geoSchemaURI );
  }
  
	// init geodata fetcher
	threeD->geoFetchMetadataHandle = GeoCreateFetchMetadataHandlerXML();

	// init search profile
  if ( threeD->geoSearchProfileHandle == NULL )
    threeD->geoSearchProfileHandle = GeoCreateSearchProfile( NULL );

	// init cellname cache
	threeD->geoCellNameCache = GeoCreateCellNameCache();

	// init geodata cache
	threeD->geoGeoDataCache = GeoCreateGeoDataCache();

	PR_Unlock (threeD->dotgeo_lock);

#endif /* HAVE_LIBGEO */

  tvBaseStartThreads (threeD);

  if (! threeD->cave)
    tvBasePostRedraw (threeD, FALSE);

  /* Start the DIS broadcasts if initialized */
  if (threeD->disSending) {
    threeD->disSending = FALSE;
    tvBaseDISEnable (threeD, TV_DIS_SEND, TRUE);
  }

  if (threeD->disMonitoring) {
    threeD->disMonitoring = FALSE;
    tvBaseDISEnable (threeD, TV_DIS_MONITOR, TRUE);
  }
}

/*
 * Synopsis:
 *   void tvBaseStop (tvBase *base)
 *
 * Doc++ Description: *//**
 *	Terminates the rendering thread associated with the given tvBase
 *      instance.
 *
 * @param base handle to rendering state
 *
 * @return void
 *
 * @author Lee Iverson
 *
 * @version Nov 1, 1999
 *
 */

void
tvBaseStop (tvBase *threeD)
{
  static char const funcName[] = "tvBaseStop";

  if (! threeD->started) return;

  /* Stop any flight path animation */
  PathStop(threeD);

  tvStopMovement (threeD);

  if (threeD->runnable) {
    TSMTRACE ((THREED_MASK|1, funcName, "(%p)", threeD));
    tvBaseSetRunnable (threeD, FALSE);

#ifdef VIS_THREAD
    /* Cancel visibility thread. */
    if (threeD->visible_thread) {
      PR_Interrupt (threeD->visible_thread);
      PR_JoinThread (threeD->visible_thread);
      threeD->visible_thread = 0;
    }
#endif

#ifdef HAVE_LIBGEO
    /* Cancel dotgeo thread. */
    if (threeD->dotgeo_thread) {
      PR_Interrupt (threeD->dotgeo_thread);
      PR_JoinThread (threeD->dotgeo_thread);
      threeD->dotgeo_thread = 0;
    }
#endif

    /* STOP GLMWINDOW UPDATES. */
    glmRemoveRedraw (threeD->glmw);
  }
}

/*
 * Synopsis:
 *   void tvBaseRemoveLinks(tvBase *base)
 *
 * Description:
 *	Remove all instances of a base from other base's lists
 *	when it is destroyed.
 *
 * Returns: void
 *
 * Author: Lee Iverson
 *
 * Date: Oct 27, 1999
 *
 */
static void
tvBaseRemoveLinks (tvBase *base)
{
  list_iter_t i;

  /* and let everybody know */
  for (i = list_first (allWindows); ! list_done (i); i = list_next (i)) {
    tvBase *from = list_value (i);
    list_remove(from->linkedBases, base);
    list_remove(from->frustumLinks, base);
    list_remove(from->tileLinks, base);
    list_remove(from->moveLinks, base);
    list_remove(from->pathLinks, base);
  }
}

/*
 * Synopsis: void tvBaseDestroy();
 *
 * Doc++ Description: *//**
 *     Deletes an instance of a tvBase structure, reclaiming its resources.
 *
 * @param base handle to rendering state
 *
 * @author
 *
 * @return  void
 *
 */
void
tvBaseDestroy(tvBase* base)
{
  static char const funcName[] = "tvBaseDestroy";
  PRInt32 destroying = PR_AtomicSet (&base->destroying, TRUE);

  if (destroying) {
    fprintf (stderr, "Warning: tvBaseDestroy(%p) called more than once.\n",
	     base);
    return;
  }

  TSMTRACE ((THREED_MASK|1, funcName, "(%p)", base));
  TSMTRINC ((THREED_MASK|1, 1));

  tvBaseStop (base);

  tvBaseLock (base); {
    PathReset (base);  /* unload the current flight path file */

    /* free all of the memory/display lists used to render features */
    FeatureFreeInstances (base);

    /* Remove this from global list. */
    list_remove (allWindows, base);

    if (base->glmw) {
      glmDestroyWindow (base->glmw);
      base->glmw = 0;
    }

    /* Now walk through the datasets, derefing tiles and severing
       connections. */
    while (! list_empty (base->geoList)) {
      GeoElement *elem = list_pop (base->geoList);
      tvGeoElementDestroy (elem);
    }

    list_destroy (base->geoList);

    /* free any other strings */
    if ( base->oaa_id ) tsmMemFree( base->oaa_id );
    base->oaa_id = NULL;

    /* destroy visible lists */
    if (base->graphics) {
      VisibleDrawDestroy (&base->drawList[0]);
      VisibleDrawDestroy (&base->drawList[1]);
      VisibleDrawDestroy (&base->drawList[2]);

      VisibleDestroy (&base->visibleList[0]);
      VisibleDestroy (&base->visibleList[1]);
    }

    eqhash_destroy (base->treehash0);
    eqhash_destroy (base->treehash1);

#ifdef VIS_THREAD
    PR_DestroyCondVar (base->vis_cond);
#endif
    istack_destroy (base->is0);
    istack_destroy (base->is1);

    PR_DestroyCondVar (base->run_cond);
    PR_DestroyCondVar (base->jumpReady);

    tvBaseRemoveLinks (base);

    /* destroy all our link view lists */
    list_destroy(base->linkedBases);
    list_destroy(base->frustumLinks);
    list_destroy(base->tileLinks);
    list_destroy(base->moveLinks);
    list_destroy(base->pathLinks);

  }; tvBaseUnlock (base);
  PR_DestroyLock (base->lock);

#ifdef HAVE_LIBGEO
  /* free the libgeo schema structures */
  PR_Lock (base->dotgeo_lock); {
		GeoDeleteCellNameCache( base->geoCellNameCache );
		GeoDeleteGeoDataCache( base->geoGeoDataCache );
		GeoDeleteCellNameSpecification( base->geoCellSpecHandle );
		GeoDeleteDiscoverClient( base->geoDiscClientHandle );
		GeoDeleteSearchProfile( base->geoSearchProfileHandle );

		// DiscoverClientTerminate must be called but as the last function to the dotgeo library
		GeoDiscoverClientTerminate();

  	if ( base->geoSchemaURI ) free( base->geoSchemaURI );
    if ( base->geoProfileURI ) free( base->geoProfileURI );

  }; PR_Unlock (base->dotgeo_lock);


  PR_DestroyLock (base->dotgeo_lock);
#endif /* HAVE_LIBGEO */

  free(base);

  TSMTRINC ((THREED_MASK|1, -1));
  TSMTRACE ((THREED_MASK|1, funcName, "(%p) Done", base));
}

/*
 * Synopsis: tvBase ** tvWindows(int *n)
 *
 * Doc++ Description: *//** 
 *    Allocates and initializes an array of all current windows.
 *    The array should be freed when done.
 *
 * Externals: allWindows
 *
 * @param n int reference containing the number of array entries on return
 *
 * @return  An array of tvBase* values or NULL.  The argument *n is
 *          initialized to the length of the array if a non-NULL value is
 *          returned.
 *
 * @author  Lee Iverson
 * 
 * @version Oct 1999
 *
 */
tvBase **
tvWindows (int *np)
{
  list_iter_t i;
  int n = 0, idx = 0;
  tvBase **wins;

  if (! allWindows) return NULL;

  for (i = list_first (allWindows); ! list_done (i); i = list_next (i))
    ++n;
  if (! n) return NULL;

  wins = malloc (n * sizeof (tvBase *));
  for (i = list_first (allWindows); ! list_done (i); i = list_next (i))
    wins[idx++] = (tvBase *) list_value (i);

  *np = n;
  return wins;
}

/*
 * Synopsis: void tvForallWindows (void (*f) (tvBase *, void *),
 *                                 void *closure)
 *
 * Doc++ Description: *//** 
 *     Iterates over all current tvBase* windows, calling the
 *     function (*f)(base,closure) for each.
 *
 * Externals: allWindows
 *
 * @param f        function to be called for each render thread.
 * @param closure  opaque data passed to f
 *
 * @return void.
 *
 * @author Lee Iverson
 * 
 * @version Oct 1999
 *
 */
void
tvForallWindows (void (*f) (tvBase *, void *), void *closure)
{
  list_iter_t i;

  if (! allWindows) return;

  for (i = list_first (allWindows); ! list_done (i); ) {
    tvBase *base = list_value (i);
    i = list_next (i);
    (*f) (base, closure);
  }
}

/*
 * Synopsis: tvBase * tvFindWindow(void (*f) (tvBase *, void *),
 *                                 void *closure)
 *
 * Description: Iterates through all current tvBase* windows and returns
 *              the first one for which the function (*f)(base,closure)
 *              returns non-zero.
 *
 * Externals: allWindows
 *
 * Returns: A window pointer or NULL if none found.
 *
 * Author: Lee Iverson
 * 
 * Date: Oct 1999
 *
 */
tvBase *
tvFindWindow (int (*f) (tvBase *, void *), void *closure)
{
  list_iter_t i;

  if (! allWindows) return NULL;

  for (i = list_first (allWindows); ! list_done (i); i = list_next (i)) {
    tvBase *base = list_value (i);
    if ((*f) (base, closure))
      return base;
  }
  return NULL;
}

/*
 * Synopsis: OpenGLFeatures(void)
 *
 * Description:  Determines which optional OpenGL features we can use.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: December 11, 1996
 */

static void
OpenGLFeatures (void)
{
  static char const *funcName = "OpenGLFeatures";
  static int checked = 0;

  if (! checked) {
    char const *glVersion = gltOGLVersion ();
    char const *glRenderer = gltOGLRenderer ();

    if (glVersion) {
      use_texture_objects = (gltOGLVersionNum () >= 1.1
			     || gltHasExtension ("texture_object"));
      TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Texture objects? %s",
		 use_texture_objects ? "YES" : "NO"));

      use_antialiasing = ! strncmp ("REV", glRenderer, 3);
      TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Antialiasing? %s",
		 use_antialiasing ? "YES" : "NO"));    

#ifdef GL_VERTEX_ARRAY
      use_vertex_array = TRUE;
#else
      use_vertex_array = gltHasExtension ("vertex_array");
#endif

      /* Tune texturing parameters wrt. graphics hardware */
      TextureTune ();

      ++checked;
    }
  }
}

#if HAVE_TEXSUBIMAGE
typedef struct TITableElem {
  tvGLXContext *tvGLX;
  int tile_w, tile_h;
  int ncomps;
  TextureImage *im;
} TITableElem;

typedef struct TITable {
  int n;
  TITableElem *elem;
  PRLock *lock;
} TITable;

static TITable tiTable;
static PRCallOnceType tiOnce;

static PRStatus
TextureImageOnce (void)
{
  tiTable.n = 8;
  tiTable.elem = calloc (8, sizeof (tiTable.elem[0]));
  tiTable.lock = PR_NewLock ();

  return PR_SUCCESS;
}

TextureImage *
GetTextureImage (tvGLXContext *tvGLX, TsGeoPyramid const *geo)
{
  TsGeoInfo const *info = TsGeoGetInfo (geo);
  int tile_w = info->tileNumPixels[0];
  int tile_h = info->tileNumPixels[1];
  int ncomps = info->tileNumComps;

  TITableElem *elem;
  int i;

  /* Set up global tile cache. */
  PR_CallOnce (&tiOnce, TextureImageOnce);

  PR_Lock (tiTable.lock); {
    for (i = 0; i < tiTable.n; ++i) {
      elem = &tiTable.elem[i];
      if (! elem->tvGLX) break;
      if (elem->tvGLX == tvGLX
	  && tile_w == elem->tile_w && tile_h == elem->tile_h
	  && ncomps == elem->ncomps) {
	PR_Unlock (tiTable.lock);
	return elem->im;
      }
    }
    if (i >= tiTable.n)
      abort ();

    /* Initialize this element. */
    elem->tvGLX = tvGLX;
    elem->tile_w = tile_w;
    elem->tile_h = tile_h;
    elem->ncomps = ncomps;
    elem->im = TextureImageCreate (tile_w, tile_h, ncomps);

  }; PR_Unlock (tiTable.lock);

  return elem->im;
}
#else /* !HAVE_TEXSUBIMAGE */

TextureImage *
GetTextureImage (tvGLXContext *tvGLX, TsGeoPyramid const *geo)
{
  return 0;
}
#endif

/*
 * Synopsis: void tvBaseInitializeGraphics()
 *	
 *
 * Description: Initializes the GL graphics for the window
 *
 * Doc++ Description: *//**
 *    Used to perform internal initializations of a render thread, which are 
 *    not safe/possible to perform until its (X) windows are visible. Should 
 *    be called once per render thread, early on (ISSUE: how early?); 
 *    typically as part of processing of an Xt Widget "realized" callback.
 *
 * @param base   handle to rendering state
 *
 * @return void
 *
 * @author Stephen Lau
 * 
 * @version 1993
 *
 * @see tvGLX_FindContext
 *
 */
static void
tvBaseInitializeGraphics(tvBase *base)
{
  static char * funcName = "tvBaseInitializeGraphics";
  tvGLXContext *tvGLX = base->tvGLX;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  glmMakeCurrentWindow (funcName, base->glmw); {

    /* Check for a viewport update. */
    if (base->update_viewport) {
      glViewport( 0, 0, base->width, base->height );
      base->update_viewport = FALSE;
    }

    /* CHECK FOR OPTIONAL OPENGL FEATURES. */
    OpenGLFeatures ();

    /* Initialize texture cache */
    {
      TextureCache *textureCache = tvGLX_GetTextureCache (tvGLX);
      if (! textureCache) {
	textureCache = TextureCacheCreate ();
	tvGLX_SetTextureCache (tvGLX, textureCache);
      }
      base->textureCache = textureCache;
      TSMTRACE ((THREED_MASK|2, funcName, "textureCache = %p", textureCache));

      /* Initialize visible tile buffers. */
      base->visibleList[0].id = 0;
      VisibleInit (&base->visibleList[0], textureCache);
      base->visibleList[1].id = 0;
      VisibleInit (&base->visibleList[1], textureCache);

      base->workingList0 = &base->visibleList[0];
      base->workingList1 = &base->visibleList[1];
      
      base->drawList[0].id = 0;
      VisibleDrawInit(&base->drawList[0]);
      base->drawList[1].id = 0;
      VisibleDrawInit(&base->drawList[1]);
      base->drawList[2].id = 0;
      VisibleDrawInit(&base->drawList[2]);
      base->currDrawList = 0;
      base->currWorkList = 1;
      base->nextDrawList = 0;
      base->nextWorkList = 2;
    }
	
    /* Initialize render primitive cache */
    {
      RenderPrimitiveCache *rp_cache = tvGLX_GetRenderPrimitiveCache (tvGLX);
      if (! rp_cache) {
	rp_cache = RenderPrimitiveCacheCreate ();
	tvGLX_SetRenderPrimitiveCache (tvGLX, rp_cache);
      }
      base->rp_cache = rp_cache;
      TSMTRACE ((THREED_MASK|2, funcName, "base->rp_cache = %p", rp_cache));
    }

    /* Set default fast rendering modes. */
    glDisable (GL_ALPHA_TEST);
    glDisable (GL_DITHER);
    glDisable (GL_LOGIC_OP);
    glDisable (GL_STENCIL_TEST);
  
    /* RGBsize(8); */
    glDepthMask (GL_TRUE);
    glEnable (GL_DEPTH_TEST);
    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

    if (base->stereoVisual) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
      glEnable (GL_BLEND);
    }

    if (use_antialiasing) {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
      glEnable (GL_BLEND);
      glEnable (GL_LINE_SMOOTH);
      glEnable (GL_POLYGON_SMOOTH);
    } else {
      glDisable (GL_BLEND);
    }

#ifdef HAVE_VERTEX_ARRAY
    tvBaseEnableVArray ();
#endif

#ifdef CAVE
    if(base->cave)
    {
      CAVEFar = 200000;
      CAVENear = 1.0;
    }
#endif /* CAVE */

    /* SET THE FAR Z PLANES */
    {
      GLfloat depth[2];
      glGetFloatv (GL_DEPTH_RANGE, depth);
      base->zfar = depth[1];
      /*glDepthRange(depth[0], depth[1]);*/
      TSMTRACE ((THREED_MASK|1, funcName, "Depth [%g, %g]", depth[0], depth[1]));
    }
  
    /*
     * INITIALIZE THE LIGHTING MODEL
     *
     */
    if (! glIsList (white_list)) {
      white_list = glGenLists (4);

      glNewList (white_list, GL_COMPILE); {
	gltMaterialDefv (GL_FRONT_AND_BACK, white_material);
      };  glEndList();
      glNewList (gray_list = white_list+1, GL_COMPILE); {
	gltMaterialDefv (GL_FRONT_AND_BACK, gray_material);
      };  glEndList();
      glNewList (ground_list = gray_list + 1, GL_COMPILE); {
	gltMaterialDefv (GL_FRONT_AND_BACK, ground_material);
      };  glEndList();
      glNewList (yellow_list = ground_list + 1, GL_COMPILE); {
	gltMaterialDefv (GL_FRONT_AND_BACK, yellow_material);
      };  glEndList();
    }

    if (! glIsList (base->sunLight_list))
      base->sunLight_list =  glGenLists (1);
    glNewList (base->sunLight_list, GL_COMPILE); {
      gltLightDefv (GL_LIGHT0, sunLight);
    }; glEndList();

    /* 
     * ADD SOME HAZE 
     *
     */
    base->fog_params[0] = 0;
    tvBaseSetFogColor (base, fogColorLight);
    tvBaseDrawFog (base);

    /*
     * INITIALIZE THE TEXTURE ENVIRONMENT
     *
     */
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glPolygonMode(GL_FRONT_AND_BACK, base->showWireframe ? GL_LINE : GL_FILL);

#if 0
    /* CREATE THE STANDARD TEXTURES */
    InitTextures();
#endif

#ifdef CAVE
    if (base->cave) {   
      /* INITIALIZE SOME CAVE STUFF */
      CAVENear = base->view.nearPlane;
      CAVEFar = base->view.farPlane;
    }
#endif /* CAVE */

    /* Clear screen */
    glColor3f (1.0, 1.0, 1.0);
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
    glClear( GL_COLOR_BUFFER_BIT );
    gltCheckErrors( FILE_LOC );
    
    base->graphics = TRUE;
  }; glmReleaseWindow (funcName, base->glmw);

  /* fake a window mapped event in order to get is in the right */
  /* state, particularly under Windows where we don't support   */
  /* the notion of a window being created but not mapped.       */

  tvBaseMapNotify( base->glmw, base );
}


static void
tvBaseSetRunnable (tvBase *threeD, int runnable)
{
  static char const funcName[] = "tvBaseSetRunnable";
  tvBaseLock (threeD); {
    TSMTRACE ((THREED_MASK|1, funcName, "%s", runnable ? "True" : "False"));
    threeD->runnable = runnable;
    if (runnable)
      PR_NotifyAllCondVar (threeD->run_cond);
  }; tvBaseUnlock (threeD);
}

typedef struct varDesc {
  tvVariable id;
  char const *name;
  enum varType {
    varBoolean,
    varInteger,
    varIntegerv,
    varFloat,
    varFloatv,
    varDouble,
    varDoublev,
    varString    
  } type;
} varDesc;

static char const *varTypeName[] = {
  "Boolean",
  "Integer",
  "Integerv",
  "Float",
  "Floatv",
  "Double",
  "Doublev", 
  "String"
};

static varDesc varDescs[] = {
  { TV_SHOW_LIGHTING, "SHOW_LIGHTING", varBoolean},
  { TV_SHOW_HEADS_UP, "SHOW_HEADS_UP", varBoolean},
  { TV_SHOW_WIREFRAME, "SHOW_WIREFRAME", varBoolean},
  { TV_SHOW_TEXTURES, "SHOW_TEXTURES", varBoolean},
  { TV_SHOW_MODELS, "SHOW_MODELS", varBoolean},
  { TV_SHOW_SKY, "SHOW_SKY", varBoolean},
  { TV_SHOW_GROUND, "SHOW_GROUND", varBoolean},
  { TV_SHOW_DATASETS, "SHOW_DATASETS", varBoolean},
  { TV_SHOW_FRUSTUMS, "SHOW_FRUSTUMS", varBoolean},
  { TV_SHOW_TILES, "SHOW_TILES", varBoolean},
  { TV_SHOW_PATHS, "SHOW_PATHS", varBoolean},
  { TV_SHOW_LOGO, "SHOW_LOGO", varBoolean},
  
  { TV_SHOW_CROSSHAIR, "SHOW_CROSSHAIR", varBoolean},
  { TV_CROSSHAIR, "CROSSHAIR", varIntegerv},
  
  { TV_FEATURES_WIREFRAME, "FEATURES_WIREFRAME", varBoolean},
  { TV_FEATURES_TEXTURED, "FEATURES_TEXTURED", varBoolean},
  { TV_FEATURES_LIGHTING, "FEATURES_LIGHTING", varBoolean},
  { TV_FEATURES_LOAD_TEXTURES, "FEATURES_LOAD_TEXTURES", varBoolean},
  { TV_FEATURES_BACKFACE_CULL, "FEATURES_BACKFACE_CULL", varBoolean},
  
  { TV_TIME_OF_DAY, "TIME_OF_DAY", varFloat},
  { TV_RESOLUTION_ADJUST, "RESOLUTION_ADJUST", varFloat},
  { TV_ELEVATION_SCALE, "ELEVATION_SCALE", varFloat},
  { TV_BACKFACE_CULL, "BACKFACE_CULL", varBoolean},
  { TV_HORIZON_CULL, "HORIZON_CULL", varBoolean},
  { TV_USE_ZBUFFER_HACK, "USE_ZBUFFER_HACK", varBoolean},
  { TV_TWOD_CONSTRAIN, "TWOD_CONSTRAIN", varBoolean},
  { TV_MIPMAP_TEXTURES, "MIPMAP_TEXTURES", varBoolean},
  { TV_FOG_DENSITY, "FOG_DENSITY", varFloat},
  { TV_HEIGHT_BASED_NAVIGATION, "HEIGHT_BASED_NAVIGATION", varBoolean},
  { TV_DOODLE_ON_TERRAIN, "DOODLE_ON_TERRAIN", varBoolean},
  
  { TV_FREEZE_TILES, "FREEZE_TILES", varBoolean},
  { TV_SHOW_TILE_DEBUG, "SHOW_TILE_DEBUG", varBoolean},
  
  { TV_STEREO, "STEREO", varBoolean},
  { TV_SCALE_IPD, "SCALE_IPD", varBoolean},
  { TV_ABSOLUTE_IPD, "ABSOLUTE_IPD", varFloat},

  { TV_VISIBLE_CYCLE, "VISIBLE_CYCLE", varIntegerv },

  { TV_DIS_SEND, "DIS_SEND", varBoolean},
  { TV_DIS_MONITOR, "DIS_MONITOR", varBoolean},
  { TV_DIS_ID, "DIS_ID", varIntegerv},
  { TV_DIS_SITE_ID, "DIS_SITE_ID", varInteger},
  { TV_DIS_FORCE_ID, "DIS_FORCE_ID", varInteger},
  { TV_DIS_ENTITY_ID, "DIS_ENTITY_ID", varInteger},

  { TV_SIZE, "SIZE", varIntegerv},

  { TV_VELOCITY, "VELOCITY", varFloat},
  { TV_ACCELERATION, "ACCELERATION", varFloat},

  { TV_IMAGE_QUALITY, "IMAGE_QUALITY", varFloat},

  { TV_SLIDE_DELTA, "SLIDE_DELTA", varFloatv},
  { TV_ATTITUDE_DELTA, "ATTITUDE_DELTA", varFloatv},

  { TV_SPLASH_MESSAGE, "SPLASH_MESSAGE", varString},

  { TV_MOTION_THRESHOLD, "MOTION_THRESHOLD", varFloat},

  { TV_TILE_BY_TILE, "TILE_BY_TILE", varBoolean},

  { TV_DOTGEO_PAUSE, "DOTGEO_PAUSE", varInteger},

  { TV_AUTO_POP_FRONT, "AUTO_POP_FRONT", varBoolean},

  { TV_SCHEMA_URL, "SCHEMA_URL", varString},

  { TV_HYPERLINK_URL, "HYPERLINK_URL", varString},
  { TV_HYPERLINK_DESC, "HYPERLINK_DESC", varString},

  { TV_MOVING, "MOVING", varBoolean},
  { TV_POPPING_STRATEGY, "POPPING_STRATEGY", varInteger},

  { TV_SHOW_HEADS_UP_DEBUG, "SHOW_HEADS_UP_DEBUG", varBoolean},

  { TV_SHOW_LATLONG, "SHOW_LATLONG", varBoolean},

  { TV_OAA_ID, "OAA_ID", varString},

  { TV_DOTGEO_COMPRESS, "DOTGEO_COMPRESS", varBoolean},
  { TV_DOTGEO_QUERY_TO_TOP, "DOTGEO_QUERY_TO_TOP", varBoolean},
  { TV_DOTGEO_MAX_RESULTS, "DOTGEO_MAX_RESULTS", varInteger},
  { TV_DOTGEO_CACHE_CELLS, "DOTGEO_CACHE_CELLS", varBoolean},
  { TV_DOTGEO_PROFILE_URL, "DOTGEO_PROFILE_URL", varString}

};

static void
checkVarDescs (void)
{
  static int checked = 0;
  if (! checked) {
    int i;
    for (i = 0; i < (int) TV_VARIABLE_END; ++i) {
      if (i != (int) varDescs[i].id) {
	fprintf (stderr, "varDescs[%d] %s out of order (%d)\n",
		 i, varDescs[i].name, (int) varDescs[i].id);
	abort ();
      }
    }
    ++checked;
  }
}

static inline PRBool
checkVarType (tvBase const *threeD, 
	      char const *set_get, enum varType desired,
	      tvVariable var)
{
  enum varType var_type = varDescs[var].type;
  if ( ! threeD ) return FALSE;
  if (var_type != desired) {
    tvBaseError (threeD, "tv%s%s(TV_%s) is %s", 
		     set_get, varTypeName[desired], 
		     varDescs[var].name, varTypeName[var_type]);
    return FALSE;
  }
  return TRUE;
}

/*
 * Doc++ Description:  *//**
 *    Get the value of a boolean state variable of the given tvBase 
 *    struct. Variables which maybe queried are:
 * <pre>
 * TV_SHOW_LIGHTING              show lighting effects
 * TV_SHOW_HEADS_UP              show heads-up display   
 * TV_SHOW_HEADS_UP_DEBUG        show heads-up display debugging info
 * TV_SHOW_WIREFRAME             show as wireframe models
 * TV_SHOW_TEXTURES              apply texture mapping 
 * TV_SHOW_MODELS                show VRML models 
 * TV_SHOW_SKY                   render sky 
 * TV_SHOW_GROUND                render ground not covered by datasets
 * TV_SHOW_DATASETS              render datasets
 * TV_SHOW_FRUSTUMS              show viewer's frustrum 
 * TV_SHOW_TILES                 show tile bounds
 * TV_SHOW_PATHS                 show path trajectories 
 * TV_SHOW_LOGO                  show SRI logo
 * TV_SHOW_CROSSHAIR             show crosshair for flight controls
 * TV_FEATURES_WIREFRAME         show VRML features as wireframe
 * TV_FEATURES_TEXTURED          apply textures to VRML features
 * TV_FEATURES_LIGHTING          apply lighting models to VRML features
 * TV_FEATURES_LOAD_TEXTURES     load textures assoc. w/VRML features
 * TV_FEATURES_BACKFACE_CULL     enable backface culling from VRML features
 * TV_BACKFACE_CULL              enable backface culling for geodata
 * TV_HORIZON_CULL               enable horizon culling (????)
 * TV_USE_ZBUFFER_HACK           enable z-buffer hack (????)
 * TV_TWOD_CONSTRAIN             constrain view to down-looking
 * TV_MIPMAP_TEXTURES            make use of MIPMAPS in texture ops
 * TV_HEIGHT_BASED_NAVIGATION    enable height-contrained navigation
 * TV_DOODLE_ON_TERRAIN          permit annotations on top of terrain
 * TV_FREEZE_TILES               freeze tiles (????)
 * TV_SHOW_TILE_DEBUG            show tile debugging information
 * TV_STEREO                     calculate stereo views
 * TV_SCALE_IPD                  ??????
 * TV_DIS_SEND                   ?????
 * TV_DIS_MONITOR                ?????
 * TV_MOVING                     is the viewpoint being moved or are we still?
 * TV_DOTGEO_COMPRESS            gzip server results from .geo?
 * TV_DOTGEO_QUERY_TO_TOP        query all cells above the targetted area
 * TV_DOTGEO_CACHE_CELLS         cache all cell query requests
 * </pre>
 *
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be retreived
 *
 * @return boolean result, value of "var"
 *
 * @see tvVariable
 *
 */
PRBool
tvGetBoolean (tvBase const *threeD, tvVariable var)
{
  if (! checkVarType (threeD, "Get", varBoolean, var))
    return FALSE;

  switch (var) {
  case TV_SHOW_LIGHTING:
    return threeD->showLighting;
  case TV_SHOW_HEADS_UP:
    return threeD->headsUp;
  case TV_SHOW_HEADS_UP_DEBUG:
    return threeD->headsUpDebug;
  case TV_SHOW_WIREFRAME:
    return threeD->showWireframe;
  case TV_SHOW_TEXTURES:
    return threeD->showTextures;
  case TV_SHOW_MODELS:
    return threeD->showModels;
  case TV_SHOW_SKY:
    return threeD->showSky;
  case TV_SHOW_GROUND:
    return threeD->showGround;
  case TV_SHOW_DATASETS:
    return threeD->showDatasets;
  case TV_SHOW_FRUSTUMS:
    return threeD->showFrustums;
  case TV_SHOW_TILES:
    return threeD->showTiles;
  case TV_SHOW_PATHS:
    return threeD->showPaths;
  case TV_SHOW_LOGO:
    return threeD->showLogo;
  case TV_SHOW_CROSSHAIR:
    return threeD->showCrossHair;
  case TV_SHOW_LATLONG:
    return threeD->showLatLong;

  case TV_FEATURES_WIREFRAME:
    return threeD->f_wireframe;
  case TV_FEATURES_TEXTURED:
    return threeD->f_textured;
  case TV_FEATURES_LIGHTING:
    return threeD->f_lighting;
  case TV_FEATURES_LOAD_TEXTURES:
    return threeD->f_loadtextures;
  case TV_FEATURES_BACKFACE_CULL:
    return threeD->f_backfacecull;

  case TV_BACKFACE_CULL:
    return threeD->backfaceCull;
  case TV_HORIZON_CULL:
    return threeD->horizonCull;
  case TV_USE_ZBUFFER_HACK:
    return threeD->useZBufferHack;
  case TV_FREEZE_TILES:
    return threeD->freezeTiles;
  case TV_TILE_BY_TILE:
    return threeD->tileByTile;
  case TV_SHOW_TILE_DEBUG:
    return threeD->showTileDebug;
  case TV_TWOD_CONSTRAIN:
    return threeD->twoDConstrain;
  case TV_MIPMAP_TEXTURES:
    return threeD->mipmapTextures;
  case TV_HEIGHT_BASED_NAVIGATION:
    return threeD->heightBasedNav;
  case TV_DOODLE_ON_TERRAIN:
    return threeD->projectToTerrain;

  case TV_STEREO:
    return threeD->stereo;
  case TV_SCALE_IPD:
    return threeD->scaleIPD;

  case TV_DIS_SEND:
    return threeD->disSending;
  case TV_DIS_MONITOR:
    return threeD->disMonitoring;

  case TV_AUTO_POP_FRONT:
    return threeD->auto_pop_to_top;

  case TV_MOVING:
    return ( threeD->moveTimer != NULL || PathIsPlaying( threeD ) );

#ifdef HAVE_LIBGEO
  case TV_DOTGEO_COMPRESS:
    return threeD->geoCompress;

  case TV_DOTGEO_QUERY_TO_TOP:
    return threeD->geoQueryToTop;

  case TV_DOTGEO_CACHE_CELLS:
    return threeD->geoCellNameCacheEnabled;

#else
  case TV_DOTGEO_COMPRESS:
  case TV_DOTGEO_QUERY_TO_TOP:
  case TV_DOTGEO_CACHE_CELLS:
    return 0;

#endif
      
  default:
    return 0;
  }
}

#define SET_WITH_PREV(var,val) (prev = (var), (var) = (val), prev != (val))

/*
 * Doc++ Description:  *//**
 *    Sets a boolean state variable in the given tvBase struct. Variables
 *    which maybe affected are:
 * <pre>
 * TV_SHOW_LIGHTING              show lighting effects
 * TV_SHOW_HEADS_UP              show heads-up display   
 * TV_SHOW_HEADS_UP_DEBUG        show heads-up display debug info
 * TV_SHOW_WIREFRAME             show as wireframe models
 * TV_SHOW_TEXTURES              apply texture mapping 
 * TV_SHOW_MODELS                show VRML models 
 * TV_SHOW_SKY                   render sky 
 * TV_SHOW_GROUND                render ground not covered by datasets
 * TV_SHOW_DATASETS              render datasets
 * TV_SHOW_FRUSTUMS              show viewer's frustrum 
 * TV_SHOW_TILES                 show tile bounds
 * TV_SHOW_PATHS                 show path trajectories 
 * TV_SHOW_LOGO                  show SRI logo
 * TV_SHOW_CROSSHAIR             show crosshair for flight controls
 * TV_FEATURES_WIREFRAME         show VRML features as wireframe
 * TV_FEATURES_TEXTURED          apply textures to VRML features
 * TV_FEATURES_LIGHTING          apply lighting models to VRML features
 * TV_FEATURES_LOAD_TEXTURES     load textures assoc. w/VRML features
 * TV_FEATURES_BACKFACE_CULL     enable backface culling from VRML features
 * TV_BACKFACE_CULL              enable backface culling for geodata
 * TV_HORIZON_CULL               enable horizon culling (????)
 * TV_USE_ZBUFFER_HACK           enable z-buffer hack (????)
 * TV_TWOD_CONSTRAIN             constrain view to down-looking
 * TV_MIPMAP_TEXTURES            make use of MIPMAPS in texture ops
 * TV_HEIGHT_BASED_NAVIGATION    enable height-contrained navigation
 * TV_DOODLE_ON_TERRAIN          permit annotations on top of terrain
 * TV_FREEZE_TILES               freeze tiles (????)
 * TV_SHOW_TILE_DEBUG            show tile debugging information
 * TV_STEREO                     calculate stereo views
 * TV_SCALE_IPD                  ??????
 * TV_DIS_SEND                   ?????
 * TV_DIS_MONITOR                ?????
 * TV_AUTO_POP_FRONT             automatically pop highest res sets to front
 * TV_DOTGEO_COMPRESS            gzip server results from .geo?
 * TV_DOTGEO_QUERY_TO_TOP        query all cells above the targetted area
 * TV_DOTGEO_CACHE_CELLS         cache all cell query requests
 * </pre>
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      value for boolean state variable
 *
 * @return previous value of "var", or FALSE upon failure.
 *
 * @see tvVariable
 *
 */
PRBool
tvSetBoolean (tvBase *threeD, tvVariable var, PRBool val)
{
  static char const funcName[] = "tvSetBoolean";
  PRBool prev;
  PRBool redraw = 0;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p, TV_%s, %d)",
	     threeD, varDescs[var].name, val));
  if (! checkVarType (threeD, "Set", varBoolean, var))
    return FALSE;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_SHOW_LIGHTING:
      redraw = SET_WITH_PREV(threeD->showLighting, val);
      break;
    case TV_SHOW_HEADS_UP:
      redraw = SET_WITH_PREV(threeD->headsUp, val);
      break;
    case TV_SHOW_HEADS_UP_DEBUG:
      redraw = SET_WITH_PREV(threeD->headsUpDebug, val);
      if (threeD->headsUpDebug && !threeD->headsUp)
	threeD->headsUp = TRUE;
      break;
    case TV_SHOW_WIREFRAME:
      redraw = SET_WITH_PREV(threeD->showWireframe, val);
      break;
    case TV_SHOW_TEXTURES:
      redraw = SET_WITH_PREV(threeD->showTextures, val);
      break;
    case TV_SHOW_SKY:
      redraw = SET_WITH_PREV(threeD->showSky, val);
      if (redraw) threeD->newBgList = TRUE;
      break;
    case TV_SHOW_GROUND:
      redraw = SET_WITH_PREV(threeD->showGround, val);
      if (redraw) threeD->newBgList = TRUE;
      break;
    case TV_SHOW_DATASETS:
      redraw = SET_WITH_PREV(threeD->showDatasets, val);
      break;
    case TV_SHOW_FRUSTUMS:
      redraw = SET_WITH_PREV(threeD->showFrustums, val);
      break;
    case TV_SHOW_TILES:
      redraw = SET_WITH_PREV(threeD->showTiles, val);
      break;
    case TV_SHOW_PATHS:
      redraw = SET_WITH_PREV(threeD->showPaths, val);
      break;
    case TV_SHOW_LOGO:
      redraw = SET_WITH_PREV(threeD->showLogo, val);
      break;
    case TV_SHOW_MODELS:
      redraw = SET_WITH_PREV(threeD->showModels, val);
      break;

    case TV_SHOW_CROSSHAIR:
      redraw = SET_WITH_PREV(threeD->showCrossHair, val);
      break;
    case TV_SHOW_LATLONG:
      redraw = SET_WITH_PREV(threeD->showLatLong, val);
      break;

    case TV_FEATURES_WIREFRAME:
      redraw = SET_WITH_PREV(threeD->f_wireframe, val);
      if (! threeD->showModels) redraw = 0;
      break;
    case TV_FEATURES_TEXTURED:
      redraw = SET_WITH_PREV(threeD->f_textured, val);
      if (! threeD->showModels) redraw = 0;
      break;
    case TV_FEATURES_LIGHTING:
      redraw = SET_WITH_PREV(threeD->f_lighting, val);
      if (! threeD->showModels) redraw = 0;
      break;
    case TV_FEATURES_LOAD_TEXTURES:
      redraw = SET_WITH_PREV(threeD->f_loadtextures, val);
      if (! threeD->showModels) redraw = 0;
      break;
    case TV_FEATURES_BACKFACE_CULL:
      redraw = SET_WITH_PREV(threeD->f_backfacecull, val);
      if (! threeD->showModels) redraw = 0;
      break;

    case TV_BACKFACE_CULL:
      redraw = SET_WITH_PREV(threeD->backfaceCull, val);
      tvBaseRedoVisibility (threeD, TRUE);
      break;
    case TV_HORIZON_CULL:
      redraw = SET_WITH_PREV(threeD->horizonCull, val);
      tvBaseRedoVisibility (threeD, TRUE);
      break;
    case TV_USE_ZBUFFER_HACK:
      redraw = SET_WITH_PREV(threeD->useZBufferHack, val);
      break;

    case TV_FREEZE_TILES:
      redraw = SET_WITH_PREV(threeD->freezeTiles, val);
      if (threeD->freezeTiles) {
	/* grab a copy of the current viewing parameters */
	threeD->freezeView = threeD->view;
#ifdef VIS_THREAD
      } else {
	TSMTRACE ((VIS_MASK|1, funcName, "freeze %d: notify vis_cond",
		   threeD->freezeTiles));
	PR_NotifyCondVar (threeD->vis_cond);
#endif
      }
      break;

    case TV_TILE_BY_TILE:
      redraw = SET_WITH_PREV(threeD->tileByTile, val);
      break;

    case TV_SHOW_TILE_DEBUG:
      redraw = SET_WITH_PREV(threeD->showTileDebug, val);
      if (! threeD->freezeTiles) redraw = FALSE;
      break;

    case TV_STEREO:
      redraw = SET_WITH_PREV(threeD->stereo, val);
      break;
    case TV_SCALE_IPD:
      redraw = SET_WITH_PREV(threeD->scaleIPD, val);
      if (! threeD->stereo) redraw = FALSE;
      break;

    case TV_TWOD_CONSTRAIN:
      redraw = SET_WITH_PREV(threeD->twoDConstrain, val);
      if (threeD->viewValid && threeD->twoDConstrain)
	tvBaseSetView_nolock (threeD, &threeD->view, TRUE);
      break;

    case TV_MIPMAP_TEXTURES:
      redraw = SET_WITH_PREV(threeD->mipmapTextures, val);
      if (threeD->textureCache)
	TextureCacheReset (threeD->textureCache);
      break;

    case TV_HEIGHT_BASED_NAVIGATION:
      SET_WITH_PREV(threeD->heightBasedNav, val);
      break;

    case TV_DOODLE_ON_TERRAIN:
      SET_WITH_PREV(threeD->projectToTerrain, val);
      break;

    case TV_DIS_SEND:
      prev = tvBaseDISEnable (threeD, TV_DIS_SEND, val);
      break;
    case TV_DIS_MONITOR:
      prev = tvBaseDISEnable (threeD, TV_DIS_MONITOR, val);
      break;

    case TV_AUTO_POP_FRONT:
      redraw = SET_WITH_PREV(threeD->auto_pop_to_top, val);
      break;

#ifdef HAVE_LIBGEO
    case TV_DOTGEO_COMPRESS:
      redraw = SET_WITH_PREV(threeD->geoCompress, val);
			if ( threeD->geoDiscClientHandle )
				GeoEnableStreamCompression( threeD->geoDiscClientHandle,
																		threeD->geoCompress );
      break;

    case TV_DOTGEO_QUERY_TO_TOP:
      redraw = SET_WITH_PREV(threeD->geoQueryToTop, val);
			if ( threeD->geoDiscClientHandle )
				GeoSetCellNameResolverMethod( threeD->geoDiscClientHandle,
																			( threeD->geoQueryToTop ) ?
																			TOP : DEFAULT );
      break;

    case TV_DOTGEO_CACHE_CELLS:
      redraw = SET_WITH_PREV(threeD->geoCellNameCacheEnabled, val);
      if ( threeD->geoDiscClientHandle ) {
				GeoSetCellNameCache( threeD->geoDiscClientHandle,
														 ( val ) ? threeD->geoCellNameCache : NULL );
			}
      break;
#endif
    default:
      prev = 0;
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return prev;
}

/*
 * Doc++ Description:  *//**
 *    Sets an integer state variable in the given tvBase struct.  Variables
 *    that may be affected are:
 * <pre>
 * TV_DIS_SITE_ID                ??purpose???
 * TV_DIS_FORCE_ID               ??purpose???
 * TV_DIS_ENTITY_ID              ??purpose???
 * TV_DOTGEO_PAUSE               ms delay between .geo requests
 * TV_POPPING_STRATEGY           An enum-value of type VisiblePoppingStrategy
 * TV_DOTGEO_MAX_RESULTS         The maximum number of hits returned by .geo
 * </pre>
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      value for integer state variable
 *
 * @return previous value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable, VisiblePoppingStrategy
 *
 */
int
tvSetInteger (tvBase *threeD, tvVariable var, int val)
{
  static char const funcName[] = "tvSetInteger";
  int prev = 0;
  PRBool redraw = FALSE;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p, TV_%s, %d)",
	     threeD, varDescs[var].name, val));

  if (! checkVarType (threeD, "Set", varInteger, var))
    return FALSE;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_DIS_SITE_ID:
      SET_WITH_PREV(threeD->disSiteID, val);
      break;
    case TV_DIS_FORCE_ID:
      SET_WITH_PREV(threeD->disForceID, val);
      break;
    case TV_DIS_ENTITY_ID:
      SET_WITH_PREV(threeD->disEntityID, val);
      break;
    case TV_DOTGEO_PAUSE:
#ifdef HAVE_LIBGEO
      SET_WITH_PREV(threeD->dotgeo_pause, val);
#endif
      break;
    case TV_DOTGEO_MAX_RESULTS:
#ifdef HAVE_LIBGEO
      SET_WITH_PREV(threeD->geoMaxResults, val);
      if ( threeD->geoDiscClientHandle )
        GeoSetFetchMaxRecords( threeD->geoDiscClientHandle, val );
#endif
      break;
    case TV_POPPING_STRATEGY:
      /* silently clamp this to a valid value... */
      if (val < VisiblePopping_None || val > VisiblePopping_LowFrag)
        val = VisiblePopping_None;
      SET_WITH_PREV(threeD->poppingStrategy, (VisiblePoppingStrategy)val);
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return prev;
}


/*
 * Doc++ Description:  *//**
 *    Gets the value of an integer state variable of the given tvBase 
 *    struct. Variables that may be queried are:
 * <pre>
 * TV_DIS_SITE_ID                ??purpose???
 * TV_DIS_FORCE_ID               ??purpose???
 * TV_DIS_ENTITY_ID              ??purpose???
 * TV_DOTGEO_PAUSE               ms delay between .geo requests
 * TV_POPPING_STRATEGY           An enum-value of type VisiblePoppingStrategy
 * TV_DOTGEO_MAX_RESULTS         The maximum number of hits returned by .geo
 * </pre>
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be retreived
 *
 * @return value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable, VisiblePoppingStrategy
 *
 */
int
tvGetInteger (tvBase const *threeD, tvVariable var)
{
  int prev = 0;

  if (! checkVarType (threeD, "Get", varInteger, var))
    return FALSE;

  switch (var) {
    case TV_DIS_SITE_ID:
      return threeD->disSiteID;
    case TV_DIS_FORCE_ID:
      return threeD->disForceID;
    case TV_DIS_ENTITY_ID:
      return threeD->disEntityID;
#ifdef HAVE_LIBGEO
    case TV_DOTGEO_PAUSE:
      return threeD->dotgeo_pause;
    case TV_DOTGEO_MAX_RESULTS:
      return threeD->geoMaxResults;
#endif
    case TV_POPPING_STRATEGY:
      return threeD->poppingStrategy;
  }

  return prev;
}


/*
 * Doc++ Description:  *//**
 *    Sets elements of an integer vector state variable in the given 
 *    tvBase struct. Variables that may be affected are:
 * <pre>
 * TV_CROSSHAIR           crosshair position (x, y)
 * TV_VISIBLE_CYCLE       purpose???? (2 ints)
 * TV_DIS_ID              purpose???? (siteid, forceid, entityid)
 * TV_SIZE                resize the render viewport (width, height)
 * </pre>
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      integer vector of (at least) same order as "var", containing
 *                 new values for the specified vector.
 *
 * @return number of array entities set.
 *
 * @see tvVariable
 *
 */
int
tvSetIntegerv (tvBase *threeD, tvVariable var, int *val)
{
  static char const funcName[] = "tvSetIntegerv";
  int ret;
  PRBool redraw = FALSE;

  if (! checkVarType (threeD, "Set", varIntegerv, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    default:
      ret = 0;
      break;
    case TV_CROSSHAIR: {
      int v0 = val[0], v1 = val[1];
      redraw = (threeD->showCrossHair 
		&& (v0 != threeD->crossHair[0] || v1 != threeD->crossHair[1]));
      threeD->crossHair[0] = v0;
      threeD->crossHair[1] = v1;
      ret = 2;
      break; }
    case TV_VISIBLE_CYCLE:
      tvBaseSetVisibleCycle (val[0], val[1]);
      ret = 2;
      break;
    case TV_DIS_ID:
      threeD->disSiteID = val[0];
      threeD->disForceID = val[1];
      threeD->disEntityID = val[2];
      ret = 3;
      break;
    case TV_SIZE:
      tvBaseSetSize( threeD, val[0], val[1] );      
      ret = 2;
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return ret;
}


/*
 * Doc++ Description:  *//**
 *    Gets the values of an integer vector state variable of the given 
 *    tvBase struct. Variables that may be queried are:
 * <pre>
 * TV_CROSSHAIR           crosshair position (x, y)
 * TV_VISIBLE_CYCLE       purpose???? (2 ints)
 * TV_DIS_ID              purpose???? (siteid, forceid, entityid)
 * TV_SIZE                current render viewport size (width, height)
 * </pre>
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      integer vector of (at least) same order as "var", to receive
 *                 the returned results.
 *
 * @return    number of array entities filled in, along with entries in "val" 
 *            filled in.
 *
 * @see tvVariable
 *
 */
int
tvGetIntegerv (tvBase const *threeD, tvVariable var, int *val)
{
  if (! checkVarType (threeD, "Get", varIntegerv, var))
    return 0;

  switch (var) {
  case TV_CROSSHAIR:
    val[0] = threeD->crossHair[0];
    val[1] = threeD->crossHair[1];
    return 2;
  case TV_VISIBLE_CYCLE:
    tvBaseVisibleCycle (&val[0], &val[1]);
    return 2;
  case TV_DIS_ID:
    val[0] = threeD->disSiteID;
    val[1] = threeD->disForceID;
    val[2] = threeD->disEntityID;
    return 3;
  case TV_SIZE:
    val[0] = threeD->width;
    val[1] = threeD->height;
    return 2;
  default:
    return 0;
  }
}


/*
 * Doc++ Description:  *//**
 *    Sets a float state variable in the given tvBase struct. Variables
 *    that may be affected are:
 * <pre>
 * TV_TIME_OF_DAY                set time of day
 * TV_RESOLUTION_ADJUST          resolution adjust (?????)
 * TV_ELEVATION_SCALE            scale factor for elevation
 * TV_ABSOLUTE_IPD               purpose??????
 * TV_VELOCITY                   set the viewer's velocity (m/s)
 * TV_ACCELERATION               set the viewer's acceleration
 * TV_FOG_DENSITY                set the fog parameters in the view
 * TV_MOTION_THRESHOLD           threshold to drop to wireframe (Hz), 0 = none
 * </pre> 
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      value for float state variable
 *
 * @return previous value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable
 *
 */
float
tvSetFloat (tvBase *threeD, tvVariable var, float val)
{
  static char const funcName[] = "tvSetFloat";
  float prev;
  PRBool redraw = FALSE;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p, TV_%s, %g)",
	     threeD, varDescs[var].name, val));

  if (! checkVarType (threeD, "Get", varFloat, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_TIME_OF_DAY:
      redraw = SET_WITH_PREV(threeD->timeOfDay, val);
      tvBaseCalculateLighting(threeD);
      break;
    case TV_RESOLUTION_ADJUST:
      redraw = SET_WITH_PREV(threeD->resAdjust, val);
      tvBaseRedoVisibility (threeD, TRUE);
      break;
    case TV_ABSOLUTE_IPD:
      redraw = SET_WITH_PREV(threeD->absoluteIPD, val);
      if (! threeD->stereo) redraw = FALSE;
      break;
    case TV_ELEVATION_SCALE:
      redraw = SET_WITH_PREV(threeD->elevScaleFactor, val);
      break;
    case TV_VELOCITY:
      redraw = SET_WITH_PREV(threeD->view.vel, val);
      break;
    case TV_ACCELERATION:
      redraw = SET_WITH_PREV(threeD->view.accel, val);
      break;
    case TV_FOG_DENSITY:
      redraw = SET_WITH_PREV(threeD->fog_params[0], val);
      break;
    case TV_MOTION_THRESHOLD:
      redraw = SET_WITH_PREV(threeD->motionThreshold, val);
      break;
    default:
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return prev;
}


/*
 * Doc++ Description:  *//**
 *    Gets the value of a float state variable of the given tvBase struct.
 * <pre>
 * TV_TIME_OF_DAY                set time of day
 * TV_RESOLUTION_ADJUST          resolution adjust (?????)
 * TV_ELEVATION_SCALE            scale factor for elevation
 * TV_ABSOLUTE_IPD               purpose??????
 * TV_VELOCITY                   get the viewer's current velocity (m/s)
 * TV_ACCELERATION               get the viewer's current acceleration
 * TV_IMAGE_QUALITY              get the current image's quality (1.0 = best)
 * TV_FOG_DENSITY                get the fog density setting
 * TV_MOTION_THRESHOLD           threshold to drop to wireframe (Hz), 0 = none
 * </pre> 
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 *
 * @return current value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable
 *
 */
float
tvGetFloat (tvBase const *threeD, tvVariable var)
{
  if (! checkVarType (threeD, "Get", varFloat, var))
    return 0;

  switch (var) {
  case TV_TIME_OF_DAY:
    return threeD->timeOfDay;
  case TV_RESOLUTION_ADJUST:
    return threeD->resAdjust;
  case TV_ABSOLUTE_IPD:
    return threeD->absoluteIPD;
  case TV_ELEVATION_SCALE:
    return threeD->elevScaleFactor;
  case TV_VELOCITY:
    return threeD->view.vel;
  case TV_ACCELERATION:
    return threeD->view.accel;
  case TV_IMAGE_QUALITY:
    return threeD->image_quality;
  case TV_FOG_DENSITY:
      return threeD->fog_params[0];
  case TV_MOTION_THRESHOLD:
    return threeD->motionThreshold;
  default:
    return 0;
  }
}


/*
 * Doc++ Description:  *//**
 *    Sets the elements of a float vector state variable of the given 
 *    tvBase struct.  </i>There are presently no state variables of type
 *    float-array that may be affected</i>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      float vector of (at least) same order as "var", with 
 *                 elements initialized to the new values.
 *
 * @return    number of array entities filled in.
 *
 * @see tvVariable
 *
 */
int
tvSetFloatv (tvBase *threeD, tvVariable var, float * val)
{
  static char const funcName[] = "tvSetFloatv";
  int ret;
  PRBool redraw = FALSE;

  if (! checkVarType (threeD, "Set", varFloatv, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_SLIDE_DELTA:
      tvBaseSetSlideDelta (threeD, val[0], val[1]);
      ret = 2;
      break;

    case TV_ATTITUDE_DELTA:
      tvBaseSetAttitudeDelta (threeD, val[0], val[1]);
      ret = 2;
      break;

    default:
      ret = 0;
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);
  return ret;
}

/*
 * Doc++ Description:  *//**
 *    Gets the values of a float vector state variable of the given 
 *    tvBase struct. </b>There are presently no state variables of type
 *    float-array that may be queried</b>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      float vector of (at least) same order as "var", to receive
 *                 the returned results.
 *
 * @return    number of array entities filled in, along with entries in "val" 
 *            filled in.
 *
 * @see tvVariable
 *
 */
int
tvGetFloatv (tvBase const *threeD, tvVariable var, float * val)
{
  int ret;

  if (! checkVarType (threeD, "Get", varFloatv, var))
    return 0;

  switch (var) {
  default:
    ret = 0;
    break;
  }
  return ret;
}

/*
 * Doc++ Description:  *//**
 *    Sets a double state variable of the given tvBase struct. </b>There 
 *    are presently no state variables of type double that may be affected</b>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      value for double state variable
 *
 * @return previous value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable
 *
 */
double
tvSetDouble (tvBase *threeD, tvVariable var, double val)
{
  static char const funcName[] = "tvSetDouble";
  double prev = 0;
  PRBool redraw = FALSE;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p, TV_%s, %g)",
	     threeD, varDescs[var].name, val));

  if (! checkVarType (threeD, "Set", varDouble, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    }
    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return prev;
}

/*
 * Doc++ Description:  *//**
 *    Gets the value of a double state variable of the given tvBase 
 *    struct. </b>There are presently no state variables of type double that 
 *    may be queried</b>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 *
 * @return current value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable
 *
 */
double
tvGetDouble (tvBase const *threeD, tvVariable var)
{
  if (! checkVarType (threeD, "Get", varDouble, var))
    return 0;

  switch (var) {
  default:
    return 0;
  }
}

/*
 * Doc++ Description:  *//**
 *    Sets the elements of a double vector state variable of the given 
 *    tvBase struct. </b>There are presently no state variables of type 
 *    double-array that may be affected</b>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      double vector of (at least) same order as "var", with 
 *                 elements initialized to the new values.
 *
 * @return    number of array entities filled in.
 *
 * @see tvVariable
 *
 */
int
tvSetDoublev (tvBase *threeD, tvVariable var, double * val)
{
  static char const funcName[] = "tvSetDoublev";
  int ret;
  PRBool redraw = FALSE;

  if (! checkVarType (threeD, "Set", varDoublev, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    default:
      ret = 0;
      break;
    }

    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);
  return ret;
}

/*
 * Doc++ Description:  *//**
 *    Gets the values of a double vector state variable of the given 
 *    tvBase struct. </i>There are presently no state variables of type 
 *    double-array that may be queried</i>.
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      double vector of (at least) same order as "var", to receive
 *                 the returned results.
 *
 * @return    number of array entities filled in, along with entries in "val" 
 *            filled in.
 *
 * @see tvVariable
 *
 */
int
tvGetDoublev (tvBase const *threeD, tvVariable var, double * val)
{
  int ret;

  if (! checkVarType (threeD, "Get", varDoublev, var))
    return 0;

  switch (var) {
  default:
    ret = 0;
    break;
  }
  return ret;
}

/*
 * Doc++ Description:  *//**
 *    Sets a string state variable of the given tvBase struct.
 *
 * <pre>
 * TV_SPLASH_MESSAGE             message displayed on splash
 * TV_SCHEMA_URL                 set and load the .geo schema file
 * TV_DOTGEO_PROFILE             set the .geo profile URI
 * TV_HYPERLINK_URL              set/clear the hyperlink info for curr link
 * TV_HYPERLINK_DESC             set/clear the hyperlink description
 * TV_OAA_ID                     set the OAA ID string for this viewer
 * </pre> 
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be set
 * @param val      value for char* state variable
 *
 * @return previous value of "var", or FALSE (0) upon failure.
 *
 * @see tvVariable
 *
 */
char const *
tvSetString (tvBase *threeD, tvVariable var, char const * val)
{
  static char const funcName[] = "tvSetString";
  char const * prev = 0;
  PRBool redraw = FALSE, prevNull;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p, TV_%s, \"%s\")",
	     threeD, varDescs[var].name, val));

  if (! checkVarType (threeD, "Set", varString, var))
    return 0;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_SPLASH_MESSAGE:
      if ( threeD->splash_message )
	tsmMemFree( threeD->splash_message );
      threeD->splash_message = ( val ) ? tsmMemStrdup( val ) : NULL;
      redraw = TRUE;
      break;

    case TV_DOTGEO_PROFILE_URL:
#ifdef HAVE_LIBGEO
      if ( threeD->geoProfileURI )
				tsmMemFree( threeD->geoProfileURI );
      threeD->geoProfileURI = ( val ) ? tsmMemStrdup( val ) : NULL;

      if ( threeD->geoSearchProfileHandle )
        GeoDeleteSearchProfile( threeD->geoSearchProfileHandle );

   	  threeD->geoSearchProfileHandle = GeoCreateSearchProfile( threeD->geoProfileURI );

      redraw = TRUE;
#endif /* HAVE_LIBGEO */
      break;

    case TV_SCHEMA_URL:

#ifdef HAVE_LIBGEO
      prevNull = ( threeD->geoSchemaURI == NULL );
      if ( threeD->geoSchemaURI )
				tsmMemFree( threeD->geoSchemaURI );

      threeD->geoSchemaURI = ( val ) ? tsmMemStrdup( val ) : NULL;
      redraw = TRUE;
			
      /* (re)load or unload the new schema URL */

			if ( val && strlen( val ) > 0 )
				tvBaseConsole( threeD, "Connected to GeoWeb - schema %s", val );
			else if ( ! prevNull )
				tvBaseConsole( threeD, "Disconnected from GeoWeb" );
			
      PR_Lock (threeD->dotgeo_lock); {
				if ( threeD->geoDiscClientHandle != NULL ) {
					GeoDeleteCellNameSpecification( threeD->geoCellSpecHandle );
					GeoDeleteDiscoverClient( threeD->geoDiscClientHandle );
					threeD->geoDiscClientHandle = NULL;
					threeD->geoCellSpecHandle = NULL;
				} 
				if ( threeD->geoSchemaURI && strcmp ("", threeD->geoSchemaURI) ) {
					threeD->geoCellSpecHandle =
						GeoCreateCellNameSpecification( threeD->geoSchemaURI );
					threeD->geoDiscClientHandle =
						GeoCreateDiscoverClient( threeD->geoCellSpecHandle );
					
					if ( threeD->geoDiscClientHandle == NULL ) {
						if ( threeD->splash_message )
							tsmMemFree( threeD->splash_message );

						threeD->splash_message =
							tsmMemStrdup( "Unable to open .geo schema" );
						tvBaseConsole( threeD, "Unable to load GeoWeb schema - aborted" );
					}

					GeoSetGeoDataCache( threeD->geoFetchMetadataHandle,
															threeD->geoGeoDataCache );
					GeoEnableStreamCompression( threeD->geoDiscClientHandle,
																			threeD->geoCompress );
          GeoSetFetchMaxRecords( threeD->geoDiscClientHandle,
																 threeD->geoMaxResults );
					GeoSetSearchProfile( threeD->geoDiscClientHandle,
															 threeD->geoSearchProfileHandle );
					GeoSetCellNameResolverMethod( threeD->geoDiscClientHandle,
																				( threeD->geoQueryToTop ) ?
																				TOP : DEFAULT );
					GeoSetCellNameCache( threeD->geoDiscClientHandle,
															 ( threeD->geoCellNameCacheEnabled ) ?
															 threeD->geoCellNameCache : NULL );
				}
      }; 
			PR_Unlock (threeD->dotgeo_lock);
#endif /* HAVE_LIBGEO */
      break;

    case TV_HYPERLINK_URL:
      if ( threeD->f_pick_url )
	tsmMemFree( threeD->f_pick_url );
      threeD->f_pick_url = ( val ) ? tsmMemStrdup( val ) : NULL;

      /* if URL now is NULL, then clear the hyperlink state also */

      if ( val == NULL ) {
	if ( threeD->f_pick_desc )
	  tsmMemFree( threeD->f_pick_desc );
	threeD->f_pick_id     = -1;
	threeD->f_pick_anchor = -1;
	threeD->f_pick_desc   = NULL;
      }
      break;

    case TV_HYPERLINK_DESC:
      if ( threeD->f_pick_desc )
	tsmMemFree( threeD->f_pick_desc );
      threeD->f_pick_desc = ( val ) ? tsmMemStrdup( val ) : NULL;
      break;

    case TV_OAA_ID:
      if ( threeD->oaa_id )
	tsmMemFree( threeD->oaa_id );
      threeD->oaa_id = ( val ) ? tsmMemStrdup( val ) : NULL;
      break;

    }
    if (redraw) tvBasePostRedraw (threeD, TRUE);
  }; tvBaseUnlock (threeD);

  return prev;
}

/*
 * Doc++ Description:  *//**
 *    Gets the value of a string state variable of the given tvBase struct.
 *  The returned string is a copy and should be freed by the caller if
 *  non-NULL.
 *
 * <pre>
 * TV_SPLASH_MESSAGE             message displayed on splash
 * TV_SCHEMA_URL                 return the .geo schema file URL
 * TV_DOTGEO_PROFILE_URL         return the .geo profile URL
 * TV_HYPERLINK_URL              url of hyperlink under cursor
 * TV_HYPERLINK_DESC             description of hyperlink
 * TV_OAA_ID                     return the OAA ID string for this viewer
 * </pre> 
 *
 * @param threeD   handle to rendering state
 * @param var      state variable to be returned
 *
 * @return current value of "var", or NULL (0) upon failure.
 *
 * @see tvVariable
 *
 */
char const *
tvGetString (tvBase const *threeD, tvVariable var)
{
  static const char *funcName = "tvGetString";
  char *result = NULL;

  if (! checkVarType (threeD, "Get", varString, var))
    return NULL;

  tvBaseLock (threeD); {
    switch (var) {
    case TV_SPLASH_MESSAGE:
      result = ( threeD->splash_message ) ? 
	tsmMemStrdup( threeD->splash_message ) : NULL;
      break;
    
    case TV_DOTGEO_PROFILE_URL:
#ifdef HAVE_LIBGEO
      result = ( threeD->geoProfileURI ) ? 
				tsmMemStrdup( threeD->geoProfileURI ) : NULL;
#else
      result = NULL;
#endif 
      break;

    case TV_SCHEMA_URL:
#ifdef HAVE_LIBGEO
      result = ( threeD->geoSchemaURI ) ? 
	tsmMemStrdup( threeD->geoSchemaURI ) : NULL;
#else
      result = NULL;
#endif 
      break;

    case TV_HYPERLINK_URL:
      result = ( threeD->f_pick_url ) ? 
	tsmMemStrdup( threeD->f_pick_url ) : NULL;
      break;

    case TV_HYPERLINK_DESC:
      result = ( threeD->f_pick_desc ) ? 
	tsmMemStrdup( threeD->f_pick_desc ) : NULL;
      break;

    case TV_OAA_ID:
      result = ( threeD->oaa_id ) ? 
	tsmMemStrdup( threeD->oaa_id ) : NULL;
      break;

    default:
      result = NULL;
    }
  }; tvBaseUnlock (threeD);

  return result;
}

/*
 * Doc++ Description: *//**
 *   Returns the arbitrary user-assigned name registered with the rendering
 *   state at the time of its creation.
 *
 * ISSUE:  Presently not called from anywhere
 *
 * @param  threeD  handle to rendering state
 *
 * @return the read-only character string copy of what was registered at 
 *         thread-state creation.
 *
 * @see    tvBaseCreate
 *
 */
char const *
tvName (tvBase const *threeD)
{
  return threeD->name;
}


/*
 * Doc++ Description: *//**
 *    Returns a handle to the established path associated with a render thread.
 *
 * @param  threeD  handle to rendering state
 *
 * @return handle to current path for thread; NULL if none exists.
 *
 *
 */
Path *
tvGetPath (tvBase const *threeD)
{
  return threeD->path;
}

static int
tvLink (list_t *list, tvBase *from, tvBase *to, int add)
{
  list_iter_t find = list_find (list, to);
  int ret;

  if (add) {
    if ((ret = ! list_iter_valid (find)))
      list_push (list, to);
  } else {
    if ((ret = list_iter_valid (find)))
      list_remove (list, to);
  }
  return ret;
}

/*
 * Doc++ Description: *//**
 *    Establishes a link between one rendering instance and another. Thus 
 *    changes in the view state of one view can be applied to another view.
 *    (ISSUE: need a better description here).
 *
 * @param from   handle to rendering state of source view (driving view)
 * @param sel    type of link  
 * @param to     handle to rendering state of target view (client view)
 *
 * @return   boolean result:  1 on success,  0 on failure
 *
 * @see      tvLinkType
 *
 */
int
tvAddLink (tvBase *from, tvLinkType sel, tvBase *to)
{
  static char const funcName[] = "tvAddLink";
  list_t *links;

  switch (sel) {
  default:
    return FALSE;
  case TV_FRUSTUM:
    tvLink (to->linkedBases, to, from, TRUE);
    links = from->frustumLinks;
    break;
  case TV_TILES:
    tvLink (to->linkedBases, to, from, TRUE);
    links = from->tileLinks;
    break;
  case TV_MOVEMENT:
    links = from->moveLinks;
    break;
  case TV_PATHS:
    tvLink (to->linkedBases, to, from, TRUE);
    links = from->pathLinks;
    break;
  }

  {
    int ret = tvLink (links, from, to, TRUE);
    TSMTRACE ((THREED_MASK|1, funcName, "Add link %s -> %s%s",
	       from->name, to->name, ret ? "" : ": FAIL"));
    tvBasePostRedraw (to, FALSE);    
    return ret;
  }
}


/*
 * Doc++ Description: *//**
 *    Breaks a link previously established between two rendering instances.
 *
 * @param from   handle to rendering state of source view (driving view)
 * @param sel    type of link  
 * @param to     handle to rendering state of target view (client view)
 *
 * @return   boolean result:  1 on success,  0 on failure
 *
 * @see      tvLinkType
 *
 */
int
tvRemoveLink (tvBase *from, tvLinkType sel, tvBase *to)
{
  static char const funcName[] = "tvRemoveLink";
  list_t *links;

  switch (sel) {
  default:
    return FALSE;
  case TV_FRUSTUM:
    tvLink (to->linkedBases, to, from, FALSE);
    links = from->frustumLinks;
    break;
  case TV_TILES:
    tvLink (to->linkedBases, to, from, FALSE);
    links = from->tileLinks;
    break;
  case TV_MOVEMENT:
    links = from->moveLinks;
    break;
  case TV_PATHS:
    tvLink (to->linkedBases, to, from, FALSE);
    links = from->pathLinks;
    break;
  }

  {
    int ret = tvLink (links, from, to, FALSE);
    TSMTRACE ((THREED_MASK|1, funcName, "Remove link %s -> %s%s",
	       from->name, to->name, ret ? "" : ": FAIL"));
    tvBasePostRedraw (to, FALSE);    
    return ret;
  }
}

/*
 * Doc++ Description: *//**
 *   Returns a list of all links of a given type associated with a rendering 
 *   instance.
 *    
 * @param from  handle to rendering state
 * @param sel   type of links to query
 *
 * @return  (possibly empty) list of handles to linked rendering instances.
 *
 * @see  tvLinkType,  list_t
 *
 */
list_t *
tvGetLinks (tvBase *from, tvLinkType sel)
{
  /*static char const funcName[] = "tvGetLinks";*/

  switch (sel) {
  default:
    return NULL;
  case TV_FRUSTUM:
    return from->frustumLinks;
  case TV_TILES:
    return from->tileLinks;
  case TV_MOVEMENT:
    return from->moveLinks;
  case TV_PATHS:
    return from->pathLinks;
  }

}

/*
 * Doc++ Description: *//**
 *   Registers a callback for handling render thread requests to install 
 *   a timer.
 *
 * @param threeD  handle to rendering state
 * @param f       timer-add handler callback
 * @param closure opaque data passed to f
 *
 * @return void
 *
 * @see    tvAddTimerHandler
 *
 */
void
tvBaseAddTimer (tvBase *threeD, void **idp, unsigned int msecs,
		void (*f) (void *closure),
		void *closure)
{
  glmAddTimer (threeD->glmw, idp, msecs, f, closure);
}

/*
 * Doc++ Description: *//**
 *   Registers a callback for handling render thread requests to delete 
 *   (cancel?) a timer.
 *
 * @param threeD  handle to rendering state
 * @param f       timer-delete handler callback
 * @param closure opaque data passed to f
 *
 * @return void
 *
 * @see    glmRemoveTimer
 *
 */
void 
tvBaseDelTimer (tvBase *threeD, void **idp)
{
  glmRemoveTimer (threeD->glmw, idp);
}

/*
 * Doc++ Description: *//**
 *   Registers a callback for client-side capture of messages generated by 
 *   the rendering thread.
 *
 * @param threeD   handle to rendering state
 * @param f        callback function
 * @param closure  opaque data passed to f
 *
 * @return void
 *
 * @see    tvMessageHandler
 *
 */ 
void
tvSetMessageHandler (tvBase *threeD,
		     tvMessageHandler f,
		     void *closure)
{
  threeD->msg_handler = f;
  threeD->msg_closure = closure;
}

static void
tvBaseMessageHandlerDefault (void *closure, tvMessageType t,
				 char const *fmt, va_list ap)
{
  static const char *id[] = {"ERROR", "Status", "Busy"};
  if ( fmt ) {
    fprintf (stderr, "%s: ", id[t]);
    vfprintf (stderr, fmt, ap);
  } else
    fprintf (stderr, "%s:\n", id[t]);
}

void
tvBaseMessage (tvBase const *threeD, tvMessageType msg, char const *fmt, ...)
{
  if (threeD->msg_handler) {
    va_list ap;
    va_start (ap, fmt);
    (*threeD->msg_handler) (threeD->msg_closure, msg, fmt, ap);
    va_end (ap);
  }
}

void
tvBaseError (tvBase const *threeD, char const *fmt, ...)
{
  if (threeD->msg_handler) {
    va_list ap;
    va_start (ap, fmt);
    (*threeD->msg_handler) (threeD->msg_closure, TV_ERROR_MSG, fmt, ap);
    va_end (ap);
  }
}

void
tvBaseConsole (tvBase const *threeD, char const *fmt, ...)
{
  if (threeD->msg_handler) {
    va_list ap;
    va_start (ap, fmt);
    (*threeD->msg_handler) (threeD->msg_closure, TV_CONSOLE_MSG, fmt, ap);
    va_end (ap);
  }
}

void
tvBaseStatus (tvBase const *threeD, char const *fmt, ...)
{
  if (threeD->msg_handler) {
    va_list ap;
    va_start (ap, fmt);
    (*threeD->msg_handler) (threeD->msg_closure, TV_STATUS_MSG, fmt, ap);
    va_end (ap);
  }
}

void
tvBaseBusy (tvBase const *threeD, char const *fmt, ...)
{
  if (threeD->msg_handler) {
    va_list ap;
    va_start (ap, fmt);
    (*threeD->msg_handler) (threeD->msg_closure, TV_BUSY_MSG, fmt, ap);
    va_end (ap);
  }
}

/*
 * Doc++ Description: *//**
 *   Registers a callback for client-side interception of update events, 
 *   generated by the render thread.
 *
 * @param threeD   handle to rendering state
 * @param f        callback function to receive update events
 * @param closure  opaque data passed to f
 *
 * @return void
 *
 * @see    tvLinkType
 *
 */
void
tvSetUpdateHandler (tvBase *threeD,
		    void (*f) (void *, tvLinkType, tvBase *),
		    void *closure)
{
  threeD->update = f;
  threeD->update_closure = closure;
}

void
tvBaseUpdate (tvBase *threeD, tvLinkType link)
{
  if (threeD->update) {
    (*threeD->update) (threeD->update_closure, link, threeD);
  }
}

void
tvBaseMapNotify (glmWindow glmw, void *closure)
{
  static const char funcName[] = "tvBaseMapNotify";
  tvBase *threeD = closure;
  int mapped = glmIsMapped (glmw);
  tvBaseLock (threeD); {
    threeD->mapped = mapped;
    TSMTRACE ((THREED_MASK|1, funcName, "%s", mapped ? "True" : "False"));
    if (mapped)
      PR_NotifyAllCondVar (threeD->run_cond);
  }; tvBaseUnlock (threeD);
}

PRStatus
tvBaseWaitRunnable (tvBase *threeD)
{
  static char const funcName[] = "tvBaseWaitRunnable";
  PRStatus status = PR_SUCCESS;
  tvBaseLock (threeD); {
    TSMTRACE ((THREED_MASK|2, funcName, "runnable %s mapped %s",
	       threeD->runnable ? "True" : "False",
	       threeD->mapped ? "True" : "False"));
    while (! threeD->runnable || ! threeD->mapped) {
      status = PR_WaitCondVar (threeD->run_cond, PR_INTERVAL_NO_TIMEOUT);
      if (status != PR_SUCCESS)
	break;
      TSMTRACE ((THREED_MASK|2, funcName, "runnable %s mapped %s",
		 threeD->runnable ? "True" : "False",
		 threeD->mapped ? "True" : "False"));
    }
  }; tvBaseUnlock (threeD);
  return status;
}


/*
 * Synopsis: tvViewpoint * tvBaseView(tvBase const *)
 *
 * Doc++ Description: *//**
 *   Returns the particular view.
 *
 * (ISSUE: Redundant with tvBaseGetView(), found in ThreeDNav.c)
 *
 * @param threeD  handle to rendering state
 *
 * @return   The view indexed in the threeD.
 *
 * @author   Lee Iverson
 *
 * @version  May 1997
 *
 */
tvViewpoint *
tvBaseView (tvBase const * threeD)
{
  return (tvViewpoint *) &threeD->view;
}


/*
 * Signal to the visibility thread that we need to redo the visibility
 * calculations.
 */

void
tvBaseRedoVisibility (tvBase * threeD, int locked)
{
  static char const funcName[] = "tvBaseRedoVisibility";

  if (! locked) tvBaseLock (threeD);

  threeD->redo_quad = 1;
  TSMTRACE ((VIS_MASK|1, funcName, "Q %d: notify vis_cond",
	     threeD->redo_quad));
#ifdef VIS_THREAD
  PR_NotifyCondVar (threeD->vis_cond);
#endif

  if (! locked) tvBaseUnlock (threeD);
}

/*
 * Doc++ Description: *//**
 *   Signal to the visibility thread that we need to generate a new visibility
 *   list.  This will not result in a full recalculation of the visibility 
 *   tree, so should only be called when a requested tile is read.
 *
 * (ISSUE: Is this an accurate description of this function?)
 *
 * @param threeD  handle to rendering state
 * @param locked      boolean value specifying whether or not the render 
 *                    thread is locked at the time of this function invocation
 *                    (ISSUE: a bit obscure; this function is also called from  *                    several places by library modules, but only once 
 *                    externally. Perhaps refactor?)
 * @return void
 */

void
tvBaseRedoVisibleList (tvBase * threeD, int locked)
{
  static char const funcName[] = "tvBaseRedoVisibleList";

  if (! locked) tvBaseLock (threeD);

  threeD->redo_vislist = 1;
  TSMTRACE ((VIS_MASK|1, funcName, "V %d: notify vis_cond",
	     threeD->redo_vislist));
#ifdef VIS_THREAD
  PR_NotifyCondVar (threeD->vis_cond);
#endif

  if (! locked) tvBaseUnlock (threeD);
}

/*
 * Synopsis: void tvBasePostRedraw(tvBase *, int locked)
 *
 * Doc++ Description: *//**
 *   Signals to the rendering thread that a redraw is required.
 *
 * @param threeD  handle to rendering state
 * @param locked      boolean value specifying whether or not the render 
 *                    thread is locked at the time of this function invocation
 *                    (ISSUE: a bit obscure; this function is also called from  *                    several places by library modules, but only once 
 *                    externally. Perhaps refactor?)
 *
 * @return  void.
 *
 * @author  Lee Iverson
 *
 * @version February 1997
 *
 */
void
tvBasePostRedraw (tvBase * threeD, int locked)
{
  static char const *funcName = "tvBasePostRedraw";
  list_iter_t i;

  if ( ! threeD ) return;

  if (threeD->cave) {
    tsmError(funcName, "Shouldn't have a Post Redraw in a CAVE widget");
    return;
  }

  TSMTRACE ((THREED_MASK|1, funcName, "pending %s",
	     glmPendingRedraw (threeD->glmw) ? "True" : "False"));

  if (! glmPendingRedraw (threeD->glmw)) {

    glmPostRedraw (threeD->glmw);

    /* Redraw those linked widgets also */
    for (i = list_first (threeD->linkedBases);
	 ! list_done (i);
	 i = list_next (i)) {
      tvBase *base = list_value (i);
      if (base != threeD) {
	tvBasePostRedraw (base, FALSE);
      }
    }
  }
}

#if HAVE_TEXSUBIMAGE
int
tvBaseCountTexRefs (TexTileRef const *tref, void *closure)
{
  tvBase const *threeD = closure;
  VisibleDrawList const *visList = threeD->drawList;
  int count = 0;
  int i, j;

  for (j = 0; j < 3; ++j, ++visList) {
    for (i = 0; i < visList->n; ++i) {
      VisibleDrawGeoList const *geo_list = &visList->geo_list[i];
      VisibleDraw const *vis = geo_list->vis, *end = vis + geo_list->n;
      for ( ; vis < end; ++vis) {
	if (vis->tex.ref == tref)
	  ++count;
      }
    }
  }
  return count;
}

void
tvBaseCheckTexRefs (tvBase const *threeD)
{
  VisibleDrawList const *visList = threeD->drawList;
  int i, j;

  for (j = 0; j < 3; ++j, ++visList) {
    for (i = 0; i < visList->n; ++i) {
      VisibleDrawGeoList *geo_list = &visList->geo_list[i];
      TsGeoPyramid const *geo = geo_list->geo;
      TextureImage const *texImage = GetTextureImage (threeD->tvGLX, geo);
      TextureImageCheckRefs (texImage, tvBaseCountTexRefs, threeD);
    }
  }
}
#endif

void
tvBaseNextDrawList (tvBase * threeD)
{
  static char const funcName[] = "tvBaseNextDrawList";

  tvBaseLock(threeD); {
#if HAVE_TEXSUBIMAGE
    if (0 && use_subtexture) tvBaseCheckTexRefs (threeD);
#endif

    RPRTR (("Nxt: list %p\n", &threeD->drawList[threeD->currWorkList]));
    threeD->nextDrawList = threeD->currWorkList;
    threeD->currWorkList = threeD->nextWorkList;
    threeD->nextWorkList = threeD->nextDrawList;

    TSMTRACE ((THREED_MASK|VIS_MASK|1, funcName,
	       "CR = %d NR = %d CQ = %d NQ = %d",
	       threeD->currDrawList,
	       threeD->nextDrawList,
	       threeD->currWorkList,
	       threeD->nextWorkList));

#ifdef VIS_THREAD
    if (threeD->viewJumped) {
      threeD->viewJumped = 0;
      TSMTRACE ((THREED_MASK|1, funcName, "Notify jump visibility complete"));
      PR_NotifyCondVar (threeD->jumpReady);
    }

    tvBasePostRedraw (threeD, TRUE);
#endif
  }; tvBaseUnlock(threeD);
}


int
tvBaseGrabDrawList (tvBase * threeD)
{
  static char const funcName[] = "tvBaseGrabDrawList";
  int currDrawList;

  tvBaseLock(threeD); {
    currDrawList = threeD->currDrawList;
    if (currDrawList != threeD->nextDrawList) {
      threeD->nextWorkList = currDrawList;
      currDrawList = threeD->nextDrawList;
      threeD->currDrawList = currDrawList;
      RPRTR (("Grb: list %p\n", &threeD->drawList[currDrawList]));
      TSMTRACE ((THREED_MASK|VIS_MASK|1, funcName, 
		 "CR = %d NR = %d CQ = %d NQ = %d",
		 threeD->currDrawList,
		 threeD->nextDrawList,
		 threeD->currWorkList,
		 threeD->nextWorkList));
    }
  }; tvBaseUnlock(threeD);
  return currDrawList;
}

/*
 *
 * Synopsis:
 *   void tvBaseFlyToPickPoint
 *
 * Doc++ Description: *//**
 *   Perform a Fly to operation on the selected point
 *   Not a "seek". We would never do that! That's patented
 *
 * Externals: NONE
 *
 * (ISSUE: should this function be publically visible? Its use is a bit 
 * obscure for client side code, and the arguments tx, ty, and res are not 
 * used?????)
 *
 * @param threeD
 * @param res
 * @param tx
 * @param ty
 * @param p[]
 * @param data
 *
 * @return   void
 *
 * @author   Nat Bletter
 *
 * @version  03/29/99
 *
 */
void
tvBaseFlyToPickPoint(tvBase *threeD, int res, int tx, int ty, 
			   double p[], void *data)
{
  double view2pick[3], scaled[3], newPick[3];
  double yaw, pitch;
  tvViewpoint const *view = &threeD->view;

  /* don't go all the way there */
  SubVectord(p, view->eye, view2pick);
  Scalard(0.8, view2pick, scaled);
  AddVectord(scaled, view->eye, newPick);

  {
    m3f rotate;
    float const *ref = view->reference_plane;
    float e0[3], p0[3];
    float eyeLVCS[3], pickLVCS[3], diffLVCS[3];
    int i;
    glRotation_m3f (ref[0], ref[1], ref[2], ref[3], &rotate);
    for (i = 0; i < 3; ++i) {
      e0[i] = view->eye[i] - view->reference[i];
      p0[i] = p[i] - view->reference[i];
    }
    mult_v3f_m3f ((v3f *) e0, &rotate, (v3f *) eyeLVCS);
    mult_v3f_m3f ((v3f *) p0, &rotate, (v3f *) pickLVCS);
    SubVector (pickLVCS, eyeLVCS, diffLVCS);

    /* and center the view in our sights */
    yaw = atan2(-diffLVCS[0], diffLVCS[1]);
    pitch = atan2(diffLVCS[2],
		  sqrt(diffLVCS[1]*diffLVCS[1] + diffLVCS[0]*diffLVCS[0]));

  }

  tvFlyTo(threeD, newPick[0], newPick[1], newPick[2],
	  yaw, pitch, 0.0);
}

/*
 * Synopsis:
 *   void tvFlyToPickPoint(tvBase *threeD, int mx, int my )
 *
 * Doc++ Description: *//**
 *   This function provides a capability to fly to the terrain that is
 *   under the specified screen coordinates. For example, this could be
 *   used to implement a click-to-zoom-to-terrain feature.
 *
 * @param threeD  handle to rendering state
 * @param x           window coordinates of pick event
 * @param y
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  30 October 2000
 *
 */

void
tvFlyToPickPoint(tvBase *threeD, int mx, int my )
{
  if ( threeD == NULL ) return;
  
  tvBaseRegisterPickCB( threeD, mx, my, tvBaseFlyToPickPoint, NULL);
}

/*
 * Synopsis:
 *   void tvBaseRegisterPickCB(tvBase *threeD, int x, int y, 
 *            PickCB cb, void *data)
 *
 * Description: Register a pick callback function
 *
 * Externals: NONE
 *
 * Doc++ Description: *//**
 *   Registers a pick-event with the render thread; should be called for 
 *   each event. An optional callback function may be specified for further 
 *   processing.
 *
 * @param threeD  handle to rendering state
 * @param x           window coordinates of pick event
 * @param y
 * @param cb          Callback function, to be called by the render thread 
 *                    during its internal pick processing.
 * @param data        opaque data passed to cb
 *
 * @return   void
 *
 * @author   Nat Bletter
 *
 * @version  03/29/99
 *
 */
void
tvBaseRegisterPickCB(tvBase *threeD, int x, int y, PickCB cb, 
		     void *data)
{
  static char const funcName[] = "tvBaseRegisterPickCB";
  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));
  threeD->selecting = TRUE;
  threeD->select_features_only = FALSE;
  threeD->pickScreen[0] = x;
  threeD->pickScreen[1] = y;
  threeD->pickCB = cb;
  threeD->pickData = data;
  tvBasePostRedraw(threeD, FALSE);
}

/*
 * Synopsis:
 *   void tvBaseCheckForHyperlink(tvBase *threeD, int x, int y);
 *
 * Doc++ Description: *//**
 *   Check for a model with a hyperlink under the current cursor
 *   position. This function is given a mouse (x, y) coordinate
 *   and will instigate a picking process to locate the VRML
 *   geometry under that screen coordinate. If the geometry of
 *   an Anchor node is found, then the tvBase's state is updated
 *   to record the url and description of that hyperlink. This
 *   can then be queried with a tvGetString() call. 
 *
 *   N.B. this function only posts an event to begin the pick
 *   process. Upon termination of the function, it is unlikely
 *   that the operation will have completed.
 *
 *   It is suggested that this function be called whenever the
 *   mouse is moved.
 *
 * @param threeD  handle to rendering state
 * @param x       window x-coordinate of mouse
 * @param y       window x-coordinate of mouse
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  27 Oct 2000
 *
 */

void
tvBaseCheckForHyperlink(tvBase *threeD, int x, int y)
{
  static const char *funcName = "tvBaseCheckForHyperlink";

  tvBaseLock( threeD ); {
    TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));
    threeD->selecting            = TRUE;
    threeD->select_features_only = TRUE;
    threeD->pickScreen[0]        = x;
    threeD->pickScreen[1]        = y;
    threeD->pickCB               = NULL;
    threeD->pickData             = NULL;
  }; tvBaseUnlock( threeD );

  tvBasePostRedraw( threeD, FALSE );
}

/*
 * Synopsis:
 *   void tvLookAtGCC( tvBase *threeD, double gcc[3] )
 *
 * Doc++ Description: *//**
 *   Takes a geocentric coordinate and flies close to that point.
 *
 *   This function will accept a geocentric coordinate, given as an
 *   array of 3 doubles. It will initiate an animation to a point
 *   where the viewer is looking at that location. The function 
 *   automatically calculates a suitable final orientation so
 *   that the viewer is looking at the point.
 *
 *   How close the user gets to the specified coordinate depends
 *   upon their initial position. The animation will take you 80%
 *   of the way to the GCC coordinate.
 *
 * @param threeD  handle to rendering state
 * @param gcc         the geocentric coordinate to look at
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  10 February 2000
 *
 */

void
tvLookAtGCC( tvBase *threeD, double gcc[3] )
{
  tvBaseFlyToPickPoint( threeD, 0, 0, 0, gcc, NULL );
}

/*
 *
 * Synopsis:
 *   void tvBaseDrawProcessPick(tvBase *threeD,
 *                                    int x, int y, double pick[3])
 *
 * Description: returns 3D coords of pick, but *ONLY* if used from the draw process
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Nat Bletter
 *
 * Date:
 *	03/29/99
 *
 */
void
tvBaseDrawProcessPick(tvBase *threeD, int x, int y, double pick[3])
{
  threeD->selecting = TRUE;
  threeD->select_features_only = FALSE;
  threeD->pickScreen[0] = x;
  threeD->pickScreen[1] = y;
  threeD->pickCB = NULL;
  tvBaseDraw (threeD->glmw, (void*)threeD);
  CopyVecd (threeD->pickPoint, pick, 3);
  threeD->selecting = FALSE;
}

/*
 * Synopsis:
 *   void tvScreenToGCC( tvBase *threeD, int x, int y, double gcc[3] )
 *
 * Doc++ Description: *//**
 *   Returns the geocentric location under a screen coordinate.
 *
 *   This function will accept a screen coordinate (x, y) in units of
 *   pixels. It will then intersect this point with the terrain in
 *   order to find out the geocentric coordinate under that point.
 *   This coordinate is returned in the gcc array.
 *
 * @param threeD  handle to rendering state
 * @param x           x screen coordinate
 * @param y           y screen coordinate
 * @param gcc         an 3 element array filled with the GCC coordinate
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  10 February 2000
 *
 */

void
tvScreenToGCC( tvBase *threeD, int x, int y, double gcc[3] )
{
  tvBaseDrawProcessPick( threeD, x, y, gcc );
}


/*
 *
 * Synopsis:
 *   void tvBaseProcessPick(tvBase *threeD,
 *                          int hits, GLUint selectBuf[])
 *
 * Description: Figure out what object was hit and the X,Y,Z and call the
 *	select callbackn
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Nat Bletter
 *
 * Date:
 *	03/29/99
 *
 */
void
tvBaseProcessPick(tvBase *threeD, int hits, unsigned int selectBuf[])
{
  static const char funcName[] = "tvBaseProcessPick";
  unsigned int i, j;
  GLuint names, z1, z2, *ptr, *namep;
  unsigned int zmin = UINT_MAX; 
  unsigned int zmax;
  GLuint *closest = NULL, num;
  int res, tx, ty, poly;
  tvViewpoint *view = &threeD->view;
  char buffer[128];

  TSMTRACE((THREED_MASK|1, funcName, "hits = %d\n", hits));
  ptr = (GLuint *) selectBuf;
  /* first figure out which is closest hit */
  for (i = 0; i < hits; i++) {        /*  for each hit  */
    namep = ptr;
    names = *ptr++;
    z1 = *ptr++;
    z2 = *ptr++;

    TSMTRACE((THREED_MASK|1, funcName, " number of names = %d", names));
    TSMTRACE((THREED_MASK|1, funcName, "  z range = %u - %u", z1, z2));

    buffer[0] = '\0';
    for (j = 0; j < names; j++) {   /*  for each name */
      sprintf( buffer, "%s %d", buffer, *(ptr++) );
      if ( strlen( buffer ) > 100 ) break;
    }
    TSMTRACE((THREED_MASK|1, funcName, "  name stack =%s", buffer));

    if (names > 1 && z1 < zmin) {
      zmin = z1;
      zmax = z2;
      closest = namep;
    }
  }
  if (closest) {
    GLint viewport[4];
    GLdouble projMatrix[16], viewMatrix[16];
    float side1[3], side2[3], normal[3], click[4], delta[4];
    double normald[3];
    double d, t;
    GLdouble winX, winY, winZ;
    const TsCoord *coords;
    const unsigned short *coordIndex;
    double *pickPoint = threeD->pickPoint;
    VisibleDrawGeoList *geo_list;

    TsGeoPyramid *geo;
    float eye_lvcs[3];

    int currDrawList = threeD->currDrawList;
    VisibleDrawList *drawList = &threeD->drawList[currDrawList];
    VisibleDraw *vis;
    TileGeometry const *geom;

    /* Advance over num, z1, z2 */
    num = *closest;
    closest += 3;

    /* if the first name is -2, then we have clicked on a VRML feature */

    if ( *closest == -2 ) {

      /* non Anchor node selected */

      if ( num < 3 ) {
	if ( threeD->f_pick_url )
	  free( threeD->f_pick_url );
	if ( threeD->f_pick_desc )
	  free( threeD->f_pick_desc );
	threeD->f_pick_url  = NULL;
	threeD->f_pick_desc = NULL;
	threeD->selecting   = FALSE;
	threeD->select_features_only = FALSE;
	return; 
      }

      /* Anchor node selected */

      FeatureSetHyperlink( threeD, closest[1], closest[2] );
      threeD->f_pick_id     = closest[1];
      threeD->f_pick_anchor = closest[2];
      threeD->selecting     = FALSE;
      threeD->select_features_only = FALSE;

      TSMTRACE((THREED_MASK|1, funcName, "VRML Model Clicked!"));
      TSMTRACE((THREED_MASK|1, funcName, "  url  = %s\n",
		(threeD->f_pick_url) ? threeD->f_pick_url : "none" ));
      TSMTRACE((THREED_MASK|1, funcName, "  desc = %s\n",
		(threeD->f_pick_desc) ? threeD->f_pick_desc : "none" ));

      return;
    }

    /* if we were only searching for a VRML hyperlink, then abort here */

    if ( threeD->select_features_only ) {
      threeD->selecting = FALSE;
      threeD->select_features_only = FALSE;
      return;
    }

    /* Otherwise we have terrain - Get the z0'th element of geoList */

    geo_list = &drawList->geo_list[*closest++];

    /* Now grab the geo */

    geo = geo_list->geo;
    
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glPushMatrix(); {
      tvBaseGLViewpointLVCS (view, geo);
      glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrix);
    }; glPopMatrix();

    winX = (double) threeD->pickScreen[0];
    winY = (double) threeD->height - (double) threeD->pickScreen[1];
    winZ = (double) zmax / (double) 0xffffffffU;
    gluUnProject(winX, winY, winZ,
		 viewMatrix, projMatrix, viewport, 
		 &pickPoint[0], &pickPoint[1], &pickPoint[2]);

    /* pick coordinate is wrt. eye, but in LVCS coords, so... */
    /* convert eye to LVCS coords. */
    TsGeoGCCtoLVCS (geo, view->eye[0], view->eye[1], view->eye[2],
		    eye_lvcs);

    /* add eye coords back in that were subtracted out at projection */
    pickPoint[0] += eye_lvcs[0];
    pickPoint[1] += eye_lvcs[1];
    pickPoint[2] += eye_lvcs[2];

    TSMTRACE((THREED_MASK|1, funcName, " Selected point = %.0f %.0f %.0f",
	      pickPoint[0],  pickPoint[1], pickPoint[2]));

    /* now compute X,Y,Z of closest hit */
    res  = *(closest++);
    tx   = *(closest++);
    ty   = *(closest++);
    poly = *(closest++);
    TSMTRACE((THREED_MASK|1, funcName,
	      "Closest hit z = %u, res = %d, tx = %d, ty = %d, poly = %d",
	      zmin, res, tx, ty, poly));

    /* retrieve the tile geometry from the list of primitives */
    {
      TsTile *tile = TsGeoTile (geo, tx, ty, res);
      if (tile) {
	int i, j;
	for (i = 0, vis = 0; ! vis && i < drawList->n; ++i) {
	  VisibleDrawGeoList const *geo_list = &drawList->geo_list[i];
	  for (j = 0; j < geo_list->n; ++j) {
	    if (geo_list->vis[j].tile == tile) {
	      vis = &geo_list->vis[j];
	      break;
	    }
	  }
	}
      }
      if (! tile || ! vis) {
	tsmError(funcName, "Couldn't find picked tile in visible List!");
	threeD->selecting = FALSE;
	return;
      }
    }

    geom = vis->prim->geometry;
    coords = vis->prim->coords;
    coordIndex = geom->coordIndex;

    /* find the plane equation of the hit poly */
    SubVector((float *)(&coords[coordIndex[poly]].vertex),
	      (float *)(&coords[coordIndex[poly+1]].vertex),
	      side1);
    SubVector((float *)(&coords[coordIndex[poly+2]].vertex),
	      (float *)(&coords[coordIndex[poly]].vertex),
	      side2);

    /* fix negative X, should be taken out with global coords */
    CrossProduct(side1, side2, normal);
    NormalizeVector(normal, normal, 3);
    ConvertFtoD(normal, normald, 3);
    d = - (normal[0] * coords[coordIndex[poly]].vertex[0] +
	   normal[1] * coords[coordIndex[poly]].vertex[1] +
	   normal[2] * coords[coordIndex[poly]].vertex[2]);

    TSMTRACE((THREED_MASK|1, funcName,
	      "Hit plane A B C D = %1.2f %1.2f %1.2f %1.2f",
	      normal[0],  normal[1],  normal[2], d));

    /* construct viewpoint to mouse click point vector */
    click[0] = view->nearPlane * 
      tan((M_PI / 180.0) * (float) view->fov) * view->aspect *
      ( (float) threeD->pickScreen[0] / 
	(float) threeD->width - 0.5);
    click[1] = - view->nearPlane * 
      tan((M_PI / 180.0) * (float) view->fov) * 
      ( (float) threeD->pickScreen[1] / 
	(float) threeD->height - 0.5);
    click[2] = -view->nearPlane;

    /* transform to gaze coords in LVCS */
    MultVecMatrix3 (click, (const_GLmatrixd) viewMatrix, delta);

    TSMTRACE((THREED_MASK|1, funcName,
	      "Clicked line delta = %1.2f %1.2f %1.2f", 
	      delta[0],  delta[1],  delta[2]));

    /* param of line equation intersection with plane */
    t = -(DotProduct (normal, eye_lvcs, 3) + d)
      / DotProduct (normal, delta, 3);

    pickPoint[0] = eye_lvcs[0] + t * delta[0];
    pickPoint[1] = eye_lvcs[1] + t * delta[1];
    pickPoint[2] = eye_lvcs[2] + t * delta[2];

    TSMTRACE((THREED_MASK|1, funcName,
	      "Selected t param = %1.2f Selected LVCS = %.0f %.0f %.0f",
	      t, pickPoint[0],  pickPoint[1],  pickPoint[2]));

    TsGeoLVCStoGCC (geo, pickPoint[0],  pickPoint[1],  pickPoint[2],
		    pickPoint);

    TSMTRACE((THREED_MASK|1, funcName, "Selected GCC = %.0f %.0f %.0f",
	      pickPoint[0],  pickPoint[1],  pickPoint[2]));

    if (threeD->pickCB)
      threeD->pickCB(threeD, res, tx, ty, pickPoint, 
		     threeD->pickData);

    /* yeah, done */
  }

  threeD->selecting = FALSE;
}


/*
 *
 * Synopsis:
 *   void tvBaseSetClipPlanes( tvBase *threeD )
 *
 * Description:
 *   Looks at the bounding box of all visible tiles for this viewpoint
 *   in order to try and set the near and far clip planes to good
 *   values for this frame. This is done by setting the far plane to
 *   the furthest point of visible geometry, and then setting the
 *   near plane so that we keep the ratio near:far within a prescribed
 *   range to eliminate flimmering. We also impose a minimum value
 *   on the near plane so that if the near plane is set to less than
 *   this value, then we threshold it and then push out the far plane
 *   as far as we can based upon the prescribed near:far ratio.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 April, 1999.
 *
 */

static void
tvBaseSetClipPlanes( tvBase *threeD )
{
  const char  *funcName = "tvBaseSetClipPlanes";
  int         currDrawList = threeD->currDrawList;
  VisibleDrawList *drawList = &threeD->drawList[currDrawList];
  TsBBox      *bbox = &drawList->bbox;
  
  const float nearfar_ratio = 10000.0;  /* keep near:far in 1:nearfar_ratio */
  /* const float max_far       = 1e10; */

  /* check that we have a valid bounding box for the visible list */

  if ( bbox->min[0] > bbox->max[0] ) {
    TSMTRACE(( THREED_MASK|1, funcName, "Invalid bbox: using last settings" ));
    return;
  }

  /* lock down the threeD because we want to read and */
  /* change its view field with the new near/far settings.  */

  if (threeD->computedNearFar) {

    tvBaseLock( threeD ); {
      double f;
      tvViewpoint *view = &threeD->view;
      /* double farD[3]; */
      double    n;
      double const *eye = view->eye;
      TsTile const *nearTile = threeD->nearTile;

      if (! nearTile) {
	TSMTRACE ((THREED_MASK|2, funcName, "near tile not set"));
	tvBaseUnlock (threeD);
	return;
      }

      TSMTRACE ((THREED_MASK|2, funcName, "near tile [%d %d %d]",
		 nearTile->id.x, nearTile->id.y, nearTile->id.res));

      TSMTRACE ((THREED_MASK|2, funcName, "eye [%.1f %.1f %.1f]",
		 eye[0], eye[1], eye[2]));

      if (threeD->inside) {
	/* we're inside a bbox */
	TSMTRACE ((THREED_MASK|2, funcName,
		   "insideTile.radius %.0f altitude %.0f",
		   threeD->nearTile->bbox.radius, view->altitude));
	n = min (threeD->nearTile->bbox.radius / 100.0, view->altitude);
      } else {
	/* We need to determine the distance from the eye plane to the near
	   point.  We thus need the equation for the eye plane which can be
	   obtained by recovering the forward direction from the viewMatrix
	   (this is the plane normal) and then computing the offset by taking
	   the inner product of this vector and the eye point. */
	double const *nearP = threeD->nearPoint;
	v3f P;
	double d;
	{
	  int i;
	  for (n = 0, i = 0; i < 3; ++i) {
	    d = nearP[i] - eye[i];
	    n += d*d;
	  }
	  TSMTRACE ((THREED_MASK|2, funcName, 
		     "actual dist %.1f", sqrt (n)));
	}
	tvHeadingForward ((m4f *) threeD->viewMatrix, &P, 1.0);
	d = - (V(P)[0]*eye[0] + V(P)[1]*eye[1] + V(P)[2]*eye[2]);
	TSMTRACE ((THREED_MASK|2, funcName,
		   "eye plane %.1f,%.1f,%.1f,%.1f",
		   V(P)[0], V(P)[1], V(P)[2], d));
	n = V(P)[0]*nearP[0] + V(P)[1]*nearP[1] + V(P)[2]*nearP[2] + d;
	TSMTRACE ((THREED_MASK|2, funcName,
		   "near plane dist %.1f", n));
      }
      n *= 0.99;

      if (n > view->altitude)
	n = view->altitude;
      if (n < MIN_NEAR_PLANE) 
	n = MIN_NEAR_PLANE;

      view->nearPlane = n;

      /* okay, let's make the furthest point a ratio of the near plane */
      /* we then calculate the far plane distance by observing that, in  */
      /* order to keep Z-buffer flimmering down, we should keep the ratio */
      /* between the near:far to within somewhere around 1:10000 for a    */
      /* 32-bit Z buffer.                                                 */

#if 0
      SubVectord (view->eye, threeD->farPoint, farD);
      f = LengthVecd (farD, 3);

      if (f > max_far)
	f = max_far;

      if (f < n * nearfar_ratio)
	view->farPlane = n * nearfar_ratio;
      else
	view->farPlane = f;
#endif

      /****** Start of hardcoded hybrid section ******/

      /* May not be elegant, but everything works great if you do */
      /* this. Currently we make this a hybrid solution, taking   */
      /* the minimum clipping plane from this calculation and the */
      /* above calculation based upon the closest tile. This main */
      /* issue is to not unduly clip GeoVRML models in the scene. */

#if 0
      if ( view->altitude < 10000 )
	view->nearPlane = min( view->nearPlane, 10.0 );
      else if ( view->altitude < 100000 )
	view->nearPlane = min( view->nearPlane, 100.0);
      else if ( view->altitude < 1000000 )
	view->nearPlane = min( view->nearPlane, 1000.0 );
      else if ( view->altitude < 10000000 )
	view->nearPlane = min( view->nearPlane, 10000.0 );
      else
	view->nearPlane = min( view->nearPlane, 100100.0 );
#endif

      if ( view->altitude < 10000 )
	view->nearPlane = 10.0;
      else if ( view->altitude < 100000 )
	view->nearPlane = 100.0;
      else if ( view->altitude < 1000000 )
	view->nearPlane = 1000.0;
      else if ( view->altitude < 10000000 )
	view->nearPlane = 10000.0;
      else
	view->nearPlane = 100100.0;

      f = view->nearPlane * nearfar_ratio;
      if ( view->farPlane < f )
	view->farPlane = f;

      /****** End of hardcoded hybrid section ******/

      /* change stuff for stereo rendering */
      if (threeD->stereo) {
	/* change it to match the scale of the world */
	threeD->ipd =
	  (threeD->scaleIPD
	   ? threeD->absoluteIPD * view->nearPlane / 50.0
	   : threeD->absoluteIPD);
      }

      TSMTRACE ((THREED_MASK|1, funcName,
		 "Setting near = %.1f, far = %.0f",
		 view->nearPlane, view->farPlane));

    }; tvBaseUnlock (threeD);
  }
}


#define TIMING (TSMTRACEP (THREED_MASK|2))

#ifdef TIMING
static void 
_3DReportElapsed (PRIntervalTime start, PRIntervalTime upd,
		  char const *fmt, ...)
{
  char buf[512];
  va_list ap;
  PRIntervalTime cur;

  glFinish ();
  cur = PR_IntervalNow();
  sprintf (buf, "E:%6.1fms ", 1000 * tsmElapsedTime (start, cur));
  if (upd) {
    sprintf (buf+strlen(buf), "D:%6.1fms ", 1000 * tsmElapsedTime (upd, cur));
    upd = cur;
  }
  va_start (ap, fmt);
  vsprintf (buf+strlen(buf), fmt, ap);
  va_end (ap);
  TSMTRACE ((0, "%%%", "%s", buf));
}
#endif

/*
 *
 * Synopsis:
 *   void tvBaseDraw(tvBase *)
 *
 * Description: Render the tvBase window
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *	03/29/93
 *
 */
void
tvBaseDraw(glmWindow glmw, void *closure)
{
  static const char funcName[] = "tvBaseDraw";
  PRIntervalTime  t_start, t_prev;
  int             numDrawnPolys;
  int             newTextures;
  int             totalTextures;
  int             i, redraw = FALSE;
  PRUint8         saveWireframe;
  /* TsGeoPyramid    *geo; */  /* used to test for NetLogger logging */

#define SELECTSIZE 500
  GLuint selectBuf[SELECTSIZE];
  int eye, eyeMax;

  tvBase * threeD = (tvBase*) closure;

  tvViewpoint *view = &threeD->view;
  VisibleDrawList *drawList;

  int currDrawList;
  int frame;
  tvViewpoint curr_view;

  static PRIntervalTime last_frame_ts = 0;    /* previous timestamp */
  PRIntervalTime curr_frame_ts;               /* this timestamp */
  static float fps_list[AV_FRAME_COUNT] = { 0.0 };     /* past frame rates */
  static int curr_frame_index = 0;                    /* index to fps_list */

  int    path_playing = PathIsPlaying (threeD);   /* are we animating */

  if (! threeD->mapped)
    return;

  /* do we have anything to render? */

  if ( list_empty (threeD->geoList) ) {
    tvDrawLoadScreen (threeD);
    return;
  }

  /* If the last movement was a jump, wait for visibility calc */
#ifdef VIS_THREAD
  tvBaseLock( threeD ); {
    while (! threeD->freezeTiles && threeD->viewJumped) {
      TSMTRACE ((THREED_MASK|1, funcName, "Wait for jump visibility"));
      PR_WaitCondVar (threeD->jumpReady, PR_INTERVAL_NO_TIMEOUT);
    }
  }; tvBaseUnlock( threeD );
#endif

  /* Calculate the average and instantaneous frame rate */

  frame = ++threeD->frame;

  /* if this frame was not caused by a user interaction, then we  */
  /* do not define the instantaneous frame rate and do not update */
  /* the average frame rate value.                                */

  if ( threeD->moveTimer == NULL && path_playing == FALSE )
    last_frame_ts = 0;

  curr_frame_ts = PR_IntervalNow();

  if ( last_frame_ts != 0 ) {  
    /* not right really...NSPR interval timers can 
       be negative, and can wrap --RLB */
    float dt;
    int   frame_rates = 0;

    /* Calculate the instantaneous (since last frame) frame rate in Hz */

    if ( ( dt = tsmElapsedTime( last_frame_ts, curr_frame_ts ) ) != 0.0 )
      threeD->frame_rate = 1.0 / dt;

    /* Calculate the averaged frame rate in Hz */

    fps_list[curr_frame_index] = threeD->frame_rate;
    curr_frame_index = ( curr_frame_index + 1 ) % AV_FRAME_COUNT;

    threeD->frame_rate_av = 0.0;
    for ( i = 0; i < AV_FRAME_COUNT; i++ ) {
      if ( fps_list[i] > 0.0 ) {
	threeD->frame_rate_av += fps_list[i];
	frame_rates++;
      }
    }

    /* only calculate average frame rate if we have more than 2 values */

    if ( frame_rates > 2 )
      threeD->frame_rate_av /= frame_rates;
    else
      threeD->frame_rate_av = 0.0;
    
  } else
    threeD->frame_rate = 0.0;

  last_frame_ts = curr_frame_ts;

  /* save the state of the wireframe rendering mode because we */
  /* may change it based upon the speed and av frame rate.     */

  saveWireframe = threeD->showWireframe;

  /* if we are moving and drop below a specified average frame */
  /* then we switch the display into wireframe mode. Note,     */
  /* setting MOTION_THRESHOLD to 0.0 will disable this feature */
  /* If we are already dropped to wireframe, then we keep in   */
  /* wireframe mode until the viewer stops moving.             */

  if ( ( threeD->frame_rate_av < threeD->motionThreshold || 
	 threeD->motionDegraded ) && threeD->frame_rate_av > 0.0 ) {

    if ( threeD->moveTimer != NULL || path_playing ) {
      threeD->showWireframe = TRUE;
      threeD->motionDegraded = TRUE;
    } else
      threeD->motionDegraded = FALSE;
  }

  /* Enable gl error checking only if tracing OpenGL */
  gltEnableCheckErrors (TSMTRACEP (THREED_MASK|OGL_MASK|1));

  TSMTRACE ((THREED_MASK|1, funcName, "Begin: frame %d", frame));
  TSMTRINC ((THREED_MASK|1, 1));

  /* INITIALIZE PLACEHOLDERS IF THE USER WANTS STATS */
  numDrawnPolys = 0;
  newTextures = 0;
  totalTextures = 0;

  t_start = PR_IntervalNow();  t_prev = t_start;

  if (threeD->cave) {
    glPolygonMode(GL_FRONT_AND_BACK,
		  threeD->showWireframe ? GL_LINE : GL_FILL);

    tvBaseDrawFog (threeD);

    if (! glIsList (threeD->sunLight_list))
      threeD->sunLight_list =  glGenLists (1);
    glNewList (threeD->sunLight_list, GL_COMPILE); {
      gltLightDefv (GL_LIGHT0, sunLight);
    }; glEndList();

    glFlush();
  } else {
    /* GET THE GRAPHICS LOCK */
    glmMakeCurrentWindow (funcName, threeD->glmw);
  }

  if (TIMING)
    _3DReportElapsed (t_start, t_prev, "Calc view");

  if (threeD->selecting) {
    glSelectBuffer(SELECTSIZE, selectBuf);
    glRenderMode (GL_SELECT);
    glInitNames();
    glPushName(-1);

    /* initialise the the current hyperlink state */

    threeD->f_pick_id     = -1;
    threeD->f_pick_anchor = -1;
    if ( threeD->f_pick_url ) free( threeD->f_pick_url );
    if ( threeD->f_pick_desc ) free( threeD->f_pick_desc );
    threeD->f_pick_url    = NULL;
    threeD->f_pick_desc   = NULL;

    /* do not flush this frame out to the screen if picking */

    glmSetWindowNoFlush( threeD->glmw, TRUE );
    redraw = TRUE;
  }

  /* Check for a viewport update. */
  if (threeD->update_viewport) {
    glViewport( 0, 0, threeD->width, threeD->height );
    threeD->update_viewport = FALSE;
  }

  if (TIMING)
    _3DReportElapsed (t_start, t_prev, "Lock");

  /* setup stuff for stereo rendering */
  if (threeD->stereo) 
    eyeMax = 2;
  else
    eyeMax = 1;

  /* GRAB THE DRAW LIST INDEX */
  currDrawList = tvBaseGrabDrawList (threeD);
  drawList = &threeD->drawList[currDrawList];
  TSMTRACE ((THREED_MASK|OGL_MASK|1, funcName, "visibleList[%d]",
	     currDrawList));

  /* Move to actual position. */
  if (view->vel != 0.0)
    tvCurrentView (threeD, &curr_view);
  else
    tvBaseGetView (threeD, &curr_view);

#ifndef VIS_THREAD
  TSMTRACE ((VIS_MASK|1, funcName, "freeze %d Q %d V %d",
	     threeD->freezeTiles,
	     threeD->redo_quad, threeD->redo_vislist));
  if (! threeD->freezeTiles
      && (threeD->redo_quad || threeD->redo_vislist)) {
    PRIntervalTime startTime, endTime;
    int numTiles;
    static int draw_count;

    startTime = PR_IntervalNow();
    numTiles = tvBaseGenerateVisible (threeD, &curr_view, &draw_count);
    endTime = PR_IntervalNow();

    if (numTiles != -1 && TsIsLogging(TV_VISIBLE_LOG))
      TsLogVisibleData(startTime, endTime, numTiles);
  }
#endif

  /* PREPARE THE TILES FOR DRAWING BY DOWNLOADING TEXTURES ETC. */
  tvBasePrepareTiles (threeD,
		      drawList,
		      &numDrawnPolys, 
		      &newTextures,
		      &totalTextures);

  if (TIMING) {
    _3DReportElapsed (t_start, t_prev, 
		      "Prepare: %d/%d", newTextures, totalTextures);
    t_prev = t_start;
  }

  /* Move to actual position. */
  if (view->vel != 0.0)
    tvCurrentView (threeD, &curr_view);
  else
    tvBaseGetView (threeD, &curr_view);

#define YSTEREO 491
#define YOFFSET_LEFT 532
  for (eye = 0; eye < eyeMax; eye++ ) {
    int stereoVisual = threeD->stereoVisual;
    int stereo = threeD->stereo;
    int leftEye = TRUE;

    if (stereo) {
      if (eye == 0) {
	leftEye = TRUE;
	if (stereoVisual)
	  glDrawBuffer(GL_BACK_LEFT);
	else
	  glViewport(0, YOFFSET_LEFT, threeD->width, YSTEREO);
      }
      else {
	leftEye = FALSE;
	if (stereoVisual)
	  glDrawBuffer(GL_BACK_RIGHT);
	else
	  glViewport(0, 0, threeD->width, YSTEREO);  
      }
    }

    /* Set the near and far clipping planes automatically */
    tvBaseSetClipPlanes (threeD);

    /* Set the perspective projection based on calculated near/far planes. */
    tvBaseGLPerspective (threeD, 10.0, 100000.0, stereo, leftEye);

    /* Set viewpoint perspective */
    tvBaseGLAttitude (threeD);

    /* DRAW THE BACKGROUND */
    tvBaseDrawBackGround (threeD, drawList, &numDrawnPolys,
			  stereo, stereoVisual, eye, t_start, t_prev);

    if (TIMING)
      _3DReportElapsed (t_start, t_prev, "Background");
    else if (threeD->tileByTile)
      glFlush ();

    /* Set the perspective projection based on calculated near/far planes. */
    tvBaseGLPerspective (threeD, 
			 threeD->view.nearPlane,
			 threeD->view.farPlane,
			 stereo, leftEye);
    glMatrixMode (GL_MODELVIEW);

    glLineWidth((GLfloat)(1));

    if (threeD->showLighting) {
      glEnable (GL_LIGHTING);
      glEnable (GL_LIGHT0);
      glCallList(threeD->sunLight_list);
      glCallList(ground_list);

      glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glEnable (GL_COLOR_MATERIAL);
    } else {
      glDisable (GL_LIGHTING);
      glDisable (GL_LIGHT0);
    }

    /* work out whether this tvBase includes a whole earth dataset */
    
    if ( threeD->showGround ) {
      int is_whole_earth = FALSE;

      /* Work out whether this tvBase includes a whole earth dataset */
      for (i = 0; i < drawList->n; ++i) {
	VisibleDrawGeoList const *geo_list = &drawList->geo_list[i];
	if ( TsGeoWholeWorld(geo_list->geo) && !TsGeoIsSky(geo_list->geo) ) {
	  /* Check that it is drawable */
	  if (TsCacheTilePrepared (geo_list->vis[0].tile)) {
	    TSMTRACE ((THREED_MASK|1, funcName, "%s displayed. No ground",
		       TsGeoName (geo_list->geo)));
	    is_whole_earth = TRUE;
	    break;
	  }
	}
      }

      /* DRAW THE GROUND */
      if (! is_whole_earth) {
	glPushAttrib (GL_ENABLE_BIT); {
	  glColor3f (1.0, 1.0, 1.0);
	  glEnable (GL_TEXTURE_2D);
	  glEnable (GL_CULL_FACE);
	  glCullFace (GL_BACK);
	  glPushMatrix (); {
	    static float sphere_diffuse[] = {105/255.0, 62/255.0, 0, 1.0};
	    glTranslated (-curr_view.eye[0],
			  -curr_view.eye[1],
			  -curr_view.eye[2]);
	    if (0) {
	      glMaterialfv (GL_FRONT, GL_DIFFUSE, sphere_diffuse);
	      glEnable (GL_LIGHTING);
	      glEnable (GL_LIGHT0);
	      glCallList(threeD->sunLight_list);
	    }
	    tvBaseDrawSphere ();
	  }; glPopMatrix ();
	}; glPopAttrib ();
	glClear (GL_DEPTH_BUFFER_BIT);
      }
    }

    tvBaseDrawTiles (threeD,
		     drawList,
		     &numDrawnPolys, 
		     t_start, t_prev);
    TSMTRACE ((THREED_MASK|OGL_MASK|1, funcName, "done [%d]", currDrawList));

    glDisable (GL_LIGHT0);
    glDisable (GL_LIGHTING);
    glDisable (GL_COLOR_MATERIAL);

    tvDrawOverlays (threeD, newTextures, totalTextures);
  } /* stereo loop */

  /* calculate the image quality for this rendered scene. This is */
  /* computed as the ratio of the number of tiles that are loaded */
  /* to the max number of tiles in the quad tree for this view.   */

  threeD->image_quality = 
    totalTextures / (float) drawList->maxTiles;

#if 0
  if (totalTextures > drawList->maxTiles) {
    VisibleDrawGeoList const
      *glist = drawList->geo_list, 
      *gend = glist + drawList->n;
    printf ("totalTextures %d > maxTiles %d {\n",
	    totalTextures, drawList->maxTiles);
    for ( ; glist < gend; ++glist) {
      VisibleDraw *vis = glist->vis, *end = vis + glist->n;
      for ( ; vis < end; ++vis) {
	TsTileId const *id = &vis->tile->id;
	VisibleDraw *p = glist->vis;
	for ( ; p < vis; ++p) 
	  if (p->tile->id.x == id->x/2
	      && p->tile->id.y == id->y/2
	      && p->tile->id.res == id->res+1)
	    break;
	if (p < vis)
	  printf ("  [%2d %2d %2d] < [%2d %2d %2d]",
		  id->x, id->y, id->res, id->x/2, id->y/2, id->res+1);
	else 
	  printf ("  [%2d %2d %2d]",
		  id->x, id->y, id->res);
	if (vis->flags & VISFLAG_LOWRES)
	  printf (" (FRAG)");
	printf ("\n");
      }
    }
    printf ("}\n");
  }
#endif

  if (threeD->selecting) { /* get the results of a pick */
    GLint hits = glRenderMode(GL_RENDER);
    if (hits > 0)
      tvBaseProcessPick(threeD, hits, selectBuf);
  }

  if (TIMING)
    _3DReportElapsed (t_start, 0, "%d: Total", threeD->frame);

  /* are we in the CAVE environment? */

  if (! threeD->cave) {
    glmReleaseWindow (funcName, threeD->glmw);

    if ( TsIsLogging(TV_RENDER_LOG) ) 
      tvBaseTiming( numDrawnPolys, newTextures, totalTextures, t_start);
  }

#ifdef CAVE
  else {
    /* DRAW THE MAP ON THE SCREEN */
    if(CAVEWall != CAVE_SCREEN3_WALL) {
      ScannedMapCAVERedraw();
    }
    CAVEDisplayBarrier();
  }
#endif /* CAVE */

  if (TSMTRACEP (THREED_MASK|OGL_MASK|1)) {
    double elapsed;
    PRIntervalTime t_end;
    t_end = PR_IntervalNow();
    elapsed = tsmElapsedTime (t_start, t_end);
    TSMTRACE ((THREED_MASK|OGL_MASK|1, funcName,
	       "%d [%g,%g,%g] Elapsed %g msec",
	       threeD->frame, view->eye[0], view->eye[1], view->eye[2],
	       elapsed*1000.0));
  }

#if 0
  /* output a Netlogger log entry for this frame, only if logging */
  /* this only checks whether the first geopyramid has logging on */

  if ( TsGeoIsLogging( (geo = tvBaseTop(threeD)) ) ) {
    TsmConnection connects[4];
    if ( TsGeoConnections( geo, connects ) > 0 ) {
      tsmLog( connects[0], "TV_FRAME", "RATE=%1.2f QUALITY=%1.2f",
	      threeD->frame_rate, threeD->image_quality );  
    }
  }
#endif

  /* return the tvBase to its previous rendering state */

  threeD->showWireframe = saveWireframe;

  /* if this was a picking draw, then refresh with an actual draw */

  if ( redraw )
    tvBasePostRedraw (threeD, FALSE);

  TSMTRINC ((THREED_MASK|1, -1));
  TSMTRACE ((THREED_MASK|1, funcName, "End"));
}

static void
tvDrawOverlays (tvBase *threeD, int newTextures, int totalTextures)
{
  
  glPushAttrib (GL_ENABLE_BIT|GL_CURRENT_BIT); {
    glDisable (GL_LIGHTING);
    glDisable (GL_TEXTURE_2D);

    /* TURN OFF THE FOG */
    if (threeD->fog_params[0] > 0.0)
      glDisable (GL_FOG);

    /* The first set of overlays are terrain-integrated.  For this, we leave
       on depth testing. */
#if 0
    /* and draw pick point */

    glPushMatrix(); {
      glTranslatef(- threeDWidget->view.eye[0],
		   - threeDWidget->view.eye[1],
		   - threeDWidget->view.eye[2]);
      glColor4ub(0xFF, 0xFF, 0x00, 0xFF);
      glPointSize((GLfloat)(10));
      glBegin( GL_POINTS ); {
	glVertex3d(threeD->pickPoint[0],
		   threeD->pickPoint[1] + 5,
		   threeD->pickPoint[2] );
      }; glEnd();
    }; glPopMatrix();
#endif

    if (threeD->showPaths) {
      list_iter_t i;

      glPushMatrix(); {
	glTranslatef(- threeD->view.eye[0],
		     - threeD->view.eye[1],
		     - threeD->view.eye[2]);
	for (i = list_first (threeD->pathLinks); 
	     ! list_done (i);
	     i = list_next (i)) {
	  tvBase *other = list_value (i);
	  PathDraw3D (other->path, &threeD->view);
	}
      }; glPopMatrix();
    }

    if (threeD->showFrustums) {
      list_iter_t i;

      glPushMatrix(); {
	glTranslatef(- threeD->view.eye[0],
		     - threeD->view.eye[1],
		     - threeD->view.eye[2]);
	for (i = list_first (threeD->frustumLinks);
	     ! list_done (i);
	     i = list_next (i)) {
	  tvBase *other = list_value (i);
	  tvBaseDrawFrustumCB (other, other->showWireframe);
	}
      }; glPopMatrix();
    }

    if (threeD->showLatLong) {
      int i, j;
      double eye_gdc[3];
      double gcc[3];
      glPushMatrix(); {
	glTranslatef(- threeD->view.eye[0],
		     - threeD->view.eye[1],
		     - threeD->view.eye[2]);
	TsGCCtoGDC (threeD->view.eye[0],
		    threeD->view.eye[1],
		    threeD->view.eye[2],
		    eye_gdc);
	glColor3f (0.5, 0.5, 0.0);
	for (i = -180; i < 180; i += 1) {
	  double long_diff = eye_gdc[0]-(double)i;
	  while (long_diff > 180) long_diff -= 360;
	  while (long_diff < -180) long_diff += 360;
	  if (fabs (long_diff) <= 90) {
	    glLineWidth ((i%10 == 0) ? 2 : 1);
	    glBegin (GL_LINE_STRIP);
	    for (j = -90; j <= 90; j += 2) {
	      TsGDCtoGCC ((double)i, (double)j, 1000.0, gcc);
	      glVertex3f (gcc[0], gcc[1], gcc[2]);
	    }
	    glEnd ();
	  }
	}
	{
	  double lng_start = eye_gdc[0]-90;
	  double lng_end = eye_gdc[0]+90;
	  double lng;
	  for (j = -80; j <= 80; j += 1) {
	    glLineWidth ((j%10 == 0) ? 2 : 1);
	    glBegin (GL_LINE_STRIP);
	    for (lng = lng_start; lng <= lng_end; lng += 2) {
	      TsGDCtoGCC (lng, (double)j, 1000.0, gcc);
	      glVertex3f (gcc[0], gcc[1], gcc[2]);
	    }
	    glEnd ();
	  }
	}
      }
    }

    /* Draw handwriting over screen from doodle option */
    RenderSentence( threeD, TRUE );
  
    /* Beyond here are non-terrain integrated overlays. */
    glDisable (GL_DEPTH_TEST);

    glPushMatrix (); {
      glLoadIdentity();

      glMatrixMode(GL_PROJECTION);
      glPushMatrix (); {
	glLoadIdentity();
	glOrtho(-0.0, threeD->width -0.0,
		-0.0, threeD->height-0.0,
		-1.0, 1.0);
      
	/* DRAW THE CROSS HAIR */
	if (threeD->showCrossHair) {
#ifdef CAVE
	  if (! threeD->cave
	      || ((CAVEWall != CAVE_SCREEN3_WALL) && (CAVEEye != CAVE_RIGHT_EYE)))
#endif
	    tvDrawCrossHair (threeD);
	}

	/* Draw HUD */
	if (threeD->headsUp)
	  tvDrawHeadsUp (threeD, newTextures, totalTextures);

	/* Draw SRI logo in bottom-right corner if web icons are off */
	if (threeD->showLogo) 
	  tvBaseSRILogo (threeD);

      }; glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
    }; glPopMatrix();
  }; glPopAttrib ();
}

static void
tvDrawCrossHair (tvBase const *threeD)
{
  /* DRAW THE CROSS HAIR */
  glColor4ub(0xFF, 0x00, 0x00, 0xFF);
  glLineWidth((GLfloat)(1));
  glBegin(GL_LINES); {
    float x = threeD->crossHair[0];
    float y = threeD->height - threeD->crossHair[1];

    glVertex2f(x-20, y);
    glVertex2f(x+20, y);
    glVertex2f(x, y-20);
    glVertex2f(x, y+20);
  }; glEnd();
}

static void
tvDrawHeadsUp (tvBase const * threeD,
	       int newTiles, int numTiles)
{
  tvViewpoint const *view = &threeD->view;
  static char       frame_desc[32];
  list_iter_t       i;
  char              buf[64];
  const int         line_sep = 12;
  int               y = 8;
  unsigned long     value;
  char const        *units;
  int               path_playing = PathIsPlaying (threeD);
  VisibleDrawList   const *drawList = &threeD->drawList[threeD->currDrawList];
  int               debug = threeD->headsUpDebug;
  int               nlines = 3;

  if (path_playing)
    ++nlines;
  if (debug)
    nlines += 4;

  glEnable  (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* Draw dark, translucent background */
  glColor4ub (0x00, 0x00, 0x00, 0x30);
  glRecti (0, 0, 250, nlines*line_sep);

  /* Text color */
  glDisable  (GL_BLEND);
  glColor4ub (0xff, 0xff, 0xff, 0xff);

  if (TSMTRACEP (OGL_MASK|2)) {
    sprintf (frame_desc, "tvDrawHeadsUp[%d]: Enter", threeD->frame);
    gltTraceState (OGL_MASK|2, frame_desc);
  }

  /* Frame rate */
  if (debug) {
    sprintf (buf, (threeD->frame_rate >= 2.0)?
	     "Speed: %2.0f fps (av. %2.0f fps)" :
	     "Speed: %3.1f fps (av. %3.1f fps)",
	     threeD->frame_rate, threeD->frame_rate_av );
    glRasterPos2i (8, y);  y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);

    value = TsLogGetTotalBytesRead() / 1024; units = "KB";
    if ( value > 1024 ) { value /= 1024; units = "MB"; }
    if ( value > 1024 ) { value /= 1024; units = "GB"; }

    sprintf (buf, "Net: %1.1f Mbit/s  (%ld %s)", TsLogGetNetBurstRate(),
	     value, units );
    glRasterPos2i (8, y);  y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);

    /* frame number and tiles in scene */

    sprintf (buf, "Frame %d_%d: %d/%d/%d tiles",
	     threeD->frame, drawList->id,
	     numTiles-newTiles, numTiles, drawList->maxTiles);
    glRasterPos2i (8, y);  y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);

    /* near and far distances */

    sprintf (buf, "Clip: near %.0fm far %.0fkm",
	     threeD->view.nearPlane, threeD->view.farPlane/1000);
    glRasterPos2i (8, y);  y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);
  }

  /* orientation */
      
  sprintf (buf, "Dir %.1f, Pitch %.1f Roll %.1f",
	   180*view->yaw/M_PI,
	   180*view->pitch  /M_PI,
	   180*view->roll /M_PI);
  glRasterPos2i (8, y); y += line_sep;
  glmCharstr (threeD->glmw, GL_FONT_R, buf);

  /* position in the world */
  {
    double gdc[3];
    char   *units = "m";
    double scale = 1.0;
    int    lng = 'E', lat = 'N';

    TsGCCtoGDC (view->eye[0], view->eye[1], view->eye[2], gdc);
    if (gdc[0] < 0) { lng = 'W'; gdc[0] = -gdc[0]; }
    if (gdc[1] < 0) { lat = 'S'; gdc[1] = -gdc[1]; }
    if (gdc[2] > 9000 ) { scale = 1000.0; units = "km"; }

    sprintf (buf, "%.3f%c %.3f%c %.0f%s (%.0f%s)",
	     gdc[0], lng, gdc[1], lat, gdc[2] / scale, units,
	     view->altitude / scale, units );
    glRasterPos2i (8, y); y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);
  }

  /* currently playing path normalised time index */

  if ( path_playing ) {
    sprintf (buf, "Time: %g", PathGetTimeIndex(threeD));
    glRasterPos2i (8, y); y += line_sep;
    glmCharstr (threeD->glmw, GL_FONT_R, buf);
  }

  /* list all currently loaded datasets */

  y += line_sep;
  for ( i = list_first( threeD->geoList );
	      ! list_done( i );
	      i = list_next( i ) ) {

      /* get the next GeoElement in this tvBase's list */

      GeoElement *geoelem = (GeoElement *) list_value( i );
      if ( geoelem == NULL || geoelem->geo == NULL )
	      continue;

      sprintf (buf, "%s", TsGeoName( geoelem->geo ) );
      glRasterPos2i (8, y); y += line_sep;
      glmCharstr (threeD->glmw, GL_FONT_R, buf);
  }

  /* display trace message */

  if (TSMTRACEP (OGL_MASK|2)) {
    sprintf (frame_desc, "tvDrawHeadsUp[%d]: Exit", threeD->frame);
    gltTraceState (OGL_MASK|2, frame_desc);
  }
}

static void
tvBaseSRILogo( tvBase * threeD )
{
  static GLuint logo_tex = 0;

  if (! logo_tex) {
    gltGenTextures (1, &logo_tex);
    TextureDefineBuffer (logo_tex, TRUE, FALSE,
			 sri_logo_width, sri_logo_height, 4,
			 sri_logo_bitmap);
  }

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable (GL_TEXTURE_2D);
  gltBindTexture (GL_TEXTURE_2D, logo_tex);
  glColor3f (1.0, 1.0, 1.0);

  glBegin (GL_QUADS); {
    float x1 = threeD->width - 5;
    float x0 = x1 - sri_logo_width;
    float y1 = 5;
    float y0 = y1 + sri_logo_height;
    glTexCoord2i (0, 0); glVertex2f (x0, y0);
    glTexCoord2i (1, 0); glVertex2f (x1, y0);
    glTexCoord2i (1, 1); glVertex2f (x1, y1);
    glTexCoord2i (0, 1); glVertex2f (x0, y1);
  }; glEnd ();
}


/*
 * Synopsis: void tvBaseTiming()
 *	
 *
 * Description: Calculates elapsed time and logs it
 *
 *
 * Externals: NONE
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *
 *		Stephen Lau
 * 
 * Date: 1994
 *
 */
static void
tvBaseTiming(int             numDrawnPolys,
	     int             newTextures,
	     int             totalTextures,
	     PRIntervalTime  t_start)
{
  /*static char const funcName[] = "tvBaseTiming";*/
  PRIntervalTime t_end;
  double refresh, elapsed;

  t_end = PR_IntervalNow();
  elapsed = tsmElapsedTime(t_start, t_end);
  refresh = elapsed * 1000.0;
  if ( TsIsLogging(TV_RENDER_LOG) )
    TsLogRenderData( t_start, t_end, refresh, numDrawnPolys,
		     totalTextures, newTextures );
}

/*
 * Synopsis: void tvBaseDrawSphere(void)
 *	
 * Description: Generate and draws the earth sphere.  Used as part of the
 *              background of the tile draing window.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:Lee Iverson
 * 
 * Date: Nov 2000
 *
 */

#include "earth_img.c"

#define ICO_X (.525731112119133606)
#define ICO_Z (.850650808352039932)

static GLfloat ico_data[12][3] = {
  {-ICO_X,0,ICO_Z}, {ICO_X,0,ICO_Z},  {-ICO_X,0,-ICO_Z}, {ICO_X,0,-ICO_Z},
  {0,ICO_Z,ICO_X},  {0,ICO_Z,-ICO_X}, {0,-ICO_Z,ICO_X},  {0,-ICO_Z,-ICO_X},
  {ICO_Z,ICO_X,0},  {-ICO_Z,ICO_X,0}, {ICO_Z,-ICO_X,0},  {-ICO_Z,-ICO_X,0}
};

static GLint ico_idx[20][3] = {
  {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
  {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
  {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
  {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};

static void
ico_drawtriangle (float const *v1, float const *v2, float const *v3)
{
  float r = 6356752.314;
  float const *v[3];
  double gdc[3][3], gcc[3][3];
  float d01, d02;
  int i;

  v[0] = v1; v[1] = v2; v[2] = v3;
  for (i = 0; i < 3; ++i) {
    TsGCCtoGDC (r*v[i][0], r*v[i][1], r*v[i][2], gdc[i]);
    TsGDCtoGCC (gdc[i][0], gdc[i][1], 0.0, gcc[i]);
  }

  for (i = 0; i < 3; ++i) {
    if (gdc[i][1] > 89 || gdc[i][1] < -89) {
      gdc[i][0] = gdc[(i+1)%3][0];
    }
  }

  d01 = fabs (gdc[0][0] - gdc[1][0]);
  d02 = fabs (gdc[0][0] - gdc[2][0]);

  if (d01 > 170) {
    if (d02 > 170) {
      /* gdc[0] is the problem */
      if (gdc[0][0] > gdc[1][0])
	gdc[0][0] -= 360;
      else
	gdc[0][0] += 360;
    } else {
      /* gdc[1] is the problem */
      if (gdc[1][0] > gdc[0][0])
	gdc[1][0] -= 360;
      else
	gdc[1][0] += 360;
    }
  } else {
    if (d02 > 170) {
      /* gdc[2] is the problem */
      if (gdc[2][0] > gdc[1][0])
	gdc[2][0] -= 360;
      else
	gdc[2][0] += 360;
    }
  }

  for (i = 0; i < 3; ++i) {
    if (gdc[i][1] > 89 || gdc[i][1] < -89) {
      gdc[i][0] = (gdc[(i+1)%3][0] + gdc[(i+2)%3][0])/2;
    }
  }

  for (i = 2; i >= 0; --i) {
    glTexCoord2f ((gdc[i][0]+180)/360.0, (gdc[i][1]+90)/180.0);
    glNormal3fv (v[i]);
    glVertex3dv (gcc[i]);
  }
}

static void
ico_subdivide (float const *v1, float const *v2, float const *v3, int l)
{
  v3f v12, v23, v31;

  if (l == 0) {
    ico_drawtriangle (v1, v2, v3);
  } else {
    add_v3f ((v3f*)v1, (v3f*)v2, &v12); normalize_v3f (&v12);
    add_v3f ((v3f*)v2, (v3f*)v3, &v23); normalize_v3f (&v23);
    add_v3f ((v3f*)v3, (v3f*)v1, &v31); normalize_v3f (&v31);

    --l;
    ico_subdivide (v1, V(v12), V(v31), l);
    ico_subdivide (v2, V(v23), V(v12), l);
    ico_subdivide (v3, V(v31), V(v23), l);
    ico_subdivide (V(v12), V(v23), V(v31), l);
  }
}

static void
tvBaseDrawSphere (void)
{
  static GLint earth;
  static GLuint earth_tex;

  if (! earth) {
    int i;

    gltGenTextures (1, &earth_tex);
    TextureDefineBuffer (earth_tex, TRUE, TRUE,
			 earth_cols, earth_rows, 3, earth_data);

    earth = glGenLists (1);
    glNewList (earth, GL_COMPILE_AND_EXECUTE); {
      gltBindTexture (GL_TEXTURE_2D, earth_tex);

      glBegin (GL_TRIANGLES); {
	for (i = 0; i < 20; ++i) {
	  ico_subdivide (ico_data[ico_idx[i][0]],
			 ico_data[ico_idx[i][1]],
			 ico_data[ico_idx[i][2]],
			 3);
	}
      }; glEnd ();
    }; glEndList ();
  } else {
    glCallList (earth);
  }
}

/*
 * Synopsis:
 *	static void tvBaseDrawBackGround(tvBase *)
 *
 * Description: Draw a default ground for areas not covered by the dataset.
 *     Draw a sky and place a scanned map as a background.
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *      April 6, 1993
 *
 */
/*static*/ void
tvBaseDrawBackGround(tvBase * threeD, VisibleDrawList *drawList,
		     int *numPolys, int stereo, int stereoVisual, int eye,
		     PRIntervalTime t_start, PRIntervalTime t_prev)
{
  static char const funcName[] = "tvBaseDrawBackGround";
  tvViewpoint *view = &threeD->view;

  /* DRAW THE SKY BASED ON TIME OF DAY and elevation*/
  if (!stereo || stereoVisual || eye == 0) {
    float v[3];

    glPushAttrib (GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT); {
      unsigned short len = drawList->n;
      unsigned short i;
      int no_stars = 0;

      /* fade the sky color to black from 6 km to 24 km */
      float altMult = 1.1 - view->altitude / 400000;
      altMult = min(1.0, altMult);
      altMult = max(0, altMult);

      glDisable (GL_TEXTURE_2D);
      glDisable (GL_DEPTH_TEST);
      glDisable (GL_DITHER);
      glClearDepth (1);

      if (threeD->timeOfDay <= 7 || threeD->timeOfDay >= 17) {
	gltCheckErrors (FILE_LOC);
	
	/* CLEAR THE WINDOW */
	if (threeD->timeOfDay <= 5 || threeD->timeOfDay >= 19) {
	  glClearColor (0.0, 0.0, 0.0, 0.0);
	  glClear (GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	} else {
	  gltClearColor4ub(0x4F*altMult, 0x65*altMult, 0x87*altMult, 0xFF);
	  glClear (GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	  glPushMatrix (); {
	    glLoadIdentity ();
	    glMatrixMode (GL_PROJECTION);
	    glPushMatrix (); {
	      glLoadIdentity ();
	      glOrtho (0, 1, 0, 1, -1, 1);
	      glBegin (GL_QUADS); {
		glColor4ub (0xFE * altMult, 0x5E * altMult, 0x00, 0xFF);
		v[0] = 0; v[1] = 0; v[2] = 0;
		glVertex3fv (v);
		v[0] = 1;
		glVertex3fv (v);

		glColor4ub (0x00, 0x37 * altMult, 0x78 * altMult, 0xFF);
		v[1] = 1;
		glVertex3fv (v);
		v[0] = 0;
		glVertex3fv (v);
	      }; glEnd();
	    }; glPopMatrix ();
	    glMatrixMode (GL_MODELVIEW);
	  }; glPopMatrix();
	}
	gltCheckErrors (FILE_LOC);

      } else if ( threeD->fog_params[0] > 0.0 ) {
	/* Set the clear colour to the fog colour */
	gltClearColor4ub( (int) (threeD->fog_params[1] * 255),
			  (int) (threeD->fog_params[2] * 255),
			  (int) (threeD->fog_params[3] * 255), 255 );
	glClear (GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	++no_stars;
      } else {
	/* Set the clear colour to match the sky texture map */
	gltClearColor4ub (0x28 * altMult, 0x50 * altMult, 0xFF * altMult, 255);
	glClear (GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
      }

      /* Work out if this viewer has any sky datasets loaded */
      /* if it does, then we will not draw our hokey stars.  */

      {
	list_iter_t i = list_first (threeD->geoList);
	for ( ; ! list_done (i); i = list_next (i) ) {
	  TsGeoPyramid *geo = list_value( i );
	  if ( TsGeoIsSky( geo ) ) {
	    no_stars++;
	    break;
	  }
	}
      }

      /* draw any sky datasets */

      for (i = 0; i < len && !threeD->selecting; ++i) {
	VisibleDrawGeoList *geo_list = &drawList->geo_list[i];
	TsGeoPyramid *geo = geo_list->geo;
	
	if ( ! TsGeoIsSky( geo ) ) continue;

	TSMTRACE ((THREED_MASK|1, funcName, "\"%s\" (SKY)", TsGeoName (geo)));
	glPushMatrix(); {

	  /* need to scale the dataset down so that it sits at roughly */
	  /* 10,000 m above the origin of the world, **not** 10,000 m  */
	  /* about the ellipsoid of the earth! Here we assume that the */
	  /* dataset is in the WGS84 ellipsoid and scale down accrdgly */

	  float alt = TsGeoOriginAltitude( geo );
	  float scale = 10000.0 / ( 6378137.0 + alt );

	  glScalef( scale, scale, scale );

	  tvBaseGLViewpointLVCS( view, geo );

	  /* no picking of sky datasets - does this make sense? */
	  /* if (threeD->selecting) glLoadName (i); */

	  /* Draw the visible tiles */

	  tvBaseDrawGeoTiles (threeD, &drawList->geo_list[i],
			      numPolys, t_start, t_prev);

	  if (TIMING)
	    _3DReportElapsed (t_start, t_prev, "Sky Tiles");

	}; glPopMatrix();

	/* Draw the features for this dataset (might be a different LVCS) */
	
	if ( threeD->showModels ) {
	  FeatureDisplay( threeD, geo );
	  if (TIMING)
	    _3DReportElapsed (t_start, t_prev, "Sky Features");
	}
	
      }

      /* Draw stars */
      if (! no_stars && altMult < 0.6) {
	int i, r1 = MAX_STARS / 3, r2 = MAX_STARS * 2 / 3;
	float mult = 1.0-altMult;
	TSMTRACE ((THREED_MASK|1, funcName, "Draw stars: %02x,%02x,%02x",
		   (int)(0xAF*mult), (int)(0xFD*mult), (int)(0xFF*mult)));
	glColor4ub((int)(0xAF*mult), (int)(0xFD*mult), (int)(0xFF*mult), 0xFF);
	glPointSize (1);
	glBegin(GL_POINTS); {
	  for (i = 0; i < r1; i++) 
	    glVertex3fv(threeD->stars[i]);
	}; glEnd();
	glPointSize (2);
	glBegin(GL_POINTS); {
	  for (i = r1; i < r2; i++)
	    glVertex3fv(threeD->stars[i]);
	}; glEnd();
	glPointSize (3);
	glBegin(GL_POINTS); {
	  for (i = r2; i < MAX_STARS; i++)
	    glVertex3fv(threeD->stars[i]);
	}; glEnd();
      }
    }; glPopAttrib ();
  }
  gltCheckErrors (FILE_LOC);

  /* TURN ON THE FOG */
  tvBaseDrawFog (threeD);
  gltCheckErrors (FILE_LOC);

  gltCheckErrors (funcName);
}


/*
 * Synopsis: void tvBaseDrawFrustum()
 *	
 *
 * Description: draws the viewing frustum from the frozen view
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1994
 *
 */
void
tvBaseDrawFrustum (tvViewpoint *view, PRBool wireframe)
{
  static char * funcName = "tvBaseDrawFrustum";
  float pts[4][3];
  float eye[3];
  int order[] = {0, 1, 3, 2};
  int v = 0;
  int x,y;

  for (x = -1; x <= 1; x += 2) {
    for (y = -1; y <= 1; y += 2) {
      pts[order[v]][0] = -(x * tan((M_PI / 180.0) * 0.5 * (float) view->fov)
			   * view->aspect * view->farPlane);
      pts[order[v]][1] =  (y * tan((M_PI / 180.0) * 0.5 * (float) view->fov)
			   * view->farPlane);
      pts[order[v]][2] = view->farPlane;
      v++;
    }
  }

  glPushMatrix(); {
    glTranslatef(view->eye[0], view->eye[1], view->eye[2]);

    /* We need to initialize the viewpoint */
    eye[0] = 0;
    eye[1] = 0;
    eye[2] = 0;
    glRotatef(view->reference_plane[0],
	      view->reference_plane[1],
	      view->reference_plane[2],
	      view->reference_plane[3]);
    glRotatef(                  -90.0, 1, 0, 0);
    glRotatef(-view->yaw  /M_PI*180.0, 0, 1, 0);
    glRotatef( view->pitch/M_PI*180.0, 1, 0, 0);
    glRotatef( view->roll /M_PI*180.0, 0, 0, 1);
    glLineWidth((GLfloat)(4));
    for (v = 0; v < 4; v++) {
      if (wireframe)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
      glEnable (GL_BLEND);
      glColor4ub(0x00, 0x00, 0xFF, 0x40);
      glBegin(GL_TRIANGLES); {
	glVertex3fv(eye);
	glVertex3fv(pts[v]);
	glVertex3fv(pts[(v+1)%4]);
      }; glEnd();
      glDisable (GL_BLEND);
      if (wireframe) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }
      glColor4ub(0x00, 0xFF, 0xFF, 0xFF);
      glBegin(GL_LINES); {
	glVertex3fv(eye);
	glVertex3fv(pts[v]);
      }; glEnd();
    }
  }; glPopMatrix();

  gltCheckErrors (funcName);
}


/*
 * Synopsis: void tvBaseTileColor(int idx)
 *	
 *
 * Description: Sets the current OpenGL color to that defined for the tile.
 *
 * Returns: NOTHING
 *
 * Author:
 *		Lee Iverson
 * 
 * Date: Dec , 1996
 *
 */

#define colorStruct(r,g,b) { r, g, b, 0xff }

static GLubyte tileColors[][4] =
{
    colorStruct( 64, 64,255),
    colorStruct( 64,255, 64),
    colorStruct( 64,255,255),
    colorStruct(255, 64, 64),
    colorStruct(255, 64,255),
    colorStruct(255,255, 64),
    colorStruct(255,255,255),
    colorStruct( 64, 64,128),
    colorStruct( 64,128, 64),
    colorStruct( 64,128,128),
    colorStruct(128, 64, 64),
    colorStruct(128, 64,128),
    colorStruct(128,128, 64),
    colorStruct(128,128,128)
};

void
tvBaseTileColor (int idx)
{
  glColor4ubv (tileColors[idx]);
}

GLubyte *
tvBaseGetTileColor (int idx)
{
  return tileColors[idx];
}

static void 
tvBaseDrawBoundingBoxes(tvBase *threeD, int id)
{
  static char const * funcName = "tvBaseDrawBoundingBoxes";
  tvViewpoint const *view = &threeD->view;
  list_iter_t i;

  glPushAttrib (GL_CURRENT_BIT|GL_ENABLE_BIT|GL_LINE_BIT); {
    glDisable (GL_DITHER);
    glDisable (GL_LIGHTING);
    glDisable (GL_TEXTURE_2D);
    if (0) {
      glDisable (GL_BLEND);
      glDisable (GL_LINE_SMOOTH);
      glDisable (GL_POLYGON_SMOOTH);
    }
    glLineWidth (1);

    for (i = list_first (threeD->tileLinks);
	 ! list_done (i);
	 i = list_next (i)) {
      tvBase *other = list_value (i);
      tvBaseLock (other); {

	int currDrawList = other->currDrawList;
	VisibleDrawList * drawList = &other->drawList[currDrawList];

	unsigned short len = drawList->ntiles;
      
	/* If there are no tiles, then punt. */
	if (len) {
	  int j;
	  for (j = 0; j < drawList->n; ++j) {
	    VisibleDrawGeoList *geo_list = &drawList->geo_list[j];
	    TsGeoPyramid *geo = geo_list->geo;
	    double const *geoOrigin = TsGeoOrigin (geo);
	    float const *geoOrient = TsGeoOrientation (geo);
	    VisibleDraw *vis = geo_list->vis, *end = vis + geo_list->n;
      
	    TSMTRACE ((THREED_MASK|1, funcName, "\"%s\"", TsGeoName (geo)));
	    glPushMatrix(); {

	      tvBaseGLViewpointLVCS (view, geo);

	      for ( ; vis < end; ++vis) {
		char str[128];
		TsTile const * oiTile = vis->tile;
		TsTileId const * tile = &oiTile->id;

		tvBaseTileColor (oiTile->id.res);

		{
		  float const *minv = oiTile->bbox.min;
		  float const *maxv = oiTile->bbox.max;
		  float const *ctrv = oiTile->bbox.center;

		  glBegin(GL_LINE_LOOP); {
		    glVertex3f (minv[0], minv[1], minv[2]);
		    glVertex3f (maxv[0], minv[1], minv[2]);
		    glVertex3f (maxv[0], maxv[1], minv[2]);
		    glVertex3f (minv[0], maxv[1], minv[2]);
		    glVertex3f (minv[0], maxv[1], maxv[2]);
		    glVertex3f (maxv[0], maxv[1], maxv[2]);
		    glVertex3f (maxv[0], minv[1], maxv[2]);
		    glVertex3f (minv[0], minv[1], maxv[2]);
		  }; glEnd ();
		  glBegin(GL_LINES); {
		    glVertex3f (minv[0], minv[1], minv[2]);
		    glVertex3f (minv[0], maxv[1], minv[2]);
		    glVertex3f (minv[0], minv[1], maxv[2]);
		    glVertex3f (minv[0], maxv[1], maxv[2]);
		    glVertex3f (maxv[0], minv[1], minv[2]);
		    glVertex3f (maxv[0], minv[1], maxv[2]);
		    glVertex3f (maxv[0], maxv[1], minv[2]);
		    glVertex3f (maxv[0], maxv[1], maxv[2]);
		  }; glEnd ();

		  if (id) {
		    glRasterPos3f(ctrv[0], ctrv[1], maxv[2]);
		    glColor4ub(0xFF, 0xFF, 0x00, 0xFF);
		    sprintf (str, "[%d %d %d] %d",
			     tile->x, tile->y, tile->res,
			     (use_subtexture
			      ? TexRefID (vis->tex.ref)
			      : TextureID (vis->tex.node)));
		    glmCharstr (threeD->glmw, GL_FONT_R, str);
		  }
		}
	      }
	    }; glPopMatrix ();
	  }
	}
      }; tvBaseUnlock (other);
    }
  }; glPopAttrib ();
}


static void 
tvBaseOutlineDatasets(tvBase *threeD)
{
  static char const *funcName = "tvBaseOutlineDatasets";
  tvViewpoint const        *view = &threeD->view;
  list_iter_t       i;

  glPushAttrib (GL_CURRENT_BIT|GL_ENABLE_BIT|GL_LINE_BIT); {
    glDisable (GL_DITHER);
    glDisable (GL_LIGHTING);
    glDisable (GL_TEXTURE_2D);
    if (0) {
      glDisable (GL_BLEND);
      glDisable (GL_LINE_SMOOTH);
      glDisable (GL_POLYGON_SMOOTH);
    }
    glLineWidth (1);

    /* go through the list of all GeoSets for this tvBase  */
    for ( i = list_first( threeD->geoList );
	  ! list_done( i );
	  i = list_next( i ) ) {

      /* get the next GeoElement in this tvBase's list */

      GeoElement *geoelem = (GeoElement *) list_value( i );
      if ( geoelem == NULL || geoelem->geo == NULL )
	continue;

      /* don't outline the earth - it's just distracting. We define */
      /* "earth" as a dataset that goes from -180 -> +180 longitude */
      
      if ( TsGeoGetInfo( geoelem->geo )->wholeEarth ) {
	continue;
      }

      /* get the top tile of the geoPyramid and draw it */
      glPushMatrix(); {
	TsTile *topTile = TsGeoTop(geoelem->geo);
	double const *geoOrigin = TsGeoOrigin (geoelem->geo);
	float const *geoOrient = TsGeoOrientation (geoelem->geo);
	float const *minv = topTile->bbox.min;
	float const *maxv = topTile->bbox.max;
	float const *ctrv = topTile->bbox.center;
      
	TSMTRACE ((THREED_MASK|1, funcName, "\"%s\"", TsGeoName (geoelem->geo)));
	tvBaseGLViewpointLVCS (view, geoelem->geo);

	glColor3ub(0xFF, 0x00, 0x00);
	/* draw bbox in wireframe */
	glBegin(GL_LINE_LOOP); {
	  glVertex3f (minv[0], minv[1], minv[2]);
	  glVertex3f (maxv[0], minv[1], minv[2]);
	  glVertex3f (maxv[0], maxv[1], minv[2]);
	  glVertex3f (minv[0], maxv[1], minv[2]);
	  glVertex3f (minv[0], maxv[1], maxv[2]);
	  glVertex3f (maxv[0], maxv[1], maxv[2]);
	  glVertex3f (maxv[0], minv[1], maxv[2]);
	  glVertex3f (minv[0], minv[1], maxv[2]);
	}; glEnd ();
	glBegin(GL_LINES); {
	  glVertex3f (minv[0], minv[1], minv[2]);
	  glVertex3f (minv[0], maxv[1], minv[2]);
	  glVertex3f (minv[0], minv[1], maxv[2]);
	  glVertex3f (minv[0], maxv[1], maxv[2]);
	  glVertex3f (maxv[0], minv[1], minv[2]);
	  glVertex3f (maxv[0], minv[1], maxv[2]);
	  glVertex3f (maxv[0], maxv[1], minv[2]);
	  glVertex3f (maxv[0], maxv[1], maxv[2]);
	}; glEnd ();

	/* draw name of dataset */
	glRasterPos3f(ctrv[0], ctrv[1], ctrv[2]);
	glColor3f(1.0, 1.0, 1.0);
	glmCharstr (threeD->glmw, GL_FONT_R, TsGeoName(geoelem->geo));
      }; glPopMatrix();
    }
  }; glPopAttrib ();
}


static void
tvBasePrepareGeoTiles (tvBase * threeD,
		       VisibleDrawList *list,
		       VisibleDrawGeoList *geo_list,
		       int *          numPolys,
		       int *          newTextures,
		       int *          totalTextures);

/*
 * Synopsis: void tvBasePrepareTiles()
 *	
 * Description: Prepares the tile for drawing.
 *
 * Externals: None.
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: April, 1999
 */

static void
tvBasePrepareTiles (tvBase * threeD,
		    VisibleDrawList *drawList,
		    int *            numPolys,
		    int *            newTextures,
		    int *            totalTextures)
{
  /*static const char funcName[] = "tvBasePrepareTiles";*/
  unsigned short len = drawList->n;
  unsigned short i;

  /* If there are no tiles, then punt. */
  if (! drawList->ntiles) return;

  RPRTR (("Prp: list %p\n", drawList));
  for (i = 0; i < len; ++i) {
    /* Draw the visible tiles */
    tvBasePrepareGeoTiles (threeD,
			   drawList,
			   &drawList->geo_list[i],
			   numPolys, 
			   newTextures,
			   totalTextures);
  }
}

/*
 * Synopsis: void tvBaseDrawTiles()
 *	
 * Description: Draws the tiles and features for all datasets in geoList.
 *
 * Externals: None.
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: April, 1999
 */
static void
tvBaseDrawTiles (tvBase * threeD,
		 VisibleDrawList * drawList,
		 int *          numPolys,
		 PRIntervalTime   t_start,
		 PRIntervalTime   t_prev)
{
  static const char funcName[] = "tvBaseDrawTiles";
  tvViewpoint const *view = &threeD->view;
  unsigned short len = drawList->n;
  unsigned short i;

  /* If there are no tiles, then punt. */
  if (! drawList->ntiles) return;

  glDepthMask (GL_TRUE);

  RPRTR (("Drw: list %p\n", drawList));
  for (i = 0; i < len; ++i) {
    VisibleDrawGeoList *geo_list = &drawList->geo_list[i];
    TsGeoPyramid *geo = geo_list->geo;

    /* is this a sky dataset? If so, don't render it as it should */
    /* have already been rendered in the background of the scene. */

    if ( TsGeoIsSky( geo ) ) continue;

    /* implement the Z-buffer hack for dataset priorities. This     */
    /* clears the Z-buffer between rendering each dataset's tiles   */
    /* in order to prevent flimmering in regions where datasets     */
    /* overlap. This technique can introduce artifacts when viewed  */
    /* from certain angles, but it is a simple short-term solution. */
    /* Currently we just clear the Z-buffer before each dataset     */
    /* (except the first); however, a more efficient approach would */
    /* be to only clear the Z-buffer if the next dataset intersects */
    /* with the current one in screen space.                        */
	  /* A more recent optimization that was suggested by Pete Tinker */
	  /* is to use glDepthFunc(GL_ALWAYS), i.e. effectively turn off  */
	  /* the depth buffer. This can cause serious artifacts in 3D     */
	  /* mode at ground level, so we only use it in 2D mode.          */
    /* Further experimentation found that this also caused visible  */
    /* artifacts in any 3D models overlaid on the terrain so the    */
    /* previous glClear() solution was re-instated.                 */

    if ( threeD->useZBufferHack && i != 0 )
        glClear (GL_DEPTH_BUFFER_BIT);

    TSMTRACE ((THREED_MASK|1, funcName, "\"%s\"", TsGeoName (geo)));
    glPushMatrix(); {

      tvBaseGLViewpointLVCS (view, geo);

      if (threeD->selecting)
	glLoadName (i);

      /* Draw the visible tiles */

      tvBaseDrawGeoTiles (threeD, &drawList->geo_list[i],
			  numPolys, t_start, t_prev);

      if (TIMING)
	_3DReportElapsed (t_start, t_prev, "Tiles");

    }; glPopMatrix();

    /* Draw the features for this dataset (might be a difference LVCS) */

    if ( threeD->showModels ) {
      FeatureDisplay( threeD, geo );
      if (TIMING)
	_3DReportElapsed (t_start, t_prev, "Features");
    }
    
  }

  /* Draw bound box of each tile */

  if (threeD->showTiles)
    tvBaseDrawBoundingBoxes (threeD, threeD->freezeTiles);

  if (threeD->showDatasets)
    tvBaseOutlineDatasets(threeD);

  /* Draw the global features */
  if ( threeD->showModels )
    FeatureDisplay( threeD, NULL );

  /* do the post draw callback if specified */
  if ( threeD->post_draw_callback )
    (*threeD->post_draw_callback)( threeD, threeD->post_draw_closure );
}

/*
 * Synopsis:
 *   tvPostDrawCallback tvSetPostDrawCallback( tvBase *threeD,
 *                                             tvPostDrawCallback callback,
 *                                             void *closure )
 *
 * Doc++ Description: *//**
 *   Registers a callback function that is called during the redraw
 *   process, after all of the tiles and models have been drawn. This
 *   lets users plug in there own rendering overlays to the TerraVision
 *   display. Your callback function will look something like:
 *
 *   <pre>
 *      void
 *      myDrawCallback( tvBase *threeD, void *closure )
 *      {
 *         ... do your work here ...
 *      }
 *   </pre>
 *
 * @param  threeD   The tvBase structure for the window you wish to add the
 *                  callback routine to.
 * @param  callback You custom callback function. Set this to NULL to 
 *                  deregister a previous callback.
 * @param  closure  A pointer that will be passed back to your callback
 *                  function (may be NULL).
 * 
 * @return the previous callback function (or NULL if none registered)
 *
 * @author Martin Reddy
 *
 */

tvPostDrawCallback
tvSetPostDrawCallback( tvBase *threeD, tvPostDrawCallback callback,
		       void *closure )
{
  const char         *funcName = "tvSetPostDrawCallback";
  tvPostDrawCallback prev;

  if ( ! threeD ) return NULL;

  tvBaseLock( threeD ); {
    prev = threeD->post_draw_callback;
    threeD->post_draw_callback = callback;
    threeD->post_draw_closure  = closure;
  }; tvBaseUnlock( threeD );

  return prev;
}

static void
tvBaseTexDecacheCB (void *id, void *closure)
{
  TsTile *tile = id;
  /* TsGeoPyramid *geo = closure; */
  NetLogTile (tile, TV_TILE_OUT_TEXCACHE);
}

typedef struct TexImageClosure {
  TsGeoPyramid *geo;
  int w, h;
  int mipmap;
  int nbytes;
} TexImageClosure;

static void
tvBaseTexImageCB (void const *tile_, void *closure)
{
  static char const funcName[] = "tvBaseTexImageCB";

  TsTile const *tile = tile_;
  TexImageClosure *texim = closure;
  int tileWidth  = texim->w;
  int tileHeight = texim->h;
  int nbytes = texim->nbytes;
  int mipmap = texim->mipmap;
  GLubyte const *tileData = tile->data;
  GLenum texComps, texFmt;

  if (nbytes <= 0 || nbytes > 4) {
    tsmError (funcName, "Can't handle %d bytes/pixel", nbytes);
    return;
  }

  texFmt   = texParams[nbytes].fmt;
  texComps = texParams[nbytes].comps;

  TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName, "%p [%d %d %d] glTexImage2D %dx%d",
	     tile, tile->id.x, tile->id.y, tile->id.res,
	     tileWidth, tileHeight));

  if (mipmap)
    TSMTRACE ((TEX_MASK|OGL_MASK|2, funcName, "mipmap 0 %d,%d",
	       tileWidth, tileHeight));

  glTexImage2D (GL_TEXTURE_2D, 0, texComps,
		tileWidth, tileHeight,
		0, texFmt, GL_UNSIGNED_BYTE,
		tileData);
  NetLogTile (tile, TV_TILE_IN_TEXCACHE);

  if ( use_texture_edge_clamp) {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TV_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TV_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		     mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  } else {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		     mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  }

  if (mipmap) {
    int level = 1;
    int w0 = tileWidth;
    int h0 = tileHeight;
    GLubyte const *im0 = tileData;
    int w1 = (w0 > 1 ? w0/2 : w0);
    int h1 = (h0 > 1 ? h0/2 : w0);
    GLubyte *im1 = (GLubyte *) alloca (w1*h1*nbytes);

    while (w0 > 1 || h0 > 1) {
      gltScaleImage (nbytes,
		     w0, h0, nbytes*w0, im0,
		     w1, h1, nbytes*w1, im1);
      TSMTRACE ((TEX_MASK|OGL_MASK|2, funcName, "mipmap %d %d,%d",
		 level, w1, h1));
      glTexImage2D (GL_TEXTURE_2D, level, texComps, w1, h1, 0,
		    texFmt, GL_UNSIGNED_BYTE, im1);

      ++level;
      w0 = w1; h0 = h1;
      w1 = (w0  > 1 ?  w0/2 : w0);
      h1 = (h0 > 1 ? h0/2 : h0);
      {
	GLubyte *tmp = im1;
	im1 = (im0 == tileData
	       ? (GLubyte *) alloca (w1*h1*nbytes)
	       : (GLubyte *) im0);
	im0 = tmp;
      }
    }
  }
}


static void
tvBasePrepareGeoTiles (tvBase * threeD,
		       VisibleDrawList *list,
		       VisibleDrawGeoList *geo_list,
		       int *          numPolys,
		       int *          newTextures,
		       int *          totalTextures)
{
  static char const funcName[] = "tvBasePrepareGeoTiles";

  int n_fast = 0;

  int mipmap = threeD->mipmapTextures;

  TsGeoPyramid * geo = geo_list->geo;
  GLuint tex_id = 0;
  TextureCache *textureCache = threeD->textureCache;

  TsGeoInfo const *info = TsGeoGetInfo (geo);
  TexImageClosure texim;

  /* If we've already prepared this list, then don't do it again. */
  if (geo_list->prepared) {
    *totalTextures += geo_list->n;
    return;
  }

  /* It is possible, that *no tiles* from a geopyramid have been prepared,
     in which case we can arrive here with no RenderPrimitive attached to a
     VisibleDraw struct.  In this case, we should just silently punt.

     Check for a single, unprepared tile and ignore if found. */
  if (0 && geo_list->n == 1 && ! geo_list->vis->prim)
    return;

  TSMTRINC ((THREED_MASK|OGL_MASK|1, 1));

  RPRTR (("Prp: geolist %p[%d]\n", geo_list, geo_list->n));

  /* Download the textures necessary. */
  if (threeD->showTextures) {

#ifdef HAVE_TEXSUBIMAGE
    if (use_subtexture) {
      VisibleDraw *vis, *vis_end;
      TextureImage *im = geo_list->texim;

      /* GET THE FIRST VISIBLE TILE FROM THE TILE LIST */
      vis = geo_list->vis;
      vis_end = vis + geo_list->n;

      for ( ; vis < vis_end && vis->prim; ++vis) {
	/* GET THE TILE FROM THE STRUCTURE */
	TsTile const * tile = vis->tile;

	  /* Ensure that texref is valid.
	     This should not be needed any longer */
	if (! vis->tex.ref) {
	  if (! im) im = GetTextureImage (threeD->tvGLX, geo);
	  vis->tex.ref = TexRefFromTile (im, tile);
	} else {
	  TexRefCheck (vis->tex.ref, tile);
	}

	/* Bind the texture ID only if necessary */
	{
	  GLuint new_id = TexRefID (vis->tex.ref);
	  if (! new_id) {
	    tex_id = TexRefBind (vis->tex.ref, tex_id);
	  } else {
	    ++n_fast;
	  }
	}
      }
    } else
#endif /* HAVE_TEXSUBIMAGE */
    {
      VisibleDraw *vis, *vis_end;

      int w, h, tileSize;

      texim.geo = geo;
      texim.w = w = info->tileNumPixels[0];
      texim.h = h = info->tileNumPixels[1];
      texim.nbytes = info->tileNumComps;
      texim.mipmap = mipmap;

      tileSize = texParams[texim.nbytes].size*w*h;

      /* GET THE FIRST VISIBLE TILE FROM THE TILE LIST */
      vis = geo_list->vis;
      vis_end = vis + geo_list->n;

      for ( ; vis < vis_end && vis->prim; ++vis) {
	/* GET THE TILE FROM THE STRUCTURE */
	TsTile const * tile = vis->tile;

	if (! vis->tex.node) {
	  vis->tex.node = TextureFind (textureCache, tile, 0, 0);
	  TextureRef (vis->tex.node);
	}

	/* IF IT'S VALID, THEN TRY AND BIND THE TEXTURE */
	if (TextureBind (vis->tex.node,
			 tileSize,
			 tvBaseTexImageCB, &texim,
			 tvBaseTexDecacheCB, geo))
	  ++n_fast;
      }
    }

    *newTextures += geo_list->n - n_fast;
    *totalTextures += geo_list->n;
  }

  TSMTRINC ((THREED_MASK|OGL_MASK|1, -1));

  geo_list->prepared = TRUE;
}

/*
 * Synopsis: void tvBaseDrawGeoTiles()
 *	
 * Description: Draws the visible tiles for a given TsGeoPyramid.
 *
 * Externals: None.
 *
 * Returns: void
 *
 * Author: Stephen Lau
 * 
 * Date: April 9, 1993
 */
static void
tvBaseDrawGeoTiles (tvBase * threeD,
		    VisibleDrawGeoList *geo_list,
		    int *          numPolys,
		    PRIntervalTime t_start,
		    PRIntervalTime t_prev)
{
  static char const funcName[] = "tvBaseDrawGeoTiles";
  VisibleDraw *vis, *vis_end;

  int ntiles = 0;
  int ncached = 0;

  int showWireframe = threeD->showWireframe;
  int mipmap = threeD->mipmapTextures;
  int white;
  int w, h, tileSize;

  TsGeoPyramid * geo = geo_list->geo;
  GLuint tex_id = 0;

  TsGeoInfo const *info = TsGeoGetInfo (geo);
  TexImageClosure texim;

#if DRAWTIMES
  int timed = (threeD->showTextures
	       && ! (threeD->selecting
		     || threeD->tileByTile 
		     || threeD->showTileDebug
		     || showWireframe));
  PRIntervalTime start;

  if (timed)
    start = PR_IntervalNow ();
#endif

  /* It is possible, that *no tiles* from a geopyramid have been prepared,
     in which case we can arrive here with no RenderPrimitive attached to a
     VisibleDraw struct.  In this case, we should just silently punt.

     Check for a single, unprepared tile and ignore if found. */
  if (geo_list->n == 1 && ! geo_list->vis->prim)
    return;

  texim.geo = geo;
  texim.w = w = info->tileNumPixels[0];
  texim.h = h = info->tileNumPixels[1];
  texim.nbytes = info->tileNumComps;
  texim.mipmap = mipmap;
  tileSize = texParams[texim.nbytes].size*w*h;

#ifdef GL_POLYGON_OFFSET_EXT
  if (threeD->tileByTile || threeD->showTileDebug) {
    glEnable (GL_POLYGON_OFFSET_EXT);
    glPolygonOffsetEXT (1e-6, 1e-3);
  } else {
    glDisable (GL_POLYGON_OFFSET_EXT);
  }
#endif

  if (threeD->selecting || !threeD->backfaceCull) {
    glDisable(GL_CULL_FACE);
  } else {

    /* set glCullFace based on -twosided, -frombelow switches */

    switch ( TsGeoVisibleSides( geo ) ) {
    case 1:
      glCullFace(GL_FRONT);
      glEnable (GL_CULL_FACE);
      break;
    case 2:
      glDisable (GL_CULL_FACE);
      break;
    default:
      glCullFace(GL_BACK);
      glEnable (GL_CULL_FACE);
    }
  }

  if (threeD->showTextures)
    glEnable (GL_TEXTURE_2D);
  else
    glDisable (GL_TEXTURE_2D);

  glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
  glColor3f (1.0, 1.0, 1.0);
  white = TRUE;

  TSMTRINC ((THREED_MASK|OGL_MASK|1, 1));

  /* GET THE FIRST VISIBLE TILE FROM THE TILE LIST */
  vis = geo_list->vis;
  vis_end = vis + geo_list->n;

  for ( ; vis < vis_end && vis->prim; ++vis) {
    /* GET THE TILE FROM THE STRUCTURE */
    TsTile const * tile = vis->tile;
    TsTileId const * id = &tile->id;
    int tex_cached = FALSE, rp_cached;

    /* COUNT NUMBER OF TILES. */
    ++ntiles;

    if (TSMTRACEP (VIS_MASK|1) && tile == threeD->nearTile) {
      double const *nearP = threeD->nearPoint;
      float near_lvcs[3];
      TsGeoGCCtoLVCS (geo, nearP[0], nearP[1], nearP[2], near_lvcs);
      glPushAttrib (GL_CURRENT_BIT); {
	glColor3f (1.0, 0.5, 0.0);
	glPointSize (6.0);
	glBegin (GL_POINTS); {
	  glVertex3f (near_lvcs[0], near_lvcs[1], near_lvcs[2]);
	}; glEnd ();
      }; glPopAttrib ();
    }

    /*
       * CHECK TO MAKE SURE THE TILE DOESN'T GO
       * OUTSIDE OF THE VALID AREA.
       *
       */
    if (threeD->showTextures) {
#ifdef HAVE_TEXSUBIMAGE
      if (use_subtexture) {
	/* Bind the texture ID only if necessary */
	GLuint new_id = TexRefID (vis->tex.ref);
	if (new_id != tex_id) {
	  glBindTexture (GL_TEXTURE_2D, new_id);
	  tex_id = new_id;
	}
      } else
#endif
      {
	/* IF IT'S VALID, THEN TRY AND BIND THE TEXTURE */
	TextureBind (vis->tex.node,
		     tileSize,
		     tvBaseTexImageCB, &texim,
		     tvBaseTexDecacheCB, geo);
      }

      if (! white) {
	glColor3f(1.0, 1.0, 1.0);
	white = TRUE;
      }
    } else {
      /* OTHERWISE DRAW AN UGLY BROWN POLYGON */
      glColor4ub(112, 81, 41, 255);
      white = FALSE;
    }

    RPRTR (("Dr: list %p[%d] tile %p[%d %d %d] prim %p->geometry = %p\n",
	    geo_list, vis-geo_list->vis, 
	    tile->cache, tile->id.x, tile->id.y, tile->id.res, 
	    vis->prim, vis->prim->geometry));
    rp_cached = tvBaseDrawPrimitive (vis, threeD->poppingStrategy,
				     threeD->selecting,
				     threeD->select_features_only);
    ncached += rp_cached;

    if (threeD->tileByTile || threeD->showTileDebug)
      tvBaseDebugTile (threeD, vis);

    if (TIMING)
      _3DReportElapsed (t_start, t_prev, 
			"-- %d: Tile %d,%d,%d T:%d R:%d", 
			ntiles, id->x, id->y, id->res,
			tex_cached, rp_cached);

    /* IF WE ARE DRAWING TILE BY TILE, FLUSH AND PAUSE */
    if (threeD->tileByTile) {
      gltCheckErrors (FILE_LOC);
      glFlush ();
      if (threeD->showTextures) tsmPause_ms (50);
    }
  }

  if (ntiles)
    TSMTRACE ((THREED_MASK|OGL_MASK|1, funcName,
	       "%.2f%% tiles cached. (%d/%d)", 
	       (100.0*ncached)/ntiles, ncached, ntiles));

  TSMTRINC ((THREED_MASK|OGL_MASK|1, -1));

  glDisable (GL_CULL_FACE);

#ifdef GL_POLYGON_OFFSET_EXT
  if (threeD->tileByTile || threeD->showTileDebug)
    glDisable (GL_POLYGON_OFFSET_EXT);
#endif

#if DRAWTIMES
  if (timed) {
    PRIntervalTime end;
    double secs;
    end = PR_IntervalNow ();
    secs = tsmElapsedTime (start, end);
    PR_Lock (drawTimes.lock); {
      int i = drawTimes.i, j;
      drawTimes.n_fast[i] = n_fast;
      drawTimes.n_slow[i] = ntiles - n_fast;
      drawTimes.secs[i] = secs;
      drawTimes.i = j = (i+1)%3;

      if (ntiles != 0 && drawTimes.n_fast[j]+drawTimes.n_slow[j] != 0) {
	float f1 = n_fast, s1 = ntiles - n_fast, t1 = secs;
	float f2 = drawTimes.n_fast[j], s2 = drawTimes.n_slow[j];
	float t2 = drawTimes.secs[j];
	float d = s1*f2 - s2*f1;
	if (d != 0) {
	  float s = ((s2+f2)*t1-(s1+f1)*t2)/d, f = (s1*t2-s2*t1)/d;
	  if (s > 0 && f > 0) {
	    float n = ++drawTimes.n;
	    TSMTRACE((0, funcName, "S[%d]=%.2fms F[%d]=%.2fms",
		      drawTimes.n, s*1e3, drawTimes.n, f*1e3));
	    drawTimes.sum_s += s; drawTimes.sum_s2 += s*s;
	    drawTimes.s = s = drawTimes.sum_s/n;
	    drawTimes.sum_f += f; drawTimes.sum_f2 += f*f;
	    drawTimes.f = f = drawTimes.sum_f/n;
	    TSMTRACE((0, funcName, "S = %.2fms F = %.2fms", s*1e3, f*1e3));
	  }
	}
      }
    }; PR_Unlock (drawTimes.lock);
  }
#endif
}


static void
tvBaseDebugTile (tvBase const *threeD, VisibleDraw *vis)
{
  /* GET THE TILES FROM THE STRUCTURE */
  TsTile const * oiTile = vis->tile;
  TsTileId const * tile = &oiTile->id;
  TileGeometry const *geometry = vis->prim->geometry;
  TsCoord const *coord = vis->prim->coords;

  glPushAttrib (GL_ENABLE_BIT|GL_CURRENT_BIT|GL_POLYGON_BIT|GL_TEXTURE_BIT); {
    glDisable (GL_TEXTURE_2D);
    if (tile->res % 2) {
      if ((tile->x+tile->y) % 2)
	glColor4ub (0x1f, 0x1f, 0xff, 0xff);
      else
	glColor4ub (0xff, 0x1f, 0xff, 0xff);
    } else {
      if ((tile->x+tile->y) % 2)
	glColor4ub (0x1f, 0xff, 0xff, 0xff);
      else
	glColor4ub (0x1f, 0xff, 0x1f, 0xff);
    }

    glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    tvBaseDrawPrimitive (vis, threeD->poppingStrategy, threeD->selecting,
			 threeD->select_features_only);

    /* Now, show normals. */
    glBegin (GL_LINES); {
      int i;
      float len = 0.25 * hypot (coord[0].vertex[0] - coord[1].vertex[0],
				coord[0].vertex[1] - coord[1].vertex[1]);
      for (i = 0; i < geometry->len_coords; ++i, ++coord) {
	glVertex3fv (coord->vertex);
	glVertex3f (coord->vertex[0] + len*coord->norm[0],
		    coord->vertex[1] + len*coord->norm[1],
		    coord->vertex[2] + len*coord->norm[2]);
      }
    }; glEnd ();
	
  }; glPopAttrib ();
}


/*
 * Whenever a new tile comes into any of the visible datasets, we want to redo
 * the visible list calculation.
 */
void
tvBaseCacheCB (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "tvBaseCacheCB";
  GeoElement *elem = closure;
  tvBase *threeD = elem->threeD;
  TsCacheTileCB const *cb = arg;
  TsTile const *tile = cb->tile;
  int imTilep = (cache == TsGeoCache (elem->geo)); 
  VISTR ((">> %p[%d %d %d] %s\n", tile->cache,
	  tile->id.x, tile->id.y, tile->id.res,
	  imTilep ? "OI" : "DEM"));
  TSMTRACE ((VIS_MASK|1, funcName, "update \"%s\" [%d %d %d] %s%s",
	     TsGeoName (elem->geo),
	     tile->id.x, tile->id.y, tile->id.res,
	     imTilep ? "OI" : "DEM",
	     TsCacheTileResident (tile) ? "" : " FAIL"));

  /* Either redo quad tree if this is a DEM tile, or just the VisibleList */
  if (imTilep || ! TsCacheTileResident (tile))
    tvBaseRedoVisibility (threeD, FALSE);
  else
    tvBaseRedoVisibleList (threeD, FALSE);
}

/*
 * Synopsis: void tvForallDatasets (tvBase *threeD,
 *                                  void (*f) (TsGeoPyramid *, void *),
 *                                  void *closure);
 *	
 * Doc++ Description: *//**
 *   Call the given function for each TsGeoPyramid in the dataset list.
 *
 * Externals: None.
 *
 * @param  threeD   handle to render thread
 * @param  f        callback function
 * @param  closure  opaque data passed to f
 *
 * @return   void
 *
 * @author   Lee Iverson.
 * 
 * @version  December 1999.
 *
 */
void
tvForallDatasets (tvBase const *threeD,
		  void (*f) (TsGeoPyramid *, void *),
		  void *closure)
{
  list_iter_t i;
  for ( i = list_first( threeD->geoList );
	! list_done( i );
	i = list_next (i)) {
    GeoElement *elem = (GeoElement *) list_value( i );
    (*f) (elem->geo, closure);
  }
}

/*
 * Synopsis: void tvFindDataset (tvBase *threeD,
 *                               int (*f) (TsGeoPyramid *, void *),
 *                               void *closure);
 *	
 * Doc++ Description: *//**
 *   Call the given function for each TsGeoPyramid in the dataset list.
 *   Returns the first dataset for which f returns TRUE.
 *
 * Externals: None.
 *
 * @param  threeD   handle to tvBase window
 * @param  f        test function (returns T/F)
 * @param  closure  opaque data passed to f
 *
 * @return   TsGeoPyramid *
 *
 * @author   Lee Iverson.
 * 
 * @version  March 1999.
 *
 */
TsGeoPyramid *
tvFindDataset (tvBase const *threeD,
	       int (*f) (TsGeoPyramid *, void *),
	       void *closure)
{
  list_iter_t i;
  for ( i = list_first( threeD->geoList );
	! list_done( i );
	i = list_next (i)) {
    GeoElement *elem = (GeoElement *) list_value( i );
    if ((*f) (elem->geo, closure))
      return elem->geo;
  }
  return NULL;
}

/*
 * Synopsis: void tvBaseAutoReorder( tvBase *threeD )
 *	
 * Doc++ Description: *//**
 *   Reorders all of the currently loaded terrain datasets
 *   so that the highest resolution sets appear on top.
 *
 * @param  threeD   handle to tvBase window
 *
 * @return   void
 *
 * @author   Martin Reddy
 * 
 * @version  21 September 2000.
 *
 */
static void
tvBaseAutoReorder( tvBase *threeD )
{
  list_t      *geoList = threeD->geoList;
  int          update = TRUE;

  while ( update ) {
    list_iter_t i, last_i = 0;
    float this_res = 0.0, last_res = 0.0;
    int pos = 0;

    update = FALSE;
    for ( i = list_first (geoList); ! list_done (i); i = list_next (i) ) {
      GeoElement *this_e = list_value( i );

      /* calculate the resolution for this dataset */
      if ( this_e && this_e->geo )
	this_res = TsGeoResolution ( this_e->geo );

      /* compare to the previous dataset */

      if ( last_i ) {
        if ( last_res < this_res ) {
	  list_delete (i);
	  list_insert_before (last_i, this_e);
	  i = last_i;
	  update = TRUE;
        }
      }

      /* prepare for another iteration */

      last_i   = i;
      last_res = this_res;
      pos++;
    }
  }

}

/*
 * Synopsis: void tvAddDataset(tvBase *, TsGeoPyramid *)
 *	
 *
 * Description: When we change GEO pyramids, we need to 
 *     reset the viewpoint and change the title of the window
 *
 *
 * Externals: 
 *
 * Doc++ Description: *//**
 *   Adds a geographical dataset to be incorporated into the rendering of 
 *   a scene.
 *
 * @param threeD  handle to rendering thread
 * @param geo         handle to new geo dataset to be added
 *
 * @return   void
 *
 * @author   Stephen Lau
 * 
 * @version  July, 1994
 *
 * @see      TsGeoCreate_OIDEM
 *
 */

void
tvAddDataset(tvBase * threeD, TsGeoPyramid *geo)
{
  static char const funcName[] = "tvAddDataset";
  list_t *geoList = threeD->geoList;
  int was_empty = FALSE;
  GeoElement *elem;

  TSMTRACE ((THREED_MASK|1, funcName, "geo %s", TsGeoName (geo)));

  elem = malloc (sizeof (*elem));
  elem->threeD = threeD;
  elem->geo = geo;
  elem->geoView = TsGeoCreateView (geo);
  elem->quad = 0;
  elem->texImage = (threeD->tvGLX 
		    ? GetTextureImage (threeD->tvGLX, geo)
		    : 0);

  tvBaseLock (threeD); {
    list_iter_t i;

    /* Look for the geo in list. */
    for (i = list_first (geoList); ! list_done (i); i = list_next (i)) {
      GeoElement *e = list_value (i);
      if (e->geo == geo) {
	free (elem);
	elem = 0;
	break;
      }
    }

    /* If we didn't find it, then add. */
    if (elem) {
      was_empty = list_empty (geoList);
      list_shift (threeD->geoList, elem);
      TsGeoAddCallback (geo, TILE_CACHE, tvBaseCacheCB, elem, 0);
      TsGeoAddCallback (geo, TILE_RESET, tvBaseCacheCB, elem, 0);
    }

    /* auto pop the highest res dataset to the top */

    if ( threeD->auto_pop_to_top )
      tvBaseAutoReorder( threeD );
    
  }; tvBaseUnlock (threeD);

  if (was_empty && threeD->width) {
    /* REINITIALIZE THE VIEWPOINT */
    tvBaseInitView (threeD, geo);
  }

  tvBaseRedoVisibility ( threeD, FALSE );

  /* signal that the dataset list has updated */

  tvBaseUpdate (threeD, TV_DATASETS);
}

static void
tvGeoElementDestroy (GeoElement *elem)
{
  TsGeoPyramid *geo = elem->geo;

  TsGeoViewDestroy (elem->geoView);

  /* remove the callback that we registered in ThreeDGeoAddDataSet */
  TsGeoRemoveCallback( geo, TILE_CACHE, tvBaseCacheCB, elem );

  /* clean the QuadTile visibility structure, if defined */
  if (elem->quad) QuadTreeDeref (elem->quad);

  /* free the memory for thie GeoElement and remove it from the list */
  tsmMemFree( elem );
}

/*
 * Synopsis: 
 *   TsGeoPyramid *tvGetNthDataset (tvBase const *threeD, int order)
 *
 * Doc++ Description: *//**
 *   This function will return the n'th dataset in the tvBase's
 *   list of datasets. Number begins at zero.
 *
 * @param  threeD  handle to rendering state
 * @param  int     number of dataset in the list, where 0 = first
 *
 * @return   The (TsGeoPyramid *), or NULL if not valid
 *
 * @author   Martin Reddy
 * 
 * @version  22 September, 2000
 *
 */

TsGeoPyramid *
tvGetNthDataset (tvBase const *threeD, int order)
{
  static char const funcName[] = "tvGetNthDataset";
  list_iter_t       i;
  int               j = 0;
  TsGeoPyramid      *geo = NULL;

  tvBaseLock (threeD); {

    /* go through all of the geos for this widget */

    for ( i = list_first( threeD->geoList ); ! list_done( i );
          i = list_next( i ), j++ ) {
      if ( j == order ) {
        GeoElement *elem = (GeoElement *) list_value( i );
        geo = elem->geo;
        break;
      }
    }

  }; tvBaseUnlock (threeD);

  return geo;
}

/*
 * Synopsis: 
 *   void tvRemoveDataset( tvBase *, TsGeoPyramid * )
 *
 * Doc++ Description: *//**
 *   This function will remove a reference to the specified GeoPyramid
 *   from this tvBase. It will also perform any necessary clean
 *   up operations as a result of removing this geoset from the widget.
 *
 * @param  threeD  handle to rendering state
 * @param  geo         handle to geo dataset to be removed
 *
 * @return   void
 *
 * @author   Martin Reddy
 * 
 * @version  25 June, 1999
 *
 */

void
tvRemoveDataset( tvBase *threeD, TsGeoPyramid *geo )
{
  static char const funcName[] = "tvRemoveDataset";
  list_iter_t i;
  int         removed = 0;

  TSMTRACE ((THREED_MASK|1, funcName, "geo %s", TsGeoName (geo)));


  PR_Lock (threeD->visible_lock); {
    int currWorkList = threeD->currWorkList;
    VisibleDrawList *workList = &threeD->drawList[currWorkList];

    /* clear the visible lists so that we don't have any */
    /* references lying around to tiles we've unloaded   */

    ClearWorkList (threeD, threeD->workingList0);
    ClearWorkList (threeD, threeD->workingList1);

    ClearDrawList (threeD, workList);
    ClearDrawList (threeD, &threeD->drawList[0]);
    ClearDrawList (threeD, &threeD->drawList[1]);
    ClearDrawList (threeD, &threeD->drawList[2]);
    threeD->redo_vislist = 1;

#ifdef HAVE_LIBGEO
    // remove from the libgeo .geo cache 
    PR_Lock (threeD->dotgeo_lock); {
#if 0
      if ( geoGetBoolean( threeD->geo, GEO_CACHE_ENABLED ) ) {
	char const *urls[2];
	int n = TsGeoURLs (geo, urls);
#if 0
	for (int i = 0; i < n; ++i)
	  geoCacheRemoveElement( threeD->geo, urls[i] );
#endif
      }
#endif
    };
    PR_Unlock (threeD->dotgeo_lock);
#endif

    /* now remove the dataset from the tvBase's memory */

    tvBaseLock (threeD); {

      /* go through all of the geos for this widget */

      for ( i = list_first( threeD->geoList ); ! list_done( i ); ) {
	GeoElement *elem = (GeoElement *) list_value( i );

	/* have we found a reference to this geo? is so, remove it */

	if ( elem && geo == elem->geo ) {
	  tvGeoElementDestroy (elem);
	  removed++;
	  i = list_delete( i );
	  break;
	} else
	  i = list_next( i );
				
      }
			
    }; tvBaseUnlock (threeD);

  }; PR_Unlock (threeD->visible_lock);
  
  if ( removed == 0 ) return;

  /* update the display to reflect the changes that we have made  */

  if ( list_count( threeD->geoList ) > 0 )
    tvBaseRedoVisibility (threeD, FALSE);

  /* signal that the dataset list has updated */

  tvBaseUpdate (threeD, TV_DATASETS);
}

/*
 * Synopsis: 
 *   int tvDatasetPriority( tvBase *, TsGeoPyramid * )
 *
 * Doc++ Description: *//**
 *   Return the priority index of a given dataset.  If the dataset is not in
 *   the current list, then -1 is returned.
 *
 * @param  threeD  handle to rendering state
 * @param  geo         handle to geo dataset
 *
 * @return   int
 *
 * @author   Lee Iverson
 * 
 * @version  March 2000
 *
 */

int
tvDatasetPriority (tvBase *threeD, TsGeoPyramid *geo)
{
  static char const funcName[] = "tvDatasetPriority";

  int count = 0;
  tvBaseLock (threeD); {
    list_iter_t i;
    list_t *l = threeD->geoList;
    
    for (i = list_first (l); ! list_done (i); i = list_next(i), ++count) {
      GeoElement *elem = (GeoElement *) list_value (i);
      if (elem->geo == geo)
	break;
    }
    if (list_done (i))
      count = -1;
  }; tvBaseUnlock (threeD);

  return count;
}

/*
 * Synopsis: 
 *   void tvDatasetSetPriority_nolock( tvBase *, TsGeoPyramid *geo, int prio )
 *
 * Doc++ Description: *//**
 *   Move the given dataset to the given priority in the list.
 *   Returns true if successful, otherwise false.
 *
 *   This private version performs no locking of the tvBase
 *   structure.
 *
 * @param  threeD  handle to rendering state
 * @param  geo         handle to geo dataset
 *
 * @return   int
 *
 * @author   Lee Iverson
 * 
 * @version  March 2000
 *
 */

static int
tvDatasetSetPriority_nolock (tvBase *threeD, TsGeoPyramid *geo, int prio)
{
  static char const funcName[] = "tvDatasetSetPriority_nolock";

  list_t *l = threeD->geoList;
  int len = list_count (l);
  GeoElement *elem = 0;
  list_iter_t i;
  int pos;

  TSMTRACE ((THREED_MASK|1, funcName, "[%d] %s", prio, TsGeoName (geo)));

  /* Normalize and check prio */
  if (prio < 0) prio = len+prio;
  if (prio < 0 || prio >= len) {
    TSMTRACE ((THREED_MASK|1, funcName, "[%d] invalid 0-%d", prio, len-1));
    return FALSE;
  }
  TSMTRACE ((THREED_MASK|1, funcName, "[%d] of %d %s",
	     prio, len, TsGeoName (geo)));

  /* First find it and delete from place. */
  for (pos = 0, i = list_first (l); ! list_done (i); i = list_next(i), ++pos) {
    elem = (GeoElement *) list_value (i);
    if (geo == elem->geo) {
      if (pos != prio) {
	TSMTRACE ((THREED_MASK|1, funcName, "[%d] delete from [%d]",
		   prio, pos));
	list_delete (i);
	break;
      }
    }
    elem = 0;
  }
  if (! elem) {
    TSMTRACE ((THREED_MASK|1, funcName, "geo %s not found", TsGeoName (geo)));
    return FALSE;
  }

  if (pos != prio) {
    /* Now insert it in the right place */
    for (i = list_first (l);
	 prio && ! list_done (i);
	 i = list_next (i), --prio);

    if (list_done (i)) {
      TSMTRACE ((THREED_MASK|1, funcName, "push on end"));
      list_push (l, elem);
    } else {
      GeoElement *here = list_value (i);
      TSMTRACE ((THREED_MASK|1, funcName, "place before %s",
		 TsGeoName (here->geo)));
      list_insert_before (i, elem);
    }
  }

  return TRUE;
}

/*
 * Synopsis: 
 *   void tvDatasetSetPriority( tvBase *, TsGeoPyramid *geo, int prio )
 *
 * Doc++ Description: *//**
 *   Move the given dataset to the given priority in the list.
 *   Returns true if successful, otherwise false.
 *
 *   This function locks the tvBase structure.
 *
 * @param  threeD  handle to rendering state
 * @param  geo         handle to geo dataset
 *
 * @return   int
 *
 * @author   Lee Iverson
 * 
 * @version  March 2000
 *
 */

int
tvDatasetSetPriority (tvBase *threeD, TsGeoPyramid *geo, int prio)
{
  static char const funcName[] = "tvDatasetSetPriority";
  int               result;

  TSMTRACE ((THREED_MASK|1, funcName, "[%d] %s", prio, TsGeoName (geo)));

  tvBaseLock (threeD); {
     result = tvDatasetSetPriority_nolock( threeD, geo, prio );
  }; tvBaseUnlock (threeD);

  return result;
}

/*
 * Synopsis: 
 *   void tvNumDatasets( tvBase * )
 *
 * Doc++ Description: *//**
 *   Return a count of the number of datasets being managed by the tvBase
 *   window specified.
 *
 * @param  threeD  tvBase window
 *
 * @return   int
 *
 * @author   Lee Iverson
 * 
 * @version  March 2000
 *
 */

int
tvNumDatasets (tvBase const *threeD)
{
  return list_count (threeD->geoList);
}

/*
 * Synopsis: void tvBaseFrameGrab()
 *	
 *
 * Description: Grabs the currently rendered frame into the
 *     buffer supplied 
 *
 *
 * Externals: NONE
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1994
 *
 */
void
tvBaseFrameGrab(tvBase * threeD,
		unsigned long * imageBuffer)
{
  static char * funcName = "tvBaseFrameGrab";

  /* SET TO THE RIGHT WINDOW */
  glmMakeCurrentWindow (funcName, threeD->glmw); {
    glReadBuffer(GL_BACK);

    /* GRAB THE IMAGE */
    glReadPixels(0, 0, threeD->width, threeD->height,
		 GL_RGB, GL_BYTE, imageBuffer);
  }; glmReleaseWindow (funcName, threeD->glmw);
}


/*
 * Synopsis: static void tvBaseCreateStars()
 *	
 *
 * Description:  Silly function to generate random stars at startup.
 *
 *
 * Externals: NONE
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: June 1995
 *
 */
static void
tvBaseCreateStars(tvBase * threeD)
{
  /*static char * funcName = "tvBaseCreateStars";*/
  int i, j;

  for (i = 0; i < MAX_STARS; i++) {
    float d, v[3];

    for (j = 0; j < 3; ++j)
      v[j] = (rand() % 1000) / 1000.0 - 0.5;

    d = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    d = sqrt (d);

    for (j = 0; j < 3; ++j)
      threeD->stars[i][j] = 1e4*v[j]/d;
  }
}

void
tvBaseStartThreads (tvBase * threeD)
{
  static const char funcName[] = "tvBaseStartThreads";

  threeD->started = TRUE;
  tvBaseSetRunnable (threeD, TRUE);

#ifdef VIS_THREAD
  /* START THE VISIBILITY THREAD. */
  if (! threeD->visible_thread) {
    if (pthr_create (&threeD->visible_thread, "Visibility 3D",
		     tvBaseVisibleThread, threeD)) {
      tsmError (funcName, "Unable to spawn the 3D visibility process");
      /* TSMTRINC ((THREED_MASK|1, -1)); */
      return;
    }
    TSMTRACE ((THREED_MASK|1, funcName, "Started visibility thread"));
  }
#endif

  /* Start the .geo discovery thread */

#ifdef HAVE_LIBGEO
  if (! threeD->dotgeo_thread) {
    if (pthr_create (&threeD->dotgeo_thread, ".geo Discovery", tvBaseDotGeoThread, threeD)) {
      tsmError (funcName, "Unable to spawn the .geo discovery process");
      return;
    }
    TSMTRACE ((DOTGEO_MASK|1, funcName, "Started .geo discovery thread"));
  }
#endif
  
}


void
tvBaseStopThreads (tvBase * threeD)
{
  static const char funcName[] = "tvBaseStopThreads";

  /* kill the visibility thread */

#ifdef VIS_THREAD
  if (threeD->visible_thread) {
    if (PR_GetCurrentThread () == threeD->visible_thread) {
      threeD->visible_thread = NULL;
      exit (0);
    } else {
      TSMTRACE ((THREED_MASK|1, funcName, "Cancel visibility thread"));
      PR_Interrupt (threeD->visible_thread);
      PR_JoinThread (threeD->visible_thread);
      TSMTRACE ((THREED_MASK|1, funcName, "Cancelled visibility thread"));
      threeD->visible_thread = NULL;
    }
  }
#endif

  /* kill the .geo discovery thread */

#ifdef HAVE_LIBGEO
  if (threeD->dotgeo_thread) {
    if (PR_GetCurrentThread () == threeD->dotgeo_thread) {
      threeD->dotgeo_thread = NULL;
      exit (0);
    } else {
      TSMTRACE ((DOTGEO_MASK|1, funcName, "Cancel .geo discovery thread"));
      PR_Interrupt (threeD->dotgeo_thread);
      PR_JoinThread (threeD->dotgeo_thread);
      TSMTRACE ((DOTGEO_MASK|1, funcName, "Cancelled .geo discovery thread"));
      threeD->dotgeo_thread = NULL;
    }
  }
#endif
}

/*
 *
 * Description: Changes the fog thickness with the new val
 *
 *
 * Externals: 
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1996
 *
 */
static void
tvBaseDrawFog (tvBase const * threeD)
{
  float const *fog_params = threeD->fog_params;

  if (fog_params[0] <= 0.0)
    glDisable (GL_FOG);
  else {
    glEnable (GL_FOG);
    glFogi (GL_FOG_MODE, GL_EXP2);
    glFogf (GL_FOG_DENSITY, threeD->fog_params[0] / 1000.0);
    glFogfv (GL_FOG_COLOR, &threeD->fog_params[1]);
  }
}

static void
tvBaseSetFogColor (tvBase * threeD, float const *color)
{
  static char const funcName[] = "tvBaseSetFogColor";

  float *fog_params = threeD->fog_params;
  memcpy (&fog_params[1], color, 3 * sizeof (fog_params[1]));
  TSMTRACE ((THREED_MASK|1, funcName, "%g,%g,%g",
	     fog_params[1], fog_params[2], fog_params[3]));
}


/*
 * Synopsis: void tvBaseCalculateLighting()
 *	
 *
 * Doc ++ Description: *//**
 *   Changes lighting and sun angle based on time of day.
 *
 * @param threeD  handle to rendering state
 *
 * @return   void
 *
 * @author   Stephen Lau
 * 
 * @version  1995
 *
 */
static void
tvBaseCalculateLighting(tvBase * threeD)
{
    static char const funcName[] = "tvBaseCalculateLighting";
    float pitch;

    /* GET NEW SUN ANGLE */
    pitch = M_PI * ((threeD->timeOfDay - 6.0) / 12.0);
    if (pitch < 0) {
	pitch = 0;
    } else if (pitch > M_PI) {
	pitch = M_PI;
    }
    sunLight[1].param[0] = cos(pitch);
    sunLight[1].param[1] = 0;
    sunLight[1].param[2] = sin(pitch);

    /* IF NIGHTTIME WE WANT DEPTH CUE INSTEAD OF FOG */
    if (threeD->timeOfDay <= 7 || threeD->timeOfDay >= 17)
	tvBaseSetFogColor (threeD, fogColorDark);
    else
	tvBaseSetFogColor (threeD, fogColorLight);

    if (threeD->timeOfDay >= 6 && threeD->timeOfDay <= 18) {
	sunLight[0].param[0] = 1.0;
	sunLight[0].param[1] = 1.0;
	sunLight[0].param[2] = 1.0;
	sunLight[2].param[0] = sin(pitch);
	sunLight[2].param[1] = sin(pitch);
	sunLight[2].param[2] = sin(pitch);
    } else {
	/* NO SUN AT NIGHT */
	sunLight[0].param[0] = 0.0;
	sunLight[0].param[1] = 0.0;
	sunLight[0].param[2] = 0.0;
	sunLight[2].param[0] = 0;
	sunLight[2].param[1] = 0;
	sunLight[2].param[2] = 0;
    }

    glmMakeCurrentWindow (funcName, threeD->glmw); {

	if (! glIsList (threeD->sunLight_list))
	    threeD->sunLight_list =  glGenLists (1);

	glNewList (threeD->sunLight_list, GL_COMPILE); {
	    gltLightDefv (GL_LIGHT0, sunLight);
	}; glEndList();

    }; glmReleaseWindow (funcName, threeD->glmw);
}


#ifdef CAVE
/*
 * Synopsis: void ThreeDWidgetCAVEChangeTimeOfDay()
 *
 *
 * Description: Changes lighting and sun angle based
 *    on time of day for the CAVE
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *              Stephen Lau
 *
 * Date: April 1996
 *
 */
void
ThreeDWidgetCAVEChangeTimeOfDay(ThreeDWidget * threeDWidget,
				float delta)
{
 static char * funcName = "ThreeDWidgetCAVEChangeTimeOfDay";
 float pitch;
 int i = 0;

    threeDWidget->threeD->timeOfDay += delta;
#ifdef DEBUG
    tsmStatus("%s: Time = %f", funcName, 
		threeDWidget->threeD->timeOfDay);
#endif

    /* MAKE SURE WE STAY INBOUNDS */
    if(threeDWidget->threeD->timeOfDay < 5)
    {
	threeDWidget->threeD->timeOfDay = 5;
    }
    if(threeDWidget->threeD->timeOfDay > 19)
    {
	threeDWidget->threeD->timeOfDay = 19;
    }

    /* GET NEW SUN ANGLE */
    pitch = M_PI * ((threeDWidget->threeD->timeOfDay - 6.0) / 12.0);
    if(pitch < 0)
    {
        pitch = 0;
    }
    if(pitch > M_PI)
    {
        pitch = M_PI;
    }
    sunLight[1].param[0] = cos(pitch);
    sunLight[1].param[1] = sin(pitch);
    sunLight[1].param[2] = 0;

    /* AT NIGHT WE WANT DEPTHCUE INSTEAD OF FOG */
    if(threeDWidget->threeD->timeOfDay <= 7 ||
       threeDWidget->threeD->timeOfDay >= 17)
      tvBaseSetFogColor (threeDWidget->threeD, fogColorDark);
    else
      tvBaseSetFogColor (threeDWidget->threeD, fogColorLight);

    if(threeDWidget->threeD->timeOfDay >= 6 &&
       threeDWidget->threeD->timeOfDay <= 18)
    {
        sunLight[0].param[0] = 1.0;
        sunLight[0].param[1] = 1.0;
        sunLight[0].param[2] = 1.0;
        sunLight[2].param[0] = sin(pitch);
        sunLight[2].param[1] = sin(pitch);
        sunLight[2].param[2] = sin(pitch);
    }
    else
    {
	/* NO SUN AT NIGHT */
        sunLight[0].param[0] = 0.0;
        sunLight[0].param[1] = 0.0;
        sunLight[0].param[2] = 0.0;
        sunLight[2].param[0] = 0;
        sunLight[2].param[1] = 0;
        sunLight[2].param[2] = 0;
    }
}
#endif /* CAVE */


void
trace_matrix (int trace_level, char const *funcName, char const *name,
	      const_GLmatrix m)
{
  char buf[512], *p = buf;
  int i, j;

  sprintf (p, "%s [\n", name);
  p = strchr (p, '\0');
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      sprintf (p, " %7.2f", m[j][i]);
      p = strchr (p, '\0');
    }
    sprintf (p, "\n");
    p = strchr (p, '\0');
  }
  sprintf (p, "]");
  p = strchr (p, '\0');
  pthr_trace_bylevel (trace_level, funcName, "%s", buf);
}


/*
 * Synopsis: void tvBaseViewMatrix (tvBase *threeD,
 *                                      GLmatrix M,
 *                                      GLmatrix P)
 *	
 *
 * Description: We construct the viewing matrices
 *    here for use in the visibility and tile request
 *    calculations. Based upon the current viewpoint 
 *    and fov, we make our own matrices
 *
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: 1995
 *
 */

void
tvBaseViewMatrix (tvBase *threeD, TsGeoPyramid const *geo,
		  GLmatrix M, GLmatrix P)
{
  static char const funcName[] = "tvBaseViewMatrix";
  tvViewpoint const *view = &threeD->view;
  GLmatrix tmp;
  float imgx, backset;

  /* If tvBaseGLAttitude() is modified, that must be reflected here
     as well. */
  TSMTRACE ((VIS_MASK|1, funcName, "roll %.1f pitch %.1f yaw %.1f",
	     view->roll*180.0/M_PI,
	     view->pitch*180.0/M_PI,
	     view->yaw*180.0/M_PI));

  if (threeD->stereo) {
    /* move the viewpoint back to enclose both off-axis frusta if in stereo */
    imgx = 2.0 * view->nearPlane * tan((M_PI / 180.0) * (float)view->fov / 
					2.0 * view->aspect);
    backset = view->nearPlane/ ( imgx / threeD->ipd - 1.0);
    TranslationMatrix(0, 0, backset, tmp);
    RotateMatrix (-view->roll*180.0/M_PI, 0, 0, 1, 
		  (const_GLmatrix) tmp, M);
  }
  else
    RotationMatrix (-view->roll*180.0/M_PI, 0, 0, 1, 
		    M);
  RotateMatrix   (-view->pitch  *180.0/M_PI, 1, 0, 0,
		  (const_GLmatrix) M, tmp);
  RotateMatrix   (-view->yaw*180.0/M_PI, 0, 1, 0,
		  (const_GLmatrix) tmp, M);
  RotateMatrix   (-90.0, 1, 0, 0, 
		   (const_GLmatrix) M, tmp);

  TSMTRACE ((VIS_MASK|1, funcName, 
	     "reference %.1f deg axis %.2f,%.2f,%.2f",
	     view->reference_plane[0],
	     view->reference_plane[1],
	     view->reference_plane[2],
	     view->reference_plane[3]));

  RotateMatrix (-view->reference_plane[0],
		view->reference_plane[1],
		view->reference_plane[2],
		view->reference_plane[3],
		(const_GLmatrix) tmp, M);

  /* Simulate modelview tfms. */
  TRACE_MATRIX (VIS_MASK|2, funcName, "M", M);

  /* Simulate perspective tfms. */
  if (P) {
    TSMTRACE ((VIS_MASK|1, funcName, 
	       "fov %d aspect %g near %g far %g",
	       view->fov, view->aspect, view->nearPlane, view->farPlane));
    if (threeD->stereo) {
      /* change the nearPlane and fov to reflect the backset viewpoint */
      PerspectiveMatrix ((360.0/M_PI)
			 * atan2(imgx , 2.0 * (backset + view->nearPlane)),
			 view->aspect,
			 view->nearPlane + backset,
			 view->farPlane,
			 P);
    }
    else
      /* use a very close near plane solely for culling, not rendering */
      PerspectiveMatrix (view->fov, view->aspect,
			 MIN_NEAR_PLANE, view->farPlane,
			 P);
    TRACE_MATRIX (VIS_MASK|2, funcName, "P", P);
  }
}

static void
tvBaseGLPerspective(tvBase * threeD, 
		    float nearPlane, float farPlane,
		    int stereo, int leftEye)
{
  static char * funcName = "tvBaseGLPerspective";
  tvViewpoint const *view = &threeD->view;

  /* SET UP THE PROJECTION MATRIX */
  TSMTRACE ((THREED_MASK|1, funcName, 
	     "fov %d aspect %g near %g far %g",
	     view->fov, view->aspect, nearPlane, farPlane));
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  /* Set up the pick matrix if selecting. */
  if (threeD->selecting) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix(threeD->pickScreen[0], 
		  viewport[3] - threeD->pickScreen[1],
		  5.0, 5.0, viewport);
  }

  /* Compute left or right eye frustum if in stereo. */
  if (stereo) {
    /* compute half image plane sizes */
    float himgx = nearPlane 
      * tan((M_PI / 360.0) * (float)view->fov * view->aspect);
    float himgy = nearPlane
      * tan((M_PI / 360.0) * (float)view->fov);
    float hipd = threeD->ipd / 2.0;
    if (leftEye) {
      glFrustum(- (himgx - hipd), himgx + hipd,	-himgy, himgy,
		nearPlane, farPlane);
      glTranslatef(hipd, 0.0, 0.0);
    } else { /* right eye */
      glFrustum(- (himgx + hipd), himgx - hipd, -himgy, himgy,
		nearPlane, farPlane);	
      glTranslatef(-hipd, 0.0, 0.0);
    }
  } else {
    gluPerspective(view->fov, view->aspect, nearPlane, farPlane);
  }

  if (TSMTRACEP (THREED_MASK|2)) {
    GLmatrix projMat;
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *) projMat);
    TRACE_MATRIX (THREED_MASK|2, funcName, "projMat", projMat);
  }
}


static void
tvBaseGLAttitude(tvBase * threeD)
{
  static char * funcName = "tvBaseGLAttitude";
  tvViewpoint const *view = &threeD->view;

#ifdef CAVE
  if (cave) {
    CAVENavTransform();
    ThreeDWidgetSetCAVEGLViewpoint (threeDWidget);
    return;
  }
#endif /* CAVE */

  /* SET UP THE MODELVIEW MATRIX */
  TSMTRACE ((THREED_MASK|1, funcName, 
	     "roll %.1f pitch %.1f yaw %.1f",
	     view->roll*180.0/M_PI,
	     view->pitch*180.0/M_PI,
	     view->yaw*180.0/M_PI));
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glRotatef(-view->roll /M_PI*180.0, 0, 0, 1);
  glRotatef(-view->pitch/M_PI*180.0, 1, 0, 0);
  glRotatef(-view->yaw  /M_PI*180.0, 0, 1, 0);
  glRotatef(                  -90.0, 1, 0, 0);

  TSMTRACE ((THREED_MASK|1, funcName, 
	     "reference %.1f deg axis %.2f,%.2f,%.2f",
	     view->reference_plane[0],
	     view->reference_plane[1],
	     view->reference_plane[2],
	     view->reference_plane[3]));
  glRotatef(-view->reference_plane[0],
	    view->reference_plane[1],
	    view->reference_plane[2],
	    view->reference_plane[3]);

  glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) threeD->viewMatrix);
  TRACE_MATRIX (THREED_MASK|2, funcName,
		"viewMat", threeD->viewMatrix);
}


/*
 * Synopsis: void tvBaseGeoViewMatrix (tvBase* threeD,
 *					     TsGeoPyramid const *geo,
 *					     GLmatrix M,
 *					     GLmatrix P,
 *					     GLmatrix PM)
 *	
 *
 * Description: We construct the viewing matrices
 *    here for use in the visibility and tile request
 *    calculations. Based upon the current viewpoint 
 *    and fov, we make our own matrices
 *
 *
 * Externals: NONE
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 */

void
tvBaseGeoViewMatrix (tvBase *threeD,
		     TsGeoPyramid const *geo,
		     GLmatrix M, GLmatrix P, GLmatrix PM)
{
  static char const funcName[] = "tvBaseGeoViewMatrix";
  double const *geoOrigin = TsGeoOrigin (geo);
  float const *geoOrient = TsGeoOrientation (geo);
  tvViewpoint const *view = &threeD->view;
  GLmatrix tmp;
  double origin[3];

  /* If tvBaseGLViewpointLVCS() is modified, that must be reflected here
     as well. */
  tvBaseViewMatrix (threeD, geo, M, P);

  if ( TsGeoIsSky( geo ) ) {
    origin[0] = geoOrigin[0];
    origin[1] = geoOrigin[1];
    origin[2] = geoOrigin[2];
  } else {
    origin[0] = geoOrigin[0] - view->eye[0];
    origin[1] = geoOrigin[1] - view->eye[1];
    origin[2] = geoOrigin[2] - view->eye[2];
  }

  TSMTRACE ((VIS_MASK|1, funcName, "eye %.0f,%.0f,%.0f",
	     view->eye[0], view->eye[1], view->eye[2]));
  TranslateMatrix (origin[0], origin[1], origin[2], (const_GLmatrix) M, tmp);
  RotateMatrix (geoOrient[0], geoOrient[1], geoOrient[2], geoOrient[3],
		(const_GLmatrix) tmp, M);

  if (P && PM) {
    MultMatrix ((const_GLmatrix) M, (const_GLmatrix) P, PM);
    TRACE_MATRIX (VIS_MASK|2, funcName, "PM", M);
  }
}

/*
 * Synopsis:
 *  void tvBaseGLViewpointLVCS (tvViewpoint const *view,
 *                              TsGeoPyramids const *geo)
 *
 * Doc++ Description: *//**
 *  Sets up the OpenGL viewpoint for a local vertical coordinate system
 *  at the specified geoorigin. That is, you specify a geocentric
 *  coordinate and orientation from which all further OpenGL commands
 *  will be relative to.
 *
 * @param  view      The current viewpoint specification
 * @param  geo       The TsGeoPyramid for the local coordinate frame
 * 
 * @return void
 *
 * @author
 *
 */

void
tvBaseGLViewpointLVCS (tvViewpoint const *view, TsGeoPyramid const *geo)
{
  static const char funcName[] = "tvBaseGLViewpointLVCS";
  double const *geoOrigin = TsGeoOrigin( geo );
  float const  *geoOrient = TsGeoOrientation( geo );

  TSMTRACE ((THREED_MASK|1, funcName, "eye %.0f,%.0f,%.0f",
	     view->eye[0], view->eye[1], view->eye[2]));

  if ( TsGeoIsSky( geo ) ) {
    glTranslatef(geoOrigin[0],
		 geoOrigin[1],
		 geoOrigin[2]);
  } else {
    glTranslatef(geoOrigin[0] - view->eye[0],
		 geoOrigin[1] - view->eye[1],
		 geoOrigin[2] - view->eye[2]);
  }

  glRotatef (geoOrient[0], geoOrient[1], geoOrient[2], geoOrient[3]);
  
  if (TSMTRACEP (THREED_MASK|VIS_MASK|2)) {
    GLmatrix viewMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &viewMatrix);
    TRACE_MATRIX (THREED_MASK|VIS_MASK|2, funcName,
		  "viewMat", &viewMatrix);
  }
}

/*
 * Synopsis:
 *  void tvBaseGLViewpointCustomLVCS (tvViewpoint const *view,
 *                                    double const geoOrigin[3],
 *                                    float const geoOrient[4])
 *
 * Doc++ Description: *//**
 *  Sets up the OpenGL viewpoint for a local vertical coordinate system
 *  at the specified geoorigin. That is, you specify a geocentric
 *  coordinate and orientation from which all further OpenGL commands
 *  will be relative to.
 *
 * @param  view      The current viewpoint specification
 * @param  geoOrigin The geocentric location of the local coordinate frame
 * @param  geoOrient The yaw, pitch, roll of the local coordinate frame
 * 
 * @return void
 *
 * @author
 *
 */

void
tvBaseGLViewpointCustomLVCS (tvViewpoint const *view,
			     double const geoOrigin[3],
			     float const geoOrient[4])
{
  static const char funcName[] = "tvBaseGLViewpointCustomLVCS";
  TSMTRACE ((THREED_MASK|1, funcName, "eye %.0f,%.0f,%.0f",
	     view->eye[0], view->eye[1], view->eye[2]));
  glTranslatef(geoOrigin[0] - view->eye[0],
	       geoOrigin[1] - view->eye[1],
	       geoOrigin[2] - view->eye[2]);
  glRotatef (geoOrient[0], geoOrient[1], geoOrient[2], geoOrient[3]);
  
  if (TSMTRACEP (THREED_MASK|VIS_MASK|2)) {
    GLmatrix viewMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) &viewMatrix);
    TRACE_MATRIX (THREED_MASK|VIS_MASK|2, funcName,
		  "viewMat", &viewMatrix);
  }
}

#ifdef CAVE
/*
 * Synopsis: void ThreeDWidgetSetCAVEGLViewpoint()
 *	
 *
 * Description: For the CAVE, we need to make our own projection
 *    matrix and viewing matrix for the visibiity and tile request
 *    processes. We use the CAVE near and far planes and also
 *    the CAVE pitch componenet
 *
 * Externals: 
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: April 1996
 *
 */
static void
ThreeDWidgetSetCAVEGLViewpoint(ThreeDWidget * threeDWidget)
{
 static char *funcName = "ThreeDWidgetSetCAVEGLViewpoint";
 GLmatrix viewMat, projMat, projMat2;
 GLmatrix tmp;
 int i, j; 
 extern float * caveYaw;
 extern float * cavePitch;
 tvViewpoint *view = &threeDWidget->view;
 double const *geoOrigin = TsGeoOrigin (geo);

    /* MAKE OUR OWN PROJECTION MATRIX */
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)projMat2);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (threeDWidget->threeD->selecting) {
      GLint viewport[4];
      
      glGetIntegerv(GL_VIEWPORT, viewport);
      gluPickMatrix(threeDWidget->threeD->pickScreen[0],
		    viewport[3] - threeDWidget->threeD->pickScreen[1],
		    1.0, 1.0, viewport);
    }
    gluPerspective(.1*(800),  view->aspect, CAVENear,  CAVEFar);
 
    /* GET THE VIEW ANGLES */
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)tmp);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
#ifdef DEBUG
    for(i = 0; i < 4; i++)
    {
	for(j = 0; j < 4; j++)
	{
	    fprintf(stderr,"%f\t", tmp[i][j]);
	}
	fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
    fprintf(stderr,"%f %f\n", asin(tmp[1][2]) * 180.0 / M_PI, asin(tmp[0][0]) * 180.0 / M_PI);
#endif
    view->pitch = asin(tmp[1][2]);
    view->yaw = *caveYaw;

    /* AND MAKE OUR OWN MATRIX */
    glLoadIdentity();
    glRotatef(view->roll*180.0/M_PI, 0, 0, 1);
    glRotatef(view->pitch * 180.0 / M_PI, 1, 0, 0);
    glRotatef(view->yaw * 180.0 / M_PI + 90.0, 0, 1, 0);

    glTranslatef((GLfloat) geoOrigin[0] - (view->eye[0] + tmp[3][0]),
		 (GLfloat) geoOrigin[1] - (view->eye[1] + tmp[3][1]),
		 (GLfloat) geoOrigin[2] - (view->eye[2] + tmp[3][2]));
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) viewMat);

#ifdef DEBUG
    for(i = 0; i < 4; i++)
    {
	for(j = 0; j < 4; j++)
	{
	    tsmStatus("%s: %f\n", funcName, tmp[i][j]);
	}
    }
#endif

    glGetFloatv(GL_PROJECTION_MATRIX,  (GLfloat *)projMat);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( (GLfloat *)projMat2);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glRotatef(-90.0, 0, 1, 0);
}


/*
 * Synopsis: void ThreeDWidgetCAVEBtnDown()
 *
 * Description:  When the user presses a mouse button down,
 *    we need to store the value for drawing crosshairs later
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *              Stephen Lau
 *
 * Date: April 1996
 *
 */
void
ThreeDWidgetCAVEBtnDown(ThreeDWidget * threeDWidget)
{
  static char * funcName = "ThreeDWidgetCAVEBtnDown";

  threeDWidget->threeD->xMouse = threeDWidget->threeD->xMouseDown = CAVEgetvaluator(CAVE_MOUSEX);
  threeDWidget->threeD->yMouse = threeDWidget->threeD->yMouseDown = CAVEgetvaluator(CAVE_MOUSEY);
  threeDWidget->threeD->MouseDown = TRUE;
}

#endif /* CAVE */


void tvBaseDrawFrustumCB(tvBase *threeD, PRBool wireframe)
{
  if (threeD->freezeTiles)
    tvBaseDrawFrustum(&(threeD->freezeView), wireframe);
  else
    tvBaseDrawFrustum(&(threeD->view), wireframe);
}


/*
 * Synopsis: 
 *   TsGeoPyramid *tvGetDataset( tvBase *, char const * )
 *
 * Doc++ Description: *//**
 *   Returns the TsGeoPyramid structure for the named geoset that is
 *   associated with the specified tvBase. For example, if you
 *   know that the name of the geoset is "Ft Benning", then this 
 *   function will give you the TsGeoPyramid structure for the 
 *   Fort Benning geoset. Alternatively, you can use the URL for
 *   the OI dataset as the search name, e.g. the geoname could also
 *   be "http://www.tvgeo.com/TileSets/Pyramids/benning.oi".
 *
 * @param threeD   handle to rendering state
 * @param geoname  name of geoset sought for retrevial
 *
 * @return    A valid TsGeoPyramid structure, or NULL
 *
 * @author    Martin Reddy
 *
 * @version   31 May 1999
 *
 */

TsGeoPyramid *
tvGetDataset( tvBase const *threeD, char const *geoname )
{
  list_iter_t i;

  if ( threeD == NULL || geoname == NULL ) return NULL;

  /* go through the list of all GeoSets for this tvBase  */

  for ( i = list_first( threeD->geoList );
	! list_done( i );
	i = list_next( i ) ) {

    /* get the next GeoElement in this tvBase's list */

    GeoElement *geoelem = list_value( i );
    if ( geoelem == NULL || geoelem->geo == NULL ) continue;

    /* does the geoname field match our search string? */

    if ( strcasecmp( TsGeoName( geoelem->geo ), geoname ) == 0 )
      return geoelem->geo;

    /* does the image dataset URL match our search string? */

    {
      char const *urls[2];
      int n = TsGeoURLs (geoelem->geo, urls);
      if (n >= 1 && strcasecmp( urls[0], geoname ) == 0 )
	return geoelem->geo;
    }
  }
  
  return NULL;
}

void
tvBaseEnableVArray (void)
{
  glEnable (GL_VERTEX_ARRAY_EXT);
  glEnable (GL_NORMAL_ARRAY_EXT);
  glEnable (GL_TEXTURE_COORD_ARRAY_EXT);
#ifdef GL_VERSION_1_1
  /* under OGL 1.1, Vertex arrays are enabled with the */
  /* glEnableClientState() call rather than glEnable() */
  glEnableClientState (GL_VERTEX_ARRAY_EXT);
  glEnableClientState (GL_NORMAL_ARRAY_EXT);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY_EXT);
#endif
}


static void
MultVecMatrix3(float const v[3], const_GLmatrixd m, float o[3])
{
  float v0 = v[0], v1 = v[1], v2 = v[2];
  o[0] = m[0][0] * v0 + m[0][1] * v1 + m[0][2] * v2;
  o[1] = m[1][0] * v0 + m[1][1] * v1 + m[1][2] * v2;
  o[2] = m[2][0] * v0 + m[2][1] * v1 + m[2][2] * v2;
}

int
tvBaseDISEnable (tvBase *threeD, tvVariable which, int val)
{
  static char const funcName[] = "tvBaseDISEnable";
  int prev;

  switch (which) {
  case TV_DIS_SEND:
    prev = threeD->disSending;
    if (prev != val) {
      TSMTRACE ((THREED_MASK|1, funcName, "SEND %s",
		 val ? "True" : "False"));
      threeD->disSending = val;
      if (threeD->started) {
	if (val) {
	  tvBaseStartDIS (threeD->disSiteID);
	  tvBaseSetDISView (threeD, TRUE,
			    threeD->disForceID, threeD->disEntityID);
	} else if (! threeD->disMonitoring)
	  tvBaseStopDIS ();
      }
    }
    break;
  case TV_DIS_MONITOR:
    prev = threeD->disMonitoring;
    if (prev != val) {
      TSMTRACE ((THREED_MASK|1, funcName, "MONITOR %s",
		 val ? "True" : "False"));
      threeD->disMonitoring = val;
      if (threeD->started) {
	if (val)
	  tvBaseStartDIS (threeD->disSiteID);
	else if (! threeD->disSending)
	  tvBaseStopDIS ();
      }
    }
    break;
  }

  return prev;
}

/*
 * Synopsis: 
 *   void tvDrawLoadScreen( tvBase *threeD )
 *
 * Description:
 *   Generates an OpenGL frame with a loading screen message that
 *   can be used to provide feedback to the user in the main
 *   window while the application is performing initialization
 *   routines and connecting to the initial datasets.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 10 January 2000
 *
 */

void
tvDrawLoadScreen( tvBase *threeD )
{
  static const char funcName[] = "tvDrawLoadScreen";
  int        width, height;
  
  if ( threeD == NULL ) return;

  width  = threeD->width;
  height = threeD->height;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  glmMakeCurrentWindow( funcName, threeD->glmw ); {

    glPushAttrib( GL_CURRENT_BIT|GL_ENABLE_BIT ); {

      /* initialise the model and projection matrices */

      glMatrixMode( GL_PROJECTION );
      glLoadIdentity();
      glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
      glMatrixMode( GL_MODELVIEW );
      glLoadIdentity();

      /* clear the screen and display the loading message */
      
      glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      
      glColor4ub( 0xFF, 0xAA, 0xAA, 0xff);
      glRasterPos2i( (width - strlen(threeD->splash_message) * 9) / 2,
		     height / 2 );
      glmCharstr( threeD->glmw, GL_FONT_R, threeD->splash_message );
      
      /* display the SRI logo in the bottom-right */
      if (threeD->showLogo) 
	tvBaseSRILogo (threeD);

    }; glPopAttrib ();

  }; glmReleaseWindow( funcName, threeD->glmw );
  
}

/*
 * Synopsis: 
 *   void tvBaseSetSize( tvBase *threeD, int width, int height )
 *
 * Doc++ Description: *//**
 *   Sets the size of the render viewport.
 *
 *   Sets the dimensions of the tvBase's viewport in units of
 *   pixels. This function should be called at initialization time in
 *   order to set the initial window size. It may also be called when the
 *   client window has been resized. Note that this does not do a
 *   a refresh.
 *
 * @param threeD  handle to rendering state
 * @param width       the new width of the viewport in pixels
 * @param height      the new height of the viewport in pixels
 *
 * @return void
 *
 * @author Martin Reddy
 *
 * @version 3 February 2000
 *
 */

static void
tvBaseSetSize( tvBase *threeD, int width, int height )
{
  const char *funcName = "tvBaseSetSize";

  TSMTRACE(( THREED_MASK|1, funcName, "%d x %d", width, height ));

  if ( threeD == NULL ) return;

  /* store the new width and height values in the tvBase */

  threeD->width  = width;
  threeD->height = height;

  /* calculate the new aspect ratio */

  if ( height != 0 ) threeD->view.aspect = width / (float) height;

  /* set the new OpenGL viewport size */
  threeD->update_viewport = TRUE;

  if ( threeD->graphics ) {
    /* update the view based on resize parameters */
    tvBaseSetView_nolock (threeD, &threeD->view, FALSE);
  }

  TSMTRACE ((THREED_MASK|1, funcName, "done"));
}

/*
 * Synopsis: 
 *   void tvSaveScreen( tvBase *, int, int, char const * )
 *
 * Doc++ Description: *//**
 *   Saves an snapshot of the current view to an image file.
 *
 *   This function will take a snapshot of the currently rendered image
 *   and write it out to the specified file on local disk. If the width
 *   and height parameters are set to -1, then the image that is saved 
 *   is the same size as the viewport. If you specify alternative values
 *   for width and height, then the image will be scaled to this 
 *   dimension before writing to disk.
 *
 *   Currently, this function only generates binary PPM format images.
 *
 *   N.B. this function must be called by the OpenGL rendering thread.
 *
 * @param threeD  handle to rendering state
 * @param width       the width of the image to write in pixels (or -1)
 * @param height      the height of the image to write in pixels (or -1)
 * @param file        the filename of the image to create.
 *
 * @return void
 *
 * @author Martin Reddy
 *
 * @version 7 May 2000
 *
 */

void
tvSaveScreen( tvBase *threeD, int width, int height, char const *file )
{
  const char *funcName = "tvSaveScreen";
  GLvoid     *pixels = NULL;
  FILE       *fp = NULL;

  TSMTRACE(( THREED_MASK|2, funcName, "Writing %d x %d image to %s",
	     width, height, file ));

  /* explicitly redraw the screen now - seem to need to do this to */
  /* make sure that we grab the latest image. Yeah, it's a hack.   */

  tvBaseDraw( threeD->glmw, threeD );

  /* allocate buffer memory to store the screen pixels */

  pixels = tsmMemMalloc( (threeD->width+1) * (threeD->height+1) * 3 );
  if ( pixels == NULL ) return;

  /* grab the current screen */

  tvBaseLock( threeD ); {
    glmMakeCurrentWindow( funcName, threeD->glmw );
  
    glPushAttrib( GL_PIXEL_MODE_BIT ); {
      glPixelStorei( GL_PACK_ALIGNMENT, 1 );
      glReadPixels( 0, 0, threeD->width, threeD->height, GL_RGB,
		    GL_UNSIGNED_BYTE, pixels );
    }; glPopAttrib();
    
    glmReleaseWindow( funcName, threeD->glmw );
  }; tvBaseUnlock( threeD );

  /* scale the image if requested */

  if ( width < 0 || height < 0 ) {
    width  = threeD->width;
    height = threeD->height;
  } else {
    GLvoid *buffer = tsmMemMalloc( width * height * 3 );

    gltScaleImage( 3, threeD->width, threeD->height,
		   threeD->width * 3, pixels, width, height,
		   width * 3, buffer );

    tsmMemFree( pixels );
    pixels = buffer;
  }

  /* invert image so that top-left corner is (0,0) */

  gltInvertImage( pixels, width, height, 3 );

  /* save the image to a binary PPM file */

  if ( ( fp = fopen( file, "wb" ) ) == NULL ) {
    tsmMemFree( pixels );
    return;
  }

  fprintf( fp, "P6\n%d %d\n255\n", width, height );
  fwrite( pixels, 3, width * height, fp );
  fclose( fp );

  tsmMemFree( pixels );
}
