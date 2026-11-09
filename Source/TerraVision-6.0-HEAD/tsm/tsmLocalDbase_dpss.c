/*
 * Name: tsmLocalDbase_dpss.c
 *
 * Description: 
 *   This file contains the DSM/DPSS specific functions for manipulating
 *   local TSM databases on DPSS's (referred to as DSM's).
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 *   A trace level of 2 is required to get any trace output.
 *
 * Function List:
 *   int tsmLoadPyramidDatabase_dsm( TsmHandle *tsmh )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmLocalDbase_dpss.c,v 1.8 2000/08/28 22:15:55 reddy Exp $
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

#ifdef DPSS

int
tsmLoadPyramidDatabase_dsm( TsmHandle *tsmh )
{
  static const char *funcName = "tsmLoadPyramidDatabase_dsm";
  DsmNameList       *nameList;
  int               numPyramids, i;
  NameUrlIds        *pyramidUrls;
  char              buffer[1024];

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "tsmh = %p.", tsmh ));

  /* Open the connection to the dsm if necessary. */

  if ( tsmOpen_dsm( tsmh, tsmh->dbPurl ) == FALSE ) {
    tsmError( funcName, "Unable to open DSM." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* Get the set names. */

  nameList = dsmLoadedSetNames( tsmh->dsmHandle, DSM_PYRAMID_TYPE, 
				&numPyramids );
  
  /* Error if numPyramids is negative. */

  if ( numPyramids < 0 ) {
    tsmError( funcName, "Unable to get pyramid set list." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }
  /* Put the set names/urls/setids into the pyramidUrls data structure */

  pyramidUrls = tsmh->pyramidUrls = tsmMakeNameUrlIds( numPyramids );

  if ( pyramidUrls == NULL ) {
    tsmError( funcName, "Error in allocating pyramidUrls." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  for (i = 0; i < numPyramids; i++) {
    pyramidUrls->names[i] = nameList[i].set_name;

    sprintf( buffer, "x-dpss://%s/TileSets/Pyramids/%d",
	     tsmh->dbPurl->hostName, nameList[i].setid );

    pyramidUrls->urls[i] = tsmMemManagerStrdup (buffer);
    pyramidUrls->ids[i] = nameList[i].setid;
  }

  TSMTRACE(( TSMMASK|2, funcName, "Done." ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

#else

int
tsmLoadPyramidDatabase_dsm( TsmHandle *tsmh )
{
  tsmNotCompiledIn( tsmh->dbPurl->protocol, "tsmLoadPyramidDatabase" );
  return FALSE;
}

#endif

