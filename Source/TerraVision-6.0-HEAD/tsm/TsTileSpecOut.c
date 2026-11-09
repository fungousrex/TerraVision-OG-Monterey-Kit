/*
 * Name: TsTileSpecOut.c
 *
 * Description: Writes the Tspec (Tile Specification) files for the system.
 *      The main data structures are mapped pretty directly to the tspec files.
 *   	Very little extra processing is done to the data read in. Refer to the
 *	header file and the Tspec documentation for further information about
 *	the different data structures involved.
 *
 *      The tspSavePyramid() function will write out all of the tspec data
 *      for an entire pyramid to a single tiles.tspec file - the new file
 *      organisation for TV-II. If you want to access the old functionality
 *      of writing out many individual tspecs, then use tspSaveOldPyramid().
 *
 * Function List:
 *      void tspSavePyramid( char *pathName, Pyramid *pyramid )
 *      int  tspSaveTspec( char *pathName, Pyramid *pyramid )
 *
 * Dependencies:
 *
 * Revision History:
 *    September 1993 - initial version
 *    July 1997 - add support for the new single-file tspec format
 *
 *    $Id: TsTileSpecOut.c,v 1.21.6.1 2001/08/17 18:54:47 leei Exp $
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

static void tspSaveIdentityTspec_fp(FILE *, IdentityTspec *, Pyramid *);
void tspSaveTileTspec_fp(FILE *, TileTspec *);
static void tspSavePixelTspec_fp(FILE *, Pyramid *);
static void tspSaveProjectionTspec_fp(FILE *, ProjectionTspec *);
static void tspSaveCoordTspec_fp(FILE *, CoordTspec *, ProjectionTspec *);
static void tspSaveAllSimpleTspecs_fp(FILE *, Pyramid *);
static void tspSavePyramidTspec_fp(FILE *, Pyramid *);
static void tspSaveImageTransformationTspec_fp(FILE *,
					       ImageTransformationTspec *);

/*
 * identity.tspec
 */

static void
tspSaveIdentityTspec_fp( FILE *fp, IdentityTspec *tspec, Pyramid *pyramid )
{
  int  i, end;
  char *planet;

  tspLFToSpace(tspec->name);
  fprintf (fp, "name \"%s\"\n", tspec->name);
  /* fprintf (fp, "set-id %d\n", tspec->setId); */

  if ( pyramid != NULL ) {

    /* output the geo-name if it is set */

    if ( pyramid->geoName != NULL ) {
      tspLFToSpace(pyramid->geoName);
      fprintf (fp, "geo-name \"%s\"\n", pyramid->geoName);
    }

    /* output a lowercase string describing the planet */

    planet = tsmMemStrdup( tsmPlanetToString( pyramid->planet ) );
    planet[0] = tolower( planet[0] );
    fprintf( fp, "planet %s\n", planet );
    tsmMemFree( planet );
  }

  fprintf (fp, "checksum-lrbt");
  end = tspec->checkSumLrbt.off + tspec->checkSumLrbt.len;
  for ( i = tspec->checkSumLrbt.off; i < end; i++ ) {
    fprintf( fp, " %d", tspec->checkSumLrbt.val[i] );
  }
  fprintf( fp, "\n" );

  fprintf (fp, "checksum-jpeg");
  end = tspec->checkSumJpeg.off + tspec->checkSumJpeg.len;
  for ( i = tspec->checkSumJpeg.off; i < end; i++ ) {
    fprintf( fp, " %d", tspec->checkSumJpeg.val[i] );
  }
  fprintf( fp, "\n" );

  tspPrintSymbol (fp, tspec->type, "type %s\n");
  tspPrintSymbol (fp, tspec->subtype, "subtype %s\n");
  tspLFToSpace(tspec->source_type);
  fprintf( fp, "source-type \"%s\"\n", ( tspec->source_type == NULL ) ?
	   "unknown" : tspec->source_type );

  tspPrintListOfQuotedStrings (fp, 
			       tspec->sources.len, 
			       tspec->sources.val,
			       "sources ", "\n");
  tspPrintListOfSymbols (fp, 
			 tspec->operations.len, 
			 tspec->operations.val, 
			 "operations ", "\n");
  tspLFToSpace(tspec->operationDate);
  fprintf (fp, "operation-date \"%s\"\n", tspec->operationDate);
  tspLFToSpace(tspec->operationDoneBy);
  fprintf (fp, "operation-done-by \"%s\"\n", tspec->operationDoneBy);
}

