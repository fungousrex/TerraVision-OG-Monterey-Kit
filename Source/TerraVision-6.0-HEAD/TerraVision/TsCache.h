/* -*- tab-width: 8; -*-

  Revision History:
      $Id: TsCache.h,v 2.33.2.7 2002/10/31 05:26:37 reddy Exp $

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

#ifndef _TS_CACHE_H_
#define _TS_CACHE_H_

#include <tsm/tsm.h>

#include <pthr/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TsCache TsCache;

/*
 * Doc++ Description: *//** 
 *   Enumerated values describing status of a given tile
 *
 *   If tsmReadNextTileData has returned data for the given tile, then
 *   its status is TS_RESIDENT otherwise it is TS_NOT_RESIDENT.  If the
 *   tile has been requested and is currently on the cache's request
 *   list then it has a value >= 0 which is its index into the request list.
 *
 * @param TS_PREPARED      The tile's data has been received and initialized.
 * @param TS_RESIDENT      The tile's data has been received.
 * @param TS_NOT_RESIDENT  The tile's data has not been received or requested.
 *
 * @see TsTile
 *
 */

typedef enum {
  TS_PREPARED,       /* Prepared for external use */
  TS_RESIDENT,       /* Resident in cache */
  TS_NOT_RESIDENT,   /* Not resident in cache */
  TS_REQUESTED       /* Requested */
} TsTileStatus;

/*
 * Doc++ Description: *//** 
 *   Unique tile identifier.
 *
 *   A tile identification marker associated with each tile.  It
 *   uniquely identifies a tile and its type.
 *
 * @param x        The tile's x coordinate at its pyramid level.
 * @param y        The tile's y coordinate at its pyramid level.
 * @param res      The tile's resolution or pyramid level.
 * @param setId    A unique dataset identifier.
 * @param sid      A unique session identifier.
 * @param tile_id  A unique tile identifier.
 * @param type     The type of data encapsulated.
 *
 * @see TsTile
 */ 

typedef struct TsTileId {
  int x;            /* X coordinate of tile */
  int y;            /* Y coordinate of tile */
  int res;          /* Resolution number of the tile */
  int setId;        /* Set Id of tile */
  int sid;          /* Session ID */
  int tile_id;      /* Unique tile id */
  TsmDataSetType type;  /* Type of tile */
} TsTileId;

/*
 * Doc++ Description: *//** 
 *   The bounding box of a Tile object.
 *
 *   A description of the bounding box of a tile in the dataset's
 *   native coordinate system.  It is filled in incrementally, so may
 *   not be valid until the radius is non-zero.
 *
 * @param min     The minimal coordinates of the box.
 * @param max     The maximal coordinates of the box.
 * @param center  The center of the bounding sphere.
 * @param radius  The radius of the bbox.  (Non-zero means valid).
 *
 * @see TsTile
 */ 

typedef enum { TsCoordUV, TsCoordLVCS, TsCoordGCC, TsCoordGDC } TsCoordType;

typedef struct TsBBox {
  TsCoordType coordType;
  float min[3];
  float max[3];
  float center[3];
  float radius;
} TsBBox;

/*
 * Doc++ Description: *//** 
 *   Neighbourhood specifier within a TsCache
 *
 *   A descriptor to select neighbouring elements in a TsCache
 *
 * @param TsCache_NW  Northwest neighbour.
 * @param TsCache_N   North neighbour.
 * @param TsCache_NE  Northeast neighbour.
 * @param TsCache_E   East neighbour.
 * @param TsCache_SE  Southeast neighbour.
 * @param TsCache_S   South neighbour.
 * @param TsCache_SW  Southwest neighbour.
 * @param TsCache_W   West neighbour.
 *
 * @see TsCacheGetNeighbour
 *
 */

typedef enum {
  TsCache_NW, TsCache_N, TsCache_NE,
  TsCache_W,  TsCache_C, TsCache_E,
  TsCache_SW, TsCache_S, TsCache_SE
} TsCacheNeighbour;
#define TsCacheNeighbor TsCacheNeighbour

/*
 * Doc++ Description: *//** 
 *   A tile object.
 *
 *   A generic tile, including identification, data and type-specific
 *   information.
 *
 * @param cache      The TsCache of which this tile is a member.
 * @param id         The TsTileId of the tile (identifies its type).
 * @param status     The tile status or location on request list.
 * @param transition Non-zero only if in transition from one state to another.
 * @param bbox       The bounding box enclosing the tile.
 * @param size       The memory footprint of this tile.
 * @param pixels     The pixel size of this tile.
 * @param refcount   A reference count.
 * @param lru_iter   A reference to the tile's entry on the LRU list.
 * @param data       A generic pointer to data from tsmGetNextTileData().
 * @param aux        Arbitrary type-specific data.
 *
 */ 

typedef struct TsTile {
  TsCache *    cache;
  TsTileStatus status;
  PRInt32      transition;
  TsTileId     id;
  TsBBox       bbox;
  PRUint32     size;
  PRUint16     pixels[2];
  PRInt32      refcount;
  list_iter_t  lru_iter;
  void const * data;
  void *       aux;
} TsTile;

extern PRUint32 TsCacheMemUsed; 
extern PRUint32 TsCacheMemReserved;

TsCache *TsCacheCreate (char const *url, TsmConnectionParams params);

