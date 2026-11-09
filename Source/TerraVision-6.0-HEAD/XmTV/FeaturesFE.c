/*
 * Name: Features.c
 *
 * Description: This file handles the placement and drawing of the 
 *    building models, in two and three d.
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
#include "VRML.h"
#include "Database.h"
#include "Info.h"

#include <stdio.h>

#include <vrml2/vrml2.h>
#include <vrml2/c_nodes.h>

#include <tsm/tsm.h>

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>

#include "BusyDialog.h"
#include "Features.h"
#include "FeaturesFE.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"

/* the global state for this module is the list of all loaded features */
/* feature_list is simply a list of FeatureItem structures.            */

extern int    last_id;
extern list_t *feature_list;
extern list_t *texture_list;

/* list the prototypes for this module's private functions */

static void FeatureSelectInit( ThreeDWidget *threeDWidget );
static void FeatureSelectDismissCB( Widget, XtPointer, XtPointer );
static int FeatureMakeGeoSetList( ThreeDWidget *, char const *, Widget, int );
static void FeatureGenerateList( ThreeDWidget *threeDWidget, int );
static void FeatureSelectedCB( Widget, XtPointer, XtPointer );
static void FeatureInfoCB( Widget, XtPointer, XtPointer );
static void FeatureToggleCB( Widget, XtPointer, XtPointer );
static void FeatureLoadCB( BusyDialog *busy );

/*
 * Synopsis:
 *   void FeatureLoadCB( BusyDialog *busy )
 *
 * Description:
 *   This is a BusyDialog callback that is used to load a VRML feature 
 *   file into memory. It will open the feature file's URL, and parse
 *   its contents into the item->wrl variable. Then it will extract
 *   the Metadata definitions appropriately and refresh the screen.
 *
 *   When this function (thread) terminates successfully, it sets
 *   the loaded variable of the FeatureItem to TRUE to signal that this
 *   feature is now ready to be rendered.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

typedef struct {
  ThreeDWidget *threeD;
  FeatureItem *item;
} FeatureLoadClosure;

static void
FeatureLoadCB( BusyDialog *busy )
{
  /*static const char funcName[] = "FeatureLoadCB";*/
  FeatureLoadClosure *closure = busy->closure;
  ThreeDWidget       *threeD = closure->threeD;

  SetupBusyCallback (threeD, busy);

  closure->item = FeatureLoad (threeD->threeD, closure->item);

  tsmMemFree( busy->closure );
  busy->closure = NULL;

  ClearBusyCallback (threeD);
  CloseBusyDialog( busy );
}

/*
 * Synopsis:
 *   int FeatureGet( ThreeDWidget *threeDWidget, char *url )
 *
 * Description:
 *   This function will attempt to load the VRML feature file at the
 *   specified URL. A unique ID is returned to identify that feature.
 *   We can then use FeatureGetItem() in order to retrieve the
 *   FeatureItem structure for the given ID.
 *
 *   First we check to see if the specified URL has already been
 *   loaded (it exists in our feature list). If this is the case,
 *   then we just return the ID of the already loaded feature.
 *
 *   Otherwise, we create a new FeatureItem structure and assign
 *   a unique ID to it. Then we vector off to the load thread in
 *   order to load this feature. 
 *
 *   This function will return with a valid ID before the feature
 *   file is loaded. Use the loaded field of the FeatureItem 
 *   structure to determine whether the feature has been fully
 *   loaded and is ready to be rendered.
 *
 * Returns: A unique ID for this new feature, or -1 if load failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static int
FeatureGet( ThreeDWidget *threeDWidget, char *url )
{
  FeatureLoadClosure *closure;
  char               message[512];
  FeatureItem        *item = GetFeatureFromUrl (url);

  /* have we already loaded this feature? */

  if (item) {
    item->refcount++;
    return item->id;
  }

  /* allocate a new closure structure */

  closure = (FeatureLoadClosure *) tsmMemCalloc(1, sizeof(FeatureLoadClosure));
  if ( closure == NULL )
    return -1;

  /* now pop up a busy dialog and start to load in the new feature file */

  sprintf( message, "Loading Features from URL:\n%s", url );

  closure->threeD = threeDWidget;
  closure->item = FeatureAlloc (url, TRUE);

  OpenBusyDialog( threeDWidget->mainForm, message, FeatureLoadCB, closure );

  return ( closure && closure->item ) ? closure->item->id : -1;
}

