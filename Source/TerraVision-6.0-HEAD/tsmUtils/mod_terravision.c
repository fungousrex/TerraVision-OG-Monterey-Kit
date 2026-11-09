/*
 * Name: mod_terravision.c
 *
 * Description:
 *     This is an Apache mod that should be compiled up to a shared
 *     object (.so file) that can be copied into the libexec directory
 *     of the Apache server. It manages the serving of tile data from
 *     TerraVision datasets over an http connection.
 *
 *     This program uses the Apache include files.
 *
 *     Current additions to the Apache conf/httpd.conf file are:
 *         
 *        LoadModule terravision_module libexec/mod_terravision.so
 *        AddModule mod_terravision.c
 *        <Location ~ "/TV/.*\.(oi|dem)">
 *          SetHandler terravision-handler
 *        </Location> 
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Compilation Notes:
 *
 *  Under RedHat 6.2 [Greg Cockroft]:
 *    1. Make sure you have the apache development rpm. Needed for headers
 *    2. Added this to the compile command
 *            -DEAPI -DLINUX -I/usr/include/apache
 *    
 *       -DEAPI  This was to keep Apache from complaining.
 *      -DLINUX  otherwise the compile fails in the apache headers
 *      -I/usr/include/apache to find httpd.h etc...
 *    
 *  Under Solaris 2.8 [Martin Reddy]:
 *    1. compile with gcc CFLAGS of -fPIC -Dsparc
 *    2. link against -ldl -lnsl -lpthread -lsocket -lposix4 -lm
 *    3. use gcc -G instead of ld -shared
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 6 April 2000.
 *
 * Revision Information:
 *
 *     $Id: mod_terravision.c,v 1.5.2.5 2001/09/25 00:48:50 reddy Exp $
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
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tsm/tsm.h>
#include <tsmUtils/terraserver.h>

#include "httpd.h"
#include "http_config.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_main.h"
#include "util_script.h"
#include "util_md5.h"

#define MAX_CONNECTS 16

module MODULE_VAR_EXPORT terravision_module;

typedef struct {
  TsmConnection connect;
  char const    *dataset;
} tvmod_state_elem;

static tvmod_state_elem tvmod_state[MAX_CONNECTS];
static int              tvmod_curr_state;

static void
tvmod_log_debug( const char *msg, ... )
{
#undef DEBUG
#ifdef DEBUG
  va_list ap;

  fprintf( stderr, "mod_terravision: " );
  va_start( ap, msg );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
#endif
}

static void
tvmod_log_error( request_rec *r, const char *msg, ... )
{
  va_list ap;

  fprintf( stderr, "mod_terravision: " );
  va_start( ap, msg );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );

  r->content_type = "text/html";
  ap_send_http_header(r);
  
  ap_rprintf(r, "<HTML><BODY>Error: %s</BODY></HTML>", msg);
}

static void
tvmod_free_elem( tvmod_state_elem *elem )
{
  if ( elem == NULL ) return;
  if ( elem->connect ) tsmDisconnect( elem->connect );
  if ( elem->dataset ) tsmMemFree( elem->dataset );
  elem->connect = NULL;
  elem->dataset = NULL;
}

static void
tvmod_init( server_rec *s, pool *p )
{
  int i;

  /* initialise the array of connection structures to be empty */

  tvmod_log_debug( "initialising connection array for pid %d", getpid() );

  tvmod_curr_state = 0;

  for ( i = 0; i < MAX_CONNECTS; i++ ) {
    tvmod_state[i].connect = NULL;
    tvmod_state[i].dataset = NULL;
  }

  /* initialise the TerraServer module */

  ts_init();
}

static void
tvmod_shutdown( server_rec *s, pool *p )
{
  int i;

  /* disconnect from all of the datasets that we connected to */

  tvmod_log_debug( "cleaning up connection array for pid %d", getpid() );

  for ( i = 0; i < MAX_CONNECTS; i++ )
    tvmod_free_elem( &tvmod_state[i] );

  /* shutdown the TerraServer module */

  ts_cleanup();
}

