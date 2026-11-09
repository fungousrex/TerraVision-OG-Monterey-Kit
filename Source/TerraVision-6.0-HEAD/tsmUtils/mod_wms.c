/*
 * Name: mod_wms.c
 *
 * Description:
 *     This is an Apache mod that should be compiled up to a shared
 *     object (.so file) that can be copied into the libexec directory
 *     of the Apache server. It manages the serving of tile data from
 *     TerraVision datasets over an http connection using the Open
 *     GIS Consortium's (OGCs) Web Mapping Server (WMS) Interface
 *     Specification.
 *
 *     This program uses the Apache include files.
 *
 *     Current additions to the Apache conf/httpd.conf file are:
 *         
 *        LoadModule wms_module libexec/mod_wms.so
 *        AddModule mod_wms.c
 *
 *     In addition to the standard OGC WMS parameters, this WMS
 *     supports the following vendor-specific options:
 *
 *       OVERLAP=   overlap pixels to the left, bottom, right, top
 *       CHECKOPT=  will tell you if the map request will be optimized
 *       CHECKMOD=  tests for the existence of this WMS
 *       LAYERINFO= for a valid map request, tells you about the layer instead
 *       WMSINFO=   show a simple listing of server capabilities
 *       DEBUG=     turn on server side debugging for the Apache module
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 6 June 2001.
 *
 * Revision Information:
 *
 *     $Id: mod_wms.c,v 1.1.2.9 2002/03/18 01:16:14 reddy Exp $
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <tsm/tsm.h>
#include <tsm/tsmWMS.h>
#include <tsmUtils/terraserver.h>
#include <prinit.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_main.h"
#include "util_script.h"
#include "util_md5.h"

static int  wms_strstr( char const *str1, char const *str2 );
static void wms_dataset_bbox( TsmConnection, double bbox[4], int, int );
static int  wms_dataset_valid( TsmDataSet *dataset );
static void wms_remove_spaces( char *name );
static char *wms_get_string( request_rec *r, char const *opt );
static int  wms_get_srs( TsmDataSet *dataset );
static void wms_output_raw( request_rec *, TsmConnection, uchar *, int, int );
static void wms_output_gif( request_rec *, TsmConnection, uchar *, int, int );
static void wms_output_jpeg( request_rec *, TsmConnection, uchar *, int, int );
static void wms_output_lrbt( request_rec *, TsmConnection, uchar *, int, int );
static void wms_output_ppm( request_rec *, TsmConnection, uchar *, int, int );
static void wms_output_geovrml( request_rec *, TsmConnection, uchar *,int,int);
extern int  tsmSaveImageGIF( char *, unsigned char *, int, int );
extern int  tsmFootprintToLatLong( Footprint *in, Footprint *out );

#define MAX_CONNECTS     8     /* max number of cached dataset connections */

#define JFIF_MAGIC_SIZE  20       /* the byte size of the JFIF file header */

#define PROJ_F           2.41421356237309490              /* 1 / tan(45/2) */
#define PROJ_SCR         512.0                        /* min(width,height) */

#define WMS_CACHE_SIZE   12        /* number of TerraVision tiles to cache */

char *wms_server_url = NULL;                             /* see wms_init() */

module MODULE_VAR_EXPORT wms_module;                  /* needed for Apache */

typedef struct {
  TsmConnection connect;
  char const    *dataset;
} wms_state_elem;

typedef void (*output_func)( request_rec *, TsmConnection, uchar *, int, int );

typedef struct {
  char          *format;
  char          *mime;
  output_func   func;
} wms_format;

typedef struct {
  int           x, y, level;     /* key for this cache entry */
  TsmConnection connect;         /* connection pointer for this tile */
  uchar         *buffer;         /* buffer returned by tsmGetTile */
} wms_tile;

wms_tile        wms_cache[WMS_CACHE_SIZE];
int             wms_cache_next;

wms_state_elem  wms_state[MAX_CONNECTS];
int             wms_curr_state;
int             layers;
TsmDatabase     *database = NULL;
int             debug = FALSE;

/* list of support image formats, their mime types, and output functions */

static wms_format formats[] = {
  { "GIF",     "image/gif",        wms_output_gif },
  { "JPEG",    "image/jpeg",       wms_output_jpeg },
  { "PPM",     "image/ppm",        wms_output_ppm },
  { "RAW",     "image/x-raw-lrtb", wms_output_raw },
  { "LRBT",    "image/x-raw-lrbt", wms_output_lrbt },
  { "GEOVRML", "model/vrml",       wms_output_geovrml },
  NULL
};

/* list of supported WMS version strings */

static char *wms_max_version = "1.1";

/*
 * output a debug message to Apache's logs/error_log file. This will
 * only happen if you have compiled with -DDEBUG or if the client
 * specified DEBUG= on the URL parameter list for this request
 */

static void
wms_debug( const char *msg, ... )
{
  va_list ap;

#ifndef DEBUG
  if ( ! debug ) return;
#endif
  fprintf( stderr, "mod_wms: " );
  va_start( ap, msg );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
}

static int
wms_error( request_rec *r, const char *msg, ... )
{
  char buffer[4096];
  va_list ap;

  va_start( ap, msg );
  vsprintf( buffer, msg, ap );
  va_end( ap );

  wms_debug( buffer );

  r->content_type = "text/html";
  ap_send_http_header(r);
  
  ap_rprintf( r, "<HTML><BODY><H1>SRI Web Mapping Server (WMS) Error</H1>" );
  ap_rprintf( r, "Error: %s</BODY></HTML>", buffer );
  return OK;
}

static int
wms_exception( request_rec *r, const char *code, const char *msg, ... )
{
  char    buffer[4096], *wmtver;
  double  version;
  va_list ap;

  /* get the version number of this WMS request */

  wmtver = wms_get_string( r, "version" );
  if ( ! wmtver ) wmtver = wms_get_string( r, "wmtver" );
  version = tsmWMSVersion( wmtver );
  free( wmtver );

  /* expand out the printf style exception description */

  va_start( ap, msg );
  vsprintf( buffer, msg, ap );
  va_end( ap );

  /* send a debug message for output to the server's error log */

  wms_debug( buffer );

  /* output the MIME type header and version-specific Service Exception XML*/

  if ( version < tsmWMSVersion( "1.0.8" ) ) {

    /* WMS 1.0 specification Service Exception (no DTD) */

    r->content_type = "text/xml";
    ap_send_http_header(r);

    ap_rprintf( r, "<?xml version='1.0' encoding=\"UTF-8\" standalone=\"no\" ?>\n" );
    ap_rprintf( r, "<WMTException version=\"1.0.0\">\n" );
    ap_rprintf( r, "%s\n", buffer );
    ap_rprintf( r, "</WMTException>\n" );

  } else {

    /* WMS 1.0.8 specification Service Exception */

    r->content_type = "application/vnd.ogc.se_xml";
    ap_send_http_header(r);

    if ( code == NULL ) code = "Unspecified";
  
    ap_rprintf( r, "<?xml version='1.0' encoding=\"UTF-8\" standalone=\"no\" ?>\n" );
    ap_rprintf( r, "<!DOCTYPE ServiceExceptionReport SYSTEM\n" );
    ap_rprintf( r, "\"http://www.digitalearth.gov/wmt/xml/exception_1_1_0.dtd\">\n" );
    ap_rprintf( r, "<ServiceExceptionReport version=\"1.1.0\">\n" );
    ap_rprintf( r, "<ServiceException code=\"%s\">\n", code );
    ap_rprintf( r, "%s\n", buffer );
    ap_rprintf( r, "</ServiceException>\n" );
    ap_rprintf( r, "</ServiceExceptionReport>\n" );
  }

  return OK;
}

static void
wms_free_elem( wms_state_elem *elem )
{
  if ( elem == NULL ) return;
  if ( elem->connect ) tsmDisconnect( elem->connect );
  if ( elem->dataset ) tsmMemFree( elem->dataset );
  elem->connect = NULL;
  elem->dataset = NULL;
}

/*
 * Initialise the WMS capability. This must be the first
 * function that is called from this module.
 */

static void
wms_init( server_rec *s, pool *p )
{
  int i;
  char *config_file, buffer[1024];

  /* initialise the tile cache */

  for ( i = 0; i < WMS_CACHE_SIZE; i++ )
    wms_cache[i].buffer = NULL;
  
  wms_cache_next = 0;

  /* Initialise the NSPR library */

  if ( PR_Initialized() == PR_FALSE )
    PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);

  /* initialise the WMS server name for the capabilities XML */

  sprintf( buffer, "http://%s/wms", s->server_hostname );
  wms_server_url = strdup( buffer );

  /* load the TerraVision config file for this WMS */

  config_file = ap_server_root_relative( p, "conf/wms.conf" );
  if ( ! config_file )
    wms_debug( "Could not determine WMS config file!" );
  else
    wms_debug( "WMS config file is %s", config_file );
  
  if ( ( database = tsmReadDatabase( config_file ) ) == NULL )
    wms_debug( "Could not open WMS config database!" );
  else
    wms_debug( "WMS config database opened with %d datasets",
		     database->numDataSets );

  /* count the number of valid layers and canonicalize the names */

  for ( i = 0; i < database->numDataSets; i++ ) {
    if ( wms_dataset_valid( &database->dataset[i] ) )
      layers++;
    wms_remove_spaces( database->dataset[i].name );
  }
  
  /* initialise the array of connection structures to be empty */

  wms_debug( "initialising connection array for pid %d", getpid() );

  wms_curr_state = 0;

  for ( i = 0; i < MAX_CONNECTS; i++ ) {
    wms_state[i].connect = NULL;
    wms_state[i].dataset = NULL;
  }

  /* initialise the TerraServer proxy module */

  ts_init();

}

/*
 * Cleanup the WMS module when done
 */

static void
wms_shutdown( server_rec *s, pool *p )
{
  int i;

  /* free all of our cached tiles */

  for ( i = 0; i < WMS_CACHE_SIZE; i++ )
    if ( wms_cache[i].buffer )
      free( wms_cache[i].buffer );

  /* disconnect from all of the datasets that we connected to */

  wms_debug( "cleaning up connection array for pid %d", getpid() );

  for ( i = 0; i < MAX_CONNECTS; i++ )
    wms_free_elem( &wms_state[i] );

  /* free the WMS server name that we created */

  if ( wms_server_url ) free( wms_server_url );

  /* closedown the TerraServer proxy module */

  ts_cleanup();

}

/*
 * Add a TerraVisoin tile to our cache 
 */

static int
wms_cache_add( TsmConnection connect, int x, int y, int level, uchar *buffer )
{
  wms_tile *cache = &wms_cache[wms_cache_next];

  /* de-cache existing tile at the next location */

  if ( cache->buffer != NULL )
    free( cache->buffer );

  /* fill it with the new information */

  cache->x       = x;
  cache->y       = y;
  cache->level   = level;
  cache->connect = connect;
  cache->buffer  = buffer;

  /* and then move the pointer onto the next location for next time round */

  wms_cache_next = ( wms_cache_next + 1 ) % WMS_CACHE_SIZE;

  return TRUE;
}

