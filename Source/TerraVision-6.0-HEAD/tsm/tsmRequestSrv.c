/*
 * Name: tsmRequestSrv.c
 *
 * Description: 
 *   Contains server-side routines for managing requests of tile data.
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
#include "tsmConnectionInt.h"

#include <prerror.h>
#include <pratom.h>
#include <assert.h>

#include <pthr/list.h>
#include <pthr/heap.h>
#include <pthr/istack.h>

typedef struct {
  tsmRequestList *req;
  tsmSimpleRequestList *lst;
} tsmRequestPair;

typedef struct {
  tsmReqElem *req;
  TsmTileHeader *header;
  uchar *buffer;
} tsmTileData;

static void
tsmReqElemSetStatus (char const *funcName, 
		     tsmReqElem *r, tsmReqStatus st, int idx)
{
  if (TSMTRACEP (TSMMASK|1)) {
    static char const *stStr[] = {
      "Unallocated", "Allocated", "Reading", "Queued", "Delivered"
    };
    char buf[128], *p = buf;
    sprintf (p, "%p[%d %d %d]%d",
	     r, r->req.x, r->req.y, r->req.res, r->req.prio);
    p = strchr (p, '\0');
    if (st != r->status)
      sprintf (p, " : %s->%s", stStr[r->status], stStr[st]), p = strchr (p, '\0');
    if (idx != r->idx) 
      sprintf (p, " [%d->%d]", r->idx, idx), p = strchr (p, '\0');
    TSMTRACE ((TSMMASK|2, funcName, "%s", buf));
  }

  r->status = st;
  r->idx = idx;
}

void
tsmReqElemAllocate (char const *funcName, tsmReqElem *r, void *conn)
{
  r->conn.generic = conn;
  tsmReqElemSetStatus (funcName, r, reqAllocated, r->idx);
  TSMTRACE ((TSMMASK|3, funcName, "[%d %d %d]%d @[%d] %p:%p",
	     r->req.x, r->req.y, r->req.res, r->req.prio, r->idx,
	     r->connect, r->conn.generic));
}

void
tsmReqElemReading (char const *funcName, tsmReqElem *r)
{
  tsmReqElemSetStatus (funcName, r, reqReading, -1);
}

/*
 * Compare two tsmReqElem* objects by priority.
 * 
 * The arguments are void* so that this can be used as the compare function
 * (heap_cmp_t) in <pthr/heap.h>
 */

int
tsmReqElemCmp (void *v1, void *v2)
{
  static char const funcName[] = "tsmReqElemCmp";
  tsmReqElem const *r1 = v1, *r2 = v2;
  TSMTRACE ((TSMMASK|3, funcName,
	     "[%d %d %d]%d @[%d] vs [%d %d %d]%d @[%d] = %d",
	     r1->req.x, r1->req.y, r1->req.res, r1->req.prio, r1->idx,
	     r2->req.x, r2->req.y, r2->req.res, r2->req.prio, r2->idx,
	     r1->req.prio - r2->req.prio));
  return r1->req.prio - r2->req.prio;
}

/*
 * Update the index field of a tsmReqElem.
 * 
 * The argument is void* so that this can be used as the compare function
 * (heap_update_t) in <pthr/heap.h>
 */

void
tsmReqElemUpdate (void *v, int idx)
{
  static char const funcName[] = "tsmReqElemUpdate";
  tsmReqElem *r = v;
  tsmReqElemSetStatus (funcName, r, r->status, idx);
}

struct tsmRequestServer_ {
  TsmConnection conn;		/* Abstract connection being served. */
  int destroying;		/* Am I destroying myself? */

  PRLock *lock;			/* Structure lock. */
  PRCondVar *ready_cond;	/* Notification of tsmRequestList ready. */

  int n, len;			/* Length of the list of lists. */
  tsmRequestPair *lists;	/* List of lists being handled by this
				   server */
  
