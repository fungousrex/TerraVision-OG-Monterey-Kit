/*
 * Name: NetLogger.c
 *
 * Description: Tile lifespan logging implemented with LBL's NetLogger
 *
 * Function List:
 *   void NetLogShow( void );
 *   void NetLogInit( Widget );
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

#include <prnetdb.h> 
#include <prsystem.h>

#include "NetLogCore.h"
#include "NetLogger.h"
#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/SeparatoG.h>

Widget NetLogWidget = NULL;
Widget hostForm = NULL;
Widget hostWidget = NULL;
Widget fileForm = NULL;
Widget fileWidget = NULL;

static void NetLogGetCurrentStrings( void );
static void NetLogDismissCB( Widget, XtPointer, XtPointer );
static void NetLogRadioCB( Widget, XtPointer, XtPointer );

/*
 * Synopsis: 
 *   void NetLogShow( void )
 *
 * Description:
 *   Displays the NetLogger Options widget to enable the user to select
 *   the logging output method etc. N.B. you must have called NetLogInit
 *   first.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

void
NetLogShow( void )
{
  if ( XtIsManaged( NetLogWidget ) ) return;
  XtManageChild( NetLogWidget );
}

/*
 * Synopsis: 
 *   void NetLogGetCurrentStrings( void )
 *
 * Description:
 *   Gets the current string values for the directory and hostname
 *   widgets of the NetLogger window. We store these in the strings
 *   NetLogDirectory and NetLogHostname, respectively.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 7 April 1999
 *
 */

static void
NetLogGetCurrentStrings( void )
{
  String temp;

  XtVaGetValues( fileWidget, XmNvalue, &temp, NULL );
  strcpy( NetLogDirectory, temp ); XtFree (temp);
  XtVaGetValues( hostWidget, XmNvalue, &temp, NULL );
  strcpy( NetLogHostname, temp ); XtFree (temp);
}

