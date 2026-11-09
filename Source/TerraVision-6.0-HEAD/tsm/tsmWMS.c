/*
 * Name: tsmWMS.c
 *
 * Description: 
 *   Contains user-level routines for interfacing with OpenGIS Consortium
 *   (OGC) compliant Web Map Servers (WMS).
 *
 * Function List:
 *
 * Revision History:
 *   Created 13 Jun 2001 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmWMS.c,v 1.1.2.7 2002/01/09 00:42:07 reddy Exp $
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

#include "tsmWMS.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

typedef xmlChar const *xmlCharPtr;

char *
tsmWMSExtractUrl( char *url )
{
  char *qmark, *result;

  if ( ! url ) return NULL;
  result = strdup( url );
  if ( ( qmark = strchr( result, '?' ) ) ) *qmark = '\0';
  return result;
}

char *
tsmWMSDatasetUrl( char *url, char *layer_name )
{
  char buffer[1024], *wms_url;
  
  if ( url == NULL || layer_name == NULL ) return NULL;

  wms_url = tsmWMSExtractUrl( url );
  sprintf( buffer, "%s?LAYERS=%s", wms_url, layer_name );
  free( wms_url );

  return strdup( buffer );
}

char *
tsmWMSExtractParam( char *url, char *param )
{
  int i, j, len;
  char *URL, *PARAM, buffer[512], *ptr, *result = NULL;

  if ( url == NULL || param == NULL ) return NULL;

  /* take a copy of the url and a copy of the param (plus an '=') */

  URL   = strdup( url );
  PARAM = malloc( strlen( param ) + 2 );
  len = strlen( param );
  memcpy( PARAM, param, len );
  PARAM[len] = '=';
  PARAM[len+1] = '\0';

  /* make both our copies uppercase to make string compares case insensitive */

  for ( i = 0, len = strlen(URL); i < len; i++ )
    URL[i] = toupper( URL[i] );
  for ( i = 0, len = strlen(PARAM); i < len; i++ )
    PARAM[i] = toupper( PARAM[i] );

  /* look for the parameter in the URL */

  if ( ( ptr = strstr( URL, PARAM ) ) != NULL ) {
    i = strlen(PARAM) + (int) (ptr - URL);
    for ( j = 0; url[i] != '&' && url[i] >= ' '; )
      buffer[j++] = url[i++];
    buffer[j] = '\0';
    result = strdup( buffer );
  }

  free( URL );
  free( PARAM );
  return result;
}

/*
 * initialise a tsmWMS structure, given a URL to the WMS service
 */

tsmWMS *
tsmWMSCreate( char *url )
{
  tsmWMS *wms = calloc( sizeof(tsmWMS), 1 );
  if ( wms ) {
    wms->url = tsmWMSExtractUrl( url );
  }
  return wms;
}

/*
 * free a tsmWMS structure and set the pointer to NULL
 */

void
tsmWMSFree( tsmWMS **wms_ptr )
{
  int i;
  tsmWMS *wms;
  if ( wms_ptr == NULL || ( wms = *wms_ptr ) == NULL ) return;
  if ( wms->url ) free( wms->url );
  if ( wms->getmap_url ) free( wms->getmap_url );
  if ( wms->title ) free( wms->title );
  if ( wms->error ) free( wms->error );
  if ( wms->version ) free( wms->version );

  for ( i = 0; i < wms->numFormats; i++ )
    free( wms->format[i] );
  free( wms->format );

  for ( i = 0; i < wms->numLayers; i++ ) {
    tsmLayer *layer = &wms->layer[i];
    if ( layer->name ) free( layer->name );
    if ( layer->title ) free( layer->title );
    if ( layer->abstract ) free( layer->abstract );
  }
  free( wms->layer );

  free( wms );
  *wms_ptr = NULL;
}

/*
 * Convert all whitespace to spaces (i.e. remove pesky newlines and others)
 */

static char *
tsmWMSRemoveNewLines( char *string )
{
  char *ptr = string;
  if ( string == NULL ) return NULL;
  while ( *ptr != '\0' ) {
    if ( isspace( *ptr ) ) *ptr = ' ';
    ptr++;
  }
  return string;
}

/*
 * add a layer to a tsmWMS structure. The layer information is copied
 * so you can do what you want with the tsmLayer afterwards.
 */

