/*
 * Name: tsmRequest.c
 *
 * Description: 
 *   Contains user-level routines for requesting tile data.
 *   These functions are externally visible to the user.
 *
 * Function List:
 *
 * Revision History:
 *   Created Sep 5, 2000 by Lee Iverson <leei@ai.sri.com>
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

#include "tsmRequest.h"
#include "tsmRequestSrv.h"

#include <stdlib.h>
#include <prtypes.h>

struct tsmRequestList_ {
  PRLock *lock;

  tsmRequestServer *server;	/* Server handling this list */

  int ready;			/* TRUE when ready to be processed. */
  void (*submit_cb) (tsmRequestList *req, void *cl);
  void *submit_closure;		/* Call submit_cb(req,submit_closure)
				   to submit this tsmRequestList. */

  int pending;			/* TRUE when a tile is ready. */
  tsmRequestNotifyCB receive_cb;
  void *receive_closure;	/* Call receive_cb(req,hdr,buf,rcvd,receive_closure)
				   to notify caller that a tile has
				   been received. */

  tsmSimpleRequestList lists[3]; /* Triple buffer of request lists. */

  tsmSimpleRequestList *serverList; /* Current server-managed list. */
  tsmSimpleRequestList *clientList; /* Current client accumulator. */
  tsmSimpleRequestList *readyList;  /* In-between for triple-buffer. */
};

tsmRequestList *
tsmRequestCreate (void)
{
  tsmRequestList *req = calloc (1, sizeof (*req));

  req->lock = PR_NewLock ();
  req->server = 0;

  /* Initialize triple buffer */
  req->serverList = &req->lists[0];
  req->clientList = &req->lists[1];
  req->readyList  = &req->lists[2];

  return req;
}

void
tsmRequestBindServer (tsmRequestList *req, tsmRequestServer *server)
{
  req->server = server;
}

tsmRequestServer *
tsmRequestBound (tsmRequestList *req)
{
  return req->server;
}

void
tsmRequestDestroy (tsmRequestList *req)
{
  int i;

  if (req->server) {
    /* The process of destroying must happen inside the server! */
    tsmReqServerForgetList (req->server, req);
  } else {
    /* Otherwise actually destroy.
       N.B. This may be called *from* the server. */
    for (i = 0; i < 3; ++i)
      if (req->lists[i].reqs)
	free (req->lists[i].reqs);

    PR_DestroyLock (req->lock);
    free (req);
  }
}

int
tsmRequestPending (tsmRequestList *req)
{
  return req->pending;
}

int
tsmRequestReady (tsmRequestList *req)
{
  return req->ready;
}

tsmSimpleRequestList *
tsmRequestServerList (tsmRequestList *req)
{
  return req->serverList;
}

/* Initialize submit callback and closure for notification of a
   tsmRequestSubmit */

void
tsmRequestNotifySubmit (tsmRequestList *req, 
			void (*submit_cb) (tsmRequestList *req, void *cl),
			void *closure)
{
  PR_Lock (req->lock); {
    req->submit_cb = submit_cb;
    req->submit_closure = closure;
  }; PR_Unlock (req->lock);
}

void
tsmRequestNotifyReceive (tsmRequestList *req,
			 tsmRequestNotifyCB receive_cb,
			 void *closure)
{
  PR_Lock (req->lock); {
    req->receive_cb = receive_cb;
    req->receive_closure = closure;
  }; PR_Unlock (req->lock);
}

/* Insert a tile request into the clientList.
   
   This is called without locking by the client process.
   The server process(es) *never* manipulate the clientList. */

void
tsmRequestAdd (tsmRequestList *req, int x, int y, int res, int prio)
{
  static char const funcName[] = "tsmRequestAdd";
  tsmSimpleRequestList *client = req->clientList;
  int i;
  tsmTileReq *r;

  /* Lengthen the clientList if necessary */
  if ((i = client->n++) >= client->len) {
    client->len *= 2;
    if (client->len < 64) client->len = 64;
    if (client->reqs)
      client->reqs = realloc (client->reqs, client->len * sizeof(tsmTileReq));
    else
      client->reqs = malloc (client->len * sizeof(tsmTileReq));
  }

  TSMTRACE ((TSMMASK|3, funcName, "%p[%d] = [%d %d %d] prio %d",
	     client, i, x, y, res, prio));
  r = &client->reqs[i];
  r->x = x;
  r->y = y;
  r->res = res;
  r->prio = prio;
}

/* Submit the client list by placing it into the ready list and
   notifying the server via the submit_cb call. */

