/*
 * Name: c_nodes.h
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
 *   $Id: c_nodes.h,v 1.11.6.1 2001/04/24 19:24:08 leei Exp $
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

#ifndef _C_NODES_H_
#define _C_NODES_H_

#include "vrml2.h"
#include "vec3.h"

/* VRML 2.x Node definitions.  Each node is implemented as a struct,
 * whose fields map directly onto the fields found in the VRML 2 spec.
 */

/*------------------------------------------------------------------*/

#define vsgcAsSFNode(n) (&(n)->super.n)
#define vsgcAsNode(n) (&(n)->super)

#define vsgcNode_DIRTY (1<<0)

typedef struct {
  struct _SFNode n;
  unsigned int flags;
  void *aux;
} vsgcNode;

typedef struct {
  vsgcNode super;

  MFNode   *children;
  SFString description;
  MFString *parameter;
  MFString *url;
  SFVec3f  bboxCenter;
  SFVec3f  bboxSize;
} vsgcAnchor;

vsgcAnchor *vsgcNewAnchor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode material;
  SFNode texture;
  SFNode textureTransform;
} vsgcAppearance;

vsgcAppearance *vsgcNewAppearance (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFString description;
  SFBool   loop;
  SFFloat  pitch;
  SFTime   startTime;
  SFTime   stopTime;
  MFString *url;

  SFBool isActive;
} vsgcAudioClip;

vsgcAudioClip *vsgcNewAudioClip (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFFloat  *groundAngle;
  MFColor  *groundColor;
  MFString *backUrl;
  MFString *bottomUrl;
  MFString *frontUrl;
  MFString *leftUrl;
  MFString *rightUrl;
  MFString *topUrl;
  MFFloat  *skyAngle;
  MFColor  *skyColor;
} vsgcBackground;

vsgcBackground *vsgcNewBackground (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode   *addChildren;
  MFNode   *removeChildren;
  SFVec3f  axisOfRotation;
  MFNode   *children;
  SFVec3f  bboxCenter;
  SFVec3f  bboxSize;
} vsgcBillboard;

vsgcBillboard *vsgcNewBillboard (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFVec3f size;
} vsgcBox;

vsgcBox *vsgcNewBox (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode   *children;
  SFBool   collide;
  SFVec3f  bboxCenter;
  SFVec3f  bboxSize;
  SFNode   proxy;
  SFTime   collideTime;
} vsgcCollision;

vsgcCollision *vsgcNewCollision (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFColor *color;
} vsgcColor;

vsgcColor *vsgcNewColor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat fraction;
  MFFloat *key;
  MFColor *keyValue;
} vsgcColorInterpolator;

vsgcColorInterpolator *vsgcNewColorInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat bottomRadius;
  SFFloat height;
  SFBool  side;
  SFBool  bottom;
} vsgcCone;

vsgcCone *vsgcNewCone (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFVec3f *point;
} vsgcCoordinate;

vsgcCoordinate *vsgcNewCoordinate (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat fraction;
  MFFloat *key;
  MFVec3f *keyValue;
} vsgcCoordinateInterpolator;

vsgcCoordinateInterpolator *vsgcNewCoordinateInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool  bottom;
  SFFloat height;
  SFFloat radius;
  SFBool  side;
  SFBool  top;

} vsgcCylinder;

vsgcCylinder *vsgcNewCylinder (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool     autoOffset;
  SFFloat    diskAngle;
  SFBool     enabled;
  SFFloat    maxAngle;
  SFFloat    minAngle;
  SFFloat    offset;
  SFBool     isActive;

} vsgcCylinderSensor;

vsgcCylinderSensor *vsgcNewCylinderSensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat ambientIntensity;
  SFColor color;
  SFVec3f direction;
  SFFloat intensity;
  SFBool  on;

} vsgcDirectionalLight;

