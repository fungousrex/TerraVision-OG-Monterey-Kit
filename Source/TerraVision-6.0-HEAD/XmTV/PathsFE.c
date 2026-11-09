/*
 * Name: PATHSFE.C
 *
 * Description:
 *     Implements TerraVision's Flight Path facilities. This lets the
 *     user define and fly a preset flight path around a dataset. Options
 *     include setting the interpolation type, looping of the animation,
 *     managing the update rate (Hz) and speed (m/s) of the flythrough, etc.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     November 26, 1998
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

#include "Dialogs.h"
#include "ScannedMap.h"
#include "PathsFE.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "OAA.h"

#include "X11Include.h"

#include <tvutils/tvutils.h>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include <prio.h>


static void PathGenericHideCB( Widget, XtPointer, XtPointer );
static void PathLoadCB( Widget, XtPointer, XtPointer );
static void PathSaveCB( Widget, XtPointer, XtPointer );
static void PathInterp1CB( Widget, XtPointer, XtPointer );
static void PathInterp2CB( Widget, XtPointer, XtPointer );
static void PathInterp3CB( Widget, XtPointer, XtPointer );
static void PathVelocityCB( Widget, XtPointer, XtPointer );
static void PathTitleCB( Widget, XtPointer, XtPointer );
static void PathLoopCB( Widget, XtPointer, XtPointer );
static void PathCloseLoopCB( Widget, XtPointer, XtPointer );
static void PathScaleCB( Widget, XtPointer, XtPointer );
static void PathListCB( Widget, XtPointer, XtPointer );
static void PathAddCB( Widget, XtPointer, XtPointer );
static void PathDeleteCB( Widget, XtPointer, XtPointer );
static void PathUpdateCB( Widget, XtPointer, XtPointer );
static void PathDuplicateCB( Widget, XtPointer, XtPointer );
static void PathUpCB( Widget, XtPointer, XtPointer );
static void PathDownCB( Widget, XtPointer, XtPointer );
static void PathRefreshList( ThreeDWidget *threeDWidget );
static void PathRefreshAll( ThreeDWidget *threeDWidget );
static void PathFileSelect( ThreeDWidget *, Widget *, char const *,
			    char const *, XtCallbackProc );

static void 
PathSaveCB( Widget dialog, XtPointer client_data, XtPointer call_data ) 
{
  char         *file = NULL, filename[256];
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

  /* Pop down the file requester first */

  XtUnmanageChild( dialog );

  /* Get the filename from the file selector widget */
  if (! XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &file ) )
    return;

  strcpy( filename, file );
  XtFree( file );
  tsmAddFilenameExtension( filename, ".path" );

  /* Save the path file */
  PathSaveFile( threeDWidget->threeD, filename );
}

static void 
PathLoadCB( Widget dialog, XtPointer client_data, XtPointer call_data ) 
{
  char         *file = NULL;
  PRFileInfo   filetest;

  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  XmFileSelectionBoxCallbackStruct *cbs =
    (XmFileSelectionBoxCallbackStruct *) call_data;

  /* Pop down the file requester first */

  XtUnmanageChild( dialog );

  /* Get the filename from the file selector widget */
  if (! XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &file ) )
    return;

  /* Check that file exists */

  if ( PR_GetFileInfo ( file, &filetest ) == PR_FAILURE ) {
    ThreeDWidgetError( threeDWidget, "Path file does not exist!" );
    XtFree( file );
    return;
  }

  /* Load the path file */
  if ( PathLoadFile( threeDWidget->threeD, file ) ) {
    PathGotoStart ( threeDWidget->threeD );
    PathRefreshAll( threeDWidget );
  }

  XtFree( file );

  /* refresh the map display as it may want to display the path */
  ScannedMapRedraw();
}

