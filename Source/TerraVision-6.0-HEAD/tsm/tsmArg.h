/*
 * Name: tsmArg.h
 *
 * Description:
 *    Contains any external definitions for tsmApi's command * line
 *    parsing routines. These are designed to make life simpler * for
 *    people developing command line tools built on the tsmApi.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *   March 1998 - Initial version by Martin Reddy
 *
 *   $Id: tsmArg.h,v 1.7 2000/08/28 22:15:23 reddy Exp $
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

#ifndef TSMARG_H
#define TSMARG_H

/* The following defines can be combined (bitwise-or or added)  */
/* and passed to the tsmDescribeArgs() function to detail which */
/* of the supported tsmApi arguments a program is supporting.   */

#define TSM_ARG_INIT    1    /* set this if you used tsmParseInitArgs() */
#define TSM_ARG_READ    2    /* set this if you used tsmParseReadArgs() */
#define TSM_ARG_WRITE   4    /* set this if you used tsmParseWriteArgs() */
#define TSM_ARG_CONNECT 8    /* set this if you used tsmParseConnectArgs() */
#define TSM_ARG_VRML    16   /* set this if you used tsmParseVRMLArgs() */

#define TSM_ARG_NONE    0  
#define TSM_ARG_ALL     ( 1 | 2 | 4 | 8 | 16)

#endif

