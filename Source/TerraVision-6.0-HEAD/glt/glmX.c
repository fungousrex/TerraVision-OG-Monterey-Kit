/*
 * Name: GLMX.C
 *
 * Description: OpenGL/GLX/Xlib helper functions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:55 $
 *     $Author: reddy $
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>

#include <pthr/trace.h>
#include <pthr/list.h>

#include "glt.h"
#include "glmx.h"

#include <assert.h>
#include <unistd.h>

glmContext xCurrentContext;
glmContextVTable xContextVTable;

static list_t *baseContexts;

static glmContext xCreateContext (Display *disp, XVisualInfo *vinfo,
				  glmContext share,
				  glmContextProperties *props);

static void
xContextPropertiesFromVisual (Display *disp, XVisualInfo *vis,
			      glmContextProperties *props)
{
  int attr;
  props->id = vis->visualid;

  glXGetConfig (disp, vis, GLX_BUFFER_SIZE, &attr);
  props->colorDepth = attr;

  glXGetConfig (disp, vis, GLX_BUFFER_SIZE, &attr);
  props->colorDepth = attr;

  glXGetConfig (disp, vis, GLX_LEVEL, &attr);
  props->overlay = (attr > 0);
  props->underlay = (attr < 0);

  glXGetConfig (disp, vis, GLX_RGBA, &attr);
  props->colorIndex = (! attr || vis->class == DirectColor);

  glXGetConfig (disp, vis, GLX_DOUBLEBUFFER, &attr);
  props->singleBuffer = !attr;

  glXGetConfig (disp, vis, GLX_STEREO, &attr);
  props->stereo = !!attr;
    
  glXGetConfig (disp, vis, GLX_DEPTH_SIZE, &attr);
  props->zbufDepth = attr;

  glXGetConfig (disp, vis, GLX_STENCIL_SIZE, &attr);
  props->stencilDepth = attr;

#ifdef GLX_SAMPLE_BUFFERS_SGIS
  glXGetConfig (disp, vis, GLX_SAMPLE_BUFFERS_SGIS, &attr);
  props->multiBufs = attr;
  glXGetConfig (disp, vis, GLX_SAMPLES_SGIS, &attr);
  props->multiBufSamples = attr;
#else
  props->multiBufs = 0;
  props->multiBufSamples = 0;
#endif

#ifdef GLX_VISUAL_CAVEAT_EXT
  glXGetConfig (disp, vis, GLX_VISUAL_CAVEAT_EXT, &attr);
  props->slow = (attr != GLX_NONE_EXT);
#else
  props->slow = 0;
#endif
}

glmContext
glmXChooseContext (Display *disp, Screen *scr,
		   glmChooseContextFunc scoref, void *closure)
{
  static char const funcName[] = "glmXChooseContext";
  XVisualInfo *best;
  int best_score = 0;
  int i, nvisuals;
  XVisualInfo tmpl;
  XVisualInfo *visuals;
  glmContextProperties props;
  glmContextProperties bestProps;
  glmContextProperties *prefs = closure;

  /* Get a list of all available visuals on screen. */
  tmpl.screen = XScreenNumberOfScreen (scr);
  visuals = XGetVisualInfo (disp, VisualScreenMask, &tmpl, &nvisuals);

  /* Use default score function */
  if (! scoref)
    scoref = glmContextChooseDefault;

  /* Walk through them all choosing largest score. */
  for (i = 0; i < nvisuals; ++i) {
    XVisualInfo *vis = &visuals[i];
    int attr, score;

    glXGetConfig (disp, vis, GLX_USE_GL, &attr);
    if (! attr)  continue;

    xContextPropertiesFromVisual (disp, vis, &props);

    if ( prefs && prefs->forceid && prefs->visualid == vis->visualid )
      score = 10000000;  /* force to a specifc visual ID */
    else
      score = (*scoref) (&props, closure);  /* otherwise, select one */

    if ( prefs && prefs->listmodes )
      fprintf( stderr, "%d: %s%s %s%s%s Col:%d Dep:%d Stl:%d Mlt:%d/%d "
	       " (score %d)\n",
	       vis->visualid,
	       props.colorIndex ? "IDX" : "RGB",
	       props.overlay ? "/Over" : props.underlay ? "/Under" : "",
	       props.singleBuffer ? "SingleBuf" : "DoubleBuf",
	       props.stereo ? "/Stereo" : "",
	       props.slow ? " Slow" : "",
	       props.colorDepth,
	       props.zbufDepth,
	       props.stencilDepth,
	       props.multiBufs, props.multiBufSamples,
	       score );

    if (score > best_score) {
      best_score = score;
      best = vis;
      bestProps = props;
    }
  }

  if ( prefs && prefs->listmodes )
    fprintf( stderr, "Selecting visual %d\n", best->visualid );

  if (! best_score) {
    XFree (visuals);
    return NULL;
  }

  /* Free most of memory allocated, returning a context based only on the
     relevant visual. */
  {
    glmContext ctx = xCreateContext (disp, best, NULL, &bestProps);
    XFree (visuals);
    return ctx;
  }
}

