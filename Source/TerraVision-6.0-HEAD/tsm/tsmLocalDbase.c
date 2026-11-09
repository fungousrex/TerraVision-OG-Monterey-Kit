/*
 * Name: tsmLocalDbase.c
 *
 * Description: 
 *   Contains routines for manipulating local TSM databases, such as
 *   the PyramidUrls files.
 *
 * Function List:
 *   int tsmAddToLocalDbase( TsmConnection connect )
 *   int tsmLoadDatabases( TsmHandle *tsmh )
 *   int tsmAddToPyramidDatabase( TsmHandle *tsmh, const char *name, char *url)
 *   int tsmLoadPyramidDatabase (TsmHandle *tsmh, const char *pdbLocator)
 *   int tsmReLoadPyramidDatabase (TsmHandle *tsmh, const char *pdbLocator)
 *   int tsmLoadPyramidDatabaseIfExist( TsmHandle *tsmh, const char * )
 *   void tsmSavePyramidDatabase( TsmHandle *tsmh, const char *pdbLocator )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmLocalDbase.c,v 1.9.6.2 2002/03/14 05:22:57 reddy Exp $
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
 *   int tsmAddToLocalDbase( TsmConnection connect );
 *
 * Doc++ Description: *//** 
 *   Add an entry to a local dataset's PyramidUrls database.
 *      
 *   This function will add an entry for a localdisk dataset to the
 *   PyramidUrls database, but only if the name does not already exist
 *   in the file. If no such file exists, then the routine will create
 *   one with a single entry for the specified name and url. For example,
 *   if the connection describes a dataset called
 *   ~magic/TileSets/Pyramids/yosemite.oi, then an entry for
 *   "yosemite.oi" will be added to
 *   ~magic/TileSets/Pyramids/PyramidUrls.
 *
 *   If the connection is for a DPSS dataset, then nothing will be 
 *   done (it is assumed that the DSM already contains an entry for
 *   the dataset). If the connection is for an HTTP dataset, then this
 *   function will fail as it is not possible to update a file on
 *   an arbitrary http server.
 *
 * @param    connect the connection handle for the dataset to be added
 *
 * @return   TRUE if all went well, FALSE if an error occurred.
 *
 * @author   Martin Reddy
 * @version  25 March 1998
 *
 * @see      
 *
 */

int
tsmAddToLocalDbase( TsmConnection connect )
{
  static const char *funcName = "tsmAddToLocalDbase";
  TsmHandle         *tsmh;
  char              tsmh_name[512];

  /* Check the connection structure and enforce its access mode */
  
  if ( ! tsmCheckConnectMode( funcName, connect, TSM_WRITE_ONLY, TRUE ) )
    return FALSE;

  switch ( connect->type ) {

  case TSM_DPSS:
    /* If the connection is for a DPSS dataset then we don't have */
    /* to do anything as the DSM will already have an entry.      */
    return TRUE;

  case TSM_HTTP:
    /* We can't write anything to an HTTP dataset's PyramidUrls file */
    tsmError( funcName, "cannot write PyramidUrls for http dataset" );
    return FALSE;
    
  case TSM_LOCAL:
    /* add an entry for the localdisk dataset to the PyramidUrls file */
    /* Until the new tsmApi provides connection-based routines for    */
    /* local database access, we'll have to do this the old way...    */

    tsmExtractTsmAndPyramidName( connect->location->url, tsmh_name, NULL );

    if ( ( tsmh = tsmOpen( tsmh_name ) ) == NULL ) {
      tsmError( funcName, "couldn't open tile set: %s\n", tsmh_name );
      return FALSE;
    }
    
    tsmAddToPyramidDatabase( tsmh, connect->name, "" );

    tsmClose( tsmh );
    return TRUE;
  default:
    tsmNoProtocolYet( connect->location->protocol, funcName );
    return FALSE;
  }
}

/*
 * Synopsis:
 *      int tsmLoadDatabases( TsmHandle *tsmh )
 *
 * Doc++ Description: *//** 
 *   Load the Pyramid database.
 *
 *   tsmLoadDatabases is a utility function to explicitly load the
 *   databases from their default locations.  You should only need to
 *   call this if you must look at the databases before getting a
 *   Pyramid. This function used to also load the GeoPyramid database
 *   file, but this functionality has been removed as GeoPyramids are
 *   no longer used.
 *
 * @param    tsmh the open TSM session handle
 *
 * @return   TRUE if successful, FALSE if an error occurred
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      
 *
 */