  heap_t *heap;			/* Local heap for distributing requests */
  istack *heap_stack;
  tsmReqElem *heap_free;	/* Reusable tsmReqElems */
  eqhash_t heap_hash;		/* Hash for request id -> tsmReqElem* */

  list_t *fifo;			/* List of "received" tiles. */

  PRThread *thread;		/* The server broker thread. */
};

static tsmReqElem *
tsmReqElemCreate (tsmRequestServer *srv, tsmTileReq const *req)
{
  static char const funcName[] = "tsmReqElemCreate";
  tsmReqElem *r;

  if (srv->heap_free) {
    r = srv->heap_free;
    srv->heap_free = r->conn.next;
    TSMTRACE ((TSMMASK|3, funcName, "%p->heap_free = %p",
	       srv, srv->heap_free));
    /* r->reqLists is still valid here, but it must be empty! */
  } else {
    r = istack_alloc (srv->heap_stack, sizeof (*r));
    r->reqLists = list_create (FALSE);
  }

  r->req = *req;
  r->errCount = 0;
  r->status = reqUnallocated;
  r->idx = -1;
  r->connect = srv->conn;
  r->conn.generic = 0;

  TSMTRACE ((TSMMASK|3, funcName, "%p[%d %d %d]%d",
	     r, r->req.x, r->req.y, r->req.res, r->req.prio));
  return r;
}

static void
tsmReqElemDestroy (tsmRequestServer *srv, tsmReqElem *r)
{
  static char const funcName[] = "tsmReqElemDestroy";
  tsmReqElem *next = srv->heap_free;

  /* Clear the elem->reqList associations */
  while (! list_empty (r->reqLists))
    list_pop (r->reqLists);

  TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d]%d",
	     r, r->req.x, r->req.y, r->req.res, r->req.prio));
  srv->heap_free = r;
  TSMTRACE ((TSMMASK|3, funcName, "%p->heap_free = %p", srv, r));
  r->conn.next = next;
}

static void
tsmReqElemFreeHeap (tsmRequestServer *srv)
{
  tsmReqElem *r = srv->heap_free;
  while (r) {
    tsmReqElem *next = r->conn.next;
    list_destroy (r->reqLists);
    r = next;
  }
}

static void tsmReqServerDestroyList (tsmRequestServer *srv,
				     int i, int locked);
static void tsmReqServerClaimList (tsmRequestServer *srv, int i);

static void tsmReqServerRescind (tsmRequestServer *srv,
				 tsmRequestList *req,
				 tsmSimpleRequestList const *lst);
static void tsmReqServerSubmit (tsmRequestServer *srv,
				tsmRequestList *req,
				tsmSimpleRequestList const *lst);

static void tsmReqServerSubmitCB (tsmRequestList *req, void *closure);
static void tsmReqServerThread (void *srv);

tsmRequestServer *
tsmReqServerCreate (TsmConnection conn)
{
  static char const funcName[] = "tsmReqServerCreate";

  tsmRequestServer *srv = calloc (1, sizeof (*srv));
  srv->conn = conn;
  srv->destroying = FALSE;

  srv->lock = PR_NewLock ();
  srv->ready_cond = PR_NewCondVar (srv->lock);

  srv->heap = heap_create (512, tsmReqElemCmp, tsmReqElemUpdate);
  srv->heap_stack = istack_create (2048);
  srv->heap_hash = eqhash_create (512);
  srv->heap_free = 0;

  srv->fifo = list_create (TRUE);

  srv->thread = PR_CreateThread(PR_USER_THREAD, 
				tsmReqServerThread, srv,
				PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
				PR_JOINABLE_THREAD, 1<<16);

  TSMTRACE ((TSMMASK|1, funcName, "%p", srv));
  return srv;
}

