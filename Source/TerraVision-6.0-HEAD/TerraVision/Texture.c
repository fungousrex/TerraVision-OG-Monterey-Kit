/* -*- tab-width: 2; -*-

  Name:  Texture.c

  Description: The texture cache for paging textures in and out of 
      texture map memory

  Revision History:
      $Id: Texture.c,v 2.91.2.6 2002/10/31 05:26:32 reddy Exp $

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

#include "TsCache.h"
#include "TVGLX.h"

#include <string.h>
#include <glt/glt.h>
#include <tsm/tsmImage.h>

#include "Texture.h"
#include "TextureI.h"

/* RCS INFO */
static char const Texture_rcsid[] = "$Id: Texture.c,v 2.91.2.6 2002/10/31 05:26:32 reddy Exp $";

#if TEXTRACE_
int texture_tracing = 0;
#endif

GLuint Sky, Clouds, Detail;
int    SkyLoaded = FALSE, CloudsLoaded = FALSE, DetailLoaded = FALSE;

GLclampf texprio_idle = 0.1;
GLclampf texprio_active = 1.0;

static int texture_memory = 0;

struct TexParams texParams[] = {
  {0, 0, 0},							/* 0 */ 
  {"LUM8", GL_LUMINANCE, GL_LUMINANCE8_EXT, 1},			/* 1 */
  {"LUMA", GL_LUMINANCE_ALPHA, GL_LUMINANCE8_ALPHA8_EXT, 2},	/* 2 */
  {"RGB ", GL_RGB, GL_RGB8_EXT, 3},				/* 3 */
  {"RGBA", GL_RGBA, GL_RGBA8_EXT, 4}				/* 4 */
};

#if HAVE_CLAMP_TO_EDGE
int use_texture_edge_clamp = 0;
#endif

#if HAVE_TEXSUBIMAGE
int max_texsize = 1024;
int use_subtexture = 0;
#endif

#define KILOBYTE (1<<10)
#define MEGABYTE (1<<20)

void
TextureTune (void)
{
  static char const funcName[] = "TextureTune";
  char const *renderer = gltOGLRenderer ();

  TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName,
	     "Tune texture mapping wrt %s", renderer));

#if HAVE_TEXSUBIMAGE
  use_subtexture = (gltOGLVersionNum() >= 1.1
		   || gltHasExtension ("GL_EXT_subtexture"));
  TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName, "Subtexturing? %s",
	     use_subtexture ? "YES" : "NO"));
#endif

#if HAVE_CLAMP_TO_EDGE
  use_texture_edge_clamp = gltHasExtension ("GL_SGIS_texture_edge_clamp");
  TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName, "Texture edge clamping? %s",
	     use_texture_edge_clamp ? "YES" : "NO"));
#endif

  /* Reality Engine, Impact or Infinite Reality. */
  if (! strncmp (renderer, "RE", 2)
      || ! strncmp (renderer, "IR", 2)
      || ! strncmp (renderer, "IMPACT", 6)
      || ! strncmp (renderer, "CRIME", 5)) {
    /* Use 16 bit internal formats. */
    /*texParams[1].comps = GL_LUMINANCE16_EXT;  texParams[1].size = 2;*/
    TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName,
	       "Using 16bit internal textures.", renderer));
#if 1
    texParams[3].comps = GL_RGBA4_EXT; texParams[3].size = 2;
    texParams[4].comps = GL_RGBA4_EXT; texParams[4].size = 2;
#else
    texParams[3].comps = GL_RGBA8_EXT; texParams[3].size = 4;
    texParams[4].comps = GL_RGBA8_EXT; texParams[4].size = 4;
#endif
  }

#if 0
  if (! strncmp (renderer, "RE", 2)) {
    /* This covers REC, RE and REV (VTX) machines */
    TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName,
	       "Disable texture cache sharing."));
    tvGLX_UseSharing (FALSE);
  }
#endif

  if (! strncmp (renderer, "CRIME", 5)
      || ! strncmp (renderer, "GLZICD", 6)) {
    texture_memory = 16*MEGABYTE;
  } else if (! strncmp (renderer, "IMPACT", 6)) {
    /* IMPACT/<pixd>/<nrm>/<texsz> with texsz in Mbytes */
    char *p = strrchr (renderer, '/');
    if (p) texture_memory = atoi (p+1) * MEGABYTE;
  } else if (! strncmp (renderer, "RE", 2)) {
    /* REV/<pixd>/<nrm>/<texsz> with texsz in Mtexels (16bit) */
    char *p = strrchr (renderer, '/');
    if (p) texture_memory = 2 * atoi (p+1) * MEGABYTE;
  } else if (! strncmp (renderer, "IR", 2)) {
    /* IR?/<pixd>/<nrm>/<texsz>/<nge> with texsz in MBytes. */
    char *p = strchr (renderer, '/');
    TSMTRACE ((TEX_MASK|1, funcName, "p = %s", p));
    if (p) p = strchr (p+1, '/');
    TSMTRACE ((TEX_MASK|1, funcName, "p = %s", p));
    if (p) p = strchr (p+1, '/');
    TSMTRACE ((TEX_MASK|1, funcName, "p = %s", p));
    if (p) texture_memory = atoi (p+1) * MEGABYTE;
  } else if (! strncmp (renderer, "Mesa DRI Voodoo3", 16)) {
    texParams[3].comps = GL_RGB4_EXT; texParams[3].size = 2;
    texParams[4].comps = GL_RGBA4_EXT; texParams[4].size = 2;
    texture_memory = 10 * MEGABYTE;
  }

