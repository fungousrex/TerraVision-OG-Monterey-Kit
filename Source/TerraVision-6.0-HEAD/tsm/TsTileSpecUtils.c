/*
 * Name: TsTileSpecUtils.c
 *
 * Description: 
 *
 * Function List:
 *
 * Revision History:
 *
 *   $Id: TsTileSpecUtils.c,v 1.21.6.2 2002/03/14 05:22:54 reddy Exp $
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

static void tspSwapFloats( float *value1, float *value2 );

/*
 * See TsTileSpecDecs.h for an explanation of the following
 * array.  
 */

const char *tspSymbolStrings[] = {
  "unknown",				/* tspUnknownSymbol */

  "",					/* tspTileSetTypeStart */
  "simple-tile-set",			/* tspSimpleTileSetType */
  "pyramid-tile-set",			/* tspPyramidTileSetType */
  "rectangular-orga-tile-set",		/* tspRectangularORGATileSetType */
  "multi-image-tile-set",		/* tspMultiImageTileSetType */
  "terraform-rectangular-area-tile-set",/* tspTerraFormRectangularAreaTileSetType */
  "terravision-tile-set",		/* tspTerraVisionTileSetType */
  "geo-pyramid-tile-set",		/* tspGeoPyramidTileSetType */
  "",					/* tspTileSetTypeEnd */

  "",					/* tspTileTypeStart */
  "dem",				/* tspDemType */
  "ortho-image",			/* tspOiType */
  "map",				/* tspMapType */
  "aerial-image",			/* tspAerialImageType */
  "",					/* tspTileTypeEnd */

  "",					/* tspSubtypeStart */
  "none",				/* tspNoneSubtype */
  "usgs",				/* tspUsgsSubtype */
  "mosaic",				/* tspMosaicSubtype */
  "usgs-control",			/* tspUsgsControlSubtype */
  "",					/* tspSubtypeEnd */

  "",					/* tspOperationStart */
  "merge",				/* tspMerge */
  "window",				/* tspWindow */
  "tile",				/* tspTile */
  "rectify",				/* tspRectify */
  "oi-trim",				/* tspOiTrim */
  "utm-to-lvcs",			/* tspUtmToLvcs */
  "mosaic",				/* tspMosaic */
  "gauss-reduce",			/* tspGaussReduce */
  "point-sample",			/* tspPointSample */
  "terraform-mosaic",			/* tspTerraFormMosaic */
  "scan",				/* tspScan */
  "usgs-rectify",			/* tspUsgsRectify */
  "usgs-mosaic",			/* tspUsgsMosaic */
  "gpyramid",				/* tspGPyramid */
  "",					/* tspOperationEnd */

  "",					/* tspComponentNumberTypeStart */
  "unsigned-byte",			/* tspUnsignedByte */
  "signed-byte",			/* tspSignedByte */
  "",					/* tspComponentNumberTypeEnd */

  "",					/* tspStorageFormatStart */
  "lrbt-raster",			/* tspLrbtRaster */
  "jpeg-compressed",			/* tspJPEGCompressed */
  "",					/* tspStorageFormatEnd */

  "",					/* tspCoordSystemTypeStart */
  "spheroid",				/* tspSpheroid */
  "",					/* tspCoordSystemTypeEnd */

  "",					/* tspSpheroidNameStart */
  "nad-83",				/* tspNad83 */
  "wgs-84",				/* tspWGS84 */
  "",					/* tspSpheroidNameEnd */

  "",					/* tspCoordSystemStart */
  "utm",				/* tspUtmCoordSystem */
  "lvcs",				/* tspLvcsCoordSystem */
  "none",				/* tspNone */
  "",					/* tspCoordSystemEnd */

  "",					/* tspCoordUnitStart */
  "meters",				/* tspMeters */
  "degrees",				/* tspDegrees */
  "",					/* tspCoordUnitEnd */

  "",					/* tspPixelComponentNameStart */
  "red",				/* tspRed */
  "green",				/* tspGreen */
  "blue",				/* tspBlue */
  "elevation",				/* tspElevation */
  "average",				/* tspAverage */
  "",					/* tspPixelComponentNameEnd */

  "",					/* tspScalingTypeStart */
  "linear",				/* tspLinear */
  "",					/* tspScalingTypeEnd */

  "",					/* tspProjectionTypeStart */
  "affine-projection",			/* tspAffineProjection */
  "utm-projection",			/* tspUtmProjection */
  "latlong-projection",			/* tspLatLongProjection */
  "",					/* tspProjectionTypeEnd */

  "",					/* tspTileMapStatusStart */
  "empty",				/* tspTileMapEmpty */
  "full",				/* tspTileMapFull */
  "",					/* tspTileMapStatusEnd */

  "",					/* tspPlanetStart */
  "mercury",				/* tspMercury */
  "venus",				/* tspVenus */
  "earth",				/* tspEarth */
  "mars",				/* tspMars */
  "jupiter",				/* tspJupiter */
  "saturn",				/* tspSaturn */
  "uranus",				/* tspUranus */
  "neptune",				/* tspNeptune */
  "pluto",				/* tspPluto */
  "",					/* tspPlanetEnd */

  "",					/* tspCoordComponentNameStart */
  "x",					/* tspX */
  "y",					/* tspY */
  "z",					/* tspZ */
  "",					/* tspCoordComponentNameEnd */

  "",					/* tspFileStart */
  "stdin",				/* tspStdin */
  "stdout",				/* tspStdout */
  "stderr",				/* tspStderr */
  "",					/* tspFileEnd */

  ""					/* tspNumSymbols */
};

/* The geoid84[][] array contains a sampling of WGS 84 geoid separation */
/* values. Each sample covers a 10x10 deg of the earth's surface. This  */
/* data was taken from the xform v1.8 package (file geoid84.dat).       */
/* These values are used to compute the geoid-separation to put in a    */
/* tspec file for any region of the earth that a tile covers.           */

#define GEOID84_X_SIZE 37                          /* longitude samples */
#define GEOID84_Y_SIZE 20                            /* latitude sample */

