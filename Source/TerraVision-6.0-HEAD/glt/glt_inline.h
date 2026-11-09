/*
 * Name: GLT_INLINE.H
 *
 * Description: Inlined versions of OpenGL helper functions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:57 $
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

#include "gltI.h"

exinline char const *
gltOGLVersion (void)
{
  if (! gltCurrVTable->glVersion)
    gltOGLGetVersion ();
  return gltCurrVTable->glVersion;
}

exinline float
gltOGLVersionNum (void)
{
  gltOGLVersion ();
  return gltCurrVTable->glVersionNum;
}

exinline char const *
gltOGLRenderer (void)
{
  gltOGLVersion ();
  return gltCurrVTable->glRenderer;
}

exinline char const *
gltOGLExtensions (void)
{
  gltOGLVersion ();
  return gltCurrVTable->glExts;
}

exinline void 
gltGenTextures (GLsizei n, GLuint *textures)
{
  (*gltCurrVTable->genTextures) (n, textures);
}

exinline void
gltBeginTexture (GLenum which_tex, GLuint id)
{
  (*gltCurrVTable->beginTexture) (which_tex, id);
}

exinline void
gltEndTexture (void)
{
  (*gltCurrVTable->endTexture) ();
}

exinline void
gltDeleteTextures (GLsizei n, const GLuint *textures)
{
  (*gltCurrVTable->deleteTextures) (n, textures);
}

exinline void
gltBindTexture (GLenum which_tex, GLuint id)
{
  (*gltCurrVTable->bindTexture) (which_tex, id);
}

exinline void 
gltLightModelDefv (gltParamDef *def)
{
  for ( ; def->pname; ++def )
    glLightModelfv (def->pname, def->param);
}

exinline void 
gltLightDefv (GLenum light, gltParamDef *def)
{
  for ( ; def->pname; ++def )
    glLightfv (light, def->pname, def->param);
}

exinline void 
gltMaterialDefv (GLenum face, gltParamDef *def)
{
  for ( ; def->pname; ++def )
    glMaterialfv (face, def->pname, def->param);
}

exinline void 
gltTexParameterDefv (GLenum target, gltParamDef *def)
{
  for ( ; def->pname; ++def )
    glTexParameterfv (target, def->pname, def->param);
}

exinline void
gltEnableCheckErrors (int enable)
{
  gltCheckErrorsEnabled = enable;
}

exinline void
gltCheckErrors (char const *funcName)
{
  if (gltCheckErrorsEnabled) {
    GLenum err;
    while ((err = glGetError ()) != GL_NO_ERROR)
      pthr_error1 (funcName, "%d %s", err, gluErrorString (err));
  }
}

exinline GLuint
power2size (GLuint x)
{
  GLuint v = 1;
  while (v < x) v <<= 1;
  return v;
}

exinline GLuint
gltTextureSize (GLuint x)
{
  GLuint p2 = power2size (x);
  return (p2 < 512 ? p2 : 512);
}
