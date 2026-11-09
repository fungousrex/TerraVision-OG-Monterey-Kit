/* -*- tab-width: 2; -*-

  Name: matrix.c
 
  Description: Miscellaneous matrix functions
 
  Revision History:
      $Id: matrix.c,v 2.23.4.5 2002/10/31 05:26:44 reddy Exp $

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define NO_matrix_INLINE
#include "matrix.h"

#include <math.h>

#ifndef HAVE_INLINE
#define exinline /* inline */
#include "matrix_inline.h"
#endif

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#ifndef HAVE_INLINE
#define exinline 
#else
#define exinline inline
#endif
#include "matrix_inline.h"

GLmatrix const Identity = {
  {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}
};

/* RCS INFO */
static char const matrix_rcsid[] = "$Id: matrix.c,v 2.23.4.5 2002/10/31 05:26:44 reddy Exp $";


/*
 * Synopsis: Takes length of a float vector
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Nat Bletter
 * 
 * Date:
 *     April 9, 1999
 *
 */
float
LengthVec(float const *a, 
	  int   dim)
{
 float len = 0;
 float len2 = 0;
 int i;

    for (i = 0; i < dim; i++)
    {
	len2 += a[i] * a[i];
    }
    len = sqrt(len2);
    return len;
}


/*
 * Synopsis: Takes length of a double vector
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Nat Bletter
 * 
 * Date:
 *     April 9, 1999
 *
 */
double
LengthVecd(double const *a, 
	  int   dim)
{
 double len = 0;
 double len2 = 0;
 int i;

    for (i = 0; i < dim; i++)
    {
	len2 += a[i] * a[i];
    }
    len = sqrt(len2);
    return len;
}


/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
NormalizeVector(float const *a, 
                float *n,
                int   dim)
{
 float len = 0;
 float len2 = 0;
 int i;

    for (i = 0; i < dim; i++)
    {
	len2 += a[i] * a[i];
    }
    len = sqrt(len2);
    for(i = 0; i < dim; i++)
    {
  	n[i] = a[i] / len;
    }
}

