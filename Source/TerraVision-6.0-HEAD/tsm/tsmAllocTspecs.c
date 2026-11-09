/*
 * Name: tsmAllocTspecs.c
 *
 * Description: 
 *   Functions to allocate and free tspec data structures.
 *
 * Function List:
 *   Pyramid *tsmAllocTspec( void );
 *   Pyramid *tsmFreeTspec (Pyramid *);
 *   Pyramid *tsmAllocatePyramid( char *, int );
 *   Pyramid *tsmAllocTspec(void);
 *   SimpleTileSet *tsmMakeSimpleTileSetArray (int);
 *   int tsmEnlargeSimpleTileSetArray (int, Pyramid *);
 *   void tsmFillSimpleTileSets( Pyramid * );
 *   void tsmFreeSimpleTileSetArray (int, SimpleTileSet *);
 *   void tsmFreeIdentityTspecContents (IdentityTspec *);
 *   void tsmFreeSimpleTileSetContents (SimpleTileSet *);
 *
 * Revision History:
 *
 *   $Id: tsmAllocTspecs.c,v 1.25.6.2 2001/03/15 20:04:10 leei Exp $
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
 *      Pyramid *tsmAllocTspec( void )
 *
 * Doc++ Description: *//** 
 *      Allocate a pyramid.
 *      
 *      Allocate memory for a Pyramid tspec structure, initialised with all 
 *      fields set to 0/NULL.
 *
 * @param    None
 *
 * @return   Pointer to allocated Pyramid structure, or NULL if failed.
 *
 * @author   Yvan Leclerc
 * @version  29 July 1997
 *
 * @see      tsmFreeTspec, Pyramid
 *
 */

Pyramid *
tsmAllocTspec( void )
{
  static const char *funcName = "tsmAllocTspec";
  Pyramid           *pyramid;

  if ( ( pyramid = (Pyramid *) tsmMemCalloc( 1, sizeof(Pyramid) ) ) == NULL ) {
    tsmError (funcName, "Error allocating Pyramid.");
    return NULL;
  }
 
  return pyramid;
}

/*
 * Synopsis:
 *      void tsmFreeIdentityTspecContents( IdentityTspec *identityTspec )
 *
 * Description:
 *      Frees up any memory that has been allocated for the array 
 *      structures and strings of an identity.tspec data structure.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Yvan Leclerc
 * 
 * Date: 29 July 1997
 *
 */

void
tsmFreeIdentityTspecContents (IdentityTspec *identityTspec)
{
  int i;

  if ( identityTspec->sources.val != NULL ) {
    for (i = 0; i < identityTspec->sources.len; i++) {
      if (identityTspec->sources.val[i])
	tsmMemManagerFree (identityTspec->sources.val[i]);
    }
    tsmMemManagerFree( identityTspec->sources.val );
  }

  if ( identityTspec->checkSumLrbt.val != NULL )
    tsmOffsetArrayFree( identityTspec->checkSumLrbt );

  if ( identityTspec->checkSumJpeg.val != NULL )
    tsmOffsetArrayFree( identityTspec->checkSumJpeg );

  if ( identityTspec->operations.val != NULL )
    tsmMemManagerFree( identityTspec->operations.val );

  if ( identityTspec->name )
    tsmMemManagerFree( identityTspec->name );

  if ( identityTspec->source_type )
    tsmMemManagerFree( identityTspec->source_type );

  if ( identityTspec->operationDate )
    tsmMemManagerFree( identityTspec->operationDate );

  if ( identityTspec->operationDoneBy )
    tsmMemManagerFree( identityTspec->operationDoneBy );
}

/*
 * Synopsis:
 *      void tsmFreePixelTspecContents( PixelTspec *tspec )
 *
 * Description:
 *      Frees up any memory that has been allocated for the string
 *      field of a pixel.tspec data structure.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Yvan Leclerc
 * 
 * Date: 29 July 1997
 *
 */

void
tsmFreePixelTspecContents( PixelTspec *tspec )
{
  if ( tspec->componentNames.val != NULL )
    tsmMemManagerFree( tspec->componentNames.val );
}

