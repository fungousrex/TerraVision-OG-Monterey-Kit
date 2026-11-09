/*
 * Name: VIEWPOINT.C
 *
 * Description: Widget and database management for viewpoint bookmarks
 *
 * Function List:
 *   void ViewpointInit( ThreeDWidget * );
 *   int ViewpointAddCurrent( ThreeDWidget *, char const *, char const * );
 *   void ViewpointGetView( ViewpointItem *item )
 *   void ViewpointSetView( ViewpointItem *item )
 *   void ViewpointRefresh( void );
 *   void ViewpointRefreshWidget( ThreeDWidget * )
 *   void ViewpointShow( ThreeDWidget * );
 *   void ViewpointHide( ThreeDWidget * );
 *   void ViewpointAddPrompt( Widget, XtPointer, XtPointer );
 *   void ViewpointAddCallback( Widget, XtPointer, XtPointer );
 *   void ViewpointDelPrompt( Widget, XtPointer, XtPointer );
 *   void ViewpointDelCallback( Widget, XtPointer, XtPointer );
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:51 $
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
#include "Viewpoint.h"
#include "Dialogs.h"
#include "Datasets.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>

#include <tsm/tsm.h>

#include <tvutils/tvutils.h>

/* a macro wrapper to the generic error report dialog in Dialogs.c */

static Widget vp_error = NULL;

#define ViewpointError(x) \
   GenericErrorDialog( threeDWidget->VpWidget, &vp_error, x )

static void ViewpointGetView( ThreeDWidget *, ViewpointItem * );
static void ViewpointSetView( ThreeDWidget *, ViewpointItem * );
static void ViewpointAddPrompt( Widget, XtPointer, XtPointer );
static void ViewpointAddCallback( Widget, XtPointer, XtPointer );
static void ViewpointDelPrompt( Widget, XtPointer, XtPointer );
static void ViewpointDelCallback( Widget, XtPointer, XtPointer );
static void ViewpointHideCB( Widget, XtPointer, XtPointer );
static void ViewpointSetButtons( ThreeDWidget * );
static void ViewpointListCB( Widget widget, XtPointer, XtPointer );
static void ViewpointSetCB( Widget, XtPointer, XtPointer );
static void ViewpointAnimateCB( Widget, XtPointer, XtPointer );

/*
 * Synopsis: 
 *   void ViewpointGetView( ThreeDWidget *, ViewpointItem *item )
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
ViewpointGetView( ThreeDWidget *threeDWidget, ViewpointItem *item )
{
  tvViewpoint view;

  if ( item == NULL ) return;

  tvBaseGetView( threeDWidget->threeD, &view );

  item->x     = view.eye[0];
  item->y     = view.eye[1];
  item->z     = view.eye[2];
  item->yaw   = view.yaw;
  item->pitch = view.pitch;
  item->roll  = view.roll;
}

/*
 * Synopsis:
 *   void ViewpointSetView( ThreeDWidget *, ViewpointItem *item )
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
ViewpointSetView( ThreeDWidget *threeDWidget, ViewpointItem *item )
{
  const char   *funcName = "ViewpointSetView";
  char         *label;
  int          labelLength;
  TsGeoPyramid *geo;

  if ( threeDWidget == NULL || item == NULL )
    return;

  /* get the TsGeoPyramid structure for this bookmark's geoset */

  if ( ( geo = tvGetDataset( threeDWidget->threeD, item->dataset ) ) == NULL )
    return;

  /* convert bookmark to GCC if currently in a local coord system */

  if ( item->coordsys == TVU_LVCS ) {
    double       pos[3];
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

  ThreeDWidgetUpdateStatusBar( threeDWidget, label );

  tsmMemFree( label );
  
  /* are we doing a fly-to animation to the new bookmark? */

  if ( threeDWidget->threeDPanel->animateBookmarks ) {
    tvFlyTo( threeDWidget->threeD, item->x, item->y, item->z, 
             item->yaw, item->pitch, item->roll );
    return;
  }

  /* if not, then just jump straight to the new location */

  tvSetView3d ( threeDWidget->threeD, 
		item->x, item->y, item->z,
		item->yaw, item->pitch, item->roll);
}

