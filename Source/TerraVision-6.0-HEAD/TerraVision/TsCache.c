/* -*- tab-width: 8; -*-

  Revision History:
      $Id: TsCache.c,v 2.62.2.11 2002/10/31 05:26:37 reddy Exp $

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

#include "TsCache.h"
#include "TsCacheI.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "NetLogCore.h"
#include "TsLogging.h"

#include <prinit.h>
#include <prerror.h>
#include <prnetdb.h> /* for PR_ntohl() */

#if !( defined(WIN32) && defined(MSVC) )
# include <values.h>
#endif

#ifdef TRACE_ALLOC
# define ALLOCTR(x) printf x
#else
# define ALLOCTR(x)
#endif

#define READER_THREAD (0)

char const *TsCacheTileStatusName[] = {
  "Prepared",       /* Prepared for external use */
  "Resident",       /* Resident in cache */
  "Not Resident",   /* Not resident in cache */
  "Requested"       /* Requested */
};

static int TsCacheTile (TsCache *cache,
			TsmTileHeader const *tileHeader,
			uchar const *tileBuffer,
			PRUint32 tileSize);
static int TsCacheResetTile (TsCache *cache,
			     TsmTileHeader const *tileHeader);
static void TsCacheTileDecache (TsTile *tile);
static void TsCacheTileExtentCB (TsCache const *cache,
				 void *arg, void *closure);

static void TsCacheFixPyramid (TsCache *cache);
static void TsCacheSetDefaultCoords (TsCache *cache);
static void TsCacheSetOrigin1 (TsCache *cache, double u, double v,
			       int update_extent);

char const * const TsCacheNeighbour_name[] = {
  "TsCache_NW", "TsCache_N", "TsCache_NE",
  "TsCache_W",  "TsCache_C", "TsCache_E",
  "TsCache_SW", "TsCache_S", "TsCache_SE"
};

TsCacheNeighbour const TsCacheNeighbour_reverse[] = {
/*TsCache_NW, TsCache_N, TsCache_NE */ 
  TsCache_SE, TsCache_S, TsCache_SW,
/*TsCache_W,  TsCache_C, TsCache_E */
  TsCache_E,  TsCache_C, TsCache_W, 
/*TsCache_SW, TsCache_S, TsCache_SE*/
  TsCache_NE, TsCache_N, TsCache_NW
};

static TsCallbackList TsCacheCallbacks[CACHE_NCALLBACKS];

typedef struct TsGlobals {
  list_t *lru;
  PRLock *lruLock;
  list_t *caches;
  PRLock *cacheLock;
} TsGlobals;

PRUint32 TsCacheMemUsed = 0; 
PRUint32 TsCacheMemReserved = 32*(1<<20);

static TsGlobals tsCache;
static PRCallOnceType tsCacheOnce;

static PRStatus
TsCacheInit (void)
{
  /* Management of the LRU list */
  tsCache.lruLock = PR_NewLock ();
  tsCache.lru = list_create (FALSE);

  /* Management of TsCaches */
  tsCache.cacheLock = PR_NewLock ();
  tsCache.caches = list_create (FALSE);

  return PR_SUCCESS;
}

void
TsCacheLRUFixup (TsTile const *tile)
{
  static const char funcName[] = "TsCacheLRUFixup";
  PR_Lock (tsCache.lruLock); {
    list_iter_t i = tile->lru_iter;
    if (tile->refcount == 0) {
      if (! list_iter_valid (i)) {
	((TsTile *)tile)->lru_iter = list_push (tsCache.lru, tile);
	ALLOCTR (("     LRU: + %p[%d %d %d]%d\n",
		  tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount));
	TSMTRACE ((CACHE_MASK|2, funcName, "ADD %p[%d %d %d] %s",
		   tile, tile->id.x, tile->id.y, tile->id.res, 
		   (tile->cache)?tile->cache->url:NULL));
      }
    } else {
      if (list_iter_valid (i)) {
	list_delete (i);
	((TsTile *)tile)->lru_iter = 0;
	ALLOCTR (("     LRU: - %p[%d %d %d]%d\n",
		  tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount));
	TSMTRACE ((CACHE_MASK|2, funcName, "DEL %p[%d %d %d] %s",
		   tile, tile->id.x, tile->id.y, tile->id.res,
		   (tile->cache)?tile->cache->url:NULL));
      }
    }
  }; PR_Unlock (tsCache.lruLock);
}

static void
TsCacheLRUDecacheFirst (void)
{
  static char const funcName[] = "TsCacheLRUDecacheFirst";
  TsTile *tile;

  /* Get the first unreferenced tile on the LRU list for deallocation. */
  for (tile = list_unshift (tsCache.lru);
       tile && tile->refcount != 0;
       tile = list_unshift (tsCache.lru)) {
    ALLOCTR (("     LRU: ! %p[%d %d %d]%d mem %d\n",
	      tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount,
	      TsCacheMemUsed));
    TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d] ref %d on LRU",
	       tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount));
    tile->lru_iter = 0;
  }

  /* Bug out if the LRU list is empty. */
  if (! tile) return;

  if (tile->refcount == 0) {
    tile->lru_iter = 0;

    ALLOCTR (("     LRU: - %p[%d %d %d]%d mem %d\n",
	      tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount,
	      TsCacheMemUsed));

    /* Oooh.  TsCacheTileDecache() *may* result in a call to
       TsCacheLRUInsert(), so undo lock while decaching. */
    PR_Unlock (tsCache.lruLock);
    TsCacheTileDecache (tile);
    PR_Lock (tsCache.lruLock);
  }
}

static void
TsCacheLRUCleanup (void)
{
  /*static const char funcName[] = "TsCacheLRUCleanup";*/
  PR_Lock (tsCache.lruLock); {
    while (TsCacheMemUsed > 8*TsCacheMemReserved/10
	   && ! list_empty (tsCache.lru))
      TsCacheLRUDecacheFirst ();
  }; PR_Unlock (tsCache.lruLock);
}

void
TsCacheClearAll (void)
{
  /*static const char funcName[] = "TsCacheClear";*/
  PR_Lock (tsCache.lruLock); {
    while (! list_empty (tsCache.lru))
      TsCacheLRUDecacheFirst ();
  }; PR_Unlock (tsCache.lruLock);
}

TsCache *
TsCacheCreate (char const *url, TsmConnectionParams params)
{
  static char const funcName[] = "TsCacheCreate";
  TsCache *cache = 0;
  int i, used;

  /* Set up global tile cache. */
  PR_CallOnce (&tsCacheOnce, TsCacheInit);

  PR_Lock (tsCache.cacheLock); {
    list_iter_t i;
    for (i = list_first (tsCache.caches); ! list_done (i); i = list_next (i)) {
      TsCache *c = list_value (i);
      if (! strcmp (c->url, url)) {
	cache = c;
	used = PR_AtomicIncrement (&cache->used);	
	break;
      }
    }
    /* Find previous cache with same URL */
  }; PR_Unlock (tsCache.cacheLock);

  if (cache) {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%s) reuse %p[%d]", 
	       url, cache, used));
    return cache;
  }

  cache = (TsCache *) malloc (sizeof (TsCache));

  TSMTRACE ((CACHE_MASK|1, funcName, "(%s)", url));
  cache->used = 1;
  cache->connect = 0;
  cache->url = tsmMemStrdup (url);

  if (! params)
    tsmInitParams (&cache->params);
  else
    cache->params = params;

  cache->byRes = 0;
  cache->readerWait = 100;
  cache->reader = 0;

  for (i = 0; i < CACHE_NCALLBACKS; ++i) {
    TsCallbackList *lst = &cache->callback[i];
    lst->n = lst->len = 0;
    lst->cb = 0;
  }

  cache->lock = PR_NewLock();

  TsCacheAddCallback (cache, TILE_INIT, TsCacheTileExtentCB, NULL, NULL);

  TSMTRACE ((CACHE_MASK|1, funcName, "<= %p", cache));
  return cache;
}

