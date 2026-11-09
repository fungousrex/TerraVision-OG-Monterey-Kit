/*
 * Name: timestamp.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: timestamp.h,v 1.7 2000/08/29 00:31:37 reddy Exp $
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

#ifndef _pthr_timestamp_h_
#define _pthr_timestamp_h_

#include <prlock.h>

#if 0 && defined(TRACING)
#include <prcvar.h>

#undef PR_Lock
#undef PR_Unlock
#undef PR_WaitCondVar

#define PR_Lock(lock) pthr_PR_Lock(funcName,lock)
#define PR_Unlock(lock) pthr_PR_Unlock(funcName,lock)
#define PR_WaitCondVar(cond,interval) pthr_PR_WaitCondVar(funcName,cond,interval)

void pthr_PR_Lock (const char *funcName, PRLock *lock);
PRStatus pthr_PR_Unlock (const char *funcName, PRLock *lock);
PRStatus pthr_PR_WaitCondVar (const char *funcName, 
			      PRCondVar *cvar, PRIntervalTime timeout);
#endif

typedef struct _pthr_timestamp pthr_timestamp_t;
typedef struct _pthr_cond pthr_cond_t;

extern pthr_timestamp_t *pthr_timestamp_create (void);
extern void pthr_timestamp_destroy (pthr_timestamp_t *ts);
extern void pthr_timestamp_update (pthr_timestamp_t *ts, PRLock *mutex);
extern PRStatus pthr_timestamp_wait (pthr_timestamp_t *ts);

extern int pthr_timestamp_newer (pthr_timestamp_t *ts1, 
				 pthr_timestamp_t *ts2);

extern void pthr_cond_broadcast (pthr_cond_t *cond, PRLock *mutex);

extern int pthr_cond_test (pthr_cond_t *cond);

extern PRStatus pthr_cond_wait (pthr_cond_t *cond, PRLock *mutex);

extern pthr_cond_t *
pthr_cond (PRLock *lock, int (*func) (void const *), void const *closure);

extern pthr_cond_t *
pthr_cond_timestamp_newer (PRLock *lock,
			   pthr_timestamp_t *newer, pthr_timestamp_t *older);

extern pthr_cond_t *
pthr_cond_all (PRLock *lock, pthr_cond_t *first, ...);

extern pthr_cond_t *
pthr_cond_any (PRLock *lock, pthr_cond_t *first, ...);

extern int __timestamp_debug (int level);

#endif /* _pthr_timestamp_h_ */
