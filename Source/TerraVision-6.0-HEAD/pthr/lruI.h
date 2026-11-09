/*
 * Name: lruI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lruI.h,v 1.1 2000/09/29 22:50:31 leei Exp $
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

#ifndef _lruI_h_
#define _lruI_h_

struct lru_handle {
  lru_handle next, *prev;
  void *p;
};

typedef void (*lru_shared_f) (lru_cache *);

struct lru_cache {
  unsigned int len;
  int used;
  struct lru_handle top;
  lru_handle contents;
  lru_handle free;

  void (*decache_f) (void *p, void *closure);
  void *decache_closure;

  lru_shared_f *shared;
};


#define lru_shared(l)   ((l)->shared)
#define lru_lock(l)     ((*(l)->shared[0])(l))
#define lru_unlock(l)   ((*(l)->shared[1])(l))

extern lru_handle lru_first (lru_cache *lru);
extern lru_handle lru_last (lru_cache *lru);
extern lru_handle lru_decache_lru (lru_cache *lru, lru_handle h);

#endif /* _lruI_h_ */
