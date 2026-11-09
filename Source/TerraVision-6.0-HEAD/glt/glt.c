/*
 * Name: GLT.C
 *
 * Description: OpenGL helper functions.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <prlock.h>
#include <prinit.h>
#include <pthr/trace.h>

#include "glt.h"
#include "gltI.h"

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

int gltCheckErrorsEnabled = 0;

static void
gltEndTextureOBJ (void)
{
}

static void APIENTRY
gltGenTexturesOLD (GLsizei n, GLuint *textures)
{
  GLuint id = glGenLists (n);
  GLuint i;

  /* Ensure that list/texture ids are identical. */
  for (i = 0; i < n; ++i)
    textures[i] = id+i;
}

static void APIENTRY
gltBeginTextureOLD (GLenum which_tex, GLuint id)
{
  glNewList (id, GL_COMPILE_AND_EXECUTE);
}

static void
gltEndTextureOLD (void)
{
  glEndList ();
}

static void APIENTRY
gltDeleteTexturesOLD (GLsizei n, const GLuint *textures)
{
  GLsizei i;
  for (i = 0; i < n; ++i)
    glDeleteLists (textures[i], 1);
}

static void APIENTRY
gltBindTextureOLD (GLenum which_tex, GLuint id)
{
  glCallList (id);
}

char const *
gltOGLGetVersion (void)
{
  static char const funcName[] = "gltOGLGetVersion()";

  gltCurrVTable->glVersion = (char const *) glGetString (GL_VERSION);

  if (gltCurrVTable->glVersion) {
    PTRACE ((GLT_MASK|1, funcName, "OpenGL version %s",
	     gltCurrVTable->glVersion));
    sscanf (gltCurrVTable->glVersion, "%f", &gltCurrVTable->glVersionNum);

    gltCurrVTable->glRenderer = (char const *) glGetString (GL_RENDERER);
    PTRACE ((GLT_MASK|1, funcName, "OpenGL renderer %s",
	     gltCurrVTable->glRenderer));

    gltCurrVTable->glExts = (char const *) glGetString (GL_EXTENSIONS);
    PTRACE ((GLT_MASK|1, funcName, "OpenGL extensions %s",
	     gltCurrVTable->glExts));

    /* Set up glt*Texture functions depending on texture_object extension. */
    /* this was an extension in 1.0, but became part of the main OpenGL    */
    /* API as of 1.1. If we have no support for this, then we emulate.     */

#ifdef GL_VERSION_1_1
    if ( gltCurrVTable->glVersionNum >= 1.1 ) {
      PTRACE ((GLT_MASK|1, funcName, "OpenGL 1.1: texture objects available"));
      gltCurrVTable->genTextures = glGenTextures;
      gltCurrVTable->beginTexture = glBindTexture;
      gltCurrVTable->endTexture = gltEndTextureOBJ;
      gltCurrVTable->deleteTextures = glDeleteTextures;
      gltCurrVTable->bindTexture = glBindTexture;
    } else
#endif
#if (!defined(GL_VERSION_1_2)) && defined(GL_EXT_texture_object)
    if ( gltHasExtension ("texture_object") ) {
      PTRACE ((GLT_MASK|1, funcName, "Use OpenGL texture objects."));
      gltCurrVTable->genTextures = glGenTexturesEXT;
      gltCurrVTable->beginTexture = glBindTextureEXT;
      gltCurrVTable->endTexture = gltEndTextureOBJ;
      gltCurrVTable->deleteTextures = glDeleteTexturesEXT;
      gltCurrVTable->bindTexture = glBindTextureEXT;
    } else 
#endif
    {
      PTRACE ((GLT_MASK|1, funcName, "Simulate OpenGL texture objects."));
      gltCurrVTable->genTextures = gltGenTexturesOLD;
      gltCurrVTable->beginTexture = gltBeginTextureOLD;
      gltCurrVTable->endTexture = gltEndTextureOLD;
      gltCurrVTable->deleteTextures = gltDeleteTexturesOLD;
      gltCurrVTable->bindTexture = gltBindTextureOLD;
    }
  }

  return gltCurrVTable->glVersion;
}

