/*
 * Name: quat.h
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
 *   $Id: quat.h,v 1.3 2000/08/28 23:21:40 reddy Exp $
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

#ifndef _quat_h_
#define _quat_h_

#include "vec3.h"

#define Q(Qp) ((Qp).q)

typedef struct quat {
  float q[4];
} quat;


quat *set_quat (float q0, float q1, float q2, float q3, quat *q);
quat *normalize_quat(quat *q);
quat *add_quat (quat const *q1, quat const *q2, quat *out);

quat *rot_v3f_quat (v3f const *axis, float phi, quat *q);
quat *rotation_quat (float a0, float a1, float a2, float phi, quat *q);
float quat_angle (quat const *q, v3f *axis);

m4f *quat_m4f (quat const *q, m4f *m);

quat *trackball (float p1x, float p1y, float p2x, float p2y, quat *q);

#endif /* _quat_h_ */
