/*
 * Name: tsmConnectionInt.h
 *
 * Description: 
 *   Internal definitions for managing connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnectionInt.h,v 1.32.2.4 2001/07/10 06:17:40 leei Exp $
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

#ifndef TSMCONNECTIONINT_H
#define TSMCONNECTIONINT_H

#include "tsmInternal.h"

#include <ghttp/http_uri.h>
#include <ghttp/http_trans.h>

#ifdef DPSS
# include <dpss/dpss.h>
# include <dpss/dsm.h>
#endif

typedef struct TsmConnectionVTable_ TsmConnectionVTable;

/* The client only ever sees pointers to this structure which has only  */  
/* the common fields declared. There are no actual instances of this    */
/* structure - only the specific connection type structures below.      */

struct TsmConnectionCommon {
/* Common fields for all connection types */

  /* determine which type of connection this is */
  TsmConnectionType type;             /* TSM_DPSS, TSM_LOCAL, TSM_HTTP */
  TsmConnectionVTable *vtable;

  /* general descriptions of the dataset */
  char              *name;            /* Name of dataset, copied from tspec */
  Purl              *location;        /* Parsed URL for the dataset */
  Pyramid           *tspec;           /* Tspec structure for the pyramid */
  char              *tspec_url;       /* URL the tspec was loaded from */
  int               format;           /* TSM_LRBT_FORMAT, TSM_JPEG_FORMAT */
  TsmTileFormat     tile_format;      /* TSM_RAW_TILES, TSM_TIFF_TILES,... */
  TsmDataSetType    dataset_type;     /* TSM_DEM_DATASET, TSM_IMAGE_DATASET */

  /* general connection options */
  int               dont_read_tiles;  /* tsmReadTile*() don't read data */
  int               show_progress;    /* Show the copy progress meter? */
  int               blocking_read;    /* Are we doing blocking reads? */
  int               jpeg_quality;     /* JPEG compression quality (0..100) */

  /* tile buffer related variables */
  int               tileBufferSize;   /* size of a tile buffer (bytes) */

  /* request and read lists for tiles */
  PRLock           *logLock;          /* mutex lock for logging stats */
  vsgWorld          *protoWrl;     /* holds VRML vtile prototype context */
  void              *proto;        /* holds vtile prototype definition */

  /* logging state variables */
  int               logging;          /* current DPSS logging stats? */
  NLhandle          *nlhandle;        /* a NetLogger handle for logging */

  /* access mode for this connection */
  TsmAccessMode     mode;             /* TSM_READ_ONLY, TSM_WRITE_ONLY, ... */

  /* managing request lists */
  tsmRequestServer  *srv;	      /* Handler for requests */
  tsmRequestList    *globalReqs;      /* Global request list (compatibility) */
};

TsmConnection tsmConnect_web (Purl *purl,
			      TsmAccessMode mode, TsmConnectionParams param);
TsmConnection tsmConnect_dpss (Purl *purl,
			       TsmAccessMode mode, TsmConnectionParams param);
TsmConnection tsmConnect_local (Purl *purl, 
				TsmAccessMode mode, TsmConnectionParams param);
TsmConnection tsmConnect_wms (Purl *purl, 
			      TsmAccessMode mode, TsmConnectionParams param);

TsmConnection tsmConnectCommon (TsmConnection connect,
				Purl *purl,
				TsmAccessMode mode,
				TsmConnectionParams param);
void tsmDisconnectCommon (TsmConnection connect);

struct TsmConnectionVTable_ {
  void (*disconnect) (TsmConnection conn);

  int (*addRequest) (TsmConnection conn, tsmReqElem *req);
  int (*removeRequest) (TsmConnection conn, tsmReqElem *req);
  int (*reprioritizeRequest) (TsmConnection conn, tsmReqElem *req);

  int (*writeTile) (TsmConnection conn, int tx, int ty, int level,
		    uchar *tileBuffer);

  int (*checkTileFormat) (TsmConnection conn, TsmTileFormat format);
  int (*checkTIFFFormat) (TsmConnection conn, int format);

  int (*beginLogging) (TsmConnection conn, 
			int method, const char *destination );
  int (*endLogging) (TsmConnection conn);
};

int tsmvBeginLogging (TsmConnection conn, 
		      int method, const char *destination );
int tsmvEndLogging (TsmConnection conn);

#endif