/*
 * Synopsis:
 *   int ViewpointAddCurrent( ThreeDWidget *, char const *, char const * )
 *
 * Description:
 *   A wrapper to tvuAddViewpoint. This takes a name for a viewpoint
 *   and attempts to add a new viewpoint to the database using the
 *   current 3D widget's viewpoint in the current dataset.
 *
 * Returns: TRUE if added, FALSE if not added
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
ViewpointAddCurrent( ThreeDWidget *threeDWidget, char const *geoname,
		     char const *new_string )
{
  ViewpointItem  new_viewpoint;
  TsGeoPyramid   *geo;

  if ( new_string == NULL || new_string[0] == '\0' )
    return FALSE;

  /* find the TsGeoPyramid structure for this dataset */

  if ( ( geo = tvGetDataset( threeDWidget->threeD, geoname ) ) == NULL )
    return FALSE;

  /* fill in the fields for this new bookmark entry */

  new_viewpoint.coordsys = TVU_GCC;
  new_viewpoint.name     = tsmMemStrdup( new_string );
  new_viewpoint.dataset  = tsmMemStrdup( TsGeoName( geo ) );
  ViewpointGetView( threeDWidget, &new_viewpoint );
  
  return tvuAddViewpoint( &new_viewpoint );
}

typedef struct {
  XmString * str;
  int entries;
} ViewPointAddGeoClosure;

static void
ViewpointAddGeoCB (TsGeoPyramid *geo, void *closure)
{
  ViewPointAddGeoClosure *cl = closure;
  char const  *dataset = TsGeoName( geo );
  char title[512];
  int i, bookmarks;

  /* add all of the bookmarks for this dataset to the list? */

  for ( i = bookmarks = 0; i < tvuViewpointCount(); i++ ) {
    if ( strcasecmp( tvuViewpoint(i)->dataset, dataset ) == 0 ) {
      char *colon;

      /* output the geo name if this is the first bookmark found */

      if ( bookmarks++ == 0 ) {
	sprintf( title, "%s:", dataset );
	cl->str[cl->entries++] = tvXmString( title );
      }

      /* get the name of this bookmark and indent it from the geo name */

      sprintf( title, "   %s", tvuViewpoint(i)->name );

      /* strip colons from bookmark names - only allow these in geo name */

      while ( ( colon = strchr( title, ':' ) ) != NULL )
	*colon = ' ';

      cl->str[cl->entries++] = tvXmString( title );
    }
  }

}

/*
 * Synopsis:
 *   void ViewpointRefreshWidget( ThreeDWidget * )
 *
 * Description:
 *   Takes the current viewpoint database and updates the contents of
 *   the list view in the GUI window. We only added viewpoints to the
 *   list view for the currently displayed dataset, if any.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
ViewpointRefreshWidget( ThreeDWidget *threeDWidget )
{
  int         maxstrings;
  ViewPointAddGeoClosure cl;
  int i;

  if ( threeDWidget == NULL || threeDWidget->VpWidget == NULL ) return;

  /* Create an array of XmStrings to put the database names into */
  /* Only add entries to list if they match the current dataset name */

  maxstrings = tvuViewpointCount() + tvNumDatasets( threeDWidget->threeD ) + 2;
  cl.str = (XmString *) XtMalloc( maxstrings * sizeof(XmString) );
  if ( cl.str == NULL ) return;
  

  /* go through the list of all GeoSets for this ThreeDWidget  */
  cl.entries = 0;
  tvForallDatasets (threeDWidget->threeD, ViewpointAddGeoCB, &cl);

  /* Set the list widget's contents to this new XmString array */

  XtVaSetValues( threeDWidget->VpList, XmNitemCount, cl.entries,
		 XmNitems, cl.str, XmNselectedItemCount, 0, NULL );

  threeDWidget->currBookmark = 0;

  /* set the sensitivity of the Set/Add/Del buttons appropriately */

  ViewpointSetButtons( threeDWidget );

  /* Clean up behind us */

  for ( i = 0; i < cl.entries; i++ )
    XmStringFree( cl.str[i] );
  XtFree( (char *) cl.str );
}