static void TsCacheReader (void *vcache);
static int tile_eql (void const *t1v, void const *t2v);
static PRUint32 tile_hash (void const *t1v);

int
TsCacheConnect (TsCache *cache)
{
  static char const funcName[] = "TsCacheConnect";
  SimpleTileSet *tset;
  TsmConnection connect;
  int ret = TRUE;

  TsCache_lock (cache); {
    if (! cache->connect) {
      TSMTRACE ((CACHE_MASK|1, funcName, "(%p) %s", cache, cache->url));
      cache->connect = connect = tsmConnect (cache->url, "r", cache->params);
      ret = FALSE;
      if (connect) {
	TileSetTspec const *tspec;
	SimpleTileSet *tspecLevel;
	int minLevel, maxLevel, numLevels, i;

	cache->tspec = tspec = (Pyramid *) tsmGet (connect, TSM_DATASET_TSPEC);
	cache->tile_size = tsmGet( connect, TSM_TILE_SIZE );

	/* Are we dealing with a southern hemisphere UTM dataset? If so, */
        /* we convert the negative northing representation (belch) into  */
        /* a positive northing with a hemisphere flag set to southern.   */
        /* Otherwise, the use of negative UTM coords really confuses a   */
        /* lot of the code. */

	cache->southern_utm = FALSE;
	tset = &tspec->tspecLevel.val[tspec->minLevel];
	if ( tspec->projectionTspec.projectionType == tspUtmProjection && 
	     tset && tset->imageTransformationTspec.ixiytoUV[5] < 0.0 ) {
	  cache->southern_utm = TRUE;

	  /* negate the -ve northing and recompute the tspec matrices */
	  tset->imageTransformationTspec.ixiytoUV[4] = (float)
	    fabs( tset->imageTransformationTspec.ixiytoUV[4] );
	  tset->imageTransformationTspec.ixiytoUV[5] = (float)
	    fabs( tset->imageTransformationTspec.ixiytoUV[5] );
	  tsmFillSimpleTileSets( tspec );
	}

	TsCacheFixPyramid (cache);

	tspecLevel = tspec->tspecLevel.val;
	minLevel = tspec->minLevel;
	maxLevel = tspec->maxLevel;
	numLevels = maxLevel - minLevel + 1;

	/* Create the byRes array. */
	cache->byRes = malloc (numLevels * sizeof (cache->byRes[0]));
	cache->byRes -= minLevel;

	/* Initialize each level. */
	for (i = minLevel; i <= maxLevel; ++i) {
	  TsCacheByRes * curr = &cache->byRes[i];
	  SimpleTileSet const * tset = &tspecLevel[i];
	  int j, len = tset->numTile[0] * tset->numTile[1];
	  curr->tset = tset;
	  curr->monitor = PR_NewMonitor ();
	  curr->tiles = eqlhash_create (64, tile_eql, tile_hash);
	  for (j = 0; j < 2; ++j) {
	    curr->min_pxl[j] = tset->minValidPixel[j];
	    curr->max_pxl[j] = tset->maxValidPixel[j];
	  }
	  TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d] %d x %d (%d-%d x %d-%d)",
		     cache, i, curr->max_pxl[0], curr->max_pxl[1],
		     tset->minTile[0], tset->maxTile[0],
		     tset->minTile[1], tset->maxTile[1]));
	}

	/* Set up the coordinate systems and their relationships. */
	cache->lvcs = FALSE;
	TsCacheSetDefaultCoords (cache);

	/* If we can set up the coordinates, then start up. */
	if (TsCacheCoordsInit (cache)) {
	  /* Start the reader thread. */
#if READER_THREAD
	  char const *name = tspec->identityTspec.name;
	  TSMTRACE ((CACHE_MASK|1, funcName, "Start %s reader", name));
	  ret = ! pthr_create (&cache->reader, name,
			       TsCacheReader, (void *)cache);
#else
	  ret = TRUE;
#endif
	} else {
	  tsmError (funcName, "Unable to initialize coordinates.");
	  ret = FALSE;
	}
      }
    }
  }; TsCache_unlock (cache);

  return ret;
}

#if !READER_THREAD
typedef struct {
  TsCache *cache;
  int errors;
} TsCacheReceiveTileClosure;

static int
TsCacheReceiveTile (tsmRequestList *req,
		    TsmTileHeader *tileHeader,
		    uchar *tileBuffer,
		    /*int received,*/
		    void *closure)
{
  static char const funcName[] = "TsCacheReceiveTile";
  TsCacheReceiveTileClosure *rt = closure;
  TsCache *cache = rt->cache;
  int tx = tsmTileHeaderX (tileHeader);
  int ty = tsmTileHeaderY (tileHeader);
  int res = tsmTileHeaderLevel (tileHeader);
  
  /* Count number of consecutive errors.  If >= 16 then assume bad
     connection and shut down. */
  if (! tileBuffer) {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p) [%d %d %d] FAIL",
	       cache, tx, ty, res));

    TsCacheResetTile (cache, tileHeader);

    if (++rt->errors >= 16) {
      /*tsmError (funcName, "Bad connection '%s'. Shutting down.", cache->url);
	TsCacheDisconnect (cache);*/
    }

    return TRUE;
  }

  /* Reset error count. */
  rt->errors = 0;

  TSMTRACE ((CACHE_MASK|2, funcName, "(%p) Read tile [%d %d %d]", cache, 
	     tx, ty, res));

  /* feed the burst rate calculation */
  TsLogBurstRate( cache->tile_size );

  /* log the read event */
  if ( TsIsLogging (TV_READER_LOG) )
    TsLogTileRead( 0 );

  /* Process tile. */
  TsCacheTile (cache, tileHeader, tileBuffer, cache->tile_size);

  return TRUE;
}
#endif

/*
 * Synopsis:
 *     tsmRequestList * TsCacheRequestList (TsCache *cache)
 *
 * Doc++ Description: *//**
 *     Creates a new request list for the given TsCache.
 *
 * @param cache The tileset cache.
 *
 * @author Lee Iverson
 * @version  November 2000
 *
 * @return  A new tsmRequestList
 *
 * @see TsCacheConnect, TsCache, tsmRequestList
 *
 */

tsmRequestList *
TsCacheRequestList (TsCache *cache)
{
  tsmRequestList *reqList = NULL;
  if (cache->connect)
    reqList = tsmConnectRequestList (cache->connect);
  
#if !READER_THREAD
  if (reqList) {
    TsCacheReceiveTileClosure *cl = malloc (sizeof (*cl));
    cl->cache = cache;
    cl->errors = 0;
    tsmRequestNotifyReceive (reqList, TsCacheReceiveTile, cl);
  }
#endif

  return reqList;
}

/*
 * Synopsis: void TsCacheFixPyramid (TsCache *cache)
 *
 * Doc++ Description: *//**
 *     Considers the Pyramid represented by the tspec for a cache.  Fixes this
 *     tspec to guarantee certain invariants, namely that all valid tiles have
 *     a parent in the pyramid, excepting the root tile.
 *
 *     This function should be the only one in the system which alters the
 *     tspec. 
 *
 * @param cache The tileset cache.
 *
 * @author Lee Iverson
 *
 * @return  void
 *
 * @see TsCache, TileSetTspec, SimpleTileSet
 *
 */