#if HAVE_TEXSUBIMAGE
  {
    GLint w, h;
    glGetIntegerv (GL_MAX_TEXTURE_SIZE, &w);
    TSMTRACE ((TEX_MASK|1, funcName, "GL_MAX_TEXTURE_SIZE = %d", w));
    if (w < max_texsize)
      max_texsize = w;

#if 0 && defined(GL_PROXY_TEXTURE_2D)
    /* Now use proxy textures to verify that estimate */
    while (max_texsize > 128) {
      TSMTRACE ((TEX_MASK|2, funcName, "Check PROXY_TEXTURE_2D %dx%d",
		 max_texsize, max_texsize));
      glTexImage2D (GL_PROXY_TEXTURE_2D, 0, texParams[3].comps,
		    max_texsize, max_texsize, 0,
		    GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      /* Check that we succeeded */
      glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
      glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
      if (w != 0 && h != 0)
	break;
      max_texsize /= 2;
    }
#endif

    if (max_texsize == 128) {
      TSMTRACE ((TEX_MASK|1, funcName, "Disable subtexures"));
      /* No point in texsubimage processing */
      use_subtexture = FALSE;
    } else {
      TSMTRACE ((TEX_MASK|1, funcName, "TextureImage %dx%d",
		 max_texsize, max_texsize));
    }
  }
#endif

  if (texture_memory) {
    TSMTRACE ((TEX_MASK|1, funcName, "Texture memory %dM",
	       texture_memory/MEGABYTE));
  }
}

int
TextureDefineBuffer (GLuint tex_id, int repeat, int mipmap,
		     int in_width, int in_height, int nbytes, GLubyte *image)
{
  static char const funcName[] = "TextureDefineBuffer";
  int tx_width, tx_height;
  GLubyte *tx_image;
  GLenum tx_format, tx_comps;

  gltInvertImage (image, in_width, in_height, nbytes);

  tx_format = texParams[nbytes].fmt;
  tx_comps = texParams[nbytes].comps;
  tx_width = gltTextureSize (in_width);
  tx_height = gltTextureSize (in_height);
  if (tx_width != in_width || tx_height != in_height) {
    tx_image = (GLubyte *) alloca (tx_width*tx_height*nbytes);
    gltScaleImage (nbytes,
		   in_width, in_height, nbytes*in_width, image,
		   tx_width, tx_height, nbytes*tx_width, tx_image);
  } else {
    tx_image = image;
  }

  gltBeginTexture (GL_TEXTURE_2D, tex_id); {
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		     mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    if (repeat) {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else if (use_texture_edge_clamp) {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TV_CLAMP_TO_EDGE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TV_CLAMP_TO_EDGE);
    } else {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    } 

    TSMTRACE ((TEX_MASK|OGL_MASK|1, funcName,
	       "tex_id %d: tx_image %d,%d format %s %s",
	       tex_id, tx_width, tx_height,
	       texParams[nbytes].name, repeat ? "Repeat" : "Clamp"));
    glPixelStorei (GL_UNPACK_ALIGNMENT, tx_width%4==0 ? 4 : 1);
    glTexImage2D (GL_TEXTURE_2D, 0, tx_comps,
		  tx_width, tx_height, 0,
		  tx_format, GL_UNSIGNED_BYTE, tx_image);
    if (tx_width*tx_height >= 8) {
      int tx_len = tx_width*tx_height;
      TSMTRACE ((TEX_MASK|OGL_MASK|2, funcName,
		 "   [%02x,%02x,%02x,%02x, ..., %02x,%02x,%02x,%02x]",
		 tx_image[0], tx_image[1], tx_image[2], tx_image[3], 
		 tx_image[tx_len-4], tx_image[tx_len-3], 
		 tx_image[tx_len-2], tx_image[tx_len-1]));
    }

    /* Define the mipmap... */
    if (mipmap) {
      int level = 1;
      int mipmap_w = (tx_width  > 1 ?  tx_width/2 : tx_width);
      int mipmap_h = (tx_height > 1 ? tx_height/2 : tx_height);
      GLubyte *mipmap_image = (GLubyte *) alloca (mipmap_w*mipmap_h*nbytes);
      while (tx_width > 1 || tx_height > 1) {
	gltScaleImage (nbytes, 
		       tx_width, tx_height, nbytes*tx_width, tx_image,
		       mipmap_w, mipmap_h, nbytes*mipmap_w, mipmap_image);
	TSMTRACE ((TEX_MASK|OGL_MASK|2, funcName,
		   "tex_id %d: mipmap %d %d,%d",
		   tex_id, level, mipmap_w, mipmap_h));
	glTexImage2D (GL_TEXTURE_2D, level, tx_comps,
		      mipmap_w, mipmap_h, 0,
		      tx_format, GL_UNSIGNED_BYTE, mipmap_image);
	if (mipmap_w*mipmap_h >= 8) {
	  int m_len = mipmap_w*mipmap_h;
	  TSMTRACE ((TEX_MASK|OGL_MASK|2, funcName,
		     "   [%02x,%02x,%02x,%02x, ..., %02x,%02x,%02x,%02x]",
		     mipmap_image[0], mipmap_image[1],
		     mipmap_image[2], mipmap_image[3], 
		     mipmap_image[m_len-4], mipmap_image[m_len-3], 
		     mipmap_image[m_len-2], mipmap_image[m_len-1]));
	}
	
	++level;
	tx_width = mipmap_w; tx_height = mipmap_h;
	mipmap_w = (tx_width  > 1 ?  tx_width/2 : tx_width);
	mipmap_h = (tx_height > 1 ? tx_height/2 : tx_height);
	{
	  GLubyte *tmp = mipmap_image;
	  mipmap_image = tx_image;
	  tx_image = tmp;
	}
      }
    }
  }; gltEndTexture ();

  return TRUE;
}

