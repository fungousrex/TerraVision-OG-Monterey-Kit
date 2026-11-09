/*****************************************************************************
 * Name:
 *    	ThreeDWidget.c
 *
 * Description:
 *	Creation and manipulation of a three D widget
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

#include "Dialogs.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCoreI.h"
#include "ScannedMap.h"
#include "Texture.h"
#include "Viewpoint.h"
#include "PathsFE.h"
#include "OAA.h"
#include "Features.h"
#include "Datasets.h"

#include <tvutils/tvutils.h>

#include <pratom.h>

#include "X11Include.h"

#include <X11/keysym.h>

#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/MwmUtil.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>

#define MAX_TURN (5.0 * (M_PI / 180.0))

/* A list of all ThreeDWidgets */
list_t * widgetList = NULL;

/* how many views have been created over time used for naming */
static int numViewers = 0;

/* STATIC LOCAL FUNCTIONS */
static void ThreeDWidgetAddTimerHandler (void **loc,
					 void *closure, unsigned int msecs, 
					 void (*f) (void *closure),
					 void *f_closure);
static void ThreeDWidgetDelTimerHandler (void **loc, void *closure);

static void ThreeDWidgetMessageHandler (void *, tvMessageType, 
					char const *, va_list);
static void ThreeDWidgetUpdateHandler (void *, tvLinkType, tvBase *);
static void ThreeDWidgetCreateWidget(ThreeDWidget *, Widget, glmContext);
static void ThreeDWidgetCreateUtilButtons(ThreeDWidget *, Widget);
static void ThreeDWidgetDismissCallback(Widget, XtPointer, XtPointer);
static void ThreeDWidgetMosaicCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetMenuCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetInitCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetResizeCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetExposeCB(Widget, XtPointer, XtPointer);
static void ThreeDWidgetUpdateWindow( ThreeDWidget * );
static void ThreeDWidgetFullScreenMode( ThreeDWidget *, int );
static XtActionProc ThreeDWidgetBtnUp(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetBtn2Up(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetBtn1Down(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetBtn2Down(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetBtn3Down(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetMotion(Widget, XEvent *, GLbyte * *, Cardinal*);
static XtActionProc ThreeDWidgetKeypress(Widget w, XEvent *, GLbyte **, Cardinal *);
static XtActionProc ThreeDWidgetKeyrelease(Widget w, XEvent *, GLbyte **, Cardinal *);
static XtTimerCallbackProc 	ThreeDWidgetUpdateTimer(XtPointer, XtIntervalId *);
static XtTimerCallbackProc 	ThreeDWidgetAccelerate(XtPointer, XtIntervalId *);
static XtTimerCallbackProc      ThreeDWidgetDeccelerate(XtPointer, XtIntervalId *);
static XtTimerCallbackProc 	ThreeDWidgetMove(XtPointer, XtIntervalId *);
static Widget   ThreeDWidgetCreateMenuBar(ThreeDWidget *, Widget);
static void ThreeDWidgetRememberWinSize( ThreeDWidget *threeDWidget );
static void ThreeDWidgetStatusBarClearView( ThreeDWidget *threeDWidget );

#ifdef CAVE
static void ThreeDWidgetSetCAVEGLViewpoint(ThreeDWidget *);
#endif

#ifdef SPACEBALL_H
static void 	ThreeDWidgetProcessSbData(ThreeDWidget *, short *);
#endif

static XtActionsRec ThreeDActionsTable[] = {
    {"ThreeDWidgetBtn1Down", (XtActionProc) ThreeDWidgetBtn1Down},
    {"ThreeDWidgetBtn1Up", (XtActionProc) ThreeDWidgetBtnUp},
    {"ThreeDWidgetBtn2Down", (XtActionProc) ThreeDWidgetBtn2Down},
    {"ThreeDWidgetBtn2Up", (XtActionProc) ThreeDWidgetBtn2Up},
    {"ThreeDWidgetBtn3Down", (XtActionProc) ThreeDWidgetBtn3Down},
    {"ThreeDWidgetBtn3Up", (XtActionProc) ThreeDWidgetBtnUp},
    {"ThreeDWidgetMotion", (XtActionProc) ThreeDWidgetMotion},
    {"ThreeDWidgetKeypress", (XtActionProc) ThreeDWidgetKeypress},
    {"ThreeDWidgetKeyrelease", (XtActionProc) ThreeDWidgetKeyrelease}
};

static char ThreeDTranslations[] =
    "<Btn1Down>:		ThreeDWidgetBtn1Down() \n\
     <Btn1Up>:			ThreeDWidgetBtn1Up() \n\
     <Btn2Down>:		ThreeDWidgetBtn2Down() \n\
     <Btn2Up>:			ThreeDWidgetBtn2Up() \n\
     <Btn3Down>:		ThreeDWidgetBtn3Down() \n\
     <Btn3Up>:			ThreeDWidgetBtn3Up()	\n\
     <Motion>:			ThreeDWidgetMotion() \n\
     <Key>:                     ThreeDWidgetKeypress() \n\
     <KeyUp>:                   ThreeDWidgetKeyrelease() \n"
;

#define DRAWTIMES (0)
#if DRAWTIMES
/* This is a summary of the draw timings used for tuning. */
typedef struct DrawTimes {
  PR_Lock * lock;
  int i;
  int n_fast[3];
  int n_slow[3];
  double secs[3];
  /* Summaries */
  int n;
  float sum_s, sum_f;
  float sum_s2, sum_f2;
  float s, f;
} DrawTimes;

static PRCallOnceType DrawTimes_once;
static DrawTimes drawTimes;

static void
DrawTimesInit (void)
{
  drawTimes.lock = PR_NewLock ();
}
#endif

/*
 *
 * Synopsis: CreateThreeDWidget(Widget *)
 *	
 * Description: Create an "out the window view" widget
 *
 *
 * Externals: NONE
 *
 *
 * Returns: Pointer to a ThreeDWidget structure on Sucess
 *          NULL - On Failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 
 *		March 25, 1993
 *
 */
ThreeDWidget *
CreateThreeDWidget( Widget parent, Boolean cave, Boolean mode3d )
{
  const char   *funcName = "CreateThreeDWidget";
  ThreeDWidget *threeDWidget;
  XtAppContext appContext = XtWidgetToApplicationContext (parent);

  glmContext   glmCtx;
  glmWindow    glmw;
  char         titleBfr[100];

  static glmContextProperties props = {0};
 
  /* CREATE THE THREEDWIDGET STRUCTURE */

  if (! (threeDWidget = (ThreeDWidget *) calloc(sizeof(ThreeDWidget),1)) ) {
    tsmStatus("%s: Error allocating space for ThreeDWidget!", funcName);
    return NULL;
  }

  sprintf(titleBfr, "TerraVision II: Viewer %d", ++numViewers);
  threeDWidget->title = strdup(titleBfr);

  props.singleBuffer = tvuGetInt( TVU_SINGLE_BUFFER );
  props.stereo = tvuGetInt( TVU_STEREO );
  glmCtx  = glmXFindContext (XtDisplay (parent),
			     XtScreen (parent),
			     True, &props);
  if (! glmCtx && tvuGetInt( TVU_STEREO ) ) {
    tsmStatus("%s: Unable to find stereo rendering context!", funcName);
    props.stereo = FALSE;
    glmCtx  = glmXFindContext (XtDisplay (parent),
			       XtScreen (parent),
			       True, &props);
  }
  TSMTRACE ((GLT_MASK|THREED_MASK|1, funcName, "glmCtx %p vinfo %p",
	     glmCtx, glmXContextVisualInfo (glmCtx)));

  if ( ! cave ) {
    ThreeDWidgetCreateWidget(threeDWidget, parent, glmCtx);

    /* CREATE THE CONTROLS FOR THE VIEWPOINT */
    if (! (threeDWidget->threeDControl = 
	CreateThreeDControl(threeDWidget->mainForm, threeDWidget))) {
      tsmError(funcName, "Can't create the control panel!");
      return NULL;
    }
  }

  glmw = glmXtCreateWindow (threeDWidget->windowArea, glmCtx, True, False);

  threeDWidget->threeD =
    tvBaseCreate (threeDWidget, threeDWidget->title, glmw);
  
  tvSetMessageHandler (threeDWidget->threeD, 
		       ThreeDWidgetMessageHandler, threeDWidget);

  tvSetUpdateHandler (threeDWidget->threeD, 
		      ThreeDWidgetUpdateHandler, threeDWidget);

  if (0) {
    /* Hack to enable DIS PDUs */
    int id[3];
    id[0] = 40; id[1] = 1; id[2] = 512;
    tvSetIntegerv (threeDWidget->threeD, TV_DIS_ID, id);
    tvSetBoolean (threeDWidget->threeD, TV_DIS_SEND, TRUE);
  }

  threeDWidget->threeD->useZBufferHack =
    XmToggleButtonGadgetGetState( threeDWidget->zbufferButton );

  threeDWidget->busyDialog = NULL;
  threeDWidget->fselWidget = NULL;
  threeDWidget->fselNumbCBs = 0;
  threeDWidget->linkMenu = NULL;  /* formerly in LinkInit() */

  threeDWidget->pathLoadSel = NULL;
  threeDWidget->pathSaveSel = NULL;
  threeDWidget->pathEditWidget = NULL;

  threeDWidget->showMenu = TRUE;
  threeDWidget->showIcons = FALSE;
  threeDWidget->showFullscreen = FALSE;
  threeDWidget->showStatusBar = TRUE;

  threeDWidget->flyMode = FALSE;
  threeDWidget->shiftDown = FALSE;
  threeDWidget->ctrlDown = FALSE;
  threeDWidget->altDown = FALSE;
  threeDWidget->mouseDown = FALSE;

  threeDWidget->statusBarViewpoint = NULL;
  threeDWidget->statusBarDatasets  = -1;
  threeDWidget->statusBarNavMode   = -1;
  threeDWidget->statusBarViewMode  = -1;

  /* CREATE THE CONTROL PANEL FOR THE TOGGLES */
  if (! (threeDWidget->threeDPanel = 
	 CreateThreeDPanel (parent, (void *)(threeDWidget), mode3d))) {
    tsmError(funcName, "Can't create the toggle control panel!");
    return NULL;
  }

  /* set the default drop to wireframe frame rate */

  tvSetFloat( threeDWidget->threeD, TV_MOTION_THRESHOLD,
	      tvuGetFloat( TVU_DROP_TO_WIREFRAME ) );

  /* set the viewer to be a 3D or constrained 2D viewer by default */

  tvSetBoolean (threeDWidget->threeD, TV_TWOD_CONSTRAIN, ! mode3d );

  /* If tileByTile requested, then set it */
  tvSetBoolean (threeDWidget->threeD, TV_TILE_BY_TILE,
		tvuGetInt( TVU_TILE_BY_TILE ) );

  ViewpointInit( threeDWidget );

  DatasetPrioInit( threeDWidget );

  DatasetDisplayInit( threeDWidget );

  ThreeDWidgetUpdateStatusBar( threeDWidget, "" );

#if DRAWTIMES
  /* Initialize the draw timing structure. */
  PR_CallOnce (&DrawTimes_once, DrawTimesInit);
#endif

  /* update the status bar with the default state */
  ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );

  /* add it to the list */
  if (! widgetList)
    widgetList = list_create (TRUE);
  list_push(widgetList, threeDWidget);

  if (! cave)
    LinkUpdateMenu(); /* update link menus */

  return threeDWidget;
}

void
DestroyThreeDWidget (ThreeDWidget *threeDWidget)
{
  static char const funcName[] = "DestroyThreeDWidget";

  /* Take us off the widget list */
  if (list_remove (widgetList, threeDWidget)) {
    
    TSMTRACE ((THREED_MASK|1, funcName, "(%p)", threeDWidget));
    TSMTRINC ((THREED_MASK|1, 1));

    if (tvGetBoolean (threeDWidget->threeD, TV_STEREO))
      tvSetBoolean (threeDWidget->threeD, TV_STEREO, False);

    tvBaseDestroy(threeDWidget->threeD);

    /* get rid of 3D panels too */
    ThreeDPanelDestroy (threeDWidget->threeDPanel);

    free (threeDWidget);

    TSMTRINC ((THREED_MASK|1, -1));
    TSMTRACE ((THREED_MASK|1, funcName, "(%p) Done", threeDWidget));
  }
}    

/*
 * Synopsis: void ThreeDWidgetForall (void (*f) (ThreeDWidget *, void *),
 *                                    void *closure)
 *
 * Description: Iterates over all current ThreeDWidget* widgets, calling the
 *              function (*f)(widget,closure) for each.
 *
 * Externals: allWidgets
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 * 
 * Date: Oct 1999
 *
 */
void
ThreeDWidgetForall (void (*f) (ThreeDWidget *, void *), void *closure)
{
  list_iter_t i;

  if (! widgetList) return;

  for (i = list_first (widgetList); ! list_done (i); ) {
    ThreeDWidget *w = list_value (i);
    i = list_next (i);
    (*f) (w, closure);
  }
}

/*
 * Synopsis: ThreeDWidget *
 *           ThreeDWidgetFind (int (*f) (ThreeDWidget *, void *),
 *                             void *closure)
 *
 * Description: Iterates over all current ThreeDWidget* widgets, returning the
 *              first one for which (*f)(widget,closure) returns non-zero.
 *
 * Externals: allWidgets
 *
 * Returns: The ThreeDWidget* found.
 *
 * Author: Lee Iverson
 * 
 * Date: Oct 1999
 *
 */
ThreeDWidget *
ThreeDWidgetFind (int (*f) (ThreeDWidget *, void *), void *closure)
{
  list_iter_t i;

  if (! widgetList) return NULL;

  for (i = list_first(widgetList); ! list_done(i); i = list_next(i)) {
    ThreeDWidget *w = list_value (i);
    if ((*f) (w, closure))
      return w;
  }

  return NULL;
}

/*
 * Synopsis:
 *    void ThreeDWidgetStatusBarClearView( ThreeDWidget *threeDWidget )
 *
 * Description:
 *    Clears the first field of the status bar - the viewpoint field.
 *    We do not clear the field if it is already clear.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 June 1999
 *
 */

static void
ThreeDWidgetStatusBarClearView( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget == NULL ) return;
  if ( threeDWidget->statusBarViewpoint == NULL ) return;
  if ( threeDWidget->statusBarViewpoint[0] == '\0' ) return;
  ThreeDWidgetUpdateStatusBar( threeDWidget, "" );
}

/*
 * Synopsis:
 *    void ThreeDWidgetUpdateStatusBar( ThreeDWidget *, char const * )
 *
 * Description:
 *    Refreshes the display of the ThreeDWidget's Status Bar to
 *    reflect any changes in the variables that it displays. The
 *    string parameter is a piece of text that is displayed in
 *    the Viewpoint field of the status bar. If this pointer is NULL,
 *    then we use the previous string (i.e. you can refresh without
 *    destroying what you displayed last time).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 June 1999
 *
 */

void
ThreeDWidgetUpdateStatusBar( ThreeDWidget *threeDWidget, char const *viewpoint)
{
  char const  *curr_view = NULL;
  XmString    string;
  char        buffer[512];
  ThreeDPanel *threeDPanel;
  tvBase *threeD;

  if ( threeDWidget == NULL ) return;
  threeDPanel = threeDWidget->threeDPanel;
  threeD = threeDWidget->threeD;
  if (! threeD) return;

  /* The first field of the status bar is to give a name to the viewpoint */

  if ( PathIsActive ( threeD ) ) {
    Path *path = tvGetPath (threeD);
    if ( path->name )
      curr_view = path->name;
  } else {
    if ( viewpoint )
      curr_view = viewpoint;
  }

  if ( threeDWidget->statusBarLabel1 && curr_view ) {

    /* is this string different from the last one we displayed? */

    if ( threeDWidget->statusBarViewpoint == NULL ||
	 strcmp( threeDWidget->statusBarViewpoint, curr_view ) != 0 ) {

      /* it is different - so let's update the field */

      string = tvXmString( curr_view );
      XtVaSetValues( threeDWidget->statusBarLabel1, XmNlabelString, string, 0);
      XmStringFree( string );

      /* remember the new string that we used here */

      if ( threeDWidget->statusBarViewpoint ) 
	tsmMemFree( threeDWidget->statusBarViewpoint );
      threeDWidget->statusBarViewpoint = tsmMemStrdup( curr_view );

    }

  }

  /* the second field tells us how many datasets are in this widget */

  if ( threeDWidget->statusBarLabel2 ) {
    int datasets = ( threeD->geoList ) ? list_count( threeD->geoList ) : 0;

    /* only update if different from last value */

    if ( datasets != threeDWidget->statusBarDatasets ) {
      sprintf( buffer, "Datasets: %d", datasets );
      string = tvXmString( buffer );
      XtVaSetValues( threeDWidget->statusBarLabel2, XmNlabelString, string, 0);
      XmStringFree( string );
      threeDWidget->statusBarDatasets = datasets;
    }
  }

  /* the third field tells us the navigation mode */

  if ( threeDWidget->statusBarLabel3 ) {
    int navmode = 0;

    if ( PathIsPlaying( threeD ) || threeDPanel == NULL ) {
      string = tvXmString( "Nav Mode: None" );
      navmode = 1;
    } else if ( threeDPanel->drawOnScreen ) {
      string = tvXmString( "Nav Mode: Doodle" );
      navmode = 2;
    } else if ( threeDPanel->seek || threeDWidget->altDown ) {
      string = tvXmString( "Nav Mode: FlyTo" );
      navmode = 3;
    } else if ( threeDWidget->shiftDown || threeDWidget->ctrlDown ||
		( threeDPanel && threeD->twoDConstrain ) ) {
      string = tvXmString( "Nav Mode: Slide" );
      navmode = 4;
    } else {
      string = tvXmString( "Nav Mode: Fly" );
      navmode = 5;
    }

    /* only update interface if value has changed */

    if ( navmode != threeDWidget->statusBarNavMode ) {
      XtVaSetValues( threeDWidget->statusBarLabel3, XmNlabelString, string, 0);
      threeDWidget->statusBarNavMode = navmode;
    }

    XmStringFree( string );
  }

  /* the fourth field tells us the view mode (2D/3D) */

  if ( threeDWidget->statusBarLabel4 ) {
    int viewmode = 0;

    if ( threeDPanel == NULL ) {
      string = tvXmString( "View Mode: Unknown" );
      viewmode = 1;
    } else if ( threeD->twoDConstrain ) {
      string = tvXmString( "View Mode: 2-D" );
      viewmode = 2;
    } else {
      string = tvXmString( "View Mode: 3-D" );
      viewmode = 3;
    }

    /* only update interface if value has changed */

    if ( viewmode != threeDWidget->statusBarViewMode ) {
      XtVaSetValues( threeDWidget->statusBarLabel4, XmNlabelString, string, 0);
      threeDWidget->statusBarViewMode = viewmode;
    }

    XmStringFree( string );
  }

}

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

void
ThreeDWidgetBusyDialog (ThreeDWidget *threeDWidget, BusyDialog *busy)
{
  threeDWidget->busyDialog = busy;
}

static void
ThreeDWidgetMessageHandler (void *closure, tvMessageType type, 
			    char const *fmt, va_list ap)
{
  ThreeDWidget *threeD = closure;
  char message[1024];
  switch (type) {
  case TV_ERROR_MSG: {
    static Widget path_error = NULL;
    vsprintf (message, fmt, ap);
    GenericErrorDialog (threeD->mainForm, &path_error, message);
    break; }
  case TV_STATUS_MSG: {
    if (fmt) {
      vsprintf (message, fmt, ap);
      ThreeDWidgetUpdateStatusBar( threeD, message );
    } else {
      ThreeDWidgetUpdateStatusBar( threeD, NULL );
    }
    break; }
  case TV_BUSY_MSG: {
    if (threeD->busyDialog) {
      vsprintf (message, fmt, ap);
      SetBusyText (threeD->busyDialog, message);
    }
    break; }
  }
}

void
ThreeDWidgetError (ThreeDWidget *threeD, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  ThreeDWidgetMessageHandler (threeD, TV_ERROR_MSG, fmt, ap);
  va_end (ap);
}

static void
ThreeDWidgetUpdateHandler (void *closure, tvLinkType link, tvBase *core)
{
  ThreeDWidget *threeD = closure;

  switch (link) {
  case TV_PATHS:
    ScannedMapRedraw();    
    break;

  case TV_DATASETS:
    /* if we have no more geos left in this widget, then close the window */
    if ( list_count( core->geoList ) == 0 ) {
      ThreeDWidgetDismiss( threeD );
      return;
    }
    ThreeDWidgetRefreshDialogs (threeD);
    break;

  default:
    break;
  }
}

/*
 * Synopsis: void ThreeDWidgetCreateWidget()
 *	
 *
 * Description: Creates the Widgets for the view
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
static void
ThreeDWidgetCreateWidget(ThreeDWidget * threeDWidget,
			 Widget parent,
			 glmContext glmCtx)
{
/*  static char    *funcName = "ThreeDWidgetCreateWidget";*/
  Widget         mainForm, topForm, bottomForm, windowArea, statusForm, frame;
  XmString       title;
  XtTranslations transTable;
  Arg            args[10];
  int            n = 0;
  XtAppContext   appContext;

  /* ATTACH THE TRANSLATIONS, LABEL etc */
  transTable = XtParseTranslationTable(ThreeDTranslations);

  title = tvXmString( threeDWidget->title );
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  threeDWidget->mainForm = mainForm =
    XmCreateFormDialog( parent, "ThreeDMapForm", args, n );
  XmStringFree( title );

  appContext = XtWidgetToApplicationContext (mainForm);
  XtAppAddActions(appContext,
		  ThreeDActionsTable,
		  XtNumber(ThreeDActionsTable));
  threeDWidget->topForm = topForm =
    XtVaCreateManagedWidget("topForm",
			    xmFormWidgetClass, mainForm,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_FORM,
			    NULL);

  threeDWidget->menuBar =
    ThreeDWidgetCreateMenuBar (threeDWidget, topForm);

  threeDWidget->windowArea = windowArea =
    XtVaCreateManagedWidget("windowArea",
			    glwMDrawingAreaWidgetClass, mainForm,
			    GLwNvisualInfo, glmXContextVisualInfo (glmCtx),
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, topForm,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNtraversalOn, True,
			    XmNheight, 480,
			    XmNwidth, 640,
			    NULL);
  
  /* ADD GL CALLBACKS */
  XtAddCallback(windowArea, GLwNresizeCallback,
		ThreeDWidgetResizeCB, (XtPointer) threeDWidget);
  XtAddCallback(windowArea, GLwNginitCallback,
		ThreeDWidgetInitCB, (XtPointer) threeDWidget);
  XtAddCallback(windowArea, GLwNexposeCallback,
		ThreeDWidgetExposeCB, (XtPointer) threeDWidget);
  
  threeDWidget->bottomForm = bottomForm =
    XtVaCreateWidget("bottomForm",
		     xmFormWidgetClass, mainForm,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNrightAttachment, XmATTACH_FORM,
		     XmNbottomAttachment, XmATTACH_FORM,
		     XmNverticalSpacing, 5,
		     XmNhorizontalSpacing, 5,
		     NULL);
  
  ThreeDWidgetCreateUtilButtons (threeDWidget, bottomForm);

  threeDWidget->statusBarForm = statusForm =
    XtVaCreateWidget("threeDWidgetStatusBarWidget",
		     xmFormWidgetClass, mainForm,
		     XmNrightAttachment, XmATTACH_FORM,
		     XmNleftAttachment, XmATTACH_FORM,
		     XmNbottomAttachment, XmATTACH_WIDGET,
		     XmNbottomWidget, bottomForm,
		     XmNresizable, False,
		     XmNverticalSpacing, 1,
		     XmNfractionBase, 10,
		     NULL);
  
  frame =
    XtVaCreateManagedWidget("threeDWidgetStatusFrame1",
			    xmFrameWidgetClass, statusForm,
			    XmNshadowType, XmSHADOW_IN,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_POSITION,
			    XmNleftPosition, 0,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition, 4,
			    NULL );

  threeDWidget->statusBarLabel1 =
    XtVaCreateManagedWidget("threeDWidgetStatusBarLabel1",
			    xmLabelGadgetClass, frame,
			    XmNalignment, XmALIGNMENT_BEGINNING,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    NULL);
  frame =
    XtVaCreateManagedWidget("threeDWidgetStatusFrame2",
			    xmFrameWidgetClass, statusForm,
			    XmNshadowType, XmSHADOW_IN,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_POSITION,
			    XmNleftPosition, 4,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition, 6,
			    NULL );
  threeDWidget->statusBarLabel2 =
    XtVaCreateManagedWidget("threeDWidgetStatusBarLabel2",
			    xmLabelGadgetClass, frame,
			    XmNalignment, XmALIGNMENT_BEGINNING,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    NULL);

  frame =
    XtVaCreateManagedWidget("threeDWidgetStatusFrame3",
			    xmFrameWidgetClass, statusForm,
			    XmNshadowType, XmSHADOW_IN,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_POSITION,
			    XmNleftPosition, 6,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition, 8,
			    NULL );
  threeDWidget->statusBarLabel3 =
    XtVaCreateManagedWidget("threeDWidgetStatusBarLabel3",
			    xmLabelGadgetClass, frame,
			    XmNalignment, XmALIGNMENT_BEGINNING,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    NULL);

  frame =
    XtVaCreateManagedWidget("threeDWidgetStatusFrame4",
			    xmFrameWidgetClass, statusForm,
			    XmNshadowType, XmSHADOW_IN,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_POSITION,
			    XmNleftPosition, 8,
			    XmNrightAttachment, XmATTACH_POSITION,
			    XmNrightPosition, 10,
			    NULL );
  threeDWidget->statusBarLabel4 =
    XtVaCreateManagedWidget("threeDWidgetStatusBarLabel4",
			    xmLabelGadgetClass, frame,
			    XmNalignment, XmALIGNMENT_BEGINNING,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    NULL);

  /* COSMETIC ATTACHMENTS */

  XtManageChild( statusForm );
  XtManageChild( bottomForm );
  
  XtVaSetValues( windowArea, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, statusForm, NULL );
  
  XtVaSetValues( mainForm, XmNinitialFocus, windowArea, NULL );
  
  /* OVERRIDE TRANSLATIONS FOR THE OUT THE WINDOW WINDOW */
  XtOverrideTranslations(windowArea, transTable);
}

/*
 * Synopsis: Widget ThreeDWidgetMainForm (ThreeDWidget const * threeDWidget)
 *	
 * Description: Return the main Widget.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson.
 * 
 * Date: November, 1997.
 *
 */
Widget
ThreeDWidgetMainForm (ThreeDWidget const * threeDWidget)
{
    return threeDWidget->mainForm;
}

/*
 * Synopsis:
 *     static void ThreeDWidgetCreateUtilButtons(ThreeDWidget *, Widget);
 *	
 * Description: Create utility buttons, such as "Dismiss', "help" etc.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	March 26, 1993
 *
 */
static void
ThreeDWidgetCreateUtilButtons(ThreeDWidget * threeDWidget,
			      Widget         formWidget)
{
/* static char * funcName = "ThreeDWidgetCreateUtilButtons";*/
 XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 int i, icon_numb = NUMB_WEB_ICONS - 1;
 XmString str;

    /* ALL OF THESE BUTTONS HAVE A CALLBACK TO 
     * ThreeDWidgetDismissCallback AND PASS A POINTER TO 
     * THE CORRESPONDING ThreeDWidget STRUCTURE
     */
    callbacks[0].callback = ThreeDWidgetDismissCallback;
    callbacks[0].closure = (XtPointer) threeDWidget;

    /* CREATE THE DISMISS BUTTON */
    str = tvXmString("Dismiss");
    XtVaCreateManagedWidget("ThreeDWidgetDismiss",
			    xmPushButtonGadgetClass,
			    formWidget,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNactivateCallback, callbacks,
			    XmNshowAsDefault, True,
			    XmNtopOffset, 25,
			    XmNbottomOffset, 25,
			    XmNlabelString, str,
			    NULL);
    XmStringFree (str);

    callbacks[0].callback = ThreeDWidgetMosaicCB;

    threeDWidget->mosaicWidgets[icon_numb] =
       XtVaCreateManagedWidget("ThreeDWidgetMosaic", 
			       xmPushButtonGadgetClass,
			       formWidget,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNactivateCallback, callbacks,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       NULL);

    for(i = 1; i < NUMB_WEB_ICONS; i++ ) {
      icon_numb--;
      threeDWidget->mosaicWidgets[icon_numb] =
	XtVaCreateManagedWidget("ThreeDWidgetMosaic", 
				xmPushButtonGadgetClass,
				formWidget,
				XmNrightAttachment, XmATTACH_WIDGET,
				XmNrightWidget,
				threeDWidget->mosaicWidgets[icon_numb+1],
				XmNtopAttachment, XmATTACH_FORM,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNactivateCallback, callbacks,
				NULL);
    }

    threeDWidget->pixmapsDone = FALSE; /* still gotta attach the pictures */
}


/*
 * Synopsis:
 *    static void ThreeDWidgetDismissCallback(Widget,
 *                                        XtPointer,
 *                                        XtPointer)
 *
 * Description: Callback function for actions such as "Dismiss" button,
 *         Function unmanages the threeDWidget and allows the user
 *         to select the overhead view.
 *
 * Externals: Widget menuThreeDButton - Used to turn on the overhead view
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
ThreeDWidgetDismissCallback(Widget    w,
			    XtPointer client_data,
			    XtPointer call_data)
{
/*  static char * funcName = "ThreeDWidgetDismissCallback";*/
  ThreeDWidget * threeDWidget;

  threeDWidget = (ThreeDWidget *) client_data;

  tvBaseStop (threeDWidget->threeD);

  /* POP DOWN ANY CONTROL PANELS */
  ThreeDControlUnmanageWidget (threeDWidget->threeDControl);

  if (threeDWidget->threeDPanel->managed[RENDER_PANEL]) {
    XtUnmanageChild(threeDWidget->threeDPanel->mainForm[RENDER_PANEL]);
    threeDWidget->threeDPanel->managed[RENDER_PANEL] = FALSE;
  }
  if (threeDWidget->threeDPanel->managed[DEBUG_PANEL]) {
    XtUnmanageChild(threeDWidget->threeDPanel->mainForm[DEBUG_PANEL]);
    threeDWidget->threeDPanel->managed[DEBUG_PANEL] = FALSE;
  }
  if (threeDWidget->threeDPanel->managed[NAV_PANEL]) {
    XtUnmanageChild(threeDWidget->threeDPanel->mainForm[NAV_PANEL]);
    threeDWidget->threeDPanel->managed[NAV_PANEL] = FALSE;
  }
  if (threeDWidget->threeDPanel->managed[POSITION_PANEL]) {
    XtUnmanageChild(threeDWidget->threeDPanel->mainForm[POSITION_PANEL]);
    threeDWidget->threeDPanel->managed[POSITION_PANEL] = FALSE;
  }

  ViewpointHide( threeDWidget );
  PathNewHide( threeDWidget );
  PathEditHide( threeDWidget );
  FeatureSelectHide( threeDWidget );
  DatasetPrioHide( threeDWidget );
  DatasetDisplayHide( threeDWidget );

  /* UNMANAGE THE WIDGET */
  XtUnmanageChild(threeDWidget->mainForm);

  /* get rid of it's structures */
  DestroyThreeDWidget(threeDWidget);

  /* Then update all the menus */
  LinkUpdateMenu();
}

/*
 * Synopsis:  void ThreeDWidgetDismiss(ThreeDWidget *)
 *
 * Description: Dismisses the out the window view
 *
 * Externals: NONE
 *
 * Returns:NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: May 1997
 *
 */
void
ThreeDWidgetDismiss (ThreeDWidget *threeDWidget)
{
  if ( ThreeDWidgetIsManaged( threeDWidget ) )
    ThreeDWidgetDismissCallback (threeDWidget->mainForm, threeDWidget, NULL);
}

/*
 * Synopsis: void ThreeDWidgetMosaicCB()
 *
 *
 * Description: When one of the mosaic buttongs get
 *   pressed we try to invoke netscape
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
 * Date:
 *		November 30, 1994
 *
 */
static void
ThreeDWidgetMosaicCB(Widget w,
		     XtPointer client_data,
		     XtPointer call_data)
{
/*  static char *funcName = "ThreeDWidgetMosaicCB";*/
  ThreeDWidget *threeDWidget;
  int i;

  threeDWidget = (ThreeDWidget *) client_data;

  for ( i = 0; i < NUMB_WEB_ICONS; i++ )
    if ( w == threeDWidget->mosaicWidgets[i] )
      tsmShowUrl( urlPaths[i] );
}

/*
 * Synopsis:
 *	ThreeDWidgetInitCB(Widget, XtPointer, XtPointer)
 *
 * Description: Callback called when the GL window is initialized. This
 *		occurs when the widget is first realized. Refer to O'Reilly
 *		for a definition of "widget realization."
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
 *		March 25, 1993
 *
 */
static void
ThreeDWidgetInitCB(Widget w,
		   XtPointer client_data,
		   XtPointer call_data)
{
  static char const funcName[] = "ThreeDWidgetInitCB";
  ThreeDWidget      *threeDWidget = (ThreeDWidget *) client_data;
  Screen            *scrn = XtScreen( w );
  int               winsize[2];
  XWindowAttributes attr;

  TSMTRACE ((THREED_MASK|1, funcName, "threeDWidget=%p", threeDWidget));
  TSMTRINC ((THREED_MASK|1, 1));

  /* Get the size of the window */

  XGetWindowAttributes( XtDisplay(w), XtWindow(w), &attr );
  winsize[0] = attr.width;
  winsize[1] = attr.height;

  /* Set the new size in the tvBase */

  tvSetIntegerv( threeDWidget->threeD, TV_SIZE, winsize );

  /* Get the max screen size */

  threeDWidget->screenMax[0] = scrn->width; 
  threeDWidget->screenMax[1] = scrn->height;

  /*
   * IF A SPACEBALL IS ATTACHED WE NEED
   * TO ADD THE SPACEBALL EVENTS.
   *
   */
#ifdef SPACEBALL_H
  if (SpaceBallAttached())
    SpaceBallAttachEvents(threeDWidget->windowArea);
#endif

  tvBaseStart (threeDWidget->threeD);

  TSMTRINC ((THREED_MASK|1, -1));
}


/*
 * Synopsis:
 *	void ThreeDWidgetResizeCB(Widget, XtPointer, XtPointer)
 *
 * Description: Callback called when a resize event occurs. The GL window
 *		is changed to reflect the new size. No redrawing needs to 
 *		occur here since a resize usually triggers an expose event.
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
 *		March 25, 1993
 *
 */
static void
ThreeDWidgetResizeCB(Widget    w,
			   XtPointer client_data,
			   XtPointer call_data)
{
/*  static char       *funcName = "ThreeDWidgetResizeCB";*/
  ThreeDWidget      *threeDWidget = (ThreeDWidget *) client_data;
  int               winsize[2];
  XWindowAttributes attr;

  /* Get the new window size */

  XGetWindowAttributes (XtDisplay(w), XtWindow(w), &attr);
  winsize[0] = attr.width;
  winsize[1] = attr.height;

  /* set the new size for the core */

  tvSetIntegerv( threeDWidget->threeD, TV_SIZE, winsize );
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
ThreeDWidgetExposeCB(Widget w, 		/* THE OPENGL WINDOW WIDGET */
		   XtPointer client_data,
		   XtPointer call_data)

{
  static char * funcName = "ThreeDWidgetExposeCB";
  ThreeDWidget * threeDWidget = (ThreeDWidget *) client_data;

  TSMTRACE ((THREED_MASK|1, funcName, "threeDWidget=%p", threeDWidget));

  if (! threeDWidget->threeD->started)
    tvBaseStart (threeDWidget->threeD);
  else
    tvBasePostRedraw (threeDWidget->threeD, False);
}

/*
 * Synopsis: Boolean ThreeDWidgetIsManaged(ThreeDWidget const *)
 *
 * Description: Determines whether or not widget is managed.
 *
 * Returns: TRUE iff widget is managed.
 *
 * Author: Lee Iverson
 *
 * Date: May 1997
 *
 */
Boolean
ThreeDWidgetIsManaged (ThreeDWidget const * threeDWidget)
{
  return glmIsMapped (threeDWidget->threeD->glmw);
}


/*
 * Synopsis: Boolean ThreeDWidgetPanel(ThreeDWidget const *)
 *
 * Description: Returns the 3D Panel,
 *
 * Returns: The panel associated with the widget structure.
 *
 * Author: Lee Iverson
 *
 * Date: May 1997
 *
 */
ThreeDPanel *
ThreeDWidgetPanel (ThreeDWidget const * threeDWidget)
{
  return threeDWidget->threeDPanel;
}

/*RLB: THIS SHOULD PROBABLY RESIDE IN CORE? UNCERTAIN AS TO WHAT UPDATEWINDOW
 * AND FULLSCREEN REQUIRE...*/
/*
 *
 * Synopsis:
 *   void ThreeDWidgetSetStereoMode(ThreeDWidget *, Bool)
 *
 * Description: set stereo modes
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
 *	04/29/99
 *
 */
void ThreeDWidgetSetStereoMode(ThreeDWidget *threeDWidget, int val)
{
  static Bool saveScreen, saveMenu, saveIcons, saveStatusBar;
  static Bool saved = False;
  Bool curr = tvGetBoolean (threeDWidget->threeD, TV_STEREO);
  if (val != curr) {
    if (val) {
      if ( tvuGetInt( TVU_STEREO ) )
	system("/usr/gfx/setmon -n 1025x768_96s");
      else {
	system("/usr/gfx/setmon -n STR_RECT");
	saveScreen = threeDWidget->showFullscreen;
	saveMenu = threeDWidget->showMenu;
	saveIcons = threeDWidget->showIcons;
	saveStatusBar = threeDWidget->showStatusBar;
	threeDWidget->showFullscreen = True;
	threeDWidget->showMenu = False;
	threeDWidget->showIcons = False;
	threeDWidget->showStatusBar = False;
	threeDWidget->threeD->showLogo = !threeDWidget->showIcons;
	saved = True;
	ThreeDWidgetUpdateWindow(threeDWidget);
	ThreeDWidgetFullScreenMode(threeDWidget, True);
      }
    }
    else {
      if (saved) {
	threeDWidget->showFullscreen = saveScreen;
	threeDWidget->showMenu = saveMenu;
	threeDWidget->showIcons = saveIcons;
	threeDWidget->showStatusBar = saveStatusBar;
	threeDWidget->threeD->showLogo = !threeDWidget->showIcons;
	saved = False;
	ThreeDWidgetUpdateWindow(threeDWidget);
	ThreeDWidgetFullScreenMode(threeDWidget, True);
      }
      system("/usr/gfx/setmon -n 72HZ");
    }
    tvSetBoolean (threeDWidget->threeD, TV_STEREO, val);
  }
}


/*
 * Synopsis:
 *	ThreeDWidget * WhichThreeDWidget(Widget w)
 *
 * Description:
 *    Maps a render widget to a specific ThreeDWidget by searching through
 *    the global list of ThreeDWidgets for a match.
 *
 * Externals: widgetList - the Widget list
 *
 * Author: Nat Bletter
 *
 * Date: Feb 22 1999
 *
 */
static ThreeDWidget *
WhichThreeDWidget(Widget w)
{
    list_iter_t i;

    /* gotta figure out which ThreeDWidget we're talking about here */
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
	ThreeDWidget *threeD = (ThreeDWidget*) list_value (i);
	if (threeD->windowArea == w) {
	    return threeD;
	}
    }
  
    /* didn't find anything */
    return NULL;
}

/*
 * Synopsis:
 *   void DrawOnScreen_BtnDn( ThreeDWidget *threeDWidget, int x, int y )
 *   void DrawOnScreen_BtnUp( ThreeDWidget *threeDWidget, int x, int y )
 *   void DrawOnScreen_Motion( ThreeDWidget *threeDWidget, int x, int y )
 *
 * Description:
 *   These three functions should be called whenever a pen
 *   up, down, or move event is generated. When a pen is
 *   depressed, we start a new stroke. When the pen is lifted
 *   we start the clocking ticking for the end of sentence
 *   time out. When the pen moves we add the pen coordinates
 *   to the current stroke list.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

static int buttonDown = FALSE;         /* true while mouse/pen button down */

static void
DrawOnScreen_BtnDn( ThreeDWidget *threeDWidget, int x, int y )
{
  buttonDown = TRUE;
  tvBeginStroke (threeDWidget->threeD, x, y);
}

static void
DrawOnScreen_BtnUp( ThreeDWidget *threeDWidget, int x, int y )
{
  buttonDown = FALSE;
  tvEndStroke (threeDWidget->threeD);
}

static void
DrawOnScreen_Motion( ThreeDWidget *threeDWidget, int x, int y )
{
  if ( buttonDown )
    tvContinueStroke (threeDWidget->threeD, x, y);
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetBtnUp(Widget,
 *                           XEvent *,
 *                           GLbyte * *,
 *                           Cardinal *);
 *
 * Description: Callback for when the user releases Button 1 of the mouse.
 *     Removes the timer for moving the viewer.
 *
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 *       April 8, 1993
 */
static XtActionProc
ThreeDWidgetBtnUp( Widget w, XEvent *generic_event, GLbyte **params,
		   Cardinal *num_params )
{
/*  static char  *funcName = "ThreeDWidgetBtnUp";*/
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeDWidget = WhichThreeDWidget(w);
  const char   *link_url;

  if (!threeDWidget)
    return NULL;

  /* no navigation during animation */
  if (threeDWidget->threeDPanel->seek || threeDWidget->altDown){
    /* not navigating */
    return NULL;
  }

  if ( PathIsPlaying(threeDWidget->threeD) ) return NULL;

  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_BtnUp( threeDWidget, event->x, event->y );
    return NULL;
  }

  /* did we clicked on a hyperlink? If so, unhighlight link for visual */
  /* feedback; also, make sure that we don't do the tvStartMovement()! */

  link_url = tvGetString( threeDWidget->threeD, TV_HYPERLINK_URL );
  if ( link_url ) {
    free( (char *) link_url );
    tvSetString( threeDWidget->threeD, TV_HYPERLINK_URL, NULL );
    ThreeDWidgetStatusBarClearView( threeDWidget );
    tvBasePostRedraw( threeDWidget->threeD, FALSE );
    return NULL;
  }

  /* Otherwise, we're navigating... */

  tvStartMovement (threeDWidget->threeD);

  /* Update the map view with the new position */
  ScannedMapRedraw();

  return NULL;
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetBtn2Up(Widget,
 *                            XEvent *,
 *                            GLbyte * *,
 *                            Cardinal *);
 *
 * Description: Callback for when the user releases Button 1 of the mouse.
 *     Removes the timer for moving the viewer.
 *
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 *       April 8, 1993
 */
static XtActionProc
ThreeDWidgetBtn2Up( Widget w, XEvent *generic_event, GLbyte **params,
		    Cardinal *num_params )
{
/*  static char         *funcName = "ThreeDWidgetBtn2Up";*/
  XButtonEvent        *event = (XButtonEvent *) generic_event;
  ThreeDWidget * threeDWidget = WhichThreeDWidget(w);

  if (!threeDWidget)
    return NULL;

  threeDWidget->mouseDown = FALSE;
  tvSetBoolean (threeDWidget->threeD, TV_SHOW_CROSSHAIR, FALSE);

  /* no navigation during animation */
  if ( PathIsPlaying(threeDWidget->threeD) ) return NULL;

  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_BtnUp( threeDWidget, event->x, event->y );
    return NULL;
  }

  tvStopMovement (threeDWidget->threeD);

  /* Update the map view with the new position */
  ScannedMapRedraw();
  return NULL;
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetBtn1Down(Widget,
 *                              XEvent *,
 *                              GLbyte * *,
 *                              Cardinal *);
 *
 * Description: Callback for when the user presses down Button 1 of the mouse.
 *    This function sets up a timer, which fires and accelerates the 
 *    viewer. As long as the button is held down, the timer is set. 
 *	
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *
 *       April 8, 1993
 */

static XtActionProc
ThreeDWidgetBtn1Down( Widget w, XEvent *generic_event, GLbyte **params,
		      Cardinal *num_params )
{
  static char  *funcName = "ThreeDWidgetBtn1Down";
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeDWidget = WhichThreeDWidget(w);
  const char   *link_url;

  if (!threeDWidget)
    return NULL;

  /* no navigation during animation */
  
  if ( PathIsPlaying(threeDWidget->threeD) ) return NULL;
 
  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeDWidget, event->x, event->y );
    return NULL;
  }

  /* are we seeking to a point on the terrain? */

  if (threeDWidget->threeDPanel->seek || threeDWidget->altDown) {
    tvFlyToPickPoint(threeDWidget->threeD, event->x, event->y);
    return NULL;
  }

  /* have we clicked on a hyperlink? */

  link_url = tvGetString( threeDWidget->threeD, TV_HYPERLINK_URL );
  if ( link_url ) {
    if ( strlen(link_url) > 0 ) tsmShowUrl( link_url );
    free( (char *) link_url );
    return NULL;
  }

  /* Otherwise, we're navigating */

  /* GET THE MOUSE POSITION */
  threeDWidget->xMouse = threeDWidget->xMouseDown = event->x;
  threeDWidget->yMouse = threeDWidget->yMouseDown = event->y;
  threeDWidget->mouseDown = TRUE;
  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", event->x, event->y));

  tvStartAccelerate (threeDWidget->threeD);

  {
    int mouse[2];
    mouse[0] = threeDWidget->xMouseDown;
    mouse[1] = threeDWidget->yMouseDown;
    tvSetIntegerv (threeDWidget->threeD, TV_CROSSHAIR, mouse);
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_CROSSHAIR, TRUE);
  }

  return NULL;
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetBtn2Down(Widget,
 *                              XEvent *,
 *                              GLbyte * *,
 *                              Cardinal *);
 *
 * Description: Callback for when the user presses down Button 2 of the mouse.
 *    This function sets up a timer, which fires and moves the 
 *    viewer. As long as the button is held down, the timer is set. 
 *	
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 *       April 9, 1993
 */
