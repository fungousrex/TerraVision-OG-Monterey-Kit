/*
 * Name: lru_hashI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru_hashI.h,v 1.1 2000/09/29 22:50:31 leei Exp $
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

#ifndef _lru_hashI_h_
#define _lru_hashI_h_

/* This is never included prior to lru_hash.h. */

typedef void (*lruh_shared_f) (lru_hashtable *lru);

struct lru_hash_handle {
  lru_hash_handle lru_next, *lru_prev;
  lru_hash_handle hash_next, *hash_prev;
  void const *key, *value;
};

struct lru_hashtable {
  unsigned int len;
  int used;
  lru_hash_handle first, last;
  lru_hash_handle contents;
  lru_hash_handle free;

  int ht_len;
  lru_hash_handle *hashtable;

  void (*decache_f) (void *key, void *value, void *closure);
  void *decache_closure;

  unsigned int (*hashf) (void const *);
  int (*equalf) (void const *, void const *);

  lruh_shared_f *shared;
};

#define lruh_shared(l)   ((l)->shared)
#define lruh_lock(l)     ((*(l)->shared[0])(l))
#define lruh_unlock(l)   ((*(l)->shared[1])(l))

extern lru_hash_handle lru_hash_newhandle (lru_hashtable *lru);

#endif /* _lru_hashI_h_ */