int TsCacheConnect (TsCache *cache);
void TsCacheDisconnect (TsCache *cache);

void TsCacheDestroy (TsCache *cache);

void TsCacheClearAll (void);

TsmConnectionParams TsCacheConnectionParams (TsCache const *cache);
TileSetTspec const *TsCacheTspec (TsCache const *cache);
SimpleTileSet const *TsCacheTileSet (TsCache const *cache, int res);
double const *TsCacheOrigin (TsCache const *cache);
TsmConnection TsCacheConnection (TsCache const *cache);
TsBBox const * TsCacheBBox (TsCache const *cache);

void TsCacheSetExtent (TsCache *cache,
		       float min_u, float min_v, float max_u, float max_v);

float const *TsCacheTfm_ixiytouv (TsCache const *cache, int res);
float const *TsCacheTfm_uvtoixiy (TsCache const *cache, int res);

void TsCacheTileWait (TsCache *cache, TsTile const *tile);

int TsCacheTileValid (TsCache const *cache, int x, int y, int res);

TsTile *TsCacheGetTile (TsCache *cache, int x, int y, int res);

tsmRequestList *TsCacheRequestList (TsCache *cache);
int TsCacheRequestTile (char const *funcName,
			tsmRequestList *req, TsTile *tile, int priority);
int TsCacheRequest (TsCache *cache, tsmRequestList *req,
		    int x, int y, int res, int priority);

TsTile *TsCacheTileParent (TsTile const *tile);
int TsCacheTileChildren (TsTile const *tile, 
			 TsTile **children);

TsTile *TsCacheTileParent_const (TsTile const *tile);
int TsCacheTileChildren_const (TsTile const *tile, TsTile **children);

TsTile *TsCacheTileNeighbour (TsTile const *tile,
			      TsCacheNeighbour neigh);

int TsCacheTileNeighbours (TsTile const *tile, TsTile **neighbours);

extern char const * const TsCacheNeighbour_name[];
extern TsCacheNeighbour const TsCacheNeighbour_reverse[];

void TsCacheTileExtent (TsTile *tile);

#define TsCacheTileRef(tile) TsCacheTileRef_ (funcName, tile)
#define TsCacheTileDeref(tile) TsCacheTileDeref_ (funcName, tile)

void TsCacheTileRef_ (char const *funcName, TsTile const *tile);
void TsCacheTileDeref_ (char const *funcName, TsTile const *tile);

int TsCacheTileResident (TsTile const *tile);
int TsCacheTilePrepared (TsTile const *tile);

void TsCacheTileAddMem (TsTile *tile, PRInt32 size);

void TsBBoxInit (TsBBox *bbox);
int TsBBoxAddPoint (TsBBox *bbox, float x, float y, float z);
int TsBBoxAddBBox (TsBBox *bbox, TsBBox const *comp);
void TsBBoxFinish (TsBBox *bbox);
void TsBBoxPrint (TsBBox *bbox);

float TsBBoxDist2 (TsBBox const *bbox, float const p[]);

typedef struct TsCallback TsCallback;
typedef void (*TsCallbackF) (TsCache const *cache, void *arg, void *closure);

typedef enum {
  TILE_INIT, TILE_CACHE, TILE_DECACHE, TILE_RESET,
  CACHE_NCALLBACKS
} TsCacheCB;

typedef struct TsCacheTileCB {
  TsTile *tile;
  TsTileStatus status;
} TsCacheTileCB;

int TsCacheAddCallback (TsCache *cache, TsCacheCB which,
			TsCallbackF func, void *closure,
			void (*destroyf) (void*));
int TsCacheRemoveCallback (TsCache *cache, TsCacheCB which,
			   TsCallbackF func, void *closure);
int TsCacheRemoveCallbacks (TsCache *cache, TsCacheCB which);

#define TFM_U(x,y,ixiytoUV) ((x)*ixiytoUV[0] + (y)*ixiytoUV[1] + ixiytoUV[2])
#define TFM_V(x,y,ixiytoUV) ((x)*ixiytoUV[3] + (y)*ixiytoUV[4] + ixiytoUV[5])
#define TFM_DU(dx,dy,ixiytoUV) ((dx)*ixiytoUV[0] + (dy)*ixiytoUV[1])
#define TFM_DV(dx,dy,ixiytoUV) ((dx)*ixiytoUV[3] + (dy)*ixiytoUV[4])

#define TFM_IX(u,v,uvtoIXIY) ((u)*uvtoIXIY[0] + (v)*uvtoIXIY[1] + uvtoIXIY[2])
#define TFM_IY(u,v,uvtoIXIY) ((u)*uvtoIXIY[3] + (v)*uvtoIXIY[4] + uvtoIXIY[5])
#define TFM_DX(du,dv,uvtoIXIY) ((du)*uvtoIXIY[0] + (dv)*uvtoIXIY[1])
#define TFM_DY(du,dv,uvtoIXIY) ((du)*uvtoIXIY[3] + (dv)*uvtoIXIY[4])

#if defined(HAVE_INLINE)
# include <pthr/istack.h>
# include <pthr/eqlhash.h>
# if !defined(NO_TsCache_INLINE)
#  ifdef __GNUC__
#   define exinline extern inline
#  else
#   define exinline inline
#  endif
#  include "TsCacheInline.h"
#  undef exinline
# endif
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _TS_CACHE_H_ */