static XtActionProc
ThreeDWidgetBtn2Down( Widget w, XEvent *generic_event, GLbyte **params,
		      Cardinal *num_params )
{
  static char  *funcName = "ThreeDWidgetBtn2Down";
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget *threeDWidget = WhichThreeDWidget(w);
  float        d[2] = { 0.0, 0.0 };

  if (!threeDWidget)
    return NULL;

  /* no navigation during animation */

  if ( PathIsPlaying(threeDWidget->threeD) ) return NULL;

  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeDWidget, event->x, event->y );
    return NULL;
  }

  /* Otherwise, we're navigating */
  
  /* Set speed back to zero when we start */

  tvSetFloat( threeDWidget->threeD, TV_VELOCITY, 0.0 );
  tvSetFloat( threeDWidget->threeD, TV_ACCELERATION, 0.0 );
  tvSetFloatv( threeDWidget->threeD, TV_SLIDE_DELTA, d );
  tvSetFloatv( threeDWidget->threeD, TV_ATTITUDE_DELTA, d );

  /* find where mouse down occured, and origin of window */
  threeDWidget->xMouse = threeDWidget->xMouseDown = event->x;
  threeDWidget->yMouse = threeDWidget->yMouseDown = event->y;
  threeDWidget->mouseDown = TRUE;
  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", event->x, event->y));

  tvStartMovement (threeDWidget->threeD);

  {
    int mouse[2];
    mouse[0] = threeDWidget->xMouseDown;
    mouse[1] = threeDWidget->yMouseDown;
    tvSetIntegerv (threeDWidget->threeD, TV_CROSSHAIR, mouse);
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_CROSSHAIR, TRUE);
  }

  return NULL;
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetBtn3Down(Widget,
 *                              XEvent *,
 *                              GLbyte * *,
 *                              Cardinal *);
 *
 * Description: Callback for when the user presses down Button 3 of the mouse.
 *    This function sets up a timer, which fires and decclerates the 
 *    viewer. As long as the button is held down, the timer is set. 
 *	
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *
 *       April 8, 1993
 */
