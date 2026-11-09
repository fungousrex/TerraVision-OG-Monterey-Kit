/*
 * Name: glmTcl.C
 *
 * Description: Tcl/Tk/Togl helper functions.  We want to substitute any 
 *     GLXContext obtained through normal "glm/TVGLX" channels with the one
 *     derived by the Togl Tk-widget.
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

#include <stdio.h>
#include <stdlib.h>

#include <prlock.h>
#include <pthr/trace.h>
#include <pthr/list.h>
#include <TerraVision/Trace.h>

#include <libTogl/togl.h>
#include <libTogl/toglinternals.h>
#include <tcl.h>
#include <tk.h>

#include "glt.h"
#include "glm.h"
#include "glmimpl.h"

/* redraw update rate in milliseconds */
int REDRAW_RATE = 10;

/* forward decls... */
static int tclIsMapped(glmWindow glmw);

static glmContext currentGlmContext = NULL;
static glmContextVTable tclContextVTable;

/*
 * g l m T c l C r e a t e C o n t e x t ( )
 *
 */
glmContext
glmTclCreateContext(struct Togl* togl)
{
    static char funcName[] = "glmTclCreateContext";
    glmContext newCtx;

    PTRACE ((GLT_MASK|1, funcName, "(togl %p)", togl));

    newCtx = (glmContext)calloc(1, sizeof(*newCtx));
    newCtx->vtable = &tclContextVTable;
    newCtx->data = (void*)togl;
    newCtx->share = NULL;

#if 0 /* NOT ONLY IS THE FOLLOWING INCORRECT (wrt depths), BUT ALSO PROPS ARE
       * APPARENTLY ONLY USED IN TRYING TO *FIND* SUITABLE VISUALS, AND TOGL
       * HAS ALREADY DONE THAT FOR US HERE   --RLB
       */
    /* get the properties from the Togl widget... */
    newCtx->props.id = 0;   /* NOT SURE WHAT THIS SHOULD BE */
    newCtx->props.overlay = ToglIsOverlaySupported(togl);
    newCtx->props.underlay = 0;  /*  NOT SURE ABOUT THIS ONE --RLB */
    newCtx->props.colorIndex = ! ToglIsRGBA(togl);
    newCtx->props.singleBuffer = !ToglIsDoubleBuffered(togl);
    newCtx->props.stereo = ToglIsStereo(togl);
    newCtx->props.slow = 0;   /* HOW TO TELL? */
    newCtx->props.colorDepth =  8 ; /**** ToglGetColorDepth(togl);***/
    newCtx->props.zbufDepth = 16 ; /**** ToglGetZDepth(togl); ****/
    newCtx->props.stencilDepth = ToglGetStencilDepth(togl);
    newCtx->props.multiBufs = 0;  /* HOW TO TELL? */
    newCtx->props.multiBufSamples = 0; /* DITTO */
#endif

    newCtx->doubleBuffer = ToglIsDoubleBuffered(togl);
    newCtx->stereo = ToglIsStereo(togl);

    return newCtx;
}

/*
 * t c l C r e a t e S h a r e d C o n t e x t ( )
 *
 */
glmContext 
tclCreateSharedContext(glmContext ctx)
{
    const char *funcName = "tclCreateSharedContext";
    TSMTRACE(( GLT_MASK|1, funcName, "Entering" ));
    return ctx;   /*  NOT RIGHT -- PLACE HOLDER */
}

/*
 * t c l D e s t r o y C o n t e x t ( ) 
 * 
 */
void
tclDestroyContext(glmContext ctx)
{
    const char *funcName = "tclDestroyContext";
    TSMTRACE(( GLT_MASK|1, funcName, "Entering" ));
}

static glmContextVTable tclContextVTable = {
    tclCreateSharedContext,
    tclDestroyContext
};



/*
 * Fonts...
 *
 */

typedef struct {
    char *name;
    int  isLoaded;
    int  base;
} font;

#define NUMFONTS 4

font initFonts[NUMFONTS] = {
    {TOGL_BITMAP_HELVETICA_10, 0, 0},
    {TOGL_BITMAP_8_BY_13, 0, 0},
    {TOGL_BITMAP_HELVETICA_12, 0, 0},
    {TOGL_BITMAP_9_BY_15, 0, 0}
};


/*
 * glmWindow...
 *
 */
static glmWindowVTable tclWindowVTable;


typedef struct tclWindowData {
    PRLock * lock;
    PRLock * redraw_lock;
    int pendingRedraw;
    glmRedrawFunc redraw;
    void *redraw_closure;
    short mapped;
    short destroying;
    glmMapNotifyFunc mapNotify;
    void *mapNotifyClosure;
    struct Togl* togl;
    font fonts[NUMFONTS];
} tclWindowData;


