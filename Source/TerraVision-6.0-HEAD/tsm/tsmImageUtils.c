/*
 * Tile Set Manager Image utility routines.
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

#include "tsmInternal.h"

/*
 * tsmTileToRgb inserts a single tile into an RGB buffer at a
 *   specified (x,y) position.  The RGB buffer can be in either
 *   interleaved or sequential RGB format.
 *
 * When interLeaveP == TRUE, the pixels are output in interleaved order:
 *   rgbrgbrgb ... (this is the same order as used in color tiles)
 * When interLeaveP == FALSE, the pixels are output tile in sequential order:
 *    rrr...ggg...bbb...  (this is the order required for SGI program "frombin", e.g.)
 *
 * useTileRangeP and shadedReliefP are only used for DEM tiles.  The DEM pixels are
 *   first scaled to the range 0-255, and then either output directly or converted
 *   to a relief image.
 *
 * When useTileRangeP == TRUE, the min and max elevation within the tile are used to
 *   scale the output pixels.
 * When useTileRangeP == FALSE, the min and max elevation of the entire pyramid (as
 *   defined in the pyramid's pixelTspec) are used to scale the output pixels.
 *
 * When shadedReliefP == TRUE, the pixels are converted to a "shaded relief" image
 *   of the elevations.
 * When shadedReliefP == FALSE, the elevations are output directly.
 *
 * TRUE is returned upon success, and byteTileBuffer will contain the 
 * converted buffer.
 *
 * FALSE is returned upon failure, and a message will be printed to stderr.  */

