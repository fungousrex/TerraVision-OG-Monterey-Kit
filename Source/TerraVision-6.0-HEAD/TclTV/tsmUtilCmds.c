/*
  tsmUtil.c
 
  Implements Tcl commands for accessing various tsm utility functions.
 

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
#include <tsm/tsm.h>
#include <string.h>

#include <ghttp/http_trans.h>

/*
 * t s m U t i l C o m m a n d ( )
 *
 * Main entry point for processing Tcl "tsmUtil" commands.
 *
 */
int
tsmUtilCommand(ClientData clientData, Tcl_Interp *interp, int objc,
    Tcl_Obj *const objv[])
{
    char* cmd;

    if (objc < 2) return TCL_ERROR;

    cmd = Tcl_GetStringFromObj(objv[1], NULL);

    if (!strcmp(cmd, "showUrl")) {
        char* url;
        if (objc != 3) return TCL_ERROR;
        url = Tcl_GetStringFromObj(objv[2], NULL);
        if (tsmShowUrl(url))
            return TCL_OK;
        else
            return TCL_ERROR;

    } else if (!strcmp(cmd, "urlExist")) {
        char *url;
        int  result;
        Tcl_Obj *retObj = Tcl_NewObj();

        if (objc != 3) return TCL_ERROR;
        url = Tcl_GetStringFromObj(objv[2], NULL);
        result = ( tsmDoesUrlExist(url) ) ? 1 : 0;
        Tcl_SetIntObj(retObj, result);
        Tcl_SetObjResult(interp, retObj);
        return TCL_OK;

#ifdef GHTTP_DEBUG
    } else if (!strcmp (cmd, "set_http_debug")) {
        if (objc != 3) return TCL_ERROR;
	return Tcl_GetIntFromObj(interp, objv[2], &ghttp_debug);

    } else if (!strcmp (cmd, "get_http_debug")) {
	Tcl_Obj *retObj = Tcl_NewObj();
	Tcl_SetIntObj(retObj, ghttp_debug);
	Tcl_SetObjResult(interp, retObj);
	return TCL_OK;
#endif

    }

    return TCL_ERROR;
}
