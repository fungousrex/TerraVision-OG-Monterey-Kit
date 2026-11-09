/*
 * Name: tsmTileInt.h
 *
 * Description: 
 *   A set of utility functions that are used by tsmVRML.c in order
 *   to provide the tsmApi VRML support facilities.
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id"
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

#ifndef TSMVRMLUTILS_H
#define TSMVRMLUTILS_H

/*
 * Utility functions for creating TerraVRML datasets
 *
 * Author: Nat Bletter
 *
 * Date: 4/28/98
 */

#include <vrml2/vrml2.h>
#include "tsm.h"

/* a macro to convert from Z-up coordinate frame to VRML's Y-up */
/* The conversion formula is: x' = x,  y' = z, z' = -y          */

extern double tsm_temp_coord;

#define TSM_MAKE_Y_UP(x,y,z) {tsm_temp_coord = y; y = z; z = -tsm_temp_coord;}

/* defines for the elev_mode parameter of tv2vGetCoords */

#define TSM_ELEV_MODE_EXACT  0
#define TSM_ELEV_MODE_MIN    1
#define TSM_ELEV_MODE_MAX    2

typedef struct{
  double ll[3];         /* lower-left (vx,vy,vz) coordinate of bounding box */
  double ur[3];         /* upper-right (vx,vy,vz) coordinate of bounding box */
  double size[3];       /* size of the bounding box in each dimension */
  double mid[3];        /* center (vx,vy,vz) coordinate of bounding box */
  uchar  *buffer;       /* a tile buffer for this tile */
  int    tx, ty, level; /* tile coordinates */
  int    key;           /* a hashing key for this tile */
} TsmCachedTile;

int tsmVRMLCacheInit( void );
int tsmVRMLCacheFree( void );
TsmCachedTile *tsmVRMLCacheGetBBox( int tx, int ty, int level );
TsmCachedTile *tsmVRMLCacheGetBuffer( int tx, int ty, int level );
int tsmVRMLCacheAddBBox( TsmCachedTile *bbox );
int tsmVRMLCacheAddBuffer( TsmCachedTile *bbox );

void tsmTileToVRMLCoord( Pyramid *tspec, double tx, double ty, int level,
			 double *x, double *y );

void tsmTileToVRMLBoundingBox( Pyramid *tspec, int tx, int ty, int level,
			    double *p1, double *p2 );

void tsmTileToUpVector( Pyramid *tspec, int tx, int ty, int level, 
			TsmConnectionParams params, float *upVector);


int tsmGetCoords( TsmConnection oiConn, TsmConnection demConn, double x, 
		  double y, int demLevel, TsmConnectionParams params,
		  double *coords, int elev_mode );

void tsmGetTexCoords( int x, int y, int tx, int ty, int level,
		      Pyramid *pyramid, double *coords, int edge_x, int edge_y,
		      TsmConnectionParams params );

int tsmTVToVRMLLevel( Pyramid const *tspec, int level );

void tsmMakeVtileFilename( Pyramid const *tspec, int tx, int ty, int level,
			   TsmConnectionParams params, char *filename );
void tsmMakeGeotileFilename( Pyramid const *tspec, int tx, int ty, int level,
			     TsmConnectionParams params, char *filename );
void tsmMakeTreeFilename( Pyramid const *tspec, int tx, int ty, int level,
			  TsmConnectionParams params, char *filename );
void tsmMakeImageFilename( Pyramid const *tspec, int tx, int ty, int level,
			   char *filename, TsmConnectionParams params );

double tsmSetInitDistance( double const *ll, double const *ur );

double tsmSetNearPlane( double const *ll, double const *ur );

double tsmCalcRange( Pyramid const *tspec, int tx, int ty, int level,
		     int end_level, TsmConnectionParams params, double *ll,
		     double *ur );

#endif
