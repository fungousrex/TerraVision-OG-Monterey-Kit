/* -*- tab-width: 2; -*-

  Revision History:
      $Id: test_nav.c,v 2.4.14.2 2002/10/31 05:26:44 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----
  
*/

/*
 * An absolutely minimal viewpoint-based navigator.
 * 
 * We have a minimal scene (a flat grid) and three mobile cubes which attempt
 * to track the viewpoint of the eye so that:
 *   red   = straight forward
 *   green = straight rightward
 *   blue  = straight upward
 *
 * We show four views:
 * -----------------------
 * |          |          |
 * |  upward  | overhead |
 * |          |          |
 * +----------+----------+
 * |          |          |
 * | forward  | rightward|
 * |          |          |
 * -----------------------
 * If everything works, then the cubes should remain centered in the forward,
 * rightward and upward views.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef HAVE_GLUT
#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>
#include <vrml2/vec3.h>
#include <stdio.h>

float yaw, pitch, roll;
float eye[3] = {0, 0, 10};
float mv[4][4];

int const extent = 500;
float const fextent = (float) 500;

int top;
int win[4];

GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
  {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3] = {  /* Will be filled in with X,Y,Z vertexes. */
  {-1, -1,  1}, {-1, -1, -1}, {-1,  1, -1}, {-1,  1,  1},
  { 1, -1,  1}, { 1, -1, -1}, { 1,  1, -1}, { 1,  1,  1} };

void
init_cube ()
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] =  1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] =  1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] =  1;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;
}

void
cube_at (float d, float x, float y, float z)
{
  int i;
  glPushMatrix (); {
    glTranslatef (eye[0] + d*x, eye[1] + d*y, eye[2] + d*z);
    for (i = 0; i < 6; i++) {
      glBegin(GL_QUADS);
      glNormal3fv(&n[i][0]);
      glVertex3fv(&v[faces[i][0]][0]);
      glVertex3fv(&v[faces[i][1]][0]);
      glVertex3fv(&v[faces[i][2]][0]);
      glVertex3fv(&v[faces[i][3]][0]);
      glEnd();
    }
  }; glPopMatrix ();
}

void
print_matrix (GLfloat (*mv)[4])
{
  int i, j;
  printf ("yaw %g pitch %g roll %g\n", yaw, pitch, roll);
  printf ("Matrix:\n");
  for (j = 0; j < 4; ++j) {
    for (i = 0; i < 4; ++i) {
      printf (" %6.3f", mv[i][j]);
    }
    printf ("\n");
  }
}

void
get_canonical_vector (float (*mv)[4], int which, float v[3])
{
  switch (which) {
  case 0: /* Forward */
    v[0] = -mv[0][2];
    v[1] = -mv[1][2];
    v[2] = -mv[2][2];
    break;
  case 1:
    v[0] =  mv[0][0];
    v[1] =  mv[1][0];
    v[2] =  mv[2][0];
    break;
  case 2:
    v[0] =  mv[0][1];
    v[1] =  mv[1][1];
    v[2] =  mv[2][1];
    break;
  }
}

void
draw_frame (GLfloat (*mv)[4])
{
  int i, j;

  glColor3f (0.5, 0.5, 0.5);
  glRectf (-extent, -extent, extent, extent);

  glLineWidth (1);
  glColor3f (1.0, 1.0, 1.0);
  glBegin (GL_LINES); {
    int spacing = 10;
    for (i = -extent/spacing; i <= extent/spacing; ++i) {
      glColor3f (1.0, 0.0, 0.0); /* red (-Y) */
      glVertex3i (i*spacing,   -extent, 0);

      glColor3f (0.0, 0.0, 1.0); /* blue (+Y) */
      glVertex3i (i*spacing,    extent, 0);

      glColor3f (0.0, 1.0, 1.0); /* cyan (-X) */
      glVertex3i (  -extent, i*spacing, 0);

      glColor3f (1.0, 1.0, 0.0); /* yellow  (+X) */
      glVertex3i (   extent, i*spacing, 0);
    }
  }; glEnd ();

#if 0
  glColor3f (1.0, 1.0, 0.0);
  glLineWidth (2);
  glBegin (GL_LINES); {
    int spacing = 50;
    for (i = -extent/spacing; i <= extent/spacing; ++i) {
      glVertex3i (i*spacing, -extent, 0);
      glVertex3i (i*spacing,  extent, 0);
      glVertex3i (-extent, i*spacing, 0);
      glVertex3i ( extent, i*spacing, 0);
    }
  }; glEnd ();
#endif

  {
    float d = 20;
    float v[3];

    glColor3f (1.0, 0.0, 0.0);
    get_canonical_vector (mv, 0, v);
    cube_at ( d, v[0], v[1], v[2]); /* Forward */

    glColor3f (0.0, 1.0, 0.0);
    get_canonical_vector (mv, 1, v);
    cube_at ( d, v[0], v[1], v[2]); /* Right */

    glColor3f (0.0, 0.0, 1.0);
    get_canonical_vector (mv, 2, v);
    cube_at ( d, v[0], v[1], v[2]); /* Up */
  }
}

