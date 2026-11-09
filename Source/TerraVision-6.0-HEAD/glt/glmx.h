/*
 * Name: GLMX.H
 *
 * Description: Common X11 glmContext implementation.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:56 $
 *     $Author: reddy $
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#ifndef _glmx_h_
#define _glmx_h_

#include "glm.h"
#include "glmimpl.h"

extern glmContext xCurrentContext;
extern glmContextVTable xContextVTable;

typedef struct {
  GLXContext glxCtx;
  Display *disp;
  GLXDrawable drawable;
  XVisualInfo vinfo;

  gltVTable *vtable;
  gltVTable vtable_s;

  GLuint state;
} xContextData;

extern glmContext xShareContext (glmContext share);
extern void xDestroyContext (glmContext ctx);

#endif /* _glmx_h_ */


