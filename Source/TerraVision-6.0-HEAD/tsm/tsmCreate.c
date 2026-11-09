/*
 * Name: tsmCreate.c
 *
 * Description: 
 *    This file contains the user-level functions for generating tiled
 *    terrain data from any of the currently supported input images
 *    formats. This includes the generation of tspec files as well as
 *    the terrain data itself
 *
 * Function List:
 *    void tsmInitTileParams( TsmTileParams *params )
 *    void tsmInitTspecParams( TsmTspecParams *params )
 *    int tsmParseCreateParams( char *image_name, TsmTileParams *tile,
 *                              TsmTspecParams *tspec )
 *    int tsmCreateTspec( TsmConnection connect, TsmTspecParams params )
 *    int tsmCreateLevelTiles( TsmConnection connect, const char *image_name,
 *                             TsmTileParams params, int level )
 *    int tsmCreateAllTiles( TsmConnection connect, const char *image_name, 
 *                           TsmTileParams params )
 *    int tsmDownSample( tsmConnection connect, int in_level,
 *                       const char *in_name, const char *out_name,
 *                       TsmTileParams params )
 *
 * Revision History:
 * 
 *   $Id: tsmCreate.c,v 1.34.2.1 2001/06/19 22:04:41 leei Exp $
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
 * How To Write Your Own File Reader:
 *   1. Create your file format reader files and add these to the autoconf
 *      build system (i.e. add to Makefile.am, run "automake --foreign" and
 *      then "autoconf")
 *   
 *         tsmReadRowsFOO.c - your file format reader functions
 *         tsmReadRowsFOO.h - definitions and prototypes for your reader
 *   
 *      You should create four functions:
 *   
 *         tsmReadRowsFOO_new() - open the file and create a state closure
 *         tsmReadRowsFOO_read() - read a number of lines from the file
 *         tsmReadRowsFOO_destroy() - close file and free up the closure
 *         tsmParseCreateParams_FOO() - parse .foo file for metadata info
 *   
 *   2. In tsmCreate.h:
 *   
 *    - add a define for TSM_IMPORT_FOO to the list
 *   
 *   3. In tsmCreate.c:
 *   
 *    - add a #include "tsmReadRowsTIFF.h" to the top of the file
 *
 *    - add support for the filename extension .foo to tsmFindImportFormat()
 *      so that it returns TSM_IMPORT_FOO
 *   
 *    - add code to the switch statement in tsmCreateAllTiles() in order
 *      to detect TSM_IMPORT_FOO and call the appropriate downsample function
 *      with a callback to your reader function in tsmReadRowsFOO.c
 *   
 *    - add code to the switch statement in tsmParseCreateParams() in order
 *      to detect TSM_IMPORT_FOO and parse any header/metadata contained in
 *      the file.
 *   
 *    - add code to the switch statement in tsmDescribeImportFormat() in
 *      order to give a textual description of the file format FOO.
 *
 */

#include "tsmInternal.h"
#include "tsmConnectionInt.h"
#include "tsmConnectionParamsInt.h"

#include "tsmReadRowsJPEG.h"
#include "tsmReadRowsPBM.h"
#include "tsmReadRowsGIF.h"
#include "tsmReadRowsRAW.h"
#include "tsmReadRowsDEM.h"
#include "tsmReadRowsDTED.h"
#include "tsmReadRowsDOQ.h"
#include "tsmReadRowsTIFF.h"
#include "tsmWriteRows.h"

/*
 * Synopsis:
 *   void tsmInitTileParams( TsmTileParams *params )
 *
 * Doc++ Description: *//** 
 *   Initialises a TsmTileParams structure.
 *
 *   The TsmTileParams structure provides an extensible and flexible
 *   means to specify details about a dataset that you wish to create
 *   from raw data. This function will initialise all of the fields
 *   to sensible default values, dependent upon the type of the
 *   dataset (OI, DEM, etc.).
 *
 *   You pass this structure to the tsmCreateLevelTiles or
 *   tsmCreateAllTiles routines in order to tell them how to generate
 *   the tile pyramid for the dataset.
 *
 * @param    params the TsmTileParams structure to be initialised
 * @param    type   the dataset type, e.g. TSM_DEM_DATASET, TSM_IMAGE_DATASET,
 *                  etc.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  22 March 1998
 *
 * @see      tsmCreateLevelTiles, tsmCreateAllTiles, tsmInitTspecParams,
 *           TsmParseCreateParams, TsmTileParams
 *
 */

