/*****************************************************************************
 * Name:
 *  tvViewer.cpp
 *
 * Description:
 *	MS-Windows specific routines
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:27:10 $
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
 *  Contributor(s): Michael Eriksen
 *  ----- END LICENSE BLOCK -----
 *
 *****************************************************************************/

#include "stdafx.h"

#include "tvViewer.h"
#include "Viewpoint.h"

#include "ThreeDCore.h"
#include "ThreeDCoreI.h"

#include <glt/glm.h>

#include <prinit.h>

#include <tvutils/tvutils.h>


tvViewer::tvViewer() {
	currBookmark			= 0;
	tvbase						= NULL;

	xMouseDown				= 0;
	yMouseDown				=	0;
	xMouse						=	0;
	yMouse						= 0;

  flyMode						=	false;
  airplaneControl		= true;
  seek							= false;
  drawOnScreen			= false;
	animateBookmarks	= false; 

  mouseDown					= false;

	modifierMode			= 0;

	main_thread				= NULL;
}

tvViewer::~tvViewer() {
  static char const funcName[] = "~tvViewer";

	if (tvbase != NULL) {
		/* remove the links to ourselves for things like tile/path display */
		tvRemoveLink( tvbase, TV_FRUSTUM, tvbase );
		tvRemoveLink( tvbase, TV_TILES, tvbase );
		tvRemoveLink( tvbase, TV_PATHS, tvbase );

		if (tvGetBoolean (tvbase, TV_STEREO))
			tvSetBoolean (tvbase, TV_STEREO, false);

		tvBaseStop(tvbase);
		//tvBaseDestroy(tvbase);
	}

  if (geoList) {
    // This causes a crash under Windows!
    // while (! list_empty (geoList)) {
    // TsGeoPyramid *geo = (TsGeoPyramid*)list_pop (geoList);
    // TsGeoDestroy (geo);
    // }
  }

  // cleanup the tvutils library
  tvuFinish();

  PR_ProcessExit( 0 );
}

void tvViewer::init( HWND hWnd ) {
  const char	 *funcName = "init";
  char         *install_dir;
  glmWindow    glmw;
  glmContext   glmCtx;
	
  // setup graphics
  static glmContextProperties props = {0};

  props.singleBuffer = FALSE;
  props.stereo = FALSE;

#if 0
 glmCtx = glmWFindContext (hWnd, TRUE, &props);
#else
  glmCtx = glmWCreateContext (hWnd, 0);
#endif

  // initialise the tvutils library
  tvuInit( TVU_PLUGIN );

  // load the config file and viewpoints fike
  tvuLoadConfig();
  tvuLoadViewpoints();

  // Initialize globals. 
  geoList = list_create ( TRUE );

  // Set the texture directory 
  if ( ( install_dir = tvuGetString( TVU_INSTALL_DIR ) ) ) {
    char texture_path[512];
    sprintf( texture_path, "%s/etc/textures", install_dir );
    TextureSetPath( texture_path );
  }

  // do any initialisation required for supporting VRML feature models 
  FeatureInit();

  // initialise the TsLog stuff in order to do net throughput logging 
  CreateTsLog();

  // Initialize thread handling 
  main_thread = PR_GetCurrentThread();
  pthr_set_name ( (void *)main_thread, "TerraVision Plugin" );

  // Initialize glmWindow 
  glmw = glmWCreateWindow( hWnd, glmCtx, true, false );

  //create tvBase
  tvbase = tvBaseCreate( this, NULL, glmw );

  // create a link to ourselves for things like tile/path display 
  tvAddLink( tvbase, TV_FRUSTUM, tvbase );
  tvAddLink( tvbase, TV_TILES, tvbase );
  tvAddLink( tvbase, TV_PATHS, tvbase );

  tvSetBoolean( tvbase, TV_TWOD_CONSTRAIN, TRUE );
}

void tvViewer::setViewpointURL(const char * url) {
  // load the user's viewpoint bookmarks file 
  tvuSetString( TVU_VIEWPOINT_URL, url );
  tvuLoadViewpoints();
}