int 
tsmLoadDatabases( TsmHandle *tsmh )
{
  static const char *funcName = "tsmLoadDatabases";
  int               returnCode;

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|1, funcName, "tsmh = %p", tsmh ));

  returnCode = tsmLoadPyramidDatabase( tsmh, "" );

  TSMTRACE(( TSMMASK|1, funcName, "returnCode = %d", returnCode ));
  TSMTRINC(( TSMMASK|1, -1 ));
  return returnCode;
}

/*
 * Synopsis:
 *      int tsmAddToPyramidDatabase( TsmHandle *tsmh, const char *name,
 *                                   char *url )
 *
 * Doc++ Description: *//** 
 *  Add an entry to a Pyramid database.
 *      
 *  Adds the specified name and url to a pyramid's PyramidsUrl
 *  database, but only if the name does not already exist in the
 *  file. If no such file exists, then the routine will create one
 *  with a single entry for the specified name and url.
 *
 * @param    tsmh the open TSM session handle
 * @param    name the name of the pyramid to be added to the database
 * @param    url the URL of the pyramid to be addded to the database
 *
 * @return   TRUE if operation successful, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version
 *
 * @see      tsmLoadPyramidDatabase, tsmSavePyramidDatabase
 *
 */

int
tsmAddToPyramidDatabase( TsmHandle *tsmh, const char *name, char *url )
{
  static const char *funcName = "tsmAddToPyramidDatabase";
  const int  translateNames = FALSE;
  int        i, numNames;
  char const **names;

  if ( url == NULL || strlen( url ) == 0 )
    url = " ";

  /* First, try to load in the database. If we can't load it in, then */
  /* there is no Pyramid database, so we should create one with a     */
  /* single entry for the desired name and url.                       */

  if ( tsmLoadPyramidDatabaseIfExist( tsmh, "" ) == FALSE ) {

    if ( ( tsmh->pyramidUrls = tsmMakeNameUrlIds( 1 ) ) == NULL ) {
      tsmError( funcName, "Couldn't allocate PyramidUrls array" );
      return FALSE;
    }

    tsmh->pyramidUrls->names[0] = tsmMemManagerStrdup( name );
    tsmh->pyramidUrls->urls[0]  = tsmMemManagerStrdup( url );
    tsmh->pyramidUrls->ids[0]   = 0;

    if ( translateNames == TRUE )
      tsmTranslateEmptyUrlsToNamedUrls( tsmh, tsmh->pyramidUrls,
					tsmh->dbPurl, "Pyramids" );

    tsmSavePyramidDatabase( tsmh, "" );
    return TRUE;
  }

  /* If we already have a PyramidUrls file, then go through each entry */
  /* and see if we already have the specified name in the database. If */
  /* we do, then we just exit - nothing to do.                         */

  numNames = tsmh->pyramidUrls->numItems;
  names    = tsmh->pyramidUrls->names;
  for ( i = 0; i < numNames; i++ ) {
    if ( strcmp( names[i], name ) == 0 )
      return FALSE;
  }

  /* If we got this far, then we have a PyramidUrls file, but it does */
  /* not contain the specified name, so we will add it here. In order */
  /* to do this, we have to expand the NameUrlIds array by one.       */

  numNames = ++tsmh->pyramidUrls->numItems;

  tsmh->pyramidUrls->names = (char const **)
    tsmMemRealloc( (void *) tsmh->pyramidUrls->names,
                   numNames*sizeof(char *) );
  if ( tsmh->pyramidUrls->names == NULL ) {
    tsmError( funcName, "Unable to realloc names array" );
    return FALSE;
  }

  tsmh->pyramidUrls->urls = (char const **)
    tsmMemRealloc( (void *) tsmh->pyramidUrls->urls,
                   numNames * sizeof(char *) );
  if ( tsmh->pyramidUrls->urls == NULL ) {
    tsmError( funcName, "Unable to realloc urls array" );
    return FALSE;
  }

  tsmh->pyramidUrls->ids = (int *)
    tsmMemManagerRealloc( tsmh->pyramidUrls->ids, numNames * sizeof(int) );
  if ( tsmh->pyramidUrls->ids == NULL ) {
    tsmError( funcName, "Unable to realloc ids array" );
    return FALSE;
  } 
  
  /* we have made some space for the new entry - now add it and save */

  tsmh->pyramidUrls->names[numNames-1] = tsmMemManagerStrdup( name );
  tsmh->pyramidUrls->urls[numNames-1]  = tsmMemManagerStrdup( url );
  tsmh->pyramidUrls->ids[numNames-1]   = 0;

  if ( translateNames == TRUE ) 
    tsmTranslateEmptyUrlsToNamedUrls( tsmh, tsmh->pyramidUrls,
				      tsmh->dbPurl, "Pyramids" );

  tsmSavePyramidDatabase( tsmh, "" );
  return TRUE;
}