void
tsmReqServerDestroy (tsmRequestServer *srv)
{
  static char const funcName[] = "tsmReqServerDestroy";
  PRThread *thr = (PRThread *) PR_AtomicSet (&srv->thread, 0);

  assert (sizeof (PRThread*) == sizeof (PRInt32));
  if (! thr) return;

  TSMTRACE ((TSMMASK|1, funcName, "%p", srv));

  PR_Interrupt (thr);
  PR_JoinThread (thr);

  PR_Lock (srv->lock); {
    /* Mark that we are destroying so that we can avoid recursive locks */
    srv->destroying = TRUE;

    /* The call will decrement srv->n */
    while (srv->n)
      tsmReqServerDestroyList (srv, 0, TRUE);

    if (srv->lists) {
      free (srv->lists);
      srv->lists = 0;
    }

    heap_destroy (srv->heap);
    srv->heap = 0;

    eqhash_destroy (srv->heap_hash);
    srv->heap_hash = 0;

    tsmReqElemFreeHeap (srv);
    srv->heap_free = 0;

    istack_destroy (srv->heap_stack);
    srv->heap_stack = 0;
  }; PR_Unlock (srv->lock);

  list_destroy (srv->fifo);
  PR_DestroyCondVar (srv->ready_cond);
  PR_DestroyLock (srv->lock);

  free (srv);
}

tsmRequestList *
tsmReqServerCreateList (tsmRequestServer *srv)
{
  static char const funcName[] = "tsmReqServerCreateList";
  tsmRequestList *req = tsmRequestCreate ();
  int i;

  PR_Lock (srv->lock); {
    if ((i = srv->n++) >= srv->len) {
      srv->len *= 2;
      if (srv->len < 64) srv->len = 64;
      if (srv->lists)
	srv->lists = realloc (srv->lists, srv->len * sizeof(srv->lists[0]));
      else
	srv->lists = malloc (srv->len * sizeof(srv->lists[0]));
    }

    TSMTRACE ((TSMMASK|1, funcName, "%p[%d] = %p", srv, i, req));
    tsmRequestBindServer (req, srv);
    srv->lists[i].req = req;
    srv->lists[i].lst = tsmRequestServerList (req);
  }; PR_Unlock (srv->lock);

  /* Call tsmReqServerSubmitCB when requestlist is submitted. */
  tsmRequestNotifySubmit (req, tsmReqServerSubmitCB, srv);

  return req;
}

static int
tsmReqServerFindList (tsmRequestServer *srv, tsmRequestList *req)
{
  int i, n = srv->n;
  for (i = 0; i < n; ++i)
    if (srv->lists[i].req == req)
      break;
  return i;
}

/*
 * This absolutely requires that srv->lock is held on entry.
 * If hold_lock is TRUE, then we don't unlock for the rescind
 */
int
tsmReqServerForgetList_idx (tsmRequestServer *srv, int i, int locked)
{
  static char const funcName[] = "tsmReqServerForgetList_idx";
  int ret = 0;

  if (! locked) PR_Lock (srv->lock);
  if (i < srv->n) {
    int n;

    /* Shorten list and remove reference to this list */
    TSMTRACE ((TSMMASK|1, funcName, "%p[%d] = %p", srv, i, srv->lists[i].req));

    n = --srv->n;
    if (i < n)
      srv->lists[i] = srv->lists[n];

    ret = 1;
  };
  if (! locked) PR_Unlock (srv->lock);

  return ret;
}

static void
tsmReqServerDestroyList (tsmRequestServer *srv, int i, int locked)
{
  static char const funcName[] = "tsmReqServerDestroyList";
  tsmRequestList *req = srv->lists[i].req;

  TSMTRACE ((TSMMASK|1, funcName, "%p[%d] %p", srv, i, req));
  TSMTRINC ((TSMMASK|1, 1));

  tsmReqServerRescind (srv, req, srv->lists[i].lst);
  tsmReqServerForgetList_idx (srv, i, locked);
  tsmRequestDestroy (req);

  TSMTRINC ((TSMMASK|1, -1));
}

