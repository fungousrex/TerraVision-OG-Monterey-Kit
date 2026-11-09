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

#include <X11/Intrinsic.h>

#include "BusyDialog.h"
#include "TsGeoPyramid.h"
#include "ThreeDPanel.h"

/* FUNCTION PROTOTYPES */
extern ThreeDWidget * CreateThreeDWidget(Widget, Boolean, Boolean);
extern void DestroyThreeDWidget (ThreeDWidget *threeD);

extern void ThreeDWidgetError (ThreeDWidget *threeD, char const *fmt, ...);

extern void ThreeDWidgetForall (void (*f) (ThreeDWidget *, void *),
				void *closure);
extern ThreeDWidget * ThreeDWidgetFind (int (*f) (ThreeDWidget *, void *),
					void *closure);

extern Widget ThreeDWidgetMainForm (ThreeDWidget const * threeDWidget);
extern void ThreeDWidgetBusyDialog (ThreeDWidget *, BusyDialog *busy);
extern void ThreeDWidgetUpdate (ThreeDWidget *threeD);
extern void ThreeDWidgetRemoveDataSet(ThreeDWidget *, TsGeoPyramid *);
extern void ThreeDWidgetDismiss(ThreeDWidget *);
extern void ThreeDWidgetLightingToggle(ThreeDWidget *, int);
extern void ThreeDWidgetStartRendering(ThreeDWidget *);
extern void ThreeDWidgetInitializeGraphics(ThreeDWidget *);
extern void ThreeDWidgetCAVEBtnDown(ThreeDWidget *);
extern void ThreeDWidgetCAVEChangeTimeOfDay(ThreeDWidget *, float);
extern int  ThreeDWidgetRegenerateVisible (ThreeDWidget *threeDWidget);
extern Boolean  ThreeDWidgetIsManaged(ThreeDWidget const *);
extern ThreeDPanel *ThreeDWidgetPanel(ThreeDWidget const *);
extern void ThreeDWidgetSetStereoMode(ThreeDWidget *threeD, int);
extern void ThreeDWidgetUpdateStatusBar( ThreeDWidget *, char const * );
extern void ThreeDWidgetCancelTimer( ThreeDWidget *threeDWidget );
extern void ThreeDWidgetRefreshDialogs( ThreeDWidget *threeDWidget );
extern void ThreeDWidgetInterpret( ThreeDWidget *, char const * );

#endif /* not defined THREEDWIDGET_H */
