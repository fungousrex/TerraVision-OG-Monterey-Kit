/*
 * Name: c_nodes.c
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: c_nodes.c,v 1.9.6.2 2001/05/14 18:40:21 leei Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stddef.h>
#include <stdlib.h>

#include "c_nodesI.h"
#include "syms.h"

static SFFloat SFFloat0 = 0;
static SFFloat SFFloat1 = 1;
static SFFloat SFFloat2 = 2;
static SFFloat SFFloat10 = 10;
static SFFloat SFFloat100 = 100;
static SFFloat SFFloat0_2 = 0.2;
static SFFloat SFFloat0_262 = 0.262;
static SFFloat SFFloatNeg1 = -1;
static SFFloat SFFloatPi_2 = 1.570796;
static SFFloat SFFloatPi_4 = 0.785398;

static SFTime SFTime0 = 0;
static SFTime SFTime1 = 1;

static SFVec2f SFVec2fZero = {{0, 0}};
static SFVec2f SFVec2fOne = {{1, 1}};
static SFVec2f SFVec2fNeg1 = {{-1, -1}};

static SFVec3f SFVec3fZero = {{0, 0, 0}};
static SFVec3f SFVec3fOne = {{1, 1, 1}};
static SFVec3f SFVec3fNeg1 = {{-1, -1, -1}};
static SFVec3f SFVec3fTwo = {{2, 2, 2}};
static SFVec3f SFVec3fXAxis = {{1, 0, 0}};
static SFVec3f SFVec3fYAxis = {{0, 1, 0}};
static SFVec3f SFVec3fZAxis = {{0, 0, 1}};
static SFVec3f SFVec3fDirDown = {{0, 0, -1}};
static SFVec3f SFVec3fViewpos = {{0, 0, 10}};

static SFColor SFColorWhite = {1, 1, 1};
static SFColor SFColorGray80 = {.8, .8, .8};
static SFColor SFColorBlack = {0, 0, 0};

static SFRotation SFRotationYAxis = {0, 1, 0, 0};
static SFRotation SFRotationZAxis = {0, 0, 1, 0};

static SFImage SFImageEmpty = {0, 0, 0};

static MFColor MFColorEmpty_s = {
  {vsgMFColor, 0, 0}, 0 };
#define MFColorEmpty (&MFColorEmpty_s)

static SFColor MFColorBlack_v[] = {
  {0, 0, 0} };
static MFColor MFColorBlack_s = {
  {vsgMFColor, 0, 1}, MFColorBlack_v };
#define MFColorBlack (&MFColorBlack_s)

static MFInt32 MFInt32Empty_s = {
  {vsgMFInt32, 0, 0}, 0 };
#define MFInt32Empty (&MFInt32Empty_s)

static MFFloat MFFloatEmpty_s = {
  {vsgMFFloat, 0, 0}, 0 };
#define MFFloatEmpty (&MFFloatEmpty_s)

static SFFloat MFFloatAvatar_v[] = {
  0.25, 1.6, 0.75 };
static MFFloat MFFloatAvatar_s = {
  {vsgMFFloat, 0, 3}, MFFloatAvatar_v };
#define MFFloatAvatar (&MFFloatAvatar_s)

static MFNode MFNodeEmpty_s = {
  {vsgMFNode, 0, 0}, 0 };
#define MFNodeEmpty (&MFNodeEmpty_s)

static MFRotation MFRotationEmpty_s = {
  {vsgMFRotation, 0, 0}, 0 };
#define MFRotationEmpty (&MFRotationEmpty_s)

static SFRotation MFRotationZout_v[] = {
  {0, 0, 1, 0} };
static MFRotation MFRotationZout_s = {
  {vsgMFRotation, 0, 1}, MFRotationZout_v };
#define MFRotationZout (&MFRotationZout_s)

static MFString MFStringEmpty_s = {
  {vsgMFString, 0, 0}, 0 };
#define MFStringEmpty (&MFStringEmpty_s)

static SFString MFStringSERIF_v[] = {
  "SERIF" };
static MFString MFStringSERIF_s = {
  {vsgMFString, 0, 1}, MFStringSERIF_v };
#define MFStringSERIF (&MFStringSERIF_s)

static SFString MFStringBEGIN_v[] = {
  "BEGIN" };
static MFString MFStringBEGIN_s = {
  {vsgMFString, 0, 1}, MFStringBEGIN_v };
#define MFStringBEGIN (&MFStringBEGIN_s)

static SFString MFStringNavType_v[] = {
  "WALK", "ANY" };
static MFString MFStringNavType_s = {
  {vsgMFString, 0, 2}, MFStringNavType_v };
#define MFStringNavType (&MFStringNavType_s)

static MFVec2f MFVec2fEmpty_s = {
  {vsgMFVec2f, 0, 0}, 0 };
#define MFVec2fEmpty (&MFVec2fEmpty_s)

static SFVec2f MFVec2fCrossSection_v[] = {
  {{1, 1}}, {{1, -1}}, {{-1, -1}}, {{-1, 1}}, {{1, 1}} };
static MFVec2f MFVec2fCrossSection_s = {
  {vsgMFVec2f, 0, 5}, MFVec2fCrossSection_v };
#define MFVec2fCrossSection (&MFVec2fCrossSection_s)

static SFVec2f MFVec2fIdentity_v[] = {
  {{1, 1}} };
static MFVec2f MFVec2fIdentity_s = {
  {vsgMFVec2f, 0, 1}, MFVec2fIdentity_v };
#define MFVec2fIdentity (&MFVec2fIdentity_s)

static MFVec3f MFVec3fEmpty_s = {
  {vsgMFVec3f, 0, 0}, 0 };
#define MFVec3fEmpty (&MFVec3fEmpty_s)

static SFVec3f MFVec3fSpine_v[] = {
  {{0, 0, 0}}, {{0, 1, 0}} };
static MFVec3f MFVec3fSpine_s = {
  {vsgMFVec3f, 0, 2}, MFVec3fSpine_v };
#define MFVec3fSpine (&MFVec3fSpine_s)


#define INIT_Anchor(wrl, a)
#define INIT_Appearance(wrl, a)
#define INIT_AudioClip(wrl, a)
#define INIT_Background(wrl, a) {		\
  vsgExtendBackgroundStack (wrl, (SFNode) a);	\
}
#define INIT_Billboard(wrl, a)
#define INIT_Box(wrl, a)
#define INIT_Collision(wrl, a)
#define INIT_Color(wrl, a)
#define INIT_ColorInterpolator(wrl, a)
#define INIT_Cone(wrl, a)
#define INIT_Coordinate(wrl, a)
#define INIT_CoordinateInterpolator(wrl, a)
#define INIT_Cylinder(wrl, a)
#define INIT_CylinderSensor(wrl, a)
#define INIT_DirectionalLight(wrl, a)
#define INIT_ElevationGrid(wrl, a)
#define INIT_Extrusion(wrl, a)
#define INIT_Fog(wrl, a) {			\
  vsgExtendFogStack (wrl, (SFNode)a);		\
}
#define INIT_FontStyle(wrl, a)
#define INIT_Group(wrl, a)
#define INIT_IdiotImage(wrl, a)
#define INIT_Image(wrl, a)
#define INIT_ImageTexture(wrl, a)
#define INIT_IndexedFaceSet(wrl, a)
#define INIT_IndexedLineSet(wrl, a)
#define INIT_Inline(wrl, a)
#define INIT_LOD(wrl, a)
#define INIT_Material(wrl, a)
#define INIT_MovieTexture(wrl, a)
#define INIT_NavigationInfo(wrl, a) {			\
  vsgExtendNavigationInfoStack (wrl, (SFNode) a);	\
}
#define INIT_Normal(wrl, a)
#define INIT_NormalInterpolator(wrl, a)
#define INIT_OrientationInterpolator(wrl, a)
#define INIT_PixelTexture(wrl, a)
#define INIT_PlaneSensor(wrl, a)
#define INIT_PointLight(wrl, a)
#define INIT_PointSet(wrl, a)
#define INIT_PositionInterpolator(wrl, a)
#define INIT_ProximitySensor(wrl, a)
#define INIT_ScalarInterpolator(wrl, a)
#define INIT_Script(wrl, a)
#define INIT_Shape(wrl, a)
#define INIT_Sound(wrl, a)
#define INIT_Sphere(wrl, a)
#define INIT_SphereSensor(wrl, a)
#define INIT_SpotLight(wrl, a)
#define INIT_Switch(wrl, a)
#define INIT_Text(wrl, a)
#define INIT_TextureCoordinate(wrl, a)
#define INIT_TextureTransform(wrl, a)
#define INIT_TimeSensor(wrl, a)
#define INIT_TouchSensor(wrl, a)
#define INIT_Transform(wrl, a)
#define INIT_Viewpoint(wrl, a) {		\
  vsgExtendViewpointStack (wrl, (SFNode) a);	\
}
#define INIT_VisibilitySensor(wrl, a)
#define INIT_WorldInfo(wrl, a)

static SFNode vsgcNewNode (vsgWorld *curr, vsgClass const *cls, int size);

/* This defines the field access functions. */
#undef DEFNODE
#define DEFNODE(node)