static void
TsCacheFixPyramid (TsCache *cache)
{
  static char const funcName[] = "TsCacheFixPyramid";
  TileSetTspec const *tspec = cache->tspec;
  int minLevel = tspec->minLevel;
  int maxLevel = tspec->maxLevel;
  SimpleTileSet *tset = &tspec->tspecLevel.val[minLevel];
  int max_ix = tset->maxValidPixel[0];
  int max_iy = tset->maxValidPixel[1];

  /* First, we make sure that we have a properly nested Pyramid. */
  {
    int res;
    int px = 0, py = 0;
    for (res = maxLevel; res >= minLevel; --res) {
      SimpleTileSet *ltset = &tspec->tspecLevel.val[res];
      int tx = ltset->maxTile[0];
      int ty = ltset->maxTile[1];
      if (tx > 2*px+1) {
	TSMTRACE ((CACHE_MASK|1, funcName, "[%d] maxTile[0] %d -> %d",
		   res, tx, 2*px+1));
	tx = 2*px+1;
	ltset->maxTile[0] = tx;
	ltset->numTile[0] = tx - ltset->minTile[0] + 1;

	/* Propagate this fix down to the pixel info. */
	if ((tx+1)*ltset->tileTspec.numPixelsX-1 < ltset->maxValidPixel[0]) {
	  TSMTRACE ((CACHE_MASK|1, funcName, "[%d] maxPixel[0] %d -> %d",
		     res, ltset->maxValidPixel[0],
		     (tx+1)*ltset->tileTspec.numPixelsX-1));
	  ltset->maxValidPixel[0] = 
	    (tx+1)*ltset->tileTspec.numPixelsX-1;
	  ltset->numPixels[0] = 
	    ltset->maxValidPixel[0] - ltset->minValidPixel[0] + 1;
	}
      }
      px = tx;

      if (ty > 2*py+1) {
	TSMTRACE ((CACHE_MASK|1, funcName, "[%d] maxTile[1] %d -> %d",
		   res, ty, 2*py+1));
	ty = 2*py+1;
	ltset->maxTile[1] = ty;
	ltset->numTile[1] = ty - ltset->minTile[1] + 1;

	/* Propagate this fix down to the pixel info. */
	if ((ty+1)*ltset->tileTspec.numPixelsY-1 < ltset->maxValidPixel[1]) {
	  TSMTRACE ((CACHE_MASK|1, funcName, "[%d] maxPixel[1] %d -> %d",
		     res, ltset->maxValidPixel[1],
		     (ty+1)*ltset->tileTspec.numPixelsY-1));
	  ltset->maxValidPixel[1] = 
	    (ty+1)*ltset->tileTspec.numPixelsY-1;
	  ltset->numPixels[1] = 
	    ltset->maxValidPixel[1] - ltset->minValidPixel[1] + 1;
	}
      }
      py = ty;
    }
  }

  max_ix = tset->maxValidPixel[0];
  max_iy = tset->maxValidPixel[1];

  /* This hack is added to protect against improperly nested pyramids that may
     occur because of improper rounding of level sizes. */
  {
    int res, mult;
    for (res = minLevel, mult = 1; res <= maxLevel; ++res, mult *= 2) {
      SimpleTileSet *ltset = &tspec->tspecLevel.val[res];
      int max_lx = ltset->maxValidPixel[0];
      int max_ly = ltset->maxValidPixel[1];
      int max_tx = ltset->tileTspec.numPixelsX * (ltset->maxTile[0]+1);
      int max_ty = ltset->tileTspec.numPixelsY * (ltset->maxTile[1]+1);
      TSMTRACE ((CACHE_MASK|2, funcName,
		 "level[%d] X pxl %d/%.1f %.1f tiles %dx%d",
		 res, max_lx, max_ix/(float)mult, 
		 max_ix/(float)mult/ltset->tileTspec.numPixelsX,
		 ltset->numTile[0],
		 ltset->tileTspec.numPixelsX, max_tx));
      TSMTRACE ((CACHE_MASK|2, funcName,
		 "level[%d] Y pxl %d/%.1f %.1f tiles %dx%d",
		 res, max_ly, max_iy/(float)mult, 
		 max_iy/(float)mult/ltset->tileTspec.numPixelsY,
		 ltset->numTile[1],
		 ltset->tileTspec.numPixelsY, max_ty));
      if (max_lx > max_tx) {
	ltset->maxValidPixel[0] = max_tx;
	max_ix = max_tx * mult;
	TSMTRACE ((CACHE_MASK|1, funcName, "level[%d] fixup X: %d -> %d",
		   res, max_lx, max_tx));
      }
      if (max_ly > max_ty) {
	ltset->maxValidPixel[1] = max_ty;
	max_iy = max_ty * mult;
	TSMTRACE ((CACHE_MASK|1, funcName, "level[%d] fixup Y: %d -> %d",
		   res, max_ly, max_ty));
      }
    }
  }
}

static void
TsCacheSetDefaultCoords (TsCache *cache)
{
  /*static char const funcName[] = "TsCacheSetDefaultCoords";*/
  TileSetTspec const *tspec = cache->tspec;
  int minLevel = tspec->minLevel;
  SimpleTileSet *tset = &tspec->tspecLevel.val[minLevel];
  int max_ix = tset->maxValidPixel[0];
  int max_iy = tset->maxValidPixel[1];
  float const *ixiytoUV = tset->imageTransformationTspec.ixiytoUV;
  double origin_u = TFM_U (0, 0, ixiytoUV);
  double origin_v = TFM_V (0, 0, ixiytoUV);

  /* We start out in UV coords in all cases. */
  cache->bbox.coordType = TsCoordUV;

  /* Define a local coordinate system around the (0,0) image point. */
  TsCacheSetOrigin1 (cache, origin_u, origin_v, FALSE);

  /* Now this is done in the local coordinate system defined above. */
  ixiytoUV = TsCacheTfm_ixiytouv (cache, minLevel);
  TsCacheSetExtent (cache, 0, 0, 
		    TFM_U (max_ix+1, max_iy+1, ixiytoUV),
		    TFM_V (max_ix+1, max_iy+1, ixiytoUV));
}

static void
TsCacheSetOrigin1 (TsCache *cache, double u, double v, int update_extent)
{
  static const char funcName[] = "TsCacheSetOrigin1";
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int i, j;
  double prev_u = cache->origin[0];
  double prev_v = cache->origin[1];

  TSMTRACE ((CACHE_MASK|1, funcName, "%g,%g", u, v));
  cache->origin[0] = u;
  cache->origin[1] = v;

  /* Initialize each level. */
  for (i = tspec->minLevel; i <= tspec->maxLevel; ++i) {
    SimpleTileSet const * tset = TsCacheTileSet (cache, i);
    float const *ixiytoUV = tset->imageTransformationTspec.ixiytoUV;
    TsCacheByRes * curr = &cache->byRes[i];
    float *tfm = curr->ixiytouv;

    for (j = 0; j < 9; ++j) tfm[j] = ixiytoUV[j];
    tfm[2] -= u;
    tfm[5] -= v;
    TSMTRACE ((CACHE_MASK|2, funcName, 
	       "[%d] ixiytouv %g,%g,%g %g,%g,%g %g,%g,%g",
	       i, tfm[0], tfm[1], tfm[2], tfm[3],
	       tfm[4], tfm[5], tfm[6], tfm[7], tfm[8]));
    tsmInverse3x3 (curr->ixiytouv, curr->uvtoixiy);
    tfm = curr->uvtoixiy;
    TSMTRACE ((CACHE_MASK|2, funcName, "[%d] uvtoixiy %g,%g,%g %g,%g,%g %g,%g,%g",
	       i, tfm[0], tfm[1], tfm[2], tfm[3],
	       tfm[4], tfm[5], tfm[6], tfm[7], tfm[8]));
  }

  if (update_extent) {
    float off_u = u - prev_u;
    float off_v = v - prev_v;
    
    TsCacheSetExtent (cache,
		      cache->bbox.min[0] - off_u,
		      cache->bbox.min[1] - off_v,
		      cache->bbox.max[0] - off_u,
		      cache->bbox.max[1] - off_v);
  }
}

void
TsCacheSetExtent (TsCache *cache,
		  float min_u, float min_v, float max_u, float max_v)
{
  static const char funcName[] = "TsCacheSetExtent";

  TSMTRACE ((CACHE_MASK|2, funcName, "%g,%g -> %g,%g",
	     min_u, min_v, max_u, max_v));
  cache->bbox.min[0] = min_u;
  cache->bbox.min[1] = min_v;
  cache->bbox.min[2] = 0;
  cache->bbox.max[0] = max_u;
  cache->bbox.max[1] = max_v;
  cache->bbox.max[2] = 0;
  cache->bbox.center[0] = (min_u + max_u)/2;
  cache->bbox.center[1] = (min_v + max_v)/2;
  cache->bbox.center[2] = 0;
}