static void
ViewpointRefreshWidgetCB( ThreeDWidget *threeDWidget, void *ignore )
{
  ViewpointRefreshWidget (threeDWidget);
}

/*
 * Synopsis:
 *   int ViewpointGetSelected( ThreeDWidget *, char **, char ** )
 *
 * Description:
 *   This function will find out the name of the currently selected
 *   viewpoint bookmark in the Viewpoint list, and also the name of 
 *   the GeoSet that this bookmark is associated with.
 *
 *   N.B. if this function returns TRUE, then either or both of
 *   the two strings may be defined. It is up to the calling function
 *   to release the memory for these strings (unless they are NULL).
 *
 * Returns: TRUE if we have at least a geoset defined, FALSE if no
 *   bookmark is selected.
 *
 * Author: Martin Reddy
 *
 * Date: 31 May 1999
 *
 */

int
ViewpointGetSelected( ThreeDWidget *threeDWidget, char **bookmark, char **geo )
{
  XmStringTable xm_selected, xm_items;
  char          *string = NULL;
  int           nSelected, nItems, i;
 
  /* set the output strings to NULL for default error result */

  *bookmark = *geo = NULL;
  
  /* get the list of all items, and the selected item from the list widget */

  XtVaGetValues( threeDWidget->VpList, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, XmNitems, &xm_items,
		 XmNitemCount, &nItems, NULL );

  /* return NULL/FALSE if there is nothing selected right now */

  if ( nSelected == 0 ) return FALSE;

  /* get the currently selected viewpoint name */

  XmStringGetLtoR( xm_selected[0], XmFONTLIST_DEFAULT_TAG, &string );
  if ( string == NULL ) return FALSE;

  /* is the current viewpoint name actually a geoset name? */

  if ( strchr( string, ':' ) != NULL ) {
    int length = strlen( string );
    *geo = tsmMemStrdup( string );
    if ( length > 0 ) (*geo)[length-1] = '\0';  /* strip trailing colon */
    XtFree( string );
    return TRUE;
  }

  /* otherwise, we have a viewpoint name, but what's its geoset? */
  /* to find this out, we just look back through the list until  */
  /* we find an entry with a colon in it (that's a geoset name). */

  *bookmark = tsmMemStrdup( string+3 );   /* remove 3 space indent */
  XtFree( string );

  /* now go back from this point and find the first geoset name entry */
  
  for ( i = threeDWidget->currBookmark - 1; i >= 0; i-- ) {
    XmStringGetLtoR( xm_items[i], XmFONTLIST_DEFAULT_TAG, &string );

    if ( strchr( string, ':' ) != NULL ) {
      int length = strlen( string );

      *geo = tsmMemStrdup( string );
      if ( length > 0 ) (*geo)[length-1] = '\0';  /* strip trailing colon */
      XtFree( string );
      return TRUE;
    }

    XtFree( string );
  }

  /* if we get here, then we couldn't find a geoset name to associate */
  /* with the bookmark. We will deal with this as an error.           */
  
  tsmMemFree( *bookmark );
  *bookmark = NULL;

  return FALSE;
}

/*
 * Synopsis:
 *   void ViewpointSetButtons( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   This function will update the sensitivity of the Set/Add/Del
 *   buttons of the Viewpoint Bookmarks dialog depending upon the
 *   current bookmark selection. Effectively, we ghost out the
 *   Set and Del buttons when there is no current selection, or if
 *   the current selection is a geoset name and not a viewpoint bookmark.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 31 May 1999
 *
 */

static void
ViewpointSetButtons( ThreeDWidget *threeDWidget )
{
  char *bookmark, *geo;

  /* get the current bookmark and deal with case where we have no selection */

  if ( ViewpointGetSelected( threeDWidget, &bookmark, &geo ) == FALSE ) {
    XtSetSensitive( threeDWidget->VpSet, False );
    XtSetSensitive( threeDWidget->VpAdd, True );
    XtSetSensitive( threeDWidget->VpDel, False );
    return;
  }

  if ( geo ) tsmMemFree( geo );

  /* is the current selection actually a geoset name (i.e. not a bookmark) */

  if ( bookmark == NULL ) {
    XtSetSensitive( threeDWidget->VpSet, False );
    XtSetSensitive( threeDWidget->VpAdd, True );
    XtSetSensitive( threeDWidget->VpDel, False );
  } else {

    /* otherwise, we have a viewpoint that we can set and delete */

    XtSetSensitive( threeDWidget->VpSet, True );
    XtSetSensitive( threeDWidget->VpAdd, True );
    XtSetSensitive( threeDWidget->VpDel, True );
    
    tsmMemFree( bookmark );
  }
}