void
tsmInitTileParams( TsmTileParams *params, TsmDataSetType type )
{
  if ( params == NULL ) return;

  params->width            = 0;
  params->height           = 0;
  params->format           = TSM_IMPORT_UNSUPPORTED;
  params->dataset_type     = type;
  params->start_col        = 0;
  params->start_row        = 0;
  params->block_size       = -1;
  params->remove_temp_imgs = TRUE;
  params->sea_level        = 0.0;
  params->scale            = 1.0;
  params->offset           = 0.0;

  if ( type == TSM_DEM_DATASET ) {
    params->comps          = 1;
    params->bytespercomp   = 4;
    params->out_bytespercomp = 4;
    params->filtering      = TSM_POINT_SAMPLE;
    params->tile_width     = 128;
    params->tile_height    = 128;
    params->tile_overlap_x = 1;
    params->tile_overlap_y = 1;
  } else {
    params->comps          = 3;
    params->bytespercomp   = 1;
    params->out_bytespercomp = 1;
    params->filtering      = TSM_GAUSSIAN_FILTER;
    params->tile_width     = 128;
    params->tile_height    = 128;
    params->tile_overlap_x = 0;
    params->tile_overlap_y = 0;
  }
}

/*
 * Synopsis:
 *   void tsmInitTspecParams( TsmTspecParams *params )
 *
 * Doc++ Description: *//** 
 *   Initialises a TsmTspecParams structure.
 *
 *   The TsmTspecParams structure provides an extensible and flexible
 *   means to specify details about a tspec (Tile Set Specification)
 *   file that you wish to create from scratch. This function will
 *   initialise all of the fields to sensible default values,
 *   dependent upon the type of the dataset (OI, DEM, etc.).
 *
 *   You pass this structure to the tsmCreateTspec routine in order
 *   to tell it how to create the tspec file.
 *
 * @param    params the TsmTspecParams structure to be initialised
 * @param    type   the dataset type, e.g. TSM_DEM_DATASET, TSM_IMAGE_DATASET,
 *                  etc.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  22 March 1998
 *
 * @see      tsmCreateTspec, tsmInitTileParams,
 *           tsmParseCreateParams, TsmTspecParams
 * */

void
tsmInitTspecParams( TsmTspecParams *params, TsmDataSetType type )
{
  if ( params == NULL ) return;

  params->width        = 0;
  params->height       = 0;
  params->description  = "";
  params->sources      = "";
  params->ops          = "";
  params->doneby       = "the tsmApi library";
  params->source_type  = "unknown";
  params->coord_sys    = TSM_NONE_COORDSYS;
  params->ll_e         = 0.0;
  params->ll_n         = 0.0;
  params->zone         = 0;
  params->ll_lat       = 0.0;
  params->ll_lon       = 0.0;
  params->lvcs_x       = 0.0;
  params->lvcs_y       = 0.0;
  params->scale        = 1.0;
  params->offset       = 0.0;
  params->dataset_type = type;
  params->planet       = TSM_EARTH;
  params->geoName      = NULL;
  params->start_lev    = 0;

  if ( type == TSM_DEM_DATASET ) {
    params->comps        = 1;
    params->start_res    = 32.0;
  } else {
    params->comps        = 3;
    params->start_res    = 1.0;
  }
}
/*
 * Synopsis:
 *   TsmImportFormat tsmFindImportFormat( char *filename )
 *
 * Doc++ Description: *//** 
 *   Works out a import format from the filename.
 *
 *   This function will try to work out the format of the specified import
 *   data file name. This is done based purely upon the extension of
 *   the filename. If no filename extension is provided, or if the
 *   extension is not recognised, then a result of
 *   TSM_IMPORT_UNSUPPORTED will be returned. The following
 *   extensions are recognised:
 *
 *   \begin{itemize}
 *   \item .img - LAS format DEM/OI data (returns TSM_IMPORT_LAS)
 *   \item .raw - raw (lrtb) DEM/OI data (returns TSM_IMPORT_RAW)
 *   \item .lrbt - raw (lrbt) DEM/OI data (returns TSM_IMPORT_LRBT)
 *   \item .ppm - PPM format (colour) image data (returns TSM_IMPORT_PPM)
 *   \item .pgm - PGM format (grayscale) image data (returns TSM_IMPORT_PGM)
 *   \end{itemize}
 *
 *   Filename extensions are matched in a case insensitive manner,
 *   i.e. foo.raw, foo.RAW, and foo.Raw all mean the same image format.
 *
 * @param    The filename for the input image to be analysed
 *
 * @return   The format of the file, returned as a TsmImportFormat enum.
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmParseCreateParams, tsmCreateAllTiles, TsmImportFormat
 *
 */

