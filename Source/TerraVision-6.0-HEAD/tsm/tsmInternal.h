/*
 * Name: tsmInternal.h
 *
 * Description:
 *    This header file contains all of the internal definitions for the
 *    tsmApi (Tile Set Manager Application Programming Interface). This
 *    header file is included by all of the source code files.
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
 *    $Id: tsmInternal.h,v 1.20.6.3 2002/03/14 05:22:57 reddy Exp $
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

#ifndef TSMINTERNAL_H
#define TSMINTERNAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* X_program needs to be defined because include/iss/utils.h redefines
 * Status.
 */

#define X_program 1

#include "tsm.h"         /* include all of the external definitions first */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
/*#include <malloc.h>*/
#include <memory.h>
#include <errno.h>
#include <ctype.h>

#include <tiffio.h>

#include <prthread.h>
#include <prlock.h>
#include <prcvar.h>
#include <prinrval.h>
#include <prtime.h>
#include <prtypes.h>
#include <prio.h>

#include <pthr/trace.h>
#include <pthr/eqhash.h>

#include <vrml2/vrml2.h>

#define TSM_TMPDIR "/tmp"
#define TSM_DEFAULT_PDB_NAME "Pyramids/PyramidUrls"

#define TSM_CHECKSUM_MOD (1024*1024*1024)

#define TSM_MAX_TRACE_PROCS 10000

#define MAX_TSPEC_LINE_LENGTH 1024 /* longest line in tspec/database file */

typedef struct tsmRequestServer_ tsmRequestServer;
typedef struct tsmReqElem_ tsmReqElem;

#include "tsmTileInt.h"
#include "tsmEmulateInt.h"
#include "tsmPackTspecs.h"
#include "tsmTspecUtils.h"

#include "tsmIntProts.h"
#include "TsTileSpecIntProts.h"
#include "tsmTrace.h"

#endif
