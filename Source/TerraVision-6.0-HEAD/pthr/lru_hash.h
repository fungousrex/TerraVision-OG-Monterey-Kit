/*
 * Name: lru_hash.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru_hash.h,v 1.11 2000/11/10 18:50:42 leei Exp $
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

#ifndef _pthr_lru_hash_h_
#define _pthr_lru_hash_h_

/* An LRU (Least Recently Used) cache of pointers */
typedef struct lru_hashtable lru_hashtable;
typedef struct lru_hash_handle *lru_hash_handle;

/* Create an lru_hashtable with len elements.  This hashtable is basically an
   association list, which associates a (void*) value with a (void*) key,
   which is hashed for lookup.  In an environment with POSIX threads, the
   shared argument can be used to request that the queue be shared among
   multiple threads.  Without POSIX threads, then setting shared will cause
   lru_hash_create() to return NULL. */

lru_hashtable * lru_hash_create (int len,
				 unsigned int (*hashf) (void const *),
				 int (*equalf) (void const *, void const *),
				 int shared);

/* Destroy an lru_hashtable as created by lru_hash_create().  This will
   decache all elements currently in the cache.  If this is a shared cache,
   then this is extremely dangerous unless all other threads accessing the
   queue have been terminated. */

void lru_hash_destroy (lru_hashtable *lru);

/* Register a callback procedure for use when an element in the cache has been
   removed from the cache. */

void lru_hash_on_decache (lru_hashtable *lru,
			  void (*decache) (void *key, void *val, void *closure),
			  void *closure);

/* Create an association in the lru_hashtable.  This returns a cache handle,
   which is used to make subsequent calls to lru_hash_use() or
   lru_hash_decache().  If h is not NULL then it is validated to ensure that
   it still references p.  If the (p,h) pair is new or invalid, then a new
   lru_handle is returned. */
lru_hash_handle lru_hash_assign (lru_hashtable *lru,
				 void const *key, lru_hash_handle h, 
				 void const *val);

/* Lookup a key in the hash table.  If you provide a handle and it correctly
   references the key, then it is used, otherwise a lookup is performed.
   The handle pointer is updated to the proper reference or NULL if not found.
   This does *not* qualify as a use of the hash entry.
*/
void * lru_hash_lookup (lru_hashtable *lru,
			void const *key, lru_hash_handle *hp);

/* Use n hash entries pointed to by hp.  This moves these entries to the front
   of the LRU list. */
void lru_hash_use (lru_hashtable *lru, int n, lru_hash_handle *hp);

/* Return the key associated with a handle. */
void const *
lru_hash_handle_key (lru_hash_handle h);
void *
lru_hash_handle_value (lru_hash_handle h);

/* Delete an association referenced by a key from the hash table.  If a handle
   is provided and it correctly references the key, then it is used. */
void lru_hash_decache (lru_hashtable *lru,
		       void const *key, lru_hash_handle h);


/* Delete all associations in the hash table. */
void lru_hash_clear (lru_hashtable *lru);

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_lru_hash_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "lru_hash_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _pthr_lru_hash_h_ */