static void
tile_destroy (void const *obj, void *val, void *closure)
{
  TsCacheTileDecache ((TsTile *) val);
}

void
TsCacheDisconnect (TsCache *cache)
{
  static char const funcName[] = "TsCacheDisconnect";
  int used;

  if ((used = cache->used)) {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p) used[%d]", cache, used));
    return;
  }

  TSMTRACE ((CACHE_MASK|1, funcName, "(%p)", cache));
  TSMTRINC ((CACHE_MASK|1, 1));

  /* This is done separately so that the reader can in initiate a
     disconnect. N.B. If the reader performs a disconnect, then the main
     thread must also call TsCacheDisconnect(). */
  if (cache->reader && PR_GetCurrentThread () != cache->reader) {
    /* Shut down reader thread. */
    TSMTRACE ((CACHE_MASK|1, funcName, "Cancel reader %p", cache->reader));
    PR_Interrupt (cache->reader);
    PR_JoinThread (cache->reader);
    cache->reader = 0;
    TSMTRACE ((CACHE_MASK|1, funcName, "Joined"));
  }

  if (cache->connect) {
    TsCache_lock (cache); {

      TsmConnection connect = cache->connect;
      TileSetTspec const *tspec = cache->tspec;
      int minLevel = tspec->minLevel, maxLevel = tspec->maxLevel;
      int i;

      TSMTRACE ((CACHE_MASK|1, funcName, "Disconnect %p", connect));
      cache->connect = 0;

      /* Decache each level. */
      for (i = minLevel; i <= maxLevel; ++i) {
	TsCacheByRes * curr = &cache->byRes[i];
	PR_EnterMonitor (curr->monitor); {
	  eqlhash_forall (curr->tiles, tile_destroy, 0);
	  eqlhash_destroy (curr->tiles);
	}; PR_ExitMonitor (curr->monitor);
	PR_DestroyMonitor (curr->monitor);
      }

      /* Destroy the coordinate conversion constants, needs tspec */
      TsCacheCoordsDestroy (cache);

      /* This is delayed, since the tspec is freed on disconnect. */
      tsmDisconnect (connect);

      /* Free the byRes indexing arrays. */
      free (cache->byRes + minLevel);
      cache->byRes = 0;
    }; TsCache_unlock (cache);
  }

  TSMTRACE ((CACHE_MASK|1, funcName, "Done"));
  TSMTRINC ((CACHE_MASK|1, -1));
}

static TsTile *
TsCacheAllocTile (TsCache *cache)
{
  static char const funcName[] = "TsCacheAllocTile";

  TSMTRACE ((CACHE_MASK|3, funcName, "(%p)", cache));

  /* First check that we have room for this tile. */
  if (TsCacheMemUsed > TsCacheMemReserved)
    TsCacheLRUCleanup ();

  /* Then allocate it once we're sure. */
  {
    TsTile *tile = calloc (1, sizeof (*tile));
    TsCacheTileAddMem (tile, sizeof (*tile));
    TSMTRACE ((CACHE_MASK|3, funcName, "tile %p", tile));
    return tile;
  }
}

static void
TsCacheFreeTile (TsTile *tile)
{
  static char const funcName[] = "TsCacheFreeTile";

  TSMTRACE ((CACHE_MASK|3, funcName, "tile %p", tile));

  /* If data is non-null, free it. */
  if (tile->data)
    tsmFreeTileBuffer ((uchar *) tile->data);

  /* Finally free the tile */
  TsCacheTileAddMem (tile, -tile->size);
  free (tile);
}

void
TsCacheDestroy (TsCache *cache)
{
  static char const funcName[] = "TsCacheDestroy";
  int i, used;

  /* Decrement use count and exit if still in use. */
  used = PR_AtomicDecrement (&cache->used);
  if (used) {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p) used[%d]", cache, used));
    return;
  }

  /* First things first.  Make sure we're disconnected. */
  if (cache->connect)
    TsCacheDisconnect (cache);

  tsmFreeParams (cache->params);

  /* This is not explicitly a critical section, but we'll wait for lock to be
     free, so we can destroy it. */
  TsCache_lock (cache);

  /* Free up the callbacks. */
  TSMTRACE ((CACHE_MASK|2, funcName, "(%p) Free data", cache));
  for (i = 0; i < CACHE_NCALLBACKS; ++i) {
    TsCallbackList *lst = &cache->callback[i];
    if (lst->cb) {
      int j;
      for (j = 0; j < lst->n; ++j) {
	TsCallback *cb = &lst->cb[j];
	if (cb->closure && cb->destroyf)
	  (*cb->destroyf) (cb->closure);
      }
      free (lst->cb);
    }
  }

  /* And finally destroy our protection. */
  TSMTRACE ((CACHE_MASK|2, funcName, "(%p) Destroy lock", cache));
  TsCache_unlock (cache);
  PR_DestroyLock (cache->lock);

  tsmMemFree (cache->url);

  free (cache);
}

int
TsCacheAddCallback (TsCache *cache, TsCacheCB which,
		    TsCallbackF func, void *closure,
		    void (*destroyf)(void*))
{
  static const char funcName[] = "TsCacheAddCallback";
  TsCallbackList *lst = (cache
			 ? &cache->callback[which]
			 : &TsCacheCallbacks[which]);
  TsCallback *cb;
  int i;

  /* Search for match. Fail if found. */
  for (i = 0, cb = lst->cb; i < lst->n; ++i, ++cb)
    if (cb->func == func && cb->closure == closure) {
      TSMTRACE ((CACHE_MASK|1, funcName, "(%p, %d, %p, %p) exists!",
		 cache, which, func, closure));
      return FALSE;
    }

  if (cache) TsCache_lock (cache); {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p, %d, %p, %p)",
	       cache, which, func, closure));
    if (lst->n == lst->len) {
      if (! lst->len) {
	lst->len = 4;
	lst->cb = malloc (lst->len * sizeof (lst->cb[0]));
      } else {
	lst->len *= 2;
	lst->cb = realloc (lst->cb, lst->len * sizeof (lst->cb[0]));
      }
      if (! lst->cb) {
	tsmError (funcName, "Failed to allocate callback array.");
	return FALSE;
      }
    }

    cb = &lst->cb[lst->n++];
    cb->func = func;
    cb->closure = closure;
    cb->destroyf = destroyf;

    for (i = 0, cb = lst->cb; i < lst->n; ++i, ++cb)
      TSMTRACE ((CACHE_MASK|1, funcName, "[%d] (%p, %d, %p, %p)",
		 i, cache, which, cb->func, cb->closure));
  }; if (cache) TsCache_unlock (cache);

  return TRUE;
}

int
TsCacheRemoveCallback (TsCache *cache, TsCacheCB which,
		       TsCallbackF func, void *closure)
{
  static const char funcName[] = "TsCacheRemoveCallback";
  TsCallbackList *lst = (cache
			 ? &cache->callback[which]
			 : &TsCacheCallbacks[which]);
  TsCallback *cb;
  int i;

  /* Search for match. */
  for (i = 0, cb = lst->cb; i < lst->n; ++i, ++cb)
    if (cb->func == func && cb->closure == closure)
      break;
  if (i == lst->n) {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p, %d, %p, %p) not found!",
	       cache, which, func, closure));
    return FALSE;
  }

  if (cache) TsCache_lock (cache); {
    TSMTRACE ((CACHE_MASK|1, funcName, "(%p, %d, %p, %p)",
	       cache, which, func, closure));
    /* If we found it, then move rest of list up. */
    --lst->n;
    for ( ; i < lst->n; ++i, ++cb)
      cb[0] = cb[1];

    for (i = 0, cb = lst->cb; i < lst->n; ++i, ++cb)
      TSMTRACE ((CACHE_MASK|1, funcName, "[%d] (%p, %d, %p, %p)",
		 i, cache, which, cb->func, cb->closure));
  }; if (cache) TsCache_unlock (cache);

  return TRUE;
}

