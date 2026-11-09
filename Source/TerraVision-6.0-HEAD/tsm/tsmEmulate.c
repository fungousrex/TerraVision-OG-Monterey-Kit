/*
 * Name: tsmEmulate.c
 *
 * Description: 
 *   Contains routines to emulate the old server-centric tsmApi interface
 *   in terms of the new dataset-centric (connection) interface. 
 *
 * Function List:
 *
 * Revision History:
 *   Created 9 Feb 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmEmulate.c,v 1.16.6.1 2001/06/19 22:04:41 leei Exp $
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

/*
 * Synopsis:
 *      TsmHandle *tsmOpen( const char *dbUrl )
 *
 * Doc++ Description: *//** 
 *   Establishes a connection to the Tile Set Manager.
 *
 * tsmOpen opens a TSM session handle and must be the first call to the
 * Tile Set Manager.
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s).
 * 
 * The dbUrl is the "base" location of the Pyramid
 * databases.  That is, unless over-ridden explicitly by a call to
 * tsmLoadPyramidDatabase, the Pyramid database is expected to
 * be in dbUrl/Pyramid.
 * 
 * If dbUrl is NULL or an empty string then you must explicitly call
 * tsmLoadPyramidDatabase with a fully specified database URL in
 * order to use names in the calls to tsmGet...Pyramid.  (This lets
 * you use different locations for the Pyramid databases).
 * If you don't explicitly call the tsmLoad...Database functions, then
 * you will only be able to access tile sets by URL.
 *  
 * Upon success, tsmOpen returns a "Tile Set Manager Session Handle".
 * All further calls to the Manager require this handle.
 *  
 * NOTES:
 * 
 * o Your program may have more than one TSM session open at the same
 * time (The DSM and ISS now allow multiple seesions.)
 * 
 * o The database specified by the URL is not actually opened until a
 * call to a function requiring the database contents (like getPyramid)
 * is actually made .  In particular, the DSM is not contacted until
 * then.
 * 
 * Valid database URLs are of the form:
 *  
 * \begin{itemize}
 * \item file://localhost/path/a/b/  
 *   where /path/a/b is the local directory containing the 
 *   pyramidUrls file
 *  
 *   Paths beginning with "~", like "~foobar" are translated to
 *   "/homedir/foobar/public_html".
 * \item x-dsm://hostname/
 * \item http:://hostname/path/a/b/
 * \end{itemize}
 *
 * @param    dbUrl  the base URL for the TSM database to open
 *
 * @return   a TSM session handle, or NULL if failed
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmClose, tsmConnect
 *
 */

TsmHandle *
tsmOpen( const char *dbUrl )
{
  static const char *funcName = "tsmOpen";
  TsmHandle         *tsmh;
  Purl              *dbPurl;

  if ( dbUrl == NULL ) dbUrl = "";

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|1, funcName, "Initialize database URL '%s'", dbUrl ));

  /* First we will try to allocate a new TsmHandle structure */

  if ( ! ( tsmh = (TsmHandle *) tsmMemCalloc( 1, sizeof(TsmHandle) ) ) ) {
    tsmError( funcName, "Unable to allocate tsmh session handle" );
    TSMTRINC(( TSMMASK|1, -1 ));
    return NULL;
  }

  /* Now we will parse the TSM URL, if one has been given */

  if ( strlen(dbUrl) > 0 ) {
    if ( ( dbPurl = tsmParseUrl( dbUrl ) ) == NULL ) {
      tsmError( funcName, "Invalid base URL '%s'", dbUrl );
      tsmMemManagerFree( tsmh );
      TSMTRINC(( TSMMASK|1, -1 ));
      return NULL;
    }
  } else { /* If the dbUrl is empty, set the protocol to "no protocol". */
    if ( ( dbPurl = tsmMakePurl() ) == NULL ) {
      tsmError( funcName, "Unable to make a Purl." );
      TSMTRINC(( TSMMASK|1, -1 ));
      return NULL;
    }

    dbPurl->originalUrl    = tsmMemStrdup( "" );
    dbPurl->url            = tsmMemStrdup( "" );
    dbPurl->uri            = tsmMemStrdup( "" );
    dbPurl->hostName       = tsmMemStrdup( "" );
    dbPurl->port           = tsmMemStrdup( "" );
    dbPurl->protocolString = tsmMemStrdup( "" );
    dbPurl->protocol       = tsmNoProtocol;
  }

  /* Finally, we set up the initial TsmHandle fields and return the struct */

  tsmh->dbPurl      = dbPurl;
  tsmh->connect     = NULL;
  tsmh->numConnects = 0;

  TSMTRACE(( TSMMASK|1, funcName, "Returning tsmh %p", tsmh ));
  TSMTRINC(( TSMMASK|1, -1 ));

  return tsmh;
}

/*
 * Synopsis:
 *      TsmHandle *tsmClose( TsmHandle *tsmh )
 *
 * Doc++ Description: *//** 
 *   Closes the connection to the Tile Set Manager.
 *
 *   tsmClose finishes the current TSM session (as opened with tsmOpen)
 *   and frees all associated data structures.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s).
 *
 * @param    tsmh the TSM session handle returned by tsmOpen
 *
 * @return   NULL
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmOpen, tsmConnect
 *
 */

TsmHandle *
tsmClose( TsmHandle *tsmh )
{
  static const char *funcName = "tsmClose";
  int               i;

  if ( tsmh == NULL ) return NULL;

  TSMTRINC (( TSMMASK|1, 1 ));

  if ( tsmh->dbPurl != NULL )
    TSMTRACE(( TSMMASK|1, funcName, "Closing tsm '%s'... ", tsmh->dbPurl->url ));

  /* Do a tsmDisconnect on all of the open connections and free the array */

  if ( tsmh->numConnects > 0 ) {
    for ( i = 0; i < tsmh->numConnects; ++i )
      tsmDisconnect( tsmh->connect[i] );
    tsmMemManagerFree( tsmh->connect );
  }

  if ( tsmh->dbPurl != NULL ) 
    if ( tsmh->dbPurl->protocol == tsmDSM || 
	 tsmh->dbPurl->protocol == tsmISS || 
	 tsmh->dbPurl->protocol == tsmDPSS )
      tsmClose_dsm( tsmh );

  /* Freeing these data structures causes a core dump in TerraVision. */

  if ( tsmh->masterHostName != NULL) tsmMemFree( tsmh->masterHostName );

  tsmFreePurl( tsmh->dbPurl );
  tsmFreePurl( tsmh->gdbPurl );
  tsmFreePurl( tsmh->pdbPurl );
  tsmFreeNameUrlIds( tsmh->pyramidUrls );

  /* Free the memory for the TsmHandle itself and then exit */

  tsmMemManagerFree( tsmh );

  TSMTRACE(( TSMMASK|1, funcName, "done." ));
  TSMTRINC(( TSMMASK|1, -1 ));
  return NULL;
}

/*
 * Synopsis:
 *   int tsmAddConnectionToTsmHandle( TsmHandle *tsmh, TsmConnection connect )
 *
 * Description:
 *   Takes a connection handle and adds this to the connection array
 *   for a TsmHandle.
 *
 * Returns: TRUE if all went well, FALSE if an error occurred
 *
 * Author: Martin Reddy
 *
 * Date: 9 February 1998
 *
 */

