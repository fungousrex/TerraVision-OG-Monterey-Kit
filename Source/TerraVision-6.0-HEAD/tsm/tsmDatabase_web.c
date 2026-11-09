/*
 * Name: tsmDatabase_web.c
 *
 * Description:
 *   Contains routines for managing the dataset database file from
 *   data located on a remote http server.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Function List:
 *    NameUrlIds *tsmReadServer_web( Purl const *purl )
 *
 * Dependencies:
 *
 * Revision History:
 *    November 1999 - Initial version by Martin Reddy
 *
 *    $Id: tsmDatabase_web.c,v 1.3 2000/08/28 22:15:42 reddy Exp $
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

/*
 * Synopsis:
 *   int NameUrlIds *tsmReadServer_web( Purl const *url )
 *
 * Description:
 *   Read all TerraVision datasets in a localdisk directory.
 *
 *   This function accepts the name of a localdisk directory (can be a
 *   file: URL). It will then scan through that directory looking for
 *   all directories that have an extension of either ".oi", ".dem",
 *   ".model", or ".map". It will then return a NameUrlIds array with
 *   all such entries. N.B. this array could be zero length if no such
 *   entries exist in the directory.
 *
 * Returns: A list of available datasets, or NULL if couldn't read directory.
 *
 * Author: Martin Reddy
 *
 * Date: 3 November 1999
 *
 */

NameUrlIds *
tsmReadServer_web( Purl const *purl )
{
  const char  *funcName = "tsmReadServer_web";
  FILE        *handle;
  char        entry[512], name[512], dirname[512];
  char        curr, prev;
  NameUrlIds  *nameUrlIds;
  int         count = 0, matches, i;
  
  if ( purl == NULL )
    return NULL;

  /* take a copy of the pathname part of the URL */
  
  strcpy( dirname, purl->url );
  
  /* append a trailing slash if one is not given */

  if ( dirname[strlen(dirname)-1] != '/' )
    strcat( dirname, "/" );

  /* try to open the directory so we can scan through it */

  if ( ( handle = tsmOpenStream( dirname, "r" ) ) == NULL ) {
    tsmError( funcName, "Cannot open directory: %s", dirname );
    return NULL;
  }

  /* allocate an empty database which we will read values into */
  
  if ( ( nameUrlIds = tsmMakeNameUrlIds( 0 ) ) == NULL ) {
    tsmError( funcName, "Unable to allocate server database array" );
    tsmCloseStream( handle );
    return NULL;
  }

  /* now parse through the HTML looking for <A HREF="url"> tags */

  while ( ! feof( handle ) ) {

    /* only continue if we are inside an HTML tag */

    if ( ( curr = fgetc( handle ) ) != '<' ) continue;

    /* only continue if we are inside an <A ..> HTML tag */

    if ( toupper( fgetc( handle ) ) != 'A' ) continue;

    /* find the HREF attribute */

    matches = 0;
    prev    = 'A';
    curr    = toupper(fgetc( handle ));

    while ( matches < 4 && curr != '>' && ! feof( handle ) ) {
      if ( curr == 'H' && isspace( prev ) ) matches = 1;
      if ( curr == 'R' && prev == 'H' && matches == 1 ) matches = 2;
      if ( curr == 'E' && prev == 'R' && matches == 2 ) matches = 3;
      if ( curr == 'F' && prev == 'E' && matches == 3 ) matches = 4;
      if ( isspace( curr ) ) matches = 0;
      prev = curr;
      curr = toupper(fgetc( handle ));
    }

    if ( matches < 4 ) continue;

    /* okay, we have found the HREF attribute, now move up to the URL */

    do {
      curr = fgetc( handle );
    } while ( curr != '\"' && curr != '>' && ! feof( handle ) );

    if ( curr != '\"' ) continue;

    /* and then read in the URL up to the closing quote marks */

    i = 0;

    do {
      curr = fgetc( handle );
      entry[i++] = curr;
    } while ( curr != '\"' && curr != '>' && ! feof( handle ) );

    entry[--i] = '\0';

    /* remove any trailing slash */

    if ( entry[i-1] == '/' ) entry[i-1] = '\0';

    /* work out the full path name for this entry */

    strcpy( name, dirname );
    strcat( name, entry );

    /* does the entry have a valid filename extension? */

    if ( tsmCheckFilenameExtension( name, ".oi" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".dem" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".model" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".map" ) == FALSE )
      continue;
    
    /* okay, we should have a valid TerraVision dataset now */

    tsmAddNameUrlId( nameUrlIds, tsmMemStrdup( entry ),
		     tsmMemStrdup( name ), count++ );

    TSMTRACE(( TSMMASK|2, funcName, "Dataset: %s", name ));

  }

  tsmCloseStream( handle );
  
  return nameUrlIds;
}
