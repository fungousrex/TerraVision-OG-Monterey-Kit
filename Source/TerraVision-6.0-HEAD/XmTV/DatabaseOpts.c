/*
 * Name: DatabaseOpts.c
 *
 * Description: 
 *     Lets the user manage the servers that are defined in the currently
 *     loaded global dataset. This includes adding and removing servers,
 *     as well as updating the list of datasets from those servers.
 *
 * Function List:
 *   void DBaseInitOptions( Widget parent )
 *   void DBaseAddCallback( Widget widget, XtPointer client_data, XtPointer )
 *   void DBaseDelCallback( Widget w, XtPointer client_data, XtPointer )
 *   void DBaseShowOpenTV( void )
 *   void DBaseHideOpenTV( void )
 *   void DBaseCallbackOpts( Widget w, XtPointer client_data, XtPointer )
 *   void DBaseOptsRefreshAll( void )
 *   void DBaseReadServerThread( BusyDialog *busy )
 *   void DBaseReadServerDone( void *closure );
 *   void DBaseOptionActions( Widget w, XtPointer client_data, XtPointer )
 *   void DBaseRefreshOpts( void )
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Trace.h"
#include "Database.h"
#include "Dialogs.h"
#include "BusyDialog.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>

typedef struct {
  char       *server_url;        /* name of current TSM server */
  int        curr_server;        /* current number of this server */
  int        total_servers;      /* number of servers we are updating */
  BusyDialog *busy;              /* our busy dialog structure */
} DBaseUpdateClosure;

static Widget db_dialog = NULL;
static Widget db_server_list = NULL;
static Widget db_add_server = NULL;
static Widget db_remove_server = NULL;
static Widget db_move_up = NULL;
static Widget db_move_down = NULL;
static Widget db_reread_server = NULL;
static Widget db_reread_all = NULL;
static Widget db_info = NULL;
static Widget db_add_dialog = NULL;
static Widget db_remove_dialog = NULL;

static TsmConnectionParams reread_param;

static void DBaseCallbackOpts( Widget, XtPointer, XtPointer );
static void DBaseOptionActions( Widget, XtPointer, XtPointer );
static void DBaseOptsRefreshAll( int );
static void DBaseAddCallback( Widget w, XtPointer client_data, XtPointer );
static void DBaseDelCallback( Widget w, XtPointer client_data, XtPointer );
static void DBaseReadServerThread( BusyDialog *busy );
static void DBaseReadServerDone( void *closure );
static void DBaseReadServerCB( Widget w, XtPointer, XtPointer );
static void DBaseReadServer( void );
static void DBaseUpdateCB( char const *name, float percent, void *closure );

/* a macro wrapper to the generic error report dialog in Dialogs.c */

static Widget db_error = NULL;

#define DBaseOptsError(x) GenericErrorDialog( db_dialog, &db_error, x )

