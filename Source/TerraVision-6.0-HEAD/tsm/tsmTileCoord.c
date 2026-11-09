/*
 * Name: tsmTileCoord.c
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmTileCoord.c,v 1.2 2000/08/28 22:16:14 reddy Exp $
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

#include <string.h>
#include <tsm/tsm.h>
#include "tsmTileCoord.h"


TileCoord *TileCoord_new() {
  TileCoord *this;

  if ((this = tsmMemMalloc(sizeof(TileCoord))) == NULL) return NULL;

  memset(this, 0, sizeof(TileCoord));

  return this;
}

TileCoord *TileCoord_new2(double u, double v, double ix, double iy, int tx, int ty, int level) {
  TileCoord *this;

  if ((this = tsmMemMalloc(sizeof(TileCoord))) == NULL) return NULL;

  this->u = v; this->v = v;
  this->ix = ix; this->iy = iy;
  this->tx = tx; this->ty = ty;
  this->level = level;

  this->dx = this->dy = 0;

  return this;
}

TileCoord *TileCoord_new1(TileCoord *coord) {
  TileCoord *this;

  if ((this = tsmMemMalloc(sizeof(TileCoord))) == NULL) return NULL;

  this->u = coord->u; this->v = coord->v;
  this->ix = coord->ix; this->iy = coord->iy;
  this->tx = coord->tx; this->ty = coord->ty;
  this->dx = coord->dx; this->dy = coord->dy;
  this->level = coord->level;

  return this;
}


void TileCoord_destroy(TileCoord **this) {
  if (this!=NULL && *this!=NULL) {
    tsmMemFree(*this);
    this = NULL;
  }
}

