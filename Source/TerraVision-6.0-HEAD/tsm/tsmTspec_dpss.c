/*
 * Name: tsmTspec_dpss.c
 *
 * Description: 
 *   Contains routines for manipulating tspec files for DPSS datasets.
 *   These functions are private to the tsmApi.
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   Pyramid *tsmReadTspec_dpss( TsmConnectionDpss *connect )
 *   Pyramid *tsmReadEmbeddedTspec_dpss( TsmConnection *connect )
 *   int tsmWriteEmbeddedTspec_dpss( TsmConnection com_connect )
 *
 * Revision History:
 *   Created 12 Mar 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTspec_dpss.c,v 1.13.4.1 2001/07/10 06:17:44 leei Exp $
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

/*
 * Synopsis:
 *   Pyramid *tsmReadTspec_dpss( TsmConnectionDpss *, TsmConnectionParams )
 *
 * Description:
 *   This function will try to load the tspec data for a DPSS dataset.
 *   First of all, the function will try to load the tspec data from
 *   the DPSS set itself (unless TSM_PARAM_DPSS_TSPEC is FALSE). If
 *   this fails, then the set_info field of the connect structure is
 *   used in order to find all of the tspec URLs associated with the
 *   DPSS dataset. It will then try to load the tspec from each of these
 *   in turn until one succeeds. If none succeed, then you're SOL.
 *
 * Returns: loaded Pyramid structured
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

Pyramid *
tsmReadTspec_dpss( TsmConnectionDpss *connect, TsmConnectionParams params )
{
  const char *funcName = "tsmReadTspec_dpss";
  TsmConnection com_connect = connect->aconn.conn;
  int        attempt;
  Pyramid    *pyramid = NULL;
  dsmSetInfo dsi;
  int        use_dpss = TRUE;

  if ( connect == NULL || connect->set_info == NULL ) return NULL;

  /* By default we try to load the tspec file from that DPSS */
  /* dataset, unless TSM_PARAM_DPSS_TSPEC is set to FALSE    */

  if ( tsmIsParamSet( params, TSM_PARAM_DPSS_TSPEC ) && ! params->dpss_tspec )
    use_dpss = FALSE;

  if ( use_dpss == TRUE )
    if ( ( pyramid = tsmReadEmbeddedTspec_dpss( connect ) ) != NULL )
      com_connect->tspec_url = tsmMemStrdup( com_connect->location->url );

  /* If we don't have a valid tspec file yet, then go  */
  /* through each of the metadata entries of the DSM.  */
  /* These contain URLs (http or file) for tspec files */

  if ( pyramid == NULL ) {
    for ( attempt = 0, dsi = connect->set_info;
	  pyramid == NULL && attempt < dsi->metadata_url.metadata_url_len;
	  ++attempt ) {

      if ( attempt > 0 )
	tsmError( funcName, "Trying next URL: %s", dsiMetadataN(dsi,attempt));

      TSMTRACE ((TSMMASK|2, funcName, "Reading Tspec from URL '%s'.",
		 dsiMetadataN(dsi,attempt)));

      pyramid = tsmReadTspec( dsiMetadataN(dsi,attempt), params );
      if ( pyramid != NULL )
	com_connect->tspec_url = tsmMemStrdup( dsiMetadataN(dsi,attempt) );
    }
  }

  /* If a geoName was not specified, then try to make one up */

  tsmEnsureGeoNameSet( pyramid );

  return pyramid;
}

/*
 * Synopsis:
 *   Pyramid *tsmReadEmbeddedTspec_dpss( TsmConnection *connect )
 *
 * Description:
 *   Read a tspec file from a DPSS Pyramid.
 *
 *   This function will attempt to read the tspec file for a pyramid
 *   that has been stored on a DPSS. You pass in the set ID of the
 *   pyramid for which you want the tspec to be read.
 *
 *   If successful, then the tspec will be parsed and a Pyramid
 *   structure will be returned that contains all of the information
 *   contained in the tspec, otherwise FALSE is returned.
 *
 *   N.B. This function is called by tsmReadTspec_dpss if a DPSS Pyramid 
 *   has no metadata (tspec) URLs associated with it, or if they all
 *   fail to produce a valid tspec file. You should not need to call
 *   this routine directly.
 *
 * @param    connect the dataset connection structure
 *
 * @return   A Pyramid structure containing the tspec info, or NULL if failed
 *
 * @author   Martin Reddy
 * @version  12 January 1998
 *
 * @see      tsmReadTspec_dpss
 *
 */

