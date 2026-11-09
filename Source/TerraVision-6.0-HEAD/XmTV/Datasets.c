/*
 * Name: DATASETS.C
 *
 * Description:
 *     Various routines and GUIs to support TerraVision's multiple
 *     dataset facilities, e.g. selecting datasets to display, setting
 *     display priorities and options, etc.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     Created by Martin Reddy, 2 June, 1999
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
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "Datasets.h"
#include "Dialogs.h"
#include "Info.h"
#include "ScannedMap.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

Widget remove_widget = NULL;         /* the Close Dataset dialog widget */
list_t *select_widgets = NULL;       /* list of all open Dataset Selectors */

typedef struct {
  Widget            *dialog;       /* pointer to widget structure for dialog */
  Widget            list;          /* the dialog's ScrolledList widget  */
  ThreeDWidget      *threeDWidget; /* the ThreeDWidget, or NULL if none */
  DatasetSelectProc *callback;     /* the func to call when selected */
  void              *closure;      /* the closure to give to the func */
} SelectInfo;

static void DatasetPrioHideCB( Widget, XtPointer, XtPointer );
static void DatasetPrioUpCB( Widget, XtPointer, XtPointer );
static void DatasetPrioDownCB( Widget, XtPointer, XtPointer );
static void DatasetPrioTopCB( Widget, XtPointer, XtPointer );
static void DatasetPrioBottomCB( Widget, XtPointer, XtPointer );
static char *DatasetPrioGet( ThreeDWidget *threeDWidget );
static void DatasetPrioSet( ThreeDWidget *threeDWidget, char const *geoname );
static void SelectDatasetDismiss( Widget, XtPointer, XtPointer );
static void SelectDatasetOkay( Widget, XtPointer, XtPointer );
static void SelectDatasetRefresh( SelectInfo *cb_info );
static void RemoveDatasetCB( ThreeDWidget *, TsGeoPyramid *, void * );
static void DatasetDisplayHideCB( Widget, XtPointer, XtPointer );
static void DatasetDisplayAddCB( Widget, XtPointer, XtPointer );
static void DatasetDisplayRemoveCB( Widget, XtPointer, XtPointer );

