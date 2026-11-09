/* -*- tab-width: 8; -*-

  Name:  Visible.c
 
  Description: Generates visible tile lists and request tile lists.
 
  Revision History:
      $Id: Visible.c,v 2.85.2.11 2002/10/31 05:26:43 reddy Exp $

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Trace.h"
#include "Visible.h"
#include "Texture.h"

#include <assert.h>

static char const Visible_rcsid[] = "$Id: Visible.c,v 2.85.2.11 2002/10/31 05:26:43 reddy Exp $";

static QuadTile *
QuadTreeAdd (TsTile const *tile,
	     TsGeoView *geoView,
	     PRBool leaf,
	     PRBool overres,
	     VisibilityParams *params)
{
  static char const funcName[] = "QuadTreeAdd";
  QuadTile * node = 0;

  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d]",
	     tile, tile->id.x, tile->id.y, tile->id.res));

  VISTR ((" %.*s%c %p[%d %d %d]\n",
	  tile->id.res, "           ", leaf ? 'L' : 'Q', 
	  tile->cache, tile->id.x, tile->id.y, tile->id.res));

  /* Store the tile if we're saving the tree. */
  if (params->ist) {
    node = (QuadTile *) istack_alloc (params->ist, sizeof (QuadTile));
    memset (node, 0, sizeof (*node));

    node->tile = tile;
    node->parent = 0;
    node->children[0] = NULL;
    node->tree_size = 1;
    node->leaves = (leaf ? 1 : 0);
    node->new_tile = (! params->prev_tree
		      || ! eqhash_get (params->prev_tree, tile));
    node->overres = overres;
    node->params = params;
  }

  /* Store this so we can know which tiles are in tree. */
  eqhash_put (params->in_tree, (void *) tile, 
	      (node ? (void *)  node : (void *) tile));

  /* Callback for a tile add. */
  if (params->add_f)
    (*params->add_f) (tile, geoView->geo, leaf, params->closure);

  return node;
}

#define QuadTile_OVERRES ((QuadTile *) 1)

QuadTile *
CalcVisibility1 (TsTile const *tile,
		 TsGeoView *geoView,
		 int check_vis,
		 VisibilityParams *params)
{
  static char * funcName = "CalcVisibility1";
  TsGeoPyramid *geo = TsGeoViewPyramid (geoView);
  QuadTile *child_nodes[4];
  int i, j = 0;
  int ret;
  int any_visible = 0;
  TsBBox const *bbox = &tile->bbox;

  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d]",
	     tile, tile->id.x, tile->id.y, tile->id.res));
  TSMTRINC ((VIS_MASK|2, 1));

  /* Prepare the tile if possible. */
  TsGeoViewTilePrepared (geoView, (TsTile *) tile, 0);

  /* Check for visibility. */
  ret = (*params->vis_f) (tile, geo, check_vis, params->closure);
  if (ret == VIS_FAIL) {
    VISTR ((" %.*s%c %p[%d %d %d]\n",
	    tile->id.res, "           ", 'X', 
	    tile->cache, tile->id.x, tile->id.y, tile->id.res));

    TSMTRINC ((VIS_MASK|2, -1));
    TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] = NULL",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return NULL;
  } else if (ret == VIS_OVERRES) {
    TSMTRINC ((VIS_MASK|2, -1));
    TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] = QuadTile_OVERRES",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return QuadTile_OVERRES;
  }

  /* The VIS_OVERRES bit implies that we should use the tile, but prevent
     recursion. */
  if (! (ret & VIS_OVERRES)) {
    /* Check each child for visibility. */
    TsTile *children[4];
    int nchildren = TsGeoChildren (tile, children);
    int overres = 0;
    TSMTRACE ((VIS_MASK|3, funcName, "%d children", nchildren));

    /* Turn off visibility checking of children if VIS_ALLIN returned */
    if (ret & VIS_ALLIN)
      check_vis = FALSE;

    /* Hold references to the children until we decide what to do with them. */
    for (i = 0; i < nchildren; ++i)
      TsCacheTileRef (children[i]);

    /* Now check visibility status of each. */
    for (i = 0; i < nchildren; ++i) {
      TsTile const *ctile = children[i];
      QuadTile *child;
      /* TsGeoTileCheckNesting (geo, ctile, tile);*/
      child = CalcVisibility1 (ctile, geoView, check_vis, params);
      if (child == QuadTile_OVERRES)
	++overres;
      else if (child)
	++any_visible;
      child_nodes[i] = child;

      /* Check for consistent tree and barf if not. */
      if (0 && TsCacheTilePrepared (ctile)) {
	TsBBox const *cbbox = &ctile->bbox;
	int j;
	for (j = 0; j < 3; ++j) {
	  if (cbbox->min[j] < bbox->min[j] || cbbox->max[j] > bbox->max[j]) {
	    fprintf (stderr, "%p[%d %d %d] vs %p[%d %d %d] nesting failure\n",
		     tile, tile->id.x, tile->id.y, tile->id.res,
		     ctile, ctile->id.x, ctile->id.y, ctile->id.res);
	    abort ();
	  }
	}
      }
    }

    /* We need some non-overres children to include any at all, so the
       any_visible variable counts these. */
    if (any_visible) {
      /* Collapse the list of nodes, including any overres children as
	 well.  This involves the slightly kludgy call to QuadTreeAdd(), but
	 this is the first that we can determine we need to add it. */
      for (i = 0, j = 0; i < nchildren; ++i) {
	QuadTile *child = child_nodes[i];
	if (child) {
	  if (child == QuadTile_OVERRES) {
	    TsTile const *chtile = children[i];
	    child = QuadTreeAdd (chtile, geoView, TRUE, TRUE, params);
	    TSMTRACE ((VIS_MASK|2, funcName, "overres %p[%d %d %d] = %p",
		       chtile, chtile->id.x, chtile->id.y, chtile->id.res,
		       child));
	  }
	  child_nodes[j++] = child;
	} else {
	  /* We're not going to use this one. */
	  TsCacheTileDeref (children[i]);
	}
      }
    } else if (nchildren) {
      /* Forget about the children */
      for (i = 0; i < nchildren; ++i)
	TsCacheTileDeref (children[i]);

      /* This selects the case where we have children but none of them are
	 visible (all children overres is OK).  In this case, the visibility
	 of the low-res tile is an "illusion" and we can punt. */
      if (! overres) {
	TSMTRINC ((VIS_MASK|2, -1));
	TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] = NULL",
		   tile, tile->id.x, tile->id.y, tile->id.res));
	return NULL;
      }
    }
  }

  /* Allocate the node and insert children. */
  {
    QuadTile *node = QuadTreeAdd (tile, geoView,
				  j == 0, (ret & VIS_OVERRES),
				  params);

    for (i = 0; i < j; ++i) {
      QuadTile *child = child_nodes[i];
      node->children[i] = child;
      node->tree_size += child->tree_size;
      node->leaves += child->leaves;
      child->parent = node;
      TSMTRACE ((VIS_MASK|2, funcName, "child[%d] = %p", i, child));
    }

    /* Terminate list if needed. */
    if (i < 4) node->children[i] = NULL;

    TSMTRINC ((VIS_MASK|2, -1));
    TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] = %p",
	       tile, tile->id.x, tile->id.y, tile->id.res, node));
    return node;
  }
}

QuadTile *
CalcVisibility (TsTile *tile,
		TsGeoView *geoView,
		VisibilityParams *params)
{
  static char * funcName = "CalcVisibility";
  QuadTile *node = 0;
  TsGeoPyramid *geo = TsGeoViewPyramid (geoView);

  VISTR (("=== CalcVisibility\n"));
	  
  TSMTRACE ((VIS_MASK|1, funcName, "\"%s\"", TsGeoName (geo)));
  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d]",
	     tile, tile->id.x, tile->id.y, tile->id.res));
  TSMTRINC ((VIS_MASK|2, 1));
  
  /* Start at the top of the pyramid. */
  TsCacheTileRef (tile);
  node = CalcVisibility1 (tile, geoView, TRUE, params);

  /* An overres top-level tile should be used. */
  if (node == QuadTile_OVERRES) {
    node = QuadTreeAdd (tile, geoView, TRUE, FALSE, params);
    node->children[0] = NULL;
  } else if (! node) {
    TsCacheTileDeref (tile);
  }

  TSMTRINC ((VIS_MASK|2, -1));

  return node;
}

void
ForallQuadTree (QuadTile *quad, 
		void (*func) (QuadTile *, void*), void *closure)
{
  int i;
  for (i = 0; i < 4; ++i) {
    QuadTile *child = quad->children[i];
    if (! child)
      break;
    ForallQuadTree (child, func, closure);
  }

  (*func)(quad, closure);
}

static void
FreeVisibilityParams (VisibilityParams *params)
{
  if (params->free_f)
    (*params->free_f) (params);
}

static void
QuadTreeDeref1 (QuadTile *quad)
{
  static char const funcName[] = "QuadTreeDeref1";
  if (quad) {
    int i;
    TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d]", quad->tile,
	       quad->tile->id.x, quad->tile->id.y, quad->tile->id.res)); 
    TSMTRINC ((VIS_MASK|2, 1));
    for (i = 0; i < 4; ++i) {
      QuadTile *child = quad->children[i];
      if (! child)
	break;
      QuadTreeDeref1 (child);
    }
    TSMTRINC ((VIS_MASK|2, -1));

    TsCacheTileDeref (quad->tile);
  }
}

