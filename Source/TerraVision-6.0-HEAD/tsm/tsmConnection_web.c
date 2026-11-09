/*
 * Name: tsmConnection_web.c
 *
 * Description: 
 *   Contains routines for manipulating connections to http datasets.
 *   These functions are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   TsmConnection tsmConnect_web( Purl *, TsmAccessMode, TsmConnectionParams )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnection_web.c,v 1.20.2.4 2001/07/09 04:19:31 leei Exp $
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

#ifdef USE_HTTP

#include <ghttp/http_trans.h>

#include "tsmConnect_web.h"
#include "tsmConnectionParamsInt.h"
#include "tsmRequestSrv.h"

static TsmConnectionVTable vtable_web = {
  tsmDisconnect_web,

  tsmAddRequest_web,
  tsmRemoveRequest_web,
  tsmReprioRequest_web,

  0 /*tsmWriteTile_web*/,

  tsmCheckTileFormat_web,
  tsmCheckTIFFFormat_web,

  tsmvBeginLogging,
  tsmvEndLogging
};

TsmConnection
tsmConnect_web( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  static const char funcName[] = "tsmConnect_web";
  TsmConnectionHTTP http_connect;
  TsmConnection connect;

  /* Now we create a connection structure which is specific for the */
  /* connection protocol, i.e. local filesystem, http, or x-dsm.    */

  http_connect = tsmMemManagerCalloc (1, sizeof (*http_connect));
  connect = &http_connect->common;

  connect->vtable = &vtable_web;
  
  connect->mode        = mode;
  connect->location    = purl;

  connect->type        = TSM_HTTP;
  connect->format      = TSM_LRBT_FORMAT;
  connect->tile_format = TSM_TIFF_TILES;

  /* The user can specify if they want to use .tile or .tiff files */

  if ( tsmIsParamSet( param, TSM_PARAM_TIFF ) == TRUE ) 
    connect->tile_format = param->tile_format;

  /* The user can specify the image format to use via the param struct */
  /* (Note that only the TIFF files allow raw and jpeg format. The     */
  /* .tile files only support raw data - which is the default option). */

  if ( connect->tile_format == TSM_TIFF_TILES )
    if ( tsmIsParamSet( param, TSM_PARAM_FORMAT ) == TRUE ) 
      connect->format = param->format;

  /* Do common initialization */
  connect = tsmConnectCommon (connect, purl, mode, param);
  if (!connect)
    return connect;

  if ( connect ) {
    http_connect->hconn = tsmOpenHostConnection_web (purl);
    http_connect->http_conn = 0;
  }

  return connect;
}

void
tsmDisconnect_web (TsmConnection connect)
{
  static const char funcName[] = "tsmDisconnect_web";
  TsmConnectionHTTP http_connect = (TsmConnectionHTTP) connect;

  if ( connect == NULL ) return;

  TSMTRACE ((TSMMASK|1, funcName, "%p", connect));

  if (http_connect->hconn)
    tsmCloseHostConnection_web (http_connect->hconn);

  if (http_connect->http_conn)
    tsmCloseHttpConnection (http_connect->http_conn);

  tsmDisconnectCommon (connect);

  tsmMemManagerFree (http_connect);
}
#else

TsmConnection
tsmConnect_web( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  tsmNotCompiledIn( purl->protocol, "tsmConnect_web" );
  return NULL;
}

void
tsmDisconnect_web( TsmConnection connect )
{
  tsmNotCompiledIn( connect->location->protocol, "tsmDisconnect_web" );
}


#endif
