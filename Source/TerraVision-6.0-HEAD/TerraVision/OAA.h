/* -*- tab-width: 2; -*-

  Name: OAA.h

  Description:
      Contains various routines that control TerraVision's interaction
      with SRI's OAA (Open Agent Architecture). This includes the features
      such as drawing on the screen and API hooks for the OAA to use.

  Revision History:
      January 19, 1999. Martin Reddy <reddy@ai.sri.com>
      $Id: OAA.h,v 2.21.10.5 2002/10/31 05:26:31 reddy Exp $

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

#ifndef OAA_H
#define OAA_H

typedef struct SentenceType SentenceType;

#include "ThreeDCore.h"

/* The following structures allow us to store an arbitrary number  */
/* of pen strokes, where each pen stroke is simply a list of (x,y) */
/* coordinates. This captures all that a user writes on screen.    */

typedef struct {
  unsigned short x, y;                 /* an (x,y) screen coord */
  double proj[3];
} PenCoordType;

typedef struct {
  int          count;                  /* no. of coords in this stroke */
  int          highwater;              /* no. of coords allocated in array */
  PenCoordType *coord;                 /* an array of coordinates */
} StrokeType;

struct SentenceType {
  int          count;              /* no. of strokes in this sentence */
  int          highwater;          /* no. of strokes allocated in array */
  int          projected;	   /* has it been projected to the terrain? */
  StrokeType   *stroke;            /* and array of strokes */
};

typedef int (* OAACallback) (char *func, char *args, char **answers);

#ifdef __cplusplus
extern "C" {
#endif

void InitSentence( tvBase *threeD );
void RenderSentence( tvBase *threeD, int is_locked );
void ClearSentence( tvBase *threeD );

/* The following functions control TerraVision's connection   */
/* to an OAA facilitator. You can set the facilitator's host  */
/* name and port number, and then connect to that facilitator */

/* Set the hostname for the facilitator - string is strdup'd */
void SetOAAHost( char const *hostname );

/* Set the port number for the facilitator on that host */
void SetOAAPort( int port );

/* Set the OAA host and port number from a string in the format host:port */
void SetOAAHostPort( char const *hostport );

/* Get the current OAA host: returns NULL if none set */
char *GetOAAHost( void );

/* Get the current OAA port number: returns -1 if none set */
int GetOAAPort( void );

/* Set's the default base OAA ID for all tvBase instances */
void SetOAADefaultID( char *id );

/* Connect to the facilitator on the specified host/port */
int OAAConnect( tvBase *threeD );

/* Disconnect from the current facilitator */
void OAADisconnect( tvBase *threeD );

/* Return host:port name of connected facilitator, or NULL if not connected */
char *GetOAAConnected( void );

/* Issue an OAA solve() directive - send a command to the facilitator */
char *OAASolve( char *expression, char *options, char *solveparams );

/* The following functions are API hooks for the OAA to use */
/* in order to control TerraVision from a facilitator       */

/* stop the camera from moving */
void tvStopMotion( tvBase *threeD );

/* re-render the screen */
void tvRefresh( tvBase *threeD );

/* animate a path from current viewpoint to the specified location */
void tvFlyTo3d( tvBase *threeD, double x, double y, double z,
		double yaw, double pitch, double roll );

/* move the viewpoint by (dx,dy,dz) from its current location */
void tvTranslate( tvBase *threeD,
		  double dx, double dy, double dz, int anim );

/* rotate the viewpoint by (dyaw,dpitch,droll) from its current orientation */
void tvRotate( tvBase *threeD,
	       double dyaw, double dpitch, double droll, int anim );

/* move the widget's viewpoint in a viewer-based direction */
int tvMoveViewpoint( tvBase *threeD,
		     double forward, double right, double up, int anim );

/* reset the viewpoint to the initial location/orientation */
void tvResetView( tvBase *threeD );

/* display a message in a dialog associated with the 3-D viewer */
void tvMessage( tvBase *threeD, char const *message );

/* interface to accepts an OAA command and execute it, e.g. "resetview" */
int tvInterpret( tvBase *threeD, char const *command );

void SetDrawTimeout (float secs);
float GetDrawTimeout (void);

/* allow a callback to be set for interpreting more OAA commands */
void OAAAddSolvable( char *solvable, OAACallback callback );

/* enable/disable the addition of the terrain3d() solvable */
void AddTVSolvables( int add_solvables );


#ifdef __cplusplus
}
#endif

#endif
