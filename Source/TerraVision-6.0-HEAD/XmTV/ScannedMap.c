/*
 * Name: 
 *    	ScannedMap.c
 *
 * Description:
 *	Creation and manipulation of a widget to display a scanned map
 *
 * Function List:
 *
 * Dependencies:
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

#include <Xm/ScrolledW.h>
#include <tsm/tsm.h>
#include "ScannedMap.h"
#include "Database.h"
#include "Dialogs.h"
#include "BusyDialog.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "PathsFE.h"
#include "Link.h"

#include "X11Include.h"

#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>

static ScannedMapWidget *CreateScannedMap( Widget );

static void ScannedMapInitCallback(Widget, XtPointer, XtPointer);
static void ScannedMapResize (ScannedMapWidget *scannedMap,
				    GLuint width, GLuint height);
static void ScannedMapResizeCallback(Widget, XtPointer, XtPointer);
static void ScannedMapExposeCallback(Widget, XtPointer, XtPointer);
static void ScannedMapDismissCB(Widget, XtPointer, XtPointer);

static void ScannedMapDraw(glmWindow glmw, void * closure);
static void ScannedMapDrawPath(ScannedMapWidget *);
static void ScannedMapPostRedraw (ScannedMapWidget * scannedMap);
static void ScannedMapCreateUtilButtons( ScannedMapWidget *, Widget );
static int ScannedMapCreateWidgets(ScannedMapWidget *, Widget, glmContext);

static XtActionProc ScannedMapBtn1Up(Widget, XEvent *, GLbyte * *, Cardinal *);
static void ScannedMapMenuCB(Widget, XtPointer, XtPointer);
static void ScannedMapUpdateMenus( void );

static void ScannedMapLoadSuccess( void *closure );
static void ScannedMapLoadImageCB( BusyDialog *busy );
static int ScannedMapLoadImage( ScannedMapWidget *, TsmMap *, char const * );
static void ScannedMapUnloadCB( Widget, XtPointer, XtPointer );

static void MapSelectInit( ScannedMapWidget *map );
static void MapSelectShowCB( Widget, XtPointer, XtPointer );
static void MapSelectHideCB( Widget, XtPointer, XtPointer );
static void MapSelectCB( Widget, XtPointer, XtPointer );
static void MapSelectRefresh( ScannedMapWidget *map );

static int NumAvailMaps( void );
static int GetMapSelection( ScannedMapWidget *map, char **url );
static char *ScannedMapGeoSet( ScannedMap *map );
static TsGeoPyramid const *ScannedMapGeo( char const *geo_name );
static void ScannedMapDrawFrustum3d( tvBase *, TsGeoPyramid * );
static void ScannedMapDrawFrustum2d( tvBase *, TsGeoPyramid * );

/* Save the max dimensions of the map viewer so we can enfore a max size */
static Dimension maxWidth = 0, maxHeight = 0;

/* The widget structures for the menu buttons */
static Widget showCoverageWidget;
static Widget showFrustumWidget;
static Widget showPathWidget;

/* Now the display state for the Map Display */

static int showCoverage = FALSE;
static int showFrustum  = FALSE;
static int showPath     = FALSE;

/* Displayer for scanned maps */
ScannedMapWidget *scannedMap;

/*
 * Synopsis:
 *   ScannedMapWidget *CreateScannedMap( Widget )
 *
 * Description:
 *   Creates a scanned map widget. By default this will have no map
 *   image associated with it.
 *
 * Externals:
 *
 * Returns: A pointer to the new ScannedMapWidget on success
 *        NULL on failure
 *
 * Author: Stephen Lau
 * 
 * Date: July 21, 1993
 *
 */

static ScannedMapWidget *
CreateScannedMap( Widget parent )
{
  static char  *funcName = "CreateScannedMap";
  Display      *display = XtDisplay( parent );
  Screen       *screen  = XtScreen( parent );

  glmContext   glmCtx = glmXFindContext (display, screen, True, 0);

#ifdef CAVE
  if ( cave ) return NULL;   /* no map viewer for CAVE */
#endif

  /* create the new ScannedMapWidget structure */

  scannedMap = (ScannedMapWidget *) tsmMemCalloc( 1,sizeof(ScannedMapWidget) );
  if ( scannedMap == NULL ) {
    tsmError( funcName, "Can't create map viewer widget!" );
    return NULL;
  }

  /* initialse the image size variables to show something */
  /* until the user actually selects a valid map.         */

  scannedMap->map.sizeX  = 5;
  scannedMap->map.sizeY  = 5;
  scannedMap->map.ncomps = 1;
  scannedMap->map.format = GL_LUMINANCE;

  /* create the map viewer window */

  if ( ! ScannedMapCreateWidgets( scannedMap, parent, glmCtx ) ) {
    tsmError( funcName, "Unable to create map viewer widgets" );
    return NULL;
  }

  return scannedMap;
}

/*
 * Synopsis:
 *   void ScannedMapUnloadCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   Unloads from memory any map image that is currently loaded
 *   and dismisses the viewer so that no map is displayed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
ScannedMapUnloadCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ScannedMapWidget *scannedMap = (ScannedMapWidget *) client_data;
  ScannedMap       *map = &scannedMap->map;

  if ( map == NULL || map->mapdata == NULL ) return;

  /* free the memory for the current image */

  tsmMemFree( map->mapdata );

  /* setup default values for when there is no image loaded */

  map->mapdata = NULL;
  map->sizeX   = 5;
  map->sizeY   = 5;
  map->ncomps  = 1;
  map->format  = GL_LUMINANCE;

  /* now pop down the Map Viewer */

  ScannedMapDismissCB( NULL, scannedMap, NULL );
}

/*
 * Synopsis:
 *   int ScannedMapCreateWidgets()
 *
 * Description:
 *   Creates all the widgets for the Map Viewer
 *
 * Externals:
 *
 * Returns: TRUE on success, FALSE on failure
 *
 * Author: Stephen Lau
 *
 * Date: 1993
 *
 */

