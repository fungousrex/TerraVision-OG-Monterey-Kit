/*
 * Name: Database.c
 *
 * Description: 
 *     Controls a series of dialogs to all the user to select datasets
 *     from a global database, manage the contents of the database, and
 *     control various advanced options of the selected datasets.
 *
 * Function List:
 *   void DBaseInit( Widget parent )
 *   void DBaseInitOpenTV( Widget parent )
 *   void DBaseShowOpenTV( void )
 *   void DBaseHideOpenTV( void )
 *   void TerraVisionLoadDataset( char *geo_name, char *oi_url, char *dem_url,
 *                                TsmConnectionParams, TsmConnectionParams )
 *   void DBaseSelectTV( void )
 *   void DBaseNewDBase( void )
 *   void DBaseCreateNewDBase( Widget w, XtPointer client_data, XtPointer )
 *   void DBaseCallbackTV( Widget w, XtPointer client_data, XtPointer )
 *   void DBaseMakeUrlsServers( TsmDataSet *, XmString *, int *, XmString *,
 *                              int *, int, TsmDataSetInst ** )
 *   void DBaseRefreshTV( int refresh )
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:26:46 $
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
#include "Dialogs.h"
#include "ThreeDCore.h"

#include <prio.h>

#include "X11Include.h"

#include <tvutils/tvutils.h>

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>
#include <Xm/SeparatoG.h>

TVDatabase tv_database;

static void DBaseInitOpenTV( Widget parent );
static void DBaseHideOpenTV( void );
static void DBaseShowAdvancedCB( Widget w, XtPointer client_data, XtPointer );
static void DBaseCallbackTV( Widget w, XtPointer client_data, XtPointer );
static void DBaseSelectTV( void );
static void DBaseNewDBase( void );

/* a macro wrapper to the generic error report dialog in Dialogs.c */

static Widget db_error = NULL;
static Widget db_confirm = NULL;

#define DBaseError(x) GenericErrorDialog( tv_database.tv_dialog, &db_error, x )


/*
 * Synopsis:
 *   void DBaseInit( Widget parent )
 *
 * Description:
 *   This functions performs a number if important initialisations
 *   for the global database interface. This function must be called
 *   before you make any calls to open any of the Database windows.
 *   It initialses various global settings and sets up the widgets
 *   for the GUI.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseInit( Widget parent )
{
  char *dbase_url = tvuGetString( TVU_DATABASE_URL );

  /* initialise the TVDatabase structure */

  tv_database.location = NULL;
  tv_database.database = NULL;

  tv_database.tv_curr_geo = -1;
  tv_database.db_selected = -1;

  tsmInitParams( &tv_database.oi_params );
  tsmInitParams( &tv_database.dem_params );

  /* If the user has a database_url in their .TerraVision */
  /* file, then let's load that for them.                 */

  if ( dbase_url != NULL ) {
    tv_database.location = tsmMemStrdup( dbase_url ) ;
    tv_database.database = tsmReadDatabase( tv_database.location );
  }

  /* initialise the widgets for all of the windows */

  DBaseInitOpenTV( parent );    /* Open TV */
  DBaseInitOptions( parent );   /* Database options */
  DBaseInitAdvanced( parent );  /* The OI and DEM advanced options */

  /* Refresh the contents of the dialog windows */

  DBaseRefreshTV( DBASE_REFRESH_ALL );

}