int
tsmAddConnectionToTsmHandle( TsmHandle *tsmh, TsmConnection connect )
{
  static const char *funcName = "tsmAddConnectionToTsmHandle";
  TsmConnection     *new_array;
  int               new_size;

  if ( tsmh == NULL || connect == NULL ) return FALSE;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "tsmh = %p, connect = %p", tsmh, connect ));

  /* Try to expand the connection array by one entry */

  new_size  = tsmh->numConnects + 1;
  new_array = (TsmConnection *)
    tsmMemRealloc( tsmh->connect, sizeof(TsmConnection) * new_size );

  if ( new_array == NULL ) {
    tsmError( funcName, "Couldn't expand connection array to %d", new_size );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* Cooool! It worked. So add the new connection pointer to the array */

  tsmh->numConnects = new_size--;
  tsmh->connect     = new_array;
  tsmh->connect[new_size] = connect;

  TSMTRACE(( TSMMASK|2, funcName, "Added %s as connect %d", connect->name, new_size ));

  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *   Pyramid *tsmGetConnectionFromTsmHandle(TsmHandle *tsmh, Pyramid *pyramid)
 *
 * Description:
 *   Looks up a connection in a TsmHandle's connection array. We look up
 *   the connection based up the Pyramid pointer for that dataset.
 *
 * Returns: A TsmConnection structure, or NULL if failed
 *
 * Author: Martin Reddy
 *
 * Date: 6 February 1998
 *
 */

TsmConnection
tsmGetConnectionFromTsmHandle( TsmHandle *tsmh, Pyramid *pyramid )
{
  int i;

  if ( tsmh == NULL || pyramid == NULL ) return NULL;

  for ( i = 0; i < tsmh->numConnects; ++i ) 
    if ( tsmh->connect[i]->tspec == pyramid )
      return tsmh->connect[i];

  return NULL;
}

/*
 * Synopsis:
 *    int tsmGetConnectionsFromTsmHandle( TsmHandle *tsmh,
 *            TsmConnection **connects, int &numConnects )
 *
 * Description:
 *    Lets you get at the TsmConnection array within the emulated
 *    TsmHandle structure.
 *
 * Returns: TRUE if valid results returned through params, FALSE otherwise
 *
 * Author: Martin Reddy
 *
 * Date: 26 May 1998
 *
 */

int
tsmGetConnectionsFromTsmHandle( TsmHandle *tsmh, TsmConnection **connects,
				int *numConnects )
{
  if ( tsmh == NULL ) return FALSE;

  *connects = tsmh->connect;
  *numConnects = tsmh->numConnects;
  return TRUE;
}

/*
 * Synopsis:
 *   void tsmSetMasterHostName (TsmHandle *tsmh, const char *masterHostName)
 *
 * Doc++ Description: *//** 
 *   Change the master host name for a TSM session.
 *      
 *  tsmSetMasterHostName should be called immediately after tsmOpen.
 *  It changes the master host name from the default (which is the
 *  hostname of the first pyramid it finds).  This has no effect except
 *  for the ISS protocol.  In that case, changing the master host name
 *  after the ISS has been contacted has no effect.
 *
 * @param    tsmh  the TSM session handle
 * @param    masterHostName the new master host name
 *
 * @return   Nothing (void)
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmOpen
 *
 */

void 
tsmSetMasterHostName( TsmHandle *tsmh, const char *masterHostName )
{
  static const char *funcName = "tsmSetMasterHostName";

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "tsmh = %p, host = %s", tsmh, masterHostName ));

  if ( masterHostName != NULL )
    tsmh->masterHostName = tsmMemStrdup( masterHostName );
  else
    tsmh->masterHostName = NULL;

  TSMTRINC(( TSMMASK|2, -1 ));
}

/*
 * Synopsis:
 *   Purl *tsmGetPyramidBasePurl( TsmHandle *tsmh, const char *pyramidName )
 *
 * Description:
 *
 * Returns: A Purl for the pyramid, or NULL if failed
 *
 * Author: Martin Reddy
 *
 * Date: 6 February 1998
 *
 */

