/*
  tvViewCmds.c
 
  Implementation of the tvBase "view" subcommand.
 

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
 
  Contributor(s): Rick Brownrigg, Martin Reddy, Lee Iverson, Yvan Leclerc.
  ----- END LICENSE BLOCK -----

*/

#include <tcl.h>
#include "ThreeDCore.h"
#include "OAA.h"
#include "ThreeDCoreI.h"  /* BUSTED: needed to pick up "tvBaseView()" */
#include "togl.h"
#include "tvBaseCmdsP.h"

/* forward decls... */
int currentCmd(tvBase*, struct Togl*, int argc, char* argv[]);
int adjustCmd(tvBase*, struct Togl*, int argc, char* argv[]);
int setViewRelativeCmd(tvBase*, struct Togl*, int argc, char* argv[]);
int setViewCmd(tvBase*, struct Togl*, int argc, char* argv[]);

Tcl_Obj* tvView2TclView(tvViewpoint* tvView);
int tclView2TvView(Tcl_Obj* tclView, tvViewpoint* tvView);


/*
 * t v V i e w C o m m a n d ( )
 *
 * This is the principle external interface to the view subcommand.  Is 
 * installed as a subcommand on the Togl widget.
 *
 */
int 
tvViewCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);

    if (argc < 3) {
        goto USAGE;
    }

    if (!strcmp(argv[2], "current")) {
        return currentCmd(tvPacket->base, togl, argc, argv);
    }
    else if (!strcmp(argv[2], "accelerate")) {
        if (argc != 3) goto USAGE;
        tvStartAccelerate(tvPacket->base);
    }
    else if (!strcmp(argv[2], "deccelerate")) {
        if (argc != 3) goto USAGE;
        tvStartDeccelerate(tvPacket->base);
    }
    else if (!strcmp(argv[2], "startMovement")) {
        if (argc != 3) goto USAGE;
        tvStartMovement(tvPacket->base);
    }
    else if (!strcmp(argv[2], "stopMovement")) {
        if (argc != 3) goto USAGE;
        tvStopMovement(tvPacket->base);
    }
    else if (!strcmp(argv[2], "origin")) {
	tvBaseInitView(tvPacket->base, tvBaseTop(tvPacket->base));
    }
    else if (!strcmp(argv[2], "adjust")) {
        return adjustCmd(tvPacket->base, togl, argc, argv);
    }
    else if (!strcmp(argv[2], "rotate") || !strcmp(argv[2], "move")) {
        return setViewRelativeCmd(tvPacket->base, togl, argc, argv);
    }
    else if (!strcmp(argv[2], "flyto") || !strcmp(argv[2], "set3D") ||
        !strcmp(argv[2], "flytoPick")) 
    {
        return setViewCmd(tvPacket->base, togl, argc, argv);
    }
    else if (!strcmp(argv[2], "setFrozen")) {
        tvBaseSetFrozenView(tvPacket->base);
    }
    else if (!strcmp(argv[2], "setgdc")) {
        return setGDCCommand(tvPacket->base, togl, argc, argv);
    }
    else if (!strcmp(argv[2], "getgdc")) {
        return getGDCCommand(tvPacket->base, togl, argc, argv);
    }
    else
        goto USAGE;

    return TCL_OK;

 USAGE:
    // eventually, make this more useful...
    return TCL_ERROR;
}


/*
 * c u r r e n t C m d ( )
 *
 * Processes the "view current" subcommand.
 *
 */
int
currentCmd(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    static tvViewpoint tvView;
    int jumpFlag;
    Tcl_Obj* tclView;
    Tcl_Interp* interp = Togl_Interp(togl);


    /* if no arguments, merely return the current view... */
    if (argc == 3) {
        Tcl_Obj* result;
        tvViewpoint* tvView = tvBaseView(base);
        result = tvView2TclView(tvView);
        Tcl_SetObjResult(interp, result);
        return TCL_OK;
    }

    /* else, we are setting the view (with optional jump)... */
    if (argc < 4 || argc > 5) 
        /* NEED BETTER ERROR MESSAGING.. */
        return TCL_ERROR;

    if (argc == 5) {
        if (Tcl_GetBoolean(interp, argv[4], &jumpFlag) != TCL_OK)
            return TCL_ERROR;
    }
    else
        jumpFlag = 0;

    tclView = Tcl_NewStringObj(argv[3], -1);
    if (tclView2TvView(tclView, &tvView) != TCL_OK) 
        return TCL_ERROR;

    tvSetView(base, &tvView, jumpFlag);

    Tcl_DecrRefCount(tclView);   /* CORRECT MEM MGMT?   */
    return TCL_OK;
}


