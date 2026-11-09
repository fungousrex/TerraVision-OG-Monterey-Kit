/*
 * Name: vec3.h
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML 97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: vec3.h,v 1.11.6.2 2001/05/16 18:26:31 leei Exp $
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

#ifndef _vec3_h_
#define _vec3_h_

#define V(vp) ((vp).v)
#define M(mp) ((mp).m)

typedef struct v2f {
  float v[2];
} v2f;

typedef struct v3f {
  float v[3];
} v3f;

typedef struct v4f {
  float v[4];
} v4f;

typedef struct v4i {
  int v[4];
} v4i;

typedef struct m3f {
  float m[3][3];
} m3f;

typedef struct m4f {
  float m[4][4];
} m4f;

v2f * set_v2f (float v0, float v1, v2f *v);
v3f * set_v3f (float v0, float v1, float v2, v3f *v);
v4f * set_v4f (float v0, float v1, float v2, float v3, v4f *v);

v2f * add_v2f (v2f const *v0_, v2f const *v1, v2f *v);
v3f * add_v3f (v3f const *v0_, v3f const *v1, v3f *v);
v4f * add_v4f (v4f const *v0_, v4f const *v1, v4f *v);

v2f * sub_v2f (v2f const *v0_, v2f const *v1, v2f *v);
v3f * sub_v3f (v3f const *v0_, v3f const *v1, v3f *v);
v4f * sub_v4f (v4f const *v0_, v4f const *v1, v4f *v);

v2f * scale_v2f (float scl, v2f const *v0_, v2f *v);
v3f * scale_v3f (float scl, v3f const *v0_, v3f *v);
v4f * scale_v4f (float scl, v4f const *v0_, v4f *v);

float inner_v2f (v2f const *, v2f const *);
float inner_v3f (v3f const *, v3f const *);
float inner_v4f_v3f (v4f const *, v3f const *);
float inner_v4f (v4f const *, v4f const *);

float length_v3f (v3f const *);
float length_v4f (v4f const *);

float normalize_v3f (v3f *v_);
float normalize_v4f (v4f *v_);

v3f *cross_v3f (v3f const *, v3f const *, v3f *);

v3f *mult_m3f_v3f (m3f const *, v3f const *, v3f *);
v4f *mult_m4f_v3f (m4f const *, v3f const *, v4f *);
v4f *mult_m4f_v4f (m4f const *, v4f const *, v4f *);
v3f *mult_v3f_m3f (v3f const *, m3f const *, v3f *);
v3f *mult_v3f_m4f (v3f const *, m4f const *, v3f *);
v4f *mult_v4f_m4f (v4f const *, m4f const *, v4f *);

m3f *mult_m3f_m3f (m3f const *, m3f const *, m3f *);
m3f *mult_m3fT_m3f (m3f const *, m3f const *, m3f *);
m3f *mult_m3f_m3fT (m3f const *, m3f const *, m3f *);
m4f *mult_m4f_m4f (m4f const *, m4f const *, m4f *);
m4f *mult_m4fT_m4f (m4f const *, m4f const *, m4f *);
m4f *mult_m3f_m4f (m3f const *, m4f const *, m4f *);
m4f *mult_m3fT_m4f (m3f const *, m4f const *, m4f *);

v3f *proj_v4f (v4f const *, v3f *);
v3f *proj_m4f_v3f (m4f const *, v3f const *, v3f *);

void identity_m4f (m4f *);
void translation_m4f (v3f const *, m4f *);
void translate_m4f (m4f const *, v3f const *, m4f *);
void scaling_m4f (v3f const *, m4f *);
void scale_m4f (m4f const *, v3f const *, m4f *);
void rotation_m4f (v4f const *, m4f *);
void rotation_m3f (v4f const *, m3f *);
void rotate_m4f (m4f const *, v4f const *, m4f *);
void perspective_m4f (v4f const *, m4f *);

void glRotation_m3f (float angle, float x, float y, float z, m3f *);
void glRotation_m4f (float angle, float x, float y, float z, m4f *);

void glTranslatef_m4f (float x, float y, float z, m4f *);
void glScalef_m4f (float x, float y, float z, m4f *);
void glRotatef_m4f (float angle, float x, float y, float z, m4f *);
void glRotatef_m3f (float angle, float x, float y, float z, m3f *);

void print_m3f (m3f const *m_);
void print_m4f (m4f const *m_);

/* Functions in invert.c */

double det2x2 (double m00, double m10,
	       double m01, double m11);
double det3x3 (double m00, double m10, double m20,
	       double m01, double m11, double m21,
	       double m02, double m12, double m22);

int inverse_m4f (m4f const *, m4f *);
void adjoint_m4f (m4f const *, m4f *);

double det_m3f (m3f const *);
double det_m4f (m4f const *);

/* Functions in lu.c */

int ludecomp_m4f (m4f const *A, int n, m4f *LU, v4i *idx);
int lusub_m4f (m4f const *LU, v4i const *idx, int n, v4f const *b, v4f *x);
int lusolve_m4f_v4f (m4f const *A, int n, v4f const *b, v4f *x);

/* Functions in decomp.c */
int decompose_m4f (m4f const *M0, v4f *P, v3f *T, v4f *R, v3f *S);

#endif /* _vec3_h_ */
