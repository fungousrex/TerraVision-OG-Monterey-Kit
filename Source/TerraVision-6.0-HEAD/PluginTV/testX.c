#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_X11

#include <stdlib.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Form.h>
#include <X11/GLw/GLwMDrawA.h>

#include <glt/glm.h>
#include <pthr/trace.h>

#define GLT_MASK (1<<4)

typedef struct {
  unsigned int singleBuffer:1;
  unsigned int stereo:1;
  unsigned int slow:1;
  int n, len;
  glmContextProperties *props;
} glmContextList;

static int
GatherContexts (glmContextProperties const *props,
		void *closure)
{
  static char const funcName[] = "GatherContexts";
  glmContextList *ctxs = closure;
  int i;

  if (props->overlay || props->underlay || props->colorIndex
      || props->singleBuffer != ctxs->singleBuffer
      || props->stereo != ctxs->stereo
      || props->slow != ctxs->slow)
    return 0;
  if (props->zbufDepth < 16
      || props->colorDepth < 8)
    return 0;

  i = ctxs->n++;
  if (i >= ctxs->len) {
    ctxs->len = ctxs->len*2;
    ctxs->props = realloc (ctxs->props, ctxs->len * sizeof (ctxs->props[0]));
  }

  PTRACE ((GLT_MASK|1, funcName, 
	   "[%d] %#x: %s%s %s%s%s P:%d D:%d S:%d M:%d/%d\n",
	   i, props->id,
	   props->colorIndex ? "IDX" : "RGB",
	   props->overlay ? "/Over" : props->underlay ? "/Under" : "",
	   props->singleBuffer ? "S" : "D",
	   props->stereo ? "/St" : "",
	   props->slow ? " Slow" : "",
	   props->colorDepth,
	   props->zbufDepth,
	   props->stencilDepth,
	   props->multiBufs, props->multiBufSamples));
  ctxs->props[i] = *props;
  return 0;
}

typedef struct {
  int exit_flag;
  glmContext glmCtx;
  glmWindow glmw;
  float best_score;
  unsigned long best_visual;
  glmContextList const *ctxs;
} VisualSearchClosure;

void
TimeVisual (Widget w, XtPointer client_data, XtPointer call_data)
{
  GLwDrawingAreaCallbackStruct *drawInfo = call_data;
  VisualSearchClosure *cl = client_data;
  int speed, score;

  glmContextProperties const *props = glmContextProps (cl->glmCtx);
  XVisualInfo *vinfo = glmXContextVisualInfo (cl->glmCtx);

  speed = glmWindowSpeed (drawInfo->width, drawInfo->height, cl->glmw);
  
  score = speed * (props->colorDepth + props->zbufDepth);

  printf ("Visual %#x: speed %d score %d\n", vinfo->visualid, speed, score);
  if (score > cl->best_score) {
    cl->best_score = score;
    cl->best_visual = vinfo->visualid;
    cl->exit_flag = TRUE;
  }
}