/*
 * Synopsis:
 *   void FeatureSelectInit( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   Builds a dialog widget for the Feature Selection option. This
 *   lets the user toggle all of the available features on or off
 *   (load/unload), and also let's then set various global effects
 *   for features such as wireframe, lighting, and texture-mapping.
 *
 *   threeDWidget->fselWidget is the dialog widget that is created.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static void
FeatureSelectInit( ThreeDWidget *threeDWidget )
{
  Widget   mainform, widget, dismissform, leftform, rightform;
  Widget   separator, frame;
  XmString string;
  Arg      args[15];
  int      n = 0;
  int val;

  /* Create the main dialog window for the feature selection */

  string = tvXmString( "Feature Selection" );
  XtSetArg( args[n], XmNdialogTitle, string ); n++;
  
  threeDWidget->fselWidget = mainform =
      XmCreateFormDialog( threeDWidget->mainForm, "Features", args, n );

  XmStringFree( string );

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

  string = tvXmString( "Dismiss" );
  widget = 
    XtVaCreateManagedWidget( "dismiss",
			     xmPushButtonGadgetClass, dismissform,
			     XmNleftOffset, 30,
			     XmNrightOffset, 20,
			     XmNshowAsDefault, 1,
			     XmNlabelString, string,
			     NULL );
  XmStringFree( string );

  XtAddCallback( widget, XmNactivateCallback, FeatureSelectDismissCB,
		 threeDWidget );

  /* put the scrolled text widget at the bottom for the model info */

  n = 0;
  XtSetArg( args[n], XmNscrollVertical, True ); n++;
  XtSetArg( args[n], XmNscrollHorizontal, True ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNwordWrap, True ); n++;
  XtSetArg( args[n], XmNrows, 4 ); n++;
  XtSetArg( args[n], XmNscrolledWindowMarginHeight, 5 ); n++;
  XtSetArg( args[n], XmNscrolledWindowMarginWidth, 5 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNbottomWidget, separator ); n++;

  threeDWidget->fselInfoText = XmCreateScrolledText(mainform, "info", args, n);
  XtManageChild( threeDWidget->fselInfoText );

  /* do the right-hand side of the selection to allow options */
  
  rightform =
    XtVaCreateManagedWidget( "rightForm",
			     xmFormWidgetClass, mainform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_WIDGET,
			     XmNbottomWidget, threeDWidget->fselInfoText,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );
  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_ETCHED_IN ); n++;
  frame = XmCreateFrame( rightform, "rightFrame", args, n );
  XtManageChild( frame );

  frame =
    XtVaCreateManagedWidget( "rowcol1", xmRowColumnWidgetClass, frame,
			     XmNorientation, XmVERTICAL,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftOffset, 15,
			     XmNrightOffset, 15,
			     NULL );

  val = tvGetBoolean (threeDWidget->threeD, TV_FEATURES_TEXTURED);
  threeDWidget->fselTextured =
    XtVaCreateManagedWidget( "Textured",
			     xmToggleButtonGadgetClass, frame,
			     XmNset, val,
			     XmNindicatorOn, True,
			     NULL );
  XtAddCallback( threeDWidget->fselTextured, XmNvalueChangedCallback,
		 FeatureToggleCB, threeDWidget );

  val = tvGetBoolean (threeDWidget->threeD, TV_FEATURES_LIGHTING);
  threeDWidget->fselLighting =
    XtVaCreateManagedWidget( "Lighting",
			     xmToggleButtonGadgetClass, frame,
			     XmNset, val,
			     XmNindicatorOn, True,
			     NULL );
  XtAddCallback( threeDWidget->fselLighting, XmNvalueChangedCallback,
		 FeatureToggleCB, threeDWidget );

  val = tvGetBoolean (threeDWidget->threeD, TV_FEATURES_WIREFRAME);
  threeDWidget->fselWireframe =
    XtVaCreateManagedWidget( "Wireframe",
			     xmToggleButtonGadgetClass, frame,
			     XmNset, val,
			     XmNindicatorOn, True,
			     NULL );
  XtAddCallback( threeDWidget->fselWireframe, XmNvalueChangedCallback,
		 FeatureToggleCB, threeDWidget );

  XtVaCreateManagedWidget( "sep2",
			   xmSeparatorGadgetClass, frame,
			   XmNshadowThickness, 2,
#if 0
			   XmNleftAttachment, XmATTACH_FORM,
			   XmNrightAttachment, XmATTACH_FORM,
			   XmNbottomAttachment, XmATTACH_WIDGET,
			   XmNbottomWidget, dismissform,
#endif
			   XmNorientation, XmHORIZONTAL,
			   NULL );


  val = tvGetBoolean (threeDWidget->threeD, TV_FEATURES_LOAD_TEXTURES);

  threeDWidget->fselLoadTextures =
    XtVaCreateManagedWidget( "Load Textures",
			     xmToggleButtonGadgetClass, frame,
			     XmNset, val,
			     XmNindicatorOn, True,
			     NULL );
  XtAddCallback( threeDWidget->fselLoadTextures, XmNvalueChangedCallback,
		 FeatureToggleCB, threeDWidget );

  val = tvGetBoolean (threeDWidget->threeD, TV_FEATURES_BACKFACE_CULL);
  threeDWidget->fselBackfaceCull =
    XtVaCreateManagedWidget( "Backface Cull",
			     xmToggleButtonGadgetClass, frame,
			     XmNset, val,
			     XmNindicatorOn, True,
			     NULL );
  XtAddCallback( threeDWidget->fselBackfaceCull, XmNvalueChangedCallback,
		 FeatureToggleCB, threeDWidget );

  /* do the right-hand side of the selection to allow options */
  
  leftform =
    XtVaCreateManagedWidget( "leftForm",
			     xmFormWidgetClass, mainform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_WIDGET,
			     XmNrightWidget, rightform,
			     XmNbottomAttachment, XmATTACH_WIDGET,
			     XmNbottomWidget, threeDWidget->fselInfoText,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_ETCHED_IN ); n++;
  leftform = XmCreateFrame( leftform, "leftFrame", args, n );
  XtManageChild( leftform );

  threeDWidget->fselSelect =
    XtVaCreateManagedWidget( "scrolledform", 
			     xmScrolledWindowWidgetClass, leftform,
			     XmNscrollBarDisplayPolicy, XmSTATIC,
			     XmNscrollingPolicy, XmAUTOMATIC,
			     XmNwidth, 400,
			     XmNheight, 300,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     NULL );
  
  FeatureGenerateList( threeDWidget, TRUE );

}