void
QuadTreeDeref (QuadTile *quad)
{
  /*static char const funcName[] = "QuadTreeDeref";*/
  VisibilityParams *params = quad->params;

  QuadTreeDeref1 (quad);
  if (params) FreeVisibilityParams (params);
}

/* Breadth-first search of quad-tree */
int
RequestVisible (TsGeoView *geoView, QuadTile *quad, int requestPrio)
{
  static char const funcName[] = "RequestVisible";
  int n = 0;

  if (quad && quad->tree_size) {
    QuadTile **qlist = (QuadTile **) alloca (quad->tree_size * sizeof(*qlist));
    QuadTile **qend = qlist;

    TSMTRACE ((VIS_MASK|1, funcName, ""));
    TSMTRINC ((VIS_MASK|1, 1));
  
    *qend++ = quad;
    while (qlist < qend) {
      int i, prio;
      quad = *qlist++;
      /* The deeper the tree under a node, the higher its priority should be!
       * --leei
       */
      prio = requestPrio*1000 + quad->tree_size;
      if (! TsGeoViewTilePrepared (geoView, (TsTile *) quad->tile, &prio)) {
	TsTile const *tile = quad->tile;
	VISTR (("<< %p[%d %d %d] prio %d\n",
		tile, tile->id.x, tile->id.y, tile->id.res, prio));
	TSMTRACE ((VIS_MASK|2, funcName, "Req %p[%d %d %d]",
		   tile, tile->id.x, tile->id.y, tile->id.res));
	++n;
      }
      for (i = 0; i < 4; ++i) {
	QuadTile *child = quad->children[i];
	if (! child)
	  break;
	*qend++ = child;
      }
    }
    TSMTRACE ((VIS_MASK|1, funcName, "Req %d tiles", n));
    TSMTRINC ((VIS_MASK|1, -1));
  }

  return n;
}

void
VisibleInit (VisibleList *list, TextureCache *textureCache)
{
  list->n = 0;
  list->len = 0;
  list->geo_list = 0;
  list->ist = istack_create (128 * sizeof (VisibleStruct));
  list->in_list = eqhash_create (512);
  list->textureCache = textureCache;
  list->ntiles = 0;
  list->maxTiles = 0;

  list->add_cb = 0;
  list->remove_cb = 0;

  /* Initialize bounding box */
  TsBBoxInit (&list->bbox);
  list->bbox.coordType = TsCoordGCC;
}

void
VisibleGeoInit (VisibleGeoList *geo_list)
{
  memset (geo_list, 0, sizeof (*geo_list));
}

void
VisibleGeoClear (VisibleGeoList *geo_list)
{
  static char const funcName[] = "VisibleGeoClear";

  VisibleStruct **vp = geo_list->visp, **end = vp + geo_list->n;
  TextureImage const *texim = geo_list->im;

  TSMTRACE ((VIS_MASK|2, funcName, "%p->lastOfRes == %p",
	     geo_list, geo_list->lastOfRes));

  for (; vp < end; ++vp) {
    VisibleStruct *vis = *vp;

    if (texim) {
      if (vis->draw.tex.ref)
	TexRefRelease (vis->draw.tex.ref);
    } else {
      if (vis->draw.tex.node)
	TextureDeref (vis->draw.tex.node);
    }

    TsCacheTileDeref ((TsTile *) vis->draw.tile);
  }

  free (geo_list->visp);
  free (geo_list->lastOfRes);
  geo_list->n = 0;
  geo_list->visp = 0;
  geo_list->lastOfRes = 0;
}


void
VisibleClear (VisibleList *list)
{
  /*static char const funcName[] = "VisibleClear";*/
  int i;
  for (i = 0; i < list->n; ++i)
    VisibleGeoClear (list->geo_list[i]);

  list->n = 0;
  list->ntiles = 0;
  list->maxTiles = 0;

  /* Initialize bounding box */
  TsBBoxInit (&list->bbox);
  list->bbox.coordType = TsCoordGCC;

  istack_clear (list->ist);
  if (list->in_list)
    eqhash_clear (list->in_list);
}

void
VisibleDestroy (VisibleList *list)
{
  /*static char const funcName[] = "VisibleDestroy";*/

  if (list->n)
    VisibleClear (list);

  if (list->geo_list)
    free (list->geo_list);

  if (list->in_list)
    eqhash_destroy (list->in_list);

  istack_destroy (list->ist);
}

#if 0
/*
 * Determine whether the tile referred to by QuadTile node and all of its
 * children should be included in the VisibleList being generated on istack
 * ob. There are two variables which differ depending on the popping strategy
 * being used: a "leaf" tile is one for which we will not recurse down to its
 * children, an "included" tile is to be inserted into the VisibleList. There
 * are a number of strategies available:
 *
 * VisiblePopping_None: Leaf tiles have either no children or not all of their
 *	children are ready.  All and only leaf tiles are included.
 *
 * VisiblePopping_LowRes: Leaf tiles have no ready children.  A tile is
 *	included if it is a leaf or if it has some but not all of its children
 *	ready.  A non-leaf tile which is included is flagged VISFLAG_LOWRES.
 *
 * VisiblePopping_LowFrag: Leaf tiles have no ready children.  A tile is
 *	included if it is a leaf or if it has some but not all of its children
 *	ready.  A non-leaf tile which is included is flagged VISFLAG_LOWRES
 *      and includes a set of flags defining which children are available.
 */

