/*
 * Name: vrml.h
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
 *   $Id: vrml2.h,v 1.25 2000/11/10 18:51:03 leei Exp $
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

#ifndef _VRML_H
#define _VRML_H

#include <stdio.h>
#include <stdarg.h>

#include <prtypes.h>

#include "symtab.h"
#include "plist.h"
#include "syms.h"

extern int vsgDataTypeSize[];
extern char const *vsgDataTypeName[];
extern char const *vsgFieldTypeName[];

/* Indices into the stub table. */
typedef enum vsgfId_t {
  vsgfInit,
  vsgfCleanup,
  vsgfPrint,
  vsgfRender,

  vsgfSetFilename,
  vsgfSetFiletype,

  vsgfStartProto,
  vsgfEndProtoFields,
  vsgfEndProto,

  vsgfDeclareField,
  vsgfVaDeclareFieldDefault,
  vsgfDeclareFieldIS,
  vsgfDeclareNodes,

  vsgfStartExternProto,
  vsgfEndExternProto,
  vsgfDeclareExtern,

  vsgfStartScript,
  vsgfEndScript,

  vsgfSetIS,
  vsgfHasIS,

  vsgfAnchor_children,
  vsgfAnchor_description,
  vsgfAnchor_parameter,
  vsgfAnchor_url,
  vsgfAnchor_bboxCenter,
  vsgfAnchor_bboxSize,

  vsgfAppearance_material,
  vsgfAppearance_texture,
  vsgfAppearance_textureTransform,

  vsgfAudioClip_description,
  vsgfAudioClip_loop,
  vsgfAudioClip_pitch,
  vsgfAudioClip_startTime,
  vsgfAudioClip_stopTime,
  vsgfAudioClip_url,

  vsgfBackground_backUrl,
  vsgfBackground_bottomUrl,
  vsgfBackground_frontUrl,
  vsgfBackground_groundAngle,
  vsgfBackground_groundColor,
  vsgfBackground_leftUrl,
  vsgfBackground_rightUrl,
  vsgfBackground_skyAngle,
  vsgfBackground_skyColor,
  vsgfBackground_topUrl,

  vsgfBillboard_axisOfRotation,
  vsgfBillboard_bboxCenter,
  vsgfBillboard_bboxSize,
  vsgfBillboard_children,

  vsgfBox_size,

  vsgfCollision_bboxCenter,
  vsgfCollision_bboxSize,
  vsgfCollision_children,
  vsgfCollision_collide,
  vsgfCollision_proxy,

  vsgfColorInterpolator_key,
  vsgfColorInterpolator_keyValue,

  vsgfColor_color,

  vsgfCone_bottom,
  vsgfCone_bottomRadius,
  vsgfCone_height,
  vsgfCone_side,

  vsgfCoordinateInterpolator_key,
  vsgfCoordinateInterpolator_keyValue,

  vsgfCoordinate_point,

  vsgfCylinderSensor_autoOffset,
  vsgfCylinderSensor_diskAngle,
  vsgfCylinderSensor_enabled,
  vsgfCylinderSensor_maxAngle,
  vsgfCylinderSensor_minAngle,
  vsgfCylinderSensor_offset,

  vsgfCylinder_bottom,
  vsgfCylinder_height,
  vsgfCylinder_radius,
  vsgfCylinder_side,
  vsgfCylinder_top,

  vsgfDirectionalLight_ambientIntensity,
  vsgfDirectionalLight_color,
  vsgfDirectionalLight_direction,
  vsgfDirectionalLight_intensity,
  vsgfDirectionalLight_on,

  vsgfElevationGrid_ccw,
  vsgfElevationGrid_color,
  vsgfElevationGrid_colorPerVertex,
  vsgfElevationGrid_creaseAngle,
  vsgfElevationGrid_height,
  vsgfElevationGrid_normal,
  vsgfElevationGrid_normalPerVertex,
  vsgfElevationGrid_solid,
  vsgfElevationGrid_texCoord,
  vsgfElevationGrid_xDimension,
  vsgfElevationGrid_xSpacing,
  vsgfElevationGrid_zDimension,
  vsgfElevationGrid_zSpacing,

  vsgfExtrusion_beginCap,
  vsgfExtrusion_ccw,
  vsgfExtrusion_convex,
  vsgfExtrusion_creaseAngle,
  vsgfExtrusion_crossSection,
  vsgfExtrusion_endCap,
  vsgfExtrusion_orientation,
  vsgfExtrusion_scale,
  vsgfExtrusion_solid,
  vsgfExtrusion_spine,

  vsgfFog_color,
  vsgfFog_fogType,
  vsgfFog_visibilityRange,

  vsgfFontStyle_family,
  vsgfFontStyle_horizontal,
  vsgfFontStyle_justify,
  vsgfFontStyle_language,
  vsgfFontStyle_leftToRight,
  vsgfFontStyle_size,
  vsgfFontStyle_spacing,
  vsgfFontStyle_style,
  vsgfFontStyle_topToBottom,

  vsgfGroup_bboxCenter,
  vsgfGroup_bboxSize,
  vsgfGroup_children,

  vsgfImageTexture_repeatS,
  vsgfImageTexture_repeatT,
  vsgfImageTexture_url,

  vsgfIndexedFaceSet_ccw,
  vsgfIndexedFaceSet_color,
  vsgfIndexedFaceSet_colorIndex,
  vsgfIndexedFaceSet_colorPerVertex,
  vsgfIndexedFaceSet_convex,
  vsgfIndexedFaceSet_coord,
  vsgfIndexedFaceSet_coordIndex,
  vsgfIndexedFaceSet_creaseAngle,
  vsgfIndexedFaceSet_normal,
  vsgfIndexedFaceSet_normalIndex,
  vsgfIndexedFaceSet_normalPerVertex,
  vsgfIndexedFaceSet_solid,
  vsgfIndexedFaceSet_texCoord,
  vsgfIndexedFaceSet_texCoordIndex,

  vsgfIndexedLineSet_color,
  vsgfIndexedLineSet_colorIndex,
  vsgfIndexedLineSet_colorPerVertex,
  vsgfIndexedLineSet_coord,
  vsgfIndexedLineSet_coordIndex,

  vsgfInline_bboxCenter,
  vsgfInline_bboxSize,
  vsgfInline_url,

  vsgfLOD_center,
  vsgfLOD_level,
  vsgfLOD_range,

  vsgfMaterial_ambientIntensity,
  vsgfMaterial_diffuseColor,
  vsgfMaterial_emissiveColor,
  vsgfMaterial_shininess,
  vsgfMaterial_specularColor,
  vsgfMaterial_transparency,

  vsgfMovieTexture_loop,
  vsgfMovieTexture_repeatS,
  vsgfMovieTexture_repeatT,
  vsgfMovieTexture_speed,
  vsgfMovieTexture_startTime,
  vsgfMovieTexture_stopTime,
  vsgfMovieTexture_url,

  vsgfNavigationInfo_avatarSize,
  vsgfNavigationInfo_headlight,
  vsgfNavigationInfo_speed,
  vsgfNavigationInfo_type,
  vsgfNavigationInfo_visibilityLimit,

  vsgfNormalInterpolator_key,
  vsgfNormalInterpolator_keyValue,

  vsgfNormal_vector,

  vsgfOrientationInterpolator_key,
  vsgfOrientationInterpolator_keyValue,

  vsgfPixelTexture_image,
  vsgfPixelTexture_repeatS,
  vsgfPixelTexture_repeatT,

  vsgfPlaneSensor_autoOffset,
  vsgfPlaneSensor_enabled,
  vsgfPlaneSensor_maxPosition,
  vsgfPlaneSensor_minPosition,
  vsgfPlaneSensor_offset,

  vsgfPointLight_ambientIntensity,
  vsgfPointLight_attenuation,
  vsgfPointLight_color,
  vsgfPointLight_intensity,
  vsgfPointLight_location,
  vsgfPointLight_on,
  vsgfPointLight_radius,

  vsgfPointSet_color,
  vsgfPointSet_coord,

  vsgfPositionInterpolator_key,
  vsgfPositionInterpolator_keyValue,

  vsgfProximitySensor_center,
  vsgfProximitySensor_enabled,
  vsgfProximitySensor_size,

  vsgfScalarInterpolator_key,
  vsgfScalarInterpolator_keyValue,

  vsgfScript_directOutput,
  vsgfScript_mustEvaluate,
  vsgfScript_url,

  vsgfShape_appearance,
  vsgfShape_geometry,

  vsgfSound_direction,
  vsgfSound_intensity,
  vsgfSound_location,
  vsgfSound_maxBack,
  vsgfSound_maxFront,
  vsgfSound_minBack,
  vsgfSound_minFront,
  vsgfSound_priority,
  vsgfSound_source,
  vsgfSound_spatialize,

  vsgfSphereSensor_autoOffset,
  vsgfSphereSensor_enabled,
  vsgfSphereSensor_offset,

  vsgfSphere_radius,

  vsgfSpotLight_ambientIntensity,
  vsgfSpotLight_attenuation,
  vsgfSpotLight_beamWidth,
  vsgfSpotLight_color,
  vsgfSpotLight_cutOffAngle,
  vsgfSpotLight_direction,
  vsgfSpotLight_intensity,
  vsgfSpotLight_location,
  vsgfSpotLight_on,
  vsgfSpotLight_radius,

  vsgfSwitch_choice,
  vsgfSwitch_whichChoice,

  vsgfText_fontStyle,
  vsgfText_length,
  vsgfText_maxExtent,
  vsgfText_string,

  vsgfTextureCoordinate_point,

  vsgfTextureTransform_center,
  vsgfTextureTransform_rotation,
  vsgfTextureTransform_scale,
  vsgfTextureTransform_translation,

  vsgfTimeSensor_cycleInterval,
  vsgfTimeSensor_enabled,
  vsgfTimeSensor_loop,
  vsgfTimeSensor_startTime,
  vsgfTimeSensor_stopTime,

  vsgfTouchSensor_enabled,

  vsgfTransform_bboxCenter,
  vsgfTransform_bboxSize,
  vsgfTransform_center,
  vsgfTransform_children,
  vsgfTransform_rotation,
  vsgfTransform_scale,
  vsgfTransform_scaleOrientation,
  vsgfTransform_translation,

  vsgfViewpoint_description,
  vsgfViewpoint_fieldOfView,
  vsgfViewpoint_jump,
  vsgfViewpoint_orientation,
  vsgfViewpoint_position,

  vsgfVisibilitySensor_center,
  vsgfVisibilitySensor_enabled,
  vsgfVisibilitySensor_size,

  vsgfWorldInfo_info,
  vsgfWorldInfo_title,

  vsgfNUM_FUNCIDS
} vsgfId_t;