/*
 * a d j u s t C m d ( )
 *
 * Processes the "view adjust" subcommand.
 *
 */
int
adjustCmd(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    tvViewpoint tvView;
    double forw, right, up;
    Tcl_Obj* tclView;
    Tcl_Interp* interp = Togl_Interp(togl);

    if (argc != 7) return TCL_ERROR;

    tclView = Tcl_NewStringObj(argv[3], -1);
    if (tclView2TvView(tclView, &tvView) != TCL_OK) return TCL_ERROR;
    if (Tcl_GetDouble(interp, argv[4], &forw) != TCL_OK  ||
        Tcl_GetDouble(interp, argv[5], &right) != TCL_OK  ||
        Tcl_GetDouble(interp, argv[6], &up) ) 
    {
        return TCL_ERROR;
    }

    tvAdjustView(base, &tvView, forw, right, up);

    /* convert resultant tvViewpoint back to TCL version and return it */
    Tcl_DecrRefCount(tclView);   /* free prev.   CORRECT MEM MGMT?   */
    tclView = tvView2TclView(&tvView);

    Tcl_SetObjResult(interp, tclView);
    return TCL_OK;
}

/*
 * s e t V i e w R e l a t i v e C m d  ( )
 *
 * Processes the "relative: set view commands: rotate and move.
 *
 */
int
setViewRelativeCmd(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    double yawOrXDelta, pitchOrYDelta, rollOrZDelta;
    Tcl_Interp* interp = Togl_Interp(togl);

    if (argc != 6) goto USAGE;

    /* pull out the arguments... */
    if (Tcl_GetDouble(interp, argv[3], &yawOrXDelta)   != TCL_OK ||
        Tcl_GetDouble(interp, argv[4], &pitchOrYDelta) != TCL_OK ||
        Tcl_GetDouble(interp, argv[5], &rollOrZDelta)  != TCL_OK)
    {
        return TCL_ERROR;
    }

    if (!strcmp(argv[2], "move")) {
        tvMoveViewpoint(base, yawOrXDelta, pitchOrYDelta, rollOrZDelta, FALSE);
    }
    else if (!strcmp(argv[2], "rotate")) {
        tvRotate(base, yawOrXDelta, pitchOrYDelta, rollOrZDelta, FALSE);
    }
    else 
        return TCL_ERROR;

    return TCL_OK;

 USAGE:
    return TCL_ERROR;
}


/*
 * s e t V i e w C m d ( )
 *
 * Processes the various set view subcommands, such as "flyto", "set3D",...
 *
 */
int
setViewCmd(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    double x, y, z;
    double yaw, pitch, roll;
    Tcl_Interp* interp = Togl_Interp(togl);

    if (argc == 5 && !strcmp(argv[2], "flytoPick")) {
        int ix, iy;
        if (Tcl_GetInt(interp, argv[3], &ix) != TCL_OK ||
            Tcl_GetInt(interp, argv[4], &iy) != TCL_OK)
        {
            return TCL_ERROR;
        }

        tvFlyToPickPoint(base, ix, iy);
        return TCL_OK;
    }

    if (argc != 9) goto USAGE;

    /* pull out the arguments... */
    if (Tcl_GetDouble(interp, argv[3], &x) != TCL_OK ||
        Tcl_GetDouble(interp, argv[4], &y) != TCL_OK ||
        Tcl_GetDouble(interp, argv[5], &z) != TCL_OK ||
        Tcl_GetDouble(interp, argv[6], &yaw) != TCL_OK ||
        Tcl_GetDouble(interp, argv[7], &pitch) != TCL_OK ||
        Tcl_GetDouble(interp, argv[8], &roll) != TCL_OK)
    {
        return TCL_ERROR;
    }

    if (!strcmp(argv[2], "flyto")) {
        tvFlyTo(base, x, y, z, yaw, pitch, roll);
    }
    else if (!strcmp(argv[2], "set3D")) {
        tvSetView3d(base, x, y, z, yaw, pitch, roll);
    }
    else 
        return TCL_ERROR;

    return TCL_OK;

 USAGE:
    return TCL_ERROR;
}

/*
 * s e t G D C C o m m a n d ( )
 *
 * Sets the view to a specificed long, lat, elev, yaw, pitch, roll
 * where lat, long, yaw, pitch, roll are in degrees, and elev is in meters
 *
 */
