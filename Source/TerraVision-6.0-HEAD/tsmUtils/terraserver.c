/*
 * Name: terraserver.c
 *
 * Description:
 *     This program provides a TerraVision interface to the Microsoft
 *     TerraServer facility. TerraServer provides tiled versions of
 *     USGS imagery for the entire USGS.
 *
 *     This can be linked against another program and exports the
 *     following four functions. See the code below for usage.
 *
 *       ts_init();
 *       ts_cleanup();
 *       ts_set_debug( int );
 *       ts_is_terrasever( char * )
 *       ts_get_terraserver_tile( TsmConnection, int, int, int, uchar ** )
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 30 June 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: terraserver.c,v 1.1.2.5 2002/03/14 05:16:07 reddy Exp $ 
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tsm/tsm.h>
#include <tsmUtils/terraserver.h>
#include <ghttp/http_trans.h>
#include <jpeg/jpeglib.h>

/* uncomment this line to create a test function main() */
/* #define TERRASERVER_MAIN */

http_trans_conn *ts_http_conn = NULL;

/* prototypes for null JPEG callbacks defined internally by tsmApi */
void null_init_source( j_decompress_ptr cinfo );
boolean null_fill_input_buffer( j_decompress_ptr cinfo );
boolean null_resync_to_restart( j_decompress_ptr cinfo, int d );
void null_term_source( j_decompress_ptr cinfo );
void null_skip_input_data( j_decompress_ptr cinfo, long num_bytes );
int tsmUrlToMemory( http_trans_conn **, char const *, char **, int * );

#define CACHE_SIZE 24  /* number of TerraServer tiles to cache */

typedef struct {
  int   s, t, x, y, z;   /* key for this cache entry */
  uchar *buffer;         /* JPEG buffer returned by TerraServer */
  int   size;            /* size of JPEG buffer in bytes */
} TerraServerTile;

TerraServerTile TerraServerCache[CACHE_SIZE];
int             cache_next_add;
int             ts_debugging = FALSE;

static void
ts_debug( const char *msg, ... )
{
  va_list ap;

#ifndef DEBUG
  if ( ! ts_debugging ) return;
#endif
  fprintf( stderr, "terraserver: " );
  va_start( ap, msg );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
}

/*
 * Switch TerraServer debugging mode to be on or off. When debugging
 * is on, various debug messages are sent to stderr (picked up by the
 * Apached error_log).
 */

void
ts_set_debug( int debug_on )
{
  ts_debugging = debug_on;
}

/*
 * Initialise the TerraServer capability. This must be the first
 * function that is called from this module
 */

void
ts_init()
{
  int i;
  for ( i = 0; i < CACHE_SIZE; i++ )
    TerraServerCache[i].buffer = NULL;
  
  cache_next_add = 0;
  ts_http_conn = NULL;
}

/*
 * Cleanup the TerraServer module when done
 */

void
ts_cleanup()
{
  int i;

  for ( i = 0; i < CACHE_SIZE; i++ )
    if ( TerraServerCache[i].buffer )
      free( TerraServerCache[i].buffer );

  if ( ts_http_conn )
    tsmCloseHttpConnection( ts_http_conn );
}

/*
 * Add a TerraServer JPEG tile to our cache 
 */

static int
ts_cache_add( int s, int t, int x, int y, int z, uchar *buffer, int size )
{
  TerraServerTile *cache = &TerraServerCache[cache_next_add];

  /* de-cache existing tile at the next location */

  if ( cache->buffer != NULL )
    free( cache->buffer );

  /* fill it with the new information */

  cache->s      = s;
  cache->t      = t;
  cache->x      = x;
  cache->y      = y;
  cache->z      = z;
  cache->buffer = buffer;
  cache->size   = size;

  /* and then move the pointer onto the next location for next time round */

  cache_next_add = ( cache_next_add + 1 ) % CACHE_SIZE;

  return TRUE;
}

/*
 * Check the cache for a TerraServer JPEG file. Returns TRUE if found and
 * sets the buffer and size fields with the relevant buffer details.
 * Otherwise, returns FALSE if tile is not in cache.
 */

