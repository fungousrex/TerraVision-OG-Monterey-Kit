/*
 * Name: tsmTile.h
 *
 * Description: 
 *   Definitions for the tile header structure and other definitions
 *   used by the tile access functions of tsmApi.
 *
 * Function List:
 *
 * Revision History:
 *
 *   "$Id"
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

#ifndef TSMTILE_H
#define TSMTILE_H

/* Used by tsmTileReqAdd() */
#define TSM_NEVER     1
#define TSM_LATER     2
#define TSM_NOW       3

/* returned by tsm*ReadNextTile() */
#define TSM_NODATA    -1

/*
 * Doc++ Description: *//** 
 *   Enumerated values defining local tile formats.
 *
 * This enumerated type defines the various file formats that tile
 * data can be found in (as opposed to the image format of the data
 * that is stored in that file format). The current list of supported
 * file formats are defined below:
 *
 * @param TSM_NONE_TILES no tile files found for the dataset
 * @param TSM_RAW_TILES  tiles are in the raw .tile format (1 .tile per tile)
 * @param TSM_TIFF_TILES tiles are in a blocked .tiff format (1 .tiff per 
 *                        pyramid level)
 * @param TSM_DPSS_TILES tiles are loaded on a DPSS (1 block per tile)
 *
 * @see tsmGet
 *
 */

enum TsmTileFormat {
  TSM_NONE_TILES,
  TSM_RAW_TILES,
  TSM_TIFF_TILES,
  TSM_DPSS_TILES
};

typedef enum TsmTileFormat TsmTileFormat;

/*
 * Doc++ Description: *//** 
 *   Holds information about a single tile's data.
 *
 * The information about a tile is divided up into two structures: one
 * a tile header, containing all of the information about that tile
 * (e.g. tile coordinates, bytes size, resolution, etc.), the other
 * structure contains the raw data for the tile.
 *
 * Here we define the structure for the tile header. This is an opaque
 * structure so that the client cannot see the internal fields. You
 * must use the provided API functions to query the contents of this
 * structure. Refer to the tsmTileHeader*() functions for details.
 *
 * @see tsmAllocTileHeader, tsmFreeTileHeader, tsmTileHeaderX, tsmTileHeaderY,
 *      tsmTileHeaderLevel, tsmTileHeaderSetId, tsmTileHeaderSessionId,
 *      tsmTileHeaderTileId, tsmTileHeaderSize, tsmTileHeaderChecksum,
 *      tsmTileHeaderType, tsmTileHeaderConnection, tsmTileHeaderSizeof
 * 
 */

typedef struct TsmTileHeader TsmTileHeader;

#endif