/*
 * Check the cache for a TerraVision tile. Returns TRUE if found and
 * sets the buffer and size fields with the relevant buffer details.
 * Otherwise, returns FALSE if tile is not in cache.
 */

static int
wms_cache_check( TsmConnection connect, int x, int y, int level,
		 uchar **buffer )
{
  int i;
  
  for ( i = 0; i < WMS_CACHE_SIZE; i++ ) {
    wms_tile *cache = &wms_cache[i];

    if ( cache->buffer == NULL ) continue;

    if ( cache->x == x && cache->y == y && cache->level == level &&
	 cache->connect == connect ) {
      *buffer = cache->buffer;
      return TRUE;
    }
  }

  return FALSE;
}

static int
wms_get_tile( TsmConnection connect, int x, int y, int level, uchar **buffer )
{
  /* check to see if we have this tile in our cache */

  if ( ! wms_cache_check( connect, x, y, level, buffer ) ) {

    /* no, so try to read the tile and cache it */

    if ( ts_is_terraserver( (char *) tsmGet( connect, TSM_DATASET_URL ) ) ) {

      /* request a TerraServer tile in TerraVision's tile size format */

      if ( ! ts_get_terraserver_tile( connect, x, y, level, buffer ) )
	return FALSE;

    } else {

      /* request a plain old TerraVision tile from the local dataset */
      
      if ( ! tsmAllocReadTile( connect, x, y, level, buffer ) )
	return FALSE;
    }

    /* add the tile to the tile cache */

    wms_cache_add( connect, x, y, level, *buffer );
  }
  
  return TRUE;
}

static char *
wms_get_string( request_rec *r, char const *opt )
{
  char buffer[128] = "", *inptr, *outptr = buffer;
  int  index = wms_strstr( r->args, opt );

  if ( index < 0 )
    return NULL;
  
  inptr = r->args + index + strlen(opt);
  if ( *inptr == '=' ) inptr++;
  while ( *inptr != '&' && *inptr != '\0' )
    *outptr++ = *inptr++;
  *outptr++ = '\0';
  
  return strdup( buffer );
}

static int
wms_get_int( request_rec *r, char const *opt )
{
  char *value = wms_get_string( r, opt );
  if ( value ) {
    int i = atoi( value );
    free( value );
    return i;
  } else
    return 0;
}

static int
wms_get_arg( request_rec *r, char const *opt )
{
  char *value = wms_get_string( r, opt );
  if ( value ) {
    free( value );
    return 1;
  } else
    return 0;
}

static char *
wms_lower_strdup( char const *str )
{
  char *s = strdup( ( str ) ? str : "" );
  int  i, len;
  
  for ( i = 0, len = strlen(s); i < len; i++ )
    s[i] = tolower(s[i]);
  
  return s;
}

static int
wms_strcmp( char const *str1, char const *str2 )
{
  char *s1 = wms_lower_strdup( str1 );
  char *s2 = wms_lower_strdup( str2 );
  int i = strcmp( s1, s2 );
  free( s1 );
  free( s2 );
  return ( i == 0 );
}

static int
wms_strstr( char const *str1, char const *str2 )
{
  char *s1 = wms_lower_strdup( str1 );
  char *s2 = wms_lower_strdup( str2 );
  char *match = strstr( s1, s2 );
  int  i = ( match ) ? match - s1 : -1;
  free( s1 );
  free( s2 );
  return i;
}

static char
dd2c( char d1, char d2 )
{
  unsigned char digit;

  digit = (d1 >= 'A' ? ((d1 & 0xdf) - 'A')+10 : (d1 - '0'));
  digit *= 16;
  digit += (d2 >= 'A' ? ((d2 & 0xdf) - 'A')+10 : (d2 - '0'));

  return (char) digit;
}

/*
 * Takes a URL string and decodes any + symbols to spaces and
 * %xx codes to their appropriate symbol. The conversion is
 * performed in-situ in the source string. We also strip any
 * leading whitespace characters.
 */

static void
wms_unescape_url( char *url )
{
  int i = 0, j = 0;

  if ( url == NULL ) return;

  /* skip leading whitespace */
  
  while ( isspace( url[i] ) ) i++;
  if ( url[i] == '\0' ) {
    url[0] = '\0';
    return;
  }

  /* transform each character */

  while ( url[i] != '\0' ) {
    switch ( url[i] ) {
    case '+':
      url[j++] = ' ';
      break;
    case '%':
      url[j] = dd2c( url[i+1], url[i+2] );
      i += 2;
      j++;
      break;
    default:
      url[j++] = url[i];
    }
    i++;
  }
  
  /* all done - terminate and return the new string */

  url[j] = '\0';
}

/*
 * Test whether a TerraVision dataset can be served by this WMS
 * Issues to consider are the coordinate system of the dataset.
 */

static int
wms_dataset_valid( TsmDataSet *dataset )
{
  if ( dataset == NULL ) return FALSE;
  if ( wms_get_srs( dataset ) == 0 ) return FALSE;

  /* XXXX - currently, this WMS will only server layers that  */
  /* have a tilesize of 128x128. This is because there is no  */
  /* way to legally advertize tile size in the WMS 1.0 or 1.1 */
  /* Capabilities XML. This will hopefully change soon!!      */

  if ( dataset->tilesize != 128 ) return FALSE;

  return TRUE;
}

/*
 * Does an in-situ replacement of all spaces in a string to underscores
 */

static void
wms_remove_spaces( char *name )
{
  if ( name == NULL ) return;
  while ( *name != '\0' ) {
    if ( isspace(*name) ) *name = '_';
    name++;
  }
}

/*
 * Open a connection to the specific TerraVision dataset, but check 
 * to see if we already have a cached connection open and use it if
 * that is the case. The MAX_CONNECTS value determines the size of
 * our connection cache for this server.
 */

static TsmConnection
wms_get_connection( request_rec *r, char const *dataset )
{
  int           i;
  TsmConnection connect;
  TsmConnectionParams params;

  /* first of all check to see if we have a cached connection */

  for ( i = 0; i < MAX_CONNECTS; i++ ) {
    if ( wms_state[i].dataset ) {
      if ( strcmp( wms_state[i].dataset, dataset ) == 0 ) {
	wms_debug( "Returning cached connection for pid %d", getpid() );
	return wms_state[i].connect;
      }
    }
  }

  /* otherwise create a new connection */

  /* No read multiplicity needed. */ 
  tsmInitParams (&params);
  tsmSetParam (params, TSM_PARAM_READ_MULTI, 1);

  if ( ( connect = tsmConnect( dataset, "r", params ) ) == NULL )
    return NULL;

  wms_debug( "Created a new connection for pid %d", getpid() );

  /* find a new cache slot for it - is the slot empty? If so, clean it */

  if ( wms_state[wms_curr_state].connect )
    wms_free_elem( &wms_state[wms_curr_state] );

  /* insert the new data for this connection into the cache */

  wms_state[wms_curr_state].connect = connect;
  wms_state[wms_curr_state].dataset = tsmMemStrdup( dataset );

  /* move the cache pointer onto the next element, round robin */

  wms_curr_state = ( wms_curr_state + 1 ) % MAX_CONNECTS;

  return connect;
}

static void
wms_debug_info(request_rec *r) 
{
  const char* hostname;
  int         i;
  
  r->content_type = "text/html";
  ap_send_http_header(r);
  hostname = ap_get_remote_host(r->connection, 
				r->per_dir_config, REMOTE_NAME);

  ap_rputs("<HTML>\n", r);
  ap_rputs("<HEADER>\n", r);
  ap_rputs("<TITLE>SRI WMS Information</TITLE>\n", r);
  ap_rputs("</HEADER>\n", r);
  ap_rputs("<BODY>\n", r);

  ap_rputs("<H1>SRI Web Mapping Server (WMS)</H1>\n", r);
  
  ap_rprintf(r, "<H2>Request Information</H1>\n");
  ap_rprintf(r, "<p><PRE>unparsed_uri = %s\n", r->unparsed_uri);
  ap_rprintf(r, "uri          = %s\n", r->uri);
  ap_rprintf(r, "filename     = %s\n", r->filename);
  ap_rprintf(r, "path_info    = %s\n", r->path_info);
  ap_rprintf(r, "args         = %s\n", r->args);
  ap_rprintf(r, "hostname     = %s\n\n", hostname);
  
  ap_rprintf(r, "</PRE><H2>Server Information</H1>\n");
  ap_rprintf(r, "<p><PRE>formats   =" );
  for ( i = 0; formats[i].format != NULL; i++ )
    ap_rprintf(r, " %s (%s)", formats[i].format, formats[i].mime );
  ap_rprintf(r, "\n" );
   
  ap_rprintf(r, "database  = %s\n", 
	     ( database ) ? "loaded" : "not available" );
  if ( database ) {
    int i, j;
    ap_rprintf(r, "layers    = %d\n", layers );
    for ( i = j = 0; i < database->numDataSets; i++ ) 
      if ( wms_dataset_valid( &database->dataset[i] ) )
	ap_rprintf(r, "layer %3d = %s\n", ++j, database->dataset[i].name );
  }

  ap_rputs("</PRE></BODY>\n", r);
  ap_rputs("</HTML>\n", r);
}

static void
wms_print_bbox( request_rec *r, double bbox[4] )
{
  ap_rprintf( r, "(%1.10lg, %1.10lg) to (%1.10lg, %1.10lg)", 
	      bbox[0], bbox[1], bbox[2], bbox[3] );
}

static void
wms_layer_info( request_rec *r, TsmConnection connect, char *name )
{
  Pyramid       *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  SimpleTileSet *tset = &tspec->tspecLevel.val[tspec->minLevel];
  TileTspec     *tile = &tspec->tileTspec;
  double        bbox[4];
  int           i;

  r->content_type = "text/html";
  ap_send_http_header(r);

  ap_rputs("<HTML>\n", r);
  ap_rputs("<HEADER>\n", r);
  ap_rputs("<TITLE>SRI WMS Information</TITLE>\n", r);
  ap_rputs("</HEADER>\n", r);
  ap_rputs("<BODY>\n", r);

  ap_rputs("<H1>SRI Web Mapping Server (WMS)</H1>\n", r);
  
  ap_rprintf(r, "<H2>Layer Information</H1>\n");
  ap_rprintf(r, "<p><PRE>layer name   = %s\n", name);
  ap_rprintf(r, "ground res   = %1.6g\n", tspec->minResolution );
  ap_rprintf(r, "image size   = %d x %d\n",
	     tset->numValidPixel[0], tset->numValidPixel[1] );
  ap_rprintf(r, "tile size    = %d x %d\n",
	     tile->numPixelsX, tile->numPixelsY );
  ap_rprintf(r, "tile overlap = 0, 0, %d, %d\n",
	     tile->overLap[0], tile->overLap[1] );
  ap_rprintf(r, "comps/pixel  = %d\n", tile->numComponentsPixel );
  ap_rprintf(r, "bytes/comp   = %d\n", tile->numBytesPerComponent );

  ap_rprintf(r, "bounding box = " );
  wms_dataset_bbox( connect, bbox, 0, TRUE );
  wms_print_bbox( r, bbox );
    ap_rprintf(r, "\n" );

  ap_rprintf(r, "levels       = %d\n", 
	     tspec->maxLevel - tspec->minLevel + 1 );

  for ( i = tspec->minLevel; i <= tspec->maxLevel; i++ ) {
    ap_rprintf(r, "level %d bbox = ", i );
    wms_dataset_bbox( connect, bbox, i - tspec->minLevel, FALSE );
    wms_print_bbox( r, bbox );
    ap_rprintf(r, "\n" );
  }

  ap_rputs("</PRE></BODY>\n", r);
  ap_rputs("</HTML>\n", r);
}

