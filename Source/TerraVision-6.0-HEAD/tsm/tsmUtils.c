/*
 * Name: tsmUtils.c
 *
 * Description: 
 *   Utility functions.  These functions generally require a trace level
 *   of at least 3 before any tracing occurs.
 *
 * Function List:
 *
 * Revision History:
 *   Created by Yvan Leclerc <leclerc@ai.sri.com>
 *
 *   "$Id: tsmUtils.c,v 1.46.2.4 2002/03/14 05:22:59 reddy Exp $"
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
#include "tsmConnectionInt.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static double tsmDet2x2( float a, float b, float c, float d );
static double tsmDet3x3( float a1, float a2, float a3,
			 float b1, float b2, float b3,
			 float c1, float c2, float c3 );
static void tsmAdjoint3x3( float *in, float *out );


/*
 * Synopsis:
 *   void tsmEnsureGeoNameSet( Pyramid pyramid ) 
 *
 * Description:
 *   As of tsmApi 1.1, we no longer require GeoPyramid tspecs. Instead,
 *   we introduce a geo-name field to the Pyramid tspec files. For 
 *   backward compatibility we automatically generate a geo-name if
 *   one is not specified. We do this by taking the dataset name and
 *   chopping off any extensions and capitalising the first letter,
 *   e.g. "irwin.128.oi" -> "Irwin"
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 28 October 1998
 *
 */

void
tsmEnsureGeoNameSet( Pyramid *pyramid ) 
{
  if ( pyramid != NULL && pyramid->geoName == NULL ) {
    char *period;

    pyramid->geoName = tsmMemStrdup( pyramid->identityTspec.name );
    pyramid->geoName[0] = toupper( pyramid->geoName[0] );

    if ( ( period = strchr( pyramid->geoName, '.' ) ) != NULL )
      *period = '\0';
  }
}

/*
 * Synopsis:
 *   int tsmSystem( const char *cmd )
 *
 * Doc++ Description: *//** 
 *   Run a command.
 *
 *   Under Unix, this will simply use the system() call.
 *   Under Windows, we use the CreateProcess() call.
 *
 * @param    cmd the command you wish to be execute
 *
 * @return   Returns a value > 0 if all was successful
 *
 * @author   Martin Reddy
 * @version  19 September 2001
 *
 * @see      tsmShowUrl
 *
 */

int
tsmSystem( char const *cmd )
{
#if defined(_WIN32) || defined(WIN32)
  STARTUPINFO         startupInfo;
  PROCESS_INFORMATION procInfo;
  BOOL                retVal;

  /* initialise the required params for CreateProcess */

  ZeroMemory( &startupInfo, sizeof(startupInfo) );
  startupInfo.cb = sizeof( startupInfo );

  /* do the CreateProcess() call to run the command */

  retVal = CreateProcess( NULL,
			  cmd,      /* the command line to run */
			  NULL,         /* security attributes for process */
			  NULL,         /* security attributes for primary
				         * thread in the new process */
			  FALSE,         /* inherit handles */
			  CREATE_NO_WINDOW,  /* creation flags */
			  NULL,         /* environment block */
			  NULL,         /* current drive and directory */
			  &startupInfo, /* our pipe handles */
			  &procInfo     /* filled on return */
			  );
  
  CloseHandle(procInfo.hThread);
  CloseHandle(procInfo.hProcess); /* detach process */

  return retVal;
#else
  return system( cmd );
#endif
}

static char *web_browser = NULL;   /* the command to run your web browser */

/*
 * Synopsis:
 *   int tsmSetWebBrowser( const char *browser )
 *
 * Doc++ Description: *//** 
 *   Set the command to run your web browser.
 *
 *   This function will set the command that is used in order to invoke
 *   a web browser on your system. This is used by the tsmShowUrl
 *   function when it displays a specified URL. By defaut, tsmShowUrl
 *   will assume a browser command of "netscape".
 *
 *   This function takes a copy of the string you pass (strdup).
 *
 * @param    browser the command to run your web browser
 *
 * @return   TRUE if browser name was changed, FALSE if you passed NULL.
 *
 * @author   Martin Reddy
 * @version  5 February 1999
 *
 * @see      tsmShowUrl
 *
 */