TsmImportFormat
tsmFindImportFormat( char const *filename )
{
  char *dot;

  /* Cannot know the image format if there is no filename specified */
  /* or if the filename does not have any extension.                */

  if ( filename == NULL || ( dot = strrchr( filename, '.' ) ) == NULL ) 
    return TSM_IMPORT_UNSUPPORTED;

  /* Otherwise check the filename extension against the valid, */
  /* currently supported format extensions (case insensitive)  */

  if ( strcasecmp( dot, ".raw" ) == 0 ) 
    return TSM_IMPORT_RAW;
  else if ( strcasecmp( dot, ".lrbt" ) == 0 ) 
    return TSM_IMPORT_LRBT;
  else if ( strcasecmp( dot, ".img" ) == 0 ) 
    return TSM_IMPORT_LAS;
  else if ( strcasecmp( dot, ".ppm" ) == 0 ) 
    return TSM_IMPORT_PPM;
  else if ( strcasecmp( dot, ".pgm" ) == 0 ) 
    return TSM_IMPORT_PGM;
  else if ( strcasecmp( dot, ".tiff" ) == 0 || strcasecmp( dot, ".tif" ) == 0)
    return TSM_IMPORT_TIFF;
  else if ( strcasecmp( dot, ".jpg" ) == 0 || strcasecmp( dot, ".jpeg" ) == 0)
    return TSM_IMPORT_JPEG;
  else if ( strcasecmp( dot, ".coq" ) == 0 )
    return TSM_IMPORT_DOQ;
  else if ( strcasecmp( dot, ".gif" ) == 0)
    return TSM_IMPORT_GIF;
  else if ( strcasecmp( dot, ".dem" ) == 0)
    return TSM_IMPORT_DEM;
  else if ( strcasecmp( dot, ".dt0" ) == 0)
    return TSM_IMPORT_DTED;

  return TSM_IMPORT_UNSUPPORTED;
}


/*
 * Synopsis:
 *   int tsmCreateTspec( TsmConnection connect, TsmTspecParams params, int )
 *
 * Doc++ Description: *//** 
 *   Create a new tspec file from scratch.
 *
 *   This function will create a new tspec file for a dataset and write
 *   this out to disk. All of the information that is required to
 *   create the tspec structure is provided in the TsmTspecParams
 *   structure. This structured should first be initialised with the 
 *   tsmInitTspecParams call, and then its fields should be filled out
 *   to describe the new dataset, e.g. the pixel width and height of
 *   the source image, the georeferencing information, etc.
 *
 *   If the parameter write_tspec is set to TRUE, then this function will
 *   create the tspec structure for the connection and then write this
 *   out to the dataset. If this parameter is FALSE, then the tspec file
 *   is not written out but it is inserted into the connection handle so
 *   that it can be retrieved with tsmGet, and manually written to disk
 *   using the tsmWriteTspec function.
 *
 *   This function will create an OI dataset tspec if you set the
 *   params->dataset_type field to TSM_IMAGE_DATASET, and it will create
 *   a DEM dataset tspec if you set this field to TSM_DEM_DATASET. If 
 *   you don't set this field at all, then the value will be taken from
 *   the connection handle (if it has been defined).
 *
 *   Please refer to the documentation page on TsmTspecParams to discover
 *   all of the fields that you can set for this function.
 *
 *   Currently we only support the UTM coordinate system for georeferencing
 *   information. This is provided as the easting/northing/zone of the
 *   lower-left corner of the dataset.
 *
 * @param    connect the connection handle for the dataset to create the
 *                   new tspec file for
 * @param    params  a TsmTspecParams structure that has been properly
 *                   initialised and instantiated.
 *
 * @return   TRUE if all was successful, FALSE if an error occurred.
 *
 * @author   Martin Reddy
 * @version  22 March 1998
 *
 * @see      tsmInitTspecParams, TsmParseCreateParams, TsmTspecParams
 *           tsmGet, tsmWriteTspec
 *
 */