/*
 * Synopsis:
 *   void DBaseInitOpenTV( Widget parent )
 *
 * Description:
 *   This function sets up all of the widgets for the Open TV 
 *   menu option. This facility lets the user select an OI and
 *   DEM from those available in the global database. 
 *
 *   This function is called by DBaseInit() above, and should
 *   not be called directly.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseInitOpenTV( Widget parent )
{
  char     *title = "Open TerraVision Dataset";
  Widget   separator1, separator2;
  Widget   dialog, mainForm, selectForm, actionForm;
  Arg      args[15];
  int      n = 0;
  XmString name;

  /* Create the main dialog window for the dataset selection */

  name = tvXmString(title);
  XtSetArg( args[n], XmNdialogTitle, name ); n++;
  XtSetArg( args[n], XmNwidth, 640 ); n++;
  XtSetArg( args[n], XmNheight, 480 ); n++;
  tv_database.tv_dialog = dialog =
    XmCreateFormDialog( parent, title, args, n );
  XmStringFree (name);

  /* At the very top of the window we have a text widget with */
  /* the location of the global database, and then a button   */
  /* beside this to access the database "Options" window.     */
  
  mainForm = XtVaCreateManagedWidget( "mainForm", 
				      xmFormWidgetClass, dialog,
				      XmNtopAttachment, XmATTACH_FORM,
				      XmNleftAttachment, XmATTACH_FORM,
				      XmNrightAttachment, XmATTACH_FORM,
				      XmNhorizontalSpacing, 10,
				      XmNverticalSpacing, 10,
				      NULL );

  {
    Widget label;

    name = tvXmString("Database :");
    label = XtVaCreateManagedWidget( "labelWidget",
				     xmLabelGadgetClass, mainForm,
				     XmNresizable, False,
				     XmNlabelString, name,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     NULL );
    XmStringFree (name);

    name = tvXmString("Options...");
    tv_database.tv_opts =
      XtVaCreateManagedWidget( "DatabaseOptions", 
			       xmPushButtonGadgetClass, mainForm,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNlabelString, name,
			       NULL );
    XmStringFree (name);
    XtAddCallback( tv_database.tv_opts, XmNactivateCallback,
		   (XtCallbackProc) DBaseShowOptions, NULL );

    tv_database.tv_dbase =
      XtVaCreateManagedWidget( "text",
			       xmTextWidgetClass, mainForm,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, label,
			       XmNrightAttachment, XmATTACH_WIDGET,
			       XmNrightWidget, tv_database.tv_opts,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNtraversalOn, True,
			       NULL );
    XtAddCallback( tv_database.tv_dbase, XmNactivateCallback,
		   (XtCallbackProc) DBaseNewDBase, NULL );
  }

  /* Separator between top box and central area. */
  separator1 = XtVaCreateManagedWidget( "separator1", 
					xmSeparatorGadgetClass, dialog,
					XmNtopAttachment, XmATTACH_WIDGET,
					XmNtopWidget, mainForm,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL );

  /* Make the initial focus go to the Database filename widget */

  XtVaSetValues( mainForm, XmNinitialFocus, tv_database.tv_dbase, NULL );

  /* Now we get down to the main part of the window, which lets */
  /* you select different GeoSets and then the locations and    */
  /* options for the DEM and OI pyramids.                       */

  selectForm = XtVaCreateManagedWidget( "selectForm",
					xmFormWidgetClass,
					dialog,
					XmNtopAttachment, XmATTACH_WIDGET,
					XmNtopWidget, separator1,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNfractionBase, 4,
					NULL );

  {
    Widget geoSetForm, form1, form2;
    Widget label;

    geoSetForm = XtVaCreateManagedWidget( "geoSetForm", 
					  xmFormWidgetClass, selectForm,
					  XmNtopAttachment, XmATTACH_FORM,
					  XmNleftAttachment, XmATTACH_FORM,
					  XmNrightAttachment, XmATTACH_POSITION,
					  XmNrightPosition, 1,
					  XmNbottomAttachment, XmATTACH_FORM,
					  XmNhorizontalSpacing, 10,
					  XmNverticalSpacing, 10,
					  NULL );

    name = tvXmString("GeoSets");
    label = XtVaCreateManagedWidget( "labelWidget",
				     xmLabelGadgetClass, geoSetForm,
				     XmNlabelString, name,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopOffset, 10,
				     NULL );
    XmStringFree (name);

    n=0;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, label ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++; 
    XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;

    tv_database.tv_geosets = XmCreateScrolledList( geoSetForm, "GeoSets",
						   args, n );
    XtAddCallback( tv_database.tv_geosets, XmNbrowseSelectionCallback,
		   DBaseCallbackTV, NULL );
    XtAddCallback( tv_database.tv_geosets, XmNdefaultActionCallback,
		   (XtCallbackProc) DBaseSelectTV, NULL );

    XtManageChild( tv_database.tv_geosets );

    /* Now the location scroll lists for the OI and DEM */

    form1 = XtVaCreateManagedWidget( "form1",
				     xmFormWidgetClass, selectForm,
				     XmNleftAttachment, XmATTACH_POSITION,
				     XmNleftPosition, 1,
				     XmNrightAttachment, XmATTACH_POSITION,
				     XmNrightPosition, 3,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNhorizontalSpacing, 10,
				     XmNverticalSpacing, 10,
				     XmNleftOffset, -10,
				     XmNrightOffset, -10,
				     XmNfractionBase, 2,
				     NULL );

    /* Contents of form1 */
    {
      name = tvXmString("Image Location");
      label = XtVaCreateManagedWidget( "labelWidget",
				       xmLabelGadgetClass, form1,
				       XmNlabelString, name,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNtopOffset, 10,
				       NULL );
      XmStringFree (name);

      n=0;
      XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNtopWidget, label ); n++;
      XtSetArg( args[n], XmNbottomAttachment, XmATTACH_POSITION ); n++;
      XtSetArg( args[n], XmNbottomPosition, 1 ); n++;
      XtSetArg( args[n], XmNbottomOffset, 5 ); n++;
      XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++; 
      XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;

      tv_database.tv_oiloc = XmCreateScrolledList( form1, "OILocation", args, n );
      XtAddCallback( tv_database.tv_oiloc, XmNbrowseSelectionCallback,
		     DBaseCallbackTV, NULL );
      XtManageChild( tv_database.tv_oiloc );

      name = tvXmString("DEM Location");
      label = XtVaCreateManagedWidget( "labelWidget",
				       xmLabelGadgetClass, form1,
				       XmNlabelString, name,
				       XmNtopAttachment, XmATTACH_POSITION,
				       XmNtopPosition, 1,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNtopOffset, 5,
				       NULL );
      XmStringFree (name);

      n=0;
      XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNtopWidget, label ); n++;
      XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;
      XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;

      tv_database.tv_demloc = XmCreateScrolledList( form1, "DEMLocation", args, n );
      XtAddCallback( tv_database.tv_demloc, XmNbrowseSelectionCallback,
		     DBaseCallbackTV, NULL );
      XtManageChild( tv_database.tv_demloc );
    }

    /* And at the edge of the window, the server lists for the OI and */
    /* DEM. Along with an "Advanced..." button for each of these.     */

    form2 = XtVaCreateManagedWidget( "form2", 
				     xmFormWidgetClass, selectForm,
				     XmNleftAttachment, XmATTACH_POSITION,
				     XmNleftPosition, 3,
				     XmNtopAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNhorizontalSpacing, 10,
				     XmNverticalSpacing, 10,
				     XmNfractionBase, 2,
				     NULL );

    /* Contents of form2 */
    {
      name = tvXmString("Image Servers");
      label = XtVaCreateManagedWidget( "labelWidget",
				       xmLabelGadgetClass, form2,
				       XmNlabelString, name,
				       XmNtopAttachment, XmATTACH_FORM,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNtopOffset, 10,
				       NULL );
      XmStringFree (name);
      
      name = tvXmString("Advanced...");
      tv_database.tv_oiadv =
	XtVaCreateManagedWidget( "OIAdvanced", 
				 xmPushButtonGadgetClass, form2,
				 XmNbottomAttachment, XmATTACH_POSITION,
				 XmNbottomPosition, 1,
				 XmNbottomOffset, 5,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNlabelString, name,
				 NULL );
      XmStringFree (name);

      XtAddCallback( tv_database.tv_oiadv, XmNactivateCallback,
		     DBaseShowAdvancedCB, (XtPointer) TSM_IMAGE_DATASET );
  
      n=0;
      XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNtopWidget, label ); n++;
      XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNbottomWidget, tv_database.tv_oiadv ); n++;
      XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNbottomOffset, 5 ); n++;
      XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++; 
      XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;

      tv_database.tv_oiserv = XmCreateScrolledList( form2, "OIOpts", args, n );
      XtAddCallback( tv_database.tv_oiserv, XmNbrowseSelectionCallback,
		     DBaseCallbackTV, NULL );
      XtManageChild( tv_database.tv_oiserv );

      name = tvXmString("DEM Servers");
      label = XtVaCreateManagedWidget( "labelWidget",
				       xmLabelGadgetClass, form2,
				       XmNlabelString, name,
				       XmNtopAttachment, XmATTACH_POSITION,
				       XmNtopPosition, 1,
				       XmNtopOffset, 5,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       NULL );
      XmStringFree (name);

      name = tvXmString("Advanced...");
      tv_database.tv_demadv =
	XtVaCreateManagedWidget( "DEMAdvanced", 
				 xmPushButtonGadgetClass, form2,
				 XmNlabelString, name,
				 XmNbottomAttachment, XmATTACH_POSITION,
				 XmNbottomPosition, 2,
				 XmNbottomOffset, 10,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 NULL );
      XmStringFree (name);
      XtAddCallback( tv_database.tv_demadv, XmNactivateCallback,
		     DBaseShowAdvancedCB, (XtPointer) TSM_DEM_DATASET );

      n=0;
      XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNtopWidget, label ); n++;
      XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
      XtSetArg( args[n], XmNbottomWidget, tv_database.tv_demadv ); n++;
      XtSetArg( args[n], XmNbottomOffset, 5 ); n++;
      XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
      XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++; 
      XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;

      tv_database.tv_demserv = XmCreateScrolledList( form2, "DEMOpts", args, n );
      XtAddCallback( tv_database.tv_demserv, XmNbrowseSelectionCallback,
		     DBaseCallbackTV, NULL );
      XtManageChild( tv_database.tv_demserv );
    }
  }

  /* Finally, we stick a separator bar in and then put the */
  /* Okay and Cancel widgets at the bottom of the window.  */

  separator2 = XtVaCreateManagedWidget( "separator2", 
					xmSeparatorGadgetClass, dialog,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNbottomOffset, 10,
					NULL );

  /* This is done in this order so that selectForm is the stretchy one. */
  XtVaSetValues (selectForm, 
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, separator2,
		 NULL);

  actionForm = XtVaCreateManagedWidget( "actionForm", 
					xmFormWidgetClass, dialog,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					NULL );

  XtVaSetValues (separator2, 
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, actionForm,
		 NULL);
  
  name = tvXmString("Open Dataset");
  tv_database.tv_okay =
    XtVaCreateManagedWidget( "tsmStartWidgetStart", 
			     xmPushButtonGadgetClass, actionForm,
			     XmNleftOffset, 15,
			     XmNtopOffset, 10,
			     XmNbottomOffset, 10,
			     XmNmarginTop, 3,
			     XmNmarginBottom, 3,
			     XmNmarginLeft, 5,
			     XmNmarginRight, 5,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);
  XtAddCallback( tv_database.tv_okay, XmNactivateCallback,
		 (XtCallbackProc) DBaseSelectTV, NULL );

  name = tvXmString("Cancel");
  tv_database.tv_cancel =
    XtVaCreateManagedWidget( "tsmStartWidgetCancel", 
			     xmPushButtonGadgetClass, actionForm,
			     XmNrightOffset, 15,
			     XmNtopOffset, 10,
			     XmNbottomOffset, 10,
			     XmNmarginTop, 3,
			     XmNmarginBottom, 3,
			     XmNmarginLeft, 5,
			     XmNmarginRight, 5,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment,XmATTACH_FORM,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);
  XtAddCallback( tv_database.tv_cancel, XmNactivateCallback,
		 (XtCallbackProc) DBaseHideOpenTV, NULL );

}