/*
 * Synopsis: 
 *   void NetLogDismissCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   A callback function that is used whenever the user clicks over
 *   the dismiss button of the NetLogger Options widget. Here we
 *   hide the window and update our copies of the directory and host
 *   name strings.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

static void
NetLogDismissCB( Widget parent, XtPointer client_data, XtPointer call_data )
{
  if ( ! XtIsManaged( NetLogWidget ) ) return;

  NetLogGetCurrentStrings();

  XtUnmanageChild( NetLogWidget );
}

/*
 * Synopsis: 
 *   void NetLogRadioCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   A callback function that is used whenever the user clicks over
 *   one of the radio buttons to choose the different NetLogger methods.
 *   Here we record the new setting and ghost out either or both of the 
 *   text widgets dependent upon the mode.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

static void
NetLogRadioCB( Widget parent, XtPointer client_data, XtPointer call_data )
{
  switch ( (int) client_data ) {
  case NL_FILE:
    XtSetSensitive( fileForm, True );
    XtSetSensitive( hostForm, False );
    NetLogMethod = NL_FILE;
    break;
  case NL_HOST:
    XtSetSensitive( fileForm, False );
    XtSetSensitive( hostForm, True );
    NetLogMethod = NL_HOST;
    break;
  case NL_SYSLOG:
    XtSetSensitive( fileForm, False );
    XtSetSensitive( hostForm, False );
    NetLogMethod = NL_SYSLOG;
    break;
  }
}

/*
 * Synopsis: 
 *   void NetLogInit( Widget parent )
 *
 * Description:
 *   This function does all of the required initialisation for the 
 *   NetLogger widget. It builds the NetLogger Option window, ready to
 *   by just managed on demand.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

void
NetLogInit( Widget parent )
{
  Widget         separator, dismiss, button;
  Widget         toggle_box;
  Widget         frame, label, workForm;
  XmString       name, btn1, btn2, btn3;
  char           hostname[512];
  char const     *logtype = NULL;
  Arg            args[10];
  int            n;
  
  /* Create the main dialog widget */
  
  n = 0;
  name = tvXmString("NetLogger Options");
  XtSetArg( args[n], XmNdialogTitle, name ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  NetLogWidget = XmCreateFormDialog( parent, "threeDPanelForm", args, n );
  XmStringFree (name);

  /* create a sub Form widget so that we can play with the spacing */

  n = 0;
  XtSetArg( args[n], XmNverticalSpacing, 10 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 10 ); n++;
  workForm = XmCreateForm( NetLogWidget, "threeDPanelForm", args, n );
 
  /* Create a frame for the radio box */

  n = 0;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_ETCHED_IN ); n++;
  frame = XmCreateFrame( workForm, "Frame", args, n );
  XtManageChild( frame );

  /* set the default log output from the netloggerLogTo resource */

  tvGetResourceAsString( "netloggerLogTo", &logtype );
  NetLogMethod = NL_FILE;
  if ( logtype != NULL ) {
    if ( strcasecmp( logtype, "host" ) == 0 )
      NetLogMethod = NL_HOST;
    else if ( strcasecmp( logtype, "syslog" ) == 0 )
      NetLogMethod = NL_SYSLOG;
  }

  /* Create the radio box */

  btn1 = XmStringCreateLocalized( "Log to file" );
  btn2 = XmStringCreateLocalized( "Log to syslog" );
  btn3 = XmStringCreateLocalized( "Log to host" );
  toggle_box = XmVaCreateSimpleRadioBox ( frame,
        "Radio Box", NetLogMethod, NetLogRadioCB,
        XmVaRADIOBUTTON, btn1, 0, NULL, NULL,
        XmVaRADIOBUTTON, btn2, 0, NULL, NULL,
	XmVaRADIOBUTTON, btn3, 0, NULL, NULL,
	XmNleftAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
        NULL);
  XtManageChild( toggle_box );
  XmStringFree( btn1 );
  XmStringFree( btn2 );
  XmStringFree( btn3 );

  /* the filename text widget */

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, frame ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  fileForm = XmCreateForm( workForm, "Sub Form 1", args, n );
  XtManageChild( fileForm );

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( fileForm, "netloggerDirectoryLabel", args, n );
  XtManageChild( label );

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, label ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNcolumns, 30 ); n++;
  fileWidget = XmCreateText( fileForm, "netloggerDirectory", args, n );
  XtManageChild( fileWidget );

  /* the hostname text widget */

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, fileForm ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  hostForm = XmCreateForm( workForm, "Sub Form 2", args, n );
  XtManageChild( hostForm );

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNalignment, XmALIGNMENT_BEGINNING ); n++;
  label = XmCreateLabel( hostForm, "netloggerHostnameLabel", args, n );
  XtManageChild( label );

  n = 0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, label ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNcolumns, 30 ); n++;
  hostWidget = XmCreateText( hostForm, "netloggerHostname", args, n );
  XtManageChild( hostWidget );
  /* XtSetSensitive( hostForm, False ); */

  separator = XtVaCreateManagedWidget( "formSeparator1",
				       xmSeparatorGadgetClass,
				       NetLogWidget,
				       XmNshadowThickness, 4,
				       XmNtopAttachment, XmATTACH_WIDGET,
				       XmNtopWidget, workForm,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNorientation, XmHORIZONTAL,
				       NULL);

  dismiss =  XtVaCreateManagedWidget( "ThreeDPanelBoard",
				      xmRowColumnWidgetClass,
				      NetLogWidget,
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

  XtVaSetValues( separator,
		 XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, dismiss,
		 NULL);

    /* CREATE THE DISMISS BUTTON */
  name = tvXmString("Dismiss");
  button = XtVaCreateManagedWidget("ThreeDPanelDismiss",
				   xmPushButtonGadgetClass,
				   dismiss,
				   XmNleftOffset, 30,
				   XmNrightOffset, 20,
				   XmNshowAsDefault, 1,
				   XmNlabelString, name,
				   NULL);
  XmStringFree (name);
  XtAddCallback( button, XmNactivateCallback, NetLogDismissCB, NULL );

  /* set the correct sensitivity for the text widgets depending */
  /* upon the default Log To mode                               */

  NetLogRadioCB( NULL, (XtPointer) NetLogMethod, NULL );

  /* Get the initial values for the text widgets. These can be changed */
  /* by the user bu setting the *value resources for each widget in    */
  /* their .TerraVision file. Default settings are in TerraVision.ad.h */

  NetLogGetCurrentStrings();

  /* try to look up the full hostname for the current machine */
  /* update the host widget with the full name if available.  */

  if (PR_GetSystemInfo (PR_SI_HOSTNAME, hostname, 512) == PR_SUCCESS) {
    PRHostEnt host;
    char netbuf[PR_NETDB_BUF_SIZE];
    if ( ( PR_GetHostByName (hostname, netbuf, PR_NETDB_BUF_SIZE, &host) )
	 == PR_SUCCESS ) {
      strcpy( NetLogHostname, host.h_name );
      XtVaSetValues( hostWidget, XmNvalue, NetLogHostname, NULL );
    }
  }

  /* finally, we manage the entire dialog */

  XtManageChild( workForm );
}