int
tsmCreateTspec( TsmConnection connect, TsmTspecParams params,
		TsmTileParams tile_params, int write_tspec )
{
  static const char *funcName = "tsmCreateTspec";
  Pyramid           *tspec;
  tspSymbol         pyramid_type;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_WRITE_ONLY, FALSE ) )
    return FALSE;

  /* If the param structure doesn't define a dataset type  */
  /* then we will use the value from the connection handle */

  if ( params.dataset_type == TSM_UNKNOWN_DATASET ) {
    if ( connect->dataset_type == TSM_UNKNOWN_DATASET ) {
      tsmError( funcName, "No dataset type info in params or connection" );
      return FALSE;
    }
    params.dataset_type = connect->dataset_type;
  }

  /* Check that we can support the georeferencing information */

  if ( params.coord_sys != TSM_UTM_COORDSYS &&
       params.coord_sys != TSM_LATLONG_COORDSYS &&
       params.coord_sys != TSM_LVCS_COORDSYS ) {
    tsmError( funcName, "Unsupported coordinate system: UTM & Lat/Long only" );
    return FALSE;
  }

  /* Work out a tspSymbol version of the dataset type for old style func */

  switch ( params.dataset_type ) {
  case TSM_DEM_DATASET:
    pyramid_type = tspDemType;
    break;
  case TSM_IMAGE_DATASET:
    pyramid_type = tspOiType;
    break;
  default:
    tsmError( funcName, "Unsupported dataset type - cannot create tspec" );
    return FALSE;
  }

  /* Create the Pyramid structure for this dataset */

  tspec = tspCreatePyramid( connect->name, pyramid_type, params, tile_params );

  if ( tspec == NULL ) {
    tsmError( funcName, "Couldn't create the new tspec structure" );
    return FALSE;
  }

  /* Now put this new pyramid structure in the connection handle */
  /* and then write this new tspec out to disk (if asked to).    */

  tsmSetTspecField( connect, tspec );

  return ( write_tspec ) ? tsmWriteTspec( connect ) : TRUE;
}

/*
 * Synopsis:
 *    int tsmCreateAllTiles( TsmConnection connect, const char *image_name, 
 *                           TsmTileParams params )
 *
 * Doc++ Description: *//** 
 *    Creates an entire dataset of tiles.
 *
 *    This function will take an input image for the highest level of a
 *    dataset. It will then proceed to segment this image into tiles 
 *    (padding the edge data to tile boundaries where necessary), then
 *    downsample this image to create data for the next level of the
 *    pyramid. This process is repeated until the image fits inside
 *    a single tile.
 *
 *    This function calls the tsmCreateLevelTiles to create the tiles
 *    for an entire level, and tsmDownSample to produce a half-resolution
 *    image.
 *
 *    All of the details about the image and how to create the tileset
 *    should be provided in the params data structure - refer to the
 *    documentation on TsmTileParams for a list of all the fields that
 *    you can set (N.B. you should always initialise a TsmTileParams
 *    structure with tsmInitTileParams). 
 *
 *    This function will create an OI dataset tspec if you set the
 *    params->dataset_type field to TSM_IMAGE_DATASET, and it will create
 *    a DEM dataset tspec if you set this field to TSM_DEM_DATASET. If 
 *    you don't set this field at all, then the value will be taken from
 *    the connection handle (if it has been defined).
 *
 *    You can specify the file format of the input file by setting the
 *    format field of the params structure. Currently we only support
 *    the LAS format for both DEMs and OIs. tsmInitTileParams will 
 *    set this field to TSM_LAS by default.
 *
 *    If you set the TSM_PROGRESS_METER setting for the connection to 
 *    TRUE (i.e. using tsmSet), then progress reports will be displayed
 *    to the terminal during the process. Setting this value to FALSE
 *    will make this operation silent (unless an error occurs).
 *
 * @param    connect a valid connection handle, as returned by tsmConnect
 * @param    image_name the (localdisk) name of the input image.
 * @param    params a TsmTileParams structure defining all about the tileset
 *
 * @return   TRUE if the operation was successful, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  23 March 1998
 *
 * @see      tsmCreateLevelTiles, tsmDownSample, tsmConnect, TsmTileParams
 *
 */

