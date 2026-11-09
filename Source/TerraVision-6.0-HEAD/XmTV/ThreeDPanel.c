/*
 * Name:  ThreeDPanel.c
 *
 * Description: Control panel for toggle buttons for view the terrain.
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
#include "Texture.h"
#include "ThreeDPanel.h"
#include "ThreeDWidgetI.h"
#include "ThreeDCoreI.h"
#include "OAA.h"

#include "X11Include.h"

#include <tvutils/tvutils.h>

#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

static void ThreeDPanelSliderChangedCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelToggleChangedCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelStereoToggleChangedCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelToggleCoreChangedCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelDrawToggleChangedCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelTimeoutChangedCB( Widget, XtPointer, XtPointer );
static void ThreeDPanelClearDoodleCB( Widget, XtPointer, XtPointer );
static void ThreeDPanelGetButtonStates(ThreeDPanel *);
static void ThreeDPanelPoppingCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelFreezeTilesToggleCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelFreezeReturnCB(Widget, XtPointer, XtPointer);
static void ThreeDPanelCreateUtilButtons(ThreeDPanel *,Widget,ThreeDPanelType);
static void ThreeDPanelDismissCallback(Widget, XtPointer, XtPointer);
static void ThreeDPanelCreateRenderWidgets(ThreeDPanel *, Widget parent);
static void ThreeDPanelCreatePositionWidgets(ThreeDPanel *, Widget parent);
static void ThreeDPanelCreateDebugWidgets(ThreeDPanel *, Widget parent);
static void ThreeDPanelCreateNavWidgets( ThreeDPanel *, Widget, Boolean );
static Widget PanelSeparator( Widget mainForm, Widget topWidget );

typedef struct ToggleCallback {
  ThreeDPanel *threeDPanel;
  short *flag;
} ToggleCallback;
    
static void
addToggleCallback (ThreeDPanel *threeDPanel, Widget toggle, short *flag)
{
  ToggleCallback *toggleCB = (ToggleCallback *) malloc(sizeof(ToggleCallback));

  toggleCB->threeDPanel = threeDPanel;
  toggleCB->flag = flag;
  XtAddCallback(toggle, XmNvalueChangedCallback,
		ThreeDPanelToggleChangedCB, toggleCB);
}

typedef struct ToggleCoreCallback {
  ThreeDPanel *threeDPanel;
  tvVariable  var;
} ToggleCoreCallback;
    
static void
addToggleCoreCallback (ThreeDPanel *threeDPanel, Widget toggle, tvVariable var)
{
  ToggleCoreCallback *toggleCB = 
    (ToggleCoreCallback *) malloc(sizeof(ToggleCoreCallback));
  tvBase *threeD = threeDPanel->threeDWidget->threeD;

  toggleCB->threeDPanel = threeDPanel;
  toggleCB->var = var;
  XtAddCallback(toggle, XmNvalueChangedCallback,
		ThreeDPanelToggleCoreChangedCB, toggleCB);

  tvSetBoolean (threeD, var, XmToggleButtonGadgetGetState (toggle));
}

/*
 * Synopsis: ThreeDPanel *CreateThreeDPanel()
 *
 * Description: Create of panels for toggling switchs
 *
 * Returns: a pointer to the new ThreeDPanel on success
 *          NULL on failure
 *
 * Author: Stephen Lau
 * 
 * Date: April 1995
 *
 */

ThreeDPanel * 
CreateThreeDPanel( Widget parent, void *threeDWidget, Boolean mode3d )
{
  static char *funcName = "CreateThreeDPanel";
  ThreeDPanel *threeDPanel = NULL;

  threeDPanel = (ThreeDPanel *) malloc( sizeof(ThreeDPanel) );
  if (!threeDPanel) {
    tsmError (funcName, "Unable to create a panel widget!");
    return NULL;
  }
  threeDPanel->threeDWidget = threeDWidget;

  if ( ! tvuGetInt( TVU_CAVE ) ) {
    ThreeDPanelCreateRenderWidgets(threeDPanel, parent);
    ThreeDPanelCreateDebugWidgets(threeDPanel, parent);
    ThreeDPanelCreateNavWidgets(threeDPanel, parent, mode3d);
    ThreeDPanelCreatePositionWidgets(threeDPanel, parent);
  }

  threeDPanel->managed[RENDER_PANEL] = FALSE;
  threeDPanel->managed[DEBUG_PANEL] = FALSE;
  threeDPanel->managed[POSITION_PANEL] = FALSE;
  threeDPanel->managed[NAV_PANEL] = FALSE;
  
  if ( ! tvuGetInt( TVU_CAVE ) ) {
    /* GET THE INITIAL BUTTON STATES */
    ThreeDPanelGetButtonStates(threeDPanel);
  }

  return threeDPanel;
}

/*
 * Synopsis: Widget *PanelSeparator( Widget, Widget )
 *
 * Description: Adds a separator bar to a threeDPanel widget
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 7 February 1995
 *
 */

static Widget
PanelSeparator( Widget mainForm, Widget topWidget )
{
  return XtVaCreateManagedWidget( "threeDPanelSeparator",
				  xmSeparatorGadgetClass, mainForm,
				  XmNshadowThickness, 4,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, topWidget,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  XmNorientation, XmHORIZONTAL,
				  NULL );
}

/*
 * Synopsis: void ThreeDPanelCreateDebugWidgets()
 *
 * Description: Creates the widgets for the debug panel
 *
 * Returns: void
 *
 * Author: Stephen Lau
 * 
 * Date:  1995
 *
 */