#undef FIELD
#define FIELD(node,type,field,_val)					    \
int									    \
vsgc##node##_set_##field (vsgWorld *wrl, SFNode gn, unsigned int sel,	    \
			  type##Arg v)					    \
{									    \
  vsgc##node *n = (vsgc##node *) gn;					    \
  if (vsgTracing) {							    \
    vsgClass const *cl = vsgNodeClass (gn);				    \
    fprintf (stderr, "%s:%p.%s = ", cl->name, n, vsgN##field);		    \
    vsgPrintAnyArg (stderr, vsg##type, (vsgAnyArg *)&v);		    \
    putc ('\n', stderr);						    \
  }									    \
  n->field = v;								    \
  n->super.flags |= vsgcNode_DIRTY;					    \
  return vsg##type >= vsgSFBool;					    \
}									    \
									    \
type##Arg								    \
vsgc##node##_##field##_changed (vsgWorld *wrl, SFNode gn, unsigned int sel) \
{									    \
  vsgc##node *n = (vsgc##node *) gn;					    \
  return n->field;							    \
}

#undef ENDNODE
#define ENDNODE(node)

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef ENDNODE

/* This defines the field access functions (set). */
#undef DEFNODE
#define DEFNODE(node)					\
vsgEventInDesc						\
vsgc##node##_setFunc (vsgClass const *c, symbol_t name)	\
{							\
  vsgEventInDesc desc;

#undef FIELD
#define FIELD(node,_type,field,_val)		\
  if (name == vsgN##field) {			\
    desc.type = vsg##_type;			\
    desc.f._type = vsgc##node##_set_##field;	\
    return desc;				\
  }

#undef ENDNODE
#define ENDNODE(node)				\
  desc.type = vsgErrorType;			\
  return desc;					\
}

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef ENDNODE

/* This defines the field access functions (get). */
#undef DEFNODE
#define DEFNODE(node)					\
vsgEventOutDesc						\
vsgc##node##_getFunc (vsgClass const *c, symbol_t name)	\
{							\
  vsgEventOutDesc desc;

#undef FIELD
#define FIELD(node,_type,field,_val)			\
  if (name == vsgN##field) {				\
    desc.type = vsg##_type;				\
    desc.f._type = vsgc##node##_##field##_changed;	\
    return desc;					\
  }

#undef ENDNODE
#define ENDNODE(node)				\
  desc.type = vsgErrorType;			\
  return desc;					\
}

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef ENDNODE

/* This defines the field access functions (eventIn). */
#undef DEFNODE
#define DEFNODE(node)					\
vsgEventInDesc						\
vsgc##node##_eventIn (vsgClass const *c, symbol_t name)	\
{							\
  vsgEventInDesc desc;

#undef FIELD
#define FIELD(node,_type,field,_val)

#undef EVENTIN
#define EVENTIN(node,_type,field)		\
  if (name == vsgN##field) {			\
    desc.type = vsg##_type;			\
    desc.f._type = vsgc##node##_set_##field;	\
    return desc;				\
  }

#undef ENDNODE
#define ENDNODE(node)				\
  desc.type = vsgErrorType;			\
  return desc;					\
}

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef EVENTIN
#define EVENTIN(node,_type,field)
#undef ENDNODE

/* This defines the field access functions (eventOut). */
#undef DEFNODE
#define DEFNODE(node)						\
vsgEventOutDesc							\
vsgc##node##_eventOut (vsgClass const *c, symbol_t name)	\
{								\
  vsgEventOutDesc desc;

#undef FIELD
#define FIELD(node,_type,field,_val)

#undef EVENTOUT
#define EVENTOUT(node,_type,field)			\
  if (name == vsgN##field) {				\
    desc.type = vsg##_type;				\
    desc.f._type = vsgc##node##_##field##_changed;	\
    return desc;					\
  }

#undef ENDNODE
#define ENDNODE(node)				\
  desc.type = vsgErrorType;			\
  return desc;					\
}

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef EVENTOUT
#define EVENTOUT(node,_type,field)
#undef ENDNODE

/* This defines the field access functions (eventOut). */
#undef DEFNODE
#define DEFNODE(node)						\
vsgClassField							\
vsgc##node##_fields[] = {

#undef EVENTIN
#define EVENTIN(node,_type,field)				\
 { vsgTeventIn, vsg##_type, 0, vsgN##field, {0} },

#undef FIELD
#define FIELD(node,_type,field,_val)				\
 { vsgTfield, vsg##_type, 0, vsgN##field, {0} },

#undef EXPOSEDFIELD
#define EXPOSEDFIELD(node,_type,field,_val)			\
  { vsgTexposedField, vsg##_type, 0, vsgN##field, {0} },

#undef EVENTOUT
#define EVENTOUT(node,_type,field)				\
 { vsgTeventOut, vsg##_type, 0, vsgN##field, {0} },

#undef ENDNODE
#define ENDNODE(node)						\
 { (vsgFieldType) 0, (vsgDataType) 0, 0, 0, {0} }		\
};  

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef EXPOSEDFIELD
#define EXPOSEDFIELD(node,_type,field,_val)		\
  FIELD(node,_type,field,_val)				\
  EVENTIN(node,_type,field)				\
  EVENTOUT(node,_type,field)
#undef EVENTIN
#define EVENTIN(node,_type,field)
#undef EVENTOUT
#define EVENTOUT(node,_type,field)
#undef ENDNODE

static vsgClassField **
vsgcClass_fields (vsgClass const *c)
{
  vsgcClass *cl = (vsgcClass *) c;
  if (! cl->fields) {
    int i, n;
    for (i = 0; ; ++i) 
      if (cl->fieldvec[i].fieldType == 0)
	break;
    n = i;
    cl->fields = malloc ((n+1) * sizeof (cl->fields[0]));
    for (i = 0; i < n; ++i)
      cl->fields[i] = &cl->fieldvec[i];
    cl->fields[i] = 0;
  }
  return cl->fields;
}

static MFProto *
vsgcClass_protos (vsgClass const *c)
{
  return NULL;
}

/* This defines the field access functions (eventOut). */
#undef DEFNODE
#define DEFNODE(node)							  \
SFNode									  \
vsgc##node##_create (vsgClass const *cls, vsgWorld *wrl)		  \
{									  \
  vsgc##node *a = (vsgc##node *) vsgcNewNode (wrl, cls, sizeof (vsgc##node));

#define FIELD(node,_type,slot,_val)		\
  a->slot = (_val);
 
#undef ENDNODE
#define ENDNODE(node)				\
  INIT_##node (wrl, a);				\
  return (SFNode) a;				\
}						\
						\
vsgcClass vsgc##node##Class = {			\
  { vsgN##node,	0,				\
    vsgc##node##_create,			\
    vsgc##node##_setFunc,			\
    vsgc##node##_getFunc,			\
    vsgc##node##_eventIn,			\
    vsgc##node##_eventOut,			\
    vsgcClass_fields,				\
    vsgcClass_protos},				\
  vsgc##node##_fields,				\
  0,						\
  0						\
};

#include "nodes.def"

#undef DEFNODE
#undef FIELD
#undef ENDNODE

void
vsgcSetup (void)
{
  int i;
  if (vsgTracing)
    fprintf (stderr, "vsgcSetup\n");

#undef DEFNODE
#define DEFNODE(name)					\
  vsgBindClass (NULL, (vsgClass *) &vsgc##name##Class);	\
  i = 0;

#undef FIELD
#define FIELD(node,_type,slot,_val)			\
  vsgc##node##_fields[i].dflt.##_type = (_val);		\
  ++i;

#undef EXPOSEDFIELD
#define EXPOSEDFIELD(node,_type,slot,_val)		\
  vsgc##node##_fields[i].dflt.##_type = (_val);		\
  ++i;

#undef EVENTIN
#define EVENTIN(node,_type,field) ++i;

#undef EVENTOUT
#define EVENTOUT(node,_type,field) ++i;

#undef ENDNODE
#define ENDNODE(node)

#include "nodes.def"  
}

#undef DEFNODE
#undef FIELD
#undef EXPOSEDFIELD
#define EXPOSEDFIELD(node,_type,field,_val)		\
  FIELD(node,_type,field,_val)				\
  EVENTIN(node,_type,field)				\
  EVENTOUT(node,_type,field)
#undef EVENTIN
#define EVENTIN(node,_type,field)
#undef EVENTOUT
#define EVENTOUT(node,_type,field)
#undef ENDNODE

vsgcNode *
vsgcEffectiveNode (SFNode node)
{
  vsgClass const *cl;
  if (! node) return NULL;
  cl = vsgNodeClass (node);
  if (vsgClassIsProto (cl)) {
    vsgProtoClass const *proto = (vsgProtoClass const *) cl;
    MFNode *nodes = proto->nodes;
    node = (nodes && nodes->mf.len > 0) ? nodes->data[0] : NULL;
  }
  return (vsgcNode *) node;
}


#define LINTERP(f,v0,v1) ((1-f)*(v0) + (f)*(v1))

static int
interpolate (MFFloat const *key, float t, float *frac)
{
  int nkeys = key->mf.len;
  SFFloat const *keyv = key->data, *keyend = &keyv[nkeys-1];
  *frac = 0;
  if (t <= keyv[0])
    return 0;
  else if (t >= keyv[nkeys-1])
    return nkeys-1;

  for ( ; keyv < keyend; ++keyv) {
    if (keyv[1] <= keyv[0]) {
      return keyv - key->data;
    } else if (t >= keyv[0] && t < keyv[1]) {
      *frac = (t - keyv[0])/(keyv[1]-keyv[0]);
      return keyv - key->data;
    }
  }

  return nkeys-1;
}

static SFNode
vsgcNewNode (vsgWorld *curr, vsgClass const *cl, int size)
{
  vsgcNode *node = vsgAlloc (curr, size);

  node->n.cl = cl;
  node->n.routes = 0;
  node->flags = 0;
  node->aux = 0;

  if (vsgTracing)
    fprintf (stderr, "vsgcNewNode <= %s<%p>:%p\n", cl->name, cl, node);
  return (SFNode) node;
}


int
vsgcAnchor_set_addChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
			    MFNodeArg val)
{
  vsgcAnchor *n = (vsgcAnchor *) gn;
  SFNode *child = val->data, *end_child = child + val->mf.len;
  for ( ; child < end_child; ++child)
    n->children = vsgAppendSFNode (wrl, n->children, *child);
  return TRUE;
}

int
vsgcAnchor_set_removeChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
			       MFNodeArg val)
{
  vsgError (wrl, "Unimplemented eventIn Anchor.set_removeChildren\n");
  return TRUE;
}

SFTimeArg
vsgcAudioClip_duration_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcAudioClip *aclip = (vsgcAudioClip *) gn;
  vsgError (wrl, "Unimplemented eventOut AudioClip.duration_changed\n");
  return SFTime_default;
}

SFBoolArg
vsgcAudioClip_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcAudioClip *n = (vsgcAudioClip *) gn;
  return n->isActive;
}

int
vsgcBackground_set_bind (vsgWorld *wrl, SFNode gn, unsigned int sel,
			 SFBoolArg val)
{
  vsgError (wrl, "Unimplemented eventIn Background.set_bind\n");
  return TRUE;
}

SFBoolArg
vsgcBackground_isBound_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut Background.isBound_changed\n");
  return FALSE;
}

