/*
 * Name: tsmChecksum.c
 *
 * Description: 
 *   Functions to checksum a pyramid in various ways.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmChecksum.c,v 1.15.6.1 2001/06/19 22:04:38 leei Exp $
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
 *
 */

#include "tsmInternal.h"
#include "tsmConnectionInt.h"

#define TSM_CHECKSUM_REQ_NONE  0
#define TSM_CHECKSUM_REQ_COL   1

#define TSM_INVALID_CHECKSUM   -1

static void tsmChecksumPyramidLevel(TsmHandle *tsmh, Pyramid *pyramid,
				    int level, int req_mode, int *checksum,
				    int *tiles);

/*
 * Synopsis:
 *   int tsmCalcTileChecksum( TsmConnection connect, uchar *tileBuffer )
 *
 * Doc++ Description: *//** 
 *   Calculates the checksum for a single tile buffer.
 *
 *   This function will take a tile buffer and the connection handle
 *   for the dataset it came from. It will then return a checksum 
 *   calculated from the data in that tile. If an error occurred during
 *   the checksum calculation then the function will return a value
 *   of TSM_INVALID_CHECKSUM.
 *
 * @param    connection the connection handle for the tile's dataset
 * @param    tileBuffer the tile buffer to be checksummed
 *
 * @return   the tile's checksum (or TSM_INVALID_CHECKSUM if error)
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      
 *
 */

int
tsmCalcTileChecksum( TsmConnection connect, const uchar *tileBuffer )
{
  static const char *funcName = "tsmCalcTileChecksum";

  if ( connect == NULL || tileBuffer == NULL ) {
    tsmError( funcName, "Connection and tile buffer must be non NULL!" );
    return TSM_INVALID_CHECKSUM;
  }

  if ( connect->tileBufferSize <= 0 ) {
    tsmError( funcName, "Connection doesn't know the tile size!" );
    return TSM_INVALID_CHECKSUM;
  }

  return tsmCheckSumTileBuffer( tileBuffer, connect->tileBufferSize );
}

/*
 * Synopsis:
 *   int tsmCalcTileStats( TsmConnection connect, const uchar *tileBuffer,
 *                         int *checksum, float *min_value, float *max_value )
 *
 * Description:
 *   Calculates the checksum for a tile buffer, and also the minimum and
 *   maximum pixel values. For 4-byte image data we apply the PixelTspec
 *   scale and offset to all values, but for 1-byte image data we assume
 *   that scale = 1.0 and offset = 0.0.
 *
 * @return   TRUE if successful, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  27 March 1998
 *
 * @see      tsmCalcTileChecksum
 *
 */