typedef void * (*vsgFunc_t) ();

extern vsgFunc_t vsgFuncs[];

typedef enum vsgDataType {
  vsgErrorType,
  vsgMFColor,
  vsgMFFloat,
  vsgMFInt32,
  vsgMFNode,
  vsgMFRotation,
  vsgMFString,
  vsgMFVec2f,
  vsgMFVec3f,
  vsgMFProto,
  vsgSFBool,
  vsgSFColor,
  vsgSFFloat,
  vsgSFImage,
  vsgSFInt32,
  vsgSFNode,
  vsgSFRotation,
  vsgSFString,
  vsgSFTime,
  vsgSFVec2f,
  vsgSFVec3f,
  vsgSFProto
} vsgDataType;

/* Primitive VRML 2 data types: I've used a bunch of typedefs to
 * define types that map directly onto things that appear in VRML 2
 * fields.  All of the MF... types are really ArrayLists.
 */

/* These 5 are literal -- the parser returns these values directly: */
typedef char* SFString;
typedef float SFFloat;
typedef PRInt32 SFInt32;
typedef unsigned int SFBool;
typedef double SFTime;

/* The remainder of the types are not literal -- they're either
 * pointers or lists.  The parser returns indices for these: */

typedef struct { SFFloat r, g, b; } SFColor;
typedef struct { SFFloat v[2]; } SFVec2f;
typedef struct { SFFloat v[3]; } SFVec3f;
typedef struct { SFFloat a, b, c, d; } SFRotation;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct vsgWorld vsgWorld;
typedef struct vsgEventInDesc vsgEventInDesc;
typedef struct vsgEventOutDesc vsgEventOutDesc;
typedef struct vsgRoute vsgRoute;
typedef struct vsgRouteLink vsgRouteLink;
typedef struct vsgClass vsgClass;
typedef struct vsgClassField vsgClassField;
typedef struct MFProto MFProto;

typedef struct _SFNode {
  vsgClass const *cl;
  vsgRouteLink *routes;
} *SFNode;

#define vsgClassIsProtoFlag  (1<<0)
#define vsgClassIsScriptFlag (1<<1)

struct vsgClass {
  symbol_t name;
  unsigned int flags;
  SFNode          (*create_f) (vsgClass const *, vsgWorld *);
  vsgEventInDesc  (*get_setFunc) (vsgClass const *, symbol_t);
  vsgEventOutDesc (*get_getFunc) (vsgClass const *, symbol_t);
  vsgEventInDesc  (*get_eventIn) (vsgClass const *, symbol_t);
  vsgEventOutDesc (*get_eventOut) (vsgClass const *, symbol_t);
  vsgClassField **(*get_fields) (vsgClass const *);
  MFProto *(*get_protos) (vsgClass const *);
};

typedef enum vsgFieldType {
  vsgTeventIn=1, vsgTfield, vsgTexposedField, vsgTeventOut
} vsgFieldType;

void vsgBindClass (vsgWorld *wrl, vsgClass const *cl);
vsgClass * vsgGetClass (vsgWorld *wrl, symbol_t name);

#define vsgClassIsProto(cl) ((cl)->flags & vsgClassIsProtoFlag)
#define vsgClassIsScript(cl) ((cl)->flags & vsgClassIsScriptFlag)

typedef struct {
  vsgDataType type:8;
  unsigned int flags:24;
  unsigned int len;
} MFShared;

typedef struct {
  MFShared mf;
  void *data;
} MFGeneric;

typedef struct {
  MFShared mf;
  SFFloat *data;
} MFFloat;

typedef struct {
  MFShared mf;
  SFRotation *data;
} MFRotation;

typedef struct {
  MFShared mf;
  SFVec2f *data;
} MFVec2f;

typedef struct {
  MFShared mf;
  SFVec3f *data;
} MFVec3f;

typedef struct {
  MFShared mf;
  SFInt32 *data;
} MFInt32;

typedef struct {
  MFShared mf;
  SFString *data;
} MFString;

typedef struct {
  MFShared mf;
  SFNode *data;
} MFNode;

