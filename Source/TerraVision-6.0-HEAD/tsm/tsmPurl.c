/*
 * Name: tsmPurl.c
 *
 * Description: 
 *   Functions for creating and manipulating Purl ("parsed url")
 *   structs, and for parsing URLs into Purls.
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: tsmPurl.c,v 1.30.6.3 2002/03/14 05:22:58 reddy Exp $
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

#ifdef HAVE_PWD_H
#include <pwd.h>
#else
#ifdef MSVC
#include <direct.h>
#endif
#endif

char *tsmProtocolStrings[tsmNumProtocols+1] = {
  "unknown protocol",
  "no protocol",
  "x-dsm",
  "file",
  "http",
  "x-iss",
  "file",
  "x-dpss",
  "http"
};

/* ================================================================
   tsmMakePurl creates an empty Purl struct with
   NULL components and the "unknown protocol".
   ================================================================ */

Purl *
tsmMakePurl (void)
{
  static char * funcName = "tsmMakePurl()";
  Purl *purl;

  if ( ( purl = (Purl *) tsmMemCalloc (sizeof (Purl), 1) ) == NULL ) {
    tsmError (funcName, "Unable to allocate purl.");
    return NULL;
  }

  purl->protocol = tsmUnknownProtocol;

  return (purl);
}

/* ================================================================
   tsmCopyPurl makes a complete copy of purl.  (If purl is NULL, then
   NULL is returned.) 
   ================================================================ */

Purl *
tsmCopyPurl (Purl *purl)
{
  Purl *newPurl=NULL;

  if (purl) {
    newPurl = tsmMakePurl ();
    if (purl->protocolString)
      newPurl->protocolString = tsmMemStrdup (purl->protocolString);
    if (purl->originalUrl)
      newPurl->originalUrl = tsmMemStrdup (purl->originalUrl);
    if (purl->url)
      newPurl->url = tsmMemStrdup (purl->url);
    if (purl->hostName)
      newPurl->hostName = tsmMemStrdup (purl->hostName);
    if (purl->port)
      newPurl->port = tsmMemStrdup (purl->port);
    if (purl->uri)
      newPurl->uri = tsmMemStrdup (purl->uri);
    newPurl->protocol = purl->protocol;
  }
  return (newPurl);
}

/* ================================================================
   tsmFreePurl frees the strings in purl and then frees
   purl itself.
   ================================================================ */

void
tsmFreePurl (Purl *purl)
{
  if (purl) {
    if (purl->protocolString) tsmMemFree (purl->protocolString);
    if (purl->originalUrl) tsmMemFree (purl->originalUrl);
    if (purl->url) tsmMemFree (purl->url);
    if (purl->hostName) tsmMemFree (purl->hostName);
    if (purl->port) tsmMemFree (purl->port);
    if (purl->uri) tsmMemFree (purl->uri);
    tsmMemFree (purl);
  }
}

/* ================================================================
   tsmPrintPurl prints the components of purl to fp.
   ================================================================ */

void
tsmPrintPurl (FILE *fp, const Purl *purl)
{
  fprintf (fp, "Original URL: '%s'.\n", purl->originalUrl);
  fprintf (fp, "Parsed URL:   '%s'.\n", purl->url);
}

/*
 * Synopsis:
 *      void tsmFilenameToURL( const char *filename, const char *url, int )
 *
 * Description:
 *      Takes a filename on the local disk and produces a string containing
 *      a file: URL for that filename. If you provide a relative filename
 *      (i.e. no leading '/'), then the current working directory will be
 *      prepended. The resulting local URL will always have any trailing
 *      '/' removed. e.g. "/home/mxr/" will give "file://localhost/home/mxr"
 *
 *      N.B. if the filename is already a URL, e.g. contains "http:" or 
 *      "file:", etc., then no conversion is performed (other than to
 *      strip any trailing '/' character). Also, if the filename is "stdout",
 *      "stdin", or "stderr", then this is converted to a string in the
 *      form file://localhost/stdout.
 *
 *      Filenames of the form "file://localhost/~user/", "~user/...", and
 *      "~/..." will all be expanded correctly.
 *
 *      If the web_path flag is set to TRUE, then any ~ characters are 
 *      expanded to the directory public_html/ in the relevant account.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 18 July 1997
 *
 */

