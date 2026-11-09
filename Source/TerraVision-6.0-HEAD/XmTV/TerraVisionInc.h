/*
 * Name:  TerraVisionInc.h
 *
 * Description: Real bad name for include file
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

#ifndef TERRAVISIONINC_H
#define TERRAVISIONINC_H

#ifndef BYTE_DEFINED
typedef unsigned char byte;
#define BYTE_DEFINED
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <errno.h>

#ifdef __sgi
#include <malloc.h>
#endif

#include <pthr/list.h>

#include "Trace.h"

#include "../RELEASE"

#ifndef sgi
#define trunc(x) ((x>=0) ? floor(x) : floor(x+0.9999999999))
#endif

/* WE NEED THIS FOR WHEN WE FAIL TO CONNECT */
#define CONNECTFAIL -1

#ifndef SCANNEDMAP_H
typedef struct ScannedMapWidget ScannedMapWidget;
#endif

#include "TerraVisionGlobals.h"

extern void tvPause (unsigned long);

extern void TerraVisionGraphicsFeatures(void);
extern void TerraVisionQuit(void);

extern void TerraVisionRefreshStatusFields( int );
extern void TerraVisionRefreshMenus( void );

extern XrmDatabase tvDatabase;
extern XrmDatabase userDatabase;
extern XrmName tvNameQ;
extern XrmClass tvClassQ;

extern void tvInitDatabase (Display *disp, char const *appName, char const *appClass,
			    int *argcp, char * argv[]);

extern Boolean tvGetResource (char const *name, XtPointer *val,
			      XrmRepresentation as_type);

extern Boolean tvGetResourceAsString (char const *name, char const **val);
extern Boolean tvGetResourceAsStringList (char const *name, list_t **val);
extern Boolean tvGetResourceAsInt (char const *name, int *val);
extern Boolean tvGetResourceAsFloat (char const *name, float *val);
extern Boolean tvGetResourceAsDouble (char const *name, double *val);

extern void TerraVisionStartLogging( void );
extern void TerraVisionStopLogging( void );
extern void TerraVisionRestartLogging( void );

extern void TerraVisionHelp( char *helpfile );
extern void TerraVisionHelpCB( Widget, XtPointer, XtPointer );

#ifdef CAVE
int TerraVisionStartCAVE (void);
extern XtAppContext TerraVisionInitCAVE (int *argc, char *argv[]);
#endif

#endif /* end ifndef TERRAVISION_INC_H */
