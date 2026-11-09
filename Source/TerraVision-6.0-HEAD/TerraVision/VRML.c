/* -*- tab-width: 2; -*-

  Name: VRML.c
 
  Description: This file performs the rendering of VRML scene graphs
     in TerraVision.
 
  Revision History:
      $Id: VRML.c,v 2.34.2.13 2002/10/31 05:26:41 reddy Exp $

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

#include <stdio.h>
#include <stddef.h>
#include <math.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef CALLBACK
#define CALLBACK
#endif

#include <vrml2/vrml2.h>
#include <vrml2/c_nodes.h>
#include <vrml2/c_nodesI.h>

#include "TerraVisionInc.h"
#include "ThreeDCoreI.h"
#include "GeoVRML.h"
#include "VRML.h"

#define TRACE_ROBJS (0)
#define DISPLAY_LISTS (1)
#define TRACE_OPT (0)

static GLenum polygon_mode[] = { GL_LINE_LOOP, GL_POLYGON };

typedef enum {
  USE_NO_COLOR = 0,
  USE_DIFFUSE_COLOR,
  USE_EMISSIVE_COLOR
} renderColor;

/* The renderState structure is passed to every node during scene */
/* graph traversal. It is used to contain the state for rendering */
/* a single instance of a VRML scene.                             */

typedef struct {
  SFBool       backface_cull:1;    /* currently culling backfacing polys? */
  SFBool       anticlockwise:1;    /* current vertex ordering */
  SFBool       transparency:1;     /* is GL_BLEND turned on? */
  SFBool       smooth_shade:1;     /* is shade model GL_SMOOTH? */
  SFBool       lighting:1;         /* is lighting on for this object? */
  SFBool       color_material:1;   /* is GL_COLOR_MATERIAL enabled? */
  SFBool       alpha_trans:1;      /* is the Material's alpha < 0? */
  SFBool       texturing:1;        /* is texture mapping on? */
  SFBool       repeatS:1;          /* current ImageTexture setting */
  SFBool       repeatT:1;          /* current ImageTexture setting */
} renderFlags;

typedef struct renderState {
  struct globalState *global;
  vsgWorld     *curr_wrl;          /* current VRML file (could be inline) */
  vsgcMaterial *material;          /* current Shape's Material node */
  vsgcMaterial *bound_material;    /* the currently bound Material node */
  renderColor  bound_color;        /* the currently bound color mode */
  FeatureTexture *texture;         /* texture for current Shape */
  FeatureTexture *bound_texture;   /* the currently bound ImageTexture node */

  SFBool       compile:1;          /* Whether we are compiling or rendering */
  SFBool       polygon:1;          /* GL_POLYGON or GL_LINE_LOOP */
  SFBool       force_backface_cull:1; /* do backface cull for all objects? */
  SFBool       lightscene:1;       /* global light switch */
  SFBool       texturescene:1;     /* global texture switch */
  SFBool       in_hyperlink:1;     /* are we in a selected Anchor node? */

  renderFlags  current;		   /* Current state of flags. */
  renderFlags  pending;		   /* State flags need to be in for render. */

  m4f          tfm, inv_tfm;
  int          in_anchor;          /* are we in an Anchor node? */
  int          anchor_count;       /* number of Anchor nodes during pick */
} renderState;

/* GL state chaning routines. These are buffered to */
/* reduce state changes */

static inline void
syncBackfaceCull( renderState *state )
{
  SFBool cull = state->pending.backface_cull;

  if ( cull != state->current.backface_cull ) {
    if ( cull )
      glEnable( GL_CULL_FACE );
    else
      glDisable( GL_CULL_FACE );

    state->current.backface_cull = cull;
  }
}

static inline void
setBackfaceCull( renderState *state, int cull, int immed )
{
  if ( state->force_backface_cull )
    cull = TRUE;
  state->pending.backface_cull = cull;
  if (immed) syncBackfaceCull (state);
}

static inline void
syncVertexOrdering( renderState *state )
{
  SFBool anticlockwise = state->pending.anticlockwise;

  if ( anticlockwise != state->current.anticlockwise ) {
    if ( anticlockwise )
      glFrontFace( GL_CCW );
    else
      glFrontFace( GL_CW );

    state->current.anticlockwise = anticlockwise;
  }
}

static inline void
setVertexOrdering( renderState *state, int anticlockwise, int immed )
{
  state->pending.anticlockwise = anticlockwise;
  if (immed) syncVertexOrdering (state);
}

static inline void
syncShadeModel( renderState *state )
{
  SFBool smooth = state->pending.smooth_shade;

  if ( smooth != state->current.smooth_shade ) {
    if ( smooth )
      glShadeModel( GL_SMOOTH );
    else
      glShadeModel( GL_FLAT );
    
    state->current.smooth_shade = smooth;
  }
}

static inline void
setShadeModel( renderState *state, int smooth, int immed )
{
  state->pending.smooth_shade = smooth;
  if (immed) syncShadeModel (state);
}

/* used to indicate that a Material node has alpha < 1 */
static inline void
setAlphaTrans( renderState *state, int alpha, int immed )
{
  state->pending.alpha_trans = alpha;
  if (immed) state->current.alpha_trans = alpha;
}

static inline void
syncLighting( renderState *state )
{
  SFBool on = state->pending.lighting;
  if ( on != state->current.lighting && state->lightscene ) {
    if ( on )
      glEnable( GL_LIGHTING );
    else
      glDisable( GL_LIGHTING );

    state->current.lighting = on;
  }
}

static inline void
setLighting( renderState *state, int on, int immed )
{
  state->pending.lighting = on;
  if (immed) syncLighting (state);
}

static inline void
syncTransparency( renderState *state )
{
  SFBool on = state->pending.transparency;
  if ( on != state->current.transparency ) {
    if ( on )
      glEnable( GL_BLEND );
    else
      glDisable( GL_BLEND );

    state->current.transparency = on;
  }
}

static inline void
setTransparency( renderState *state, int on, int immed )
{
  state->pending.transparency = on;
  if (immed) syncTransparency (state);
}

static inline void
syncColorMaterial( renderState *state )
{
  SFBool on = state->pending.color_material;
  if ( on != state->current.color_material ) {
    if ( on )
      glEnable( GL_COLOR_MATERIAL );
    else
      glDisable( GL_COLOR_MATERIAL );

    state->current.color_material = on;
  }
}

static inline void
setColorMaterial( renderState *state, int on, int immed )
{
  state->pending.color_material = on;
  if (immed) syncColorMaterial (state);
}

static inline void
syncTexturing( renderState *state )
{
  SFBool on = state->pending.texturing;
  if ( on != state->current.texturing ) {
    if ( on )
      glEnable( GL_TEXTURE_2D );
    else
      glDisable( GL_TEXTURE_2D );

#if TRACE_ROBJS
    printf ("  texturing \n", on ? "on!" : "off");
#endif

    state->current.texturing = on;
  }
}

static inline void
setTexturing ( renderState *state, int on, int immed )
{
  state->pending.texturing = on;
  if (immed) syncTexturing (state);
}

static inline void
syncRender( renderState *state )
{
  syncVertexOrdering (state);
  syncShadeModel (state);
  syncBackfaceCull (state);
  syncTransparency (state);
  syncColorMaterial (state);
  syncLighting (state);
  syncTexturing (state);
}

typedef struct RenderObj RenderObj;

struct RenderObj {
  RenderObj *next;

  /*TsGeoBBox bbox;*/
  renderFlags saved;
  renderFlags enter;
  renderFlags exit;

  int in_anchor;
  vsgcMaterial *material;
  renderColor material_color;
  FeatureTexture *texture;

  m4f tfm;
  struct {
    SFBool translate:1;
    SFBool rotate:1;
    SFBool scale:1;
    SFBool tfm:1;
  } flags;
  SFVec3f T;
  SFRotation R;
  SFVec3f S;
  GLuint materialDL[2];
  GLuint geometryDL;
  MFNode *geometry;
};

static int addRenderObj (m4f const *tfm, 
			 renderState const *exit, 
			 SFNode geom);
static void finishRenderObjs (renderState *state);
static void renderRenderObjs (renderState *state);

typedef struct globalState {
  int          flags;
  vsgWorld     *root_wrl;          /* the root VRML file for this feature */
  FeatureItem  *item;              /* the feature item information */
  tvBase       *threeD;            /* the tvBase structure for the display */
  double       ref_offset[3];      /* View's reference origin location */
  double       ref_plane[4];       /* View's reference origin orientation */
  plist_t      props;
  SFBool       compiled:1;
  int          nobjs;
  RenderObj    *robjs;
  RenderObj    **vobj;
} globalState;

/* The VertexInfo structure completely describes a single vertex, */
/* including colour, normal, and texture coordinates. It is used  */
/* by renderIFS to collect information about a vertex, and also   */
/* makeConvexIFS to pass vertex info to the tesselation callbacks */

typedef struct {
  GLfloat     *vertex;                  /* pointer to a GLfloat[3] vertex */
  SFInt32     vertex_index;             /* index of current vertex in IFS */
  int         do_color;                 /* TRUE iff r, g, b are valid */
  GLfloat     r, g, b;                  /* the color of this vertex */
  SFInt32     color_index;              /* index of this colour in IFS */
  int         do_texcoords;             /* TRUE iff u, v are valid */
  GLfloat     u, v;                     /* texture coords for this vertex */
  SFInt32     texcoord_index;           /* index of this texcoord in IFS */
  int         do_normals;               /* TRUE iff n[3] is valid */ 
  GLfloat     n[3];                     /* normal vector for this vertex */
  SFInt32     normal_index;             /* index of this normal in IFS */
} VertexInfo;

/* The IFSInfo structure is used to contain state about an IFS node  */
/* during the concave->convex tessellation process. It also contains */
/* various state variable needed for the tessellation process.       */

typedef struct {
  vsgcIndexedFaceSet *ifs;              /* the original IndexedFaceSet */
  MFInt32            *coord_index;      /* the new coordinate indices */
  MFInt32            *color_index;      /* the new color indices */
  MFInt32            *normal_index;     /* the new normal indices */
  MFInt32            *texcoord_index;   /* the new texture coord indices */
  list_t             *vlist;            /* list of allocate VertexInfos */
  list_t             *dlist;            /* list of allocate GLdouble[3]s */
  vsgWorld           *wrl;              /* the parsed vrml scene graph */
  int                verts_this_poly;   /* count of vertices in polygon */
  int                first_vertex;      /* TRUE for 1st vertex after Begin */
  int                tris_this_strip;   /* no. of tris made from this strip */
  GLenum             type;              /* current GL geometric primitive */
  VertexInfo         *fan_pivot;        /* pivot vertex for GL_TRIANGLE_FAN */
  VertexInfo         *fan_last;         /* last vertex for GL_TRIANGLE_FAN */
  VertexInfo         *strip_last1;      /* last vertex (GL_TRIANGLE_STRIP) */
  VertexInfo         *strip_last2;      /* 2nd last vertex (for tri strips) */
} IFSInfo;

#define FEQ_tol(x,y,tol) (fabs((x)-(y)) <= (tol))

static int
m4f_eq (m4f const *m1, m4f const *m2)
{
  float const *p1 = M(*m1)[0], *p2 = M(*m2)[0];
  int i;
  for (i = 0; i < 16; ++i, ++p1, ++p2) {
    if (*p1 != *p2)
      return FALSE;
  }
  return TRUE;
}

static inline int
vec_zerop (SFFloat const *v)
{
  return v[0] == 0.0f && v[1] == 0.0f && v[2] == 0.0f;
}

static inline int
vec_onep (SFFloat const *v)
{
  return FEQ_tol(v[0],1.0f,1e-5) && FEQ_tol(v[1],1.0f,1e-5) && FEQ_tol(v[2],1.0f,1e-5);
}

static inline int
rot_zerop (SFRotation const *rot)
{
  return rot->d == 0.0f;
}

static void renderGL (vsgWorld *wrl, void *closure);
static void renderMFNode (MFNode *mf, renderState *state);
static void renderGroup (vsgcNode *node, renderState *state);
static void renderCollision (vsgcNode *node, renderState *state);
static void renderAppearance (vsgcNode *node, renderState *state);
static void renderAnchor (vsgcNode *node, renderState *state);
static void renderTransform (vsgcNode *node, renderState *state);
static void renderShape (vsgcNode *node, renderState *state);
static void renderText( vsgcNode *node, renderState *state );
static void rememberMaterial (vsgcNode *node, renderState *state);
static void rememberTexture (vsgcNode *node, renderState *state);
static void renderMaterial (renderState *state, renderColor which_color);
static void renderIFS (vsgcNode *node, renderState *state);
static void renderILS (vsgcNode *node, renderState *state);
static void renderSphere( vsgcNode *node, renderState *state );
static void renderCylinder( vsgcNode *node, renderState *state );
static void renderBox( vsgcNode *node, renderState *state );
static void renderCone( vsgcNode *node, renderState *state );
static void renderInline( vsgcNode *node, renderState *state );
static void autoGenTexCoords( double min[3], double max[3], int s, int t,
			      SFVec3f *point, GLfloat *u, GLfloat *v );
static void makeConvexIFS( vsgcIndexedFaceSet *ifs, renderState *state );
static void renderGeoLocation( SFNode node, renderState *state );
static void renderGeoCoordinate( SFNode node, renderState *state );
static vsgcCoordinate *convertGeoCoordinate( SFNode node, renderState *state );

static int
isPROTONode (SFNode node, renderState *state, char const *name)
{
  vsgcClass *cls = (vsgcClass *) vsgNodeClass (node);
  if ( vsgClassIsProto( &cls->c ) ) {
    symbol_t proto_name = vsgIntern( state->curr_wrl, cls->c.name );
    if ( proto_name == vsgIntern( state->curr_wrl, name ) )
      return TRUE;
  }
  return FALSE;
}