void
tsmFilenameToUrl( const char *filename, char *url, int web_path )
{
  struct passwd *password;
  char   *cwd, *temp_ptr, *home_dir, *web_dir_name;
  char   userbuf[64], *pathptr, filebuf[512];
  int    len, is_win = FALSE;

  /* check for a valid filename. */

  if ( filename == NULL || strlen( filename ) == 0 ) {
    strcpy( url, "" );
    return;
  }
  
  /* is the filename for a windows system, e.g. C:\foo */

  if ( filename[1] == ':' ) is_win = TRUE;

  /* are we putting in a web directory? */

  web_dir_name = ( web_path ) ? "public_html/" : "";

  /* Skip leading spaces otherwise this can screw things up */

  while ( *filename && isspace( *filename ) ) ++filename;

  /* If we already have a file://localhost URL, but it contains a '~' */
  /* character, then we will strip away the protocol/host bit to make */
  /* sure that the '~' character is correctly expanded.               */

  if ( strncmp( "file://localhost/~", filename, 18 ) == 0 ) 
    filename += 17;

#ifdef HAVE_PWD_H
  /* Expand any filenames beginning '~user' to the full pathname for */
  /* the account "user". Do this by looking up the passwd file entry */
  /* Or if '~/...' is used, use the value of the $HOME env var.      */

  if ( filename[0] == '~' ) {
    if ( filename[1] == '/' ) {

      /* get the value of $HOME to fill in a ~/ style shortcut */

      if ( ( home_dir = getenv( "HOME" ) ) != NULL )
	sprintf( filebuf, "%s/%s", home_dir, &filename[1] );
      else
	strcpy( filebuf, filename );

    } else {

      /* Copy the username to userbuf */

      temp_ptr = (char *) &filename[1];
      len = strcspn( temp_ptr, "/" );
      strncpy( userbuf, temp_ptr, len );
      userbuf[len] = 0;

      /* Set urlPtr to the start of path. */

      pathptr = ( strlen(temp_ptr) > len + 1 ) ? temp_ptr + len + 1 : "";

      /* attempt to lookup the username in the password file to find */
      /* out the full pathname for the account. If this fails, then  */
      /* we will default to the old /homedir/username format.        */

      if ( ( password = getpwnam( userbuf ) ) == NULL )
	sprintf( filebuf, "/homedir/%s/%s%s", userbuf, web_dir_name, pathptr );
      else
	sprintf( filebuf, "%s/%s%s", password->pw_dir, web_dir_name, pathptr );
      
    }

    /* use the expanded filename from now on */

    filename = (char *) filebuf;
  }
#endif

  /* Does the filename already have a protocol specified? We check this */
  /* by just seeing if the filename contains a colon ':'                */

  if ( strchr( filename, ':' ) != NULL && ! is_win ) {
    strcpy( url, filename );
  } else {

    /* create the base URL protocol and host. If the filename is relative */
    /* then prepend the cwd, otherwise just add the filename as-is        */

    strcpy( url, "file://localhost" );

    if ( ( strcmp( filename, "stdout" ) == 0 ) ||
	 ( strcmp( filename, "stderr" ) == 0 ) ||
	 ( strcmp( filename, "stdin" ) == 0 ) || 
	 is_win )
      strcat( url, "/" );
    else if ( filename[0] != '/' ) {
      if ( ( cwd = (char *) getcwd( NULL, 256 ) ) != NULL ) {
	if ( cwd[0] != '/' )
	  strcat( url, "/" );
	strcat( url, cwd );
	free( cwd );
      }
      if ( url[strlen(url)-1] != '/' ) strcat( url, "/" );
    }

    strcat( url, filename );

  }

  /* remove any trailing '/' character */

  len = strlen(url);
  if ( url[len-1] == '/' )
    url[len-1] = '\0';
}

