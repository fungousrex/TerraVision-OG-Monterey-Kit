/*****************************************************************************
 * Name:
 *    	Platform_X.c
 *
 * Description:
 *	X/Windows specific routines
 *
 * Function List:
 *
 * Dependencies:
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
 *****************************************************************************/

#include "TerraVisionInc.h"
#include "ThreeDCoreI.h"
#include "ThreeDWidgetI.h"
#include "ThreeDWidget.h"
#include "Platform_X.h"

#include <pratom.h>

#include <tvutils/tvutils.h>

#include <X11/GLw/GLwMDrawA.h>
#include <X11/IntrinsicP.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/ShellP.h>
#include <Xm/Form.h>

static int  TerraVisionInitGraphics( ThreeDWidget * );
static void TerraVisionSetupCanvas( ThreeDWidget * );
static void TerraVisionChooseVisuals ( ThreeDWidget * );
static void TerraVisionDefineColormap( ThreeDWidget * );
static void TerraVisionOverrideShellDefaults( XVisualInfo *v );
static void PlatformCallTimedFunc( XtPointer closure, XtIntervalId *id );
static void ThreeDWidgetInitCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetResizeCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetExposeCB(Widget, XtPointer, XtPointer);
static XtActionProc PlatformBtnUp(Widget, XEvent *, GLbyte **, Cardinal*);
static XtActionProc PlatformBtn2Up(Widget, XEvent *, GLbyte **, Cardinal*);
static XtActionProc PlatformBtn1Down(Widget, XEvent *,GLbyte **,Cardinal*);
static XtActionProc PlatformBtn2Down(Widget, XEvent *,GLbyte **,Cardinal*);
static XtActionProc PlatformBtn3Down(Widget, XEvent *,GLbyte **,Cardinal*);
static XtActionProc PlatformMotion(Widget, XEvent *, GLbyte **, Cardinal*);
static XtActionProc PlatformKeypress(Widget w, XEvent *, GLbyte **,
					 Cardinal *);
static XtActionProc PlatformKeyrelease(Widget w, XEvent *, GLbyte **,
					   Cardinal *);
static void ThreeDWidgetAddTimerHandler (void **loc, 
					 void *closure, unsigned int msecs, 
					 void (*f) (void *f_closure),
					 void *f_closure);
static void ThreeDWidgetDelTimerHandler (void **loc, void *closure);

static XtActionsRec ThreeDActionsTable[] = {
    {"PlatformBtn1Down", (XtActionProc) PlatformBtn1Down},
    {"PlatformBtn1Up", (XtActionProc) PlatformBtnUp},
    {"PlatformBtn2Down", (XtActionProc) PlatformBtn2Down},
    {"PlatformBtn2Up", (XtActionProc) PlatformBtn2Up},
    {"PlatformBtn3Down", (XtActionProc) PlatformBtn3Down},
    {"PlatformBtn3Up", (XtActionProc) PlatformBtnUp},
    {"PlatformMotion", (XtActionProc) PlatformMotion},
    {"PlatformKeypress", (XtActionProc) PlatformKeypress},
    {"PlatformKeyrelease", (XtActionProc) PlatformKeyrelease}
};

static char ThreeDTranslations[] =
    "<Btn1Down>:		PlatformBtn1Down() \n\
     <Btn1Up>:			PlatformBtn1Up() \n\
     <Btn2Down>:		PlatformBtn2Down() \n\
     <Btn2Up>:			PlatformBtn2Up() \n\
     <Btn3Down>:		PlatformBtn3Down() \n\
     <Btn3Up>:			PlatformBtn3Up()	\n\
     <Motion>:			PlatformMotion() \n\
     <Key>:                     PlatformKeypress() \n\
     <KeyUp>:                   PlatformKeyrelease() \n"
;


int
PlatformSetupGraphics( ThreeDWidget *pluginWidget )
{
  TerraVisionSetupCanvas( pluginWidget );
  TerraVisionInitGraphics( pluginWidget );
  return TRUE;
}