static XtActionProc
ThreeDWidgetBtn3Down( Widget w, XEvent *generic_event, GLbyte **params,
		      Cardinal *num_params )
{
  static char * funcName = "ThreeDWidgetBtn3Down";
  XButtonEvent *event = (XButtonEvent *) generic_event;
  ThreeDWidget * threeDWidget = WhichThreeDWidget(w);

  if (!threeDWidget)
    return NULL;

  /* no navigation during animation */

  if ( PathIsPlaying(threeDWidget->threeD) ) return NULL;

  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeDWidget, event->x, event->y );
    return NULL;
  }

  /* Otherwise, we're navigating */

  /* find where mouse down occured, and origin of window */
  threeDWidget->xMouse = threeDWidget->xMouseDown = event->x;
  threeDWidget->yMouse = threeDWidget->yMouseDown = event->y;
  threeDWidget->mouseDown = TRUE;
  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", event->x, event->y));

  tvStartDeccelerate (threeDWidget->threeD);

  {
    int mouse[2];
    mouse[0] = threeDWidget->xMouseDown;
    mouse[1] = threeDWidget->yMouseDown;
    tvSetIntegerv (threeDWidget->threeD, TV_CROSSHAIR, mouse);
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_CROSSHAIR, TRUE);
  }

  return NULL;
}

