/* -*- tab-width: 2; -*-

  Name: ThreeDDraw.c
 
  Description: Drawing primitives for a tvBase instance.
 
  Revision History:
      $Id: ThreeDDraw.c,v 2.23.2.7 2002/10/31 05:26:36 reddy Exp $

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

#include "TerraVisionInc.h"

#include "ThreeDCoreI.h"
#include "Texture.h"

#include <math.h>
#include <assert.h>

#if !( defined(WIN32) && defined(MSVC) )
# include <values.h>
#endif

/* RCS INFO */
static char const ThreeDDraw_rcsid[] = "$Id: ThreeDDraw.c,v 2.23.2.7 2002/10/31 05:26:36 reddy Exp $";

/*
 * The code below represents a system for caching RenderPrimitives based on
 * matching OI and DEM pointers and the tile neighbourhood signature.
 *
 * The definitions control whether and how the cache is managed:
 *
 * CACHE_RP: If true, then we use the cache, otherwise no caching.
 * CACHE_RENDER: If true, then we generate display lists for RenderPrimitives
 *   and reuse them as appropriate.  (The big win!?)
 */

#define CACHE_RP (1)

typedef struct CachedRenderPrimitive {
  struct CachedRenderPrimitive *  next;
  struct CachedRenderPrimitive ** loc;

  TsTile const * tile;

  PRInt32 use_count;

  RenderPrimitive prim;
} CachedRenderPrimitive;

/* Given a RenderPrimitive, return the associated CachedRenderPrimitive. */
#define RPtoCACHE(rp) ((CachedRenderPrimitive *) \
		       (((char *)rp) - offsetof(CachedRenderPrimitive, prim)))

/* Multiplicative hash function. */
#define A (2654435769U)
#define HASH(k) (A*((PRUint32)k))

static CachedRenderPrimitive **
findCache (TsTile const *tile, unsigned int sig,
	   CachedRenderPrimitive **cache, unsigned int size)
{
  static char const * funcName = "findCache";
  unsigned long idx;
  CachedRenderPrimitive **loc;
  unsigned long hashOI  = HASH (tile);

  /* Compute hash value. */
  idx  = hashOI << 2;
  idx ^= sig;
  idx %= size;
  
  /* Search along linked list. */
  loc = &cache[idx];
  TSMTRACE ((THREED_MASK|4, funcName,
	     "cache[%d] OI:[%d %d %d] sig %02x", 
	     idx, tile->id.x, tile->id.y, tile->id.res, sig));
  while (*loc) {
    CachedRenderPrimitive *p = *loc;
    TSMTRACE ((THREED_MASK|4, funcName,
	       "cache[%d] loc %p next %p", idx, loc, p->next));
    if (p->tile == tile && p->prim.sig == sig)
      break;
    loc = &p->next;
  }

  TSMTRACE ((THREED_MASK|4, funcName, "cache[%d] <= loc %p %s", 
	     idx, loc, (*loc ? "Found" : "Empty")));
  return loc;
}

struct RenderPrimitiveCache {
  CachedRenderPrimitive **cache;
  int size;
  int count;
  CachedRenderPrimitive *reuse;
  PRLock *lock;
};

RenderPrimitiveCache *
RenderPrimitiveCacheCreate (void)
{
  RenderPrimitiveCache *cache = calloc (1, sizeof (RenderPrimitiveCache));
  cache->lock = PR_NewLock ();
  return cache;
}

static RenderPrimitive *
findRenderPrimitive (RenderPrimitiveCache *rp_cache,
		     TsTile const *tile, unsigned int sig)
{
  static char const * funcName = "findRenderPrimitive";

  CachedRenderPrimitive *crp;
  CachedRenderPrimitive **loc;

  PR_Lock (rp_cache->lock);

  if (CACHE_RP) {
    /* Ensure that we have a cache and that it is sufficiently large. */
    if (! rp_cache->size) {
      rp_cache->size = 517;
      rp_cache->cache = (CachedRenderPrimitive **)
	calloc (rp_cache->size, sizeof (CachedRenderPrimitive *));
      TSMTRACE ((THREED_MASK|1, funcName, "Create RP cache %d", rp_cache->size));
    } else if (2*rp_cache->count >= rp_cache->size) {
      int new_size = 2*rp_cache->size+17;
      CachedRenderPrimitive **new_cache;
      int i;
      TSMTRACE ((THREED_MASK|1, funcName, "!*! Expand RP cache %d => %d !*!",
		 rp_cache->size, new_size));
      new_cache = (CachedRenderPrimitive **)
	calloc (new_size, sizeof (CachedRenderPrimitive *));
      for (i = 0; i < rp_cache->size; ++i) {
	CachedRenderPrimitive *p;
	for (p = rp_cache->cache[i]; p; p = p->next) {
	  CachedRenderPrimitive **loc =
	    findCache (p->tile, p->prim.sig, new_cache, new_size);
	  *loc = p;
	}
      }
      free (rp_cache->cache);
      rp_cache->cache = new_cache;
      rp_cache->size = new_size;
    }
  
    /* Search for it in the cache. */
    loc = findCache (tile, sig, rp_cache->cache, rp_cache->size);
    if (*loc) {
      crp = *loc;
      TSMTRACE ((THREED_MASK|2, funcName,
		 "([%d %d %d],%02x) Match prim %p",
		 tile->id.x, tile->id.y, tile->id.res, 
		 sig, &crp->prim));
      PR_AtomicIncrement (&crp->use_count);
      RPRTR ((" +: [%d] prim %p tile %p[%d %d %d]\n", 
	      crp->use_count, &crp->prim, crp->tile->cache,
	      crp->tile->id.x, crp->tile->id.y, crp->tile->id.res));
      PR_Unlock (rp_cache->lock);
      return &crp->prim;
    }
  }

  /* Make a new one and add it to the cache. */
  if (rp_cache->reuse) {
    crp = rp_cache->reuse;
    rp_cache->reuse = crp->next;
    TSMTRACE ((THREED_MASK|3, funcName, "reuse  %p loc %p", crp, loc));
  } else {
    crp = (CachedRenderPrimitive *) malloc (sizeof (CachedRenderPrimitive));
    TSMTRACE ((THREED_MASK|3, funcName, "malloc %p loc %p", crp, loc));
  }

  if (CACHE_RP) {
    /* Link into cache list.  <loc> points to the previous next link. */
    crp->loc = loc;
    *loc = crp;
    crp->next = 0;
  }

  /* Store cache-relevant info and signal clear cPrims. */
#if 0
  printf ("D %p[%d %d %d] sig %d\n",
	  tile, tile->id.x, tile->id.y, tile->id.res, sig);
#endif

  crp->tile = tile;
  crp->use_count = 1;
  crp->prim.sig = sig;
  crp->prim.disp_list = 0;
  crp->prim.geometry = 0;
  TSMTRACE ((THREED_MASK|2, funcName,
	     "[%d %d %d] %02x: Init prim %p",
	     tile->id.x, tile->id.y, tile->id.res, sig, &crp->prim));

  RPRTR ((" +: [%d] prim %p tile %p[%d %d %d]\n", 
	  crp->use_count, &crp->prim, crp->tile->cache,
	  crp->tile->id.x, crp->tile->id.y, crp->tile->id.res));
  PR_Unlock (rp_cache->lock);

  return &crp->prim;
}

