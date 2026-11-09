/*
 * Name: tsmTspec_wms.c
 *
 * Description: 
 *   Contains routines for manipulating tspec files for WMS datasets.
 *   These functions are private to the tsmApi.
 *
 * Function List:
 *   Pyramid *tsmReadTspec_wms( char *pathname, TsmConnectionParams params )
 *
 * Revision History:
 *   Created 15 Jun 2001 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTspec_wms.c,v 1.1.2.5 2001/07/09 19:04:09 leei Exp $
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
#include "tsmConnectionParamsInt.h"
#include "tsmWMS.h"

/*
 * Synopsis:
 *    int tsmDoesTspecExist_wms( char *pathname )
 *
 * Description:
 *    This function will test to see if either a tiles.tspec or a
 *    tile-set.tspec file exists in the specified directory.
 *
 * Returns: TRUE if tspec file(s) exist, FALSE otherwise.
 *
 * Author: Martin Reddy
 *
 * Date: 10 March 1998
 *
 */

int
tsmDoesTspecExist_wms( char *pathname )
{
  char *name;

  if ( pathname == NULL ) return FALSE;

  /* check that the URL contains a layer=<name> parameter */

  name = tsmWMSExtractParam( pathname, "layers" );
  if ( name == NULL )
    name = tsmWMSExtractParam( pathname, "layer" );
  if ( name == NULL )
    return FALSE;

  free( name );
  return TRUE;
}


/*
 * Synopsis:
 *   Pyramid *tsmReadTspec_wms( char *pathname, TsmConnectionParams )
 *
 * Description:
 *   Queries a Web Mapping Server for its capabilities XML and then
 *   builds a tspec on the fly for the specified WMS layer. The 
 *   URL needs to be of the form http://server/wms?layer=name
 *
 * Returns:
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

Pyramid *
tsmReadTspec_wms( char *pathname, TsmConnectionParams params )
{
  static const char *funcName = "tsmReadTspec_wms";
  Pyramid           *pyramid;
  tsmWMS            *wms;
  tsmLayer          *layer;
  char              *name, *wms_url, *ptr;
  TsmTspecParams    tspec_params;
  TsmTileParams     tile_params;
  TsmDataSetType    dataset_type;
  int               width, height;

  if ( pathname == NULL ) return NULL;

  /* work out the WMS URL and the name of the layer */

  name = tsmWMSExtractParam( pathname, "layers" );
  if ( name == NULL )
    name = tsmWMSExtractParam( pathname, "layer" );

  if ( name == NULL ) {
    TSMTRACE(( TSMMASK|1, funcName, "No layer name in URL %s", pathname ));
    return NULL;
  }

  if ( ( wms_url = tsmWMSExtractUrl( pathname ) ) == NULL ) {
    TSMTRACE(( TSMMASK|1, funcName, "Not a valid WMS URL %s", pathname ));
    return NULL;
  }

  /* have we been supplied with a WMS capabilities structure? */

  if ( tsmIsParamSet( params, TSM_PARAM_WMS ) == TRUE ) {
    wms = params->wms;
  } else {
    if ( ( wms = tsmWMSCreate( pathname ) ) ) {
      if ( tsmWMSGetCapabilities( wms ) == FALSE ) {
	TSMTRACE(( TSMMASK|1, funcName, "Could not read WMS %s", pathname ));
	free( wms );
	return NULL;
      }
    }
  }

  /* find the named layer in the WMS capabilities structure */

  if ( ( layer = tsmWMSFindLayer( wms, name ) ) == NULL ) {
    TSMTRACE(( TSMMASK|1, funcName, "No layer %s on WMS %s", name, pathname ));
    return NULL;
  }

  /* work out if we are browsing an OI or a DEM over this WMS */

  dataset_type = TSM_IMAGE_DATASET;
  if ( strstr( name, ".dem" ) != NULL ) 
    dataset_type = TSM_DEM_DATASET;

  /* Allocate the tile and tspec parameter structures */

  tsmInitTileParams( &tile_params, dataset_type );
  tsmInitTspecParams( &tspec_params, dataset_type );

  /* fill in the easy stuff from the layer information */

  tspec_params.start_res   = layer->resx;
  if ( layer->abstract ) tspec_params.description = layer->abstract;

  width  = floor(0.5 + (layer->uv_bbox[2] - layer->uv_bbox[0]) / layer->resx);
  height = floor(0.5 + (layer->uv_bbox[3] - layer->uv_bbox[1]) / layer->resy);

  tspec_params.height = tile_params.height = height;
  tspec_params.width  = tile_params.width  = width;

  /* create the geoname, lobbing off the [Image] or [DEM] bits */
  /* that the SRI Web Map Server adds to differentiate datasets */

  tspec_params.geoName = strdup( layer->title );
  if ( ( ptr = strchr( tspec_params.geoName, '[' ) ) != NULL ) {
    if ( isspace( *(ptr-1) ) ) ptr--;
    *ptr = '\0';
  }

  /* translate the WMS-style georeference info into tspec-speak */

  if ( layer->srs == 4326 /* WGS84 geodetic */ ) {
    tspec_params.coord_sys = TSM_LATLONG_COORDSYS;
    tspec_params.ll_lon = layer->ll_bbox[0];
    tspec_params.ll_lat = layer->ll_bbox[1];

  } else if ( layer->srs > 32600 && layer->srs <= 32760 ) {
    tspec_params.coord_sys = TSM_UTM_COORDSYS;
    tspec_params.ll_e = layer->uv_bbox[0];
    tspec_params.ll_n = layer->uv_bbox[1];
    
    if ( layer->srs > 32700 /* southern hemi zone */ ) {
      tspec_params.zone = layer->srs - 32700;
      tspec_params.ll_n = -fabs( tspec_params.ll_n );
    } else /* northern hemi zone */ {
      tspec_params.zone = layer->srs - 32600;
    }
    
  } else {
    tsmError( funcName, "Unknown SRS %d for layer %s on WMS %s", layer->srs,
	      layer->name, wms_url );
    return NULL;
  }

  /* XXX - tiles are hardwired to 128x128 pixels right now  */
  /* because the WMS specification has no way of telling us */
  /* anything about the internal tiling structure of a map. */

  tile_params.tile_height = tile_params.tile_width = 128;
  
  /* create the tspec structure on the fly */

  pyramid = tspCreatePyramid( layer->name, (dataset_type == TSM_IMAGE_DATASET)
			      ? tspOiType : tspDemType, tspec_params,
			      tile_params );

  if ( pyramid == NULL ) {
    TSMTRACE(( TSMMASK|1, funcName, "Could not create tspec for layer %s ",
	       "on WMS %s", name, pathname ));
    return NULL;
  }

  /* If a geoName was not specified, then try to make one up */

  tsmEnsureGeoNameSet( pyramid );

  /* free the wms structure if we allocated it here */

  if ( tsmIsParamSet( params, TSM_PARAM_WMS ) == FALSE )
    tsmWMSFree( &wms );

  return pyramid;
}