int
tsmSetWebBrowser( const char *browser )
{
  if ( browser == NULL ) return FALSE;
  if ( web_browser != NULL ) tsmMemFree( web_browser );
  web_browser = tsmMemStrdup( browser );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmShowUrl( const char *url )
 *
 * Doc++ Description: *//** 
 *   Displays a web page using netscape.
 *
 *   This function will take a URL as input and then attempt to
 *   display it using the netscape browser. This assumes that the
 *   program "netscape" is somewhere in your $PATH (and also that
 *   /bin/sh exists). 
 *
 *   The function is intelligent in that if there is already a
 *   netscape session currently running, then it will display the
 *   URL using that netscape, rather than trying to run another 
 *   netscape from scratch. Also, this function will not block
 *   until you quit netscape - as soon as the page has been displayed,
 *   control is passed back to the client.
 *
 *   This function can be useful for embedding web documentation or
 *   links in your program, e.g. 
 *     tsmShowUrl( "http://www.ai.sri.com/~magic/tsmApi" );
 *
 *   N.B. the URL string that you pass in must begin with "http:" or
 *   "file:" (lower case only). If not, then an error (FALSE) will be
 *   returned.
 *
 *   You can change the web browser to use by using the tsmSetWebBrowser
 *   function. If you are compiling this under Windows, or if the
 *   web browser is not specified as netscape, then this function will
 *   not attempt to do the fancy -remote stuff.
 *
 * @param    url the http or file URL to be displayed
 *
 * @return   TRUE if command was executed (it still might fail in the shell),
 *           FALSE if an error occurred trying to execute the command.
 *
 * @author   Martin Reddy
 * @version  17 March 1998
 *
 * @see      tsmSetWebBrowser
 *
 */

int
tsmShowUrl( const char *url )
{
  static const char *funcName = "tsmShowUrl";
  static const char *script = "/tmp/tsmShowUrl.sh";
  static const char *netscape_browser = "netscape";
  static const char *explorer_browser = "explorer";
  FILE              *handle;
  char              *cmd, *comspec;
  int               is_windows = FALSE;

#if defined(_WIN32) || defined(WIN32)
  is_windows = TRUE;
#endif

  /* first of all, if this is Windows, then we can try and */
  /* run the default web browser as follows.               */

  if ( is_windows && ( comspec = getenv( "COMSPEC" ) ) ) {
    cmd = (char *) tsmMemMalloc( strlen( comspec ) + strlen(url) + 20 );
    if ( cmd == NULL ) return FALSE;

    sprintf( cmd, "\"%s\" /c start %s &", comspec, url );

    TSMTRACE(( TSMMASK|1, funcName, "Running command: %s", cmd ));

    tsmSystem( cmd );
    tsmMemFree( cmd );
    return TRUE;
  }

  /* Set up netscape as the default browser if none specified */
  /* or use Internet Explorer by default if using a PC.       */

  if ( web_browser == NULL ) {
    if ( is_windows ) {
      web_browser = tsmMemStrdup( explorer_browser );
    } else {
      web_browser = tsmMemStrdup( netscape_browser );
    }
  }

  /* Check that we have a valid URL */

  if ( url == NULL || strlen( url ) == 0 ) {
    tsmError( funcName, "Invalid URL. Must be non NULL and non empty" );
    return FALSE;
  }

  /* Check that we have an http: or a file: URL */

  if ( strncmp( url, "http:", 5 ) != 0 && strncmp( url, "file:", 5 ) != 0 ) {
    tsmError( funcName, "URL must begin with http: or file: (%s)", url );
    return FALSE;
  }

  /* If we are compiling under Windows, or if the web browser being */
  /* used is not netscape, then just do the simple case of spawning */
  /* the browser with the specified string. We assume that we are   */
  /* still running in a shell environment with background process   */
  /* capabilities (i.e. ending command in &), e.g. bash/tcsh shell  */

  if ( is_windows || !strstr( web_browser, netscape_browser ) ) {
  
    cmd = (char *) tsmMemMalloc( strlen(web_browser) + strlen(url) + 10 );
    if ( cmd == NULL ) return FALSE;

    sprintf( cmd, "\"%s\" %s &", web_browser, url );

    TSMTRACE(( TSMMASK|1, funcName, "Running command: %s", cmd ));

    tsmSystem( cmd );
    tsmMemFree( cmd );
    return TRUE;
  }

#ifdef HAVE_UNISTD_H
  /* If we are compiling under Unix, and are using the netscape */
  /* browser, then use the -remote command line option in order */
  /* to use an already running netscape process if available.   */

  /* Try to create a shell script that will do the dirty */

  if ( ( handle = fopen( script, "w" ) ) == NULL ) {
    tsmError( funcName, "Couldn't write temp file: %s\n", script );
    return FALSE;
  }

  fprintf( handle, "#!/bin/sh\n# generated by tsmShowUrl (tsmApi)\n" );
  fprintf( handle, "if ( netscape -remote \"openURL(%s)\" 2>/dev/null", url );
  fprintf( handle, " ) ; then\n  :\nelse\n netscape %s &\nfi\n", url );
  fclose( handle );

  /* set execute permissions for the script that we just generated  */
  /* and then try to execute the script in a shell. Once the script */
  /* has completed its work, we can delete it.                      */

  TSMTRACE(( TSMMASK|1, funcName, "Running shell script: %s", script ));

  chmod( script, 0775 );
  system( script );
  unlink( script );

  return TRUE;
#else
  return FALSE;
#endif
}

/*
 * Synopsis:
 *   double tsmElapsedTime (PRIntervalTime, PRIntervalTime)
 *
 * Doc++ Description: *//** 
 *   Calculates the difference between two times.
 *
 * tsmElapsedTime returns the time difference between a stop and
 * start timestamp. The value is returned in units of seconds.
 *
 * @param    start the start time
 * @param    end the end time
 *
 * @return   the time difference (seconds)
 *
 * @author   
 * @version  
 *
 * @see      
 *
 */

double
tsmElapsedTime (PRIntervalTime start, PRIntervalTime end)
{
  double diff_sec = ((PRIntervalTime)(end - start))
    / (double)PR_TicksPerSecond();
  return diff_sec;
}

/*
 * Synopsis:
 *   char *tsmDateString( const time_t the_time )
 *
 * Description:
 *   Concatenates a string onto the end of a (possibly) existing
 *   string, growing the size of the original string and returning
 *   its (potentially) new address as the result, e.g.
 *
 *   char *string = NULL;
 *   string = tsmStringCat( string, "Hello" );
 *   string = tsmStringCat( string, " World" );
 *
 * Author: Martin Reddy
 *
 * Date: 20 December 2001
 *
 */
char *
tsmStringCat( char const *string, char const *suffix )
{
  char *newstring;
  if ( suffix == NULL ) return string;
  if ( string == NULL ) return strdup( suffix );
  newstring = realloc( string, strlen( string ) + strlen( suffix ) + 1 );
  strcat( newstring, suffix );
  return newstring;
}


/*
 * Synopsis:
 *   char *tsmDateString( const time_t the_time )
 *
 * Description:
 *   Return current date string.
 *
 *   This function returns a date string in the same format as that 
 *   returned by the unix 'date' command. If the parameter to this
 *   function is NULL, then it will return a string for the current
 *   time/date. If you pass a non-zero time_t variable, then the
 *   function will return a date string for that time. If you pass
 *   a value of -1, then "n/a" will be returned.
 *
 * Externals: None
 *
 * Author: Martin Reddy
 *
 * Date: 10 December 1997
 *
 */
char* 
tsmDateString( PRTime the_time)
{
    static char dayNames[7][4] =
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static char monNames[12][4] =
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
         "Nov", "Dec"};
    static char timeString[30];   /*have a strict format; this is big enough*/
    PRExplodedTime t;

    /* NOTE: Under Linux at least, there's a subtle difference between
     * the date/time reported by the "date" command vs. ctime():  the later
     * does not include timeszone designation.  --RLB
     */
    if ( the_time == (PRTime) -1 )
	return "n/a";
    if ( the_time > 0 )
	PR_ExplodeTime(the_time, PR_GMTParameters, &t);
    else
	PR_ExplodeTime(PR_Now(), PR_GMTParameters, &t);
    
    sprintf(timeString, "%s %s %2d %02d:%02d:%02d %4d",
	dayNames[t.tm_wday], monNames[t.tm_month], t.tm_mday, t.tm_hour, 
	t.tm_min, t.tm_sec, t.tm_year);

    return timeString;
} 
	    

/*
 * Synopsis:
 *   TsmPlanet tsmStringToPlanet( const char *name )
 *
 * Doc++ Description: *//** 
 *   Converts a string into a TsmPlanet variable.
 *
 *   This function will take a string and attempt to convert it to the
 *   enumerated type TsmPlanet, e.g. "mercury" -> TSM_MERCURY.  The
 *   string comparison if case insensitive. If the string does not
 *   contain a recognised planet name, or is NULL, then TSM_EARTH will
 *   be returned.
 *
 * @param    name (char *) the string containing a planet name, e.g. "Earth"
 *
 * @return   the TsmPlanet conversion of the string
 *
 * @author   Martin Reddy
 * @version  28 August 1998
 *
 * @see      tsmPlanetToString
 * */

TsmPlanet
tsmStringToPlanet( const char *name )
{
  if ( name == NULL ) return TSM_EARTH;

  if ( ! strcasecmp( name, "mercury" ) )
    return TSM_MERCURY;
  else if ( ! strcasecmp( name, "venus" ) )
    return TSM_VENUS;
  else if ( ! strcasecmp( name, "earth" ) )
    return TSM_EARTH;
  else if ( ! strcasecmp( name, "mars" ) )
    return TSM_MARS;
  else if ( ! strcasecmp( name, "jupiter" ) )
    return TSM_JUPITER;
  else if ( ! strcasecmp( name, "saturn" ) )
    return TSM_SATURN;
  else if ( ! strcasecmp( name, "uranus" ) )
    return TSM_URANUS;
  else if ( ! strcasecmp( name, "neptune" ) )
    return TSM_NEPTUNE;
  else if ( ! strcasecmp( name, "pluto" ) )
    return TSM_PLUTO;
  else
    return TSM_EARTH; /* default if string not make sense */
}

/*
 * Synopsis:
 *   char *tsmPlanetToString( TsmPlanet planet )
 *
 * Doc++ Description: *//** 
 *   Converts a TsmPlanet variable to a string.
 *
 *   This function will take a TsmPlanet enumerated type and convert it
 *   to a string, e.g. TSM_MERCURY -> "Mercury". The string will be
 *   capitalised. The returned string should not be freed or modified.
 *
 * @param    planet (TsmPlanet) the enumerated type to be converted
 *
 * @return   the string representation of the planet name.
 *
 * @author   Martin Reddy
 * @version  28 August 1998
 *
 * @see      tsmStringToPlanet
 *
 */

char *
tsmPlanetToString( TsmPlanet planet )
{
  switch ( planet ) {
  case TSM_MERCURY:
    return "Mercury";
  case TSM_VENUS:
    return "Venus";
  case TSM_EARTH:
    return "Earth";
  case TSM_MARS:
    return "Mars";
  case TSM_JUPITER:
    return "Jupiter";
  case TSM_SATURN:
    return "Saturn";
  case TSM_URANUS:
    return "Uranus";
  case TSM_NEPTUNE:
    return "Neptune";
  case TSM_PLUTO:
    return "Pluto";
  default:
    return "";
  }
} 

/* ================================================================
   tsmParsedIdentifier attempts to parse an identifier as a URL.  It
   returns the parsed URL if its successful.  Otherwise, it looks up
   the corresponding URL in the nameUrlPairs and returns the parsed
   version of that.
   ================================================================ */