int
GenVisibleGeoList (TsGeoView *geoView, v3f const *eye,
		   QuadTile const *node, eqhash_t in_tree,
		   TextureImage *im,
		   VisibleList *list)
{
  static char *funcName = "GenVisibleGeoList";

  TsGeoPyramid *geo = geoView->geo;
  list_t *l = list_create (0);
  istack *ist = list->ist;
  eqhash_t in_list = list->in_list;
  VisiblePoppingStrategy poppingStrategy = list->poppingStrategy;

  /* Create a new VisibleGeoList and use it. */
  VisibleGeoList *geo_list = istack_blank (ist, sizeof (*geo_list));

  /* Make sure there's space for this new geo_list */
  if (list->n >= list->len) {
    int space;
    list->len = (list->len==0 ? 8 : list->len * 2);
    space = list->len * sizeof (*list->geo_list);
    if (! list->geo_list)
      list->geo_list = malloc (space);
    else
      list->geo_list = realloc (list->geo_list, space);
  }
  /* And place it in the list */
  list->geo_list[list->n++] = geo_list;

  /* Now initialize it... */
  geo_list->n = 0;
  geo_list->len = 64;
  geo_list->visp = malloc (geo_list->len * sizeof (geo_list->visp[0]));
  geo_list->geo = geo;
  geo_list->geoView = geoView;
  geo_list->quad = node;
  geo_list->im = im;

  /* Initialize bounding box */
  TsBBoxInit (&geo_list->bbox);
  geo_list->bbox.coordType = TsCoordLVCS;

  TSMTRINC ((VIS_MASK|1, 1));

  list_shift (l, (void*) node);

  while ((node = list_unshift (l)) != NULL) {
    TsTile const *tile = node->tile;
    int leaf, include;
    unsigned int flags, prepared_bits = 0;
    int i;
    int seamable = TRUE;

    /* Check whether we can fix seams around this tile if we need to */
    {
      TsTile const *parent = node->parent ? node->parent->tile : 0;
      TsTile const *pneighs[9], *neigh;
      if (parent) {
	TsGeoNeighbours (geo, parent, (TsTile **)pneighs);
	seamable  = (!! (neigh = pneighs[(tile->id.x%2) ? TsGeo_E : TsGeo_W])
		     || ! eqhash_get (in_tree, neigh)
		     || eqhash_get (in_list, neigh));
	TSMTRACE ((VIS_MASK|1, funcName,
		   "%p[%d %d %d] %s <= [%2d %2d %2d] %s%s",
		   tile, tile->id.x, tile->id.y, tile->id.res,
		   seamable ? "good" : "bad ",
		   (neigh ? neigh->id.x : -1),
		   (neigh ? neigh->id.y : -1),
		   (neigh ? neigh->id.res : -1),
		   (neigh && eqhash_get (in_tree, neigh)) ? "T" : "",
		   (neigh && eqhash_get (in_list, neigh)) ? "L" : ""));
	if (seamable) {
	  seamable = (!! (neigh = pneighs[(tile->id.y%2) ? TsGeo_N : TsGeo_S])
		      || ! eqhash_get (in_tree, neigh)
		      || eqhash_get (in_list, neigh));
	  TSMTRACE ((VIS_MASK|1, funcName,
		     "%p[%d %d %d] %s <= [%2d %2d %2d] %s%s",
		     tile, tile->id.x, tile->id.y, tile->id.res,
		     seamable ? "good" : "bad ",
		     (neigh ? neigh->id.x : -1),
		     (neigh ? neigh->id.y : -1),
		     (neigh ? neigh->id.res : -1),
		     (neigh && eqhash_get (in_tree, neigh)) ? "T" : "",
		     (neigh && eqhash_get (in_list, neigh)) ? "L" : ""));
	}
      }
    }
    
    /* The LEAF variable determines whether or not we are going to recurse
       down and look for higher resolution children.
       The INCLUDE variable determines whether this tile should be included in
       the rendered set. */

    if (poppingStrategy == VisiblePopping_None) {
      leaf = FALSE;
      for (i = 0; i < 4; ++i) {
	QuadTile *child = node->children[i];
	int prepared;
	if (! child)
	  break;
	prepared = !!TsGeoViewTilePrepared (geoView, (TsTile *)child->tile, 0);
	prepared_bits |= (prepared << i);
	if (! prepared) {
	  TsTile const *ctile = child->tile;
	  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] need %p[%d %d %d]",
		     tile, tile->id.x, tile->id.y, tile->id.res,
		     ctile, ctile->id.x, ctile->id.y, ctile->id.res));
	  leaf = TRUE;
	  break;
	}
      }
      if (i == 0)
	leaf = TRUE;
      include = leaf;
      flags = 0;
    } else {
      include = FALSE;
      leaf = TRUE;
      flags = 0;
      for (i = 0; i < 4; ++i) {
	QuadTile *child = node->children[i];
	int prepared;
	if (! child)
	  break;
	prepared = !!TsGeoViewTilePrepared (geoView, (TsTile *)child->tile, 0);
	prepared_bits |= (prepared << i);
	if (prepared)
	  leaf = FALSE;
	else {
	  TsTile const *ctile = child->tile;
	  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] need %p[%d %d %d]",
		     tile, tile->id.x, tile->id.y, tile->id.res,
		     ctile, ctile->id.x, ctile->id.y, ctile->id.res));
	  include = TRUE;
	  if (poppingStrategy == VisiblePopping_LowFrag) {
	    int E = child->tile->id.x % 2;
	    int N = child->tile->id.y % 2;
	    flags |= (E
		      ? (N ? VISFLAG_SHOW_NE : VISFLAG_SHOW_SE)
		      : (N ? VISFLAG_SHOW_NW : VISFLAG_SHOW_SW));
	  }
	}
      }
      if (leaf)
	include = TRUE;
      else if (include)
	flags |= VISFLAG_LOWRES;
    }

    TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d]%s%s",
	       tile, tile->id.x, tile->id.y, tile->id.res,
	       leaf ? " leaf" : "", include ? " include" : ""));

    /* Check supposedly included tiles for backface culling */
    if (include && eye && TileBackfacing (tile, eye)) {
      TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] backfacing",
		 tile, tile->id.x, tile->id.y, tile->id.res));
      include = FALSE;
    }

    /* Add any nodes to be included in the rendered set. */
    if (include) {
      VisibleStruct *vis = istack_blank (ist, sizeof (VisibleStruct));
      vis->geo = geo;
      vis->quad = node;
      vis->draw.flags = flags;
      vis->draw.tile = tile;
      vis->draw.prim = 0;
      vis->draw.tex.ref = (im ? TexRefFromTile (im, tile) : 0);

      /* Make sure there is enough room... */
      if (geo_list->n >= geo_list->len) {
	geo_list->len *= 2;
	geo_list->visp = realloc (geo_list->visp,
				  geo_list->len * sizeof (geo_list->visp[0]));
      }
      /* ... then add vis to list */
      geo_list->visp[geo_list->n++] = vis;

      TsCacheTileRef (tile);
      if (in_list)
	eqhash_put (in_list, (void*) tile, (void*) vis);

      TSMTRACE ((VIS_MASK|1, funcName, "%p[%d %d %d]%s",
		 tile, tile->id.x, tile->id.y, tile->id.res,
		 leaf ? " leaf" : ""));
      
      /* update the bounding box of all visible tiles in the list */
      TsBBoxAddBBox (&geo_list->bbox, &tile->bbox);
    }

    /* Walk down if not a leaf node. */
    if (! leaf) {
      for (i = 0; i < 4; ++i) {
	QuadTile *child = node->children[i];
	if (! child) break;
	TSMTRACE ((VIS_MASK|2, funcName, "  child %p[%d %d %d]",
		   child->tile, child->tile->id.x, child->tile->id.y,
		   child->tile->id.res));
	if (prepared_bits & (1<<i))
	  list_push (l, child);
      }
    }
  }

  list_destroy (l);

  if ( geo_list->n > 0 ) {
    float const *corner[2];
    int i, j, k;

    TsBBoxFinish (&geo_list->bbox);

    corner[0] = geo_list->bbox.min;
    corner[1] = geo_list->bbox.max;

    /* Project all corners and accumulate minmax values. */
    for (i = 0; i < 2; ++i) {
      for (j = 0; j < 2; ++j) {
	for (k = 0; k < 2; ++k) {
	  double g[3];
	  TsGeoLVCStoGCC (geo, corner[i][0], corner[j][1], corner[k][2], g);
	  TsBBoxAddPoint (&list->bbox, g[0], g[1], g[2]);
	}
      }
    }

    TsBBoxFinish (&list->bbox);
  }

  list->ntiles += geo_list->n;

  TSMTRACE ((VIS_MASK|2, funcName, "children %d", geo_list->n));
  TSMTRINC ((VIS_MASK|1, -1));

  return geo_list->n;
}
#endif

static char const *
VisibleFlagsString (int flags)
{
  static char buf[6];
  buf[0] = (flags&VISFLAG_LOWRES) ? '_' : ' ';
  buf[1] = (flags&VISFLAG_SHOW_NE) ? '0' : ' ';
  buf[2] = (flags&VISFLAG_SHOW_NW) ? '1' : ' ';
  buf[3] = (flags&VISFLAG_SHOW_SE) ? '2' : ' ';
  buf[4] = (flags&VISFLAG_SHOW_SW) ? '3' : ' ';
  buf[5] = '\0';
  return buf;
}

/*
 * Here, I need to determine whether or not adding this tile will create an
 * unfixable seam.  After playing around, it seems that the right criterion is
 * to determine whether the neighbouring tile is more than one level of detail
 * away from the current level.  So, we look next door and then up the
 * hierarchy two levels.  The first neighbouring tile we find is the relevant
 * one and if it is two levels up then we fail.
 */

static int
VisibleStructNeigh (TsGeoPyramid const *geo,
		    TsTile const *tile, TsTile const *ptile,
		    int off_x, int off_y,
		    eqhash_t in_list,
		    VisibleStruct **vnp)
{
  static char const funcName[] = "VisibleStructNeigh";
  int tx = tile->id.x, ty = tile->id.y, res = tile->id.res;
  int ok = 1;
  TsTile const *n;
  VisibleStruct *vn = 0;

  if (ptile)
    TSMTRACE ((VIS_MASK|3, funcName,
	       "[%d %d %d]->[%d %d %d] parent [%d %d %d]",
	       tx, ty, res, tx+off_x, ty+off_y, res,
	       ptile->id.x, ptile->id.y, ptile->id.res));
  else
    TSMTRACE ((VIS_MASK|3, funcName, "[%d %d %d]->[%d %d %d]",
	       tx, ty, res, tx+off_x, ty+off_y, res));

  if (TsGeoTileValid (geo, tx+off_x, ty+off_y, res)
      && (n = TsGeoTile (geo, tx+off_x, ty+off_y, res))) {
    vn = eqhash_get (in_list, n);
    TSMTRACE ((VIS_MASK|3, funcName, "[%d %d %d]->[%d %d %d] %sin_list",
	       tx, ty, res, tx+off_x, ty+off_y, res, vn ? "" : "!"));
    if (! vn && ptile) {
      int px = ptile->id.x+off_x;
      int py = ptile->id.y+off_y;
      int pres = ptile->id.res;
      if (TsGeoTileValid (geo, px, py, pres)
	  && (n = TsGeoTile (geo, px, py, pres))) {
	vn = eqhash_get (in_list, n);
	TSMTRACE ((VIS_MASK|3, funcName, "[%d %d %d]->[%d %d %d] %sin_list",
		   tx, ty, res, px, py, pres, vn ? "" : "!"));
	/* Check that parent and grandparent share relevant boundary */
	if (! vn && (off_x == (px%2==0 ? 1 : -1)
		     || off_y == (py%2==0 ? 1 : -1))) {
	  /* We now go up 2 levels, if this tile is in_list, then we fail */
	  if (TsGeoTileValid (geo, px/2, py/2, pres+1)
	      && (n = TsGeoTile (geo, px/2, py/2, pres+1))) {
	    vn = eqhash_get (in_list, n);
	    ok = ! vn;
	    TSMTRACE ((VIS_MASK|3, funcName,
		       "[%d %d %d]->[%d %d %d] %sin_list%s",
		       tx, ty, res, px/2, py/2, pres+1,
		       vn ? "" : "!", ok ? "" : " <= FAIL"));
	  }
	}
      }
    }
  }

  if (vn)
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %2d,%2d  = [%d %d %d]%s",
	       tx, ty, res, off_x, off_y,
	       n->id.x, n->id.y, n->id.res, ok ? " OK" : ""));
  else
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %2d,%2d%s",
	       tx, ty, res, off_x, off_y, ok ? " OK" : ""));
    
  *vnp = vn;
  return ok;
}

