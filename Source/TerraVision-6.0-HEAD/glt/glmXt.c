/*
 * Name: GLMXT.C
 *
 * Description: OpenGL/GLX/Xt helper functions.
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

#include <prlock.h>
#include <prthread.h>
#include <pthr/trace.h>

#include "glt.h"
#include "glmx.h"

#include <assert.h>
#include <unistd.h>

#include <pthr/list.h>

static glmContext xtCurrentContext;

static glmWindowVTable xtWindowVTable;

/* queue to keep track of glm's that need redraws */
static PRLock *redrawLock = 0;
static list_t *redrawQ = NULL;
static XtWorkProcId workProc = 0;

static XtInputId wakeupID;
static int workPipe[2];

/*
  These two functions handle the workPipe, which is provides a second input
  stream inside the XtAppNextEvent() select so that we can wake up the event
  processing thread if needed.

  The problem is that when we add a new WorkProc, the main loop may be waiting
  inside XtAppNextEvent() for input from the X protocol stream. Unfortunately,
  neither XtAppAddWorkProc() and XtAppAddTimeOut() will interrupt the select()
  in XtAppNextEvent().  So we provide a single byte of input which we also
  handle and then XtAppNextEvent() notices that there is a new WorkProc
  registered.

  Sigh -- leei
*/

static void
xtWakeupCB (XtPointer cl, int *src, XtInputId *id)
{
  static char const funcName[] = "xtWakeupCB";
  char v;
  PTRACE((GLT_MASK|2, funcName,
	  "id %d: read from %d (%d)", *id, workPipe[0], src[0]));
  read (src[0], &v, 1);
}

static void
xtWakeup (void)
{
  static char const funcName[] = "xtWakeup";
  char v = 1;
  PTRACE((GLT_MASK|2, funcName, "write to %d", workPipe[1]));
  write (workPipe[1], &v, 1);
}

static void
xtInit (Widget w)
{
  static char const funcName[] = "xtInit";

  redrawLock = PR_NewLock ();
  redrawQ = list_create (FALSE);

  pipe (workPipe);
  wakeupID = XtAppAddInput (XtWidgetToApplicationContext(w),
			    workPipe[0], (XtPointer) XtInputReadMask,
			    xtWakeupCB, 0);
  PTRACE((GLT_MASK|2, funcName, "workPipe[%d, %d] wakeupID %d",
	  workPipe[0], workPipe[1], wakeupID));
}

/*
 * In this section, we define a structure and functions used to manage OpenGL
 * rendering operations in a multi-threaded Xt/Motif application.  Because
 * OpenGL uses the X communications path to query the GLX server, most OpenGL
 * rendering and definitions must be enclosed in XtWorkProc's inside the
 * applications Xt event loop.  The following structures and functions are
 * used to control this process in a reasonably transparent fashion.
 */

/* The structure of a GLX rendering window and XtWorkProc management unit. */
typedef struct {
  PRThread *thread;
  PRLock * lock;
  PRLock * redraw_lock;
  Widget widget;

  Boolean pendingRedraw;

  glmRedrawFunc redraw;
  void *redraw_closure;

  short mapped;
  short destroying;
  glmMapNotifyFunc mapNotify;
  void *mapNotifyClosure;
} xtWindowData;

static void xtTrackMapped (Widget w, XtPointer closure,
			   XEvent *xev, Boolean *dispatch);
static void xtRemoveRedraw (glmWindow glmw);
static void xtRemoveRedraw1 (glmWindow glmw);

