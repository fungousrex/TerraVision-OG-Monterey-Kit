/*
 * Name: list.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: list.c,v 1.29 2000/12/01 08:20:51 reddy Exp $
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

#include <prlock.h>
#include <prcvar.h>

#include "list.h"
#include "listI.h"

#include <stdlib.h>
#include <string.h>

#include <stddef.h>
#include <assert.h>

#undef assert
#define assert(x)

typedef struct {
  list_t l;
  PRLock    *lock;
  PRCondVar *not_empty;
} shared_list;

static inline int
l_lock (list_t *l)
{
  PR_Lock (((shared_list*) l)->lock);
  return 0;
}

static inline int
l_unlock (list_t *l)
{
  PR_Unlock (((shared_list*) l)->lock);
  return 0;
}

static int
l_notempty (list_t *l)
{
  PR_NotifyCondVar (((shared_list*) l)->not_empty);
  l_unlock (l);
  return 0;
}

static int
l_wait (list_t *l)
{
  l_lock (l);
  while (list_empty (l)) {
    if (PR_WaitCondVar (((shared_list*) l)->not_empty,
			PR_INTERVAL_NO_TIMEOUT) != PR_SUCCESS) {
      break;
    }
  }
  return ! list_empty (l);
}

static list_shared_f locking_funcs[] = {
  l_lock, l_unlock, l_notempty, l_wait
};

#define elem_lock() ((! elems.lock ? (elems.lock = PR_NewLock()): 0), \
                     PR_Lock(elems.lock))
#define elem_unlock() (PR_Unlock(elems.lock))

#define NELEMS (512)

static struct {
  PRLock *lock;
  int nlists;
  list_iter_t free, curr, end;
  list_iter_t *blocks, *blocks_end, *curr_block;
} elems;

static void
elems_make_curr_valid (void)
{
  if (! elems.curr) {
    /* Ensure that we initialize the list of blocks. */
    elems.blocks = (list_iter_t *) calloc (32, sizeof (*elems.blocks));
    elems.blocks_end = elems.blocks + 32;
    elems.curr_block = elems.blocks;

  } else if (++elems.curr_block >= elems.blocks_end) {
    /* If next block is beyond block list, then realloc block list. */
    int n = elems.blocks_end - elems.blocks;
    elems.blocks = (list_iter_t *)
      realloc (elems.blocks, 2*n*sizeof (*elems.blocks));
    elems.blocks_end = elems.blocks + 2*n;
    memset (elems.blocks+n, 0, n * sizeof (elems.blocks[0]));
    elems.curr_block = elems.blocks + n;
  }

  elems.curr = *elems.curr_block =
    (list_iter_t) malloc (NELEMS * sizeof(*elems.curr));
  elems.end = elems.curr + NELEMS;
}

static inline list_iter_t 
elem_new (list_t *l, void const *val)
{
  list_iter_t el;

  elem_lock ();

  /* Check for shortcut.  Just return first free list elem. */
  if ((el = elems.free)) {
    elems.free = el->next;
  } else {
    /* If curr is valid, then return it immediately. */
    if (elems.curr == elems.end)
      elems_make_curr_valid ();
    el = elems.curr++;
  }

  elem_unlock ();
  
  el->l = l;
  el->val = val;

  return el;
}

static inline void
elem_free (list_iter_t e)
{
  elem_lock (); {
    e->next = elems.free;
    e->prev = &e->next;
    elems.free = e;
  }; elem_unlock ();
}

static inline void
elem_check (list_iter_t e)
{
  assert (*e->prev == e);
  assert (e->next->prev == &e->next);
}

list_t *
list_create (int shared)
{
  list_t *l;
  
  if (shared) {
    shared_list *ls = (shared_list *) malloc (sizeof (shared_list));
    if (! ls) return 0;
    ls->lock = PR_NewLock ();
    ls->not_empty = PR_NewCondVar (ls->lock);
    l = &ls->l;
    l->shared = locking_funcs;
  } else {
    l = (list_t *) malloc (sizeof (*l));
    if (! l) return 0;
    l->shared = 0;
  }

  l->top.next = &l->top;
  l->top.prev = &l->top.next;

  return l;
}

static inline void
list_detach (list_iter_t el)
{
  *el->prev = el->next;
  el->next->prev = el->prev;
}

