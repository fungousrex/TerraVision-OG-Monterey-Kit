/*
 * Name: tsmTile_dpss.c
 *
 * Description: 
 *   Contains routines for reading and writing tiles on a DPSS.
 *   These functions are private to the tsmApi.
 *
 *   If DPSS is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   void tsmStartRequestThread_dpss ( TsmConnection connect )
 *   int tsmSendRequestListToDPSS( TsmConnection connect )
 *   int tsmReadNextTile_dpss( TsmConnection com_connect, uchar *tileBuffer )
 *   int tsmWriteTile_dpss( TsmConnection, int, int, int, uchar * )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTile_dpss.c,v 1.27.4.2 2001/07/10 06:17:43 leei Exp $
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
#include <dimage/tload.h>
#include <dimage/image.h>

#include <prerror.h>

/*
 * Synopsis:
 *   void *tsmRequestThread_dpss( TsmConnectionDpss *connect )
 *
 * Description:
 *   This thread is started whenever we open a read connection to
 *   a DPSS (see tsmConnect_dpss). It just performs an infinite
 *   loop of sleeping for a specified time and then requesting
 *   the current tile list from the DPSS. This thread is killed
 *   by tsmDisconnect_dpss.
 *
 *   You can change the sleep timeout with the call
 *   tsmSet( connect, TSM_REQUEST_TIMEOUT, value ).
 *
 * Returns: NULL
 *
 * Author: Martin Reddy
 *
 * Date: 3 May 1999
 *
 */

static void
tsmRequestThread_dpss( void *arg )
{
  const char        *funcName = "tsmRequestThread_dpss";
  TsmConnectionDpss *connect = arg;
  TsmConnection com_connect;

  if ( connect == NULL ) {
    tsmError( funcName, "NULL connection!! Exiting request thread" );
    return;
  }

  /* Show that it is running. */
  connect->requestThreadRunning = TRUE;
  com_connect = connect->aconn.conn;

  while ( TRUE ) {

    /* perform the request operation */

    TSMTRACE(( TSMMASK|1, funcName, "Waking up to request blocks for %s",
	       com_connect->name ));

    tsmSendRequestListToDPSS( connect );

    /* sleep for the specified timeout */

    TSMTRACE(( TSMMASK|1, funcName, "Sleeping %f ms for %s",
	       connect->requestTimeout*1e3, com_connect->name ));

    tsmPause (connect->requestTimeout);

    if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR) {
      /* This is the signal to exit. */
      TSMTRACE ((TSMMASK|1, funcName, "Interrupted request thread"));
      return;
    }
  }
}

/*
 * Synopsis:
 *   void tsmStartRequestThread_dpss( TsmConnectionDpss *connect )
 *
 * Description:
 *   Start the requester thread for a DPSS connection.  If the thread already
 *   exists, then just return.
 *
 * Returns: Nothing
 *
 * Author: Lee Iverson
 *
 * Date: 5 May 1999
 *
 */

void
tsmStartRequestThread_dpss ( TsmConnectionDpss *connect )
{
  static const char funcName[] = "tsmStartRequestThread_dpss";

  if (connect->requestThreadRunning)
    return;

  /* if we have a read connection to a DPSS dataset, then we spawn */
  /* off a requester thread. This thread will wake up every n secs */
  /* and send a request list to the DPSS for the last requested    */
  /* list of tiles.                                                */
  if ( pthr_create( &connect->requestThread, "DPSS Requester", 
        	    tsmRequestThread_dpss, connect ) ) {
    tsmError( funcName, "Unable to spawn DPSS tile requester thread" );
    tsmDisconnect( (TsmConnection) connect );
  }
}

