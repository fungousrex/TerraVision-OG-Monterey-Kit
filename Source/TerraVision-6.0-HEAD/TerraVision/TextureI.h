/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TextureI.h,v 2.2.2.2 2002/10/31 05:26:33 reddy Exp $

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

#ifndef _TextureI_h_
#define _TextureI_h_

#include <assert.h>
#include <prthread.h>
#include <pratom.h>

#include "Trace.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TEXTRACE_ 0

#if TEXTRACE_

extern int texture_tracing;

typedef struct TextureTrace {
  char                       op[3];
  PRThread                   thread;
  TextureHandle const *      h;
  void const *               id;
  GLuint                     tex_id;
  PRInt32                    refcount;
} TextureTrace;

#define TEXTRACE(cache,op,h,id) \
  do { if (texture_tracing) TextureTraceAdd (cache,op,h,id); } while (0)

#define TextureTrace_len (1024)
typedef struct TextureTraceList {
  PRLock *        lock;
  TextureTrace    list[TextureTrace_len];
  unsigned short  next;
  unsigned short  wrap;
} TextureTraceList;
#else
#define TEXTRACE(cache,op,h,id)
#endif

struct TextureCache {
  PRLock *              lock;
  PRUint32              size;
  PRUint32              texmem;
  eqhash_t              eqhash;
  list_t *              lru;
  list_t *              unused;

#if TEXTRACE_
  TextureTraceList      traces;
#endif
};

#define TextureCacheLock(cache) PR_Lock(cache->lock)
#define TextureCacheUnlock(cache) PR_Unlock(cache->lock)

struct TextureHandle {
  unsigned int         size:31;
  unsigned int         tex_matched:1;
  PRInt32              refcount;
  GLuint               tex_id;
  list_iter_t          lru_iter;
  void const *         id;
  TextureCache *       cache;
  void (*decache_f)    (void *id, void *closure);
  void *               closure;
};

#if HAVE_TEXSUBIMAGE

typedef struct TIElem TIElem;

struct TexTileRef {
  TsTile const *tile;
  TextureImage *im;
  TIElem *elem;
  PRInt32 refcount;
  GLuint tex_id;
  list_iter_t lru_iter;
  float const *texCoord;
};

#endif

#ifdef __cplusplus
}
#endif

#endif /* _TextureI_h_ */