vsgcDirectionalLight *vsgcNewDirectionalLight (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode   color;
  SFNode   normal;
  SFNode   texCoord;
  MFFloat  *height;
  SFBool   ccw;
  SFBool   colorPerVertex;
  SFFloat  creaseAngle;
  SFBool   normalPerVertex;
  SFBool   solid;
  SFInt32  xDimension;
  SFFloat  xSpacing;
  SFInt32  zDimension;
  SFFloat  zSpacing;
} vsgcElevationGrid;

vsgcElevationGrid *vsgcNewElevationGrid (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool     beginCap;
  SFBool     ccw;
  SFBool     convex;
  SFFloat    creaseAngle;
  MFVec2f    *crossSection;
  SFBool     endCap;
  MFRotation *orientation;
  MFVec2f    *scale;
  SFBool     solid;
  MFVec3f    *spine;

} vsgcExtrusion;

vsgcExtrusion *vsgcNewExtrusion (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFColor  color;
  SFString fogType;
  SFFloat  visibilityRange;

  SFBool   isBound;

} vsgcFog;

vsgcFog *vsgcNewFog (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *family;
  SFBool   horizontal;
  MFString *justify;
  SFString language;
  SFBool   leftToRight;
  SFFloat  size;
  SFFloat  spacing;
  SFString style;
  SFBool   topToBottom;
} vsgcFontStyle;

vsgcFontStyle *vsgcNewFontStyle (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode  *addChildren;
  MFNode  *removeChildren;
  MFNode  *children;
  SFVec3f bboxCenter;
  SFVec3f bboxSize;
} vsgcGroup;

vsgcGroup *vsgcNewGroup (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *url;
  SFBool   repeatS;
  SFBool   repeatT;
} vsgcImageTexture;

vsgcImageTexture *vsgcNewImageTexture (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode  color;
  SFNode  coord;
  SFNode  normal;
  SFNode  texCoord;
  SFBool  ccw;
  MFInt32 *colorIndex;
  SFBool  colorPerVertex;
  SFBool  convex;
  MFInt32 *coordIndex;
  SFFloat creaseAngle;
  MFInt32 *normalIndex;
  SFBool  normalPerVertex;
  SFBool  solid;
  MFInt32 *texCoordIndex;
} vsgcIndexedFaceSet;

vsgcIndexedFaceSet *vsgcNewIndexedFaceSet (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode  color;
  SFNode  coord;
  MFInt32 *colorIndex;
  SFBool  colorPerVertex;
  MFInt32 *coordIndex;
} vsgcIndexedLineSet;

vsgcIndexedLineSet *vsgcNewIndexedLineSet (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *url;
  SFVec3f  bboxCenter;
  SFVec3f  bboxSize;
} vsgcInline;

vsgcInline *vsgcNewInline (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode  *level;
  SFVec3f center;
  MFFloat *range;
} vsgcLOD;

vsgcLOD *vsgcNewLOD (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat ambientIntensity;
  SFColor diffuseColor;
  SFColor emissiveColor;
  SFFloat shininess;
  SFColor specularColor;
  SFFloat transparency;
} vsgcMaterial;

vsgcMaterial *vsgcNewMaterial (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool   loop;
  SFFloat  speed;
  SFTime   startTime;
  SFTime   stopTime;
  MFString *url;
  SFBool   repeatS;
  SFBool   repeatT;

  SFBool   isActive;
} vsgcMovieTexture;

vsgcMovieTexture *vsgcNewMovieTexture (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFFloat  *avatarSize;
  SFBool   headlight;
  SFFloat  speed;
  MFString *type;
  SFFloat  visibilityLimit;
  SFBool   isBound;
} vsgcNavigationInfo;

vsgcNavigationInfo *vsgcNewNavigationInfo (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFVec3f *vector;
} vsgcNormal;

vsgcNormal *vsgcNewNormal (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat fraction;
  MFFloat *key;
  MFVec3f *keyValue;
} vsgcNormalInterpolator;