#if 0
/*
 * Synopsis: glmVisualInfo
 *           glmXChooseVisualByScore (Display *disp, Screen *scr,
 *                                   int nattribs, int *attribs,
 *                                   int (*scoref) (int *attribs, void *)
 *                                   void *closure)
 *
 * Description:  Finds the "best" OpenGL visual available by applying a
 *               scoring function based on a set of attributes returned from
 *               glXGetConfig().  Highest score wins.
 *
 * Externals: None.
 *
 * Returns: Pointer to the selected XVisualInfo
 *
 * Author: Lee Iverson
 *
 * Date: February 1998
 *
 */

XVisualInfo *
glmXChooseVisualByScore (Display *disp, Screen *scr,
			 int nattribs, int *attribs,
			 int (*scoref) (XVisualInfo *vis, 
					int *attribs, void *),
			 void *closure)
{
  static char const funcName[] = "glmChooseVisualByScore";
  XVisualInfo *best;
  int best_score = 0;
  int i, nvisuals;
  XVisualInfo tmpl;
  XVisualInfo *visuals;
  int *attr_vals = alloca (nattribs * sizeof (int));

  /* Get a list of all available visuals on screen. */
  tmpl.screen = XScreenNumberOfScreen (scr);
  visuals = XGetVisualInfo (disp, VisualScreenMask, &tmpl, &nvisuals);

  /* Walk through them all choosing largest score. */
  for (i = 0; i < nvisuals; ++i) {
    int attr, score;

    glXGetConfig (disp, &visuals[i], GLX_USE_GL, &attr_vals[0]);
    if (! attr_vals[0])
      continue;

    for (attr = 0; attr < nattribs; ++attr)
      glXGetConfig (disp, &visuals[i], attribs[attr], &attr_vals[attr]);

    PTRACE ((GLT_MASK|1, funcName, "vis %#x", visuals[i].visualid));
    score = (*scoref) (&visuals[i], attr_vals, closure);
    if (score > best_score) {
      best_score = score;
      best = &visuals[i];
    }
  }

  if (! best_score) {
    XFree (visuals);
    return NULL;
  }

  /* Free most of memory allocated, returning only the relevant visual. */
  {
    XVisualInfo *ret = (XVisualInfo *) malloc (sizeof (XVisualInfo));
    *ret = *best;
    XFree (visuals);
    return ret;
  }
}

/*
 * Synopsis: XVisualInfo *
 *           glmChooseVisual (Display *disp, Screen *scr, int doublebuffer)
 *
 * Description:  Finds a sufficiently rich and deep visual by making repeated
 *               passes through glXChooseVisual ().
 *
 * Externals: None.
 *
 * Returns: Pointer to the selected XVisualInfo
 *
 * Author: Lee Iverson
 *
 * Date: March 1997
 *
 */
XVisualInfo *
glmXChooseVisual (Display *disp, Screen *scr, int doublebuffer, int stereo)
{
  static char const funcName[] = "glmChooseVisual";

  XVisualInfo *glxVInfo = 0;

  /* Initialize OpenGL context */
  int screen_num = XScreenNumberOfScreen (scr);

  /* Attributes for OpenGL visuals. */
  int glXattributes[] = {
    GLX_DEPTH_SIZE, 0,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_LEVEL, 0,
    GLX_RGBA,
#ifdef GLX_SAMPLES_SGIS
    GLX_SAMPLES_SGIS, 4,
#endif
    None,
    None
  };

  /* As far as GLX visuals, first choice is 24 bits, second is 12. */
  static struct {
    int depth, rgb_size;
  } glxSelection[] = {
    {24, 8}, {16, 8}, {24, 4}, {16, 4}, {12, 4}, {8, 4}, {0, 0}
  };
  int idx = 0;

  /* Optionally specify double buffering. */
  if (doublebuffer)
    glXattributes[11] = GLX_DOUBLEBUFFER;

  PTRINC ((GLT_MASK|1, 1));
  PTRACE ((GLT_MASK|1, funcName, "display %p screen %d", disp, screen_num));

  for (idx = 0; glxSelection[idx].depth; ++idx) {
#ifdef GLX_SAMPLES_SGIS
    int smpl, smpl_i;
#endif
    int depth = glxSelection[idx].depth;
    int rgb_size = glxSelection[idx].rgb_size;
    glXattributes[1] = depth;
    glXattributes[3] = rgb_size;
    glXattributes[5] = rgb_size;
    glXattributes[7] = rgb_size;
    PTRACE ((GLT_MASK|1, funcName, "Check for rgb size %d and depth %d",
	     rgb_size, depth));
#ifdef GLX_SAMPLES_SGIS
    for (smpl_i = 7; glXattributes[smpl_i] == GLX_SAMPLES_SGIS; ++smpl_i);
    for (smpl = 4; smpl >= 0; smpl -= 2) {
      if (smpl) {
	glXattributes[smpl_i] = GLX_SAMPLES_SGIS;
	glXattributes[smpl_i+1] = smpl;
	PTRACE ((GLT_MASK|1, funcName, "Check for multisample size %d", smpl));
      } else {
	glXattributes[smpl_i] = None;
      }
#endif
      glxVInfo = glXChooseVisual (disp, screen_num, glXattributes);
      if (glxVInfo) {
	PTRACE ((GLT_MASK|1, funcName, 
		 "Using GLX visual %p id=%02x depth=%d cls=%d",
		 glxVInfo, glxVInfo->visualid,
		 glxVInfo->depth, glxVInfo->class));
	goto done;
      }
#ifdef GLX_SAMPLES_SGIS
    }
#endif
  }
 done:

  PTRINC ((GLT_MASK|1, -1));
  return glxVInfo;
}
#endif