/*
 * Synopsis:
 *    static XtActionProc
 *         ThreeDWidgetMotion(Widget,
 *                              XEvent *,
 *                              GLbyte * *,
 *                              Cardinal *);
 *
 * Description: Callback that is called whenever the user moves the
 *     mouse.
 *	
 * Externals: ThreeDWidget * threeD
 *     No other way to get around this with X, unfortunately. 
 *     You can't pass parameters in to a ActionProc.
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *
 *       April 8, 1993
 */

static XtActionProc
ThreeDWidgetMotion( Widget w, XEvent *generic_event, GLbyte **params,
		    Cardinal *num_params )
{
  static char  *funcName = "ThreeDWidgetMotion";
  XMotionEvent *event = (XMotionEvent *) generic_event;
  ThreeDWidget *threeDWidget = WhichThreeDWidget(w);
  tvBase       *threeD;
  const char   *link_desc;

  if (!threeDWidget)
    return NULL;

  /* are we drawing on the screen rather than navigating? */

  if ( threeDWidget->threeDPanel->drawOnScreen ) {
    DrawOnScreen_Motion( threeDWidget, event->x, event->y );
    return NULL;
  }

  /* Determine the core object */
  threeD = threeDWidget->threeD;

  /* otherwise, we're navigating */
  if (threeDWidget->mouseDown || threeDWidget->shiftDown || 
      threeDWidget->ctrlDown  || threeDWidget->altDown) {
    float scale = 0.5 * ((threeD->width > threeD->height)
			  ? threeD->width : threeD->height);
    float d[2];

    d[0] =  ((event->x - threeDWidget->xMouseDown) / scale);
    d[1] = -((event->y - threeDWidget->yMouseDown) / scale);
    
    threeDWidget->xMouse = event->x;
    threeDWidget->yMouse = event->y;

    if (threeDWidget->shiftDown || threeDWidget->ctrlDown
	|| threeD->twoDConstrain)
      tvSetFloatv (threeD, TV_SLIDE_DELTA, d);
    else {
      if (! threeDWidget->airplaneControl)
	d[1] *= -1; 
      tvSetFloatv (threeD, TV_ATTITUDE_DELTA, d);
    }

    /* clear the Viewpoint field of the status bar (if set), because */
    /* if we have moved then we are no longer at that viewpoint.     */

    ThreeDWidgetStatusBarClearView( threeDWidget );

    TSMTRACE ((THREED_MASK|2, funcName, "%d,%d", event->x, event->y));

  } else {

    /* send an event to find the hyperlink under the   */
    /* new cursor position, but only if we are moving. */

    if ( ! tvGetBoolean( threeD, TV_MOVING ) )
      tvBaseCheckForHyperlink( threeD, event->x, event->y );

    /* get a description for the hyperlink under cursor, if any */
 
    if ( ( link_desc = tvGetString( threeD, TV_HYPERLINK_DESC ) ) == NULL )
      link_desc = tvGetString( threeD, TV_HYPERLINK_URL );

    /* update the status bar with that description */

    if ( link_desc == NULL )
      ThreeDWidgetStatusBarClearView( threeDWidget );
    else {
      ThreeDWidgetUpdateStatusBar( threeDWidget, link_desc );
      free( (char *) link_desc );
    }

  }

  return NULL;
}