float geoid84[GEOID84_Y_SIZE][GEOID84_X_SIZE] = {
 -30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,
 -4, -1,  1,  4,  4,  6,  5,  4,  2, -6,-15,-24,-33,-40,-48,-50,-53,-52,-53,-54,-55,-52,-48,-42,-38,-38,-29,-26,-26,-24,-23,-21,-19,-16,-12, -8, -4,
 16, 16, 17, 21, 20, 26, 26, 22, 16, 10, -1,-16,-29,-36,-46,-55,-54,-59,-61,-60,-61,-55,-49,-44,-38,-31,-25,-16, -6,  1,  4,  5,  4,  2,  6, 12, 16,
 16, 19, 25, 30, 35, 33, 35, 30, 27, 10, -2,-14,-23,-30,-33,-29,-35,-43,-45,-43,-37,-32,-30,-25,-23,-22,-16,-10, -2, 10, 20, 20, 21, 24, 22, 17, 16,
 25, 26, 34, 39, 45, 45, 38, 39, 28, 13, -1,-15,-22,-22,-18,-15,-14,-10,-15,-18,-18,-16,-17,-15,-10,-10, -8, -2,  6, 14, 13,  3,  3, 10, 20, 27, 25,
 18, 26, 31, 33, 39, 41, 30, 24, 13, -2,-20,-32,-33,-27,-14, -2,  5, 20, 21,  6,  1, -7,-12,-12,-12,-10, -7, -1,  8, 23, 15, -2, -6,  6, 21, 24, 18,
 22, 27, 34, 29, 14, 15, 15,  7, -9,-25,-37,-39,-23,-14, 15, 33, 34, 45, 46, 22,  5, -2, -8,-13,-10, -7, -4,  1,  9, 32, 16,  4, -8,  4, 12, 15, 22,
 17, 23, 21,  8, -9,-10,-11,-20,-40,-47,-45,-25,  5, 23, 45, 58, 57, 63, 51, 27, 10,  0, -9,-11, -5, -2, -3, -1,  9, 35, 20, -5, -6, -5,  0, 13, 17,
 12, 13, -2,-14,-25,-32,-38,-60,-75,-63,-26,  0, 35, 52, 68, 76, 64, 52, 36, 22, 11,  6, -1, -8,-10, -8,-11, -9,  1, 32,  4,-18,-13, -9,  4, 14, 12,
 18, 12,-13, -9,-28,-49,-62,-89,-102,-63,-9, 33, 58, 73, 74, 63, 50, 32, 22, 16, 17, 13,  1,-12,-23,-20,-14, -3, 14, 10,-15,-27,-18,  3, 12, 20, 18,
 22, 23,  2, -3, -7,-36,-59,-90,-95,-63,-24, 12, 53, 60, 58, 46, 36, 26, 13, 12, 11,  2,-11,-28,-38,-29,-10,  3,  1,-11,-41,-42,-16,  3, 17, 33, 22,
 31, 26, 15,  6,  1,-29,-44,-61,-67,-59,-36,-11, 21, 39, 49, 39, 22, 10,  5, 10,  7, -7,-23,-39,-47,-34, -9,-10,-20,-45,-48,-32, -9, 17, 25, 31, 31,
 36, 28, 29, 17, 12,-20,-15,-40,-33,-34,-34,-28,  7, 29, 43, 20,  4, -6, -7, -5, -8,-15,-28,-40,-42,-29,-22,-26,-32,-51,-40,-17, 17, 31, 34, 44, 36,
 52, 48, 35, 40, 33, -9,-28,-39,-48,-59,-50,-28,  3, 23, 37, 18, -1,-11,-12,-10,-13,-20,-31,-34,-21,-16,-26,-34,-33,-35,-26,  2, 33, 59, 52, 51, 52,
 47, 48, 42, 28, 12,-10,-19,-33,-43,-42,-43,-29, -2, 17, 23, 22,  6,  2, -8,  8,  8,  1,-11,-19,-16,-18,-22,-35,-40,-26,-12, 24, 45, 63, 62, 59, 47,
 47, 41, 21, 18, 14,  7, -3,-22,-29,-32,-32,-26,-15, -2, 13, 17, 19,  6,  2,  9, 17, 10, 13,  1,-14,-30,-39,-46,-42,-21,  6, 29, 49, 65, 60, 57, 47,
 51, 43, 29, 20, 12,  5, -2,-10,-14,-12,-10,-14,-12, -6, -2,  3,  6,  4,  2,  2,  1, -1, -3, -7,-14,-24,-27,-25,-19,  3, 24, 37, 47, 60, 61, 58, 51,
 33, 34, 28, 23, 17, 13,  9,  4,  4,  1, -2, -2,  0,  2,  3,  2,  1,  1,  3,  1, -2, -3, -3, -3, -1,  3,  1,  5,  9, 11, 19, 27, 31, 34, 33, 34, 33,
 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13
};

static const char lpqSet[] = "(\"";
enum lpqIndex {
  lpqLeftParen = 0,
  lpqQuote
};
typedef enum lpqIndex lpqIndex;

static const char rpqSet[] = ")\"";
enum rpqIndex {
  rpqRightParen = 0,
  rpqQuote
};
typedef enum rpqIndex rpqIndex;

/* STATIC FUNCTION PROTOYPES */

static int charIndexFromSet (char *, const char *);
static int nextCharIndexFromFile (FILE *, const char *);

static int
charIndexFromSet (char *testChar, const char *charSet)
{
  int charIndex;

  for (charIndex = 0; charIndex < (int) strlen (charSet); ++charIndex) {
    if (strncmp (testChar, &charSet[charIndex], 1) == 0) return charIndex;
  }
  return -1;
}

static int
nextCharIndexFromFile (FILE *fp, const char *charSet)
{
  char buffer[1];
  int numItems, charIndex;

  do {
    numItems = fscanf (fp, "%1c", buffer);
    if (numItems == 0) return -1;
    charIndex = charIndexFromSet (buffer, charSet);
  } while (charIndex == -1);
  return charIndex;
}