/*
 * Synopsis:
 *   void DBaseGiveHelpIfNoDatabase( void )
 *
 * Description:
 *   A helpful little message on what to do if you don't have
 *   a database when you first run TerraVision. Gee, sometimes
 *   I'm just too darn nice!
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
DBaseGiveHelpIfNoDatabase( void )
{
  static int help_given = FALSE;

  if ( tv_database.database == NULL && help_given == FALSE ) {
    DBaseError( "You do not have a valid database file specified.\n\n"
                "You can create a new database by typing a\n" 
		"a filename into the text widget and pressing\n"
		"Return. A suggested database filename is\n"
                "~/.TerraVisionDB (entering an empty string\n"
                "will default to this filename.)\n\n"
		"Once the new database has been created, you will\n"
		"be given the Database Options window to enter\n"
		"in the URLs of one or more tileset servers,\n"
                "e.g. ~magic/TileSets\n\n"
                "Once you have specifed your server list, click\n"
		"on Re-Read All to read all of these servers\n\n"
                "Future invocations of TerraVision will use the\n"
                "last database file that you specified" );
    help_given = TRUE;
  }

}

/*
 * Synopsis:
 *   void DBaseShowOpenTV( void )
 *
 * Description:
 *   This function will display the Open TV window, if it is not
 *   already displayed.
 *
 *   You should have called DBaseInit() before getting anywhere
 *   near this function.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseShowOpenTV( void )
{
  if ( tv_database.tv_dialog == NULL ) return;

  if ( ! XtIsManaged( tv_database.tv_dialog ) )
    XtManageChild( tv_database.tv_dialog );

  DBaseGiveHelpIfNoDatabase();
}

/*
 * Synopsis:
 *   void DBaseHideOpenTV( void )
 *
 * Description:
 *   This function will dismiss the Open TV window if it is currently
 *   being displayed on the screen. This is used by the callback for
 *   the cancel button.
 *
 *   You should have called DBaseInit() before getting anywhere
 *   near this function.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseHideOpenTV( void )
{
  if ( tv_database.tv_dialog == NULL ) return;

  if ( XtIsManaged( tv_database.tv_dialog ) )
    XtUnmanageChild( tv_database.tv_dialog );

  DBaseHideOptions();
  DBaseHideAdvanced();
}

/*
 * Synopsis:
 *   void DBaseSelectTV( void )
 *
 * Description:
 *   This is the callback that is made whenever the user clicks on
 *   the Open Dataset button of the Open TV window. It will work out
 *   what the currently selected OI and DEM are, and then load this
 *   dataset. Checks are made to ensure that the user did in fact
 *   select something, and that the database does actually contain
 *   both an OI and DEM for this particular GeoSet.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseSelectTV( void )
{
  /*static const char funcName[] = "DBaseSelectTV";*/
  TsmDataSetInst *oiInst, *demInst;
  char           *geoName;
  list_iter_t    i;

  if ( tv_database.tv_curr_geo >= 0 ) {
    oiInst  = tv_database.curr_oi_inst[tv_database.tv_curr_oi_serv];
    demInst = tv_database.curr_dem_inst[tv_database.tv_curr_dem_serv];
    geoName = tv_database.database->geoset[tv_database.tv_curr_geo].name;

    /* check to see if this dataset has already been loaded */

    for ( i = list_first( geoList ); ! list_done( i ); i = list_next( i ) ) {
      TsGeoPyramid *geo = list_value( i );
      if ( geo && strcmp( geoName, TsGeoName( geo ) ) == 0 ) {
	DBaseError( "That dataset has already been opened!" );
	return;
      }
    }

    if ( oiInst != NULL && demInst != NULL ) {

      /* Set the set ID connection parameter - this is ignored */
      /* for anything other than DPSS datasets anyway.         */

      tsmSetParam( tv_database.oi_params, TSM_PARAM_SETID, oiInst->set_id );
      tsmSetParam( tv_database.dem_params, TSM_PARAM_SETID, demInst->set_id );

      /* Load the specified OI and DEM dataset */

      DBaseHideOpenTV();

      TerraVisionLoadDataset( geoName,
			      oiInst->location->url, demInst->location->url,
			      tv_database.oi_params, tv_database.dem_params );

    } else
      DBaseError( "No data for geoset!" );

  } else
    DBaseError( "No dataset selected!" );
}