int
TsCacheRemoveCallbacks (TsCache *cache, TsCacheCB which)
{
  static const char funcName[] = "TsCacheRemoveCallbacks";
  TsCallbackList *lst = (cache
			 ? &cache->callback[which]
			 : &TsCacheCallbacks[which]);

  TSMTRACE ((CACHE_MASK|2, funcName, "(%p, %d)", cache, which));
  lst->n = 0;  

  return TRUE;
}

static void
TsCacheCallCallback (TsCache *cache, TsCacheCB which, void *call)
{
  static char const funcName[] = "TsCacheCallCallback";
  TsCallbackList const *lst = &cache->callback[which];
  int i;
  TSMTRACE ((CACHE_MASK|3, funcName, "(%p, %d, %p)", cache, which, call));
  for (i = 0; i < lst->n; ++i)
    (*lst->cb[i].func) (cache, call, lst->cb[i].closure);

  lst = &TsCacheCallbacks[which];
  TSMTRACE ((CACHE_MASK|3, funcName, "(%p, %d, %p)", NULL, which, call));
  for (i = 0; i < lst->n; ++i)
    (*lst->cb[i].func) (cache, call, lst->cb[i].closure);
}

static int
tile_eql (void const *t1v, void const *t2v)
{
  TsTileId const *t1 = (TsTileId const *) t1v;
  TsTileId const *t2 = (TsTileId const *) t2v;
  return t1->x == t2->x && t1->y == t2->y;
}

static PRUint32
tile_hash (void const *t1v)
{
  TsTileId const *t1 = (TsTileId const *) t1v;
  return EQLHASH(t1->x) ^  EQLHASH(t1->y);
}

static inline TsTile *
TsCacheRef (TsCache const *cache, int x, int y, int res, int locked)
{
  /* Assume we've verified validity *before* coming here. */
  TsCacheByRes *curr = &cache->byRes[res];
  TsTile *ref;
  TsTileId id;
  id.x = x; id.y = y; id.res = res;
  if (!locked) PR_EnterMonitor (curr->monitor);
  ref = eqlhash_get (curr->tiles, &id);
  if (!locked) PR_ExitMonitor (curr->monitor);
  return ref;
}

static inline void
TsCachePut (TsCache const *cache, TsTile *tile, int locked)
{
  TsCacheByRes *curr = &cache->byRes[tile->id.res];
  if (!locked) PR_EnterMonitor (curr->monitor);
  eqlhash_put (curr->tiles, &tile->id, tile);
  if (!locked) PR_ExitMonitor (curr->monitor);
}

static inline void
TsCacheDel (TsCache const *cache, TsTile *tile, int locked)
{
  TsCacheByRes *curr = &cache->byRes[tile->id.res];
  if (!locked) PR_EnterMonitor (curr->monitor);
  eqlhash_delete (curr->tiles, &tile->id);
  if (!locked) PR_ExitMonitor (curr->monitor);
}

TsTile *
TsCacheGetTile (TsCache *cache, int x, int y, int res)
{
  static char const funcName[] = "TsCacheGetTile";
  TsTile *tile;
  TsCacheByRes *curr = &cache->byRes[res];

  PR_EnterMonitor (curr->monitor); {
    tile = TsCacheRef (cache, x, y, res, TRUE);
    if (! tile) {
      SimpleTileSet const *tset = TsCacheTileSet (cache, res);
      int i;

      /* Get an unused tile, either one previously decached or a new one. */
      tile = TsCacheAllocTile (cache);

      TSMTRACE ((CACHE_MASK|3, funcName, "(%p, %p[%d %d %d]) New", 
		 cache, tile, x, y, res));

      /* Partly initialize tile */
      tile->cache = cache;
      tile->status = TS_NOT_RESIDENT;
      tile->id.x = x; tile->id.y = y; tile->id.res = res;
      tile->id.tile_id = tsmMakeTileId (x, y, res);

      /* And the pixel extent. */
      for (i = 0; i < 2; ++i) {
	int min_px = cache->byRes[res].min_pxl[i];
	int max_px = cache->byRes[res].max_pxl[i];
	int w = (i == 0 
		 ? tset->tileTspec.numPixelsX
		 : tset->tileTspec.numPixelsY);
	int px = w * (i==0 ? x : y);
	TSMTRACE ((CACHE_MASK|3, funcName,
		   "[%d %d %d] pixels[%d] px %d+%d vs [%d,%d]",
		   x, y, res, i, px, w, min_px, max_px));
	if (px < min_px) { w -= min_px - px; px = min_px; }
	if (px + w > max_px) w = max_px - px + 1;
	TSMTRACE ((CACHE_MASK|3, funcName, "pixels[%d] = %d", i, w));
	tile->pixels[i] = w;
      }

      ALLOCTR (("%p: + %p[%d %d %d]%d mem %d\n", cache, 
		tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount,
		TsCacheMemUsed));

      /* Do this here, so we have something to work with for visibility. */
      TsCacheCallCallback (cache, TILE_INIT, tile);

      /* Create pointer to tile from cache. */
      TsCachePut (cache, tile, TRUE);
    } else {
      TSMTRACE ((CACHE_MASK|3, funcName,
		 "(%p, %p[%d %d %d]) ", cache, tile, x, y, res));
      assert (tile->id.x == x && tile->id.y == y && tile->id.res == res);
    }
  }; PR_ExitMonitor (curr->monitor);

  return tile;
}

static void
TsCacheTileDecache (TsTile *tile)
{
  static char const funcName[] = "TsCacheTileDecache";
  TsCache *cache = tile->cache;

  TSMTRACE ((CACHE_MASK|2, funcName, "(%p) Decache %p[%d %d %d]",
	     cache, tile, tile->id.x, tile->id.y, tile->id.res));

  if (cache->connect)
    TsCacheDel (cache, tile, FALSE);

  /*printf ("- %p[%d %d %d]\n", tile, tile->id.x, tile->id.y, tile->id.res);*/
  TsCacheCallCallback (cache, TILE_DECACHE, tile);
  NetLogTile( tile, TV_TILE_OUT_CACHE );

  {
    int x = tile->id.x, y = tile->id.y;
    int res = tile->id.res, refcount = tile->refcount;
    TsCacheFreeTile (tile);
    ALLOCTR (("%p: - %p[%d %d %d]%d mem %d\n", cache, 
	      tile, x, y, res, refcount,
	      TsCacheMemUsed));
  }
}

TsTile *
TsCacheTileParent_const (TsTile const *tile)
{
  TsCache const *cache = tile->cache;
  int nx = tile->id.x/2, ny = tile->id.y/2, nres = tile->id.res+1;
  return (TsCacheTileValid (cache, nx, ny, nres)
	  ? TsCacheRef (cache, nx, ny, nres, FALSE)
	  : NULL);
}


int
TsCacheTileChildren_const (TsTile const *tile, 
			   TsTile **children)
{
  TsCache const *cache = tile->cache;
  int n = 0;
  int nres = tile->id.res-1;
  if (nres >= cache->tspec->minLevel) {
    int nx = 2*tile->id.x, ny = 2*tile->id.y;
    TsCacheByRes *curr = &cache->byRes[nres];
    TsTile *cp;

    PR_EnterMonitor (curr->monitor); {
      if (TsCacheTileValid (cache, nx, ny, nres)
	  && (cp = TsCacheRef (cache, nx, ny, nres, TRUE)))
	children[n++] = cp;
      if (TsCacheTileValid (cache, nx+1, ny, nres)
	  && (cp = TsCacheRef (cache, nx+1, ny, nres, TRUE)))
	children[n++] = cp;
      if (TsCacheTileValid (cache, nx, ny+1, nres)
	  && (cp = TsCacheRef (cache, nx, ny+1, nres, TRUE)))
	children[n++] = cp;
      if (TsCacheTileValid (cache, nx+1, ny+1, nres)
	  && (cp = TsCacheRef (cache, nx+1, ny+1, nres, TRUE)))
	children[n++] = cp;
    }; PR_ExitMonitor (curr->monitor);
  }

  return n;
}