int
vsgcBillboard_set_addChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
			       MFNodeArg val)
{
  vsgcBillboard *n = (vsgcBillboard *) gn;
  SFNode *child = val->data, *end_child = child + val->mf.len;
  for ( ; child < end_child; ++child)
    n->children = vsgAppendSFNode (wrl, n->children, *child);
  return TRUE;
}

int
vsgcBillboard_set_removeChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
				  MFNodeArg val)
{
  vsgError (wrl, "Unimplemented eventIn Billboard.set_removeChildren\n");
  return TRUE;
}

int
vsgcCollision_set_addChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
			       MFNodeArg val)
{
  vsgcCollision *n = (vsgcCollision *) gn;
  SFNode *child = val->data, *end_child = child + val->mf.len;
  for ( ; child < end_child; ++child)
    n->children = vsgAppendSFNode (wrl, n->children, *child);
  return TRUE;
}

int
vsgcCollision_set_removeChildren (vsgWorld *wrl, SFNode gn, unsigned int sel,
				  MFNodeArg val)
{
  vsgError (wrl, "Unimplemented eventIn Collision.set_removeChildren\n");
  return TRUE;
}

SFTimeArg
vsgcCollision_collideTime_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcCollision *n = (vsgcCollision *) gn;
  vsgError (wrl, "Unimplemented eventOut Collision.collideTime_changed\n");
  return n->collideTime;
}

