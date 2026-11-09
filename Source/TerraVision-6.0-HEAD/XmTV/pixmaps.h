/*
 * Name: pixmaps.h
 *
 * Description: routines for managing the pixmaps that are used for the
 *    web icons in the viewer.
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

#ifndef PIXMAPS_H
#define PIXMAPS_H

#include <X11/Intrinsic.h>

/* The number of web icons that appear at the bottom of the viewer  */

#define NUMB_WEB_ICONS   4

/* list the web icon names - you can change the order in which these */
/* are defined in order to change the order of the web icons on screen */

typedef enum {
  DARPA_PIXMAP = 0,
  SRI_PIXMAP,
  MAGIC_PIXMAP,
  TV_PIXMAP
} WebIcon_t;

/* this array contains the X11 Pixmaps for each of the web icons */
/* but only after you have called the MakeWebIconPixmaps() func. */

extern Pixmap web_icon[NUMB_WEB_ICONS];


/* call this function to create the web icon pixmaps */

void MakeWebIconPixmaps( Widget wid );

/* call this function to make a pixmap to be used for the iconified image */

Pixmap MakeIconifiedPixmap( Widget wid );


#endif
