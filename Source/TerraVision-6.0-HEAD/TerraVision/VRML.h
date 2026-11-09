/* -*- tab-width: 2; -*-

  Name:  VRML.h
 
  Description: Header file for VRML rendering support in TerraVision
 
  Revision History:
      $Id: VRML.h,v 2.4.14.3 2002/10/31 05:26:43 reddy Exp $

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

#ifndef VRML_H
#define VRML_H

/* install TerraVision's VRML rendering code as a callback   */
/* for the vrml2 library's call vsgRender(). This only needs */
/* to be done once. */

void InitVRML( void );

/* these functions are exported from the tsmApi library */

extern void tsmNormalizeVector( double const *a, double *n, int dim );
extern void tsmCrossProduct( double const a[3], double const b[3], double c[3] );

#endif

