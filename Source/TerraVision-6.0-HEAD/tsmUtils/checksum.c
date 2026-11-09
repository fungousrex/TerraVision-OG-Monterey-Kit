/*
 * Name: checksum.c
 *
 * Description:
 *     A utility for manipulating the checksum fields of a pyramid.
 *     You can calculate a set of checksums for the pyramid, write these
 *     to the tspec, clear the tspec values, read the tspec values, and
 *     compare the calculated values against the tspec values.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 3 September 1997.
 *
 * Function List:
 *      void usage( void )
 *      void displayChecksum( int *, int, tspSymbol, char * )
 *      void parseArgs( int argc, char **argv )
 *      void doChecksum( TsmHandle *, Pyramid *, tspSymbol )
 *      int main( int argc, char **argv )
 *
 * Revision History:
 *     3 September 1997 - Martin Reddy (Initial version)
 *
 *     $Id: checksum.c,v 1.4 2000/11/27 23:57:30 reddy Exp $
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

#define MODE_CALC  1               /* calculate and display the checksums */
#define MODE_WRITE 2           /* calculate checksums and write to tspecs */
#define MODE_READ  3                    /* read checksums from the tspecs */
#define MODE_CHECK 4     /* compare calculated checksum with tspec values */
#define MODE_CLEAR 5     /* compare calculated checksum with tspec values */

/*
 * Synopsis:
 *      void usage( void )
 *
 * Description:
 *      Displays the program usage information & description, then exits.
 *
 * Author: Martin Reddy
 *
 * Date: 26 August 1997
 *
 */

void
usage( void )
{
  printf( "checksum Release %s\n", APP_VERSION );
  puts( "\nusage: checksum <pyramid-url> [...]" ); 
  puts( "\nThis utility can be used to calculate the checksum for a pyramid" );
  puts( "Options allow you to write the checksum value to the tspec once it" );
  puts( "has been calculated, or to compare the calculated checksum with" );
  puts( "the one currently stored in the tspec. N.B Current limitation is" );
  puts( "that only raw tile data can be checksummed, not JPEG data.\n" );
  puts( " <pyramid-url> = file: URL or local filename for pyramid" );
  puts( " -calc = (default) calculate and display checksum" );
  puts( " -read = display the checksum values from the tspec" );
  puts( " -write = calc checksum then write value to the tspec" );
  puts( " -check = calc checksum then compare with value in tspec" );
  puts( " -clear = write -1 to each checksum in the tspec" );
  tsmDescribeArgs( TSM_ARG_INIT );
  puts( "" );
  exit( 1 );
}

/*
 * Synopsis:
 *      void displayChecksum( tsmChecksumArray, tspSymbol, char * )
 *
 * Description:
 *      Display an array of checksum values with an optional message.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 3 September 1997
 *
 */

void
displayChecksum( tsmChecksumArray *checksum, tspSymbol imgFmt, char *message )
{
  int i;

  printf( "checksum-%s", ( imgFmt == tspJPEGCompressed ) ? "jpeg" : "lrbt" );

  for ( i = 0; i < checksum->len; i++ ) {
    printf( " " );
    if ( i > 0 ) printf( "[%d]=", i-1 );
    printf( "%d", checksum->val[i+checksum->off] );
  }

  if ( message != NULL && message[0] != '\0' )
    printf( " (%s)", message );

  printf( "\n" );
  
}

/*
 * Synopsis:
 *      void parseArgs( int argc, char **argv )
 *
 * Description:
 *      parses the command line options for the utility. Outputs a help
 *      text if not enough params. Updates various global variables
 *      when options are present.
 *
 * Returns: the action to perform (e.g. MODE_CALC, etc.)
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

int
parseArgs( int argc, char **argv )
{
  int i;
  int do_action = MODE_CALC; /* default action to do */

  if ( argc < 2 ) usage();

  tsmParseInitArgs( argc, argv );   /* support tsmApi initialisation options */

  for ( i = 2; i < argc; i++ ) {
    if ( strcmp( argv[i], "-write" ) == 0 ) {
      do_action = MODE_WRITE;
    } else if ( strcmp( argv[i], "-read" ) == 0 ) {
      do_action = MODE_READ;
    } else if ( strcmp( argv[i], "-calc" ) == 0 ) {
      do_action = MODE_CALC;
    } else if ( strcmp( argv[i], "-check" ) == 0 ) {
      do_action = MODE_CHECK;
    } else if ( strcmp( argv[i], "-clear" ) == 0 ) {
      do_action = MODE_CLEAR;
    } else
      fprintf( stderr, "unknown option: %s\n", argv[i] );
  }

  return do_action;
}

/*
 * Synopsis:
 *      void doChecksum( TsmHandle *, Pyramid *, int, tspSymbol )
 *
 * Description:
 *      Does the checksum action for the specified (open) pyramid.
 *      The actual action is dependent upon the value of do_action.
 *      We currently supporting:
 *         - read & display current tspec checksum values
 *         - calculate & display checksums
 *         - calculate checksums and write to tspec
 *         - clear the checksums to -1 and write to tspec
 *         - calculate checksums and compare with values in tspec
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 3 September 1997
 *
 */

