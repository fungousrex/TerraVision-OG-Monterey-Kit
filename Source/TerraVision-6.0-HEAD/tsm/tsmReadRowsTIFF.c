/*
 * Name: tsmReadRowsTIFF.c
 *
 * Description: 
 *   Import functions for supporting the TIFF and GeoTIFF image formats.
 *
 * Function List:
 *
 * Revision History:
 *   Created Jul 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsTIFF.c,v 1.4.2.2 2001/05/11 23:44:39 reddy Exp $
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

#include <math.h>

#include "tsmReadRowsTIFF.h"

#include <libgeotiff/libxtiff/xtiffio.h>
#include <libgeotiff/geotiffio.h>
#include <libgeotiff/geotiff.h>
#include <libgeotiff/geokeys.h>
#include <libgeotiff/geonames.h>

static int tsmReadRowsTIFF_readHeader( TsmReadRowsTIFF *this );

TsmReadRowsTIFF *
tsmReadRowsTIFF_new( const char *filename )
{
  static const char *funcName = "tsmReadRowsTIFF_new";
  TsmReadRowsTIFF *this;

  if ( ( this = tsmMemCalloc( sizeof(TsmReadRowsTIFF ), 1 ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate TsmReadRowsTIFF instance." );
    return NULL;
  }

  if ( ( this->tiffFile = TIFFOpen( filename, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open TIFF file named %s", filename );
    tsmReadRowsTIFF_destroy( &this );
    return NULL;
  }

  if ( ! tsmReadRowsTIFF_readHeader( this ) ) {
    tsmReadRowsTIFF_destroy( &this );
    return NULL;
  }

  this->compsPerPixel = 3;
  this->bytesPerComp = 1;

  return this;
}

void
tsmReadRowsTIFF_destroy( TsmReadRowsTIFF **this )
{
  if ( this != NULL && *this != NULL ) {
    if ( (*this)->tiffFile != NULL )
      TIFFClose( (*this)->tiffFile );

    tsmMemFree( *this );
    *this = NULL;
  }
}

static int
tsmReadRowsTIFF_readHeader( TsmReadRowsTIFF *this )
{
  /* static const char *funcName = "tsmReadRowsTIFF_readHeader"; */
  uint16            bitsPerComp;

  if ( this == NULL || this->tiffFile == NULL )
    return FALSE;

  /* get the information we need from the TIFF file */

  TIFFGetField( this->tiffFile, TIFFTAG_PHOTOMETRIC, &this->photometric );
  TIFFGetField( this->tiffFile, TIFFTAG_IMAGEWIDTH, &this->width );
  TIFFGetField( this->tiffFile, TIFFTAG_IMAGELENGTH, &this->height );
  TIFFGetField( this->tiffFile, TIFFTAG_SAMPLESPERPIXEL, &this->compsPerPixel);
  TIFFGetField( this->tiffFile, TIFFTAG_BITSPERSAMPLE, &bitsPerComp );

  this->bytesPerComp = bitsPerComp / 8;

  this->scanline_size = TIFFScanlineSize( this->tiffFile );

  /* get a reference to the palette lookup tables */
  
  if ( this->photometric == PHOTOMETRIC_PALETTE )
    TIFFGetField( this->tiffFile, TIFFTAG_COLORMAP, &this->r, &this->g,
		  &this->b );
  else
    this->r = this->g = this->b = NULL;

  this->currentRow = 0;

  return TRUE;
}

int
tsmReadRowsTIFF_read( void *buffer, int startRow, int numberOfRows,
		      int numberOfBytes, void *closure )
{
  static const char *funcName = "tsmReadRowsTIFF_read";
  TsmReadRowsTIFF   *this = (TsmReadRowsTIFF *) closure;
  unsigned char     *indexBuffer, *currBuffer, *ptr;
  int               size, i;

  if ( this == NULL || buffer == NULL || this->tiffFile == NULL )
    return FALSE;

  /* allocate a buffer to store all of the request rows */

  size = numberOfRows * this->scanline_size;

  /* do we need an extra buffer for the 8-bit palette lookups? */

  if ( this->photometric == PHOTOMETRIC_PALETTE ) {

    /* allocate buffer to read in 8-bit values */

    if ( ( indexBuffer = tsmMemMalloc( size ) ) == NULL ) {
      tsmError(funcName, "Couldn't allocate buffer to read from TIFF file." );
      return FALSE;
    }

  } else {

    /* we can just read 24-bit values directly into the client's buffer */

    indexBuffer = (unsigned char *) buffer;

  }

  /* read in numberOfRows of pixels from the file */

  currBuffer = indexBuffer;
  for ( i = 0; i < numberOfRows; i++ ) {

    /* read the next scanline from the TIFF file */

    if ( TIFFReadScanline( this->tiffFile, currBuffer, this->currentRow++, 0 )
	 == -1 ) {
      tsmError( funcName, "Error during TIFFReadscanline" );
      return FALSE;
    }

    currBuffer += this->scanline_size;
    
  }

  /* do we have to do the 8-bit palette lookups? */

  if ( this->photometric == PHOTOMETRIC_PALETTE ) {
  
    /* scanline is in 8-bit palette lookup form - convert to RGB */

    ptr = buffer;
    
    for ( i = 0; i < size; i++ ) {
      uchar value = indexBuffer[i];
      *(ptr++) = this->r[value];
      *(ptr++) = this->g[value];
      *(ptr++) = this->b[value];
    }
    
    /* free the temporary buffer when done */

    tsmMemFree( indexBuffer );
  }
  
  return TRUE;
}

