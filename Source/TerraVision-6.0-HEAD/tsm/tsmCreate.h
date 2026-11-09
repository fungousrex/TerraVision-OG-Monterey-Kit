/*
 * Name: tsmCreate.h
 *
 * Description:
 *   Contains all of the data structure definitions to help in the
 *   generation of new terrain datasets. This includes the generation
 *   of tspec files as well as tiled DEM and OI data.
 *
 * Structure List:
 *     TsmImportFormat
 *     TsmTspecParams
 *     TsmTileParams
 *
 * Revision History:
 *
 *     $Id: tsmCreate.h,v 1.19.6.1 2001/08/17 18:54:47 leei Exp $
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

#ifndef TSMCREATE_H
#define TSMCREATE_H

#define TSM_POINT_SAMPLE      0      /* used for down sampling operation */
#define TSM_GAUSSIAN_FILTER   1

/*
 * Doc++ Description: *//** 
 *   Specifies all supported import file formats.
 *
 *   The TsmImportFormat enumerated type is used to define all of the
 *   file formats that the tsmApi can import in order to generate terrain
 *   data. This is used as a parameter to the tile creation functions.
 *
 *   @param TSM_IMPORT_UNSUPPORTED the input file format is not known
 *   @param TSM_IMPORT_LAS EDC's LAS format DEM and OI files
 *   @param TSM_IMPORT_RAW RAW (lrtb: left->right, top->bottom) image
 *                             data for DEM and OI files
 *   @param TSM_IMPORT_LRBT RAW (lrbt: left->right, bottom->top) image
 *                              data for DEM and OI files
 *   @param TSM_IMPORT_PPM PPM format (colour) image
 *   @param TSM_IMPORT_PGM PGM format (grayscale) image
 *   @param TSM_IMPORT_TIFF TIFF or GeoTIFF format image
 *
 *   @see tsmCreateAllTiles
 *
 */

enum TsmImportFormat {
  TSM_IMPORT_UNSUPPORTED,      /* not a supported image format */
  TSM_IMPORT_LAS,              /* EDC's LAS format DEM and OI files */
  TSM_IMPORT_RAW,              /* raw RGB, grayscale, or DEM data */
  TSM_IMPORT_LRBT,             /* raw lrbt RGB, grayscale, or DEM data */
  TSM_IMPORT_PPM,              /* ortho-imagery in PPM format (RGB) */
  TSM_IMPORT_PGM,              /* ortho-imagery in PGM format (grayscale) */
  TSM_IMPORT_TIFF,             /* TIFF or GeoTIFF format images */
  TSM_IMPORT_GIF,              /* GIF image file */
  TSM_IMPORT_JPEG,             /* JPEG image file */
  TSM_IMPORT_DOQ,              /* USGS DOQ imagery */
  TSM_IMPORT_DEM,              /* USGS DEM format */
  TSM_IMPORT_DTED              /* Military DTED Level 0 tiles */
};

typedef enum TsmImportFormat TsmImportFormat;

/*
 * Doc++ Description: *//** 
 *   Holds all options for creating a number of tiles.
 *
 *   The TsmTileParams structure provides an extensible and flexible
 *   means to specify details about a dataset that you wish to create
 *   from raw data.
 *
 *   N.B. you should always initialise a TsmTileParams structure
 *   by calling tsmInitTileParams.
 *
 *   @param width (int) the width of the input image
 *   @param height (int) height of the input image
 *   @param comps (int) no. of components per pixel, e.g. 3
 *   @param format (TsmImportFormat) the file format of input image
 *   @param dataset_type (TsmDataSetType) TSM_DEM_DATASET, TSM_IMAGE_DATASET
 *   @param filtering (int) TSM_GAUSSIAN_FILTER, TSM_POINT_SAMPLE
 *   @param start_col (int) start column of the block create
 *   @param start_row (int) start row of the block create
 *   @param block_size (int) size of block (width & height)
 *   @param tile_width (int) width of a single tile in pixels
 *   @param tile_height (int) height of a single tile in pixels
 *   @param tile_overlap_x (int) number of overlap pixels in x direction
 *   @param tile_overlap_y (int) number of overlap pixels in y direction
 *   @param sea_level (double) height value to represent sea regions.
 *   @param scale (double) scale factor to apply to pixel data
 *   @param offset (double) offset to apply after scaling factor
 *
 *   @see tsmInitTileParams, tsmCreateAllTiles
 *
 */

