/*
 * Name: dem_to_pgm.c
 *
 * Description:
 *     Reads a DEM pyramid and outputs a pgm image representing this
 *     dataset. The image is grayscale, where pixel intensity represents
 *     altitude (white=high). This utility will automatically strip out
 *     overlap pixels and is useful for visualising a DEM dataset. The
 *     scaling from elevation values to 8-bit intensities is done based
 *     upon the min/max pixel values in the DEM's tspec file.
 *
 *     See the function usage() for the command line syntax.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 9 March 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: dem_to_pgm.c,v 1.4.2.1 2002/03/18 01:16:14 reddy Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tsm/tsm.h>

#define IMAGE_FILE  "dem_img.pgm"  /* the default filename to write out to */
#define APP_VERSION tsmCvsToString( "$Revision: 1.4.2.1 $" )

/*
 * Synopsis:
 *   void usage( void )
 *
 * Description:
 *   Displays the program usage information & description, then exits.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * Date: 18 August 1998
 *
 */

void usage( void )
{
  printf( "dem_to_pgm - release %s\n", APP_VERSION );
  printf( "\nusage: dem_to_pgm <pyramid-url> <level> [<pgm-file>] [...]\n" );
  printf( "\nReads a DEM (elevation) pyramid level and outputs this as a\n" );
  printf( "grayscale image where white = high. Useful for visualisation.\n" );
  printf( "\nParameters:\n" );
  printf( " <pyramid-url>   = URL/filename of the pyramid to be read\n" );
  printf( " <level> = the level of the DEM pyramid to read\n" );
  printf( " <pgm-file> = filename of PGM file to write (def=%s)\n",IMAGE_FILE);
  tsmDescribeArgs( TSM_ARG_INIT | TSM_ARG_READ );
  printf( "\n" );
  exit( 0 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty. This checks the command line arguments,
 *      opens the desired pyramid, and then attempts to read the specified
 *      DEM level and output this to an image file.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 18 August 1998
 *
 */

int main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  Pyramid             *tspec;
  SimpleTileSet       *tileset;
  TileTspec           *tile;
  uchar               *tileBuffer, *rowBuffer, *out;
  char                *filename = IMAGE_FILE;
  int                 tx, ty, ix, iy, i, level, *in;
  int                 height, width;
  int                 bytes_to_read, tile_offset;
  float               min_val, max_val, offset, scale, elev;
  FILE                *handle;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( argc < 3 ) usage();

  level = atoi( argv[2] );
  if ( argc > 3 && argv[3][0] != '-' ) filename = argv[3];

  params = tsmParseReadArgs( argc, argv );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Display the SRI copyright message */

  printf( "DemToPgm %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open a connection to the specified dataset - make sure it's a DEM */

  if ( ( connect = tsmConnect( argv[1], "r", params ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }

  if ( tsmGet( connect, TSM_DATASET_TYPE ) != TSM_DEM_DATASET ) {
    fprintf( stderr, "ERROR: this program only works on DEM pyramids\n" );
    tsmDisconnect( connect );
    exit( 1 );
  }

  /* Get any convenience pointers and values describing the dem level */

  tspec   = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  tileset = &tspec->tspecLevel.val[level];
  tile    = &tspec->tileTspec;
  width   = tileset->numValidPixel[0];
  height  = tileset->numValidPixel[1];
  min_val = tspec->pixelTspec.minVal;
  max_val = tspec->pixelTspec.maxVal;
  scale   = tspec->pixelTspec.scale * 255.0 / ( max_val - min_val );
  offset  = tspec->pixelTspec.offset;

  /* Validate the level number that was supplied and the min/max value */

  if ( level < tspec->minLevel || level > tspec->maxLevel ) {
    fprintf( stderr, "ERROR: Invalid level number: %d\n", level );
    tsmDisconnect( connect );
    exit( 1 );
  }

  if ( min_val == max_val ) {
    fprintf( stderr, "ERROR: no min/max values in tspec. Try calc_stats.\n" );
    tsmDisconnect( connect );
    exit( 1 );
  }

  /* allocate a buffer to read dem tiles into and a row buffer */

  if ( ( tileBuffer = tsmAllocTileBuffer( connect ) ) == NULL ) {
    fprintf( stderr, "ERROR: Could not allocate a tile buffer!\n" );
    tsmDisconnect( connect );
    exit( 1 );
  }

  if ( ( rowBuffer = (uchar *) tsmMemMalloc( width + 33 ) ) == NULL ) {
    fprintf( stderr, "ERROR: Could not allocate a row buffer: %d\n", width );
    tsmDisconnect( connect );
    exit( 1 );
  }

  /* create the image file and output the PGM binary header */

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    fprintf( stderr, "ERROR: Could not write to image file: %s\n", filename );
    tsmDisconnect( connect );
    exit( 1 );
  }

  fprintf( handle, "P5\n%d %d\n255\n", width, height );

  /* Read in each line of the DEM and write it out to the image */

  for ( iy = height - 1; iy >= 0; --iy ) {
    for ( ix = 0; ix < width; ix += tile->numPixelsX ) {
      
      /* read the appropriate tile to get this bit of the row */
      /* N.B. this is extremely inefficient. We load the same */
      /* same tile for every row in the tile. Fine for now.   */

      tx = ix / tile->numPixelsX;
      ty = iy / tile->numPixelsY;
      
      if ( tsmReadTile( connect, tx, ty, level, tileBuffer ) == FALSE ) {
	fprintf( stderr, "ERROR: could read %d %d lev %d\n", tx, ty, level );
	tsmDisconnect( connect );
	exit( 1 );
      }

      /* how many bytes do we need from this tile? We validate this */
      /* number to make sure we never write more the width pixels.  */
      
      if ( tx < tileset->maxTile[0] )
	bytes_to_read = tile->numPixelsX;
      else
	bytes_to_read = width % tile->numPixelsX;

      bytes_to_read = min( width - ( tx * tile->numPixelsX ), bytes_to_read );

      /* now resample each 32-bit DEM value to an 8-bit intensity */

      tile_offset = ( iy % tile->numPixelsY ) * tile->totalPixels[1];
      in  = ((int *) tileBuffer ) + tile_offset;
      out = rowBuffer + tx * tile->numPixelsX;

      for ( i = 0; i < bytes_to_read; ++i, ++out, ++in ) {
	elev = ( (*in) - offset ) * scale;
	*out = (uchar) max( min( elev, 255.0 ), 0.0 );
      }
      
    }
    
    /* we've read in one entire row, so now write it out to the file */

    fwrite( rowBuffer, 1, width, handle );

    /* give the user some feedback so they know how far we've got */

    tsmProgressReport( connect, "Completed %d%%...",
		       ( height - iy ) * 100 / height );

  }

  /* tell us how far along we are - feedback is good. */

  printf( "Wrote %d x %d image to %s.\n", width, height, filename );

  /* clean up */

  fclose( handle );
  tsmMemFree( rowBuffer );
  tsmFreeTileBuffer( tileBuffer );
  tsmFreeParams( params );
  tsmDisconnect( connect );

  return 0;
}

/*** EOF: dem_to_pgm.c ***/