static void
ThreeDPanelCreateDebugWidgets( ThreeDPanel *threeDPanel, Widget parent )
{
/*  static char  *funcName = "ThreeDPanelCreateDebugWidgets";*/
  Widget       separator, toggle, form;
  XmString     label;
  int          n = 0; 
  Arg          args[10];
  extern short debugTexture;
  ThreeDWidget *threeD = (ThreeDWidget*) threeDPanel->threeDWidget;

  label = tvXmString( "Debug Options" );
  XtSetArg( args[n], XmNdialogTitle, label ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  XtSetArg( args[n], XmNnoResize, TRUE ); n++;

  threeDPanel->mainForm[DEBUG_PANEL] =
    XmCreateFormDialog(parent, "threeDPanelForm", args, n);

  XmStringFree( label );

  /* Toggle button to freeze the display list */

  toggle = XtVaCreateManagedWidget("threeDPanelFreezeTilesWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[DEBUG_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  threeD->freezeButton = toggle;
  tvSetBoolean (threeD->threeD, TV_FREEZE_TILES,
		XmToggleButtonGadgetGetState(toggle));
  XtAddCallback( toggle, XmNvalueChangedCallback,
		 ThreeDPanelFreezeTilesToggleCB, threeDPanel );

  /* Button to return to frozen viewpoint. */

  form = XtVaCreateManagedWidget( "freezeForm", xmFormWidgetClass,
				  threeDPanel->mainForm[DEBUG_PANEL],
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, toggle,
				  XmNrightAttachment, XmATTACH_FORM,
				  NULL );
 
  threeDPanel->freezeReturn =
    XtVaCreateManagedWidget("threeDPanelFreezeReturnButton",
			    xmPushButtonGadgetClass, form,
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_FORM,
			    XmNbottomAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM, NULL );

  XtAddCallback (threeDPanel->freezeReturn, XmNactivateCallback,
		 ThreeDPanelFreezeReturnCB, threeDPanel);
  XtSetSensitive (threeDPanel->freezeReturn,
		  tvGetBoolean (threeD->threeD, TV_FREEZE_TILES));

  separator = PanelSeparator( threeDPanel->mainForm[DEBUG_PANEL], form );

#if 0
  /* Toggle button to turn on the debug texture map */

  toggle = XtVaCreateManagedWidget("threeDPanelDebugTextureWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[DEBUG_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  debugTexture = XmToggleButtonGadgetGetState(toggle);
  addToggleCallback (threeDPanel, toggle, &debugTexture);
  separator = PanelSeparator( threeDPanel->mainForm[DEBUG_PANEL], toggle );

  XtSetSensitive( toggle, DetailLoaded );
#endif

  /* Toggle button to turn on the debug HUD display */

  toggle = XtVaCreateManagedWidget("threeDPanelDebugHUDWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[DEBUG_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_HEADS_UP_DEBUG);
  separator = PanelSeparator( threeDPanel->mainForm[DEBUG_PANEL], toggle );

  /* Toggle button to show the debug tile effect */

  toggle = XtVaCreateManagedWidget("threeDPanelTileDebugWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[DEBUG_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_TILE_DEBUG);
  separator = PanelSeparator( threeDPanel->mainForm[DEBUG_PANEL], toggle );
  
  ThreeDPanelCreateUtilButtons( threeDPanel, separator, DEBUG_PANEL );
}

/*
 * Synopsis: void ThreeDPanelCreateNavWidgets()
 *
 * Description: Creates the widgets for the nav panel
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 7 February 1995
 *
 */

static void
ThreeDPanelCreateNavWidgets( ThreeDPanel *threeDPanel, Widget parent,
			     Boolean mode3d )
{
/*  static char  *funcName = "ThreeDPanelCreateNavWidgets";*/
  Widget       separator, toggle, widget, button, label, form;
  XmString     string;
  int          n = 0; 
  Arg          args[10];
  char         buffer[32];

  /* Create the dialog widget for the navigation options window */

  string = tvXmString( "Navigation Options" );
  XtSetArg( args[n], XmNdialogTitle, string ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  XtSetArg( args[n], XmNnoResize, TRUE ); n++;

  threeDPanel->mainForm[NAV_PANEL] =
    XmCreateFormDialog( parent, "threeDPanelForm", args, n );
  XmStringFree( string );
 
  /* Toggle button for the airplane controls feature */

  toggle = XtVaCreateManagedWidget("threeDPanelAirplaneWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);
  /* BLECH! -- this awkward, but only temporary. I've moved this variable from
   * ThreeDPanel into tvBase.  Lee's new API (in progress) will make this 
   * more reasonable.   --RLB
   */
  threeDPanel->threeDWidget->airplaneControl = 
      XmToggleButtonGadgetGetState(toggle);
  addToggleCallback (threeDPanel, toggle, 
      &threeDPanel->threeDWidget->airplaneControl);
  separator = PanelSeparator( threeDPanel->mainForm[NAV_PANEL], toggle );

  /* Toggle button for the height based navigation feature */

  toggle = XtVaCreateManagedWidget("threeDPanelHeightScaledWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNindicatorOn, True,
				   NULL);

  /* see comments above for airplaneControl --RLB */
  addToggleCoreCallback (threeDPanel, toggle, TV_HEIGHT_BASED_NAVIGATION);
  separator = PanelSeparator( threeDPanel->mainForm[NAV_PANEL], toggle );

  /* Toggle button for the 2D navigation constraint */

  threeDPanel->constrainWidget = toggle =
    XtVaCreateManagedWidget("threeDPanelTwoDConstrainWidget",
			    xmToggleButtonGadgetClass,
			    threeDPanel->mainForm[NAV_PANEL],
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, separator,
			    XmNindicatorOn, True,
			    XmNset, ! mode3d,
			    NULL);
  /* see comments above for airplaneControl --RLB */
  addToggleCoreCallback (threeDPanel, toggle, TV_TWOD_CONSTRAIN);
  separator = PanelSeparator( threeDPanel->mainForm[NAV_PANEL], toggle );

  /* Toggle button for seek mode */

  toggle = XtVaCreateManagedWidget("threeDPanelSeekModeWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNindicatorOn, True,
				   NULL);

  threeDPanel->seek = XmToggleButtonGadgetGetState(toggle);
  addToggleCallback (threeDPanel, toggle, &threeDPanel->seek);
  separator = PanelSeparator( threeDPanel->mainForm[NAV_PANEL], toggle );

  /* Toggle button for the draw on screen feature */

  toggle = XtVaCreateManagedWidget("threeDPanelDrawWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNindicatorOn, True,
				   NULL);
  threeDPanel->drawOnScreen = XmToggleButtonGadgetGetState(toggle);
  XtAddCallback( toggle, XmNvalueChangedCallback,
		 ThreeDPanelDrawToggleChangedCB, threeDPanel );

  /* Toggle button for the project to terrain feature */

  toggle = XtVaCreateManagedWidget("threeDPanelProjectToTerrainWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNindicatorOn, True,
				   NULL);
  addToggleCoreCallback (threeDPanel, toggle, TV_DOODLE_ON_TERRAIN);

  /* Input widget for the doodle timeout setting */

  label = XtVaCreateManagedWidget( "Timeout (secs) : ",
				   xmLabelGadgetClass, 
				   threeDPanel->mainForm[NAV_PANEL],
				   XmNtopOffset, 6,
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   NULL );

  form = XtVaCreateManagedWidget( "DrawForm", xmFormWidgetClass,
				  threeDPanel->mainForm[NAV_PANEL],
				  XmNleftOffset, 10,
				  XmNrightOffset, 10,
				  XmNtopAttachment, XmATTACH_WIDGET,
				  XmNtopWidget, label,
				  XmNleftAttachment, XmATTACH_FORM,
				  XmNrightAttachment, XmATTACH_FORM,
				  NULL );
				  
  string = tvXmString( "Clear" );
  button = XtVaCreateManagedWidget( "threeDPanelDrawClear",
				    xmPushButtonGadgetClass, form,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNlabelString, string, NULL);
  XmStringFree( string );
  threeDPanel->drawClear = button;
  XtAddCallback( button, XmNactivateCallback,
		 ThreeDPanelClearDoodleCB, threeDPanel );

  sprintf( buffer, "%1.1f", GetDrawTimeout() );
  widget = XtVaCreateManagedWidget( "threeDPanelTimeoutWidget",
				    xmTextFieldWidgetClass, form,
				    XmNcolumns, 3,
				    XmNvalue, buffer,
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_WIDGET,
				    XmNrightWidget, button,
				    XmNtopAttachment, XmATTACH_FORM,
				    XmNbottomAttachment, XmATTACH_FORM,
				    NULL );

  separator = PanelSeparator( threeDPanel->mainForm[NAV_PANEL], form );
  XtAddCallback( widget, XmNvalueChangedCallback,
		 ThreeDPanelTimeoutChangedCB, threeDPanel );

  ThreeDPanelCreateUtilButtons( threeDPanel, separator, NAV_PANEL );
}


/*
 * Synopsis: void ThreeDPanelCreateRenderWidgets()
 *
 * Description: Creates widgets for render panel
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: 1995
 *
 */

static void
ThreeDPanelCreateRenderWidgets( ThreeDPanel *threeDPanel, Widget parent )
{
/*  static char  *funcName = "ThreeDPanelCreateRenderWidgets";*/
  ThreeDWidget *threeDWidget = threeDPanel->threeDWidget;
  tvBase   *threeD = threeDWidget->threeD;
  Widget       separator, toggle, widget;
  XmString     label, buttons[3];
  XmButtonType button_type[3];
  int          n = 0; 
  Arg          args[12];


  label = tvXmString( "Render Options" );
  XtSetArg(args[n], XmNdialogTitle, label ); n++;
  XtSetArg( args[n], XmNverticalSpacing, 6 ); n++;
  XtSetArg( args[n], XmNhorizontalSpacing, 2 ); n++;
  XtSetArg( args[n], XmNnoResize, TRUE ); n++;

  threeDPanel->mainForm[RENDER_PANEL] =
    XmCreateFormDialog( parent, "threeDPanelForm", args, n );
  XmStringFree( label );
 
  /* Toggle button for showing lighting */

  toggle = XtVaCreateManagedWidget("threeDPanelShowLightingWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_LIGHTING);

  /* Toggle button for heads up display */
  
  threeDPanel->hudWidget = toggle =
    XtVaCreateManagedWidget("threeDPanelHeadsUpWidget",
			    xmToggleButtonGadgetClass,
			    threeDPanel->mainForm[RENDER_PANEL],
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, toggle,
			    XmNindicatorOn, True,
			    NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_HEADS_UP);

#if 0
  /* Toggle button for generating mipmaps */

  toggle = XtVaCreateManagedWidget("threeDPanelMipmapWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_MIPMAP_TEXTURES);
#endif

  /* Toggle button for show the tiles as wireframe */

  toggle = XtVaCreateManagedWidget("threeDPanelShowWireFrameWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_WIREFRAME);
  
  /* Toggle button to texture map the scene */

  toggle = XtVaCreateManagedWidget("threeDPanelShowTexturesWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_TEXTURES);

  /* Toggle button to texture map the scene */

  toggle = XtVaCreateManagedWidget("threeDPanelShowModelsWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_MODELS);

#if 0
  /* Toggle button to show the sky texture */

  toggle = XtVaCreateManagedWidget("threeDPanelShowSkyWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_SKY);
#endif

  /* Toggle button to texture map the scene */

  toggle = XtVaCreateManagedWidget("threeDPanelShowGroundWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_GROUND);

  /* Toggle button to texture map the scene */

  toggle = XtVaCreateManagedWidget("threeDPanelShowDatasetsWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SHOW_DATASETS);

  /* Toggle button for backfacing */

  toggle = XtVaCreateManagedWidget("threeDPanelBackfaceWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_BACKFACE_CULL);

  /* Toggle button for horizon culling */

  toggle = XtVaCreateManagedWidget("threeDPanelHorizonCullingWidget",
				   xmToggleButtonGadgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, toggle,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNindicatorOn, True,
				   NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_HORIZON_CULL);

  /* Toggle button to go into stereo mode */

  threeDPanel->stereoWidget = toggle = 
    XtVaCreateManagedWidget("threeDPanelStereoWidget",
			    xmToggleButtonGadgetClass,
			    threeDPanel->mainForm[RENDER_PANEL],
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, toggle,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNindicatorOn, True,
			    NULL);

  XtAddCallback( toggle, XmNvalueChangedCallback,
		 ThreeDPanelStereoToggleChangedCB, threeDPanel );
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );

  /* Toggle button to scale IPD */

  threeDPanel->stereoWidget = toggle = 
    XtVaCreateManagedWidget("threeDPanelScaleIPDWidget",
			    xmToggleButtonGadgetClass,
			    threeDPanel->mainForm[RENDER_PANEL],
			    XmNleftAttachment, XmATTACH_FORM,
			    XmNtopAttachment, XmATTACH_WIDGET,
			    XmNtopWidget, toggle,
			    XmNrightAttachment, XmATTACH_FORM,
			    XmNindicatorOn, True,
			    NULL);

  addToggleCoreCallback (threeDPanel, toggle, TV_SCALE_IPD);
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );

  /* change the tile popping strategy */

  label          = tvXmString( "Popping: " );
  buttons[0]     = tvXmString( "None" );
  buttons[1]     = tvXmString( "LowRes" );
  buttons[2]     = tvXmString( "LowFrag" );
  button_type[0] = XmPUSHBUTTON;
  button_type[1] = XmPUSHBUTTON;
  button_type[2] = XmPUSHBUTTON;

  n=0;
  XtSetArg( args[n], XmNbuttonCount, 3 ); n++;
  XtSetArg( args[n], XmNbuttons, buttons ); n++;
  XtSetArg( args[n], XmNbuttonType, button_type ); n++;
  XtSetArg( args[n], XmNbuttonSet, (int) threeD->poppingStrategy ); n++;
  XtSetArg( args[n], XmNsimpleCallback, ThreeDPanelPoppingCB ); n++;
  XtSetArg( args[n], XmNoptionLabel, label ); n++;
  XtSetArg( args[n], XmNleftAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
  XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
  XtSetArg( args[n], XmNtopWidget, separator ); n++;

  widget = XmCreateSimpleOptionMenu( threeDPanel->mainForm[RENDER_PANEL],
				     "dataset_menu", args, n );
  XtManageChild( widget );

  /* get the pulldown menupane widget for the options menu */
  XtVaGetValues( widget, XmNsubMenuId, &threeDPanel->poppingWidget, NULL );

  XmStringFree( buttons[0] );
  XmStringFree( buttons[1] );
  XmStringFree( buttons[2] );
  XmStringFree( label );

  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], widget );
  
  /* Controls the inter pupil distance */

  {
    float val = tvGetFloat(threeD, TV_ABSOLUTE_IPD);
    toggle = XtVaCreateManagedWidget("threeDPanelIPDWidget",
				     xmScaleWidgetClass,
				     threeDPanel->mainForm[RENDER_PANEL],
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, separator,
				     XmNorientation, XmHORIZONTAL,
				     XmNmaximum, 200,
				     XmNminimum, 0,
				     XmNdecimalPoints, 1,
				     XmNshowValue, True,
				     XmNvalue,
				     (int)(val*10),
				     NULL);
  }

  threeDPanel->ipdWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->ipdWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

  /* Controls the resolution adjustment factor */

  {
    float val = tvGetFloat(threeD, TV_RESOLUTION_ADJUST);
    toggle = XtVaCreateManagedWidget("threeDPanelResolutionWidget",
				     xmScaleWidgetClass,
				     threeDPanel->mainForm[RENDER_PANEL],
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, separator,
				     XmNorientation, XmHORIZONTAL,
				     XmNmaximum, 20,
				     XmNminimum, -5,
				     XmNdecimalPoints, 1,
				     XmNshowValue, True,
				     XmNvalue, (int)(val*10),
				     NULL);
  }

  threeDPanel->resWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->resWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

#if 0
  /* CONTROLS THE ELEVATION SCALING FACTOR */

  toggle =  XtVaCreateManagedWidget("threeDPanelScaleWidget",
				    xmScaleWidgetClass,
				    threeDPanel->mainForm[RENDER_PANEL],
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, separator,
				    XmNorientation, XmHORIZONTAL,
				    XmNmaximum, 200,
				    XmNminimum, 0,
				    XmNdecimalPoints, 1,
				    XmNshowValue, True,
				    XmNvalue, 10,
				    NULL);

  threeDPanel->scaleWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->scaleWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );
#endif

  /* Controls the FOV scaling factor */

  toggle = XtVaCreateManagedWidget("threeDPanelFOVWidget",
				   xmScaleWidgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNorientation, XmHORIZONTAL,
				   XmNmaximum, 60,
				   XmNminimum, 2,
				   XmNshowValue, True,
				   XmNvalue, THREED_INIT_FOV,
				   NULL);

  threeDPanel->fovWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->fovWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

#if 0
  /* Controls the near clipping plane */

  toggle = XtVaCreateManagedWidget("threeDPanelNearWidget",
				   xmScaleWidgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNorientation, XmHORIZONTAL,
				   XmNmaximum, 10000,
				   XmNminimum, 1,
				   XmNdecimalPoints, 2,
				   XmNshowValue, True,
				   XmNvalue, THREED_INIT_NEAR*100,
				   NULL);

  threeDPanel->nearWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->nearWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

  /* Controls the far clipping plane */

  toggle = XtVaCreateManagedWidget("threeDPanelFarWidget",
				   xmScaleWidgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNorientation, XmHORIZONTAL,
				   XmNmaximum, 200000,
				   XmNminimum, 1000,
				   XmNshowValue, True,
				   XmNvalue, THREED_INIT_FAR,
				   NULL);

  threeDPanel->farWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->farWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );
#endif

  /* Controls the fog scaling factor: 0 = none, 1 = heavy */

  toggle =  XtVaCreateManagedWidget("threeDPanelFogWidget",
				    xmScaleWidgetClass,
				    threeDPanel->mainForm[RENDER_PANEL],
				    XmNleftAttachment, XmATTACH_FORM,
				    XmNrightAttachment, XmATTACH_FORM,
				    XmNtopAttachment, XmATTACH_WIDGET,
				    XmNtopWidget, separator,
				    XmNorientation, XmHORIZONTAL,
				    XmNmaximum, 1000,
				    XmNminimum, 0,
				    XmNshowValue, True,
				    XmNdecimalPoints, 3,
				    XmNvalue, 0,
				    NULL);

  threeDPanel->fogWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->fogWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

  /* Controls the time of day */

  toggle = XtVaCreateManagedWidget("threeDPanelLightWidget",
				   xmScaleWidgetClass,
				   threeDPanel->mainForm[RENDER_PANEL],
				   XmNleftAttachment, XmATTACH_FORM,
				   XmNrightAttachment, XmATTACH_FORM,
				   XmNtopAttachment, XmATTACH_WIDGET,
				   XmNtopWidget, separator,
				   XmNorientation, XmHORIZONTAL,
				   XmNminimum, 50,
				   XmNmaximum, 190,
				   XmNshowValue, True,
				   XmNdecimalPoints, 1,
				   XmNvalue, 120,
				   NULL);

  threeDPanel->lightWidget = toggle;
  separator = PanelSeparator( threeDPanel->mainForm[RENDER_PANEL], toggle );
  XtAddCallback( threeDPanel->lightWidget, XmNvalueChangedCallback,
		 ThreeDPanelSliderChangedCB, threeDPanel );

  ThreeDPanelCreateUtilButtons( threeDPanel, separator, RENDER_PANEL );
}

/*
 * Synopsis: void ThreeDPanelPoppingCB( Widget, XtPointer, XtPoitner )
 *
 * Description:
 *   This callback is invoked whenever the users selects a new tile
 *   popping strategy from the Render Options panel. 
 *
 * Externals:
 *   widgetList
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 8 June 1999
 *
 */

static void
ThreeDPanelPoppingCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    const char  *funcName = "ThreeDPanelPoppingCB";
    int         value = (int) client_data;
    list_iter_t i;

    w = XtParent( w );  /* get the pulldown menu parent from this button */

    /* Find the ThreeDWidget that this event came from. The OptionsMenu */
    /* simpleCallback only passes the number of the option selected.    */

    if (widgetList) {
      for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
	ThreeDWidget *threeDWidget = (ThreeDWidget*) list_value(i);
	if ( threeDWidget && threeDWidget->threeDPanel ) {
	  if ( threeDWidget->threeDPanel->poppingWidget == w ) {

	    /* found the threeDWidget! Now set its popping strategy */
	    tvSetInteger (threeDWidget->threeD,
			  TV_POPPING_STRATEGY, value);
	    TSMTRACE(( THREED_MASK|1, funcName, 
		       "Popping Strategy = %d for 3-D Widget %p",
		       value, threeDWidget ));
	  }
	}
      }
    }
}

/*
 * Synopsis: void ThreeDPanelSliderChangedCB(Widget,
 *					  XtPointer,
 *					  Xtpointer)
 *
 * Description: When the user clicks and drags the sacle widget, this callback
 *      is called and the value is retrieved from the widget. The value is
 *      copied to the threeDWidget * for later use.
 *
 * Externals:
 *
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date: July 1994
 *
 */
static void
ThreeDPanelSliderChangedCB(Widget    w,
			   XtPointer client_data,
			   XtPointer call_data)
{
/* static char * funcName = "ThreeDPanelSliderChangedCB";*/
 ThreeDPanel * threeDPanel = (ThreeDPanel *) client_data;
 ThreeDWidget *threeD = (ThreeDWidget *) threeDPanel->threeDWidget;
 tvViewpoint *view = tvBaseView (threeD->threeD);
 int val;


    /* GET THE VALUE FROM THE WIDGET AND SAVE IT FOR LATER USE */
    XmScaleGetValue(w, &val);

    if (w == threeDPanel->resWidget)
    {
      tvSetFloat (threeD->threeD, TV_RESOLUTION_ADJUST, val/10.0);
    }
    else if (w == threeDPanel->scaleWidget)
    {
      tvSetFloat (threeD->threeD, TV_ELEVATION_SCALE, val/10.0);
    }
    else if(w == threeDPanel->fovWidget)
    {
	view->fov = val;
	tvSetView (threeD->threeD, view, FALSE);
    }
    else if(w == threeDPanel->nearWidget)
    {
	view->nearPlane = val * .01;
	tvSetView (threeD->threeD, view, FALSE);
    }
    else if(w == threeDPanel->farWidget)
    {
	view->farPlane = val;
	tvSetView (threeD->threeD, view, FALSE);
    }
    else if(w == threeDPanel->fogWidget)
    {
        tvSetFloat(threeD->threeD, TV_FOG_DENSITY, (float) val * 1e-3);
    }
    else if(w == threeDPanel->lightWidget)
    {
        tvSetFloat(threeD->threeD, TV_TIME_OF_DAY, (float)val / 10.0);
    }
    else if(w == threeDPanel->ipdWidget)
    {
      /* actually put it in meters, not cm */
      tvSetFloat (threeD->threeD, TV_ABSOLUTE_IPD, (float) val/100.0);
    }

    tvBasePostRedraw (threeD->threeD, FALSE);
}

/*
 * Synopsis: void ThreeDPanelToggleChangedCB(Widget, XtPointer, XtPointer)
 *
 * Description: When a toggle button is
 *	pressed, tis callback is called. The flag in ThreeDPanel is changed
 *	to reflect the changed state.
 *
 * Externals:
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date: 1993
 *
 */

static void
ThreeDPanelToggleChangedCB(Widget    w,
			   XtPointer client_data,
			   XtPointer call)
{
/*  static char * funcName = "ThreeDPanelToggleChangedCB";*/
  ToggleCallback *cb = (ToggleCallback *) client_data;
  ThreeDWidget *threeDWidget = cb->threeDPanel->threeDWidget;

  /* GET THE STATE OF THE BUTTON AND CHANGE THE APPROPRIATE FLAG */
  *cb->flag = XmToggleButtonGadgetGetState (w);

  if (w == cb->threeDPanel->stereoWidget)
    ThreeDWidgetSetStereoMode(threeDWidget, *cb->flag);

  tvBaseUpdateView( threeDWidget->threeD, FALSE);
  ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
}

/*
 * Synopsis: void ThreeDPanelToggleCoreChangedCB(Widget, XtPointer, XtPointer)
 *
 * Description: When a toggle button is
 *	pressed, tis callback is called. The flag in ThreeDPanel is changed
 *	to reflect the changed state.
 *
 * Externals:
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date: 1993
 *
 */

static void
ThreeDPanelToggleCoreChangedCB(Widget    w,
			   XtPointer client_data,
			   XtPointer call)
{
/*  static char * funcName = "ThreeDPanelToggleCoreChangedCB";*/
  ToggleCoreCallback *cb = (ToggleCoreCallback *) client_data;
  ThreeDWidget *threeDWidget = cb->threeDPanel->threeDWidget;
  PRBool state = XmToggleButtonGadgetGetState (w);

  if (tvSetBoolean (threeDWidget->threeD, cb->var, state) != state) {
    /*tvBaseUpdateView( threeDWidget->threeD, FALSE, FALSE );*/
    ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
  }
}

static void
ThreeDPanelStereoToggleChangedCB(Widget    w,
				 XtPointer client_data,
				 XtPointer call)
{
/*  static char * funcName = "ThreeDPanelToggleCoreChangedCB";*/
  ThreeDPanel *threeDPanel = client_data;
  ThreeDWidget *threeDWidget = threeDPanel->threeDWidget;
  PRBool state = XmToggleButtonGadgetGetState (w);

  ThreeDWidgetSetStereoMode(threeDWidget, state);
}

static void
ThreeDPanelDrawToggleChangedCB( Widget w, XtPointer client_data,
				XtPointer call )
{
/*  static char * funcName = "ThreeDPanelDrawToggleChangedCB";*/
  ThreeDPanel  *threeDPanel  = (ThreeDPanel *) client_data;
  ThreeDWidget *threeDWidget = threeDPanel->threeDWidget;

  /* GET THE STATE OF THE DRAW BUTTON AND CHANGE THE APPROPRIATE FLAG */
  threeDPanel->drawOnScreen = XmToggleButtonGadgetGetState (w);

  /* Whenever we click over the Draw On Screen button, we want to */
  /* make sure that we aren't moving and refresh the display.     */

  tvStopMotion( threeDWidget->threeD );
  tvBasePostRedraw( threeDWidget->threeD, FALSE );
  ThreeDWidgetUpdateStatusBar( threeDWidget, NULL );
}

static void
ThreeDPanelTimeoutChangedCB( Widget w, XtPointer client_data, XtPointer call )
{
/*  static char  *funcName = "ThreeDPanelTimeoutChangedCB";*/
/*  ThreeDPanel  *threeDPanel  = (ThreeDPanel *) client_data;*/
/*  ThreeDWidget *threeDWidget = threeDPanel->threeDWidget;*/
  char         *string;

  string = XmTextFieldGetString( w );
  SetDrawTimeout( atof(string) );

}

static void
ThreeDPanelClearDoodleCB( Widget w, XtPointer client_data, XtPointer call )
{
/*  static char  *funcName = "ThreeDPanelClearDoodleCB";*/
  ThreeDPanel  *threeDPanel  = (ThreeDPanel *) client_data;
  ThreeDWidget *threeDWidget = threeDPanel->threeDWidget;

  ClearSentence( threeDWidget->threeD );
  tvBasePostRedraw( threeDWidget->threeD, FALSE );
}

/*
 * Synopsis: void ThreeDPanelGetButtonStates()
 *	
 *
 * Description: Get the initial state of the widgets from resource file
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: December 9, 1994
 *
 */
static void 
ThreeDPanelGetButtonStates(ThreeDPanel * threeDPanel)
{
  static char const *funcName = "ThreeDPanelGetButtonStates";
  Widget w;
  ThreeDWidget *threeDWidget = (ThreeDWidget*) threeDPanel->threeDWidget;

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelAirplaneWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    /* BLECH!  Very awkward at the moment, but only temporary.  This 
     * state variable is now part of tvBase
     * --RLB
     */
    threeDPanel->threeDWidget->airplaneControl = 
	XmToggleButtonGadgetGetState(w);
  }

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelHeightScaledWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDPanel->threeDWidget->threeD,
		  TV_HEIGHT_BASED_NAVIGATION,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelTwoDConstrainWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    /* see comments above about airplaneControl --RLB */
    tvSetBoolean (threeDPanel->threeDWidget->threeD, TV_TWOD_CONSTRAIN,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelSeekModeWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    threeDPanel->seek = XmToggleButtonGadgetGetState(w);
  }

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelProjectToTerrainWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    threeDWidget->threeD->projectToTerrain = XmToggleButtonGadgetGetState(w);
  }

  w = XtNameToWidget(threeDPanel->mainForm[NAV_PANEL],
		     "*threeDPanelDrawWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    threeDPanel->drawOnScreen = XmToggleButtonGadgetGetState(w);
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowLightingWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_LIGHTING,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelHeadsUpWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_HEADS_UP,
		  XmToggleButtonGadgetGetState(w));
  }

#if 0
  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelMipmapWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_MIPMAP_TEXTURES,
		  XmToggleButtonGadgetGetState(w));
  }
#endif

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowWireFrameWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_WIREFRAME,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowTexturesWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_TEXTURES,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowModelsWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_MODELS,
		  XmToggleButtonGadgetGetState(w));
  }

#if 0
  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowSkyWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_SKY,
		  XmToggleButtonGadgetGetState(w));
  }
#endif

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowGroundWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_GROUND,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelShowDatasetsWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SHOW_DATASETS,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelHorizonCullingWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_HORIZON_CULL,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelStereoWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_STEREO,
		  XmToggleButtonGadgetGetState(w));
  }

  w = XtNameToWidget(threeDPanel->mainForm[RENDER_PANEL],
		     "*threeDPanelScaleIPDWidget");
  if (! w) {
    tsmError (funcName, "Can't find button widget");
  } else {
    tvSetBoolean (threeDWidget->threeD, TV_SCALE_IPD,
		  XmToggleButtonGadgetGetState(w));
  }

}