/*
 * Synopsis:
 *   void ViewpointRefresh( void )
 *
 * Description:
 *   Refreshes the viewpoint bookmark widget for all currently
 *   opened ThreeDWidgets.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 22 February 1999
 *
 */

void
ViewpointRefresh( void )
{
  ThreeDWidgetForall (ViewpointRefreshWidgetCB, NULL);
}

/*
 * Synopsis:
 *   void ViewpointInit( ThreeDWidget * )
 *
 * Description:
 *   This is the initialise function that must get called before doing
 *   anything. It sets up all of the persistent widgets and reads in
 *   the user's .TerraVisionViewpoints file (if exists). This function
 *   is called by the 3D widget's init function, CreateThreeDWidget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
ViewpointInit( ThreeDWidget *threeDWidget )
{
  XmString label, title;
  Widget   mainform, topform, form, dismissform, separator, widget;
  Arg      args[10];
  int      n;

  if ( threeDWidget == NULL || threeDWidget->mainForm == NULL )
    return;

  /* Create the main Viewpoint Bookmarks widget instance */

  title = tvXmString( "Viewpoint Bookmarks" );

  n = 0;
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  mainform = XmCreateFormDialog( threeDWidget->mainForm,
				 "Viewpoint Bookmarks", args, n );

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

  XtAddCallback( widget, XmNactivateCallback, ViewpointHideCB, threeDWidget );

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

  /* create the "animate bookmarks" checkbox */

  widget = XtVaCreateManagedWidget( "threeDPanelSmoothWidget",
				    xmToggleButtonGadgetClass, topform,
				    XmNleftAttachment, XmATTACH_FORM,
				    /* XmNrightAttachment, XmATTACH_FORM, */
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNindicatorOn, True,
				    NULL);
  threeDWidget->threeDPanel->animateBookmarks =
    XmToggleButtonGadgetGetState( widget );

  XtAddCallback( widget, XmNvalueChangedCallback, ViewpointAnimateCB,
		 threeDWidget );

  /* Create the Set/Add/Del etc buttons below the scrolled list */

  n = 0;
  XtSetArg( args[n], XmNfractionBase, 3 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNbottomWidget, widget ); n++;
  form = XtCreateWidget( "buttons", xmFormWidgetClass, topform, args, n );
  XtManageChild( form );

  threeDWidget->VpSet
    = XtVaCreateManagedWidget( " Set View ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 0,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 1,
			       NULL );
  XtAddCallback( threeDWidget->VpSet, XmNactivateCallback, ViewpointSetCB,
		 threeDWidget );

  threeDWidget->VpAdd
    = XtVaCreateManagedWidget( " Add View ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNmarginLeft, 0,
			       XmNmarginRight, 0,
			       XmNborderWidth, 0,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 1,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 2,
			       NULL );
  XtAddCallback( threeDWidget->VpAdd, XmNactivateCallback, ViewpointAddPrompt,
		 threeDWidget );

  threeDWidget->VpDel
    = XtVaCreateManagedWidget( " Del View ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 2,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 3,
			       NULL );
  XtAddCallback( threeDWidget->VpDel, XmNactivateCallback, ViewpointDelPrompt,
		 threeDWidget );

  /* Create the scrolled text widget where all the viewpoints are listed */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNbottomWidget, form ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 13 ); n++;

  threeDWidget->VpList = XmCreateScrolledList( topform, "vplist", args, n );
  XtManageChild( threeDWidget->VpList );

  /* set up callbacks for browsing and double-clicking bookmark entries */

  XtAddCallback( threeDWidget->VpList, XmNbrowseSelectionCallback,
		 ViewpointListCB, threeDWidget );
  XtAddCallback( threeDWidget->VpList, XmNdefaultActionCallback,
		 ViewpointSetCB, threeDWidget );
  
  /* setup the ThreeDWidget state to remember this dialog */

  threeDWidget->VpAddWidget = NULL;
  threeDWidget->VpDelWidget = NULL;
  threeDWidget->VpWidget    = mainform;

  /* Finally, let's load the user's viewpoint bookmark database */

  tvuLoadViewpoints();
  ViewpointRefreshWidget( threeDWidget );
}

