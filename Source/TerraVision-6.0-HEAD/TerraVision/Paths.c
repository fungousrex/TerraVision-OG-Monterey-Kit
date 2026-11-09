/* -*- tab-width: 2; -*-

  Name: Paths.c

  Description:
      Implements TerraVision's Flight Path facilities. This lets the
      user define and fly a preset flight path around a dataset. Options
      include setting the interpolation type, looping of the animation,
      managing the update rate (Hz) and speed (m/s) of the flythrough, etc.

  Revision History:
      November 26, 1998
      $Id: Paths.c,v 2.61.2.2 2002/10/31 05:26:31 reddy Exp $

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
#include "ThreeDCoreI.h"

#include <prio.h>

#define PATH_BUFFER_SIZE 512              /* max size of a line in file */

static void PathFree( Path *path );
static Path *PathAlloc( int count );
static int PathReadLine( FILE *handle, char *buffer );
static int PathCountItems( FILE *handle );
static void PathParseCommand( Path *path, char *buffer );
static void PathParseItem( char *buffer, PathItem *item );
static double PathCalcDistance( Path *path, int index1, int index2 );
static void PathPreProcess( tvBase *threeD, Path *path );
static void PathInterpolate( Path *path, double index, PathItem *item );
static void PathFixAngle( double *angle1, double *angle2 );
static void PathSetView(  tvBase *threeD, PathItem *item, PRBool jump );
static void PathGetView( tvBase *threeD, PathItem *item );
static void PathPlayNoCheck( tvBase *threeD );
static void PathStopNoCheck( tvBase *threeD );
static void PathActivateRecord( tvBase *threeD, int msecs );

static char *default_path_dir = NULL;

void
PathCoreInit( tvBase *threeD )
{
  threeD->playing = FALSE;            /* TRUE if currently animating */
  threeD->flying = FALSE;             /* TRUE if flying to a new loc */
  threeD->curr_index = 0.0;           /* normalised time index [0-1] */
  threeD->index_inc = 0.0;            /* time increment */
  threeD->pathTimer = NULL;
}

static void
PathFree( Path *path )
{
  if ( path == NULL ) return;

  if ( path->name != NULL ) tsmMemFree( (void *) path->name );
  if ( path->item != NULL ) tsmMemFree( (void *) path->item );
  tsmMemFree( path );
}

static Path *
PathAlloc( int count )
{
  Path *path;

  if ( ( path = tsmMemCalloc( 1, sizeof(Path) ) ) == NULL )
    return NULL;
  
  path->interpolate   = PATH_INTERPOLATE_SPLINE;
  path->speed_scaling = TRUE;
  path->name          = tsmMemStrdup( "unnamed" );
  path->count         = count;
  path->update_rate   = 30.0;
  path->speed         = 500.0;
  path->loop          = FALSE;
  path->close_loop    = FALSE;
  path->coords_gcc    = TRUE;
  path->frame         = 0;
  
  if ( ( path->item = tsmMemCalloc( count, sizeof(PathItem) ) ) == NULL )
    return NULL;

  return path;
}

static int
PathReadLine( FILE *handle, char *buffer )
{
  int length;

  if ( feof( handle ) ) return FALSE;
  buffer[0] = '\0';
  fgets( buffer, PATH_BUFFER_SIZE, handle );
  if ( feof( handle ) ) return FALSE;

  length = strlen( buffer );
  while ( length > 0 && isspace(buffer[length-1]) )
    buffer[--length] = '\0';
  
  return TRUE;
}

static int
PathCountItems( FILE *handle )
{
  char buffer[PATH_BUFFER_SIZE];
  int  items = 0;

  rewind( handle );

  while ( PathReadLine( handle, buffer ) ) {
    if ( buffer[0] == '\0' ||
	 buffer[0] == '#' ||
	 buffer[0] == '$' ) continue;
    items++;
  }

  rewind( handle );
  return items;
}

static void
PathParseCommand( Path *path, char *buffer )
{
  char *cmd = buffer, *value;

  /* find the end of the command name */

  cmd++;
  value = cmd;
  while ( isspace(*value) == FALSE && *value != '=' )
    value++;
  *(value++) = '\0';

  /* find the start of the command value */

  while ( isspace(*value) || *value == '=' )
    value++;

  /* printf( "Cmd = '%s', Value = '%s'\n", cmd, value ); */

  /* Check for supported commands */

  if ( strcmp( cmd, "interpolate" ) == 0 ) {

    if ( strcmp( value, "linear" ) == 0 ) 
      path->interpolate = PATH_INTERPOLATE_LINEAR;
    else if ( strcmp( value, "spline" ) == 0 )
      path->interpolate = PATH_INTERPOLATE_SPLINE;
    else if ( strcmp( value, "none" ) == 0 )
      path->interpolate = PATH_INTERPOLATE_NONE;
    else
      fprintf( stderr, "Unrecognised interpolate value: %s\n", value );

  } else if ( strcmp( cmd, "name" ) == 0 ) {

    if ( path->name != NULL ) tsmMemFree( (void*) path->name );
    path->name = tsmMemStrdup( value );

  } else if ( strcmp( cmd, "update_rate" ) == 0 ) {
 
    path->update_rate = atof( value );

  } else if ( strcmp( cmd, "speed" ) == 0 ) {
 
    path->speed = atof( value );

  } else if ( strcmp( cmd, "loop" ) == 0 ) {
 
    path->loop = ( strcmp( value, "yes" ) == 0 );

  } else if ( strcmp( cmd, "close" ) == 0 ) {
 
    path->close_loop = ( strcmp( value, "yes" ) == 0 );

  } else if ( strcmp( cmd, "speed_scaling" ) == 0 ) {
 
    path->speed_scaling = ( strcmp( value, "yes" ) == 0 );

  } else if ( strcmp( cmd, "coords_gcc" ) == 0 ) {
 
    path->coords_gcc = ( strcmp( value, "yes" ) == 0 );

  } else 

    fprintf( stderr, "Unrecognised command: %s\n", cmd );
  
}

