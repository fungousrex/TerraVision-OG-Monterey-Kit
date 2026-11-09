/*
 * Name: GLT.H
 *
 * Description: OpenGL helper functions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:42:49 $
 *     $Author: martinreddy $
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

#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#define GLT_MASK (1<<30)

/* OpenGL 1.0 doesn't have these */
#ifndef GL_VERSION_1_1
#ifdef GL_EXT_texture_object
#define glBindTexture      glBindTextureEXT
#define glDeleteTextures   glDeleteTexturesEXT
#define glGenTextures      glGenTexturesEXT
#else
#define glGenTextures(n,p)	((void)(0))
#define glBindTextures(t,tx)	((void)(0))
#define glDeleteTextures(n,p)	((void)(0))
#endif
#ifdef GL_EXT_subtexture
#define glTexSubImage1D		glTexSubImage1DEXT
#define glTexSubImage2D		glTexSubImage2DEXT
#else
#define glTexSubImage1D(t,l,x,w,f,ty,p)		((void)(0))
#define glTexSubImage2D(t,l,x,w,h,f,ty,p)	((void)(0))
#endif
#endif

#if defined(GL_VERSION_1_2) || !defined(GL_EXT_polygon_offset)
#define glPolygonOffsetEXT    glPolygonOffset /*((void)(0))*/
#endif

#ifndef GL_LUMINANCE8_EXT
#define GL_LUMINANCE8_EXT    GL_LUMINANCE8
#endif

#ifndef GL_LUMINANCE8_ALPHA8_EXT
#define GL_LUMINANCE8_ALPHA8_EXT GL_LUMINANCE8_ALPHA8
#endif

#ifndef GL_RGB8_EXT
#define GL_RGB8_EXT GL_RGB8
#endif

#ifndef GL_RGBA8_EXT
#define GL_RGBA8_EXT GL_RGBA8
#endif

#ifndef GL_RGB4_EXT
#define GL_RGB4_EXT GL_RGB4
#endif

#ifndef GL_RGBA4_EXT
#define GL_RGBA4_EXT GL_RGBA4
#endif

/* XXX reddy - end of above change */

#ifdef __cplusplus
extern "C" {
#endif

extern char const *gltOGLVersion (void);
extern float gltOGLVersionNum (void);
extern char const *gltOGLRenderer (void);
extern char const * gltOGLExtensions (void);

extern int gltHasExtension (char const *ext_name);

typedef struct {
  GLenum pname;
  GLfloat param[4];
} gltParamDef;

#define GLT_PARAMDEF_END {0}

extern void gltLightModelDefv (gltParamDef *def);
extern void gltLightDefv (GLenum light, gltParamDef *def);
extern void gltMaterialDefv (GLenum face, gltParamDef *def);
extern void gltTexParameterDefv (GLenum target,  gltParamDef *def);

#ifdef __GNUC__
#define LOCSTR1(f,l) f #l
#define LOCSTR(f,l) LOCSTR1(f ":", l)
#define FILE_LOC LOCSTR(__FILE__,__LINE__)
#else
#define FILE_LOC funcName
#endif

extern GLuint gltGenList (void);
extern void gltDeleteList (GLuint list);

extern int gltCheckErrorsEnabled;
extern void gltEnableCheckErrors (int enable);
extern void gltCheckErrors (char const *location);

/* These abstract away the use of texture objects. */
extern void gltGenTextures (GLsizei n, GLuint *textures);
extern void gltBeginTexture (GLenum which_tex, GLuint id);
extern void gltEndTexture (void);
extern void gltDeleteTextures (GLsizei n, const GLuint *textures);
extern void gltBindTexture (GLenum which_tex, GLuint id);

extern GLuint glGenList (void);
extern void glDeleteList (GLuint);

extern GLuint power2size (GLuint x);
extern GLuint gltTextureSize (GLuint x);

#ifndef gltClearColor4ub
# define gltClearColor4ub(r,g,b,a) \
    glClearColor((r)/255.0,(g)/255.0,(b)/255.0,(a)/255.0)
#endif

void
gltScaleImage (GLuint ncomps, 
	       GLuint in_w, GLuint in_h, GLuint in_span,
	       GLubyte const *in,
	       GLuint out_w, GLuint out_h, GLuint out_span,
	       GLubyte *out);

void
gltInvertImage (GLubyte *image, GLuint width, GLuint height, GLuint ncomps);

/* These functions are defined in glt_state.c for tracing and comparing OpenGL
   states */

typedef struct gltGLState gltGLState;

extern void 
gltTraceState (unsigned int cond, char const *descr);

extern gltGLState *
gltGetState (void);

extern void
gltFreeState (gltGLState *state);

extern void
gltTraceStateDiff (unsigned int cond, char const *descr,
		   gltGLState const *s1, gltGLState const *s2);

#if defined(HAVE_INLINE) && !defined(NO_glt_INLINE)
# if defined(__GNUC__)
#  define exinline extern inline
# else
#  define exinline inline
# endif
# include "glt_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _GL_UTILS_H_ */