char *tspAppendFileNameToPathName (const char *pathName, const char *fileName)
{
  static char * funcName = "tspAppendFileNameToPathName()";
  char *finalName;

  finalName = (char *)
     tsmMemManagerCalloc (1, 
			  (strlen(pathName)+strlen(fileName)+2) *
			  (sizeof(char)));
  if(!finalName) {
    tsmError (funcName, "Unable to allocate space for finalName.");
    return NULL;
  }

  strcpy (finalName, pathName);
  if (strncmp(&finalName[strlen(finalName)-1], "/", 1) != 0)
    strcat (finalName, "/");
  strcat (finalName, fileName);

  return (finalName);
}

void
tspTileFileName (int ix, int iy, char *tileFileName)
{
  sprintf (tileFileName, "%s%d%s%d.tile", 
	   (ix<0) ? "m" : "p",
	   ix,
	   (iy<0) ? "m" : "p",
	   iy);
}

void
tspLevelTileFileName (int ix, int iy, int level, char *levelTileFileName)
{
  sprintf (levelTileFileName, "%d/tiles/%s%d%s%d.tile", 
	   level,
	   (ix<0) ? "m" : "p",
	   ix,
	   (iy<0) ? "m" : "p",
	   iy);
}

void
tspTilePathName (const char *pyramidPathName, int ix, int iy, int level, char *tilePathName)
{
  if (strncmp(&pyramidPathName[strlen(pyramidPathName)-1], "/", 1) != 0) {
    sprintf (tilePathName, "%s/%d/tiles/%s%d%s%d.tile", 
	     pyramidPathName,
	     level,
	     (ix<0) ? "m" : "p",
	     ix,
	     (iy<0) ? "m" : "p",
	     iy);
  }
  else {
    sprintf (tilePathName, "%s%d/tiles/%s%d%s%d.tile", 
	     pyramidPathName,
	     level,
	     (ix<0) ? "m" : "p",
	     ix,
	     (iy<0) ? "m" : "p",
	     iy);
  }
}

void
tspTIFFFileName (const char *pyramidPathName, int level, tspSymbol imgFmt,
		 char *TIFFFileName)
{
  sprintf (TIFFFileName, "%s/%d/tiles%s.tiff",
	   pyramidPathName,
	   level,
	   ( imgFmt == tspJPEGCompressed ) ? "_jpeg" : "_lrbt" );
}

/* tsmTIFFFilename is the new version of tspTIFFFileName that */
/* directly supports the connection interface */

void
tsmTIFFFilename( TsmConnection connect, int level, char *TIFFFilename )
{
  sprintf( TIFFFilename, "%s/%d/tiles%s.tiff", connect->location->uri, level,
	   ( connect->format == TSM_JPEG_FORMAT ) ? "_jpeg" :
	   (( connect->format == TSM_LZW_FORMAT ) ? "_lzw" : "_lrbt") );
}

void
tspTspecFileName (const char *pyramidPathName, char *tspecFileName)
{
  strcpy( tspecFileName, pyramidPathName );
  if ( tspecFileName[strlen(tspecFileName)-1] != '/' )
    strcat( tspecFileName, "/" );
  strcat( tspecFileName, "tiles.tspec" );
}

int
tspStringIndexFromSet (const char *testString, int minIndex, int maxIndex,
		       const char **stringSet, int unknownIndex)
{
  static const char *funcName = "tspStringIndexFromSet()";
  int stringIndex, i;

  stringIndex = unknownIndex;
  TSMTRACE ((TSMMASK|2, funcName, "check \"%s\" against stringSet[%d - %d]",
	     testString, minIndex, maxIndex));
  for (i = minIndex; i <= maxIndex; ++i) {
    if (strcmp (testString, stringSet[i]) == 0) {
      stringIndex = i;
      break;
    }
  }
  TSMTRACE ((TSMMASK|2, funcName, "return %d", stringIndex));
  return stringIndex;
}

/*
 * Synopsis:
 *   const char *tspStringFromSymbol (tspSymbol symbol)
 *
 * Doc++ Description: *//** 
 *   Return a string description of a tspSymbol.
 *
 *   tspSymbols are used to represent various enumerated values within
 *   the tsmApi. This function lets you take a tspSymbol and return
 *   a string to represent that symbol, e.g. (tspSymbol) tspOiType would
 *   return a string "ortho-image".
 *
 *   The tspSymbol enum is defined in the file TsTileSpecDecs.h.
 *
 * @param    symbol the tspSymbol to be described
 *
 * @return   the string describing the tspSymbol, or "unknown" if an error
 *           occurred
 *
 * @author   
 * @version  
 *
 * @see      tspSymbolFromString
 *
 */

const char *
tspStringFromSymbol (tspSymbol symbol)
{
  if ( (symbol >= tspUnknownSymbol) && (symbol < tspNumSymbols-1) )
    return tspSymbolStrings[symbol];
  else
    fprintf (stderr, "INTERNAL ERROR: invalid argument to tspStringFromSymbol: %d\n", symbol);
    return tspSymbolStrings[tspUnknownSymbol];
}

/*
 * Synopsis:
 *    tspSymbol tspSymbolFromString (const char *buffer, 
 *               tspSymbol minSymbol, tspSymbol maxSymbol)
 *
 * Doc++ Description: *//** 
 *   Parse a string to return a tspSymbol.
 *
 *   tspSymbols are used to represent various enumerated values within
 *   the tsmApi. This function lets you take a string and return the
 *   tspSymbol that the string represents, e.g. "ortho-image" would
 *   return a tspSymbol tspOiType.
 *
 *   You can restrict the valid range of tspSymbols which can be returned
 *   via two tspSymbol parameters, e.g. tspTileTypeStart and tspTileTypeEnd.
 *   The tspSymbol enum is defined in the file TsTileSpecDecs.h.
 *
 * @param    buffer the string to be parsed to a tspSymbol
 * @param    minSymbol the minimum tspSymbol to match the string against
 * @param    maxSymbol the maximum tspSymbol to match the string against
 *
 * @return   the extracted tspSymbol, or tspSymbolError if failed
 *
 * @author   Martin Reddy
 * @version  24 October 1997
 *
 * @see      tspStringFromSymbol
 *
 */

