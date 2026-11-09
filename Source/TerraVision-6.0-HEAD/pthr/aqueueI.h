/*
 * Name: aqueueI.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: aqueueI.h,v 1.1 2000/09/29 22:50:31 leei Exp $
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

#ifndef _aqueueI_h_
#define _aqueueI_h_

typedef int (*aqueue_shared_f) (aqueue *aq);

struct aqueue {
  int head, tail, len;
  void **contents;
  aqueue_shared_f *shared;
};

#define aqueue_shared(aq)   ((aq)->shared)
#define aqueue_lock(aq)     ((*(aq)->shared[0])(aq))
#define aqueue_unlock(aq)   ((*(aq)->shared[1])(aq))
#define aqueue_notempty(aq) ((*(aq)->shared[2])(aq))
#define aqueue_wait(aq)     ((*(aq)->shared[3])(aq))

extern int aqueue_expand (aqueue *aq);

#endif /* _aqueueI_h_ */
