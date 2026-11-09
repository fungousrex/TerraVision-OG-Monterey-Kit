/*
 * Name: istack.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: istack.h,v 1.9 2000/11/10 18:50:42 leei Exp $
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

#ifndef _istack_h_
#define _istack_h_

typedef struct istack istack;

istack *istack_create (int size);
void istack_destroy (istack*);

void * istack_alloc (istack *, int size);

void * istack_copy (istack *, void *p, int size);
void * istack_copy0 (istack *, void *p, int size);

void * istack_unwind (istack*, void *p);
void * istack_clear (istack*);

void * istack_blank (istack *, int size);

void istack_grow (istack*, void *p, int size);
void istack_grow0 (istack*, void *p, int size);

void istack_1grow (istack *, int ch);
void istack_ptr_grow (istack *, void *p);
void istack_int_grow (istack *, int v);

void * istack_finish (istack *);

int istack_object_size (istack *);

int istack_room (istack *);
void istack_make_room (istack *, int size);
void istack_1grow_fast (istack *, int data_char);
void istack_ptr_grow_fast (istack *, void *data);
void istack_int_grow_fast (istack *, int data);
void * istack_blank_fast (istack *, int size);

void * istack_base (istack *);
void * istack_next_free (istack *);

int istack_alignment (istack *, int);
int istack_region_size (istack *);
int istack_memory_used (istack *);

#define istack_new(is,T)			\
  (istack_alloc(is, sizeof(T)))

#define istack_new_vector(is,T,n)		\
  (istack_alloc(is, (n)*sizeof(T)))

#define istack_new_blank(is,T)			\
  (istack_blank (is, sizeof(T)))

#define istack_new_vector_blank(is,T,n)		\
  (istack_alloc (is,(n)*sizeof(T)))

/* Stuff below here is really part of the implementation */
#include <stdlib.h>

typedef struct istack_region {
  struct istack_region *prev;
  char *end;
  char start[4];
} istack_region;

typedef void (*istack_notifier) (istack *, int, void *);

extern void istack_alloc_notify (istack *is,
				 istack_notifier notify, void *closure);

struct istack {
  unsigned long alignment_mask;
  unsigned int region_size:30;
  unsigned int empty_object:1;

  istack_notifier alloc_notify;
  void *alloc_notify_closure;

  istack_region *region;
  char *base;
  char *next_free;
};

extern void _istack_expand (istack *is, int size);

#define istack_room(is)				\
  ((is)->region->end - (is)->next_free)

#define istack_make_room(is,size)		\
  ((void)					\
   (istack_room (is) < (size)			\
    ? (_istack_expand (is, size), 0)		\
    : 0))

#ifdef __GNUC__
#define istack_blank_fast(is, size)		\
  ({						\
     void *p = is->next_free;			\
     is->next_free += (size);			\
     p;						\
  })
#else
#define istack_blank_fast(is, size)		\
  (is->next_free += (size),			\
   (void*) (is->next_free - (size)))
#endif

#define istack_blank(is,size)			\
  (istack_make_room (is, size),			\
   istack_blank_fast (is, size))

#define istack_grow_fast(is, p, size)		\
  (memcpy (is->next_free, p, size),		\
   (void) (is->next_free += size))

#define istack_grow(is, p, size)		\
  (istack_make_room (is, size),			\
   istack_grow_fast (is, p, size))

#define istack_grow0(is, p, size)		\
  (istack_make_room (is, (size)+1),		\
   memcpy (is->next_free, p, size),		\
   is->next_free += (size),			\
   (void) (*is->next_free++ = '\0'))

#define istack_align_ptr(p, mask)			\
  ((char*) ((((unsigned long) p) + (mask)) & ~(mask)))

#ifdef __GNUC__
#define istack_finish(is)					\
  ({								\
    void *p = (is)->base;					\
								\
    if ((is)->base == (is)->next_free)				\
      (is)->empty_object = 1;					\
								\
    (is)->next_free = istack_align_ptr ((is)->next_free,	\
					(is)->alignment_mask);	\
    (is)->base = is->next_free;					\
    p;								\
  })
#endif

#define istack_alloc(is, size)			\
  (istack_blank (is, size),			\
   istack_finish (is))

#define istack_copy(is, p, size)		\
  (istack_grow (is, p, size),			\
   istack_finish (is))

#define istack_copy0(is, p, size)		\
  (istack_grow0 (is, p, size),			\
   istack_finish (is))

#define istack_1grow_fast(is, ch)		\
  (void) (*is->next_free++ = (ch))

#define istack_1grow(is, ch)			\
  (istack_make_room (is, 1),			\
   (void) istack_1grow_fast (is, (char) ch))

#define istack_ptr_grow_fast(is, p)		\
  (*(void **)is->next_free = (p),		\
   (void) (is->next_free += sizeof(p)))

#define istack_ptr_grow(is, p)			\
  (istack_make_room (is, sizeof(p)),		\
   (void) istack_ptr_grow_fast (is, p))

#define istack_int_grow_fast(is, v)		\
  (*(int *)is->next_free = (v),			\
   (void) (is->next_free += sizeof(int)))

#define istack_int_grow(is, v)			\
  (istack_make_room (is, sizeof(int)),		\
   (void) istack_int_grow_fast (is, (int) (v)))

#define istack_clear(is)			\
  (istack_unwind (is, 0))			\

#define istack_object_size(is)			\
  (is->next_free - is->base)

#define istack_base(is)				\
  ((void *) is->base)

#define istack_next_free(is)			\
  ((void *) is->next_free)

#define istack_region_size(is)			\
  (is->region_size)

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_istack_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "istack_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif /* _istack_h_ */