struct TsmTileParams {
  /* Fields to describe the input image */
  int             width;            /* width of the input image */
  int             height;           /* height of the input image */
  int             comps;            /* no. of components per pixel, e.g. 3 */
  int             bytespercomp;     /* bytes per component for input image */
  int             out_bytespercomp; /* bytes per component for output dataset */
  TsmImportFormat format;           /* file format of input image */
  TsmDataSetType  dataset_type;     /* TSM_DEM_DATASET, TSM_IMAGE_DATASET */
  /* Fields to control the output of the dataset */
  int             filtering;        /* TSM_GAUSSIAN_FILTER, TSM_POINT_SAMPLE */
  int             start_col;        /* start column of the block create */
  int             start_row;        /* start row of the block create */
  int             block_size;       /* size of block (width & height) */
  int             remove_temp_imgs; /* delete intermediate downsampled imgs */
  /* Fields to define the dimensions of each tile */
  int             tile_width;       /* width of a single tile in pixels */
  int             tile_height;      /* height of a single tile in pixels */
  int             tile_overlap_x;   /* number of overlap pixels in x-axis */
  int             tile_overlap_y;   /* number of overlap pixels in y-axis */
  /* pixel transformation information */
  double          scale;            /* scale factor to apply to pixel data */
  double          offset;           /* offset to apply after scaling factor */
  double          sea_level;        /* height representing sea regions */
};

typedef struct TsmTileParams TsmTileParams;

/*
 * Doc++ Description: *//** 
 *   Holds all options for creating a tspec file.
 *
 *   The TsmTspecParams structure provides an extensible and flexible
 *   means to specify details about a tspec (Tile Set Specification)
 *   file that you wish to create from scratch.
 *
 *   N.B. you should always initialise a TsmTspecParams structure
 *   by calling tsmInitTspecParams.
 *
 *   @param width (int) the width of the input image
 *   @param height (int) height of the input image
 *   @param comps (int) no. of components per pixel, e.g. 3
 *   @param start_lev (int) the start level number of dataset
 *   @param start_res (float) the resolution for this start level
 *   @param dataset_type (TsmDataSetType) TSM_DEM_DATASET, TSM_IMAGE_DATASET
 *   @param sources (char *) a list of sources of the image
 *   @param ops (char *) a list of operations performed on img
 *   @param doneby (char *) who performed the creation
 *   @param source_type (char *) the source of the image
 *   @param geoName (char *) the geoset name for this dataset
 *   @param coord_sys (TsmCoordSystem) Currently only TSM_UTM_COORDSYS
 *   @param ll_e (double) UTM lower-left corner for dataset (Eastings)
 *   @param ll_n (double)UTM lower-left corner for dataset (Northings)
 *   @param zone (int) UTM zone number for dataset
 *   @param planet (TsmPlanet) one of TSM_EARTH, TSM_MARS, etc.
 *   @param scale (double) scale factor to apply to pixel data
 *   @param offset (double) offset to apply after scaling factor
 *
 *   @see tsmInitTspecParams, tsmCreateTspec
 *
 */

struct TsmTspecParams {
  /* Fields to describe the highest resolution level image */
  int             width;            /* width of the image */
  int             height;           /* height of the image */
  int             comps;            /* no. of components per pixel, e.g. 3 */
  int             start_lev;        /* the start level number of dataset */
  double          start_res;        /* the resolution for this start level */
  TsmDataSetType  dataset_type;     /* TSM_DEM_DATASET, TSM_IMAGE_DATASET */
  /* descriptive strings */
  char            *description;     /* a description for the dataset */
  char            *sources;         /* a list of sources of the image */
  char            *ops;             /* a list of operations performed on img */
  char            *doneby;          /* who performed the creation */
  char            *source_type;     /* the source of the image */
  char            *geoName;         /* the geoset name for this dataset */
  /* georeferencing information */
  TsmCoordSystem  coord_sys;        /* TSM_UTM_COORDSYS, TSM_LATLONG_COORDSYS*/
  double          ll_e, ll_n;       /* UTM lower-left corner for dataset */
  int             zone;             /* UTM zone number for dataset */
  double          ll_lon, ll_lat;   /* lat/long lower-left corner */
  double          lvcs_x, lvcs_y;   /* LVCS offset from lat/long origin */
  TsmPlanet       planet;           /* TSM_EARTH, TSM_MARS, etc. */
  /* pixel transformation information */
  double          scale;            /* scale factor to apply to pixel data */
  double          offset;           /* offset to apply after scaling factor */
  /* rendering information */
  float           altitude;	    /* altitude above the terrain/ellipsoid */
  int             sky;		    /* is this a sky background? */
  int             two_sided;	    /* is this two-sided?  */
  int             view_from_below;  /* is this a viewable from below?  */
};

typedef struct TsmTspecParams TsmTspecParams;


#endif
