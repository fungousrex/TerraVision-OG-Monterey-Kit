/*
 * Name: tsmEmulate_dpss.c
 *
 * Description: 
 *   This file contains the DSM/DPSS specific functions for the 
 *   emulation layer of the new tsmApi. This provides emulation of
 *   the old server-style calls that the tsmApi used to use.
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 *   A trace level of 2 is required to get any trace output.
 *
 * Function List:
 *   int tsmOpen_dsm( TsmHandle *tsmh, Purl *basePurl )
 *   void tsmClose_dsm( TsmHandle *tsmh )
 *   void tsmPutPyramid_iss( TsmHandle *tsmh, Pyramid *pyramid, Purl *basePurl)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmEmulate_dpss.c,v 1.11 2000/09/23 01:03:28 leei Exp $
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

#include "tsmInternal.h"
#include "tsmConnect_dpss.h"

#ifdef DPSS

int
tsmOpen_dsm( TsmHandle *tsmh, Purl *basePurl )
{
  TSMTRACE ((TSMMASK|2, "tsmOpen_dsm", "host = %s", basePurl->hostName));
  if ( tsmh == NULL ) return TRUE;
  if ( basePurl == NULL ) return FALSE;
  if ( tsmh->dsmHandle == NULL )
    tsmh->dsmHandle = dsmOpen( basePurl->hostName, DSMPORT, NULL );
  return ( tsmh->dsmHandle != NULL );
}

void
tsmClose_dsm( TsmHandle *tsmh )
{
  if ( tsmh == NULL ) return;
  if ( tsmh->dsmHandle != NULL ) dsmClose( tsmh->dsmHandle );
}

#else

int
tsmOpen_dsm( TsmHandle *tsmh, Purl *basePurl )
{
  tsmNotCompiledIn( tsmh->dbPurl->protocol, "tsmOpen_dsm" );
  return FALSE;
}

void
tsmClose_dsm( TsmHandle *tsmh )
{
  tsmNotCompiledIn( tsmh->dbPurl->protocol, "tsmClose_dsm" );
  return;
}

#endif

void
tsmPutPyramid_iss( TsmHandle *tsmh, Pyramid *pyramid, Purl *basePurl )
{
  tsmNoProtocolYet( basePurl->protocol, "tsmPutPyramid" );
}