/*
 * tile.tspec
 */

void
tspSaveTileTspec_fp( FILE *fp, TileTspec *tileTspec )
{
  fprintf (fp, "num-pixels-per-tile-ix %d\n", tileTspec->numPixelsX);
  fprintf (fp, "num-pixels-per-tile-iy %d\n", tileTspec->numPixelsY);
  fprintf (fp, "num-pixels-overlap-per-tile-ix %d\n", tileTspec->overLap[0]);
  fprintf (fp, "num-pixels-overlap-per-tile-iy %d\n", tileTspec->overLap[1]);
  fprintf (fp, "num-components-per-pixel %d\n", tileTspec->numComponentsPixel);
  fprintf (fp, "num-bytes-per-component %d\n", tileTspec->numBytesPerComponent);
  tspPrintSymbol (fp, tileTspec->componentNumberType, "component-number-type %s\n");
}

/*
 * pixel.tspec
 */

static void
tspSavePixelTspec_fp( FILE *fp, Pyramid *tspec )
{
  PixelTspec *pixelTspec = &tspec->pixelTspec;

  tspPrintListOfSymbols (fp, pixelTspec->componentNames.len, 
			 pixelTspec->componentNames.val, 
			 "component-names ", "\n");
  fprintf (fp, "min-pixel-value %g\n", pixelTspec->minVal);
  fprintf (fp, "max-pixel-value %g\n", pixelTspec->maxVal);
  tspPrintSymbol (fp, pixelTspec->scalingType, "pixel-scaling-type %s\n");
  fprintf (fp, "scale %g\n", pixelTspec->scale);
  fprintf (fp, "offset %g\n", pixelTspec->offset);
}

/*
 * projection.tspec
 */

static void
tspSaveProjectionTspec_fp( FILE *fp, ProjectionTspec *projectionTspec )
{
  int  i;

  tspPrintSymbol (fp, projectionTspec->projectionType, "projection-type %s\n");
  switch (projectionTspec->projectionType){
  case tspAffineProjection:
    fprintf (fp, "lcs-to-uv-projective-matrix ");
    tspPrintFloatVector (fp, 16, projectionTspec->lcsToUvProjectiveMatrix);
    fprintf (fp, "uva-to-lcs-matrix ");
    tspPrintFloatVector (fp, 16, projectionTspec->uvaToLcsMatrix);
    fprintf (fp, "inverse-focal-length %g\n",
	     projectionTspec->inverseFocalLength);
    fprintf (fp, "piercing-point %g %g\n", 
	     projectionTspec->piercingPoint[0], 
	     projectionTspec->piercingPoint[1]);
    if (projectionTspec->numCoeffs==6) {
      fprintf( fp, "coeffs" );
      
      for ( i = 0; i < 6; i++ )
	fprintf (fp, " %g", projectionTspec->coeffs[i]);
      fprintf (fp, "\n");
    }
    break;
  case tspUtmProjection:
    fprintf (fp, "utm-zone-number %d\n", projectionTspec->utmZoneNumber);
    fprintf (fp, "central-meridian %g\n",
	     projectionTspec->centralMeridian);
    fprintf (fp, "k0 %g\n", projectionTspec->k0);
    fprintf (fp, "geoid-separation %g\n",
	     projectionTspec->geoidSeparation);
    break;
  case tspLatLongProjection:
    break;
  default:
    fprintf (fp, "projection-type UNKOWN\n");
  }
}