static void
renderNode (SFNode node, renderState *state)
{
  vsgcClass *cls;
  if ( node == NULL ) return;

  cls = (vsgcClass *) vsgNodeClass (node);

  /* If the node is a proto or a script, then don't do anything */
  /* later we can plug in code to deal with our custom protos.  */

  if ( vsgClassIsScript( &cls->c ) ) return;

  if ( vsgClassIsProto( &cls->c ) ) {
    if ( isPROTONode( node, state, "GeoLocation" ) )
      renderGeoLocation( node, state );
    return;
  };

  /* otherwise, render the node if we have a callback registered */

  if (cls->render) (*cls->render) ((vsgcNode *) node, state);
}

static void
renderGL (vsgWorld *wrl, void *closure)
{
  static char const *globalName = "globalState";

  MFNode          *nodes;
  plist_t         props;
  FeatureInstance *feature = (FeatureInstance *) closure;
  FeatureItem     *item;
  tvBase          *threeD;
  renderState     state;
  globalState     *global;
  tvViewpoint     view;

  /* attributes for the light we use to light the VRML scene graph */

  GLfloat diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat position[] = { 1.0f, 1.0f, -1.0f, 0.0f };
  GLfloat ambient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
  GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

  /* check for valid values and then get info from the closure */

  if ( wrl == NULL || closure == NULL ) return;

  item   = FeatureGetItem( feature->id );
  threeD = feature->threeD;

  /* are we doing a pick? If so, we only care about scenes with Anchors */

  if ( threeD->selecting && item->anchors == 0 )
    return;

  nodes  = vsgWorldNodes( wrl );
  props  = vsgWorldProperties (wrl);

  /* XXX temporary blatant hack using global geo in order to */
  /* set the light for this model overhead */

  tvBaseGetView( threeD, &view );

  position[0] = -(float) view.eye[0];
  position[1] = -(float) view.eye[1];
  position[2] = -(float) view.eye[2];
  NormalizeVector( position, position, 3 );

  /* setup the initial rendering state */
  global = plist_get (props, wrl, globalName);
  if (! global) {
    global = vsgAlloc (wrl, sizeof (*global));
    global->flags = 0;
    global->root_wrl = wrl;
    global->item = item;
    global->threeD = threeD;
    global->props = props;
    global->compiled = FALSE;
    global->robjs = 0;
    global->nobjs = 0;
    plist_put (props, wrl, globalName, global);
  }

  /* grab a copy of the current view's reference origin and orientation */

  memcpy( global->ref_offset, view.reference, sizeof(double)*3 );
  memcpy( global->ref_plane, view.reference_plane, sizeof(double)*4 );
 
  state.global = global;
  state.curr_wrl = wrl;
  state.material = NULL;
  state.bound_material = NULL;
  state.bound_color = USE_NO_COLOR;
  state.polygon = TRUE;
  state.force_backface_cull = threeD->f_backfacecull;
  state.lightscene = TRUE;
  state.texturescene = TRUE;

  state.current.backface_cull = TRUE;
  state.current.transparency = FALSE;
  state.current.smooth_shade = FALSE;
  state.current.anticlockwise = TRUE;
  state.current.lighting = FALSE;
  state.current.color_material = FALSE;
  state.current.texturing = FALSE;
  state.pending = state.current;

  state.texture = NULL;
  state.bound_texture = NULL;
  state.anchor_count = 0;
  state.in_anchor = 0;
  state.in_hyperlink = 0;

  identity_m4f (&state.tfm);
  identity_m4f (&state.inv_tfm);

  /* take a look at the tvBase settings */

  if ( threeD != NULL ) {
    state.polygon = ! threeD->f_wireframe;
    state.lightscene = threeD->f_lighting;
    state.texturescene = threeD->f_textured;
  }

#if DISPLAY_LISTS
  if (! global->compiled) {
    state.compile = TRUE;
    renderMFNode( nodes, &state );
    finishRenderObjs (&state);
    global->compiled = TRUE;
  }
#endif /* DISPLAY_LISTS */

  glPushAttrib( GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT |
		GL_TEXTURE_BIT ); {
    
    /* put the important GL settings into a known state. */
    /* many of these are reflected in the state structure */

    glShadeModel( GL_FLAT );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
    /*glEnable( GL_NORMALIZE );*/
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glDisable( GL_ALPHA_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    /* setup lighting on light1 */

    if ( state.lightscene ) {
      setLighting ( &state, FALSE, TRUE );
      glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse );
      glLightfv( GL_LIGHT1, GL_POSITION, position );
      glLightfv( GL_LIGHT1, GL_AMBIENT, ambient );
      glLightfv( GL_LIGHT1, GL_SPECULAR, specular );
      glEnable( GL_LIGHT1 );
      setLighting ( &state, TRUE, TRUE );
    }
    
    /* now render the VRML scene graph */

    glLoadName( -2 );            /* -2 used to identify VRML models */
    glPushName( feature->id );   /* followed by a feature ID */

    state.compile = FALSE;

#if DISPLAY_LISTS
    renderRenderObjs (&state);
#else
    renderMFNode( nodes, &state );
#endif /* DISPLAY_LISTS */

    glPopName();                 /* pop the feature ID name */

  }; glPopAttrib ();
}

static int
colorsEqual (SFColor const *c1, SFColor const *c2)
{
  if (! FEQ_tol (c1->r, c2->r, 1e-2))
    return FALSE;
  if (! FEQ_tol (c1->g, c2->g, 1e-2))
    return FALSE;
  if (! FEQ_tol (c1->b, c2->b, 1e-2))
    return FALSE;

  return TRUE;
}

static int
materialsEqual (vsgcMaterial const *m1, vsgcMaterial const *m2)
{
  if (m1 == m2) return TRUE;
  if (m1 == NULL || m2 == NULL) return FALSE;

  if (! FEQ_tol (m1->ambientIntensity, m2->ambientIntensity, 1e-2))
    return FALSE;
  if (! FEQ_tol (m1->shininess, m2->shininess, 1e-2))
    return FALSE;
  if (! FEQ_tol (m1->transparency, m2->transparency, 1e-2))
    return FALSE;

  if (! colorsEqual (&m1->diffuseColor, &m2->diffuseColor))
    return FALSE;
  if (! colorsEqual (&m1->specularColor, &m2->specularColor))
    return FALSE;
  if (! colorsEqual (&m1->emissiveColor, &m2->emissiveColor))
    return FALSE;

  return TRUE;
}

static int
addRenderObj (m4f const *tfm, 
	      renderState const *exit, 
	      SFNode geom)
{
  globalState *global = exit->global;
  RenderObj *curr = global->robjs;
  vsgWorld *wrl = global->root_wrl;

  if (curr 
      && exit->in_anchor == curr->in_anchor
      && m4f_eq (tfm, &curr->tfm)
      && materialsEqual (exit->bound_material, curr->material)
      && (exit->pending.texturing && exit->bound_texture == curr->texture)) {
    vsgAppendSFNode (wrl, curr->geometry, geom);
#if TRACE_ROBJS
    printf ("  append geom %p\n", geom);
#endif
  } else {
    RenderObj *obj;

    if (curr) {
      vsgEndMFNode (wrl, curr->geometry);
#if TRACE_ROBJS
      printf ("} [%d]\n", curr->geometry->mf.len);
#endif
    }

    obj = vsgCalloc (wrl, sizeof (*obj));
    if (! obj) return FALSE;

    obj->next = curr;

    obj->tfm = *tfm;
    obj->in_anchor = exit->in_anchor;
    obj->material = exit->bound_material;
    obj->material_color = exit->bound_color;
    obj->texture = (exit->pending.texturing ? exit->bound_texture : 0);
    obj->saved = exit->current;

    obj->geometry = vsgStartMFNode (wrl);
    vsgAppendSFNode (wrl, obj->geometry, geom);

    {
      v4f P;
      decompose_m4f (tfm, &P, (v3f*)&obj->T, (v4f*)&obj->R, (v3f*)&obj->S);
      obj->flags.tfm |= obj->flags.translate = ! vec_zerop (obj->T.v);
      obj->flags.tfm |= obj->flags.rotate = ! rot_zerop (&obj->R);
      obj->flags.tfm |= obj->flags.scale = ! vec_onep (obj->S.v);
    }

#if TRACE_ROBJS
    printf ("addRenderObj[%d]: anchor %d material %p texture %p {\n",
	    global->nobjs, obj->in_anchor, obj->material, obj->texture);
    if (obj->flags.translate)
      printf ("  T[%g,%g,%g]", obj->T.v[0], obj->T.v[1], obj->T.v[2]);
    if (obj->flags.rotate)
      printf ("  R[%g,%g,%g,%g]", obj->R.a, obj->R.b, obj->R.c, obj->R.d);
    if (obj->flags.scale)
      printf ("  S[%g,%g,%g]", obj->S.v[0], obj->S.v[1], obj->S.v[2]);
    if (obj->flags.tfm)
      printf ("\n");
    printf ("  append geom %p\n", geom);
#endif

    global->robjs = obj;
    ++global->nobjs;
  }

  return TRUE;
}

static void
finishRenderObjs (renderState *state)
{
  globalState *global = state->global;
  RenderObj *curr = global->robjs;
  vsgWorld *wrl = global->root_wrl;
  int i;

  if (curr) {
    vsgEndMFNode (wrl, curr->geometry);
#if TRACE_ROBJS
    printf ("} [%d]\n", curr->geometry->mf.len);
#endif
  }

  /* Create the array to be sorted. */
  global->vobj = vsgAlloc (wrl, global->nobjs * sizeof (curr));
  for (i = global->nobjs-1; i >= 0; --i, curr = curr->next) {
    global->vobj[i] = curr;
  }
}

static void
renderRenderObjs (renderState *state)
{
  globalState *global = state->global;
  tvBase     *base = global->threeD;
  RenderObj **vobj = global->vobj;
  SFBool in_item = ( ! global->threeD->selecting
		     && ( global->item->id == base->f_pick_id ));
  int bound_link = 0;
  int bound_id = 0;
  int i;
  
  state->compile = FALSE;

#if TRACE_ROBJS
  printf ("renderRenderObjs[%d] {\n", global->nobjs);
#endif
  for (i = 0; i < global->nobjs; ++i) {
    RenderObj *obj = vobj[i];
    int in_link = 0;
    int id = obj->in_anchor;

    /* Ensure that we have the correct anchor id bound */
    if (id != bound_id) {
      if (bound_id == 0)
	glPushName (id);
      else if (id == 0)
	glPopName ();
      else
	glLoadName (id);
      bound_id = id;
    }

    state->in_anchor = id;
    state->in_hyperlink = in_link = (in_item && (id == base->f_pick_anchor));

#if TRACE_ROBJS
  printf (" obj[%d] anchor %d%s \n", i, id, (in_link ? " (in_link)" : ""));
#endif
    /* Render the material */
    if (obj->material != state->bound_material
	|| in_link != bound_link
	|| (state->texturescene && obj->texture != state->bound_texture)) {
      state->material = obj->material;
      state->texture = obj->texture;
      state->current.repeatS = obj->saved.repeatS;
      state->current.repeatT = obj->saved.repeatT;
#if TRACE_ROBJS
      printf ("  material %p color %d", obj->material, obj->material_color);
      if (obj->texture)
	printf (" texture %p {%d \"%s\"}",
		obj->texture, obj->texture->tex_id, obj->texture->url);
      printf ("\n");
#endif
      renderMaterial (state, obj->material_color);
      bound_link = in_link;
    }

#if DISPLAY_LISTS
    /* And then the geometry */
    if (obj->geometryDL) {
      /* Make sure that flags are what rendering expects. */
      state->pending = obj->enter;
      syncRender (state);
#if TRACE_ROBJS
      printf ("  geometry %p <= dl %d\n", obj->geometry, obj->geometryDL);
#endif
      glCallList (obj->geometryDL);
      state->current = obj->exit;
    } else {
      syncRender (state);
      obj->enter = state->current;
      obj->geometryDL = glGenLists (1);
#if TRACE_ROBJS
      printf ("  geometry %p >= dl %d\n", obj->geometry, obj->geometryDL);
#endif
      glNewList (obj->geometryDL, GL_COMPILE_AND_EXECUTE); {
#endif /* DISPLAY_LISTS */
	if (obj->flags.tfm) {
	  glPushMatrix ();
	  if (obj->flags.translate)
	    glTranslatef (obj->T.v[0], obj->T.v[1], obj->T.v[2]);
	  if (obj->flags.rotate)
	    glRotatef (obj->R.d*(180/M_PI), obj->R.a, obj->R.b, obj->R.c);
	  if (obj->flags.scale)
	    glScalef (obj->S.v[0], obj->S.v[1], obj->S.v[2]);
	}
	renderMFNode (obj->geometry, state);
	if (obj->flags.tfm)
	  glPopMatrix ();

#if DISPLAY_LISTS
      }; glEndList ();
      obj->exit = state->current;
    }
#endif /* DISPLAY_LISTS */

  }
#if TRACE_ROBJS
  printf ("}\n");
#endif

  /* Make sure that we exit anchor id's gracefully */
  if (bound_id != 0) 
    glPopName ();
}


static inline void
glSFRotate (SFRotation const *rot, float mult, renderState *s)
{
  if (rot->d != 0) {
    GLfloat r = mult*(180/M_PI*rot->d);
    if (! s->compile)
      glRotatef (r, rot->a, rot->b, rot->c);
    else
      glRotatef_m4f (r, rot->a, rot->b, rot->c, &s->tfm);
  }
}

static inline void
glSFScale (SFVec3f const *v, renderState *s)
{
  GLfloat s0 = v->v[0], s1 = v->v[1], s2 = v->v[2];
  if (! s->compile)
    glScalef (s0, s1, s2);
  else
    glScalef_m4f (s0, s1, s2, &s->tfm);
}

static inline void
glSFTranslate (SFVec3f const *v, float m, renderState *s)
{
  GLfloat t0 = m*v->v[0], t1 = m*v->v[1], t2 = m*v->v[2];
  if (! s->compile)
    glTranslatef (t0, t1, t2);
  else
    glTranslatef_m4f (t0, t1, t2, &s->tfm);
}


