/*
 * Name: decomp.c
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
 *   $Id: decomp.c,v 1.6 2000/08/28 23:21:34 reddy Exp $
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

#include <math.h>
#include <float.h>

#include "vec3.h"

#define TRUE (1)
#define FALSE (0)

#define DBG(s)

static int
extractP_m4f (m4f *M_, v4f *P_)
{
  float (*M)[4] = M(*M_);
  if (M[0][3] != 0 || M[1][3] != 0 || M[2][3] != 0 || M[3][3] != 1) {
    float b[4];
    int i;
    for (i = 0; i < 3; ++i) {
      b[i] = M[i][3]; M[i][3] = 0;
    }
    b[3] = M[3][3]; M[3][3] = 1; 
    lusolve_m4f_v4f (M_, 4, (v4f *)b, P_);
  } else {
    float *P = V(*P_);
    P[0] = 0; P[1] = 0; P[2] = 0; P[3] = 1;
  }

  return TRUE;
}

static int
extractT_m4f (m4f *M_, v3f *T_)
{
  float (*M)[4] = M(*M_);
  float *T = V(*T_);
  int j;
  for (j = 0; j < 3; ++j) {
    T[j] = M[3][j];
    M[3][j] = 0;
  }

  return TRUE;
}

/* These functions operate on rows of a m4f 4x4 matrix.
   N.B. We cheat here, since we know that we've zeroed everything 
   outside of the first three rows and columns. */

/* Compute the inner product of rows <i0> and <i1>. */
static float
rowdot (m4f const *M_, int i0, int i1)
{
  float const *row0 = &M(*M_)[i0][0];
  float const *row1 = &M(*M_)[i1][0];
  float sum;
  int j;
  for (j = 0, sum = 0; j < 3; ++j)
    sum += row0[j] * row1[j];
  return sum;
}

/* Compute the norm of row <i>. */
static float
rowlen (m4f const *M, int i)
{
  return sqrt (rowdot (M, i, i));
}

/* Multiply all values in row <i> by <scale>. */
static void
rowscale (m4f *M, int i, float scale)
{
  float *row = &M(*M)[i][0];
  int j;
  for (j = 0; j < 3; ++j)
    row[j] *= scale;
}

/* Compute scaling S and shears (which should all be zero). */
#define MAX_SHEAR (1e-5)

static int
extractS_m4f (m4f *M, v3f *S_)
{
  float (*m)[4] = M(*M);
  float *S = V(*S_);
  float sx, sy, sz;
  float sxy, sxz, syz;
  int j;

  /* Find X scale and normalize row 0. */
  S[0] = sx = rowlen (M, 0);
  rowscale (M, 0, 1/sx);
  DBG ((printf ("M': sx = %.2f\n", sx),  print_m4f (M)));

  /* Compute XY shear and orthogonalize row 1. */
  sxy = rowdot (M, 0, 1);
  if (fabs (sxy) > MAX_SHEAR) {
    for (j = 0; j < 3; ++j)
      m[1][j] -= sxy * m[0][j];
    DBG ((printf ("M': sxy = %.2f\n", sxy), print_m4f (&M)));
    return FALSE;
  }

  /* Find Y scale and normalize row 1. */
  S[1] = sy = rowlen (M, 1);
  rowscale (M, 1, 1/sy);
  sxy /= sy;
  DBG ((printf ("M': sy = %.2f sxy = %.2f\n", sy, sxy), print_m4f (M)));

  /* Compute XZ shear and orthogonalize row 2 (wrt. row 0). */
  sxz = rowdot (M, 0, 2);
  if (fabs (sxz) > MAX_SHEAR) {
    for (j = 0; j < 3; ++j)
      m[2][j] -= sxz * m[0][j];
    DBG ((printf ("M': sxz = %.2f\n", sxz), print_m4f (M)));
    return FALSE;
  }
    
  /* Compute YZ shear and orthogonalize row 2 (wrt. row 1). */
  syz = rowdot (M, 1, 2);
  if (fabs (syz) > MAX_SHEAR) {
    for (j = 0; j < 3; ++j)
      m[2][j] -= syz * m[1][j];
    DBG ((printf ("M': syz = %.2f\n", syz), print_m4f (M)));
    return FALSE;
  }
    
  /* Find Z scale and normalize row 2. */
  S[2] = sz = rowlen (M, 2);
  rowscale (M, 2, 1/sz);
  sxz /= sz;
  syz /= sz;
  DBG ((printf ("M': sz = %.2f sxz = %.2f syz = %.2f\n", sz, sxz, syz),
	print_m4f (M)));

  return TRUE;
}