/*
 * Synopsis:
 *   void ViewpointShow( ThreeDWidget * )
 *
 * Description:
 *   Does an XtManageChild on the main Viewpoint Bookmarks GUI
 *   (checks to see if already managed first though of course)
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
ViewpointShow( ThreeDWidget *threeDWidget )
{
  if ( ! XtIsManaged( threeDWidget->VpWidget ) )
    XtManageChild( threeDWidget->VpWidget );
}

/*
 * Synopsis:
 *   void ViewpointShow( ThreeDWidget * )
 *
 * Description:
 *   Does an XtUnmanageChild on the main Viewpoint Bookmarks GUI
 *   (checks to see if already unmanaged first though of course).
 *   This routine will also unmanage any dialog windows that we
 *   have open, such as the add prompt or the delete confirm.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
ViewpointHide( ThreeDWidget *threeDWidget )
{
  if ( XtIsManaged( threeDWidget->VpWidget ) )
    XtUnmanageChild( threeDWidget->VpWidget );
}

static void
ViewpointHideCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  if ( threeDWidget ) ViewpointHide( threeDWidget );
}

/*
 * Synopsis:
 *   void ViewpointClearText( Widget widget )
 *
 * Description:
 *   This will set the XmNtextString attribute of the specified
 *   widget to be an empty string (""). This is used to clear
 *   the currently selected string from the main GUI after deleting
 *   an entry, and also to clear the string in the Add Viewpoint
 *   window for each invocation.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
ViewpointClearText( Widget widget )
{
  XmString empty;

  if ( widget == NULL ) return;
  empty = tvXmString( "" );
  XtVaSetValues( widget, XmNtextString, empty, NULL );
  XmStringFree( empty );
}

/*
 * Synopsis:
 *   void ViewpointSetCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is used ony for the Set View button. It will
 *   advance the specified widget's viewpoint to the currently
 *   selected bookmark (if any) in the Viewpoint Bookmarks dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 31 May 1998
 *
 */

static void
ViewpointSetCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char         *bookmark, *geo;

  /* do a quick sanity check on the parameters we were given */

  if ( threeDWidget == NULL || threeDWidget->currBookmark == 0 )
    return;

  /* get the current bookmark and jump to */

  if ( ViewpointGetSelected( threeDWidget, &bookmark, &geo ) == FALSE )
    ViewpointError( "No Viewpoint Selected" );
  else
    ViewpointSetView( threeDWidget, tvuFindViewpoint( geo, bookmark ) );

  /* clean up the geoset and bookmark strings that we were given */

  if ( geo ) tsmMemFree( geo );
  if ( bookmark ) tsmMemFree( bookmark );
}

/*
 * Synopsis:
 *   void ViewpointListCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This is a callback function that is invoked whenever the user
 *   selects a new entry in the bookmark list of the Viewpoint Bookmarks
 *   dialog. This just refreshes the display of the dialog appropriately,
 *   e.g. ghosts and un-ghosts buttons depending upon the current selection.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 31 May 1999
 *
 */

static void
ViewpointListCB( Widget widget, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget         *threeDWidget = (ThreeDWidget *) client_data;
  XmListCallbackStruct *info = (XmListCallbackStruct *) call_data;

  /* remember the position of the current selection. We need */
  /* to know this information for ViewpointGetSelected().    */

  threeDWidget->currBookmark = info->item_position - 1;

  /* update the ghosting of the Set/Add/Del buttons for this selection */

  ViewpointSetButtons( threeDWidget );
}

/*
 * Synopsis:
 *   void ViewpointAnimateCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This is a callback function that is invoked whenever the user
 *   toggles the "Animate Bookmarks" checkbox in the Bookmarks dialog. 
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 31 May 1999
 *
 */