/*
 * Synopsis: int TextureDefine(char *fname, GLuint tex_id, int repeat)
 *	
 * Description: Reads an image file then defines as a texture, normal or tiled.
 *
 * Returns: TRUE if successful.
 *
 * Author: Lee Iverson
 * 
 * Date: October 1997.
 *
 */
static int
TextureDefine (char *fname, GLuint tex_id, int repeat)
{
  static char const funcName[] = "TextureDefine";
  int in_width, in_height, nbytes;
  GLubyte *image;
  Image_t *img;
  int ret;

  if ( ( img = tsmReadImage( fname ) ) == NULL ) {
    tsmError( funcName, "Couldn't load image file: %s", fname );
    return FALSE;
  }
  
  in_width  = img->width;
  in_height = img->height;
  nbytes    = img->components;
  image     = (GLubyte *) img->bitmap;

  tsmFreeImage( img, FALSE /* don't free img->bitmap */ );

  ret = TextureDefineBuffer (tex_id, repeat, TRUE,
			     in_width, in_height, nbytes, image);

  free (image);

  return ret;
}

#if 0
static void
mipmapTexture (GLubyte const *im,
	       int nbytes, int im_w, int im_h, int level)
{
  GLubyte *tx_im = (GLubyte *) alloca (2 * im_w * im_h * nbytes);
  int span = 2*im_w*nbytes;
  GLubyte *to_im;
  int to_w, to_h;

  /* First copy the image into the temporary buffer at 0,0 */
  {
    GLubyte const *p;
    GLubyte *q;
    GLubyte const *im_end = im + (im_w*im_h*nbytes);
    int im_span = im_w*nbytes;
    for (p = im, q = tx_im; p < im_end; p += im_span, q += span) 
      memcpy (q, p, im_span);
  }

  /* Now downsample it repeatedly */
  im = tx_im;
  to_im = tx_im + im_w*nbytes;
  to_w = im_w/2;
  to_h = im_h/2;

  while (to_w > 1 || to_h > 1) {
    gltScaleImage (nbytes, im_w, im_h, span, im,
		   to_w, to_h, span, to_im);
    
    im = to_im; im_w = to_w; im_h = to_h;
    to_im += to_w * nbytes;
    to_w = to_w/2;
    to_h = to_h/2;
  }

  gltScaleImage (nbytes, im_w, im_h, span, im,
		 to_w, to_h, span, to_im);
}
#endif

static char const  *texturePath;

/*
 * Synopsis: void TextureSetPath(char const *path)
 *	
 *
 * Doc++ Description: *//**
 *   Sets the path for looking up Sky, Detail, and Ground texture images.
 *
 * @param path   path to directory containing support textures.
 *
 * @return  void
 *
 * @author  Lee Iverson
 * 
 * @version November 1999
 *
 */

void
TextureSetPath (char const *path)
{
  texturePath = strdup (path);
}

