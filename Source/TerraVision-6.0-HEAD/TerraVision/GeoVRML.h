/* -*- tab-width: 2; -*-

  Name: GeoVRML.h
 
  Description: This file specifies the interface to the various utility
     functions for supporting GeoVRML nodes

  Revision History:
      $Id: GeoVRML.h,v 2.2.4.2 2002/10/31 05:26:30 reddy Exp $

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

#ifndef GEOVRML_H
#define GEOVRML_H

#include <vrml2/vrml2.h>
#include "ThreeDCore.h"

/* allocation and freeing of the GeoVRML state structure */

typedef struct GeoVRMLStruct *GeoVRML;

GeoVRML gvNew( vsgWorld *wrl );
void gvFree( GeoVRML gv );

/* version info routine for this implementation of GeoVRML */

char const *gvGetVersion( void );

/* origin manipulation routines */

void gvGetOrigin( GeoVRML gv, SFNode node, double gcc[3] );
void gvSetOrigin( GeoVRML gv, SFNode node );
void gvSetOrigin3fv( GeoVRML gv, double origin[3] );

/* calculating the local orientation at a point on the earth's surface */

void gvLocalOrientation( GeoVRML gv, double gcc[3], float orient[4] );

/* metadata node parsing routines */

char *gvGetMetadataItem( GeoVRML gv, SFNode node, char const *token );

/* coordinate parsing routines for GeoVRML 1.0 style strings */

void gvParserMFInit( GeoVRML gv, MFString *, MFString * );
void gvParserSFInit( GeoVRML gv, SFString, MFString * );
int gvParserNextGCC( GeoVRML gv, double gcc[3] );

/* a routine to pass in a tvBase structure - needed for DEM-relative elevs */

void gvSetViewer( GeoVRML gv, tvBase *threeD );


#endif