/*
 * Return the n'th specified entry in the giveb string,
 * for example, the LAYERS or the STYLES attribute.
 * Will return NULL if there is no n'th layer specified.
 * The first layer in the LAYERS attribute is at index 0.
 * Layers are separated in the string by a comma character.
 * If a non-NULL result is returned, client must free
 * the string when finished.
 */

char *
wms_get_from_list( request_rec *r, char *listname, int i )
{
  char *start, *string, buffer[512], *ptr;

  if ( i < 0 ) return NULL;
  string = start = wms_get_string( r, listname );
  if ( start == NULL ) return NULL;

  for ( ; i > 0; i--, start++ ) {
    start = strchr( start, ',' );
    if ( start == NULL ) return NULL;
  }

  ptr = buffer;
  while ( *start != ',' && *start != '\0' )
    *(ptr++) = *(start++);
  *ptr = '\0';

  if ( string ) free( string );

  return strdup( buffer );
}

/*
 * returns the number of comma-separated entries
 * in the string value of the specified attribute.
 * e.g. LAYERS=one,two will return 2.
 */

int
wms_list_size( request_rec *r, char *listname )
{
  char *start, *string;
  int  count;

  string = start = wms_get_string( r, listname );
  if ( start == NULL ) return 0;

  for ( count = 1; ; count++, start++ ) {
    start = strchr( start, ',' );
    if ( start == NULL ) break;
  }

  free( string );
  return count;
}

/*
 * Parse out the EPSG code from the SRS attribute.
 * Returns 0 if the code could not be parsed.
 */

int 
wms_parse_srs( request_rec *r )
{
  char *srs = wms_get_string( r, "srs" );
  int  i = 0;
  wms_debug( "SRS = %s", srs?srs:"n/a" );
  i = tsmWMSParseSRS( srs );
  wms_debug( "EPSG is %d", i );
  free( srs );
  return i;
}

/*
 * Takes a TerraVision dataset definition from the database
 * and returns the EPSG code for the coordinate system.
 * Returns 0 for a coordinate system that is not supported.
 *
 * The EPSG codes are documented at http://www.epsg.org/,
 * they are also used as the codes in the GeoTIFF standard.
 */

static int
wms_get_srs( TsmDataSet *dataset )
{
  int zone, north_hemi;

  if ( dataset == NULL ) return 0;

  switch ( dataset->footprint.coordsys ) {
  case TSM_LATLONG_COORDSYS:
    /* hardwire all lat/long datasets to the EPSG code for WGS84 */
    /* other possible geodetic codes are WGS72 (4322), NAD83 (4269) */
    return 4326;

  case TSM_UTM_COORDSYS:
    /* There is a different EPSG code for each UTM zone/hemisphere */
    /* We currently hardwire all UTM datasets to just be WGS84. */
    zone = dataset->footprint.utm.c1.zone;
    north_hemi = ( dataset->footprint.utm.c1.n > 0.0 );
    return ( north_hemi ) ? 32600 + zone : 32700 + zone;

  default:
    /* unsupported coordinate systems just return 0 */
    return 0;
  }
}

/*
 * Takes a TerraVision dataset definition from the database
 * and returns the GeoVRML geoSystem string for the coordinate
 * system. See www.geovrml.org.
 *
 * Returns "" for a coordinate system that is not supported.
 *
 * The caller must free the string when finished with it.
 */

static char *
wms_get_geosystem( TsmConnection connect )
{
  char            buffer[64];
  Pyramid         *tspec;
  ProjectionTspec *proj;
  SimpleTileSet   *tset;

  if ( connect == NULL ) return strdup( "" );

  tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  proj = &tspec->projectionTspec;

  switch ( proj->projectionType ) {
  case tspLatLongProjection:
    return strdup( "\"GDC\"" );

  case tspUtmProjection:
    tset = &tspec->tspecLevel.val[tspec->minLevel];
    sprintf( buffer, "\"UTM\", \"Z%d\"", proj->utmZoneNumber );
    if ( tset->imageTransformationTspec.ixiytoUV[5] < 0 /* southern hemi */ )
      strcat( buffer, " \"S\"" );
    return strdup( buffer );

  default:
    /* unsupported coordinate systems just return "" */
    return strdup( "" );
  }
}

/*
 * Take a layer name and return the appropriate dataset definition
 */

TsmDataSet *
wms_get_dataset( char *layer )
{
  int i;

  if ( layer == NULL ) return NULL;

  for ( i = 0; i < database->numDataSets; i++ ) {
    TsmDataSet *dataset = &database->dataset[i];
    if ( wms_strcmp( layer, dataset->name ) &&
	 wms_dataset_valid( dataset ) ) 
      return dataset;
  }
  
  return NULL;
}

/*
 * Take a dataset and return the full pathname for the
 * first localdisk-resident TerraVision dataset.
 */

char *
wms_get_url( TsmDataSet *dataset )
{
  int j;

  if ( dataset == NULL ) return NULL;

  for ( j = 0; j < dataset->numInstances; j++ ) {
    TsmDataSetInst *inst = &dataset->instance[j];
    if ( inst->location->protocol == tsmLocalFile )
      return inst->location->url;
  }

  return NULL;
}

/*
 * Calculate the bounding box of this dataset. There are two types
 * of bounding boxes for TerraVision datasets: one that covers all
 * the valid pixels for the dataset (valid_only==TRUE), and one that
 * covers the tightest bounding collection of image tiles
 * (valid_only=FALSE). You can also specifiy which level of the pyramid
 * you want to calculate the bounding box for, where 0 is the first.
 */

void
wms_dataset_bbox( TsmConnection connect, double bbox[4], int level, 
		  int valid_only )
{
  Pyramid       *tspec;
  SimpleTileSet *tset;
  float         *matrix;

  tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  if ( tspec == NULL ) return;

  tset = &tspec->tspecLevel.val[tspec->minLevel + level];
  matrix = tset->imageTransformationTspec.ixiytoUV;

  bbox[0] = matrix[2];
  bbox[1] = matrix[5];
  if ( valid_only ) {
    bbox[2] = tset->numValidPixel[0] * matrix[0] + matrix[2];
    bbox[3] = tset->numValidPixel[1] * matrix[4] + matrix[5];
  } else {
    bbox[2] = tset->numPixels[0] * matrix[0] + matrix[2];
    bbox[3] = tset->numPixels[1] * matrix[4] + matrix[5];
  }
}

/* return the fractional part of a double value */

double
wms_frac( double x )
{
  int k; k = x;  /* C compilers round towards zero to get int values */
  return fabs( x - (double) k );
}

/* test whether the double value is a whole number (within some epsilon) */

int
wms_wholenumber( double x, double epsilon )
{
  double frac = wms_frac( x );
  return ( frac < epsilon || frac > 1.0-epsilon );
}

/*
 * work out whether a map request can be optimized because
 * it aligns with the TerraVision dataset tile boundary.
 */

int
wms_optimized( TsmConnection connect, int width, int height, int overlap[4],
	       double req_bbox[4], int *x, int *y, int *level )
{
  Pyramid   *tspec;
  TileTspec *tile;
  double    x1, y1, x2, y2, res, data_bbox[4], epsilon;
  int       i;

  /* if we don't have a valid tspec, then we're well and truly screwed! */

  tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  if ( tspec == NULL ) return FALSE;
  tile = &tspec->tileTspec;

  /* width and height must be a tile size */

  wms_debug( "req(%d, %d, %d, %d) <-> layer(%d, %d, %d, %d )",
	     width, height, overlap[2], overlap[3], tile->numPixelsX,
	     tile->numPixelsY, tile->overLap[0], tile->overLap[1] );

  if ( width != tile->numPixelsX ) return FALSE;
  if ( height != tile->numPixelsY ) return FALSE;
  
  /* the overlap pixels must match also */

  if ( overlap[0] != 0 ) return FALSE;
  if ( overlap[1] != 0 ) return FALSE;
  if ( overlap[2] != tile->overLap[0] ) return FALSE;
  if ( overlap[3] != tile->overLap[1] ) return FALSE;

  /* calculate the initial epsilon value as being less than a pixel error */

  wms_dataset_bbox( connect, data_bbox, 0, TRUE );
  epsilon = (data_bbox[2] - data_bbox[0]) / tspec->maxResolution / width / 4.0;

  wms_debug( "setting epsilon for bbox comparisions to %1.10lg", epsilon );

  /* check whether the bbox corresponds to a single tile in the dataset */

  res = tspec->maxResolution;

  for ( i = tspec->maxLevel;
	i >= tspec->minLevel;
	i--, res /= 2.0 ) {

    /* divide the tile number by the power-of-2 for this level */
    /* where highest res level = 1, then 2, then 4, etc.       */

    wms_dataset_bbox( connect, data_bbox, i - tspec->minLevel, FALSE );

    x1 = ( req_bbox[0] - data_bbox[0] ) / res / tile->numPixelsX;
    x2 = ( req_bbox[2] - data_bbox[0] ) / res / tile->numPixelsX;
    y1 = ( req_bbox[1] - data_bbox[1] ) / res / tile->numPixelsY;
    y2 = ( req_bbox[3] - data_bbox[1] ) / res / tile->numPixelsY;

    wms_debug( "res %1.0lf: x1 = %lg, y1 = %lg, x2 = %lg, y2 = %lg "
	       "(epsilon = %1.10lg)", res, x1, y1, x2, y2, epsilon );

    /* if the tile size is not 1, then cannot do */

    if ( x2 - x1 > 1.0001 ) { wms_debug( "x dim > 1" ); break; }
    if ( y2 - y1 > 1.0001 ) { wms_debug( "y dim > 1" ); break; }

    if ( x2 - x1 < 0.999 ) { wms_debug( "x dim < 1" ); continue; }
    if ( y2 - y1 < 0.999 ) { wms_debug( "y dim < 1" ); continue; }

    /* check whether each of the tile indicies are whole numbers */

    if ( ! wms_wholenumber( x1, epsilon ) ) { wms_debug( "x1 not int: %1.10lg", x1 ); continue; }
    if ( ! wms_wholenumber( x2, epsilon ) ) { wms_debug( "x2 not int" ); continue; }
    if ( ! wms_wholenumber( y1, epsilon ) ) { wms_debug( "y1 not int" ); continue; }
    if ( ! wms_wholenumber( y2, epsilon ) ) { wms_debug( "y2 not int" ); continue; }

    /* we have an optimized path!!! */

    *level = i;
    *x = (int) floor( x1 + 0.5 );
    *y = (int) floor( y1 + 0.5 );

    wms_debug( "Single Tile Found: %d, %d at level %d", *x, *y, *level );
    return TRUE;
  }

  return FALSE;
}

