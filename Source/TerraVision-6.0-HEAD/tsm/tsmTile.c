/*
 * Name: tsmTile.c
 *
 * Description: 
 *   Contains user-level routines for reading and writing tile data.
 *   These functions are externally visible to the user.
 *
 * Function List:
 *   int tsmReadTile( TsmConnection connect, int tx, int ty, int lev, uchar * )
 *   int tsmReadNextTile( TsmConnection connect, TsmTileHeader *, uchar * )
 *   void tsmTileReqBegin( TsmConnection connect )
 *   int tsmTileReqAdd( TsmConnection connect, int tx, int ty, int, int )
 *   int tsmTileReqEnd( TsmConnection connect )
 *   tsmRequestList * tsmTileRequestList( TsmConnection connect )
 *   int tsmWriteTile( TsmConnection connect, int tx, int ty, int lev, uchar *)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
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
#include "tsmRequestSrv.h"

typedef struct {
  PRUint16 x, y, res;
  PRUintn received:1;
  PRUintn status:1;
  uchar *buf;
  PRLock *lock;
  PRCondVar *ready;
} ReceiveClosure;

static int
receiveTile (tsmRequestList *req,
	     TsmTileHeader *tileHeader,
	     uchar *tileBuffer,
	     void *cl)
{
  static char const funcName[] = "receiveTile";
  ReceiveClosure *rcv = cl;
  TSMTRACE(( TSMMASK|2, funcName, "%p[%d %d %d] %p",
	     req, tileHeader->tx, tileHeader->ty, tileHeader->level,
	     tileBuffer));

  if (rcv->x == tileHeader->tx
      && rcv->y == tileHeader->ty
      && rcv->res == tileHeader->level) {

    PR_Lock (rcv->lock); {
      rcv->buf = tileBuffer;
      rcv->status = !!tileBuffer;
      rcv->received = TRUE;
      TSMTRACE(( TSMMASK|2, funcName, "notify (%p, %d)",
		 tileBuffer, rcv->status));
      PR_NotifyCondVar (rcv->ready);
    }; PR_Unlock (rcv->lock);

    /* Only notify me once. */
    tsmRequestNotifyReceive (req, 0, 0);
    return TRUE;
  } else {
    return FALSE;
  }
}

/*
 * Synopsis:
 *    int tsmAllocReadTile( TsmConnection connect, int tx, int ty, int lev,
 *                          uchar **buffer )
 *
 * Doc++ Description: *//** 
 *    Gets a single tile from a dataset.
 *
 *    This function performs the same action as tsmReadTile, except that
 *    it allocates the memory for the tile buffer inside the function.
 *    You must therefore pass a pointer to the tile buffer pointer, so that
 *    the allocated memory address can be filled in for you. You free the
 *    memory that has been returned using the tsmFreeTileBuffer function
 *    as usual.
 *
 *    Using this function allows for future internal extensions to tsmApi
 *    that can efficiently cache a number of tile buffers for you to use
 *    in a threaded application.
 *
 *    You can set the TSM_DONT_READ_TILES setting to TRUE using tsmSet
 *    and this function will perform no disk or network access. This may
 *    be useful for assessing your application's computational overhead.
 *
 * @param    connect the dataset connection to read from.
 * @param    tx      the x-coordinate of the tile to retrieve.
 * @param    ty      the y-ccordinate of the tile to retrieve.
 * @param    level   the level of the pyramid to retrieve from.
 * @param    buffer  a pointer to a pointer which is filled in by the function
 *
 * @return   TRUE if successful, FALSE if error occurred (buffer also == NULL).
 *
 * @author   Martin Reddy
 * @version  5 March 1998
 *
 * @see      tsmReadTile, tsmFreeTileBuffer
 *
 */

