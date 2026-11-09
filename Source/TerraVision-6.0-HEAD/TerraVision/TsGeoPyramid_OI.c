/* -*- tab-width: 8; -*-

  Revision History:
      $Id: TsGeoPyramid_OI.c,v 1.1.2.8 2002/10/31 05:26:41 reddy Exp $

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Trace.h"

#include <limits.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include <vrml2/quat.h>
#include <vrml2/vec3.h>

#include "TerraVisionInc.h"  /* needed for M_PI under Windows */
#include "TsGeoPyramid.h"
#include "TsGeoPyramidI.h"

/*
 * This file is the implementation of a TsGeoPyramid class that
 * represents an OI pyramid with no underlying DEM.  In this case, an
 * altitude above the ellipsoid/geoid is given and the imagery is
 * rendered on that surface.
 *
 * The user is given the option of also specifying other parameters to
 * describe whether or not the image has transparency, is viewable
 * from above or below, is lightable or is to be used as the sky
 * surface.
 */

static void TsGeoOI_Destroy (TsGeoPyramid *geo);
static int TsGeoOI_Connections (TsGeoPyramid *geo, TsmConnection *cons);
static TsGeoView * TsGeoOI_CreateView (TsGeoPyramid *geo);
static float TsGeoOI_Elevation (TsGeoPyramid const *geo,
				float u, float v);
static int TsGeoOI_AddCallback (TsGeoPyramid *geo, TsCacheCB which,
				TsCallbackF func, void *closure,
				void (*destroyf)(void*));
static int TsGeoOI_RemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
				   TsCallbackF func, void *closure);
static int TsGeoOI_URLs (TsGeoPyramid const *geo, char const **urls);
static void TsGeoOI_SetBBox (TsGeoPyramid *geo);
static void TsGeoOI_TileComputeBBox (TsGeoPyramid const *geo, TsTile *tile,
				     float min_z, float max_z);

static void TsGeoOI_InitLoad (TsGeoView *gv);
static void TsGeoOI_TileExtent (TsCache const *cache,
				void *arg, void *closure);

static void TsGeoOI_MakeOIAux (TsGeoPyramid const *geo, TsTile *tile,
			       float min_z, float max_z);
static void TsGeoOI_FreeOIAux (TsCache const *cache, void *arg, void *closure);

typedef struct {
  TsGeoPyramid geo;
  TsGeoView *baseView;
  float altitude;
} TsGeoPyramid_OI;