static void APIENTRY
gltGenTexturesINIT (GLsizei n, GLuint *textures)
{
  gltOGLVersion ();
  if (gltCurrVTable->genTextures != gltGenTexturesINIT)
    (*gltCurrVTable->genTextures) (n, textures);
}

static void APIENTRY
gltBeginTextureINIT (GLenum which_tex, GLuint id)
{
  gltOGLVersion ();
  if (gltCurrVTable->beginTexture != gltBeginTextureINIT)
    (*gltCurrVTable->beginTexture) (which_tex, id);
}

static void
gltEndTextureINIT (void)
{
  gltOGLVersion ();
  if (gltCurrVTable->endTexture != gltEndTextureINIT)
    (*gltCurrVTable->endTexture) ();
}

static void APIENTRY
gltDeleteTexturesINIT (GLsizei n, const GLuint *textures)
{
  gltOGLVersion ();
  if (gltCurrVTable->deleteTextures != gltDeleteTexturesINIT)
    (*gltCurrVTable->deleteTextures) (n, textures);
}

static void APIENTRY
gltBindTextureINIT (GLenum which_tex, GLuint id)
{
  gltOGLVersion ();
  if (gltCurrVTable->bindTexture != gltBindTextureINIT)
    (*gltCurrVTable->bindTexture) (which_tex, id);
}

#define VTableINIT_val {			\
  0, 0, 0, 0,					\
  gltGenTexturesINIT,				\
  gltBeginTextureINIT,				\
  gltEndTextureINIT,				\
  gltDeleteTexturesINIT,			\
  gltBindTextureINIT				\
}

gltVTable gltVTableINIT = VTableINIT_val;

gltVTable gltVTableGlobal = VTableINIT_val;
gltVTable *gltCurrVTable = &gltVTableGlobal;


static struct {
  PRLock *lock;
  GLuint *start, *end;
  GLuint *free;
} Lists;

PRCallOnceType initListsOnce;

static PRStatus
initLists (void)
{
  Lists.lock = PR_NewLock ();
  return PR_SUCCESS;
}

GLuint
gltGenList (void)
{
  static const char funcName[] = "gltGenList";
  GLuint val = 0;

  PR_CallOnce (&initListsOnce, initLists);

  PR_Lock (Lists.lock); {
    if (Lists.free > Lists.start)
      val = *--Lists.free;
  }; PR_Unlock (Lists.lock);

  return (val ? val : glGenLists (1));
;
}

void
gltDeleteList (GLuint list)
{
  PR_Lock (Lists.lock); {
    if (! Lists.start) {
      Lists.start = (GLuint *) malloc (256 * sizeof (GLuint));
      Lists.end = Lists.start + 256;
      Lists.free = Lists.start;
    } else if (Lists.free == Lists.end) {
      int n = (Lists.end - Lists.start);
      Lists.start = (GLuint *) realloc (Lists.start, 2*n * sizeof (GLuint));
      Lists.end = Lists.start + 2*n;
      Lists.free = Lists.start + n;
    }

    *Lists.free++ = list;
  }; PR_Unlock (Lists.lock);
}

int
gltHasExtension (char const *ext_name)
{
  gltOGLVersion ();

  {
    char const *idx = strstr (gltCurrVTable->glExts, ext_name);
    int len = strlen (ext_name);

    while (idx) {
      if (idx-6 >= gltCurrVTable->glExts && strncmp (idx-6, "GL_EXT", 6)
	  && (idx[len] == ' ' || idx[len] == '\0'))
	return 1;
      idx = strstr (idx+len, ext_name);
    }
  }

  return 0;
}


static int
log_pow2 (unsigned int x)
{
  unsigned int i;
  for (i = 0; i < 32; ++i)
    if (x <= (1<<i))
      return i;
  return i;
}

static void
gltScaleImagePow2 (GLuint ncomps, 
		   GLuint in_w, GLuint in_h, GLuint in_span,
		   GLubyte const *in,
		   GLuint out_w_shft, GLuint out_h_shft, GLuint out_span_shft,
		   GLubyte *out);

