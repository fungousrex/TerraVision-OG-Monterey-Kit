/*
 * Name: tsmTile_local.c
 *
 * Description: 
 *   Contains routines for reading and writing tiles on local disk
 *   These functions are private to the tsmApi.
 *
 * Function List:
 *   int tsmReadNextTile_local( TsmConnection com_connect, uchar *tileBuffer )
 *   int tsmReadTIFF_local( TsmConnection, int, int, int, uchar * )
 *   int tsmReadTILE_local( TsmConnection, int, int, int, uchar * )
 *   int tsmWriteTile_local( TsmConnection, int, int, int, uchar * )
 *   int tsmWriteTIFF_local( TsmConnection, int, int, int, const uchar * )
 *   int tsmWriteTILE_local( TsmConnection, int, int, int, const uchar * )
 *   int tsmCheckTilesFormat_local( TsmConnection, int, tspSymbol )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTile_local.c,v 1.26.2.9 2002/03/13 23:40:56 leei Exp $
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
#include "tsmConnect_local.h"
#include "tsmRequestSrv.h"

#include <prerror.h>
#include <prinit.h>

#include <assert.h>

/*
 * Synopsis:
 *   int tsmAddRequest_local( TsmConnection, tsmReqElem * )
 *
 * Description:
 *   Adds a tile request to this connection.
 *
 * Returns: TRUE if all went okay, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 *
 * Date: 13 January 1998
 *
 */

int
tsmAddRequest_local (TsmConnection conn, tsmReqElem *req)
{
  static char const funcName[] = "tsmAddRequest_local";
  TsmConnectionLOCAL loc_conn = (TsmConnectionLOCAL) conn;
  TsmConnectionLocal *connect;

  if ( req == NULL )
    return FALSE;

  /* Extremely simple-minded allocation strategy. Just cycle through */
  {
    int idx;

    PR_Lock (loc_conn->lock); {
      idx = loc_conn->nextReader;
      if (++idx >= loc_conn->numReaders)
	idx = 0;
      loc_conn->nextReader = idx;
    }; PR_Unlock (loc_conn->lock);

    if (! (connect = loc_conn->readers[idx]))
      return FALSE;

    TSMTRACE ((TSMMASK|2, funcName, "[%d %d %d]%d => r[%d]",
	       req->req.x, req->req.y, req->req.res, req->req.prio, idx));
  }

  TSMTRINC ((TSMMASK|2, 1));
  PR_Lock (connect->lock); {
    tsmTileReq tile = req->req;
    tsmReqElemAllocate (funcName, req, connect);
    heap_add (connect->heap, req);
    PR_NotifyCondVar (connect->ready_cond);
    tsmLogTile(conn, TV_TILE_REQ, tile.x, tile.y, tile.res);
  }; PR_Unlock (connect->lock);
  TSMTRINC ((TSMMASK|2, -1));

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmRemoveRequest_local( TsmConnection, tsmReqElem * )
 *
 * Description:
 *   Removes a tile request from this connection.
 *
 * Returns: TRUE if all went okay, FALSE if an error occurred.
 *
 * Author: Lee Iverson
 *
 * Date: September 14, 2000
 *
 */

int
tsmRemoveRequest_local (TsmConnection conn, tsmReqElem *req)
{
  static char const funcName[] = "tsmRemoveRequest_local";
  int ret = 0;

  assert (req->connect == conn);
  if (req->conn.generic) {
    TsmConnectionLocal *connect = req->conn.generic;
    PR_Lock (connect->lock); {
      TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d]", 
		 req, req->req.x, req->req.y, req->req.res, req->req.prio));
      ret = heap_remove_idx (connect->heap, req, req->idx);
      req->conn.generic = 0;
    }; PR_Unlock (connect->lock);
  }

  return ret;
}

/*
 * Synopsis:
 *   int tsmReprioRequest_local( TsmConnection, tsmReqElem * )
 *
 * Description:
 *   Reprioritizes a tile request from this connection.
 *
 * Returns: TRUE if all went okay, FALSE if an error occurred.
 *
 * Author: Lee Iverson
 *
 * Date: September 14, 2000
 *
 */

int
tsmReprioRequest_local (TsmConnection conn, tsmReqElem *req)
{
  static char const funcName[] = "tsmReprioRequest_local";
  int ret = 0;

  assert (req->connect == conn);

  TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d] recost %d conn %p:%p", 
	     req, req->req.x, req->req.y, req->req.res, req->req.prio,
	     req->connect, req->conn.generic));

  if (req->conn.generic) {
    TsmConnectionLocal *connect = req->conn.generic;

    PR_Lock (connect->lock); {
      TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d] recost %d", 
		 req, req->req.x, req->req.y, req->req.res, req->req.prio));
      ret = heap_recost_idx (connect->heap, req, req->idx);
    }; PR_Unlock (connect->lock);
  }

  return ret;
}

