/* -*- tab-width: 2; -*-

  Name: NetLogCore.h
 
  Description: Tile lifespan logging implemented with LBL's NetLogger
 
  Function List:
    void NetLogConnection( TsmConnection connect, int status );
    void NetLogClean( void );
    void NetLogTile( TsTile const *, char const * );

  Revision History:
      $Id: NetLogCore.h,v 2.2.2.2 2002/10/31 05:26:30 reddy Exp $

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
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----
 
*/

#ifndef NETLOGCORE_H
#define NETLOGCORE_H

#include "TsCache.h"

extern char NetLogDirectory[512];
extern char NetLogHostname[512];
extern int  NetLogMethod;

void NetLogConnection( TsmConnection connect, int status );

void NetLogTile( TsTile const *tile, char const *event );
void NetLogClean( void );


#endif
