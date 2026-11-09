/* -*- tab-width: 2; -*-

  Name: NetLogCore.c
 
  Description: Tile lifespan logging implemented with LBL's NetLogger
 
  Function List:
    void NetLogConnection( TsmConnection connect, int status );
    void NetLogClean( void );

  Revision History:
      $Id: NetLogCore.c,v 2.3.2.3 2002/10/31 05:26:30 reddy Exp $

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

#include "TerraVisionInc.h"

#include <prnetdb.h> 
#include <prsystem.h>

#include "NetLogCore.h"

char NetLogDirectory[512];
char NetLogHostname[512];
int  NetLogMethod = NL_FILE;

/*
 * Synopsis: 
 *   void NetLogTile( TsTile const *, char * )
 *
 * Description:
 *   Logs an event using the tsmApi NetLogger support. This is used
 *   for logging the life of a tile through some cache, e.g. in and out
 *   of tile or texture cache. You supply the TsCache that the tile is
 *   in and the TsTileId that describes the tile. Finally you just need
 *   to specify which type of event you want to log, e.g. TV_TILE_IN_CACHE,
 *   TV_TILE_OUT_CACHE, TV_TILE_IN_TEXCACHE, or TV_TILE_OUT_TEXCACHE.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 21 October 1998
 *
 */

void
NetLogTile( TsTile const *tile, char const *event )
{
  return; /* this seems to crash a lot with invalid cache pointer */

  if ( tile == NULL || event == NULL ) return;

  tsmLogTile( TsCacheConnection( tile->cache ), event,
	      tile->id.x, tile->id.y, tile->id.res );
}

/*
 * Synopsis: 
 *   void NetLogConnection( TsmConnection connect, int status )
 *
 * Description:
 *   Starts or stops logging for a particular connection, using the
 *   currently set NetLogger options. If status == TRUE then we start
 *   logging, FALSE stops logging. tsmApi is built so that if you
 *   try to start logging when you are already logging, then it will
 *   stop logging first, then start again.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

void
NetLogConnection( TsmConnection connect, int status )
{
  char logoutput[512];
  int  length;

  if ( connect == NULL ) return;

  /* If we are stopping logging, then that's easy... */

  if ( status == FALSE ) {
    tsmEndLogging( connect );
    return;
  }

  /* If we are starting logging, then we have to build either */
  /* the log filename, or the log hostname */

  switch ( NetLogMethod ) {
  case NL_FILE:
    strcpy( logoutput, NetLogDirectory );
    if ( ( length = strlen(logoutput) ) > 0 && logoutput[length-1] != '/' )
      strcat( logoutput, "/" );
    strcat( logoutput, "tvlog-" );
    strcat( logoutput, (char *) tsmGet( connect, TSM_DATASET_NAME ) );
    break;

  case NL_HOST:
    strcpy( logoutput, NetLogHostname );
    break;

  case NL_SYSLOG:
    logoutput[0] = '\0';
    break;
  }

  /* Now we know where we're logging to, just do it */

  tsmBeginLogging( connect, NetLogMethod, logoutput );
}

/*
 * Synopsis: 
 *   void NetLogClean( void )
 *
 * Doc++ Description: *//**
 *   Remove any TerraVision/NetLogger log files on user demand.
 *
 *   We leave log files that are written to disk because we may want
 *   to visualise them later with the NetLogger Visualization tool.
 *   This option lets us remove any TerraVision/NetLogger log files
 *   one user demand.
 *
 * @return  void
 *
 * @author  Martin Reddy
 *
 * @version 3 June 1998
 *
 */

void
NetLogClean( void )
{
  char logcmd[512];
  int  length;

  strcpy( logcmd, "rm -f " );
  strcat( logcmd, NetLogDirectory );
  if ( ( length = strlen( logcmd ) ) > 0 && logcmd[length-1] != '/' )
    strcat( logcmd, "/" );
  strcat( logcmd, "tvlog-*" );

  system( logcmd );
}
