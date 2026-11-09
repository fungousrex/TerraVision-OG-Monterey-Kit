/*****************************************************************************
 * Name:
 *  tvViewer.h
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

#ifndef __tvViewer_H__
#define __tvViewer_H__

//#include "TsGeoPyramid.h"
#include "ThreeDCore.h"

#include "tvWinDef.h"
#include "tvEvent.h"
#include "tvDVector.h"

class tvViewer {
// Creating and destroying
public:
	tvViewer();
	~tvViewer();

// Public methodes
public:
	void	init(HWND hWnd);

	void	processEvent(tvEvent *currentEvent);

	void	render();
	void	startRendering();
	void	stopRendering();

	void	setViewpointURL(const char * url);

// Set/Get
	tvBase *getTVBase() { return tvbase; }

// Private methodes
private:
	void	processMouse(tvEvent *currentEvent);
	void	processKey(tvEvent *currentEvent);

	bool	getKeyMode(tvEvent::ModifierEnum key);
	void	setKeyMode(tvEvent::ModifierEnum key, bool mode);

	void	EmulateMMB( int lmb, int *mmb, int rmb, int x, int y );

	void	ThreeDWidgetBtn1Down( int x, int y );
	void	ThreeDWidgetBtn2Down( int x, int y );
	void	ThreeDWidgetBtn3Down( int x, int y );

	void	ThreeDWidgetBtnUp( int x, int y );
	void	ThreeDWidgetBtn2Up( int x, int y );

	void	ThreeDWidgetMotion( int x, int y );

	void	DrawOnScreen_BtnDn( int x, int y );
	void	DrawOnScreen_BtnUp( int x, int y );
	void	DrawOnScreen_Motion( int x, int y );

// Variables
private:
  tvBase		   *tvbase;							// pointer to our core renderer 

  int           currBookmark;				// used to cycle between views

  int 	        xMouseDown;
  int 	        yMouseDown;
  int           xMouse;
  int           yMouse;

  short	        flyMode;
  short         airplaneControl;
  short         seek;
  short         drawOnScreen;
  short         animateBookmarks;

  short         mouseDown;					// TRUE while mouse buttons down 

  tvViewpoint   originView;

	int						modifierMode;				// control SHIFT, ALT and CTRL, see tvEvent.h

	PRThread		 *main_thread;
};

#endif