TsGeoPyramid *
TsGeoCreate_OI (char const *im_url,
		TsmConnectionParams im_params)
{
  static char const funcName[] = "TsGeoCreate_OI";

  static TsGeoCalls TsGeoCalls_OI = {
    TsGeoOI_Destroy,
    TsGeoOI_Connections,
    TsGeoOI_CreateView,
    TsGeoOI_Elevation,
    TsGeoOI_AddCallback,
    TsGeoOI_RemoveCallback,
    TsGeoOI_URLs
  };

  int connected;

  TsGeoPyramid_OI *geov = (TsGeoPyramid_OI *)
    malloc (sizeof (TsGeoPyramid_OI));
  TsGeoPyramid *geo = &geov->geo;

  TileSetTspec const *oiTspec;

  TSMTRACE ((1, funcName, "%s", im_url));
  TSMTRINC ((CACHE_MASK|1, 1));

  geo->name = NULL;
  geo->type = TsGeo_OI_DEM;
  geo->logging = FALSE;

  /* Create the individual caches. */
  geo->im = TsCacheCreate (im_url, im_params);
  geo->calls = &TsGeoCalls_OI;

  /* Clear it to start with */
  geov->baseView = NULL;

  /* Now connect them. */
  im_params = TsCacheConnectionParams (geo->im);
  tsmSetParam (im_params, TSM_PARAM_BLOCKING_READ, TRUE);
  tsmSetParam (im_params, TSM_PARAM_REQUEST_CHECK, TRUE);

  connected = TsCacheConnect (geo->im);

  /* Check for failure or interrupt */
  if (! connected || PR_Sleep (PR_INTERVAL_NO_WAIT) == PR_FAILURE) {
    TsGeoOI_Destroy (geo);
    TSMTRINC ((CACHE_MASK|1, -1));
    TSMTRACE ((CACHE_MASK|1, funcName, "FAIL"));
    return NULL;
  }

  /* Update the geoset name from the OI's tspec geoname entry */
  TsGeoCorrectName (geo);

  /* Set up ease-of-use references to the image cache */
  geo->origin = geo->im->lvcs_origin;
  geo->rotate = &geo->im->lvcs_rotate;
  geo->orientation = geo->im->orientation;

  /* The bbox of the image is in UV.  We need to convert this to the LVCS. */
  {
    int i, j, k;
    TsBBox const *uv_bbox = TsCacheBBox (geo->im);
    float const *uv_off[2];

    uv_off[0] = uv_bbox->min;
    uv_off[1] = uv_bbox->max;
    TsBBoxInit (&geo->lvcs_bbox);
    TsBBoxInit (&geo->gdc_bbox);
    for (i = 0; i < 2; ++i) {
      double u = uv_off[i][0];
      for (j = 0; j < 2; ++j) {
	double v = uv_off[j][1];
	for (k = 0; k < 2; ++k) {
	  double z = uv_off[k][2];
	  float l[3];
	  double gdc[3];
	  /* calculate the LVCS representation of the UV coordinate */
	  if (TsCacheUVtoLVCS (geo->im, u, v, z, l)) {
	    TsBBoxAddPoint (&geo->lvcs_bbox, l[0], l[1], l[2]);
	  } else {
	    TSMTRACE((1, funcName, "Corner %d,%d,%d: %g,%g,%g => LVCS FAILED",
		      i, j, k, u, v, z));
	  }
	  /* calculate the GDC representation of the UV coordinate */
	  if (TsCacheUVtoGDC (geo->im, u, v, z, gdc)) {
	    TsBBoxAddPoint (&geo->gdc_bbox, gdc[0], gdc[1], gdc[2]);
	  } else {
	    TSMTRACE((1, funcName, "Corner %d,%d,%d: %g,%g,%g => GDC FAILED",
		      i, j, k, u, v, z));
	  }
	}
      }
    }
    TsBBoxFinish (&geo->lvcs_bbox);
    TsBBoxFinish (&geo->gdc_bbox);
    geo->lvcs_bbox.coordType = TsCoordLVCS;
    geo->gdc_bbox.coordType = TsCoordGDC;
  }

  /* Initialize TsGeoInfo */
  oiTspec = TsCacheTspec (geo->im);
  geo->info.minLevel = oiTspec->minLevel;
  geo->info.maxLevel = oiTspec->maxLevel;
  geo->info.tileNumPixels[0] = oiTspec->tileTspec.numPixelsX;
  geo->info.tileNumPixels[1] = oiTspec->tileTspec.numPixelsY;
  geo->info.tileNumComps     = oiTspec->tileTspec.numComponentsPixel;
  geo->info.earthRadius[0]   = oiTspec->coordTspec.equatorialRadius;
  geo->info.earthRadius[1]   = oiTspec->coordTspec.polarRadius;

  geo->info.wholeEarth       = oiTspec->wrap_lon;
  geo->info.skySurface       = oiTspec->sky;
  geo->info.oneSided         = !oiTspec->twoSided;
  geo->info.viewAbove        = !oiTspec->fromBelow;
  geo->info.transparent      = FALSE;
  geo->info.lightable        = FALSE;

  /* The altitude is obtained from the tspec as well */
  geov->altitude = oiTspec->coordTspec.originAltitude;

  /* Callbacks specific to this GeoPyramid arrangement. */
  TsCacheAddCallback (geo->im, TILE_INIT, TsGeoOI_TileExtent, geov, NULL);
  TsCacheAddCallback (geo->im, TILE_DECACHE, TsGeoOI_FreeOIAux, NULL, NULL);

  TsCacheOI_SetupRemap (geo->im);

  /* Create default request lists */
  geov->baseView = TsGeoCreateView (geo);
  TsGeoOI_InitLoad (geov->baseView);

  /* Now, initialize dataset bounding box. */
  TsGeoOI_SetBBox (geo);

  TsGeoCreate_Common (geo);

  TSMTRINC ((CACHE_MASK|1, -1));
  TSMTRACE ((CACHE_MASK|1, funcName, "<= %p", geo));
  return (TsGeoPyramid *) geo;
}

