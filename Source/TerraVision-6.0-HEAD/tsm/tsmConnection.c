/*
 * Name: tsmConnection.c
 *
 * Description: 
 *   Contains user-level routines for manipulating dataset connections.
 *   These functions are externally visible to the user.
 *
 * Function List:
 *   TsmConnection *tsmConnect( char *, char *, TsmConnectionParams )
 *   void tsmDisconnect( TsmConnection connect )
 *
 *   void tsmSet( TsmConnection connect, TsmSetting setting, ... )
 *   int tsmGet( TsmConnection connect, TsmSetting setting )
 *
 *   int tsmBeginLogging( TsmConnection connect, int method, const char * )
 *   int tsmEndLogging( TsmConnection connect )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnection.c,v 1.55.2.7 2002/03/14 05:22:55 reddy Exp $
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
#include "tsmConnectionInt.h"
#include "tsmConnectionParamsInt.h"
#include "tsmRequestSrv.h"

#include <vrml2/c_nodes.h>

/*
 * Synopsis:
 *   TsmConnection *tsmConnect( char *, char *, TsmConnectionParams )
 *
 * Doc++ Description: *//** 
 *   Open a connection to a dataset.
 *
 *   In order to do anything to a dataset, you need to use this function
 *   to open a connection to that particular dataset. Note that you use
 *   one connection for one dataset, so if you wish to open two datasets
 *   then you will need to do to tsmConnect's.
 *
 *   In order to open a connection you need to specify two pieces of
 *   information (with an optional third). These are:
 *
 *   \begin{enumerate}
 *      \item The name and location of the dataset. This can be a local 
 *            filesystem filename, or a URL. Examples of valid dataset
 *            names include:
 *
 *   \begin{enumerate}
 *    \item {\tt ~magic/TileSets/Pyramids/ntc-1-utm-2scene.128.oi}
 *    \item {\tt file://localhost/homedir/magic/TileSets/Pyramids/yosemite.oi}
 *    \item {\tt x-dpss://iss-1.sri.magic.net/Pyramids/ntc-1-utm-2scene.33.dem}
 *   \end{enumerate}
 *
 *      \item The access mode that you wish for the connection. Currently
 *            you can request a read-only connection ("r"), a
 *            write-only connection ("w"), or a read-write connection
 *            ("rw"). For example, if you request a read-only connection
 *            then you will not be able to use the tsmWriteTile() function.
 *
 *      \item (Optional) You can also provide a TsmConnectionParams structure
 *            which lets you specify further details about the dataset and
 *            override any default settings (such as the image format of
 *            tiles to use). If you don't want to specify this, then you 
 *            can pass a NULL pointer for this parameter. To find out what
 *            details you can specify with the TsmConnectionParams structure
 *            and how to use it, please refer to the triumvirate:
 *            tsmInitParams, tsmSetParams, and tsmFreeParams.
 *   
 *   \end{enumerate}
 *
 *   Once you have opened a connection to a dataset, then you can do
 *   any processing that you wish, e.g. read tiles from the dataset 
 *   (if you have requested a readable connection), or write new tiles
 *   to the dataset (if you have asked for a write connection). When
 *   you have finished manipulating the dataset, you call the tsmDisconnect
 *   function to close the connection and free up any held resources.
 *
 *   The following list provides details to help you understand how the
 *   use of the TsmConnectionParams structure affects the dataset that
 *   is actually connected to:
 *
 *   {\bf Reading a Local Disk Dataset} (file:// URL)
 *
 *   \begin{itemize}
 *   \item {\bf Reading Tile Data} :
 *   Local disk pyramids can contain any of a number of different 
 *   representations of the tile data. These include .tile files (1 .tile
 *   file per tile), and .tiff files (1 .tiff file per pyramid level). In
 *   addition, you can have raw .tiff files or jpeg compressed .tiff files.
 * 
 *   You can specify if you want to use the raw .tile format tiles by
 *   setting TSM_PARAM_TIFF to FALSE, otherwise the default will be to
 *   use a TIFF format file for the data.
 *
 *   If you are asking for a TIFF file, then you can use the TSM_PARAM_FORMAT
 *   hint to specify if you want to use the raw lrbt (TSM_LRBT_FORMAT), or
 *   jpeg compressed (TSM_JPEG_FORMAT) version of the dataset. The default
 *   is to use the raw lrbt tiles. This hint will be ignored if you have
 *   set TSM_PARAM_TIFF to FALSE.
 *
 *   If the specified combination of .tile/.tiff and lrbt/jpeg does not
 *   exist, then tsmApi will try to find another version of dataset that
 *   it can use, e.g. if the lrbt TIFF file doesn't exist and there is
 *   a jpeg TIFF, then tsmApi will use this one instead. You can override
 *   this behaviour by setting TSM_PARAM_FALLBACK to FALSE. In which case,
 *   tsmApi will fail immediately if it cannot fulfil your specific dataset
 *   requirements.
 *
 *   \item {\bf Reading Tspecs} : Tspecs can be stored in two different 
 *   formats: an older multi-file format where many different tspec files
 *   were used, and a newer single-file format where all this information
 *   is stored in a single file. By default, tsmApi will try to load the
 *   newer style tspec file. If this fails (e.g. file does not exist), then
 *   it will fallback and try to load the older style tspec files. You
 *   can override this by setting the value of TSM_PARAM_NEW_TSPEC. If
 *   this is TRUE, then only the new single-file tspec will be looked
 *   for. If this is FALSE, then only the older tspec files will be
 *   looked for.
 *
 *   Once a tspec file has been read it, it will be validated to check 
 *   that it contains valid data. This check is performed by the
 *   tsmValidateTspec routine (refer to the documentation for this page
 *   to see which fields are validated). You may disable this validation
 *   process by setting the TSM_PARAM_VALIDATE_TSPEC param to FALSE using
 *   tsmSetParam.
 *   \end{itemize}
 *
 *   {\bf Reading a DPSS Dataset} (x-dpss:// URL):
 *
 *   \begin{itemize}
 *   \item {\bf Reading Tile Data} :
 *   A dataset on a DPSS is uniquely identified by a set-id. It is 
 *   entirely possible to have two datasets with exactly the same name,
 *   server combination, and image format; as long as their set IDs differ.
 *   The following steps are performed in order to try and find a dataset's
 *   set ID:
 *
 *   \begin{enumerate}
 *   \item If the dataset URL that you provide is in the form
 *   x-dpss://host/Pyramids/set-id then this set ID is used.
 *
 *   \item Next, if the param structure has a set ID defined
 *   (TSM_PARAM_SETID), then this value is used.
 *
 *   \item Otherwise, if the URL that you provide is in the form
 *   x-dpss://host/Pyramids/name and the param structure has a list of
 *   server names defined (TSM_PARAM_SERVERS) then these are used to
 *   try and find a dataset with that combination of servers. In
 *   addition, if you specify an image format with TSM_PARAM_FORMAT
 *   then this will try to be matched against also (the default image
 *   format is raw lrbt).
 *
 *   \item If all of this fails to find a valid set ID, then tsmApi will try
 *   to find any dataset with the specified name on the DPSS (i.e. this
 *   could be any server combination and any image format).
 *   \end{enumerate}
 *
 *   N.B. if you set TSM_PARAM_FALLBACK to be FALSE, then tsmApi
 *   will fail immediately at step 3) if the specified server/image
 *   combination does not exist (i.e. you need to specify a server
 *   combination if you set TSM_PARAM_FALLBACK to FALSE).
 *
 *   \item {\bf Reading Tspecs} : Tspec data for a DPSS dataset can
 *   exist either on some other server (accessed via http or file
 *   protocols), or they can be embedded in a block in the DPSS
 *   dataset itself. By default, tsmApi will check to see if the
 *   dataset exists on the DPSS dataset itself. It this fails, then
 *   the various metadata (tspec) URLs for that set are tried in turn
 *   until one succeeds. If you want to override the loading of the
 *   tspec file from the DPSS set, then set TSM_PARAM_DPSS_TSPEC to
 *   FALSE.  This tells tsmApi to only try the metadata URLs.
 *
 *   The TSM_PARAM_VALIDATE_TSPEC param can also be set to FALSE to
 *   bypass the tspec validation process once it has been found and
 *   loaded.
 *   \end{itemize}
 *
 *   {\bf Reading HTTP Datasets} (http:// URL):
 *
 *   \begin{itemize}
 *   \item {\bf Reading Tile Data} : reading of tile data is not
 *   currently supported over http links.
 *
 *   \item {\bf Reading Tspecs} : Reading tspec files over http has
 *   the same behaviour as reading tspec files from localdisk. That
 *   is, the new tspec format will be tried first, and if this is not
 *   available then the old tspec files will be tried. You can set the
 *   value of TSM_PARAM_NEW_TSPEC to override this default behaviour
 *   as you can for localdisk datasets, and TSM_PARAM_VALIDATE_TSPEC
 *   param can be set to FALSE to bypass the tspec validation process
 *   upon loading.
 *
 *   \end{itemize}
 *
 * @param    dataset a filename or URL for the pyramid dataset to connect to
 * @param    access_mode either "r" (read), "w" (write), or "rw" (read/write)
 * @param    param a number of hints to specify more information about
 *           the dataset, or NULL
 *
 * @return   An allocated connection structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  9 January 1997
 *
 * @see      tsmDisconnect, tsmInitParams, tsmSetParams, tsmReadTile,
 *           tsmWriteTile, tsmGet, tsmSet
 * */

