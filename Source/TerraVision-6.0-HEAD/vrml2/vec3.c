/*
 * Name:  vec3.c
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
 *   $Id: vec3.c,v 1.13.6.2 2001/05/16 18:26:30 leei Exp $
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
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.14159262535)
#endif

v2f *
set_v2f (float v0, float v1, v2f *v)
{
  V(*v)[0] = v0;
  V(*v)[1] = v1;
  return v;
}

v3f *
set_v3f (float v0, float v1, float v2, v3f *v)
{
  V(*v)[0] = v0;
  V(*v)[1] = v1;
  V(*v)[2] = v2;
  return v;
}

v4f *
set_v4f (float v0, float v1, float v2, float v3, v4f *v)
{
  V(*v)[0] = v0;
  V(*v)[1] = v1;
  V(*v)[2] = v2;
  V(*v)[3] = v3;
  return v;
}

v2f *
add_v2f (v2f const *v0_, v2f const *v1_, v2f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] + v1[0];
  V(*v)[1] = v0[1] + v1[1];
  return v;
}

v3f *
add_v3f (v3f const *v0_, v3f const *v1_, v3f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] + v1[0];
  V(*v)[1] = v0[1] + v1[1];
  V(*v)[2] = v0[2] + v1[2];
  return v;
}

v4f *
add_v4f (v4f const *v0_, v4f const *v1_, v4f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] + v1[0];
  V(*v)[1] = v0[1] + v1[1];
  V(*v)[2] = v0[2] + v1[2];
  V(*v)[3] = v0[3] + v1[3];
  return v;
}

v2f *
sub_v2f (v2f const *v0_, v2f const *v1_, v2f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] - v1[0];
  V(*v)[1] = v0[1] - v1[1];
  return v;
}

v3f *
sub_v3f (v3f const *v0_, v3f const *v1_, v3f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] - v1[0];
  V(*v)[1] = v0[1] - v1[1];
  V(*v)[2] = v0[2] - v1[2];
  return v;
}

v4f *
sub_v4f (v4f const *v0_, v4f const *v1_, v4f *v)
{
  float const *v0 = V(*v0_);
  float const *v1 = V(*v1_);
  V(*v)[0] = v0[0] - v1[0];
  V(*v)[1] = v0[1] - v1[1];
  V(*v)[2] = v0[2] - v1[2];
  V(*v)[3] = v0[3] - v1[3];
  return v;
}

v2f *
scale_v2f (float scl, v2f const *v0_, v2f *v)
{
  float const *v0 = V(*v0_);
  V(*v)[0] = scl * v0[0];
  V(*v)[1] = scl * v0[1];
  return v;
}

v3f *
scale_v3f (float scl, v3f const *v0_, v3f *v)
{
  float const *v0 = V(*v0_);
  V(*v)[0] = scl * v0[0];
  V(*v)[1] = scl * v0[1];
  V(*v)[2] = scl * v0[2];
  return v;
}

v4f *
scale_v4f (float scl, v4f const *v0_, v4f *v)
{
  float const *v0 = V(*v0_);
  V(*v)[0] = scl * v0[0];
  V(*v)[1] = scl * v0[1];
  V(*v)[2] = scl * v0[2];
  V(*v)[3] = scl * v0[3];
  return v;
}

float
inner_v2f (v2f const *v1_, v2f const *v2_)
{
  float const *v1 = V(*v1_), *v2 = V(*v2_);
  return v1[0]*v2[0] + v1[1]*v2[1];
}

float
inner_v3f (v3f const *v1_, v3f const *v2_)
{
  float const *v1 = V(*v1_), *v2 = V(*v2_);
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

float 
inner_v4f_v3f (v4f const *v1_, v3f const *v2_)
{
  float const *v1 = V(*v1_), *v2 = V(*v2_);
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3];
}

float 
inner_v4f (v4f const *v1_, v4f const *v2_)
{
  float const *v1 = V(*v1_), *v2 = V(*v2_);
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3];
}

float
length_v3f (v3f const *v_)
{
  float norm = inner_v3f (v_, v_);
  return sqrt (norm);
}

float
length_v4f (v4f const *v_)
{
  float norm = inner_v4f (v_, v_);
  return sqrt (norm);
}

float
normalize_v3f (v3f *v_)
{
  float norm = length_v3f (v_);
  int i;

  for (i = 0; i < 3; ++i)
    V(*v_)[i] /= norm;
  return norm;
}

float
normalize_v4f (v4f *v_)
{
  float norm = length_v4f (v_);
  int i;

  for (i = 0; i < 4; ++i)
    V(*v_)[i] /= norm;
  return norm;
}

v3f *
cross_v3f (v3f const *v1_, v3f const *v2_, v3f *v0_)
{
  float const *v1 = V(*v1_), *v2 = V(*v2_);
  float *v0 = V(*v0_);

  v0[0] = (v1[1]*v2[2] - v1[2]*v2[1]);
  v0[1] = (v1[2]*v2[0] - v1[0]*v2[2]);
  v0[2] = (v1[0]*v2[1] - v1[1]*v2[0]);

  return v0_;
}

v3f *
mult_m3f_v3f (m3f const *m_, v3f const *v_, v3f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;

  for (i = 0; i < 3; i++) {
    float const (*m)[3] = (float const (*)[3]) &M(*m_)[0][i];
    *v0p++ = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2];
  }

  return v0_;
}

v4f *
mult_m4f_v3f (m4f const *m_, v3f const *v_, v4f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;
  
  for (i = 0; i < 4; i++) {
    float const (*m)[4] = (float const (*)[4]) &M(*m_)[0][i];
    *v0p++ = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0];
  }

  return v0_;
}

v4f *
mult_m4f_v4f (m4f const *m_, v4f const *v_, v4f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;
  
  for (i = 0; i < 4; i++) {
    float const (*m)[4] = (float const (*)[4]) &M(*m_)[0][i];
    *v0p++ = m[0][0] * v[0] + m[1][0] * v[1] + m[2][0] * v[2] + m[3][0] * v[3];
  }

  return v0_;
}

v4f *
mult_v4f_m4f (v4f const *v_, m4f const *m_, v4f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;

  for (i = 0; i < 4; i++) {
    float const (*m)[4] = (float const (*)[4]) &M(*m_)[i][0];
    *v0p++ = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2] + v[3] * m[0][3];
  }

  return v0_;
}

v3f *
mult_v3f_m3f (v3f const *v_, m3f const *m_, v3f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;

  for (i = 0; i < 3; i++) {
    float const (*m)[3] = (float const (*)[3]) &M(*m_)[i][0];
    *v0p++ = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2];
  }

  return v0_;
}

v3f *
mult_v3f_m4f (v3f const *v_, m4f const *m_, v3f *v0_)
{
  float const *v = V(*v_);
  float *v0p = V(*v0_);
  int i;

  for (i = 0; i < 3; i++) {
    float const (*m)[4] = (float const (*)[4]) &M(*m_)[i][0];
    *v0p++ = v[0] * m[0][0] + v[1] * m[0][1] + v[2] * m[0][2];
  }

  return v0_;
}

m3f *
mult_m3f_m3f (m3f const *m1_, m3f const *m2_, m3f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 3; j++) {
    float const (*m2)[3] = (float const (*)[3]) &M(*m2_)[j][0];
    for (i = 0; i < 3; i++) {
      float const (*m1)[3] = (float const (*)[3]) &M(*m1_)[0][i];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[1][0] * m2[0][1]
		+ m1[2][0] * m2[0][2]);
    }
  }

  return m0_;
}

m3f *
mult_m3fT_m3f (m3f const *m1_, m3f const *m2_, m3f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 3; j++) {
    float const (*m2)[3] = (float const (*)[3]) &M(*m2_)[j][0];
    for (i = 0; i < 3; i++) {
      float const (*m1)[3] = (float const (*)[3]) &M(*m1_)[i][0];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[0][1] * m2[0][1]
		+ m1[0][2] * m2[0][2]);
    }
  }

  return m0_;
}

m3f *
mult_m3f_m3fT (m3f const *m1_, m3f const *m2_, m3f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 3; j++) {
    float const (*m2)[3] = (float const (*)[3]) &M(*m2_)[0][j];
    for (i = 0; i < 3; i++) {
      float const (*m1)[3] = (float const (*)[3]) &M(*m1_)[0][i];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[1][0] * m2[1][0]
		+ m1[2][0] * m2[2][0]);
    }
  }

  return m0_;
}

m4f *
mult_m4f_m4f (m4f const *m1_, m4f const *m2_, m4f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 4; j++) {
    float const (*m2)[4] = (float const (*)[4]) &M(*m2_)[j][0];
    for (i = 0; i < 4; i++) {
      float const (*m1)[4] = (float const (*)[4]) &M(*m1_)[0][i];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[1][0] * m2[0][1]
		+ m1[2][0] * m2[0][2]
		+ m1[3][0] * m2[0][3]);
    }
  }

  return m0_;
}

m4f *
mult_m4fT_m4f (m4f const *m1_, m4f const *m2_, m4f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 4; j++) {
    float const (*m2)[4] = (float const (*)[4]) &M(*m2_)[j][0];
    for (i = 0; i < 4; i++) {
      float const (*m1)[4] = (float const (*)[4]) &M(*m1_)[i][0];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[0][1] * m2[0][1]
		+ m1[0][2] * m2[0][2]
		+ m1[0][3] * m2[0][3]);
    }
  }

  return m0_;
}

m4f *
mult_m3f_m4f (m3f const *m1_, m4f const *m2_, m4f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 4; j++) {
    float const (*m2)[4] = (float const (*)[4]) &M(*m2_)[j][0];
    for (i = 0; i < 3; i++) {
      float const (*m1)[3] = (float const (*)[3]) &M(*m1_)[0][i];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[1][0] * m2[0][1]
		+ m1[2][0] * m2[0][2]);
    }
    *m0p++ = m2[0][3];
  }

  return m0_;
}

m4f *
mult_m3fT_m4f (m3f const *m1_, m4f const *m2_, m4f *m0_)
{
  float *m0p = M(*m0_)[0];
  int i, j;

  for (j = 0; j < 4; j++) {
    float const (*m2)[4] = (float const (*)[4]) &M(*m2_)[j][0];
    for (i = 0; i < 3; i++) {
      float const (*m1)[3] = (float const (*)[3]) &M(*m1_)[i][0];
      *m0p++ = (m1[0][0] * m2[0][0]
		+ m1[0][1] * m2[0][1]
		+ m1[0][2] * m2[0][2]);
    }
    *m0p++ = m2[0][3];
  }

  return m0_;
}

v3f *
proj_v4f (v4f const *v1_, v3f *v0_)
{
  float const *v1 = V(*v1_);
  float *v0 = V(*v0_);
  float w = v1[3];
  int i;

  for (i = 0; i < 3; ++i)
    v0[i] = v1[i]/w;

  return v0_;
}

v3f *
proj_m4f_v3f (m4f const *m_, v3f const *v_, v3f *v0_)
{
  v4f p;
  mult_m4f_v3f (m_, v_, &p);
  proj_v4f (&p, v0_);
  return v0_;
}


void
identity_m4f (m4f *m_)
{
  float (*m)[4] = M(*m_);
  int i;
  memset (&m[0][0], 0, 16*sizeof(m[0][0]));
  for (i = 0; i < 4; ++i)
    m[i][i] = 1;
}

void
translation_m4f (v3f const *T_, m4f *M_)
{
  float const *T = V(*T_);
  float (*M)[4] = M(*M_);
  int i;

  identity_m4f (M_);
  for (i = 0; i < 3; ++i)
    M[3][i] = T[i];
}

void
translate_m4f (m4f const *M0_, v3f const *T_, m4f *M_)
{
  float const *T = V(*T_);
  float const (* const M0)[4] = M(*M0_);
  float (*M)[4] = M(*M_);
  int i;

  memcpy (M_, M0_, sizeof (*M_));
  for (i = 0; i < 3; ++i) {
    float d = M0[3][i];
    d += M0[0][i] * T[0];
    d += M0[1][i] * T[1];
    d += M0[2][i] * T[2];
    M[3][i] = d;
  }
}

void
glTranslatef_m4f (float x, float y, float z, m4f *m)
{
  v3f T;
  V(T)[0] = x; V(T)[1] = y; V(T)[2] = z; 
  translate_m4f (m, &T, m);
}

#define ROTATION(M,x,y,z,c,s)			\
{						\
  float v = 1.0-(c);				\
						\
  M[0][0] = x*x*v + c;				\
  M[0][1] = x*y*v + z*s;			\
  M[0][2] = x*z*v - y*s;			\
						\
  M[1][0] = y*x*v - z*s;			\
  M[1][1] = y*y*v + c;				\
  M[1][2] = y*z*v + x*s;			\
						\
  M[2][0] = z*x*v + y*s;			\
  M[2][1] = z*y*v - x*s;			\
  M[2][2] = z*z*v + c;				\
}

void
rotation_m3f (v4f const *R_, m3f *M_)
{
  float const *R = V(*R_);
  float (*M)[3] = M(*M_);
  float theta = R[3];
  float x, y, z, c, s;

  normalize_v3f ((v3f *) R_);
  x = R[0]; y = R[1]; z = R[2];
  c = cos (theta);
  s = sin (theta);

  ROTATION (M, x,y,z, c,s);
}

void
rotation_m4f (v4f const *R_, m4f *M_)
{
  float const *R = V(*R_);
  float (*M)[4] = M(*M_);
  float theta = R[3];
  float x, y, z, c, s;

  normalize_v3f ((v3f *) R_);
  x = R[0]; y = R[1]; z = R[2];
  c = cos (theta);
  s = sin (theta);

  ROTATION (M, x,y,z, c,s);

  M[0][3] = 0;
  M[1][3] = 0;
  M[2][3] = 0;

  M[3][0] = 0.0;
  M[3][1] = 0.0;
  M[3][2] = 0.0;
  M[3][3] = 1.0;

#if 0
  printf ("rotation_m4f [%.2f, %.2f, %.2f, %.2f]:\n", x, y, z, theta);
  print_m4f (M_);
#endif
}

void 
rotate_m3f (m3f const *M0_, v4f const *R_, m3f *M_)
{
  m3f M1_;
  m3f Rot;

  /* Protect against in-place rotation */
  if (M0_ == M_) {
    M1_ = *M0_;
    M0_ = &M1_;
  }

  rotation_m3f (R_, &Rot);
  mult_m3f_m3f (M0_, &Rot, M_);
}