static void
TsGeoOI_SetBBox (TsGeoPyramid *geo)
{
  /*static char const funcName[] = "TsGeoOI_SetBBox";*/
  TsGeoPyramid_OI *geov = (TsGeoPyramid_OI *) geo;
  TileSetTspec const *imTspec = TsCacheTspec (geo->im);
  TsTile *imTop = TsCacheGetTile (geo->im, 0, 0, imTspec->maxLevel);

  /* In order to get elevation range, we need the OI tile.
     This has already been requested by TsCacheDEM_InitLoad. */
  if (! TsCacheTilePrepared (imTop))
    TsCacheTileWait (geo->im, imTop);

  {
    float const *min = imTop->bbox.min, *max = imTop->bbox.max;
    TsBBox *uv_bbox = (TsBBox *) TsCacheBBox (geo->im);

    uv_bbox->min[2] = min[2];
    uv_bbox->max[2] = max[2];
    TsBBoxFinish (uv_bbox);

    TsGeoUpdateBBox (geo);
  }
}

static void
TsGeoOI_Destroy (TsGeoPyramid *geo)
{
  static char const funcName[] = "TsGeoOI_Destroy";
  TsGeoPyramid_OI *geov = (TsGeoPyramid_OI *) geo;
  TsCache *im = geo->im;

  TSMTRACE ((CACHE_MASK|1, funcName, "(%p)", geo));
  TSMTRINC ((CACHE_MASK|1, 1));

  /* Call superclass destructor. */
  TsGeoDestroy_Common (geo);

  if ( geov->baseView ) TsGeoViewDestroy (geov->baseView);

  /* Ensure that im doesn't depend on dem anymore. */
  if (im)  TsCacheRemoveCallbacks (im, TILE_CACHE);

  /* First disconnect them... */
  if (im)  TsCacheDisconnect (im);

  /* Now it is safe to destroy them */
  if (im)  TsCacheDestroy (im);

  if ( geo->name ) tsmMemFree ((void*) geo->name);

  free (geov);

  TSMTRINC ((CACHE_MASK|1, -1));
  TSMTRACE ((CACHE_MASK|1, funcName, "Done."));
}

static int
TsGeoOI_Connections (TsGeoPyramid *geo, TsmConnection *cons)
{
  /*static char const funcName[] = "TsGeoOI_Connections";*/
  TsmConnection con;
  int n = 0;
  if ((con = TsCacheConnection (geo->im))) 
    cons[n++] = con;
  return n;
}

static float
TsGeoOI_Elevation (TsGeoPyramid const *geo,
		   float u, float v)
{
  static char const funcName[] = "TsGeoOI_Elevation";
  TsGeoPyramid_OI *geov = (TsGeoPyramid_OI *) geo;
  double gdc[3];

  TsBBox const *uv_bbox = TsCacheBBox (geo->im);
  float const slop = 1;

  assert (uv_bbox->coordType == TsCoordUV);

  /* Remove slop and fail if outside bounds. */
  if (u    < uv_bbox->min[0]-slop || u > uv_bbox->max[0]+slop
      || v < uv_bbox->min[1]-slop || v > uv_bbox->max[1]+slop) {
    TSMTRACE ((CACHE_MASK|2, funcName, "%g,%g outside bbox [%g,%g -> %g,%g]",
	       u, v,
	       uv_bbox->min[0], uv_bbox->min[1],
	       uv_bbox->max[0], uv_bbox->max[1]));
    return TS_BAD_ELEVATION;
  }

  /* Convert to GDC and use that elevation. */
  if (! TsCacheUVtoGDC (geo->im, u, v, geov->altitude, gdc))
    return TS_BAD_ELEVATION;

  return gdc[2];
}

static int
TsGeoOI_AddCallback (TsGeoPyramid *geo, TsCacheCB which,
		     TsCallbackF func, void *closure,
		     void (*destroyf)(void*))
{
  return TsCacheAddCallback (geo->im, which, func, closure, destroyf);
}