int
tsmAllocReadTile( TsmConnection connect, int tx, int ty, int lev,
		  uchar **buffer )
{
  const char    *funcName = "tsmAllocReadTile";
  ReceiveClosure rcv;
  int status;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, TRUE ) )
    return FALSE;
  
  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p [%d %d %d]",
	     connect, tx, ty, lev ));

  /* Build a request list for a single tile */

  tsmTileReqBegin( connect );

  if ( tsmTileReqAdd( connect, tx, ty, lev, TSM_NOW ) == FALSE ) {
    tsmError( funcName, "Couldn't request tile (%d, %d, %d) - "
	      "probably not in dataset", tx, ty, lev );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* Set up for receiver callback. */
  rcv.x = tx; rcv.y = ty; rcv.res = lev;
  rcv.lock = PR_NewLock ();
  rcv.ready = PR_NewCondVar (rcv.lock);
  rcv.buf = 0;
  rcv.status = FALSE;
  rcv.received = FALSE;
  tsmRequestNotifyReceive (connect->globalReqs, receiveTile, &rcv);

  /* The tsmTileReqEnd() function also sends the request */
  /* list to the server, if appropriate (i.e. DPSS).     */

  if ( tsmTileReqEnd( connect ) == FALSE ) {
    tsmError( funcName, "Couldn't send tile request list" );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* Wait for the tile requested to be received. */
  PR_Lock (rcv.lock); {
    PRIntervalTime timeout = PR_SecondsToInterval (10);
    while (! rcv.received) {
      TSMTRACE(( TSMMASK|2, funcName, "wait"));
      if (PR_WaitCondVar (rcv.ready, timeout) != PR_SUCCESS) {
	TSMTRACE ((TSMMASK|1, funcName, "Break"));
	status = FALSE;
	goto exit;
      }
    }
    status = rcv.status;
  }; PR_Unlock (rcv.lock);

  if (status == FALSE ) {
    tsmError( funcName, "Couldn't read tile (%d, %d, %d) of dataset %s",
	      tx, ty, lev, (char *) tsmGet( connect, TSM_DATASET_URL ) );
  } else if (status == TRUE) {
    /* Copy into destination */
    *buffer = rcv.buf;
    TSMTRACE(( TSMMASK|2, funcName, "Successful return." ));
  } else {
    TSMTRACE(( TSMMASK|2, funcName, "Return status %d.", status ));
  }

 exit:;

  /* Clean up locking */
  tsmRequestNotifyReceive (connect->globalReqs, 0, 0);
  PR_DestroyCondVar (rcv.ready);
  PR_DestroyLock (rcv.lock);

  TSMTRINC(( TSMMASK|2, -1 ));
  return status;
}

/*
 * Synopsis:
 *    int tsmReadTile( TsmConnection connect, int tx, int ty, int lev,
 *                     uchar *buffer )
 *
 * Doc++ Description: *//** 
 *      Gets a single tile from a dataset.
 *
 *      The tsmReadTile function retrieves a single tile from the dataset
 *      with a tile index ( tx, ty, level ). The buffer memory is assumed
 *      to be sufficiently large to hold the entire tile. N.B. This
 *      function is NOT recommended when accessing mulitple tiles from a
 *      DPSS because it has all the overhead of building and transmitting 
 *      a request list for each tile. Instead, use the request list mechanism.
 *
 *    You can set the TSM_DONT_READ_TILES setting to TRUE using tsmSet
 *    and this function will perform no disk or network access. This may
 *    be useful for assessing your application's computational overhead.
 *
 * @param    connect the dataset connection to read from.
 * @param    tx      the x-coordinate of the tile to retrieve.
 * @param    ty      the y-ccordinate of the tile to retrieve.
 * @param    level   the level of the pyramid to retrieve from.
 * @param    buffer  a buffer to write the tile data into.
 *
 * @return   TRUE if successful, FALSE if error occurred.
 *
 * @author   Martin Reddy
 * @version  13 January 1998
 *
 * @see      tsmWriteTile, tsmConnect, tsmSet, tsmMakeTileBuffer
 *
 */

int
tsmReadTile( TsmConnection connect, int tx, int ty, int lev,
	     uchar *buffer )
{
  static const char *funcName = "tsmReadTile";
  uchar *ret_buf = 0;
  int ret;

  /* Try to read in the tile to this buffer */
  ret = tsmAllocReadTile( connect, tx, ty, lev, &ret_buf );
  if ( ret != TRUE ) {
    tsmError( funcName, "Couldn't read the tile" );
    return ret;
  }

  /* Now copy into the destination buffer */
  memcpy (buffer, ret_buf, connect->tileBufferSize);
  tsmFreeTileBuffer (ret_buf);

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmReadNextTile( TsmConnection connect, TsmTileHeader *tileHeader, 
 *                        uchar *tileBuffer )
 *
 * Doc++ Description: *//** 
 *   Read the next tile from server.
 *
 *   Once you have built a tile request list and sent it to the server,
 *   you can then use this function to read in each tile as it is
 *   served to you. Note that these may not be received in the same
 *   order that you requested them so you should always check the tile
 *   header to find out which tile you have received.
 *
 *    You can set the TSM_DONT_READ_TILES setting to TRUE using tsmSet
 *    and this function will perform no disk or network access. This may
 *    be useful for assessing your application's computational overhead.
 *
 *   If you have started logging using the tsmBeginLogging call, then
 *   this function will automatically generate a TV_TILE_READ event to
 *   the log file for the requested tile.
 *
 *   If the specified connection is an http dataset, then there must
 *   be a /cgi-bin/get_http_tile CGI program available on that server.
 *   tsmApi will send a request to this CGI script for the specified tile.
 *
 *   By default, this function will return a value of TSM_NODATA if
 *   there are no more tiles to be read. However, if you specified the
 *   TSM_PARAMS_BLOCKING_READ connection parameter to tsmConnect, then
 *   this function will block until a new tile has been requested in
 *   another thread. That is, if you have requested blocking reads, then
 *   you should never get a TSM_NODATA return code.
 *
 *   This function will set the tile_id field of the passed tile header
 *   so that it is unique for the specified (x,y,level) tuplet. This
 *   will be unique for 0 <= levels < 64, 0 <= x < 4095, 0 <= y < 4095.
 *
 * @param    connect    the dataset connection structure
 * @param    tileHeader the allocated header to write the tile data into
 * @param    tileBuffer the allocated buffer to write the tile data into
 *
 * @return   TRUE if all went well, FALSE if something nasty happened,
 *           or TSM_NODATA if we have read all tiles that were requested
 *           and we are not doing read blocking.
 *
 * @author   Martin Reddy
 * @version  13 January 1998
 *
 * @see      tsmTileReqBegin, tsmSet, tsmAllocTileHeader, tsmAllocTileBuffer
 *
 */

int
tsmReadNextTile( TsmConnection connect, TsmTileHeader *tileHeader, 
		 uchar *tileBuffer )
{
  const char *funcName = "tsmReadNextTile";
  int        returnCode;
  TsmTileHeader *tileHeader_l;
  uchar *tileBuffer_l;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, TRUE ) )
    return FALSE;
  
  if ( connect->dont_read_tiles == TRUE ) return TRUE;

  TSMTRINC(( TSMMASK|3, 1 ));

  /* Now we can read the tile data in a protocol-specific manner */
  returnCode = tsmReqServerDequeueTile (connect->srv,
					&tileHeader_l, &tileBuffer_l);

  /* Then give a little trace message if successful (a dot) and return */

  if ( returnCode == TRUE ) {
    TSMTRACE ((TSMMASK|1, funcName, "."));

    *tileHeader = *tileHeader_l;
    memcpy (tileBuffer, tileBuffer_l, tileHeader->size);

    tsmMemFree (tileHeader_l);
    tsmMemFree (tileBuffer_l);

    if (tileHeader->tile_id <= 0)
      tileHeader->tile_id =
	tsmMakeTileId (tileHeader->tx, tileHeader->ty, tileHeader->level);
  }

  TSMTRINC(( TSMMASK|3, -1 ));
  return returnCode;
}