Purl *
tsmGetPyramidBasePurl( TsmHandle *tsmh, const char *pyramidName )
{
  static const char *funcName = "tsmGetPyramidBasePurl";
  char dsmUrl[1024];
  Purl *basePurl;

  if ( pyramidName == NULL ) pyramidName = "";

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "tsmh = %p, pyramidName = %s", tsmh, pyramidName ));

  /* Get the PyramidUrls if necessary. */

  if ( tsmReLoadPyramidDatabase( tsmh, "" ) == FALSE ) {
    tsmError( funcName, "Couldn't load Pyramid database '%s'", pyramidName );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* If the pyramidName is just a number, then check to see if the   */
  /* database is an x-dsm URL. If so, then assume that we have a set */
  /* name on that DSM and build the appropriate, full pyramid URL    */

  if ( ( tsmh->dbPurl != NULL && ( tsmh->dbPurl->protocol == tsmDSM ||
       tsmh->dbPurl->protocol == tsmDPSS ) ) && atoi(pyramidName) > 0 ) {

    if ( tsmh->dbPurl->url[strlen(tsmh->dbPurl->url)-1] == '/' )
      sprintf( dsmUrl, "%sPyramids/%s", tsmh->dbPurl->url, pyramidName );
    else
      sprintf( dsmUrl, "%s/Pyramids/%s", tsmh->dbPurl->url, pyramidName );
    
    pyramidName = dsmUrl;
    TSMTRACE(( TSMMASK|2, funcName, "Transformed pyramidName = %s", dsmUrl ));
  }

  /* Parse the pyramidName */

  basePurl = tsmParseIdentifier( pyramidName, tsmh->pyramidUrls );
  if ( basePurl == FALSE ) {
    if ( TSMTRACEP (TSMMASK|4) )
      tsmError( funcName, "Couldn't parse URL '%s'", pyramidName );

    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  TSMTRACE(( TSMMASK|2, funcName, "basePurl = %p", basePurl ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return basePurl;
}

/*
 * Synopsis:
 *      Pyramid *tsmGetPyramid (TsmHandle *tsmh, const char *pyramidName )
 *
 * Doc++ Description: *//** 
 *  Open a connection to a pyramid.
 *      
 *  tsmGetPyramid gets a Pyramid as identified by pyramidName.
 *  The identifier can be either a name or a URL.  If the identifier
 *  can be parsed as a URL, then its assumed to be just that.
 *  Otherwise, its assumed to be a name and the corresponding URL is
 *  found in Pyramid Database.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s).
 *
 * @param    tsmh the open TSM session handle
 * @param    pyramidName location of the Pyramid
 *
 * @return   the Pyramid if successful, FALSE if an error occurred
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmPutPyramid, tsmGetPyramid, tsmOpen, tsmConnect, Pyramid
 *
 */

Pyramid *
tsmGetPyramid( TsmHandle *tsmh, const char *pyramidName )
{
  return tsmGetPyramid_params( tsmh, pyramidName, NULL );
}

Pyramid *
tsmGetPyramid_params( TsmHandle *tsmh, const char *pyramidName,
		      TsmConnectionParams params )
{
  static const char *funcName = "tsmGetPyramid";
  TsmConnection     connect;
  Purl              *purl;

  if ( tsmh == NULL ) return NULL;

  if ( pyramidName == NULL ) pyramidName = "";

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "tsmh = %p, pyramid = '%s'", tsmh, pyramidName ));

  /* If the pyramid name has a directory structure to it, i.e. it is not */
  /* just a tileset name, then we will parse it as a URL. Otherwise, we  */
  /* will have to try to match the name against those in the local dbase */

  if ( strchr( pyramidName, '/' ) != NULL )
    purl = tsmParseUrl( pyramidName );
  else
    purl = tsmGetPyramidBasePurl( tsmh, pyramidName );

  if ( purl == NULL ) {
    tsmError( funcName, "Couldn't parse pyramid name '%s'.", pyramidName );
    TSMTRINC(( TSMMASK|1, -1 ));
    return NULL;
  }

  /* vector off to the new connection call, which will automatically */
  /* read the dataset's tspec file into memory.                      */

  if ( ( connect = tsmConnect( purl->url, "r", params ) ) == NULL ) {
    tsmError( funcName, "Couldn't connect to %s\n", purl->url );
    tsmFreePurl( purl );
    TSMTRINC(( TSMMASK|1, -1 ));
    return NULL;
  }

  /* Clean up and add this connection to the tsm handle's list */

  tsmFreePurl( purl );
  tsmAddConnectionToTsmHandle( tsmh, connect );

  TSMTRACE(( TSMMASK|2, funcName, "Done. pyramid = %p", connect->tspec ));
  TSMTRINC(( TSMMASK|1, -1 ));
  return connect->tspec;
}

/*
 * Synopsis:
 *      Pyramid *tsmMakePyramid( void )
 *
 * Doc++ Description: *//** 
 *      Allocate a pyramid.
 *      
 *      Allocate memory for a Pyramid tspec structure, initialised with all 
 *      fields set to 0/NULL.
 *
 * @return   Pointer to allocated Pyramid structure, or NULL if failed.
 *
 * @author   Yvan Leclerc
 * @version  29 July 1997
 *
 * @see      tsmFreePyramid, Pyramid
 *
 */

Pyramid *
tsmMakePyramid( void )
{
  return tsmAllocTspec();
}


/*
 * Synopsis:
 *      Pyramid *tsmFreePyramid( Pyramid *pyramid )
 *
 * Doc++ Description: *//** 
 *   Previously used to free a pyramid structure.
 *      
 *   This function used to be used by the old tsmApi to frees up all
 *   memory associated with a pyramid. However, this is a dangerous
 *   thing for you to do in the new tsmApi because tspec allocation/
 *   freeing is done for you by tsmConnect/tsmDisconnect. This function
 *   therefore no longer does anything - if you use tsmGetPyramid, then
 *   the returned Pyramid structure will get freed for you by tsmClose
 *   (when it calls tsmDisconnect).
 *
 *   If you really do require the ability to free a Pyramid structure -
 *   i.e. if you allocated it yourself with tsmMakePyramid - then you
 *   should use the tsmFreeTspec function.
 *
 * @param    pyramid the dataset's tspec structure 
 *
 * @return   NULL
 *
 * @author   Martin Reddy
 * @version  13 March 1998
 *
 * @see      tsmMakePyramid, Pyramid, tsmFreeTspec
 *
 */

Pyramid *
tsmFreePyramid( Pyramid *pyramid )
{
  return NULL;
}

/*
 * Synopsis:
 *    void tsmPutPyramid( TsmHandle *tsmh, Pyramid *pyramid, const char * )
 *
 * Doc++ Description: *//** 
 *    Write a pyramid's tspec information.
 *
 *   tsmPutPyramid writes pyramid tspec to the location specified by baseUrl.
 *   The new single-file tspec format will be used to write the data.
 *   The baseUrl should be NULL, "", "stdout", or the URL of the pyramid.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s).
 *
 * @param    tsmh the TSM session handle
 * @param    pyramid the Pyramid tspec structure to write out
 * @param    baseUrl the URL of the pyramid to write to, or "stdout"
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      tsmGetPyramid, Pyramid
 *
 */

void
tsmPutPyramid( TsmHandle *tsmh, Pyramid *pyramid, const char *baseUrl )
{
  static const char *funcName = "tsmPutPyramid()";
  TsmConnection     connect;
  Purl              *purl;

  if ( tsmh == NULL || pyramid == NULL ) return;

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return;
  }

  /* Are we to write out to stdout? If not, then the baseUrl must */
  /* be the same as the baseUrl for the pyramid's connection.     */

  if ( baseUrl != NULL ) {
    if ( strcmp( baseUrl, "stdout" ) == 0 ) {
      tsmPrintTspec( connect );
      return;
    }

    if ( ( purl = tsmParseUrl( baseUrl ) ) == NULL ) {
      tsmError( funcName, "Couldn't parse '%s'", baseUrl );
      return;
    }

    if ( strcmp( purl->url, connect->location->url ) != 0 ) {
      tsmError( funcName, "Can only write to pyramid URL '%s'",
		connect->location->url );
      tsmFreePurl( purl );
      return;
    }

    tsmFreePurl( purl );
  }

  /* Okay, all went fine, so let's write out this tspec */
  /* structure to the URL for the connection            */
  
  if ( pyramid != connect->tspec )
    tsmSetTspecField( connect, pyramid );

  tsmWriteTspec( connect );
}

/*
 * Synopsis:
 *      int tsmGetTile (TsmHandle *tsmh, Pyramid *pyramid, int ix, int iy,
 *                      int level, unsigned char *tileBuffer, ... )
 *
 * Doc++ Description: *//** 
 *      Gets a single tile from a Pyramid.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmReadTile.
 *
 *      The tsmGetTile function retrieves a single tile from Pyramid 
 *      pyramid at tile index (ix,iy, level). The tileBuffer is assumed
 *      to be sufficiently large to hold the entire tile.
 *      This is NOT recommended when accessing tiles from the DPSS.
 *      Instead, use the request list mechanism.
 *
 * @param    tsmh       the TSM session handle returned from tsmOpen.
 * @param    pyramid    the pyramid structure, e.g. as returned from
 *                      tsmGetPyramid.
 * @param    ix         the x-coordinate of the tile to retrieve.
 * @param    iy         the y-ccordinate of the tile to retrieve.
 * @param    level      the level of the pyramid to retrieve from.
 * @param    tileBuffer a buffer to write the tile data into.
 * @param    numServer  ignored
 *
 * @return   TRUE if successful, FALSE if error occurred.
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmConnect, tsmReadTile, tsmPutTile
 *
 */

int
tsmGetTile( TsmHandle *tsmh, Pyramid *pyramid, int ix, int iy, int level, 
	    unsigned char *tileBuffer, ... )
{
  static const char *funcName = "tsmGetTile";
  TsmConnection     connect;

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return FALSE;
  }

  /* vector off to the new connection-based equivalent of tsmGetTile */
  /* Return a value of TRUE or FALSE. TSM_NODATA = FALSE for old Api */

  return ( tsmReadTile( connect, ix, iy, level, tileBuffer ) == TRUE );
}

