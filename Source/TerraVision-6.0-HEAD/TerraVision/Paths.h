/* -*- tab-width: 2; -*-

  Name: Paths.h
 
  Description: Manages the fight path capabilities of TerraVision
 
  Revision History:
      $Id: Paths.h,v 2.22.2.2 2002/10/31 05:26:32 reddy Exp $

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

#ifndef PATHS_H
#define PATHS_H

typedef struct Path Path;

#include "TsGeoPyramid.h"
#include "ThreeDCore.h"

typedef enum {
  PATH_INTERPOLATE_NONE,
  PATH_INTERPOLATE_LINEAR,
  PATH_INTERPOLATE_SPLINE
} PathInterpType;

typedef struct {
  double x, y, z;
  double yaw, pitch, roll;
  float  index;
} PathItem;

struct Path {
  char const     *name;
  int            count;
  PathItem       *item;
  PathInterpType interpolate;
  int   	 speed_scaling;
  double         distance;
  double         update_rate;
  double         speed;
  double         scaled_speed;
  int            loop;
  int            close_loop;
  int            coords_gcc;
  int            frame;
  int            out_w, out_h;    /* for recording to images */
};

#ifdef __cplusplus
extern "C" {
#endif

/* External interface for managing paths in the core. */
void PathCoreInit( tvBase *threeD );
void PathNew (tvBase *threeD);
void PathSetInterpolate (tvBase *threeD, PathInterpType interp);
void PathSetName (tvBase *threeD, char const *name);

int PathAdd (tvBase *threeD);
int PathDelete (tvBase *threeD, int selected);
void PathUpdate (tvBase *threeD, int selected);
int PathDuplicate (tvBase *threeD, int selected);
int PathUp (tvBase *threeD, int selected);
int PathDown (tvBase *threeD, int selected);

void PathGoto (tvBase *threeD, int i);
void PathGotoStart( tvBase *threeD );
void PathGotoNext( tvBase *threeD );
void PathGotoPrev( tvBase *threeD );
void PathGotoEnd( tvBase *threeD );
void PathPlay( tvBase *threeD );
void PathStop( tvBase *threeD );
void PathPause( tvBase *threeD );
void PathReset( tvBase *threeD );
void PathRecord( tvBase *threeD, int, int );

int PathIsPlaying( tvBase const *threeD );
int PathIsActive( tvBase const *threeD );
double PathGetTimeIndex( tvBase const *threeD );

/* spew OpenGL commands for the specified path */

void PathDraw2D( Path *path, TsGeoPyramid *geo );
void PathDraw3D( Path *path, tvViewpoint *view );

/* here is a low-level hook for you to load a path file by name */

int PathLoadFile( tvBase *threeD, char const *filename );

/* create a path on the fly to the specified location and fly there */

void tvFlyTo( tvBase *threeD, double x, double y, double z,
	      double yaw, double pitch, double roll );

void PathSaveFile( tvBase *threeD, char const *filename );

void PathStartAnimation( tvBase *threeD );
int PathUpdateAnimation( tvBase *threeD );
int PathUpdateRecord (tvBase *threeD);

/* set the default directory for paths without an absolute pathname */

void PathSetDefaultDir( char *directory );

#ifdef __cplusplus
}
#endif

#endif
