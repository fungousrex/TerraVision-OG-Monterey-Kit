/*
 * Name: TsTileSpecIn.c
 *
 * Description: Code for reading tspec (Tile Specification) files. The
 *      main data structures are mapped pretty directly to the tspec files.
 *      (See AIC Technical Note 541, "Tile Set Definitions" for details).
 *
 * 	WARNING: The current parsing algorithm assumes that entries in the
 *               tspec files are in a pre-specified order.  A more powerful
 *               parser should be used!
 *
 *      WARNING: All of this code assumes that all lines from tspec files
 *               are fewer than 1024 characters.
 *
 *      WARNING: Though a reasonable amount of verification and cross-checking
 *               is done while reading the tspec files, there are likely to be
 *               certain types of errors that will not be caught using the
 *               parsing approach used here.
 *
 * Public Functions:
 *
 *      tspReadTileSetType () - This function reads the "tile-set-type"
 *                              keyword to determine the type of the tile
 *                              set residing in a given directory.
 *
 *	tspReadPyramid() - Read a Pyramid Tile Set.
 *
 *	tspReadSimpleTileSet() - Read a Simple Tile Set. 
 *			
 * Private Functions:
 *
 *      tspReadPyramidTspec () - Read the "tile-set.tspec" file for 
 *                                           a Pyramid Tile Set.
 *
 *      tspReadSimpleTspec () - Read the "tile-set.tspec" file for 
 *                                          a Simple Tile Set.
 *
 * 	tspReadIdentityTspec() - Reads the "identity.tspec" file.
 *
 *      tspReadTileTspec()	- Reads the "tile.tspec" files for each level.
 *
 *      tspReadPixelTspec() - Reads the "pixel.tspec" files for each level.
 *
 *	tspReadImageTransformationTspec() - Reads the
 *                              "image-transformation.tspec" files.
 *
 *	tspReadCoordTspec() - Reads the "3d-coordinate-system.tspec" file.
 *
 *	tspReadProjectionTspec() - Reads the "projection.tspec" file.
 *
 *      tspError () - Utility function to print error message and return FALSE.
 *
 *      tspErrorAndClose () - Utility function to print an error message,
 *                            close a file, and return FALSE.
 *
 * Dependencies:
 *
 * Revision History:  September 1993
 *                    October 1994: YGL, major revision, including full error
 *                                  checking and the ability to read all
 *                                  possible tspec types.
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
 * Synopsis: int tspReadTileSetType (char * pathName)
 *
 * Description:
 * 	Read the "tile-set-type" keyword in either the file pointed by the
 *      pathName, or the file "tile-set.tspec" file in
 *      directory "pathName".  Returns the type of tile set (an enum tspSymbol)
 *      if a valid tile set type can be determined, or -1 if either the file cannot
 *      be opened or the tile-set-type is invalid.
 *
 * Externals: NONE
 *
 *
 * Returns: enum tspSymbol on success
 *          -1 on failure
 *
 * Author:
 *		Yvan Leclerc
 * 
 * Date:
 *    September 1994
 *
 */

int tspReadTileSetType (char * pathName) 
{
  FILE * fp; 
  char buffer[1024]; 
  int tileSetType;
  
  sprintf(buffer,"%s/tile-set.tspec", pathName);
  fp = fopen(buffer,"rb");
  if (!fp) {
    fp = fopen (pathName, "rb");
    if (!fp) {
      tspError (pathName, "While reading (unknown) tile-set.tspec: couldn't open file!");
      return -1;
    }
  }
  
  tileSetType = tspReadSymbol (fp, "tile-set-type %s\n", 2, tspTileSetTypeStart+1, tspTileSetTypeEnd-1);
  if (tileSetType == tspSymbolError) {
    tspError (pathName, "While reading (unknown) tile-set.tspec: invalid tile-set-type!");
    return -1;
  }
  return tileSetType;
}


int
tspReadPyramids (int numPyramids, char **pyramidNames, NameUrlPairs *pyramidUrls, Pyramid *pyramids)
{
  char buffer[1024];
  int i, pyramidIndex, status;

  for (i = 0; i < numPyramids; ++i) {
    pyramidIndex = tspMatchName (pyramidNames[i], pyramidUrls);
    if (pyramidIndex < 0) {
      fprintf (stderr, "ERROR: failed to match pyramid name \"%s\"", pyramidNames[i]);
      return FALSE;
    }
    status = tspUrlToPathName (pyramidUrls->urls.val[pyramidIndex], buffer);
    if (status == TRUE) {
      status = tspReadPyramid (buffer, &pyramids[i], TRUE);
      if (status == FALSE)
	return tspError (buffer, "Couldn't read pyramid!");
    }
    else 
      return tspError (buffer, "Couldn't match URL!");

  }
  return TRUE;
}

/*
 * Synopsis: int tspReadPyramid (char *pathName,
 *                               Pyramid *tspec,
 * 				 int readChildrenP)
 *
 * Description:
 * 	Read in a set of tspec files that are pointed to by a pathname. Provides a nice
 *    	front end of the end user. All they have to do is call this function with a 
 *    	pointer to the pathname and a pointer to PRE-ALLOCATED space pointing to a 
 *      Tspec structure. The structure is filled with the data and all the
 *    	corresponding files are then closed.
 *  
 *      What the user does with the data afterwards is up the them.
 *
 * Externals: NONE
 *
 *
 * Returns: TRUE - on success, tspec points to structure containing tspec data.
 *          FALSE - on failure, tspec data is undefined.
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *    September 27, 1993
 *    September, 1994: YGL
 *
 */