/*
 * Synopsis:
 *   void DBaseInitOptions( Widget parent )
 *
 * Description:
 *   This function sets up all of the widgets for the Database
 *   options widget. This lets you manage the servers that are
 *   stored in the database and update the database contents
 *   from these servers.
 *
 *   This function is called by DBaseInit() [Database.c], and
 *   should not be called directly.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseInitOptions( Widget parent )
{
  char     *title = "Dataset Options";
  Widget   form, widget, separator, buttonform, dismissform;
  Widget   frame, frameform;
  XmString name;
  Arg      args[15];
  int      n = 0;

  /* Create the main dialog window for the dataset selection */

  name = tvXmString( title );
  XtSetArg( args[n], XmNdialogTitle, name ); n++;
  db_dialog = XmCreateFormDialog( parent, title, args, n );
  XmStringFree( name );

  /* At the very top of the window we have a text widget with */
  /* the location of the global database, and then a button   */
  /* beside this to access the database "Options" window.     */
  
  form = XtVaCreateManagedWidget( "mainForm", 
				  xmFormWidgetClass, db_dialog,
				  XmNtopAttachment, XmATTACH_FORM,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  XmNhorizontalSpacing, 10,
				  XmNverticalSpacing, 10,
				  NULL );

  buttonform = XtVaCreateManagedWidget( "mainForm", 
					xmFormWidgetClass, form,
					XmNtopAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					XmNbottomAttachment, XmATTACH_FORM,
					NULL );

  /* First lets create the batch of buttons that we will */
  /* be using to manipulate the contents of the server list */

  name = tvXmString("Add Server");
  db_add_server =
    XtVaCreateManagedWidget( "Add Server", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);

  name = tvXmString("Remove Server");
  db_remove_server =
    XtVaCreateManagedWidget( "Remove Server", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, db_add_server,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);

  name = tvXmString("Move Up");
  db_move_up =
    XtVaCreateManagedWidget( "Move Up", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, db_remove_server,
			     XmNlabelString, name,
			     XmNtopOffset, 10,
			     NULL );
  XmStringFree (name);

  name = tvXmString("Move Down");
  db_move_down =
    XtVaCreateManagedWidget( "Move Down", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, db_move_up,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);

  name = tvXmString("Re-read Server");
  db_reread_server =
    XtVaCreateManagedWidget( "Reread Server", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, db_move_down,
			     XmNlabelString, name,
			     XmNtopOffset, 10,
			     NULL );
  XmStringFree (name);

  name = tvXmString("Re-read All");
  db_reread_all =
    XtVaCreateManagedWidget( "Reread All", 
			     xmPushButtonGadgetClass, buttonform,
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_WIDGET,
			     XmNtopWidget, db_reread_server,
			     XmNlabelString, name,
			     NULL );
  XmStringFree (name);

  /* Add callbacks to all of the buttons so that we can act upon them */

  XtAddCallback( db_add_server, XmNactivateCallback, DBaseOptionActions, 
		 NULL );
  XtAddCallback( db_remove_server, XmNactivateCallback, DBaseOptionActions,
		 NULL );
  XtAddCallback( db_move_up, XmNactivateCallback, DBaseOptionActions,
		 NULL );
  XtAddCallback( db_move_down, XmNactivateCallback, DBaseOptionActions,
		 NULL );
  XtAddCallback( db_reread_server, XmNactivateCallback, DBaseOptionActions,
		 NULL );
  XtAddCallback( db_reread_all, XmNactivateCallback, DBaseOptionActions,
		 NULL );

  /* We have a little display at the bottom of the window to tell */
  /* us the last time that the currently selected server was contacted */

  frameform = XtVaCreateManagedWidget( "frameForm", 
				       xmFormWidgetClass, db_dialog,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       XmNhorizontalSpacing, 10,
				       XmNbottomOffset, 10,
				       NULL );

  frame = XtVaCreateManagedWidget( "frame",
				   xmFrameWidgetClass, frameform,
				   XmNshadowType, XmSHADOW_IN,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNbottomAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   NULL );

  name = tvXmString(" ");
  db_info = XtVaCreateManagedWidget( "timestamp",
				     xmLabelGadgetClass, frame,
				     XmNlabelString, name,
				     XmNleftAttachment, XmATTACH_FORM,
				     NULL );
  XmStringFree (name);

  /* Now create the Dismiss button area and cordon this off with */
  /* one of those pretty separator bar things.                   */

  separator = XtVaCreateManagedWidget( "separator", 
				       xmSeparatorGadgetClass, db_dialog,
				       XmNleftAttachment, XmATTACH_FORM,
				       XmNrightAttachment, XmATTACH_FORM,
				       NULL );

  dismissform = XtVaCreateManagedWidget( "dissmissForm", 
					 xmFormWidgetClass, db_dialog,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNrightAttachment, XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_FORM,
					 XmNhorizontalSpacing, 10,
					 XmNverticalSpacing, 10,
					 NULL );

  XtVaSetValues( separator, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, dismissform, NULL );

  name = tvXmString("Dismiss");
  widget =
    XtVaCreateManagedWidget( "Dismiss",
			     xmPushButtonGadgetClass, dismissform,
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
  XtAddCallback( widget, XmNactivateCallback,
		 (XtCallbackProc) DBaseHideOptions, NULL );


  /* Now that we have all of the dependent widgets created, let's */
  /* make the list view which will contain all of the servers.    */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNrightWidget, buttonform ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNlistSizePolicy, XmCONSTANT ); n++; 
  XtSetArg( args[n], XmNselectionPolicy, XmBROWSE_SELECT ); n++;
  XtSetArg( args[n], XmNwidth, 300 ); n++;
  XtSetArg( args[n], XmNheight, 250 ); n++;

  db_server_list = XmCreateScrolledList( form, "Servers", args, n );
  XtAddCallback( db_server_list, XmNbrowseSelectionCallback,
		 DBaseCallbackOpts, NULL );
  XtManageChild( db_server_list );

  XtVaSetValues( form, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, frameform, NULL );
  XtVaSetValues( frameform, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, separator, NULL );

}

