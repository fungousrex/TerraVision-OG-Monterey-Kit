/*
 * Name: GLM.C
 *
 * Description: glmWindow/glmContext helper functions.
 *   Generic versions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:54 $
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "glm.h"

int
glmContextMatchProperties (glmContextProperties const *prefs, 
			   glmContextProperties const *props)
{
  if (prefs->colorDepth && props->colorDepth < prefs->colorDepth)
    return FALSE;
  if (prefs->zbufDepth && props->zbufDepth < prefs->zbufDepth)
    return FALSE;
  if (prefs->stencilDepth && props->stencilDepth < prefs->stencilDepth)
    return FALSE;
  if (prefs->singleBuffer != props->singleBuffer)
    return FALSE;
  if (prefs->stereo != props->stereo)
    return FALSE;
  /*
  if (prefs->overlay != props->overlay
      || prefs->underlay != props->underlay
      || prefs->colorIndex != props->colorIndex
      || prefs->singleBuffer != props->singleBuffer
      || prefs->stereo != props->stereo
      || prefs->slow != props->slow)
    return FALSE;
    */
  return TRUE;
}

#include <stdio.h>

int
glmContextChooseDefault (glmContextProperties const *props,
			 void *closure)
{
  glmContextProperties const *prefs = closure;
  int score = 0;

  score = (!props->slow) * 1000;
  score += props->colorDepth;
  score += props->zbufDepth;

  if (prefs) {
    if (! glmContextMatchProperties (prefs, props))
      score = 0;
  } else {
    if (props->singleBuffer || props->stereo
	/* || props->overlay || props->underlay */
	|| props->colorIndex) 
      score = 0;
  }

  return score;
}

glmContext
glmBaseContext (glmContext ctx)
{
  while (ctx->share)
    ctx = ctx->share;

  return ctx;
}

int
glmSetWindowNoFlush (glmWindow glmw, int noflush)
{
  int oldvalue;

  if ( glmw == NULL ) return FALSE;

  oldvalue = glmw->noflush;
  glmw->noflush = noflush;

  return oldvalue;
}
