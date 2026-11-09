/*
 * Name: test_list.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: test_list.c,v 1.9 2000/08/29 00:31:36 reddy Exp $
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

#include "config.h"

#include <prthread.h>

#include "list.h"

static int refs[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 3, 7, 2, 1, 1, 0, 4};
#define n_refs (sizeof(refs)/sizeof(refs[0]))

int
test_list (void)
{
  int i;

  list_t *l = list_create (0);
  list_iter_t li;

  if (! list_empty (l)) {
    printf ("init: list not empty\n");
    return 0;
  }

  for (i = 0; i < n_refs; ++i) {
    int ref = refs[i];
    list_push (l, (void *)ref);
  }

  for (i = 0, li = list_first (l);
       i < n_refs && ! list_done (li);
       ++i, li = list_next (li)) {
    int ref = refs[i];
    int r = (int) list_value (li);
    if (ref != r) {
      printf ("iter: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }
  if (! list_done (li)) {
      printf ("iter: list not done\n");
      return 0;
  }

  for (i = 0; i < n_refs; ++i) {
    int r, ref = refs[i];
    r = (int) list_unshift (l);
    if (ref != r) {
      printf ("unshift: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }

  if (! list_empty (l)) {
    printf ("unshift: list not empty\n");
    return 0;
  }

  for (i = 0; i < n_refs; ++i) {
    int ref = refs[i];
    list_shift (l, (void *)ref);
  }

  for (i = n_refs-1, li = list_first (l);
       i >= 0 && ! list_done (li);
       --i, li = list_next (li)) {
    int ref = refs[i];
    int r = (int) list_value (li);
    if (ref != r) {
      printf ("iter: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }
  if (! list_done (li)) {
      printf ("iter: list not done\n");
      return 0;
  }

  for (i = 0; i < n_refs; ++i) {
    int r, ref = refs[i];
    r = (int) list_pop (l);
    if (ref != r) {
      printf ("pop: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }

  if (! list_empty (l)) {
    printf ("pop: list not empty\n");
    return 0;
  }

  return 1;
}

static void
writer_thread1 (void *g_l)
{
  int i;
  list_t *l = g_l;

  for (i = 0; i < n_refs; ++i) {
    int ref = refs[i];
    list_push (l, (void *)ref);
  }
}

static void
writer_thread2 (void *g_l)
{
  int i;
  list_t *l = g_l;

  for (i = 0; i < n_refs; ++i) {
    int ref = refs[i];
    list_shift (l, (void *)ref);
  }
}

int
test_list_threads (void)
{
  int i;
  list_t *l = list_create (1);

  PRThread *writer;

  if ((writer = PR_CreateThread (PR_USER_THREAD, writer_thread1, l,
				 PR_PRIORITY_NORMAL, PR_LOCAL_THREAD,
				 PR_JOINABLE_THREAD, 0)) == NULL) {
    printf ("Unable to create writer_thread1.\n");
    return 0;
  }
  
  for (i = 0; i < n_refs; ++i) {
    int r, ref = refs[i];
    r = (int) list_unshift (l);
    if (ref != r) {
      printf ("shared unshift: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }

  if (PR_JoinThread (writer) != PR_SUCCESS)
    return 0;

  if ((writer = PR_CreateThread (PR_USER_THREAD, writer_thread2, l,
				 PR_PRIORITY_NORMAL, PR_LOCAL_THREAD,
				 PR_JOINABLE_THREAD, 0)) == NULL) {
    printf ("Unable to create writer_thread2.\n");
    return 0;
  }
  
  for (i = 0; i < n_refs; ++i) {
    int r, ref = refs[i];
    r = (int) list_pop (l);
    if (ref != r) {
      printf ("shared pop: refs[%d]: %d != %d\n", i, ref, r);
      return 0;
    }
  }

  if (PR_JoinThread (writer) != PR_SUCCESS)
    return 0;

  return 1;
}

int
main (int argc, char *argv[])
{
  if (! test_list ())
    return 1;

  if (! test_list_threads ())
    return 1;

  return 0;
}