/*
 * image-transformation.tspec
 */

static void
tspSaveImageTransformationTspec_fp( FILE *fp, ImageTransformationTspec *imageTransformationTspec)
{
  fprintf (fp, "uv-to-ixiy-matrix ");
  tspPrintFloatVector (fp, 9, imageTransformationTspec->uvtoIXIY);

  fprintf (fp, "ixiy-to-uv-matrix ");
  tspPrintFloatVector (fp, 9, imageTransformationTspec->ixiytoUV);
}

/*
 * 3d-coordinate-system.tspec
 */

/*
I don't understand why coordTspec->lvcstoGeo and coordTspec->geotoLVCS
appear to be exchanged!!
*/

static void
tspSaveCoordTspec_fp( FILE *fp, CoordTspec *coordTspec, ProjectionTspec *projTspec)
{
  tspPrintSymbol (fp, coordTspec->systemType, "3d-coordinate-system-type %s\n");
  tspPrintSymbol (fp, coordTspec->spheroidName, "spheroid-name %s\n");
  fprintf (fp, "equatorial-radius %1.10g\n",
	   coordTspec->equatorialRadius);
  fprintf (fp, "polar-radius %1.10g\n", coordTspec->polarRadius);
  fprintf (fp, "geocentric-to-lcs-matrix ");
  tspPrintFloatVector (fp, 16, coordTspec->lvcstoGeo);
  fprintf (fp, "lcs-to-geocentric-matrix ");
  tspPrintFloatVector (fp, 16, coordTspec->geotoLVCS);
  tspPrintSymbol (fp, coordTspec->lcsType, "lcs-type %s\n");

  if ( coordTspec->lcsType == tspLvcsCoordSystem ) {

    fprintf (fp, "origin-lat %g\n", coordTspec->originLat);
    fprintf (fp, "origin-long %g\n", coordTspec->originLong);
    fprintf (fp, "local-units-per-meter %g\n",
	     coordTspec->localUnitsPerMeter);
    tspPrintListOfSymbols (fp, 
			   coordTspec->componentNames.len, 
			   coordTspec->componentNames.val,
			   "coord-component-names ", "\n");
    tspPrintSymbol (fp, coordTspec->componentUnits, "component-units %s\n");

  } else if ( projTspec != NULL 
	      && projTspec->projectionType == tspLatLongProjection ) {

    fprintf (fp, "origin-lat %g\n", coordTspec->originLat);
    fprintf (fp, "origin-long %g\n", coordTspec->originLong);
    fprintf (fp, "origin-altitude %g\n", coordTspec->originAltitude);
    tspPrintSymbol (fp, coordTspec->componentUnits, "component-units %s\n");

  } else if ( coordTspec->lcsType != tspNone )
    fprintf (stderr, "Error, invalid lcs-type %d!\n", coordTspec->lcsType);

}

/*
 * all simple tile-set.tspec data for every level in a pyramid
 */

static void
tspSaveAllSimpleTspecs_fp( FILE *fp, Pyramid *pyramid )
{
  SimpleTileSet *tspec;
  int           i;
  int           minLevel = pyramid->minLevel;
  int           maxLevel = pyramid->maxLevel;

  tspec  = pyramid->tspecLevel.val;

  fprintf( fp, "min-tile-tx" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].minTile[0] );

  fprintf( fp, "\nmin-tile-ty" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].minTile[1] );

  fprintf( fp, "\nmax-tile-tx" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].maxTile[0] );
  
  fprintf( fp, "\nmax-tile-ty" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].maxTile[1] );

  /*
  fprintf( fp, "\nnum-tiles-tx" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numTile[0] );

  fprintf (fp, "\nnum-tiles-ty" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numTile[1] );
  */

  fprintf (fp, "\nnum-pixels-ix" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numPixels[0] );

  fprintf (fp, "\nnum-pixels-iy" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numPixels[1] );

  fprintf (fp, "\nmin-valid-pixel-ix" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].minValidPixel[0] );

  fprintf (fp, "\nmin-valid-pixel-iy" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].minValidPixel[1] );

  fprintf (fp, "\nmax-valid-pixel-ix" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].maxValidPixel[0] );

  fprintf (fp, "\nmax-valid-pixel-iy" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].maxValidPixel[1] );

  /*
  fprintf (fp, "\nnum-valid-pixels-ix" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numValidPixel[0] );

  fprintf (fp, "\nnum-valid-pixels-iy" );
  for ( i = minLevel; i <= maxLevel; i++ ) 
    fprintf( fp, " %d", tspec[i].numValidPixel[1] );
  */

  /* tspPrintSymbol( fp, tspec->tileMapStatus, "\ntile-map %s\n" ); */

  if ( tspec[minLevel].imageFileName != NULL )
    fprintf( fp, "\nimage-file-name \"%s\"", tspec[minLevel].imageFileName );

  fprintf( fp, "\n" );
}

