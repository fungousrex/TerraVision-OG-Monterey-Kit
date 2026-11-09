/*
 * Name:  BusyDialog.c
 *
 * Description: 
 *    Provides routines to manage the display, dismissal, and animation
 *    for the busy dialog. This is a window that pops up in the current
 *    thread with a rotating world image. A work function is spawned in
 *    another thread. A work function skeleton will look like this:
 *
 *    void example_work_func( BusyDialog *busy ) 
 *    {
 *       printf( "Busy = %p, Closure = %p", busy, busy->closure );
 *       ... do your hard work here ...
 *       CloseBusyDialog( busy ); 
 *    }
 *
 *    Your work thread can perform no X/OpenGL operations. However,
 *    if you want to pop up an error or status window after you have
 *    finished, then you can use SetBusyResult(). You may also change
 *    the text in the busy dialog with SetBusyText();
 *
 * Function List:
 *   void InitBusyDialog( void );
 *   int OpenBusyDialog( Widget, char const *, void *(*)(BusyDialog*), void *);
 *   void CloseBusyDialog( BusyDialog *busy );
 *   void SetBusyText( BusyDialog *busy, char const *message );
 *   void SetBusyResult( BusyDialog *busy, const char *message );
 *
 * Revision History:
 *        $Date: 2002/10/31 05:26:46 $
 *        $Author: reddy $
 *        $Id: BusyDialog.c,v 1.6.2.1 2002/10/31 05:26:46 reddy Exp $
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

#include "BusyDialog.h"
#include "Dialogs.h"

#include "X11Include.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>

#include <tsm/tsm.h>

/* inline the RGB bitmaps for each frame of the rotating world animation */

#include "pixmaps/earth1.h"
#include "pixmaps/earth2.h"
#include "pixmaps/earth3.h"
#include "pixmaps/earth4.h"
#include "pixmaps/earth5.h"
#include "pixmaps/earth6.h"
#include "pixmaps/earth7.h"
#include "pixmaps/earth8.h"
#include "pixmaps/earth9.h"
#include "pixmaps/earth10.h"
#include "pixmaps/earth11.h"
#include "pixmaps/earth12.h"
#include "pixmaps/earth13.h"
#include "pixmaps/earth14.h"
#include "pixmaps/earth15.h"

#define MAX_ANIM_FRAMES     15

static int anim_width;
static int anim_height;
static unsigned char *anim_bitmap[MAX_ANIM_FRAMES] = {
  bitmap_earth1, bitmap_earth2, bitmap_earth3, bitmap_earth4,
  bitmap_earth5, bitmap_earth6, bitmap_earth7, bitmap_earth8,
  bitmap_earth9, bitmap_earth10, bitmap_earth11, bitmap_earth12,
  bitmap_earth13, bitmap_earth14, bitmap_earth15
};

/* define prototypes for all the local module functions */

static void BusyRedraw( glmWindow glmw, void *closure );
static void *BusyExposeCallback( Widget, XtPointer, XtPointer );
static XtTimerCallbackProc BusyCallback( XtPointer, XtIntervalId * );
static int CreateBusyDialog( BusyDialog *busy );


/*
 * Synopsis:
 *   void InitBusyDialog( void )
 *
 * Description:
 *   Performs all of the required initialisations for the busy widget.
 *   This function must be called before any other function in this
 *   module. Currently it works out the dimensions of the rotating
 *   world images and inverts the raw bits of each frame for use with
 *   OpenGL.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
InitBusyDialog( void )
{
  int i;

  /* assume that all frames have the same width and height */

  anim_width  = width_earth1;
  anim_height = height_earth2;
  
  /* flip all the images for rendering with OpenGL */

  for ( i = 0; i < MAX_ANIM_FRAMES; i++ )
    gltInvertImage( anim_bitmap[i], anim_width, anim_height, 3 );
}