/*
 * Synopsis:
 *   void FeatureMakeName( char *buffer, char const *name, Purl const *purl )
 *
 * Description:
 *   This is the routine that determines the string that is used to
 *   describe a feature in the Feature Selection window. This is built
 *   from the name of the feature and it's location.
 *
 *   The description is copied into the provided character buffer.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static void
FeatureMakeName( char *buffer, char const *name, Purl const *purl )
{
  switch ( purl->protocol ) {
  case tsmLocalFile:
    sprintf( buffer, "%s (localhost)", name );
    break;
  case tsmHTTP:
    sprintf( buffer, "%s (%s)", name, purl->hostName );
    break;
  default:
    strcpy( buffer, name );
    break;
  }
}

/*
 * Synopsis:
 *   int FeatureMakeGeoSetList( ThreeDWidget *, char const *, Widget, int )
 *
 * Description:
 *   Creates a label widget that gives the title of dataset, and then
 *   creates a rowcol manager that contains toggle button entries for
 *   all of the features that are associated with that dataset.
 *
 *   Implementation Note: we use a static array for the callback
 *   functions and are limited to MAX_FEATURES callback functions
 *   to load feature files. This is because if we used realloc to
 *   grow a dynamic array, then the entire array might be moved to
 *   another place in memory and so any previous callback pointers
 *   would point to the wrong place.
 *
 *   The all_off parameter is used to determine whether the toggle
 *   switches for all available datasets should be set to off (TRUE),
 *   or if they should be set to reflect whether the feature is
 *   currently in memory or not (FALSE). This distinction is required
 *   so that when we first create the Feature Select widget we set
 *   all toggles to be off by default.
 *
 * Returns: the number of feature toggle buttons that were created.
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static int
FeatureGeoSetListAddModel( ThreeDWidget *threeDWidget, TsmModel *model,
			   Widget form, int all_off )
{
  Widget      widget, modelform;
  XmString    label;
  int         j, count = 0, loaded;
  char        buffer[512];
  Purl        *purl;
  FeatureCallback *cb;

  for ( j = 0; j < model->numLocations; j++ ) {
    purl = model->location[j];
      
    modelform = XtVaCreateManagedWidget( "modelform",
					 xmFormWidgetClass, form,
					 NULL );

    /* create a name for this model instance */

    if ( model->title )
      FeatureMakeName( buffer, model->title, purl );
    else if ( model->name )
      FeatureMakeName( buffer, model->name, purl );
    else
       FeatureMakeName( buffer, "Unnamed VRML Model", purl );
    
    /* work out the state for this toggle switch - either it is */
    /* off or we work out if the model has already been loaded. */
    
    if ( all_off )
      loaded = FALSE;
    else
      loaded = ( GetFeatureFromUrl( purl->url ) != NULL );
    
    /* add a button for this model instance */
    
    label = tvXmString( buffer );
    widget = XtVaCreateWidget( model->name, xmToggleButtonGadgetClass,
			       modelform, XmNindicatorOn, True,
			       XmNset, loaded,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNlabelString, label, NULL );
    
    /* create a callback structure to record all the info we need */
    
    if ( threeDWidget->fselNumbCBs + 1 < MAX_FEATURES ) {
      cb = &threeDWidget->fselCBs[threeDWidget->fselNumbCBs];
      threeDWidget->fselNumbCBs++;
      
      cb->model = model;
      cb->curr_loc = j;
      cb->threeDWidget = threeDWidget;
      
      XtAddCallback( widget, XmNvalueChangedCallback, FeatureSelectedCB, cb);
    } else
      cb = NULL;
    
    /* manage the widget */
    
    XtManageChild( widget );
    XmStringFree( label );
    
    /* now add a push button widget to the end of the entry */
    
    label = tvXmString( "info" );
    widget = XtVaCreateManagedWidget( model->name,
				      xmPushButtonGadgetClass, modelform,
				      XmNlabelString, label,
				      XmNleftAttachment, XmATTACH_WIDGET,
				      XmNleftWidget, widget,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNbottomAttachment, XmATTACH_FORM,
				      NULL );
    XmStringFree( label );
    if ( ! loaded ) XtSetSensitive( widget, False );
    
    XtAddCallback( widget, XmNactivateCallback, FeatureInfoCB, cb );
    cb->info_widget = widget;
    
    /* increment the count of models we have added to the list */
    
    count++;
  }

  return count;
}