int
tsmCreateAllTiles(TsmConnection connect, const char *filename,
		  TsmTileParams params)
{
  static const char *funcName = "tsmCreateAllTiles";
  TsmWriteRows *writer;
  Pyramid *pyramid;
  int type;
  int rv;
  int flags = 0;

  if ((pyramid = (Pyramid *) tsmGet(connect, TSM_DATASET_TSPEC)) == NULL) {
    tsmError(funcName, "Couldn't get tspec structure from dataset.");
    return FALSE;
  }

  /* IMPORTANT */
  /* The tileTspec.numComponentsPixel and tileTspec.numBytesPerComponent need to
     be set correctly before initializing TsmWriteRows instance. */
  /* pyramid->tileTspec.numComponentsPixel = params.comps; */
  /* pyramid->tileTspec.numBytesPerComponent = params.out_bytespercomp; */

  if ((writer = tsmWriteRows_new(connect, &params)) == NULL) {
    tsmError(funcName, "Couldn't create TsmWriteRows instance.");
    return FALSE;
  }

  type = tsmFindImportFormat(filename);

  if (params.dataset_type == TSM_DEM_DATASET)
    flags |= TSM_DOWNSAMPLE_DEM;

  switch ( type ) {
  case TSM_IMPORT_PPM:
  case TSM_IMPORT_PGM: {
    TsmReadRowsPBM *reader;
    if ((reader = tsmReadRowsPBM_new(filename)) == NULL) return FALSE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags, tsmReadRowsPBM_read,
				reader, tsmWriteRows_write, writer);
    tsmReadRowsPBM_destroy(&reader);
  }
  break;

  case TSM_IMPORT_GIF: {
    TsmReadRowsGIF *reader;
    if ((reader = tsmReadRowsGIF_new(filename)) == NULL) return FALSE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags, tsmReadRowsGIF_read,
				reader, tsmWriteRows_write, writer);
    tsmReadRowsGIF_destroy(&reader);
  }
  break;

  case TSM_IMPORT_TIFF: {
    TsmReadRowsTIFF *reader;
    if ((reader = tsmReadRowsTIFF_new(filename)) == NULL) return FALSE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags, tsmReadRowsTIFF_read,
				reader, tsmWriteRows_write, writer);
    tsmReadRowsTIFF_destroy(&reader);
  }
  break;

  case TSM_IMPORT_JPEG: {
    TsmReadRowsJPEG *reader;
    if ((reader = tsmReadRowsJPEG_new(filename)) == NULL) return FALSE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags, tsmReadRowsJPEG_read,
				reader, tsmWriteRows_write, writer);
    tsmReadRowsJPEG_destroy(&reader);
  }
  break;

  case TSM_IMPORT_DOQ: {
    TsmReadRowsDOQ *reader;
    if ((reader = tsmReadRowsDOQ_new(filename)) == NULL) return FALSE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags, tsmReadRowsDOQ_read,
				reader, tsmWriteRows_write, writer);
    tsmReadRowsDOQ_destroy(&reader);
  }
  break;

  case TSM_IMPORT_RAW:
  case TSM_IMPORT_LRBT: {
    TsmReadRowsRAW *reader;
    SimpleTileSet *tileset = &pyramid->tspecLevel.val[pyramid->minLevel];
    writer->lrbt = (type==TSM_IMPORT_LRBT);
    if ((reader = tsmReadRowsRAW_new(filename, tileset->numValidPixel[0],
				     tileset->numValidPixel[1], params.comps,
				     params.bytespercomp)) == NULL) {
      return FALSE;
    }
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags,
				tsmReadRowsRAW_read, reader,
				tsmWriteRows_write, writer);
    tsmReadRowsRAW_destroy(&reader);
  }
  break;

  case TSM_IMPORT_DEM: {
    TsmReadRowsDEM *reader;
    if ((reader = tsmReadRowsDEM_new(filename)) == NULL) return FALSE;
    writer->lrbt = TRUE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags,
				tsmReadRowsDEM_read, reader,
				tsmWriteRows_write, writer);
    tsmReadRowsDEM_destroy(&reader);
  }
  break;

  case TSM_IMPORT_DTED: {
    TsmReadRowsDTED *reader;
    if ((reader = tsmReadRowsDTED_new(filename)) == NULL) return FALSE;
    writer->lrbt = TRUE;
    rv = tsmDownSample_inmemory(params.width, params.height,
				pyramid->maxLevel-pyramid->minLevel+1,
				params.tile_width, params.bytespercomp,
				params.comps, flags,
				tsmReadRowsDTED_read, reader,
				tsmWriteRows_write, writer);
    tsmReadRowsDTED_destroy(&reader);
  }
  break;

  default:
    tsmError( funcName, "No reader callback specified for file format!" );
    tsmWriteRows_destroy(&writer);
    return FALSE;
  }

  tsmWriteRows_destroy(&writer);

  return rv;
}