static int
TsGeoOI_RemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
			TsCallbackF func, void *closure)
{
  return TsCacheRemoveCallback (geo->im, which, func, closure);
}

static int
TsGeoOI_URLs (TsGeoPyramid const *geo, char const **urls)
{
  urls[0] = geo->im->tspec->pathName;
  return 1;
}

typedef struct {
  TsGeoView super;

  tsmRequestList *oiReq;
} TsGeoView_OI;

static int TsGeoOI_PrepareTile (TsGeoView *gv,
				TsTile *tile, int const *request_prio);
static void TsGeoOI_SyncView (TsGeoView *geo);
static void TsGeoOI_ClearView (TsGeoView *geo);
static void TsGeoOI_DestroyView (TsGeoView *geo);

static TsGeoView *
TsGeoOI_CreateView (TsGeoPyramid *geo)
{
  static char const funcName[] = "TsGeoOI_CreateView";
  static TsGeoViewVTable TsGeoOI_viewTable = {
    TsGeoOI_PrepareTile,
    TsGeoOI_SyncView,
    TsGeoOI_ClearView,
    TsGeoOI_DestroyView
  };

  TsGeoView_OI *v = malloc (sizeof (*v));
  TsGeoView *super = &v->super;

  super->geo = geo;
  super->vtable = &TsGeoOI_viewTable;

  v->oiReq = TsCacheRequestList (geo->im);

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", v, geo));
  return &v->super;
}

/* Load a small subset of the OI tiles down to the level where >1 tile
   exists. */
static void
TsGeoOI_InitLoad (TsGeoView *gv)
{
  static char const funcName[] = "TsGeoOI_InitLoad";
  TsGeoView_OI *view = (TsGeoView_OI *) gv;
  tsmRequestList *req = view->oiReq;
  TsGeoPyramid *geo = gv->geo;
  TsCache *cache = geo->im;
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int level = tspec->maxLevel;
  int min_level = level;

  /* Find level with >1 tile */
  for ( ; ; --min_level) {
    SimpleTileSet const *tset = TsCacheTileSet (cache, min_level);
    if (tset->minTile[0] != tset->maxTile[0]
	|| tset->minTile[1] != tset->maxTile[1])
      break;
  }

  for ( ; level >= min_level; --level) {
    SimpleTileSet const *tset = TsCacheTileSet (cache, level);
    int x, y;

    TSMTRACE ((CACHE_MASK|2, funcName, "Level %d: %d-%d,%d-%d",
	       level, tset->minTile[0], tset->maxTile[0],
	       tset->minTile[1], tset->maxTile[1]));
    for (x = tset->minTile[0]; x <= tset->maxTile[0]; ++x)
      for (y = tset->minTile[1]; y <= tset->maxTile[1]; ++y)
	TsCacheRequest (cache, req, x, y, level, TSM_NOW);
  }
  tsmRequestSubmit (req);
}

static void
TsGeoOI_SyncView (TsGeoView *gv) {
  static char const funcName[] = "TsGeoOI_SyncView";
  TsGeoView_OI *view = (TsGeoView_OI *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));

  tsmRequestSubmit (view->oiReq);
}

static void
TsGeoOI_ClearView (TsGeoView *gv) {
  static char const funcName[] = "TsGeoOI_ClearView";
  TsGeoView_OI *view = (TsGeoView_OI *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));

  tsmRequestSubmit (view->oiReq);  tsmRequestSubmit (view->oiReq);
}

static void
TsGeoOI_DestroyView (TsGeoView *gv) {
  static char const funcName[] = "TsGeoOI_DestroyView";
  TsGeoView_OI *view = (TsGeoView_OI *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));
  tsmRequestDestroy (view->oiReq);
  free (view);
}

typedef struct {
  TsGeoTileAux geo;
  PRUint32 size;
  TsBBox uv_bbox;
} TsOITileAux;

/* Create the whole set of auxiliary data for an OI tile.
   This includes calculating the range of DEM tiles needed for support, an
   initial estimate of the bounding box, and then the base elevations and a
   good estimate of the bounding box if the supporting DEM tiles have been
   prepared.
 */