void tvViewer::processMouse(tvEvent *currentEvent) {
  static char *funcName = "processMouse";
  int screensize[2], twod_mode;
  static int lmb = FALSE, mmb = FALSE, rmb = FALSE;
  static int x = 0, y = 0;

  if (tvbase == NULL) return;

	x = currentEvent->x;
	y = currentEvent->y;

	switch (currentEvent->id) {
		case tvEvent::MOUSE_DOWN:
			switch (currentEvent->button) {
				case  tvEvent::LEFT_MOUSE_BUTTON: 
			    lmb = TRUE;
					ThreeDWidgetBtn1Down( x, y );
					EmulateMMB( lmb, &mmb, rmb, x, y );
				break;					

				case tvEvent::MIDDLE_MOUSE_BUTTON:
			    mmb = TRUE;
			    ThreeDWidgetBtn2Down( x, y );
				break;
	
				case tvEvent::RIGHT_MOUSE_BUTTON:
			    rmb = TRUE;
					ThreeDWidgetBtn3Down( x, y );
					EmulateMMB( lmb, &mmb, rmb, x, y );
				break;
			}

		break;

		case tvEvent::MOUSE_UP: 
			switch (currentEvent->button) {
				case tvEvent::LEFT_MOUSE_BUTTON: 
			    lmb = FALSE;
					ThreeDWidgetBtnUp( x, y );
					EmulateMMB( lmb, &mmb, rmb, x, y );
				break;

				case tvEvent::MIDDLE_MOUSE_BUTTON:
			    mmb = FALSE;
			    ThreeDWidgetBtn2Up( x, y );
				break;

				case tvEvent::RIGHT_MOUSE_BUTTON:
			    rmb = FALSE;
					ThreeDWidgetBtnUp( x, y );
					EmulateMMB( lmb, &mmb, rmb, x, y );
				break;
			}
		break;

		case tvEvent::MOUSE_MOVE:
	    ThreeDWidgetMotion( x, y );
		break;
	}
}

