/*
 * Name: test_syms.c
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
 *   $Id: test_syms.c,v 1.4 2000/08/28 23:21:43 reddy Exp $
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

#include <stdlib.h>
#include <stdio.h>

#include "vrml2int.h"
#include "parse.h"
#include "syms.c"

#define HEADER_TOP(s)
#define SOURCE_TOP(s)
#define STRING_PREFIX(s)
#define FUNC_NAME(s)
#define STRUCT_NAME(s)
#define SYM(s,d)

struct symbol {
  char const *sym;
  int len;
} all_syms[] = {
#undef SYM
#define SYM(s,d) { vsgN##s, sizeof(#s)-1 },
#include "symlist.def"
#undef SYM
#define SYM(s,d)
  { 0, 0 }
};

int
main (int argc, char *argv[])
{
  struct symbol *s;
  
  for (s = all_syms; s->sym; ++s) {
    struct vsgpId *id = is_reserved_word (s->sym, s->len);
    if (! id || id->name != s->sym) {
      fprintf (stderr, "FAIL: symbol %s\n", s->sym);
      return 1;
    }
  }

  return 0;
}