static int
FeatureMakeGeoSetList( ThreeDWidget *threeDWidget, char const *geoset_name,
		       Widget parent, int all_off )
{
  TsmDatabase *database;
  TsmGeoSet   *geoset = NULL;
  TsmModel    *model;
  Widget      form;
  XmString    label;
  int         i, count = 0;
  char        *name;

  /* Get the list of models for the named geoset */

  if ( ( database = GetDBase() ) == NULL ) return 0;
  if ( geoset_name ) {
    if ( ( geoset = tsmGetGeoSet( database, geoset_name ) ) == NULL ) return 0;
    if ( geoset->numModels == 0 ) return 0;
  }

  /* create a title for the models of this geoset */

  name  = ( geoset ) ? geoset->name : "Global Features";
  label = tvXmString( name );
  XtVaCreateManagedWidget( name, xmLabelGadgetClass, parent,
			   XmNlabelString, label, NULL );
  XmStringFree( label );

  count = 1;  /* we will have created at least one widget */

  /* now go through the list and create button widgets from them */

  form = XtVaCreateWidget( "inset_rowcol", xmRowColumnWidgetClass, parent,
			   NULL );

  if ( geoset ) {
    for ( i = 0; i < geoset->numModels; i++ ) {
      model = tsmGetModel( database, geoset->model_name[i] );
      if ( model != NULL )
	count += FeatureGeoSetListAddModel( threeDWidget, model, form,
					    all_off );
    }
  } else {
    for ( i = 0; i < database->numModels; i++ ) {
      model = &database->model[i];
      if ( model->geoname == NULL )
	count += FeatureGeoSetListAddModel( threeDWidget, model, form,
					    all_off );
    }
  }

  XtManageChild( form );

  return count;
}