static int
VisibleHasNeighbours (TsGeoPyramid const *geo,
		      TsTile const *tile, TsTile const *parent,
		      eqhash_t in_list,
		      VisibleStruct *neighs[2])
{
  int tx = tile->id.x, ty = tile->id.y;
  int nx = (tx%2 == 0 ? -1 : 1);
  int ny = (ty%2 == 0 ? -1 : 1);

  return (VisibleStructNeigh (geo, tile, parent, nx,  0,
			      in_list, &neighs[0])
	  && VisibleStructNeigh (geo, tile, parent,  0, ny,
				 in_list, &neighs[1]));
}

static void
VisibleStructSetNeighs (VisibleStruct *vis, VisibleStruct *neighs[2])
{
  static char const funcName[] = "VisibleStructSetNeighs";
  TsTile const *tile = vis->draw.tile;
  VisibleStruct *ovis;
  TsTile const *otile;
  int W = (tile->id.x%2 == 0);
  int S = (tile->id.y%2 == 0);
  vis->neigh[W ? 0 : 2] = neighs[0];
  vis->neigh[W ? 2 : 0] = 0;
  vis->neigh[S ? 1 : 3] = neighs[1];
  vis->neigh[S ? 3 : 1] = 0;

  if (neighs[0])
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %c neigh [%d %d %d]",
	       tile->id.x, tile->id.y, tile->id.res,
	       W ? 'W' : 'E', 
	       neighs[0]->draw.tile->id.x, neighs[0]->draw.tile->id.y,
	       neighs[0]->draw.tile->id.res));
  if (neighs[1])
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %c neigh [%d %d %d]",
	       tile->id.x, tile->id.y, tile->id.res,
	       S ? 'S' : 'N', 
	       neighs[1]->draw.tile->id.x, neighs[1]->draw.tile->id.y,
	       neighs[1]->draw.tile->id.res));

  /* Update neighbouring struct if necessary */
  ovis = neighs[0];
  if (ovis && ovis->draw.tile->id.res == tile->id.res) {
    otile = ovis->draw.tile;
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %c neigh [%d %d %d]",
	       otile->id.x, otile->id.y, otile->id.res,
	       W ? 'E' : 'W', tile->id.x, tile->id.y, tile->id.res));
    ovis->neigh[W ? 2 : 0] = vis;
  }
  ovis = neighs[1];
  if (ovis && ovis->draw.tile->id.res == tile->id.res) {
    otile = ovis->draw.tile;
    TSMTRACE ((VIS_MASK|2, funcName, "[%d %d %d] %c neigh [%d %d %d]",
	       otile->id.x, otile->id.y, otile->id.res,
	       S ? 'N' : 'S', tile->id.x, tile->id.y, tile->id.res));
    ovis->neigh[S ? 3 : 1] = vis;
  }
}

static int
VisibleStructInit (VisibleList *list, VisibleGeoList *geo_list,
		   VisibleStruct *vis,
		   QuadTile const *quad,
		   VisibleStruct *neighs[2])
{
  static char const funcName[] = "VisibleStructInit";
  TsTile const *tile = quad->tile;
  QuadTile const *parent = quad->parent;
  int new_texture = 0;

  TSMTRACE ((VIS_MASK|2, funcName, "%p %p[%d %d %d] quad %p",
	     vis, tile, tile->id.x, tile->id.y, tile->id.res, quad));

  /* Now initialize this VisibleStruct */
  vis->geoView = geo_list->geoView;
  vis->quad = quad;
  vis->parent = (parent ? parent->tile : 0);
  vis->draw.flags = VISFLAG_SHOW_ALL;
  vis->draw.tile = tile;
  vis->draw.prim = 0;
  vis->draw.tex.ref = 0;

  /* Establish TextureImage binding if being used */
  if (geo_list->im) {
    vis->draw.tex.ref = TexRefFromTile (geo_list->im, tile);
    /* If this is a new texture, then track and possibly break out. */
    if (! TexRefID (vis->draw.tex.ref))
      new_texture = 1;
    TSMTRACE ((VIS_MASK|2, funcName, "%p %p[%d %d %d] texref %p%s",
	       vis, tile, tile->id.x, tile->id.y, tile->id.res,
	       vis->draw.tex.ref, new_texture ? " NEW" : ""));
  }

  /* Find neighbours for tile */
  if (neighs) {
    VisibleStructSetNeighs (vis, neighs);
  } else {
    int i;
    for (i = 0; i < 4; ++i)
      vis->neigh[i] = 0;
  }

  /* Callback when adding a new tile */
  if (list->add_cb)
    (*list->add_cb) (list, geo_list, vis, list->add_closure);

  return new_texture;
}

static void
VisibleTileRemember (char const *funcName, 
		     VisibleList *list,
		     TsTile const *tile, VisibleStruct *vis)
{
  TsCacheTileRef ((TsTile *) tile);
  if (list->in_list)
    eqhash_put (list->in_list, tile, vis);
}

static void
VisibleTileForget (char const *funcName, 
		   VisibleList *list, TsTile const *tile)
{
  TsCacheTileDeref ((TsTile *) tile);
  if (list->in_list)
    eqhash_delete (list->in_list, tile);
}

static void
VisibleGeoCheckConsistency1 (VisibleList const *list,
			     VisibleGeoList const *geo_list,
			     VisibleStruct const *vis)
{
  VisiblePoppingStrategy strategy = list->poppingStrategy;
  eqhash_t in_list = list->in_list;
  QuadTile const *quad = vis->quad;

  switch (strategy) {
  case VisiblePopping_None: {
    int i;
    for (i = 0; i < 4; ++i) {
      QuadTile const *child = quad->children[i];
      if (! child) break;
      assert (! eqhash_get (in_list, child->tile));
    }
    break;
  }

  case VisiblePopping_LowFrag:
  case VisiblePopping_LowRes:
    break;
  }
}

static void
VisibleGeoCheckConsistency (VisibleList const *list,
			    VisibleGeoList const *geo_list,
			    VisibleStruct const *vis)
{
  if (vis->parent) {
    VisibleStruct const *parent = eqhash_get (list->in_list, vis->parent);
    if (parent)
      VisibleGeoCheckConsistency1 (list, geo_list, parent);
  }
  VisibleGeoCheckConsistency1 (list, geo_list, vis);
}

static void
VisibleGeoCheck (VisibleGeoList const *geo_list, int loc, int res)
{
  TsGeoInfo const *info = TsGeoGetInfo (geo_list->geo);
  int minLevel = info->minLevel;
  int maxLevel = info->maxLevel;
  int n = geo_list->n;
  int pres = maxLevel;
  int i;
  PRInt16 *lastOfRes = alloca ((maxLevel+1) * sizeof (lastOfRes[0]));

  for (i = minLevel; i <= maxLevel; ++i)
    lastOfRes[i] = -1;

  for (i = 0; i < n; ++i) {
    if (i == loc) {
      assert (res <= pres);
      pres = res;
    } else {
      VisibleStruct const *vis = geo_list->visp[i];
      int ires = vis->draw.tile->id.res;
      assert (ires <= pres);
      pres = ires;
    }
    lastOfRes[pres] = i;
  }

  for (i = minLevel; i <= maxLevel; ++i)
    assert (lastOfRes[i] == geo_list->lastOfRes[i]);
}

static void
VisibleGeoSetVis (VisibleList *list, VisibleGeoList *geo_list, int idx,
		  VisibleStruct *vis, TsTile const *tile)
{
  static char const funcName[] = "VisibleGeoSetVis";
  int res = tile->id.res;
  PRInt16 *lastOfRes = geo_list->lastOfRes;

  TSMTRACE ((VIS_MASK|3, funcName, "%p->lastOf[%d] == %d",
	     geo_list, res, lastOfRes[res]));
  assert (idx < geo_list->n && geo_list->n <= geo_list->len);
  TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] = vis %p [%d %d %d]",
	     geo_list, idx, vis, tile->id.x, tile->id.y, res));
  geo_list->visp[idx] = vis;

  /* Update lastOfRes[res] */
  if (idx > lastOfRes[res]) {
    lastOfRes[res] = idx;
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]",
	       geo_list, idx, res));
  }
  TSMTRACE ((VIS_MASK|3, funcName, "%p->lastOf[%d] == %d",
	     geo_list, res, lastOfRes[res]));

  /* Check that we are placing this in a valid location */
  if (TSMTRACEP (VIS_MASK|1))
    VisibleGeoCheck (geo_list, idx, res);

  VisibleTileRemember (funcName, list, tile, vis);

  /* Update the bounding box of the geo_list */
  TsBBoxAddBBox (&geo_list->bbox, &tile->bbox);
}

