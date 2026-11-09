/*
 * Name: TsTileSpecParse.c
 *
 * Description:
 *      Parses the Tspec (Tile Specification) file(s) for a pyramid. The
 *      parser is flexible in that it allows optional comments within the
 *      tspec files (lines beginning with a hash symbol), and all of the
 *      tokens can appear in any order within the file. The code in this
 *      file supercedes that of TsTileSpecIn.c which supports the old style
 *      tspec organisation used by TV-I (N.B. that code is still used if
 *      we find that the pyramid is in this old format, rather than the 
 *      new single-file organisation.)
 *
 * Function List:
 *      int tspReadPyramid( char *pathName, Pyramid *pyramid, ... ) 
 *      int tspReadTspec( char *, Pyramid * )
 *
 * Dependencies:
 *
 * Revision History: 
 *   July 1997 - Initial version by Martin Reddy
 *
 *   $Id: TsTileSpecParse.c,v 1.26.6.1 2001/11/20 18:52:52 reddy Exp $
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

/* Skip over non-space characters. */
static char const *
tspTokenEnd (char const *p)
{
  char c;
  while ((c = *p) && ! isspace (c)) ++p;
  return p;
}

/* Skip over space characters. */
static char const *
tspTokenStart (char const *p)
{
  char c;
  while ((c = *p) && isspace (c)) ++p;
  return (c ? p : NULL);
}

/* Places the next token in <out>.  Assumes we are currently at the token
   end position.  Returns buffer pointer to start of next token or NULL if no  
   token available. */

static char const *
tspNextToken (char const *buf, char *out)
{
  char c;
  if (! (buf = tspTokenStart (buf)))
    return buf;
  for ( ; (c = *buf) && ! isspace (c); ++buf, ++out)
    *out = c;
  *out = '\0';
  return buf;
}

static char const *
tspNextSymbol (char const *buf, tspSymbol *out)
{
  char c, pbuf[128], *p;
  if (! (buf = tspTokenStart (buf)))
    return buf;
  for ( p = pbuf; (c = *buf) && ! isspace (c); ++buf, ++p)
    *p = c;
  *p = '\0';
  *out = tspSymbolFromString (pbuf, tspUnknownSymbol, tspNumSymbols);
  return buf;
}

/* Places the next token in <out>.  Assumes we are currently at the token
   start position.  Returns buffer pointer to start of next token. */

static char const *
tspNextInt (char const *buf, int *val)
{
  if (! (buf = tspTokenStart (buf)))
    return buf;
  *val = strtol (buf, (char **) &buf, 10);
  return buf;
}

static char const *
tspNextDouble (char const *buf, double *val)
{
  if (! (buf = tspTokenStart (buf)))
    return buf;
  *val = strtod (buf, (char **) &buf);
  return buf;
}

/*
 * Synopsis:
 *      int tspReadLineFromTspec( FILE *fp, char *buffer )
 *
 * Description:
 *      Reads in the next single line from the specified open file. 
 *      This routine will automatically skip over any lines that begin
 *      with a hash symbol (comment) or whitespace (blank line).
 *
 * Externals: None
 *
 * Returns: TRUE if a line was returned, FALSE if EOF was reached.
 *
 * Author: Martin Reddy
 * 
 * Date: 29 July 1997
 *
 */

int
tspReadLineFromTspec( FILE *fp, char *buffer )
{
  int empty_line;

  /* read the first valid (non-comment, non-blank) line */
  do {
    int i, last;

    empty_line = FALSE;
    if ( fgets( buffer, MAX_TSPEC_LINE_LENGTH, fp ) == NULL )
      return FALSE;

    /* check for empty string */
    last = strlen (buffer)-1;
    if (last < 0) {
      empty_line = TRUE;
      continue;
    }

    /* strip any carriage return from the end of the string */
    while ( last >= 0 && (buffer[last] == '\n' || buffer[last] == '\r'))
      buffer[last--] = '\0';
  
    /* check for empty line. */
    for ( i = 0; i <= last; ++i )
      if ( ! isspace (buffer[i]) )
	break;
    if (i > last)
      empty_line = TRUE;

  } while ( empty_line || buffer[0] == '#' );

  return TRUE;
}

/*
 * Synopsis:
 *      void tspExtractTokenAndData( char *buffer, char *token, char *data )
 *
 * Description:
 *      Takes a singe line read from a tspec file and returns pointers
 *      to a string containing the token part of the line (e.g. min-tile-tx)
 *      and to a string containing the data part of the token. E.g. if
 *      you pass "checksum -1", then token will point to "checksum" and
 *      data will point to "-1". N.B. the contents of buffer are modified.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 29 July 1997
 *
 */

void
tspExtractTokenAndData( char *buffer, char **token, char **data )
{
  char *scan;

  /* Check for the case where an empty string is passed */
  if ( strlen( buffer ) == 0 ) {
    *token = buffer;
    *data  = buffer;
    return;
  }

  /* skip over any initial whitespace characters in the input */

  *token = buffer;
  while ( **token == ' ' || **token == '\t' )
    (*token)++;

  /* the token is simply at the start of the string, but we want */
  /* to add a null-terminator after the end.                     */

  scan = *token;
  while ( *scan > ' ' ) 
    scan++;
  *scan = '\0';

  /* Now the start of the data is simply the first non-whitespace char */

  *data = ++scan;
  while ( **data > '\0' && **data <= ' ' )
    (*data)++;
}