tspSymbol
tspSymbolFromString (const char *buffer, 
		     tspSymbol minSymbol, tspSymbol maxSymbol)
{
  return (tspSymbol) tspStringIndexFromSet (buffer, minSymbol, maxSymbol, tspSymbolStrings, tspSymbolError);
}

void
tspPrintListOfQuotedStrings (FILE *fp, int numStrings, char **stringArray,
			     const char *initialFormatString, const char *finalFormatString)
{
  int i;

  if ( (int) strlen (initialFormatString) > 0)
    fprintf (fp, initialFormatString);

  if (numStrings == 0)
    fprintf (fp, "\"\"");
  else {
    for (i = 0; i < numStrings-1; ++i)
      fprintf (fp, "\"%s\" ", stringArray[i]);
    fprintf (fp, "\"%s\"", stringArray[numStrings-1]);
  }

  if ( (int) strlen (finalFormatString) > 0)
    fprintf (fp, finalFormatString);
}

void
tspPrintListOfStrings (FILE *fp, int numStrings, const char **stringArray,
		       const char *initialFormatString, const char *finalFormatString)
{
  int i;

  if ( (int) strlen (initialFormatString) > 0)
    fprintf (fp, initialFormatString);

  if (numStrings > 0) {
    for (i = 0; i < numStrings-1; ++i)
      fprintf (fp, "%s ", stringArray[i]);
    fprintf (fp, "%s", stringArray[numStrings-1]);
  }

  if ( (int) strlen (finalFormatString) > 0)
    fprintf (fp, finalFormatString);
}

void
tspPrintSymbol (FILE *fp, const tspSymbol symbol, const char *formatString)
{
  if ( symbol < 0 || (int) symbol >= tspNumSymbols)
    fprintf (fp, formatString, "INVALID");
  else
    fprintf (fp, formatString, tspSymbolStrings[symbol]);
}

void
tspPrintListOfSymbols (FILE *fp, const int numSymbols, const tspSymbol *symbolArray,
		       const char *initialFormatString, const char *finalFormatString)
{
  int i;

  if ( (int) strlen (initialFormatString) > 0)
    fprintf (fp, initialFormatString);

  if (numSymbols > 0) {
    for (i = 0; i < numSymbols-1; ++i) 
      tspPrintSymbol (fp, symbolArray[i], "%s ");
    tspPrintSymbol (fp, symbolArray[numSymbols-1], "%s");
  }

  if ( (int) strlen (finalFormatString) > 0)
    fprintf (fp, finalFormatString);
}

int
tspReadListOfQuotedStrings (FILE *fp, u_int *numStrings, char ***stringArray,
			    const char *initialFormatString, const char *finalFormatString)
{
  static char * funcName = "tspReadListOfQuotedStrings()";
  char buffer[1024];
  u_int stringArrayLength = 8;
  int charIndex, numItems, end;
  char **StringArray;

  if ( (int) strlen (initialFormatString) > 0) {
    numItems = fscanf (fp, initialFormatString);
    if (numItems != 0)
      return FALSE;
  }

  *numStrings = 0;
  StringArray = (char **)
     tsmMemManagerCalloc (1,
			  stringArrayLength * sizeof (char *));
  if(!StringArray) {
    tsmError (funcName, "Error in Calloc of StringArray");
    return FALSE;
  }

  /* Search for left paren or quote */
  charIndex = nextCharIndexFromFile (fp, lpqSet);

  /* If neither were found, then the list is bogus */
  if (charIndex < 0) return FALSE;

  /* If we find a quote, then the "list" was really just a single quoted string */
  if (charIndex == lpqQuote) {
    *numStrings = 1;
    numItems = fscanf (fp, "%[^\"]\"", buffer);
    StringArray[0] = tsmMemManagerStrdup (buffer);
    if ( (int) strlen (finalFormatString) > 0)
      fscanf (fp, finalFormatString);
  }
  else {
  
    /* Otherwise, the character was a left paren, so collect the set of
     * quoted strings
     */
  
    end = FALSE;
    do {
    
      /* Search for the next quote or close paren */
      charIndex = nextCharIndexFromFile (fp, rpqSet);

      /* If neither is found, the list was bogus  */
      if (charIndex < 0) return FALSE;
    
      /* If the character is a right paren, this is the end of the list */
      if (charIndex == rpqRightParen) {
	if ( (int) strlen (finalFormatString) > 0)
	  fscanf (fp, finalFormatString);
	end = TRUE;
      }
      else { /* Otherwise, this is the start of a quoted string */
	
	/* First, check to see that we have enough room to store the new string.
	 * If not, increase the size of the array, and continue.
	 */
	
	if (*numStrings >= stringArrayLength) {
	  stringArrayLength *= 2;
	  StringArray = (char **)
	     tsmMemManagerRealloc (StringArray, 
				   stringArrayLength * sizeof (char *));
	  if(StringArray == (char **) -1) {
	    tsmError (funcName, "Error in realloc for StringArray #1.");
	    return FALSE;
	  }
	}
	
	/* If so, read in the remainder of the string, and copy it to the internalStringArray */
	
	numItems = fscanf (fp, "%[^\"]", buffer);
	StringArray[*numStrings] = tsmMemManagerStrdup (buffer);
	*numStrings += 1;
	fscanf (fp, "\"");
      }
    } while (end == FALSE);
  }
  
  /* Finally, resize the array to its final size, and move the pointer to stringArray */

  StringArray = (char **)
     tsmMemManagerRealloc (StringArray, 
			   *numStrings * sizeof (char *));
  if(StringArray == (char **) -1) {
    tsmError (funcName, "Error in realloc for StringArray #2.");
    return FALSE;
  }
  *stringArray = StringArray;
  return TRUE;
}