/*
 * Synopsis:
 *   void DatasetPrioInit( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   Creates the dialog for the Dataset Priority GUI. This should
 *   be called before any of the other routines that use the
 *   Dataset Priority dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

void
DatasetPrioInit( ThreeDWidget *threeDWidget )
{
  XmString label, title;
  Widget   mainform, topform, form, dismissform, separator, widget;
  Arg      args[10];
  int      n;

  if ( threeDWidget == NULL || threeDWidget->mainForm == NULL )
    return;

  /* Create the main Viewpoint Bookmarks widget instance */

  title = tvXmString( "Dataset Priority" );

  n = 0;
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  threeDWidget->prioWidget = mainform =
    XmCreateFormDialog( threeDWidget->mainForm,
			"Dataset Priority", args, n );

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

  XtAddCallback( widget, XmNactivateCallback, DatasetPrioHideCB, threeDWidget);

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

  /* Create the Set/Add/Del etc buttons below the scrolled list */

  n = 0;
  XtSetArg( args[n], XmNfractionBase, 4 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  form = XtCreateWidget( "buttons", xmFormWidgetClass, topform, args, n );
  XtManageChild( form );

  widget
    = XtVaCreateManagedWidget( " To Top ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 0,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 1,
			       NULL );
  XtAddCallback( widget, XmNactivateCallback, DatasetPrioTopCB,
		 threeDWidget );

  widget
    = XtVaCreateManagedWidget( " Move Up ",
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
  XtAddCallback( widget, XmNactivateCallback, DatasetPrioUpCB,
		 threeDWidget );

  widget 
    = XtVaCreateManagedWidget( " Move Down ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 2,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 3,
			       NULL );
  XtAddCallback( widget, XmNactivateCallback, DatasetPrioDownCB,
		 threeDWidget );

  widget 
    = XtVaCreateManagedWidget( " To Bottom ",
			       xmPushButtonGadgetClass, form,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_POSITION,
			       XmNleftPosition, 3,
			       XmNrightAttachment, XmATTACH_POSITION,
			       XmNrightPosition, 4,
			       NULL );
  XtAddCallback( widget, XmNactivateCallback, DatasetPrioBottomCB,
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
  XtSetArg( args[n], XmNvisibleItemCount, 11 ); n++;

  threeDWidget->prioList = XmCreateScrolledList( topform, "list", args, n );
  XtManageChild( threeDWidget->prioList );

  /* Finally, let's refresh the list with the list of datasets */

  DatasetPrioRefreshWidget( threeDWidget );
}

/*
 * Synopsis:
 *   void DatasetPrioShow( ThreeDWidget * )
 *
 * Description:
 *   Does an XtManageChild on the main Dataset Priority GUI
 *   (checks to see if already managed first though of course)
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

void
DatasetPrioShow( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget && threeDWidget->prioWidget )
    if ( ! XtIsManaged( threeDWidget->prioWidget ) )
      XtManageChild( threeDWidget->prioWidget );
}

/*
 * Synopsis:
 *   void DatasetPrioHide( ThreeDWidget * )
 *
 * Description:
 *   Does an XtUnmanageChild on the main Dataset Priority GUI
 *   (checks to see if already unmanaged first though of course).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
DatasetPrioHide( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget && threeDWidget->prioWidget )
    if ( XtIsManaged( threeDWidget->prioWidget ) )
      XtUnmanageChild( threeDWidget->prioWidget );
}

static void
DatasetPrioHideCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  DatasetPrioHide( (ThreeDWidget *) client_data );
}

/*
 * Synopsis:
 *   void DatasetPrioRefreshWidget( ThreeDWidget * )
 *
 * Description:
 *   Takes the current list of datasets that are associated with a
 *   ThreeDWidget and updates the contents of the list view in the
 *   Dataset Priority window. 
 *
 *   N.B. the first dataset in the geoList is the first one to be
 *   rendered, i.e. the lowest priority. Therefore we need to display
 *   the last dataset in the geolist at the top of the list (first),
 *   and vice versa.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

typedef struct {
  XmString    *str;
  int         entries, maxstrings;
} DatasetListClosure;

static void
DatasetListCB (TsGeoPyramid *geo, void *closure)
{
  DatasetListClosure *cl = closure;
  int i;

  if ( geo == NULL ) return;

  /* add an entry for this geoset to the list of XmStrings */
  if (cl->maxstrings)
    /* put the first dataset last and the last dataset first.    */
    i = cl->maxstrings - ++cl->entries;
  else
    i = cl->entries++;
  cl->str[i] = tvXmString( TsGeoName( geo ) );
}

void
DatasetPrioRefreshWidget( ThreeDWidget *threeDWidget )
{
  DatasetListClosure cl;
  int         i;

  if ( threeDWidget == NULL || threeDWidget->prioList == NULL ) return;

  /* Create an array of XmStrings to put the database names into */
  /* Only add entries to list if they match the current dataset name */

  cl.maxstrings = tvNumDatasets( threeDWidget->threeD );
  cl.str = (XmString *) XtMalloc( cl.maxstrings * sizeof(XmString) );
  if ( cl.str == NULL ) return;
  
  /* go through the list of all GeoSets for this ThreeDWidget  */

  cl.entries = 0;
  tvForallDatasets (threeDWidget->threeD, DatasetListCB, &cl);

  /* Set the list widget's contents to this new XmString array */

  XtVaSetValues( threeDWidget->prioList, XmNitemCount, cl.entries,
		 XmNitems, cl.str, XmNselectedItemCount, 0, NULL );

  /* set the sensitivity of the Set/Add/Del buttons appropriately */

  /* Clean up behind us */

  for ( i = 0; i < cl.entries; i++ )
    XmStringFree( cl.str[i] );
  XtFree( (char *) cl.str );
}

/*
 * Synopsis:
 *   char *DatasetPrioGet( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   Returns the string for the currently selected entry in the
 *   Dataset Priority list widget. This will be the name of the
 *   dataset, as returned by TsGeoName().
 *
 *   You must free any non-NULL string returned by this program
 *   using XtFree();
 *
 * Returns: a valid string or NULL if no selection
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static char *
DatasetPrioGet( ThreeDWidget *threeDWidget )
{
  XmStringTable xm_selected;
  char          *string = NULL;
  int           nSelected;

  /* get the currently selected item from the list widget */

  XtVaGetValues( threeDWidget->prioList, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, NULL );

  /* return NULL if there is nothing selected right now */

  if ( nSelected == 0 ) return NULL;

  /* get the currently selected viewpoint name */

  XmStringGetLtoR( xm_selected[0], XmFONTLIST_DEFAULT_TAG, &string );
  return string;
}

/*
 * Synopsis:
 *   void DatasetPrioSet( ThreeDWidget *threeDWidget, char const *geoname )
 *
 * Description:
 *   Takes the string that was returned by DatasetPrioGet(). It will
 *   perform a refresh on the list widget of the Dataset Priority window
 *   and then highlight the named dataset as the currently selected one.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static void
DatasetPrioSet( ThreeDWidget *threeDWidget, char const *geoname )
{
  XmString  selected[1];

  /* refresh the list of datasets in the Dataset Priority dialog */

  DatasetPrioRefreshWidget( threeDWidget );

  /* set this named dataset to be the currently selected */

  selected[0] = tvXmString( geoname );
  XtVaSetValues( threeDWidget->prioList, XmNselectedItems, selected,
		 XmNselectedItemCount, 1, NULL );
  XmStringFree( selected[0] );

  /* update the ThreeDWidget's display to reflect the new priority */

  tvBaseUpdateView( threeDWidget->threeD, FALSE, FALSE );

}

