/*
 * Name: queue_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: aqueue_inline.h,v 1.5 2000/09/29 22:50:31 leei Exp $
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

/* This is never included prior to aqueue.h. */

#include "aqueueI.h"

exinline int
aqueue_empty (aqueue *aq)
{
  return aq->head == aq->tail;
}

exinline int
aqueue_full (aqueue *aq)
{
  return (aq->tail+1)%aq->len == aq->head;
}

exinline void
aqueue_enqueue (aqueue *aq, void *p)
{
  if (aqueue_shared (aq))
    aqueue_lock (aq);

  if (aqueue_full (aq) && ! aqueue_expand (aq))
    return;

  aq->contents[aq->tail++] = p;
  if (aq->tail == aq->len)
    aq->tail = 0;

  if (aqueue_shared (aq))
    aqueue_notempty (aq);
}

exinline void *
aqueue_dequeue (aqueue *aq)
{
  void *p = 0;

  if (aqueue_shared (aq) && ! aqueue_wait (aq)) {
    aqueue_unlock (aq);
    return 0;
  }

  if (! aqueue_empty (aq)) {
    p = aq->contents[aq->head++];
    if (aq->head == aq->len)
      aq->head = 0;
  }

  if (aqueue_shared (aq))
    aqueue_unlock (aq);

  return p;
}
