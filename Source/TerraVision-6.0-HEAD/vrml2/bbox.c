/*
 * Name: bbox.c
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
 *   $Id: bbox.c,v 1.2 2000/08/28 23:21:29 reddy Exp $
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

#if defined(WIN32)
#pragma warning (disable:4244)          /* disable bogus conversion warnings */
#endif

#include "bbox.h"

bbox3f *
bbox_point (v3f const *v, bbox3f *b)
{
  float *min = b->min, *max = b->max;
  int i;
  for (i = 0; i < 3; ++i)
    min[i] = max[i] = V(*v)[i];
  return b;
}

bbox3f *
bbox_add_point (v3f const *v, bbox3f *b)
{
  float *min = b->min, *max = b->max;
  int i;
  for (i = 0; i < 3; ++i) {
    float val = V(*v)[i];
    if (val < min[i]) min[i] = val;
    else if (val > max[i]) max[i] = val;
  }
  return b;
}

/* Transform by the view matrix.
   Fast BBox transform taken from Graphics Gems: Transforming Axis-Aligned
   Bounding Boxes pp. 548-550. */

bbox3f *
bbox_m4f (m4f const *M_, bbox3f const *b0, bbox3f *b1)
{
  float const (* const M)[4] = M(*M_);
  float Tx = M[3][0], Ty = M[3][1], Tz = M[3][2];
  float const *min = b0->min, *max = b0->max;
  float x0, x1, y0, y1, z0, z1;
  int j;

  x0 = x1 = Tx;
  for (j = 0; j < 3; ++j) {
    float a = M[j][0] * min[j];
    float b = M[j][0] * max[j];
    if (a < b)
      x0 += a, x1 += b;
    else
      x0 += b, x1 += a;
  }
  b1->min[0] = x0; b1->max[0] = x1;

  y0 = y1 = Ty;
  for (j = 0; j < 3; ++j) {
    float a = M[j][1] * min[j];
    float b = M[j][1] * max[j];
    if (a < b)
      y0 += a, y1 += b;
    else
      y0 += b, y1 += a;
  }
  b1->min[1] = y0; b1->max[1] = y1;

  z0 = z1 = Tz;
  for (j = 0; j < 3; ++j) {
    float a = M[j][2] * min[j];
    float b = M[j][2] * max[j];
    if (a < b)
      z0 += a, z1 += b;
    else
      z0 += b, z1 += a;
  }
  b1->min[2] = z0; b1->max[2] = z1;

  return b1;
}

