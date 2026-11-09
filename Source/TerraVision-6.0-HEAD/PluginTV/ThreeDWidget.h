/*
 * Name:  
 *	ThreeDWidget.h
 *
 * Description: 
 *	Header file for a threeD map widget.
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
#ifndef THREEDWIDGET_H
#define THREEDWIDGET_H

#ifndef ThreeDWidget_typedef
#define ThreeDWidget_typedef
typedef struct ThreeDWidget ThreeDWidget;
#endif

#include "TsGeoPyramid.h"

extern ThreeDWidget *pluginWidget;
extern int          exit_flag;

/* FUNCTION PROTOTYPES */
extern ThreeDWidget *CreateThreeDWidget( void );

extern int  InitThreeDWidget( ThreeDWidget *, int );
extern void DestroyThreeDWidget (ThreeDWidget *threeD);

extern void ThreeDWidgetUpdate (ThreeDWidget *threeD);
extern void ThreeDWidgetAddDataset (ThreeDWidget *, TsGeoPyramid *);
extern void ThreeDWidgetRemoveDataset(ThreeDWidget *, TsGeoPyramid *);
extern void ThreeDWidgetLightingToggle(ThreeDWidget *, int);
extern void ThreeDWidgetStartRendering(ThreeDWidget *);
extern void ThreeDWidgetInitializeGraphics(ThreeDWidget *);
extern int  ThreeDWidgetRegenerateVisible (ThreeDWidget *threeDWidget);
extern void ThreeDWidgetSetStereoMode(ThreeDWidget *threeD, int);
extern void ThreeDWidgetCancelTimer( ThreeDWidget *threeDWidget );

extern void ThreeDWidgetBtnUp( ThreeDWidget *threeD, int x, int y );
extern void ThreeDWidgetBtn2Up( ThreeDWidget *threeD, int x, int y );
extern void ThreeDWidgetBtn1Down( ThreeDWidget *threeD, int x, int y );
extern void ThreeDWidgetBtn2Down( ThreeDWidget *threeD, int x, int y );
extern void ThreeDWidgetBtn3Down( ThreeDWidget *threeD, int x, int y );
extern void ThreeDWidgetMotion( ThreeDWidget *threeD, int x, int y );


#endif /* not defined THREEDWIDGET_H */
