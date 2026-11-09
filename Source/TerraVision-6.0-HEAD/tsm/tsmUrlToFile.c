/*
 * Name: tsmUrlToFile.c
 *
 * Description: 
 *   Routines for accessing files on remote servers using the http
 *   protocol. We use the GNOME HTTP (ghttp) library for our http
 *   support.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmUrlToFile.c,v 1.42.4.6 2001/12/14 01:36:52 reddy Exp $
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

#include <prinit.h>
#include <pthr/list.h>

#include <ghttp/http_uri.h>
#include <ghttp/http_hdrs.h>
#include <ghttp/http_trans.h>
#include <ghttp/http_req.h>
#include <ghttp/http_resp.h>
#include <ghttp/http_date.h>
#include <ghttp/http_global.h>

/* remember the current proxy server definition */

static http_uri *proxy = NULL;

/* local state to remember all temp files that we opened with tsmOpenStream */

typedef struct {
  char *filename;
  FILE *handle;
} HandleAndName;

static list_t *handle_list = NULL;

/*
 * Synopsis:
 *   http_trans_conn *tsmOpenHttpConnection( http_trans_conn *conn,
 *                                           char const *url)
 *
 * Doc++ Description: *//** 
 *   Get the http connection or create one if not already available
 *
 *   This function will return a valid connection structure in order
 *   to do http requests. If you give it a TsmConnection structure then
 *   it will try to cache this http connection structure in there so 
 *   that the next time round it will not need to open a new socket
 *   again (the connection will be closed on a tsmDisconnect). If you
 *   pass NULL for the http_trans_conn then it will just create the 
 *   new http connection and return that.
 *
 *   In the case of a cached http connection, the function will check
 *   that the socket is still open and will reconnect if the connection
 *   has lost or reset by peer.
 *
 * @param    connect the http_trans_conn structure, or NULL if no caching
 * @param    url the URL for the remote server to be connected to
 * @return   connect the http connection structure, or NULL if failed
 *
 * @author   Martin Reddy
 * @version  15 March 2000
 *
 * @see      tsmCloseHttpConnection
 *
 */

http_trans_conn *
tsmOpenHttpConnection( http_trans_conn *conn, char const *url )
{
  const char      *funcName = "tsmOpenHttpConnection";
  http_uri        *uri;

  /* check to see if we lost our connection and need to reconnect */

  if ( conn ) {
    if ( ! http_trans_conn_connected( conn ) ) {
      TSMTRACE(( TSMMASK|2, funcName, "Reconnecting to %s", conn->host ));
      tsmCloseHttpConnection( conn );
    }
  }

  /* do we need to create a new connection? */

  if ( conn == NULL ) {

    /* parse the URL */
    
    if ( ( uri = http_uri_new() ) == NULL )
      return NULL;

    if ( http_uri_parse( (char *) url, uri ) < 0 ) {
      http_uri_destroy( uri );
      return NULL;
    }

    /* do the connect to the remote host */

    if ( ( conn = http_trans_conn_new() ) == NULL ) {
      http_uri_destroy( uri );
      return NULL;
    }
    conn->host = tsmMemStrdup( uri->host );
    conn->port = uri->port;
    conn->sync = HTTP_TRANS_ASYNC;
    conn->io_buf_chunksize = 16384;

    if ( proxy ) {
      conn->proxy_host = tsmMemStrdup( proxy->host );
      conn->proxy_port = proxy->port;
    }

    http_uri_destroy( uri );

    if ( http_trans_connect( conn ) < 0 ) {
      char const *err = "unknown error";

      if ( conn->error_type == http_trans_err_type_errno )
	err = strerror( conn->error );
      else if ( conn->error_type == http_trans_err_type_host )
	err = http_trans_get_host_error( h_errno );

      TSMTRACE(( TSMMASK|1, funcName, "Failed to connect to %s: %s",
		 conn->host, err ));

      tsmCloseHttpConnection (conn);

      return NULL;
    }
    
    TSMTRACE(( TSMMASK|1, funcName, "Connected to %p host %s",
	       conn, conn->host ));

  } else {

    /* return the cached http connection from a previous call */

    TSMTRACE(( TSMMASK|2, funcName, "Connected to %p host %s (cached)",
	       conn, conn->host));

    /* Make sure there's nothing left over from previous requests */
    http_trans_buf_reset (conn);
  }

  return conn;
}

