/*
 * Name: Info.c
 *
 * Description: Display information about the various datasets that
 *    TerraVision is displaying, e.g. dataset sizes, resolutions,
 *    locations, etc. 
 *
 * Function List:
 *   void InfoFindDatasets( void )
 *   void InfoInit( Widget parent )
 *   void InfoRefreshDatasetList( void )
 *   void InfoOptionCB( Widget menu_item, XtPointer client_data, XtPointer )
 *   void InfoShow( Widget parent )
 *   void InfoHide( void )
 *   void InfoMakeSize( double bytes, char *buffer )
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:47 $
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
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

#include "TerraVisionInc.h"
#include "Database.h"
#include "ScannedMap.h"
#include "Info.h"
#include "ThreeDCore.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/SeparatoG.h>

static TsmConnection dataset[32];
static int           num_datasets;
static int           curr_dataset = 0;
static Widget        infoWidget = NULL;
static Widget        infoText = NULL;
static Widget        infoChoice = NULL;
static Widget        infoForm = NULL;

/* the following function is private to tsmApi, but we'll snag it here */
void tsmCalcFootprintFromXform( Pyramid *pyramid, TsmDataSet *dataset );
static void InfoAggregateText( void );

/*
 * Synopsis:
 *   void InfoFindDatasets( void )
 *
 * Description:
 *   This function will refresh the Info window's internal list of
 *   datasets. It will check to see all of the datasets that are
 *   currently in use and maintain a list of TsmConnection structures
 *   for each of these. For TerraVision Classic, this will just be
 *   the OI and DEM, but for TerraVision NextGeneration, we could have
 *   nested OI/DEM/VRML combinations (this isn't supported just yet).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoFindDatasets( void )
{
  list_iter_t i;

  num_datasets = 0;
  for (i = list_first (geoList); ! list_done (i); i = list_next (i)) {
    TsGeoPyramid *geo = list_value (i);
    num_datasets += TsGeoConnections (geo, &dataset[num_datasets]);
  }
}

/*
 * Synopsis:
 *   void InfoInit( Widget parent )
 *
 * Description:
 *   Generates the window for the Info facility and sets up all of the
 *   widgets. The window is not displayed until you do an InfoShow.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoInit( Widget parent )
{
  Widget    pane, form, widget, pulldown, sep;
  Arg       args[15];
  int       n = 0;
  XmString  none, label;

  /* Set up a DialogShell as a popup window.  Set the delete window */
  /* protocol response to XmDO_NOTHING to stop the user from being  */
  /* able to kill the window, causing us to loose it forever.       */

  n = 0;
  label = tvXmString("Dataset Info");
  XtSetArg( args[n], XmNdialogTitle, label ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  infoWidget = XmCreateFormDialog( parent, "Dataset Info", args, n );
  XmStringFree( label );

  /* Create a PanedWindow to manage the stuff in this dialog. */
  /* Set the sash size to as small as possible (can't do 0).  */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  pane = XtCreateWidget( "pane", xmFormWidgetClass, infoWidget, args, n );
  XtManageChild( pane );

  /* Create a RowColumn widget to contain the scroll text and */
  /* the dataset selection widget. This is the control area.  */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomOffset, 8 ); n++;
  infoForm = XtCreateWidget( "rowcol", xmFormWidgetClass, pane, args, n );
  XtManageChild( infoForm );

  /* prepare the ScrolledText object to hold all of the info */

  n = 0;
  XtSetArg( args[n], XmNscrollVertical, True ); n++;
  XtSetArg( args[n], XmNscrollHorizontal, True ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNwordWrap, True ); n++;
  XtSetArg( args[n], XmNrows, 15 ); n++;
  XtSetArg( args[n], XmNcolumns, 45 ); n++;
  XtSetArg( args[n], XmNscrolledWindowMarginHeight, 5 ); n++;
  XtSetArg( args[n], XmNscrolledWindowMarginWidth, 5 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;

  infoText = XmCreateScrolledText( infoForm, "info_text", args, n );
  XtManageChild( infoText );

  /* Now prepare the OptionsMenu for the dataset selection */

  label = XmStringCreateLocalized( "Dataset : " );
  none  = XmStringCreateLocalized( "None" );

  pulldown = XmVaCreateSimplePulldownMenu( infoForm, "pulldown", 2, NULL,
       XmVaPUSHBUTTON, none, 0, NULL, NULL, NULL );

  n = 0;
  XtSetArg( args[n], XmNsubMenuId, pulldown ); n++;
  XtSetArg( args[n], XmNlabelString, label ); n++;
  infoChoice = XmCreateOptionMenu( infoForm, "dataset_menu", args, n );
  XtManageChild( infoChoice );

  XmStringFree( label );
  XmStringFree( none );


  /* Set up the attachments so that the OptionsMenu widget is    */
  /* attached to the bottom of the form and the textfield widget */
  /* is attached on top of this - lets you resize the window.    */
			 
  XtVaSetValues( XtParent(infoText),
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNtopAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, infoChoice,
		 NULL );

  XtVaSetValues( infoChoice,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_FORM,
		 NULL );

  /* Create a separator bar */

  sep = XtVaCreateManagedWidget( "formSeparator1",
				 xmSeparatorGadgetClass, pane,
				 XmNshadowThickness, 4,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNorientation, XmHORIZONTAL,
				 XmNbottomOffset, 3,
				 NULL );

  XtVaSetValues( infoForm, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, sep, NULL );

  /* Create another form to act as the action area for the dialog */

  n = 0;
  XtSetArg( args[n], XmNfractionBase, 3 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  form = XtCreateWidget( "form2", xmFormWidgetClass, pane, args, n );
  XtManageChild( form );

  XtVaSetValues( sep, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, form, NULL );

  widget = XtVaCreateManagedWidget( "OK",
				    xmPushButtonGadgetClass, form,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_POSITION,
				    XmNleftPosition, 1,
				    XmNrightAttachment, XmATTACH_POSITION,
				    XmNrightPosition, 2,
				    XmNdefaultButtonShadowThickness, 1,
				    NULL );
  XtAddCallback( widget, XmNactivateCallback, (XtCallbackProc) InfoHide, NULL);
}

/*
 * Synopsis:
 *   void InfoRefreshDatasetList( void )
 *
 * Description:
 *   This function will refresh the Info window with the currently
 *   loaded datasets. If the window is currently open, then it will
 *   be updated to reflect any changes. If you do anything in 
 *   TerraVision that adds/removes datasets, then this is the function
 *   that you should call in order to update the Info window on the fly.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoRefreshDatasetList( void )
{
  Widget   pulldown;
  Arg      args[15];
  int      n = 0, i;
  XmString buttons[30];
  XmButtonType button_type[30];

  /* If we have not opened the window yet, then there is nothing to update */
  
  if ( infoWidget == NULL || infoChoice == NULL ) return;

  /* Get the currently loaded list of datasets */

  InfoFindDatasets();

  /* If we have no datasets, then just make an OptionMenu */
  /* with a single option saying None, with no callback.  */

  if ( num_datasets == 0 ) {
    buttons[0] = XmStringCreateLocalized( "None" );
    XtSetArg( args[n], XmNbuttonCount, 1 ); n++;
    XtSetArg( args[n], XmNbuttons, buttons ); n++;
    XtSetArg( args[n], XmNbuttonSet, 0 ); n++;
    pulldown = XmCreateSimplePulldownMenu( infoForm, "pulldown", args, n );
    XtVaSetValues( infoChoice, XmNsubMenuId, pulldown, NULL );
    XmStringFree( buttons[0] );
    InfoRefreshText();
    return;
  }

  /* Otherwise, we can make a new OptionMenu entry for each */
  /* of the currently loaded datasets, e.g. DEM and OI.     */

  buttons[0] = tvXmString( "Aggregate Report" );
  button_type[0] = XmPUSHBUTTON;

  for ( i = 0; i < num_datasets; ++i ) {
    buttons[i+1] = tvXmString( (char *) tsmGet(dataset[i], TSM_DATASET_NAME) );
    button_type[i+1] = XmPUSHBUTTON;
  }

  if ( curr_dataset > num_datasets ) curr_dataset = 0;

  XtSetArg( args[n], XmNbuttonCount, num_datasets + 1 ); n++;
  XtSetArg( args[n], XmNbuttons, buttons ); n++;
  XtSetArg( args[n], XmNbuttonType, button_type ); n++;
  XtSetArg( args[n], XmNsimpleCallback, InfoOptionCB ); n++;
  pulldown = XmCreateSimplePulldownMenu( infoForm, "pulldown", args, n );

  /* set the new selection of options in our OptionsMenu widget */

  XtVaSetValues( infoChoice, XmNsubMenuId, pulldown, NULL );

  for ( i = 0; i < num_datasets; ++i )
    XmStringFree( buttons[i] );

  /* refresh the information display for the current dataset */

  InfoRefreshText();
}

