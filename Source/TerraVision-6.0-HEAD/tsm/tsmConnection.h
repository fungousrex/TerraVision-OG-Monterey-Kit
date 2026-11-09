/*
 * Name: tsmConnection
 *
 * Description: 
 *   External definitions for managing connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnection.h,v 1.15.6.1 2001/06/20 22:26:13 reddy Exp $
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

#ifndef TSMCONNECTION_H
#define TSMCONNECTION_H

/* some default event strings for logging data with tsmLog */

#define TV_TILE_REQ           "TV_TILE_REQ"
#define TV_TILE_READ          "TV_TILE_READ"
#define TV_TILE_WRITE         "TV_TILE_WRITE"
#define TV_TILE_IN_CACHE      "TV_TILE_IN_CACHE"
#define TV_TILE_OUT_CACHE     "TV_TILE_OUT_CACHE"
#define TV_TILE_IN_TEXCACHE   "TV_TILE_IN_TEXCACHE"
#define TV_TILE_OUT_TEXCACHE  "TV_TILE_OUT_TEXCACHE"
#define TV_REQUEST_SENT       "TV_REQUEST_SENT"
#define TV_TILES_AVAIL_RATIO  "TV_TILES_AVAIL_RATIO"

/* A short cut to log information about a single tile event */

#define tsmLogTile(connect,event,tx,ty,level) \
    tsmLog( connect, event, "DPSS.BID=%d.%d.%d.0", tx, ty, level )

/*
 * Doc++ Description: *//** 
 *   Enumerated values for all connection settings.
 *
 *   This enumerated type contains values for all of the connection
 *   settings. You use these values with the tsmGet and tsmSet functions
 *   in order to specify which settings you wish to update or query.
 *
 *   Refer to the pages on these two functions for a full listing of the
 *   currently available enumerated types.
 *
 * @see tsmGet, tsmSet.
 *
 */

enum TsmSetting {
  /* Read-only settings for a connection */
  TSM_CONNECT_PROTOCOL,          /* TSM_DPSS, TSM_LOCAL, or TSM_HTTP */
  TSM_DATASET_NAME,              /* The name of the dataset */
  TSM_DATASET_URL,               /* The full URL for the dataset */
  TSM_DATASET_TSPEC,             /* The pyramid's tspec structure */
  TSM_IMAGE_FORMAT,              /* The image format for the dataset */
  TSM_IMAGE_FORMAT_STRING,       /* A string description of TSM_IMAGE_FORMAT */
  TSM_TILE_FORMAT,               /* The tile file format to read/write */
  TSM_TILE_FORMAT_STRING,        /* A string description of TSM_TILE_FORMAT */
  TSM_DATASET_TYPE,              /* TSM_DEM_DATASET, TSM_IMAGE_DATASET, etc. */
  TSM_DATASET_TYPE_STRING,       /* String description of TSM_DATASET_TYPE */
  TSM_READWRITE_MODE,            /* The Read/Write mode for the connection */
  TSM_READWRITE_MODE_STRING,     /* String description of TSM_READWRITE_MODE */
  TSM_TILE_SIZE,                 /* The size of a tile (bytes) */
  TSM_DPSS_SETID,                /* The set ID of a dataset on a DPSS */
  TSM_PROGRESS_METER,            /* Show a progress meter for long ops */
  TSM_TSPEC_URL,                 /* The URL the tspec file was loaded from */
  TSM_SEA_ELEVATION,             /* The DEM value representing areas of sea */
  /* Read and Write settings for a connection */
  TSM_DONT_READ_TILES,           /* To stop reading of tile data */
  TSM_JPEG_QUALITY,              /* The JPEG compression quality value */
  TSM_NO_DECOMPRESS,             /* Do we want raw JPEG data returned? */
  TSM_METADATA_URL,              /* Used to set a tspec URL for a DPSS set */
  TSM_FINISH_LOAD_ON_DISCONNECT, /* For writing a new set to DPSS */
  TSM_DPSS_SERVERS,              /* A string with all DPSS server names */
  TSM_REQUEST_TIMEOUT            /* timeout between requesting DPSS blocks */
};

typedef enum TsmSetting TsmSetting;

/* An enumerated type used to decided the type of a TsmConnection* struct */
/*
 * Doc++ Description: *//** 
 *   Enumerated values for a connection protocol.
 *
 *   This enumerated type contains values for all of the connection
 *   protocol types. This is, whether the dataset is being accessed from
 *   localdisk, over http, or from a DPSS.
 *
 *   @param TSM_DPSS  Connection is for a dataset on a DPSS
 *   @param TSM_LOCAL Connection is for a dataset on localdisk
 *   @param TSM_HTTP  Connection is for a dataset on an http server 
 *
 *   @see tsmGet
 *
 */

enum TsmConnectionType {
  TSM_DPSS,        /* Connection is for a dataset on a DPSS */
  TSM_LOCAL,       /* Connection is for a dataset on localdisk */
  TSM_HTTP,        /* Connection is for a dataset on an http server */ 
  TSM_WMS          /* Connection is for an OGC Web Mapping Server (WMS) */
};

typedef enum TsmConnectionType TsmConnectionType;

/*
 * Doc++ Description: *//** 
 *   Enumerated type for access mode.
 *
 *   This enumerated type is used to specify the access mode for the dataset.
 *   You can use the tsmGet function to query a connection on its access
 *   mode. The currently supported values are as follows:
 *   
 * @param TSM_READ_ONLY  Connection is read only
 * @param TSM_WRITE_ONLY Connection is write only
 * @param TSM_READ_WRITE Connection is read and write
 *
 * @see tsmGet
 *
 */
 
enum TsmAccessMode {
  TSM_READ_ONLY,   /* Connection is read only */
  TSM_WRITE_ONLY,  /* Connection is write only */
  TSM_READ_WRITE   /* Connection is read and write */
};

typedef enum TsmAccessMode TsmAccessMode;

/*
 * Doc++ Description: *//** 
 *   Defines all state for a connection.
 *
 * This handle is returned by the tsmConnect function and is used to
 * hold all state and information for a single connection. It is used
 * for of the connection-based routines of the tsmApi.
 *
 * This is an opaque struct: the fields of TsmConnection are hidden
 * from the external client so that future changes can be made
 * internally without breaking user code.
 *
 * If you want to query or set connection values, then you can use
 * tsmGet and tsmSet API functions.
 *
 * @see tsmConnect, tsmGet, tsmSet
 *
 */

typedef struct TsmConnectionCommon *TsmConnection;


#endif