/*
 * This is called externally to notify the server that a list should be
 * forgotten.  It unbinds it and then signals the server to pick this up. */
int
tsmReqServerForgetList (tsmRequestServer *srv, tsmRequestList *req)
{
  int ret = 0;
  if (! srv->destroying) PR_Lock (srv->lock);
  {
    /* This tells the server to delete this request list */
    tsmRequestBindServer (req, 0);

    PR_NotifyCondVar (srv->ready_cond);
  }
  if (! srv->destroying) PR_Unlock (srv->lock);
  return ret;
}

static void
tsmReqServerForgetElem (tsmRequestServer *srv, tsmReqElem *req)
{
  static char const funcName[] = "tsmReqServerForgetElem";
  PRUint32 id = tsmMakeTileId (req->req.x, req->req.y, req->req.res);

  TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d]",
	     req, req->req.x, req->req.y, req->req.res));
  if (! eqhash_delete (srv->heap_hash, (void*)id)) {
    TSMTRACE ((TSMMASK|1, funcName,
	       "Didn't delete %p[%d %d %d]  Possible leak",
	       req, req->req.x, req->req.y, req->req.res));
    return;
  }

  tsmReqElemDestroy (srv, req);
}

static void 
tsmReqElemReprioritize (tsmRequestServer *srv,
			tsmReqElem *r,
			int new_prio)
{
  static char const funcName[] = "tsmReqElemReprioritize";
  tsmTileReq *req = &r->req;
  char const *annotate = "";

  /* Don't worry about received tiles */
  if (r->status == reqQueued)
    return;

  /* Reconstruct the request's priority from remaining requests */
  if (new_prio == 0) {
    list_iter_t i;
    TSMTRACE ((TSMMASK|3, funcName, "%p[%d %d %d] prio %d {",
	       r, req->x, req->y, req->res, req->prio));
    TSMTRINC ((TSMMASK|3, 1));
    for (i = list_first (r->reqLists); ! list_done (i); i = list_next (i)) {
      tsmRequestList *l = list_value (i);
      tsmSimpleRequestList const *srvList = tsmRequestServerList (l);
      int j;
      for (j = 0; j < srvList->n; ++j) {
	tsmTileReq const *sreq = &srvList->reqs[j];
	if (sreq->x == req->x && sreq->y == req->y && sreq->res == req->res) {
	  TSMTRACE ((TSMMASK|3, funcName, "in %p[%d] prio %d",
		     srvList, j, sreq->prio));
	  if (sreq->prio > new_prio)
	    new_prio = sreq->prio;
	  break;
	}
      }
    }
    TSMTRINC ((TSMMASK|3, -1));
    TSMTRACE ((TSMMASK|3, funcName, "}"));
    annotate = " (reconstruct)";
  }

  if (new_prio != req->prio) {
    TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d] prio %d -> %d%s",
	       r, req->x, req->y, req->res, req->prio, new_prio, annotate));
    req->prio = new_prio;

    /* Reprioritize depending on tile status */
    switch (r->status) {
    case reqUnallocated:
      heap_recost_idx (srv->heap, r, r->idx);
      break;
    case reqAllocated:
      tsmReprioritizeRequest (srv->conn, r);
      break;
    }
  }
}

/* Rescind requests for all of the tiles in reqList.
 *
 * First, we find the associated tsmReqElem in the hash table.  We
 * then remove reqList from the tsmReqElem's list of "interested"
 * request lists.  If this list is now empty, then there is no longer
 * any pending request for this tile and we can deallocate it.
 * If there are still pending requests for this tile, then if the
 * priority of this request is the same as the tsmReqElem's priority,
 * we recalculate the priority based on the pending tsmRequestLists.
 */

