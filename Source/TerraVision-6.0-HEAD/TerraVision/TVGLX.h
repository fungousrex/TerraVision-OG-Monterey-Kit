/* -*- tab-width: 2; -*-

  Name:  TerraVisionGLX.h

  Description: Interface between TerraVision and GLX libraries.

  Revision History:
      $Id: TVGLX.h,v 2.13.6.2 2002/10/31 05:26:32 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

 */

#ifndef TVGLX_H
#define TVGLX_H

#include <glt/glm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* We needed this typedef to pacify Doc++, which doesn't like return types
 * of the form "struct Foo myFunc() { }"; several of these kinds of definitions
 * exist in TVGLX.c.  However, there is a circular dependency between this
 * file and ThreeDCoreI.h, where this type is also declared (and referenced by
 * modules that don't include this header).  Its a bit of a mess, so this
 * little kludge is a work around.      --RLB  6/15/2000
 */
#ifndef TEXTURE_H
typedef struct TextureCache TextureCache;
#endif

#ifndef THREEDCOREI_H
typedef struct RenderPrimitiveCache RenderPrimitiveCache;
#endif

typedef struct {
  glmContext const ctx;
  TextureCache * const txCache;
  RenderPrimitiveCache * const rpCache;
} tvGLXContext;

extern tvGLXContext * tvGLX_FindContext (glmContext ctx);

#define tvGLX_GetContext(tvglx) ((tvglx)->ctx)

#define tvGLX_GetTextureCache(tvglx) ((tvglx)->txCache)
extern tvGLXContext * tvGLX_SetTextureCache (tvGLXContext *ctx,
					     TextureCache *);

#define tvGLX_GetRenderPrimitiveCache(tvglx) ((tvglx)->rpCache)
extern tvGLXContext * tvGLX_SetRenderPrimitiveCache (tvGLXContext *ctx,
						     RenderPrimitiveCache *);

#ifdef __cplusplus
}
#endif

#endif /* TVGLX_H */

