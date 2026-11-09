/*
 * Name: test_istack.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: test_istack.c,v 1.2 2000/08/29 00:31:36 reddy Exp $
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

#include "istack.h"

int
main ()
{
  istack *is = istack_create (4000);
  int i;
  void *p;

  for (i = 0; i < 50; ++i)
    istack_alloc (is, 126);
  p = istack_finish (is);
  for (i = 0; i < 50; ++i)
    istack_alloc (is, 128);
  istack_unwind (is, p);

  for (i = 0; i < 16; ++i)
    istack_alloc (is, 8176);

  istack_destroy (is);
}