static void
tsmReqServerRescind (tsmRequestServer *srv,
		     tsmRequestList *reqList,
		     tsmSimpleRequestList const *lst)
{
  static char const funcName[] = "tsmReqServerRescind";
  tsmTileReq const *req;
  int i;

  /* Remove these requests from the heap */
  TSMTRACE ((TSMMASK|2, funcName, "%p:%p[%d]", reqList, lst, lst->n));
  if (0 && TSMTRACEP ((TSMMASK|2))) {
    char buf[2048], *p = buf;
    int i;
    p[0] = '\0';
    for (i = 0; i < lst->n; ++i) {
      tsmTileReq const *treq = &lst->reqs[i];
      sprintf (p, "  [%d %d %d]%d\n", treq->x, treq->y, treq->res, treq->prio);
      p = strrchr (p, '\0');
    }
    TSMTRACE ((TSMMASK|2, funcName, "%p[%d] {\n%s}", lst, lst->n, buf));
  }

  if (lst->n == 0) return;

  TSMTRINC ((TSMMASK|2, 1));
  if (! srv->destroying) PR_Lock (srv->lock);
  {
    for (i = 0, req = lst->reqs; i < lst->n; ++i, ++req) {
      PRUint32 id = tsmMakeTileId (req->x, req->y, req->res);
      tsmReqElem *r = eqhash_get (srv->heap_hash, (void*)id);
      if (r) {
	/* Here we need to possibly adjust priority upwards */
	if (list_remove (r->reqLists, reqList)) {
	  if (list_empty (r->reqLists)) {
	    /* Nobody's requesting it anymore... */
	    TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d]%d @[%d] Delete",
		       r, req->x, req->y, req->res, req->prio, r->idx));
	    TSMTRINC ((TSMMASK|2, 1));

	    if (r->idx >= 0) {
	      /* It needs to be removed from either the connections list,
		 if assigned, or the global one. */
	      if (r->conn.generic) {
		tsmRemoveRequest (srv->conn, r);
	      } else if (! heap_remove_idx (srv->heap, r, r->idx)) {
		abort ();
	      }

	      tsmReqServerForgetElem (srv, r);
	    }

	    TSMTRINC ((TSMMASK|2, -1));
	  } else if (req->prio == r->req.prio) {
	    TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d]%d @[%d] Reprio",
		       r, req->x, req->y, req->res, req->prio, r->idx));
	    tsmReqElemReprioritize (srv, r, 0);
	  }
	} else {
	  TSMTRACE ((TSMMASK|2, funcName, "[%d %d %d]%d ignored!",
		     req->x, req->y, req->res, req->prio));
	}
      } else {
	TSMTRACE ((TSMMASK|2, funcName, "[%d %d %d]%d completed.",
		   req->x, req->y, req->res, req->prio));
      }
    }
  }
  if (! srv->destroying) PR_Unlock (srv->lock);
  TSMTRINC ((TSMMASK|2, -1));
}

/*
 * Process each of the requests in reqList.
 *
 * First, map the tile id to a tsmReqElem* using a hash table.  If
 * there is no such tsmReqElem, then this is a new request.  We create
 * a new tsmReqElem and add it to the allocation heap.  If a
 * tsmReqElem already exists for this tile, then we increment it's
 * reference count and potentially modify it's priority (to greater of
 * new or existing priorities).  */