void
PlatformShowWindow( ThreeDWidget *pluginWidget )
{
  XtRealizeWidget( pluginWidget->platform.parent );
}

void
PlatformTimedFunc( ThreeDWidget *pluginWidget, void (*func)(void), int ms )
{
  XtAppAddTimeOut( pluginWidget->platform.appContext, ms, (XtTimerCallbackProc)
		   PlatformCallTimedFunc, func );

}

void
PlatformGraphicsLoop( void *arg )
{
  static char const funcName[] = "PlatformGraphicsLoop";
  ThreeDWidget *pluginWidget = (ThreeDWidget *) arg;
  XtAppContext appContext = pluginWidget->platform.appContext;

  pthr_set_name (PR_GetCurrentThread (), "Renderer");

  /* LOOP AWAY */

  while ( ! exit_flag ) {
    XEvent event;
    XtAppNextEvent (appContext, &event);
    XtDispatchEvent (&event);
  }

  TSMTRACE ((INIT_MASK|1, funcName, "Exit Renderer"));
}

void
PlatformCloseDown( ThreeDWidget *threeDWidget )
{
  
}

static void
PlatformCallTimedFunc( XtPointer closure, XtIntervalId *id )
{
  void (*func)(void) = (void (*)(void)) closure;
  (*func)();
}

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
  
  score = speed * (props->colorDepth + props->zbufDepth + props->stencilDepth);

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
PlatformInitThreeDWidget( ThreeDWidget *threeDWidget )
{
  /*static const char funcName[] = "PlatformInitThreeDWidget";*/
  Widget         form, parent = threeDWidget->platform.parent;
  XtTranslations transTable;
  XtAppContext   appContext = XtWidgetToApplicationContext (parent);
  glmWindow      glmw;
  glmContext     glmCtx;

  glmCtx = tvuGetContext( parent );

  form = XtVaCreateWidget("topForm",
			  xmFormWidgetClass, parent,
			  XmNleftAttachment, XmATTACH_FORM,
			  XmNrightAttachment, XmATTACH_FORM,
			  XmNtopAttachment, XmATTACH_FORM,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
  
  threeDWidget->platform.windowArea =
    XtVaCreateWidget("windowArea",
		     glwMDrawingAreaWidgetClass, form,
		     GLwNvisualInfo, glmXContextVisualInfo (glmCtx),
		     XmNtopAttachment, XmATTACH_FORM,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNrightAttachment, XmATTACH_FORM,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNtraversalOn, True,
		     XmNheight, 300,
		     XmNwidth, 300,
		     NULL);
  
  /* ADD GL CALLBACKS */

  XtAddCallback(threeDWidget->platform.windowArea, GLwNresizeCallback,
		ThreeDWidgetResizeCB, (XtPointer) threeDWidget);
  XtAddCallback(threeDWidget->platform.windowArea, GLwNginitCallback,
		ThreeDWidgetInitCB, (XtPointer) threeDWidget);
  XtAddCallback(threeDWidget->platform.windowArea, GLwNexposeCallback,
		ThreeDWidgetExposeCB, (XtPointer) threeDWidget);

  glmw = glmXtCreateWindow (threeDWidget->platform.windowArea,
			    glmCtx, True, False);

  threeDWidget->threeDCore =
    tvBaseCreate (threeDWidget, threeDWidget->title, glmw);

  XtManageChild (threeDWidget->platform.windowArea);
  XtManageChild (form);

  /* attach the new translations table for event callbacks */

  transTable = XtParseTranslationTable( ThreeDTranslations );
  XtAppAddActions( appContext, ThreeDActionsTable,
		   XtNumber(ThreeDActionsTable) );
  XtOverrideTranslations( threeDWidget->platform.windowArea, transTable );

  return TRUE;
}

/*
 * Synopsis:
 *   void TerraVisionSetupCanvas( ThreeDWidget * )
 *
 * Description:
 *   Sets up the OpenGL drawing canvas for our plugin
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 15 December, 1999
 *
 */

static void
TerraVisionSetupCanvas( ThreeDWidget *pluginWidget )
{
  const char     *funcName = "TerraVisionSetupCanvas";
  static XtAppContext app = {0};
  static char const     *appClass = "TerraVision";
  Widget         toplevel;
  PlatformWidget *platform = &pluginWidget->platform;
  char           *argv[2] = { "TerraVision", "" };  /* dummy argv array */
  int            argc = 1;

  /* get the settings from the command line */

  platform->window = (Window) tvuGetInt( TVU_NS_WIN_ID );
  pluginWidget->width = tvuGetInt( TVU_WIDTH );
  pluginWidget->height = tvuGetInt( TVU_HEIGHT );

  if ( pluginWidget->width  == 0 ) pluginWidget->width = 450;
  if ( pluginWidget->height == 0 ) pluginWidget->height = 450;

  /* currently always create a new window, even if called as a   */
  /* plugin. Do this because we can't find a way to get a Widget */
  /* structure from the Window that Netscape passes us.          */

  if ( TRUE || platform->window == 0 ) {
    
    TSMTRACE(( INIT_MASK|1, funcName, "Creating dummy window context" ));

    XtSetLanguageProc( NULL, NULL, NULL );

    toplevel = XtVaOpenApplication( &app, appClass, NULL, 0, &argc, argv,
				    NULL, applicationShellWidgetClass,
				    NULL, 0, NULL );

    platform->window         = XtWindow( toplevel );
    platform->display        = XtDisplay( toplevel );
    platform->parent         = toplevel;
    platform->appContext     = app;
    pluginWidget->x          = 0;
    pluginWidget->y          = 0;
    pluginWidget->is_plugin  = FALSE;

    XtVaSetValues( toplevel, XmNwidth, pluginWidget->width, XmNheight,
		   pluginWidget->height, NULL );

  } else {

    TSMTRACE(( INIT_MASK|1, funcName, "Using Netscape window ID %d",
	       platform->window ));

    XtToolkitInitialize();

    platform->appContext = XtCreateApplicationContext();
    platform->display    = XtOpenDisplay( platform->appContext, NULL,
					  "TerraVision", appClass, NULL, 0,
					  &argc, argv );
    pluginWidget->x          = 0;
    pluginWidget->y          = 0;
    pluginWidget->is_plugin  = TRUE;
    platform->parent         = XtWindowToWidget( platform->display,
						 platform->window );

    fprintf( stderr, "tv_window  = %#x\n", platform->window );
    fprintf( stderr, "tv_display = %p (%s)\n", platform->display,
	     DisplayString( platform->display ) );
    fprintf( stderr, "tv_defscrn = %d\n", DefaultScreen( platform->display ) );
    fprintf( stderr, "tv_rootwin = %#x\n",
	     RootWindow( platform->display, 
			 DefaultScreen( platform->display )));
    fprintf( stderr, "tv_connumb = %d\n", ConnectionNumber(platform->display));
    fprintf( stderr, "tv_parent  = %p\n", platform->parent );
  }

}

/*
 * Synopsis: TerraVisionInitGraphics()
 *
 *
 * Description:  Initializes the graphics, X11 and OpenGL, and runs the
 *    		 event loop.
 *
 * Externals:
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *	November 7, 1993
 */

static int
TerraVisionInitGraphics( ThreeDWidget *pluginWidget )
{
  static char    *funcName = "TerraVisionInitGraphics";
  /*PlatformWidget *platform = &pluginWidget->platform;*/

  TSMTRINC ((INIT_MASK|OGL_MASK|1, 1));
  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Entering."));

  /* Check for various OpenGL features. */

  TerraVisionChooseVisuals( pluginWidget );
  TerraVisionDefineColormap( pluginWidget );


  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Done"));
  TSMTRINC ((INIT_MASK|OGL_MASK|1, -1));
  return TRUE;
}

/*
 * Synopsis: TerraVisionChooseVisuals( ThreeDWidget * )
 *
 * Description:  Chooses an appropriate visual.
 *
 * Returns: NONE
 *
 * Author:
 *		Lee Iverson
 *
 * Date:
 *	December 11, 1996
 */

static void
TerraVisionChooseVisuals( ThreeDWidget *pluginWidget )
{
  static char * funcName = "TerraVisionChooseVisuals";
  Display *disp = pluginWidget->platform.display;
  Screen  *scr = XtScreen( pluginWidget->platform.parent );
  XVisualInfo *vis = &pluginWidget->platform.visInfo;

  /* INITIALIZE OPENGL CONTEXT */
  int screen_num = XScreenNumberOfScreen (scr);

  /* CHECK IF WE'VE GOT A 24 BIT TRUECOLOR VISUAL. */
  if (XMatchVisualInfo (disp, screen_num, 24, TrueColor, vis)) {

  /* CHECK IF WE'VE GOT A 16 BIT TRUECOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num, 16, TrueColor, vis)) {

  /* CHECK IF WE'VE GOT A 16 BIT PSEUDOCOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num, 16, PseudoColor, vis)) {
    
  /* CHECK IF WE'VE GOT AN 8 BIT PSEUDOCOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num, 8, PseudoColor, vis)) {
  } else {
    tsmError (funcName, "Unable to find usable basic visual.");
    exit (1);
  }

  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, 
	     "tvVisualInfo {vis=%p, vid=%x, scrn=%d, depth=%d, cls=%d}",
	     vis->visual, vis->visualid, vis->screen, vis->depth, vis->class));
}

/*
 * Synopsis: TerraVisionDefineColormap(ThreeDWidget *)
 *
 * Description:  Chooses an appropriate colormap based on vinfo and
 *		 attaches it to shell.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: December 11, 1996
 */

static void
TerraVisionDefineColormap( ThreeDWidget *pluginWidget )
{
  Widget  shell = pluginWidget->platform.parent;
  Display *dpy = pluginWidget->platform.display;
  Screen  *tvScreen = XtScreen( pluginWidget->platform.parent );
  XVisualInfo *vinfo = &pluginWidget->platform.visInfo;

  /* When we use a the default visual, nothing interesting is required. */
  if ( DefaultVisual( dpy, vinfo->screen ) == vinfo->visual )
    return;

  /* Otherwise, create an appropriate colormap. */
  {
    Colormap colormap;
    XColor color, exact;
    unsigned long foreground, background;
    unsigned long top_shadow, bot_shadow, select;

    colormap = XCreateColormap( dpy, RootWindow( dpy, vinfo->screen),
				vinfo->visual, AllocNone );

    /* Allocate the background and foreground. */
    if ( ! XAllocNamedColor( dpy, colormap, "gray", &color, &exact ) )
      return;
    background = color.pixel;

    /* Ask Motif to allocate appropriate coverage colors. */
    XmGetColors (tvScreen, colormap, background, &foreground,
		 &top_shadow, &bot_shadow, &select);

    /* And initialize the applicationShell for inheritance. */
    XtVaSetValues (shell,
		   XmNdepth, vinfo->depth,
		   XmNcolormap, colormap,
		   XmNvisual, vinfo->visual,
		   XmNbackground, background,
		   XmNforeground, foreground,
		   XmNarmColor, select,
		   XmNtopShadowColor, top_shadow,
		   XmNbottomShadowColor, bot_shadow,
		   NULL);

    TerraVisionOverrideShellDefaults (vinfo);
  }
}

/*
 * Synopsis: TerraVisionOverrideShellDefaults(XVisualInfo *v)
 *
 * Description:  Overrides the visual used by all shells created by the
 *               application.  This is necessary because Motif properly
 *               inherits depth and colormap from the applicationShell but not
 *               the visual.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: December 11, 1996
 */

static struct {
  void (*Initialize) ();
  Visual *visual;
} shellValues;

static void
TerraVisionShellInitialize (Widget treq, Widget tnew,
			    ArgList args, Cardinal *num_args)
{
  ShellWidget shnew = (ShellWidget) tnew;

  shnew->shell.visual = shellValues.visual;

  if (shellValues.Initialize)
    (*shellValues.Initialize) (treq, tnew, args, num_args);
}

static void
TerraVisionOverrideShellDefaults (XVisualInfo *v)
{
  shellValues.visual = v->visual;

  shellValues.Initialize = shellWidgetClass->core_class.initialize;
  shellWidgetClass->core_class.initialize = TerraVisionShellInitialize;
}

/*
 * Synopsis:
 *   void ThreeDWidgetInitCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   Callback called when the GL window is initialized. This occurs when
 *   the widget is first realized. Refer to O'Reilly for a definition
 *   of "widget realization."
 *
 * Returns: void
 *
 * Author: Stephen Lau
 *
 * Date: March 25, 1993
 *
 */

static void
ThreeDWidgetInitCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  char const        funcName[] = "ThreeDWidgetInitCB";
  ThreeDWidget      *threeDWidget = (ThreeDWidget *) client_data;
  int               winsize[2];
  XWindowAttributes attr;

  TSMTRACE ((THREED_MASK|1, funcName, "threeDWidget=%p", threeDWidget));
  TSMTRINC ((THREED_MASK|1, 1));

  /* Get the size of the window */

  XGetWindowAttributes( XtDisplay(w), XtWindow(w), &attr );
  winsize[0] = attr.width;
  winsize[1] = attr.height;

  /* Set the new size in the tvBase */

  tvSetIntegerv( threeDWidget->threeDCore, TV_SIZE, winsize );

  /* initialise the graphics for the tvBase */

  ThreeDWidgetStartRendering ( threeDWidget );

  TSMTRINC ((THREED_MASK|1, -1));
}

/*
 * Synopsis:
 *   void ThreeDWidgetResizeCB(Widget, XtPointer, XtPointer)
 *
 * Description:
 *   Callback called when a resize event occurs. The GL window
 *   is changed to reflect the new size. No redrawing needs to 
 *   occur here since a resize usually triggers an expose event.
 *
 * Returns: void
 *
 * Author: Stephen Lau
 *
 * Date: March 25, 1993
 *
 */

static void
ThreeDWidgetResizeCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  /*static char const funcName[] = "ThreeDWidgetResizeCB";*/
  ThreeDWidget      *threeDWidget = (ThreeDWidget *) client_data;
  int               winsize[2];
  XWindowAttributes attr;

  /* Get the new window size */

  XGetWindowAttributes (XtDisplay(w), XtWindow(w), &attr);
  winsize[0] = attr.width;
  winsize[1] = attr.height;

  /* set the new size for the core */

  tvSetIntegerv( threeDWidget->threeDCore, TV_SIZE, winsize );

  /* and refresh the display */

  tvBaseUpdateView( threeDWidget->threeDCore, FALSE );
}


/*
 * Synopsis: ThreeDWidgetExposeCB(Widget, XtPointer, XtPointer)
 *
 * Description: Called on exposure, simply posts a redraw signal.
 *
 * Returns: NULL
 *
 * Author: Lee Iverson
 *
 * Date: February 1997
 *
 */

static void
ThreeDWidgetExposeCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  static char  *funcName = "ThreeDWidgetExposeCB";
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  TSMTRACE ((THREED_MASK|1, funcName, "threeDWidget=%p", threeDWidget));

  tvBasePostRedraw (threeDWidget->threeDCore, False);
}