int
setGDCCommand(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    double x, y, z, yaw, pitch, roll;
    Tcl_Interp* interp = Togl_Interp(togl);

    if ( argc != 9 ) return TCL_ERROR;

    /* pull out the arguments... */

    if ( Tcl_GetDouble(interp, argv[3], &x) != TCL_OK ||
	 Tcl_GetDouble(interp, argv[4], &y) != TCL_OK ||
	 Tcl_GetDouble(interp, argv[5], &z) != TCL_OK ||
	 Tcl_GetDouble(interp, argv[6], &yaw) != TCL_OK ||
	 Tcl_GetDouble(interp, argv[7], &pitch) != TCL_OK ||
	 Tcl_GetDouble(interp, argv[8], &roll) != TCL_OK )
      return TCL_ERROR;

    tvSetViewGDC3d( base, x, y, z, yaw, pitch, roll );

    return TCL_OK;
}

/*
 * g e t G D C C o m m a n d ( )
 *
 * gets the view as a list of: long, lat, elev, yaw, pitch, roll
 * where lat, long, yaw, pitch, roll are in degrees, and elev is in meters
 *
 */
int
getGDCCommand(tvBase* base, struct Togl* togl, int argc, char* argv[])
{
    double x, y, z, yaw, pitch, roll;
    Tcl_Interp* interp = Togl_Interp(togl);
    Tcl_Obj *values[6], *object;

    /* get the current position (lat/long/elev) and orient */

    tvGetView3d( base, &x, &y, &z, &yaw, &pitch, &roll );
    tvGetViewGDC3d( base, &x, &y, &z );

    /* return coordinates to Tcl */

    values[0] = Tcl_NewDoubleObj(y); /* return long first */
    values[1] = Tcl_NewDoubleObj(x); /* then lat */
    values[2] = Tcl_NewDoubleObj(z);
    values[3] = Tcl_NewDoubleObj(yaw);
    values[4] = Tcl_NewDoubleObj(pitch);
    values[5] = Tcl_NewDoubleObj(roll);
    object = Tcl_NewListObj(6, values);
    Tcl_SetObjResult(interp, object);

    return TCL_OK;
}

/*
 * Utlities to convert between a Tcl "Viewpoint" representation and a 
 * tvViewpoint.
 *
 * The TclView structure is a list, organized as:
 *
 *    { {eyeX eyeY eyeZ} 
 *      altitude 
 *      {refX refY refZ} 
 *      {refPlA refPlB refPlC refPlD} 
 *      nearPlane
 *      farPlane
 *      fov
 *      aspect
 *      {yaw pitch roll}
 *      vel
 *      accel
 *      base_accel
 *    }
 *
 */

/*
 * t v V i e w 2 T c l V i e w ( )
 *
 */
Tcl_Obj*
tvView2TclView(tvViewpoint* tvView)
{
    Tcl_Obj* tclView;
    Tcl_Obj* elements[12];
    Tcl_Obj* eye[3];
    Tcl_Obj* ref[3];
    Tcl_Obj* refPlane[4];
    Tcl_Obj* ypr[3];

    /* convert "eye" */
    eye[0] = Tcl_NewDoubleObj(tvView->eye[0]);
    eye[1] = Tcl_NewDoubleObj(tvView->eye[1]);
    eye[2] = Tcl_NewDoubleObj(tvView->eye[2]);
    elements[0] = Tcl_NewListObj(3, eye);

    elements[1] = Tcl_NewDoubleObj((double)tvView->altitude);

    ref[0] = Tcl_NewDoubleObj(tvView->reference[0]);
    ref[1] = Tcl_NewDoubleObj(tvView->reference[1]);
    ref[2] = Tcl_NewDoubleObj(tvView->reference[2]);
    elements[2] = Tcl_NewListObj(3, ref);

    refPlane[0] = Tcl_NewDoubleObj(tvView->reference_plane[0]);
    refPlane[1] = Tcl_NewDoubleObj(tvView->reference_plane[1]);
    refPlane[2] = Tcl_NewDoubleObj(tvView->reference_plane[2]);
    refPlane[3] = Tcl_NewDoubleObj(tvView->reference_plane[3]);
    elements[3] = Tcl_NewListObj(4, refPlane);

    elements[4] = Tcl_NewDoubleObj((double)tvView->nearPlane);
    elements[5] = Tcl_NewDoubleObj((double)tvView->farPlane);
    elements[6] = Tcl_NewIntObj(tvView->fov);
    elements[7] = Tcl_NewDoubleObj((double)tvView->aspect);

    ypr[0] = Tcl_NewDoubleObj((double)tvView->yaw);
    ypr[1] = Tcl_NewDoubleObj((double)tvView->pitch);
    ypr[2] = Tcl_NewDoubleObj((double)tvView->roll);
    elements[8] = Tcl_NewListObj(3, ypr);

    elements[9] = Tcl_NewDoubleObj((double)tvView->vel);
    elements[10] = Tcl_NewDoubleObj((double)tvView->accel);
    elements[11] = Tcl_NewDoubleObj((double)tvView->base_accel);

    tclView = Tcl_NewListObj(12, elements);
    return tclView;
}