/*
 * Synopsis:
 *   void DBaseAddCallback( Widget widget, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function is called whenever the user enters a new server URL
 *   and presses Okay in the Add Server requester. It will check that
 *   the URL is valid, and then add this to the current database. The
 *   updated database will be saved back to disk and the various GUI
 *   widgets are refreshed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 August 1998
 *
 */

void
DBaseAddCallback( Widget widget, XtPointer client_data, XtPointer call_data )
{
  char     *selection;
  XmString text_string;
  Purl     *purl;

  /* Get the string that the user typed in */

  XtVaGetValues( widget, XmNtextString, &text_string, NULL );
  XmStringGetLtoR( text_string, XmFONTLIST_DEFAULT_TAG, &selection );

  XtUnmanageChild( widget );

  /* Check that the string is a valid (non-relative) URL */
  
  if ( selection == NULL || selection[0] == '\0' )
    return;

  if ( ( purl = tsmParseUrl( selection ) ) == NULL ) {
    DBaseOptsError( "You did not enter a valid URL!" );
    return;
  }
  tsmFreePurl( purl );

  if ( selection[0] != '/' && selection[0] != '~' && !strchr( selection,':' )){
    DBaseOptsError( "Relative URLs are not allowed - make it absolute" );
    return;
  }

  /* All okay, so let's add the new server entry to the database */

  tsmAddServerToDatabase( tv_database.database, selection );
  tv_database.db_selected = tv_database.database->numServers - 1;
  DBaseOptsRefreshAll( FALSE );

  /* Ask if the user wants to read the contents of that server now */

  GenericConfirmDialog( db_dialog, &db_remove_dialog, "Would you like to"
			" read this server now?", DBaseReadServerCB, NULL );
}

/*
 * Synopsis:
 *   void DBaseDelCallback( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function is called whenever the user confirms that they wish
 *   to remove the currently selected server from the database. It will
 *   check that we still have a valid server selected and then proceed
 *   to remove the server, and all datasets on that server, from the
 *   database. The updated database will be saved back to disk and the
 *   various GUI widgets are refreshed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 August 1998
 *
 */

void
DBaseDelCallback( Widget w, XtPointer client_data, XtPointer call_data )
{
  TsmServer *server;

  if ( tv_database.database == NULL || tv_database.db_selected < 0 )
    return;

  /* Get the currently selected TsmServer entry and remove it */

  server = &tv_database.database->server[tv_database.db_selected];

  tsmRemoveServerSetsFromDatabase( tv_database.database, server, NULL );
  tsmRemoveServerFromDatabase( tv_database.database, server->location->url );

  /* Now refresh the display */

  tv_database.db_selected = -1;

  DBaseOptsRefreshAll( FALSE );
}

/*
 * Synopsis:
 *   void DBaseShowOpenTV( void )
 *
 * Description:
 *   This function will display the Database Options window, if it is not
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
DBaseShowOptions( void )
{
  if ( db_dialog == NULL ) return;

  if ( ! XtIsManaged( db_dialog ) ) {
    DBaseRefreshOpts();
    XtManageChild( db_dialog );
  }

}

/*
 * Synopsis:
 *   void DBaseHideOpenTV( void )
 *
 * Description:
 *   This function will dismiss the Dabase Options window if it is currently
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
DBaseHideOptions( void )
{
  if ( db_dialog == NULL ) return;

  if ( XtIsManaged( db_dialog ) )
    XtUnmanageChild( db_dialog );
}

/*
 * Synopsis:
 *   void DBaseCallbackOpts( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This callback is invoked whenever the user selects a new server
 *   in the list of servers of the Database Options window. All we need
 *   to do here is record the new server selection and refresh the
 *   display of when the server was last contacted.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseCallbackOpts( Widget w, XtPointer client_data, XtPointer call_data )
{
  XmListCallbackStruct *list_cbs = (XmListCallbackStruct *) call_data;

  tv_database.db_selected = list_cbs->item_position - 1;

  DBaseRefreshOpts();
}

/*
 * Synopsis:
 *   void DBaseOptsRefreshAll( void )
 *
 * Description:
 *   This function is used to do all of the relevant updating whenever
 *   the contents of the database is changed. This involves refreshing
 *   any GUI components and also saving the lastest version of the
 *   database back to disk.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 August 1998
 *
 */

