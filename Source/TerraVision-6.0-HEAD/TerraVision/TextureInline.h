/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TextureInline.h,v 2.21.4.2 2002/10/31 05:26:33 reddy Exp $

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

#include <assert.h>
#include <prthread.h>
#include <pratom.h>

#include "Trace.h"

#include "TextureI.h"

#ifdef __cplusplus
extern "C" {
#endif

#if TEXTRACE_

extern int texture_tracing;

exinline void
TextureTraceAdd (TextureCache *cache,
		 char op[3], TextureHandle const *h,
		 void const *id)
{
  TextureTraceList *traces = &cache->traces;
  TextureTrace *p;

  PR_Lock (traces->lock); {
    p = &traces->list[traces->next];
    if (++traces->next == TextureTrace_len) {
      traces->next = 0;
      traces->wrap = 1;
    }
  }; PR_Unlock (traces->lock);

  memcpy (p->op, op, 3);
  p->thread = PR_GetCurrentThread ();
  p->h = h;
  p->id = id;
  p->tex_id = (h ? h->tex_id : 0);
  p->refcount = (h ? h->refcount : 0);
}
#endif

exinline int
TextureCacheSize (TextureCache *cache)
{
  return cache->size;
}

exinline float
TextureCacheRelativeSize (TextureCache *cache)
{
  return cache->size/(float)cache->texmem;
}

exinline GLuint
TextureID (TextureHandle *h)
{
  return h->tex_id;
}

/*
 * This is the only call in the entire suite of TextureCache manipulation
 * routines which is guaranteed to be in an active OpenGL context.  All others
 * must refrain from making *any* OpenGL calls.
 */

extern void
TextureBindSlow (TextureHandle *h,
		 PRUint32 size,
		 void (*teximage_f) (void const *id, void *closure),
		 void *teximage_closure,
		 void (*decache_f) (void *id, void *closure),
		 void *decache_closure);

exinline int
TextureBind (TextureHandle * h,
	     PRUint32 size,
	     void (*teximage_f) (void const *id, void *closure),
	     void *teximage_closure,
	     void (*decache_f) (void *id, void *closure),
	     void *decache_closure)
{
  static const char funcName[] = "TextureBind";

  TEXTRACE (cache, "bnd", h, h->id);
  if (h->tex_matched) {
    gltBindTexture (GL_TEXTURE_2D, h->tex_id);
    TSMTRACE ((TEX_MASK|3, funcName, "Bind %p:%p tex_id %d", 
	       h, h->id, h->tex_id));
    return TRUE;
  } else {
    TextureBindSlow (h, size,
		     teximage_f, teximage_closure, 
		     decache_f, decache_closure);
    return FALSE;
  }
}

extern void TextureLRUFixup (TextureHandle const *h);

exinline TextureHandle *
TextureRef_ (char const *funcName, 
	     TextureHandle * h)
{
  /* This should be an atomic add (NSPR) */
  TSMTRACE ((TEX_MASK|3, funcName, "REF %p:%p %d", h, h->id, h->refcount));
  if (PR_AtomicIncrement (&h->refcount) == 1)
    TextureLRUFixup (h);
  return h;
}
    
exinline void
TextureDeref_ (char const *funcName, 
	       TextureHandle * h)
{
  /* This should be an atomic add (NSPR) */
  TSMTRACE ((TEX_MASK|3, funcName, "DEREF %p:%p %d", h, h->id, h->refcount));
  if (PR_AtomicDecrement (&h->refcount) == 0)
    TextureLRUFixup (h);
}

#if HAVE_TEXSUBIMAGE

exinline GLuint
TexRefID (TexTileRef const *tref)
{
  return tref->tex_id;
}

exinline void
TexRefCheck (TexTileRef const *tref, TsTile const *tile)
{
  assert (tile == tref->tile);
}

extern void TexRefLRUFixup (TexTileRef *tref, int lock);

exinline void
TexRef (TexTileRef *tref)
{
  static char const funcName[] = "TexRef";
  TSMTRACE((TEX_MASK|2, funcName, "%p cnt %d [%d %d %d]",
	    tref, tref->refcount+1, 
	    tref->tile->id.x, tref->tile->id.y, tref->tile->id.res));
  if (PR_AtomicIncrement (&tref->refcount) == 1)
    TexRefLRUFixup (tref, TRUE);
}

exinline void
TexRefRelease (TexTileRef *tref)
{
  static char const funcName[] = "TexRefRelease";
  TSMTRACE((TEX_MASK|2, funcName, "%p cnt %d [%d %d %d]",
	    tref, tref->refcount-1, 
	    tref->tile->id.x, tref->tile->id.y, tref->tile->id.res));
  if (PR_AtomicDecrement (&tref->refcount) == 0) {
    TexRefLRUFixup (tref, TRUE);
  }
}


extern float const *
TexRefGenCoords (TexTileRef *tref, int len, TsCoord const *coords);

exinline float const *
TexRefCoords (TexTileRef *tref, int len, TsCoord const *coords)
{
  if (tref->texCoord)
    return tref->texCoord;
  else
    return TexRefGenCoords (tref, len, coords);
}

#else /* ! HAVE_TEXSUBIMAGE */

exinline TextureImage *
TextureImageCreate (int tile_w, int tile_h, int ncomps)
{
  abort ();
  return 0;
}

exinline void
TextureImageDestroy (TextureImage *im)
{
  abort ();
}

exinline TexTileRef *
TexRefFromTile (TextureImage *im, TsTile const *tile)
{
  abort ();
  return 0;
}

exinline GLuint
TexRefID (TexTileRef const *tref)
{
  abort ();
  return 0;
}

exinline float const *
TexRefCoords (TexTileRef *tref, int len, TsCoord const *coords)
{
  abort ();
  return 0;
}

exinline GLuint
TexRefBind (TexTileRef *tref, GLuint tex_id)
{
  abort ();
  return 0;
}

exinline void
TexRef (TexTileRef *tref)
{
  abort ();
}

exinline void
TexRefRelease (TexTileRef *tref)
{
  abort ();
}

exinline void
TexRefCheck (TexTileRef const *tref, TsTile const *tile)
{
  abort ();
}

#endif

#ifdef __cplusplus
}
#endif