typedef struct {
  MFShared mf;
  SFColor *data;
} MFColor;

struct MFProto {
  MFShared mf;
  struct vsgClass **data;
};

/*------------------------------------------------------------------*/
/*        Image: not a node, but will be needed below:              */

typedef struct {
  int width, height, depth;
  unsigned char *data;
} SFImage;

/*------------------------------------------------------------------*/
/*    These are the types which are used for passing arguments.     */

typedef SFString     SFStringArg;
typedef SFFloat      SFFloatArg;
typedef SFInt32      SFInt32Arg;
typedef SFBool       SFBoolArg;
typedef SFTime       SFTimeArg;
typedef SFColor      SFColorArg;
typedef SFVec2f      SFVec2fArg;
typedef SFVec3f      SFVec3fArg;
typedef SFRotation   SFRotationArg;
typedef SFNode       SFNodeArg;
typedef SFImage      SFImageArg;
typedef MFFloat *    MFFloatArg;
typedef MFRotation * MFRotationArg;
typedef MFVec2f *    MFVec2fArg;
typedef MFVec3f *    MFVec3fArg;
typedef MFInt32 *    MFInt32Arg;
typedef MFString *   MFStringArg;
typedef MFNode *     MFNodeArg;
typedef MFColor *    MFColorArg;

extern SFString SFString_default;
extern SFFloat SFFloat_default;
extern SFInt32 SFInt32_default;
extern SFBool SFBool_default;
extern SFTime SFTime_default;
extern SFColor SFColor_default;
extern SFVec2f SFVec2f_default;
extern SFVec3f SFVec3f_default;
extern SFRotation SFRotation_default;
extern SFNode SFNode_default;
extern SFImage SFImage_default;

extern MFFloat *MFFloat_default;
extern MFRotation *MFRotation_default;
extern MFVec2f *MFVec2f_default;
extern MFVec3f *MFVec3f_default;
extern MFInt32 *MFInt32_default;
extern MFString *MFString_default;
extern MFNode *MFNode_default;
extern MFColor *MFColor_default;

typedef union {
  SFStringArg    SFString;
  SFFloatArg     SFFloat;
  SFInt32Arg     SFInt32;
  SFBoolArg      SFBool;
  SFTimeArg      SFTime;
  SFColorArg     SFColor;
  SFVec2fArg     SFVec2f;
  SFVec3fArg     SFVec3f;
  SFRotationArg  SFRotation;
  SFNodeArg      SFNode;
  SFImageArg     SFImage;
  vsgClass *     SFProto;
  MFFloatArg     MFFloat;
  MFRotationArg  MFRotation;
  MFVec2fArg     MFVec2f;
  MFVec3fArg     MFVec3f;
  MFInt32Arg     MFInt32;
  MFStringArg    MFString;
  MFNodeArg      MFNode;
  MFColorArg     MFColor;
  MFGeneric *    MFGeneric;
} vsgAnyArg;

struct vsgClassField {
  vsgFieldType fieldType:4;
  vsgDataType type:8;
  unsigned int flags:16;
  symbol_t name;
  vsgAnyArg dflt;
};

/*------------------------------------------------------------------*/
/*    These are the generic functions for setting typed values      */

struct vsgEventInDesc {
  vsgDataType type:8;
  unsigned int sel:24;
  union {
    int (*MFColor) (vsgWorld *, SFNode, unsigned int, MFColorArg);
    int (*MFFloat) (vsgWorld *, SFNode, unsigned int, MFFloatArg);
    int (*MFInt32) (vsgWorld *, SFNode, unsigned int, MFInt32Arg);
    int (*MFNode)  (vsgWorld *, SFNode, unsigned int, MFNodeArg);
    int (*MFRotation) (vsgWorld *, SFNode, unsigned int, MFRotationArg);
    int (*MFString) (vsgWorld *, SFNode, unsigned int, MFStringArg);
    int (*MFVec2f) (vsgWorld *, SFNode, unsigned int, MFVec2fArg);
    int (*MFVec3f) (vsgWorld *, SFNode, unsigned int, MFVec3fArg);
    int (*SFBool) (vsgWorld *, SFNode, unsigned int, SFBoolArg);
    int (*SFColor) (vsgWorld *, SFNode, unsigned int, SFColorArg);
    int (*SFFloat) (vsgWorld *, SFNode, unsigned int, SFFloatArg);
    int (*SFImage) (vsgWorld *, SFNode, unsigned int, SFImageArg);
    int (*SFInt32) (vsgWorld *, SFNode, unsigned int, SFInt32Arg);
    int (*SFNode) (vsgWorld *, SFNode, unsigned int, SFNodeArg);
    int (*SFRotation) (vsgWorld *, SFNode, unsigned int, SFRotationArg);
    int (*SFString) (vsgWorld *, SFNode, unsigned int, SFStringArg);
    int (*SFTime) (vsgWorld *, SFNode, unsigned int, SFTimeArg);
    int (*SFVec2f) (vsgWorld *, SFNode, unsigned int, SFVec2fArg);
    int (*SFVec3f) (vsgWorld *, SFNode, unsigned int, SFVec3fArg);
    int (*generic) (vsgWorld *, SFNode, unsigned int, ...);
  } f;
};

int vsgSetMFColor (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFColor *val);
int vsgSetMFFloat (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFFloat *val);
int vsgSetMFInt32 (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFInt32 *val);
int vsgSetMFNode (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFNode *val);
int vsgSetMFRotation (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFRotation *val);
int vsgSetMFString (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFString *val);
int vsgSetMFVec2f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFVec2f *val);
int vsgSetMFVec3f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, MFVec3f *val);
int vsgSetSFBool (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFBool val);
int vsgSetSFColor (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFColor val);
int vsgSetSFFloat (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFFloat val);
int vsgSetSFImage (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFImage val);
int vsgSetSFInt32 (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFInt32 val);
int vsgSetSFNode (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFNode val);
int vsgSetSFRotation (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFRotation val);
int vsgSetSFString (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFString val);
int vsgSetSFTime (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFTime val);
int vsgSetSFVec2f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFVec2f val);
int vsgSetSFVec3f (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, SFVec3f val);

int vsgDescSet (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, ...);
int vsgVaDescSet (vsgWorld *wrl, SFNode node, vsgEventInDesc desc, va_list ap);
int vsgSet (vsgWorld *wrl, SFNode node, symbol_t fieldName, ...);
int vsgSetv (vsgWorld *wrl, SFNode node, ...);

struct vsgEventOutDesc {
  vsgDataType type:8;
  unsigned int sel:24;
  union {
    MFColorArg    (*MFColor) (vsgWorld *, SFNode, unsigned int);
    MFFloatArg    (*MFFloat) (vsgWorld *, SFNode, unsigned int);
    MFInt32Arg    (*MFInt32) (vsgWorld *, SFNode, unsigned int);
    MFNodeArg     (*MFNode) (vsgWorld *, SFNode, unsigned int);
    MFRotationArg (*MFRotation) (vsgWorld *, SFNode, unsigned int);
    MFStringArg   (*MFString) (vsgWorld *, SFNode, unsigned int);
    MFVec2fArg    (*MFVec2f) (vsgWorld *, SFNode, unsigned int);
    MFVec3fArg    (*MFVec3f) (vsgWorld *, SFNode, unsigned int);
    SFBoolArg     (*SFBool) (vsgWorld *, SFNode, unsigned int);
    SFColorArg    (*SFColor) (vsgWorld *, SFNode, unsigned int);
    SFFloatArg    (*SFFloat) (vsgWorld *, SFNode, unsigned int);
    SFImageArg    (*SFImage) (vsgWorld *, SFNode, unsigned int);
    SFInt32Arg    (*SFInt32) (vsgWorld *, SFNode, unsigned int);
    SFNodeArg     (*SFNode) (vsgWorld *, SFNode, unsigned int);
    SFRotationArg (*SFRotation) (vsgWorld *, SFNode, unsigned int);
    SFStringArg   (*SFString) (vsgWorld *, SFNode, unsigned int);
    SFTimeArg     (*SFTime) (vsgWorld *, SFNode, unsigned int);
    SFVec2fArg    (*SFVec2f) (vsgWorld *, SFNode, unsigned int);
    SFVec3fArg    (*SFVec3f) (vsgWorld *, SFNode, unsigned int);
    void          (*generic) (vsgWorld *, SFNode, unsigned int);
  } f;
};

