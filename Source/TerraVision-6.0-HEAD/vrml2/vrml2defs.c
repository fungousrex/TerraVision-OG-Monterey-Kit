/*
 * Name:  vrml2defs.c
 *
 * Library:
 *    This source file is part of the vrml2 library, a library for parsing
 *    and generating VRML 97 format files. This library is supplied as part
 *   of the tsmApi  distribution, available from:
 *
 *        http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: vrml2defs.c,v 1.26.2.1 2001/05/13 00:19:33 reddy Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "vrml2int.h"
#include "syms.h"

#define HAVE_STRING_H
#include <pthr/istack.h>

int vsgTracing = 0;

static plist_t globalProps = 0;
static char className[] = "class";

symbol_t
vsgIntern (vsgWorld *wrl, char const *str)
{
  struct vsgpId *id;
  if ((id = is_reserved_word (str, strlen (str))))
    return id->name;
  return sym_intern (wrl->symtab, str);
}

void
vsgBindClass (vsgWorld *wrl, vsgClass const *cl)
{
  plist_t props = wrl ? vsgiCurrProps (wrl) : globalProps;
  if (! props)
    props = globalProps = plist_create (512);
  plist_put (props, cl->name, className, (void *) cl);
}

vsgClass *
vsgGetClass (vsgWorld *wrl, symbol_t name)
{
  if (wrl && wrl->frame) {
    vsgClass *cl = plist_get (vsgiCurrProps (wrl), name, className);
    if (cl) return cl;
  }

  return globalProps ? plist_get (globalProps, name, className) : NULL;
}

SFNode
vsgNewNode (vsgWorld *wrl, symbol_t name)
{
  vsgClass *cl = vsgGetClass (wrl, name);
  return vsgCreateNode (wrl, cl);
}

vsgWorld *
vsgNewWorld (symtab_t symtab)
{
  int i;
  vsgWorld *curr = calloc (1, sizeof (*curr));

  for (i = 0; i < NUM_STACKS; ++i)
    curr->stacks[i] = istack_create (2048);
  curr->data = curr->stacks[0];
  curr->data_i = 0;
  curr->data_busy = 0;

  curr->externs = 0;

  curr->symtab_localp = (! symtab);
  curr->symtab = symtab ? symtab : symtab_create (0);

  curr->backgroundStack = 0;
  curr->fogStack = 0;
  curr->navigationInfoStack = 0;
  curr->viewpointStack = 0;

  curr->frame = 0;
  curr->lexer = 0;

  curr->filetype = "#VRML V2.0 utf8";

  return curr;
}

typedef struct vsgiExternRef {
  struct vsgiExternRef *next;
  void *p;
  void (*detach)(vsgWorld *, void*);
} vsgiExternRef;

void
vsgDestroyWorld (vsgWorld *curr)
{
  vsgiExternRef *ext = curr->externs;
  int i;

  /* Pop all accumulated frames. */
  while (curr->frame)
    vsgPopFrame (curr);

  /* Detach all external references. */
  for ( ; ext; ext = ext->next )
    (*ext->detach)(curr, ext->p);

  for (i = 0; i < NUM_STACKS; ++i)
    istack_destroy (curr->stacks[i]);

  if (curr->symtab_localp)
    symtab_destroy (curr->symtab);

  free (curr);
}

void
vsgRegisterExtern (vsgWorld *curr, void *p,
		   void (*detach)(vsgWorld *, void*))
{
  if (p && detach) {
    vsgiExternRef *ref = istack_new (curr->data, vsgiExternRef);
    
    ref->p = p;
    ref->detach = detach;
    ref->next = curr->externs;
    curr->externs = ref;
  }
}

char const *
vsgWorldFilename (vsgWorld *curr)
{
  return curr->filename;
}

char const *
vsgWorldFiletype (vsgWorld *curr)
{
  return curr->filetype;
}

MFNode *
vsgWorldNodes (vsgWorld *curr)
{
  return vsgiCurrNodes (curr);
}

plist_t
vsgWorldProperties (vsgWorld *curr)
{
  return vsgiCurrProps (curr);
}

MFProto *
vsgWorldProtos (vsgWorld *curr)
{
  return vsgiCurrProtos (curr);
}

MFNode *
vsgWorldAppend (vsgWorld *curr, SFNode node)
{
  return vsgAppendSFNode (curr, vsgiCurrNodes (curr), node);
}

/* Procedures on binding stacks. */
static int
vsgiIsBound (vsgiBindingStack *stack, SFNode node)
{
  return (stack && stack->node == node);
}

static void
vsgiBindStack (vsgWorld *curr, vsgiBindingStack **startp, SFNode node)
{
  vsgiBindingStack *stack = *startp;
  if (! vsgiIsBound (stack, node)) {
    vsgiBindingStack **stackp = startp;
    for ( ; stack; stackp = &stack->next, stack = *stackp)
      if (stack->node == node)
	break;
    if (stack) {
      /* Move found stack element to start. */
      *stackp = stack->next;
    } else {
      stack = vsgAlloc (curr, sizeof (vsgiBindingStack));
      stack->node = node;
    }
    stack->next = *startp;
    *startp = stack;
  }
}

static void
vsgiExtendStack (vsgWorld *curr, vsgiBindingStack **startp, SFNode node)
{
  vsgiBindingStack *stack = *startp;
  vsgiBindingStack **stackp = startp;
  for ( ; stack; stackp = &stack->next, stack = *stackp)
    if (stack->node == node)
      break;
  if (stack) {
    vsgError (curr, "Bindable node %p reused", node);
  } else {
    stack = vsgAlloc (curr, sizeof (vsgiBindingStack));
    stack->node = node;
    stack->next = 0;
    *stackp = stack;
  }
}

/* This is called to bind a Background node. */
void
vsgBindBackground (vsgWorld *curr, SFNode node)
{
  vsgiBindStack (curr, &curr->backgroundStack, node);
}

/* This should be called when a new Background node is created. */
void
vsgExtendBackgroundStack (vsgWorld *curr, SFNode node)
{
  vsgiExtendStack (curr, &curr->backgroundStack, node);
}

SFNode
vsgWorldBackground (vsgWorld *curr)
{
  vsgiBindingStack *stack = curr->backgroundStack;
  return (stack ? stack->node : NULL);
}

void
vsgBindFog (vsgWorld *curr, SFNode node)
{
  vsgiBindStack (curr, &curr->fogStack, node);
}

/* This should be called when a new Fog node is created. */
void
vsgExtendFogStack (vsgWorld *curr, SFNode node)
{
  vsgiExtendStack (curr, &curr->fogStack, node);
}

SFNode
vsgWorldFog (vsgWorld *curr)
{
  vsgiBindingStack *stack = curr->fogStack;
  return (stack ? stack->node : NULL);
}

void
vsgBindNavigationInfo (vsgWorld *curr, SFNode node)
{
  vsgiBindStack (curr, &curr->navigationInfoStack, node);
}

/* This should be called when a new NavigationInfo node is created. */
void
vsgExtendNavigationInfoStack (vsgWorld *curr, SFNode node)
{
  vsgiExtendStack (curr, &curr->navigationInfoStack, node);
}

SFNode
vsgWorldNavigationInfo (vsgWorld *curr)
{
  vsgiBindingStack *stack = curr->navigationInfoStack;
  return (stack ? stack->node : NULL);
}

void
vsgBindViewpoint (vsgWorld *curr, SFNode node)
{
  vsgiBindStack (curr, &curr->viewpointStack, node);
}

/* This should be called when a new Viewpoint node is created. */
void
vsgExtendViewpointStack (vsgWorld *curr, SFNode node)
{
  vsgiExtendStack (curr, &curr->viewpointStack, node);
}

SFNode
vsgWorldViewpoint (vsgWorld *curr)
{
  vsgiBindingStack *stack = curr->viewpointStack;
  return (stack ? stack->node : NULL);
}

static void
exFree (vsgWorld *wrl, void *p)
{
  free (p);
}

void *
vsgAlloc (vsgWorld *curr, int size)
{
#if 0
  if (istack_object_size (curr->data) != 0) 
    return NULL;

  return istack_alloc (curr->data, size);
#else
  void *p = malloc (size);
  vsgRegisterExtern (curr, p, exFree);
  return p;
#endif
}

