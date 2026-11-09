/*
 * Name:  DIALOGS.C 
 *
 * Description: Dialog boxes
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
#include "X11Include.h"
#include "Dialogs.h"
#include "LICENSE.h"

#include <tsm/tsm.h>

#include <tvutils/tvutils.h>

#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>
#include <Xm/SeparatoG.h>

extern void TerraVisionButtonCB( Widget, XtPointer, XtPointer );


/*
 * Synopsis:
 *   CreateQuitDialog(Widget)
 *
 * Description:
 *    Create Confirm Quit Dialog Box
 *
 * Externals:
 *    quitDialog - Dialog Widget
 *
 * Returns: NONE
 *
 * Author: Stephen Lau
 * 
 * Date: October 19, 1992
 *
 */

Widget
CreateQuitDialog( Widget parent )
{
  /*static char const funcName[] = "CreateQuitDialog";*/
  Widget      quitDialog;
  XmString    string;
  int         n = 0;
  Arg         args[10];

  string = tvXmString("Quit Confirmation");
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL), n++;
  XtSetArg(args[n], XmNnoResize, TRUE), n++;
  XtSetArg(args[n], XmNdialogTitle, string), n++;
  quitDialog = XmCreateQuestionDialog(parent, "Quit Dialog", args, n);
  XmStringFree( string );

  string = tvXmString( "Do You Want To Quit TerraVision?" );
  XtVaSetValues( quitDialog, XmNmessageString, string, NULL );
  XmStringFree( string );

  /* WE DON'T WANT TO SEE THE HELP BUTTON */
  XtUnmanageChild(XmMessageBoxGetChild(quitDialog, XmDIALOG_HELP_BUTTON));

  /* AND WE WANT SOME ACTION ASSOCIATED WITH THE "OK" AND "CANCEL" BUTTONS */

  XtAddCallback(quitDialog, XmNcancelCallback, TerraVisionButtonCB, NULL);
  XtAddCallback(quitDialog, XmNokCallback, TerraVisionButtonCB, NULL);

  return quitDialog;
}

/*
 * Synopsis:
 *    CreateWarningDialog(Widget)
 *
 * Description:
 *      Create Warning Dialog Box
 *
 * Returns: NONE
 *
 * Author: Stephen Lau
 * 
 * Date: October 19, 1992
 *
 */