/*
 * Synopsis:
 *   void InfoOptionCB( Widget menu_item, XtPointer client_data, XtPointer )
 *
 * Description:
 *   An X callback that gets called whenever the user selects a dataset
 *   from the OptionsMenu widget. All we need to do here is display the
 *   info text for the selected dataset.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoOptionCB( Widget menu_item, XtPointer client_data, XtPointer call_data )
{
  curr_dataset = (int) client_data;
  InfoRefreshText();
}

/*
 * Synopsis:
 *   void InfoShow( Widget parent )
 *
 * Description:
 *   This is the function that you should call in order to display
 *   the Info window. It will perform any widget initialisation on 
 *   the first invocation, refreshes the dataset list, and displays
 *   the window on the screen (if it's not already there).
 *
 *   You can make the window go away again by calling InfoHide().
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoShow( Widget parent )
{
  if ( infoWidget == NULL )
    InfoInit( parent );

  InfoRefreshDatasetList();

  if ( ! XtIsManaged( infoWidget ) )
    XtManageChild( infoWidget );
}

/*
 * Synopsis:
 *   void InfoHide( void )
 *
 * Description:
 *   Closes the Info window. N.B. this just does an unmanage, it does
 *   not destroy the widget. This function is called whenever you
 *   click the OK button on the Info window.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoHide( void )
{
  if ( infoWidget == NULL )
    return;

  if ( XtIsManaged( infoWidget ) )
    XtUnmanageChild( infoWidget );

  curr_dataset = 0; /* can't make OptionMenu widget preserve its state! */
}

