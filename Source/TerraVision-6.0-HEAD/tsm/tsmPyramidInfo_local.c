/*
 * Name: tsmPyramidInfo_local.c
 *
 * Description: 
 *   Contains routines for compiling complete server information about
 *   datasets. These are the localdisk specific routines and are
 *   private to the tsmApi.
 *
 * Function List:
 *   PyramidInfo *tsmGetPyramidInfo_local( TsmHandle *, const char *, Purl * )
 *
 * Revision History:
 *   Created 12 Jan 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmPyramidInfo_local.c,v 1.8 2000/08/28 22:16:01 reddy Exp $
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

PyramidInfo *
tsmGetPyramidInfo_local( TsmHandle *tsmh, const char *pyramidName, Purl *basePurl)
{
  static char * funcName = "tsmGetPyramidInfo_local()";
  PyramidInfo *pyramidInfo;
  
  pyramidInfo = (PyramidInfo *)
     tsmMemManagerCalloc (1, sizeof (PyramidInfo));
  if(!pyramidInfo) {
    tsmError (funcName, "Unable to allocate pyramidInfo.");
    return NULL;
  }
  pyramidInfo->name = tsmMemManagerStrdup (pyramidName);
  pyramidInfo->numServerGroups = 1;
  pyramidInfo->serverGroup = (ServerGroup *)
     tsmMemManagerCalloc (1, sizeof (ServerGroup));
  if(!pyramidInfo->serverGroup) {
    tsmError (funcName, "Unable to allocate pyramidInfo->serverGroup.");
    return NULL;
  }

  pyramidInfo->serverGroup->setId = tsmNameToId (pyramidName, tsmh->pyramidUrls);
  pyramidInfo->serverGroup->numServers = 1;
  pyramidInfo->serverGroup->masterHostName = basePurl->hostName;
  pyramidInfo->serverGroup->serverHostNames = (char **)
     tsmMemManagerCalloc (1, sizeof (char *));
  if(!pyramidInfo->serverGroup->serverHostNames) {
    tsmError (funcName, "Unable to allocate pyramidInfo->serverGroup->serverHostNames");
    return NULL;
  }
  pyramidInfo->serverGroup->serverHostNames[0] = basePurl->hostName;
  
  return (pyramidInfo);
}

NameUrlIds *
tsmReadNameUrlIds( const char *url )
{
  const char *funcName = "tsmReadNameUrlIds";
  FILE       *fp;
  char       buffer1[1024], buffer2[1024], line[1024];
  int        i = 0, numItems;
  NameUrlIds *nameUrlIds;

  /* try to open the file (could be an http URL too) */

  if ( ( fp = tsmOpenStream( url, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open file: %s", url );
    return NULL;
  }

  /* allocate an empty database which we will read values into */
  
  if ( ( nameUrlIds = tsmMakeNameUrlIds(0) ) == NULL ) {
    tsmError( funcName, "Unable to allocate database array");
    return NULL;
  }

  /* now read each line and try to parse it into a "name" "url" pair */

  while ( ! feof( fp ) ) {

    line[0] = '\0';
    fgets( line, 1024, fp );

    /* lines with no quotes or lines beginning with a '#' are ignored. */
    /* previously, the PyramidUrls file was headed with a number - the */
    /* number of entries to read. Now we ignore this number and read   */
    /* all the valid entries in a file. The logic below will ignore a  */
    /* line with a number on it and so maintain backward compatibility */
    /* with old PyramidUrls files.                                     */
    
    if ( strchr( line, '\"' ) == NULL || line[0] == '#' || feof(fp) )
      continue;

    /* parse the current line into a name and URL string */

    buffer1[0] = buffer2[0] = '\0';
    numItems = sscanf( line, "\"%[^\"]\" \"%[^\"]\"", buffer1, buffer2 );

    if ( numItems == 0 ) {
      tsmError( funcName, "Ignoring entry: '%s'\n", line );
      line[0] = '\0';
      fgets( line, 1024, fp );
      continue;
    }

    /* add this entry to the NameUrlsIds database */

    if ( strcmp( buffer2, " " ) == 0 ) buffer2[0] = '\0';
    tsmAddNameUrlId( nameUrlIds, tsmMemStrdup(buffer1),
		     tsmMemStrdup(buffer2), i++ );
  }

  tsmCloseStream( fp );

  return nameUrlIds;
}

void
tsmWriteNameUrlIds (const char *pathName, NameUrlIds *nameUrlIds)
{
  const char funcName[] = "tsmWriteNameUrlIds";
  int        i;
  FILE       *fp;
  char const *location;
  char       *slash;
  char       source_dir[512], target_dir[512];
  Purl       *purl;

  /* make the directory structure for the file if not already exist */

  tsmMakeDirIfNotExist( pathName, TSM_PATH_AND_FILE );

  /* find the directory name of the file - i.e. lob of all after last '/' */

  strcpy( source_dir, pathName );
  if ( ( slash = (char *) strrchr( source_dir, '/' ) ) != NULL )
    *slash = '\0';

  /* write out the name/url pairs to the file */

  if ( ( fp = tspOpenForWrite (pathName, "") ) != NULL ) {
    fprintf (fp, "%d\n", nameUrlIds->numItems);
    for (i = 0; i < nameUrlIds->numItems; i++) {

      location = nameUrlIds->urls[i];

      /* if the location of the name is a file: url and resolves  */
      /* to the same directory as the file itself, then url = " " */

      if ( ( purl = tsmParseUrl( location ) ) != NULL ) {

	if ( purl->protocol == tsmLocalFile ) {
	  strcpy( target_dir, purl->uri );
	  if ( ( slash = (char *) strrchr( target_dir, '/' ) ) != NULL )
	    *slash = '\0';
	  
	  if ( strcmp( source_dir, target_dir ) == 0 )
	    location = " ";
	}

	tsmFreePurl( purl );
      }

      fprintf (fp, "\"%s\" \"%s\"\n", nameUrlIds->names[i], location );
    }
    tspCloseForWrite (fp);
  } else 
    tsmError( funcName, "could not write to %s", pathName );
}