int
tsmCalcTileStats( TsmConnection connect, const uchar *tileBuffer,
		  int *checksum, float *min_value, float *max_value )
{
  static const char *funcName = "tsmCalcTileStats";
  Pyramid           *tspec;
  signed int        *demBuffer4;
  signed short      *demBuffer2;
  int               i, numPixels;
  int               min_int, max_int, x_int;
  float             min_float, max_float, x_float;
  float             scale, offset;

  /* Check that we have a valid connection handle */

  if ( connect == NULL || tileBuffer == NULL ) {
    tsmError( funcName, "Connection and tile buffer must be non NULL!" );
    return FALSE;
  }

  if ( ( numPixels = connect->tileBufferSize ) <= 0 ) {
    tsmError( funcName, "Connection doesn't know the tile size!" );
    return FALSE;
  }

  if ( ( tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC ) ) == NULL ) {
    tsmError( funcName, "Connection doesn't have a tspec structure!" );
    return FALSE;
  }

  /* Calculate the checksum value first */

  *checksum = tsmCheckSumTileBuffer( tileBuffer, connect->tileBufferSize );

  /* Now calculate the min and max pixel values, dependent on data type */

  switch ( tspec->tileTspec.numBytesPerComponent ) {
  case 1:

    /* Calculate min/max pixel value for 1-byte pixel data  */
    /* We assume that these data are not scaled or offset & */
    /* so can use integer math for the entire calculation.  */

    min_int = max_int = tileBuffer[0];

    for ( i = 1; i < numPixels; ++i ) {
      x_int = tileBuffer[i];
      if ( x_int > max_int ) max_int = x_int;
      if ( x_int < min_int ) min_int = x_int;
    }

    *max_value = (float) max_int;
    *min_value = (float) min_int;

    break;

  case 2:

    /* Calculate min/max pixel value for 2-byte pixel data */
    /* These values can have a floating point scale and    */
    /* offset applied to them, so we use f-p math here.    */

    demBuffer2 = (signed short *) tileBuffer;
    numPixels /= 2;
    scale     = tspec->pixelTspec.scale;
    offset    = tspec->pixelTspec.offset;
    x_float   = ( demBuffer2[0] * scale ) + offset; 
    min_float = max_float = ( x_float == -9999.0 ) ? 0.0 : x_float;

    for ( i = 1; i < numPixels; ++i ) {
      x_float = ( demBuffer2[i] * scale ) + offset;
      if ( x_float == -9999.0 ) x_float = 0.0;  /* -9999 = sea level */
      if ( x_float > max_float ) max_float = x_float;
      if ( x_float < min_float ) min_float = x_float;
    }

    *max_value = max_float;
    *min_value = min_float;

    break;

  case 4:

    /* Calculate min/max pixel value for 4-byte pixel data */
    /* These values can have a floating point scale and    */
    /* offset applied to them, so we use f-p math here.    */

    demBuffer4 = (signed int *) tileBuffer;
    numPixels /= 4;
    scale     = tspec->pixelTspec.scale;
    offset    = tspec->pixelTspec.offset;
    x_float   = ( demBuffer4[0] * scale ) + offset; 
    min_float = max_float = ( x_float == -9999.0 ) ? 0.0 : x_float;

    for ( i = 1; i < numPixels; ++i ) {
      x_float = ( demBuffer4[i] * scale ) + offset;
      if ( x_float == -9999.0 ) x_float = 0.0;  /* -9999 = sea level */
      if ( x_float > max_float ) max_float = x_float;
      if ( x_float < min_float ) min_float = x_float;
    }

    *max_value = max_float;
    *min_value = min_float;

    break;

  default:
    tsmError( funcName, "Unsupported image data. 1 or 4 byte pixels only" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *    int tsmUpdateTileStats( TsmConnection connect, const uchar *tileBuffer,
 *                            int level )
 *
 * Description:
 *    This function will take a tile buffer for a specified level in a
 *    dataset. It will then calculate the checksum for that tile buffer
 *    and also the minimum and maximum pixel value. These data are then
 *    integrated into the tspec structure for the dataset, i.e. for
 *    the checksum, the value is added to the total checksum for the
 *    specified level (so never call this function twice for the same
 *    tilebuffer), and then the total dataset checksum is recalculated.
 *    The min & max pixel values are entered into the tspec PixelTspec
 *    structure if they are smaller (or larger, resp.) than the current
 *    values (or if the current values are -1).
 *
 *    If is assumed that the min/max values and the checksum array will
 *    all be set to -1 by default. This function then adds values to
 *    these fields (or sets the field value if its the first time).
 *
 * Returns: TRUE if worked okay, FALSE if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 6 February 1998
 *
 */

int
tsmUpdateTileStats( TsmConnection connect, const uchar *tileBuffer, int level )
{
  /*static const char funcName[] = "tsmCalcTileStats";*/
  tsmChecksumArray  *array;
  Pyramid           *tspec;
  PixelTspec        *pixel;
  int               checksum, total, i;
  float             minval, maxval;

  /* Calculate the checksum, and min/max values for the tile */
  
  if ( ! tsmCalcTileStats( connect, tileBuffer, &checksum, &minval, &maxval ) )
    return FALSE;

  /* insert the new min and max values to the tspec if necessary */

  tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  pixel = &tspec->pixelTspec;

  if ( pixel->minVal == -1.0 || pixel->minVal > minval )
    pixel->minVal = minval;

  if ( pixel->maxVal == -1.0 || pixel->maxVal < maxval )
    pixel->maxVal = maxval;

  /* add the tile checksum to that of the specified level */

  if ( connect->format == TSM_JPEG_FORMAT )
    array = &tspec->identityTspec.checkSumJpeg;
  else
    array = &tspec->identityTspec.checkSumLrbt;

  if ( array->val[level] < 0 )
    array->val[level] = checksum;
  else
    array->val[level] = ( array->val[level] + checksum ) % TSM_CHECKSUM_MOD;

  /* update the pyramid wide checksum value (stored in the */
  /* array entry directly after the last level checksum).  */

  total = tspec->maxLevel + 1;
  array->val[total] = 0;

  for ( i = tspec->minLevel; i <= tspec->maxLevel; ++i )
    array->val[total] = (array->val[total] + array->val[i]) % TSM_CHECKSUM_MOD;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmCheckSumTileBuffer (const unsigned char *tileBuffer, int numBytes)
 *
 * Doc++ Description: *//** 
 *   Calculates the checksum for a single tile buffer.
 *
 * @param    tileBuffer the tile buffer to be checksummed
 * @param    numBytes the number of bytes in the tileBuffer
 *
 * @return   the tile's checksum
 *
 * @author   
 * @version  
 *
 * @see      tsmCheckSumTile
 *
 */

int
tsmCheckSumTileBuffer (const unsigned char *tileBuffer, int numBytes)
{
  int i, sum=0;

  for (i = 0; i < numBytes; ++i) {
    sum += tileBuffer[i];
  }

  return (sum);
}

/*
 * Synopsis:
 *      void tsmChecksumPyramidLevel( TsmHandle *, Pyramid *, int, int,
 *                                    int *, int * )
 *
 * Description:
 *      Calculates a checksum for a single level of a pyramid. The function
 *      returns the checksum and the number of tiles in the pyramid through
 *      call-by-reference parameters. You can perform a tile request before
 *      attempting to load the tiles (req_mode=TSM_CHECKSUM_REQ_[NONE|COL]).
 *
 *      You can specify whether you want to checksum the lrbt tiles or the
 *      jpeg tiles using tsmSetReadTileFormat() function.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 27 August 1997
 *
 */

static void
tsmChecksumPyramidLevel( TsmHandle *tsmh, Pyramid *pyramid, int level,
			 int req_mode, int *checksum, int *tiles )
{
  SimpleTileSet  *tileSet;
  TsmTileHeader  tileHeader;
  PRIntervalTime levelStartTime, levelEndTime;
  unsigned char  *tileBuffer;
  double         elapsedTime, MBytes, MBits;
  int            tileCheckSum, ix, iy, numBytesPerTile, status;
  FILE           *traceStream = pthr_trace_stream( NULL );

  /* Start the pyramid level timer if tracing */

  if ( TSMTRACEP(TSMMASK|1) )
      levelStartTime = PR_IntervalNow();

  /* Make a tile buffer to hold the data */

  tileBuffer      = tsmMakeTileBuffer( pyramid );
  numBytesPerTile = tsmNumBytesPerTile( pyramid );

  /* a spot of initialisation... */

  tileSet   = &pyramid->tspecLevel.val[level];
  *checksum = 0;
  *tiles    = tileSet->numTile[0] * tileSet->numTile[1];

  /* Now do the checksum calculation for the entire pyramid level */

  for ( ix = tileSet->minTile[0]; ix <= tileSet->maxTile[0]; ix++ ) {

    /* request the current column of tiles for this pyramid level */

    if ( req_mode == TSM_CHECKSUM_REQ_COL ) {
      tsmStartReqTiles( tsmh );
      for ( iy = tileSet->minTile[1]; iy <= tileSet->maxTile[1]; iy++ )
	tsmReqTile( tsmh, pyramid, ix, iy, level, TSM_NOW );
      tsmStopReqTiles( tsmh );
    }

    /* Then get each tile, compute it's checksum, and update level sum */

    for (iy = tileSet->minTile[1]; iy <= tileSet->maxTile[1]; iy++) {

      if ( req_mode == TSM_CHECKSUM_REQ_NONE ) 
	status = tsmGetTile( tsmh, pyramid, ix, iy, level, tileBuffer, 0);
      else
	status = tsmGetNextTile( tsmh, &tileHeader, tileBuffer, 0);

      if ( status == FALSE ) {
	fprintf( stderr, "error getting tile (%d,%d) level %d\n", ix,iy,level);
	return;
      } else {
	tileCheckSum = tsmCheckSumTileBuffer( tileBuffer, numBytesPerTile );
	*checksum = ( *checksum + tileCheckSum ) % TSM_CHECKSUM_MOD;
      }

    }

    /* Print a "." after each column is done if tracing */

    if ( TSMTRACEP(TSMMASK|2) ) {
      fprintf( traceStream, "." );
      fflush( traceStream );
      if ( TSMTRACEP(TSMMASK|3) ) { /* Print the cumulative checksum. */
	printf( "\nDone checksum of column %d/(%d-%d). ",
		ix, tileSet->minTile[0], tileSet->maxTile[0] );
	printf( "Level checksum so far = %d\n", *checksum );
      }
    }
  }

  /* Print out the level checksum & elapsed time if tracing */

  if ( TSMTRACEP(TSMMASK|1) ) {
    levelEndTime = PR_IntervalNow();
    elapsedTime = tsmElapsedTime( levelStartTime, levelEndTime );
    MBytes      = *tiles * numBytesPerTile / 1000000.0;
    MBits       = 8.0 * MBytes;

    fprintf( traceStream, "\nChecksum at level %d = %d;", level, *checksum );
    fprintf( traceStream, " number of tiles = %d (%4f Mbytes, %4f Mbits)\n",
	     *tiles, MBytes, MBits );
    fprintf( traceStream, "Elapsed time for this level = %4f seconds",
	     elapsedTime );
    fprintf( traceStream, " (%4f M bits per second)\n", MBits/elapsedTime );
    fflush( traceStream );
  }

}

/*
 * Synopsis:
 *      int tsmCalcChecksum( TsmHandle *, Pyramid *, tsmChecksumArray * )
 *
 * Doc++ Description: *//** 
 *      Calculate a pyramid's set of checksums.
 *      
 *      Calculates the entire checksum array for an entire pyramid, as
 *      well as for each of the pyramid levels. All of this information
 *      is put into a tsmChecksumArray.
 *      The checksum array that you pass in must be already allocated,
 *      e.g. with tsmMakeChecksum().
 *
 *      You can specify whether you want to
 *      check the lrbt or jpeg tiles by using tsmSetReadTileFormat().
 *
 * @param    tsmh the TSM session handle
 * @param    pyramid the Pyramid to be checksummed
 * @param    checksum the checksum array to fill in
 *
 * @return   TRUE if calculation was successful, FALSE if it failed.
 *
 * @author   Martin Reddy
 * @version  3 September 1997
 *
 * @see      tsmSetPyramidChecksum, tsmCheckSumTile, tsmCheckSumTileBuffer,
 *           tsmChecksumPyramid
 *
 */

int
tsmCalcChecksum (TsmHandle *tsmh, Pyramid *pyramid, tsmChecksumArray *checksum)
{
  const char *funcName = "tsmCalcChecksum";
  int   levelCheckSum, levelNumTiles, iLevel;
  int   pyramidCheckSum = 0, pyramidNumTiles = 0;

  /* check that we have a valid int array to store all checksums in */

  if ( checksum == NULL || checksum->val == NULL ) {
    tsmError( funcName, "NULL checksum array passed - must be allocated" );
    return FALSE;
  }

  /* Checksum the whole pyramid. */

  fflush( stdout );   /* for some reason we seg fault if we don't do */
                      /* this and no text has been output to stdout! */

  for ( iLevel = pyramid->minLevel; iLevel <= pyramid->maxLevel; iLevel++ ) {

    /* Calculate the checksum for this level of the pyramid */

    tsmChecksumPyramidLevel( tsmh, pyramid, iLevel, TSM_CHECKSUM_REQ_NONE,
			     &levelCheckSum, &levelNumTiles );

    /* Store it in the int array */
    checksum->val[iLevel] = levelCheckSum;

    /* Keep a running total of the entire pyramid's checksum too */

    pyramidCheckSum = (pyramidCheckSum + levelCheckSum) % TSM_CHECKSUM_MOD;
    pyramidNumTiles = pyramidNumTiles + levelNumTiles;

  }

  /* store the pyramid checksum as the last entry in the int array */

  checksum->val[pyramid->maxLevel+1] = pyramidCheckSum;

  return TRUE;
}

/*
 * Synopsis:
 *      int tsmSetPyramidChecksum( Pyramid *pyramid, tsmChecksumArray *array, 
 *                                 tspSymbol imgFormat )
 *
 * Doc++ Description: *//** 
 *   Insert a checksum array into a pyramid tspec.
 *
 * This function will take a tsmChecksumArray (e.g. as calculated using
 * the tsmCalcChecksum function) and will copy the values from this
 * array into the appropriate array in the specified pyramid structure.
 *
 * Currently there are two checksum arrays for a single pyramid: one for
 * the lrbt tiles, the other for the jpeg tiles. You specify which array
 * you want to write to by supplying a tspSymbol parameter indicating the
 * image format.
 *
 * The function will be smart and check to see if the currently allocated
 * checksum array in the pyramid is the right size for the new array (it
 * really should be, but ya never know!). If it's not the right size then
 * it will be free'd and malloc'd to the right size.
 *
 * @param    pyramid   the pyramid structure to copy the array into
 * @param    array     the checksum array to be copied into the pyramid
 * @param    imgFormat tspLrbtRaster or tspJPEGCompressed
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  30 October 1997
 *
 * @see      tsmCalcChecksum
 *
 */

int
tsmSetPyramidChecksum( Pyramid *pyramid, tsmChecksumArray *array, 
		       tspSymbol imgFormat )
{
  const char       *funcName = "tsmSetPyramidChecksum";
  tsmChecksumArray *curr_checksum;
  int              i;

  /* Grab a pointer to the particular checksum array so that we can  */
  /* just dereference this pointer to find the correct array and not */
  /* have to duplicate code for each checksum array.                 */

  if ( imgFormat == tspJPEGCompressed )
    curr_checksum = &pyramid->identityTspec.checkSumJpeg;
  else
    curr_checksum = &pyramid->identityTspec.checkSumLrbt;

  /* check to see if the array is the right size, if not then we free */
  /* it up and malloc a new array that will hold the new checksum data */
  
  if ( curr_checksum->len != array->len || curr_checksum->off != array->off ) {
    tsmOffsetArrayFree( *curr_checksum );
    tsmOffsetArrayMalloc( int, *curr_checksum, array->len, array->off );
    
    if ( curr_checksum->val == NULL ) {
      tsmError( funcName, "couldn't allocate new array (len=%d,off=%d)",
		array->len, array->len );
      return FALSE;
    }
  }

  /* now we can just copy the checksum data directly over into the pyramid */

  for ( i = array->off; i < array->len+array->off; ++i ) 
    curr_checksum->val[i] = array->val[i];

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmChecksumPyramid (TsmHandle *tsmh, Pyramid *pyramid)
 *
 * Doc++ Description: *//** 
 *   Calculate a pyramid's checksum.
 *      
 *  tsmChecksumPyramid computes and returns the checksum for a pyramid.
 *  The checksum is defined as the sum of all the bytes of all the
 *  tiles modulo TSM_CHECKSUM_MOD.
 *
 *  This variation gets and processes one tile at a time.
 *
 *  Trace levels:
 *
 *   1: Show pyramid id at start, and checksum/time at end.
 *   2: Show "." after each column is done, and checksum/time after each level.
 *   3: Show cumulative checksum after every column.
 *
 *  You can specify whether you want to
 *  check the lrbt or jpeg tiles by using tsmSetReadTileFormat().
 *
 * @param    tsmh the TSM session handle
 * @param    pyramid the Pyramid to be checksummed.
 *
 * @return   the pyramid's checksum
 *
 * @author   
 * @version  
 *
 * @see      tsmCalcChecksum, tsmChecksumPyramid_col
 *
 */

int
tsmChecksumPyramid (TsmHandle *tsmh, Pyramid *pyramid)
{
  int    levelCheckSum, pyramidCheckSum, iLevel;
  int    levelNumTiles, pyramidNumTiles;

  PRIntervalTime pyramidStartTime, pyramidEndTime;
  double elapsedTime, MBytes, MBits;

  FILE *traceStream = pthr_trace_stream (NULL);

  if (TSMTRACEP(TSMMASK|1)) {
    pthr_report_start (); {
      fprintf (traceStream, "For Pyramid '%s':\n", 
	       tsmPyramidName (pyramid));
      fprintf (traceStream, "Getting tspecs from '%s':\n",
	       tsmPyramidTspecBaseUrl (pyramid));
      fprintf (traceStream, "Getting tiles from '%s'.\n",
	       tsmPyramidTileBaseUrl
    (pyramid));
    }; pthr_report_end ();
  }
  
  if (TSMTRACEP(TSMMASK|1)) /* Start the Pyramid timer. */
    pyramidStartTime = PR_IntervalNow();

  /* Checksum the whole pyramid. */

  pyramidNumTiles = 0;
  pyramidCheckSum = 0;

  for (iLevel = pyramid->minLevel; iLevel <= pyramid->maxLevel; iLevel++) {

    tsmChecksumPyramidLevel( tsmh, pyramid, iLevel, TSM_CHECKSUM_REQ_NONE,
			     &levelCheckSum, &levelNumTiles );
	    
    pyramidCheckSum = (pyramidCheckSum + levelCheckSum) % TSM_CHECKSUM_MOD;
    pyramidNumTiles = pyramidNumTiles + levelNumTiles;
  } /* End pyramid processing. */

  if (TSMTRACEP(TSMMASK|1)) {
    /* Print out the Pyramid checksum & elapsed time. */
    pyramidEndTime = PR_IntervalNow();
    elapsedTime = tsmElapsedTime (pyramidStartTime, pyramidEndTime);    
    MBytes = pyramidNumTiles * tsmNumBytesPerTile(pyramid) / 1000000.0;
    MBits = 8 * MBytes;
    printf ("Checksum = %d;", pyramidCheckSum);
    printf (" number of tiles = %d (%4f Mbytes, %4f Mbits)\n",
	    pyramidNumTiles, MBytes, MBits);
    printf ("Elapsed time for the pyramid = %4f seconds (%4f Mbits per second)\n",
	    elapsedTime, MBits / elapsedTime);
    fflush (stdout);
  } /* End Pyramid trace. */

  return pyramidCheckSum;
}

/*
 * Synopsis:
 *    int tsmChecksumPyramid_col (TsmHandle *tsmh, Pyramid *pyramid)
 *
 * Doc++ Description: *//** 
 *   Calculate a pyramid's checksum.
 *      
 *  tsmChecksumPyramid_col computes and returns the checksum for a pyramid.
 *  The checksum is defined as the sum of all the bytes of all the
 *  tiles modulo TSM_CHECKSUM_MOD.
 *
 *  This variation requests one column of tiles at a time.
 *
 *  Trace levels:
 *
 *  1: Show pyramid id at start, and checksum/time at end.
 *  2: Show "." after each column is done, and checksum/time after each level.
 *  3: Show cumulative checksum after every column.
 *
 *   You can specify whether you want to
 *   check the lrbt or jpeg tiles by using tsmSetReadTileFormat().
 *
 * @param    tsmh the TSM session handle
 * @param    pyramid the Pyramid to be checksummed.
 *
 * @return   the pyramid's checksum
 *
 * @author   
 * @version  
 *
 * @see      tsmCalcChecksum, tsmChecksumPyramid
 *
 */

int
tsmChecksumPyramid_col (TsmHandle *tsmh, Pyramid *pyramid)
{
  int    levelCheckSum, pyramidCheckSum, iLevel;
  int    levelNumTiles, pyramidNumTiles;

  PRIntervalTime pyramidStartTime, pyramidEndTime;
  double elapsedTime, MBytes, MBits;

  FILE   *traceStream = pthr_trace_stream( NULL );


  if ( TSMTRACEP(TSMMASK|1) ) {
    pthr_report_start (); {
      fprintf( traceStream, "For Pyramid '%s':\n", tsmPyramidName(pyramid) );
      fprintf( traceStream, "Getting tspecs from '%s':\n", 
	       tsmPyramidTspecBaseUrl(pyramid) );
      fprintf( traceStream, "Getting tiles from '%s'.\n",
	       tsmPyramidTileBaseUrl(pyramid) );
    }; pthr_report_end ();
  }
  
  /* Start the Pyramid timer if tracing */

  if ( TSMTRACEP(TSMMASK|1) )
    pyramidStartTime = PR_IntervalNow();

  /* Checksum the whole pyramid. */

  pyramidNumTiles = 0;
  pyramidCheckSum = 0;

  for ( iLevel = pyramid->minLevel; iLevel <= pyramid->maxLevel; iLevel++ ) {

    tsmChecksumPyramidLevel( tsmh, pyramid, iLevel, TSM_CHECKSUM_REQ_COL,
			     &levelCheckSum, &levelNumTiles );
	    
    pyramidCheckSum = ( pyramidCheckSum + levelCheckSum ) % TSM_CHECKSUM_MOD;
    pyramidNumTiles = pyramidNumTiles + levelNumTiles;
  }

  /* Print out the Pyramid checksum & elapsed time if tracing */

  if ( TSMTRACEP(TSMMASK|1) ) {
    pyramidEndTime = PR_IntervalNow();
    elapsedTime = tsmElapsedTime (pyramidStartTime, pyramidEndTime);    
    MBytes      = pyramidNumTiles * tsmNumBytesPerTile(pyramid) / 1000000.0;
    MBits       = 8.0 * MBytes;

    fprintf( traceStream, "Checksum = %d;", pyramidCheckSum );
    fprintf( traceStream, " number of tiles = %d (%4f Mbytes, %4f Mbits)\n",
	    pyramidNumTiles, MBytes, MBits );
    fprintf( traceStream, "Elapsed time for the pyramid = %4f seconds",
	     elapsedTime );
    fprintf( traceStream, " (%4f Mbits per second)\n", MBits / elapsedTime );
    fflush( traceStream );
  }

  return pyramidCheckSum;
}

/*
 * Checksum routine for Internet Protocol family headers (C Version).
 *
 * Refer to "Computing the Internet Checksum" by R. Braden, D. Borman and
 * C. Partridge, Computer Communication Review, Vol. 19, No. 2, April 1989,
 * pp. 86-101, for additional details on computing this checksum.
 */

int                             /* return checksum in low-order 16 bits */
ip_checksum( register u_short *ptr, register int nbytes )
{
  /*static const char []funcName = "ip_checksum";*/
  register int      sum;            /* assumes int == 32 bits */
  u_short           oddbyte;
  register u_short  answer;         /* assumes u_short == 16 bits */

  /*
   * Our algorithm is simple, using a 32-bit accumulator (sum),
   * we add sequential 16-bit words to it, and at the end, fold back
   * all the carry bits from the top 16 bits into the lower 16 bits.
   */

  /*
    TSMTRACE ((TSMMASK|4, funcName,
    "computing checksum on %d bytes, data = %d %d %d %d",
    nbytes, ptr[0], ptr[1], ptr[2], ptr[3]));
  */

  sum = 0;
  while (nbytes > 1) {
    sum += ntohs(*ptr);
    ptr++;
    nbytes -= 2;
  }
  
  /* mop up an odd byte, if necessary */

  if (nbytes == 1) {
    oddbyte = 0;            /* make sure top half is zero */
    *((u_char *) &oddbyte) = *(u_char *)ptr;   /* one byte only */
    sum += oddbyte;
  }
  
  /*
   * Add back carry outs from top 16 bits to low 16 bits.
   */

  sum  = (sum >> 16) + (sum & 0x0000ffff); /* add high-16 to low-16 */
  sum += (sum >> 16);                     /* add carry */
  answer = ~sum;          /* ones-complement, then truncate to 16 bits */
  
  /*
    TSMTRACE ((TSMMASK|4, funcName, "computed checksum: %u", (unsigned int)answer));
  */
  return (int)answer;
}

