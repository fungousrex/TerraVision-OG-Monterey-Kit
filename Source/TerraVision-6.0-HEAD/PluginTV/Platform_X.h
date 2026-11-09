/*
 * Name:  
 *	Platform_X.h
 *
 * Description: 
 *	Header file for the X/Windows specific definitions.
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

#ifndef PLATFORM_X_H
#define PLATFORM_X_H

#include <X11/Intrinsic.h>

typedef struct {
  Widget         windowArea;         /* the OpenGL rendering widget for X */
  Window         window;             /* Window structure returned by NC */
  Widget         parent;             /* NC's Window converted to Widget */
  Display        *display;           /* The X Server display */
  XtAppContext   appContext;         /* the X application context */
  XVisualInfo    visInfo;            /* the X visual information */
} PlatformWidget;

#endif /* not defined THREEDWIDGET_H */
