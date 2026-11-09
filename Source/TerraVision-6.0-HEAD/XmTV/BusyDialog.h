/*
 * Name:  BusyDialog.h
 *
 * Description: Header file for BusyDialog.c. This provides routines
 *    to manage the display, dismissal, and animation for the busy
 *    dialog. This is a window that pops up in the current thread with a
 *    rotating world image. A work function is spawned in another
 *    thread. A work function skeleton will look like this:
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
 *   int OpenBusyDialog( Widget, char const *, void *(*)(void*), void * );
 *   void CloseBusyDialog( BusyDialog *busy );
 *   void SetBusyText( BusyDialog *busy, char const *message );
 *   void SetBusyResult( BusyDialog *busy, const char *message );
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

#ifndef BUSYDIALOG_H
#define BUSYDIALOG_H

typedef struct BusyDialog BusyDialog;

#include "ThreeDWidget.h"

#include <Xm/Xm.h>
#include <prthread.h>
#include <prlock.h>
#include <glt/glt.h>
#include <glt/glm.h>

struct BusyDialog {
  int        dismiss;         /* set to TRUE to dismiss the dialog */
  PRThread  *busy_thread;     /* the worker thread ID */
  glmWindow  glmw;            /* OpenGL context information for animation */
  int        frame;           /* frame number for animation */
  Widget     parent;          /* the parent window we are attached to */
  Widget     mainForm;        /* the form dialog for the busy window */
  Widget     label;           /* label widget for the dialog message */
  XmString   message;         /* message to display in busy dialog */
  Widget     render;          /* the glwMDrawingAreaWidgetClass widget */
  void       *closure;        /* closure passed into OpenBusyDialog */
  char       *result;         /* a message to display when done */
  PRLock     *lock;           /* lock for changing dismiss or message */
  void       (*followup)(void*); /* function for parent to call when done */
  void       *follow_closure; /* closure for the followup function */
};

/* must call this initialization function before anything else here */

void InitBusyDialog( void );

/* display a busy dialog and perform the work_func in a separate thread */

int OpenBusyDialog( Widget parent, char const *message, 
		    void (*work_func)(BusyDialog*), void *closure );

/* close a busy dialog and finish the work function (inside work thread) */

void CloseBusyDialog( BusyDialog *busy );

/* change the title of a busy dialog (inside work thread) */

void SetBusyText( BusyDialog *busy, char const *message );

/* set a text message to display on completion (set inside work thread) */

void SetBusyResult( BusyDialog *busy, const char *message );

/* set a function to be called by the parent thread upon completion */

void SetBusyFollowup( BusyDialog *, void (*followup)(void*), void *closure );

/* set a callback function for busy messages from ThreeDWidget */

void SetupBusyCallback (ThreeDWidget *, BusyDialog *);

/* clear the callback function for busy messages from ThreeDWidget */

void ClearBusyCallback (ThreeDWidget *);

#endif