#if 0
/*
 * Synopsis: void InitTextures(void)
 *	
 *
 * Description: Gets the textures.
 *
 * Externals:
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: 1993
 *
 */
void 
InitTextures (void)
{
  static char const funcName[] = "InitTextures";
  char        buffer[500];
  GLuint      tex[3];

  if (! texturePath) {
    tsmError (funcName, "Must call TextureSetPath() for sky and ground.");
    return;
  }

  /* ALLOCATE THE GENERALIZED TEXTURE IDS. */
  gltGenTextures (3, tex);

  Sky    = tex[0];
  Clouds = tex[1];
  Detail = tex[2];

  sprintf(buffer,"%s/sky.ppm", texturePath);
  SkyLoaded = TextureDefine (buffer, Sky, TRUE);
  gltCheckErrors ("InitTextures() Sky");

  sprintf(buffer,"%s/detail.ppm", texturePath);
  DetailLoaded = TextureDefine (buffer, Detail, TRUE);
  gltCheckErrors ("InitTextures() Detail");

  sprintf(buffer,"%s/clouds.ppm", texturePath);
  CloudsLoaded = TextureDefine (buffer, Clouds, TRUE);
  gltCheckErrors ("InitTextures() Clouds");

}
#endif

/* Manage the LRU list part of the texture cache */

void
TextureLRUFixup (TextureHandle const *h_)
{
  static const char funcName[] = "TextureLRUFixup";
  TextureHandle *h = (TextureHandle *) h_;
  TextureCache *cache = h->cache;
  TextureCacheLock (cache); {
    list_iter_t i = h->lru_iter;
    if (h->refcount == 0) {
      /* Make sure it's on LRU list. */
      if (! list_iter_valid (i)) {
	TEXTRACE (cache, "ins", h, h->id);
	h->lru_iter = list_push (cache->lru, h);
      }
      TSMTRACE ((TEX_MASK|2, funcName, "ADD %p:%p", h, h->id));
    } else {
      /* Make sure that it's not on LRU list */
      if (list_iter_valid (i)) {
	TEXTRACE (cache, "ext", h, h->id);
	((TextureHandle *)h)->lru_iter = 0;
	list_delete (i);
	TSMTRACE ((TEX_MASK|2, funcName, "DEL %p:%p", h, h->id));
      }
    }
  }; TextureCacheUnlock (cache);
}

static void
TextureDecache (TextureHandle *h)
{
  static char const funcName[] = "TextureDecache";
  TextureCache * cache = h->cache;

  TSMTRACE ((TEX_MASK|1, funcName, "%p:%p tex_id %d", h, h->id, h->tex_id));

  TEXTRACE (cache, "dec", h, h->id);

  eqhash_delete (cache->eqhash, h->id);
  if (h->decache_f) (*h->decache_f) ((void *) h->id, h->closure);
  h->id = 0;

  /* Remove from LRU if necessary. */
  {
    list_iter_t i = h->lru_iter;
    if (list_iter_valid (i)) {
      h->lru_iter = 0;
      list_delete (i);
    }
  }

  /* In this case, we place on front of list so it will be reused quickly */
  list_shift (cache->unused, h);
}

static void
TextureDecacheCB (TsCache const *tsCache, void *arg, void *closure)
{
  static char const funcName[] = "TextureDecacheCB";
  TsTile const *tile = arg;
  TextureCache *cache = closure;
  TextureCacheLock (cache); {
    TextureHandle *h = eqhash_get (cache->eqhash, tile);
    if (h && h->refcount == 0) {
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p [%d %d %d] tex_id %d",
		 h, h->id, tile->id.x, tile->id.y, tile->id.res, h->tex_id));
      TextureDecache (h);
    }
  }; TextureCacheUnlock (cache);
}

/* This is necessarily called from within a valid OpenGL context. */

static void
TextureDelete (TextureCache * cache, TextureHandle *h)
{
  static char const funcName[] = "TextureDelete";

  TSMTRACE ((TEX_MASK|1, funcName, "%p:%p tex_id %d", h, h->id, h->tex_id));

  TEXTRACE (cache, "del", h, h->id);

  if (h->tex_id) {
    TSMTRACE ((TEX_MASK|2, funcName, "%p:%p DEL tex_id %d",
	       h, h->id, h->tex_id));
    gltDeleteTextures (1, &h->tex_id);
    h->tex_id = 0;
  }
  
  cache->size -= h->size;
  h->size = 0;
  TSMTRACE ((TEX_MASK|2, funcName, "cache size %d", cache->size));

  eqhash_delete (cache->eqhash, h->id);
  if (h->decache_f) (*h->decache_f) ((void *) h->id, h->closure);
  h->id = 0;

  list_push (cache->unused, h);
}