/*
 * Synopsis:
 *   void FeatureGenerateList( ThreeDWidget *threeDWidget, int all_off )
 *
 * Description:
 *   Creates a root rowcol widget for the scrolled list window of 
 *   the Feature Selection dialog, and then creates toggle button
 *   entries for (potentially) multiple datasets.
 *
 *   If no features are available for any of the datasets, then
 *   the rowcol widget just contains a label to this effect.
 *
 *   The all_off parameter is used to determine whether the toggle
 *   switches for all available datasets should be set to off (TRUE),
 *   or if they should be set to reflect whether the feature is
 *   currently in memory or not (FALSE). This distinction is required
 *   so that when we first create the Feature Select widget we set
 *   all toggles to be off by default.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

typedef struct {
  ThreeDWidget *threeDWidget;
  Widget rowcol;
  int all_off;
  int entries;
  char const  *name;
} MakeSetListClosure;

static void
MakeSetList (TsGeoPyramid *geo, void *closure)
{
  MakeSetListClosure *setList = closure;

  if ( geo == NULL ) return;

  /* create entries for all of the models associated with this geo */
  setList->name = TsGeoName( geo );
  setList->entries +=
    FeatureMakeGeoSetList( setList->threeDWidget, setList->name,
			   setList->rowcol, setList->all_off );
}

static void
FeatureGenerateList( ThreeDWidget *threeDWidget, int all_off )
{
  char const  *name = "<none>";
  int         entries = 0;
  Widget      rowcol;
  XmString    label;

  /* clear the current list of callbacks 'cos we're gonna make new ones */

  threeDWidget->fselNumbCBs = 0;

  /* remove the previous list of models */

  if ( threeDWidget->fselRowCol ) {
    XtUnmanageChild( threeDWidget->fselRowCol );
    XtDestroyWidget( threeDWidget->fselRowCol );
  }

  /* the top widget in the scrolled window is a rowcolumn widget */
  
  rowcol = XtVaCreateWidget( "rowcol", xmRowColumnWidgetClass,
			     threeDWidget->fselSelect,
			     XmNorientation, XmVERTICAL,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     NULL );

  /* go through the list of all GeoSets for this ThreeDWidget  */
  /* and append all of the models for each geoset to our list. */

  {
    MakeSetListClosure closure;
    closure.threeDWidget = threeDWidget;
    closure.rowcol = rowcol;
    closure.all_off = all_off;
    closure.entries = entries;
    closure.name = name;

    tvForallDatasets (threeDWidget->threeD, MakeSetList, &closure);

    name = closure.name;
    entries = closure.entries;
  }

  /* now add all of the global features - those not associated */
  /* with a specific dataset, i.e. GeoVRML models              */

  entries += FeatureMakeGeoSetList( threeDWidget, NULL, rowcol, all_off );

  /* if there were no models, then tell the user this */

  if ( entries == 0 ) {
    label = tvXmString( "No feature sets available!" );
    XtVaCreateManagedWidget( name, xmLabelGadgetClass, rowcol,
			     XmNlabelString, label, NULL );
    XmStringFree( label );
  }

  XtManageChild( rowcol );
  threeDWidget->fselRowCol = rowcol;
}

/*
 * Synopsis:
 *   void FeatureSelectShow( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   This function is used to pop up the Feature Selection GUI for
 *   the specified ThreeDWidget widget. You should have called
 *   FeatureInit() first.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

void
FeatureSelectShow( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget->fselWidget == NULL ) FeatureSelectInit( threeDWidget );
  if ( threeDWidget->fselWidget == NULL ) return;

  if ( ! XtIsManaged( threeDWidget->fselWidget ) )
    XtManageChild( threeDWidget->fselWidget );
}

/*
 * Synopsis:
 *   void FeatureSelectHide( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   This function is used to pop down the Feature Selection GUI for
 *   the specified ThreeDWidget widget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

void
FeatureSelectHide( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget == NULL || threeDWidget->fselWidget == NULL ) return;
  if ( XtIsManaged( threeDWidget->fselWidget ) )
    XtUnmanageChild( threeDWidget->fselWidget );
}

/*
 * Synopsis:
 *   void FeatureSelectRefresh( ThreeDWidget *threeDWidget )
 *
 * Description:
 *   Refreshes the list of all available models for this viewer.
 *   This might change if we have loaded or unloaded a geo, i.e.
 *   this should be called by ThreeDWidgetAddDataSet().
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 May, 1999
 *
 */