typedef void (*renderType) (vsgcNode *node, void *state);

/* GeoVRML 1.0 nodes */

static void
renderGeoLocation( SFNode node, renderState *state )
{
  globalState     *global = state->global;
  GeoVRML         gv = gvNew( state->curr_wrl );
  MFNode          *children;
  MFString        *geoSystem = NULL;
  SFString        geoCoords = NULL;
  vsgEventOutDesc evout;
  double          gcc[3];
  symbol_t        name;
  float           orient[4];

  if ( gv == NULL ) return;

  /* pass in the tvBase structure for DEM-relative elevations */

  gvSetViewer( gv, global->threeD );

  /* support the GeoOrigin setting to define a local coordinate system */

  gvSetOrigin3fv( gv, global->item->gcc_origin );

  /* work out the transformation that we need to perform */

  if ( ( name = vsgIntern( state->curr_wrl, "geoCoords" ) ) != NULL ) {
    evout = vsgGetFunc( node, name );
    geoCoords = vsgGetSFString( state->curr_wrl, node, evout );
  }

  if ( ( name = vsgIntern( state->curr_wrl, "geoSystem" ) ) != NULL ) {
    evout = vsgGetFunc( node, name );
    geoSystem = vsgGetMFString( state->curr_wrl, node, evout );
  }

  /* bomb out if we don't have a valid geographic location */

  if ( geoCoords == NULL || geoSystem == NULL ) {
    gvFree( gv );
    return;
  }

  /* parse out the (origin offset) geocentric coordinate */
    
  gvParserSFInit( gv, geoCoords, geoSystem );
  gvParserNextGCC( gv, gcc );

  /* translate to the specified geographic location and render children */

  glPushMatrix(); {
    SFVec3f t;
    SFRotation r;

    t.v[0] = gcc[0]; t.v[1] = gcc[1]; t.v[2] = gcc[2];
    glSFTranslate( &t, 1.0, state );
    
    /* rotate to the LVCS orientation, and then rotate into VRML's Y-up */
    
    gvLocalOrientation( gv, gcc, orient );
    r.d = (M_PI/180)*orient[0];
    r.a = orient[1], r.b = orient[2], r.c = orient[3];
    glSFRotate ( &r, 1.0, state );
    
    /* we are done with the GeoVRML structure now */

    gvFree( gv );

    /* get the children nodes and render them in the new position */

    if ( ( name = vsgIntern( state->curr_wrl, "children" ) ) != NULL ) {
      evout = vsgGetFunc( node, name );
      if ( ( children = vsgGetMFNode( state->curr_wrl, node, evout ) ) )
        renderMFNode( children, state );
    }

  }; glPopMatrix();

}

static vsgcCoordinate *
convertGeoCoordinate( SFNode node, renderState *state )
{
  globalState     *global = state->global;
  GeoVRML         gv = gvNew( state->curr_wrl );
  MFString        *geoSystem = NULL;
  MFString        *point = NULL;
  vsgEventOutDesc evout;
  double          gcc[3];
  symbol_t        name;
  int             watershed = 0, increment = 50;
  vsgcCoordinate  *coords = calloc( sizeof(vsgcCoordinate), 1 );
  MFVec3f         *vec = calloc( sizeof(MFVec3f), 1 );

  if ( gv == NULL || coords == NULL || vec == NULL ) return NULL;

  /* pass in the tvBase structure for DEM-relative elevations */

  gvSetViewer( gv, global->threeD );

  /* support the GeoOrigin setting to define a local coordinate system */

  gvSetOrigin3fv( gv, global->item->gcc_origin );

  /* extract the list of coordinates as a string array */

  if ( ( name = vsgIntern( state->curr_wrl, "point" ) ) != NULL ) {
    evout = vsgGetFunc( node, name );
    point = vsgGetMFString( state->curr_wrl, node, evout );
  }

  if ( ( name = vsgIntern( state->curr_wrl, "geoSystem" ) ) != NULL ) {
    evout = vsgGetFunc( node, name );
    geoSystem = vsgGetMFString( state->curr_wrl, node, evout );
  }

  /* bomb out if we don't have a valid geographic location */

  if ( point == NULL || geoSystem == NULL ) {
    gvFree( gv );
    free( vec );
    free( coords );
    return NULL;
  }

  /* initialise the vector array, allocate in chunks of 'increment' */

  coords->point = vec;
  watershed = increment;
  vec->data = malloc( sizeof( SFVec3f ) * watershed );
  vec->mf.len  = 0;

  /* parse out the (origin offset) geocentric coordinate */
    
  gvParserMFInit( gv, point, geoSystem );
  while ( gvParserNextGCC( gv, gcc ) ) {

    if ( vec->mf.len == watershed ) {
      watershed += increment;
      vec->data = realloc( vec->data, sizeof( SFVec3f) * watershed );
      if ( vec->data == NULL ) {
	gvFree( gv );
	free( vec );
	free( coords );
	return NULL;
      }
    }

    vec->data[vec->mf.len].v[0] = (float) gcc[0];
    vec->data[vec->mf.len].v[1] = (float) gcc[1];
    vec->data[vec->mf.len].v[2] = (float) gcc[2];
    vec->mf.len++;
  }

  return coords;
}

/* Grouping nodes. */

static void
renderMFNode (MFNode *mf, renderState *state)
{
  int i;
  for (i = 0; i < mf->mf.len; ++i) {
    renderNode (mf->data[i], state);
  }
}

static void
renderGroup (vsgcNode *node, renderState *state)
{
  vsgcGroup *group = (vsgcGroup *) node;
  renderMFNode (group->children, state);
}

static void
renderCollision (vsgcNode *node, renderState *state)
{
  vsgcCollision *group = (vsgcCollision *) node;
  renderMFNode (group->children, state);
}

static void
renderAnchor (vsgcNode *node, renderState *state)
{
  globalState *global = state->global;
  vsgcAnchor *group = (vsgcAnchor *) node;
  tvBase     *base = global->threeD;
  int        id;
  int        in_anchor = state->in_anchor;

  if (! node->aux) {
    id = ++state->anchor_count;
    node->aux = (void *) id;
  } else {
    id = (int) node->aux;
  }

  glPushName( id );

  if ( ! global->threeD->selecting )
    state->in_hyperlink = ( global->item->id == base->f_pick_id ) &&
                          ( id == base->f_pick_anchor );

  state->in_anchor = id;
  renderMFNode (group->children, state);

  glPopName();

  if ( ! global->threeD->selecting )
    state->in_hyperlink = FALSE;

  state->in_anchor = in_anchor;
}

static void
renderTransform (vsgcNode *node, renderState *state)
{
  vsgcTransform *tfm = (vsgcTransform *) node;
  m4f save_tfm = state->tfm;
  m4f save_inv_tfm = state->inv_tfm;
  SFBool trans, center, scale, rotate, sclOri, push;
  
  union {
    void *p;
    struct {
      SFBool push:1;
      SFBool trans:1;
      SFBool center:1;
      SFBool scale:1;
      SFBool rotate:1;
      SFBool sclOri:1;
    } flags;
  } tfmState;

  if (node->aux) {
    tfmState.p = node->aux;
    trans  = tfmState.flags.trans;
    center = tfmState.flags.center;
    scale  = tfmState.flags.scale;
    rotate = tfmState.flags.rotate;
    sclOri = tfmState.flags.sclOri;
    push   = tfmState.flags.push;
  } else {
    tfmState.flags.trans  = trans  = ! vec_zerop (tfm->translation.v);
    tfmState.flags.center = center = ! vec_zerop (tfm->center.v);
    tfmState.flags.scale  = scale  = ! vec_onep (tfm->scale.v);
    tfmState.flags.rotate = rotate = ! rot_zerop (&tfm->rotation);
    tfmState.flags.sclOri = sclOri = ! rot_zerop (&tfm->scaleOrientation);
    tfmState.flags.push   = push = (trans || center || scale || rotate || sclOri);
    node->aux = tfmState.p;
  }

  if (push) {
    glPushMatrix();

    if (trans) {
      glSFTranslate (&tfm->translation, 1.0, state );
    }

    if (center) {
      glSFTranslate (&tfm->center, 1.0, state);
    }

    if (rotate)
      glSFRotate (&tfm->rotation, 1.0, state);

    if (sclOri)
      glSFRotate (&tfm->scaleOrientation, 1.0, state);

    if (scale) {
      glSFScale (&tfm->scale, state);
      /*glEnable( GL_NORMALIZE );*/
    }

    if (sclOri) 
      glSFRotate( &tfm->scaleOrientation, -1.0, state) ;

    if (center)
      glSFTranslate (&tfm->center, -1.0,  state);

#if 0
    /* Track transform matrices. */
    glGetFloatv( GL_MODELVIEW_MATRIX, (float*) M(state->tfm) );
    inverse_m4f( &state->tfm, &state->inv_tfm );
#endif
  }

  renderMFNode( tfm->children, state );

  if (push) {
    glPopMatrix();
  }

  state->tfm = save_tfm;
  state->inv_tfm = save_inv_tfm;
}

/* Geometric primitives */
static void
renderShape (vsgcNode *node, renderState *state)
{
  vsgcShape *shape = (vsgcShape *) node;

  state->material = NULL;
  state->texture = NULL;

  renderNode (shape->appearance, state);
  if ( ( state->material != NULL && state->material->transparency < 1.0 )
       || state->material == NULL ) {
    if (state->compile) {
      m4f tfm = state->tfm;
      renderNode (shape->geometry, state);
      addRenderObj (&tfm, state, shape->geometry);
    } else {
      renderNode (shape->geometry, state);
    }
  }
}

static void
renderAppearance (vsgcNode *node, renderState *state)
{
  vsgcAppearance *appearance = (vsgcAppearance *) node;
  renderNode (appearance->material, state);
  renderNode (appearance->texture, state);
}

static void
rememberMaterial (vsgcNode *node, renderState *state)
{
  /* just remember the material node for the time being */
  /* the geometry node will render it with renderMaterial */
  state->material = (vsgcMaterial *) node;
}

static void
rememberTexture (vsgcNode *node, renderState *state)
{
  int              i;
  MFString         *url;
  vsgcImageTexture *texnode = (vsgcImageTexture *) node;

  /* just remember the FeatureTexture node for the time being */
  /* the geometry node will render it with renderMaterial */

  if ( node == NULL ) return;

  /* remember the repeatS and repeatT settings */

  state->current.repeatS = texnode->repeatS;
  state->current.repeatT = texnode->repeatT;

  /* find the first valid texture map from the URL MFString */

  url = texnode->url;

  for ( i = 0; i < url->mf.len; i++ )
    if ( ( state->texture = FeatureTexUse( state->curr_wrl, url->data[i] ) ) )
      break;
}

typedef struct {
  GLuint dl[3][2];
  renderFlags enter[3][2];
  renderFlags exit[3][2];
} materialAux;

static void
bindMaterial (renderState *state, renderColor which_color)
{
  /*static const char funcName[] = "bindMaterial";*/

  vsgcMaterial *material;
  GLfloat      alpha, c[4];
  SFColor      *diffuse, *emissive, *specular, highlight;
  SFFloat      ambient, shininess;
  SFBool       post_lighting = TRUE;
  SFBool       is_bound;
  GLuint       dl = 0;
  renderFlags  *exitp;

  static vsgcMaterial def_material = { 
    {{ NULL, NULL }, 0},  /* super */
    0.2f,                 /* ambientIntensity */
    { 0.8f, 0.8f, 0.8f }, /* diffuseColor */
    { 0.0f, 0.0f, 0.0f }, /* emissiveColor */
    0.2f,                 /* shininess */
    { 0.0f, 0.0f, 0.0f }, /* specularColor */
    0.0f                  /* transparency */
  };

  if ( state == NULL ) return;

  /* If there is no material specified, then the VRML97 */
  /* spec says that the object should be unlit. We also */
  /* need to set up a default material in this case.    */

  if ( state->material == NULL ) {
    state->material = &def_material;
    post_lighting = FALSE;
  }

  material = state->material;

  is_bound = materialsEqual (material, state->bound_material);
  if (! is_bound || state->bound_color != which_color) {

    /* extract all of the relevant values from the Material node */
    if (! state->compile) {

      if (1) {
	materialAux  *aux = material->super.aux;
	int in_link = state->in_hyperlink;
	renderFlags *enterp;

	/* Check for and use stored display lists */
	if (! aux) {
	  material->super.aux = aux =
	    vsgCalloc (state->global->root_wrl, sizeof(*aux));
	}

	dl = aux->dl[which_color][in_link];
	enterp = &aux->enter[which_color][in_link];
	exitp = &aux->exit[which_color][in_link];

	if (! dl) {
#if DISPLAY_LISTS
	  dl = glGenLists (1);
#endif /* DISPLAY_LISTS */
	  aux->dl[which_color][in_link] = dl;
	  syncRender (state);
	  *enterp = state->current;
#if DISPLAY_LISTS
	  glNewList (dl, GL_COMPILE_AND_EXECUTE);
#endif /* DISPLAY_LISTS */
	} else {
	  state->pending = *enterp;
	  syncRender (state);
#if DISPLAY_LISTS
	  glCallList (dl);
#endif /* DISPLAY_LISTS */
	  state->bound_material = material;
	  state->bound_color = which_color;
	  state->current = *exitp;
	  state->pending = *exitp;
	  return;
	}
      }

      alpha     = 1.0f - material->transparency;
      ambient   = material->ambientIntensity;
      shininess = material->shininess;
      diffuse   = &material->diffuseColor;
      emissive  = &material->emissiveColor;
      specular  = &material->specularColor;

      /* are we highlighting this geometry because its a hyperlink? */

      if ( state->in_hyperlink ) {
	float offset;

	/* make a temporary copy of the diffuse colour (we will change it) */

	highlight = *diffuse;
	diffuse = &highlight;

	/* if colour is bright, then make it darker, else make it lighter */
    
	if ( diffuse->r + diffuse->g + diffuse->b > 2.0 )
	  offset = -0.4f;
	else
	  offset = +0.4f;

	/* apply the colour change */

	diffuse->r = max( min( diffuse->r + offset, 1.0f ), 0.0f );
	diffuse->g = max( min( diffuse->g + offset, 1.0f ), 0.0f );
	diffuse->b = max( min( diffuse->b + offset, 1.0f ), 0.0f );
      }

      /* Color changes affect diffuse color. */

      setLighting( state, FALSE, TRUE );

      glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
      setColorMaterial (state, TRUE, FALSE);

      /* do we want to enable/disable transparency for this shape? */

      /* setTransparency(state, ( alpha < 1.0f ), FALSE); */
      /* setAlphaTrans(state, ( alpha < 1.0f ), FALSE); */

      /* Set the diffuse colour */

      c[0] = diffuse->r; c[1] = diffuse->g; c[2] = diffuse->b; c[3] = alpha;
      glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, c);
      if ( which_color == USE_DIFFUSE_COLOR ) glColor4fv( c );

      /* set the ambient colour */

      c[0] *= ambient; c[1] *= ambient; c[2] *= ambient;
      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, c);

      /* set the emissive colour */

      c[0] = emissive->r; c[1] = emissive->g; c[2] = emissive->b;
      glMaterialfv (GL_FRONT_AND_BACK, GL_EMISSION, c);
      if ( which_color == USE_EMISSIVE_COLOR ) glColor3fv( c );

      /* set the shininess factor */

      glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
      if (shininess > 0.0) {
	c[0] = specular->r; c[1] = specular->g; c[2] = specular->b; c[3] = 1.0f;
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, c);
      }

      if ( post_lighting )
	setLighting( state, TRUE, FALSE );
    }

    state->bound_material = material;
    state->bound_color = which_color;

    if (! state->compile && dl) {
      syncRender (state);
      glEndList ();
      *exitp = state->current;
    }
  } else if (is_bound) {
    /* Speed up subsequent equality checks. */
    state->material = state->bound_material;
  }
}