void
DBaseOptsRefreshAll( int options_gui_only )
{
  
  /* Update the database file on disk with the current memory version */
  /* We do this before refreshing the widgets because the save func   */
  /* performs a sort on the geosets and removes degenerate entries.   */

  if ( tv_database.database != NULL ) {

    /* let's deselect the current entry in the Open TV window because */
    /* the order of datasets and servers may have changed on screen.  */

    tv_database.tv_curr_geo = -1; 

    if ( tsmWriteDatabase( tv_database.database, NULL ) == FALSE ) 
      DBaseOptsError( "An error occurred while saving the database file" );
  }

  /* Refresh all relevant widgets */
  
  if ( ! options_gui_only )
    DBaseRefreshTV( DBASE_REFRESH_ALL );

  DBaseRefreshOpts();
}

/*
 * Synopsis:
 *   void DBaseReadServerDone( void *closure )
 *
 * Description:
 *   This function is called whenever a server re-read operation has
 *   been completed. It simply refreshes the Database window with the
 *   new database contents. This function is invoked as a followup
 *   callback from a BusyDialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 26 August 1998
 *
 */

static void
DBaseReadServerDone( void *closure )
{
  DBaseOptsRefreshAll( FALSE );
}

/*
 * Synopsis:
 *   void DBaseUpdateCB( char const *name, float percent, void *closure )
 *
 * Description:
 *   This callback is invoked by tsmUpdateDatabaseFromServerCB whenever
 *   it starts looking at a new dataset. We use this in order to update
 *   BusyDialog widget so that the user can get some feedback about what
 *   we are up to, rather than just sitting and spinning.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 May 1999
 *
 */

static void
DBaseUpdateCB( char const *name, float percent, void *closure )
{
  DBaseUpdateClosure *info = (DBaseUpdateClosure *) closure;
  char               message[1024];
  float              complete = 0.0;

  /* Update the message for the busy dialog window */

  strcpy( message, "Reading Dataset: " );
  strcat( message, name );
  strcat( message, "\nFrom Server: " );
  strcat( message, info->server_url );
  strcat( message, "\nPercentage Complete: " );

  /* work out the percentage of the total update operation we have done */
  /* this involves taking into account the percentage done for this     */
  /* server and also the number of servers that we are updating.        */
  
  if ( info->total_servers > 0 )
    complete = ( percent / info->total_servers ) + 
      ( 100.0 * info->curr_server / info->total_servers );

  sprintf( message, "%s%1.0f%%", message, complete );

  /* finally, set the new busy dialog message */

  SetBusyText( info->busy, message );
}

/*
 * Synopsis:
 *   void DBaseReadServerThread( BusyDialog *busy )
 *
 * Description:
 *   This function is called by OpenBusyDialog to run in a new thread.
 *   It will contact one or more servers that are specified in the
 *   database and will read the list of datasets that are held at
 *   that server.
 *
 *   If you passed a closure of NULL to OpenBusyDialog, then all of the
 *   servers in the database will be contacted. Otherwise, it is
 *   assumed that closure is a pointer to an int that specifies
 *   the number of the (single) server to be contacted.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 26 August 1998
 *
 */