int
tsmWMSAddLayer( tsmWMS *wms, tsmLayer *layer )
{
  tsmLayer *new_layer = NULL;

  if ( wms == NULL ) return FALSE;

  if ( wms->layer == NULL ) {
    wms->numLayers = 1;
    wms->layer = malloc( sizeof( tsmLayer ) );
  } else {
    wms->numLayers++;
    wms->layer = realloc( wms->layer, sizeof( tsmLayer ) * wms->numLayers );
  }

  if ( wms->layer == NULL ) return FALSE;

  new_layer = &wms->layer[wms->numLayers-1];

  /* do a shallow copy first */

  memcpy( new_layer, layer, sizeof( tsmLayer ) );

  /* followed by a deep copy of the string pointers */

  if ( layer->name )
    new_layer->name = tsmWMSRemoveNewLines( strdup( layer->name ) );
  if ( layer->title )
    new_layer->title = tsmWMSRemoveNewLines( strdup( layer->title ) );
  if ( layer->abstract )
    new_layer->abstract = tsmWMSRemoveNewLines( strdup( layer->abstract ) );

  /* remove charater codes like new lines from the strings */

  return TRUE;
}

/*
 * search for a layer in this WMS and return it if found
 */

tsmLayer *
tsmWMSFindLayer( tsmWMS *wms, char *name )
{
  int i;

  for ( i = 0; i < wms->numLayers; i++ ) {
    tsmLayer *layer = &wms->layer[i];
    if ( strcmp( layer->name, name ) == 0 || strcmp( layer->title, name ) == 0)
      return layer;
  }

  return NULL;
}

/*
 * Add a new format string to the list of formats 
 */

static int
tsmWMSAddFormat( tsmWMS *wms, char *format )
{
  const int max_formats = 20;
  
  if ( wms == NULL ) return FALSE;
  if ( wms->numFormats >= max_formats ) return FALSE;

  if ( wms->format == NULL ) {
    wms->format = malloc( sizeof(char *) * max_formats );
    wms->numFormats = 0;
  }

  wms->format[wms->numFormats++] = strdup( format );

  return TRUE;
}

/*
 * Work out if we have a format (semantic or mime type). Returns
 * valid index to format array if found, or -1 if not found.
 */

static int
tsmWMSCheckFormat( tsmWMS *wms, char *format, char *mime_type )
{
  int i;

  for ( i = 0; i < wms->numFormats; i++ )
    if ( strcmp( wms->format[i], format ) == 0 ||
	 strcmp( wms->format[i], mime_type ) == 0 )
      return i;

  return -1;
}

/*
 * Takes a NULL terminated buffer and works out if this contains 
 * a default WMS Service Exception message. Returns NULL if no
 * exception, otherwise returns the error message string.
 */

char *
tsmWMSGetException( char *buffer )
{
  char *start, *end, *result, tmp;
  if ( ( start = strstr( buffer, "<WMTException" ) ) ) {
    if ( ( start = strchr( start, '>' ) ) == NULL ) return NULL;
    while ( isspace(*++start) );
    end = strstr( start, "</WMTException" );
    if ( end == NULL ) return NULL;
    while ( isspace( *(end-1) ) ) end--;
    tmp = *end;
    *end = '\0';
    result = strdup( start );
    *end = tmp;
    return result;

  } else if ( ( start = strstr( buffer, "<ServiceException " ) ) ) {
    if ( ( start = strchr( start, '>' ) ) == NULL ) return NULL;
    while ( isspace(*++start) );
    end = strstr( start, "</ServiceException" );
    if ( end == NULL ) return NULL;
    while ( isspace( *(end-1) ) ) end--;
    tmp = *end;
    *end = '\0';
    result = strdup( start );
    *end = tmp;
    return result;

  } else
    return NULL;
}

/*
 * Takes a string of the form "x.y" or "x.y.z" and returns a floating
 * point representation of that string for easy version number comparisons
 */

double
tsmWMSVersion( char *version )
{
  double value = 1.0;
  char   *dot, *ptr, buffer[512];

  if ( version == NULL ) return value;

  strcpy( buffer, version );
  ptr = buffer;

  if ( ( dot = strchr( ptr, '.' ) ) == NULL )
    return value;

  *dot = '\0';
  value = (double) atoi( ptr );
  ptr = ++dot;

  if ( ( dot = strchr( ptr, '.' ) ) == NULL )
    return value + ( (double) atoi( ptr ) ) / 100.0;

  *dot = '\0';
  value += ( (double) atoi( ptr ) ) / 100.0;
  ptr = ++dot;

  if ( ( dot = strchr( ptr, '.' ) ) != NULL )
    *dot = '\0';

  value += ( (double) atoi( ptr ) ) / 10000.0;

  return value;
}