static void
PathFileSelect( ThreeDWidget *threeDWidget, Widget *path_sel,
		char const *dialog_title, char const *button_name,
		XtCallbackProc callback )
{
  Arg      args[10];
  int      n = 0;
  char     dirname[512];
  XmString title, pattern, directory;
  Widget   widget;

  if ( *path_sel == NULL ) {

    /* Set the initial arguments, i.e. the file selector title */

    title = tvXmString( dialog_title );
    XtSetArg( args[n], XmNdialogTitle, title ); n++;

    /* Create the file selector widget and callbacks */

    *path_sel = XmCreateFileSelectionDialog( applicationShell, "sel", args, n);
    XtAddCallback( *path_sel, XmNokCallback, callback, threeDWidget );
    XtAddCallback( *path_sel, XmNcancelCallback, (XtCallbackProc)
		   XtUnmanageChild, NULL );
    XmStringFree( title );

    /* gray out the Help button as we don't use it */

    widget = XmFileSelectionBoxGetChild( *path_sel, XmDIALOG_HELP_BUTTON );
    if ( widget ) XtSetSensitive( widget, FALSE );

    /* set the name of the Okay button accordingly */

    widget = XmFileSelectionBoxGetChild( *path_sel, XmDIALOG_OK_BUTTON );
    if ( widget ) {
      title = tvXmString( button_name );
      XtVaSetValues( widget, XmNlabelString, title, NULL );
      XmStringFree( title );
    }

  }

  /* Set the directory and pattern for the file selector */

  strcpy( dirname, tvuGetString( TVU_PATHS_DIR ) );
  tsmMakeDirIfNotExist( dirname, TSM_PATH_ONLY );

  directory = tvXmString( dirname );
  pattern = tvXmString( "*.path" );
  XtVaSetValues( *path_sel, XmNdirectory, directory,
		 XmNpattern, pattern, NULL );
  XmStringFree( directory );
  XmStringFree( pattern );

  /* pop up the file selector */

  XtManageChild( *path_sel );
  XtPopup( XtParent(*path_sel), XtGrabNone );
}


void
PathLoad( ThreeDWidget *threeDWidget )
{
  PathFileSelect( threeDWidget, &threeDWidget->pathLoadSel,
		  "Load Path File", "Load", PathLoadCB );
}

void
PathSaveAs( ThreeDWidget *threeDWidget )
{
  PathFileSelect( threeDWidget, &threeDWidget->pathSaveSel,
		  "Save Path File", "Save", PathSaveCB );
}