int
tspReadListOfStrings (FILE *fp, int *numStrings, char ***stringArray,
		      const char *initialFormatString, const char *finalFormatString)
{
  static char * funcName = "tspReadListOfStrings()";
  char buffer[1024];
  int stringArrayLength = 8;
  int numItems, numChars, bufferOffset, end;
  char **StringArray;

  if ( (int) strlen (initialFormatString) > 0) {
    numItems = fscanf (fp, initialFormatString);
    if (numItems != 0)
      return FALSE;
  }

  *numStrings = 0;
  StringArray = (char **)
     tsmMemManagerCalloc (1, stringArrayLength * sizeof (char *));
  if(!StringArray) {
    tsmError (funcName, "Unable to allocate space for StringArray.");
    return FALSE;
  }

  /* Find first string */
  numItems = fscanf (fp, "%s", buffer);
  if (numItems != 1)
    return FALSE;

  /* If the first character is not a left paren, then this is a "list" 
   * with just one unquoted string
   */

  if (strncmp (buffer, "(", 1) != 0) {
    StringArray[0] = (char *) 
       tsmMemManagerCalloc (1, (strlen(buffer)+1) * sizeof (char));
    if(!StringArray[0]) {
      tsmError (funcName, "Unable to allocate space for StringArray[0].");
      return FALSE;
    }
    strcpy (StringArray[0], buffer);
    *numStrings = 1;
    if ( (int) strlen (finalFormatString) > 0)
      fscanf (fp, finalFormatString);
  }
  else { /* Otherwise, the first character is a left paren, so its a real list */

    /* bufferOfset is set to 1 for the first string so that only the characters
     * after the initial paren are copied to stringArray.  Its then reset
     * to zero.
     */
    
    bufferOffset = 1;
    
    end = FALSE;
    do {
    
      /* First, check to see that we have enough room to store the new string.
       * If not, increase the size of the array, and continue.
       */

      if (*numStrings >= stringArrayLength) {
	stringArrayLength *= 2;
	StringArray = (char **)
	   tsmMemManagerRealloc (StringArray,
				 stringArrayLength * sizeof (char *));
	if(StringArray == (char **) -1) {
	  tsmError (funcName, "Error in realloc for StringArray");
	  return FALSE;
	}
      }

      numChars = strlen(buffer);

      /* If the string ends in a right paren, then we're finished */
     
      if (strncmp (&buffer[numChars-1], ")", 1) == 0) {
	numChars -= (1+bufferOffset);
	StringArray[*numStrings] = (char *)
	   tsmMemManagerCalloc (1, (numChars+1) * sizeof (char));
	if(!StringArray[*numStrings]) {
	  tsmError (funcName, "Unable to allocate space for StringArray."); 
	  return FALSE;
	}

	buffer[numChars+bufferOffset] = 0;
	strcpy (StringArray[*numStrings], &buffer[bufferOffset]);
	*numStrings += 1;
	if ( (int) strlen (finalFormatString) > 0)
	  fscanf (fp, finalFormatString);
	end = TRUE;
      }
      else {

	/* Otherwise, copy the whole string to StringArray */

	numChars -= bufferOffset;
	StringArray[*numStrings] = (char *) 
	   tsmMemManagerCalloc (1, (numChars+1) * sizeof (char));
	if(!StringArray[*numStrings]) {
	  tsmError (funcName, "Unable to allocate space for StringArray.");
	  return FALSE;
	}
	strncpy (StringArray[*numStrings], &buffer[bufferOffset], numChars);
	*numStrings += 1;
	
	/* Read in the next string, and start over again */
	
	bufferOffset = 0;
	numItems = fscanf (fp, "%s", buffer);
	if (numItems != 1)
	  return FALSE;
      }
    } while (end == FALSE);
  }

  /* Finally, resize the array to its final size, and move the pointer to stringArray */

  StringArray = (char **)
     tsmMemManagerRealloc (StringArray,
			   *numStrings * sizeof (char *));
  if(StringArray == (char **) -1) {
    tsmError (funcName, "Error in realloc for StringArray.");
    return FALSE;
  }
  *stringArray = StringArray;

  return TRUE;
}

tspSymbol
tspReadSymbol (FILE *fp, const char *formatString, int numArgs, ...)
{
  static char * funcName = "tspReadSymbol()";
  va_list argList;
  char buffer[1024];
  tspSymbol minSymbol, maxSymbol;
  int numItems;

  minSymbol = tspUnknownSymbol;
  maxSymbol = tspNumSymbols;

  TSMTRACE ((TSMMASK|2, funcName, "fmt = \"%s\" at fpos %ld",
	     formatString, ftell (fp)));
  numItems = fscanf (fp, formatString, buffer);
  if (numItems != 1)
    return tspSymbolError;
  
  switch (numArgs) {
    
  case 0:
    break;

  case 1:
    va_start (argList, numArgs);
    minSymbol = va_arg (argList, tspSymbol);
    break;

  case 2:
    va_start (argList, numArgs);
    minSymbol = va_arg (argList, tspSymbol);
    maxSymbol = va_arg (argList, tspSymbol);
    break;
    
  default:
    fprintf (stderr, "ERROR: tspReadSymbol was passed %d numArgs, which must be between 0 and 2!", numArgs);
    return tspSymbolError;
  }

  return tspSymbolFromString (buffer, minSymbol, maxSymbol);
}