/*
 * Parse out the EPSG code from the SRS attribute.
 * Returns 0 if the code could not be parsed.
 */

int 
tsmWMSParseSRS( char *srs )
{
  int  i = 0;
  if ( ! srs ) return 0;
  if ( strncmp( srs, "EPSG:", 5 ) == 0 )
    i = atoi( srs+5 );
  return i;
}

/*
 * Search through all the siblings from this node, looking 
 * for a named XML node. Return the node or NULL.
 */

static xmlNodePtr
xmlFindNode( xmlNodePtr node, char const *name )
{
  while ( node && strcmp( name, (char *) node->name ) != 0 )
    node = node->next;
  while ( node && strcmp( name, (char *) node->name ) != 0 )
    node = node->prev;
  return node;
}

/*
 * Return the text part of a node, e.g. Fred from <tag>Fred</tag>
 */

static char *
xmlGetNodeText( xmlNodePtr node )
{
  if ( node == NULL ) return NULL;
  return strdup( (char *) xmlNodeGetContent( node ) );
}

/*
 * Return the named attribute string for a node, or NULL if not exist 
 */

static char *
xmlGetNodeAttr( xmlNodePtr node, char *name )
{
  char *prop;
  if ( node == NULL ) return NULL;
  prop = (char *) xmlGetProp( node, (const xmlChar *) name );
  return ( prop ) ? strdup( prop ) : NULL;
}

/*
 * read the capabilities XML of the WMS in order to work out the
 * appropriate version number and the list of layers available 
 */

