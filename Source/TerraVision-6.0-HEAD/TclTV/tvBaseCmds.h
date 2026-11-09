/*
  tvBaseCmds.h

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

#ifndef _tvBaseCmds_h_
#define _tvBaseCmds_h_

#include <togl.h>

extern void tvToglCreateCB(struct Togl*);
extern void tvToglDisplayCB(struct Togl*);
extern void tvToglReshapeCB(struct Togl*);
extern void tvToglDestroyCB(struct Togl*);

extern int tvPathCommand(struct Togl*, int, char*[]);
extern int tvViewCommand(struct Togl*, int, char*[]);
extern int tvSetCommand(struct Togl*, int, char*[]);
extern int tvGetCommand(struct Togl*, int, char*[]);
extern int tvLinkCommand(struct Togl*, int, char*[]);
extern int tvFeatureCommand(struct Togl*, int, char*[]);
extern int tvLogCommand(struct Togl*, int, char*[]);
extern int tvOAACommand(struct Togl*, int, char*[]);
extern int tvGeosetCommand(struct Togl*, int, char*[]);
extern int tvDestroyCommand(struct Togl*, int, char*[]);
extern int tvStrokeCommand(struct Togl*, int, char* []);
extern int tvAnchorCommand(struct Togl*, int, char* []);
extern void tvBaseCmdsInit(void);
extern void tvConsoleMsg( char const *fmt, ... );
extern int tvMsgListAdd( char const *mode, char const *id,
			 char const *msg, char const *opt );

extern void tvMsgListClear(void);

extern int tvutilsInterface(ClientData, Tcl_Interp *, int, Tcl_Obj *const []);

#endif