/*
 * Synopsis: void ThreeDPanelManageWidget()
 *
 * Description: Based on the panel type, we manage the widget
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: July 1994
 *
 */

void
ThreeDPanelManageWidget( ThreeDPanel *threeDPanel, ThreeDPanelType type )
{
/*  static char *funcName = "ThreeDPanelManageWidget";*/

  XtManageChild( threeDPanel->mainForm[type] );
  threeDPanel->managed[type] = TRUE;
}

/*
 * Synopsis: void ThreeDPanelFreezeTilesToggleCB()
 *	
 * Description: Toggles if we should freeze the tiles
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: 1995
 *
 */
static void
ThreeDPanelFreezeTilesToggleCB(Widget    w,
				XtPointer client_data,
				XtPointer call_data)
{
/*  static char * funcName = "ThreeDPanelFreezeTilesToggleCB";*/
  ThreeDPanel * threeDPanel = (ThreeDPanel *) client_data;
  ThreeDWidget * threeDWidget = (ThreeDWidget*) threeDPanel->threeDWidget;
  int state = XmToggleButtonGadgetGetState(w);

  /* GET THE STATE OF THE BUTTON AND CHANGE THE APPROPRIATE FLAG */
  tvSetBoolean (threeDWidget->threeD, TV_FREEZE_TILES, state);
  XtSetSensitive (threeDPanel->freezeReturn, state);
}

