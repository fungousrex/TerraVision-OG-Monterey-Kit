/*
 * Name: LINK.C
 *
 * Description: Widget for overhead view of terrain.
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

#include <glt/glt.h>
#include <pthr/list.h>

#include "X11Include.h"

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>

#include <Xm/CascadeB.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

#include "ThreeDWidgetI.h"
#include "ThreeDCore.h"
#include "Dialogs.h"
#include "Link.h"

#define SELECT_MENU 	  "Select Window..."
#define SELF_MENU   	  "Self"
#define SELECT_ALL_MENU   "Select All"
#define DESELECT_ALL_MENU "Deselect All"
#define ON_MENU		  "On"

static int HasShell (ThreeDWidget *w, void *closure);

/*
 * Routine to let user select a window using the mouse
 */

ThreeDWidget *Select_Window(Widget parent)
{
  static const char funcName[] = "Select_Window";
  int status;
  Cursor cursor;
  XEvent event;
  Window root;
  int dummyi;
  unsigned int dummy;
  Window target_win = None;
  int buttons = 0;
  static Widget link_cancel_dlg = NULL;
  Display *dpy = XtDisplay(parent);

  root = RootWindowOfScreen(XtScreen(parent));

  /* Make the target cursor */
  cursor = XCreateFontCursor(dpy, XC_crosshair);

  /* Grab the pointer using target cursor, letting it room all over */
  status = XGrabPointer(dpy, root, False,
                        ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                        GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) tsmError(funcName, "Can't grab the mouse.");

  GenericMessageDialog( parent, &link_cancel_dlg,
			"Select a window to link to, or press 'Esc' to cancel",
			 NULL );

  /* Let the user select a window... */
  while ((target_win == None) || (buttons != 0)) {
    /* allow one more event */
    XAllowEvents(dpy, SyncPointer, CurrentTime);
    XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
    switch (event.type) {
    case ButtonPress:
      if (target_win == None) {
        target_win = event.xbutton.subwindow; /* window selected */
        if (target_win == None) target_win = root;
      }
      buttons++;
      break;
    case ButtonRelease:
      if (buttons > 0) /* there may have been some down before we started */
        buttons--;
       break;
    case KeyPress:
      if (XLookupKeysym( (XKeyEvent *)&event, 0 ) == XK_Escape)
	return NULL;
      break;
    }
  } 

  XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

  XtUnmanageChild(link_cancel_dlg);

  /* get surrounding window */
  if (XGetGeometry (dpy, target_win, &root, &dummyi, &dummyi,
		    &dummy, &dummy, &dummy, &dummy) &&
      target_win != root)
    target_win = XmuClientWindow (dpy, target_win);
  else
    return NULL;

  return ThreeDWidgetFind (HasShell, (void *) target_win);
}

static int
HasShell (ThreeDWidget *w, void *closure)
{
  Window target_win = (Window) closure;

  return XtWindow(XtParent(XtParent(w->windowArea))) == target_win;
}

/*
 * Synopsis:
 *   void LinkSubmenu()
 *
 * Description:
 *	Callback for Link submenu buttons
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
static void 
LinkSubmenu (ThreeDWidget *fromWgt,
	     ThreeDWidget *toWgt,
	     Widget parent, 
	     Boolean on)
{
  tvBase *fromCore = fromWgt->threeD;
  tvBase *toCore = toWgt->threeD;
  tvLinkType sel;

  if (parent == fromWgt->moveMenu)
    sel = TV_MOVEMENT;

  else if (parent == fromWgt->frustumMenu)
    sel = TV_FRUSTUM;

  else if (parent == fromWgt->tileMenu)
    sel = TV_TILES;

  else if (parent == fromWgt->pathMenu)
    sel = TV_PATHS;
  else
    return;

  if (on)
    tvAddLink (fromCore, sel, toCore);
  else
    tvRemoveLink (fromCore, sel, toCore);
}


/*
 * Synopsis:
 *   Widget LinkGetMenu()
 *
 * Description:
 *	Return link menu associated with toWgt on the menu parent
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
Widget LinkGetMenu(ThreeDWidget *toWgt, Widget parent)
{
  Cardinal nChildren;
  WidgetList children;
  int cChild;

  XtVaGetValues(parent, XmNnumChildren, &nChildren, XmNchildren, &children, NULL);
  
  for (cChild = 0; cChild < nChildren; cChild++) {
    XmString str;
    char *title;
    Boolean free_title;

    /* what was the name of this menu subitem? */
    XtVaGetValues(children[cChild], XmNlabelString, &str, NULL);
    free_title = tvXmGetString(str, &title);