/*
 * pyramid tile-set.tspec
 */

static void
tspSavePyramidTspec_fp( FILE *fp, Pyramid *tspec )
{
  fprintf (fp, "inverse-resolution-scale-factor %g\n",
	   tspec->invResScaleFactor);
  fprintf (fp, "min-valid-level-and-resolution %d %g\n",
	   tspec->minLevel, tspec->minResolution);
  fprintf (fp, "max-valid-level-and-resolution %d %g\n",
	   tspec->maxLevel, tspec->maxResolution);
  if (tspec->imageHierarchyFileName != NULL) {
    fprintf (fp, "image-hierarchy-min-valid-level %d\n",
	     tspec->imageHierarchyMinValidLevel);
    fprintf (fp, "image-hierarchy-max-valid-level %d\n",
	     tspec->imageHierarchyMaxValidLevel);
    fprintf (fp, "image-hierarchy-file-name \"%s\"\n",
	     tspec->imageHierarchyFileName);
  }
  if ( tspec->sky )
    fprintf (fp, "sky 1\n");
  if ( tspec->twoSided )
    fprintf (fp, "two-sided 1\n");
  if ( tspec->fromBelow )
    fprintf (fp, "view-from-below 1\n");

  if ( tspec->description != NULL ) {
    tspLFToSpace(tspec->description);
    fprintf (fp, "tile-set-description \"%s\"\n", tspec->description);
  } else
    fprintf (fp, "tile-set-description \"\"\n" );

}

/*
 *  rectification tspec
 */

static void
tspSaveRectificationTspec_fp( FILE *fp, Pyramid *pyramid )
{
  fprintf( fp, "rectification " );
  switch ( pyramid->rectification ) {
  case TSM_LATLONG_COORDSYS:
    fprintf( fp, "lat-long\n" );
    break;
  case TSM_UTM_COORDSYS:
    fprintf( fp, "utm\n" );
    break;
  default:
    fprintf( fp, "none\n" );
    fprintf( fp, "rectification-grid %s\n",
	     ( pyramid->rectificationGrid != NULL ) ?
	     pyramid->rectificationGrid->url : "\"\"" );
    break;
  }
}

/*
 * write out a comment to the new single-file tspec
 */

static void
tspWriteTspecComment( FILE *fp, char *header )
{
  fprintf( fp, "# ---- %s ----\n", header );
}

/*
 * Write the entire tspec file for the new single-file tspec organisation
 * used by TV-II. Proving a pathName of "stdout" will write all of the
 * tspec data to stdout rather than to a file on disk.
 */