static int
ScannedMapCreateWidgets( ScannedMapWidget *scannedMap, Widget parent,
			 glmContext glmCtx )
{
  /*static char const funcName[] = "ScannedMapCreateWidgets";*/
  Widget         topForm, formWidget;
  Widget         menuBar, menuPane, cascade, widget;
  XmString       title;
  XtTranslations transTable;
  XtAppContext 	 appContext;
  Arg            args[10];
  int            n = 0;
  XtCallbackRec  callbacks[] = {{NULL, NULL}, {NULL, NULL}};
  static char ScannedMapTranslations[] = "<Btn1Up>:	ScannedMapBtn1Up() \n";
  static XtActionsRec ScannedMapActionsTable[] = {
    {"ScannedMapBtn1Up", (XtActionProc) ScannedMapBtn1Up}
  };

  if ( scannedMap->mainForm != NULL ) return TRUE;

  title = tvXmString( "TerraVision II: Map Viewer" );
  XtSetArg(args[n], XmNdialogTitle, title ); n++;
  scannedMap->mainForm =
    XmCreateFormDialog(parent, "ScannedMapForm", args, n);
  XmStringFree( title );

  /* Add actions for the widget */

  appContext = XtWidgetToApplicationContext(parent);
  XtAppAddActions(appContext, 
		  ScannedMapActionsTable,
		  XtNumber(ScannedMapActionsTable));
  transTable = XtParseTranslationTable(ScannedMapTranslations);

  
  topForm = XtVaCreateManagedWidget( "topWidget",
				     xmFormWidgetClass, scannedMap->mainForm,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_FORM,
				     NULL );

  /* Add the Map Display menu bar */

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;

  menuBar = XmCreateMenuBar( topForm, "menuBar", args, n);
  XtManageChild( menuBar );

  /* Create the Maps menu bar */

  n = 0;
  XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
  menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

  callbacks[0].callback = ScannedMapMenuCB;
  callbacks[0].closure = NULL;

  widget =
    XtVaCreateManagedWidget( "Select Map",
			     xmPushButtonGadgetClass, menuPane,
			     XmNmnemonic, 'S',
			     NULL );
  XtAddCallback( widget, XmNactivateCallback, MapSelectShowCB, scannedMap );

  widget =
    XtVaCreateManagedWidget( "Unload Map",
			     xmPushButtonGadgetClass, menuPane,
			     XmNmnemonic, 'U',
			     NULL );
  XtAddCallback( widget, XmNactivateCallback, ScannedMapUnloadCB, scannedMap );

  /* Put a dismiss menu entry in there just for kicks */

  callbacks[0].callback = ScannedMapDismissCB;
  callbacks[0].closure = (XtPointer) scannedMap;

  XtVaCreateManagedWidget( "sep", xmSeparatorGadgetClass, menuPane, NULL );

  XtVaCreateManagedWidget( "Dismiss",
			   xmPushButtonGadgetClass, menuPane,
			   XmNactivateCallback, callbacks,
			   XmNmnemonic, 'D',
			   NULL );

  n = 0;
  XtSetArg( args[n], XmNsubMenuId, menuPane ); n++;
  XtSetArg( args[n], XmNmnemonic, 'M' ); n++;
  cascade = XmCreateCascadeButton( menuBar, "Maps", args, n );
  XtManageChild( cascade );

  /* Create the Options menu bar */

  n = 0;
  XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
  menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

  showCoverageWidget =
    XtVaCreateManagedWidget( "Show Coverage",
			     xmToggleButtonGadgetClass, menuPane,
			     XmNindicatorOn, True,
			     XmNvisibleWhenOff, True,
			     XmNset, (showCoverage) ? True : False,
			     XmNmnemonic, 'C',
			     NULL );
  XtAddCallback( showCoverageWidget, XmNvalueChangedCallback,
		 ScannedMapMenuCB, NULL );

  showFrustumWidget =
    XtVaCreateManagedWidget( "Show Frustum",
			     xmToggleButtonGadgetClass, menuPane,
			     XmNindicatorOn, True,
			     XmNvisibleWhenOff, True,
			     XmNset, (showFrustum) ? True : False,
			     XmNmnemonic, 'F',
			     NULL );
  XtAddCallback( showFrustumWidget, XmNvalueChangedCallback,
		 ScannedMapMenuCB, NULL );

  showPathWidget =
    XtVaCreateManagedWidget( "Show Path",
			     xmToggleButtonGadgetClass, menuPane,
			     XmNindicatorOn, True,
			     XmNvisibleWhenOff, True,
			     XmNset, (showPath) ? True : False,
			     XmNmnemonic, 'P',
			     NULL );
  XtAddCallback( showPathWidget, XmNvalueChangedCallback,
		 ScannedMapMenuCB, NULL );

  n = 0;
  XtSetArg( args[n], XmNsubMenuId, menuPane ); n++;
  XtSetArg( args[n], XmNmnemonic, 'O' ); n++;
  cascade = XmCreateCascadeButton( menuBar, "Options", args, n );
  XtManageChild( cascade );

  /* create the "Help" menu */

  n = 0;
  XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
  menuPane = XmCreatePulldownMenu (menuBar, "menuPane", args, n);

  widget = XtVaCreateManagedWidget( "Help on Map Viewer",
				    xmPushButtonGadgetClass, menuPane,
				    XmNmnemonic, 'H', NULL );
  XtAddCallback( widget, XmNactivateCallback, TerraVisionHelpCB,
		 (XtPointer) "mapviewer.shtml" );

  widget = XtVaCreateManagedWidget( "Help on TerraVision",
				    xmPushButtonGadgetClass, menuPane,
				    XmNmnemonic, 'T', NULL );
  XtAddCallback( widget, XmNactivateCallback, TerraVisionHelpCB, NULL );

  n = 0;
  XtSetArg (args[n], XmNsubMenuId, menuPane); n++;
  XtSetArg (args[n], XmNmnemonic, 'H'); n++;
  cascade = XmCreateCascadeButton (menuBar, "Help", args, n);
  XtManageChild(cascade);

  /* Tell Motif that this last menu was the Help menu - this */
  /* will make Motif display it on the right side of the bar */

  XtVaSetValues( menuBar, XmNmenuHelpWidget, cascade , NULL );

  /* Create the scrolled window widget for the map image */

  scannedMap->mapFormWidget = formWidget =
    XtVaCreateManagedWidget("scrolledform", 
			    xmScrolledWindowWidgetClass, 
			    scannedMap->mainForm,
			    XmNscrollingPolicy, XmAUTOMATIC,
			    /*XmNscrollBarDisplayPolicy, XmSTATIC,*/
			    XmNwidth, scannedMap->map.sizeX,
			    XmNheight, scannedMap->map.sizeY,
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, menuBar,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNleftAttachment, XmATTACH_FORM,
			    NULL);

  scannedMap->mapImageWidget = 
    XtVaCreateManagedWidget("ScannedMapImageWidget",
			    glwMDrawingAreaWidgetClass,
			    formWidget,
			    GLwNvisualInfo, glmXContextVisualInfo (glmCtx),
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNwidth, scannedMap->map.sizeX,
			    XmNheight, scannedMap->map.sizeY,
			    XmNleftOffset, 4,
			    XmNrightOffset, 4,
			    XmNbottomOffset, 4,
			    XmNtopOffset, 4,
			    NULL);
 
  /* CREATE AN OPENGL CONTEXT FOR THIS ONE. */
  scannedMap->glmw = glmXtCreateWindow (scannedMap->mapImageWidget,
					glmCtx, True, False);
  glmRegisterRedraw (scannedMap->glmw, ScannedMapDraw, scannedMap);

  /* ADD GL CALLBACKS */

  XtAddCallback(scannedMap->mapImageWidget,
		GLwNexposeCallback, 
		ScannedMapExposeCallback, 
		scannedMap);

  XtAddCallback(scannedMap->mapImageWidget,
		GLwNginitCallback, 
		ScannedMapInitCallback, 
		scannedMap);

  XtAddCallback(scannedMap->mapImageWidget,
		GLwNresizeCallback, 
		ScannedMapResizeCallback, 
		scannedMap);

  /* OVERRIDE TRANSLATIONS FOR THE MAP IMAGE */
  XtOverrideTranslations(scannedMap->mapImageWidget, transTable);

  ScannedMapCreateUtilButtons(scannedMap, formWidget);

  /* update the menu entry names for the initial state */
  ScannedMapUpdateMenus();
 
  return TRUE;
}

/*
 * Synopsis:
 *   void ScannedMapShow( void )
 *
 * Description:
 *   Displays the Map Viewer if there are maps available. If no
 *   map is currently loaded, then we automatically pop up the
 *   Map Select dialog too.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 December 1998
 *
 */

void
ScannedMapShow( void )
{
  const char  *funcName = "ScannedMapShow";
  static char *msg = "No maps are available for the loaded datasets";
  XmString    string;

  /* check to see if we actually have any maps to display */

  if ( NumAvailMaps() == 0 ) {
    if ( warningDialog == NULL ) return;
    string = tvXmString( msg );
    XtVaSetValues( warningDialog, XmNmessageString, string, NULL );
    XtManageChild( warningDialog );
    XmStringFree( string );
    return;
  }

  /* create the map viewer if this is the first time round */

  if ( scannedMap == NULL ) {
    if ( ( scannedMap = CreateScannedMap( applicationShell ) ) == NULL ) {
      tsmError( funcName, "Unable to create the scanned map structure!" );
      return;
    }
  }

  /* if we have no map image loaded, then pop up a list of avail maps */

  if ( scannedMap == NULL || scannedMap->map.mapdata == NULL ) {
    MapSelectShowCB( NULL, scannedMap, NULL );
    return;
  }

  /* otherwise, display the map viewer */

  if ( scannedMap->mainForm != NULL )
    XtManageChild( scannedMap->mainForm );

}

