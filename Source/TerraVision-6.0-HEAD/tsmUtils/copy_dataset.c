/*
 * Name: copy_dataset.c
 *
 * Description:
 *     Copies tile and tspec data from one location to another. The source
 *     and destination can be on different media, e.g. localdisk or DPSS.
 *     Options are provided to selectively copy only the tspec or only   
 *     the tile data. The PyramidUrls database will be updated for
 *     localdisk datasets whenever a tspec file is generated.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 10 March 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: copy_dataset.c,v 1.4.2.1 2001/01/19 17:29:42 reddy Exp $
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

#define HELP "http://www.tsmapi.com/examples/Docs/copy_dataset.html"
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
 * Date: 11 July 1997
 *
 */

void usage( void )
{
  printf( "copy_dataset - release %s\n", APP_VERSION );
  puts( "\nusage: copy_dataset <input-pyramid> <output-pyramid> [...]" );
  puts( "\nThis utility copies all the tile and tspec data from one" );
  puts( "location to another. You can selectively copy tspec or tiles." );
  puts( "The PyramidUrls dbase is updated (if nec.) when tspec created." );
  puts( "\nParameters:" );
  puts( " <input-pyramid> = URL/filename of the pyramid to be read" );
  puts( " <output-pyramid> = URL/filename of the pyramid to create" );
  puts( " -tspeconly = only copy the tspec file from the input set" );
  puts( " -tilesonly = only copy the tile data from the input set" );
  tsmDescribeArgs( TSM_ARG_INIT | TSM_ARG_READ | TSM_ARG_WRITE );
  puts( " -help = display the help web page for copy_dataset using netscape" );
  puts( " -info = display info for the dataset that was created" );
  puts( "" );
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
  TsmConnection       connect_in, connect_out;
  TsmConnectionParams params_in, params_out;
  int                 do_tiles, do_tspecs, do_info;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( tsmGetArg( argc, argv, "-help" ) == TRUE ) {
    puts( "Attempting to display help page using netscape..." );
    tsmShowUrl( HELP );
    exit( 0 );
  }

  if ( argc < 3 ) usage();

  do_tiles  = ! tsmGetArg( argc, argv, "-tspeconly" );
  do_tspecs = ! tsmGetArg( argc, argv, "-tilesonly" );
  do_info   = tsmGetArg( argc, argv, "-info" );

  params_in  = tsmParseReadArgs( argc, argv );
  params_out = tsmParseWriteArgs( argc, argv );

  if ( do_tiles == FALSE && do_tspecs == FALSE ) {
    fprintf( stderr, "Make up your mind! Nothing to copy\n" );
    exit( 1 );
  }

  /* Display the SRI copyright message */

  printf( "CopyDataset %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open a connection to the specified input dataset */

  if ( ( connect_in = tsmConnect( argv[1], "r", params_in ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }

  tsmParseConnectArgs( argc, argv, connect_in );

  /* Open a connection to the specified output dataset */

  if ( ( connect_out = tsmConnect( argv[2], "w", params_out ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't create %s\n", argv[2] );
    exit( 1 );
  }

  tsmParseConnectArgs( argc, argv, connect_out );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Now copy all of the relevant data from the input to the output set */
  /* N.B. it is important to do the tspec copy first so that the output */
  /* dataset has information about how to write its tile data.          */

  if ( do_tspecs == TRUE ) {
    printf( "Copying tspec data...\n" );
    tsmCopyTspec( connect_in, connect_out );
    
    printf( "Adding entry to PyramidUrls...\n" );
    tsmAddToLocalDbase( connect_out );
  }

  if ( do_tiles == TRUE ) {

    if ( (Pyramid *) tsmGet( connect_out, TSM_DATASET_TSPEC ) == NULL ) {
      printf( "ERROR: the output dataset must have a tspec file first\n" );
      exit( 1 );
    }

    printf( "Copying tile data...\n" );
    tsmCopyAllTiles( connect_in, connect_out );
  }

  /* If the user supplied the -info option then we will output some */
  /* information about the dataset that we just created. We defer   */
  /* this until now so that we have all information available.      */

  if ( do_info == TRUE ) {
    printf( "Output dataset:\n" );
    tsmDescribeConnection( connect_out );
  }

  /* clean up */

  tsmFreeParams( params_in );
  tsmFreeParams( params_out );

  tsmDisconnect( connect_out );
  tsmDisconnect( connect_in );

  return 0;
}

/*** EOF: copy_dataset.c ***/