static void
decacheRenderPrimitive (RenderPrimitiveCache *rp_cache, RenderPrimitive *rp)
{
  static const char *funcName = "decacheRenderPrimitive";
  CachedRenderPrimitive *crp = RPtoCACHE (rp);
  
  TSMTRACE ((THREED_MASK|2, funcName, "PRIM [%d %d %d] %02x: decache %d %p",
	     crp->tile->id.x, crp->tile->id.y, crp->tile->id.res,
	     rp->sig, rp->disp_list, rp->geometry));

#if 0
  printf ("U %p[%d %d %d] sig %d\n",
	  crp->tile, crp->tile->id.x, crp->tile->id.y, crp->tile->id.res,
	  rp->sig);
#endif

  if (CACHE_RP && crp->loc) {
    CachedRenderPrimitive *next = crp->next;

    /* Link around it on hash list. */
    if (next) next->loc = crp->loc;
    if (crp->loc) *crp->loc = next;
    crp->loc = 0;
  }

  if (rp->coords && rp->sig != 0) {
    free ((void *) rp->coords);
    rp->coords = 0;
  }
  if (rp->geometry) {
    free ((void *) rp->geometry);
    rp->geometry = 0;
  }

  /* Allow reuse of display list. */
  if (rp->disp_list) {
    gltDeleteList (rp->disp_list);
    rp->disp_list = 0;
  }
}

static void
useRenderPrimitive (RenderPrimitiveCache *rp_cache, RenderPrimitive *rp)
{
  static const char *funcName = "useRenderPrimitive";
  CachedRenderPrimitive *crp = RPtoCACHE (rp);

  PR_AtomicIncrement (&crp->use_count);
  RPRTR ((" +: [%d] prim %p tile %p[%d %d %d]\n", 
	  crp->use_count, rp, crp->tile->cache,
	  crp->tile->id.x, crp->tile->id.y, crp->tile->id.res));
  TSMTRACE ((THREED_MASK|2, funcName, "%p count %d", rp, crp->use_count));
}

static void
releaseRenderPrimitive (RenderPrimitiveCache *rp_cache, RenderPrimitive *rp)
{
  static const char *funcName = "releaseRenderPrimitive";
  CachedRenderPrimitive *crp = RPtoCACHE (rp);

  if (PR_AtomicDecrement (&crp->use_count) == 0) {
    CachedRenderPrimitive *reuse = rp_cache->reuse;

    TSMTRACE ((THREED_MASK|2, funcName, "delete %p", crp));
    RPRTR ((" -: [%d] prim %p tile %p[%d %d %d]\n", 
	    crp->use_count, rp, crp->tile->cache,
	    crp->tile->id.x, crp->tile->id.y, crp->tile->id.res));
    PR_Lock (rp_cache->lock);
    decacheRenderPrimitive (rp_cache, rp);

    /* Add it to reuse list. */
    rp_cache->reuse = crp;
    crp->next = reuse;
    PR_Unlock (rp_cache->lock);
  } else {
    RPRTR ((" -: [%d] prim %p tile %p[%d %d %d]\n", 
	    crp->use_count, rp, crp->tile->cache,
	    crp->tile->id.x, crp->tile->id.y, crp->tile->id.res));
  }
  TSMTRACE ((THREED_MASK|2, funcName, "%p count %d", rp, crp->use_count));
}

/* obsoleted function?  --RLB */
void
tvBaseClearCache (tvBase *threeD)
{
  RenderPrimitiveCache *rp_cache = threeD->rp_cache;

  int i;
  PR_Lock (rp_cache->lock);
  for (i = 0; i < rp_cache->size; ++i) {
    CachedRenderPrimitive *p, *next;
    for (p = rp_cache->cache[i]; p; p = next) {
      next = p->next;
      decacheRenderPrimitive (rp_cache, &p->prim);
    }
  }
  PR_Unlock (rp_cache->lock);
}

