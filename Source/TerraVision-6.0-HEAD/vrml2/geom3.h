/*
 * Name: geom3.h
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
 *   $Id: geom3.h,v 1.4 2000/08/28 23:21:36 reddy Exp $
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

#ifndef _geom3_h_
#define _geom3_h_

#include "vec3.h"

typedef v3f pt3f;

/* A plane is represented by P in the equation P.x = 0, where x is a
   homogeneous point.   Thus the first three elements in P represent the
   normal vector to P. */ 
typedef v4f plane3f;

void normalize_plane3f (plane3f *P);
float distance_plane3f_pt3f (plane3f const *P, pt3f const *x);

#endif /* _geom3_h_ */
