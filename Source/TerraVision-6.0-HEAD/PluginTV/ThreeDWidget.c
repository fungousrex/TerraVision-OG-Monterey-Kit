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

#ifdef HAVE_CONFIG_H
# ifdef MSVC
#  include "winbuild/config.h"
# else
#  include "config.h"
# endif
#endif

#include <tvutils/tvutils.h>

#include "TerraVisionInc.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "Viewpoint.h"
#include "PathsFE.h"

/* Height (meters) at which acceleration is defined for height-scaled nav */ 

#define ACCEL_BASE_HEIGHT  100

static void ThreeDWidgetMessageHandler (void *, tvMessageType, 
					char const *, va_list);
static void ThreeDWidgetUpdateHandler (void *, tvLinkType, tvBase *);


/*
 *
 * Synopsis: CreateThreeDWidget(void)
 *	
 * Description: Create a ThreeDWidget structure
 *
 * Returns: Pointer to a ThreeDWidget structure, or NULL on failure
 *
 * Author: Martin Reddy
 * 
 * Date: 13 December, 1999
 *
 */

ThreeDWidget *
CreateThreeDWidget( void )
{
  const char   *funcName = "CreateThreeDWidget";
  ThreeDWidget *threeDWidget;

  /* CREATE THE THREEDWIDGET STRUCTURE */

  if (! (threeDWidget = (ThreeDWidget *) calloc(sizeof(ThreeDWidget),1)) ) {
    tsmStatus("%s: Error allocating space for ThreeDWidget!", funcName);
    return NULL;
  }

  return threeDWidget;
}

/*
 * Synopsis: InitThreeDWidget(ThreeDWidget, Widget, in)
 *	
 * Description: Create a 3-D widget OpenGL context 
 *
 * Returns: TRUE on success, FALSE on failure
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999
 *
 */

int
InitThreeDWidget( ThreeDWidget *threeDWidget, int mode3d )
{
  /*static const char funcName[] = "InitThreeDWidget";*/
  tvBase *threeDCore;

  if ( threeDWidget == NULL) return FALSE;

  if ( threeDWidget->threeDCore ) return TRUE;

  PlatformInitThreeDWidget( threeDWidget );

  threeDCore = threeDWidget->threeDCore;

  tvSetMessageHandler( threeDCore, ThreeDWidgetMessageHandler, threeDWidget );
  tvSetUpdateHandler( threeDCore, ThreeDWidgetUpdateHandler, threeDWidget );

  threeDWidget->title = (char*)malloc( sizeof( char ) * 128 );
	strcpy( threeDWidget->title, "TerraVision Plugin" );

  threeDWidget->flyMode = FALSE;
  threeDWidget->airplaneControl = TRUE;
  threeDWidget->seek = FALSE;
  threeDWidget->drawOnScreen = FALSE;
  threeDWidget->shiftDown = FALSE;
  threeDWidget->ctrlDown = FALSE;
  threeDWidget->altDown = FALSE;
  threeDWidget->mouseDown = FALSE;

  tvSetBoolean( threeDCore, TV_HEIGHT_BASED_NAVIGATION, TRUE );
  tvSetBoolean( threeDCore, TV_USE_ZBUFFER_HACK, TRUE );
  tvSetBoolean( threeDCore, TV_SHOW_LIGHTING, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_HEADS_UP, TRUE );
  tvSetBoolean( threeDCore, TV_MIPMAP_TEXTURES, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_WIREFRAME, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_TEXTURES, TRUE );
  tvSetBoolean( threeDCore, TV_SHOW_MODELS, TRUE );
  tvSetBoolean( threeDCore, TV_SHOW_TILES, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_FRUSTUMS, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_PATHS, FALSE );
  tvSetBoolean( threeDCore, TV_FEATURES_LOAD_TEXTURES, FALSE );
  tvSetBoolean( threeDCore, TV_SHOW_SKY, TRUE );
  tvSetBoolean( threeDCore, TV_SHOW_GROUND, TRUE );
  tvSetBoolean( threeDCore, TV_SHOW_DATASETS, FALSE );
  tvSetBoolean( threeDCore, TV_HORIZON_CULL, TRUE );
  tvSetBoolean( threeDCore, TV_STEREO, FALSE );
  tvSetBoolean( threeDCore, TV_SCALE_IPD, TRUE );
  tvSetFloat( threeDCore, TV_MOTION_THRESHOLD,
	      tvuGetInt( TVU_DROP_TO_WIREFRAME ) );

  /* set the viewer to be a 3D or constrained 2D viewer by default */

  tvSetBoolean( threeDCore, TV_TWOD_CONSTRAIN, ! mode3d );

  /* create a link to ourselves for things like tile/path display */

  tvAddLink( threeDCore, TV_FRUSTUM, threeDCore );
  tvAddLink( threeDCore, TV_TILES, threeDCore );
  tvAddLink( threeDCore, TV_PATHS, threeDCore );

#ifdef WIN32
  /* we don't have an Init callback for GL under win, so do it here */
  /* (maybe we can use the WM_CREATE windows message to do this?)   */

  ThreeDWidgetInitCallback( pluginWidget );
#endif

  return TRUE;
}

