/*
 * Name: terraserver.h
 *
 * Description:
 *     This module provides a TerraVision interface to the Microsoft
 *     TerraServer facility. TerraServer provides tiled versions of
 *     USGS imagery for the entire USGS.
 *
 *     This can be linked against another program and exports the
 *     following four functions. See the code below for usage.
 *
 *       ts_init();
 *       ts_cleanup();
 *       ts_set_debug( int );
 *       ts_is_terrasever( char * )
 *       ts_get_terraserver_tile( TsmConnection, int, int, int, uchar ** )
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 25 June 2001.
 *
 * Revision Information:
 *
 *     $Id: terraserver.h,v 1.1.2.1 2001/06/26 22:05:31 reddy Exp $
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
 *   Portions are Copyright (c) SRI International, 1998-2001.
 */

#ifndef TSMUTIL_TERRASERVER_H
#define TSMUTIL_TERRASERVER_H

#include <tsm/tsm.h>

/*
 * Initialise the TerraServer capability. This must be the first
 * function that is called from this module
 */

void ts_init();

/*
 * Cleanup the TerraServer module when done
 */

void ts_cleanup();

/*
 * Switch TerraServer debugging mode to be on or off. When debugging
 * is on, various debug messages are sent to stderr (picked up by the
 * Apached error_log).
 */

void ts_set_debug( int debug_on );

/*
 * Test whether a URL is for a TerraServer dataset
 *
 * This exported function can be used by a client to decide whether
 * or not to call the ts_get_terraserver_tile() function.
 */

int ts_is_terraserver( char *url );

/*
 * Main entry point for serving TerraServer tiles in TerraVision form.
 *
 * This exported function is what a client should use in order to
 * access the TerraServer database using the normal TerraVision
 * semantics. This function will first check whether the dummy 
 * TerraVision dataset contains data for the requested level and
 * will use that. Otherwise, we contact TerraServer and download the
 * terrain data. This lets you fill the low-res parts of the pyramid
 * with data that TerraServer doesn't supply.
 */

int ts_get_terraserver_tile( TsmConnection connect, int x, int y, int level,
			     uchar **tileBuffer );


#endif
