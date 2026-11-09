/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsCacheInline.h,v 2.23.2.4 2002/10/31 05:26:38 reddy Exp $

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

#include <prthread.h>
#include <prlock.h>
#include <prcvar.h>
#include <pratom.h> 
#include <prinrval.h>

#include "TsCacheI.h"

#include "TsLogging.h"

#include <tsm/tsmTile.h>

extern void TsCacheLRUFixup (TsTile const *tile);

#define TsCache_lock(cache) PR_Lock ((cache)->lock);
#define TsCache_unlock(cache) PR_Unlock ((cache)->lock);

exinline TsmConnectionParams
TsCacheConnectionParams (TsCache const *cache)
{
  return cache->params;
}

exinline TileSetTspec const *
TsCacheTspec (TsCache const *cache)
{
  return cache->tspec;
}

exinline SimpleTileSet const *
TsCacheTileSet (TsCache const *cache, int res)
{
  return cache->byRes[res].tset;
}

exinline TsBBox const *
TsCacheBBox (TsCache const *cache)
{
  return &cache->bbox;
}

exinline double const *
TsCacheOrigin (TsCache const *cache)
{
  return cache->origin;
}

exinline float const *
TsCacheTfm_ixiytouv (TsCache const *cache, int res)
{
  return cache->byRes[res].ixiytouv;
}

exinline float const *
TsCacheTfm_uvtoixiy (TsCache const *cache, int res)
{
  return cache->byRes[res].uvtoixiy;
}

exinline PRUint32
TsCacheMinPixelX (TsCache const *cache, int res)
{
  return cache->byRes[res].min_pxl[0];
}

exinline PRUint32
TsCacheMaxPixelX (TsCache const *cache, int res)
{
  return cache->byRes[res].max_pxl[0];
}

exinline PRUint32
TsCacheMinPixelY (TsCache const *cache, int res)
{
  return cache->byRes[res].min_pxl[1];
}

exinline PRUint32
TsCacheMaxPixelY (TsCache const *cache, int res)
{
  return cache->byRes[res].max_pxl[1];
}

exinline TsmConnection
TsCacheConnection (TsCache const *cache)
{
  return cache->connect;
}

exinline int
TsCacheTileValid (TsCache const *cache, int x, int y, int res)
{
  SimpleTileSet const *tset;
  return (res >= cache->tspec->minLevel
	  && res <= cache->tspec->maxLevel
	  && (tset = TsCacheTileSet (cache, res),
	      x >= tset->minTile[0]
	      && x <= tset->maxTile[0]
	      && y >= tset->minTile[1]
	      && y <= tset->maxTile[1]));
}

exinline void
TsCacheTileRef_ (char const *funcName, TsTile const *tile)
{
  TSMTRACE ((CACHE_MASK|4, funcName, "REF %p [%d %d %d] %d",
						 tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount));
  if (PR_AtomicIncrement (&((TsTile *)tile)->refcount) == 1)
    TsCacheLRUFixup (tile);
}

exinline void
TsCacheTileDeref_ (char const *funcName, TsTile const *tile)
{
  TSMTRACE ((CACHE_MASK|4, funcName, "DEREF %p [%d %d %d] %d",
						 tile, tile->id.x, tile->id.y, tile->id.res, tile->refcount));
  if (PR_AtomicDecrement (&((TsTile *)tile)->refcount) == 0)
    TsCacheLRUFixup (tile);
}

exinline void
TsCacheTileAddMem (TsTile *tile, PRInt32 size)
{
  tile->size += size;
  TsCacheMemUsed += size;
}

exinline int
TsCacheTileResident (TsTile const *tile)
{
  return (tile->status <= TS_RESIDENT);
}

exinline int
TsCacheTilePrepared (TsTile const *tile)
{
  return (tile->status <= TS_PREPARED);
}

exinline TsTile *
TsCacheTileParent (TsTile const *tile)
{
  TsCache *cache = tile->cache;
  int nx = tile->id.x/2, ny = tile->id.y/2, nres = tile->id.res+1;
  return (TsCacheTileValid (cache, nx, ny, nres)
	  ? TsCacheGetTile (cache, nx, ny, nres)
	  : NULL);
}

exinline int
TsCacheTileChildren (TsTile const *tile, 
		     TsTile **children)
{
  TsCache *cache = tile->cache;
  int n = 0;
  int nx = 2*tile->id.x, ny = 2*tile->id.y, nres = tile->id.res-1;

  if (TsCacheTileValid (cache, nx, ny, nres))
    children[n++] = TsCacheGetTile (cache, nx, ny, nres);
  if (TsCacheTileValid (cache, nx+1, ny, nres))
    children[n++] = TsCacheGetTile (cache, nx+1, ny, nres);
  if (TsCacheTileValid (cache, nx, ny+1, nres))
    children[n++] = TsCacheGetTile (cache, nx, ny+1, nres);
  if (TsCacheTileValid (cache, nx+1, ny+1, nres))
    children[n++] = TsCacheGetTile (cache, nx+1, ny+1, nres);
  
  return n;
}

exinline int
TsCacheTileStartTransition (TsTile *tile)
{
  if (PR_AtomicSet (&tile->transition, 1) != 0)
    return FALSE;
  return TRUE;
}

extern char const *TsCacheTileStatusName[];

exinline int
TsCacheTileFinishTransition (char const *funcName,
														 TsTile *tile, TsTileStatus tilestatus)
{
	TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] %s",
						 tile, tile->id.x, tile->id.y, tile->id.res,
						 TsCacheTileStatusName[tilestatus])); 
  tile->status = tilestatus;
  return PR_AtomicSet (&tile->transition, 0);
}

exinline int
TsCacheRequestTile (char const *funcName,
										tsmRequestList *req, TsTile *tile, int priority)
{
  /* Don't request tile if already requested. */
  if (! tile->cache->connect
      || TsCacheTileResident (tile)
      || tile->transition)
    return FALSE;

	if (! TsCacheTileStartTransition (tile)) {
		TSMTRACE ((CACHE_MASK|1, funcName,
							 "%p[%d %d %d] can't start transition",
							 tile, tile->id.x, tile->id.y, tile->id.res));
		TSMTRINC ((CACHE_MASK|2, -1));
		return FALSE;
	}

  tsmRequestAdd (req, tile->id.x, tile->id.y, tile->id.res, priority);
  TsCacheTileFinishTransition (funcName, tile, TS_REQUESTED);

  return TRUE;
}

exinline int
TsCacheRequest (TsCache *cache, tsmRequestList *req,
		int x, int y, int res, int priority)
{
	static char const funcName[] = "TsCacheRequest";
  TsTile *tile = TsCacheGetTile (cache, x, y, res);
  return TsCacheRequestTile (funcName, req, tile, priority);
}

exinline void
TsCacheLogRequests (TsCache *cache)
{
  PRIntervalTime begin, end;
  int pri1, pri2, pri3;
  tsmTileReqInfo( cache->connect, &begin, &end, &pri1, &pri2, &pri3 );
  TsLogRequestListData( begin, end, pri1, pri2, pri3 );
}