void
DestroyThreeDWidget( ThreeDWidget *threeD )
{
  static char const funcName[] = "DestroyThreeDWidget";
  tvBase *threeDCore = threeD->threeDCore;

  TSMTRACE ((THREED_MASK|1, funcName, "(%p)", threeD));
  TSMTRINC ((THREED_MASK|1, 1));

  /* remove the links to ourselves for things like tile/path display */
  tvRemoveLink( threeDCore, TV_FRUSTUM, threeDCore );
  tvRemoveLink( threeDCore, TV_TILES, threeDCore );
  tvRemoveLink( threeDCore, TV_PATHS, threeDCore );

  if (tvGetBoolean (threeDCore, TV_STEREO))
    tvSetBoolean (threeDCore, TV_STEREO, FALSE);

  tvBaseDestroy (threeDCore);

  PlatformCloseDown (threeD);

  free (threeD->title);
  free (threeD);

  TSMTRINC ((THREED_MASK|1, -1));
  TSMTRACE ((THREED_MASK|1, funcName, "(%p) Done", threeD));
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

void
DrawOnScreen_BtnDn( ThreeDWidget *threeDWidget, int x, int y )
{
  buttonDown = TRUE;
  tvBeginStroke (threeDWidget->threeDCore, x, y);
}

void
DrawOnScreen_BtnUp( ThreeDWidget *threeDWidget, int x, int y )
{
  buttonDown = FALSE;
  tvEndStroke (threeDWidget->threeDCore);
}

void
DrawOnScreen_Motion( ThreeDWidget *threeDWidget, int x, int y )
{
  if ( buttonDown )
    tvContinueStroke (threeDWidget->threeDCore, x, y);
}

/*
 * Synopsis: ThreeDWidgetBtnUp( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   when the user releases Button 1 of the mouse.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetBtnUp( ThreeDWidget *threeD, int x, int y )
{
  /*static char const funcName[] = "ThreeDWidgetBtnUp";*/
  const char   *link_url;

  if ( threeD == NULL ) return;

  /* no navigation during animation */

  if ( threeD->seek || threeD->altDown ) return;

  if ( PathIsPlaying(threeD->threeDCore) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_BtnUp( threeD, x, y );
    return;
  }

  /* did we clicked on a hyperlink? If so, unhighlight link for visual */
  /* feedback; also, make sure that we don't do the tvStartMovement()! */

  link_url = tvGetString( threeD->threeDCore, TV_HYPERLINK_URL );
  if ( link_url ) {
    if ( strlen(link_url) > 0 ) free( (char *) link_url );
    tvSetString( threeD->threeDCore, TV_HYPERLINK_URL, NULL );
    /* ThreeDWidgetStatusBarClearView( threeDWidget ); */
    tvBasePostRedraw( threeD->threeDCore, FALSE );
    return;
  }

  /* Otherwise, we're navigating... */

  tvStartMovement (threeD->threeDCore);
}