/*
 * Synopsis:
 *   static void BusyRedraw( glmWindow glmw, void *closure )
 *
 * Description:
 *   This is the redraw callback that renders the animation. It
 *   uses OpenGL glDrawPixels to just splat the rawbits directly
 *   to the viewport. The animation is progressed on by a single
 *   frame on each call to this function.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static void
BusyRedraw( glmWindow glmw, void *closure )
{
  const char *funcName = "BusyRedraw";
  BusyDialog *busy = (BusyDialog *) closure;
  
  if ( busy->glmw == NULL ) return; 

  busy->frame = ( busy->frame + 1 ) % MAX_ANIM_FRAMES;

  glmMakeCurrentWindow( funcName, busy->glmw ); {
    glDrawPixels( anim_width, anim_height, GL_RGB, GL_UNSIGNED_BYTE,
		  anim_bitmap[busy->frame] );
    glFlush();
  }; glmReleaseWindow( funcName, busy->glmw );
}

/*
 * Synopsis:
 *   static void *BusyExposeCallback( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   A callback that is invoked whenever an EXPOSE event is generated,
 *   i.e. a partially hidden part of the window has become visible.
 *   This just posts a redraw of the animation.
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static void *
BusyExposeCallback( Widget w, XtPointer client_data, XtPointer call_data )
{
  BusyDialog *busy = (BusyDialog *) client_data;
  glmPostRedraw( busy->glmw );
  return NULL;
}

/*
 * Synopsis:
 *   static XtTimerCallbackProc BusyCallback( XtPointer, XtIntervalId * )
 *
 * Description:
 *   An X timer callback that is used to progress the animation.
 *   This callback will also update the dialog's text if this has
 *   been changed with SetBusyText(). This callback will check to
 *   see if the work thread has finished, and if so it will remove
 *   the busy dialog, clean up, and display any parting message that
 *   was specifed by SetBusyResult().
 *
 *   The work thread signals that it is done by calling CloseBusyDialog().
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static XtTimerCallbackProc
BusyCallback( XtPointer client_data, XtIntervalId *id )
{
  /*static const char funcName[] = "BusyCallback";*/
  BusyDialog   *busy = (BusyDialog *) client_data;
  XtAppContext app = XtWidgetToApplicationContext( busy->mainForm );
  int          dismiss;

  /* see if the worker thread has signalled a completion */

  PR_Lock( busy->lock ); {
    dismiss = busy->dismiss;
  }; PR_Unlock( busy->lock );

  if ( dismiss ) {

    /* remove the busy dialog and clean up everything */

    XtUnmanageChild( busy->mainForm );
    XtRemoveCallback( busy->render, GLwNexposeCallback, (XtCallbackProc)
		      BusyExposeCallback, busy );
    glmRemoveRedraw( busy->glmw );
    glmDestroyWindow( busy->glmw );

    XtDestroyWidget( busy->mainForm );

    /* wait for the work thread to finish */

    PR_JoinThread( busy->busy_thread );

    /* restore the parent's cursor */

    XUndefineCursor( XtDisplay( busy->parent ), XtWindow( busy->parent ) );

    /* display the work thread's parting message, if given */
    
    if ( busy->result ) {
      static Widget widget = NULL;
      GenericErrorDialog( busy->parent, &widget, busy->result );
      tsmMemFree( busy->result );
    }

    /* perform a followup function, if given */

    if ( busy->followup ) (*busy->followup)( busy->follow_closure );

    /* all done - free the BusyDialog structure */

    PR_DestroyLock( busy->lock );
    tsmMemFree( busy );

  } else {

    /* not done yet - call this same function in 20 ms time */

    XtAppAddTimeOut( app, 1000/20, (XtTimerCallbackProc) BusyCallback, busy );

    /* set a new dialog message if the work func changed it */

    PR_Lock( busy->lock ); {
      if ( busy->message ) {
	XtVaSetValues( busy->label, XmNlabelString, busy->message, NULL );
	XmStringFree( busy->message );
	busy->message = NULL;
      }
    }; PR_Unlock( busy->lock );

    /* update the OpenGL animation */

    glmPostRedraw( busy->glmw );

  }

  return NULL;
}

