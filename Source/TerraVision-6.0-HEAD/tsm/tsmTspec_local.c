/*
 * Name: tsmTspec_local.c
 *
 * Description: 
 *   Contains routines for manipulating tspec files for local disk datasets.
 *   These functions are private to the tsmApi.
 *
 * Function List:
 *   Pyramid *tsmReadTspec_local( char *pathname, TsmConnectionParams params )
 *
 * Revision History:
 *   Created 12 mar 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTspec_local.c,v 1.9.6.1 2001/06/19 22:04:45 leei Exp $
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

#include "tsmInternal.h"
#include "tsmConnectionParamsInt.h"

/*
 * Synopsis:
 *    int tsmDoesTspecExist_local( char *pathname )
 *
 * Description:
 *    This function will test to see if either a tiles.tspec or a
 *    tile-set.tspec file exists in the specified directory.
 *
 * Returns: TRUE if tspec file(s) exist, FALSE otherwise.
 *
 * Author: Martin Reddy
 *
 * Date: 10 March 1998
 *
 */

int
tsmDoesTspecExist_local( char *pathname )
{
  char        filename[512];
  PRFileInfo  filetest;

  /* Check for the tiles.tspec file in the directory specified */

  strcpy( filename, pathname );
  if ( filename[strlen(filename)-1] != '/' ) strcat( filename, "/" );
  strcat( filename, "tiles.tspec" );

  if ( PR_GetFileInfo ( filename, &filetest ) == PR_SUCCESS )
    return TRUE;

  /* Check for the tile-set.tspec file in the directory specified */

  strcpy( filename, pathname );
  if ( filename[strlen(filename)-1] != '/' ) strcat( filename, "/" );
  strcat( filename, "tile-set.tspec" );

  if ( PR_GetFileInfo ( filename, &filetest ) == PR_SUCCESS )
    return TRUE;

  return FALSE;
}


/*
 * Synopsis:
 *   Pyramid *tsmReadTspec_local( char *pathname, TsmConnectionParams )
 *
 * Description:
 *   Loads a pyramid tspec file off of local disk and parses it into
 *   a Pyramid structure (allocated within the function). This filled
 *   structure is then returned by the function (or NULL if failed).
 *
 *   The pathname parameter is a local filesystem directory path for the
 *   pyramid.  
 *
 * Returns:
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

Pyramid *
tsmReadTspec_local( char *pathname, TsmConnectionParams params )
{
  static const char *funcName = "tsmReadTspec_local";
  PRFileInfo        status;
  char              tspecFileName[512];
  Pyramid           *pyramid;
  int               old_format = FALSE;

  /* Allocate the pyramid structure then use the old tspec reading func */

  if ( ( pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a pyramid tspec" );
    return NULL;
  }

  /* Did the user request the old style tspec files? */
  
  if ( tsmIsParamSet( params, TSM_PARAM_NEW_TSPEC ) && ! params->new_tspec ) 
    old_format = TRUE;

  /* If the user didn't request any particular format, then we */
  /* are allowed to try and fallback from new to old style if  */
  /* there is no new style tspec                               */

  tspTspecFileName( pathname, tspecFileName );
  if ( tsmIsParamSet( params, TSM_PARAM_NEW_TSPEC ) == FALSE &&
       ( PR_GetFileInfo ( tspecFileName, &status ) ) == PR_FAILURE )
    old_format = TRUE;

  /* Try to open the new single-file format tspec file for reading.  */
  /* If we can't get a lock on this, then we assume that the pyramid */
  /* is using the old tspec file organisation so we default back to  */
  /* the old tspReadPyramid() function to read these files.          */

  if ( old_format == TRUE
       || ( PR_GetFileInfo ( tspecFileName, &status ) ) == PR_FAILURE ) {
    if ( tspReadOldPyramid( pathname, pyramid, TRUE ) == FALSE ) {
      tsmError( funcName, "Could not load any tspec files!" );
      tsmFreeTspec( pyramid );
      return NULL;
    }
  } else {
    if ( tspReadTspec( tspecFileName, pyramid ) == FALSE ) {
      tsmError( funcName, "Could not load new tspec file!" );
      tsmFreeTspec( pyramid );
      return NULL;
    }
  }

  /* If a geoName was not specified, then try to make one up */

  tsmEnsureGeoNameSet( pyramid );

  return pyramid;
}

