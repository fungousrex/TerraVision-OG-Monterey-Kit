/*
 * Name: tsmRequestSrv.h
 *
 * Description: 
 *   Internal definitions for handling request lists.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmRequestSrv.h,v 1.3.4.5 2002/03/13 23:40:56 leei Exp $
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

#ifndef TSMREQUESTSRV_H
#define TSMREQUESTSRV_H

#include "tsmRequest.h"

#include <pthr/list.h>

/*
 * The processing status of a tsmReqElem.
 */

typedef enum { 
  reqUnallocated,		/* Not yet processed */
  reqAllocated,			/* Assigned to a particular connection */
  reqReading,			/* Being read */
  reqQueued,			/* Finished reading.  Queued for delivery */
  reqDelivered			/* Delivered to external interested parties */
} tsmReqStatus;

struct tsmReqElem_ {
  tsmTileReq req;		/* The request */
  PRInt16 idx;			/* Location of this element in a list/heap */
  tsmReqStatus status:16;	/* Current processing status of this request */
  int errCount;			/* How many times has a read attempt failed? */
  list_t *reqLists;		/* List of all tsmRequestLists that
				   have asked for this tile. */

  TsmConnection connect;
  union {
    void *generic;
    struct tsmReqElem_ *next;
    struct HostConnection_ *hconn;
  } conn;
};

extern tsmRequestServer * tsmReqServerCreate (TsmConnection conn);
extern void tsmReqServerDestroy (tsmRequestServer *srv);

extern tsmRequestList * tsmReqServerCreateList (tsmRequestServer *srv);
extern int tsmReqServerForgetList (tsmRequestServer *srv, tsmRequestList *req);

extern list_t * tsmReqReceivers (tsmReqElem *r);
extern void tsmReqServerQueueTile (tsmRequestServer *srv,
				   tsmReqElem *req,
				   TsmTileHeader *hdr,
				   uchar *buf,
				   list_t *reqLists);
extern int tsmReqServerDequeueTile (tsmRequestServer *srv,
				    TsmTileHeader **hdrp,
				    uchar **bufp);

extern int tsmReqElemCmp (void *v1, void *v2);
extern void tsmReqElemUpdate (void *v, int idx);

extern void tsmReqElemAllocate (char const *funcName, tsmReqElem *r, void *conn);
extern void tsmReqElemReading (char const *funcName, tsmReqElem *r);

#endif /* TSMREQUESTSRV_H */
