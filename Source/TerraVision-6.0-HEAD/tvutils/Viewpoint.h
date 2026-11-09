/*
  Filename: Viewpoint.h

  Description: Contains headers for the routines to manage the
  TerraVision viewpoint bookmarks file, including locating and parsing
  the viewpoints list.

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#ifndef TVU_VIEWPOINT_H 
#define TVU_VIEWPOINT_H

#define TVU_PI  3.1415926535897932

/* loads the new XML-format TerraVision viewpoints file */
int tvuLoadViewpoints_new( void );

/* loads the specified (or default) old-style TV viewpoints file */
int tvuLoadViewpoints_old( char *filename );


#endif
