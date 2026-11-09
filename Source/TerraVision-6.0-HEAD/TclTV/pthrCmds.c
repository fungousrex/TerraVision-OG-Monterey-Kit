/*
  pthrCmds.c
 

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
#include "glt/glt.h"
#include "pthr/trace.h"
#include "Trace.h"
#include "TsCache.h"

/* forward decls... */
int traceCommand(Tcl_Interp*, int, Tcl_Obj *const []);


/*
 * p t h r P r o c ( ) 
 *
 * Tcl-command to interface with "pthr" facilities.
 *
 */
int
pthrProc(ClientData data, Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    char* cmd;

    if (objc < 2) return TCL_OK;

    cmd = Tcl_GetStringFromObj(objv[1], NULL);

    if (!strcmp(cmd, "trace")) {
        return traceCommand(interp, objc, objv);
    }

    return TCL_OK;
}


/*
 * t r a c e C o m m a n d ( )
 *
 * Handles the "pthr trace" subcommands.
 *
 */
int
traceCommand(Tcl_Interp* interp, int objc, Tcl_Obj *const objv[])
{
    static FILE* theLogFile = 0;
    char* action;

    if (objc < 3) return TCL_ERROR;

    /* get the trace action directive... */
    action = Tcl_GetStringFromObj(objv[2], NULL);

    /*
     * l e v e l
     */
    if (!strcmp(action, "level")) {
        int level;
        if (objc != 4) return TCL_ERROR;
        if (Tcl_GetIntFromObj(interp, objv[3], &level) == TCL_ERROR)
            return TCL_ERROR;
        if (level < 0 || level > 4) return TCL_ERROR;

        pthr_trace_level(level);
        return TCL_OK;
    } 

    /*
     * g e t l e v e l
     */
    if (!strcmp(action, "getlevel")) {
      Tcl_Obj *retObj;
      int level;

      level = pthr_trace_level( 0 );
      pthr_trace_level( level );

      retObj = Tcl_NewObj();
      Tcl_SetIntObj(retObj, level);
      Tcl_SetObjResult(interp, retObj);

      return TCL_OK;
    } 

    /*
     * m a s k 
     */
    if (!strcmp(action, "mask")) {
        int mask = 0;
        int i;

        /* here we have to map mask-names as we know them on the Tcl side,
         * to appropriate masks on the pthr side.  Kinda tedious...
         */
        for (i=3; i<objc; i++) {
            char* maskStr = Tcl_GetStringFromObj(objv[i], NULL);
            if (!strcmp(maskStr, "TILE_MASK"))
                mask |= TSMMASK;

            else if (!strcmp(maskStr, "TEXTURE_MASK"))
                mask |= TEX_MASK;

            else if (!strcmp(maskStr, "INIT_MASK"))
                mask |= INIT_MASK;

            else if (!strcmp(maskStr, "VIS_MASK"))
                mask |= VIS_MASK;

            else if (!strcmp(maskStr, "2D_MASK")) 
                mask |= (TWOD_MASK | GLT_MASK);

            else if (!strcmp(maskStr, "3D_MASK"))
                mask |= (THREED_MASK | GLT_MASK);

            else if (!strcmp(maskStr, "MAP_MASK"))
                mask |= (MAP_MASK | GLT_MASK);

            else if (!strcmp(maskStr, "OPENGL_MASK"))
                mask |= (OGL_MASK | GLT_MASK);

            else if (!strcmp(maskStr, "OAA_MASK"))
                mask |= OAA_MASK;

            else if (!strcmp(maskStr, "LOGGING_MASK"))
                mask |= LOG_MASK;

            else if (!strcmp(maskStr, "CACHE_MASK"))
                mask |= CACHE_MASK;

            else if (!strcmp(maskStr, "GEO_MASK"))
                mask |= DOTGEO_MASK;

            else
                return TCL_ERROR;
        }

        pthr_trace_mask(mask);
        return TCL_OK;
    }

    /*
     * l o g f i l e 
     */
    if (!strcmp(action, "logfile")) {
        char* arg;

        if (objc != 4) return TCL_ERROR;

        arg = Tcl_GetStringFromObj(objv[3], NULL);

        if (!strcmp(arg, "close") || theLogFile) {
            if (theLogFile != stdout && theLogFile != stderr)
                fclose(theLogFile);
        }

        /* open the given file if its not stdout or stderr... */
        if (!strcmp(arg, "stdout"))
            theLogFile = stdout;
        else if (!strcmp(arg, "stderr")) 
            theLogFile = stderr;
        else {
            theLogFile = fopen(arg, "w");
            if (!theLogFile) {
                Tcl_Obj* errReturn = 
                    Tcl_NewStringObj("Unable to open log file: ", -1);
                Tcl_AppendToObj(errReturn, arg, -1);
                Tcl_SetObjResult(interp, errReturn);
                return TCL_ERROR;
            }
        }
        
        /* setlinebuf(theLogFile); */ /* not avail under mingwin32 */
        pthr_trace_stream(theLogFile);
        pthr_trace_html(0);
        pthr_trace_print_pid(1);
        
        return TCL_OK;
    }

    return TCL_ERROR;
}
