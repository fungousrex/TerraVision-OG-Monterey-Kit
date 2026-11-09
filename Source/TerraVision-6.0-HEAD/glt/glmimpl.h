/*
 * Name: GLMIMPL.H
 *
 * Description: OpenGL/window system layer basic implementation
 *    data structures..
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

#ifndef _GLMIMPL_H_
#define _GLMIMPL_H_

typedef struct glmContextVTable {
  glmContext (*createShared) (glmContext);
  void (*destroy) (glmContext);
} glmContextVTable;

struct _glmContext {
  glmContextVTable *vtable;
  void *data;

  // Attributes
  glmContext share;
  glmContextProperties props;
  int doubleBuffer;
  int stereo;
};

typedef struct glmWindowVTable {
  void * (*getWidget) (glmWindow);
  
  glmRedrawFunc (*registerRedraw) (glmWindow,
				   glmRedrawFunc redraw, void *closure);
  void (*removeRedraw) (glmWindow);
  void (*postRedraw) (glmWindow);
  int (*pendingRedraw) (glmWindow);

  void (*registerMapNotify) (glmWindow,
			     glmMapNotifyFunc notify, void *closure);
  int (*isMapped) (glmWindow);

  glmContext (*makeCurrent) (glmWindow, char const *);
  void (*release) (glmWindow, char const *);

  void (*addTimer) (glmWindow, void **id, unsigned int msecs,
		    void (*f) (void *closure), void *closure);
  void (*removeTimer) (glmWindow, void **id);

  void (*charstr) (glmWindow, int fidx, char const *str);

  void (*destroy) (glmWindow);
} glmWindowVTable;

struct _glmWindow {
  glmWindowVTable *vtable;
  void *data;

  // Attributes
  glmContext ctx;
  int doubleBuffer;
  int stereo;
  int noflush;
};

#endif /* _GLMIMPL_H_ */
