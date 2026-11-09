/*
 * Name: vrml2il.h
 *
 * Library:
 *   This source file is part of the vrml2 library, a library for parsing
 *   and generating VRML 97 format files. This library is supplied as part
 *   of the tsmApi distribution, available from:
 *
 *     http://www.tsmApi.com/
 *
 * Revision History:
 *   VRML 2.0 version by Chris Connolly, SRI International
 *   Updated interface by Lee Iverson, SRI International
 *
 *   $Id: vrml2il.h,v 1.6.6.1 2002/07/30 22:26:12 reddy Exp $
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

exinline vsgClass const *
vsgNodeClass (SFNode node)
{
  return node->cl;
}

exinline vsgEventInDesc 
vsgiGet_setFunc (vsgClass const *cl, symbol_t name)
{
  return (*cl->get_setFunc) (cl, name);
}

exinline vsgEventOutDesc 
vsgiGet_getFunc (vsgClass const *cl, symbol_t name)
{
  return (*cl->get_getFunc) (cl, name);
}

exinline vsgEventInDesc 
vsgiGet_eventIn (vsgClass const *cl, symbol_t name)
{
  return (*cl->get_eventIn) (cl, name);
}

exinline vsgEventOutDesc 
vsgiGet_eventOut (vsgClass const *cl, symbol_t name)
{
  return (*cl->get_eventOut) (cl, name);
}

exinline SFNode
vsgCreateNode (vsgWorld *wrl, vsgClass const *cl)
{
  return ( cl ) ? (*cl->create_f) (cl, wrl) : NULL;
}

exinline vsgClassField **
vsgGetFields (vsgClass const *cl)
{
  return (*cl->get_fields) (cl);
}

exinline MFProto *
vsgGetProtos (vsgClass const *cl)
{
  return (*cl->get_protos) (cl);
}

exinline vsgEventInDesc 
vsgSetFunc (SFNode node, symbol_t name)
{
  vsgClass const *cl = vsgNodeClass (node);
  return vsgiGet_setFunc (cl, name);
}

exinline vsgEventOutDesc 
vsgGetFunc (SFNode node, symbol_t name)
{
  vsgClass const *cl = vsgNodeClass (node);
  return vsgiGet_getFunc (cl, name);
}

exinline int
vsgSetMFColor (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFColor *val)
{
  return (*desc.f.MFColor) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFFloat (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFFloat *val)
{
  return (*desc.f.MFFloat) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFInt32 (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFInt32 *val)
{
  return (*desc.f.MFInt32) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFNode (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFNode *val)
{
  return (*desc.f.MFNode) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFRotation (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFRotation *val)
{
  return (*desc.f.MFRotation) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFString (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFString *val)
{
  return (*desc.f.MFString) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFVec2f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFVec2f *val)
{
  return (*desc.f.MFVec2f) (wrl, node, desc.sel, val);
}

exinline int
vsgSetMFVec3f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFVec3f *val)
{
  return (*desc.f.MFVec3f) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFBool (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFBool val)
{
  return (*desc.f.SFBool) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFColor (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFColor val)
{
  return (*desc.f.SFColor) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFFloat (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFFloat val)
{
  return (*desc.f.SFFloat) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFImage (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFImage val)
{
  return (*desc.f.SFImage) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFInt32 (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFInt32 val)
{
  return (*desc.f.SFInt32) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFNode (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFNode val)
{
  return (*desc.f.SFNode) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFRotation (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFRotation val)
{
  return (*desc.f.SFRotation) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFString (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFString val)
{
  return (*desc.f.SFString) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFTime (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFTime val)
{
  return (*desc.f.SFTime) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFVec2f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFVec2f val)
{
  return (*desc.f.SFVec2f) (wrl, node, desc.sel, val);
}

exinline int
vsgSetSFVec3f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFVec3f val)
{
  return (*desc.f.SFVec3f) (wrl, node, desc.sel, val);
}

exinline MFColor *
vsgGetMFColor (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFColor) (wrl, node, desc.sel);
}

exinline MFFloat *
vsgGetMFFloat (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFFloat) (wrl, node, desc.sel);
}

exinline MFInt32 *
vsgGetMFInt32 (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFInt32) (wrl, node, desc.sel);
}

exinline MFNode *
vsgGetMFNode (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFNode) (wrl, node, desc.sel);
}

exinline MFRotation *
vsgGetMFRotation (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFRotation) (wrl, node, desc.sel);
}

exinline MFString *
vsgGetMFString (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFString) (wrl, node, desc.sel);
}

exinline MFVec2f *
vsgGetMFVec2f (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFVec2f) (wrl, node, desc.sel);
}

exinline MFVec3f *
vsgGetMFVec3f (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.MFVec3f) (wrl, node, desc.sel);
}

exinline SFBool
vsgGetSFBool (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFBool) (wrl, node, desc.sel);
}

exinline SFColor
vsgGetSFColor (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFColor) (wrl, node, desc.sel);
}

exinline SFFloat
vsgGetSFFloat (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFFloat) (wrl, node, desc.sel);
}

exinline SFImage
vsgGetSFImage (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFImage) (wrl, node, desc.sel);
}

exinline SFNode
vsgGetSFNode (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFNode) (wrl, node, desc.sel);
}

exinline SFInt32
vsgGetSFInt32 (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFInt32) (wrl, node, desc.sel);
}

exinline SFRotation
vsgGetSFRotation (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFRotation) (wrl, node, desc.sel);
}

exinline SFString
vsgGetSFString (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFString) (wrl, node, desc.sel);
}

exinline SFTime
vsgGetSFTime (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFTime) (wrl, node, desc.sel);
}

exinline SFVec2f
vsgGetSFVec2f (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFVec2f) (wrl, node, desc.sel);
}

exinline SFVec3f
vsgGetSFVec3f (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc)
{
  return (*desc.f.SFVec3f) (wrl, node, desc.sel);
}

exinline SFColor
vsgMakeSFColor (SFFloat r, SFFloat g, SFFloat b)
{
  SFColor c;
  c.r = r; c.g = g; c.b = b;
  return c;
}

exinline SFRotation
vsgMakeSFRotation (SFFloat a, SFFloat b, SFFloat c, SFFloat d)
{
  SFRotation r;
  r.a = a; r.b = b; r.c = c; r.d = d;
  return r;
}

exinline SFVec2f
vsgMakeSFVec2f (SFFloat v0, SFFloat v1)
{
  SFVec2f v;
  v.v[0] = v0; v.v[1] = v1;
  return v;
}

exinline SFVec3f
vsgMakeSFVec3f (SFFloat v0, SFFloat v1, SFFloat v2)
{
  SFVec3f v;
  v.v[0] = v0; v.v[1] = v1; v.v[2] = v2;
  return v;
}