/*
 * Synopsis:
 *   void InfoMakeSize( double bytes, char *buffer )
 *
 * Description:
 *   A simple routine that will create a string given a number of bytes.
 *   It will try to convert the byte count into suitable units, e.g.
 *   KB, MB, of GB, depending upon its magnitude.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoMakeSize( double bytes, char *buffer )
{
  static const float KB = 1024.0;
  static const float MB = 1024.0 * 1024.0;
  static const float GB = 1024.0 * 1024.0 * 1024.0;

  if ( bytes < KB )
    sprintf( buffer, "%1.0f bytes", bytes );
  else if ( bytes < MB )
    sprintf( buffer, "%1.1f KB", bytes / KB );
  else if ( bytes < GB )
    sprintf( buffer, "%1.1f MB", bytes / MB );
  else
    sprintf( buffer, "%1.1f GB", bytes / GB );
}

/*
 * Synopsis:
 *   void AppendString( char **string, char const *append )
 *
 * Description:
 *   Appends a string onto another string, growing the memory
 *   for the current string appropriately before appending the
 *   new section. If *string is NULL, then a strdup() is performed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 May 1999
 *
 */

void
AppendString( char **string, char const *append )
{
  if ( string == NULL || append == NULL ) return;

  /* If we haven't got a string yet, then just copy the append string */

  if ( *string == NULL ) {
    *string = tsmMemStrdup( append );
    return;
  }

  /* Otherwise, we grow the string and append the new string */

  *string = (char *) tsmMemRealloc( *string, strlen(*string)+strlen(append)+1);
  if ( *string == NULL ) return;
  strcat( *string, append );
}

/*
 * Synopsis:
 *   void InfoRefreshText( void )
 *
 * Description:
 *   This is the bit that displays the actual information in the
 *   scolledtext widget of the Info window. It looks at the 
 *   currently selected dataset and reports a number of interesting
 *   and/or useful quantities such as how large a dataset is,
 *   where it was loaded from, what resolution it is, etc.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 August 1998
 *
 */