/*
 * Synopsis:
 *   void tsmCloseHTTPConnection( http_trans_conn *conn )
 *
 * Doc++ Description: *//** 
 *   Closes an http connection if currently opened.
 *
 *   This function takes a http_trans_conn structure and an http
 *   connection. It will close the connection to the http server.
 *
 * @param    connect the http_trans_conn structure (may be NULL)
 * @param    conn the http connection structure (may be NULL)
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  15 March 2000
 *
 * @see      tsmOpenHttpConnection
 *
 */

void
tsmCloseHttpConnection( http_trans_conn *conn )
{
  const char *funcName = "tsmCloseHttpConnection";

  if ( conn ) {
    char *host = conn->host;
    char *proxy_host = conn->proxy_host;

    TSMTRACE(( TSMMASK|1, funcName, "Disconnecting from %p host %s", 
	       conn, host ));
    http_trans_conn_destroy( conn );

    if ( host ) tsmMemFree( host );
    if ( proxy_host ) tsmMemFree( proxy_host );
  }
}

/*
 * Synopsis:
 *   void tsmInitHttp( char const *proxy )
 *
 * Doc++ Description: *//** 
 *   Initialise the http support routines.
 *
 *   This function will initialise the internal state and set up
 *   the library to perform preemptive (blocking) http reads.
 *   
 * @return   Nothing
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmCleanupHttp
 *
 */

void
tsmInitHttp( void )
{
}

/*
 * Synopsis:
 *   void tsmCleanupHttp( void )
 *
 * Doc++ Description: *//** 
 *   Closes down and cleans up the http routines.
 *
 *   This function should be called just before exiting the application.
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmInitHttp
 *
 */

void
tsmCleanupHttp( void )
{
  tsmSetHttpProxy( NULL );
}

/*
 * Synopsis:
 *   void tsmSetHttpProxy( char const *proxy )
 *
 * Doc++ Description: *//** 
 *   Registers a proxy server for all http accesses.
 *
 *   This function will register a proxy server to be used for
 *   all subsequent http accesses. This is useful if the client
 *   is behind a firewall and can only access the outside world
 *   via a proxy server.
 *
 *   You may call this function multiple times in order to update a
 *   proxy server definition. Passing a NULL string will un-register
 *   the proxy so that afterwards all requests for http objects will
 *   go directly to the specified host.
 *
 * @param    proxy the url for the proxy, e.g. "http://yuba.ai.sri.com:8000/"
 *           or NULL to unregister a previous proxy.
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      
 *
 */

void
tsmSetHttpProxy( char const *newproxy )
{
  if ( proxy )
    http_uri_destroy( proxy );

  if ( newproxy ) {
    if ( ( proxy = http_uri_new() ) != NULL ) {
      if ( http_uri_parse( (char *) newproxy, proxy ) < 0 ) {
	http_uri_destroy( proxy );
	proxy = NULL;
      }
    }
  } else
    proxy = NULL;
}

/*
 * Synopsis:
 *   int tsmReadHttpFile( TsmConnection com_connect, char const *url,
 *                        http_req_type reqtype, int bytes,
 *                        http_resp **response )
 *
 * Doc++ Description: *//** 
 *   Download a file on an http server to memory.
 *
 *   This function takes an http url, e.g. "http://www.ai.sri.com/" and
 *   returns the result of an http accesss request on that url. The request
 *   type is used to specify whether you want to do a GET, HEAD, etc.
 *   operation (enums defined in <ghttp/http_req.h>).
 *
 *   A byte value can be optionally specified. If this value is < 0, then
 *   the entire file will be downloaded, however if you specify a positive
 *   value then only that number of bytes are retrieved from the file. This
 *   lets you read just the header part of a file without downloading the
 *   entire object.
 *
 *   The final parameter is a pointer to an http_resp pointer that will be
 *   allocated on successful completion of the function. This contains all
 *   the header information and the file's body. The client must free this
 *   structure when done by using the http_resp_destroy() function.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *   
 * @param    connect the TsmConnection structure (may be NULL)
 * @param    url the absolute url for the http object
 * @param    http_req_type the http request type, e.g. http_req_type_get 
 * @param    bytes the number of bytes to retrieve, or -1 for full file
 * @param    response used to return the details about the downloaded file
 *
 * @return   TRUE if the file was downloaded successfully, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  15 March 2000
 *
 * @see      tsmUrlToMemory, tsmUrlToFile, tsmUrlToTempFile, tsmSetHttpProxy
 *
 */