Pyramid *
tsmReadEmbeddedTspec_dpss( TsmConnectionDpss *connect )
{
  const char    *funcName = "tsmReadEmbeddedTspec_dpss";
  unsigned char *tileBuffer;
  TileHeader    tileHeader;
  AppRequest    ar;
  RequestList   *rl = NULL;
  dsmSetInfo    dsi = connect->set_info;
  Pyramid       *pyramid;
  FILE          *handle;
  char          filename[100];
  int           tiles;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName,
	     "Reading Tspec from DPSS set %d", connect->set_id ));

  /* create a request structure for the tspec block, build a request */
  /* list with this structure, and then send it off to the DPSS. We  */
  /* identify the tspec block with the id [-999,-999,-999,-999].     */

  ar.block_id.id[0] = TSPEC_BLOCK_KEY;
  ar.block_id.id[1] = TSPEC_BLOCK_KEY;
  ar.block_id.id[2] = TSPEC_BLOCK_KEY;
  ar.block_id.id[3] = TSPEC_BLOCK_KEY;
  ar.set_id         = connect->set_id;

  if ( issBuildRequestList( &rl, &ar, 1 ) != ISS_OK ) {
    tsmError( funcName, "Couldn't build tile request list" );
    TSMTRINC(( TSMMASK|3, -1 ));
    return NULL;
  }

  /* We use issSendRequestListSynch to send the request list to the   */
  /* DPSS because this returns the number of tiles that are available */
  /* Because we are only requesting one tile, then this tells us if   */
  /* the block exists on the DPSS, i.e. if the tspec was loaded.      */
  
  tiles = issSendRequestListSynch( connect->issHandle, IdtTile, rl );

  TSMTRACE(( TSMMASK|3, funcName, "issSendRequestListSync returned %d", tiles ));

  if ( tiles == 0 ) {
    TSMTRACE(( TSMMASK|3, funcName, "Tspec file does not exist on the DPSS" ));
    TSMTRINC(( TSMMASK|3, -1 ));
    return NULL;
  } else if ( tiles != 1 ) {
    tsmError( funcName, "Couldn't send request list to DPSS" );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  /* Now we will make a buffer to hold the tspec file in */

  tileBuffer = tsmMemCalloc( dsiBlockSize(dsi) * 1024, sizeof(char) );

  if ( tileBuffer == NULL ) {
    tsmError( funcName, "Couldn't allocate a tile buffer" );
    TSMTRINC(( TSMMASK|3, -1 ));
    return NULL;
  }

  /* get the next tile from the DPSS */

  if ( issReadBlocks( connect->issHandle, IdtTile, &tileHeader,
		      (char **) &tileBuffer, 1 ) != ISS_OK ) {
    tsmError( funcName, "Couldn't read tspec block from DPSS" );
    tsmMemFree( tileBuffer );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  /* Cooool! We have retrieved the tspec from the DPSS... now let's */
  /* check that it actually contains some data and then we can save */
  /* this out to a file and parse it into a Pyramid structure       */

  if ( tileBuffer[0] == '\0' ) {
    tsmError( funcName, "Tspec contains no data or is corrupted" );
    tsmMemFree( tileBuffer );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  if ( ( pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a Pyramid structure" );
    tsmMemFree( tileBuffer );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  sprintf( filename, "/tmp/readtspec_%ld.tmp", (int) getpid() + time(NULL) );
  if ( ( handle = fopen( filename, "w" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open temp file for writing" );
    tsmMemFree( tileBuffer );
    tsmMemFree( pyramid );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  /* now we write out the tspec buffer to the temp file. We can use the */
  /* strlen() function to work out the file size because the tspec file */
  /* should not contain an null-terminator characters, but it will be   */
  /* terminated by one because the memory was allocated with calloc.    */

  fwrite( tileBuffer, 1, strlen((const char *) tileBuffer), handle );
  fclose( handle );

  if ( tspReadTspec( filename, pyramid ) == FALSE ) {
    tsmError( funcName, "Couldn't read the tspec file" );
    tsmMemFree( tileBuffer );
    tsmMemFree( pyramid );
    remove( filename );
    TSMTRINC ((TSMMASK|3, -1));
    return NULL;
  }

  /* all worked just hunky-dorey if we got this far - return the pyramid */

  remove( filename );
  tsmMemFree( tileBuffer );
  tsmEnsureGeoNameSet( pyramid );
  
  TSMTRACE ((TSMMASK|3, funcName, "Returning pyramid %p", pyramid));
  TSMTRINC ((TSMMASK|3, -1));
  return pyramid;
}

/*
 * Synopsis:
 *   Pyramid *tsmWriteTspec_dpss( TsmConnection connect )
 *
 * Description:
 *   This function will write out the tspec structure for a connection
 *   to a DPSS set. Currently this will only write to a block on the
 *   DPSS. In future it might be nice to write out to the metadata
 *   URL of a DPSS if it's tspecs are still stored over http.
 *
 * Returns: TRUE if successful, FALSE if failed
 *
 * Author: Martin Reddy
 *
 * Date: 12 March 1998
 *
 */

int
tsmWriteTspec_dpss( TsmConnectionDpss *connect )
{
  return tsmWriteEmbeddedTspec_dpss( connect );
}

/*
 * Synopsis:
 *   int tsmWriteEmbeddedTspec_dpss( TsmConnection com_connect )
 *
 * Description:
 *   Write a tspec file to a specific block on a DPSS pyramid.
 *
 *   This function will write the tspec file for a dataset to a
 *   single block on a DPSS set. The tspec block written by this
 *   function is given a special block ID that does not conflict with
 *   any valid tile block ID. The standard tsmWriteTile_dpss routine
 *   is used to output the block to the DPSS.
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  13 November 1997
 *
 *
 */

int
tsmWriteEmbeddedTspec_dpss( TsmConnectionDpss *connect )
{
  static const char *funcName = "tsmWriteEmbeddedTspec_dpss";
  TsmConnection     com_connect = connect->aconn.conn;
  FILE              *handle;
  char              temp_file_name[100];
  char              temp_dir_name[100];
  int               temp_dir_code, result;
  uchar             *tileBuffer;

  /* make a temp directory to store the pyramid's tiles.tspec file in */

  temp_dir_code = ( (int) getpid() + (int) time(NULL) ) % 0xFFFF;
  sprintf( temp_dir_name, "/tmp/tspecload_%d", temp_dir_code );
  sprintf( temp_file_name, "%s/tiles.tspec", temp_dir_name );
  tsmMakeDirIfNotExist( temp_dir_name, TSM_PATH_ONLY );

  /* save the tspec file out to the temp directory */

  if ( tspSaveTspec( temp_dir_name, com_connect->tspec ) == FALSE ) {
    tsmError( funcName, "Couldn't write %s", temp_file_name );
    return FALSE;
  }

  /* now allocate a tile buffer to put the tspec file into */

  if ( ( tileBuffer = tsmAllocTileBuffer( com_connect ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a tile buffer for the tspec" );
    return FALSE;
  }

  /* then read the file that we have just created into the tile buffer */

  if ( ( handle = fopen( temp_file_name, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't find the tspec I just wrote out!" );
    return FALSE;
  }

  fread( tileBuffer, 1, com_connect->tileBufferSize, handle );
  fclose( handle );

  remove( temp_file_name );
  remove( temp_dir_name );

  /* and now write out the tile buffer with the tspec file in it */
  /* to the DPSS, using the standard tile loader function above  */

  result = tsmWriteTile_dpss( connect, TSPEC_BLOCK_KEY, TSPEC_BLOCK_KEY,
			      TSPEC_BLOCK_KEY, tileBuffer );

  tsmFreeTileBuffer( tileBuffer );
  return result;
}

#else

Pyramid *
tsmReadTspec_dpss( TsmConnectionDpss *connect, TsmConnectionParams params )
{
  TsmConnection com_connect = &connect->common;
  tsmNotCompiledIn( com_connect->location->protocol, "tsmReadTspec_dpss" );
  return NULL;
}

int
tsmWriteTspec_dpss( TsmConnectionDpss *connect )
{
  TsmConnection com_connect = &connect->common;
  tsmNotCompiledIn( com_connect->location->protocol, "tsmWriteTspec_dpss" );
  return FALSE;
}

#endif
