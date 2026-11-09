/*
 * Name: tsmTile_web.c
 *
 * Description: 
 *   Contains routines for reading and writing tiles over an http link
 *   These functions are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmHost_web.c,v 1.1.2.14 2002/03/14 05:22:57 reddy Exp $
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

#include "tsmConnect_web.h"
#include "tsmConnect_wms.h"

#ifdef USE_HTTP

#include "tsmRequestSrv.h"
#include "tsmWMS.h"

#include <prerror.h>
#include <prinit.h>

struct HostConnection_ {
  int count;
  char const *hostName;
  char const *port;

  PRUintn apache_mod:1;  /* does server have the apache mod? */
  PRUintn wms_mod:1;     /* is this an OGC Web Mapping Server? */

  heap_t *heap;
  PRLock *lock;
  PRCondVar *ready_cond;

  int nthreads;
  PRThread **threads;
};

static struct {
  PRLock *lock;
  int n;
  int len;
  HostConnection *hosts;
} allHosts;

static PRCallOnceType allHostsOnce;

static PRStatus
allHostsInit (void)
{
  static char const funcName[] = "allHostsInit";
  TSMTRACE ((TSMMASK|1, funcName, ""));
  allHosts.lock = PR_NewLock ();
  return PR_SUCCESS;
}

HostConnection tsmCreateConnection_host (Purl *purl);
void tsmDestroyConnection_host (HostConnection connect);

void tsmReaderThread_host (void *ptr);

HostConnection
tsmOpenHostConnection_web (Purl *purl)
{
  static char const funcName[] = "tsmOpenHostConnection_web";
  HostConnection connect = 0;
  int i;

  PR_CallOnce (&allHostsOnce, allHostsInit);

  TSMTRACE ((TSMMASK|1, funcName, "%s:%s",
	     purl->hostName, purl->port));
  TSMTRINC ((TSMMASK|1, 1));

  PR_Lock (allHosts.lock); {
    /* Look for it and return if found */
    for (i = 0; i < allHosts.n; ++i) {
      HostConnection h = allHosts.hosts[i];
      if (! strcmp (purl->hostName, h->hostName)
	  && ! strcmp (purl->port, h->port)) {
	connect = h;
	++h->count;
	TSMTRACE ((TSMMASK|1, funcName, "Reopen %p:%s[%d]",
		   h, purl->hostName, h->count));
	PR_Unlock (allHosts.lock);
	TSMTRINC ((TSMMASK|1, -1));
	return connect;
      }
    }

    connect = tsmCreateConnection_host (purl);

    if (++allHosts.n >= allHosts.len) {
      allHosts.len = (allHosts.len ? 2*allHosts.len : 8);
      if (! allHosts.hosts)
	allHosts.hosts = malloc (allHosts.len * sizeof (allHosts.hosts[0]));
      else
	allHosts.hosts = realloc (allHosts.hosts, 
				  allHosts.len * sizeof (allHosts.hosts[0]));
    }

    TSMTRACE ((TSMMASK|1, funcName, "Add %p:%s[%d]",
	       connect, purl->hostName, connect->count));
    allHosts.hosts[allHosts.n-1] = connect;
  }; PR_Unlock (allHosts.lock);

  TSMTRINC ((TSMMASK|1, -1));
  return connect;
}

int
tsmCloseHostConnection_web (HostConnection hconn)
{
  static char const funcName[] = "tsmCloseHostConnection_web";
  int i;

  PR_Lock (allHosts.lock); {
    /* Look for it and return if found */
    for (i = 0; i < allHosts.n; ++i) {
      if (hconn == allHosts.hosts[i]) {
	break;
      }
    }
    if (i >= allHosts.n) {
      PR_Unlock (allHosts.lock);
      return FALSE;
    }

    /* Decrement the count and clean up if last reference */
    TSMTRACE ((TSMMASK|1, funcName, "Close host connection %p:%s[%d]",
	       hconn, hconn->hostName, hconn->count));
    TSMTRINC ((TSMMASK|2, 1));
    if (--hconn->count == 0) {
      tsmDestroyConnection_host (hconn);

      /* Move the last one into this slot. */
      if (i < --allHosts.n)
	allHosts.hosts[i] = allHosts.hosts[allHosts.n];
    }
    TSMTRINC ((TSMMASK|2, -1));
  }; PR_Unlock (allHosts.lock);

  return TRUE;
}

