/*
 * Name: glmWin.C
 *
 * Description: OpenGL/Windows helper functions for MS-Windows.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Open Issues:
 *     o What about the sharing and copying of contexts?
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
#include <assert.h>

#include <prlock.h>
#include <pratom.h>
#include <pthr/trace.h>
#include <pthr/list.h>

#include "glt.h"
#include "glm.h"
#include "glmimpl.h"

static glmWindowVTable winWindowVTable;
static glmContextVTable winContextVTable;

static void winRemoveRedraw (glmWindow glmw);
static void winRemoveRedraw1 (glmWindow glmw);

static list_t *baseContexts;

typedef struct winContextData {
  HGLRC   glctx;
  HWND    hWnd;
  HDC     hDC;
  int     iFormat;
  PIXELFORMATDESCRIPTOR pfd;

  gltVTable glt_vtable;
} *winContextData;

glmContext glmCurrentContext;

/* queue to keep track of glm's that need redraws */
static list_t *redrawQ = NULL;

#define GLM_TIMER  99
UINT glm_timer = 0;

static void
winContextPropertiesFromPFD (int iFormat, 
			     PIXELFORMATDESCRIPTOR const *pfd, 
			     glmContextProperties *props)
{
  static char const funcName[] = "winContextPropertiesFromPFD";
  props->id = iFormat;
  props->overlay = (pfd->iLayerType == PFD_OVERLAY_PLANE);
  props->underlay = (pfd->iLayerType == PFD_UNDERLAY_PLANE);

  /* It turns out that on nVidia and Oxygen-based cards, SUPPORT_GDI  */
  /* seems to imply software-only rendering. Pete Tinker, Nov 28 2000 */

  props->colorIndex = (! (pfd->iPixelType == PFD_TYPE_RGBA)
		       || ! (pfd->dwFlags & PFD_SUPPORT_GDI));

  /*
  props->colorIndex = ! (pfd->iPixelType == PFD_TYPE_RGBA);
  */

  props->singleBuffer = ! (pfd->dwFlags & PFD_DOUBLEBUFFER);
  props->stereo = (pfd->dwFlags & PFD_STEREO);
  /* Maybe check PFD_GENERIC_ACCELERATED? */
  props->slow = (pfd->dwFlags & PFD_GENERIC_FORMAT);
  props->colorDepth = pfd->cColorBits;
  props->zbufDepth = pfd->cDepthBits;
  props->stencilDepth = pfd->cStencilBits;
  props->multiBufs = 0;
  props->multiBufSamples = 0;
}

static glmContext
winCreateContext (HWND hWnd, HDC hDC, 
		  int iFormat, PIXELFORMATDESCRIPTOR *pfd,
		  glmContext shareCtx, 
		  glmContextProperties const *props)
{
  static char const funcName[] = "winCreateContext";
  glmContext ctx;
  winContextData data;
  HGLRC glctx;
  PIXELFORMATDESCRIPTOR my_pfd;

  if (! pfd) {
    pfd = &my_pfd;
    if (! DescribePixelFormat (hDC, iFormat, sizeof(*pfd), pfd))
      return 0;
  }

  /* set the pixel format for the DC */
  PTRACE ((GLT_MASK|1, funcName, "SetPixelFormat (%p, %d, %p)",
	   hDC, iFormat, pfd));
  if (! SetPixelFormat( hDC, iFormat, pfd )) {
    PTRACE ((GLT_MASK|1, funcName, "SetPixelFormat failed"));
    return 0;
  }

  glctx = wglCreateContext( hDC );
  if (! glctx) return 0;

  PTRACE ((GLT_MASK|1, funcName, "hDC %p iFormat %d share %p",
	   hDC, iFormat, shareCtx));
  if ( shareCtx ) {
    winContextData shdata = shareCtx->data;
    wglShareLists( shdata->glctx, glctx );
  }

  /* allocate a glmContext structure */

  if ( ( ctx = (glmContext) calloc (1, sizeof (*ctx)) ) == NULL )
    return NULL;
  if ( ( data = (winContextData) calloc (1, sizeof (*data)) ) == NULL )
    return NULL;

  /* store the device context */

  data->glctx = glctx;
  data->hWnd  = hWnd;
  data->hDC   = hDC;
  data->iFormat = iFormat;
  data->pfd   = *pfd;

  data->glt_vtable = gltVTableINIT;

  /* now fill in the glmContext structure */

  ctx->data = data;
  ctx->vtable = &winContextVTable;
  ctx->share = shareCtx;

  if (props)
    ctx->props = *props;
  else {
    winContextPropertiesFromPFD (iFormat, pfd, &ctx->props);
    props = &ctx->props;
  }

  PTRACE ((GLT_MASK|1, funcName,
	   "%p/%d: %s%s %s%s%s P:%d D:%d S:%d M:%d/%d",
	   ctx, iFormat,
	   props->colorIndex ? "IDX" : "RGB",
	   props->overlay ? "/Over" : props->underlay ? "/Under" : "",
	   props->singleBuffer ? "S" : "D",
	   props->stereo ? "/St" : "",
	   props->slow ? " Slow" : "",
	   props->colorDepth,
	   props->zbufDepth,
	   props->stencilDepth,
	   props->multiBufs, props->multiBufSamples));

  ctx->doubleBuffer = TRUE;
  ctx->stereo = FALSE;

  /* Add to list of baseContexts if not shared */
  if (! shareCtx) {
    if (! baseContexts) baseContexts = list_create (TRUE);
    list_push (baseContexts, ctx);
  }

  return ctx;
}