static int
tsmReadHttpFile ( http_trans_conn **connp, char const *url,
		  http_req_type reqtype, int bytes, http_resp **response )
{
  const char        *funcName = "tsmReadHttpFile";
  http_trans_conn   *conn = (connp ? *connp : 0);
  http_req          *request = NULL;
  http_uri          *uri = NULL;
  http_resp         *resp = NULL;
  int               result, success = FALSE;
  char              range[32], redirect_url[512], req_url[512];
  int               reset = 0, redirected = 0;

  strcpy( req_url, url );

redirect:

  redirect_url[0] = '\0';

  /* open an http connection  */

  if (! conn) {
    conn = tsmOpenHttpConnection( conn, req_url );
    if ( connp ) *connp = conn;
    if ( conn == NULL )
      return FALSE;
  }

  /* parse the URI for the file to be downloaded from the server */

  if ( ( uri = http_uri_new() ) == NULL ) goto cleanup;

  if ( http_uri_parse( req_url, uri ) < 0 ) goto cleanup;

  /* create a new http request */

  if ( ( request = http_req_new() ) == NULL ) goto cleanup;

  request->type     = reqtype;
  /* request->http_ver = 1.1; */
  request->host     = uri->host;
  request->full_uri = uri->full;
  request->resource = uri->resource;

  if ( bytes >= 0 ) {
    sprintf( range, "0-%d", bytes-1 );
    http_hdr_set_value( request->headers, http_hdr_Range, range );
  }

  http_req_prepare( request );

  /* send the new request */

  do {
    if ( ( result = http_req_send( request, conn ) ) == HTTP_TRANS_ERR ) {
      TSMTRACE(( TSMMASK|1, funcName, "Error sending request for %s",
		 req_url ));
      TSMTRACE(( TSMMASK|1, funcName, "ghttp: %s", conn->errstr ));
      reset = TRUE;
      goto cleanup;
    }

  } while ( result == HTTP_TRANS_NOT_DONE );

  /* read the headers for the remote file */

  *response = resp = http_resp_new();

  do {
    if ( ( result = http_resp_read_headers( resp, conn ) ) == HTTP_TRANS_ERR ){
      TSMTRACE(( TSMMASK|1, funcName, "Error reading headers for %s",
		 req_url ));
      TSMTRACE(( TSMMASK|1, funcName, "ghttp: %s", conn->errstr ));
      reset = TRUE;
      goto cleanup;
    }
  } while ( result == HTTP_TRANS_NOT_DONE );

  /* did we get a redirect?? */

  if ( resp->status_code == 301 || resp->status_code == 302 ) {
    int i;
    char **hdr = resp->headers->header;
    char **val = resp->headers->value;

    for ( i = 0; hdr[i] != NULL; i++ ) {
      if ( strcmp( hdr[i], "Location" ) == 0 ||
	   strcmp( hdr[i], "location" ) == 0 ) {
	strcpy( redirect_url, val[i] );  /* found the redirect URL */
	break;
      }
    }
  }

  /* Check to see if server has signalled Connection: close */
  if ( conn ) {
    char const *conn_value =
      http_hdr_get_value (resp->headers, "Connection");
    if ( conn_value && ! strcmp (conn_value, "close")) {
      /* Clear cached connection */
      TSMTRACE(( TSMMASK|1, funcName, "Server closed connection"));
      reset = TRUE;
    }
  }

  /* read the body of the remote file if doing a GET */

  if ( reqtype == http_req_type_get ) {
    do {
      result = http_resp_read_body( resp, request, conn );
      if ( result == HTTP_TRANS_ERR ) {
	TSMTRACE(( TSMMASK|1, funcName, "Error reading body for %s",
		   req_url ));
	TSMTRACE(( TSMMASK|1, funcName, "ghttp: %s", conn->errstr ));
	goto cleanup;
      }
    } while ( result == HTTP_TRANS_NOT_DONE );
  }

  /* everything worked if we got this far - set the return value to okay */

  success = TRUE;

cleanup:

  /* free all of our temporary resources for this request */

  if ( uri ) http_uri_destroy( uri );
  if ( request ) http_req_destroy( request );
  if ( resp && !success ) http_resp_destroy( resp );

  /* close our http connection, only if we are not caching it though */
  /* or if we need to do a redirect! Should we also close if we ever */
  /* did a redirect in this function? Probably yes. */

  if ( ! connp || reset || redirected || redirect_url[0] != '\0') {
    tsmCloseHttpConnection( conn );
    conn = NULL;
    if (connp) *connp = 0;
  }

  /* are we doing a redirect? */

  if ( redirect_url[0] != '\0' ) {
    strcpy( req_url, redirect_url );
    redirected = 1;
    goto redirect;
  }

  return success;
}