/*
 * Synopsis:
 *    XtActionProc PlatformKeypress(Widget,XEvent *,GLbyte **,Cardinal *);
 *
 * Description:
 *    Callback that is called whenever the user presses a key.
 *    Find keysym codes in /usr/include/X11/keysymdef.h
 *	
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 *
 * Date: 8 February 1999
 *
 */

static XtActionProc
PlatformKeypress( Widget w, XEvent *generic_event, 
		  GLbyte **params, Cardinal *num_params )
{
  static char * funcName = "PlatformKeypress";
  XKeyEvent *event = (XKeyEvent *) generic_event;
  ThreeDWidget * threeD = pluginWidget;
  tvBase * threeDCore;
  int grabKeyboard = 0;

  /* these delta are used for the cursor key movements */

  double pitch_delta = 2.0 * M_PI / 180.0;
  double yaw_delta   = 2.0 * M_PI / 180.0;
  double slide_delta = 50.0;
  double zoom_delta  = 50.0;

  if ( threeD == NULL ) return NULL;
  threeDCore = threeD->threeDCore;

  /* reverse action of up/down cursor keys depending on airplane controls */
  if ( ! threeD->airplaneControl )
    pitch_delta *= -1.0; 

  switch ( XLookupKeysym( event, 0 ) ) {

  case XK_space:
    tvStopMotion( threeDCore );
    break;

    /* Escape = close 3-D viewer window */

  case XK_Escape:
    TerraVisionQuit();
    return NULL;

    /* 2 = go to 2-D constrain mode */

  case XK_2:
    threeDCore->twoDConstrain = TRUE;
    tvBaseUpdateView ( threeDCore, FALSE );
    break;

    /* 3 = go to 3-D mode */

  case XK_3:
    threeDCore->twoDConstrain = FALSE;
    tvBaseUpdateView ( threeDCore, FALSE );
    break;

    /* CTRL + N = Goto Next path waypoint */

  case XK_N:
  case XK_n:
    if ( threeD->ctrlDown )
      PathGotoNext( threeDCore );
    break;

    /* CTRL + V = Goto Previous path waypoint */

  case XK_V:
  case XK_v:
    if ( threeD->ctrlDown )
      PathGotoPrev( threeDCore );
    break;

    /* CTRL + P = Play Path */

  case XK_P:
  case XK_p:
    if ( threeD->ctrlDown )
      PathPlay( threeD->threeDCore );
    break;

    /* CTRL + U = Play Pause/Continue */

  case XK_U:
  case XK_u:
    if ( threeD->ctrlDown )
      PathPause( threeD->threeDCore );
    break;

    /* CTRL + T = Goto Start of Path */

  case XK_T:
  case XK_t:
    if ( threeD->ctrlDown )
      PathGotoStart( threeD->threeDCore );
    break;

    /* CTRL + E = Goto End of Path */

  case XK_E:
  case XK_e:
    if ( threeD->ctrlDown )
      PathGotoEnd( threeD->threeDCore );
    break;

    /* O = reset to origin */

  case XK_O:
  case XK_o:
    tvBaseInitView(threeD->threeDCore, tvBaseTop (threeD->threeDCore));
    break;

    /* H = toggle the HUD */

  case XK_H:
  case XK_h:
    threeD->threeDCore->headsUp = ! threeD->threeDCore->headsUp;
    tvBasePostRedraw( threeD->threeDCore, FALSE );
    break;

      /* L = toggle the SRI Logo */

  case XK_L:
  case XK_l:
    threeDCore->showLogo = ! threeDCore->showLogo;
    tvBasePostRedraw( threeDCore, FALSE );
    break;

    /* - = toggle z-buffer hack */

  case XK_minus:
    threeD->threeDCore->useZBufferHack = !threeD->threeDCore->useZBufferHack;
    tvBaseUpdateView( threeD->threeDCore, FALSE );
    break;

    /* CTRL+S = Path Stop */

  case XK_S:
  case XK_s:
    if ( threeD->ctrlDown ) 
      PathStop( threeD->threeDCore );
    break;

    /* Left Arrow = rotate or slide to the left */

  case XK_Left:
  case XK_KP_Left:
    if ( threeD->shiftDown )
      tvMoveViewpoint( threeDCore, 0, -slide_delta, 0, FALSE );
    else
      tvRotate( threeDCore, yaw_delta, 0, 0, FALSE );
    break;

    /* Right Arrow = rotate or slide to the right */

  case XK_Right:
  case XK_KP_Right:
    if ( threeD->shiftDown )
      tvMoveViewpoint( threeDCore, 0, slide_delta, 0, FALSE );
    else
      tvRotate( threeDCore, -yaw_delta, 0, 0, FALSE );
    break;

    /* Up Arrow = increase pitch, slide, or zoom */

  case XK_Up:
  case XK_KP_Up:
    if ( threeD->shiftDown )
      tvMoveViewpoint( threeDCore, 0, 0, -slide_delta, FALSE );
    else if ( threeD->ctrlDown )
      tvMoveViewpoint( threeDCore, zoom_delta, 0, 0, FALSE );
    else
      tvRotate( threeDCore, 0, -pitch_delta, 0, FALSE );
    break;

    /* Down Arrow = decrease pitch, slide, or zoom */

  case XK_Down:
  case XK_KP_Down:
    if ( threeD->shiftDown )
      tvMoveViewpoint( threeDCore, 0, 0, slide_delta, FALSE );
    else if ( threeD->ctrlDown )
      tvMoveViewpoint( threeDCore, -zoom_delta, 0, 0, FALSE );
    else
      tvRotate( threeDCore, 0, pitch_delta, 0, FALSE );
    break;

    /* PgUp = fly to next viewpoint in bookmark list */

  case XK_Page_Up:
  case XK_KP_Page_Up:
    ViewpointCycleUp( threeD );
    break;

    /* PgUp = fly to previous viewpoint in bookmark list */

  case XK_Page_Down:
  case XK_KP_Page_Down:
    ViewpointCycleDown( threeD );
    break;

    /* Record whether the shift key is currently depressed */

  case XK_Shift_L:
  case XK_Shift_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press SHIFT"));
    threeD->shiftDown = TRUE;
    grabKeyboard = (! threeD->ctrlDown && ! threeD->altDown);
    break;

    /* Record whether the ctrl key is currently depressed */

  case XK_Control_L:
  case XK_Control_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press CTRL"));
    threeD->ctrlDown = TRUE;
    grabKeyboard = (! threeD->shiftDown && ! threeD->altDown);
    break;

    /* Record whether the alt key is currently depressed */

  case XK_Alt_L:
  case XK_Alt_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press ALT"));
    threeD->altDown = TRUE;
    grabKeyboard = (! threeD->ctrlDown && ! threeD->shiftDown);
    break;

  }

  if (grabKeyboard) {
    TSMTRACE ((THREED_MASK|1, funcName, "Grab keyboard"));
    XtGrabKeyboard (w, False, GrabModeAsync, GrabModeAsync, CurrentTime);
  }

  return NULL;
}

