/*
 * Name: tsmDatabase_wms.c
 *
 * Description:
 *   Contains routines for managing the dataset database file from
 *   data located on a remote OGC Web Mapping Server (WMS)
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Function List:
 *    NameUrlIds *tsmReadServer_wms( Purl const *purl )
 *
 * Dependencies:
 *
 * Revision History:
 *    18 June 2001 - Initial version by Martin Reddy
 *
 *    $Id: tsmDatabase_wms.c,v 1.1.2.3 2002/03/13 23:40:55 leei Exp $
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
#include "tsmWMS.h"

/*
 * Synopsis:
 *   int NameUrlIds *tsmReadServer_wms( Purl const *url )
 *
 * Description:
 *   Read all TerraVision datasets on an OGC Web Map Server.
 *
 * Returns: A list of available datasets, or NULL if couldn't read directory.
 *
 * Author: Martin Reddy
 *
 * Date: 18 June 2001
 *
 */

NameUrlIds *
tsmReadServer_wms( Purl const *purl )
{
  const char  *funcName = "tsmReadServer_wms";
  NameUrlIds  *nameUrlIds;
  int         count = 0, i;
  tsmWMS      *wms;
  
  if ( purl == NULL ) return NULL;

  /* Create a tsmWMS instance and read the Capabilities XML of the WMS */
  
  if ( ( wms = tsmWMSCreate( purl->url ) ) == NULL ) return NULL;

  if (! tsmWMSGetCapabilities( wms ) ) {
    tsmWMSFree( &wms );
    return NULL;
  }

  /* allocate an empty database which we will read values into */
  
  if ( ( nameUrlIds = tsmMakeNameUrlIds( 0 ) ) == NULL ) {
    tsmError( funcName, "Unable to allocate server database array" );
    return NULL;
  }

  /* now loop through each layer in the WMS and create an entry for it */

  for ( i = 0; i < wms->numLayers; i++ ) {
    tsmLayer *layer = &wms->layer[i];

    tsmAddNameUrlId( nameUrlIds, tsmMemStrdup( layer->name ),
		     tsmWMSDatasetUrl( wms->url, layer->name ), count++ );

    TSMTRACE(( TSMMASK|2, funcName, "Dataset: %s", layer->name ));

  }

  tsmWMSFree( &wms );

  return nameUrlIds;
}