static TileGeometry const *
TileCreateGeometry (TsGeoPyramid const *geo,
		    TsTile const *tile)
{
  static char const funcName[] = "TsGeoOIDEM_CreateGeometry";
  TsGeoTileAux const *geoAux = TsGeoTileGetAux (tile);

  int coords_w = geoAux->coords_w;
  int coords_h = geoAux->coords_h;
  int len_coords = coords_w * coords_h;
  int len_triStrips = coords_h-1;
  int len_coordIndex = 2*coords_w * len_triStrips;
  PRUint16 *coordIndex, *triStrips;
  int j;

  PRUint32 geom_size = sizeof (TileGeometry)
    + len_coordIndex * sizeof (*coordIndex)
    + len_triStrips * sizeof (*triStrips);
  TileGeometry *geometry = (TileGeometry *) malloc (geom_size);

  /* Add this size to the size of the aux. */
  TsCacheTileAddMem ((TsTile *) tile, geom_size);

  geometry->len_coords = len_coords;
  geometry->len_coordIndex = len_coordIndex;
  geometry->coordIndex = coordIndex = (PRUint16 *) (geometry+1);
  geometry->len_triStrips = len_triStrips;
  geometry->triStrips = triStrips = (PRUint16 *) (coordIndex + len_coordIndex);

  geometry->neighIndex = geoAux->neighIndex;

  TSMTRACE ((THREED_MASK|2, funcName, "[%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res));

  /* Initialize the sequence for the tri-strip. */
  for (j = 0; j < coords_h-1; ++j) {
    PRUint16 *coordIndex0 = coordIndex;
    int base = j*coords_w;
    int base_end = base + coords_w;
    for ( ; base < base_end; ++base) {
      TSMTRACE ((THREED_MASK|3, funcName, "coordIndex[%d] = %d, %d",
		 coordIndex-geometry->coordIndex, base + coords_w, base));
      *coordIndex++ = base + coords_w;
      *coordIndex++ = base;
    }
    TSMTRACE ((THREED_MASK|3, funcName, "triStrips[%d] = %d",
	       triStrips-geometry->triStrips, coordIndex-coordIndex0));
    *triStrips++ = coordIndex-coordIndex0;
  }
  assert (coordIndex == geometry->coordIndex + len_coordIndex);
  assert (triStrips == geometry->triStrips + len_triStrips);

  return geometry;
}

TileGeometry const *
GetGeometry (TsGeoPyramid const *geo, TsTile const *tile)
{
  if (! TsCacheTilePrepared (tile))
    return NULL;

  return TileCreateGeometry (geo, tile);
}

static void
TileFixSeam (TsGeoPyramid const *geo,
	     TsTile const *tile,
	     TsCacheNeighbour dir,
	     TsTile const *up,
	     TsCoord *coords)
{
  static char const funcName[] = "TileFixSeam";
  TsGeoTileAux const *aux = tile->aux;
  int span = aux->coords_w;

  TsGeoTileAux const *up_aux = up->aux;
  TsCoord const *up_coords = up_aux->coords;
  int up_span = up_aux->coords_w;

  int i0, i1, j0, j1, di, dj;
  int x_low, y_low;

  TSMTRACE ((THREED_MASK|2, funcName, "[%d %d %d] vs [%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res,
	     up->id.x, up->id.y, up->id.res));

  if (aux->npolys != up_aux->npolys) {
    TSMTRACE ((THREED_MASK|2, funcName,
	       "Cannot fix seam %d polys => %d polys",
	       aux->npolys, up_aux->npolys));
    return;
  }

  /* These cases handle the single point adjustments necessary for a
     change of LOD across a diagonal. */
  switch (dir) {
  case TsCache_SW:
  case TsCache_SE:
  case TsCache_NW:
  case TsCache_NE:
    i0 = aux->neighIndex[dir];
    j0 = up_aux->neighIndex[TsCacheNeighbour_reverse[dir]];
    TSMTRACE ((THREED_MASK|2, funcName,
	       "Fix %s [%d] %.0f,%.0f,%.0f -> %s [%d] %.0f,%.0f,%.0f",
	       TsCacheNeighbour_name[dir],
	       i0, coords[i0].vertex[0], coords[i0].vertex[1], coords[i0].vertex[2],
	       TsCacheNeighbour_name[TsCacheNeighbour_reverse[dir]],
	       j0, up_coords[j0].vertex[0], up_coords[j0].vertex[1], up_coords[j0].vertex[2]));
    coords[i0].vertex[0] = up_coords[j0].vertex[0];
    coords[i0].vertex[1] = up_coords[j0].vertex[1];
    coords[i0].vertex[2] = up_coords[j0].vertex[2];
    return;
  default:
    break;
  }

  /* These cases handle the more common adjustment along a side. */
  switch (dir) {
  case TsCache_W:
    y_low = ! (tile->id.y%2);
    i0 = aux->neighIndex[TsCache_SW];
    i1 = aux->neighIndex[TsCache_NW];
    di = span;
    j0 = up_aux->neighIndex[y_low ? TsCache_SE : TsCache_E];
    j1 = up_aux->neighIndex[y_low ? TsCache_E : TsCache_NE];
    dj = up_span;
    break;
  case TsCache_E:
    y_low = ! (tile->id.y%2);
    i0 = aux->neighIndex[TsCache_SE];
    i1 = aux->neighIndex[TsCache_NE];
    di = span;
    j0 = up_aux->neighIndex[y_low ? TsCache_SW : TsCache_W];
    j1 = up_aux->neighIndex[y_low ? TsCache_W : TsCache_NW];
    dj = up_span;
    break;
  case TsCache_N:
    x_low = ! (tile->id.x%2);
    i0 = aux->neighIndex[TsCache_NW];
    i1 = aux->neighIndex[TsCache_NE];
    di = 1;
    j0 = up_aux->neighIndex[x_low ? TsCache_SW : TsCache_S];
    j1 = up_aux->neighIndex[x_low ? TsCache_S : TsCache_SE];
    dj = 1;
    break;
  case TsCache_S:
    x_low = ! (tile->id.x%2);
    i0 = aux->neighIndex[TsCache_SW];
    i1 = aux->neighIndex[TsCache_SE];
    di = 1;
    j0 = up_aux->neighIndex[x_low ? TsCache_NW : TsCache_N];
    j1 = up_aux->neighIndex[x_low ? TsCache_N : TsCache_NE];
    dj = 1;
    break;
  default:
    /* This is an error condition. */
    abort ();
  }

  TSMTRACE ((THREED_MASK|2, funcName, "Fix %s [%d-%d by %d] -> [%d-%d by %d]",
	     TsCacheNeighbour_name[dir], i0, i1, di, j0, j1, dj));

  {
    TsCoord *c = &coords[i0];
    float *v = c->vertex;
    TsCoord const *up_c = &up_coords[j0];
    float const *up_v = up_c->vertex;
    for ( ; i0 < i1; i0 += di, c += di, v = c->vertex) {
      float const *up_v0 = up_c->vertex;
      TSMTRACE ((THREED_MASK|2, funcName,
		 "Fix [%d]@%p %.0f,%.0f,%.0f -> [%d]@%p %.0f,%.0f,%.0f",
		 i0, v, v[0], v[1], v[2],
		 j0, up_v, up_v[0], up_v[1], up_v[2]));
      v[0] = up_v[0]; v[1] = up_v[1]; v[2] = up_v[2];
      i0 += di; c += di; v = c->vertex;
      j0 += dj; up_c += dj; up_v = up_c->vertex;
      if (i0 >= i1) break;
      TSMTRACE ((THREED_MASK|2, funcName,
		 "Fix [%d]@%p %.0f,%.0f,%.0f -> [%d]@%p %.0f,%.0f,%.0f",
		 i0, v, v[0], v[1], v[2],
		 j0, up_v, (up_v0[0]+up_v[0])/2,
		 (up_v0[1]+up_v[1])/2,
		 (up_v0[2]+up_v[2])/2));
      v[0] = (up_v0[0]+up_v[0])/2;
      v[1] = (up_v0[1]+up_v[1])/2;
      v[2] = (up_v0[2]+up_v[2])/2;
    }

    /* Protect against overflow. */
    if (i0 > i1) { i0 = i1; c = &coords[i0]; v = c->vertex; }
    if (j0 > j1) { j0 = j1; up_c = &up_coords[j0]; up_v = up_c->vertex; }
    TSMTRACE ((THREED_MASK|2, funcName,
	       "Fix [%d]@%p %.0f,%.0f,%.0f -> [%d]@%p %.0f,%.0f,%.0f",
	       i0, v, v[0], v[1], v[2],
	       j0, up_v, up_v[0], up_v[1], up_v[2]));
    v[0] = up_v[0]; v[1] = up_v[1]; v[2] = up_v[2];
  }
}

static int
SeamNeedsFixing (TsGeoPyramid *geo, char const *dir,
		 int tx, int ty, int res, eqhash_t in_list,
		 TsTile const **adjacent)
{
  static const char funcName[] = "SeamNeedsFixing";
  TsTile const *neigh = TsGeoTile (geo, tx, ty, res);
  TsTile const *fix = 0;
  TSMTRACE ((THREED_MASK|3, funcName, "%s [%d %d %d] in_list? %s",
	     dir, tx, ty, res,
	     (neigh && TileOnVisibleList (in_list, neigh)) ? "Y" : "N"));
  if (neigh && ! TileOnVisibleList (in_list, neigh)) {
    TsTile const *up = TsGeoParent (neigh);
    TSMTRACE ((THREED_MASK|3, funcName, "- up [%d %d %d] in_list? %s",
	       up->id.x, up->id.y, up->id.res,
	       (up && TileOnVisibleList (in_list, up)) ? "Y" : "N"));
    if (TileOnVisibleList (in_list, up))
      fix = up;
  }

  TSMTRACE ((THREED_MASK|3, funcName, "- fix %p", fix));
  *adjacent = fix;
  return !!fix;
}

typedef enum {
  SIG_NONE = 0,
  SIG_W = (1<<0), SIG_E = (1<<1),
  SIG_S = (1<<2), SIG_N = (1<<3),
  SIG_DIAG = (1<<4)
} SeamFlags;

/*
 * Synopsis: void CreateRenderPrimitive(VisibleDraw *vis,
 *                                      VisibleDrawList *list,
 *                                      VisibleDrawGeoList *geo_list,
 *                                      RenderPrimitiveCache *rp_cache)
 *	
 * Description: Create a renderPrimitive, which is used for rendering tiles.
 *		This actually accesses the renderPrimitive cache in such a
 *		way that these primitives are reused if at all possible.
 *
 * Externals: None.
 *
 * Returns: Nothing.
 *
 * Author: Stephen Lau and Lee Iverson
 * 
 * Date: June 1995, October 1997
 *
 */

void
CreateRenderPrimitive (VisibleDraw *vis,
		       VisibleDrawList *list,
		       VisibleDrawGeoList *geo_list,
		       RenderPrimitiveCache *rp_cache)
{
  static const char funcName[] = "CreateRenderPrimitives";

  TsTile const *tile = vis->tile;
  TsGeoTileAux const *aux = tile->aux;
  int tx = tile->id.x, ty = tile->id.y, res = tile->id.res;
  RenderPrimitive *prim = vis->prim;
  eqhash_t in_list = list->in_list;
  TsGeoPyramid *geo = geo_list->geo;

  SeamFlags sig = SIG_NONE;
  TsCacheNeighbour diag_dir;
  TileGeometry const *geom;
  TsTile const *adjacent_parent[5];
  int i;

  TSMTRACE ((THREED_MASK|2, funcName, "[%d %d %d] => %p",
	     tx, ty, res, vis));
  TSMTRINC ((THREED_MASK|2, 1));

  /* Resolve seam references. */
  if (TsGeoParent (tile)) {
    if (tx%2 == 0) {
      if (SeamNeedsFixing (geo, " W", tx-1, ty, res, in_list, 
			   &adjacent_parent[0]))
	sig |= SIG_W;
    } else {
      if (SeamNeedsFixing (geo, " E", tx+1, ty, res, in_list, 
			   &adjacent_parent[1]))
	sig |= SIG_E;
    }
    if (ty%2 == 0) {
      if (SeamNeedsFixing (geo, "S ", tx, ty-1, res, in_list, 
			   &adjacent_parent[2]))
	sig |= SIG_S;
    } else {
      if (SeamNeedsFixing (geo, "N ", tx, ty+1, res, in_list, 
			   &adjacent_parent[3]))
	sig |= SIG_N;
    }

    /* Only if none of the sides need fixing might we need a diagonal
       adjustment. */
    if (! sig) {
      if (tx%2 == 0) {
	if (ty%2 == 0) {
	  if (SeamNeedsFixing (geo, "SW", tx-1, ty-1, res, in_list, 
			       &adjacent_parent[4]))
	    sig |= SIG_DIAG, diag_dir = TsCache_SW;
	} else {
	  if (SeamNeedsFixing (geo, "NW", tx-1, ty+1, res, in_list, 
			       &adjacent_parent[4]))
	    sig |= SIG_DIAG, diag_dir = TsCache_NW;
	}
      } else {
	if (ty%2 == 0) {
	  if (SeamNeedsFixing (geo, "SE", tx+1, ty-1, res, in_list, 
			       &adjacent_parent[4]))
	    sig |= SIG_DIAG, diag_dir = TsCache_SE;
	} else {
	  if (SeamNeedsFixing (geo, "NE", tx+1, ty+1, res, in_list, 
			       &adjacent_parent[4]))
	    sig |= SIG_DIAG, diag_dir = TsCache_NE;
	}
      }
    }

    TSMTRACE ((THREED_MASK|2, funcName,
	       "[%d %d %d] W[%c] E[%c] S[%c] N[%c]", tx, ty, res, 
	       (sig&SIG_W) ? '^' : ' ',
	       (sig&SIG_E) ? '^' : ' ',
	       (sig&SIG_S) ? '^' : ' ',
	       (sig&SIG_N) ? '^' : ' '));
  }

  vis->prim = prim = findRenderPrimitive (rp_cache, tile, sig);
  RPRTR (("CR: vis[%d %d %d] %p->prim = %p->geometry = %p\n",
	  tile->id.x, tile->id.y, tile->id.res, 
	  vis, prim, prim->geometry));

  /* This will be true if we are reusing a currently valid RP */
  if (prim->geometry) {
    TSMTRACE ((THREED_MASK|2, funcName, "PRIM [%d %d %d] %02x: reuse",
	       tx, ty, res, prim->sig));
    TSMTRINC ((THREED_MASK|2, -1));
    return;
  }

  /* Initialize the geometry... */
  prim->geometry = geom = TileCreateGeometry (geo, tile);
  RPRTR (("CR: vis[%d %d %d] %p->prim = %p->geometry = %p\n",
	  tile->id.x, tile->id.y, tile->id.res, 
	  vis, prim, prim->geometry));

  /* And the coords (this is dependent on the need for seam correction) */
  if (sig == 0) {
    prim->coords = aux->coords;
    TSMTRACE ((THREED_MASK|2, funcName, "PRIM [%d %d %d] %02x: reset %d",
	       tx, ty, res, prim->sig, prim->disp_list));
  } else {
    int len_coords = geom->len_coords;
    TsCoord *coords0 = malloc (len_coords * sizeof (coords0[0]));
    TsCoord *coords;

    TSMTRACE ((THREED_MASK|2, funcName, "[%d %d %d] coords %p[%d]", 
	       tile->id.x, tile->id.y, tile->id.res,
	       coords0, len_coords));

    /* Override coords. */
    memcpy (coords0, aux->coords, len_coords * sizeof(coords0[0]));
    prim->coords = coords0;

    TSMTRACE ((THREED_MASK|2, funcName,
	       "PRIM [%d %d %d] %02x: recalc %d %p",
	       tx, ty, res, prim->sig, prim->disp_list, coords0));

    /* Fix up seams if we need to. */
    if (sig) {
      if (sig & SIG_W) {
	TsTile const *up = adjacent_parent[0];
	TSMTRACE ((THREED_MASK|2, funcName, "Fix seam W -> [%d %d %d]",
		   up->id.x, up->id.y, up->id.res));
	TileFixSeam (geo, tile, TsCache_W, up, coords0);
      } else if (sig & SIG_E) {
	TsTile const *up = adjacent_parent[1];
	TSMTRACE ((THREED_MASK|2, funcName, "Fix seam E -> [%d %d %d]",
		   up->id.x, up->id.y, up->id.res));
	TileFixSeam (geo, tile, TsCache_E, up, coords0);
      }
      if (sig & SIG_S) {
	TsTile const *up = adjacent_parent[2];
	TSMTRACE ((THREED_MASK|2, funcName, "Fix seam S -> [%d %d %d]",
		   up->id.x, up->id.y, up->id.res));
	TileFixSeam (geo, tile, TsCache_S, up, coords0);
      } else if (sig & SIG_N) {
	TsTile const *up = adjacent_parent[3];
	TSMTRACE ((THREED_MASK|2, funcName, "Fix seam N -> [%d %d %d]",
		   up->id.x, up->id.y, up->id.res));
	TileFixSeam (geo, tile, TsCache_N, up, coords0);
      }
      if (sig & SIG_DIAG) {
	TsTile const *up = adjacent_parent[4];
	TSMTRACE ((THREED_MASK|2, funcName, "Fix seam diag -> [%d %d %d]",
		   up->id.x, up->id.y, up->id.res));
	TileFixSeam (geo, tile, diag_dir, up, coords0);
      }
    }

    /* And fix them up. */
    if (TSMTRACEP (THREED_MASK|3)) {
      for (i = 0, coords = coords0; i < len_coords; ++i, ++coords) {
	TSMTRACE ((THREED_MASK|3, funcName, 
		   "%2d: V[%g,%g,%g] N[%g,%g,%g] T[%g,%g]", i,
		   coords->vertex[0], coords->vertex[1], coords->vertex[2],
		   coords->norm[0], coords->norm[1], coords->norm[2],
		   coords->texCoord[0], coords->texCoord[1]));
      }
    }
  }

  TSMTRINC ((THREED_MASK|2, -1));
}

static void
CreateDrawListPrimitive (VisibleDraw *vis,
			 VisibleDrawList *list,
			 VisibleDrawGeoList *geo_list,
			 VisibleStruct const *visibleStruct,
			 RenderPrimitiveCache *rp_cache)
{
  static const char funcName[] = "CreateDrawListPrimitive";

  TsTile const *tile = visibleStruct->draw.tile;
  int tx = tile->id.x, ty = tile->id.y, res = tile->id.res;

  /* Initialize the draw structure. */
  *vis = visibleStruct->draw;
  TsCacheTileRef (tile);
  assert (! vis->prim);

  if (use_subtexture) {
    if (vis->tex.ref)
      TexRef (vis->tex.ref);
  } else {
    if (vis->tex.node)
      TextureRef (vis->tex.node);
  }
  TSMTRACE ((THREED_MASK|2, funcName, "%p [%d %d %d] <= struct %p",
	     vis, tx, ty, res, visibleStruct));

  if (! TsCacheTilePrepared (tile)) {
    TSMTRACE ((THREED_MASK|1, funcName, "Unprepared [%d %d %d]",
	       tx, ty, res));
    if (vis->prim) {
      abort ();
      releaseRenderPrimitive (rp_cache, vis->prim);
    }
    vis->prim = NULL;
    return;
  }

  if (! vis->prim) {
    RPRTR ((" CrD: geolist %p[%d] release\n", geo_list, vis-geo_list->vis));
    CreateRenderPrimitive (vis, list, geo_list, rp_cache);
  }
}

void
VisibleDrawInit (VisibleDrawList *list)
{
  list->n = 0;
  list->geo_list = 0;
  list->ntiles = 0;

  /* Initialize bounding box */
  TsBBoxInit (&list->bbox);
  list->bbox.coordType = TsCoordGCC;
}

void
VisibleDrawGeoClear (VisibleDrawGeoList *list)
{
  static char const funcName[] = "VisibleDrawGeoClear";
  TextureImage const *texim = list->texim;
  unsigned int i;

  for (i = 0; i < list->n; ++i) {
    VisibleDraw *vis = &list->vis[i];
    if (texim) {
      if (vis->tex.ref)
	TexRefRelease (vis->tex.ref);
    } else {
      if (vis->tex.node)
	TextureDeref (vis->tex.node);
    }

    TsCacheTileDeref ((TsTile *) vis->tile);
  }
  free (list->vis);
  list->n = 0;
  list->vis = 0;
}

void
VisibleDrawClear (VisibleDrawList *list)
{
  /*static char const funcName[] = "VisibleDrawClear";*/
  int i;
  for (i = 0; i < list->n; ++i)
    VisibleDrawGeoClear (&list->geo_list[i]);

  free (list->geo_list);

  VisibleDrawInit (list);
}

void
VisibleDrawDestroy (VisibleDrawList *list)
{
  /*static char const funcName[] = "VisibleDrawDestroy";*/

  VisibleDrawClear (list);

  if (list->geo_list)
    free (list->geo_list);
}

void
CreateDrawList (tvBase * threeD,
								VisibleList *list,
								VisibleDrawList *drawList,
								tvViewpoint const *view)
{
  /*static const char funcName[] = "CreateDrawList";*/
  RenderPrimitiveCache *rp_cache = threeD->rp_cache;
  int i;

  ClearDrawList (threeD, drawList);

  drawList->n = list->n;
  drawList->ntiles = list->ntiles;
  drawList->maxTiles = list->maxTiles;
  drawList->in_list = list->in_list;
  drawList->bbox = list->bbox;
  drawList->geo_list = malloc (list->n * sizeof (drawList->geo_list[0]));
  drawList->view = *view;
  drawList->bbox = list->bbox;

  RPRTR (("CrD: list %p\n", drawList));
  for (i = 0; i < list->n; ++i) {
    VisibleGeoList *src_list = list->geo_list[i];
    VisibleDrawGeoList *geo_list = &drawList->geo_list[i];

    VisibleStruct **vp = src_list->visp, **vend = vp + src_list->n;
    VisibleDraw *vis;

    RPRTR (("CrD: geolist %p[%d]\n", geo_list, geo_list->n));
    geo_list->n = src_list->n;
    geo_list->texim = src_list->im;
    geo_list->geo = src_list->geo;
    geo_list->vis = vis = malloc (geo_list->n * sizeof (geo_list->vis[0]));
    geo_list->prepared = FALSE;

    for ( ; vp < vend; ++vp, ++vis) 
      CreateDrawListPrimitive (vis, drawList, geo_list, *vp, rp_cache);
  }
}

#ifdef HAVE_VERTEX_ARRAY

#if !defined(GL_VERSION_1_2) && !defined(WIN32)

static inline void
myDrawElements (GLenum mode, GLsizei count, GLenum type,
		const GLvoid *indices_)
{
  /*static char const funcName[] = "myDrawElements";*/
  glBegin (mode); {
    int i;
    switch (type) {
    case GL_UNSIGNED_BYTE: {
      const GLubyte *indices = indices_;
      for (i = 0; i < count; ++i) {
	glArrayElementEXT (indices[i]);
      }
      break;
    }
    case GL_UNSIGNED_SHORT: {
      const GLushort *indices = indices_;
      for (i = 0; i < count; ++i) {
	glArrayElementEXT (indices[i]);
      }
      break;
    }
    case GL_UNSIGNED_INT: {
      const GLuint *indices = indices_;
      for (i = 0; i < count; ++i) {
	glArrayElementEXT (indices[i]);
      }
      break;
    }
    }
  }; glEnd ();
}

#else /* GL_VERSION_1_2 */

#define myDrawElements glDrawElements

#define glTexCoordPointerEXT(num,type,stride,len,ptr) \
  glTexCoordPointer(num,type,stride,ptr)
#define glNormalPointerEXT(type,stride,len,ptr) \
  glNormalPointer(type,stride,ptr)
#define glVertexPointerEXT(num,type,stride,len,ptr) \
  glVertexPointer(num,type,stride,ptr)
#define glArrayElementEXT glArrayElement

#endif

static inline void
DrawVArrayFragment (int p0, int p1, int p2, int span)
{
  /*static const char funcName[] = "DrawVArrayFragment";*/
  int w = p1 - p0;
  int i, j;
  for (j = p0; j < p2; j += span) {
    glBegin (GL_TRIANGLE_STRIP); {
      for (i = j; i <= j+w; ++i) {
	glArrayElementEXT (i+span);
	glArrayElementEXT (i);
      }
    }; glEnd ();
  }
}

int
tvBaseDrawPrimitive (VisibleDraw const *vis,
		     VisiblePoppingStrategy poppingStrategy,
		     PRBool selecting, PRBool selectModelsOnly)
{
  static char const *funcName = "tvBaseDrawPrimitive";

  RenderPrimitive const *prim = vis->prim;
  TsTile const *tile = vis->tile;
  TileGeometry const *geom = prim->geometry;
  TsCoord const *coords = prim->coords;
  unsigned short const *coordIndex = geom->coordIndex;
  int len_triStrips = geom->len_triStrips;
  unsigned short const *triStrips = geom->triStrips;
  int i;

  TSMTRACE ((THREED_MASK|2, funcName, "[%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res));     

  if (use_subtexture) {
    float const *texCoords
      = TexRefCoords (vis->tex.ref, geom->len_coords, coords);
    TSMTRACE ((TEX_MASK|2, funcName, "texref %p [%d %d %d] texCoords %p",
	       vis->tex.ref, tile->id.x, tile->id.y, tile->id.y, texCoords));
    glTexCoordPointerEXT (2, GL_FLOAT, 2 * sizeof (float),
			  geom->len_coords, texCoords);
  } else {
    glTexCoordPointerEXT (2, GL_FLOAT, sizeof (coords[0]),
			  geom->len_coords, &coords->texCoord);
  }

  glNormalPointerEXT (GL_FLOAT, sizeof (coords[0]),
		      geom->len_coords, &coords->norm);
  glVertexPointerEXT (3, GL_FLOAT, sizeof (coords[0]),
		      geom->len_coords, &coords->vertex);

  if ((vis->flags & VISFLAG_LOWRES)
      && poppingStrategy == VisiblePopping_LowFrag) {
    unsigned int flags = vis->flags;
    unsigned short const *neighIndex = geom->neighIndex;
    int span = neighIndex[TsCache_SE] - neighIndex[TsCache_SW] + 1;
    if (flags & VISFLAG_SHOW_SW) {
      int p0 = neighIndex[TsCache_SW];
      int p1 = neighIndex[(flags & VISFLAG_SHOW_SE) ? TsCache_SE : TsCache_S];
      int p2 = neighIndex[TsCache_W];
      TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		 2*tile->id.x, 2*tile->id.y, tile->id.res-1));
      if (flags & VISFLAG_SHOW_SE)
	TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		   2*tile->id.x+1, 2*tile->id.y, tile->id.res-1));
      DrawVArrayFragment (p0, p1, p2, span);
    } else if (flags & VISFLAG_SHOW_SE) {
      int p0 = neighIndex[TsCache_S];
      int p1 = neighIndex[TsCache_SE];
      int p2 = neighIndex[TsCache_C];
      TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		 2*tile->id.x+1, 2*tile->id.y, tile->id.res-1));
      DrawVArrayFragment (p0, p1, p2, span);
    }
    if (flags & VISFLAG_SHOW_NW) {
      int p0 = neighIndex[TsCache_W];
      int p1 = neighIndex[(flags & VISFLAG_SHOW_NE) ? TsCache_E : TsCache_C];
      int p2 = neighIndex[TsCache_NW];
      TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		 2*tile->id.x, 2*tile->id.y+1, tile->id.res-1));
      if (flags & VISFLAG_SHOW_SE)
	TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		   2*tile->id.x+1, 2*tile->id.y+1, tile->id.res-1));
      DrawVArrayFragment (p0, p1, p2, span);
    } else if (flags & VISFLAG_SHOW_NE) {
      int p0 = neighIndex[TsCache_C];
      int p1 = neighIndex[TsCache_E];
      int p2 = neighIndex[TsCache_N];
      TSMTRACE ((THREED_MASK|2, funcName, "- frag [%d %d %d]",
		 2*tile->id.x+1, 2*tile->id.y+1, tile->id.res-1));
      DrawVArrayFragment (p0, p1, p2, span);
    }
  } else if (! selecting) {
    for (i = 0; i < len_triStrips; ++i) {
      int n = triStrips[i];
      myDrawElements (GL_TRIANGLE_STRIP, n, GL_UNSIGNED_SHORT, coordIndex);
      coordIndex += n;
    }
  } else if ( ! selectModelsOnly ){
    PRBool flip = FALSE;
    int j, tri;

    /* gotta draw them as triangles for picking, rather than tri-strips */
    glPushName(vis->tile->id.res);
    glPushName(vis->tile->id.x);
    glPushName(vis->tile->id.y);

    for (j = 0, tri = 0; j < len_triStrips; ++j) {
      int n = triStrips[j];
      for (i = 0; i < n-2; ++i, ++tri, flip = !flip) {
	unsigned short const *cindex = &coordIndex[tri];
	glPushName(tri);
	glBegin (GL_TRIANGLES); {
	  glArrayElementEXT (cindex[0]);
	  if (! flip) {
	    glArrayElementEXT (cindex[1]);
	    glArrayElementEXT (cindex[2]);
	  } else {
	    glArrayElementEXT (cindex[2]);
	    glArrayElementEXT (cindex[1]);
	  }
	}; glEnd ();
	glPopName();
      }
      tri += 2;
    }

    glPopName();
    glPopName();
    glPopName();
  }

  return 1;
}
#else
static int
tvBaseDrawPrimitiveSlow (VisibleDraw const *vis,
			 VisiblePoppingStrategy poppingStrategy,
			 PRBool selecting, PRBool selectModelsOnly)
{
  static char const *funcName = "tvBaseDrawPrimitiveSlow";

  RenderPrimitive *prim = vis->prim;
  TsTile const *tile = vis->tile;
  TileGeometry const *geom = prim->geometry;
  int len_triStrips = geom->len_triStrips;
  unsigned short const *triStrips = geom->triStrips;
  unsigned short const *coordIndex = geom->coordIndex;
  TsCoord *coords = prim->coords;
  float const *texCoords = 0;
  int i, j;

  if (!selecting) {
    prim->disp_list = gltGenList ();
    TSMTRACE ((THREED_MASK|OGL_MASK|2, funcName,
	       "PRIM [%d %d %d] %02x: create %d %p",
	       tile->id.x, tile->id.y, tile->id.res,
	       prim->sig,  prim->disp_list, geom));
    glNewList (prim->disp_list, GL_COMPILE_AND_EXECUTE);

    if (use_subtexture) {
      texCoords = TexRefCoords (vis->tex.ref, geom->len_coords, coords);
      TSMTRACE ((TEX_MASK|2, funcName, "texref %p [%d %d %d] texCoords %p",
		 vis->tex.ref, tile->id.x, tile->id.y, tile->id.y, texCoords));
    }

    TSMTRINC ((THREED_MASK|3, 1));
    for (j = 0; j < len_triStrips; ++j) {
      int n = *triStrips++;
      glBegin (GL_TRIANGLE_STRIP); {
	for (i = 0; i < n; ++i) {
	  int idx = *coordIndex++;
	  TsCoord const *coord = &coords[idx];
	  TSMTRACE((THREED_MASK|3, funcName,
		    "%d V:%.1f,%.1f,%.1f N:%.2f,%.2f,%.2f T:%.2f,%.2f", idx,
		    coord->vertex[0], coord->vertex[1], coord->vertex[2],
		    coord->norm[0], coord->norm[1], coord->norm[2],
		    coord->texCoord[0], coord->texCoord[1]));
	  if (texCoords)
	    glTexCoord2fv (&texCoords[2*idx]);
	  else
	    glTexCoord2fv (coord->texCoord);
	  glNormal3fv (coord->norm);
	  glVertex3fv (coord->vertex);
	}
      }; glEnd ();
    }
    TSMTRINC ((THREED_MASK|3, -1));

    glEndList ();
  } else if ( ! selectModelsOnly ) {
    PRBool flip = FALSE;
    int tri;

    /* gotta draw them as triangles for picking, rather than tri-strips */
    glPushName(tile->id.res);
    glPushName(tile->id.x);
    glPushName(tile->id.y);

    for (j = 0, tri = 0; j < len_triStrips; ++j) {
      int n = *triStrips++;
      for (i = 0; i < n-2; ++i, ++tri, flip = !flip) {
	PRUint16 const *cindex = &coordIndex[tri];
	glPushName(tri);
	glBegin (GL_TRIANGLES); {
	  glVertex3fv (coords[cindex[0]].vertex);
	  if (! flip) {
	    glVertex3fv (coords[cindex[1]].vertex);
	    glVertex3fv (coords[cindex[2]].vertex);
	  }
	  else {
	    glVertex3fv (coords[cindex[2]].vertex);
	    glVertex3fv (coords[cindex[1]].vertex);
	  }
	}; glEnd ();
	glPopName();
      }
      tri += 2;
    }

    glPopName();
    glPopName();
    glPopName();
  }

  gltCheckErrors (FILE_LOC);

  return 0;
}

