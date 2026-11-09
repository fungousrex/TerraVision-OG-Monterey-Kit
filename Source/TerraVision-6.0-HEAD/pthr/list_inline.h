/*
 * Name: list_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: list_inline.h,v 1.18.4.1 2001/07/12 20:52:14 leei Exp $
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

/* This is never included prior to list.h. */

#include "listI.h"

exinline int
list_iter_valid (list_iter_t e)
{
  return e && e->prev != &e->next;
}

exinline int
list_empty (list_t *l)
{
  return ! l || ! list_iter_valid (&l->top);
}

exinline void
list_forall (list_t *l, void (*func) (void *, void *), void *closure)
{
  list_iter_t el;

  if (! l) return;

  if (list_shared (l)) list_lock (l);

  for (el = l->top.next; el; el = el->next)
    (*func) ((void *) el->val, closure);

  if (list_shared (l)) list_unlock (l);
}

exinline int
list_count (list_t *l)
{
  list_iter_t el;
  int i = 0;

  if (! l) return 0;

  if (list_shared (l)) list_lock (l);
  
  for (el = l->top.next; el != &l->top; el = el->next)
    ++i;

  if (list_shared (l)) list_unlock (l);
  
  return i;
}
  
exinline list_iter_t
list_first (list_t *l)
{
  list_iter_t iter;
  list_iter_t el;

  if (! l) return 0;

  if (list_shared (l)) list_lock (l);

  el = l->top.next;
  iter = (el == &l->top) ? 0 : el;

  if (list_shared (l)) list_unlock (l);

  return iter;
}

exinline list_iter_t
list_last (list_t *l)
{
  list_iter_t iter;
  list_iter_t el;

  if (! l) return 0;

  if (list_shared (l)) list_lock (l);

  el = (list_iter_t) l->top.prev;
  iter = (el == &l->top) ? 0 : el;

  if (list_shared (l)) list_unlock (l);

  return iter;
}

exinline list_iter_t
list_next (list_iter_t iter)
{
  list_t *l = iter->l;
  if (list_shared (l)) list_lock (l);

  if (list_iter_valid (iter)) {
    iter = iter->next;
    if (iter == &l->top)
      iter = 0;
  }

  if (list_shared (l)) list_unlock (l);

  return iter;
}

exinline list_iter_t
list_prev (list_iter_t iter)
{
  list_t *l = iter->l;
  if (list_shared (l)) list_lock (l);

  if (list_iter_valid (iter)) {
    iter = (list_iter_t) iter->prev;
    if (iter == &l->top)
      iter = 0;
  }

  if (list_shared (l)) list_unlock (l);

  return iter;
}

exinline int
list_done (list_iter_t iter)
{
  return ! list_iter_valid (iter);
}

exinline void *
list_value (list_iter_t iter)
{
  return iter ? (void *) iter->val : 0;
}


exinline list_iter_t 
list_find (list_t *l, void const *p)
{
  list_iter_t i;

  for (i = list_first (l); ! list_done (i); i = list_next (i))
    if (list_value (i) == p) 
      return (i);
  i = 0;
  return (i);
}
 

exinline int 
list_remove (list_t *l, void const *p)
{
  list_iter_t i;
  int ret = 0;

  /* find it first */
  i = list_find (l, p);

  /* now remove it */
  if (i) {
    list_delete (i);
    ret = 1;
  }

  return ret;
}
 
