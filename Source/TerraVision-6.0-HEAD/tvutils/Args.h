/*
  Filename: Args.h

  Description: Contains platform-specific parsing of command line arguments
  for TerraVision applications.

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

#ifndef TVU_ARGS_H
#define TVU_ARGS_H

#include <stdio.h>

#include "../RELEASE"          /* TerraVision release number */

#define MAX_URLS  10                   /* maximum number of -oi/-dem urls */
#define MAX_CMDS  15                   /* maximum number of -cmd strings */


/* the following variables are exported by the Args.c module */
/* These are to be used *only* internally within libtvutils. */
/* external clients should go throught the tvuGet functions. */

extern int   args_parsed;               /* number of arguments parsed */
extern int   args_total;                /* total number of arguments given */
extern float drop_to_wireframe;         /* frame rate to reduce complexity */
extern FILE  *debug_file;               /* file handle for debug output file */
extern char  *debug_filename;           /* filename for debug output */
extern char  *schema_url;               /* url for .geo schema */
extern int   tv_trace_mask;             /* trace mask set from command line */
extern char  *initialDem[];             /* list of initial DEMs */
extern char  *initialOi[];              /* list of initial OIs */
extern int   numInitialDems;            /* size of initial DEM list */
extern int   numInitialOis;             /* size of initial OI list */
extern int   initial_viewer;            /* open 2d or 3d viewer? */
extern int   stereoVisual;              /* Request a stereo visual */
extern int   singleBuffer;              /* Render in single buffer mode */
extern int   tileByTile;                /* tile-by-tile debug mode. */
extern char  *cmd_list[];               /* list of commands from -cmd */
extern int   cmd_list_size;             /* number of -cmd commands found */
extern char  *config_file;              /* pathname to the config file */
extern int   XErrorAbort;               /* do an abort() on an X error */
extern int   showLicense;               /* show the TerraVision license */
extern int   logOnStart;                /* start logging on startup? */
extern int   no_ipc;                    /* allow commands typed on stdin */
extern int   ns_win_id;                 /* Netscape plugin window ID */
extern int   width;                     /* requested window width */
extern int   height;                    /* requested window height */
extern int   cave;                      /* is the CAVE on? */
extern int   no_config;                 /* ignore the configuration file? */
extern char  *path_url;                 /* url to a TerraVision path file */
extern char  *viewpoint_url;            /* url to a viewpoint file */
extern char  *database_url;             /* url to the database file */
extern char  *license_accepted;         /* accepted TV version license */
extern char  *profile_url;              /* .geo profile XML file to use */
extern char  *tcl_path;                 /* path to TerraVision.tcl */
extern int   ogl_visual_id;             /* ID of desired OpenGL visual */
extern int   list_visuals;              /* list all OGL visuals and scores */

/* this function is used to clean up the Args module */

void tvuArgFinish( void );


#endif