/*
 * Synopsis:
 *   static int CreateBusyDialog( BusyDialog *busy )
 *
 * Description:
 *   This function creates the busy dialog and starts the timer
 *   callback to render the spining globe animation.
 *
 *   We pass in an allocated BusyDialog structure. We assume the 
 *   busy->message structure must already been initialised to a
 *   valid XmString.
 *
 * Returns: TRUE if successful, FALSE if not
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

static int
CreateBusyDialog( BusyDialog *busy )
{
  static char  *funcName = "CreateBusy()";
  Widget       frame;
  XmString     title;
  int          n = 0;
  Arg          args[15];
  XtAppContext app;
  glmContext   glmCtx;

  /* if no parent widget specified, assume the root TerraVision window */

  if ( busy->parent == NULL ) busy->parent = applicationShell;

  /* create a new form dialog widget for the busy window */

  n = 0;
  title = tvXmString( "Please Wait..." );
  XtSetArg( args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL ); n++;
  XtSetArg( args[n], XmNnoResize, True ); n++;
  XtSetArg( args[n], XmNresizable, True ); n++;
  XtSetArg( args[n], XmNdialogTitle, title ); n++;

  busy->mainForm = XmCreateFormDialog( busy->parent, "busyDialog", args, n );
  
  XmStringFree( title );

  if ( busy->mainForm == NULL ) {
    tsmError( funcName, "could not open busy dialog" );
    return FALSE;
  }

  /* create a raised frame to display the animation in */

  n=0;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNbottomOffset, 10 ); n++;
  XtSetArg( args[n], XmNleftOffset, 10 ); n++;
  XtSetArg( args[n], XmNtopOffset, 10 ); n++;
  XtSetArg( args[n], XmNshadowType, XmSHADOW_OUT ); n++;
  frame = XmCreateFrame( busy->mainForm, "frame", args, n );
  XtManageChild( frame );

  /* create a label to display the dialog's text */

  busy->label = 
    XtVaCreateManagedWidget( "busyLabel",
			     xmLabelWidgetClass, busy->mainForm,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNleftAttachment, XmATTACH_WIDGET,
			     XmNleftWidget, frame,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNbottomAttachment, XmATTACH_FORM,
			     XmNrecomputeSize, True,
			     XmNleftOffset, 20,
			     XmNrightOffset, 20,
			     XmNtopOffset, 10,
			     XmNbottomOffset, 10,
			     NULL );

  PR_Lock( busy->lock ); {
    if ( busy->message ) {
      XtVaSetValues( busy->label, XmNlabelString, busy->message, NULL );
      XmStringFree( busy->message );
      busy->message = NULL;
    }
  }; PR_Unlock( busy->lock );

  /* set up the OpenGL rendering area for the animation */

  glmCtx = glmXFindContext (XtDisplay (busy->mainForm),
			    XtScreen (busy->mainForm),
			    True, 0);

  busy->render = 
    XtVaCreateManagedWidget( "renderWidget",
			     glwMDrawingAreaWidgetClass, frame,
			     GLwNvisualInfo, glmXContextVisualInfo (glmCtx),
			     XmNleftAttachment, XmATTACH_FORM,
			     XmNrightAttachment, XmATTACH_FORM,
			     XmNtopAttachment, XmATTACH_FORM,
			     XmNrecomputeSize, False,
			     XmNwidth, anim_width,
			     XmNheight, anim_height,
 			     NULL);

  busy->glmw = glmXtCreateWindow( busy->render, glmCtx, True, False );

  /* Add OpenGL callbacks and draw the first frame */
  
  glmRegisterRedraw( busy->glmw, BusyRedraw, busy );
  XtAddCallback( busy->render, GLwNexposeCallback, (XtCallbackProc) 
		 BusyExposeCallback, busy );
  XtAddCallback( busy->render, GLwNginitCallback, (XtCallbackProc)
		 BusyExposeCallback, busy );

  /* Add the timer based callback, operating at 20Hz */

  app = XtWidgetToApplicationContext( busy->mainForm );
  XtAppAddTimeOut( app, 1000/20, (XtTimerCallbackProc) BusyCallback, busy );

  /* set a busy cursor, display the window, and we're all done */

  XDefineCursor( XtDisplay(busy->parent), XtWindow(busy->parent), busyCursor );
  XtManageChild( busy->mainForm );

  return TRUE;
}