static void
PathParseItem( char *buffer, PathItem *item )
{
  char   *token = buffer, *value, *end_value;
  char   *end_string = buffer + strlen(buffer)-1;
  double n;

  while ( token < end_string ) {

    /* Skip over any leading whitespace */
   
    while ( isspace(*token) && token < end_string )
      token++;

    /* Find the end of the token and null-terminate it */

    value = token;
    while ( isspace(*value) == FALSE && *value != '=' && value < end_string )
      value++;
    *(value++) = '\0';

    /* Find the start of the value string */

    while ( ( isspace(*value) || *value == '=' ) && value < end_string )
      value++;

    /* Find the end of the value and null-terminate it */

    end_value = value;
    while ( isspace(*end_value) == FALSE && end_value < end_string )
      end_value++;
    *end_value = '\0';

    /* Test the token for the supported strings */

    n = atof( value );

    /* printf( "token = '%s' value = '%s' (%lf)\n", token, value, n ); */

    if ( strcmp( token, "x" ) == 0 )
      item->x = n;
    else if ( strcmp( token, "y" ) == 0 )
      item->y = n;
    else if ( strcmp( token, "z" ) == 0 )
      item->z = n;
    else if ( strcmp( token, "yaw" ) == 0 )
      item->yaw = n;
    else if ( strcmp( token, "pitch" ) == 0 )
      item->pitch = n;
    else if ( strcmp( token, "roll" ) == 0 )
      item->roll = n;
    else
      fprintf( stderr, "Unrecognised path token: %s\n", token );

    /* Move the token pointer on to the next field */

    token = end_value + 1;
  }
}

void
PathUpgradeItem (PathItem *item, int from_version)
{
  /* convert from an older version. (N.B. No break statements.  That way,
     these can cascade from older to newer). */
  switch (from_version) {
  case 1: {
    double tmp = item->y;
    item->y = item->z;
    item->z = tmp;
    item->pitch = -item->pitch + M_PI/2;
    item->yaw   =  item->yaw   - M_PI/2;
  }
  }

  /* canonicalize pitch and yaw */
  if (item->yaw <= -M_PI)
    item->yaw += 2*M_PI;  
  else if (item->yaw > M_PI)
    item->yaw -= 2*M_PI;  
  if (item->pitch <= -M_PI)
    item->pitch += 2*M_PI;  
  else if (item->pitch > M_PI)
    item->pitch -= 2*M_PI;  
}

static double
PathCalcDistance( Path *path, int index1, int index2 )
{
  PathItem *item1, *item2;
  double   dx, dy, dz;

  if ( index1 < 0 || index2 < 0 ) return 0.0;
  if ( index1 >= path->count || index2 >= path->count ) return 0.0;

  item1 = &(path->item[index1]);
  item2 = &(path->item[index2]);
  dx    = item2->x - item1->x;
  dy    = item2->y - item1->y;
  dz    = item2->z - item1->z;

  return sqrt( ( dx * dx ) + ( dy * dy ) + ( dz * dz ) );
}

static void
PathFixAngle( double *angle1, double *angle2 )
{
  /* Ensure that the magnitude between two angles is <= 180.0 deg */

  if ( fabs( *angle1 - *angle2 ) <= M_PI ) return;

  if ( *angle1 > *angle2 )
    *angle1 -= M_PI * 2.0;
  else
    *angle2 -= M_PI * 2.0;
}

static void
PathPreProcess( tvBase *threeD, Path *path )
{
  int i;

  /* Calculate the total length of the path */

  path->distance = 0.0;
  for ( i = 0; i < path->count - 1; i++ )
    path->distance += PathCalcDistance( path, i, i+1 );

  /* If total distance is zero and we have only 2 points, */
  /* then make the distance equal to the orientation difference */

  if ( path->distance == 0.0 && path->count == 2 ) {
    double dyaw   = path->item[0].yaw - path->item[1].yaw;
    double dpitch = path->item[0].pitch - path->item[1].pitch;
    double droll  = path->item[0].roll - path->item[1].roll;

    path->distance = sqrt( ( dyaw * dyaw ) + ( dpitch * dpitch ) +
			   ( droll * droll ) );
  }

  /* Calculate the time indices for each of the data points */

  for ( i = 0; i < path->count; i++ )
    path->item[i].index = i / ( path->count - 1.0 );

  /* Ensure the delta between successive orient angles is <= 180 deg */
  /* This stops us from doing silly things like rotating 355 deg to  */
  /* the left instead of only 5 deg to the right.                    */

  for ( i = 0; i < path->count - 1; i++ ) {
    PathFixAngle( &path->item[i].yaw, &path->item[i+1].yaw );
    PathFixAngle( &path->item[i].pitch, &path->item[i+1].pitch );
    PathFixAngle( &path->item[i].roll, &path->item[i+1].roll );
  }

  /* Calculate the time increment for each frame */
  threeD->index_inc = ( path->speed / path->distance ) / path->update_rate;
}


static double
SplineInterp( double p0, double p1, double p2, double p3, double s )
{

  /* perform a Catmull-Rom spline interpolation between p1 and p2 */
  /* we require the previous and next points, p0 and p3, in order */
  /* to calculate the tangents at p1 and p2 (you can make p0 = p1 */
  /* and/or p3 = p2 if at the start/end of list). s = 0.0 gives   */
  /* p1, and s = 1.0 gives p2. */

  double s2 = s * s;
  double s3 = s2 * s;
  double h1 = 2.0 * s3 - 3.0 * s2 + 1.0;
  double h2 = -2.0 * s3 + 3.0 * s2;
  double h3 = s3 - 2 * s2 + s;
  double h4 = s3 - s2;

  /* calculate the tangent values. Catmull-Rom splines are a subset */
  /* of Cardinal splines, where the multiplier, a, is 0.5. You can */
  /* vary this value to change the tightness of the curve (0..1). */

  double t1 = 0.5 * ( p2 - p0 );
  double t2 = 0.5 * ( p3 - p1 );

  /* P = S * h * C,  using a Hermite curve notation where:   */
  /*                                                         */
  /*      | s^3 |        | P1 |          |  2  -2   1   1 |  */
  /*  S = | s^2 |    C = | P2 |      h = | -3   3  -2  -1 |  */
  /*      | s^1 |        | T1 |          |  0   0   1   0 |  */
  /*      |  1  |        | T2 |          |  1   0   0   0 |  */

  return h1 * p1 + h2 * p2 + h3 * t1 + h4 * t2;
}