/*
 * Synopsis:
 *      Pyramid *tsmFreeTspec( Pyramid *pyramid )
 *
 * Doc++ Description: *//** 
 *   Frees a pyramid tspec structure.
 *      
 *   Frees up all memory associated with a pyramid. This includes the
 *   array of the SimpleTileSets, any strings, and Purls, and any 
 *   data in the pyramid's pixel and identity tspecs. The memory for
 *   the Pyramid structure itself is also deallocated.
 *
 * @param    pyramid the dataset's tspec structure 
 *
 * @return   NULL
 *
 * @author   Martin Reddy
 * @version  13 March 1998
 *
 * @see      tsmAllocTspec, Pyramid, tsmFreePyramid
 *
 */

Pyramid *
tsmFreeTspec (Pyramid *pyramid)
{
  static char const funcName[] = "tsmFreeTspec";

  if ( pyramid == NULL ) return NULL;

  TSMTRACE ((TSMMASK|1, funcName, "%p", pyramid));

  /* free any arrays that have been allocated in the pyramid tspecs */

  tsmFreePixelTspecContents( &pyramid->pixelTspec );
  tsmFreeIdentityTspecContents( &pyramid->identityTspec );

  /* free the SimpleTileSet array for the pyramid */

  tsmFreeSimpleTileSetArray( &pyramid->tspecLevel );

  /* free any purls or strings that have been allocated for this pyramid */

  if ( pyramid->tspecBasePurl != NULL )
    tsmFreePurl( pyramid->tspecBasePurl );

  if ( pyramid->tileBasePurl != NULL )
    tsmFreePurl( pyramid->tileBasePurl );

  if ( pyramid->rectificationGrid != NULL )
    tsmFreePurl( pyramid->rectificationGrid );

  if ( pyramid->pathName != NULL )
    tsmMemManagerFree( pyramid->pathName );

  if ( pyramid->geoName != NULL )
    tsmMemManagerFree( pyramid->geoName );

  if ( pyramid->mapName != NULL )
    tsmMemManagerFree( pyramid->mapName );

  if ( pyramid->description != NULL )
    tsmMemManagerFree( pyramid->description );

  if ( pyramid->imageHierarchyFileName != NULL )
    tsmMemManagerFree( pyramid->imageHierarchyFileName );

  /* finally, free the pyramid structure itself */

  tsmMemManagerFree (pyramid);
  return NULL;
}

/*
 * Synopsis:
 *    int tsmCopyIdentityTspec( IdentityTspec *old_id, IdentityTspec *new_id )
 *
 * Description:
 *    Makes copies of all fields in an IdentityTspec structure, copying
 *    them into the relevant fields of another (already allocated)
 *    IdentityTspec structure. This function is used during the routines
 *    to copy a SimpleTileSet as well as an entire Pyramid.
 *
 * Returns: TRUE if nothing went wrong
 *
 * Author: Martin Reddy
 *
 * Date: 5 February 1998
 *
 */