int
tsmWMSGetCapabilities( tsmWMS *wms )
{
  const char funcName[] = "tsmWMSGetCapabilities";
  char url[1024], *buffer, *string, *error;
  http_trans_conn *conn = NULL;
  int  size, do_add;
  xmlDocPtr doc;
  xmlNodePtr node, tmpnode, global_srs, global_scalehint;

  if ( ! wms ) return FALSE;

  /* clear any previous error */

  if ( wms->error ) {
    free( wms->error );
    wms->error = NULL;
  }

  /* get the WMS's Capabilities XML */

  sprintf( url, "%s?REQUEST=GetCapabilities&VERSION=1.1", wms->url );
  if ( ! tsmUrlToMemory( &conn, url, &buffer, &size ) ) {
    sprintf( url, "%s?REQUEST=capabilities&WMTVER=1.0.0", wms->url );
    if ( ! tsmUrlToMemory( &conn, url, &buffer, &size ) ) {
      wms->error = strdup( "Could not contact server for Capabilities XML" );
      return FALSE;
    }
  }

  if ( conn ) tsmCloseHttpConnection( conn );

  /* null terminate the string into a buffer called 'string' */

  if ( ( string = malloc( size + 1 ) ) == NULL ) return FALSE;
  memcpy( string, buffer, size );
  string[size] = '\0';
  free( buffer );

  /* check for a Service Exception */

  error = tsmWMSGetException( string );
  if ( error != NULL ) {
    wms->error = error;
    return FALSE;
  }

  /* parse the XML */

  if ( ( doc = xmlParseMemory( string, size ) ) == NULL ) {
    free( string );
    return FALSE;
  }

  free( string );

  /* find the root <WMT_MS_Capabilities> tag and get the version number */

  node = xmlDocGetRootElement( doc );
  wms->version = xmlGetNodeAttr( node, "version" );

  /* get the <Capability> tag */

  node = xmlFindNode( node->children, "Capability" );
  tmpnode = xmlFindNode( node->children, "Request" );

  if ( tsmWMSVersion( wms->version ) >= tsmWMSVersion( "1.0.8" ) ) {

    /* extract all of the <Format>mime_type</Format> definitions */

    xmlNodePtr getmap = xmlFindNode( tmpnode->children, "GetMap" );
    xmlNodePtr format = getmap->children;
    while ( format ) {
      
      if ( strcmp( (char *) format->name, "Format" ) == 0 ) {
	char *name = xmlGetNodeText( format );
	if ( name ) {
	  if ( strcmp( name, "text" ) != 0 )
	    tsmWMSAddFormat( wms, name );
	  free( name );
	}
      } else if ( (char *) strcmp( (char *) format->name, "DCPType" ) == 0 ) {

	/* extract the GetMap request URL for HTTP GET operations */

	xmlNodePtr http = xmlFindNode( format->children, "HTTP" );
	http = xmlFindNode( http->children, "Get" );
	http = xmlFindNode( http->children, "OnlineResource" );
	wms->getmap_url = xmlGetNodeAttr( http, "href" );
      }
      format = format->next;
    }

  } else {

    /* extract all of the <Format><TYPE/></Format> definitions */

    xmlNodePtr getmap = xmlFindNode( tmpnode->children, "Map" );
    xmlNodePtr format = xmlFindNode( getmap->children, "Format" );
    format = format->children;
    while ( format ) {
      if ( strcmp( (char *) format->name, "text" ) != 0 )
	tsmWMSAddFormat( wms, (char *) format->name );
      format = format->next;
    }

    /* extract the Map request URL for HTTP GET operations */

    format = xmlFindNode( getmap->children, "DCPType" );
    if ( format ) {
      xmlNodePtr http = xmlFindNode( format->children, "HTTP" );
      http = xmlFindNode( http->children, "Get" );
      wms->getmap_url = xmlGetNodeAttr( http, "onlineResource" );
    }
  }

  /* if no HTTP GET url is specified, use the one we used to contact server */

  if ( wms->getmap_url == NULL )
    wms->getmap_url = strdup( wms->url );

  /* work out if we have a format that we can use - favour JPEG */

  wms->mapFormat   = tsmWMSCheckFormat( wms, "JPEG", "image/jpeg" );
  wms->mapFormatID = TSM_WMS_FORMAT_JPEG;

  /*
  wms->mapFormat   = tsmWMSCheckFormat( wms, "LRBT", "image/x-raw-lrbt" );
  wms->mapFormatID = TSM_WMS_FORMAT_LRBT;
  */

  if ( wms->mapFormat < 0 ) {
    wms->mapFormat   = tsmWMSCheckFormat( wms, "LRBT", "image/x-raw-lrbt" );
    wms->mapFormatID = TSM_WMS_FORMAT_LRBT;
  }

  if ( wms->mapFormat < 0 ) {
    TSMTRACE(( TSMMASK|1, funcName, "Cannot find a suitable image format" ));
    xmlFreeDoc( doc );
    return FALSE;
  }

  /* Get the root <Layer> node */

  node = xmlFindNode( node->children, "Layer" );
  global_srs = xmlFindNode( node->children, "SRS" );
  global_scalehint = xmlFindNode( node->children, "ScaleHint" );
  tmpnode = xmlFindNode( node->children, "Title" );
  wms->title = xmlGetNodeText( tmpnode );

  /* Get the start of our layer list */
  
  for ( node = node->children; node; node = node->next ) {
    tsmLayer layer;
    char     *prop;

    if ( strcmp( (char *) node->name, "Layer" ) != 0 ) continue;

    memset( &layer, 0, sizeof( tsmLayer ) );

    tmpnode = xmlFindNode( node->children, "Name" );
    layer.name = xmlGetNodeText(tmpnode);

    tmpnode = xmlFindNode( node->children, "Title" );
    layer.title = xmlGetNodeText(tmpnode);

    tmpnode = xmlFindNode( node->children, "Abstract" );
    layer.abstract = xmlGetNodeText(tmpnode);

    tmpnode = xmlFindNode( node->children, "SRS" );
    if ( tmpnode == NULL ) tmpnode = global_srs;
    prop = xmlGetNodeText(tmpnode);
    layer.srs = tsmWMSParseSRS( prop );
    if ( prop ) free( prop );

    tmpnode = xmlFindNode( node->children, "ScaleHint" );
    if ( tmpnode == NULL ) tmpnode = global_scalehint;
    if ( ( prop = xmlGetNodeAttr( tmpnode, "min" ) ) ) {
      layer.scaleHint[0] = atof( prop );
      free( prop );
    }
    if ( ( prop = xmlGetNodeAttr( tmpnode, "max" ) ) ) {
      layer.scaleHint[1] = atof( prop );
      free( prop );
    }

    tmpnode = xmlFindNode( node->children, "LatLonBoundingBox" );
    if ( ( prop = xmlGetNodeAttr( tmpnode, "minx" ) ) ) {
      layer.ll_bbox[0] = atof( prop );
      free( prop );
    }
    if ( ( prop = xmlGetNodeAttr( tmpnode, "miny" ) ) ) {
      layer.ll_bbox[1] = atof( prop );
      free( prop );
    }
    if ( ( prop = xmlGetNodeAttr( tmpnode, "maxx" ) ) ) {
      layer.ll_bbox[2] = atof( prop );
      free( prop );
    }
    if ( ( prop = xmlGetNodeAttr( tmpnode, "maxy" ) ) ) {
      layer.ll_bbox[3] = atof( prop );
      free( prop );
    }
    
    tmpnode = xmlFindNode( node->children, "BoundingBox" );
    if ( tmpnode ) {

      if ( ( prop = xmlGetNodeAttr( tmpnode, "minx" ) ) ) {
	layer.uv_bbox[0] = atof( prop );
	free( prop );
      }
      if ( ( prop = xmlGetNodeAttr( tmpnode, "miny" ) ) ) {
	layer.uv_bbox[1] = atof( prop );
	free( prop );
      }
      if ( ( prop = xmlGetNodeAttr( tmpnode, "maxx" ) ) ) {
	layer.uv_bbox[2] = atof( prop );
	free( prop );
      }
      if ( ( prop = xmlGetNodeAttr( tmpnode, "maxy" ) ) ) {
	layer.uv_bbox[3] = atof( prop );
	free( prop );
      }
      if ( ( prop = xmlGetNodeAttr( tmpnode, "resx" ) ) ) {
	layer.resx = atof( prop );
	free( prop );
      }
      if ( ( prop = xmlGetNodeAttr( tmpnode, "resy" ) ) ) {
	layer.resy = atof( prop );
	free( prop );
      }

    } else {

      /* no BoundingBox node, assume it is the same as LatLongBoundingBox */

      layer.uv_bbox[0] = layer.ll_bbox[0];
      layer.uv_bbox[1] = layer.ll_bbox[1];
      layer.uv_bbox[2] = layer.ll_bbox[2];
      layer.uv_bbox[3] = layer.ll_bbox[3];
      layer.resx = 0.0;
      layer.resy = 0.0;
    }

    /* do we need to do some math to work out some of our required info? */

    if ( layer.resx == 0.0 || layer.resy == 0.0 ) {
      if ( layer.scaleHint[0] > 0.0 ) {
	/* assume that the min ScaleHint value is the dataset resolution */
	layer.resx = layer.scaleHint[0];
	/* convert to degrees if using geodetic coords (use WGS84 ellipsoid) */
	if ( layer.srs == 4326 )
	  layer.resx = ( layer.resx * 360.0 ) / 6378137.0 * 2.0 * 3.14159265;
	layer.resy = layer.resx;

	TSMTRACE(( TSMMASK|1, funcName, "Caution: using ScaleHint tag to "
		   "estimate resolution of layer %s (res = %1.10lg)",
		   layer.title, layer.resx ));
      } else {
	/* no resolution information! Let's just assume a default */
	layer.resx = ( layer.uv_bbox[2] - layer.uv_bbox[0] ) / 4096.0;
	layer.resy = layer.resx;

	TSMTRACE(( TSMMASK|1, funcName, "Caution: I'm going to make up "
		   "numbers for the resolution of layer %s (res = %1.10lg)",
		   layer.title, layer.resx ));
      }
    }

    /* do we have enough information? */

    do_add = TRUE;

    if ( layer.title == NULL || layer.name == NULL ) {
      TSMTRACE(( TSMMASK|1, funcName, "Layer has no name - ignoring" ));
      do_add = FALSE;
    }

    if ( layer.resx == 0.0 || layer.resy == 0.0 ) {
      TSMTRACE(( TSMMASK|1, funcName, "No resolution specified for %s",
		 layer.title ));
      do_add = FALSE;
    }

    if ( layer.resx != layer.resy ) {
      TSMTRACE(( TSMMASK|1, funcName, "Anisotropic resolution for %s",
		 layer.title ));
      do_add = FALSE;
    }

    /* we're all good to go - add this layer to our list */

    if ( do_add ) {
      tsmWMSAddLayer( wms, &layer );
      TSMTRACE(( TSMMASK|1, funcName, "Add WMS Layer %s\n", layer.title ));
    }

    if ( layer.name ) free( layer.name );
    if ( layer.title ) free( layer.title );
    if ( layer.abstract ) free( layer.abstract );
    layer.name = layer.title = layer.abstract = NULL;
  }

  xmlFreeDoc( doc );
  return TRUE;
}