static void
VisibleGeoMoveBack (VisibleGeoList *geo_list,
		    VisibleStruct *vis, int idx)
{
  static char const funcName[] = "VisibleGeoMoveBack";
  int n = geo_list->n;
  PRInt16 *lastOfRes = geo_list->lastOfRes;
  VisibleStruct *ovis = geo_list->visp[idx];
  int res = ovis->draw.tile->id.res;
  int last = lastOfRes[res];

  /* We've grabbed previous one, so set it. */
  if (vis)
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] = vis %p %p[%d %d %d]",
	       geo_list, idx, vis, vis->draw.tile,
	       vis->draw.tile->id.x, vis->draw.tile->id.y,
	       vis->draw.tile->id.res));
  geo_list->visp[idx] = vis;

  assert (last >= 0);
  if (last == n-1) {
    lastOfRes[res] = n;
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]", geo_list, n, res));
    geo_list->visp[n] = ovis;
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] = vis %p[%d %d %d]",
	       geo_list, n, ovis, ovis->draw.tile->id.x,
	       ovis->draw.tile->id.y, ovis->draw.tile->id.res));
  } else {
    int oidx = last+1;
    lastOfRes[res] = oidx;
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]",
	       geo_list, oidx, res));
    VisibleGeoMoveBack (geo_list, ovis, oidx);
  }
}

void VisibleStopHere () {}

static int
VisibleGeoAddLocation (VisibleGeoList *geo_list, int res)
{
  static char const funcName[] = "VisibleGeoAddLocation";
  TsGeoInfo const *info = TsGeoGetInfo (geo_list->geo);
  int maxLevel = info->maxLevel;
  int n = geo_list->n;
  PRInt16 *lastOfRes = geo_list->lastOfRes;
  int last = lastOfRes[res];
  int loc;

  /* Extend geo_list if necessary */
  if (n+1 >= geo_list->len) {
    geo_list->len *= 2;
    geo_list->visp =
      realloc (geo_list->visp, geo_list->len * sizeof (geo_list->visp[0]));
  }

  if (last == n-1) {
    loc = lastOfRes[res] = ++last;
  } else {
    int nlast = last;
    if (nlast == -1) {
      /* Place immediately after lower res */
      int pres;
      for (pres = res+1; pres <= maxLevel; ++pres) {
	nlast = lastOfRes[pres];
	if (nlast > -1) break;
      };
      TSMTRACE ((VIS_MASK|3, funcName, "%p->lastOf[%d] == %d",
		 geo_list, pres, nlast));
    }

    loc = lastOfRes[res] = nlast+1;
  }
  TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]", geo_list, loc, res));

  if (loc < n)
    VisibleGeoMoveBack (geo_list, 0, loc);

  ++geo_list->n;

  TSMTRACE ((VIS_MASK|3, funcName, "<= %d", loc));

  /* Check that we are placing this in a valid location */
  if (TSMTRACEP (VIS_MASK|1))
    VisibleGeoCheck (geo_list, loc, res);

  return loc;
}

static void
VisibleGeoAddVis (VisibleList *list, VisibleGeoList *geo_list,
		  VisibleStruct *vis, TsTile const *tile)
{
  static char const funcName[] = "VisibleGeoAddVis";
  int loc = VisibleGeoAddLocation (geo_list, tile->id.res);

  VISTR ((" + %p %p[%d %d %d]\n", tile->cache, tile,
	  tile->id.x, tile->id.y, tile->id.res));
  VisibleGeoSetVis (list, geo_list, loc, vis, tile);

  /* Adjust flags of parent */
  if (vis->parent) {
    VisibleStruct *parent_vis = eqhash_get (list->in_list, vis->parent);
    if (parent_vis) {
      TsTile const *parent = parent_vis->quad->tile;
      parent_vis->draw.flags |= VISFLAG_LOWRES;
      if (list->poppingStrategy == VisiblePopping_LowFrag) {
	int E = tile->id.x % 2;
	int N = tile->id.y % 2;
	parent_vis->draw.flags &= ~(E
				    ? (N ? VISFLAG_SHOW_NE : VISFLAG_SHOW_SE)
				    : (N ? VISFLAG_SHOW_NW : VISFLAG_SHOW_SW));
	VISTR ((" ! %p[%d %d %d] flags %s\n", parent,
		parent->id.x, parent->id.y, parent->id.res,
		VisibleFlagsString (parent_vis->draw.flags)));
	TSMTRACE ((VIS_MASK|2, funcName, "%p[%d %d %d] flags %s",
		   parent, parent->id.x, parent->id.y, parent->id.res, 
		   VisibleFlagsString (parent_vis->draw.flags)));
      }
    }
  }

  ++list->ntiles;
}

static void
VisibleGeoReplVis (VisibleList *list, VisibleGeoList *geo_list, int idx,
		   VisibleStruct *vis, TsTile const *tile)
{
  static char const funcName[] = "VisibleGeoReplVis";
  VisibleStruct *old_vis = geo_list->visp[idx];
  TsTile const *old_tile = old_vis->draw.tile;
  int old_res = old_tile->id.res;
  int res = tile->id.res;
  PRInt16 *lastOfRes = geo_list->lastOfRes;
  PRInt16 last = lastOfRes[old_res];

  VISTR ((" - %p[%d %d %d] R\n", old_tile,
	  old_tile->id.x, old_tile->id.y, old_tile->id.res));
  VISTR ((" + %p[%d %d %d] R\n", tile,
	  tile->id.x, tile->id.y, tile->id.res));

  TSMTRACE ((VIS_MASK|2, funcName, "%p[%d] was vis = %p %p[%d %d %d]",
	     geo_list, idx, old_vis, old_tile,
	     old_tile->id.x, old_tile->id.y, old_tile->id.res));

  VisibleTileForget (funcName, list, old_tile);

  assert (old_res == res+1);
  if (idx < last) {
    VisibleStruct *mvis = geo_list->visp[last];
    geo_list->visp[idx] = mvis;
    TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] = vis %p %p[%d %d %d]",
	       geo_list, idx, mvis, mvis->draw.tile,
	       mvis->draw.tile->id.x, mvis->draw.tile->id.y,
	       mvis->draw.tile->id.res));
    idx = last;
  }

  /* Update lastOfRes[old_res], checking for deletion of last one. */
  if (--last >= 0) {
    VisibleStruct *mvis = geo_list->visp[last];
    if (mvis->draw.tile->id.res != old_res)
      last = -1;
  }
  lastOfRes[old_res] = last;
  TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]",
	     geo_list, last, old_res));

  VisibleGeoSetVis (list, geo_list, idx, vis, tile);
}

static void
VisibleGeoDelVis (VisibleList *list, VisibleGeoList * geo_list, int idx)
{
  static char const funcName[] = "VisibleGeoDelVis";
  VisibleStruct *vis = geo_list->visp[idx];
  TsTile const *tile = vis->draw.tile;
  int minLevel = TsGeoGetInfo(geo_list->geo)->minLevel;
  int res = tile->id.res;

  VISTR ((" - %p[%d %d %d]\n", tile->cache,
	  tile->id.x, tile->id.y, tile->id.res));

  TSMTRACE ((VIS_MASK|3, funcName, "[%d] X %p [%d %d %d]",
	     idx, vis, tile->id.x, tile->id.y, tile->id.res));
  TSMTRINC ((VIS_MASK|3, 1));

  /* Make this location empty */
  VisibleTileForget (funcName, list, tile);

  /* Now move higher res stuff forward to fill in empty location */
  while (res >= minLevel) {
    PRInt16 *lastOfRes = geo_list->lastOfRes;
    int oidx = lastOfRes[res];
    VisibleStruct *ovis;
    TsTile const *otile;

    if (oidx >= 0) {
      if (idx < oidx) {
	ovis = geo_list->visp[oidx];
	otile = ovis->draw.tile;
	geo_list->visp[idx] = ovis;
	TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] = vis %p [%d %d %d]",
		   geo_list, idx, ovis,
		   otile->id.x, otile->id.y, otile->id.res));
      }

      /* Now move on to the next location */
      idx = oidx--;
      lastOfRes[res] = oidx;
      TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]",
		 geo_list, oidx, res));
      if (idx < 0) break;

      /* Check to see if we've eliminated the last of this res */
      if (oidx >= 0) {
	ovis = geo_list->visp[oidx];
	otile = ovis->draw.tile;
	
	if (otile->id.res != res) {
	  lastOfRes[res] = oidx = -1;
	  TSMTRACE ((VIS_MASK|3, funcName, "%p[%d] lastOf[%d]",
		     geo_list, oidx, res));
	}
      }
    }
    --res;
  }

  --geo_list->n;
  --list->ntiles;

  TSMTRINC ((VIS_MASK|3, -1));
}

