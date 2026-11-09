/*
 * Name:  vrml2stubs.c
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
 *   $Id: vrml2stubs.c,v 1.16 2000/08/28 23:21:50 reddy Exp $
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

#include "vrml2int.h"

#include <stdlib.h>

vsgFunc_t vsgFuncs[vsgfNUM_FUNCIDS];

typedef int (*int_func_t) ();
typedef void * (*ptr_func_t) ();

void
vsgAppendChild (vsgWorld *wrl, SFNode node, SFNode child)
{
  MFNode *mfnode = vsgGetMFNode (wrl, node, vsgGetFunc (node, vsgNchildren));

  vsgAppendSFNode (wrl, mfnode, child);
}

#define VSG_CLASS(name)					\
vsgClass const *vsg##name##Class = 0;			\
SFNode							\
vsgNew##name (vsgWorld *wrl)				\
{							\
  if (! vsg##name##Class) 				\
    vsg##name##Class = vsgGetClass (wrl, vsgN##name);	\
  return vsgCreateNode (wrl, vsg##name##Class);		\
}

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

#define SET_GENERIC(node,field,_type)					\
int vsgSet##node##_##field (vsgWorld *wrl, SFNode n, _type##Arg val)	\
{									\
  static vsgEventInDesc evin = {vsgErrorType, 0, {0}};			\
  vsgClass const *cl = vsgNodeClass (n);				\
  if (cl != vsg##node##Class) {						\
    vsgError (wrl, "vsgSet" #node "_" #field " called with %s node",	\
	      cl->name);						\
    return 0;								\
  }									\
  if (evin.type == vsgErrorType)					\
    evin = vsgiGet_setFunc (cl, vsgN##field);				\
  return vsgSet##_type (wrl, n, evin, val);				\
}

SET_GENERIC (Anchor, children, MFNode);
SET_GENERIC (Anchor, description, SFString);
SET_GENERIC (Anchor, parameter, MFString);
SET_GENERIC (Anchor, url, MFString);
SET_GENERIC (Anchor, bboxCenter, SFVec3f);
SET_GENERIC (Anchor, bboxSize, SFVec3f);

SET_GENERIC (Appearance, material, SFNode);
SET_GENERIC (Appearance, texture, SFNode);
SET_GENERIC (Appearance, textureTransform, SFNode);

SET_GENERIC (AudioClip, description, SFString);
SET_GENERIC (AudioClip, loop, SFBool);
SET_GENERIC (AudioClip, pitch, SFFloat);
SET_GENERIC (AudioClip, startTime, SFTime);
SET_GENERIC (AudioClip, stopTime, SFTime);
SET_GENERIC (AudioClip, url, MFString);

SET_GENERIC (Background, backUrl, MFString);
SET_GENERIC (Background, bottomUrl, MFString);
SET_GENERIC (Background, frontUrl, MFString);
SET_GENERIC (Background, groundAngle, MFFloat);
SET_GENERIC (Background, groundColor, MFColor);
SET_GENERIC (Background, leftUrl, MFString);
SET_GENERIC (Background, rightUrl, MFString);
SET_GENERIC (Background, skyAngle, MFFloat);
SET_GENERIC (Background, skyColor, MFColor);
SET_GENERIC (Background, topUrl, MFString);

SET_GENERIC (Billboard, axisOfRotation, SFVec3f);
SET_GENERIC (Billboard, bboxCenter, SFVec3f);
SET_GENERIC (Billboard, bboxSize, SFVec3f);
SET_GENERIC (Billboard, children, MFNode);

SET_GENERIC (Box, size, SFVec3f);

SET_GENERIC (Collision, bboxCenter, SFVec3f);
SET_GENERIC (Collision, bboxSize, SFVec3f);
SET_GENERIC (Collision, children, MFNode);
SET_GENERIC (Collision, collide, SFBool);
SET_GENERIC (Collision, proxy, SFNode);

SET_GENERIC (ColorInterpolator, key, MFFloat);
SET_GENERIC (ColorInterpolator, keyValue, MFColor);

SET_GENERIC (Color, color, MFColor);

SET_GENERIC (Cone, bottom, SFBool);
SET_GENERIC (Cone, bottomRadius, SFFloat);
SET_GENERIC (Cone, height, SFFloat);
SET_GENERIC (Cone, side, SFBool);

SET_GENERIC (CoordinateInterpolator, key, MFFloat);
SET_GENERIC (CoordinateInterpolator, keyValue, MFVec3f);

SET_GENERIC (Coordinate, point, MFVec3f);

SET_GENERIC (CylinderSensor, autoOffset, SFBool);
SET_GENERIC (CylinderSensor, diskAngle, SFFloat);
SET_GENERIC (CylinderSensor, enabled, SFBool);
SET_GENERIC (CylinderSensor, maxAngle, SFFloat);
SET_GENERIC (CylinderSensor, minAngle, SFFloat);
SET_GENERIC (CylinderSensor, offset, SFFloat);

SET_GENERIC (Cylinder, bottom, SFBool);
SET_GENERIC (Cylinder, height, SFFloat);
SET_GENERIC (Cylinder, radius, SFFloat);
SET_GENERIC (Cylinder, side, SFBool);
SET_GENERIC (Cylinder, top, SFBool);

SET_GENERIC (DirectionalLight, ambientIntensity, SFFloat);
SET_GENERIC (DirectionalLight, color, SFColor);
SET_GENERIC (DirectionalLight, direction, SFVec3f);
SET_GENERIC (DirectionalLight, intensity, SFFloat);
SET_GENERIC (DirectionalLight, on, SFBool);

SET_GENERIC (ElevationGrid, ccw, SFBool);
SET_GENERIC (ElevationGrid, color, SFNode);
SET_GENERIC (ElevationGrid, colorPerVertex, SFBool);
SET_GENERIC (ElevationGrid, creaseAngle, SFFloat);
SET_GENERIC (ElevationGrid, height, MFFloat);
SET_GENERIC (ElevationGrid, normal, SFNode);
SET_GENERIC (ElevationGrid, normalPerVertex, SFBool);
SET_GENERIC (ElevationGrid, solid, SFBool);
SET_GENERIC (ElevationGrid, texCoord, SFNode);
SET_GENERIC (ElevationGrid, xDimension, SFInt32);
SET_GENERIC (ElevationGrid, xSpacing, SFFloat);
SET_GENERIC (ElevationGrid, zDimension, SFInt32);
SET_GENERIC (ElevationGrid, zSpacing, SFFloat);

SET_GENERIC (Extrusion, beginCap, SFBool);
SET_GENERIC (Extrusion, ccw, SFBool);
SET_GENERIC (Extrusion, convex, SFBool);
SET_GENERIC (Extrusion, creaseAngle, SFFloat);
SET_GENERIC (Extrusion, crossSection, MFVec2f);
SET_GENERIC (Extrusion, endCap, SFBool);
SET_GENERIC (Extrusion, orientation, MFRotation);
SET_GENERIC (Extrusion, scale, MFVec2f);
SET_GENERIC (Extrusion, solid, SFBool);
SET_GENERIC (Extrusion, spine, MFVec3f);

SET_GENERIC (Fog, color, SFColor);
SET_GENERIC (Fog, fogType, SFString);
SET_GENERIC (Fog, visibilityRange, SFFloat);

SET_GENERIC (FontStyle, family, MFString);
SET_GENERIC (FontStyle, horizontal, SFBool);
SET_GENERIC (FontStyle, justify, MFString);
SET_GENERIC (FontStyle, language, SFString);
SET_GENERIC (FontStyle, leftToRight, SFBool);
SET_GENERIC (FontStyle, size, SFFloat);
SET_GENERIC (FontStyle, spacing, SFFloat);
SET_GENERIC (FontStyle, style, SFString);
SET_GENERIC (FontStyle, topToBottom, SFBool);

SET_GENERIC (Group, bboxCenter, SFVec3f);
SET_GENERIC (Group, bboxSize, SFVec3f);
SET_GENERIC (Group, children, MFNode);

SET_GENERIC (ImageTexture, repeatS, SFBool);
SET_GENERIC (ImageTexture, repeatT, SFBool);
SET_GENERIC (ImageTexture, url, MFString);

SET_GENERIC (IndexedFaceSet, ccw, SFBool);
SET_GENERIC (IndexedFaceSet, color, SFNode);
SET_GENERIC (IndexedFaceSet, colorIndex, MFInt32);
SET_GENERIC (IndexedFaceSet, colorPerVertex, SFBool);
SET_GENERIC (IndexedFaceSet, convex, SFBool);
SET_GENERIC (IndexedFaceSet, coord, SFNode);
SET_GENERIC (IndexedFaceSet, coordIndex, MFInt32);
SET_GENERIC (IndexedFaceSet, creaseAngle, SFFloat);
SET_GENERIC (IndexedFaceSet, normal, SFNode);
SET_GENERIC (IndexedFaceSet, normalIndex, MFInt32);
SET_GENERIC (IndexedFaceSet, normalPerVertex, SFBool);
SET_GENERIC (IndexedFaceSet, solid, SFBool);
SET_GENERIC (IndexedFaceSet, texCoord, SFNode);
SET_GENERIC (IndexedFaceSet, texCoordIndex, MFInt32);

SET_GENERIC (IndexedLineSet, color, SFNode);
SET_GENERIC (IndexedLineSet, colorIndex, MFInt32);
SET_GENERIC (IndexedLineSet, colorPerVertex, SFBool);
SET_GENERIC (IndexedLineSet, coord, SFNode);
SET_GENERIC (IndexedLineSet, coordIndex, MFInt32);

SET_GENERIC (Inline, bboxCenter, SFVec3f);
SET_GENERIC (Inline, bboxSize, SFVec3f);
SET_GENERIC (Inline, url, MFString);

SET_GENERIC (LOD, center, SFVec3f);
SET_GENERIC (LOD, level, MFNode);
SET_GENERIC (LOD, range, MFFloat);

SET_GENERIC (Material, ambientIntensity, SFFloat);
SET_GENERIC (Material, diffuseColor, SFColor);
SET_GENERIC (Material, emissiveColor, SFColor);
SET_GENERIC (Material, shininess, SFFloat);
SET_GENERIC (Material, specularColor, SFColor);
SET_GENERIC (Material, transparency, SFFloat);

SET_GENERIC (MovieTexture, loop, SFBool);
SET_GENERIC (MovieTexture, repeatS, SFBool);
SET_GENERIC (MovieTexture, repeatT, SFBool);
SET_GENERIC (MovieTexture, speed, SFFloat);
SET_GENERIC (MovieTexture, startTime, SFTime);
SET_GENERIC (MovieTexture, stopTime, SFTime);
SET_GENERIC (MovieTexture, url, MFString);

SET_GENERIC (NavigationInfo, avatarSize, MFFloat);
SET_GENERIC (NavigationInfo, headlight, SFBool);
SET_GENERIC (NavigationInfo, speed, SFFloat);
SET_GENERIC (NavigationInfo, type, MFString);
SET_GENERIC (NavigationInfo, visibilityLimit, SFFloat);

SET_GENERIC (NormalInterpolator, key, MFFloat);
SET_GENERIC (NormalInterpolator, keyValue, MFVec3f);

SET_GENERIC (Normal, vector, MFVec3f);

SET_GENERIC (OrientationInterpolator, key, MFFloat);
SET_GENERIC (OrientationInterpolator, keyValue, MFRotation);

SET_GENERIC (PixelTexture, image, SFImage);
SET_GENERIC (PixelTexture, repeatS, SFBool);
SET_GENERIC (PixelTexture, repeatT, SFBool);

SET_GENERIC (PlaneSensor, autoOffset, SFBool);
SET_GENERIC (PlaneSensor, enabled, SFBool);
SET_GENERIC (PlaneSensor, maxPosition, SFVec2f);
SET_GENERIC (PlaneSensor, minPosition, SFVec2f);
SET_GENERIC (PlaneSensor, offset, SFVec3f);

SET_GENERIC (PointLight, ambientIntensity, SFFloat);
SET_GENERIC (PointLight, attenuation, SFVec3f);
SET_GENERIC (PointLight, color, SFColor);
SET_GENERIC (PointLight, intensity, SFFloat);
SET_GENERIC (PointLight, location, SFVec3f);
SET_GENERIC (PointLight, on, SFBool);
SET_GENERIC (PointLight, radius, SFFloat);

SET_GENERIC (PointSet, color, SFNode);
SET_GENERIC (PointSet, coord, SFNode);

SET_GENERIC (PositionInterpolator, key, MFFloat);
SET_GENERIC (PositionInterpolator, keyValue, MFVec3f);

SET_GENERIC (ProximitySensor, center, SFVec3f);
SET_GENERIC (ProximitySensor, enabled, SFBool);
SET_GENERIC (ProximitySensor, size, SFVec3f);

SET_GENERIC (ScalarInterpolator, key, MFFloat);
SET_GENERIC (ScalarInterpolator, keyValue, MFFloat);

SET_GENERIC (Script, directOutput, SFBool);
SET_GENERIC (Script, mustEvaluate, SFBool);
SET_GENERIC (Script, url, MFString);

SET_GENERIC (Shape, appearance, SFNode);
SET_GENERIC (Shape, geometry, SFNode);

SET_GENERIC (Sound, direction, SFVec3f);
SET_GENERIC (Sound, intensity, SFFloat);
SET_GENERIC (Sound, location, SFVec3f);
SET_GENERIC (Sound, maxBack, SFFloat);
SET_GENERIC (Sound, maxFront, SFFloat);
SET_GENERIC (Sound, minBack, SFFloat);
SET_GENERIC (Sound, minFront, SFFloat);
SET_GENERIC (Sound, priority, SFFloat);
SET_GENERIC (Sound, source, SFNode);
SET_GENERIC (Sound, spatialize, SFBool);

SET_GENERIC (SphereSensor, autoOffset, SFBool);
SET_GENERIC (SphereSensor, enabled, SFBool);
SET_GENERIC (SphereSensor, offset, SFRotation);

SET_GENERIC (Sphere, radius, SFFloat);

SET_GENERIC (SpotLight, ambientIntensity, SFFloat);
SET_GENERIC (SpotLight, attenuation, SFVec3f);
SET_GENERIC (SpotLight, beamWidth, SFFloat);
SET_GENERIC (SpotLight, color, SFColor);
SET_GENERIC (SpotLight, cutOffAngle, SFFloat);
SET_GENERIC (SpotLight, direction, SFVec3f);
SET_GENERIC (SpotLight, intensity, SFFloat);
SET_GENERIC (SpotLight, location, SFVec3f);
SET_GENERIC (SpotLight, on, SFBool);
SET_GENERIC (SpotLight, radius, SFFloat);

SET_GENERIC (Switch, choice, MFNode);
SET_GENERIC (Switch, whichChoice, SFInt32);

SET_GENERIC (Text, fontStyle, SFNode);
SET_GENERIC (Text, length, MFFloat);
SET_GENERIC (Text, maxExtent, SFFloat);
SET_GENERIC (Text, string, MFString);

SET_GENERIC (TextureCoordinate, point, MFVec2f);

SET_GENERIC (TextureTransform, center, SFVec2f);
SET_GENERIC (TextureTransform, rotation, SFFloat);
SET_GENERIC (TextureTransform, scale, SFVec2f);
SET_GENERIC (TextureTransform, translation, SFVec2f);

SET_GENERIC (TimeSensor, cycleInterval, SFTime);
SET_GENERIC (TimeSensor, enabled, SFBool);
SET_GENERIC (TimeSensor, loop, SFBool);
SET_GENERIC (TimeSensor, startTime, SFTime);
SET_GENERIC (TimeSensor, stopTime, SFTime);

SET_GENERIC (TouchSensor, enabled, SFBool);

SET_GENERIC (Transform, bboxCenter, SFVec3f);
SET_GENERIC (Transform, bboxSize, SFVec3f);
SET_GENERIC (Transform, center, SFVec3f);
SET_GENERIC (Transform, children, MFNode);
SET_GENERIC (Transform, rotation, SFRotation);
SET_GENERIC (Transform, scale, SFVec3f);
SET_GENERIC (Transform, scaleOrientation, SFRotation);
SET_GENERIC (Transform, translation, SFVec3f);

SET_GENERIC (Viewpoint, description, SFString);
SET_GENERIC (Viewpoint, fieldOfView, SFFloat);
SET_GENERIC (Viewpoint, jump, SFBool);
SET_GENERIC (Viewpoint, orientation, SFRotation);
SET_GENERIC (Viewpoint, position, SFVec3f);

SET_GENERIC (VisibilitySensor, center, SFVec3f);
SET_GENERIC (VisibilitySensor, enabled, SFBool);
SET_GENERIC (VisibilitySensor, size, SFVec3f);

SET_GENERIC (WorldInfo, info, MFString);
SET_GENERIC (WorldInfo, title, SFString);

vsgClass *
vsgStartProto (vsgWorld *curr, symbol_t id)
{
  vsgFunc_t func = vsgFuncs[vsgfStartProto];
  return func ? (*func)(curr, id) : NULL;
}

void
vsgDeclareField (vsgWorld *curr, vsgFieldType ftype, vsgDataType type, 
		 symbol_t fname)
{
  vsgFunc_t func = vsgFuncs[vsgfDeclareField];
  func ? (*func)(curr, ftype, type, fname) : NULL;
}

void
vsgVaDeclareFieldDefault (vsgWorld *curr, 
			  vsgDataType type, symbol_t fname, va_list ap)
{
  vsgFunc_t func = vsgFuncs[vsgfVaDeclareFieldDefault];
  func ? (*func)(curr, type, fname, ap) : NULL;
}

void
vsgDeclareFieldDefault (vsgWorld *curr, vsgDataType type, symbol_t fname, ...)
{
  va_list ap;
  va_start (ap, fname);
  vsgVaDeclareFieldDefault (curr, type, fname, ap);
  va_end (ap);
}

void
vsgEndProtoFields (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfEndProtoFields];
  func ? (*func)(curr) : NULL;
}

void
vsgDeclareExtern (vsgWorld *curr, MFString *extern_urls)
{
  vsgFunc_t func = vsgFuncs[vsgfDeclareExtern];
  func ? (*func)(curr, extern_urls) : NULL;
}

void
vsgDeclareNodes (vsgWorld *curr, MFNode *nodes)
{
  vsgFunc_t func = vsgFuncs[vsgfDeclareNodes];
  func ? (*func)(curr, nodes) : NULL;
}

vsgClass const *
vsgEndProto (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfEndProto];
  return func ? (*func)(curr) : NULL;
}

vsgClass *
vsgStartExternProto (vsgWorld *curr, symbol_t id)
{
  vsgFunc_t func = vsgFuncs[vsgfStartExternProto];
  return func ? (*func)(curr, id) : NULL;
}

vsgClass const *
vsgEndExternProto (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfEndExternProto];
  return func ? (*func)(curr) : NULL;
}

void
vsgSetIS (vsgWorld *curr, SFNode node, symbol_t fieldId, symbol_t is_name)
{
  vsgFunc_t func = vsgFuncs[vsgfSetIS];
  func ? (*func)(curr, node, fieldId, is_name) : NULL;
}

vsgClassField *
vsgHasIS (vsgWorld *curr, plist_t props, SFNode node, symbol_t name)
{
  vsgFunc_t func = vsgFuncs[vsgfHasIS];
  return func ? (*func)(curr, props, node, name) : NULL;
}

vsgClass * 
vsgStartScript (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfStartScript];
  return func ? (*func)(curr) : NULL;
}

vsgClass const * vsgEndScript (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfEndScript];
  return func ? (*func)(curr) : NULL;
}

void
vsgStartWorld (vsgWorld *curr)
{
  (*vsgDefault (vsgfInit, vsgdInit)) ();
  vsgPushFrame (curr, NULL);
}

void
vsgEndWorld (vsgWorld *curr)
{
  vsgFunc_t func = vsgFuncs[vsgfCleanup];

  /* Get rid of the lexer. */
  if (curr->lexer) {
    vsglDestroy (curr->lexer);
    curr->lexer = 0;
  }

  func ? (*func)(curr) : NULL;
}

int
vsgPrint (vsgWorld *curr, FILE *fp)
{
  vsgFunc_t func = vsgFuncs[vsgfPrint];
  return func ? (*(int_func_t)func)(curr, fp) : 0;
}

void
vsgRender (vsgWorld *curr, void *closure)
{
  vsgFunc_t func = vsgFuncs[vsgfRender];
  func ? (*func)(curr,closure) : 0;
}

void
vsgSetFilename (vsgWorld *curr, char const *fname)
{
  vsgFunc_t func = vsgFuncs[vsgfSetFilename];
  curr->filename = fname;
  func ? (*func)(curr, fname) : NULL;
}

void
vsgSetFiletype (vsgWorld *curr, char const *ftype)
{
  vsgFunc_t func = vsgFuncs[vsgfSetFiletype];
  curr->filetype = ftype;
  func ? (*func)(curr, ftype) : NULL;
}