static void
PathInitEdit( ThreeDWidget *threeDWidget )
{
  Widget   mainform, form, widget, topform, dismissform, leftform, rightform;
  Widget   separator, label, textlist, frame;
  XmString string;
  Arg      args[15];
  int      n = 0;

  /* Create the main dialog window for the dataset selection */

  mainform = XmCreateFormDialog( threeDWidget->mainForm, "Path", args, n );
  threeDWidget->pathEditWidget = mainform;

  /* The main part of the window is split into a left and right form */
  
  topform =
    XtVaCreateManagedWidget( "mainForm",
			     xmFormWidgetClass, mainform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNverticalSpacing, 10,
			     XmNhorizontalSpacing, 10,
			     NULL );

  rightform =
    XtVaCreateManagedWidget( "rightForm",
			     xmFormWidgetClass, topform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     NULL );

  leftform =
    XtVaCreateManagedWidget( "leftForm",
			     xmFormWidgetClass, topform,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_WIDGET,
			     XmNrightWidget, rightform,
			     NULL );

  /* Create the Add/Del/Dup etc buttons below the scrolled list */

  n = 0;
  XtSetArg( args[n], XmNfractionBase, 6 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  form = XtCreateWidget( "buttons", xmFormWidgetClass, leftform, args, n );
  XtManageChild( form );

  threeDWidget->pathAddWidget =
    XtVaCreateManagedWidget( " Add ",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 0,
			     XmNrightAttachment, XmATTACH_POSITION,
			     XmNrightPosition, 1,
			     NULL );
  XtAddCallback( threeDWidget->pathAddWidget, XmNactivateCallback,
		 PathAddCB, threeDWidget );

  threeDWidget->pathDelWidget =
    XtVaCreateManagedWidget( " Del ",
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
  XtAddCallback( threeDWidget->pathDelWidget, XmNactivateCallback,
		 PathDeleteCB, threeDWidget );

  threeDWidget->pathUpdtWidget =
    XtVaCreateManagedWidget( " Updt ",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 2,
			     XmNrightAttachment, XmATTACH_POSITION,
			     XmNrightPosition, 3,
			     NULL );
  XtAddCallback( threeDWidget->pathUpdtWidget, XmNactivateCallback,
		 PathUpdateCB, threeDWidget );

  threeDWidget->pathDupWidget =
    XtVaCreateManagedWidget( " Dup ",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 3,
			     XmNrightAttachment, XmATTACH_POSITION,
			     XmNrightPosition, 4,
			     NULL );
  XtAddCallback( threeDWidget->pathDupWidget, XmNactivateCallback,
		 PathDuplicateCB, threeDWidget );

  threeDWidget->pathUpWidget =
    XtVaCreateManagedWidget( " Up ",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 4,
			     XmNrightAttachment, XmATTACH_POSITION,
			     XmNrightPosition, 5,
			     NULL );
  XtAddCallback( threeDWidget->pathUpWidget, XmNactivateCallback,
		 PathUpCB, threeDWidget );

  threeDWidget->pathDnWidget =
    XtVaCreateManagedWidget( " Dn ",
			     xmPushButtonGadgetClass, form,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_POSITION,
			     XmNleftPosition, 5,
			     XmNrightAttachment, XmATTACH_POSITION,
			     XmNrightPosition, 6,
			     NULL );
  XtAddCallback( threeDWidget->pathDnWidget, XmNactivateCallback,
		 PathDownCB, threeDWidget );

  /* Create the scrolled text widget where all the viewpoints are listed */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNbottomWidget, form ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++;  
  XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;
  XtSetArg( args[n], XmNvisibleItemCount, 10 ); n++;
  XtSetArg( args[n], XmNwidth, 300 ); n++;

  textlist = XmCreateScrolledList( leftform, "scrolledList", args, n );
  threeDWidget->pathListWidget = textlist;
  XtManageChild( textlist );
  XtAddCallback( textlist, XmNbrowseSelectionCallback, PathListCB,
		 threeDWidget );

  /* Create the frame that contains the various path options */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_ETCHED_IN ); n++;
  frame = XmCreateFrame( rightform, "frame1", args, n );
  XtManageChild( frame );

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  frame = XtCreateWidget( "frame1form", xmFormWidgetClass, frame, args, n );
  XtManageChild( frame );

  /* create the path title widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "titleform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Path Title :", args, n ); n++;
  XtManageChild( label );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNcolumns, 10 ); n++;
  XtSetArg( args[n], XmNvalue, "" ); n++;
  threeDWidget->pathTitleWidget = XmCreateText( form, "pathTitle", args, n );
  XtManageChild( threeDWidget->pathTitleWidget );
  XtAddCallback( threeDWidget->pathTitleWidget, XmNvalueChangedCallback,
		 PathTitleCB, threeDWidget );

  /* create the velocity value widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, form ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "velform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Velocity (m/s) :", args, n ); n++;
  XtManageChild( label );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNcolumns, 10 ); n++;
  XtSetArg( args[n], XmNvalue, "" ); n++;
  threeDWidget->pathVelWidget = XmCreateText( form, "pathVelocity", args, n );
  XtManageChild( threeDWidget->pathVelWidget );
  XtAddCallback( threeDWidget->pathVelWidget, XmNvalueChangedCallback,
		 PathVelocityCB, threeDWidget );

  /* create the looping widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, form ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "loopform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Continual Loop :", args, n ); n++;
  XtManageChild( label );

  n=0;
  string = tvXmString( "" );
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNindicatorOn, True ); n++;
  XtSetArg( args[n], XmNlabelString, string ); n++;
  threeDWidget->pathLoopWidget =
    XmCreateToggleButton( form, "loopToggle", args, n );
  XtManageChild( threeDWidget->pathLoopWidget );
  XmStringFree( string );
  XtAddCallback( threeDWidget->pathLoopWidget, XmNvalueChangedCallback,
		 PathLoopCB, threeDWidget );

  /* create the clos the loop widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, form ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "closeform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Close the Loop :", args, n ); n++;
  XtManageChild( label );

  n=0;
  string = tvXmString( "" );
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNindicatorOn, True ); n++;
  XtSetArg( args[n], XmNlabelString, string ); n++;
  threeDWidget->pathCloseLoopWidget =
    XmCreateToggleButton( form, "closeToggle", args, n );
  XtManageChild( threeDWidget->pathCloseLoopWidget );
  XmStringFree( string );
  XtAddCallback( threeDWidget->pathCloseLoopWidget, XmNvalueChangedCallback,
		 PathCloseLoopCB, threeDWidget );

  /* create the speed scaling widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, form ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "scaleform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Height Speed Scaling :", args, n ); n++;
  XtManageChild( label );

  n=0;
  string = tvXmString( "" );
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNindicatorOn, True ); n++;
  XtSetArg( args[n], XmNlabelString, string ); n++;
  threeDWidget->pathScaleWidget =
    XmCreateToggleButton( form, "scaleToggle", args, n );
  XtManageChild( threeDWidget->pathScaleWidget );
  XmStringFree( string );
  XtAddCallback( threeDWidget->pathScaleWidget, XmNvalueChangedCallback,
		 PathScaleCB, threeDWidget );

  /* create the interpolation widget */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, form ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  form = XtCreateWidget( "interpform", xmFormWidgetClass, frame, args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( form, "Interpolation :", args, n ); n++;
  XtManageChild( label );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNleftWidget, label ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_ETCHED_IN ); n++;
  form = XmCreateFrame( form, "frame1", args, n );
  XtManageChild( form );

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  widget = XmCreateRadioBox( form, "interpRadio", args, n );

  threeDWidget->pathInterp[0] =
    XtVaCreateManagedWidget( "None", xmToggleButtonGadgetClass, widget, NULL );
  XtAddCallback( threeDWidget->pathInterp[0], XmNvalueChangedCallback,
		 PathInterp1CB, threeDWidget );

  threeDWidget->pathInterp[1] =
    XtVaCreateManagedWidget( "Linear", xmToggleButtonGadgetClass, widget,NULL);
  XtAddCallback( threeDWidget->pathInterp[1], XmNvalueChangedCallback,
		 PathInterp2CB, threeDWidget );

  threeDWidget->pathInterp[2] =
    XtVaCreateManagedWidget( "Spline", xmToggleButtonGadgetClass, widget,NULL);
  XtAddCallback( threeDWidget->pathInterp[2], XmNvalueChangedCallback,
		 PathInterp3CB, threeDWidget );

  XtManageChild( widget );

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

  XtVaSetValues( topform, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, separator, NULL );

  XtAddCallback( widget, XmNactivateCallback, PathGenericHideCB, threeDWidget);

}

static void
PathInterp1CB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  if ( ((XmToggleButtonCallbackStruct *) call_data)->set ) {
    PathSetInterpolate (threeDWidget->threeD, PATH_INTERPOLATE_NONE);
  }
}

