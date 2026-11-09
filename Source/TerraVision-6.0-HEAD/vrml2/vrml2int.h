/*
 * Name: vrml2int.h
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
 *   $Id: vrml2int.h,v 1.13 2000/11/10 18:51:05 leei Exp $
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

#ifndef _vrml2int_h_
#define _vrml2int_h_

#ifdef __cplusplus
extern "C" {
#endif

#if !( defined(WIN32) && defined(MSVC) )
#include "config.h"
#endif

#include <stdio.h>
#include <symtab.h>
#include <plist.h>

#include <pthr/istack.h>

#ifdef __cplusplus
}
#endif

#include "vrml2.h"

#if YYDEBUG
extern int vsgpDebug;
#endif

typedef struct vsgpLexer vsgpLexer;

struct vsgiFrame {
  vsgiFrame *prev;

  vsgProtoClass *proto;
  MFNode *nodes;
  MFProto *protos;
  plist_t props;

  vsgRouteLink *routes;
};

#define vsgiCurrNodes(curr) ((curr)->frame->nodes)
#define vsgiCurrProto(curr) ((curr)->frame->proto)
#define vsgiCurrProtos(curr) ((curr)->frame->protos)
#define vsgiCurrProps(curr) ((curr)->frame->props)
#define vsgiCurrRoutes(curr) ((curr)->frame->routes)

typedef struct vsgiBindingStack vsgiBindingStack;
struct vsgiBindingStack {
  SFNode node;
  vsgiBindingStack *next;
};

#define NUM_STACKS (5)

struct vsgWorld {
  vsgpLexer *lexer;

  const char *filename;
  const char *filetype;

  vsgiFrame *frame;

  istack *stacks[NUM_STACKS];
  istack *data;
  int data_i;
  int data_busy;

  void *externs;

  vsgiBindingStack *backgroundStack;
  vsgiBindingStack *fogStack;
  vsgiBindingStack *navigationInfoStack;
  vsgiBindingStack *viewpointStack;

  symtab_t symtab;
  int symtab_localp;
};

void vsglDestroy (vsgpLexer *l);

int vsgldebug (vsgpLexer *l, int dbg);

void vsglPop (vsgWorld* wrl);
void vsglPushMeta (vsgWorld* wrl);
void vsglPushProto (vsgWorld* wrl);
void vsglPushNormal (vsgWorld* wrl);

int vsglLinenum (vsgWorld* wrl);

void vsgError (vsgWorld *p, char const *fmt, ...);

/*
 * The following structure and flags represent the internal form of the MF*
 * node.  The flags reflect various states of the object:
 *	MFGEN_ISTACK:  The data is on an istack (specified by u.stack)
 *	MFGEN_PARTIAL: StartMF* was used to create, but EndMF* not yet called.
 *	MFGEN_MALLOC:  The data has been malloced.
 */

#define MFGEN_ISTACK   (1<<0)
#define MFGEN_PARTIAL  (1<<1)
#define MFGEN_MALLOC   (1<<2)

typedef struct {
  MFGeneric gen;
  union {
    istack *stack;
    int size;
  } u;
} MFGenericI;

int vsgiCheckStack (vsgWorld *curr);
int vsgiClaimStack (vsgWorld *curr);
void vsgiReleaseStack (vsgWorld *curr, istack *ob);

vsgEventInDesc vsgiGet_setFunc (vsgClass const *cl, symbol_t name);
vsgEventOutDesc vsgiGet_getFunc (vsgClass const *cl, symbol_t name);
vsgEventInDesc vsgiGet_eventIn (vsgClass const *cl, symbol_t name);
vsgEventOutDesc vsgiGet_eventOut (vsgClass const *cl, symbol_t name);

void vsgiSetAnchorChildren (vsgWorld *curr, MFNode * mfnode);
void vsgiSetAnchorDescription (vsgWorld *curr, SFString description);
void vsgiSetAnchorParameter (vsgWorld *curr, MFString * mfstring);
void vsgiSetAnchorURL (vsgWorld *curr, MFString * mfstring);
void vsgiSetAnchorBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetAnchorBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetAppearanceMaterial (vsgWorld *curr, SFNode * sfnode);
void vsgiSetAppearanceTexture (vsgWorld *curr, SFNode * sfnode);
void vsgiSetAppearanceTextureTransform (vsgWorld *curr, SFNode * sfnode);

