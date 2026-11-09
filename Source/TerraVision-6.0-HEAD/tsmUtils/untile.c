/*
 * Name: untile.c
 *
 * Description:
 *     Reads a single level of an OI or DEM pyramid and outputs a
 *     single image bitmap for that level; in effect, this takes all
 *     of the tiles for a pyramid level and stiches them back together
 *     into a single, non-tiled bitmap. This is one way to get source
 *     data from the input TerraVision tiled pyramid.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 40 May 1999.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: untile.c,v 1.7.2.1 2002/03/18 01:16:14 reddy Exp $
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
#include <math.h>
#include <tsm/tsm.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.7.2.1 $" )

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
 * Date: 30 May 1999
 *
 */

void usage( void )
{
  printf( "untile - release %s\n", APP_VERSION );
  printf( "\nusage: untile <pyramid-url> <image-name> [...]\n" );
  printf( "\nThis utility reads a single pyramid level and outputs it as\n" );
  printf( "a single PPM/PGM (for an OI) or raw bitmap (for a DEM)\n" );
  printf( "\nParameters:\n" );
  printf( " <pyramid-url> = URL/filename of the pyramid to be read\n" );
  printf( " <image-name> = the filename of the image to write\n" );
  printf( " -level <lev> = the pyramid level to convert (default=highest)\n" );
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
 *      opens the desired pyramid, and then attempts to write out the
 *      bitmap image to the specified filename;
 *
 * Externals: None
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 30 May 1999
 *
 */

int
main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams params;
  Pyramid             *tspec;
  SimpleTileSet       *tset;
  uchar               *tileBuffer, *buffer;
  int                 level = -1, bufferSize;
  int                 tWidth, tHeight, iWidth, iHeight, pSize;
  int                 tx, ty, iy, bytes, offset;
  int                 minTile[2], maxTile[2];
  TileTspec           *tile;
  FILE                *handle;
  char                filename[512];
  TsmImportFormat     format;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( argc < 3 ) usage();

  tsmGetArgInt( argc, argv, "-level", &level );
  params = tsmParseReadArgs( argc, argv );

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Display the SRI copyright message */

  printf( "UnTile %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open a connection to the specified dataset and get the tspec info */

  if ( ( connect = tsmConnect( argv[1], "r", params ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }
  tsmFreeParams( params );

  tspec  = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  tile   = &tspec->tileTspec;

  /* use the highest resolution level if no -level option given */

  if ( level < 0 ) level = tspec->minLevel;

  if ( level < tspec->minLevel || level > tspec->maxLevel ) {
    fprintf( stderr, "Invalid level specified. Must be %d - %d\n",
	     tspec->minLevel, tspec->maxLevel );
    exit( 1 );
  }

  /* calculate the various image/tile size variables */

  tset = &tspec->tspecLevel.val[level];

  iWidth  = tset->numValidPixel[0];
  iHeight = tset->numValidPixel[1];
  tWidth  = tile->numPixelsX;
  tHeight = tile->numPixelsY;
  pSize   = tile->numComponentsPixel * tile->numBytesPerComponent;

  /* allocate a buffer for one row of tiles */

  bufferSize = iWidth * pSize * tHeight;
  
  if ( ( buffer = (uchar *) malloc( bufferSize ) ) == NULL ) {
    fprintf( stderr, "Couldn't create buffer of %dK\n", bufferSize/1024 );
    tsmDisconnect( connect );
    exit( 1 );
  }
  
  /* work out the image format to write to */

  switch ( (TsmDataSetType) tsmGet( connect, TSM_DATASET_TYPE ) ) {
  case TSM_DEM_DATASET:
    format = TSM_IMPORT_RAW;
    break;
  case TSM_IMAGE_DATASET:
    format = (tile->numComponentsPixel == 1) ? TSM_IMPORT_PGM : TSM_IMPORT_PPM;
    break;
  default:
    fprintf( stderr, "Unknown dataset type for input pyramid!\n" );
    tsmDisconnect( connect );
    exit( 1 );
  }

  /* open the output file and dump out the appropriate image header */
  
  strcpy( filename, argv[2] );

  switch( format ) {
  case TSM_IMPORT_RAW:
    tsmAddFilenameExtension( filename, ".raw" );
    break;
  case TSM_IMPORT_LRBT:
    tsmAddFilenameExtension( filename, ".lrbt" );
    break;
  case TSM_IMPORT_PGM:
    tsmAddFilenameExtension( filename, ".pgm" );
    break;
  case TSM_IMPORT_PPM:
    tsmAddFilenameExtension( filename, ".ppm" );
    break;
  }

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't create %s\n", argv[2] );
    tsmDisconnect( connect );
    exit( 1 );
  }

  /* output the appropriate file header to the new image file */

  switch( format ) {
  case TSM_IMPORT_RAW:
  case TSM_IMPORT_LRBT:
    break;
  case TSM_IMPORT_PGM:
    fprintf( handle, "P5\n%d %d\n255\n", iWidth, iHeight );
    break;
  case TSM_IMPORT_PPM:
    fprintf( handle, "P6\n%d %d\n255\n", iWidth, iHeight );
    break;
  }

  /* calculate the minTile[2] and maxTile[2] values for the  */
  /* tileset. We do this rather than use the values from the */
  /* tspec file because it is possible (i.e. in the old Ft.  */
  /* Irwin dataset) to have the valid pixel size be way less */
  /* than the number of pixels, such that the valid pixels   */
  /* end before the last tile in the dataset.                */

  minTile[0] = tset->minTile[0];
  minTile[1] = tset->minTile[1];

  maxTile[0] = (int) ( ceil( tset->numValidPixel[0] / (float) tWidth ) )
                     + minTile[0] - 1;
  maxTile[1] = (int) ( ceil( tset->numValidPixel[1] / (float) tHeight ) )
                     + minTile[1] - 1;

  /* read the tiles and output the data to the new file using */
  /* an lrtb (left->right, top->bottom) byte ordering scheme. */
  /* N.B. this involves flipping the TerraVision lrbt data.   */

  tileBuffer = tsmAllocTileBuffer( connect );

  for ( ty = maxTile[1]; ty >= minTile[1]; ty-- ) {
    
    /* a little progress meter */

    printf( "Untiling... %d row(s) to go  \r", ty - minTile[1] + 1 );
    fflush( stdout );

    for ( tx = minTile[0]; tx <= maxTile[0]; tx++ ) {

      /* read the next tile from the dataset */

      if ( ! tsmReadTile( connect, tx, ty, level, tileBuffer ) )
	fprintf( stderr, "Unable to read tile (%d,%d,%d)\n",
		 tx, ty, level );

      /* work out how many pixels on each row of this tile */

      if ( tx == maxTile[0] ) {
	bytes = iWidth % tWidth;
	if ( bytes == 0 ) bytes = tWidth;
      } else 
	bytes = tWidth;

      bytes *= pSize;

      /* work out the x offset into the image buffer */

      offset = ( tx - minTile[0] ) * tWidth * pSize;

      /* insert it into the new image buffer */

      for ( iy = 0; iy < tHeight; iy++ )
	memcpy( buffer + (tHeight - iy - 1) * iWidth *pSize + offset,
		tileBuffer + iy * tile->totalPixels[0] * pSize,
		bytes );

    }

    /* flush out each row of tiles as we complete it */

    if ( ty == maxTile[1] ) {
      offset = ( tHeight - ( iHeight % tHeight ) ) * iWidth * pSize;
      bytes  = ( iHeight % tHeight ) * iWidth * pSize;
      fwrite( buffer + offset, bytes, 1, handle );
    } else
      fwrite( buffer, bufferSize, 1, handle );
  } 

  /* now clean up */

  printf( "                                   \r" );
  printf( "%d x %d image written to %s\n", iWidth, iHeight, filename );

  fclose( handle );
  tsmFreeTileBuffer( tileBuffer );
  tsmDisconnect( connect );

  return 0;
}