static int tsmHttpToLocalFile = FALSE;  /* disabled by default */

/*
 * Synopsis:
 *    void tsmSetHttpToLocalFile (int flag)
 *
 * Doc++ Description: *//** 
 *   Set all http URLs to localhost if possible.
 *
 *   N.B THIS ROUTINE IS CURRENTLY DISABLED - TSMAPI WILL CURRENTLY 
 *   ALWAYS USE AN HTTP URL IF GIVEN ONE. IT WILL NOT TRY TO CONVERT
 *   THIS TO A LOCALHOST FILENAME.
 *
 *   If this flag is set to TRUE, then whenever the tsmApi parses an http URL
 *   it checks to see if the hostname of the URL is that of the local machine,
 *   e.g. the same hostname or "localhost". If this is the case, then the
 *   URL is transformed into a "file://localhost/..." URL so that all access
 *   to the data use the local filesystem calls.
 *
 *   This flag is set to FALSE by default.
 *
 * @param    flag TRUE or FALSE
 *
 * @return   Nothing (void)
 *
 * @author   
 * @version  
 *
 * @see      
 *
 */

void tsmSetHttpToLocalFile (int flag)
{
  /*  tsmHttpToLocalFile = flag; */
}

/* ================================================================
   tsmParseUrl parses a URL of the form:
 
   protocol://hostname:port/uri...
 
   where protocol must be one of the protocols supported by the Tile
   Set Manager, such as "x-iss" or "file", and :port is optional
   (defaults to 80).

   If the string is not a URL, but a filename on a local disk (i.e. it
   has no colon ':' in the string), then the filename is turned into a 
   file: URL and processing follows as if this file: URL was originally
   passed in to the routine.
 
   If the URL does not conform to a supported protocol pattern, then NULL
   is returned.
 
   Otherwise, a Purl is returned containing the URL components.

   NOTES:

   o The "/uri..." after hostname is optional.  

   o The "/" preceding "uri" above is always included in the uri slot
   of Purl, even if "uri" is empty or the preceding "/" is
   missing.

   o By default, If the protocol is file or http and the host name is
   either "localhost" or an alias the local host, then the protocol is
   translated to tsmLocalFile and the host name is tranlated to
   "localhost".  You can override the translation of http to localfile
   by calling tsmSetHttpToLocalFile(FALSE).

   o When the protocol is tsmLocalFile, then uri's beginning with
   "/~username" are now translated to the correct, full pathname (by
   looking up the passwd file entry).

   For example, the uri for "file://localhost/~magic/Pyramids" becomes
   something like "/home/crazy2/magic/public_html/Pyramids" at SRI, but
   elsewhere it could be "/usr/people/magic/public_html/Pyramids".

   ================================================================ */