static void
DBaseReadServerThread( BusyDialog *busy )
{
  DBaseUpdateClosure info;
  TsmServer          *server;
  char               message[256];
  int                start, stop, i;

  /* Work out the range of servers that we are going to contact */

  if ( busy->closure != NULL ) {
    start = stop = *((int *) busy->closure);
  } else {
    start = 0;
    stop = tv_database.database->numServers - 1;
  }

  /* Now loop through each of these servers and read from them */

  for ( i = start; i <= stop; ++i ) {
    server = &tv_database.database->server[i];

    /* Update the message in the busy dialog window */

    sprintf( message, "Reading Datasets From Server:\n%s",
	     server->location->url);
    SetBusyText( busy, message );

    /* set up the callback closure for the update routine */

    info.busy          = busy;
    info.server_url    = server->location->url;
    info.curr_server   = i - start;
    info.total_servers = stop - start + 1;

    /* and now call the update routine */
    if ( reread_param == NULL )
      tsmInitParams( &reread_param );
    tsmSetParam( reread_param, TSM_PARAM_READ_MULTI, 1 );
    tsmUpdateDatabaseFromServerCB( tv_database.database, server, reread_param,
				   DBaseUpdateCB, &info );
  }

  /* We've done our business, so finish up */
  
  SetBusyFollowup( busy, DBaseReadServerDone, NULL );
  CloseBusyDialog( busy );
}

/*
 * Synopsis:
 *   void DBaseReadServer( void )
 *
 * Description:
 *   Puts up a working requester with a spinning globe and tries to
 *   read all of the datasets that are listed at the currently 
 *   selected server.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 2 September 1998
 *
 */

static void
DBaseReadServer( void )
{
  /*static const char funcName[] = "DBaseReadServer";*/

  if ( tv_database.db_selected < 0 ) {
    DBaseOptsError( "Please select a server to read from first" );
    return;
  }

  OpenBusyDialog( NULL, "Reading Datasets From Server\n",
		  DBaseReadServerThread, &tv_database.db_selected );
}

static void
DBaseReadServerCB( Widget w, XtPointer closure, XtPointer call_data )
{
  /*static const char funcName[] = "DBaseReadServerCB";*/
  DBaseReadServer ();
}

/*
 * Synopsis:
 *   void DBaseOptionActions( Widget w, XtPointer client_data, XtPointer )
 *
 * Description:
 *   This function is called whenever any of the server management buttons
 *   are pressed, e.g. "Add Server", "Remove Server", etc.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseOptionActions( Widget w, XtPointer client_data, XtPointer call_data )
{
  /*static const char funcName[] = "DBaseOptionActions";*/
  TsmServer  *server, temp_server;

  if ( tv_database.database == NULL ) {
    DBaseOptsError( "You do not have a valid database loaded.\n\n"
		    "You can load an existing database, or create\n"
		    "A new one, by typing the filename into the\n"
		    "Database field and pressing RETURN" );
    return;
  }

  if ( w == db_add_server ) {

    /* Add Server: lets the user add a new server to the list of  */
    /* servers for the current database. Here we just fire up the */
    /* input widget - the real meat is done in DBaseAddCallback() */

    GenericPromptDialog( db_dialog, &db_add_dialog,
			 "Enter the URL for the new server:",
			 DBaseAddCallback, NULL );
    
  } else if ( w == db_remove_server ) {

    /* Remove Server: this option will delete the selected server */
    /* entry from the database and also remove all of the dataset */
    /* instances in the database that are on that server. Here we */
    /* just fire up the confirmation widget - the real meat is    */
    /* done in the function DBaseDelCallback().                   */

    if ( tv_database.db_selected < 0 ) {
      DBaseOptsError( "Please select a server to remove first" );
      return;
    }

    GenericConfirmDialog( db_dialog, &db_remove_dialog, "Do you really want"
			  " to remove this\nserver from the database?",
			  DBaseDelCallback, NULL );

  } else if ( w == db_move_up ) {

    /* Move Up: bubble the current server up through the list. This */
    /* controls the order in which instances are listed in the main */
    /* Open TV selection window.                                    */

    if ( tv_database.db_selected < 0 ) {
      DBaseOptsError( "Please select a server to move first" );
      return;
    } else if ( tv_database.db_selected == 0 ) /* already at top */
      return;

    server = tv_database.database->server;
    temp_server = server[tv_database.db_selected];
    server[tv_database.db_selected] = server[tv_database.db_selected-1];
    server[--tv_database.db_selected] = temp_server;
    DBaseOptsRefreshAll( TRUE );

  } else if ( w == db_move_down ) {

    /* Move Down: bubble the current server down through the list.  */
    /* This controls the order in which instances are listed in the */
    /* main Open TV selection window.                               */

    if ( tv_database.db_selected < 0 ) {
      DBaseOptsError( "Please select a server to move first" );
      return;
    } else if ( tv_database.db_selected == tv_database.database->numServers-1 )
      return;

    server = tv_database.database->server;
    temp_server = server[tv_database.db_selected];
    server[tv_database.db_selected] = server[tv_database.db_selected+1];
    server[++tv_database.db_selected] = temp_server;
    DBaseOptsRefreshAll( TRUE );

  } else if ( w == db_reread_server ) {

    /* Re-read Server: this will contact the specified server and   */
    /* update the database with all of the datasets that are listed */
    /* at that server location.                                     */

    DBaseReadServer();

  } else if ( w == db_reread_all ) {

    /* Re-read All: this will contact all of the servers that are    */
    /* listed in the current database & update the database with all */
    /* of the datasets that are listed at each of these locations    */

    if ( tv_database.database->numServers == 0 ) {
      DBaseOptsError( "The database contains no servers to read from!" );
      return;
    }
    
    /* Try to contact each server that has been specified and   */
    /* update the database with all the datasets on that server */

    OpenBusyDialog( NULL, "Reading Datasets From Server\n",
		    DBaseReadServerThread, NULL );

  }
}

