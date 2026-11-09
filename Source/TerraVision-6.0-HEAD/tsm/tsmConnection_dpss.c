/*
 * Name: tsmConnection_dpss.c
 *
 * Description: 
 *   Contains routines for manipulating connections to DPSS datasets.
 *   These functions are private to the tsmApi.
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   TsmConnection tsmConnect_dpss( Purl *, TsmAccessMode, TsmConnectionParams)
 *   void tsmDisconnect_dpss( TsmConnection connect )
 *
 *   int tsmGetSetId( TsmConnection connect, TsmConnectionParams param )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnection_dpss.c,v 1.27.2.4 2001/07/10 06:17:40 leei Exp $
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
#include <dpss/dsm.h>

int tsmCheckTileFormat_dpss ( TsmConnection conn, TsmTileFormat fmt);
int tsmCheckTIFFFormat_dpss( TsmConnection conn, int fmt);

static TsmConnectionVTable vtable_dpss = {
  tsmDisconnect_dpss,

  0 /* tsmAddRequest_dpss */,
  0 /* tsmRemoveRequest_dpss */,
  0 /* tsmReprioRequest_dpss */,

  tsmWriteTile_dpss,

  tsmCheckTileFormat_dpss,
  tsmCheckTIFFFormat_dpss,

  tsmBeginLogging_dpss,
  tsmEndLogging_dpss
};

ActualConnection
tsmActualConnect_dpss (TsmConnection com_connect, int idx,
		       Purl *purl, TsmAccessMode mode,
		       TsmConnectionParams param);