/*
 * Convert a 4-point Footprint to a min/max bounding box representation
 */ 

void
wms_footprint_to_bbox( Footprint *fp, double bbox[4] )
{
  switch ( fp->coordsys ) {
  case TSM_LATLONG_COORDSYS:
    bbox[0] = bbox[2] = fp->latlong.c1.lon;
    bbox[1] = bbox[3] = fp->latlong.c1.lat;

    if ( fp->latlong.c2.lon < bbox[0] ) bbox[0] = fp->latlong.c2.lon;
    if ( fp->latlong.c2.lon > bbox[2] ) bbox[2] = fp->latlong.c2.lon;
    if ( fp->latlong.c2.lat < bbox[1] ) bbox[1] = fp->latlong.c2.lat;
    if ( fp->latlong.c2.lat > bbox[3] ) bbox[3] = fp->latlong.c2.lat;

    if ( fp->latlong.c3.lon < bbox[0] ) bbox[0] = fp->latlong.c3.lon;
    if ( fp->latlong.c3.lon > bbox[2] ) bbox[2] = fp->latlong.c3.lon;
    if ( fp->latlong.c3.lat < bbox[1] ) bbox[1] = fp->latlong.c3.lat;
    if ( fp->latlong.c3.lat > bbox[3] ) bbox[3] = fp->latlong.c3.lat;

    if ( fp->latlong.c4.lon < bbox[0] ) bbox[0] = fp->latlong.c4.lon;
    if ( fp->latlong.c4.lon > bbox[2] ) bbox[2] = fp->latlong.c4.lon;
    if ( fp->latlong.c4.lat < bbox[1] ) bbox[1] = fp->latlong.c4.lat;
    if ( fp->latlong.c4.lat > bbox[3] ) bbox[3] = fp->latlong.c4.lat;
    break;

  case TSM_UTM_COORDSYS:
    bbox[0] = bbox[2] = fp->utm.c1.e;
    bbox[1] = bbox[3] = fp->utm.c1.n;

    if ( fp->utm.c2.e < bbox[0] ) bbox[0] = fp->utm.c2.e;
    if ( fp->utm.c2.e > bbox[2] ) bbox[2] = fp->utm.c2.e;
    if ( fp->utm.c2.n < bbox[1] ) bbox[1] = fp->utm.c2.n;
    if ( fp->utm.c2.n > bbox[3] ) bbox[3] = fp->utm.c2.n;

    if ( fp->utm.c3.e < bbox[0] ) bbox[0] = fp->utm.c3.e;
    if ( fp->utm.c3.e > bbox[2] ) bbox[2] = fp->utm.c3.e;
    if ( fp->utm.c3.n < bbox[1] ) bbox[1] = fp->utm.c3.n;
    if ( fp->utm.c3.n > bbox[3] ) bbox[3] = fp->utm.c3.n;

    if ( fp->utm.c4.e < bbox[0] ) bbox[0] = fp->utm.c4.e;
    if ( fp->utm.c4.e > bbox[2] ) bbox[2] = fp->utm.c4.e;
    if ( fp->utm.c4.n < bbox[1] ) bbox[1] = fp->utm.c4.n;
    if ( fp->utm.c4.n > bbox[3] ) bbox[3] = fp->utm.c4.n;
    break; 

  default:
    bbox[0] = bbox[1] = bbox[2] = bbox[3] = 0.0;
  }
}

/*
 * Implement the GetCapabilities request for this WMS
 * Returning WMS 1.1.0 compliant Capabilities XML
 */