Widget
CreateWarningDialog(Widget parent)
{
  /*static char const funcName[] = "CreateWarningDialog";*/
  Widget      warningDialog;
  XmString    string1, string2;
  int         n = 0;
  Arg         args[12];

  string1 = tvXmString( "                          Generic Warning!\n" );
  string2 = tvXmString( "Notice!" );
  XtSetArg(args[n], XmNmessageString, string2 ), n++;
  XtSetArg(args[n], XmNdialogStyle,XmDIALOG_PRIMARY_APPLICATION_MODAL), n++;
  XtSetArg(args[n], XmNnoResize, TRUE), n++;
  XtSetArg(args[n], XmNdialogTitle, string2 ), n++;
 
  warningDialog = XmCreateWarningDialog( parent, "Warning Dialog", args, n);

  XmStringFree( string1 );
  XmStringFree( string2 );

  XtUnmanageChild(XmMessageBoxGetChild(warningDialog, XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(warningDialog, XmDIALOG_CANCEL_BUTTON));
  XtAddCallback(warningDialog, XmNokCallback, TerraVisionButtonCB, NULL);

  return warningDialog;
}

/*
 * Synopsis:
 *   void GenericErrorDialog( Widget shell, Widget *dialog, char const *msg )
 *
 * Description:
 *   If *dialog is NULL, then this function will creates an instance of
 *   an error dialog with the specified error message. The Widget instance
 *   for this dialog is then stored in *dialog.
 *
 *   If *dialog is non-NULL, then it is assumed that it contains a valid
 *   widget instance that was created by a previous call to this function.
 *   The function will then refresh this widget's label with the new
 *   one and pop the dialog up.
 *
 *   This function can be used to easily create any error dialog within 
 *   TerraVision. For an example use of this function, see Database.c.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
GenericErrorDialog( Widget shell, Widget *dialog, char const *msg )
{
  char       *title = "TerraVision Message";
  Widget     parent;
  XmString   xm_text, xm_title;

  /* If we have already created this widget, then just pop it up */

  if ( *dialog != NULL ) {

    xm_text = tvXmString( msg );
    XtVaSetValues( *dialog, XmNmessageString, xm_text, NULL );
    XmStringFree( xm_text );

    if ( ! XtIsManaged( *dialog ) )
      XtManageChild( *dialog );

    return;
  }

  /* Otherwise, we will create a new instance for this widget */

  xm_text  = tvXmString( msg );
  xm_title = tvXmString( title );

  parent = XtParent( shell );
  if ( parent == NULL ) parent = shell;
  *dialog = XmCreateMessageDialog( parent, title, NULL, 0 );

  XtVaSetValues( *dialog,
		 XmNdialogType, XmDIALOG_ERROR,
		 XmNdialogTitle, xm_title,
		 XmNmessageString, xm_text,
		 XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		 XmNnoResize, True,
		 XmNmessageAlignment, XmALIGNMENT_BEGINNING,
		 NULL );

  XmStringFree( xm_text );
  XmStringFree( xm_title );

  XtUnmanageChild( XmMessageBoxGetChild( *dialog, XmDIALOG_CANCEL_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( *dialog, XmDIALOG_HELP_BUTTON ) );
  XtAddCallback( *dialog, XmNokCallback, GenericUnmanageCB, NULL);
  XtManageChild( *dialog );
}

/*
 * Synopsis:
 *   void GenericConfirmDialog( Widget shell, Widget *dialog, char *msg,
 *                              XtCallbackProc callback )
 *   
 * Description:
 *   If *dialog is NULL, then this function will create a confirm widget
 *   with the specified message and two buttons: a Yes and a No button.
 *   The callback function that you specified will be called whenever
 *   the user clicks over the Yes button. If the user clicks over the
 *   No button, then the window will simply be dismissed and no action
 *   will be happen.
 *
 *   If *dialog is non-NULL, then it is assumed that it contains a valid
 *   widget instance that was created by a previous call to this function.
 *   The function will then refresh this dialog's message with the new
 *   one, install the new callback function, and pop the dialog up.
 *
 *   This function can be used to easily create any confirmation dialogs
 *   in TerraVision. For an example use of this function, see Database.c.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
GenericConfirmDialog( Widget shell, Widget *dialog, char const *msg,
		      XtCallbackProc callback, XtPointer client_data )
{
  char     *title = "TerraVision Confirm";
  XmString label, name, yes, no;

  /* If we have already created this widget, then just pop it up */

  if ( *dialog != NULL ) {

    /* Install the new callback function, removing the previous one */
    
    XtRemoveAllCallbacks( *dialog, XmNokCallback );
    XtAddCallback( *dialog, XmNokCallback, callback, client_data );

    /* Set up the new label for the widget */

    label = tvXmString( msg );
    XtVaSetValues( *dialog, XmNmessageString, label, NULL );
    XmStringFree( label );

    /* And pop it up */

    if ( ! XtIsManaged( *dialog ) )
      XtManageChild( *dialog );

    return;
  }

  /* Otherwise we will create the new instance of the confirmation dialog */

  *dialog = XmCreateWarningDialog( shell, title, NULL, 0 );
  
  label = tvXmString( msg );
  name  = tvXmString( title );
  yes   = tvXmString( " Yes " );
  no    = tvXmString( " No " );

  XtVaSetValues( *dialog,
		 XmNmessageString,label,
		 XmNokLabelString, yes,
		 XmNcancelLabelString, no,
		 XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON,
		 XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		 XmNdialogType, XmDIALOG_QUESTION,
		 XmNmessageAlignment, XmALIGNMENT_CENTER,
		 XmNnoResize, True,
		 XmNdialogTitle, name,
		 NULL );

  XmStringFree( label );
  XmStringFree( name );
  XmStringFree( yes );
  XmStringFree( no );

  XtUnmanageChild( XmMessageBoxGetChild( *dialog, XmDIALOG_HELP_BUTTON ) );
  XtAddCallback( *dialog, XmNokCallback, callback, client_data );
  XtAddCallback( *dialog, XmNcancelCallback, GenericUnmanageCB, NULL );
  XtManageChild( *dialog );
}

