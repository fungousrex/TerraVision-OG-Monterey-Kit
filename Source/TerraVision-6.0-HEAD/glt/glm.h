/*
 * Name: GLM.H
 *
 * Description: OpenGL/X/Motif helper functions.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:55 $
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

#ifndef _GLM_H_
#define _GLM_H_

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <GL/gl.h>

#include <stdlib.h>

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _glmWindow *glmWindow;
typedef struct _glmContext *glmContext;

typedef void (*glmRedrawFunc) (glmWindow, void *);
typedef void (*glmMapNotifyFunc) (glmWindow, void *);

/*
 * Doc++ Description: *//**
 *
 * A window-system independent description of an OpenGL context/mode.
 * This structure is used both as a description and as a template for
 * selecting contexts or modes in an OpenGL-supporting window system.
 *
 * The boolean flags are chosen in such a way that the normally desired
 * context can be selected with an empty (zero-filled) template.
 *
 @see glmXFindContext, glmWFindContext
 */

typedef struct {
  unsigned long id;		/* Unique identifier for context */
  unsigned int overlay:1;	/* Is this an OpenGL overlay mode? */
  unsigned int underlay:1;	/* Is this an OpenGL underlay mode? */
  unsigned int colorIndex:1;	/* Is this a color indexed mode? */
  unsigned int singleBuffer:1;	/* Is this a single-buffered mode? */
  unsigned int stereo:1;	/* Is this a stereo mode? */
  unsigned int slow:1;		/* Is this a slow (unaccelerated) mode? */
  unsigned int listmodes:1;	/* NOT A PROPERTY: display list of gfx modes */
  unsigned int forceid:1;   	/* force the use of visual with ID visualid */
  unsigned int visualid;	/* ID of a specific visual to use */
  unsigned short colorDepth;	/* The number of color bits. */
  unsigned short zbufDepth;	/* The number of depth bits. */
  unsigned short stencilDepth;	/* The number of stencil buffer bits. */
  unsigned short multiBufs;	/* The number of SGI multibuffers. */
  unsigned short multiBufSamples; /* The number of samples per multibuffer. */
} glmContextProperties;

typedef int (*glmChooseContextFunc) (glmContextProperties const *props,
				     void *closure);
#ifdef __cplusplus
}
#endif

#ifdef HAVE_X11

/* These are the X Window System specific interface functions. */

#include <X11/Intrinsic.h>
#include <GL/glx.h>

#ifdef __cplusplus
extern "C" {
#endif

extern glmContext
glmXChooseContext (Display *disp, Screen *scr,
		   glmChooseContextFunc choose, void *closure);

extern glmContext
glmXFindContext (Display *disp, Screen *scr, 
		 int create, glmContextProperties *);

extern glmContext
glmXCreateContext (Display *disp, XVisualInfo *vinfo);

extern XVisualInfo *
glmXContextVisualInfo (glmContext ctx);

#if 0
extern XVisualInfo *
glmXChooseVisualByScore (Display *disp, Screen *scr,
			 int nattribs, int *attribs,
			 int (*scoref) (XVisualInfo *, int *attribs, void *),
			 void *closure);

extern XVisualInfo *
glmXChooseVisual (Display *disp, Screen *scr, int doublebuffer, int stereo);
#endif

extern glmWindow
glmXtCreateWindow (Widget w, glmContext ctx, int doublebuffer, int stereo);

extern glmWindow
glmXCreateWindow (Window w, glmContext ctx, int doublebuffer, int stereo);

#ifdef __cplusplus
}
#endif

#endif

#ifdef WIN32

#ifdef __cplusplus
extern "C" {
#endif

extern glmContext
glmWChooseContext (HWND hWnd, glmChooseContextFunc choose, void *closure);
		   
extern glmContext
glmWCreateContext (HWND hWnd, int iFormat);

extern glmContext
glmWFindContext (HWND hWnd, int create, glmContextProperties *);

extern glmWindow
glmWCreateWindow (HWND hWnd, glmContext ctx, int doublebuffer, int stereo);

extern int
glmWGetPixelFormat( glmContext ctx );

/* And what do these ones do? */

extern void
glmWRunWorkProcs( void );

#ifdef __cplusplus
}
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int
glmContextChooseDefault (glmContextProperties const *props,
			 void *closure);

/* These are the generic interface functions */

extern int
glmContextDoubleBuffered (glmContext ctx);

extern int
glmContextStereo (glmContext ctx);

extern glmContextProperties const *
glmContextProps (glmContext ctx);

extern int
glmContextMatchProperties (glmContextProperties const *prefs, 
			   glmContextProperties const *props);

extern glmContext
glmBaseContext (glmContext ctx);

extern void
glmDestroyContext (glmContext ctx);

extern int
glmWindowDoubleBuffered (glmWindow glmw);

extern int
glmWindowStereo (glmWindow glmw);

extern int
glmWindowNoFlush (glmWindow glmw);

extern void
glmDestroyWindow (glmWindow glmw);

extern int
glmIsMapped (glmWindow glmw);

extern glmRedrawFunc
glmRegisterRedraw (glmWindow glmw, glmRedrawFunc redraw, void *closure);

extern void
glmRemoveRedraw (glmWindow glmw);

extern void
glmRegisterMapNotify (glmWindow glmw, glmMapNotifyFunc notify, void *closure);

extern int
glmWindowDoubleBuffered (glmWindow glmw);

extern int
glmWindowStereo (glmWindow glmw);

extern int
glmWindowNoFlush (glmWindow glmw);

extern glmContext
glmWindowContext (glmWindow glmw);

extern void
glmDestroyWindow (glmWindow glmw);

extern void
glmPostRedraw (glmWindow glmw);

extern int
glmPendingRedraw (glmWindow glmw);

extern glmContext
glmMakeCurrentWindow (char const *funcName, glmWindow);

extern void
glmReleaseWindow (char const *funcName, glmWindow);

extern int
glmSetWindowNoFlush (glmWindow glmw, int noflush);

#define GL_FONT_R 0
#define GL_FONT_I 1
#define GL_FONT_B 2
#define GL_FONT_BI 3

extern void glmCharstr (glmWindow glmw, int fidx, char const *str);

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_glm_INLINE)
# if defined(__GNUC__)
#  define exinline extern inline
# else
#  define exinline inline
# endif
# include "glmil.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _GLM_H_ */