void vsgiSetAudioClipDescription (vsgWorld *curr, SFString desc);
void vsgiSetAudioClipLoop (vsgWorld *curr, SFBool loop);
void vsgiSetAudioClipPitch (vsgWorld *curr, SFFloat *pitch);
void vsgiSetAudioClipStarttime (vsgWorld *curr, SFTime *t);
void vsgiSetAudioClipStoptime (vsgWorld *curr, SFTime *t);
void vsgiSetAudioClipUrl (vsgWorld *curr, MFString * mfstring);

void vsgiSetBackgroundBackurl (vsgWorld *curr, MFString * mfstring);
void vsgiSetBackgroundBottomurl (vsgWorld *curr, MFString * mfstring);
void vsgiSetBackgroundFronturl (vsgWorld *curr, MFString * mfstring);
void vsgiSetBackgroundGroundangle (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetBackgroundGroundcolor (vsgWorld *curr, MFColor * mfcolor);
void vsgiSetBackgroundLefturl (vsgWorld *curr, MFString * mfstring);
void vsgiSetBackgroundRighturl (vsgWorld *curr, MFString * mfstring);
void vsgiSetBackgroundSkyangle (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetBackgroundSkycolor (vsgWorld *curr, MFColor * mfcolor);
void vsgiSetBackgroundTopurl (vsgWorld *curr, MFString * mfstring);

void vsgiSetBillboardAxisofrotation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetBillboardBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetBillboardBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetBillboardChildren (vsgWorld *curr, MFNode * mfnode);

void vsgiSetBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetCollisionBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetCollisionBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetCollisionChildren (vsgWorld *curr, MFNode * mfnode);
void vsgiSetCollisionCollide (vsgWorld *curr, SFBool c);
void vsgiSetCollisionProxy (vsgWorld *curr, SFNode * sfnode);

void vsgiSetColorInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetColorInterpolatorKeyvalue (vsgWorld *curr, MFColor * mfcolor);

void vsgiSetColorColor (vsgWorld *curr, MFColor * mfcolor);

void vsgiSetConeBottom (vsgWorld *curr, SFBool b);
void vsgiSetConeBottomradius (vsgWorld *curr, SFFloat *br);
void vsgiSetConeHeight (vsgWorld *curr, SFFloat *h);
void vsgiSetConeSide (vsgWorld *curr, SFBool s);

void vsgiSetCoordinateInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetCoordinateInterpolatorKeyvalue (vsgWorld *curr, MFVec3f * mfvec3f);
void vsgiSetCoordinatePoint (vsgWorld *curr, MFVec3f * mfvec3f);

void vsgiSetCylinderSensorAutooffset (vsgWorld *curr, SFBool ao);
void vsgiSetCylinderSensorDiskangle (vsgWorld *curr, SFFloat *angle);
void vsgiSetCylinderSensorEnabled (vsgWorld *curr, SFBool enabled);
void vsgiSetCylinderSensorMaxangle (vsgWorld *curr, SFFloat *maxang);
void vsgiSetCylinderSensorMinangle (vsgWorld *curr, SFFloat *minang);
void vsgiSetCylinderSensorOffset (vsgWorld *curr, SFFloat *offset);

void vsgiSetCylinderBottom (vsgWorld *curr, SFBool b);
void vsgiSetCylinderHeight (vsgWorld *curr, SFFloat *h);
void vsgiSetCylinderRadius (vsgWorld *curr, SFFloat *r);
void vsgiSetCylinderSide (vsgWorld *curr, SFBool s);
void vsgiSetCylinderTop (vsgWorld *curr, SFBool t);

void vsgiSetDirectionalLightAmbient (vsgWorld *curr, SFFloat *state);
void vsgiSetDirectionalLightColor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetDirectionalLightDirection (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetDirectionalLightIntensity (vsgWorld *curr, SFFloat *i);
void vsgiSetDirectionalLightOn (vsgWorld *curr, SFBool state);

void vsgiSetElevationGridCcw (vsgWorld *curr, SFBool ccw);
void vsgiSetElevationGridColor (vsgWorld *curr, SFNode * sfnode);
void vsgiSetElevationGridColorpervertex (vsgWorld *curr, SFBool cpv);
void vsgiSetElevationGridCreaseangle (vsgWorld *curr, SFFloat *angle);
void vsgiSetElevationGridHeight (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetElevationGridNormal (vsgWorld *curr, SFNode * sfnode);
void vsgiSetElevationGridNormalpervertex (vsgWorld *curr, SFBool npv);
void vsgiSetElevationGridSolid (vsgWorld *curr, SFBool solid);
void vsgiSetElevationGridTexcoord (vsgWorld *curr, SFNode * sfnode);
void vsgiSetElevationGridXdimension (vsgWorld *curr, SFInt32 xdim);
void vsgiSetElevationGridXspacing (vsgWorld *curr, SFFloat *xsp);
void vsgiSetElevationGridZdimension (vsgWorld *curr, SFInt32 zdim);
void vsgiSetElevationGridZspacing (vsgWorld *curr, SFFloat *zsp);

void vsgiSetExtrusionBegincap (vsgWorld *curr, SFBool b);
void vsgiSetExtrusionCcw (vsgWorld *curr, SFBool ccw);
void vsgiSetExtrusionConvex (vsgWorld *curr, SFBool convexp);
void vsgiSetExtrusionCreaseangle (vsgWorld *curr, SFFloat *angle);
void vsgiSetExtrusionCrosssection (vsgWorld *curr, MFVec2f * mfvec2f);
void vsgiSetExtrusionEndcap (vsgWorld *curr, SFBool e);
void vsgiSetExtrusionOrientation (vsgWorld *curr, MFRotation * mfrotation);
void vsgiSetExtrusionScale (vsgWorld *curr, MFVec2f * mfvec2f);
void vsgiSetExtrusionSolid (vsgWorld *curr, SFBool solidp);
void vsgiSetExtrusionSpine (vsgWorld *curr, MFVec3f * mfvec3f);

void vsgiSetFogColor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetFogFogtype (vsgWorld *curr, SFString type);
void vsgiSetFogVisibilityrange (vsgWorld *curr, SFFloat *range);

void vsgiSetFontStyleFamily (vsgWorld *curr, MFString * mfstring);
void vsgiSetFontStyleHorizontal (vsgWorld *curr, SFBool horizp);
void vsgiSetFontStyleJustify (vsgWorld *curr, MFString * mfstring);
void vsgiSetFontStyleLanguage (vsgWorld *curr, SFString lang);
void vsgiSetFontStyleLefttoright (vsgWorld *curr, SFBool lr);
void vsgiSetFontStyleSize (vsgWorld *curr, SFFloat *size);
void vsgiSetFontStyleSpacing (vsgWorld *curr, SFFloat *spacing);
void vsgiSetFontStyleStyle (vsgWorld *curr, SFString style);
void vsgiSetFontStyleToptobottom (vsgWorld *curr, SFBool tb);

void vsgiSetGroupBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetGroupBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetGroupChildren (vsgWorld *curr, MFNode * mfnode);

void vsgiSetImageTextureRepeats (vsgWorld *curr, SFBool rs);
void vsgiSetImageTextureRepeatt (vsgWorld *curr, SFBool rt);
void vsgiSetImageTextureUrl (vsgWorld *curr, MFString * mfstring);

void vsgiSetIndexedFaceSetCcw (vsgWorld *curr, SFBool ccw);
void vsgiSetIndexedFaceSetColor (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedFaceSetColorindex (vsgWorld *curr, MFInt32 * mfint32);
void vsgiSetIndexedFaceSetColorpervertex (vsgWorld *curr, SFBool cpv);
void vsgiSetIndexedFaceSetConvex (vsgWorld *curr, SFBool convex);
void vsgiSetIndexedFaceSetCoord (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedFaceSetCoordindex (vsgWorld *curr, MFInt32 * mfint32);
void vsgiSetIndexedFaceSetCreaseangle (vsgWorld *curr, SFFloat *angle);
void vsgiSetIndexedFaceSetNormal (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedFaceSetNormalindex (vsgWorld *curr, MFInt32 * mfint32);
void vsgiSetIndexedFaceSetNormalpervertex (vsgWorld *curr, SFBool npv);
void vsgiSetIndexedFaceSetSolid (vsgWorld *curr, SFBool solidp);
void vsgiSetIndexedFaceSetTexcoord (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedFaceSetTexcoordindex (vsgWorld *curr, MFInt32 * mfint32);

void vsgiSetIndexedLineSetColor (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedLineSetColorindex (vsgWorld *curr, MFInt32 * mfint32);
void vsgiSetIndexedLineSetColorpervertex (vsgWorld *curr, SFBool sfbool);
void vsgiSetIndexedLineSetCoord (vsgWorld *curr, SFNode * sfnode);
void vsgiSetIndexedLineSetCoordindex (vsgWorld *curr, MFInt32 * mfint32);

void vsgiSetInlineBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetInlineBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetInlineUrl (vsgWorld *curr, MFString * mfstring);

void vsgiSetLODCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetLODLevel (vsgWorld *curr, MFNode * mfnode);
void vsgiSetLODRange (vsgWorld *curr, MFFloat * mffloat);

void vsgiSetMaterialAmbientintensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetMaterialDiffusecolor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetMaterialEmissivecolor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetMaterialShininess (vsgWorld *curr, SFFloat *shineola);
void vsgiSetMaterialSpecularcolor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetMaterialTransparency (vsgWorld *curr, SFFloat *trans);

void vsgiSetMovieTextureLoop (vsgWorld *curr, SFBool loop);
void vsgiSetMovieTextureRepeats (vsgWorld *curr, SFBool rs);
void vsgiSetMovieTextureRepeatt (vsgWorld *curr, SFBool rt);
void vsgiSetMovieTextureSpeed (vsgWorld *curr, SFFloat *speed);
void vsgiSetMovieTextureStarttime (vsgWorld *curr, SFTime *t);
void vsgiSetMovieTextureStoptime (vsgWorld *curr, SFTime *t);
void vsgiSetMovieTextureUrl (vsgWorld *curr, MFString * mfstring);

void vsgiSetNavigationInfoAvatarsize (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetNavigationInfoHeadlight (vsgWorld *curr, SFBool headlightp);
void vsgiSetNavigationInfoSpeed (vsgWorld *curr, SFFloat *speed);
void vsgiSetNavigationInfoType (vsgWorld *curr, MFString * mfstring);
void vsgiSetNavigationInfoVisibilitylimit (vsgWorld *curr, SFFloat *vislim);

void vsgiSetNormalInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetNormalInterpolatorKeyvalue (vsgWorld *curr, MFVec3f * mfvec3f);

void vsgiSetNormalVector (vsgWorld *curr, MFVec3f * mfvec3f);

void vsgiSetOrientationInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetOrientationInterpolatorKeyvalue (vsgWorld *curr, MFRotation * mfrotation);

void vsgiSetPixelTextureImage (vsgWorld *curr, SFImage * sfimage);
void vsgiSetPixelTextureRepeats (vsgWorld *curr, SFBool rs);
void vsgiSetPixelTextureRepeatt (vsgWorld *curr, SFBool rt);

void vsgiSetPlaneSensorAutooffset (vsgWorld *curr, SFBool ao);
void vsgiSetPlaneSensorEnabled (vsgWorld *curr, SFBool enabled);
void vsgiSetPlaneSensorMaxposition (vsgWorld *curr, SFVec2f * sfvec2f);
void vsgiSetPlaneSensorMinposition (vsgWorld *curr, SFVec2f * sfvec2f);
void vsgiSetPlaneSensorOffset (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetPointLightAmbientintensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetPointLightAttenuation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetPointLightColor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetPointLightIntensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetPointLightLocation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetPointLightOn (vsgWorld *curr, SFBool onp);
void vsgiSetPointLightRadius (vsgWorld *curr, SFFloat *r);

void vsgiSetPointSetColor (vsgWorld *curr, SFNode * sfnode);
void vsgiSetPointSetCoord (vsgWorld *curr, SFNode * sfnode);

void vsgiSetPositionInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetPositionInterpolatorKeyvalue (vsgWorld *curr, MFVec3f * mfvec3f);

void vsgiSetProximitySensorCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetProximitySensorEnabled (vsgWorld *curr, SFBool enabledp);
void vsgiSetProximitySensorSize (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetScalarInterpolatorKey (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetScalarInterpolatorKeyvalue (vsgWorld *curr, MFFloat * mffloat);

void vsgiSetScriptDirectoutput (vsgWorld *curr, SFBool directp);
void vsgiSetScriptMustevaluate (vsgWorld *curr, SFBool must_evalp);
void vsgiSetScriptUrl (vsgWorld *curr, MFString * mfstring);

void vsgiSetShapeAppearance (vsgWorld *curr, SFNode * sfnode);
void vsgiSetShapeGeometry (vsgWorld *curr, SFNode * sfnode);

void vsgiSetSoundDirection (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetSoundIntensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetSoundLocation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetSoundMaxback (vsgWorld *curr, SFFloat *maxback);
void vsgiSetSoundMaxfront (vsgWorld *curr, SFFloat *maxfront);
void vsgiSetSoundMinback (vsgWorld *curr, SFFloat *minback);
void vsgiSetSoundMinfront (vsgWorld *curr, SFFloat *minfront);
void vsgiSetSoundPriority (vsgWorld *curr, SFFloat *priority);
void vsgiSetSoundSource (vsgWorld *curr, SFNode * sfnode);
void vsgiSetSoundSpatialize (vsgWorld *curr, SFBool spatialp);

void vsgiSetSphereSensorAutooffset (vsgWorld *curr, SFBool ao);
void vsgiSetSphereSensorEnabled (vsgWorld *curr, SFBool enabledp);
void vsgiSetSphereSensorOffset (vsgWorld *curr, SFRotation * sfrotation);

void vsgiSetSphereRadius (vsgWorld *curr, SFFloat *r);

void vsgiSetSpotLightAmbientintensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetSpotLightAttenuation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetSpotLightBeamwidth (vsgWorld *curr, SFFloat *beamwidth);
void vsgiSetSpotLightColor (vsgWorld *curr, SFColor * sfcolor);
void vsgiSetSpotLightCutOffangle (vsgWorld *curr, SFFloat *angle);
void vsgiSetSpotLightDirection (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetSpotLightIntensity (vsgWorld *curr, SFFloat *intensity);
void vsgiSetSpotLightLocation (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetSpotLightOn (vsgWorld *curr, SFBool onp);
void vsgiSetSpotLightRadius (vsgWorld *curr, SFFloat *r);

void vsgiSetSwitchChoice (vsgWorld *curr, MFNode * mfnode);
void vsgiSetSwitchWhichchoice (vsgWorld *curr, SFInt32 which);

void vsgiSetTextFontstyle (vsgWorld *curr, SFNode * sfnode);
void vsgiSetTextLength (vsgWorld *curr, MFFloat * mffloat);
void vsgiSetTextMaxextent (vsgWorld *curr, SFFloat *extent);
void vsgiSetTextString (vsgWorld *curr, MFString * mfstring);

void vsgiSetTextureCoordPoint (vsgWorld *curr, MFVec2f * mfvec2f);

void vsgiSetTextureTransformCenter (vsgWorld *curr, SFVec2f * sfvec2f);
void vsgiSetTextureTransformRotation (vsgWorld *curr, SFFloat *angle);
void vsgiSetTextureTransformScale (vsgWorld *curr, SFVec2f * sfvec2f);
void vsgiSetTextureTransformTranslation (vsgWorld *curr, SFVec2f * sfvec2f);

void vsgiSetTimeSensorCycleinterval (vsgWorld *curr, SFTime *interval);
void vsgiSetTimeSensorEnabled (vsgWorld *curr, SFBool enabledp);
void vsgiSetTimeSensorLoop (vsgWorld *curr, SFBool loop);
void vsgiSetTimeSensorStarttime (vsgWorld *curr, SFTime *t);
void vsgiSetTimeSensorStoptime (vsgWorld *curr, SFTime *t);

void vsgiSetTouchSensorEnabled (vsgWorld *curr, SFBool enabledp);

void vsgiSetTransformBBoxCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetTransformBBoxSize (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetTransformCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetTransformChildren (vsgWorld *curr, MFNode * mfnode);
void vsgiSetTransformRotation (vsgWorld *curr, SFRotation * sfrotation);
void vsgiSetTransformScale (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetTransformScaleorientation (vsgWorld *curr, SFRotation * sfrotation);
void vsgiSetTransformTranslation (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetViewpointDescription (vsgWorld *curr, SFString desc);

void vsgiSetViewpointFieldofview (vsgWorld *curr, SFFloat *fov);
void vsgiSetViewpointJump (vsgWorld *curr, SFBool jump);
void vsgiSetViewpointOrientation (vsgWorld *curr, SFRotation * sfrotation);
void vsgiSetViewpointPosition (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetVisibilitySensorCenter (vsgWorld *curr, SFVec3f * sfvec3f);
void vsgiSetVisibilitySensorEnabled (vsgWorld *curr, SFBool enabledp);
void vsgiSetVisibilitySensorSize (vsgWorld *curr, SFVec3f * sfvec3f);

void vsgiSetWorldInfoInfo (vsgWorld *curr, MFString * mfstring);
void vsgiSetWorldInfoTitle (vsgWorld *curr, SFString string);

void vsgiStartSFImage (vsgWorld *curr, int w, int h, int nplanes,
		       int v1, int v2);

#ifdef __cplusplus
}
#endif

#endif /* _vrml2int_h_ */
