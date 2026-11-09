/*
 * Name: tsmTile_wms.c
 *
 * Description: 
 *   Contains routines for reading and writing tiles from an OGC
 *   Web Mapping Server. These functions are private to the tsmApi.
 *
 * Function List:
 *
 * Revision History:
 *   Created 17 Jun 2001 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTile_wms.c,v 1.1.2.5 2001/07/24 00:48:04 reddy Exp $
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
 *   Portions are Copyright (c) SRI International, 1998-2001.
 *
 */

#include "tsmInternal.h"
#include "tsmConnect_wms.h"
#include "tsmRequest.h"

#include <prerror.h>
#include <prinit.h>

int
tsmAddRequest_wms (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionWMS wms_conn = (TsmConnectionWMS) conn;
  tsmTileReq tile;

  if ( req == NULL || conn == NULL ) return FALSE;

  tile = req->req;
  tsmLogTile(conn, TV_TILE_REQ, tile.x, tile.y, tile.res);

  return tsmAddRequest_host (wms_conn->hconn, req);
}

int
tsmRemoveRequest_wms (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionWMS wms_conn = (TsmConnectionWMS) conn;
  return tsmRemoveRequest_host (wms_conn->hconn, req);
}

int
tsmReprioRequest_wms (TsmConnection conn, tsmReqElem *req)
{
  TsmConnectionWMS wms_conn = (TsmConnectionWMS) conn;
  return tsmReprioRequest_host (wms_conn->hconn, req);
}

