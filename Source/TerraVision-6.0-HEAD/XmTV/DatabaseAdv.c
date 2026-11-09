/*
 * Name: DatabaseAdv.c
 *
 * Description:
 *     Lets the user manage various advanced options for the OI and
 *     DEM connections. This includes things like the image format
 *     the tile format, where tspecs come from, etc.
 *
 * Function List:
 *   void DBaseCreateDialog( Widget, TsmDataSetType, TsmConnectionParams );
 *   void DBaseHideAdvancedCB( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_DPSSFirst( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_MetadataFirst( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_Fallback( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_Validate( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_Tile( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_TIFF( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_Raw( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_JPEG( Widget w, XtPointer client_data, XtPointer );
 *   void AdvCB_LZW( Widget w, XtPointer client_data, XtPointer );
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

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>

static Widget adv_oi_dialog  = NULL;
static Widget adv_dem_dialog = NULL;

static void DBaseCreateDialog( Widget, TsmDataSetType, TsmConnectionParams );
static void DBaseHideAdvancedCB( Widget w, XtPointer client_data, XtPointer );

static void AdvCB_DPSSFirst( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_MetadataFirst( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_Fallback( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_Validate( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_Tile( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_TIFF( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_Raw( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_JPEG( Widget w, XtPointer client_data, XtPointer );
static void AdvCB_LZW( Widget w, XtPointer client_data, XtPointer );

/*
 * Synopsis:
 *   void DBaseInitAdvanced( Widget parent )
 *
 * Description:
 *   This function sets up all of the widgets for the Database
 *   advanced widget. This lets you manage various advanced
 *   settings for the OI and DEM connection.
 *
 *   This function is called by DBaseInit() [Database.c], and
 *   should not be called directly.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
DBaseInitAdvanced( Widget parent )
{
  DBaseCreateDialog( parent, TSM_IMAGE_DATASET, tv_database.oi_params );
  DBaseCreateDialog( parent, TSM_DEM_DATASET, tv_database.dem_params );
}

void
DBaseCreateDialog( Widget parent, TsmDataSetType type,
		   TsmConnectionParams params )
{
  char      *title = "Dataset Options";
  Widget    dialog, form, widget, frame;
  Widget    rowcol, toggle1, toggle2, toggle3, sep, radio;
  XmString  name;
  XtPointer xparams = (XtPointer) params;
  Arg       args[15];
  int       n = 0;

  /* Create the main dialog window for the dataset selection */

  if ( type == TSM_IMAGE_DATASET )
    name = tvXmString( "Advanced Imagery Settings" );
  else
    name = tvXmString( "Advanced DEM Settings" );
  
  XtSetArg( args[n], XmNdialogTitle, name ); n++;
  XtSetArg( args[n], XmNnoResize, True ); n++;
  dialog = XmCreateFormDialog( parent, title, args, n );
  XmStringFree( name );
  
  form = XtVaCreateManagedWidget( "mainForm", 
				  xmFormWidgetClass, dialog,
				  XmNtopAttachment, XmATTACH_FORM,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  XmNhorizontalSpacing, 10,
				  XmNverticalSpacing, 10,
				  NULL );

  /* The Image Format frame section */

  frame = XtVaCreateManagedWidget( "frame1",
				   xmFrameWidgetClass, form,
				   XmNshadowType, XmSHADOW_IN,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNbottomAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_FORM,
				   NULL );
  name = tvXmString("Image Format");
  XtVaCreateManagedWidget( "frame1Title",
			   xmLabelGadgetClass, frame,
			   XmNchildType, XmFRAME_TITLE_CHILD,
			   XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
			   XmNlabelString, name,
			   NULL );
  XmStringFree (name);

  radio = XmCreateRadioBox( frame, "radio1", NULL, 0 );

  toggle1 = XtVaCreateManagedWidget( "Raw uncompressed",
				     xmToggleButtonGadgetClass, radio, NULL );
  XmToggleButtonGadgetSetState( toggle1, True, False );
  XtAddCallback( toggle1, XmNvalueChangedCallback, AdvCB_Raw, xparams );

  toggle2 = XtVaCreateManagedWidget( "JPEG compressed",
				     xmToggleButtonGadgetClass, radio, NULL );
  XtAddCallback( toggle2, XmNvalueChangedCallback, AdvCB_JPEG, xparams );

  toggle3 = XtVaCreateManagedWidget( "LZW compressed",
				     xmToggleButtonGadgetClass, radio, NULL );
  XtAddCallback( toggle3, XmNvalueChangedCallback, AdvCB_LZW, xparams );

  XtManageChild( radio );

  /* The Tile Format frame section */

  frame = XtVaCreateManagedWidget( "frame2",
				   xmFrameWidgetClass, form,
				   XmNshadowType, XmSHADOW_IN,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNbottomAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_WIDGET,
				   XmNleftWidget, frame,
				   NULL );

  name = tvXmString("Local Tile Format");
  XtVaCreateManagedWidget( "frame2Title",
			   xmLabelGadgetClass, frame,
			   XmNchildType, XmFRAME_TITLE_CHILD,
			   XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
			   XmNlabelString, name,
			   NULL );
  XmStringFree (name);

  radio = XmCreateRadioBox( frame, "radio2", NULL, 0 );

  toggle1 = XtVaCreateManagedWidget( "TIFF files (.tiff)",
				     xmToggleButtonGadgetClass, radio, NULL );
  XmToggleButtonGadgetSetState( toggle1, True, False );
  XtAddCallback( toggle1, XmNvalueChangedCallback, AdvCB_TIFF, xparams );

  toggle2 = XtVaCreateManagedWidget( "Raw tiles (.tile)",
				     xmToggleButtonGadgetClass, radio, NULL );
  XtAddCallback( toggle2, XmNvalueChangedCallback, AdvCB_Tile, xparams );

  XtManageChild( radio );

  /* The DPSS Tspec frame section */

  frame = XtVaCreateManagedWidget( "frame3",
				   xmFrameWidgetClass, form,
				   XmNshadowType, XmSHADOW_IN,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNbottomAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_WIDGET,
				   XmNleftWidget, frame,
				   XmNrightAttachment, XmATTACH_FORM,
				   NULL );

  name = tvXmString("DPSS tspecs");
  XtVaCreateManagedWidget( "frame3Title",
			   xmLabelGadgetClass, frame,
			   XmNchildType, XmFRAME_TITLE_CHILD,
			   XmNchildVerticalAlignment, XmALIGNMENT_WIDGET_TOP,
			   XmNlabelString, name,
			   NULL );
  XmStringFree (name);

  radio = XmCreateRadioBox( frame, "radio3", NULL, 0 );

  toggle1 = XtVaCreateManagedWidget( "Check on DPSS first",
				     xmToggleButtonGadgetClass, radio, NULL );
  XmToggleButtonGadgetSetState( toggle1, True, False );
  XtAddCallback( toggle1, XmNvalueChangedCallback, AdvCB_DPSSFirst, xparams );

  toggle2 = XtVaCreateManagedWidget( "Check Metadata URLs only",
				     xmToggleButtonGadgetClass, radio, NULL );
  XtAddCallback( toggle2, XmNvalueChangedCallback, AdvCB_MetadataFirst,
		 xparams );

  XtManageChild( radio );

  /* Now we add the on/off advanced selections */

  form = XtVaCreateManagedWidget( "mainForm", 
				  xmFormWidgetClass, dialog,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, form,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  XmNhorizontalSpacing, 10,
				  XmNverticalSpacing, 10,
				  NULL );

  rowcol = XtVaCreateManagedWidget( "rowcol", xmRowColumnWidgetClass, form,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    NULL );

  toggle1 = XtVaCreateManagedWidget( "Fail if above options are not available",
				     xmToggleButtonGadgetClass, rowcol,
				     NULL );
  XtAddCallback( toggle1, XmNvalueChangedCallback, AdvCB_Fallback, xparams );

  toggle2 = XtVaCreateManagedWidget( "Validate tspec file on load",
				     xmToggleButtonGadgetClass, rowcol,
				     NULL );
  XmToggleButtonGadgetSetState( toggle2, True, False );
  XtAddCallback( toggle2, XmNvalueChangedCallback, AdvCB_Validate, xparams );

  /* Add a separator bar, 'cos they're cool. */

  sep = XtVaCreateManagedWidget( "separator", 
				 xmSeparatorGadgetClass, dialog,
				 XmNtopAttachment, XmATTACH_WIDGET,
				 XmNtopWidget, form,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 NULL );

  /* Add finally we add an okay button */

  form = XtVaCreateManagedWidget( "mainForm", 
				  xmFormWidgetClass, dialog,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, sep,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  XmNbottomAttachment, XmATTACH_FORM,
				  XmNhorizontalSpacing, 10,
				  XmNverticalSpacing, 10,
				  NULL );

  name = tvXmString("Dismiss");
  widget =
    XtVaCreateManagedWidget( "Dismiss",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNlabelString, name,
			     XmNmarginTop, 3,
			     XmNmarginBottom, 3,
			     XmNmarginLeft, 5,
			     XmNmarginRight, 5,
			     NULL );
  XmStringFree (name);

  XtAddCallback( widget, XmNactivateCallback, DBaseHideAdvancedCB, dialog );

  /* Save the dialog widget as either the DEM or the OI */

  if ( type == TSM_IMAGE_DATASET )
    adv_oi_dialog = dialog;
  else if ( type == TSM_DEM_DATASET )
    adv_dem_dialog = dialog;
}

