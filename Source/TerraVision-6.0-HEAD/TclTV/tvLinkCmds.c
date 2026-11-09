/*
  tvLinkCmds.c
 
  Implementation of the tvbase "link" subcommand.
 

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
#include "togl.h"
#include "ThreeDCore.h"
#include "tvBaseCmdsP.h"

/*
 * t v L i n k C o m m a n d 
 *
 * Handler for link subcommands.
 *
 */
int
tvLinkCommand(struct Togl* togl, int argc, char* argv[])
{
    TvBasePacket* tvPacket = (TvBasePacket*)Togl_GetClientData(togl);
    TvBasePacket* target;
    tvLinkType type;
    char* arg;

    if (argc != 5) goto USAGE;

    /* what type of link are we operating on... */
    if (!strcmp(argv[2], "view"))
        type = TV_MOVEMENT;
    else if (!strcmp(argv[2], "frustum"))
        type = TV_FRUSTUM;
    else if (!strcmp(argv[2], "paths"))
        type = TV_PATHS;
    else if (!strcmp(argv[2], "tiles"))
        type = TV_TILES;
    else
        goto USAGE;

    /* find the target window... */
    target = (TvBasePacket*)Tcl_GetAssocData(Togl_Interp(togl), argv[4], NULL);
    if (!target) {
        Tcl_Obj* errReturn = 
            Tcl_NewStringObj("Unable to find internal association with window: ", -1);
        Tcl_AppendToObj(errReturn, argv[4], -1);
        Tcl_SetObjResult(Togl_Interp(togl), errReturn);
        return TCL_ERROR;
    }

    /* perform the action... */
    if (!strcmp(argv[3], "add")) 
        tvAddLink(tvPacket->base, type, target->base);
    else if (!strcmp(argv[3], "delete"))
        tvRemoveLink(tvPacket->base, type, target->base);
    else
        goto USAGE;

    return TCL_OK;

 USAGE:
    return TCL_ERROR;
}