/*
 * Synopsis:
 *      int tsmPutTile(TsmHandle *, Pyramid *, int, int, int, 
 *                     const unsigned char *, ... )
 *
 * Doc++ Description: *//** 
 *	Writes a single tile to a Pyramid.
 *      The tsmPutTile function will write a single tile (ix,iy,level) to 
 *      the Pyramid, pyramid. 
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmWriteTile.
 *
 *      The function supports one optional parameter to specify the
 *      image format to use (see below).
 *
 *      N.B. when writing tiles to a TIFF file on local disk, the order
 *      in which you write the data is important. The tiles should be
 *      written in row-major, left-to-right order, e.g. p0p0, p1p0, p2p0,...
 *      p0p1, p1p1, p2p1... etc. The TIFF file will be created automatically
 *      when then first tile is output, and it will be closed automatically
 *      when then last tile is written.
 *
 * @param    tsmh       the TSM session handle returned from tsmOpen.
 * @param    pyramid    the pyramid structure, e.g. as returned from
 *                      tsmGetPyramid.
 * @param    ix         the x-coordinate of the tile to retrieve.
 * @param    iy         the y-ccordinate of the tile to retrieve.
 * @param    level      the level of the pyramid to retrieve from.
 * @param    tileBuffer the buffer of data to write to the pyramid
 * @param    imgFormat  (optional) the image format to write the data in, 
 *                      e.g. tspLrbtRaster (default) or tspJPEGCompressed.
 *
 * @return   TRUE if tile written okay, FALSE if the operation failed
 *
 * @author   Yvan Leclerc
 * @version  updated by Martin Reddy, 10 July 1997
 *
 * @see      tsmGetTile, tsmSetUseNewTileFlag, tsmSetJPEGQuality, tsmOpen
 *
 */

int
tsmPutTile( TsmHandle *tsmh, Pyramid *pyramid, int ix, int iy, int level, 
	    const unsigned char *tileBuffer, tspSymbol imgFormat )
{
  static const char *funcName = "tsmPutTile";
  TsmConnection     connect;

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return FALSE;
  }

  /* vector off to the new connection-based equivalent of tsmGetTile */

  return tsmWriteTile( connect, ix, iy, level, (uchar *) tileBuffer );
}

/*
 * Synopsis:
 *      void tsmStartReqTiles (TsmHandle *tsmh)
 *
 * Doc++ Description: *//** 
 *   Start a new tile request list.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmTileReqBegin.
 *      
 *   In order to support the asynchronous receipt of terrain tile data
 *   over a network, we use a model whereby we built a request list of
 *   tile data, send that list, and then use the new tile data as it is
 *   received from the server. This model can of course be easily applied
 *   to local disk access also.
 *
 *   We build the request list by making a call to tsmStartReqTiles, then
 *   making multiple calls to tsmReqTiles to specify all of the tiles that
 *   we want to receive, then we call tsmStopReqTiles.
 *
 *   Once we have called tsmStopReqTiles then we can start reading in the
 *   tiles as they are received (trivial from local disk!) by calling a
 *   function such as tsmGetNextTile.
 *
 * @param    tsmh an open TSM session handle
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      tsmReqTile, tsmStopReqTiles, tsmGetNextTile
 *
 */

void
tsmStartReqTiles (TsmHandle *tsmh)
{ 
  static const char *funcName = "tsmStartReqTiles";
  int               i;

  /* Because we have no explicit connection information here, and */
  /* because tsmReqTile can request tiles from any pyramid (i.e.  */
  /* any connection), we apply the start request list to all of   */
  /* the connections that have been opened for this TSM session.  */
  
  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Starting request list for all connections" ));

  for ( i = 0; i < tsmh->numConnects; ++i )
    tsmTileReqBegin( tsmh->connect[i] );

  TSMTRACE(( TSMMASK|2, funcName, "Done." ));
  TSMTRINC(( TSMMASK|2, -1 ));
}

void
tsmReStartReqTiles( TsmHandle *tsmh )
{
  tsmStartReqTiles( tsmh );
}

/*
 * Synopsis:
 *      int tsmReqTile (TsmHandle *tsmh, Pyramid *pyramid, 
 *                      int ix, int iy, int level, int priority)
 *
 * Doc++ Description: *//** 
 *      Add a tile to the request list.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmTileReqAdd.
 *
 *   In order to support the asynchronous receipt of terrain tile data
 *   over a network, we use a model whereby we built a request list of
 *   tile data, send that list, and then use the new tile data as it is
 *   received from the server. This model can of course be easily applied
 *   to local disk access also.
 *
 *   We build the request list by making a call to tsmStartReqTiles, then
 *   making multiple calls to tsmReqTiles to specify all of the tiles that
 *   we want to receive, then we call tsmStopReqTiles.
 *
 *   Once we have called tsmStopReqTiles then we can start reading in the
 *   tiles as they are received (trivial from local disk!) by calling a
 *   function such as tsmGetNextTile.
 *
 * @param    tsmh an open TSM session handle
 * @param    pyramid the pyramid to request the tiles from
 * @param    ix the x-coordinate of the tile to request
 * @param    iy the y-coordinate of the tile to request
 * @param    level the level of the pyramid to request the tile from
 * @param    priority one of: TSM_NOW, TSM_NEVER, or TSM_LATER
 *
 * @return   TRUE if operation successful, FALSE if an error occurred
 *
 * @author   
 * @version  
 *
 * @see      tsmStartReqTiles, tsmStopReqTiles, tsmGetNextTile.
 *
 */