/*
 * Synopsis: glmWindow glmXtCreateWindow (Widget widget,
 *                                        glmContext ctx,
 *                                        Boolean double_buffer)
 *
 * Description:  Creates and initializes a glmWindow structure which has its
 *               own glmContext (shared with ctx).  The context is bound to
 *               the given Widget and is optionally double buffered.
 *
 * Externals: None.
 *
 * Returns: The glmWindow created.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

glmWindow
glmXtCreateWindow (Widget widget, glmContext ctx, 
		   int doubleBuffer, int stereo)
{
  static char const *funcName = "glmXtCreateWindow";

  glmWindow glmw = calloc (1, sizeof (*glmw));
  xtWindowData *data = calloc (1, sizeof (*data));

  if (redrawQ == NULL)
    xtInit (widget);

  PTRACE ((GLT_MASK|1, funcName, "glwm %p (widget %p \"%s\")", 
	   glmw, widget, XtName (widget)));

  data->thread = PR_GetCurrentThread ();
  data->lock = PR_NewLock ();
  data->redraw_lock = PR_NewLock ();

  data->widget = widget;
  data->destroying = FALSE;

  glmw->vtable = &xtWindowVTable;
  glmw->data = data;
  glmw->ctx = xShareContext (ctx);
  glmw->doubleBuffer = doubleBuffer ? ctx->doubleBuffer : FALSE;
  glmw->stereo = stereo ? ctx->stereo : FALSE;
  glmw->noflush = FALSE;

  /* Only Shell widgets get StructureNotify events. */
  for ( ; ! XtIsShell (widget); widget = XtParent (widget));
  PTRACE ((GLT_MASK|2, funcName, "XtAddEventHandler %p (%p, %p)",
	   widget, xtTrackMapped, glmw));
  XtAddEventHandler (widget, SubstructureNotifyMask, True,
		     xtTrackMapped, glmw);

  return glmw;
}

/*
 * Synopsis: void glmDestroyWindow (glmWindow glmw)
 *
 * Description:  Destroys a previously created glmWindow.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: June 1998
 *
 */

static void
xtDestroyWindow (glmWindow glmw)
{
  static char const funcName[] = "xtDestroyWindow";
  xtWindowData *data = glmw->data;
  Widget widget = data->widget;

  if (data->thread != PR_GetCurrentThread ()) {
    fprintf (stderr, "glmDestroyWindow called from wrong thread\n");
    abort ();
  }

  PR_Lock (data->lock); {
    PTRACE ((GLT_MASK|1, funcName, "%p", glmw));
    PTRINC ((GLT_MASK|1, 1));
    data->destroying = TRUE;
    PTRACE ((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
	     widget, XtName (widget),
	     data->destroying, data->pendingRedraw));
    if (data->pendingRedraw || workProc)
      xtRemoveRedraw1 (glmw);

    /* Only Shell widgets get StructureNotify events. */
    for ( ; ! XtIsShell (widget); widget = XtParent (widget));
    PTRACE ((GLT_MASK|2, funcName, "XtRemoveEventHandler %p (%p, %p)",
	     widget, xtTrackMapped, glmw));
    XtRemoveEventHandler (widget, SubstructureNotifyMask, True,
			  xtTrackMapped, glmw);
    data->mapNotify = 0;

    glmDestroyContext (glmw->ctx);
    PR_DestroyLock (data->redraw_lock);
  }; PR_Unlock (data->lock); 

  PR_DestroyLock (data->lock);
  free (data);
  free (glmw);
  PTRINC ((GLT_MASK|1, -1));
}

void *
xtGetWidget (glmWindow glmw)
{
  xtWindowData *data = glmw->data;
  return data->widget;
}