/*
 * Synopsis:
 *   void DBaseCreateNewDBase( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is made whenever the user selects that they would
 *   like to create a new database. This is done by entering a filename
 *   into the database text widget that does not exist.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
DBaseCreateNewDBase( Widget w, XtPointer client_data, XtPointer call_data )
{
  tv_database.database = tsmMakeDatabase();
  tv_database.database->location = tsmParseUrl( tv_database.location );

  if ( tsmWriteDatabase( tv_database.database, NULL ) == TRUE )
    DBaseShowOptions();
  else
    DBaseError( "Couldn't create new database!\nPlease check filename." );
}

/*
 * Synopsis:
 *   void DBaseNewDBase( void )
 *
 * Description:
 *   This is the callback that is made whenever the user presses
 *   return after entering a new database filename in the Database
 *   text widget. It will unload the current database and then
 *   try to load the new database (presenting an error message if
 *   this operation failed). Also, we store any new database url
 *   in the TerraVision config file.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseNewDBase( void )
{
  char        *new_name;
  Purl        *purl;
  PRFileInfo  filetest;

  /* Free the memory for the current database and it's filename */

  if ( tv_database.database != NULL )
    tsmFreeDatabase( tv_database.database );

  if ( tv_database.location != NULL )
    tsmMemFree( tv_database.location );

  /* Grab the new database filename and put into a canonical form */

  XtVaGetValues( tv_database.tv_dbase, XmNvalue, &new_name, NULL );
  if ( new_name == NULL || new_name[0] == '\0' ) new_name = "~/.TerraVisionDB";
  
  if ( ( purl = tsmParseUrl( new_name ) ) != NULL ) {
    tv_database.location = tsmMemStrdup( purl->uri );
    tsmFreePurl( purl );
  } else
    tv_database.location = tsmMemStrdup( new_name );

  /* Store the filename in the TerraVisio configuration file */

  tvuAddConfig1( "database", tv_database.location );
  tvuSaveConfig();

  /* Update the status fields of the main GUI */
	
  TerraVisionRefreshStatusFields( FALSE );

  /* Check to see if the database file exists first */

  if ( PR_GetFileInfo ( tv_database.location, &filetest ) == PR_FAILURE ) {
    char message[512];

    /* File doesn't exist - does the user want to create it? */

    tv_database.database    = NULL;
    tv_database.tv_curr_geo = -1;
    tv_database.db_selected = -1;
    
    DBaseRefreshTV( DBASE_REFRESH_ALL );
    DBaseRefreshOpts();

    sprintf( message, "Database file does not exist:\n%s\n"
	     "Do you want to create a new database?", tv_database.location );

    GenericConfirmDialog( tv_database.tv_dialog, &db_confirm, message,
			  DBaseCreateNewDBase, NULL );

    return;
  }

  /* Load the new database and reset the current selection to none */

  tv_database.database    = tsmReadDatabase( tv_database.location );
  tv_database.tv_curr_geo = -1;
  tv_database.db_selected = -1;

  /* Refresh any GUI widgets */

  DBaseRefreshTV( DBASE_REFRESH_ALL );
  DBaseRefreshOpts();

  /* If the database could not be loaded, then tell the user */

  if ( tv_database.database == NULL )
    DBaseError( "Database file appears to be corrupt!" );

}