void *
vsgCalloc (vsgWorld *curr, int size)
{
  void *n = vsgAlloc (curr, size);
  if (n) memset (n, 0, size);
  return n;
}

char *
vsgStrdup (vsgWorld *curr, char const *str)
{
  int len = strlen(str) + 1;
  char *new_str = vsgAlloc (curr, strlen(str) + 1);

  if (new_str) memcpy (new_str, str, len);
  return new_str;
}

vsgiFrame *
vsgPushFrame (vsgWorld *curr, vsgClass *proto)
{
  vsgiFrame *prev = curr->frame;
  vsgiFrame *frm = (vsgiFrame *) malloc (sizeof (*frm));

  curr->frame = frm;

  frm->prev = prev;
  frm->proto = (vsgProtoClass *) proto;
  frm->protos = NULL;
  frm->props = (prev ? prev->props : plist_create (0));
  frm->nodes = vsgMakeMFNode (curr);
  frm->routes = NULL;

  if (vsgTracing)
    fprintf (stderr, "vsgPushFrame: %p (prev %p)\n", frm, frm->prev);

  return frm;
}

void
vsgPopFrame (vsgWorld *curr)
{
  vsgiFrame *frm = curr->frame;
  vsgiFrame *prev = frm->prev;

  if (! frm) abort ();

  if (frm->props && (! prev || frm->props != prev->props))
    plist_destroy (frm->props);

  free (frm);

  frm = curr->frame = prev;

  if (vsgTracing)
    fprintf (stderr, "vsgPopFrame: %p (prev %p)\n", frm, frm ? frm->prev : 0);
}

vsgFunc_t
vsgOverride1 (vsgfId_t id, vsgFunc_t func)
{
  vsgFunc_t old_func = vsgFuncs[id];
  vsgFuncs[id] = func;
  return old_func;
}

vsgFunc_t
vsgDefault1 (vsgfId_t id, vsgFunc_t func)
{
  if (! vsgFuncs[id])
    vsgFuncs[id] = func;
  return vsgFuncs[id];
}

static vsgClass * vsgdStartProto (vsgWorld *curr, symbol_t id);
static void vsgdEndProtoFields (vsgWorld *curr);
static vsgClass const * vsgdEndProto (vsgWorld *curr);

static vsgClass * vsgdStartExternProto (vsgWorld *curr, symbol_t id);
static vsgClass const * vsgdEndExternProto (vsgWorld *curr);

static void vsgdDeclareField (vsgWorld *curr,
			      vsgFieldType ftype, vsgDataType dtype,
			      symbol_t fname);
static void vsgdVaDeclareFieldDefault (vsgWorld *curr, 
				       vsgDataType dtype, symbol_t fname,
				       va_list ap);
static void vsgdDeclareExtern (vsgWorld *curr, MFString *extern_urls);
static void vsgdDeclareNodes (vsgWorld *curr, MFNode *nodes);

static vsgClass * vsgdStartScript (vsgWorld *curr);
static vsgClass const * vsgdEndScript (vsgWorld *curr);

static void vsgdSetIS (vsgWorld *curr, SFNode node, symbol_t name, 
		       symbol_t is_name);
static vsgClassField *vsgdHasIS (vsgWorld *curr, plist_t props,
				 SFNode node, symbol_t name);

void
vsgdInit (void)
{
  static int init = 0;
  if (! init) {
#define VSG_CLASS(name) vsg##name##Class = vsgGetClass (0, vsgN##name)
    VSG_CLASS(Anchor);
    VSG_CLASS(Appearance);
    VSG_CLASS(AudioClip);
    VSG_CLASS(Background);
    VSG_CLASS(Billboard);
    VSG_CLASS(Box);
    VSG_CLASS(Collision);
    VSG_CLASS(Color);
    VSG_CLASS(ColorInterpolator);
    VSG_CLASS(Cone);
    VSG_CLASS(Coordinate);
    VSG_CLASS(CoordinateInterpolator);
    VSG_CLASS(Cylinder);
    VSG_CLASS(CylinderSensor);
    VSG_CLASS(DirectionalLight);
    VSG_CLASS(ElevationGrid);
    VSG_CLASS(Extrusion);
    VSG_CLASS(Fog);
    VSG_CLASS(FontStyle);
    VSG_CLASS(Group);
    VSG_CLASS(ImageTexture);
    VSG_CLASS(IndexedFaceSet);
    VSG_CLASS(IndexedLineSet);
    VSG_CLASS(Inline);
    VSG_CLASS(LOD);
    VSG_CLASS(Material);
    VSG_CLASS(MovieTexture);
    VSG_CLASS(NavigationInfo);
    VSG_CLASS(Normal);
    VSG_CLASS(NormalInterpolator);
    VSG_CLASS(OrientationInterpolator);
    VSG_CLASS(PixelTexture);
    VSG_CLASS(PlaneSensor);
    VSG_CLASS(PointLight);
    VSG_CLASS(PointSet);
    VSG_CLASS(PositionInterpolator);
    VSG_CLASS(ProximitySensor);
    VSG_CLASS(ScalarInterpolator);
    VSG_CLASS(Script);
    VSG_CLASS(Shape);
    VSG_CLASS(Sound);
    VSG_CLASS(Sphere);
    VSG_CLASS(SphereSensor);
    VSG_CLASS(SpotLight);
    VSG_CLASS(Switch);
    VSG_CLASS(Text);
    VSG_CLASS(TextureCoordinate);
    VSG_CLASS(TextureTransform);
    VSG_CLASS(TimeSensor);
    VSG_CLASS(TouchSensor);
    VSG_CLASS(Transform);
    VSG_CLASS(Viewpoint);
    VSG_CLASS(VisibilitySensor);
    VSG_CLASS(WorldInfo);

    vsgSetupDefaults ();
    init = 1;
  }
}

void
vsgSetupDefaults (void)
{
  vsgDefault (vsgfStartProto,            vsgdStartProto);
  vsgDefault (vsgfEndProtoFields,        vsgdEndProtoFields);
  vsgDefault (vsgfEndProto,              vsgdEndProto);

  vsgDefault (vsgfStartExternProto,      vsgdStartExternProto);
  vsgDefault (vsgfEndExternProto,        vsgdEndExternProto);

  vsgDefault (vsgfDeclareField,          vsgdDeclareField);
  vsgDefault (vsgfVaDeclareFieldDefault, vsgdVaDeclareFieldDefault);
  vsgDefault (vsgfDeclareExtern,         vsgdDeclareExtern);
  vsgDefault (vsgfDeclareNodes,          vsgdDeclareNodes);

  vsgDefault (vsgfStartScript,           vsgdStartScript);
  vsgDefault (vsgfEndScript,             vsgdEndScript);

  vsgDefault (vsgfSetIS,                 vsgdSetIS);
  vsgDefault (vsgfHasIS,                 vsgdHasIS);

  vsgDefault (vsgfPrint,                 vsgdPrint);
}

static int
vsgVaDescSet1 (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, va_list *ap)
{
  switch (desc.type) {
  case vsgMFColor:
    return vsgSetMFColor (wrl, node, desc, va_arg (*ap, MFColor *));
  case vsgMFFloat:
    return vsgSetMFFloat (wrl, node, desc, va_arg (*ap, MFFloat *));
  case vsgMFInt32:
    return vsgSetMFInt32 (wrl, node, desc, va_arg (*ap, MFInt32 *));
  case vsgMFNode:
    return vsgSetMFNode (wrl, node, desc, va_arg (*ap, MFNode *));
  case vsgMFRotation:
    return vsgSetMFRotation (wrl, node, desc, va_arg (*ap, MFRotation *));
  case vsgMFString:
    return vsgSetMFString (wrl, node, desc, va_arg (*ap, MFString *));
  case vsgMFVec2f:
    return vsgSetMFVec2f (wrl, node, desc, va_arg (*ap, MFVec2f *));
  case vsgMFVec3f:
    return vsgSetMFVec3f (wrl, node, desc, va_arg (*ap, MFVec3f *));
  case vsgSFBool:
    return vsgSetSFBool (wrl, node, desc, va_arg (*ap, SFBool));
  case vsgSFColor:
    return vsgSetSFColor (wrl, node, desc, va_arg (*ap, SFColor));
  case vsgSFFloat:
    return vsgSetSFFloat (wrl, node, desc, va_arg (*ap, double));
  case vsgSFImage:
    return vsgSetSFImage (wrl, node, desc, va_arg (*ap, SFImage));
  case vsgSFInt32:
    return vsgSetSFInt32 (wrl, node, desc, va_arg (*ap, SFInt32));
  case vsgSFNode:
    return vsgSetSFNode (wrl, node, desc, va_arg (*ap, SFNode));
  case vsgSFRotation:
    return vsgSetSFRotation (wrl, node, desc, va_arg (*ap, SFRotation));
  case vsgSFString:
    return vsgSetSFString (wrl, node, desc, va_arg (*ap, SFString));
  case vsgSFTime:
    return vsgSetSFTime (wrl, node, desc, va_arg (*ap, double));
  case vsgSFVec2f:
    return vsgSetSFVec2f (wrl, node, desc, va_arg (*ap, SFVec2f));
  case vsgSFVec3f:
    return vsgSetSFVec3f (wrl, node, desc, va_arg (*ap, SFVec3f));
  default:
    return 0;
  }
}