int
tsmTileToRgb (unsigned char *tileBuffer, TileTspec *tileTspec, PixelTspec *pixelTspec,
	      unsigned char *rgbBuffer, int nxOut, int nyOut, int ixOut, int iyOut, int borderWidth,
	      int interLeaveP, int useTileRangeP, int shadedReliefP)
{
  const char *funcName = "tsmTileToRgb";
  int *intTileBuffer;
  int i, ix, iy, ixp1, iyp1, i00, i11, ir, ig, ib;
  int ixScale, iyScale, irOffset, igOffset, ibOffset;
  int minElevation, maxElevation, iValue;
  int numPixelsX, numPixelsY;
  int ixStart, iyStart, overlapX, overlapY;
  int totPixelsX, totPixelsY, totPixels;
  int nxIn, nyIn, numPixelsOut, rgbBufferSize;
  double elevationScale, elevationOffset, fValue;

  rgbBufferSize = (nxOut * nyOut * 3);

  numPixelsX = tileTspec->numPixelsX;
  numPixelsY = tileTspec->numPixelsY;

  overlapX = tileTspec->overLap[0];
  overlapY = tileTspec->overLap[1];

  totPixelsX = tileTspec->totalPixels[0];
  totPixelsY = tileTspec->totalPixels[1];
  totPixels = (totPixelsX * totPixelsY);

  ixStart = overlapX/2 + borderWidth;
  iyStart = overlapY/2 + borderWidth;

  ixOut = ixOut + borderWidth;
  iyOut = iyOut + borderWidth;

  nxIn = min (numPixelsX - borderWidth, nxOut - ixOut);
  nyIn = min (numPixelsY - borderWidth, nyOut - iyOut);

  numPixelsOut = nxOut * nyOut;

/* Ensure that the input storage format is tspLrbtRaster (may generalize later).*/

  /*
  if (tileTspec->storageFormat != tspLrbtRaster) {
    printf ( "ERROR in tsmTileToRgb: Unknown storage format.\n");
    tspSaveTileTspec ("stdout", tileTspec);
    return (FALSE);
  }
  */

  /* 
   * Compute scale and offset for converting the (ix,iy) index of the input tile
   * to red, green, blue indexes of the output tile as a function of 
   * the output tile size (nxOut, nyOut), output start (ixOut, iyOut), and interLeaveP.
   *
   * When interLeaveP == TRUE, the pixels in the output tile are in the order rgbrgbrgb ...
   * When interLeaveP == FALSE, the pixels in the output tile are in the order rrr...ggg...bbb...
   *
   * Use as:
   *
   * ir = (ix * ixScale) + (iy * iyScale) + irOffset;
   * ig = ir + igOffset;
   * ib = ir + ibOffset;
   * rgbBuffer[ir] = red;
   * rgbBuffer[ig] = green;
   * rgbBuffer[ib] = blue;
   */

  if (interLeaveP == TRUE) {
    ixScale = 3;
    iyScale = 3 * nxOut;
    irOffset = ((ixOut - ixStart) * ixScale) + ((iyOut - iyStart) * iyScale);
    igOffset = 1;
    ibOffset = igOffset + 1;
  }
  else {
    ixScale = 1;
    iyScale = 1 * nxOut;
    irOffset = ((ixOut - ixStart) * ixScale) + ((iyOut - iyStart) * iyScale);
    igOffset = numPixelsOut;
    ibOffset = igOffset + numPixelsOut;
  }

  /* Determine the pixel type, and convert accordingly */

  if ((tileTspec->componentNumberType == tspSignedByte) && 
      (tileTspec->numBytesPerComponent == 4) &&
      (tileTspec->numComponentsPixel == 1)) { 

    /* ================================================================
     * DEM image tile 
     * ================================================================*/

    if (TSMTRACEP(TSMMASK|1)) {
      pthr_report_start ();
      fprintf (pthr_trace_stream (NULL),
	       "Converting DEM tile (%d,%d).\n", totPixelsX, totPixelsY);
      pthr_report_end ();
    }

    /* point the integer tile buffer alias to the tile buffer */
    intTileBuffer = (int *) tileBuffer;

    /* Compute elevation scale and offset to produce a number in the range (0,1) using:
     * out = elevationOffset + elevationScale * in
     *
     * Use either the pyramid-wide min and max values, or compute it from the tile.
     */

    if (useTileRangeP == TRUE) {
      minElevation = intTileBuffer[0];
      maxElevation = intTileBuffer[0];
      for (i = 1; i < totPixels; i++) {
	if (intTileBuffer[i] < minElevation) minElevation = intTileBuffer[i];
	if (intTileBuffer[i] > maxElevation) maxElevation = intTileBuffer[i];
      }
      elevationScale = 1.0 / (maxElevation-minElevation);
      elevationOffset = -minElevation * elevationScale;
    }
    else {
      /* minVal and maxVal are the min and max elevations AFTER scaling
       * using scale and offset.  So reverse the scaling to get raw min and
       * max elevations, and compute elevationScale and elevationOffset.
       */
      minElevation = pixelTspec->minVal;
      maxElevation = pixelTspec->maxVal;
      minElevation = (minElevation - pixelTspec->offset) / pixelTspec->scale;
      maxElevation = (maxElevation - pixelTspec->offset) / pixelTspec->scale;
      elevationScale = 1.0 / (maxElevation-minElevation);
      elevationOffset = -minElevation * elevationScale;
    }

    if (shadedReliefP) {
      TSMTRACE ((TSMMASK|1, funcName, "... using shaded relief."));

      /* ================================================================
       * ... using shaded relief
       * ================================================================*/

      for (ix = ixStart; ix < ixStart+nxIn; ix++) {
	for (iy = iyStart; iy < iyStart+nyIn; iy++) {
	  /* ensure that we don't go outside the tile */
	  ixp1 = min (ix+1, totPixelsX-1);
	  iyp1 = min (iy+1, totPixelsY-1);
	  /* compute the shaded relief vector */
	  i00 = ix + (iy * totPixelsX);
	  i11 = ixp1 + (iyp1 * totPixelsX);
	  /* Since we don't know the scale of the difference, guess that its 10 x the elevation range */
	  fValue = 10.0 * elevationScale * (intTileBuffer[i00]-intTileBuffer[i11]);
	  /* convert it to integer in the range 0 to 255 */
	  iValue = rint ((fValue+1.) * 255.0 / 2.0);
	  iValue = max (min (iValue, 255), 0);
	  /* replicate the value into the output buffer */
	  ir = (ix * ixScale) + (iy * iyScale) + irOffset;
	  ig = ir + igOffset;
	  ib = ir + ibOffset;
	  rgbBuffer[ir] = iValue;
	  rgbBuffer[ig] = iValue;
	  rgbBuffer[ib] = iValue;
	  if ((ir >= rgbBufferSize) || (ig >= rgbBufferSize) || (ib >= rgbBufferSize)) {
	    fprintf (stdout, "WARNING: offsets too big for rgbBufferSize od %d :\n",
		     rgbBufferSize);
	    fprintf (stdout, "  ir = %d, ig = %d, ib = %d\n", ir, ig, ib);
	  }
	}
      }
    }

    else {
      TSMTRACE ((TSMMASK|1, funcName, "... using scaled elevation."));

      /* ================================================================
       * ... using scaled elevation
       * ================================================================*/
      
      for (ix = ixStart; ix < nxIn; ix++) {
	for (iy = iyStart; iy < nyIn; iy++) {
	  i = ix + (iy * totPixelsX);
	  /* scale the elevations to the range 0-255 */
	  fValue = intTileBuffer[i];
	  iValue = rint (255.0 * (elevationOffset + elevationScale * fValue));
	  /* ensure that they're in the proper range (in case of imprecise math) */
	  iValue = max (min (iValue, 255), 0);
	  /* replicate the value into the output buffer */
	  ir = (ix * ixScale) + (iy * iyScale) + irOffset;
	  ig = ir + igOffset;
	  ib = ir + ibOffset;
	  rgbBuffer[ir] = iValue;
	  rgbBuffer[ig] = iValue;
	  rgbBuffer[ib] = iValue;
	}
      }
    }
  }

  else if ((tileTspec->componentNumberType == tspUnsignedByte) && 
	   (tileTspec->numBytesPerComponent == 1) &&
	   (tileTspec->numComponentsPixel == 1)) { 

    /* ================================================================
     * mono image tile 
     * ================================================================*/

    if (TSMTRACEP(TSMMASK|1)) {
      pthr_report_start ();
      fprintf (pthr_trace_stream(NULL),
	       "Converting mono tile (%d,%d).\n", totPixelsX, totPixelsY);
      pthr_report_end ();
    }

    for (ix = ixStart; ix < nxIn; ix++) {
      for (iy = iyStart; iy < nyIn; iy++) {
	/* get the input value */
	i = ix + (iy * totPixelsX);
	iValue = tileBuffer[i];
	/* replicate the value into the output buffer */
	ir = (ix * ixScale) + (iy * iyScale) + irOffset;
	ig = ir + igOffset;
	ib = ir + ibOffset;
	rgbBuffer[ir] = iValue;
	rgbBuffer[ig] = iValue;
	rgbBuffer[ib] = iValue;
      }
    }
  }

  else if ((tileTspec->componentNumberType == tspUnsignedByte) && 
	   (tileTspec->numBytesPerComponent == 1) &&
	   (tileTspec->numComponentsPixel == 3)) { 

    /* ================================================================
     * rgb image tile 
     * ================================================================*/
    
    if (TSMTRACEP(TSMMASK|1)) {
      pthr_report_start ();
      fprintf (pthr_trace_stream (NULL),
	       "Converting rgb tile (%d,%d).\n", totPixelsX, totPixelsY);
      pthr_report_end ();
    }

    for (ix = ixStart; ix < nxIn; ix++) {
      for (iy = iyStart; iy < nyIn; iy++) {
	/* compute the input index */
	i = 3 * (ix + (iy * totPixelsX));
	/* copy the red, green, and blue values to the output buffer */
	ir = (ix * ixScale) + (iy * iyScale) + irOffset;
	ig = ir + igOffset;
	ib = ir + ibOffset;
	rgbBuffer[ir] = tileBuffer[i+2];
	rgbBuffer[ig] = tileBuffer[i+1];
	rgbBuffer[ib] = tileBuffer[i];
      }
    }
  }

  else { 

    /* ================================================================
     * unknown image tile 
     * ================================================================*/

    printf ( "ERROR in tsmTileToRgb: unknown pixel component configuration.\n");
    tspSaveTileTspec_fp (stdout, tileTspec);
    return (FALSE);
  }

  return (TRUE);
}

