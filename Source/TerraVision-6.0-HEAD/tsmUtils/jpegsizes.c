/*
 * Name: jpegsizes.c (Primary File)
 *
 * Description:
 *     Calculates the compressed size of each tile in a JPEG compressed
 *     TIFF file. Information is provided that includes the smallest and 
 *     largest compressed tile size, the mean tile size, and the total
 *     number of compressed bytes. The operation can work on a single
 *     pyramid level or an entire pyramid. A verbose setting will output
 *     compressed size for every tile encountered.
 *
 * Function List:
 *     void usage(void)
 *     void parseArgs( int argc, char **argv );
 *     void analysePyramidLevel( TsmHandle *, Pyramid *, int, int );
 *     int  main(int, char**)
 *
 * Dependencies:
 *     libtsmApi (library for all tile I/O)
 *     libtiff (library for reading/writing TIFF v6.0 images)
 *     ...
 *
 * Revision History:
 *     26 August 1997 - Martin Reddy (Initial version)
 *
 *     $Id: jpegsizes.c,v 1.4 2000/11/27 23:57:31 reddy Exp $
 * 
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmApi.com/
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

int do_level = -1;                /* pyramid level to convert. -1 = all */
int do_detail = FALSE;           /* give verbose detail for each level? */

typedef unsigned char uchar;

/*
 * Synopsis:
 *      void usage( void )
 *
 * Description:
 *      Displays the program usage information & description, then exits.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

void
usage( void )
{
  printf( "jpegsizes Release %s\n", APP_VERSION );
  tsmPrintReleaseInfo( stdout );
  puts( "\nusage: jpegsizes <pyramid-url> [-level <num>] [-verbose]" );
  puts( "\nThis utility can be used to report the compressed size of each" );
  puts( "tile in a JPEG compressed TIFF file for a pyramid. The utility" );
  puts( "can work over an entire pyramid, or just a single pyramid level." );
  puts( "Information provided includes smallest and largest compressed" );
  puts( "tile size, and mean tile size. Also reported is the compression" );
  puts( "percentage for each level, i.e. what is the percentage of the size" );
  puts( "of the jpeg pyramid level compared to the lrbt version" );
  puts( "" );
  puts( "   <pyramid-url> = file: URL or local filename for pyramid" );
  puts( "   -level <num>  = analyse only the specified level, otherwise all" );
  puts( "   -verbose      = get compressed size for every tile in a pyramid" );
  puts( "" );
  exit( 1 );
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
 * Externals: do_level, do_detail
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

void
parseArgs( int argc, char **argv )
{
  int i;

  if ( argc < 2 ) usage();

  for ( i = 2; i < argc; i++ ) {
    if ( strcmp( argv[i], "-level" ) == 0 ) {
      if ( i < argc-1 )
	do_level = atoi( argv[++i] );
      else
	fprintf( stderr, "no value given for -level option.\n" );
    } else if ( strcmp( argv[i], "-verbose" ) == 0 ) {
      do_detail = TRUE;
    } else
      fprintf( stderr, "unknown option: %s\n", argv[i] );
  }
}

/*
 * Synopsis:
 *      void analysePyramidLevel( TsmHandle *, Pyramid *, int, int )
 *
 * Description:
 *      analyses the filesize of every tile in a single tile level.
 *      You can control the verbosity of the output (detail param).
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

void 
analysePyramidLevel( TsmHandle *tsmh, Pyramid *pyramid, int level, int detail )
{
  SimpleTileSet *tileset;
  int  width, height;
  int  ix, iy;
  int  min_size = -1, max_size = -1, curr_size, total_files = 0, mean_size;
  int  uncomp_tile_bytes;
  long total_bytes = 0, uncomp_total_bytes;
  
  /* Check to see if the TIFF file exists */

  if ( tsmIsTIFFPresent( pyramid, level, tspJPEGCompressed ) ) {

    /* get the number of tiles to be converted */

    tileset = &pyramid->tspecLevel.val[level];
    width   = tileset->numTile[0];
    height  = tileset->numTile[1];

    uncomp_tile_bytes  = tsmNumBytesPerTile(pyramid);

    for ( iy = 0; iy < height; iy++ ) {

      for ( ix = 0; ix < width; ix++ ) {

	/* get the compressed size of the current tile */

	curr_size = tsmGetJPEGTileSize( pyramid, level, ix, iy );

	/* update our stats if there was no error getting the tile size */

	if ( curr_size > 0 ) {
	  if ( min_size < 0 ) {
	    min_size = curr_size;
	    max_size = curr_size;
	  } else {
	    min_size = min( min_size, curr_size );
	    max_size = max( max_size, curr_size );
	  }

	  total_files++;
	  total_bytes += curr_size;

	  if ( detail ) {
	    printf( "tile (%d,%d) = %d bytes (%d%% of %d)\n", ix, iy,
		    curr_size, curr_size * 100 / uncomp_tile_bytes,
		    uncomp_tile_bytes );
	  }

	}

      }
    }

    /* report the stats for the pyramid level */

    mean_size = ( total_files == 0 ) ? 0 : total_bytes / total_files;
    uncomp_total_bytes = (long) ( uncomp_tile_bytes * total_files );

    printf( "Level %d: min = %d, max = %d, mean = %d bytes",
	    level, min_size, max_size, mean_size );
    if ( uncomp_total_bytes > 0 )
      printf( ", cmprsd = %ld%%", total_bytes * 100 / uncomp_total_bytes );
    printf( " (%d tiles)\n", total_files );
    
  } else if ( do_level >= 0 )
    printf( "Level %d JPEG TIFF not exist\n", level );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty.
 *
 * Externals: do_detail, do_level
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

int main( int argc, char **argv )
{
  TsmHandle *tsmh;
  Pyramid *pyramid;
  int level, start_level, end_level;
  char pyramid_url[257], tsmh_name[257];

  /* Do any initialisation */

  tsmSetTraceLevel( 0 );

  /* Check the command line arguments - display usage if invalid */

  parseArgs( argc, argv );

  strcpy( pyramid_url, argv[1] );
  tsmExtractTsmAndPyramidName( pyramid_url, tsmh_name, NULL );

  /* Display the SRI copyright message */

  printf( "JpegSizes %s, Copyright (c) 2000 SRI International. "
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

  /* work out which levels to work over. */

  if ( do_level < 0 ) {
    start_level = pyramid->minLevel;
    end_level   = pyramid->maxLevel;
  } else {
    start_level = do_level;
    end_level   = do_level;
  }

  /* go through each pyramid level and convert its data to TIFF format */
  /* and then update the storage-format field of the tile.tspec data   */

  for ( level = start_level; level <= end_level; level++ ) {
    analysePyramidLevel( tsmh, pyramid, level, do_detail );
  }

  tsmClose( tsmh );
  return 0;
}

/*** EOF: jpegsizes.c ***/
