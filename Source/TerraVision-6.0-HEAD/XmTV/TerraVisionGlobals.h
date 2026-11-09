/*
 * Name:  TerraVisionGlobals.h
 *
 * Description: Global variables for TerraVision.
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

#ifndef TERRAVISIONGLOBALS_H
#define TERRAVISIONGLOBALS_H

#include <pthr/list.h>

#include "pixmaps.h"

/* ENSURE THAT THESE ARE GLOBALLY VISIBLE AND DEFINED IN TerraVision.c */
#ifndef tvExtern
#  define tvExtern extern
#endif

tvExtern char const *urlPaths[NUMB_WEB_ICONS];   /* PATHS FOR THE URLS */
tvExtern int        *scannedMapFlag;

#ifndef CAVE_DEFS
#define CAVE_DEFS
#ifdef CAVE
#include <cave_ogl.h>
  tvExtern float *caveYaw;
  tvExtern float *cavePitch;
#endif
#endif


#endif /* TERRAVISIONGLOBALS_H */