static void
PathInterp2CB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  if ( ((XmToggleButtonCallbackStruct *) call_data)->set ){
    PathSetInterpolate (threeDWidget->threeD, PATH_INTERPOLATE_LINEAR);
  }
}

static void
PathInterp3CB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  if ( ((XmToggleButtonCallbackStruct *) call_data)->set ) {
    PathSetInterpolate (threeDWidget->threeD, PATH_INTERPOLATE_SPLINE);
  }
}

static void
PathLoopCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  Path *path = tvGetPath (threeDWidget->threeD);
  path->loop = ((XmToggleButtonCallbackStruct *) call_data)->set;
}

static void
PathCloseLoopCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  Path *path = tvGetPath (threeDWidget->threeD);
  path->close_loop = ((XmToggleButtonCallbackStruct *) call_data)->set;
}

static void
PathScaleCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  Path *path = tvGetPath (threeDWidget->threeD);
  path->speed_scaling = ((XmToggleButtonCallbackStruct *) call_data)->set;
}

static void
PathTitleCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char *text;

  if ( ( text = XmTextGetString( w ) ) != NULL ) {
    PathSetName (threeDWidget->threeD, text);
    XtFree( text );
  }
}

static void
PathVelocityCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  char *text;

  if ( ( text = XmTextGetString( w ) ) != NULL ) { 
    Path *path = tvGetPath (threeDWidget->threeD);
    path->speed = fabs( atof( text ) );
    XtFree( text );
    if ( path->speed == 0.0 )
      PathStop (threeDWidget->threeD);
  }
}

static void
PathListCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int          numb = ((XmListCallbackStruct *) call_data)->item_position - 1;

  PathGoto (threeDWidget->threeD, numb);
}

static int
PathGetSelected( ThreeDWidget *threeDWidget )
{
  Widget list = threeDWidget->pathListWidget;
  XmStringTable selected, items;
  int           count, selcount, n;
  char          *str1, *str2;

  XtVaGetValues( list, XmNselectedItems, &selected, XmNselectedItemCount,
		 &selcount, XmNitems, &items, XmNitemCount, &count, NULL );

  if ( selcount == 1 ) {
    XmStringGetLtoR( selected[0], XmFONTLIST_DEFAULT_TAG, &str1 );
    for ( n = 0; n < count; n++ ) {
      XmStringGetLtoR( items[n], XmFONTLIST_DEFAULT_TAG, &str2 );
      if ( strcmp( str1, str2 ) == 0 ) {
	XtFree( str1 );
	XtFree( str2 );
	return n;
      }
      XtFree( str2 );
    }
    XtFree( str1 );
  }

  return -1;
}