int
vsgVaDescSet (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, va_list ap)
{
  return vsgVaDescSet1 (wrl, node, desc, &ap);
}

int
vsgDescSet (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, ...)
{
  int ret;
  va_list ap;
  va_start (ap, desc);
  ret = vsgVaDescSet (wrl, node, desc, ap);
  va_end (ap);
  return ret;
}

int
vsgSet (vsgWorld *wrl, SFNode node, symbol_t fieldName, ...)
{
  vsgEventInDesc evin = vsgSetFunc (node, fieldName);
  if (evin.type == vsgErrorType) return 0;

  {
    int ret;
    va_list ap;
    va_start (ap, fieldName);
    ret = vsgVaDescSet (wrl, node, evin, ap);
    va_end (ap);
    return ret;
  }
}

int
vsgSetv (vsgWorld *wrl, SFNode node, ...)
{
  vsgClass const *cl = vsgNodeClass (node);
  symbol_t fieldName;

  va_list ap;
  va_start (ap, node);

  while ((fieldName = va_arg (ap, symbol_t))) {
    vsgEventInDesc evin = vsgiGet_setFunc (cl, fieldName);
    if (evin.type == vsgErrorType) return 0;

    vsgVaDescSet1 (wrl, node, evin, &ap);
  }

  va_end (ap);
  return 1;
}

void
vsgDescGet (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc, vsgAnyArg *val)
{
  switch (desc.type) {
  case vsgMFColor:
    val->MFColor = vsgGetMFColor (wrl, node, desc);
    break;
  case vsgMFFloat:
    val->MFFloat = vsgGetMFFloat (wrl, node, desc);
    break;
  case vsgMFInt32:
    val->MFInt32 = vsgGetMFInt32 (wrl, node, desc);
    break;
  case vsgMFNode:
    val->MFNode = vsgGetMFNode (wrl, node, desc);
    break;
  case vsgMFRotation:
    val->MFRotation = vsgGetMFRotation (wrl, node, desc);
    break;
  case vsgMFString:
    val->MFString = vsgGetMFString (wrl, node, desc);
    break;
  case vsgMFVec2f:
    val->MFVec2f = vsgGetMFVec2f (wrl, node, desc);
    break;
  case vsgMFVec3f:
    val->MFVec3f = vsgGetMFVec3f (wrl, node, desc);
    break;
  case vsgSFBool:
    val->SFBool = vsgGetSFBool (wrl, node, desc);
    break;
  case vsgSFColor:
    val->SFColor = vsgGetSFColor (wrl, node, desc);
    break;
  case vsgSFFloat:
    val->SFFloat = vsgGetSFFloat (wrl, node, desc);
    break;
  case vsgSFImage:
    val->SFImage = vsgGetSFImage (wrl, node, desc);
    break;
  case vsgSFInt32:
    val->SFInt32 = vsgGetSFInt32 (wrl, node, desc);
    break;
  case vsgSFNode:
    val->SFNode = vsgGetSFNode (wrl, node, desc);
    break;
  case vsgSFRotation:
    val->SFRotation = vsgGetSFRotation (wrl, node, desc);
    break;
  case vsgSFString:
    val->SFString = vsgGetSFString (wrl, node, desc);
    break;
  case vsgSFTime:
    val->SFTime = vsgGetSFTime (wrl, node, desc);
    break;
  case vsgSFVec2f:
    val->SFVec2f = vsgGetSFVec2f (wrl, node, desc);
    break;
  case vsgSFVec3f:
    val->SFVec3f = vsgGetSFVec3f (wrl, node, desc);
    break;
  default:
    /* vsgError */
    break;
  }
}

int
vsgGet (vsgWorld *wrl, SFNode node, symbol_t fieldName, vsgAnyArg *val)
{
  vsgEventOutDesc evout = vsgGetFunc (node, fieldName);
  if (evout.type == vsgErrorType) return 0;
  vsgDescGet (wrl, node, evout, val);
  return 1;
}

SFImage *
vsgStartSFImage (vsgWorld *curr, int w, int h, int depth)
{
  SFImage *im;
  im = (SFImage *) istack_alloc (curr->data, sizeof(*im));
  im->data = (unsigned char *) curr->data;

  if (! vsgiClaimStack (curr)) {
    vsgError (curr, "Cannot start SFImage.  No available istack.");
    return 0;
  }

  im->width = w;
  im->height = h;
  im->depth = depth;

  return im;
}

void
vsgiStartSFImage (vsgWorld *curr,
		  int w, int h, int nplanes, int v1, int v2)
{
  SFImage *im = vsgStartSFImage (curr, w, h, nplanes);
  vsgAppendPixel (curr, im, v1);
  vsgAppendPixel (curr, im, v2);
}

SFImage *
vsgEndSFImage (vsgWorld *curr, SFImage *im)
{
  istack *ist = (istack *) im->data;
  int len = istack_object_size (ist);

  im->data = (unsigned char *) istack_finish (ist);

  if (len < im->width*im->height*im->depth) {
    vsgError (curr, "incomplete image [%d provided, %d expected]",
		len/im->depth, im->width*im->height);
  }

  vsgiReleaseStack (curr, ist);

  return im;
}

void
vsgAppendPixel (vsgWorld *curr, SFImage *im, int val)
{
  istack *ist = (istack *) im->data;

  switch (im->depth) {
  case 4:
    istack_1grow (ist, (val >> 24) & 0xff);
  case 3:
    istack_1grow (ist, (val >> 16) & 0xff);
  case 2:
    istack_1grow (ist, (val >>  8) & 0xff);
  case 1:
    istack_1grow (ist, (val >>  0) & 0xff);
  }
}

int
vsgiCheckStack (vsgWorld *curr)
{
  return ! (curr->data_busy & (1<<curr->data_i));
}

int
vsgiClaimStack (vsgWorld *curr)
{
  int i = curr->data_i;
  int end;
  if (! vsgiCheckStack (curr))
    return 0;

  curr->data_busy |= (1<<i);
  end = i;
  i = (i+1)%NUM_STACKS;
  for ( ; i != end; i = (i+1)%NUM_STACKS) {
    if (! (curr->data_busy & (1<<i))) {
      curr->data = curr->stacks[i];
      curr->data_i = i;
      break;
    }
  }
  return 1;
}

void
vsgiReleaseStack (vsgWorld *curr, istack *ist)
{
  int i;
  for (i = 0; i < NUM_STACKS; ++i)
    if (ist == curr->stacks[i])
      break;
  curr->data_busy &= ~(1<<i);
}

MFGeneric *
vsgiStartMFGeneric (vsgWorld *curr, vsgDataType type, int elsize)
{
  MFGenericI *mfgeni;
  MFGeneric *mfgen;
  istack *ist = curr->data;

  if (! vsgiClaimStack (curr)) {
    vsgError (curr, "Cannot start %s.  No available istack.",
		vsgDataTypeName[type]);
    return 0;
  }

  mfgeni = (MFGenericI *) istack_alloc (ist, sizeof(*mfgeni));
  mfgen = &mfgeni->gen;

  mfgen->mf.type = type;
  mfgen->mf.flags = MFGEN_ISTACK|MFGEN_PARTIAL;
  mfgen->mf.len = 0;
  mfgen->data = 0;
  mfgeni->u.stack = ist;

  if (vsgTracing >= 2)
    fprintf (stderr, "vsgStart%s <= %p\n",
	     vsgDataTypeName[type], mfgen);

  return mfgen;
}

