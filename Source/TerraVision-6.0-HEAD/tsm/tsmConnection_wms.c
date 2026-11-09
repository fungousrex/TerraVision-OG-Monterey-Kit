/*
 * Name: tsmConnection_wms.c
 *
 * Description: 
 *   Contains routines for manipulating connections to Web Mapping
 *   Server datasets. These functions are private to the tsmApi.
 *
 * Function List:
 *   TsmConnection tsmConnect_wms( Purl *, TsmAccessMode, TsmConnectionParams )
 *
 * Revision History:
 *   Created 15 Jun 2001 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnection_wms.c,v 1.1.2.4 2001/07/09 04:19:31 leei Exp $
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
#include <ghttp/http_trans.h>

#include "tsmConnect_wms.h"
#include "tsmConnectionParamsInt.h"

static TsmConnectionVTable vtable_web = {
  tsmDisconnect_wms,

  tsmAddRequest_wms,
  tsmRemoveRequest_wms,
  tsmReprioRequest_wms,

  0 /*tsmWriteTile_wms*/,

  tsmCheckTileFormat_wms,
  tsmCheckTIFFFormat_wms,

  tsmvBeginLogging,
  tsmvEndLogging
};

TsmConnection
tsmConnect_wms( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  static const char funcName[] = "tsmConnect_wms";
  TsmConnectionWMS wms_connect;
  TsmConnection connect;

  /* Now we create a connection structure which is specific for the */
  /* connection protocol, i.e. local filesystem, http, or x-dsm.    */

  wms_connect = tsmMemCalloc( 1, sizeof(*wms_connect) );
  connect = &wms_connect->common;

  connect->vtable = &vtable_web;
  
  connect->mode        = mode;
  connect->location    = purl;

  connect->type        = TSM_WMS;
  connect->format      = TSM_LRBT_FORMAT;
  connect->tile_format = TSM_TIFF_TILES;

  /* have we been supplied with a WMS capabilities structure? */

  if ( tsmIsParamSet( param, TSM_PARAM_WMS ) == TRUE ) {
    wms_connect->wms = param->wms;
    wms_connect->free_wms_on_exit = FALSE;
  } else {
    wms_connect->wms = tsmWMSCreate( purl->url );
    if ( wms_connect->wms ) {
      if ( tsmWMSGetCapabilities( wms_connect->wms ) == FALSE ) {
	TSMTRACE(( TSMMASK|1, funcName, "Could not read WMS %s", purl->url ));
	free( wms_connect );
	return NULL;
      }
      wms_connect->free_wms_on_exit = TRUE;
    }
  }
  
  /* Do common initialization */

  connect = tsmConnectCommon (connect, purl, mode, param);
  if (! connect)
    return connect;

  wms_connect->hconn = tsmOpenHostConnection_web (purl);
  wms_connect->http_conn = 0;

  return connect;
}

void
tsmDisconnect_wms (TsmConnection connect)
{
  static const char funcName[] = "tsmDisconnect_wms";
  TsmConnectionWMS wms_connect = (TsmConnectionWMS) connect;

  if ( connect == NULL ) return;

  TSMTRACE ((TSMMASK|1, funcName, "%p", connect));

  if ( wms_connect->wms && wms_connect->free_wms_on_exit )
    tsmWMSFree( &wms_connect->wms );

  if ( wms_connect->hconn )
    tsmCloseHostConnection_web( wms_connect->hconn );

  if ( wms_connect->http_conn )
    tsmCloseHttpConnection( wms_connect->http_conn );

  tsmDisconnectCommon( connect );

  tsmMemManagerFree (wms_connect);
}