VisibleGeoList *
AddVisibleGeoList (VisibleList *list,
		   TsGeoView *geoView, v3f const *eye,
		   QuadTile const *node,
		   TextureImage *im,
		   int *max_textures)
{
  static char const funcName[] = "AddVisibleGeoList";

  /* Create a new VisibleGeoList and use it. */
  VisibleGeoList *geo_list = istack_alloc (list->ist, sizeof (*geo_list));
  VisibleStruct *vis;
  TsGeoPyramid *geo = TsGeoViewPyramid (geoView);
  TsGeoInfo const *geoInfo = TsGeoGetInfo (geo);
  int maxLevel = geoInfo->maxLevel;
  int i;

  /* Make sure there's space for this new geo_list */
  if (list->n >= list->len) {
    int space;
    list->len = (list->len==0 ? 8 : list->len * 2);
    space = list->len * sizeof (*list->geo_list);
    if (! list->geo_list)
      list->geo_list = malloc (space);
    else
      list->geo_list = realloc (list->geo_list, space);
  }
  /* And place it in the list */
  list->geo_list[list->n++] = geo_list;

  /* Now initialize it... */
  geo_list->n = 0;
  geo_list->len = 128;
  geo_list->visp = malloc (geo_list->len * sizeof (geo_list->visp[0]));
  geo_list->geo = geo;
  geo_list->geoView = geoView;
  geo_list->quad = node;
  geo_list->im = im;
  geo_list->lastOfRes = calloc (maxLevel+1, sizeof (geo_list->lastOfRes[0]));
  TSMTRACE ((VIS_MASK|3, funcName, "%p[-1] lastOf[*]", geo_list));
  for (i = 0; i <= maxLevel; ++i)
    geo_list->lastOfRes[i] = -1;

  /* Initialize bounding box */
  TsBBoxInit (&geo_list->bbox);
  geo_list->bbox.coordType = TsCoordLVCS;

  vis = istack_alloc (list->ist, sizeof (VisibleStruct));
  if (VisibleStructInit (list, geo_list, vis, node, 0))
    --*max_textures;

  VisibleGeoAddVis (list, geo_list, vis, node->tile);
  
  return geo_list;
}

static void
VisibleListUpdateBBox (VisibleList *list, VisibleGeoList const *geo_list)
{
  TsGeoPyramid const *geo = geo_list->geo;
  float const *corner[2];
  int i, j, k;

  corner[0] = geo_list->bbox.min;
  corner[1] = geo_list->bbox.max;

  /* Project all corners and accumulate minmax values. */
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 2; ++j) {
      for (k = 0; k < 2; ++k) {
	double g[3];
	TsGeoLVCStoGCC (geo, corner[i][0], corner[j][1], corner[k][2], g);
	TsBBoxAddPoint (&list->bbox, g[0], g[1], g[2]);
      }
    }
  }
}

/*
 * Assume that the QuadTree associated with a VisibleList has not changed.
 * In this case, we need simply to adjust the tileset in each of the
 * VisibleGeoLists based on which tiles are now available and the limitation
 * not to download too many new textures.
 */