/* queue to keep track of glm's that need redraws */
static PRLock *redrawLock = 0;
static list_t *redrawQ = NULL;
static Tcl_TimerToken timer = NULL;

/*
 * g l m T c l C r e a t e W i n d o w ( )
 * 
 */
glmWindow
glmTclCreateWindow(struct Togl* togl, glmContext ctx, int doubleBuffer, 
		   int stereo)
{
  static char const *funcName = "glmTclCreateWindow";
  int i;

  glmWindow glmw = calloc(1, sizeof(*glmw));
  tclWindowData *data = calloc(1, sizeof(tclWindowData));

  if (redrawQ == NULL) {
      redrawQ = list_create(TRUE);
      redrawLock = PR_NewLock();
  }

  PTRACE ((GLT_MASK|1, funcName, "glwm %p ctx %p (togl %p)", 
	   glmw, ctx, togl));

  data->lock = PR_NewLock();
  data->redraw_lock = PR_NewLock();
  data->pendingRedraw = 0;
  data->destroying = FALSE;

  data->togl = togl;

  glmw->vtable = &tclWindowVTable;
  glmw->data = data;
  glmw->ctx = ctx;  /**RLB??? xShareContext(ctx); */
  glmw->doubleBuffer = doubleBuffer ? ctx->doubleBuffer : FALSE;
  glmw->stereo = stereo ? ctx->stereo : FALSE;
  glmw->noflush = FALSE;

  for (i=0; i<NUMFONTS; i++)
      data->fonts[i] = initFonts[i];

  return glmw;
}


/*
 * t c l G e t W i d g e t ( )
 *
 */
void*
tclGetWidget (glmWindow glmw)
{
  static char const funcName[] = "tclGetWidget";
  tclWindowData* data = (tclWindowData*)glmw->data;
  PTRACE ((GLT_MASK|1, funcName,"%p <= %p", glmw, data->togl));
  return data->togl;
}

/*
 * t c l T i m e r P r o c ( )
 *
 */
void
tclTimerProc(ClientData c)
{
    static char const *funcName = "tclTimerProc";

    TSMTRACE(( GLT_MASK|1, funcName, "Entering" ));

    PR_Lock (redrawLock);

    if (list_empty (redrawQ)) {
        PTRACE((GLT_MASK|2, funcName, "remove timerproc"));
        timer = NULL;
        PR_Unlock (redrawLock);
        return;
    }

    {
        /* pop one off the redraw queue */
        /* @@@ May be a race condition here! -- leei */
        glmWindow glmw = (glmWindow) list_pop (redrawQ);
        tclWindowData *data = (tclWindowData*)glmw->data;
        PR_Unlock (redrawLock);
    
        PR_Lock (data->lock); {
            data->pendingRedraw = 0;
        }; PR_Unlock (data->lock);

        /* we have no apparent way to tell whether the underlying TkWin is 
         * visible (i.e,, managed)
         */	
        PTRACE((GLT_MASK|2, funcName, "lock redraw"));
        PR_Lock (data->redraw_lock); {
            PTRACE((GLT_MASK|1, funcName, "redraw call %p (%p, %p)",
                data->redraw, glmw, data->redraw_closure));
            (*data->redraw) (glmw, data->redraw_closure);
        }; PR_Unlock (data->redraw_lock);
    }

    PR_Lock (redrawLock); {
        if (list_empty (redrawQ)) {
            PTRACE((GLT_MASK|2, funcName, "remove timerproc"));
            timer = NULL;
        } else {
            PTRACE((GLT_MASK|2, funcName, "reinstate timerproc"));
            timer = Tcl_CreateTimerHandler(REDRAW_RATE, tclTimerProc, NULL);
        }
    }; PR_Unlock (redrawLock);

}


/*
 * t c l R e g i s t e r R e d r a w ( ) 
 *
 */
static glmRedrawFunc
tclRegisterRedraw(glmWindow glmw, glmRedrawFunc func, void* closure)
{
    static char const funcName[] = "tclRegisterRedraw";
    tclWindowData* data = (tclWindowData*)glmw->data;
    glmRedrawFunc prevFunc = data->redraw;

    TSMTRACE ((GLT_MASK|1, funcName, "%p -> (*%p)(%p)", glmw, func, closure));

    data->redraw = func;
    data->redraw_closure = closure;
    return prevFunc;
}

/*
 * g l m T c l R e d r a w D i s p a t c h ( ) 
 *
 * Called from Tk idleProc in response to a PostRedraw.
 */
void
glmTclRedrawDispatch(ClientData data)
//***glmTclRedrawDispatch(glmWindow glmw, void* closure)
{
    glmWindow glmw = (glmWindow)data;
    tclWindowData* window = (tclWindowData*)glmw->data;
    (*window->redraw)(glmw, window->redraw_closure);
}