MFGeneric *
vsgiEndMFGeneric (vsgWorld *curr, MFGeneric *mfgen)
{
  MFGenericI *mfgeni = (MFGenericI *) mfgen;
  istack *ist = mfgeni->u.stack;

  if (vsgTracing >= 2)
    fprintf (stderr, "vsgEnd%s: %p len %d\n", 
	     vsgDataTypeName[mfgen->mf.type], mfgen, mfgen->mf.len);

  mfgen->mf.flags &= ~MFGEN_PARTIAL;
  mfgen->data = istack_finish (ist);

  vsgiReleaseStack (curr, ist);

  return mfgen;
}

static void
vsgiFreeMFGeneric (vsgWorld *curr, void *p)
{
  MFGeneric *mfgen = (MFGeneric *) p;
  free (mfgen->data);
}

static void *
vsgiAppendLocMalloc (vsgWorld *curr, MFGeneric **mfgenp, int elsize)
{
  MFGeneric *mfgen = *mfgenp;
  MFGenericI *mfgeni = (MFGenericI *) mfgen;
  int is_static = 0;

  /* Convert to malloc if is on istack or if it is static. */
  if (mfgen->mf.flags & MFGEN_ISTACK
      || (is_static = ! (mfgen->mf.flags&(MFGEN_ISTACK|MFGEN_MALLOC)))) {
    int new_len = mfgen->mf.len < 13 ? 16 : (mfgen->mf.len * 17)/12;
    void *new_area = malloc (new_len * elsize);
    if (is_static) {
      /* In this case, we allocate a new header and substitute it. */
      mfgeni = (MFGenericI *) istack_alloc (curr->data, sizeof (MFGenericI));
      mfgeni->gen = *mfgen;
      *mfgenp = &mfgeni->gen;
    }
    memcpy (new_area, mfgen->data, mfgen->mf.len * elsize);
    mfgeni->u.size = new_len;
    mfgen->data = new_area;
    mfgen->mf.flags = MFGEN_MALLOC;

    vsgRegisterExtern (curr, mfgen, vsgiFreeMFGeneric);
  }

  /* We know that we're malloc-based.  Check for need to extend. */
  if (mfgen->mf.len >= mfgeni->u.size) {
    int new_size = (mfgeni->u.size * 17)/12;
    mfgeni->u.size = new_size;
    mfgen->data = realloc (mfgen->data, new_size * elsize);
  }

  return ((char *) mfgen->data) + (mfgen->mf.len++ * elsize);
}

static void *
vsgiAppendLoc (vsgWorld *curr, MFGeneric **mfgenp,
	       vsgDataType type, int elsize)
{
  MFGeneric *mfgen = *mfgenp;
  MFGenericI *mfgeni = (MFGenericI *) mfgen;
  if (mfgen->mf.type != type)
    return 0;

  if (mfgen->mf.flags & MFGEN_PARTIAL) {
    istack_blank (mfgeni->u.stack, elsize);
    ++mfgen->mf.len;
    return ((char *) istack_next_free (mfgeni->u.stack)) - elsize;
  } else
    return vsgiAppendLocMalloc (curr, mfgenp, elsize);
}

SFString SFString_default = "";
SFFloat SFFloat_default = 0;
SFInt32 SFInt32_default = 0;
SFBool SFBool_default = /*FALSE*/ -1; /* SFBool default can be TRUE or FALSE */
SFTime SFTime_default = 0;
SFColor SFColor_default = {0, 0, 0};
SFVec2f SFVec2f_default = {{0, 0}};
SFVec3f SFVec3f_default = {{0, 0, 0}};
SFRotation SFRotation_default = {0, 0, 1, 0};
SFNode SFNode_default = NULL;
SFImage SFImage_default = { 0, 0, 0, 0 };

MFFloat MFFloat_default_s = {{vsgMFFloat, 0, 0}, 0};
MFFloat *MFFloat_default = &MFFloat_default_s;
MFRotation MFRotation_default_s = {{vsgMFRotation, 0, 0}, 0};
MFRotation *MFRotation_default = &MFRotation_default_s;
MFVec2f MFVec2f_default_s = {{vsgMFVec2f, 0, 0}, 0};
MFVec2f *MFVec2f_default = &MFVec2f_default_s;
MFVec3f MFVec3f_default_s = {{vsgMFVec3f, 0, 0}, 0};
MFVec3f *MFVec3f_default = &MFVec3f_default_s;
MFInt32 MFInt32_default_s = {{vsgMFInt32, 0, 0}, 0};
MFInt32 *MFInt32_default = &MFInt32_default_s;
MFString MFString_default_s = {{vsgMFString, 0, 0}, 0};
MFString *MFString_default = &MFString_default_s;
MFNode MFNode_default_s = {{vsgMFNode, 0, 0}, 0};
MFNode *MFNode_default = &MFNode_default_s;
MFColor MFColor_default_s = {{vsgMFColor, 0, 0}, 0};
MFColor *MFColor_default = &MFColor_default_s;

void
vsgDefaultAnyArg (vsgDataType type, vsgAnyArg *a)
{
  switch (type) {
  case vsgMFColor:
    a->MFColor = MFColor_default;
    break;
  case vsgMFFloat:
    a->MFFloat = MFFloat_default;
    break;
  case vsgMFInt32:
    a->MFInt32 = MFInt32_default;
    break;
  case vsgMFNode:
    a->MFNode = MFNode_default;
    break;
  case vsgMFRotation:
    a->MFRotation = MFRotation_default;
    break;
  case vsgMFString:
    a->MFString = MFString_default;
    break;
  case vsgMFVec2f:
    a->MFVec2f = MFVec2f_default;
    break;
  case vsgMFVec3f:
    a->MFVec3f = MFVec3f_default;
    break;
  case vsgSFBool:
    a->SFBool = SFBool_default;
    break;
  case vsgSFColor:
    a->SFColor = SFColor_default;
    break;
  case vsgSFFloat:
    a->SFFloat = SFFloat_default;
    break;
  case vsgSFImage:
    a->SFImage = SFImage_default;
    break;
  case vsgSFInt32:
    a->SFInt32 = SFInt32_default;
    break;
  case vsgSFNode:
    a->SFNode = SFNode_default;
    break;
  case vsgSFRotation:
    a->SFRotation = SFRotation_default;
    break;
  case vsgSFString:
    a->SFString = SFString_default;
    break;
  case vsgSFTime:
    a->SFTime = SFTime_default;
    break;
  case vsgSFVec2f:
    a->SFVec2f = SFVec2f_default;
    break;
  case vsgSFVec3f:
    a->SFVec3f = SFVec3f_default;
    break;
  }
}

void
vsgPrintAnyArg (FILE *f, vsgDataType type, vsgAnyArg *a)
{
  switch (type) {
  case vsgMFColor:
  case vsgMFFloat: 
  case vsgMFInt32:
  case vsgMFNode:
  case vsgMFRotation:
  case vsgMFString:
  case vsgMFVec2f:
  case vsgMFVec3f:
  case vsgMFProto:
    fprintf (f, "%p[%d]", a->MFGeneric, a->MFGeneric->mf.len);
    break;
  case vsgSFBool:
    fprintf (f, "%s", a->SFBool ? "TRUE" : "FALSE");
    break;
  case vsgSFColor:
    fprintf (f, "%g %g %g", a->SFColor.r, a->SFColor.g, a->SFColor.b);
    break;
  case vsgSFFloat:
    fprintf (f, "%g", a->SFFloat);
    break;
  case vsgSFImage: {
    SFImage im = a->SFImage;
    fprintf (f, "%d %d %d %p", im.width, im.height, im.depth, im.data);
    break;
  }
  case vsgSFInt32:
    fprintf (f, "%ld", (long) a->SFInt32);
    break;
  case vsgSFNode: 
    if (a->SFNode) {
      vsgClass const *cl = vsgNodeClass (a->SFNode);
      fprintf (f, "%s:%p", cl->name, a->SFNode);
    } else {
      fputs ("NULL", f);
    }
    break;
  case vsgSFRotation:
    fprintf (f, "%g %g %g %g", 
	     a->SFRotation.a, a->SFRotation.b,
	     a->SFRotation.c, a->SFRotation.d);
    break;
  case vsgSFString:
    fprintf (f, "\"%s\"", a->SFString);
    break;
  case vsgSFTime:
    fprintf (f, "%g", a->SFTime);
    break;
  case vsgSFVec2f:
    fprintf (f, "%g %g", a->SFVec2f.v[0], a->SFVec2f.v[1]);
    break;
  case vsgSFVec3f:
    fprintf (f, "%g %g %g", a->SFVec3f.v[0], a->SFVec3f.v[1], a->SFVec3f.v[2]);
    break;
  case vsgSFProto:
    fprintf (f, "cl:%s", a->SFProto->name);
    break;
  default:
    if (type >= vsgErrorType && type <= vsgSFProto)
      fprintf (f, "<%s>", vsgDataTypeName[type]);
    else
      fprintf (f, "<#%d>", type);
    break;
  }
}