TsTile *
TsCacheTileNeighbour (TsTile const *tile, TsCacheNeighbour neigh)
{
  TsCache const *cache = tile->cache;
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int wrap_lon = tspec->wrap_lon;
  SimpleTileSet const *tset = TsCacheTileSet (cache, tile->id.res);
  int nx = tile->id.x;
  int ny = tile->id.y;
  int res = tile->id.res;

  switch (neigh) {
  TsCache_SW:
    nx = (nx == 0 && wrap_lon)? tset->maxTile[0] : nx-1;
    --ny;
    break;
  TsCache_W:
    nx = (nx == 0 && wrap_lon)? tset->maxTile[0] : nx-1;
    break;
  TsCache_NW:
    nx = (nx == 0 && wrap_lon)? tset->maxTile[0] : nx-1;
    ++ny;
    break;
  TsCache_N:
    ++ny;
    break;
  TsCache_NE:
    nx = (nx == tset->maxTile[0] && wrap_lon)? 0 : nx+1;
    ++ny;
    break;
  TsCache_E:
    nx = (nx == tset->maxTile[0] && wrap_lon)? 0 : nx+1;
    break;
  TsCache_SE:
    nx = (nx == tset->maxTile[0] && wrap_lon)? 0 : nx+1;
    --ny;
    break;
  TsCache_S:
    --ny;
    break;
  }

  return (TsCacheTileValid (cache, nx, ny, res)
	  ? TsCacheGetTile ((TsCache *) cache, nx, ny, res)
	  : 0);
}

int
TsCacheTileNeighbours (TsTile const *tile,
		       TsTile **neighbours)
{
  /*static char const funcName[] = "TsCacheTileNeighbours";*/
  int n = 0;
  TsCacheNeighbour dir;

  for (dir = TsCache_NW; dir <= TsCache_SE; ++dir) {
    TsTile *neigh = TsCacheTileNeighbour (tile, dir);
    if (neigh) ++n;
  }
  
  return n;
}


#define TILEREORDER(name,type,byteorder)				\
static void								\
name (void *raw, int span, int x0, int y0, int x1, int y1)		\
{									\
  /*static char const funcName[] = #name;*/				\
  type *p = raw, *pend = p+span*(y1+1);					\
  for (p += y0*span ; p < pend; p += span) {				\
    type *p0 = p+x0, *p0end = p+x1+1;					\
    for ( ; p0 < p0end; ++p0) {						\
      p0[0] = byteorder (p0[0]);					\
    }									\
  }									\
}

TILEREORDER(tileReorder16, PRUint16, PR_ntohs)
TILEREORDER(tileReorder32, PRUint32, PR_ntohl)

static void
tileReorder (TsCache const *cache, TsTile *tile)
{
  SimpleTileSet const *tset = TsCacheTileSet (cache, tile->id.res);
  int nbytes = tset->tileTspec.numBytesPerComponent;

  if (nbytes == 2 || nbytes == 4) {
    int tile_ix = tile->id.x * tset->tileTspec.numPixelsX;
    int tile_iy = tile->id.y * tset->tileTspec.numPixelsY;

    int min_ix = TsCacheMinPixelX (cache, tile->id.res) - tile_ix;
    int max_ix = TsCacheMaxPixelX (cache, tile->id.res) - tile_ix;
    int min_iy = TsCacheMinPixelY (cache, tile->id.res) - tile_iy;
    int max_iy = TsCacheMaxPixelY (cache, tile->id.res) - tile_iy;

    int span = (tset->tileTspec.totalPixels[0]
		* tset->tileTspec.numComponentsPixel);

    int x0 = 0;
    int x1 = span-1;
    int y0 = 0;
    int y1 = tset->tileTspec.totalPixels[1]-1;

    if (min_ix > x0) x0 = min_ix;
    if (max_ix < x1) x1 = max_ix;
    if (min_iy > y0) y0 = min_iy;
    if (max_iy < y1) y1 = max_iy;

    if (nbytes == 2)
      tileReorder16 ((void*) tile->data, span, x0, y0, x1, y1);
    else if (nbytes == 4)
      tileReorder32 ((void*) tile->data, span, x0, y0, x1, y1);
  }
}

static int
TsCacheResetTile (TsCache *cache,
		  TsmTileHeader const *tileHeader)
{
  static char const funcName[] = "TsCacheResetTile";
  int x = tsmTileHeaderX (tileHeader);
  int y = tsmTileHeaderY (tileHeader);
  int res = tsmTileHeaderLevel (tileHeader);
  TsCacheTileCB cb;

  TsTile *tile = TsCacheGetTile (cache, x, y, res);
  if (! tile) {
    tsmError (funcName, "[%d %d %d] failed", x, y, res);
    return FALSE;
  }

  if (TsCacheTileResident (tile))
    return TRUE;

  if (! TsCacheTileStartTransition (tile)) {
    TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] already in transition",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return FALSE;
  }

  /* Fill in status before we change it. */
  cb.tile = tile;
  cb.status = tile->status;

  TsCacheTileFinishTransition (funcName, tile, TS_NOT_RESIDENT);

  TsCacheCallCallback (cache, TILE_RESET, &cb);
  return TRUE;
}

static int
TsCacheTile (TsCache *cache,
	     TsmTileHeader const *tileHeader,
	     uchar const *tileBuffer,
	     PRUint32 tileSize)
{
  static char const funcName[] = "TsCacheTile";
  TsmDataSetType type = tsmTileHeaderType (tileHeader);
  int x = tsmTileHeaderX (tileHeader);
  int y = tsmTileHeaderY (tileHeader);
  int res = tsmTileHeaderLevel (tileHeader);
  TsCacheTileCB cb;

  TsTile *tile = TsCacheGetTile (cache, x, y, res);
  if (! tile) {
    tsmError (funcName, "[%d %d %d] failed", x, y, res);
    return FALSE;
  }

  /* Is this the right behaviour?  Should we substitute new data? */
  if (TsCacheTileResident (tile)) {
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "IGNORE %p[%d %d %d] buf %p. Tile resident! (status %d)",
	       tile, tile->id.x, tile->id.y, tile->id.res, tileBuffer,
	       tile->status));
    return FALSE;
  }

  /* Make sure that we're marked as being in transition. */
  if (! TsCacheTileStartTransition (tile)) {
    TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] already in transition",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return FALSE;
  }

  TSMTRACE ((CACHE_MASK|2, funcName, "(%p) %p[%d %d %d] buf %p",
	     cache, tile, tile->id.x, tile->id.y, tile->id.res, tileBuffer));

  /* Fill in the rest of the tile identity. */
  tile->id.type    = type;
  tile->id.setId   = tsmTileHeaderSetId (tileHeader);
  tile->id.sid     = tsmTileHeaderSessionId (tileHeader);
  tile->id.tile_id = tsmTileHeaderTileId (tileHeader);

  if (tile->data) {
    TSMTRACE ((CACHE_MASK|2, funcName, "<< already has data %p >>",
	       tile->data));
    tsmFreeTileBuffer ((uchar *) tile->data);
  } else {
    /* Augment the tile size. */
    TsCacheTileAddMem (tile, tileSize);
  }

  /* And the data. */
  tile->data = tileBuffer;

  /* Fill in status before we change it. */
  cb.tile = tile;
  cb.status = tile->status;

  /* Correct for network/host byte order issues. */
  tileReorder (cache, tile);

  if (! TsCacheTileFinishTransition (funcName, tile, TS_RESIDENT))
    TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] already in transition",
	       tile, tile->id.x, tile->id.y, tile->id.res));

  NetLogTile( tile, TV_TILE_IN_CACHE );
  TsCacheCallCallback (cache, TILE_CACHE, &cb);

  return TRUE;
}

typedef struct TileWaitStruct {
  PRCondVar *cond;
  TsTile const *tile;
} TileWaitStruct;

