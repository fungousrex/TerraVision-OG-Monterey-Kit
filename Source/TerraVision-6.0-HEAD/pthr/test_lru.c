/*
 * Name: test_lru.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: test_lru.c,v 1.3 2000/08/29 00:31:36 reddy Exp $
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

#include "lru.h"
#include "lru_hash.h"

typedef struct {
  int idx;
  int decache[25];
} lru_closure;

static void 
lru_decache_cb (void *v, void *closure)
{
  int val = (int) v;
  lru_closure *c = closure;
  c->decache[c->idx++] = val;
}

int
test_lru (void)
{
  static lru_handle handle[10] = {0};
  static int refs[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 3, 7, 2, 1, 1, 0, 4};
  static int decache_expected[] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 3};
  static lru_closure closure = {0};
  int i;

  lru_cache *lru = lru_create (5, 0);
  lru_on_decache (lru, lru_decache_cb, &closure);

  for (i = 0; i < sizeof(refs)/sizeof(refs[0]); ++i) {
    int ref = refs[i];
    handle[ref] = lru_use (lru, (void *)ref, handle[ref]);
  }
  lru_destroy (lru);

  for (i = 0; i < sizeof(decache_expected)/sizeof(decache_expected[0]); ++i)
    if (decache_expected[i] != closure.decache[i])
      return 0;

  return 1;
}

int
main (int argc, char *argv[])
{
  if (! test_lru ())
    return 1;

  return 0;
}