static void
bindTexture (renderState *state)
{
  static const char funcName[] = "bindTexture";

  if ( state == NULL ) return;

  /* set the texture map */

  if ( state->texture && state->texturescene ) {

    if ( state->texture != state->bound_texture ) {

      /* bind the texture map for this object */

      if ( state->texture->tex_id ) {

	/* we have already made this texture object, so just use it */
#if TRACE_ROBJS
	printf ("  bind texture %p[%d]\n", state->texture, state->texture->tex_id);
#endif
	gltBindTexture( GL_TEXTURE_2D, state->texture->tex_id );
      } else {

	/* get a new texture object id, if this is a new texture */

	gltGenTextures( 1, &state->texture->tex_id );

	/* do all of the texture setup stuff */

	gltBeginTexture( GL_TEXTURE_2D, state->texture->tex_id ); {
	  Image_t *image = state->texture->image;
	  GLenum  format;
	
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    
	  /* setup the current texture mapping characteristics */

	  glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
			   ( state->current.repeatS ) ? GL_REPEAT : GL_CLAMP );
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
			   ( state->current.repeatT ) ? GL_REPEAT : GL_CLAMP );

	  /* work out the image format from the number of components */
	
	  switch ( image->components ) {
	  case 1:
	    format = GL_LUMINANCE;
      setTransparency(state,FALSE,FALSE);
	    break;
	  case 2:
	    format = GL_LUMINANCE_ALPHA;
      setTransparency(state,TRUE,FALSE);
	    break;
	  case 3:
	    format = GL_RGB;
      setTransparency(state,FALSE,FALSE);
	    break;
	  case 4:
	    format = GL_RGBA;
      setTransparency(state,TRUE,FALSE);
	    break;
	  default:
	    tsmError( funcName, "Unsupport components: %d", image->components );
	  }

	  /* load the texture into texture cache */

	  glTexImage2D( GL_TEXTURE_2D, 0, image->components, image->width,
			image->height, 0, format, GL_UNSIGNED_BYTE,
			image->bitmap);

#if TRACE_ROBJS
	  printf ("  define texture %p[%d] %dx%d\n",
		  state->texture, state->texture->tex_id,
		  image->width, image->height);
#endif
	}; gltEndTexture ();
      }
    }

    /* turn on texture mapping */
    setTexturing (state, TRUE, FALSE);

    state->bound_texture = state->texture;

  } else {
    setTexturing (state, FALSE, FALSE);
  }
}

static void
renderMaterial (renderState *state, renderColor which_color)
{
  /*static const char funcName[] = "renderMaterial";*/

  bindMaterial (state, which_color);
  bindTexture (state);
}

static void
calcIFSNormal( SFVec3f *point, SFInt32 *index, int curr, 
	       SFBool anticlockwise, renderState *state, GLfloat normal[3] )
{
  int     pivot, prev, next;
  double  vec1[3], vec2[3], cross[3], norm[3];

  if ( ! state->polygon ) return;

  /* make sure we are not the first index - we need a prev and next */

  if ( curr == 0 || index[curr-1] == -1 ) curr++;
  pivot = index[curr];
  prev  = index[curr - 1];
  next  = index[curr + 1];

  /* if polygon has less than 3 vertices, we can't do much! */

  if ( pivot == -1 || prev == -1 || next == -1 ) return;

  /* calculate p(n+1)-p(n), and p(n-1)-p(n) */
  
  vec1[0] = point[next].v[0] - point[pivot].v[0];
  vec1[1] = point[next].v[1] - point[pivot].v[1];
  vec1[2] = point[next].v[2] - point[pivot].v[2];

  vec2[0] = point[prev].v[0] - point[pivot].v[0];
  vec2[1] = point[prev].v[1] - point[pivot].v[1];
  vec2[2] = point[prev].v[2] - point[pivot].v[2];

  /* calculate the cross product (vertex order dependent) */
  /* and normalise in order to get the normal for this face */

  if ( anticlockwise )
    tsmCrossProduct( vec2, vec1, cross );
  else
    tsmCrossProduct( vec1, vec2, cross );

  tsmNormalizeVector( cross, norm, 3 );

  /* map the double-sized normal into the float-sized result vector */

  normal[0] = norm[0];
  normal[1] = norm[1];
  normal[2] = norm[2];
}

/*
 * Synopsis:
 *   void calcBBox( SFVec3f *vertex, SFInt32 *index, int num_indices,
 *                  int start, double min[3], double max[3] )
 *
 * Description:
 *   Calculates the bounding box of a single polygon in an IndexedFaceSet
 *   style of vertex+index list. You specify the list of SFVec3f vertices,
 *   the list of indices into this vertex list for the entire Shape node,
 *   and the index at which the desired polygon starts at. 
 *
 *   Each polygon definition ends with an index of -1. You also supply
 *   the number of indices that exist in the index array so that we can
 *   do some extra error bounds checking.
 *
 *   On success, this function will have looked at every vertex in the
 *   specified polygon and set the min vector to contain the smallest
 *   X, Y, and Z coordinates and the max vector to contain the largest
 *   such coordinates.
 *
 *   In the result of a failure, the min and max vectors are both set
 *   to (0,0,0).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 April, 1999
 *
 */

static void
calcBBox( SFVec3f *vertex, SFInt32 *index, int num_indices, int start,
	  double min[3], double max[3] )
{
  int idx;

  /* setup default bounding box to return in time of error */

  min[0] = min[1] = min[2] = 0.0;
  max[0] = max[1] = max[2] = 0.0;

  if ( vertex == NULL || index == NULL || start >= num_indices )
    return;

  /* set min and max to the value of the first vertex in the poly */

  if ( ( idx = index[start] ) == -1 )
    return;

  min[0] = max[0] = vertex[idx].v[0];
  min[1] = max[1] = vertex[idx].v[1];
  min[2] = max[2] = vertex[idx].v[2];

  /* now loop through the remaining vertices in this poly */

  while ( index[++start] != -1 && start < num_indices ) {
    idx = index[start];

    if ( vertex[idx].v[0] < min[0] ) min[0] = vertex[idx].v[0];
    if ( vertex[idx].v[1] < min[1] ) min[1] = vertex[idx].v[1];
    if ( vertex[idx].v[2] < min[2] ) min[2] = vertex[idx].v[2];

    if ( vertex[idx].v[0] > max[0] ) max[0] = vertex[idx].v[0];
    if ( vertex[idx].v[1] > max[1] ) max[1] = vertex[idx].v[1];
    if ( vertex[idx].v[2] > max[2] ) max[2] = vertex[idx].v[2];
  }
}

/*
 * Synopsis:
 *   void calcST( double min[3], double max[3], int *s, int *t )
 *
 * Description:
 *   This function will take the bounding box for an object in its
 *   local coordinate system and work out which of the cardinal axes
 *   should be used for the s and t texture coordinate axes.
 *
 *   The VRML97 spec says the following on this matter: "a default
 *   texture mapping is calculated using the local coordinate system
 *   bounding box of the shape". The longest dimension of the bounding
 *   box defines the S coordinates, and the next longest defines the T
 *   coordinates. If two or all three dimensions of the bound box are
 *   equal, ties shall be broken by choosing the X, Y, or Z dimension
 *   in that order of preference."
 *
 *   You can use calcBBox() to calculate the bounding box of a polygon,
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 April, 1999
 *
 */

static void
calcST( double min[3], double max[3], int *s, int *t )
{
  double d[3];
  int    first, second, i;

  /* work out the distance between the min and max coords */

  d[0] = max[0] - min[0];
  d[1] = max[1] - min[1];
  d[2] = max[2] - min[2];

  /* find the longest edge of the bounding box */

  for ( first = 0, i = 1; i < 3; i++ )
    if ( d[i] > d[first] )
      first = i;

  /* otherwise, find the second longest edge of the bounding box */

  second = ( first + 1 ) % 3;
  for ( i = 0; i < 3; i++ )
    if ( d[i] > d[second] && i != first )
      second = i;

  /* if either length is zero, then we have an invalid bounding box */

  if ( d[first] == 0.0 || d[second] == 0.0 ) {
    *s = *t = -1;
    return;
  }

  /* if d[first] ~= d[start], then the VRML97 spec says that we */
  /* should prefer the X coord, then Y, then Z in that order.   */
  /* here we make sure that S is given priority in this order.  */

  if ( fabs( d[first] - d[second] ) < ( d[first] / 10000.0 ) &&
       first > second ) {
    int temp = first;
    first  = second;
    second = temp;
  }

  /* according to the VRML97 spec says that the longest dimension of  */
  /* the bounding box defines the S coordinates, and the next longest */
  /* defines the T coordinates. So let's do just that...              */

  *s = first;
  *t = second;
}

/*
 * Synopsis:
 *   void autoGenTexCoords( double min[3], double max[3], int s, int t,
 *                          SFVec3f *point, GLfloat *u, GLfloat *v )
 *
 * Description:
 *   Automatically calculates the texture coordinates for the specified
 *   vertex. This is done to the VRML97 specification, section 6.23. 
 *   For this we need to know the bonding box of the entire polygon that
 *   this point resides in (min[3] and max[3]), and we also need to know
 *   which of the dimensions of this bounding box is to be used for the
 *   S texture coordinate, and which for the T texture coordinate. The
 *   s and t parameters should be integers in the range 0..2, where 0
 *   represents X, 1 represents Y, and 2 represents Z.  S ranges 0..1,
 *   T ranges 0 to the ratio of the second greatest dimension to the
 *   greatest dimension.
 *
 *   This function will then compute the correct (S,T) texture coordinates
 *   for this point and perform the appropriate glTexCoord OpenGL call
 *   to define these.
 *
 *   You can use calcBBox() to calculate the bounding box of a polygon,
 *   and calcST() to calculate the s and t lookup values.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 16 April, 1999
 *
 */

static void
autoGenTexCoords( double min[3], double max[3], int s, int t, SFVec3f *point,
		  GLfloat *u, GLfloat *v )
{
  GLfloat  S, T;
  double   s_size, t_size;

  /* some sanity checks on the values that we've been handed */

  if ( s < 0 || t < 0 || point == NULL ) return;

  /* calculate the S and T texture coordinates */

  s_size = max[s] - min[s];
  t_size = max[t] - min[t];

  S = ( point->v[s] - min[s] ) / s_size;
  T = ( point->v[t] - min[t] ) / t_size;

  /* according to the VRML97 spec, the T coord should be range    */
  /* from zero to the ratio of the bounding box dimension in the  */
  /* T direction to the bounding box dimension in the S direction */

  T *= t_size / s_size;

  /* store these texture coordinates in the VertexInfo structure */

  *u = S;
  *v = T;
}

static void
renderVertex( VertexInfo *vinfo )
{
  if ( vinfo == NULL ) return;
  if ( vinfo->do_color ) glColor3f( vinfo->r, vinfo->g, vinfo->b );
  if ( vinfo->do_texcoords ) glTexCoord2f( vinfo->u, vinfo->v );
  if ( vinfo->do_normals ) glNormal3f( vinfo->n[0], vinfo->n[1], vinfo->n[2] );
  if ( vinfo->vertex ) glVertex3fv( vinfo->vertex );
}

typedef struct IFSAux {
  void (*render) (vsgcIndexedFaceSet *ifs, renderState *state,
		  struct IFSAux *aux);

  vsgcCoordinate        *coord;
  vsgcColor             *color;
  vsgcTextureCoordinate *texcoord;
  vsgcNormal            *normal;

  /* the various data fields for the IFS */

  SFVec3f               *coords;
  SFInt32               *coordIndex;
  SFColor               *colors;
  SFInt32               *colorIndex;
  SFVec3f               *normals;
  SFInt32               *normalIndex;
  SFVec2f               *texCoords;
  SFInt32               *texCoordIndex;

  int                   coordIndexLen;
  int                   maxTexIndex;
  int                   maxNormalIndex;
  int                   maxColorIndex;

  /* the various rendering options of the IFS */

  SFBool                colorPerVertex;
  SFBool                normalPerVertex;

  MFInt32               *optimized;
  MFColor               *optColors;
  MFVec3f               *optNormals;
  MFVec2f               *optTexture;
} IFSAux;