static int
tvmod_get_arg( request_rec *r, char const *opt )
{
  char buffer[128] = "", *inptr, *outptr = buffer;
  
  if ( ( inptr = strstr( r->args, opt ) ) == NULL )
    return 0;
  
  inptr += strlen(opt);
  while ( *inptr != '&' && *inptr != '\0' )
    *outptr++ = *inptr++;
  *outptr++ = '\0';
  
  if ( strlen(buffer) == 0 )
    return 1;

  return atoi( buffer );
}

static char *
tvmod_get_dataset( request_rec *r )
{
  char buffer[512], *ptr;
  
  strcpy( buffer, r->filename );
  ptr = buffer + strlen(buffer) - 3;
  if ( ( strcmp( ptr, "/TV" ) == 0 ) || ( strcmp( ptr, "/tv" ) == 0 ) )
    *ptr = '\0';
  
  strcat( buffer, r->path_info );

  return strdup( buffer );
}

static TsmConnection
tvmod_get_connection( request_rec *r, char const *dataset )
{
  int           i;
  TsmConnection connect;
  TsmConnectionParams params;

  /* first of all check to see if we have a cached connection */

  for ( i = 0; i < MAX_CONNECTS; i++ ) {
    if ( tvmod_state[i].dataset ) {
      if ( strcmp( tvmod_state[i].dataset, dataset ) == 0 ) {
	tvmod_log_debug( "Returning cached connection for pid %d", getpid() );
	return tvmod_state[i].connect;
      }
    }
  }

  /* otherwise create a new connection */

  /* No read multiplicity needed. */ 
  tsmInitParams (&params);
  tsmSetParam (params, TSM_PARAM_READ_MULTI, 1);

  if ( ( connect = tsmConnect( dataset, "r", params ) ) == NULL )
    return NULL;

  tvmod_log_debug( "Created a new connection for pid %d", getpid() );

  /* find a new cache slot for it - is the slot empty? If so, clean it */

  if ( tvmod_state[tvmod_curr_state].connect )
    tvmod_free_elem( &tvmod_state[tvmod_curr_state] );

  /* insert the new data for this connection into the cache */

  tvmod_state[tvmod_curr_state].connect = connect;
  tvmod_state[tvmod_curr_state].dataset = tsmMemStrdup( dataset );

  /* move the cache pointer onto the next element, round robin */

  tvmod_curr_state = ( tvmod_curr_state + 1 ) % MAX_CONNECTS;

  return connect;
}

static void
tvmod_debug_info(request_rec *r) 
{
  const char* hostname;
  
  r->content_type = "text/html";
  ap_send_http_header(r);
  hostname = ap_get_remote_host(r->connection, 
				r->per_dir_config, REMOTE_NAME);

  ap_rputs("<HTML>\n", r);
  ap_rputs("<HEADER>\n", r);
  ap_rputs("<TITLE>TerraVision Apache mod debug output</TITLE>\n", r);
  ap_rputs("</HEADER>\n", r);
  ap_rputs("<BODY>\n", r);
  ap_rprintf(r, "<H1>TerraVision Apache Mode Info</H1>\n");
  ap_rprintf(r, "<p><PRE>unparsed_uri = %s\n", r->unparsed_uri);
  ap_rprintf(r, "uri          = %s\n", r->uri);
  ap_rprintf(r, "filename     = %s\n", r->filename);
  ap_rprintf(r, "path_info    = %s\n", r->path_info);
  ap_rprintf(r, "args         = %s\n\n", r->args);
  ap_rprintf(r, "hostname     = %s\n\n", hostname);
  
  ap_rprintf(r, "dataset      = %s\n", tvmod_get_dataset( r ) );
  ap_rprintf(r, "x            = %d\n", tvmod_get_arg( r, "x=" ) );
  ap_rprintf(r, "y            = %d\n", tvmod_get_arg( r, "y=" ) );
  ap_rprintf(r, "level        = %d\n", tvmod_get_arg( r, "level=" ) );
  ap_rprintf(r, "debug        = %d\n", tvmod_get_arg( r, "debug=" ) );
  ap_rprintf(r, "terraserver? = %d\n", ts_is_terraserver( r->uri ) );
  
  ap_rputs("</PRE></BODY>\n", r);
  ap_rputs("</HTML>\n", r);
}

