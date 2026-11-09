/*
 * Name: invert.c
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
 *   $Id: invert.c,v 1.4 2000/08/28 23:21:36 reddy Exp $
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
#include "vec3.h"

/* 
 *   inverse_m4f ( original_matrix, inverse_matrix )
 * 
 *    calculate the inverse of a 4x4 matrix
 *
 *     -1     
 *     A  = ___1__ adjoint A
 *         det A
 */

int
inverse_m4f ( m4f const *m_, m4f *m0_)
{
  int i, j;
  double det;

  adjoint_m4f (m_, m0_);
  det = det_m4f (m_);

  if ( fabs(det) < 1e-8)
    return 0;

  for (i = 0; i < 4; ++i) {
    float (*m0)[4] = (float (*)[4]) &M(*m0_)[i][0];
    for (j = 0; j < 4; ++j)
      m0[0][j] /= det;
  }

  return 1;
}

/*
 * double det2x2 ( a, b, c, d )
 * 
 * calculate the determinant of a 3x3 matrix:
 *
 *     | m00, m01 |
 *     | m10, m11 |
 */

double
det2x2 (double m00, double m01,
	double m10, double m11)
{
  return m00 * m11 - m01 * m10;
}

/*
 * double det3x3 ( m00, m10, m20, m01, m11, m21, m02, m12, m22 )
 * 
 * calculate the determinant of a 3x3 matrix:
 *
 *     | m00,  m01,  m02 |
 *     | m10,  m11,  m12 |
 *     | m20,  m21,  m22 |
 */

double 
det3x3 (double m00, double m10, double m20,
	double m01, double m11, double m21,
	double m02, double m12, double m22)
{
  return (+ m00 * det2x2 (m11, m21, m12, m22)
	  - m01 * det2x2 (m10, m20, m12, m22)
	  + m02 * det2x2 (m10, m20, m11, m21));
}

/*
 * calculate the determinant of a 3x3 matrix.
 */

double det_m3f (m3f const *m_)
{
  float const (*m)[3] = M(*m_);

  return det3x3 (m[0][0], m[1][0], m[2][0],
		 m[0][1], m[1][1], m[2][1],
		 m[0][2], m[1][2], m[2][2]);
}

/*
 * double = det4x4 (matrix)
 * 
 * calculate the determinant of a 4x4 matrix.
 */

double det_m4f (m4f const *m_)
{
  float const (*m)[4] = M(*m_);

  double m00, m10, m20, m30, m01, m11, m21, m31;
  double m02, m12, m22, m32, m03, m13, m23, m33;

  /* assign to individual variable names to aid selecting */
  /*  correct elements */

  m00 = m[0][0]; m01 = m[0][1]; 
  m02 = m[0][2]; m03 = m[0][3];

  m10 = m[1][0]; m11 = m[1][1]; 
  m12 = m[1][2]; m13 = m[1][3];

  m20 = m[2][0]; m21 = m[2][1]; 
  m22 = m[2][2]; m23 = m[2][3];

  m30 = m[3][0]; m31 = m[3][1]; 
  m32 = m[3][2]; m33 = m[3][3];

  return (+ m00 * det3x3 (m11, m21, m31, m12, m22, m32, m13, m23, m33)
	  - m01 * det3x3 (m10, m20, m30, m12, m22, m32, m13, m23, m33)
	  + m02 * det3x3 (m10, m20, m30, m11, m21, m31, m13, m23, m33)
	  - m03 * det3x3 (m10, m20, m30, m11, m21, m31, m12, m22, m32));
}

/* 
 *   adjoint( original_matrix, inverse_matrix )
 * 
 *     calculate the adjoint of a 4x4 matrix
 *
 *      Let  a   denote the minor determinant of matrix A obtained by
 *           ij
 *
 *      deleting the ith row and jth column from A.
 *
 *                    i+j
 *     Let  b   = (-1)    a
 *          ij            ji
 *
 *    The matrix B = (b  ) is the adjoint of A
 *                     ij
 */

void
adjoint_m4f (m4f const *m_, m4f *m0_)
{
  float const (*m)[4] = M(*m_);
  float (*m0)[4] = M(*m0_);

  double m00, m10, m20, m30, m01, m11, m21, m31;
  double m02, m12, m22, m32, m03, m13, m23, m33;

  /* assign to individual variable names to aid  */
  /* selecting correct values  */

  m00 = m[0][0]; m01 = m[0][1]; 
  m02 = m[0][2]; m03 = m[0][3];

  m10 = m[1][0]; m11 = m[1][1];
  m12 = m[1][2]; m13 = m[1][3];

  m20 = m[2][0]; m21 = m[2][1];
  m22 = m[2][2]; m23 = m[2][3];

  m30 = m[3][0]; m31 = m[3][1]; 
  m32 = m[3][2]; m33 = m[3][3];

  /* row column labeling reversed since we transpose rows & columns */
  m0[0][0]  =   det3x3 (m11, m21, m31, m12, m22, m32, m13, m23, m33);
  m0[1][0]  = - det3x3 (m10, m20, m30, m12, m22, m32, m13, m23, m33);
  m0[2][0]  =   det3x3 (m10, m20, m30, m11, m21, m31, m13, m23, m33);
  m0[3][0]  = - det3x3 (m10, m20, m30, m11, m21, m31, m12, m22, m32);
        
  m0[0][1]  = - det3x3 (m01, m21, m31, m02, m22, m32, m03, m23, m33);
  m0[1][1]  =   det3x3 (m00, m20, m30, m02, m22, m32, m03, m23, m33);
  m0[2][1]  = - det3x3 (m00, m20, m30, m01, m21, m31, m03, m23, m33);
  m0[3][1]  =   det3x3 (m00, m20, m30, m01, m21, m31, m02, m22, m32);
        
  m0[0][2]  =   det3x3 (m01, m11, m31, m02, m12, m32, m03, m13, m33);
  m0[1][2]  = - det3x3 (m00, m10, m30, m02, m12, m32, m03, m13, m33);
  m0[2][2]  =   det3x3 (m00, m10, m30, m01, m11, m31, m03, m13, m33);
  m0[3][2]  = - det3x3 (m00, m10, m30, m01, m11, m31, m02, m12, m32);
        
  m0[0][3]  = - det3x3 (m01, m11, m21, m02, m12, m22, m03, m13, m23);
  m0[1][3]  =   det3x3 (m00, m10, m20, m02, m12, m22, m03, m13, m23);
  m0[2][3]  = - det3x3 (m00, m10, m20, m01, m11, m21, m03, m13, m23);
  m0[3][3]  =   det3x3 (m00, m10, m20, m01, m11, m21, m02, m12, m22);
}