static void compileIFS (vsgcIndexedFaceSet *ifs, renderState *state);
static MFInt32 *optimizeIFS (vsgcIndexedFaceSet *ifs, renderState *state);

static void
renderIFS (vsgcNode *node, renderState *state)
{
  /* references to the various nodes in the IndexedFaceSet */

  vsgcIndexedFaceSet    *ifs = (vsgcIndexedFaceSet *) node;

  /* do some initial sanity checks */

  if ( ifs == NULL || ifs->coord == NULL ) return;
  if ( ifs->coordIndex == NULL || ifs->coordIndex->data == NULL ) return;

  if (! node->aux)
    compileIFS (ifs, state);

  {
    IFSAux *ifsAux = node->aux;
    if ( ifsAux != NULL && ifsAux->coord != NULL ) {

      /* render the material that is bound with this geometry */

      if ( ifsAux->colors )
	renderMaterial( state, USE_NO_COLOR );
      else
	renderMaterial( state, USE_DIFFUSE_COLOR );

      if (state->compile) return;

      /* support the solid field - toggle backface culling appropriately */

      setBackfaceCull( state, ifs->solid, FALSE );

      /* support the ccw field to specify vertex ordering */

      setVertexOrdering( state, ifs->ccw, FALSE );

      /* support the per-vertex mode by enabling smooth shading */

      setShadeModel( state, ifs->colorPerVertex, FALSE );

      /* synchronize rendering state */

      syncRender (state);

      (*ifsAux->render) (ifs, state, ifsAux);
    }
  }
}

static void
renderIFS_generic (vsgcIndexedFaceSet *ifs, renderState *state,
		   IFSAux *ifsAux)
{
  vsgcColor             *color = ifsAux->color;

  /* the various data fields for the IFS */

  SFVec3f               *coords = ifsAux->coords;
  SFInt32               *coordIndex = ifsAux->coordIndex;
  SFColor               *colors = ifsAux->colors;
  SFInt32               *colorIndex = ifsAux->colorIndex;
  SFVec3f               *normals = ifsAux->normals;
  SFInt32               *normalIndex = ifsAux->normalIndex;
  SFVec2f               *texCoords = ifsAux->texCoords;
  SFInt32               *texCoordIndex = ifsAux->texCoordIndex;

  int                   coordIndexLen = ifsAux->coordIndexLen;
  int                   maxTexCoord = ifsAux->maxTexIndex;
  int                   maxNormalCoord = ifsAux->maxNormalIndex;
  int                   maxColorCoord = ifsAux->maxColorIndex;

  /* the various rendering options of the IFS */

  SFBool                colorPerVertex = ifsAux->colorPerVertex;
  SFBool                normalPerVertex = ifsAux->normalPerVertex;

  double                bbox_min[3], bbox_max[3];
  int                   s_index, t_index;

  int                   i, first = TRUE;
  int                   curr_poly = 0, curr_normal = 0;

  VertexInfo            vinfo;

  /* calculate the first bounding box - currently we just do this */
  /* when we are texture mapping because we need this information */
  /* in order to automatically generate texture coordinates.      */

  if ( state->pending.texturing && !texCoords ) {
    calcBBox( coords, coordIndex, coordIndexLen, 0, bbox_min, bbox_max );
    calcST( bbox_min, bbox_max, &s_index, &t_index ); 
  }

  /* generate the OpenGL constructs for the IndexedFaceSet */

  glBegin( polygon_mode[state->polygon] );

  for (i = 0; i < coordIndexLen; ++i) {
    int idx = coordIndex[i];

    /* initialise the VertexInfo structure for the vertex */

    memset( &vinfo, 0, sizeof(vinfo) );

    if (idx == -1) {

      /* end of current polygon definition: start a new polygon */

      glEnd (); 
      glBegin (polygon_mode[state->polygon]);

      /* get the bounding box info for this new polygon */

      if ( state->pending.texturing && !texCoords ) {
	calcBBox( coords, coordIndex, coordIndexLen, i+1, bbox_min, bbox_max );
	calcST( bbox_min, bbox_max, &s_index, &t_index );
      }

      first = TRUE;

    } else {

      /* in the midst of a polygon definition: first thing */
      /* to do is to set the per-face or per-vertex colour */

      /*----------- support colour fields of the IFS node ---------*/

      if ( color ) {
	if ( colorPerVertex == FALSE && first ) {

	  /* colorPerVertex is FALSE - use colorIndex if not empty */

	  /* otherwise apply the colours of Color node in order. */

	  SFColor *c = ( colorIndex ) ?
	    &colors[colorIndex[curr_poly++]] : &colors[curr_poly++];

	  vinfo.r = c->r;
	  vinfo.g = c->g;
	  vinfo.b = c->b;
	  vinfo.do_color = TRUE;
	  
	} else if ( colorPerVertex == TRUE ) {
	  
	  /* colorPerVertex is TRUE - use colorIndex if not empty */
	  /* otherwise we use the coordIndex to choose colors. */

	  SFColor *c = ( colorIndex ) ? &colors[colorIndex[i]] : &colors[idx];

	  vinfo.r = c->r;
	  vinfo.g = c->g;
	  vinfo.b = c->b;
	  vinfo.do_color = TRUE;
	}
      }

      /*-------- support texture coordinate fields of the IFS node -------*/

      if ( state->pending.texturing ) {
	if ( texCoords ) {

	  /* use the texCoordIndex field (if specified) to look-up into */
	  /* the texCoord array; otherwise we use the coordIndex field. */

	  SFVec2f *n = NULL;

    /* make sure that the index into the texCoord array is valud! */
    if ( texCoordIndex ) {
      int t_i = texCoordIndex[i];
      if ( t_i < maxTexCoord ) n = &texCoords[t_i];
    } else if ( idx < maxTexCoord ) {
      n = &texCoords[idx];
    }

    if ( n ) {
      vinfo.u = n->v[0];
	    vinfo.v = n->v[1];
	    vinfo.do_texcoords = TRUE;
    }
	  
	} else {

	  /* if the texCoord field is NULL, but a texture map image was  */
	  /* specified, then we need to generate texture coordinates for */
	  /* the user as per the VRML97 spec. This is done here.         */
	  
	  autoGenTexCoords( bbox_min, bbox_max, s_index, t_index,
			    &coords[idx], &vinfo.u, &vinfo.v );

	  vinfo.do_texcoords = TRUE;
	}
      }

      /*------------ support normal fields of the IFS node ----------*/

      if ( normals ) {
	if ( normalPerVertex == FALSE && first ) {

	  /* normalPerVertex is FALSE - use normalIndex if not empty */
	  /* otherwise apply normals in the order they are specified */

	  SFVec3f *n = ( normalIndex ) ?
	    &normals[normalIndex[curr_normal++]] : &normals[curr_normal++];

	  vinfo.n[0] = n->v[0];
	  vinfo.n[1] = n->v[1];
	  vinfo.n[2] = n->v[2];
	  vinfo.do_normals = TRUE;

	} else if ( normalPerVertex == TRUE ) {

	  /* normalPerVertex is TRUE - use normalIndex if not empty */
	  /* otherwise we use the coordIndex to choose normals. */

    SFVec3f *n = ( normalIndex ) ? &normals[normalIndex[i]] :
       &normals[idx];

    vinfo.n[0] = n->v[0];
	  vinfo.n[1] = n->v[1];
    vinfo.n[2] = n->v[2];
    vinfo.do_normals = TRUE;
	}

      } else if ( first ) { /* no normal information specified */

	/* generate per-face normals for the polygon on-the-fly */

	GLfloat norm[3];
	calcIFSNormal( coords, coordIndex, i, ifs->ccw, state, norm );

	vinfo.n[0] = norm[0];
	vinfo.n[1] = norm[1];
	vinfo.n[2] = norm[2];
	vinfo.do_normals = TRUE;

      }

      /*---------- support coordinate fields of the IFS node --------*/

      vinfo.vertex = coords[idx].v;

      /*-------- render the contents of the VertexInfo struct -------*/

      renderVertex( &vinfo );

      first = FALSE;
    }
  }

  glEnd ();
}

typedef union {
  SFInt32 i32;
  struct {
    char op:8;
    unsigned int idx:24;
  } op;
} IFSOptCommand;

static void
renderIFS_opt (vsgcIndexedFaceSet *ifs, renderState *state,
	       IFSAux *ifsAux)
{
  /* the various data fields for the IFS */

  SFVec3f               *coords = ifsAux->coords;

  /* the various rendering options of the IFS */

  MFInt32 const         *optimized = ifsAux->optimized;
  SFInt32 const         *optIndex = optimized->data;
  unsigned int const    optLen = optimized->mf.len;

  int                   i;

  IFSOptCommand         cmd;

  /* generate the OpenGL constructs for the IndexedFaceSet */

  cmd.i32 = optIndex[0];
  glBegin (cmd.op.idx);

  for (i = 1; i < optLen; ++i) {
    IFSOptCommand cmd;
    unsigned int idx;

    cmd.i32 = optIndex[i];
    idx = cmd.op.idx;

    switch (cmd.op.op) {
    case 'p':
      glEnd ();
      glBegin (idx);
      break;
    case 'c':
      glColor3fv (&ifsAux->optColors->data[idx].r);
      break;
    case 'n':
      glNormal3fv (ifsAux->optNormals->data[idx].v);
      break;
    case 't':
      glTexCoord2fv (ifsAux->optTexture->data[idx].v);
      break;
    case 'v':
      glVertex3fv (coords[idx].v);
      break;
    }
  }

  glEnd ();
}

static void
compileIFS (vsgcIndexedFaceSet *ifs, renderState *state)
{
  globalState *global = state->global;
  vsgcNode *node = &ifs->super;

  vsgcCoordinate        *coord = NULL;
  vsgcColor             *color = NULL;
  vsgcTextureCoordinate *texcoord = NULL;
  vsgcNormal            *normal = NULL;

  /* the various data fields for the IFS */

  SFVec3f               *coords = NULL;
  SFInt32               *coordIndex = NULL;
  SFColor               *colors = NULL;
  SFInt32               *colorIndex = NULL;
  SFVec3f               *normals = NULL;
  SFInt32               *normalIndex = NULL;
  SFVec2f               *texCoords = NULL;
  SFInt32               *texCoordIndex = NULL;

  int                   coordIndexLen;

  /* the various rendering options of the IFS */

  SFBool                colorPerVertex;
  SFBool                normalPerVertex;

  IFSAux *ifsAux;

  /* Allocate aux to hold these values. */
  ifsAux = node->aux = vsgAlloc (global->root_wrl, sizeof (IFSAux));
  ifsAux->coord = NULL;

  /* convert polygon to convex if it is concave */

  if ( ifs->convex == FALSE )
    makeConvexIFS( ifs, state );

  /* get the Coordinate node - could also be a GeoCoordinate node! */

  if ( isPROTONode( ifs->coord, state, "GeoCoordinate" ) )
    coord = convertGeoCoordinate( ifs->coord, state );
  else
    coord = (vsgcCoordinate *) vsgcEffectiveNode (ifs->coord);

  if ( coord == NULL || coord->point == NULL ) return;

  /* grab pointers to everything that has been defined in this node */

  if ( ifs->color )
    color = (vsgcColor *) vsgcEffectiveNode (ifs->color);
  if ( ifs->texCoord )
    texcoord = (vsgcTextureCoordinate *) vsgcEffectiveNode( ifs->texCoord );
  if ( ifs->normal )
    normal = (vsgcNormal *) vsgcEffectiveNode( ifs->normal );

  colorPerVertex   = ifs->colorPerVertex;
  coordIndex       = ifs->coordIndex->data;
  coordIndexLen    = ifs->coordIndex->mf.len;
  normalPerVertex  = ifs->normalPerVertex;

  colors           = ( color ) ? color->color->data : NULL;
  colorIndex       = ( ifs->colorIndex ) ? ifs->colorIndex->data : NULL;
  normals          = ( normal ) ? normal->vector->data : NULL;
  normalIndex      = ( ifs->normalIndex ) ? ifs->normalIndex->data : NULL;
  texCoords        = (texcoord && texcoord->point)?texcoord->point->data:NULL;
  texCoordIndex    = ( ifs->texCoordIndex ) ? ifs->texCoordIndex->data : NULL;
  coords           = coord->point->data;

  /* check that we have enough tex vertices/normals to match the vertex list */

  if ( ifs->texCoordIndex ) {
    int size = ifs->texCoordIndex->mf.len;
    if ( size > 0 && size < coordIndexLen )
      coordIndexLen = size;  /* too few tex coords! */
  }

  if ( ifs->normalIndex ) {
    int size = ifs->normalIndex->mf.len;
    if ( size > 0 && size < coordIndexLen )
      coordIndexLen = size;  /* too few normal coords! */
  }
  
  /* start copying the fields we need into the closure structure */

  ifsAux->coord = coord;
  ifsAux->color = color;
  ifsAux->texcoord = texcoord;
  ifsAux->normal = normal;

  /* the various data fields for the IFS */

  ifsAux->coords = coords;
  ifsAux->coordIndex = coordIndex;
  ifsAux->colors = colors;
  ifsAux->colorIndex = colorIndex;
  ifsAux->normals = normals;
  ifsAux->normalIndex = normalIndex;
  ifsAux->texCoords = texCoords;
  ifsAux->texCoordIndex = texCoordIndex;

  ifsAux->coordIndexLen = coordIndexLen;
  ifsAux->maxTexIndex = ( texcoord && texcoord->point ) ?
    texcoord->point->mf.len : 0;
  ifsAux->maxNormalIndex = ( normal && normal->vector ) ?
    normal->vector->mf.len : 0;
  ifsAux->maxColorIndex = ( color && color->color ) ?
    color->color->mf.len : 0;

  /* the various rendering options of the IFS */

  ifsAux->colorPerVertex = colorPerVertex;
  ifsAux->normalPerVertex = normalPerVertex;

  ifsAux->render = renderIFS_generic;

  ifsAux->optimized = optimizeIFS (ifs, state);
  if (ifsAux->optimized)
    ifsAux->render = renderIFS_opt;
}