void 
rotate_m4f (m4f const *M0_, v4f const *R_, m4f *M_)
{
  m4f M1_;
  m4f Rot;

  /* Protect against in-place rotation */
  if (M0_ == M_) {
    M1_ = *M0_;
    M0_ = &M1_;
  }

  rotation_m4f (R_, &Rot);
  mult_m4f_m4f (M0_, &Rot, M_);
}

void
glRotation_m4f (float angle, float x, float y, float z, m4f *m)
{
  v4f R;
  V(R)[0] = x; V(R)[1] = y; V(R)[2] = z; V(R)[3] = angle * (M_PI/180.0);
  rotation_m4f (&R, m);
}

void
glRotation_m3f (float angle, float x, float y, float z, m3f *m)
{
  v4f R;
  V(R)[0] = x; V(R)[1] = y; V(R)[2] = z; V(R)[3] = angle * (M_PI/180.0);
  rotation_m3f (&R, m);
}

void
glRotatef_m4f (float angle, float x, float y, float z, m4f *m)
{
  v4f R;
  V(R)[0] = x; V(R)[1] = y; V(R)[2] = z; V(R)[3] = angle * (M_PI/180.0);
  rotate_m4f (m, &R, m);
}

void
glRotatef_m3f (float angle, float x, float y, float z, m3f *m)
{
  v4f R;
  V(R)[0] = x; V(R)[1] = y; V(R)[2] = z; V(R)[3] = angle * (M_PI/180.0);
  rotate_m3f (m, &R, m);
}

