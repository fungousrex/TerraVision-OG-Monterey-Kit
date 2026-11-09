/*
 * Name: get_tspec.c
 *
 * Description:
 *     Reads the tspec file from a dataset and the outputs this to stdout.
 *     You can redirect this to a file if you want to save the tspec to
 *     disk. The normal tsmApi read options are available so you can select
 *     where you want to read the tspec from for DPSS and localdisk datasets.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 28 March 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: get_tspec.c,v 1.4.2.1 2001/07/09 20:04:47 reddy Exp $
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
#include <tsm/tsm.h>

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
 * Date: 28 March 1998
 *
 */

void usage( void )
{
  printf( "get_tspec - release %s\n", APP_VERSION );
  printf( "\nusage: gettile <dataset-url> [...]\n" );
  printf( "\nThis utility reads the tspec file from the specified dataset\n" );
  printf( "and can displays this to stdout.\n" );
  printf( "\nParameters:\n" );
  printf( " <dataset-url>   = URL/filename of the dataset to be read\n" );
  printf( " -info = display information about the dataset only\n" );
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
 *      tspec from the pyramid.
 *
 * Externals: None
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 28 March 1998
 *
 */

int main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  int                 do_info;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 ) usage();

  params = tsmParseReadArgs( argc, argv );

  do_info = tsmGetArg( argc, argv, "-info" );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Display the SRI copyright message */

  printf( "GetTspec %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open a connection to the specified dataset */

  if ( ( connect = tsmConnect( argv[1], "r", params ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }

  /* tell us a bit about the dataset if -info specified */

  if ( do_info == TRUE ) {
    printf( "Dataset:\n" );
    tsmDescribeConnection( connect );
    return 0;
  }

  /* Otherwise, output the tspec file to stdout */

  tsmPrintTspec( connect );

  tsmDisconnect( connect );

  return 0;
}

/*** EOF: get_tspec.c ***/
