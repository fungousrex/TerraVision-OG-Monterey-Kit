/*
 * Name: c_nodesI.h
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
 *   $Id: c_nodesI.h,v 1.4 2000/08/28 23:21:32 reddy Exp $
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

#ifndef _C_NODESI_H_
#define _C_NODESI_H_

#include "c_nodes.h"

typedef struct {
  vsgDataType type:8;
  unsigned int offset:8;
  unsigned int blank:16;
} vsgcSlotComponents;

typedef union {
  vsgcSlotComponents s;
  struct {
    unsigned long val:32;
  } id;
} vsgcSlot;

typedef struct {
  symbol_t name;
  vsgcSlotComponents slot;
  void *dflt;
} vsgcSlotDescr_t;

typedef struct {
  vsgcSlotDescr_t *slots;
} vsgcNodeDescr_t;

typedef struct vsgcClass {
  vsgClass c;
  vsgClassField *fieldvec;
  vsgClassField **fields;
  void (*render) (vsgcNode *node, void *state);
} vsgcClass;

extern vsgcClass vsgcAnchorClass;
extern vsgcClass vsgcAppearanceClass;
extern vsgcClass vsgcAudioClipClass;
extern vsgcClass vsgcBackgroundClass;
extern vsgcClass vsgcBillboardClass;
extern vsgcClass vsgcBoxClass;
extern vsgcClass vsgcCollisionClass;
extern vsgcClass vsgcColorClass;
extern vsgcClass vsgcColorInterpolatorClass;
extern vsgcClass vsgcConeClass;
extern vsgcClass vsgcCoordinateClass;
extern vsgcClass vsgcCoordinateInterpolatorClass;
extern vsgcClass vsgcCylinderClass;
extern vsgcClass vsgcCylinderSensorClass;
extern vsgcClass vsgcDirectionalLightClass;
extern vsgcClass vsgcElevationGridClass;
extern vsgcClass vsgcExtrusionClass;
extern vsgcClass vsgcFogClass;
extern vsgcClass vsgcFontStyleClass;
extern vsgcClass vsgcGroupClass;
extern vsgcClass vsgcImageTextureClass;
extern vsgcClass vsgcIndexedFaceSetClass;
extern vsgcClass vsgcIndexedLineSetClass;
extern vsgcClass vsgcInlineClass;
extern vsgcClass vsgcLODClass;
extern vsgcClass vsgcMaterialClass;
extern vsgcClass vsgcMovieTextureClass;
extern vsgcClass vsgcNavigationInfoClass;
extern vsgcClass vsgcNormalClass;
extern vsgcClass vsgcNormalInterpolatorClass;
extern vsgcClass vsgcOrientationInterpolatorClass;
extern vsgcClass vsgcPixelTextureClass;
extern vsgcClass vsgcPlaneSensorClass;
extern vsgcClass vsgcPointLightClass;
extern vsgcClass vsgcPointSetClass;
extern vsgcClass vsgcPositionInterpolatorClass;
extern vsgcClass vsgcProximitySensorClass;
extern vsgcClass vsgcScalarInterpolatorClass;
extern vsgcClass vsgcScriptClass;
extern vsgcClass vsgcShapeClass;
extern vsgcClass vsgcSoundClass;
extern vsgcClass vsgcSphereClass;
extern vsgcClass vsgcSphereSensorClass;
extern vsgcClass vsgcSpotLightClass;
extern vsgcClass vsgcSwitchClass;
extern vsgcClass vsgcTextClass;
extern vsgcClass vsgcTextureCoordinateClass;
extern vsgcClass vsgcTextureTransformClass;
extern vsgcClass vsgcTimeSensorClass;
extern vsgcClass vsgcTouchSensorClass;
extern vsgcClass vsgcTransformClass;
extern vsgcClass vsgcViewpointClass;
extern vsgcClass vsgcVisibilitySensorClass;
extern vsgcClass vsgcWorldInfoClass;

#endif