#define PATH_INTERP(x,y,r) ( x + ( y - x ) * r )

static void
PathInterpolate( Path *path, double index, PathItem *item )
{
  int            i = 0, last_i = path->count - 1;
  PathItem       *item0, *item1, *item2, *item3;
  double         ratio;
  PathInterpType interp;

  /* Validate the parameters */

  if ( item == NULL ) return;
  interp = path->interpolate;
  if ( index < 0.0 ) index = 0.0;

  if (path->close_loop) {
    while ( i < last_i && path->item[i+1].index < index )
      i++;
  }
  else {
    if ( index > 1.0 ) index = 1.0;

    /* Find the index of the base path item to use for the interpolation   */
    /* Future: a binary search would be more efficient here for long paths */

    while ( i < last_i && path->item[i+1].index < index )
      i++;

    if ( i == last_i ) interp = PATH_INTERPOLATE_NONE;
  }

  item1 = &(path->item[i]);
  item2 = &(path->item[(i+1)%path->count]);

  if (path->close_loop && index > 1.0)
    ratio = ( index - item1->index ) / 
      ( 1.0 + 1.0 / (path->count - 1) + item2->index - item1->index );
  else
    ratio = ( index - item1->index ) / ( item2->index - item1->index );

  switch ( interp ) {
  case PATH_INTERPOLATE_NONE:
    
    /* If we are doing no interpolation then this is only we need to do */
    /* Also, if this is the last datapoint, then we are done too.       */

    if ( index == 1.0 ) i = last_i;
    *item = path->item[i];
    item->index = index;
    return;

  case PATH_INTERPOLATE_LINEAR:

    /* do a linear interpolation between the two datapoints */

    item->x     = PATH_INTERP( item1->x, item2->x, ratio );
    item->y     = PATH_INTERP( item1->y, item2->y, ratio );
    item->z     = PATH_INTERP( item1->z, item2->z, ratio );
    item->yaw   = PATH_INTERP( item1->yaw, item2->yaw, ratio );
    item->pitch = PATH_INTERP( item1->pitch, item2->pitch, ratio );
    item->roll  = PATH_INTERP( item1->roll, item2->roll, ratio );
    item->index = index;
    return;

  case PATH_INTERPOLATE_SPLINE:

    /* Otherwise we do a Catmull-Rom spline interpolation */

    /* get the 4 control points = the two datapoints to interpolate */
    /* between and the datapoint at either side of these. We duplicate */
    /* the start/end datapoint if we are at the start/end of the list */

    item0 = ( i > 0 ) ? &path->item[i-1] : item1;
    if (path->close_loop)
      item3 = &path->item[(i+2) % path->count];
    else
      item3 = ( i < last_i-1 ) ? &path->item[i+2] : item2;

    /* do the spline interpolation of the position and orientation */

    item->x = SplineInterp( item0->x, item1->x, item2->x, item3->x, ratio );
    item->y = SplineInterp( item0->y, item1->y, item2->y, item3->y, ratio );
    item->z = SplineInterp( item0->z, item1->z, item2->z, item3->z, ratio );

    item->yaw   = SplineInterp( item0->yaw, item1->yaw, item2->yaw,
				item3->yaw, ratio );
    item->pitch = SplineInterp( item0->pitch, item1->pitch, item2->pitch,
				item3->pitch, ratio );
    item->roll  = SplineInterp( item0->roll, item1->roll, item2->roll,
				item3->roll, ratio );

    return;

  }
}

/*
 * Doc++ Description: *//**
 *   Loads a path from the given file, replacing any established path for 
 *   the thread. Generates messages if the file doesn't exist or is not a 
 *   valid path file.
 *
 * @param threeD     handle to render state
 * @param filename   name of file to be loaded.
 *
 * @return  boolean result: 1 on success, 0 on failure.
 *
 *
 */

int
PathLoadFile( tvBase *threeD, char const *filename )
{
  FILE     *handle;
  int      items;
  PathItem item;
  char     buffer[512], pathname[512];
  int      version;

  Path     *path = threeD->path;

  /* is the filename relative, if so then try to make it absolute */

  strcpy( pathname, filename );

  if ( default_path_dir )
    tsmMakePathAbsolute( pathname, default_path_dir, 512 );

  /* Try to open the path file (from local disk or http) */

  if ( ( handle = tsmOpenStream( pathname, "r" ) ) == NULL ) {
    tvBaseError( threeD, "Couldn't load path file:\n%s does not exist!",
		 pathname );
    return FALSE;
  }

  /* Check the magic number on the first line */

  PathReadLine( handle, buffer );  
  if ( strncmp( buffer, "#PATH", 5 ) != 0 ) {
    tvBaseError( threeD, "%s is not a valid TerraVision path file!",
		 pathname );
    tsmCloseStream( handle );
    return FALSE;
  }

  /* Determine the version of the path file. */
  version = 1;
  sscanf (buffer, "#PATH V%d", &version);

  /* Count how many path items we have in this file */

  if ( ( items = PathCountItems( handle ) ) < 2 ) {
    tvBaseError( threeD, "%s contains less than 2 data points!" );
    tsmCloseStream( handle );
    return FALSE;
  }

  /* We have a valid path file, so let's reset our path structure */
  
  if (path) {
    PathFree (path);
    threeD->path = path = NULL;
  }

  if ( (path = PathAlloc( items )) == NULL ) {
    tvBaseError( threeD, "Couldn't allocate memory for the path data!" );
    tsmCloseStream( handle );
    return FALSE;
  } 

  threeD->path = path;

  /* Setup an initial position */

  item.x = item.y = item.z = 0.0;
  item.yaw = item.pitch = item.roll = 0.0;

  /* Now parse the file */

  path->count = 0;
  while ( PathReadLine( handle, buffer ) ) {

    /* Is this line whitespace/comment? */
    
    if ( buffer[0] == '#' || buffer[0] == '\0' ) continue;

    /* Do we have a command? */

    if ( buffer[0] == '$' ) {
      PathParseCommand( path, buffer );
    } else {

      /* Otherwise, we have a new data point */

      PathParseItem( buffer, &item );
      PathUpgradeItem (&item, version);

      /* Don't support old LVCS paths anymore - only GCC ones */

      if ( ! path->coords_gcc ) {
	tvBaseError( threeD, "Old LVCS path files are no longer supported!" );
	tsmCloseStream( handle );
	return FALSE;
      }

      path->item[path->count++] = item;
    }

  }

  /* Work out the total distance and time indices */

  PathPreProcess(threeD, path);

  /* close the path file and we are done */

  tsmCloseStream( handle );
  return TRUE;
}


