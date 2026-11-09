/*
 * Name: PATHSFE.C
 *
 * Description:
 *     Implements TerraVision's Flight Path facilities. This lets the
 *     user define and fly a preset flight path around a dataset. Options
 *     include setting the interpolation type, looping of the animation,
 *     managing the update rate (Hz) and speed (m/s) of the flythrough, etc.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     November 26, 1998
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#include "TerraVisionInc.h"

#include "PathsFE.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"

#include <tvutils/tvutils.h>

/*
 * Synopsis:
 *   int PathLoadInitial( ThreeDWidget *threeDWidget, int play_it_too )
 *
 * Description:
 *   This function will take the path filename that was specified
 *   on the command line. It will then work out the full
 *   pathname for the flight file by prepending the directory,
 *   $HOME/.tv/paths. The function will also add a ".path"
 *   extension to the filename if not already specified.
 *
 *   Once we have the full pathname for the flight path, we load it
 *   and optionally begin playing it (if play_it_too is TRUE).
 *
 *   If the full path file does not exist, or if it is not loaded
 *   successfully, then this function will return silently with a
 *   result of FALSE.
 *
 * Returns: TRUE if the load operation was successful, FALSE otherwise
 *
 * Author: Martin Reddy
 * 
 * Date: 7 April, 1999
 * */

int
PathLoadInitial( ThreeDWidget *threeDWidget, int play_it_too )
{
  PRFileInfo filetest;
  char       *filename = tvuGetString( TVU_PATH_URL );

  if ( filename == NULL || threeDWidget == NULL )
    return FALSE;

  /* Check that file exists */

  if ( PR_GetFileInfo ( filename, &filetest ) == PR_FAILURE )
    return FALSE;
  
  /* try to load the path file */

  if ( PathLoadFile( threeDWidget->threeDCore, filename ) == FALSE )
    return FALSE;

  /* do we want to animate it right now? */

  if ( play_it_too )
    PathPlay( threeDWidget->threeDCore );

  return TRUE;
}