int
wms_capabilities_1_1( request_rec *r )
{
  char   *dtd = "http://www.digitalearth.gov/wmt/xml/capabilities_1_1_0.dtd";
  int    i;
  double bbox[4];
  Footprint gdc;

  r->content_type = "text/xml";
  ap_send_http_header(r);

  /* spit out the XML header for the Capabilities */

  ap_rprintf(r, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  ap_rprintf(r, "<!DOCTYPE WMT_MS_Capabilities SYSTEM \"%s\"\n", dtd);
  ap_rprintf(r, "[\n<!-- Vendor-specific elements -->\n" );
  ap_rprintf(r, "<!ELEMENT VendorSpecificCapabilities EMPTY>\n" );
  ap_rprintf(r, "]>  <!-- end of DOCTYPE declaration -->\n\n" );
  ap_rprintf(r, "<WMT_MS_Capabilities version=\"1.1.0\" updateSequence=\"0\">\n" );

  /* Give the Service part of the Capabilities XML */

  ap_rprintf(r, "<!-- Service Metadata -->\n" );
  ap_rprintf(r, "\n<Service>\n" );
  ap_rprintf(r, "<Name>GetMap</Name>\n" );
  ap_rprintf(r, "<Title>SRI International Web Mapping Server</Title>\n" );
  ap_rprintf(r, "<Abstract>SRI's Web Mapping Server Interface for "
	     "TerraVision terrain models</Abstract>\n");
  ap_rprintf(r, "<KeywordList>\n" );
  ap_rprintf(r, "<Keyword>TerraVision</Keyword>\n" );
  ap_rprintf(r, "<Keyword>GeoVRML</Keyword>\n" );
  ap_rprintf(r, "</KeywordList>\n" );
  ap_rprintf(r, "<OnlineResource\n"
	        "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
	        "  xlink:type=\"simple\"\n"
	        "  xlink:href=\"http://www.tvgeo.com/\"/>\n");

  ap_rprintf(r, "<ContactInformation>\n" );
  ap_rprintf(r, "<ContactPersonPrimary>\n" );
  ap_rprintf(r, "<ContactPerson>Martin Reddy</ContactPerson>\n" );
  ap_rprintf(r, "<ContactOrganization>SRI International</ContactOrganization>\n" );
  ap_rprintf(r, "</ContactPersonPrimary>\n" );
  ap_rprintf(r, "<ContactPosition>Computer Scientist</ContactPosition>\n" );
  ap_rprintf(r, "<ContactAddress>\n" );
  ap_rprintf(r, "<AddressType>postal</AddressType>\n" );
  ap_rprintf(r, "<Address>333 Ravenswood Avenue</Address>\n" );
  ap_rprintf(r, "<City>Menlo Park</City>\n" );
  ap_rprintf(r, "<StateOrProvince>CA</StateOrProvince>\n" );
  ap_rprintf(r, "<PostCode>94025</PostCode>\n" );
  ap_rprintf(r, "<Country>USA</Country>\n" );
  ap_rprintf(r, "</ContactAddress>\n" );
  ap_rprintf(r, "<ContactElectronicMailAddress>reddy@ai.sri.com</ContactElectronicMailAddress>\n" );
  ap_rprintf(r, "</ContactInformation>\n" );

  ap_rprintf(r, "<Fees>none</Fees>\n" );
  ap_rprintf(r, "<AccessConstraints>none</AccessConstraints>\n" );
  ap_rprintf(r, "</Service>\n\n" );
  
  /* start the Capability part of the Capabilities XML */

  ap_rprintf(r, "<!-- Capability Metadata -->\n" );
  ap_rprintf(r, "<Capability>\n\n" );
  ap_rprintf(r, "<Request>\n" );
  
  /* define the format and URL for GetCapabilities requests */

  ap_rprintf(r, "<GetCapabilities>\n" );

  ap_rprintf(r, "<Format>application/vnd.ogc.wms_xml</Format>\n");
  ap_rprintf(r, "<DCPType><HTTP>\n");
  ap_rprintf(r, "<Get>\n" );
  ap_rprintf(r, "<OnlineResource\n"
	        "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
	        "  xlink:type=\"simple\"\n"
	        "  xlink:href=\"%s\"/>", wms_server_url );
  ap_rprintf(r, "</Get>\n" );
  ap_rprintf(r, "<Post>\n" );
  ap_rprintf(r, "<OnlineResource\n"
	        "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
	        "  xlink:type=\"simple\"\n"
	        "  xlink:href=\"%s\"/>", wms_server_url );
  ap_rprintf(r, "</Post>\n" );
  ap_rprintf(r, "</HTTP></DCPType>\n");
  
  ap_rprintf(r, "</GetCapabilities>\n" );

  /* define the format and URL for GetMap requests */

  ap_rprintf(r, "<GetMap>\n" );

  for ( i = 0; formats[i].format != NULL; i++ )
    ap_rprintf(r, "<Format>%s</Format>", formats[i].mime );
  ap_rprintf(r, "<DCPType><HTTP>\n");
  ap_rprintf(r, "<Get>\n" );
  ap_rprintf(r, "<OnlineResource\n"
	        "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
	        "  xlink:type=\"simple\"\n"
	        "  xlink:href=\"%s\"/>", wms_server_url );
  ap_rprintf(r, "</Get>\n" );
  ap_rprintf(r, "</HTTP></DCPType>\n");

  ap_rprintf(r, "</GetMap>\n\n" );

  ap_rprintf(r, "</Request>\n\n" );

  /* specify the exception formats */

  ap_rprintf(r, "<!-- Exception Metadata -->\n" );
  ap_rprintf(r, "<Exception>\n" );
  ap_rprintf(r, "<Format>application/vnd.ogc.se_xml</Format>\n" );
  ap_rprintf(r, "</Exception>\n\n" );

  /* specify the vendor-specific hints */

  ap_rprintf(r, "<!-- Vendor Specific Metadata -->\n" );
  ap_rprintf(r, "<VendorSpecificCapabilities/>\n\n" );

  /* spit out all of the layers. All of our layers need to be nested */
  /* inside of a single root <layer> tag for compliance with the DTD */

  ap_rprintf(r, "<!-- Layer Metadata -->\n" );

  ap_rprintf(r, "\n<Layer queryable=\"0\">\n");
  ap_rprintf(r, "<Title>SRI TerraVision Terrain Models</Title>\n");
  ap_rprintf(r, "<SRS></SRS>\n");

  for ( i = 0; i < database->numDataSets; i++ ) {
    TsmDataSet *dataset = &database->dataset[i];
    if ( wms_dataset_valid( dataset ) ) {
      ap_rprintf(r, "\n<Layer queryable=\"0\">\n");
      ap_rprintf(r, "<Name>%s</Name>\n", dataset->name );
      ap_rprintf(r, "<Title>%s [%s]</Title>\n", dataset->geoname,
		 ( dataset->type == TSM_DEM_DATASET ) ? "DEM" : "Image" );
      if ( dataset->description && strlen( dataset->description ) > 0 )
	ap_rprintf(r, "<Abstract>%s</Abstract>\n", dataset->description );

      switch ( dataset->type ) {
      case TSM_DEM_DATASET:
	ap_rprintf(r, "<KeywordList>\n" );
	ap_rprintf(r, "<Keyword>Elevation</Keyword>\n" );
	ap_rprintf(r, "</KeywordList>\n" );
	break;
      case TSM_IMAGE_DATASET:
	ap_rprintf(r, "<KeywordList>\n" );
	ap_rprintf(r, "<Keyword>Imagery</Keyword>\n" );
	ap_rprintf(r, "</KeywordList>\n" );
	break;
      }

      ap_rprintf(r, "<SRS>EPSG:%d</SRS>\n", wms_get_srs( dataset ) );
      
      tsmFootprintToLatLong( &dataset->footprint, &gdc );
      wms_footprint_to_bbox( &gdc, bbox );

      ap_rprintf(r, "<LatLonBoundingBox minx=\"%1.10lg\" miny=\"%1.10lg\" "
		 "maxx=\"%1.10lg\" maxy=\"%1.10lg\"/>\n",
		 bbox[0], bbox[1], bbox[2], bbox[3] );

      wms_footprint_to_bbox( &dataset->footprint, bbox );

      ap_rprintf(r, "<BoundingBox SRS=\"EPSG:%d\" minx=\"%1.10lg\" "
		 "miny=\"%1.10lg\" maxx=\"%1.10lg\" maxy=\"%1.10lg\" "
		 "resx=\"%1.10g\" resy=\"%1.10g\"/>\n",
		 wms_get_srs( dataset ), bbox[0], bbox[1], bbox[2],
		 bbox[3], dataset->min_res, dataset->min_res );


      /*
      ap_rprintf(r, "<TileHint>\n" );
      ap_rprintf(r, "<TileSize x=\"128\" y=\"128\"/>\n" );
      ap_rprintf(r, "<TileOverlap left=\"0\" top=\"0\" bottom=\"0\" "
		 "right=\"0\"/>\n" );
      ap_rprintf(r, "<MaxNumPixels x=\"0\" y=\"0\"/>\n" );
      ap_rprintf(r, "</TileHint>\n" );
      */

      ap_rprintf(r, "</Layer>\n");
    }
  }
  ap_rprintf(r, "</Layer>\n");

  ap_rputs("\n</Capability>\n", r);
  ap_rputs("</WMT_MS_Capabilities>", r);

  return OK;
}

/*
 * Implement the GetCapabilities request for this WMS
 * Returning WMS 1.0.0 compliant Capabilities XML
 */

int
wms_capabilities_1_0( request_rec *r )
{
  char   *dtd = "http://www.digitalearth.gov/wmt/xml/capabilities_1_0_0.dtd";
  int    i;
  double bbox[4];
  Footprint gdc;

  r->content_type = "text/xml";
  ap_send_http_header(r);

  /* spit out the XML header for the Capabilities */

  ap_rprintf(r, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
  ap_rprintf(r, "<!DOCTYPE WMT_MS_Capabilities SYSTEM \"%s\"\n", dtd);
  ap_rprintf(r, "[\n<!-- Output formats  -->\n" );
  ap_rprintf(r, "<!ENTITY %% KnownFormats \" GEOVRML | RAW | LRBT | GIF | JPEG | PNG | WebCGM | SVG | GML.1 | INIMAGE | PPM | BLANK | WMS_XML \">\n" );
  ap_rprintf(r, "<!ELEMENT GEOVRML EMPTY> <!-- GeoVRML model -->\n");
  ap_rprintf(r, "<!ELEMENT RAW     EMPTY> <!-- RAW top-left origin -->\n");
  ap_rprintf(r, "<!ELEMENT LRBT    EMPTY> <!-- RAW bottom-left origin -->\n");
  ap_rprintf(r, "<!ELEMENT VendorSpecificCapabilities EMPTY>\n" );
  ap_rprintf(r, "]>  <!-- end of DOCTYPE declaration -->\n\n" );
  ap_rprintf(r, "<WMT_MS_Capabilities version=\"1.0.0\" updateSequence=\"0\">\n" );

  /* Give the Service part of the Capabilities XML */

  ap_rprintf(r, "<!-- Service Metadata -->\n" );
  ap_rprintf(r, "\n<Service>\n" );
  ap_rprintf(r, "<Name>GetMap</Name>\n" );
  ap_rprintf(r, "<Title>SRI International Web Mapping Server</Title>\n" );
  ap_rprintf(r, "<Abstract>SRI's Web Mapping Server Interface for "
	     "TerraVision terrain models</Abstract>\n");
  ap_rprintf(r, "<Keywords>TerraVision GeoVRML</Keywords>\n" );
  ap_rprintf(r, "<OnlineResource>http://www.tvgeo.com/</OnlineResource>\n" );
  ap_rprintf(r, "<Fees>none</Fees>\n" );
  ap_rprintf(r, "</Service>\n\n" );
  
  /* start the Capability part of the Capabilities XML */

  ap_rprintf(r, "<!-- Capability Metadata -->\n" );
  ap_rprintf(r, "<Capability>\n\n" );
  ap_rprintf(r, "<Request>\n" );
  
  /* define the format and URL for GetMap requests */

  ap_rprintf(r, "<Map>\n" );

  ap_rprintf(r, "<Format>" );
  for ( i = 0; formats[i].format != NULL; i++ )
    ap_rprintf(r, "<%s/>", formats[i].format );
  ap_rprintf(r, "</Format>\n" );
  ap_rprintf(r, "<DCPType><HTTP>\n");
  ap_rprintf(r, "<Get onlineResource=\"%s\"/>\n", wms_server_url );
  ap_rprintf(r, "</HTTP></DCPType>\n");

  ap_rprintf(r, "</Map>\n\n" );

  /* define the format and URL for GetCapabilities requests */

  ap_rprintf(r, "<Capabilities>\n" );

  ap_rprintf(r, "<Format><WMS_XML/></Format>\n");
  ap_rprintf(r, "<DCPType><HTTP>\n");
  ap_rprintf(r, "<Get onlineResource=\"%s\"/>\n", wms_server_url );
  ap_rprintf(r, "<Post onlineResource=\"%s\"/>\n", wms_server_url );
  ap_rprintf(r, "</HTTP></DCPType>\n");
  
  ap_rprintf(r, "</Capabilities>\n" );

  ap_rprintf(r, "</Request>\n\n" );

  /* specify the exception formats */

  ap_rprintf(r, "<!-- Exception Metadata -->\n" );

  /* specify the vendor-specific hints */

  ap_rprintf(r, "<!-- Vendor Specific Metadata -->\n" );

  /* spit out all of the layers. All of our layers need to be nested */
  /* inside of a single root <layer> tag for compliance with the DTD */

  ap_rprintf(r, "<!-- Layer Metadata -->\n" );

  ap_rprintf(r, "\n<Layer queryable=\"0\">\n");
  ap_rprintf(r, "<Title>SRI TerraVision Terrain Models</Title>\n");
  ap_rprintf(r, "<SRS></SRS>\n");

  for ( i = 0; i < database->numDataSets; i++ ) {
    TsmDataSet *dataset = &database->dataset[i];
    if ( wms_dataset_valid( dataset ) ) {
      ap_rprintf(r, "\n<Layer queryable=\"0\">\n");
      ap_rprintf(r, "<Name>%s</Name>\n", dataset->name );
      ap_rprintf(r, "<Title>%s</Title>\n", dataset->geoname );
      if ( dataset->description && strlen( dataset->description ) > 0 )
	ap_rprintf(r, "<Abstract>%s</Abstract>\n", dataset->description );

      switch ( dataset->type ) {
      case TSM_DEM_DATASET:
	ap_rprintf(r, "<Keywords>Elevation</Keywords>\n" );
	break;
      case TSM_IMAGE_DATASET:
	ap_rprintf(r, "<Keywords>Imagery</Keywords>\n" );
	break;
      }
      
      ap_rprintf(r, "<SRS>EPSG:%d</SRS>\n", wms_get_srs( dataset ) );

      tsmFootprintToLatLong( &dataset->footprint, &gdc );
      wms_footprint_to_bbox( &gdc, bbox );

      ap_rprintf(r, "<LatLonBoundingBox minx=\"%1.10lg\" miny=\"%1.10lg\" "
		 "maxx=\"%1.10lg\" maxy=\"%1.10lg\"/>\n",
		 bbox[0], bbox[1], bbox[2], bbox[3] );

      wms_footprint_to_bbox( &dataset->footprint, bbox );
      ap_rprintf(r, "<BoundingBox SRS=\"EPSG:%d\" minx=\"%1.10lg\" "
		 "miny=\"%1.10lg\" maxx=\"%1.10lg\" maxy=\"%1.10lg\"/>\n",
		 wms_get_srs( dataset ), bbox[0], bbox[1], bbox[2], bbox[3] );

      ap_rprintf(r, "</Layer>\n");
    }
  }

  ap_rprintf(r, "</Layer>\n");

  ap_rputs("\n</Capability>\n", r);
  ap_rputs("</WMT_MS_Capabilities>", r);

  return OK;
}

static int
wms_resample_map( TsmConnection connect, int width, int height, double bbox[4],
		  uchar **tileBuffer )
{
  Pyramid       *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  SimpleTileSet *tset;
  int           level, x, y, comps, bytespercomp, tilesize;
  int           tx, ty, tix, tiy, pixel, row, prev_tx, prev_ty;
  float         *matrix, resx, resy, res;
  float         extent[2], u, v, ix, iy;
  uchar         *partBuffer, *currPixel;

  /* calculate the extent of the dataset in x and y */

  extent[0] = (float) ( bbox[2] - bbox[0] );
  extent[1] = (float) ( bbox[3] - bbox[1] );

  /* calculate the resolution of the requested map */

  resx = extent[0] / width;
  resy = extent[1] / height;
  res  = min( resx, resy );

  /* find the optimal level of our pyramid to use */

  for ( level = tspec->maxLevel; level >= tspec->minLevel; level-- ) {
    tset = &tspec->tspecLevel.val[level];
    matrix = tset->imageTransformationTspec.ixiytoUV;
    if ( matrix[0] < res ) break;  /* found optimal resolution level */
  }

  /* allocate the map buffer */

  comps        = tspec->tileTspec.numComponentsPixel;
  tilesize     = tspec->tileTspec.numPixelsX;
  bytespercomp = tspec->tileTspec.numBytesPerComponent;
  pixel        = comps * bytespercomp;
  row          = tspec->tileTspec.totalPixels[0] * pixel;

  *tileBuffer = malloc( width * height * pixel );
  if ( *tileBuffer == NULL ) return FALSE;

  /* resample the pixels */

  prev_tx = prev_ty = -1;

  for ( y = 0; y < height; y++ ) {

    /* calculate all of the per-row state */

    v   = resy * y + (float) bbox[1];    /* geographic location */
    iy  = ( v - matrix[5] ) / matrix[4]; /* pixel coord in this level */
    ty  = floor( iy / tilesize );       /* tile coord */
    tiy = iy - ( ty * tilesize );        /* pixel offset in this tile */

    currPixel = *tileBuffer + ( pixel * width * y );

    /* process this row */

    for ( x = 0; x < width; x++, currPixel += pixel ) {

      u   = resx * x + (float) bbox[0];     /* geographic location */
      ix  = ( u - matrix[2] ) / matrix[0];  /* pixel coord in this level */
      tx  = floor( ix / tilesize );        /* tile coord */
      tix = ix - ( tx * tilesize );         /* pixel offset in this tile */

      if ( ix >= tset->numValidPixel[0] || iy >= tset->numValidPixel[1] )
	goto blank_pixel;

      /* get the tile data for this tile (if different from last tile) */

      if ( prev_tx != tx || prev_ty != ty ) {
	if ( ! wms_get_tile( connect, tx, ty, level, &partBuffer ) )
	  goto blank_pixel;
	prev_tx = tx;
	prev_ty = ty;
      }

      memcpy( currPixel, partBuffer + (row * tiy) + (pixel * tix), pixel );

      continue;
      
      /* fall through here if there was an error - make a blank pixel */
      blank_pixel:
      memset( currPixel, 0, pixel );
      
    }
  }

  return TRUE;
}

/*
 * Implement the GetMap request for this WMS 
 */

static int
wms_map( request_rec *r )
{
  int           x, y, res;
  int           width, height, srs, dataset_srs;
  char          *format, *value, *layer, *url;
  uchar         *tileBuffer;
  TsmConnection connect;
  TsmDataSet    *dataset;
  int           i, supported, opt, overlap[4] = { 0, 0, 0, 0 };
  double        bbox[4], tile_bbox[4];
  const double  epsilon = 0.000001;

  /* check that we have enough parameters to complete the request */

  format = wms_get_string( r, "format" );
  if ( ! format )
    return wms_exception( r, NULL, "No FORMAT specified for the output map" );
  for ( i = supported = 0; formats[i].format != NULL; i++ )
    if ( wms_strcmp( format, formats[i].format ) ||
	 wms_strcmp( format, formats[i].mime ) )
      supported = TRUE;
  if ( ! supported )
    return wms_exception( r, NULL, "FORMAT %s is not supported by this WMS",
			  format );

  if ( ! wms_get_arg( r, "layers" ) )
    return wms_exception( r, NULL, "No LAYERS names were specified" );

  if ( ! wms_get_arg( r, "styles" ) )
    return wms_exception( r, NULL, "No STYLES settings were specified" );

  if ( ! wms_get_arg( r, "width" ) )
    return wms_exception( r, NULL, "No WIDTH value was specified" );

  if ( ! wms_get_arg( r, "height" ) )
    return wms_exception( r, NULL, "No HEIGHT value was specified" );

  if ( ! wms_get_arg( r, "srs" ) )
    return wms_exception( r, "InvalidSRS", "No SRS value was specified" );
  if ( ( srs = wms_parse_srs( r ) ) == 0 )
    return wms_exception( r, "InvalidSRS", "Unsupported SRS value. Only supported EPSG codes" );

  if ( ! wms_get_arg( r, "bbox" ) )
    return wms_exception( r, NULL, "No BBOX bounding box range was specified" );

  if ( wms_list_size( r, "bbox" ) != 4 )
    return wms_exception( r, NULL, "BBOX does not contain four comma-separated values" );

  /* now parse out the parameters that we need for this request */

  width   = wms_get_int( r, "width" );
  height  = wms_get_int( r, "height" );
  layer   = wms_get_from_list( r, "layers", 0 );

  dataset = wms_get_dataset( layer );
  if ( dataset == NULL )
    return wms_exception( r, NULL, "Unknown layer: %s", layer );

  url     = wms_get_url( dataset );
  if ( url == NULL )
    return wms_exception( r, NULL, "No locally-mounted version of the layer %s", layer );

  dataset_srs = wms_get_srs( dataset );

  if ( srs != dataset_srs )
    return wms_exception( r, NULL, "Requested SRS does not match SRS of layer (%d, %d)",
		      srs, dataset_srs );

  for ( i = 0; i < 4; i++ ) {
    if ( ( value = wms_get_from_list( r, "bbox", i ) ) ) {
      bbox[i] = atof( value );
      free( value );
    } else
      bbox[i] = 0.0;
  }

  if ( bbox[2] < bbox[0] || bbox[3] < bbox[1] )
    return wms_exception( r, NULL, "Malformed BBOX. Should be minx, miny, "
			  "maxx, maxy" );

  /* the overlap pixels are a vendor specific parameter that we */
  /* can use to optimize on for the tile requests.              */

  if ( wms_get_arg( r, "overlap" ) ) {

    if ( wms_list_size( r, "overlap" ) != 4 )
      return wms_exception( r, NULL, "OVERLAP does not have 4 comma-separated "
			    "values" );

    for ( i = 0; i < 4; i++ ) {
      if ( ( value = wms_get_from_list( r, "overlap", i ) ) ) {
	wms_debug( "overlap value %d = %s", i, value );
	overlap[i] = atoi( value );
	free( value );
      }
    }

  }

  /* try to connect to the dataset (or use a cached connection) */

  if ( ( connect = wms_get_connection( r, url ) ) == NULL ) {
    wms_exception( r, NULL, "Could not connect to layer %s", layer );
    free( layer );
    return OK;
  }

  /* support the LAYERINFO parameter */

  if ( wms_get_arg( r, "layerinfo" ) ) {
    wms_layer_info( r, connect, layer );
    free( layer );
    return OK;
  }  

  /* work out whether we can perform an optimized tile request */

  opt = wms_optimized( connect, width, height, overlap, bbox, &x, &y, &res );

  /* support the CHECKOPT attribute for debug purposes */

  if ( wms_get_arg( r, "checkopt" ) ) {
    if ( opt )
      return wms_exception( r, NULL, "This map request IS OPTIMIZED. It "
			    "resolves to tile (%d, %d) level %d of layer %s",
			    x, y, res, layer );
    else
      return wms_exception( r, NULL, "This map request IS SUBOPTIMAL." );
  }

  /* implement the map get - use the optimized method if possible */

  if ( opt ) {

    /* read the single tile from the tileset */

    wms_debug( "Get_tile %d %d %d of %s", x, y, res, url );

    tsmSetTraceLevel( 2 );

    /* request a tile from the local dataset */
      
    if ( wms_get_tile( connect, x, y, res, &tileBuffer ) == FALSE ) {
      wms_exception( r, NULL, "Could not read tile (%d, %d, %d)", x, y, res );
      free( layer );
      return OK;
    }

    tsmSetTraceLevel( 0 );

    wms_debug( "Received tile %d %d %d of %s", x, y, res, url );

    width += overlap[2];
    height += overlap[3];

  } else {

    if ( ! wms_resample_map( connect, width, height, bbox, &tileBuffer ) ) {
      wms_exception( r, NULL, "Could not resample data to requested BBOX" );
      return OK;
    }

  }

  /* now output the tile to the client in the request format */

  for ( i = 0; formats[i].format != NULL; i++ ) {
    if ( wms_strcmp( formats[i].format, format ) ||
	 wms_strcmp( formats[i].mime, format ) ) {
      if ( formats[i].func )
	(*formats[i].func)( r, connect, tileBuffer, width, height );
      else
	wms_exception( r, NULL, "Output function not implemented for %s", format );
    }
  }
  
  /* clean up and finished the GetMap request... phewwww!! */

  if ( tileBuffer ) free( tileBuffer );
  free( layer );
  return OK;
}

/*
 * dumps a file's contents out as the http result. Used to stream
 * out the various image output formats that need to create the
 * image file on local disk first.
 */

static void
wms_output_tmpfile( request_rec *r, char *filename )
{
  FILE *fp = fopen( filename, "rb" );
  if ( fp == NULL ) {
    wms_exception( r, NULL, "Could not flush temp file to http stream!" );
    return;
  }
  while ( !feof( fp ) ) {
    char c = fgetc( fp );
    if ( feof( fp ) ) break;
    ap_rputc( c, r );
  }
  fclose( fp );
}

/*
 * Flip a tile buffer vertically - TerraVision returns raw data in
 * LRBT (left->right, bottom->top), i.e. the origin is at the bottom
 * left. Most image formats such as GIF and JPEG use the top-left.
 */

static void
wms_flip_tile( uchar *tileBuffer, int width, int height, int comps )
{
  int bufferSize = width * height * comps;
  int bytesPerRow = width * comps, y;
  uchar *tempBuffer, *src, *dest;

  /* Allocate a buffer to contain a copy of the image data */

  if ( ( tempBuffer = (uchar *) malloc( bufferSize ) ) == NULL )
    return;

  memcpy( tempBuffer, tileBuffer, bufferSize );

  /* Now mirror each row from the temporary buffer to the tile buffer */
  /* We try to optimise this by using pointer arithmetic instead of   */
  /* array look-ups (i.e. tileBuffer[]), and using memcopy().         */

  src  = tempBuffer + bufferSize - bytesPerRow;
  dest = tileBuffer;
  for ( y = 0; y < height; y++ ) {
    memcpy( dest, src, bytesPerRow );
    dest += bytesPerRow;
    src  -= bytesPerRow;
  }

  /* Free the temporary image buffer and return a successful result */
  free( tempBuffer );
}

/*
 * Forces an image buffer to be RGB. Will convert a greyscale
 * image into RGB as appropriate. Will do nothing for DEM buffers
 */

static void
wms_make_rgb( TsmConnection connect, uchar **tileBufferPtr, int width,
	      int height )
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  uchar   *src = *tileBufferPtr, *dest, *newBuffer;
  int     pixels = width * height;

  if ( tspec->identityTspec.type != tspOiType ) return;
  if ( tspec->tileTspec.numComponentsPixel == 3 ) return;

  if ( ( dest = newBuffer = malloc( pixels * 3 ) ) == NULL ) return;

  wms_debug( "Resample %d x %d (%d) greyscale image to RGB", width, height,
	     pixels );

  while ( pixels-- > 0 ) {
    uchar v = *src++;
    *dest++ = v;
    *dest++ = v;
    *dest++ = v;
  }

  free( *tileBufferPtr );
  *tileBufferPtr = newBuffer;
}

/*
 * Forces a 16-bit DEM to be a 32-bit DEM 
 */

static void
wms_make_32bit( TsmConnection connect, uchar **tileBufferPtr, int width,
		int height )
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  uchar   *src = *tileBufferPtr, *dest, *newBuffer;
  int     pixels = width * height;
  
  if ( tspec->identityTspec.type != tspDemType ) return;
  if ( tspec->tileTspec.numBytesPerComponent != 2 ) return;

  if ( ( dest = newBuffer = malloc( pixels * 4 ) ) == NULL ) return;

  wms_debug( "Resample 16-bit DEM to 32-bit DEM" );

  while ( pixels-- > 0 ) {
    *dest++ = 0;
    *dest++ = 0;
    *dest++ = *src++;
    *dest++ = *src++;
  }
  
  free( *tileBufferPtr );
  *tileBufferPtr = newBuffer;
}