static void
ViewpointAnimateCB( Widget w, XtPointer client_data, XtPointer call )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  ThreeDPanel  *threeDPanel  = threeDWidget->threeDPanel;

  threeDPanel->animateBookmarks = XmToggleButtonGadgetGetState( w );
}

/*
 * Synopsis:
 *   void ViewpointAddPrompt( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is used whenever the user clicks over the Add button.
 *   It basically fires up the Add Viewpoint requester, if it is not
 *   already being displayed. We also clear the text string each time.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

static void
ViewpointAddPrompt( Widget widget, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  /* do a quick sanity check on the parameters we were given */

  if ( threeDWidget == NULL ) return;

  /* pop up an input requester to get a name of this new bookmark */

  ViewpointClearText( threeDWidget->VpAddWidget );

  GenericPromptDialog( threeDWidget->VpWidget, &threeDWidget->VpAddWidget,
		       "Enter a name for the new viewpoint:",
		       ViewpointAddCallback, (void *) threeDWidget );
}

/*
 * Synopsis:
 *   void ViewpointAddCB( ThreeDWidget *, TsGeoPyramid *, void *)
 *
 * Description:
 *   This callback is the one that does the adding of a bookmark to a
 *   specified dataset when there is more than one dataset loaded.
 *   This callback is invoked by SelectDataset(), from ViewpointAddCallback.
 *
 *   The closure that we get passed is the TextField widget where the
 *   user entered the name of the new bookmark.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

static void
ViewpointAddCB( ThreeDWidget *threeDWidget, TsGeoPyramid *geo, void *closure )
{
  Widget     list = (Widget) closure;
  XmString   text_string;
  char       *bookmark = NULL;
  char const *geoname = TsGeoName( geo );

  /* get the name of this new bookmark from the TextField widget */

  XtVaGetValues( list, XmNtextString, &text_string, NULL );
  XmStringGetLtoR( text_string, XmFONTLIST_DEFAULT_TAG, &bookmark );
  if ( bookmark == NULL ) return;

  /* try to add the new bookmark to the global list */

  if ( ViewpointAddCurrent( threeDWidget, geoname, bookmark ) == FALSE ) {
    XtFree( bookmark );
    ViewpointError( "Could not add the new viewpoint!\n"
		    "The name you gave is not unique for this dataset" );
    return;
  }

  /* free the X text string and refresh the Bookmarks dialogs */

  XtFree( bookmark );
  ViewpointRefresh();
  tvuSaveViewpoints();
}

/*
 * Synopsis:
 *   void ViewpointAddCallback( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This is the callback that is used whenever the user enters
 *   a string into the Add Viewpoint widget. If no string was entered
 *   then we try to make one up. Then we add this viewpoint name
 *   to the database for the current 3D widget's position & orientation.
 *
 *   If there is only one dataset associated with this ThreeDWidget
 *   then the decision on which dataset to add the bookmark to is simple.
 *   If the widget has more than one dataset associated with it, then
 *   we use the SelectDataset function in order to ask the user to 
 *   select a dataset to associate the new bookmark with.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

static void
ViewpointAddCallback(Widget widget, XtPointer client_data, XtPointer call_data)
{
  static int ViewpointFirstGeo (TsGeoPyramid *geo, void *cl);

  char         *bookmark = NULL;
  XmString     text_string;
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  /* pop down the input dialog and check that we have a valid name */

  XtUnmanageChild( widget );

  XtVaGetValues( widget, XmNtextString, &text_string, NULL );
  XmStringGetLtoR( text_string, XmFONTLIST_DEFAULT_TAG, &bookmark );
  
  if ( bookmark == NULL || bookmark[0] == '\0' ) {
    ViewpointError( "No name was specified for the bookmark!" );
    return;
  }
  XtFree( bookmark );

  /* if there is only dataset loaded for this widget, then it's real */
  /* easy to work out which dataset to associate the bookmark with!  */
  if ( tvNumDatasets( threeDWidget->threeD ) == 1 ) {
    TsGeoPyramid *geo =
      tvFindDataset (threeDWidget->threeD, ViewpointFirstGeo, NULL);
    if ( geo != NULL ) {
      ViewpointAddCB( threeDWidget, geo, widget );
      return;
    }
  }

  /* if we have more than one geoset loaded, then we need to resolve */
  /* which geoset to associate this bookmark with. So we pop up a    */
  /* list of available geosets and get the user to select one.       */

  SelectDataset( threeDWidget, &threeDWidget->VpAddSelector,
		 "Choose Dataset to Add Bookmark To", "Add Bookmark",
		 ViewpointAddCB, widget );

}