/*
 * Synopsis: void ScannedMapUpdateMenus( void )
 *
 * Description: Updates menu entries depending upon current state
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 10 December 1998
 *
 */

static void
ScannedMapUpdateMenus( void )
{
  XtVaSetValues( showCoverageWidget, XmNset, showCoverage, NULL );
  XtVaSetValues( showFrustumWidget, XmNset, showFrustum, NULL );
  XtVaSetValues( showPathWidget, XmNset, showPath, NULL );
}

/*
 * Synopsis: void ThreeDWidgetMenuCB( Widget, XtPointer, XtPointer )
 *
 * Description: Callback functions for the Map Viewer menu buttons
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 10 December 1998
 *
 */

static void
ScannedMapMenuCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  /*static char const funcName[] = "ScannedMapMenuCB";*/

  if ( w == showCoverageWidget )
    showCoverage = ! showCoverage;
  else if ( w == showFrustumWidget )
    showFrustum = ! showFrustum;
  else if ( w == showPathWidget )
    showPath = ! showPath;

  ScannedMapUpdateMenus();
}

/*
 * Synopsis:
 *   void ScannedMapLoadSuccess( void *closure )
 *
 * Description:
 *   This function is called whenever a new map image has been loaded.
 *   It is called as a follow-up callback from the BusyDialog thread
 *   that performs that actual loading. This function contains all 
 *   of the routines to refresh the X state because these cannot
 *   appear in the BusyDialog (multi-threading + X = bad!).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 July 1999
 *
 */

static void
ScannedMapLoadSuccess( void *closure )
{
  /*static char const funcName[] = "ScannedMapLoadSuccess";*/
  ScannedMapWidget *scannedMap = (ScannedMapWidget *) closure;
  ScannedMap       *map = &scannedMap->map;

  /* Resize the appropriate X and OpenGL widgets */

  maxWidth = maxHeight = 0;

  XtVaSetValues( scannedMap->mapFormWidget, XmNwidth, map->sizeX,
		 XmNheight, map->sizeY, NULL );
  XtVaSetValues( scannedMap->mapImageWidget, XmNwidth, map->sizeX,
		 XmNheight, map->sizeY, NULL );

  /* pop up the Map View widget if it is not already managed */

  if ( scannedMap->mainForm != NULL && ! XtIsManaged( scannedMap->mainForm ) )
    XtManageChild( scannedMap->mainForm );

  /* update the dimensions of the dialog and refresh its contents */

  ScannedMapResize( scannedMap, map->sizeX, map->sizeY );
}

/*
 * Synopsis:
 *   void ScannedMapLoadImageCB( BusyDialog *busy )
 *
 * Description:
 *   This is the BusyDialog thread that performs the actual
 *   loading of a new map image. It is called by the function
 *   ScannedMapLoadImage. If everything loads fine, then we
 *   install the new map and then, once this thread finishes,
 *   we update the X state via ScannedMapLoadSuccess().
 *
 *   The closure for this thread is a ScannedMap structure
 *   where the min/max (u,v) coords have been specified, along
 *   with the name and url for the map. All other fields need
 *   to be filled in once we have loaded the image.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 July 1999
 *
 */

static void
ScannedMapLoadImageCB( BusyDialog *busy )
{
  static const char *funcName = "ScannedMapLoadImageCB";
  ScannedMap        *new_map = (ScannedMap *) busy->closure;
  ScannedMap        *map = &scannedMap->map;
  Image_t           *image;
  char              *geo_name;

  /* First of all, try to load the new image */

  if ( ( image = tsmReadImage( new_map->map_url ) ) == NULL ) {
    char message[512];
    sprintf( message, "Unable to read the image file for map: %s\n%s\n"
	     "The server may be down, or your database may be out of date",
	     new_map->map_name, new_map->map_url );

    tsmError( funcName, "Unable to read image: %s", new_map->map_url );
    SetBusyResult( busy, message );
    CloseBusyDialog( busy );
  }

  /* if it worked, then set this as the current map viewer image */

  if ( map->mapdata ) tsmMemFree( map->mapdata );
  if ( map->map_name ) tsmMemFree( map->map_name );
  if ( map->map_url ) tsmMemFree( map->map_url );

  map->mapdata      = (GLubyte *) image->bitmap;
  map->sizeX        = image->width;
  map->sizeY        = image->height;
  map->ncomps       = image->components;
  map->format       = ( map->ncomps == 1 ) ? GL_LUMINANCE : GL_RGB;

  map->minCoords[0] = new_map->minCoords[0];
  map->minCoords[1] = new_map->minCoords[1];
  map->maxCoords[0] = new_map->maxCoords[0];
  map->maxCoords[1] = new_map->maxCoords[1];
  map->map_name     = new_map->map_name;
  map->map_url      = new_map->map_url;

  TSMTRACE(( MAP_MASK|1, funcName, "u,v extent = (%g,%g) -> (%g,%g)",
	      map->minCoords[0], map->minCoords[1],  map->maxCoords[0],
	      map->maxCoords[1] ));

  /* work out the tspec of the OI that this map is associate with */

  if ( ( geo_name = ScannedMapGeoSet( map ) ) != NULL )
    map->geo = (TsGeoPyramid *) ScannedMapGeo( geo_name );
  else
    map->geo = NULL;

  /* need to invert the image for OpenGL */

  gltInvertImage( map->mapdata, map->sizeX, map->sizeY, map->ncomps );

  /* specify a follow-up function to do the X stuff in the X thread */

  SetBusyFollowup( busy, ScannedMapLoadSuccess, scannedMap );

  /* signal that we have finished loading the image */

  map->loading = FALSE;

  /* free the temporary structures and finish up */

  tsmFreeImage( image, FALSE /* don't free image->bitmap */ );
  tsmMemFree( new_map );

  CloseBusyDialog( busy );
}

/*
 * Synopsis:
 *   void ScannedMapLoadImage( ScannedMapWidget *, TsmMap *, char const * )
 *
 * Description:
 *   This function initiates the loading of a map image. It is currently
 *   called by the callback which is invoked when the user double-clicks
 *   over a map name in the Map Select dialog.
 *
 *   In this function, we set everything up for the new image and then
 *   fire off a BusyDialog thread in orer to do the loading (because we
 *   could be loading the image from http and it may take some time).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 July 1999
 *
 */

static int
ScannedMapLoadImage( ScannedMapWidget *scannedMap, TsmMap *tsm_map,
		     char const *url )
{
  /*static const char funcName[] = "ScannedMapLoadImage";*/
  ScannedMap       *map = &scannedMap->map, *new_map;
  static Widget    map_error = NULL;
  char             buffer[512];

  /* Check to see if this map is already being displayed */

  if ( map->map_url && strcmp( map->map_url, url ) == 0 && map->mapdata ){
    GenericErrorDialog( scannedMap->mainForm, &map_error,
			"This map is already loaded" );
    return TRUE;
  }

  /* allocate a new ScannedMap structure to contain the new info */

  if ( ! ( new_map = (ScannedMap *) tsmMemMalloc( sizeof(ScannedMap) ) ) )
    return FALSE;

  /* Set up the new ScannedMap structure for the new image. This */
  /* acts as a closure that we pass into the BusyDialog thread.  */

  new_map->map_name = tsmMemStrdup( tsm_map->name );
  new_map->map_url  = tsmMemStrdup( url );
  new_map->minCoords[0] = tsm_map->min_u;
  new_map->minCoords[1] = tsm_map->min_v;
  new_map->maxCoords[0] = tsm_map->max_u;
  new_map->maxCoords[1] = tsm_map->max_v;
  
  /* Now we vector off to another thread to do all of the image loading */

  sprintf( buffer, "Please wait... Loading map image from:\n%s", url );

  OpenBusyDialog( NULL, buffer, ScannedMapLoadImageCB, new_map );

  return TRUE;
}

