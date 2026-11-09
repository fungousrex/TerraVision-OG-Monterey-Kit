/*
 * Name:  ThreeDPanel.h
 *
 * Description: Header file for toggle button panel
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

#ifndef THREEDPANEL_H
#define THREEDPANEL_H

#include <X11/Intrinsic.h>



typedef enum ThreeDPanelType {
  RENDER_PANEL = 0,
  DEBUG_PANEL,
  POSITION_PANEL,
  NAV_PANEL,
  NO_OF_PANELS
} ThreeDPanelType;

typedef struct ThreeDPanel ThreeDPanel;

typedef struct PanelCallback {
  ThreeDPanel *threeDPanel;
  int         val;
} PanelCallback;

struct ThreeDPanel {
  Widget mainForm[NO_OF_PANELS];
  Widget dismissButton[NO_OF_PANELS];
  int    managed[NO_OF_PANELS];
  Widget positionLabelWidget[3];
  Widget orientationLabelWidget[3];
  Widget scaleWidget;
  Widget stereoWidget;
  Widget ipdWidget;
  Widget resWidget;
  Widget fovWidget;
  Widget lightWidget;
  Widget fogWidget;
  Widget nearWidget;
  Widget farWidget;
  Widget drawClear;
  Widget freezeReturn;
  Widget poppingWidget;
  Widget constrainWidget;
  Widget hudWidget;
  struct ThreeDWidget *threeDWidget;
  short  fullScreen;
/*RLB  short  airplaneControl;*/
/*RLB  short  heightBasedNav;*/
/*RLB  short  twoDConstrain;*/
  short  seek;
  short  drawOnScreen;
  short  animateBookmarks;
  PanelCallback *panelCB;
};

ThreeDPanel *CreateThreeDPanel(Widget, void *, Boolean);
void ThreeDPanelManageWidget(ThreeDPanel *, ThreeDPanelType);
void ThreeDPanelUpdatePosition(ThreeDPanel *);
void ThreeDPanelDestroy(ThreeDPanel *);

#endif