Purl *
tsmParseIdentifier (const char *identifier, NameUrlIds *nameUrlIds)
{
  const char *funcName="tsmParseIdentifier";
  Purl *purl = NULL;
  char const *url;

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName, "identifier = '%s'.", identifier));

  /* Attempt to parse the identifier as a URL. */

  if ( strchr( identifier, ':' ) != NULL )
    purl = tsmParseUrl (identifier);

  /* If it parses, then we're done. */
  if (purl) {
    TSMTRACE ((TSMMASK|3, funcName, "Parsed URL = '%s'.", purl->url));
    TSMTRINC ((TSMMASK|3, -1));
    return (purl);
  }

  /* Otherwise, its just a name. */
  else {

    /* Make sure that nameUrlIds is there before doing a lookup. */
    if (nameUrlIds) {

      /* Look up its URL in the database. */
      url = tsmNameToUrl (identifier, nameUrlIds);

      /* Return the parsed URL if the URL was found. */
      if (!url) {
	TSMTRACE ((TSMMASK|3, funcName, "Database lookup failed."));
	TSMTRINC ((TSMMASK|3, -1));
	return (NULL);
      }
      else {
	purl = tsmParseUrl (url);
	TSMTRACE ((TSMMASK|3, funcName,
		   "Identifier translated to URL '%s'.", purl->url));
	TSMTRINC ((TSMMASK|3, -1));
	return (purl);
      }
    }
    else {
      TSMTRACE ((TSMMASK|3, funcName, "No nameUrlIds provided!"));
      TSMTRINC ((TSMMASK|3, -1));
      return (NULL);
    }
  }
}

/*
   tsmPurlToSetId takes a parsed URL that was of the form:

   x-iss://hostname/datasettype/setid

   or

   x-iss://hostname/datasettype/setid/whatever

   and returns setid.  If the protocol was not tsmISS or it couldn't
   parsed the uri, then it returns invalidSetId.
*/

int
tsmPurlToSetId (Purl *purl, int invalidSetId)
{
  const char *funcName = "tsmPurlToSetId";
  int setId;

  TSMTRINC ((TSMMASK|3, 1));
  if (purl) {
    TSMTRACE ((TSMMASK|4, funcName,
	       "Extracting set id from URI '%s'.", purl->uri));
    if ((sscanf (purl->uri, "/TileSets/GeoPyramids/%d", &setId) != 1) &&
	(sscanf (purl->uri, "/TileSets/Pyramids/%d", &setId) != 1) &&
	(sscanf (purl->uri, "/GeoPyramids/%d", &setId) != 1) &&
	(sscanf (purl->uri, "/Pyramids/%d", &setId) !=1))
      setId = invalidSetId;
  }
  else
    setId = invalidSetId;

  TSMTRACE ((TSMMASK|3, funcName, "set id = %d.", setId));
  TSMTRINC ((TSMMASK|3, -1));
  return (setId);
}

/*
 * Synopsis:
 *   int tsmAddNameUrlId( NameUrlIds *, char const *, char const *, int )
 *
 * Description:
 *   Takes a name, url, and id tuple describing a single pyramid and
 *   addes these to the appropriate array entries in the specified
 *   NameUrlIds structure. This lets us grow this structure as we find
 *   new entries, rather than just allocating a static amount of memory
 *   and not being able to add new entries.
 *
 *   The two string pointers, name and url, are placed as-is into the
 *   arrays, so you must do a strdup operation on these if you want to
 *   store copies of the strings.
 *
 * Returns: TRUE if the new fields were added to the arrays
 *
 * Author: Martin Reddy
 * 
 * Date: 8 April, 1999
 *
 */

int
tsmAddNameUrlId( NameUrlIds *nameUrlIds, char const *name, char const *url,
		 int id )
{
  const char *funcName = "tsmAddNameUrlId";
  int newsize;

  if ( nameUrlIds == NULL ) return FALSE;

  /* grow the arrays by one */

  newsize = nameUrlIds->numItems + 1;
  
  nameUrlIds->names = (char const **)
    tsmMemRealloc( (void *) nameUrlIds->names, newsize * sizeof(char *) );
  nameUrlIds->urls  = (char const **)
    tsmMemRealloc( (void *) nameUrlIds->urls, newsize * sizeof(char *) );
  nameUrlIds->ids   = (int *)
    tsmMemRealloc( (void *) nameUrlIds->ids, newsize * sizeof(int) );

  if ( nameUrlIds->names == NULL || nameUrlIds->ids == NULL ) {
    tsmError( funcName, "could not grow the NameUrlIds array!" );
    return FALSE;
  }

  /* add the new fields to the end of the lists */
  
  nameUrlIds->names[newsize-1] = (char *) name;
  nameUrlIds->urls[newsize-1]  = (char *) url;
  nameUrlIds->ids[newsize-1]   = id;
  nameUrlIds->numItems         = newsize;

  return TRUE;
}

NameUrlIds *
tsmMakeNameUrlIds (int numItems)
{
  const char *funcName = "tsmMakeNameUrlIds";
  NameUrlIds *nameUrlIds;

  nameUrlIds = (NameUrlIds *) tsmMemManagerCalloc (1, sizeof (NameUrlIds));
  if(!nameUrlIds) {
    tsmError (funcName, "Unable to allocate space for nameUrlIds");
    return NULL;
  }
  nameUrlIds->numItems = numItems;

  /* return immediately if no item arrays to allocate */

  if ( numItems == 0 )
    return nameUrlIds;

  /* otherwise, let's allocate the approprate arrays */

  nameUrlIds->names = (char const **) 
     tsmMemManagerCalloc (1, numItems *sizeof (char *));
  if(!nameUrlIds->names) {
    tsmError (funcName, "Unable to allocate space for nameUrlIds->names");
    return NULL;
  }
  nameUrlIds->urls = (char const **)
     tsmMemManagerCalloc (1, numItems *sizeof (char *));
  if(!nameUrlIds->urls) {
    tsmError (funcName, "Unable to allocate space for nameUrlIds->urls");
    return NULL;
  }
  nameUrlIds->ids = (int *) 
     tsmMemManagerCalloc (1, numItems *sizeof (int));
  if(!nameUrlIds->ids) {
    tsmError (funcName, "Unable to allocate space for nameUrlIds->ids");
    return NULL;
  }
  return (nameUrlIds);
}

void
tsmFreeNameUrlIds (NameUrlIds *nameUrlIds)
{
  int i;

  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (nameUrlIds->names[i])
	tsmMemManagerFree ((void *) nameUrlIds->names[i]);
      if (nameUrlIds->urls[i])
	tsmMemManagerFree ((void *) nameUrlIds->urls[i]);
    }
    tsmMemManagerFree (nameUrlIds->names);
    tsmMemManagerFree (nameUrlIds->urls);
    tsmMemManagerFree (nameUrlIds->ids);
    tsmMemManagerFree (nameUrlIds);
  }
}

char const *
tsmNameToUrl (const char *name, NameUrlIds *nameUrlIds)
{
  int i;

  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (strcmp (name, nameUrlIds->names[i]) == 0) 
	return (nameUrlIds->urls[i]);
    }
  }
  return (NULL);
}

int
tsmNameToId (const char *name, NameUrlIds *nameUrlIds)
{
  int i;

  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (strcmp (name, nameUrlIds->names[i]) == 0) 
	return (nameUrlIds->ids[i]);
    }
  }
  return (-1);
}

const char *
tsmIdToName (int setId, NameUrlIds *nameUrlIds)
{
  int i;

  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (nameUrlIds->ids[i] == setId)
	return (nameUrlIds->names[i]);
    }
  }
  return ("");
}

const char *
tsmIdToUrl (int setId, NameUrlIds *nameUrlIds)
{
  int i;

  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (nameUrlIds->ids[i] == setId)
	return (nameUrlIds->urls[i]);
    }
  }
  return ("");
}

