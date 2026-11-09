/*
 * Name: tsm.h
 *
 * Description:
 *    This header file contains all of the external definitions for the
 *    tsmApi (Tile Set Manager Application Programming Interface). If you
 *    are developing applications that use the tsmApi, then this is the
 *    only file that you need include in your code, as follows:
 *
 *        #include <tsm/tsm.h>
 *
 *    The tsmApi is a library of high level C functions for reading, writing,
 *    and processing the terrain data used by the TerraVision terrain
 *    visualization application, developed at SRI International. The
 *    library acts as an abstraction layer between the application program
 *    and the terrain data.
 *
 *    To find out more about the tsmApi, visit the tsmApi home page. This
 *    site includes the latest release for download, example programs,
 *    API specifications, file format descriptions, tutorials, etc.:
 *
 *        http://www.ai.sri.com/~magic/tsmApi
 *
 * Function List:
 *    For a list of all the functions that tsmApi exports, please refer
 *    to the tsmExtProts.h and TsTileSpecExtProts.h files. Full documentation
 *    on these functions can be found at the above web page.
 *
 *    Copyright (c) 1998 SRI International, Menlo Park, CA 94025, USA.
 *    All rights reserved.
 *
 *    $Id: tsm.h,v 1.25.2.4 2002/06/03 21:13:38 reddy Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 *
 */

#ifndef TSMAPI_H
#define TSMAPI_H

#include <stdio.h>
#include <stdlib.h>

#include <prtypes.h>
#include <prinrval.h>
#include <prtime.h>

/* stop Visual Studio from bitching so much */

#ifdef MSVC
#pragma warning(disable:4005)    /* macro redefinition */
#pragma warning(disable:4018)    /* signed/unsigned mismatch */
#pragma warning(disable:4100)    /* unreferenced formal parameter */
#pragma warning(disable:4101)    /* unreferenced local variable */
#pragma warning(disable:4142)    /* benign typedef redefinition */
#pragma warning(disable:4209)    /* benign typedef redefinition */
#pragma warning(disable:4305)    /* truncation from double to float */
#pragma warning(disable:4505)    /* unreferenced local function removed */
#pragma warning(disable:4514)    /* unreferenced inline function removed */
#pragma warning(disable:4244)    /* conversion coersion */
#endif

/* Any and all external type definitions */

typedef unsigned char uchar;
typedef struct tsmWMS_ tsmWMS;

/* Include all of the external header files into this one place */

#include "TsTileSpecDecs.h"      /* definitions for tspec manipulations */
#include "tsmTile.h"             /* the tile header definition */
#include "tsmPurl.h"             /* routines for parsing URLs */
#include "tsmTrace.h"				     /* tracing and debug functions */
#include "tsmArray.h"            /* the int and request list array routines */
#include "tsmPyramidInfo.h"      /* for the DPSS server group information */
#include "tsmLocalDbase.h"       /* for all local database structures */
#include "tsmDatabase.h"         /* for the new dataset database structures */
#include "tsmConnection.h"       /* for the connection based structures */
#include "tsmConnectionParams.h" /* for the connection parameters */
#include "tsmImage.h"            /* for the image manipulation functions */
#include "tsmEmulate.h"          /* emulation layer for old tsmApi funcs */
#include "tsmArg.h"              /* command line parsing utilities */
#include "tsmCreate.h"           /* for creating datasets from raw data */
#include "logging.h"             /* LBL's NetLogger C header file */

#include "tsmExtProts.h"         /* prototypes for the tsmApi functions */
#include "TsTileSpecExtProts.h"  /* prototypes for the tspec-related funcs */

/* All of the constants used by various tsmApi functions */

#define TSM_PATH_ONLY         1      /* used by tsmMakeDirIfNotExist() */
#define TSM_PATH_AND_FILE     0

#ifdef MAXNUMREQ
#define TSM_MAX_NUMREQS       MAXNUMREQ
#else
#define TSM_MAX_NUMREQS       8192  /* value defined by DPSS 2.5 */
#endif

#define WGS84_AXIS   6378137.0
#define WGS84_RFLAT  298.257223563

/* and finally, some useful macro functions */

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))    /* smallest value of x and y */
#endif

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))    /* largest value of x and y */
#endif

#ifndef pow2i
#define pow2i(x) (1<<(x))              /* pow2(x) = 2^x */
#endif

#ifndef scale2
#define scale2(x,y) ((x)<<(y))        /* scale2(x,y) = x * 2^y */
#endif

void tsmTileReqInfo( TsmConnection connect, PRIntervalTime *begin,
		PRIntervalTime *end, int *num_now, int *num_never,
		int *num_later );

#endif
