/*
 * Name: tsmTspec_web.c
 *
 * Description: 
 *   Contains routines for manipulating tspec files for http datasets.
 *   These functions are private to the tsmApi.
 *
 *   If USE_HTTP is not #defined, then the dummy functions at the end are
 *   defined.  These will print error messages on stderr and return the
 *   appropriate error code.
 *
 * Function List:
 *   Pyramid *tsmReadTspec_web( Purl *purl, TsmConnectionParams )
 *
 * Revision History:
 *   Created 12 Mar 1998 by Martin Reddy <reddy@ai.sri.com>
 *
 *   $Id: tsmTspec_web.c,v 1.16.4.2 2001/06/19 22:04:45 leei Exp $
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

#ifdef USE_HTTP

#include "tsmConnectionParamsInt.h"

/*
 * Synopsis:
 *   Pyramid *tsmReadTspec_web( Purl *purl, TsmConnectionParams )
 *
 * Description:
 *   Loads a pyramid tspec file over an http link and parses it into
 *   a Pyramid structure (allocated within the function). This filled
 *   structure is then returned by the function (or NULL if failed).
 *
 *   The purl parameter is an http URL for the pyramid.
 *
 * Returns:
 *
 * Author: Martin Reddy
 *
 * Date: 12 January 1998
 *
 */

Pyramid *
tsmReadTspec_web( Purl *purl, TsmConnectionParams params )
{
  /*static const char funcName[] = "tsmReadTspec_web";*/
  Pyramid           *pyramid;

  if ( tsmIsParamSet( params, TSM_PARAM_NEW_TSPEC ) == TRUE ) {

    /* If the client requested a particular tspec format, then use it */

    if ( params->new_tspec == TRUE ) {
      if ( ( pyramid = tsmReadTspecNEW_web( purl->url ) ) == NULL )
	return NULL;
    } else {
      if ( ( pyramid = tsmReadTspecOLD_web( purl->url ) ) == NULL )
	return NULL;
    }

  } else {

    /* If no preference, then try the new tspec file first, then */
    /* fallback to the old tspec files if this is unavailable    */

    if ( ( pyramid = tsmReadTspecNEW_web( purl->url ) ) == NULL ) {
      if ( ( pyramid = tsmReadTspecOLD_web( purl->url ) ) == NULL )
	return NULL;
    }

  }

  /* okay, we've done with the loading in of the tspec information  */
  /* Put the base Purls into the pyramid's Purl slots.  By default, */
  /* the tile base Purl is the same as the tspec base Purl.         */

  pyramid->tspecBasePurl = tsmCopyPurl( purl );
  pyramid->tileBasePurl = tsmCopyPurl( purl );
  pyramid->identityTspec.setId = tsmGenerateUniqueSetID();
    
  /* If a geoName was not specified, then try to make one up */

  tsmEnsureGeoNameSet( pyramid );

  return pyramid;
}

/*
 * Synopsis:
 *   Pyramid *tsmReadTspecNEW_web( char *baseUrl )
 *
 * Description:
 *   Reads in a new single-file tspec over an http link. This is done
 *   by copying the file from the remote http server to a temp file on
 *   localdisk. Then we just use the localdisk tsmReadTspec routine to
 *   parse the file. 
 *
 *   The function will check the file that it downloads to make sure 
 *   that the file exists on the server - if it doesn't exist, we still
 *   get a file, but if contains an http error message page, so we need
 *   to make sure that this is not the case.
 *
 * Returns: An allocated Pyramid structure, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 17 March 1998
 *
 */