void
TimeVisualsEventHandler (Widget form, XtPointer client_data,
			 XEvent *event, Boolean *cont)
{
  XtAppContext appContext = XtWidgetToApplicationContext (form);
  VisualSearchClosure *cl = client_data;
  glmContextList const *ctxs = cl->ctxs;
  Display *disp = XtDisplay (form);
  int i;

  if (event->type != MapNotify || cl->exit_flag) return;
  /* Only handle this once. */
  XtRemoveEventHandler (form, SubstructureNotifyMask, True,
			TimeVisualsEventHandler, client_data);

  for (i = 0; i < ctxs->n; ++i) {
    Widget w;
    XVisualInfo tmpl;
    XVisualInfo *vinfo;
    int n;

    tmpl.visualid = ctxs->props[i].id;
    vinfo = XGetVisualInfo (disp, VisualIDMask, &tmpl, &n);
    if (n < 1) continue;

    w = XtVaCreateWidget("windowArea",
			 glwMDrawingAreaWidgetClass, form,
			 GLwNvisualInfo, vinfo,
			 XmNtopAttachment, XmATTACH_FORM,
			 XmNleftAttachment, XmATTACH_FORM,
			 XmNrightAttachment, XmATTACH_FORM,
			 XmNbottomAttachment, XmATTACH_FORM,
			 XmNtraversalOn, True,
			 NULL);
  
    cl->glmCtx = glmXCreateContext (disp, vinfo);
    cl->glmw = glmXtCreateWindow (w, cl->glmCtx,
				  !ctxs->singleBuffer, ctxs->stereo);
    cl->exit_flag = FALSE;

    XtAddCallback(w, GLwNexposeCallback, TimeVisual, client_data);
    XtManageChild (w);

    while ( ! cl->exit_flag ) {
      XEvent event;
      XtAppNextEvent (appContext, &event);
      XtDispatchEvent (&event);
    }

    XtDestroyWidget (w);
    glmDestroyWindow (cl->glmw);
    glmDestroyContext (cl->glmCtx);
    XFree (vinfo);
  }

  cl->exit_flag = TRUE;
}

unsigned long
FindBestVisual (Widget parent, glmContextList const *ctxs)
{
  VisualSearchClosure closure;

  XtAppContext appContext = XtWidgetToApplicationContext (parent);
  Widget form = XtVaCreateWidget("topForm",
				 xmFormWidgetClass, parent,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNtopAttachment, XmATTACH_FORM,
				 XmNbottomAttachment, XmATTACH_FORM,
				 NULL);

  /* ADD GL CALLBACKS */
  closure.exit_flag = 0;
  closure.ctxs = ctxs;
  XtAddEventHandler (form, SubstructureNotifyMask, True,
		     TimeVisualsEventHandler, (XtPointer) &closure);
  XtManageChild (form);

  while ( ! closure.exit_flag ) {
    XEvent event;
    XtAppNextEvent (appContext, &event);
    XtDispatchEvent (&event);
  }

  XtDestroyWidget (form);

  return closure.best_visual;
}

int
main (int argc, char *argv[])
{
  static XtAppContext app = {0};
  static char const     *appClass = "TerraVision";

  Widget top =
    XtVaOpenApplication( &app, appClass, NULL, 0, &argc, argv,
			 NULL, applicationShellWidgetClass,
			 NULL, 0, NULL );
  
  Widget         parent = top;
  XtAppContext   appContext = XtWidgetToApplicationContext (parent);
  unsigned long  visualid;

  static glmContextProperties props = {0};
  
  glmWindow      glmw;
  glmContext     glmCtx;

  XtVaSetValues( top, XmNwidth, 450, XmNheight, 450, NULL );
  XtRealizeWidget( top );

  if (1) {
    glmContextList ctxs;

    /* Gather up a list of potentially usable contexts to test for speed */
    memset (&ctxs, 0, sizeof(ctxs));
    ctxs.len = 8;
    ctxs.props = malloc (ctxs.len * sizeof (ctxs.props[0]));
    ctxs.singleBuffer = TRUE;
    ctxs.stereo = FALSE;
    glmXChooseContext (XtDisplay (parent),
		       XtScreen (parent),
		       GatherContexts, &ctxs);

    if (ctxs.n > 1) {
      visualid = FindBestVisual (parent, &ctxs);
    } else if (ctxs.n == 1) {
      visualid = FindBestVisual (parent, &ctxs);
    } else {
      fprintf (stderr, "Cannot find usable visual\n");
      exit (1);
    }

    free (ctxs.props);

  }

#if 0
  while ( ! exit_flag ) {
    XEvent event;
    XtAppNextEvent (appContext, &event);
    XtDispatchEvent (&event);
  }
#endif

  sleep (2);

  return 0;
}

#else
int
main (int argc, char *argv[])
{
  return 0;
}
#endif