void
display_top ()
{
  glClearColor(0.3, 0.3, 0.3, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glutSwapBuffers ();
}

void
setup_drawing ()
{
  glEnable (GL_POLYGON_OFFSET_EXT);
  glPolygonOffsetEXT (1e-5, 1e-4);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective( /* field of view in degree */ 40.0,
		  /* aspect ratio */ 1.0,
		  /* Z near */ 1.0,
		  /* Z far */ 2.F*fextent);

  glMatrixMode (GL_MODELVIEW);
}

void
display_sub ()
{
  int i;

  for (i = 0; i < 3; ++i) {
    glutSetWindow (win[i]);

    setup_drawing ();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity ();

    switch (i) {
    case 0:
      ; /* Heading Forward */
      break;
    case 1:
      glRotatef ( 90, 0, 1, 0); /* Heading Right */
      break;
    case 2:
      glRotatef (-90, 1, 0, 0); /* Heading Up */
      break;
    }

    glRotatef (-roll,   0,  0, -1);
    glRotatef (-pitch,  1,  0,  0);
    glRotatef (-yaw,    0,  1,  0);

    glRotatef ( -90,    1,  0,  0); /* Z Up */

    if (i == 0)
      glGetFloatv (GL_MODELVIEW_MATRIX, &mv[0][0]);
  
    glTranslatef (-eye[0], -eye[1], -eye[2]);

    draw_frame (mv);
    glutSwapBuffers ();
  }

  /* Do the overhead view. */
  glutSetWindow (win[3]);
  setup_drawing ();

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity ();
  glTranslatef (-eye[0], -eye[1], -(eye[2]+100));

  draw_frame (mv);

  glColor3f (1.0, 1.0, 1.0);
  cube_at (0, 0, 0, 0);
  glutSwapBuffers ();
}

void
reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective( /* field of view in degree */ 40.0,
		  /* aspect ratio */ w/(float)h,
		  /* Z near */ 1.0,
		  /* Z far */ 2*fextent);
  glMatrixMode(GL_MODELVIEW);
}

void
move_eye (float spd, float v[3])
{
  int i;
  for (i = 0; i < 3; ++i)
    eye[i] += spd*v[i];
}

void
special (int key, int x, int y)
{
  int mods = glutGetModifiers ();
  int shift = mods & GLUT_ACTIVE_SHIFT;
  int ctrl  = mods & GLUT_ACTIVE_CTRL;
  int alt   = mods & GLUT_ACTIVE_ALT;

  if (alt) {
    float v[3];
    float spd = (shift ? 4 : 1);
    /* Sliding ops */
    switch (key) {
    case GLUT_KEY_PAGE_UP:
      get_canonical_vector (mv, 2, v);
      move_eye ( spd, v);
      break;
    case GLUT_KEY_PAGE_DOWN:
      get_canonical_vector (mv, 2, v);
      move_eye (-spd, v);
      break;
    case GLUT_KEY_RIGHT:
      get_canonical_vector (mv, 1, v);
      move_eye ( spd, v);
      break;
    case GLUT_KEY_LEFT:
      get_canonical_vector (mv, 1, v);
      move_eye (-spd, v);
      break;
    case GLUT_KEY_UP:
      /* Forward */
      get_canonical_vector (mv, 0, v);
      move_eye ( spd, v);
      break;
    case GLUT_KEY_DOWN:
      /* Backward */
      get_canonical_vector (mv, 0, v);
      move_eye (-spd, v);
      break;
    }
  } else if (shift) {
    switch (key) {
    case GLUT_KEY_RIGHT:
      roll -= 1;
      break;
    case GLUT_KEY_LEFT:
      roll += 1;
      break;
    }
  } else {
    switch (key) {
    case GLUT_KEY_PAGE_UP:
      eye[2] += 1;
      break;
    case GLUT_KEY_PAGE_DOWN:
      eye[2] -= 1;
      break;
    case GLUT_KEY_RIGHT:
      yaw -= 1;
      break;
    case GLUT_KEY_LEFT:
      yaw += 1;
      break;
    case GLUT_KEY_DOWN:
      pitch -= 1;
      break;
    case GLUT_KEY_UP:
      pitch += 1;
      break;
    }
  }

  glutPostRedisplay ();
}

void
key (unsigned char key, int x, int y)
{
  int mods = glutGetModifiers ();
  int shift = mods & GLUT_ACTIVE_SHIFT;
  int ctrl  = mods & GLUT_ACTIVE_CTRL;
  int alt   = mods & GLUT_ACTIVE_ALT;

  switch (key) {
  case 'r':
    yaw = pitch = roll = 0;
    break;
  case 'm':
    print_matrix (mv);
    break;
  case 27:
    exit (0);
  }
  glutPostRedisplay ();
}

int
main (int argc, char *argv[])
{
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (512, 512);

  top = glutCreateWindow ("test_nav");
  glutDisplayFunc (display_top);
  glutSpecialFunc (special);
  glutKeyboardFunc(key);

  win[0] = glutCreateSubWindow (top,   0, 256, 256, 256);
  glutDisplayFunc (display_sub);
  glutSpecialFunc (special);
  glutKeyboardFunc(key);

  win[1] = glutCreateSubWindow (top, 256, 256, 256, 256);
  glutDisplayFunc (display_sub);
  glutSpecialFunc (special);
  glutKeyboardFunc(key);

  win[2] = glutCreateSubWindow (top,   0,   0, 256, 256);
  glutDisplayFunc (display_sub);
  glutSpecialFunc (special);
  glutKeyboardFunc(key);

  win[3] = glutCreateSubWindow (top, 256,   0, 256, 256);
  glutDisplayFunc (display_sub);
  glutSpecialFunc (special);
  glutKeyboardFunc(key);

  glutMainLoop ();
  return 0;
}
#else
int
main ()
{
  printf ("Can't run without GLUT.\n");
  return 1;
}
#endif