/*
 * Synopsis: glmWindow glmMakeCurrent (char const *funcName,
 *                                     glmContext ctx)
 *
 * Description: Bind the glmContext and thus enable OpenGL rendering.
 *
 * Externals: The glm_lock for serializing OpenGL access.
 *
 * Returns: The previously bound glmWindow.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static glmContext
xtMakeCurrent (glmWindow glmw, char const *funcName)
{
  xtWindowData *wdata = glmw->data;
  glmContext ctx = glmw->ctx;
  xContextData *cdata = ctx->data;

  glmContext old_ctx = xtCurrentContext;
  GLXDrawable drawable = XtWindow (wdata->widget);

  if (wdata->thread != PR_GetCurrentThread ()) {
    fprintf (stderr, "glmMakeCurrent called from wrong thread\n");
    abort ();
  }

  if (ctx != old_ctx) {
    PTRACE ((GLT_MASK|2, funcName, 
	     "xtMakeCurrent %s, ctx=%p", XtName(wdata->widget), ctx));
    if (! glXMakeCurrent (cdata->disp, drawable, cdata->glxCtx))
      return 0;
    cdata->drawable = drawable;

    xtCurrentContext = ctx;
    gltCurrVTable = cdata->vtable;
  }

  return old_ctx;
}

/*
 * Synopsis: void glmRelease (char const *funcName, glmContext ctx)
 *
 * Description:  Release the OpenGL rendering binding from the glmContext..
 *
 * Externals: The glm_lock for serializing OpenGL access.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static void
xtRelease (glmWindow glmw, char const *funcName)
{
  xtWindowData *data = glmw->data;
  Widget w = data->widget;
  xContextData *cdata = glmw->ctx->data;

  if (data->thread != PR_GetCurrentThread ()) {
    fprintf (stderr, "glmRelease called from wrong thread\n");
    abort ();
  }

  gltCheckErrors (funcName);

  PTRACE ((GLT_MASK|2, funcName, "xtRelease %s", XtName(w)));

  if ( ! glmw->noflush ) {

    if (glmw->doubleBuffer)
      glXSwapBuffers (cdata->disp, cdata->drawable);
    else
      glFlush ();

  } else
    glmw->noflush = FALSE;  /* "noflush" only works one time */


  if (0) {
    PTRACE ((GLT_MASK|3, funcName, "glXMakeCurrent (..., None, NULL)"));
    glXMakeCurrent (cdata->disp, None, NULL);
  }
}

/*
 * Synopsis: void xtRegisterMapNotify (glmWindow glmw,
 *                                      glmMapNotifyFunc notify,
 *                                      void *closure)
 *
 * Description:  Register a function to be called when the map status changes.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: Feb. 1999
 *
 */

static void
xtRegisterMapNotify (glmWindow glmw, glmMapNotifyFunc notify, void *closure)
{
  static const char funcName[] = "xtRegisterMapNotify";
  xtWindowData *data = glmw->data;
  
  PTRACE ((GLT_MASK|1, funcName, "call %p(%p, %p)", notify, glmw, closure));
  data->mapNotify = notify;
  data->mapNotifyClosure = closure;
}

/*
 * Synopsis: void xtTrackMapped (Widget w,
 *                                XtPointer closure,
 *                                XEvent *xev,
 *                                Boolean *dispatch)
 *
 * Description:  Tracks the status of the glmWindow in terms of X mapping.  In
 *               general, a mapped widget is visible and active.
 *
 * Externals: None.
 *
 * Returns: Nothing
 *
 * Author: Lee Iverson
 *
 * Date: June 1998
 *
 */

static void
xtTrackMapped (Widget w, XtPointer closure,
	       XEvent *xev, Boolean *dispatch)
{
  static char const funcName[] = "xtTrackMapped";
  glmWindow glmw = (glmWindow) closure;
  xtWindowData *data = glmw->data;
  int old = data->mapped;

  PTRACE ((GLT_MASK|2, funcName, "glwm %p (widget %p \"%s\") %d", 
	   glmw, w, XtName (w), xev->type));

  switch (xev->type) {
  case MapNotify:
    data->mapped = TRUE;
    break;
  case UnmapNotify:
    data->mapped = FALSE;
    break;
  }

  PTRACE ((GLT_MASK|2, funcName, "glmw %p: %s",
	   glmw, data->mapped? "Mapped" : "Unmapped"));
  if (data->mapNotify && data->mapped != old) {
    PTRACE ((GLT_MASK|1, funcName, "call mapNotifyClosure %p(%p, %p)",
	     data->mapNotify, glmw, data->mapNotifyClosure));
    (*data->mapNotify) (glmw, data->mapNotifyClosure);
  }
}