/*
 * Synopsis:
 *    XtActionProc ThreeDWidgetKeypress(Widget,XEvent *,GLbyte **,Cardinal *);
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
ThreeDWidgetKeypress( Widget w, XEvent *generic_event, GLbyte **params,
		      Cardinal *num_params )
{
  static char * funcName = "ThreeDWidgetKeypress";
  XKeyEvent *event = (XKeyEvent *) generic_event;
  ThreeDWidget * threeDWidget = WhichThreeDWidget(w);

  tvBase * threeD;
  ThreeDPanel * threeDPanel;
  int grabKeyboard = 0;

  /* these delta are used for the cursor key movements */

  double pitch_delta = 2.0 * M_PI / 180.0;
  double yaw_delta   = 2.0 * M_PI / 180.0;
  double slide_delta = 50.0;
  double zoom_delta  = 50.0;

  if (! threeDWidget) return NULL;
  threeD = threeDWidget->threeD;
  if (! threeD) return NULL;
  threeDPanel = threeDWidget->threeDPanel;

  /* reverse action of up/down cursor keys depending on airplane controls */
  if ( ! threeDWidget->airplaneControl )
    pitch_delta *= -1.0; 

  switch ( XLookupKeysym( event, 0 ) ) {

  case XK_space:
    tvStopMotion( threeD );
    break;

    /* Escape = close 3-D viewer window */

  case XK_Escape:
    ThreeDWidgetDismiss( threeDWidget );
    return NULL;

    /* 2 = go to 2-D constrain mode */

  case XK_2:
    tvSetBoolean (threeD, TV_TWOD_CONSTRAIN, TRUE);
    ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
    XtVaSetValues( threeDPanel->constrainWidget, XmNset, True, NULL );
    break;

    /* 3 = go to 3-D mode */

  case XK_3:
    tvSetBoolean (threeD, TV_TWOD_CONSTRAIN, FALSE);
    ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
    XtVaSetValues( threeDPanel->constrainWidget, XmNset, False, NULL );
    break;

    /* M = toggle menu bar on or off */

  case XK_M:
  case XK_m:
    threeDWidget->showMenu = !threeDWidget->showMenu;
    ThreeDWidgetUpdateWindow( threeDWidget );
    break;

    /* W = toggle web icon panel on or off */
    
  case XK_W:
  case XK_w:
    threeDWidget->showIcons = !threeDWidget->showIcons;
    tvSetBoolean (threeD, TV_SHOW_LOGO, !threeDWidget->showIcons);
    ThreeDWidgetUpdateWindow( threeDWidget );
    break;

    /* A = toggle status bar on or off */
    
  case XK_A:
  case XK_a:
    threeDWidget->showStatusBar = !threeDWidget->showStatusBar;
    ThreeDWidgetUpdateWindow( threeDWidget );
    break;

    /* L = toggle SRI logo on or off */
    
  case XK_L:
  case XK_l:
    threeD->showLogo = ! threeD->showLogo;
    tvBasePostRedraw( threeD, FALSE );
    break;

    /* F = toggle full screen mode */

  case XK_F:
  case XK_f:
    threeDWidget->showFullscreen = !threeDWidget->showFullscreen;
    ThreeDWidgetFullScreenMode( threeDWidget, TRUE );
    break;

    /* R = display the Render Settings window */

  case XK_R:
  case XK_r:
    ThreeDPanelManageWidget( threeDWidget->threeDPanel, RENDER_PANEL );
    break;

    /* N = display the Navigation Settings window */
    /* CTRL + N = Goto Next path waypoint */

  case XK_N:
  case XK_n:
    if ( threeDWidget->ctrlDown )
      PathGotoNext( threeD );
    else
      ThreeDPanelManageWidget( threeDWidget->threeDPanel, NAV_PANEL );
    break;

    /* D = display the Debug Settings window */

  case XK_D:
  case XK_d:
    ThreeDPanelManageWidget( threeDWidget->threeDPanel, DEBUG_PANEL );
    break;

    /* C = display the Viewpoint Controls window */

  case XK_C:
  case XK_c:
    ThreeDControlManageWidget(threeDWidget->threeDControl);
    break;

    /* V = display the VRML Feature Selection window */
    /* CTRL + V = Goto Previous path waypoint */

  case XK_V:
  case XK_v:
    if ( threeDWidget->ctrlDown )
      PathGotoPrev( threeD );
    else
      FeatureSelectShow( threeDWidget );
    break;

    /* P = display the Viewpoint Position window */
    /* CTRL + P = Play Path */

  case XK_P:
  case XK_p:
    if ( threeDWidget->ctrlDown )
      PathPlay( threeDWidget->threeD );
    else
      ThreeDPanelManageWidget( threeDWidget->threeDPanel, POSITION_PANEL );
    break;

    /* CTRL + U = Play Pause/Continue */

  case XK_U:
  case XK_u:
    if ( threeDWidget->ctrlDown )
      PathPause( threeDWidget->threeD );
    break;

    /* CTRL + T = Goto Start of Path */

  case XK_T:
  case XK_t:
    if ( threeDWidget->ctrlDown )
      PathGotoStart( threeDWidget->threeD );
    break;

    /* CTRL + E = Goto End of Path */

  case XK_E:
  case XK_e:
    if ( threeDWidget->ctrlDown )
      PathGotoEnd( threeDWidget->threeD );
    break;

    /* B = display the Viewpoint Bookmarks window */

  case XK_B:
  case XK_b:
    ViewpointShow( threeDWidget );
    break;

    /* O = reset to origin */

  case XK_O:
  case XK_o:
    tvBaseInitView(threeDWidget->threeD, tvBaseTop (threeDWidget->threeD));
    break;

    /* I = Dataset Priority */

  case XK_I:
  case XK_i:
    DatasetPrioShow( threeDWidget );
    break;

    /* H = toggle the HUD in debug mode */

    /* H = toggle the HUD */

  case XK_H:
  case XK_h: {
    PRBool headsUp;
    if (threeDWidget->shiftDown) {
      PRBool headsUpDebug = tvGetBoolean (threeDWidget->threeD, 
					  TV_SHOW_HEADS_UP_DEBUG);
      tvSetBoolean (threeDWidget->threeD, TV_SHOW_HEADS_UP_DEBUG, 
		    !headsUpDebug);
      headsUp = tvGetBoolean (threeDWidget->threeD, TV_SHOW_HEADS_UP);
    } else {
      headsUp = tvGetBoolean (threeDWidget->threeD, TV_SHOW_HEADS_UP);
      tvSetBoolean (threeDWidget->threeD, TV_SHOW_HEADS_UP, ! headsUp);
      headsUp = !headsUp;
    }
    XtVaSetValues( threeDPanel->hudWidget, XmNset, headsUp, NULL);
    break;
  }
    /* - = toggle z-buffer hack */

  case XK_minus: {
    Boolean val = ! tvGetBoolean (threeDWidget->threeD, TV_USE_ZBUFFER_HACK);
    tvSetBoolean (threeDWidget->threeD, TV_USE_ZBUFFER_HACK, val);
    XtVaSetValues(threeDWidget->zbufferButton, XmNset, val, NULL);
    break;
  }

    /* X = Freeze tiles */

  case XK_X:
  case XK_x: {
    Boolean val = ! tvGetBoolean (threeDWidget->threeD, TV_FREEZE_TILES);
    tvSetBoolean (threeDWidget->threeD, TV_FREEZE_TILES, val);
    XtVaSetValues(threeDWidget->freezeButton, XmNset, val, NULL);
    XtSetSensitive (threeDWidget->threeDPanel->freezeReturn, val);
    break;
  }

    /* Y = Dataset Display */

  case XK_Y:
  case XK_y:
    DatasetDisplayShow( threeDWidget );
    break;

    /* S = turn stereo mode on/off */
    /* CTRL+S = Path Stop */

  case XK_S:
  case XK_s:
    if ( threeDWidget->ctrlDown ) 
      PathStop( threeDWidget->threeD );
    else {
      int prev = tvGetBoolean (threeDWidget->threeD, TV_STEREO);
      ThreeDWidgetSetStereoMode(threeDWidget, !prev);
    }
    break;

    /* turn stereo monitor mode off manually */

  case XK_Z:
  case XK_z:
    system("/usr/gfx/setmon -n 72HZ");
    break;

    /* Left Arrow = rotate or slide to the left */

  case XK_Left:
  case XK_KP_Left:
    if ( threeDWidget->shiftDown )
      tvMoveViewpoint( threeD, 0, -slide_delta, 0, FALSE );
    else
      tvRotate( threeD, yaw_delta, 0, 0, FALSE );
    break;

    /* Right Arrow = rotate or slide to the right */

  case XK_Right:
  case XK_KP_Right:
    if ( threeDWidget->shiftDown )
      tvMoveViewpoint( threeD, 0, slide_delta, 0, FALSE );
    else
      tvRotate( threeD, -yaw_delta, 0, 0, FALSE );
    break;

    /* Up Arrow = increase pitch, slide, or zoom */

  case XK_Up:
  case XK_KP_Up:
    if ( threeDWidget->shiftDown )
      tvMoveViewpoint( threeD, 0, 0, -slide_delta, FALSE );
    else if ( threeDWidget->ctrlDown )
      tvMoveViewpoint( threeD, zoom_delta, 0, 0, FALSE );
    else
      tvRotate( threeD, 0, -pitch_delta, 0, FALSE );
    break;

    /* Down Arrow = decrease pitch, slide, or zoom */

  case XK_Down:
  case XK_KP_Down:
    if ( threeDWidget->shiftDown )
      tvMoveViewpoint( threeD, 0, 0, slide_delta, FALSE );
    else if ( threeDWidget->ctrlDown )
      tvMoveViewpoint( threeD, -zoom_delta, 0, 0, FALSE );
    else
      tvRotate( threeD, 0, pitch_delta, 0, FALSE );
    break;

    /* PgUp = fly to next viewpoint in bookmark list */

  case XK_Page_Up:
  case XK_KP_Page_Up:
    ViewpointCycleUp( threeDWidget );
    break;

    /* PgUp = fly to previous viewpoint in bookmark list */

  case XK_Page_Down:
  case XK_KP_Page_Down:
    ViewpointCycleDown( threeDWidget );
    break;

    /* Record whether the shift key is currently depressed */

  case XK_Shift_L:
  case XK_Shift_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press SHIFT"));
    threeDWidget->shiftDown = TRUE;
    grabKeyboard = (! threeDWidget->ctrlDown && ! threeDWidget->altDown);
    break;

    /* Record whether the ctrl key is currently depressed */

  case XK_Control_L:
  case XK_Control_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press CTRL"));
    threeDWidget->ctrlDown = TRUE;
    grabKeyboard = (! threeDWidget->shiftDown && ! threeDWidget->altDown);
    break;

    /* Record whether the alt key is currently depressed */

  case XK_Alt_L:
  case XK_Alt_R:
    TSMTRACE ((THREED_MASK|1, funcName, "Press ALT"));
    threeDWidget->altDown = TRUE;
    grabKeyboard = (! threeDWidget->ctrlDown && ! threeDWidget->shiftDown);
    break;

  }

  if (grabKeyboard) {
    TSMTRACE ((THREED_MASK|1, funcName, "Grab keyboard"));
    XtGrabKeyboard (w, False, GrabModeAsync, GrabModeAsync, CurrentTime);
  }

  ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );

  return NULL;
}