/*
 * Synopsis:
 *   int tsmUrlToMemory( char const *url, char **buffer, int *size )
 *
 * Doc++ Description: *//** 
 *   Download a file on an http server to memory.
 *
 *   This function takes an http url, e.g. "http://www.ai.sri.com/" and
 *   returns the result of a GET request on that url. The contents of
 *   the file is returned in the string "buffer" - the user should free
 *   this string when they are finished. The size of the file in bytes
 *   is returned in the "size" parameter.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 * @param    buffer a pointer to a (char *) that returns the file contents
 * @param    size a pointer to an (int) that returns the file size
 *
 * @return   TRUE if the file was downloaded successfully, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmUrlToFile, tsmUrlToTempFile, tsmSetHttpProxy
 *
 */

int
tsmUrlToMemory( http_trans_conn **connp, char const *url,
		char **buffer, int *size )
{
  http_resp *resp;

  /* a quick sanity check and initialisation */

  if ( url == NULL || buffer == NULL || size == NULL )
    return FALSE;

  *buffer = NULL;
  *size   = 0;
  
  /* read the file from the http server */

  if ( ! tsmReadHttpFile( connp, url, http_req_type_get, -1, &resp ) )
    return FALSE;

  /* check for an error */

  if ( resp->status_code > 299 ) {
    http_resp_destroy( resp );
    return FALSE;
  }

  /* download the specified absolute http URL */

  *size = resp->body_len;
  if ( *size > 0 && ( *buffer = tsmMemMalloc( *size ) ) != NULL )
    memcpy( *buffer, resp->body, *size );
  
  /* clean up the request structure if we created it just now */
  
  http_resp_destroy( resp );

  /* If chunk != NULL then we have the data */
  
  return ( *buffer != NULL ); 
}

/*
 * Synopsis:
 *   int tsmUrlToBuffer( char const *url, char *buffer, int buffer_size, int *)
 *
 * Doc++ Description: *//** 
 *   Download a file on an http server to an existing memory buffer.
 *
 *   This function takes an http url, e.g. "http://www.ai.sri.com/" and
 *   returns the result of a GET request on that url. The contents of
 *   the file are copied into the pre-allocated buffer that is provided.
 *   At most, buffer_size bytes will be read from the remote file.
 *   The number of bytes actually read is returned in the "size" parameter.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 * @param    buffer a pointer to a block of memory to be filled
 * @param    buffer_size the maximum number of bytes to read
 * @param    size a pointer to an (int) that returns the bytes read
 *
 * @return   TRUE if the file was downloaded successfully, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmUrlToFile, tsmUrlToTempFile, tsmSetHttpProxy
 *
 */

int
tsmUrlToBuffer( http_trans_conn **connp, char const *url, char *buffer,
		int buffer_size, int *size )
{
  http_resp *resp;

  /* a quick sanity check and initialisation */

  if ( url == NULL || buffer == NULL || size == NULL )
    return FALSE;

  *size = 0;
  
  /* read the file from the http server - only the first buffer_size bytes */

  if ( ! tsmReadHttpFile( connp, url, http_req_type_get, buffer_size, &resp))
    return FALSE;

  /* check for an error */

  if ( resp->status_code > 299 ) {
    if ( resp->body ) {
      free( resp->body );
      resp->body = NULL;
    }
    http_resp_destroy( resp );
    return FALSE;
  }

  /* copy the files contents into the buffer we have been given */

  *size = resp->body_len;
  if ( *size > 0 )
    memcpy( buffer, resp->body, min(buffer_size,*size) );

  /* clean up the request structure */
  
  if ( resp->body ) {
    free( resp->body );
    resp->body = NULL;
  }

  http_resp_destroy( resp );

  /* If the body length is > 0 then we have the data */

  return ( *size > 0 ); 
}

/*
 * Synopsis:
 *   int tsmUrlToFile( char const *url, char const *filename )
 *
 * Doc++ Description: *//** 
 *   Download a file on an http server to localdisk.
 *
 *   This function takes an http url, e.g. "http://www.ai.sri.com/" and
 *   returns the result of a GET request on that url. The contents of
 *   the remote file are written to file on localdisk specified by the
 *   filename parameter.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 * @param    filename the file on localdisk to create
 *
 * @return   TRUE if the file was downloaded successfully, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmUrlToMemory, tsmUrlToTempFile, tsmSetHttpProxy
 *
 */

