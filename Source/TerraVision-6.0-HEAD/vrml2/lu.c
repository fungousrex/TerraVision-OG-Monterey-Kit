/*
 * Name: lu.c
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
 *   $Id: lu.c,v 1.7 2000/08/28 23:21:37 reddy Exp $
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

#include "vec3.h"
#include <math.h>

#define TINY 1e-20

int
ludecomp_m4f (m4f const *A_, int n, m4f *LU_, v4i *pivot_)
{
  float (*LU)[4] = M(*LU_);
  int *pivot = V(*pivot_);

  int	i, j, k, k_max;
  v4f scale_;
  float *scale = V(scale_);

  if (A_ != LU_)
    *LU_ = *A_;

  /* initialise pivot with identity permutation */
  for (i = 0; i<  n; ++i )
    pivot[i] = i;

  /* set scale parameters */
  for (i=0; i<n; i++) {
    float max1 = 0.0;
    for ( j=0; j<n; j++ ) {
      float temp = fabs (LU[i][j]);
      if (temp > max1) max1 = temp;
    }
    scale[i] = max1;
  }

  /* main loop */
  k_max = n-1;
  for ( k=0; k<k_max; k++ ) {
    /* find best pivot row */
    float max1 = 0.0;
    int i_max = -1;
    for ( i=k; i<n; i++ ) {
      if ( fabs(scale[i]) >= TINY*fabs(LU[i][k]) ) {
	float temp = fabs(LU[i][k])/scale[i];
	if ( temp > max1 ){
	  max1 = temp;
	  i_max = i;
	}
      } else {
	return 0;
      }
    }

    /* if no pivot then ignore column k... */
    if ( i_max == -1 ) {
      /* set pivot entry LU[k][k] exactly to zero,
	 rather than just "small" */
      LU[k][k] = 0.0;
      continue;
    }
	    
    /* do we pivot ? */
    if ( i_max != k ) {
      /* yes we do... */
      int temp = pivot[i_max];
      pivot[i_max] = pivot[k];
      pivot[k] = temp;

      for ( j=0; j<n; j++ ) {
	float temp = LU[i_max][j];
	LU[i_max][j] = LU[k][j];
	LU[k][j] = temp;
      }
    }
	    
    /* row operations */
    for (i = k+1; i < n; i++) {
      /* for each row do... */
      float temp = LU[i][k] = LU[i][k]/LU[k][k];
      for ( j = k+1; j < n; j++ )
	LU[i][j] -= temp*LU[k][j];
    }
  }

  return 1;
}


int
lusub_m4f (m4f const *LU_, v4i const *pivot_, int n, v4f const *b_, v4f *x_)
{
  float const (*LU)[4] = M(*LU_);
  int const *pivot = V(*pivot_);
  float const *b = V(*b_);
  float *x = V(*x_);
  int i, i_lim, j;

  for (i = 0; i < n; ++i )
    x[i] = b[pivot[i]];

  for ( i=0; i<n; i++ )
    if ( x[i] != 0.0 )
      break;
  i_lim = i;

  for ( ; i < n; ++i ) {
    float sum = x[i];
    float const *mat_row = &(LU[i][i_lim]);
    float *out_col = &(x[i_lim]);
    for ( j = i_lim; j < i; ++j )
      sum -= LU[i][j]*x[j];
    sum -= (*mat_row++)*(*out_col++);
    x[i] = sum;
  }

  for ( i=n-1; i>=0; i-- )
    if ( x[i] != 0.0 )
      break;
  i_lim = i;

  for (    ; i>=0; i-- ) {
    float sum = x[i];
    float const *mat_row = &(LU[i][i+1]);
    float *out_col = &(x[i+1]);
    for ( j=i+1; j<=i_lim; j++ )
      sum -= LU[i][j]*x[j];
    sum -= (*mat_row++)*(*out_col++);
    if ( fabs(LU[i][i]) <= TINY*fabs(sum) )
      return 0;
    else
      x[i] = sum/LU[i][i];
  }

  return 1;
}

int
lusolve_m4f_v4f (m4f const *m_, int n, v4f const *b, v4f *x)
{
  m4f LU;
  v4i idx;

  if (ludecomp_m4f (m_, n, &LU, &idx) != 0) {
    lusub_m4f (&LU, &idx, n, b, x);
    return 1;
  }
  return 0;
}