int
vsgcColorInterpolator_set_fraction (vsgWorld *wrl, SFNode gn, unsigned int sel,
				    SFFloatArg val)
{
  vsgcColorInterpolator *n = (vsgcColorInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

SFColorArg
vsgcColorInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgcColorInterpolator *n = (vsgcColorInterpolator *)gn;
  MFColor *keyval = n->keyValue;
  float frac;
  int idx = interpolate (n->key, n->fraction, &frac);
  SFColor *c = &keyval->data[idx];
  if (frac > 0.0)
    /* @@ This interpolation shoudl be done in HSV space. */
    return vsgMakeSFColor (LINTERP (frac, c[0].r, c[1].r),
			   LINTERP (frac, c[0].g, c[1].g),
			   LINTERP (frac, c[0].b, c[1].b));
  else
    return *c;
}

int
vsgcCoordinateInterpolator_set_fraction (vsgWorld *wrl, SFNode gn,
					 unsigned int sel, SFFloatArg val)
{
  vsgcCoordinateInterpolator *n = (vsgcCoordinateInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

MFVec3fArg
vsgcCoordinateInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
					  unsigned int sel)
{
  vsgcCoordinateInterpolator *n = (vsgcCoordinateInterpolator *)gn;
  MFFloat *key = n->key;
  MFVec3f *keyval = n->keyValue;
  float frac;
  int idx = interpolate (key, n->fraction, &frac);
  int m = keyval->mf.len / key->mf.len;
  MFVec3f *mf = vsgStartMFVec3f (wrl);
  SFVec3f *v = &keyval->data[m*idx];
  for (; m; --m, ++v) {
    SFVec3f v_i;
    if (frac > 0.0)
      v_i = vsgMakeSFVec3f (LINTERP (frac, v[0].v[0], v[1].v[0]),
			    LINTERP (frac, v[0].v[1], v[1].v[1]),
			    LINTERP (frac, v[0].v[2], v[1].v[2]));
    else
      v_i = *v;
    mf = vsgAppendSFVec3f (wrl, mf, v_i);
  }

  return vsgEndMFVec3f (wrl, mf);
}

SFBoolArg
vsgcCylinderSensor_isActive_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgcCylinderSensor *n = (vsgcCylinderSensor *) gn;
  return n->isActive;
}

SFRotationArg
vsgcCylinderSensor_rotation_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut CylinderSensor.rotation_changed\n");
  return SFRotation_default;
}

