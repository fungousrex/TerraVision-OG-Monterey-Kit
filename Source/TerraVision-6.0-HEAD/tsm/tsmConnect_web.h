/*
 * Name: tsmConnectWeb.h
 *
 * Description: 
 *   Internal definitions for HTTP-based connections.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnect_web.h,v 1.3.2.2 2001/06/19 22:04:39 leei Exp $
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

#ifndef TSMCONNECTWEB_H
#define TSMCONNECTWEB_H

#include "tsmInternal.h"
#include "tsmConnectionInt.h"

#include <pthr/heap.h>

/* The TsmConnectionHTTP structure holds all of the state for */
/* managing a connection to a dataset through an http server. */

typedef struct TsmConnectionHTTP *TsmConnectionHTTP;
typedef struct HostConnection_ *HostConnection;

struct TsmConnectionHTTP {
  struct TsmConnectionCommon common;

  HostConnection   hconn;
  http_trans_conn *http_conn;
};

extern void tsmDisconnect_web( TsmConnection );

extern void tsmReaderThread_web (void *ptr);

extern int tsmAddRequest_web (TsmConnection conn, tsmReqElem *req);
extern int tsmRemoveRequest_web (TsmConnection conn, tsmReqElem *req);
extern int tsmReprioRequest_web (TsmConnection conn, tsmReqElem *req);

extern int tsmCheckTileFormat_web (TsmConnection aconn,
				   TsmTileFormat format );
extern int tsmCheckTIFFFormat_web( TsmConnection aconn,
				   int format );

HostConnection tsmOpenHostConnection_web (Purl *purl);
int tsmCloseHostConnection_web (HostConnection hconn);

int tsmAddRequest_host (HostConnection hconn, tsmReqElem *req);
int tsmRemoveRequest_host (HostConnection hconn, tsmReqElem *req);
int tsmReprioRequest_host (HostConnection hconn, tsmReqElem *req);

int tsmCheckTileFormat_host (TsmConnection com_connect, 
			     TsmTileFormat format);
int tsmCheckTIFFFormat_host( TsmConnection com_connect,
			     int format );
#endif /* TSMCONNECTWEB_H */