void
InfoRefreshText( void )
{
  TsmConnection connect;
  TsmDataSet    dset;
  Pyramid       *tspec;
  TileTspec     *tile;
  char          *new_text=NULL, buffer[1024];
  char          size1[32];
  char          *units;
  int           *pixels;
  float         *ixiytouv, x, y;
  double        imgsize;
  Purl          *purl;

  /* Check for degenerate cases: if there are no datasets then  */
  /* display no text. If the current dataset pointer is invalid */
  /* then there is nothing that we can do, so display nothing.  */

  if ( num_datasets == 0 || curr_dataset > num_datasets ) {
    XtVaSetValues( infoText, XmNvalue, "", NULL );
    return;
  }

  /* the first entry in the list is an Aggregate Report. This */
  /* is a special case and we deal with this separately.      */

  if ( curr_dataset == 0 ) {
    InfoAggregateText();
    return;
  }

  /* First of all take some convenience pointers and do */
  /* some initial pre-computation stuff.                */

  connect = dataset[curr_dataset-1];
  tspec   = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  tile    = &tspec->tileTspec;
  units   = ( tspec->coordTspec.componentUnits == tspDegrees ) ? "deg" : "m";
  pixels  = tspec->tspecLevel.val[tspec->minLevel].numValidPixel;
  ixiytouv= tspec->tspecLevel.val[tspec->minLevel].imageTransformationTspec.ixiytoUV;
  imgsize = pixels[0] * pixels[1] * tile->numComponentsPixel *
            (double) tile->numBytesPerComponent;

  /* Describe information about the dataset, its name, type, size, etc. */

  AppendString( &new_text, "Dataset:\n" );

  sprintf( buffer, "   Name = %s\n",
	   (char *) tsmGet( connect, TSM_DATASET_NAME ) );
  AppendString( &new_text, buffer );

  sprintf( buffer, "   Type = %s\n",
	   (char *) tsmGet( connect, TSM_DATASET_TYPE_STRING ) );
  AppendString( &new_text, buffer );

  InfoMakeSize( imgsize * 4.0 / 3.0, size1 );
  sprintf( buffer, "   Total Size = %s\n", size1 );
  AppendString( &new_text, buffer );

  /* Describe the resolution of the data */

  AppendString( &new_text, "\nResolution:\n" );

  sprintf( buffer, "   Ground Res = %g %s to %g %s\n",
	   tspec->minResolution, units, tspec->maxResolution, units );
  AppendString( &new_text, buffer );

  x = tspec->minResolution * pixels[0];
  y = tspec->minResolution * pixels[1];

  if ( ( x > 1000.0 || y > 1000.0 ) && strcmp( units, "m" ) == 0 ) {
    x /= 1000.0;
    y /= 1000.0;
    units = "km";
  }
  sprintf( buffer, "   Ground Cover = %g x %g %s\n", x, y, units );
  AppendString( &new_text, buffer );

  InfoMakeSize( imgsize, size1 );
  sprintf( buffer, "   Pixel Res = %d x %d (%s)\n",
	   pixels[0], pixels[1], size1 );
  AppendString( &new_text, buffer );

  sprintf( buffer, "   Levels = %d (%d..%d)\n",
	   tspec->maxLevel - tspec->minLevel + 1,
	   tspec->minLevel, tspec->maxLevel );
  AppendString( &new_text, buffer );

  /* Where is the data coming from? */

  AppendString( &new_text, "\nLocation:\n" );
  purl = tsmParseUrl( (char *) tsmGet( connect, TSM_DATASET_URL ) );

  switch ( tsmGet( connect, TSM_CONNECT_PROTOCOL ) ) {
  case TSM_DPSS:
    sprintf( buffer, "   DPSS = %s\n", purl->hostName );
    AppendString( &new_text, buffer );

    sprintf( buffer, "   Servers = %s\n",
	     (char *) tsmGet( connect, TSM_DPSS_SERVERS ) );
    AppendString( &new_text, buffer );

    sprintf( buffer, "   Set ID = %d\n", tsmGet( connect, TSM_DPSS_SETID ) );
    AppendString( &new_text, buffer );

    sprintf( buffer, "   Tspec = %s\n", (char*)tsmGet(connect,TSM_TSPEC_URL) );
    AppendString( &new_text, buffer );
    break;

  case TSM_LOCAL:
    sprintf( buffer, "   Localhost = %s\n", purl->uri );
    AppendString( &new_text, buffer );
    break;

  case TSM_HTTP:
    sprintf( buffer, "   HTTP = %s\n", purl->url );
    AppendString( &new_text, buffer );
    break;

  }

  /* What format is the tiled data in? tiff/raw jpeg/raw */

  AppendString( &new_text, "\nFormat:\n" );

  sprintf( buffer, "   Image Format = %s\n",
	   (char *) tsmGet( connect, TSM_IMAGE_FORMAT_STRING ) );
  AppendString( &new_text, buffer );

  sprintf( buffer, "   Tile Format = %s\n",
	   (char *) tsmGet( connect, TSM_TILE_FORMAT_STRING ) );
  AppendString( &new_text, buffer );

  /* Where on the planet is this dataset? */

  AppendString( &new_text, "\nGeoreference:\n" );
  tsmCalcFootprintFromXform( tspec, &dset );

  switch ( tspec->projectionTspec.projectionType ) {
  case tspUtmProjection:
    sprintf( buffer, "   Coordinate System = UTM\n" );
    AppendString( &new_text, buffer );
    sprintf( buffer, "   Lower-Left = %g easting %g northing %d zone\n", 
	     dset.footprint.utm.c1.e, dset.footprint.utm.c1.n,
	     dset.footprint.utm.c1.zone );
    AppendString( &new_text, buffer );
    sprintf( buffer, "   Upper-Right = %g easting %g northing %d zone\n", 
	     dset.footprint.utm.c3.e, dset.footprint.utm.c3.n,
	     dset.footprint.utm.c3.zone );
    AppendString( &new_text, buffer );
    break;
  case tspAffineProjection:
    sprintf( buffer, "   Coordinate System = LVCS\n" );
    AppendString( &new_text, buffer );
    break;
  case tspLatLongProjection:
    sprintf( buffer, "   Coordinate System = Lat/Long\n" );
    AppendString( &new_text, buffer );
    sprintf( buffer, "   Lower-Left = %g latitude %g longitude\n", 
	     ixiytouv[5], ixiytouv[2] );
    AppendString( &new_text, buffer );
    break;
  default:
    sprintf( buffer, "   Coordinate System = Unknown<%d>\n",
	     tspec->projectionTspec.projectionType );
    AppendString( &new_text, buffer );
    break;
  }

  sprintf( buffer, "   Planet = %s\n", tsmPlanetToString( tspec->planet ) );
  AppendString( &new_text, buffer );

#if 0
  /* What about the map imagery (if available)? */

  mapno = 0;
  while ( ( mapinfo = DBaseGetMap( mapno++ ) ) != NULL ) {
    sprintf( buffer, "\nMap Image:\n" );
    AppendString( &new_text, buffer );
    sprintf( buffer, "   Name = %s\n", mapinfo->name );
    AppendString( &new_text, buffer );
    for ( i = 0; i < mapinfo->numLocations; i++ ) {
      sprintf( buffer, "   URL = %s\n", mapinfo->location[i]->url );
      AppendString( &new_text, buffer );
    }
    sprintf( buffer, "   Lower-left = %1.10g, %1.10g\n",
	     mapinfo->min_u, mapinfo->min_v );
    AppendString( &new_text, buffer );
    sprintf( buffer, "   Upper-right = %1.10g, %1.10g\n",
	     mapinfo->max_u, mapinfo->max_v );
    AppendString( &new_text, buffer );
  }
#endif

  XtVaSetValues( infoText, XmNvalue, new_text, NULL );

  if ( new_text ) tsmMemFree( new_text );

}