static void
renderILS (vsgcNode *node, renderState *state)
{
  vsgcIndexedLineSet *ils = (vsgcIndexedLineSet *) node;

  vsgcCoordinate     *coord = NULL;
  vsgcColor          *color = NULL;
  SFVec3f            *coords = NULL;
  SFInt32            *coordIndex = NULL;
  SFColor            *colors = NULL;
  SFInt32            *colorIndex = NULL;
  int                colorPerVertex;
  int                coordIndexLen;
  int                i, curr_poly = 0, first = TRUE;

  /* do some initial sanity checks */

  if ( ils == NULL || ils->coord == NULL ) return;
  if ( ils->coordIndex == NULL || ils->coordIndex->data == NULL ) return;

  /* get the Coordinate node - could also be a GeoCoordinate node! */

  if ( isPROTONode( ils->coord, state, "GeoCoordinate" ) )
    coord = convertGeoCoordinate( ils->coord, state );
  else
    coord = (vsgcCoordinate *) vsgcEffectiveNode (ils->coord);

  if ( coord == NULL || coord->point == NULL ) return;

  /* grab pointers to everything that has been defined in this node */

  color          = (vsgcColor *) vsgcEffectiveNode( ils->color );
  colorPerVertex = ils->colorPerVertex;
  coordIndexLen  = ils->coordIndex->mf.len;
  coordIndex     = ils->coordIndex->data;
  coords         = coord->point->data;
  colors         = ( color && color->color ) ? color->color->data : NULL;
  colorIndex     = ( ils->colorIndex ) ? ils->colorIndex->data : NULL;

  /* render the material that is bound with this geometry */

  if ( colors )
    renderMaterial( state, USE_NO_COLOR );
  else
    renderMaterial( state, USE_EMISSIVE_COLOR );

  if (state->compile) return;

  /* support the per-vertex mode by enabling smooth shading */

  setShadeModel( state, colorPerVertex, FALSE );

  /* generate the OpenGL constructs for the IndexedLineSet */

  syncRender (state);

  glBegin (GL_LINE_STRIP);
  for (i = 0; i < coordIndexLen; ++i) {
    int idx = coordIndex[i];

    if (idx == -1) {

      /* end of current polyline definition: start a new polyline */

      glEnd (); 
      glBegin (GL_LINE_STRIP);
      first = TRUE;

    } else {

      /*----------- support colour fields of the ILS node ---------*/

      if ( color ) {
	if ( colorPerVertex == FALSE && first ) {

	  /* colorPerVertex is FALSE - use colorIndex if not empty */
	  /* otherwise apply the colours of Color node in order. */

	  SFColor *c = ( colorIndex ) ?
	    &colors[colorIndex[curr_poly++]] : &colors[curr_poly++];
	  glColor3f( c->r, c->g, c->b );
	  
	} else if ( colorPerVertex == TRUE ) {
	  
	  /* colorPerVertex is TRUE - use colorIndex if not empty */
	  /* otherwise we use the coordIndex to choose colors. */

	  SFColor *c = ( colorIndex ) ? &colors[colorIndex[i]] : &colors[idx];
	  glColor3f( c->r, c->g, c->b );
	}

      }

      /*---------- support coordinate fields of the ILS node --------*/

      glVertex3fv( coords[idx].v );

      first = FALSE;
    }
  }
  glEnd ();
}