#define DEFINE_AGGREGATION(aggr,indiv)				\
aggr *								\
vsgStart##aggr (vsgWorld *curr)					\
{								\
  return (aggr *)						\
    vsgiStartMFGeneric (curr, vsg##aggr, sizeof (indiv));	\
}								\
								\
aggr *								\
vsgEnd##aggr (vsgWorld *curr, aggr *mf)				\
{								\
  return (aggr *) vsgiEndMFGeneric (curr, (MFGeneric *) mf);	\
}								\
								\
aggr *								\
vsgAppend##indiv (vsgWorld *curr, aggr *mf, indiv c)		\
{								\
  indiv *p = vsgiAppendLoc (curr, (MFGeneric **) &mf,		\
			    vsg##aggr, sizeof (indiv));		\
  if (vsgTracing >= 2) {					\
    fprintf (stderr, "vsgAppend" #aggr ": %p[%d] = ",	 	\
	     mf, mf->mf.len-1);					\
    vsgPrintAnyArg (stderr, vsg##indiv, (vsgAnyArg*) &c);	\
    putc ('\n', stderr);					\
  }								\
  *p = c;							\
  return mf;							\
}								\
								\
aggr *								\
vsgMake##aggr(vsgWorld *curr) {					\
  aggr *mf = vsgStart##aggr (curr);				\
  return vsgEnd##aggr (curr, mf);				\
}								\
								\
aggr *								\
vsgCopy##aggr(vsgWorld *curr, aggr *mf0)			\
{								\
  int i;							\
  aggr *mf = vsgStart##aggr (curr);				\
  for (i = 0; i < mf0->mf.len; ++i)				\
    vsgAppend##indiv (curr, mf, mf0->data[i]);			\
  return vsgEnd##aggr (curr, mf);				\
}

DEFINE_AGGREGATION (MFColor, SFColor)
DEFINE_AGGREGATION (MFFloat, SFFloat)
DEFINE_AGGREGATION (MFRotation, SFRotation)
DEFINE_AGGREGATION (MFVec2f, SFVec2f)
DEFINE_AGGREGATION (MFVec3f, SFVec3f)
DEFINE_AGGREGATION (MFInt32, SFInt32)
DEFINE_AGGREGATION (MFString, SFString)

MFNode *
vsgStartMFNode (vsgWorld *curr)
{
  MFGenericI *mfgeni;
  MFNode *mf;

  mfgeni = (MFGenericI *) istack_alloc (curr->data, sizeof(*mfgeni));
  mf = (MFNode *) &mfgeni->gen;

  mf->mf.type = vsgMFNode;
  mf->mf.flags = MFGEN_MALLOC;
  mf->mf.len = 0;
  mf->data = malloc (16 * sizeof (SFNode));
  mfgeni->u.size = 16;

  vsgRegisterExtern (curr, mf, vsgiFreeMFGeneric);

  if (vsgTracing >= 2)
    fprintf (stderr, "vsgStartMFNode: %p\n", mf->data);

  return mf;
}

MFNode *
vsgEndMFNode (vsgWorld *curr, MFNode *mf)
{
  if (vsgTracing >= 2)
    fprintf (stderr, "vsgEndMFNode: %p\n", mf->data);
  return mf;
}

MFNode *
vsgAppendSFNode (vsgWorld *curr, MFNode *mf, SFNode c)
{
  SFNode *p = vsgiAppendLoc (curr, (MFGeneric **) &mf,
			     vsgMFNode, sizeof (SFNode));
  *p = c;
  return mf;
}

MFNode *
vsgMakeMFNode(vsgWorld *curr)
{
  MFNode *mf = vsgStartMFNode (curr);
  return vsgEndMFNode (curr, mf);
}

MFNode *
vsgCopyMFNode(vsgWorld *curr, MFNode *mf0)
{
  int i;
  MFNode *mf = vsgStartMFNode (curr);
  for (i = 0; i < mf0->mf.len; ++i)
    vsgAppendSFNode (curr, mf, mf0->data[i]);
  return vsgEndMFNode (curr, mf);
}

MFProto *
vsgStartMFProto (vsgWorld *curr)
{
  MFGenericI *mfgeni;
  MFProto *mf;

  mfgeni = (MFGenericI *) istack_alloc (curr->data, sizeof(*mfgeni));
  mf = (MFProto *) &mfgeni->gen;

  mf->mf.type = vsgMFProto;
  mf->mf.flags = MFGEN_MALLOC;
  mf->mf.len = 0;
  mf->data = malloc (16 * sizeof (vsgProtoClass *));
  mfgeni->u.size = 16;

  vsgRegisterExtern (curr, mf, vsgiFreeMFGeneric);

  if (vsgTracing >= 2)
    fprintf (stderr, "vsgStartMFProto: %p\n", mf->data);

  return mf;
}

MFProto *
vsgEndMFProto (vsgWorld *curr, MFProto *mf)
{
  if (vsgTracing >= 2)
    fprintf (stderr, "vsgEndMFProto: %p\n", mf->data);
  return mf;
}

MFProto *
vsgAppendSFProto (vsgWorld *curr, MFProto *mf, vsgClass *c)
{
  vsgClass **p = vsgiAppendLoc (curr, (MFGeneric **) &mf,
				vsgMFProto, sizeof (vsgClass *));
  *p = c;
  return mf;
}

MFProto *
vsgMakeMFProto(vsgWorld *curr)
{
  MFProto *mf = vsgStartMFProto (curr);
  return vsgEndMFProto (curr, mf);
}

symbol_t
vsgHasDEF (plist_t props, SFNode val)
{
  return (symbol_t) plist_get (props, val, vsgNDEF);
}

void
vsgDEF (vsgWorld *curr, symbol_t name, SFNode val)
{
  plist_put (curr->frame->props, name, vsgNDEF, val);
  plist_put (curr->frame->props, val, vsgNDEF, (char *) name);
  if (vsgTracing)
    fprintf (stderr, "vsgDEF(%s) <= %p\n", name, val);
}

SFNode
vsgUSE (vsgWorld *curr, symbol_t name)
{
  SFNode val = plist_get (curr->frame->props, name, vsgNDEF);
  if (vsgTracing)
    fprintf (stderr, "vsgUSE(%s) => %p\n", name, val);
  return val;
}

static symbol_t
vsgEventInName (vsgWorld *wrl, symbol_t name)
{
  /* Strip set_ prefix if necessary. */
  if (! strncmp (name, "set_", 4))
    name = vsgIntern (wrl, name+4);
  return name;
}

vsgEventInDesc 
vsgEventIn (vsgWorld *wrl, SFNode node, symbol_t name)
{
  vsgClass const *cl = vsgNodeClass (node);
  return vsgiGet_eventIn (cl, vsgEventInName (wrl, name));
}

static symbol_t
vsgEventOutName (vsgWorld *wrl, symbol_t name)
{
  int len = strlen (name);
  /* Strip _changed suffix if necessary. */
  if (len > 8 && ! strncmp (name+len-8, "_changed", 8)) {
    char *tmp = alloca (len-7);
    tmp[len-8] = '\0';
    memcpy (tmp, name, len-8);
    name = vsgIntern (wrl, tmp);
  }
  return name;
}
vsgEventOutDesc 
vsgEventOut (vsgWorld *wrl, SFNode node, symbol_t name)
{
  vsgClass const *cl = vsgNodeClass (node);
  return vsgiGet_eventOut (cl, vsgEventOutName (wrl, name));
}

vsgRoute *
vsgAddRoute (vsgWorld *wrl,
	     SFNode from, symbol_t fromField,
	     SFNode to, symbol_t toField)
{
  vsgEventOutDesc evout = vsgEventOut (wrl, from, fromField);
  vsgEventInDesc evin = vsgEventIn (wrl, to, toField);

  if (evout.type == vsgErrorType)
    vsgError (wrl, "eventOut %s.%s not found",
	      vsgNodeClass (from)->name, fromField);
  if (evin.type == vsgErrorType)
    vsgError (wrl, "eventIn %s.%s not found",
	      vsgNodeClass (to)->name, toField);
  if (evout.type == vsgErrorType || evin.type == vsgErrorType)
    return 0;

  {
    vsgRoute *route = vsgAlloc (wrl, sizeof (*route));
    vsgRouteLink *link_from = vsgAlloc (wrl, sizeof (*link_from));
    vsgRouteLink *link = vsgAlloc (wrl, sizeof (*link));
    vsgiFrame *frm = wrl->frame;

    /* Initialize route. */
    route->from = from;
    route->fromField = fromField;
    route->fromDesc = evout;
    route->to = to;
    route->toField = toField;
    route->toDesc = evin;

    /* Add route to from node. */
    link_from->route = route;
    link_from->next = from->routes;
    from->routes = link_from;

    /* Add route to frame. */
    link->route = route;
    link->next = frm->routes;
    frm->routes = link;

    return route;
  }
}


static SFNode 
vsgProto_create (vsgClass const *cl, vsgWorld *wrl)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgProtoNode *node = vsgAlloc (wrl, sizeof (*node));
  int nfields = pclass->nfields;
  int i;

  node->pclass = pclass;
  node->nodes = NULL;
  node->bindings = (vsgAnyArg *)
    istack_alloc (wrl->data, nfields * sizeof (node->bindings[0]));

  for (i = 0; i < nfields; ++i)
    node->bindings[i] = pclass->fields[i]->c.dflt;

  return (SFNode) node;
}

#define DEFINE_TYPEFUNCS(type)						\
static int								\
vsgProto_set##type (vsgWorld *wrl, SFNode gnode, unsigned int idx,	\
		    type##Arg arg)					\
{									\
  vsgProtoNode *node = (vsgProtoNode *) gnode;				\
  node->bindings[idx].type = arg;					\
  return vsg##type >= vsgSFBool;					\
}									\
									\
static type##Arg							\
vsgProto_get##type (vsgWorld *wrl, SFNode gnode, unsigned int idx)	\
{									\
  vsgProtoNode *node = (vsgProtoNode *) gnode;				\
  return node->bindings[idx].type;					\
}

DEFINE_TYPEFUNCS(MFColor)
DEFINE_TYPEFUNCS(MFFloat)
DEFINE_TYPEFUNCS(MFInt32)
DEFINE_TYPEFUNCS(MFNode)
DEFINE_TYPEFUNCS(MFRotation)
DEFINE_TYPEFUNCS(MFString)
DEFINE_TYPEFUNCS(MFVec2f)
DEFINE_TYPEFUNCS(MFVec3f)
DEFINE_TYPEFUNCS(SFBool)
DEFINE_TYPEFUNCS(SFColor)
DEFINE_TYPEFUNCS(SFFloat)
DEFINE_TYPEFUNCS(SFImage)
DEFINE_TYPEFUNCS(SFInt32)
DEFINE_TYPEFUNCS(SFNode)
DEFINE_TYPEFUNCS(SFRotation)
DEFINE_TYPEFUNCS(SFString)
DEFINE_TYPEFUNCS(SFTime)
DEFINE_TYPEFUNCS(SFVec2f)
DEFINE_TYPEFUNCS(SFVec3f)

typedef int (*setFunc_t) (vsgWorld *, SFNode, unsigned int, ...);
static setFunc_t vsgProto_setField[] = {
  0,
  (setFunc_t) vsgProto_setMFColor,
  (setFunc_t) vsgProto_setMFFloat,
  (setFunc_t) vsgProto_setMFInt32,
  (setFunc_t) vsgProto_setMFNode,
  (setFunc_t) vsgProto_setMFRotation,
  (setFunc_t) vsgProto_setMFString,
  (setFunc_t) vsgProto_setMFVec2f,
  (setFunc_t) vsgProto_setMFVec3f,
  0,
  (setFunc_t) vsgProto_setSFBool,
  (setFunc_t) vsgProto_setSFColor,
  (setFunc_t) vsgProto_setSFFloat,
  (setFunc_t) vsgProto_setSFImage,
  (setFunc_t) vsgProto_setSFInt32,
  (setFunc_t) vsgProto_setSFNode,
  (setFunc_t) vsgProto_setSFRotation,
  (setFunc_t) vsgProto_setSFString,
  (setFunc_t) vsgProto_setSFTime,
  (setFunc_t) vsgProto_setSFVec2f,
  (setFunc_t) vsgProto_setSFVec3f,
  0
};

typedef void (*getFunc_t) (vsgWorld *, SFNode, unsigned int);
static getFunc_t vsgProto_getField[] = {
  0,
  (getFunc_t) vsgProto_getMFColor,
  (getFunc_t) vsgProto_getMFFloat,
  (getFunc_t) vsgProto_getMFInt32,
  (getFunc_t) vsgProto_getMFNode,
  (getFunc_t) vsgProto_getMFRotation,
  (getFunc_t) vsgProto_getMFString,
  (getFunc_t) vsgProto_getMFVec2f,
  (getFunc_t) vsgProto_getMFVec3f,
  0,
  (getFunc_t) vsgProto_getSFBool,
  (getFunc_t) vsgProto_getSFColor,
  (getFunc_t) vsgProto_getSFFloat,
  (getFunc_t) vsgProto_getSFImage,
  (getFunc_t) vsgProto_getSFInt32,
  (getFunc_t) vsgProto_getSFNode,
  (getFunc_t) vsgProto_getSFRotation,
  (getFunc_t) vsgProto_getSFString,
  (getFunc_t) vsgProto_getSFTime,
  (getFunc_t) vsgProto_getSFVec2f,
  (getFunc_t) vsgProto_getSFVec3f,
  0
};

static int
findPROTOField (vsgProtoClass const *pclass,
		symbol_t id, vsgFieldType baseType)
{
  vsgProtoField **fields = pclass->fields;
  int nfields = pclass->nfields;
  int i;
  for (i = 0; i < nfields; ++i) {
    vsgProtoField *f = fields[i];
    if (f->c.name == id 
	&& (f->c.fieldType == baseType
	    || f->c.fieldType == vsgTexposedField))
      return i;
  }
  return -1;
}

static vsgEventInDesc
vsgProto_setf (vsgClass const *cl, symbol_t id)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgEventInDesc evin;
  int i = findPROTOField (pclass, id, vsgTfield);
  vsgProtoField *f = (i >= 0 ? pclass->fields[i] : NULL);

  if (f) {
    evin.type = f->c.type;
    evin.sel = i;
    evin.f.generic = vsgProto_setField[f->c.type];
  } else {
    evin.type = vsgErrorType;
  }
  return evin;
}

