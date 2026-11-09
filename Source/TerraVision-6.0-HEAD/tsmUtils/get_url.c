/*
 * Name: get_url.c
 *
 * Description:
 *     A simple little program to get any named file on localdisk or
 *     or an http server by its URL. For example, if you want to get
 *     a file on an http server, but your internet browser won't let 
 *     you, e.g. it is a CSS stylesheet, then you can use this command
 *     to just download the file without any interpretation.
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
 *     $Id: get_url.c,v 1.6.2.1 2001/09/14 21:22:22 reddy Exp $ 
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
#include <prinit.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.6.2.1 $" )

/*
 * Synopsis:
 *      void usage( void )
 *
 * Description:
 *      Displays the program usage information & description, then exits.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * Date: 20 November 1997
 *
 */

void usage( void )
{
  printf( "get_url - release %s\n", APP_VERSION );
  printf( "\nusage: get_url <url> [-o <filename>] [-proxy <url>]\n" );
  printf( "\nThis utility will grab a file on local disk or on an http\n" );
  printf( "server and save it to the desired filename. If no filename is\n" );
  printf( "specified, then the filename from the URL is used.\n" );
  printf( " -o <filename> = sets the filename the remote file is saved to\n" );
  printf( " -proxy <url> = sets proxy url, e.g. http://foo.bar:80/\n\n" );
  exit( 0 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 20 November 1997
 *
 */

int main( int argc, char **argv )
{
  FILE *infile, *outfile;
  char filename[512], proxy[512], *slash, curr_char;
  int  bytes_written = 0, i;

  /* do we have enough arguments? */

  if ( argc < 2 ) usage();

  /* Initialise the NSPR library */

  PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);

  /* work out the filename to save to */

  filename[0] = proxy[0] = '\0';

  if ( argc > 2 ) {

    /* check for -proxy or -o arguments */

    for ( i = 1; i < argc; i++ ) {
      if ( strcmp( argv[i], "-proxy" ) == 0 )
	strcpy( proxy, argv[++i] );
      else if ( strcmp( argv[i], "-o" ) == 0 )
	strcpy( filename, argv[++i] );
    }

  } 

  if ( strlen( filename ) == 0 ) {
    if ( ( slash = strrchr( argv[1], '/' ) ) == NULL ) {
      fprintf( stderr, "Why would you want to copy a file over itself?\n");
      exit( 1 );
    }

    strcpy( filename, ++slash );
	
    if ( strlen( filename ) == 0 ) {
      fprintf( stderr, "Cannot copy directories - only files\n" );
      exit( 1 );
    }
  }

  /* Display the SRI copyright message */

  printf( "GetURL %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );
  
  /* set the proxy, if specified */

  if ( strlen( proxy ) > 0 ) {
    fprintf( stderr, "Using proxy server %s\n", proxy );
    tsmSetHttpProxy( proxy );
  }


  /* try to open the local/http file for reading */

  if ( ( infile = tsmOpenStream( argv[1], "r" ) ) == NULL ) {
    fprintf( stderr, "Cannot open: %s\n", argv[1] );
    exit( 1 );
  }

  /* try to create the output file */

  if ( ( outfile = tsmOpenStream( filename, "wb" ) ) == NULL ) {
    fprintf( stderr, "Cannot write: %s\n", filename );
    tsmCloseStream( infile );
    exit( 1 );
  }

  /* copy the file over */

  while ( TRUE ) {
    curr_char = fgetc( infile );
    if ( feof( infile ) ) break;
    fputc( curr_char, outfile );
    bytes_written++;
  }

  /* all done */

  fprintf( stderr, "File downloaded to %s (%d bytes)\n", filename,
	   bytes_written );
  
  tsmCloseStream( infile );
  tsmCloseStream( outfile );

  return 0;
}

/*** EOF: get_url.c ***/
