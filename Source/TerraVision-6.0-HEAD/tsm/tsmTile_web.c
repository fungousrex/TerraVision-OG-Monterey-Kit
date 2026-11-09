/*
 * Name: tsmTile_web.c
 *
 * Description: 
 *   Contains routines for reading and writing tiles over an http link
 *   These functions are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTile_web.c,v 1.16.2.3 2001/06/19 22:04:45 leei Exp $
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
#include "tsmRequestSrv.h"
#include "tsmConnect_web.h"

#ifdef USE_HTTP

int
tsmAddRequest_web (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionHTTP http_conn = (TsmConnectionHTTP) conn;
  tsmTileReq tile;

  if ( req == NULL || conn == NULL ) return FALSE;

  tile = req->req;
  tsmLogTile(conn, TV_TILE_REQ, tile.x, tile.y, tile.res);

  return tsmAddRequest_host (http_conn->hconn, req);
}

int
tsmRemoveRequest_web (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionHTTP http_conn = (TsmConnectionHTTP) conn;
  return tsmRemoveRequest_host (http_conn->hconn, req);
}

int
tsmReprioRequest_web (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionHTTP http_conn = (TsmConnectionHTTP) conn;
  return tsmReprioRequest_host (http_conn->hconn, req);
}

#else /* !USE_HTTP */

int
tsmCheckTileFormat_web( TsmConnection connect, TsmTileFormat format )
{
  tsmNotCompiledIn( connect->location->protocol, "tsmCheckTileFormat_web" );
  return FALSE;
}

int
tsmCheckTIFFFormat_web( TsmConnection connect, int format )
{
  tsmNotCompiledIn( connect->location->protocol, "tsmCheckTIFFFormat_web" );
  return FALSE;
}

#endif /* !USE_HTTP */