static void
TsGeoOI_MakeOIAux (TsGeoPyramid const *geo, TsTile *tile,
		   float min_z, float max_z)
{
  static char const funcName[] = "TsGeoOI_MakeOIAux";
  PRUint32 aux_size;
  TsOITileAux *aux;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res));
  TSMTRINC ((CACHE_MASK|2, 1));

  aux_size = sizeof (*aux);
  aux = (TsOITileAux *) malloc (aux_size);
  aux->size = aux_size;
  aux->uv_bbox = tile->bbox;
  aux->geo.coords = 0;
  aux->geo.npolys = 0;
  
  tile->aux = aux;
  TsCacheTileAddMem (tile, aux->size);

  /* Initial estimate of UV bbox */
  TsGeoOI_TileComputeBBox (geo, tile, min_z, max_z);

  TSMTRINC ((CACHE_MASK|2, -1));
}

static void
TsGeoOI_TileComputeBBox (TsGeoPyramid const *geo,
			 TsTile *tile, 
			 float min_z, float max_z)
{
  TsGeoOIDEM_TileComputeBBox (geo, tile, min_z, max_z);
}

static void
TsGeoOI_FreeOIAux (TsCache const *cache, void *arg, void *closure)
{
  /*static char const funcName[] = "TsGeoOI_FreeOIAux";*/
  TsTile *tile = arg;
  if (tile->aux) {
    TsOITileAux *aux = tile->aux;
    if (aux->geo.coords) free (aux->geo.coords);
    TsCacheTileAddMem (tile, -(int)aux->size);
    free (aux);
    tile->aux = 0;
  }
}

/* Compute an OI tile's auxiliary information after the tile is loaded.  This
   function is registered as a TILE_INIT callback of the OI TsCache.
*/

static void
TsGeoOI_TileExtent (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "TsGeoOI_TileExtent";
  TsGeoPyramid *geo = closure;
  TsTile *tile = arg;
  TileSetTspec const *tspec = TsCacheTspec( cache );
  float alt = 0.0;

  /* If the parent is prepared, then we grab our estimated bounds from it. */
  TsTile const *parent = TsCacheTileParent_const (tile);
  
  TsBBox const *uv_bbox = 0;

  if (parent && TsCacheTilePrepared (parent)) {
    TsOITileAux const *aux = parent->aux;
    uv_bbox = &aux->uv_bbox;
  } else {
    /* Otherwise, use the dataset's bounds. */
    uv_bbox = TsCacheBBox (geo->im);
  }

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] Z [%g-%g]",
	     tile->id.x, tile->id.y, tile->id.res, 
	     uv_bbox->min[2], uv_bbox->max[2]));

  /* This is an OI with no DEM - it floats at a constant altitude */
  if ( tspec )
    alt = tspec->coordTspec.originAltitude;

  TsGeoOI_MakeOIAux (geo, tile, alt, alt );
}