int
tsmUrlToId (char *url, NameUrlIds *nameUrlIds)
{
  const char *funcName="tsmUrlToId";
  int i, id;

  TSMTRINC ((TSMMASK|2, 1));
  if (nameUrlIds) {
    for (i = 0; i < nameUrlIds->numItems; i++) {
      if (strcmp (url, nameUrlIds->urls[i]) == 0) {
	id = nameUrlIds->ids[i];
	TSMTRACE ((TSMMASK|2, funcName,
		   "Translated URL '%s' to id %d.", url, id));
	TSMTRINC ((TSMMASK|2, -1));
	return (id);
      }
    }
  }
  TSMTRACE ((TSMMASK|2, funcName,
	     "Failed translation of URL '%s' to id.", url));
  TSMTRINC ((TSMMASK|2, -1));
  return (-1);
}

char *
tsmSetIdToPyramidTspecUrl (char const *hostName, int setId)
{
  char buffer[1024];

  sprintf (buffer, "x-dsm://%s/TileSets/Pyramids/%d", hostName, setId);
  return (tsmMemManagerStrdup (buffer));
}

char *
tsmPyramidUrlFromSetId (int protocol, char const *hostName, char const *uri, int setId)
{
  char buffer[1024];

  sprintf (buffer, "%s://%s%sPyramids/%d",
	   tsmProtocolStrings[protocol], hostName, uri, setId);
  return (tsmMemManagerStrdup (buffer));
}

char *
tsmPyramidUrlFromString (int protocol, char const *hostName, char const *uri,
			 char const *string)
{
  char buffer[1024];

  sprintf (buffer, "%s://%s%sPyramids/%s",
	   tsmProtocolStrings[protocol], hostName, uri, string);
  return (tsmMemManagerStrdup (buffer));
}

/* External calls for above */

char const *
tsmPyramidNameToUrl (TsmHandle *tsmh, char const *pyramidName)
{
  if (tsmh->pyramidUrls)
    return (tsmNameToUrl (pyramidName, tsmh->pyramidUrls));
  else
    return ("");
}

int
tsmPyramidNameToId (TsmHandle *tsmh, char const *pyramidName)
{
  if (tsmh->pyramidUrls)
    return (tsmNameToId (pyramidName, tsmh->pyramidUrls));
  else
    return (-1);
}

void
tsmNoProtocolYet (TsmProtocol protocol, const char *message)
{
  fprintf (stderr, "ERROR in %s: protocol %s not implemented yet.\n",
	   message, tsmProtocolStrings[protocol] );
}

void
tsmNotCompiledIn( TsmProtocol protocol, const char *message )
{
  fprintf( stderr, "ERROR in %s: %s support not compiled in.\n",
	   message, tsmProtocolStrings[protocol] );
}

/* ================================================================
   tsmAppendRelativePathNameToUrl creates a new URL by appending name
   to the URL specified in url, putting the result in buffer.

   Notes:
   
   o name is assumed to be a "relative path name" that does NOT start
   with "/".  A "/" is appended to the end of the uri if necessary.
================================================================ */

void
tsmAppendRelativePathNameToUrl (const char *name, const char *url, char *buffer)
{
  if (url[strlen(url)-1] == '/') 
    sprintf (buffer, "%s%s", url, name);
  else
    sprintf (buffer, "%s/%s", url, name);
}

/* ================================================================
   tsmAppendRelativePathNameToPurl creates a new Purl by appending
   name to the URL specified in purl and reparsing the URL.

   Notes:
   
   o name is assumed to be a "relative path name" that does NOT start
   with "/".  A "/" is appended to the end of the uri if necessary.
================================================================ */

Purl *
tsmAppendRelativePathNameToPurl (const char *name, const Purl *purl)
{
  char buffer[1024];
  tsmAppendRelativePathNameToUrl (name, purl->url, buffer);
  return (tsmParseUrl (buffer));
}

/* ================================================================ A
   "locator" in this context is a string that is either a
   fully-specified URL or a relative path name.  If the locator can be
   parsed as a URL, then the parsed URL will be returned.  If not, the
   locator is assumed to be a relative path name and is appended to
   purl's uri to form a new purl.
   ================================================================ */

Purl *
tsmAppendLocatorToPurl (const char *locator, const Purl *purl, const char *defaultName)
{
  const char *funcName="tsmAppendLocatorToPurl";
  Purl *newPurl = NULL;

  TSMTRINC ((TSMMASK|3, 1));
  TSMTRACE ((TSMMASK|3, funcName, "Original URL: '%s'.", purl->originalUrl));

  /* If the locator is empty, then set it to the default name. */

  if (!locator || (strlen (locator) == 0)) locator = defaultName;

  /* Attempt to parse the locator as a URL.  If its parsable, then
     use it directly.  */

  if ( strchr( locator, ':' ) != NULL )
    newPurl = tsmParseUrl (locator);

  /* Otherwise, assume its a relative pathname and append it to the
     purl. */

  if (!newPurl && purl) 
    newPurl = tsmAppendRelativePathNameToPurl (locator, purl);

  TSMTRACE ((TSMMASK|3, funcName, "Parsed URL: '%s'.", purl->url));
  TSMTRINC ((TSMMASK|3, -1));
  return (newPurl);
}

const char *
tsmPyramidTspecBaseUrl (Pyramid *pyramid)
{
  return (pyramid->tspecBasePurl->url);
}

const char *
tsmPyramidTileBaseUrl (Pyramid *pyramid)
{
  return (pyramid->tileBasePurl->url);
}

TsmProtocol
tsmPyramidTileProtocol (Pyramid *pyramid)
{
  return (pyramid->tileBasePurl->protocol);
}

const char *
tsmPyramidName (Pyramid *pyramid)
{
  return (pyramid->identityTspec.name);
}

void
tsmPrintPyramidBasePurls (FILE *fp, Pyramid *pyramid)
{
  if (pyramid->tspecBasePurl) {
    fprintf (fp, "tspec Base Purl:\n\n");
    tsmPrintPurl (fp, pyramid->tspecBasePurl);
  }
  if (pyramid->tileBasePurl) {
    fprintf (fp, "tile Base Purl:\n\n");
    tsmPrintPurl (fp, pyramid->tileBasePurl);
  }
}

void
tsmTranslateEmptyUrlsToNamedUrls (TsmHandle *tsmh, NameUrlIds *nameUrlIds, 
				  Purl *basePurl, char *subDirectoryName)
{
  const char *funcName="tsmTranslateEmptyUrlsToNamedUrls";
  int i;
  char buffer1[1024], buffer2[1024];

  for (i = 0; i < nameUrlIds->numItems; i++) {
    if (!nameUrlIds->urls[i] ||
	(strlen (nameUrlIds->urls[i]) == 0)) {
      tsmAppendRelativePathNameToUrl (subDirectoryName, basePurl->originalUrl, buffer1);
      tsmAppendRelativePathNameToUrl (nameUrlIds->names[i], buffer1, buffer2);
      if ( nameUrlIds->urls[i] != NULL )
	tsmMemFree((void *) nameUrlIds->urls[i] );
      nameUrlIds->urls[i] = tsmMemManagerStrdup (buffer2);
      TSMTRACE ((TSMMASK|3, funcName, 
		 "Translated empty [Geo]Pyramid URL to '%s'.", 
		 nameUrlIds->urls[i]));
    }
  }
}

/* ================================================================
   Same as pthr_trace except that stringArg is passed to fprintf as well
   as formatString.
   ================================================================ */

void
pthr_trace_intArray (int traceLevel, const char *funcName,
		     const char *prefix,
		     const IntArray *intArray, int start, 
		     const char *suffix)
{
  if (TSMTRACEP (TSMMASK|traceLevel)) {
    int i, *array = intArray->array;
    char *p, *buf = alloca (intArray->len * 16);

    for (i = start, p = buf; i < intArray->len; i++) {
      sprintf (p, (i == start ? "[%d" : " %d"), array[i]);
      p = strchr (p, '\0');
    }
    strcat (p, "]");

    pthr_trace ("%s: %s%s%s", funcName, prefix, buf, suffix);
  }
}