/*
 * Synopsis:
 *   void DBaseRefreshOpts( void )
 *
 * Description:
 *   This function will refresh the Database Options window. It will
 *   update the scrolled list widget with the server list from the
 *   currently loaded database (if there is one), and will display the
 *   the timestamp when the currently selected server was last contacted.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
DBaseRefreshOpts( void )
{
  XmString  name[100];
  TsmServer *server;
  char      buffer[100];
  int       n = 0;

  /* Make the list of XmString server names */
  
  if ( tv_database.database != NULL ) {
    for ( n = 0; n < tv_database.database->numServers; ++n ) {
      name[n] = tvXmString( tv_database.database->server[n].location->url );
    }
  }
  
  /* Refresh the list widget of the database options window */

  XtVaSetValues( db_server_list, XmNitems, name, XmNitemCount, n, NULL );

  /* Update the selected item of the list */

  if ( tv_database.db_selected >= 0 )
    XmListSelectPos( db_server_list, tv_database.db_selected + 1, False );
  else
    XtVaSetValues( db_server_list, XmNselectedItemCount, 0, NULL );

  /* Free the array of XmStrings that we have just created */
  
  while ( n > 0 ) XmStringFree( name[--n] );

  /* Now update the last contacted widget */

  if ( tv_database.db_selected >= 0 ) {
    server = &tv_database.database->server[tv_database.db_selected];
    sprintf( buffer, "Last contacted : %s",
	     tsmDateString( server->last_contacted ) );

    n = strlen(buffer);
    if ( buffer[n-1] == '\n' ) buffer[n-1] = '\0';

    name[0] = tvXmString( buffer );
  } else
    name[0] = tvXmString(" ");

  XtVaSetValues( db_info, XmNlabelString, name[0], NULL );

  XmStringFree( name[0] );

}

/*
 * Synopsis:
 *   void DBaseSetFullDPSSRead( int do_full_dpss_read )
 *
 * Description:
 *   This function is used to control how the re-read server function
 *   works for DPSS servers. The default is to do a fast DPSS read
 *   which means that we only read a dataset's information if it is
 *   not already in the database and if the load date for the set is
 *   later than the last time we updated the server. If you pass
 *   a value of TRUE to this function, then this means that you want
 *   to do a full update of every set on the DPSS.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 8 July 1999
 *
 */

void
DBaseSetFullDPSSRead( int do_full_dpss_read )
{
  const char *funcName = "DBaseSetFullDPSSRead";

  if ( reread_param == NULL )
    tsmInitParams( &reread_param );
  if ( reread_param ) {
    tsmSetParam( reread_param, TSM_PARAM_FULL_DPSS_READ, do_full_dpss_read );
    TSMTRACE(( TSMMASK|1, funcName, "Setting DPSS full server reads: %d",
	       do_full_dpss_read ));
  }
}