static void
TsGeoOI_TilePoints (TsGeoPyramid_OI *geov, TsTile *tile)
{
  static char const funcName[] = "TsGeoOI_TilePoints";
  TsOITileAux *aux = tile->aux;
  TsGeoPyramid const *geo = &geov->geo;
  
  int iy0, iy1, iy_spacing;
  float min_iyf, max_iyf;
  int ix0, ix1, ix_spacing;
  float min_ixf, max_ixf;

  /* Don't redo this... */
  if (aux->geo.coords)
    return;

  {
    TsCache const *imCache = geo->im;
    SimpleTileSet const * const tset = TsCacheTileSet (imCache, tile->id.res);
    
    TsBBox const *uv_bbox = &imCache->bbox;
    float const *ixiytouv = TsCacheTfm_ixiytouv (imCache, tile->id.res);
    float const *uvtoixiy = TsCacheTfm_uvtoixiy (imCache, tile->id.res);

    int npolys = 4;
    int tile_w = tset->tileTspec.numPixelsX;
    int tile_h = tset->tileTspec.numPixelsY;
    int tile_ix = tile_w * tile->id.x;
    int tile_iy = tile_h * tile->id.y;
    int ix = tile_ix;
    int iy = tile_iy;
    int ixe = tile_ix + tile->pixels[0];
    int iye = tile_iy + tile->pixels[1];
    int coords_w, coords_h, coords_len;

    /* The code below assumes that the TFM matrix is positive definite...
       Is that a problem? */
    float min_u = uv_bbox->min[0];
    float max_u = uv_bbox->max[0];
    float min_v = uv_bbox->min[1];
    float max_v = uv_bbox->max[1];

    ix0 = ix;
    iy0 = iy;
    ix_spacing = tile_w / npolys;
    iy_spacing = tile_h / npolys;

    min_ixf = TFM_IX (min_u, min_v, uvtoixiy);
    min_iyf = TFM_IY (min_u, min_v, uvtoixiy);
    max_ixf = TFM_IX (max_u, max_v, uvtoixiy);
    max_iyf = TFM_IY (max_u, max_v, uvtoixiy);

    if (min_ixf < tile_ix) min_ixf = tile_ix;
    if (max_ixf > ixe) max_ixf = ixe;
    if (min_iyf < tile_iy) min_iyf = tile_iy;
    if (max_iyf > iye) max_iyf = iye;

    TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] %.1f-%.1f, %.1f-%.1f",
	       tile->id.x, tile->id.y, tile->id.res,
	       min_ixf, max_ixf, min_iyf, max_iyf));

    /* Find bounds of ix */
    coords_w = 1;
    for ( ; ix < min_ixf; ix += ix_spacing)
      ix0 = ix;
    if (ix > min_ixf) ++coords_w;
    for ( ; ix < max_ixf; ix += ix_spacing, ++coords_w) {
      TSMTRACE ((CACHE_MASK|3, funcName, "ix %d [%d]", ix, coords_w));
    }
    ix1 = ix;

    coords_h = 1;
    for ( ; iy < min_iyf; iy += iy_spacing)
      iy0 = iy;
    if (iy > min_iyf) ++coords_h;
    for ( ; iy < max_iyf; iy += iy_spacing, ++coords_h) {
      TSMTRACE ((CACHE_MASK|3, funcName, "iy %d [%d]", iy, coords_h));
    }
    iy1 = iy;

    TSMTRACE ((CACHE_MASK|2, funcName,
	       "X %.1f-%.1f => W %d", min_ixf, max_ixf, coords_w));
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "Y %.1f-%.1f => H %d", min_iyf, max_iyf, coords_h));

    coords_len = coords_w * coords_h;
    TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] %dx%d = %d pts",
	       tile->id.x, tile->id.y, tile->id.res,
	       coords_w, coords_h, coords_len));

    aux->geo.coords_w = coords_w;
    aux->geo.coords_h = coords_h;
    aux->geo.npolys = npolys;

    {
      int size = (coords_len * sizeof (aux->geo.coords[0]));
      aux->geo.coords = malloc (size);

      TSMTRACE ((CACHE_MASK|2, funcName, "malloc [%p - %p] %d %dx%d",
		 aux->geo.coords, ((char*)aux->geo.coords)+size,
		 size, coords_w, coords_h));

      aux->size += size;
    }

    {
      int tile_ix = tile_w * tile->id.x;
      int tile_iy = tile_h * tile->id.y;
      int bix = tile_ix;
      int biy = tile_iy;
      TsBBox *bbox = (TsBBox *) &tile->bbox; /* Cast away const */
      float min_tx, max_tx, min_ty, max_ty;
      float alt = geov->altitude;

      TsCoord *coords = aux->geo.coords;
      PRUint16 *neighIndex = aux->geo.neighIndex;
      int cx, cy;

      int mid_x = npolys/2;
      int mid_y = npolys/2;

      int normDown = (geo->info.oneSided && !geo->info.viewAbove);

      /* Find starting index and midpoint */
      for ( ; biy+iy_spacing <= iy0; biy += iy_spacing, --mid_y);
      for ( ; bix+ix_spacing <= ix0; bix += ix_spacing, --mid_x);
      if (mid_x >= coords_w) mid_x = coords_w-1;
      if (mid_y >= coords_h) mid_y = coords_h-1;

      assert (bbox->coordType == TsCoordLVCS);
      TsCacheOI_InitFinalBBox (imCache, tile);

      min_tx = 0.5/(float)tile_w;
      min_ty = 0.5/(float)tile_h;
      max_tx = (tile->pixels[0]-0.5)/(float)tile_w;
      max_ty = (tile->pixels[1]-0.5)/(float)tile_h;

      for (iy = biy, cy = 0; iy <= iy1; iy += iy_spacing, ++cy) {
	float y = (iy < min_iyf ? min_iyf : (iy > max_iyf ? max_iyf : iy));
	for (ix = bix, cx = 0;
	     ix <= ix1;
	     ix += ix_spacing, ++coords, ++cx) {
	  float x = (ix < min_ixf ? min_ixf : (ix > max_ixf ? max_ixf : ix));
	  float u = TFM_U (x, y, ixiytouv);
	  float v = TFM_V (x, y, ixiytouv);
	  float tx, ty;
	  double gcc[3];
	  float *vtx = coords->vertex;

	  if (u < min_u) u = min_u; else if (u > max_u) u = max_u;
	  if (v < min_v) v = min_v; else if (v > max_v) v = max_v;
	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "[%3d]@%p pxl %g %g => %.1f,%.1f",
		     coords-aux->geo.coords, coords, x, y, u, v));

	  TsCacheUVtoGCC (imCache, u, v, alt, gcc);
	  TsCacheGCCtoLVCS (imCache, gcc[0], gcc[1], gcc[2], vtx);

	  /* Calculate the normal.
	     The GCC coordinate is a decent normal vector */
	  {
	    float d = sqrt (gcc[0]*gcc[0] + gcc[1]*gcc[1] + gcc[2]*gcc[2]);
	    if (normDown) d = -d;
	    coords->norm[0] = gcc[0]/d;
	    coords->norm[1] = gcc[1]/d;
	    coords->norm[2] = gcc[2]/d;
	  }

	  /* Update tile's LVCS bbox with transformed point */
	  TsBBoxAddPoint (bbox, vtx[0], vtx[1], vtx[2]);

	  /* These are wrt. the tile's image. */
	  tx = (x - tile_ix)/(float)tile_w;
	  if (tx < min_tx) tx = min_tx; else if (tx > max_tx) tx = max_tx;

	  ty = (y - tile_iy)/(float)tile_h;
	  if (ty < min_ty) ty = min_ty; else if (ty > max_ty) ty = max_ty;
	  
	  coords->texCoord[0] = tx;
	  coords->texCoord[1] = ty;

	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "[%3d] %.0f,%.0f,%.0f N %.2f,%.2f,%.2f T %.3f,%.3f",
		     coords-aux->geo.coords,
		     coords->vertex[0], coords->vertex[1], coords->vertex[2],
		     coords->norm[0], coords->norm[1], coords->norm[2],
		     coords->texCoord[0], coords->texCoord[1]));
	}
      }

      assert (coords == aux->geo.coords + coords_len);

      /* Initialize the neighIndex pointers. */
      neighIndex[TsCache_SW] = 0;
      neighIndex[TsCache_S]  = mid_x;
      neighIndex[TsCache_SE] = (coords_w-1);
      neighIndex[TsCache_W]  = coords_w*mid_y;
      neighIndex[TsCache_C]  = coords_w*mid_y + mid_x;
      neighIndex[TsCache_E]  = coords_w*mid_y + (coords_w-1);
      neighIndex[TsCache_NW] = coords_w*(coords_h-1);
      neighIndex[TsCache_N]  = coords_w*(coords_h-1) + mid_x;
      neighIndex[TsCache_NE] = coords_w*(coords_h-1) + (coords_w-1);

      TSMTRACE ((CACHE_MASK|2, funcName, "SW: %3d %3d %3d",
		 neighIndex[TsCache_SW], neighIndex[TsCache_S],
		 neighIndex[TsCache_SE]));
      TSMTRACE ((CACHE_MASK|2, funcName, " W: %3d %3d %3d",
		 neighIndex[TsCache_W], neighIndex[TsCache_C],
		 neighIndex[TsCache_E]));
      TSMTRACE ((CACHE_MASK|2, funcName, "NW: %3d %3d %3d",
		 neighIndex[TsCache_NW], neighIndex[TsCache_N],
		 neighIndex[TsCache_NE]));

      if (TSMTRACEP(CACHE_MASK|1)) {
	int i;
	for (i = 0; i <= TsCache_SE; ++i)
	  assert (neighIndex[i] < coords_len);
      }

      /* Redo the bbox. */
      TsBBoxFinish (bbox);
      /*TsCacheOI_PropagateBBox (imCache, tile);*/
    }

  }
}