void 
doChecksum( TsmHandle *tsmh, Pyramid *pyramid, int do_action,
	    tspSymbol imgFormat )
{
  IdentityTspec    *ident = &pyramid->identityTspec;
  tsmChecksumArray checksum, original, *curr_checksum;
  int              i, curlev, levels, same;

  /* Are we dealing with the lrbt or the jpeg checksum array? */

  if ( imgFormat == tspJPEGCompressed )
    curr_checksum = &ident->checkSumJpeg;
  else
    curr_checksum = &ident->checkSumLrbt;
  
  levels = pyramid->tspecLevel.len;
  
  /* if we only want to read the current checksum then we can do that */
  /* straight off and not have to do any actual calculation.          */

  if ( do_action == MODE_READ ) {
    displayChecksum( curr_checksum, imgFormat, "read" );
    return;
  }

  /* allocate a checksum array for the calculation */

  if ( ! tsmMakeChecksum( &checksum, pyramid->minLevel, pyramid->maxLevel ) ) {
    fprintf( stderr, "couldn't allocate checksum array\n" );
    exit( 1 );
  }

  /* clearing the checksum data can be done without calculating. */
  /* tsmMakeChecksum() sets all entries to -1 by default anyway. */

  if ( do_action != MODE_CLEAR ) {

    /* calculate the checksum */

    printf( "calculating checksums...\r" );
    fflush( stdout );

    if ( tsmCalcChecksum( tsmh, pyramid, &checksum ) == FALSE ) {
      fprintf( stderr, "error calculating checksum\n" );
      exit( 1 );
    }
    
  }

  /* now do whatever function we have been requested to do */

  switch ( do_action ) {

  case MODE_WRITE :
  case MODE_CLEAR :

    /* write the calculated checksum values to the pyramid tspec, then */
    /* display the new checksum values for the user to see             */

    tsmSetPyramidChecksum( pyramid, &checksum, imgFormat );

    displayChecksum( &checksum, imgFormat, "written" );

    break;

  case MODE_CHECK :

    /* compare the calculated checksum values with the tspec values */

    if ( curr_checksum == NULL || curr_checksum->val == NULL ) {
      printf( "pyramid has no checksum data\n" );
    } else {

      original = *curr_checksum;
    
      for ( i = 0, same = TRUE; i <= levels; i++ ) {
	curlev = i + original.off;

	if ( original.val[curlev] == -1 ) {
	  if ( i == 0 ) {
	    printf( "pyramid checksum not calculated for tspec yet\n" );
	  } else {
	    printf( "level %d checksum not calculated for tspec yet\n", i-1 );
	  }
	  same = FALSE;
	} else if ( checksum.val[curlev] != original.val[curlev] ) {
	  if ( i == 0 ) {
	    printf( "pyramid checksums differ (%d != %d)\n", 
		    checksum.val[curlev], original.val[curlev] );
	  } else {
	    printf( "level %d checksums differ (%d != %d)\n", i-1,
		    checksum.val[curlev], original.val[curlev] );
	  }
	  same = FALSE;
	}
      }

      if ( same == TRUE ) 
	printf( "%s pyramid checksums are okay\n",
		( imgFormat == tspLrbtRaster ) ? "lrbt" : "jpeg" );
    }

    free( checksum.val );
    break;

  case MODE_CALC :
  default :

    /* the default action is just to print out the checksum values */

    displayChecksum( &checksum, imgFormat, "calc" );

    /*free( checksum.val );*/
    break;
  }

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
 * Date: 26 August 1997
 *
 */

int
main( int argc, char **argv )
{
  TsmHandle *tsmh;
  Pyramid   *pyramid;
  char      pyramid_url[257], tsmh_name[257];
  int       do_action;

  /* Check the command line arguments - display usage if invalid */

  do_action = parseArgs( argc, argv );

  strcpy( pyramid_url, argv[1] );
  tsmExtractTsmAndPyramidName( pyramid_url, tsmh_name, NULL );

  /* Display the SRI copyright message */

  printf( "Checksum %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open the desired pyramid */

  if ( ( tsmh = tsmOpen( tsmh_name ) ) == NULL ) {
    fprintf( stderr, "%s: couldn't open tile set\n", argv[0] );
    exit( 0 );
  }

  if ( ( pyramid = tsmGetPyramid( tsmh, pyramid_url ) ) == NULL ) {
    fprintf( stderr, "%s: couldn't open pyramid\n", argv[0] );
    exit( 0 );
  }

  /* calculate the checksums for the pyramid */

  doChecksum( tsmh, pyramid, do_action, tspLrbtRaster );
  
  /* write the new tspec files if -write or -clear options used */

  if ( do_action == MODE_WRITE || do_action == MODE_CLEAR )
    tsmPutPyramid( tsmh, pyramid, pyramid->tspecBasePurl->url );

  tsmClose( tsmh );
  return 0;
}

/*** EOF: checksum.c ***/