static void
PathSetSelected( ThreeDWidget *threeDWidget, int numb )
{
  Widget list = threeDWidget->pathListWidget;
  int           count;

  if (1) {
    XtVaGetValues( list, XmNitemCount, &count, NULL );

    if ( count == 0 ) return;
    if ( numb < 0 ) numb = 0;
    if ( numb >= count ) numb = count - 1;

    XmListSelectPos (list, numb+1, True);
  } else {
    XmStringTable item;
    XmString sel[1];

    XtVaGetValues( list, XmNitems, &item, XmNitemCount, &count, NULL );

    if ( count == 0 ) return;
    if ( numb < 0 ) numb = 0;
    if ( numb >= count ) numb = count - 1;

    sel[0] = item[numb];
    XtVaSetValues( list,
		   XmNselectedItems, sel,
		   XmNselectedItemCount, 1,
		   NULL );
  }
}

static void
PathAddCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int sel = PathAdd (threeDWidget->threeD);
  PathRefreshList( threeDWidget );
  PathSetSelected( threeDWidget, sel );
}

static void
PathDeleteCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int selected = PathGetSelected( threeDWidget );
  int sel = PathDelete (threeDWidget->threeD, selected);
  PathRefreshList( threeDWidget );
  PathSetSelected( threeDWidget, sel );
}

static void
PathUpdateCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int selected = PathGetSelected( threeDWidget );

  PathUpdate (threeDWidget->threeD, selected);
  PathRefreshList( threeDWidget );
}

static void
PathDuplicateCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int selected = PathGetSelected( threeDWidget );
  int sel = PathDuplicate (threeDWidget->threeD, selected);

  PathRefreshList( threeDWidget );
  PathSetSelected( threeDWidget, sel );
}

static void
PathUpCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int selected = PathGetSelected( threeDWidget );
  int sel = PathUp (threeDWidget->threeD, selected);

  PathRefreshList( threeDWidget );
  PathSetSelected( threeDWidget, sel );
}

static void
PathDownCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;
  int selected = PathGetSelected( threeDWidget );
  int sel = PathDown (threeDWidget->threeD, selected);

  PathRefreshList( threeDWidget );
  PathSetSelected( threeDWidget, sel );
}

static void
PathRefreshList( ThreeDWidget *threeDWidget )
{
  Widget   list = threeDWidget->pathListWidget;
  char     buffer[256], lng, lat;
  XmString name[100];
  int      n;
  Path 	   *path = tvGetPath (threeDWidget->threeD);
  double   gdc[3];

  if ( list == NULL ) return;

  for ( n = 0; n < path->count; ++n ) {
    PathItem *item = &path->item[n];
    TsGCCtoGDC( item->x, item->y, item->z, gdc );

    if ( gdc[0] < 0.0 ) { lng = 'W'; gdc[0] = -gdc[0]; } else lng = 'E';
    if ( gdc[1] < 0.0 ) { lat = 'S'; gdc[1] = -gdc[1]; } else lat = 'N';

    sprintf( buffer, "%d: %1.3g%c %1.3g%c %1.0fm",
	     n+1, gdc[0], lng, gdc[1], lat, gdc[2] );

    name[n] = tvXmString( buffer );
  }
  
  /* Refresh the list widget of the database options window */

  XtVaSetValues( list, XmNitems, name, XmNitemCount, n, NULL );

  /* Free the array of XmStrings that we have just created */
  
  while ( n > 0 ) XmStringFree( name[--n] );
}

