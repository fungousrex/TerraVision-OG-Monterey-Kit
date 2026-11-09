/*
  Filename: Values.c

  Description: Contains routines for returning the current state of the
  tvutils library to the client, and for querying certain system values.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthr/trace.h>

#include "tvutils.h"
#include "Args.h"
#include "Config.h"
#include "Init.h"

/* include the version info for tvutils and TerraVision */

#include "RELEASE"
#include "../RELEASE"


/* setup TRUE (1) and FALSE (0) defines, if not already setup */

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

static char *
ConvSlashes( char *filename )
{
  char *ptr = filename;

  if ( filename == NULL ) 
    return NULL;

  /* convert all \ slashes into / slashes */

  while ( *ptr != '\0' ) {
    if ( *ptr == '\\' )
      *ptr = '/';
    ptr++;
  }

  /* remove any trailing slash character */

  if ( *filename != '\0' && *(--ptr) == '/' )
    *ptr = '\0';

  return filename;
}

float
tvuGetFloat( int token )
{
  switch ( token ) {
  case TVU_DROP_TO_WIREFRAME:
    return drop_to_wireframe;

  case TVU_LICENSE_ACCEPTED:
    return (float) atof(license_accepted);

  case TVU_VERSION:
    return TVU_RELEASE;  /* defined in RELEASE file */
  }
  return -1.0;
}

int
tvuGetInt( int token )
{

  switch ( token ) {

  case TVU_ARGS_PARSED:
    return args_parsed;

  case TVU_TOTAL_ARGS:
    return args_total;

  case TVU_TRACE_MASK:
    return tv_trace_mask;

  case TVU_NUM_DATASETS:
    return numInitialOis;

  case TVU_INITIAL_VIEWER:
    return initial_viewer;

  case TVU_STEREO:
    return stereoVisual;

  case TVU_SINGLE_BUFFER:
    return singleBuffer;

  case TVU_TILE_BY_TILE:
    return tileByTile;

  case TVU_NUM_COMMANDS:
    return cmd_list_size;

  case TVU_X_ERROR_ABORT:
    return XErrorAbort;

  case TVU_SHOW_LICENSE:
    return showLicense;

  case TVU_LOG:
    return logOnStart;

  case TVU_NO_IPC:
    return no_ipc;

  case TVU_NS_WIN_ID:
    return ns_win_id;

  case TVU_WIDTH:
    return width;

  case TVU_HEIGHT:
    return height;

  case TVU_CAVE:
    return cave;

  case TVU_MODE:
    return tvu_mode;

  case TVU_VISUAL_ID:
    return ogl_visual_id;

  }
  return -1;
}

char *
tvuGetString( int token )
{
  switch ( token ) {
  case TVU_SCHEMA_URL:
    return ConvSlashes( tvuGetSchemaUrl() );

  case TVU_PROFILE_URL:
    return ConvSlashes( tvuGetProfileUrl() );

  case TVU_CONFIG_URL:
    return ConvSlashes( tvuGetConfigFilename() );

  case TVU_INSTALL_DIR:
    return ConvSlashes( tvuGetInstallDir() );

  case TVU_USER_DIR:
    return ConvSlashes( tvuGetUserDir() );

  case TVU_PATH_URL:
    return ( path_url ) ? ConvSlashes( tvuGetPathUrl( path_url ) ) : NULL;

  case TVU_PATHS_DIR:
    return ConvSlashes( tvuGetPathUrl( NULL ) );

  case TVU_VIEWPOINT_URL:
    return ConvSlashes( tvuGetViewpointsFilename() );

  case TVU_DATABASE_URL:
    return ConvSlashes( tvuGetDatabaseFilename() );

  case TVU_TCL_FILENAME:
    return ConvSlashes( tvuGetTclFilename() );

  case TVU_TRACE_FILE:
    if ( debug_filename )
      return ConvSlashes( debug_filename );
    else
      return "stderr";

  case TVU_TV_VERSION:
    return TERRAVISION_RELEASE;

  case TVU_LICENSE_ACCEPTED:
    return license_accepted;
  }

  return NULL;
}

char *
tvuGetStringN( int token, int n )
{
  switch ( token ) {
  case TVU_COMMAND:
    if ( n < 0 || n >= cmd_list_size )
      return NULL;
    else
      return cmd_list[n];

  case TVU_OI_URL:
    if ( n < 0 || n >= numInitialOis )
      return NULL;
    else
      return ConvSlashes( initialOi[n] );

  case TVU_DEM_URL:
    if ( n < 0 || n >= numInitialDems )
      return NULL;
    else 
      return ConvSlashes( initialDem[n] );
  }

  return NULL;
}