HostConnection
tsmCreateConnection_host (Purl *purl)
{
  static char const funcName[] = "tsmCreateConnection_host";
  char buf[512];
  HostConnection hconn;
  int i;

  /* Create a new one. */
  hconn = calloc (1, sizeof (*hconn));
  hconn->count = 1;
  hconn->hostName = tsmMemStrdup (purl->hostName);
  hconn->port = tsmMemStrdup (purl->port);

  /* is this a connection for an OGC Web Map Server? */

  hconn->wms_mod = ( purl->protocol == tsmWMT );

  /* check to see if the server is installed with the Apache mod */
  /* for serving TerraVision tiles, or the old CGI script method */

  sprintf( buf, "http://%s:%s/TV/dummy.oi?check=1",\
	   purl->hostName, purl->port );
    
  {
    http_trans_conn *http_conn = 0;
    hconn->apache_mod = tsmUrlExists( &http_conn, buf );
    if (http_conn)
      tsmCloseHttpConnection (http_conn);
  }

  /* If we are going to read tiles, then start the reader thread */
  hconn->heap = heap_create (512, tsmReqElemCmp, tsmReqElemUpdate);
  hconn->lock = PR_NewLock ();
  hconn->ready_cond = PR_NewCondVar (hconn->lock);

  /* Initialize threads list and spawn them */
  hconn->nthreads = 6;
  hconn->threads = malloc (hconn->nthreads * sizeof (hconn->threads[0]));

  for (i = 0; i < hconn->nthreads; ++i) {
    sprintf (buf, "%s:%s[%d]", hconn->hostName, hconn->port, i);
    pthr_create (&hconn->threads[i], buf,
		 tsmReaderThread_host, hconn);
  }

  TSMTRACE ((TSMMASK|1, funcName, "Create host connection %p:%s[%d]",
	     hconn, purl->hostName, hconn->count));
  
  return hconn;
}

void
tsmDestroyConnection_host (HostConnection hconn)
{
  static char const funcName[] = "tsmDestroyConnection_host";
  int i;

  TSMTRACE ((TSMMASK|1, funcName, "Destroy host connection %p:%s[%d]",
	     hconn, hconn->hostName, hconn->count));

  /* Clean up the reader threads */
  for (i = 0; i < hconn->nthreads; ++i) {
    if (hconn->threads[i]) {
      PR_Interrupt (hconn->threads[i]);
      PR_JoinThread (hconn->threads[i]);
    }
  }
  tsmMemFree (hconn->threads);

  /* Clean up the heap */
  heap_destroy (hconn->heap);
  PR_DestroyCondVar (hconn->ready_cond);
  PR_DestroyLock (hconn->lock);

  tsmMemFree (hconn->hostName);
  tsmMemFree (hconn->port);
  tsmMemFree (hconn);
}

/*
 * Synopsis:
 *   int tsmAddRequest_host( HostConnection, tsmReqElem * )
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
tsmAddRequest_host (HostConnection hconn, tsmReqElem *req)
{
  static char const funcName[] = "tsmAddRequest_host";
  PR_Lock (hconn->lock); {
    TSMTRACE ((TSMMASK|2, funcName, "req %p[%d %d %d]",
	       req, req->req.x, req->req.y, req->req.res));
    tsmReqElemAllocate (funcName, req, hconn);
    heap_add (hconn->heap, req);
    TSMTRACE ((TSMMASK|2, funcName, "heap[%d] push [%d %d %d]",
	       heap_size (hconn->heap),
	       req->req.x, req->req.y, req->req.res));
    PR_NotifyCondVar (hconn->ready_cond);
  }; PR_Unlock (hconn->lock);

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmRemoveRequest_host( HostConnection, tsmReqElem * )
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
tsmRemoveRequest_host (HostConnection hconn, tsmReqElem *req)
{
  static char const funcName[] = "tsmRemoveRequest_host";
  int ret;

  PR_Lock (hconn->lock); {
    ret = heap_remove_idx (hconn->heap, req, req->idx);
    req->conn.generic = 0;
    TSMTRACE ((TSMMASK|2, funcName, "req %p[%d %d %d] @[%d] %s",
	       req, req->req.x, req->req.y, req->req.res, req->idx,
	       ret ? "true" : "false"));
  }; PR_Unlock (hconn->lock);

  return ret;
}

/*
 * Synopsis:
 *   int tsmReprioRequest_host( HostConnection, tsmReqElem * )
 *
 * Description:
 *   Reprioritizes a tile request from this connection.
 *
 * Returns: TRUE if all went okay, FALSE if an error occurred.
 *
 * Author: Lee Iverson
 *
 * Date: November 22, 2000
 *
 */

