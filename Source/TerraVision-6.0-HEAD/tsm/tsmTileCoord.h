/*
 * Name: tsmTileCoord.h
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmTileCoord.h,v 1.2 2000/08/28 22:16:15 reddy Exp $
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

#ifndef _TILECOORD_H_
#define _TILECOORD_H_


typedef struct {
  double u;
  double v;     /* (u,x) from lower left of TileSet, in geographic coordinate */
  double ix;
  double iy;    /* (x,y) from lower left of Tileset */
  double dx;
  double dy;    /* (x,y) from lower left of Tile */
  int tx;
  int ty;    /* Tile indices */
  int level; /* Which level */
} TileCoord;

TileCoord *TileCoord_new();
TileCoord *TileCoord_new1(TileCoord *coord);
TileCoord *TileCoord_new2(double u, double v, double ix, double iy, int tx, int ty, int level);
void TileCoord_destroy(TileCoord **this);


#endif
