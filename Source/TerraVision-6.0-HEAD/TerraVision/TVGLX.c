/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TVGLX.c,v 2.16.4.2 2002/10/31 05:26:32 reddy Exp $

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

#include "TerraVisionInc.h"
#include "TVGLX.h"
#include "Texture.h"

#include <glt/glt.h>

#include <stdlib.h>
#include <pthr/eqhash.h>

static eqhash_t hash;
static PRLock *lock;

tvGLXContext *
tvGLX_FindContext (glmContext glmCtx)
{
  static char const funcName[] = "tvGLX_FindContext";
  tvGLXContext *ctx;

  if (! lock) {
    hash = eqhash_create (32);
    lock = PR_NewLock ();
  }
  PR_Lock (lock); {
    ctx = eqhash_get (hash, glmCtx);
    TSMTRACE ((GLT_MASK|1, funcName, "glmCtx %p => %p", glmCtx, ctx));
    if (! ctx) {
      ctx = calloc (1, sizeof (*ctx));
      *((glmContext *)&ctx->ctx) = glmCtx;
      eqhash_put (hash, glmCtx, ctx);
      TSMTRACE ((GLT_MASK|1, funcName, "glmCtx %p creates %p", glmCtx, ctx));
    }
  }; PR_Unlock (lock);
  return ctx;
}

tvGLXContext *
tvGLX_SetTextureCache (tvGLXContext *ctx,
		       TextureCache *txCache)
{
  static char const funcName[] = "tvGLX_SetTextureCache";
  TSMTRACE ((GLT_MASK|1, funcName, "%p txCache = %p", ctx, txCache));
  *((TextureCache **) &ctx->txCache) = txCache;
  return ctx;
}

tvGLXContext *
tvGLX_SetRenderPrimitiveCache (tvGLXContext *ctx,
			       RenderPrimitiveCache *rpCache)
{
  static char const funcName[] = "tvGLX_SetRenderPrimitiveCache";
  TSMTRACE ((GLT_MASK|1, funcName, "%p rpCache = %p", ctx, rpCache));
  *((RenderPrimitiveCache **) &ctx->rpCache) = rpCache;
  return ctx;
}

