/*
 * Name: get_tile.c
 *
 * Description:
 *     Reads a single tile from a dataset and saves this out to a PBM
 *     format file in the current directory called "tile.ppm".  This
 *     will also display various bits of information about the
 *     connection such as it's dimensions, tile sizes, name, etc.
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
 *     $Id: get_tile.c,v 1.5.2.2 2002/03/18 01:16:14 reddy Exp $
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

#define TILE_FILE  "tile"    /* the file to write out if -save given */
#define APP_VERSION tsmCvsToString( "$Revision: 1.5.2.2 $" )

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
 * Date: 11 July 1997
 *
 */

void usage( void )
{
  printf( "get_tile - release %s\n", APP_VERSION );
  printf( "\nusage: get_tile <pyramid-url> <tx> <ty> <lev> [...]\n" );
  printf( "\nThis utility reads a single tile from the specified dataset\n" );
  printf( "and can save it to a PBM file. Also shows some connection info\n" );
  printf( "\nParameters:\n" );
  printf( " <pyramid-url>   = URL/filename of the pyramid to be read\n" );
  printf( " <tx> <ty> <lev> = the coordinate and level of the tile\n" );
  printf( " -save = save the tile to the PBM file: %s.ppm\n", TILE_FILE );
  printf( " -raw = save the raw tile data to the file : %s.raw\n", TILE_FILE );
  printf( " -noinfo = don't display the dataset/tile information\n" );
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
 *      opens the desired pyramid, and then attempts to read the desired
 *      tile from the pyramid.
 *
 * Externals: None
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 11 July 1997
 *
 */

int main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  uchar               *tileBuffer;
  int                 tx, ty, level;
  int                 do_save, do_raw, do_noinfo;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( argc < 5 ) usage();

  tx    = atoi( argv[2] );
  ty    = atoi( argv[3] );
  level = atoi( argv[4] );

  params = tsmParseReadArgs( argc, argv );

  do_save   = tsmGetArg( argc, argv, "-save" );
  do_raw    = tsmGetArg( argc, argv, "-raw" );
  do_noinfo = tsmGetArg( argc, argv, "-noinfo" );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Display the SRI copyright message */

  printf( "GetTile %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* No read multiplicity needed. */ 

  if ( ! params ) tsmInitParams( &params );
  tsmSetParam( params, TSM_PARAM_READ_MULTI, 1 );

  /* Open a connection to the specified dataset */

  if ( ( connect = tsmConnect( argv[1], "r", params ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }

  /* tell us a bit about the dataset and the tile about to be read */

  if ( do_noinfo == FALSE ) {
    printf( "Dataset:\n" );
    tsmDescribeConnection( connect );
    printf( "Tile:\n" );
    printf( "  Coord   = (%d,%d)\n", tx, ty );
    printf( "  Level   = %d\n", level );
    printf( "  Size    = %d bytes\n", tsmGet( connect, TSM_TILE_SIZE ) );
  }

  /* Try to read the specified tile from the dataset */

  if ( tsmAllocReadTile( connect, tx, ty, level, &tileBuffer ) == FALSE ) {
    fprintf( stderr, "ERROR: Could not read the tile!\n" );
    tsmDisconnect( connect );
    exit( 1 );
  } 

  printf( "Tile was read from the dataset successfully\n" );

  /* Save the tile out to tile.ppm if -save option supplied */

  if ( do_save == TRUE ) {
    tsmSaveImage( connect, tileBuffer, TILE_FILE, TSM_PPM );
    printf( "Tile saved to PBM file (%s.ppm)\n", TILE_FILE );
  }

  if ( do_raw == TRUE ) {
    char filename[256];
    FILE *handle;

    sprintf( filename, "%s.raw", TILE_FILE );
    if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
      fprintf( stderr, "ERROR: couldn't write raw tile data\n" );
      tsmDisconnect( connect );
      exit( 1 );
    }
    
    fwrite( tileBuffer, 1, tsmGet( connect, TSM_TILE_SIZE ), handle );
    fclose( handle );
    printf( "Raw tile data saved to file (%s.raw)\n", TILE_FILE );
  }

  /* clean up */

  tsmFreeTileBuffer( tileBuffer );
  tsmFreeParams( params );
  tsmDisconnect( connect );

  return 0;
}

/*** EOF: get_tile.c ***/