static void
renderBox( vsgcNode *node, renderState *state )
{
  vsgcBox *box = (vsgcBox *) node;
  GLfloat x, y, z;
				   
  renderMaterial( state, USE_DIFFUSE_COLOR );

  if (state->compile) return;

  setBackfaceCull( state, TRUE, FALSE );
  setVertexOrdering( state, TRUE, FALSE );
  setShadeModel( state, FALSE, FALSE );

  syncRender (state);

  x = box->size.v[0] / 2.0f;
  y = box->size.v[1] / 2.0f;
  z = box->size.v[2] / 2.0f;

  if ( state->polygon ) {
    SFBool texturing = state->pending.texturing;

    glBegin( GL_QUADS ); {
      glNormal3f( 0.0f, -1.0f, 0.0f );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -x, -y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f(  x, -y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f(  x, -y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( -x, -y,  z );
      
      glNormal3f( 0.0f, 1.0f, 0.0f );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f( -x,  y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f(  x,  y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f(  x,  y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f( -x,  y, -z );
      
      glNormal3f( 0.0f, 0.0f, -1.0f );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -x,  y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f(  x,  y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f(  x, -y, -z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( -x, -y, -z );

      glNormal3f( 0.0f, 0.0f, 1.0f );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -x, -y,  z );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f(  x, -y,  z );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f(  x,  y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( -x,  y,  z );
      
      glNormal3f( 1.0f, 0.0f, 0.0f );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f(  x,  y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f(  x,  y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f(  x, -y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f(  x, -y, -z );
      
      glNormal3f( -1.0f, 0.0f, 0.0f );
      if ( texturing ) glTexCoord2f( 1.0f, 0.0f );
      glVertex3f( -x, -y, -z );
      if ( texturing ) glTexCoord2f( 1.0f, 1.0f );
      glVertex3f( -x, -y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 1.0f );
      glVertex3f( -x,  y,  z );
      if ( texturing ) glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -x,  y, -z );
    }; glEnd();
  } else {
    glBegin( GL_LINE_LOOP ); {
      glNormal3f( 0.0f, -1.0f, 0.0f );
      glVertex3f( -x, -y, -z );
      glVertex3f(  x, -y, -z );
      glVertex3f(  x, -y,  z );
      glVertex3f( -x, -y,  z );
    } ;glEnd();
      
    glBegin( GL_LINE_LOOP ); {
      glNormal3f( 0.0f, 1.0f, 0.0f );
      glVertex3f( -x,  y,  z );
      glVertex3f(  x,  y,  z );
      glVertex3f(  x,  y, -z );
      glVertex3f( -x,  y, -z );
    } ;glEnd();
      
    glBegin( GL_LINES ); {
      glNormal3f( 0.707107f, 0.0f, -0.707107f );
      glVertex3f(  x,  y, -z );
      glVertex3f(  x, -y, -z );

      glNormal3f( -0.707107f, 0.0f, 0.707107f );
      glVertex3f( -x,  y,  z );
      glVertex3f( -x, -y,  z );
      
      glNormal3f( 0.707107f, 0.0f, 0.707107f );
      glVertex3f(  x,  y,  z );
      glVertex3f(  x, -y,  z );
      
      glNormal3f( -0.707107f, 0.0f, -0.707107f );
      glVertex3f( -x,  y, -z );
      glVertex3f( -x, -y, -z );
    }; glEnd();
  }
  
}

static void
renderSphere( vsgcNode *node, renderState *state )
{
  vsgcSphere    *sphere = (vsgcSphere *) node;
  GLUquadricObj *qobj = gluNewQuadric();
  const int     slices = 10, stacks = 10;

  renderMaterial( state, USE_DIFFUSE_COLOR );
  
  if (state->compile) return;

  setBackfaceCull( state, TRUE, FALSE );
  setVertexOrdering( state, TRUE, FALSE );
  setShadeModel( state, TRUE, FALSE );

  syncRender (state);

  gluQuadricDrawStyle( qobj, state->polygon ? GLU_FILL : GLU_LINE );
  gluQuadricNormals( qobj, GLU_SMOOTH );
  if ( state->pending.texturing ) gluQuadricTexture( qobj, GL_TRUE );

  gluSphere( qobj, sphere->radius, slices, stacks );
  gluDeleteQuadric( qobj );
}

static void
renderText( vsgcNode *node, renderState *state )
{
  vsgcText *text = (vsgcText *) node;
  globalState *global = state->global;  
  tvBase   *threeD = global->threeD;

  if ( text == NULL || text->string == NULL || text->string->mf.len == 0 )
    return;

  renderMaterial( state, USE_DIFFUSE_COLOR );

  if (state->compile) return;

  syncRender (state);

  glRasterPos3f( 0.0f, 0.0f, 0.0f );
  glmCharstr( threeD->glmw, GL_FONT_R, text->string->data[0] );
}

static void
renderCylinder( vsgcNode *node, renderState *state )
{
  vsgcCylinder  *cyl = (vsgcCylinder *) node;
  GLUquadricObj *qobj = gluNewQuadric();
  const int     slices = 10, stacks = 10;

  renderMaterial( state, USE_DIFFUSE_COLOR );

  if (state->compile) return;

  setBackfaceCull( state, cyl->side && cyl->top && cyl->bottom, FALSE );
  setVertexOrdering( state, TRUE, FALSE );
  setShadeModel( state, FALSE, FALSE );

  syncRender (state);

  gluQuadricDrawStyle( qobj, state->polygon ? GLU_FILL : GLU_LINE );
  gluQuadricNormals( qobj, GLU_FLAT );
  if ( state->pending.texturing ) gluQuadricTexture( qobj, GL_TRUE );

  /* GLU cylinder is oriented along z-axis, from z=0 to height */
  /* VRML cylinder is oriented along y-axis from z=-height/2 to height/2 */
  /* N.B. the GLU cylinder does not appear to be capped! */

  glPushMatrix (); {

    glTranslatef( 0.0f, cyl->height / 2.0, 0.0f ); 
    glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
    
    if ( cyl->side )
      gluCylinder(qobj, cyl->radius, cyl->radius, cyl->height, slices, stacks);

    if ( cyl->top ) {
      setVertexOrdering( state, FALSE, TRUE );
      gluDisk( qobj, 0, cyl->radius, slices, 1 );
    }

    if ( cyl->bottom ) {
      glTranslatef( 0.0f, 0.0f, cyl->height );
      setVertexOrdering( state, FALSE, TRUE );
      gluQuadricOrientation( qobj, GLU_INSIDE );
      gluDisk( qobj, 0, cyl->radius, slices, 1 );
    }
    
  }; glPopMatrix ();
  
  gluDeleteQuadric( qobj );
}

static void
renderCone( vsgcNode *node, renderState *state )
{
  vsgcCone      *cone = (vsgcCone *) node;
  GLUquadricObj *qobj = gluNewQuadric();
  const int     slices = 10, stacks = 10;

  renderMaterial( state, USE_DIFFUSE_COLOR );

  if (state->compile) return;

  setBackfaceCull( state, cone->side && cone->bottom, FALSE );
  setVertexOrdering( state, TRUE, FALSE );
  setShadeModel( state, FALSE, FALSE );

  syncRender (state);

  gluQuadricDrawStyle( qobj, state->polygon ? GLU_FILL : GLU_LINE );
  gluQuadricNormals( qobj, GLU_FLAT );
  if ( state->pending.texturing ) gluQuadricTexture( qobj, GL_TRUE );

  /* GLU cylinder is oriented along z-axis, from z=0 to height */
  /* VRML cone is oriented along y-axis from z=-height/2 to height/2 */
  /* N.B. the GLU cylinder does not appear to be capped! */

  glPushMatrix (); {

    glTranslatef( 0.0f, -cone->height / 2.0, 0.0f ); 
    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );

    if ( cone->side )
      gluCylinder( qobj, cone->bottomRadius, 0, cone->height, slices, stacks );

    if ( cone->bottom ) {
      setVertexOrdering( state, FALSE, TRUE );
      gluDisk( qobj, 0, cone->bottomRadius, slices, 1 );
    }
  
  }; glPopMatrix ();
  
  gluDeleteQuadric( qobj );

}

static void
renderInline( vsgcNode *node, renderState *state )
{
  vsgcInline *inline_node = (vsgcInline *) node;
  int        i;
  vsgWorld   *save_wrl, *inline_wrl;
  
  for ( i = 0; i < inline_node->url->mf.len; i++ ) {

    /* get the scene graph of this inlined file */

    inline_wrl = FeatureGetInline( state->curr_wrl, inline_node->url->data[i]);
    if ( inline_wrl != NULL ) {
      
      /* render the first URL in the Inline node that works then break out */

      save_wrl = state->curr_wrl;
      state->curr_wrl = inline_wrl;
      renderMFNode( vsgWorldNodes( inline_wrl ), state );
      state->curr_wrl = save_wrl;
      break;
    }
  }
}

void
InitVRML( void )
{
  vsgcSetupCustomRender( (vsgCallback_t) renderGL );

  vsgcAppearanceClass.render = (renderType) renderAppearance;
  vsgcGroupClass.render = (renderType) renderGroup;
  vsgcCollisionClass.render = (renderType) renderCollision;
  vsgcAnchorClass.render = (renderType) renderAnchor;
  vsgcIndexedFaceSetClass.render = (renderType) renderIFS;
  vsgcIndexedLineSetClass.render = (renderType) renderILS;
  vsgcSphereClass.render = (renderType) renderSphere;
  vsgcCylinderClass.render = (renderType) renderCylinder;
  vsgcConeClass.render = (renderType) renderCone;
  vsgcBoxClass.render = (renderType) renderBox;
  vsgcTextClass.render = (renderType) renderText;
  vsgcMaterialClass.render = (renderType) rememberMaterial;
  vsgcImageTextureClass.render = (renderType) rememberTexture;
  vsgcShapeClass.render = (renderType) renderShape;
  vsgcTransformClass.render = (renderType) renderTransform;
  vsgcInlineClass.render = (renderType) renderInline;
}

/*
 * Synopsis:
 *   void tessBegin( GLenum type, IFSInfo *ifs_info )
 *
 * Description:
 *   This callback is invoked by the GLU tessellator every time
 *   it wants to through out a new geometric primitive. This could
 *   be any one of GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, or
 *   GL_TRIANGLES. We record which one and do some initialisation
 *   in preparaton for the tessVertex callbacks.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 24 June, 1999
 *
 */

static void CALLBACK
tessBegin( GLenum type, IFSInfo *ifs_info )
{
  ifs_info->type            = type;
  ifs_info->verts_this_poly = 0;
  ifs_info->tris_this_strip = 0;
  ifs_info->first_vertex    = TRUE;
  ifs_info->fan_pivot       = NULL;
  ifs_info->fan_last        = NULL;
  ifs_info->strip_last1     = NULL;
  ifs_info->strip_last2     = NULL;
}

/*
 * Synopsis:
 *   void tessEnd( IFSInfo *ifs_info )
 *
 * Description:
 *   This callback gets invoked by the GLU tessellator whenever it
 *   is finished defining a geometric primitive. In our case, this
 *   means terminating the appropriate index lists with a -1, in
 *   standard VRML fashion.
 *
 *   Note, we also call this function in tessVertex in order to
 *   start a new triangle when we want to split up a tri fan or strip.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 24 June, 1999
 *
 */

static void CALLBACK
tessEnd( IFSInfo *ifs_info )
{
  vsgWorld *wrl = ifs_info->wrl;

  /* output terminators (-1) to all of the index lists that need them */

  vsgAppendSFInt32( ifs_info->wrl, ifs_info->coord_index, -1 );

  /* terminate our new color index for this polygon, if appropriate */

  if ( ifs_info->color_index && ifs_info->ifs->colorPerVertex )
    vsgAppendSFInt32( wrl, ifs_info->color_index, -1 );

  /* terminate our new texture coordinate index, if appropriate */

  if ( ifs_info->texcoord_index )
    vsgAppendSFInt32( wrl, ifs_info->texcoord_index, -1 );

  /* terminate our new normal index, if appropriate */

  if ( ifs_info->normal_index && ifs_info->ifs->normalPerVertex )
      vsgAppendSFInt32( wrl, ifs_info->normal_index, -1 );

  /* reset the number of vertices for the next polygon and increment */
  /* the count of triangles we have created since tessBegin.         */

  ifs_info->verts_this_poly = 0;
  ifs_info->tris_this_strip++;
}

/*
 * Synopsis:
 *   void tessOutputVertex( VertexInfo *v_info, IFSInfo *ifs_info )
 *
 * Description:
 *   This function will take the specified vertex and will output
 *   a reference to it in the appropriate index lists, e.g. in 
 *   the coordIndex list for the IFS. This function is used by
 *   the tessVertex() callback, defined below.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 24 June, 1999
 *
 */

static void
tessOutputVertex( VertexInfo *v_info, IFSInfo *ifs_info )
{
  vsgWorld *wrl = ifs_info->wrl;

  /* add this vertex index to the new index list that we are creating */

  vsgAppendSFInt32( wrl, ifs_info->coord_index, v_info->vertex_index );

  /* add this color index to the new list, if appropriate */

  if ( ifs_info->color_index )
    if ( ifs_info->ifs->colorPerVertex || ifs_info->verts_this_poly == 0 )
      vsgAppendSFInt32( wrl, ifs_info->color_index, v_info->color_index );

  /* add this texture coordinate index to the new list, if appropriate */

  if ( ifs_info->texcoord_index )
    vsgAppendSFInt32( wrl, ifs_info->texcoord_index, v_info->texcoord_index );

  /* add this normal index to the new list, if appropriate */

  if ( ifs_info->normal_index )
    if ( ifs_info->ifs->normalPerVertex || ifs_info->verts_this_poly == 0 )
      vsgAppendSFInt32( wrl, ifs_info->normal_index, v_info->normal_index );

  ifs_info->verts_this_poly++;
  ifs_info->first_vertex = FALSE;
}

/*
 * Synopsis:
 *   void tessVertex( VertexInfo *v_info, IFSInfo *ifs_info )
 *
 * Description:
 *   This callback is invoked by the GLU tessellator whenever it
 *   wants to define a new vertex in the contour that it has
 *   tessellated. We want to store this in a VRML97 IFS node,
 *   which only handles polygon primitives. However, we could
 *   be getting vertices for a triangle fan, a triangle strip,
 *   or a list of triangles. We therefore need to do a little bit
 *   of smarts in order to convert these primitives into -1
 *   separated triangles in the appropriate IFS node index lists.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 24 June, 1999
 *
 */

static void CALLBACK
tessVertex( VertexInfo *v_info, IFSInfo *ifs_info )
{
  switch ( ifs_info->type ) {
  case GL_TRIANGLE_FAN:

    /* a triangle fan of [0, 1, 2, 3, 4] gets converted to a list */
    /* of triangles, [0, 1, 2], [0, 2, 3], [0, 3, 4] and so on. */ 

    if ( ifs_info->first_vertex )
      ifs_info->fan_pivot = v_info;

    if ( ifs_info->verts_this_poly == 3 ) {
      tessEnd( ifs_info );
      tessOutputVertex( ifs_info->fan_pivot, ifs_info );
      tessOutputVertex( ifs_info->fan_last, ifs_info );
    }

    tessOutputVertex( v_info, ifs_info );
    
    ifs_info->fan_last = v_info;
    break;

  case GL_TRIANGLE_STRIP:

    /* a triangle strip of [0, 1, 2, 3, 4] gets converted to a list */
    /* of triangles, [0, 1, 2], [2, 1, 3], [2, 3, 4] and so on.     */

    if ( ifs_info->verts_this_poly == 3 ) {
      tessEnd( ifs_info );
      if ( ifs_info->tris_this_strip % 2 == 0 /* is even */ ) {
	tessOutputVertex( ifs_info->strip_last2, ifs_info );
	tessOutputVertex( ifs_info->strip_last1, ifs_info );
      } else {
	tessOutputVertex( ifs_info->strip_last1, ifs_info );
	tessOutputVertex( ifs_info->strip_last2, ifs_info );
      }
    }

    tessOutputVertex( v_info, ifs_info );

    ifs_info->strip_last2 = ifs_info->strip_last1;
    ifs_info->strip_last1 = v_info;

    break;

  case GL_TRIANGLES:

    /* a triangle set of [0, 1, 2, 3, 4, 5] gets converted to */
    /* a list of triangles, [0, 1, 2], [3, 4, 5], and so on.  */

    if ( ifs_info->verts_this_poly == 3 )
      tessEnd( ifs_info );

    tessOutputVertex( v_info, ifs_info );

    break;
  }
}

/*
 * Synopsis:
 *   void makeConvexIFS( vsgcIndexedFaceSet *ifs, renderState *state )
 *
 * Description:
 *   This function will take an IFS node that is marked as being concave
 *   and will tessellate this into triangles so that the OpenGL routines
 *   can render them properly. We use the GLU tessellator object in order
 *   to perform this tessellation. At the end of this function, unless
 *   any error occurred, the index lists of the IFS node will have been
 *   updated to define only triangles, and the convex field of the node
 *   will be set to TRUE. N.B. we do not change any of the vertices 
 *   because this node might be DEF/USE'd somewhere else.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 24 June, 1999
 *
 */

static void
makeConvexIFS( vsgcIndexedFaceSet *ifs, renderState *state )
{
#ifndef GLU_VERSION_1_2
  /* we currently require GLU 1.2 for the concave->convex tessellation */
  return;
#else
  /* references to the various nodes in the IndexedFaceSet */

  vsgcCoordinate        *coord = NULL;
  vsgcColor             *color = NULL;
  vsgcTextureCoordinate *texcoord = NULL;
  vsgcNormal            *normal = NULL;

  /* the various data fields for the IFS */

  SFVec3f               *coords = NULL;
  SFInt32               *coordIndex = NULL;
  SFInt32               *colorIndex = NULL;
  SFVec3f               *normals = NULL;
  SFInt32               *normalIndex = NULL;
  SFVec2f               *texCoords = NULL;
  SFInt32               *texCoordIndex = NULL;

  int                   coordIndexLen;

  /* the various rendering options of the IFS */

  int                   i, curr_poly = 0;

  /* state to record all of the properties of this poly/vertex */

  VertexInfo            *vinfo;
  IFSInfo               ifs_info = { NULL };

  /* state to support the concave -> convex tessellation */

  GLUtesselator         *tobj = NULL;
  GLdouble              *vertex;

  /* grab pointers to everything that has been defined in this node */

  coord = (vsgcCoordinate *) vsgcEffectiveNode (ifs->coord);
  if ( coord == NULL || coord->point == NULL ) return;

  if ( ifs->color )
    color = (vsgcColor *) vsgcEffectiveNode (ifs->color);
  if ( ifs->texCoord )
    texcoord = (vsgcTextureCoordinate *) vsgcEffectiveNode( ifs->texCoord );
  if ( ifs->normal )
    normal = (vsgcNormal *) vsgcEffectiveNode( ifs->normal );

  coordIndex       = ifs->coordIndex->data;
  coordIndexLen    = ifs->coordIndex->mf.len;
  colorIndex       = ( ifs->colorIndex ) ? ifs->colorIndex->data : NULL;
  normals          = ( normal ) ? normal->vector->data : NULL;
  normalIndex      = ( ifs->normalIndex ) ? ifs->normalIndex->data : NULL;
  texCoords        = (texcoord && texcoord->point)?texcoord->point->data:NULL;
  texCoordIndex    = ( ifs->texCoordIndex ) ? ifs->texCoordIndex->data : NULL;
  coords           = coord->point->data;

  /* only do this routine if the IFS is flagged as being concave */

  if ( ifs->convex == TRUE ) return;

  /* initialise the IFSInfo node to contain all state for the conversion */

  ifs_info.ifs = ifs;
  ifs_info.wrl = state->curr_wrl;

  if ( ( ifs_info.coord_index = vsgStartMFInt32( state->curr_wrl ) ) == NULL )
    return;

  if ( colorIndex != NULL )
    ifs_info.color_index = vsgStartMFInt32( state->curr_wrl );

  if ( normalIndex != NULL )
    ifs_info.normal_index = vsgStartMFInt32( state->curr_wrl );

  if ( texCoordIndex != NULL )
    ifs_info.texcoord_index = vsgStartMFInt32( state->curr_wrl );

  ifs_info.vlist = list_create( FALSE );
  ifs_info.dlist = list_create( FALSE );

  /* initialise the GLU tesselator */

  if ( ( tobj = gluNewTess() ) == NULL ) return;

  gluTessCallback( tobj, GLU_TESS_BEGIN_DATA, tessBegin );
  gluTessCallback( tobj, GLU_TESS_VERTEX_DATA, tessVertex );
  gluTessCallback( tobj, GLU_TESS_END_DATA, tessEnd );
  gluTessCallback( tobj, GLU_TESS_COMBINE_DATA, NULL );
  gluTessCallback( tobj, GLU_TESS_ERROR_DATA, NULL );
  
  /* tessellate polygons for the IndexedFaceSet */

  gluTessBeginPolygon( tobj, &ifs_info );
  gluTessBeginContour( tobj );

  for (i = 0; i < coordIndexLen; ++i) {
    int idx = coordIndex[i];

    if ( idx == -1 ) {
      
      /* end of current polygon definition: start a new polygon */

      gluTessEndContour( tobj );
      gluTessEndPolygon( tobj );
      gluTessBeginPolygon( tobj, &ifs_info );
      gluTessBeginContour( tobj );
      
      curr_poly++;
      continue;

    }

    /* create a new VertexInfo structure for this vertex */

    vinfo = (VertexInfo *) tsmMemCalloc( sizeof(VertexInfo), 1 );
    if ( vinfo == NULL ) break;

    /*----------- support colour fields of the IFS node ---------*/

    if ( color ) {
      if ( ifs->colorPerVertex == FALSE )

	/* colorPerVertex is FALSE - use colorIndex if not empty */
	/* otherwise apply the colours of Color node in order. */

	vinfo->color_index = (colorIndex) ? colorIndex[curr_poly] : curr_poly;

      else

	/* colorPerVertex is TRUE - use colorIndex if not empty */
	/* otherwise we use the coordIndex to choose colors. */
	
	vinfo->color_index = ( colorIndex ) ? colorIndex[i] : idx;
      
      vinfo->do_color = TRUE;
    }
    
    /*-------- support texture coordinate fields of the IFS node -------*/
    
    if ( texCoords ) {
      
      /* use the texCoordIndex field (if specified) to look-up into */
      /* the texCoord array; otherwise we use the coordIndex field. */
      
      vinfo->texcoord_index = ( texCoordIndex ) ? texCoordIndex[i] : idx;
      vinfo->do_texcoords = TRUE;
    }
    
    /*------------ support normal fields of the IFS node ----------*/
    
    if ( normals ) {
      if ( ifs->normalPerVertex == FALSE )
	
	/* normalPerVertex is FALSE - use normalIndex if not empty */
	/* otherwise apply normals in the order they are specified */
	
	vinfo->normal_index = ( normalIndex ) ? normalIndex[curr_poly] :
	                      curr_poly;
      
      else
	
	/* normalPerVertex is TRUE - use normalIndex if not empty */
	/* otherwise we use the coordIndex to choose normals. */
	
	vinfo->normal_index = ( normalIndex ) ? normalIndex[i] : idx;
      
      vinfo->do_normals = TRUE;
    }
    
    /*---------- support coordinate fields of the IFS node --------*/
    
    vinfo->vertex_index = idx;
    
    /*-------- render the contents of the VertexInfo struct -------*/
    
    /* create a GLdouble[] version of the vertex for the tesselator */
    
    vertex = (GLdouble *) tsmMemMalloc( sizeof(GLdouble) * 3 );
    if ( vertex != NULL ) {
      vertex[0] = (GLdouble) coords[idx].v[0];
      vertex[1] = (GLdouble) coords[idx].v[1];
      vertex[2] = (GLdouble) coords[idx].v[2];
      
      /* add this vertex to our list so we can free it later, and */
      /* then give it to the GLU tessellator for it to process.   */
      
      list_push( ifs_info.vlist, vinfo );
      list_push( ifs_info.dlist, vertex );
      
      gluTessVertex( tobj, vertex, vinfo );
    }

  }

  gluTessEndContour( tobj );
  gluTessEndPolygon( tobj );
  gluDeleteTess( tobj );

  /* close our new index lists that were filled by the tessellator */

  vsgEndMFInt32( state->curr_wrl, ifs_info.coord_index );
  if ( ifs_info.color_index )
    vsgEndMFInt32( state->curr_wrl, ifs_info.color_index );
  if ( ifs_info.normal_index )
    vsgEndMFInt32( state->curr_wrl, ifs_info.normal_index );
  if ( ifs_info.texcoord_index )
    vsgEndMFInt32( state->curr_wrl, ifs_info.texcoord_index );
  
  /* deallocate all of the VertexInfo structures that we made */

  if ( ifs_info.vlist ) {
    list_iter_t i = list_first (ifs_info.vlist);
    for ( ; ! list_done (i); i = list_next (i) ) {
      VertexInfo *v = (VertexInfo *) list_value( i );
      if ( v != NULL ) tsmMemFree( v );
    }
    list_destroy( ifs_info.vlist );
  }

  /* deallocate all of the GLdouble[3] vertex structures that we made */

  if ( ifs_info.dlist ) {
    list_iter_t i = list_first( ifs_info.dlist );
    for ( ; ! list_done( i ); i = list_next( i ) ) {
      VertexInfo *v = (VertexInfo *) list_value( i );
      if ( v != NULL ) tsmMemFree( v );
    }
    list_destroy( ifs_info.dlist );
  }

  /* check that we were able to make a valid index table */

  if ( ifs_info.coord_index->mf.len == 0 )
    return;

  /* now, update the IFS fields for the new convex polygon definition */

  ifs->coordIndex    = ifs_info.coord_index;
  ifs->colorIndex    = ifs_info.color_index;
  ifs->normalIndex   = ifs_info.normal_index;
  ifs->texCoordIndex = ifs_info.texcoord_index;
  ifs->convex        = TRUE;

#endif
}

typedef struct {
  SFInt32     idx;                      /* index of current vertex in IFS */
  GLdouble    coord[3]; 
  SFColor     *color;                   /* Non-zero iff color attached. */
  SFVec3f     *normal;                  /* normal vector for this vertex */
  SFVec2f     *st;                      /* texture coords for this vertex */
} IFSOptVertex;

typedef struct {
  vsgWorld *wrl;
  MFInt32  *accum;
  MFColor  *colors;
  MFVec3f  *normals;
  MFVec2f  *st;
  GLenum   type;
  int      start;
} IFSOptInfo;

static void
optCommand (IFSOptInfo *ifs_info, char op, unsigned int idx)
{
  IFSOptCommand cmd;
  cmd.op.op = op;
  cmd.op.idx = idx;
  vsgAppendSFInt32 (ifs_info->wrl, ifs_info->accum, cmd.i32);
}

static void
optStart (GLenum type, IFSOptInfo *ifs_info)
{
  ifs_info->type = type;
  ifs_info->start = ifs_info->accum->mf.len;
  optCommand (ifs_info, 'p', type);
#if TRACE_OPT
  printf ("%p[%d]: %d %s {\n", ifs_info, ifs_info->start, type,
	  type==GL_TRIANGLE_FAN ? "TRIANGLE_FAN" 
	  : type==GL_TRIANGLE_STRIP ? "TRIANGLE_STRIP"
	  : "TRIANGLES");
#endif
}

static void
optEnd (IFSOptInfo *ifs_info)
{
  if (ifs_info->type != 0) {
#if TRACE_OPT
    printf ("};\n");
#endif
    ifs_info->type = 0;
  }
}

static void CALLBACK
optBegin( GLenum type, IFSOptInfo *ifs_info )
{
  switch (type) {
  case GL_TRIANGLE_FAN:
  case GL_TRIANGLE_STRIP:
    optEnd (ifs_info);
    optStart (type, ifs_info);
    break;

  case GL_TRIANGLES:
    if (ifs_info->type != type) {
      optEnd (ifs_info);
      optStart (type, ifs_info);
    }
    break;

  default:
    printf ("HUH?\n");
    break;
  }
}

static void CALLBACK
optVertex( IFSOptVertex *v, IFSOptInfo *ifs_info )
{
  int idx = v->idx;
#if TRACE_OPT
  printf (" [%d]", idx);
  if (v->normal)
    printf (" n[%.3f:%.3f:%.3f]",
	    v->normal->v[0],
	    v->normal->v[1],
	    v->normal->v[2]);
  if (v->st)
    printf (" t[%.3f:%.3f]",
	    v->st->v[0],
	    v->st->v[1]);
  if (v->color)
    printf (" c[%.0f%%:%.0f%%:%.0f%%]",
	    v->color->r*100,
	    v->color->g*100,
	    v->color->b*100);
  printf ("\n");
#endif

  if (v->color) {
    vsgAppendSFColor (ifs_info->wrl, ifs_info->colors, *v->color);
    optCommand (ifs_info, 'c', ifs_info->colors->mf.len-1);
  }

  if (v->normal) {
    vsgAppendSFVec3f (ifs_info->wrl, ifs_info->normals, *v->normal);
    optCommand (ifs_info, 'n', ifs_info->normals->mf.len-1);
  }

  if (v->st) {
    vsgAppendSFVec2f (ifs_info->wrl, ifs_info->st, *v->st);
    optCommand (ifs_info, 't', ifs_info->st->mf.len-1);
  }

  optCommand (ifs_info, 'v', idx);
}

static void CALLBACK
optError( GLenum err)
{
}

static MFInt32 *
optimizeIFS( vsgcIndexedFaceSet *ifs, renderState *state )
{
#ifndef GLU_VERSION_1_2
  /* we currently require GLU 1.2 for the concave->convex tessellation */
  return NULL;
#elif defined(WIN32)
  /* this concave->convex conversion crashes under Win32 for some reason?? */
  return NULL;
#else
  IFSAux *ifsAux = ifs->super.aux;

  vsgcColor             *color = ifsAux->color;

  /* the various data fields for the IFS */

  SFVec3f               *coords = ifsAux->coords;
  SFInt32               *coordIndex = ifsAux->coordIndex;
  SFColor               *colors = ifsAux->colors;
  SFInt32               *colorIndex = ifsAux->colorIndex;
  SFVec3f               *normals = ifsAux->normals;
  SFInt32               *normalIndex = ifsAux->normalIndex;
  SFVec2f               *texCoords = ifsAux->texCoords;
  SFInt32               *texCoordIndex = ifsAux->texCoordIndex;

  int                   coordIndexLen = ifsAux->coordIndexLen;

  /* the various rendering options of the IFS */

  SFBool                texturing = !!state->texture;
  SFBool                colorPerVertex = ifsAux->colorPerVertex;
  SFBool                normalPerVertex = ifsAux->normalPerVertex;

  /* the various rendering options of the IFS */

  double                bbox_min[3], bbox_max[3];
  int                   s_index, t_index;

  int                   curr_poly = 0, curr_normal = 0;
  int                   i, first = TRUE;

  /* state to support the concave -> convex tessellation */

  GLUtesselator         *tobj = NULL;

  IFSOptInfo            ifs_info;

  istack                *st = istack_create (1024);
       
  /* initialise the IFSInfo node to contain all state for the conversion */

  ifs_info.wrl  = state->curr_wrl;
  ifs_info.type = 0;

  if ( ( ifs_info.accum = vsgStartMFInt32( state->curr_wrl ) ) == NULL )
    return NULL;

  ifs_info.colors = 0;
  if ( color ) {
    if ( ( ifs_info.colors = vsgStartMFColor( state->curr_wrl ) ) == NULL )
      return NULL;
  }

  ifs_info.normals = 0;
  if ( ( ifs_info.normals = vsgStartMFVec3f( state->curr_wrl ) ) == NULL )
    return NULL;

  ifs_info.st = 0;
  if ( texturing ) {
    if ( ( ifs_info.st = vsgStartMFVec2f( state->curr_wrl ) ) == NULL )
      return NULL;
  }

  /* initialise the GLU tesselator */

  if ( ( tobj = gluNewTess() ) == NULL ) return NULL;

  gluTessCallback( tobj, GLU_TESS_BEGIN_DATA, optBegin );
  gluTessCallback( tobj, GLU_TESS_VERTEX_DATA, optVertex );
  gluTessCallback( tobj, GLU_TESS_END_DATA, NULL );
  gluTessCallback( tobj, GLU_TESS_COMBINE_DATA, NULL );
  gluTessCallback( tobj, GLU_TESS_ERROR_DATA, optError );
  
  /* tessellate polygons for the IndexedFaceSet */

  gluTessBeginPolygon( tobj, &ifs_info );
  gluTessBeginContour( tobj );

  if ( texturing && !texCoords ) {
    calcBBox( coords, coordIndex, coordIndexLen, 0, bbox_min, bbox_max );
    calcST( bbox_min, bbox_max, &s_index, &t_index ); 
  }

#if TRACE_OPT
  printf ("ifs %p texturing %d texCoords %p\n", ifs, texturing, texCoords);
#endif

  for (i = 0; i < coordIndexLen; ++i) {
    int idx = coordIndex[i];
    IFSOptVertex *vtx = istack_alloc (st, sizeof (IFSOptVertex));

    if ( idx == -1 ) {
      
      /* end of current polygon definition: start a new polygon */

      gluTessEndContour( tobj );
      gluTessEndPolygon( tobj );
      gluTessBeginPolygon( tobj, &ifs_info );
      gluTessBeginContour( tobj );
      
      /* get the bounding box info for this new polygon */

      if ( texturing && !texCoords ) {
	calcBBox( coords, coordIndex, coordIndexLen, i+1, bbox_min, bbox_max );
	calcST( bbox_min, bbox_max, &s_index, &t_index );
      }

      first = TRUE;
      continue;
    }

    /* in the midst of a polygon definition: first thing */
    /* to do is to set the per-face or per-vertex colour */

    /*----------- support colour fields of the IFS node ---------*/

    vtx->color = 0;
    if ( color ) {
      if ( colorPerVertex == FALSE && first ) {

	/* colorPerVertex is FALSE - use colorIndex if not empty */

	/* otherwise apply the colours of Color node in order. */

	SFColor *c = ( colorIndex ) ?
	  &colors[colorIndex[curr_poly++]] : &colors[curr_poly++];

	vtx->color = c;
	  
      } else if ( colorPerVertex == TRUE ) {
	  
	/* colorPerVertex is TRUE - use colorIndex if not empty */
	/* otherwise we use the coordIndex to choose colors. */

	SFColor *c = ( colorIndex ) ? &colors[colorIndex[i]] : &colors[idx];

	vtx->color = c;
      }
    }

    /*-------- support texture coordinate fields of the IFS node -------*/

    vtx->st = 0;
    if ( texturing ) {
      if ( texCoords ) {

	/* use the texCoordIndex field (if specified) to look-up into */
	/* the texCoord array; otherwise we use the coordIndex field. */

	SFVec2f *n = ( texCoordIndex ) ?
	  &texCoords[texCoordIndex[i]] : &texCoords[idx];

	vtx->st = n;
	  
      } else {

	/* if the texCoord field is NULL, but a texture map image was  */
	/* specified, then we need to generate texture coordinates for */
	/* the user as per the VRML97 spec. This is done here.         */
	SFVec2f *uv = istack_alloc (st, sizeof(SFVec2f));
	autoGenTexCoords( bbox_min, bbox_max, s_index, t_index,
			  &coords[idx], &uv->v[0], &uv->v[1] );

	vtx->st = uv;
      }
    }

    /*------------ support normal fields of the IFS node ----------*/

    vtx->normal = 0;
    if ( normals ) {
      if ( normalPerVertex == FALSE && first ) {

	/* normalPerVertex is FALSE - use normalIndex if not empty */
	/* otherwise apply normals in the order they are specified */

	SFVec3f *n = ( normalIndex ) ?
	  &normals[normalIndex[curr_normal++]] : &normals[curr_normal++];

	vtx->normal = n;

      } else if ( normalPerVertex == TRUE ) {

	/* normalPerVertex is TRUE - use normalIndex if not empty */
	/* otherwise we use the coordIndex to choose normals. */

	SFVec3f *n = ( normalIndex ) ?
	  &normals[normalIndex[i]] : &normals[idx];

	vtx->normal = n;
      }

    } else if ( first ) { /* no normal information specified */

      /* generate per-face normals for the polygon on-the-fly */
      SFVec3f *norm = istack_alloc (st, sizeof(SFVec3f));

      calcIFSNormal( coords, coordIndex, i, ifs->ccw, state, norm->v );

      vtx->normal = norm;
    }

    /* remember the vertex index */
    vtx->idx = idx;

    /*-------- render the contents of the IFSOptVertex struct -------*/
    
    /* create a GLdouble[] version of the vertex for the tesselator */

    vtx->coord[0] = (GLdouble) coords[idx].v[0];
    vtx->coord[1] = (GLdouble) coords[idx].v[1];
    vtx->coord[2] = (GLdouble) coords[idx].v[2];
      
    gluTessVertex( tobj, vtx->coord, vtx );
    first = FALSE;
  }

  gluTessEndContour( tobj );
  gluTessEndPolygon( tobj );
  gluDeleteTess( tobj );

  istack_destroy (st);

  /* close our new index lists that were filled by the tessellator */

  if ( ifs_info.accum->mf.len != 0)
    optEnd (&ifs_info);

  vsgEndMFInt32( state->curr_wrl, ifs_info.accum );

  if ( ifs_info.colors )
    vsgEndMFColor( state->curr_wrl, ifs_info.colors );
  ifsAux->optColors = ifs_info.colors;

  if ( ifs_info.normals )
    vsgEndMFVec3f( state->curr_wrl, ifs_info.normals );
  ifsAux->optNormals = ifs_info.normals;
  
  if ( ifs_info.st )
    vsgEndMFVec2f( state->curr_wrl, ifs_info.st );
  ifsAux->optTexture = ifs_info.st;

  /* check that we were able to make a valid index table */

  if ( ifs_info.accum->mf.len == 0 )
    return NULL;

  /* now, update the IFS fields for the new convex polygon definition */
  return ifs_info.accum;
#endif
}
