/*
 * Name: aqueue.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: aqueue.h,v 1.8 2000/11/10 18:50:41 leei Exp $
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

#ifndef _pthr_aqueue_h_
#define _pthr_aqueue_h_

/* An array-based queue of pointers */
typedef struct aqueue aqueue;

/* Create an aqueue of with len elements.  In an environment with POSIX
   threads, the shared argument can be used to request that the queue be
   shared among multiple threads.  Without POSIX threads, then a setting
   shared will cause aqueue_create() to return NULL. */

aqueue * aqueue_create (int len, int shared);

/* Destroy an aqueue as created by aqueue_create().  If this is a shared
   queue, then this is extremely dangerous unless all other threads accessing
   the queue have been terminated. */

void     aqueue_destroy (aqueue *aq);

/* Check whether the aqueue is empty or full. */
int aqueue_empty (aqueue *aq);
int aqueue_full (aqueue *aq);

/* Add a pointer to the end of the queue.  Auto-expands if needed. */
void aqueue_enqueue (aqueue *aq, void *p);

/* Remove and return the first element in the queue.  If this is a shared
   queue, then the call will block until such an element is available.  If not
   shared, then return NULL.  The caller should thus check for empty before
   calling in a non-shared environment. */
void * aqueue_dequeue (aqueue *aq);

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_aqueue_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "aqueue_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _pthr_aqueue_h_ */