int
tsmReqTile( TsmHandle *tsmh, Pyramid *pyramid, 
	    int ix, int iy, int level, int priority )
{
  static const char *funcName = "tsmReqTile";
  TsmConnection     connect;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName, "request pyramid=%p tile=(%d,%d) lev=%d pri=%d",
	     pyramid, ix, iy, level, priority ));

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  /* vector off to the new connection-based equivalent of tsmReqTile */

  if ( tsmTileReqAdd( connect, ix, iy, level, priority ) == FALSE ) {
    tsmError( funcName, "tsmTileReqAdd failed", pyramid );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  TSMTRACE(( TSMMASK|3, funcName, "Done." ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *      int tsmStopReqTiles (TsmHandle *tsmh)
 *
 * Doc++ Description: *//** 
 *      Finish a batch of tile requests.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmTileReqEnd.
 *
 *   tsmStopReqTiles is used to tell the tsmApi that you are finished
 *   compiling a list of tiles to be requested (e.g. with tsmReqTile).
 *   The list can then be transmitted to the server, if appropriate,
 *   so that the tiles can begin to be sent.
 *
 *   Once this function has been called, you can begin to read the
 *   incoming tiles using the tsmGetNextTile function.
 *
 * @param    tsmh an open TSM session handle
 *
 * @return   TRUE if operation successful, FALSE if an error occurred
 *
 * @author   
 * @version  
 *
 * @see      tsmGetNextTile, tsmStartReqTiles, tsmReqTile
 *
 */

int
tsmStopReqTiles( TsmHandle *tsmh )
{
  static const char *funcName = "tsmStopReqTiles";
  int               i;

  /* Because we have no explicit connection information here, and */
  /* because tsmReqTile can request tiles from any pyramid (i.e.  */
  /* any connection), we apply the end request list to all of the */
  /* connections that have been opened for this TSM session.      */

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|2, funcName,
	     "Sending a stop request list to all connections" ));

  for ( i = 0; i < tsmh->numConnects; ++i )
    tsmTileReqEnd( tsmh->connect[i] );

  TSMTRACE(( TSMMASK|2, funcName, "Done." ));
  TSMTRINC(( TSMMASK|1, -1 ));

  return TRUE;
}
  
/*
 * Synopsis:
 *     int tsmGetNextTile( TsmHandle *tsmh, TsmTileHeader *tileHeader,
 *                         unsigned char *tileBuffer, ... )
 *
 * Doc++ Description: *//** 
 *   Retrieve the tile header and data for the next tile in the queue.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmReadNextTile.
 *
 *   Once you have requested a number of tiles using tsmReqTiles, then
 *   you can use tsmGetNextTile to retrieve the tiles as they become
 *   available. This approach is primarily for accessing
 *   tiles from the DPSS, but works for any protocol.
 *      
 *   The tile header contains various pieces of information about a tile
 *   such as its coordinates, level, etc. These can be accessed through
 *   API calls - see tsmMakeTileHeader for further details.
 *
 * @param    tsmh        the open TSM session handle
 * @param    tileHeader  a TSM tile header to be updated
 * @param    tileBuffer  a buffer to write the tile data to
 *
 * @return   TRUE if data retrieved okay, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmReqTile, tsmMakeTileHeader
 *
 */

int
tsmGetNextTile( TsmHandle *tsmh, TsmTileHeader *tileHeader,
		unsigned char *tileBuffer, ... )
{
  static const char *funcName = "tsmGetNextTile";
  TsmConnection     connect;
  int               returnCode = FALSE, i, attempts;

  /* We have no information about which connection we should try */
  /* to read the next tile from, so we simply cycle through each */
  /* of the connections that have been opened until we find one  */
  /* that will give us some data. We try to spread the load by   */
  /* reading from a random connection each time (if >1 connects) */

  i = ( tsmh->numConnects == 1 ) ? 0 : rand() % tsmh->numConnects;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Vectoring to tsmReadNextTile (connect %d)", i ));

  for ( attempts = 0; attempts < tsmh->numConnects; ++attempts ) {

    connect = tsmh->connect[i];

    /* only try to read tiles from read only or read/write connections */

    if ( connect->mode != TSM_WRITE_ONLY ) {
      returnCode = tsmReadNextTile( connect, tileHeader, tileBuffer );
      if ( returnCode == TRUE ) break;
    }

    /* if there was no tile to read here, then try the next connection */

    i = ( i + 1 ) % tsmh->numConnects;
  }

  /* Return a value of TRUE or FALSE. TSM_NODATA = FALSE for old Api */

  TSMTRACE(( TSMMASK|2, funcName, "Done. Return code = %d", returnCode ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return ( returnCode == TRUE );
}

/*
 * Synopsis:
 *     int tsmAllocGetNextTile( TsmHandle *tsmh, TsmTileHeader **tileHeader,
 *                              unsigned char **tileBuffer )
 *
 * Doc++ Description: *//** 
 *   Retrieve the tile header and data for the next tile in the queue.
 *
 * N.B. this function is part of the old style server-centric interface
 * to tsmApi. You should really start using the new leaner, cleaner,
 * and sexier connection-based interface (see tsmConnect). This function
 * is now just an emulation layer on top of the relevant 
 * connection-based function(s): tsmAllocReadNextTile.
 *
 *   This function performs the same action as tsmGetNextTile, except that
 *   it allocates the memory for the tile header and buffer inside the
 *   function. You must therefore pass pointers to the tile header/buffer
 *   pointers, so that the allocated memory addresses can be filled in for
 *   you. You free the memory that has been returned using the
 *   tsmFreeTileHeader and tsmFreeTileBuffer functions as usual.
 *
 * @param    tsmh        the open TSM session handle
 * @param    tileHeader  a pointer to a TSM tile header to be updated
 * @param    tileBuffer  a pointer to a buffer to write the tile data to
 *
 * @return   TRUE if data retrieved okay, FALSE if an error occurred,
 *           or TSM_NODATA if we have read all tiles that were requested
 *
 * @author   Martin Reddy
 * @version  5 March 1998
 *
 * @see      tsmGetNextTile, tsmFreeTileHeader, tsmFreeTileBuffer
 *
 */

int
tsmAllocGetNextTile( TsmHandle *tsmh, TsmTileHeader **tileHeader,
		     unsigned char **tileBuffer, ... )
{
  static const char *funcName = "tsmAllocGetNextTile";
  TsmConnection     connect;
  int               returnCode = FALSE, i, attempts;

  /* We have no information about which connection we should try */
  /* to read the next tile from, so we simply cycle through each */
  /* of the connections that have been opened until we find one  */
  /* that will give us some data. We try to spread the load by   */
  /* reading from a random connection each time (if >1 connects) */

  i = ( tsmh->numConnects == 1 ) ? 0 : rand() % tsmh->numConnects;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Vectoring to tsmAllocReadNextTile (con=%d)", i ));

  for ( attempts = 0; attempts < tsmh->numConnects; ++attempts ) {

    connect = tsmh->connect[i];

    /* only try to read tiles from read only or read/write connections */

    if ( connect->mode != TSM_WRITE_ONLY ) {
      returnCode = tsmAllocReadNextTile( connect, tileHeader, tileBuffer );
      if ( returnCode == TRUE ) break;
    }

    /* if there was no tile to read here, then try the next connection */

    i = ( i + 1 ) % tsmh->numConnects;
  }

  TSMTRACE(( TSMMASK|2, funcName, "Done. Return code = %d", returnCode ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return returnCode;
}

/*
 * Synopsis:
 *      unsigned int tsmNumBytesPerTile (Pyramid *pyramid)
 *
 * Doc++ Description: *//** 
 *   Returns the size of a tile.
 *      
 *  tsmNumBytesPerTile is a utility function that returns the total
 *  number of bytes in an uncompressed tile of Pyramid pyramid. If
 *  you pass a NULL pointer, then the function will return a value of 0.
 *
 * @param    pyramid the pyramid to calculate the tile size for
 *
 * @return   the size of the tile (bytes)
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmMakeTileBuffer, tsmTotalNumTiles
 *
 */

unsigned int
tsmNumBytesPerTile (Pyramid *pyramid)
{
  TileTspec *tileTspec;

  if ( pyramid == NULL ) return 0;

  tileTspec = &pyramid->tileTspec;

  return tileTspec->totalPixels[0] * tileTspec->totalPixels[1] *
         tileTspec->numComponentsPixel * tileTspec->numBytesPerComponent;
}

/*
 * Synopsis:
 *    int tsmTotalNumTiles( Pyramid *tspec, int replicate )
 *
 * Doc++ Description: *//** 
 *    Calculate total tiles in a pyramid.
 *    
 *    If replicate is FALSE, then this function calculates
 *    the total number of tiles in an entire pyramid (i.e. blocks
 *    in a set on the DPSS). If replicate is TRUE, then the
 *    value returned includes the extra tiles required for the
 *    replication mode on the DPSS.
 *
 * @param    tspec the pyramid to use for the calculation
 * @param    replicate TRUE or FALSE
 *
 * @return   the number of tiles
 *
 * @author   Martin Reddy
 * @version  22 October 1997
 *
 * @see      tsmNumBytesPerTile
 *
 */

int
tsmTotalNumTiles( Pyramid *tspec, int replicate )
{
  SimpleTileSet *tsl;
  int           min_res, max_res;
  int           i, total_tiles = 0;

  tsl     = tspec->tspecLevel.val;
  min_res = tspec->minLevel;
  max_res = tspec->maxLevel;
  
  for ( i = min_res; i <= max_res; ++i ) {
    if ( replicate && i > min_res )
      /* include space for replicated tiles */
      total_tiles += 2 * tsl[i].numTile[0] * tsl[i].numTile[1];
    else
      total_tiles += tsl[i].numTile[0] * tsl[i].numTile[1];
  }
  
  return total_tiles;
}

/*
 * Synopsis:
 *      unsigned char *tsmMakeTileBuffer (Pyramid *pyramid)
 *
 * Doc++ Description: *//** 
 *   Allocate memory to store a tile.
 *      
 *  tsmMakeTileBuffer malloc's a buffer large enough to hold a single
 *  (uncompressed) tile for the input pyramid.
 *
 * @param    pyramid the pyramid for which the tile size should be computed
 *
 * @return   the allocated tile buffer, of NULL if failed
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmNumBytesPerTile, tsmReMakeTileBuffer
 *
 */

unsigned char *
tsmMakeTileBuffer (Pyramid *pyramid)
{
  const char *funcName = "tsmMakeTileBuffer";
  unsigned char * buffer;
  int numBytes = tsmNumBytesPerTile (pyramid);

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName,
	     "Creating tile buffer of size %d.", numBytes));
  buffer = (unsigned char *) tsmMemManagerCalloc (1, numBytes);
  if(!buffer)
  {
      tsmError (funcName, "Couldn't allocate tile buffer.");
      TSMTRINC ((TSMMASK|3, -1));
      return (NULL);
  }

  TSMTRACE ((TSMMASK|3, funcName, "Returning buffer %p.", buffer));
  TSMTRINC ((TSMMASK|3, -1));
  return (buffer);
}

/*
 * Synopsis:
 *   unsigned char *tsmReMakeTileBuffer (Pyramid *pyramid,
 *                                       unsigned char *tileBuffer)
 *
 * Doc++ Description: *//** 
 *   Allocate memory to store a tile, if not already done.
 *      
 *  tsmReMakeTileBuffer realloc's tileBuffer so that it is large enough
 *  to hold a single tile for the input pyramid. If the tile buffer that
 *  is passed is NULL, then a new buffer is allocated.
 *
 * @param    pyramid the pyramid for which the tile size should be computed
 * @param    tileBuffer the current tile buffer to be realloc'd
 *
 * @return   the allocated tile buffer, of NULL if failed
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmMakeTileBuffer
 *
 */

unsigned char *
tsmReMakeTileBuffer (Pyramid *pyramid, unsigned char *tileBuffer)
{
  const char *funcName = "tsmReMakeTileBuffer";
  int numBytes = tsmNumBytesPerTile (pyramid);

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName,
	     "Creating tile buffer of size %d.", numBytes));
  tileBuffer = (!tileBuffer)? 
     tsmMemManagerCalloc (1, numBytes) 	:
	tsmMemManagerRealloc (tileBuffer, numBytes);
  if((!tileBuffer && numBytes) ||
     (tileBuffer == (unsigned char *) -1))
  {
      tsmError (funcName, "Couldn't allocate tile buffer.");
      TSMTRINC ((TSMMASK|3, -1));
      return (NULL);
  }

  TSMTRACE ((TSMMASK|3, funcName, "Returning buffer %p.", tileBuffer));
  TSMTRINC ((TSMMASK|3, -1));
  return (tileBuffer);
}

/*
 * Synopsis:
 *      int tsmIsTIFFPresent( Pyramid *, int, tspSymbol )
 *
 * Description:
 *      Checks to see if the image tiles for a particular pyramid level
 *      are stored as a single blocked TIFF file, or as a collection of
 *      raw .tile files.
 *
 * Externals: None
 *
 * Returns: TRUE if images stored in a TIFF, FALSE if stored as .tile files
 *
 * Author: Martin Reddy
 * 
 * Date: 10 July 1997
 *
 */

int
tsmIsTIFFPresent( Pyramid *pyramid, int level, tspSymbol imgFormat )
{
  const char *funcName = "tsmIsTIFFPresent";
  Purl  *tileBasePurl;
  int   returnCode;

  /* Check that we have a tileBasePurl */

  if ( ( tileBasePurl = pyramid->tileBasePurl ) == NULL ) {
    tsmError( funcName, "No Tile Base Purl!" );
    return FALSE;
  }

  /* Now that we have the URL, do the check in a protocol-specific manner */

  switch ( tileBasePurl->protocol ) {
  case tsmLocalFile:
    returnCode = tsmIsTIFFPresent_local( tileBasePurl, pyramid, level,
					 imgFormat );
    break;
  case tsmDPSS:
  case tsmDSM:
  case tsmISS:
    returnCode = FALSE;
    break;
  case tsmHTTP:
    returnCode = tsmIsTIFFPresent_web( tileBasePurl, pyramid, level,
				       imgFormat );
    break;
  default:
    tsmNoProtocolYet( tileBasePurl->protocol, funcName );
    returnCode = FALSE;
  }

  return returnCode;
}

/*
 * Synopsis:
 *     int tsmCheckSumTile (TsmHandle *tsmh, Pyramid *pyramid, int ix, int iy,
 *                          int level, unsigned char *tileBuffer, int numBytes)
 *
 * Doc++ Description: *//** 
 *     Retrieves and checksums a tile.
 *
 *    This routine will perform a tsmGetTile for the specified tile and
 *    then do a tsmCheckSumTileBuffer on the resulting data to calculate
 *    the tile's checksum.
 *
 * @param    tsmh       the TSM session handle returned from tsmOpen.
 * @param    pyramid    the pyramid structure, e.g. as returned from
 *                      tsmGetPyramid.
 * @param    ix         the x-coordinate of the tile to checksum.
 * @param    iy         the y-ccordinate of the tile to checksum.
 * @param    level      the level of the pyramid to retrieve the tile from.
 * @param    tileBuffer a buffer to write the tile data into.
 * @param    numBytes   the byte size of the tile buffer (if set to 0, then
 *                      this value will be computed using tsmNumBytesPerTile)
 *
 * @return   the tile's checksum, or -1 if tsmGetTile failed
 *
 * @author   
 * @version  
 *
 * @see      tsmCheckSumTileBuffer, tsmGetTile, tsmNumBytesPerTile
 *
 */

int
tsmCheckSumTile( TsmHandle *tsmh, Pyramid *pyramid, int ix, int iy, int level, 
		 unsigned char *tileBuffer, int numBytes )
{
  const char *funcName = "tsmCheckSumTile";

  if ( tsmGetTile( tsmh, pyramid, ix, iy, level, tileBuffer ) == FALSE ) {
    tsmError (funcName, "Couldn't read tile!");
    return -1;
  }

  if ( numBytes < 0 ) numBytes = tsmNumBytesPerTile( pyramid );

  return tsmCheckSumTileBuffer( tileBuffer, numBytes );
}

/*
 * Synopsis:
 *      int tsmGetJPEGTileSize( const Pyramid *, int, int, int )
 *
 * Description:
 *      Calculates the compressed size of a single tile in a TIFF file.
 *      This will only work for local disk URLs because TIFFs are only
 *      used for local disk storage of tile data. 
 *
 * Externals: None
 *
 * Returns: the (compressed) size of the JPEG tile, or 0 if error.
 *
 * Author: Martin Reddy
 * 
 * Date: 26 August 1997
 *
 */

int
tsmGetJPEGTileSize( const Pyramid *pyramid, int level, int ix, int iy )
{
  const         char *funcName = "tsmGetJPEGTileSize";
  char          filename[1024];
  TIFF          *handle;
  tsize_t       filesize, numBytes;
  ttile_t       tile_no;
  unsigned char *tileBuffer;
  SimpleTileSet *tileset;

  /* it only makes sense to ask for the JPEG filesize for file: urls */

  if ( pyramid->tileBasePurl->protocol != tsmLocalFile ) {
    tsmError( funcName, "Can only get JPEG tile size for local files" );
    return 0;
  }

  /* work out the name of the JPEG TIFF file */

  tspTIFFFileName( pyramid->pathName, level, tspJPEGCompressed, filename );

  /* open the file for reading */

  if ( ( handle = TIFFOpen( filename, "r" ) ) == NULL ) {
    tsmError( funcName, "Unable to open TIFF file %s", filename );
    return 0;
  }

  /* allocate a tile buffer as we currently read in the entire tile    */
  /* in order to know it's length. This is inefficient when the number */
  /* of tiles in an image is small, but when we have 1000s of tiles,   */
  /* then this will probably be more efficient than reading in the     */
  /* entire directory of file offsets for each tile.                   */

  if ( ( tileBuffer = tsmMakeTileBuffer( (Pyramid *) pyramid ) ) == NULL ) {
    tsmError( funcName, "couldn't allocate a tile buffer" );
    TIFFClose( handle );
    return 0;
  }

  /* read in the tile and save the actual number of bytes read */

  tileset  = &pyramid->tspecLevel.val[level];
  tile_no  = ( iy * tileset->numTile[0] ) + ix;
  numBytes = tsmNumBytesPerTile( (Pyramid *) pyramid );
  filesize = TIFFReadRawTile( handle, tile_no, tileBuffer, numBytes );

  /* got what we wanted, so close the file and return the result */

  free( tileBuffer );
  TIFFClose( handle );
  return max(filesize,0);
}

TsmTileHeader *
tsmMakeTileHeader( void )
{
  return tsmAllocTileHeader();
}

/*
 * Synopsis:
 *   int tsmStartLogging( TsmHandle *tsmh, Pyramid *pyramid, int method, 
 *                        const char *destination )
 *
 * Doc++ Description: *//** 
 *   Start logging for old tsmApi interface.
 *
 *   This function is provided to support logging of tile statistics
 *   in the old server-based tsmApi interface. If you use tsmOpen and
 *   tsmGetPyramid to query a tileset, then you can use this function
 *   to access the logging facilities of tsmApi.
 *
 *   This function is simply a wrapper for the connection-based call,
 *   tsmBeginLogging. Please refer to the documentation for that call
 *   for further details.
 *
 * @param    tsmh a TSM session handle, as returned by tsmOpen
 * @param    pyramid a Pyramid tspec structure, as returned by tsmGetPyramid
 * @param    method one of NL_FILE, NL_SYSLOG, or NL_HOST
 * @param    destination the filename to write the log data to (if method
 *           is NL_FILE), or the host to write the log data to (if method
 *           is NL_HOST).
 *
 * @return   TRUE if logging started successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  22 May 1998
 *
 * @see      tsmStopLogging, tsmPutLog, tsmBeginLogging
 *
 */

int
tsmStartLogging( TsmHandle *tsmh, Pyramid *pyramid, int method, 
		 const char *destination )
{
  static const char *funcName = "tsmStartLogging";
  TsmConnection     connect;

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return FALSE;
  }

  /* vector off to the new connection-based equivalent */

  return tsmBeginLogging( connect, method, destination );
}