/*
 * Synopsis:
 *   void InfoAggregateText( void )
 *
 * Description:
 *   This routine is used to display the text in the Dataset Info
 *   widget for the Aggregate Report. This gives cumulative values
 *   for interesting values, for example, the total dataset size
 *   (summing the size of all datasets).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 30 June 1999
 *
 */

static void
InfoAggregateText( void )
{
  TsmConnection connect;
  Pyramid       *tspec;
  TileTspec     *tile;
  char          *new_text=NULL, buffer[1024];
  char          size1[32];
  int           *pixels, j;
  double        imgsize;
  double        total_size = 0.0, total_pixels = 0.0, total_dem_postings = 0.0;

  /* First of all take some convenience pointers and do */
  /* some initial pre-computation stuff.                */

  for ( j = 0; j < num_datasets; j++ ) {
    connect = dataset[j];
    tspec   = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
    tile    = &tspec->tileTspec;
    pixels  = tspec->tspecLevel.val[tspec->minLevel].numValidPixel;
    imgsize = pixels[0] * pixels[1] * tile->numComponentsPixel *
              (double) tile->numBytesPerComponent;

    /* add to the total aggregate size of all loaded datasets */

    total_size += imgsize * 4.0 / 3.0;

    /* add to the total aggregate number of pixels/DEM postings */

    switch ( (TsmDataSetType) tsmGet( connect, TSM_DATASET_TYPE ) ) {
    case TSM_DEM_DATASET:
      total_pixels += (double) pixels[0] * pixels[1];
      break;
    case TSM_IMAGE_DATASET:
      total_dem_postings += (double) pixels[0] * pixels[1];
      break;
    }
  }

  /* Describe information about the dataset, its name, type, size, etc. */

  AppendString( &new_text, "Aggregate Report:\n" );

  sprintf( buffer, "   Datasets Loaded = %d\n",
	   ( geoList ) ? list_count( geoList ) : 0 );
  AppendString( &new_text, buffer );

  InfoMakeSize( total_size, size1 );
  sprintf( buffer, "   Aggregate Dataset Size = %s\n", size1 );
  AppendString( &new_text, buffer );

  sprintf( buffer, "   Aggregate Pixels = %1.1f million\n",
	   total_pixels / 1e6 );
  AppendString( &new_text, buffer );

  sprintf( buffer, "   Aggregate DEM postings = %1.1f million\n",
	   total_dem_postings / 1e6 );
  AppendString( &new_text, buffer );

  XtVaSetValues( infoText, XmNvalue, new_text, NULL );

  if ( new_text ) tsmMemFree( new_text );

}