/*
 * Synopsis:
 *    XtActionProc ThreeDWidgetKeyrelease(Widget,XEvent *,GLbyte **,Cardinal *)
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
ThreeDWidgetKeyrelease( Widget w, XEvent *generic_event, GLbyte **params,
			Cardinal *num_params )
{
  static char const funcName[] = "ThreeDWidgetKeypress";
  XKeyEvent *event = (XKeyEvent *) generic_event;
  ThreeDWidget *threeD = WhichThreeDWidget(w);

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

  ThreeDWidgetUpdateStatusBar( threeD, NULL );

  return NULL;
}

/*
 *
 * Synopsis:
 *	static void ThreeDWidgetUpdate(ThreeDWidget *)
 *
 * Description: Based upon where the mouse is in the viewport,
 *      point the viewer in that direction.
 *      The viewport is divided into 4 sections. As the mouse
 *      moves closer to one side, the viewer will "turn" faster.
 *
 *      Top - Point the nose up
 *      Bottom - Point the nose down
 *      Left - Turn toward the left
 *      RIght - Turn toward the right
 *
 * Externals: NONE - Just Say No to Globals
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	03/29/93
 *
 */
void
ThreeDWidgetUpdate(ThreeDWidget * threeDWidget)
{
  static char *funcName = "ThreeDWidgetUpdate";
  double mouseX, mouseY;
  tvViewpoint view_s, *view = &view_s;
  double heightOld, heightNew;
  int moved = 0;

  TSMTRACE ((THREED_MASK|1, funcName, "Updating"));
  TSMTRINC ((THREED_MASK|1, 1));

  tvCurrentView (threeDWidget->threeD, view);

  /* FIND DISTANCE FROM POINT OF MOUSE DOWN */
  mouseX = threeDWidget->xMouseDown - threeDWidget->xMouse;
  mouseY = threeDWidget->yMouseDown - threeDWidget->yMouse;

  if (! threeDWidget->airplaneControl)
    mouseY *= -1; 

  TSMTRACE ((THREED_MASK|2, funcName, "%4.4f/%d %4.4f/%d%s%s%s", 
	     mouseX, threeDWidget->threeD->width,
	     mouseY, threeDWidget->threeD->height,
	     threeDWidget->shiftDown ? " Shift" : "",
	     threeDWidget->ctrlDown ? " Ctrl" : "",
	     threeDWidget->altDown ? " Alt" : ""));

  heightOld = view->altitude;

  if ( heightOld < 10.0 ) heightOld = 10.0;

  /* If the user has SHIFT down while moving, then the behaviour */
  /* of the navigation changes to:                               */
  /*   LMB = move forward along current view direction           */
  /*   RMB = move backward along current view direction          */
  /*   move = slide up/down/left/right relative to click point   */

  if ( threeDWidget->shiftDown
       || threeDWidget->threeD->twoDConstrain
       || threeDWidget->ctrlDown) {
    if (threeDWidget->threeD->heightBasedNav)
      if ( threeDWidget->threeD->twoDConstrain ) 
	tvAdjustView (threeDWidget->threeD, view, view->vel * 0.005,
		      mouseX * -0.01 * heightOld / ACCEL_BASE_HEIGHT, 
		      mouseY * -0.01 * heightOld / ACCEL_BASE_HEIGHT);
      else
	tvAdjustView (threeDWidget->threeD, view, view->vel * 0.1,
		      mouseX * -0.05 * heightOld / ACCEL_BASE_HEIGHT, 
		      mouseY * -0.05 * heightOld / ACCEL_BASE_HEIGHT);
    else
      tvAdjustView (threeDWidget->threeD, view, view->vel * 0.1,
		    mouseX * -0.1, mouseY * -0.1);
    moved = 1;

  } else {

    /* TURN THE VIEWER VIEWPOINT BASED UPON THE MOUSE LOCATION */
    /* view->roll =  -MAX_ROLL * mouseX / (threeDWidget->threeD->width / 2.0);*/
    if (threeDWidget->ctrlDown) {
      view->yaw   += (M_PI / 180.0) * -mouseX / 
	  (threeDWidget->threeD->width  / 2.0);
      view->pitch += (M_PI / 180.0) *  mouseY / 
	  (threeDWidget->threeD->height / 2.0);
    }
    else if (! threeDWidget->threeD->twoDConstrain) {
      view->yaw   += (M_PI / 180.0) *  mouseX / 
	  (threeDWidget->threeD->width  / 2.0);
      view->pitch += (M_PI / 180.0) * -mouseY / 
	  (threeDWidget->threeD->height / 2.0);
    }

  }

  TSMTRACE ((THREED_MASK|2, funcName, "roll %.1f pitch %.1f yaw %.1f",
	     view->roll*180.0/M_PI,
	     view->pitch*180.0/M_PI,
	     view->yaw*180.0/M_PI));
  TSMTRACE ((THREED_MASK|2, funcName, "vel %5.2f accel %5.2f",
	     view->vel, view->accel));

  if (moved && threeDWidget->threeD->heightBasedNav) {
    /* compute the new speed based on the elevation */
    heightNew = tvBasePointAltitude (threeDWidget->threeD, view->eye);
    if ( heightNew < 10.0 ) heightNew = 10.0;
    view->vel *= heightNew / heightOld;
  }

  TSMTRACE ((THREED_MASK|2, funcName, "eye %.0f,%.0f,%.0f",
	     view->eye[0], view->eye[1], view->eye[2]));

  /* UPDATE THE ACTIVE VIEWPOINT. */
  tvSetView (threeDWidget->threeD, view, FALSE);

  /* ASK FOR A REDRAW TO BE PERFORMED ONLY IF TILES ARE FROZEN,
     OTHERWISE VISIBILITY WILL TAKE CARE OF IT. */
  if (threeDWidget->threeD->freezeTiles)
    tvBasePostRedraw (threeDWidget->threeD, False);

  TSMTRINC ((THREED_MASK|1, -1));
}


#ifdef SPACEBALL_H
/*
 * Synopsis: void ThreeDWidgetProcessSbEvent()
 *	
 *
 * Description: Processes Spaceball events
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
 * Date: 1993
 *
 */
void 
ThreeDWidgetProcessSbEvent(ThreeDWidget * threeDWidget,
			   XEvent *event)
{
 static char * funcName = "ThreeDWidgetProcessSbEvent";
 int butno;		/* the button pressed */
 int i;
 static short sbdata[7] = {0};
 static long last_time = 0, guess_time = TRUE;
 XDeviceButtonEvent *B, *b;
 tvViewpoint view;

    tvBaseGetView (threeDWidget->threeD, &view);

    if ( event->type == sb_event_type ) 
    {
	/* these are spaceball motion events */
	XDeviceMotionEvent *M = (XDeviceMotionEvent *) event;
	if ( M->deviceid == sb_device->device_id ) 
	{
	    for ( i = 0; i < M->axes_count; i++ )
	    {
		*(sbdata+M->first_axis+i) = M->axis_data[i];
	    }

	    /* Calculate the PERIOD (16th's of a millisecond) */
	    sbdata[6] = (short) (16 * (guess_time ? 50 :
				       M->time - last_time));
	    last_time = M->time;

	    guess_time = (sbdata[0] == 0 && sbdata[1] == 0 &&
			  sbdata[2] == 0 && sbdata[3] == 0 &&
			  sbdata[4] == 0 && sbdata[5] == 0);

	    ThreeDWidgetProcessSbData (threeDWidget, sbdata);
	}
    }
    else if (event->type == sb_button_press_event_type)
    {
	/* these are spaceball button presses */
        B = (XDeviceButtonEvent *)event;
	butno = B->button;
    }
    else if (event->type == sb_button_release_event_type) 
    {

	/* these are spaceball button releases */
	b = (XDeviceButtonEvent *) event;
	butno = b->button;

	switch (butno)
	{
	    case SB_BUTTON1:
/*
                view.eye[0] -= 100.0;
                tvSetView(threeDWidget->threeD, &view);
                tsmStatus("%s: In ShiftZ Key",
                          funcName);
*/
	    	break;

	    case SB_BUTTON2:
		if(threeDWidget->threeD->flyMode)
		{
		    view.vel += 5 * view.base_accel;
		    tsmStatus("%s: Accelerating..", funcName);
		    tvSetView(threeDWidget->threeD, &view);
		}
		break;

	    case SB_BUTTON3:
		if(threeDWidget->threeD->flyMode)
		{
		    view.vel -= 5 * view.base_accel;
		    tsmStatus("%s: Deccelerating..", funcName);
		    tvSetView(threeDWidget->threeD, &view);
		}
		break;

	    case SB_BUTTON4:
		threeDWidget->threeD->flyMode = !threeDWidget->threeD->flyMode;
		fprintf(stderr,
			(threeDWidget->threeD->flyMode)? "Flying\n" : "Static\n");
		break;

	    case SB_BUTTON5:
	    	view.pitch = M_PI_2;
	    	view.roll = 0;
            	tvSetView(threeDWidget->threeD &view);
	    	break;

	    case SB_BUTTON6:
/*
	    	printf ( "Spaceball Sensitivity Decreased\n" );
	    	threeDWidget->threeD->sensitivity += .01;
	   	 if(threeDWidget->threeD->sensitivity > 1.0)
	    	{
		    threeDWidget->threeD->sensitivity = 1.0;
	        }
	    	tsmStatus("%s: %f",
 		          funcName, 
			  threeDWidget->threeD->sensitivity);
*/
		view.eye[2] -= 100.0;
		tvSetView(threeDWidget->threeD, &view);
		tsmStatus("%s: In ShiftZ Key", funcName);
	    	break;

	    case SB_BUTTON7:
/*
	    	printf ( "Spaceball Sensitivity Increased\n" );
	    	threeDWidget->threeD->sensitivity -= .01;
	    	if(threeDWidget->threeD->sensitivity < 0)
	    	{
		    threeDWidget->threeD->sensitivity = 0;
	        }
	        tsmStatus("%s: %f", funcName,  threeDWidget->threeD->sensitivity);
*/
		view.eye[2] -= 100.0;
           	tvSetView(threeDWidget->threeD, &view);
		tsmStatus("%s: In ShiftZ Key", funcName);
	        break;

	    case SB_BUTTON8:
		ThreeDWidgetDumpView(threeDWidget, &threeDWidget->view);
/*
	    	printf ( "Spaceball Rezereod\n" );
   	    	sbrezero();
*/
	    	break;

	    case SB_PICK:
	    	tvBaseInitView(threeDWidget->threeD);
	    	break;
	}
/*
	sbbeep("c");
*/
    }
    else
	printf ( "spaceball event type problem\n" );
}