MFColor *    vsgGetMFColor (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFFloat *    vsgGetMFFloat (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFInt32 *    vsgGetMFInt32 (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFNode *     vsgGetMFNode (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFRotation * vsgGetMFRotation (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFString *   vsgGetMFString (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFVec2f *    vsgGetMFVec2f (vsgWorld *, SFNode node, vsgEventOutDesc desc);
MFVec3f *    vsgGetMFVec3f (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFBool       vsgGetSFBool (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFColor      vsgGetSFColor (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFFloat      vsgGetSFFloat (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFImage      vsgGetSFImage (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFNode       vsgGetSFNode (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFInt32      vsgGetSFInt32 (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFRotation   vsgGetSFRotation (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFString     vsgGetSFString (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFTime       vsgGetSFTime (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFVec2f      vsgGetSFVec2f (vsgWorld *, SFNode node, vsgEventOutDesc desc);
SFVec3f      vsgGetSFVec3f (vsgWorld *, SFNode node, vsgEventOutDesc desc);

void vsgDescGet (vsgWorld *wrl, SFNode node, vsgEventOutDesc desc, vsgAnyArg *val);
int vsgGet (vsgWorld *wrl, SFNode node, symbol_t fieldName, vsgAnyArg *val);

struct vsgRouteLink {
  vsgRoute *route;
  vsgRouteLink *next;
};

struct vsgRoute {
  SFNode from;
  symbol_t fromField;
  vsgEventOutDesc fromDesc;
  SFNode to;
  symbol_t toField;
  vsgEventInDesc toDesc;
  void (*process) (vsgRoute const *);
};

vsgRoute * vsgAddRoute (vsgWorld *wrl,
			SFNode from, symbol_t fromField,
			SFNode to, symbol_t toField);

/* Structures for dealing with PROTO/EXTERNPROTO nodes. */

typedef struct vsgFieldRefs vsgFieldRefs;

struct vsgFieldRefs {
  vsgFieldRefs *next;
  SFNode node;
  vsgClassField *field;
  vsgEventInDesc eventIn;
  vsgEventOutDesc eventOut;
};

typedef struct vsgProtoField {
  vsgClassField c;
  vsgFieldRefs *IS_refs;
} vsgProtoField;

typedef struct vsgProtoClass {
  vsgClass c;
  unsigned int is_extern :  1;
  unsigned int nfields   : 31;
  vsgProtoField **fields;
  MFNode *nodes;
  plist_t props;
  MFProto *protos;
  vsgRouteLink *routes;
} vsgProtoClass;

typedef struct vsgExternProtoClass {
  vsgProtoClass p;
  MFString *extern_urls;
} vsgExternProtoClass;

typedef struct vsgProtoNode {
  vsgProtoClass const *pclass;
  MFNode *nodes;
  vsgAnyArg *bindings;
} vsgProtoNode;

typedef struct vsgiFrame vsgiFrame;

extern int vsgTracing;

void vsgdInit (void);

int vsgParseStream (vsgWorld *state, FILE *f, char const *source);
int vsgParseFile (vsgWorld *state, char const *source);
int vsgParseString (vsgWorld *wrl, char const *str);
int vsgParseStringN (vsgWorld *wrl, char const *str, size_t len);

vsgWorld * vsgNewWorld (symtab_t symtab);
void vsgDestroyWorld (vsgWorld *curr);

int vsgPrint (vsgWorld *curr, FILE *fp);
void vsgRender (vsgWorld *curr, void *closure);

void * vsgAlloc (vsgWorld *curr, int size);
void * vsgCalloc (vsgWorld *curr, int size);
char * vsgStrdup (vsgWorld *curr, char const *str);
symbol_t vsgIntern (vsgWorld *wrl, char const *str);

#define vsgOverride(id,func) vsgOverride1(id,(vsgFunc_t)(func))
#define vsgDefault(id,func) vsgDefault1(id,(vsgFunc_t)(func))

vsgFunc_t vsgOverride1 (vsgfId_t id, vsgFunc_t func);
vsgFunc_t vsgDefault1  (vsgfId_t id, vsgFunc_t func);

void vsgSetupDefaults (void);

char const *vsgWorldFiletype (vsgWorld *curr);
char const *vsgWorldFilename (vsgWorld *curr);

MFNode *vsgWorldNodes (vsgWorld *curr);
plist_t vsgWorldProperties (vsgWorld *curr);
MFProto *vsgWorldProtos (vsgWorld *curr);
MFNode *vsgWorldAppend (vsgWorld *curr, SFNode node);

/* Procedures on binding stacks. */
SFNode vsgWorldBackground (vsgWorld *curr);
void vsgBindBackground (vsgWorld *curr, SFNode node);
void vsgExtendBackgroundStack (vsgWorld *curr, SFNode node);

SFNode vsgWorldFog (vsgWorld *curr);
void vsgBindFog (vsgWorld *curr, SFNode node);
void vsgExtendFogStack (vsgWorld *curr, SFNode node);

SFNode vsgWorldNavigationInfo (vsgWorld *curr);
void vsgBindNavigationInfo (vsgWorld *curr, SFNode node);
void vsgExtendNavigationInfoStack (vsgWorld *curr, SFNode node);

SFNode vsgWorldViewpoint (vsgWorld *curr);
void vsgBindViewpoint (vsgWorld *curr, SFNode node);
void vsgExtendViewpointStack (vsgWorld *curr, SFNode node);

vsgClass const *vsgNodeClass (SFNode node);

vsgEventInDesc vsgSetFunc (SFNode node, symbol_t name);
vsgEventOutDesc vsgGetFunc (SFNode node, symbol_t name);
vsgEventInDesc vsgEventIn (vsgWorld *wrl, SFNode node, symbol_t name);
vsgEventOutDesc vsgEventOut (vsgWorld *wrl, SFNode node, symbol_t name);

SFNode vsgCreateNode (vsgWorld *wrl, vsgClass const *cl);
vsgClassField **vsgGetFields (vsgClass const *cl);
MFProto *vsgGetProtos (vsgClass const *cl);

SFNode vsgNewNode (vsgWorld *wrl, symbol_t name);

symbol_t vsgHasDEF (plist_t props, SFNode val);

vsgiFrame *vsgPushFrame (vsgWorld *curr, vsgClass *proto);
void vsgPopFrame (vsgWorld *curr);

MFColor    * vsgStartMFColor (vsgWorld *curr);
MFFloat    * vsgStartMFFloat (vsgWorld *curr);
MFInt32    * vsgStartMFInt32 (vsgWorld *curr);
MFNode     * vsgStartMFNode (vsgWorld *curr);
MFRotation * vsgStartMFRotation (vsgWorld *curr);
MFString   * vsgStartMFString (vsgWorld *curr);
MFVec2f    * vsgStartMFVec2f (vsgWorld *curr);
MFVec3f    * vsgStartMFVec3f (vsgWorld *curr);

MFColor    * vsgEndMFColor (vsgWorld *curr, MFColor *);
MFFloat    * vsgEndMFFloat (vsgWorld *curr, MFFloat *);
MFInt32    * vsgEndMFInt32 (vsgWorld *curr, MFInt32 *);
MFNode     * vsgEndMFNode (vsgWorld *curr, MFNode *);
MFRotation * vsgEndMFRotation (vsgWorld *curr, MFRotation *);
MFString   * vsgEndMFString (vsgWorld *curr, MFString *);
MFVec2f    * vsgEndMFVec2f (vsgWorld *curr, MFVec2f *);
MFVec3f    * vsgEndMFVec3f (vsgWorld *curr, MFVec3f *);

SFColor vsgMakeSFColor (SFFloat r, SFFloat g, SFFloat b);
SFRotation vsgMakeSFRotation (SFFloat a, SFFloat b, SFFloat c, SFFloat d);
SFVec2f vsgMakeSFVec2f (SFFloat v0, SFFloat v1);
SFVec3f vsgMakeSFVec3f (SFFloat v0, SFFloat v1, SFFloat v2);

MFColor * vsgAppendSFColor (vsgWorld *curr, MFColor *, SFColor c);
MFFloat * vsgAppendSFFloat (vsgWorld *curr, MFFloat *, SFFloat sffloat);
MFInt32 * vsgAppendSFInt32 (vsgWorld *curr, MFInt32 *, SFInt32 sfint32);
MFNode * vsgAppendSFNode (vsgWorld *curr, MFNode *, SFNode n);
MFRotation * vsgAppendSFRotation (vsgWorld *curr, MFRotation *, SFRotation r);
MFString * vsgAppendSFString (vsgWorld *curr, MFString *, SFString sfstr);
MFVec2f * vsgAppendSFVec2f (vsgWorld *curr, MFVec2f *, SFVec2f v);
MFVec3f * vsgAppendSFVec3f (vsgWorld *curr, MFVec3f *, SFVec3f v);

MFColor *    vsgMakeMFColor (vsgWorld *curr);
MFFloat *    vsgMakeMFFloat (vsgWorld *curr);
MFInt32 *    vsgMakeMFInt32 (vsgWorld *curr);
MFNode  *    vsgMakeMFNode (vsgWorld *curr);
MFRotation * vsgMakeMFRotation (vsgWorld *curr);
MFString *   vsgMakeMFString (vsgWorld *curr);
MFVec2f *    vsgMakeMFVec2f (vsgWorld *curr);
MFVec3f *    vsgMakeMFVec3f (vsgWorld *curr);

MFColor *    vsgCopyMFColor (vsgWorld *curr, MFColor *);
MFFloat *    vsgCopyMFFloat (vsgWorld *curr, MFFloat *);
MFInt32 *    vsgCopyMFInt32 (vsgWorld *curr, MFInt32 *);
MFNode  *    vsgCopyMFNode (vsgWorld *curr, MFNode  *);
MFRotation * vsgCopyMFRotation (vsgWorld *curr, MFRotation *);
MFString *   vsgCopyMFString (vsgWorld *curr, MFString *);
MFVec2f *    vsgCopyMFVec2f (vsgWorld *curr, MFVec2f *);
MFVec3f *    vsgCopyMFVec3f (vsgWorld *curr, MFVec3f *);

/* These functions are the actions taken by the parser. */
SFImage * vsgStartSFImage (vsgWorld *curr, int w, int h, int nplanes);
SFImage * vsgEndSFImage (vsgWorld *curr, SFImage *im);
void vsgAppendPixel (vsgWorld *curr, SFImage *im, int val);

void vsgPrintAnyArg (FILE *f, vsgDataType type, vsgAnyArg *a);

extern vsgClass const *vsgAnchorClass;
extern vsgClass const *vsgAppearanceClass;
extern vsgClass const *vsgAudioClipClass;
extern vsgClass const *vsgBackgroundClass;
extern vsgClass const *vsgBillboardClass;
extern vsgClass const *vsgBoxClass;
extern vsgClass const *vsgCollisionClass;
extern vsgClass const *vsgColorClass;
extern vsgClass const *vsgColorInterpolatorClass;
extern vsgClass const *vsgConeClass;
extern vsgClass const *vsgCoordinateClass;
extern vsgClass const *vsgCoordinateInterpolatorClass;
extern vsgClass const *vsgCylinderClass;
extern vsgClass const *vsgCylinderSensorClass;
extern vsgClass const *vsgDirectionalLightClass;
extern vsgClass const *vsgElevationGridClass;
extern vsgClass const *vsgExtrusionClass;
extern vsgClass const *vsgFogClass;
extern vsgClass const *vsgFontStyleClass;
extern vsgClass const *vsgGroupClass;
extern vsgClass const *vsgImageTextureClass;
extern vsgClass const *vsgIndexedFaceSetClass;
extern vsgClass const *vsgIndexedLineSetClass;
extern vsgClass const *vsgInlineClass;
extern vsgClass const *vsgLODClass;
extern vsgClass const *vsgMaterialClass;
extern vsgClass const *vsgMovieTextureClass;
extern vsgClass const *vsgNavigationInfoClass;
extern vsgClass const *vsgNormalClass;
extern vsgClass const *vsgNormalInterpolatorClass;
extern vsgClass const *vsgOrientationInterpolatorClass;
extern vsgClass const *vsgPixelTextureClass;
extern vsgClass const *vsgPlaneSensorClass;
extern vsgClass const *vsgPointLightClass;
extern vsgClass const *vsgPointSetClass;
extern vsgClass const *vsgPositionInterpolatorClass;
extern vsgClass const *vsgProximitySensorClass;
extern vsgClass const *vsgScalarInterpolatorClass;
extern vsgClass const *vsgScriptClass;
extern vsgClass const *vsgShapeClass;
extern vsgClass const *vsgSoundClass;
extern vsgClass const *vsgSphereClass;
extern vsgClass const *vsgSphereSensorClass;
extern vsgClass const *vsgSpotLightClass;
extern vsgClass const *vsgSwitchClass;
extern vsgClass const *vsgTextClass;
extern vsgClass const *vsgTextureCoordinateClass;
extern vsgClass const *vsgTextureTransformClass;
extern vsgClass const *vsgTimeSensorClass;
extern vsgClass const *vsgTouchSensorClass;
extern vsgClass const *vsgTransformClass;
extern vsgClass const *vsgViewpointClass;
extern vsgClass const *vsgVisibilitySensorClass;
extern vsgClass const *vsgWorldInfoClass;

int vsgSetAnchor_children (vsgWorld *curr, SFNode, MFNode * mfnode);
int vsgSetAnchor_description (vsgWorld *curr, SFNode, SFString description);
int vsgSetAnchor_parameter (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetAnchor_url (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetAnchor_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetAnchor_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetAppearance_material (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetAppearance_texture (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetAppearance_textureTransform (vsgWorld *curr, SFNode, SFNode sfnode);

int vsgSetAudioClip_description (vsgWorld *curr, SFNode, SFString desc);
int vsgSetAudioClip_loop (vsgWorld *curr, SFNode, SFBool loop);
int vsgSetAudioClip_pitch (vsgWorld *curr, SFNode, SFFloat pitch);
int vsgSetAudioClip_startTime (vsgWorld *curr, SFNode, SFTime t);
int vsgSetAudioClip_stoptIme (vsgWorld *curr, SFNode, SFTime t);
int vsgSetAudioClip_url (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetBackground_backUrl (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetBackground_bottomUrl (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetBackground_frontUrl (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetBackground_groundAngle (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetBackground_groundColor (vsgWorld *curr, SFNode, MFColor * mfcolor);
int vsgSetBackground_leftUrl (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetBackground_rightUrl (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetBackground_skyAngle (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetBackground_skyColor (vsgWorld *curr, SFNode, MFColor * mfcolor);
int vsgSetBackground_topUrl (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetBillboard_axisOfRotation (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetBillboard_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetBillboard_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetBillboard_children (vsgWorld *curr, SFNode, MFNode * mfnode);

int vsgSetBox_size (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetCollision_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetCollision_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetCollision_children (vsgWorld *curr, SFNode, MFNode * mfnode);
int vsgSetCollision_collide (vsgWorld *curr, SFNode, SFBool c);
int vsgSetCollision_proxy (vsgWorld *curr, SFNode, SFNode sfnode);

int vsgSetColorInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetColorInterpolator_keyValue (vsgWorld *curr, SFNode, MFColor * mfcolor);

int vsgSetColor_color (vsgWorld *curr, SFNode, MFColor * mfcolor);

int vsgSetCone_bottom (vsgWorld *curr, SFNode, SFBool b);
int vsgSetCone_bottomRadius (vsgWorld *curr, SFNode, SFFloat br);
int vsgSetCone_height (vsgWorld *curr, SFNode, SFFloat h);
int vsgSetCone_side (vsgWorld *curr, SFNode, SFBool s);

int vsgSetCoordinateInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetCoordinateInterpolator_keyValue (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetCoordinate_point (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetCylinderSensor_autoOffset (vsgWorld *curr, SFNode, SFBool ao);
int vsgSetCylinderSensor_diskAngle (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetCylinderSensor_enabled (vsgWorld *curr, SFNode, SFBool enabled);
int vsgSetCylinderSensor_maxAngle (vsgWorld *curr, SFNode, SFFloat maxang);
int vsgSetCylinderSensor_minAngle (vsgWorld *curr, SFNode, SFFloat minang);
int vsgSetCylinderSensor_offset (vsgWorld *curr, SFNode, SFFloat offset);

int vsgSetCylinder_bottom (vsgWorld *curr, SFNode, SFBool b);
int vsgSetCylinder_height (vsgWorld *curr, SFNode, SFFloat h);
int vsgSetCylinder_radius (vsgWorld *curr, SFNode, SFFloat r);
int vsgSetCylinder_side (vsgWorld *curr, SFNode, SFBool s);
int vsgSetCylinder_top (vsgWorld *curr, SFNode, SFBool t);

int vsgSetDirectionalLight_ambientIntensity (vsgWorld *curr, SFNode, SFFloat state);
int vsgSetDirectionalLight_color (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetDirectionalLight_direction (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetDirectionalLight_intensity (vsgWorld *curr, SFNode, SFFloat i);
int vsgSetDirectionalLight_on (vsgWorld *curr, SFNode, SFBool state);

int vsgSetElevationGrid_ccw (vsgWorld *curr, SFNode, SFBool ccw);
int vsgSetElevationGrid_color (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetElevationGrid_colorPerVertex (vsgWorld *curr, SFNode, SFBool cpv);
int vsgSetElevationGrid_creaseAngle (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetElevationGrid_height (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetElevationGrid_normal (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetElevationGrid_normalPerVertex (vsgWorld *curr, SFNode, SFBool npv);
int vsgSetElevationGrid_solid (vsgWorld *curr, SFNode, SFBool solid);
int vsgSetElevationGrid_texCoord (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetElevationGrid_xDimension (vsgWorld *curr, SFNode, SFInt32 xdim);
int vsgSetElevationGrid_xSpacing (vsgWorld *curr, SFNode, SFFloat xsp);
int vsgSetElevationGrid_zDimension (vsgWorld *curr, SFNode, SFInt32 zdim);
int vsgSetElevationGrid_zSpacing (vsgWorld *curr, SFNode, SFFloat zsp);

int vsgSetExtrusion_beginCap (vsgWorld *curr, SFNode, SFBool cap);
int vsgSetExtrusion_ccw (vsgWorld *curr, SFNode, SFBool ccw);
int vsgSetExtrusion_convex (vsgWorld *curr, SFNode, SFBool convexp);
int vsgSetExtrusion_creaseAngle (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetExtrusion_crossSection (vsgWorld *curr, SFNode, MFVec2f * mfvec2f);
int vsgSetExtrusion_endCap (vsgWorld *curr, SFNode, SFBool e);
int vsgSetExtrusion_orientation (vsgWorld *curr, SFNode, MFRotation * mfrotation);
int vsgSetExtrusion_scale (vsgWorld *curr, SFNode, MFVec2f * mfvec2f);
int vsgSetExtrusion_solid (vsgWorld *curr, SFNode, SFBool solidp);
int vsgSetExtrusion_spine (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetFog_color (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetFog_fogType (vsgWorld *curr, SFNode, SFString type);
int vsgSetFog_visibilityRange (vsgWorld *curr, SFNode, SFFloat range);

int vsgSetFontStyle_family (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetFontStyle_horizontal (vsgWorld *curr, SFNode, SFBool horizp);
int vsgSetFontStyle_justify (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetFontStyle_language (vsgWorld *curr, SFNode, SFString lang);
int vsgSetFontStyle_leftToRight (vsgWorld *curr, SFNode, SFBool lr);
int vsgSetFontStyle_size (vsgWorld *curr, SFNode, SFFloat size);
int vsgSetFontStyle_spacing (vsgWorld *curr, SFNode, SFFloat spacing);
int vsgSetFontStyle_style (vsgWorld *curr, SFNode, SFString style);
int vsgSetFontStyle_topToBottom (vsgWorld *curr, SFNode, SFBool tb);

int vsgSetGroup_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetGroup_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetGroup_children (vsgWorld *curr, SFNode, MFNode * mfnode);

int vsgSetImageTexture_repeatS (vsgWorld *curr, SFNode, SFBool rs);
int vsgSetImageTexture_repeatT (vsgWorld *curr, SFNode, SFBool rt);
int vsgSetImageTexture_url (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetIndexedFaceSet_ccw (vsgWorld *curr, SFNode, SFBool ccw);
int vsgSetIndexedFaceSet_color (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedFaceSet_colorIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);
int vsgSetIndexedFaceSet_colorPerVertex (vsgWorld *curr, SFNode, SFBool cpv);
int vsgSetIndexedFaceSet_convex (vsgWorld *curr, SFNode, SFBool convex);
int vsgSetIndexedFaceSet_coord (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedFaceSet_coordIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);
int vsgSetIndexedFaceSet_creaseAngle (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetIndexedFaceSet_normal (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedFaceSet_normalIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);
int vsgSetIndexedFaceSet_normalPerVertex (vsgWorld *curr, SFNode, SFBool npv);
int vsgSetIndexedFaceSet_solid (vsgWorld *curr, SFNode, SFBool solidp);
int vsgSetIndexedFaceSet_texCoord (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedFaceSet_texCoordIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);

int vsgSetIndexedLineSet_color (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedLineSet_colorIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);
int vsgSetIndexedLineSet_colorPerVertex (vsgWorld *curr, SFNode, SFBool sfbool);
int vsgSetIndexedLineSet_coord (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetIndexedLineSet_coordIndex (vsgWorld *curr, SFNode, MFInt32 * mfint32);

int vsgSetInline_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetInline_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetInline_url (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetLOD_center (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetLOD_level (vsgWorld *curr, SFNode, MFNode * mfnode);
int vsgSetLOD_range (vsgWorld *curr, SFNode, MFFloat * mffloat);

int vsgSetMaterial_ambientintensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetMaterial_diffuseColor (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetMaterial_emissiveColor (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetMaterial_shininess (vsgWorld *curr, SFNode, SFFloat shineola);
int vsgSetMaterial_specularcolor (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetMaterial_transparency (vsgWorld *curr, SFNode, SFFloat trans);

int vsgSetMovieTexture_loop (vsgWorld *curr, SFNode, SFBool loop);
int vsgSetMovieTexture_repeatS (vsgWorld *curr, SFNode, SFBool rs);
int vsgSetMovieTexture_repeatT (vsgWorld *curr, SFNode, SFBool rt);
int vsgSetMovieTexture_speed (vsgWorld *curr, SFNode, SFFloat speed);
int vsgSetMovieTexture_startTime (vsgWorld *curr, SFNode, SFTime t);
int vsgSetMovieTexture_stopTime (vsgWorld *curr, SFNode, SFTime t);
int vsgSetMovieTexture_url (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetNavigationInfo_avatarSize (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetNavigationInfo_headlight (vsgWorld *curr, SFNode, SFBool headlightp);
int vsgSetNavigationInfo_speed (vsgWorld *curr, SFNode, SFFloat speed);
int vsgSetNavigationInfo_type (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetNavigationInfo_visibilityLimit (vsgWorld *curr, SFNode, SFFloat vislim);

int vsgSetNormalInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetNormalInterpolator_keyValue (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetNormal_vector (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetOrientationInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetOrientationInterpolator_keyValue (vsgWorld *curr, SFNode, MFRotation * mfrotation);

int vsgSetPixelTexture_image (vsgWorld *curr, SFNode, SFImage sfimage);
int vsgSetPixelTexture_repeatS (vsgWorld *curr, SFNode, SFBool rs);
int vsgSetPixelTexture_repeatT (vsgWorld *curr, SFNode, SFBool rt);

int vsgSetPlaneSensor_autoOffset (vsgWorld *curr, SFNode, SFBool ao);
int vsgSetPlaneSensor_enabled (vsgWorld *curr, SFNode, SFBool enabled);
int vsgSetPlaneSensor_maxPosition (vsgWorld *curr, SFNode, SFVec2f sfvec2f);
int vsgSetPlaneSensor_minPosition (vsgWorld *curr, SFNode, SFVec2f sfvec2f);
int vsgSetPlaneSensor_offset (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetPointLight_ambientIntensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetPointLight_attenuation (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetPointLight_color (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetPointLight_intensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetPointLight_location (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetPointLight_on (vsgWorld *curr, SFNode, SFBool onp);
int vsgSetPointLight_radius (vsgWorld *curr, SFNode, SFFloat r);

int vsgSetPointSet_color (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetPointSet_coord (vsgWorld *curr, SFNode, SFNode sfnode);

int vsgSetPositionInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetPositionInterpolator_keyValue (vsgWorld *curr, SFNode, MFVec3f * mfvec3f);

int vsgSetProximitySensor_center (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetProximitySensor_enabled (vsgWorld *curr, SFNode, SFBool enabledp);
int vsgSetProximitySensor_size (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetScalarInterpolator_key (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetScalarInterpolator_keyValue (vsgWorld *curr, SFNode, MFFloat * mffloat);

int vsgSetScript_directOutput (vsgWorld *curr, SFNode, SFBool directp);
int vsgSetScript_mustEvaluate (vsgWorld *curr, SFNode, SFBool must_evalp);
int vsgSetScript_url (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetShape_appearance (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetShape_geometry (vsgWorld *curr, SFNode, SFNode sfnode);

int vsgSetSound_direction (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetSound_intensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetSound_location (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetSound_maxback (vsgWorld *curr, SFNode, SFFloat maxback);
int vsgSetSound_maxfront (vsgWorld *curr, SFNode, SFFloat maxfront);
int vsgSetSound_minback (vsgWorld *curr, SFNode, SFFloat minback);
int vsgSetSound_minfront (vsgWorld *curr, SFNode, SFFloat minfront);
int vsgSetSound_priority (vsgWorld *curr, SFNode, SFFloat priority);
int vsgSetSound_source (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetSound_spatialize (vsgWorld *curr, SFNode, SFBool spatialp);

int vsgSetSphereSensor_autoOffset (vsgWorld *curr, SFNode, SFBool ao);
int vsgSetSphereSensor_enabled (vsgWorld *curr, SFNode, SFBool enabledp);
int vsgSetSphereSensor_offset (vsgWorld *curr, SFNode, SFRotation sfrotation);

int vsgSetSphere_radius (vsgWorld *curr, SFNode, SFFloat r);

int vsgSetSpotLight_ambientintensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetSpotLight_attenuation (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetSpotLight_beamWidth (vsgWorld *curr, SFNode, SFFloat beamwidth);
int vsgSetSpotLight_color (vsgWorld *curr, SFNode, SFColor sfcolor);
int vsgSetSpotLight_cutOffAngle (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetSpotLight_direction (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetSpotLight_intensity (vsgWorld *curr, SFNode, SFFloat intensity);
int vsgSetSpotLight_location (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetSpotLight_on (vsgWorld *curr, SFNode, SFBool onp);
int vsgSetSpotLight_radius (vsgWorld *curr, SFNode, SFFloat r);

int vsgSetSwitch_choice (vsgWorld *curr, SFNode, MFNode * mfnode);
int vsgSetSwitch_whichChoice (vsgWorld *curr, SFNode, SFInt32 which);

int vsgSetText_fontStyle (vsgWorld *curr, SFNode, SFNode sfnode);
int vsgSetText_length (vsgWorld *curr, SFNode, MFFloat * mffloat);
int vsgSetText_maxExtent (vsgWorld *curr, SFNode, SFFloat extent);
int vsgSetText_string (vsgWorld *curr, SFNode, MFString * mfstring);

int vsgSetTextureCoordinate_point (vsgWorld *curr, SFNode, MFVec2f * mfvec2f);

int vsgSetTextureTransform_center (vsgWorld *curr, SFNode, SFVec2f sfvec2f);
int vsgSetTextureTransform_rotation (vsgWorld *curr, SFNode, SFFloat angle);
int vsgSetTextureTransform_scale (vsgWorld *curr, SFNode, SFVec2f sfvec2f);
int vsgSetTextureTransform_translation (vsgWorld *curr, SFNode, SFVec2f sfvec2f);

int vsgSetTimeSensor_cycleInterval (vsgWorld *curr, SFNode, SFTime interval);
int vsgSetTimeSensor_enabled (vsgWorld *curr, SFNode, SFBool enabledp);
int vsgSetTimeSensor_loop (vsgWorld *curr, SFNode, SFBool loop);
int vsgSetTimeSensor_startTime (vsgWorld *curr, SFNode, SFTime t);
int vsgSetTimeSensor_stopTime (vsgWorld *curr, SFNode, SFTime t);

int vsgSetTouchSensor_enabled (vsgWorld *curr, SFNode, SFBool enabledp);

int vsgSetTransform_bboxCenter (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetTransform_bboxSize (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetTransform_center (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetTransform_children (vsgWorld *curr, SFNode, MFNode * mfnode);
int vsgSetTransform_rotation (vsgWorld *curr, SFNode, SFRotation sfrotation);
int vsgSetTransform_scale (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetTransform_scaleOrientation (vsgWorld *curr, SFNode, SFRotation sfrotation);
int vsgSetTransform_translation (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetViewpoint_description (vsgWorld *curr, SFNode, SFString desc);

int vsgSetViewpoint_fieldOfView (vsgWorld *curr, SFNode, SFFloat fov);
int vsgSetViewpoint_jump (vsgWorld *curr, SFNode, SFBool jump);
int vsgSetViewpoint_orientation (vsgWorld *curr, SFNode, SFRotation sfrotation);
int vsgSetViewpoint_position (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetVisibilitySensor_center (vsgWorld *curr, SFNode, SFVec3f sfvec3f);
int vsgSetVisibilitySensor_enabled (vsgWorld *curr, SFNode, SFBool enabledp);
int vsgSetVisibilitySensor_size (vsgWorld *curr, SFNode, SFVec3f sfvec3f);

int vsgSetWorldInfo_info (vsgWorld *curr, SFNode, MFString * mfstring);
int vsgSetWorldInfo_title (vsgWorld *curr, SFNode, SFString string);

SFNode vsgNewAnchor (vsgWorld *curr);
SFNode vsgNewAppearance (vsgWorld *curr);
SFNode vsgNewAudioClip (vsgWorld *curr);
SFNode vsgNewBackground (vsgWorld *curr);
SFNode vsgNewBillboard (vsgWorld *curr);
SFNode vsgNewBox (vsgWorld *curr);
SFNode vsgNewCollision (vsgWorld *curr);
SFNode vsgNewColor (vsgWorld *curr);
SFNode vsgNewColorInterpolator (vsgWorld *curr);
SFNode vsgNewCone (vsgWorld *curr);
SFNode vsgNewCoordinate (vsgWorld *curr);
SFNode vsgNewCoordinateInterpolator (vsgWorld *curr);
SFNode vsgNewCylinder (vsgWorld *curr);
SFNode vsgNewCylinderSensor (vsgWorld *curr);
SFNode vsgNewDirectionalLight (vsgWorld *curr);
SFNode vsgNewElevationGrid (vsgWorld *curr);
SFNode vsgNewExtrusion (vsgWorld *curr);
SFNode vsgNewFog (vsgWorld *curr);
SFNode vsgNewFontStyle (vsgWorld *curr);
SFNode vsgNewGroup (vsgWorld *curr);
SFNode vsgNewImageTexture (vsgWorld *curr);
SFNode vsgNewIndexedFaceSet (vsgWorld *curr);
SFNode vsgNewIndexedLineSet (vsgWorld *curr);
SFNode vsgNewInline (vsgWorld *curr);
SFNode vsgNewLOD (vsgWorld *curr);
SFNode vsgNewMaterial (vsgWorld *curr);
SFNode vsgNewMovieTexture (vsgWorld *curr);
SFNode vsgNewNavigationInfo (vsgWorld *curr);
SFNode vsgNewNormal (vsgWorld *curr);
SFNode vsgNewNormalInterpolator (vsgWorld *curr);
SFNode vsgNewOrientationInterpolator (vsgWorld *curr);
SFNode vsgNewPixelTexture (vsgWorld *curr);
SFNode vsgNewPlaneSensor (vsgWorld *curr);
SFNode vsgNewPointLight (vsgWorld *curr);
SFNode vsgNewPointSet (vsgWorld *curr);
SFNode vsgNewPositionInterpolator (vsgWorld *curr);
SFNode vsgNewProximitySensor (vsgWorld *curr);
SFNode vsgNewScalarInterpolator (vsgWorld *curr);
SFNode vsgNewScript (vsgWorld *curr);
SFNode vsgNewShape (vsgWorld *curr);
SFNode vsgNewSound (vsgWorld *curr);
SFNode vsgNewSphere (vsgWorld *curr);
SFNode vsgNewSphereSensor (vsgWorld *curr);
SFNode vsgNewSpotLight (vsgWorld *curr);
SFNode vsgNewSwitch (vsgWorld *curr);
SFNode vsgNewText (vsgWorld *curr);
SFNode vsgNewTextureCoordinate (vsgWorld *curr);
SFNode vsgNewTextureTransform (vsgWorld *curr);
SFNode vsgNewTimeSensor (vsgWorld *curr);
SFNode vsgNewTouchSensor (vsgWorld *curr);
SFNode vsgNewTransform (vsgWorld *curr);
SFNode vsgNewPROTONode (vsgWorld *curr, symbol_t id);
SFNode vsgNewViewpoint (vsgWorld *curr);
SFNode vsgNewVisibilitySensor (vsgWorld *curr);
SFNode vsgNewWorldInfo (vsgWorld *curr);

void vsgDEF (vsgWorld *curr, symbol_t name, SFNode val);
SFNode vsgUSE (vsgWorld *curr, symbol_t name);

vsgClass *vsgStartProto (vsgWorld *curr, symbol_t id);
void vsgEndProtoFields (vsgWorld *curr);
vsgClass const * vsgEndProto (vsgWorld *curr);

vsgClass *vsgStartExternProto (vsgWorld *curr, symbol_t id);
vsgClass const * vsgEndExternProto (vsgWorld *curr);

void vsgDeclareField (vsgWorld *curr,
		      vsgFieldType ftype, vsgDataType type, symbol_t fname);
void vsgDeclareFieldDefault (vsgWorld *curr,
			     vsgDataType type, symbol_t fname, ...);
void vsgVaDeclareFieldDefault (vsgWorld *curr,
			       vsgDataType type, symbol_t fname, va_list ap);
void vsgDeclareFieldIS (vsgWorld *curr, symbol_t fname, symbol_t is_name);
void vsgDeclareExtern (vsgWorld *curr, MFString *extern_urls);
void vsgDeclareNodes (vsgWorld *curr, MFNode *nodes);

vsgClass *vsgStartScript (vsgWorld *curr);
vsgClass const * vsgEndScript (vsgWorld *curr);

void vsgSetIS (vsgWorld *curr,
	       SFNode node, symbol_t fieldId, symbol_t is_name);
vsgClassField * vsgHasIS (vsgWorld *curr, plist_t props,
			  SFNode node, symbol_t fieldId);

void vsgEndWorld (vsgWorld *curr);
void vsgStartWorld (vsgWorld *curr);

void vsgSetFilename (vsgWorld *curr, char const *fname);
void vsgSetFiletype (vsgWorld *curr, char const *ftype);

void vsgRegisterExtern (vsgWorld *curr, void *p,
			void (*detach)(vsgWorld *,void*));

int vsgdPrint (vsgWorld *wrl, FILE *fp);

#if defined(HAVE_INLINE) && !defined(NO_vrml2_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "vrml2il.h"
# undef exinline
#endif /* HAVE_INLINE */

#endif