/*
 * Synopsis:
 *   void GenericPromptDialog( Widget shell, Widget *dialog, char const *msg,
 *                             XtCallbackProc callback, XtPointer )
 *
 * Description:
 *   If *dialog is NULL, then this function will create a prompt widget
 *   with the specified label and okay and cancel buttons. The callback
 *   function that you specified will be called whenever the user clicks
 *   over the okay button, or presses return. If the user clicks over the
 *   cancel button, then the window will simply be dismissed and no action
 *   will be happen.
 *
 *   If *dialog is non-NULL, then it is assumed that it contains a valid
 *   widget instance that was created by a previous call to this function.
 *   The function will then refresh this dialog's label with the new
 *   one, install the new callback function, and pop the dialog up.
 *
 *   This function can be used to easily create any text prompt dialogs
 *   in TerraVision. For an example use of this function, see DatabaseOpts.c.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
GenericPromptDialog( Widget shell, Widget *dialog, char const *msg,
		     XtCallbackProc callback, XtPointer client_data )
{
  XmString label, name;
  Arg      args[10];
  int      n = 0;

  /* If we have already created this widget, then just pop it up */

  if ( *dialog != NULL ) {

    /* Install the new callback function, removing the previous one */
    
    XtRemoveAllCallbacks( *dialog, XmNokCallback );
    XtAddCallback( *dialog, XmNokCallback, callback, client_data );

    /* Set up the new label for the widget */

    label = tvXmString( msg );
    XtVaSetValues( *dialog, XmNselectionLabelString, label, NULL );
    XmStringFree( label );

    /* And pop it up */

    if ( ! XtIsManaged( *dialog ) )
      XtManageChild( *dialog );

    return;
  }

  /* Otherwise, let's create a new prompt dialog instance */

  label = tvXmString( msg );
  name  = tvXmString( "TerraVision Prompt" );

  XtSetArg( args[n], XmNselectionLabelString, label ); n++;
  XtSetArg( args[n], XmNautoUnmanage, False ); n++;
  XtSetArg( args[n], XmNdialogTitle, name ); n++;
  XtSetArg( args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL ); n++;
  XtSetArg( args[n], XmNtextColumns, 40 ); n++;
  XtSetArg( args[n], XmNnoResize, True ); n++;
  *dialog = XmCreatePromptDialog( shell, "TerraVision Prompt", args, n );

  XmStringFree( label );
  XmStringFree( name );
  
  XtAddCallback( *dialog, XmNokCallback, callback, client_data );
  XtAddCallback( *dialog, XmNcancelCallback, GenericUnmanageCB, NULL );
  XtUnmanageChild( XmSelectionBoxGetChild( *dialog, XmDIALOG_HELP_BUTTON ) );
  XtManageChild( *dialog );
}

/*
 * Synopsis:
 *   void GenericMessageDialog( Widget shell, Widget *dialog, char const *msg,
 *                             XtCallbackProc callback )
 *
 * Description:
 *   If *dialog is NULL, then this function will create a prompt widget
 *   with the specified label with only a cancel button. The callback
 *   function that you specified will be called whenever the user clicks
 *   over the cancel button, or presses return. 
 *
 *   If *dialog is non-NULL, then it is assumed that it contains a valid
 *   widget instance that was created by a previous call to this function.
 *   The function will then refresh this dialog's label with the new
 *   one, install the new callback function, and pop the dialog up.
 *
 *   This function can be used to easily create any text prompt dialogs
 *   in TerraVision. For an example use of this function, see DatabaseOpts.c.
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: Feb 19 1999
 *
 */