void
NormalizeVectord(double const *a, 
                double *n,
                int   dim)
{
 double len = 0;
 double len2 = 0;
 int i;

    for (i = 0; i < dim; i++)
    {
	len2 += a[i] * a[i];
    }
    len = sqrt(len2);
    for(i = 0; i < dim; i++)
    {
  	n[i] = a[i] / len;
    }
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */
float
DotProduct(float const *a, 
           float const *b, 
           int    dim)
{
 int i;
 float dot = 0;

    for(i = 0; i < dim; i++)
    {
	dot += a[i] * b[i];
    }
    return dot;
}


double
DotProductd(double const *a, 
           double const *b, 
           int    dim)
{
 int i;
 double dot = 0;

    for(i = 0; i < dim; i++)
    {
	dot += a[i] * b[i];
    }
    return dot;
}


/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
CrossProduct(float const a[3], 
             float const b[3], 
             float c[3])
{
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
SphericalToRect(float theta,
                float phi,
                float a[3])
{
    a[0] = cos(theta) * sin(phi);
    a[2] = sin(theta) * sin(phi);
    a[1] = cos(phi);
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
GLMultMatrix(const_GLmatrix src,
             const_GLmatrix multi, 
             GLmatrix dest)
{
  GLint mm;
  glGetIntegerv (GL_MATRIX_MODE, &mm);
  if (mm != GL_MODELVIEW_MATRIX) glMatrixMode (GL_MODELVIEW_MATRIX);
  glPushMatrix (); {
    glLoadMatrixf((GLfloat *) multi);
    glMultMatrixf((GLfloat *) src);
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) dest);
  }; glPopMatrix();
  if (mm != GL_MODELVIEW_MATRIX) glMatrixMode (mm);
}

void
MultMatrixVec(const_GLmatrix m, float const v[3], float o[4])
{
  float v0 = v[0], v1 = v[1], v2 = v[2];
  o[0] = m[0][0] * v0 + m[1][0] * v1 + m[2][0] * v2 + m[3][0];
  o[1] = m[0][1] * v0 + m[1][1] * v1 + m[2][1] * v2 + m[3][1];
  o[2] = m[0][2] * v0 + m[1][2] * v1 + m[2][2] * v2 + m[3][2];
  o[3] = m[0][3] * v0 + m[1][3] * v1 + m[2][3] * v2 + m[3][3];
}

void
MultMatrixVecd(const_GLmatrix m, double const v[3], double o[4])
{
  double v0 = v[0], v1 = v[1], v2 = v[2];
  o[0] = m[0][0] * v0 + m[1][0] * v1 + m[2][0] * v2 + m[3][0];
  o[1] = m[0][1] * v0 + m[1][1] * v1 + m[2][1] * v2 + m[3][1];
  o[2] = m[0][2] * v0 + m[1][2] * v1 + m[2][2] * v2 + m[3][2];
  o[3] = m[0][3] * v0 + m[1][3] * v1 + m[2][3] * v2 + m[3][3];
}

void
MultVecMatrix(float const v[4], const_GLmatrix m, float o[4])
{
  float v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
  o[0] = m[0][0] * v0 + m[0][1] * v1 + m[0][2] * v2 + m[0][3] * v3;
  o[1] = m[1][0] * v0 + m[1][1] * v1 + m[1][2] * v2 + m[1][3] * v3;
  o[2] = m[2][0] * v0 + m[2][1] * v1 + m[2][2] * v2 + m[2][3] * v3;
  o[3] = m[3][0] * v0 + m[3][1] * v1 + m[3][2] * v2 + m[3][3] * v3;
}

void
MultVecMatrixd(double const v[4], const_GLmatrix m, double o[4])
{
  double v0 = v[0], v1 = v[1], v2 = v[2], v3 = v[3];
  o[0] = m[0][0] * v0 + m[0][1] * v1 + m[0][2] * v2 + m[0][3] * v3;
  o[1] = m[1][0] * v0 + m[1][1] * v1 + m[1][2] * v2 + m[1][3] * v3;
  o[2] = m[2][0] * v0 + m[2][1] * v1 + m[2][2] * v2 + m[2][3] * v3;
  o[3] = m[3][0] * v0 + m[3][1] * v1 + m[3][2] * v2 + m[3][3] * v3;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
MultMatrix(const_GLmatrix m1,
	   const_GLmatrix m2,
	   GLmatrix dest)
{
 int i,j;
 GLfloat *dp = (GLfloat *) dest;

 for (i = 0; i < 4; i++) {
   GLfloat (*mult1)[4] = (GLfloat (*)[4]) &m1[i][0];
   GLfloat m1_i0 = mult1[0][0];
   GLfloat m1_i1 = mult1[0][1];
   GLfloat m1_i2 = mult1[0][2];
   GLfloat m1_i3 = mult1[0][3];
   for (j = 0; j < 4; j++) {
     GLfloat (*mult2)[4] = (GLfloat (*)[4]) &m2[0][j];
     GLfloat v;
     v  = m1_i0 * mult2[0][0];
     v += m1_i1 * mult2[1][0];
     v += m1_i2 * mult2[2][0];
     v += m1_i3 * mult2[3][0];
     *dp++ = v;
   }
 }
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void
MultMatrixd(const_GLmatrixd m1,
	    const_GLmatrix m2,
	    GLmatrixd dest)
{
 int i,j;
 GLdouble *dp = (GLdouble *) dest;

 for (i = 0; i < 4; i++) {
   GLdouble (*mult1)[4] = (GLdouble (*)[4]) &m1[i][0];
   for (j = 0; j < 4; j++) {
     GLfloat (*mult2)[4] = (GLfloat (*)[4]) &m2[0][j];
     GLdouble v;
     v  = mult1[0][0] * mult2[0][0];
     v += mult1[0][1] * mult2[1][0];
     v += mult1[0][2] * mult2[2][0];
     v += mult1[0][3] * mult2[3][0];
     *dp++ = v;
   }
 }
}

void CopyMatrix(const_GLmatrix src, GLmatrix dst)
{
  
  memcpy((void *)dst, (void const *)src, sizeof(GLmatrix));
}


/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *     April 9, 1993
 *
 */
void 
TranslationMatrix (float t0, float t1, float t2,
		   GLmatrix m)
{
  memcpy (m, Identity, sizeof (GLmatrix));
  m[3][0] = t0;
  m[3][1] = t1;
  m[3][2] = t2;
}

void
TranslateMatrix(float t0, float t1, float t2,
		const_GLmatrix m2,
		GLmatrix dest)
{
 int j;
 GLfloat *dp = (GLfloat *) dest;

 memcpy (dest, m2, 12 * sizeof (dp[0]));
 dp = (GLfloat *) &dest[3][0];

 for (j = 0; j < 4; j++) {
   GLfloat v;
   v  = t0 * m2[0][j];
   v += t1 * m2[1][j];
   v += t2 * m2[2][j];
   v +=      m2[3][j];
   *dp++ = v;
 }
}

void 
RotationMatrix (float angle, float x, float y, float z,
		GLmatrix m)
{
  /* This function contributed by Erich Boleyn (erich@uruk.org) */
  float mag, s, c;
  float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

  s = sin( angle * M_PI/180 );
  c = cos( angle * M_PI/180 );

  mag = sqrt ( x*x + y*y + z*z );

  if (mag == 0.0) {
    /* generate an identity matrix and return */
    memcpy (m, Identity, sizeof(m[0][0])*16);
    return;
  }

   x /= mag;
   y /= mag;
   z /= mag;

#define M(row,col)  m[col][row]
   xx = x * x;
   yy = y * y;
   zz = z * z;
   xy = x * y;
   yz = y * z;
   zx = z * x;
   xs = x * s;
   ys = y * s;
   zs = z * s;
   one_c = 1.0F - c;

   M(0,0) = (one_c * xx) + c;
   M(0,1) = (one_c * xy) - zs;
   M(0,2) = (one_c * zx) + ys;
   M(0,3) = 0.0F;

   M(1,0) = (one_c * xy) + zs;
   M(1,1) = (one_c * yy) + c;
   M(1,2) = (one_c * yz) - xs;
   M(1,3) = 0.0F;

   M(2,0) = (one_c * zx) - ys;
   M(2,1) = (one_c * yz) + xs;
   M(2,2) = (one_c * zz) + c;
   M(2,3) = 0.0F;

   M(3,0) = 0.0F;
   M(3,1) = 0.0F;
   M(3,2) = 0.0F;
   M(3,3) = 1.0F;

#undef M
}

void
RotateMatrix(float angle, float x, float y, float z,
	     const_GLmatrix m2,
	     GLmatrix dest)
{
  GLmatrix R;

  RotationMatrix (angle, x, y, z, R);
  MultMatrix ((const_GLmatrix) R, m2, dest);
}

void
RotateVec(float angle, float x, float y, float z,
	  float const *v,
	  float *dest)
{
  GLmatrix R;
  float d[4];

  RotationMatrix (angle, x, y, z, R);
  MultMatrixVec ((const_GLmatrix) R, v, d);
  memcpy (dest, d, 3 * sizeof (d[0]));
}

#include <stdio.h>
void
RotateVecd(float angle, float x, float y, float z,
	   double const *v,
	   double *dest)
{
  GLmatrix R;
  double d[4];

  RotationMatrix (angle, x, y, z, R);
  MultMatrixVecd ((const_GLmatrix) R, v, d);
  memcpy (dest, d, 3 * sizeof (d[0]));
}

void
FrustumMatrix (float left, float right,
	       float bottom, float top,
	       float nearPlane, float farPlane,
	       GLmatrix m)
{
  float width = right-left;
  float height = top-bottom;
  float range = farPlane-nearPlane;
  float A = (right+left)/width;
  float B = (top+bottom)/height;
  float C = -(nearPlane+farPlane)/range;
  float D = -2*farPlane*nearPlane/range;

  m[0][0] = 2*nearPlane/width; m[1][0] = 0; m[2][0] = A; m[3][0] = 0;
  m[0][1] = 0; m[1][1] = 2*nearPlane/height; m[2][1] = B; m[3][1] = 0;
  m[0][2] = 0; m[1][2] = 0; m[2][2] = C; m[3][2] = D;
  m[0][3] = 0; m[1][3] = 0; m[2][3] = -1; m[3][3] = 0;
}

void
PerspectiveMatrix (float fovy, float aspect, float nearPlane, float farPlane,
		   GLmatrix m)
{
  float ymax = nearPlane * tan (fovy * M_PI/360.0);
  FrustumMatrix (-ymax*aspect, ymax*aspect, -ymax, ymax, nearPlane, farPlane,
		 m);
}

/*
 * Synopsis: double Sqr(double x)
 *	
 *
 * Description: returns the square of the input
 *
 *
 * Externals:
 *
 *
 * Returns: double
 *
 * Author:
 *		Nat Bletter
 * 
 * Date:
 *     April 9, 1993
 *
 */
float
Sqr(float x)
{

  return (x*x);
}

#include <stdio.h>

void
print_matrix (const_GLmatrix m)
{
  int i, j;

  printf ("[\n");
  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 4; ++j) {
      printf (" %.2f", m[j][i]);
    }
    printf ("\n");
  }
  printf ("]");
}

