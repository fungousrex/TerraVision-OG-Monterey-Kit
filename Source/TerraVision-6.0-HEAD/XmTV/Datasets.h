/*
 * Name: DATASETS.C
 *
 * Description:
 *     Various routines and GUIs to support TerraVision's multiple
 *     dataset facilities, e.g. selecting datasets to display, setting
 *     display priorities and options, etc.
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

#ifndef DATASETS_H
#define DATASETS_H

#include "ThreeDWidget.h"

/* typedef specifing the callback function type for SelectDataset() */

typedef void (DatasetSelectProc) (ThreeDWidget *, TsGeoPyramid *, void *);


/* SelectDataset provides a generic GUI that displays a list of */
/* available datasets and performs some operation on the one    */
/* that the user selects.                                       */

void SelectDataset( ThreeDWidget *threeDWidget, Widget *, char const *title,
		    char const *button, DatasetSelectProc *callback,
		    void *closure );

void RefreshDatasetSelectors( void );


/* routines to control Dataset Priority dialog */

void DatasetPrioInit( ThreeDWidget *threeDWidget );
void DatasetPrioShow( ThreeDWidget *threeDWidget );
void DatasetPrioHide( ThreeDWidget *threeDWidget );
void DatasetPrioRefreshWidget( ThreeDWidget *threeDWidget );


/* routines to control the display of any subset of loaded datasets */

void DatasetDisplayInit( ThreeDWidget *threeDWidget );
void DatasetDisplayHide( ThreeDWidget *threeDWidget );
void DatasetDisplayShow( ThreeDWidget *threeDWidget );
void DatasetDisplayRefreshWidget( ThreeDWidget *threeDWidget );

/* routines to control the remove of datasets from our global list */

void RemoveDataset( void );


#endif