/*
 * Doc++ Description: *//**
 *   Writes the currently established path to the given file. Generates a 
 *   message if either there is no established path or the file could not be 
 *   created.
 *
 * @param threeD    handle to render state
 * @param filename  name of file to be created
 *
 * @return void
 *
 */

void
PathSaveFile( tvBase *threeD, char const *filename )
{
  FILE     *handle;
  int      n;
  PathItem *item;
  Path *path = threeD->path;

  /* do we have a path to save? */

  if ( path == NULL ) {
    tvBaseError (threeD, "There is no current flight path to save!");
    return;
  }

  /* Try to create the path file */

  if ( ( handle = fopen( filename, "w" ) ) == NULL ) {
    tvBaseError( threeD, "Couldn't create the Path File!" );
    return;
  }

  fprintf( handle, "#PATH V2.0\n\n" );
  fprintf( handle, "$name          = %s\n", path->name );
  fprintf( handle, "$speed         = %1.1f\n", path->speed );
  fprintf( handle, "$loop          = %s\n",
	   ( path->loop ) ? "yes" : "no" );
  fprintf( handle, "$close         = %s\n",
	   ( path->close_loop ) ? "yes" : "no" );
  fprintf( handle, "$speed_scaling = %s\n",
	   ( path->speed_scaling ) ? "yes" : "no" );
  fprintf( handle, "$coords_gcc    = %s\n",
	   ( path->coords_gcc ) ? "yes" : "no" );
  fprintf( handle, "$interpolate   = " );
  switch ( path->interpolate ) {
  case PATH_INTERPOLATE_NONE:
    fprintf( handle, "none\n" );
    break;
  case PATH_INTERPOLATE_LINEAR:
    fprintf( handle, "linear\n" );
    break;
  case PATH_INTERPOLATE_SPLINE:
    fprintf( handle, "spline\n" );
    break;
  }
  fprintf( handle, "\n" );

  for ( n = 0; n < path->count; n++ ) {
    item = & path->item[n];
    fprintf( handle, "x=%f y=%f z=%f yaw=%f pitch=%f roll=%f\n",
	     item->x, item->y, item->z,
	     item->yaw, item->pitch, item->roll );
  }

  fclose( handle );
}


/*
 * Doc++ Description: *//**
 *   Creates a new path instance in the given render thread. Memory and state 
 *   for any previously established path is freed.
 *
 * @param threeD  handle to rendering state
 *
 * @return void
 *
 */

void
PathNew (tvBase *threeD)
{
  if (threeD->path)
    PathFree (threeD->path);
  threeD->path = PathAlloc( 0 );
}

/*
 * Doc++ Description: *//**
 *   Specifies how the playback animation should be smoothed.
 *
 * @param threeD   handle to rendering state
 * @param interp   interpolation type;  is one of:
 * <pre>
 *   PATH_INTERPOLATE_NONE
 *   PATH_INTERPOLATE_LINEAR
 *   PATH_INTERPOLATE_SPLINE
 * </pre>
 * 
 * @return void
 *
 */
void
PathSetInterpolate (tvBase *threeD, PathInterpType interp)
{
  Path *path = threeD->path;
  if (path->interpolate != interp) {
    path->interpolate = interp;
    tvBaseUpdateView(threeD, FALSE);
  }
}

/*
 * Doc++ Description: *//**
 *   Assigns an arbitrary name to the currently defined path.
 *
 * @param threeD  handle to rendering state
 * @param name    user-assigned name for the current path
 *
 * @return void
 *
 */
void
PathSetName (tvBase *threeD, char const *name)
{
  if ( threeD->path->name )
    tsmMemFree( (void *) threeD->path->name );
  threeD->path->name = tsmMemStrdup( name );
}

/*
 * Doc++ Description: *//**
 *   Adds the current viewpoint state of the render thread to the current path 
 *   (creating a new one if none was previously established). The newly added 
 *   path entry may later be referenced with the index returned by this 
 *   function.
 *
 * @param threeD  handle to rendering state
 *
 * @return integer index associated with new path entry.
 *
 */
int
PathAdd (tvBase *threeD)
{
  Path *path = threeD->path;

  if ( path == NULL )
    threeD->path = path = PathAlloc( 0 );
  
  path->count++;
  path->item = (PathItem *) realloc(path->item, sizeof(PathItem)*path->count);

  PathGetView( threeD, &path->item[path->count-1] );
  PathPreProcess (threeD, path);
  return path->count-1;
}

/*
 * Doc++ Description: *//**
 *   Deletes a path entry at the specified index. Any subsequent entries have 
 *   their indices decreased by one.
 *
 * @param threeD    handle to rendering state
 * @param index     index of path element to be removed
 *
 * @return   Index of the deleted entry on successful completion.
 *           -1 if the index is invalid or there is no established path for 
 *           the thread.
 *
 */
