/*
 * Name: lru_hash.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru_hash.c,v 1.24 2000/09/29 22:50:31 leei Exp $
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

#include <prtypes.h>

#include <prlock.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "lru_hash.h"
#include "lru_hashI.h"

typedef struct {
  lru_hashtable lru;
  PRLock *lock;
} shared_lru_hashtable;

static inline void
l_lock (lru_hashtable *l)
{
  PR_Lock (((shared_lru_hashtable*) l)->lock);
}

static inline void
l_unlock (lru_hashtable *l)
{
  PR_Unlock (((shared_lru_hashtable*) l)->lock);
}

static lruh_shared_f locking_funcs[] = {
  l_lock, l_unlock
};

lru_hashtable *
lru_hash_create (int len, 
		 unsigned int (*hashf) (void const *),
		 int (*equalf) (void const *, void const *),
		 int shared)
{
  lru_hashtable *lru;
  if (len == 0) return 0;
  
  if (shared) {
    shared_lru_hashtable *lrus =
      (shared_lru_hashtable *) malloc (sizeof (shared_lru_hashtable));
    if (! lrus) return 0;
    lrus->lock = PR_NewLock();
    lru = &lrus->lru;
    lru->shared = locking_funcs;
  } else {
    lru = (lru_hashtable *) malloc (sizeof (lru_hashtable));
    if (! lru) return 0;
    lru->shared = 0;
  }

  lru->len = len;
  lru->hashf = hashf;
  lru->equalf = equalf;
  lru->used = 0;
  lru->contents = (lru_hash_handle) malloc (len * sizeof (lru->contents[0]));
  lru->first = lru->last = 0;
  lru->free = 0;

  lru->ht_len = 3 * len;
  lru->hashtable = (lru_hash_handle *)
    calloc (lru->ht_len, sizeof (lru->hashtable[0]));

  lru->decache_f = 0;
  lru->decache_closure = 0;

  return lru;
}

void
lru_hash_destroy (lru_hashtable *lru)
{
  if (lru->decache_f) {
    int i;
    if (lruh_shared (lru))
      lruh_lock (lru);

    for (i = 0; i < lru->used; ++i)
      (*lru->decache_f) ((void *) lru->contents[i].key,
			 (void *) lru->contents[i].value,
			 lru->decache_closure);

    if (lruh_shared (lru))
      lruh_unlock (lru);
  }

  if (lruh_shared (lru)) {
    shared_lru_hashtable *lrus = (shared_lru_hashtable *) lru;
    PR_DestroyLock (lrus->lock);
  }

  free (lru->contents);
  free (lru->hashtable);
  free (lru);
}

void
lru_hash_on_decache (lru_hashtable *lru,
		     void (*decache) (void *key, void *val, void *closure),
		     void *closure)
{
  lru->decache_f = decache;
  lru->decache_closure = closure;
}

lru_hash_handle
lru_hash_decache_lru (lru_hashtable *lru, lru_hash_handle h)
{
  if (lru->decache_f)
    (*lru->decache_f) ((void *) h->key, (void *) h->value,
		       lru->decache_closure);

  lru_hash_lru_detach (lru, h);

  *h->hash_prev = h->hash_next;
  if (h->hash_next)
    h->hash_next->hash_prev = h->hash_prev;

  h->key = 0;

  return h;
}

/* Multiplicative hash function. */
#define A (2654435769U)
#define HASH(k) (A*((PRUint32)k))

lru_hash_handle
lru_hash_assign1 (lru_hashtable *lru,
		  void const *key, lru_hash_handle h, void const *val)
{
  unsigned int hash = lru->hashf ? (*lru->hashf) (key) : HASH (key);
  lru_hash_handle *hp = &lru->hashtable[hash%lru->ht_len];

  for (h = *hp; h; hp = &h->hash_next, h = *hp)
    if (lru_hash_key_equal (lru, key, h))
      break;
  if (h) {
    h->value = val;
    lru_hash_use1 (lru, h);
  } else {
    h = lru_hash_newhandle (lru);
    h->key = key;
    h->value = val;
    /* Insert in hash table. */
    *hp = h;
    h->hash_next = 0;
    h->hash_prev = hp;
  }

  return h;
}

lru_hash_handle
lru_hash_lookup1 (lru_hashtable *lru, void const *key)
{
  lru_hash_handle h;
  unsigned int hash = lru->hashf ? (*lru->hashf) (key) : HASH (key);
  lru_hash_handle *hp = &lru->hashtable[hash%lru->ht_len];

  for (h = *hp; h; hp = &h->hash_next, h = *hp)
    if (lru_hash_key_equal (lru, key, h))
      break;
  return h;
}

void
lru_hash_decache (lru_hashtable *lru, void const *key, lru_hash_handle h)
{
  if (! h || ! lru_hash_key_equal (lru, key, h))
    h = lru_hash_lookup1 (lru, key);

  if (h) {
    if (lruh_shared (lru))
      lruh_lock (lru);

    /* Hasn't already been decached. */
    lru_hash_decache_lru (lru, h);

    /* Add it to start of free list. */
    h->lru_next = lru->free;
    lru->free = h;

    if (lruh_shared (lru))
      lruh_unlock (lru);
  }
}

void
lru_hash_clear (lru_hashtable *lru)
{
  int i;
  
  if (lruh_shared (lru))
    lruh_lock (lru);

  for (i = 0; i < lru->used; ++i) {
    lru_hash_handle h = &lru->contents[i];
    if (lru->decache_f)
      (*lru->decache_f) ((void *) h->key, (void *) h->value,
			 lru->decache_closure);
    h->value = 0;
  }

  lru->used = 0;
  lru->first = lru->last = 0;

  memset (lru->hashtable, 0, 
	  lru->ht_len * sizeof (lru->hashtable[0]));

  if (lruh_shared (lru))
    lruh_unlock (lru);
}

void *
lru_hash_check_lookup (lru_hashtable *lru, void const *key)
{
  lru_hash_handle h, h_end;

  if (lruh_shared (lru))
    lruh_lock (lru);

  for (h = lru->contents, h_end = h + lru->used; h < h_end; ++h)
    if (lru_hash_key_equal (lru, key, h))
      return (void *) h->value;
  
  if (lruh_shared (lru))
    lruh_unlock (lru);

  return NULL;
}
