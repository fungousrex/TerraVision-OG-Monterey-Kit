/*
 * Name:  Features.h
 *
 * Description: Header file for drawing building models
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

#ifndef FEATURESFE_H
#define FEATURESFE_H

typedef struct FeatureCallback FeatureCallback;

#include <X11/Intrinsic.h>

#include <tsm/tsm.h>

#include "ThreeDWidget.h"

#define MAX_FEATURES   100       /* max number of feature files supported */

/* FeatureCallback is the structure that is passed to the Feature      */
/* Selection callback routines. It gives enough information to specify */
/* the model file that should be loaded, and the 3D widget to refresh. */

struct FeatureCallback {
  TsmModel     *model;           /* model structure for selected model */
  int          curr_loc;         /* current entry in location array */
  ThreeDWidget *threeDWidget;    /* 3-D widget to refresh */
  Widget       info_widget;      /* the "info" push button widget */
};

/* pop up and down the Feature Selection dialog */

void FeatureSelectShow( ThreeDWidget *threeDWidget );

void FeatureSelectHide( ThreeDWidget *threeDWidget );

void FeatureSelectRefresh( ThreeDWidget *threeDWidget );


#endif

