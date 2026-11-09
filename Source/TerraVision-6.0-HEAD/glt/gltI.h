/*
 * Name: GLTI.H
 *
 * Description: Internal structure for OpenGL helper functions.
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

#ifndef _gltI_h_
#define _gltI_h_

#include <pthr/report.h>

#ifndef APIENTRY
# define APIENTRY
#endif

typedef struct {
  char const *glVersion;
  float glVersionNum;
  char const *glExts;
  char const *glRenderer;
  
  void (APIENTRY *genTextures) (GLsizei, GLuint *);
  void (APIENTRY *beginTexture) (GLenum which_tex, GLuint id);
  void (*endTexture) (void);
  void (APIENTRY *deleteTextures) (GLsizei n, const GLuint *textures);
  void (APIENTRY *bindTexture) (GLenum which_tex, GLuint id);
} gltVTable;

#ifdef __cplusplus
extern "C" {
#endif

extern gltVTable gltVTableINIT;
extern gltVTable *gltCurrVTable;

extern char const *gltOGLGetVersion (void);

#ifdef __cplusplus
}
#endif

#endif /* _gltI_h_ */