/* Helper function for finding first non-NULL geopyramid */
static int
ViewpointFirstGeo (TsGeoPyramid *geo, void *cl)
{
  return geo != NULL;
}

/*
 * Synopsis:
 *   void ViewpointDelPrompt( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This is the callback this is used whenever the user clicks on
 *   the Delete button on the main GUI. It basically pops up the
 *   Delete confirmation widget. We check that the user has actually
 *   selected something first though.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

static void
ViewpointDelPrompt( Widget widget, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char         *bookmark, *geo, message[512];

  /* do a quick sanity check on the parameters we were given */

  if ( threeDWidget == NULL || threeDWidget->currBookmark == 0 )
    return;

  /* get the current bookmark so that we can tell the user its name */

  if ( ViewpointGetSelected( threeDWidget, &bookmark, &geo ) == FALSE ) {
    ViewpointError( "No Viewpoint Selected" );
    return;
  }

  sprintf( message, "Do you really want to delete the viewpoint\n"
	   "\"%s\" from the dataset %s?", bookmark, geo );

  if ( geo ) tsmMemFree( geo );
  if ( bookmark ) tsmMemFree( bookmark );

  /* Okay, we're go for launch so far. Let's just confirm first though */

  GenericConfirmDialog( threeDWidget->VpWidget, &threeDWidget->VpDelWidget, 
			message, ViewpointDelCallback, threeDWidget );
}

/*
 * Synopsis:
 *   void ViewpointDelCallback(Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This is the callback that is used whenever the user confirms that
 *   they wish to delete the currently selected viewpoint from the
 *   database.
 *
 * Returns: void
 * 
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

static void
ViewpointDelCallback(Widget widget, XtPointer client_data, XtPointer call_data)
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char         *bookmark, *geo;

  /* get the bookmark and its geoset that we want to delete */

  if ( ViewpointGetSelected( threeDWidget, &bookmark, &geo ) == FALSE )
    return;

  /* pop down the delete confirmation dialog */

  XtUnmanageChild( widget );

  /* try to delete the viewpoint */
  
  if ( tvuDeleteViewpoint( geo, bookmark ) == FALSE ) {
    if ( geo ) tsmMemFree( geo );
    if ( bookmark ) tsmMemFree( bookmark );
    ViewpointError( "Could Not Remove Viewpoint!" );
    return;
  }

  /* if successful, then refresh the screen and the bookmarks file */
  
  if ( geo ) tsmMemFree( geo );
  if ( bookmark ) tsmMemFree( bookmark );

  ViewpointRefresh();
  tvuSaveViewpoints();
}