static int readNextTile( TsmConnectionLocal *connect,
			 int x, int y, int res,
			 TsmTileHeader *tileHeader,
			 uchar *tileBuffer );

void
tsmReaderThread_local (void *ptr)
{
  static char const funcName[] = "tsmReaderThread_local";
  TsmConnectionLocal *connect = ptr;
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection com_connect = &loc_connect->common;
  TsmTileHeader *tileHeader;
  uchar *tileBuffer;

  while (1) {
    PRStatus status = PR_SUCCESS;
    tsmReqElem *req = 0;
    list_t *reqLists;

    PR_Lock (connect->lock); {
      while (heap_is_empty (connect->heap)) {
	TSMTRACE ((TSMMASK|2, funcName, "Wait for heap"));
	status = PR_WaitCondVar (connect->ready_cond, PR_INTERVAL_NO_TIMEOUT);
	if (status != PR_SUCCESS) {
	  TSMTRACE ((TSMMASK|1, funcName, "Break: status %d", status));
	  break;
	}
	TSMTRACE ((TSMMASK|2, funcName, "Signalled"));
      }
      if (status == PR_SUCCESS) {
	req = heap_pop (connect->heap);
	if ( req )
	  tsmReqElemReading (funcName, req);
      }
    }; PR_Unlock (connect->lock);

    if (status == PR_FAILURE) {
      PRErrorCode err = PR_GetError ();
      if (err == PR_PENDING_INTERRUPT_ERROR) {
	TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	break;
      }
      TSMTRACE ((TSMMASK|1, funcName, "Error: %d (???)", err));
      continue;
    }

    if ( ! req ) continue;

    /* Allocate and read the tile */
    tileHeader = tsmAllocTileHeader ();
    tileBuffer = tsmAllocTileBuffer (com_connect);
    reqLists = tsmReqReceivers (req);

    while (1) {
      int val = readNextTile (connect,
			      req->req.x, req->req.y, req->req.res,
			      tileHeader, tileBuffer);

      if (val == TSM_NODATA) {
	if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
	  PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
	  TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	  return;
	}
	/* otherwise, there is no data to use... */
	req->errCount++;
	break;
      } else if (! val) {
	if (++req->errCount <= 3) {
	  /* We retry one of these up to 3 times, after which we fail */
	  TSMTRACE ((TSMMASK|1, funcName, "Errs %d: Requeue", req->errCount));
	  PR_Lock (connect->lock); {
	    tsmReqElemAllocate (funcName, req, connect);
	    heap_add (connect->heap, req);
	    req = heap_pop (connect->heap);
	    tsmReqElemReading (funcName, req);
	  }; PR_Unlock (connect->lock);
	} else {
	  /* This is the failure case, returning the request with nonzero
	     errCount */
	  break;
	}
      } else {
	req->errCount = 0;
	break;
      }
    }

    /* If we're returning a failed tile, make tileBuffer NULL */
    if (req->errCount) {
      tsmFreeTileBuffer (tileBuffer);
      tileBuffer = 0;
    }

    /* Make the tile available to a reader */
    tsmReqServerQueueTile (com_connect->srv, 
			   req, tileHeader, tileBuffer, reqLists);
    list_destroy (reqLists);
  }
}

