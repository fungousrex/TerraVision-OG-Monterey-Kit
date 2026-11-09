/*
 * Name: TVLogger.h
 *
 * Description: Header file for logging information
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
#ifndef TVLOGGER_H
#define TVLOGGER_H

#include <X11/Intrinsic.h>
#include <glt/glm.h>
#include "TsLogging.h"

#define TV_NUM_GRAPHS 4

typedef struct 
{
    Widget mainForm; 
    Widget drawWidget;
    glmWindow drawglmw;
    Widget requestWidget;
    glmWindow requestglmw;
    Widget textureWidget;
    glmWindow textureglmw;
    Widget cacheWidget;
    glmWindow cacheglmw;
    Widget scaleWidget;
    Widget offsetWidget;
    Widget freezeWidget;
    Widget legendWidget[7];
    glmWindow legendglmw[7];
    Widget averageWidgets[TV_NUM_GRAPHS];
    int    freeze;
    PRIntervalTime freezeTime;
    float  graphScale;
    float  offsetScale;
    GLXContext glxCtx;
    double lastTime;
    int pendingRedraw;
    PRThread *thread;
} TVLogger;

extern TVLogger *tvLogger;

TVLogger * CreateTVLogger(Widget);
Boolean    TVLoggerIsManaged(TVLogger const *);
void       TVLoggerShowGraphs(TVLogger *);
void       TVLoggerUpdateWidgets(TVLogger *);

#endif