/*
 * Synopsis:
 *   int tsmPutLog( TsmHandle *tsmh, Pyramid *pyramid, const char *event_name,
 *                  const char *data, ... )
 *
 * Doc++ Description: *//** 
 *   Log an event for old tsmApi interface.
 *
 *   This function is provided to support logging of tile statistics
 *   in the old server-based tsmApi interface. If you use tsmOpen and
 *   tsmGetPyramid to query a tileset, then you can use this function
 *   to access the logging facilities of tsmApi.
 *
 *   This function is provides the same functionality as the
 *   connection-based call, tsmLog. Please refer to the documentation
 *   for that call for further details.
 *
 * @param    tsmh a TSM session handle, as returned by tsmOpen
 * @param    pyramid a Pyramid tspec structure, as returned by tsmGetPyramid
 * @param    event_name a string describing the event being logged
 * @param    a printf style string specifying the data to be logged for
 *           the event
 *
 * @return   TRUE if data was logged successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  22 May 1998
 *
 * @see      tsmStopLogging, tsmEndLogging, tsmLog
 *
 */

int 
tsmPutLog( TsmHandle *tsmh, Pyramid *pyramid, const char *event_name,
	   const char *data, ... )
{
  static const char *funcName = "tsmPutLog";
  TsmConnection     connect;
  va_list           args;
  char              buffer[512];

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return FALSE;
  }

  /* Now vector off to the appropriate NetLogger call */

  buffer[0] = '\0';
  va_start( args, data );
  vsprintf( buffer, data, args );
  va_end( args );

  return NetLoggerWriteString( connect->nlhandle, (char *) event_name,
			       (char *) buffer ) == 1;
}