int
tspReadListOfSymbols (FILE *fp, u_int *numSymbols, tspSymbol **symbolArray,
		      const char *initialFormatString, const char *finalFormatString,
		      int numArgs, ...)
{
  static char * funcName = "tspReadListOfSymbols()";
  va_list argList;
  char buffer[1024];
  int numItems, numChars, bufferOffset, end;
  u_int symbolArrayLength = 8;
  tspSymbol minSymbol, maxSymbol, *SymbolArray;

  minSymbol = tspUnknownSymbol;
  maxSymbol = tspNumSymbols;

  switch (numArgs) {
    
  case 0:
    break;

  case 1:
    va_start (argList, numArgs);
    minSymbol = va_arg (argList, tspSymbol);
    break;

  case 2:
    va_start (argList, numArgs);
    minSymbol = va_arg (argList, tspSymbol);
    maxSymbol = va_arg (argList, tspSymbol);
    break;
    
  default:
    fprintf (stderr, "ERROR: tspReadListOfSymbols was passed %d numArgs, which must be between 0 and 2!", numArgs);
    return tspSymbolError;
  }

  if ( (int) strlen (initialFormatString) > 0) {
    numItems = fscanf (fp, initialFormatString);
    if (numItems != 0)
      return FALSE;
  }

  *numSymbols = 0;
  SymbolArray = (tspSymbol *)
     tsmMemManagerCalloc (1,
			  symbolArrayLength * sizeof (tspSymbol));
  if(!SymbolArray) {
    tsmError (funcName, "Unable to allocate space for SymbolArray.");
    return FALSE;
  }

  /* Find first string */
  numItems = fscanf (fp, "%s", buffer);
  if (numItems != 1)
    return FALSE;

  /* If the first character is not a left paren, then this is a "list" 
   * with just one symbol
   */

  if (strncmp (buffer, "(", 1) != 0) {
    SymbolArray[0] = tspSymbolFromString (buffer, minSymbol, maxSymbol);
    *numSymbols = 1;
    if ( (int) strlen (finalFormatString) > 0)
      fscanf (fp, finalFormatString);
  } 
  else { /* Otherwise, the first character is a left paren, so its a real list */

    /* bufferOffset is set to 1 for the first string so that only the characters
     * after the initial paren are copied to stringArray.  Its then reset
     * to zero.
     */

    bufferOffset = 1;

    end = FALSE;
    do {
    
      /* First, check to see that we have enough room to store the new symbol.
       * If not, increase the size of the array, and continue.
       */

      if (*numSymbols >= symbolArrayLength) {
	symbolArrayLength *= 2;
	SymbolArray = (tspSymbol *)
	   tsmMemManagerRealloc (symbolArray,
				 symbolArrayLength * sizeof (tspSymbol));
	if(SymbolArray == (tspSymbol *) -1) {
	  tsmError (funcName, "Error in realloc for SymbolArray");
	  return FALSE;
	}
      }

      /* If the string ends in a right paren, then we're finished */
     
      numChars = strlen(buffer);

      if (strncmp (&buffer[numChars-1], ")", 1) == 0) {
	numChars -= (1+bufferOffset);
	buffer[numChars+bufferOffset] = 0;
	SymbolArray[*numSymbols] = tspSymbolFromString (&buffer[bufferOffset], minSymbol, maxSymbol);
	*numSymbols += 1;
	if ( (int) strlen (finalFormatString) > 0)
	  fscanf (fp, finalFormatString);
	end = TRUE;
      }
      else { /* Otherwise, copy the symbol to symbols */

	numChars -= bufferOffset;
	SymbolArray[*numSymbols] = tspSymbolFromString (&buffer[bufferOffset], minSymbol, maxSymbol);
	*numSymbols += 1;

	/* Read in the next string, and start over again */

	bufferOffset = 0;
	numItems = fscanf (fp, "%s", buffer);
	if (numItems != 1)
	  return FALSE;
      }
    } while (end == FALSE);
  }

  /* Finally, resize the array to its final size, and move the pointer to symbolArray */

  SymbolArray = (tspSymbol *)
     tsmMemManagerRealloc (SymbolArray,
			   *numSymbols * sizeof (tspSymbol));
  if(SymbolArray == (tspSymbol *) -1) {
    tsmError (funcName, "Error in realloc for StringArray");
    return FALSE;
  }
  *symbolArray = SymbolArray;
  return TRUE;
}

/* Utility function to print an error message and return FALSE */

int
tspError (const char *pathName, const char *string)
{
  if (pathName) {
    fprintf (stderr, "ERROR at pathName %s: %s\n", pathName, string);
  }
  else {
    fprintf (stderr, "ERROR at NULL pathName: %s\n", string);
  }
  return FALSE;
}

/* Utility function to print an error message, close a file, and return FALSE */

int
tspErrorAndClose (const char *pathName, const char *string, FILE *fp)
{
  if (pathName) {
    fprintf (stderr, "ERROR at pathName %s: %s\n", pathName, string);
  }
  else {
    fprintf (stderr, "ERROR at NULL pathName: %s\n", string);
  }
  fclose (fp);
  return FALSE;
}

FILE
*tspOpenForWrite (const char *pathName, const char *fileName)
{
  FILE *fp;
  char *fullName;
  tspSymbol pathType;
  tspSymbol minSymbol = tspFileStart, maxSymbol = tspFileEnd;

  pathType = tspSymbolFromString (pathName, ++minSymbol, --maxSymbol );

  switch (pathType){

  case tspStdin:
    fp = stdout;
    if (fileName && strlen (fileName) > 0)
      fprintf (fp, "\n%s: \n\n", fileName);
    break;

  case tspStdout:
    fp = stdout;
    if (fileName && strlen (fileName) > 0)
      fprintf (fp, "\n%s: \n\n", fileName);
    break;

  case tspStderr:
    fp = stderr;
    if (fileName && strlen (fileName) > 0)
      fprintf (fp, "\n%s: \n\n", fileName);
    break;

  default:
    fullName = tspAppendFileNameToPathName (pathName, fileName);
    fp = fopen (fullName, "wd");
    tsmMemManagerFree (fullName);
    break;
  }
  return fp;
}

void
tspCloseForWrite (FILE *fp)
{
  if ( (fp != stdout) && (fp != stderr) ) 
    fclose (fp);
}

void
tspPrintFloatVector (FILE *fp, int numElements, float *matrix)
{
  int  i;

  fprintf (fp, "%1.10g", matrix[0]);
  for (i=1; i<numElements; ++i)
    fprintf (fp, " %1.10g", matrix[i]);
  fprintf (fp, "\n");
}


