/*
 * Name: listI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: listI.h,v 1.1 2000/09/29 22:50:31 leei Exp $
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

#ifndef _listI_h_
#define _listI_h_

struct list_elem {
  list_iter_t next, *prev;
  list_t *l;
  void const *val;
};

typedef int (*list_shared_f) (list_t *list);

struct list {
  struct list_elem top;
  list_shared_f *shared;
};

#define list_shared(l)   ((l)->shared)
#define list_lock(l)     ((*(l)->shared[0])(l))
#define list_unlock(l)   ((*(l)->shared[1])(l))
#define list_notempty(l) ((*(l)->shared[2])(l))
#define list_wait(l)     ((*(l)->shared[3])(l))

#endif /* _listI_h_ */