/*
 * Synopsis:
 *    int tsmPrintTile (SimpleTileSet *simpleTileSet, 
 *                      unsigned char *tileBuffer, int tx, int ty, FILE *file)
 *
 * Doc++ Description: *//** 
 *    Display a tile's buffer data.
 *
 *    Takes the data for a particular tile and outputs a formatted table
 *    of the data values to a stream (e.g. stdout or stderr). This is useful
 *    for querying the contents of a tile that has been retrieved with
 *    tsmGetTile.
 *
 * @param    simpleTileSet the SimpleTileSet containing the tile in question
 * @param    tileBuffer the raw tile buffer data
 * @param    tx the tile's x-coordinate
 * @param    ty the tile's y-coordinate
 * @param    file the stream to output to (e.g. stdout)
 *
 * @return   TRUE if function completed successfully, FALSE if failed.
 *
 * @author   
 * @version  
 *
 * @see      tsmGetTile
 *
 */

int
tsmPrintTile (SimpleTileSet *simpleTileSet, 
	      unsigned char *tileBuffer, int tx, int ty, FILE *file)
{
  TileTspec *tileTspec;
  int returnCode;

  tileTspec = &simpleTileSet->tileTspec;

  if ((tileTspec->numComponentsPixel == 1) &&
      (tileTspec->numBytesPerComponent == 4) &&
      (tileTspec->componentNumberType == tspSignedByte)) {
    returnCode = tsmPrintElevationTile (simpleTileSet, tileBuffer, tx, ty, file);
  }
  else 
  if ((tileTspec->numComponentsPixel == 1) &&
      (tileTspec->numBytesPerComponent == 1) &&
      (tileTspec->componentNumberType == tspUnsignedByte)) {
    returnCode = tsmPrintAverageTile (simpleTileSet, tileBuffer, tx, ty, file);
  }
  else
  if ((tileTspec->numComponentsPixel == 3) &&
      (tileTspec->numBytesPerComponent == 1) &&
      (tileTspec->componentNumberType == tspUnsignedByte)) {
    returnCode = tsmPrintRgbTile (simpleTileSet, tileBuffer, tx, ty, file);
  }
  else {
    returnCode = FALSE;
  }

  return (returnCode);
}

int
tsmPrintElevationTile (SimpleTileSet *simpleTileSet, unsigned char *tileBuffer,
		       int tx, int ty, FILE *file)

{
  TileTspec *tileTspec;
  PixelTspec *pixelTspec;
  int *intTileBuffer;
  int numPixelsPerTileX, numPixelsPerTileY;
  int totalPixelsX, totalPixelsY;
  int ix, iy, i;
  float scale, offset;

  intTileBuffer = (int *) tileBuffer;
  tileTspec = &simpleTileSet->tileTspec;
  pixelTspec = &simpleTileSet->pixelTspec;

  numPixelsPerTileX = tileTspec->numPixelsX;
  numPixelsPerTileY = tileTspec->numPixelsY;
  totalPixelsX = tileTspec->totalPixels[0];
  totalPixelsY = tileTspec->totalPixels[1];

  scale = pixelTspec->scale;
  offset = pixelTspec->offset;

  /* Print column numbers. */
  fprintf (file, "            ");
  for (ix = 0; ix < totalPixelsX; ix++) {
    fprintf (file, "%8d/%3d", (tx * numPixelsPerTileX) + ix, ix);
  }
  fprintf (file, "\n\n");

  for (iy = totalPixelsY-1; iy >= 0; iy--) {
    /* Print row number. */
    fprintf (file, "%8d/%3d:", (ty * numPixelsPerTileY) + iy, iy);
    for (ix = 0; ix < totalPixelsX; ix++) {
      i = ix + (iy * totalPixelsX);
      fprintf (file, "%11.2f ", (intTileBuffer[i] * scale) + offset);
    }
    fprintf (file, "\n");
  }
  return (TRUE);
}

int
tsmPrintAverageTile (SimpleTileSet *simpleTileSet, 
		     unsigned char *tileBuffer, int tx, int ty, FILE *file)
{
  return (FALSE);
}

int
tsmPrintRgbTile (SimpleTileSet *simpleTileSet, 
		 unsigned char *tileBuffer, int tx, int ty, FILE *file)
{
  return (FALSE);
}

#ifdef USE_HTTP

static char pathName[1024];

char *
tsmCopyUrlToTmpFile (char *url)
{
  const char *funcName = "tsmCopyUrlToTmpFile";

  sprintf (pathName, "%s/tsm-tmp-%d", TSM_TMPDIR, (int) getpid());
  if ( ! tsmUrlToFile( NULL, url, pathName ) ) {
    tsmError (funcName, "URL '%s' not found!", url);
    return (NULL);
  }
  TSMTRACE ((TSMMASK|3, funcName, "Wrote file '%s'.", pathName));
  return (pathName);
}

#else

char *
tsmCopyUrlToTmpFile (char *url)
{
  fprintf (stderr, "tsmCopyUrlToTmpFile should never be called when HTTP is not defined.\n");
  exit(1);
  return (NULL);
}

#endif

/*
 */

/*
 * Synopsis:
 *   char *tsmNameFromPathname( char *pathname )
 *
 * Doc++ Description: *//** 
 *   Extract the filename from a path.
 *
 * This routine returns the last part of a pathname,
 * i.e. the filename or last directory name. If the pathname
 * ends with a '/', then this is ignored. The function will
 * always return a valid string - this will be empty if an
 * error occurred. This string should be freed when finished with.
 *
 * @param    pathname a filename string
 *
 * @return   the last part of the filename string, or "" if error occurred
 *
 * @author   Martin Reddy
 * @version  6 February 1998
 *
 * @see      tsmAddFilenameExtension
 *
 */

char *
tsmNameFromPathname( char *pathname )
{
  char *divider, *name;
  
  /* return an empty string if invalid path */

  if ( pathname == NULL || pathname[0] == '\0' )
    return tsmMemManagerStrdup("");
  
  /* Go to end of string, back one character if last char is a '/' */

  divider = pathname + strlen(pathname) - 1;
  if ( *divider == '/' ) divider--;
  
  /* trace back to the last '/' character, or the start of the pathname */

  while ( *divider != '/' && divider >= pathname )
    divider--;

  /* take a copy from the last '/' to the end of the pathname */

  name = tsmMemManagerStrdup( ++divider );

  /* remove any trailing '/' character */

  if ( name[strlen(name)-1] == '/' )
    name[strlen(name)-1] = '\0';

  return name;
}

/*
 * Calculate the determinant of a 2x2 matrix - | a  b |
 * Code adapted from Gfx Gems I (p769)         | c  d |
 */

static double 
tsmDet2x2( float a, float b, float c, float d )
{
  return a * d - b * c;
}

/*
 * Calculate the determinant of a 3x3 matrix - | a1 a2 a3 |
 * Code adapted from Gfx Gems I (p769)         | b1 b2 b3 |
 *                                             | c1 c2 c3 |
 */

static double
tsmDet3x3( float a1, float a2, float a3,
	   float b1, float b2, float b3,
	   float c1, float c2, float c3 )
{
  return a1 * tsmDet2x2( b2, b3, c2, c3 ) -
         b1 * tsmDet2x2( a2, a3, c2, c3 ) +
         c1 * tsmDet2x2( a2, a3, b2, b3 );
}

/*
 * calculate the adjoint of a 3x3 mtrix. The input and output matrices
 * are given as a single-dim array of 9 floats, e.g.
 *
 *   | in[0] in[1] in[2] |
 *   | in[3] in[4] in[5] |
 *   | in[6] in[7] in[8] |
 *
 * If a_ij represents the entry at (i,j) position (where a_00 = [0]), 
 * the adjoint B is found by b_ij = (-1)^(i+j) * a_ji. 
 * Code was adapted from the 4x4 example given in Gfx Gems I, p 767,
 */