static void
TextureCacheMakeRoom (TextureCache *cache, PRUint32 size)
{
  static const char funcName[] = "TextureCacheMakeRoom";

  TSMTRACE ((TEX_MASK|2, funcName, "cache size %d + %d > %d?", 
	     cache->size, size, cache->texmem));

  while (cache->size + size > cache->texmem
	 && ! list_empty (cache->lru)) {
    /* Get the first tile on the LRU list for deallocation. */
    TextureHandle *h = list_unshift (cache->lru);
    if (h->refcount == 0) {
      h->lru_iter = 0;
      TextureDelete (cache, h);
    }
  }
}

TextureHandle *
TextureFind (TextureCache * cache,
	     void const *id,
	     void (*cache_f)(void *id, void *closure),
	     void *closure)
{
  static const char funcName[] = "TextureFind";
  TextureHandle *h;
  TextureCacheLock (cache); {
    h = eqhash_get (cache->eqhash, id);
    if (! h) {
      /* Create/reuse a handle and place in hash */
      h = (list_empty (cache->unused)
	   ? calloc (1, sizeof (*h))
	   : list_unshift (cache->unused));
      TSMTRACE ((TEX_MASK|1, funcName, "%p:%p", h, id));
      TEXTRACE (cache, "new", h, id);
      h->id = id;
      h->cache = cache;
      h->refcount = 0;
      h->tex_matched = 0;
      if (cache_f) (*cache_f) ((void *) id, closure);
      eqhash_put (cache->eqhash, id, h);
    }
  }; TextureCacheUnlock (cache);
  return h;
}

void
TextureBindSlow (TextureHandle *h,
		 PRUint32 size,
		 void (*teximage_f) (void const *id, void *closure),
		 void *teximage_closure,
		 void (*decache_f) (void *id, void *closure),
		 void *decache_closure)
{
  static char const funcName[] = "TextureBindSlow";
  TextureCache * cache = h->cache;

  TextureCacheLock (cache); {
    /* If this handle has a current tex_id, then free up its space. */
    if (h->tex_id) {
      cache->size -= h->size;
    } else {
      gltGenTextures (1, &h->tex_id);
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p GEN tex_id %d",
		 h, h->id, h->tex_id));
    }

    /* Update the handle and cache sizes. */
    h->size = size;
    h->decache_f = decache_f;
    h->closure = decache_closure;
    cache->size += size;
    TSMTRACE ((TEX_MASK|2, funcName, "cache size %d", cache->size));

    /* If there's still not enough room try to clean up some more. */
    if (cache->size > cache->texmem)
      TextureCacheMakeRoom (cache, size);

    /* Mark tex_id as bound to id. */
    h->tex_matched = 1;
  }; TextureCacheUnlock (cache);

  gltBeginTexture (GL_TEXTURE_2D, h->tex_id); {
    TSMTRACE ((TEX_MASK|1, funcName, "Bind %p:%p tex_id %d",
	       h, h->id, h->tex_id));
    (*teximage_f) (h->id, teximage_closure);
  }; gltEndTexture ();

  /* BIND IT FOR USE */
  glEnable (GL_TEXTURE_2D);
}

/*
 * Synopsis: TextureCache * TextureCacheCreate()
 *	
 * Description: Creates a texture cache for texture paging
 *
 * Externals: None.
 *
 * Returns: a pointer to the texture cache on sucess
 *          NULL on failure
 *
 * Author: Lee Iverson
 * 
 * Date: February 1999
 *
 */

TextureCache *
TextureCacheCreate (void)
{
  static char const funcName[] = "TextureCacheCreate";
  TextureCache *textureCache;

  /* GET THE SIZE OF THE CACHE */
  textureCache = (TextureCache *) malloc (sizeof (TextureCache));
  textureCache->size = 0;
  textureCache->texmem = texture_memory;
  textureCache->lock = PR_NewLock();

  textureCache->eqhash = eqhash_create (256);
  textureCache->lru = list_create (FALSE);
  textureCache->unused = list_create (FALSE);

  TSMTRACE ((TEX_MASK|1, funcName, "%p size %d",
	     textureCache, textureCache->texmem));

  TsCacheAddCallback (NULL, TILE_DECACHE, TextureDecacheCB, textureCache, 0);

#if TEXTRACE_
  memset (&textureCache->traces, 0, sizeof (textureCache->traces));
  textureCache->traces.lock = PR_NewLock ();
#endif

  return textureCache;
}

void
TextureCacheReset (TextureCache *cache)
{
  static char const funcName[] = "TextureCacheReset";
  TSMTRACE ((TEX_MASK|1, funcName, ""));
  /* Decache all unreferenced tiles. */
	if (cache == NULL)
		return;
  
	TextureCacheLock (cache); {
    while (! list_empty (cache->lru)) {
      TextureHandle *h = list_unshift (cache->lru);
      if (h->refcount == 0)
        TextureDecache (h);
    }
  }; TextureCacheUnlock (cache);
}

