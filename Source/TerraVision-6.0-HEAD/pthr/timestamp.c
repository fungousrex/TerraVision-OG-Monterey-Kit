/*
 * Name: timestamp.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: timestamp.c,v 1.10 2000/08/29 00:31:37 reddy Exp $
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

#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#define DEBUG (1)

#ifdef DEBUG
#include "report.h"

#include <prerror.h>
#include <prcvar.h>

#define DBGP(l) (debug_level >= (l))
#define DBG(l,x) (DBGP (l) ? (pthr_trace x, 1) : (0))

static int debug_level = 0;

int
__timestamp_debug (int level)
{
  int old_level = debug_level;
  debug_level = level;
  return old_level;
}
#else /* !DEBUG */
#define DBGP(l) (0)
#define DBG(l,x) (0)

#endif

#ifdef TRACING
#include <prlock.h>

void
pthr_PR_Lock (const char *funcName, PRLock *lock)
{
  DBG(2,("%s: PR_Lock %p wait", funcName, lock));
  PR_Lock (lock);
  DBG(2,("%s: PR_Lock %p held", funcName, lock));
}

PRStatus
pthr_PR_Unlock (const char *funcName, PRLock *lock)
{
  PRStatus ret = PR_Unlock (lock);
  DBG(2,("%s: PR_Unlock %p released %d", funcName, lock, ret));
  return ret;
}

PRStatus
pthr_PR_WaitCondVar (const char *funcName, 
		     PRCondVar *cvar, PRIntervalTime timeout)
{
  PRStatus ret; 
  DBG(2,("%s: PR_WaitCondVar %p", funcName, cvar));
  ret = PR_WaitCondVar (cvar, timeout);
  DBG(2,("%s: PR_WaitCondVar %p signalled %d", funcName, cvar, ret));
  return ret;
}
#endif

#include "timestamp.h"

#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

typedef struct _list {
  struct _list *next;
  void *val;
} *list_t;

typedef void (*list_f)(void *arg, void *closure);

#define LOCK(mutex) ((mutex) ? PR_Lock (mutex): (void)0)
#define UNLOCK(mutex) ((mutex) ? PR_Unlock (mutex): 0)

static list_t
list_add (list_t *list, PRLock *mutex, void *val)
{
  static const char funcName[] = "list_add";
  list_t node = (list_t) malloc (sizeof (*node));
  DBG(2,("list_add: %p(%s) %s mutex %s", list, pthr_name(*list), 
	 pthr_name(val), pthr_name(mutex)));
  node->val = val;
  LOCK (mutex); {
    node->next = *list;
    *list = node;
  }; UNLOCK (mutex);

  DBG(2,("list_add: %p(%s)", list, pthr_name(*list)));
  return node;
}

static list_t
list_find (list_t *list, PRLock *mutex, void *val)
{
  static const char funcName[] = "list_find";
  list_t ret = 0, node;
  DBG(2,("list_find: %p(%s) %s mutex %s", list, pthr_name(*list),
	 pthr_name(val), pthr_name(mutex)));
  LOCK (mutex); {
    for (node = *list; node; node = node->next) {
      if (node->val == val) {
	ret = node;
	break;
      }
    }
  }; UNLOCK (mutex);

  DBG(2,("list_find: %p(%s) = %p", list, pthr_name(*list), ret));
  return ret;
}

static list_t
list_forall (list_t *list, PRLock *mutex, list_f func, void *closure)
{
  static const char funcName[] = "list_forall";
  list_t node;
  DBG(2,("list_forall: %p(%s) mutex %s", list, pthr_name(*list)));
  LOCK (mutex); {
    for (node = *list; node; node = *list) {
      UNLOCK (mutex);
      DBG(2,("  list_forall: call %p(%s)", func, pthr_name(node->val)));
      (*func)(node->val, closure);
      LOCK (mutex);
      list = &node->next;
    }
  }; UNLOCK (mutex);

  DBG(2,("list_forall: done."));
  return node;
}