static void
tsmAdjoint3x3( float *in, float *out )
{
  float a1 = in[0], a2 = in[1], a3 = in[2];
  float b1 = in[3], b2 = in[4], b3 = in[5];
  float c1 = in[6], c2 = in[7], c3 = in[8];
  
  out[0] =  tsmDet2x2( b2, b3,  c2, c3 );
  out[1] = -tsmDet2x2( a2, a3,  c2, c3 );
  out[2] =  tsmDet2x2( a2, a3,  b2, b3 );

  out[3] = -tsmDet2x2( b1, b3,  c1, c3 );
  out[4] =  tsmDet2x2( a1, a3,  c1, c3 );
  out[5] = -tsmDet2x2( a1, a3,  b1, b3 );

  out[6] =  tsmDet2x2( b1, b2,  c1, c2 );
  out[7] = -tsmDet2x2( a1, a2,  c1, c2 );
  out[8] =  tsmDet2x2( a1, a2,  b1, b2 );
}

/*
 * Calculate the inverse of a 3x3 matrix. The format of the in and out
 * matrices are as for adjoint3x3() above. The inverse is given by:
 *
 *      A^(-1) = ( 1 / det A ) * adjoint A
 *
 * The function will return a value of FALSE if the matrix does not have
 * an inverse, otherwise a value of TRUE is returned and the inverse is
 * written to the array out.
 *
 * Code based up the 4x4 example in Gfx Gems I, p 766.
 * See Foley & van Dam A.4.4 (p.1108 in 2nd ed.) for further details.
 *
 * N.B. The code in Gfx Gems is WRONG! In function inverse() on p. 766
 * the determinant is calculated from the adjoint (out) not the original
 * matrix (in). This has been corrected in the code below.
 *
 */

int
tsmInverse3x3( float *in, float *out )
{
  double det;
  int    i;

  /* calculate the adjoint and determinant of the 3x3 matrix */

  tsmAdjoint3x3( in, out );
  det = tsmDet3x3( in[0], in[1], in[2],
		   in[3], in[4], in[5],
		   in[6], in[7], in[8] );

  /* if the matrix is zero then there is no inverse */

  if ( fabs( det ) < 1e-30 ) {
    for ( i = 0; i < 9; i++ ) out[i] = 0;
    return FALSE;
  }

  /* otherwise, divide the adjoint by the determinant for the inverse */

  for ( i = 0; i < 9; i++ ) {
    out[i] /= det;
  }
  
  return TRUE;
}

/*
 * Synopsis:
 *      int tsmExtractTsmAndPyramidName( const char *, char *, char * )
 *
 * Doc++ Description: *//** 
 *      Extract the DSM and Pyramid name from a URL.
 *      
 *      Takes a filename (or URL) for a pyramid directory and splits
 *      this into two components: the base DSM pathname (i.e. the bit up
 *      to the "Pyramid" part of the filename), and
 *      the pyramid name (i.e. the bit after the "[Geo]Pyramid" part).
 *      e.g. "/home/magic/TileSet/Pyramids/yosemite" will copy the
 *      string "/home/magic/TileSet" to base, and "yosemite" to 
 *      pyramid. N.B. trailing '/'s are always stripped from the base
 *      and pyramid strings.
 *
 *      You can pass in a NULL base or a pyramid pointer and that string
 *      will simply not be generated, i.e. if you only want the base name
 *      then you can call with pyramid == NULL.
 *
 *      If you specify an x-iss URL, then the base string (used to pass
 *      to tsmOpen) will have this changed to x-dsm.
 *
 * @param    path the original URL to be parsed
 * @param    base the extracted DSM name buffer to write into (or NULL)
 * @param    pyramid the pyramid name buffer to write into (or NULL)
 *
 * @return   TRUE if non-empty-string base and pyramid names extracted
 *
 * @author   Martin Reddy
 * @version  21 July 1997
 *
 * @see      
 *
 */

int
tsmExtractTsmAndPyramidName( const char *path, char *base, char *pyramid )
{
  const char *funcName = "tsmExtractTsmAndPyramidName";
  Purl *pathPurl;
  char *pstart;
  char *pend;

  /* First we will convert the path to a full URL so that we will always */
  /* return an absolute pathname, even if "path" is relative.            */

  if ( ( pathPurl = tsmParseUrl( path ) ) == NULL ) {
    tsmError( funcName, "path is not a valid pathname or URL" );
    return FALSE;
  }

  /* Find the occurence of the text "Pyramid" in the pathname */

  if ( ( pstart = strstr( pathPurl->url, "Pyramid" ) ) == NULL ) {
    if ( base != NULL ) strcpy( base, pathPurl->url );
    if ( pyramid != NULL ) strcpy( pyramid, "" );
  } else {

    /* Found occurence of "Pyramid", now backtrack to '/', e.g. could   */
    /* have found "/GeoPyramid" or "/Pyramid". We drop the trailing '/' */

    while ( *(pstart-1) != '/' && pstart > pathPurl->url )
      pstart--;

    /* We have now found the end of the tsm base name, so copy it */

    if ( base != NULL ) {
      strcpy( base, "" );
      strncat( base, pathPurl->url, pstart - pathPurl->url - 1 );
    }

    /* Now find the start of the Pyramid name section, if present */
    
    pstart++;
    pend = pathPurl->url + strlen(pathPurl->url);
    while ( *(pstart-1) != '/' && pstart < pend )
      pstart++;

    /* and add anything after the /Pyramid section to the pyramid */
    /* string. We will ensure that any trailing '/' is not copied */

    if ( pyramid != NULL ) {
      strcpy( pyramid, "" );
      if ( pstart < pend ) {
	if ( *(pend-1) == '/' ) pend--;
	strncat( pyramid, pstart, pend - pstart );
      }
    }

  }

  /* if the pyramid URL uses the x-iss protocol, then we have to change */
  /* this to x-dsm for the tsm handle name                              */

  if ( pathPurl->protocol == tsmISS && base != NULL ) {
    memcpy( base, "x-dsm", 5 );
  }

  tsmFreePurl( pathPurl );

  return ( pyramid != NULL && pyramid[0] != '\0' );
}

/*
 * Synopsis:
 *      void tsmAddFilenameExtension( char* filename, const char* ext )
 *
 * Doc++ Description: *//** 
 *      Add an extension to a filename.
 *      
 *      will add an extension to a filename if not already present,
 *      or replace any existing extension if there is already one.
 *
 * @param    filename a buffer containing the filename, with space for the ext
 * @param    ext the filename extension to add to filename, e.g. ".gif"
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  30 June 1997
 *
 * @see      tsmCheckFilenameExtension
 *
 */

void
tsmAddFilenameExtension( char* filename, const char* ext )
{
  char *dot, *slash;

  if ( filename == NULL || ext == NULL ) return;

  /* Remove any trailing slash */

  if ( filename[strlen(filename)-1] == '/' )
    filename[strlen(filename)-1] = '\0';

  /* Find the position of the last dot and the last slash */

  dot   = (char *) strrchr( filename, '.' );
  slash = (char *) strrchr( filename, '/' );

  /* Only remove the current extension if the dot is after the last slash */
  /* else we would convert http://host.com/pathname to http://host.ext    */

  if ( dot != NULL && ( slash == NULL || slash < dot ) )
    *dot = '\0';

  strcat( filename, ext );
}

/*
 * Synopsis:
 *      int tsmCheckFilenameExtension( char const *filename, char const *ext )
 *
 * Doc++ Description: *//** 
 *      Checks if a file has a specific extension.
 *      
 *      This function will check to see if the extension part of a filename
 *      (the text after any last period character) is equal to the specified
 *      extension. The check is case insensitive, so ".GIF" will match ".gif".
 *
 * @param    filename a buffer containing the filename, with space for the ext
 * @param    ext the filename extension to check for, e.g. ".oi"
 *
 * @return   TRUE if the filename has the specified extension, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  30 June 1997
 *
 * @see      tsmAddFilenameExtension
 *
 */