/*
 * Synopsis:
 *   void DatasetPrioTopCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "To Top"
 *   button, directly below the list widget of the Dataset Priority dialog.
 *   It will reorder the currently selected dataset in the ThreeDWidget's
 *   geoList so that it appears at the end of the list (not the start,
 *   because the first dataset in the list has the lowest priority).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static void
DatasetPrioTopCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget  *threeDWidget = (ThreeDWidget *) client_data;
  char          *geoname = NULL;
  TsGeoPyramid  *geo;

  /* get the currently selected item from the list widget */

  if ( ( geoname = DatasetPrioGet( threeDWidget ) ) == NULL )
    return;

  /* Find the named dataset and move it to highest priority */

  geo = tvGetDataset (threeDWidget->threeD, geoname);
  if (geo) {
    tvDatasetSetPriority (threeDWidget->threeD, geo, -1);
    DatasetPrioSet( threeDWidget, geoname );
  }

  XtFree( geoname );
}

/*
 * Synopsis:
 *   void DatasetPrioUpCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "Up"
 *   button, directly below the list widget of the Dataset Priority dialog.
 *   It will reorder the currently selected dataset in the ThreeDWidget's
 *   geoList so that it appears one position further in the list (the
 *   first position in the list is the lowest priority).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static void
DatasetPrioUpCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char         *geoname = NULL;
  TsGeoPyramid  *geo;

  /* get the currently selected item from the list widget */

  if ( ( geoname = DatasetPrioGet( threeDWidget ) ) == NULL )
    return;

  /* Find the named dataset and move it to a higher priority */

  geo = tvGetDataset (threeDWidget->threeD, geoname);
  if (geo) {
    int prio = tvDatasetPriority (threeDWidget->threeD, geo);
    tvDatasetSetPriority (threeDWidget->threeD, geo, prio+1);
    DatasetPrioSet( threeDWidget, geoname );
  }

  XtFree( geoname );
}

/*
 * Synopsis:
 *   void DatasetPrioDownCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "Down"
 *   button, directly below the list widget of the Dataset Priority dialog.
 *   It will reorder the currently selected dataset in the ThreeDWidget's
 *   geoList so that it appears one position earlier in the list (the
 *   first position in the list is the lowest priority).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static void
DatasetPrioDownCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char         *geoname = NULL;
  TsGeoPyramid  *geo;

  /* get the currently selected item from the list widget */

  if ( ( geoname = DatasetPrioGet( threeDWidget ) ) == NULL )
    return;

  /* Find the named dataset and move it to a lower priority */

  geo = tvGetDataset (threeDWidget->threeD, geoname);
  if (geo) {
    int prio = tvDatasetPriority (threeDWidget->threeD, geo);
    tvDatasetSetPriority (threeDWidget->threeD, geo, prio-1);
    DatasetPrioSet( threeDWidget, geoname );
  }

  XtFree( geoname );
}

/*
 * Synopsis:
 *   void DatasetPrioBottomCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "To Bottom"
 *   button, directly below the list widget of the Dataset Priority dialog.
 *   It will reorder the currently selected dataset in the ThreeDWidget's
 *   geoList so that it appears at the start of the list (not the end,
 *   because the first dataset in the list has the lowest priority).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

static void
DatasetPrioBottomCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char          *geoname = NULL;
  TsGeoPyramid  *geo;

  /* get the currently selected item from the list widget */

  if ( ( geoname = DatasetPrioGet( threeDWidget ) ) == NULL )
    return;

  /* Find the named dataset and move it to lowest priority */

  geo = tvGetDataset (threeDWidget->threeD, geoname);
  if (geo) {
    tvDatasetSetPriority (threeDWidget->threeD, geo, 0);
    DatasetPrioSet( threeDWidget, geoname );
  }

  XtFree( geoname );
}