int
tsmReprioRequest_host (HostConnection hconn, tsmReqElem *req)
{
  static char const funcName[] = "tsmReprioRequest_host";
  int ret;

  PR_Lock (hconn->lock); {
    TSMTRACE ((TSMMASK|2, funcName, "%p[%d %d %d] recost %d", 
	       req, req->req.x, req->req.y, req->req.res, req->req.prio));
    ret = heap_recost_idx (hconn->heap, req, req->idx);
  }; PR_Unlock (hconn->lock);

  return ret;
}

static int readNextTile( HostConnection hconn,
			 http_trans_conn **http_connp,
			 tsmReqElem const *req,
			 TsmTileHeader *tileHeader,
			 uchar *tileBuffer );

static void
trace_req (void *rv, void *c)
{
  tsmReqElem *req = rv;
  TSMTRACE ((TSMMASK|2, "", "[%d] req %p[%d %d %d] prio %d",
	     req->idx, req, 
	     req->req.x, req->req.y, req->req.res, req->req.prio));
}

void
tsmReaderThread_host (void *ptr)
{
  static char const funcName[] = "tsmReaderThread_host";
  HostConnection hconn = ptr;
  TsmTileHeader *tileHeader;
  uchar *tileBuffer;
  http_trans_conn *http_conn = 0;
  
#ifdef GHTTP_DEBUG
  if (TSMTRACEP (TSMMASK|3))
    ghttp_debug = 1;
#endif

  while (1) {
    PRStatus status = PR_SUCCESS;
    tsmReqElem *req = 0;
    list_t *reqLists;

    PR_Lock (hconn->lock); {
      if (TSMTRACEP (TSMMASK|2) && !heap_is_empty (hconn->heap)) {
	TSMTRACE ((TSMMASK|2, funcName,
		   "heap[%d] {", heap_size (hconn->heap)));
	TSMTRINC ((TSMMASK|2, 1));
	heap_forall (hconn->heap, trace_req, 0);
	TSMTRINC ((TSMMASK|2, -1));
	TSMTRACE ((TSMMASK|2, funcName,
		   "} heap[%d]", heap_size (hconn->heap)));
      }
      while (heap_is_empty (hconn->heap)) {
	TSMTRACE ((TSMMASK|2, funcName, "Wait for heap"));
	status = PR_WaitCondVar (hconn->ready_cond, PR_INTERVAL_NO_TIMEOUT);
	if (status != PR_SUCCESS) {
	  TSMTRACE ((TSMMASK|1, funcName, "Break. status %d", status));
	  break;
	}
	TSMTRACE ((TSMMASK|2, funcName, "Signalled"));
      }
      if (status == PR_SUCCESS) {
	req = heap_pop (hconn->heap);
	TSMTRACE ((TSMMASK|2, funcName, "heap[%d] pop [%d %d %d]",
		   heap_size (hconn->heap),
		   req->req.x, req->req.y, req->req.res));
	if ( req )
	  tsmReqElemReading (funcName, req);
      }
    }; PR_Unlock (hconn->lock);

    if (status == PR_FAILURE) {
      PRErrorCode err = PR_GetError ();
      if (err == PR_PENDING_INTERRUPT_ERROR) {
	TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	goto exit;
      }
      TSMTRACE ((TSMMASK|1, funcName, "Error: %d (???)", err));
      continue;
    }

    if ( ! req ) continue;

    /* Allocate and read the tile */
    tileHeader = tsmAllocTileHeader ();
    tileBuffer = tsmAllocTileBuffer (req->connect);
    reqLists = tsmReqReceivers (req);

    while (1) {
      int val;
      
      if ( hconn->wms_mod ) {

	/* contact the OGC Web Map Server for the tile */

	val = tsmWMSReadNextTile( hconn, &http_conn, req, tileHeader,
				  tileBuffer );
      } else {

	/* contact the SRI TerraVision server for the tile */

	val = readNextTile (hconn, &http_conn, req, tileHeader, tileBuffer);
      }

      if (val == TSM_NODATA) {
	if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
	  PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
	  TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
	  goto exit;
	}

	/* otherwise, there is no data to use... */
	++req->errCount;
      } else if (! val) {
	++req->errCount;
      } else {
	req->errCount = 0;
	break;
      }

      if (req->errCount) {
	if (req->errCount < 3) {
	  /* We retry one of these up to 3 times, after which we fail */
	  TSMTRACE ((TSMMASK|1, funcName, "Errs %d: Requeue", req->errCount));
	  PR_Lock (hconn->lock); {
	    tsmReqElemAllocate (funcName, req, hconn);
	    heap_add (hconn->heap, req);
	    TSMTRACE ((TSMMASK|2, funcName, "heap[%d] push [%d %d %d]",
		       heap_size (hconn->heap),
		       req->req.x, req->req.y, req->req.res));
	    req = heap_pop (hconn->heap);
	    TSMTRACE ((TSMMASK|2, funcName, "heap[%d] pop  [%d %d %d]",
		       heap_size (hconn->heap),
		       req->req.x, req->req.y, req->req.res));
	    tsmReqElemReading (funcName, req);
	  }; PR_Unlock (hconn->lock);
	} else {
	  break;;
	}
      }
    }

    /* If we're returning a failed tile, make tileBuffer NULL */
    if (req->errCount) {
      tsmFreeTileBuffer (tileBuffer);
      tileBuffer = 0;
    }

    /* Make the tile available to a reader */
    tsmReqServerQueueTile (req->connect->srv, 
			   req, tileHeader, tileBuffer, reqLists);
    list_destroy (reqLists);
  }

 exit:;

  /* Clean up this connection. */
  if (http_conn)
    tsmCloseHttpConnection (http_conn);

  TSMTRACE ((TSMMASK|1, funcName, "Exiting"));
}