/*
 * Convert a DEM tile buffer into a shaded image for displaying.
 * N.B. This will replace the tileBuffer on successful completion.
 */

static void
wms_dem_to_image( TsmConnection connect, uchar **tileBufferPtr, int width,
		  int height ) 
{
  uchar   *tileBuffer = *tileBufferPtr;
  uchar   *demBuffer = malloc( width * height * 3 ), *ptr;
  uchar   *intBuffer = tileBuffer;
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  int     comps, x, y;
  float   minval, maxval, scale, offset, elev;

  if ( ! demBuffer ) return;

  comps  = tspec->tileTspec.numBytesPerComponent;
  minval = tspec->pixelTspec.minVal;
  maxval = tspec->pixelTspec.maxVal;
  scale  = tspec->pixelTspec.scale;
  scale  *= 255.0 / ( maxval - minval );
  offset = tspec->pixelTspec.offset;

  wms_debug( "Rendering shaded-relief of DEM" );

  ptr = demBuffer;
  for ( y = 0; y < height; ++y ) {
    for ( x = 0; x < width; ++x ) {
      /* this is not little/big-endian portable!! */
      elev = ( comps == 2 ) ? (int)*((short *)intBuffer) : *((int *)intBuffer);
      elev = ( elev - offset ) * scale;
      *ptr++ = (uchar) elev;
      *ptr++ = (uchar) elev;
      *ptr++ = (uchar) elev;
      intBuffer += comps;
    }
  }

  free( tileBuffer );
  *tileBufferPtr = demBuffer;
}

