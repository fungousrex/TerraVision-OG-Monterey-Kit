/*
 * Name: lru_hash_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru_hash_inline.h,v 1.11 2000/09/29 22:50:31 leei Exp $
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

/* This is never included prior to lru_hash.h. */

#include "lru_hashI.h"

exinline void
lru_hash_lru_detach (lru_hashtable *lru, lru_hash_handle h)
{
  /* Fix up adjacent prev/next pointers. */
  *h->lru_prev = h->lru_next;
  if (h->lru_next)
    h->lru_next->lru_prev = h->lru_prev;

  /* If was last, then update last pointer. */
  if (h == lru->last)
    lru->last = (lru_hash_handle) h->lru_prev;
  if (h == lru->first)
    lru->first = h->lru_next;
}

exinline void
lru_hash_lru_first (lru_hashtable *lru, lru_hash_handle h)
{
  /* Move h to start. */
  h->lru_next = lru->first;
  h->lru_prev = &lru->first;

  if (lru->first)
    lru->first->lru_prev = &h->lru_next;
  else
    lru->last = h;

  lru->first = h;
}

exinline void
lru_hash_use1 (lru_hashtable *lru, lru_hash_handle h)
{
  /* Just move to front. */
  if (h == lru->first)
    return;

  lru_hash_lru_detach (lru, h);
  lru_hash_lru_first (lru, h);
}

exinline void
lru_hash_use (lru_hashtable *lru, int n, lru_hash_handle *hp)
{
  lru_hash_handle *hp_end = hp+n;

  if (lruh_shared (lru))
    lruh_lock (lru);
  
  for ( ; hp < hp_end; ++hp)
    if (*hp)
      lru_hash_use1 (lru, *hp);

  if (lruh_shared (lru))
    lruh_unlock (lru);
}

lru_hash_handle
lru_hash_decache_lru (lru_hashtable *lru, lru_hash_handle h);

exinline lru_hash_handle
lru_hash_newhandle (lru_hashtable *lru)
{
  lru_hash_handle h;

  if (lru->free) {
    h = lru->free;
    lru->free = h->lru_next;
  } else if (lru->used < lru->len) {
    /* Extend and return. */
    h = &lru->contents[lru->used++];
  } else {
    /* Decache last and use for new. */
    h = lru_hash_decache_lru (lru, lru->last);
  }

  lru_hash_lru_first (lru, h);

  return h;
}

exinline int
lru_hash_key_equal (lru_hashtable *lru, void const *key, lru_hash_handle h)
{
  return h->key == key || (lru->equalf && (*lru->equalf) (key, h->key));
}

lru_hash_handle
lru_hash_assign1 (lru_hashtable *lru,
		  void const *key, lru_hash_handle h, void const *val);

exinline lru_hash_handle
lru_hash_assign (lru_hashtable *lru,
		 void const *key, lru_hash_handle h, void const *val)
{
  if (lruh_shared (lru))
    lruh_lock (lru);
  
  if (h && lru_hash_key_equal (lru, key, h)) {
    h->value = val;
    lru_hash_use1 (lru, h);
  } else {
    h = lru_hash_assign1 (lru, key, h, val);
  }

  if (lruh_shared (lru))
    lruh_unlock (lru);

  return h;
}

lru_hash_handle
lru_hash_lookup1 (lru_hashtable *lru, void const *key);

exinline void *
lru_hash_lookup (lru_hashtable *lru, void const *key, lru_hash_handle *h0)
{
  void *val = 0;
  lru_hash_handle h = (h0 ? *h0 : 0);

  if (h && lru_hash_key_equal (lru, key, h)) {
    val = (void *) h->value;
  } else {
    if (lruh_shared (lru))
      lruh_lock (lru);

    h = lru_hash_lookup1 (lru, key);

    if (h) {
      if (h0) *h0 = h;
      val = (void *) h->value;
    }

    if (lruh_shared (lru))
      lruh_unlock (lru);
  }

  return val;
}

exinline void const *
lru_hash_handle_key (lru_hash_handle h)
{
  return h->key;
}

exinline void *
lru_hash_handle_value (lru_hash_handle h)
{
  return (void *) h->value;
}