/*
 * t c l R e m o v e R e d r a w ( ) 
 *
 */
static void
tclRemoveRedraw(glmWindow glmw)
{
    static char const *funcName = "tclRemoveRedraw";
    tclWindowData *data = (tclWindowData*)glmw->data;

    TSMTRACE(( GLT_MASK|1, funcName, "Entering" )); 

    PR_Lock (data->lock); {
        PTRACE((GLT_MASK|2, funcName, " %p D=%d P=%d",
            glmw, data->destroying, data->pendingRedraw));

        if (data->pendingRedraw) {
            PR_Lock (data->redraw_lock); {
                PR_Lock (redrawLock); {

                    /* take our glmWindow off the redraw queue... */
                    list_remove (redrawQ, (void *)glmw);
                    data->pendingRedraw = 0;
                    PTRACE((GLT_MASK|2, funcName, "redraw cancelled %p",
                        glmw));

                    /* if list now empty, cancel timerProc... */
                    if (list_empty (redrawQ)) {
                        PTRACE((GLT_MASK|2, funcName, "timerproc cancelled"));
                        Tcl_DeleteTimerHandler(timer);
                        timer = NULL;
                    }
                }; PR_Unlock (redrawLock);
            }; PR_Unlock (data->redraw_lock);
        }
    }; PR_Unlock (data->lock);
}

/*
 * t c l P o s t R e d r a w ( ) 
 *
 */
static void
tclPostRedraw(glmWindow glmw)
{
    static char const funcName[] = "tclPostRedraw";
    tclWindowData *data = (tclWindowData*)glmw->data;

    TSMTRACE(( GLT_MASK|1, funcName, "Entering" ));

    if (!tclIsMapped(glmw)) /* PROBABLY WANT THIS LOCKED FIRST? --RLB*/
      return;

    PR_Lock (data->lock); {
        PTRACE((GLT_MASK|1, funcName, "glmw %p D=%d P=%d",
	    glmw, data->destroying, data->pendingRedraw));
    
        if (! data->destroying && ! data->pendingRedraw) {
            /* pendingRedraw implies on list, therefore we can unconditionally
               shift onto the list here. */
            PTRACE((GLT_MASK|2, funcName, "list_shift (%p)", glmw));
            PR_Lock (redrawLock); {
                list_shift (redrawQ, (void *)glmw);
                data->pendingRedraw = 1;

#if 1 // THIS IS BUSTED?  --RLB
                if (!timer) {
                    timer = Tcl_CreateTimerHandler(REDRAW_RATE, tclTimerProc, 
                        NULL);
                    PTRACE((GLT_MASK|2, funcName, "timer: (%p)", timer));
                }
#else // TRY THIS INSTEAD --RLB
                Tcl_DoWhenIdle(glmTclRedrawDispatch, (ClientData)glmw);
#endif
            }; PR_Unlock (redrawLock);
        }

    }; PR_Unlock (data->lock);

}

/*
 * t c l P e n d i n g R e d r a w ( ) 
 * 
 */
static int
tclPendingRedraw(glmWindow glmw)
{
    static char const funcName[] = "tclPendingRedraw";
    tclWindowData* data = (tclWindowData*)glmw->data;

    PTRACE ((GLT_MASK|1, funcName, "%p <= %d", glmw, data->pendingRedraw));

    return data->pendingRedraw; 
}

/*
 * t c l R e g i s t e r M a p N o t i f y ( )
 *
 */
static void
tclRegisterMapNotify(glmWindow glmw, glmMapNotifyFunc func, void* closure)
{
    static char const funcName[] = "tclRegisterMapNotify";
    tclWindowData* data = (tclWindowData*)glmw->data;
    data->mapNotify = func;
    data->mapNotifyClosure = closure;
    TSMTRACE ((GLT_MASK|1, funcName, "%p -> (*%p)(%p)", glmw, func, closure));
}

/*
 * g l m T c l M a p N o t i f y D i s p a t c h ( )
 *
 */
void
glmTclMapNotifyDispatch(glmWindow glmw, void* closure)
{
    static char const funcName[] = "glmTclMapNotifyDispatch";
    tclWindowData* data = (tclWindowData*)glmw->data;
    PTRACE ((GLT_MASK|1, funcName, "(*%p)(%p, %p)",
        data->mapNotify, glmw, closure));
    (*data->mapNotify)(glmw, closure);
}

/*
 * t c l I s M a p p e d ( )
 *
 */
static int
tclIsMapped(glmWindow glmw)
{
    static char const funcName[] = "tclIsMapped";
    tclWindowData* data = (tclWindowData*)glmw->data;

    PTRACE ((GLT_MASK|1, funcName, "%p <= %d", 
        glmw, Tk_IsMapped(Togl_TkWin(data->togl))));

    return Tk_IsMapped(Togl_TkWin(data->togl));
}