static void
tsmReqServerSubmit (tsmRequestServer *srv, 
		    tsmRequestList *reqList,
		    tsmSimpleRequestList const *lst)
{
  static char const funcName[] = "tsmReqServerSubmit";
  tsmTileReq const *req;
  int i;

  TSMTRACE ((TSMMASK|1, funcName, "%p:%p[%d]", reqList, lst, lst->n));
  if (0 && TSMTRACEP ((TSMMASK|2))) {
    char buf[2048], *p = buf;
    int i;
    p[0] = '\0';
    for (i = 0; i < lst->n; ++i) {
      tsmTileReq const *treq = &lst->reqs[i];
      sprintf (p, "  [%d %d %d]%d\n", treq->x, treq->y, treq->res, treq->prio);
      p = strrchr (p, '\0');
    }
    TSMTRACE ((TSMMASK|2, funcName, "%p[%d] {\n%s}", lst, lst->n, buf));
  }
  if (lst->n == 0) return;

  TSMTRINC ((TSMMASK|1, 1));
  PR_Lock (srv->lock); {
    /* Add these requests to the heap */
    heap_maybe_resize (srv->heap, lst->n);
    for (i = 0, req = lst->reqs; i < lst->n; ++i, ++req) {
      PRUint32 id = tsmMakeTileId (req->x, req->y, req->res);
      tsmReqElem *r = eqhash_get (srv->heap_hash, (void*)id);
      if (! r) {
	r = tsmReqElemCreate (srv, req);
	eqhash_put (srv->heap_hash, (void*)id, r);
	TSMTRACE ((TSMMASK|2, funcName, "Add %p[%d %d %d]%d",
		   r, req->x, req->y, req->res, req->prio));
	heap_add_nocheck (srv->heap, r);
      } else {
	/* Here we need to possibly adjust priority upwards */
	TSMTRACE ((TSMMASK|2, funcName, "Found %p[%d %d %d]%d @[%d] %p:%p",
		   r, req->x, req->y, req->res, r->req.prio,
		   r->idx, r->connect, r->conn.generic));
	if (req->prio > r->req.prio)
	  tsmReqElemReprioritize (srv, r, req->prio);
      }

      /* Add to list of requestLists */
      list_push (r->reqLists, reqList);
      if (0 && TSMTRACEP (TSMMASK|2)) {
	char buf[256], *p = buf;
	list_iter_t i = list_first (r->reqLists);
	p[0] = '\0';
	for ( ; ! list_done (i); i = list_next (i)) {
	  tsmRequestList *l = list_value (i);
	  sprintf (p, " %p", l);
	  p = strrchr (p, '\0');
	}
	TSMTRACE ((TSMMASK|1, funcName, "Add %p to:%s", reqList, buf));
      }
    }
  }; PR_Unlock (srv->lock);
  TSMTRINC ((TSMMASK|1, -1));
}

void
tsmReqServerUpdateRequests (tsmRequestServer *srv, int i)
{
  static char const funcName[] = "tsmReqServerUpdateRequests";

  assert (i < srv->n);
  TSMTRACE ((TSMMASK|1, funcName, "req[%d]", i));

  if (! tsmRequestBound (srv->lists[i].req))
    tsmReqServerDestroyList (srv, i, FALSE);
  else
    tsmReqServerClaimList (srv, i);
}

/* This function is called inside the server thread when this
   particular list has just been submitted and is marked "ready".
   
   We process the list by saving a copy of its currently registered
   request list, "claiming" the readyList, adding the requests from
   the readyList to the server's heap, and then removing the saved
   requests.

   This is done in this order so that the reference counts for
   requests on the new readyList will not be zeroed.
   */

static void
tsmReqServerClaimList (tsmRequestServer *srv, int i)
{
  static char const funcName[] = "tsmReqServerClaimList";

  tsmSimpleRequestList prev, *curr = srv->lists[i].lst;
  tsmTileReq *prev_reqs = 0;
  tsmRequestList *req = srv->lists[i].req;

  /* Save a copy of old requests */
  prev = *curr;
  TSMTRACE ((TSMMASK|2, funcName, "[%d] %p[%d] save %p", 
	     i, curr, curr->n, &prev));
  if (prev.n) {
    prev_reqs = malloc (prev.n * sizeof (*prev_reqs));
    memcpy (prev_reqs, prev.reqs, prev.n * sizeof (*prev_reqs));
    prev.reqs = prev_reqs;
  }
  if (0 && TSMTRACEP ((TSMMASK|2))) {
    char buf[2048], *p = buf;
    int i;
    p[0] = '\0';
    for (i = 0; i < prev.n; ++i) {
      tsmTileReq const *treq = &prev.reqs[i];
      sprintf (p, "  [%d %d %d]%d\n", treq->x, treq->y, treq->res, treq->prio);
      p = strrchr (p, '\0');
    }
    TSMTRACE ((TSMMASK|2, funcName, "%p[%d] {\n%s}", &prev, prev.n, buf));
  }

  /* Now claim new requests */
  curr = tsmRequestClaim (req);
  TSMTRACE ((TSMMASK|2, funcName, "[%d] curr %p[%d]", i, curr, curr->n));

  if (curr) {
    srv->lists[i].lst = curr;

    /* Update requests by submitting new and rescinding previous */
    tsmReqServerSubmit (srv, req, curr);
    if (prev_reqs)
      tsmReqServerRescind (srv, req, &prev);
  }

  /* Free saved previous list */
  if (prev_reqs)
    free (prev_reqs);
}