/*
 * Synopsis:
 *   int tsmStopLogging( TsmHandle *tsmh, Pyramid *pyramid )
 *
 * Doc++ Description: *//** 
 *   Stop logging for old tsmApi interface.
 *
 *   This function is provided to support logging of tile statistics
 *   in the old server-based tsmApi interface. If you use tsmOpen and
 *   tsmGetPyramid to query a tileset, then you can use this function
 *   to access the logging facilities of tsmApi.
 *
 *   This function is simply a wrapper for the connection-based call,
 *   tsmEndLogging. Please refer to the documentation for that call
 *   for further details.
 *
 * @param    tsmh a TSM session handle, as returned by tsmOpen
 * @param    pyramid a Pyramid tspec structure, as returned by tsmGetPyramid
 *
 * @return   TRUE if logging stopped successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  22 May 1998
 *
 * @see      tsmStartLogging, tsmPutLog, tsmEndLogging
 *
 */

int
tsmStopLogging( TsmHandle *tsmh, Pyramid *pyramid )
{
  static const char *funcName = "tsmStopLogging";
  TsmConnection     connect;

  /* Try and get the connection structure for this pyramid */

  if ( ( connect = tsmGetConnectionFromTsmHandle( tsmh, pyramid ) ) == NULL ) {
    tsmError( funcName, "No such pyramid (%p) in tsmh", pyramid );
    return FALSE;
  }

  /* vector off to the new connection-based equivalent */

  return tsmEndLogging( connect );
}