/*
 * t c l M a k e C u r r e n t ( ) 
 *
 */
static glmContext
tclMakeCurrent(glmWindow glmw, char const* funcName)
{
    glmContext prevContext = currentGlmContext;
    tclWindowData* data = (tclWindowData*)glmw->data;
    Togl_MakeCurrent(data->togl);

    TSMTRACE ((GLT_MASK|1, funcName, "tclMakeCurrent %p", glmw));
    currentGlmContext = glmw->ctx;
    return prevContext;
}

/*
 * t c l R e l e a s e ( )
 *
 */
static void
tclRelease(glmWindow glmw, char const* funcName)
{
    tclWindowData* data;

    TSMTRACE ((GLT_MASK|1, funcName, "tclRelease %p", glmw));

    if ( glmw->noflush ) {
      glmw->noflush = FALSE;  /* "noflush" only works one time */
      return;
    }

    if (glmw->doubleBuffer) {
        data = (tclWindowData*)glmw->data;
        Togl_SwapBuffers(data->togl);
    }
    else
        glFlush();
}


typedef struct TclTimerClosure {
    Tcl_TimerToken  id;
    void (*f) (void* closure);
    void* closure;
} TclTimerClosure;


/*
 * t c l T i m e r D i s p a t c h ( )
 *
 */
static void
tclTimerDispatch(ClientData closure)
{
    const char *funcName = "tclTimerDispatch";
    void** loc = closure;
    TclTimerClosure* tc;

    TSMTRACE(( GLT_MASK|1, funcName, "Entering" )); 

    /* Clear the reference location to "take control" of tc.
       By doing this here, we allow a new timer handler to be re-established
       by the callback below. */
//***    assert (sizeof (void *) == sizeof (PRInt32));
    tc = (TclTimerClosure*) PR_AtomicSet ((PRInt32 *) loc, 0);

    if (tc) {
        /* Execute, and of course optionally restart. */
        (*tc->f) (tc->closure);
        
        /* Now free the pointer */
        free (tc);
    }
}

/*
 * t c l A d d T i m e r ( )
 *
 */
static void
tclAddTimer(glmWindow glmw, void** id, unsigned int msecs, 
	    void (*f)(void *closure), void* closure)
{
    static char const funcName[] = "tclAddTimer";
    TclTimerClosure* tc = (TclTimerClosure*)malloc(sizeof(TclTimerClosure));

    PTRACE ((GLT_MASK|1, funcName, "%p:%p %dms => (*%p) (%p)",
	   glmw, id, msecs, f, closure));

    *id = tc;
    TSMTRACE(( GLT_MASK|1, funcName, "Entering" ));

    tc->f = f;
    tc->closure = closure;
    tc->id = Tcl_CreateTimerHandler(msecs, tclTimerDispatch, (ClientData)id);
}

/*
 * t c l R e m o v e T i m e r ( )
 *
 */
static void
tclRemoveTimer(glmWindow glmw, void** id)
{
  static char const funcName[] = "tclRemoveTimer";
  TclTimerClosure* tc;

  PTRACE ((GLT_MASK|1, funcName, "%p:%p", glmw, id));

  tc = (void*)PR_AtomicSet((PRInt32*)id, 0);
  if (tc) {
      Tcl_DeleteTimerHandler(tc->id);
      free(tc);
  }
}


/*
 * t c l C h a r S t r ( )
 *
 */
static void
tclCharStr(glmWindow glmw, int fidx, char const* str)
{
    tclWindowData* data = (tclWindowData*)glmw->data;

    if (fidx < 0 || fidx > 3) return;

    if (!data->fonts[fidx].isLoaded) {
        data->fonts[fidx].base = 
            Togl_LoadBitmapFont(data->togl, data->fonts[fidx].name);
        if (data->fonts[fidx].base) {
            data->fonts[fidx].isLoaded = 1;
        }
    }

    glListBase(data->fonts[fidx].base);
    glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}

/* 
 * t c l D e s t r o y W i n d o w ( )
 *
 */
static void
tclDestroyWindow(glmWindow glmw)
{
  static char const funcName[] = "tclDestroyWindow";
  PTRACE ((GLT_MASK|1, funcName, "%p", glmw));
}


static glmWindowVTable tclWindowVTable = {
  tclGetWidget,

  tclRegisterRedraw,
  tclRemoveRedraw,
  tclPostRedraw,
  tclPendingRedraw,

  tclRegisterMapNotify,
  tclIsMapped,

  tclMakeCurrent,
  tclRelease,

  tclAddTimer,
  tclRemoveTimer,

  tclCharStr,
  tclDestroyWindow
};