int
tspSaveTspec( char *pathName, Pyramid *pyramid )
{
  /*static const  char funcName[] = "tspSaveTspec";*/
  IdentityTspec *id;
  FILE          *fp;
  int           use_stdout;
  char          tspecFileName[512];

  /* Open file (or stdout) for writing. Create directory if required */

  use_stdout = ( strcmp( pathName, "stdout" ) == 0 );

  if ( use_stdout == FALSE )
    tsmMakeDirIfNotExist( pathName, TSM_PATH_ONLY );

  strcpy( tspecFileName, pathName );

  if ( ( fp = tspOpenForWrite( tspecFileName, "tiles.tspec" ) ) == NULL ) {
    tspError( tspecFileName, "Couldn't save tspec!" );
    return FALSE;
  }

  /* Make sure that we do have a checksum arrays so that we will always   */
  /* write out something for the checksum and not just leave a blank line */

  id = &pyramid->identityTspec;

  if ( id->checkSumLrbt.val == NULL ) {
    if (! tsmMakeChecksum (&id->checkSumLrbt,
			   pyramid->minLevel, pyramid->maxLevel)) {
      return FALSE;
    }
  }

  if ( id->checkSumJpeg.val == NULL ) {
    if (! tsmMakeChecksum (&id->checkSumJpeg,
			   pyramid->minLevel, pyramid->maxLevel)) {
      return FALSE;
    }
  }

  /* Write out all of the tspecs that used to associated with the */
  /* base pyramid level                                           */

  tspWriteTspecComment( fp, "tile-set.tspec" );
  tspSavePyramidTspec_fp( fp, pyramid );
  tspWriteTspecComment( fp, "identity.tspec" );
  tspSaveIdentityTspec_fp( fp, &pyramid->identityTspec, pyramid );
  tspWriteTspecComment( fp, "tile.tspec" );
  tspSaveTileTspec_fp( fp, &pyramid->tileTspec );
  tspWriteTspecComment( fp, "pixel.tspec" );
  tspSavePixelTspec_fp( fp, pyramid );
  /* tspWriteTspecComment( fp, "rectification.tspec" ); */
  /* tspSaveRectificationTspec_fp( fp, pyramid ); */
  tspWriteTspecComment( fp, "projection.tspec" );
  tspSaveProjectionTspec_fp( fp, &pyramid->projectionTspec );

  /* the wrap-lat/long fields are for documentation only. They */
  /* are not read in but are computed automatically from other */
  /* data in the tspec file - see tsmFillSimpleTileSets().     */
  
  if ( pyramid->projectionTspec.projectionType == tspLatLongProjection ) {
    fprintf( fp, "wrap-lat \"%s\"\n", 
	     ( pyramid->wrap_lat ) ? "yes" : "no" );
    fprintf( fp, "wrap-long \"%s\"\n",
	     ( pyramid->wrap_lon ) ? "yes" : "no" );
  }

  tspWriteTspecComment( fp, "3d-coordinate-system.tspec" );
  tspSaveCoordTspec_fp( fp, &pyramid->coordTspec, &pyramid->projectionTspec );

  if ( pyramid->tspecLevel.val != NULL ) {

    /* write out the image-transformation.tspec for level 0 of the   */
    /* pyramid - this is assumed to be the same for all other levels */
    /* or can be generated on-the-fly when the tspec is read.        */
    
    tspWriteTspecComment( fp, "image-transformation.tspec" );
    tspSaveImageTransformationTspec_fp( fp,
       &pyramid->tspecLevel.val[pyramid->minLevel].imageTransformationTspec );

    /* And now write out all of the tile-set.tspec data for all of */
    /* the pyramid levels to the new single-file tspec.            */

    tspWriteTspecComment( fp, "tile-set.tspec (all levels)" );
    tspSaveAllSimpleTspecs_fp( fp, pyramid );

  }

  tspCloseForWrite( fp );
  return TRUE;
}

/*
 * A wrapper function for the new tspSaveTspec() function so that the
 * original API protocol can be used and hence backward compatibility
 * be maintained. Note that tspSavePyramid() discards the result of
 * tspSaveTspec(), so if you want to know if the operation was successful,
 * you should us the latter function.
 */

void
tspSavePyramid( char *pathName, Pyramid *pyramid )
{
  tspSaveTspec( pathName, pyramid );
}