int
tvBaseDrawPrimitive (VisibleDraw const *vis,
		     VisiblePoppingStrategy poppingStrategy,
		     PRBool selecting, PRBool selectModelsOnly)
{
  static char const *funcName = "tvBaseDrawPrimitive";
  RenderPrimitive *prim = vis->prim;

  if (! prim)
    return 0;

  if (prim->disp_list && !selecting) {
    TSMTRACE ((THREED_MASK|OGL_MASK|2, funcName,
	       "PRIM [%d %d %d] %02x: use %d",
	       vis->tile->id.x, vis->tile->id.y, vis->tile->id.res,
	       prim->sig,  prim->disp_list));
    glCallList (prim->disp_list);
    return 1;
  } else {
    return tvBaseDrawPrimitiveSlow (vis, poppingStrategy, selecting, 
				    selectModelsOnly);
  }
}
#endif

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: June 1995
 *
 */

void
ClearWorkList(tvBase * threeD,
	      VisibleList * list)
{
  /*static char const *funcName = "ClearWorkList";*/
  int i;

  for (i = 0; i < list->n; ++i) {
    VisibleGeoList *geo_list = list->geo_list[i];
    VisibleStruct **vp = geo_list->visp, **vend = vp + geo_list->n;

    /* CLEAR THE ACCUMULATED RENDERING PRIMITIVES */
    for ( ; vp < vend; ++vp) {
      VisibleStruct *vis = *vp;
      assert (! vis->draw.prim);
    }
  }

  /* CLEAR THE VISIBILITY MANAGEMENT VARIABLES. */ 
  VisibleClear (list);
}