/*
 * Synopsis:
 *   void DBaseShowAdvancedCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user clicks on either of the
 *   Advanced buttons below the DEM and OI server lists. It will then
 *   work out if the button was for the DEM or the OI and open the
 *   approriate options window accordingly.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseShowAdvancedCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  switch ( (TsmDataSetType) client_data ) {
  case TSM_IMAGE_DATASET:
    DBaseShowAdvanced( TSM_IMAGE_DATASET );
    break;
  case TSM_DEM_DATASET:
    DBaseShowAdvanced( TSM_DEM_DATASET );
    break;
  default:
    puts( "This should not happen!" );
  }
}

/*
 * Synopsis:
 *   void DBaseCallbackTV( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is used for all of the dataset/geoset selection 
 *   events, i.e. whenever the user clicks over a new geoset, or
 *   new dataset instance, or new server combination. All we need
 *   to do is record the new position and then refresh the window.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseCallbackTV( Widget w, XtPointer client_data, XtPointer call_data )
{
  XmListCallbackStruct *list_cbs = (XmListCallbackStruct *) call_data;

  if ( w == tv_database.tv_geosets ) {

    /* If we select a new GeoSet, then refresh the OI/DEM displays */

    tv_database.tv_curr_geo = list_cbs->item_position - 1;
    tv_database.tv_curr_oi = 0;
    tv_database.tv_curr_dem = 0;
    tv_database.tv_curr_oi_serv = 0;
    tv_database.tv_curr_dem_serv = 0;
    DBaseRefreshTV( DBASE_REFRESH_ALL_OI | DBASE_REFRESH_ALL_DEM );

  } else if ( w == tv_database.tv_oiloc ) {

    /* If we select a new OI location, then update the server list */

    tv_database.tv_curr_oi = list_cbs->item_position - 1;
    tv_database.tv_curr_oi_serv = 0;
    DBaseRefreshTV( DBASE_REFRESH_OI_SERVERS );

  } else if ( w == tv_database.tv_demloc ) {

    /* If we select a new DEM location, then update the server list */

    tv_database.tv_curr_dem = list_cbs->item_position - 1;
    DBaseRefreshTV( DBASE_REFRESH_DEM_SERVERS );

  } else if ( w == tv_database.tv_oiserv ) {

    /* If we select a new OI server, then record this - no refresh */

    tv_database.tv_curr_oi_serv = list_cbs->item_position - 1;

  } else if ( w == tv_database.tv_demserv ) {

    /* If we select a new OI server, then record this - no refresh */

    tv_database.tv_curr_dem_serv = list_cbs->item_position - 1;

  } else
    puts( "Callback: Other" );
}

