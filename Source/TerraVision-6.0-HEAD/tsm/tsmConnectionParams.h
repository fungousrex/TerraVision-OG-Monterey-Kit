/*
 * Name: tsmConnectionParams.h
 *
 * Description: 
 *   External definitions to support the tsmApi's Connection
 *   Parameter facility. These enables the user to specify a
 *   number of option hints or parameters the major connection
 *   management functions.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmConnectionParams.h,v 1.28.4.2 2001/06/20 22:26:16 reddy Exp $
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

#ifndef TSMCONNECTIONPARAMS_H
#define TSMCONNECTIONPARAMS_H

#include <prtypes.h>

typedef PRInt64 TsmParamCode;

#define ONE64 ((PRInt64)1)

#define TSM_PARAM_NONE           0
#define TSM_PARAM_SETID          ONE64<<0
#define TSM_PARAM_SERVERS        ONE64<<1
#define TSM_PARAM_FORMAT         ONE64<<2
#define TSM_PARAM_TIFF           ONE64<<3
#define TSM_PARAM_FALLBACK       ONE64<<4
#define TSM_PARAM_VALIDATE_TSPEC ONE64<<5
#define TSM_PARAM_DPSS_TSPEC     ONE64<<6
#define TSM_PARAM_NEW_TSPEC      ONE64<<7
#define TSM_PARAM_VRML_DIR       ONE64<<8
#define TSM_PARAM_TEX_URNS       ONE64<<9
#define TSM_PARAM_LCS            ONE64<<10
#define TSM_PARAM_NUM_POLYS      ONE64<<11
#define TSM_PARAM_NUM_LODS       ONE64<<12
#define TSM_PARAM_TREE_SKIP      ONE64<<13
#define TSM_PARAM_GAUSSIAN       ONE64<<14
#define TSM_PARAM_COLLISION      ONE64<<15
#define TSM_PARAM_TEST_VTILES    ONE64<<16
#define TSM_PARAM_START_LEVEL    ONE64<<17
#define TSM_PARAM_END_LEVEL      ONE64<<18
#define TSM_PARAM_TEXEL_SCALE    ONE64<<19
#define TSM_PARAM_FLAT           ONE64<<20
#define TSM_PARAM_ELEV_SCALE     ONE64<<21
#define TSM_PARAM_WORLD_SCALE    ONE64<<22
#define TSM_PARAM_ROOT_LEVEL     ONE64<<23
#define TSM_PARAM_TREE_TYPE      ONE64<<24
#define TSM_PARAM_GEOTILES       ONE64<<25
#define TSM_PARAM_NORMALS     	 ONE64<<26
#define TSM_PARAM_SOLID     	 ONE64<<27
#define TSM_PARAM_FAST_BBOXES    ONE64<<28
#define TSM_PARAM_VRML_URL       ONE64<<29
#define TSM_PARAM_BLOCKING_READ  ONE64<<30
#define TSM_PARAM_REQUEST_CHECK  ONE64<<31
#define TSM_PARAM_INCLUDE_VTILES ONE64<<32
#define TSM_PARAM_FULL_DPSS_READ ONE64<<33
#define TSM_PARAM_CACHE_CONNS    ONE64<<34
#define TSM_PARAM_MODELS         ONE64<<35
#define TSM_PARAM_TEXTURES       ONE64<<36
#define TSM_PARAM_READ_MULTI     ONE64<<37
#define TSM_PARAM_WRITE_MULTI    ONE64<<38
#define TSM_PARAM_WMS            ONE64<<39

/* tree file types */
#define TSM_TREE_TYPE_ANCHOR     0
#define TSM_TREE_TYPE_QUADLOD    1
#define TSM_TREE_TYPE_LOD	 2

/* vtile coordinate formats */
#define TSM_FLAT_VTILES		1
#define TSM_UTM_VTILES		2
#define TSM_GEOCNTRC_VTILES	4

/*
 * Doc++ Description: *//** 
 *   Holds optional parameters for tsmConnect.
 *
 *   The TsmConnectionParamsStruct provides an extensible and flexible
 *   means to specify details about the dataset that you wish to select
 *   using tsmConnect.
 *
 *   This structure is opaque - it's fields are hidden from an external
 *   client. You must use the provided API calls in order to create,
 *   fill, and discard this structure. 
 *
 * @see tsmInitParams, tsmSetParam, tsmFreeParams, tsmPrintParams, tsmConnect
 *
 */

typedef struct TsmConnectionParamsStruct *TsmConnectionParams;

#endif
