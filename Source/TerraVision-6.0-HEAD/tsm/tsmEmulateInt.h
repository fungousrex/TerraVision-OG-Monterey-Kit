/*
 * Name: tsmEmulateInt.h
 *
 * Description: 
 *   Internal definitions for the old-style tsmApi emulation layer
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmEmulateInt.h,v 1.8 2000/08/28 22:15:44 reddy Exp $
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

#ifndef TSMEMULATEINT_H
#define TSMEMULATEINT_H

#include "tsmInternal.h"

#ifdef DPSS
#include <dpss/dsm.h>
#endif

/* This is the new TsmHandle for the emulation layer. It basically */
/* holds the local database information, and then just an array of */
/* connection handles. Old-style tsmApi functions, e.g. tsmOpen,   */
/* can then maintain the old calling protocol, but be made to use  */
/* the newer, sleeker, sexier, connection-based routines.          */

struct TsmHandle {

  /* Fields preserved from original TsmHandle structure */

  Purl          *dbPurl;         /* database parsed base URL (Purl) */
  Purl          *gdbPurl;        /* GeoPyramid database base Purl (obsolete) */
  Purl          *pdbPurl;        /* Pyramid database base Purl */
  char          *masterHostName; /* DPSS Master hostname */
  NameUrlIds    *geoPyramidUrls; /* All local GeoPyramid triples (obsolete) */
  NameUrlIds    *pyramidUrls;    /* All local Pyrmamid Name/Url/Id triples */

  /* All connection-oriented fields replaced by array of connection handles */

  TsmConnection *connect;        /* All open connections for this session */
  int           numConnects;     /* Number of entries in connect array */

  /* DPSS fields reorganised from original TsmHandle */

#ifdef DPSS
  int           logging;         /* Are we logging data currently? */
  dsmSession    dsmHandle;       /* DSM connection for Pyramid info */
#endif

};

/* function to output a message to tell the client that an old */
/* tsmApi function is not emulated by this version of the API. */

void tsmTsmHandleNotEmulated( char *funcName );


#endif