/*
 * Synopsis:
 *      char *tspExtractNthString( char *inbuffer, char *outbuffer, int n )
 *
 * Description:
 *      Takes a string which contains a list of strings separated by
 *      one or more whitespace characters. Will return a pointer to
 *      the (null-terminated) 'n'th string in such a list. Used for
 *      parsing the lists of data contained in certain tspec fields.
 *      The routine will correctly handle quoted strings (e.g. "a b c"
 *      is one string) and will strip the quotes from the returned string.
 *      N.B. numbering begins at zero, i.e. the first string is n == 0.
 *      If n < 0, then behaviour will be as if n == 0.
 *
 * Externals: None
 *
 * Returns: Pointer to extracted string, NULL if no nth string
 *
 * Author: Martin Reddy
 * 
 * Date: 30 July 1997
 *
 */

char *
tspExtractNthString( char *inbuffer, char *outbuffer, int n )
{
  char *nth = outbuffer;
  int  strings = 0, quotes = FALSE;

  if ( inbuffer == NULL || outbuffer == NULL || inbuffer[0] == '\0' )
    return NULL;

  /* skip over all strings until we get to the start of the nth string */
  /* if we run in to a null-terminator at any point, then return NULL. */

  while ( strings < n ) {
    while ( *inbuffer > ' ' || quotes == TRUE ) {           /* skip string */
      if ( *inbuffer == '"' )
	quotes = !quotes;
      if ( *inbuffer == '\0' )
	return NULL;
      inbuffer++;
    }

    while ( *inbuffer > '\0' && *inbuffer <= ' ' )       /* skip whitespace */
      inbuffer++;

    if ( *inbuffer == '\0' )                        /* error if end of list */
      return NULL;

    strings++;
  }

  /* we've found the start of the nth string, so now let's copy this to */
  /* the outbuffer and then add a null-terminator to the end of it.     */
  /* We skip over any initial quote and do not output the end quote.    */

  if ( *inbuffer == '"' ) {
    inbuffer++;
    quotes = TRUE;
  }

  while ( ( *inbuffer > ' ' || quotes == TRUE ) && *inbuffer != '\0' ) {
    if ( *inbuffer == '"' )
      quotes = !quotes;
    else {
      *outbuffer = *inbuffer;
      outbuffer++;
    }
    inbuffer++;
  }

  *outbuffer = '\0';

  return nth;
}

/* A couple of convenience functions to return the 'n'th string in */
/* a list of strings as an integer, a floating point value, and a  */
/* tspSymbol. We also add a routine to return an strdup'd string.  */

int
tspExtractNthInt( char *inbuffer, char *outbuffer, int n )
{
  char *string = tspExtractNthString( inbuffer, outbuffer, n ); 
  return ( string == NULL ) ? 0 : atoi( string );
}

double
tspExtractNthFloat( char *inbuffer, char *outbuffer, int n )
{
  char *string = tspExtractNthString( inbuffer, outbuffer, n ); 
  return ( string == NULL ) ? 0.0 : atof( string );
}

tspSymbol
tspExtractNthSymbol( char *inbuffer, char *outbuffer, int n )
{
  char *string = tspExtractNthString( inbuffer, outbuffer, n );
  if ( string == NULL )
    return tspSymbolError;
  else
    return tspSymbolFromString (string, tspUnknownSymbol, tspNumSymbols);
}

char *
tspExtractNthStrdup( char *inbuffer, char *outbuffer, int n )
{
  char *string = tspExtractNthString( inbuffer, outbuffer, n );
  return ( string == NULL ) ? NULL : tsmMemManagerStrdup( string );
}

char *
tspExtractAllStrdup( char *inbuffer )
{
  char *string = NULL;
  if ( inbuffer ) string = tsmMemStrdup( inbuffer );
  if ( string ) tspStripQuotes( string );
  return string;
}

/*
 * Synopsis:
 *      void tspStripQuotes( char *string )
 *
 * Description:
 *      Performs an in-place modification of the specified string,
 *      stripping all spaces and quote (") characters from the start
 *      and end of the string.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April 1999
 *
 */

void
tspStripQuotes( char *string )
{
  char *pos = string;

  if ( string == NULL ) return;

  /* strip all quotes and spaces at the start of the string */

  while ( *pos == ' ' || *pos == '\"' )
    pos++;

  if ( pos != string )
    strcpy( string, pos );

  /* strip all quotes and spaces at the end of the string */

  pos = string + strlen( string );
  while ( pos > string && ( pos[-1] == ' ' || pos[-1] == '\"' ) )
    pos--;

  *pos = '\0';
}

/*
 * Synopsis:
 *      void tspParseCharArray( char *, char *, u_int *, charPtr ** )
 *
 * Description:
 *      Takes a string composed of a list of strings separated by
 *      whitespace and creates an array holding copies of each string.
 *      The length and array parameters are updated to hold the length
 *      of the array, and the array pointer itself.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 31 July 1997
 *
 */

void
tspParseCharArray( char *inbuffer, char *outbuffer, u_int *length, 
		   charPtr **array )
{
  const char *funcName = "tspParseCharArray()";
  int        n = 0, i;

  /* count how many strings we have in the inbuffer first of all */

  while ( tspExtractNthString( inbuffer, outbuffer, n ) != NULL )
    n++;

  /* now allocate an array for that many charPtr's */
  
  *array = (charPtr *) tsmMemManagerMalloc( sizeof(charPtr) * n );

  if ( *array == NULL ) {
    tsmError( funcName, "couldn't allocate array" );
    return;
  }

  /* now fill the array with copies of all of the strings */

  for ( i = 0; i < n; i++ ) {
    (*array)[i] = tspExtractNthStrdup( inbuffer, outbuffer, i );
  }

  *length = n;
  return;
}