void
FeatureSelectRefresh( ThreeDWidget *threeDWidget )
{
  if ( threeDWidget != NULL && threeDWidget->fselWidget != NULL )
    FeatureGenerateList( threeDWidget, FALSE );
}

/*
 * Synopsis:
 *   void FeatureSelectedCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on one of the
 *   toggle widgets for loading/unloading a specific model file. It is
 *   passed a FeatureCallback structure that details the URL of the
 *   model to load as well as the ThreeDWidget structure that should
 *   be updated.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static void
FeatureSelectedCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  FeatureCallback *cb = (FeatureCallback *) client_data;
  Purl            *purl;
  int             state = ((XmToggleButtonCallbackStruct *) call_data)->set;
  int             id;
  FeatureItem     *item = NULL;

  if ( cb == NULL || cb->model == NULL ) return;

  /* get the URL for the VRML file to (un)load */

  purl = cb->model->location[cb->curr_loc];

  /* now, do we want to load it or unload it? */

  if ( state ) {
    id = FeatureGet( cb->threeDWidget, purl->url );
    if ( ( item = FeatureGetItem( id ) ) != NULL ) {
      item->name = tsmMemStrdup( cb->model->name );
      FeatureAddToWidget( cb->threeDWidget->threeD, id );
      if ( cb->info_widget ) XtSetSensitive( cb->info_widget, True );

      /* we haven't loaded the metadata node yet so this won't display much */
      /* FeatureInfoCB( w, cb, call_data ); */

    }
  } else {
    FeatureInfoCB( w, cb, NULL );
    id = FeatureUnload( cb->threeDWidget->threeD, purl->url );
    FeatureRemoveFromWidget( cb->threeDWidget->threeD, id );
    if ( cb->info_widget ) XtSetSensitive( cb->info_widget, False );
  }

  /* refresh the screen. Note that this is only done here to refresh */
  /* features that are already in memory. Features that were loaded  */
  /* by this callback won't be in memory yet, but the FeatureLoadCB  */
  /* will post a redraw for those when they are done.                */

  tvBasePostRedraw( cb->threeDWidget->threeD, FALSE );
}

/*
 * Synopsis:
 *   void FeatureInfoCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on the "info"
 *   widget for a particular model. It will update the text field
 *   at the bottom of the dialog with some text about the model.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 May, 1999
 *
 */

static void
FeatureInfoCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  FeatureCallback *cb = (FeatureCallback *) client_data;
  Purl            *purl;
  FeatureItem     *item = NULL;
  char            *buffer, *info = NULL;
  int             size;

  if ( cb == NULL || cb->model == NULL ) return;

  /* passing a NULL call_data will clear the currently */
  /* displayed text in the scrolled info widget.       */

  if ( call_data == NULL ) {
    XtVaSetValues( cb->threeDWidget->fselInfoText, XmNvalue, "", NULL );
    return;
  }

  /* get the URL for the VRML file to (un)load */

  purl = cb->model->location[cb->curr_loc];

  /* get the FeatureItem structure for this model */

  if ( ( item = GetFeatureFromUrl( purl->url ) ) == NULL )
    return;
  
#if 0
  for ( i = list_first( feature_list ); ! list_done( i ); i = list_next( i ) ){
    item = (FeatureItem *) list_value( i );
    if ( item == NULL || item->name == NULL ) continue;
    if ( strcmp( item->name, cb->model->name ) == 0 ) break;
  }

  if ( item == NULL ) return;
