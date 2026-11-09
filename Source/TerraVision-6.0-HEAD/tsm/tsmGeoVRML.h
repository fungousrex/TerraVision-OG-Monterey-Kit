/*
 * Name: tsmGeoVRML.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   Updated Oct 2001 by Mike McCann of MBARI to support GeoOrigin
 *
 *   $Id: tsmGeoVRML.h,v 1.4.2.2 2001/10/30 18:31:23 reddy Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#ifndef _TSMGEOVRML_H_
#define _TSMGEOVRML_H_

#include <vrml2/vrml2.h>
#include "tsm.h"
#include "tsmTileCoord.h"

typedef struct {
  TsmConnection oiConnection;
  TsmConnection demConnection;
  TsmConnectionParams params;

  Pyramid *oiPyramid;
  Pyramid *demPyramid;

  vsgWorld *wrl;
  vsgClass *geoOriginClass;
  vsgClass *geoLODClass;
  vsgClass *geoElevationGridClass;
  vsgClass *geoViewpointClass;
  vsgClass *geoCoordinateClass;
  vsgClass *geoTouchSensorClass;
  vsgClass *ScriptClass;

  FILE *ferror;

  double maxu;
  double maxv;
} TsmGeoVRML;


#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }

#define DEBUG(msg) printf(msg);

#define EARTH_RADIUS 6378137

TsmGeoVRML *TsmGeoVRML_new(int argc, char **argv);
void TsmGeoVRML_destroy(TsmGeoVRML **this);
void TsmGeoVRML_usage();
double TsmGeoVRML_getElevation(TsmGeoVRML *this, TileCoord *oiCoord);
SFNode TsmGeoVRML_createGeoElevationGrid(TsmGeoVRML *this, TileCoord *oiCoord, double *ll, double *ur, SFNode geoOriginNode);
SFNode TsmGeoVRML_createTestTileModel(TsmGeoVRML *this, TileCoord *coord, double *ll, double *ur);
int TsmGeoVRML_writeTileModel(TsmGeoVRML *this, TileCoord *oiCoord);
int TsmGeoVRML_writeTileImage(TsmGeoVRML *this, TileCoord *oiCoord);
int TsmGeoVRML_writeAllTiles(TsmGeoVRML *this);
int TsmGeoVRML_oi2DemCoord(TsmGeoVRML *this, TileCoord *oiCoord, TileCoord *demCoord);
uchar *TsmGeoVRML_getDEMTile(TsmGeoVRML *this, TileCoord *tileCoord);

void TsmGeoVRML_ixiy2uv(Pyramid *pyramid, int level, double ix, double iy, double *u, double *v);
void TsmGeoVRML_uv2ixiy(Pyramid *pyramid, int level, double u, double v, double *ix, double *iy);
int TsmGeoVRML_uv2gcc(Pyramid *pyramid, double u, double v, double elevation, double *x, double *y, double *z);
int TsmGeoVRML_gcc2uv(Pyramid *pyramid, double x, double y, double z, double *u, double *v, double *elevation);

int TsmGeoVRML_getCenter(Pyramid *pyramid, double *ll, double *ur, double *u, double*v, double *elevation);
double TsmGeoVRML_calcRange(TsmGeoVRML *this, TileCoord *coord, double *ll, double *ur);
uchar *TsmGeoVRML_convertToRGBBuffer(Pyramid *pyramid, uchar *buffer);

void TsmGeoVRML_calcNormal(TsmGeoVRML *this, TileCoord *coord, double height, double dx, double dy, double *normal);
void TsmGeoVRML_crossProduct(double *v1, double *v2, double *result);
void TsmGeoVRML_normalize(double *v);


#endif