#define GLM_LOCKING (0)

#if GLM_LOCKING
# include <prlock.h>

  /* A global lock for accessing the OpenGL/GLX pipeline. */
  static PRLock *glm_lock;

# define GLM_LOCK() ((void) PR_Lock (glm_lock))
# define GLM_UNLOCK() ((void) PR_Unlock (glm_lock))
#else
# define GLM_LOCK() ((void) 0)
# define GLM_UNLOCK() ((void) 0)
#endif

static glmContext
xCreateContext (Display *disp, XVisualInfo *vinfo,
		glmContext share,
		glmContextProperties *props)
{
  static char const funcName[] = "xCreateContext";
  GLXContext shareCtx = 0;
  GLXContext glxCtx;

  if (share) {
    xContextData *shdata = share->data;
    shareCtx = shdata->glxCtx;
  }

  glxCtx = glXCreateContext (disp, vinfo, shareCtx, TRUE);
  if (! glxCtx) {
    return 0;
  } else {
    glmContext ctx = (glmContext) calloc (1, sizeof (*ctx));
    xContextData *data = (xContextData *) calloc (1, sizeof (*data));
    data->disp = disp;
    data->glxCtx = glxCtx;
    data->vinfo = *vinfo;
    data->vtable_s = gltVTableINIT;
    data->vtable = &data->vtable_s;

    ctx->data = data;
    ctx->vtable = &xContextVTable;
    ctx->share = share;

    if (props)
      ctx->props = *props;
    else {
      props = &ctx->props;
      xContextPropertiesFromVisual (disp, vinfo, props);
    }

    PTRACE ((GLT_MASK|1, funcName,
	     "%p/%#x: %s%s %s%s%s P:%d D:%d S:%d M:%d/%d",
	     ctx, props->id,
	     props->colorIndex ? "IDX" : "RGB",
	     props->overlay ? "/Over" : props->underlay ? "/Under" : "",
	     props->singleBuffer ? "S" : "D",
	     props->stereo ? "/St" : "",
	     props->slow ? " Slow" : "",
	     props->colorDepth,
	     props->zbufDepth,
	     props->stencilDepth,
	     props->multiBufs, props->multiBufSamples));
	     
    ctx->doubleBuffer = !ctx->props.singleBuffer;
    ctx->stereo = ctx->props.stereo;
    
    /* Add to list of baseContexts if not shared */
    if (! share) {
      if (! baseContexts) baseContexts = list_create (TRUE);
      list_push (baseContexts, ctx);
    }

    return ctx;
  }
}

glmContext
glmXCreateContext (Display *disp, XVisualInfo *vinfo)
{
  int gl_supported;

  glXGetConfig (disp, vinfo, GLX_USE_GL, &gl_supported);
  if (! gl_supported)
    return 0;

  return xCreateContext (disp, vinfo, NULL, NULL);
}

glmContext
xShareContext (glmContext share)
{
  xContextData *data = share->data;
  return xCreateContext (data->disp, &data->vinfo, share, NULL);
}

void
xDestroyContext (glmContext ctx)
{
  static char const funcName[] = "xDestroyContext";
  xContextData *data = ctx->data;

  PTRACE ((GLT_MASK|1, funcName, "%p", ctx));

  /* Remove this base context */
  if (baseContexts) {
    list_iter_t i = list_find (baseContexts, ctx);
    if (list_iter_valid (i)) list_delete (i);
  }

  glXDestroyContext (data->disp, data->glxCtx);
  free (data);
  free (ctx);
}

glmContext
glmXFindContext (Display *disp, Screen *scr,
		 int create,
		 glmContextProperties *props)
{
  list_iter_t i;

  /* Match against pre existing base contexts */
  if (baseContexts) {
    for (i = list_first (baseContexts); ! list_done (i); i = list_next (i)) {
      glmContext ctx = list_value (i);
      xContextData *data = ctx->data;
      if (data->disp == disp
	  && scr == ScreenOfDisplay (disp, data->vinfo.screen)
	  /* Optionally match against passed-in props */
	  && (! props || glmContextMatchProperties (props, &ctx->props)))
	return ctx;
    }
  }

  /* If create is set, then create a context if not found */
  if (create)
    return glmXChooseContext (disp, scr, glmContextChooseDefault, props);

  return 0;
}

XVisualInfo *
glmXContextVisualInfo (glmContext ctx)
{
  xContextData *data = ctx->data;
  return &data->vinfo;
}

glmContextVTable xContextVTable = {
  xShareContext,
  xDestroyContext
};