/*
 * Synopsis:
 *   int tsmAllocReadNextTile( TsmConnection connect, TsmTileHeader **,
 *                             uchar **tileBuffer )
 *
 * Doc++ Description: *//** 
 *   Read the next tile from server.
 *
 *   This function performs the same action as tsmReadNextTile, except that
 *   it allocates the memory for the tile header and buffer inside the
 *   function. You must therefore pass pointers to the tile header/buffer
 *   pointers, so that the allocated memory addresses can be filled in for
 *   you. You free the memory that has been returned using the
 *   tsmFreeTileHeader and tsmFreeTileBuffer functions as usual.
 *
 *   If this function returns a value of FALSE or TSM_NODATA, then there
 *   is no memory to be freed, so always check the return code!
 *
 *   Using this function allows for future internal extensions to tsmApi
 *   that can efficiently cache a number of tile headers and buffers for
 *   you to use in a threaded application.
 *
 *    You can set the TSM_DONT_READ_TILES setting to TRUE using tsmSet
 *    and this function will perform no disk or network access. This may
 *    be useful for assessing your application's computational overhead.
 *
 * @param    connect    the dataset connection structure
 * @param    tileHeader a pointer to a pointer that will be set for you 
 * @param    tileBuffer a pointer to a pointer that will be set for you 
 *
 * @return   TRUE if all went well, FALSE if something nasty happened,
 *           or TSM_NODATA if we have read all tiles that were requested
 *
 * @author   Martin Reddy
 * @version  5 March 1998
 *
 * @see      tsmTileReqBegin, tsmSet, tsmAllocTileHeader, tsmAllocTileBuffer
 *
 */

