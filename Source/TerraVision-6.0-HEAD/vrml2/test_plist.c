/*
 * Name: test_plist.c
 *
 * Library:
 *    This source file is part of the vrml2 library, a library for parsing
 *    and generating VRML97 format files. This library is supplied as part
 *    of the tsmApi distribution, available from:
 *
 *        http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: test_plist.c,v 1.3 2000/08/28 23:21:42 reddy Exp $
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

#include <stdio.h>

#include "symtab.h"
#include "plist.h"

int
main ()
{
  plist_t p = plist_create (0);
  symtab_t symtab = symtab_create (0);
  int i;
  
  for (i = 0; i < 1000; ++i) {
    symbol_t sym;
    char buf[10];
    sprintf (buf, "sym%d", i);
    sym = sym_intern (symtab, buf);
    plist_put (p, (void*)main, sym, (void*)i);

    if (strcmp (buf, sym)) {
      fprintf (stderr, "FAIL: buf \"%s\" != sym \"%s\"\n", buf, sym);
      return 1;
    }
    if (plist_get (p, (void*)main, sym) != (void*)i) {
      fprintf (stderr, "FAIL: get %d != i %d\n", 
	       plist_get (p, (void*)main, sym), i);
      return 1;
    }
  }

  return 0;
}