Pyramid *
tsmReadTspecNEW_web( char *baseUrl )
{
  static const char *funcName = "tsmReadTspecNEW_web";
  Pyramid           *pyramid;
  char              tspecUrl[1024];
  FILE              *handle;

  tsmAppendRelativePathNameToUrl( "tiles.tspec", baseUrl, tspecUrl );

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Reading NEW tspec file: %s", tspecUrl ));

  /* open the http file for reading */

  if ( ( handle = tsmOpenStream( tspecUrl, "r" ) ) == NULL ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* this is the new code to read the single-tspec format */

  if ( ( pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a new Pyramid structure" );
    tsmCloseStream( handle );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* set the pathName for compatibility with old API */

  pyramid->pathName = tsmMemStrdup( baseUrl );

  /* read in the tspec data over the http stream */

  if ( tspReadTspecStream( handle, pyramid ) == FALSE ) {
    tsmError( funcName, "Couldn't read tspec file over http" );
    tsmFreeTspec( pyramid );
    tsmCloseStream( handle );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* all done! */

  tsmCloseStream( handle );
  TSMTRACE(( TSMMASK|2, funcName, "Read NEW tspec file okay" ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return pyramid;
}

/*
 * Synopsis:
 *  Pyramid *tsmReadTspecOLD_web( char *baseUrl )
 *
 * Description:
 *
 * Returns: An allocated Pyramid structure, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 17 March 1998
 *
 */

Pyramid *
tsmReadTspecOLD_web( char *baseUrl )
{
  static const char *funcName = "tsmReadTspecOLD_web";
  int               i;
  char              tmpBuffer[1024], levelUrl[1024];
  Pyramid           *pyramid;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName,
	     "Reading OLD tspec files from: %s", baseUrl ));

  if ( ( pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "Couldn't allocate a new Pyramid structure" );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* this is the old code to read in the loads-of-tspecs format */

  if ( tsmReadPyramidTspec_web( baseUrl, pyramid ) == FALSE ) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
  
  if ( tsmReadIdentityTspec_web( baseUrl, &pyramid->identityTspec ) == FALSE) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
    
  if ( tsmReadCoordTspec_web( baseUrl, &pyramid->coordTspec ) == FALSE ) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
  
  if (tsmReadProjectionTspec_web(baseUrl,&pyramid->projectionTspec) == FALSE){
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  if ( tsmReadTileTspec_web( baseUrl, &pyramid->tileTspec ) == FALSE ) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
    
  if ( tsmReadPixelTspec_web( baseUrl, &pyramid->pixelTspec ) == FALSE ) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }

  /* CALCULATE THE TILE SIZE BASED ON THE PIXEL INFORMATION */

  pyramid->tileSize = pyramid->tileTspec.totalPixels[0] * 
    pyramid->tileTspec.totalPixels[1] * 
    pyramid->tileTspec.numComponentsPixel * 
    pyramid->tileTspec.numBytesPerComponent;

  /* ALLOCATE THE TILE SET LEVELS */

  if ( tsmMakeSimpleTileSetArray( &pyramid->tspecLevel, pyramid->minLevel,
				  pyramid->maxLevel ) == FALSE ) {
    tsmMemFree( pyramid );
    TSMTRINC(( TSMMASK|2, -1 ));
    return NULL;
  }
    
  /* READ THE TSPEC FILES THAT CORRESPOND TO EACH LEVEL */
    
  for ( i = pyramid->minLevel; i <= pyramid->maxLevel; i++ ) {

    sprintf( tmpBuffer, "%d", i );
    tsmAppendRelativePathNameToUrl( tmpBuffer, baseUrl, levelUrl );

    if( ! tsmReadSimpleTileSet_web( levelUrl, &pyramid->tspecLevel.val[i] ) ) {
      tsmMemFree( pyramid );
      tspError( levelUrl, "tsmReadTspec: failed to read simple tile set!" );
      return NULL;
    }
  }

  pyramid->type = pyramid->identityTspec.type;
  TSMTRACE(( TSMMASK|2, funcName, "Read OLD tspec files okay" ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return pyramid;
}

/* ================================================================
   The following are "wrapper" functions for reading tspec files over
   the web.  They copy each tspec file into a tmp directory, then use
   the tsp...  functions to read the copy.
   ================================================================ */

int
tsmReadSimpleTileSet_web (const char *baseUrl, SimpleTileSet *simpleTileSet)
{
  if (tsmReadSimpleTspec_web (baseUrl, simpleTileSet) == FALSE)
    return FALSE;
  if (tsmReadIdentityTspec_web (baseUrl, &simpleTileSet->identityTspec) == FALSE)
    return tspError (baseUrl, "tsmReadIdentityTspec_web failed!");
  if (tsmReadCoordTspec_web (baseUrl, &simpleTileSet->coordTspec) == FALSE)
    return FALSE;
  if (tsmReadProjectionTspec_web (baseUrl, &simpleTileSet->projectionTspec) == FALSE)
    return FALSE;
  if (tsmReadTileTspec_web (baseUrl, &simpleTileSet->tileTspec) == FALSE)
    return FALSE;
  if (tsmReadPixelTspec_web (baseUrl, &simpleTileSet->pixelTspec) == FALSE)
    return FALSE;
  if (tsmReadImageTransformationTspec_web (baseUrl,
					   &simpleTileSet->imageTransformationTspec) == FALSE)
    return FALSE;
  return TRUE;
}

int
tsmReadCoordTspec_web (const char *baseUrl, CoordTspec *coordTspec)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("3d-coordinate-system.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadCoordTspec1 (tmpPathName, coordTspec);
  unlink (tmpPathName);
  if (returnCode == FALSE)
    tspError (baseUrl, "tsmReadCoordTspec_web failed!");
  return (returnCode);
}

int
tsmReadIdentityTspec_web (const char *baseUrl, IdentityTspec *identityTspec)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("identity.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadIdentityTspec1 (tmpPathName, identityTspec);
  unlink (tmpPathName);
  if (returnCode == FALSE) 
    tspError (baseUrl, "tsmReadIdentityTspec_web failed!");
  return (returnCode);
}

int
tsmReadImageTransformationTspec_web (const char *baseUrl, ImageTransformationTspec *imageTransform)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("image-transformation.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadImageTransformationTspec1 (tmpPathName, imageTransform);
  unlink (tmpPathName);
  if (returnCode == FALSE) 
    tspError (baseUrl, "tsmReadImageTransformationTspec failed!");
  return (returnCode);
}

int
tsmReadPixelTspec_web (const char *baseUrl, PixelTspec *pixelTspec)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("pixel.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadPixelTspec1 (tmpPathName, pixelTspec);
  unlink (tmpPathName);
  if (returnCode == FALSE)
    tspError (baseUrl, "tsmReadPixelTspec failed!");
  return (returnCode);
}

int
tsmReadProjectionTspec_web (const char *baseUrl, ProjectionTspec *projectionTspec)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("projection.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadProjectionTspec1 (tmpPathName, projectionTspec);
  unlink (tmpPathName);
  if (returnCode == FALSE)
    tspError (baseUrl, "tsmReadProjectionTspec failed!");
  return (returnCode);
}

int
tsmReadPyramidTspec_web (const char *baseUrl, Pyramid *pyramid)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("tile-set.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadPyramidTspec1 (tmpPathName, pyramid);
  if (returnCode == FALSE) tspError (baseUrl, "tsmReadPyramidTspec_web failed!");
  return (returnCode);
}

int
tsmReadSimpleTspec_web (const char *baseUrl, SimpleTileSet *simpleTileSet)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("tile-set.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadSimpleTspec1 (tmpPathName, simpleTileSet);
  unlink (tmpPathName);
  if (returnCode == FALSE)
    tspError (baseUrl, "tsmReadSimpleTspec_web failed!");
  return (returnCode);
}

int
tsmReadTileTspec_web (const char *baseUrl, TileTspec *tileTspec)
{
  char tspecUrl[1024], *tmpPathName;
  int returnCode;

  tsmAppendRelativePathNameToUrl ("tile.tspec", baseUrl, tspecUrl);

  if ( ( tmpPathName = tsmCopyUrlToTmpFile( tspecUrl ) ) == NULL )
    return FALSE;

  returnCode = tspReadTileTspec1 (tmpPathName, tileTspec);
  unlink (tmpPathName);
  if (returnCode == FALSE)
    tspError (baseUrl, "tsmReadTileTspec failed!");
  return (returnCode);
}

#else

Pyramid *
tsmReadTspec_web( Purl *purl, TsmConnectionParams params )
{
  tsmNotCompiledIn( purl->protocol, "tsmReadTspec_web" );
  return NULL;
}

#endif