static int
readNextTile( HostConnection connect,
	      http_trans_conn **http_connp,
	      tsmReqElem const *r,
	      TsmTileHeader *tileHeader,
	      uchar *tileBuffer )
{
  const char        *funcName = "readNextTile";
  TsmConnection     com_connect = r->connect;
  int               x = r->req.x;
  int               y = r->req.y;
  int               res = r->req.res;
  char              *name, url[1024];
  int               nread;
  int               bufSize = com_connect->tileBufferSize;

  TSMTRACE ((TSMMASK|1, funcName, "[%d %d %d] %s", x, y, res,
	     com_connect->location->uri ));

  /* Fill in the tile header structure */

  tileHeader->tx         = x;
  tileHeader->ty         = y;
  tileHeader->level      = res;
  tileHeader->pad        = 0;
  tileHeader->set_id     = com_connect->tspec->identityTspec.setId;
  tileHeader->session_id = -1;
  tileHeader->tile_id    = -1;
  tileHeader->size       = bufSize;
  tileHeader->checksum   = 0;
  tileHeader->type       = com_connect->dataset_type;
  tileHeader->connect    = com_connect;
  
  /* Work out the pyramid name from the connection URL */
  
  if ( ( name = strrchr( com_connect->location->uri, '/' ) ) == NULL )
    name = com_connect->location->uri;
  else
    name++;

  /* Build a URL that redirects to the get_http_tile CGI program */
  /* The source to this CGI program can be found from the tsmApi */
  /* sources page: http://www.ai.sri.com/tsmApi/tutorials. This  */
  /* needs to be installed in the cgi-bin dir of the http server */

  if ( connect->apache_mod )
    sprintf( url, "http://%s:%s/TV%s?x=%d&y=%d&level=%d",
	     com_connect->location->hostName, com_connect->location->port,
	     com_connect->location->uri,
	     tileHeader->tx, tileHeader->ty, tileHeader->level );
  else
    sprintf( url, "http://%s:%s/cgi-bin/get_http_tile"
	     "?pyramid=%s&x=%d&y=%d&level=%d",
	     com_connect->location->hostName, com_connect->location->port,
	     name,
	     tileHeader->tx, tileHeader->ty, tileHeader->level );

  switch ( com_connect->format ) {
  case TSM_LRBT_FORMAT:
    strcat( url, "&lrbt=yes" );
    break;
  case TSM_JPEG_FORMAT:
    strcat( url, "&jpeg=yes" );
    break;
  case TSM_LZW_FORMAT:
    strcat( url, "&lzw=yes" );
    break;
  }

  switch ( com_connect->tile_format ) {
  case TSM_RAW_TILES:
    strcat( url, "&rawtiles=yes" );
    break;
  case TSM_TIFF_TILES:
    strcat( url, "&tiffs=yes" );
    break;
  }

  TSMTRACE(( TSMMASK|2, funcName, "Requesting %s", url ));

  /* Call the Apache mod/CGI program to generate the tile buffer */

  if (! tsmUrlToBuffer( http_connp, url, 
			(char *) tileBuffer, bufSize, &nread )
      || nread != bufSize) {
    if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
      PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
      TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
    }

    TSMTRACE(( TSMMASK|1, funcName, "FAIL [%d %d %d] %d (nread %d)",
	       x, y, res, bufSize, nread));
    return TSM_NODATA;
  }
  TSMTRACE(( TSMMASK|1, funcName, "Done [%d %d %d] buf %p [%u,%u,%u...]",
	     x, y, res, tileBuffer,
	     tileBuffer[0], tileBuffer[1], tileBuffer[2]));

  if ( nread == bufSize ) {
    tsmLogTile( com_connect, TV_TILE_READ, x, y, res );
    return TRUE;
  }

  return FALSE;
}

