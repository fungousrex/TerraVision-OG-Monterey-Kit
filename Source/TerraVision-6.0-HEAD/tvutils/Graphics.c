/*
  Filename: Graphics.c

  Description: Contains code to manage the OpenGL context side of things

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

#ifdef HAVE_CONFIG_H
# ifdef MSVC
#  include "winbuild/config.h"
# else
#  include "config.h"
# endif
#endif

#include "tvutils.h"
#include "Args.h"
#include "Config.h"
#include "Platform.h"

glmContext
tvuGetContext( void *handle )
{
#ifdef TVU_WINDOWS
  HWND       hWnd = (HWND) handle;
  glmContext ctx;

  if ( handle == NULL ) return NULL;

  if ( ogl_visual_id >= 0 )
    
    /* use visual specified on command line or in config.xml */

    ctx = glmWCreateContext( hWnd, ogl_visual_id );

  else {

    /* otherwise, try to find a good accelerated visual to use */

    ctx = glmWChooseContext( hWnd, NULL, NULL );

    /* and find out what the ID is for the visual we just selected */

    if ( ctx )
      ogl_visual_id = glmWGetPixelFormat( ctx );
  }

  /* make this visual ID permanent so we don't need to check each time */

  if ( ogl_visual_id >= 0 ) {
    char vis_id[32];
    sprintf( vis_id, "%d", ogl_visual_id );
    tvuAddConfig1( "visual", vis_id );
    tvuSaveConfig();
  }
  
  return ctx;
#else
  Widget parent = (Widget) handle;
  glmContextProperties props = { 0 };
  glmContext ctx;

  if ( handle == NULL ) return NULL;

  props.singleBuffer = singleBuffer;
  props.stereo       = stereoVisual;
  props.listmodes    = list_visuals;
  props.forceid      = ( ogl_visual_id >= 0 );
  props.visualid     = ogl_visual_id;
  ctx = glmXFindContext( XtDisplay(parent), XtScreen(parent), TRUE, &props );

  if ( ! ctx ) {
    fprintf( stderr, "Cannot find any suitable visual! Aborting...\n" );
    exit( 0 );
  }

  return ctx;
#endif
}