/*
 * Synopsis: void ScannedMapThreeDWidget()
 *
 * Description: Attaches a threeDWidget to the scanned map.
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: Oct 20, 1997.
 *
 */
void
ScannedMapThreeDWidget (ScannedMapWidget * scannedMap,
			ThreeDWidget *threeDWidget)
{
  scannedMap->linked3DWidget = threeDWidget;
}

/*
 * Synopsis: void ScannedMapCreateUtilButtons()
 *
 * Description: Creates utility widgets
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	March 26, 1993
 *
 */
static void
ScannedMapCreateUtilButtons(ScannedMapWidget * scannedMap,
			    Widget topWidget)
{
  /*static char const funcName[] = "ScannedMapCreateUtilButtons";*/
  Widget bBoard, separator;
  XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};

  callbacks[0].callback = ScannedMapDismissCB;
  callbacks[0].closure = (XtPointer) scannedMap;

  separator = XtVaCreateManagedWidget("Separator", xmSeparatorGadgetClass, 
				      scannedMap->mainForm,
				      XmNheight, 4,
				      XmNshadowThickness, 4,
				      XmNtopOffset, 4,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      NULL);

  XtVaSetValues(topWidget,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, separator,
		NULL);

  bBoard = XtVaCreateManagedWidget("ScannedMapBoard", 
				   xmRowColumnWidgetClass,
				   scannedMap->mainForm,
				   XmNbottomAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNentryAlignment, XmALIGNMENT_CENTER,
				   XmNorientation, XmHORIZONTAL,
				   XmNpacking, XmPACK_COLUMN,
				   XmNspacing, 100,
				   XmNmarginWidth, 5,
				   XmNmarginHeight, 5,
				   XmNnoResize, TRUE,
				   NULL);

  XtVaSetValues(separator,
		XmNbottomAttachment, XmATTACH_WIDGET,
		XmNbottomWidget, bBoard,
		NULL);

  XtVaCreateManagedWidget("ScannedMapDismiss", 
			  xmPushButtonGadgetClass,
			  bBoard,
			  XmNleftOffset, 20,
			  XmNrightOffset, 20,
			  XmNshowAsDefault, 1,
			  XmNactivateCallback, callbacks,
			  XmNlabelString, tvXmString("Dismiss"),
			  NULL);
}

/*
 * Synopsis:
 *   void ScannedMapDismissCB()
 *
 * Description:
 *   X callback in order to dismiss a Map Viewer dialog.
 *
 * Returns: void
 *
 * Author: Stephen Lau
 * 
 * Date: March 25, 1993
 *
 */

static void
ScannedMapDismissCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  /* static const char funcName[] = "ScannedMapDismissCB";*/
  ScannedMapWidget *scannedMap = (ScannedMapWidget *) client_data;

  if ( scannedMap->mapSelectWidget &&
       XtIsManaged( scannedMap->mapSelectWidget ) )
    XtUnmanageChild( scannedMap->mapSelectWidget );

  if ( scannedMap->mainForm && XtIsManaged( scannedMap->mainForm ) )
    XtUnmanageChild( scannedMap->mainForm );
}

/*
 * Synopsis: static void ScannedMapResize(ScannedMapWidget *scannedMap,
 *                                              GLuint width, GLuint height)
 *	
 * Description: Called when widget changes size.
 *
 * Externals: None.
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: June 1997
 *
 */
static void
ScannedMapResize (ScannedMapWidget *scannedMap,
			GLuint width, GLuint height)
{
  static char const *funcName = "ScannedMapResize";

  glmWindow glmw = scannedMap->glmw;

  TSMTRACE ((MAP_MASK|1, funcName, "%p \"%s\" glmw %p %dx%d", 
	     scannedMap->mapImageWidget, XtName (scannedMap->mapImageWidget),
	     glmw, width, height));

  glmMakeCurrentWindow (funcName, glmw); {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
  }; glmReleaseWindow (funcName, glmw);

  ScannedMapPostRedraw (scannedMap);
}

/*
 * Synopsis: static void ScannedMapInitCallback()
 *	
 *
 * Description: Initialize callbavck for widget. 
 *    initializes the GL window
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
ScannedMapInitCallback(Widget w, 
			     XtPointer client_data,
			     XtPointer call_data)
{
  static char * funcName = "ScannedMapInitCallback";
  GLwDrawingAreaCallbackStruct * data = 
    (GLwDrawingAreaCallbackStruct *) call_data;
  ScannedMapWidget * scannedMap = (ScannedMapWidget *) client_data;

  TSMTRACE ((MAP_MASK|1, funcName, "%p \"%s\"", 
	     scannedMap->mapImageWidget, XtName (scannedMap->mapImageWidget)));

  /* Initialize the OpenGL window */

  ScannedMapResize (scannedMap, data->width, data->height);

}

/*
 * Synopsis: static void
 *      ScannedMapResizeCallback()
 *
 * Description: Callback for resize events
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */
static void
ScannedMapResizeCallback(Widget w, 
			       XtPointer client_data,
			       XtPointer call_data)
{
  static char * funcName = "ScannedMapResizeCallback";
  GLwDrawingAreaCallbackStruct * data = 
    (GLwDrawingAreaCallbackStruct *) call_data;
  ScannedMapWidget * scannedMap = (ScannedMapWidget *) client_data;

  TSMTRACE ((MAP_MASK|1, funcName, "%p \"%s\"", 
	     scannedMap->mapImageWidget, XtName (scannedMap->mapImageWidget)));

  ScannedMapResize (scannedMap, data->width, data->height);
}

/*
 * Synopsis: static void
 *      ScannedMapExposeCallback()
 *
 * Description: Expose event callback
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *		March 25, 1993
 *
 */

static void
ScannedMapExposeCallback( Widget w, XtPointer client_data, XtPointer call_data)
{
  static char * funcName = "ScannedMapExposeCallback";
  ScannedMapWidget * scannedMap = (ScannedMapWidget *) client_data;

  TSMTRACE ((MAP_MASK|1, funcName, "Enter"));
  ScannedMapPostRedraw(scannedMap);
}

/*
 * Synopsis: Boolean ScannedMapPostRedraw(ScannedMapWidget * scannedMap)
 *
 * Description: Signals to the widget that a redraw is required.
 *
 * Returns: void.
 *
 * Author: Lee Iverson
 *
 * Date: February 1998
 *
 */

static void
ScannedMapPostRedraw (ScannedMapWidget * scannedMap)
{
  static char * funcName = "ScannedMapPostRedraw";

  TSMTRACE ((MAP_MASK|1, funcName, ""));
  if ( ScannedMapIsManaged( scannedMap ) )
    glmPostRedraw (scannedMap->glmw);
}

/*
 * Synopsis: void ScannedMapRedraw( void )
 *
 * Description: An externally visible function to refresh the map display
 *
 * Returns: void.
 *
 * Author: Martin Reddy
 *
 * Date: 10 December 1998
 *
 */

void
ScannedMapRedraw( void )
{
  ScannedMapPostRedraw( scannedMap );
}

/*
 * Synopsis:
 *   Boolean ScannedMapIsManaged(ScannedMapWidget const * scannedMap)
 *
 * Description:
 *   Determines whether or not widget is managed.
 *
 * Returns: TRUE iff widget is managed.
 *
 * Author: Lee Iverson
 *
 * Date: May 1997
 *
 */