/*
 * Synopsis:
 *   void SelectDatasetDismiss( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user selects the "Dismiss" button
 *   on a Dataset Selector dialog, as opened with the SelectDataset call.
 *   It will destroy the Selector dialog and free all resources that we
 *   were using for it.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

static void
SelectDatasetDismiss( Widget w, XtPointer client_data, XtPointer call_data )
{
  SelectInfo *cb_info = (SelectInfo *) client_data;
  XtUnmanageChild( *cb_info->dialog );
  XtDestroyWidget( *cb_info->dialog );
  *cb_info->dialog = NULL;
  list_remove( select_widgets, cb_info );
  tsmMemFree( cb_info );
}

/*
 * Synopsis:
 *   void SelectDatasetOkay( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user selects a datset from
 *   a Dataset Selector dialog, as opened with the SelectDataset call.
 *   It will call the specified callback function with the TsGeoPyramid
 *   structure for the dataset that the user selected.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

static void
SelectDatasetOkay( Widget w, XtPointer client_data, XtPointer call_data )
{
  SelectInfo    *cb_info = (SelectInfo *) client_data;
  XmStringTable xm_selected;
  char          *string = NULL;
  int           nSelected;
  list_iter_t   i;
  TsGeoPyramid  *geop = NULL;

  /* get the currently selected item from the list widget */

  XtVaGetValues( cb_info->list, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, NULL );

  /* return NULL if there is nothing selected right now */

  if ( nSelected == 0 ) return;

  /* get the currently selected viewpoint name */

  XmStringGetLtoR( xm_selected[0], XmFONTLIST_DEFAULT_TAG, &string );

  /* Find the TsGeoPyramid for this named dataset */

  for ( i = list_first( geoList ); ! list_done( i ); i = list_next( i ) ) {
    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
    if ( geo && strcmp( string, TsGeoName( geo ) ) == 0 ) {
      geop = geo;
      break;
    }
  }

  /* call the users callback, dismiss the dialog, and free all resources */

  if ( geop )
    (*cb_info->callback)( cb_info->threeDWidget, geop, cb_info->closure );
 
  SelectDatasetDismiss( w, client_data, call_data );
  XtFree( string );
}

/*
 * Synopsis:
 *   void SelectDatasetFromCore( SelectInfo *cb_info, tvBase *threeD )
 *
 * Description:
 *   This function is used to refresh a single Dataset Selector dialog.
 *   We pass in the SelectInfo structure that provides all of the 
 *   information that we need about the Selector and the associated
 *   ThreeDWidget, then we only display the datasets that are being used by
 *   that widget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

static void
SelectDatasetFromCore( SelectInfo *cb_info, tvBase *threeD )
{
  int         i;
  DatasetListClosure cl;

  /* Create an array of XmStrings to put the geoset names into */

  cl.str = (XmString *) XtMalloc( tvNumDatasets (threeD) * sizeof(XmString) );
  if ( cl.str == NULL ) return;

  cl.maxstrings = 0;
  cl.entries = 0;
  tvForallDatasets (threeD, DatasetListCB, &cl);

  /* Set the list widget's contents to this new XmString array */

  XtVaSetValues( cb_info->list,
		 XmNitemCount, cl.entries,
		 XmNitems, cl.str, NULL );

  /* Clean up behind us */

  for ( i = 0; i < cl.entries; i++ ) XmStringFree( cl.str[i] );
  XtFree( (char *) cl.str );
}

/*
 * Synopsis:
 *   void SelectDatasetRefresh( SelectInfo *cb_info )
 *
 * Description:
 *   This function is used to refresh a single Dataset Selector dialog.
 *   We pass in the SelectInfo structure that provides all of the 
 *   information that we need about the Selector. 
 *
 *   If the Selector is associated with a particular ThreeDWidget, then
 *   we only display the datasets that are being used by that widget;
 *   otherwise, we display TerraVision's global list of all datasets.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

static void
SelectDatasetRefresh( SelectInfo *cb_info )
{
  /* Create an array of XmStrings to put the geoset names into */

  if ( cb_info->threeDWidget ) {
    SelectDatasetFromCore (cb_info, cb_info->threeDWidget->threeD);
  } else {
    XmString    *str;
    int         i, entries = 0;
    list_iter_t g;

    str = (XmString *) XtMalloc( list_count(geoList) * sizeof(XmString) );
    if ( str == NULL ) return;
  
    /* go through the list of all GeoSets and add their names to the array */
    
    for ( g = list_first( geoList ); ! list_done( g ); g = list_next( g ) ) {

      /* get the next GeoPyramid in this list */
    
      TsGeoPyramid *geo;

      geo = (TsGeoPyramid *) list_value( g );

      /* add it's name to the list */

      if ( geo != NULL )
	str[entries++] = tvXmString( TsGeoName( geo ) );
    }

    /* Set the list widget's contents to this new XmString array */

    XtVaSetValues( cb_info->list, XmNitemCount, entries, XmNitems, str, NULL );

    /* Clean up behind us */

    for ( i = 0; i < entries; i++ ) XmStringFree( str[i] );
    XtFree( (char *) str );
  }
}