/*
 * Synopsis:
 *  int tsmParseCreateParams_TIFF( char *image_name, TsmTileParams *tile,
 *                                 TsmTspecParams *tspec )
 *
 * Description:
 *   Opens the TIFF file and extracts as much information from it 
 *   as we need. From this we should be able to get the image's
 *   width and height, the number of bands, etc. In addition, if
 *   the file is actually a GeoTIFF file, then we can extract the
 *   georeferencing information such as coordinate system,
 *   resolution, etc.
 *
 * Author: Martin Reddy
 *
 * Date: 14 September 1999
 *
 */

int
tsmParseCreateParams_TIFF( char *image_name, TsmTileParams *tile,
			  TsmTspecParams *tspec )
{
  const char *funcName = "tsmParseCreateParams_TIFF";
  TIFF       *tif = NULL;         /* TIFF-level descriptor */
  GTIF       *gtif = NULL;        /* GeoKey-level descriptor */
  int        versions[3];
  int        count;
  int        south_hemi = FALSE;  /* is UTM coord in southern hemisphere? */
  int        pcs_okay = FALSE;    /* TRUE if we recognise the PCS */
  geocode_t  model, pcs;          /* all key-codes are of this type */
  double     *pixelscale;
  double     *tiepoints;
  uint32     value32;
  uint16     value16;
  char       buffer[128] = "(no citation)";

  TSMTRINC(( TSMMASK|1, 1 ));

  /* Open TIFF descriptor to read GeoTIFF tags */

  if ( ( tif = XTIFFOpen( image_name, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open tiff image file: %s", image_name );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }
  
  /* Don't support planar (PLANARCONFIG_SEPARATE) data - only chunky */

  TIFFGetField( tif, TIFFTAG_PLANARCONFIG, &value16 );

  if ( value16 != PLANARCONFIG_CONTIG ) {
    tsmError( funcName, "TIFF image is in planar format, not chunky!" );
    XTIFFClose( tif );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* Get the base TIFF info for this image */

  TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &value32 );
  tspec->width = tile->width = value32;

  TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &value32 );
  tspec->height = tile->height = value32;

  TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &value16 );
  tspec->comps = tile->comps = (int) value16;

  TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &value16 );
  tile->bytespercomp = (int) value16 / 8;

  TSMTRACE(( TSMMASK|1, funcName, "TIFF image: %d x %d, %d comps (%d bytes)",
	     tile->width, tile->height, tile->comps, tile->bytespercomp ));

  /* Only read palette colour, RGB, or greyscale image data */

  TIFFGetField( tif, TIFFTAG_PHOTOMETRIC, &value16 );

  if ( value16 != PHOTOMETRIC_RGB && value16 != PHOTOMETRIC_PALETTE &&
       value16 != PHOTOMETRIC_MINISBLACK ) {
    tsmError( funcName, "TIFF image not RGB, gray, or color palette (%d)",
	      (int)value16 );
    XTIFFClose( tif );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* if the image is colour palette, then we will create RGB from it */

  if ( value16 == PHOTOMETRIC_PALETTE && tile->bytespercomp == 1 )
    tspec->comps = tile->comps = 3;

  /* Open the GeoTIFF Key parser; keys will be read at this time. */

  if ( ( gtif = GTIFNew( tif ) ) == NULL ) {
    tsmError( funcName, "Could not parse as GeoTIFF file: %s", image_name );
    XTIFFClose( tif );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }
  
  /* Get the GeoTIFF directory info */

  GTIFDirectoryInfo( gtif, versions, 0 );

  if ( versions[0] > 1 ) {
    tsmError( funcName, "GeoTIFF version %d is unsupported", versions[0] );
    XTIFFClose( tif );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* get the model type - this will fail if file is not a GeoTIFF */

  if ( GTIFKeyGet( gtif, GTModelTypeGeoKey, &model, 0, 1) == 0 ) {

    /* Model type is undefined. This means the file is probably not */
    /* a GeoTIFF. It is still a valid TIFF so this is not an error. */

    XTIFFClose( tif );
    TSMTRACE(( TSMMASK|1, funcName, "image is a TIFF but not a GeoTIFF" ));
    TSMTRINC(( TSMMASK|1, -1 ));
    return TRUE;
  }

  /* now we check to see what coordinate system model type we have */

  switch ( model ){
  case ModelTypeProjected:

    /* get the type of the particular projected coordinate system */

    if ( GTIFKeyGet( gtif, ProjectedCSTypeGeoKey, &pcs, 0, 1 ) ) {

      /* Do we have a WGS84 UTM coordinate in the N/S hemisphere? */

      if ( pcs >= PCS_WGS84_UTM_zone_1N && pcs <= PCS_WGS84_UTM_zone_60N ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS84 UTM" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS84_UTM_zone_1N + 1;
	pcs_okay         = TRUE;
      }

      if ( pcs >= PCS_WGS84_UTM_zone_1S && pcs <= PCS_WGS84_UTM_zone_60S ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS84 UTM (S)" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS84_UTM_zone_1S + 1;
	south_hemi       = TRUE;
	pcs_okay         = TRUE;
      }

      /* Do we have a WGS72 UTM coordinate in N/S hemisphere? */

      if ( pcs >= PCS_WGS72_UTM_zone_1N && pcs <= PCS_WGS72_UTM_zone_60N ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS72 UTM" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS72_UTM_zone_1N + 1;
	pcs_okay         = TRUE;
      }

      if ( pcs >= PCS_WGS72_UTM_zone_1S && pcs <= PCS_WGS72_UTM_zone_60S ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS72 UTM (S)" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS72_UTM_zone_1S + 1;
	south_hemi       = TRUE;
	pcs_okay         = TRUE;
      }

      /* Do we have a WGS72BE UTM coordinate in N/S hemisphere? */

      if ( pcs >= PCS_WGS72BE_UTM_zone_1N && pcs <= PCS_WGS72BE_UTM_zone_60N ){
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS72BE UTM" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS72BE_UTM_zone_1N + 1;
	pcs_okay         = TRUE;
      }

      if ( pcs >= PCS_WGS72BE_UTM_zone_1S && pcs <= PCS_WGS72BE_UTM_zone_60S ){
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses WGS72BE UTM (S)"));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_WGS72BE_UTM_zone_1S + 1;
	south_hemi       = TRUE;
	pcs_okay         = TRUE;
      }

      /* Do we have a NAD27 UTM coordinate */

      if ( pcs >= PCS_NAD27_UTM_zone_3N && pcs <= PCS_NAD27_UTM_zone_22N ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses NAD27 UTM" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_NAD27_UTM_zone_3N + 3;
	pcs_okay         = TRUE;
      }

      /* Do we have a NAD83 UTM coordinate */

      if ( pcs >= PCS_NAD83_UTM_zone_3N && pcs <= PCS_NAD83_UTM_zone_23N ) {
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses NAD83 UTM" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = pcs - PCS_NAD83_UTM_zone_3N + 3;
	pcs_okay         = TRUE;
      }

    }

    /* Take the citation geokey as our description field */

    GTIFKeyGet( gtif, PCSCitationGeoKey, buffer, 0, 128 );
    tspec->description = tsmMemStrdup( buffer );

    /* ERDAS Imagine encodes its georeference info as a string */
    
    if ( pcs == 32767 /*user defined*/ || !strncmp( buffer, "IMAGINE", 7 ) ) {
      char erdas[1024], *match;

      /* get the fill citation string */
      GTIFKeyGet( gtif, PCSCitationGeoKey, erdas, 0, 1024 );
      
      /* parse out the UTM Zone if defined */
      if ( ( match = strstr( erdas, "UTM Zone" ) ) != NULL ) {
	match += 9;
	TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF data uses UTM (ERDAS)" ));
	tspec->coord_sys = TSM_UTM_COORDSYS;
	tspec->zone      = atoi( match );
	south_hemi       = ( match[1] == 'S' || match[2] == 'S' );
	pcs_okay         = TRUE;
      }

      if ( ! pcs_okay )
	tsmError( funcName, "Unknown ERDAS IMAGINE file:\n%s\n---\n", erdas );
    }

    /* if none of the above, then tell the user */

    if ( ! pcs_okay )
      tsmError( funcName, "Unsupported GeoTIFF PCS: %s", tspec->description );

    break;

  case ModelTypeGeographic:
    
    /* currently we assume that all geographic data is in lat/long    */
    /* with the WGS84 datum/ellipsoid, and that values are in degrees */
    /* i.e. we don't check the GeogGeodeticDatumKey, GeogAngularUnits-*/
    /* GeoKey, or GeogPrimeMeridianGeoKey.                            */

    tspec->coord_sys = TSM_LATLONG_COORDSYS;

    /* Take the citation geokey as our description field */

    GTIFKeyGet( gtif, GeogCitationGeoKey, buffer, 0, 128 );
    tspec->description = tsmMemStrdup( buffer );

    break;

  case ModelTypeGeocentric:
    tsmError( funcName, "Geocentric GeoTIFF model type not supported" );
    break;

  default:
    tsmError( funcName, "Unsupported GeoTIFF model type" );
    break;
  }

  /* Work out the resolution of the imagery. This is */
  /* defined by the GeoTIFF ModelPixelScaleTag tag.  */

  if ( TIFFGetField( tif, TIFFTAG_GEOPIXELSCALE, &count, &pixelscale ) ) {
    if ( pixelscale[0] != pixelscale[1] ) {
      tsmError( funcName, "Non-square PixelScale (%lf x %lf), assuming %lf",
		pixelscale[0], pixelscale[1], pixelscale[0] );
    }

    tspec->start_res = pixelscale[0];
    TSMTRACE(( TSMMASK|1, funcName, "GeoTIFF PixelScale = %lf",pixelscale[0]));

  } else
    tsmError( funcName, "GeoTIFF file does not specify ModelPixelScale" );

  /* Find the lower-left coordinate for the image. This will be stored  */
  /* in the ModelTiepointTag. Normally the (0,0) image point is defined */
  /* N.B. (0,0) in TIFF is the top-left, so we need to calculate what   */
  /* the bottom left would be from this and the other data we know.     */
  /* N.B. we should support (0.5,0.5) too, as output by ERDAS Imagine.  */

  if ( TIFFGetField( tif, TIFFTAG_GEOTIEPOINTS, &count, &tiepoints ) ) {
    
    if ( fabs(tiepoints[0]) > 1.0 || fabs(tiepoints[1]) > 1.0 )
      tsmError( funcName, "First GeoTIFF tiepoint is not (0,0) - ignoring" );
    else {
    
      switch ( tspec->coord_sys ) {
      case TSM_UTM_COORDSYS:
	/* negative northing values used for southern hemi */
	if ( south_hemi && tiepoints[4] >= 0.0 ) 
	  tiepoints[4] = -tiepoints[4];

	tspec->ll_n = tiepoints[4] - ( pixelscale[1] * tspec->height );
	tspec->ll_e = tiepoints[3];
	break;
      case TSM_LATLONG_COORDSYS:
	tspec->ll_lat = tiepoints[4] - ( pixelscale[1] * tspec->height );
	tspec->ll_lon = tiepoints[3];
	break;
      }
      
    }

  } else
    tsmError( funcName, "GeoTIFF file does not specify ModelTiepointTag" );

  /* output tracing information for the GeoTIFF values we extracted */

  switch ( tspec->coord_sys ) {
  case TSM_UTM_COORDSYS:
    TSMTRACE(( TSMMASK|1, funcName, "UTM: %1.1lfe %1.1lfn zone %d, res %g",
	       tspec->ll_e, tspec->ll_n, tspec->zone, tspec->start_res ));
    break;
  case TSM_LATLONG_COORDSYS:
    TSMTRACE(( TSMMASK|1, funcName, "Lat/Long: %1.1lf lat %1.1lf long, res %g",
	       tspec->ll_lat, tspec->ll_lon, tspec->start_res ));
    break;
  }
  TSMTRACE(( TSMMASK|1, funcName, "Description: %s", tspec->description ));

  /* get rid of the key parser and close the TIFF file */

  GTIFFree( gtif );
  XTIFFClose( tif );

  TSMTRINC(( TSMMASK|1, -1 ));
  return TRUE;
}