/*
 * Synopsis:
 *      void tspParseSymbolArray( char *, char *, u_int *, tspSymbol ** )
 *
 * Description:
 *      Takes a string composed of a list of strings separated by
 *      whitespace and creates an array of tspSymbols, with each entry
 *      derived from the equivalent string entry in the string list.
 *      The length and array parameters are updated to hold the length
 *      of the array, and the array pointer itself.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 31 July 1997
 *
 */

void
tspParseSymbolArray( char *inbuffer, char *outbuffer, u_int *length, 
		     tspSymbol **array )
{
  const char *funcName = "tspParseSymbolArray()";
  int        n = 0, i;

  /* count how many strings we have in the inbuffer first of all */

  while ( tspExtractNthString( inbuffer, outbuffer, n ) != NULL )
    n++;

  /* now allocate an array for that many tspSymbol's */
  
  *array = (tspSymbol *) tsmMemManagerMalloc( sizeof(tspSymbol) * n );

  if ( *array == NULL ) {
    tsmError( funcName, "couldn't allocate array" );
    return;
  }

  /* now fill the array with tspSymbols derived from each string */

  for ( i = 0; i < n; i++ ) {
    (*array)[i] = tspExtractNthSymbol( inbuffer, outbuffer, i );
  }

  *length = n;
  return;
}

/*
 * Synopsis:
 *      void tspParseFloatArray( char *, char *, int *, float *, u_int )
 *
 * Description:
 *      Takes a string composed of a list of strings separated by
 *      whitespace and fills an array of floats, with each entry
 *      derived from the equivalent string entry in the string list.
 *      The length parameter and array contents are updated accordingly.
 *      N.B. The array will not be filled beyond the expected_size in
 *      order to prevent memory write overflows and also to check that
 *      the requisite number of floats have been read in. You can pass
 *      a length pointer of NULL if you don't want to return the array
 *      size in a variable.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 31 July 1997
 *
 */

void
tspParseFloatArray( char *inbuffer, char *outbuffer, int *length, 
		    float *array, u_int expected_size )
{
  const char *funcName = "tspParseFloatArray()";
  int        n = 0;

  /* We are not allocating an array here so we can just read in */
  /* each float and add it to the array in turn, checking for   */
  /* any overflow as we go...                                   */

  while ( tspExtractNthString( inbuffer, outbuffer, n ) != NULL ) {
    if ( n >= expected_size ) {
      tsmError( funcName, "Too many floats: %s", inbuffer );
      return;
    }
    array[n++] = atof( outbuffer );
  }

  /* All read in okay - do we have enough values? */

  if ( length != NULL )
    *length = n;
  if ( n < expected_size )
    tsmError( funcName, "Too few floats: %s", inbuffer );

  return;
}

void
tspParseIntArray( char *inbuffer, char *outbuffer, u_int *length, 
		  int **array, u_int expected_size )
{
  const char *funcName = "tspParseIntArray()";
  int        n = 0;

  /* allocate an array for the requested number of ints */
  
  *array = (int *) tsmMemManagerMalloc( sizeof(int) * expected_size );
  
  if ( *array == NULL ) {
    tsmError( funcName, "couldn't allocate array" );
    return;
  }

  /* We are not allocating an array here so we can just read in */
  /* each int and add it to the array in turn, checking for any */
  /* overflow as we go...                                       */

  while ( tspExtractNthString( inbuffer, outbuffer, n ) != NULL ) {
    if ( n >= expected_size ) {
      tsmError( funcName, "Too many ints: %s", inbuffer );
      return;
    }
    (*array)[n++] = atoi( outbuffer );
  }

  /* All read in okay - do we have enough values? */

  if ( length != NULL )
    *length = n;
  if ( n < expected_size )
    tsmError( funcName, "Too few ints: %s", inbuffer );

  return;
}

/* a macro to define a hashing of a string to speed parsing of keywords */
#define HASH_KEY(x,y,z) ((x)+((y)<<8)+((z)<<16))

/*
 * Synopsis:
 *      int tspParseTspecLine( char *buffer, Pyramid *pyramid )
 *
 * Description:
 *      Takes a string containing a single line from a tspec file and
 *      attempts to parse it and set any corresponding field of the
 *      passed pyramid structure with the appropriate value.
 *
 *      The only imposed structure is that the min-level-and-res* field
 *      should appear BEFORE the max-level-and-res* field, and that both
 *      of these should appear BEFORE the tile-set.tspec information for
 *      all of the individual pyramid levels. Of course, tspSaveTspec()
 *      writes out the information in line with these requirements.
 *
 * Externals: None
 *
 * Returns: TRUE if the line was parsed, FALSE if nothing was done with it
 *
 * Author: Martin Reddy
 * 
 * Date: 29 July 1997
 *
 */

