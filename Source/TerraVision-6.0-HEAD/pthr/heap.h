/*
 * Name: heap.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: heap.h,v 1.4.2.1 2001/06/07 21:29:20 reddy Exp $
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

#ifndef _pthr_heap_h_
#define _pthr_heap_h_

typedef void *heap_elem_t;
typedef struct heap heap_t;

typedef int (*heap_cmp_t) (heap_elem_t, heap_elem_t);
typedef void (*heap_update_t) (heap_elem_t, int);

heap_t * heap_create (unsigned int len, heap_cmp_t cmp, heap_update_t upd);
void heap_destroy (heap_t *h);

int heap_size (heap_t const *h);
int heap_is_empty (heap_t const *h);

heap_elem_t * heap_maybe_resize (heap_t *h, unsigned int nelems);

int heap_add_nocheck (heap_t *h, heap_elem_t elem);
int heap_add_noadjust (heap_t *h, heap_elem_t elem);
int heap_add (heap_t *h, heap_elem_t elem);

int heap_recost_idx (heap_t *h, heap_elem_t elem, int idx);
int heap_remove_idx (heap_t *h, heap_elem_t elem, int idx);

heap_elem_t heap_pop (heap_t *h);

void heap_check (heap_t *h);

void heap_forall (heap_t *h, void (*f)(void *v, void *c), void *closure);

#if 0 && (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_heap_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "heapil.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _pthr_heap_h_ */