void
scaling_m4f (v3f const *S_, m4f *M_)
{
  float const *S = V(*S_);
  float (*M)[4] = M(*M_);
  int i;

  memset (M, 0, sizeof (m4f));
  for (i = 0; i < 3; ++i)
    M[i][i] = S[i];
  M[3][3] = 1.0;
}

void
scale_m4f (m4f const *M0_, v3f const *S_, m4f *M_)
{
  float const *S = V(*S_);
  float const (* const M0)[4] = M(*M0_);
  float (*M)[4] = M(*M_);
  int i, j;

  for (i = 0; i < 3; ++i) {
    float d = S[i];
    for (j = 0; j < 3; ++j)
      M[j][i] = d*M0[j][i];
    M[3][i] = M0[3][i];
  }
  M[3][3] = M0[3][3];
}

void 
glScalef_m4f (float x, float y, float z, m4f *m)
{
  v3f S;
  V(S)[0] = x; V(S)[1] = y; V(S)[2] = z; 
  scale_m4f (m, &S, m);
}


#include <stdio.h>

void
print_m3f (m3f const *m_)
{
  float const (*m)[3] = M(*m_);
  int i, j;
  for (i = 0; i < 3; ++i) {
    fputs ("|", stdout);
    for (j = 0; j < 3; ++j)
      printf ("%s%.2f", j>0?" ":"", m[j][i]);
    fputs ("|\n", stdout);
  }
}

void
print_m4f (m4f const *m_)
{
  float const (*m)[4] = M(*m_);
  int i, j;
  for (i = 0; i < 4; ++i) {
    fputs ("|", stdout);
    for (j = 0; j < 4; ++j)
      printf ("%s%.2f", j>0?" ":"", m[j][i]);
    fputs ("|\n", stdout);
  }
}