int
tsmUrlToFile( http_trans_conn **connp,
	      char const *url, char const *filename )
{
  const char *funcName = "tsmUrlToFile";
  FILE *handle;
  char *buffer;
  int  size;

  /* download the file from the http server in to memory */

  if ( ! tsmUrlToMemory( connp, url, &buffer, &size ) )
    return FALSE;

  /* create the localdisk file for storing the remote data */

  if ( ( handle = fopen( filename, "wb" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write to file: %s\n", filename );
    tsmMemFree( buffer );
    return FALSE;
  }

  /* write the buffer to the file */

  fwrite( buffer, size, 1, handle );
  fclose( handle );

  /* free the file data from memory */

  tsmMemFree( buffer );

  return TRUE;
}

/*
 * Synopsis:
 *   char *tsmUrlToTempFile( char const *url )
 *
 * Doc++ Description: *//** 
 *   Download a file on an http server to a temporary file.
 *
 *   This function takes an http url, e.g. "http://www.ai.sri.com/" and
 *   returns the result of a GET request on that url. The contents of
 *   the remote file are written to file on localdisk. The function 
 *   creates a name for the file and returns that filename as its result.
 * 
 *   If this function returns non-NULL, then you must free the memory
 *   returned when you are finished with it.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 *
 * @return   The name of the temp file created, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  26 January 2000
 *
 * @see      tsmUrlToMemory, tsmUrlToFile, tsmSetHttpProxy
 *
 */

char *
tsmUrlToTempFile( http_trans_conn **connp, char const *url )
{
  const char *funcName = "tsmUrlToTempFile";
  char filename[L_tmpnam];

  /* work out a name for the new temp file using stdio.h */

  if ( ! tmpnam( filename ) ) {
    tsmError( funcName, "Can't create temp file name" );
    return NULL;
  }

  /* download the file from the http server to this temp file */
  
  if ( ! tsmUrlToFile( connp, url, filename ) ) {
    tsmError( funcName, "Couldn't download url to temp file!" );
    return NULL;
  }

  /* return a copy of the temp filename */

  return tsmMemStrdup( filename );
}

/*
 * Synopsis:
 *   char *tsmUrlMimeType( char const *url )
 *
 * Doc++ Description: *//** 
 *   Returns the MIME type of the file specified by a URL.
 *
 *   This function will perform a HEAD request on the specified URL.
 *   This retrieves the meta-information describing the content at
 *   that URL. We extract from this the MIME type and return this
 *   as a string. You must free this string when you are done with it.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 *
 * @return   String containing the URL's MIME type, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  27 January 2000
 *
 * @see      tsmUrlExists
 *
 */

char *
tsmUrlMimeType( http_trans_conn **connp, char const *url )
{
  http_resp *resp;
  char      *mimetype = NULL;

  /* a quick sanity check */

  if ( url == NULL )
    return NULL;

  /* do a head on the http file */

  if ( ! tsmReadHttpFile( connp, url, http_req_type_head, -1, &resp ) )
    return NULL;

  /* check for an error */

  if ( resp->status_code > 299 ) {
    http_resp_destroy( resp );
    return tsmMemStrdup( "www/unknown" );
  }

  /* get the mime type string */

  mimetype = http_hdr_get_value( resp->headers, http_hdr_Content_Type );
  mimetype = ( mimetype ) ? tsmMemStrdup( mimetype ) : NULL;

  /* clean up the request structure */
  
  http_resp_destroy( resp );
  
  return mimetype;
}

/*
 * Synopsis:
 *   int tsmUrlExists( char const *url )
 *
 * Doc++ Description: *//** 
 *   Checks to see if a valid file exists at the specified URL.
 *
 *   This function will perform a HEAD request on the specified URL
 *   and check the returned MIME type information. We assume that
 *   a MIME type of "www/unknown" means that there is no valid file.
 *
 *   A TsmConnection structure can be supplied and this will be used to
 *   cache the connection to the http server so that this does not need
 *   to be reperformed the next time round. This value can be NULL, in which
 *   case the connection is explicitly opened and closed on each call.
 *   
 * @param    connect an HTTP TsmConnection handle or NULL for no state
 * @param    url the absolute url for the http object
 *
 * @return   TRUE if there is a valid file at the URL, FALSE if not exist
 *
 * @author   Martin Reddy
 * @version  27 January 2000
 *
 * @see      tsmUrlMimeType
 *
 */

int
tsmUrlExists( http_trans_conn **connp, char const *url )
{
  char *mimetype = tsmUrlMimeType( connp, url );
  int  result    = FALSE;

  if ( mimetype ) {
    result = ( strcmp( mimetype, "www/unknown" ) != 0 );
    tsmMemFree( mimetype );
  }

  return result;
}

/* a convenience interface to tsmUrlExists without the http conn param */

int
tsmDoesUrlExist( char const *url )
{
  Purl *purl;
  int  result;

  /* Check for an invalid URL */

  if ( url == NULL || ( purl = tsmParseUrl( url ) ) == NULL ) return FALSE;

  /* otherwise, if this is a local host URL, try to open the file */
  /* or if it is an http URL, try to access the mime type         */

  if ( purl->protocol == tsmLocalFile ) {
    FILE *fp = fopen( purl->uri, "r" );
    result = ( fp != NULL );
    if ( fp ) fclose( fp );
  } else {
    http_trans_conn *conn = NULL;
    result = tsmUrlExists( &conn, url );
  }

  /* clean up and return the result */

  tsmFreePurl( purl );
  return result;
}

/*
 * Synopsis:
 *   void tsmAddHandle( FILE *handle, char *filename )
 *
 * Doc++ Description: *//** 
 *   Remembers a stream and filename pair.
 *
 *   This function takes a FILE stream and the filename corresponding
 *   to that stream. It then stores this in an internal list structure.
 *
 * @param    handle a (FILE *) stream variable
 * @param    filename the name of the file associated with the stream
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  18 January 2000
 *
 * @see      tsmFilenameFromHandle
 *
 */

static void
tsmAddHandle( FILE *handle, char *filename )
{
  HandleAndName *elem;

  if ( ( elem = tsmMemMalloc( sizeof( HandleAndName ) ) ) == NULL ) 
    return;

  if ( handle_list == NULL )
    if ( ( handle_list = list_create( TRUE ) ) == NULL )
      return;

  elem->filename = tsmMemStrdup( filename );
  elem->handle   = handle;

  list_push( handle_list, elem );
}

/*
 * Synopsis:
 *   char *tsmFilenameFromHandle( FILE *handle )
 *
 * Doc++ Description: *//** 
 *   Recalls a filename from a stream.
 *
 *   This function will search through the list of stream handles that
 *   were supplied to tsmAddHandle. If a match is found, then that
 *   entry is removed from the internal list and the filename of the
 *   file is returned.
 *
 *   If this function returns a non-NULL result, then you should release
 *   the memory for the string when finished, e.g. using tsmMemFree().
 *
 * @param    handle a (FILE *) stream variable
 *
 * @return   The filename associate with stream, or NULL.
 *
 * @author   Martin Reddy
 * @version  18 January 2000
 *
 * @see      tsmAddHandle
 *
 */

static char *
tsmFilenameFromHandle( FILE *handle )
{
  list_iter_t i;
  
  if ( handle_list == NULL ) return NULL;

  for ( i = list_first(handle_list); ! list_done(i); i = list_next(i) ) {
    HandleAndName *elem = list_value (i);
    
    if ( elem->handle == handle ) {
      char *result = elem->filename;
      list_delete( i );
      tsmMemFree( elem );
      
      return result;
    }
  }
  
  return NULL;
}

/*
 * Synopsis:
 *   FILE *tsmOpenStream( char const *url, char const *access_mode )
 *
 * Doc++ Description: *//** 
 *   Open an http or local filesystem I/O stream.
 *
 *   This function can be used to open a stream for a file, where the
 *   file can be an http URL, a file URL, or a localdisk filename. The
 *   function returns a standard FILE * stream that can use operated on
 *   by any of the stdio functions, e.g. fread, fgetc, etc. For example,
 *   valid urls including:
 * 
 *   \begin{itemize}
 *   \item http://www.ai.sri.com/~magic/tsmApi/index.html
 *   \item file://localhost/~user/foo.txt
 *   \item /homedir/user/files/foo
 *   \item ~user/tmp/foo.doc
 *   \end{itemize}
 *
 *   The access_mode parameter can be used to specified whether you
 *   want to read, write, append, etc the file. For localdisk files
 *   then this string can be any of those supported by fopen(3),
 *   e.g. "r", "w", "a", etc. For http files, the only valid 
 *   access_mode string is "r". That is, you can only read from an
 *   http server, not write to it.
 *
 *   Once you have finished using the stream, you should close it 
 *   using the tsmCloseStream() call (unless you use a function which
 *   closes the stream for you).
 *
 *   This function was basically designed to behave exactly the same
 *   as fopen for localdisk files, whilst also providing support for
 *   opening files that are located on a remote http server.
 *
 * @param    url an http or localdisk url to be opened
 * @param    access_mode one of "r", "w", "a", etc. N.B. only "r" for http.
 *
 * @return   a FILE * stream if all went well, or NULL if an error occurred
 *
 * @author   Martin Reddy
 * @version  1 May 1998
 *
 * @see      tsmCloseStream, tsmStreamToFile, tsmStreamToBuffer,
 *           tsmStreamToStdout
 *
 */

FILE *
tsmOpenStream( char const *url, char const *access_mode )
{
  static const char *funcName = "tsmOpenStream";
  Purl              *purl;
  FILE              *handle = NULL;

  /* Check that we have a value URL string */

  if ( url == NULL ) {
    tsmError( funcName, "NULL url passed" );
    return NULL;
  }

  /* Parse the url */

  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "Could not parse the URL: %s", url );
    return NULL;
  }

  /* support local filesystem and http streams */

  switch ( purl->protocol ) {
  case tsmLocalFile:
    handle = fopen( purl->uri, access_mode );
    break;

  case tsmHTTP: {
    char *tmpfilename;

    /* we can only read http urls, not write them */

    if ( strchr( access_mode, 'w' ) != NULL ) {
      TSMTRACE(( TSMMASK|1, funcName, "only support \"r\" access for http" ));
      tsmFreePurl( purl );
      return NULL;
    }

    /* download the url to a temp file on local disk */

    if ( ( tmpfilename = tsmUrlToTempFile( NULL, url ) ) == NULL ) {
      TSMTRACE(( TSMMASK|1, funcName, "Couldn't get remote url: %s", url ));
      tsmFreePurl( purl );
      return NULL;
    }

    /* open the file up for reading */

    if ( ( handle = fopen( tmpfilename, access_mode ) ) == NULL ) {
      TSMTRACE(( TSMMASK|1, funcName, "Couldn't open file: %s", tmpfilename ));
      tsmMemFree( tmpfilename );
      tsmFreePurl( purl );
      return NULL;
    }

    /* remember the filename/handle for deleting the tmp file later */

    TSMTRACE(( TSMMASK|1, funcName, 
	       "Remembering temp http file: %s", tmpfilename ));
    tsmAddHandle( handle, tmpfilename );

    tsmMemFree( tmpfilename );
    break;
  }

  default:
    tsmNoProtocolYet( purl->protocol, funcName );
    handle = NULL;
    break;
  }

  tsmFreePurl( purl );
  return handle;
}