#endif

  /* work out a buffer size so that we never cause any overflow */
  /* the description field could be arbitrarily long.           */

  size = 1024;
  if ( item->author ) size = max( size, strlen( item->author ) + 128 );
  if ( item->description ) size = max( size, strlen( item->description )+128 );

  if ( ( buffer = (char *) tsmMemMalloc( size ) ) == NULL )
    return;

  /* build the info text for this model */

  if ( cb->model->title ) {
    sprintf( buffer, "Name: %s\n", cb->model->title );
    AppendString( &info, buffer );
  } else if ( item->name ) {
    sprintf( buffer, "Name: %s\n", item->name );
    AppendString( &info, buffer );
  }

  if ( purl->protocol == tsmLocalFile )
    sprintf( buffer, "URL: %s\n", purl->uri );
  else
    sprintf( buffer, "URL: %s\n", purl->url );
  AppendString( &info, buffer );

  sprintf( buffer, "Textures: %d\n", item->textures );
  AppendString( &info, buffer );

  sprintf( buffer, "Hyperlinks: %d", item->anchors );
  AppendString( &info, buffer );

  if ( item->author ) {
    sprintf( buffer, "\nAuthor: %s", item->author );
    AppendString( &info, buffer );
  }

  if ( item->copyright ) {
    sprintf( buffer, "\nCopyright: %s", item->copyright );
    AppendString( &info, buffer );
  }

  if ( item->description ) {
    int  col;
    char *inbuf, *outbuf;

    strcpy( buffer, "\nDescription: " );

    col    = strlen( buffer );
    inbuf  = buffer + col;
    outbuf = item->description;

    /* copy the description string to buffer, but word wrap */
    /* all text so that we enforce a set right margin.      */
    
    while ( *outbuf != '\0' ) {
      if ( isspace( *outbuf ) ) {
	
	/* for one or more whitespace, we either word wrap  */
        /* (+indent) or output a single space to the buffer */

	if ( col++ > 50 ) {
	  *inbuf++ = '\n';
	  *inbuf++ = ' ';
	  *inbuf++ = ' ';
	  col = 2;
	} else
	  *inbuf++ = *outbuf++;
	while ( isspace( *outbuf ) && *outbuf != '\0' )
	  outbuf++;

      } else {

	/* for non-whitespace, we just copy it to the buffer */

	*inbuf++ = *outbuf++;
	col++;
      }
      
    }
    *inbuf = '\0';

    AppendString( &info, buffer );
  }

  /* set the text contents of the scrolled text widget */

  if ( info ) {
    XtVaSetValues( cb->threeDWidget->fselInfoText, XmNvalue, info, NULL );
    tsmMemFree( info );
  }
  tsmMemFree( buffer );
}

/*
 * Synopsis:
 *   void FeatureToggleCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function is called whenever the user clicks over one of
 *   the global state toggle buttons, i.e. Wireframe, Lighting, or
 *   Textured. It then sets toggle's the ThreeDWidget's state for
 *   the appropriate characteristic. These are used by the render
 *   callback to determine how to render all of the features for
 *   this ThreeDWidget.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static void
FeatureToggleCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  tvBase *threeD = threeDWidget->threeD;
  int state = XmToggleButtonGadgetGetState( w );

  /* set the new display characteristics */

  if ( w == threeDWidget->fselWireframe ) {
    tvSetBoolean (threeD, TV_FEATURES_WIREFRAME, state);
  } else if ( w == threeDWidget->fselTextured ) {
    tvSetBoolean (threeD, TV_FEATURES_TEXTURED, state);
  } else if ( w == threeDWidget->fselLighting ) {
    tvSetBoolean (threeD, TV_FEATURES_LIGHTING, state);
  } else if ( w == threeDWidget->fselLoadTextures ) {
    tvSetBoolean (threeD, TV_FEATURES_LOAD_TEXTURES, state);
  } else if ( w == threeDWidget->fselBackfaceCull ) {
    tvSetBoolean (threeD, TV_FEATURES_BACKFACE_CULL, state);
  }
}

/*
 * Synopsis:
 *   void FeatureSelectDismissCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   The callback that is invoked when the user clicks on the Dismiss
 *   button of the Feature Selection dialog. This closes the dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 9 April, 1999
 *
 */

static void
FeatureSelectDismissCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  if ( threeDWidget == NULL || threeDWidget->fselWidget == NULL ) return;

  if ( XtIsManaged( threeDWidget->fselWidget ) )
    XtUnmanageChild( threeDWidget->fselWidget );
}