int
AdjustVisibleList (VisibleList *list, int *max_textures)
{
  static char const funcName[] = "AdjustVisibleList";
  eqhash_t in_list = list->in_list;
  istack *ist = list->ist;
  VisiblePoppingStrategy poppingStrategy = list->poppingStrategy;
  int new_tiles = 0;
  int del_tiles = 0;
  int i;

  ++list->id;
  TSMTRACE ((VIS_MASK|1, funcName, "%p:%d", list, list->id));
  
  /* Highest priority pyramids are at back of list, so start there */
  for (i = list->n-1; i >= 0 && *max_textures; --i) {
    VisibleGeoList *geo_list = list->geo_list[i];
    TextureImage const *texim = geo_list->im;
    TsGeoPyramid *geo = geo_list->geo;
    TsGeoView *geoView = geo_list->geoView;
    int new_geo_tiles = 0;
    int del_geo_tiles = 0;
    int ti;
    VisibleStruct *neighs[4][2];
    TSMTRACE ((VIS_MASK|1, funcName, "geo %s max %d",
	       TsGeoName (geo), *max_textures));
    TSMTRINC ((VIS_MASK|1, 1));

    VISTR (("--- Adjust %p: %d textures\n", geo->im, *max_textures));
    for (ti = 0; ti < geo_list->n; ++ti) {
      VisibleStruct *vis = geo_list->visp[ti];
      QuadTile const *quad = vis->quad;
      QuadTile const *add_children[4];
      int j, n;
      int dont_replace = 0;
      int have_children = 0;
      int nv = 0;
      VisibleStruct const *newVis[4];

      TSMTRACE ((VIS_MASK|2, funcName, "vis[%d/%d] = %p[%d %d %d]",
		 ti, geo_list->n, quad->tile,
		 quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
      TSMTRINC ((VIS_MASK|1, 1));

      /* Don't try to subdivide a tile if its parent is still around. */
      if (0 && vis->parent && eqhash_get (in_list, vis->parent)) {
	VISTR (("   %p %p[%d %d %d] has parent\n", geo->im, quad->tile,
		quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
	TSMTRACE ((VIS_MASK|3, funcName, "parent %p[%d %d %d] in_list",
		   vis->parent, vis->parent->id.x, vis->parent->id.y,
		   vis->parent->id.res)); 
	TSMTRINC ((VIS_MASK|1, -1));
	continue;
      }

      for (j = 0, n = 0; j < 4; ++j) {
	QuadTile const *child = quad->children[j];
	if (! child) break;

	/* If not in list, then either add it or don't replace
	   the current vis */
	TSMTRACE ((VIS_MASK|3, funcName, "child %p[%d %d %d]",
		   child->tile,
		   child->tile->id.x, child->tile->id.y, child->tile->id.res));
	if (! eqhash_get (in_list, child->tile)) {
	  /* This should be a full test of whether we should add this tile to
	     the list now... That means checking for stitchable neighbours. */
	  int prepared = 
	    TsGeoViewTilePrepared (geoView, (TsTile *) child->tile, NULL);
	  if (prepared
	      && VisibleHasNeighbours (geo, child->tile, quad->tile,
				       in_list, neighs[n])) {
	    add_children[n++] = child;
	  } else {
	    TSMTRACE ((VIS_MASK|1, funcName,
		       "stop %p[%d %d %d] needs %p[%d %d %d]: %s",
		       quad->tile,
		       quad->tile->id.x, quad->tile->id.y, quad->tile->id.res,
		       child->tile,
		       child->tile->id.x, child->tile->id.y, 
		       child->tile->id.res,
		       prepared ? "neighbours" : "unprepared"));
	    if (! prepared)
	      VISTR (("   %p[%d %d %d] needs %p[%d %d %d]\n",
		      quad->tile,
		      quad->tile->id.x, quad->tile->id.y, quad->tile->id.res,
		      child->tile,
		      child->tile->id.x, child->tile->id.y, 
		      child->tile->id.res));
	    ++dont_replace;
	  }
	} else {
	  ++have_children;
	}
      }

      if (j > 0 && have_children == j) {
	/* This means that all children are already in list.  Delete. */
	TSMTRACE ((VIS_MASK|1, funcName,
		   "del %p[%d %d %d]: all children in list.", quad->tile,
		   quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
	VisibleGeoDelVis (list, geo_list, ti--);
	++del_geo_tiles;
	TSMTRINC ((VIS_MASK|1, -1));
	continue;
      }

      if (poppingStrategy == VisiblePopping_None) {
	/* We don't recurse unless all child tiles are available */
	if (n > 0 && dont_replace) {
	  TSMTRACE ((VIS_MASK|1, funcName,
		     "skip %p[%d %d %d]: all children not available.",
		     quad->tile,
		     quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
	  TSMTRINC ((VIS_MASK|1, -1));
	  continue;
	}
      }

      for (j = n-1; j >= 0; --j) {
	QuadTile const *child = add_children[j];
	VisibleStruct *new_vis = ((j > 0 || dont_replace)
				  ? istack_alloc (ist, sizeof (VisibleStruct))
				  : vis);
	/* Depending on whether we've allocated a new VisibleStruct or not. */
	if (new_vis == vis) {
	  ++del_geo_tiles;
	  VisibleGeoReplVis (list, geo_list, ti, new_vis, child->tile);

	  TSMTRACE ((VIS_MASK|1, funcName,
		     "del %p[%d %d %d]", quad->tile,
		     quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
	  if (texim && vis->draw.tex.ref) {
	    /* Presumably we were counting this then. */
	    if (! TexRefID (vis->draw.tex.ref))
	      ++*max_textures;
	    TexRefRelease (vis->draw.tex.ref);
	    vis->draw.tex.ref = 0;
	  }

	  /* Callback to signal that vis has been modified */
	  if (list->remove_cb)
	    (*list->remove_cb) (list, geo_list, vis, list->remove_closure);

	  --ti;

	  if (VisibleStructInit (list, geo_list, new_vis, child, neighs[j]))
	    --*max_textures;
	} else {
	  /* Initialize before adding, so we have parent in place. */
	  if (VisibleStructInit (list, geo_list, new_vis, child, neighs[j]))
	    --*max_textures;

	  /* ... then add vis to list */
	  VisibleGeoAddVis (list, geo_list, new_vis, child->tile);
	}

	/* Count the added tiles */
	++new_geo_tiles;
	TSMTRACE ((VIS_MASK|1, funcName,
		   "add %p[%d %d %d] %d textures left", child->tile,
		   child->tile->id.x, child->tile->id.y, child->tile->id.res, 
		   *max_textures));

	/* Save new VisibleStruct items for later consistency check. */
	if (poppingStrategy == VisiblePopping_None && TSMTRACEP (VIS_MASK|1))
	  newVis[nv++] = new_vis;

	/* Examine budget and get out if necessary. */
	if (poppingStrategy != VisiblePopping_None) {
	  if (TSMTRACEP (VIS_MASK|1))
	    VisibleGeoCheckConsistency (list, geo_list, new_vis);
	  if (*max_textures <= 0)
	    break;
	}
      }
      
      /* Examine saved VisibleStruct items and consistency check. */
      if (poppingStrategy == VisiblePopping_None && TSMTRACEP (VIS_MASK|1)) {
	VisibleStruct const *n = newVis[--nv];
	for ( ; nv >= 0; n = newVis[--nv])
	  VisibleGeoCheckConsistency (list, geo_list, n);
      }

      TSMTRINC ((VIS_MASK|1, -1));
      
      if (*max_textures <= 0) {
	VISTR (("out of budget. done\n"));
	TSMTRACE ((VIS_MASK|2, funcName, "texture threshold"));
	break;
      }
    }

    /* Update bounding boxes */
    if (new_geo_tiles) {
      TsBBoxFinish (&geo_list->bbox);
      VisibleListUpdateBBox (list, geo_list);

      new_tiles += new_geo_tiles;
      del_tiles += del_geo_tiles;
    }

    if (TSMTRACEP (VIS_MASK|3)) {
      for (ti = 0; ti < geo_list->n; ++ti) {
	VisibleStruct const *vis = geo_list->visp[ti];
	TsTile const *tile = vis->draw.tile;
	TSMTRACE ((VIS_MASK|3, funcName, "vis[%d] = %p %p[%d %d %d]",
		   ti, vis, tile, tile->id.x, tile->id.y, tile->id.res));
      }
    }

#ifdef TRACE_VIS
    VISTR (("Adjust {\n"));
    for (ti = 0; ti < geo_list->n; ++ti) {
      VisibleStruct const *vis = geo_list->visp[ti];
      TsTile const *tile = vis->draw.tile;
      VISTR ((" [%d]%p = %p[%d %d %d]\n",
	      ti, vis, tile, tile->id.x, tile->id.y, tile->id.res));
    }
    VISTR (("}\n"));
#endif

    TSMTRACE ((VIS_MASK|2, funcName, "len[%d]", geo_list->n));
    TSMTRINC ((VIS_MASK|1, -1));
  }

  if (new_tiles || del_tiles) {
    TsBBoxFinish (&list->bbox);
    TSMTRACE ((VIS_MASK|1, funcName, "changed [%d new %d del]", 
	       new_tiles, del_tiles));
  }

  return new_tiles;
}

/*
 * Test whether a tile is backfacing wrt. a given eye position.
 *
 * The tile's normal and a backfacing threshold are defined by
 * TileComputeSolidNormal, with the threshold being the cosine of the angle
 * above which a eye->point vector can be considered backfacing.
 *
 * In order to verify that a tile is backfacing, we walk through the corners
 * of the tile's bounding box.  If any of these points passes is deemed to be
 * *not* backfacing then we immediately exit with FALSE.  Only if all of them
 * pass the backfacing test will the tile be considered to be backfacing.
 *
 * N.B. Each point test requires a sqrt() to normalize the eye->point vector.
 *
 */
 
int
TileBackfacing (TsTile const *tile, v3f const *eye)
{
  static char const funcName[] = "TileBackfacing";

  v3f const *norm = TsGeoTileNormal (tile);
  float thresh = TsGeoTileNormalThreshold (tile);
  float ex = V(*eye)[0], ey = V(*eye)[1], ez = V(*eye)[2];
  int i, j, k;

  float const *corner[2];
  corner[0] = tile->bbox.min;
  corner[1] = tile->bbox.max;

  TSMTRACE ((VIS_MASK|2, funcName, 
	     "%p[%d %d %d] norm %.2f,%.2f,%.2f thresh %.3f",
	     tile, tile->id.x, tile->id.y, tile->id.res,
	     V(*norm)[0], V(*norm)[1], V(*norm)[2], thresh));
  TSMTRACE ((VIS_MASK|3, funcName, "eye %.2f,%.2f,%.2f", ex, ey, ez));
  TSMTRINC ((VIS_MASK|2, 1));

  for (i = 0; i < 2; ++i) {
    float x = corner[i][0];
    for (j = 0; j < 2; ++j) {
      float y = corner[j][1];
      for (k = 0; k < 2; ++k) {
	float z = corner[k][2];
	v3f dir;
	float inner;
	int   frontface;
	V(dir)[0] = ex - x;
	V(dir)[1] = ey - y;
	V(dir)[2] = ez - z;
	normalize_v3f (&dir);
	inner = inner_v3f (&dir, norm);
	frontface = ( inner >= thresh );

	TSMTRACE ((VIS_MASK|3, funcName, "pos %.2f,%.2f,%.2f", x, y, z));
	TSMTRACE ((VIS_MASK|2, funcName,
		   "%d%d%d: dir %.2f,%.2f,%.2f inner %.3f",
		   i,j,k, V(dir)[0], V(dir)[1], V(dir)[2], inner));

	if ( frontface ) {
	  TSMTRACE ((VIS_MASK|2, funcName, "Nope."));
	  TSMTRINC ((VIS_MASK|2, -1));
	  return FALSE;
	}
      }
    }
  }

  TSMTRACE ((VIS_MASK|2, funcName, "Backfacing."));
  TSMTRINC ((VIS_MASK|2, -1));
  return TRUE;
}

static VisibleStruct *
VisibleGeoAddCoverage (VisibleList *list, VisibleGeoList *geo_list,
		       QuadTile const *quad, int *texture)
{
  /*static char const funcName[] = "VisibleGeoAddCoverage";*/
  VisibleStruct *new_vis = istack_alloc (list->ist, sizeof (VisibleStruct));

  /* Add new_vis to list */
  if (VisibleStructInit (list, geo_list, new_vis, quad, 0))
    *texture = 1;
  VisibleGeoAddVis (list, geo_list, new_vis, quad->tile);

  /* Update bbox */
  TsBBoxAddBBox (&geo_list->bbox, &quad->tile->bbox);

  return new_vis;
}

static int
VisibleGeoQuadCovered (VisibleList *list, VisibleGeoList *geo_list,
		       QuadTile const *quad)
{
  for ( ; quad; quad = quad->parent)
    if (eqhash_get (list->in_list, quad->tile))
      return TRUE;
  return FALSE;
}

static int
VisibleGeoDelBelow (VisibleList *list, VisibleGeoList *geo_list,
		    QuadTile const *quad)
{
  int deleted = 0;
  int i;
  /* Remove any children of this tile */
  for (i = 0; i < 4; ++i) {
    QuadTile const *child = quad->children[i];
    VisibleStruct *v;
    if (! child) break;
    deleted += VisibleGeoDelBelow (list, geo_list, child);
    v = eqhash_get (list->in_list, child->tile);
    if (v) {
      int ti;
      for (ti = 0; ti < geo_list->n; ++ti) {
	if (geo_list->visp[ti] == v) {
	  VisibleGeoDelVis (list, geo_list, ti);
	  ++deleted;
	  break;
	}
      }
    }
  }
  return deleted;
}

static int
VisibleGeoCompleteCoverage (VisibleList *list, VisibleGeoList *geo_list,
			    QuadTile const *quad)
{
  static char const funcName[] = "VisibleGeoCompleteCoverage";
  int textures = 0;

  /* Ensure coverage if it is either new or a leaf tile */
  if (quad->new_tile || ! quad->children[0]) {
    QuadTile const *tquad = quad;

    TSMTRACE ((VIS_MASK|2, funcName, "Cover %p[%d %d %d]", quad->tile, 
	       quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));

    /* Go up until we find a prepared tile */
    while (! TsCacheTilePrepared (tquad->tile))
      tquad = tquad->parent;
    assert (tquad != 0);

    if (! VisibleGeoQuadCovered (list, geo_list, tquad)) {
      VisibleStruct *vis;
      int i;

      TSMTRACE ((VIS_MASK|2, funcName,
		 "Add %p[%d %d %d] above %p[%d %d %d]", tquad->tile, 
		 tquad->tile->id.x, tquad->tile->id.y, tquad->tile->id.res,
		 quad->tile,
		 quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
      vis = VisibleGeoAddCoverage (list, geo_list, tquad, &textures);

      switch (list->poppingStrategy) {
      case VisiblePopping_None: {
	/* Remove any children of this tile */
	VisibleGeoDelBelow (list, geo_list, tquad);
	break;
      }

      case VisiblePopping_LowFrag: {
	int flags = VISFLAG_SHOW_ALL;

	/* Clean up display list below this tile */
	for (i = 0; i < 4; ++i) {
	  QuadTile const *child = tquad->children[i];
	  VisibleStruct *v;
	  if (! child) break;

	  /* First, ensure that there are no tiles below this child. */
	  VisibleGeoDelBelow (list, geo_list, child);

	  /* Now fix up the fragment flags for this tile/child pair */
	  v = eqhash_get (list->in_list, child->tile);
	  if (v) {
	    int E = child->tile->id.x % 2;
	    int N = child->tile->id.y % 2;
	    flags &= ~(E
		       ? (N ? VISFLAG_SHOW_NE : VISFLAG_SHOW_SE)
		       : (N ? VISFLAG_SHOW_NW : VISFLAG_SHOW_SW));

	    /* We know that this child now has no children */
	    if (v->draw.flags != VISFLAG_SHOW_ALL) {
	      VISTR ((" ! %p[%d %d %d] flags %s\n", child->tile,
		      child->tile->id.x, child->tile->id.y,
		      child->tile->id.res,
		      VisibleFlagsString (VISFLAG_SHOW_ALL)));
	      v->draw.flags = VISFLAG_SHOW_ALL;
	    }
	  }
	}

	/* Update the flags so we know which children exist in list. */
	if (flags != VISFLAG_SHOW_ALL)
	  flags |= VISFLAG_LOWRES;
	VISTR ((" ! %p[%d %d %d] flags %s\n", tquad->tile,
		tquad->tile->id.x, tquad->tile->id.y, tquad->tile->id.res,
		VisibleFlagsString (flags)));
	vis->draw.flags = flags;

	break;
      }

      case VisiblePopping_LowRes:
	break;
      }

      if (TSMTRACEP (VIS_MASK|1))
	VisibleGeoCheckConsistency (list, geo_list, vis);
    }
  } else {
    /* Recursively complete coverage for subtree under this quad */
    int i;
    for (i = 0; i < 4; ++i) {
      QuadTile const *child = quad->children[i];
      if (! child) break;
      textures += 
	VisibleGeoCompleteCoverage (list, geo_list, child);
    }
  }

  return textures;
}

static int
VisibleGeoCheckCoverage (VisibleList *list, 
			 VisibleGeoList *geo_list,
			 QuadTile const *quad)
{
  static char const funcName[] = "VisibleGeoCheckCoverage";
  int success = TRUE;

  if (! quad->children[0]) {
    if (! VisibleGeoQuadCovered (list, geo_list, quad)) {
      TSMTRACE ((VIS_MASK|1, funcName, "! %p[%d %d %d]", quad->tile,
		 quad->tile->id.x, quad->tile->id.y, quad->tile->id.res));
      success = FALSE;
    }
  } else {
    /* Recursively complete coverage for subtree under this quad */
    int i;
    for (i = 0; i < 4; ++i) {
      QuadTile const *child = quad->children[i];
      if (! child) break;
      success &= VisibleGeoCheckCoverage (list, geo_list, child);
    }
  }

  return success;
}

/*
 * Synopsis: int ApplyVisibleGeoList (VisibleList *list,
 *                                    VisibleGeoList *geo_list,
 *                                    VisibleGeoList const *prev_list)
 *
 * Doc++ Description: *//**
 *     Apply a previous VisibleGeoList to the new QuadTile tree embodied in
 *     the VisibleGeoList geo_list.
 *
 * @param cache The tileset cache.
 *
 * @author Lee Iverson
 *
 * @return  void
 *
 * @see VisibleList, VisibleGeoList
 *
 */

int
ApplyVisibleGeoList (VisibleList *list, VisibleGeoList *geo_list,
		     eqhash_t in_tree,
		     VisibleGeoList const *prev_list,
		     int *max_textures)
{
  static char const funcName[] = "ApplyVisibleGeoList";
  int ti;
  int textures = 0;

  VISTR (("=== Apply %p: %d textures\n", geo_list->geo->im, *max_textures));

  /* First, remove the initial VisibleStruct and undo .*/
  assert (geo_list->n == 1);
  VisibleGeoDelVis (list, geo_list, 0);

  TsBBoxInit (&geo_list->bbox);
  geo_list->bbox.coordType = TsCoordLVCS;

  /* First, we walk through, inserting prev_list into geo_list when the
     prev_list tile is also in the new quadtree. */
  TSMTRACE ((VIS_MASK|2, funcName, "apply prev_list %p[%d]",
	     prev_list, prev_list->n));
  TSMTRINC ((VIS_MASK|2, 1));

  for (ti = 0; ti < prev_list->n; ++ti) {
    VisibleStruct *vis = prev_list->visp[ti];
    TsTile const *tile = vis->draw.tile;
    QuadTile *tquad = eqhash_get (in_tree, tile);
    if (tquad) {
      int textures = 0;
      VisibleGeoAddCoverage (list, geo_list, tquad, &textures);
      *max_textures -= textures;
    } else {
      TSMTRACE ((VIS_MASK|2, funcName, "discard %p[%d %d %d]",
		 tile, tile->id.x, tile->id.y, tile->id.res));
    }
  }

  TSMTRACE ((VIS_MASK|2, funcName, "complete"));

  /* Now, we need to fill in the geo_list so that all leaf nodes in the
     quadtree are *covered* by tiles in the geo_list. */
  if (TsCacheTilePrepared (geo_list->quad->tile)) {
    *max_textures -= 
      VisibleGeoCompleteCoverage (list, geo_list, geo_list->quad);

    if (TSMTRACEP (VIS_MASK|1))
      VisibleGeoCheckCoverage (list, geo_list, geo_list->quad);
  }

  /* Finalize the bounding box info */
  TsBBoxFinish (&geo_list->bbox);
  VisibleListUpdateBBox (list, geo_list);

  /* Finally go over the list again to update neighbourhood relationships */
  for (ti = 0; ti < geo_list->n; ++ti) {
    VisibleStruct *vis = geo_list->visp[ti];
    TsTile const *tile = vis->draw.tile;
    VisibleStruct *neighs[2];
    VisibleHasNeighbours (geo_list->geo, tile, 0, list->in_list, neighs);
    VisibleStructSetNeighs (vis, neighs);
  }
  
#ifdef TRACE_VIS
  VISTR (("Apply {\n"));
  for (ti = 0; ti < geo_list->n; ++ti) {
    VisibleStruct const *vis = geo_list->visp[ti];
    TsTile const *tile = vis->draw.tile;
    VISTR ((" [%d]%p = %p[%d %d %d]\n",
	    ti, tile, tile->cache, tile->id.x, tile->id.y, tile->id.res));
  }
  VISTR (("}\n"));
#endif

  TSMTRINC ((VIS_MASK|2, -1));
  TSMTRACE ((VIS_MASK|2, funcName, "need %d textures", textures));

  return textures;
}

void
ApplyVisibleList (VisibleList *list, 
		  eqhash_t in_tree,
		  VisibleList const *prev,
		  int *max_textures)
{
  static char const funcName[] = "ApplyVisibleList";
  int i, j;

  TSMTRACE ((VIS_MASK|1, funcName, "%p[%d] <= %p[%d]", 
	     list, list->ntiles, prev, prev->ntiles));
  TSMTRINC ((VIS_MASK|1, 1));

  /* Make sure generations are monotonic. */
  list->id = prev->id;

  /* Highest priority pyramids are at back of list, so start there */
  TsBBoxInit (&list->bbox);
  for (i = list->n-1; i >= 0; --i) {
    VisibleGeoList *geo_list = list->geo_list[i];
    for (j = 0; j < prev->n; ++j) {
      VisibleGeoList *prev_list = prev->geo_list[j];
      if (prev_list->geo == geo_list->geo) {
	ApplyVisibleGeoList (list, geo_list, in_tree, prev_list, max_textures);
	break;
      }
    }
  }
  TsBBoxFinish (&list->bbox);

  TSMTRINC ((VIS_MASK|1, -1));
}

static void
PrintQuadTree1 (FILE *out, QuadTile const *quad, int indent)
{
  int i;
  TsTile const *tile = quad->tile;

  for (i = 0; i < indent; ++i)
    fputc (' ', out);
  fprintf (out, "%p %p[%d %d %d] %s\n",
	   quad, tile, tile->id.x, tile->id.y, tile->id.res,
	   quad->new_tile ? "NEW" : "");

  indent += 2;
  for (i = 0; i < 4; ++i) {
    QuadTile const *child = quad->children[i];
    if (! child) break;
    PrintQuadTree1 (out, child, indent);
  }
}

void
PrintQuadTree (FILE *out, QuadTile const *quad)
{
  PrintQuadTree1 (out, quad, 0);
}

void
PrintVisibleList (FILE *out, VisibleList const *list)
{
  int i;
  fprintf (out, "VisibleList %p:%d\n", list, list->id);
  for (i = 0; i < list->n; ++i) {
    VisibleGeoList const *geo_list = list->geo_list[i];
    int j;
    fprintf (out, "[%d] geo %p '%s'\n",
	     i, geo_list->geo, TsGeoName (geo_list->geo));
    for (j = 0; j < geo_list->n; ++j) {
      VisibleStruct const *vis = geo_list->visp[j];
      TsTile const *tile = vis->draw.tile;
      int k;
      fprintf (out, "  %8p %p[%d %d %d] quad %p\n",
	       vis, tile, tile->id.x, tile->id.y, tile->id.res,
	       vis->quad);
      fprintf (out, "    neighs: ");
      for (k = 0; k < 4; ++k) {
	static char const d[] = "WSEN";
	char buf[32];
	buf[0] = '\0';
	if (vis->neigh[k]) {
	  TsTile const *ntile = vis->neigh[k]->draw.tile;
	  sprintf (buf, "%d %d %d", ntile->id.x, ntile->id.y, ntile->id.res);
	}
	fprintf (out, "%c: %-10s ", d[k], buf);
      }
      fprintf (out, "\n");
    }
  }
}