static int
readNextTile( TsmConnectionLocal *connect,
	      int x, int y, int res,
	      TsmTileHeader *tileHeader,
	      uchar *tileBuffer )
{
  const char        *funcName = "readNextTile";
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection      com_connect = &loc_connect->common;
  int                status;

  /* Fill in the tile header structure */

  tileHeader->tx         = x;
  tileHeader->ty         = y;
  tileHeader->level      = res;
  tileHeader->pad        = 0;
  tileHeader->set_id     = com_connect->tspec->identityTspec.setId;
  tileHeader->session_id = -1;
  tileHeader->tile_id    = -1;
  tileHeader->size       = com_connect->tileBufferSize;
  tileHeader->checksum   = 0;
  tileHeader->type       = com_connect->dataset_type;
  tileHeader->connect    = com_connect;

  /* Now get the actual tile. There are two possible formats here:      */
  /* 1) the old format of an individual raw bitmap file for each tile   */
  /* 2) the new format of a single Tiled TIFF file containing all tiles */

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName, "%s [%d %d %d]", 
	     com_connect->name, x, y, res ));

  if ( com_connect->tile_format == TSM_RAW_TILES ) {

    /* No TIFF, so we must therefore have a directory full of .tile */
    /* files. These are only provided in lrbt-raster format.        */

    status = tsmReadTILE_local( connect, x, y, res, tileBuffer );

  } else {

    /* Otherwise, we have a Tagged TIFF file for all tile images */

    status = tsmReadTIFF_local( connect, x, y, res, tileBuffer );

  }

  if (status != TRUE) {
    if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
      PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
      TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
    }
    status = TSM_NODATA;
  } else 
    tsmLogTile( &connect->conn->common, TV_TILE_READ, x, y, res );

  TSMTRACE(( TSMMASK|3, funcName, "[%d %d %d] returned code %d.",
	     x, y, res, status ));
  TSMTRINC(( TSMMASK|3, -1 ));

  return status;
}

/*
 * Synopsis:
 *   int tsmReadTILE_local( TsmConnection com_connect, int tx, int ty,
 *                          int level, uchar *tileBuffer )
 *
 * Description:
 *      Fills the tileBuffer array with the image data for the
 *      relevant tile from the specified .tile file. The .tile file is
 *      the old format that was used to store TerraVision data. This is
 *      a single file for one tile, containing the raw data for that tile.
 *
 * Externals: None
 *
 * Returns: TRUE if operation was succesful, FALSE if it failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 23 February 1998
 *
 */