/*
 * Synopsis: void ThreeDPanelFreezeReturnCB()
 *	
 * Description: Return to frozen viewpoint.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Lee Iverson
 * 
 * Date: 1999
 *
 */
static void
ThreeDPanelFreezeReturnCB(Widget    w,
			  XtPointer client_data,
			  XtPointer call_data)
{
/*  static char * funcName = "ThreeDPanelFreezeReturnCB";*/
  ThreeDPanel * threeDPanel = (ThreeDPanel *) client_data;

  tvBaseSetFrozenView (threeDPanel->threeDWidget->threeD);
}

/*
 *
 * Synopsis:
 *     static void ThreeDPanelCreateUtilButtons(ThreeDPanel *, Widget);
 *	
 * Description: Create utility buttons, such as "Dismiss', "help" etc.
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author: Stephen Lau
 * 
 * Date: July 1994
 *
 */

static void
ThreeDPanelCreateUtilButtons( ThreeDPanel *threeDPanel, Widget topWidget,
			      ThreeDPanelType type )
{
/*  static char   *funcName = "ThreeDPanelCreateUtilButtons";*/
  XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
  Widget        parent = threeDPanel->mainForm[type];
  XmString      label;
  PanelCallback *panelCB = (PanelCallback *)malloc(sizeof(PanelCallback));

  panelCB->threeDPanel = threeDPanel;
  panelCB->val = type;
  threeDPanel->panelCB = panelCB; /* save to kill later */

  /* All of these buttons have a callback to 
   * ThreeDPanelDismissCallback and pass a pointer to 
   * the corrseponding ThreeDPanel structure
   */

  callbacks[0].callback = ThreeDPanelDismissCallback;
  callbacks[0].closure = (XtPointer) panelCB;

  threeDPanel->dismissButton[type] = 
    XtVaCreateManagedWidget("ThreeDPanelBoard",
			    xmRowColumnWidgetClass, parent,
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
			    NULL);
  
  XtVaSetValues( topWidget, XmNbottomAttachment, XmATTACH_WIDGET,
		 XmNbottomWidget, threeDPanel->dismissButton[type], NULL );

  /* Create the dismiss button */

  label = tvXmString( "Dismiss" );
  XtVaCreateManagedWidget("ThreeDPanelDismiss",
			  xmPushButtonGadgetClass,
			  threeDPanel->dismissButton[type],
			  XmNleftOffset, 20,
			  XmNrightOffset, 20,
			  XmNactivateCallback, callbacks,
			  XmNshowAsDefault, 1,
			  XmNlabelString, label, NULL);
  XmStringFree( label );
}