static void
TileWaitCallback (TsCache const *cache, void *arg, void *closure)
{
  /*static char const funcName[] = "TileWaitCallback";*/
  TsCacheTileCB *cb = arg;
  TileWaitStruct *wait = closure;
  
  if (wait->tile == cb->tile) {
    TsCache_lock (cache); {
      PR_NotifyCondVar (wait->cond);
    }; TsCache_unlock (cache);
  }
}

void
TsCacheTileWait (TsCache *cache, TsTile const *tile)
{
  /*static const char funcName[] = "TsCacheTileWait";*/
  TileWaitStruct wait;

  if (TsCacheTileResident (tile))
    return;

  wait.cond = PR_NewCondVar (cache->lock);
  wait.tile = tile;

  TsCacheAddCallback (cache, TILE_CACHE, TileWaitCallback, &wait, 0);

  TsCache_lock (cache); {
    if (! TsCacheTileResident (tile))
      PR_WaitCondVar (wait.cond, PR_INTERVAL_NO_TIMEOUT);
  }; TsCache_unlock (cache);

  TsCacheRemoveCallback (cache, TILE_CACHE, TileWaitCallback, &wait);
  PR_DestroyCondVar (wait.cond);
}

#if READER_THREAD
/*
 * Synopsis: void TsCacheReader (void *vcache)
 *
 * Doc++ Description: *//**
 *     Reads data from the TsmConnection associated with the specified
 *     TsCache.  This function is run as a separate thread and has the
 *     responsibility for managing the TsCache structure as new data is read.
 *
 *     Since the tsmApi now takes care of pipelining I/O requests, this
 *     function simply blocks waiting for new data in response to requests
 *     generated elsewhere and caches the returned buffers in the TsCache.
 *
 *     This process is repeated indefinitely until the thread is interrupted.
 *
 * @param state A generic pointer to a TsCache structure
 *
 * @author Lee Iverson
 *
 * @return void
 *
 * @see TsCache, tsmAllocReadNextTile
 */

static void
TsCacheReader (void *state)
{
  static char const funcName[] = "TsCacheReader";
  TsCache *cache = state;
  TsmConnection connect = cache->connect;

  TsmTileHeader *tileHeader;
  uchar *tileBuffer;
  int ret = 0;

  int tile_size;
  int errors = 0;

  /* Calculate the size of a tile buffer for burst rate monitoring */
  
  tile_size = tsmGet( connect, TSM_TILE_SIZE );

  for ( ;; ) {
    TSMTRACE ((CACHE_MASK|2, funcName, "(%p) Read next tile", cache));
    ret = tsmAllocReadNextTile (connect, &tileHeader, &tileBuffer);

    if (ret == TSM_NODATA) {
      /* This is the signal to exit. */
      if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
	TSMTRACE ((CACHE_MASK|1, funcName, "Interrupted.  Exiting thread"));
	break;
      }

      TSMTRACE ((CACHE_MASK|3, funcName, "(%p) No data -- pause %dms",
		 cache, cache->readerWait));
      tsmPause_ms (cache->readerWait);
      continue;
    } else if (! ret) {
      if (errors <= 2) {
	tsmError (funcName, "Failed to read tile (%d) Continuing...", ret);
	/*++errors;*/
	continue;
      } else {
	tsmError (funcName, "Bad connection. Shutting down.");
	TsCacheDisconnect (cache);
	break;
      }
    }

    /* feed the burst rate calculation */

    TsLogBurstRate( tile_size );

    /* log the read event */
 
    if ( TsIsLogging (TV_READER_LOG) )
      TsLogTileRead( 0 );

    /* Process tile. */
    if (! TsCacheTile (cache, tileHeader, tileBuffer, tile_size))
      tsmFreeTileBuffer (tileBuffer);

    tsmFreeTileHeader (tileHeader);
  }

 exiting:

  TSMTRACE ((CACHE_MASK|1, funcName, "Exiting"));
  return;
}
#endif

static void
TsCacheComputeTileExtent (TsCache const *cache, TsTile const *tile,
			  TsBBox *bbox)
{
  static char const funcName[] = "TsCacheComputeTileExtent";

  /* Assume we've verified tile validity *before* coming here. */
  int x = tile->id.x;
  int y = tile->id.y;
  int res = tile->id.res;

  SimpleTileSet const * tset = TsCacheTileSet (cache, res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, res);
  int w = tset->tileTspec.numPixelsX;
  int h = tset->tileTspec.numPixelsY;
  int x0 = x*w, y0 = y*h;
  int x1 = x0 + w;
  int y1 = y0 + h;

  float min_u, max_u, min_v, max_v;

  TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d] %d,%d -> %d,%d",
	     tile, tile->id.x, tile->id.y, tile->id.res,
	     x0, y0, x1, y1));
  TSMTRINC ((CACHE_MASK|2, 1));

  {
    float fx0 = x0, fy0 = y0, fx1 = x1, fy1 = y1;
    float u, v;

    /* Now calculate the min/max values as transformed. */
    min_u = max_u = u = TFM_U (fx0, fy0, ixiytouv);
    min_v = max_v = v = TFM_V (fx0, fy0, ixiytouv);
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "XY %g,%g -> UV %.1f,%.1f", fx0, fy0, u, v));
  
    u = TFM_U (fx0, fy1, ixiytouv);
    if (u > max_u) max_u = u; else if (u < min_u) min_u = u;
    v = TFM_V (fx0, fy1, ixiytouv);
    if (v > max_v) max_v = v; else if (v < min_v) min_v = v;
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "XY %g,%g -> UV %.1f,%.1f", fx0, fy1, u, v));
  
    u = TFM_U (fx1, fy0, ixiytouv);
    if (u > max_u) max_u = u; else if (u < min_u) min_u = u;
    v = TFM_V (fx1, fy0, ixiytouv);
    if (v > max_v) max_v = v; else if (v < min_v) min_v = v;
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "XY %g,%g -> UV %.1f,%.1f", fx1, fy0, u, v));
  
    u = TFM_U (fx1, fy1, ixiytouv);
    if (u > max_u) max_u = u; else if (u < min_u) min_u = u;
    v = TFM_V (fx1, fy1, ixiytouv);
    if (v > max_v) max_v = v; else if (v < min_v) min_v = v;
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "XY %g,%g -> UV %.1f,%.1f", fx1, fy1, u, v));
  }

  {
    TsBBox const *cacheBBox = TsCacheBBox (cache);

    /* Restrict extent to cacheBBox. */
    if (min_u < cacheBBox->min[0]) min_u = cacheBBox->min[0];
    if (min_v < cacheBBox->min[1]) min_v = cacheBBox->min[1];
    if (max_u > cacheBBox->max[0]) max_u = cacheBBox->max[0];
    if (max_v > cacheBBox->max[1]) max_v = cacheBBox->max[1];

    bbox->coordType = TsCoordUV;

    bbox->min[0] = min_u; bbox->min[1] = min_v;
    bbox->max[0] = max_u; bbox->max[1] = max_v;

    bbox->center[0] = (min_u+max_u)/2;
    bbox->center[1] = (min_v+max_v)/2;

    TSMTRACE ((CACHE_MASK|2, funcName,
	       "UV [%.0f,%.0f -> %.0f,%.0f] (%.0fx%.0f)",
	       min_u, min_v, max_u, max_v,
	       max_u-min_u, max_v-min_v));
  }

  /* Update the pixel extent of the tile if restricted. */
  {
    PRUint16 *pixels = (PRUint16 *) tile->pixels;
    float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
    float min_ix, max_ix, min_iy, max_iy;
    float ix, iy;
    int pxls;

    ix = TFM_IX (min_u, min_v, uvtoixiy);
    min_ix = max_ix = ix;
    iy = TFM_IY (min_u, min_v, uvtoixiy);
    min_iy = max_iy = iy;

    ix = TFM_IX (max_u, min_v, uvtoixiy);
    if (ix < min_ix) min_ix = ix;
    if (ix > max_ix) max_ix = ix;
    iy = TFM_IY (max_u, min_v, uvtoixiy);
    if (iy < min_iy) min_iy = iy;
    if (iy > max_iy) max_iy = iy;
    
    ix = TFM_IX (min_u, max_v, uvtoixiy);
    if (ix < min_ix) min_ix = ix;
    if (ix > max_ix) max_ix = ix;
    iy = TFM_IY (min_u, max_v, uvtoixiy);
    if (iy < min_iy) min_iy = iy;
    if (iy > max_iy) max_iy = iy;
    
    ix = TFM_IX (max_u, max_v, uvtoixiy);
    if (ix < min_ix) min_ix = ix;
    if (ix > max_ix) max_ix = ix;
    iy = TFM_IY (max_u, max_v, uvtoixiy);
    if (iy < min_iy) min_iy = iy;
    if (iy > max_iy) max_iy = iy;

    max_ix = ceil (max_ix);
    if (max_ix < TsCacheMaxPixelX (cache, res))
      max_ix = TsCacheMaxPixelX (cache, res);
    min_ix = floor (min_ix);
    if (min_ix < TsCacheMinPixelX (cache, res))
      min_ix = TsCacheMinPixelX (cache, res);
    pxls = max_ix - min_ix + 1;
    if (pxls < pixels[0]) pixels[0] = pxls;

    max_iy = ceil (max_iy);
    if (max_iy < TsCacheMaxPixelY (cache, res))
      max_iy = TsCacheMaxPixelY (cache, res);
    min_iy = floor (min_iy);
    if (min_iy < TsCacheMinPixelY (cache, res))
      min_iy = TsCacheMinPixelY (cache, res);
    pxls = max_iy - min_iy + 1;
    if (pxls < pixels[1]) pixels[1] = pxls;

    TSMTRACE ((CACHE_MASK|2, funcName, "%dx%d pixels", pixels[0], pixels[1]));
    TSMTRACE ((CACHE_MASK|2, funcName, "UV [%.1f,%.1f -> %.1f,%.1f]",
	       TFM_U (min_ix, min_iy, ixiytouv),
	       TFM_V (min_ix, min_iy, ixiytouv),
	       TFM_U (max_ix+1, max_iy+1, ixiytouv),
	       TFM_V (max_ix+1, max_iy+1, ixiytouv)));
    TSMTRINC ((CACHE_MASK|2, -1));
  }
}