static list_t
list_remove (list_t *list, PRLock *mutex, void *val)
{
  static const char funcName[] = "list_remove";
  list_t node;
  DBG(2,("list_remove: %p(%s) %s mutex %s", list, pthr_name(*list), 
	 pthr_name(val), pthr_name(mutex)));
  LOCK (mutex); {
    for (node = *list; node; node = *list) {
      if (node->val == val) {
	*list = node->next;
	DBG(2,("list_remove: %p(%s) found %s node %p", list, pthr_name(*list), 
	       pthr_name(val), node));
	free (node);
	break;
      }
      list = &node->next;
    }
  }; UNLOCK (mutex);

  DBG(2,("list_remove: %p(%s) = %p", list, pthr_name(*list), node));
  return node;
}

static list_t
list_destroy (list_t *list, PRLock *mutex, list_f detachf, void *closure)
{
  static const char funcName[] = "list_destroy";
  list_t node, next;

  DBG(2,("list_destroy: %p(%s) mutex %s", list, pthr_name(*list),
	 pthr_name(mutex)));

  LOCK (mutex); {
    node = *list;
    *list = 0;
  }; UNLOCK (mutex);

  /* We've detached it, so we can do the deallocation unencumbered. */
  for (; node; node = next) {
    if (detachf) (*detachf)(node->val, closure);
    next = node->next;
    free (node);
  }

  DBG(2,("list_destroy: done = %p", node));
  return node;
}

static unsigned long ts_next = 1;
static PRLock *ts_mutex = NULL;

static list_t all_timestamps;

struct _pthr_timestamp {
  unsigned long stamp;
  PRLock *mutex;
  list_t conds;
};

char const *cond_types[] = {
  "newer", "generic", "conjunction", "disjunction"
};

struct _pthr_cond {
  enum { TS_NEWER, TS_GENERIC, TS_CONJUNCTION, TS_DISJUNCTION } type;
  union {
    struct {
      int (*func) (void const *);
      void const *closure;
    } generic;
    struct {
      pthr_timestamp_t *older;
      pthr_timestamp_t *newer;
    } newer;
    struct {
      int n;
      pthr_cond_t **conds;
    } conj;
    struct {
      int n;
      pthr_cond_t **conds;
    } disj;
  } u;

  int valid;
  PRLock    *mutex;
  PRLock    *cond_mutex;
  PRCondVar *cond;
  list_t parents;
};

static int
ts_compare (void const *v1, void const *v2)
{
  pthr_timestamp_t const *t1 = *(pthr_timestamp_t const **)v1;
  pthr_timestamp_t const *t2 = *(pthr_timestamp_t const **)v2;
  unsigned long s1 = t1->stamp, s2 = t2->stamp;

  return (s1 < s2 ? -1 :
	  s1 == s2 ? 0 :
	  1);
}

static unsigned long
next_timestamp (void)
{
  static const char funcName[] = "next_timestamp";
  unsigned long next;
  PR_Lock (ts_mutex); {
    if ((next = ts_next++) == 0) {
      /* Rolled over, reset all timestamps, retaining sequence. */
      int n = 0;
      void **array, **ap;
      list_t ts = all_timestamps;
      for (; ts; ts = ts->next) ++n;
      array = (void **) alloca (n * sizeof (void*));
      for (ts = all_timestamps, ap = array; ts; ++ap, ts = ts->next)
	*ap = ts;
      qsort (array, n, sizeof (void*), ts_compare);
      next = n+1;
      for (ap = array, n = 1; n <= next; ++ap, ++n)
	((pthr_timestamp_t *) *ap)->stamp = n;
      next = n;
    }
  }; PR_Unlock (ts_mutex);
  return next;
}

pthr_timestamp_t *
pthr_timestamp_create (void)
{
  pthr_timestamp_t *ts = (pthr_timestamp_t *) calloc (1, sizeof (*ts));

  /* Is this the most appropriate place for initializing this lock? --RLB */
  if (!ts_mutex) ts_mutex = PR_NewLock ();

  ts->stamp = next_timestamp ();
  ts->conds = 0;
  ts->mutex = PR_NewLock();

  DBG(1,("pthr_timestamp_create: %s mutex %p", pthr_name(ts), &ts->mutex));

  list_add (&all_timestamps, ts_mutex, ts);

  return ts;
}

static void cond_invalidate (pthr_cond_t *cond);

void
pthr_timestamp_destroy (pthr_timestamp_t *ts)
{
  list_destroy (&ts->conds, ts->mutex, (list_f) cond_invalidate, 0);
  PR_DestroyLock(ts->mutex);
  
  DBG(1,("pthr_timestamp_destroy: %s mutex %p", pthr_name(ts), &ts->mutex));

  list_remove (&all_timestamps, ts_mutex, ts);

  free (ts);
}