/*
 * Synopsis:
 *    static void ThreeDPanelDismissCallback(Widget,
 *                                        XtPointer,
 *                                        XtPointer)
 *
 * Description: Callback function for actions such as "Dismiss" button,
 *         Function unmanages the threeDPanel widget
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *	July 1994
 *
 */
static void
ThreeDPanelDismissCallback(Widget    w,
			   XtPointer client_data,
			   XtPointer call_data)

{
/* static char * funcName = "ThreeDPanelDismissCallback";*/
 PanelCallback * panelCallback = (PanelCallback *) client_data;
 ThreeDPanel *threeDPanel = panelCallback->threeDPanel;

    /* UNMANAGE THE WIDGET */
    XtUnmanageChild(threeDPanel->mainForm[panelCallback->val]);
    threeDPanel->managed[panelCallback->val] = FALSE;

}


/*
 * Synopsis:
 *     void ThreeDPanelDestroy(ThreeDPanel *threeDPanel)
 *
 * Description: Clears memory of ThreeDPanel, when it's time to call it a day
 *         Function unmanages the threeDPanel widget
 *
 * Externals: NONE
 *
 * Returns: NOTHING
 *
 * Author:
 *		Nat Bletter
 * 
 * Date:
 *	Feb 24 1999
 *
 */
 void
