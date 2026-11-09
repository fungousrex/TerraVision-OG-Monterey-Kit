/*
 * Name: tsmTspec.c
 *
 * Description: 
 *   Contains user-level routines for manipulating tspec files.
 *   These functions are externally visible to the user.
 *
 * Function List:
 *   Pyramid *tsmReadTspec( char *url, TsmConnectionParams params )
 *   int tsmWriteTspec( TsmConnection connect )
 *   int tsmPrintTspec( tsmConnection connect );
 *   int tsmSetTspecField( TsmConnection connect, Pyramid *tspec )
 *   int tsmCopyTspecField( TsmConnection connect_in,TsmConnection connect_out)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTspec.c,v 1.12.4.2 2001/06/20 22:26:21 reddy Exp $
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

/*
 * Synopsis:
 *   Pyramid *tsmReadTspec( char *url, TsmConnectionParams params )
 *
 * Doc++ Description: *//** 
 *   Read a tspec file from disk.
 *
 *   Parses a tspec file at the end of a file or http URL. The parsed
 *   tspec is returned in a Pyramid structure, or NULL if failed.
 *   N.B. this function is implicitly called by tsmConnect, so you should
 *   never need to use this function directly.
 *
 * @param    url the file or http URL for the pyramid to get the tspec from
 * @param    params a TsmConnectionParams structure, or NULL
 *
 * @return   Allocated Pyramid structure, or NULL
 *
 * @author   Martin Reddy
 * @version  12 January 1998
 *
 * @see      tsmConnect
 *
 */

Pyramid *
tsmReadTspec( char *url, TsmConnectionParams params )
{
  const char *funcName = "tsmReadTspec";
  Pyramid    *pyramid;
  Purl       *purl;

  /* Try to parse the URL into a Parsed Url (purl) structure */

  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "Could not parse tspec URL '%s'", url );
    return NULL;
  }

  /* Now try to load the tspec in a protocol specific manner */
  /* Currently only file and http protocols are supported.   */
  /* DPSS's either contain file or http URLs to their tspecs */
  /* or they have the tspec stored as a block of the dataset */

  switch ( purl->protocol ) {
  case tsmLocalFile:
    pyramid = tsmReadTspec_local( purl->uri, params );
    break;
  case tsmHTTP:
    pyramid = tsmReadTspec_web( purl, params );
    break;
  case tsmWMT:
    pyramid = tsmReadTspec_wms( purl->url, params );
    break;
  case tsmDPSS:
    pyramid = NULL;
    break;
  default:
    tsmNoProtocolYet( purl->protocol, funcName );
    pyramid = NULL;
  }

  tsmFreePurl( purl );

  /* Do we do a validate on this tspec structure? */

  if ( params == NULL || params->validate_tspec == TRUE ) {
    if ( tsmValidateTspec( pyramid ) == FALSE ) {
      tsmError( funcName, "Tspec error in %s", url );
      tsmFreeTspec( pyramid );
      return NULL;
    }
  }

  return pyramid;
}

/*
 * Synopsis:
 *    int tsmWriteTspec( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *    Save a connection's tspec to disk.
 *
 *    Writes out the tspec file that is associated with a connection.
 *    You only need to do this if you have changed a tspec file entry
 *    or if you are creating a new dataset (write mode) and no
 *    tspec file currently exists, e.g. 
 *
 *    \begin{enumerate}
 *    \item changing an entry: do a
 *             tspec = (Pyramid *) tsmGet( c, TSM_DATASET_TSPEC )
 *          call, change something in tspec, then call this function.
 *    \item creating new tspec: associate a Pyramid structure with the
 *          connection with either tsmSetTspecField or tsmCopyTspecField,
 *          then call this function.
 *    \end{enumerate}
 *
 *    If you are writing the tspec to a DPSS, then currently we only 
 *    write the file to a block on the DPSS itself. We don't update any
 *    metadata URL files. This also means that you should only use this
 *    function to write tspecs to a DPSS for sets that were written
 *    using the tsmApi (because it will always allocate an extra block
 *    for the tspec).
 *
 *    N.B. the tsmConnect call will automatically read in a tspec for
 *    a dataset if it exists.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 *
 * @return   TRUE if all went well, FALSE if something went wrong
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmSetTSpecField, tsmCopyTspecField, tsmConnect, tsmReadTspec
 *
 */

