/*
* Name: VIEWPOINT.C
*
* Description: Widget and database management for viewpoint bookmarks
*
* Function List:
*   void ViewpointGetView( ViewpointItem *item )
*   void ViewpointSetView( ViewpointItem *item )
*
* Dependencies:
*
* Revision History:
*     $Date: 2002/10/31 05:27:10 $
*     $Author: reddy $
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
*  Contributor(s): Michael Eriksen
*  ----- END LICENSE BLOCK -----
*
*/

#include "TerraVisionInc.h"
#include "Viewpoint.h"

#include <tsm/tsm.h>

#include <tvutils/tvutils.h>

/* a macro wrapper to the generic error report dialog in Dialogs.c */

static void ViewpointGetView( tvBase *, ViewpointItem * );
static void ViewpointSetView( tvBase *, ViewpointItem * );

/*
* Synopsis: 
*   void ViewpointGetView( tvBase *, ViewpointItem *item )
*
* Description:
*   Works out the current 3D widget viewpoint position and orientation
*   and writes these values into the ViewpointItem that is
*   passed as a parameter.
*
* Returns: void
*
* Author: Martin Reddy
*
* Date: 29 May 1998
*
*/

static void
ViewpointGetView( tvBase *tvbase, ViewpointItem *item )
{
  static char const funcName[] = "ViewpointGetView";
  tvViewpoint              view;
	
  if ( item == NULL ) return;
	
  tvBaseGetView( tvbase, &view );
  item->x     = view.eye[0];
  item->y     = view.eye[1];
  item->z     = view.eye[2];
  item->yaw   = view.yaw;
  item->pitch = view.pitch;
  item->roll  = view.roll;
}

/*
* Synopsis:
*   void ViewpointSetView( tvBase *, ViewpointItem *item )
*
* Description:
*   Takes a ViewpointItem and sets the current 3D widget's
*   viewpoint based upon the values inside this structure. This
*   is called whenever the user presses the Set View button.
*
* Returns: void
*
* Author: Martin Reddy
*
* Date: 29 May 1998
*
*/

static void
ViewpointSetView( tvBase *tvbase, ViewpointItem *item )
{
  static char const funcName[] = "ViewpointSetView";
  TsGeoPyramid      *geo;
  char              *label;
  int               labelLength;
	
  if ( tvbase == NULL || item == NULL )
    return;
	
  /* get the TsGeoPyramid structure for this bookmark's geoset */
	
  if ( ! ( geo = tvGetDataset( tvbase, item->dataset ) ))
    return;
	
  /* convert bookmark to GCC if currently in a local coord system */
	
  if ( item->coordsys == TVU_LVCS ) {
    double pos[3];
    TsGeoLVCStoGCC( geo, item->x, item->y, item->z, pos );
		
    TSMTRACE(( THREED_MASK|1, funcName, 
			"Viewpoint %s:%s %g,%g,%g -> %.0f,%.0f,%g",
			TsGeoName(geo), item->name, item->x, item->y, item->z,
			pos[0], pos[1], pos[2] ));
		
    item->x = pos[0]; item->y = pos[1]; item->z = pos[2];
    item->coordsys = TVU_GCC;
  }
	
  /* update the status bar with the name of this bookmark */
	
  labelLength = strlen( item->name ) + strlen( item->dataset ) + 10;
  if ( ( label = tsmMemMalloc( labelLength ) ) == NULL ) return;
  sprintf( label, "%s - %s", item->name, item->dataset );
	
  tsmMemFree( label );
  
  /* are we doing a fly-to animation to the new bookmark? */
	/*
  if ( animateBookmarks ) {
	FlyToLocation( tvbase, item->x, item->y, item->z, 
	item->yaw, item->pitch, item->roll );
	return;
  }
	*/
  /* if not, then just jump straight to the new location */
	
  tvSetView3d ( tvbase, 
		item->x, item->y, item->z,
		item->yaw, item->pitch, item->roll);
}

/*
* Synopsis:
*   void int GetViewpointOrigin( tvBase *threeDCoew, double *x,
*                                double *y, double *z, double *yaw,
*                                double *pitch, double *roll )
*
* Description:
*   This function will check to see if there is a bookmark called "Origin"
*   in the list of currently loaded bookmarks. If there is one, then
*   it will fill in the values of location and orientation and return a
*   value of TRUE. Otherwise, the parameter values are not changed and
*   a value of FALSE is returned.
*
*   In the case of multiple datasets, this function will look for a
*   bookmark called Origin in the first dataset in the list of loaded
*   datasets for the particular tvbase.
*
* Returns: TRUE if details have been returned for "Origin" bookmark.
*
* Author: Martin Reddy
*
* Date: 29 May 1998
*
*/