void tvViewer::processKey(tvEvent *currentEvent) {
  const char *funcName = "processKey";
  double pitch_delta = 2.0 * M_PI / 180.0;
  double yaw_delta   = 2.0 * M_PI / 180.0;
  double slide_delta = 50.0;
  double zoom_delta  = 50.0;
	tvEvent event;

	if (tvbase) 
		tvBasePostRedraw(tvbase, FALSE);
	
	switch (currentEvent->id) {
		case tvEvent::KEY_PRESS: {
			switch (currentEvent->key) {
				case tvEvent::KEY_BACK_SPACE: {
					int num;

					if ( ( num = tvNumDatasets( tvbase ) ) > 0 ) {
						TsGeoPyramid *geo = tvGetNthDataset( tvbase, num-1 );
						if ( geo ) tvRemoveDataset( tvbase, geo );
					}
				}
				break;

					// SPACE = stop + refresh window (simulate a MMB click) 
				case tvEvent::KEY_SPACE:
					ThreeDWidgetBtn2Down( xMouse, yMouse );
					ThreeDWidgetBtn2Up( xMouse, yMouse );
				break;
				
          // 2 = go to 2-D mode 
				case '2':
					tvSetBoolean( tvbase, TV_TWOD_CONSTRAIN, TRUE );
					tvBaseUpdateView ( tvbase, FALSE );
				break;
				
				// 3 = go to 3-D mode 
				case '3':
					tvSetBoolean( tvbase, TV_TWOD_CONSTRAIN, FALSE );
					tvBaseUpdateView ( tvbase, FALSE );
				break;
				
				// O = reset to origin 
				case 'O':
				case 'o':
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED)) {
						tvSetBoolean( tvbase, TV_SHOW_DATASETS, ! tvGetBoolean( tvbase, TV_SHOW_DATASETS ) );
						tvBasePostRedraw( tvbase, FALSE );
					} else {
						ThreeDWidgetBtn2Down( xMouse, yMouse );
						ThreeDWidgetBtn2Up( xMouse, yMouse );
						tvBaseInitView( tvbase, tvBaseTop(tvbase) );
					}
				break;
				
				//* H = toggle the HUD 
				case 'H':
				case 'h':
					tvSetBoolean( tvbase, TV_SHOW_HEADS_UP, ! tvGetBoolean( tvbase, TV_SHOW_HEADS_UP ) );
					tvBasePostRedraw( tvbase, FALSE );
				break;

				// L = toggle the SRI Logo/lighting
				case 'L':
				case 'l':
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED)) {
						tvSetBoolean( tvbase, TV_SHOW_LOGO, ! tvGetBoolean( tvbase, TV_SHOW_LOGO ) );
						tvBasePostRedraw( tvbase, FALSE );
					}	else {
						tvSetBoolean( tvbase, TV_SHOW_LIGHTING, ! tvGetBoolean( tvbase, TV_SHOW_LIGHTING ) );
						tvBasePostRedraw( tvbase, FALSE );
					}
				break;
			
				// W = toggle wireframe
				case 'W':
				case 'w':
					tvSetBoolean( tvbase, TV_SHOW_WIREFRAME, ! tvGetBoolean( tvbase, TV_SHOW_WIREFRAME ) );
					tvBasePostRedraw( tvbase, FALSE );
				break;

				// - = toggle z-buffer hack 
				case '-':
					tvSetBoolean( tvbase, TV_USE_ZBUFFER_HACK, ! tvGetBoolean( tvbase, TV_USE_ZBUFFER_HACK ) );
					tvBaseUpdateView( tvbase, FALSE );
				break;
				
				// Left Arrow = rotate or slide to the left 
				case tvEvent::KEY_LEFT:
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED))
						tvMoveViewpoint( tvbase, 0, -slide_delta, 0, FALSE );
					else
						tvRotate( tvbase, yaw_delta, 0, 0, FALSE );
				break;
				
				// Right Arrow = rotate or slide to the right 
				case tvEvent::KEY_RIGHT:
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED))
						tvMoveViewpoint( tvbase, 0, slide_delta, 0, FALSE );
					else
						tvRotate( tvbase, -yaw_delta, 0, 0, FALSE );
				break;
				
				// Up Arrow = increase pitch, slide, or zoom 
				case tvEvent::KEY_UP:
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED))
						tvMoveViewpoint( tvbase, 0, 0, -slide_delta, FALSE );
					else if (getKeyMode(tvEvent::CTRL_KEY_PRESSED))
						tvMoveViewpoint( tvbase, zoom_delta, 0, 0, FALSE );
					else
						tvRotate( tvbase, 0, -pitch_delta, 0, FALSE );
				break;
				
				// Down Arrow = decrease pitch, slide, or zoom 
				case tvEvent::KEY_DOWN:
					if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED))
						tvMoveViewpoint( tvbase, 0, 0, slide_delta, FALSE );
					else if (getKeyMode(tvEvent::CTRL_KEY_PRESSED))
						tvMoveViewpoint( tvbase, -zoom_delta, 0, 0, FALSE );
					else
						tvRotate( tvbase, 0, pitch_delta, 0, FALSE );
				break;
				
				// PgUp = fly to next viewpoint in bookmark list 
				case tvEvent::KEY_PAGE_UP:
					ViewpointCycleUp(tvbase, &currBookmark);
				break;
				
				// PgUp = fly to previous viewpoint in bookmark list 
				case tvEvent::KEY_PAGE_DOWN:
					ViewpointCycleDown(tvbase, &currBookmark);
				break;
			}
		}			
		break;
		
		case tvEvent::KEY_ACTION: {
			switch (currentEvent->modifier) {
				// Record whether the shift key is currently depressed 
				case tvEvent::SHIFT_KEY_PRESSED:
					setKeyMode(tvEvent::SHIFT_KEY_PRESSED, true);
				break;
				
				// Record whether the ctrl key is currently depressed 
				case tvEvent::CTRL_KEY_PRESSED:
					setKeyMode(tvEvent::CTRL_KEY_PRESSED, true);
				break;
					
				// Record whether the alt key is currently depressed 
				case tvEvent::ALT_KEY_PRESSED:
					setKeyMode(tvEvent::ALT_KEY_PRESSED, true);
				break;
			}
		}
		break;

		case tvEvent::KEY_ACTION_RELEASE: {
			switch (currentEvent->modifier) {
				// Record whether the shift key is currently depressed 
				case tvEvent::SHIFT_KEY_PRESSED:
					setKeyMode(tvEvent::SHIFT_KEY_PRESSED, false);
				break;
				
				// Record whether the ctrl key is currently depressed 
				case tvEvent::CTRL_KEY_PRESSED:
					setKeyMode(tvEvent::CTRL_KEY_PRESSED, false);
				break;
					
				// Record whether the alt key is currently depressed 
				case tvEvent::ALT_KEY_PRESSED:
					setKeyMode(tvEvent::ALT_KEY_PRESSED, false);
				break;
			}
		}
		break;
	}
}