/*
 * Apply the DEM tile's scale and offset so that the DEM contains
 * values in integer units of meters. Assumes that DEM is 32-bit.
 */

static void
wms_dem_scale( TsmConnection connect, uchar *tileBuffer, int width,
	       int height ) 
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  int     comps = width * height;
  float   scale, offset, elev;
  int     *ptr = (int *) tileBuffer;

  if ( tspec->identityTspec.type != tspDemType ) return;

  offset = tspec->pixelTspec.offset;
  scale  = tspec->pixelTspec.scale;

  if ( offset == 0.0f && scale == 1.0f ) return;

  wms_debug( "Changing scale from %1.10g (offset %1.10g)", scale, offset );

  for ( ; comps > 0; comps-- ) {
    elev = ( ((float)(*ptr)) - offset ) * scale;
    *ptr++ = (int) elev;
  }
}

/*
 * output function for FORMAT=LRBT (origin = bottom-left)
 */

static void
wms_output_lrbt( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		int width, int height )
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  int bytesperpixel;

  /* ensure images are RGB and DEMS are 32-bit */

  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_scale( connect, tileBuffer, width, height );
    bytesperpixel = 4;
  } else {
    wms_make_rgb( connect, &tileBuffer, width, height );
    bytesperpixel = 3;
  }

  /* send the raw stream out as the HTTP result */

  r->content_type = "text/html";
  ap_send_http_header( r );
  ap_rwrite( tileBuffer, width * height * bytesperpixel, r );
}

/*
 * output function for FORMAT=RAW (origin = top-left)
 */

static void
wms_output_raw( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		int width, int height )
{
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  int bytesperpixel;

  /* ensure images are RGB and DEMS are 32-bit */

  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_scale( connect, tileBuffer, width, height );
    bytesperpixel = 4;
  } else {
    wms_make_rgb( connect, &tileBuffer, width, height );
    bytesperpixel = 3;
  }

  /* flip the origin from bottom-left to top-left */

  wms_flip_tile( tileBuffer, width, height, bytesperpixel );

  /* send the raw stream out as the HTTP result */

  r->content_type = "text/html";
  ap_send_http_header( r );
  ap_rwrite( tileBuffer, width * height * bytesperpixel, r );
}

/*
 * output function for FORMAT=PPM (origin = top-left)
 */

static void
wms_output_ppm( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		int width, int height )
{
  /* flip the origin, ensure images are RGB and DEMS are 32-bit */

  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_to_image( connect, &tileBuffer, width, height );
  } else {
    wms_make_rgb( connect, &tileBuffer, width, height );
  }
  wms_flip_tile( tileBuffer, width, height, 3 );

  /* send the raw PPM stream out as the HTTP result */

  r->content_type = "image/ppm";
  ap_send_http_header( r );
  ap_rprintf( r, "P6\n%d %d\n255\n", width, height );
  ap_rwrite( tileBuffer, width * height * 3, r );
}

/*
 * output function for FORMAT=GIF (force RGB and 32-bit DEMs)
 */

static void
wms_output_gif( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		int width, int height )
{
  char filename[1024];
  tmpnam( filename );

  /* flip the origin, ensure images are RGB and DEMS are 32-bit */

  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_to_image( connect, &tileBuffer, width, height );
  } else {
    wms_make_rgb( connect, &tileBuffer, width, height );
  }
  wms_flip_tile( tileBuffer, width, height, 3 );

  /* save to a temporary GIF image */

  if ( ! tsmSaveImageGIF( filename, tileBuffer, width, height ) ) {
    wms_exception( r, NULL, "Could not create temporary GIF image!" );
    return;
  }

  /* stream the temporary image as the HTTP result */

  r->content_type = "image/gif";
  ap_send_http_header(r);
  wms_output_tmpfile( r, filename );
  remove( filename );
}

/*
 * output function for FORMAT=JPEG (force RGB and 32-bit DEMs)
 */

static void
wms_output_jpeg( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		 int width, int height )
{
  static const int offset = JFIF_MAGIC_SIZE - 2; /* +header, -M_SOI marker */
  int    length;
  uchar  *jpegBuffer;

  /* store the JFIF header section in a static array for quick access */

  static unsigned char jfif_header[JFIF_MAGIC_SIZE] = {
    0xff, 0xD8,                  /* M_SOI marker */
    0xFF, 0xE0, 0x00, 0x10,      /* M_APP0 marker and length */
    'J', 'F', 'I', 'F', '\0',    /* JFIF identifier */
    0x01, 0x02,                  /* JFIF version = 1.02 */
    0x00,                        /* units = none */
    0x00, 0x01, 0x00, 0x01,      /* x and y densities = 1 */
    0x00, 0x00                   /* no thumbnail image */
  };

  /* ensure images are RGB and DEMS are 32-bit */

  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_to_image( connect, &tileBuffer, width, height );
  } else {
    wms_make_rgb( connect, &tileBuffer, width, height );
  }

  /* allocate a large enough buffer to compress the buffer into */

  length = width * height * 3 + offset;

  if ( ( jpegBuffer = malloc( length ) ) == NULL ) {
    wms_exception( r, NULL, "Couldn't allocate a JPEG decompress buffer" );
    return;
  }

  /* Compress the tile into the jpeg buffer (leaving room for header) */

  length = tsmJPEGCompress( tileBuffer, jpegBuffer + offset, width, height,
			    3, 1, 80 );

  if ( length == 0 ) {
    wms_exception( r, NULL, "Couldn't JPEG compress the given tile" );
    free( jpegBuffer );
    return;
  }

  /* Write the JFIF APP header to the start of the jpeg buffer */

  memcpy( jpegBuffer, jfif_header, JFIF_MAGIC_SIZE );

  /* stream the temporary image as the HTTP result */

  r->content_type = "image/jpeg";
  ap_send_http_header(r);
  ap_rwrite( jpegBuffer, length + offset, r );
  free( jpegBuffer );
}

static void
wms_geovrml_header( request_rec *r, char *geosystem, char *viewpoint,
		    char *name, char *texurl )
{
  ap_rprintf( r, "#VRML V2.0 utf8\n" );
  ap_rprintf( r, "#\n" );
  ap_rprintf( r, "# GeoVRML terrain model produced using SRI International's\n" );
  ap_rprintf( r, "# OGC-compliant Web Mapping Server (WMS). For details on\n" );
  ap_rprintf( r, "# this WMS, refer to www.tvgeo.com. For details on GeoVRML\n" );
  ap_rprintf( r, "# refer to www.geovrml.org. For details on OGC and the WMS\n" );
  ap_rprintf( r, "# specification refer to www.opengis.org.\n" );
  ap_rprintf( r, "#\n" );
  ap_rprintf( r, "# Generating parameters = %s\n", r->unparsed_uri );
  ap_rprintf( r, "# Client hostname = %s\n", r->hostname );
  ap_rprintf( r, "#\n" );
  ap_rprintf( r, "\n" );
  ap_rprintf( r, "\n" );
  ap_rprintf( r, "EXTERNPROTO GeoViewpoint [\n" );
  ap_rprintf( r, "  field        SFNode     geoOrigin\n" );
  ap_rprintf( r, "  field        MFString   geoSystem\n" );
  ap_rprintf( r, "  field        SFString   position\n" );
  ap_rprintf( r, "  field        SFRotation orientation\n" );
  ap_rprintf( r, "  field        SFString   description\n" );
  ap_rprintf( r, "  exposedField SFFloat    fieldOfView\n" );
  ap_rprintf( r, "  exposedField SFBool     jump\n" );
  ap_rprintf( r, "  exposedField MFString   navType\n" );
  ap_rprintf( r, "  eventIn      SFBool     set_bind\n" );
  ap_rprintf( r, "  eventIn      SFString   set_position\n" );
  ap_rprintf( r, "  eventOut     SFTime     bindTime\n" );
  ap_rprintf( r, "  eventOut     SFBool     isBound\n" );
  ap_rprintf( r, "] [\n" );
  ap_rprintf( r, " \"urn:web3d:geovrml:1.0/protos/GeoViewpoint.wrl\"\n" );
  ap_rprintf( r, " \"file:///C|/Program%%20Files/GeoVRML/1.0/protos/GeoViewpoint.wrl\"\n" );
  ap_rprintf( r, " \"http://www.geovrml.org/1.0/protos/GeoViewpoint.wrl\"\n" );
  ap_rprintf( r, "]\n" );
  ap_rprintf( r, "\n" );
  ap_rprintf( r, "EXTERNPROTO GeoElevationGrid [\n" );
  ap_rprintf( r, "  field        SFNode   geoOrigin\n" );
  ap_rprintf( r, "  field        MFString geoSystem\n" );
  ap_rprintf( r, "  field        SFString geoGridOrigin\n" );
  ap_rprintf( r, "  field        SFInt32  xDimension\n" );
  ap_rprintf( r, "  field        SFString xSpacing\n" );
  ap_rprintf( r, "  field        SFInt32  zDimension\n" );
  ap_rprintf( r, "  field        SFString zSpacing\n" );
  ap_rprintf( r, "  field        MFFloat  height\n" );
  ap_rprintf( r, "  exposedField SFNode   color\n" );
  ap_rprintf( r, "  exposedField SFNode   texCoord\n" );
  ap_rprintf( r, "  exposedField SFNode   normal\n" );
  ap_rprintf( r, "  field        SFBool   normalPerVertex\n" );
  ap_rprintf( r, "  field        SFBool   ccw\n" );
  ap_rprintf( r, "  field        SFBool   colorPerVertex\n" );
  ap_rprintf( r, "  field        SFFloat  creaseAngle\n" );
  ap_rprintf( r, "  field        SFBool   solid\n" );
  ap_rprintf( r, "] [\n" );
  ap_rprintf( r, " \"urn:web3d:geovrml:1.0/protos/GeoElevationGrid.wrl\"\n" );
  ap_rprintf( r, " \"file:///C|/Program%%20Files/GeoVRML/1.0/protos/GeoElevationGrid.wrl\"\n" );
  ap_rprintf( r, " \"http://www.geovrml.org/1.0/protos/GeoElevationGrid.wrl\"\n" );
  ap_rprintf( r, "]\n" );
  ap_rprintf( r, "\n" );
  ap_rprintf( r, "Group {\n" );
  ap_rprintf( r, "  children [\n" );
  ap_rprintf( r, "     Background { skyColor 1 1 1 groundColor 1 1 1 }\n" );
  ap_rprintf( r, "     GeoViewpoint {\n" );
  ap_rprintf( r, "	geoSystem [ %s ]\n", geosystem );
  ap_rprintf( r, "	position %s\n", viewpoint );
  ap_rprintf( r, "	orientation 1 0 0 -1.57\n" );
  ap_rprintf( r, "	description \"%s\"\n", name );
  ap_rprintf( r, "     }\n" );
  ap_rprintf( r, "     Shape {\n" );
  ap_rprintf( r, "	appearance Appearance {\n" );
  ap_rprintf( r, "	   material Material { diffuseColor 0.5 0.5 0.5 }\n" );
  if ( texurl != NULL && strlen( texurl ) > 0 )
    ap_rprintf( r, "	   texture ImageTexture { url \"%s\" }\n", texurl );
  ap_rprintf( r, "	}\n" );
  ap_rprintf( r, "	geometry\n" );
}