/*
 * Synopsis:
 *   void SelectDataset( ThreeDWidget *, Widget *, char const *,
 *                       char const *, DatasetSelectProc *, void * )
 *
 * Description:
 *   This is a cool list function that provides a generic GUI for doing
 *   some operation that requires the user to specify a dataset. For
 *   example, removing a dataset or adding a bookmark to a dataset.
 *
 *   The function will create a dialog with a ScrolledList widget that
 *   displays all of the available datasets and lets the user select
 *   one of them. If the user selects a dataset to operate on, then
 *   the specified callback function is called with the specified 
 *   closure. The callback function should look like:
 *
 *     void do_operation( ThreeDWidget *, TsGeoPyramid *, void * )
 *
 *   You can specify the title of the dialog (title) and the text that
 *   is used on the 'okay' button (button).
 * 
 *   You also supply a pointer to a Widget structure for this dialog.
 *   This is done so that you never have multiple versions of the same
 *   dialog up at any time. If you call this function, and the dialog
 *   pointer points to a non-NULL Widget structure, then we simply do
 *   an XtManageChild() call on that Widget. N.B. when the Dataset
 *   Selector dialog is dismissed, then the dialog widget is destroyed
 *   and we set the pointed-to widget to NULL.
 *
 *   Finally, this function can be used to list all of the datasets that
 *   are currently loaded by TerraVision, or it can be used to display
 *   only those datasets that are being used by a specific ThreeDWidget.
 *   If you specify a NULL threeDWidget structure, then it is assumed
 *   that you want to list all loaded datasets. For example, to remove
 *   a dataset from TerraVision's global list you would pass a NULL
 *   threeDWidget; to add a bookmark for a specific view you would
 *   probably want to pass the appropriate ThreeDWidget structure.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

void
SelectDataset( ThreeDWidget *threeDWidget, Widget *dialog, char const *title,
	       char const *button, DatasetSelectProc *callback,
	       void *closure )
{
  XmString    label;
  Widget      parent, mainform, topform, dismissform, separator, widget;
  Arg         args[10];
  int         n = 0;
  SelectInfo  *cb_info;

  /* check the validity of the parameters that we have been given */

  if ( callback == NULL || title == NULL || button == NULL )
    return;

  /* if the widget has already been created, then just refresh it */

  if ( *dialog != NULL ) {
    XtManageChild( *dialog );
    return;
  }

  /* work out the parent widget. If we have a ThreeDWidget structure, */
  /* then use it's mainForm, otherwise we use the main TV menu form.  */

  if ( threeDWidget == NULL )
    parent = applicationShell;
  else
    parent = threeDWidget->mainForm;

  /* Create the main Viewpoint Bookmarks widget instance */

  label = tvXmString( title );

  n = 0;
  XtSetArg( args[n], XmNdialogTitle, label ); n++;

  *dialog = mainform =
    XmCreateFormDialog( parent, "DatasetSelectorWidget", args, n );

  XmStringFree( label );

  /* allocate a SelectInfo structure to pass to the callbacks */

  cb_info = (SelectInfo *) tsmMemCalloc( sizeof(SelectInfo), 1 );
  if ( cb_info == NULL ) return;

  cb_info->callback     = callback;
  cb_info->dialog       = dialog;
  cb_info->closure      = closure;
  cb_info->threeDWidget = threeDWidget;

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

  label = tvXmString( button );
  widget =  XtVaCreateManagedWidget( "okay",
				     xmPushButtonGadgetClass, dismissform,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftOffset, 30,
				     XmNrightOffset, 20,
				     XmNshowAsDefault, 1,
				     XmNlabelString, label,
				     NULL );
  XmStringFree( label );

  XtAddCallback( widget, XmNactivateCallback, SelectDatasetOkay, cb_info );

  label = tvXmString( "Dismiss" );
  widget =  XtVaCreateManagedWidget( "dismiss",
				     xmPushButtonGadgetClass, dismissform,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftOffset, 30,
				     XmNrightOffset, 20,
				     XmNshowAsDefault, 1,
				     XmNlabelString, label,
				     NULL );
  XmStringFree( label );

  XtAddCallback( widget, XmNactivateCallback, SelectDatasetDismiss, cb_info );

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

  /* Create the scrolled text widget where all the viewpoints are listed */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 11 ); n++;

  cb_info->list = XmCreateScrolledList( topform, "list", args, n );
  XtManageChild( cb_info->list );

  /* refresh the contents of the ScrolledList with the list of    */
  /* all geosets, either the global list of the ThreeDWidget list */

  SelectDatasetRefresh( cb_info );

  /* add this selection widget to our global list of open selectors */

  if ( select_widgets == NULL )
    select_widgets = list_create( FALSE );

  list_push( select_widgets, cb_info );

  /* finally pop up this dialog */

  XtManageChild( *dialog );
}