/*
 * Synopsis:
 *   int tsmSendRequestListToDPSS( TsmConnection connect )
 *
 * Description:
 *   Sends a list of tile requests to the DPSS so that it can
 *   start sending us the data. The request list is built using
 *   the tsmTileReqBegin, Add, End functions.
 *
 * Returns: TRUE if all went well, FALSE if an error occurred
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

int
tsmSendRequestListToDPSS( TsmConnectionDpss *connect )
{
  const char        *funcName = "tsmSendRequestListToDPSS";
  TsmConnection     com_connect = connect->aconn.conn;
  TrArray           *trArray;
  TrItem            *trItem;
  TrLists           *trList;
  int               requestName[4];
  int               i, returnCode;

  /* Check that we have a DPSS request list allocated */

  if ( connect->requestList == NULL ) {
    tsmError( funcName, "No request list allocated - this shouldn't happen!" );
    return FALSE;
  }

  /* Reset the request list */

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName, 
	     "Doing issResetRequestList for %s", com_connect->name));

  if ( ( returnCode = issResetRequestList( connect->requestList ) ) != OK ) { 
    tsmError( funcName, "issResetRequestList failed (%d).", returnCode );
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  /* grab the current read list - i.e. last completed request list */

  if ( ( trList = tsmGrabValidReadList( com_connect ) ) == NULL ) {
    TSMTRACE ((TSMMASK|3, funcName, "No requests in list"));
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  if ( trList->trIndex >= trList->trArray->len ) {
    TSMTRACE ((TSMMASK|3, funcName, "Same list as last time - not resending"));
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  PR_Lock (com_connect->requestLock); {

    trArray = trList->trArray;

    /* Loop through each entry in the request list and */
    /* add this to the DPSS request list structure.    */

    for ( i = 0; i < trArray->len; ++i ) {

      trItem  = &trArray->array[i];

      requestName[0] = trItem->ix;
      requestName[1] = trItem->iy;
      requestName[2] = trItem->level;
      requestName[3] = 0;

      tsmLogTile( com_connect, TV_TILE_REQ, trItem->ix, trItem->iy,
		  trItem->level );
      
      TSMTRACE(( TSMMASK|3, funcName,
		 "issAddToRequestList [%d,%d,%d,%d], setId %d, prio %d.",
		 requestName[0], requestName[1], requestName[2],
		 requestName[3], connect->set_id, trItem->priority ));

      returnCode = issAddToRequestList( connect->requestList, requestName,
					connect->set_id, trItem->priority );
      if ( returnCode != OK ) {
	tsmError( funcName, "issAddToRequestList failed." );
	TSMTRINC ((TSMMASK|3, -1));
	return FALSE;
      }
    }

    /* advance the current pointer to show that we've requested these tiles */

    trList->trIndex = trArray->len;

  }; PR_Unlock ( com_connect->requestLock );

  /* Now that we have built the DPSS request list, send it */

  TSMTRACE ((TSMMASK|3, funcName, "Starting issSendRequestList for %d tiles",
	     trArray->len ));

  returnCode = issSendRequestList( connect->issHandle, IdtTile,
				   connect->requestList );
  if (returnCode != OK) {
    tsmError( funcName, "issSendRequestList failed (%d).", returnCode );
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  /* output a TV_REQUEST_SENT NetLogger event if we are logging */

  tsmLog( com_connect, TV_REQUEST_SENT, "TILES=%d", trArray->len );

  /* And that's it! */

  TSMTRACE ((TSMMASK|3, funcName, "succeeded, return code = %d.", returnCode));
  TSMTRINC ((TSMMASK|3, -1));

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmReconnect_dpss( TsmConnection connect )
 *
 * Description:
 *   Attempts to reconnect to a DPSS server. This function spawns
 *   a new thread that continutally tries to reconnect to a DPSS
 *   dataset. When the reconnect has succeeded, then the thread
 *   exits. This function is called whenever we get a SERVER_DIED
 *   return from dpssReadBlocks.
 *
 *   tsmReconnectThread_dpss is the routine that does the work
 *   in the separate thread. tsmReconnect_dpss just spawns a 
 *   new thread to do this work.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 26 January 1999
 *
 */

void
tsmReconnectThread_dpss( void *closure )
{
  const char        *funcName = "tsmReconnectThread_dpss";
  TsmConnectionDpss *connect = (TsmConnectionDpss *) closure;
  TsmConnection     com_connect = connect->aconn.conn;

  TSMTRACE(( TSMMASK|1, funcName, "Reconnecting to: %s",
	     com_connect->location->url ));

  /* try to reconnect to the DPSS server(s) for this connection */

  while ( issReReady( connect->issHandle, connect->set_id ) != ISS_OK ) {

    /* failure: timeout and try again */

    tsmPause (5.0);
    
    if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR) {
      /* This is the signal to exit. */
      TSMTRACE ((TSMMASK|1, funcName, "Interrupted reconnect thread"));
      return;
    }
  }

  if ( issConnectAllRcvStreams( connect->issHandle ) != ISS_OK )
    tsmError( funcName, "failed!!!! issConnectAllRcvStreams\n" );

  /* success: signal that this thread is over and then exit the thread */

  PR_Lock( connect->reconnect_lock ); {
    connect->reconnect_thread = NULL;
  }; PR_Unlock( connect->reconnect_lock );

  TSMTRACE(( TSMMASK|1, funcName, "server back up: %s",
	     com_connect->location->url ));
}

void
tsmReconnect_dpss( TsmConnectionDpss *connect )
{
  const char *funcName = "tsmReconnect_dpss";

  /* obtain lock for looking at connect->reconnect_thread */

  PR_Lock( connect->reconnect_lock ); {

    /* iff no reconnect thread is running, then create one */

    if ( connect->reconnect_thread == NULL )
	if ( pthr_create( &connect->reconnect_thread, "DPSS Reconnect",  
	                  tsmReconnectThread_dpss, connect ) )
	tsmError( funcName, "Unable to spawn DPSS reconnect thread" );

  }; PR_Unlock( connect->reconnect_lock );
}

/*
 * Synopsis:
 *   int tsmReadNextTile_dpss( TsmConnection com_connect, uchar *tileBuffer )
 *
 * Description:
 *   Reads the next tile from a DPSS
 *
 * Returns: TRUE          if all went okay,
 *          FALSE         if an error occurred,
 *          TSM_NODATA    if no data was read,
 *
 * Author: Martin Reddy
 *
 * Date: 13 January 1998
 *
 */

int
tsmReadNextTile_dpss( ActualConnection aconn,
		      TsmTileHeader *tileHeader,
		      uchar *tileBuffer )
{
  const char        *funcName = "tsmReadNextTile_dpss";
  TsmConnectionDpss *connect = (TsmConnectionDpss *) aconn;
  TsmConnection com_connect = aconn->conn;
  TileHeader        dpssHeader;
  int               status;
  TrLists *         trList;

  TSMTRINC(( TSMMASK|3, 1 ));

  /* Get new read list if needed. */
  trList = tsmGrabReadList (com_connect, FALSE);
  if (! com_connect->blocking_read && ! tsmReadListReady (trList))
    return TSM_NODATA;

  while (TRUE) {
    /* If the current connection is for a JPEG dataset, then we need to */
    /* read the tile into a separate buffer and decompress this to the  */
    /* tile buffer. Otherwise, we just read directly into the tile buf. */

    TSMTRACE(( TSMMASK|2, funcName,
	       "Calling issReadBlocks for %s", com_connect->name ));
    if ( com_connect->format == TSM_JPEG_FORMAT ) {

      /* allocate a JPEG decompress buffer, if we've got a jpg connect */
      if (! connect->jpegBuffer)
	connect->jpegBuffer = (uchar *) tsmMemMalloc(com_connect->tileBufferSize);

      TSMTRACE(( TSMMASK|3, funcName, "Reading JPEG tile data..." ));

      status = issReadBlocks( connect->issHandle, IdtTile, &dpssHeader,
			      (char **) &connect->jpegBuffer, 1 );
    
      if ( status == ISS_OK ) {
	TSMTRACE ((TSMMASK|3, funcName, "Decompressing JPEG buffer."));
	if ( tsmJPEGDecompressTile( connect->jpegBuffer, tileBuffer, 
				    &com_connect->tspec->tileTspec ) == FALSE ) {
	  tsmError( funcName, "Couldn't decompress JPEG tile" );
	  TSMTRINC ((TSMMASK|3, -1));
	  return FALSE;
	}
      }

    } else 
      status = issReadBlocks( connect->issHandle, IdtTile, &dpssHeader,
			      (char **) &tileBuffer, 1 );

#if 0
    /* XXX - GRUESOME HACK ALERT. Remove this when issReadBlocks */
    /* returns suitable error codes during a server down fault.  */

    if ( status == ISS_OK ) {
      int res = dpssHeader.ar.block_id.id[2];
      SimpleTileSet *tset = &connect->tspec->tspecLevel.val[res];
      if ( res < connect->tspec->minLevel ||
	   res > connect->tspec->maxLevel ||
	   dpssHeader.ar.block_id.id[0] < tset->minTile[0] ||
	   dpssHeader.ar.block_id.id[1] < tset->minTile[1] ||
	   dpssHeader.ar.block_id.id[0] > tset->maxTile[0] ||
	   dpssHeader.ar.block_id.id[1] > tset->maxTile[1] ||
	   dpssHeader.size == 0 ||
	   dpssHeader.size > connect->tileBufferSize ) {
	fprintf( stderr, "Forcing a SERVER_DIED instead of %d for [%d %d %d]\n",
		 status, dpssHeader.ar.block_id.id[0],
		 dpssHeader.ar.block_id.id[1], res );
	fprintf( stderr, "minRes = %f, maxRes = %f, x = %d..%d, y = %d..%d,"
		 " size = %d (%d)\n", connect->tspec->minResolution,
		 connect->tspec->maxResolution, tset->minTile[0], 
		 tset->maxTile[0],  tset->minTile[1],  tset->maxTile[1],
		 dpssHeader.size, connect->tileBufferSize );
	       
	status = SERVER_DIED;
      }
    }
#endif

    /* A good tile.  Break out of the loop and return. */
    if ( status == ISS_OK )
      break;

    /* After a timeout, check for interrupt.  Otherwise just try again. */
    if ( status == ISS_LIB_TIMEOUT) {
      TSMTRACE ((TSMMASK|2, funcName, "Timeout..."));
      if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
	PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
	TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	TSMTRINC(( TSMMASK|3, -1 ));
	return TSM_NODATA;
      }
      continue;
    }

    /* Try to reconnect if server down... */
    if ( status == SERVER_DIED ) {
      TSMTRACE(( TSMMASK|1, funcName, "DPSS server died: %s", com_connect->name ));
      tsmError( funcName, "Server died - trying to reconnect..." );
      tsmReconnect_dpss( connect );
    } else 
      tsmError( funcName, "issReadBlocks failed, returned %d", status );

    TSMTRINC(( TSMMASK|3, -1 ));
    return TSM_NODATA;
  }

  /* increment the number of tiles that we have read in.. */

  ++trList->trIndex;

  /* If no problems, then map the DPSS tile header into the tsmApi header */

  tileHeader->tx         = dpssHeader.ar.block_id.id[0];
  tileHeader->ty         = dpssHeader.ar.block_id.id[1];
  tileHeader->level      = dpssHeader.ar.block_id.id[2];
  tileHeader->pad        = dpssHeader.ar.block_id.id[3];
  tileHeader->set_id     = dpssHeader.ar.set_id;
  tileHeader->session_id = dpssHeader.ar.session_id;
  tileHeader->tile_id    = dpssHeader.ar.tile_id;
  tileHeader->size       = dpssHeader.size;
  tileHeader->checksum   = dpssHeader.check_sum;
  tileHeader->type       = com_connect->dataset_type;
  tileHeader->connect    = com_connect;

  TSMTRACE(( TSMMASK|3, funcName, "Done." ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *    BlockMap *tsmUpdateBlockTable( BlockMap *old_map, BlockMap *new_map )
 *
 * Description:
 *     sets loaded flag of all entries from old_map into new_map,
 *     and returns the updated new map, or NULL if an error occurred.
 *
 *     This function is used to create a copy of the block table for
 *     a dataset that has already been created, but which we want to
 *     continue adding blocks to.
 *
 *     N.B. if the define DPSS_CONT_LOAD is not set, then this function
 *     will just return NULL. This is to allow the library to still
 *     compile under the Irix 5.3 version of the DPSS which doesn't have
 *     the required library calls in it for continuation loads.
 *
 * Externals: None
 *
 * Returns: the updated BlockMap
 *
 */

BlockMap *
tsmUpdateBlockTable( BlockMap *old_map, BlockMap *new_map )
{
  const char    *funcName = "tsmUpdateBlockTable";
#ifndef DPSS_CONT_LOAD
  tsmError( funcName, "DPSS continution load support not available" );
  return NULL;
#else
  block_info *ns = NULL;
  int        nlocs;
  lu_key     luk;
  DB         *dbp = old_map->dbt;
  DBC        *dbcp = NULL;

  /* get the DB cursor first of all */

  if ( ( dbcp = iss_db_init_cursor( dbp ) ) == NULL ) {
    tsmError( funcName, "iss_db_init_cursor failed" );
    return FALSE;
  }

  /* loop over all records in the old map */

  while ( ( ns = iss_db_get_next_block( dbp, &luk, &nlocs, dbcp ) ) != NULL ) {

    /* this routine updates records with checksum, size, and sets */
    /* the "stored" flag                                          */
    
    if ( iss_db_update( new_map->dbt, &luk, ns->checksum, ns->size ) < 0 ) {
      iss_db_close_cursor( dbcp );
      return NULL;
    }
      
    free( ns );
  }

  iss_db_close_cursor( dbcp );
  return new_map;
#endif
}

/*
 * Synopsis:
 *   int tsmCalcDPSSBlockSize( TsmConnectionDpss *connect )
 *
 * Description:
 *   Calculate the size of a single DPSS block to store a single
 *   TV tile. This can varying depending upon whether the tile
 *   is to be JPEG compressed, and also whether the user has 
 *   specified a block size that they want to use.
 *
 * Returns: a block size in KB (not bytes!)
 *
 * Author: Martin Reddy
 *
 * Date: 12 March 1998
 *
 */

int
tsmCalcDPSSBlockSize( TsmConnectionDpss *connect )
{
  TsmConnection com_connect = connect->aconn.conn;
  int block_size = com_connect->tileBufferSize;

  /* JPEG block sizes can be a lot smaller than raw ones. */
  /* Here we choose a conservative size of 20% of the raw */
  /* size - that is the raw block size divided by 5.      */
  
  if ( com_connect->format == TSM_JPEG_FORMAT )
    block_size /= 5;

  /* Return the block size in units of KB for the DPSS, not bytes */

  return block_size / 1024;
}

/*
 * Synopsis:
 *   int tsmCalcDPSSSetSize( TsmConnectionDpss *connect )
 *
 * Description:
 *   Work out the size of the entire dataset on the DPSS (in KB).
 *   This includes one extra block to be used to store the pyramid's
 *   tspec file.
 *
 * Returns: the DPSS set size in KB (not bytes!)
 *
 * Author: Martin Reddy
 *
 * Date: 12 March 1998
 *
 */

int
tsmCalcDPSSSetSize( TsmConnectionDpss *connect )
{
  TsmConnection com_connect = connect->aconn.conn;
  int numtiles = tsmTotalNumTiles( com_connect->tspec, connect->replicate_mode );

  return ( numtiles + 1 ) * tsmCalcDPSSBlockSize( connect );
}

/*
 * Synopsis:
 *    BlockMap *tsmGetExistingBlockMap( IssHandle * ih_p, dsmSession dsm,
 *                   int dsid, IssAllocInfo * bl_alloc_info, Pyramid * tspec,
 *                   int replicate_mode )
 *
 * Description:
 *    create block map for continuing a partial load to the DPSS
 *
 *     N.B. if the define NODPSSCONTLOAD is set, then this function will
 *     just return NULL. This is to allow the library to still compile
 *     under the Irix 5.3 version of the DPSS which doesn't have the
 *     required library calls in it for continuation loads.
 *
 * Externals: None
 *
 * Returns: block map for pyramid, or NULL if failed
 *
 */

BlockMap *
tsmGetExistingBlockMap( TsmConnectionDpss *connect )
{
  const char    *funcName = "tsmGetExistingBlockMap";
#ifndef DPSS_CONT_LOAD
  tsmError( funcName, "DPSS continution load support not available" );
  return NULL;
#else
  TsmConnection com_connect = connect->aconn.conn;
  dsmSetInfo    dsm_info;
  DB            *dbp = NULL;
  BlockMap      *block_table = NULL, *new_block_table = NULL;
  dpssBlockList *pl_info = NULL;
  int           num_blocks, set_size, block_size, setid;

  /* calculate the set size for the new pyramid (in KB) */

  block_size = tsmCalcDPSSBlockSize( connect );
  set_size   = tsmCalcDPSSSetSize( connect );
  setid      = connect->set_id;

  if ( ( dsm_info = dsmFetch( connect->dsmHandle, setid ) ) == FALSE ) {
    tsmError( funcName, "Error getting dsm_info for set %d", connect->set_id );
    return NULL;
  }

  /* only continue of load flag = DSMLOADINPROGRESS. */

  if ( dsiCheckLoadFlag( dsm_info, DSMLOADINPROGRESS ) == FALSE ) {
    tsmError( funcName, "DSM load state not DSMLOADINPROGRESS" );
    return NULL;
  }

  /* for now, make sure set size is the same. Later allow set to grow */

  if ( dsiTotalSize( dsm_info ) != set_size ||
       dsiBlockSize( dsm_info ) != block_size ) {
    tsmError( funcName, "Cannot change the size of an existing set (yet)" );
    return NULL;
  }

  /* getting previous lookup from DPSS... */

  if ( ( block_table = issGetLookup( connect->issHandle, setid ) ) == NULL ) {
    tsmError( funcName, "Error in issGetLookup.");
    return NULL;
  }

  /* get previously allocated block list from DPSS, i.e. what was */
  /* returned by issGetFreeBlocks in the previous load            */

  if ( ( pl_info = issGetPlacementInfo(connect->issHandle, setid) ) == NULL ) {
    tsmError( funcName, "Error in issGetPlacementInfo." );
    return NULL;
  }

  num_blocks = dsiTotalSize( dsm_info ) / dsiBlockSize( dsm_info );

  if ( ( dbp = iss_db_open( NULL, DB_HASH, num_blocks ) ) == NULL ) {
    tsmError( funcName, "error opening internal hash database" );
    return NULL;
  }

  new_block_table = issBuildTilePlacementMapRR( dbp, dsm_info,
						com_connect->tspec, pl_info,
						connect->replicate_mode );

  if ( new_block_table == NULL )
    return NULL;

  /* next, for every entry in new_block_table, need to check    */
  /* old_block_table to see if already loaded, and send if not. */

  block_table = tsmUpdateBlockTable( block_table, new_block_table );

  /* this normally done during AllocBlocks(), so in continuation */
  /* load do it here                                             */

  if ( issSetScribeAddrs( connect->issHandle, connect->alloc_info ) != OK ) {
    tsmError( funcName, "error determining scribe addresses" );
    return NULL;
  }
  
  return block_table;
#endif
}

/*
 * Synopsis:
 *   int tsmInitDPSSDataset( TsmConnectionDpss *connect, TsmConnectionParams )
 *
 * Description:
 *   We call this during the tsmConnect phase. It basically just checks
 *   that we have a servers array specified, and then tries to work out
 *   a set id for the (possibly) new DPSS set. This is done as follows:
 *   If the DPSS set has been referenced using a set ID for it's name
 *   (e.g. x-dpss://host/Pyramids/101) then this is assumed to be a set
 *   that was previously started to be loaded, but not finished, and so
 *   we will use that set id and continue the load. Otherwise, we assume
 *   the user is generating a new set from scratch and always generate a
 *   new ID.
 *
 * Returns: TRUE if all went well, FALSE if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 11 March 1998
 *
 */

int
tsmInitDPSSDataset( TsmConnectionDpss *connect, TsmConnectionParams params )
{
  static const char *funcName = "tsmInitDPSSDataset";
  TsmConnection     com_connect = connect->aconn.conn;
  dsmSetInfo        dsm_info;
  int               id = 0;

  /* check that the user has specified a DPSS server combination */

  if ( connect->numServers == 0 ) {
    tsmError( funcName, "You must specify a server combination for the set" );
    return FALSE;
  }

  /* Get a new set ID for the pyramid, if not already allocated. */
  /* record whether this set has been newly created or not. We   */
  /* do this because we need to allocate a block map if we are   */
  /* creating a new set from scratch.                            */

  if ( ( id = atoi( com_connect->name ) ) == 0 )
    if ( tsmIsParamSet( params, TSM_PARAM_SETID ) == TRUE )
      id = params->set_id;

  if ( id != 0 ) {

    /* Check to see if the continuation set actually exists */

    if ( ( dsm_info = dsmFetch( connect->dsmHandle, id ) ) == FALSE ) {
      tsmError( funcName, "Set %d doens't exist! Cannot continue load", id );
      return FALSE;
    }
    connect->new_dataset = FALSE;

  } else {

    /* We are creating a new dataset on the DPSS... */

    connect->set_id = dsmGetNewId( connect->dsmHandle );
    connect->new_dataset = TRUE;
  }

  /* Set the tiles_loaded count to zero. This flag is used to build  */
  /* the block map on the first tile write to the DPSS. We defer the */
  /* block map building until then because we probably don't have a  */
  /* valid tspec right now so we can't calculate a block size.       */

  connect->tiles_loaded = 0;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmPrepareDPSSDataset( TsmConnection com_connect )
 *
 * Description:
 *   Prepare a DPSS for writing out tiles to a pyramid.
 *
 *   This function will set up a DPSS to receive tiles for a pyramid.
 *   If the pyramid does not already exist on the DPSS, then a new set
 *   will be created for it and a block table allocated to store all
 *   of the tiles in the pyramid. This function is called by
 *   tsmWriteTile_dpss the first time it tries to write a tile to the DPSS.
 *
 *   (N.B. the function actually allocates enough blocks for all of the
 *   tiles in a pyramid, plus one extra block to (optionally) hold the
 *   tspec file for that pyramid.)
 *
 * @return   TRUE if all went well, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  12 March 1998
 *
 */

int
tsmPrepareDPSSDataset( TsmConnectionDpss *connect )
{
  static const char *funcName = "tsmPrepareDPSSDataset";
  TsmConnection     com_connect = connect->aconn.conn;
  IssHandle         *iss_handle;
  dsmSession        dsm_handle;
  dsmSetInfo        dsm_info;
  char              description[1024], *name;
  int               set_size, block_size, i;

  iss_handle = connect->issHandle;
  dsm_handle = connect->dsmHandle;
  name       = com_connect->name;
  block_size = tsmCalcDPSSBlockSize( connect );
  set_size   = tsmCalcDPSSSetSize( connect );

  /* specify where to load the data using a issBuildAllocInfo*() func */
  /* NB currently it is hardcoded to use "sri-server1" as the server  */
  /* list - this should be configurable somehow                       */

  connect->alloc_info = issBuildAllocInfoFromServerNames( iss_handle,
							  connect->server,
							  connect->numServers,
							  connect->set_id,
							  set_size );

  if ( connect->alloc_info == NULL ) {
    tsmError( funcName, "could not create DPSS AllocInfo structure" );
    return FALSE;
  }

  /* Now create the new DSM entry */

  sprintf( description, "type: %s, subtype: %s\ndate: %s, done by: %s\n",
	   tspStringFromSymbol( com_connect->tspec->identityTspec.type ),
	   tspStringFromSymbol( com_connect->tspec->identityTspec.subtype ),
	   com_connect->tspec->identityTspec.operationDate,
	   com_connect->tspec->identityTspec.operationDoneBy );

  dsm_info = dsmCreateNewPyramidSet( name, description, connect->set_id,
				     set_size, block_size,
				     com_connect->tspec );

  if ( dsm_info == NULL ) {
    tsmError( funcName, "could not create DSM entry" );
    return FALSE;
  }

  /* Check that we have enough room to store the new list of metadata URLs */

  if ( dsm_info->metadata_url.metadata_url_len < connect->numMetadata ) {
    if ( dsm_info->metadata_url.metadata_url_val != NULL )
      free( dsm_info->metadata_url.metadata_url_val );
    dsm_info->metadata_url.metadata_url_val = (char **)
      tsmMemManagerMalloc( connect->numMetadata * sizeof(char *) );
  }
  dsm_info->metadata_url.metadata_url_len = connect->numMetadata;

  /* fill in the metadata information for the tspecs + compression value */

  for ( i = 0; i < connect->numMetadata; ++i ) {
    strcpy( description, connect->metadata[i] );
    if ( description[strlen(description)-1] != '/' )
      strcat( description, "/" );
    strcat( description, "Pyramids/" );
    strcat( description, name );

    dsiMetadataN( dsm_info, i ) = tsmMemManagerStrdup( description );
  }

  if ( com_connect->format == TSM_JPEG_FORMAT ) 
    dsiCompression( dsm_info ) = ISS_JPEG;
  else
    dsiCompression( dsm_info ) = ISS_UNCOMPRESSED;

#ifdef dsiSetReplicated   /* defined for DPSS 2.4.4, but not for 2.4.3 */
  dsiSetReplicated( dsm_info, connect->replicate_mode );
#endif

  if ( dsmAddSet( dsm_handle, connect->set_id, dsm_info ) < 0 ) {
    tsmError( funcName, "could not add metadata to DSM entry" );
    return FALSE;
  }

  /* Tell the DSM that a load is in progress... */

  if ( dsmSetLoading( dsm_handle, connect->set_id, DSMLOADINPROGRESS ) < 0 ) {
    tsmError( funcName, "failed sending DSMLOADINPROGRESS to DSM" );
    return FALSE;
  }

  /* If we did not create the pyramid set id this time round, then we  */
  /* must have already allocated the block map, so we don't do that    */
  /* again. However we did want to go this far in order to potentially */
  /* update any metadata information.                                  */

  if ( connect->new_dataset == TRUE ) {

    /* Now we ask the DPSS for enough space to store the entire pyramid */
  
    connect->block_table = issGetPyramidBlockMap( iss_handle, dsm_info,
						  com_connect->tspec,
						  connect->alloc_info,
						  connect->replicate_mode,
						  connect->pl_method );

    if ( connect->block_table == NULL ) {
      tsmError( funcName, "couldn't build block map - is DPSS full?" );
      return FALSE;
    }

    connect->new_dataset = FALSE;

  } else
    connect->block_table = tsmGetExistingBlockMap( connect );

  /* Connect the send streams to get the scribes ready for writing */
  
  if ( issConnectAllSendStreams( iss_handle ) < 0 ) {
    tsmError( funcName, "Error connecting Send Streams" );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteTile_dpss( TsmConnection, int, int, int, uchar * )
 *
 * Description:
 *   Write a single tile to a DPSS pyramid.
 *
 *   This function will write a single tile buffer to a DPSS pyramid.
 *   On the first call to this function, a new dataset will be created
 *   on the DPSS which is large enough to hold the entire dataset.
 *   Subsequent calls will just fill in more blocks on this set.
 *
 * @return   TRUE if tile written okay, FALSE if the operation failed
 *
 * @author   Martin Reddy
 * @version  12 March 1998
 *
 */

static int
tsmWriteTileActual_dpss( ActualConnection aconn, int tx, int ty, int level,
			 uchar *tileBuffer )
{
  static const char *funcName = "tsmWriteTile_dpss";
  TsmConnection     com_connect = aconn->conn;
  TsmConnectionDpss *connect = (TsmConnectionDpss *) aconn;
  IssHandle         *iss_handle;
  int               block_id[4], block_size;
  int               quality, jpeg_size;
  uchar             *buffer;

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName,
	     "Loading tile (%d,%d) level %d", tx, ty, level));

  /* If this is the first tile being written to the DPSS then it's */
  /* time to create a block map (if there is not one already made) */

  if ( connect->tiles_loaded == 0 ) {
    if ( tsmPrepareDPSSDataset( connect ) == FALSE ) {
      tsmError( funcName, "Couldn't prepare DPSS for writing!" );
      TSMTRINC(( TSMMASK|3, -1 ));
      return FALSE;
    }
  }

  iss_handle  = connect->issHandle;
  block_size  = tsmCalcDPSSBlockSize( connect ) * 1024;  /* in bytes */

  /* work out the block size - smaller if JPEG - and compress the tile */
  /* to a JPEG stream if we are supposed to be writting out JPEG data  */

  if ( com_connect->format == TSM_JPEG_FORMAT ) {
    if (! connect->jpegBuffer)
      connect->jpegBuffer = (uchar *) tsmMemMalloc(com_connect->tileBufferSize);

    buffer = connect->jpegBuffer;

    /* we try to compress the tile to within a target byte size. If a  */
    /* given compression produces a jpeg file too large, we decrease   */
    /* the quality level by 5% and try again. This is done to ensure   */
    /* that we fit the data within a DPSS block. This shouldn't happen */
    /* very often - if at all.                                         */

    quality = com_connect->jpeg_quality;

    do {
      jpeg_size = tsmJPEGCompressTile( (unsigned char *) tileBuffer,
				       buffer,
				       &com_connect->tspec->tileTspec,
				       quality );

      if ( jpeg_size == 0 ) {
	tsmError( funcName, "Couldn't JPEG compress tile" );
	TSMTRINC ((TSMMASK|3, -1));
	return FALSE;
      } else if ( jpeg_size > block_size ) {
	quality -= 5;
	TSMTRACE ((TSMMASK|3, funcName,
		   "Reducing JPEG Quality to %d", quality ));
      }

    } while ( jpeg_size > block_size );

  } else
    buffer = (uchar *) tileBuffer;   /* just use the tileBuffer as-is */

  /* This should never happen, but just in case.... */

  if ( connect->set_id < 0 || connect->block_table == NULL ) {
    tsmError( funcName, "Woooh!!! Something really went wrong here!!" );
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  /* okay, so now let's set up a block_id structure for the DPSS to  */
  /* give it the coordinates and level of the tile to be loaded...   */
  /* We currently don't use the 4th value of the block ID, except    */
  /* in the case of the tspec block, where it equals TSPEC_BLOCK_KEY */

  block_id[0] = tx;
  block_id[1] = ty;
  block_id[2] = level;
  block_id[3] = ( tx == TSPEC_BLOCK_KEY ) ? TSPEC_BLOCK_KEY : 0;

  /* Now send the tile to the DPSS.... */

  if ( issSendBlock( iss_handle, IdtTile, connect->block_table, block_id,
		     (char *) buffer, block_size ) != ISS_OK ) {
    tsmError( funcName, "DPSS send block failed (%d,%d,%d)", tx, ty, level );
    TSMTRINC ((TSMMASK|3, -1));
    return FALSE;
  }

  /* All finished: it worked!!! */

  connect->tiles_loaded++;

  TSMTRACE ((TSMMASK|3, funcName, "DPSS Load Successful" ));
  TSMTRINC ((TSMMASK|3, -1));
  return TRUE;
}

int
tsmWriteTile_dpss ( TsmConnection connect,
		    int tx, int ty, int level,
		    uchar *tileBuffer )
{
  const char *funcName = "tsmWriteTile_dpss";
  int        status;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_WRITE_ONLY, TRUE ) )
    return FALSE;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p, tile = [%d %d %d]",
	     connect, tx, ty, level ));

  /* write the tile in a protocol-specific manner. */

  status = tsmWriteTileActual_dpss (connect->writer,
				    tx, ty, level, tileBuffer );

  if ( status )
    tsmLogTile( connect, TV_TILE_WRITE, tx, ty, level );

  if (! status)
    TSMTRACE(( TSMMASK|2, funcName, "Done (return code = %d).", status ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return status;
}

/*
 * Synopsis:
 *  int tsmFinishDPSSLoad( TsmConnection com_connect )
 *
 * Description:
 *   Tell the DPSS and DSM that a load has finished.
 *
 *   Once you have finished loading a set of tiles to a DPSS, then you
 *   should use this function in order to tell the DSM and DPSS that
 *   the set has been loaded. This function will do nothing if no tiles
 *   have yet been output to the dataset.
 *
 *   If the TSM_FINISH_LOAD_ON_DISCONNECT setting is FALSE, then we will
 *   not perform the close down. Thus allowing the user to continue loading
 *   tiles during another session.
 *
 *   Tiles are loaded to the DPSS using the tsmWriteTile_dpss function,
 *   which will automatically allocated space with tsmPrepareDPSSDataset.
 * 
 * @author   Martin Reddy
 * @version  12 March 1998
 *
 */

int
tsmFinishDPSSLoad( TsmConnectionDpss *connect )
{
  static const char *funcName = "tsmFinishDPSSLoad";

  /* Don't close the DPSS set if we want to do a continuation load */

  if ( connect->close_on_disconnect == FALSE || connect->tiles_loaded == 0 )
    return TRUE;

  TSMTRACE(( TSMMASK|3, funcName, "Closing DSM/DPSS, %d tiles were loaded",
	     connect->tiles_loaded ));

  /* Otherwise tell the DSM and the DPSS that we're done */

  issLoadFinished( connect->issHandle, connect->set_id );

  if ( dsmSetLoading( connect->dsmHandle, connect->set_id, DSMLOADED ) < 0 ) {
    tsmError( funcName, "failed sending DMSLOADED to DSM" ); 
    return FALSE;
  }

  return TRUE;
}

#else

void
tsmStartRequestThread_dpss ( TsmConnectionDpss *connect )
{
  TsmConnection com_connect = &connect->common;
  tsmNotCompiledIn( com_connect->location->protocol, "tsmStartRequestThread_dpss" );
}

int
tsmSendRequestListToDPSS( TsmConnectionDpss *connect )
{
  TsmConnection com_connect = &connect->common;
  tsmNotCompiledIn( com_connect->location->protocol, "tsmSendRequestListToDPSS" );
  return FALSE;
}

int
tsmReadNextTile_dpss( TsmConnection connect, TsmTileHeader *tileHeader,
		      uchar *tileBuffer )
{
  tsmNotCompiledIn( connect->location->protocol, "tsmReadNextTile_dpss" );
  return FALSE;
}

int
tsmWriteTile_dpss( TsmConnection com_connect, int tx, int ty, int level,
		   uchar *tileBuffer )
{
  tsmNotCompiledIn( com_connect->location->protocol, "tsmWriteTile_dpss" );
  return FALSE;
}

#endif