ThreeDPanelDestroy(ThreeDPanel *threeDPanel)

{
/* static char * funcName = "ThreeDPanelDestroy";*/

 /*
 for (type = RENDER_PANEL; type < NO_OF_PANELS; type++) {
   XtUnmanageChild(threeDPanel->mainForm[type]);
   threeDPanel->managed[type] = FALSE;
 }
 */

 /* kill off it's memory */
 free(threeDPanel->panelCB);
 
 free(threeDPanel);
}


/*
 * Synopsis: void ThreeDPanelCreatePositionWidgets()
 *	
 * Description: Creates a panel to diaplay current location and viewpoint
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 */
static void
ThreeDPanelCreatePositionWidgets(ThreeDPanel * threeDPanel,
				 Widget        parent)
{
/* static char * funcName = "ThreeDPanelCreatePositionWidgets";*/
 Widget       separator, labelWidget, formWidget;
 int          n = 0; 
 Arg          args[10];
 XmString     label;

    label = tvXmString("Viewpoint Information");
    XtSetArg(args[n], XmNdialogTitle, label), n++;

    /*
     * THE UNDERLYING WIDGET IS A FORM WIDGET THAT ACTS AS A DIALOG BOX.
     * ALL OTHER WIDGETS FOR THE OVERHEAD VIEW ARE DERVIED FROM THIS ONE
     *
     */
    threeDPanel->mainForm[POSITION_PANEL] =
       XmCreateFormDialog(parent, "threeDPanelForm", args, n);
    XmStringFree (label);

    /* SET THE SPACING OF THE DIALOG BOX */
    XtVaSetValues(threeDPanel->mainForm[POSITION_PANEL],
		  XmNverticalSpacing, 6,
		  XmNhorizontalSpacing, 2,
		  XmNnoResize, TRUE,
		  NULL);

    labelWidget =
       XtVaCreateManagedWidget("threeDPanelViewpointLabelWidget",
			       xmLabelGadgetClass,
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       NULL);
    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNheight, 6,
			       XmNshadowThickness, 6,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, labelWidget,
			       XmNtopOffset, 2,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("formWidget",
			       xmFormWidgetClass,
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNverticalSpacing, 2,
			       XmNhorizontalSpacing, 20,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    label = tvXmString("X :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->positionLabelWidget[0] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       formWidget,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, 
			       threeDPanel->positionLabelWidget[0],
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNorientation, XmVERTICAL,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNtopAttachment, XmATTACH_FORM,
			       NULL);

    label = tvXmString("Y :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, separator,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->positionLabelWidget[1] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       formWidget,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, 
			       threeDPanel->positionLabelWidget[1],
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNorientation, XmVERTICAL,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       NULL);

    label = tvXmString("Z :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, separator,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->positionLabelWidget[2] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       NULL);

    formWidget =
       XtVaCreateManagedWidget("formWidget",
			       xmFormWidgetClass,
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNverticalSpacing, 2,
			       XmNhorizontalSpacing, 15,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, separator,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       NULL);

    label = tvXmString("Heading :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->orientationLabelWidget[0] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       formWidget,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, 
			       threeDPanel->orientationLabelWidget[0],
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNorientation, XmVERTICAL,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       NULL);

    label = tvXmString("Pitch :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, separator,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->orientationLabelWidget[1] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       formWidget,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, 
			       threeDPanel->orientationLabelWidget[1],
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNorientation, XmVERTICAL,
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       NULL);

    label = tvXmString("Roll :");
    labelWidget =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, separator,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    label = tvXmString("N/A");
    threeDPanel->orientationLabelWidget[2] =
       XtVaCreateManagedWidget("threeDPanelLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNtopAttachment, XmATTACH_FORM,
			       XmNbottomAttachment, XmATTACH_FORM,
			       XmNleftAttachment, XmATTACH_WIDGET,
			       XmNleftWidget, labelWidget,
			       XmNlabelString, label,
			       NULL);
    XmStringFree (label);

    separator =
       XtVaCreateManagedWidget("separator", xmSeparatorGadgetClass, 
			       threeDPanel->mainForm[POSITION_PANEL],
			       XmNheight, 4,
			       XmNshadowThickness, 4,
			       XmNleftAttachment, XmATTACH_FORM,
			       XmNrightAttachment, XmATTACH_FORM,
			       XmNtopAttachment, XmATTACH_WIDGET,
			       XmNtopWidget, formWidget,
			       NULL);
 
    ThreeDPanelCreateUtilButtons( threeDPanel, separator, POSITION_PANEL );

}

/*
 * Synopsis: void ThreeDPanelUpdatePosition()
 *	
 *
 * Description: Changes the fields in the position panel 
 *     to new location
 *
 *
 * Externals:
 *
 *
 * Returns: NOTHING
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 */
void
ThreeDPanelUpdatePosition(ThreeDPanel * threeDPanel)
{
/* static char * funcName = "ThreeDPanelUpdatePosition";*/
  ThreeDWidget * threeDWidget;
  char buffer[200];
  tvViewpoint const *view;
  XmString label;

  threeDWidget = (ThreeDWidget *) threeDPanel->threeDWidget;
  view = tvBaseView (threeDWidget->threeD);

  sprintf(buffer,"%4.2f", view->eye[0]);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->positionLabelWidget[0],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);

  sprintf(buffer,"%4.2f", view->eye[1]);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->positionLabelWidget[1],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);

  sprintf(buffer,"%4.2f", view->eye[2]);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->positionLabelWidget[2],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);

  sprintf(buffer,"%4.2f", view->yaw * 180.0 / M_PI);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->orientationLabelWidget[0],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);

  sprintf(buffer,"%4.2f", view->pitch * 180.0 / M_PI);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->orientationLabelWidget[1],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);

  sprintf(buffer,"%4.2f", view->roll * 180.0 / M_PI);
  label = tvXmString(buffer);
  XtVaSetValues(threeDPanel->orientationLabelWidget[2],
		XmNlabelString, label,
		NULL);
  XmStringFree (label);
}