SFVec3fArg
vsgcCylinderSensor_trackPoint_changed (vsgWorld *wrl, SFNode gn,
				       unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut CylinderSensor.trackPoint_changed\n");
  return SFVec3f_default;
}

int
vsgcFog_set_bind (vsgWorld *wrl, SFNode gn, unsigned int sel, SFBoolArg val)
{
  vsgError (wrl, "Unimplemented eventIn Fog.set_bind\n");
  return TRUE;
}

SFBoolArg
vsgcFog_isBound_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcFog *n = (vsgcFog *) gn;
  return n->isBound;
}

int
vsgcGroup_set_addChildren (vsgWorld *wrl, SFNode gn,
			   unsigned int sel, MFNodeArg val)
{
  vsgcGroup *n = (vsgcGroup *) gn;
  SFNode *child = val->data, *end_child = child + val->mf.len;
  for ( ; child < end_child; ++child)
    n->children = vsgAppendSFNode (wrl, n->children, *child);
  return TRUE;
}

int
vsgcGroup_set_removeChildren (vsgWorld *wrl, SFNode gn,
			      unsigned int sel, MFNodeArg val)
{
  vsgError (wrl, "Unimplemented eventIn Group.set_removeChildren\n");
  return TRUE;
}

SFTimeArg
vsgcMovieTexture_duration_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut MovieTexture.duration_changed\n");
  return 0;
}