/*
 * Synopsis:
 *   TsmConnection tsmConnect_dpss( Purl *, TsmAccessMode, TsmConnectionParams)
 *
 * Description:
 *   Does all of the DPSS-specific stuff for opening a connection. This
 *   includes opening connections to the relevant DSM and DPSS hosts, 
 *   loading the tspec (from a metadata URL, or embedded in the DPSS set),
 *   grabbing a dsmSetInfo structure, preparing the DPSS for reading or
 *   writing, etc.
 *
 * Returns: a TsmConnectionDpss pointer, cast down to a TsmConnection 
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

TsmConnection
tsmConnect_dpss( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  const char        *funcName = "tsmConnect_dpss";

  TsmConnectionDPSS dpss_connect;
  int i;

  /* Now we create a connection structure which is specific for the */
  /* connection protocol, i.e. local filesystem, http, or x-dsm.    */

  dpss_connect = tsmMemManagerCalloc (1, sizeof (*connect));
  connect = &dpss_connect->common;
  
  connect->mode        = mode;
  connect->location    = purl;

  connect->type        = TSM_DPSS;
  connect->format      = TSM_LRBT_FORMAT;
  connect->tile_format = TSM_DPSS_TILES;

  /* The user can specify the image format to use via the param struct */
  /* And also the server combination to use for the dataset.           */

  if ( tsmIsParamSet( param, TSM_PARAM_FORMAT ) == TRUE ) 
    connect->format = param->format;

  /* Do common initialization */
  connect = tsmConnectCommon (connect, purl, mode, params);
  if (!connect) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return connect;
  }

  if ( !(connect->lock = PR_NewLock()) ) {
    tsmError( funcName, "Unable to create connection lock." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  connect->numReaders = 0;
  connect->nextReader = 0;
  if (mode == TSM_READ_WRITE || mode == TSM_READ_ONLY) {
    connect->numReaders = 4;
    if (tsmIsParamSet( param, TSM_PARAM_READ_MULTI ))
      connect->numReaders = param->read_multi;
    connect->readers = tsmMemManagerCalloc (connect->numReaders,
					    sizeof (connect->readers[0]));

    for (i = 0; i < connect->numReaders; ++i) {
      connect->readers[i] = makeActualConnection (connect, i, purl,
						  TSM_READ_ONLY,
						  param);
      /* Reduce multiplicity on failure */
      if (! connect->readers[i]) {
	--connect->numReaders;
	--i;
      }
    }
  }

  /* Abort now if we couldn't open any connections. Otherwise, we  */
  /* initialise all of the general connection options to defaults */

  if ( mode != TSM_WRITE_ONLY && ! connect->numReaders ) {
    tsmError (funcName, "%s: Unable to connect %s", purl);
    tsmDisconnect (connect);
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  connect->writer = 0;
  if (mode == TSM_READ_WRITE || mode == TSM_WRITE_ONLY) {
    connect->writer = makeActualConnection (connect, 0, purl,
					    TSM_WRITE_ONLY,
					    param);
    if ( connect->writer == NULL ) {
      /* Clean up stuff that we've initialize above. */
      tsmDisconnect (connect);
      TSMTRINC(( TSMMASK|2, -1 ));
      return NULL;
    }
  }

  return connect;
}

ActualConnection
tsmActualConnect_dpss( TsmConnection com_connect, int idx,
		       Purl *purl, TsmAccessMode mode,
		       TsmConnectionParams param )
{
  const char        *funcName = "tsmConnect_dpss";
  TsmConnectionDpss *connect;
  ActualConnection   aconn;
  IssServerInfo     *s_info;
  int               set_id, num_servers, i;

  /* Allocate a new TsmConnectionDpss structure and fill in some defaults */

  connect = tsmMemManagerCalloc( 1, sizeof(TsmConnectionDpss) );
  if ( connect == NULL ) {
    tsmError( funcName, "Couldn't allocate ISS connection structure" );
    return NULL;
  }

  aconn = &connect->aconn;
  aconn->conn = com_connect;
  aconn->vtable = 0;

  connect->pl_method           = 1;
  connect->close_on_disconnect = TRUE;

  connect->requestTimeout = 0.3;  /* timeout (secs) for requesting blocks */
  TSMTRACE ((TSMMASK|1, funcName, "%p->requestTimeout = %f",
	     connect, connect->requestTimeout));
  
  /* create a mutex lock for controlling the DPSS reconnect thread var */

  if ( !(connect->reconnect_lock = PR_NewLock()) ) {
    tsmError( funcName, "Unable to create the DPSS reconnect mutex lock" );
    tsmMemFree( connect );
    return NULL;
  }

  /* open connections to the DSM and DPSS */

  TSMTRACE(( TSMMASK|2, funcName, "About to open DSM..." ));

  if ( ! ( connect->dsmHandle = dsmOpen( purl->hostName, DSMPORT, NULL ) ) ) {
    tsmError( funcName, "Couldn't open DSM '%s'", purl->hostName );
    tsmDisconnect_dpss( aconn );
    return NULL;
  }

  TSMTRACE(( TSMMASK|2, funcName, "About to open DPSS..." ));

  if ( ! ( connect->issHandle = issOpen( purl->hostName ) ) ) {
    tsmError( funcName, "Couldn't open DPSS '%s'", purl->hostName );
    tsmDisconnect_dpss( aconn );
    return NULL;
  }

  TSMTRACE(( TSMMASK|2, funcName, "DPSS/DSM opened successfully." ));

  /* Now we have different things to do if we are */
  /* reading or writing to the DPSS.              */

  if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE ) {

    /* This is done so that the ReadBlocks doesn't block indefinitely.
       Allows us to check for PR_Interrupt. */
    issSetAllRcvStreamsTimeout (connect->issHandle, 0, 50000);

    /* Do we have a set Id for the dataset to be connected to? */
    /* First we look at the URL to see if it is in the form:   */
    /* .../Pyramid/<set-id>. If it's not, then the set ID needs*/
    /* to be provided in the connection parameter structure, or*/
    /* a list of server names, otherwise you take what ya get. */

    if ( ( set_id = tsmPurlToSetId( purl, -1 ) ) == -1 ) {
      if ( tsmIsParamSet( param, TSM_PARAM_SETID ) == FALSE ) {

	if ( ( set_id = tsmGetSetId( connect, param ) ) == -1 ) {
	  
	  /* Can't find specified dataset. Shall we find another? */
	  
	  if ( tsmIsParamSet( param, TSM_PARAM_FALLBACK ) == TRUE &&
	       param->fallback == FALSE ) {
	    tsmError( funcName, "Specified dataset is not available" );
	    tsmDisconnect_dpss( aconn );
	    return NULL;
	  } else
	    set_id = dsmGetSetid( connect->dsmHandle, com_connect->name );
	  
	}
	
      } else
	set_id = param->set_id;

    }

    if ( ( connect->set_id = set_id ) <= 0 ) {
      tsmError( funcName, "No DPSS dataset exists for %s", com_connect->name );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    TSMTRACE(( TSMMASK|2, funcName, "Setting up read connection for set %d",
	       set_id ));

    /* Initialise the request list for reading tiles */

    issInitRequestList( &connect->requestList, TSM_MAX_NUMREQS );

    /* Work out the compression for the dataset and also */
    /* make sure that we are connected to a PYRAMID set. */

    if ( ! ( connect->set_info = dsmFetch( connect->dsmHandle, set_id ) ) ) {
      tsmError( funcName, "Error getting DSM info for set %d.", set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    if ( connect->set_info->datatype != DSM_PYRAMID_TYPE ) {
      tsmError( funcName, "DPSS set %d is not a Pyramid!", set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    if ( dsiCompression(connect->set_info) == ISS_JPEG )
      com_connect->format = TSM_JPEG_FORMAT;
    else
      com_connect->format = TSM_LRBT_FORMAT;

    /* Query the DPSS for the number of servers and their names */

    num_servers = issGetServerNames( connect->issHandle, set_id, &s_info );

    if ( num_servers <= 0 ) {
      tsmError( funcName, "Invalid number of servers (%d)!", num_servers );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    connect->numServers  = num_servers;
    connect->server = (char **) tsmMemMalloc( sizeof(char *) * num_servers );

    for ( i = 0; i < num_servers; i++ )
      connect->server[i] = tsmMemStrdup( s_info[i].server_name );

    /* Prepare the dataset for reading and start up the read streams */

    TSMTRACE(( TSMMASK|2, funcName, "Preparing set %d", set_id ));

    if ( issPrepareSet( connect->issHandle, set_id ) != ISS_OK ) {
      tsmError( funcName, "Couldn't prepare set %d. Try rebooting server(s)",
		set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    TSMTRACE(( TSMMASK|2, funcName, "Start request streams" ));
    if ( issConnectAllRequestStreams( connect->issHandle ) != ISS_OK ) {
      tsmError( funcName, "Couldn't connect set %d request streams", set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    TSMTRACE(( TSMMASK|2, funcName, "Start receiver streams" ));
    if ( issConnectAllRcvStreams( connect->issHandle ) != ISS_OK ) {
      tsmError( funcName, "Couldn't connect set %d rcv streams", set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    /* Then we sync the DPSS for the new dataset */

    TSMTRACE(( TSMMASK|2, funcName, "Check issReady" ));
    if ( issReady( connect->issHandle, set_id ) != ISS_OK ) {
      tsmError( funcName, "issReady failed for set %d", set_id );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

    /* print the servers that we are actually using for this connection */

    TSMTRACE(( TSMMASK|2, funcName, "Call dpssPrintServers" ));
    dpssPrintServers( connect->issHandle );

    /* Try to load the tspec file for the dataset */

    TSMTRACE(( TSMMASK|2, funcName, "About to load tspec for %s",
	       com_connect->name ));

    if (! com_connect->tspec) {
      com_connect->tspec = tsmReadTspec_dpss( connect, param );
      if ( com_connect->tspec == NULL ) {
	tsmError( funcName, "Couldn't parse the tspec file" );
	tsmDisconnect_dpss( aconn );
	return NULL;
      }

      /* Update the connect->name field from the tspec if */
      /* the name is currently just a set id number.      */

      if ( com_connect->name != NULL ) {
	if ( atoi( com_connect->name ) != 0 ) {
	  tsmMemManagerFree( com_connect->name );
	  com_connect->name = tsmMemStrdup( com_connect->tspec->identityTspec.name );
	}
      }

      /* set the set-id field in the tspec */

      com_connect->tspec->identityTspec.setId = connect->set_id;
    }
  } else if ( mode == TSM_WRITE_ONLY ) {

    /* Copy the specified server combo to the connection handle */
    /* if it has been specified.                                */

    if ( tsmIsParamSet( param, TSM_PARAM_SERVERS ) == TRUE ) {
      connect->numServers = param->numServers;
      connect->server = (char **) tsmMemMalloc( sizeof(char *) *
						param->numServers );
      
      for ( i = 0; i < connect->numServers; i++ )
	connect->server[i] = tsmMemStrdup( param->server[i] );

    } else {

      /* If no server list has been specified, then get the default */
      /* server list in the DPSS hwdb structure (iss_config).       */

      char **server_list = NULL;
      int  nservers;

      server_list = issGetServerWriteList( connect->issHandle, &nservers );
      if ( server_list == NULL) {
	tsmError( funcName, "Couldn't get default write servers from DPSS" );
	tsmDisconnect_dpss( aconn );
	return NULL;
      }

      connect->numServers = nservers;
      connect->server = (char **) tsmMemMalloc( sizeof(char *) * nservers );
      for ( i = 0; i < connect->numServers; i++ )
	connect->server[i] = tsmMemStrdup( server_list[i] );

    }

    /* Try to initialise the DPSS in preparation for a load */

    TSMTRACE(( TSMMASK|2, funcName, "About to init %s for DPSS load",
	       com_connect->name ));

    if ( tsmInitDPSSDataset( connect, param ) == FALSE ) {
      tsmError( funcName, "Couldn't initialise DPSS set for loading" );
      tsmDisconnect_dpss( aconn );
      return NULL;
    }

  }

  /* Don't start the request thread until requests have been submitted. */
  connect->requestThreadRunning = 0;
  if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE)
    tsmStartRequestThread_dpss (connect);

  return aconn;
}

/*
 * Synopsis:
 *   void tsmDisconnect_dpss( TsmConnection connect )
 *
 * Description:
 *   Does all of the DPSS-specific stuff for closing a connection. This
 *   includes closing the connections to the DSM and DPSS, if open.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

void
tsmDisconnectActual_dpss( ActualConnection aconn )
{
  const char        *funcName = "tsmDisconnectActual_dpss";
  TsmConnectionDpss *connect = (TsmConnectionDpss *) aconn;

  if ( connect == NULL ) return;

  /* stop the DPSS requester thread */

  if ( connect->requestThreadRunning ) {
    PR_Interrupt( connect->requestThread );
    PR_JoinThread (connect->requestThread);
    connect->requestThreadRunning = 0;
  }

  /* close the set if we were writing a new one */

  tsmFinishDPSSLoad( connect );

  /* Disconnect from the DSM if it has been opened */

  if ( connect->dsmHandle != NULL ) {
    dsmClose( connect->dsmHandle );
    connect->dsmHandle = NULL;
  }

  /* Disconnect from the DPSS if it has been opened */

  if ( connect->issHandle != NULL ) {
    if ( issClose( connect->issHandle ) != ISS_OK )
      tsmError( funcName, "unable to disconnect from the DPSS!" );
    else
      connect->issHandle = NULL;
  }

  if ( connect->jpegBuffer != NULL )
    tsmMemManagerFree( connect->jpegBuffer );

  /* Free up the DPSS request list if allocated */

  if ( connect->requestList != NULL )
    issFreeRequestList( connect->requestList );

  /* kill any running DPSS reconnect thread and destroy the mutex lock */

  PR_Lock( connect->reconnect_lock ); {
    if ( connect->reconnect_thread )
      PR_Interrupt( connect->reconnect_thread );
  }; PR_Unlock( connect->reconnect_lock );

  PR_DestroyLock( connect->reconnect_lock );

  tsmMemFree (connect);
}

void
tsmDisconnect_dpss (TsmConnection connect)
{
  static const char funcName[] = "tsmDisconnect_dpss";
  int        i;

  if ( connect == NULL ) return;

  TSMTRACE ((TSMMASK|1, funcName, "%p", connect));

  /* Do any protocol specific disconnection */

  if (connect->readers) {
    for (i = 0; i < connect->numReaders; ++i)
      tsmDisconnectActual_dpss (connect->readers[i]);
    tsmMemFree (connect->readers);
  }

  if (connect->writer)
    tsmDisconnectActual_dpss (connect->writer);

  tsmDisconnectCommon (connect);
}

/*
 * Synopsis:
 *   int tsmGetSetId( TsmConnection connect, TsmConnectionParams param )
 *
 * Doc++ Description: *//** 
 *   Finds a set ID for a connection.
 *
 *   This function will find the set id for a particular instantiation of
 *   a pyramid on a DPSS. A Pyramid stored on a DPSS is uniquely identified
 *   by a pyramid name, a list of servers, and an image format. (i.e. a 
 *   pyramid name is not enough.)
 *
 *   This function accepts a DPSS connection structure and
 *   (optionally) a connection parameter structure. If the connection
 *   structure has the server and numServer fields filled out, then
 *   these will be used to query the DSM for the set id. Otherwise,
 *   these data are assumed to be in the connection parameter
 *   structure. Likewise, the image format to match against is taken
 *   from the connection structure, unless this is TSM_NONE_FORMAT in
 *   which case the connection parameter structure is used.
 *
 *   If TSM_PARAM_FALLBACK is TRUE, then it is possible to only supply
 *   an image format to match against. In which case we will try to find
 *   any server combination of the dataset with the specified image format.
 *   If TSM_PARAM_FALLBACK is FALSE, then a server combination and image
 *   format must be specified.
 *
 * @return   The pyramid's set ID, or -1 if the pyramid doesn't exist yet
 *
 * @author   Martin Reddy
 * @version  13 January 1998
 *
 * @see      
 *
 */

int
tsmGetSetId( TsmConnectionDpss *connect, TsmConnectionParams param )
{
  const char    *funcName = "tsmGetSetId";
  TsmConnection com_connect;
  dsmSetInfo    *dsiarray, setinfo;
  IssServerInfo *serverlist;
  int           nsets, nservers, i, m, n, same, set_id = -1;
  int           numServers;
  int           src_format, test_format;
  char          **server;

  /* Check that everything is opened okay */
  
  if ( connect == NULL ) return set_id;
  com_connect = connect->aconn.conn;

  if ( connect->dsmHandle == NULL || connect->issHandle == NULL ) {
    tsmError( funcName, "Connection must have DSM and DPSS open" );
    return set_id;
  }

  /* use the connection server info, if provided. Otherwise */
  /* go to the connection parameter structure for the info  */

  if ( connect->server != NULL ) {
    numServers = connect->numServers;
    server     = connect->server;
  } else if ( tsmIsParamSet( param, TSM_PARAM_SERVERS ) ) {
    numServers = param->numServers;
    server     = param->server;
  } else if ( tsmIsParamSet( param, TSM_PARAM_FALLBACK ) &&
	      param->fallback == FALSE ) {
    return set_id;
  } else {
    numServers = 0;  /* don't care about server combination */
  }

  if ( com_connect->format != TSM_NONE_FORMAT )
    src_format = com_connect->format;
  else if ( tsmIsParamSet( param, TSM_PARAM_FORMAT ) )
    src_format = param->format;
  else
    src_format = TSM_LRBT_FORMAT;

  /* get the list of all pyramids from the DSM */

  dsiarray = dsmSetList( connect->dsmHandle, DSM_PYRAMID_TYPE, &nsets );
  if ( dsiarray == NULL )
    return set_id;

  /* search through the list for a name/server list match  */

  for ( i = 0; i < nsets && set_id < 0; ++i ) {
    setinfo = dsiarray[i];

    if ( strcmp( dsiName(setinfo), com_connect->name ) == 0 ) {

      /* we have found a pyramid with the same name, so let's get */
      /* it's compression value and see if this matches also.     */

      if ( dsiCompression(setinfo) == ISS_JPEG )
	test_format = TSM_JPEG_FORMAT;
      else
	test_format = TSM_LRBT_FORMAT;
      
      if ( test_format == src_format ) {

	/* If we didn't care about the server combo, then we've matched */

	if ( numServers == 0 )
	  set_id = dsiSetid(setinfo);
	else {

	  /* Everything matches so far, so now let's get the list */
	  /* of servers and see if they are the same also         */

	  nservers = issGetServerNames( connect->issHandle, dsiSetid(setinfo),
					&serverlist );

	  if ( nservers == numServers ) {

	    /* we have the same number of servers now too - getting close!  */
	    /* now we shall just check to see if the server names are the   */
	    /* same. N.B. this routine will match server names in any order */
	    /* e.g. "a","b" will match "b","a" - this is right isn't it?    */
	  
	    for ( n = same = 0; n < nservers; ++n )
	      for ( m = 0; m < nservers; ++m )
		if ( strcmp( serverlist[m].server_name, server[n] ) == 0 )
		  same++;
	    
	    if ( same == nservers )
	      set_id = dsiSetid(setinfo);
	  }

	  if ( nservers > 0 )
	    free( serverlist );
	}
      }
    }
  }

  /* free up the array returned by dsmSetList and return the set_id */

  for ( i = 0; i < nsets; i++ )
    deleteDsmSetInfo( dsiarray[i] );
  free( dsiarray );

  return set_id;
}

int
tsmBeginLogging_dpss( TsmConnection com_connect, int method,
		      const char *destination )
{
  static const char *funcName = "tsmBeginLogging_dpss";
  ActualConnection aconn = com_connect->readers[0];
  TsmConnectionDpss *connect = (TsmConnectionDpss *) aconn;
  char              buffer[512];
  TsmAccessMode     mode;
  extern char       *pname;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p, destination = %s",
	     connect, buffer ));

  tsmvBeginLogging (com_connect);

  /* If we are already logging, then stop that before starting again */

  if ( connect->dpssLogging == TRUE )
    tsmEndLogging_dpss( aconn );

  /* N.B. These DPSS routines rely on a global string called pname */
  /* being set to the name of the program being logged.            */

  pname = "TerraVision";

  /* grab a copy of the connection's access mode */

  mode = com_connect->mode;

  switch ( method ) {
  case NL_FILE:
    /* log to a local file - specified file with .dpss extension */
    sprintf( buffer, "%s.dpss", destination );
    if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE )
      issStartRcvLogging( connect->issHandle, method, buffer, NULL, 0 );
    if ( mode == TSM_WRITE_ONLY )
      issStartSendLogging( connect->issHandle, method, buffer, NULL, 0 );
    break;

  case NL_SYSLOG:
    /* log to syslog */
    if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE )
      issStartRcvLogging( connect->issHandle, method, NULL, NULL, 0 );
    if ( mode == TSM_WRITE_ONLY )
      issStartSendLogging( connect->issHandle, method, NULL, NULL, 0 );
    break;

  case NL_HOST:
    /* (use the Netlogger program "netlogd" to collect these logs) */
    if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE )
      issStartRcvLogging( connect->issHandle, method, NULL, (char *)
			  destination, DPSS_NETLOGGER_PORT );
    if ( mode == TSM_WRITE_ONLY )
      issStartSendLogging( connect->issHandle, method, NULL, (char *)
			   destination, DPSS_NETLOGGER_PORT );
    break;
    
  default:
    tsmError( funcName, "Unsupported NetLogger method" );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  connect->dpssLogging = TRUE;

  TSMTRACE(( TSMMASK|2, funcName, "Done." ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

int
tsmEndLogging_dpss (TsmConnection com_connect)
{
  static const char *funcName = "tsmEndLogging_dpss";
  ActualConnection aconn = com_connect->readers[0];
  TsmConnectionDpss *connect = (TsmConnectionDpss *) aconn;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p.", connect ));

  /* If we are not currently logging DPSS data, then there's nothing to do */

  if ( connect->dpssLogging == FALSE ) {
    TSMTRINC (( TSMMASK|2, -1 ));
    return TRUE;
  }

  /* Otherwise, stop the current logging process */

  switch ( com_connect->mode ) {
  case TSM_READ_ONLY:
  case TSM_READ_WRITE:
    issStopRcvLogging( connect->issHandle );
    break;
    
  case TSM_WRITE_ONLY:
    issStopSendLogging( connect->issHandle );
    break;
  }

  connect->dpssLogging = FALSE;

  tsmvEndLogging (com_connect);

  TSMTRACE(( TSMMASK|2, funcName, "Done." ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

int
tsmCheckTileFormat_dpss ( TsmConnection conn, TsmTileFormat fmt)
{
  return FALSE;
}

int
tsmCheckTIFFFormat_dpss( TsmConnection conn, int fmt)
{
  return FALSE;
}

#else

TsmConnection
tsmConnect_dpss( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  tsmNotCompiledIn( purl->protocol, "tsmConnect_dpss" );
  return NULL;
}

void
tsmDisconnect_dpss(TsmConnection connect)
{
  tsmNotCompiledIn( connect->location->protocol, "tsmDisconnect_dpss" );
}

int
tsmBeginLogging_dpss( TsmConnection com_connect, int method,
		      const char *filename )
{
  tsmNotCompiledIn( com_connect->location->protocol, "tsmBeginLogging_dpss" );
  return FALSE;
}

int
tsmEndLogging_dpss( TsmConnection com_connect )
{
  tsmNotCompiledIn( com_connect->location->protocol, "tsmEndLogging_dpss" );
  return FALSE;
}

#endif