/*
 * Synopsis:
 *   int tsmParseCreateParams( char *image_name, TsmImportFormat format,
 *                             TsmTileParams *tile, TsmTspecParams *tspec )
 *
 * Doc++ Description: *//** 
 *   Try to extract info about an input image.
 *
 *   This function can be used to try and extract as much information
 *   about an image that is available in the image file itself, or in
 *   any standard associated files. The information that is extracted
 *   is written directly into the provided TsmTileParams and/or
 *   TsmTspecParams structures (which can then be passed to the
 *   tsmCreateTspec or tsmCreate*Tiles functions). This can be used to
 *   make life easier for the client by reducing the amount of work
 *   they need to do.
 *
 *   The actual information that can be extract is obviously dependent
 *   on how much information is provided by a particular file format:
 *
 *   \begin{itemize}
 *   \item {\bf LAS} : LAS files are raw data and contain no descriptions of
 *         these data. Such descriptions are contained in the DDR file.
 *         The DDR file is a binary file, so the folks at EDC (USGS
 *         Eros Data Center) produce an ASCII version of this file with
 *         a ".prt" extension. tsmParseCreateParams will therefore try
 *         to find a .prt file associated with the specified image. If
 *         found, it will try to parse this and set all relevant fields.
 *         Currently, this includes the width and height of the image,
 *         the number of components per pixel (bands), and
 *         the UTM lower-left coordinate and zone number. The function will
 *         fail if the .prt file defines the projection type as anything
 *         other than UTM, or of the .prt file does not exist.
 *   \item {\bf PGM / PPM} : The image formats contain a short header
 *         at the start of the file that define the pixel width and 
 *         height for the image. This information is extracted, and also
 *         the number of components per pixel is set to 1 for PGM files
 *         (as these are grayscale).
 *   \item {\bf TIFF / GEOTIFF} : TIFF images contain all of the information
 *         that we need to know about the image raster, e.g. width, height,
 *         number of components, etc. In addition, if a GeoTIFF file is
 *         used, then we can extract the georeference information.
 *   \end{itemize}
 *
 *   N.B. you should call tsmInitTileParams and tsmInitTspecParams
 *   for the tile and tspec structures, respectively, before calling
 *   this function.
 *
 * @param    image_name the name of the source image that we want to try
 *                      and find out about.
 * @param    format the format of the image file, currently only TSM_LAS
 * @param    tile the TsmTileParams to fill with any relevant values that
 *                are found for the image file.
 * @param    tspec the TsmTspecParams to fill with any relevant values that
 *                are found for the image file.
 *
 * @return   TRUE if the operation was successful, FALSE if an error occurred.
 *
 * @author   Martin Reddy
 * @version  23 March 1998
 *
 * @see      tsmInitTileParams, tsmInitTspecParams, tsmCreateTspec,
 *           tsmCreateLevelsTiles, tsmCreateAllTiles, TsmTileParams,
 *           TsmTspecParams
 *
 */