int
tsmCopyIdentityTspec( const IdentityTspec *old_id, IdentityTspec *new_id )
{
  static const char *funcName = "tsmCopyIdentityTspec()";
  int               i;

  /* do a bit-by-bit copy first of all to get all the easy stuff */

  *new_id = *old_id;

  /* first duplicate all of the strings from the identity tspec */

  if ( old_id->name != NULL )
    new_id->name = tsmMemManagerStrdup( old_id->name );

  if ( old_id->source_type != NULL )
    new_id->source_type = tsmMemManagerStrdup( old_id->source_type );

  if ( old_id->operationDate != NULL )
    new_id->operationDate = tsmMemManagerStrdup( old_id->operationDate );

  if ( old_id->operationDoneBy != NULL )
    new_id->operationDoneBy = tsmMemManagerStrdup( old_id->operationDoneBy );

  /* now duplicate the checkSumLrbt array, if necessary */

  if ( old_id->checkSumLrbt.val != NULL ) {
    if (! tsmOffsetArrayDup(int, new_id->checkSumLrbt, old_id->checkSumLrbt)) {
      tsmError( funcName, "couldn't allocate checkSumLrbt array" );
      return FALSE;
    }
  }

  /* now duplicate the checkSumJpeg array, if necessary */

  if ( old_id->checkSumJpeg.val != NULL ) {
    if (! tsmOffsetArrayDup(int, new_id->checkSumJpeg, old_id->checkSumJpeg)) {
      tsmError( funcName, "couldn't allocate checkSumJpeg array" );
      return FALSE;
    }
  }

  /* now duplicate the sources array, if necessary */

  if ( old_id->sources.val != NULL ) {
    if ( ! tsmArrayDup( charPtr, new_id->sources, old_id->sources ) ) {
      tsmError( funcName, "couldn't allocate sources array" );
      return FALSE;
    }

    for ( i = 0; i < old_id->sources.len; ++i )
      new_id->sources.val[i] = tsmMemManagerStrdup( old_id->sources.val[i] );
  }

  /* and finally for the identity tspec duplicate the operations array */

  if ( old_id->operations.val != NULL ) {
    if ( ! tsmArrayDup( tspSymbol, new_id->operations, old_id->operations ) ) {
      tsmError( funcName, "couldn't allocate operations array" );
      return FALSE;
    }
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmCopySimpleTileSet( SimpleTileSet *old_set, SimpleTileSet *new_set )
 *
 * Description:
 *    Makes copies of all fields in an SimpleTileSet structure, copying
 *    them into the relevant fields of another (already allocated)
 *    SimpleTileSet structure. This function is used by the routine to  
 *    copy an entire Pyramid.
 *
 * Returns: TRUE if successful, FALSE if failed
 *
 * Author: Martin Reddy
 * 
 * Date: 16 September 1997
 *
 */

int
tsmCopySimpleTileSet( SimpleTileSet *old_set, SimpleTileSet *new_set )
{
  static const char *funcName = "tsmCopySimpleTileSet()";

  /* do a bit-by-bit copy first of all to get all the easy stuff */

  *new_set = *old_set;

  /* now duplicate the strings in the SimpleTileSet structure */

  if ( old_set->imageFileName != NULL )
    new_set->imageFileName = tsmMemManagerStrdup( old_set->imageFileName );

  if ( old_set->description != NULL )
    new_set->description = tsmMemManagerStrdup( old_set->description );

  /* The identity tspec has lots of pointers that we need to duplicate */

  tsmCopyIdentityTspec( &old_set->identityTspec, &new_set->identityTspec );

  /* The coord tspec has an array which we want to duplicate */

  if ( old_set->coordTspec.componentNames.val != NULL ) {
    if ( ! tsmArrayDup( tspSymbol, new_set->coordTspec.componentNames,
			old_set->coordTspec.componentNames ) ) {
      tsmError( funcName, "couldn't allocate coord tspec array" );
      return FALSE;
    }
  }

  /* And finally, the pixel tspec has an array to be duplicated */

  if ( old_set->pixelTspec.componentNames.val != NULL ) {
    if ( ! tsmArrayDup( tspSymbol, new_set->pixelTspec.componentNames,
			old_set->pixelTspec.componentNames ) ) {
      tsmError( funcName, "couldn't allocate pixel tspec array" );
      return FALSE;
    }
  }

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmCopySimpleTileSetLinked( Pyramid *pyramid, SimpleTileSet *old_set,
 *                                    SimpleTileSet *new_set )
 *
 * Description:
 *   Copies all of the SimpleTileSet information from one tileset to 
 *   another, but copies all of the tspecs for this new tileset from
 *   the root tspecs of a given pyramid structure. This is done so that
 *   all of the pointers in a SimpleTileSet can be made to point to
 *   the relevant fields in it's root pyramid to conserve memory.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 5 February 1998
 *
 */

void
tsmCopySimpleTileSetLinked( Pyramid *pyramid, SimpleTileSet *old_set,
			    SimpleTileSet *new_set )
{
  if ( pyramid == NULL || old_set == NULL || new_set == NULL )
    return;

  /* do a bit-by-bit copy first of all to get all the easy stuff */

  *new_set = *old_set;

  /* now duplicate the strings in the SimpleTileSet structure */

  if ( old_set->imageFileName != NULL )
    new_set->imageFileName = tsmMemManagerStrdup( old_set->imageFileName );

  if ( old_set->description != NULL )
    new_set->description = tsmMemManagerStrdup( old_set->description );

  /* Now just do a bit copy of the root Pyramid fields so that all */
  /* of the pointers for this SimpleTileSet point to these fields  */
  
  new_set->identityTspec   = pyramid->identityTspec;
  new_set->tileTspec       = pyramid->tileTspec;
  new_set->pixelTspec      = pyramid->pixelTspec;
  new_set->projectionTspec = pyramid->projectionTspec;
  new_set->coordTspec      = pyramid->coordTspec;
  
}

/*
 * Synopsis:
 *      Pyramid *tsmCopyPyramid( const Pyramid *pyramid, char *new_url )
 *
 * Doc++ Description: *//** 
 *      Copy a pyramid structure.
 *      
 *      Takes a pyramid structure and creates a copy of this, returning
 *      a pointer to the newly allocated structure. All of the strings,
 *      arrays, and other pointer fields are duplicated so that there are
 *      no common pointers between the two copies of the pyramid. As an
 *      option you can also specify a new URL with which to anchor the
 *      pyramid (specifying a new_url of NULL will just copy the url
 *      information from the source pyramid).
 *
 * @param    pyramid the pyramid to be copied
 * @param    new_url the base URL of the new pyramid (NULL means keep same)
 *
 * @return   the copied pyramid structure
 *
 * @author   Martin Reddy
 * @version  16 September 1997
 *
 * @see      tsmAllocTspec, tsmFreeTspec
 *
 */

Pyramid *
tsmCopyPyramid( const Pyramid *pyramid, char *new_url )
{
  static const char    *funcName = "tsmCopyPyramid()";
  SimpleTileSet *new_set, *old_set;
  Pyramid       *new_pyramid;
  int           i;

  /* allocate a new pyramid structure for the copy */

  if ( ( new_pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "couldn't allocate a new pyramid structure" );
    return NULL;
  }

  /* do a bit-by-bit copy of the structure first of all to ensure that */
  /* all of the non-pointer fields are copied okay.                    */

  memcpy( new_pyramid, pyramid, sizeof(Pyramid) );

  /* now make copies of each of the pointer fields in the pyramid struct */

  if ( pyramid->imageHierarchyFileName != NULL )
    new_pyramid->imageHierarchyFileName =
      tsmMemManagerStrdup( pyramid->imageHierarchyFileName );

  if ( pyramid->description != NULL )
    new_pyramid->description = tsmMemManagerStrdup( pyramid->description );
  
  if ( pyramid->geoName != NULL )
    new_pyramid->geoName = tsmMemManagerStrdup( pyramid->geoName );
  
  if ( pyramid->mapName != NULL )
    new_pyramid->mapName = tsmMemManagerStrdup( pyramid->mapName );
  
  /* the pixel tspec has an array to be duplicated */

  if ( pyramid->pixelTspec.componentNames.val != NULL ) {
    if ( ! tsmArrayDup( tspSymbol, new_pyramid->pixelTspec.componentNames,
			pyramid->pixelTspec.componentNames ) ) {
      tsmError( funcName, "couldn't allocate pixel tspec array" );
      return FALSE;
    }
  }

  /* The coord tspec has an array which we want to duplicate */

  if ( pyramid->coordTspec.componentNames.val != NULL ) {
    if ( ! tsmArrayDup( tspSymbol, new_pyramid->coordTspec.componentNames,
			pyramid->coordTspec.componentNames ) ) {
      tsmError( funcName, "couldn't allocate coord tspec array" );
      return FALSE;
    }
  }

  /* The identity tspec has lots of pointers that we need to duplicate */

  tsmCopyIdentityTspec( &pyramid->identityTspec, &new_pyramid->identityTspec );

  /* if we have been given a new URL for the pyramid structure, then we */
  /* want to fill in all of the relevant fields, otherwise we just copy */
  /* them across just like all the other fields.                        */

  if ( new_url == NULL || strcmp( new_url, "" ) == 0 ) {
    if ( pyramid->pathName != NULL )
      new_pyramid->pathName = tsmMemManagerStrdup( pyramid->pathName );
    if ( pyramid->tspecBasePurl != NULL )
      new_pyramid->tspecBasePurl = tsmCopyPurl( pyramid->tspecBasePurl );
    if ( pyramid->tileBasePurl != NULL )
      new_pyramid->tileBasePurl = tsmCopyPurl( pyramid->tileBasePurl );
  } else {
    Purl *purl;

    /* attempt to parse the new URL that we've been given */

    if ( ( purl = tsmParseUrl( new_url ) ) == NULL ) {
      tsmError( funcName, "could not parse URL - pyramid not changed" );
      return NULL;
    }

    new_pyramid->tspecBasePurl = purl;
    new_pyramid->tileBasePurl = tsmCopyPurl( purl );
    new_pyramid->pathName = 
      tsmMemManagerStrdup( new_pyramid->tspecBasePurl->uri );
  }
  
  /* now we want to create a new SimpleTileSet array and copy all of */
  /* the data across from the original pyramid to the new one.       */

  if ( pyramid->tspecLevel.val != NULL ) {

    if (! tsmOffsetArrayDup (SimpleTileSet, 
			     new_pyramid->tspecLevel,
			     pyramid->tspecLevel)) {
      tsmError( funcName, "could not allocate a SimpleTileSet array" );
      return NULL;
    }

    for ( i = pyramid->minLevel; i <= pyramid->maxLevel; i++ ) {
      
      old_set = &pyramid->tspecLevel.val[i];
      new_set = &new_pyramid->tspecLevel.val[i];

      /* We don't actually copy any of the dynamic memory in the  */
      /* SimpleTileSets of a pyramid. To conserve memory, we just */
      /* link all pointers to the relevant pointers for the same  */
      /* fields that exist in the root Pyramid structure, i.e.    */
      /* note that tsmFreeSimpleTileSet() does nothing.           */

      /* tsmCopySimpleTileSet( old_set, new_set ); */

      tsmCopySimpleTileSetLinked( new_pyramid, old_set, new_set );
    }

  }

  return new_pyramid;
}

/*
 * Synopsis:
 *      SimpleTileSet *tsmMakeSimpleTileSetArray( int numLevels )
 *
 * Description:
 *      Allocates memory for a specified (numLevels) number of
 *      SimpleTileSet structures
 *
 * Externals: None
 *
 * Returns: pointer to SimpleTileSet array, NULL if failed.
 *
 * Author: Yvan Leclerc
 * 
 * Date: 29 July 1997
 *
 */

SimpleTileSet *
tsmMakeSimpleTileSetArray (SimpleTileSetArray *arr, int minLevel, int maxLevel)
{
  static const char * funcName = "tsmMakeSimpleTileSetArray()";

  if (! tsmOffsetArrayCallocRange (SimpleTileSet, *arr, minLevel, maxLevel)) {
    tsmError (funcName, "Error allocating simple tile set!");
    return NULL;
  }
  return arr->val;
}

/*
 * Synopsis:
 *      void tsmFreeSimpleTileSetContents( SimpleTileSet *simpleTileSet )
 *
 * Description:
 *      Frees up all memory used by any of the fields/structures in a
 *      SimpleTileSet. This actually does nothing right now because,
 *      under the new system used for TV-II, we never allocate anything
 *      in a SimpleTileSet - all strings, arrays, etc. are kept in the
 *      Pyramid tspecs. Any pointers to strings and arrays that occur
 *      within a SimpleTileSet are copies of those in the Pyramid tspecs
 *      and so should never be deallocated here - tsmFreeTspec() will
 *      do that for you.
 *
 * Externals: None
 *
 * Returns: pointer to SimpleTileSet array, NULL if failed.
 *
 * Author: Yvan Leclerc
 * 
 * Date: 29 July 1997
 *
 */

void
tsmFreeSimpleTileSetContents (SimpleTileSet *simpleTileSet)
{
}

/*
 * Synopsis:
 *      void tsmFreeSimpleTileSetArray( int, SimpleTileSet * )
 *
 * Description:
 *      Frees up all memory used by a SimpleTileSet array, including the
 *      memory for the array itself.
 *
 * Externals: None
 *
 * Returns: pointer to SimpleTileSet array, NULL if failed.
 *
 * Author: Yvan Leclerc
 * 
 * Date: 29 July 1997
 *
 */

void
tsmFreeSimpleTileSetArray (SimpleTileSetArray *arr)
{
  int i;
  int off = arr->off;
  int end = arr->len + off;

  for (i = off; i <= end; i++)
    tsmFreeSimpleTileSetContents (&arr->val[i]);

  tsmOffsetArrayFree (*arr);
}

/*
 * Synopsis:
 *      int tsmEnlargeSimpleTileSetArray(SimpleTileSetArray *tspecLevel,
 *                                       int numLevels)
 *
 * Description:
 *      Takes a pyramid structure and enlarges the SimpleTileSet array so
 *      that it contains at least 'numLevels' levels. If the present array is
 *      smaller than this value then a larger memory chunk will be allocated
 *      and all of the current tileset information copied to this new chunk
 *      of memory. Any new tilesets will be initialised as if allocated
 *      using calloc().
 *
 * Externals: None
 *
 * Returns: TRUE if operation successful, FALSE if not enough memory
 *
 * Author: Martin Reddy
 * 
 * Date: 15 July 1997
 *
 */

int
tsmEnlargeSimpleTileSetArray (SimpleTileSetArray *tspecLevel, int numLevels)
{
  static const char *funcName = "tsmEnlargeSimpleTileSetArray()";
  SimpleTileSet *tilesets;
  int           origLevels = tspecLevel->len;

  /* if the requested size is smaller than the current size then */
  /* we don't do anything at all. (This isn't a failure though.) */

  if ( origLevels >= numLevels )
    return TRUE;

  /* Try to reallocate some memory with enough space to hold all */
  /* of the requested number of SimpleTileSets. Realloc will     */
  /* automatically copy the existing data to the new location.   */

  tilesets = tsmMemManagerRealloc( &tspecLevel->val[tspecLevel->off],
				   numLevels * sizeof(SimpleTileSet) );

  if ( tilesets == NULL ) {
    tsmError( funcName, "couldn't realloc simple tile set array" );
    return FALSE;
  }

  /* all worked okay, so now we want to initialise all of the new */
  /* SimpleTileSets that have been added to the array.            */

  tspecLevel->val = tilesets;
  tspecLevel->len = numLevels;
  memset( &tspecLevel->val[origLevels], 0,
	  sizeof(SimpleTileSet) * (numLevels - origLevels) );
  tspecLevel->val -= tspecLevel->off;

  return TRUE;
}


/*
 * Synopsis:
 *      tsmAllocatePyramid( char *baseUrl, int levels );
 *
 * Description:
 *      Makes a pyramid structure, allocating a SimpleTileSet array with
 *      "levels" entries, and fills the basePurl fields of the Pyramid.
 *      This is different from MakePyramid() which just allocs the Pyramid
 *      structure itself but doesn't allocate the SimpleTileSet array.
 *      The returned pyramid can be freed with tsmFreeTspec() as usual.
 *      N.B. the pixel and identity tspec arrays are NOT allocated - you
 *      should perform this yourself on a need-to-use basis.
 *
 *      If you don't know the Pyramid's URL yet, then you can pass a NULL
 *      or "" for baseUrl and the routine will not attempt to fill in any
 *      of the Purl/pathname fields of the pyramid.
 *
 * Externals: None
 *
 * Returns: Pointer to allocated Pyramid structure, or NULL if failed.
 *
 * Author: Martin Reddy
 * 
 * Date: 15 July 1997
 *
 */

Pyramid*
tsmAllocatePyramid( char *baseUrl, int minLevel, int maxLevel )
{
  static const char *funcName = "tsmAllocatePyramid()";
  Pyramid           *pyramid;

  /* Allocate memory for a pyramid structure */

  if ( ( pyramid = tsmAllocTspec() ) == NULL ) {
    tsmError( funcName, "Could not allocate pyramid structure" );
    return NULL;
  }

  /* Allocate memory for a sufficiently sized array of SimpleTileSets */

  if (! tsmMakeSimpleTileSetArray (&pyramid->tspecLevel, minLevel, maxLevel)) {
    tsmError( funcName, "Could not allocate simple tile set array" );
    return NULL;
  }

  /* Generate the tspec and tile basePurls - currently the same */

  if ( baseUrl != NULL && baseUrl[0] != '\0' ) {
    pyramid->tspecBasePurl = tsmParseUrl( baseUrl );
    pyramid->tileBasePurl  = tsmCopyPurl( pyramid->tspecBasePurl );
    pyramid->pathName = tsmMemManagerStrdup( pyramid->tspecBasePurl->uri );
  }

  return pyramid;
}

/*
 * Synopsis:
 *      void tsmFillSimpleTileSets( Pyramid *pyramid )
 *
 * Description:
 *      Copies all common tspec data from the main Pyramid structure
 *      into all of its SimpleTileSet structures. Each SimpleTileSet
 *      structure can contain various tspecs, but most of these aren't
 *      used any more - to ensure compatibility, this routine will fill
 *      all of these tspecs with the same values from the base pyramid
 *      tspecs. N.B. any space for arrays and strings are not copied:
 *      the same pointer will be used.
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
tsmFillSimpleTileSets( Pyramid *pyramid )
{
  static const char *funcName = "tsmFillSimpleTileSets()";

  ImageTransformationTspec *src_xform, *dest_xform;
  ProjectionTspec          *src_proj;
  SimpleTileSet            *src_tileset, *dest_tileset;
  TileTspec                *src_tile;
  float                    scale;
  int                      i;
  int                      minLevel = pyramid->minLevel;
  int                      maxLevel = pyramid->maxLevel;

  /* get the SimpleTileSet array and its size - check the pointers too */

  if ( pyramid == NULL || ( src_tileset = pyramid->tspecLevel.val ) == NULL )
    return;

  /* work out the total-num-pixels-* fields for the pyramid Tiletspec */
  /* as we no longer provide this information in the tspec file       */

  src_tile = &pyramid->tileTspec;

  src_tile->totalPixels[0] = src_tile->numPixelsX + src_tile->overLap[0];
  src_tile->totalPixels[1] = src_tile->numPixelsY + src_tile->overLap[1];

  /* negative northings were once used to represent UTM points in the   */
  /* southern hemisphere. Better to use a negative zone number for this */
  /* so that we don't need to deal with negative coords later on.       */

  /*
  src_xform = &src_tileset[minLevel].imageTransformationTspec;
  src_proj  = &pyramid->projectionTspec;

  if ( src_proj->projectionType == tspUtmProjection &&
       src_xform->ixiytoUV[5] < 0.0 ) {

    src_xform->ixiytoUV[5] = -src_xform->ixiytoUV[5];
    if ( src_proj->utmZoneNumber > 0 )
      src_proj->utmZoneNumber = -src_proj->utmZoneNumber;

    TSMTRACE ((TSMMASK|2, funcName, "set zone to %d and proj to %f %f",
	       src_proj->utmZoneNumber, src_xform->ixiytoUV[2],
	       src_xform->ixiytoUV[5]));
  }
  */
  
  /* go through each SimpleTileSet in the array and copy everything over */

  for ( i = minLevel; i <= maxLevel; i++ ) {

    /* copy all of the pyramid tspecs to the current SimpleTileSet */

    dest_tileset = &src_tileset[i];

    memcpy( &dest_tileset->identityTspec, &pyramid->identityTspec,
	    sizeof(IdentityTspec) );

    memcpy( &dest_tileset->tileTspec, &pyramid->tileTspec,
	    sizeof(TileTspec) );

    memcpy( &dest_tileset->pixelTspec, &pyramid->pixelTspec,
	    sizeof(PixelTspec) );

    memcpy( &dest_tileset->projectionTspec, &pyramid->projectionTspec,
	    sizeof(ProjectionTspec) );

    memcpy( &dest_tileset->coordTspec, &pyramid->coordTspec,
	    sizeof(CoordTspec) );

    /* work out the num-* fields of the tile-set.tspec for this level */
    /* as these are no longer provided explicitly in the tspec file   */

    dest_tileset->numTile[0] =
      dest_tileset->maxTile[0] - dest_tileset->minTile[0] + 1;

    dest_tileset->numTile[1] =
      dest_tileset->maxTile[1] - dest_tileset->minTile[1] + 1;

    dest_tileset->numValidPixel[0] =
      dest_tileset->maxValidPixel[0] - dest_tileset->minValidPixel[0] + 1;

    dest_tileset->numValidPixel[1] =
      dest_tileset->maxValidPixel[1] - dest_tileset->minValidPixel[1] + 1;

    /* generate the image-transformation tspec for this level */
    /* on the fly from the level 0 image-transformation data. */

    dest_xform = &dest_tileset->imageTransformationTspec;

    if ( i > minLevel ) {

      src_xform  = &src_tileset[minLevel].imageTransformationTspec;
      scale      = pow2i(i-minLevel);

      TSMTRACE ((TSMMASK|2, funcName, "Copy xform src[%d]->dest[%d] scaled %g",
		 pyramid->minLevel, i, scale));

      /* copy the level 0 ixiy->uv matrix and scale it appropriately */

      memcpy( dest_xform, src_xform, sizeof(ImageTransformationTspec) );

      dest_xform->ixiytoUV[0] *= scale;
      dest_xform->ixiytoUV[4] *= scale;

      /* the uv->ixiy matrix is simply the inverse of the ixiy->uv one */

      if ( !tsmInverse3x3( dest_xform->ixiytoUV, dest_xform->uvtoIXIY ) ) {
	tsmError( funcName, "No inverse of ixiytoUV for level %d", i );
      }

    }

    TSMTRACE ((TSMMASK|2, funcName, "ixiytoUV [%g,%g,%g %g,%g,%g %g,%g,%g]",
	       dest_xform->ixiytoUV[0], dest_xform->ixiytoUV[1],
	       dest_xform->ixiytoUV[2], dest_xform->ixiytoUV[3],
	       dest_xform->ixiytoUV[4], dest_xform->ixiytoUV[5],
	       dest_xform->ixiytoUV[6], dest_xform->ixiytoUV[7],
	       dest_xform->ixiytoUV[8]));
  }

  /* work out the wrap_lat and wrap_lon fields. These are used to */
  /* define whether the dataset wraps around the earth vertically */
  /* and/or horizontally, respectively. This only makes sense for */
  /* lat/long projection datasets as these are global.            */

  if ( pyramid->projectionTspec.projectionType == tspLatLongProjection ) {
    pyramid->wrap_lat = pyramid->minResolution * 
      pyramid->tspecLevel.val[minLevel].numValidPixel[1] > 179.0;
    pyramid->wrap_lon = pyramid->minResolution * 
      pyramid->tspecLevel.val[minLevel].numValidPixel[0] > 359.0;
  } else
    pyramid->wrap_lat = pyramid->wrap_lon = FALSE;

}

/*
 * Synopsis:
 *      int *tsmMakeChecksum( tsmChecksumArray *, int, int )
 *
 * Description:
 *      allocates a checksum array for a pyramid. You need to provide the
 *      number of levels that exist in the pyramid so that the correct 
 *      amount of space is allocated. The returned memory will be
 *      initialised to contain all -1, which signifies that no checksum
 *      has yet been calculated.
 *
 * Externals: None
 *
 * Returns: the pointer to the memory that was allocated and initialised
 *
 * Author: Martin Reddy
 * 
 * Date: 3 September 1997
 *
 */

int *
tsmMakeChecksum (tsmChecksumArray *arr, int minLevel, int maxLevel)
{
  static const char *funcName = "tsmMakeChecksum()";
  int i;

  if (! tsmOffsetArrayMallocRange (int, *arr, minLevel, maxLevel+1)) {
    tsmError (funcName, "Error allocating checksum array.");
    return NULL;
  }

  for ( i = minLevel; i <= maxLevel+1; i++ ) {
    arr->val[i] = -1;
  }
 
  return arr->val;
}