/*
 * Synopsis:
 *      int tsmLoadPyramidDatabase (TsmHandle *tsmh, const char *pdbLocator)
 *
 * Doc++ Description: *//** 
 *  Load a Pyramid database.
 *      
 *  Use tsmLoadPyramidDatabase is to explicitly load the Pyramid
 *  database.  Unless you wish to override defaults, you should not
 *  need to call this function, since tsmGetPyramid will load it
 *  when necessary.
 *
 *  The URL of the database is determined by "appending" pdbLocator to
 *  the baseUrl (the argument to tsmOpen) unless pdbLocator is a fully
 *  specified URL, in which case pdbLocator will be used directly.
 *
 *  Use the empty string ("") to open the database at its default
 *  location relative to the baseUrl.  See tsmAppendLocatorToPurl for a
 *  definition of "appending".
 *
 * @param    tsmh       the TSM session handle.
 * @param    pdbLocator used to determine the URL of the database
 *
 * @return   TRUE if database is loaded, FALSE if load failed
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmSavePyramidDatabase, tsmAddToPyramidDatabase
 *
 */

int
tsmLoadPyramidDatabase( TsmHandle *tsmh, const char *pdbLocator )
{
  static const char *funcName = "tsmLoadPyramidDatabase";
  int               returnCode;

  if (!pdbLocator) pdbLocator = "";

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|1, funcName, "tsmh = %p, pdbLocator = '%s'.", tsmh, pdbLocator ));

  tsmh->pdbPurl = tsmAppendLocatorToPurl( pdbLocator, tsmh->dbPurl, 
					  TSM_DEFAULT_PDB_NAME );

  TSMTRACE(( TSMMASK|2, funcName, "pdbLocator = %s", tsmh->pdbPurl->url ));

  if ( tsmh->pdbPurl ) {
    switch ( tsmh->pdbPurl->protocol ) {
    case tsmDSM:
    case tsmDPSS:
      returnCode = tsmLoadPyramidDatabase_dsm (tsmh);
      break;
    case tsmLocalFile:
      returnCode = tsmLoadPyramidDatabase_local (tsmh);
      break;
    case tsmHTTP:
      returnCode = tsmLoadPyramidDatabase_web (tsmh);
      break;
    default:
      tsmNoProtocolYet (tsmh->pdbPurl->protocol, funcName);
      returnCode = FALSE;
    }
  } else
    returnCode = FALSE;

  if ( returnCode == TRUE ) {
    tsmTranslateEmptyUrlsToNamedUrls( tsmh, tsmh->pyramidUrls, tsmh->dbPurl,
				      "Pyramids");
    if ( TSMTRACEP(TSMMASK|4) ) {
      TSMTRACE(( TSMMASK|4, funcName, "Loaded the following Pyramid from '%s':\n", 
		 tsmh->pdbPurl->url ));

      if ( pthr_trace_stream (NULL) == stdout )
	tsmSavePyramidDatabase_local1 (tsmh, "stdout");
      else
	tsmSavePyramidDatabase_local1 (tsmh, "stderr");
    }
  }
  else
    TSMTRACE(( TSMMASK|1, funcName, "couldn't load database: %s", tsmh->pdbPurl->url));

  /* -- changed the error reporting to be silent by default so that we
     -- don't get any unnecessary errors during operations such as
     -- tsmAddToPyramidDatabase() where we attempt to load and create
     -- a database if one not found.
  else
    tsmError (funcName, "Couldn't load database '%s'.", tsmh->pdbPurl->url);
  */

  TSMTRACE(( TSMMASK|1, funcName, "returnCode = %d", returnCode ));
  TSMTRINC(( TSMMASK|1, -1));
  return returnCode;
}

/* ================================================================
   This function will perform the same operation as tsmLoadPyramidDatabase
   but it will try to check whether the database file exists first. If
   it does not then it simply returns a FALSE result and outputs no
   error message (whereas tsmLoadPyramidDatabase would output an error
   message if it tried to open a database that didn't exist)
   ================================================================ */