static void
TsGeoOI_TileNormalRange (TsTile *tile)
{
  static char const funcName[] = "TsGeoOI_TileNormalRange";
  TsOITileAux *aux = tile->aux;
  v3f *avg = &aux->geo.normal;
  TsCoord const *coords = aux->geo.coords, *end;
  TsCoord const *coord;
  int w, h;
  float min_v;

  w = aux->geo.coords_w;
  h = aux->geo.coords_h;
  end = coords + w*h;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res));

  /* Compute the average normal */
  V(*avg)[0] = V(*avg)[1] = V(*avg)[2] = 0;
  for (coord = coords; coord < end; ++coord) {
    add_v3f ((v3f *)coord->norm, avg, avg);
  }
  normalize_v3f (avg);

  /* And find the angle by finding the minimal inner product of avg with
     any of the normals. */
  for (coord = coords, min_v = 1; coord < end; ++coord) {
    float v = inner_v3f (avg, (v3f *)coord->norm);
    if (v < min_v) min_v = v;
  }

  /* normalThreshold is the threshold below which v.n is considered to be
     backfaced. */
  if (min_v < 0)
    aux->geo.normalThreshold = -1;
  else
    aux->geo.normalThreshold = cos (acos (min_v) + M_PI/2);
  TSMTRACE ((CACHE_MASK|2, funcName, "Angle: %.1f  Thresh: %.2f (%.1f)",
	     acos (min_v) * 180/M_PI, aux->geo.normalThreshold,
	     (acos (min_v) + M_PI/2) * 180/M_PI));
}