/*
 * This routine chooses an accelerated pixel format under Windows.
 * The code is taken from: http://www.wischik.com/lu/programmer/wingl.html
 */

/* this does not seem to be defined under mingwin32 */
#ifndef PFD_GENERIC_ACCELERATED
#define PFD_GENERIC_ACCELERATED  0x00001000
#endif

static int
glmWChoosePixelFmt( HDC hdc, int *p_bpp, int *p_depth, int *p_dbl, int *p_acc )
{ 
  int wbpp   = ( p_bpp==NULL ) ? -1 : *p_bpp;
  int wdepth = ( p_depth==NULL ) ? 16 : *p_depth;
  int wdbl   = ( p_dbl==NULL ) ? -1 : *p_dbl;
  int wacc   = ( p_acc==NULL ) ? 1 : *p_acc;
  int maxindex = 0, num, i;
  int max_bpp, max_depth, max_dbl, max_acc;
  unsigned int maxqual = 0, q;
  PIXELFORMATDESCRIPTOR pfd;

  ZeroMemory(&pfd,sizeof(pfd));
  pfd.nSize=sizeof(pfd);
  pfd.nVersion=1;
  num=DescribePixelFormat(hdc,1,sizeof(pfd),&pfd);
  if (num==0) return 0;

  for (i=1; i<=num; i++) {
    int bpp, depth;
    int pal, mcd, soft, icd, opengl, window, bitmap, dbuff;

    ZeroMemory(&pfd,sizeof(pfd));
    pfd.nSize=sizeof(pfd);
    pfd.nVersion=1;

    DescribePixelFormat(hdc,i,sizeof(pfd),&pfd);

    bpp=pfd.cColorBits;
    depth=pfd.cDepthBits;

    pal=(pfd.iPixelType==PFD_TYPE_COLORINDEX);
    mcd=((pfd.dwFlags & PFD_GENERIC_FORMAT) &&
	      (pfd.dwFlags & PFD_GENERIC_ACCELERATED));
    soft=((pfd.dwFlags & PFD_GENERIC_FORMAT) &&
	       !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
    icd=(!(pfd.dwFlags & PFD_GENERIC_FORMAT) &&
	      !(pfd.dwFlags & PFD_GENERIC_ACCELERATED));
    opengl=(pfd.dwFlags & PFD_SUPPORT_OPENGL);
    window=(pfd.dwFlags & PFD_DRAW_TO_WINDOW);
    bitmap=(pfd.dwFlags & PFD_DRAW_TO_BITMAP);
    dbuff=(pfd.dwFlags & PFD_DOUBLEBUFFER);

    q = 0;
    if (opengl && window) q=q+0x8000;
    if (wdepth==-1 || (wdepth>0 && depth>0)) q=q+0x4000;
    if (wdbl==-1 || (wdbl==0 && !dbuff) || (wdbl==1 && dbuff)) q=q+0x2000;
    if (wacc==-1 || (wacc==0 && soft) || (wacc==1 && (mcd || icd))) q=q+0x1000;
    if (mcd || icd) q=q+0x0040; if (icd) q=q+0x0002;
    if (wbpp==-1 || (wbpp==bpp)) q=q+0x0800;
    if (bpp>=16) q=q+0x0020; if (bpp==16) q=q+0x0008;
    if (wdepth==-1 || (wdepth==depth)) q=q+0x0400;
    if (depth>=16) q=q+0x0010; if (depth==16) q=q+0x0004;
    if (!pal) q=q+0x0080;
    if (bitmap) q=q+0x0001;
    if (q>maxqual) {
      maxqual=q;
      maxindex=i;
      max_bpp=bpp;
      max_depth=depth;
      max_dbl=dbuff?1:0;
      max_acc=soft?0:1;
    }
  }
  if (maxindex==0) return maxindex;
  if (p_bpp!=NULL) *p_bpp=max_bpp;
  if (p_depth!=NULL) *p_depth=max_depth;
  if (p_dbl!=NULL) *p_dbl=max_dbl;
  if (p_acc!=NULL) *p_acc=max_acc;
  return maxindex;
}

glmContext
glmWChooseContext (HWND hWnd, glmChooseContextFunc choosef, void *closure)
{
  static char const funcName[] = "glmWChooseContext";
  PIXELFORMATDESCRIPTOR pfd;
  HDC                   hDC = GetDC( hWnd );
  int                   n_pfds, i;
  int                   best = 0, bestScore = 0;
  glmContextProperties  props;
  glmContextProperties  bestProps;
  PIXELFORMATDESCRIPTOR bestPfd;

  /* Use default score function to find best visual */

  if ( ! choosef ) {
    int bpp=-1;   /* don't care. (or a positive integer) */
    int depth=-1; /* don't care. (or a positive integer) */
    int dbl=1;    /* we want double-buffering. (-1='don't care', 0='none') */
    int acc=1;    /* we want acceleration. (or -1 or 0) */

    int pf = glmWChoosePixelFmt( hDC, &bpp, &depth, &dbl, &acc );  

    ReleaseDC( hWnd, hDC );

    return glmWCreateContext( hWnd, pf );
  }


  /* otherwise, use the user's custom callback routine */

  n_pfds = DescribePixelFormat (hDC, 1, sizeof (pfd), &pfd);

  for (i = 1; i <= n_pfds; ++i) {
    int score;

    if (! DescribePixelFormat (hDC, i, sizeof (pfd), &pfd)
	|| ! (pfd.dwFlags | PFD_SUPPORT_OPENGL))
      continue;

    winContextPropertiesFromPFD (i, &pfd, &props);

    score = (*choosef) (&props, closure);
    PTRACE ((GLT_MASK|1, funcName,
	     "%p/%d: %s%s %s%s%s P:%d D:%d S:%d M:%d/%d (score %d)",
	     hDC, i,
	     props.colorIndex ? "IDX" : "RGB",
	     props.overlay ? "/Over" : props.underlay ? "/Under" : "",
	     props.singleBuffer ? "S" : "D",
	     props.stereo ? "/St" : "",
	     props.slow ? " Slow" : "",
	     props.colorDepth,
	     props.zbufDepth,
	     props.stencilDepth,
	     props.multiBufs, props.multiBufSamples,
	     score));
    if (score > bestScore) {
      best = i;
      bestScore = score;
      bestProps = props;
      bestPfd = pfd;
    }
  }

  if (! bestScore) {
    ReleaseDC (hWnd, hDC);
    return 0;
  } else {
    glmContext ctx;
    bestProps.id = best;
    PTRACE ((GLT_MASK|1, funcName, "Choose %d (score %d)", best, bestScore));
    ctx = winCreateContext (hWnd, hDC, best, &bestPfd, NULL, &bestProps);
    if (! ctx) ReleaseDC (hWnd, hDC);
    return ctx;
  }
}

int
glmWGetPixelFormat( glmContext ctx )
{
  if ( ctx == NULL || ctx->data == NULL )
    return 0;
  else
    return ((winContextData) ctx->data)->iFormat;
}

glmContext
glmWCreateContext (HWND hWnd, int iFormat)
{
  static char const funcName[] = "glmWCreateContext";
  PIXELFORMATDESCRIPTOR pfd;
  glmContext            ctx;
  HDC                   hDC = GetDC (hWnd);
  
  if (iFormat == 0) {
    /* set the pixel format for the DC */

    ZeroMemory( &pfd, sizeof(pfd) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat( hDC, &pfd );
    if (! iFormat) {
      PTRACE ((GLT_MASK|1, funcName, "ChoosePixelFormat failed"));
      ReleaseDC (hWnd, hDC);
      return 0;
    }
  } else {
    DescribePixelFormat (hDC, iFormat, sizeof (pfd), &pfd);
  }

  PTRACE ((GLT_MASK|1, funcName, "hDC %p iFormat %d", hDC, iFormat));
  if (! SetPixelFormat( hDC, iFormat, &pfd )) {
    PTRACE ((GLT_MASK|1, funcName, "SetPixelFormat failed"));
    ReleaseDC (hWnd, hDC);
    return 0;
  }

  ctx = winCreateContext (hWnd, hDC, iFormat, &pfd, NULL, NULL);
  PTRACE ((GLT_MASK|1, funcName, "%p", ctx));
  if (! ctx) ReleaseDC (hWnd, hDC);
  return ctx;
}

glmContext
glmWFindContext (HWND hWnd,
		 int create,
		 glmContextProperties *props)
{
  list_iter_t i;

  /* Match against pre existing base contexts */
  if (baseContexts) {
    for (i = list_first (baseContexts); ! list_done (i); i = list_next (i)) {
      glmContext ctx = list_value (i);
      winContextData data = ctx->data;
      if (data->hWnd == hWnd
	  /* Optionally match against passed-in props */
	  && (! props || glmContextMatchProperties (props, &ctx->props)))
	return ctx;
    }
  }

  /* If create is set, then create a context if not found */
  if (create)
    return glmWChooseContext (hWnd, glmContextChooseDefault, props);

  return 0;
}

static glmContext
winShareContext (glmContext share)
{
  /* there is no sharing of contexts in Windows? */
  winContextData shdata = share->data;
  return winCreateContext (shdata->hWnd, shdata->hDC,
			   shdata->iFormat, &shdata->pfd,
			   share, NULL);
}

static void
winDestroyContext (glmContext ctx)
{
  static char const funcName[] = "winDestroyContext";
  winContextData data = ctx->data;

  PTRACE ((GLT_MASK|1, funcName, "%p", ctx));

  /* Remove this base context */
  if (baseContexts) {
    list_iter_t i = list_find (baseContexts, ctx);
    if (list_iter_valid (i)) list_delete (i);
    PTRACE ((GLT_MASK|1, funcName, "Remove from baseContext list"));
  }

  PTRACE ((GLT_MASK|1, funcName, "ReleaseDC %p wglDeleteContext %p",
	   data->hDC, data->glctx));
  ReleaseDC (data->hWnd, data->hDC);
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( data->glctx );
  free (data);
  free (ctx);
}

typedef struct winWindowData {
  PRLock * lock;
  PRLock * redraw_lock;

  int pendingRedraw;

  glmRedrawFunc redraw;
  void *redraw_closure;

	HWND hWnd;
  short mapped;
  short destroying;
  glmMapNotifyFunc mapNotify;
  void *mapNotifyClosure;
} *winWindowData;

glmWindow
glmWCreateWindow (HWND hWnd, glmContext ctx, int double_buffer, int stereo)
{
  static char const *funcName = "glmWCreateWindow";

  glmWindow glmw = (glmWindow) calloc (1, sizeof (*glmw));
  winWindowData data = (winWindowData) calloc (1, sizeof (*data));

  if (redrawQ == NULL)
    redrawQ = list_create (TRUE);

  PTRACE ((GLT_MASK|1, funcName, "glwm %p (ctx %p)", glmw, ctx));

  data->lock = PR_NewLock ();
  data->redraw_lock = PR_NewLock ();
  data->destroying = FALSE;
  data->hWnd = hWnd;

  glmw->ctx = winShareContext (ctx); /* XXX just ctx here? */
 
  glmw->data = data;
  glmw->vtable = &winWindowVTable;
  glmw->doubleBuffer = double_buffer;
  glmw->stereo = stereo;
  glmw->noflush = FALSE;

  return glmw;
}

static void
winDestroyWindow (glmWindow glmw)
{
  static char const funcName[] = "winDestroyWindow";
  winWindowData data = glmw->data;

  PR_Lock (data->lock); {
    PTRACE ((GLT_MASK|1, funcName, "%p", glmw));
    PTRINC ((GLT_MASK|1, 1));
    data->destroying = TRUE;

    if (data->pendingRedraw || glm_timer)
      winRemoveRedraw1 (glmw);

    /* Since we create a context above, delete it here. */
    winDestroyContext (glmw->ctx);

    PR_DestroyLock (data->redraw_lock);
  }; PR_Unlock (data->lock); 

  PR_DestroyLock (data->lock);
  free (data);
  free (glmw);

  PTRINC ((GLT_MASK|1, -1));
}

static glmContext
winMakeCurrent (glmWindow glmw, char const *funcName)
{
  winWindowData data = glmw->data;
  glmContext ctx = glmw->ctx;
  glmContext old_ctx = glmCurrentContext;

  PTRACE ((GLT_MASK|2, funcName, "winMakeCurrent %p", glmw));

  if ( ctx != old_ctx ) {
    winContextData cdata = ctx->data;
    PTRACE ((GLT_MASK|2, funcName, "wglMakeCurrent %p", ctx));

    if ( ! wglMakeCurrent (cdata->hDC, cdata->glctx) )
      return NULL;

    glmCurrentContext = ctx;
    gltCurrVTable = &cdata->glt_vtable;
  }
  
  return old_ctx;
}

static void
winRelease (glmWindow glmw, char const *funcName)
{
  int double_buffer = glmw->doubleBuffer;
  winWindowData data = glmw->data;
  glmContext ctx = glmw->ctx;
  winContextData cdata = ctx->data;

  gltCheckErrors (funcName);

  PTRACE ((GLT_MASK|2, funcName, "winRelease %p", glmw));

  if ( ! glmw->noflush ) {

    if (double_buffer)
      SwapBuffers (cdata->hDC);
    else
      glFlush ();

  } else
    glmw->noflush = FALSE;  /* "noflush" only works one time */

  PTRACE ((GLT_MASK|3, funcName, "wglMakeCurrent (NULL, NULL)"));
  wglMakeCurrent ( NULL, NULL );
  glmCurrentContext = NULL;
}

static void
winRegisterMapNotify (glmWindow glmw, glmMapNotifyFunc notify, void *closure)
{
  static const char funcName[] = "winRegisterMapNotify";
  winWindowData data = glmw->data;
  
  PTRACE ((GLT_MASK|1, funcName, "call %p(%p, %p)", notify, glmw, closure));
  data->mapNotify = notify;
  data->mapNotifyClosure = closure;
}

static int
winIsMapped (glmWindow glmw)
{
  return TRUE;
  /* return glmw->mapped; */
}

static glmRedrawFunc
winRegisterRedraw (glmWindow glmw, glmRedrawFunc redraw, void *closure)
{
  static char const *funcName = "winRegisterRedraw";
  winWindowData data = glmw->data;

  glmRedrawFunc old = data->redraw;

  PTRACE ((GLT_MASK|1, funcName, "glwm %p (redraw %p closure %p)", 
	   glmw, redraw, closure));

  data->redraw = redraw;
  data->redraw_closure = closure;

  return old;
}

static void
winWorkProc( )
{
  static char const *funcName = "winWorkProc";

  if ( redrawQ == NULL || list_empty (redrawQ) ) {
    PTRACE((GLT_MASK|2, funcName, "no redraws pending"));
    return;
  }

  {
    glmWindow glmw = (glmWindow) list_pop (redrawQ);
    winWindowData data = glmw->data;

    PR_Lock (data->lock); {
      data->pendingRedraw = 0;
      PTRACE((GLT_MASK|2, funcName, "D=%d P=%d",
	      data->destroying, data->pendingRedraw));
    }; PR_Unlock (data->lock);

    if ( TRUE  ) {

      PTRACE((GLT_MASK|2, funcName, "lock redraw"));
      PR_Lock (data->redraw_lock); {
	PTRACE((GLT_MASK|1, funcName, "call %p (%p)",
		data->redraw, data->redraw_closure));

	(*data->redraw) (glmw, data->redraw_closure);

	PTRACE((GLT_MASK|2, funcName, "unlock redraw"));
      }; PR_Unlock (data->redraw_lock);

    } else {
      PTRACE((GLT_MASK|2, funcName, "unmanaged window"));
      winRemoveRedraw (glmw);
    }
  }
}

static void
winRemoveRedraw1 (glmWindow glmw)
{
  static char const *funcName = "winRemoveRedraw1";
  winWindowData data = glmw->data;

  /* glmw->lock *must* be held on entry. */
  /*assert (pthread_mutex_trylock (&glmw->lock) == EBUSY);*/

  PTRACE((GLT_MASK|2, funcName, "D=%d P=%d",
	  data->destroying, data->pendingRedraw));

  if (data->pendingRedraw) {
    list_remove (redrawQ, (void *)glmw); /* take it off the queue */
    data->pendingRedraw = 0;

    PTRACE((GLT_MASK|2, funcName, "D=%d P=%d",
	    data->destroying, data->pendingRedraw));
  }
}

static void
winRemoveRedraw (glmWindow glmw)
{
  static char const *funcName = "winRemoveRedraw";
  winWindowData data = glmw->data;
  
  PR_Lock (data->lock); {
    PTRACE((GLT_MASK|2, funcName, "D=%d P=%d",
	    data->destroying, data->pendingRedraw));

    if (data->pendingRedraw || glm_timer )
      winRemoveRedraw1 (glmw);

  }; PR_Unlock (data->lock);
}

static void
winPostRedraw (glmWindow glmw)
{
  static char const *funcName = "winPostRedraw";
  winWindowData data = glmw->data;

  PR_Lock (data->lock); {
    PTRACE((GLT_MASK|1, funcName, "D=%d P=%d",
	    data->destroying, data->pendingRedraw));

    if (! data->destroying && ! data->pendingRedraw) {
      /* pendingRedraw implies on list, therefore we can unconditionally
	 shift onto the list here. */

      PTRACE((GLT_MASK|2, funcName, "list_shift (%p)", glmw ));

      list_shift (redrawQ, (void *)glmw);
      data->pendingRedraw = 1;
    }
  }; PR_Unlock (data->lock);
}

static int
winPendingRedraw (glmWindow glmw)
{
  winWindowData data = glmw->data;
  return data->pendingRedraw;
}

typedef struct {
  int   height;
  int   weight;
  int   italics;
  char  *name;
  HFONT id;
  int   base;
} font;

font fonts[] = {
  { 8, FW_NORMAL, FALSE, "courier", 0 },
  { 8, FW_NORMAL, TRUE, "courier", 0 },
  { 8, FW_BOLD, FALSE, "courier", 0 },
  { 8, FW_BOLD, TRUE, "courier", 0 },
};

static void
winInitCharstr (glmWindow glmw, int fidx)
{
  winWindowData data = glmw->data;
  winContextData cdata = glmw->ctx->data;

  fonts[fidx].base = glGenLists (256);

  fonts[fidx].id =
    CreateFont( fonts[fidx].height, 0, 0, 0, fonts[fidx].weight,
		fonts[fidx].italics, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DRAFT_QUALITY,
		DEFAULT_PITCH, fonts[fidx].name );

  if ( fonts[fidx].base == 0 || fonts[fidx].id == 0 ) return;
  
  SelectObject( cdata->hDC, fonts[fidx].id );
  
  wglUseFontBitmaps( cdata->hDC, 0, 255, fonts[fidx].base );
}

static void
winCharstr (glmWindow glmw, int fidx, char const *str)
{
  if ( fidx < 0 || fidx > 3 ) return;

  if ( ! fonts[fidx].id )
    winInitCharstr( glmw, fidx );

  glListBase( fonts[fidx].base );
  glCallLists( strlen( str ), GL_UNSIGNED_BYTE, str );
}

static PRLock *timer_lock = NULL;
static list_t *timer_list = NULL;

typedef struct {
  void           *loc;
  PRIntervalTime action_time;
  void           (*f) (void *closure);
  void           *closure;
} TimerClosure;

static void
winTimerHandler( void )
{
  TimerClosure *tc;
  list_iter_t  i;
  list_t       *del_list = NULL;

  if ( timer_lock == NULL || timer_list == NULL ) return;

  /* pop off all of the timers that have expired onto del_list */

  PR_Lock( timer_lock ); {
    
    for ( i = list_first( timer_list); ! list_done( i ); ) {
      tc = list_value( i );
            
      /* Zero the loc to claim the timer.  Don't process if zero. */
      if ( (int) ( tc->action_time - PR_IntervalNow() ) <= 0 ) {
	if (! PR_AtomicSet ((PRInt32 *) tc->loc, 0)) {
	  i = list_delete (i);
	} else {
	  if ( ! del_list ) del_list = list_create( FALSE );
	  list_push( del_list, tc );
	  i = list_delete( i );
	}
      } else {
	i = list_next( i );
      }
    }

  } PR_Unlock( timer_lock );

  /* go through del_list and call each callback, then destroy the event */

  if ( del_list ) {
    for ( i = list_first( del_list); ! list_done( i ); ) {
      tc = list_value( i );
      (*tc->f) (tc->closure);
      i = list_delete( i );
      free (tc);
    }

    list_destroy( del_list );
  }
}

static void
winAddTimer (glmWindow glmw, void **loc, unsigned int msecs, 
	     void (*f) (void *closure), void *f_closure)
{
  PRUint32 ticks_per_msec = PR_TicksPerSecond() / 1000;
  winWindowData data = glmw->data;

  TimerClosure *tc = malloc (sizeof (*tc));
  *loc = tc;
  if ( tc == NULL ) return;

  tc->loc          = loc;
  tc->f            = f;
  tc->closure      = f_closure;
  tc->action_time  = PR_IntervalNow() + msecs * ticks_per_msec;

  if ( ! timer_lock )
    timer_lock = PR_NewLock();

  if ( ! timer_list )
    timer_list = list_create( FALSE );

  PR_Lock( timer_lock ); {
    list_push( timer_list, tc );
  } PR_Unlock( timer_lock );
}

static void
winRemoveTimer (glmWindow glmw, void **loc)
{
  TimerClosure *tc;

  /* Clear the reference location to "take control" of tc.
     By doing this here, we allow a new timer handler to be re-established
     by the callback below. */
  assert (sizeof (void *) == sizeof (PRInt32));
  tc = (void *) PR_AtomicSet ((PRInt32 *) loc, 0);

  if (tc) {
    PR_Lock( timer_lock ); {
      if ( list_remove( timer_list, tc ) )
	free( tc );
    } PR_Unlock( timer_lock );
  }
}

static void *
winGetWidget (glmWindow glmw)
{
	winWindowData data = glmw->data;
	glmContext ctx = glmw->ctx;
	winContextData cdata = ctx->data;
	return data->hWnd;
}

void
glmWRunWorkProcs (void)
{
	winWorkProc ();
	winTimerHandler ();
}

static glmWindowVTable winWindowVTable = {
  winGetWidget,

  winRegisterRedraw,
  winRemoveRedraw,
  winPostRedraw,
  winPendingRedraw,

  winRegisterMapNotify,
  winIsMapped,

  winMakeCurrent,
  winRelease,

  winAddTimer,
  winRemoveTimer,

  winCharstr,
  winDestroyWindow
};

static glmContextVTable winContextVTable = {
  winShareContext,
  winDestroyContext
};