int
tsmWriteTspec( TsmConnection connect )
{
  const char *funcName = "tsmWriteTspec";
  int        status;

  /* Check the connection structure and enforce it's access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_WRITE_ONLY, TRUE ) )
    return FALSE;
  
  /* Write out the tspec file - currently only works to localhost */

  switch ( connect->type ) {
  case TSM_LOCAL:
    status = tspSaveTspec( connect->location->uri, connect->tspec );
    break;
  case TSM_DPSS:
    status = tsmWriteTspec_dpss( connect );
    break;
  case TSM_HTTP:
  default:
    tsmNoProtocolYet( connect->location->protocol, funcName );
    status = FALSE;
  }

  return status;
}

/*
 * Synopsis:
 *    int tsmPrintTspec( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *    Output a connection to stdout.
 *
 *    Displays the tspec file that is associated with a connection.
 *    The output is directed to stdout. If you want to write the
 *    tspec file to the dataset, then you should use tsmWriteTspec.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  9 February 1998
 *
 * @see      tsmWriteTspec, tsmSetTSpecField, tsmCopyTspecField, tsmConnect,
 *           tsmReadTspec
 *
 */

void
tsmPrintTspec( TsmConnection connect )
{
  static const char *funcName = "tsmPrintTspec";

  /* Check the connection structure and enforce it's access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_WRITE, TRUE ) )
    return;
  
  /* Write out the tspec file to stdout */

  tspSaveTspec( "stdout", connect->tspec );
}

/*
 * Synopsis:
 *   int tsmSetTspecField( TsmConnection connect, Pyramid *tspec )
 *
 * Doc++ Description: *//** 
 *   Associates a pyramid tspec structure with a connection.
 *
 *   For example, if you are creating a new dataset then you need to
 *   create a tspec file for this dataset. You would do this by first
 *   creating the tspec structure (e.g. with tsmCreateTspec), then
 *   associating this structure with the connection (this function),
 *   then you could call tsmWriteTspec to write the tspec file out to
 *   disk.
 *
 *   N.B. this function will take a complete *copy* of the tspec
 *   structure. The memory allocated for this copy will of course
 *   be deallocated when you do a tsmDisconnect. If the connection
 *   already has a tspec structure associated with it, then this
 *   will be deallocated for you before taking a copy of the new
 *   tspec structure.
 *
 *   N.B.2. the tsmConnect call will automatically read in a tspec for
 *   a dataset if it exists, so the tspec field of the connection
 *   will already be filled out in this case.
 *
 * @param    connect a dataset connection, as returned by tsmConnect
 * @param    tspec the Pyramid tspec structure to associate with connection
 *
 * @return   TRUE if all went well, FALSE if something went wrong
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmWriteTspec, tspCreateTspec
 * */

