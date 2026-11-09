/*
 * Name: lru.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru.c,v 1.17 2000/09/29 22:50:31 leei Exp $
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

#include <prlock.h>

#include <stdlib.h>

#include "lru.h"
#include "lruI.h"

typedef struct {
  lru_cache lru;
  PRLock *lock;
} shared_lru_cache;

static inline void
l_lock (lru_cache *l)
{
  PR_Lock (((shared_lru_cache*) l)->lock);
}

static inline void
l_unlock (lru_cache *l)
{
  PR_Unlock (((shared_lru_cache*) l)->lock);
}

static lru_shared_f locking_funcs[] = {
  l_lock, l_unlock
};

lru_cache *
lru_create (int len, int shared)
{
  lru_cache *lru;
  if (len == 0) return 0;
  
  if (shared) {
    shared_lru_cache *lrus =
      (shared_lru_cache *) malloc (sizeof (shared_lru_cache));
    if (! lrus) return 0;
    lrus->lock = PR_NewLock();
    lru = &lrus->lru;
    lru->shared = locking_funcs;
  } else {
    lru = (lru_cache *) malloc (sizeof (lru_cache));
    if (! lru) return 0;
    lru->shared = 0;
  }

  lru->len = len;
  lru->used = 0;
  lru->contents = (lru_handle) malloc (len * sizeof (lru->contents[0]));
  lru->top.next = &lru->top;
  lru->top.prev = &lru->top.next;
  lru->free = 0;
  lru->decache_f = 0;
  lru->decache_closure = 0;

  return lru;
}

void
lru_destroy (lru_cache *lru)
{
  if (lru_shared (lru))
    lru_lock (lru);

  if (lru->decache_f) {
    int i;

    for (i = 0; i < lru->used; ++i)
      (*lru->decache_f) (lru->contents[i].p, lru->decache_closure);
  }

  free (lru->contents);
  
  if (lru_shared (lru)) {
    shared_lru_cache *lrus = (shared_lru_cache *) lru;
    lru_unlock (lru);
    PR_DestroyLock(lrus->lock);
  }

  free (lru);
}

void
lru_on_decache (lru_cache *lru,
		void (*decache) (void *val, void *closure),
		void *closure)
{
  lru->decache_f = decache;
  lru->decache_closure = closure;
}

lru_handle 
lru_use (lru_cache *lru, void *p, lru_handle h)
{
  if (lru_shared (lru))
    lru_lock (lru);

  if (h && h->p == p) {
    /* Just move to front. */
    if (h == lru_first (lru)) {
      if (lru_shared (lru))
	lru_unlock (lru);
      return h;
    }
    lru_detach (h);
  } else if (lru->free) {
    h = lru->free;
    lru->free = h->next;
  } else if (lru->used < lru->len) {
    /* Extend and return. */
    h = &lru->contents[lru->used++];
    h->p = p;
  } else {
    /* Decache last and use for new. */
    h = lru_decache_lru (lru, lru_last (lru));
    h->p = p;
  }

  /* Move h to start. */
  lru_insert_after (h, &lru->top);

  if (lru_shared (lru))
    lru_unlock (lru);

  return h;
}

void
lru_decache (lru_cache *lru, void *p, lru_handle h)
{
  if (h && h->p == p) {
    /* Hasn't already been decached. */
    if (lru_shared (lru))
      lru_lock (lru);

    lru_decache_lru (lru, h);
    h->p = 0;

    /* Add to free list. */
    h->next = lru->free;
    lru->free = h;

    if (lru_shared (lru))
      lru_unlock (lru);
  }
}

lru_handle
lru_reuse (lru_cache *lru)
{
  /* Hasn't already been decached. */
  lru_handle h;
  
  if (lru_shared (lru))
    lru_lock (lru);

  /* Grab the least recently used. */
  h = lru_decache_lru (lru, lru_last (lru));

  /* Move h to start. */
  lru_insert_after (h, &lru->top);
  
  if (lru_shared (lru))
    lru_unlock (lru);

  return h;
}