/*
 * Synopsis: ThreeDWidgetBtn2Up( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   when the user releases Button 2 of the mouse.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetBtn2Up( ThreeDWidget *threeD, int x, int y )
{
  /*static char const funcName[] = "ThreeDWidgetBtn2Up";*/

  if ( threeD == NULL ) return;

  threeD->mouseDown = FALSE;
  tvSetBoolean( threeD->threeDCore, TV_SHOW_CROSSHAIR, FALSE );

  /* no navigation during animation */

  if ( PathIsPlaying(threeD->threeDCore) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_BtnUp( threeD, x, y );
    return;
  }

  tvStopMovement (threeD->threeDCore);
}

/*
 * Synopsis: ThreeDWidgetBtn1Down( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   when the user pressed Button 1 of the mouse.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetBtn1Down( ThreeDWidget *threeD, int x, int y )
{
  static char *funcName = "ThreeDWidgetBtn1Down";
  int         crosshairpos[2];
  const char  *link_url;

  if ( threeD == NULL ) return;

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */
  
  if ( PathIsPlaying(threeD->threeDCore) ) return;
 
  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeD, x, y );
    return;
  }

  /* are we seeking to a point on the terrain? */

  if (threeD->seek || threeD->altDown) {
    tvFlyToPickPoint(threeD->threeDCore, x, y);
    return;
  }

  /* have we clicked on a hyperlink? */

  link_url = tvGetString( threeD->threeDCore, TV_HYPERLINK_URL );
  if ( link_url ) {
    tsmShowUrl( link_url );
    free( (char *) link_url );
    return;
  }

  /* Otherwise, we're navigating */

  /* GET THE MOUSE POSITION */

  threeD->xMouse = threeD->xMouseDown = x;
  threeD->yMouse = threeD->yMouseDown = y;
  threeD->mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartAccelerate (threeD->threeDCore);

  tvSetBoolean( threeD->threeDCore, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( threeD->threeDCore, TV_CROSSHAIR, crosshairpos );
}

/*
 * Synopsis: ThreeDWidgetBtn2Down( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   when the user pressed Button 2 of the mouse.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetBtn2Down( ThreeDWidget *threeD, int x, int y )
{
  static char *funcName = "ThreeDWidgetBtn2Down";
  int         crosshairpos[2];

  if ( threeD == NULL ) return;

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */

  if ( PathIsPlaying(threeD->threeDCore) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeD, x, y );
    return;
  }

  /* Otherwise, we're navigating */
  
  /* Set speed back to zero when we start */

  tvSetFloat( threeD->threeDCore, TV_VELOCITY, 0.0 );
  tvSetFloat( threeD->threeDCore, TV_ACCELERATION, 0.0 );

  /* find where mouse down occured, and origin of window */

  threeD->xMouse = threeD->xMouseDown = x;
  threeD->yMouse = threeD->yMouseDown = y;
  threeD->mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartMovement (threeD->threeDCore);

  tvSetBoolean( threeD->threeDCore, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( threeD->threeDCore, TV_CROSSHAIR, crosshairpos );
}

/*
 * Synopsis: ThreeDWidgetBtn3Down( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   when the user pressed Button 3 of the mouse.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetBtn3Down( ThreeDWidget *threeD, int x, int y )
{
  static char *funcName = "ThreeDWidgetBtn3Down";
  int         crosshairpos[2];

  if ( threeD == NULL ) return;

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */

  if ( PathIsPlaying(threeD->threeDCore) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_BtnDn( threeD, x, y );
    return;
  }

  /* Otherwise, we're navigating */

  /* find where mouse down occured, and origin of window */

  threeD->xMouse = threeD->xMouseDown = x;
  threeD->yMouse = threeD->yMouseDown = y;
  threeD->mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartDeccelerate (threeD->threeDCore);

  tvSetBoolean( threeD->threeDCore, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( threeD->threeDCore, TV_CROSSHAIR, crosshairpos );
}

/*
 * Synopsis: ThreeDWidgetMotion( ThreeDWidget *, int x, int y )
 *
 * Description: Generic function that implements navigation features
 *   while the user has a mouse button pressed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 December, 1999.
 *
 */

