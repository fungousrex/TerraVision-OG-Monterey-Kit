/*
 * Name: tsmRequest.h
 *
 * Description: 
 *   Internal definitions for handling request lists.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmRequest.h,v 1.4.2.1 2002/03/13 23:40:56 leei Exp $
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

#ifndef TSMREQUEST_H
#define TSMREQUEST_H

typedef struct {
  PRUint16 prio;
  PRUint16 x, y, res;
} tsmTileReq;

typedef struct {
  PRUint32 n, len;
  tsmTileReq *reqs;
} tsmSimpleRequestList;

#include "tsmRequestSrv.h"

extern tsmRequestList * tsmRequestCreate (void);

extern void tsmRequestBindServer (tsmRequestList *req, tsmRequestServer *srv);
extern tsmRequestServer * tsmRequestBound (tsmRequestList *req);

extern int tsmRequestReady (tsmRequestList *req);

extern void tsmRequestNotifySubmit (tsmRequestList *req, 
				    void (*submit_cb) (tsmRequestList *req,
						       void *cl),
				    void *closure);

extern tsmSimpleRequestList * tsmRequestClaim (tsmRequestList *req);

extern int tsmRequestReceive (tsmRequestList *req,
			      TsmTileHeader *tileHeader,
			      uchar *tileBuffer,
			      int received);

extern tsmSimpleRequestList * tsmRequestServerList (tsmRequestList *req);

#endif /* TSMREQUEST_H */
