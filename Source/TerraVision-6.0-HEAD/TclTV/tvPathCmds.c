/*
  tvPathCmds.c
 
  Implements the various "path" subcommands for a tvBase.
 

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

#include <stdio.h>
#include <tcl.h>
#include "ThreeDCore.h"
#include "Paths.h"
#include "TsCoords.h"
#include "togl.h"
#include "tvBaseCmdsP.h"



/*
 * t v P a t h C o m m a n d ( )
 *
 * Top-level command dispatch for the "path" subcommand.  It is installed
 * directly as a subcommand on the Togl widget.
 *
 */
int 
tvPathCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    Tcl_Interp* interp = Togl_Interp(togl);

    if (argc < 3) {
        goto USAGE;
    }

    /*
     * n e w 
     */
    if (!strcmp(argv[2], "new")) {
        PathNew(tvPacket->base);
        return TCL_OK;
    }

    /*
     * l o a d 
     */
    if (!strcmp(argv[2], "load")) {
        Tcl_Obj* retVal = Tcl_NewBooleanObj(0);

        if (argc != 4) goto USAGE;

        if (PathLoadFile(tvPacket->base, argv[3])) 
            Tcl_SetBooleanObj(retVal, 1);

        Tcl_SetObjResult(interp, retVal);
        return TCL_OK;
    }

    /*
     * p l a y
     */
    if (!strcmp(argv[2], "play")) {
        PathPlay(tvPacket->base);
        return TCL_OK;
    }

    /*
     * s t o p 
     */
    if (!strcmp(argv[2], "stop")) {
        PathStop(tvPacket->base);
        return TCL_OK;
    }

    /*
     * c o n t i n u e / p a u s e
     */
    if (!strcmp(argv[2], "continue") || !strcmp(argv[2], "pause")) {
        PathPause(tvPacket->base);
        return TCL_OK;
    }

    /*
     * g o t o 
     */
    if (!strcmp(argv[2], "goto")) {
        int index;

        if (argc != 4) goto USAGE;
        if (!strcmp(argv[3], "start"))
            PathGotoStart(tvPacket->base);
        else if (!strcmp(argv[3], "end"))
            PathGotoEnd(tvPacket->base);
        else if (!strcmp(argv[3], "next"))
            PathGotoNext(tvPacket->base);
        else if (!strcmp(argv[3], "prev"))
            PathGotoPrev(tvPacket->base);
        else if (sscanf(argv[3], "%d", &index) == 1)
            PathGoto(tvPacket->base, index);
        else
            goto USAGE;

        return TCL_OK;
    }

    /* 
     * s a v e 
     */
    if (!strcmp(argv[2], "save")) {
        Tcl_Obj* retVal = Tcl_NewBooleanObj(0);

        if (argc != 4) goto USAGE;

        /* THIS IS BUSTED!  SHOULD RETURN A STATUS!  NOT CONSISTENT WITH LOAD*/
        /** if ( **/ PathSaveFile(tvPacket->base, argv[3]) /**  )  **/ ;
            Tcl_SetBooleanObj(retVal, 1);

        Tcl_SetObjResult(interp, retVal);
        return TCL_OK;

    }

    /* 
     * i s A c t i v e 
     *
     * NOTE: this is not the same functionality as the library function
     * PathIsActive().  This reports whether or not there's a path that's
     * currently initialized and associated with the tvBase.
     *
     */
    if (!strcmp(argv[2], "isActive")) {
        Tcl_Obj* retVal = Tcl_NewBooleanObj(0);
        if (tvGetPath(tvPacket->base)) {
            Tcl_SetBooleanObj(retVal, 1);
        }

        Tcl_SetObjResult(interp, retVal);
        return TCL_OK;
    }

    /* 
     * i s P l a y i n g 
     */
    if (!strcmp(argv[2], "isPlaying")) {
        Tcl_Obj* retVal = Tcl_NewBooleanObj(0);
        if (PathIsActive(tvPacket->base) && PathIsPlaying(tvPacket->base)) {
            Tcl_SetBooleanObj(retVal, 1);
        }

        Tcl_SetObjResult(interp, retVal);
        return TCL_OK;
    }

    /*
     * a d d 
     */
    if (!strcmp(argv[2], "add")) {
        PathAdd(tvPacket->base);
        return TCL_OK;
    }

    /*
     * d e l e t e 
     */
    if (!strcmp(argv[2], "delete")) {
        int index;

        if (argc != 4) goto USAGE;
        if (sscanf(argv[3], "%d", &index) != 1) goto USAGE;

        if (PathDelete(tvPacket->base, index) < 0) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("attempt to delete waypoint from list has failed", -1);
            Tcl_SetObjResult(interp, errReturn);
            return TCL_ERROR;
        }
        
        return TCL_OK;
    }

    /*
     * u p 
     */
    if (!strcmp(argv[2], "up")) {
        int index;

        if (argc != 4) goto USAGE;
        if (sscanf(argv[3], "%d", &index) != 1) goto USAGE;

        if (PathUp(tvPacket->base, index) < 0) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("attempt to move waypoint up in the list has failed", -1);
            Tcl_SetObjResult(interp, errReturn);
            return TCL_ERROR;
        }
        
        return TCL_OK;
    }

    /*
     * d o w n 
     */
    if (!strcmp(argv[2], "down")) {
        int index;

        if (argc != 4) goto USAGE;
        if (sscanf(argv[3], "%d", &index) != 1) goto USAGE;

        if (PathDown(tvPacket->base, index) < 0) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("attempt to move waypoint down in the list has failed", -1);
            Tcl_SetObjResult(interp, errReturn);
            return TCL_ERROR;
        }
        
        return TCL_OK;
    }
    /*
     * d u p l i c a t e 
     */
    if (!strcmp(argv[2], "duplicate")) {
        int index;

        if (argc != 4) goto USAGE;
        if (sscanf(argv[3], "%d", &index) != 1) goto USAGE;

        if (PathDuplicate(tvPacket->base, index) < 0) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("attempt to duplicate waypoint failed", -1);
            Tcl_SetObjResult(interp, errReturn);
            return TCL_ERROR;
        }
        
        return TCL_OK;
    }

    /*
     * u p d a t e 
     */
    if (!strcmp(argv[2], "update")) {
        int index;

        if (argc != 4) goto USAGE;
        if (sscanf(argv[3], "%d", &index) != 1) goto USAGE;

        /* THIS IS BUSTED!  SHOULD RETURN A STATUS!  NOT CONSISTENT WITH OTHER
         *  PATH COMMANDS
         */
        /** if ( **/ PathUpdate(tvPacket->base, index); /** < 0) {
            Tcl_Obj* errReturn = Tcl_NewStringObj("attempt to update waypoint has failed", -1);
            Tcl_SetObjReturn(errReturn);
            return TCL_ERROR;
            } **/
        
        return TCL_OK;
    }

    /*
     * r e c o r d
     */
    if (!strcmp(argv[2], "record")) {
        int xres = 320;
        int yres = 240;

        if (argc != 3 && argc !=5) goto USAGE;
        if (argc == 5) {
	  sscanf(argv[3], "%d", &xres);
	  sscanf(argv[4], "%d", &yres);
        }

        PathRecord(tvPacket->base, xres, yres);
        return TCL_OK;
    }

    /*
     * l i s t 
     */
    if (!strcmp(argv[2], "list")) {
        Path* path = tvGetPath(tvPacket->base);
        Tcl_Obj* list = Tcl_NewListObj(0, NULL);
        Tcl_Obj* elem;
        char ew, ns;
        char buff[50];

        if (path) {
            int i;
            double gdc[3];

            for (i=0; i<path->count; i++) {
                PathItem* item = &path->item[i];

                /* convert to GDC coords, then generate a "nice" string rep. */
                TsGCCtoGDC(item->x, item->y, item->z, gdc);
                if (gdc[0] < 0.) {
                    ew = 'W';
                    gdc[0] = -gdc[0];
                }
                else
                    ew = 'E';
                if (gdc[1] < 0.) {
                    ns = 'S';
                    gdc[1] = -gdc[1];
                }
                else
                    ns = 'N';
                sprintf(buff, "%1.3g%c %1.3g%c %1.0fm", gdc[0], ew, gdc[1],
                    ns, gdc[2]);

                elem = Tcl_NewStringObj(buff, -1);
                Tcl_ListObjAppendElement(interp, list, elem);
            }
        }

        Tcl_SetObjResult(interp, list);
        return TCL_OK;
    }

    /*
     * v e l o c i t y 
     */
    if (!strcmp(argv[2], "velocity")) {
        
        if (argc == 3) {
            Path * path = tvGetPath(tvPacket->base);
            Tcl_Obj* retVal = Tcl_NewDoubleObj(path->speed);
            Tcl_SetObjResult(interp, retVal);
            return TCL_OK;
        }

        if (argc < 4) goto USAGE;

        if (!strcmp(argv[3], "scale")) {
            Path* path = tvGetPath(tvPacket->base);

            if (argc == 4) {
                Tcl_Obj* retVal;
                char* val = "off";
                if (path->speed_scaling) val = "on";
                retVal = Tcl_NewStringObj(val, -1);
                Tcl_SetObjResult(interp, retVal);
                return TCL_OK;
            }
                
            if (argc != 5) goto USAGE;
            if (!strcmp(argv[4], "on")) 
                path->speed_scaling = 1;
            else if (!strcmp(argv[4], "off"))
                path->speed_scaling = 0;
            else 
                goto USAGE;
        }
        else {
            double speed;
            Path* path = tvGetPath(tvPacket->base);

            if (argc != 4) goto USAGE;
            if (sscanf(argv[3], "%lf", &speed) != 1) goto USAGE;
            path->speed = speed;
            if (speed == 0.) PathStop(tvPacket->base);
        }

        return TCL_OK;
    }

    /*
     * l o o p 
     */
    if (!strcmp(argv[2], "loop")) {
        Path* path = tvGetPath(tvPacket->base);

        if (argc == 3) {
            Tcl_Obj* retVal = Tcl_NewStringObj((path->loop)?"on":"off", -1);
            Tcl_SetObjResult(interp, retVal);
            return TCL_OK;
        }

        if (argc < 4) goto USAGE;
        if (!strcmp(argv[3], "close")) {
            if (argc == 4) {
                Tcl_Obj* retVal = 
                    Tcl_NewStringObj((path->close_loop)?"on":"off", -1);
                Tcl_SetObjResult(interp, retVal);
                return TCL_OK;
            }

            if (argc != 5) goto USAGE;
            if (!strcmp(argv[4], "on")) 
                path->close_loop = 1;
            else if (!strcmp(argv[4], "off"))
                path->close_loop = 0;
            else 
                goto USAGE;
        }
        else {
            if (!strcmp(argv[3], "on")) 
                path->loop = 1;
            else if (!strcmp(argv[3], "off"))
                path->loop = 0;
            else 
                goto USAGE;
        }

        return TCL_OK;
    }

    /*
     * i n t e r p o l a t e 
     */
    if (!strcmp(argv[2], "interpolate")) {
        
        if (argc == 3) {
            Path* path = tvGetPath(tvPacket->base);
            Tcl_Obj* retVal = Tcl_NewObj();
            if (path->interpolate == PATH_INTERPOLATE_NONE)
                Tcl_SetStringObj(retVal, "none", -1);
            else if (path->interpolate == PATH_INTERPOLATE_LINEAR)
                Tcl_SetStringObj(retVal, "linear", -1);
            else if (path->interpolate == PATH_INTERPOLATE_SPLINE)
                Tcl_SetStringObj(retVal, "spline", -1);
            Tcl_SetObjResult(interp, retVal);
            return TCL_OK;
        }

        if (argc != 4) goto USAGE;

        if (!strcmp(argv[3], "none"))
            PathSetInterpolate(tvPacket->base, PATH_INTERPOLATE_NONE);
        else if (!strcmp(argv[3], "linear"))
            PathSetInterpolate(tvPacket->base, PATH_INTERPOLATE_LINEAR);
        else if (!strcmp(argv[3], "spline"))
            PathSetInterpolate(tvPacket->base, PATH_INTERPOLATE_SPLINE);
        else
            goto USAGE;

        return TCL_OK;
    }

    /*
     * n a m e 
     */
    if (!strcmp(argv[2], "name")) {
        
        if (argc == 4) {
            PathSetName(tvPacket->base, argv[3]);
        }
        else {
            Path* path = tvGetPath(tvPacket->base);
            Tcl_Obj* returnObj;
            if (path) 
                returnObj = Tcl_NewStringObj(path->name, -1);
            else
                returnObj = Tcl_NewStringObj("", -1);
            Tcl_SetObjResult(interp, returnObj);
        }

        return TCL_OK;
    }


 USAGE:
    // NEED BETTER REPORTING HERE!
    return TCL_ERROR;
}