void
pthr_timestamp_update (pthr_timestamp_t *ts, PRLock *holding)
{
  ts->stamp = next_timestamp ();
  DBG(1,("pthr_timestamp_update: %s <- %d", pthr_name(ts), ts->stamp));
  list_forall (&ts->conds, ts->mutex, (list_f) pthr_cond_broadcast, holding);
}

static void
timestamp_add_cond (pthr_timestamp_t *ts, pthr_cond_t *cond)
{
  if (! list_find (&ts->conds, ts->mutex, cond)) {
    DBG(2,("pthr_timestamp_add_cond: %s cond %s",
	   pthr_name(ts), pthr_name(cond)));
    list_add (&ts->conds, ts->mutex, cond);
  }
}

static void
cond_invalidate (pthr_cond_t *cond)
{
  static const char funcName[] = "cond_invalidate";

  PR_Lock (cond->mutex); {
    DBG(2,("pthr_cond_invalidate: cond %s", pthr_name(cond)));
    cond->valid = 0;
  }; PR_Unlock (cond->mutex);

  list_forall (&cond->parents, cond->mutex, (list_f) cond_invalidate, 0);
}

void
pthr_cond_broadcast (pthr_cond_t *cond, PRLock *holding)
{
  static const char funcName[] = "pthr_cond_broadcast";
  if (cond->type == TS_GENERIC
      || cond->type == TS_NEWER
      || cond->type == TS_DISJUNCTION) {
    DBG(1,("pthr_cond_broadcast: cond %s[%s]", 
	   pthr_name(cond), cond_types[cond->type]));

    if (holding != cond->mutex) PR_Lock (cond->mutex);

    PR_NotifyAllCondVar (cond->cond);
    DBG(2,("pthr_cond_broadcast: cond %s notified", pthr_name(cond->cond)));

    if (holding != cond->mutex) PR_Unlock (cond->mutex);

    list_forall (&cond->parents, cond->mutex,
		 (list_f) pthr_cond_broadcast, holding);
  }
}

int
pthr_timestamp_newer (pthr_timestamp_t *newer, pthr_timestamp_t *older)
{
  DBG(1,("pthr_timestamp_newer: %s %d > %s %d? %s",
	 pthr_name(newer), newer->stamp,
	 pthr_name(older), older->stamp,
	 (newer->stamp > older->stamp ? "YES" : "NO")));
  return newer->stamp > older->stamp;
}

int
pthr_cond_test (pthr_cond_t *cond)
{
  switch (cond->type) {
  case TS_GENERIC:
    DBG(1,("pthr_cond_test: cond %s[%s]: %p(%p)? %s",
	   pthr_name(cond), cond_types[cond->type],
	   cond->u.generic.func, cond->u.generic.closure, 
	   ((*cond->u.generic.func)(cond->u.generic.closure)
	    ? "YES" : "NO")));
    return (*cond->u.generic.func)(cond->u.generic.closure);

  case TS_NEWER:
    DBG(1,("pthr_cond_test: cond %s[%s]: %s %d > %s %d? %s",
	   pthr_name(cond), cond_types[cond->type],
	   pthr_name(cond->u.newer.newer), cond->u.newer.newer->stamp,
	   pthr_name(cond->u.newer.older), cond->u.newer.older->stamp,
	   (cond->u.newer.newer->stamp > cond->u.newer.older->stamp
	    ? "YES" : "NO")));
    return cond->u.newer.newer->stamp > cond->u.newer.older->stamp;

  case TS_CONJUNCTION: {
    int n = cond->u.conj.n;
    pthr_cond_t **conds = cond->u.conj.conds, **conds_end = conds+n;
    if (DBGP(1)) pthr_trace_indent (1, 2);
    for ( ; conds < conds_end; ++conds)
      if (! pthr_cond_test (*conds)) {
	if (DBGP(1)) pthr_trace_indent (1, -2);
	DBG(1,("pthr_cond_test: cond %s[%s] conj[%d]? NO",
	       pthr_name(cond), cond_types[cond->type], n));
	return 0;
      }
    if (DBGP(1)) pthr_trace_indent (1, -2);
    DBG(1,("pthr_cond_test: cond %s[%s] conj[%d]? YES",
	   pthr_name(cond), cond_types[cond->type], n));
    return 1;
  }

  case TS_DISJUNCTION: {
    int n = cond->u.conj.n;
    pthr_cond_t **conds = cond->u.disj.conds, **conds_end = conds+n;
    if (DBGP(1)) pthr_trace_indent (1, 2);
    for ( ; conds < conds_end; ++conds)
      if (pthr_cond_test (*conds)) {
	if (DBGP(1)) pthr_trace_indent (1, -2);
	DBG(1,("pthr_cond_test: cond %s[%s] disj[%d]? YES",
	       pthr_name(cond), cond_types[cond->type], n));
	return 1;
      }
    if (DBGP(1)) pthr_trace_indent (1, -2);
    DBG(1,("pthr_cond_test: cond %s[%s] disj[%d]? NO",
	   pthr_name(cond), cond_types[cond->type], n));
    return 0;
  }
  }

  return 0;
}