static vsgEventOutDesc
vsgProto_getf (vsgClass const *cl, symbol_t id)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgEventOutDesc evout;
  int i = findPROTOField (pclass, id, vsgTfield);
  vsgProtoField *f = (i >= 0 ? pclass->fields[i] : NULL);

  if (f) {
    evout.type = f->c.type;
    evout.sel = i;
    evout.f.generic = vsgProto_getField[f->c.type];
  } else {
    evout.type = vsgErrorType;
  }
  return evout;
}

static vsgEventInDesc
vsgProto_eventIn (vsgClass const *cl, symbol_t id)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgEventInDesc evin;
  int i = findPROTOField (pclass, id, vsgTeventIn);
  vsgProtoField *f = (i >= 0 ? pclass->fields[i] : NULL);

  if (f) {
    evin.type = f->c.type;
    evin.sel = i;
    evin.f.generic = vsgProto_setField[f->c.type];
  } else {
    evin.type = vsgErrorType;
  }
  return evin;
}

static vsgEventOutDesc
vsgProto_eventOut (vsgClass const *cl, symbol_t id)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgEventOutDesc evout;
  int i = findPROTOField (pclass, id, vsgTeventOut);
  vsgProtoField *f = (i >= 0 ? pclass->fields[i] : NULL);

  if (f) {
    evout.type = f->c.type;
    evout.sel = i;
    evout.f.generic = vsgProto_getField[f->c.type];
  } else {
    evout.type = vsgErrorType;
  }
  return evout;
}

