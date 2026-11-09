/*
 * Name: tsmConnection_local.c
 *
 * Description: 
 *   Contains routines for manipulating connections to local disk datasets.
 *   These functions are private to the tsmApi.
 *
 * Function List:
 *   TsmConnection tsmConnect_local(Purl *, TsmAccessMode, TsmConnectionParams)
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmConnection_local.c,v 1.17.2.6 2002/03/14 05:22:55 reddy Exp $
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
#include "tsmConnectionParamsInt.h"
#include "tsmRequestSrv.h"

#include <prio.h>
#include <tiff.h>

static void tsmDisconnect_local (TsmConnection conn);
static void tsmDisconnectActual_local (TsmConnectionLocal *conn);
extern void tiffCacheClose (TIFF *tiff);


static TsmConnectionVTable vtable_local = {
  tsmDisconnect_local,

  tsmAddRequest_local,
  tsmRemoveRequest_local,
  tsmReprioRequest_local,

  tsmWriteTile_local,

  tsmCheckTileFormat_local,
  tsmCheckTIFFFormat_local,

  tsmvBeginLogging,
  tsmvEndLogging
};

TsmConnectionLocal *
tsmActualConnect_local( TsmConnectionLOCAL loc_connect, int idx,
			Purl *purl, TsmAccessMode mode,
			TsmConnectionParams param );

TsmConnection
tsmConnect_local( Purl *purl, TsmAccessMode mode, TsmConnectionParams param )
{
  const char         *funcName = "tsmConnect_local";
  PRFileInfo         filetest;

  TsmConnectionLOCAL loc_connect;
  TsmConnection connect;
  int i;

  /* Now we create a connection structure which is specific for the */
  /* connection protocol, i.e. local filesystem, http, or x-dsm.    */

  loc_connect = tsmMemManagerCalloc (1, sizeof (*loc_connect));
  connect = &loc_connect->common;

  connect->vtable = &vtable_local;
  
  connect->mode        = mode;
  connect->location    = purl;

  connect->type        = TSM_LOCAL;
  connect->format      = TSM_LRBT_FORMAT;
  connect->tile_format = TSM_TIFF_TILES;

  /* First, let's check to see if the specified dataset directory exists */

  if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE ) {
    if ( PR_GetFileInfo ( purl->uri, &filetest ) == PR_FAILURE ) {
      tsmError( funcName, "Dataset does not exist: %s", purl->uri );
      return NULL;
    }
  }

  /* The user can specify if they want to use .tile or .tiff files */

  if ( tsmIsParamSet( param, TSM_PARAM_TIFF ) == TRUE ) 
    connect->tile_format = param->tile_format;

  /* The user can specify the image format to use via the param struct */
  /* (Note that only the TIFF files allow raw and jpeg format. The     */
  /* .tile files only support raw data - which is the default option). */

  if ( connect->tile_format == TSM_TIFF_TILES )
    if ( tsmIsParamSet( param, TSM_PARAM_FORMAT ) == TRUE ) 
      connect->format = param->format;

  /* Do common initialization */
  connect = tsmConnectCommon (connect, purl, mode, param);
  if ( ! connect ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  if ( !(loc_connect->lock = PR_NewLock()) ) {
    tsmError( funcName, "Unable to create connection lock." );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  loc_connect->numReaders = 0;
  loc_connect->nextReader = 0;
  if (mode == TSM_READ_WRITE || mode == TSM_READ_ONLY) {
    loc_connect->numReaders = 4;
    if (tsmIsParamSet( param, TSM_PARAM_READ_MULTI ))
      loc_connect->numReaders = param->read_multi;
    loc_connect->readers =
      tsmMemManagerCalloc (loc_connect->numReaders,
			   sizeof (loc_connect->readers[0]));

    for (i = 0; i < loc_connect->numReaders; ++i) {
      loc_connect->readers[i] = tsmActualConnect_local (loc_connect, i, purl,
							TSM_READ_ONLY,
							param);
      /* Reduce multiplicity on failure */
      if (! loc_connect->readers[i]) {
	--loc_connect->numReaders;
	--i;
      }
    }
  }

  /* Restrict multiplexing for TIFF files. */
  if (connect->tile_format == TSM_TIFF_TILES && mode != TSM_WRITE_ONLY ) {
    Pyramid const *tspec = connect->tspec;
    int i;
    loc_connect->readLevel_lock =
      tsmMemManagerCalloc (tspec->maxLevel+1, sizeof (PRLock*));
    loc_connect->readLevel_tiff =
      tsmMemManagerCalloc (tspec->maxLevel+1, sizeof (TIFF*));
    for (i = tspec->minLevel; i <= tspec->maxLevel; ++i)
      loc_connect->readLevel_lock[i] = PR_NewLock ();
  } else {
    loc_connect->readLevel_lock = NULL;
    loc_connect->readLevel_tiff = NULL;
  }

  /* Abort now if we couldn't open any connections. Otherwise, we  */
  /* initialise all of the general connection options to defaults */

  if ( mode != TSM_WRITE_ONLY && ! loc_connect->numReaders ) {
    tsmError (funcName, "%s: Unable to connect %s", purl);
    tsmDisconnect (connect);
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  loc_connect->writer = 0;
  if (mode == TSM_READ_WRITE || mode == TSM_WRITE_ONLY) {
    loc_connect->writer = tsmActualConnect_local (loc_connect, 0, purl,
						  TSM_WRITE_ONLY,
						  param);
    if ( loc_connect->writer == NULL ) {
      /* Clean up stuff that we've initialize above. */
      tsmDisconnect (connect);
      TSMTRINC(( TSMMASK|2, -1 ));
      return NULL;
    }
  }

  return connect;
}

TsmConnectionLocal *
tsmActualConnect_local( TsmConnectionLOCAL loc_connect, int idx,
			Purl *purl, TsmAccessMode mode,
			TsmConnectionParams param )
{
  const char         *funcName = "tsmConnect_local";
  TsmConnection com_connect = &loc_connect->common;
  TsmConnectionLocal *connect;

  /* Allocate a new TsmConnectionLocal structure */

  connect = tsmMemManagerCalloc( 1, sizeof(TsmConnectionLocal) );
  if ( connect == NULL ) {
    tsmError( funcName, "Couldn't allocate local connection structure" );
    return NULL;
  }

  /* Fill in all of the easy bits of this structure */
  connect->conn = loc_connect;

  /* If the default or requested image format does not exist, then */
  /* try looking for another format (if read connection).          */

  if ( mode == TSM_READ_ONLY || mode == TSM_READ_WRITE ) {

    /* This is done here so that checkTileFormat et al work */
    loc_connect->readers[idx] = connect;

    if (tsmCheckTileFormat_local (com_connect,
				  com_connect->tile_format) == FALSE) {
      int raw_tiles, tiff_lrbt, tiff_jpeg, tiff_lzw;

      /* Has the user requested that we don't do any fallbacks? */

      if ( tsmIsParamSet( param, TSM_PARAM_FALLBACK ) == TRUE &&
	   param->fallback == FALSE ) {
	tsmError( funcName, "Specified dataset is not available: %s / %s",
		  tsmGet( com_connect, TSM_TILE_FORMAT_STRING ),
		  tsmGet( com_connect, TSM_IMAGE_FORMAT_STRING ) );
	tsmDisconnectActual_local ( connect );
	return NULL;
      }

      /* Okay, so we try to find another version of the dataset... */

      raw_tiles = tsmCheckTileFormat_local( com_connect, TSM_RAW_TILES );
      tiff_lrbt = tsmCheckTIFFFormat_local( com_connect, TSM_LRBT_FORMAT );
      tiff_jpeg = tsmCheckTIFFFormat_local( com_connect, TSM_JPEG_FORMAT );
      tiff_lzw  = tsmCheckTIFFFormat_local( com_connect, TSM_LZW_FORMAT );

      if ( tiff_lrbt == TRUE ) {
	com_connect->tile_format = TSM_TIFF_TILES;
	com_connect->format      = TSM_LRBT_FORMAT;
      } else if ( tiff_jpeg == TRUE ) {
	com_connect->tile_format = TSM_TIFF_TILES;
	com_connect->format      = TSM_JPEG_FORMAT;
      } else if ( tiff_lzw == TRUE ) {
	com_connect->tile_format = TSM_TIFF_TILES;
	com_connect->format      = TSM_LZW_FORMAT;
      } else if ( raw_tiles == TRUE ) {
	com_connect->tile_format = TSM_RAW_TILES;
	com_connect->format      = TSM_LRBT_FORMAT;
      } else {
	tsmError( funcName, "Couldn't find any valid tile data to use" );
	tsmDisconnectActual_local ( connect );
	return NULL;
      }
    }
  }

  /* If we are going to read tiles, then start the reader thread */
  if ( mode != TSM_WRITE_ONLY) {
    char buf[128];

    connect->heap = heap_create (512, tsmReqElemCmp, tsmReqElemUpdate);
    connect->lock = PR_NewLock ();
    connect->ready_cond = PR_NewCondVar (connect->lock);

    sprintf (buf, "%s reader[%d]", 
	     com_connect->tspec->identityTspec.name, idx);
    pthr_create (&connect->thread, buf,
		 tsmReaderThread_local, connect);
  }

  return connect;
}

static void
tsmDisconnectActual_local (TsmConnectionLocal *connect)
{
  if (connect->jpegBuffer)
    tsmMemFree (connect->jpegBuffer);

  if (connect->thread) {
    PR_Interrupt (connect->thread);
    PR_JoinThread (connect->thread);

    heap_destroy (connect->heap);
    PR_DestroyCondVar (connect->ready_cond);
    PR_DestroyLock (connect->lock);
  }

  tsmMemFree (connect);
}

void
tsmDisconnect_local (TsmConnection connect)
{
  int i;
  TsmConnectionLOCAL loc_connect = (TsmConnectionLOCAL) connect;

  /* Shut down readers and writers */

  if (loc_connect->readers) {
    for (i = 0; i < loc_connect->numReaders; ++i)
      tsmDisconnectActual_local (loc_connect->readers[i]);
    tsmMemFree (loc_connect->readers);
  }

  if (loc_connect->writer)
    tsmDisconnectActual_local (loc_connect->writer);

  if (loc_connect->lock)
    PR_DestroyLock (loc_connect->lock);

  /* Do any protocol specific disconnection */

  if ( loc_connect->readLevel_lock ) {
    Pyramid const *tspec = connect->tspec;

    for (i = tspec->minLevel; i <= tspec->maxLevel; ++i) {
      if (loc_connect->readLevel_tiff[i])
	tiffCacheClose (loc_connect->readLevel_tiff[i]);
      PR_DestroyLock (loc_connect->readLevel_lock[i]);
    }
    tsmMemManagerFree (loc_connect->readLevel_lock);
    tsmMemManagerFree (loc_connect->readLevel_tiff);
  }

  tsmDisconnectCommon (connect);

  tsmMemManagerFree (loc_connect);
}

int 
tsmGenerateUniqueSetID( void )
{
  static int set_id = 0;
  return set_id++;
}
