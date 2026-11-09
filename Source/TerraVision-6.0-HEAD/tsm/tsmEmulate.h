/*
 * Name: tsmEmulate.h
 *
 * Description: 
 *   External definitions for the old-style tsmApi emulation layer.
 *   This just involves exposing the (opaque) TsmHandle structure.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmEmulate.h,v 1.3 2000/08/28 22:15:43 reddy Exp $
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

#ifndef TSMEMULATE_H
#define TSMEMULATE_H


/*
 * Doc++ Description: *//** 
 *   Holds an old style TSM session handle.
 *
 * All of the old server-based tsmApi routines used a TsmHandle struct.
 * This is defined here for emulation purposes with the old routines.
 * The fields of the TsmHandle are hidden from the external client so
 * that future changes can be made internally without breaking user code.
 *
 * @see tsmOpen, tsmGetPyramid, tsmGetTile
 *
 */

typedef struct TsmHandle TsmHandle;


#endif