static int
tvTclError( Tcl_Obj *tclView )
{
  fprintf(stderr,"Tcl-View to tvView conversion error:\n");
  fprintf(stderr,"  %s\n", Tcl_GetStringFromObj(tclView, NULL));
  return TCL_ERROR;
}

int
tclView2TvView(Tcl_Obj* tclView, tvViewpoint* tvView)
{
    int len;
    int i;
    int listLen;
    double d;
    Tcl_Obj* list;
    Tcl_Obj* element;

    /*
     * check the structure of the list for validity...
     */
    if (Tcl_ListObjLength(NULL, tclView, &len) != TCL_OK || len != 12) 
        return tvTclError( tclView );

    /* get the eyepoint... */
    listLen = 3;
    if (Tcl_ListObjIndex(NULL, tclView, 0, &list) == TCL_ERROR ||
        list == NULL ||
        Tcl_ListObjLength(NULL, list, &len) != TCL_OK
        || len != listLen)
        return tvTclError( tclView );

    for (i=0; i<listLen; i++) {
        if (Tcl_ListObjIndex(NULL, list, i, &element) != TCL_OK || 
            Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
            return tvTclError( tclView );
        tvView->eye[i] = d;
    }

    /* get the altitude... */
    if (Tcl_ListObjIndex(NULL, tclView, 1, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->altitude = d;

    /* get the reference point... */
    listLen = 3;
    if (Tcl_ListObjIndex(NULL, tclView, 2, &list) == TCL_ERROR ||
        list == NULL ||
        Tcl_ListObjLength(NULL, list, &len) != TCL_OK
        || len != listLen)
        return tvTclError( tclView );

    for (i=0; i<listLen; i++) {
        if (Tcl_ListObjIndex(NULL, list, i, &element) != TCL_OK || 
            Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
            return tvTclError( tclView );
        tvView->reference[i] = d;
    }
    
    /* get the reference plane... */
    listLen = 4;
    if (Tcl_ListObjIndex(NULL, tclView, 3, &list) == TCL_ERROR ||
        list == NULL ||
        Tcl_ListObjLength(NULL, list, &len) != TCL_OK
        || len != listLen)
        return tvTclError( tclView );

    for (i=0; i<listLen; i++) {
        if (Tcl_ListObjIndex(NULL, list, i, &element) != TCL_OK || 
            Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
            return tvTclError( tclView );
        tvView->reference_plane[i] = d;
    }
        
    /* get the nearPlane... */
    if (Tcl_ListObjIndex(NULL, tclView, 4, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->nearPlane = d;

    /* get the farPlane... */
    if (Tcl_ListObjIndex(NULL, tclView, 5, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->farPlane = d;

    /* get the FOV... */
    if (Tcl_ListObjIndex(NULL, tclView, 6, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetIntFromObj(NULL, element, &i) != TCL_OK)
        return tvTclError( tclView );
    tvView->fov = i;

    /* get the aspect... */
    if (Tcl_ListObjIndex(NULL, tclView, 7, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->aspect = d;

    /* get the yaw, pitch, roll... */
    listLen = 3;
    if (Tcl_ListObjIndex(NULL, tclView, 8, &list) == TCL_ERROR ||
        list == NULL ||
        Tcl_ListObjLength(NULL, list, &len) != TCL_OK
        || len != listLen)
        return tvTclError( tclView );

    if (Tcl_ListObjIndex(NULL, list, 0, &element) != TCL_OK || 
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->yaw = d;

    if (Tcl_ListObjIndex(NULL, list, 1, &element) != TCL_OK || 
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->pitch = d;

    if (Tcl_ListObjIndex(NULL, list, 2, &element) != TCL_OK || 
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->roll = d;
    
    /* get the velocity... */
    if (Tcl_ListObjIndex(NULL, tclView, 9, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->vel = d;

    /* get the accel... */
    if (Tcl_ListObjIndex(NULL, tclView, 10, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->accel = d;

    /* get the base_accel... */
    if (Tcl_ListObjIndex(NULL, tclView, 11, &element) == TCL_ERROR || 
        element == NULL ||
        Tcl_GetDoubleFromObj(NULL, element, &d) != TCL_OK)
        return tvTclError( tclView );
    tvView->base_accel = d;

    return TCL_OK;
}