static int
ts_cache_check( int s, int t, int x, int y, int z, uchar **buffer, int *size )
{
  int i;
  
  for ( i = 0; i < CACHE_SIZE; i++ ) {
    TerraServerTile *cache = &TerraServerCache[i];

    if ( cache->s == s && cache->t == t && cache->x == x && cache->y == y &&
	 cache->z == z ) {
      *buffer = cache->buffer;
      *size   = cache->size;
      return TRUE;
    }
  }

  return FALSE;
}

/*
 * Take a TerraServer JPEG file and decompress this to a raw raster.
 * We enforce that the JPEG is the standard TerraServer 200x200 tile
 * size, and that the number of components in the image must match the
 * stated size (e.g. 1 for greyscale, 3 for RGB).
 *
 * The raw raster is allocated for you and should be freed by the client.
 */

static int
ts_jpeg_decomp( uchar *jpegBuffer, uchar **tileBuffer, int size, int comps )
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;
  struct jpeg_source_mgr        src_mgr;
  JSAMPROW                      buffer;
  int                           row_size;

  if ( jpegBuffer == NULL ) {
    ts_debug( "input and output JPEG buffers must be non-NULL!\n" );
    return FALSE;
  }

  /* Initialise the JPEG decompression object with default error handling. */

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_decompress( &cinfo );

  /* set up the data source manager to read from jpegBuffer */

  src_mgr.next_input_byte   = jpegBuffer;
  src_mgr.bytes_in_buffer   = size;
  src_mgr.init_source       = null_init_source;
  src_mgr.fill_input_buffer = null_fill_input_buffer;
  src_mgr.skip_input_data   = null_skip_input_data;
  src_mgr.resync_to_restart = null_resync_to_restart;
  src_mgr.term_source       = null_term_source;

  cinfo.src = &src_mgr;

  /* Read the JPEG header and set default decompression parameters */

  jpeg_read_header( &cinfo, TRUE );

  /* Check that the number of components in the JPEG file matches */

  if ( cinfo.num_components != comps ) {
    ts_debug( "JPEG Decompress buffer is wrong size! (%d != %d comps)",
	      cinfo.num_components, comps );
    return FALSE;
  }

  /* check that the tile size is the standard TerraServer 200 x 200 */

  if ( cinfo.image_width != 200 || cinfo.image_height != 200 ) {
    ts_debug( "TerraServer tile size is %d x %d (not 200x200)!!\n",
	      cinfo.image_width, cinfo.image_height );
    return FALSE;
  }

  /* now we can allocate our output tile buffer */

  if ( ( *tileBuffer = calloc( 200 * 200, comps ) ) == NULL ) {
    ts_debug( "Could not allocate a JPEG output buffer!\n" );
    return FALSE;
  }

  /* Start decompressor */

  jpeg_start_decompress( &cinfo );

  /* Decompress the data into tileBuffer, starting with the last row  */
  /* and working back to the first row because we also need to switch */
  /* the origin from JPEG's lrtb to TV's lrbt                         */

  row_size = cinfo.image_width * cinfo.num_components;
  buffer   = (JSAMPROW) *tileBuffer;

  while ( cinfo.output_scanline < cinfo.output_height ) {
    jpeg_read_scanlines( &cinfo, &buffer, 1 );
    buffer += row_size;
  }

  /* Finish decompression and release memory. */

  jpeg_finish_decompress( &cinfo );
  jpeg_destroy_decompress( &cinfo );

  return TRUE;
}

/*
 * Saves a tile out to a PPM file on disk with the specified filename
 *
 * You can specify the tile size (128 for TV, 200 for TS) and the number
 * of components per pixel (1 or 3). This is used for debugging purposes 
 */

static void
ts_write_tile_ppm( char *filename, uchar *tileBuffer, int tilesize, int comps )
{
#ifdef TERRASERVER_MAIN
  FILE *fp;
  if ( tileBuffer == NULL ) return;
  if ( ( fp = fopen( filename, "w" ) ) != NULL ) {
    fprintf( fp, "P%d\n%d %d\n255\n", ( comps == 1 ) ? 5 : 6,
	     tilesize, tilesize  );
    fwrite( tileBuffer, tilesize * tilesize, comps, fp );
    fclose( fp );
    ts_debug( "Wrote %s (%d bytes)\n", filename, tilesize * tilesize * comps );
  }
#endif
}

