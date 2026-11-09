/*
 * Name:  test_vec3.c
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
 *   $Id: test_vec3.c,v 1.9 2000/09/01 23:50:38 reddy Exp $
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
#include "vec3.h"

#ifdef HAVE_GLUT
#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>
#include <stdio.h>

void
get_glModelview (m4f *m_)
{
  glGetFloatv (GL_MODELVIEW_MATRIX, (float*) M(*m_));
}

void
get_glProjection (m4f *m_)
{
  glGetFloatv (GL_PROJECTION_MATRIX, (float*) M(*m_));
}

int
main (int argc, char **argv)
{
  m4f M, P, T, R, TR, S, TRS;
  v3f v0 = {{1, 0, 0}};
  v4f v1, r;

  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_SINGLE);
  glutCreateWindow ("test_vec3");

  get_glProjection (&P);
  printf ("-- P:\n"); print_m4f (&P);
  get_glModelview (&M);
  printf ("-- M:\n"); print_m4f (&M);

  glTranslatef (2.0, 3.0, 4.0);
  printf ("translate (2, 3, 4)\n");
  get_glModelview (&T);
  printf ("-- T:\n"); print_m4f (&T);

  mult_m4f_v3f (&T, &v0, &v1);
  printf ("T [%.2f %.2f %.2f] = [%.2f %.2f %.2f %.2f]\n", 
	  V(v0)[0], V(v0)[1], V(v0)[2],
	  V(v1)[0], V(v1)[1], V(v1)[2], V(v1)[3]);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glRotatef (90.0, 0.5, 1.0, 0.0);
  printf ("rotate (90, 0.5, 1, 0)\n");
  get_glModelview (&R);
  printf ("-- R:\n"); print_m4f (&R);

  V(r)[0] = 0.5; V(r)[1] = 1.0; V(r)[2] = 0.0;
  V(r)[3] = 90*M_PI/180;
  rotation_m4f (&r, &R);
  printf ("-- R:\n"); print_m4f (&R);

  mult_m4f_v3f (&R, &v0, &v1);
  printf ("R [%.2f %.2f %.2f] = [%.2f %.2f %.2f %.2f]\n", 
	  V(v0)[0], V(v0)[1], V(v0)[2],
	  V(v1)[0], V(v1)[1], V(v1)[2], V(v1)[3]);
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glScalef (1.0, 1.5, 2.0);
  printf ("scale (1.0, 1.5, 2.0)\n");
  get_glModelview (&S);
  printf ("-- S:\n"); print_m4f (&S);

  mult_m4f_v3f (&S, &v0, &v1);
  printf ("S [%.2f %.2f %.2f] = [%.2f %.2f %.2f %.2f]\n", 
	  V(v0)[0], V(v0)[1], V(v0)[2],
	  V(v1)[0], V(v1)[1], V(v1)[2], V(v1)[3]);
  
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();
  glTranslatef (2.0, 3.0, 4.0);
  glRotatef (90.0, 0.5, 1.0, 0.0);
  glScalef (1.0, 1.5, 2.0);
  get_glModelview (&TRS);
  printf ("-- TRS:\n"); print_m4f (&TRS);
  
  mult_m4f_m4f (&T, &R, &TR);
  printf ("-- TR:\n"); print_m4f (&TR);

  mult_m4f_m4f (&TR, &S, &TRS);
  printf ("-- TRS:\n"); print_m4f (&TRS);

  V(v0)[0] = 0;
  mult_m4f_v3f (&TRS, &v0, &v1);
  printf ("TRS [%.2f %.2f %.2f] = [%.2f %.2f %.2f %.2f]\n", 
	  V(v0)[0], V(v0)[1], V(v0)[2],
	  V(v1)[0], V(v1)[1], V(v1)[2], V(v1)[3]);

  {
    v3f Tv, Sv;
    v4f Pv, Rv;
    decompose_m4f (&TRS, &Pv, &Tv, &Rv, &Sv);
    printf ("T = [%.2f %.2f %.2f]\n", 
	    V(Tv)[0], V(Tv)[1], V(Tv)[2]); 
    printf ("R = [%.2f %.2f %.2f %.2f]\n", 
	    V(Rv)[0], V(Rv)[1], V(Rv)[2], V(Rv)[3]);
    printf ("S = [%.2f %.2f %.2f]\n", 
	    V(Sv)[0], V(Sv)[1], V(Sv)[2]);

  }

  {
    m4f R0 = {{{0, 0, 1, 0}, {0, 1, 0, 0}, {-1, 0, 0, 0}, {0, 0, 0, 1}}};
    v3f Tv, Sv;
    v4f Pv, Rv;
    printf ("-- R0:\n"); print_m4f (&R0);
    decompose_m4f (&R0, &Pv, &Tv, &Rv, &Sv);
    printf ("R = [%.2f %.2f %.2f %.2f]\n", 
	    V(Rv)[0], V(Rv)[1], V(Rv)[2], V(Rv)[3]);
  }

  {
    m4f TR0, TR1;
    v3f T;
    V(T)[0] = 1; V(T)[1] = 2; V(T)[2] = 3;
    translate_m4f (&R, &T, &TR0);
    printf ("-- TR0:\n"); print_m4f (&TR0);

    glLoadMatrixf (M(R)[0]);
    glTranslatef (1.0, 2.0, 3.0);
    get_glModelview (&TR1);
    printf ("-- TR1:\n"); print_m4f (&TR1);
  }

  return 0;
}

#endif