Boolean
ScannedMapIsManaged (ScannedMapWidget const * scannedMap)
{
  return scannedMap && scannedMap->glmw && glmIsMapped (scannedMap->glmw);
}

/*
 * Synopsis: void ScannedMapEnforceMax( void )
 *
 * Description: this function will ensure that the Map Display window
 *   does not exceed a certain maximum size. If it does, then it will
 *   be resized back to the maximum. This is a real hack! I can't see
 *   how to tell X to not let the user resize the window beyond this
 *   maximum (this only appears to work for Shells, and these can only
 *   hand on objrect).
 *
 * Externals: maxWidth, maxHeight
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 10 December 1998
 *
 */

static void
ScannedMapEnforceMax( void )
{
  Dimension width, height;

  XtVaGetValues( scannedMap->mainForm, XmNwidth, &width,
		 XmNheight, &height, NULL );

  /* Take the initial size of the window as the max size */

  if ( maxWidth == 0 || maxHeight == 0 ) {
    maxWidth = width;
    maxHeight = height;
    return;
  }

  /* Otherwise, enforce this maximum window size */

  if ( width > maxWidth )
    XtVaSetValues( scannedMap->mainForm, XmNwidth, maxWidth, NULL );

  if ( height > maxHeight )
    XtVaSetValues( scannedMap->mainForm, XmNheight, maxHeight, NULL );
}

/*
 * Synopsis:
 *   void ScannedMapDraw(glmWindow glmw, void * closure)
 *
 * Description:
 *    Redraws the map in the window
 *
 * Returns: void
 *
 * Author: Stephen Lau
 * 
 * Date: 1993
 *
 */

static void
ScannedMapDraw(glmWindow glmw, void * closure)
{
  static char         *funcName = "ScannedMapDraw";
  ScannedMapWidget    *scannedMap = (ScannedMapWidget *) closure;
  ScannedMap const    *map = &scannedMap->map;
  float               minDataX, minDataY, maxDataX, maxDataY; 

  if (! ScannedMapIsManaged (scannedMap))
    return;

  TSMTRACE ((MAP_MASK|1, funcName, "Entering"));

  ScannedMapEnforceMax();

  glmMakeCurrentWindow (funcName, scannedMap->glmw); {

    /* Clear screen */

    gltClearColor4ub (0xFF, 0xFF, 0xFF, 0xFF);
    glClear (GL_COLOR_BUFFER_BIT);

    /* Draw the map on the screen */

    glRasterPos2i(0, 0);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

    /* render the map image into the window */

    if ( map->mapdata ) {
      glDrawPixels( map->sizeX, map->sizeY, map->format, GL_UNSIGNED_BYTE,
		    map->mapdata );
    } else {

      /* if no data loaded - use a mid-gray background image instead */

      int     bytes = map->sizeX * map->sizeY;
      GLubyte *blank = tsmMemMalloc( bytes );

      if ( blank ) {
	memset( blank, 0x7f, bytes );
	glDrawPixels( map->sizeX, map->sizeY, map->format, GL_UNSIGNED_BYTE,
		      blank );
	tsmMemFree( blank );
      }

    }

    /* Change to the LVCS coordinate system for the map */

    glPushMatrix (); {
      TsBBox const *bbox;

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(map->minCoords[0], map->maxCoords[0],
	      map->minCoords[1], map->maxCoords[1],
	      -1, 1);

      TSMTRACE(( MAP_MASK|1, funcName, "u,v coords = (%g,%g) -> (%g,%g)",
		 map->minCoords[0], map->minCoords[1],  map->maxCoords[0],
		 map->maxCoords[1] ));

      bbox = TsGeoBBoxLVCS (map->geo);
      minDataX   = bbox->min[0];
      minDataY   = bbox->min[1];

      /* Draw data set extent */

      if ( showCoverage ) {

	/* find the maximum ranges of actual data in set */

	maxDataX   = bbox->max[0];
	maxDataY   = bbox->max[1];

	TSMTRACE(( MAP_MASK|1, funcName, "bbox %g,%g,%g -> %g,%g,%g",
		   bbox->min[0], bbox->min[1], bbox->min[2],
		   bbox->max[0], bbox->max[1], bbox->max[2] ));

	/* draw the transparent box for the coverage */

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
	glColor4ub (0x00, 0xFF, 0x00, 0x20);
	glRectf (minDataX, minDataY, 
		 maxDataX, maxDataY);
	glDisable(GL_BLEND);
	
	glColor4ub (0x00, 0xFF, 0x00, 0xFF);
	glLineWidth((GLfloat)(3));
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glRectf (minDataX, minDataY,
		 maxDataX, maxDataY);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth((GLfloat)(1));

	TSMTRACE(( MAP_MASK|1, funcName, "tileset %g,%g -> %g,%g",
		   minDataX, minDataY, maxDataX, maxDataY ));

      }

      /* The rest of these are rendered in the LVCS, so tfm. */

      glTranslatef (minDataX, minDataY, 0);

      /* Overlay the current flight path on the map */

      if ( showPath )
	ScannedMapDrawPath(scannedMap);

      /* Draw frustums of all twoD and ThreeD views */

      if ( showFrustum ) {
	static void DrawFrustum (tvBase *core, void *closure);
	tvForallWindows (DrawFrustum, map->geo);
      }

    }; glPopMatrix ();
  }; glmReleaseWindow (funcName, scannedMap->glmw);

  TSMTRACE ((MAP_MASK|1, funcName, "Done."));
}

static void
DrawFrustum (tvBase *threeD, void *closure)
{
  TsGeoPyramid *geo = closure;
  int constrain = tvGetBoolean (threeD, TV_TWOD_CONSTRAIN);
  if ( constrain )
    ScannedMapDrawFrustum2d( threeD, geo );
  else
    ScannedMapDrawFrustum3d( threeD, geo );
}

/*
 * Synopsis:
 *   void ScannedMapDrawFrustum3d( ThreeDWidget *threeDWidget, TsGeoPyramid * )
 *
 * Description:
 *   Draws a triangle on the map to indiciate the position and direction
 *   of the camera in the specified ThreeDWidget, transformed into the
 *   local coordinate system (LVCS) of the specified GeoPyramid.
 *
 *   This function displays the projection of a 3D view frustum.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 20 July 1999
 *
 */

static void
ScannedMapDrawFrustum3d( tvBase *threeD, TsGeoPyramid *geo )
{
  /*static const char funcName[] = "ScannedMapDrawFrustum3d";*/
  tvViewpoint view;
  float       fovOffset;
  float       v[2], lvcs[3], yaw;
  int         i;

  tvBaseGetView( threeD, &view );

  TsGeoGCCtoLVCS( geo, view.eye[0], view.eye[1], view.eye[2], lvcs );

  fovOffset = 0.5 * view.fov * (M_PI / 180.0);
  yaw = view.yaw + ( M_PI / 2.0 );

  /* DRAW THE FOV */

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_BLEND );
  glLineWidth( (GLfloat)(3) );

  /* draw the triangle twice: once filled transparently, once to outline */

  for ( i = 0; i < 2; i++ ) {
    GLenum prim_type;

    if ( i == 0 ) {
      glColor4ub( 0x00, 0x00, 0xFF, 0x20 );
      prim_type = GL_TRIANGLES;
    } else {
      glColor4ub( 0x00, 0xFF, 0xFF, 0xFF );
      prim_type = GL_LINE_LOOP;
    }

    glBegin( prim_type ); {
      v[0] = lvcs[0];
      v[1] = lvcs[1];
      glVertex2fv(v);
      
      v[0] = lvcs[0] + 25000.0 * cos( yaw + fovOffset );
      v[1] = lvcs[1] + 25000.0 * sin( yaw + fovOffset );
      glVertex2fv(v);
      
      v[0] = lvcs[0] + 25000.0 * cos( yaw - fovOffset );
      v[1] = lvcs[1] + 25000.0 * sin( yaw - fovOffset );
      glVertex2fv(v);
    }; glEnd();

  }

  glDisable( GL_BLEND );
  glLineWidth( (GLfloat)(1) );
}


