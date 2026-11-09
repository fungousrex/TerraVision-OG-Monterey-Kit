/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsGeoPyramidI.h,v 2.3.2.5 2002/10/31 05:26:40 reddy Exp $

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

#ifndef _TsGeoPyramidI_h_
#define _TsGeoPyramidI_h_

#include "TsCacheI.h"

typedef struct TsGeoCalls TsGeoCalls;

/*
 * Doc++ Description: *//** 
 *   A call vector for type-specific functions on TsGeoPyramids.
 *
 *   The base type for manipulation of geographical data, organized as a
 *   multi-resolution pyramid for efficient acces and display.  It is indexed
 *   via the imagery pyramid (a TsCache) and gets its geometry from an
 *   associated geometric structure (either a VRML tree or a DEM pyramid).
 *
 * @param destroy      Destroy pyramid and free its memory
 * @param prepare_tile Prepare a tile for display
 * @param sync         Send all pending tile requests.
 * @param reset        Clear all pending requests
 * @param geometry     Obtain the geometry description for a tile.
 *
 * @see TsGeoPyramid
 *
 */

struct TsGeoCalls {
  void (*destroy) (TsGeoPyramid *geo);
  int (*connections) (TsGeoPyramid *geo, TsmConnection *cons);
  TsGeoView * (*create_view) (TsGeoPyramid *geo);

  float (*elevation) (TsGeoPyramid const *geo, float u, float v);
  int (*add_callback) (TsGeoPyramid *geo, TsCacheCB which,
		       TsCallbackF func, void *closure, 
		       void (*destroyf)(void*));
  int (*remove_callback) (TsGeoPyramid *geo, TsCacheCB which,
			  TsCallbackF func, void *closure);
  int (*urls) (TsGeoPyramid const *geo, char const **urls);
}; 

/*
 * Doc++ Description: *//** 
 *   A pyramid of terrain and imagery.
 *
 *   The base type for manipulation of geographical data, organized as a
 *   multi-resolution pyramid for efficient acces and display.  It is indexed
 *   via the imagery pyramid (a TsCache) and gets its geometry from an
 *   associated geometric structure (either a VRML tree or a DEM pyramid).
 *
 * @param type  The type identifier
 * @param im    The imagery pyramid cache.
 * @param calls The type-specific call vector for type-specific functions.
 *
 * @see TsGeoPyramidType, TsCache, TsGeoCalls
 *
 */

struct TsGeoPyramid {
  TsGeoPyramidType type;
  TsCache *im;
  TsGeoCalls *calls;
  unsigned short logging;

  /* More or less public. */
  char const *name;
  TsBBox bbox;
  TsBBox lvcs_bbox;
  TsBBox gdc_bbox;
  TsGeoInfo info;

  /* These are pointers to the image cache */
  double const *origin;
  float const *orientation;
  m3f *rotate;
};

typedef struct {
  int (*prepareTile) (TsGeoView *gv, TsTile *tile, int const *request_prio);
  void (*sync) (TsGeoView *gv);
  void (*clear) (TsGeoView *gv);
  void (*destroy) (TsGeoView *gv);
} TsGeoViewVTable;

struct TsGeoView {
  TsGeoPyramid *geo;
  TsGeoViewVTable const *vtable;
};

/* In TsGeoPyramid.c */
extern TsTile const *TsCacheOI_PropagateBBox (TsCache const *cache,
					      TsTile *tile);
extern void TsCacheOI_SetupRemap (TsCache *cache);

extern void TsGeoOIDEM_SetBBox (TsGeoPyramid *geo);

extern void TsGeoCorrectName (TsGeoPyramid *geo);

extern void TsGeoOIDEM_TileComputeBBox (TsGeoPyramid const *geo,
					TsTile *tile, 
					float min_z, float max_z);

extern void TsCacheOI_InitFinalBBox (TsCache const *cache, TsTile *tile);

extern TsTile const *TsCacheOI_PropagateBBox (TsCache const *cache,
					      TsTile *tile);

extern void TsGeoUpdateBBox (TsGeoPyramid *geo);



#endif /* _TsGeoPyramidI_h_ */