int
tsmAllocReadNextTile( TsmConnection connect, TsmTileHeader **tileHeader, 
		      uchar **tileBuffer )
{
  static const char *funcName = "tsmAllocReadNextTile";
  int               status;

  /* Try to allocate the new tile header and buffer */

  status = tsmReqServerDequeueTile (connect->srv, tileHeader, tileBuffer);

  if ( status != TRUE ) {
    if ( status == FALSE )
      tsmError( funcName, "Couldn't read the next tile" );

    *tileHeader = NULL;
    *tileBuffer = NULL;

    return status;
  }

  TSMTRACE ((TSMMASK|1, funcName, "."));
  return TRUE;
}

/*
 * Synopsis:
 *      void tsmTileReqBegin( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Start a new tile request list.
 *      
 *   In order to support the asynchronous receipt of terrain tile data
 *   over a network, we use a model whereby we build a request list of
 *   tile data, send that list, and then use the new tile data as it is
 *   received from the server. This model can of course be easily applied
 *   to local disk access also.
 *
 *   We build the request list by making a call to tsmTileReqBegin, then
 *   making multiple calls to tsmTileReqAdd to specify all of the tiles
 *   that we want to receive, then we call tsmTileReqEnd.
 *
 *   Once we have called tsmTileReqEnd then we can start reading in the
 *   tiles as they are received (trivial from local disk!) by calling a
 *   function such as tsmReadNextTile.
 *
 *   If you start a new request list before you have read in all of
 *   the tiles from a previous request list, then the previous request
 *   list will be thrown away in favour of the new list.
 *
 * @param    connection a dataset connection handle
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmTileReqAdd, tsmTileReqEnd, tsmReadNextTile
 * */

void
tsmTileReqBegin( TsmConnection connect )
{
  const char *funcName = "tsmTileReqBegin";

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, FALSE ) )
    return;

  /* Nothing here. With new request mechanism, this is a NOOP */

  TSMTRACE ((TSMMASK|2, funcName, "Done."));
  TSMTRINC ((TSMMASK|2, -1));
}

/*
 * Synopsis:
 *      int tsmTileReqAdd( TsmConnection connect, int tx, int ty,
 *                         int level, int priority )
 *
 * Doc++ Description: *//** 
 *   Add a tile to the request list.
 *
 *   In order to support the asynchronous receipt of terrain tile data
 *   over a network, we use a model whereby we built a request list of
 *   tile data, send that list, and then use the new tile data as it is
 *   received from the server. This model can of course be easily applied
 *   to local disk access also.
 *
 *   We build the request list by making a call to tsmTileReqStart, then
 *   making multiple calls to tsmTileReqAdd to specify all of the tiles that
 *   we want to receive, then we call tsmTileReqEnd.
 *
 *   Once we have called tsmTileReqEnd then we can start reading in the
 *   tiles as they are received (trivial from local disk!) by calling a
 *   function such as tsmReadNextTile.
 *
 *   N.B. if you request a tile that does not exist in the dataset, then
 *   the function will not add it to the request list, but it will still
 *   return a value of TRUE (so that we don't trip a complete failure).
 *
 *   There is a maximum number of requests that can be added to the
 *   request list. This is imposed by the DPSS implementation which
 *   can only handle request lists up to a compiled-in fixed size. This
 *   maximum request list size is defined by TSM_MAX_NUMREQS. If you
 *   request more tiles than this value in one list, then this function
 *   will fail with a FALSE return code. If this happens, you should
 *   investigate breaking up your tile requests into small chunks.
 *
 *   If you have started logging using the tsmBeginLogging call, then
 *   this function will automatically generate a TV_TILE_REQ event to
 *   the log file for the requested tile.
 *
 * @param    connect a dataset connection structure
 * @param    tx the x-coordinate of the tile to request
 * @param    ty the y-coordinate of the tile to request
 * @param    level the level of the pyramid to request the tile from
 * @param    priority one of: TSM_NOW, TSM_NEVER, or TSM_LATER
 *
 * @return   TRUE if operation successful, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmTileReqBegin, tsmTileReqEnd, tsmReadNextTile.
 *
 */

