/*
 * Name: lru.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru.h,v 1.8 2000/11/10 18:50:42 leei Exp $
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

#ifndef _pthr_lru_h_
#define _pthr_lru_h_

/* An LRU (Least Recently Used) cache of pointers */
typedef struct lru_cache lru_cache;
typedef struct lru_handle *lru_handle;

/* Create an lru_cache with len elements.  In an environment with POSIX
   threads, the shared argument can be used to request that the queue be
   shared among multiple threads.  Without POSIX threads, then setting
   shared will cause lru_create() to return NULL. */

lru_cache * lru_create (int len, int shared);

/* Destroy an lru_cache as created by lru_create().  This will decache all
   elements currently in the cache.  If this is a shared cache, then this is
   extremely dangerous unless all other threads accessing the queue have been
   terminated. */

void lru_destroy (lru_cache *lru);

/* Register a callback procedure for use when an element in the cache has been
   removed from the cache. */

void lru_on_decache (lru_cache *lru,
		     void (*decache) (void *val, void *closure),
		     void *closure);

/* Use a pointer in the lru_cache.  This returns a cache handle, which is used
   to make subsequent calls to lru_use() or lru_decache().  If h is not NULL
   then it is validated to ensure that it still references p.  If the (p,h)
   pair is new or invalid, then a new lru_handle is returned. */
lru_handle lru_use   (lru_cache *lru, void *p, lru_handle h);

/* Remove the value from the LRU cache. */
void lru_decache (lru_cache *lru, void *p, lru_handle h);

/* Decache the least recently used handle and return it for reuse.
   N.B. The handle is still attached to the same value pointer, so this should
   be accessed (via lru_handle_value) and reused. */
lru_handle lru_reuse (lru_cache *lru);

/* Retrieve the value associated with a handle. */
void *lru_handle_value (lru_handle h);

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_lru_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "lru_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _pthr_lru_h_ */