void
ClearDrawList(tvBase * threeD,
	      VisibleDrawList * list)
{
  /*static char const *funcName = "ClearDrawList";*/
  int i;

  RPRTR (("ClD: list %p\n", list));
  for (i = 0; i < list->n; ++i) {
    VisibleDrawGeoList *geo_list = &list->geo_list[i];
    VisibleDraw *vis = geo_list->vis, *vend = vis + geo_list->n;

    RPRTR (("ClD: geolist %p[%d]\n", geo_list, geo_list->n));
    /* CLEAR THE ACCUMULATED RENDERING PRIMITIVES */
    for ( ; vis < vend; ++vis) {
      if (vis->prim) {
	RPRTR ((" ClD: geolist %p[%d] release\n", geo_list, vis-geo_list->vis));
	releaseRenderPrimitive (threeD->rp_cache, vis->prim);
	vis->prim = NULL;
      }
    }
  }

  /* CLEAR THE VISIBILITY MANAGEMENT VARIABLES. */ 
  VisibleDrawClear (list);
}

void
tvBaseReset (tvBase * threeD)
{
  static const char funcName[] = "tvBaseReset";
  int i;
  TSMTRACE ((THREED_MASK|1, funcName, ""));

  tvBaseStopThreads (threeD);
  for (i = 0; i < 3; ++i)
    ClearDrawList (threeD, &threeD->drawList[i]);

  TextureCacheReset (threeD->textureCache);
}