void
ThreeDWidgetMotion( ThreeDWidget *threeD, int x, int y )
{
  static char *funcName = "ThreeDWidgetMotion";
  tvBase      *threeDCore;
  int         screensize[2], twod_mode;
  const char  *link_desc;

  if ( threeD == NULL ) return;

  threeDCore = threeD->threeDCore;

  /* are we drawing on the screen rather than navigating? */

  if ( threeD->drawOnScreen ) {
    DrawOnScreen_Motion( threeD, x, y );
    return;
  }

  /* get the window's current width and height, and other settings */

  tvGetIntegerv( threeDCore, TV_SIZE, screensize );

  twod_mode = tvGetBoolean( threeDCore, TV_TWOD_CONSTRAIN );
 
  /* otherwise, we're navigating */

  if (threeD->mouseDown || threeD->shiftDown || 
      threeD->ctrlDown  || threeD->altDown) {
    int scale = (( screensize[0] > screensize[1] )
		 ? screensize[0] : screensize[1] );
    float d[2];

    d[0] =  ((x - threeD->xMouseDown) / (scale / 2.0));
    d[1] = -((y - threeD->yMouseDown) / (scale / 2.0));

    threeD->xMouse = x;
    threeD->yMouse = y;

    if (threeD->shiftDown || threeD->ctrlDown || twod_mode)
      tvSetFloatv (threeDCore, TV_SLIDE_DELTA, d);
    else {
      if (! threeD->airplaneControl)
	d[1] *= -1; 
      tvSetFloatv (threeDCore, TV_ATTITUDE_DELTA, d);
    }

    /* clear the Viewpoint field of the status bar (if set), because */
    /* if we have moved then we are no longer at that viewpoint.     */
    
    TSMTRACE ((THREED_MASK|2, funcName, "%d,%d", x, y));

  } else {

    /* send an event to find the hyperlink under the   */
    /* new cursor position, but only if we are moving. */

    if ( ! tvGetBoolean( threeDCore, TV_MOVING ) )
      tvBaseCheckForHyperlink( threeDCore, x, y );

    /* get a description for the hyperlink under cursor, if any */
 
    if ( ( link_desc = tvGetString( threeDCore, TV_HYPERLINK_DESC ) ) == NULL )
      link_desc = tvGetString( threeDCore, TV_HYPERLINK_URL );

    /* update the status bar with that description */

    if ( link_desc == NULL )
      /* ThreeDWidgetStatusBarClearView( threeDWidget ) */;
    else {
      /*ThreeDWidgetUpdateStatusBar( threeDWidget, link_desc )*/;
      free( (char *) link_desc );
    }

  }

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
  tvBase  *threeDCore;
  double      mouseX, mouseY;
  tvViewpoint view_s, *view = &view_s;
  double      heightOld, heightNew;
  int         moved = 0;
  int         screensize[2];
  int         twod_mode, height_nav, freeze_tiles;

  if ( threeDWidget == NULL ) return;

  TSMTRACE ((THREED_MASK|1, funcName, "Updating"));
  TSMTRINC ((THREED_MASK|1, 1));

  threeDCore = threeDWidget->threeDCore;

  /* get the window's current width and height, and other settings */

  tvGetIntegerv( threeDCore, TV_SIZE, screensize );

  twod_mode    = tvGetBoolean( threeDCore, TV_TWOD_CONSTRAIN );
  height_nav   = tvGetBoolean( threeDCore, TV_HEIGHT_BASED_NAVIGATION );
  freeze_tiles = tvGetBoolean( threeDCore, TV_FREEZE_TILES );
 
  tvCurrentView( threeDCore, view );

  /* FIND DISTANCE FROM POINT OF MOUSE DOWN */

  mouseX = threeDWidget->xMouseDown - threeDWidget->xMouse;
  mouseY = threeDWidget->yMouseDown - threeDWidget->yMouse;

  if (! threeDWidget->airplaneControl)
    mouseY *= -1; 

  TSMTRACE ((THREED_MASK|2, funcName, "%4.4f/%d %4.4f/%d%s%s%s", 
	     mouseX, screensize[0], mouseY, screensize[1],
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

  if ( threeDWidget->shiftDown || threeDWidget->ctrlDown || twod_mode ) {
    if ( height_nav )
      if ( twod_mode ) 
	tvAdjustView( threeDCore, view, view->vel * 0.005,
		      mouseX * -0.01 * heightOld / ACCEL_BASE_HEIGHT, 
		      mouseY * -0.01 * heightOld / ACCEL_BASE_HEIGHT );
      else
	tvAdjustView( threeDCore, view, view->vel * 0.1,
		      mouseX * -0.05 * heightOld / ACCEL_BASE_HEIGHT, 
		      mouseY * -0.05 * heightOld / ACCEL_BASE_HEIGHT );
    else
      tvAdjustView( threeDCore, view, view->vel * 0.1,
		    mouseX * -0.1, mouseY * -0.1 );
    moved = 1;

  } else {

    /* TURN THE VIEWER VIEWPOINT BASED UPON THE MOUSE LOCATION */

    if ( threeDWidget->ctrlDown ) {
      view->yaw   += (M_PI / 180.0) * -mouseX / (screensize[0] / 2.0);
      view->pitch += (M_PI / 180.0) *  mouseY / (screensize[1] / 2.0);
    }
    else if ( ! twod_mode ) {
      view->yaw   += (M_PI / 180.0) *  mouseX / (screensize[0] / 2.0);
      view->pitch += (M_PI / 180.0) * -mouseY / (screensize[1] / 2.0);
    }

  }

  TSMTRACE ((THREED_MASK|2, funcName, "roll %.1f pitch %.1f yaw %.1f",
	     view->roll*180.0/M_PI,
	     view->pitch*180.0/M_PI,
	     view->yaw*180.0/M_PI));
  TSMTRACE ((THREED_MASK|2, funcName, "vel %5.2f accel %5.2f",
	     view->vel, view->accel));

  if ( moved && height_nav ) {
    /* compute the new speed based on the elevation */
    heightNew = tvBasePointAltitude( threeDCore, view->eye );
    if ( heightNew < 10.0 ) heightNew = 10.0;
    view->vel *= heightNew / heightOld;
  }

  TSMTRACE ((THREED_MASK|2, funcName, "eye %.0f,%.0f,%.0f",
	     view->eye[0], view->eye[1], view->eye[2]));

  /* UPDATE THE ACTIVE VIEWPOINT. */
  tvSetView( threeDCore, view, FALSE );

  /* ASK FOR A REDRAW TO BE PERFORMED ONLY IF TILES ARE FROZEN,
     OTHERWISE VISIBILITY WILL TAKE CARE OF IT. */

  if ( freeze_tiles )
    tvBasePostRedraw( threeDCore, FALSE );

  TSMTRINC ((THREED_MASK|1, -1));
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
  static char * funcName = "ThreeDWidgetStartRendering";
  tvViewpoint *origin = &threeDWidget->originView;

  /* Check for the Origin bookmark name and use it if set. */

  memset (origin, 0, sizeof (*origin));

  if ( GetViewpointOrigin (threeDWidget, origin->eye, 
			   &origin->yaw, &origin->pitch, &origin->roll) ) {
    TSMTRACE ((THREED_MASK|1, funcName, "Found origin: %.0f,%.0f,%.0f\n",
	       origin->eye[0], origin->eye[1], origin->eye[2]));
    tvSetInitialView (threeDWidget->threeDCore, origin);
  }

  tvBaseStart (threeDWidget->threeDCore);
}

void
ThreeDWidgetAddDataset (ThreeDWidget *threeDWidget,
			TsGeoPyramid *geo)
{
  tvAddDataset (threeDWidget->threeDCore, geo);

  if (list_count (geoList) == 1)
    ThreeDWidgetStartRendering (threeDWidget);
}

static void
ThreeDWidgetMessageHandler (void *closure, tvMessageType type, 
			    char const *fmt, va_list ap)
{
  /*ThreeDWidget *threeD = closure;*/
  /*char message[1024];*/
  switch (type) {
  case TV_ERROR_MSG: {
    break; }
  case TV_STATUS_MSG: {
    break; }
  case TV_BUSY_MSG: {
    break; }
  }
}

static void
ThreeDWidgetUpdateHandler (void *closure, tvLinkType link, tvBase *core)
{
  /*ThreeDWidget *threeD = closure;*/

  switch (link) {
  case TV_PATHS:
    break;
  case TV_DATASETS:
    break;
  default:
    break;
  }
}
