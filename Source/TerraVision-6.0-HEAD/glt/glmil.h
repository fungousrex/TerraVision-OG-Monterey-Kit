/*
 * Name: GLMIL.H
 *
 * Description: Inlined versions of glmWindow abstraction functions.
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

#ifdef __cplusplus
extern "C" {
#endif

#include "glmimpl.h"

exinline int
glmWindowDoubleBuffered (glmWindow glmw)
{
  return glmw->doubleBuffer;
}

exinline int
glmWindowStereo (glmWindow glmw)
{
  return glmw->stereo;
}

exinline int
glmWindowNoFlush (glmWindow glmw)
{
  return glmw->noflush;
}

exinline glmContext
glmWindowContext (glmWindow glmw)
{
  return glmw->ctx;
}

exinline void *
glmGetWidget (glmWindow glmw)
{
  return (*glmw->vtable->getWidget) (glmw);
}

exinline glmRedrawFunc
glmRegisterRedraw (glmWindow glmw, glmRedrawFunc redraw, void *closure)
{
  return (*glmw->vtable->registerRedraw) (glmw, redraw, closure);
}

exinline void
glmRemoveRedraw (glmWindow glmw)
{
  (*glmw->vtable->removeRedraw) (glmw);
}

exinline void
glmPostRedraw (glmWindow glmw)
{
  (*glmw->vtable->postRedraw) (glmw);
}

exinline int
glmPendingRedraw (glmWindow glmw)
{
  return (*glmw->vtable->pendingRedraw) (glmw);
}

exinline glmContext
glmMakeCurrentWindow (char const *funcName, glmWindow glmw)
{
  return (*glmw->vtable->makeCurrent) (glmw, funcName);
}

exinline void
glmReleaseWindow (char const *funcName, glmWindow glmw)
{
  (*glmw->vtable->release) (glmw, funcName);
}

exinline void
glmRegisterMapNotify (glmWindow glmw, glmMapNotifyFunc notify, void *closure)
{
  (*glmw->vtable->registerMapNotify) (glmw, notify, closure);
}

exinline int
glmIsMapped (glmWindow glmw)
{
  return (*glmw->vtable->isMapped) (glmw);
}

exinline void
glmAddTimer (glmWindow glmw, void **id, unsigned int msecs,
	      void (*f) (void *closure), void *closure)
{
  (glmw->vtable->addTimer) (glmw, id, msecs, f, closure);
}

exinline void
glmRemoveTimer (glmWindow glmw, void **id)
{
  (glmw->vtable->removeTimer) (glmw, id);
}

exinline void
glmCharstr (glmWindow glmw, int fidx, char const *str)
{
  (*glmw->vtable->charstr) (glmw, fidx, str);
}

exinline void
glmDestroyWindow (glmWindow glmw)
{
  (*glmw->vtable->destroy) (glmw);
}

/* These are the generic interface functions for glmContexts */

exinline int
glmContextDoubleBuffered (glmContext ctx)
{
  return ctx->doubleBuffer;
}

exinline int
glmContextStereo (glmContext ctx)
{
  return ctx->stereo;
}

exinline glmContextProperties const *
glmContextProps (glmContext ctx)
{
  return &ctx->props;
}

exinline glmContext
glmShareContext (glmContext ctx)
{
  return (*ctx->vtable->createShared) (ctx);
}

exinline void
glmDestroyContext (glmContext ctx)
{
  (*ctx->vtable->destroy) (ctx);
}

#ifdef __cplusplus
}
#endif