/*
 * Synopsis:
 *   void RefreshDatasetSelectors( void )
 *
 * Description:
 *   This function will go through all of the Dataset Selector dialogs
 *   that are currently open and refresh the list of datasets that 
 *   they are displaying. These dialogs are opened by calling the
 *   SelectDataset function, above.
 *
 *   This function is used to refresh the dialogs with the new set
 *   of datsets whenever we add or remove datasets to the application.
 *
 * Returns: A Widget structure for the dialog
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

void
RefreshDatasetSelectors( void )
{
  list_iter_t i;

  if ( select_widgets == NULL ) return;

  for ( i = list_first(select_widgets); ! list_done(i); i = list_next(i) ) {
    SelectDatasetRefresh( (SelectInfo *) list_value( i ) );    
  }
}

/*
 * Synopsis:
 *   void RemoveDatasetCB( ThreeDWidget *, TsGeoPyramid *, void *)
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "Close Dataset"
 *   widget of the "File/Close Dataset" dialog. We are given the
 *   TsGeoPyramid structure for the dataset that was selected and we
 *   proceed to unlink it from all open ThreeDWidgets, and then remove it
 *   from the global list of geos.
 *
 *   The ThreeDWidget structure is given to us by the SelectDataset
 *   callback, but it is not used in this case (it will be NULL
 *   anyway).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

static void
RemoveDatasetCB( ThreeDWidget *threeDWidget, TsGeoPyramid *geo, void *closure )
{
  list_iter_t i;

  if ( geo == NULL ) return;

  /* go through all of the tvBase's and remove any */
  /* reference to this geoset from all of them.          */

  tvForallWindows ((void (*)(tvBase*,void*)) tvRemoveDataset, geo);

  /* now go through the global list of geos and remove this entry */

  for ( i = list_first( geoList ); ! list_done( i ); ) {
    TsGeoPyramid *geop = (TsGeoPyramid *) list_value( i );
    if ( geop && geop == geo ) {

      /* remove the TsGeoPyramid and clean everything up */

      TsGeoDestroy( geo );
      i = list_delete( i );
      break;

    } else
      i = list_next( i );
  }

  /* refresh the sensitivity of the Views menu entries, i.e. these */
  /* should get ghosted out when there are no more datasets loaded */

  TerraVisionRefreshMenus();

  /* refresh the Dataset Info window in case it's open */

  InfoRefreshDatasetList();

  /* Refresh any map viewers */

  ScannedMapRefreshLists();

}

/*
 * Synopsis:
 *   void RemoveDataset( void )
 *
 * Description:
 *   This function is called in order to remove a dataset from the
 *   global list of datasets, and also from any of the ThreeDWidgets
 *   that are displaying this dataset. The user is given a list of
 *   all loaded datasets and can select which one they want to remove.
 *   
 *   This operation is what gets done when the "Close Dataset" menu
 *   option is selected.
 *
 *   We usee the SelectDataset() convenience function to implement the
 *   GUI. If the users selects a dataset to remove, then we call the
 *   function RemoveDatasetCB (above) on that dataset.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 June 1999
 *
 */

void
RemoveDataset( void )
{
  static Widget dataset_error = NULL;

  if ( list_count( geoList ) == 0 )
    GenericErrorDialog( applicationShell, &dataset_error, 
			"There are no datasets currently loaded!" );
  else
    SelectDataset( NULL, &remove_widget, "Select a Dataset to Close",
		   "Close Dataset", RemoveDatasetCB, NULL );
}

/*
 * Synopsis:
 *   void DatasetPrioRefreshWidget( ThreeDWidget * )
 *
 * Description:
 *   Takes the current list of datasets that are associated with a
 *   ThreeDWidget and updates the contents of the list view in the
 *   Dataset Priority window. 
 *
 *   N.B. the first dataset in the geoList is the first one to be
 *   rendered, i.e. the lowest priority. Therefore we need to display
 *   the last dataset in the geolist at the top of the list (first),
 *   and vice versa.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 June 1999
 *
 */