int
tvuSetInt( int token, int value )
{
  switch ( token ) {

  case TVU_ARGS_PARSED:
    args_parsed = value;
    return TRUE;

  case TVU_TOTAL_ARGS:
    args_total = value;
    return TRUE;

  case TVU_TRACE_MASK:
    tv_trace_mask = value;
    pthr_trace_mask( tv_trace_mask );
    return TRUE;

  case TVU_NUM_DATASETS:
    numInitialOis = value;
    return TRUE;

  case TVU_INITIAL_VIEWER:
    initial_viewer = value;
    return TRUE;

  case TVU_STEREO:
    stereoVisual = value;
    return TRUE;

  case TVU_SINGLE_BUFFER:
    singleBuffer = value;
    return TRUE;

  case TVU_TILE_BY_TILE:
    tileByTile = value;
    return TRUE;

  case TVU_NUM_COMMANDS:
    cmd_list_size = value;
    return TRUE;

  case TVU_X_ERROR_ABORT:
    XErrorAbort = value;
    return TRUE;

  case TVU_SHOW_LICENSE:
    showLicense = value;
    return TRUE;

  case TVU_LOG:
    logOnStart = value;
    return TRUE;

  case TVU_NO_IPC:
    no_ipc = value;
    return TRUE;

  case TVU_NS_WIN_ID:
    ns_win_id = value;
    return TRUE;

  case TVU_WIDTH:
    width = value;
    return TRUE;

  case TVU_HEIGHT:
    height = value;
    return TRUE;

  case TVU_CAVE:
    cave = value;
    return TRUE;

  case TVU_MODE:
    tvu_mode = value;
    return TRUE;

  case TVU_VISUAL_ID:
    ogl_visual_id = value;
    return TRUE;
  }
  return FALSE;
}

int
tvuSetFloat( int token, float value )
{
  char buffer[32];

  switch ( token ) {
  case TVU_DROP_TO_WIREFRAME:
    drop_to_wireframe = value;
    return TRUE;

  case TVU_LICENSE_ACCEPTED:
    sprintf( buffer, "%1g", value );
    if ( license_accepted ) free( license_accepted );
    license_accepted = strdup( buffer );
    return TRUE;
  }
  return FALSE;
}

int
tvuSetString( int token, char const *value )
{
  switch ( token ) {
  case TVU_SCHEMA_URL:
    if ( schema_url ) free( schema_url );
    schema_url = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_PROFILE_URL:
    if ( profile_url ) free( profile_url );
    profile_url = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_CONFIG_URL:
    if ( config_file ) free( config_file );
    config_file = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_INSTALL_DIR:
  case TVU_USER_DIR:
  case TVU_PATHS_DIR:
    return FALSE;

  case TVU_PATH_URL:
    if ( path_url ) free( path_url );
    path_url = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_VIEWPOINT_URL:
    if ( viewpoint_url ) free( viewpoint_url );
    viewpoint_url = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_DATABASE_URL:
    if ( database_url ) free( database_url );
    database_url = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_TCL_FILENAME:
    if ( tcl_path ) free( tcl_path );
    tcl_path = ( value ) ? strdup( value ) : NULL;
    return TRUE;

  case TVU_TRACE_FILE:

    if ( ! value ) value = "stderr";

    if ( strcmp( value, "stderr" ) != 0 ) {
      FILE *new_debug_file;

      if ( ( new_debug_file = fopen( value, "w" ) ) != NULL ) {
	if ( debug_file )
	  fclose( debug_file );
	if ( debug_filename )
	  free( debug_filename );
	debug_filename = strdup( value );
	debug_file = new_debug_file;
	pthr_trace_stream( new_debug_file );
      }

    } else {
      if ( debug_file ) {
	fclose( debug_file );
	debug_file = NULL;
      }
      if ( debug_filename ) {
	free( debug_filename );
	debug_filename = NULL;
      }
      pthr_trace_stream( stderr );
    }

    return TRUE;

  case TVU_LICENSE_ACCEPTED:
    if ( license_accepted ) free( license_accepted );
    license_accepted = ( value ) ? strdup( value ) : NULL;
    return TRUE;
  }
  return FALSE;
}
