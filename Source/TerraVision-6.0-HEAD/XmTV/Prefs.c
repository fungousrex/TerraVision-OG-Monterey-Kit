/*
 * Name: Prefs.c
 *
 * Description: Routines to control the general TerraVision preferences
 *    settings. These are accessed throught the Prefs menu on the
 *    main TerraVision menu bar.
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
#include "ThreeDCore.h"
#include "TsGeoPyramid.h"
#include "Prefs.h"
#include "Database.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#include <stdlib.h>

static Widget dpss_dialog = NULL;                  /* the DPSS prefs window */
static float  dpss_timeout = 0.3;       /* the DPSS request timeout setting */


static void PrefsDPSSReadCB( Widget, XtPointer, XtPointer );
static void PrefsDPSSTimeoutCB( Widget, XtPointer, XtPointer );
static void PrefsInitDPSS( Widget parent );


/*
 * Synopsis:
 *   void PrefsDPSSTimeoutCB( Widget w, XtPointer client_data, XtPointer call )
 *
 * Description:
 *   This callback is invoked whenever the user changes the DPSS request
 *   list timeout from the DPSS Preferences window. It will work out the
 *   new value and then set the request rate for all of the currently
 *   available connections in all geos.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 4 May 1999
 *
 */

static void
PrefsDPSSTimeoutCB( Widget w, XtPointer client_data, XtPointer call )
{
  static char  *funcName = "PrefsDPSSTimeoutCB";
  list_iter_t gi;

  /* get the new value for DPSS request delay from the text widget */

  dpss_timeout = atof( XmTextFieldGetString( w ) );
  
  if ( dpss_timeout < 0.001 ) dpss_timeout = 0.001;

  TSMTRACE(( TSMMASK|1, funcName, "Setting new DPSS request timeout: %f",
	     dpss_timeout ));

  for (gi = list_first (geoList); ! list_done (gi); gi = list_next (gi)) {
    TsGeoPyramid *geo = list_value (gi);
    TsmConnection cons[2];
    int           numb_cons, i;

    /* get all of the connections for this geo and set the timeout */
    numb_cons = TsGeoConnections( geo, cons );
    for ( i = 0; i < numb_cons; i++ )
      tsmSet( cons[i], TSM_REQUEST_TIMEOUT, dpss_timeout );
  }
}

/*
 * Synopsis:
 *   void PrefsDPSSReadCB( Widget w, XtPointer client_data, XtPointer call )
 *
 * Description:
 *   This callback is invoked whenever the user changes the toggle
 *   button for full DPSS server reads from the Prefs/DPSS dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 8 July 1999
 *
 */

static void
PrefsDPSSReadCB( Widget w, XtPointer client_data, XtPointer call )
{
  /*static char const funcName[] = "PrefsDPSSReadCB";*/
  DBaseSetFullDPSSRead( XmToggleButtonGadgetGetState(w) );
}

/*
 * Synopsis:
 *   void PrefsInitDPSS( Widget parent )
 *
 * Description:
 *   Sets up all of the widgets for the DPSS Preferences window.
 *   Currently, this just lets you change the DPSS request timeout.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 4 May 1999
 *
 */

static void
PrefsInitDPSS( Widget parent )
{
  Widget    pane, form, widget, label, sep, toggle;
  Arg       args[15];
  int       n = 0;
  XmString  title;
  char      buffer[32];

  n = 0;
  title = tvXmString("DPSS Preferences");
  XtSetArg( args[n], XmNdialogTitle, title ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  dpss_dialog = XmCreateFormDialog( parent, "DPSS Preferences", args, n );
  XmStringFree( title );

  /* Create a form widget to manage the stuff in this dialog. */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 4 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 4 ); n++;
  pane = XtCreateWidget( "pane", xmFormWidgetClass, dpss_dialog, args, n );
  XtManageChild( pane );

  toggle = XtVaCreateManagedWidget( "Full DPSS server reads",
				    xmToggleButtonGadgetClass, pane,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNindicatorOn, True,
				    NULL );

  XtAddCallback( toggle, XmNvalueChangedCallback, PrefsDPSSReadCB, NULL );

  label = XtVaCreateManagedWidget( "Request Delay (secs) : ",
				   xmLabelGadgetClass, pane,
				   XmNtopOffset, 17,
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   NULL );

  sprintf( buffer, "%1.1f", dpss_timeout );
  widget = XtVaCreateManagedWidget( "threeDPanelTimeoutWidget",
				    xmTextFieldWidgetClass, pane,
				    XmNtopOffset, 10,
				    XmNcolumns, 3,
				    XmNvalue, buffer,
				    XmNleftAttachment, XmATTACH_WIDGET,
				    XmNleftWidget, label,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, toggle,
				    XmNbottomAttachment, XmATTACH_FORM,
				    NULL );

  XtAddCallback( widget, XmNvalueChangedCallback, PrefsDPSSTimeoutCB, NULL );

  /* Create a separator bar */

  sep = XtVaCreateManagedWidget( "formSeparator1",
				 xmSeparatorGadgetClass, dpss_dialog,
				 XmNshadowThickness, 4,
				 XmNleftAttachment, XmATTACH_FORM,
				 XmNrightAttachment, XmATTACH_FORM,
				 XmNorientation, XmHORIZONTAL,
				 XmNbottomOffset, 3,
				 NULL );

  XtVaSetValues( pane, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, sep, NULL );

  /* Create another form to act as the action area for the dialog */

  n = 0;
  XtSetArg( args[n], XmNfractionBase, 3 ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  form = XtCreateWidget( "form2", xmFormWidgetClass, dpss_dialog, args, n );
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
  XtAddCallback( widget, XmNactivateCallback, (XtCallbackProc) PrefsHideDPSS,
		 NULL );
}

/*
 * Synopsis:
 *   void PrefsShowDPSS( Widget parent )
 *
 * Description:
 *   Opens the DPSS Preferences window, if not already open.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 4 May 1999
 *
 */

void
PrefsShowDPSS( Widget parent )
{
  if ( dpss_dialog == NULL ) 
    PrefsInitDPSS( parent );

  if ( dpss_dialog && ! XtIsManaged( dpss_dialog ) )
    XtManageChild( dpss_dialog );
}

/*
 * Synopsis:
 *   void PrefsHideDPSS( void )
 *
 * Description:
 *   Closes the DPSS Preferences window, if open.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 4 May 1999
 *
 */

void
PrefsHideDPSS( void )
{
  if ( dpss_dialog && XtIsManaged( dpss_dialog ) )
    XtUnmanageChild( dpss_dialog );
}
