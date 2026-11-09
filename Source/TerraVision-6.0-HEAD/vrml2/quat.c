/*
 * Name: quat.c
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
 *   $Id: quat.c,v 1.5 2000/08/28 23:21:40 reddy Exp $
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

/*
 * (c) Copyright 1993, 1994, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the above
 * copyright notice appear in all copies and that both the copyright notice
 * and this permission notice appear in supporting documentation, and that
 * the name of Silicon Graphics, Inc. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
 * GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
 * KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
 * LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
 * THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is Silicon Graphics,
 * Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
 *
 * OpenGL(TM) is a trademark of Silicon Graphics, Inc.
 */

/*
 * Trackball code:
 *
 * Implementation of a virtual trackball.
 * Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 *   the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 *
 * Vector manip code:
 *
 * Original code from:
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Much mucking with by:
 * Gavin Bell
 */
#if defined(WIN32)
#pragma warning (disable:4244)          /* disable bogus conversion warnings */
#endif
#include <math.h>
#include "quat.h"

#ifndef M_PI
#define M_PI (3.14159262535)
#endif

/*
 * This size should really be based on the distance from the center of
 * rotation to the point on the object underneath the mouse.  That
 * point would then track the mouse as closely as possible.  This is a
 * simple example, though, so that is left as an Exercise for the
 * Programmer.
 */
#define TRACKBALLSIZE  (0.8)

/*
 * Local function prototypes (not defined in trackball.h)
 */
static float tb_project_to_sphere(float, float, float);

quat *
set_quat (float q0, float q1, float q2, float q3, quat *q)
{
  Q(*q)[0] = q0;
  Q(*q)[1] = q1;
  Q(*q)[2] = q2;
  Q(*q)[3] = q3;
  return q;
}

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic sheet of rotation away from the
 * center.  This particular function was chosen after trying out
 * several variations.
 *
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */
quat *
trackball(float p1x, float p1y, float p2x, float p2y, quat *q)
{
  v3f a;	/* Axis of rotation */
  float phi;  /* how much to rotate about axis */
  v3f p1, p2, d;
  float t;

  if (p1x == p2x && p1y == p2y) {
    /* Zero rotation */
    set_quat(0, 0, 0, 1, q);
    return q;
  }

  /*
   * First, figure out z-coordinates for projection of P1 and P2 to
   * deformed sphere
   */
  set_v3f (p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y), &p1);
  set_v3f (p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y), &p2);

  /*
   *  Now, we want the cross product of P1 and P2
   */
  cross_v3f (&p2, &p1, &a);

  /*
   *  Figure out how much to rotate around that axis.
   */
  sub_v3f (&p1, &p2, &d);
  t = length_v3f (&d) / (2.0*TRACKBALLSIZE);

  /*
   * Avoid problems with out-of-control values...
   */
  if (t > 1.0) t = 1.0;
  if (t < -1.0) t = -1.0;
  phi = 2.0 * asin(t)  * (180/M_PI);

  rot_v3f_quat (&a, phi, q);

  return q;
}

/*
 *  Given an axis and angle, compute quaternion.
 */
quat *
rot_v3f_quat(v3f const *a, float phi, quat *q)
{
  float angle = phi/2.0 * (M_PI/180);
  float len = length_v3f (a);

  scale_v3f (sin(angle)/len, a, (v3f *) q);
  Q(*q)[3] = cos(angle);
  return q;
}

quat *
rotation_quat (float a0, float a1, float a2, float phi, quat *q)
{
  v3f a;
  V(a)[0] = a0; V(a)[1] = a1; V(a)[2] = a2;
  return rot_v3f_quat (&a, phi, q);
}

float
quat_angle (quat const *q, v3f *axis)
{
  float cos_phi_2 = Q(*q)[3];
  float phi_2 = acos (cos_phi_2);
  if (axis) {
    float sin_phi_2 = sin (phi_2);
    scale_v3f (1/sin_phi_2, (v3f *) Q(*q), axis);
  }
  return (180/M_PI) * 2 * phi_2;
}

/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
static float
tb_project_to_sphere(float r, float x, float y)
{
    float d, t, z;

    d = sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440) {    /* Inside sphere */
        z = sqrt(r*r - d*d);
    } else {           /* On hyperbola */
        t = r / 1.41421356237309504880;
        z = t*t / d;
    }
    return z;
}

/*
 * Given two rotations, e1 and e2, expressed as quaternion rotations,
 * figure out the equivalent single rotation and stuff it into dest.
 *
 * This routine also normalizes the result every RENORMCOUNT times it is
 * called, to keep error from creeping in.
 *
 * NOTE: This routine is written so that q1 or q2 may be the same
 * as dest (or each other).
 */

#define RENORMCOUNT 97

quat *
add_quat (quat const *q1, quat const *q2, quat *dest)
{
  static int count=0;
  v3f t1, t2;

  scale_v3f (Q(*q2)[3], (v3f *) q1, &t1);
  scale_v3f (Q(*q1)[3], (v3f *) q2, &t2);
  add_v3f (&t1, &t2, &t1);

  cross_v3f ((v3f *) q2, (v3f *) q1, &t2);
  add_v3f (&t1, &t2, (v3f *) dest);
  Q(*dest)[3] = Q(*q1)[3] * Q(*q2)[3] - inner_v3f ((v3f *) q1, (v3f *) q2);

  if (++count > RENORMCOUNT) {
    count = 0;
    normalize_quat (dest);
  }

  return dest;
}

/*
 * Quaternions always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * If they don't add up to 1.0, dividing by their magnitude will
 * renormalize them.
 *
 * Note: See the following for more information on quaternions:
 *
 * - Shoemake, K., Animating rotation with quaternion curves, Computer
 *   Graphics 19, No 3 (Proc. SIGGRAPH'85), 245-254, 1985.
 * - Pletinckx, D., Quaternion calculus as a basic tool in computer
 *   graphics, The Visual Computer 5, 2-13, 1989.
 */
quat *
normalize_quat(quat *q_)
{
  float *q = Q(*q_);
  int i;
  float mag;

  mag = sqrt (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
  mag = 1/mag;
  for (i = 0; i < 4; i++) q[i] *= mag;
  return q_;
}

/*
 * Build a rotation matrix, given a quaternion rotation.
 *
 */
m4f *
quat_m4f (quat const *q_, m4f * m_)
{
  float const *q = Q(*q_);
  float (*m)[4] = M(*m_);

  m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
  m[1][0] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
  m[2][0] = 2.0 * (q[2] * q[0] + q[1] * q[3]);
  m[3][0] = 0.0;

  m[0][1] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
  m[1][1]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
  m[2][1] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
  m[3][1] = 0.0;

  m[0][2] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
  m[1][2] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
  m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
  m[3][2] = 0.0;

  m[0][3] = 0.0;
  m[1][3] = 0.0;
  m[2][3] = 0.0;
  m[3][3] = 1.0;

  return m_;
}