void tvViewer::processEvent(tvEvent *currentEvent) {
	int winsize[2];

	switch (currentEvent->id) {
		case tvEvent::MOUSE_UP:
		case tvEvent::MOUSE_DOWN:
		case tvEvent::MOUSE_MOVE:
			processMouse(currentEvent);
		break;

		case tvEvent::KEY_PRESS:
		case tvEvent::KEY_RELEASE:
		case tvEvent::KEY_ACTION:
		case tvEvent::KEY_ACTION_RELEASE:
			processKey(currentEvent);
		break;

		case tvEvent::WINDOW_RESIZED:
			winsize[0] = currentEvent->x;
			winsize[1] = currentEvent->y;

			if (tvbase) {
				tvSetIntegerv(tvbase, TV_SIZE, winsize);
				tvBasePostRedraw(tvbase, FALSE);
			}
		break;

		case tvEvent::WINDOW_REDRAW:
			if (tvbase) 
				tvBasePostRedraw( tvbase, FALSE );
		break;
	}
}

void tvViewer::startRendering() { 
  static char * funcName = "startRendering";
  tvViewpoint *origin = &originView;

  /* Check for the Origin bookmark name and use it if set. */
  memset (origin, 0, sizeof (*origin));

  if ( GetViewpointOrigin (origin->eye, &origin->yaw, &origin->pitch, &origin->roll) ) {
    TSMTRACE ((THREED_MASK|1, funcName, "Found origin: %.0f,%.0f,%.0f\n", origin->eye[0], origin->eye[1], origin->eye[2]));
    tvSetInitialView (tvbase, origin);
  }

  tvBaseStart (tvbase);
}

void tvViewer::stopRendering() { 
	tvBaseReset(tvbase);
	tvBaseStop(tvbase);
}

bool tvViewer::getKeyMode(tvEvent::ModifierEnum key) {
	if (modifierMode & key)
		return true;
	else
		return false;
}

void tvViewer::setKeyMode(tvEvent::ModifierEnum key, bool mode) {
	if (mode)
		modifierMode |= key ;
	else
		modifierMode ^= key ;
}

void tvViewer::render() {
	glmWRunWorkProcs();
}

void tvViewer::ThreeDWidgetBtn1Down( int x, int y ) {
  static char *funcName = "ThreeDWidgetBtn1Down";
  int         crosshairpos[2];

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */
  
  if ( PathIsPlaying(tvbase) ) return;
 
  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_BtnDn( x, y );
    return;
  }

  if (seek || getKeyMode(tvEvent::ALT_KEY_PRESSED)) {
    double gcc[3];
    tvScreenToGCC( tvbase, x, y, gcc );
    tvLookAtGCC( tvbase, gcc );
    return;
  }

  /* Otherwise, we're navigating */

  /* GET THE MOUSE POSITION */

  xMouse = xMouseDown = x;
  yMouse = yMouseDown = y;
  mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartAccelerate (tvbase);

  tvSetBoolean( tvbase, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( tvbase, TV_CROSSHAIR, crosshairpos );
}