vsgcNormalInterpolator *vsgcNewNormalInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat    fraction;
  MFFloat    *key;
  MFRotation *keyValue;
} vsgcOrientationInterpolator;

vsgcOrientationInterpolator *vsgcNewOrientationInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFImage  image;
  SFBool   repeatS;
  SFBool   repeatT;
} vsgcPixelTexture;

vsgcPixelTexture *vsgcNewPixelTexture (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool  autoOffset;
  SFBool  enabled;
  SFVec2f maxPosition;
  SFVec2f minPosition;
  SFVec3f offset;

  SFBool  isActive;
  SFVec3f trackPoint_changed;
  SFVec3f translation_changed;
} vsgcPlaneSensor;

vsgcPlaneSensor *vsgcNewPlaneSensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat ambientIntensity;
  SFVec3f attenuation;
  SFColor color;
  SFFloat intensity;
  SFVec3f location;
  SFBool  on;
  SFFloat radius;
} vsgcPointLight;

vsgcPointLight *vsgcNewPointLight (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode color;
  SFNode coord;
} vsgcPointSet;

vsgcPointSet *vsgcNewPointSet (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat fraction;
  MFFloat *key;
  MFVec3f *keyValue;
} vsgcPositionInterpolator;

vsgcPositionInterpolator *vsgcNewPositionInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFVec3f    center;
  SFVec3f    size;
  SFBool     enabled;

  SFBool     isActive;
  SFVec3f    position_changed;
  SFRotation orientation_changed;
  SFTime     enterTime;
  SFTime     exitTime;

} vsgcProximitySensor;

vsgcProximitySensor *vsgcNewProximitySensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat fraction;
  MFFloat *key;
  MFFloat *keyValue;
} vsgcScalarInterpolator;

vsgcScalarInterpolator *vsgcNewScalarInterpolator (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *url;
  SFBool   directOutput;
  SFBool   mustEvaluate;

  /* # <I>And any number of</I>:
  eventTypeName eventName;
  fieldTypeName fieldName;
  eventTypeName eventName; */

} vsgcScript;

vsgcScript *vsgcNewScript (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFNode appearance;
  SFNode geometry;
} vsgcShape;

vsgcShape *vsgcNewShape (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFVec3f  direction;
  SFFloat intensity;
  SFVec3f  location;
  SFFloat maxBack;
  SFFloat maxFront;
  SFFloat minBack;
  SFFloat minFront;
  SFFloat priority;
  SFNode  source;
  SFBool  spatialize;
} vsgcSound;

vsgcSound *vsgcNewSound (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat radius;
} vsgcSphere;

vsgcSphere *vsgcNewSphere (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFBool    autoOffset;
  SFBool    enabled;
  SFRotation offset;

  SFBool    isActive;
  SFRotation rotation_changed;
  SFVec3f    trackPoint_changed;
} vsgcSphereSensor;

vsgcSphereSensor *vsgcNewSphereSensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat ambientIntensity;
  SFVec3f  attenuation;
  SFFloat beamWidth;
  SFColor  color;
  SFFloat cutOffAngle;
  SFVec3f  direction;
  SFFloat intensity;
  SFVec3f  location;
  SFBool  on;
  SFFloat radius;
} vsgcSpotLight;

vsgcSpotLight *vsgcNewSpotLight (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode  *choice;
  SFInt32 whichChoice;
} vsgcSwitch;

vsgcSwitch *vsgcNewSwitch (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *string;
  SFNode   fontStyle;
  MFFloat  *length;
  SFFloat  maxExtent;
} vsgcText;

vsgcText *vsgcNewText (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFVec2f *point;
} vsgcTextureCoordinate;

vsgcTextureCoordinate *vsgcNewTextureCoordinate (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFVec2f  center;
  SFFloat rotation;
  SFVec2f  scale;
  SFVec2f  translation;

} vsgcTextureTransform;

