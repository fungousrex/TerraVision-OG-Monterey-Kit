/****************************************************************************
 * Name:
 *    	glmTiming.c
 *
 * Description:
 *	Routines for testing the speed of glm visual contexts.
 *
 * Function List:
 *
 * Dependencies:
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 ****************************************************************************/

#include <stdlib.h>

#include <glt/glt.h>
#include <glt/glm.h>

#include <prtime.h>

static GLuint
setupTiming (int w, int h, int format, void *texture_data)
{
  GLuint texid;

  /*
   * Disable stuff that's likely to slow down 
   * glDrawPixels.(Omit as much of this as possible, 
   * when you know in advance that the OpenGL state is
   * already set correctly.)
   */
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_DITHER);
  glDisable(GL_FOG);
  glDisable(GL_LIGHTING);
  glDisable(GL_LOGIC_OP);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
  glPixelTransferi(GL_RED_SCALE, 1);
  glPixelTransferi(GL_RED_BIAS, 0);
  glPixelTransferi(GL_GREEN_SCALE, 1);
  glPixelTransferi(GL_GREEN_BIAS, 0);
  glPixelTransferi(GL_BLUE_SCALE, 1);
  glPixelTransferi(GL_BLUE_BIAS, 0);
  glPixelTransferi(GL_ALPHA_SCALE, 1);
  glPixelTransferi(GL_ALPHA_BIAS, 0);

  /*
     * Disable extensions that could slow down 
     * glDrawPixels.(Actually, you should check for the 
     * presence of the proper extension before making 
     * these calls.I omitted that code for simplicity.)
     */

#ifdef GL_EXT_histogram
  glDisable(GL_HISTOGRAM_EXT);
  glDisable(GL_MINMAX_EXT);
#endif

#ifdef GL_EXT_texture3D
  glDisable(GL_TEXTURE_3D_EXT);
#endif

  /*
     * The following is needed only when using a  
     * multisample-capable visual. 
     */

#ifdef GL_SGIS_multisample
  glDisable(GL_MULTISAMPLE_SGIS);
#endif

  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

  glEnable (GL_TEXTURE_2D);
  glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  gltGenTextures (1, &texid);
  gltBeginTexture (GL_TEXTURE_2D, texid); {
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB4_EXT, w, h,
		  0, format, GL_UNSIGNED_BYTE, texture_data);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }; gltEndTexture ();

  /* Set up initial drawing parameters */
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();

  glClearColor (0.0, 0.0, 0.0, 0.0);
  glColor3f (1.0, 1.0, 1.0);
  glShadeModel (GL_FLAT);
  glLineWidth (1.0);

  return texid;
}

static void
drawTexture (GLuint id)
{
  gltBindTexture (GL_TEXTURE_2D, id);
  glBegin (GL_TRIANGLE_STRIP); {
    glTexCoord2f (0.0, 0.0);
    glVertex2f (-0.5, -0.5);

    glTexCoord2f (0.0, 1.0);
    glVertex2f (-0.5,  0.5);

    glTexCoord2f (1.0, 0.0);
    glVertex2f ( 0.5, -0.5);

    glTexCoord2f (1.0, 1.0);
    glVertex2f ( 0.5,  0.5);
  }; glEnd ();
}

static void
cleanupTiming (GLuint texid)
{
  gltDeleteTextures (1, &texid);
}

typedef struct {
  unsigned char r, g, b;
} RGB;

int
glmWindowSpeed (int width, int height, glmWindow glmw)
{
  RGB *texture = calloc (128*128, sizeof (*texture));
  RGB **tex = calloc (128, sizeof(*tex));
  int i, j;
  GLuint texid;
  PRTime start, now;

  static char const speed_message[] = "Testing display modes...";

  for (i = 0; i < 128; ++i)
    tex[i] = &texture[128*i];

  for (i = 0; i < 128; ++i) {
    RGB *v;
    for (j = 0; j < 128; ++j) {
      v = &tex[i][j];
      v->r = i*2+1; v->b = j*2+1;
    }
    v = &tex[i][i];
    v->r = v->g = v->b = 255;
    v = &tex[i][127-i];
    v->r = v->g = v->b = 255;
  }

  /* Clear to black. */
  glmMakeCurrentWindow ("TimeVisual", glmw); {
    glDisable (GL_DITHER);
    glViewport( 0, 0, width, height );
    glClearColor (0.0, 0.0, 0.0, 1.0); 
    glClearDepth (-1.0); 
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( -0.5, width-0.5, -0.5, height-0.5, -1.0, 1.0 );

    glColor4ub (0xFF, 0xCA, 0xCA, 0xff);
    glRasterPos2i ((width - strlen(speed_message) * 9) / 2,
		   height / 2);
    glmCharstr (glmw, GL_FONT_R, speed_message);

    texid = setupTiming (128, 128, GL_RGB, texture);
  }; glmReleaseWindow ("TimeVisual", glmw);

  /* Now do timing tests. */
  start = PR_Now ();
  glmMakeCurrentWindow ("TimeVisual", glmw); {
    for (i = 0; ; ++i) {
      glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      glPushMatrix (); {
	glRotatef (i*10.0, 0.0, 0.0, 1.0);
	drawTexture (texid);
      }; glPopMatrix ();
      glFlush ();
      now = PR_Now ();
      if (now-start > 500000) break;
    }
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  }; glmReleaseWindow ("TimeVisual", glmw);
  i *= (now-start)/500000.0;
  /*printf ("Timing: %d iters/sec\n", 2*i);*/

  glmMakeCurrentWindow ("TimeVisual", glmw); {
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    cleanupTiming (texid);
  }; glmReleaseWindow ("TimeVisual", glmw);
  
  return i;
}
