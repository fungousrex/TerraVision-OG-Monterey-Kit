/*
 * Name: aqueue.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: aqueue.c,v 1.14 2000/09/29 22:50:31 leei Exp $
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <prthread.h>
#include <prlock.h>
#include <prcvar.h>

#include <stdlib.h>
#include <string.h>

#include "aqueue.h"
#include "aqueueI.h"

typedef struct {
  aqueue aq;
  PRLock* lock;
  PRCondVar* not_empty;
} shared_aqueue;

static inline int
aq_lock (aqueue *aq)
{
  PR_Lock (((shared_aqueue*) aq)->lock);
  return 1;
}

static inline int
aq_unlock (aqueue *aq)
{
  PR_Unlock (((shared_aqueue*) aq)->lock);
  return 1;
}

static int
aq_notempty (aqueue *aq)
{
  PR_NotifyCondVar (((shared_aqueue*) aq)->not_empty);
  return aq_unlock (aq);
}

static int
aq_wait (aqueue *aq)
{
  aq_lock (aq);
  while (aqueue_empty (aq))
    if (PR_WaitCondVar (((shared_aqueue*) aq)->not_empty,
			PR_INTERVAL_NO_TIMEOUT) != PR_SUCCESS)
      break;
  return ! aqueue_empty (aq);
}

static aqueue_shared_f locking_funcs[] = {
  aq_lock, aq_unlock, aq_notempty, aq_wait
};

aqueue *
aqueue_create (int len, int shared)
{
  aqueue *aq;
  if (len == 0) len = 64;
  
  if (shared) {
    shared_aqueue *aqs = (shared_aqueue *) malloc (sizeof (shared_aqueue));
    if (! aqs) return 0;
    aqs->lock = PR_NewLock ();
    aqs->not_empty = PR_NewCondVar (aqs->lock);

    aq = &aqs->aq;
    aq->shared = locking_funcs;
  } else {
    if (shared)
      return 0;
    aq = (aqueue *) malloc (sizeof (aqueue));
    if (! aq) return 0;
    aq->shared = 0;
  }

  aq->head = aq->tail = 0;
  aq->len = len;
  aq->contents = (void **) malloc (len * sizeof (void *));

  return aq;
}

void
aqueue_destroy (aqueue *aq)
{
  if (aqueue_shared (aq)) {
    shared_aqueue *aqs = (shared_aqueue *) aq;
    PR_DestroyCondVar (aqs->not_empty);
    PR_DestroyLock (aqs->lock);
  }

  free (aq->contents);
  free (aq);
}

int
aqueue_expand (aqueue *aq)
{
  int len = aq->len, new_len = 2*len;
  void **new_contents = (void **) malloc (new_len * sizeof (void *));
  int n = len - aq->head;

  if (! new_contents) return 0;

  /* Since the queue is full, we know that n+tail == len-1. */
  memcpy (new_contents, &aq->contents[aq->head], n * sizeof (void *));
  if (n < len-1)
    memcpy (&new_contents[n], aq->contents, aq->tail * sizeof (void *));

  free (aq->contents);
  aq->head = 0;
  aq->tail = len-1;
  aq->len = new_len;
  aq->contents = new_contents;

  return ! aqueue_full (aq);
}
