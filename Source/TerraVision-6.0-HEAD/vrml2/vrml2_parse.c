/*
 * Name: vrml2_parse.c
 *
 * Description: This file performs a syntactical analysis of a VRML 2.0 file.
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: vrml2_parse.c,v 1.4 2000/08/28 23:21:46 reddy Exp $
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

#include "config.h"
#include "c_nodes.h"

int 
main (int argc, char **argv)
{
  int result, i;
  extern int optind;
  int c;
  vsgWorld *state;
  int syntax_only = 0;
  int print = 0;

  while ((c = getopt(argc, argv, "dpst")) != EOF) {
    switch (c) {
    case 'p':
      print = 1;
      break;

    case 's':
      syntax_only = 1;
      break;

#if YYDEBUG
    case 'd': {
      extern int vsgpDebug;
      vsgpDebug = 1;
      break;
    }
    case 't': {
      ++vsgTracing;
      break;
    }
#endif
    }
  }

  if (! syntax_only)
    vsgcSetup ();

  state = vsgNewWorld (0);

  if (optind >= argc) {
    result = vsgParseStream(state, stdin, "<stdin>");
    if (result==0) printf("Parsing of input successful.\n");
    else printf("Parsing of input failed.\n");
  } else {
    for  (i = optind; i < argc; i++) {
      result = vsgParseFile(state, argv[i]);
      if (result==0) printf("Parsing of %s successful.\n", argv[i]);
      else printf("Parsing of %s failed.\n", argv[i]);
    }
  }

  if (! syntax_only && print)
    vsgPrint (state, stdout);

  vsgDestroyWorld (state);

  return result;
}