#if TEXTRACE_
static void
TextureTraceShow (char const *funcName, TextureTrace const *tr)
{
  char const *name = pthr_name ((void const *) tr->thread);
  if (! name) name = "";
  TSMTRACE ((0, funcName, "%10.10s[%p] %.3s %p:%p tex_id %d refs %d",
	     name, tr->thread, tr->op,
	     tr->h, tr->id, tr->tex_id, tr->refcount));
}
#endif

static void
TextureElemDump (void const *obj, void *val, void *closure)
{
  TextureHandle const *h = val;
  
  TSMTRACE ((0, "  ", "%p:%p tex_id %d refs %d", 
	     h, h->id, h->tex_id, h->refcount));
}

void
TextureCacheDump (char const *funcName, TextureCache *cache)
{
  TSMTRACE ((0, funcName, "TextureCache %p {", cache));
  TSMTRINC ((0, 1));

  TextureCacheLock (cache); {
    /* Walk through hash table. */
    eqhash_forall (cache->eqhash, TextureElemDump, 0);

#if TEXTRACE_
    {
      int i, end;
      TextureTraceList const *traces = &cache->traces;
      i = traces->next;
      end = (traces->wrap ? i : 0);
      if (i != end || traces->wrap) {
	do {
	  if (--i < 0) i = TextureTrace_len-1;
	  TextureTraceShow ("  ", &traces->list[i]);
	} while (i != end);
      }
    }
#endif

  }; TextureCacheUnlock (cache);

  TSMTRINC ((0, -1));
  TSMTRACE ((0, funcName, "} %p", cache));
}

#if HAVE_TEXSUBIMAGE

/*
 * This is a different kind of texture cache.
 *
 * It is based around a small set of large images (each defined by a TIElem),
 * and the tiled allocation of subimages within these.
 */

struct TextureImage {
  PRLock *lock;
  struct TexParams *params;
  PRUint16 tile_w, tile_h;
  PRUint16 w, h;
  PRUint32 elem_size;
  eqhash_t tile_table;

  struct TIElem {
    TIElem *next;
    TextureImage *im;
    PRLock *lock;
    GLuint tex_id;
    TexTileRef *tiles;
    TexTileRef *end;
    list_t *lru;
  } *elem;
};

static TIElem * TextureImageElemAdd (TextureImage *im);
static void TextureImageElemRemove (TIElem *e);
static void TextureImageDecacheCB (TsCache const *tsCache,
				   void *arg, void *closure);

TextureImage *
TextureImageCreate (int tile_w, int tile_h, int ncomps)
{
  static char const funcName[] = "TextureImageCreate";
  TextureImage *im = malloc (sizeof (*im));

  im->lock = PR_NewLock ();
  im->params = &texParams[ncomps];
  im->elem_size = texParams[ncomps].size*max_texsize*max_texsize;

  im->tile_w = tile_w;
  im->tile_h = tile_h;
  im->w = max_texsize/tile_w;
  im->h = max_texsize/tile_h;
  im->elem = 0;

  im->tile_table = eqhash_create (256);

  TSMTRACE ((TEX_MASK|1, funcName, "%p", im));
  TextureImageElemAdd (im);

  TsCacheAddCallback (NULL, TILE_DECACHE, TextureImageDecacheCB, im, 0);
  
  return im;
}

void
TextureImageDestroy (TextureImage *im)
{
  static char const funcName[] = "TextureImageDestroy";
  TSMTRACE ((TEX_MASK|1, funcName, "%p", im));

  PR_Lock (im->lock); {
    while (im->elem)
      TextureImageElemRemove (im->elem);

    eqhash_destroy (im->tile_table);
    im->tile_table = 0;
  }; PR_Unlock (im->lock);

  TsCacheRemoveCallback (NULL, TILE_DECACHE, TextureImageDecacheCB, im);
  
  PR_DestroyLock (im->lock);
  free (im);
}

static TIElem *
TextureImageElemAdd (TextureImage *im)
{
  static char const funcName[] = "TextureImageElemAdd";
  TIElem *e = malloc (sizeof (*e));
  TIElem **p;

  TSMTRACE ((TEX_MASK|2, funcName, "%p:%p", im, e));

  e->next = 0;
  e->im = im;
  e->lock = PR_NewLock ();
  e->tiles = calloc (im->w*im->h, sizeof (e->tiles[0]));
  e->end = &e->tiles[im->w*im->h];
  e->lru = list_create (FALSE);

  /* This will be allocated in TextureImageElemBind() */
  e->tex_id = 0;

  /* Now add on to end of list. */
  for (p = &im->elem; *p; p = &(*p)->next);
  *p = e;

  return e;
}