TsmConnection
tsmConnect( char const *dataset, 
	    char const *access_mode,
	    TsmConnectionParams param )
{
  const char         *funcName = "tsmConnect";    /* function name */
  Purl               *purl;                       /* location of dataset */
  TsmAccessMode      mode;                        /* read or write? */
  TsmConnection      connect = NULL;              /* cast of one of above */
  
  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Entering..." ));

  /* Check the mode string to see if it is one of "r" (read), */
  /* "w" (write) or "rw" (read and write).                    */

  if ( strcmp( access_mode, "r" ) == 0 ) 
    mode = TSM_READ_ONLY;
  else if ( strcmp( access_mode, "w" ) == 0 )
    mode = TSM_WRITE_ONLY;
  else if ( strcmp( access_mode, "rw" ) == 0 )
    mode = TSM_READ_WRITE;
  else {
    tsmError( funcName, "Unsupported mode: %s. Use \"r\", \"w\" or \"rw\"",
	      access_mode );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* Try to parse the dataset URL */

  if ( ( purl = tsmParseUrl( dataset ) ) == NULL ) {
    tsmError( funcName, "Couldn't parse URL: %s", dataset );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  switch ( purl->protocol ) {
  case tsmISS:
  case tsmDSM:
  case tsmDPSS:

    /* Set up a connection for a DPSS dataset */

    connect = tsmConnect_dpss( purl, mode, param );
    break;

  case tsmHTTP:

    /* Set up a connection for an HTTP dataset */

    connect = tsmConnect_web( purl, mode, param );
    break;

  case tsmLocalFile:

    /* Set up a connection for a LOCAL FILESYSTEM dataset */

    connect = tsmConnect_local( purl, mode, param );
    break;

  case tsmWMT:

    /* Set up a connection for an OGC Web Mapping Server */

    connect = tsmConnect_wms( purl, mode, param );
    break;

  default:
    tsmNoProtocolYet( purl->protocol, funcName );
    /* fall through to the (connect == NULL) check below */
  }

  return connect;
}

TsmConnection
tsmConnectCommon (TsmConnection connect,
		  Purl *purl,
		  TsmAccessMode mode,
		  TsmConnectionParams param)
{
  static char const funcName[] = "tsmConnectCommon";

  /* Are we doing blocking reads for this connection? If the client  */
  /* has specified no preference, then the default is to do blocking */

  if ( tsmIsParamSet( param, TSM_PARAM_BLOCKING_READ ) )
    connect->blocking_read = param->blocking_read;
  else
    connect->blocking_read = TRUE;

  /* Now we will check to see if we can load a tspec file for the */
  /* dataset. This can fail if we have opened a write connection, */
  /* but otherwise a failure to complete this operation is fatal. */

  if ( mode != TSM_WRITE_ONLY )
    connect->tspec = tsmReadTspec( purl->url, param );

  if ( connect->tspec == NULL && mode != TSM_WRITE_ONLY 
       && purl->protocol != tsmISS
       && purl->protocol != tsmDSM
       && purl->protocol != tsmDPSS) {
    tsmError( funcName, "Couldn't parse the tspec file" );
    tsmDisconnect (connect);
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* Update the connect->name and other fields from the tspec */

  connect->tspec_url = tsmMemStrdup( purl->url );
  if ( connect->name != NULL ) tsmMemFree( connect->name );

  if ( connect->tspec != NULL ) {
    connect->tspec->identityTspec.setId = tsmGenerateUniqueSetID();
    connect->name =
      tsmMemStrdup( connect->tspec->identityTspec.name );
  } else {
    char *parsed_name = strrchr( purl->url, '/' );
    if ( parsed_name == NULL ) parsed_name = purl->url; else parsed_name++;
    connect->name = tsmMemStrdup( parsed_name );
  }
  
  connect->dont_read_tiles = FALSE;
  connect->jpeg_quality    = 75;
  connect->show_progress   = TRUE;

  /* put purls for the TSM into the other Pyramid fields so */
  /* that legacy code relying on these will still work.     */
  /* N.B. this may have been done by the tspec reading func */

  if ( connect->tspec != NULL ) {
    if ( connect->tspec->tileBasePurl == NULL )
      connect->tspec->tileBasePurl = tsmCopyPurl( purl );
    if ( connect->tspec->tspecBasePurl == NULL )
      connect->tspec->tspecBasePurl = tsmCopyPurl( purl );
  }

  /* Calculate the size of a tile buffer for this dataset. If we */
  /* don't know the size yet (e.g. we are writing a dataset and  */
  /* no tspec has been created yet), then make a worst case guess*/

  connect->tileBufferSize = tsmNumBytesPerTile( connect->tspec );

  if ( connect->tileBufferSize == 0 )
    connect->tileBufferSize = 128 * 128 * 3;

  /* Work out the type of the dataset from the tspec file, if available */
  /* The type refers to whether the dataset is a DEM or OI pyramid.     */

  connect->dataset_type = TSM_UNKNOWN_DATASET;

  if ( connect->tspec != NULL ) {
    switch ( connect->tspec->identityTspec.type ) {
    case tspDemType:
      connect->dataset_type = TSM_DEM_DATASET;
      break;
    case tspOiType:
      connect->dataset_type = TSM_IMAGE_DATASET;
      break;
    }
  }

  /* If we have selected jpeg format for a DEM dataset, then */
  /* that's a no-no. We don't support DEMs in JPEG format    */
  /* (for one, JPEG doesn't allow 32-bits per sample).       */
  /* Also, LZW is currently broken for DEMs because the tiff */
  /* library lzw support won't handle our overlap pixels.    */

  if ( connect->dataset_type == TSM_DEM_DATASET &&
       ( connect->format == TSM_JPEG_FORMAT ||
	 connect->format == TSM_LZW_FORMAT ) ) {

    /* If TSM_PARAM_FALLBACK is FALSE then we can't complete the connect */

    if ( tsmIsParamSet( param, TSM_PARAM_FALLBACK ) && ! param->fallback ) {
      tsmError( funcName, "We don't support JPEG/LZW format DEM datasets!\n" );
      tsmDisconnect( connect );
      TSMTRINC(( TSMMASK|2, -1 ));
      return NULL;
    }

    /* otherwise we can silently switch to lrbt DEMs */

    connect->format = TSM_LRBT_FORMAT;
  }

  /* We have now filled in the protocol specific information of the */
  /* connection structure. Now let's fill in the common information */
  /* This meands the read/request lists and associated locks.       */

  /* There is one request server per connection.  It acts as a broker */
  /* to handle all of the request lists that are associated with this */
  /* particular TsmConnection */

  connect->srv = tsmReqServerCreate (connect);
  connect->globalReqs = tsmReqServerCreateList (connect->srv);

  connect->protoWrl = NULL;
  connect->proto = NULL;
  vsgcSetup();

  if ( !(connect->logLock = PR_NewLock()) ) {
    tsmError( funcName, "Unable to create logging lock." );
    tsmDisconnect (connect);
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  TSMTRINC(( TSMMASK|2, -1 ));
  return connect;
}

/*
 * Synopsis:
 *      tsmRequestList * tsmConnectRequestList( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Create a new tile request list.
 *
 *   tsmConnectRequestList creates a new "connection" for accumulating tile
 *   requests to the TsmConnection.  Each such request list is separately
 *   maintained and managed and therefore represents an independent means of
 *   accessing the tiles associated with a given connection \em{without}
 *   opening a completely new connection to the same dataset.
 *
 *   Once this function has been called, you can manage the request list
 *   completely independent of the connection using the tsmRequestAdd and
 *   tsmRequestSubmit calls on the tsmRequestList.
 *
 * @param    connect a dataset connection structure
 *
 * @return   A new tsmRequestList * or NULL if failed.
 *
 * @author   Lee Iverson
 * @version  November 7, 2000
 *
 * @see      tsmRequestList, tsmRequestAdd, tsmRequestSubmit, tsmRequestDestroy
 *
 */

tsmRequestList *
tsmConnectRequestList (TsmConnection connect)
{
  static char const funcName[] = "tsmConnectRequestList";
  tsmRequestList *req;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, FALSE ) )
    return NULL;
  
  req = tsmReqServerCreateList (connect->srv);

  TSMTRACE ((TSMMASK|1, funcName, "%p", req));

  return req;
}

/*
 * Synopsis:
 *   void tsmDisconnectCommon ( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Close a connection.
 *
 *   Disconnect from a dataset and free up all memory associated
 *   with the connection. Use this function when you have finished
 *   all desired operations on a dataset.
 *
 *   If you are disconnecting from a DPSS dataset you have been
 *   writing to, then the TSM_FINISH_LOAD_ON_DISCONNECT flag will be
 *   honoured here.  Refer to tsmSet for details on setting this flag
 *   and what it does.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  12 January 1998
 *
 * @see      tsmConnect, tsmSet
 * */

void
tsmDisconnectCommon (TsmConnection connect)
{
  static const char funcName[] = "tsmDisconnectCommon";

  if ( connect == NULL ) return;

  TSMTRACE ((TSMMASK|1, funcName, "%p", connect));

  /* Free up any strings, Purls, tspec structures, etc. */

  if ( connect->name != NULL )
    tsmMemManagerFree( connect->name );

  if ( connect->tspec_url != NULL )
    tsmMemManagerFree( connect->tspec_url );

  if ( connect->location != NULL )
    tsmFreePurl( connect->location );

  if ( connect->tspec != NULL )
    tsmFreeTspec( connect->tspec );

  if ( connect->logging == TRUE )
    tsmEndLogging( connect );

  tsmDestroyProtoWrl( connect );

  /* Clean up all request lists for this connection */
  if (connect->globalReqs)
    tsmRequestDestroy (connect->globalReqs);

  if (connect->srv)
    tsmReqServerDestroy (connect->srv);

  /* Free up any arrays/locks that were allocated for request/read lists */

  if (connect->logLock) PR_DestroyLock( connect->logLock );
}

/*
 * Synopsis:
 *   int tsmCheckConnectMode( const char *funcName, TsmConnection connect,
 *                            TsmAccessMode mode, int tspecRequired )
 *
 * Description:
 *   This function should be the first thing that is called by any
 *   connection function which does any I/O. It is used to check the
 *   validity of the connection parameter, as well as impose the 
 *   access mode of the connection. That is, you specify the access 
 *   mode required by the operation, and if it is not compatible 
 *   with the connection's access mode, then you are returned a value
 *   of FALSE. N.B. This routine outputs any relevant error messages
 *   so you just need to do a return if you get a FALSE back.
 *
 * Returns: TRUE if the connection supports the access mode, FALSE if not.
 *
 * Author: Martin Reddy
 *
 * Date: 5 February 1998
 *
 */

int
tsmCheckConnectMode( const char *funcName, TsmConnection connect,
		     TsmAccessMode mode, int tspecRequired )
{
  char descr[25];

  /* First of all, check to see if we have been give a non-NULL */
  /* connection handle! This would be kinda handy to have!      */

  if ( connect == NULL ) {
    tsmError( funcName, "NULL connection parameter passed - aborting" );
    return FALSE;
  }

  /* We can optionally check to make sure that we have a tspec */
  /* structure filled in for the connection. If this is a read */
  /* connection then this will always be the case, but if we   */
  /* are just creating the dataset, then we may not have gotten*/
  /* that far yet.                                             */

  if ( tspecRequired && connect->tspec == NULL ) {
    tsmError( funcName, "Connection has no tspec file associated with it" );
    return FALSE;
  }

  /* Now check to see if the access mode for this operation is */
  /* compatible with the access mode of the connection, i.e.   */
  /* you can't do a write operation if you've opened a read    */
  /* only connection. By definition, a read/write connection   */
  /* can perform any I/O operation.                            */

  if ( mode != TSM_READ_WRITE && connect->mode != TSM_READ_WRITE ) {
    if ( connect->mode != mode ) {
      switch ( connect->mode ) {
      case TSM_READ_ONLY:
	strcpy( descr, "read" );
	break;
      case TSM_WRITE_ONLY:
	strcpy( descr, "write" );
	break;
      case TSM_READ_WRITE:
	strcpy( descr, "read/write" );
	break;
      }
      
      tsmError( funcName, "Invalid operation: connection is %s only", descr );
      return FALSE;
    }
  }

  /* All okay, so let's tell the Big Guy who called us */

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmBeginLogging( TsmConnection connect, int method, const char * )
 *
 * Doc++ Description: *//** 
 *   Start logging tile statistics.
 *
 *   tsmApi offers functionality to log a number of time-stamped events
 *   to either a log file, to a port/host combination, or to syslog. This
 *   functionality is implemented using the NetLogger library from
 *   LBNL (http://www-didc.lbl.gov/NetLogger).
 *
 *   Logging is off by default. In order to start logging you must first
 *   call this function for the particular connection. To do this you
 *   specify the logging method you wish to use (NL_FILE, NL_SYSLOG, or
 *   NL_HOST). If you specify NL_FILE, then you must supply the name of
 *   the log file you wish to generate in the destination parameter. If
 *   you specify NL_HOST, then you must specify the name of the host that
 *   you wish to log to via the destination parameter. If you specify
 *   NL_SYSLOG, then you should pass NULL for destination.
 *   
 *   Currently, it is hardwired that: a) the program name that is
 *   written out for each event is "TerraVision", and b) the port to
 *   use when you specify NL_HOST is always DPSS_NETLOGGER_PORT
 *   (14830).
 *
 *   Once logging has been started, a number of tsmApi calls will
 *   automatically generate log events, e.g. tsmReadTile. However, you
 *   can explicitly send your own log event to the log file by using
 *   the tsmLog function. When you are done logging, you should call
 *   tsmEndLogging.
 *
 *   N.B. if the connection that is being logged is for a DPSS dataset,
 *   then this function will also tell the DPSS to start logging its
 *   data (which is also done using NetLogger). If you have requested
 *   logging to a file, then the DPSS logging will be sent to the filename 
 *   given by the filename you specified in the destination parameter with
 *   a ".dpss" extension appended.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 * @param    method one of NL_FILE, NL_SYSLOG, or NL_HOST
 * @param    destination the filename to write the log data to (if method
 *           is NL_FILE), or the host to write the log data to (if method
 *           is NL_HOST).
 *
 * @return   TRUE if logging started successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  21 May 1998
 *
 * @see      tsmEndLogging, tsmLog
 *
 */

int
tsmvBeginLogging( TsmConnection connect, int method, const char *destination )
{
  static const char *funcName = "tsmvBeginLogging";
  static char       *pname    = "TerraVision";

  if ( connect == NULL ) return FALSE;

  /* If we are currently logging, then stop it! */

  if ( connect->logging == TRUE )
    tsmEndLogging( connect );

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|1, funcName, "connect = %p, destination = %s",
	     connect, destination ));

  /* Open a NetLogger session for the desired method */

  switch ( method ) {
  case NL_FILE:
    connect->nlhandle = NetLoggerOpen( method, pname, (char *) destination,
				       NULL, 0 );
    break;
  case NL_SYSLOG:
    connect->nlhandle = NetLoggerOpen( method, pname, NULL, NULL, 0 );
    break;
  case NL_HOST:
    connect->nlhandle = NetLoggerOpen( method, pname, NULL, (char *)
				       destination, DPSS_NETLOGGER_PORT );
    break;
  default:
    tsmError( funcName, "Unsupported NetLogger method" );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* Check that we were able to start up NetLogger */

  if ( connect->nlhandle == NULL ) {
    tsmError( funcName, "Unable to start NetLogger!" );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* Check that we were able to open the output log file/socket */
  /* NetLoggerOpen returns a valid NLHandle even if the file or */
  /* socket could not be opened!!! */

  if ( connect->nlhandle->log_fp == NULL &&
       connect->nlhandle->log_socket == 0 ) {

    if ( connect->nlhandle->pname ) tsmMemFree( connect->nlhandle->pname );
    if ( connect->nlhandle->hname ) tsmMemFree( connect->nlhandle->hname );
    if ( connect->nlhandle->level ) tsmMemFree( connect->nlhandle->level );
    tsmMemFree( connect->nlhandle );
    connect->nlhandle = NULL;

    tsmError( funcName, "Unable to open NetLogger file/socket!" );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  /* All done. */
  connect->logging = TRUE;

  TSMTRACE(( TSMMASK|1, funcName, "Logging started..." ));
  TSMTRINC(( TSMMASK|1, -1 ));

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmLog( TsmConnection connect, const char *event_name,
 *               const char * fmt, ... )
 *
 * Doc++ Description: *//** 
 *   Send a log event to the log file.
 *
 *   Once you have started logging of data with the tsmBeginLogging call,
 *   you can use this function to send a log event to the log file. For
 *   example, 
 *
 *   tsmLog( connect, "EVENT_NAME", "TILE=%d.%d.%d", x, y, res );
 *
 *   The tsmApi defines a number of standard events that it uses to
 *   instrument your session. These events are automatically logged for
 *   you once you have started logging:
 *
 *   \begin{itemize}
 *   \item {\bf TV_TILE_REQ}: an event triggered whenever a tile is
 *         requested by the application program.
 *   \item {\bf TV_TILE_READ} : an event triggered whenever a tile is
 *         received by the tsmApi
 *   \item {\bf TV_TILE_WRITE} : an event triggered whenever a tile is
 *         written to a storage medium.
 *   \end{itemize}
 *
 *   In addition to these automatically-logged events, the tsmApi defines
 *   a number of standard events for an application to use to log the
 *   life of a tile beyond the tsmApi:
 *
 *   \begin{itemize}
 *   \item {\bf TV_TILE_IN_CACHE} : an event triggered whenever a tile is
 *         placed in some general cache by the application program
 *   \item {\bf TV_TILE_OUT_CACHE} : an event triggered whenever a tile is
 *         removed from the general cache by the application program
 *   \item {\bf TV_TILE_IN_TEXCACHE} : an event triggered whenever a tile is
 *         placed in the texture-map cache by the application program
 *   \item {\bf TV_TILE_OUT_TEXCACHE} : an event triggered whenever a tile is
 *         removed from the texture-map cache by the application program
 *   \end{itemize}
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 * @param    event_name a string describing the event being logged
 * @param    data a printf style string specifying the data to be logged for
 *           the event
 *
 * @return   TRUE if data was logged successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  21 May 1998
 *
 * @see      tsmBeginLogging, tsmEndLogging
 *
 */

int
tsmLog( TsmConnection connect, const char *event_name, const char *data, ... )
{
  va_list args;
  char    buffer[512];
  int     result;

  if ( connect == NULL ) return FALSE;
  if ( connect->logging == FALSE ) return TRUE;

  PR_Lock( connect->logLock ); {

    /* add the user's list of NetLogger events */

    buffer[0] = '\0';
    va_start( args, data );
    vsprintf( buffer, data, args );
    va_end( args );
    
    /* add our connection-specific events */
    
    sprintf( buffer, "%s DATASET=%s", buffer, connect->name );
    
    /* and then send the event string to the NetLogger API */

    result = NetLoggerWriteString( connect->nlhandle, (char *) event_name,
				   (char *) buffer );
  }; PR_Unlock( connect->logLock );

  return ( result == 1 );
}

/*
 * Synopsis:
 *   int tsmEndLogging( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Stop logging tile statistics.
 *
 *   If you have previously called tsmBeginLogging for this connection,
 *   then this function will let you stop that logging. This function
 *   will be called automatically by tsmDisconnect if you forget.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 *
 * @return   TRUE if logging stopped successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  21 May 1998
 *
 * @see      tsmBeginLogging, tsmLog
 *
 */

int
tsmvEndLogging( TsmConnection connect )
{
  static const char *funcName = "tsmvEndLogging";

  if ( connect == NULL ) return FALSE;
  if ( connect->logging == FALSE ) return TRUE;

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|1, funcName, "connect = %p", connect ));

  if ( connect->nlhandle != NULL ) {
    NetLoggerClose( connect->nlhandle );
    connect->nlhandle = NULL;
  }

  connect->logging = FALSE;

  TSMTRACE(( TSMMASK|1, funcName, "Logging stopped." ));
  TSMTRINC(( TSMMASK|1, -1 ));

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmSet( TsmConnection connect, TsmSetting setting, ... )
 *
 * Doc++ Description: *//** 
 *   Set a connection variable.
 *
 *   The tsmSet function lets you change various characteristics of a
 *   connection. These can be use to control things such as the format
 *   of data to write tiles to, the JPEG compression quality, etc.
 *
 *   This function uses varargs for the actual parameter value so that you
 *   can pass in values of any time. This does mean however that you should
 *   be careful to supply a value of the correct type for the parameter code
 *   that you specify, e.g. tsmSet( c, TSM_JPEG_QUALITY, 50 ) because
 *   TSM_JPEG_QUALITY is expecting an int.
 *
 *   The following settings can be modified for a connection. The type of the
 *   third parameter specifying the value is given in square parentheses.
 *
 *   \begin{itemize}
 *     \item {\bf TSM_DONT_READ_TILES}: A value of TRUE means that the 
 *           tsmReadTile and
 *           tsmReadNextTile functions will not make any disk/network
 *           access (and hence not read the tile data). This allows programs
 *           to test all overhead and computation without doing any I/O.
 *           (default = FALSE) [int]
 *     \item {\bf TSM_JPEG_QUALITY}: A value between 5 and 100 (inclusive) that
 *           specifies the JPEG quality percentage used when compressing
 *           tiles to JPEG, i.e. 100% means highest quality but largest
 *           file size.
 *           (default = 75) [int]
 *     \item {\bf TSM_PROGRESS_METER}: TRUE means that you want to have tsmApi
 *           display a progress meter for operations that take a potentially
 *           long time, e.g. tsmCopyLevelTiles, tsmCopyAllTiles,
 *           tsmCreateLevelTiles, tsmCreateAllTiles, etc. Use a value of FALSE
 *           to keep these routines silent.
 *           (default = TRUE) [int]
 *     \item {\bf TSM_METADATA_URL}: This token requires a string that  
 *           contains a URL for a tspec file to be used for a DPSS dataset.
 *           Doing a tsmSet(c,TSM_METADATA_URL,"...") call *adds* the specifed
 *           URL to a list of URLs associate with the dataset. So you can
 *           make multiple calls to provide multiple URLs. Note that this
 *           setting is only used when writing a new DPSS dataset.
 *           (default = <empty list>) [char *]
 *     \item {\bf TSM_FINISH_LOAD_ON_DISCONNECT}: If this value is set to TRUE
 *           then whenever you create a new DPSS dataset, that set will be
 *           marked as loaded when you do a tsmDisconnect. If you set this
 *           value to FALSE, then the set is left open and you can continue
 *           loading tiles during another session if you connect to the
 *           set specifying its set-id.
 *           (default = TRUE) [int]
 *     \item {\bf TSM_REQUEST_TIMEOUT}: This will set the current timeout
 *           (secs) that is used between sending a request list to a DPSS.
 *           That is, a value of 2.0 means that a request list will be sent
 *           to the DPSS only once every 2 seconds, no matter how often you
 *           call tsmTileReqEnd. If you specify a timeout < 0, then the value
 *           will be clamped to 0.0. This setting applies only to DPSS
 *           connections.
 *   \end{itemize}
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 * @param    setting the connection option to update, e.g. TSM_JPEG_QUALITY
 * @param    value the new value for the option, e.g. TRUE, FALSE, 70, etc.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  12 January 1998
 *
 * @see      tsmGet, TsmSetting
 *
 */

void
tsmSet( TsmConnection connect, TsmSetting setting, ... )
{
  const char *funcName = "tsmSet";
  va_list    args;
  int        value;

  if ( connect == NULL ) return;

  switch ( setting ) {

  case TSM_DONT_READ_TILES:           /* To stop reading of tile data */
    va_start( args, setting );
    connect->dont_read_tiles = ( va_arg( args, int ) != FALSE );
    va_end( args );
    break;

  case TSM_JPEG_QUALITY:              /* The JPEG compression quality value */
    va_start( args, setting );
    value = (int) va_arg( args, int );
    connect->jpeg_quality = max( min( value, 100 ), 5 );
    va_end( args );
    break;

  case TSM_PROGRESS_METER:            /* Show a progress meter for long ops? */
    va_start( args, setting );
    connect->show_progress = ( va_arg( args, int ) != FALSE );
    va_end( args );
    break;

  case TSM_METADATA_URL:
#ifdef DPSS
    if ( connect->type == TSM_DPSS ) {

      TsmConnectionDpss *dpsscon = (TsmConnectionDpss *) connect;
      char              **new_metadata;
      int               new_size;

      new_size     = dpsscon->numMetadata + 1;
      new_metadata = (char **) tsmMemRealloc( dpsscon->metadata,
					      sizeof(char*) * new_size );

      if ( new_metadata == NULL ) {
	tsmError( funcName, "Couldn't grow array! Resetting to empty" );
	dpsscon->numMetadata = 0;
	break;
      }

      dpsscon->numMetadata = new_size;
      dpsscon->metadata    = new_metadata;
      va_start( args, setting );
      dpsscon->metadata[new_size-1] = tsmMemStrdup( va_arg( args, char * ) );
      va_end( args );
      
    }
#endif
    break;

  case TSM_NO_DECOMPRESS:             /* Do we want raw JPEG data returned? */
    break;

  case TSM_FINISH_LOAD_ON_DISCONNECT:  /* For writing a new set to DPSS */
#ifdef DPSS
    va_start( args, setting );
    if ( connect->type == TSM_DPSS )
      ((TsmConnectionDpss *) connect)->close_on_disconnect = 
         ( va_arg( args, int ) != FALSE );
    va_end( args );
#endif
    break;

  case TSM_REQUEST_TIMEOUT:         /* timeout for sending requests to DPSS */
#ifdef DPSS
    va_start( args, setting );
    if ( connect->type == TSM_DPSS ) {
      TsmConnectionDpss *dpss_con = (TsmConnectionDpss *) connect;
      float value = (float) va_arg( args, double );
      dpss_con->requestTimeout = ( value < 0 ) ? 0.0 : value;
      TSMTRACE ((TSMMASK|1, funcName, "%p->requestTimeout = %f (%f)",
		 dpss_con, dpss_con->requestTimeout, value));
    }
    va_end( args );
#endif
    break;

  case TSM_CONNECT_PROTOCOL:
  case TSM_DATASET_NAME:
  case TSM_DATASET_URL:
  case TSM_DATASET_TSPEC:
  case TSM_IMAGE_FORMAT:
  case TSM_TILE_FORMAT:
  case TSM_READWRITE_MODE:
  case TSM_DATASET_TYPE:
  case TSM_IMAGE_FORMAT_STRING:
  case TSM_TILE_FORMAT_STRING:
  case TSM_READWRITE_MODE_STRING:
  case TSM_DATASET_TYPE_STRING:
  case TSM_TILE_SIZE:
  case TSM_DPSS_SETID:
    tsmError( funcName, "Can't change setting %d: read only", (int) setting );
    break;

  default:
    tsmError( funcName, "unknown connection setting (%d)", (int) setting );
  }

}

/*
 * Synopsis:
 *   int tsmGet( TsmConnection connect, TsmSetting setting )
 *
 * Doc++ Description: *//** 
 *   Query a connection variable.
 *
 *   The tsmGet function lets you query various characteristics of a
 *   connection; a number of which may be changed with the associated tsmSet
 *   function. The setting value can be any of the following. In each case
 *   we state whether a settings can only be read (i.e. returned by tsmGet),
 *   or if it can also be changed (i.e. using tsmSet).
 *
 *   N.B. this function always returns an int. We note where this should be
 *   cast to another type, e.g.
 *        name = (char *) tsmGet( connect, TSM_DATASET_NAME );
 *
 *   \begin{itemize}
 *     \item {\bf TSM_CONNECT_PROTOCOL}: The connection protocol being used:
 *           one of TSM_DPSS, TSM_LOCAL, or TSM_HTTP. You should cast this
 *           to a (TsmConnectionType). (READ ONLY)
 *     \item {\bf TSM_DATASET_NAME}: The name of the dataset that was connected
 *           to using the tsmConnect function call. You should cast this
 *           to a (char *). (READ ONLY)
 *     \item {\bf TSM_DATASET_URL}: The full URL for the dataset that was
 *           connected to using the tsmConnect function call. You should cast
 *           this to a (char *). (READ ONLY)
 *     \item {\bf TSM_DATASET_TSPEC}: The Pyramid Tspec structure for the
 *           connection. You should cast this to a (Pyramid *). (READ ONLY)
 *     \item {\bf TSM_TSPEC_URL}: The location where the dataset's tspec
 *           file was loaded from, e.g. for DPSS sets this could be from
 *           the DPSS itself or from some metadata URL. (READ ONLY)
 *     \item {\bf TSM_IMAGE_FORMAT}: One of TSM_LRBT_FORMAT, 
 *           TSM_JPEG_FORMAT, or TSM_LZW_FORMAT. (READ ONLY)
 *     \item {\bf TSM_IMAGE_FORMAT_STRING}: One of "lrbt-raster", 
 *           "jpeg-compressed", or "lzw-compressed". You should cast this
 *           value to a (char *). (READ ONLY)
 *     \item {\bf TSM_TILE_FORMAT}: One of TSM_NONE_TILES, TSM_RAW_TILES,
 *           TSM_TIFF_TILES, or TSM_DPSS_TILES. The raw and tiff formats are
 *           used for local disk tiles, while the dpss format means that the
 *           tile data are stored on a DPSS. You should cast this to a
 *           (TsmTileFormat) value.  (READ ONLY)
 *     \item {\bf TSM_TILE_FORMAT_STRING}: One of "none", "raw-tiles",
 *           "blocked-tiff", "dpss". You should cast this to a
 *           (char *) value.  (READ ONLY)
 *     \item {\bf TSM_DATASET_TYPE}: One of TSM_DEM_DATASET, TSM_IMAGE_DATASET,
 *           TSM_VRML_DATASET, or TSM_UNKNOWN_DATASET. You should case this
 *           to a (TsmDataSetType). (READ ONLY)
 *     \item {\bf TSM_DATASET_TYPE_STRING}: One of "dem", "image",
 *           "vrml", or "unknown". You should case this
 *           to a (char *). (READ ONLY)
 *     \item {\bf TSM_READWRITE_MODE}: One of TSM_READ_ONLY, TSM_WRITE_ONLY, or
 *           TSM_READ_WRITE, describing the connection mode. This should be
 *           case to a (TsmAccessMode). (READ ONLY)
 *     \item {\bf TSM_READWRITE_MODE_STRING}: One of "read-only", "write-only",
 *           or "read/write" describing the connection mode. This should be
 *           case to a (char *). (READ ONLY)
 *     \item {\bf TSM_TILE_SIZE}: The size of a single tile for the connection,
 *           given in bytes. (READ ONLY)
 *     \item {\bf TSM_DPSS_SETID}: Returns the set ID for a dataset on a DPSS.
 *           If you ask this value for a non DPSS set, you will get a value
 *           of -1. (READ ONLY)
 *     \item {\bf TSM_DPSS_SERVERS}: Returns a string containing a list of
 *           whitespace-separated server names for a DPSS dataset. Returns
 *           an empty string if dataset is not on a DPSS. (READ ONLY)
 *     \item {\bf TSM_DONT_READ_TILES}: A value of TRUE means that the
 *           tsmReadTile
 *           and tsmReadNextTile functions will not make any disk/network
 *           access (and hence not read the tile data). This allows programs
 *           to test all overhead and computation without doing any I/O.
 *           (default = FALSE, READ/WRITE)
 *     \item {\bf TSM_JPEG_QUALITY}: A value between 5 and 100 (inclusive) that
 *           specifies the JPEG quality percentage used when compressing
 *           tiles to JPEG, i.e. 100% means highest quality but largest
 *           file size.
 *           (default = 75, READ/WRITE)
 *     \item {\bf TSM_PROGRESS_METER}: TRUE means that you want to have tsmApi
 *           display a progress meter for operations that take a potentially
 *           long time, e.g. tsmCopyLevelTiles, tsmCopyAllTiles,
 *           tsmCreateLevelTiles, tsmCreateAllTiles, etc. Use a value of FALSE
 *           to keep these routines silent.
 *           (default = TRUE, READ/WRITE)
 *     \item {\bf TSM_FINISH_LOAD_ON_DISCONNECT}: If this value is TRUE then
 *           whenever you create a new DPSS dataset, that set will be marked
 *           as loaded when you do a tsmDisconnect. If this value is FALSE,
 *           then the set is left open when you disconnect and you can continue
 *           loading tiles during another session if you connect to the
 *           set specifying its set-id.
 *           (default = TRUE, READ/WRITE)
 *     \item {\bf TSM_REQUEST_TIMEOUT}: This will return the current timeout
 *           (secs) being used between sending a request list to a DPSS.
 *           This setting applies only to DPSS connections - any other
 *           types of connections will simply return zero. You must cast
 *           this result to (float).
 *   \end{itemize}
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 * @param    setting the connection option being queried, e.g. TSM_JPEG_QUALITY
 *
 * @return   the value of the specified setting (cast to int if other type)
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmSet, TsmSetting
 *
 */

int
tsmGet( TsmConnection connect, TsmSetting setting )
{
  const  char *funcName = "tsmGet";
  static char servers[256];

  if ( connect == NULL ) return 0;

  switch ( setting ) {
  case TSM_CONNECT_PROTOCOL:       /* The connection protocol type */
    return (int) connect->type;

  case TSM_DATASET_NAME:              /* The name of the dataset */
    return (int) connect->name;

  case TSM_DATASET_URL:               /* The full URL for the dataset */
    return (int) connect->location->url;

  case TSM_DATASET_TSPEC:             /* The pyramid's tspec structure */
    return (int) connect->tspec;

  case TSM_IMAGE_FORMAT:              /* The image format for the dataset */
    return connect->format;

  case TSM_TILE_FORMAT:               /* The file format for the tiles */
    return (int) connect->tile_format;

  case TSM_DATASET_TYPE:              /* Dataset type, e.g. DEM, OI, etc. */
    return connect->dataset_type;

  case TSM_READWRITE_MODE:            /* The Read/Write mode for connection */
    return (int) connect->mode;

  case TSM_TILE_SIZE:                 /* The size of a tile (bytes) */
    return connect->tileBufferSize;

  case TSM_TSPEC_URL:                 /* The location of the tspec file */
    return (int) connect->tspec_url;

  case TSM_DPSS_SETID:                /* The set id of a dataset on a DPSS */
#ifdef DPSS
    if ( connect->type == TSM_DPSS ) 
      return ((TsmConnectionDpss *) connect)->set_id;
    else
#endif
      return -1;

  case TSM_REQUEST_TIMEOUT:           /* time between request lists for DPSS */
#ifdef DPSS
    if ( connect->type == TSM_DPSS )
      return ((TsmConnectionDpss *) connect)->requestTimeout;
    else
#endif
      return (int) 0.0;

  case TSM_DPSS_SERVERS:              /* A string with all DPSS servers */
    servers[0] = '\0';
    if ( connect->type != TSM_DPSS ) return (int) servers;

#ifdef DPSS
    {
      int n, num;
      num = ((TsmConnectionDpss *) connect)->numServers;
      for ( n = 0; n < num; ++n ) {
	strcat( servers, ((TsmConnectionDpss *) connect)->server[n] );
	if ( n + 1 < num ) strcat( servers, " " );
      }
    }
    return (int) servers;
#endif

  case TSM_DONT_READ_TILES:           /* To stop reading of tile data */
    return connect->dont_read_tiles;

  case TSM_JPEG_QUALITY:              /* The JPEG compression quality value */
    return connect->jpeg_quality;

  case TSM_PROGRESS_METER:            /* Do we show progress meters? */
    return connect->show_progress;

  case TSM_NO_DECOMPRESS:             /* Do we want raw JPEG data returned? */
    return 0;

  case TSM_FINISH_LOAD_ON_DISCONNECT:  /* For writing a new set to DPSS */
#ifdef DPSS
    if ( connect->type == TSM_DPSS ) 
      return ((TsmConnectionDpss *) connect)->close_on_disconnect;
    else
#endif
      return TRUE;

  case TSM_IMAGE_FORMAT_STRING:       /* The image format for the dataset */
    switch ( connect->format ) {
    case TSM_LRBT_FORMAT:
      return (int) "lrbt-raster";
    case TSM_JPEG_FORMAT:
      return (int) "jpeg-compressed";
    case TSM_LZW_FORMAT:
      return (int) "lzw-compressed";
    default:
      return (int) "unknown";
    }

  case TSM_TILE_FORMAT_STRING:        /* The file format for the tiles */
    switch ( connect->tile_format ) {
    case TSM_RAW_TILES:
      return (int) "raw-tiles";
    case TSM_TIFF_TILES:
      return (int) "blocked-tiff";
    case TSM_DPSS_TILES:
      return (int) "dpss";
    default:
      return (int) "none";
    }

  case TSM_DATASET_TYPE_STRING:       /* Dataset type, e.g. DEM, OI, etc. */
    switch ( connect->dataset_type ) {
    case TSM_DEM_DATASET:
      return (int) "dem";
    case TSM_IMAGE_DATASET:
      return (int) "image";
    case TSM_VRML_DATASET:
      return (int) "vrml";
    default:
      return (int) "unknown";
    }

  case TSM_READWRITE_MODE_STRING:     /* The Read/Write mode for connection */
    switch ( connect->mode ) {
    case TSM_READ_ONLY:
      return (int) "read-only";
    case TSM_WRITE_ONLY:
      return (int) "write-only";
    case TSM_READ_WRITE:
      return (int) "read/write";
    default:
      return (int) "unknown";
    }

  default:
    tsmError( funcName, "unknown connection setting (%d)", (int) setting );
    return 0;
  }
}

/*
 * Synopsis:
 *   TsmConnectionType tsmConnectionType (TsmConnection connect)
 *
 * Doc++ Description: *//** 
 *   Return the type of a connection.
 *
 * @param    connect the connection handle 
 *
 * @return   Nothing (void)
 *
 * @author   Lee Iverson
 * @version  July 23, 1999
 *
 * @see      TsmConnection
 *
 */

TsmConnectionType
tsmConnectionType (TsmConnection connect)
{
  return connect->type;
}

/*
 * Synopsis:
 *   void tsmDescribeConnection( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Describe a connection and its dataset.
 *
 *   This function will take an open connection handle, as returned by
 *   tsmConnect, and will display information about the dataset that
 *   is connected to. For example, this will include the name of the
 *   dataset and the access mode, the size of the dataset (if this is
 *   known), the connection protocol, the image format, etc. Also,
 *   if the connection is for a DPSS dataset then information such as
 *   the set ID and the server combination will be output.
 *
 *   All output is sent to stdout, with each line indented by 2 spaces.
 *
 * @param    connect the connection handle to be described
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  14 March 1998
 *
 * @see      tsmConnect
 *
 */

void
tsmDescribeConnection( TsmConnection connect )
{
  int               minlev, maxlev, *pixels;
  float             minres, maxres;

  if ( connect == NULL ) return;

  /* Start off with the name and access mode */

  printf( "  Name    = %s\n",
	  (char *) tsmGet( connect, TSM_DATASET_NAME ) );
  printf( "  Mode    = %s\n",
	  (char *) tsmGet( connect, TSM_READWRITE_MODE_STRING ) );

  /* If we have a valid tspec structure, then query some of its fields */

  if ( connect->tspec != NULL ) {
    minlev = connect->tspec->minLevel;
    maxlev = connect->tspec->maxLevel;
    minres = connect->tspec->minResolution;
    maxres = connect->tspec->maxResolution;

    printf( "  Min Lev = %d (%1.1f)\n", minlev, minres );
    printf( "  Max Lev = %d (%1.1f)\n", maxlev, maxres );

    pixels = connect->tspec->tspecLevel.val[minlev].numValidPixel;
    printf( "  Pixels  = %d x %d\n", pixels[0], pixels[1] );

    printf( "  Tiles   = %s\n", 
	    (char*) tsmGet( connect, TSM_TILE_FORMAT_STRING ) );
  } else
    printf( "  Tspec   = no data loaded\n" );

  /* Give us some info on the type and image format of the dataset */

  printf( "  Type    = %s\n",
	  (char*) tsmGet( connect, TSM_DATASET_TYPE_STRING ) );
  printf( "  Format  = %s\n",
	  (char*) tsmGet( connect, TSM_IMAGE_FORMAT_STRING ) );

  /* And if we have a DPSS dataset, then we have some DPSS-specific info */

#ifdef DPSS
  {
    TsmConnectionDpss *dpsscon = (TsmConnectionDpss *) connect;
    int n;

    if ( connect->type == TSM_DPSS ) {
      printf( "  Set ID  = %d\n", dpsscon->set_id );
      printf( "  Servers =" );
      for ( n = 0; n < dpsscon->numServers; ++n )
	printf( " %s", dpsscon->server[n] );
      printf( "\n" );
      if ( connect->tspec_url )
	printf( "  Tspec   = %s\n", connect->tspec_url );
    }
  }
#endif

}

/*
 * Synopsis:
 *   void tsmProgressReport( TsmConnection connect, const char *fmt, ... )
 *
 * Description:
 *  This function is used to display a progress report on the 
 *  terminal that gets overwritten by the next progress report
 *  (or any other output sent to the terminal). It is designed
 *  to work in a similar way to printf. Output is only generated
 *  if the TSM_PROGRESS_METER setting is TRUE, or if the connect
 *  parameter is NULL.
 *
 * Author: Martin Reddy
 *
 * Date: 22 March 1998
 *
 */

void
tsmProgressReport( TsmConnection connect, const char *fmt, ... )
{
  va_list ap;

  if ( connect != NULL && connect->show_progress == FALSE ) 
    return;

  printf( "                                                           \r" );

  va_start( ap, fmt );
  vprintf( fmt, ap );
  va_end( ap );

  printf( "\r" );
  fflush( stdout );
}

void
tsmDisconnect (TsmConnection conn)
{
  conn->vtable->disconnect (conn);
}

int
tsmAddRequest (TsmConnection conn, tsmReqElem *req)
{
  return conn->vtable->addRequest (conn, req);
}

int
tsmRemoveRequest (TsmConnection conn, tsmReqElem *req)
{
  return conn->vtable->removeRequest (conn, req);
}

int
tsmReprioritizeRequest (TsmConnection conn, tsmReqElem *req)
{
  return conn->vtable->reprioritizeRequest (conn, req);
}

int
tsmBeginLogging (TsmConnection conn, 
		  int method, const char *destination )
{
  if (conn->vtable->beginLogging)
    return conn->vtable->beginLogging (conn, method, destination);
  else
    return FALSE;
}

int
tsmEndLogging (TsmConnection conn)
{
  if (conn->vtable->endLogging)
    return conn->vtable->endLogging (conn);
  else
    return FALSE;
}