/*
 * Synopsis:
 *   void DBaseShowAdvanced( TsmDataSetType type )
 *
 * Description:
 *   This function will display the Database Advanced window for
 *   either the DEM or the OI dataset. If you specify a type 
 *   other than TSM_IMAGE_DATASET or TSM_DEM_DATASET then nothing
 *   will happen.
 *
 *   You should have called DBaseInit() before getting anywhere
 *   near this function.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
DBaseShowAdvanced( TsmDataSetType type )
{
  Widget dialog;

  if ( type == TSM_IMAGE_DATASET )
    dialog = adv_oi_dialog;
  else if ( type == TSM_DEM_DATASET )
    dialog = adv_dem_dialog;
  else 
    return;

  if ( dialog != NULL && ! XtIsManaged( dialog ) )
    XtManageChild( dialog );
}

/*
 * Synopsis:
 *   void DBaseHideAdvancedCB( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function will dismiss the Database Advanced windows for
 *   either the DEM or the OI dataset. The callback function is used
 *   to dismiss a particular window via the Dismiss button. The other
 *   function dismiss both windows.
 *
 *   You should have called DBaseInit() before getting anywhere
 *   near this function.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
DBaseHideAdvancedCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  Widget dialog = (Widget) client_data;

  if ( dialog != NULL && XtIsManaged( dialog ) )
    XtUnmanageChild( dialog );

  /*
  printf( "OI:\n---\n" );
  tsmPrintParams( tv_database.oi_params, stdout );
  printf( "DEM:\n----\n" );
  tsmPrintParams( tv_database.dem_params, stdout );
  */
}