/* here's the content handler */

static int
terravision_handler(request_rec *r) {
  int              x, y, level, terraserver;
  char             *dataset;
  uchar            *tileBuffer;
  TsmConnection    connect;
  
  /* just output an empty page if we are checking for this mod */

  if ( tvmod_get_arg( r, "check=" ) ) {
    r->content_type = "text/html";
    ap_send_http_header(r);
    ap_rputs("<HTML><BODY>mod_terravision installed</BODY></HTML>\n", r);
    return OK;
  }

  /* just output some debug information if &debug=& param found */
  
  if ( tvmod_get_arg( r, "debug=" ) ) {
    tvmod_debug_info( r );
    return OK;
  }

  /* parse out all of the dataset parameters that we need */

  x     = tvmod_get_arg( r, "x=" );
  y     = tvmod_get_arg( r, "y=" );
  level = tvmod_get_arg( r, "level=" );

  dataset = tvmod_get_dataset( r );

  terraserver = ts_is_terraserver( dataset );

  /* try to connect to the dataset (or used a cached connection) */

  if ( ( connect = tvmod_get_connection( r, dataset ) ) == NULL ) {
    tvmod_log_error( r, "could not connect to dataset" );
    free( dataset );
    return OK;
  }

  free( dataset );

  /* now allocate a tile buffer and read the requested tile */

  if ( terraserver ) {

    /* request a TerraServer tile in TerraVision's tile size format */

    if ( ! ts_get_terraserver_tile( connect, x, y, level, &tileBuffer ) ) {
      tvmod_log_error( r, "could not read the specified TerraServer tile" );
      return OK;
    }

  } else {

    /* request a plain old TerraVision tile from the local dataset */

    if ( ! tsmAllocReadTile( connect, x, y, level, &tileBuffer ) ) {
      tvmod_log_error( r, "could not read the specified TerraVision tile" );
      return OK;
    } 

  }

  /* now output the tile to the client */

  r->content_type = "text/html";
  ap_send_http_header(r);

  ap_rwrite(tileBuffer, tsmGet( connect, TSM_TILE_SIZE ), r );

  tsmFreeTileBuffer( tileBuffer );

  return OK;
}
 
/* Make the name of the content handler known to Apache */
static handler_rec terravision_handlers[] =
{
  { "terravision-handler", terravision_handler },
  { NULL }
};

/* Tell Apache what phases of the transaction we handle */
module MODULE_VAR_EXPORT terravision_module =
{
  STANDARD_MODULE_STUFF,
  NULL,                     /* module initializer                 */
  NULL,                     /* per-directory config creator       */
  NULL,                     /* dir config merger                  */
  NULL,                     /* server config creator              */
  NULL,                     /* server config merger               */
  NULL,                     /* command table                      */
  terravision_handlers,     /* [7]  content handlers              */
  NULL,                     /* [2]  URI-to-filename translation   */
  NULL,                     /* [5]  check/validate user_id        */
  NULL,                     /* [6]  check user_id is valid *here* */
  NULL,                     /* [4]  check access by host address  */
  NULL,                     /* [7]  MIME type checker/setter      */
  NULL,                     /* [8]  fixups                        */
  NULL,                     /* [9]  logger                        */
  NULL,                     /* [3]  header parser                 */
  tvmod_init,               /* process initialization             */
  tvmod_shutdown,           /* process exit/cleanup               */
  NULL                      /* [1]  post read_request handling    */
};