void tvViewer::ThreeDWidgetBtn2Down( int x, int y ) {
  static char *funcName = "ThreeDWidgetBtn2Down";
  int         crosshairpos[2];

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */

  if ( PathIsPlaying(tvbase) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_BtnDn( x, y );
    return;
  }

  /* Otherwise, we're navigating */
  
  /* Set speed back to zero when we start */

  tvSetFloat( tvbase, TV_VELOCITY, 0.0 );
  tvSetFloat( tvbase, TV_ACCELERATION, 0.0 );

  /* find where mouse down occured, and origin of window */

  xMouse = xMouseDown = x;
  yMouse = yMouseDown = y;
  mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartMovement (tvbase);

  tvSetBoolean( tvbase, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( tvbase, TV_CROSSHAIR, crosshairpos );
}

void tvViewer::ThreeDWidgetBtn3Down( int x, int y ) {
  static char *funcName = "ThreeDWidgetBtn3Down";
  int         crosshairpos[2];

  crosshairpos[0] = x;
  crosshairpos[1] = y;

  /* no navigation during animation */

  if ( PathIsPlaying(tvbase) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_BtnDn( x, y );
    return;
  }

  /* Otherwise, we're navigating */

  /* find where mouse down occured, and origin of window */

  xMouse = xMouseDown = x;
  yMouse = yMouseDown = y;
  mouseDown = TRUE;

  TSMTRACE ((THREED_MASK|1, funcName, "%d,%d", x, y));

  tvStartDeccelerate (tvbase);

  tvSetBoolean( tvbase, TV_SHOW_CROSSHAIR, TRUE );
  tvSetIntegerv( tvbase, TV_CROSSHAIR, crosshairpos );
}

void tvViewer::ThreeDWidgetBtnUp( int x, int y ) {
  /*static char const funcName[] = "ThreeDWidgetBtnUp";*/

  /* no navigation during animation */

  if ( seek || getKeyMode(tvEvent::ALT_KEY_PRESSED) ) return;

  if ( PathIsPlaying(tvbase) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_BtnUp( x, y );
    return;
  }

  /* Otherwise, we're navigating... */

  tvStartMovement (tvbase);
}

void tvViewer::ThreeDWidgetBtn2Up( int x, int y ) {
  /*static char const funcName[] = "ThreeDWidgetBtn2Up";*/

  mouseDown = FALSE;
  tvSetBoolean( tvbase, TV_SHOW_CROSSHAIR, FALSE );

  /* no navigation during animation */

  if ( PathIsPlaying(tvbase) ) return;

  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_BtnUp( x, y );
    return;
  }

  tvStopMovement (tvbase);
}

void tvViewer::EmulateMMB( int lmb, int *mmb, int rmb, int x, int y ) {
  static int prev_mmb = FALSE;

  /* emulate the middle mouse button by pressing the left and right */

  if ( lmb && rmb && ! prev_mmb ) {
    ThreeDWidgetBtnUp( x, y );
    ThreeDWidgetBtn2Down( x, y );
    *mmb = TRUE;
  }

  /* release the middle mouse button when left and right released */

  if ( lmb == FALSE && rmb == FALSE && prev_mmb ) {
    ThreeDWidgetBtn2Up( x, y );
    *mmb = FALSE;
  }

  prev_mmb = *mmb;
}

void tvViewer::ThreeDWidgetMotion( int x, int y ){
  static char *funcName = "ThreeDWidgetMotion";
  int     screensize[2], twod_mode;

  /* are we drawing on the screen rather than navigating? */

  if ( drawOnScreen ) {
    DrawOnScreen_Motion( x, y );
    return;
  }

  /* get the window's current width and height, and other settings */

  tvGetIntegerv( tvbase, TV_SIZE, screensize );

  twod_mode = tvGetBoolean( tvbase, TV_TWOD_CONSTRAIN );
 
  /* otherwise, we're navigating */

  if (mouseDown || getKeyMode(tvEvent::SHIFT_KEY_PRESSED) || getKeyMode(tvEvent::CTRL_KEY_PRESSED)  || getKeyMode(tvEvent::ALT_KEY_PRESSED)) {
    int scale = (( screensize[0] > screensize[1] ) ? screensize[0] : screensize[1] );
    float d[2];

    d[0] =  ((x - xMouseDown) / (scale / 2.0));
    d[1] = -((y - yMouseDown) / (scale / 2.0));

    xMouse = x;
    yMouse = y;

    if (getKeyMode(tvEvent::SHIFT_KEY_PRESSED) || getKeyMode(tvEvent::CTRL_KEY_PRESSED) || twod_mode)
      tvSetFloatv (tvbase, TV_SLIDE_DELTA, d);
    else {
      if (! airplaneControl)
				d[1] *= -1; 
      tvSetFloatv (tvbase, TV_ATTITUDE_DELTA, d);
    }

    /* clear the Viewpoint field of the status bar (if set), because */
    /* if we have moved then we are no longer at that viewpoint.     */
    
    TSMTRACE ((THREED_MASK|2, funcName, "%d,%d", x, y));
  }
}

void tvViewer::DrawOnScreen_BtnDn( int x, int y ) {
/*
  buttonDown = TRUE;
  tvBeginStroke (tvbase, x, y);
*/
}

void tvViewer::DrawOnScreen_BtnUp( int x, int y ) {
/*
  buttonDown = FALSE;
  tvEndStroke (tvbase);
*/
}

void tvViewer::DrawOnScreen_Motion( int x, int y ) {
/*
  if ( buttonDown )
    tvContinueStroke (tvbase, x, y);
*/
}