int
tsmTileReqAdd( TsmConnection connect, int tx, int ty, int level, int priority )
{
  static const char *funcName = "tsmTileReqAdd";
  SimpleTileSet     *tileset;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, TRUE ) )
    return FALSE;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName,
	     "con = %p, %s. Tile = (%d,%d) level %d. Prior = %d",
	     connect, connect->name, tx, ty, level, priority ));

  /* Validate the tile to make sure it exists in the dataset */

  if ( level < connect->tspec->minLevel || level > connect->tspec->maxLevel ) {
    TSMTRACE(( TSMMASK|3, funcName,
	       "Requested level does not exist in dataset" ));
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  tileset = &connect->tspec->tspecLevel.val[level];

  if ( tx < tileset->minTile[0] || tx > tileset->maxTile[0] ||
       ty < tileset->minTile[0] || ty > tileset->maxTile[1] ) {
    TSMTRACE(( TSMMASK|3, funcName,
	       "Requested tile does not exist in dataset" ));
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  tsmRequestAdd (connect->globalReqs, tx, ty, level, priority);
  TSMTRACE(( TSMMASK|3, funcName, "Done." ));

  /* log this tile request as long as it is not for a DPSS connection  */
  /* DPSS request logging happens in the request thread when the tiles */
  /* are *actually* request. See tsmSendRequestListToDPSS().           */

  if ( connect->type != TSM_DPSS )
    tsmLogTile( connect, TV_TILE_REQ, tx, ty, level );

  TSMTRINC(( TSMMASK|3, -1 ));

  return TRUE;
}

/*
 * Synopsis:
 *      int tsmTileReqEnd( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Finish a batch of tile requests.
 *
 *   tsmTileReqEnd is used to tell the tsmApi that you are finished
 *   compiling a list of tiles to be requested (e.g. with tsmTileReqAdd).
 *   The list can then be transmitted to the server, if appropriate,
 *   so that the tiles can begin to be sent.
 *
 *   Once this function has been called, you can begin to read the
 *   incoming tiles using the tsmReadNextTile routine.
 *
 *   N.B. If you start a new request list before you have read in all of
 *   the tiles from a previous request list, then the previous request
 *   list will be thrown away in favour of the new list.
 *
 * @param    connect a dataset connection structure
 *
 * @return   TRUE if operation successful, FALSE if an error occurred
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmReadNextTile, tsmTileReqBegin, tsmTileReqAdd
 *
 */

int
tsmTileReqEnd( TsmConnection connect )
{
  const char *funcName = "tsmTileReqEnd";
  int        returnCode;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_READ_ONLY, FALSE ) )
    return FALSE;

  TSMTRINC(( TSMMASK|1, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p.", connect ));

  /* Check that we have a valid connection structure */

  if ( connect == NULL || connect->location == NULL ) {
    tsmError( funcName, "NULL connection, or connection URL, passed" );
    TSMTRINC(( TSMMASK|1, -1 ));
    return FALSE;
  }

  tsmRequestSubmit (connect->globalReqs);
  returnCode = TRUE;

  TSMTRACE(( TSMMASK|2, funcName, "Done, return code = %d.", returnCode ));
  TSMTRINC(( TSMMASK|1, -1 ));

  return returnCode;
}

/*
 * Synopsis:
 *      int tsmCheckTileFormat( TsmConnection connect, TsmTileFormat )
 *
 * Description:
 *      Checks to see if the image tiles for a particular
 *      dataset are stored in a particular format, e.g. for local disk
 *      this can be used to test whether the dataset is represented
 *      using blocked TIFF files, or as collections of raw .tile
 *      files.
 *
 * Externals: None
 *
 * Returns: TRUE if images stored in specified format, FALSE otherwise.
 *
 * Author: Martin Reddy
 * 
 * Date: 14 January 1998
 *
 */

int
tsmCheckTileFormat (TsmConnection conn, TsmTileFormat format)
{
  return conn->vtable->checkTileFormat (conn, format);
}

/*
 * Synopsis:
 *      int tsmCheckTIFFFormat( TsmConnection connect, int )
 *
 * Description:
 *      Checks to see if the tiles for a localdisk dataset are stored
 *      in a particular image format using the TIFF file format. The
 *      result will be FALSE if the specified format TIFF file is not
 *      present in the highest resolution level of the dataset pyramid.
 *
 *      format should only be TSM_LRBT_FORMAT or TSM_JPEG_FORMAT.
 *
 * Externals: None
 *
 * Returns: TRUE if images stored in specified TIFF format, FALSE otherwise.
 *
 * Author: Martin Reddy
 * 
 * Date: 14 January 1998
 *
 */

int
tsmCheckTIFFFormat (TsmConnection conn, int format)
{
  return conn->vtable->checkTIFFFormat (conn, format);
}

/*
 * Synopsis:
 *   int tsmWriteTile( TsmConnection connect, int tx, int ty, int lev, uchar *)
 *
 * Doc++ Description: *//** 
 *   Write a single tile to a dataset.
 *
 *   Writes the given tile buffer to the dataset for the specified
 *   connection. There are a number of connection settings that can
 *   be defined via the tsmSet function in order to control the
 *   characteristics of this function. They include the TSM_WRITE_TIFFS,
 *   and TSM_JPEG_QUALITY settings (see tsmSet for more information).
 *
 *   N.B. when writing tiles to a TIFF file on local disk, the order
 *   in which you write the data is important. The tiles should be
 *   written in row-major, left-to-right order, e.g. p0p0, p1p0, p2p0,...
 *   p0p1, p1p1, p2p1... etc. The TIFF file will be created automatically
 *   when then first tile is output, and it will be closed automatically
 *   when then last tile is written. If you wish to write a number of
 *   levels of tiles, then you should write out all of the tile for a
 *   particular level at the same time (consecutively).
 *
 *   If you have started logging using the tsmBeginLogging call, then
 *   this function will automatically generate a TV_TILE_WRITE event to
 *   the log file for the requested tile.
 *
 * @param    connect the dataset connection to write to.
 * @param    tx      the x-coordinate of the tile to write.
 * @param    ty      the y-ccordinate of the tile to write.
 * @param    level   the level of the pyramid to write to.
 * @param    buffer  the tile data to write to the dataset.
 *
 * @return   TRUE if successful, FALSE if error occurred.
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmReadTile, tsmSet
 *
 */

int
tsmWriteTile (TsmConnection conn, int tx, int ty, int level,
	       uchar *tileBuffer)
{
  if (conn->vtable->writeTile)
    return conn->vtable->writeTile (conn, tx, ty, level, tileBuffer);
  else
    return 0;
}

/*
 * Synopsis:
 *      TsmTileHeader *tsmAllocTileHeader( void )
 *
 * Doc++ Description: *//** 
 *   Allocate memory to store a tile header.
 *      
 *  This function will return an allocated area of memory sufficient to
 *  hold a tile header. Tile Headers contains all of the descriptive
 *  variables about a raw tile buffer, e.g. the tile coordinates and
 *  resolution (level) that it refers to, etc.
 *
 * @param    connect an open connection handle, as returned by tsmConnect
 *
 * @return   the allocated tile buffer, of NULL if failed
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmFreeTileHeader, tsmAllocTileBuffer, tsmConnect
 *      tsmTileHeaderX, tsmTileHeaderY, tsmTileHeaderLevel, tsmTileHeaderSetId,
 *      tsmTileHeaderSessionId, tsmTileHeaderTileId, tsmTileHeaderSize, 
 *      tsmTileHeaderChecksum, tsmTileHeaderType, tsmTileHeaderConnection
 *
 */

TsmTileHeader *
tsmAllocTileHeader( void )
{
  return tsmMemCalloc( 1, sizeof(TsmTileHeader) );
}

/*
 * Synopsis:
 *   void tsmFreeTileHeader( TsmTileHeader *tileHeader );
 *
 * Doc++ Description: *//** 
 *   Deallocates memory used to store a tile header.
 *      
 *  This function should be used to free the memory for a header once 
 *  you are finished with it. You will have of course used the
 *  tsmAllocTileHeader routine to allocate the memory in the first place.
 *  The routine will do nothing if you pass a NULL pointer.
 *
 * @param    tileHeader the header that was allocated by tsmAllocTileHeader
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmAllocTileHeader, tsmFreeTileBuffer
 *
 */

void
tsmFreeTileHeader( TsmTileHeader *tileHeader )
{
  if ( tileHeader != NULL )
    tsmMemFree( tileHeader );
}

/*
 * Synopsis:
 *   int tsmTileHeaderX( TsmTileHeader const *tileHeader )
 *
 * Doc++ Description: *//** 
 *   Return the x-coord information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the x-coordinate of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's x-coord
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderX( TsmTileHeader const *tileHeader )
{
  return tileHeader->tx;
}

/*
 * Synopsis:
 *   int tsmTileHeaderY (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the y-coord information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the y-coordinate of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's y-coord
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderY( TsmTileHeader const *tileHeader )
{
  return tileHeader->ty;
}

/*
 * Synopsis:
 *   int tsmTileHeaderLevel (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the pyramid level information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the level of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's level number
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderLevel( TsmTileHeader const *tileHeader )
{
  return tileHeader->level;
}

/*
 * Synopsis:
 *   int tsmTileHeaderSetId (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the set ID information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the set ID of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's set ID
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderSetId( TsmTileHeader const *tileHeader )
{
  return tileHeader->set_id;
}

/*
 * Synopsis:
 *   int tsmTileHeaderSessionId (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the session ID information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the session ID associated with a tile.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's session ID value
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderSessionId( TsmTileHeader const *tileHeader )
{
  return tileHeader->session_id;
}

/*
 * Synopsis:
 *   int tsmTileHeaderTileId (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the tile ID information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the ID of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's ID
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderTileId( TsmTileHeader const *tileHeader )
{
  return  tileHeader->tile_id;
}

/*
 * Synopsis:
 *   int tsmTileHeaderSize (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the size information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the size (bytes) of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's size
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderSize( TsmTileHeader const *tileHeader )
{
  return tileHeader->size;
}

/*
 * Synopsis:
 *   int tsmTileHeaderChecksum (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the checksum information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the checksum of the tile that it describes.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's checksum
 *
 * @author   
 * @version  
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

int
tsmTileHeaderChecksum( TsmTileHeader const *tileHeader )
{
  return tileHeader->checksum;
}

/*
 * Synopsis:
 *   int tsmTileHeaderType (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the dataset type information from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the type of the dataset that the tile came from, e.g. this
 *   could be TSM_DEM_DATASET, TSM_IMAGE_DATASET, or TSM_UNKNOWN_DATASET.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's dataset type
 *
 * @author   Martin Reddy 
 * @version  6 March 1998
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile
 *
 */

TsmDataSetType
tsmTileHeaderType( TsmTileHeader const *tileHeader )
{
  return tileHeader->type;
}

/*
 * Synopsis:
 *   TsmConnection tsmTileHeaderConnection (TsmTileHeader const *tileHeader)
 *
 * Doc++ Description: *//** 
 *   Return the connection structure from a tile header.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will take a tile header and
 *   return the connection structure for the dataset that the tile came from.
 *   This gives you access to everything you could ever want to know about
 *   the individual tile, or the dataset that it came from, e.g. you can
 *   use tsmGet() on this structure to query various characteristics.
 *
 * @param    tileHeader a tile header, as filled by tsmReadNextTile
 *
 * @return   the tile's connection structure
 *
 * @author   Martin Reddy 
 * @version  6 March 1998
 *
 * @see      tsmAllocTileHeader, tsmReadNextTile, tsmGet
 *
 */

TsmConnection
tsmTileHeaderConnection( TsmTileHeader const *tileHeader )
{
  return tileHeader->connect;
}

/*
 * Synopsis:
 *   int tsmTileHeaderSizeof( void )
 *
 * Doc++ Description: *//** 
 *   Returns the size of a tile header structure.
 *
 *   The TsmTileHeader structure is filled in by the tile reading routines
 *   (e.g. tsmReadNextTile) and provides information about the raw tile data.
 *
 *   The tile header data structure is opaque (i.e. you cannot access its
 *   fields directly), so you must use the API calls to access the contents
 *   of a tile header. This particular API call will return the size of a  
 *   tile header structure (in bytes). This may be useful when wanting to
 *   exactly know how many bytes of data have been transfered at any time.
 *
 * @return   the tile header structure size (bytes)
 *
 * @author   Martin Reddy 
 * @version  13 March 1998
 *
 * @see      tsmAllocTileHeader, tsmFreeTileHeader
 *
 */

int
tsmTileHeaderSizeof( void )
{
  return sizeof( TsmTileHeader );
}

/*
 * Synopsis:
 *      uchar *tsmAllocTileBuffer( TsmConnection connect )
 *
 * Doc++ Description: *//** 
 *   Allocate memory to store a tile.
 *      
 *  This function will return an allocated area of memory sufficient to
 *  hold a single (uncompressed) tile for the dataset that is associated
 *  with a connection. Checks are made to ensure that you have passed
 *  a valid connect structure.
 *
 *  Tile buffers are used to store the data for a single terrain tile.
 *  For example, tsmReadTile will fill a tile buffer with data.
 *
 * @param    connect an open connection handle, as returned by tsmConnect
 *
 * @return   the allocated tile buffer, of NULL if failed
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmFreeTileBuffer, tsmAllocTileHeader, tsmConnect, tsmReadTile
 *
 */

uchar *
tsmAllocTileBuffer( TsmConnection connect )
{
  static const char *funcName = "tsmAllocTileBuffer";
  uchar             *buffer;
  int               numBytes;

  /* Do some sanity checks first of all... */

  if ( connect == NULL ) {
    tsmError( funcName, "NULL connection parameter passed" );
    return NULL;
  }

  if ( ( numBytes = connect->tileBufferSize ) <= 0 ) {
    tsmError( funcName, "Connection does not have a valid tileBufferSize" );
    return NULL;
  }

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName,
	     "Creating tile buffer of size %d.", numBytes ));

  /* Now we try to allocate the tile buffer memory */

  if ( ( buffer = (uchar *) tsmMemCalloc( 1, numBytes ) ) == NULL ) {
    tsmError( funcName, "Couldn't allocate tile buffer." );
    TSMTRINC(( TSMMASK|3, -1 ));
    return NULL;
  }

  TSMTRACE(( TSMMASK|3, funcName, "Returning buffer %p.", buffer ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return buffer;
}

/*
 * Synopsis:
 *   void tsmFreeTileBuffer( uchar *tileBuffer );
 *
 * Doc++ Description: *//** 
 *   Deallocates memory used to store a tile.
 *      
 *  This function should be used to free the memory for a tile once 
 *  you are finished with it. You will have of course used the
 *  tsmAllocTileBuffer routine to allocate the buffer in the first place.
 *  The routine will do nothing if you pass a NULL pointer.
 *
 * @param    tileBuffer the buffer that was allocated by tsmAllocTileBuffer
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmAllocTileBuffer, tsmFreeTileHeader
 *
 */

void
tsmFreeTileBuffer( uchar *tileBuffer )
{
  if ( tileBuffer != NULL )
    tsmMemFree( tileBuffer );
}

/*
 * Synopsis:
 *   void tsmFreeTile( TsmTileHeader *tileHeader, uchar *tileBuffer );
 *
 * Doc++ Description: *//** 
 *   Deallocates a tile header and tile data buffer.
 *      
 *  This function can be used to free the memory for a tile header and
 *  a tile data buffer at the same time. It will effectively perform a
 *  tsmFreeTileHeader and tsmFreeTileBuffer call for you. You can pass
 *  NULL pointers for either buffer and no deallocation will be attempted
 *  for that pointer.
 *
 * @param    tileBuffer the buffer that was allocated by tsmAllocTileBuffer
 * @param    tileHeader the buffer that was allocated by tsmAllocTileHeader
 *
 * @author   Martin Reddy
 * @version  16 March 1998
 *
 * @see      tsmAllocTileBuffer, tsmAllocTileHeader
 *           tsmFreeTileHeader, tsmFreeTileBuffer
 *
 */

void
tsmFreeTile( TsmTileHeader *tileHeader, uchar *tileBuffer )
{
  tsmFreeTileHeader( tileHeader );
  tsmFreeTileBuffer( tileBuffer );
}

/*
 * Synopsis:
 *      void tsmTileReqInfo( TsmConnection connect, PRIntervalTime *begin,
 *              PRIntervalTime *end, int *num_now, int *num_never,
 *              int *num_later )
 *
 * Doc++ Description: *//** 
 *   Get stats on the last tile request list.
 *  
 *   This function is deprecated and currently useless, since its model of
 *   statistical data gathering no longer corresponds to the new tile
 *   requesting infrastructure.
 *
 *   Please don't use it any more.  It simply fills in zeroes!
 *
 * @param    connect a dataset connection structure
 * @param    begin the timestamp before sending the request list
 * @param    end the timestamp after sending the request list
 * @param    num_now the number of TSM_NOW tiles in the request list
 * @param    num_never the number of TSM_NEVER tiles in the request list
 * @param    num_later the number of TSM_LATER tiles in the request list
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  
 *
 * @see      tsmTileReqBegin, tsmTileReqAdd, tsmTileReqEnd
 *
 */

void
tsmTileReqInfo( TsmConnection connect, PRIntervalTime *begin,
		PRIntervalTime *end, int *num_now, int *num_never,
		int *num_later )
{
  if ( begin)      *begin     = 0;
  if ( end )       *end       = 0;
  if ( num_now )   *num_now   = 0;
  if ( num_never ) *num_never = 0;
  if ( num_later ) *num_later = 0;
}