static void
PathRefreshAll( ThreeDWidget *threeDWidget )
{
  char buffer[32];
  Path *path = tvGetPath (threeDWidget->threeD);

  if ( threeDWidget == NULL || threeDWidget->pathEditWidget == NULL ) return;

  /* Refresh the display of all viewpoints */
  PathRefreshList( threeDWidget );

  /* Refresh the path title name */
  XtVaSetValues( threeDWidget->pathTitleWidget, XmNvalue, path->name, NULL );

  /* Refresh the velocity setting */
  sprintf( buffer, "%1.1f", path->speed );
  XtVaSetValues( threeDWidget->pathVelWidget, XmNvalue, buffer, NULL );

  /* Refresh the looping status */
  XtVaSetValues( threeDWidget->pathLoopWidget, XmNset, path->loop, NULL );

  /* Refresh the close loop status */
  XtVaSetValues( threeDWidget->pathCloseLoopWidget, XmNset, path->close_loop, NULL );

  /* Refresh the scaling status */
  XtVaSetValues( threeDWidget->pathScaleWidget, XmNset, path->speed_scaling, NULL );

  /* Refresh the interpolation mode */
  XtVaSetValues( threeDWidget->pathInterp[0], XmNset,
		 (path->interpolate == PATH_INTERPOLATE_NONE), NULL );
  XtVaSetValues( threeDWidget->pathInterp[1], XmNset,
		 (path->interpolate == PATH_INTERPOLATE_LINEAR), NULL );
  XtVaSetValues( threeDWidget->pathInterp[2], XmNset,
		 (path->interpolate == PATH_INTERPOLATE_SPLINE), NULL );
}

static void
PathGenericShow( ThreeDWidget *threeDWidget, char const *title )
{

  /* create the widget structures if this is the first time round */

  if ( threeDWidget == NULL ) return;
  if ( threeDWidget->pathEditWidget == NULL ) PathInitEdit( threeDWidget );

  PathRefreshAll( threeDWidget );

  /* if window currently displayed, do nothing */

  if ( XtIsManaged( threeDWidget->pathEditWidget ) ) return;

  /* set the window title if we have a new title */

  if ( title != NULL ) {
    XmString text = tvXmString( title );
    XtVaSetValues( threeDWidget->pathEditWidget, XmNdialogTitle, text, NULL );
    XmStringFree( text );
  }

  /* display the window */

  XtManageChild( threeDWidget->pathEditWidget );
}

static void
PathGenericHideCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  ThreeDWidget *threeDWidget = (ThreeDWidget *) client_data;

  if ( threeDWidget == NULL || threeDWidget->pathEditWidget == NULL ) return;

  if ( XtIsManaged( threeDWidget->pathEditWidget ) )
    XtUnmanageChild( threeDWidget->pathEditWidget );
}

void
PathEditHide( ThreeDWidget *threeDWidget )
{
  PathGenericHideCB( NULL, threeDWidget, NULL );
}

void
PathNewHide( ThreeDWidget *threeDWidget )
{
  PathStop(threeDWidget->threeD);
  PathGenericHideCB( NULL, threeDWidget, NULL );
}

void
PathNewFE( ThreeDWidget *threeDWidget )
{
  PathNew (threeDWidget->threeD);
  PathGenericShow( threeDWidget, "New Path" );
}

void
PathEdit( ThreeDWidget *threeDWidget )
{
  if ( ! tvGetPath (threeDWidget->threeD) )
    PathNewFE( threeDWidget );
  else
    PathGenericShow( threeDWidget, "Edit Path" );  
}

/*
 * Synopsis:
 *   int PathLoadInitial( ThreeDWidget *threeDWidget, int play_it_too )
 *
 * Description:
 *   This function will take the filename that was specified in the
 *   last call to PathSetInitial(). It will then work out the full
 *   pathname for the flight file by prepending the directory,
 *   $HOME/.TerraVisionPaths. The function will also add a ".path"
 *   extension to the filename if not already specified.
 *
 *   Once we have the full pathname for the flight path, we load it
 *   and optionally begin playing it (if play_it_too is TRUE).
 *
 *   If the full path file does not exist, or if it is not loaded
 *   successfully, then this function will return silently with a
 *   result of FALSE.
 *
 * Returns: TRUE if the load operation was successful, FALSE otherwise
 *
 * Author: Martin Reddy
 * 
 * Date: 7 April, 1999
 * */

int
PathLoadInitial( ThreeDWidget *threeDWidget, int play_it_too )
{
  PRFileInfo filetest;
  char       *filename = tvuGetString( TVU_PATH_URL );

  if ( filename == NULL || threeDWidget == NULL )
    return FALSE;

  /* Check that file exists */

  if ( PR_GetFileInfo ( filename, &filetest ) == PR_FAILURE )
    return FALSE;
  
  /* try to load the path file */

  if ( PathLoadFile( threeDWidget->threeD, filename ) == FALSE )
    return FALSE;

  /* do we want to animate it right now? */

  if ( play_it_too )
    PathPlay( threeDWidget->threeD );

  return TRUE;
}
