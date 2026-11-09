/*
 * Name: list.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: list.h,v 1.21 2000/11/10 18:50:42 leei Exp $
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

#ifndef _pthr_list_h_
#define _pthr_list_h_

/* An array-based queue of pointers */
typedef struct list list_t;
typedef struct list_elem *list_iter_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Create a list.  In an environment with POSIX threads, the shared argument
   can be used to request that the queue be shared among multiple threads.
   Without POSIX threads, then a setting shared will cause list_create() to
   return NULL. */

list_t * list_create (int shared);

/* Destroy a list created by list_create().  If this is a shared list, then
   this is extremely dangerous unless all other threads accessing the queue
   have been terminated. */

void list_destroy (list_t *lst);

/* Check whether the list is empty. */
int list_empty (list_t *lst);

/* Add a pointer to the start of the list. */
list_iter_t list_shift (list_t *lst, void const *p);

/* Add a pointer to the end of the list. */
list_iter_t list_push (list_t *lst, void const *p);

/* Remove and return the first element in the list.  If this is a shared
   list, then the call will block until such an element is available.  If not
   shared, then return NULL.  The caller should thus check for empty before
   calling in a non-shared environment. */
void * list_unshift (list_t *lst);

/* Remove and return the last element in the list.  Blocks as above. */
void * list_pop (list_t *lst);

/* Number of elements in the list */
int list_count (list_t *lst);

/* Remove a particular pointer in a list, and return if it was found or not. */
int list_remove (list_t *l, void const *p);

/* Walk through the list, calling func on each element. The second argument to
   func is the closure argument. */
void list_forall (list_t *lst, 
		  void (*func) (void *val, void *closure),
		  void *closure);

/* These are used for walking through a list:
 *
 * for (i = list_first (l); ! list_done (i); i = list_next (l, i)) {
 *   void *v = list_value (&iter);
 * }
 *
 */

/* Obtain an iterator initialized to the first element of a list. */
list_iter_t list_first (list_t *l);

/* Obtain an iterator initialized to the last element of a list. */
list_iter_t list_last (list_t *l);

/* Increment the iterator to point to the next element in the list. */
list_iter_t list_next (list_iter_t iter);

/* Increment the iterator to point to the previous element in the list. */
list_iter_t list_prev (list_iter_t iter);

/* Test if iterator is at the end of list. */
int list_done (list_iter_t iter);

/* Check whether the iterator is usable. */
int list_iter_valid (list_iter_t iter);

/* Extract value from iterator. */
void *list_value (list_iter_t iter);

/* Delete the iterator element and point to the next element in the list. */
list_iter_t list_delete (list_iter_t iter);

/* Insert the pointer after the iterator and point to the new element. */
list_iter_t list_insert_after (list_iter_t iter, void const *p);

/* Insert the pointer before the iterator and point to the new element. */
list_iter_t list_insert_before (list_iter_t iter, void const *p);

/* Find an iterator to a particular pointer in a list */
list_iter_t list_find (list_t *l, void const *p);


#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_list_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "list_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _pthr_list_h_ */