SFBoolArg
vsgcMovieTexture_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcMovieTexture *n = (vsgcMovieTexture *) gn;
  return n->isActive;
}

int
vsgcNavigationInfo_set_bind (vsgWorld *wrl, SFNode gn,
			     unsigned int sel, SFBoolArg val)
{
  vsgError (wrl, "Unimplemented eventIn NavigationInfo.set_bind\n");
  return TRUE;
}

SFBoolArg
vsgcNavigationInfo_isBound_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcNavigationInfo *n = (vsgcNavigationInfo *) gn;
  return n->isBound;
}

int
vsgcNormalInterpolator_set_fraction (vsgWorld *wrl, SFNode gn,
				     unsigned int sel, SFFloatArg val)
{
  vsgcNormalInterpolator *n = (vsgcNormalInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

MFVec3fArg
vsgcNormalInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
				      unsigned int sel)
{
  vsgcNormalInterpolator *n = (vsgcNormalInterpolator *)gn;
  MFFloat *key = n->key;
  MFVec3f *keyval = n->keyValue;
  float frac;
  int idx = interpolate (key, n->fraction, &frac);
  int m = keyval->mf.len / key->mf.len;
  MFVec3f *mf = vsgStartMFVec3f (wrl);
  SFVec3f *v = &keyval->data[m*idx];

  for (; m; --m, ++v) {
    SFVec3f v_i;
    if (frac > 0.0)
      v_i = vsgMakeSFVec3f (LINTERP (frac, v[0].v[0], v[1].v[0]),
			    LINTERP (frac, v[0].v[1], v[1].v[1]),
			    LINTERP (frac, v[0].v[2], v[1].v[2]));
    else
      v_i = *v;
    mf = vsgAppendSFVec3f (wrl, mf, v_i);
  }

  return vsgEndMFVec3f (wrl, mf);
}

int
vsgcOrientationInterpolator_set_fraction (vsgWorld *wrl, SFNode gn,
					  unsigned int sel, SFFloatArg val)
{
  vsgcOrientationInterpolator *n = (vsgcOrientationInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

MFRotationArg
vsgcOrientationInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
					   unsigned int sel)
{
  vsgcOrientationInterpolator *n = (vsgcOrientationInterpolator *)gn;
  MFFloat *key = n->key;
  MFRotation *keyval = n->keyValue;
  float frac;
  int idx = interpolate (key, n->fraction, &frac);
  int m = keyval->mf.len / key->mf.len;
  MFRotation *mf = vsgStartMFRotation (wrl);
  SFRotation *v = &keyval->data[m*idx];

  for (; m; --m, ++v) {
    SFRotation v_i;
    if (frac > 0.0)
      v_i = vsgMakeSFRotation (LINTERP (frac, v[0].a, v[1].a),
			       LINTERP (frac, v[0].b, v[1].b),
			       LINTERP (frac, v[0].c, v[1].c),
			       LINTERP (frac, v[0].d, v[1].d));
    else
      v_i = *v;
    mf = vsgAppendSFRotation (wrl, mf, v_i);
  }

  return vsgEndMFRotation (wrl, mf);
}

SFBoolArg
vsgcPlaneSensor_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcPlaneSensor *n = (vsgcPlaneSensor *) gn;
  return n->isActive;
}

SFVec3fArg
vsgcPlaneSensor_trackPoint_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut PlaneSensor.trackPoint_changed\n");
  return SFVec3f_default;
}