int
PathDelete (tvBase *threeD, int selected)
{
  Path *path = threeD->path;
  int n;

  if ( path == NULL || selected < 0 || selected >= path->count) {
    tvBaseError( threeD, "Delete: No Viewpoint Selected!" );
    return -1;
  }

  path->count--;
  for ( n = selected; n < path->count; n++ )
    path->item[n] = path->item[n+1];

  PathPreProcess(threeD, path);

  return selected;
}

/*
 * Doc++ Description: *//**
 *   Replaces the path entry at the specified index with whatever is the 
 *   current viewpoint state of the thread. Generates a message if the index 
 *   is invalid or there is no establish path for the thread.
 *
 * @param threeD    handle to rendering state
 * @param index     index of the path element to be replaced.
 *
 * @return void
 *
 */
void
PathUpdate (tvBase *threeD, int selected)
{
  Path 	       *path = threeD->path;

  if ( path == NULL || selected < 0 ) {
    tvBaseError( threeD, "Update: No Viewpoint Selected!" );
    return;
  }

  PathGetView( threeD, &path->item[selected] );
  PathPreProcess(threeD, path);
}

/*
 * Doc++ Description: *//**
 *   Duplicates the indexed path entry at the end of the list of entries.
 *
 * @param threeD    handle to rendering state
 * @param index     index of element to be duplicated.
 *
 * @return   Index of the newly appended entry on successful completion.
 *           -1 if index is less than zero or if there is no established path.
 *
 */
int
PathDuplicate (tvBase *threeD, int selected)
{
  Path *path = threeD->path;

  if ( path == NULL || selected < 0 ) {
    tvBaseError( threeD, "Duplicate: No Viewpoint Selected!" );
    return selected;
  }

  path->count++;
  path->item = (PathItem *) realloc(path->item, sizeof(PathItem)*path->count);

  path->item[path->count-1] = path->item[selected];

  PathPreProcess(threeD, path);

  return path->count-1;
}

/*
 * Doc++ Description: *//**
 *   Swaps the indexed path entry with the entry immediately preceeding it.
 *
 * @param threeD   handle to rendering state.
 * @param index    index of entry to be swapped.
 *
 * @return   New index of the entry on success.  -1 if the index is less 
 *           than or equal to zero, or there is no currently established path.
 *
 */
int
PathUp (tvBase *threeD, int selected)
{
  Path 	       *path = threeD->path;
  PathItem temp;

  if ( path == NULL || selected < 0 ) {
    tvBaseError( threeD, "Bubble Up: No Viewpoint Selected!" );
    return -1;
  } else if ( selected == 0 )
    return -1;

  temp = path->item[selected-1];
  path->item[selected-1] = path->item[selected];
  path->item[selected] = temp;

  PathPreProcess(threeD, path);

  return selected-1;
}

/*
 * Doc++ Description: *//**
 *   Swaps the indexed path entry with the entry immediately following it.
 *
 * @param threeD   handle to rendering state
 * @param index    index of the entry to be swapped.
 *
 * return   New index of the entry on success.  -1 if the index is less than 
 *          zero, greater than the number of path entries, or there is no 
 *          currently established path.
 *
 */
int
PathDown (tvBase *threeD, int selected)
{
  Path 	       *path = threeD->path;
  PathItem     temp;

  if ( path == NULL || selected < 0 ) {
    tvBaseError( threeD, "Bubble Up: No Viewpoint Selected!" );
    return -1;
  } else if ( selected == path->count-1 )
    return -1;

  temp = path->item[selected+1];
  path->item[selected+1] = path->item[selected];
  path->item[selected] = temp;
  
  PathPreProcess(threeD, path);
  return selected+1;
}

static void
PathGetView( tvBase *threeD, PathItem *item )
{
  tvViewpoint view;

  if ( item == NULL ) return;

  tvCurrentView( threeD, &view );

  item->x     = view.eye[0];
  item->y     = view.eye[1];
  item->z     = view.eye[2];
  item->yaw   = view.yaw;
  item->pitch = view.pitch;
  item->roll  = view.roll;
}

static void
PathSetView( tvBase *threeD, PathItem *item, PRBool jump )
{
  tvViewpoint view;

  if ( item == NULL ) return;

  tvCurrentView (threeD, &view);

  view.eye[0] = item->x;
  view.eye[1] = item->y;
  view.eye[2] = item->z;
  view.yaw    = item->yaw;
  view.pitch  = item->pitch;
  view.roll   = item->roll;

  tvSetView (threeD, &view, jump);
}

static void
PathAnimationCB( void *closure )
{
  tvBase *threeD = (tvBase *) closure;

  if (PathUpdateAnimation (threeD))
    PathStartAnimation(threeD);
}

static void
PathRecordCB( void *closure )
{
  tvBase *threeD = (tvBase *) closure;

  if (PathUpdateRecord (threeD))
    PathActivateRecord (threeD, 50);
}

static void
PathActivateRecord( tvBase *threeD, int msecs )
{
  if ( msecs ) {
    tvBaseAddTimer (threeD, &threeD->pathTimer, msecs,
		    PathRecordCB, threeD);
  } else {
    tvBaseDelTimer (threeD, &threeD->pathTimer );
  }
}

static void
PathActivateAnimation( tvBase *threeD, int msecs )
{
  if (msecs) {
    tvBaseAddTimer (threeD, &threeD->pathTimer, msecs, 
		    PathAnimationCB, threeD);
  } else {
    tvBaseDelTimer (threeD, &threeD->pathTimer );
  }
}

void
PathStartAnimation( tvBase *threeD )
{
  PathActivateAnimation (threeD, 1000 / threeD->path->update_rate);
}

#define TIMEVAL_TO_DOUBLE(t) ((double)t / (double)PR_TicksPerSecond())