/*
 * Synopsis:
 *      float tspCalcUTMZone( double longitude )
 *
 * Doc++ Description: *//** 
 *      Calculate the UTM Zone.
 *      
 *      Calculates the UTM zone for any longitude. Each UTM zone is
 *      6 deg wide, with zone 1 beginning at long. -180 deg and zone
 *      60 ending at long. +180. We assume that the longitude being
 *      passed in is in the range -180..+180 deg.
 *
 *      If you have a UTM zone/easting coordinate, then you can convert
 *      this to a longitude using the tsmUTMToLatLong function.
 *
 * @param    longitude the longitude (degrees) to calculate the UTM zone for
 *
 * @return   the UTM zone
 *
 * @author   Martin Reddy
 * @version  25 August 1997
 *
 * @see      tspCalcCentralMeridian, tsmUTMToGeo
 *
 */

int
tspCalcUTMZone( double longitude )
{
  const char *funcName = "tspCalcUTMZone";

  /* check for legal UTM coords */

  if ( longitude < -180.0 || longitude > 180.0 ) {
    tsmError( funcName, "longitude not in -180..180 (%1.3f)", longitude );
  }

  /* xform coords from [-180..180] up to [0..360) to simplify relation */

  longitude = min( longitude + 180.0, 359.0 );

  /* calculate zone by simply dividing by the zone width - 6 deg */

  return (int) floor( longitude / 6.0 ) + 1.0;
}

/*
 * Synopsis:
 *      float tspCalcCentralMeridian( int utm-zone )
 *
 * Doc++ Description: *//** 
 *      Calculate the Central Meridian of a UTM Zone (degrees).
 *      
 *      Takes a UTM zone number and returns the central meridian 
 *      which that zone corresponds to. The function checks that
 *      the zone is in range (zones range from 1 to 60). For a 
 *      diagram of the relationship between these two values,
 *      check out Snyder, J. P. (1987). "Map Projections: A Working
 *      Manual", Professional Paper 1395, U.S. Geological Survey.
 *
 *      If you wish to find the UTM zone/easting of the central meridian,
 *      then you can use the tsmGeoToUTM function to transfor, the result
 *      of this function.
 *
 * @param    utm_zone the UTM zone to calculate the central meridian for
 *
 * @return   the central meridian if successful. 0.0 if failed.
 *           (N.B. 0.0 is not a valid central meridian)
 *
 * @author   Martin Reddy
 * @version  21 August 1997
 *
 * @see      tspCalcUTMZone, tspCalcGeoidSeparation, tsmGeoToUTM
 *
 */

float
tspCalcCentralMeridian( int utm_zone )
{
  const char *funcName = "tspCalcCentralMeridian";

  if ( utm_zone < 1 || utm_zone > 60 ) {
    tsmError( funcName, "invalid utm_zone (%d). 1..60 only", utm_zone );
    return 0.0;
  }

  if ( utm_zone <= 30 )
    return ( 30.0 - utm_zone ) * -6.0 - 3.0;
  else
    return ( utm_zone - 31.0 ) * 6.0 + 3.0;
}

/*
 * Synopsis:
 *      float tspSwapFloats( float *value1, float *value2 )
 *
 * Description:
 *      swap function that works on floats
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 August 1997
 *
 */

static void
tspSwapFloats( float *value1, float *value2 )
{
  float temp;

  temp    = *value1;
  *value1 = *value2;
  *value2 = temp;
}

/*
 * Synopsis:
 *      float tspLinearInterpolate( float value1, float value2,
 *                                  float sample1, float sample2, 
 *                                  float sample )
 *
 * Description:
 *      Takes the 1D coordinate for two points (sample1 and sample2) and
 *      the value at each of these points (value and value2, respectively).
 *      You then provide any 1D coordinate and the linearly interpolated
 *      result will be returned.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 August 1997
 *
 */

float
tspLinearInterpolate( float value1, float value2,
		      float sample1, float sample2, 
		      float sample )
{
  const char *funcName = "tspLinearInterpolate";
  float ratio;

  /* check for the error case where sample1 == sample2, and also make */
  /* sure that sample2 is greater than sample1 to simplify things     */

  if ( fabs( sample2 - sample1 ) < 0.000001 ) {
    tsmError( funcName, "Cannot interpolate point source %f", sample1 );
    return value1;
  } else if ( sample1 > sample2 ) {
    tspSwapFloats( &sample1, &sample2 );
    tspSwapFloats( &value1, &value2 );
  }

  /* now we can do the interpolation. First we find the scaling ratio */
  /* i.e. the relative location of sample between sample1->sample2.   */
  /* then we can use this ratio to find the interpolated value.       */

  ratio = ( sample - sample1 ) / ( sample2 - sample1 );

  return ( value2 - value1 ) * ratio + value1;
}

/*
 * Synopsis:
 *      float tspBiLinearInterpolate( 
 *                float c1_x_sample, float c1_y_sample, float c1_value,
 *                float c2_x_sample, float c2_y_sample, float c2_value,
 *                float c3_x_sample, float c3_y_sample, float c3_value,
 *                float c4_x_sample, float c4_y_sample, float c4_value,
 *                float t_x_sample,  float t_y_sample )
 *
 * Description:
 *      Does a bi-linear interpolation over a grid of values. The routine
 *      is general, but requires that the grid being interpolated over is
 *      square, not skewed in any direction. E.g. you pass in the coordinates
 *      (c?_[x|y]_sample) for each corner of the grid, and the values at 
 *      each of these four corners (c?_value). You then provide a coordinate
 *      somewhere inside the square grid and the function returns the 
 *      bilinearly interpolated result. The corner number is as follows:
 *
 *         c1 +----+ c2
 *            |    |
 *         c3 +----+ c4
 *
 *      The routine proceeds by doing a linear interpolation between c1->c2,
 *      then an interpolation between c3->c4, and then it will interpolate
 *      between these two to get the interpolated point in 2D space.
 *
 *      N.B. this routine was written more with readability, rather than
 *      efficiency, in mind; e.g. some of the parameters are redundant.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 August 1997
 *
 */

