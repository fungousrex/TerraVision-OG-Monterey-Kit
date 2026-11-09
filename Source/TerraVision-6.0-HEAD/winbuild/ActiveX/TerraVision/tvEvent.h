/*****************************************************************************
 * Name:
 *    	tvEvent.h
 *
 * Description:
 *	MS-Windows specific routines
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
 *  Contributor(s): Michael Eriksen
 *  ----- END LICENSE BLOCK -----
 *
 *****************************************************************************/

#ifndef __tvEvent_H__
#define __tvEvent_H__

#include "tvWinDef.h"

class tvEvent {
// Constructors
public:
	tvEvent();
	virtual ~tvEvent();

// Enums
public:
	enum ModifierEnum {
		SHIFT_KEY_PRESSED = 0x1,
		CTRL_KEY_PRESSED 	= 0x2,
		ALT_KEY_PRESSED 	= 0x4
	};

  enum MouseButtonEnum {
		LEFT_MOUSE_BUTTON		= 0x1,
		MIDDLE_MOUSE_BUTTON	= 0x2,
		RIGHT_MOUSE_BUTTON	= 0x4
  };

  enum SpecialKeyEnum {
   	// function keys 
		KEY_F1				=	1,
		KEY_F2				= 2,
		KEY_F3				= 3,
		KEY_F4				= 4,
		KEY_F5				= 5,
		KEY_F6				= 6,
		KEY_F7				= 7,
		KEY_F8				= 8,
		KEY_F9				= 9,
		KEY_F10				= 10,
		KEY_F11				= 11,
		KEY_F12				= 12,

    // directional keys 
		KEY_LEFT				=	200,
		KEY_UP					=	201,
		KEY_RIGHT				= 202,
		KEY_DOWN				= 203,
		KEY_PAGE_UP			= 204,
		KEY_PAGE_DOWN		= 205,
		KEY_HOME				= 206,
		KEY_END					= 207,
		KEY_INSERT			= 208,
		KEY_BACK_SPACE	= 209,

		// misc
		KEY_SPACE     = 300
  };

	enum EventTypeEnum {
		NONE = 0,
		WINDOW_RESIZED = 1,
		WINDOW_REDRAW = 2,
		WINDOW_CLOSE = 3,
		MOUSE_UP = 4,
		MOUSE_DOWN = 5,
		MOUSE_MOVE = 6,
		KEY_PRESS = 7,
		KEY_RELEASE = 8,
		KEY_ACTION = 9,
		KEY_ACTION_RELEASE = 10,
		TIMER = 11
	};

public:			
	int			id;				// Type identifier, e.g, MOUSE_UP
	int			key;			// ASCII(??) key value
	Widget	target;		// Parent window
	int			x, y;			// Multipurpose
	int			button;		// Mouse button, e.g., LEFT_BUTTON
	int			modifier;	// Modifer keys
};

#endif