SFVec3fArg
vsgcPlaneSensor_translation_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut PlaneSensor.translation_changed\n");
  return SFVec3f_default;
}

int
vsgcPositionInterpolator_set_fraction (vsgWorld *wrl, SFNode gn,
				       unsigned int sel, SFFloatArg val)
{
  vsgcPositionInterpolator *n = (vsgcPositionInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

SFVec3fArg
vsgcPositionInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
					unsigned int sel)
{
  vsgcPositionInterpolator *n = (vsgcPositionInterpolator *)gn;
  MFVec3f *keyval = n->keyValue;
  float frac;
  int idx = interpolate (n->key, n->fraction, &frac);
  SFVec3f *v = &keyval->data[idx];
  if (frac > 0.0)
    return vsgMakeSFVec3f (LINTERP (frac, v[0].v[0], v[1].v[0]),
			   LINTERP (frac, v[0].v[1], v[1].v[1]),
			   LINTERP (frac, v[0].v[2], v[1].v[2]));
  else
    return *v;
}

SFBoolArg
vsgcProximitySensor_isActive_changed (vsgWorld *wrl, SFNode gn,
				      unsigned int sel)
{
  vsgcProximitySensor *n = (vsgcProximitySensor *) gn;
  return n->isActive;
}

SFVec3fArg
vsgcProximitySensor_position_changed (vsgWorld *wrl, SFNode gn,
				      unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut ProximitySensor.position_changed\n");
  return SFVec3f_default;
}

SFRotationArg
vsgcProximitySensor_orientation_changed (vsgWorld *wrl, SFNode gn,
					 unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut ProximitySensor.orientation_changed\n");
  return SFRotation_default;
}

SFTimeArg
vsgcProximitySensor_enterTime_changed (vsgWorld *wrl, SFNode gn,
				       unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut ProximitySensor.enterTime_changed\n");
  return SFTime_default;
}

SFTimeArg
vsgcProximitySensor_exitTime_changed (vsgWorld *wrl, SFNode gn,
				      unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut ProximitySensor.exitTime_changed\n");
  return SFTime_default;
}

int
vsgcScalarInterpolator_set_fraction (vsgWorld *wrl, SFNode gn,
				     unsigned int sel, SFFloatArg val)
{
  vsgcScalarInterpolator *n = (vsgcScalarInterpolator *)gn;
  n->fraction = val;
  return TRUE;
}

SFFloatArg
vsgcScalarInterpolator_value_changed (vsgWorld *wrl, SFNode gn,
				      unsigned int sel)
{
  vsgcScalarInterpolator *n = (vsgcScalarInterpolator *)gn;
  MFFloat *keyval = n->keyValue;
  float frac;
  int idx = interpolate (n->key, n->fraction, &frac);
  SFFloat *c = &keyval->data[idx];
  if (frac > 0.0)
    return LINTERP (frac, c[0], c[1]);
  else
    return *c;
}

SFBoolArg
vsgcSphereSensor_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcSphereSensor *n = (vsgcSphereSensor *) gn;
  return n->isActive;
}

