/*
 * Name: tsmConnect_wms.h
 *
 * Description: 
 *   Internal definitions for Web Mapping Server connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnect_wms.h,v 1.1.2.2 2001/07/09 19:04:06 leei Exp $
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

#ifndef TSMCONNECTWMS_H
#define TSMCONNECTWMS_H

#include "tsmInternal.h"
#include "tsmConnect_web.h"
#include "tsmWMS.h"

#include <pthr/heap.h>

/* The TsmConnectionWMS structure holds all of the state for */
/* managing a connection to a dataset on an OGC WMS server   */

typedef struct TsmConnectionWMS *TsmConnectionWMS;

struct TsmConnectionWMS {
  struct TsmConnectionCommon common;

  tsmWMS *wms;                    /* The WMS Capabilities structure */
  int    free_wms_on_exit;        /* was WMS struct passed in or allocated */

  HostConnection   hconn;         /* host aggregated connection */
  http_trans_conn *http_conn;     /* HTTP connection structure */
};

extern void tsmDisconnect_wms( TsmConnection );

extern int tsmAddRequest_wms( TsmConnection conn, tsmReqElem *req );
extern int tsmRemoveRequest_wms( TsmConnection conn, tsmReqElem *req );
extern int tsmReprioRequest_wms( TsmConnection conn, tsmReqElem *req );

extern int tsmCheckTileFormat_wms( TsmConnection, TsmTileFormat );
extern int tsmCheckTIFFFormat_wms( TsmConnection, int );

extern int tsmWMSReadNextTile( HostConnection connect, http_trans_conn **,
			       tsmReqElem const *r, TsmTileHeader *, uchar * );


#endif /* TSMCONNECTWMS_H */