int
tspParseTspecLine( char *buffer, Pyramid *pyramid )
{
  const char *funcName = "tspParseTspecLine()";
  char       *token, *data;
  char       temp[MAX_TSPEC_LINE_LENGTH];
  int        levels, lvl, i, parsed = FALSE;
  int        minLevel = pyramid->minLevel;

  SimpleTileSet    *tsLevel = pyramid->tspecLevel.val;
  LatLongFootprint *ll_foot;
  UTMFootprint     *utm_foot;
  GeoFootprint     *geo_foot;

  TSMTRACE ((TSMMASK|2, funcName, "\"%s\"", buffer));
  tspExtractTokenAndData( buffer, &token, &data );

  /* Now try to parse the line. We are a bit more clever here than a  */
  /* list of if/elseif's by creating a hash key from the first three  */
  /* characters of the keyword and using this in a switch() statement */

  switch ( HASH_KEY(token[0],token[1],token[2]) ) {

  case HASH_KEY('i','n','v'):

    if ( !strcmp( token, "inverse-resolution-scale-factor" ) ) {
      pyramid->invResScaleFactor = atof( data );
      parsed = TRUE;
    } else if ( !strcmp( token, "inverse-focal-length" ) ) {
      pyramid->projectionTspec.inverseFocalLength =
	tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('r','e','c'):

    if ( !strcmp( token, "rectification" ) ) {
      if ( ( data = tspExtractNthString( data, temp, 0 ) ) != NULL ) {
	if ( !strcmp( data, "lat-long" ) )
	  pyramid->rectification = TSM_LATLONG_COORDSYS;
	else if ( !strcmp( data, "utm" ) )
	  pyramid->rectification = TSM_UTM_COORDSYS;
	else if ( !strcmp( data, "none" ) )
	  pyramid->rectification = TSM_NONE_COORDSYS;
	else {
	  tsmError( funcName, "invalid rectification value: %s", data );
	  pyramid->rectification = TSM_NONE_COORDSYS;
	}
      }
      parsed = TRUE;
    } else if ( !strcmp( token, "rectification-grid" ) ) {
      if ( ( data = tspExtractNthString( data, temp, 0 ) ) != NULL ) {
	if ( strlen(data) > 0 ) {
	  if ( ( pyramid->rectificationGrid = tsmParseUrl(data) ) == NULL )
	    tsmError( funcName, "couldn't parse URL: %s", data );
	}
      }
      parsed = TRUE;
    }
    break;

  case HASH_KEY('m','i','n'):

    if ( !strcmp( token, "min-valid-level-and-resolution" ) ) {
      pyramid->minLevel      = tspExtractNthInt( data, temp, 0 );
      pyramid->minResolution = tspExtractNthFloat( data, temp, 1 );
      parsed = TRUE;
    } else if ( !strcmp( token, "min-pixel-value" ) ) {
      pyramid->pixelTspec.minVal = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "min-valid-pixel-ix" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].minValidPixel[0] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "min-valid-pixel-iy" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].minValidPixel[1] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "min-tile-tx" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ ) {
	tsLevel[lvl].minTile[0] = tspExtractNthInt (data, temp, i );
	TSMTRACE ((TSMMASK|2, funcName, "level %d minTile[X] = %d",
		   lvl, tsLevel[lvl].minTile[0]));
      }
      parsed = TRUE;
    } else if ( !strcmp( token, "min-tile-ty" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ ) {
	tsLevel[lvl].minTile[1] = tspExtractNthInt (data, temp, i);
	TSMTRACE ((TSMMASK|2, funcName, "level %d minTile[Y] = %d",
		   lvl, tsLevel[lvl].minTile[1]));
      }
      parsed = TRUE;
    }
    break;

  case HASH_KEY('m','a','x'):

    if ( !strcmp( token, "max-valid-level-and-resolution" ) ) {
      pyramid->maxLevel      = tspExtractNthInt( data, temp, 0 );
      pyramid->maxResolution = tspExtractNthFloat( data, temp, 1 );
      parsed = TRUE;

      /* once we know the min/max levels, allocate a SimpleTileSet array */
      /* we assume that we have already parsed the min level information */
      /* N.B. check that a suitable SimpleTileSetArray doesn't exist 1st */
      /* if it does, use it. If it is too small, then enlarge it.        */

      levels = pyramid->maxLevel - minLevel + 1;
      if ( pyramid->tspecLevel.val == NULL ) {
	if (! tsmMakeSimpleTileSetArray (&pyramid->tspecLevel, 
					 minLevel, pyramid->maxLevel)) {
	  tsmError( funcName, "Could not allocate simple tile set array" );
	}
      } else if ( pyramid->tspecLevel.len < levels ) {
	tsmEnlargeSimpleTileSetArray (&pyramid->tspecLevel, levels);
      }
    } else if ( !strcmp( token, "max-pixel-value" ) ) {
      pyramid->pixelTspec.maxVal = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "max-valid-pixel-ix" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].maxValidPixel[0] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "max-valid-pixel-iy" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].maxValidPixel[1] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "max-tile-tx" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ ) {
	tsLevel[lvl].maxTile[0] = tspExtractNthInt (data, temp, i);
	TSMTRACE ((TSMMASK|2, funcName, "level %d maxTile[X] = %d",
		   lvl, tsLevel[lvl].maxTile[0]));
      }
      parsed = TRUE;
    } else if ( !strcmp( token, "max-tile-ty" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ ) {
	tsLevel[lvl].maxTile[1] = tspExtractNthInt (data, temp, i);
	TSMTRACE ((TSMMASK|2, funcName, "level %d maxTile[Y] = %d",
		   lvl, tsLevel[lvl].maxTile[1]));
      }
      parsed = TRUE;
    }
    break;

  case HASH_KEY('t','i','l'):
 
    if ( !strcmp( token, "tile-set-description" ) ) {
      pyramid->description = tspExtractNthStrdup( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "tile-map" ) ) {
      tsLevel[minLevel].tileMapStatus = tspExtractNthSymbol (data, temp, 0);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('i','m','a'):
 
    if ( !strcmp( token, "image-hierarchy-file-name" ) ) {
      pyramid->imageHierarchyFileName = tspExtractNthStrdup( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "image-hierarchy-min-valid-level" ) ) {
      pyramid->imageHierarchyMinValidLevel = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "image-hierarchy-max-valid-level" ) ) {
      pyramid->imageHierarchyMaxValidLevel = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "image-file-name" ) ) {
      tsLevel[minLevel].imageFileName = tspExtractNthStrdup (data, temp, 0);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('n','a','m'):

    if ( !strcmp (token, "name") ) {
      pyramid->identityTspec.name = tspExtractNthStrdup( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','e','t'):

    if ( !strcmp (token, "set-id") ) {
      pyramid->identityTspec.setId = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','k','y'):

    if ( !strcmp (token, "sky") ) {
      pyramid->sky = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('t','w','o'):

    if ( !strcmp (token, "two-sided") ) {
      pyramid->twoSided = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('v','i','e'):

    if ( !strcmp (token, "view-from-below") ) {
      pyramid->fromBelow = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('c','h','e'):

    if ( !strcmp (token, "checksum-lrbt") ) {
      tspParseIntArray (data, temp,
			&pyramid->identityTspec.checkSumLrbt.len,
			&pyramid->identityTspec.checkSumLrbt.val,
			pyramid->tspecLevel.len + 1);
      pyramid->identityTspec.checkSumLrbt.off = minLevel;
      pyramid->identityTspec.checkSumLrbt.val -= minLevel;
      parsed = TRUE;
    } else if ( !strcmp (token, "checksum-jpeg") ) {
      tspParseIntArray (data, temp,
			&pyramid->identityTspec.checkSumJpeg.len,
			&pyramid->identityTspec.checkSumJpeg.val,
			pyramid->tspecLevel.len + 1);
      pyramid->identityTspec.checkSumJpeg.off = minLevel;
      pyramid->identityTspec.checkSumJpeg.val -= minLevel;
      parsed = TRUE;
    }
    break;

  case HASH_KEY('t','y','p'):

    if ( !strcmp( token, "type" ) ) {
      pyramid->identityTspec.type = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','u','b'):

    if ( !strcmp( token, "subtype" ) ) {
      pyramid->identityTspec.subtype = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','o','u'):

    if ( !strcmp( token, "source-type" ) ) {
      pyramid->identityTspec.source_type = tspExtractNthStrdup( data, temp, 0);
      parsed = TRUE;
    } else if ( !strcmp( token, "sources" ) ) {
      tspParseCharArray (data, temp,
			 &pyramid->identityTspec.sources.len,
			 &pyramid->identityTspec.sources.val);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('o','p','e'):

    if ( !strcmp( token, "operation-date" ) ) {
      pyramid->identityTspec.operationDate = tspExtractNthStrdup(data, temp,0);
      parsed = TRUE;
    } else if ( !strcmp( token, "operation-done-by" ) ) {
      pyramid->identityTspec.operationDoneBy =tspExtractNthStrdup(data,temp,0);
      parsed = TRUE;
    } else if ( !strcmp (token, "operations") ) {
      tspParseSymbolArray (data, temp,
			   &pyramid->identityTspec.operations.len,
			   &pyramid->identityTspec.operations.val);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('n','u','m'):

    if ( !strcmp( token, "num-pixels-per-tile-ix" ) ) {
      pyramid->tileTspec.numPixelsX = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "num-pixels-per-tile-iy" ) ) {
      pyramid->tileTspec.numPixelsY = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "num-pixels-overlap-per-tile-ix" ) ) {
      pyramid->tileTspec.overLap[0] = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "num-pixels-overlap-per-tile-iy" ) ) {
      pyramid->tileTspec.overLap[1] = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "num-components-per-pixel" ) ) {
      pyramid->tileTspec.numComponentsPixel = tspExtractNthInt( data, temp, 0);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-bytes-per-component" ) ) {
      pyramid->tileTspec.numBytesPerComponent = tspExtractNthInt(data, temp,0);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-tiles-tx" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numTile[0] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-tiles-ty" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numTile[1] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-pixels-ix" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numPixels[0] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-pixels-iy" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numPixels[1] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-valid-pixels-ix" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numValidPixel[0] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    } else if ( !strcmp( token, "num-valid-pixels-iy" ) ) {
      for ( lvl = minLevel, i = 0; lvl <= pyramid->maxLevel; lvl++, i++ )
	tsLevel[lvl].numValidPixel[1] = tspExtractNthInt (data, temp, i);
      parsed = TRUE;
    }
    break;
      
  case HASH_KEY('t','o','t'):

    if ( !strcmp( token, "total-num-pixels-per-tile-ix" ) ) {
      pyramid->tileTspec.totalPixels[0] = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "total-num-pixels-per-tile-iy" ) ) {
      pyramid->tileTspec.totalPixels[1] = tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','t','o'):

    if ( !strcmp( token, "storage-format" ) ) {
      pyramid->tileTspec.storageFormat = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('c','o','m'):
    
    if ( !strcmp( token, "component-number-type" ) ) {
      pyramid->tileTspec.componentNumberType =tspExtractNthSymbol(data,temp,0);
      parsed = TRUE;
    } else if ( !strcmp( token, "component-names" ) ) {
      tspParseSymbolArray (data, temp,
			   &pyramid->pixelTspec.componentNames.len,
			   &pyramid->pixelTspec.componentNames.val);
      parsed = TRUE;
    } else if ( !strcmp( token, "component-units" ) ) {
      pyramid->coordTspec.componentUnits = tspExtractNthSymbol( data, temp, 0);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','c','a'):
    
    if ( !strcmp( token, "scale" ) ) {
      pyramid->pixelTspec.scale = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('o','f','f'):

    if ( !strcmp( token, "offset" ) ) {
      pyramid->pixelTspec.offset = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('p','i','x'):
    
    if ( !strcmp( token, "pixel-scaling-type" ) ) {
      pyramid->pixelTspec.scalingType = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('p','l','a'):
    
    if ( !strcmp( token, "planet" ) ) {
      tspExtractNthSymbol( data, temp, 0 );
      pyramid->planet = tsmStringToPlanet( temp );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('p','r','o'):
    
    if ( !strcmp( token, "projection-type" ) ) {
      pyramid->projectionTspec.projectionType =
	tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('l','c','s'):
    
    if ( !strcmp( token, "lcs-to-uv-projective-matrix" ) ) {
      tspParseFloatArray( data, temp, NULL,
	 pyramid->projectionTspec.lcsToUvProjectiveMatrix, 16 );
      parsed = TRUE;
    } else if ( !strcmp( token, "lcs-to-geocentric-matrix" ) ) {
      tspParseFloatArray( data, temp, NULL, pyramid->coordTspec.lvcstoGeo, 16);
      parsed = TRUE;
    } else if ( !strcmp( token, "lcs-type" ) ) {
      pyramid->coordTspec.lcsType = tspExtractNthSymbol( data, temp, 0 );
      if ( pyramid->coordTspec.lcsType == tspNoneSubtype )
	pyramid->coordTspec.lcsType = tspNone;
      parsed = TRUE;
    }
    break;

  case HASH_KEY('u','v','a'):
    
    if ( !strcmp( token, "uva-to-lcs-matrix" ) ) {
      tspParseFloatArray( data, temp, NULL,
	pyramid->projectionTspec.uvaToLcsMatrix, 16 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('p','i','e'):
    
    if ( !strcmp( token, "piercing-point" ) ) {
      tspParseFloatArray( data, temp, NULL,
	pyramid->projectionTspec.piercingPoint, 2 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('c','o','e'):

    if ( !strcmp( token, "coeffs" ) ) {
      tspParseFloatArray( data, temp, &pyramid->projectionTspec.numCoeffs,
			  pyramid->projectionTspec.coeffs, 6 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('u','t','m'):

    if ( !strcmp( token, "utm-zone-number" ) ) {
      pyramid->projectionTspec.utmZoneNumber =
	tspExtractNthInt( data, temp, 0 );
      parsed = TRUE;
    } if ( !strcmp( token, "utm-footprint" ) ) {

      if ( tspExtractNthString( data, temp, 11 ) == NULL ) {
	tsmError( funcName, "Insufficient values for utm-footprint" );
	return FALSE;
      }

      utm_foot = &pyramid->footprint.utm;
      utm_foot->c1.e    = tspExtractNthFloat( data, temp, 0 );
      utm_foot->c1.n    = tspExtractNthFloat( data, temp, 1 );
      utm_foot->c1.zone = tspExtractNthInt( data, temp, 2 );
      utm_foot->c2.e    = tspExtractNthFloat( data, temp, 3 );
      utm_foot->c2.n    = tspExtractNthFloat( data, temp, 4 );
      utm_foot->c2.zone = tspExtractNthInt( data, temp, 5 );
      utm_foot->c3.e    = tspExtractNthFloat( data, temp, 6 );
      utm_foot->c3.n    = tspExtractNthFloat( data, temp, 7 );
      utm_foot->c3.zone = tspExtractNthInt( data, temp, 8 );
      utm_foot->c4.e    = tspExtractNthFloat( data, temp, 9 );
      utm_foot->c4.n    = tspExtractNthFloat( data, temp, 10 );
      utm_foot->c4.zone = tspExtractNthInt( data, temp, 11 );
      pyramid->footprint.coordsys = TSM_UTM_COORDSYS;

      parsed = TRUE;
    }
    break;

  case HASH_KEY('l','a','t'):

    if ( !strcmp( token, "latlong-footprint" ) ) {

      if ( tspExtractNthString( data, temp, 7 ) == NULL ) {
	tsmError( funcName, "Insufficient values for latlong-footprint" );
	return FALSE;
      }

      ll_foot = &pyramid->footprint.latlong;
      ll_foot->c1.lat = tspExtractNthFloat( data, temp, 0 );
      ll_foot->c1.lon = tspExtractNthFloat( data, temp, 1 );
      ll_foot->c2.lat = tspExtractNthFloat( data, temp, 2 );
      ll_foot->c2.lon = tspExtractNthFloat( data, temp, 3 );
      ll_foot->c3.lat = tspExtractNthFloat( data, temp, 4 );
      ll_foot->c3.lon = tspExtractNthFloat( data, temp, 5 );
      ll_foot->c4.lat = tspExtractNthFloat( data, temp, 6 );
      ll_foot->c4.lon = tspExtractNthFloat( data, temp, 7 );
      pyramid->footprint.coordsys = TSM_LATLONG_COORDSYS;

      parsed = TRUE;
    }
    break;

  case HASH_KEY('c','e','n'):

    if ( !strcmp( token, "central-meridian" ) ) {
      pyramid->projectionTspec.centralMeridian =
	tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('k','0','\0'):

    if ( !strcmp( token, "k0" ) ) {
      pyramid->projectionTspec.k0 = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('g','e','o'):

    if ( !strcmp( token, "geoid-separation" ) ) {
      pyramid->projectionTspec.geoidSeparation =
	tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "geocentric-to-lcs-matrix" ) ) {
      tspParseFloatArray( data, temp, NULL,
			  pyramid->coordTspec.geotoLVCS, 16 );
      parsed = TRUE;
    } if ( !strcmp( token, "geo-footprint" ) ) {

      if ( tspExtractNthString( data, temp, 11 ) == NULL ) {
	tsmError( funcName, "Insufficient values for geo-footprint" );
	return FALSE;
      }

      geo_foot = &pyramid->footprint.geo;
      geo_foot->c1.x = tspExtractNthFloat( data, temp, 0 );
      geo_foot->c1.y = tspExtractNthFloat( data, temp, 1 );
      geo_foot->c1.z = tspExtractNthFloat( data, temp, 2 );
      geo_foot->c2.x = tspExtractNthFloat( data, temp, 3 );
      geo_foot->c2.y = tspExtractNthFloat( data, temp, 4 );
      geo_foot->c2.z = tspExtractNthFloat( data, temp, 5 );
      geo_foot->c3.x = tspExtractNthFloat( data, temp, 6 );
      geo_foot->c3.y = tspExtractNthFloat( data, temp, 7 );
      geo_foot->c3.z = tspExtractNthFloat( data, temp, 8 );
      geo_foot->c4.x = tspExtractNthFloat( data, temp, 9 );
      geo_foot->c4.y = tspExtractNthFloat( data, temp, 10 );
      geo_foot->c4.z = tspExtractNthFloat( data, temp, 11 );
      pyramid->footprint.coordsys = TSM_GEO_COORDSYS;

      parsed = TRUE;
    } else if ( !strcmp( token, "geo-name" ) ) {
      pyramid->geoName = tspExtractNthStrdup( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('3','d','-'):

    if ( !strcmp( token, "3d-coordinate-system-type" ) ) {
      pyramid->coordTspec.systemType = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('s','p','h'):

    if ( !strcmp( token, "spheroid-name" ) ) {
      pyramid->coordTspec.spheroidName = tspExtractNthSymbol( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('e','q','u'):

    if ( !strcmp( token, "equatorial-radius" ) ) {
      pyramid->coordTspec.equatorialRadius = tspExtractNthFloat(data, temp, 0);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('p','o','l'):

    if ( !strcmp( token, "polar-radius" ) ) {
      pyramid->coordTspec.polarRadius = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('o','r','i'):

    if ( !strcmp( token, "origin-lat" ) ) {
      pyramid->coordTspec.originLat = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "origin-long" ) ) {
      pyramid->coordTspec.originLong = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    } else if ( !strcmp( token, "origin-altitude" ) ) {
      pyramid->coordTspec.originAltitude = tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('l','o','c'):

    if ( !strcmp( token, "local-units-per-meter" ) ) {
      pyramid->coordTspec.localUnitsPerMeter =
	tspExtractNthFloat( data, temp, 0 );
      parsed = TRUE;
    }
    break;

  case HASH_KEY('c','o','o'):

    if ( !strcmp( token, "coord-component-names" ) ) {
      tspParseSymbolArray (data, temp,
			   &pyramid->coordTspec.componentNames.len,
			   &pyramid->coordTspec.componentNames.val);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('u','v','-'):

    if ( !strcmp( token, "uv-to-ixiy-matrix" ) ) {
      tspParseFloatArray (data, temp, NULL,
	tsLevel[minLevel].imageTransformationTspec.uvtoIXIY, 9);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('i','x','i'):

    if ( !strcmp( token, "ixiy-to-uv-matrix" ) ) {
      tspParseFloatArray (data, temp, NULL,
	tsLevel[minLevel].imageTransformationTspec.ixiytoUV, 9);
      parsed = TRUE;
    }
    break;

  case HASH_KEY('w','r','a'):

    /* we explicitly calculate these values in tsmFillSimpleTileSets */

    if ( !strcmp( token, "wrap-lat" ) ) 
      parsed = TRUE;
    if ( !strcmp( token, "wrap-long" ) )
      parsed = TRUE;

    break;

  }


  return parsed;
}

/*
 * Synopsis:
 *      int tspReadTspecStream( FILE *tspecStream, Pyramid *pyramid )
 *
 * Description:
 *      Takes a file stream for a tspec file and reads in all of the tspec
 *      information for that pyramid, filling a Pyramid structure with
 *      all the data. This routine supports the old style tspec format
 *      as well as the new single-file tspec format used by TV-II.
 *      N.B. you must allocate the pyramid structure with tspMakePyramid()
 *      before calling this function. 
 *
 * Externals: None
 *
 * Returns: TRUE if operation was successful, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 * 
 * Date: 28 September 1999
 *
 */

int
tspReadTspecStream( FILE *tspecStream, Pyramid *pyramid )
{
  const char *funcName = "tspReadTspecStream()";
  char       buffer[MAX_TSPEC_LINE_LENGTH];
  int        tokens = 0;

  if ( tspecStream == NULL || pyramid == NULL ) 
    return FALSE;

  /* Set up default values for optional entries in the file */

  pyramid->planet = TSM_EARTH;

  /* Now, read each valid line from the file and try to parse it */

  while ( tspReadLineFromTspec( tspecStream, buffer ) == TRUE ) {
    if ( tspParseTspecLine( buffer, pyramid ) == FALSE ) {
      TSMTRACE(( TSMMASK|2, funcName, "Unknown tspec token: %s", buffer ));
    } else
      tokens++;
  }

  /* check that we did actually read some valid data from the file  */
  /* If we open the file and it's not a tspec (or there is an I/O   */
  /* error), then we would previously still return a result of TRUE */

  if ( tokens == 0 ) {
    tsmError( funcName, "No valid data in the tspec file!" );
    return FALSE;
  }

  /* fill in all of the SimpleTileSet structures and calculate the   */
  /* image transformation tspec for each level from the level 0 data */

  tsmFillSimpleTileSets( pyramid );

  /* Calculate the tile size for the pyramid and return the structure */

  pyramid->tileSize = pyramid->tileTspec.totalPixels[0] * 
    pyramid->tileTspec.totalPixels[1] * 
      pyramid->tileTspec.numComponentsPixel * 
	pyramid->tileTspec.numBytesPerComponent;

  return TRUE;
}

/*
 * Synopsis:
 *      int tspReadTspec( char *tspecName, Pyramid *pyramid )
 *
 * Description:
 *      Takes a tspec filename for a pyramid and reads in all of the tspec
 *      information for that pyramid, filling a Pyramid structure with
 *      all the data. This routine supports the old style tspec format
 *      as well as the new single-file tspec format used by TV-II.
 *      N.B. you must allocate the pyramid structure with tspMakePyramid()
 *      before calling this function. 
 *
 * Externals: None
 *
 * Returns: TRUE if operation was successful, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 * 
 * Date: 29 July 1997
 *
 */

int
tspReadTspec( char *tspecName, Pyramid *pyramid )
{
  const char *funcName = "tspReadTspec()";
  FILE       *fp;
  char       pathName[512], *slash;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Reading NEW tspec: %s", tspecName ));

  /* Check that the caller has not passed a NULL pyramid pointer */

  if ( pyramid == NULL ) {
    tsmError( funcName, "you must pass an allocated pyramid" );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* Try to open the new single-file format tspec file for reading.  */

  if ( ( fp = fopen( tspecName, "r" ) ) == NULL ) {
    tsmError( funcName, "Couldn't open new tspec (%s)", pathName );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* cut off the filename from tspecName to give the pathName */

  strcpy( pathName, tspecName );
  if ( ( slash = strrchr( pathName, '/' ) ) != NULL )
    *slash = '\0';

  /* take a copy of the pathName into the pyramid. The BasePurl */
  /* fields should be filled in by tsmGetPyramid().             */

  if ( ( pyramid->pathName = tsmMemManagerStrdup( pathName ) ) == NULL ) {
    tsmError( funcName, "Error in allocating pyramid->pathName" );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  /* vector off to the stream reading interface for tspec files */

  if ( tspReadTspecStream( fp, pyramid ) == FALSE ) {
    fclose( fp );
    TSMTRINC(( TSMMASK|2, -1 ));
    return FALSE;
  }

  fclose( fp );

  TSMTRACE(( TSMMASK|2, funcName, "Read the NEW tspec file okay" ));
  TSMTRINC(( TSMMASK|2, -1 ));
  return TRUE;
}

/*
 * Synopsis:
 *      int tspIsFileAValidTspec( char *filename )
 *
 * Description:
 *      performs a parse on the specified local file and returns a value
 *      of TRUE if the file was a valid, parseable tspec file. Otherwise
 *      a value of FALSE is returned. This is used by the web routines to
 *      read a tspec - we can copy a URL to a file, but this file could
 *      contain an HTTP error message, so we need to know if we have the
 *      actual data that we requested or if we have a file that says that.
 *      the file does not exist!
 *
 * Externals: None
 *
 * Returns: TRUE if file is a valid tspec, FALSE if not or operation failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 24 September 1997
 *
 */

int
tspIsFileAValidTspec( char *filename )
{
  Pyramid *pyramid;
  FILE    *fp;
  char    buffer[MAX_TSPEC_LINE_LENGTH];

  if ( ( fp = fopen( filename, "r" ) ) == NULL )
    return FALSE;

  /* create a dummy pyramid structure to fill in */

  if ( ( pyramid = tsmAllocTspec() ) == NULL )
    return FALSE;

  /* Now, read each valid line from the file and try to parse it */

  while ( tspReadLineFromTspec( fp, buffer ) == TRUE ) {
    if ( tspParseTspecLine( buffer, pyramid ) == FALSE ) {
      fclose( fp );
      tsmFreeTspec( pyramid );
      return FALSE;
    }
  }

  fclose( fp );
  tsmFreeTspec( pyramid );
  return TRUE;
}

/*
 * Synopsis:
 *      int tspReadPyramid( char *pathName, Pyramid *pyramid, ... ) 
 *
 * Description:
 *      A wrapper function to read the new or the old tspecs. This is done to
 *      maintain the API protocol and therefore allow older source code
 *      to transparently support the new single-file tspec organisation.
 *      The old tspReadPyramid() function accepted a third parameter,
 *      this can still be optionally provided for backward compatibility
 *      but it will be ignored. N.B. What used to be tspReadPyramid() has
 *      now been renamed to tspReadOldPyramid().
 *
 * Externals: None
 *
 * Returns: TRUE if operation was successful, FALSE if an error occurred.
 *
 * Author: Martin Reddy
 * 
 * Date: 30 July 1997
 *
 */

int
tspReadPyramid( char *pathName, Pyramid *pyramid, ... ) 
{
  PRFileInfo status;
  char   tspecFileName[512];

  /*** N.B. This function is now completely in tsmReadTspec_local ****/

  /* Try to open the new single-file format tspec file for reading.  */
  /* If we can't get a lock on this, then we assume that the pyramid */
  /* is using the old tspec file organisation so we default back to  */
  /* the old tspReadPyramid() function to read these files.          */

  tspTspecFileName( pathName, tspecFileName );
  if ( ( PR_GetFileInfo ( tspecFileName, &status ) ) == PR_FAILURE )
    return tspReadOldPyramid( pathName, pyramid, TRUE );
  else
    return tspReadTspec( tspecFileName, pyramid );
}