/*
 * Synopsis:
 *   void ScannedMapDrawFrustum2d( tvBase *threeD, TsGeoPyramid * )
 *
 * Description:
 *   Draws a triangle on the map to indiciate the position and direction
 *   of the camera in the specified ThreeDWidget, transformed into the
 *   local coordinate system (LVCS) of the specified GeoPyramid.
 *
 *   This function displays the projection of a 2D view frustum.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 20 July 1999
 *
 */

static void
ScannedMapDrawFrustum2d( tvBase *threeD, TsGeoPyramid *geo )
{
  /*static const char funcName[] = "ScannedMapDrawFrustum2d";*/
  tvViewpoint view;
  float       lvcs[3], size;
  int         i;

  tvBaseGetView( threeD, &view );

  TsGeoGCCtoLVCS( geo, view.eye[0], view.eye[1], view.eye[2], lvcs );

  /* calculate the length (/2) of the overlay square to draw */
  /* N.B. this assumes terrain at sea-level!!                */

  size = view.altitude * tan( ( view.fov * (M_PI / 180.0) ) / 2.0 );

  /* translate to the center of the screen on the map */

  glTranslatef( lvcs[0], lvcs[1], 0.0 );

  /* Draw the FOV */

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable( GL_BLEND );
  glLineWidth( (GLfloat)(3) );

  /* draw the rect twice: once filled transparently, once to outline */

  for ( i = 0; i < 2; i++ ) {

    if ( i == 0 ) {
      glColor4ub( 0x00, 0x00, 0xFF, 0x20 );
    } else {
      glColor4ub( 0x00, 0xFF, 0xFF, 0xFF );
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }

    glRectf( -size * view.aspect, -size, size * view.aspect, size );

  }

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDisable( GL_BLEND );
  glLineWidth( (GLfloat)(1) );
}

/*
 * Synopsis: void ScannedMapDrawPath()
 *
 * Description: Draws a line showing the currently loaded flight path
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 10 December 1998
 *
 */

static void
ScannedMapDrawPath(ScannedMapWidget * scannedMap)
{
  /*static char const funcName[] = "ScannedMapDrawPath";*/
  static void DrawPath (tvBase *, void *);

  glColor4ub (0xFF, 0x33, 0x00, 0xFF);
  glLineWidth ((GLfloat)(3));

  tvForallWindows (DrawPath, scannedMap->map.geo);

  glLineWidth ((GLfloat)(1));
}

static void
DrawPath (tvBase *threeD, void *closure)
{
  PathDraw2D(tvGetPath (threeD), closure);
}

/*
 * Synopsis:
 *   char *ScannedMapGeoSet( ScannedMap *map )
 *
 * Description:
 *   Takes a ScannedMap structure and works out the geoset name that
 *   the map is associated with. This done by looking up the list
 *   of currently loaded geos.
 *  
 *   You should not free the pointer that is returned by this function.
 *   It is a copy of a pointer from the loaded database structure.
 *
 * Returns: a geoset name, or NULL if failed
 *
 * Author: Martin Reddy
 * 
 * Date: 20 July 1999
 *
 */

static char *
ScannedMapGeoSet( ScannedMap *map )
{
  TsmDatabase *database = GetDBase();
  int         j;
  list_iter_t g;

  if ( map == NULL || map->map_name == NULL )
    return NULL;

  /* go through all of the loaded geosets */

  for ( g = list_first( geoList ); ! list_done( g ); g = list_next( g ) ) {

    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( g );

    /* find the TsmGeoSet entry for this geo */

    TsmGeoSet *geoset = tsmGetGeoSet( database, TsGeoName( geo ) );

    /* does this geoset had the specified map name associate with it? */

    for ( j = 0; j < geoset->numMaps; j++ ) {
      if ( strcmp( geoset->map_name[j], map->map_name ) == 0 )
	return geoset->name;
    }

  }

  return NULL;
}

/*
 * Synopsis:
 *   TsGeoPyramid const *ScannedMapGeo( char const *geo_name )
 *
 * Description:
 *   This function will take a geo set name and will go through the
 *   list of all currently loaded geosets looking for an entry where
 *   the names match. If a match is found, the TsGeoPyramid pointer is 
 *   returned.
 *
 *   N.B. Although this is currently called ScannedMapGeo(), it is
 *   a general routine for finding a TsGeoPyramid given a geoset name.
 *
 * Returns: a TsGeoPyramid structure, or NULL if failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 20 July 1999
 *
 */

static TsGeoPyramid const *
ScannedMapGeo( char const *geo_name )
{
  list_iter_t g;

  /* go through all of the currently loaded geosets */

  for ( g = list_first( geoList ); ! list_done( g ); g = list_next( g ) ) {

    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( g );

    /* if the geoset names match, then return the TsGeoPyramid pointer */

    if ( strcmp( TsGeoName( geo ), geo_name ) == 0 )
      return geo;

  }  

  return NULL;
}

/*
 * Synopsis:
 *   ScannedMapBtn1Up()
 *
 * Description:
 *   When the user presses down button 1 of the mouse on the map,
 *   we teleport to that location on the map 
 *
 * Returns: an XtActionProc
 *
 * Author: Stephen Lau
 * 
 * Date: 1994
 *
 */

static XtActionProc
ScannedMapBtn1Up( Widget w, XEvent *event, GLbyte **params,
		  Cardinal *num_params )
{
  static char  *funcName = "ScannedMapBtn1Up";
  float        mouseX;
  float        mouseY;
  Dimension    xSize, ySize;
  XButtonEvent *button = (XButtonEvent *) event;
  ScannedMap   *map = &scannedMap->map;
  float        minDataX, minDataY, lvcs[3];
  double       gcc[3];
  list_iter_t  i;
  tvViewpoint  view;
  TsBBox const *bbox;

  XWindowAttributes   attr;

  if ( ScannedMapIsManaged( scannedMap ) == FALSE )
    return NULL;

  TSMTRACE ((MAP_MASK|1, funcName, "Entering"));

  /* Get the current tile set info */
  bbox = TsGeoBBoxLVCS (map->geo);
  minDataX   = bbox->min[0];
  minDataY   = bbox->min[1];

  /* Get the size of the window */

  XGetWindowAttributes( XtDisplay(w), XtWindow(w), &attr );
  xSize = attr.width;
  ySize = attr.height;

  /* The X11 Y-coord is top->bottom based, hence the reversal. */

  mouseX = ( map->minCoords[0] + ( (button->x / (float) xSize)
	     * (map->maxCoords[0] - map->minCoords[0])) - minDataX );
  mouseY = ( map->maxCoords[1] - ((button->y / (float) ySize)
             * (map->maxCoords[1] - map->minCoords[1])) - minDataY );

  TSMTRACE ((MAP_MASK|1, funcName, "Button %d,%d => %f,%f",
	     button->x, button->y, mouseX, mouseY));

  /* Update the viewpoint of all ThreeDWidgets... */
  /* eventually we may want to specify which one. */

  if (widgetList) {
    for ( i = list_first(widgetList); ! list_done(i); i = list_next(i) ) {
      ThreeDWidget *threeDWidget = (ThreeDWidget*) list_value(i);
      if ( ! ThreeDWidgetIsManaged( threeDWidget ) ) continue;

      /* find the current elevation of this widget's viewpoint */

      tvBaseGetView( threeDWidget->threeD, &view );

      TsGeoGCCtoLVCS( map->geo, view.eye[0], view.eye[1], view.eye[2], lvcs );

      /* now convert this (u,v,elev) coordinate into GCC */

      TsGeoLVCStoGCC( map->geo, mouseX, mouseY, lvcs[2], gcc );

      TSMTRACE(( MAP_MASK|1, funcName, "New GCC coordinate = %lg %lg %lg",
		 gcc[0], gcc[1], gcc[2] ));

      /* now we have our new viewpoint position in world coordinates */

      view.eye[0] = gcc[0];
      view.eye[1] = gcc[1];
      view.eye[2] = gcc[2];

      tvSetView( threeDWidget->threeD, &view, TRUE );
    }
  }

  /* refresh the Map viewer for things like the view frustum display */

  ScannedMapPostRedraw( scannedMap );

  return NULL;
}