Purl *
tsmParseUrl (const char *url) 
{
  static char * funcName = "tsmParseUrl";
  struct passwd *password;    /* used to get homedir for an account */
  Purl *purl;
  const char *urlPtr;
  char buffer[1024], *buffer1, *endbuf;
  char uri_buffer[1024];
  int len, portLen, host_match, i, j, is_win = FALSE;

  if (! url) return 0;

  TSMTRINC ((TSMMASK|5, 1));
  TSMTRACE ((TSMMASK|5, funcName, "Original URL: '%s'.", url));

  /* is the filename for a windows system, e.g. C:\foo */

  if ( url[1] == ':' ) is_win = TRUE;

  /* first we take a copy of the url into buffer. If the url does not  */
  /* have a colon ':' in it then we attempt to convert this to a file: */
  /* URL using the tsmFilenameToUrl() function. Also, if the URL has   */
  /* a '~' character in it, then we pass it to tsmFilenameToUrl.       */

  if ( strchr( url, ':' ) == NULL || strchr( url, '~' ) != NULL || is_win ) {
    tsmFilenameToUrl( url, buffer, FALSE );
    TSMTRACE ((TSMMASK|5, funcName, "Transformed URL: '%s'.", buffer));
  }
  else
    strcpy( buffer, url );

  /* convert any Windows style (\) slashes to Unix style (/) */

  buffer1 = (char *) buffer;
  while ( *buffer1 ) {
    if ( *buffer1 == '\\' ) 
      *buffer1 = '/';
    buffer1++;
  }

  /* Skip leading spaces. */

  buffer1 = (char *) buffer;
  while (*buffer1 && isspace (*buffer1)) ++buffer1;

  /* Strip trailing spaces. */

  endbuf = buffer1 + strlen( buffer1 ) - 1;
  while ( endbuf >= buffer1 && isspace(*endbuf) )
    *endbuf-- = '\0';

  /* Extract the protocol. */

  if (strncasecmp (buffer1, "x-dsm://", 8) == 0) {
    purl = tsmMakePurl ();
    purl->protocol = tsmDSM;
    purl->protocolString = tsmMemStrdup( "x-dsm" );
    urlPtr = &buffer1[8];
  }
  else if (strncasecmp (buffer1, "file://", 7) == 0) {
    purl = tsmMakePurl ();
    purl->protocol = tsmFile;
    purl->protocolString = tsmMemStrdup( "file" );
    urlPtr = &buffer1[7];
  }
  else if (strncasecmp (buffer1, "http://", 7) == 0) {
    purl = tsmMakePurl ();
    purl->protocol = tsmHTTP;
    purl->protocolString = tsmMemStrdup( "http" );
    urlPtr = &buffer1[7];
  }
  else if (strncasecmp (buffer1, "x-iss://", 8) == 0) {
    purl = tsmMakePurl ();
    purl->protocol = tsmISS;
    purl->protocolString = tsmMemStrdup( "x-iss" );
    urlPtr = &buffer1[8];
  }
  else if (strncasecmp (buffer1, "x-dpss://", 9) == 0) {
    purl = tsmMakePurl ();
    purl->protocol = tsmDPSS;
    purl->protocolString = tsmMemStrdup( "x-dpss" );
    urlPtr = &buffer1[9];
  }
  else { /* Unknown protocol, return NULL. */
    TSMTRACE ((TSMMASK|5, funcName, "Unknown Protocol."));
    TSMTRINC ((TSMMASK|5, -1));
    return (NULL);
  }

  /* Duplicate the url now that we have a valid protocol. */
  purl->originalUrl = tsmMemStrdup (buffer1);

  /* Skip any spaces between the protocol string and the hostname */

  while ( isspace(*urlPtr) && urlPtr < buffer1+strlen(buffer1) ) ++urlPtr;

  /* Extract the hostname by finding the first non-/ character. Note
     that strscpn will do the right thing even if the final "/" after
     the hostname is missing. */

  len = strcspn (urlPtr, "/");
  
  /* Search for the port number by looking for a ":" If no ":" is
     present, then the whole string is the hostname.  Otherwise, only
     that part up to the ":" is the port number. */

  portLen = strcspn (urlPtr, ":");
  if (portLen >= len) {
    purl->hostName = (char *) tsmMemCalloc (1, len+1);
    if(!purl->hostName) {
      tsmError (funcName, "Unable to allocate purl hostname");
      TSMTRINC ((TSMMASK|5, -1));
      return NULL;
    }
    strncpy (purl->hostName, urlPtr, len);
    purl->hostName[len] = 0;
    urlPtr = &urlPtr[len];
    purl->port = tsmMemStrdup ("80");
  }
  else {
    purl->hostName = (char *) tsmMemCalloc (1, portLen+1);
    if(!purl->hostName) {
      tsmError (funcName, "Unable to allocate purl hostname.");
      TSMTRINC ((TSMMASK|5, -1));
      return NULL;
    }
    strncpy (purl->hostName, urlPtr, portLen);
    purl->hostName[portLen] = 0;
    urlPtr = &urlPtr[portLen+1];
    purl->port = (char *) tsmMemCalloc (1, len-portLen);
    if(!purl->port) {
      tsmError (funcName, "Unable to allocate purl port.");
      TSMTRINC ((TSMMASK|5, -1));
      return NULL;
    }
    strncpy (purl->port, urlPtr, len-portLen-1);
    purl->port[len-portLen-1] = 0;
    urlPtr = &urlPtr[len-portLen-1];
  }

  /* if the protocol is file and the hostname is localhost, */
  /* then we can specialize the protocol to tsmLocalFile.   */

  if ( purl->protocol == tsmFile && 
       strcasecmp( purl->hostName, "localhost" ) == 0 )
    purl->protocol = tsmLocalFile;

  /* If we have an HTTP URL with a hostname that is an alias for */
  /* localhost, then specialize the protocol to tsmLocalFile,    */
  /* (but only if the HttpToLocalFile flag is set to TRUE.)      */

  if ( tsmHttpToLocalFile && purl->protocol == tsmHTTP
       && ( host_match = tsmIsLocalHost( purl->hostName ) ) > 0 ) {

    /* if we got here then it could be because the domain name of */
    /* of the local host and the http host were equal. In which   */
    /* case it is likely that the file is accessible on localdisk */
    /* via NFS. But just to check, we see if we can access the    */
    /* uri part of the URL and only then set protocol to file://  */

    if ( host_match != 1 ) {
      PRFileInfo file_stat;
      char   temp_buffer[512], *temp_uri;
    
      /* We haven't parsed the purl->uri bit yet, so let's expand it */
      /* here using the tsmFilenameToUrl function (so that any tilde */
      /* [~] characters are expanded correctly for the local host).  */
      
      if ( urlPtr[1] == '~' ) {
	tsmFilenameToUrl( &urlPtr[1], temp_buffer, TRUE );
	temp_uri = temp_buffer + strlen("file://localhost");
      } else {
	strcpy( temp_buffer, urlPtr );
	temp_uri = temp_buffer;
      }

      /* check to see if the file is accessible via NFS */

      host_match = ( PR_GetFileInfo ( temp_uri, &file_stat ) == PR_SUCCESS );

      TSMTRACE ((TSMMASK|5, funcName, "stat file %s = %d\n",
		 temp_uri, host_match ));
    }

    /* if the file or http URL is accessible on localhost, */
    /* then let's change the URL to a file://localhost URL */
    
    if ( host_match == TRUE ) {
      TSMTRACE ((TSMMASK|5, funcName, "Setting %s to localhost URL", purl->hostName));
      purl->protocol = tsmLocalFile;
      tsmMemFree( purl->protocolString ); /* this must've been alloc'd above */
      purl->protocolString = tsmMemStrdup( "file" );
      /* Get rid of the old hostName and replace it with "localhost". */
      tsmMemFree (purl->hostName);
      purl->hostName = tsmMemStrdup ("localhost");
    }
  }

  /* Finally, extract the uri.  Always start the uri with "/" */
  /* unless we have a windows style filename, e.g. C:\        */

#ifdef HAVE_PWD_H
  /* If the uri begins with "/~username/path", expand this to a full 
     pathname, e.g. "/homedir/username/public_html/path". */

  if ((purl->protocol == tsmLocalFile) &&
      (urlPtr[0] == '/') && (urlPtr[1] == '~')) {

    /* Copy username to buffer1 to the start of username. */

    urlPtr = &urlPtr[2];
    len = strcspn (urlPtr, "/");
    strncpy (buffer1, urlPtr, len);
    buffer1[len] = 0;

    /* Set urlPtr to the start of path. */

    urlPtr += len+1;

    /* attempt to lookup the username in the password file to find */
    /* out the full pathname for the account. If this fails, then  */
    /* we will default to the old /homedir/username format.        */

    if ( ( password = getpwnam( buffer1 ) ) == NULL )
      sprintf( uri_buffer, "/homedir/%s/public_html/%s", buffer1, urlPtr );
    else
      sprintf( uri_buffer, "%s/public_html/%s", password->pw_dir, urlPtr );

    purl->uri = tsmMemStrdup( uri_buffer );

  } else
#endif /* HAVE_PWD_H */

  { /* Otherwise, copy the string. */
    len = strlen (urlPtr);
    if (len == 0) {
      purl->uri = tsmMemStrdup ("/");
    }
    else {

      /* if the uri is stdout/stdin/stderr, or if the uri looks to be a */
      /* Win32 style full path (C:\...), then don't add the '/' prefix. */

      if ( ( strcmp( urlPtr, "/stdout" ) == 0 ) ||
	   ( strcmp( urlPtr, "/stdin" ) == 0 ) ||
	   ( strcmp( urlPtr, "/stderr" ) == 0 ) ||
	   ( urlPtr[0] == '/' && urlPtr[2] == ':' ) ) {
	urlPtr++;
	len--;
      }

      if ( ( purl->uri = (char *) tsmMemCalloc(1, len+1) ) == NULL ) {
	tsmError (funcName, "Unable to allocate purl uri");
	TSMTRINC ((TSMMASK|5, -1));
	return NULL;
      }
      strncpy (purl->uri, urlPtr, len);
      purl->uri[len] = 0;
    }
  }

  /* collapse any multiple contiguous '/' symbols to a single '/' */

  if ( purl->uri != NULL ) {
    for ( i = 0; i < strlen( purl->uri ); ) {
      if ( purl->uri[i] == '/' && purl->uri[i+1] == '/' ) {
	for ( j = i + 1; j < strlen( purl->uri ); j++ )
	  purl->uri[j] = purl->uri[j+1];
      } else
	i++;
    }
  }

  /* check to see if this URL is for an OGC Web Mapping Server */

  if ( strchr( purl->uri, '?' ) != NULL )
    purl->protocol = tsmWMT;

  /* recreate the URL string */

  purl->url = (char *) 
     tsmMemCalloc (1, strlen (purl->protocolString) + 3 +
			  strlen (purl->hostName) + 1 +
			  strlen (purl->port) + 1 +
			  strlen (purl->uri) + 1);
  if(!purl->url) {
    tsmError (funcName, "Unable to allocate purl url");
    TSMTRINC ((TSMMASK|5, -1));
    return NULL;
  }
  if (strcmp (purl->port, "80") == 0)
    sprintf (purl->url, "%s://%s%s%s",
	     purl->protocolString, purl->hostName,
	     (purl->uri[0]!='/')?"/":"", purl->uri);
  else
    sprintf (purl->url, "%s://%s:%s%s%s",
	     purl->protocolString, purl->hostName, purl->port,
	     (purl->uri[0]!='/')?"/":"", purl->uri);

  TSMTRACE ((TSMMASK|5, funcName, "Parsed URL: '%s'.", purl->url));
  TSMTRINC ((TSMMASK|5, -1));
  return (purl);
}


/*
 * Synopsis:
 *    int tsmIsUrlValid( char const *url )
 *
 * Doc++ Description: *//** 
 *   Checks to see if the string is a valid filename or url
 *
 *   This routine will try to parse the given string as a localhost filename
 *   or a fully-specified URL. If this fails, then the function returns
 *   a value of FALSE, otherwise a value of TRUE is returned. A valid URL
 *   must contain a protocol string ("file://", "http://", or "x-dpss://"),
 *   a hostname, and optionally a path.
 *
 * @param    url the url string to test for validity
 *
 * @return   TRUE if string is a valid url
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      
 *
 */

int 
tsmIsUrlValid( char const *url )
{
  Purl *purl;

  /* Check for an invalid URL */

  if ( url == NULL || ( purl = tsmParseUrl( url ) ) == NULL ) return FALSE;

  /* Otherwise the URL must be okay! */

  tsmFreePurl( purl );
  return TRUE;
}