void
DBaseHideAdvanced( void )
{
  DBaseHideAdvancedCB( NULL, adv_oi_dialog, NULL );
  DBaseHideAdvancedCB( NULL, adv_dem_dialog, NULL );
}

/*
 * Synopsis:
 *   void AdvCB_DPSSFirst( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_MetadataFirst( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_Fallback( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_Validate( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_Tile( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_TIFF( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_Raw( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_JPEG( Widget w, XtPointer client_data, XtPointer )
 *   void AdvCB_LZW( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   A load of callback functions that are entered when the particular
 *   radio button is pressed from the advanced options window. We use
 *   a different callback function for each option because we need to
 *   differentiate between the OI and DEM advanced window, and also
 *   between different TsmConnectionParams structures. Because we can
 *   only pass one client_data pointer, it was easier to just use
 *   dedicated functions rather.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
AdvCB_Validate( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct*) call_data;
  tsmSetParam( param, TSM_PARAM_VALIDATE_TSPEC, t->set );
}

void
AdvCB_Fallback( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  XmToggleButtonCallbackStruct *t = (XmToggleButtonCallbackStruct*) call_data;
  tsmSetParam( param, TSM_PARAM_FALLBACK, !t->set );
}

void
AdvCB_DPSSFirst( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_DPSS_TSPEC, TRUE );
}

void
AdvCB_MetadataFirst( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_DPSS_TSPEC, FALSE );
}

void
AdvCB_TIFF( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_TIFF, TRUE );
}

void
AdvCB_Tile( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_TIFF, FALSE );
}

void
AdvCB_Raw( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_FORMAT, TSM_LRBT_FORMAT );
}

void
AdvCB_JPEG( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_FORMAT, TSM_JPEG_FORMAT );
}

void
AdvCB_LZW( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmConnectionParams param = (TsmConnectionParams) client_data;
  if ( ((XmToggleButtonCallbackStruct*) call_data)->set ) 
    tsmSetParam( param, TSM_PARAM_FORMAT, TSM_LZW_FORMAT );
}