PRStatus
pthr_cond_wait (pthr_cond_t *cond, PRLock *mutex)
{
  static const char funcName[] = "pthr_cond_wait";
  PRStatus status;

  /* Simply return with an invalidated condition. */
  if (! cond->valid) {
    DBG(1,("pthr_cond_wait: cond %s[%s] invalid.",
	   pthr_name(cond), cond_types[cond->type]));
    return PR_FAILURE;
  }

  switch (cond->type) {
  case TS_GENERIC:
  case TS_NEWER:
  case TS_DISJUNCTION:
    while (! pthr_cond_test (cond)) {
      DBG(1,("pthr_cond_wait: cond %s[%s]",
	     pthr_name(cond), cond_types[cond->type]));
      assert (mutex == cond->cond_mutex);
      DBG(1,("PR_WaitCondVar %p", cond->cond));
      status = PR_WaitCondVar (cond->cond, PR_INTERVAL_NO_TIMEOUT);
      if (status != PR_SUCCESS) {
	DBG(1,("pthr_cond_wait: return %d", status));
 	return status;
      }
    }
    break;

  case TS_CONJUNCTION: {
    int n = cond->u.conj.n;
    pthr_cond_t **conds = cond->u.conj.conds, **conds_end = conds+n;
    while (! pthr_cond_test (cond)) {
      pthr_cond_t **condp;
      DBG(1,("pthr_cond_wait: conj %s[%d]", pthr_name(cond), n));
      if (DBGP(1)) pthr_trace_indent (1, 2);
      for (condp = conds; condp < conds_end; ++condp) {
	status = pthr_cond_wait (*condp, mutex);
	if (status != PR_SUCCESS) {
	  DBG(1,("pthr_cond_wait: return %d", status));
	  if (DBGP(1)) pthr_trace_indent (1, -2);
 	  return status;
	}
      }
      if (DBGP(1)) pthr_trace_indent (1, -2);
    }
    break;
  }
  }

  DBG(1,("pthr_cond_wait: cond %s[%s]: done.",
	 pthr_name(cond), cond_types[cond->type]));
  return PR_SUCCESS;
}

static void
cond_init (pthr_cond_t *cond, PRLock *mutex)
{
  cond->valid = 1;
  cond->parents = 0;

  cond->mutex = PR_NewLock ();

  cond->cond_mutex = mutex;
}

pthr_cond_t *
pthr_cond (PRLock *mutex, int (*func) (void const *), void const *closure)
{
  pthr_cond_t *cond = (pthr_cond_t *) calloc (1, sizeof (*cond));

  cond_init (cond, mutex);

  cond->type = TS_GENERIC;
  cond->u.generic.func = func;
  cond->u.generic.closure = closure;

  cond->cond = PR_NewCondVar (mutex);

  DBG(1,("pthr_cond_generic: cond %s[%s]: %p(%p).", 
	 pthr_name(cond), cond_types[cond->type], func, closure));

  return cond;
}

pthr_cond_t *
pthr_cond_timestamp_newer (PRLock *mutex, 
			   pthr_timestamp_t *newer, pthr_timestamp_t *older)
{
  pthr_cond_t *cond = (pthr_cond_t *) calloc (1, sizeof (*cond));

  cond_init (cond, mutex);

  cond->type = TS_NEWER;
  cond->u.newer.newer = newer;
  cond->u.newer.older = older;

  cond->cond = PR_NewCondVar (mutex);

  timestamp_add_cond (newer, cond);

  DBG(1,("pthr_cond_newer: cond %s[%s]: %s > %s.", 
	 pthr_name(cond), cond_types[cond->type],
	 pthr_name(newer), pthr_name(older)));
  return cond;
}

