/* -*- tab-width: 2; -*-

  Name:  Texture.h
 
  Description:
     A data structure for binding and caching texture objects for
     the OpenGL layer.  Managed by reference counting, each TextureCache maps
     an arbitrary ID (void *) to a TextureHandle *, which holds the current
     binding state of the referenced texture.  Textures are allocated with
     the functions TextureFind and TextureRef (which does an implicit
     TextureFind if necessary). A TextureHandle is released with the function
     TextureDeref.  Allocated textures are bound to an image with a call to
     TextureBind, which checks if the texture is already bound and calls back
     to an externally provided routine if not.
 
     Given a fixed amount of texture memory (either user-defined or derived
     from the hardware description) bound textures accumulate until we have
     no more room for them, at which point unused textures are deleted and
     their space reclaimed for new textures.
 
     The TextureCache is designed to operate in a multithreaded environment,
     so cache updates and reference counting are properly interlocked.  In
     fact, for maximum flexibility, TextureBind is the *only* function which
     must be called within an active GLX context.

  Revision History:
      $Id: Texture.h,v 2.39.2.4 2002/10/31 05:26:33 reddy Exp $

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

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
# define  WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

#include <GL/gl.h>
#include <prtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TVGLX_H
typedef struct TextureCache TextureCache;
#endif

typedef struct TextureHandle TextureHandle;

typedef struct TextureImage TextureImage;
typedef struct TexTileRef TexTileRef;

#if 0
extern GLuint Sky, Clouds, Detail;
extern int    SkyLoaded, CloudsLoaded, DetailLoaded;
#endif

#define HAVE_TEXSUBIMAGE \
  (defined(GL_VERSION_1_1) || defined(GL_EXT_subtexture))

#if HAVE_TEXSUBIMAGE
extern int use_subtexture;
#else
#define use_subtexture (0)
#endif

#define HAVE_CLAMP_TO_EDGE \
  (defined (GL_CLAMP_TO_EDGE) || defined (GL_CLAMP_TO_EDGE_SGIS))

#if HAVE_CLAMP_TO_EDGE
# ifdef GL_CLAMP_TO_EDGE
#  define TV_CLAMP_TO_EDGE GL_CLAMP_TO_EDGE
# else
#  define TV_CLAMP_TO_EDGE GL_CLAMP_TO_EDGE_SGIS
# endif
	extern int use_texture_edge_clamp;
#else
# define use_texture_edge_clamp (0)
# define TV_CLAMP_TO_EDGE GL_CLAMP
#endif

/*
 * Doc++ Description: *//** 
 *   Tuned texture definition parameters.
 *
 *   Parameters tuned to optimize rendering textures for the particular
 *   graphics hardware being used.  Indexed by the number of bytes in the
 *   texture data being used (maximum 4 bytes per pixel).
 *
 *   This array is initialize by the TextureTune function.
 *
 * @param name    A short identifier for the texture size.
 * @param fmt     The pixel data format argument to glTexImage2D
 * @param comps   The components argument to glTexImage2D
 * @param size    The number of bytes/pixel of texture memory used
 *
 * @see TextureTune, TextureBind
 */ 

extern struct TexParams {
  char const *name;
  GLenum fmt;
  GLenum comps;
  int size;
} texParams[5];

extern void TextureTune (void);

extern void TextureSetPath (char const *path);

extern void InitTextures (void);

extern int TextureDefineBuffer (GLuint tex_id, int repeat, int mipmap,
				int in_width, int in_height, int nbytes,
				GLubyte *image);

/* Create and initialize a new TextureCache object. */
extern TextureCache * TextureCacheCreate (void);

/* Clear cache w/o deleting */
extern void TextureCacheReset (TextureCache *cache);

/* Returns the current size (in bytes) of the bound images in the cache. */
extern int TextureCacheSize (TextureCache *cache);

/* Returns the current size (as a fraction of the total available texture
   memory) of the bound images in the cache. */
extern float TextureCacheRelativeSize (TextureCache *cache);

/* Returns the texture object ID associated with a TextureHandle */
extern GLuint TextureID (TextureHandle *h);

/* Bind the TextureHandle's texture in the current OpenGL context.  If the
   texture is currently unbound, then size bytes are added to the cache's
   reserved memory and function teximage_f is called to bind the image.
   When the texture is subsequently decached, the function pointed to by
   decache_f is called with the passed closure value.

   If there is not enough space for this image, then unused textures are
   unbound until there is enough space to bind this one. */
extern int TextureBind (TextureHandle * h,
			PRUint32 size,
			void (*teximage_f) (void const *id, void *),
			void *teximage_closure,
			void (*decache_f) (void *id, void *),
			void *decache_closure);

/* Associate a TextureHandle with the id.  Either finds a preexisting
   association or creates a new one as needed.  If a new one is created, then
   a function passed in as the cache_f argument will be called. The
   TextureHandle is initially unreferenced, so a subsequent call to
   TextureRef() is required. */
extern TextureHandle *TextureFind (TextureCache * cache,
				   void const *id,
				   void (*cache_f)(void *id, void *closure),
				   void *closure);

/* Register a reference to the TextureHandle.  Any time a TextureHandle is
   stored in a data structure it must be so referenced. */
#define TextureRef(h) TextureRef_(funcName,h)

extern TextureHandle *TextureRef_ (char const *funcName, 
				   TextureHandle * h);

/* Deregister a reference to the TextureHandle.  Any time a data structure
   containing a TextureHandle is cleared, it must be dereferenced.  If the
   TextureHandle is unreferenced as a result, then it is placed on a queue and
   available for deletion or reuse, at which time, the function passed as the
   decache_f argument will be called. */
#define TextureDeref(h) TextureDeref_(funcName,h)

extern void TextureDeref_ (char const *funcName, 
			   TextureHandle * h);

/* Print a description of the cache in the trace stream */
extern void TextureCacheDump (char const *funcName, TextureCache *cache);


/* This is a second kind of image cache, only useful for non-repeating
   textures.  Currently, it is specific to TsTile image tiles, but I'm going
   to fix that soon -- leei */

#include "TsGeoPyramid.h"

extern TextureImage *TextureImageCreate (int tile_w, int tile_h, int ncomps);
extern void TextureImageDestroy (TextureImage *im);

extern TexTileRef * TexRefFromTile (TextureImage *im, TsTile const *tile);

extern GLuint TexRefID (TexTileRef const *tref);
extern float const * TexRefCoords (TexTileRef *tref,
				   int len, TsCoord const *coords);

extern GLuint TexRefBind (TexTileRef *tref, GLuint tex_id);

extern void TexRef (TexTileRef *tref);
extern void TexRefRelease (TexTileRef *tref);

extern void TexRefCheck (TexTileRef const *tref, TsTile const *tile);

#ifdef HAVE_INLINE
# include <pthr/eqhash.h>
# include <pthr/list.h>
# include <glt/glt.h>
# ifndef NO_Texture_INLINE
#  ifdef __GNUC__
#   define exinline extern inline
#  elif ( defined(WIN32) && defined(MSVC) )
#   define exinline __inline
#  else
#   define exinline inline
#  endif
#  include "TextureInline.h"
#  undef exinline
# endif /* NO_Texture_INLINE */
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _TEXTURE_H_ */