static void
TextureImageElemBind (TIElem *e)
{
  static char const funcName[] = "TextureImageElemBind";
  TextureImage *im = e->im;

  glGenTextures (1, &e->tex_id);
  glBindTexture (GL_TEXTURE_2D, e->tex_id);
  TSMTRACE ((TEX_MASK|2, funcName,
	     "glTexImage2D (GL_TEXTURE_2D, 0, %d, %d, %d, 0, %d, GL_UNSIGNED_BYTE, NULL)",
	     im->params->comps, 
	     im->w*im->tile_w, im->h*im->tile_h,
	     im->params->fmt));
  glTexImage2D (GL_TEXTURE_2D, 0, im->params->comps, 
		im->w*im->tile_w, im->h*im->tile_h, 0,
		im->params->fmt, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static void
TextureImageElemRemove (TIElem *e)
{
  static char const funcName[] = "TextureImageElemRemove";
  TIElem **p;
  
  PR_Lock (e->lock); {
    TSMTRACE ((TEX_MASK|2, funcName, "%p:%p", e->im, e));

    free (e->tiles);
    e->tiles = 0;
    list_destroy (e->lru);
    e->lru = 0;

    /* Find link to e and pass over e */
    for (p = &e->im->elem; *p != e; p = &(*p)->next);
    *p = e->next;
  }; PR_Unlock (e->lock);

  PR_DestroyLock (e->lock);
  free (e);
}

static TexTileRef *
TexRefAlloc (TextureImage *im, TsTile const *tile)
{
  static char const funcName[] = "TexRefAlloc";
  TexTileRef *tref = 0;
  TIElem *e;

  /* First, see if there is any unallocated space available. */
  for (e = im->elem; e; e = e->next) {
    PR_Lock (e->lock);
    if (e->end > e->tiles) {
      tref = --e->end;
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p New ref %p", im, e, tref));
      break;
    }
    PR_Unlock (e->lock);
    TSMTRACE ((TEX_MASK|2, funcName, "%p:%p elem full", im, e));
  }

  if (! tref) {
    /* Then check for unused TexTileRefs. */
    for (e = im->elem; e; e = e->next) {
      PR_Lock (e->lock);
      if ((tref = list_unshift (e->lru)))
	break;
      PR_Unlock (e->lock);
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p elem full", im, e));
    }
  }

  /* If we failed to find an empty region (the TIElem *e will be NULL), then
     add a new element and try again */
  if (! e) {
    TSMTRACE ((TEX_MASK|2, funcName, "%p All elems full", im));
    PR_Lock (im->lock); {
      TextureImageElemAdd (im);
    }; PR_Unlock (im->lock);
    return TexRefAlloc (im, tile);
  }

  /* If we are reusing a bound tref, free its coords. */
  if (tref->texCoord) {
    TSMTRACE ((TEX_MASK|2, funcName, "%p:%p Reuse ref %p", im, e, tref));
    free ((void *)tref->texCoord);
  }

  /* Now place it in the table */
  PR_Lock (im->lock); {
    /* Forget previous tile=>tref mapping */
    if (tref->tile) {
      eqhash_delete (im->tile_table, tref->tile);
      NetLogTile (tref->tile, TV_TILE_OUT_TEXCACHE);
    }
    /* Register the new tile=>tref mapping */
    eqhash_put (im->tile_table, tile, tref);
  }; PR_Unlock (im->lock);

  /* And finally release the TIElem lock */
  PR_Unlock (e->lock);

  TSMTRACE ((TEX_MASK|2, funcName, "%p:%p ref %p @ %d,%d",
	     im, e, tref, (tref-e->tiles)%im->w, (tref-e->tiles)/im->w));

  tref->tile = tile;
  tref->im = im;
  tref->elem = e;
  tref->refcount = 1;
  tref->tex_id = 0;
  tref->lru_iter = 0;
  tref->texCoord = 0;

  TSMTRACE((TEX_MASK|2, funcName, "%p cnt %d [%d %d %d]",
	    tref, tref->refcount, 
	    tref->tile->id.x, tref->tile->id.y, tref->tile->id.res));
  return tref;
}

void
TexRefLRUFixup (TexTileRef *tref, int lock)
{
  static char const funcName[] = "TextureImageLRUFixup";
  TIElem *e = tref->elem;

  if (lock) PR_Lock (tref->im->lock);

  if (tref->refcount == 0) {
    if (! tref->lru_iter) {
      tref->lru_iter = list_push (e->lru, tref);
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p Unused ref %p: lru %p",
		 e->im, e, tref, tref->lru_iter));
    }
  } else {
    if (tref->lru_iter) {
      list_delete (tref->lru_iter);
      tref->lru_iter = 0;
      TSMTRACE ((TEX_MASK|2, funcName, "%p:%p Reused ref %p", e->im, e, tref));
    }
  }

  if (lock) PR_Unlock (tref->im->lock);
}