/*
 * Synopsis:
 *   int tsmCloseStream( FILE *stream )
 *
 * Doc++ Description: *//** 
 *   Closes a stream opened with tsmOpenStream.
 *
 *   This function should be used to close a stream that was previously
 *   opened by the tsmOpenStream function. This function will not perform
 *   any close operation if the stream is NULL. Note that some of the
 *   stream utility functions close the stream automatically upon 
 *   completion so you do not need to call this function if you use one
 *   of tsmStreamToFile, tsmStreamToBuffer, or tsmStreamToStdout.
 *
 * @param    stream a valid stream as returned by tsmOpenStream
 *
 * @return   currently only returns a value of 0, for compatibility with
 *           fclose.
 *
 * @author   Martin Reddy
 * @version  1 May 1998
 *
 * @see      tsmOpenStream, tsmStreamToFile, tsmStreamToBuffer,
 *           tsmStreamToStdout
 *
 */

int
tsmCloseStream( FILE *stream )
{
  const char *funcName = "tsmCloseStream";
  char *filename;

  if ( stream != NULL ) {
    fclose( stream );
    if ( ( filename = tsmFilenameFromHandle( stream ) ) != NULL ) {
      TSMTRACE(( TSMMASK|1, funcName,
		 "Deleting temp http file: %s", filename ));
      remove( filename );
      tsmMemFree( filename );
    }
  }
  return 0;
}