/*
 * Synopsis:
 *    XtActionProc PlatformKeyrelease(Widget,XEvent *,GLbyte **,Cardinal *)
 *
 * Description:
 *    Callback that is called whenever the user releases a key.
 *    Used primarly to find out when Shift/Ctrl keys are released.
 *    Find keysym codes in /usr/include/X11/keysymdef.h
 *	
 * Returns: NULL
 *
 * Author: Martin Reddy
 *
 * Date: 20 February 1999
 *
 */

static XtActionProc
PlatformKeyrelease( Widget w, XEvent *generic_event, GLbyte **params,
			Cardinal *num_params )
{
  static char const funcName[] = "PlatformKeyrelease";
  XKeyEvent *event = (XKeyEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  if (! threeD) return NULL;

  switch ( XLookupKeysym( event, 0 ) ) {

    /* Record whether the shift key is currently depressed */

  case XK_Shift_L:
  case XK_Shift_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Release SHIFT"));
    threeD->shiftDown = FALSE;
    if (! threeD->ctrlDown && ! threeD->altDown) {
      TSMTRACE ((THREED_MASK|1, funcName, "Ungrab keyboard"));
      XtUngrabKeyboard (w, CurrentTime);
    }
    break;

    /* Record whether the ctrl key is currently depressed */

  case XK_Control_L:
  case XK_Control_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Release CTRL"));
    threeD->ctrlDown = FALSE;
    if (! threeD->shiftDown && ! threeD->altDown) {
      TSMTRACE ((THREED_MASK|1, funcName, "Ungrab keyboard"));
      XtUngrabKeyboard (w, CurrentTime);
    }
    break;

    /* Record whether the alt key is currently depressed */

  case XK_Alt_L:
  case XK_Alt_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Release ALT"));
    threeD->altDown = FALSE;
    if (! threeD->shiftDown && ! threeD->ctrlDown) {
      TSMTRACE ((THREED_MASK|1, funcName, "Ungrab keyboard"));
      XtUngrabKeyboard (w, CurrentTime);
    }
    break;

  }

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformBtn1Up(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback for when the user releases Button 1 of the mouse.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformBtnUp( Widget w, XEvent *generic_event, GLbyte **params,
	       Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformBtnUp";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetBtnUp( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformBtn2Up(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback for when the user releases Button 2 of the mouse.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformBtn2Up( Widget w, XEvent *generic_event, GLbyte **params,
		Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformBtn2Up";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetBtn2Up( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformBtn1Down(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback for when the user presses Button 1 of the mouse.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformBtn1Down( Widget w, XEvent *generic_event, GLbyte **params,
		  Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformBtn1Down";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetBtn1Down( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformBtn2Down(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback for when the user presses Button 2 of the mouse.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformBtn2Down( Widget w, XEvent *generic_event, GLbyte **params,
		      Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformBtn2Down";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetBtn2Down( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformBtn3Down(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback for when the user presses Button 3 of the mouse.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformBtn3Down( Widget w, XEvent *generic_event, GLbyte **params,
		  Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformBtn3Down";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetBtn3Down( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   XtActionProc PlatformMotion(Widget, XEvent *, GLbyte * *, Cardinal *)
 *
 * Description:
 *   Callback that is invoked when the user moves the mouse
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

static XtActionProc
PlatformMotion( Widget w, XEvent *generic_event, GLbyte **params,
		Cardinal *num_params )
{
  /*static char const funcName[] = "PlatformMotion";*/
  XMotionEvent *event = (XMotionEvent *) generic_event;
  ThreeDWidget *threeD = pluginWidget;

  ThreeDWidgetMotion( threeD, event->x, event->y );

  return NULL;
}

/*
 * Synopsis:
 *   void ThreeDWidgetAddTimerHandler(...)
 *   void ThreeDWidgetDelTimerHandler(...)
 *
 * Description:
 *   tvBase callbacks for managing timers under X11
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 17 December, 1999.
 *
 */

typedef struct {
  XtIntervalId id;
  void (*f) (void *closure);
  void *closure;
} TimerClosure;

static void
TimerHandler (XtPointer closure, XtIntervalId *interval)
{
  void **loc = closure;
  TimerClosure *tc;

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
ThreeDWidgetAddTimerHandler (void **loc, 
			     void *closure, unsigned int msecs, 
			     void (*f) (void *closure), void *f_closure)
{
  XtAppContext appContext = closure;

  TimerClosure *tc = malloc (sizeof (*tc));
  *loc = tc;

  tc->f = f;
  tc->closure = f_closure;
  tc->id = XtAppAddTimeOut (appContext, msecs, TimerHandler, loc);
}

static void
ThreeDWidgetDelTimerHandler (void **loc, void *closure)
{
  TimerClosure *tc;

  /* Clear the reference location to "take control" of tc */
  assert (sizeof (void *) == sizeof (PRInt32));
  tc = (void *) PR_AtomicSet ((PRInt32 *) loc, 0);

  if (tc) {
    XtRemoveTimeOut (tc->id);

    /* Disable this pointer, then free */
    free (tc);
  }
}