/*
 * Synopsis: 
 *   int OpenBusyDialog( Widget parent, char const *message, 
 *                       void *(*work_func)(BusyDialog*), void *closure )
 *
 * Description:
 *   This is the exported function that is used in order to open
 *   a busy dialog. You pass in the Widget id for the dialog that
 *   the busy window is to be a child of, the text you want to
 *   appear on the dialog, and details of the work function to
 *   perform.
 *
 *   The work_func function is executed in a new thread. It should
 *   follow the skeleton given at the top of this file. The closure
 *   that you pass into OpenBusyDialog can be retrieved in your
 *   work_func via the closure field of the BusyDialog structure.
 *   Inside work_func, you may call SetBusyText(), SetBusyResult(),
 *   and CloseBusyDialog().
 *
 *   The parent dialog will have a busy cursor installed and you
 *   will not be able to interact with it until work_func completes.
 *   If you specify a NULL parent, then the root TerraVision window
 *   is assumed (applicationShell).
 *
 * Returns: TRUE if successful, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

int
OpenBusyDialog( Widget parent, char const *message, 
		void (*work_func)(BusyDialog*), void *closure )
{
  const char *funcName = "ShowBusyDialog";
  BusyDialog *busy;

  busy = (BusyDialog *) tsmMemCalloc( 1, sizeof(BusyDialog) );
  if ( busy == NULL ) return FALSE;

  /* initialise the dialog structure */

  busy->dismiss  = FALSE;
  busy->frame    = 0;
  busy->closure  = closure;
  busy->parent   = parent;
  busy->result   = NULL;
  busy->followup = NULL;

  /* create a mutex lock for controlling the changing of fields */

  if ( !(busy->lock = PR_NewLock()) ) {
    tsmError( funcName, "Unable to create lock mutex." );
    tsmMemFree( busy );
    return FALSE;
  }

  /* set the default message string for this widget */

  SetBusyText( busy, message );

  /* display the busy dialog */

  CreateBusyDialog( busy );

  /* spawn a new thread that performs the work for this action */

  if ( pthr_create( &busy->busy_thread, "Busy Dialog", 
		    (void (*)(void *)) work_func, busy ) ) {
    tsmError( funcName, "Unable to spawn busy dialog work thread" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   void CloseBusyDialog( BusyDialog *busy )
 *
 * Description:
 *   Signal the end of the work thread. This should be the last thing
 *   performed by the work thread. It will signal to the parent thread
 *   that we want to close the busy dialog and then it will kill itself.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
CloseBusyDialog( BusyDialog *busy )
{
  /*static const char funcName[] = "CloseBusyDialog";*/
  if ( busy == NULL ) return;

  /* this flag is caught by BusyCallback in the parent thread */

  PR_Lock( busy->lock ); {
    busy->dismiss = TRUE;
  }; PR_Unlock( busy->lock );

  /* this thread is now done */
}

/*
 * Synopsis:
 *   void SetBusyText( BusyDialog *busy, char const *message )
 *
 * Description:
 *   Changes the text that is displayed in a current busy dialog.
 *   This function just changes the BusyDialog contents. The widget
 *   is changed in the timer callback function BusyCallback().
 *
 *   A NULL message string will remove all text from the dialog.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
SetBusyText( BusyDialog *busy, char const *message )
{
  /*static const char funcName[] = "SetBusyText";*/
  if ( busy == NULL ) return;

  /* this message is picked up by BusyCallback() in the parent thread */

  PR_Lock( busy->lock ); {
    if (busy->message) XmStringFree (busy->message);
    busy->message = tvXmString( ( message ) ? (char *) message : "" );
  }; PR_Unlock( busy->lock );
}

/*
 * Synopsis:
 *   void SetBusyResult( BusyDialog *busy, const char *message )
 *
 * Description:
 *   You should not perform any X/OpenGL calls in the worker thread.
 *   This means not displaying a popup dialog to the user informing
 *   them of an error or the successful completion of the operation.
 *
 *   This function helps to get round this limitation by storing a
 *   a text string that the parent function will display once the
 *   work thread exits. If you do not call this function in the 
 *   work thread, then no message is displayed on termination.
 *
 *   You can removing a previously installed result string by 
 *   calling this function again with message = NULL.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
SetBusyResult( BusyDialog *busy, const char *message )
{
  if ( busy == NULL ) return;
  
  /* this text is displayed in a dialog when the thread finishes */

  if ( busy->result ) tsmMemFree( busy->result );
  busy->result = tsmMemStrdup( message );
}

/*
 * Synopsis:
 *   void SetBusyFollowup( BusyDialog *busy, void (*followup)(void*), void * )
 *
 * Description:
 *   You should not perform any X/OpenGL calls in the worker thread.
 *   If you want to do more than just pop up a window, or if you
 *   want the parent thread to perform some followup operations once
 *   the work thread has completed, then this is how you do it.
 *
 *   This function lets you install a callback (with closure) that
 *   is called by the parent thread once the work thread has exited.
 *
 *   You can removing a previously installed followon callback by 
 *   calling this function again with followup = NULL.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 5 April, 1999
 *
 */

void
SetBusyFollowup( BusyDialog *busy, void (*followup)(void*), void *closure )
{
  if ( busy == NULL ) return;
  
  busy->followup = followup;
  busy->follow_closure = closure;
}

/*
 * Synopsis:
 *   void SetupBusyCallback(tvBase *threeD, BusyDialog *busy)
 *
 * Description:
 *   Registers a callback function with tvBase so that any busy messages
 *   from inside the tvBase functionality are routed to the selected
 *   BusyDialog.
 *
 * Returns: void
 *
 * Author: Lee Iverson
 * 
 * Date: November, 1999
 *
 */

void 
SetupBusyCallback (ThreeDWidget *threeD, BusyDialog *busy)
{
  ThreeDWidgetBusyDialog (threeD, busy);
}

/*
 * Synopsis:
 *   void ClearBusyCallback(tvBase *threeD)
 *
 * Description:
 *   Unregisters any BusyDialog callbacks previously registered.
 *
 * Returns: void
 *
 * Author: Lee Iverson
 * 
 * Date: November, 1999
 *
 */

void
ClearBusyCallback (ThreeDWidget *threeD)
{
  ThreeDWidgetBusyDialog (threeD, NULL);
}