void
TsCacheTileExtent (TsTile *tile)
{
  /* Only do this once. */
  if (tile->bbox.center[0] == 0.0)
    TsCacheComputeTileExtent (tile->cache, tile, &tile->bbox);
}

static void
TsCacheTileExtentCB (TsCache const *cache, void *arg, void *closure)
{
  if (cache->tspec) {
    TsTile *tile = arg;
    TsCacheTileExtent (tile);
  }
}

void
TsBBoxInit (TsBBox *bbox)
{
  /*static const char funcName[] = "TsBBoxInit";*/
  float *min = bbox->min, *max = bbox->max;
  min[0] = min[1] = min[2] =  1e38;
  max[0] = max[1] = max[2] = -1e38;
}

int
TsBBoxAddPoint (TsBBox *bbox, float x, float y, float z)
{
  /*static const char funcName[] = "TsBBoxAddPoint";*/
  float *min = bbox->min, *max = bbox->max;
  int changed = 0;
  if (x < min[0]) { min[0] = x; ++changed; }
  if (x > max[0]) { max[0] = x; ++changed; }
  if (y < min[1]) { min[1] = y; ++changed; }
  if (y > max[1]) { max[1] = y; ++changed; }
  if (z < min[2]) { min[2] = z; ++changed; }
  if (z > max[2]) { max[2] = z; ++changed; }
  return changed;
}

int
TsBBoxAddBBox (TsBBox *bbox, TsBBox const *comp)
{
  /*static const char funcName[] = "TsBBoxAddBBox";*/
  float *min = bbox->min, *max = bbox->max;
  float const *comp_min = comp->min, *comp_max = comp->max;
  int i;
  int changed = 0;
  for (i = 0; i < 3; ++i) {
    if (comp_min[i] < min[i]) { min[i] = comp_min[i]; ++changed; }
    if (comp_max[i] > max[i]) { max[i] = comp_max[i]; ++changed; }
  }
  return changed;
}

void
TsBBoxFinish (TsBBox *bbox)
{
  static const char funcName[] = "TsBBoxFinish";
  int i;
  double sum2 = 0;

  TSMTRACE ((CACHE_MASK|2, funcName, "%.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
	     bbox->min[0], bbox->min[1], bbox->min[2],
	     bbox->max[0], bbox->max[1], bbox->max[2]));

  for (i = 0; i < 3; ++i) {
    double min = bbox->min[i];
    double max = bbox->max[i];
    double d = max - min;
    bbox->center[i] = (max + min)/2;
    sum2 += d*d;
  }
  
  bbox->radius = sqrt (sum2)/2;
  TSMTRACE ((CACHE_MASK|2, funcName, "center %.0f,%.0f,%.0f radius %.0f",
	     bbox->center[0], bbox->center[1], bbox->center[2], bbox->radius));
}

/*
 * Synopsis:
 *   float TsBBoxDist2 (TsBBox const *bbox, float const p[])
 *
 * Doc++ Description: *//** 
 *   Determine the distance from a point to an axis-aligned bounding box in
 *   the same local coordinate system.  Returns the square of the distance.
 *
 *   This calculation gives an accurate distance to the closest point on the
 *   surface of the box, and zero if the point is inside.
 *
 *   For each axis, we check whether the point's coordinate is "inside" the
 *   (min,max) bounds for that axis.  The distance is derived from the sum of
 *   the squared distances to the min/max coordinates which are "outside".
 *
 * @param    bbox	An axis-aligned bounding box.
 * @param    p		A point in the same coordinate system.
 *
 * @return   The squared distance from p to the surface of the box.
 *
 * @author   Lee Iverson
 * @version  January 2001
 *
 * @see      TsBBox
 *
 */

float
TsBBoxDist2 (TsBBox const *bbox, float const p[])
{
  static char const funcName[] = "TsBBoxDist2";
  float const *center = bbox->center;
  float const *corner[2];
  int not_in = 0;
  float d2 = 0;
  int i;

  corner[0] = bbox->min;
  corner[1] = bbox->max;

  for (i = 0; i < 3; ++i) {
    float px = p[i];
    int idx = px > center[i];
    float bx = corner[idx][i];
    float ox = corner[!idx][i];
    float d = px - bx;
    TSMTRACE ((VIS_MASK|2, funcName, "[%d] ex %.0f [%.0f < %.0f < %.0f] %.0f",
	       i, px, bx, center[i], ox, d));
    if (idx ? (d > 0 || px < ox) : (d < 0 || px > ox)) {
      d2 += d*d;
      TSMTRACE ((VIS_MASK|2, funcName, "[%d] !inside d %.0f", i, d));
      ++not_in;
    }
  }

  if (not_in == 0) {
    /* We are inside the bbox. */
    TSMTRACE ((VIS_MASK|2, funcName, "<= inside"));
    return 0.0;
  } else {
    /* Otherwise, our closest point is the corner. */
    TSMTRACE ((VIS_MASK|2, funcName, "<= %s %.0f",
	       not_in==3 ? "corner" : "edge", sqrt (d2)));
    return d2;
  }
}

void
TsBBoxPrint (TsBBox *bbox)
{
  if ( ! bbox ) return;
  switch ( bbox->coordType ) {
  case TsCoordUV:
    fprintf( stderr, "UV: " );
    break;
  case TsCoordLVCS:
    fprintf( stderr, "LVCS: " );
    break;
  case TsCoordGCC:
    fprintf( stderr, "GCC: " );
    break;
  case TsCoordGDC:
    fprintf( stderr, "GDC: " );
    break;
  }
  fprintf( stderr, "center %f, %f, %f  radius %f\n", bbox->center[0],
	   bbox->center[1], bbox->center[2], bbox->radius );
}
