/*
  tsmCmds.h
 
  Public interfaces to the tsmDatabase and tsmUtil commands.


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

#ifndef _tsmCmds_h_
#define _tsmCmds_h_

extern int tsmDatabaseCommand(ClientData data, Tcl_Interp*, int objc, 
    Tcl_Obj *const objv[]);

extern int tsmUtilCommand(ClientData data, Tcl_Interp*, int objc,
    Tcl_Obj *const objv[]);

#endif