int
tsmLoadPyramidDatabaseIfExist( TsmHandle *tsmh, const char *pdbLocator )
{
  static const char *funcName = "tsmLoadPyramidDatabaseIfExist()";
  PRFileInfo fileinfo;
  Purl   *tempPurl;

  if ( pdbLocator == NULL )
    pdbLocator = "";

  tempPurl = tsmAppendLocatorToPurl( pdbLocator, tsmh->dbPurl,
				     TSM_DEFAULT_PDB_NAME );

  if ( tempPurl != NULL ) {
    switch( tempPurl->protocol ) {
    case tsmLocalFile:
      if ( PR_GetFileInfo ( tempPurl->uri, &fileinfo ) == PR_SUCCESS )
	return tsmLoadPyramidDatabase( tsmh, pdbLocator );
      break;
    case tsmDSM:
    case tsmDPSS:
    case tsmHTTP:
    default:
      tsmError( funcName, "no check if database exists: loading anyway" );
      return tsmLoadPyramidDatabase( tsmh, pdbLocator );
    }
  }

  return FALSE;
}

/*
 * Synopsis:
 *      void tsmSavePyramidDatabase( TsmHandle *tsmh, const char *pdbLocator )
 *
 * Doc++ Description: *//** 
 *  Save a Pyramid database.
 *      
 *  Use tsmSavePyramidDatabase is to explicitly save the Pyramid
 *  database.  Unless you wish to override defaults, you should not
 *  need to call this function, since tsmAddToPyramidDatabase will
 *  call it when necessary.
 *
 *  The URL of the database is determined by "appending" pdbLocator to
 *  the baseUrl (the argument to tsmOpen) unless pdbLocator is a fully
 *  specified URL, in which case pdbLocator will be used directly.
 *
 *  Use the empty string ("") to open the database at its default
 *  location relative to the baseUrl.  See tsmAppendLocatorToPurl for a
 *  definition of "appending".
 *
 * @param    tsmh       the TSM session handle.
 * @param    pdbLocator used to determine the URL of the database
 *
 * @return   Nothing (void)
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmLoadPyramidDatabase, tsmAddToPyramidDatabase
 *
 */

void
tsmSavePyramidDatabase( TsmHandle *tsmh, const char *pdbLocator )
{
  static const char *funcName = "tsmSavePyramidDatabase()";

  if ( pdbLocator == NULL )
    pdbLocator = "";

  TSMTRINC ((TSMMASK|1, 1));
  TSMTRACE ((TSMMASK|1, funcName, "tsmh = %p, pdbLocator = '%s'.",
	     tsmh, pdbLocator));
  tsmh->pdbPurl = tsmAppendLocatorToPurl (pdbLocator, tsmh->dbPurl, 
					  TSM_DEFAULT_PDB_NAME);
  TSMTRACE ((TSMMASK|2, funcName, "pdbLocator = %s", tsmh->pdbPurl->url));

  if ( tsmh->pdbPurl ) {
    switch ( tsmh->pdbPurl->protocol ) {
    case tsmLocalFile:
      tsmSavePyramidDatabase_local( tsmh, tsmh->pdbPurl );
      break;
    case tsmDSM:
    case tsmDPSS:
    case tsmHTTP:
    default:
      tsmNoProtocolYet( tsmh->pdbPurl->protocol, funcName );
    }
  }

  TSMTRACE ((TSMMASK|1, funcName, "done"));
  TSMTRINC ((TSMMASK|1, -1));
}

/*
 * Synopsis:
 *    int tsmReLoadPyramidDatabase (TsmHandle *tsmh, const char *pdbLocator)
 *
 * Doc++ Description: *//** 
 *  Load a Pyramid database if not done already.
 *      
 *  tsmReLoadPyramidDatabase is the same as tsmLoadPyramidDatabase
 *  except that it will only load the database if its not already
 *  there.
 *
 * @param    tsmh the TSM session handle.
 * @param    pdbLocator used to determine the URL of the database
 *
 * @return   TRUE if database is (or was) loaded, FALSE if load failed
 *
 * @author   Yvan Leclerc
 * @version  
 *
 * @see      tsmLoadPyramidDatabase
 * */

int 
tsmReLoadPyramidDatabase (TsmHandle *tsmh, const char *pdbLocator)
{
  static const char *funcName = "tsmReLoadPyramidDatabase()";
  int returnCode;

  if (!pdbLocator) pdbLocator = "";
  TSMTRINC ((TSMMASK|5, 1));
  TSMTRACE ((TSMMASK|5, funcName, "tsmh = %p, pdbLocator = %s", 
	     tsmh, pdbLocator));

  /* If the database is already loaded, or there's no base URL, then
     there's nothing to do. */
  if (tsmh->pyramidUrls || (tsmh->dbPurl->protocol == tsmNoProtocol))
    returnCode = TRUE;
  else
    returnCode = tsmLoadPyramidDatabase (tsmh, pdbLocator);

  TSMTRACE ((TSMMASK|5, funcName, "returnCode = %d", returnCode));
  TSMTRINC ((TSMMASK|5, -1));
  return (returnCode);
}