/*
 * Synopsis: void glmIsMapped (glmWindow glmw)
 *
 * Description: Returns true if window is currently mapped.
 *
 * Externals: None.
 *
 * Returns: Mapped state of window.
 *
 * Author: Lee Iverson
 *
 * Date: June 1998
 *
 */

static int
xtIsMapped (glmWindow glmw)
{
  xtWindowData *data = glmw->data;
  return data->mapped;
}

/*
 * Synopsis: glmRedrawFunc xtRegisterRedraw (glmWindow glmw,
 *                                           glmRedrawFunc redraw,
 *                                           void *closure)
 *
 * Description:  Registers a redraw function to be called inside the
 *               XtWorkProc after glmPostRedraw() is called.  The function is
 *               then called with two arguments, the glmWindow and the closure
 *               value provided at registration.
 *
 * Externals: None.
 *
 * Returns: The previously registered redraw function.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static glmRedrawFunc
xtRegisterRedraw (glmWindow glmw, glmRedrawFunc redraw, void *closure)
{
  static char const *funcName = "glmRegisterRedraw";
  xtWindowData *data = glmw->data;
  
  glmRedrawFunc old = data->redraw;

  PTRACE ((GLT_MASK|1, funcName, "glwm %s (redraw %p closure %p)", 
	   XtName (data->widget), redraw, closure));

  data->redraw = redraw;
  data->redraw_closure = closure;

  return old;
}

/*
 * Synopsis: static Boolean xtWorkProc (XtPointer ptr)
 *
 * Description:  An XtWorkProc used to redraw a glmWindow.
 *
 * Externals: glm_lock for locking access to the OpenGL pipeline.
 *
 * Returns: TRUE if the workProc is *not* retained.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static Boolean 
xtWorkProc (XtPointer ptr)
{
  static char const *funcName = "xtWorkProc";
  Boolean ret;

  PR_Lock (redrawLock);

  if (list_empty (redrawQ)) {
    PTRACE((GLT_MASK|2, funcName, "remove workproc %p.", workProc));
    workProc = 0;
    PR_Unlock (redrawLock);
    PTRACE((GLT_MASK|2, funcName, "return True."));
    return TRUE;
  }

  {
    /* pop one off the redraw queue */
    /* @@@ May be a race condition here! -- leei */
    glmWindow glmw = (glmWindow) list_pop (redrawQ);
    xtWindowData *data = glmw->data;
    PR_Unlock (redrawLock);
    
    PR_Lock (data->lock); {
      data->pendingRedraw = 0;
      PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
	      data->widget, XtName (data->widget),
	      data->destroying, data->pendingRedraw));
    }; PR_Unlock (data->lock);

    if (XtIsManaged (data->widget)) {
      PTRACE((GLT_MASK|2, funcName, "lock redraw"));
      PR_Lock (data->redraw_lock); {
	PTRACE((GLT_MASK|2, funcName, "call %p (%s, %p)",
		data->redraw, XtName (data->widget), data->redraw_closure));
	(*data->redraw) (glmw, data->redraw_closure);
	PTRACE((GLT_MASK|2, funcName, "unlock redraw"));
      }; PR_Unlock (data->redraw_lock);
    } else {
      PTRACE((GLT_MASK|2, funcName, "unmanaged window %p \"%s\"",
	      data->widget, XtName (data->widget)));
      xtRemoveRedraw (glmw);
    }
  }

  PR_Lock (redrawLock); {
    if (list_empty (redrawQ)) {
      PTRACE((GLT_MASK|2, funcName, "remove workproc %p.", workProc));
      workProc = 0;
      ret = TRUE;
    } else {
      PTRACE((GLT_MASK|2, funcName, "recall workproc %p.", workProc));
      ret = FALSE;
    }
  }; PR_Unlock (redrawLock);

  
  PTRACE((GLT_MASK|2, funcName, "return %s.", ret ? "True" : "False"));
  return ret;
}