float tspBiLinearInterpolate( 
    float c1_x_sample, float c1_y_sample, float c1_value,
    float c2_x_sample, float c2_y_sample, float c2_value,
    float c3_x_sample, float c3_y_sample, float c3_value,
    float c4_x_sample, float c4_y_sample, float c4_value,
    float t_x_sample,  float t_y_sample )
{
  float i1, i2, i3;

  i1 = tspLinearInterpolate( c1_value, c2_value,
			     c1_x_sample, c2_x_sample,
			     t_x_sample );

  i2 = tspLinearInterpolate( c3_value, c4_value,
			     c3_x_sample, c4_x_sample,
			     t_x_sample );

  i3 = tspLinearInterpolate( i1, i2,
			     c1_y_sample, c3_y_sample,
			     t_y_sample );

  return i3;
}

/*
 * Synopsis:
 *      float tspCalcGeoidSeparation( double latitude, double longitude )
 *
 * Doc++ Description: *//** 
 *      Calculate the geoid separation for any point.
 *      
 *      Works out the geoid separation for any latitude or longitude.
 *      A geoid is the name for the undulating shape that the Earth
 *      would assume if it were all measured at mean sea level. The
 *      geoid separation is the distance between a point on this surface
 *      and the point on the ideal spheroid for the Earth's surface.
 *
 *      The value is computed by bilinearly interpolating a 2D table of
 *      values sampled at a resolution of 10x10 deg. This table is
 *      defined in TsTileSpecGen.h. Values of lat/long given in the
 *      range -90..+90, and -180..+180 respectively. (N.B. you can
 *      also specify longitude in the range 0..360, where 0 is still
 *      Greenwich Meridian, but 180..360 maps to -180..0. It is not
 *      obvious that a 0..180 mapping makes sense for latitude so we
 *      do not support this.)
 *
 *      If you have UTM coordinates, then you can convert them to lat/long
 *      using the tsmUTMToGeo function.
 *
 * @param    latitude the latitude (degrees) of the point to find the 
 *           geoid separation for (-90..+90)
 * @param    longitude the longitude (degrees) of the point to find the
 *           geoid separation for (-180..+180)
 *
 * @return   the geoid separation value, or 0.0 if invalid coordinate given
 *
 * @author   Martin Reddy
 * @version  25 August 1997
 *
 * @see      tspCalcK0, tsmUTMToGeo
 *
 */

float tspCalcGeoidSeparation( double latitude, double longitude )
{
  const char  *funcName = "tspCalcGeoidSeparation";
  const float epsilon = 1.0e-3;
  double      sample_x, sample_y;
  int         cell_x, cell_y;
  float       geoid_sep;

  /* support longitudes in the range 0..360 as well. In */
  /* this case: 0..180 -> 0..180, 180..360 -> -180..0.  */

  if ( longitude > 180.0 ) longitude -= 360.0;

  /* let's clamp the coords if they are within an epsilon of */
  /* the boundardy values, e.g. in case there has just been  */
  /* some floating point rouding error stuff going on.       */

  if ( latitude > 90.0 && latitude < 90.0 + epsilon ) latitude = 90.0;
  if ( latitude < -90.0 && latitude > -90.0 - epsilon ) latitude = -90.0;
  if ( longitude > 180.0 && longitude < 180.0 + epsilon ) longitude = 180.0;
  if ( longitude < -180.0 && longitude > -180.0 - epsilon ) longitude = -180.0;

  /* check that we have valid lat/long coordinates */

  if ( longitude < -180.0 || longitude > 180.0 ) {
    tsmError( funcName, "longitude not in range (%1.2f)", longitude );
    return 0.0;
  }
  if ( latitude < -90.0 || latitude > 90.0 ) {
    tsmError( funcName, "latitude not in range (%1.2f)", latitude );
    return 0.0;
  }

  /* transform lat/long values from the range -90..+90, -180..+180 */
  /* into the range 0..180, 0..360 for indexing into look-up table */

  latitude  += 90.0;
  longitude += 180.0;

  /* work out the sample coordinates and values for the 10x10 deg cell */
  /* in the look-up table that is the top-left cell that we wish to    */
  /* interpolate over.                                                 */

  sample_x = longitude / 10.0;
  sample_y = latitude / 10.0;
  
  cell_x = (int) floor( sample_x );
  cell_y = (int) floor( sample_y );

  geoid_sep = tspBiLinearInterpolate(
      (float) cell_x,   (float) cell_y,   geoid84[cell_y][cell_x],
      (float) cell_x+1, (float) cell_y,   geoid84[cell_y][cell_x+1],
      (float) cell_x,   (float) cell_y+1, geoid84[cell_y+1][cell_x],
      (float) cell_x+1, (float) cell_y+1, geoid84[cell_y+1][cell_x+1],
      (float) sample_x, (float) sample_y );

  return geoid_sep;
}

/*
 * Synopsis:
 *      float tspCalcK0( void )
 *
 * Doc++ Description: *//** 
 *     Calculate k0 for WGS84/GRS80.
 *      
 *      Calculates the value of k0, which represents the flattening of
 *      the ellipsoid (ratio between major and minor axes) used to
 *      model the Earth. The value returned is for the WGS 84 system,
 *      which is the same as the GRS 80 system within the accuracy used.
 *      For further details, refer to Table 1, p.12 of Snyder (1987).
 *      (see above for biblio details.)
 *
 * @param    None
 *
 * @return   the value of k0
 *
 * @author   Martin Reddy
 * @version  25 August 1997
 *
 * @see      tspCalcGeoidSeparation
 *
 */

float tspCalcK0( void )
{
  return 1.0f - ( 1.0f / 298.257f );
}

/*
 * Synopsis:
 *   void tspLFToSpace( char *message )
 *
 * Description:
 *   Routine to convert all linefeed occurrences (\n) in a string to a
 *   space. This is done to process strings that are to go in a tspec
 *   file because each entry must exist on one line only, e.g. if the
 *   user entered a description string with linefeeds, then this would
 *   create an invalid tspec file.
 *
 * Externals: None
 *
 * Returns: Nothing
 *
 * Author: Martin Reddy
 *
 * Date: 12 December 1997
 *
 */

void
tspLFToSpace( char *message )
{
  int  i, len;

  if ( message != NULL ) 
    for ( i = 0, len = strlen(message); i < len; ++i, ++message )
      if ( *message == '\n' || *message == '\r' )
	*message = ' ';
}

