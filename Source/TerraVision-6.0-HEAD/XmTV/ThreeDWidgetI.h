/*
 * Name:  
 *	ThreeDWidgetI.h
 *
 * Description: 
 *	Internal header file for a threeD widget.
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
#ifndef THREEDWIDGETI_H
#define THREEDWIDGETI_H

#include <X11/Intrinsic.h>

#include "ThreeDWidget.h"
#include "ThreeDControl.h"
#include "ThreeDCore.h"

#include "TsGeoPyramid.h"
#include "Link.h"
#include "FeaturesFE.h"

#include "matrix.h"
#include "pixmaps.h"

#include <pthr/list.h>

/* A list of all ThreeDWidgets */
extern list_t * widgetList;

#define MAX_ROLL 5.0

/* Height at which acceleration is defined, for height-scaled navigation
   (in meters) */ 
#define ACCEL_BASE_HEIGHT  100

struct ThreeDWidget {
  Widget 	     windowArea;
  Widget             statusBarForm;
  Widget             topForm;
  Widget             bottomForm;
  /* Status bar label widgets */
  Widget             statusBarLabel1;
  Widget             statusBarLabel2;
  Widget             statusBarLabel3;
  Widget             statusBarLabel4;
  /* status bar state so we only refresh new logistics */
  char               *statusBarViewpoint;  
  int                statusBarDatasets;
  int                statusBarNavMode;
  int                statusBarViewMode;
  /* Menu button widgets */
  Widget             datasetDisplayButton;
  Widget             zbufferButton;
  Widget             priorityButton;
  Widget             viewpointButton;
  Widget             viewpointBookmarksButton;
  Widget             viewpointStatusButton;
  Widget             pathNewButton;
  Widget             pathLoadButton;
  Widget             pathSaveButton;
  Widget             pathEditButton;
  Widget             pathPlayButton;
  Widget             pathStopButton;
  Widget             pathStartButton;
  Widget             pathDisplayButton;
  Widget             pathNextButton;
  Widget             pathPrevButton;
  Widget             pathEndButton;
  Widget             pathPauseButton;
  Widget             pathMovie1Button;
  Widget             pathMovie2Button;
  Widget             debugButton;
  Widget             rendererButton;
  Widget             navButton;
  Widget             featureButton;
  Widget	     linkButton;
  Widget             freezeButton;
  Widget             showMenuWidget;
  Widget             showIconsWidget;
  Widget             showFullscreenWidget;
  Widget             showStatusBarWidget;
  Widget             dismissOption;
  Widget             mosaicWidgets[NUMB_WEB_ICONS];
  Boolean            pixmapsDone;
  /* remember the maximum screen size */
  int                screenMax[2];
  /* window size and dimensions in the non-fullscreen mode */
  int                unzoomedX;        /* window x-coord for non fullscrn */
  int                unzoomedY;        /* window y-coord for non fullscrn */
  int                unzoomedWidth;    /* window width for non fullscrn */
  int                unzoomedHeight;   /* window height for non fullscrn */
  /* Dataset Select dialog state */
  Widget             dsWidget;         /* the Dataset Select widget */
  Widget             dsList1;          /* the list of loaded datasets */
  Widget             dsList2;          /* the list of displayed datasets */
  /* Viewpoint Bookmarks state */
  Widget             VpWidget;         /* view bookmarks dialog widget */
  Widget             VpList;           /* the scrolled list widget */
  Widget             VpSet;            /* The "Set View" button */
  Widget             VpAdd;            /* The "Add View" button */
  Widget             VpDel;            /* The "Del View" button */
  Widget             VpAddWidget;      /* add bookmark dialog */
  Widget             VpDelWidget;      /* remove bookmark confirmation */
  Widget             VpAddSelector;    /* Dataset Selector dialog */
  int                currBookmark;     /* used to cycle between views */
  /* Flight paths state */
  Widget             pathEditWidget;   /* path new/edit window */
  Widget             pathListWidget;   /* list of paths in edit/new window */
  Widget             pathTitleWidget;  /* text widget for title */
  Widget             pathVelWidget;    /* text widget for velocity */
  Widget             pathLoopWidget;   /* toggle widget for looping */
  Widget             pathCloseLoopWidget;   /* toggle widget for looping */
  Widget             pathScaleWidget;  /* toggle widget for speed scaling */
  Widget             pathInterp[3];    /* toggle buttons for interp sel */
  Widget             pathAddWidget;    /* button to add a viewpoint */
  Widget             pathDelWidget;    /* button to delete a viewpoint */
  Widget             pathUpdtWidget;   /* button to goto a viewpoint */
  Widget             pathDupWidget;    /* button to duplicate a viewpoint */
  Widget             pathUpWidget;     /* button to bubble viewpoint up */
  Widget             pathDnWidget;     /* button to bubble viewpoint down */
  Widget             pathLoadSel;      /* file selector for path load */
  Widget             pathSaveSel;      /* file selector for path load */
  /* Feature selection state */
  Widget             fselWidget;       /* main form widget for selection */
  Widget             fselSelect;       /* scrolled selection widget */
  Widget             fselWireframe;    /* the wireframe toggle button */
  Widget             fselTextured;     /* the textured toggle button */
  Widget             fselLighting;     /* the lighting toggle button */
  Widget             fselLoadTextures; /* load textures for models? */
  Widget             fselBackfaceCull; /* Do backface cull for all objects?*/
  Widget             fselRowCol;       /* the rowcol to list all models */
  Widget             fselInfoText;     /* the scrolled text info list */
  
  /* Dataset Priority GUI */
  Widget             prioWidget;       /* Dataset Priority dialog widget */
  Widget             prioList;         /* the list widget for all datasets */
  
  ThreeDControl     *threeDControl;
  ThreeDPanel       *threeDPanel;
  tvBase            *threeD;
  
  /* menus in link  (These formerly resided in ShareWidgetInfo) */
  Widget             linkMenu;
  Widget             moveMenu;
  Widget             frustumMenu;
  Widget             tileMenu;
  Widget             pathMenu;
  Widget             mainForm;
  Widget             menuBar;  /* overall menuBar */
  char 	             *title;
  
  BusyDialog         *busyDialog;
  
  FeatureCallback    fselCBs[MAX_FEATURES]; /* list of callback structs */
  int                fselNumbCBs;      /* current callback number */

  int 	             xOrig;
  int 	             yOrig;
  int 	             xMouseDown;
  int 	             yMouseDown;
  int                xMouse;
  int                yMouse;

  short	             flyMode;
  short              airplaneControl;

  short              mouseDown;   /* TRUE while mouse key(s) are pressed */
  short              shiftDown;   /* TRUE while shift key(s) are pressed */
  short              ctrlDown;    /* TRUE while control key(s) are pressed */
  short              altDown;     /* TRUE while alt key(s) are pressed */

  int                showMenu;         /* show the menu bar? */
  int                showIcons;        /* show the web icons? */
  int                showFullscreen;   /* zoomed to full screen? */
  int                showStatusBar;    /* display the status bar? */

  tvViewpoint        originView;
};

#ifdef SPACEBALL_H
extern void ThreeDWidgetProcessSbEvent(ThreeDWidget *, XEvent *);
#endif

extern void DestroyThreeDWidget (ThreeDWidget *threeD);

#endif /* THREEDWIDGETI_H */

