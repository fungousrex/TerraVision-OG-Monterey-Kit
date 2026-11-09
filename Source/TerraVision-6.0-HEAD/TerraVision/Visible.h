/* -*- tab-width: 2; -*-

  Name:  Visible.h
 
  Description: Header file for the visibility calc functions
 
  Revision History:
      $Id: Visible.h,v 2.40.2.7 2002/10/31 05:26:43 reddy Exp $

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

#ifndef VISIBLE_H
#define VISIBLE_H

typedef struct QuadTile QuadTile;
typedef struct VisibilityParams VisibilityParams;
typedef struct VisibleStruct VisibleStruct;
typedef struct VisibleList VisibleList;

/*
 * Doc++ Description: *//** 
 * An enum denoting valid values for texture-popping strategy. Values are:
 * <pre>
 * VisiblePopping_None
 * VisiblePopping_LowRes
 * VisiblePopping_LowFrag
 * </pre>
 *
 */  
enum VisiblePoppingStrategy {
  VisiblePopping_None,
  VisiblePopping_LowRes,
  VisiblePopping_LowFrag
};
typedef enum VisiblePoppingStrategy VisiblePoppingStrategy;

#include <prtypes.h>

#include <pthr/istack.h>
#include <pthr/eqhash.h>
#include <vrml2/vec3.h>

#include "Texture.h"
#include "TsGeoPyramid.h"

struct QuadTile {
  PRUintn new_tile:1;
  PRUintn overres:1;
  PRUintn leaves:15;
  PRUintn tree_size:15;
  TsTile const *tile;
  QuadTile *parent;
  QuadTile *children[4];
  VisibilityParams *params;
};

/* the GeoBBox structure holds the geocentric coordinates for a */
/* bounding box. There is a field called valid which is used to */
/* determine whether this structure has a valid bbox defined.   */

typedef struct GeoBBox {
  double min[3];
  double max[3];
  int    valid;
} GeoBBox;

typedef struct VisibleDraw {
  PRUint16 flags;
  TsTile const *tile;
  struct RenderPrimitive *prim;
  union {
    TextureHandle *node;
    TexTileRef *ref;
  } tex;
} VisibleDraw;

struct VisibleStruct {
  TsGeoView *geoView;
  QuadTile const *quad;
  TsTile const *parent;
  VisibleStruct *neigh[4];
  VisibleDraw draw;
};

typedef struct VisibleGeoList {
  PRUint16 n, len;
  VisibleStruct **visp;
  PRInt16 *lastOfRes;

  TsGeoPyramid *geo;
  TsGeoView *geoView;
  QuadTile const *quad;
  TextureImage *im;
  TsBBox bbox; /* This is in the TsGeoPyramid's LVCS */
} VisibleGeoList;

struct VisibleList {
  PRUint32 id;
  PRUint16 n, len;
  VisibleGeoList **geo_list;

  PRUint16 ntiles, maxTiles;
  istack *ist;
  TsBBox bbox; /* This is in the GCC coords */

  TextureCache *textureCache;

  eqhash_t in_list;
  VisiblePoppingStrategy poppingStrategy;

  void (*add_cb) (VisibleList*, VisibleGeoList*, VisibleStruct*, void *);
  void *add_closure;
  void (*remove_cb) (VisibleList*, VisibleGeoList*, VisibleStruct*, void *);
  void *remove_closure;
};

#define VISFLAG_LOWRES   (1<<0)	/* Tile has higher res children in list */
#define VISFLAG_SHOW_SW  (1<<1)	/* Tile doesn't have (0,0) child in list. */
#define VISFLAG_SHOW_SE  (1<<2)	/* Tile doesn't have (0,1) child in list. */
#define VISFLAG_SHOW_NW  (1<<3)	/* Tile doesn't have (1,0) child in list. */
#define VISFLAG_SHOW_NE  (1<<4)	/* Tile doesn't have (1,1) child in list. */
#define VISFLAG_SHOW_ALL (0xf<<1)/* Tile doesn't have any children. */

extern void VisibleInit (VisibleList *list, TextureCache *textureCache);
extern void VisibleGeoClear (VisibleGeoList *geo_list);
extern void VisibleClear (VisibleList *list);
extern void VisibleDestroy (VisibleList *list);

#define VIS_FAIL      (0)	/* Fail visibility test */
#define VIS_OVERRES   (1<<1)	/* Visible but overresolution */
#define VIS_VISIBLE   (1<<2)	/* Visible */
#define VIS_ALLIN     (1<<3)	/* Entirely inside view fustrum */

typedef int (*VisibleCBFunc) (TsTile *tile, TsGeoPyramid *geo, void *closure);

struct VisibilityParams {
  istack *ist;
  eqhash_t in_tree;
  eqhash_t prev_tree;
  int  (*vis_f) (TsTile const *tile, TsGeoPyramid const *geo, int check_vis,
		 void *closure);
  void (*add_f) (TsTile const *tile, TsGeoPyramid const *geo, int leaf, 
		 void *closure);
  void (*free_f) (VisibilityParams *params);
  void *closure;
};

QuadTile * CalcVisibility (TsTile *tile,
			   TsGeoView *geoView,
			   VisibilityParams *params);

void ForallQuadTree (QuadTile *quad, 
		     void (*func) (QuadTile *, void*), void *closure);
void QuadTreeDeref (QuadTile *quad);

int RequestVisible (TsGeoView *geoView, QuadTile *quad, int requestPrio);

#if 0
int GenVisibleGeoList (TsGeoView *geoView, v3f const *eye,
		       QuadTile const *node, eqhash_t in_tree,
		       TextureImage *im,
		       VisibleList *list);
#endif

#define TileOnVisibleList(in_list,tile)				\
  (!! eqhash_get (in_list, (void*) (tile)))


int TileBackfacing (TsTile const *tile, v3f const *eye);

/* Create a new VisibleGeoList for a QuadTree within an existing VisibleList.
 */
VisibleGeoList * AddVisibleGeoList (VisibleList *list,
				    TsGeoView *geoView, v3f const *eye,
				    QuadTile const *node,
				    TextureImage *im,
				    int *max_textures);


int AdjustVisibleList (VisibleList *list, int *max_textures);

void ApplyVisibleList (VisibleList *list, 
		       eqhash_t in_tree,
		       VisibleList const *prev,
		       int *max_textures);

#endif