/*
 * Synopsis: void xtRemoveRedraw (glmWindow glmw)
 *
 * Description:  Ask the glmWindow to *not* redraw.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static void
xtRemoveRedraw1 (glmWindow glmw)
{
  static char const *funcName = "glmRemoveRedraw1";
  xtWindowData *data = glmw->data;

  /* glmw->lock *must* be held on entry. */
  /*assert (pthread_mutex_trylock (&glmw->lock) == EBUSY);*/

  PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
	  data->widget, XtName (data->widget),
	  data->destroying, data->pendingRedraw));
  PR_Lock (data->redraw_lock); {
    PR_Lock (redrawLock); {
      if (data->pendingRedraw) {
	list_remove (redrawQ, (void *)glmw); /* take it off the queue */
	data->pendingRedraw = 0;
	PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
		data->widget, XtName (data->widget),
		data->destroying, data->pendingRedraw));
      }

      PTRACE((GLT_MASK|2, funcName, "lock redraw"));
      PTRACE((GLT_MASK|1, funcName, "(%p) widget %p \"%s\"",
	      glmw, data->widget, XtName (data->widget)));
      if (list_empty (redrawQ)) {
	PTRACE((GLT_MASK|2, funcName, "remove workProc %p", workProc));
	XtRemoveWorkProc (workProc);
	workProc = 0;
      }
      PTRACE((GLT_MASK|2, funcName, "unlock redraw"));
    }; PR_Unlock (redrawLock);
  }; PR_Unlock (data->redraw_lock);
}

static void
xtRemoveRedraw (glmWindow glmw)
{
  static char const *funcName = "glmRemoveRedraw";
  xtWindowData *data = glmw->data;

  PR_Lock (data->lock); {
    PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
	    data->widget, XtName (data->widget),
	    data->destroying, data->pendingRedraw));
    if (data->pendingRedraw || workProc)
      xtRemoveRedraw1 (glmw);
  }; PR_Unlock (data->lock);
}

/*
 * Synopsis: void xtPostRedraw (glmWindow glmw)
 *
 * Description:  Ask the glmWindow to redraw the next time it can inside the
 *               application's event loop.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: June 1997
 *
 */

static void
xtPostRedraw (glmWindow glmw)
{
  static char const funcName[] = "xtPostRedraw";
  xtWindowData *data = glmw->data;

  PR_Lock (data->lock); {
    PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
	    data->widget, XtName (data->widget),
	    data->destroying, data->pendingRedraw));
    
    if (! data->destroying && ! data->pendingRedraw) {
      /* pendingRedraw implies on list, therefore we can unconditionally
	 shift onto the list here. */
      PTRACE((GLT_MASK|2, funcName, "list_shift (%p \"%s\")",
	      glmw, XtName (data->widget)));
      PR_Lock (redrawLock); {
	list_shift (redrawQ, (void *)glmw);
	data->pendingRedraw = 1;
	PTRACE((GLT_MASK|2, funcName, "widget %p \"%s\" D=%d P=%d",
		data->widget, XtName (data->widget),
		data->destroying, data->pendingRedraw));

	if (! workProc) {
	  XtAppContext appContext =
	    XtWidgetToApplicationContext(data->widget);
	  workProc =
	    XtAppAddWorkProc (appContext, xtWorkProc, (XtPointer) glmw);
	  PTRACE((GLT_MASK|2, funcName, "add workProc %p", workProc));

	  /* Wake up the idle processing loop */
	  xtWakeup ();
	}
      }; PR_Unlock (redrawLock);
    }

  }; PR_Unlock (data->lock);
}

static int
xtPendingRedraw (glmWindow glmw)
{
  xtWindowData *data = glmw->data;
  return data->pendingRedraw;
}

typedef struct {
  XtIntervalId id;
  void (*f) (void *closure);
  void *closure;
} xtTimerClosure;