/*
 * Synopsis:
 *      int tsmCheckTileFormat_wms( TsmConnection, TsmTileFormat )
 *
 * Description:
 *      Checks to see if the image tiles for a particular
 *      dataset are stored in a particular format. For a WMS, this
 *      is just constrained to a singel format.
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
tsmCheckTileFormat_wms (TsmConnection connect, TsmTileFormat format )
{
  const char *funcName = "tsmCheckTileFormat_wms";

  if ( connect == FALSE || connect->tspec == FALSE ) {
    tsmError( funcName, "connection doesn't have a tspec field!" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *      int tsmCheckTIFFFormat_wms( TsmConnection connect, int )
 *
 * Description:  * Games with LOD tech will have a market edge over those
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

int
tsmCheckTIFFFormat_wms (TsmConnection connect, int format)
{
  const char *funcName = "tsmCheckTIFFFormat_wms";

  if ( connect == FALSE || connect->tspec == FALSE ) {
    tsmError( funcName, "connection doesn't have a tspec field!" );
    return FALSE;
  }

  return TRUE;
}


int
tsmWMSReadNextTile( HostConnection connect, http_trans_conn **http_connp,
		    tsmReqElem const *r, TsmTileHeader *tileHeader,
		    uchar *tileBuffer )
{
  const char        *funcName = "tsmWMSReadNextTile";
  TsmConnection     com_connect = r->connect;
  TsmConnectionWMS  wms_connect = (TsmConnectionWMS) r->connect;
  int               x = r->req.x;
  int               y = r->req.y;
  int               res = r->req.res;
  char              *name, url[2048], *format;
  char              *tmpBuffer = NULL, *exception;
  int               nread;
  int               bufSize = com_connect->tileBufferSize;
  int               tilesize, overlap, wms10;
  tsmWMS            *wms = wms_connect->wms;
  tsmLayer          *layer;
  Pyramid           *tspec = com_connect->tspec;
  double            bbox[4], tilescale;
  float             *matrix;

  TSMTRACE ((TSMMASK|2, funcName, "[%d %d %d]", x, y, res));

  /* Fill in the tile header structure */

  tileHeader->tx         = x;
  tileHeader->ty         = y;
  tileHeader->level      = res;
  tileHeader->pad        = 0;
  tileHeader->set_id     = tspec->identityTspec.setId;
  tileHeader->session_id = -1;
  tileHeader->tile_id    = -1;
  tileHeader->size       = bufSize;
  tileHeader->checksum   = 0;
  tileHeader->type       = com_connect->dataset_type;
  tileHeader->connect    = com_connect;
  
  /* Get the layer details based upon the name of the layer */
  
  name = tsmWMSExtractParam( com_connect->location->url, "layers" );
  if ( name == NULL )
    name = tsmWMSExtractParam( com_connect->location->url, "layer" );
  layer = tsmWMSFindLayer( wms, name );
  free( name );

  if ( layer == NULL ) {
    TSMTRACE(( TSMMASK|1, funcName, "FAIL (no layer) [%d %d %d]", x, y, res));
    return TSM_NODATA;
  }

  /* work out the bounding box to query over */

  tilesize  = tspec->tileTspec.numPixelsX;
  overlap   = tspec->tileTspec.overLap[0];
  matrix    = tspec->tspecLevel.val[res].imageTransformationTspec.ixiytoUV;
  tilescale = matrix[0] * tilesize;  /* UV tile extent */

  bbox[0] = layer->uv_bbox[0] + ( tilescale * x );
  bbox[1] = layer->uv_bbox[1] + ( tilescale * y );
  bbox[2] = layer->uv_bbox[0] + ( tilescale * (x+1) );
  bbox[3] = layer->uv_bbox[1] + ( tilescale * (y+1) );

  /* do we create a wms 1.0 or wms 1.1 style request */

  wms10 = tsmWMSVersion( wms->version ) < tsmWMSVersion( "1.0.8" );

  /* work out the format name/MIME for the request */

  format = wms->format[wms->mapFormat];

  /* Build an HTTP GET request for the OGC Web Map Server */

  sprintf( url, "%s?REQUEST=%s&%s=%s&FORMAT=%s&LAYERS=%s&STYLES=&"
	   "WIDTH=%d&HEIGHT=%d&SRS=EPSG:%d&BBOX=%1.10lg,%1.10lg,%1.10lg,"
	   "%1.10lg%s",
	   wms->getmap_url,
	   ( wms10 ) ? "map" : "GetMap",
	   ( wms10 ) ? "WMTVER" : "VERSION",
	   wms->version,
	   wms->format[wms->mapFormat],
	   layer->name, 
	   tilesize, tilesize,
	   layer->srs,
	   bbox[0], bbox[1], bbox[2], bbox[3],
	   ( wms10 ) ? "" : "&EXCEPTIONS=" );

  if ( overlap > 0 )
    strcat( url, "&OVERLAP=0,0,1,1" );

  TSMTRACE(( TSMMASK|2, funcName, "Requesting %s", url ));

  /* Download the tile from the Web Map Server */

  if ( ! tsmUrlToMemory( http_connp, url, &tmpBuffer, &nread ) ) {
    if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR) {
      PR_SetError (PR_PENDING_INTERRUPT_ERROR, 0);
      TSMTRACE ((TSMMASK|1, funcName, "Interrupted"));
    }

    TSMTRACE(( TSMMASK|1, funcName, "FAIL [%d %d %d]", x, y, res));
    return TSM_NODATA;
  }

  /* Need to translate/extract the buffer to the TerraVision buffer */
  /* e.g. support different return formats from WMS such as JPEG.   */

  switch ( wms->mapFormatID ) {
  case TSM_WMS_FORMAT_LRBT:

    /* LRBT returned - just copy the bytes */
    
    if ( nread != bufSize ) {
      tmpBuffer[nread] = '\0';
      exception = tsmWMSGetException( tmpBuffer );
      if ( exception ) {
	tsmError( funcName, "WMS Exception: %s", exception );
	free( exception );
      } else {
	tsmError( funcName, "Invalid tile returned from WMS (%d bytes)!",
		  nread );
      }
      free( tmpBuffer );
      return TSM_NODATA;
    }

    memcpy( tileBuffer, tmpBuffer, bufSize );
    free( tmpBuffer );
    break;

  case TSM_WMS_FORMAT_JPEG:

    /* JPEG returned - decompress it  */

    if ( ! tsmJPEGDecompressTile( (uchar *)tmpBuffer, tileBuffer,
				  &tspec->tileTspec ) ){
      tsmError( funcName, "Could not decompress JPEG tile (%d, %d, %d)",
		x, y, res );
      free( tmpBuffer );
      return TSM_NODATA;
    }

    free( tmpBuffer );
    break;

  default:

    /* unsupported image format - flag an error */

    tmpBuffer[nread] = '\0';
    exception = tsmWMSGetException( tmpBuffer );
    if ( exception ) {
      tsmError( funcName, "WMS Exception: %s", exception );
      free( exception );
    } else {
      tsmError( funcName, "No support for this WMS format yet (%d: %s)!",
		wms->mapFormatID, format );
    }

    free( tmpBuffer );
    return TSM_NODATA;
  }

  TSMTRACE(( TSMMASK|2, funcName, "Done [%d %d %d] buf %p [%u,%u,%u...]",
	     x, y, res, tileBuffer,
	     tileBuffer[0], tileBuffer[1], tileBuffer[2]));

  tsmLogTile( com_connect, TV_TILE_READ, x, y, res );
  return TRUE;
}

