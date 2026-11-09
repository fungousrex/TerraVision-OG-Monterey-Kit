/*
 * Name: istack_inline.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: istack_inline.h,v 1.5 2000/08/29 00:31:30 reddy Exp $
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

#ifndef __GNUC__
exinline void *
istack_finish (istack *is)
{
  void *p = is->base;

  /* Check for an empty object. */
  if (is->base == is->next_free)
    is->empty_object = 1;

  /* Align the start of the next object. */
  is->next_free = istack_align_ptr (is->next_free, is->alignment_mask);
  is->base = is->next_free;

  return p;
}
#endif

extern void *_istack_unwind (istack *is, void *p);

exinline void *
istack_unwind (istack *is, void *p)
{
  if (p && p >= (void*) is->region->start && p < (void*) is->next_free) {
    is->base = is->next_free = (char *)p;
    return p;
  } else {
    return _istack_unwind (is, p);
  }
}

exinline void
istack_alloc_notify (istack *is, istack_notifier notify, void *closure)
{
  is->alloc_notify = notify;
  is->alloc_notify_closure = closure;
}