static void
tsmReqServerSubmitCB (tsmRequestList *req, void *closure)
{
  static char const funcName[] = "tsmReqServerSubmitCB";
  tsmRequestServer *srv = closure;
  PR_Lock (srv->lock); {
    TSMTRACE ((TSMMASK|2, funcName, "signal %p: req %p", srv, req));
    PR_NotifyCondVar (srv->ready_cond);
  }; PR_Unlock (srv->lock);
}

/*
 * Returns the index of the next requestlist ready *after* the index end.
 */
static int
tsmReqServerNextReady (tsmRequestServer *srv, int end)
{
  static char const funcName[] = "tsmReqServerNextReady";
  tsmRequestList *req;
  int i, done;
  int n = srv->n;

  /* No lists means none ready. */
  if (n == 0)
    return -1;

  /* Make sure termination condition is reasonable. */
  while (end < 0)
    end += n;
  if (end >= n) end = 0;

  TSMTRACE((TSMMASK|2, funcName, "[%d] start %d end %d", srv->n, end+1, end));

  /* Now look ahead for the next ready one. */
  for (i = end+1, done = 0; ! done; ++i) {
    /* We're locked...!! */
    assert (n == srv->n);
    if (i >= n) i = 0;
    if (i == end) ++done;
    req = srv->lists[i].req;
    if (! tsmRequestBound (req) || tsmRequestReady (req)) {
      TSMTRACE((TSMMASK|2, funcName, "[%d] %p", i, req));
      return i;
    }
  }

  TSMTRACE((TSMMASK|2, funcName, "None"));
  return -1;
}

static void
tsmReqServerAssignRequests (tsmRequestServer *srv)
{
  static char const funcName[] = "tsmReqServerAssignRequests";
  TsmConnection conn = srv->conn;

  TSMTRACE ((TSMMASK|2, funcName, "%p", srv));
  TSMTRINC ((TSMMASK|2, 1));
  PR_Lock(srv->lock); {
    while (! heap_is_empty (srv->heap)) {
      tsmReqElem *req = heap_pop (srv->heap);
      tsmAddRequest (conn, req);
    }
  }; PR_Unlock(srv->lock);
  TSMTRINC ((TSMMASK|2, -1));
}

static void
tsmReqServerThread (void *arg)
{
  static char const funcName[] = "tsmReqServerThread";
  tsmRequestServer *srv = arg;
  int done = 0;
  int i = -1;

  /* Set the thread's name. */
  {
    char buf[256];
    Pyramid * tspec = (Pyramid *) tsmGet (srv->conn, TSM_DATASET_TSPEC);
    if ( tspec == NULL ) return;

    sprintf (buf, "%s ReqServer", tspec->identityTspec.name);
    pthr_set_name (PR_CurrentThread (), buf);
  }
  TSMTRACE ((TSMMASK|1, funcName, "%p", srv));

  while (! done) {
    PRStatus status = PR_SUCCESS;
    PR_Lock (srv->lock); {
      while ((i = tsmReqServerNextReady (srv, i)) == -1) {
	TSMTRACE ((TSMMASK|2, funcName, "Waiting for ready"));
	status = PR_WaitCondVar (srv->ready_cond, PR_INTERVAL_NO_TIMEOUT);
	if (status != PR_SUCCESS)
	  break;
      }
    }; PR_Unlock (srv->lock);
    if (status == PR_FAILURE) {
      PRErrorCode err = PR_GetError ();
      if (err == PR_PENDING_INTERRUPT_ERROR) {
	TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	break;
      }
    }

    /* So, there is a list waiting at index i */
    tsmReqServerUpdateRequests (srv, i);

    tsmReqServerAssignRequests (srv);
  }
}