#if 0
    /* XXX 
       This should be necessary, but seems to cause problems w/Motif
       under Irix -- leei */
    XmStringFree (str); 
#endif

    if (strcmp(title, toWgt->title) == 0) {
      if (free_title) XtFree (title);
      return children[cChild];
    }
    if (free_title) XtFree (title);
  }

  return NULL;
}

typedef struct {
  ThreeDWidget *from;
  Widget parent;
  int set;
} SetSubmenuArg;

static void
SetSubmenu (ThreeDWidget *to, void *closure)
{
  SetSubmenuArg const *arg = closure;
  Widget checkMenu = LinkGetMenu(to, arg->parent);
  if (checkMenu) {
    XtVaSetValues (checkMenu, XmNset, arg->set, NULL);
    LinkSubmenu (arg->from, to, arg->parent, arg->set);
  }
}

static void
SetAllSubmenus (ThreeDWidget *from, Widget parent, int val)
{
  SetSubmenuArg cl;
  cl.from = from;
  cl.parent = parent;
  cl.set = val;
  ThreeDWidgetForall (SetSubmenu, &cl);
}

static int
HasTitle (ThreeDWidget *w, void *closure)
{
  return ! strcmp (w->title, (char *) closure);
}

/*
 * Synopsis:
 *   void LinkMenuCB()
 *
 * Description:
 *	Callback for Link menu buttons
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
static void 
LinkMenuCB(Widget    menu,
	   XtPointer client_data,
	   XtPointer call_data)
{
  /*static const char funcName[] = "LinkMenuCB()";*/
  Widget parent = XtParent(menu);
  XmString str;
  char *title;
  Widget checkMenu;
  Boolean set;
  ThreeDWidget *toWgt;
  ThreeDWidget *fromWgt = (ThreeDWidget *)client_data;
  Boolean free_title;

  /* what was the name of this menu subitem? */
  XtVaGetValues(menu, XmNlabelString, &str, NULL);
  free_title = tvXmGetString(str, &title);
#if 0
  /* XXX Problems with Motif on Irix -- leei */
  XmStringFree (str);
#endif

  if (strcmp(ON_MENU, title) == 0) {
    set = XmToggleButtonGadgetGetState(menu);
    if (parent == fromWgt->frustumMenu)
      tvSetBoolean (fromWgt->threeD, TV_SHOW_FRUSTUMS, set);
    else if (parent == fromWgt->tileMenu)
      tvSetBoolean (fromWgt->threeD, TV_SHOW_TILES, set);
    else if (parent == fromWgt->pathMenu)
      tvSetBoolean (fromWgt->threeD, TV_SHOW_PATHS, set);
  }
  else if (strcmp(SELECT_MENU, title) == 0
	   && (toWgt = Select_Window(fromWgt->mainForm))) {
    /* the user picked one, which was it? */
    checkMenu = LinkGetMenu(toWgt, parent);
    if (checkMenu) {
      XtVaGetValues( checkMenu, XmNset, &set, NULL );
      XtVaSetValues( checkMenu, XmNset, !set, NULL );
      LinkSubmenu(fromWgt, toWgt, parent, !set);
    }
  }
  else if (strcmp(SELF_MENU, title) == 0) {
    /* link to self */
    checkMenu = LinkGetMenu(fromWgt, parent);
    XtVaGetValues( checkMenu, XmNset, &set, NULL );
    XtVaSetValues( checkMenu, XmNset, !set, NULL );
    LinkSubmenu(fromWgt, fromWgt, parent, !set);
  }
  else if (strcmp(SELECT_ALL_MENU, title) == 0) {
    /* turn them all on */
    SetAllSubmenus (fromWgt, parent, TRUE);
  }
  else if (strcmp(DESELECT_ALL_MENU, title) == 0) {
    /* turn them all off */
    SetAllSubmenus (fromWgt, parent, FALSE);
  }
  else {
    /* figure out which ThreeD widget was picked */
    ThreeDWidget *wgt = ThreeDWidgetFind (HasTitle, title);
    if (wgt) {
      XtVaGetValues( menu, XmNset, &set, NULL );
      /* don't need to change check box, done by value changed callback */
      LinkSubmenu(fromWgt, wgt, parent, set);
    }
  }

  if (free_title) XtFree (title);
}

