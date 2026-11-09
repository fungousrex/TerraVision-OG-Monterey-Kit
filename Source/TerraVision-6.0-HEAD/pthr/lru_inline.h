/*
 * Name: lru_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: lru_inline.h,v 1.3 2000/09/29 22:50:31 leei Exp $
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

/* This is never included prior to lru.h. */

#include "lruI.h"

exinline lru_handle
lru_first (lru_cache *lru)
{
  return lru->top.next;
}

exinline lru_handle
lru_last (lru_cache *lru)
{
  /* h->prev is a pointer to a h->next slot. */
  return (lru_handle) lru->top.prev;
}

exinline void
lru_detach (lru_handle h)
{
  *h->prev = h->next;
  h->next->prev = h->prev;
}

exinline lru_handle
lru_decache_lru (lru_cache *lru, lru_handle h)
{
  if (lru->decache_f)
    (*lru->decache_f) (h->p, lru->decache_closure);

  lru_detach (h);

  return h;
}

exinline void
lru_insert_after (lru_handle h, lru_handle after)
{
  h->prev = &after->next;
  h->next = after->next;
  after->next = h;
  h->next->prev = &h->next;
}

exinline void
lru_insert_before (lru_handle h, lru_handle before)
{
  h->prev = before->prev;
  h->next = before;
  before->prev = &h->next;
  *h->prev = h;
}

exinline void *
lru_handle_value (lru_handle h)
{
  return h->p;
}