/**
 * Grab the list of registered receivers for this tile.
 *
 * Returns the number of lists stored in the receivers
 */

list_t *
tsmReqReceivers (tsmReqElem *r)
{
  tsmRequestServer *srv = r->connect->srv;
  list_t *rcvs = list_create (FALSE);
  PR_Lock (srv->lock); {
    list_iter_t i = list_first (r->reqLists);
    for ( ; ! list_done (i); i = list_next (i)) {
      tsmRequestList *l = list_value (i);
      list_push (rcvs, l);
    }
  }; PR_Unlock (srv->lock);

  return rcvs;
}

void
tsmReqServerQueueTile (tsmRequestServer *srv,
		       tsmReqElem *r,
		       TsmTileHeader *hdr,
		       uchar *buf,
		       list_t *reqLists)
{
  static char const funcName[] = "tsmReqServerQueueTile";
  int received = FALSE;
  
  /* A tile is considered "received" if any request list "receives" the tile
   * via the tsmRequestNotifyReceive callback.  This return value is
   * equivalent to a statement by the receiver that it has taken control of
   * the tileBuffer memory.
   *
   * N.B. Don't allow new requests to be registered while we're checking this.
   */
  if (! reqLists)
    reqLists = r->reqLists;
  PR_Lock (srv->lock); {
    list_iter_t i = list_first (reqLists);

    /* Notify request lists that tile has been received */
    for ( ; ! list_done (i); i = list_next (i)) {
      tsmRequestList *l = list_value (i);
      if (tsmRequestReceive (l, hdr, buf, received))
	received = TRUE;
    }

    /* We're done with it. */
    if (received)
      tsmReqServerForgetElem (srv, r);
  }; PR_Unlock (srv->lock);

  /* No need to continue, it's been delivered. We should free the header
     here. */
  if (received) {
    tsmFreeTileHeader (hdr);
    return;
  }

  /* If the tile was not "received" above, then queue it for later reading */
  {
    tsmTileData *data = malloc (sizeof (*data));
    data->req = r;
    tsmReqElemSetStatus (funcName, r, reqQueued, r->idx);
    data->header = hdr;
    data->buffer = buf;
    list_shift (srv->fifo, data);
  }
}

int
tsmReqServerDequeueTile (tsmRequestServer *srv,
			 TsmTileHeader **hdrp,
			 uchar **bufp)
{
  static char const funcName[] = "tsmReqServerDequeueTile";
  tsmTileData *data;
  tsmReqElem *r;

  /* If we're not blocking, and nothing's ready, just return. */
  if ( ! srv->conn->blocking_read && list_empty (srv->fifo))
    return TSM_NODATA;

  /* Since this is a shared list, this is a blocking read */
  data = list_pop (srv->fifo);
  if (! data)
    return TSM_NODATA;

  r = data->req;
  tsmReqElemSetStatus (funcName, r, reqDelivered, r->idx);

  /* N.B. Whoever calls this is responsible for freeing the both the data and
     header buffers! */
  *hdrp = data->header;
  *bufp = data->buffer;

  /* Have to do this locked. */
  PR_Lock (srv->lock); {
    tsmReqElem *r = data->req;
    tsmReqServerForgetElem (srv, r);
  }; PR_Unlock (srv->lock);

  free (data);

  return !! *bufp;
}