/*
 * Synopsis:
 *      int tsmCheckTileFormat_host( TsmConnection, TsmTileFormat )
 *
 * Description:
 *      Checks to see if the image tiles for a particular http
 *      dataset are stored in a particular format, i.e. blocked TIFF
 *      files or collections of raw .tile files.
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
 * Date: 7 December 1997
 *
 */

int
tsmCheckTileFormat_web (TsmConnection com_connect, 
			TsmTileFormat format )
{
  /*static char const funcName[] = "tsmCheckTileFormat_web";*/
  char          filename[512], url[512];
  int           raw_tiles, tiff_tiles, level = com_connect->tspec->minLevel;
  SimpleTileSet *tls = &com_connect->tspec->tspecLevel.val[level];
  TsmConnectionHTTP connect = (TsmConnectionHTTP) com_connect;

  /* Okay, let's get some data first. Do we have either of .tile or */
  /* .tiff files? We only check the lowest level of the pyramid and */
  /* assume that this is representative of the dataset as a whole.  */

  sprintf( url, "%s/%d/tiles/p%dp%d.tile", com_connect->location->url,
	   level, tls->minTile[0], tls->minTile[1] );
  raw_tiles = tsmUrlExists ( &connect->http_conn, url );

  tsmTIFFFilename( com_connect, level, filename );
  sprintf( url, "http://%s:%s%s", com_connect->location->hostName,
	   com_connect->location->port, filename );
  tiff_tiles = tsmUrlExists( &connect->http_conn, url );

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

/*
 * Synopsis:
 *      int tsmCheckTIFFFormat_host( TsmConnection connect, int )
 *
 * Description:
 *      Checks to see if the tiles for an http dataset are stored
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
 * Date: 7 December 1998
 *
 */

int
tsmCheckTIFFFormat_web( TsmConnection com_connect,
			 int format )
{
  /*static char const funcName[] = "tsmCheckTIFFFormat_web";*/
  char        filename[512], url[512];
  int         format_saved;
  TsmConnectionHTTP connect = (TsmConnectionHTTP) com_connect;

  if ( format == TSM_DPSS_TILES || format == TSM_NONE_TILES ) 
    return FALSE;

  /* Save the value of connect->format because we will change it */
  /* Then we will work out the name of the TIFF file to test for */

  format_saved = com_connect->format;

  com_connect->format = format;
  tsmTIFFFilename( com_connect, com_connect->tspec->minLevel, filename );
  com_connect->format = format_saved;

  sprintf( url, "http://%s:%s%s", com_connect->location->hostName,
	   com_connect->location->port, filename );

  /* Try to open a connection to the http file to see if it exists */

  return tsmUrlExists( &connect->http_conn, url );
}

#else

int
tsmCheckTileFormat_host( TsmConnection com_connect,
			 HostConnection connect, TsmTileFormat format )
{
  tsmNotCompiledIn( com_connect->location->protocol,
		    "tsmCheckTileFormat_host" );
  return FALSE;
}

int
tsmCheckTIFFFormat_host( TsmConnection com_connect,
			 HostConnection connect, int format )
{
  tsmNotCompiledIn( com_connect->location->protocol,
		    "tsmCheckTIFFFormat_host" );
  return FALSE;
}

#endif