static int 
TsGeoOI_PrepareTile (TsGeoView *gv,
		     TsTile *tile, int const *request_prio)
{
  static char const funcName[] = "TsGeoOI_PrepareTile";
  TsGeoView_OI *view = (TsGeoView_OI *) gv;
  TsGeoPyramid *geo = gv->geo;
  TsGeoPyramid_OI *geov = (TsGeoPyramid_OI *) geo;

  if (TsCacheTileResident (tile)) {
    TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d]",
	       tile, tile->id.x, tile->id.y, tile->id.res)); 
    TSMTRINC ((CACHE_MASK|2, 1));

    /* Check that we can do this transition ourselves. */
    if (! TsCacheTileStartTransition (tile)) {
      TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] can't start transition",
		 tile, tile->id.x, tile->id.y, tile->id.res));
      TSMTRINC ((CACHE_MASK|2, -1));
      return FALSE;
    }

    /* Calculate tile's points and range of normals */
    TsGeoOI_TilePoints (geov, tile);
    TsGeoOI_TileNormalRange (tile);

    /* Indicate that this tile is now in the PREPARED state. */
    TsCacheTileFinishTransition (funcName, tile, TS_PREPARED);

    /* Now ensure the consistency of the pyramid. */
    {
      TsTile const *top = TsCacheOI_PropagateBBox (geo->im, tile);
      if (top) {
	/* Update OI bbox if necessary */
	TsBBox const *top_bbox = &top->bbox;
	TsBBox const *lvcs_bbox = TsGeoBBoxLVCS (geo);

	assert (top_bbox->coordType == lvcs_bbox->coordType);
	if (top_bbox->min[2] < lvcs_bbox->min[2]
	    || top_bbox->max[2] > lvcs_bbox->max[2]) {
	  TSMTRACE ((CACHE_MASK|1, funcName, 
		     "WARNING: %p[%d %d %d] elev %g->%g > %g->%g",
		     top, top->id.x, top->id.y, top->id.res,
		     top_bbox->min[2], top_bbox->max[2],
		     lvcs_bbox->min[2], lvcs_bbox->max[2]));
	}
      }
    }
  
    TSMTRINC ((CACHE_MASK|2, -1));
    return TRUE;
  } else {
    /* Request the tile */
    if (request_prio)
      TsCacheRequestTile (funcName, view->oiReq, tile, *request_prio);
  }

  return FALSE;
}