static vsgClassField **
vsgProto_fields (vsgClass const *cl)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  return (vsgClassField **) pclass->fields;
}

static MFProto *
vsgProto_protos (vsgClass const *cl)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  return pclass->protos;
}

static vsgClass protoClass = {
  vsgNPROTO,
  vsgClassIsProtoFlag,
  vsgProto_create,
  vsgProto_setf,
  vsgProto_getf,
  vsgProto_eventIn,
  vsgProto_eventOut,
  vsgProto_fields,
  vsgProto_protos
};

vsgClass *
vsgdStartProto (vsgWorld *curr, symbol_t id)
{
  vsgProtoClass *proto;
  MFProto *protos = vsgiCurrProtos (curr);
  vsgiFrame *frm;

  /* This establishes a local DEF/USE context. */
  proto = (vsgProtoClass *) istack_alloc (curr->data, sizeof (vsgProtoClass));
  proto->c = protoClass;
  proto->c.name = id;
  proto->is_extern = FALSE;
  proto->nfields = 0;
  proto->fields = 0;
  proto->nodes = (MFNode *) curr->data;
  proto->props = plist_create (32);

  /* Define the prototype via an entry in the frame's protos list and a name
     in the current frame. */
  if (! protos) curr->frame->protos = protos = vsgMakeMFProto (curr);
  vsgAppendSFProto (curr, protos, (vsgClass *) proto);
  vsgBindClass (curr, (vsgClass *) proto);

  if (! vsgiClaimStack (curr)) {
    vsgError (curr, "Cannot start PROTO %s.  No available istack.", id);
    return 0;
  }

  if (vsgTracing)
    fprintf (stderr, "vsgdStartProto(%s) => %p\n", id, proto);

  frm = vsgPushFrame (curr, (vsgClass *) proto);
  frm->props = proto->props;

  return (vsgClass *) proto;
}

void
vsgdEndProtoFields (vsgWorld *curr)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);
  istack *ist = (istack *) proto->nodes;
  void *null = NULL;

  /* Add NULL to finish the field list. */
  istack_grow (ist, &null, sizeof (sizeof (vsgProtoField *)));
  proto->fields = (vsgProtoField **) istack_finish (ist);
  vsgiReleaseStack (curr, ist);

  if (vsgTracing)
    fprintf (stderr, "vsgdEndProtoFields(%s) %p[%d]\n",
	     proto->c.name, proto->fields, proto->nfields);
}

static void
vsgiDetachPlist (vsgWorld *curr, void *p)
{
  plist_destroy ((plist_t) p);
}

vsgClass const *
vsgdEndProto (vsgWorld *curr)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);
  plist_t props = vsgiCurrProps (curr);

  if (vsgTracing)
    fprintf (stderr, "vsgdEndProto(%s) scene %p\n",
	     proto->c.name, proto->nodes);

  /* Make sure that the frame properties don't get deallocated. */
  curr->frame->props = 0;
  vsgRegisterExtern (curr, props, vsgiDetachPlist);

  /* Save the local proto declarations. */
  proto->protos = vsgiCurrProtos (curr);
  proto->routes = vsgiCurrRoutes (curr);

  vsgPopFrame (curr);

  return (vsgClass const *) proto;
}

void
vsgdDeclareField (vsgWorld *curr,
		  vsgFieldType ftype, vsgDataType type,
		  symbol_t fname)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);
  istack *ist = (istack *) proto->nodes;
  vsgProtoField *field = (vsgProtoField *)
    istack_alloc (curr->data, sizeof(*field));

  field->c.name = fname;
  field->c.type = type;
  field->c.fieldType = ftype;
  field->c.flags = 0;
  field->IS_refs = 0;

  /* Add pointer to list. */
  istack_grow (ist, &field, sizeof (field));
  proto->fields = (vsgProtoField **) istack_base (ist);
  ++proto->nfields;

  if (vsgTracing)
    fprintf (stderr, "vsgdDeclareField(%s %p) %s\n",
	     fname, fname, vsgDataTypeName[type]);

  vsgDefaultAnyArg (type, &field->c.dflt);
}

static void
vsgdVaDeclareFieldDefault (vsgWorld *curr,
			   vsgDataType type, symbol_t fname, va_list ap)
{
  vsgProtoClass *pclass = vsgiCurrProto (curr);
  vsgProtoField *field;
  int i;

  if (pclass->is_extern) {
    vsgError (curr, "Cannot declare default for EXTERNPROTO field %s",
	      fname);
    return;
  }

  i = findPROTOField (pclass, fname, vsgTfield);
  if (i < 0) {
    vsgError (curr, "No field %s for default declaration", fname);
    return;
  }

  field = pclass->fields[i];
  if (type != field->c.type) {
    vsgError (curr, "Mismatched types (%s != %s) in %s default",
		vsgDataTypeName[type], vsgDataTypeName[field->c.type],
		field->c.name);
    return;
  }

  switch (type) {
  case vsgMFColor:
  case vsgMFFloat:
  case vsgMFInt32:
  case vsgMFNode:
  case vsgMFRotation:
  case vsgMFString:
  case vsgMFVec2f:
  case vsgMFVec3f:
    field->c.dflt.MFGeneric = va_arg (ap, MFGeneric *);
    break;
  case vsgSFBool:
    field->c.dflt.SFBool = va_arg (ap, SFBool);
    break;
  case vsgSFColor:
    field->c.dflt.SFColor = va_arg (ap, SFColor);
    break;
  case vsgSFFloat:
    field->c.dflt.SFFloat = va_arg (ap, double);
    break;
  case vsgSFImage:
    field->c.dflt.SFImage = va_arg (ap, SFImage);
    break;
  case vsgSFInt32:
    field->c.dflt.SFInt32 = va_arg (ap, SFInt32);
    break;
  case vsgSFNode:
    field->c.dflt.SFNode = va_arg (ap, SFNode);
    break;
  case vsgSFRotation:
    field->c.dflt.SFRotation = va_arg (ap, SFRotation);
    break;
  case vsgSFString:
    field->c.dflt.SFString = va_arg (ap, SFString);
    break;
  case vsgSFTime:
    field->c.dflt.SFTime = va_arg (ap, double);
    break;
  case vsgSFVec2f:
    field->c.dflt.SFVec2f = va_arg (ap, SFVec2f);
    break;
  case vsgSFVec3f:
    field->c.dflt.SFVec3f = va_arg (ap, SFVec3f);
    break;
  default:
    vsgError (curr, "Unknown data type '%d'", type);
    break;
  }
}

static void
vsgdDeclareNodes (vsgWorld *curr, MFNode *nodes)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);

  proto->nodes = nodes;
}

static vsgClass externProtoClass = {
  vsgNEXTERNPROTO,
  vsgClassIsProtoFlag,
  vsgProto_create,
  vsgProto_setf,
  vsgProto_getf,
  vsgProto_eventIn,
  vsgProto_eventOut,
  vsgProto_fields,
  vsgProto_protos
};

vsgClass *
vsgdStartExternProto (vsgWorld *curr, symbol_t id)
{
  vsgExternProtoClass *externproto;
  vsgProtoClass *proto;
  MFProto *protos = vsgiCurrProtos (curr);
  vsgiFrame *frm;

  /* This establishes a local DEF/USE context. */
  externproto = (vsgExternProtoClass *)
    istack_alloc (curr->data, sizeof (vsgExternProtoClass));
  proto = &externproto->p;
  proto->c = externProtoClass;
  proto->c.name = id;
  proto->is_extern = TRUE;
  proto->nfields = 0;
  proto->fields = 0;
  proto->nodes = (MFNode *) curr->data;
  proto->props = 0;
  externproto->extern_urls = NULL;

  /* Define the prototype via an entry in the frame's protos list and a name
     in the current frame. */
  if (! protos) curr->frame->protos = protos = vsgMakeMFProto (curr);
  vsgAppendSFProto (curr, protos, (vsgClass *) proto);
  vsgBindClass (curr, (vsgClass *) proto);

  if (! vsgiClaimStack (curr)) {
    vsgError (curr, "Cannot start EXTERNPROTO %s.  No available istack.", id);
    return 0;
  }

  if (vsgTracing)
    fprintf (stderr, "vsgdStartExternProto(%s) => %p\n", id, proto);

  frm = vsgPushFrame (curr, (vsgClass *) proto);

  return (vsgClass *) proto;
}