/*
 * Synopsis:
 *   void DBaseMakeUrlsServers( TsmDataSet *, XmString *, int *, XmString *,
 *                              int *, int, TsmDataSetInst ** )
 *
 * Description:
 *   This function is used to build both the DEM and the OI list of
 *   instances and servers. This is slightly complicated by the fact
 *   that different DPSS sets are stored as separate instances in the
 *   database, but really we just want to see one instance in the
 *   window and then see the various set ids in the server combination
 *   list.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseMakeUrlsServers( TsmDataSet *dataset, XmString *urls, int *numUrls,
		      XmString *servers, int *numServers, int selected,
		      TsmDataSetInst **inst_array )
{
  int            n, i, prevpos;
  TsmDataSetInst *instance;
  char           buffer[100];
  Purl           *purl;

  *numUrls = *numServers = 0;

  /* check for the (really bad!) case where we don't have a dataset */

  if ( dataset == NULL ) {
    inst_array[0] = NULL;
    return;
  }

  /* Go through each of the instances for this dataset */
  /* and add any unique ones to the the urls array.    */ 

  for ( n = 0; n < dataset->numInstances; ++n ) {
    prevpos  = -1;
    instance = &dataset->instance[n];
    purl     = instance->location;

    /* Is this url unique, or have we seen it before. */
    /* prevpos == position of previous occurrence.    */
    
    for ( i = 0; i < n; ++i ) {
      if ( strcmp( purl->url, dataset->instance[i].location->url ) == 0 ) {
	prevpos = i;
	break;
      }
    }

    /* If this is one of the selected instances, then  */
    /* add the server information for this instance to */
    /* the server list for the dataset. This server    */
    /* info is protocol dependent.                     */
    
    if ( ( *numUrls == selected && prevpos < 0 ) ||
	 ( prevpos >= 0 && prevpos == selected ) ) {
    
      switch ( purl->protocol ) {
      case tsmLocalFile:
	strcpy( buffer, "localhost" );
	break;
      case tsmISS:
      case tsmDSM:
      case tsmDPSS:
	sprintf( buffer, "%d:", instance->set_id );
	for ( i = 0; i < instance->numServers; ++i ) {
	  strcat( buffer, " " );
	  strcat( buffer, instance->serverName[i] );
	}
	break;
      default:
	strcpy( buffer, "default" );
      }
      servers[(*numServers)] = tvXmString( buffer );
      inst_array[(*numServers)++] = instance;
    }

    /* Finally, if we have a unique url, then add it to */
    /* the list of URLs for this dataset.               */
      
    if ( prevpos == -1 )
      urls[(*numUrls)++] = tvXmString( purl->url );
      
  }
}