/*
 * tsmRgbBufferToGifFile writes an rgbBuffer (which must be in sequential, NOT
 *   interleaved order) into a GIF format file.
 *
 * tilePathName is the path name of the file, without any extension (e.g.,
 *  "/usr/local/tmp/tile-1234"
 *
 * Upon success, the path name of the GIF file is returned.  This is the same as
 *  the input tilePathName, but with the extension ".gif" added to it.
 *
 * Upon failure, NULL is returned.
 */

char *
tsmRgbBufferToGifFile (char *tilePathName, 
		       unsigned char *rgbBuffer, int nxOut, int nyOut)
{
  /*static char const funcName[] = "tsmRgbBufferToGifFile()";*/
  char gifPathName[1024];
  
  sprintf (gifPathName, "%s.gif", tilePathName);
  rgb_image_to_gif_file (gifPathName, rgbBuffer, nxOut, nyOut);
  return (tsmMemManagerStrdup (gifPathName));
}

#if 0
  char binPathName[1024], rgbPathName[1024], gifPathName[1024];
  char *fromBinCommand, *toGifCommand;
  char *rmBinCommand, *rmRgbCommand;
  int returnCode;
  FILE *fp;

  /* Write the file out into raw ("bin") format. */

  sprintf (binPathName, "%s.bin", tilePathName);
  fp = fopen (binPathName, "wb");
  if (!fp) {
    fprintf (stderr, "ERROR in tsmRgbBufferToGifFile: fopen failed.\n");
    return (NULL);
  }

  fwrite (rgbBuffer, 1, nxOut * nyOut * 3, fp);
  returnCode = ferror (fp);
  if (returnCode != 0) {
    fprintf (stderr, "ERROR in tsmRgbBufferToGifFile: fwrite to '%s' failed; ferror = %d.\n",
	     binPathName, returnCode);
    fclose (fp);
    return (NULL);
  }
  fclose (fp);

  /* Convert the bin file to SGI "RGB" format. */

  sprintf (rgbPathName, "%s.rgb", tilePathName);
  
  fromBinCommand = (char *)
     tsmMemManagerCalloc (1,
			  sizeof (binPathName) + sizeof (rgbPathName) + 100);
  if(!fromBinCommand) {
    tsmError (funcName, "Unable to allocate fromBinCommand");
    return NULL;
  }
  sprintf (fromBinCommand, "/usr/sbin/frombin %s %s %d %d 3", 
	   binPathName, rgbPathName, nxOut, nyOut);

  returnCode = system (fromBinCommand);
  if (returnCode != 0) {
    fprintf (stderr, "ERROR in tsmSaveTileBufferAsGIF:");
    fprintf (stderr, "system call \"%s\" returned %d",
	     fromBinCommand, returnCode);
    tsmMemManagerFree (fromBinCommand);
    return (NULL);
  }

  /* Finally, convert the SGI "RGB" file to GIF format. */

  sprintf (gifPathName, "%s.gif", tilePathName);

  toGifCommand = (char *) 
       tsmMemManagerCalloc (1, sizeof (rgbPathName) + sizeof (gifPathName) + 100);
  if(!toGifCommand) {
    tsmError (funcName, "Error allocating toGifCommand");
    return NULL;
  }
  sprintf (toGifCommand, "/usr/sbin/togif %s %s", rgbPathName, gifPathName);
  returnCode = system (toGifCommand);
  if (returnCode != 0) {
    fprintf (stderr, "ERROR in tsmSaveTileBufferAsGIF:");
    fprintf (stderr, "system call \"%s\" returned %d",
	     toGifCommand, returnCode);
    tsmMemManagerFree (fromBinCommand);
    tsmMemManagerFree (toGifCommand);
    return (NULL);
  }

  /* Delete the .bin and .rgb files, and free the strings. */

  rmBinCommand = tsmMemManagerCalloc (1, sizeof (binPathName) + 100);
  sprintf (rmBinCommand, "rm %s", binPathName);
  returnCode = system (rmBinCommand);
  if (returnCode != 0) {
    fprintf (stderr, "ERROR in tsmSaveTileBufferAsGIF:");
    fprintf (stderr, "system call \"%s\" returned %d",
	     rmBinCommand, returnCode);
    tsmMemManagerFree (fromBinCommand);
    tsmMemManagerFree (toGifCommand);
    tsmMemManagerFree (rmBinCommand);
    return (NULL);
  }


  rmRgbCommand = tsmMemManagerCalloc (1, sizeof (rgbPathName) + 100);
  sprintf (rmRgbCommand, "rm %s", rgbPathName);
  returnCode = system (rmRgbCommand);
  if (returnCode != 0) {
    fprintf (stderr, "ERROR in tsmSaveTileBufferAsGIF:");
    fprintf (stderr, "system call \"%s\" returned %d",
	     rmRgbCommand, returnCode);
    tsmMemManagerFree (fromBinCommand);
    tsmMemManagerFree (toGifCommand);
    tsmMemManagerFree (rmBinCommand);
    tsmMemManagerFree (rmRgbCommand);
    return (NULL);
  }

  tsmMemManagerFree (fromBinCommand);
  tsmMemManagerFree (toGifCommand);
  tsmMemManagerFree (rmBinCommand);
  tsmMemManagerFree (rmRgbCommand);

#endif