static void
wms_geovrml_footer( request_rec *r )
{
  ap_rprintf( r, "     }\n" );
  ap_rprintf( r, "     \n" );
  ap_rprintf( r, "    ]\n" );
  ap_rprintf( r, "}\n" );
}

static void
wms_col_init( int *col )
{
  *col = 0;
}

static void
wms_col_print( request_rec *r, int *col, char *string )
{
  if ( *col == 0 ) {
    ap_rprintf( r, "%s", string );
    *col = strlen( string );

  } else if ( *col + strlen( string ) < 78 ) {
    *col += strlen( string ) + 1;
    ap_rprintf( r, " %s", string );

  } else {
    *col = strlen( string );
    ap_rprintf( r, "\n%s", string );
  }
}

/*
 * output function for FORMAT=JPEG (force RGB and 32-bit DEMs)
 */

static void
wms_output_geovrml( request_rec *r, TsmConnection connect, uchar *tileBuffer,
		    int width, int height )
{
  double bbox[4], extent[2], geo_size, img_size, alt;
  char   *geosystem = wms_get_geosystem( connect );
  char   *name = (char *) tsmGet( connect, TSM_DATASET_NAME );
  char   viewpoint[128], buffer[64], texurl[1024], *ptr;
  int    x, y, col;

  /* get the bounding box of this map */

  for ( x = 0; x < 4; x++ ) {
    if ( ( ptr = wms_get_from_list( r, "bbox", x ) ) ) {
      bbox[x] = atof( ptr );
      free( ptr );
    } else
      bbox[x] = 0.0;
  }

  extent[0] = bbox[2] - bbox[0];
  extent[1] = bbox[3] - bbox[1];

  /* work out a decent viewpoint over this map */
  /* N.B. ordering in GeoVRML is (lat, long) and (northing, easting) */

  geo_size = max( extent[0], extent[1] );
  img_size = max( width, height );
  alt = ( geo_size * 0.5 * PROJ_F * PROJ_SCR ) / img_size;

  if ( strstr( geosystem, "GDC" ) != NULL ) alt *= 1000.0;

  sprintf( viewpoint, "\"%1.10lg %1.10lg %1.10lg\"", 
	   bbox[1] + extent[1] / 2.0, bbox[0] + extent[0] / 2.0, alt );

  /* are we overlaying an image over a DEM? */

  strcpy( texurl, "" );
  if ( tsmGet( connect, TSM_DATASET_TYPE ) == TSM_DEM_DATASET ) {
    char *oilayer = wms_get_from_list( r, "layers", 1 );
    if ( oilayer ) {
      char *srs    = tsmWMSExtractParam( r->args, "srs" );
      char *width  = tsmWMSExtractParam( r->args, "width" );
      char *height = tsmWMSExtractParam( r->args, "height" );
      char *bbox   = tsmWMSExtractParam( r->args, "bbox" );
      sprintf( texurl, "http://%s/%s?WMTVER=1.0&REQUEST=map&LAYERS=%s&"
	       "SRS=%s&STYLES=&FORMAT=JPEG&WIDTH=%s&HEIGHT=%s&BBOX=%s",
	       r->server->server_hostname, r->uri,
	       oilayer, srs, width, height, bbox );

      if ( srs ) free( srs );
      if ( width ) free( width );
      if ( height ) free( height );
      if ( bbox ) free( bbox );
      free( oilayer );
    }
  }

  /* output the GeoVRML header */

  r->content_type = "model/vrml";
  ap_send_http_header(r);

  wms_geovrml_header( r, geosystem, viewpoint, name, texurl );

  /* output the GeoElevationGrid node for this map */

  ap_rprintf( r, "      # BBOX = %1.10lg, %1.10lg, %1.10lg, %1.10lg\n",
	      bbox[0], bbox[1], bbox[2], bbox[3] );

  ap_rprintf( r, "      GeoElevationGrid {\n" );
  ap_rprintf( r, "	   geoSystem [ %s ]\n", geosystem );
  ap_rprintf( r, "	   geoGridOrigin \"%1.10lg %1.10lg 0\"\n",
	      bbox[1], bbox[0] );
  ap_rprintf( r, "	   xDimension %d\n", width );
  ap_rprintf( r, "	   zDimension %d\n", height );
  ap_rprintf( r, "	   xSpacing \"%1.10lg\"\n", extent[0] / ( width-1 ) );
  ap_rprintf( r, "	   zSpacing \"%1.10lg\"\n", extent[1] / ( height-1 ) );
  ap_rprintf( r, "	   solid FALSE\n" );
  ap_rprintf( r, "	   height [\n" );

  /* output the actual GeoElevationGrid coordinate values */

  switch ( tsmGet( connect, TSM_DATASET_TYPE ) ) {

  case TSM_DEM_DATASET:
    wms_make_32bit( connect, &tileBuffer, width, height );
    wms_dem_scale( connect, tileBuffer, width, height );

    wms_col_init( &col );
    for ( y = 0; y < height; y++ ) {
      int *intBuffer = ((int *) tileBuffer) + ( y * width );
      for ( x = 0; x < width; x++ ) {
	sprintf( buffer, "%d", *intBuffer++ );
	wms_col_print( r, &col, buffer );
      }
    }
    ap_rprintf( r, " ]\n" ); 
    break;

  case TSM_IMAGE_DATASET:
    wms_make_rgb( connect, &tileBuffer, width, height );

    wms_col_init( &col );
    sprintf( buffer, "0" );
    for ( y = 0; y < height; y++ )
      for ( x = 0; x < width; x++ )
	wms_col_print( r, &col, buffer );

    ap_rprintf( r, " ]\n" ); 
    ap_rprintf( r, "           colorPerVertex TRUE\n" );
    ap_rprintf( r, "           color Color { color [\n" );

    wms_col_init( &col );
    for ( y = 0; y < height; y++ ) {
      uchar *ptr = tileBuffer + y * ( width * 3 );
      for ( x = 0; x < width; x++ ) {
	float R = *ptr++;
	float G = *ptr++;
	float B = *ptr++;
	sprintf( buffer, "%1.2f %1.2f %1.2f,", R/255.0f, G/255.0f, B/255.0f );
	wms_col_print( r, &col, buffer );
      }
    }

    ap_rprintf( r, " ] }\n" ); 
    break;
  }

  ap_rprintf( r, "	}\n" );

  /* output the end bit of the GeoVRML file */

  wms_geovrml_footer( r );

  /* clean up */

  if ( geosystem ) free( geosystem );
}

/* here's the content handler that is called by Apache */

static int
wms_handler(request_rec *r) {
  char   *wmtver, *request;
  double version;

  /* unescape the URLs that are given to us */

  wms_unescape_url( r->args );

  /* just output an empty page if we are checking for this mod */

  if ( wms_get_arg( r, "checkmod" ) ) {
    r->content_type = "text/html";
    ap_send_http_header(r);
    ap_rputs("<HTML><BODY>mod_wms installed</BODY></HTML>\n", r);
    return OK;
  }

  /* just output some debug information if &debug=& param found */

  if ( wms_get_arg( r, "wmsinfo" ) ) {
    wms_debug_info( r );
    return OK;
  }

  /* turn on debugging? */

  if ( wms_get_arg( r, "debug" ) ) {
    debug = TRUE;
    ts_set_debug( TRUE );  /* turn on TerraServer module debugging too */
  }

  /* check that we have a valid WMS database */

  if ( database == NULL )
    return wms_exception( r, NULL, "WMS has no valid config/wms.config "
			  "database." );

  /* check that we have enough parameters for a generic WMS request */

  if ( ! wms_get_arg( r, "request" ) )
    return wms_exception( r, NULL, "No REQUEST type specified" );

  wmtver = wms_get_string( r, "version" );
  if ( ! wmtver ) {
    wmtver = wms_get_string( r, "wmtver" );
    if ( ! wmtver )
      return wms_exception( r, NULL, "No VERSION or WMTVER version number "
			    "specified" );
  }

  version = tsmWMSVersion( wmtver );
  
  if ( version > tsmWMSVersion( wms_max_version ) ) {
    wms_exception( r, NULL, "This WMS supports WMTVER/VERSION requests up to "
		   "%s. A value of %s was specified", wms_max_version, wmtver);
    free( wmtver );
    return OK;
  }
  free( wmtver );

  /* support the REQUEST types "map" and "capabilities" */

  request = wms_get_string( r, "request" );

  if ( wms_strcmp( request, "capabilities" ) ||
       wms_strcmp( request, "GetCapabilities" ) ) {
    free( request );

    /* work out which version of the Capabilities XML to return */

    wms_debug( "this version is %1.10lg versus %1.10lg", version,
	     tsmWMSVersion( "1.0.8" ) );

    if ( version >= tsmWMSVersion( "1.0.8" ) )
      return wms_capabilities_1_1( r );
    else
      return wms_capabilities_1_0( r );
  }

  if ( wms_strcmp( request, "map" ) ||
       wms_strcmp( request, "GetMap" ) ) {
    free( request );
    return wms_map( r );
  }

  wms_exception( r, NULL, "Unsupported REQUEST type: %s", request );
  free( request );
  return OK;
}

/* Make the name of the content handler known to Apache */
static handler_rec wms_handlers[] =
{
  { "wms-handler", wms_handler },
  { NULL }
};

/* Tell Apache what phases of the transaction we handle */
module MODULE_VAR_EXPORT wms_module =
{
  STANDARD_MODULE_STUFF,
  NULL,                     /* module initializer                 */
  NULL,                     /* per-directory config creator       */
  NULL,                     /* dir config merger                  */
  NULL,                     /* server config creator              */
  NULL,                     /* server config merger               */
  NULL,                     /* command table                      */
  wms_handlers,             /* [7]  content handlers              */
  NULL,                     /* [2]  URI-to-filename translation   */
  NULL,                     /* [5]  check/validate user_id        */
  NULL,                     /* [6]  check user_id is valid *here* */
  NULL,                     /* [4]  check access by host address  */
  NULL,                     /* [7]  MIME type checker/setter      */
  NULL,                     /* [8]  fixups                        */
  NULL,                     /* [9]  logger                        */
  NULL,                     /* [3]  header parser                 */
  wms_init,               /* process initialization             */
  wms_shutdown,           /* process exit/cleanup               */
  NULL                      /* [1]  post read_request handling    */
};

