/*
 * Name:  
 *	ThreeDWidgetI.h
 *
 * Description: 
 *	Internal header file for a threeD widget.
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
 */
#ifndef THREEDWIDGETI_H
#define THREEDWIDGETI_H

#include "ThreeDWidget.h"
#include "ThreeDCore.h"
#include "TsGeoPyramid.h"
#include "matrix.h"

#ifdef WIN32
#include "Platform_win.h"
#else
#include "Platform_X.h"
#endif

struct ThreeDWidget {
  PlatformWidget     platform;          /* platform-specific structures */

  int                currBookmark;      /* used to cycle between views */
  tvBase             *threeDCore;       /* pointer to our core renderer */
  char 	             *title;            /* title for this context */

  int                x, y;              /* Window (x, y) from NC */
  int                width, height;     /* Window (width, height) from NC */
  int                is_plugin;         /* TRUE = called as a plugin */

  int 	             xMouseDown;
  int 	             yMouseDown;
  int                xMouse;
  int                yMouse;

  short	             flyMode;
  short              airplaneControl;
  short              seek;
  short              drawOnScreen;
  short              animateBookmarks;
  short              singleBuffer;	/* Render in single buffer mode? */


  short              mouseDown;         /* TRUE while mouse key(s) down */
  short              shiftDown;         /* TRUE while shift key(s) down */
  short              ctrlDown;          /* TRUE while control key(s) down */
  short              altDown;           /* TRUE while alt key(s) down */

  tvViewpoint        originView;
};

extern void DestroyThreeDWidget( ThreeDWidget * );

extern void DrawOnScreen_BtnDn( ThreeDWidget *threeDWidget, int x, int y );
extern void DrawOnScreen_BtnUp( ThreeDWidget *threeDWidget, int x, int y );
extern void DrawOnScreen_Motion( ThreeDWidget *threeDWidget, int x, int y );

/* the following functions are implemented platform-specifically */
/* either in Platform_X.c for X11 or Platform_win.c for MS-Win.  */

extern int  PlatformSetupGraphics( ThreeDWidget * );
extern void PlatformShowWindow( ThreeDWidget * );
extern void PlatformTimedFunc( ThreeDWidget *, void (*func)(void), int ms );
extern int  PlatformInitThreeDWidget( ThreeDWidget * );
extern void PlatformGraphicsLoop( void *arg );
extern void PlatformCloseDown( ThreeDWidget *threeDWidget );
extern void ThreeDWidgetInitCallback( ThreeDWidget *threeDWidget );


#endif /* THREEDWIDGETI_H */