SFRotationArg
vsgcSphereSensor_rotation_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut SphereSensor.rotation_changed\n");
  return SFRotation_default;
}

SFVec3fArg
vsgcSphereSensor_trackPoint_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut SphereSensor.trackPoint_changed\n");
  return SFVec3f_default;
}


SFTimeArg
vsgcTimeSensor_cycleTime_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TimeSensor.cycleTime_changed\n");
  return SFTime_default;
}

SFFloatArg
vsgcTimeSensor_fraction_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TimeSensor.fraction_changed\n");
  return SFFloat_default;
}

SFBoolArg
vsgcTimeSensor_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcTimeSensor *n = (vsgcTimeSensor *) gn;
  return n->isActive;
}

SFTimeArg
vsgcTimeSensor_time_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TimeSensor.time_changed\n");
  return SFTime_default;
}

SFVec3fArg
vsgcTouchSensor_hitNormal_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TouchSensor.hitNormal_changed\n");
  return SFVec3f_default;
}

SFVec3fArg
vsgcTouchSensor_hitPoint_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TouchSensor.hitPoint_changed\n");
  return SFVec3f_default;
}

SFVec2fArg
vsgcTouchSensor_hitTexCoord_changed (vsgWorld *wrl, SFNode gn,
				     unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TouchSensor.hitTexCoord_changed\n");
  return SFVec2f_default;
}

SFBoolArg
vsgcTouchSensor_isActive_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcTouchSensor *n = (vsgcTouchSensor *) gn;
  return n->isActive;
}

SFBoolArg
vsgcTouchSensor_isOver_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TouchSensor.isOver_changed\n");
  return FALSE;
}

SFTimeArg
vsgcTouchSensor_touchTime_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut TouchSensor.touchTime_changed\n");
  return SFTime_default;
}

int
vsgcTransform_set_addChildren (vsgWorld *wrl, SFNode gn,
			       unsigned int sel, MFNodeArg val)
{
  vsgcTransform *n = (vsgcTransform *) gn;
  SFNode *child = val->data, *end_child = child + val->mf.len;
  for ( ; child < end_child; ++child)
    n->children = vsgAppendSFNode (wrl, n->children, *child);
  return TRUE;
}

int
vsgcTransform_set_removeChildren (vsgWorld *wrl, SFNode gn,
				  unsigned int sel, MFNodeArg val)
{
  vsgError (wrl, "Unimplemented eventIn Transform.set_removeChildren\n");
  return TRUE;
}

int
vsgcViewpoint_set_bind (vsgWorld *wrl, SFNode gn,
			unsigned int sel, SFBoolArg val)
{
  vsgError (wrl, "Unimplemented eventIn Viewpoint.set_bind\n");
  return TRUE;
}

SFTimeArg
vsgcViewpoint_bindTime_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut Viewpoint.bindTime_changed\n");
  return SFTime_default;
}

SFBoolArg
vsgcViewpoint_isBound_changed (vsgWorld *wrl, SFNode gn, unsigned int sel)
{
  vsgcViewpoint *n = (vsgcViewpoint *) gn;
  return n->isBound;
}

SFTimeArg
vsgcVisibilitySensor_enterTime_changed (vsgWorld *wrl, SFNode gn,
					unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut VisibilitySensor.enterTime_changed\n");
  return SFTime_default;
}

SFTimeArg
vsgcVisibilitySensor_exitTime_changed (vsgWorld *wrl, SFNode gn,
				       unsigned int sel)
{
  vsgError (wrl, "Unimplemented eventOut VisibilitySensor.exitTime_changed\n");
  return SFTime_default;
}

SFBoolArg
vsgcVisibilitySensor_isActive_changed (vsgWorld *wrl, SFNode gn,
				       unsigned int sel)
{
  vsgcVisibilitySensor *n = (vsgcVisibilitySensor *) gn;
  return n->isActive;
}
