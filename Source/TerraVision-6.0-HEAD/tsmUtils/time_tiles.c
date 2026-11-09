/*
 * Name: time_tiles.c
 *
 * Description:
 *     Written to test the time it takes to read a dataset. This is a crude 
 *     utility, but could be used to get an idea of how much faster one data
 *     representation is than other, e.g. DPSS v localdisk, TIFF v .tiles, 
 *     old .tspecs v new tspec, etc.
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
 *     double timeEvent( int mode, char *message )
 *     int main( int argc, char **argv )
 *
 * Dependencies:
 *
 * Revision History:
 *     14 September 1997 - Martin Reddy
 *
 *     $Id: time_tiles.c,v 1.9.2.2 2002/03/14 05:16:07 reddy Exp $
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
#include <tsm/tsm.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.9.2.2 $" )

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
  printf( "time_tiles - release %s\n", APP_VERSION );
  puts( "\nusage: time_tiles <pyramid-url> [...]\n" );
  puts( "This utility will time how long it takes to connect to a dataset" );
  puts( "and then how long it takes to read all of the tiles in the dataset" );
  puts( "as a whole, or from a single level of the dataset.\n" );
  puts( " -multi <n> = change the read multiplicity to <n>" );
  puts( " -level <n> = read only this level of tiles from dataset" );
  puts( " -ignore_errors = keeping reading tiles even if error occurs" );
  tsmDescribeArgs( TSM_ARG_INIT | TSM_ARG_READ );
  exit( 0 );
}

/*
 * Synopsis:
 *      double timeEvent( int mode, char *message )
 *
 * Description:
 *
 *      a function to time how long something takes to execute. Simply
 *      call timeEvent(START_TIMER,"") before the function and then
 *      timeEvent(STOP_TIMER,"messsage") afterwards. The elapsed time
 *      will be output to stdout.  The message for STOP_TIMER is
 *      simply some identifying text such as the function name that
 *      was being timed.
 *
 * Author: Martin Reddy
 * 
 * Date: 13 March 1998
 *
 */

#define START_TIMER    0
#define STOP_TIMER     1

