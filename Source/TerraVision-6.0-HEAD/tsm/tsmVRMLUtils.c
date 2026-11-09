/*
 * Name: tsmVRMLUtils.c
 *
 * Description: 
 *   Contains routines used in the creation of TerraVRML datasets.
 *
 * Function List:
 *
 * Revision History:
 *   Created 27 April 1998 by Nat Bletter
 *
 *   $Id: tsmVRMLUtils.c,v 1.27.2.1 2001/06/19 22:04:50 leei Exp $
 *
 * License Agreement:
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

#include "tsmInternal.h"
#include "tsmVRMLUtils.h"
#include "tsmConnectionParamsInt.h"

#ifndef M_PI
#define M_PI (3.1415926535)
#endif

#define PROJ_F              (double) 2.41421356237309490   /* 1 / tan(45/2) */
#define PROJ_SCR            (double) 512.0             /* min(width,height) */

#define VRML_CACHE_SIZE     50

static TsmCachedTile *vrml_cache[VRML_CACHE_SIZE] = { NULL };
static int           vrml_cache_size = 0;
static int           vrml_cache_del = 0;
static int           vrml_cache_ref = 0;

double tsm_temp_coord;  /* used by the TSM_MAKE_Y_UP macro in tsmVRMLUtils.h */

/*
 * Synopsis:
 *    int tsmVRMLCacheInit( int )
 *    int tsmVRMLCacheFree( void )
 *
 * Description:
 *    These two functions are used to initialise and free the tile cache.
 *    You must initialise the cache before using it. If not, then no
 *    caching will occur. The boolean argument to the initialise function
 *    used to say that you don't want to initialise the cache if it is 
 *    already initialised (TRUE), or to always re-initialise (FALSE).
 *
 *    We reference count the number of times the init function is called
 *    only only perform the actual cache freeing on the last free function
 *    call, i.e. you can nest pairs of these functions.
 *
 * Author: Martin Reddy
 *
 * Date: 17 June 1998
 *
 */

int
tsmVRMLCacheInit( void )
{
  int i;

  /* return FALSE; */  /* uncomment this line to turn off caching */

  if ( ++vrml_cache_ref > 1 ) return FALSE;

  /* free all previously allocated cached tiles */

  for ( i = 0; i < vrml_cache_size; ++i )
    if ( vrml_cache[i] != NULL )
      tsmMemFree( vrml_cache[i] );

  /* initialise all available cells to NULL */

  for ( i = 0; i < VRML_CACHE_SIZE; ++i )
    vrml_cache[i] = NULL;

  vrml_cache_size = 0;
  vrml_cache_del  = 0;

  return TRUE;
}

int
tsmVRMLCacheFree( void )
{
  int i;

  if ( vrml_cache_ref == 0 ) return FALSE;
  if ( --vrml_cache_ref > 0 ) return FALSE;

  for ( i = 0; i < vrml_cache_size; ++i ) {
    if ( vrml_cache[i] != NULL ) {
      if ( vrml_cache[i]->buffer != NULL )
	tsmMemFree( vrml_cache[i]->buffer );
      tsmMemFree( vrml_cache[i] );
    }
  }

  vrml_cache_size = 0;

  return TRUE;
}

/*
 * Synopsis:
 *   static int tsmVRMLCacheKey( int tx, int ty, int level )
 *
 * Description:
 *   Calculates a search key value for a tile given it's
 *   coordinates and resolution. This will be unique for
 *   most tiles, but uniqueness should not be relied upon.
 *
 * Author: Martin Reddy
 *
 * Date: 17 June 1998
 *
 */

static int
tsmVRMLCacheKey( int tx, int ty, int level )
{
  return ( tx << 18 ) | ( ty << 6 ) | level;
}

/*
 * Synopsis:
 *   static TsmCachedTile *tsmVRMLCacheGet( int tx, int ty, int level )
 *   int TsmCachedTile *tsmVRMLCacheGetBBox( int tx, int ty, int level )
 *   int TsmCachedTile *tsmVRMLCacheGetBuffer( int tx, int ty, int level )
 *
 * Description:
 *   Routines to return the TsmCachedTile structure from the cache for
 *   the specified tile. These will return NULL if the tile does not
 *   exist in the cache. tsmVRMLCacheGetBBox will also return NULL if
 *   the tile exists but its size[0] field is zero (i.e. no bounding box
 *   defined). Similarly, tsmVRMLCacheGetBuffer will return NULL if the
 *   tile exists, but its buffer field is NULL.
 *
 * Author: Martin Reddy
 *
 * Date: 17 June 1998
 *
 */

static TsmCachedTile *
tsmVRMLCacheGet( int tx, int ty, int level )
{
  int i, key = tsmVRMLCacheKey( tx, ty, level );
  TsmCachedTile *bbox;

  if ( vrml_cache_ref == 0 ) return NULL;

  for ( i = 0; i < vrml_cache_size; ++i ) {
    bbox = vrml_cache[i];
    if ( bbox->key == key ) {

      /* the key will be unique for most tile coordinates, but let's */
      /* just check to make sure that we have the right tile first.  */

      if ( bbox->tx == tx && bbox->ty == ty && bbox->level == level )
	return bbox;
    }
  }

  return NULL;
}

TsmCachedTile *
tsmVRMLCacheGetBBox( int tx, int ty, int level )
{
  TsmCachedTile *bbox;

  if ( ( bbox = tsmVRMLCacheGet( tx, ty, level ) ) == NULL ) return NULL;
  if ( bbox->size[0] == 0.0 ) return NULL;
  return bbox;
}