/*
 * Synopsis:
 *   void int GetViewpointOrigin( ThreeDWidget *threeDWidget, double *x,
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
 *   datasets for the particular ThreeDWidget.
 *
 * Returns: TRUE if details have been returned for "Origin" bookmark.
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
GetViewpointOrigin( ThreeDWidget *threeDWidget, double xyz[3],
		    float *yaw, float *pitch, float *roll )
{
  static const char funcName[] = "GetViewpointOrigin";
  ViewpointItem     *origin;
  char const        *geoname;
  list_iter_t       i;
  TsGeoPyramid      *geo;

  if ( threeDWidget == NULL ) return FALSE;

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
 *   in the specified threeDWidget. 
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
 *   void ViewpointCycleUp( ThreeDWidget *threeDWidget )
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
ViewpointCycleUp( ThreeDWidget *threeDWidget )
{
  XmStringTable xm_items;
  XmString      selected[1];
  int           nItems, i;
  char          *geoname = NULL, *bookmark = NULL;

  if ( threeDWidget->VpList == NULL ) return;

  /* get the list of all bookmarks from the scrolled list widget */

  XtVaGetValues( threeDWidget->VpList, XmNitems, &xm_items,
		 XmNitemCount, &nItems, NULL );

  if ( nItems == 0 ) return;

  /* cycle up to the previous (non geoset name) bookmark */
  
  do {

    threeDWidget->currBookmark--;

    if ( threeDWidget->currBookmark >= nItems ||
	 threeDWidget->currBookmark < 0 )
      threeDWidget->currBookmark = nItems - 1;

    if ( bookmark != NULL ) tsmMemFree( bookmark );
    XmStringGetLtoR( xm_items[threeDWidget->currBookmark],
		     XmFONTLIST_DEFAULT_TAG, &bookmark );

  } while ( strchr( bookmark, ':' ) != NULL );

  /* find the geoset name for this bookmark */

  for ( i = threeDWidget->currBookmark - 1; i >= 0; i-- ) {
    XmStringGetLtoR( xm_items[i], XmFONTLIST_DEFAULT_TAG, &geoname );

    if ( strchr( geoname, ':' ) != NULL ) {
      int length = strlen( geoname );
      if ( length > 0 ) geoname[length-1] = '\0';  /* strip trailing colon */
      break;
    }

    XtFree( geoname );
  }

  if ( i < 0 ) return;

  /* now jump to the new bookmark and select it in the viewpoint dialog */
  /* (the bookmark+3 is done to strip the 3 spaces of indentation).     */

  ViewpointSetView( threeDWidget, tvuFindViewpoint( geoname, bookmark+3 ) );

  selected[0] = xm_items[threeDWidget->currBookmark];
  XtVaSetValues( threeDWidget->VpList, XmNselectedItems, selected,
		 XmNselectedItemCount, 1, NULL );

  XtFree( bookmark );
  XtFree( geoname );
}

/*
 * Synopsis:
 *   void ViewpointCycleDown( ThreeDWidget *threeDWidget )
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
ViewpointCycleDown( ThreeDWidget *threeDWidget )
{
  XmStringTable xm_items;
  XmString      selected[1];
  int           nItems, i;
  char          *geoname = NULL, *bookmark = NULL;

  if ( threeDWidget->VpList == NULL ) return;

  /* get the list of all bookmarks from the scrolled list widget */

  XtVaGetValues( threeDWidget->VpList, XmNitems, &xm_items,
		 XmNitemCount, &nItems, NULL );

  if ( nItems == 0 ) return;

  /* cycle down to the next (non geoset name) bookmark */
  
  do {

    threeDWidget->currBookmark++;

    if ( threeDWidget->currBookmark >= nItems ||
	 threeDWidget->currBookmark < 0 )
      threeDWidget->currBookmark = 0;

    if ( bookmark != NULL ) tsmMemFree( bookmark );
    XmStringGetLtoR( xm_items[threeDWidget->currBookmark],
		     XmFONTLIST_DEFAULT_TAG, &bookmark );

  } while ( strchr( bookmark, ':' ) != NULL );

  /* find the geoset name for this bookmark */

  for ( i = threeDWidget->currBookmark - 1; i >= 0; i-- ) {
    XmStringGetLtoR( xm_items[i], XmFONTLIST_DEFAULT_TAG, &geoname );

    if ( strchr( geoname, ':' ) != NULL ) {
      int length = strlen( geoname );
      if ( length > 0 ) geoname[length-1] = '\0';  /* strip trailing colon */
      break;
    }

    XtFree( geoname );
  }

  if ( i < 0 ) return;

  /* now jump to the new bookmark and select it in the viewpoint dialog */
  /* (the bookmark+3 is done to strip the 3 spaces of indentation).     */

  ViewpointSetView( threeDWidget, tvuFindViewpoint( geoname, bookmark+3 ) );

  selected[0] = xm_items[threeDWidget->currBookmark];
  XtVaSetValues( threeDWidget->VpList, XmNselectedItems, selected,
		 XmNselectedItemCount, 1, NULL );

  XtFree( bookmark );
  XtFree( geoname );
}