pthr_cond_t *
pthr_cond_all (PRLock *mutex, pthr_cond_t *first, ...)
{
  va_list ap;
  int n = 1;

  pthr_cond_t *cond = (pthr_cond_t *) calloc (1, sizeof (*cond));
  pthr_cond_t **conds, *curr;

  va_start (ap, first);
  while (va_arg (ap, pthr_cond_t *))
    ++n;
  va_end (ap);

  cond_init (cond, mutex);

  cond->type = TS_CONJUNCTION;
  cond->u.conj.n = n;
  cond->u.conj.conds = conds = (pthr_cond_t **) malloc (n * sizeof (*conds));
  DBG(1,("pthr_cond_all: cond %s[%d]", pthr_name(cond), n));

  /* Fill the array of conds. */
  va_start (ap, first);
  DBG(1,("pthr_cond_all: conj %s[%d] = %s",
	 pthr_name(cond), conds-cond->u.conj.conds, pthr_name(first)));
  *conds = first;
  list_add (&(*conds)->parents, mutex, cond);
  ++conds;
  while ((curr = va_arg (ap, pthr_cond_t *))) {
    *conds = curr;
    DBG(1,("pthr_cond_all: conj %s[%d] = %s",
	   pthr_name(cond), conds-cond->u.conj.conds, pthr_name(curr)));
    list_add (&(*conds)->parents, mutex, cond);
    ++conds;
  }
  va_end (ap);
  
  return cond;
}

static void
cond_register_descendants (pthr_cond_t *cond, pthr_cond_t *curr)
{
  pthr_cond_t **conds, **conds_end;
  switch (curr->type) {
  case TS_NEWER:
    timestamp_add_cond (curr->u.newer.newer, cond);
    break;

  case TS_CONJUNCTION:
  case TS_DISJUNCTION:
    conds = curr->u.conj.conds; conds_end = conds+curr->u.conj.n;
    for (; conds < conds_end; ++conds)
      cond_register_descendants (cond, *conds);
    break;
  }
}

pthr_cond_t *
pthr_cond_any (PRLock *mutex, pthr_cond_t *first, ...)
{
  va_list ap;
  int n = 1;

  pthr_cond_t *cond = (pthr_cond_t *) calloc (1, sizeof (*cond));
  pthr_cond_t **conds, *curr;

  va_start (ap, first);
  while (va_arg (ap, pthr_cond_t *))
    ++n;
  va_end (ap);

  cond_init (cond, mutex);

  cond->type = TS_DISJUNCTION;
  cond->u.disj.n = n;
  cond->u.disj.conds = conds = (pthr_cond_t **)
    malloc (n * sizeof (*conds));
  DBG(1,("pthr_cond_any: cond %s[%d]", pthr_name(cond), n));

  /* Fill the array of conds. */
  va_start (ap, first);
  DBG(1,("pthr_cond_any: disj %s[%d] = %s",
	 pthr_name(cond), conds-cond->u.disj.conds, pthr_name(first)));
  *conds = first;
  ++conds;
  while ((curr = va_arg (ap, pthr_cond_t *))) {
    *conds = curr;
    DBG(1,("pthr_cond_any: disj %s[%d] = %s",
	   pthr_name(cond), conds-cond->u.disj.conds, pthr_name(curr)));
    list_add (&(*conds)->parents, mutex, cond); 
    ++conds;
  }
  va_end (ap);

  cond->cond = PR_NewCondVar (mutex);

  cond_register_descendants (cond, cond);

  return cond;
}

PRStatus
pthr_timestamp_wait (pthr_timestamp_t *ts)
{
  static const char funcName[] = "pthr_timestamp_wait";
  pthr_timestamp_t *now = pthr_timestamp_create ();
  pthr_cond_t *cond = pthr_cond_timestamp_newer (ts->mutex, ts, now);
  PRStatus status;

  PR_Lock (ts->mutex); {
    status = pthr_cond_wait (cond, ts->mutex);
  }; PR_Unlock (ts->mutex);

  /*pthr_cond_destroy (cond);*/
  pthr_timestamp_destroy (now);

  return status;
}