/*
 * Synopsis:
 *   Widget LinkCreateSubmenu(Widget parent)
 *
 * Description:
 *	Create a Link submenu 
 *
 * Returns: created cascading menu
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
static Widget
LinkCreateSubmenu(Widget parent, ThreeDWidget *widget, Boolean self)
{
  Widget submenu;
  list_iter_t i;
  static XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 
  callbacks[0].callback = LinkMenuCB;
  callbacks[0].closure = (XtPointer) widget;


  submenu = XmCreatePulldownMenu (parent, "submenu", NULL, 0);

  XtVaCreateManagedWidget( ON_MENU,
			   xmToggleButtonGadgetClass, submenu,
			   XmNindicatorOn, True,
			   XmNvisibleWhenOff, True,
			   XmNset, True,
			   XmNvalueChangedCallback, callbacks,
			   XmNmnemonic, 'O', NULL );

  XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, submenu, NULL );

  XtVaCreateManagedWidget( SELECT_MENU,
			   xmPushButtonGadgetClass, submenu,
			   XmNactivateCallback, callbacks,
			   XmNmnemonic, 'S',
			   NULL);

  if (self)
    XtVaCreateManagedWidget( SELF_MENU,
			     xmPushButtonGadgetClass, submenu,
			     XmNactivateCallback, callbacks,
			     XmNmnemonic, 'S',
			     NULL);

  XtVaCreateManagedWidget( SELECT_ALL_MENU,
			   xmPushButtonGadgetClass, submenu,
			   XmNactivateCallback, callbacks,
			   XmNmnemonic, 'A',
			   NULL);

  XtVaCreateManagedWidget( DESELECT_ALL_MENU,
			   xmPushButtonGadgetClass, submenu,
			   XmNactivateCallback, callbacks,
			   XmNmnemonic, 'D',
			   NULL);

  XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, submenu, NULL );

  if (widgetList) {
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
      ThreeDWidget *wgt = (ThreeDWidget*) list_value (i);
      if (!(!self && wgt == widget))
	/* add all widgets unless it's the parent widget and we're not */
	/* supposed to add ourselves */
	XtVaCreateManagedWidget( wgt->title,
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, False,
				 XmNvalueChangedCallback, callbacks,
				 NULL);
    }
  }
  
  /*  XtManageChild(submenu);*/

  return submenu;
}