/*****************************************************************************
 * Synopsis:
 *	
 *
 * Description:
 *     extract from the spaceball values the current incremental
 *     change in the viewpoint and reference point
 *
 *
 * PARAMETERS: short *sbvals		array of spaceball values
 *	       GLfloat *view		the current viewpoint
 *	       GLfloat *ref		the current reference point
 *
 * CALLS:      linearize()
 *	       normalize()
 * COMMENTS:   sbvals[0] - SBTX
 *	       sbvals[1] - SBTY
 *	       sbvals[2] - SBTZ
 *	       sbvals[3] - SBRX
 *	       sbvals[4] - SBRY
 *	       sbvals[5] - SBRZ
 *	       sbvals[6] - SBPERIOD
 * Externals:
 *
 *
 * Returns:  void
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 *****************************************************************************/
static void
ThreeDWidgetProcessSbData(ThreeDWidget * threeDWidget,
			  short *sbvals)
{
  static char * funcName = "ThreeDWidgetProcessSbData";
  double	translation[3];	/* normalized translation SB value */
  double	rotation[3];	/* normalize rotation about x,y, z SB value */
				/* after normalization:  -1 <= rot <= 1 */
  double        MaxSBRot[3];
  double        MaxSBTrans[3];

  double	SBRotYScale = M_PI / 16.0;
  double	SBRotXScale = M_PI / 32.0;

  double	SBTransScale[3];
  double 	radius;
  double	radXY;
  int 		i;
  tvViewpoint	view_s, *view = &view_s;
  double 	pitch, yaw;

    for(i = 0; i < 3; i++)
    {
	MaxSBRot[i] = 1000;
        MaxSBTrans[i] = 1000;
	SBTransScale[i] = 1000;
    }

    /* pseudo linearize the important spaceball values */
    for(i = 0; i < 3; i++)
    {
	rotation[i] = linearize((const)sbvals[i + 3], MaxSBRot[i]);
    	translation[i] = linearize((const)sbvals[i], MaxSBTrans[i]);
    }
    if(fabs(rotation[0]) < threeDWidget->threeD->sensitivity &&
       fabs(rotation[1]) < threeDWidget->threeD->sensitivity &&
       fabs(rotation[2]) < threeDWidget->threeD->sensitivity &&
       fabs(translation[0]) < threeDWidget->threeD->sensitivity &&
       fabs(translation[1]) < threeDWidget->threeD->sensitivity &&
       fabs(translation[2]) < threeDWidget->threeD->sensitivity)
    {
	return;
    }

    tvBaseGetView (threeDWidget->threeD, view);

    view->roll += MAX_ROLL * rotation[2];
    view->yaw += SBRotYScale * rotation[1];
    view->pitch -= SBRotXScale * rotation[0];

    if(!threeDWidget->threeD->flyMode)
    {
        radius = sqrt(((SBTransScale[0] * translation[0]) *
		       (SBTransScale[0] * translation[0])) +
		      ((SBTransScale[1] * translation[1]) *
		       (SBTransScale[1] * translation[1])) +
	      	      ((SBTransScale[2] * translation[2]) *
		       (SBTransScale[2] * translation[2])));

	radXY = sqrt(((SBTransScale[0] * translation[0]) *
		      (SBTransScale[0] * translation[0])) +
  	      	      ((SBTransScale[2] * translation[2]) *
		       (SBTransScale[2] * translation[2])));

	theta = atan2(translation[2], translation[0]);
	phi = atan2(translation[1], radXY);

	yaw = atan2(translation[1], translation[0]);
	pitch = atan2(translation[2], radXY);

	view->eye[0] += -radius * sin(pitch - view->pitch) *
	  sin(yaw - view->yaw);
	view->eye[1] += -radius * sin(pitch - view->pitch) *
	  cos(yaw - view->yaw); 
	view->eye[2] += radius * cos(pitch - view->pitch);
    }

    tvSetView(threeDWidget->threeD, view);
}
#endif /* SPACEBALL_H */

/*
 * Synopsis:
 *   void ThreeDWidgetRefreshDialogs( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   This function will refresh all of the dialogs that are 
 *   associated with a ThreeDWidget, e.g. the Bookmarks dialog,
 *   the Feature Selection dialog, the Status Bar, etc. This is
 *   the one stop function for updating everything if you change
 *   something major such as loading or unloading a dataset.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

void
ThreeDWidgetRefreshDialogs( ThreeDWidget *threeDWidget )
{
  FeatureSelectRefresh( threeDWidget );

  ViewpointRefreshWidget( threeDWidget );

  DatasetPrioRefreshWidget( threeDWidget );

  DatasetDisplayRefreshWidget( threeDWidget );

  ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
}

static void
ThreeDWidgetUpdateWindow( ThreeDWidget *threeDWidget )
{

  /* remember the window size/location before we change its display */

  if ( ! threeDWidget->showFullscreen )
    ThreeDWidgetRememberWinSize( threeDWidget );

  /* Do we want to display the web icons at the bottom of the screen? */

  if ( threeDWidget->showIcons )
    XtManageChild( threeDWidget->bottomForm );
  else
    XtUnmanageChild( threeDWidget->bottomForm );

  XtVaSetValues( threeDWidget->showIconsWidget,
		 XmNset, threeDWidget->showIcons, NULL );

  /* Do we want to display the status bar at the bottom of the screen? */

  if ( threeDWidget->showStatusBar ) {
    XtManageChild( threeDWidget->statusBarForm );

    if ( threeDWidget->showIcons )
      XtVaSetValues( threeDWidget->statusBarForm, XmNbottomAttachment,
		     XmATTACH_WIDGET, XmNbottomWidget,
		     threeDWidget->bottomForm, NULL );
    else
      XtVaSetValues( threeDWidget->statusBarForm, XmNbottomAttachment,
		     XmATTACH_FORM, NULL );

    XtVaSetValues( threeDWidget->windowArea, XmNbottomAttachment,
		   XmATTACH_WIDGET, XmNbottomWidget,
		   threeDWidget->statusBarForm, NULL );
  } else {
    XtUnmanageChild( threeDWidget->statusBarForm );

    if ( threeDWidget->showIcons )
      XtVaSetValues( threeDWidget->windowArea, XmNbottomAttachment,
		     XmATTACH_WIDGET, XmNbottomWidget,
		     threeDWidget->bottomForm, NULL );
    else
      XtVaSetValues( threeDWidget->windowArea, XmNbottomAttachment,
		     XmATTACH_FORM, NULL );

  }

  XtVaSetValues( threeDWidget->showStatusBarWidget,
		 XmNset, threeDWidget->showStatusBar, NULL );

  /* Do we want to display the menu bar just above the render window */

  if ( threeDWidget->showMenu ) {
    XtManageChild(threeDWidget->topForm);
    XtVaSetValues( threeDWidget->windowArea, XmNtopAttachment, XmATTACH_WIDGET,
		   XmNtopWidget, threeDWidget->topForm, NULL );
  } else {
    XtUnmanageChild(threeDWidget->topForm);
    XtVaSetValues( threeDWidget->windowArea, XmNtopAttachment, XmATTACH_FORM,
		   NULL );
  }

  XtVaSetValues( threeDWidget->showMenuWidget,
		 XmNset, threeDWidget->showMenu, NULL );

  /* Refresh the screen size */

  ThreeDWidgetFullScreenMode( threeDWidget, FALSE );

  /* force a redraw */

  tvBasePostRedraw (threeDWidget->threeD, FALSE);
}


/*
 * Synopsis: Widget ThreeDWidgetCreateMenuBar()
 *	
 *
 * Description: Creates the menu for the window
 *
 *
 * Externals:
 *
 *
 * Returns: The menu bar
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: July 1994
 *
 */
static Widget
ThreeDWidgetCreateMenuBar(ThreeDWidget * threeDWidget, Widget parent)
{
/* static char  *funcName = "ThreeDWidgetCreateMenuBar";*/
 Widget       cascade, menuPane, menuBar, widget;
 Arg          args[10];
 XmString     accel;
 unsigned     int n;
 static XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};

    n = 0;
    XtSetArg(args[n], XmNspacing, 15); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;

    menuBar = XmCreateMenuBar(parent, "menuBar", args, n);
    XtManageChild(menuBar);

    callbacks[0].callback = ThreeDWidgetMenuCB;
    callbacks[0].closure = (XtPointer) threeDWidget;

    /* CREATE THE "Datasets" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    accel = XmStringCreateLocalized( "Y" );
    threeDWidget->datasetDisplayButton =
      XtVaCreateManagedWidget( "Dataset Display",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'Y', NULL );
    XmStringFree (accel);

    accel = XmStringCreateLocalized( "I" );
    threeDWidget->priorityButton =
      XtVaCreateManagedWidget( "Dataset Priority",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'I', NULL );
    XmStringFree (accel);

    XtVaCreateManagedWidget( "sep", xmSeparatorGadgetClass, menuPane, NULL );

    accel = XmStringCreateLocalized( "H" );
    threeDWidget->zbufferButton =
      XtVaCreateManagedWidget( "threeDWidgetZBufferHack",
			       xmToggleButtonGadgetClass, menuPane,
			       XmNindicatorOn, True,
			       XmNvisibleWhenOff, True,
			       XmNvalueChangedCallback, callbacks,
			       NULL );
    XmStringFree (accel);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'D'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Datasets", args, n);
    XtManageChild(cascade);

    /* CREATE THE "Options" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    accel = XmStringCreateLocalized( "N" );
    threeDWidget->navButton =
      XtVaCreateManagedWidget( "Navigation Settings",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'N', NULL );
    XmStringFree (accel);

    accel = XmStringCreateLocalized( "R" );
    threeDWidget->rendererButton =
      XtVaCreateManagedWidget( "Render Settings",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'R', NULL );
    XmStringFree (accel);

    accel = XmStringCreateLocalized( "D" );
    threeDWidget->debugButton =
      XtVaCreateManagedWidget( "Debug Settings",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'D', NULL);
    XmStringFree (accel);

    XtVaCreateManagedWidget( "sep", xmSeparatorGadgetClass, menuPane, NULL );

    accel = XmStringCreateLocalized( "V" );
    threeDWidget->featureButton =
      XtVaCreateManagedWidget( "Feature Selection",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'F', NULL );
    XmStringFree (accel);

    XtVaCreateManagedWidget( "sep", xmSeparatorGadgetClass, menuPane, NULL );

    accel = XmStringCreateLocalized( "Esc" );
    threeDWidget->dismissOption =
      XtVaCreateManagedWidget( "Dismiss",
			       xmPushButtonGadgetClass, menuPane,
			       XmNactivateCallback, callbacks,
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'D', NULL );
    XmStringFree (accel);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'O'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Options", args, n);
    XtManageChild(cascade);

    /* CREATE THE "Viewpoint" MENU */
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    accel = XmStringCreateLocalized( "C" );
    threeDWidget->viewpointButton =
      XtVaCreateManagedWidget("Viewpoint Controls",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'C',
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree (accel);

    accel = XmStringCreateLocalized( "P" );
    threeDWidget->viewpointStatusButton =
      XtVaCreateManagedWidget("Viewpoint Status",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'P',
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree (accel);

    accel = XmStringCreateLocalized( "B" );
    threeDWidget->viewpointBookmarksButton =
      XtVaCreateManagedWidget("Viewpoint Bookmarks",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'B',
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree (accel);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'V'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Viewpoint", args, n);
    XtManageChild(cascade);

    /* CREATE THE "Paths" MENU */
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    threeDWidget->pathNewButton =
      XtVaCreateManagedWidget("New Path",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'N',
			      NULL);

    threeDWidget->pathLoadButton =
      XtVaCreateManagedWidget("Load Path...",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'L',
			      NULL);

    threeDWidget->pathSaveButton =
      XtVaCreateManagedWidget("Save Path As...",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'A',
			      NULL);

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menuPane, NULL );

    threeDWidget->pathEditButton =
      XtVaCreateManagedWidget("Edit Path",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'E',
			      NULL);

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menuPane, NULL );

    accel = XmStringCreateLocalized( "Ctrl+P" );
    threeDWidget->pathPlayButton =
      XtVaCreateManagedWidget("Play",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'P',
			      XmNaccelerator, "Ctrl<Key>P",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "Ctrl+S" );
    threeDWidget->pathStopButton =
      XtVaCreateManagedWidget("Stop",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'S',
			      XmNaccelerator, "Ctrl<Key>S",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "Ctrl+U" );
    threeDWidget->pathPauseButton =
      XtVaCreateManagedWidget("Continue",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'u',
			      XmNaccelerator, "Ctrl<Key>U",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menuPane, NULL );

    accel = XmStringCreateLocalized( "Ctrl+T" );
    threeDWidget->pathStartButton =
      XtVaCreateManagedWidget("Goto Start",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 't',
			      XmNaccelerator, "Ctrl<Key>T",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "Ctrl+N" );
    threeDWidget->pathNextButton =
      XtVaCreateManagedWidget("Goto Next",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'x',
			      XmNaccelerator, "Ctrl<Key>N",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "Ctrl+V" );
    threeDWidget->pathPrevButton =
      XtVaCreateManagedWidget("Goto Prev",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'v',
			      XmNaccelerator, "Ctrl<Key>V",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "Ctrl+E" );
    threeDWidget->pathEndButton =
      XtVaCreateManagedWidget("Goto End",
			      xmPushButtonGadgetClass,
			      menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'd',
			      XmNaccelerator, "Ctrl<Key>E",
			      XmNacceleratorText, accel,
			      NULL);
    XmStringFree( accel );

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menuPane, NULL );

    threeDWidget->pathMovie1Button =
      XtVaCreateManagedWidget("Save to Images (320x240)",
			      xmPushButtonGadgetClass, menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, '1',
			      NULL);

    threeDWidget->pathMovie2Button =
      XtVaCreateManagedWidget("Save to Images (480x360)",
			      xmPushButtonGadgetClass, menuPane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, '2',
			      NULL);


    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'P'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Paths", args, n);
    XtManageChild(cascade);

    /* CREATE THE "Window" MENU */
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    accel = XmStringCreateLocalized( "M" );
    threeDWidget->showMenuWidget =
      XtVaCreateManagedWidget( "Show Menu Bar",
			       xmToggleButtonGadgetClass, menuPane,
			       XmNindicatorOn, True,
			       XmNvisibleWhenOff, True,
			       XmNset, True,
			       XmNaccelerator, "Ctrl<Key>M",
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'M', NULL );
    XtAddCallback( threeDWidget->showMenuWidget, XmNvalueChangedCallback,
		   ThreeDWidgetMenuCB, (XtPointer) threeDWidget );
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "W" );
    threeDWidget->showIconsWidget =
      XtVaCreateManagedWidget( "Show Web Icons",
			       xmToggleButtonGadgetClass, menuPane,
			       XmNindicatorOn, True,
			       XmNvisibleWhenOff, True,
			       XmNset, True,
			       XmNaccelerator, "Ctrl<Key>W",
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'W', NULL );
    XtAddCallback( threeDWidget->showIconsWidget, XmNvalueChangedCallback,
		   ThreeDWidgetMenuCB, (XtPointer) threeDWidget );
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "A" );
    threeDWidget->showStatusBarWidget =
      XtVaCreateManagedWidget( "Show Status Bar",
			       xmToggleButtonGadgetClass, menuPane,
			       XmNindicatorOn, True,
			       XmNvisibleWhenOff, True,
			       XmNset, True,
			       XmNaccelerator, "Ctrl<Key>A",
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'A', NULL );
    XtAddCallback( threeDWidget->showStatusBarWidget, XmNvalueChangedCallback,
		   ThreeDWidgetMenuCB, (XtPointer) threeDWidget );
    XmStringFree( accel );

    accel = XmStringCreateLocalized( "F" );
    threeDWidget->showFullscreenWidget =
      XtVaCreateManagedWidget( "Show Fullscreen",
			       xmToggleButtonGadgetClass, menuPane,
			       XmNindicatorOn, True,
			       XmNvisibleWhenOff, True,
			       XmNset, False,
			       XmNaccelerator, "Ctrl<Key>F",
			       XmNacceleratorText, accel,
			       XmNmnemonic, 'F', NULL );
    XtAddCallback( threeDWidget->showFullscreenWidget, XmNvalueChangedCallback,
		   ThreeDWidgetMenuCB, (XtPointer) threeDWidget );
    XmStringFree( accel );

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'W'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Window", args, n);
    XtManageChild(cascade);

    /* CREATE THE "Help" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

    widget = XtVaCreateManagedWidget( "Help on 3-D Viewer",
				      xmPushButtonGadgetClass, menuPane,
				      XmNmnemonic, 'H', NULL );
    XtAddCallback( widget, XmNactivateCallback, TerraVisionHelpCB,
		   (XtPointer) "motif/3dviewer.shtml" );

    widget = XtVaCreateManagedWidget( "Help on TerraVision",
				      xmPushButtonGadgetClass, menuPane,
				      XmNmnemonic, 'T', NULL );
    XtAddCallback( widget, XmNactivateCallback, TerraVisionHelpCB, NULL );

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
    XtSetArg (args[n], XmNmnemonic, 'H'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Help", args, n);
    XtManageChild(cascade);

    /* Tell Motif that this last menu was the Help menu - this */
    /* will make Motif display it on the right side of the bar */

    XtVaSetValues( menuBar, XmNmenuHelpWidget, cascade , NULL );

    return menuBar;
}