TsmCachedTile *
tsmVRMLCacheGetBuffer( int tx, int ty, int level )
{
  TsmCachedTile *bbox;

  if ( ( bbox = tsmVRMLCacheGet( tx, ty, level ) ) == NULL ) return NULL;
  if ( bbox->buffer == NULL ) return NULL;
  return bbox;
}

/*
 * Synopsis:
 *   static int tsmVRMLCacheAdd( TsmCachedTile *bbox )
 *   int tsmVRMLCacheAddBBox( TsmCachedTile *bbox )
 *   int tsmVRMLCacheAddBuffer( TsmCachedTile *bbox )
 *
 * Description:
 *   Routines to add a new TsmCachedTile structure for a tile into the
 *   cache. tsmVRMLCacheAddBBox will first check to see if the tile already
 *   exists in the cache - if it does then it only updates the bounding box
 *   information stored in the cache. Similarly, tsmVRMLCacheAddBuffer will
 *   first check is the tile exists in the cache and only update the buffer
 *   field of this tile if it does exist (freeing any previous buffer memory).
 *
 * Author: Martin Reddy
 *
 * Date: 17 June 1998
 *
 */

static int
tsmVRMLCacheAdd( TsmCachedTile *bbox )
{
  if ( bbox == NULL || vrml_cache_ref == 0 ) return FALSE;

  /* Calculate the lookup key for this tile */

  bbox->key = tsmVRMLCacheKey( bbox->tx, bbox->ty, bbox->level );

  /* Check to see if we have a full cache */

  if ( vrml_cache_size == VRML_CACHE_SIZE ) {
    
    /* remove the oldest tile from the cache (free its memory) */

    if ( vrml_cache[vrml_cache_del] != NULL ) {
      if ( vrml_cache[vrml_cache_del]->buffer != NULL )
	tsmMemFree( vrml_cache[vrml_cache_del]->buffer );
      tsmMemFree( vrml_cache[vrml_cache_del] );
    }

    /* put the new tile info in its place */

    vrml_cache[vrml_cache_del] = bbox;
    vrml_cache_del = ( vrml_cache_del + 1 ) % VRML_CACHE_SIZE;

    return TRUE;
  }

  /* The cache has space, so just add the entry to the end. This  */
  /* is pretty inefficient and a key-sorted array would be better */

  vrml_cache[vrml_cache_size] = bbox;
  vrml_cache_size++;
  return TRUE;
}

int
tsmVRMLCacheAddBBox( TsmCachedTile *bbox )
{
  TsmCachedTile *curr;

  /* If we don't have this tile cached, then cache it */

  if ( ( curr = tsmVRMLCacheGet( bbox->tx, bbox->ty, bbox->level ) ) == NULL )
    return tsmVRMLCacheAdd( bbox );

  /* If we have already cached info for this tile */
  /* then only replace the bounding box fields    */

  memcpy( curr->size, bbox->size, sizeof(double) * 3 );
  memcpy( curr->mid, bbox->mid, sizeof(double) * 3 );
  memcpy( curr->ll, bbox->ll, sizeof(double) * 3 );
  memcpy( curr->ur, bbox->ur, sizeof(double) * 3 );
  return TRUE;
}

int
tsmVRMLCacheAddBuffer( TsmCachedTile *bbox )
{
  TsmCachedTile *curr;

  /* If we don't have this tile cached, then cache it */

  if ( ( curr = tsmVRMLCacheGet( bbox->tx, bbox->ty, bbox->level ) ) == NULL )
    return tsmVRMLCacheAdd( bbox );

  /* If we have already cached info for this tile */
  /* then only replace the buffer field.          */

  if ( curr->buffer != NULL ) tsmMemFree( curr->buffer );
  curr->buffer = bbox->buffer;
  return TRUE;
}

/*
 * Synopsis:
 *    ulong tsmReadDEMTile( TsmConnection , int, int, int )
 *
 * Description:
 *    Reads the specified tile from the DEM pyramid. This employs a
 *    crude caching functionality whereby if you request the same tile
 *    as the last call, then the last buffer is returned immediately and
 *    no disk/network access is done.
 *
 *    Uses statics - not thread safe.
 *
 * Returns: the tile buffer, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 27 January 1998
 *
 */