int
list_check (list_t *l)
{
  list_iter_t el;
  int valid = 1;

  if (list_shared (l)) list_lock (l);
  
  el = l->top.next;
  if (list_iter_valid (el)) {
    list_iter_t el2 = &l->top;
    int sgn = 0;
    for ( ; el != &l->top; el = el->next) {
      if (! el || ! list_iter_valid (el) || el == el2) {
	valid = 0;
	break;
      }
      el2 = ((sgn = !sgn) ? el2->next : el2);
    }
  }

  if (list_shared (l)) list_unlock (l);

  return valid;
}

void
list_destroy (list_t *l)
{
  list_iter_t el;
  if (list_shared (l)) {
    shared_list *ls = (shared_list *) l;
    PR_DestroyCondVar (ls->not_empty);
    PR_DestroyLock (ls->lock);
  }
  for (el = l->top.next; el != &l->top; ) {
    list_iter_t next = el->next;
    elem_free (el);
    el = next;
  }

  free (l);
}

static inline list_iter_t
list_head (list_t *l)
{
  list_iter_t el = l->top.next;
  return el != &l->top ? el : NULL;
}

static inline list_iter_t
list_tail (list_t *l)
{
#if 0
  /* This may be more "portable" but is unnecessary. */
  char *elp = ((char *) l->top.prev) - offsetof (struct list_elem, next);
#else
  char *elp = ((char *) l->top.prev);
#endif
  list_iter_t el = (list_iter_t) elp;
  return el != &l->top ? el : NULL;
}

static inline void
list_insert_elem_after (list_iter_t el, list_iter_t after)
{
  el->prev = &after->next;
  el->next = after->next;
  after->next = el;
  el->next->prev = &el->next;

  elem_check (after);
  elem_check (el);
}

static inline void
list_insert_elem_before (list_iter_t el, list_iter_t before)
{
  el->prev = before->prev;
  el->next = before;
  before->prev = &el->next;
  *el->prev = el;

  elem_check (el);
  elem_check (before);
}

list_iter_t
list_insert_after (list_iter_t iter, void const *p)
{
  list_t *l = iter->l;
  list_iter_t newel = elem_new (l, p);

  if (list_shared (l)) list_lock (l);
  list_insert_elem_after (newel, iter);
  if (list_shared (l)) list_notempty (l);

  return newel;
}

list_iter_t
list_insert_before (list_iter_t iter, void const *p)
{
  list_t *l = iter->l;
  list_iter_t newel = elem_new (l, p);

  if (list_shared (l)) list_lock (l);
  list_insert_elem_before (newel, iter);
  if (list_shared (l)) list_notempty (l);

  return newel;
}

list_iter_t
list_push (list_t *l, void const *p)
{
  list_iter_t el = elem_new (l, p);

  if (list_shared (l)) list_lock (l);
  list_insert_elem_before (el, &l->top);
  if (list_shared (l)) list_notempty (l);

  return el;
}

void *
list_pop (list_t *l)
{
  void const *val = 0;
  list_iter_t el;

  if (list_shared (l) && ! list_wait (l)) {
    l_unlock (l);
    return NULL;
  }

  if ((el = list_tail (l))) {
    list_detach (el);
    val = el->val;
  }

  if (list_shared (l)) list_unlock (l);

  if (el)
    elem_free (el);

  return (void *) val;
}

list_iter_t
list_shift (list_t *l, void const *p)
{
  list_iter_t el = elem_new (l, p);

  if (list_shared (l)) list_lock (l);
  list_insert_elem_after (el, &l->top);
  if (list_shared (l)) list_notempty (l);

  return el;
}

void *
list_unshift (list_t *l)
{
  void const *val = 0;
  list_iter_t el;
 
  if (list_shared (l) && ! list_wait (l)) {
    l_unlock (l);
    return NULL;
  }

  if ((el = list_head (l))) {
    list_detach (el);
    val = el->val;
  }

  if (list_shared (l)) list_unlock (l);

  if (el) 
    elem_free (el);

  return (void *) val;
}

list_iter_t
list_delete (list_iter_t iter)
{
  list_t *l = iter->l;
  list_iter_t el;
  list_iter_t next;

  if (list_shared (l)) list_lock (l);

  el = iter;
  next = iter->next;
  if (list_iter_valid (el) && el != &l->top) {
    list_detach (el);
    elem_free (el);
  } else {
    next = 0;
  }
  
  if ( next == &l->top ) next = 0;  /* last element deleted */

  if (list_shared (l)) list_unlock (l);

  return next;
}