static int
extractR_m4f (m4f const *T_, v4f *R_, int no_prop)
{
  float *R = V(*R_);
  float const (*T)[4] = M(*T_);
  float arg = (T[0][0] + T[1][1] + T[2][2] - 1.0) / 2.0;
  float sn2;

  if( arg > 1.0 ) {
    arg = 1.0;
  } else if( arg < -1.0 ) {
    arg = -1.0;
  }

  R[3] = acos (arg);
  sn2 = 2.0 *  sin(R[3]);

  if (fabs (sn2) > 1e-6) {
    R[0] = (T[1][2] - T[2][1]) / sn2;
    R[1] = (T[2][0] - T[0][2]) / sn2;
    R[2] = (T[0][1] - T[1][0]) / sn2;
  } else {
    m4f Ttry;
    int i, isign, ibest, isignbest;
    float dbest = FLT_MAX;
    for (isign = 0; isign < 2; isign++) {
      for (i = 0; i < 3; i++) {
	float dif;
	int ii, jj;
	R[0] = 0; R[1] = 0; R[2] = 0;
	R[i] = isign ? -1.0 : 1.0;
	rotation_m4f (R_, &Ttry);
	dif = 0;
	for (ii = 0; ii < 3; ++ii) {
	  for (jj = 0; jj < 3; ++jj) {
	    float d = M(*T_)[ii][jj] - M(Ttry)[ii][jj];
	    dif += d*d;
	  }
	}
	dif = sqrt (dif);
	if (dif < dbest ) {
	  ibest = i;
	  isignbest = isign;
	  dbest = dif;
	}
      }
    }


    if (dbest < 0 || no_prop) {
      R[0] = 0; R[1] = 0; R[2] = 0;
      R[ibest] = isignbest ? -1.0 : 1.0;
    } else {
      m4f Ttmp, Tnew;
      v4f vtmp;
      V(vtmp)[0] = 1.0;
      V(vtmp)[1] = 2.0;
      V(vtmp)[2] = 3.0;
      V(vtmp)[3] = 0.001;
      normalize_v3f ((v3f *) &vtmp);
      rotation_m4f (&vtmp, &Ttmp);
      mult_m4f_m4f (&Ttmp, T_, &Tnew);
      return extractR_m4f (&Tnew, R_, 1);
    }
  }

  return TRUE;
}

/* This process is adapted from Graphics Gems II, VII.I "Decomposing a Matrix
   into Simple Transformations. */

int
decompose_m4f (m4f const *M0, v4f *P, v3f *T, v4f *R, v3f *S)
{
  m4f M = *M0;
  float (*m)[4] = M(M);

  if (! extractP_m4f (&M, P))
    return FALSE;
  DBG ((printf ("M\\P: P = [%.2f, %.2f, %.2f, %.2f]\n",
		V(*P)[0], V(*P)[1], V(*P)[2], V(*P)[3]),
	print_m4f (&M)));

  if (! extractT_m4f (&M, T))
    return FALSE;
  DBG ((printf ("M\\P\\T: T = [%.2f, %.2f, %.2f]\n", 
		V(*T)[0], V(*T)[1], V(*T)[2]),
	print_m4f (&M)));

  if (! extractS_m4f (&M, S))
    return FALSE;

  /* We now have a pure 3x3 rotation matrix in the upper-left corner. */
  {
    float det = det3x3 (m[0][0], m[1][0], m[2][0],
			m[0][1], m[1][1], m[2][1],
			m[0][2], m[1][2], m[2][2]);

    /* If determinant is -1, then negate matrix and S */
    if (det < 0) {
      int i;
      for (i = 0; i < 3; ++i) {
	rowscale (&M, i, -1.0);
	V(*S)[i] *= -1;
      }
    }
    DBG ((printf ("M': det = %.2f\n", det),
	  print_m4f (&M)));
  }

  return extractR_m4f (&M, R, 0);
}