void
DatasetDisplayRefreshWidget( ThreeDWidget *threeDWidget )
{
  XmString    *str;
  int         i, entries, maxstrings;
  list_iter_t g;
  DatasetListClosure cl;

  if ( threeDWidget == NULL || threeDWidget->dsWidget == NULL ) return;
  if ( geoList == NULL ) return;

  /*-------------- update the Available Datasets list ---------------*/

  /* Create an array of XmStrings to put the database names into */
  /* Only add entries to list if they match the current dataset name */

  maxstrings = list_count( geoList );
  str = (XmString *) XtMalloc( maxstrings * sizeof(XmString) );
  if ( str == NULL ) return;
  
  /* go through the list of all GeoSets for this ThreeDWidget  */

  for ( g = list_first( geoList ), entries = 0;
	! list_done( g );
	g = list_next( g ) ) {

    /* add an entry for this geoset to the ScrolledList widget */
    
    TsGeoPyramid *geo = (TsGeoPyramid *) list_value( g );
    if ( geo != NULL ) {

      /* only add an entry to Avail Datasets if not in Displayed Datasets */
      if (! tvGetDataset (threeDWidget->threeD, TsGeoName(geo))) {
	str[entries++] = tvXmString( TsGeoName( geo ) );
      }
    }

  }

  /* Set the list widget's contents to this new array, and clean up */

  XtVaSetValues( threeDWidget->dsList1, XmNitemCount, entries,
		 XmNitems, str, XmNselectedItemCount, 0, NULL );

  for ( i = 0; i < entries; i++ ) XmStringFree( str[i] );
  XtFree( (char *) str );

  /*--------------- update the Displayed Datasets list ----------------*/

  /* Create an array of XmStrings to put the database names into */
  /* Only add entries to list if they match the current dataset name */

  cl.maxstrings = tvNumDatasets ( threeDWidget->threeD );
  cl.str = (XmString *) XtMalloc( cl.maxstrings * sizeof(XmString) );
  if ( cl.str == NULL ) return;
  
  /* go through the list of all GeoSets for this ThreeDWidget  */

  cl.entries = 0;
  tvForallDatasets (threeDWidget->threeD, DatasetListCB, &cl);

  /* Set the list widget's contents to this new array, and clean up */

  XtVaSetValues( threeDWidget->dsList2, XmNitemCount, cl.entries,
		 XmNitems, cl.str, XmNselectedItemCount, 0, NULL );

  for ( i = 0; i < cl.entries; i++ ) XmStringFree( cl.str[i] );
  XtFree( (char *) cl.str );

}

/*
 * Synopsis:
 *   void DatasetDisplayInit( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   This function will setup the Dataset Display dialog, which lets
 *   you add and remove the datasets that you wish to display in any
 *   specific ThreeDWidget. This function must be called before
 *   any of the other Dataset Display GUI functions.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 June 1999
 *
 */

void
DatasetDisplayInit( ThreeDWidget *threeDWidget )
{
  XmString label, title;
  Widget   mainform, topform, leftform, midform, rightform;
  Widget   dismissform, separator, widget, label1, label2;
  Arg      args[10];
  int      n;

  if ( threeDWidget == NULL || threeDWidget->mainForm == NULL )
    return;

  /* Create the main Viewpoint Bookmarks widget instance */

  title = tvXmString( "Dataset Select" );

  n = 0;
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  threeDWidget->dsWidget = mainform =
      XmCreateFormDialog( threeDWidget->mainForm,
			"Dataset Select", args, n );

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

  XtAddCallback( widget, XmNactivateCallback, DatasetDisplayHideCB,
		 threeDWidget);

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

  leftform =
    XtVaCreateManagedWidget( "leftForm",
			     xmFormWidgetClass, topform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );

  rightform =
    XtVaCreateManagedWidget( "rightForm",
			     xmFormWidgetClass, topform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );

  midform =
    XtVaCreateManagedWidget( "midForm",
			     xmFormWidgetClass, topform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_WIDGET,
			     XmNleftWidget, leftform,
			     XmNrightAttachment, XmATTACH_WIDGET,
			     XmNrightWidget, rightform,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 0,
			     NULL );

  /* add the labels to the top of both ScrolledList forms */

  label = tvXmString( "Available Datasets" );
  label1 =  XtVaCreateManagedWidget( "avail",
				     xmLabelGadgetClass, leftform,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNlabelString, label, NULL );
  XmStringFree( label );

  label = tvXmString( "Displayed Datasets" );
  label2 =  XtVaCreateManagedWidget( "display",
				     xmLabelGadgetClass, rightform,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNlabelString, label, NULL );
  XmStringFree( label );

  /* Create the scrolled text widgets where all the datasets are listed */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, label1 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmMULTIPLE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 11 ); n++;

  threeDWidget->dsList1 = XmCreateScrolledList( leftform, "list1", args, n );
  XtManageChild( threeDWidget->dsList1 );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, label2 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmMULTIPLE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 11 ); n++;

  threeDWidget->dsList2 = XmCreateScrolledList( rightform, "list2", args, n );
  XtManageChild( threeDWidget->dsList2 );

  /* create the Add/Remove buttons between the two lists */

  label = tvXmString( " >> Add " );
  widget =  XtVaCreateManagedWidget( "add",
				     xmPushButtonGadgetClass, midform,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNtopOffset, 100,
				     XmNlabelString, label, NULL );
  XmStringFree( label );

  XtAddCallback( widget, XmNactivateCallback, DatasetDisplayAddCB,
		 threeDWidget);
  
  label = tvXmString( " << Remove " );
  widget =  XtVaCreateManagedWidget( "remove",
				     xmPushButtonGadgetClass, midform,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, widget,
				     XmNlabelString, label, NULL );
  XmStringFree( label );

  XtAddCallback( widget, XmNactivateCallback, DatasetDisplayRemoveCB,
		 threeDWidget);

  DatasetDisplayRefreshWidget( threeDWidget );
}