void
GenericMessageDialog( Widget shell, Widget *dialog, char const *msg,
		      XtCallbackProc callback)
{
  char       *title = "TerraVision Message";
  Widget     parent;
  XmString   xm_text, xm_title;

  /* If we have already created this widget, then just pop it up */

  if ( *dialog != NULL ) {

    xm_text = tvXmString( msg );
    XtVaSetValues( *dialog, XmNmessageString, xm_text, NULL );
    XmStringFree( xm_text );

    if ( ! XtIsManaged( *dialog ) )
      XtManageChild( *dialog );

    return;
  }

  /* Otherwise, we will create a new instance for this widget */

  xm_text  = tvXmString( msg );
  xm_title = tvXmString( title );

  parent = XtParent( shell );
  if ( parent == NULL ) parent = shell;
  *dialog = XmCreateMessageDialog( parent, title, NULL, 0 );

  XtVaSetValues( *dialog,
		 XmNdialogType, XmDIALOG_ERROR,
		 XmNdialogTitle, xm_title,
		 XmNmessageString, xm_text,
		 XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
		 XmNnoResize, True,
		 XmNmessageAlignment, XmALIGNMENT_BEGINNING,
		 NULL );

  XmStringFree( xm_text );
  XmStringFree( xm_title );

  /* turn off the OK and HELP buttons */
  XtUnmanageChild( XmMessageBoxGetChild( *dialog, XmDIALOG_OK_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( *dialog, XmDIALOG_HELP_BUTTON ) );
  XtAddCallback( *dialog, XmNcancelCallback, callback, NULL );
  XtManageChild( *dialog );
}


/*
 * Synopsis:
 *   void GenericUnmanageCB( Widget widget, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function simply does an XtUnmanageChild on the specified widget.
 *   This can be used as a X callback function when all you want a widget
 *   to do is dismiss itself when activated, e.g. a cancel button on a
 *   dialog window.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 27 August 1998
 *
 */

void
GenericUnmanageCB( Widget widget, XtPointer client_data, XtPointer call_data )
{
  if ( XtIsManaged( widget ) )
    XtUnmanageChild( widget );
}

/*
 * Synopsis:
 *   void LicenseAcceptedCB( Widget widget, XtPointer client_data, XtPointer )
 *   
 * Description:
 *   This callback is invoked whenever the user clicks over the "I Agree"
 *   button of the TerraVision license requester. It simply writes an
 *   updated version of the users .TerraVision file with an extra field
 *   specifying that the user has accepted license and giving the version
 *   of TerraVision that this applies to.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 2 September 1998
 *
 */

static void
LicenseAcceptedCB( Widget widget, XtPointer client_data, XtPointer call_data )
{

  /* Save the TerraVision config file with the license_accepted info */

  tvuAddConfig1( "license", TERRAVISION_RELEASE );
  tvuSaveConfig();

  /* Fall through to the generic callback to unmanage the requester */

  GenericUnmanageCB( (Widget) client_data, NULL, call_data );
}

/*
 * Synopsis:
 *   void LicenseRejectedCB( Widget widget, XtPointer client_data, XtPointer )
 *   
 * Description:
 *   This callback is invoked whenever the user clicks over the "I Disagree"
 *   button of the TerraVision license requester. It simply writes an
 *   updated version of the users .TerraVision file with the license field
 *   removed and then quits.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 September 1998
 *
 */

static void
LicenseRejectedCB( Widget widget, XtPointer client_data, XtPointer call_data )
{

  /* Save the TerraVision config file with the license_accepted info */

  tvuAddConfig1( "license", "no" );
  tvuSaveConfig();

  /* Close the requester window and exit TerraVision */

  GenericUnmanageCB( (Widget) client_data, NULL, call_data );
  TerraVisionQuit();
}

/*
 * Synopsis:
 *   void DismissLicenseCB( Widget widget, XtPointer client_data, XtPointer )
 *   
 * Description:
 *   This callback is invoked whenever the user clicks over the "Dismiss"
 *   button of the TerraVision license window, as displayed by ShowLicense.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 7 December 1998
 *
 */

static void
DismissLicenseCB( Widget widget, XtPointer client_data, XtPointer call_data )
{
  GenericUnmanageCB( (Widget) client_data, NULL, call_data );
}

/*
 * Synopsis:
 *   void ConfirmLicense( void )
 *   
 * Description:
 *  Calling this function will pop up a requester with the TerraVision
 *  license and ask the user if they want to accept or decline the
 *  license. If they decline then they are thrown out of TerraVision,
 *  if they accept then this fact is recorded in their .TerraVision file
 *  and they can continue as normal. The user cannot interact with the
 *  TerraVision system until they accept the license.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 2 September 1998
 *
 */

void
ConfirmLicense( void )
{
  char            *title = "TerraVision License";
  Widget          dialog, text_w, form, sep, widget, mainform;
  XmString        xm_title;
  Arg             args[10];
  int             n = 0;
  XmFontList      fontlist;
  XmFontListEntry fontentry;
  XFontStruct     *font;
  Display         *tvDisplay = XtDisplay(applicationShell);

  /* Create the form dialog to contain the license. Make sure that */
  /* the user cannot close the window and that they cannot use any */
  /* any other part of the application until they answer.          */

  xm_title = tvXmString( title );
  XtSetArg( args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL ); n++;
  XtSetArg( args[n], XmNdialogTitle, xm_title ); n++;
  XtSetArg( args[n], XmNdeleteResponse, XmDO_NOTHING ); n++;
  dialog = XmCreateFormDialog( applicationShell, title, args, n );

  XmStringFree( xm_title );

  /* Create a form widget to fill the entire dialog */

  mainform = XtVaCreateWidget( "mainform", xmFormWidgetClass, dialog,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       NULL );

  /* Create a form to contain the list widget containing the license text */

  form = XtVaCreateWidget( "form1", xmFormWidgetClass, mainform,
			   XmNbottomOffset, 5,
			   XmNtopOffset, 5,
			   XmNleftOffset, 5,
			   XmNrightOffset, 5,
			   NULL );

  /* grab a fixed-width font to format the license text with */

  font = XLoadQueryFont( tvDisplay, "-*-courier-medium-r-*--14-*" );
  fontentry = XmFontListEntryCreate( "tag1", XmFONT_IS_FONT, (XtPointer)font );
  fontlist = XmFontListAppendEntry( NULL, fontentry );

  /* Create the scrolled text widget containing the license text */

  n = 0;
  XtSetArg( args[n], XmNscrollVertical, True ); n++;
  XtSetArg( args[n], XmNscrollHorizontal, True ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNwordWrap, False ); n++;
  XtSetArg( args[n], XmNvalue, license_text ); n++;
  XtSetArg( args[n], XmNrows, 30 ); n++;
  XtSetArg( args[n], XmNcolumns, 70 ); n++;
  XtSetArg( args[n], XmNfontList, fontlist ); n++;
  text_w = XmCreateScrolledText( form, "license", args, n );

  XtVaSetValues( XtParent(text_w), XmNleftAttachment, XmATTACH_FORM,
		 XmNtopAttachment, XmATTACH_FORM, 
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_FORM, NULL );

  /* Put a separator between the text and the buttons */

  sep = XtVaCreateManagedWidget( "separator", 
				 xmSeparatorGadgetClass, mainform,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNbottomOffset, 5,
				 NULL );

  XtVaSetValues( form, XmNleftAttachment, XmATTACH_FORM,
		 XmNtopAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, sep, NULL );
  XtManageChild( text_w );
  XtManageChild( form );

  /* Create another form to act as the action area for the dialog */

  form = XtVaCreateWidget( "form2", xmFormWidgetClass, mainform,
			   XmNleftAttachment, XmATTACH_FORM,
			   XmNrightAttachment, XmATTACH_FORM,
			   XmNbottomAttachment, XmATTACH_FORM,
			   XmNbottomOffset, 5,
			   XmNtopOffset, 5,
			   XmNleftOffset, 5,
			   XmNrightOffset, 5,
			   NULL );

  XtVaSetValues( sep, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, form, NULL );

  /* Create the "I Agree" button */

  widget = XtVaCreateManagedWidget( " I Agree ",
				    xmPushButtonGadgetClass, form,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNshowAsDefault, False,
				    XmNdefaultButtonShadowThickness, 1,
				    XmNmarginTop, 3,
				    XmNmarginBottom, 3,
				    XmNmarginLeft, 5,
				    XmNmarginRight, 5,
				    NULL );
  XtAddCallback( widget, XmNactivateCallback, LicenseAcceptedCB, dialog );
  
  /* Create the "I Disagree" button */

  widget = XtVaCreateManagedWidget( " I Disagree ",
				    xmPushButtonGadgetClass, form,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNshowAsDefault, True,
				    XmNdefaultButtonShadowThickness, 1,
				    XmNmarginTop, 3,
				    XmNmarginBottom, 3,
				    XmNmarginLeft, 5,
				    XmNmarginRight, 5,
				    NULL );
  XtAddCallback( widget, XmNactivateCallback, LicenseRejectedCB, dialog );

  /* Display everything... */

  XtManageChild( form );
  XtManageChild( mainform );
  XtManageChild( dialog );
}

/*
 * Synopsis:
 *   void ShowLicense( void )
 *   
 * Description:
 *  Calling this function will pop up a window with the TerraVision
 *  license. This is for viewing purposes only.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 7 December 1998
 *
 */

void
ShowLicense( void )
{
  char            *title = "TerraVision License";
  static Widget   dialog = NULL;
  Widget          text_w, form, sep, widget, mainform;
  XmString        xm_title;
  Arg             args[10];
  int             n = 0;
  XmFontList      fontlist;
  XmFontListEntry fontentry;
  XFontStruct     *font;
  Display         *tvDisplay = XtDisplay(applicationShell);

  /* If we have already created the dialog, then just pop it up */

  if ( dialog != NULL ) {
    XtManageChild( dialog );
    return;
  }

  /* Create the form dialog to contain the license. Make sure that */
  /* the user cannot close the window and that they cannot use any */
  /* any other part of the application until they answer.          */

  xm_title = tvXmString( title );
  XtSetArg( args[n], XmNdialogTitle, xm_title ); n++;
  /*XtSetArg( args[n], XmNdeleteResponse, XmDO_NOTHING ); n++;*/
  dialog = XmCreateFormDialog( applicationShell, title, args, n );

  XmStringFree( xm_title );

  /* Create a form widget to fill the entire dialog */

  mainform = XtVaCreateWidget( "mainform", xmFormWidgetClass, dialog,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       NULL );

  /* Create a form to contain the list widget containing the license text */

  form = XtVaCreateWidget( "form1", xmFormWidgetClass, mainform,
			   XmNbottomOffset, 5,
			   XmNtopOffset, 5,
			   XmNleftOffset, 5,
			   XmNrightOffset, 5,
			   NULL );

  /* grab a fixed-width font to format the license text with */

  font = XLoadQueryFont( tvDisplay, "-*-courier-medium-r-*--14-*" );
  fontentry = XmFontListEntryCreate( "tag1", XmFONT_IS_FONT, font );
  fontlist = XmFontListAppendEntry( NULL, fontentry );

  /* Create the scrolled text widget containing the license text */

  n = 0;
  XtSetArg( args[n], XmNscrollVertical, True ); n++;
  XtSetArg( args[n], XmNscrollHorizontal, True ); n++;
  XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
  XtSetArg( args[n], XmNeditable, False ); n++;
  XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
  XtSetArg( args[n], XmNwordWrap, False ); n++;
  XtSetArg( args[n], XmNvalue, license_text ); n++;
  XtSetArg( args[n], XmNrows, 30 ); n++;
  XtSetArg( args[n], XmNcolumns, 70 ); n++;
  XtSetArg( args[n], XmNfontList, fontlist ); n++;
  text_w = XmCreateScrolledText( form, "license", args, n );

  XtVaSetValues( XtParent(text_w), XmNleftAttachment, XmATTACH_FORM,
		 XmNtopAttachment, XmATTACH_FORM, 
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_FORM, NULL );

  /* Put a separator between the text and the buttons */

  sep = XtVaCreateManagedWidget( "separator", 
				 xmSeparatorGadgetClass, mainform,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNbottomOffset, 5,
				 NULL );

  XtVaSetValues( form, XmNleftAttachment, XmATTACH_FORM,
		 XmNtopAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, sep, NULL );
  XtManageChild( text_w );
  XtManageChild( form );

  /* Create another form to act as the action area for the dialog */

  form = XtVaCreateWidget( "form2", xmFormWidgetClass, mainform,
			   XmNleftAttachment, XmATTACH_FORM,
			   XmNrightAttachment, XmATTACH_FORM,
			   XmNbottomAttachment, XmATTACH_FORM,
			   XmNbottomOffset, 5,
			   XmNtopOffset, 5,
			   XmNleftOffset, 5,
			   XmNrightOffset, 5,
			   NULL );

  XtVaSetValues( sep, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, form, NULL );

  /* Create the "Dismiss" button */

  widget = XtVaCreateManagedWidget( " Dismiss ",
				    xmPushButtonGadgetClass, form,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNshowAsDefault, False,
				    XmNdefaultButtonShadowThickness, 1,
				    XmNmarginTop, 3,
				    XmNmarginBottom, 3,
				    XmNmarginLeft, 5,
				    XmNmarginRight, 5,
				    NULL );
  XtAddCallback( widget, XmNactivateCallback, DismissLicenseCB, dialog );
  
  /* Display everything... */

  XtManageChild( form );
  XtManageChild( mainform );
  XtManageChild( dialog );
}