/*
 * Synopsis: void DestroyScannedMap()
 *
 * Description: Destroy the scanned map
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: April 1995
 *
 */

void
DestroyScannedMap(ScannedMapWidget **scannedMap)
{
  /*static char const funcName[] = "DestroyScannedMap";*/
  ScannedMapWidget *mapwidget;

  if ( scannedMap == NULL ) return;
  if ( ( mapwidget = *scannedMap ) == NULL ) return;

  if ( mapwidget->map.mapdata != NULL )
    tsmMemFree( mapwidget->map.mapdata );

  /* ScannedMapDismissCB( NULL, scannedMap, NULL );*/

  /* For some reason, destroying the glmw and mainForm structures */
  /* Really screws things up when we open a subsequent dataset in */
  /* the same TerraVision session. */

  /*
  if ( mapwidget->glmw )
    glmDestroyWindow( mapwidget->glmw );

  if ( mapwidget->mainForm )
    XtDestroyWidget( mapwidget->mainForm );
  */

  free( mapwidget );
  maxWidth = maxHeight = 0;
  mapwidget = NULL;
}

#ifdef CAVE
/*
 * Synopsis: ScannedMapCAVERedraw()
 *
 *
 * Description: Draw the map on the screen for the CAVE
 *    it shuld appear in the lower left corned of the screen
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *              Stephen Lau
 *
 * Date: April 1996
 *
 */
void
ScannedMapCAVERedraw(void)
{
 static char * funcName = "ScannedMapCAVERedraw";
 extern ScannedMapWidget * scannedMap;
 TsBBox const *bbox;
 float minDataX, minDataY, maxDataX, maxDataY;
 extern int *scannedMapFlag;
 float height, width;
 ScannedMap const *map;
 list_iter_t i;

    if(!*scannedMapFlag)
    {
 	return;
    }

    /* find the maximum ranges of actual data in set @@ JUNK@@ */
    /*tspec = TsGeoTspec (geo); BROKEN with multi-datasets, must be fixed */
    bbox = TsGeoBBoxLVCS (map->geo);
    minDataX = bbox->min[0];
    maxDataX = bbox->max[0];
    minDataY = bbox->min[1];
    maxDataY = bbox->max[1];

    map = &scannedMap->map;

    width = (map->maxCoords[0] - 
		map->minCoords[0]) / 
	       map->sizeX * 1280;

    height = (map->maxCoords[1] - 
	       map->minCoords[1]) /
	       map->sizeY * 1024;

    /* PUT THE IMAGE ON THE SCREEN */
    glRasterPos2i(0, 0);
    glDrawPixels(map->sizeX,
		 map->sizeY,
		 GL_ABGR_EXT, GL_BYTE,
		 map->mapdata);

    /* LOAD THE NEW COORDINATES */
    glPushMatrix (); {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(map->minCoords[0],
	      width+map->minCoords[0],
	      map->minCoords[1],
	      height+map->minCoords[1],
	      -1, 1);
    }; glPopMatrix ();

    glClearDepth(1.); glClear(GL_DEPTH_BUFFER_BIT);

    /* DRAW THE VEHICLE MODELS */

    glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA); 
    glEnable (GL_BLEND);
    glColor4ub (0x00, 0xFF, 0x00, 0x20);
    glRectf(minDataX, minDataY, maxDataX, maxDataY);
    glDisable(GL_BLEND);
    glColor4ub (0x00, 0xFF, 0x00, 0xFF);
    glLineWidth((GLfloat)(3));
	/* OGLXXX rect: remove extra PolygonMode changes */
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glRectf(minDataX, minDataY, maxDataX, maxDataY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth((GLfloat)(1));

    /* DRAW THE THREED VIEWING FRUSTRUM */
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
      GenericWidget *wgt = (GenericWidget *)list_value (i);
      wgt->shared.drawFrustum(wgt, FALSE);
    }
}
#endif /* CAVE */

/*
 * Synopsis:
 *   static int NumAvailMaps( void )
 *
 * Description:
 *   Counts the number of maps that we have available from all of
 *   the currently loaded datasets. This does not count all of the
 *   different instances of a single map - if the same map is on
 *   various local or http servers then this will still be counted
 *   as one map.
 *
 * Returns: the number of maps available - 0 = no maps available
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static int
NumAvailMaps( void )
{
  list_iter_t i;
  TsmDatabase *database = GetDBase();
  int         count = 0;

  /* check we have a database and a list of loaded datasets */

  if ( geoList == NULL || database == NULL )
    return 0;

  /* go through the list of all loaded datasets */

  for ( i = list_first( geoList); ! list_done( i ); i = list_next( i ) ) {

    /* get the TsmGeoSet database entry for this dataset */

    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
    TsmGeoSet *geoset = tsmGetGeoSet( database, TsGeoName( geo ) );

    /* increment our count with the number of maps for thie geoset */
    
    if ( geoset != NULL )
      count += geoset->numMaps;

  }
  
  return count;
}

/*
 * Synopsis:
 *   void ScannedMapRefreshLists( void )
 *
 * Description:
 *   This is the call that should be used in order to update the
 *   dataset lists for any map viewers by functions outside of this
 *   file. This will
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

void
ScannedMapRefreshLists( void )
{
  if ( scannedMap == NULL ) return;

  /* if current map image no longer associated with any currently */
  /* loaded GeoSet, then will should unload the current map.      */

  if ( ScannedMapGeoSet( &scannedMap->map ) == NULL ) {
    ScannedMapUnloadCB( NULL, scannedMap, NULL );
    return;
  }

  /* if we no longer have any maps available then unload the current map */

  if ( NumAvailMaps() == 0 ) {
    ScannedMapUnloadCB( NULL, scannedMap, NULL );
    return;
  }

  /* otherwise refresh the list of datasets for this map viewer */

  MapSelectRefresh( scannedMap );
}

/*
 * Synopsis:
 *   void MapSelectRefresh( ScannedMapWidget *map )
 *
 * Description:
 *   Takes a map viewer widget and refreshes the list of datasets
 *   that are contained in the Select Map dialog.
 *
 * Returns: 
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
MapSelectRefresh( ScannedMapWidget *map )
{
  list_iter_t i;
  TsmDatabase *database = GetDBase();
  int         count = 0, m, j;
  int const   max_maps = 100;
  XmString    *str;
  char        title[512];

  /* check we have a database and a list of loaded datasets */

  if ( map == NULL || geoList == NULL || database == NULL )
    return;

  /* create an array of XmStrings to put the map names into */

  if ( ( str = (XmString *) XtMalloc( max_maps * sizeof(XmString) ) ) == NULL )
    return;

  /* go through the list of all loaded datasets */

  for ( i = list_first( geoList); ! list_done( i ); i = list_next( i ) ) {

    /* get the TsmGeoSet database entry for this dataset */

    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
    TsmGeoSet *geoset = tsmGetGeoSet( database, TsGeoName( geo ) );

    /* do we have any maps for this geoset? */
    
    if ( geoset == NULL || geoset->numMaps == 0 ) 
      continue;

    /* if so, then let's add them to the list widget */

    sprintf( title, "%s:", geoset->name );
    str[count++] = tvXmString( title );

    for ( m = 0; m < geoset->numMaps; m++ ) {
      TsmMap *tsmap = tsmGetMap( database, geoset->map_name[m] );

      for ( j = 0; j < tsmap->numLocations; j++ ) {
	sprintf( title, "   %s: %s", tsmap->name, tsmap->location[j]->url );
	str[count++] = tvXmString( title );
      }

    }

  }

  /* set the list widget's contents with the new list of maps */
  
  XtVaSetValues( map->mapListWidget, XmNitemCount, count,
		 XmNitems, str, XmNselectedItemCount, 0, NULL );

  /* Clean up behind us */

  for ( j = 0; j < count; j++ )
    XmStringFree( str[j] );
  XtFree( (char *) str );

}

