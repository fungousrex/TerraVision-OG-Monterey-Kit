/* -*- tab-width: 8; -*-

  Revision History:
      $Id: TsGeoPyramidInline.h,v 2.31.2.4 2002/10/31 05:26:40 reddy Exp $

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

#include "TsGeoPyramidI.h"

exinline char const *
TsGeoName (TsGeoPyramid const *geo)
{
  return geo->name;
}

exinline TsCache const *
TsGeoCache (TsGeoPyramid const *geo)
{
  return geo->im;
}

exinline void
TsGeoDestroy (TsGeoPyramid *geo)
{
  list_remove (geoList, geo);
  (*geo->calls->destroy) (geo);
}

exinline int
TsGeoConnections (TsGeoPyramid *geo, TsmConnection *cons)
{
  return (*geo->calls->connections) (geo, cons);
}

exinline TsGeoView *
TsGeoCreateView (TsGeoPyramid *geo)
{
  return (*geo->calls->create_view) (geo);
}

exinline int
TsGeoChildren (TsTile const *tile, TsTile **children)
{
  return TsCacheTileChildren (tile, children);
}

exinline TsTile *
TsGeoParent (TsTile const *tile)
{
  return TsCacheTileParent (tile);
}

exinline TsTile *
TsGeoNeighbour (TsTile const *tile, TsCacheNeighbour dir)
{
  return TsCacheTileNeighbour (tile, dir);
}

exinline int
TsGeoNeighbours (TsTile const *tile,
		 TsTile **neighbours)
{
  return TsCacheTileNeighbours (tile, neighbours);
}

exinline TsGeoTileAux const *
TsGeoTileGetAux (TsTile const *tile)
{
  TsGeoTileAux const *aux = (const struct TsGeoTileAux *)tile->aux;
  return aux;
}

exinline v3f const *
TsGeoTileNormal (TsTile const *tile)
{
  TsGeoTileAux const *aux = (const struct TsGeoTileAux *)tile->aux;
  return &aux->normal;
}

exinline float
TsGeoTileNormalThreshold (TsTile const *tile)
{
  TsGeoTileAux const *aux = (const struct TsGeoTileAux *)tile->aux;
  return aux->normalThreshold;
}

exinline TsBBox const *
TsGeoBBox (TsGeoPyramid const *geo)
{
  return &geo->bbox;
}

exinline TsBBox const *
TsGeoBBoxLVCS (TsGeoPyramid const *geo)
{
  return &geo->lvcs_bbox;
}

exinline TsBBox const *
TsGeoBBoxUV (TsGeoPyramid const *geo)
{
  return &geo->im->bbox;
}

exinline TsBBox const *
TsGeoBBoxGDC (TsGeoPyramid const *geo)
{
  return &geo->gdc_bbox;
}

exinline double const *
TsGeoOrigin (TsGeoPyramid const *geo)
{
  return geo->origin;
}

exinline float const *
TsGeoOrientation (TsGeoPyramid const *geo)
{
  return geo->orientation;
}

exinline void
TsGeoLVCStoGCC (TsGeoPyramid const *geo,
		float l0, float l1, float l2, double g[3])
{
  TsCacheLVCStoGCC (geo->im, l0, l1, l2, g);
}

exinline void
TsGeoGCCtoLVCS (TsGeoPyramid const *geo,
		double g0, double g1, double g2, float l[3])
{
  TsCacheGCCtoLVCS (geo->im, g0, g1, g2, l);
}

exinline int
TsGeoUVtoGCC (TsGeoPyramid const *geo, 
	      float u, float v, float z,
	      double gcc[3])
{
  return TsCacheUVtoGCC (geo->im, u, v, z, gcc);
}

exinline int
TsGeoGCCtoUV (TsGeoPyramid const *geo, 
	      double x, double y, double z,
	      float uv[3])
{
  return TsCacheGCCtoUV (geo->im, x, y, z, uv);
}

exinline void
TsGeoLVCStoUV (TsGeoPyramid const *geo, 
	       float x, float y, float z,
	       float uv[3])
{
  TsCacheLVCStoUV (geo->im, x, y, z, uv);
}

exinline void
TsGeoUVtoLVCS (TsGeoPyramid const *geo, 
	       float u, float v, float z,
	       float lvcs[3])
{
  TsCacheUVtoLVCS (geo->im, u, v, z, lvcs);
}

exinline float
TsGeoElevationUV (TsGeoPyramid const *geo, float u, float v)
{
  return (*geo->calls->elevation) (geo, u, v);
}

exinline float
TsGeoElevationLVCS (TsGeoPyramid const *geo, float x, float y, float z)
{
  float uv[3];
  TsGeoLVCStoUV (geo, x, y, z, uv);
  return TsGeoElevationUV (geo, uv[0], uv[1]);
}

exinline float
TsGeoElevation (TsGeoPyramid const *geo, double const pos[3])
{
  float uv[3];
  TsGeoGCCtoUV (geo, pos[0], pos[1], pos[2], uv);
  return TsGeoElevationUV (geo, uv[0], uv[1]);
}

exinline float
TsGeoAltitude (TsGeoPyramid const *geo, double const pos[3])
{
  float uv[3];
  TsGeoGCCtoUV (geo, pos[0], pos[1], pos[2], uv);
  return uv[2] - TsGeoElevationUV (geo, uv[0], uv[1]);
}

exinline float
TsGeoAltitudeLVCS (TsGeoPyramid const *geo, float const l[3])
{
  double gcc[3];
  TsGeoLVCStoGCC (geo, l[0], l[1], l[2], gcc);
  return TsGeoAltitude (geo, gcc);
}

exinline int
TsGeoAddCallback (TsGeoPyramid *geo, TsCacheCB which,
		  TsCallbackF func, void *closure,
		  void (*destroyf)(void*))
{
  return (*geo->calls->add_callback) (geo, which, func, closure, destroyf);
}

exinline int
TsGeoRemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
		     TsCallbackF func, void *closure)
{
  return (*geo->calls->remove_callback) (geo, which, func, closure);
}

exinline int
TsGeoURLs (TsGeoPyramid const *geo, char const **urls)
{
  return (*geo->calls->urls) (geo, urls);
}

exinline TsGeoInfo const *
TsGeoGetInfo (TsGeoPyramid const *geo)
{
  return &geo->info;
}

exinline int
TsGeoTileValid (TsGeoPyramid const *geo, int x, int y, int res)
{
  return TsCacheTileValid (geo->im, x, y, res);
}

exinline TsTile *
TsGeoTile (TsGeoPyramid const *geo, int x, int y, int res)
{
  if (TsGeoTileValid (geo, x, y, res))
    return TsCacheGetTile (geo->im, x, y, res);
  return 0;
}

exinline float
TsGeoResolution (TsGeoPyramid const *geo)
{
  TileSetTspec const *tspec = TsCacheTspec( geo->im );
  float res = tspec->minResolution;
  if ( tspec->projectionTspec.projectionType == tspLatLongProjection ) 
    res *= 100000;
  return res;
}

exinline TsGeoPyramid *
TsGeoViewPyramid (TsGeoView *gv)
{
  return gv->geo;
}

exinline int
TsGeoViewTilePrepared (TsGeoView *gv, TsTile *tile, int const *request_prio)
{
  if (TsCacheTilePrepared (tile))
    return TRUE;
  
  return (*gv->vtable->prepareTile) (gv, tile, request_prio);
}

exinline void
TsGeoViewSync (TsGeoView *gv)
{
  (*gv->vtable->sync) (gv);
}

exinline void
TsGeoViewClear (TsGeoView *gv)
{
   (*gv->vtable->clear) (gv);
}

exinline void
TsGeoViewDestroy (TsGeoView *gv)
{
   (*gv->vtable->destroy) (gv);
}