int 
tsmSetTspecField( TsmConnection connect, Pyramid *tspec )
{
  static const char *funcName = "tsmSetTspecField";
  char              *name;
  int               set_id;

  if ( connect == NULL || tspec == NULL ) {
    tsmError( funcName, "Connection and tspec structs must be non NULL" );
    return FALSE;
  }

  /* Grab the current tspec's name and set-id */

  if ( connect->tspec != NULL && connect->tspec->identityTspec.name != NULL )
    name = tsmMemStrdup( connect->tspec->identityTspec.name );
  else
    name = tsmMemStrdup( connect->name );

  if ( connect->tspec != NULL )
    set_id = connect->tspec->identityTspec.setId;
  else
    set_id = -1;

  /* Free the old tspec data */

  if ( connect->tspec != NULL )
    tsmFreeTspec( connect->tspec );

  /* copy the new tspec fields into the connection */

  connect->tspec = tsmCopyPyramid( tspec, connect->location->url );

  /* set the tspec name and set-id fields for the new tspec */

  if ( connect->tspec->identityTspec.name != NULL )
    tsmMemFree( connect->tspec->identityTspec.name );

  connect->tspec->identityTspec.name  = name;
  connect->tspec->identityTspec.setId = set_id;

  /* recalculate the tile size and reallocate any tile buffers */

  connect->tileBufferSize = tsmNumBytesPerTile( connect->tspec );

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmCopyTspecField(TsmConnection connect_in, TsmConnection connect_out)
 *
 * Doc++ Description: *//** 
 *    Takes a copy of a tspec structure. 
 *
 *    If you have a connection with a tspec structure, and you want to
 *    associate this same tspec structure with another connection, then
 *    you can use this function. For example, if you are creating a
 *    copy of a dataset, then you should open a read connection to
 *    the dataset and a write connection. Then you can create the
 *    new dataset tspec file by calling this function, followed by
 *    tsmWriteTspec().
 *
 *    This function effectively does a tsmSetTspecField for the output
 *    connection, from the input connection's tspec field. N.B. it does
 *    not write out any data to disk. See tsmCopyTspec to do this.
 *
 * @param    connect_in the input connection to read the tspec structure from
 * @param    connect_out the output connection to be updated
 *
 * @return   TRUE if all went well, FALSE if something went wrong
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmCopyTspec, tsmSetTspecField, tsmWriteTspec,
 *           tsmCopyTile, tsmCopyAllTiles
 *
 */

int
tsmCopyTspecField( TsmConnection connect_in, TsmConnection connect_out )
{
  const char *funcName = "tsmCopyTspecField";

  if ( connect_in == NULL || connect_out == NULL ) {
    tsmError( funcName, "Both connection structs must be non NULL" );
    return FALSE;
  }

  return tsmSetTspecField( connect_out, connect_in->tspec );
}

/*
 * Synopsis:
 *    int tsmValidateTspec( Pyramid *tspec );
 *
 * Doc++ Description: *//** 
 *    Validate a tspec structure loaded from disk.
 *
 *    This function will take a Pyramid structure (for example, as 
 *    returned by tsmReadTspec) and will do various checks to ensure
 *    that the fields in the structure are valid. This routine may
 *    help to throw up some hints or solutions to some dataset
 *    problems.
 *
 *    The following checks are currently performed (with an error message
 *    and a return code of FALSE resulting if any one check fails):
 *
 *    \begin{itemize}
 *      \item tspec != NULL
 *      \item tspec->geoName != NULL
 *      \item tspec->invResScaleFactor == 2
 *      \item tspec->maxLevel >= tspec->minLevel
 *      \item tspec->maxResolution >= tspec->minResolution
 *      \item tspec->tspecLevel.val != NULL
 *      \item tspec->tspecLevel.len == tspec->maxLevel - tspec->MinLevel + 1
 *      \item tile->totalPixels[0] == tile->numPixelsX + tile->overlap[0]
 *      \item tile->totalPixels[1] == tile->numPixelsY + tile->overlap[1]
 *      \item for each SimpleTileSet, check:
 *      \begin{itemize}
 *        \item numTile[0] == maxTile[0] - minTile[0] + 1
 *        \item numTile[1] == maxTile[1] - minTile[1] + 1
 *        \item numValidPixel[0] == maxValidPixel[0] - minValidPixel[0] + 1
 *        \item numValidPixel[1] == maxValidPixel[1] - minValidPixel[1] + 1
 *      \end{itemize}
 *    \end{itemize}
 *
 * @param    tspec a (hopefully) valid Pyramid structure
 *
 * @return   TRUE if tspec is valid, FALSE if it failed a validity test
 *
 * @author   Martin Reddy
 * @version  16 March 1998
 *
 * @see      tsmReadTspec
 *
 */

int
tsmValidateTspec( Pyramid *tspec )
{
  static const char *funcName = "tsmValidateTspec";
  SimpleTileSet     *tset;
  TileTspec         *tile;
  int               lev;

  TSMTRACE(( TSMMASK|2, funcName, "Validating tspec structure (%p)", tspec ));

  /* Validate fields in the base Pyramid and SimpleTileSet structures */

  if ( tspec == NULL ) {
    tsmError( funcName, "Tspec pointer is NULL" );
    return FALSE;
  }

  if ( tspec->geoName == NULL ) {
    tsmError( funcName, "Tspec geo-name string is NULL" );
    return FALSE;
  }

  if ( tspec->invResScaleFactor != 2.0 ) {
    tsmError( funcName, "invResScaleFactor != 2.0 (%1.2f)",
	      tspec->invResScaleFactor );
    return FALSE;
  }

  if ( tspec->maxLevel < tspec->minLevel ) {
    tsmError( funcName, "maxLevel < minLevel (%d < %d)",
	      tspec->maxLevel, tspec->minLevel );
    return FALSE;
  }

  if ( tspec->maxResolution < tspec->minResolution ) {
    tsmError( funcName, "maxResolution < minResolution (%d < %d)",
	      tspec->maxResolution, tspec->minResolution );
    return FALSE;
  }

  if ( tspec->tspecLevel.val == NULL ) {
    tsmError( funcName, "SimpleTileSet array pointer is NULL" );
    return FALSE;
  }

  if ( tspec->tspecLevel.len != tspec->maxLevel - tspec->minLevel + 1 ) {
    tsmError( funcName, "%d SimpleTileSets found. Should be %d",
	      tspec->tspecLevel.len, tspec->maxLevel - tspec->minLevel + 1 );
    return FALSE;
  }

  /* Validate entries in the Pyramid's base TileTspec structure */

  tile = &tspec->tileTspec;

  if ( tile->totalPixels[0] != tile->numPixelsX + tile->overLap[0] ) {
    tsmError( funcName, "Invalid Total X Pixels (%d). Should be %d",
	      tile->totalPixels[0], tile->numPixelsX + tile->overLap[0] );
    return FALSE;
  }

  if ( tile->totalPixels[1] != tile->numPixelsY + tile->overLap[1] ) {
    tsmError( funcName, "Invalid Total Y Pixels (%d). Should be %d",
	      tile->totalPixels[1], tile->numPixelsY + tile->overLap[1] );
    return FALSE;
  }

  /* Validate entries in each of the SimpleTileSet levels */

  for ( lev = tspec->minLevel; lev <= tspec->maxLevel; ++lev ) {

    tset = &tspec->tspecLevel.val[lev];

    if ( tset->numTile[0] != tset->maxTile[0] - tset->minTile[0] + 1 ) {
      tsmError( funcName, "Invalid Lev %d Num Tiles (%d). Should be %d", lev,
		tset->numTile[0], tset->maxTile[0] - tset->minTile[0] + 1 );
      return FALSE;
    }

    if ( tset->numTile[1] != tset->maxTile[1] - tset->minTile[1] + 1 ) {
      tsmError( funcName, "Invalid Lev %d Num Tiles (%d). Should be %d", lev,
		tset->numTile[0], tset->maxTile[0] - tset->minTile[0] + 1 );
      return FALSE;
    }

    if ( tset->numValidPixel[0] != tset->maxValidPixel[0] -
	 tset->minValidPixel[0] + 1 ) {
      tsmError( funcName, "Invalid Lev %d Valid X Pixels (%d). Should be %d",
		lev, tset->numValidPixel[0], tset->maxValidPixel[0] -
		tset->minValidPixel[0] + 1 );
      return FALSE;
    }

    if ( tset->numValidPixel[1] != tset->maxValidPixel[1] -
	 tset->minValidPixel[1] + 1 ) {
      tsmError( funcName, "Invalid Lev %d Valid Y Pixels (%d). Should be %d",
		lev, tset->numValidPixel[1], tset->maxValidPixel[1] -
		tset->minValidPixel[1] + 1 );
      return FALSE;
    }

  }

  TSMTRACE(( TSMMASK|2, funcName, "tspec structure is valid (%p)", tspec ));
  return TRUE;
}
