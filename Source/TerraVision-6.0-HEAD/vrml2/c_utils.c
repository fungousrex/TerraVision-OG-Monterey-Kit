/*
 * Name: c_utils.c
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
 *   $Id: c_utils.c,v 1.4 2000/08/28 23:21:33 reddy Exp $
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

#include "c_nodesI.h"

int
vsgcValuesEqual (vsgDataType type, void const *v1p, void const *v2p)
{
  if (v1p == v2p)
    return 1;

  switch (type) {
  case vsgSFColor: {
    SFColor const *v1 = v1p, *v2 = v2p;
    return v1->r == v2->r && v1->g == v2->g && v1->b == v2->b;
  }
  case vsgSFImage: {
    SFImage const *v1 = v1p, *v2 = v2p;
    return (v1->width == v2->width
	    && v1->height == v2->height
	    && v1->depth == v2->depth
	    && v1->data == v2->data);
  }
  case vsgSFFloat: {
    SFFloat v1 = *(SFFloat const *)v1p, v2 = *(SFFloat const *)v2p;
    return v1 == v2;
  }
  case vsgSFRotation: {
    SFRotation const *v1 = v1p, *v2 = v2p;
    return (v1->a == v2->a && v1->b == v2->b
	    && v1->c == v2->c && v1->d == v2->d);
  }
  case vsgSFTime: {
    SFTime v1 = *(SFTime const *)v1p, v2 = *(SFTime const *)v2p;
    return v1 == v2;
  }
  case vsgSFVec2f: {
    SFVec2f const *v1 = v1p, *v2 = v2p;
    return (v1->v[0] == v2->v[0] && v1->v[1] == v2->v[1]);
  }
  case vsgSFVec3f: {
    SFVec3f const *v1 = v1p, *v2 = v2p;
    return (v1->v[0] == v2->v[0]
	    && v1->v[1] == v2->v[1]
	    && v1->v[2] == v2->v[2]);
  }
  default:
    return 0;
  }
}