int
tspReadOldPyramid (char *pathName, Pyramid *tspec, int readChildrenP)
{
  static char *funcName = "tspReadOldPyramid()";
  char        levelPathName[1024];
  int         i;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Read OLD tspecs from: %s", pathName ));

  if ( tspec == NULL ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError( pathName, "NULL tspec give to tspReadOldPyramid()" );
  }

  if ( tspReadPyramidTspec( pathName, tspec ) == FALSE ) {
    tsmError( funcName, "Couldn't open old tspec (%s)", pathName );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  if ( tspReadIdentityTspec(pathName, &tspec->identityTspec) == FALSE ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError (pathName, "tspReadIdentityTspec failed!");
  }

  if ( tspReadCoordTspec(pathName, &tspec->coordTspec) == FALSE ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError (pathName, "tspReadCoordTspec failed!");
  }

  if( tspReadProjectionTspec(pathName, &tspec->projectionTspec) == FALSE ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError (pathName, "tspReadProjectionTspec failed!");
  }

  if( tspReadTileTspec(pathName, &tspec->tileTspec) == FALSE ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError (pathName, "tspReadTileTspec failed!");
  }

  if( tspReadPixelTspec(pathName, &tspec->pixelTspec) == FALSE ) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return tspError (pathName, "tspReadPixelTspec failed!");
  }

  /* Put the pathname in the tspec */
  
  if ( ( tspec->pathName = tsmMemManagerStrdup( pathName ) ) == NULL ) {
    tsmError (funcName, "Error in allocating tspec->pathName");
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* CALCULATE THE TILE SIZE BASED ON THE PIXEL INFORMATION */
  tspec->tileSize = tspec->tileTspec.totalPixels[0] * 
    tspec->tileTspec.totalPixels[1] * 
      tspec->tileTspec.numComponentsPixel * 
	tspec->tileTspec.numBytesPerComponent;

  /* CREATE EMPTY ARRAYS */

  if (! tsmMakeChecksum (&tspec->identityTspec.checkSumLrbt,
			 tspec->minLevel, tspec->maxLevel)
      || ! tsmMakeChecksum (&tspec->identityTspec.checkSumJpeg,
			    tspec->minLevel, tspec->maxLevel)) {
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  if (readChildrenP == TRUE) {
    /* ALLOCATE SPACE FOR ALL OF THE LEVELS OF RESOLUTION */
    if (! tsmOffsetArrayCallocRange (SimpleTileSet, tspec->tspecLevel,
				     tspec->minLevel, tspec->maxLevel)) {
      tsmError (funcName, "Error in allocating tspec->tspecLevel");
      TSMTRINC(( TSMMASK|2, -1 ));
      return FALSE;
    }
    
    /* READ THE TSPEC FILES THAT CORRESPOND TO EACH LEVEL */
    for( i = tspec->minLevel; i <= tspec->maxLevel; i++) {
      sprintf (levelPathName, "%s/%d", pathName, i);
      if(!tspReadSimpleTileSet (levelPathName, &tspec->tspecLevel.val[i])) {
	TSMTRINC(( TSMMASK|2, -1 ));
	return tspError( levelPathName, "tspReadOldPyramid: failed reading a simple tile set!");
      }
    }
  } else {
    tspec->tspecLevel.off = 0;
    tspec->tspecLevel.len = 0;
    tspec->tspecLevel.val = NULL;
  }

  tspec->type = tspec->identityTspec.type;
  TSMTRACE(( TSMMASK|2, funcName, "Read OLD tspec files okay" ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

/*
 * Synopsis: tspReadSimpleTileSet (char *, int, SimpleTileSet *)
 *
 * Description: Reads the tspec files for a Simple Tile Set. 
 *
 * Externals: NONE
 *
 * Returns: FALSE - on failure
 *	    TRUE  - on success.
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *    September 27, 1993
 */
int
tspReadSimpleTileSet (char *pathName, SimpleTileSet * tspec)
{
  if (tspReadSimpleTspec (pathName, tspec) == FALSE)
    return FALSE;
  if (tspReadIdentityTspec(pathName, &tspec->identityTspec) == FALSE)
    return tspError (pathName, "tspReadIdentityTspec failed!");
  if (tspReadCoordTspec(pathName, &tspec->coordTspec) == FALSE)
    return FALSE;
  if (tspReadProjectionTspec(pathName, &tspec->projectionTspec) == FALSE)
    return FALSE;
  if (tspReadTileTspec(pathName, &tspec->tileTspec) == FALSE)
    return FALSE;
  if (tspReadPixelTspec(pathName, &tspec->pixelTspec) == FALSE)
    return FALSE;
  if (tspReadImageTransformationTspec(pathName, &tspec->imageTransformationTspec) == FALSE)
    return FALSE;

  return TRUE;
}

int 
tspReadPyramidTspec (char *basePathName, Pyramid *tspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/tile-set.tspec", basePathName);
  return (tspReadPyramidTspec1 (pathName, tspec));
}

int
tspReadPyramidTspec1 (char *pathName, Pyramid *tspec)
{
  static char * funcName = "tspReadPyramidTspec1()";
  FILE * fp;
  char buffer[1024];
  int numItems;
  tspSymbol type;
  
  fp = fopen (pathName, "r");
  if (!fp)
    return tspError (pathName, "While reading pyramid tile-set.tspec: couldn't open file!");
  
  type = tspReadSymbol (fp, "tile-set-type %s\n", 2, tspPyramidTileSetType, tspPyramidTileSetType);
  if (type == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid tile-set-type!", fp);
  
  numItems = fscanf(fp, "inverse-resolution-scale-factor %f\n",
		    &tspec->invResScaleFactor);
  if (numItems!=1) 
    return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid inverse-resolution-scale-factor!", fp);
  
  numItems = fscanf(fp, "min-valid-level-and-resolution (%d %f)\n",&tspec->minLevel, &tspec->minResolution);
  if (numItems!=2) 
    return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid min-valid-level-and-resolution!", fp);
  
  numItems = fscanf(fp, "max-valid-level-and-resolution (%d %f)\n",&tspec->maxLevel, &tspec->maxResolution);
  if (numItems!=2)
    return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid max-valid-level-and-resolution!", fp);
  
  numItems = fscanf (fp, "%s", buffer);
  if (numItems != 1)
    return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: no entry after max-valid-level-and-resolution!", fp);
  
  if (strcmp (buffer, "image-hierarchy-min-valid-level") == 0) {
    
    numItems = fscanf(fp, "%d\n", &tspec->imageHierarchyMinValidLevel);
    if (numItems != 1)
      return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid image-hierarchy-min-valid-level!", fp);
    
    numItems = fscanf(fp, "image-hierarchy-max-valid-level %d\n", &tspec->imageHierarchyMaxValidLevel);
    if (numItems != 1)
      return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid image-hierarchy-max-valid-level!", fp);
    
    numItems = fscanf(fp, "image-hierarchy-file-name \"%[^\"]\"\n", buffer);
    if (numItems != 1)
      return tspErrorAndClose (pathName, "While reading pyramid tile-set.tspec: invalid image-hierarchy-file-name!", fp);
    tspec->imageHierarchyFileName = (char *)
       tsmMemManagerCalloc (1, (strlen(buffer)+1) * sizeof(char));
    if(!tspec->imageHierarchyFileName)
    {
	tsmError (funcName, "Error in allocating imageHierarchyFileName.");
	return FALSE;
    }
    strcpy (tspec->imageHierarchyFileName, buffer);
    
    fscanf (fp, "%s", buffer);
  }
  else /* If image-hierarch-min-valid-level is not defined, set the slot value to -1 */
    tspec->imageHierarchyMinValidLevel = -1;

  numItems = fscanf(fp, "sky %d", &tspec->sky);
  numItems = fscanf(fp, "two-sided %d", &tspec->twoSided);
  numItems = fscanf(fp, "view-from-below %d", &tspec->fromBelow);

  tspec->description = NULL;
  if (numItems == 1 && strcmp (buffer, "tile-set-description") == 0) {
    numItems = fscanf(fp, " \"%[^\"]\n", buffer);
    if (numItems == 1) {
      tspec->description = (char *)
	 tsmMemManagerCalloc (1, (strlen(buffer)+1) * sizeof(char));
      if(!tspec->description)
      {
	  tsmError (funcName, "Unable to allocate tspec->description.");
	  return FALSE;
      }
      strcpy (tspec->description, buffer);
    }
    else
      fprintf (stderr, "WARNING, invalid description while reading %s\n", pathName);
  }
 fclose(fp);
 return TRUE;
}

int 
tspReadSimpleTspec (char *basePathName, SimpleTileSet *tspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/tile-set.tspec", basePathName);
  return (tspReadSimpleTspec1 (pathName, tspec));
}

int
tspReadSimpleTspec1 (char *pathName, SimpleTileSet *tspec)
{
  static char * funcName = "tspReadSimpleTspec1()";
  char buffer[1024];
  FILE * fp;
  int numItems;
  tspSymbol type;

  fp = fopen (pathName, "r");
  if (!fp)
    return tspError (pathName, "While reading simple tile-set.tspec: couldn't open file!");

  type = tspReadSymbol (fp, "tile-set-type %s\n", 2, tspSimpleTileSetType, tspSimpleTileSetType);
  if (type == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid tile-set-type!", fp);
  
  numItems = fscanf(fp,"min-tile-tx %d\n min-tile-ty %d\n",
		    &tspec->minTile[0],&tspec->minTile[1]);
  if (numItems!=2)
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid min-tile-tx or min-tile-ty!", fp);
  TSMTRACE ((TSMMASK|2, funcName, "level %d minTile[X] = %d minTile[Y] = %d",
	     tspec->minTile[0], tspec->minTile[1]));
  
  numItems = fscanf(fp,"max-tile-tx %d\n max-tile-ty %d\n",
		    &tspec->maxTile[0], &tspec->maxTile[1]);
  if (numItems!=2 || 
      tspec->maxTile[0] < tspec->minTile[0] ||
      tspec->maxTile[1] < tspec->minTile[1])
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid max-tile-tx or max-tile-ty!", fp);
  TSMTRACE ((TSMMASK|2, funcName, "level %d maxTile[X] = %d maxTile[Y] = %d",
	     tspec->maxTile[0], tspec->maxTile[1]));
  
  numItems = fscanf(fp,"num-tiles-tx %d\n num-tiles-ty %d\n",
		    &tspec->numTile[0], &tspec->numTile[1]);
  if (numItems!=2 ||
      tspec->numTile[0] != (tspec->maxTile[0] - tspec->minTile[0] + 1) ||
      tspec->numTile[1] != (tspec->maxTile[1] - tspec->minTile[1] + 1))
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid num-tiles-tx or num-tiles-ty!", fp);
  
  tspec->tileMapStatus = tspReadSymbol (fp, "tile-map %s\n", 2, tspTileMapStatusStart+1, tspTileMapStatusEnd-1);
  if (tspec->tileMapStatus == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid tile-map!", fp);
 
  numItems = fscanf(fp,"num-pixels-ix %d\n num-pixels-iy %d\n",
		    &tspec->numPixels[0], &tspec->numPixels[1]);
  if (numItems!=2)
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid num-pixels-ix or num-pixels-iy!", fp);
  
  numItems = fscanf(fp,"min-valid-pixel-ix %d\n min-valid-pixel-iy %d\n",
		    &tspec->minValidPixel[0], &tspec->minValidPixel[1]);
  if (numItems!=2)
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid min-valid-pixel-ix or min-valid-pixel-iy!", fp);
  
  numItems = fscanf(fp,"max-valid-pixel-ix %d\n max-valid-pixel-iy %d\n",
		    &tspec->maxValidPixel[0], &tspec->maxValidPixel[1]);
  if (numItems!=2 ||
      tspec->maxValidPixel[0] < tspec->minValidPixel[0] ||
      tspec->maxValidPixel[1] < tspec->minValidPixel[1])
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid max-valid-pixel-ix or max-valid-pixel-iy!", fp);

  numItems = fscanf(fp,"num-valid-pixels-ix %d\n num-valid-pixels-iy %d\n",
		    &tspec->numValidPixel[0], &tspec->numValidPixel[1]);
  if (numItems!=2 ||
      tspec->numValidPixel[0] != (tspec->maxValidPixel[0] - tspec->minValidPixel[0] + 1) ||
      tspec->numValidPixel[1] != (tspec->maxValidPixel[1] - tspec->minValidPixel[1] + 1))
    return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid max-valid-pixel-ix or max-valid-pixel-iy!", fp);
  
  tspec->imageFileName = NULL;
  fscanf(fp,"%s", buffer);
  if (strcmp(buffer, "image-file-name") == 0) {
    numItems = fscanf (fp, " \"%[^\"]\"\n", buffer);
    if (numItems != 1)
      return tspErrorAndClose (pathName, "While reading simple tile-set.tspec: invalid image-file-name!", fp);
    tspec->imageFileName = (char *)
       tsmMemManagerCalloc (1, (strlen(buffer)+1) * sizeof(char));
    if(!tspec->imageFileName)
    {
	tsmError (funcName, "Error allocating tspec->imageFileName.");
	return FALSE;
    }
    strcpy (tspec->imageFileName, buffer);
    fscanf (fp, "%s", buffer);
  }

  tspec->description = NULL;
  if (numItems == 1 && strcmp (buffer, "tile-set-description") == 0) {
    numItems = fscanf(fp, " \"%[^\"]\n", buffer);
    if (numItems == 1) {
      tspec->description = (char *) 
	 tsmMemManagerCalloc (1, (strlen(buffer)+1) * sizeof(char));
      if(!tspec->description)
      {
	  tsmError (funcName, "Error allocating tspec->description.");
	  return FALSE;
      }
      strcpy (tspec->description, buffer);
    }
    else
      fprintf (stderr, "WARNING, invalid description while reading %s\n", pathName);
  }
  fclose(fp);
  return TRUE;
}

/*
 * Synopsis: int tspReadIdentityTspec(char *, IdentityTspec *)
 *
 * Description: This function reads the "identity.tspec" file associated with
 *      each simple or pyramid tile set.
 *
 *      This routine is much too long because a proper parser for lists of
 *      strings has not been written.
 *
 * Externals: NONE
 *
 * Returns:  TRUE - on success, tileTspec is filled with the data from the tspec file 
 * 	     FALSE - on failure
 *
 * Author:
 *		Yvan Leclerc
 * 
 * Date:
 *    October 10, 1994
 *
 */

int 
tspReadIdentityTspec (char *basePathName, IdentityTspec *tspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/identity.tspec", basePathName);
  return (tspReadIdentityTspec1 (pathName, tspec));
}

int 
tspReadIdentityTspec1 (char *pathName, IdentityTspec *tspec)
{
  static char * funcName = "tspReadIdentityTspec1()";
  FILE * fp;
  char buffer[4096];
  int numItems, readStatus;

  fp = fopen (pathName, "r");
  if (!fp) {
    fprintf (stderr, "WARNING, %s/identity.tspec file did not exist!!\n", pathName);
    tspec->name = "unknown";
    tspec->setId = -1;
    tspec->type = tspUnknownSymbol;
    tspec->subtype = tspUnknownSymbol;
    tspec->sources.len = 0;
    tspec->sources.val = NULL;
    tspec->operations.len = 0;
    tspec->operations.val = NULL;
    tspec->operationDate = "unknown";
    tspec->operationDoneBy = "unknown";
    return TRUE;
  }
/*
    return tspError (pathName, "While reading identity.tspec: couldn't open file!");
*/
  
  numItems = fscanf(fp, "name \"%[^\"]\"\n", buffer);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid name!", fp);
  tspec->name = tsmMemManagerStrdup (buffer);
  
  /* set-id is an optional field, whose default value is -1 */
  numItems = fscanf(fp, "%s", buffer);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: no entry after name!", fp);
  /*fprintf (stdout, "entry after name = %s\n", buffer);*/

  tspec->setId = -1;
  if (strcmp (buffer, "set-id") == 0) { /* set-id optional entry */
    numItems = fscanf(fp, "%d\n", &tspec->setId);
    /* fprintf (stderr, "set-id = %d\n", tspec->setId); */
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading identity.tspec: invalid set-id!", fp);
    numItems = fscanf (fp, "%s", buffer);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading identity.tspec: no entry after optional set-id!", fp);
    /* fprintf (stderr, "entry after set-id = %s\n", buffer); */
  }

  /* checksum is an optional field, whose default value is -1 */
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: no entry after optional set-id!", fp);
  /*fprintf (stderr, "entry after set-id = %s\n", buffer);*/

  tspec->checkSumLrbt.len = 0;
  tspec->checkSumLrbt.val = NULL;
  tspec->checkSumJpeg.len = 0;
  tspec->checkSumJpeg.val = NULL;
  /*
    if (strcmp (buffer, "checksum") == 0) {
    numItems = fscanf(fp, "%d\n", &tspec->checkSum);
    if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid checksum!", fp);
    numItems = fscanf (fp, "%s", buffer);
    if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: no entry after optional checksum!", fp);
  }
  */

  if (strcmp (buffer, "type") == 0) { /* type entry */
    tspec->type = tspReadSymbol (fp, "%s\n", 2, tspTileTypeStart+1, tspTileTypeEnd-1);
    if (tspec->type == tspSymbolError)
      return tspErrorAndClose (pathName, "While reading identity.tspec: invalid type!", fp);
  }
  else
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid entry after checksum!\n", fp);
  
  tspec->subtype = tspReadSymbol (fp, "subtype %s\n", 2, tspSubtypeStart+1, tspSubtypeEnd-1);
  if (tspec->subtype == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid subtype!", fp);

  /* Read the sources list of quoted strings */

  readStatus = tspReadListOfQuotedStrings (fp,
					   &tspec->sources.len,
					   &tspec->sources.val,
					   "sources", "\n");
  if (readStatus == FALSE)
    return tspErrorAndClose (pathName, "While reading identity.tspec: error in reading sources list!", fp);
    
  /* Read the list of operation symbols */

  readStatus = tspReadListOfSymbols (fp,
				     &tspec->operations.len,
				     &tspec->operations.val,
				     "operations", "\n", 2,
				     tspOperationStart+1, tspOperationEnd-1);
  if (readStatus == FALSE)
    return tspErrorAndClose (pathName, "While reading identity.tspec: error in reading operations list!", fp);

  numItems = fscanf(fp, "operation-date \"%[^\"]\"\n", buffer);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid operation-date!", fp);
  tspec->operationDate = (char *)
     tsmMemManagerCalloc (1, (strlen (buffer)+1) * sizeof (char));
  if(!tspec->operationDate)
  {
      tsmError (funcName, "Error allocating tspec->operationDate");
      return FALSE;
  }
  strcpy (tspec->operationDate, buffer);
  
  numItems = fscanf(fp, "operation-done-by \"%[^\"]\"\n", buffer);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading identity.tspec: invalid operation-done-by!", fp);
  tspec->operationDoneBy = (char *)
     tsmMemManagerCalloc (1,
			  (strlen (buffer)+1) * sizeof (char));
  if(!tspec->operationDoneBy)
  {
      tsmError (funcName, "Error allocating tspec->operationDoneBy");
      return FALSE;
  }
  strcpy (tspec->operationDoneBy, buffer);
  
  fclose(fp);
  return TRUE;
}

/*
 * Synopsis: int tspReadTileTspec(char *, TileTspec)
 *
 * Description: This function reads the "tile.tspec" files associated with each 
 * 	resolution level. Refer to the documentation about what each field is about.
 *      This function is hidden from the end user. Once again, the space must
 *      be pre-allocated.
 *
 * Externals: NONE
 *
 * Returns:  TRUE - on success, tileTspec is filled with the data from the tspec file 
 * 	     FALSE - on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *    September 27, 1993
 *
 */

int 
tspReadTileTspec (char *basePathName, TileTspec *tileTspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/tile.tspec", basePathName);
  return (tspReadTileTspec1 (pathName, tileTspec));
}

int 
tspReadTileTspec1 (char *pathName, TileTspec *tileTspec)
{
  FILE * fp;
  int numItems;

  fp = fopen (pathName, "rb");
  if (!fp)
    return tspError (pathName, "While reading tile.tspec: couldn't open file!");

  numItems = fscanf(fp, "num-pixels-per-tile-ix %d\n", &tileTspec->numPixelsX);
  if (numItems!=1 || tileTspec->numPixelsX < 1)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-pixels-per-tile-ix!", fp);
 
  numItems = fscanf(fp, "num-pixels-per-tile-iy %d\n", &tileTspec->numPixelsY);
  if (numItems!=1 || tileTspec->numPixelsY < 1)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-pixels-per-tile-iy!", fp);
 
  numItems = fscanf(fp, "num-pixels-overlap-per-tile-ix %d\n", &tileTspec->overLap[0]);
  if (numItems!=1 || tileTspec->overLap[0] < 0)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-pixels-overlap-per-tile-ix!", fp);
 
  numItems = fscanf(fp, "num-pixels-overlap-per-tile-iy %d\n", &tileTspec->overLap[1]);
  if (numItems!=1 || tileTspec->overLap[1] < 0)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-pixels-overlap-per-tile-iy!", fp);
 
  numItems = fscanf(fp, "total-num-pixels-per-tile-ix %d\n", &tileTspec->totalPixels[0]);
  if (numItems!=1 || tileTspec->totalPixels[0] != (tileTspec->numPixelsX + tileTspec->overLap[0]))
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid total-num-pixels-per-tile-ix!", fp);
 
  numItems = fscanf(fp, "total-num-pixels-per-tile-iy %d\n", &tileTspec->totalPixels[1]);
  if (numItems!=1 || tileTspec->totalPixels[1] != (tileTspec->numPixelsY + tileTspec->overLap[1]))
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid total-num-pixels-per-tile-iy!", fp);
 
  numItems = fscanf(fp, "num-components-per-pixel %d\n", &tileTspec->numComponentsPixel);
  if (numItems!=1 || 
      tileTspec->numComponentsPixel<1 ||
      tileTspec->numComponentsPixel==2 ||
      tileTspec->numComponentsPixel>3)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-components-per-pixel!", fp);
 
  numItems = fscanf(fp, "num-bytes-per-component %d\n", &tileTspec->numBytesPerComponent);
  if (numItems!=1 || 
      tileTspec->numBytesPerComponent<1 ||
      tileTspec->numBytesPerComponent==3 ||
      tileTspec->numBytesPerComponent>4)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid num-bytes-per-component!", fp);

  tileTspec->componentNumberType = tspReadSymbol (fp, "component-number-type %s\n", 2,
						  tspComponentNumberTypeStart+1, tspComponentNumberTypeEnd-1);
  if (tileTspec->componentNumberType == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid component-number-type!", fp);
 
  tileTspec->storageFormat = tspReadSymbol (fp, "storage-format %s\n", 2,
					    tspStorageFormatStart+1, tspStorageFormatEnd-1);
  if (tileTspec->storageFormat == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading tile.tspec: invalid storage-format!", fp);
 
  fclose(fp);
  return TRUE;
}

/*
 * Synopsis: int tspReadPixelTspec(char *, PixelTspec *)
 *
 * Description: Read the "pixel.tspec" files which correspond to data such as number
 * 	of pixels in each tiles, scaling and offsets. Refer to docs about the different
 *	fields. This function is hidden from the end user. Once again, pixelTspec is 
 *      assumed to point to a pre-allocated space.
 *
 * Externals: NONE
 *
 *
 * Returns: TRUE - on success, pixelTspec contains the data.
 *	    FALSE - on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *    September 27, 1993
 *
 */

int 
tspReadPixelTspec (char *basePathName, PixelTspec *pixelTspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/pixel.tspec", basePathName);
  return (tspReadPixelTspec1 (pathName, pixelTspec));
}

int
tspReadPixelTspec1 (char *pathName, PixelTspec *pixelTspec)
{
  FILE * fp;
  int numItems, readStatus;

  fp = fopen (pathName, "r");
  if (!fp)
    return tspError (pathName, "While reading pixel.tspec: couldn't open file!");
  
  readStatus = tspReadListOfSymbols (fp,
				     &pixelTspec->componentNames.len, 
				     &pixelTspec->componentNames.val, 
				     "component-names", "\n", 2, 
				     tspPixelComponentNameStart+1,
				     tspPixelComponentNameEnd-1);
  if (readStatus == FALSE)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid component-names!", fp);

  numItems = fscanf(fp, "min-pixel-value %f\n", &pixelTspec->minVal);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid min-pixel-value!", fp);

  numItems = fscanf(fp, "max-pixel-value %f\n", &pixelTspec->maxVal);
  if (numItems!=1 || pixelTspec->maxVal < pixelTspec->minVal)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid max-pixel-value!", fp);

  pixelTspec->scalingType = tspReadSymbol (fp, "pixel-scaling-type %s\n", 2, tspScalingTypeStart+1, tspScalingTypeEnd-1);
  if (pixelTspec->scalingType == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid pixel-scaling-type!", fp);

  numItems = fscanf(fp, "scale %f\n", &pixelTspec->scale);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid scale!", fp);

  numItems = fscanf(fp, "offset %f\n", &pixelTspec->offset);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading pixel.tspec: invalid offset!", fp);

  fclose(fp);
  return TRUE;
}

/*
 * Synopsis: int tspReadImageTransformationTspec(char *, ImageTransformationTspec)
 *
 * Description: Reads the image transformation tspec files for a level. Given a pathname
 *   	the resolution level and a pointer to a buffer to store the data, this function
 *      opens the file and reads the data. Refer to the docs for further explanation on
 *      each of the data fields. This function is hidden from the end user.
 *
 * Externals: NONE
 *
 *
 * Returns: FALSE - on failure
 *	    TRUE  - on success, the data is stored in imageTransform.
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *    September 27, 1993
 *
 */

int 
tspReadImageTransformationTspec (char *basePathName, ImageTransformationTspec *imageTransform)
{
  char pathName[1024];
  sprintf (pathName, "%s/image-transformation.tspec", basePathName);
  return (tspReadImageTransformationTspec1 (pathName, imageTransform));
}

int 
tspReadImageTransformationTspec1 (char * pathName, ImageTransformationTspec *imageTransform)
{
 FILE * fp;
 int numItems;

 fp = fopen (pathName,"rb");
 if (!fp)
   return tspError (pathName, "While reading image-transformation.tspec: couldn't open file!");
 
 numItems = fscanf(fp,"uv-to-ixiy-matrix (%f %f %f %f %f %f %f %f %f)\n",
		   &imageTransform->uvtoIXIY[0], &imageTransform->uvtoIXIY[1],
		   &imageTransform->uvtoIXIY[2], &imageTransform->uvtoIXIY[3],
		   &imageTransform->uvtoIXIY[4], &imageTransform->uvtoIXIY[5],
		   &imageTransform->uvtoIXIY[6], &imageTransform->uvtoIXIY[7],
		   &imageTransform->uvtoIXIY[8]);
 if (numItems!=9)
   return tspErrorAndClose (pathName, "While reading image-transformation.tspec: invalid uv-to-ixiy-matrix!", fp);

 numItems = fscanf(fp,"ixiy-to-uv-matrix (%f %f %f %f %f %f %f %f %f)\n",
		   &imageTransform->ixiytoUV[0], &imageTransform->ixiytoUV[1],
		   &imageTransform->ixiytoUV[2], &imageTransform->ixiytoUV[3], 
		   &imageTransform->ixiytoUV[4], &imageTransform->ixiytoUV[5],
		   &imageTransform->ixiytoUV[6], &imageTransform->ixiytoUV[7],
		   &imageTransform->ixiytoUV[8]);
 if (numItems!=9)
   return tspErrorAndClose (pathName, "While reading image-transformation.tspec: invalid ixiy-to-uv-matrix!", fp);
 
 fclose(fp);
 return TRUE;	
}

int 
tspReadProjectionTspec (char *basePathName, ProjectionTspec *tspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/projection.tspec", basePathName);
  return (tspReadProjectionTspec1 (pathName, tspec));
}

int
tspReadProjectionTspec1 (char *pathName, ProjectionTspec *tspec)
{
  FILE * fp;
  char buffer[1024];
  int numItems, i;

  fp = fopen (pathName, "rb");
  if (!fp)
    return tspError (pathName, "While reading projection.tspec: couldn't open file!");
  
  tspec->projectionType = tspReadSymbol (fp, "projection-type %s\n", 2, tspProjectionTypeStart+1, tspProjectionTypeEnd-1);
  if (tspec->projectionType == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading projection.tspec: invalid projection-type!", fp);

  switch (tspec->projectionType) {
  
  case tspAffineProjection:

    numItems = fscanf(fp,"lcs-to-uv-projective-matrix (%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f)\n",
		      &tspec->lcsToUvProjectiveMatrix[0], &tspec->lcsToUvProjectiveMatrix[1],
		      &tspec->lcsToUvProjectiveMatrix[2], &tspec->lcsToUvProjectiveMatrix[3], 
		      &tspec->lcsToUvProjectiveMatrix[4], &tspec->lcsToUvProjectiveMatrix[5],
		      &tspec->lcsToUvProjectiveMatrix[6], &tspec->lcsToUvProjectiveMatrix[7],
		      &tspec->lcsToUvProjectiveMatrix[8], &tspec->lcsToUvProjectiveMatrix[9],
		      &tspec->lcsToUvProjectiveMatrix[10], &tspec->lcsToUvProjectiveMatrix[11],
		      &tspec->lcsToUvProjectiveMatrix[12], &tspec->lcsToUvProjectiveMatrix[13],
		      &tspec->lcsToUvProjectiveMatrix[14], &tspec->lcsToUvProjectiveMatrix[15]);
    if (numItems!=16)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid lcs-to-uv-projective-matrix!", fp);

    /* uva-to-lcs-matrix is an optional entry after lcs-to-uv-projective-matrix, whose default value is the identity matrix */

    numItems = fscanf(fp, "%s", buffer);
    if (numItems != 1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid entry after lcs-to-uv-projective-matrix!", fp);
   
    if (strcmp(buffer, "uva-to-lcs-matrix") == 0) {
      numItems = fscanf(fp,"lcs-to-uv-projective-matrix (%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f)\n",
			&tspec->uvaToLcsMatrix[0], &tspec->uvaToLcsMatrix[1],
			&tspec->uvaToLcsMatrix[2], &tspec->uvaToLcsMatrix[3], 
			&tspec->uvaToLcsMatrix[4], &tspec->uvaToLcsMatrix[5],
			&tspec->uvaToLcsMatrix[6], &tspec->uvaToLcsMatrix[7],
			&tspec->uvaToLcsMatrix[8], &tspec->uvaToLcsMatrix[9],
			&tspec->uvaToLcsMatrix[10], &tspec->uvaToLcsMatrix[11],
			&tspec->uvaToLcsMatrix[12], &tspec->uvaToLcsMatrix[13],
			&tspec->uvaToLcsMatrix[14], &tspec->uvaToLcsMatrix[15]);
      if (numItems!=16)
	return tspErrorAndClose (pathName, "While reading projection.tspec: invalid uva-to-lcs-matrix!", fp);
    }
    else { /* If the uva-to-lcs-matrix entry is missing, fill in the matrix as an identity matrix. */
      for (i=0; i < 16; ++i) 
	tspec->uvaToLcsMatrix[i] = 0.0;
      for (i=0; i < 16; i+= 5) 
	tspec->uvaToLcsMatrix[i] = 1.0;
    }
    if (strcmp(buffer, "inverse-focal-length") != 0)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid inverse-focal-length keyword!", fp);
    
    numItems = fscanf(fp,"%f\n", &tspec->inverseFocalLength);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid inverse-focal-length value!", fp);
    
    numItems = fscanf(fp,"piercing-point (%f%f)\n", &tspec->piercingPoint[0], &tspec->piercingPoint[1]);
    if (numItems!=2)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid piercing-point!", fp);

    /* coeffs is an optional array of 6 numbers.  numCoeffs should either be 0 (no coeffs read), or 6 */

    tspec->numCoeffs=0;
    numItems = fscanf(fp,"coeffs (%f%f%f%f%f%f)\n", &tspec->coeffs[0], &tspec->coeffs[1],
		      &tspec->coeffs[2], &tspec->coeffs[3],
		      &tspec->coeffs[4], &tspec->coeffs[5]);
    if (numItems==6)
      tspec->numCoeffs=6;
    
    break;
   
  case tspUtmProjection:
    
    numItems = fscanf(fp,"utm-zone-number %d\n", &tspec->utmZoneNumber);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid utm-zone-number!", fp);
    
    numItems = fscanf(fp,"central-meridian %f\n", &tspec->centralMeridian);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid central-meridian!", fp);
    
    numItems = fscanf(fp,"k0 %f\n", &tspec->k0);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid k0!", fp);
    
    numItems = fscanf(fp,"geoid-separation %f\n", &tspec->geoidSeparation);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading projection.tspec: invalid geoid-separation!", fp);
    
    break;
    
  default:
    return tspErrorAndClose (pathName, "While reading projection.tspec: invalid projection-type!", fp);
  }
  
  fclose(fp);
  return TRUE;
}

/*
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Stephen Lau
 * 
 * Date:
 *
 */

int 
tspReadCoordTspec (char *basePathName, CoordTspec *coordTspec)
{
  char pathName[1024];
  sprintf (pathName, "%s/3d-coordinate-system.tspec", basePathName);
  return (tspReadCoordTspec1 (pathName, coordTspec));
}

int 
tspReadCoordTspec1 (char *pathName, CoordTspec *coordTspec)
{
  char buffer[1024];
  FILE * fp;
  int numItems, readStatus;

  fp = fopen (pathName, "r");
  if (!fp)
    return tspError (pathName, "While reading 3d-coordinate-system.tspec: couldn't open file!");

  coordTspec->systemType = tspReadSymbol (fp, "3d-coordinate-system-type %s\n", 2, 
					  tspCoordSystemTypeStart+1, tspCoordSystemTypeEnd-1);
  if (coordTspec->systemType == tspSymbolError || coordTspec->systemType == tspNone)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid 3d-coordinate-system-type!", fp);

  coordTspec->spheroidName = tspReadSymbol (fp, "spheroid-name %s\n", 2, tspSpheroidNameStart+1, tspSpheroidNameEnd-1);
  if (coordTspec->spheroidName == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid spheroid-name!", fp);

  numItems = fscanf(fp,"equatorial-radius %lf\n", &coordTspec->equatorialRadius);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid equatorial-radius!", fp);

  numItems = fscanf(fp,"polar-radius %lf\n", &coordTspec->polarRadius);
  if (numItems!=1)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid polar-radius!", fp);

  numItems = fscanf(fp,"geocentric-to-lcs-matrix (%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f)\n",
		    &coordTspec->lvcstoGeo[0], &coordTspec->lvcstoGeo[1],
		    &coordTspec->lvcstoGeo[2], &coordTspec->lvcstoGeo[3], 
		    &coordTspec->lvcstoGeo[4], &coordTspec->lvcstoGeo[5],
		    &coordTspec->lvcstoGeo[6], &coordTspec->lvcstoGeo[7],
		    &coordTspec->lvcstoGeo[8], &coordTspec->lvcstoGeo[9],
		    &coordTspec->lvcstoGeo[10], &coordTspec->lvcstoGeo[11],
		    &coordTspec->lvcstoGeo[12], &coordTspec->lvcstoGeo[13],
		    &coordTspec->lvcstoGeo[14], &coordTspec->lvcstoGeo[15]);
  if (numItems!=16)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid geocentric-to-lcs-matrix!", fp);

  numItems = fscanf(fp,"lcs-to-geocentric-matrix (%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f)\n",
		    &coordTspec->geotoLVCS[0], &coordTspec->geotoLVCS[1],
		    &coordTspec->geotoLVCS[2], &coordTspec->geotoLVCS[3], 
		    &coordTspec->geotoLVCS[4], &coordTspec->geotoLVCS[5],
		    &coordTspec->geotoLVCS[6], &coordTspec->geotoLVCS[7],
		    &coordTspec->geotoLVCS[8], &coordTspec->geotoLVCS[9],
		    &coordTspec->geotoLVCS[10], &coordTspec->geotoLVCS[11],
		    &coordTspec->geotoLVCS[12], &coordTspec->geotoLVCS[13],
		    &coordTspec->geotoLVCS[14], &coordTspec->geotoLVCS[15]);
  if (numItems!=16)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid lcs-to-geocentric-matrix!", fp);

  coordTspec->lcsType = tspReadSymbol (fp, "lcs-type %s\n", 2, tspLvcsCoordSystem, tspNone);
  if (coordTspec->lcsType == tspSymbolError)
    return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid lcs-type!", fp);
      
  if (coordTspec->lcsType == tspLvcsCoordSystem) {

    numItems = fscanf(fp,"origin-lat %f\n", &coordTspec->originLat);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid origin-lat!", fp);

    numItems = fscanf(fp,"origin-long %f\n", &coordTspec->originLong);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid origin-long!", fp);

    coordTspec->originAltitude = 0;
    numItems = fscanf(fp,"origin-altitude %f\n", &coordTspec->originAltitude);
    /* It's OKO if this is not present */

    numItems = fscanf(fp,"local-units-per-meter %f\n", &coordTspec->localUnitsPerMeter);
    if (numItems!=1)
      return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid local-units-per-meter!", fp);
   
    readStatus = tspReadListOfSymbols (fp, 
				       &coordTspec->componentNames.len, 
				       &coordTspec->componentNames.val,
				       "component-names", "\n",
				       2, tspCoordComponentNameStart+1, tspCoordComponentNameEnd-1);
    if (readStatus == FALSE)
      return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid component-names!", fp);

    /* Need to check for both "meters" and ":meters" because the initial definition of component-units was ambiguous */
    numItems = fscanf(fp,"component-units %s\n", buffer);
    coordTspec->componentUnits = tspMeters;
    if (numItems!=1 || !( (strcmp (buffer, "meters") ==0) || (strcmp (buffer, ":meters") ==0) ))
      return tspErrorAndClose (pathName, "While reading 3d-coordinate-system.tspec: invalid component-units!", fp);
  }

  fclose(fp);
  return TRUE;
}