int
PathUpdateAnimation (tvBase *threeD)
{
  PathItem       item;
  PRIntervalTime curr_time;
  double         inc, secs_diff, last_secs, curr_secs;
  double	 heightOld, heightNew;
  Path           *path = threeD->path;

  /* If we've been signalled to stop, then do it */

  if ( path == NULL || path->distance == 0.0 || path->speed == 0.0 ) 
    threeD->playing = FALSE;
  
  if ( ! threeD->playing ) {
    tvBasePostRedraw (threeD, FALSE);
    return FALSE;
  }

  /* Otherwise work out how long since the last frame */

  if ( threeD->timestamp == 0 ) {
    threeD->timestamp = PR_IntervalNow();
    secs_diff = 0.0;
  } else {
    last_secs = TIMEVAL_TO_DOUBLE( threeD->timestamp );

    curr_time = PR_IntervalNow();
    curr_secs = TIMEVAL_TO_DOUBLE( curr_time );

    secs_diff = curr_secs - last_secs;

    /* If before our update rate, then sleep for a bit (0.1 of update rate) */

    if ( secs_diff < ( 1.0 / path->update_rate ) ) {
      PathActivateAnimation (threeD, 100.0 / path->update_rate);
      return FALSE;
    } else {
      threeD->timestamp = curr_time;
    }
  }

  inc = secs_diff / ( path->distance / (path->speed_scaling ? 
					path->speed : path->scaled_speed) );

  /* Advance our normalised time counter and check for end condition */
  threeD->curr_index += inc;
  if ( (threeD->curr_index > 1.0 && !path->close_loop) ||
       (threeD->curr_index > 1.0 + 1.0 / (path->count - 1) && 
	path->close_loop)) {

    /* Is this the end of a Fly-To animation? */

    if ( threeD->flying ) {
      PathSetView( threeD, &path->item[1], FALSE ); /* display last frame */
      PathStopNoCheck(threeD);	/* stop the fly-to animation */
      PathFree(path);		/* free the temp path we just made */
      threeD->flying = FALSE;	/* no longer flying-to a location */
      threeD->path = path = threeD->old_path; /* restore the originally loaded path */
      threeD->old_path = NULL; /* signal that fly-to is done */
      tvBaseStatus (threeD, NULL);
      return FALSE;
    } 

    /* Otherwise this is the end of a flight path */

    if ( path->loop ) {
      PathPlay(threeD);
      return FALSE;
    } else
      PathStop(threeD);

  }

  /* Work out the interpolated position/orientation for this time */

  PathInterpolate( path, threeD->curr_index, &item );
  heightOld = threeD->view.altitude;
  
  path->frame++;
  
  /* Set the new viewpoint in the 3D widget and keep the animation going */
  
  PathSetView (threeD, &item, FALSE);

  /* change the speed if so desired */
  if (path->speed_scaling) {
    heightNew = tvBasePointAltitude(threeD, threeD->view.eye);
    path->scaled_speed *= heightNew / heightOld;
  }

  return TRUE;
}

int
PathUpdateRecord (tvBase *threeD)
{
  PathItem       item;
  double         inc, secs_diff, total_secs, curr_secs, last_secs;
  double	 heightOld, heightNew;
  Path           *path = threeD->path;
  char           filename[32];

  /* If we've been signalled to stop, then do it */

  if ( path == NULL || path->distance == 0.0 || path->speed == 0.0 ) 
    threeD->playing = FALSE;
  
  if ( ! threeD->playing ) {
    printf( "                                 \r" ); fflush( stdout );
    tvBasePostRedraw (threeD, FALSE);
    return FALSE;
  }

  /* how long since our last frame? */

  last_secs = TIMEVAL_TO_DOUBLE( threeD->timestamp );
  curr_secs = TIMEVAL_TO_DOUBLE( PR_IntervalNow() );
  secs_diff = curr_secs - last_secs;

  /* Do we have a highest quality image yet? That is, have */
  /* all of the tiles been loaded in for this viewpoint?   */
  /* If not, then we will go back to sleep until they are. */
  /* In case we get wedged, then proceed after 10 secs.    */

  if ( threeD->image_quality < 0.99 && secs_diff < 10.0 )
    return TRUE;

  /* save out the current image */

  printf( "Creating frame %d (%d x %d)   \r", path->frame, path->out_w,
	  path->out_h );
  fflush( stdout );

  sprintf( filename, "tvimage%04d.ppm", path->frame++ );
  tvSaveScreen( threeD, path->out_w, path->out_h, filename );

  /* otherwise, we need to progress onto the next frame to */
  /* be rendered.                                          */

  total_secs = ( path->distance / (path->speed_scaling ? 
  				   path->speed : path->scaled_speed) );

  inc = 1.0 / ( total_secs * 30 ); /* output at 30 fps */

  /* Advance our normalised time counter and take a time stamp*/

  threeD->curr_index += inc;

  threeD->timestamp = PR_IntervalNow();

  /* Check for the end condition and stop recording if we reach it */

  if ( threeD->curr_index > 1.0 )
    PathStop( threeD );

  /* Work out the interpolated position/orientation for this time */

  PathInterpolate( path, threeD->curr_index, &item );
  heightOld = threeD->view.altitude;
  
  /* Set the new viewpoint in the 3D widget and keep the animation going */
  
  PathSetView (threeD, &item, FALSE);

  /* change the speed if so desired */
  if (path->speed_scaling) {
    heightNew = tvBasePointAltitude(threeD, threeD->view.eye);
    path->scaled_speed *= heightNew / heightOld;
  }

  return TRUE;
}

/*
 * Doc++ Description: *//**
 *   Sets the current thread viewpoint to the path entry at the given index. 
 *   Silently returns if no path is established.
 *
 * @param threeD   handle to rendering state
 * @param index    index of the path entry.
 *
 * @return  void
 *
 */
void
PathGoto (tvBase *threeD, int i)
{
  Path *path = threeD->path;
  if ( path == NULL || threeD->flying ) return;

  if ( i >= 0 && i < path->count ) {
    threeD->playing = FALSE;
    threeD->curr_index = path->item[i].index;
    PathSetView( threeD, &path->item[i], TRUE );
    tvBaseUpdate (threeD, TV_PATHS);
  }
}

/*
 * Doc++ Description: *//**
 *   Sets the thread's current viewpoint to the first entry of the established
 *   path.  Silently returns if no path is established.
 *
 * @param threeD  handle to rendering state.
 * 
 * @return void
 *
 */
