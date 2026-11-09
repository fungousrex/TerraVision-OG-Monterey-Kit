/*
 * Name: calc_stats.c
 *
 * Description:
 *     This utility will read every tile in a pyramid in order to work out  
 *     the value of the min and max data point as well as the checksum
 *     arrays. These data are then written to the tspec file for the dataset.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 14 September 1997.
 *
 * Function List:
 *     void usage( void )
 *     int main( int argc, char **argv )
 *
 * Dependencies:
 *
 * Revision History:
 *     6 August 1998 - Martin Reddy
 *
 *     $Id: calc_stats.c,v 1.4 2000/11/27 23:57:30 reddy Exp $
 * 
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

#define APP_VERSION tsmCvsToString( "$Revision: 1.4 $" )

/*
 * Synopsis:
 *      void usage( void )
 *
 * Description:
 *      Displays the program usage information & description, then exits.
 *
 * Author: Martin Reddy
 * 
 * Date: 25 September 1997
 *
 */

void usage( void )
{
  printf( "calc_stats - release %s\n", APP_VERSION );
  puts( "\nusage: calc_stats <pyramid-url> [...]\n" );
  puts( "This utility will compute the min/max pixel values and the" );
  puts( "checksum array for a pyramid. The tspec file will be updated." );
  tsmDescribeArgs( TSM_ARG_INIT | TSM_ARG_READ );
  exit( 0 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty. This gets the URL from the command line,
 *      opens the desired dataset, and then attempts to read all of the
 *      tiles from the pyramid.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 24 September 1997
 *
 */

int
main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  Pyramid             *pyramid;
  SimpleTileSet       *tileset;
  TsmTileHeader       *tileHeader;
  uchar               *tileBuffer;
  int                 x, y, level, tiles;

  /* Check the command line arguments - display usage if invalid */
  /* we use the tsmApi's tsmParse* functions to let us support a */
  /* number of standard tsmApi command line options.             */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 ) usage();

  params = tsmParseReadArgs( argc, argv );

  /* Display the SRI copyright message */

  printf( "CalcStats %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Time how long it takes to make a connection to the dataset. */
  /* This includes loading the tspec information for the set.    */

  if ( ( connect = tsmConnect( argv[1], "rw", params ) ) == NULL ) {
    fprintf( stderr, "couldn't open dataset: %s\n", argv[1] );
    exit( 1 );
  }

  /* Get a pointer to the dataset's tspec structure so that we know */
  /* the dimensions of the dataset, e.g. number of levels, tiles,.. */

  pyramid = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Allocate a tile buffer and a header buffer to read the tiles */

  tileBuffer = tsmAllocTileBuffer( connect );
  tileHeader = tsmAllocTileHeader();

  if ( tileBuffer == NULL || tileHeader == NULL ) {
    fprintf( stderr, "couldn't allocate tile buffers\n" );
    exit( 1 );
  }

  /* initialise the min/max pixel and checksum arrays to be empty */

  pyramid->pixelTspec.minVal = -1.0;
  pyramid->pixelTspec.maxVal = -1.0;
  tsmMakeChecksum( &pyramid->identityTspec.checkSumLrbt,
		   pyramid->minLevel, pyramid->maxLevel );
  tsmMakeChecksum( &pyramid->identityTspec.checkSumJpeg,
		   pyramid->minLevel, pyramid->maxLevel );

  /* now go through every tile in each level and read it in */

  tiles = 0;

  for ( level = pyramid->minLevel; level <= pyramid->maxLevel; level++ ) {

    tileset = &pyramid->tspecLevel.val[level];

    for ( y = tileset->minTile[1]; y <= tileset->maxTile[1]; y++ ) {

      /* we use the tile request mechanisn to read tiles in  */
      /* so that we don't disadvantage DPSS transfers. We do */
      /* this by requesting a single row of tiles at a time  */

      tsmProgressReport( connect, "Level %d: %d%%", level,
			 100 * (y - tileset->minTile[1]) / 
			 (tileset->maxTile[1] - tileset->minTile[1] ) );

      tsmTileReqBegin( connect );

      for ( x = tileset->minTile[0]; x <= tileset->maxTile[0]; x++ ) {
	
	if ( tsmTileReqAdd( connect, x, y, level, TSM_NOW ) == FALSE ) {
	  fprintf( stderr, "Couldn't request tile (%d,%d) %d\n", x, y, level );
	  tsmDisconnect( connect );
	  exit( 1 );
	}
      }
      
      tsmTileReqEnd( connect );

      /* we've built the request list for this row of tiles, */
      /* now we will try and read in that many tiles - we    */
      /* don't care about the order here because we are only */
      /* interested in throughput. Otherwise we would have   */
      /* to check the tile's info in the tile header.        */

      for ( x = tileset->minTile[0]; x <= tileset->maxTile[0]; x++ ) {
	
	if ( tsmReadNextTile( connect, tileHeader, tileBuffer ) == FALSE ) {
	  fprintf( stderr, "Couldn't read tile (%d,%d) %d\n", x, y, level );
	  tsmDisconnect( connect );
	  exit( 1 );
	}

	tsmUpdateTileStats( connect, tileBuffer, level );

	tiles++;
      }

    }

  }

  /* Save the updated tspec file */

  tsmWriteTspec( connect );
  
  /* finish up: free buffers and disconnect from the dataset */

  tsmFreeTileBuffer( tileBuffer );
  tsmFreeTileHeader( tileHeader );
  tsmFreeParams( params );
  tsmDisconnect( connect );
  puts( "Updated pyramid's min/max pixel and checksums." );

  return 0;
}

/*** EOF: calc_stats.c ***/