int
tsmCheckFilenameExtension( char const *filename, char const *ext )
{
  char *dot;

  if ( filename == NULL || ext == NULL )
    return FALSE;

  /* Find the position of the last dot */

  if ( ( dot = (char *) strrchr( filename, '.' ) ) == NULL )
    return FALSE;

  /* is the extension equal to the specified one? */

  return ( strcasecmp( dot, ext ) == 0 );
}

/*
 * Synopsis:
 *   void tsmMakeDirIfNotExist( const char *dirname, int isDir )
 *      
 * Doc++ Description: *//** 
 *   Create a directory tree.
 *
 *      Takes a directory name and will create that directory if it does
 *      not already exist. This routine will create all of the intermediate
 *      directories necessary, e.g. if you ask to create "/a/b/c/d", but
 *      only "/a" exists, then this function will create "/a/b", then
 *      "/a/b/c", and then finally "/a/b/c/d".
 *
 *      The second parameter specifies whether the string in the first
 *      parameter contains a directory name only (TSM_PATH_ONLY), or
 *      whether it also contains a filename at the end (TSM_PATH_AND_FILE).
 *      This enables you to pass a full filename to the function and it
 *      will create all of the directory structure necessary for that
 *      file, i.e. if ( isDir==TSM_PATH_AND_FILE ), then the last part of
 *      dirname is ignored.
 *
 * @param    dirname the directory or full filename of the target 
 * @param    isDir   one of TSM_PATH_ONLY or TSM_PATH_AND_FILE, refering to
 *                   the dirname string
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  15 July 1997
 *
 * @see      
 *
 */

void
tsmMakeDirIfNotExist( const char *dirname, int isDir )
{
  PRFileInfo stat_struct;
  char   pathname[512];
  int    i, len;
  
  if ( PR_GetFileInfo ( dirname, &stat_struct ) != PR_SUCCESS ) {

    /* get the length of the dirname, without any trailing '/' */

    len = strlen(dirname);
    if ( dirname[len-1] == '/' )
      len--;

    /* Check each directory name in turn and create if necessary */

    for ( i = 0; i < len; i++ ) {
      pathname[i] = dirname[i];
      if ( ( dirname[i] == '/' && i > 0 ) || ( i == len-1 && isDir ) ) {
	pathname[i+1] = '\0';
	if ( PR_GetFileInfo ( pathname, &stat_struct ) != PR_SUCCESS ) {
	  PR_MkDir( pathname, 0755 );
	}
      }
    }

  }  
}

/*
 * Synopsis:
 *   int tsmCopyTile( TsmConnection, TsmConnection, int, int, int )
 *
 * Doc++ Description: *//** 
 *   Copy a single tile from one dataset to another.
 *
 *   This function will take an input connection and copy the specified
 *   tile (tx,tx,res) from this dataset to an output connection. The
 *   input connection should of course be a read connection (i.e. 
 *   opened with a "r" or "rw" parameter to tsmConnect), and the output
 *   connction should be a write connection.
 *
 *   Of course, the connections need not be on the same medium. So for
 *   example you could copy tiles from local disk to local disk, but
 *   also from a DPSS to local disk, etc.
 *
 *    N.B. you should ensure that the output connection has a valid tspec
 *    associated with it before using this function, and that it specifies
 *    a dataset size that can handle the tile coordinate from 
 *    the input pyramid. The best way to do this is to do a tsmCopyTspec
 *    or tsmCopyTspecField before calling this function.
 *
 * @param    in the input (read) connection to get the tile from
 * @param    out the output (write) connection to send the tile to
 * @param    tx  the x-coordinate of the tile to read from and write to
 * @param    ty  the y-coordinate of the tile to read from and write to
 * @param    level  the level of the tile to read from and write to
 *
 * @return   TRUE if all went peachy, FALSE if things went pear-shaped.
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmCopyLevelTiles, tsmCopyAllTiles,
 *           tsmCopyTspec, tsmCopyTspecField
 *
 */

int
tsmCopyTile( TsmConnection in, TsmConnection out, int tx, int ty, int level )
{
  static const char *funcName = "tsmCopyTile";
  uchar             *tileBuffer;

  /* read the specified tile from the input connection */

  if ( tsmAllocReadTile( in, tx, ty, level, &tileBuffer ) == FALSE ) {
    tsmError( funcName, "Couldn't read tile %d,%d level %d", tx, ty, level );
    return FALSE;
  }

  /* try to write tile to the same tile location in the output connection */

  if ( tsmWriteTile( out, tx, ty, level, tileBuffer ) == FALSE ) {
    tsmError( funcName, "Couldn't write tile %d,%d level %d", tx, ty, level );
    return FALSE;
  }

  /* And then free up the memory for the tile buffer */

  tsmFreeTileBuffer( tileBuffer );
  return TRUE;
}

/*
 * Synopsis:
 *    int tsmCopyLevelTiles( TsmConnection in, TsmConnection out, int level )
 *
 * Doc++ Description: *//** 
 *    Copies one pyramid level of tiles.
 *
 *    This function will take an input connection and an output connection.
 *    It will then proceed to copy every tile from the specified level of
 *    input connection to the same level of the output connection.
 *    The two connections can be for different media,
 *    e.g. DPSS and localdisk. If an error occurs copying any single tile
 *    then the function will exit immediately with a FALSE return code.
 *
 *    A level refers to a single resolution layer of a dataset. That is,
 *    each dataset contains a number of different resolutions of the same
 *    data, where each lower resolution being half that of its parent.
 *    If you simply want to copy the entire multi-resolution dataset, then
 *    you can use the tsmCopyAllTiles function.
 *
 *    N.B. you should ensure that the output connection has a valid tspec
 *    associated with it before using this function, and that it specifies
 *    a dataset size that contains the specified level number.
 *    The best way to do this is to do a tsmCopyTspec
 *    or tsmCopyTspecField before calling this function.
 *
 *    You can set the TSM_PROGRESS_METER setting for the input connection
 *    using tsmSet in order
 *    to decide whether you want this function to display a progress 
 *    meter as it goes along, or to be silent.
 *
 * @param    in the input (read) connection to get the tiles from
 * @param    out the output (write) connection to send the tiles to
 * @param    level the level number to be copied
 *
 * @return   TRUE if copy completed successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  9 March 1998
 *
 * @see      tsmCopyTile, tsmCopyAllTiles, tsmCopyTspec, tsmCopyTspecField
 *
 */