void
PathGotoStart( tvBase *threeD )
{
  PathGoto (threeD, 0);
}

/*
 * Doc++ Description: *//**
 *   Sets the thread's current point to the next highest indexed entry in the 
 *   path, given the current state of path playback (ISSUE: how is this 
 *   defined if playback is not in progress?). Silently returns if no path 
 *   is established.
 *
 * @param threeD  handle to rendering state.
 *
 * @return void
 *
 */
void
PathGotoNext( tvBase *threeD )
{
  int i = 0;
  Path *path = threeD->path;

  if ( path == NULL || threeD->flying ) return;

  while ( i < path->count-1 && path->item[i].index <= threeD->curr_index )
    i++;
  
  PathGoto (threeD, i);
}

/*
 * Doc++ Description: *//**
 *   Sets the thread's current point to the next lowest indexed entry in the 
 *   path, given the current state of path playback (ISSUE: what is this 
 *   defined as if playback is not in progress?). Silently returns if no path 
 *   is established.
 *
 * @param threeD  handle to rendering thread/
 *
 * @return void
 *
 */
void
PathGotoPrev( tvBase *threeD )
{
  int i = 0;
  Path *path = threeD->path;

  if ( path == NULL || threeD->flying ) return;

  while ( i < path->count-1 && path->item[i+1].index < threeD->curr_index )
    i++;

  PathGoto (threeD, i);
}

/*
 * Doc++ Description: *//**
 *   Sets the thread's current viewpoint to the last entry of the established
 *   path.  Silently returns if no path is established.
 *
 * @param threeD  handle to rendering state.
 *
 * @return  void
 *
 */
void
PathGotoEnd( tvBase *threeD )
{
  Path *path = threeD->path;

  PathGoto (threeD, path->count-1);
}

static void
PathPlayNoCheck( tvBase *threeD )
{
  Path *path = threeD->path;

  path->scaled_speed = path->speed;
  path->frame = 0;
  threeD->curr_index = 0.0;
  threeD->playing = TRUE;
  threeD->timestamp = 0;
  PathStartAnimation(threeD);
  tvBaseUpdate (threeD, TV_PATHS);
}

/*
 * Doc++ Description: *//**
 *   Begins playback of a path. Silently returns if no path is established 
 *   for the thread. (ISSUE: playback always starts at the beginning of the 
 *   path? or where it left off?)
 *
 * @param threeD  handle to rendering state.
 *
 * @return void
 *
 */
void
PathPlay( tvBase *threeD )
{
  if ( threeD->path == NULL || threeD->flying ) return;

  if ( threeD->playing ) PathStopNoCheck( threeD );

  PathPlayNoCheck( threeD );
  tvBaseStatus( threeD, "" );
}

/*
 * Doc++ Description: *//**
 *   Begins recording of a path to a series of framegrabs for compilation
 *   into an MPEG (or other) movie file.
 *
 * @param threeD  handle to rendering state.
 *
 * @return void
 *
 */
void
PathRecord( tvBase *threeD, int width, int height )
{
  Path *path = threeD->path;

  if ( path == NULL || threeD->flying ) return;

  if ( threeD->playing ) PathStopNoCheck( threeD );

  path->frame = 0;
  path->scaled_speed = path->speed;
  path->out_w = width;
  path->out_h = height;
  threeD->curr_index = 0.0;
  threeD->playing = TRUE;
  threeD->timestamp = 0;

  PathActivateRecord( threeD, 50 /* 50 ms = 20 Hz */ );

  tvBaseUpdate (threeD, TV_PATHS);

  tvBaseStatus( threeD, "" );
}

static void
PathStopNoCheck( tvBase *threeD )
{
  threeD->path->frame = 0;
  threeD->playing = FALSE;
  PathActivateAnimation (threeD, 0);
  tvBaseUpdate (threeD, TV_PATHS);
}

/*
 * Doc++ Description: *//**
 *   Halts playback of the path. Silently returns if no path is established 
 *   for the thread.
 *
 * @param threeD  handle to rendering state.
 *
 * @return  void
 *
 */
void
PathStop( tvBase *threeD )
{
  if ( threeD->path == NULL || threeD->flying ) return;
  PathStopNoCheck( threeD );
  tvBaseStatus( threeD, "" );
  tvBasePostRedraw (threeD, FALSE);
}

/*
 * Doc++ Description: *//**
 *   Pauses playback of the path. Silently returns if no path is established 
 *   for the thread.
 *
 * @param threeD  handle to rendering state
 *
 * @return void
 *
 */
void
PathPause( tvBase *threeD )
{
  if ( threeD->path == NULL || threeD->flying ) return;
  threeD->playing = ! threeD->playing;
  threeD->timestamp = 0;
  if ( threeD->playing ) PathStartAnimation(threeD);
  tvBaseUpdate (threeD, TV_PATHS);
  tvBaseStatus( threeD, "" );
}

/*
 * Doc++ Description: *//**
 *   Unloads any currently loaded flight path, frees all flight
 *   path memory, and deletes any animation timers. Use this
 *   call to unload a flight path.
 *
 * @param threeD  handle to rendering state
 *
 * @return void
 *
 */
void
PathReset( tvBase *threeD )
{
  PathFree(threeD->path);
  threeD->path = NULL;
  threeD->playing = FALSE;

  PathActivateAnimation (threeD, 0);
}

/*
 * Doc++ Description: *//**
 *  Returns boolean result indicating if a path playback is currently in 
 *  progress.
 *
 * @param threeD  handle to rendering state.
 *
 * @return  boolean result:  1 if playback in progress,  0 if no playback in 
 *          progress.
 *
 */
int
PathIsPlaying( tvBase const *threeD )
{
  return threeD->playing;
}

/*
 * Doc++ Description: *//**
 *   Returns boolean result indicating if a path is currently established for 
 *   a rendering thread.
 *
 * @param threeD  handle to rendering state.
 *
 * @return  boolean result:  1 if playback in progress,  0 if no playback in 
 *          progress.
 *
 */