/*
 * Synopsis:
 *   void MapSelectInit( ScannedMapWidget * )
 *
 * Description:
 *   Creates a dialog for a specific Map Viewer that lets the user
 *   select between all of the available scanned maps for display
 *   in the viewer.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
MapSelectInit( ScannedMapWidget *map )
{
  XmString label, title;
  Widget   mainform, topform, dismissform, separator, widget;
  Arg      args[10];
  int      n;

  if ( map == NULL || map->mainForm == NULL )
    return;

  /* Create the main map selector widget instance */

  title = tvXmString( "Select Map to Display" );

  n = 0;
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  map->mapSelectWidget = mainform =
    XmCreateFormDialog( applicationShell, "MapSelector", args, n );

  XmStringFree( title );

  /* Add a separator and a Dismiss widget to the bottom of the window */

  dismissform =
    XtVaCreateManagedWidget( "dismissform",
			     xmRowColumnWidgetClass, mainform,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNentryAlignment, XmALIGNMENT_CENTER,
			     XmNorientation, XmHORIZONTAL,
			     XmNpacking, XmPACK_COLUMN,
			     XmNspacing, 200,
			     XmNmarginWidth, 5,
			     XmNmarginHeight, 5,
			     XmNnoResize, TRUE,
			     NULL );

  separator =
    XtVaCreateManagedWidget( "dismisssep",
			     xmSeparatorGadgetClass, mainform,
			     XmNshadowThickness, 4,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_WIDGET,
			     XmNbottomWidget, dismissform,
			     XmNorientation, XmHORIZONTAL,
			     NULL );

  label = tvXmString( "Dismiss" );
  widget =  XtVaCreateManagedWidget( "dismiss",
				     xmPushButtonGadgetClass, dismissform,
				     XmNleftOffset, 30,
				     XmNrightOffset, 20,
				     XmNshowAsDefault, 1,
				     XmNlabelString, label,
				     NULL );
  XmStringFree( label );

  XtAddCallback( widget, XmNactivateCallback, MapSelectHideCB, map );

  /* add the top form with the selection list and widgets */

  topform =
    XtVaCreateManagedWidget( "mainForm",
			     xmFormWidgetClass, mainform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_WIDGET,
			     XmNbottomWidget, separator,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );

  /* Create the scrolled text widget where all the maps are listed */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 11 ); n++;
  XtSetArg( args[n], XmNwidth, 400 ); n++;

  map->mapListWidget = XmCreateScrolledList( topform, "maplist", args, n );
  XtManageChild( map->mapListWidget );

  /* set up callbacks for double-clicking bookmark entries */

  XtAddCallback( map->mapListWidget, XmNdefaultActionCallback,
		 MapSelectCB, map );
  
  /* Finally, let's fill in the list widget */
  
  MapSelectRefresh( map );
}

/*
 * Synopsis:
 *   void MapSelectShowCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   Callback which is used to pop up the Map Selector dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
MapSelectShowCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ScannedMapWidget *map = (ScannedMapWidget *) client_data;

  if ( map == NULL ) return;

  /* create the Map Select dialog if this is the first time round */

  if ( map->mapSelectWidget == NULL ) 
    MapSelectInit( map );

  /* pop the Map Select dialog up */

  if ( map->mapSelectWidget && ! XtIsManaged( map->mapSelectWidget ) )
    XtManageChild( map->mapSelectWidget );

}

/*
 * Synopsis:
 *   void MapSelectHideCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   Callback which is used to dismiss the Map Selection dialog.
 *   If the Map Select dialog is dismissed and we did not initiate
 *   a load, and there is no current map loaded, then we also close
 *   the Map Viewer.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
MapSelectHideCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ScannedMapWidget *map = (ScannedMapWidget *) client_data;

  if ( map->mapSelectWidget && XtIsManaged( map->mapSelectWidget ) )
    XtUnmanageChild( map->mapSelectWidget );

  if ( map->map.mapdata == NULL && map->map.loading == FALSE )
    ScannedMapDismissCB( NULL, map, NULL );
}

/*
 * Synopsis:
 *   int GetMapSelection( ScannedMapWidget *map, char **url )
 *
 * Description:
 *   This function will find out the name of the currently selected
 *   map in the Map Select dialog. It will return the url for that
 *   map in the url parameter for this function. We know the url
 *   because it is encoded at the end of the entry in the list widget.
 *
 * Returns: TRUE if a map url is selected and has been returned via 
 *   the url parameter. FALSE indicates no valid selection.
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static int
GetMapSelection( ScannedMapWidget *map, char **url )
{
  XmStringTable xm_selected, xm_items;
  char          *string = NULL;
  int           nSelected, nItems;
 
  /* set the output string to NULL for default error result */

  *url = NULL;
  
  /* get the list of all items, and the selected item from the list widget */

  XtVaGetValues( map->mapListWidget, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, XmNitems, &xm_items,
		 XmNitemCount, &nItems, NULL );

  /* return NULL/FALSE if there is nothing selected right now */

  if ( nSelected == 0 )
    return FALSE;

  /* get the currently selected viewpoint name */

  XmStringGetLtoR( xm_selected[0], XmFONTLIST_DEFAULT_TAG, &string );

  if ( string == NULL || strlen(string) == 0 )
    return FALSE;

  /* is the current selection actually a geoset name? */

  if ( string[strlen(string)-1] == ':' )
    return FALSE;

  /* otherwise, we have a valid map entry - return the URL part */
  /* (we encode the URL after a ": " character sequence.)       */

  *url = strstr( string, ": " ) + 2;

  return TRUE;
}

/*
 * Synopsis:
 *   void MapSelectCB( Widget w, XtPointer client_data, XtPointer call_data )
 *
 * Description:
 *   Callback that is invoked whenever the user double-clicks over a
 *   map name in the Map Select dialog of the Map Viewer.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 July 1999
 *
 */

static void
MapSelectCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ScannedMapWidget *scannedMap = (ScannedMapWidget *) client_data;
  char             *url;
  TsmDatabase      *database = GetDBase();
  int              i, j;

  if ( database == NULL || scannedMap == NULL ) return;

  /* get the URL for the map from the current selection */

  if ( GetMapSelection( scannedMap, &url ) == FALSE )
    return;

  /* signal that we are attempting to load this new image */
  /* (must do this before calling MapSelectHideCB().      */
  
  scannedMap->map.loading = TRUE;

  /* pop down the Map Select dialog */

  MapSelectHideCB( NULL, scannedMap, NULL );

  /* look for the map entry in the database with this url */

  for ( i = 0; i < database->numMaps; i++ ) {
    TsmMap *tsmap = &database->map[i];

    for ( j = 0; j < tsmap->numLocations; j++ ) {

      /* found the TsmMap entry - now load the new image */

      if ( strcmp( tsmap->location[j]->url, url ) == 0 ) {
	ScannedMapLoadImage( scannedMap, tsmap, url );
	return;
      }

    }
  }

}