static void
xtTimerHandler (XtPointer closure, XtIntervalId *interval)
{
  void **loc = closure;
  xtTimerClosure *tc;

  /* Clear the reference location to "take control" of tc.
     By doing this here, we allow a new timer handler to be re-established
     by the callback below. */
  assert (sizeof (void *) == sizeof (PRInt32));
  tc = (void *) PR_AtomicSet ((PRInt32 *) loc, 0);

  if (tc) {
    /* Execute, and of course optionally restart. */
    (*tc->f) (tc->closure);

    /* Now free the pointer */
    free (tc);
  }
}

static void
xtAddTimer (glmWindow glmw, void **loc, unsigned int msecs, 
	    void (*f) (void *closure), void *closure)
{
  xtWindowData *data = glmw->data;
  XtAppContext appContext = XtWidgetToApplicationContext (data->widget);

  xtTimerClosure *tc = malloc (sizeof (*tc));
  *loc = tc;

  tc->f = f;
  tc->closure = closure;
  tc->id = XtAppAddTimeOut (appContext, msecs, xtTimerHandler, loc);
}

static void
xtRemoveTimer (glmWindow glmw, void **loc)
{
  xtTimerClosure *tc;

  /* Clear the reference location to "take control" of tc */
  assert (sizeof (void *) == sizeof (PRInt32));
  tc = (void *) PR_AtomicSet ((PRInt32 *) loc, 0);

  if (tc) {
    XtRemoveTimeOut (tc->id);

    /* Disable this pointer, then free */
    free (tc);
  }
}

typedef struct {
  char *name;
  Font id;
  int base;
} font;

font fonts[] = {
  {"-adobe-courier-medium-r-*-*-*-120-75-75-*-*-iso8859-1", 0, 0},
  {"-adobe-courier-medium-o-*-*-*-120-75-75-*-*-iso8859-1", 0, 0},
  {"-adobe-courier-bold-r-*-*-*-120-75-75-*-*-iso8859-1", 0, 0},
  {"-adobe-courier-bold-o-*-*-*-120-75-75-*-*-iso8859-1", 0, 0}
};

char *altfontnames[] = {
  "-misc-fixed-medium-r-*-*-*-120-75-75-*-*-iso8859-1",
  "-misc-fixed-medium-r-*-*-*-120-75-75-*-*-iso8859-1",
  "-misc-fixed-bold-r-*-*-*-120-75-75-*-*-iso8859-1",
  "-misc-fixed-bold-r-*-*-*-120-75-75-*-*-iso8859-1"
};

static void
xtInitCharstr (glmWindow glmw, int fidx)
{
  xtWindowData *data = glmw->data;
  Widget w = data->widget;
  XFontStruct *font = XLoadQueryFont (XtDisplay (w), fonts[fidx].name);

  /* if the default font does not exist in a large enough version   */
  /* then use an alternative fixed width font that should be there. */
  /* The ascent field of XFontStruct is the logical extent of the   */
  /* font above the baseline - we use this for our validity check.  */

  if ( font == NULL || font->ascent < 10 ) {
    fonts[fidx].name = altfontnames[fidx];
    font = XLoadQueryFont (XtDisplay (w), fonts[fidx].name);
  }

  fonts[fidx].id = font->fid;
  fonts[fidx].base = glGenLists (256);
  glXUseXFont (font->fid, 0, 256, fonts[fidx].base);
}

static void
xtCharstr (glmWindow glmw, int fidx, char const *str)
{
  if (! fonts[fidx].id)
    xtInitCharstr (glmw, fidx);

  glListBase (fonts[fidx].base);
  glCallLists (strlen (str), GL_UNSIGNED_BYTE, str);
}

static glmWindowVTable xtWindowVTable = {
  xtGetWidget,

  xtRegisterRedraw,
  xtRemoveRedraw,
  xtPostRedraw,
  xtPendingRedraw,

  xtRegisterMapNotify,
  xtIsMapped,

  xtMakeCurrent,
  xtRelease,

  xtAddTimer,
  xtRemoveTimer,

  xtCharstr,
  xtDestroyWindow
};