double
timeEvent( int mode, char *message )
{
  static PRIntervalTime startTime, stopTime;
  double elapsedTime = 0;
  
  switch ( mode ) {
  case START_TIMER:
    /*printf( "Timing... please wait\r" );*/
    fflush( stdout );
    startTime = PR_IntervalNow ();
    break;
  case STOP_TIMER:
    stopTime = PR_IntervalNow ();
    elapsedTime = tsmElapsedTime( startTime, stopTime );
    printf( "Time for %s = %1.4lf secs\n", message, elapsedTime );
    break;
  }
  return elapsedTime;
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

int main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  Pyramid             *pyramid;
  SimpleTileSet       *tileset;
  TsmTileHeader       *tileHeader;
  uchar               *tileBuffer;
  int                 x, y, level, tiles;
  int                 start_lev, end_lev;
  int                 ignore_errors;
  char                message[100];
  double              dt;

  extern int ghttp_debug;

  /* Check the command line arguments - display usage if invalid */
  /* we use the tsmApi's tsmParse* functions to let us support a */
  /* number of standard tsmApi command line options.             */

  tsmParseInitArgs( argc, argv );

  ghttp_debug = 0;
  
  if ( argc < 2 ) usage();

  params = tsmParseReadArgs( argc, argv );

  /* Display the SRI copyright message */

  printf( "TimeTiles %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Time how long it takes to make a connection to the dataset. */
  /* This includes loading the tspec information for the set.    */

  timeEvent( START_TIMER, "" );

  if ( ( connect = tsmConnect( argv[1], "r", params ) ) == NULL ) {
    fprintf( stderr, "couldn't open dataset: %s\n", argv[1] );
    exit( 1 );
  }

  timeEvent( STOP_TIMER, "tsmConnect()" );

  /* Get a pointer to the dataset's tspec structure so that we know */
  /* the dimensions of the dataset, e.g. number of levels, tiles,.. */

  pyramid = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );

  if ( tsmGetArgInt( argc, argv, "-level", &start_lev ) == TRUE ) {
    end_lev = start_lev;
    printf( "Reading all tiles in level %d\n", start_lev );
  } else {
    start_lev = pyramid->maxLevel;
    end_lev   = pyramid->minLevel;
    printf( "Reading all tiles in levels %d..%d\n", start_lev, end_lev );
  }

  ignore_errors =  tsmGetArg( argc, argv, "-ignore_errors" );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* now go through every tile in each level and read it in */

  timeEvent( START_TIMER, "" );
  for ( level = start_lev, tiles = 0; level >= end_lev; level-- ) {
    PRIntervalTime tileStart;

    tileset = &pyramid->tspecLevel.val[level];
    
    printf( " Level %d [%dx%d]\n", level,
	    tileset->maxTile[0] - tileset->minTile[0] + 1,
	    tileset->maxTile[1] - tileset->minTile[1] + 1);

    /* we use the tile request mechanisn to read tiles in  */
    /* so that we don't disadvantage DPSS transfers. We do */
    /* this by requesting a single row of tiles at a time  */

    tsmTileReqBegin( connect );

    for ( y = tileset->minTile[1]; y <= tileset->maxTile[1]; y++ ) {

      for ( x = tileset->minTile[0]; x <= tileset->maxTile[0]; x++ ) {
	
	if ( tsmTileReqAdd( connect, x, y, level, TSM_NOW ) == FALSE ) {
	  fprintf( stderr, "Couldn't request tile (%d,%d) %d\n", x, y, level );
	  if ( ! ignore_errors ) {
	    tsmDisconnect( connect );
	    exit( 1 );
	  }
	}
      }
    }

    tsmTileReqEnd( connect );

    /* we've built the request list for this row of tiles, */
    /* now we will try and read in that many tiles - we    */
    /* don't care about the order here because we are only */
    /* interested in throughput. Otherwise we would have   */
    /* to check the tile's info in the tile header.        */

    tileStart = PR_IntervalNow ();

    for ( y = tileset->minTile[1]; y <= tileset->maxTile[1]; y++ ) {
      for ( x = tileset->minTile[0]; x <= tileset->maxTile[0]; x++ ) {
	PRIntervalTime tileEnd;
	double tileElapsed;

	if ( tsmAllocReadNextTile( connect, &tileHeader, &tileBuffer ) == FALSE ) {
	  fprintf( stderr, "Couldn't read tile (%d,%d) %d\n", x, y, level );
	  if ( ! ignore_errors ) {
	    tsmDisconnect( connect );
	    exit( 1 );
	  }
	}

	tileEnd = PR_IntervalNow ();
	tileElapsed = tsmElapsedTime( tileStart, tileEnd );
	if (tileElapsed > 2) {
	  printf (" -- Stalled (%.1fs) on tile (%d,%d)\n", tileElapsed,
		  tsmTileHeaderX (tileHeader), tsmTileHeaderY (tileHeader));
	}
	tileStart = tileEnd;

	/* Throw them away */
	tsmFreeTileBuffer( tileBuffer );
	tsmFreeTileHeader( tileHeader );

	tiles++;
      }
    }
  }

  sprintf( message, "%d tsmReadNextTile()'s", tiles );
  dt = timeEvent( STOP_TIMER, message );
  /*printf ("=> %.3f tiles/s\n", tiles/dt);*/

  {
    TileTspec const *tile = &pyramid->tileTspec;
    int bytes_per_tile = (tile->totalPixels[0] * tile->totalPixels[1]
			  * tile->numComponentsPixel
			  * tile->numBytesPerComponent);
    float bytes = (float) (tiles * bytes_per_tile);
    float mbytes = bytes / (1<<20);

    printf ("%d bytes/tile => %.2fMB => %.3fMb/s\n",
	    bytes_per_tile, mbytes, 10*mbytes/dt);
  }

  /* finish up: free buffers and disconnect from the dataset */

  tsmFreeParams( params );
  tsmDisconnect( connect );

  return 0;
}

/*** EOF: tile_tiles.c ***/