vsgClass const *
vsgdEndExternProto (vsgWorld *curr)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);
  istack *ist = (istack *) proto->nodes;
  void *null = NULL;

  /* Clear nodes, since we were using the slot as a temporary. */
  proto->nodes = 0;

  if (vsgTracing)
    fprintf (stderr, "vsgdEndExternProto(%s)\n", proto->c.name);

  /* Add NULL to finish the field list. */
  istack_grow (ist, &null, sizeof (sizeof (vsgProtoField *)));
  proto->fields = (vsgProtoField **) istack_finish (ist);
  vsgiReleaseStack (curr, ist);

  vsgPopFrame (curr);

  return (vsgClass const *) proto;
}

static void 
vsgdDeclareExtern (vsgWorld *curr, MFString *extern_urls)
{
  vsgProtoClass *proto = vsgiCurrProto (curr);
  vsgExternProtoClass *externproto = (vsgExternProtoClass *) proto;
  if (! proto->is_extern) {
    vsgError (curr, "Attempt to set extern_urls for PROTO %s", proto->c.name);
    return;
  }
  externproto->extern_urls = extern_urls;
}

static SFNode 
vsgScript_create (vsgClass const *cl, vsgWorld *wrl)
{
  vsgProtoClass const *pclass = (vsgProtoClass const *) cl;
  vsgProtoNode *node = vsgAlloc (wrl, sizeof (*node));
  int nfields = pclass->nfields;
  int i;

  node->pclass = pclass;
  node->nodes = NULL;
  node->bindings = (vsgAnyArg *)
    istack_alloc (wrl->data, nfields * sizeof (node->bindings[0]));

  for (i = 0; i < nfields; ++i)
    node->bindings[i] = pclass->fields[i]->c.dflt;

  return (SFNode) node;
}

static MFProto *
vsgScript_protos (vsgClass const *cl)
{
  return NULL;
}

static vsgClass scriptClass = {
  vsgNScript,
  vsgClassIsScriptFlag,
  vsgScript_create,
  vsgProto_setf,
  vsgProto_getf,
  vsgProto_eventIn,
  vsgProto_eventOut,
  vsgProto_fields,
  vsgScript_protos
};

static MFNode MFNodeEmpty_s = { {vsgMFNode, 0, 0}, 0 };
static MFNode *MFNodeEmpty = &MFNodeEmpty_s;

vsgClass *
vsgdStartScript (vsgWorld *curr)
{
  vsgProtoClass *script;

  /* This establishes a local DEF/USE context. */
  script = (vsgProtoClass *) istack_alloc (curr->data, sizeof (vsgProtoClass));
  script->c = scriptClass;
  script->c.name = vsgNScript;
  script->is_extern = FALSE;
  script->nfields = 0;
  script->fields = 0;
  script->nodes = (MFNode *) curr->data;
  script->props = 0;

  if (! vsgiClaimStack (curr)) {
    vsgError (curr, "Cannot start Script.  No available istack.");
    return 0;
  }

  if (vsgTracing)
    fprintf (stderr, "vsgdStartScript => %p\n", script);

  vsgPushFrame (curr, (vsgClass *) script);

  vsgDeclareField (curr, vsgTexposedField, vsgMFString, vsgNurl);
  vsgDeclareFieldDefault (curr, vsgMFString, vsgNurl, MFNodeEmpty);
  vsgDeclareField (curr, vsgTfield, vsgSFBool, vsgNdirectOutput);
  vsgDeclareFieldDefault (curr, vsgSFBool, vsgNdirectOutput, FALSE);
  vsgDeclareField (curr, vsgTfield, vsgSFBool, vsgNmustEvaluate);
  vsgDeclareFieldDefault (curr, vsgSFBool, vsgNmustEvaluate, FALSE);
  
  return &script->c;
}

vsgClass const * 
vsgdEndScript (vsgWorld *curr)
{
  vsgProtoClass *script = vsgiCurrProto (curr);
  istack *ist = (istack *) script->nodes;
  void *null = NULL;

  /* Add NULL to finish the field list. */
  istack_grow (ist, &null, sizeof (sizeof (vsgProtoField *)));
  script->fields = (vsgProtoField **) istack_finish (ist);
  vsgiReleaseStack (curr, ist);

  if (vsgTracing)
    fprintf (stderr, "vsgdEndScript\n");

  vsgPopFrame (curr);

  return (vsgClass const *) script;
}

void
vsgdSetIS (vsgWorld *curr, SFNode node, symbol_t fieldId, symbol_t is_name)
{
  vsgFieldRefs *slist;
  vsgClass const *cl = vsgNodeClass (node);
  vsgClassField *field, **fields = vsgGetFields (cl);
  vsgProtoClass *proto = vsgiCurrProto (curr);
  vsgProtoField *protoField, **protoFields = proto->fields;

  /* Find the node field. */
  for ( ; (field = *fields); ++fields)
    if (field->name == fieldId)
      break;
  if (! field) {
    vsgError (curr, "Field %s.%s not found", cl->name, fieldId);
    return;
  }

  /* Find the PROTO field. */
  for ( ; (protoField = *protoFields); ++protoFields)
    if (is_name == protoField->c.name)
      break;
  if (! protoField) {
    vsgError (curr, "Field %s.%s not found", proto->c.name, is_name);
    return;
  }

  slist = (vsgFieldRefs *) istack_alloc (curr->data, sizeof(*slist));
  slist->next = protoField->IS_refs;
  slist->node = node;
  slist->field = field;

  protoField->IS_refs = slist;
  plist_put (vsgiCurrProps (curr), node, fieldId, protoField);

  if (vsgTracing)
    fprintf (stderr, "vsgdSetIS(%s:%p.%s) <= %s.%s\n",
	     cl->name, node, fieldId, proto->c.name, is_name);
}

vsgClassField *
vsgdHasIS (vsgWorld *curr, plist_t props, SFNode node, symbol_t id)
{
  vsgProtoField *field;

  if (! id) return NULL;

  field = (vsgProtoField *) plist_get (props, node, id);

  if (vsgTracing && field) {
    vsgClass const *cl = vsgNodeClass (node);
    fprintf (stderr, "vsgHasIS (%s:%p.%s) => %s\n",
	     cl->name, node, id, field->c.name);
  }

  return &field->c;
}


symbol_t vsgDataTypeName[] = {
  "Error",
  vsgNMFColor,
  vsgNMFFloat,
  vsgNMFInt32,
  vsgNMFNode,
  vsgNMFRotation,
  vsgNMFString,
  vsgNMFVec2f,
  vsgNMFVec3f,
  "MFProto",
  vsgNSFBool,
  vsgNSFColor,
  vsgNSFFloat,
  vsgNSFImage,
  vsgNSFInt32,
  vsgNSFNode,
  vsgNSFRotation,
  vsgNSFString,
  vsgNSFTime,
  vsgNSFVec2f,
  vsgNSFVec3f,
  "SFProto"
};

symbol_t vsgFieldTypeName[] = {
  0,
  vsgNeventIn,
  vsgNfield,
  vsgNexposedField,
  vsgNeventOut
};

int vsgDataTypeSize[] = {
  0,

  sizeof (MFColor*),
  sizeof (MFFloat*),
  sizeof (MFInt32*),
  sizeof (MFNode*),
  sizeof (MFRotation*),
  sizeof (MFString*),
  sizeof (MFVec2f*),
  sizeof (MFVec3f*),
  sizeof (MFProto*),
  sizeof (SFBool),
  sizeof (SFColor),
  sizeof (SFFloat),
  sizeof (SFImage),
  sizeof (SFInt32),
  sizeof (SFNode),
  sizeof (SFRotation),
  sizeof (SFString),
  sizeof (SFTime),
  sizeof (SFVec2f),
  sizeof (SFVec3f),
  0
};