int 
tsmReadTILE_local( TsmConnectionLocal *connect, int tx, int ty, int level,
		   uchar *tileBuffer )
{
  static const char  *funcName = "tsmReadTILE_local";
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection      com_connect = &loc_connect->common;
  char               tileFilename[512];
  int                numBytesRead, numBytes;
  FILE               *fp;

  /* Work out the filename for the .tile file on disk, and its size */

  tspTilePathName( com_connect->location->uri, tx, ty, level, tileFilename );
  numBytes = com_connect->tileBufferSize;
  
  /* Open the file and read in all of its contents to the tile buffer */

  if ( ( fp = fopen( tileFilename, "rb" ) ) == NULL ) {
    tsmError( funcName, "Unable to get tile at %s", tileFilename );
    memset( tileBuffer, 0, numBytes );
    return FALSE;
  }

  numBytesRead = fread( tileBuffer, 1, numBytes, fp );

  /* Check for any read errors */

  if ( ( ferror( fp ) != 0 ) || ( numBytesRead != numBytes ) ) {
    tsmError( funcName, "error reading tile '%s' (%d bytes read out of %d)",
	      tileFilename, numBytesRead, numBytes );
    fclose( fp );
    return FALSE;
  }

  fclose( fp );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmReadTIFF_local( TsmConnection com_connect, int tx, int ty,
 *                          int level, uchar *tileBuffer )
 *
 * Description:
 *      Fills the tileBuffer array with the image data for the
 *      relevant tile from the specified TIFF file. If the TIFF data 
 *      is stored as JPEG, then this will be decompressed automagically.
 *
 *      The information about the number and organisation of tiles in
 *      the TIFF file is taken from the Pyramid's tspecs rather than the 
 *      image - these should correlate anyway!
 *
 *      N.B. Currently we open the TIFF file and close it again for
 *      every tile access - we should really try and cache these things
 *      to cut out the overhead of opening a TIFF file each time.
 *
 * Externals: None
 *
 * Returns: TRUE if operation was succesful, FALSE if it failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 5 February 1998
 *
 */

typedef struct {
  TsmConnectionLOCAL connect;
  int level;
  TIFF *tiff;
} TiffHandle;

static PRLock *tiff_lock;
static list_t *tiff_handles;
static PRCallOnceType tiffInit;

static PRStatus
tiffInitFunc (void)
{
  tiff_lock = PR_NewLock ();
  tiff_handles = list_create (FALSE);
  return PR_SUCCESS;
}

static void
tiffCacheHandle (TsmConnectionLOCAL connect, int level, TIFF *tiff)
{
  static const char funcName[] = "tiffCacheHandle";
  TiffHandle *h;
  int max_handles = 32, count;
  
  PR_Lock (tiff_lock); {
    count = list_count (tiff_handles);
    if (count+1 >= max_handles) {
      TsmConnectionLOCAL oconnect;
      PRLock *lock;

      h = list_unshift (tiff_handles);
      oconnect = h->connect;
      lock = oconnect->readLevel_lock[h->level];

      PR_Lock (lock); {
	TIFFClose (h->tiff);
	oconnect->readLevel_tiff[h->level] = 0;
	TSMTRACE ((TSMMASK|2, funcName, "Close TIFF %s %p[%d] %p",
		   h->connect->common.name,
		   h->connect, h->level, h->tiff));
      }; PR_Unlock (lock);
    } else {
      h = tsmMemManagerMalloc (sizeof (TiffHandle));
    }

    TSMTRACE ((TSMMASK|2, funcName, "Add TIFF %s %p[%d] %p",
	       connect->common.name, connect, level, tiff));

    connect->readLevel_tiff[level] = tiff;
    h->connect = connect;
    h->level = level;
    h->tiff = tiff;
    list_push (tiff_handles, h);
  }; PR_Unlock (tiff_lock);
}

void
tiffCacheClose (TIFF *tiff)
{
  static const char funcName[] = "tiffCacheClose";
  list_iter_t i;

  PR_Lock (tiff_lock); {
    for (i = list_first (tiff_handles); ! list_done (i); i = list_next (i)) {
      TiffHandle *h = list_value (i);
      if (h->tiff == tiff) {
	TsmConnectionLOCAL connect = h->connect;
	PRLock *lock = connect->readLevel_lock[h->level];
	list_delete (i);
	PR_Lock (lock); {
	  TSMTRACE ((TSMMASK|2, funcName, "TIFF %s %p[%d] %p",
		     connect->common.name, h->connect, h->level, h->tiff));
	  TIFFClose (h->tiff);
	  connect->readLevel_tiff[h->level] = 0;
	}; PR_Unlock (lock);
	tsmMemManagerFree (h);
	break;
      }
    }
  }; PR_Unlock (tiff_lock);
}

int
tsmReadTIFF_local( TsmConnectionLocal *connect, int tx, int ty, int level,
		   uchar *tileBuffer )
{
  const char         *funcName = "tsmReadTIFF_local";
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection      com_connect = &loc_connect->common;
  SimpleTileSet      *tileset = &com_connect->tspec->tspecLevel.val[level];
  tsize_t            numBytes = com_connect->tileBufferSize;
  char               tiffFilename[512];
  TIFF               *handle = 0;
  ttile_t            tile_no;
  tsize_t            result;
  uint16             compr;
  PRLock             *lock = loc_connect->readLevel_lock[level];

  PR_Lock (lock); {
    handle = loc_connect->readLevel_tiff[level];
    TSMTRACE ((TSMMASK|2, funcName, "TIFF %s %p[%d] %p",
	       com_connect->name, com_connect, level, handle));

    if (! handle) {
      PR_CallOnce (&tiffInit, tiffInitFunc);

      /* TryOnce opening the TIFF file for reading... */
      tsmTIFFFilename( com_connect, level, tiffFilename );

      if ( ( handle = TIFFOpen( tiffFilename, "r" ) ) == NULL ) {
	tsmError( funcName, "Unable to open TIFF file %s", tiffFilename );
	memset( tileBuffer, 0, numBytes );
	PR_Unlock (lock);
	return FALSE;
      }

      /* Don't do this until we have a global cache of TIFF handles. */
      tiffCacheHandle (loc_connect, level, handle);
    }

    /* Work out what format the image data is in. If it is JPEG compressed  */
    /* then the pixel data will be in YCbCr - we want to convert it to RGB. */
    /* see tsmWriteTIFF_local() below for details on the YCbCr usage        */

    TIFFGetField( handle, TIFFTAG_COMPRESSION, &compr );
    if ( compr == COMPRESSION_JPEG )
      TIFFSetField( handle, TIFFTAG_JPEGCOLORMODE, (int) JPEGCOLORMODE_RGB );

    /* Do some sanity checks on the tile coordinates to be safe */

    if ( tx < tileset->minTile[0] || ty < tileset->minTile[1] ||
	 tx > tileset->maxTile[0] || ty > tileset->maxTile[1] ) {
      tsmError( funcName, "tile (%d,%d) is out of range", tx, ty );
      memset( tileBuffer, 0, numBytes );
      PR_Unlock (lock);
      return FALSE;
    }

    /* We find the index of the desired tile assuming that the tiles */
    /* are stored in row-major (increasing coordinate value) order.  */
    /* Then just get the tile with that index - simple!              */
    /* N.B. if the data is JPEG compressed, then we use the tsmApi   */
    /* call tsmJPEGDecompressTile() to convert JPEG to a TV tile.    */
  
    tile_no = ( ty * tileset->numTile[0] ) + tx;

    if ( compr == COMPRESSION_JPEG ) {

      /* allocate a JPEG decompress buffer, if we've got a jpg connect */

      if (! connect->jpegBuffer)
	connect->jpegBuffer = (uchar *) tsmMemMalloc( com_connect->tileBufferSize );

      result = TIFFReadRawTile( handle, tile_no, connect->jpegBuffer, numBytes );

      /* decompress the data to the users raw tile buffer */

      if ( tsmJPEGDecompressTile( connect->jpegBuffer, tileBuffer,
				  &com_connect->tspec->tileTspec ) == FALSE ) {
	tsmError( funcName, "Error decompressing JPEG tile (%d,%d)", tx, ty );
	PR_Unlock (lock);
	return FALSE;
      }

      /* Otherwise, just read the tile and do any necessary */
      /* decompression, e.g. for LZW compressed TIFF tiles. */

    } else if ( compr == COMPRESSION_NONE ) 
      result = TIFFReadRawTile( handle, tile_no, tileBuffer, numBytes );
    else
      result = TIFFReadEncodedTile( handle, tile_no, tileBuffer, numBytes );

  }; PR_Unlock (lock);

  /* Was there any problem getting the tile from the TIFF files? */

  if ( result < 0 ) {
    tsmError( funcName, "Couldn't read (%d,%d) in %s", tx, ty, tiffFilename );
    memset( tileBuffer, 0, numBytes );
    return FALSE;
  }

  return TRUE;
}

int
tsmWriteTileActual_local( TsmConnectionLocal *connect,
			  int tx, int ty, int level,
			  uchar *tileBuffer )
{
  const char *funcName = "tsmWriteTileActual_local_local";
  TsmConnection com_connect = &connect->conn->common;

  if ( com_connect->tile_format == TSM_TIFF_TILES ) {

    /* Write out the tile using the new Tagged TIFF file format */

    return tsmWriteTIFF_local( connect, tx, ty, level, tileBuffer );

  } else if ( com_connect->tile_format == TSM_RAW_TILES ) {

    /* Write out the tile using the old .tile file format */

    return tsmWriteTILE_local( connect, tx, ty, level, tileBuffer );

  } else {

    /* This should never happen!! */

    tsmError( funcName, "Unknown format: %d", (int) com_connect->tile_format );
    return FALSE;

  }
}

/*
 * Synopsis:
 *   int tsmWriteTile_local( TsmConnection, int, int, int, uchar * )
 *
 * Description:
 *   Writes a tile to a localhost dataset in either the old .tile
 *   format, or the new .tiff format.
 *
 * @return TRUE if successful, FALSE if error occurred.
 *
 * Author: Martin Reddy
 *
 * Date: 4 February 1998
 *
 */

int
tsmWriteTile_local ( TsmConnection connect,
		     int tx, int ty, int level,
		     uchar *tileBuffer )
{
  const char *funcName = "tsmWriteTile_local";
  TsmConnectionLOCAL loc_connect = (TsmConnectionLOCAL) connect;
  int        status;

  /* Check the connection structure and enforce its access mode */

  if ( ! tsmCheckConnectMode( funcName, connect, TSM_WRITE_ONLY, TRUE ) )
    return FALSE;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "connect = %p, tile = [%d %d %d]",
	     connect, tx, ty, level ));

  /* write the tile in a protocol-specific manner. */

  status = tsmWriteTileActual_local (loc_connect->writer,
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
 *   int tsmWriteTILE_local( TsmConnection, int, int, int, const uchar * )
 *
 * Description:
 *      Writes out a tile buffer to a .tile format file on local disk.
 *      The .tile format simply contains the raw data for a single tile.
 *      It was the original file format used by TerraVision I.
 *
 * Returns: TRUE if tile written okay, FALSE if the operation failed
 *
 * Author: Martin Reddy
 * 
 * Date: 23 February 1998
 *
 */

int
tsmWriteTILE_local( TsmConnectionLocal *connect, int tx, int ty, int level,
		    const uchar *tileBuffer )
{
  static const char *funcName = "tsmPutLocalTile";
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection      com_connect = &loc_connect->common;
  char               tileFilename[1024];
  FILE              *fp;

  tspTilePathName( com_connect->location->uri, tx, ty, level, tileFilename );
  tsmMakeDirIfNotExist( tileFilename, TSM_PATH_AND_FILE );

  if ( ( fp = fopen ( tileFilename, "wb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write tile file '%s'", tileFilename );
    return FALSE;
  }

  fwrite( tileBuffer, 1, com_connect->tileBufferSize, fp );
  fclose( fp );

  if ( ferror( fp ) != 0 ) {
    tsmError( funcName, "Error writing tile to '%s'", tileFilename );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteTIFF_local( TsmConnection, int, int, int, const uchar * )
 *
 * Description:
 *      Adds a tile to a TIFF file. Tiles can be added in any order; 
 *      However you must output an entire level of tiles in one session.
 *      The TIFF file will be created automatically when then first tile
 *      is output, and it will be closed automatically when then last
 *      tile is written.
 *
 *      The TIFF file can be written in either lrbt-raster format (raw)
 *      or JPEG format (compressed), depending upon connection's image
 *      format setting.
 *
 * Returns: TRUE if tile written okay, FALSE if the operation failed
 *
 * Author: Martin Reddy
 * 
 * Date: 4 February 1998
 *
 */

int
tsmWriteTIFF_local( TsmConnectionLocal *connect, int tx, int ty, int level,
		    const uchar *tileBuffer )
{
  const char         *funcName = "tsmWriteTIFF_local";
  TsmConnectionLOCAL loc_connect = connect->conn;
  TsmConnection      com_connect = &loc_connect->common;
  SimpleTileSet      *tileset = &com_connect->tspec->tspecLevel.val[level];
  tsize_t            numBytes = com_connect->tileBufferSize;
  ttile_t            new_tile_no;
  int                result;
  char               tiffFilename[512];
  static int         levels = -1;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName,
	     "tile (%d,%d) at level = %d", tx, ty, level ));

  if (levels == -1) {
    Pyramid *pyramid = (Pyramid *) tsmGet(com_connect, TSM_DATASET_TSPEC);
    levels = pyramid->maxLevel - pyramid->minLevel + 1;
  }

  if ( connect->TIFFhandle == NULL ) {
    connect->TIFFhandle = tsmMemCalloc( sizeof( TIFF* ), levels );
    connect->tiles_written = tsmMemCalloc( sizeof( int ), levels );
  }

  /* Is this the first ever tile to be added to the TIFF file? */

  if ( connect->TIFFhandle[level] == NULL ) {

    connect->tiles_written[level] = 0;

    tsmTIFFFilename( com_connect, level, tiffFilename );
    tsmMakeDirIfNotExist( tiffFilename, TSM_PATH_AND_FILE );

    /* No TIFF file is currently open, so open one and output all of */
    /* the inital fields to the image header                         */

    if ( ( connect->TIFFhandle[level] = TIFFOpen( tiffFilename, "w" ) ) == NULL ) {
      tsmError( funcName, "Couldn't create %s", tiffFilename );
      TSMTRINC(( TSMMASK|3, -1 ));
      return FALSE;
    }

    TSMTRACE(( TSMMASK|3, funcName, "Created %s", tiffFilename ));

    /* Output all of the fields describing the attributes of the image */

    /* The Tile Width and Heights must be powers of 2, so we can't */
    /* specify the actual value in the case of DEMS, e.g. 33x33    */

    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_IMAGEWIDTH,
		  (uint32) tileset->numPixels[0] );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_IMAGELENGTH,
		  (uint32) tileset->numPixels[1] );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_TILEWIDTH,
		  (uint32) tileset->tileTspec.numPixelsX );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_TILELENGTH,
		  (uint32) tileset->tileTspec.numPixelsY );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_BITSPERSAMPLE,
		  (uint16) tileset->tileTspec.numBytesPerComponent * 8 );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_SAMPLESPERPIXEL,
		  (uint16) tileset->tileTspec.numComponentsPixel );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PLANARCONFIG,
		  (uint16) PLANARCONFIG_CONTIG );
    TIFFSetField( connect->TIFFhandle[level], TIFFTAG_SUBFILETYPE,
		  (uint32) FILETYPE_PAGE );

    /* GRUESOME HACK ALERT!!!  It appears that you can't have a TIFF */
    /* image with only one tile (!) i.e. the image dimensions equal  */
    /* the tile dimensions. A problem with the tiff library? We get  */
    /* round this by making the image width one pixel smaller than   */
    /* the tile width when we have 1 tile only. This doesn't affect  */
    /* our application because we can still store a 128x128 tile in  */
    /* the TIFF, even though the image size is set to 127x128.       */

    if ( tileset->numPixels[0] <= tileset->tileTspec.numPixelsX &&
	 tileset->numPixels[1] <= tileset->tileTspec.numPixelsY )
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_IMAGEWIDTH, 
		    (uint32) tileset->numPixels[0] - 1 );

    /* If we have an image description, we may as well record it too */

    if ( tileset->description != NULL ) 
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_IMAGEDESCRIPTION,
		    (char *) tileset->description );

    /* Define the compression scheme to use: NONE or JPEG */

    if ( com_connect->format == TSM_LRBT_FORMAT ) {

      /* If we have no compression, then we will use raw RGB to store the */
      /* data with an orientation of bottom-left, i.e. lrbt-raster        */

      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_COMPRESSION,
		    (uint16) COMPRESSION_NONE );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_ORIENTATION,
		    (uint16) ORIENTATION_BOTLEFT );

      if ( tileset->tileTspec.numComponentsPixel == 1 ) {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_MINISBLACK );
      } else {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_RGB );
      }
    } else if ( com_connect->format == TSM_LZW_FORMAT ) {

      /* If we have LZW compression, then we will tell libtiff to do this */
      /* compression with an orientation of bottom-left, i.e. lrbt-raster */

      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_COMPRESSION,
		    (uint16) COMPRESSION_LZW );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_ORIENTATION,
		    (uint16) ORIENTATION_BOTLEFT );

      if ( tileset->tileTspec.numComponentsPixel == 1 ) {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_MINISBLACK );
      } else {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_RGB );
      }

    } else {

      /* If we are using JPEG compression, then we will convert the RGB   */
      /* data to YCbCr before compression. This is done because the JFIF  */
      /* file format (used to serve images to VRML Browsers) requires the */
      /* YCbCr colour space. Using YCbCr also enables us to achieve much  */
      /* better compression ratios than RGB, although for TerraVision we  */
      /* will have to convert back into RGB once we decompress the JPEG.  */
      /* For ease of extraction, we will include all JPEG Quantization &  */
      /* Huffman tables in the tile's JPEG datastream (JPEGTABLESMODE=0)  */

      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_COMPRESSION,
		    (uint16) COMPRESSION_JPEG );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_JPEGQUALITY,
		    (int) com_connect->jpeg_quality );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_ORIENTATION,
		    (uint16) ORIENTATION_BOTLEFT );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_JPEGTABLESMODE,
		    (int) 0 );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_JPEGCOLORMODE,
		    (int) JPEGCOLORMODE_RGB );
      TIFFSetField( connect->TIFFhandle[level], TIFFTAG_YCBCRSUBSAMPLING,
		    (uint16) 1, (uint16) 1 );

      if ( tileset->tileTspec.numComponentsPixel == 1 ) {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_MINISBLACK );
      } else {
	TIFFSetField( connect->TIFFhandle[level], TIFFTAG_PHOTOMETRIC,
		      (uint16) PHOTOMETRIC_YCBCR );
      }

    }

  }

  /* All okay so far, so let's add the new tile to the TIFF file in the */
  /* desired image format, currently lrbt-raster or jpeg-compressed.    */
  /* N.B. we have to flip JPEG data before storage because it must be   */
  /* given in a format where the top-left pixel is the origin, but lrbt */
  /* uses the bottom-left pixel as the origin.                          */

  new_tile_no = ( ty * tileset->numTile[0] ) + tx;

  if ( com_connect->format == TSM_LRBT_FORMAT )
    result = TIFFWriteRawTile( connect->TIFFhandle[level], new_tile_no,
			       (tdata_t) tileBuffer, numBytes );
  else {
    if ( com_connect->format == TSM_JPEG_FORMAT )
      tsmFlipTileVertically( (uchar *) tileBuffer, &tileset->tileTspec );

    result = TIFFWriteEncodedTile( connect->TIFFhandle[level], new_tile_no,
				   (tdata_t) tileBuffer, numBytes );
  }

  if ( result < 0 ) {
    tsmError( funcName, "failed write %s tile (%d,%d)", tiffFilename, tx, ty );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  connect->tiles_written[level]++;
  
  /* Was that the last tile to be add to the TIFF? If so, clean up */

  if ( connect->tiles_written[level] == tileset->numTile[0] * tileset->numTile[1] ) {
    TIFFClose( connect->TIFFhandle[level] );
    connect->TIFFhandle[level] = NULL;
    tileset->tileTspec.storageFormat = tspLrbtRaster;
    TSMTRACE(( TSMMASK|3, funcName, "Closed TIFF file" ));
  }

  TSMTRACE(( TSMMASK|3, funcName, "Completed successfully" ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *      int tsmCheckTileFormat_local( TsmConnection, TsmTileFormat )
 *
 * Description:
 *      Checks to see if the image tiles for a particular
 *      dataset are stored in a particular format, e.g. for local disk
 *      this can be used to test whether the dataset is represented
 *      using blocked TIFF files, or as collections of raw .tile
 *      files.
 *
 *      N.B. currently we assume that the tile format is the same for all
 *      levels of a dataset, i.e. if level n is in TIFF format, then so
 *      are all of the other levels of the pyramid.
 *
 * Externals: None
 *
 * Returns: TRUE if images stored in specified format, FALSE otherwise.
 *
 * Author: Martin Reddy
 * 
 * Date: 14 January 1997
 *
 */

int
tsmCheckTileFormatActual_local( TsmConnection com_connect, TsmTileFormat format )
{
  static char const funcName[] = "tsmCheckTileFormat_local";
  char          filename[1024];
  PRFileInfo64  filetest;
  int           raw_tiles, tiff_tiles, level = com_connect->tspec->minLevel;
  SimpleTileSet *tls = &com_connect->tspec->tspecLevel.val[level];

  /* Okay, let's get some data first. Do we have either of .tile or */
  /* .tiff files? We only check the lowest level of the pyramid and */
  /* assume that this is representative of the dataset as a whole.  */

  tspTilePathName( com_connect->location->uri,
		   tls->minTile[0], tls->minTile[1],
		   level, filename );
  raw_tiles = ( PR_GetFileInfo64 ( filename, &filetest ) == PR_SUCCESS );

  tsmTIFFFilename( com_connect, level, filename );
  tiff_tiles = ( PR_GetFileInfo64 ( filename, &filetest ) == PR_SUCCESS );

  TSMTRACE ((TSMMASK|2, funcName, "%s [Raw %s TIFF %s] Format %d",
	     filename, raw_tiles ? "Yes" : "No",
	     tiff_tiles ? "Yes" : "No",
	     format));

  /* Now that we know the available tile formats, we can work out */
  /* if the user's requested format is available.                 */

  switch ( format ) {
  case TSM_NONE_TILES:
    return ( raw_tiles == FALSE && tiff_tiles == FALSE );
  case TSM_RAW_TILES:
    return ( raw_tiles == TRUE );
  case TSM_TIFF_TILES:
    return ( tiff_tiles == TRUE );
  default:
    return FALSE;
  }
}

int
tsmCheckTileFormat_local (TsmConnection connect, TsmTileFormat format )
{
  const char *funcName = "tsmCheckTileFormat_local";

  if ( connect == FALSE || connect->tspec == FALSE ) {
    tsmError( funcName, "connection doesn't have a tspec field!" );
    return FALSE;
  }

  /* do the check in a protocol-specific manner */

  return tsmCheckTileFormatActual_local (connect, format);
}

/*
 * Synopsis:
 *      int tsmCheckTIFFFormat_local( TsmConnection connect, int )
 *
 * Description:
 *      Checks to see if the tiles for a localdisk dataset are stored
 *      in a particular image format using the TIFF file format. The
 *      result will be FALSE if the specified format TIFF file is not
 *      present in the highest resolution level of the dataset pyramid.
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

static int
tsmCheckTIFFFormatActual_local( TsmConnection com_connect, int format )
{
  /*static char const funcName[] = "tsmCheckTIFFFormat_local";*/
  char         filename[1024];
  PRFileInfo64 filetest;
  int          format_saved;

  if ( format == TSM_DPSS_TILES || format == TSM_NONE_TILES ) 
    return FALSE;

  /* Save the value of connect->format because we will change it */
  /* Then we will work out the name of the TIFF file to test for */

  format_saved = com_connect->format;

  com_connect->format = format;
  tsmTIFFFilename( com_connect, com_connect->tspec->minLevel, filename );
  com_connect->format = format_saved;

  return ( PR_GetFileInfo64 ( filename, &filetest ) == PR_SUCCESS );
}

int
tsmCheckTIFFFormat_local (TsmConnection connect, int format)
{
  const char *funcName = "tsmCheckTIFFFormat_local";

  if ( connect == FALSE || connect->tspec == FALSE ) {
    tsmError( funcName, "connection doesn't have a tspec field!" );
    return FALSE;
  }

  /* do the check in a protocol-specific manner */
  return tsmCheckTIFFFormatActual_local ( connect, format );
}