static uchar *
tsmReadDEMTile( TsmConnection demConn, int tx, int ty, int level )
{
  const char    *funcName = "tsmReadDEMTile";
  SimpleTileSet *tileset;
  uchar         *tileBuffer = NULL;
  Pyramid       *demPyramid;
  TsmCachedTile *cached_tile;

  /* Check to see if the DEM buffer data for this tile have */
  /* already been cached. If so, then just return the info. */

  if ( ( cached_tile = tsmVRMLCacheGetBuffer( tx, ty, level ) ) != NULL )
    return cached_tile->buffer;

  demPyramid = (Pyramid *)tsmGet(demConn, TSM_DATASET_TSPEC);

  /* Check that we have valid tile coordinates for the level */

  if ( demPyramid == NULL || level > demPyramid->maxLevel )
    return NULL;

  tileset = &demPyramid->tspecLevel.val[level];
  
  if ( tx > tileset->maxTile[0] || ty > tileset->maxTile[1] )
    return NULL;

  /* Allocate a new cache entry structure to fill in the buffer field */

  cached_tile = (TsmCachedTile *) tsmMemMalloc( sizeof(TsmCachedTile) );
  if ( cached_tile == NULL ) return NULL;

  /* Allocate a tile buffer for the DEM pyramid tiles */

  if ( ( tileBuffer = tsmAllocTileBuffer( demConn ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a tile buffer" );
    return NULL;
  }

  /* read in the specified tile from the DEM pyramid */

  if ( ! tsmReadTile( demConn, tx, ty, level, tileBuffer) ) {
    tsmError( funcName, "ERROR: Could not get the tile!" );
    return NULL;
  }

  /* Add this tile to the cache and return it */

  cached_tile->tx      = tx;
  cached_tile->ty      = ty;
  cached_tile->level   = level;
  cached_tile->size[0] = 0.0;
  cached_tile->buffer  = tileBuffer;
  tsmVRMLCacheAddBBox( cached_tile );

  return tileBuffer;
}


void
tsmLCSOrigin( Pyramid *tspec, int level, double orig[2],
	      TsmConnectionParams params )
{
  int i;
  double tileSize;
  
  tileSize = tspec->tileTspec.numPixelsX *
    tspec->tspecLevel.val[level].imageTransformationTspec.ixiytoUV[0];

  for (i = 0;i < params->num_lcs; i++) {
    if (tileSize > params->lcs[i])
      break;
  }
  i -= 1;
}

/*
 * Synopsis:
 *    void tsmTileToVRMLCoord( Pyramid *, double, double, int,
 *                             double *, double * )
 *    void tsmTileToVRMLBoundingBox( Pyramid *, int, int, int, double *,
 *                                   double * )
 *
 * Description:
 *    Routines to convert between a tile index (double-valued coord
 *    and level) to either it's lower-left geocentric coordinates in
 *    VRML, or the tile's bounding box in VRML coordinates.
 *
 *    N.B. These functions only calculate the (x,y) coordinates, the
 *    z coordinate is not calculated (in the bounding box routine, it
 *    is set to 0.0).
 *
 * Parameters:
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 16 January 1998
 *
 */

void
tsmTileToVRMLCoord( Pyramid *tspec, double tx, double ty, int level,
		    double *x, double *y )
{
  TileTspec     *tile;
  double        scale;
  float         *ixiytouv;

  tile     = &tspec->tileTspec;
  ixiytouv = tspec->tspecLevel.val[level].imageTransformationTspec.ixiytoUV;

  /* Work out the scaling factor so that we can scale the */
  /* entire valid (non-padded) terrain to the range 0..1  */

  scale = tspec->minResolution *
          tspec->tspecLevel.val[tspec->minLevel].maxValidPixel[0];

  /* Work out the VRML (vx,vy) coordinate for the tile (tx,ty) coords */

  *x = ( tile->numPixelsX * tx * ixiytouv[0] ) / scale;
  *y = ( tile->numPixelsY * ty * ixiytouv[4] ) / scale;

}

void
tsmTileToVRMLBoundingBox( Pyramid *tspec, int tx, int ty, int level,
			  double *p1, double *p2 )
{
  SimpleTileSet *tileset;
  TileTspec     *tile;
  double        tx2, ty2;

  /* Work out the VRML (vx,vy) coord of the lower-left tile corner */

  tsmTileToVRMLCoord( tspec, (double) tx, (double) ty, level, &p1[0], &p1[1] );

  /* Work out the tile coordinate (tx,ty) of the upper-right tile corner */

  tileset = &tspec->tspecLevel.val[level];
  tile    = &tspec->tileTspec;

  if ( tx >= tileset->maxTile[0] ) {
    tx2 = tx + ( tileset->maxValidPixel[0] % tile->numPixelsX ) /
          (double) tile->numPixelsX;
  } else
    tx2 = tx + 1;

  if ( ty >= tileset->maxTile[1] ) {
    ty2 = ty + ( tileset->maxValidPixel[1] % tile->numPixelsY ) /
          (double) tile->numPixelsY;
  } else
    ty2 = ty + 1;

  /* Convert this upper-right tile coordinate into VRML (vx,vy) coords */

  tsmTileToVRMLCoord( tspec, tx2, ty2, level, &p2[0], &p2[1] );
  p1[2] = p2[2] = 0.0;
}

/*
 * Synopsis:
 *    void tsmTileToUpVector( Pyramid *, int tx, int ty, int level, 
 *                            float *upVector)
 *
 * Description:
 *    Returns the vector pointing perpendicular to the sphereoid plane
 *    at the centroid of this tile.
 *
 * Parameters:
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 5 May 1998
 *
 */
void
tsmTileToUpVector( Pyramid *tspec, int tx, int ty, int level, 
		   TsmConnectionParams params, float *upVector)
{

  if (params->flat != TSM_GEOCNTRC_VTILES) {
    /* up vector is always pointing towards +Z */
    upVector[0] = 0;
    upVector[1] = 0;
    upVector[2] = 1;
  }
  else {
    double px, py;
    double center[3];
    double length;

    /* normalized line from center of earth to tile centroid */
    /* this should return lat long */
    tsmTileToVRMLCoord( tspec,  tx + 0.5, ty + 0.5, level, &px, &py);
    tsmLatLongToGeocentric(px, py, 0, &center[0], &center[1], &center[2]);

    /* now normalize it */
    length = sqrt(center[0] * center[0] + center[1] * center[1] + center[2] * center[2]);
    upVector[0] = center[0] / length;
    upVector[1] = center[1] / length;
    upVector[2] = center[2] / length;
  }

}

/*
 * Synopsis:
 *      int tsmGetCoords( TsmConnection oiConn, TsmConnection demConn,
 *                        double u, double v, int demLevel, TsmConnectionParams
 *                        params, double *coords, int elev_mode )
 *
 * Description:
 *      Given the u, v, OI level, and Pyramid pointer for a specific otile,
 *      returns the x, y, z world coords in coords.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Nat Bletter
 * 
 * Date: 1/13/98
 *
 */

int
tsmGetCoords( TsmConnection oiConn, TsmConnection demConn, double u, 
	      double v, int demLevel, TsmConnectionParams params,
	      double *coords, int elev_mode )
{
  const char    *funcName = "tsmGetCoords";
  double    txf, tyf;
  int       tx, ty;
  double    ixf, iyf;
  int       ix, iy;
  double    xlerp, ylerp;
  int       px, py;
  double    lerp[2][2];
  int       width;
  double    scale;
  double    geo_coords[3];
  double    oi_coords[3];
  double    dem_coords[3];
  double    lat, lon;
  double    easting, northing;
  double    orig_east, orig_north;
  int	    zone, orig_zone;
  double    eps;
  static double  deg2mtrs = 2 * M_PI * WGS84_AXIS / 360.0; /* degrees to meters at equator */


  Pyramid       *demPyramid = (Pyramid *)tsmGet(demConn, TSM_DATASET_TSPEC);
  Pyramid       *oiPyramid = (Pyramid *)tsmGet(oiConn, TSM_DATASET_TSPEC);
  SimpleTileSet *tileset  = &demPyramid->tspecLevel.val[demLevel];
  TileTspec     *tile     = &demPyramid->tileTspec;
  float         *uvtoixiy = tileset->imageTransformationTspec.uvtoIXIY;
  float         *ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  float		*oi_ixiytouv = oiPyramid->tspecLevel.val[0].imageTransformationTspec.ixiytoUV;
  uchar         *tileBuffer;

  coords[0] = 0.0;
  coords[1] = 0.0;
  coords[2] = 0.0;

  /* Set the scaling factor for the scene so that we can scale */
  /* the z-axis to the same degree as the x and y-axes. N.B    */
  /* this value should be the same as in tsmTileToVRMLCoord().    */

  /* scale = demPyramid->minResolution *
	  demPyramid->tspecLevel.val[demPyramid->minLevel].maxValidPixel[0]; */
  scale = params->world_scale;

  if (demPyramid->projectionTspec.projectionType == tspUtmProjection &&
      oiPyramid->projectionTspec.projectionType == tspLatLongProjection) {
    /* convert to utm for elevatiopn calculation */
    tsmLatLongToUTM(v, u, &zone, &dem_coords[0], &dem_coords[1] );
    if (zone != demPyramid->projectionTspec.utmZoneNumber) {
      fprintf(stderr, "Lat/Long OI data not in same UTM zone as UTM Dem data!\n");
      return FALSE;
    }
  }
  else if (demPyramid->projectionTspec.projectionType == tspLatLongProjection
	   && oiPyramid->projectionTspec.projectionType == tspUtmProjection) {
    /* convert to lat/long for elevatiopn calculation */
    tsmUTMToLatLong(oiPyramid->projectionTspec.utmZoneNumber, u, v, &dem_coords[1], &dem_coords[0]);
  }
  else if (demPyramid->projectionTspec.projectionType == 
	   oiPyramid->projectionTspec.projectionType) {
    dem_coords[0] = u;
    dem_coords[1] = v;
  }    
  else {
    fprintf(stderr, "OI and DEM data not in convertible data formats!\n");
    return FALSE;
  }

  /* make sure two ends of earth meet at 180/-180 */
  eps = ixiytouv[0] * 0.5;	/* half a pixel */
  if (demPyramid->projectionTspec.projectionType == 
      tspLatLongProjection && fabs(180.0 - dem_coords[0]) < eps)
    dem_coords[0] = -180.0;


  /* the (x,y) coordinates are easy to compute... */

  oi_coords[0] = u;
  oi_coords[1] = v;

  /* find float pixel coords */

  txf = (dem_coords[0] * uvtoixiy[0] + uvtoixiy[2]) / tile->numPixelsX;
  tyf = (dem_coords[1] * uvtoixiy[4] + uvtoixiy[5]) / tile->numPixelsY;

  if (txf < 0.0 && txf > -1.0/tile->numPixelsX)
    txf = 0.0;
  if (tyf < 0.0 && tyf > -1.0/tile->numPixelsY)
    tyf = 0.0;

  /* get tx, ty (tile coordinates) and ix, iy (pixel coordinates) */

  tx = (int) floor( txf );
  ty = (int) floor( tyf );

  ixf = tile->numPixelsX * ( txf - tx );
  iyf = tile->numPixelsY * ( tyf - ty );

  ix = (int) floor( ixf );
  iy = (int) floor( iyf );

  /* get the inter-pixel coordinates */

  xlerp = ixf - ix;
  ylerp = iyf - iy;

  /* Check that the elevation mode is valid. That is, if we   */
  /* have been asked to use the min or max dataset elev value */
  /* then make sure that we have these values in the tspec    */

  if ( elev_mode != TSM_ELEV_MODE_EXACT &&
       demPyramid->pixelTspec.minVal == demPyramid->pixelTspec.maxVal ) {

    tsmError( funcName, "Tspec doesn't have min/max values: doing slow calc");
    elev_mode = TSM_ELEV_MODE_EXACT;

  }

  /* Now calculate the elevation value for this point. We    */
  /* have a number of modes here: an exact mode which reads  */
  /* the actual elevation value from the dataset, doing a    */
  /* bilinear interpolation; then there are two other modes  */
  /* which just take the minimum and maximum elevation value */
  /* for the entire dataset, respectively. These are useful  */
  /* for doing very fast approximations for bounding boxes.  */

  switch ( elev_mode ) {
  case TSM_ELEV_MODE_EXACT:
    
    /* read the DEM tile, but only if it exists */

    if (tx < tileset->minTile[0] || tx > tileset->maxTile[0] ||
	ty < tileset->minTile[1] || ty > tileset->maxTile[1] ||
	! ( tileBuffer = tsmReadDEMTile( demConn, tx, ty, demLevel ) ) ) {
      return FALSE;
    }

    /* get 4 points for bilinear interpolation */

    width = tile->totalPixels[0];
  
    for ( px = 0; px < 2; px++ )
      for ( py = 0; py < 2; py++ ) {
	lerp[px][py] = *((long *)tileBuffer + (ix + px + (iy + py) * width ));
#if 0
	if ( lerp[px][py] * demPyramid->pixelTspec.scale +
	     demPyramid->pixelTspec.offset == demPyramid->sea_level )
	  lerp[px][py] = 0.0; /* set sea level height value to be 0 */
#endif
      }
  
    /* now do the bilerp */
    
    oi_coords[2] = (1.0-ylerp) * (lerp[0][0] * (1.0-xlerp) + lerp[1][0]
		    * xlerp) + ylerp * (lerp[0][1] * (1.0 - xlerp)
                    + lerp[1][1] * xlerp);
    break;

  case TSM_ELEV_MODE_MIN:
    oi_coords[2] = demPyramid->pixelTspec.minVal;
    break;

  case TSM_ELEV_MODE_MAX:
    oi_coords[2] = demPyramid->pixelTspec.maxVal;
    break;

  }

  /* Apply the appropriate scale and offset factors to the elevation */

  oi_coords[2] = ( oi_coords[2] * demPyramid->pixelTspec.scale
                    + demPyramid->pixelTspec.offset ) * params->elev_scale;

  /* Now project the point into the appropriate coordinate system */

  if (params->flat == TSM_GEOCNTRC_VTILES) {
    switch (oiPyramid->projectionTspec.projectionType) {
    case tspUtmProjection:
      tsmUTMToLatLong(oiPyramid->projectionTspec.utmZoneNumber, oi_coords[0], oi_coords[1], &lat, &lon);
      break;

    case tspLatLongProjection:
      lat = oi_coords[1];
      lon = oi_coords[0];
      break;

    default:
      tsmError( funcName, "Unsupported coordinate system: can only handle UTM and LAT/LONG" );
      coords[0] = oi_coords[0];
      coords[1] = oi_coords[1];
      coords[2] = 0.0;
      return TRUE;
    }
    tsmLatLongToGeocentric(lat, lon, oi_coords[2], &(geo_coords[0]), &(geo_coords[1]), &(geo_coords[2]));
    coords[0] = geo_coords[0]/scale;
    coords[1] = geo_coords[1]/scale;
    coords[2] = geo_coords[2]/scale;
  }
  else {
    switch (oiPyramid->projectionTspec.projectionType) {
    case tspUtmProjection:
      if (params->flat == TSM_UTM_VTILES) { /* keep it in UTM */
	/* subtract out UTM origin */
	coords[0] = (oi_coords[0] - oi_ixiytouv[2]) /scale;
	coords[1] = (oi_coords[1] - oi_ixiytouv[5]) /scale;
	coords[2] = oi_coords[2]/scale;
      }
      else {   /* Flat lat/long grid, change oi coords from UTM to Lat/Long */
	tsmUTMToLatLong(oiPyramid->projectionTspec.utmZoneNumber, oi_coords[0], oi_coords[1], &lat, &lon);
	coords[0] = lon * deg2mtrs / scale;
	coords[1] = lat * deg2mtrs / scale;
	coords[2] = oi_coords[2]/scale;
      }
      break;

    case tspLatLongProjection:
      if (params->flat == TSM_UTM_VTILES) { /* change to UTM */
	tsmLatLongToUTM(oi_coords[1], oi_coords[0], &zone, &easting, &northing );
	if (demPyramid->projectionTspec.projectionType == tspUtmProjection) {
          if (demPyramid->projectionTspec.utmZoneNumber != zone) {
            fprintf(stderr, "Coordinate UTM zone and dataset origin zone not equal!\a\n");
            return FALSE;
	  }
	  coords[0] = (easting - ixiytouv[2]) / scale;
	  coords[1] = (northing - ixiytouv[5]) / scale;
	}
	else {
  	  tsmLatLongToUTM(oi_ixiytouv[5], oi_ixiytouv[2], &orig_zone, &orig_east, &orig_north);
          if (orig_zone != zone) {
            fprintf(stderr, "Coordinate UTM zone and dataset origin zone not equal!\a\n");
            return FALSE;
	  }
	  coords[0] = (easting - orig_east)/ scale;
	  coords[1] = (northing - orig_north) / scale;
	}
	coords[2] = oi_coords[2]/scale;
      }
      else {			/* Flat lat/long grid requested */
	coords[0] = oi_coords[0] * deg2mtrs / scale;
	coords[1] = oi_coords[1] * deg2mtrs / scale;
	coords[2] = oi_coords[2]/scale;
      }	
      break;

    default:
      tsmError( funcName, "Unsupported coordinate system: can only handle UTM and LAT/LONG" );
      coords[0] = oi_coords[0];
      coords[1] = oi_coords[1];
      coords[2] = 0.0;
      return FALSE;
    }
      
  }

  return TRUE;
}

/*
 * Synopsis:
 *      void tsmGetTexCoords( int x, int y, int tx, int ty, int level,
 *                            Pyramid *pyramid, double *coords, int edge_x,
 *                            int edge_y, TsmConnectionParams params )
 *
 * Description:
 *      Given the x, y, and Pyramid pointer for a specific tile,
 *      returns the u, v texture coords in coords.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Nat Bletter
 * 
 * Date: 1/13/98
 *
 */

void
tsmGetTexCoords( int x, int y, int tx, int ty, int level, Pyramid *pyramid,
		 double *coords, int edge_x, int edge_y,
		 TsmConnectionParams params )
{
  TileTspec     *tile = &pyramid->tileTspec;
  SimpleTileSet *tileset = &pyramid->tspecLevel.val[level];
  double        pixels_x, pixels_y;
  int 		valpix_x, valpix_y;

  pixels_x = (double) tile->numPixelsX;
  pixels_y = (double) tile->numPixelsY;

  /* Calculate the texture coordinates. We deal with two cases: */
  /* the non-edge polys, and the edge polys. In each case, the  */
  /* texture coordinates represent the center point of the      */
  /* appropriate pixel in the texture map (hence the offsets of */
  /* 0.5/num_pixels that are used).                             */

#ifdef TEX_EDGE_OFFSET
  if ( edge_x == FALSE ) {
    coords[0] = (double) x / params->num_polys;
    coords[0] = (coords[0] * pixels_x) / (pixels_x + 1) + (0.5 / pixels_x);
  } else {
    valpix_x = tileset->numValidPixel[0] % tile->numPixelsX;
    if (valpix_x == 0)
      coords[0] = 1.0 - ( 0.5 / pixels_x );
    else
      coords[0] = (double)valpix_x / ( pixels_x + 1) + ( 0.5 / pixels_x );
  }
#else
  if ( edge_x == FALSE ) 
    coords[0] = (double) x / params->num_polys;
  else {
    valpix_x = tileset->numValidPixel[0] % tile->numPixelsX;
    if (valpix_x == 0)
      coords[0] = 1.0;
    else
      coords[0] = (double)valpix_x / pixels_x;
  }
#endif

#ifdef TEX_EDGE_OFFSET
  if ( edge_y == FALSE ) {
    coords[1] = (double) y / params->num_polys;
    coords[1] = (coords[1] * pixels_y) / (pixels_y + 1) + (0.5 / pixels_y);
  } else {
    valpix_y = tileset->numValidPixel[1] % tile->numPixelsY;
    if (valpix_y == 0)
      coords[1] = 1.0 - ( 0.5 / pixels_y );
    else
      coords[1] = (double)valpix_y / ( pixels_y + 1 ) + (0.5 / pixels_y );
  }
#else
  if ( edge_y == FALSE ) 
    coords[1] = (double) y / params->num_polys;
  else {
    valpix_y = tileset->numValidPixel[1] % tile->numPixelsY;
    if (valpix_y == 0)
      coords[1] = 1.0;
    else
      coords[1] = (double)valpix_y / pixels_y;
  }
#endif      


}


/*
 * Synopsis:
 *    int tsmTVToVRMLLevel( Pyramid const *tspec, int level )
 *
 * Description:
 *    A function that takes a TerraVision hierarchy level number and returns
 *    a level number for a VRML hierarchy. In TV, level 0 was the highest
 *    resolution level, but in the VRML level 0 should be the lowest 
 *    resolution level.
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

int
tsmTVToVRMLLevel( Pyramid const *tspec, int level )
{
  return ( tspec != NULL ) ? tspec->maxLevel - level : 0;
}


/*
 * Synopsis:
 *    void tsmMakeVtileFilename( Pyramid const *, int, int, int, char * )
 *
 * Description:
 *    Routine to create a filename for a vtile VRML file
 *
 * Parameters:
 *    tspec    - the pyramid tspec for the OI dataset
 *    tx, ty   - the tile (x,y) coordinates
 *    level    - the (TerraVision) level number. N.B. this will be
 *               converted into a VRML level number.
 *    filename - a buffer to write the filename into.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

void
tsmMakeVtileFilename( Pyramid const *tspec, int tx, int ty, int level,
		      TsmConnectionParams params, char *filename )
{
  sprintf( filename, "%s/vtiles/%d/p%dp%d.wrl", params->vrml_dir, 
	   tsmTVToVRMLLevel( tspec, level ), tx, ty );
}

/*
 * Synopsis:
 *    void tsmMakeGeotileFilename( Pyramid const *, int, int, int, char * )
 *
 * Description:
 *    Routine to create a filename for a geotile VRML file
 *
 * Parameters:
 *    tspec    - the pyramid tspec for the OI dataset
 *    tx, ty   - the tile (x,y) coordinates
 *    level    - the (TerraVision) level number. N.B. this will be
 *               converted into a VRML level number.
 *    filename - a buffer to write the filename into.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

void
tsmMakeGeotileFilename( Pyramid const *tspec, int tx, int ty, int level,
			TsmConnectionParams params, char *filename )
{
  sprintf( filename, "%s/%s/%d/p%dp%d.wrl", params->vrml_dir, 
	   ( params->tree_type == TSM_TREE_TYPE_QUADLOD ) ? "trees":"geotiles",
	   tsmTVToVRMLLevel( tspec, level ), tx, ty );
}

/*
 * Synopsis:
 *    void tsmMakeTreeFilename( Pyramid const *, int, int, int, char * )
 *
 * Description:
 *    Routine to create a filename for a tree file
 *
 * Parameters:
 *    tspec    - the pyramid tspec for the OI dataset
 *    tx, ty   - the tile (x,y) coordinates
 *    level    - the (TerraVision) level number. N.B. this will be
 *               converted into a VRML level number.
 *    filename - a buffer to write the filename into.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

void
tsmMakeTreeFilename( Pyramid const *tspec, int tx, int ty, int level,
		     TsmConnectionParams params, char *filename )
{
  sprintf( filename, "%s/trees/%d/p%dp%d.wrl", params->vrml_dir,
	   tsmTVToVRMLLevel( tspec, level ), tx, ty );
}

/*
 * Synopsis:
 *    void tsmMakeImageFilename( Pyramid *, char *, int, int, int, char * )
 *
 * Description:
 *    Routine to create a filename for a JPG image.
 *
 * Parameters:
 *    tspec    - the pyramid tspec for the OI dataset
 *    tx, ty   - the tile (x,y) coordinates
 *    level    - the (TerraVision) level number. N.B. this will be
 *               converted into a VRML level number.
 *    filename - a buffer to write the filename into.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

void
tsmMakeImageFilename( Pyramid const *tspec, int tx, int ty, int level,
		      char *filename, TsmConnectionParams params )
{
  sprintf( filename, "%s/images/%d/p%dp%d.jpg", params->vrml_dir, 
	   tsmTVToVRMLLevel( tspec, level ), tx, ty );
}

/*
 * Synopsis:
 *    void tsmMakeGeoTileFilename( Pyramid *, double, double, double, char * )
 *
 * Description:
 *    Routine to create a filename for a GeoTile file
 *
 * Parameters:
 *    tspec    - the pyramid tspec for the OI dataset
 *    lat, long - SW tile origin coordinates in degrees
 *    scale    - the size of the tile in longitudinal degrees on a side
 *    filename - a buffer to write the filename into.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 14 January 1998
 *
 */

#if 0
void
tsmMakeGeoTileFilename( Pyramid const *tspec, double lat, double lon,
			double scale, char *filename,
			TsmConnectionParams params )
{
  int level = floor(log(360 / scale)/log(2.0));
  int tx = floor(lon / scale);
  int ty = floor((lon + 90.0) / scale); /* make it all positive */
  sprintf( filename, "%s/geotiles/%d/p%dp%d.wrl", params->vrml_dir, level,
	   tx, ty );
}
#endif

/*
 * Synopsis:
 *   double tsmSetInitDistance(  double const *ll, double const *ur )
 *   double tsmSetNearPlane( double const *ll, double const *ur )
 *
 * Description:
 *   Two routines to calculate the initial overhead view distance for
 *   the tile, and the near clipping plane distance. The initial view
 *   distance is calculated so that the tile always occupies a certain
 *   percentage of the screen (for the longest dimension, be it X or Y).
 *
 * Parameters: 
 *
 * Returns:
 *
 * Author: Martin Reddy
 *
 * Date: 19 January 1998
 *
 */

double
tsmSetInitDistance( double const *ll, double const *ur )
{
  double geo_coverage;                          /* geom dimensions */
  double percent_coverage = 0.4;        /* initial screen coverage */

  geo_coverage  = max( max( ur[0] - ll[0], ur[1] - ll[1] ), ur[2] - ll[2] );

  return ( geo_coverage * 0.5 * PROJ_F ) / percent_coverage + ur[2];
}

double
tsmSetNearPlane( double const *ll, double const *ur )
{
  double geo_coverage;

  geo_coverage  = max( max( ur[0] - ll[0], ur[1] - ll[1] ), ur[2] - ll[2] );

  return geo_coverage / 5000.0;
}

/*
 * Synopsis:
 *    double tsmCalcRange( Pyramid tspec, int tx, int ty, int level )
 *
 * Description:
 *    This function calculates the switching distance for a tile based upon
 *    when the texels in the texture map project to a single pixel on the
 *    display. However, there are a number of uncertains due to the limitations
 *    in VRMLs LOD model:
 *
 *    1. We have to assume a certain display size (currently 512 pixels)
 *       because there is no way of knowing or a priori compensating for
 *       different screen resolutions.
 * 
 *    2. The VRML LOD model is a distance-based system, rather than a
 *       size-based one. This means that when viewing the tile at a low
 *       angle the switching may occur later than might be expected because
 *       the switch is based upon the distance to the center of the tile,
 *       and not the part of the tile that is closest to the viewer.
 * 
 *    N.B. a scaling value can be applied to the function to effectively
 *    specify the number of pixels that a texel should cover before the
 *    tile switches LOD (the default is 1 pixel).
 *
 * Parameters: 
 *    tspec - the Pyramid tspec structure for the OI pyramid
 *    tx, ty - the (x,y) coordinates for the tile to calc the range for
 *    level - the level of the tile to calc the LOD range for
 *
 * Returns:
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

double
tsmCalcRange( Pyramid const *tspec, int tx, int ty, int level, int end_level,
	      TsmConnectionParams params, double *ll, double *ur )
{
  double          /*geo_width, geo_height,*/ geo_size;   /* geom dimensions */
  double          tex_width, tex_height, tex_size;    /* texture dimensions */
  SimpleTileSet   *tileset;
  const TileTspec *tile;

  /* We use the following constants for the calculation:              */
  /*                                                                  */  
  /* 1. "PROJ_F" equals the distance from the tip of the perspective  */
  /* pyramid to the viewport. This can be calculated from the display */
  /* field of view by the equation 1 / tan( fov / 2.0 ). We assume a  */
  /* default field of view of 45 degrees.                             */
  /*                                                                  */  
  /* 2. "PROJ_SCR" represents the smallest value of the display's     */
  /* width and height, in pixels. It is this value that the field of  */
  /* view setting refers to in VRML.                                  */

  tileset = &tspec->tspecLevel.val[level];
  tile    = &tspec->tileTspec;

  /* Now we want to be able to calculate the size of the geometry and */
  /* the number of pixels in the texture map that is applied to that  */
  /* geometry. This is done for both the X and Y axes.                */
  
  /*
  geo_width  = ur[0] - ll[0];
  geo_height = ur[1] - ll[1];
  */

  geo_size = max( max( ur[0] - ll[0], ur[1] - ll[1] ), ur[2] - ll[2] );

  /* Find the valid number of pixels in the texture map. This is  */
  /* the number of pixels that are mapped onto the above geometry */

  if ( tx < tileset->maxTile[0] )
    tex_width = tile->numPixelsX;
  else {
    tex_width = tileset->numValidPixel[0] % tile->numPixelsX;
    if ( tex_width == 0 ) tex_width = tile->numPixelsX;
  }

  if ( ty < tileset->maxTile[1] )
    tex_height = tile->numPixelsY;
  else {
    tex_height = tileset->numValidPixel[1] % tile->numPixelsY;
    if ( tex_height == 0 ) tex_height = tile->numPixelsY;
  }

  tex_size = max( tex_height, tex_width );
  
  /* With this info, we see which dimension has the largest texels,  */
  /* and use this dimension's screen width and texture size for the  */
  /* reverse perspective projection calculation.                     */

  /*
  if ( geo_width / tex_width > geo_height / tex_height ) {
    geo_size = geo_width;
    tex_size = tex_width;
  } else {
    geo_size = geo_height;
    tex_size = tex_height;
  }
  */

  /* Finally we can do the reverse perspective calculation. Normally */
  /* we can find the (x',y') screen coordinates from an (x,y,x)      */
  /* world coordinate by computing the equation:                     */
  /*       x' = ( screen_width * x * f ) / z                         */
  /*       y' = ( screen_height * y * f ) / z                        */
  /* Here we turn this round in terms of the distance variable, z.   */

  return ( geo_size * 0.5 * PROJ_F * PROJ_SCR ) /
         ( tex_size * params->texel_scale );
}

/*
 * Synopsis:
 *    int tsmTileElev( TsmConnection oiConn, TsmConnection demConn, int tx,
 *                     int ty, int iLevel, double *min_elev, double *max_elev,
 *                     TsmConnectionParams params )
 *
 * Description:
 *    Works out the bounding box for a specific tile
 *
 *    Currently not used anywhere - can be removed?
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 January 1998
 *
 */

int
tsmTileElev( TsmConnection oiConn, TsmConnection demConn, int tx, int ty,
	     int iLevel, double *min_elev, double *max_elev, 
	     TsmConnectionParams params )
{
  /*static const char funcName[] = "tsmTileElev";*/
  double        p[3];
  int           polyX, polyY /*, polyx, polyy*/;
  double        x, y;
  double        demLevel, maxix, maxiy;
  float         *ixiytouv;
  TileTspec     *tile;
  SimpleTileSet *tileset, *tileset0;

  Pyramid *oiPyramid = (Pyramid *)tsmGet(oiConn, TSM_DATASET_TSPEC);
  Pyramid *demPyramid = (Pyramid *)tsmGet(demConn, TSM_DATASET_TSPEC);

  if ( params->test_vtiles == TRUE ) {
    *max_elev = ( tsmTVToVRMLLevel( oiPyramid, iLevel ) + 1 ) / 2000.0;
    *min_elev = -(*max_elev);
    return TRUE;
  }

  /* Take some convenience pointers to commonly used structures */

  tileset  = &oiPyramid->tspecLevel.val[iLevel];
  tileset0 = &oiPyramid->tspecLevel.val[oiPyramid->minLevel];
  ixiytouv = tileset->imageTransformationTspec.ixiytoUV;
  tile     = &oiPyramid->tileTspec;

  /* Work out the DEM level to use */

  if ( params->gaussian ) {

    /* use appropriately filtered level and make sure it's in range */

    demLevel = floor(log((double)(tile->numPixelsX)/params->num_polys)/log(2)+iLevel);
    demLevel = max( demPyramid->minLevel, demLevel );
    demLevel = min( demPyramid->maxLevel, demLevel );

  } else
    demLevel = demPyramid->minLevel;

  /* See CreateVtile func for description of maxix/polyx vars */

  maxix = oiPyramid->minResolution * tileset0->maxValidPixel[0];
  maxiy = oiPyramid->minResolution * tileset0->maxValidPixel[1];

  /*
  polyx = polyy = params->num_polys;

  if ( tx >= tileset->maxTile[0] )
  polyx = (int) ceil( (double) tileset->numValidPixel[0] * params->num_polys /
  tileset->numPixels[0] );

  if ( ty >= tileset->maxTile[1] )
  polyy = (int) ceil( (double) tileset->numValidPixel[1] * params->num_polys /
  tileset->numPixels[1] );
  */

  /* read each of the sampled coordinates first */

  for ( polyY = 0; polyY <= params->num_polys; polyY++ ) {
    for ( polyX = 0; polyX <= params->num_polys; polyX++ ) {

      /* Work out the (x,y,z) coordinates for the geometry */

      x = tile->numPixelsX * ( tx + (double) polyX / params->num_polys ) * ixiytouv[0];
      y = tile->numPixelsY * ( ty + (double) polyY / params->num_polys ) * ixiytouv[4];

      /* Clip the coordinates to valid texture map pixels */

      x = min( x, maxix );
      y = min( y, maxiy );
      
      tsmGetCoords( oiConn, demConn, x, y, demLevel, params, p,
		    TSM_ELEV_MODE_EXACT );

      /* Check to see if we need to update the min or max elevation */

      if ( polyX == 0 && polyY == 0 )
	*min_elev = *max_elev = p[2];
      else {
	*max_elev = max( *max_elev, p[2] );
	*min_elev = min( *min_elev, p[2] );
      }

    }
  }

  return TRUE;
}