/*
 * Synopsis:
 *    int tsmStreamToFile( FILE *stream, char *filename )
 *
 * Doc++ Description: *//** 
 *    Copies a stream to a file.
 *
 *    Once you have opened an http or localdisk stream using tsmOpenStream,
 *    you can use this function to copy all of the data from the stream
 *    into the specified file. The filename must be a localdisk filename,
 *    e.g. "/tmp/foo.html".
 *
 * @param    stream a valid stream as returned by tsmOpenStream
 * @param    filename the name of the file to copy the stream data to
 *
 * @return   Returns TRUE upon success, or FALSE otherwise
 *
 * @author   Martin Reddy
 * @version  1 May 1998
 *
 * @see      tsmOpenStream, tsmCloseStream, tsmStreamToBuffer,
 *           tsmStreamToStdout
 *
 */

int
tsmStreamToFile( FILE *stream, char *filename )
{
  char buf[2048];
  int numBytes;
  FILE *file_stream;

  if (!stream) {
    fprintf (stderr, "ERROR: tsmStreamToFile given a NULL input stream!\n");
    return (0);
  }

  if (!filename) {
    fprintf (stderr, "ERROR: tsmStreamToFile given a NULL filename!\n");
    return (0);
  }

  file_stream = fopen (filename, "wb");
  if (!file_stream) {
    fprintf (stderr, "ERROR: Couldn't open file '%s'\n", filename);
    return (0);
  }

  /* Read bytes from stream and write them out until numBytes is 0. */
  while ( (numBytes = fread (buf, 1, sizeof (buf), stream)) > 0) {
    fwrite (buf, 1, numBytes, file_stream);
  }
  
  fclose (file_stream);
  return (1);
}