/*
 * Synopsis: static void
 *           ThreeDWidgetMenuCB(Widget,
 *                                    XtPointer,
 *                                    XtPointer)
 *
 * Description: Callback functions for the menu buttons
 *
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:  July, 1994
 *
 */
static void
ThreeDWidgetMenuCB(Widget    w,
		   XtPointer client_data,
		   XtPointer call_data)
{
/*  static char * funcName = "ThreeDWidgetMenuCB";*/
  ThreeDWidget * threeDWidget = (ThreeDWidget *) client_data;
  int was_playing = PathIsPlaying(threeDWidget->threeD);

  if(w == threeDWidget->priorityButton) {
    DatasetPrioShow( threeDWidget );
  }
  if(w == threeDWidget->datasetDisplayButton) {
    DatasetDisplayShow( threeDWidget );
  }
  if(w == threeDWidget->zbufferButton) {
    tvSetBoolean (threeDWidget->threeD, TV_USE_ZBUFFER_HACK,
		  XmToggleButtonGadgetGetState( w ));
  }
  if(w == threeDWidget->viewpointButton) {
    ThreeDControlManageWidget(threeDWidget->threeDControl);
  }
  if(w == threeDWidget->navButton) {
    ThreeDPanelManageWidget(threeDWidget->threeDPanel, NAV_PANEL);
  }
  if(w == threeDWidget->rendererButton) {
    ThreeDPanelManageWidget(threeDWidget->threeDPanel, RENDER_PANEL);
  }
  if(w == threeDWidget->featureButton) {
    FeatureSelectShow( threeDWidget );
  }
  if(w == threeDWidget->dismissOption) {
    ThreeDWidgetDismiss( threeDWidget );
  }
  if(w == threeDWidget->debugButton) {
    ThreeDPanelManageWidget(threeDWidget->threeDPanel, DEBUG_PANEL);
  }
  if(w == threeDWidget->viewpointStatusButton) {
    ThreeDPanelManageWidget(threeDWidget->threeDPanel, POSITION_PANEL);
  }
  if(w == threeDWidget->viewpointBookmarksButton) {
    ViewpointShow( threeDWidget );
  }
  if(w == threeDWidget->pathNewButton) {
    PathNewFE ( threeDWidget );
  }
  if(w == threeDWidget->pathLoadButton) {
    PathLoad( threeDWidget );
  }
  if (w == threeDWidget->pathSaveButton) {
    PathSaveAs( threeDWidget );
  }
  if(w == threeDWidget->pathEditButton) {
    PathEdit( threeDWidget );
  }
  if(w == threeDWidget->pathPlayButton) {
    PathPlay(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathStopButton) {
    PathStop(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathPauseButton) {
    PathPause(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathStartButton) {
    PathGotoStart(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathNextButton) {
    PathGotoNext(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathPrevButton) {
    PathGotoPrev(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathEndButton) {
    PathGotoEnd(threeDWidget->threeD);
  }
  if(w == threeDWidget->pathMovie1Button) {
    PathRecord(threeDWidget->threeD, 320, 240);
  }
  if(w == threeDWidget->pathMovie2Button) {
    PathRecord(threeDWidget->threeD, 480, 360);
  }
  if(w == threeDWidget->showMenuWidget) {
    threeDWidget->showMenu = XmToggleButtonGadgetGetState(w);
    ThreeDWidgetUpdateWindow( threeDWidget );
  }
  if(w == threeDWidget->showIconsWidget) {
    threeDWidget->showIcons = XmToggleButtonGadgetGetState(w);
    threeDWidget->threeD->showLogo = !threeDWidget->showIcons;
    ThreeDWidgetUpdateWindow( threeDWidget );
  }
  if(w == threeDWidget->showStatusBarWidget) {
    threeDWidget->showStatusBar = XmToggleButtonGadgetGetState(w);
    ThreeDWidgetUpdateWindow( threeDWidget );
  }
  if(w == threeDWidget->showFullscreenWidget) {
    threeDWidget->showFullscreen = XmToggleButtonGadgetGetState(w);
    ThreeDWidgetFullScreenMode( threeDWidget, TRUE );
  }

  /* Change Path menu name to Pause or Continue accordingly, if necessary */
  {
    int playing = PathIsPlaying(threeDWidget->threeD);
    if ( was_playing != playing ) {
      XmString item = tvXmString( playing ? "Pause" : "Continue" );
      XtVaSetValues (threeDWidget->pathPauseButton, XmNlabelString, item, NULL);
      XmStringFree( item );
    }
  }
}


/*
 * Synopsis: void ThreeDWidgetStartRendering()
 *	
 *
 * Description: Initialization function 
 *    for when we start up rendering the view
 *
 * Externals:
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
void
ThreeDWidgetStartRendering (ThreeDWidget *threeDWidget)
{ 
  static char *funcName = "ThreeDWidgetStartRendering";
  static int  pixmapsMade = FALSE;
  tvViewpoint *origin = &threeDWidget->originView;
  int         i;

  /* Check for the Origin bookmark name and use it if set. */

  memset (origin, 0, sizeof (*origin));
  if (GetViewpointOrigin (threeDWidget, origin->eye, 
			  &origin->yaw, &origin->pitch, &origin->roll)) {
    TSMTRACE ((THREED_MASK|1, funcName, "Found origin: %.0f,%.0f,%.0f\n",
	       origin->eye[0], origin->eye[1], origin->eye[2]));
    tvSetInitialView (threeDWidget->threeD, origin);
  }

  if (!threeDWidget->threeD->cave) {
    /* Always open with a menu bar */
    threeDWidget->showMenu = True;
    ThreeDWidgetUpdateWindow( threeDWidget );

    XtManageChild (threeDWidget->mainForm);

    if (! pixmapsMade) {
      MakeWebIconPixmaps(threeDWidget->mainForm);
      pixmapsMade = TRUE;
    }

    if (!threeDWidget->pixmapsDone) {
      for (i = 0; i < NUMB_WEB_ICONS; i++) {
	XtVaSetValues(threeDWidget->mosaicWidgets[i], 
		      XmNlabelPixmap, web_icon[i],
		      XmNlabelType, XmPIXMAP,
		      NULL);
	XtManageChild(threeDWidget->mosaicWidgets[i]);
      }
    }
#ifdef XCPS
    XtManageChild(threeDWidget->grabber->mainForm);
#endif

#ifdef SCROLLBAR_H
    sbbeep("c");
#endif
  }

  /*tvBaseStart (threeDWidget->threeD);*/

  /* load any path specified by the -path option, and start playing it */

  PathLoadInitial( threeDWidget, TRUE );
}

/*
 * Synopsis: void ThreeDWidgetInterpret( ThreeDWidget *, const char * )
 *
 * Description: Executes an OAA command in the context of the specified
 *              ThreeDWidget structure.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 June 2000
 *
 */

void
ThreeDWidgetInterpret( ThreeDWidget *threeDWidget, char const *command ) 
{
  if ( threeDWidget && threeDWidget->threeD )
    tvInterpret( threeDWidget->threeD, command );
}

/*
 * Synopsis: static void ThreeDWidgetRememberWinSize( ThreeDWidget * )
 *
 * Description: Records the current size and location of the 3-D viewer
 *    window in the ThreeDWidget structure.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 28 March 1999
 *
 */

static void
ThreeDWidgetRememberWinSize( ThreeDWidget *threeDWidget )
{
  Dimension width, height;
  Position  x, y;

  XtVaGetValues( threeDWidget->mainForm, XmNwidth, &width,
		 XmNheight, &height, XmNx, &x, XmNy, &y, NULL );

  threeDWidget->unzoomedWidth = width;
  threeDWidget->unzoomedHeight = height;
  threeDWidget->unzoomedX = x;
  threeDWidget->unzoomedY = y;
}

/*
 * Synopsis: void ThreeDWidgetFullScreenMode( ThreeDWidget *, int )
 *
 * Description: To switch to full screen mode, or restore
 *    the window size/position to its previous state.
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: 1995
 *
 */

static void
ThreeDWidgetFullScreenMode( ThreeDWidget * threeDWidget, int save_state )
{
#ifdef __NUTC__
  static int border_xoff = 4, border_yoff = 4;
#else
  static int border_xoff = 0, border_yoff = 0;
#endif

  if ( threeDWidget->showFullscreen ) {

    /* Record the current window size and position*/

    if ( save_state )
      ThreeDWidgetRememberWinSize( threeDWidget );

    /* move window to top-left corner of screen and set size to max.   */
    /* we would like this to be a borderless window, however it seems  */
    /* that you can't do this for anything other than the root app     */
    /* window! (i.e. setting XmNmwmDecorations to 0). So instead we    */
    /* artificially grow the window so that the borders are off screen */
    /* doesn't work under IRIX as 4DWM imposes a max size limit :-(    */

    XtVaSetValues( threeDWidget->mainForm,
		   XmNx, 0,
		   XmNy, 0,
		   XmNwidth, threeDWidget->screenMax[0] + border_xoff,
		   XmNheight, threeDWidget->screenMax[1] + border_yoff,
		   NULL );

  } else if ( threeDWidget->unzoomedWidth > 0 ) {

    /* Reset the window size and position */

      XtVaSetValues( threeDWidget->mainForm,
		   XmNx, threeDWidget->unzoomedX,
		   XmNy, threeDWidget->unzoomedY,
		   XmNwidth, threeDWidget->unzoomedWidth,
		   XmNheight, threeDWidget->unzoomedHeight,
		   NULL );

  }

  XtVaSetValues( threeDWidget->showFullscreenWidget,
		 XmNset, threeDWidget->showFullscreen, NULL );

  tvBasePostRedraw (threeDWidget->threeD, FALSE);
}