int 
tsmCopyLevelTiles( TsmConnection in, TsmConnection out, int level )
{
  static const char *funcName = "tsmCopyLevelTiles";
  SimpleTileSet     *tileset;
  int               tx, ty, percent;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName, "Entering" ));

  /* Do some sanity checks on the connections first of all */

  if ( ! tsmCheckConnectMode( funcName, in, TSM_READ_ONLY, TRUE ) ) {
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }
  
  if ( ! tsmCheckConnectMode( funcName, out, TSM_WRITE_ONLY, TRUE ) ) {
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  if ( level < in->tspec->minLevel || level > in->tspec->maxLevel ) {
    tsmError( funcName, "There is no such level for the input connection" );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  if ( level < out->tspec->minLevel || level > out->tspec->maxLevel ) {
    tsmError( funcName, "There is no such level for the output connection" );
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  /* Go through each tile of the input pyramid and copy it */

  TSMTRACE(( TSMMASK|3, funcName, "Copying level %d tiles", level ));

  tileset     = &in->tspec->tspecLevel.val[level];

  for ( ty = tileset->minTile[1]; ty <= tileset->maxTile[1]; ++ty ) {

    percent = ( tileset->maxTile[1] > 0 ) ? ty*100/tileset->maxTile[1] : 100;
    tsmProgressReport( in, "Copying Level %d Tiles (%d%%)\r", level, percent );

    for ( tx = tileset->minTile[0]; tx <= tileset->maxTile[0]; ++tx ) {

      if ( tsmCopyTile( in, out, tx, ty, level ) == FALSE ) {
	tsmProgressReport( in, "" );
	tsmError( funcName, "Failed level %d tile copy", level );
	TSMTRINC(( TSMMASK|3, -1 ));
	return FALSE;
      }

    }
  }

  /* All worked fine - excellent! */

  tsmProgressReport( in, "" );
  TSMTRACE(( TSMMASK|3, funcName, "Done." ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *    int tsmCopyAllTiles( TsmConnection in, TsmConnection out )
 *
 * Doc++ Description: *//** 
 *    Copies an entire pyramid of tiles.
 *
 *    This function will take an input connection and an output connection.
 *    It will then proceed to copy every tile from the input connection to
 *    the output connection. The two connections can be for different media,
 *    e.g. DPSS and localdisk. If an error occurs copying any single tile
 *    then the function will exit immediately with a FALSE return code.
 *
 *    N.B. you should ensure that the output connection has a valid tspec
 *    associated with it before using this function, and that it specifies
 *    a dataset size that can handle the full collection of tiles from 
 *    the input pyramid. The best way to do this is to do a tsmCopyTspec
 *    or tsmCopyTspecField before calling this function.
 *
 *    You can set the TSM_PROGRESS_METER setting using tsmSet in order
 *    to decide whether you want this function to display a progress 
 *    meter as it goes along, or to be silent.
 *
 * @param    in the input (read) connection to get the tiles from
 * @param    out the output (write) connection to send the tiles to
 *
 * @return   TRUE if copy completed successfully, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  4 March 1998
 *
 * @see      tsmCopyTile, tsmCopyTspec, tsmCopyTspecField
 *
 */

int 
tsmCopyAllTiles( TsmConnection in, TsmConnection out )
{
  static const char *funcName = "tsmCopyAllTiles";
  int               level;

  TSMTRINC(( TSMMASK|3, 1 ));
  TSMTRACE(( TSMMASK|3, funcName, "Entering" ));

  /* Do some sanity checks on the connections first of all */

  if ( ! tsmCheckConnectMode( funcName, in, TSM_READ_ONLY, TRUE ) ) {
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }
  
  if ( ! tsmCheckConnectMode( funcName, out, TSM_WRITE_ONLY, TRUE ) ) {
    TSMTRINC(( TSMMASK|3, -1 ));
    return FALSE;
  }

  /* Go through each level of the input pyramid and copy it */

  for ( level = in->tspec->minLevel; level <= in->tspec->maxLevel; ++level ) {
    if ( tsmCopyLevelTiles( in, out, level ) == FALSE ) {
      tsmError( funcName, "Failed dataset copy" );
      TSMTRINC(( TSMMASK|3, -1 ));
      return FALSE;
    }
  }

  /* All worked fine - excellent! */

  TSMTRACE(( TSMMASK|3, funcName, "Done." ));
  TSMTRINC(( TSMMASK|3, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmCopyTspec( TsmConnection in, TsmConnection out )
 *
 * Doc++ Description: *//** 
 *    Copy one connection's tspec to another.
 *
 *    This function will read the tspec field from the input
 *    connection into the tspec field of the output connection, and
 *    then write out this tspec field to disk (for the output connection).
 *
 *    i.e. this results in copying the tspec file from the input connection
 *    to the output connection, and also updates the output connection's
 *    tspec field so that it contains the same information.
 *
 * @param    in the input (read) connection to get the tspec file from
 * @param    out the output (write) connection to send the tspec file to
 *
 * @return   TRUE if all went well, FALSE if something went wrong
 *
 * @author   Martin Reddy
 * @version  4 February 1998
 *
 * @see      tsmCopyTspecField, tsmCopyTile, tsmCopyAllTiles
 *
 */

int 
tsmCopyTspec( TsmConnection in, TsmConnection out )
{
  static const char *funcName = "tsmCopyTspec";

  if ( tsmCopyTspecField( in, out ) == FALSE ) {
    tsmError( funcName, "Couldn't set the output connection's tspec field" );
    return FALSE;
  }

  if ( tsmWriteTspec( out ) == FALSE ) {
    tsmError( funcName, "Couldn't write the output connection's tspec file" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmPause( float interval )
 *
 * Doc++ Description: *//** 
 *    Pause for the specified number of seconds (resolution in milliseconds)
 *
 * @param    interval The amount of time to wait (seconds).
 *
 * @author   Lee Iverson
 * @version  June 1999
 *
 * @see      tsmPause_ms, tsmPause_us
 *
 */

void
tsmPause (float s)
{
  if (s > 0.0) {
    PRIntervalTime timeout = PR_MillisecondsToInterval (floor (s*1e3));
    PR_Poll (NULL, 0, timeout);
  }
}

/*
 * Synopsis:
 *   void tsmPause_ms( PRUint32 interval )
 *
 * Doc++ Description: *//** 
 *    Pause for the specified number of milliseconds.
 *
 * @param    interval The amount of time to wait (milliseconds).
 *
 * @author   Lee Iverson
 * @version  June 1999
 *
 * @see      tsmPause, tsmPause_us
 *
 */

void
tsmPause_ms (PRUint32 ms)
{
  PRIntervalTime timeout = PR_MillisecondsToInterval (ms);
  PR_Poll (NULL, 0, timeout);
}

/*
 * Synopsis:
 *   void tsmPause_us( PRUint32 interval )
 *
 * Doc++ Description: *//** 
 *    Pause for the specified number of microseconds.
 *
 * @param    interval The amount of time to wait (microseconds).
 *
 * @author   Lee Iverson
 * @version  June 1999
 *
 * @see      tsmPause, tsmPause_ms
 *
 */

void
tsmPause_us (PRUint32 us)
{
  PRIntervalTime timeout = PR_MicrosecondsToInterval (us);
  PR_Poll (NULL, 0, timeout);
}

#define EMPTY_STR       "<none>"


/************************************************************************/
int 
tsmCheckString(char **str_p)
{
  /*static const char funcName[] = "tsmCheckString()";*/

  if (*str_p == NULL) {
    *str_p = EMPTY_STR;
  }
  return(0);
}
/* end tsmCheckString */

/*
 * Synopsis:
 *   int MakePathAbsolute( char *pathname, char *absdir, int buffer_size )
 *
 * Doc++ Description: *//** 
 *   This function will check to see if the pathname is relative or
 *   absolute (an absolute filename begins with a slash or contains
 *   a colon). If the pathname is absolute, then the function returns
 *   with no operation. If the pathname is relative, then the absdir
 *   string is prepended to the pathname (assuming that the resulting
 *   string does not exceed the size of the pathname array, given by
 *   buffer_size).
 *
 * @param    pathname the buffer containing the filename
 * @param    absdir the absolute pathname to prepend to the pathname buffer
 * @param    buffer_size the size in bytes of the pathname buffer 
 *
 * @return   TRUE if pathname buffer was modified, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  7 January 2001
 *
 * @see      
 *
 */

int
tsmMakePathAbsolute( char *pathname, char const *absdir, int buffer_size )
{
  char *tmp, lastchar;

  /* we need to have a pathname and an absolute dir in order to modify */

  if ( pathname == NULL || absdir == NULL || absdir[0] == '\0' )
    return FALSE;
  
  /* is the pathname already absolute? */

  if ( pathname[0] == '/' || pathname[0] == '\\' || strchr( pathname, ':' ) )
    return FALSE;

  /* we need to prepend the absdir, but do we have the room? */

  if ( strlen(pathname) + strlen(absdir) + 2 > buffer_size )
    return FALSE;

  /* otherwise, we are go for launch... prepend the absdir to pathname */

  tmp = strdup( pathname );
  strcpy( pathname, absdir );
  lastchar = pathname[strlen(pathname)-1];
  if ( lastchar != '/' && lastchar != '\\' )
    strcat( pathname, "/" );
  strcat( pathname, tmp );
  free( tmp );

  return TRUE;
}