int
PathIsActive ( tvBase const *threeD )
{
  return threeD->playing && ! threeD->flying;
}


double
PathGetTimeIndex( tvBase const *threeD )
{
  return threeD->curr_index;
}

/*
 * Doc++ Description: *//**
 *   Draws the projection of a given path upon the surface defined by the 
 *   given geo pyramid. In effect, this function will go through each
 *   absolute geographic coordinate in a path, transform this into the
 *   local vertical coordinate system for the specified dataset, and
 *   then produce glVertex2f() calls for the resulting 2d (x,y) coordinates.
 *   Note, these coordinates are with respect to the origin of the 
 *   dataset. This function was produced for drawing 2D lines over a
 *   digital map.
 *
 * @param path   handle to an existing path.
 * @param geo    geoset upon which to draw.
 *
 * @return void
 *
 * @see    TsGeoPyramid
 *
 */
void
PathDraw2D( Path *path, TsGeoPyramid *geo )
{
  int i;
  float lvcs[3];

  if ( path == NULL || path->count < 2 ) return;
  
  glBegin( GL_LINE_STRIP ); {
    for ( i = 0 ; i < path->count; i++ ) {
      PathItem *item = &path->item[i];
      TsGeoGCCtoLVCS( geo, item->x, item->y, item->z, lvcs );
      glVertex2f( lvcs[0], lvcs[1] );
    }
  } glEnd();

}

void
PathDraw3D( Path *path, tvViewpoint *fromView )
{
  int i;
  tvViewpoint view;

  if ( path == NULL || path->count < 2 ) return;
  
  /* draw lines */
  glColor4ub(0x00, 0xFF, 0x10, 0xFF);
  glLineWidth((GLfloat)(2));
  glBegin( GL_LINE_STRIP ); {
    if (path->interpolate == PATH_INTERPOLATE_LINEAR) {
      
      for ( i = 0 ; i < path->count; i++ )
	glVertex3f( path->item[i].x, path->item[i].y, path->item[i].z );
    }
    else if (path->interpolate == PATH_INTERPOLATE_SPLINE) {
      double index;
      PathItem item;

      for ( index = 0 ; index <= 1.0; index += 0.01 ) { /* make 100 steps */
	PathInterpolate( path, index, &item );
	glVertex3f( item.x, item.y, item.z );
      }
    }

  } glEnd();

  /* and draw vertices */
  glColor4ub(0xA0, 0x00, 0xA0, 0xFF);
  glPointSize((GLfloat)(5));
  glBegin( GL_POINTS ); {
    for ( i = 0 ; i < path->count; i++ )
      glVertex3f( path->item[i].x, path->item[i].y, path->item[i].z );
  } glEnd();

  /* then frustums */
  view.farPlane = 500;
  view.nearPlane = 0;
  view.aspect = fromView->aspect;
  view.fov = fromView->fov;
  for ( i = 0 ; i < path->count; i++ ) {
    view.eye[0] = path->item[i].x;
    view.eye[1] = path->item[i].y;
    view.eye[2] = path->item[i].z;
    view.yaw = path->item[i].yaw;
    view.pitch = path->item[i].pitch;
    view.roll = path->item[i].roll;
    view.reference_plane[0] = fromView->reference_plane[0];
    view.reference_plane[1] = fromView->reference_plane[1];
    view.reference_plane[2] = fromView->reference_plane[2];
    view.reference_plane[3] = fromView->reference_plane[3];
    tvBaseDrawFrustum(&view, FALSE);
  }
}

/*
 * Doc++ Description: *//**
 *   Performs a smooth animation from the current viewpoint to the new
 *   location. Parameters (x,y,z) are meters, and (yaw,pitch,roll) are
 *   radians.
 *
 * @param threeD  Handle to rendering thread
 * @param x       new viewpoint x coordinate
 * @param y        "      "     y     "
 * @param z        "      "     z     "
 * @param yaw     yaw in radians
 * @param pitch   pitch in radians
 * @param roll    roll in radians
 *
 * @return void
 *
 */
void
tvFlyTo(  tvBase *threeD, double x, double y, double z, 
	  double yaw, double pitch, double roll )
{
  PathItem *start, *finish;
  tvViewpoint view;
  Path     *path = threeD->path;

  /* Save the current path and then signal we are doing a fly-to anim */

  if ( ! threeD->flying ) {
    threeD->old_path = path;
    threeD->path = path = NULL;
    threeD->flying = FALSE;
  }

  /* Initialise the fly-to animation coordinates */

  PathStop (threeD);      /* stop any current flight path anim */

  /* allocate a path with two entries in it */
  if (path) PathFree (path);
  threeD->path = path = PathAlloc( 2 );

  path->speed_scaling = FALSE;
  start  = &path->item[0];
  finish = &path->item[1];

  tvCurrentView (threeD, &view);

  start->x     = view.eye[0];   /* grab the current viewpoint */
  start->y     = view.eye[1];
  start->z     = view.eye[2];
  start->yaw   = view.yaw;
  start->pitch = view.pitch;
  start->roll  = view.roll;

  finish->x = x;          /* copy the destination viewpoint */
  finish->y = y;
  finish->z = z;
  finish->yaw = yaw;
  finish->pitch = pitch;
  finish->roll = roll;

  PathPreProcess(threeD, path); /* calc total distance and time indices */

  /* Make the animation happen in a set time period */
  /* We make it zoom faster if the point is nearer  */

  if ( path->distance < 100.0 )
    path->speed = path->distance * 2.0;
  if ( threeD->path->distance < 500.0 )
    path->speed = path->distance;
  if ( threeD->path->distance < 1000.0 )
    path->speed = path->distance / 2.0;
  else
    path->speed = path->distance / 3.0;

  /* Let's set things going... */
  threeD->flying = TRUE;
  PathPlayNoCheck( threeD );
  tvBaseStatus( threeD, NULL );
}

void
PathSetDefaultDir( char *directory )
{
  char *prev = default_path_dir;
  default_path_dir = ( directory ) ? strdup( directory ) : NULL;
  if ( prev ) free( prev );
}