/*
 * Synopsis:
 *    int tsmStreamToBuffer( FILE *stream, char *buffer, int bufferSize )
 *
 * Doc++ Description: *//** 
 *    Copies a stream to an area of memory.
 *
 *    Once you have opened an http or localdisk stream using
 *    tsmOpenStream, you can use this function to copy data from the
 *    stream into a pre-allocated buffer. Data is copied until there is
 *    no more room in the buffer, or there is no more data to be read
 *    (EOF).  The filename must be a localdisk filename,
 *    e.g. "/tmp/foo.html".
 *
 * @param    stream a valid stream as returned by tsmOpenStream
 * @param    buffer a pointer to the start of the memory block to copy into
 * @param    bufferSize the size in bytes of the buffer
 *
 * @return   Returns the number of bytes read into the buffer, or -1 if failed
 *
 * @author   Martin Reddy
 * @version  1 May 1998
 *
 * @see      tsmOpenStream, tsmCloseStream, tsmStreamToFile,
 *           tsmStreamToStdout
 *
 */

int
tsmStreamToBuffer( FILE *stream, char *buffer, int bufferSize )
{
  char buf[2048];
  int bufferIndex, numBytes, totalNumBytes;

  if (!stream) {
    fprintf (stderr, "ERROR: tsmStreamToBuffer given a NULL input stream!\n");
    return (0);
  }

  bufferIndex = 0;
  totalNumBytes = 0;

  /* Read bytes from stream and put them into buffer until either there's
     no more room in the buffer, or there's no more data on the stream. */
  while ( (numBytes = fread (buf, 1, sizeof (buf), stream)) > 0) {
    if (bufferIndex + numBytes < bufferSize) {
      memcpy (&buffer[bufferIndex], buf, numBytes);
      bufferIndex += numBytes;
      totalNumBytes += numBytes;
    }
    else {
      memcpy (&buffer[bufferIndex], buf, bufferSize - bufferIndex);
      totalNumBytes = bufferSize;
      break;
    }
  }

  return (totalNumBytes);
}

/*
 * Synopsis:
 *    int tsmStreamToStdout( FILE *stream )
 *
 * Doc++ Description: *//** 
 *    Copies a stream to stdout.
 *
 *    Once you have opened an http or localdisk stream using
 *    tsmOpenStream, you can use this function to direct all of the
 *    data from the stream to standard output (stdout).
 *
 * @param    stream a valid stream as returned by tsmOpenStream
 *
 * @return   Returns TRUE upon success, or FALSE otherwise
 *
 * @author   Martin Reddy
 * @version  1 May 1998
 *
 * @see      tsmOpenStream, tsmCloseStream, tsmStreamToFile,
 *           tsmStreamToBuffer
 *
 */

int
tsmStreamToStdout( FILE *stream )
{
  char buf[2048];

  if (!stream) {
    fprintf (stderr, "ERROR: tsmStreamToStdout given a NULL input stream!\n");
    return (0);
  }

  while (fgets (buf, sizeof (buf), stream)) {
    fputs (buf, stdout);
    fflush (stdout);
  }

  return (1);
}
