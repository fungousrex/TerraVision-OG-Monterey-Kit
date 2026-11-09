/*
 * Name:  
 *	ScannedMap.h
 *
 * Description: 
 *	Header file for a scannedMap widget
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

#ifndef SCANNEDMAP_H
#define SCANNEDMAP_H

#include <glt/glm.h>

#include "TsGeoPyramid.h"
#include "ThreeDWidget.h"

#ifndef TERRAVISIONINC_H
typedef struct ScannedMapWidget ScannedMapWidget;
#endif

/* Displayer for scanned maps */
extern ScannedMapWidget *scannedMap;

/* This structure holds all of the info for a single map */

typedef struct ScannedMap {
  GLubyte      *mapdata;        /* the bitmap data for the scanned map */
  GLenum       format;          /* one of GL_LUMINANCE or GL_RGB */
  int          sizeX, sizeY;    /* the dimensions of the image */
  int          ncomps;          /* depth of the image: 3 = RGB, 1 = grey */
  float        minCoords[2];    /* the (u,v) coords for bottom left corner */
  float        maxCoords[2];    /* the (u,v) coords for the top right corner */
  char         *map_url;        /* url for the currently loaded map */
  char         *map_name;       /* name for the currently loaded map */
  TsGeoPyramid *geo;            /* Geo that this map is associated with */
  int          loading;         /* TRUE while loading a map image */
} ScannedMap;

/* This structure holds all of the Map Viewer widget structures */

struct ScannedMapWidget {
  Widget 	   mainForm;
  Widget           mapFormWidget;
  Widget           mapImageWidget;
  Widget           mapSelectWidget;
  Widget           mapListWidget;
  ScannedMap       map;
  ThreeDWidget     *linked3DWidget;
  glmWindow        glmw;
};

void DestroyScannedMap( ScannedMapWidget ** );
Boolean ScannedMapIsManaged( ScannedMapWidget const * );
void ScannedMapRedraw( void );
void ScannedMapShow( void );
void ScannedMapRefreshLists( void );

#ifdef CAVE
void ScannedMapCAVERedraw( void );
#endif

#endif /* not defined SCANNEDMAP_H */