void
gltScaleImage (GLuint ncomps, 
	       GLuint in_w, GLuint in_h, GLuint in_span,
	       GLubyte const *in,
	       GLuint out_w_u, GLuint out_h_u, GLuint out_span_u,
	       GLubyte *out)
{
  int out_w = out_w_u, out_h = out_h_u, out_span = out_span_u;

  GLubyte const *in_start, *in_last;
  GLubyte *ostart;
  GLubyte *out_end;

  int xi0, yi, dxi, dyi;
  int xr0, yr, dxr, dyr;
  GLuint i, ndxi;

  /* Pass on to specialized routine for powers of 2. */
  {
    GLuint l2_w = log_pow2 (out_w);
    GLuint l2_h = log_pow2 (out_h);
    if (out_w == (1<<l2_w) && out_h == (1<<l2_h)) {
      gltScaleImagePow2 (ncomps, in_w, in_h, in_span, in,
			 l2_w, l2_h, out_span_u, out);
      return;
    }
  }

  in_start = in;
  in_last = in + in_span*(in_h-1);
  ostart = out;
  out_end = out + out_span*out_h;

  /* Determine int/remainder components of start and step. */
  {
    int dx = in_w, dy = in_h;
    int x0 = (dx-out_w)/2, y0 = (dy-out_h)/2;

    if (x0 < 0) {
      xi0 = (x0 / out_w) - 1;
      xr0 = out_w + (x0 % out_w);
    } else {
      xi0 = x0 / out_w;
      xr0 = x0 % out_w;
    }

    if (y0 < 0) {
      yi = (y0 / out_h) - 1;
      yr = out_h + (y0 % out_h);
    } else {
      yi = y0 / out_h;
      yr = y0 % out_h;
    }
    dxi = dx / out_w; dyi = dy / out_h;
    dxr = dx % out_w; dyr = dy % out_h;
  }

  in += yi*in_span;
  ndxi = ncomps*dxi;

  for ( ; in < in_start; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i2 = in+ncomps*xi0+in_span, *i2_last = in+2*in_span-ncomps;
    int xr = xr0;

    for ( ; i2 < in+in_span; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i2[i+ncomps];

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    for ( ; o1 < o1_end && i2 < i2_last; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (xr_inv*bi2[i] + xr*bi4[i])/out_w;
      }

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i2[i];

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }
  
  for ( ; out < out_end && in < in_last; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i1 = in+ncomps*xi0, *i2 = i1+in_span, *i1_last = in+in_span-ncomps;
    int xr = xr0;
    GLuint yr_inv = out_h-yr;

    for ( ; i1 < in; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*bi3[i] +  yr*bi4[i])/out_h;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    for ( ; o1 < o1_end && i1 < i1_last; o1 += ncomps) {
      int out_div = out_w*out_h;

      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*(xr_inv*bi1[i] + xr*bi3[i])
		    +   yr*(xr_inv*bi2[i] + xr*bi4[i]))/out_div;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte *bo1 = (GLubyte *) o1;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*bi1[i] + yr*bi2[i])/out_h;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }

  for ( ; out < out_end; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i1 = in+ncomps*xi0, *i1_last = in+in_span-ncomps;
    int xr = xr0;

    for ( ; i1 < in; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i1[i+ncomps];

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    for ( ; o1 < o1_end && i1 < i1_last; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (xr_inv*bi1[i] + xr*bi3[i])/out_w;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i1[i];

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }
}  

static void
gltScaleImagePow2 (GLuint ncomps, 
		   GLuint in_w, GLuint in_h, GLuint in_span,
		   GLubyte const *in,
		   GLuint out_w_shft, GLuint out_h_shft, GLuint out_span,
		   GLubyte *out)
{
  int out_w = 1<<out_w_shft, out_h = 1<<out_h_shft;
  GLuint out_w_mask = out_w-1, out_h_mask = out_h-1;

  GLubyte const *in_start = in, *in_last = in + in_span*(in_h-1);
  GLubyte *out_end = out + out_span*out_h;

  int xi0, yi, dxi, dyi;
  int xr0, yr, dxr, dyr;
  GLuint i, ndxi;

  /* Determine int/remainder components of start and step. */
  {
    int dx = in_w, dy = in_h;
    int x0 = (dx-(int)out_w)/2, y0 = (dy-(int)out_h)/2;

    if (x0 < 0) {
      xi0 = (x0 / out_w) - 1;
      xr0 = out_w + (x0 % out_w);
    } else {
      xi0 = x0 / out_w;
      xr0 = x0 % out_w;
    }

    if (y0 < 0) {
      yi = (y0 / out_h) - 1;
      yr = out_h + (y0 % out_h);
    } else {
      yi = y0 / out_h;
      yr = y0 % out_h;
    }
    dxi = dx >> out_w_shft; dyi = dy >> out_h_shft;
    dxr = dx  & out_w_mask; dyr = dy  & out_h_mask;
  }

  in += yi*in_span;
  ndxi = ncomps*dxi;

  for ( ; in < in_start; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i2 = in+ncomps*xi0+in_span, *i2_last = in+2*in_span-ncomps;
    int xr = xr0;

    for ( ; i2 < in+in_span; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i2[i+ncomps];

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    for ( ; o1 < o1_end && i2 < i2_last; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (xr_inv*bi2[i] + xr*bi4[i])>>out_w_shft;
      }

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i2[i];

      /* Step input components and pointers. Per output pixel. */
      i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i2 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }
  
  for ( ; out < out_end && in < in_last; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i1 = in+ncomps*xi0, *i2 = i1+in_span, *i1_last = in+in_span-ncomps;
    int xr = xr0;
    unsigned int yr_inv = out_h-yr;

    for ( ; i1 < in; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*bi3[i] +  yr*bi4[i])>>out_h_shft;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    for ( ; o1 < o1_end && i1 < i1_last; o1 += ncomps) {
      GLuint out_shft = out_w_shft + out_h_shft;

      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte const *bi4 = (GLubyte const *) (i2+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*(xr_inv*bi1[i] + xr*bi3[i])
		    +   yr*(xr_inv*bi2[i] + xr*bi4[i]))>>out_shft;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi2 = (GLubyte const *) i2;
	GLubyte *bo1 = (GLubyte *) o1;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (yr_inv*bi1[i] +  yr*bi2[i])>>out_h_shft;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi; i2 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; i2 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }

  for ( ; out < out_end; out += out_span) {
    GLubyte *o1 = out, *o1_end = out+out_span;
    GLubyte const *i1 = in+ncomps*xi0, *i1_last = in+in_span-ncomps;
    GLuint xr = xr0;

    for ( ; i1 < in; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i1[i+ncomps];

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    for ( ; o1 < o1_end && i1 < i1_last; o1 += ncomps) {
      /* Combine components. */
      {
	GLubyte const *bi1 = (GLubyte const *) i1;
	GLubyte const *bi3 = (GLubyte const *) (i1+ncomps);
	GLubyte *bo1 = (GLubyte *) o1;
	GLuint xr_inv = out_w-xr;

	for (i = 0; i < ncomps; ++i)
	  bo1[i] = (xr_inv*bi1[i] + xr*bi3[i])>>out_w_shft;
      }

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    for ( ; o1 < o1_end; o1 += ncomps) {
      /* No interpolation needed. */
      for (i = 0; i < ncomps; ++i)
	o1[i] = i1[i];

      /* Step input components and pointers. Per output pixel. */
      i1 += ndxi;
      xr += dxr;
      if (xr > out_w) { xr -= out_w; i1 += ncomps; }
    }

    /* Step input components and pointers. Per output line. */
    in += dyi*in_span;
    yr += dyr;
    if (yr > out_h) { yr -= out_h; in += in_span; }
  }
}

void
gltInvertImage (GLubyte *image, GLuint width, GLuint height, GLuint ncomps)
{
  GLuint line_len = ncomps*width;
  GLubyte *top = image;
  GLubyte *bot = image + line_len*(height-1);
  GLubyte *tmp = alloca (line_len);

  for ( ; top <= bot; top += line_len, bot -= line_len) {
    memcpy (tmp, top, line_len);
    memcpy (top, bot, line_len);
    memcpy (bot, tmp, line_len);
  }
}