/*
 * This routine downloads an actual TerraServer tile, using the 
 * TerraServer tile indexing scheme. This indexing scheme is:
 *
 *  S = scale (10 = 1 m, 11 = 2m, 12 = 4m, 13 = 18m, ...)
 *  T = map type (1 = DOQ, 2 = DRG)
 *  X = scaled easting. True easting = 2^(S-10) * X * 200
 *  Y = scaled northing. True northing = 2^(S-10) * Y * 200
 *  Z = UTM zone number (1..60)
 *
 * Note that you pass in the number of components per pixel, and that
 * this value is used to decided the value of T for you. The resulting
 * image is returned as a raw bitmap, the memory for which is allocated
 * for you, i.e. you must free this when done.
 */

static int
ts_get_terraserver( int s, int x, int y, int z, int comps, uchar **buffer )
{
  char  url[1024];
  uchar *jpeg_buffer;
  int   size, success;
  int   t = ( comps == 1 ) ? 1 : 2;

  /* check to see if we have this tile in our cache */

  if ( ! ts_cache_check( s, t, x, y, z, &jpeg_buffer, &size ) ) {

    /* terraserver.microsoft.com currently redirects to their 
       home advisor.msn.com site. Use this address for now to
       avoid the extra overhead of a redirect, but if this fails,
       fallback to the terraserver.microsoft.com address */
    
    sprintf( url, "http://terraserver.homeadvisor.msn.com/tile/tile.asp?"
	     "S=%d&T=%d&X=%d&Y=%d&Z=%d", s, t, x, y, z );

    ts_debug( "URL = %s\n", url );

    success = tsmUrlToMemory( &ts_http_conn, url, &jpeg_buffer, &size );
    if ( ! success ) {
      /* okay - try the original terraserver.microsoft.com address */
      sprintf( url, "http://terraserver.microsoft.com/tile/tile.asp?"
	       "S=%d&T=%d&X=%d&Y=%d&Z=%d", s, t, x, y, z );
      ts_debug( "2nd URL = %s\n", url );
      success = tsmUrlToMemory( &ts_http_conn, url, &jpeg_buffer, &size );
      if ( ! success ) return FALSE;
    }

    /* we now have a jpeg buffer - let's add it to our cache for next time */

    ts_cache_add( s, t, x, y, z, jpeg_buffer, size );

    ts_debug( "Add to cache: s=%d t=%d x=%d y=%d z=%d (size = %d)\n",
	      s, t, x, y, z, size );

  } else {
    ts_debug( "Return from cache: s=%d t=%d x=%d y=%d z=%d (size=%d)\n",
	      s, t, x, y, z, size );
  }

  success = ts_jpeg_decomp( jpeg_buffer, buffer, size, comps );
  if ( ! success ) {
    free( jpeg_buffer );
    ts_debug( "JPEG decompress error\n" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Carve out a part of a TS tile and copy it into a TV tile.
 *
 * This routine is used to carve out the appropriate bits of the
 * 1 to 4 TerraServer 200x200 tiles in order to serve a 128x128
 * TerraVision tile.
 */

static void
ts_tile_copy_rows( uchar *in, int ix, int iy, int w, int h,
		   uchar *out, int ox, int oy, int comps, int tv_tilesize )
{
  int irow = 200 * comps, y;
  int orow = tv_tilesize * comps;
  uchar *instart, *outstart;

  iy = 200 - iy - 1;

  instart  = in + ( iy * irow ) + ( ix * comps );
  outstart = out + ( oy * orow ) + ( ox * comps );

  ts_debug( "copy (%d,%d - %d,%d) to %d,%d\n", ix, iy, w, h, ox, oy );

  for ( y = 0; y < h; y++ ) {
    memcpy( outstart, instart, w * comps );
    outstart += orow;
    instart -= irow;
  }
}

/*
 * The routine where all the magic happens!
 *
 * This function will take a connection to the dummy TerraVision
 * dataset (with tspec) for the TerraServer range. You specify
 * the tile (x, y) and level number in the usual TerraVision fashion.
 * This function will then work out which TerraServer tiles to 
 * download and then copy the relevant bits of these into a TerraVision
 * 128x128 tile.
 */

static int
get_terraserver_tile( TsmConnection connect, int x, int y, int level,
		      uchar **tileBuffer )
{
  Pyramid   *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  TileTspec *tile = &tspec->tileTspec;
  ProjectionTspec *proj = &tspec->projectionTspec;
  SimpleTileSet *tset0 = &tspec->tspecLevel.val[tspec->minLevel];
  
  float *ixiy2uv = tset0->imageTransformationTspec.ixiytoUV;
  float ll_e = ixiy2uv[2];
  float ll_n = ixiy2uv[5];

  float level_pow2 = pow( 2, level );
  float ts_tilesize = 200;
  float tv_tilesize = tile->numPixelsX;
  float tilesize_ratio  = tv_tilesize / ts_tilesize;

  int   origin_e = (int) (ll_e / ts_tilesize);
  int   origin_n = (int) (ll_n / ts_tilesize);
  int   ts_scale = 10 + level;
  int   x1, y1, x2, y2;
  int   comps = tile->numComponentsPixel;
  int   utm_zone = proj->utmZoneNumber;

  int   w1, w2, h1, h2;

  int   success = FALSE;

  float tx1, ty1, tx2, ty2;

  float x_lev0 = x * level_pow2;
  float y_lev0 = y * level_pow2;

  uchar *buff1 = NULL, *buff2 = NULL, *buff3 = NULL, *buff4 = NULL, *buff;

  tx1 = ( origin_e + ( x_lev0 * tilesize_ratio ) ) / level_pow2;
  ty1 = ( origin_n + ( y_lev0 * tilesize_ratio ) ) / level_pow2;
  tx2 = tx1 + tilesize_ratio;
  ty2 = ty1 + tilesize_ratio;

  x1 = (int) floor( tx1 );
  y1 = (int) floor( ty1 );
  x2 = (int) floor( tx2 );
  y2 = (int) floor( ty2 );

  ts_debug("(%d, %d, %d) -> S%d, X=%1.2f -> %1.2f, Y=%1.2f -> %1.2f (%1.1f)\n",
	   x, y, level, ts_scale, tx1, tx2, ty1, ty2, level_pow2 );

  /* download the 1 to 4 TerraServer tiles that we need */

  ts_get_terraserver( ts_scale, x1, y1, utm_zone, comps, &buff1 );
  if ( x2 != x1 )
    ts_get_terraserver( ts_scale, x2, y1, utm_zone, comps, &buff2 );
  if ( y2 != y1 )
    ts_get_terraserver( ts_scale, x1, y2, utm_zone, comps, &buff3 );
  if ( y2 != y1 && x2 != x1 )
    ts_get_terraserver( ts_scale, x2, y2, utm_zone, comps, &buff4 );

  /* try a second time in case we got some black patches */

  if ( buff1 == NULL )
    ts_get_terraserver( ts_scale, x1, y1, utm_zone, comps, &buff1 );
  if ( x2 != x1 && buff2 == NULL )
    ts_get_terraserver( ts_scale, x2, y1, utm_zone, comps, &buff2 );
  if ( y2 != y1 && buff3 == NULL )
    ts_get_terraserver( ts_scale, x1, y2, utm_zone, comps, &buff3 );
  if ( y2 != y1 && x2 != x1 && buff4 == NULL )
    ts_get_terraserver( ts_scale, x2, y2, utm_zone, comps, &buff4 );

  ts_write_tile_ppm( "tile1.pgm", buff1, 200, comps );
  ts_write_tile_ppm( "tile2.pgm", buff2, 200, comps );
  ts_write_tile_ppm( "tile3.pgm", buff3, 200, comps );
  ts_write_tile_ppm( "tile4.pgm", buff4, 200, comps );

  /* allocate a TerraVision tile buffer */

  *tileBuffer = buff = tsmAllocTileBuffer( connect );
  if ( *tileBuffer == NULL ) goto cleanup;

  /* cut out the relevant parts from each TerraServer tile */

  x1 = ( tx1 - floor( tx1 ) ) * ts_tilesize;
  y1 = ( ty1 - floor( ty1 ) ) * ts_tilesize;
  w1 = ( x1 + tv_tilesize <= ts_tilesize ) ? tv_tilesize : ts_tilesize - x1;
  h1 = ( y1 + tv_tilesize <= ts_tilesize ) ? tv_tilesize : ts_tilesize - y1;
  w2 = tv_tilesize - w1;
  h2 = tv_tilesize - h1;

  if ( buff1 )
    ts_tile_copy_rows(buff1, x1, y1, w1, h1, buff, 0, 0, comps, tv_tilesize);

  if ( buff2 ) 
    ts_tile_copy_rows(buff2, 0, y1, w2, h1, buff, w1, 0, comps, tv_tilesize);

  if ( buff3 )
    ts_tile_copy_rows(buff3, x1, 0, w1, h2, buff, 0, h1, comps, tv_tilesize);

  if ( buff4 )
    ts_tile_copy_rows(buff4, 0, 0, w2, h2, buff, w1, h1, comps, tv_tilesize);

  /* free up */

  success = TRUE;

 cleanup:  
  if ( buff1 ) free( buff1 );
  if ( buff2 ) free( buff2 );
  if ( buff3 ) free( buff3 );
  if ( buff4 ) free( buff4 );

  return success;
}

/*
 * Test whether a URL is for a TerraServer dataset
 *
 * This exported function can be used by a client to decide whether
 * or not to call the ts_get_terraserver_tile() function.
 */

int
ts_is_terraserver( char *url )
{
  return ( strstr( url, "TerraServer_" ) != NULL );
}

/*
 * Main entry point for serving TerraServer tiles in TerraVision form.
 *
 * This exported function is what a client should use in order to
 * access the TerraServer database using the normal TerraVision
 * semantics. This function will first check whether the dummy 
 * TerraVision dataset contains data for the requested level and
 * will use that. Otherwise, we contact TerraServer and download the
 * terrain data. This lets you fill the low-res parts of the pyramid
 * with data that TerraServer doesn't supply.
 */

int
ts_get_terraserver_tile( TsmConnection connect, int x, int y, int level,
			 uchar **tileBuffer )
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  int     local_data = FALSE;

  /* check to see if we have any local data for this level of the pyramid */

  if ( tspec && level > tspec->minLevel &&
      tsmGet( connect, TSM_CONNECT_PROTOCOL ) == TSM_LOCAL ) {
    PRFileInfo64 filetest;
    char         filename[512];
    tsmTIFFFilename( connect, level, filename );
    local_data = ( PR_GetFileInfo64 ( filename, &filetest ) == PR_SUCCESS );
  }

  ts_debug( "(%d, %d, %d) -> local data = %s\n", x, y, level,
	    local_data ? "yes" : "no" );

  if ( local_data ) {

    /* request a plain old TerraVision tile from the local dataset */

    return tsmAllocReadTile( connect, x, y, level, tileBuffer );
    
  } else {

    /* no local data for this level - go out to terraserver.microsoft.com */

    return get_terraserver_tile( connect, x, y, level, tileBuffer );
  }
}


/*
 * Test function main()
 *
 * This can be used to create a standalone program for testing the
 * TerraServer functionality. To use this, you should uncomment the
 * #define below.
 */

#ifdef TERRASERVER_MAIN

int
main( int argc, char **argv )
{
  int x, y, level;
  uchar *tileBuffer = NULL;
  char *url="/home/avila3/TileSets/TerraServer/Pyramids/TerraServer_sfbay.oi";
  TsmConnection connect;

  if ( argc < 4 ) {
    printf( "usage: %s <x> <y> <level>\n", argv[0] );
    exit( 0 );
  }

  if ( ( connect = tsmConnect( url, "r", NULL ) ) == NULL ) {
    printf( "Couldn't connect to TerraServer dataset\n%s\n", url );
    exit( 0 );
  }

  x = atoi( argv[1] );
  y = atoi( argv[2] );
  level = atoi( argv[3] );

  ts_init();
  ts_get_terraserver_tile( connect, x, y, level, &tileBuffer );  
  tsmFreeTileBuffer( tileBuffer );

  ts_get_terraserver_tile( connect, x, y, level, &tileBuffer );  
  ts_cleanup();

  ts_write_tile_ppm( "tile.pgm", tileBuffer, 128, 1 );

  tsmFreeTileBuffer( tileBuffer );

  tsmDisconnect( connect );
}
#endif

/*** EOF: terraserver.c ***/