vsgcTextureTransform *vsgcNewTextureTransform (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFTime  cycleInterval;
  SFBool  enabled;
  SFBool  loop;
  SFTime  startTime;
  SFTime  stopTime;
  SFTime  cycleTime;
  SFFloat fraction_changed;

  SFBool  isActive;
  SFTime  time;
} vsgcTimeSensor;


vsgcTimeSensor *vsgcNewTimeSensor (vsgWorld *curr);

typedef struct {
  vsgcNode super;

  SFBool  enabled;
  SFVec3f  hitNormal_changed;
  SFVec3f  hitPoint_changed;
  SFVec2f  hitTexCoord_changed;

  SFBool  isActive;
  SFBool  isOver;
  SFTime  touchTime;

} vsgcTouchSensor;

vsgcTouchSensor *vsgcNewTouchSensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFNode      *addChildren;
  MFNode      *removeChildren;
  SFVec3f     center;
  MFNode      *children;
  SFRotation  rotation;
  SFVec3f     scale;
  SFRotation  scaleOrientation;
  SFVec3f     translation;
  SFVec3f     bboxCenter;
  SFVec3f     bboxSize;
} vsgcTransform  ;

vsgcTransform *vsgcNewTransform (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFFloat   fieldOfView;
  SFBool    jump;
  SFRotation orientation;
  SFVec3f    position;
  SFString   description;
  SFTime    bindTime;
  SFBool    isBound;

  m4f       tfm;
} vsgcViewpoint;

vsgcViewpoint *vsgcNewViewpoint (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  SFVec3f center;
  SFBool enabled;
  SFVec3f size;
  SFTime enterTime;
  SFTime exitTime;

  SFBool isActive;

} vsgcVisibilitySensor;

vsgcVisibilitySensor *vsgcNewVisibilitySensor (vsgWorld *curr);


typedef struct {
  vsgcNode super;

  MFString *info;
  SFString title;
} vsgcWorldInfo;

vsgcWorldInfo *vsgcNewWorldInfo (vsgWorld *curr);

#define EXPOSEDFIELD(node,_type,field,_val)		\
  FIELD(node,_type,field,_val)				\
  EVENTIN(node,_type,field)				\
  EVENTOUT(node,_type,field)

#define EVENTIN(node,_type,field)
#define EVENTOUT(node,_type,field)

#undef DEFNODE
#define DEFNODE(name)

#undef FIELD
#define FIELD(node,_type,field,_val)		\
  void * vsgc##node##_##field (vsgcNode *n);

#undef ENDNODE
#define ENDNODE(name)

#include "nodes.def"

typedef struct {
  char *filename;
  char *version_string;

  vsgcGroup *top_group;
  MFNode *prototypes;

} vsgcInfo;

typedef void *(*vsgCallback_t)(vsgWorld *,void *);

vsgcNode *vsgcEffectiveNode (SFNode node);

int vsgcValuesEqual (vsgDataType type, void const *v1p, void const *v2p);

void vsgcSetup (void);
int vsgcSetupRenderGL (void);
void vsgcSetupPrinting (void);
void vsgcSetupCustomRender (vsgCallback_t);

/* This declares the field access functions. */
#undef DEFNODE
#define DEFNODE(node)

#undef FIELD
#define FIELD(node,_type,field,_val)

#undef EVENTIN
#define EVENTIN(node,_type,field)					\
int vsgc##node##_set_##field (vsgWorld *wrl, SFNode gn,			\
			      unsigned int sel, _type##Arg v);

#undef EVENTOUT
#define EVENTOUT(node,_type,field)					 \
_type##Arg vsgc##node##_##field##_changed (vsgWorld *wrl, SFNode gn,	 \
					   unsigned int sel);

#undef ENDNODE
#define ENDNODE(node)

#include "nodes.def"

#undef EVENTIN
#define EVENTIN(node,_type,field)
#undef EVENTOUT
#define EVENTOUT(node,_type,field)

#endif