void
tsmRequestSubmit (tsmRequestList *req)
{
  static char const funcName[] = "tsmRequestSubmit";
  void (*submit_cb) (tsmRequestList *req, void *cl) = 0;
  void *submit_closure;

  PR_Lock (req->lock); {
    /* Swap clientList into readyList... */
    tsmSimpleRequestList *tmp = req->readyList;
    req->readyList = req->clientList;
    req->clientList = tmp;
    req->clientList->n = 0;	/* Clear client list */

    /* Indicate that this new request list is ready to go and notify whoever
       is managing it. */
    req->ready = 1;
    TSMTRACE ((TSMMASK|2, funcName,
	       "%p: ready %p[%d] client %p[%d] ",
	       req, req->readyList, req->readyList->n,
	       req->clientList, req->clientList->n));

    if (0 && TSMTRACEP ((TSMMASK|2))) {
      char buf[2048], *p = buf;
      int i;
      p[0] = '\0';
      for (i = 0; i < req->readyList->n; ++i) {
	tsmTileReq const *treq = &req->readyList->reqs[i];
	sprintf (p, "  [%d %d %d]%d\n", treq->x, treq->y, treq->res, treq->prio);
	p = strrchr (p, '\0');
      }
      TSMTRACE ((TSMMASK|2, funcName, "%p[%d] {\n%s}",
		 req->readyList, req->readyList->n, buf));
    }

    /* Grab inside here. */
    submit_cb = req->submit_cb;
    submit_closure = req->submit_closure;
  }; PR_Unlock (req->lock);

  if (submit_cb)
    submit_cb (req, submit_closure);
}

/* Move the ready list into the safe server-side area.
   
   This is called by the server process as part of the triple
   buffering (as part of the process triggered by the submit_cb call). */

tsmSimpleRequestList *
tsmRequestClaim (tsmRequestList *req)
{
  static char const funcName[] = "tsmRequestClaim";
  tsmSimpleRequestList *ret = 0;
  PR_Lock (req->lock); {
    if (req->ready) {
      ret = req->readyList;
      req->readyList = req->serverList;
      req->serverList = ret;
      req->readyList->n = 0;

      /* Indicate that this new request list has been claimed. */
      TSMTRACE ((TSMMASK|2, funcName,
		 "%p: server %p[%d] ready %p[%d]",
		 req, ret, ret->n, req->readyList, req->readyList->n));
      req->ready = 0;
      req->pending = req->serverList->n;

      if (0 && TSMTRACEP ((TSMMASK|2))) {
	char buf[2048], *p = buf;
	int i;
	p[0] = '\0';
	for (i = 0; i < req->serverList->n; ++i) {
	  tsmTileReq const *treq = &req->serverList->reqs[i];
	  sprintf (p, "  [%d %d %d]%d\n",
		   treq->x, treq->y, treq->res, treq->prio);
	  p = strrchr (p, '\0');
	}
	TSMTRACE ((TSMMASK|2, funcName, "%p[%d] {\n%s}",
		   req->serverList, req->serverList->n, buf));
      }
    }
  }; PR_Unlock (req->lock);

  return ret;
}

/*
 * Returns TRUE iff the tile is on the list and a receive_cb is called and
 * returns TRUE.  The tile is only then considered "received" by the list.
 * If the receive_cb returns TRUE, that means that this request list has no
 * interest in seeing this tile via a call to tsmReadNextTile().
 */
int
tsmRequestReceive (tsmRequestList *req,
		   TsmTileHeader *tileHeader,
		   uchar *tileBuffer,
		   int received)
{
  static char const funcName[] = "tsmRequestReceive";
  int i;
  int x = tsmTileHeaderX (tileHeader);
  int y = tsmTileHeaderY (tileHeader);
  int res = tsmTileHeaderLevel (tileHeader);

  {
    tsmRequestNotifyCB receive_cb;
    void * receive_closure;

    PR_Lock (req->lock); {
      tsmSimpleRequestList *lst = req->serverList;
      tsmTileReq *tileReq = lst->reqs;
      int i, n = lst->n;

      /* Simple linear search for this tile */
      for (i = 0; i < n; ++i, ++tileReq)
	if (tileReq->x == x && tileReq->y == y && tileReq->res == res)
	  break;

      /* If we actually found it in the list... */
      if (i < n) {
	/* Shorten the list */
	req->pending = lst->n = --n;
	*tileReq = lst->reqs[n];
	TSMTRACE ((TSMMASK|2, funcName, "%p[%d] [%d %d %d]", 
		   lst, lst->n, 
		   tileHeader->tx, tileHeader->ty, tileHeader->level));
      }

      /* Grab this while locked, but make the call outside the lock. */
      receive_cb = req->receive_cb;
      receive_closure = req->receive_closure;
    }; PR_Unlock (req->lock);

    /* Notify whoever is waiting for tiles. */
    if (receive_cb) {
      int ret;
      TSMTRACE ((TSMMASK|1, funcName, "Notify list %p: [%d %d %d] buf %p",
		 req, x, y, res, tileBuffer));
      ret = receive_cb (req, tileHeader, tileBuffer, /*received,*/
			receive_closure);
      TSMTRACE ((TSMMASK|1, funcName, ret ? "Received" : "Ignored"));
      return ret;
    }
  }

  return FALSE;
}