/*
 * Synopsis:
 *   void DBaseRefreshTV( int refresh )
 *
 * Description:
 *   Does all of the internal structure and widget refreshing for
 *   the Open TV window. You can pass in a refresh mask to specify
 *   which widgets you want to refresh, or DBASE_REFRESH_ALL for 
 *   all of them.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseRefreshTV( int refresh )
{
  char           *filename;
  XmString       item[100], server[100];
  TsmGeoSet      *geoset;
  TsmDataSet     *dataset;
  int            n, urls, servers;

  /* Fill in the database location text widget if we've been asked to */

  if ( refresh & DBASE_REFRESH_FILENAME ) {
    filename = ( tv_database.location ) ? tv_database.location : "";
    XtVaSetValues( tv_database.tv_dbase, XmNvalue, filename, NULL );
  }

  /* Fill in the GeoSets list */
  
  n = 0;
  if ( tv_database.database != NULL ) {
    for ( n = 0; n < tv_database.database->numGeoSets; ++n ) {
      item[n] = tvXmString( tv_database.database->geoset[n].name );
    }
  }

  /* Now refresh the GeoSet list widget if we have been told to */

  if ( refresh & DBASE_REFRESH_GEOSETS ) {
    XtVaSetValues( tv_database.tv_geosets, XmNitems, item,
		   XmNitemCount, n, NULL );

    if ( tv_database.tv_curr_geo >= 0 )
      XmListSelectPos( tv_database.tv_geosets, tv_database.tv_curr_geo + 1,
		       False );
    else
      XtVaSetValues( tv_database.tv_geosets, XmNselectedItemCount, 0, NULL );
  }

  /* Free the list of geoset names */

  while ( n > 0 ) XmStringFree( item[--n] );
  
  /* Make the list of OI server combinarions for selected geoset */

  urls = servers = 0;
  if ( tv_database.tv_curr_geo >= 0 ) {
    geoset = &tv_database.database->geoset[tv_database.tv_curr_geo];
    dataset = tsmGetDataSet( tv_database.database, geoset->oi_name );

    DBaseMakeUrlsServers( dataset, item, &urls, server, &servers,
			  tv_database.tv_curr_oi, tv_database.curr_oi_inst );

  }

  /* Now refresh the OI URLs list widget if we have been told to */

  if ( refresh & DBASE_REFRESH_OI_URLS ) {
    XtVaSetValues( tv_database.tv_oiloc, XmNitems, item,
		   XmNitemCount, urls, NULL );

    if ( tv_database.tv_curr_oi >= 0 )
      XmListSelectPos( tv_database.tv_oiloc, tv_database.tv_curr_oi+1, False);
  }

  /* And refresh the OI servers list widget if we have been told to */

  if ( refresh & DBASE_REFRESH_OI_SERVERS ) {
    XtVaSetValues( tv_database.tv_oiserv, XmNitems, server,
		   XmNitemCount, servers, NULL );

    if ( tv_database.tv_curr_oi_serv >= 0 )
      XmListSelectPos( tv_database.tv_oiserv, tv_database.tv_curr_oi_serv + 1,
		       False );
  }

  /* Free the list of OI urls and server names */

  while ( urls > 0 ) XmStringFree( item[--urls] );
  while ( servers > 0 ) XmStringFree( server[--servers] );

  /* Make the list of DEM server combinarions for selected geoset */

  urls = servers = 0;
  if ( tv_database.tv_curr_geo >= 0 ) {
    geoset = &tv_database.database->geoset[tv_database.tv_curr_geo];
    dataset = tsmGetDataSet( tv_database.database, geoset->dem_name );

    DBaseMakeUrlsServers( dataset, item, &urls, server, &servers,
			  tv_database.tv_curr_dem, tv_database.curr_dem_inst );

  }

  /* Now refresh the DEM URLs list widget if we have been told to */

  if ( refresh & DBASE_REFRESH_DEM_URLS ) {
    XtVaSetValues( tv_database.tv_demloc, XmNitems, item,
		   XmNitemCount, urls, NULL );

    if ( tv_database.tv_curr_dem >= 0 )
      XmListSelectPos( tv_database.tv_demloc, tv_database.tv_curr_dem+1,
		       False);
  }

  /* And refresh the DEM servers list widget if we have been told to */

  if ( refresh & DBASE_REFRESH_DEM_SERVERS ) {
    XtVaSetValues( tv_database.tv_demserv, XmNitems, server,
		   XmNitemCount, servers, NULL );

    if ( tv_database.tv_curr_dem_serv >= 0 )
      XmListSelectPos( tv_database.tv_demserv, tv_database.tv_curr_dem_serv+1,
		       False );
  }

  /* Free the list of DEM urls and server names */

  while ( urls > 0 ) XmStringFree( item[--urls] );
  while ( servers > 0 ) XmStringFree( server[--servers] );

}

/*
 * Synopsis:
 *   TsmDatabase *GetDBase( void )
 *
 * Description:
 *   This function will return the currently loaded TsmDatabase structure.
 *
 * Returns: A TsmDatabase structure
 *
 * Author: Martin Reddy
 *
 * Date: 8 March 1999
 *
 */

TsmDatabase *
GetDBase( void )
{
  return tv_database.database;
}