int
tsmParseCreateParams( char *image_name, TsmTileParams *tile,
		      TsmTspecParams *tspec )
{
  static const char *funcName = "tsmParseCreateParams";

  /* Check that we have valid parameters */

  if ( image_name == NULL || tile == NULL || tspec == NULL ) {
    tsmError( funcName, "All params must be non NULL" );
    return FALSE;
  }

  /* Work out the import format, if not already specfied */
  if ( tile->format == TSM_IMPORT_UNSUPPORTED ) {
    tile->format = tsmFindImportFormat( (char const *) image_name );
    
    if ( tile->format == TSM_IMPORT_UNSUPPORTED ) {
      tsmError( funcName, "Image filename extension is not recognised" );
      return FALSE;
    }
  }

  /* Now try to read the various creation params from any auxiliary */
  /* data file that is associated with the specific image format.   */

  switch ( tile->format ) {
  case TSM_IMPORT_LAS:
    return /* tsmParseCreateParams_LAS( image_name, tile, tspec ) */ FALSE;
  case TSM_IMPORT_RAW:
  case TSM_IMPORT_LRBT:
    return TRUE;
  case TSM_IMPORT_DEM:
    return tsmParseCreateParams_DEM( image_name, tile, tspec );
  case TSM_IMPORT_DTED:
    return tsmParseCreateParams_DTED( image_name, tile, tspec );
  case TSM_IMPORT_PPM:
    return tsmParseCreateParams_PPM( image_name, tile, tspec );
  case TSM_IMPORT_PGM:
    return tsmParseCreateParams_PGM( image_name, tile, tspec );
  case TSM_IMPORT_TIFF:
    return tsmParseCreateParams_TIFF( image_name, tile, tspec );
  case TSM_IMPORT_GIF:
    return tsmParseCreateParams_GIF( image_name, tile, tspec );
  case TSM_IMPORT_JPEG:
    return tsmParseCreateParams_JPEG( image_name, tile, tspec );
  case TSM_IMPORT_DOQ:
    return tsmParseCreateParams_DOQ( image_name, tile, tspec );
  default:
    tsmError( funcName, "Unsupported image format!" );
    return FALSE;
  }

}

/*
 * Synopsis:
 *   void tsmDescribeImportFormat( TsmTileParams tile )
 *
 * Doc++ Description: *//** 
 *   Describe the import format to stdout.
 *
 *   Outputs a single line of text to state what file format is
 *   currently set for the specified TsmTileParams structure.
 *   This value is set to "unknown" (TSM_IMPORT_UNSUPPORTED) by
 *   default, but is updated for the actual file format once
 *   you call tsmParseCreateParams.
 *
 * @param    tile a TsmTileParams structure 
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  4 April 1998
 *
 * @see      tsmParseCreateParams, tsmFindImportFormat, TsmTileParams
 *
 */

void
tsmDescribeImportFormat( TsmTileParams tile )
{
  printf( "Input file format = " );
  switch ( tile.format ) {
  case TSM_IMPORT_LAS:
    puts( "LAS (Land Analysis System)" );
    break;
  case TSM_IMPORT_RAW:
    puts( "RAW (left->right, top->bottom big-endian bitmap)" );
    break;
  case TSM_IMPORT_LRBT:
    puts( "LRBT (left->right, bottom->top big-endian bitmap)" );
    break;
  case TSM_IMPORT_DEM:
    puts( "DEM (USGS Digital Elevation Model format)");
    break;
  case TSM_IMPORT_DTED:
    puts("DTED (Digital Terrain Elevation Model (level 0) format)");
    break;
  case TSM_IMPORT_PPM:
    puts( "PPM (colour image)" );
    break;
  case TSM_IMPORT_PGM:
    puts( "PGM (grayscale image)" );
    break;
  case TSM_IMPORT_TIFF:
    puts( "TIFF / GeoTIFF" );
    break;
  case TSM_IMPORT_GIF:
    puts( "GIF (indexed color image)" );
    break;
  case TSM_IMPORT_JPEG:
    puts( "JPEG (color image)" );
    break;
  case TSM_IMPORT_DOQ:
    puts( "DOQ (USGS Digital Orthophoto Quadrangle grayscale imagery)" );
    break;
  default:
    puts( "unknown" );
    break;
  }
}
