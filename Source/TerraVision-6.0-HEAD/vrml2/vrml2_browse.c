/*
 * Name: vrml2_browse.c
 *
 * Description: read and display a VRML file.
 *
 * Library:
 *    This source file is part of the vrml2 library, a library for
 *    parsing and generating VRML 97 format files.  This library is
 *    supplied as part of the tsmApi distribution, available from:
 *
 *        http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: vrml2_browse.c,v 1.8 2000/09/01 23:50:37 reddy Exp $
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
#ifdef HAVE_GLUT
#include <GL/gl.h>
#include <GL/glut.h>

#include "c_nodes.h"
#include "config.h"

vsgWorld *state;

static void
render (void)
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  /* N.B. this no longer does anything because we have moved the  */
  /* render code out of the vrml2 lib and into TerraVision. The   */
  /* user can install a custom callback with vsgSetupCustomRender */
  /* See TerraVision/VRML.c for an example of this.               */

  vsgRender (state, NULL);
  
  glutSwapBuffers();
}

int 
main (int argc, char **argv)
{
  int result, i;
  FILE *f;
  extern char *optarg;
  extern int optind;
  int c;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);
  glutCreateWindow("VRML97 Browser");

  while ((c = getopt(argc, argv, "dt")) != EOF) {
    switch (c) {

#if defined(YYDEBUG) && YYDEBUG
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

  vsgcSetup ();
  vsgcSetupRenderGL ();

  state = vsgNewWorld (0);

  if (optind >= argc) {
    result = vsgParseStream(state, stdin, "<stdin>");
  } else {
    for  (i = optind; i < argc; i++) {
      result = vsgParseFile (state, argv[i]);
    }
  }

  glutDisplayFunc (render);
  glutMainLoop();

  vsgDestroyWorld (state);

  return result;
}

#endif