void
tsmCopyPyramidLevelToBinaryStream (TsmHandle *tsmh, Pyramid *pyramid, 
				   int level, FILE *stream)
{
  const char *funcName = "tsmCopyPyramidLevelToBinaryStream";
  int tilex, tiley, totalNumBytesPerLine, numBytesPerLine, numLines;
  int interchangep;
  char red, blue;
  int i, ix, iy, ixStart, iyStart;
  SimpleTileSet *simpleTileSet;
  TileTspec *tileTspec;
  uchar **tileBuffers, *position, *tmpBuffer;
  TsmTileHeader tileHeader;

  TSMTRACE ((2, funcName, " "));

  /* Get the pyramid level (simple tile set), and allocate a tile
     buffer for each tile of one row. */
  simpleTileSet = &pyramid->tspecLevel.val[level];
  tileTspec = &simpleTileSet->tileTspec;
  tileBuffers = (uchar **)
     tsmMemManagerCalloc (simpleTileSet->numTile[0], sizeof (char *));
  if(!tileBuffers) {
    tsmError (funcName, "Unable to allocate tileBuffers");
    return;
  }
  
  for (tilex = simpleTileSet->minTile[0]; tilex <= simpleTileSet->maxTile[0];
       tilex++) {
    tileBuffers[tilex] = tsmMakeTileBuffer(pyramid);
  }
  tmpBuffer = tsmMakeTileBuffer(pyramid);

  totalNumBytesPerLine = tileTspec->totalPixels[1] * 
    tileTspec->numComponentsPixel * tileTspec->numBytesPerComponent;
  numBytesPerLine = tileTspec->numPixelsX * 
    tileTspec->numComponentsPixel * tileTspec->numBytesPerComponent;
  ixStart = tileTspec->overLap[0]/2;
  iyStart = tileTspec->overLap[1]/2;
  numLines = tileTspec->numPixelsY;
  
  /* For color images, must interchange red and blue, since the tiles have the
     data in bgr order, but everybody else expects rgb. */
  if (tileTspec->numComponentsPixel == 3)
    interchangep = TRUE;
  else
    interchangep = FALSE;

  for (tiley = simpleTileSet->minTile[1]; 
       tiley <= simpleTileSet->maxTile[1];
       tiley++) {
    /* Request the row tiley of tiles. */
    TSMTRACE ((3, funcName, "Requesting row %d", tiley));
    tsmStartReqTiles (tsmh);
    for (tilex = simpleTileSet->minTile[0];
	 tilex <= simpleTileSet->maxTile[0];
	 tilex++) {
      tsmReqTile (tsmh, pyramid, tilex, tiley, level, TSM_NOW);
    }
    tsmStopReqTiles (tsmh);
    /* Get the row of tiles. */
    TSMTRACE ((3, funcName, "Getting row %d\n", tiley));
    for (i = 0; i < simpleTileSet->numTile[0]; i++) {
      tsmGetNextTile( tsmh, &tileHeader, tmpBuffer, 0 );
      tilex = tsmTileHeaderX( &tileHeader );
      memcpy( tileBuffers[tilex], tmpBuffer, tsmTileHeaderSize(&tileHeader) );
      TSMTRACE ((3, funcName, "Got tile, x = %d.", tilex));
    }
    /* Write out the tiles in row order, from bottom to top. */
    TSMTRACE ((3, funcName, "Writing row %d.", tiley));
    for (iy = iyStart; iy < iyStart+numLines; iy++) {
      for (tilex = simpleTileSet->minTile[0];
	   tilex <= simpleTileSet->maxTile[0];
	   tilex++) {
	position = tileBuffers[tilex] + (iy * totalNumBytesPerLine) + ixStart;
	if (interchangep) {
	  for (ix = 0; ix < numBytesPerLine; ix+=3) {
	    blue = position[ix];
	    red = position[ix+2];
	    position[ix] = red;
	    position[ix+2] = blue;
	  }
	}
	fwrite (position, 1, numBytesPerLine, stream);
      }
    }
  }
  tsmFreeTileBuffer( tmpBuffer );
}

/*
 * Synopsis:
 *   void tsmTsmHandleNotEmulated( char *funcName )
 *
 * Description:
 *   A simple routine to provide a standardised error report for 
 *   old functions in the tsmApi that are not included in the
 *   emulation layer of the new tsmApi.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 6 February 1998
 *
 */

void
tsmTsmHandleNotEmulated( char *funcName )
{
  tsmError( funcName, "obsolete tsmApi routine: not emulated" );
}

/* The following functions are all part of the old tsmApi        */
/* but are not emulated under the new connection-based interface */

TsmHandle *
tsmReOpen( const char *dbUrl, TsmHandle *tsmh )
{
  tsmTsmHandleNotEmulated( "tsmReOpen" );
  return NULL;
}

int
tsmNumServers( TsmHandle *tsmh )
{
  tsmTsmHandleNotEmulated( "tsmNumServers" );
  return 1;
}

Pyramid *
tsmReGetPyramid( TsmHandle *tsmh, const char *pyramidName, Pyramid *pyramid )
{
  tsmTsmHandleNotEmulated( "tsmReGetPyramid" );
  return NULL;
}

int
tsmGetAllPyramids( TsmHandle *tsmh, Pyramids *pyramids )
{
  tsmTsmHandleNotEmulated( "tsmGetAllPyramids" );
  return FALSE;
}

int
tsmPrepareForDataSets( TsmHandle *tsmh, int *setIds, int numSetIds )
{
  tsmTsmHandleNotEmulated( "tsmPrepareForDataSets" );
  return TRUE;
}

int
tsmChangeGeoPyramids( TsmHandle *tsmh )
{
  tsmTsmHandleNotEmulated( "tsmChangeGeoPyramids" );
  return TRUE;
}

int
tsmGetNextTileHeader( TsmHandle *tsmh, TsmTileHeader *tileHeader, ... )
{
  tsmTsmHandleNotEmulated( "tsmGetNextTileHeader" );
  return FALSE;
}

int
tsmGetNextTileData( TsmHandle *tsmh, TsmTileHeader *tileHeader,
		    unsigned char *tileBuffer, ... )
{
  tsmTsmHandleNotEmulated( "tsmGetNextTileData" );
  return FALSE;
}

void
tsmSetGetTileFlag( int flag )
{
  tsmTsmHandleNotEmulated( "tsmSetGetTileFlag" );
}

int
tsmGetGetTileFlag( void )
{
  return TRUE;
}

void
tsmSetUseNewTileFlag (int flag)
{
  tsmTsmHandleNotEmulated( "tsmSetUseNewTileFlag" );
}

int
tsmGetUseNewTileFlag( void )
{
  return TRUE;
}

void
tsmSetReadTileFormat( tspSymbol fmt )
{
  tsmTsmHandleNotEmulated( "tsmSetReadTileFormat" );
}

tspSymbol
tsmGetReadTileFormat( void )
{
  return tspLrbtRaster;
}

int
tsmGetTileJFIF( Pyramid *pyramid, int ix, int iy, int level,
		int *numBytes, unsigned char *tileBuffer )
{
  tsmTsmHandleNotEmulated( "tsmGetTileJFIF" );
  return FALSE;
}

void
tsmSetJPEGQuality( int quality ) 
{
  tsmTsmHandleNotEmulated( "tsmSetJPEGQuality" );
}

int
tsmGetJPEGQuality( void )
{
  return 75;
}

int
tsmImg2Tile( TsmHandle *tsmh, char *path_url, char *imgfile, int nl, int ns,
	     int nb, int level, int start_col, int start_row, int block_size,
             tspSymbol imgFormat )
{
  tsmTsmHandleNotEmulated( "tsmImg2Tile" );
  return FALSE;
}

int
tsmDem2Tile( TsmHandle *tsmh, char *path_url, char *imgfile, int nl, int ns,
	     int nb, int level, int start_col, int start_row, int block_size,
             tspSymbol imgFormat )
{
  tsmTsmHandleNotEmulated( "tsmDem2Tile" );
  return FALSE;
}
