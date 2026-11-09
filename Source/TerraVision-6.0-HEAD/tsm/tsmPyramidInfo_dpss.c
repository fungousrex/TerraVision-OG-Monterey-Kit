/*
 * Name: tsmPyramidInfo_dpss.c
 *
 * Description: 
 *   Contains routines for compiling complete server information about
 *   datasets. These are the DSM/DPSS specific routines and are private
 *   to the tsmApi.
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 *   A trace level of 2 is required to get any trace output.
 *
 * Function List:
 *   PyramidInfo *tsmGetPyramidInfo_dsm(TsmHandle *tsmh, const char *, Purl *)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmPyramidInfo_dpss.c,v 1.8 2000/09/23 01:03:28 leei Exp $
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

#include <dpss/dpss.h>
#include <dpss/sequential.h>

PyramidInfo *
tsmGetPyramidInfo_dsm( TsmHandle *tsmh, const char *pyramidName,
		       Purl *basePurl )
{
  static const char *funcName = "tsmGetPyramidInfo_dsm";
  dsmSetInfoStr     *dsmSetInfo;
  PyramidInfo       *pyramidInfo;
  IssHandle         *issHandle;
  IssServerInfo     *isi_p;
  char const        *name;
  char              *hostname;
  int               server, numServerGroups, numServers, setId, i, j, k;

  if ( pyramidName == NULL ) pyramidName = "";

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName,
	     "tsmh = %p, pyramidName = '%s', basePurl = %p.",
	     tsmh, pyramidName, basePurl));

  /* Open the connection to the dsm if necessary. */

  if ( tsmOpen_dsm( tsmh, tsmh->dbPurl ) == FALSE ) {
    tsmError( funcName, "Unable to open DSM." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* Work out the setid, hostname, and allocate a PyramidInfo struct */

  setId = tsmNameToId( pyramidName, tsmh->pyramidUrls );

  hostname = basePurl->hostName;

  pyramidInfo = (PyramidInfo *) tsmMemCalloc( 1, sizeof(PyramidInfo) );

  if ( pyramidInfo == NULL ) {
    tsmError( funcName, "Error allocating space for pyramid info." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
  pyramidInfo->name = tsmMemStrdup( pyramidName );

  /* Figure out how many server groups there are by comparing */
  /* the names of all set ids against pyramidName.            */

  for ( i = numServerGroups = 0; i < tsmh->pyramidUrls->numItems; i++ ) {
    setId = tsmh->pyramidUrls->ids[i];
    name  = tsmh->pyramidUrls->names[i];

    if ( strcmp( name, pyramidName ) == 0 )
      numServerGroups++;
  }
  TSMTRACE(( TSMMASK|3, funcName,
	     "Number of server groups = %d", numServerGroups ));

  /* Allocate the space necessary for the server groups. */

  pyramidInfo->numServerGroups = numServerGroups;
  pyramidInfo->serverGroup = (ServerGroup *) tsmMemCalloc( numServerGroups, 
						     sizeof(ServerGroup) );
  if( pyramidInfo->serverGroup == NULL ) {
    tsmError( funcName, "Unable to allocate space for ServerGroup." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* For DPSS 2.4, we need to get the number of servers per */
  /* set id from the DPSS rather than from the DSM!!!!!!!   */
  
  TSMTRACE ((TSMMASK|2, funcName, "Opening DPSS '%s'.", hostname ));

  if ( ( issHandle = issOpen ( hostname ) ) == NULL ) {
    tsmError( funcName, "issOpen (%s) failed.", hostname );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* For each set id with a matching name, get the names of */
  /* the servers from the DSM.                              */

  for ( i = j = 0; i < tsmh->pyramidUrls->numItems; i++) {

    setId = tsmh->pyramidUrls->ids[i];
    name  = tsmh->pyramidUrls->names[i];

    if ( strcmp( name, pyramidName ) == 0 ) {
      pyramidInfo->serverGroup[j].setId = setId;

      if ( ( dsmSetInfo = dsmFetch( tsmh->dsmHandle, setId ) ) == NULL ) {
	tsmError( funcName, "Couldn't get dsmSetInfo for setId %d.", setId );
	TSMTRINC(( TSMMASK|2, -1 ));
	issClose( issHandle );
	return NULL;
      }

      numServers = issGetServerNames( issHandle, setId, &isi_p );

      /* Fill in the image format information */

      if ( dsiCompression(dsmSetInfo) == ISS_JPEG )
	pyramidInfo->serverGroup[j].imageFormat = tspJPEGCompressed;
      else
	pyramidInfo->serverGroup[j].imageFormat = tspLrbtRaster;

      /* Fill in the metadata URL information */

      for ( k = 0; k < dsmSetInfo->metadata_url.metadata_url_len; ++k ) {
	tsmAddPyramidInfoMetaData(pyramidInfo, dsiMetadataN(dsmSetInfo,k), j);
      }

      if ( numServers < 0 ) {
	tsmError( funcName, "Number of servers (%d) is < 0", numServers );
	TSMTRINC(( TSMMASK|2, -1 ));
	issClose( issHandle );
	return( NULL );
      }

      TSMTRACE ((TSMMASK|2, funcName, "For set id %d %p, numServers = %d.",
		 setId, &pyramidInfo->serverGroup[j], numServers));

      pyramidInfo->serverGroup[j].numServers      = numServers;
      pyramidInfo->serverGroup[j].masterHostName  = tsmMemStrdup( hostname );
      pyramidInfo->serverGroup[j].serverHostNames = (char **) 
	tsmMemCalloc( numServers, sizeof(char *) );

      if( pyramidInfo->serverGroup[j].serverHostNames == NULL ) {
	tsmError( funcName, "Unable to allocate space for server hostnames." );
	TSMTRINC(( TSMMASK|2, -1 ));
	issClose( issHandle );
	return NULL;
      }

      for ( server = 0; server < numServers; server++ ) {
	if ( isi_p[server].server_name == NULL ) {
	  tsmError( funcName, "Server name for server %d is NULL", server );
	  TSMTRINC(( TSMMASK|2, -1 ));
	  issClose( issHandle );
	  return NULL;
	}
	pyramidInfo->serverGroup[j].serverHostNames[server] = 
	  tsmMemStrdup( isi_p[server].server_name );
      }
      j++;
    }
  }

  issClose( issHandle );

  TSMTRACE(( TSMMASK|2, funcName, "Done, pyramidInfo = %p.", pyramidInfo) );
  TSMTRINC(( TSMMASK|2, -1 ));
  return pyramidInfo;
}

#else

PyramidInfo *
tsmGetPyramidInfo_dsm(TsmHandle *tsmh, const char *pyramidName, Purl *basePurl)
{
  tsmNotCompiledIn( basePurl->protocol, "tsmGetPyramidInfo_dsm" );
  return NULL;
}

#endif