int
GetViewpointOrigin( double xyz[3], float *yaw, float *pitch, float *roll )
{
  static const char funcName[] = "GetViewpointOrigin";
  ViewpointItem *origin;
  char const        *geoname;
  list_iter_t       i;
  TsGeoPyramid      *geo;
	
  /* get the first geoset from the list of geos for this widget */
	
  i = list_first( geoList );
  geo = list_value( i );
  if ( geo == NULL ) 
    return FALSE;
	
  /* do we have a bookmark called "Origin"? */
	
  geoname = TsGeoName( geo );
	
  if ( ( origin = tvuFindViewpoint( geoname, "Origin" ) ) == NULL )
    return FALSE;
	
  /* If we do, then return its position and orientation */
	
  if ( origin->coordsys == TVU_LVCS ) {
    TsGeoLVCStoGCC ( geo, origin->x, origin->y, origin->z, xyz);
		
    TSMTRACE(( THREED_MASK|1, funcName, 
			"Viewpoint %s:%s %g,%g,%g -> %.0f,%.0f,%g", geoname,
			origin->name, origin->x, origin->y, origin->z,
			xyz[0], xyz[1], xyz[2] ));
  } else {
    xyz[0] = origin->x;
    xyz[1] = origin->y;
    xyz[2] = origin->z;
  }
	
  *yaw   = origin->yaw;
  *pitch = origin->pitch;
  *roll  = origin->roll;
  
  return TRUE;
}

/*
* Synopsis:
*   static int ViewpointCountBookmarks( TsGeoPyramid const *geo )
*
* Description:
*   This function iterates through the list of all bookmarks and 
*   calculates how many of these are for the dataset being displayed
*   in the specified tvbase. 
*
* Returns: number of bookmarks for dataset, or 0 if none/error
*
* Author: Martin Reddy
*
* Date: 27 March 1999
*
*/

static int
ViewpointCountBookmarks( TsGeoPyramid const *geo )
{
  char const *dataset;
  int  i, count;
  
  if ( geo == NULL ) return 0;
	
  /* Work out the name of the current dataset */
	
  dataset = TsGeoName( geo );
	
  /* count all of the bookmarks with the same dataset name as this one */
	
  for ( i = count = 0; i < tvuViewpointCount(); i++ )
    if ( strcasecmp( tvuViewpoint(i)->dataset, dataset ) == 0 )
      count++;
		
		return count;
}


/*
* Synopsis:
*   void ViewpointCycleUp( tvBase *tvbase )
*
* Description:
*   This function will cycle up through the list of bookmarks for
*   the current dataset and will then jump to this previous bookmark.
*   If the user has the Animate Bookmarks option checked, then 
*   we will fly to the new viewpoint, otherwise we jump there
*   directly.
*
* Returns: void
*
* Author: Martin Reddy
*
* Date: 27 March 1999
*
*/

void
ViewpointCycleUp( tvBase *tvbase, int *currBookmark )
{
  int i = currBookmark, j;
  int size = tvuViewpointCount();
  ViewpointItem *item;
	
  for ( j = 0; j < size; j++ ) {
    i--;
    if ( i >= size || i < 0 ) i = size - 1;
    item = tvuViewpoint(i);
		
    if ( tvGetDataset( tvbase, item->dataset ) ) {
      ViewpointSetView( tvbase, item );
      currBookmark = i;
      return;
    }
  }
}

/*
* Synopsis:
*   void ViewpointCycleDown( tvBase *tvbase )
*
* Description:
*   This function will cycle down through the list of bookmarks for
*   the current dataset and will then jump to this next bookmark.
*   If the user has the Animate Bookmarks option checked, then 
*   we will fly to the new viewpoint, otherwise we jump there
*   directly.
*
* Returns: void
*
* Author: Martin Reddy
*
* Date: 27 March 1999
*
*/

void
ViewpointCycleDown( tvBase *tvbase, int *currBookmark )
{
  int i = currBookmark, j;
  int size = tvuViewpointCount();
  ViewpointItem *item;
	
  for ( j = 0; j < size; j++ ) {
    i++;
    if ( i >= size || i < 0 ) i = 0;
    item = tvuViewpoint(i);
		
    if ( tvGetDataset( tvbase, item->dataset ) ) {
      ViewpointSetView( tvbase, item );
      currBookmark = i;
      return;
    }
  }
}