/*
 * Synopsis:
 *   void DatasetDisplayShow( ThreeDWidget * )
 *
 * Description:
 *   Does an XtManageChild on the main Dataset Display GUI
 *   (checks to see if already managed first though of course)
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 June 1999
 *
 */

void
DatasetDisplayShow( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget && threeDWidget->dsWidget )
    if ( ! XtIsManaged( threeDWidget->dsWidget ) )
      XtManageChild( threeDWidget->dsWidget );
}

/*
 * Synopsis:
 *   void DatasetDisplayHide( ThreeDWidget * )
 *
 * Description:
 *   Does an XtUnmanageChild on the main Dataset Display GUI
 *   (checks to see if already unmanaged first though of course).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 June 1998
 *
 */

void
DatasetDisplayHide( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget && threeDWidget->dsWidget )
    if ( XtIsManaged( threeDWidget->dsWidget ) )
      XtUnmanageChild( threeDWidget->dsWidget );
}

static void
DatasetDisplayHideCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  DatasetDisplayHide( (ThreeDWidget *) client_data );
}

/*
 * Synopsis:
 *   void DatasetDisplayAddCB( ThreeDWidget * )
 *
 * Description:
 *   Adds all of the selected datasets in the Available Datasets
 *   list to the current ThreeDWidget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 June 1998
 *
 */

static void
DatasetDisplayAddCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget  *threeDWidget = (ThreeDWidget *) client_data;
  XmStringTable xm_selected;
  char          *string = NULL, **name;
  int           nSelected, i;
  list_iter_t   g;

  /* get the currently selected item from the list widget */

  XtVaGetValues( threeDWidget->dsList1, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, NULL );

  if ( nSelected == 0 ) return;

  if ( ( name = tsmMemMalloc( sizeof(char *) * nSelected ) ) == NULL )
    return;

  for ( i = 0; i < nSelected; i++ ) {
    XmStringGetLtoR( xm_selected[i], XmFONTLIST_DEFAULT_TAG, &string );
    name[i] = tsmMemStrdup( string );
    XtFree( string );
  }

  /* get the currently selected viewpoint name */

  for ( i = 0; i < nSelected; i++ ) {

    /* get the TsGeoPyramid structure for this dataset name */

    for ( g = list_first( geoList ); ! list_done( g ); g = list_next( g ) ) {
      TsGeoPyramid *geo = (TsGeoPyramid *) list_value( g );
      if ( geo != NULL && strcmp( TsGeoName(geo), name[i] ) == 0 )
	tvAddDataset( threeDWidget->threeD, geo );
    }
    
  }
  
  /* free all of the temporary strings */

  for ( i = 0; i < nSelected; i++ )
    tsmMemFree( name[i] );
  tsmMemFree( name );

  /* refresh the selection window */

  DatasetDisplayRefreshWidget( threeDWidget );
}

/*
 * Synopsis:
 *   void DatasetDisplayRemoveCB( ThreeDWidget * )
 *
 * Description:
 *   Removes all of the selected datasets in the Displayed Datasets
 *   list from the current ThreeDWidget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 June 1998
 *
 */

static void
DatasetDisplayRemoveCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget  *threeDWidget = (ThreeDWidget *) client_data;
  XmStringTable xm_selected;
  char          *string;
  int           nSelected, i;
  TsGeoPyramid  **geo;

  /* get the currently selected item from the list widget */

  XtVaGetValues( threeDWidget->dsList2, XmNselectedItems, &xm_selected,
		 XmNselectedItemCount, &nSelected, NULL );

  if ( nSelected == 0 )
    return;

  if ( ( geo = malloc( nSelected * sizeof(TsGeoPyramid *) ) ) == NULL )
    return;

  for ( i = 0; i < nSelected; i++ ) {
    XmStringGetLtoR( xm_selected[i], XmFONTLIST_DEFAULT_TAG, &string );
    geo[i] = tvGetDataset ( threeDWidget->threeD, string );
    XtFree( string );
  }

  /* remove all of the geos that were selected */

  for ( i = 0; i < nSelected; i++ )
    if ( geo[i] )
      tvRemoveDataset (threeDWidget->threeD, geo[i] );

  tsmMemFree( geo );

}