static void
TextureImageDecacheCB (TsCache const *tsCache, void *arg, void *closure)
{
  static char const funcName[] = "TextureImageDecacheCB";
  TsTile const *tile = arg;
  TextureImage *im = closure;
  PR_Lock (im->lock); {
    TexTileRef *tref = eqhash_get (im->tile_table, tile);
    if (tref) {
      TSMTRACE((TEX_MASK|2, funcName, "%p cnt %d [%d %d %d]",
		tref, 0,
		tref->tile->id.x, tref->tile->id.y, tref->tile->id.res));
      tref->refcount = 0;
      TexRefLRUFixup (tref, FALSE);
      eqhash_delete (im->tile_table, tile);
      NetLogTile (tile, TV_TILE_OUT_TEXCACHE);
    }
  }; PR_Unlock (im->lock);
}

TexTileRef *
TexRefFromTile (TextureImage *im, TsTile const *tile)
{
  static char const funcName[] = "TexRefFromTile";
  TexTileRef *tref;

  PR_Lock (im->lock); {
    tref = eqhash_get (im->tile_table, tile);
    if (tref) {
      TSMTRACE((TEX_MASK|2, funcName, "%p cnt %d [%d %d %d]",
		tref, tref->refcount+1,
		tref->tile->id.x, tref->tile->id.y, tref->tile->id.res));
      if (PR_AtomicIncrement (&tref->refcount) == 1)
	TexRefLRUFixup (tref, FALSE);
    }
  }; PR_Unlock (im->lock);
  if (tref)
    return tref;

  /* Allocate a new region in the image. */
  return TexRefAlloc (im, tile);
}

GLuint
TexRefBind (TexTileRef *tref, GLuint tex_id)
{
  static char const funcName[] = "TexRefBind";
  TextureImage *im = tref->im;
  TIElem *e = tref->elem;
  TsTile const *tile = tref->tile;
  int tw = im->tile_w;
  int th = im->tile_h;
  int offset = tref - e->tiles;
  int x = offset % im->w;
  int y = offset / im->w;

  /* Do we need to allocate a new texture object for this
     particular TIElem? */
  if (! e->tex_id)
    TextureImageElemBind (e);
  else if (e->tex_id != tex_id)
    glBindTexture (GL_TEXTURE_2D, e->tex_id);

  TSMTRACE ((TEX_MASK|1, funcName, "%p @ %d,%d [%d %d %d] buf %p",
	     tref, x*tw, y*th,
	     tile->id.x, tile->id.y, tile->id.res, tile->data));
  glTexSubImage2D (GL_TEXTURE_2D, 0, x*tw, y*th, tw, th, 
		   im->params->fmt, GL_UNSIGNED_BYTE, tile->data);

  NetLogTile (tile, TV_TILE_IN_TEXCACHE);
  tref->tex_id = e->tex_id;

  return e->tex_id;
}

float const *
TexRefGenCoords (TexTileRef *tref, int len, TsCoord const *coords)
{
  static char const funcName[] = "TexRefGenCoords";
  TextureImage *im = tref->im;
  TIElem *e = tref->elem;
  float *texCoord = malloc (2 * len * sizeof (float));

  int offset = tref - e->tiles;
  int x = offset % im->w;
  int y = offset / im->w;
  float scl_x = im->w;
  float scl_y = im->h;
  int i;
  float *t;

  /* And calculate the new texture coordinates */
  TSMTRACE ((TEX_MASK|2, funcName, "%p @ %d,%d texCoords %p",
	     tref, x, y, texCoord));
  TSMTRINC ((TEX_MASK|3, 1));
  for (i = 0, t = texCoord; i < len; ++i, t += 2) {
    t[0] = (x + coords[i].texCoord[0])/scl_x;
    t[1] = (y + coords[i].texCoord[1])/scl_y;
    TSMTRACE ((TEX_MASK|3, funcName, "%.3f,%.3f => %.3f,%.3f",
	       x + coords[i].texCoord[0], y + coords[i].texCoord[1],
	       t[0], t[1]));
  }
  TSMTRINC ((TEX_MASK|3, -1));
  tref->texCoord = texCoord;

  return texCoord;
}

void
TextureImageCheckRefs (TextureImage const *im,
		       int (*count) (TexTileRef const *tref, void *closure),
		       void *closure)
{
  TIElem const *e;

  for (e = im->elem; e; e = e->next) {
    TexTileRef const *tref = e->tiles;
    list_iter_t i;

    for (tref = e->tiles; tref < e->end; ++tref) {
      if (tref) 
	assert (tref->refcount == (*count) (tref, closure));
    }

    for (i = list_first (e->lru); ! list_done (i); i = list_next (i)) {
      tref = list_value (i);
      assert (tref->refcount == (*count) (tref, closure));
    }
  }
}

#endif