/*
 * Synopsis:
 *   void LinkCreateMenu()
 *
 * Description:
 *	Create the Link menu for the ThreeD widget to connect to other
 *	views with frustums, tiles, views, and paths.
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
void
LinkCreateMenu(Widget menubar, ThreeDWidget *widget)
{
  Widget submenu, menupane;
  int            n = 0;
  Arg            args[10];

  /* Now we can actually create the menus! */
  n = 0;
  XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
  submenu = XmCreatePulldownMenu (menubar, "menuPane", args, n);

  n = 0;
  XtSetArg( args[n], XmNmnemonic, 'C' ); n++;
  menupane = XmCreateCascadeButton( submenu, "Control view of...", args, n );
  widget->moveMenu = LinkCreateSubmenu(menupane, widget, FALSE);
  XtVaSetValues(  menupane, XmNsubMenuId, widget->moveMenu, NULL);
  XtManageChild( menupane ); 
    
  n = 0;
  XtSetArg( args[n], XmNmnemonic, 'F' ); n++;
  menupane = XmCreateCascadeButton( submenu, "Show View Frustum of...", args, n );
  widget->frustumMenu = LinkCreateSubmenu(menupane, widget, TRUE);
  XtVaSetValues(  menupane, XmNsubMenuId, widget->frustumMenu, NULL);
  XtManageChild( menupane ); 
    
  n = 0;
  XtSetArg( args[n], XmNmnemonic, 'T' ); n++;
  menupane = XmCreateCascadeButton( submenu, "Show Tile of...", args, n );
  widget->tileMenu = LinkCreateSubmenu(menupane, widget, TRUE);
  XtVaSetValues(  menupane, XmNsubMenuId, widget->tileMenu, NULL);
  XtManageChild( menupane );
    
  n = 0;
  XtSetArg( args[n], XmNmnemonic, 'P' ); n++;
  menupane = XmCreateCascadeButton( submenu, "Show Paths of...", args, n );
  widget->pathMenu = LinkCreateSubmenu(menupane, widget, TRUE);
  XtVaSetValues(  menupane, XmNsubMenuId, widget->pathMenu, NULL);
  XtManageChild( menupane );

  n = 0;
  XtSetArg (args[n], XmNsubMenuId, submenu); n++;
  XtSetArg (args[n], XmNmnemonic, 'L'); n++;
  widget->linkMenu = XmCreateCascadeButton (menubar, "Link", args, n);
  XtManageChild(widget->linkMenu);

  /*XtManageChild( submenu);*/
  widget->menuBar = menubar;
}


/*
 * Synopsis:
 *   void LinkUpdateSubmenu()
 *
 * Description:
 *	Change the link submenus checkboxes to reflect a new widget addition
 *	
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
static void
LinkUpdateSubmenu(Widget menu, list_t *lst)
{
  list_iter_t i;

  if (widgetList) {
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
      Widget subMenu;
      ThreeDWidget *toWgt = list_value (i);
      subMenu = LinkGetMenu (toWgt, menu);
      if (subMenu) {
	list_iter_t find = list_find (lst, toWgt->threeD);
	if (! list_iter_valid (find))
	  XtVaSetValues(subMenu, XmNset, False, NULL );
	else
	  XtVaSetValues(subMenu, XmNset, True, NULL );
      }
    }
  }
}


/*
 * Synopsis:
 *   void LinkUpdateMenu()
 *
 * Description:
 *	Change the link menus to reflect a new widget addition
 *	
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 21 Feb 1999
 *
 */
void
LinkUpdateMenu(void)
{
  list_iter_t i;

  if (widgetList) {
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
      ThreeDWidget* wgt = (ThreeDWidget*) list_value (i);
      if (wgt->menuBar) {
	if (wgt->linkMenu) {
	  /* first destroy old link menu */
	  XtUnmanageChild(wgt->linkMenu);
	  XtDestroyWidget(wgt->linkMenu);
	}
	/* then create a new one */
	LinkCreateMenu(wgt->menuBar, wgt);

	/* then check each box appropriately */
	LinkUpdateSubmenu(wgt->moveMenu,
			  tvGetLinks (wgt->threeD, TV_MOVEMENT));
	LinkUpdateSubmenu(wgt->frustumMenu, 
			  tvGetLinks (wgt->threeD, TV_FRUSTUM));
	LinkUpdateSubmenu(wgt->tileMenu,
			  tvGetLinks (wgt->threeD, TV_TILES));
	LinkUpdateSubmenu(wgt->pathMenu,
			  tvGetLinks (wgt->threeD, TV_PATHS));
      }
    }
  }
}
