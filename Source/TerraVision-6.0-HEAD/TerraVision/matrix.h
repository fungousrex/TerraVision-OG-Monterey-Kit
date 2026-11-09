/* -*- tab-width: 2; -*-

  Name: Matrix.h
 
  Description: Miscellaneous matrix functions
 
  Revision History:
      $Id: matrix.h,v 2.19.12.2 2002/10/31 05:26:44 reddy Exp $

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

#ifndef MATRIX_H
#define MATRIX_H

#ifdef WIN32
#include <windows.h>
#endif

#include <math.h>
#include <GL/gl.h>

#ifndef min
#define min(a, b)  (((a)<(b)) ? (a) : (b))
#define max(a, b)  (((a)>(b)) ? (a) : (b))
#endif

typedef GLfloat GLmatrix[4][4];
typedef GLdouble GLmatrixd[4][4];

typedef GLfloat const (* const_GLmatrix)[4];
typedef GLdouble const (* const_GLmatrixd)[4];

extern GLmatrix const Identity;

void  CopyVec(float *src, float *dst, int dims);
void  CopyVecd(double *src, double *dst, int dims);
void  ConvertDtoF(double *d, float *f, int dims);
void  ConvertFtoD(float *f, double *d, int dims);
void  SubVector(float const a[3], float const b[3], float c[3]);
void  SubVectord(double const a[3], double const b[3], double c[3]);
void  AddVector(float const a[3], float const b[3], float c[3]);
void  AddVectord(double const a[3], double const b[3], double c[3]);
void  Scalard(double const a, double const b[3], double c[3]);
void  Scalar(float const a, float const b[3], float c[3]);
float DotProduct(float const *, float const *, int);
double DotProductd(double const *, double const *, int);
void  NormalizeVector(float const *, float *, int);
void  NormalizeVectord(double const *, double *, int);
double LengthVecd(double const *a, int   dim);
float LengthVec(float const *a, int   dim);
void  CrossProduct(float const [3], float const [3], float [3]);
void  SphericalToRect(float, float, float *);
void  MultMatrix(const_GLmatrix, const_GLmatrix, GLmatrix);
void  MultMatrixd(const_GLmatrixd, const_GLmatrix, GLmatrixd);
void  MultVecMatrix(float const [4], const_GLmatrix, float [4]);
void  MultMatrixVec(const_GLmatrix, float const [3], float [4]);
void  MultMatrixVecd(const_GLmatrix, double const [3], double [4]);
void  MultVecMatrixd(double const [4], const_GLmatrix, double [4]);
void  GLMultMatrix(const_GLmatrix, const_GLmatrix, GLmatrix);
void  CopyMatrix(const_GLmatrix src, GLmatrix dst);
float Sqr(float);

void  TranslateMatrix(float t0, float t1, float t2,
		      const_GLmatrix m2, GLmatrix dest);
void  RotateMatrix(float angle, float x, float y, float z,
		   const_GLmatrix m2,
		   GLmatrix dest);
void RotateVec(float angle, float x, float y, float z,
	       float const *v, float *dest);
void RotateVecd(float angle, float x, float y, float z,
		double const *v, double *dest);

void RotationMatrix (float angle, float x, float y, float z,
		     GLmatrix m);
void TranslationMatrix (float t0, float t1, float t2,
			GLmatrix m);

void FrustumMatrix (float left, float right,
		    float bottom, float top,
		    float nearPlane, float farPlane,
		    GLmatrix m);
void PerspectiveMatrix (float fovy, float aspect, float nearPlane,
			float farPlane, GLmatrix m);

#if defined(HAVE_INLINE) && !defined(NO_matrix_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# else
#  define exinline inline
# endif
# include "matrix_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif

