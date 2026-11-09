/*
 * Name: tsmReadRowsDEM.c
 *
 * Description: 
 *   This module is to be used with tsmDownSample_inmemory for
 *   generation of TerraVision dataset from USGS DEM data file. The
 *   function tsmReadRowsDEM_read is to be hooked up with
 *   tsmDownSample_inmemory for this purpose.
 *
 *   The initialization function, tsmReadRowsDEM_new, will read in a
 *   7.5-minute DEM data file into memory, and resample the data into
 *   a regular grid before writing out to a temporary
 *   file. Subsequently, tsmReadRowsDEM_read will simply read from
 *   this file for the rows of data requested.
 *
 *   Note: Several functions, especially the ones dealing with reading
 *   actual USGS DEM file content, are taken from Dr. Martin Reddy's
 *   DEM to GeoElevationGrid conversion utility program. The original
 *   source can be found at http://www.ai.sri.com/~reddy/geovrml/dem2geoeg
 *
 * Function List:
 *
 * Revision History:
 *   Created 9 Aug 2000 by Yong Tze Chi <chi@ai.sri.com>
 *
 *   $Id: tsmReadRowsDEM.c,v 1.11.2.1 2002/03/12 04:26:15 reddy Exp $
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

#include <limits.h>
#include "tsmInternal.h"
#include "tsmReadRowsDEM.h"


static int tsmReadRowsDEM_saveDEMAsPBM(TsmReadRowsDEM *this, char *filename,
				       int type);
static int tsmReadRowsDEM_saveDEMAsRAW(TsmReadRowsDEM *this, char *filename);
static int tsmReadRowsDEM_readDEMHeader(TsmReadRowsDEM *this);
static int tsmReadRowsDEM_readDEM(TsmReadRowsDEM *this);
static int tsmReadRowsDEM_readDEM75(TsmReadRowsDEM *this);
static int tsmReadRowsDEM_readDEM30(TsmReadRowsDEM *this);
static int tsmReadRowsDEM_readDEM1(TsmReadRowsDEM *this);
static void tsmReadRowsDEM_stitchDEMEdges(TsmReadRowsDEM *this);
static void tsmReadRowsDEM_printDEMInfo(TsmReadRowsDEM *this);

#ifndef CHECK_NOT_NULL0
#define CHECK_NOT_NULL0(this) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return; }
#endif

#ifndef CHECK_NOT_NULL
#define CHECK_NOT_NULL(this, rv) if ((this)==NULL) { tsmError(funcName, "NULL instance"); return (rv); }
#endif


/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_new                                               | *
 * | Description:                                                           | *
 * |   Creates a new instance of TsmReadRowsDEM, and initializes all the    | *
 * |   parameters needed. Also calls the tsmReadRowsDEM_readDEM function    | *
 * |   to create a regular grid DEM array. The regular DEM array is then    | *
 * |   written out to a temporary file, where subsequent                    | *
 * |   tsmReadRowsDEM_read calls will access.                               | *
 * | Parameters:                                                            | *
 * |   char *filename     File name of the USGS DEM file.                   | *
 * | Return value:                                                          | *
 * |   Pointer to TsmReadRowsDEM instance.                                  | *
 * |                                                                        | *
 * | Author: Yong Tze Chi                                                   | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
TsmReadRowsDEM *tsmReadRowsDEM_new(char const *filename) {
  static const char *funcName = "tsmReadRowsDEM_new";
  TsmReadRowsDEM *this;

  if ((this = tsmMemMalloc(sizeof(TsmReadRowsDEM))) == NULL) {
    tsmError(funcName, "Failed to allocate memory for TsmReadRowsDEM instance.");
    return NULL;
  }

  if ((this->fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Failed to open file named %s.", filename);
    tsmReadRowsDEM_destroy(&this);
    return NULL;
  }

  if ((this->demInfo = tsmMemMalloc(sizeof(DEMInfo))) == NULL) {
    tsmError(funcName, "Failed to allocate memory for DEMInfo structure.\n");
    tsmReadRowsDEM_destroy(&this);
    return NULL;
  }

  if (!tsmReadRowsDEM_readDEMHeader(this)) {
    tsmError(funcName, "Failed to read in 7.5-minute DEM header");
    return NULL;
  }

  if (!tsmReadRowsDEM_readDEM75(this)) {
    tsmError(funcName, "Failed to read in 7.5-minute DEM data.");
    tsmReadRowsDEM_destroy(&this);
    return NULL;
  }

  tsmReadRowsDEM_stitchDEMEdges(this);

  if (!tsmReadRowsDEM_saveDEMAsRAW(this, "demdownsample_tmp.raw")) {
    tsmError(funcName, "Failed to save processed DEM data into temporary file.");
    tsmReadRowsDEM_destroy(&this);
    return NULL;
  }

  fclose(this->fin);
  tsmMemFree(this->buffer);
  this->buffer = NULL;

  this->fin = fopen("demdownsample_tmp.raw", "rb");

  this->compsPerPixel = 1;
  this->bytesPerComp = sizeof(short);
 
  return this;
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_destroy                                           | *
 * | Description:                                                           | *
 * |   Cleans up and set reference variable to NULL.                        | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM **this    Pointer to TsmReadRowsDEM reference         | *
 * |                            variable.                                   | *
 * | Return value:                                                          | *
 * |   None.                                                                | *
 * |                                                                        | *
 * | Author: Yong Tze Chi                                                   | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
void tsmReadRowsDEM_destroy(TsmReadRowsDEM **this) {
  if (this!=NULL && *this!=NULL) {
    if ((*this)->fin != NULL) fclose((*this)->fin);
    system("rm -f demdownsample_tmp.raw");
    if ((*this)->buffer != NULL) tsmMemFree((*this)->buffer);
    tsmMemFree(*this);
    *this = NULL;
  }
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_readDouble                                        | *
 * | Description:                                                           | *
 * |   Reads a single 24-byte double-precision floating point number from a | *
 * |   DEM file.                                                            | *
 * | Parameters:                                                            | *
 * |   FILE *fin    File handle to read double from.                        | *
 * | Return value:                                                          | *
 * |   Double read from the file.                                           | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static double tsmReadRowsDEM_readDouble(FILE *fin) {
  char buffer[32], *exp_sym;
  fscanf(fin, "%24c", buffer);

  /* convert any D+ style exponentials to E+ */
  exp_sym = strchr( buffer, 'D' );
  if ( ! exp_sym ) exp_sym = strchr( buffer, 'd' );
  if ( exp_sym )  *exp_sym = 'E';

  return strtod(buffer, (char **) NULL);
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_readDEMHeader                                     | *
 * | Description:                                                           | *
 * |   Reads the DEM data elements for the Logical Record Type A. This is   | *
 * |   the header at the start of the DEM file that contains all of the     | *
 * |   metadata about the elevation data.                                   | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM *this    TsmReadRowsDEM instance.                     | *
 * | Return value:                                                          | *
 * |   TRUE if succesful, FALSE if there's any error.                       | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static int tsmReadRowsDEM_readDEMHeader(TsmReadRowsDEM *this) {
  static const char *funcName = "tsmReadRowsDEM_readDEMHeader";
  int 	 i, j;
  int    array_size;
  
  /* read the first five data elements in the DEM file */

  fscanf( this->fin, "%144c%6d%6d%6d%6d",
    this->demInfo->mapLabel, 						 /* quadrangle name field */
    &this->demInfo->DEMlevel,						 /* DEM level code */
    &this->demInfo->elevationPattern,		 /* pattern code */
    &this->demInfo->groundSystem,				 /* planimetric code */
    &this->demInfo->groundZone );				 /* UTM zone */
  
  /* there is no magic number to identify DEM files, but we can  */
  /* at least check to see if we have valid values for the first */
  /* few fields to check that we have been given a USGS DEM file */
  
  if ( this->demInfo->DEMlevel <= 0 || this->demInfo->DEMlevel > 3 ) {
    fprintf( stderr, "input file is not a valid USGS DEM!\n" );
    exit( 1 );
  }
  
  /* read in the projection parameters. These can be ignored for	*/
  /* geographic, UTM, and state plane DEMs, i.e. most of the time */
  
  for ( i = 0; i < 15; i++ )
    this->demInfo->projectParams[i] = tsmReadRowsDEM_readDouble(this->fin);
  
  /* read the next three data elements from the DEM file */
  
  fscanf( this->fin,"%6d%6d%6d",
    &this->demInfo->planeUnitOfMeasure,	 /* units for ground coords */
    &this->demInfo->elevUnitOfMeasure, 	 /* units for elevation coords */
    &this->demInfo->polygonSizes );			 /* DEM polygon sides (4) */
  
  /* next we have the 4 x 2 array of ground coordinates for the corners */
  
  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 2 ; j++ )
      this->demInfo->groundCoords[i][j] = tsmReadRowsDEM_readDouble(this->fin);
    
  /* and then another three data elements: the min and max				*/
  /* elevation values and the degree of counterclockwise rotation */
  
  this->demInfo->elevBounds[0] = tsmReadRowsDEM_readDouble(this->fin);		/* min elev */
  this->demInfo->elevBounds[1] = tsmReadRowsDEM_readDouble(this->fin);		/* max elev */
  this->demInfo->localRotation = tsmReadRowsDEM_readDouble(this->fin);		/* ccw rot. */
  
  /* next we have the final data elements that we are concerned with */
  
  fscanf( this->fin,"%1d%12le%12le%12le%6d%6d",
    &this->demInfo->accuracyCode,					/* non or unknown accuracy */
    &this->demInfo->spatialResolution[0],	/* x spatial resolution */
    &this->demInfo->spatialResolution[1],	/* y spatial resolution */
    &this->demInfo->spatialResolution[2],	/* z spatial resolution */
    &this->demInfo->profileDimension[0], 	/* number of rows */
    &this->demInfo->profileDimension[1] ); /* number of columns */
  
  /* old DEM format stops here - just read over the last 160 bytes */
  
  for( i = 0; i < 160; i++ ) 
    fgetc( this->fin );
  
  /* we are now done reading the DEM header, however, now we will */
  /* calculate some values from these for later convenience.			*/
  
  /* work out the extremities of the DEM and then use these to */
  /* calculate the number of rows and columns we have. N.B. 	 */
  /* DEMs have their corners rectified against lat/long, so 	 */
  /* this creates a rotated and skewed bounding box in UTM. 	 */
  
  this->demInfo->eastMost	= MAX( this->demInfo->groundCoords[NE][0],
				 this->demInfo->groundCoords[SE][0] );
  this->demInfo->westMost	= MIN( this->demInfo->groundCoords[NW][0],
				 this->demInfo->groundCoords[SW][0] );
  this->demInfo->northMost = MAX( this->demInfo->groundCoords[NE][1],
				 this->demInfo->groundCoords[NW][1] );
  this->demInfo->southMost = MIN( this->demInfo->groundCoords[SW][1],
				 this->demInfo->groundCoords[SE][1] );
  
  this->demInfo->eastMostSample =
    ( (int) ( this->demInfo->eastMost / this->demInfo->spatialResolution[0] ) )
    * (int) this->demInfo->spatialResolution[0];
  this->demInfo->westMostSample =
    ( (int) ( this->demInfo->westMost / this->demInfo->spatialResolution[0] ) )
    * (int) this->demInfo->spatialResolution[0] ;
  this->demInfo->northMostSample =
    ( (int) ( this->demInfo->northMost / this->demInfo->spatialResolution[1] ) )
    * (int) this->demInfo->spatialResolution[1] ;
  this->demInfo->southMostSample =
    ( (int) ( this->demInfo->southMost / this->demInfo->spatialResolution[1] ) )
    * (int) this->demInfo->spatialResolution[1] ;
  
  /* now that we now the furthest points, we can easily work out */
  /* how many samples we need for each row and column (N.B. DEMs */
  /* are not regular m x n grids of values due to the fact that  */
  /* the boundaries are aligned in lat/long but the data is      */
  /* specified in UTM coordinates.                               */
  
  this->demInfo->cols = (this->demInfo->eastMostSample - this->demInfo->westMostSample)
    / (int) this->demInfo->spatialResolution[0] + 1;
  this->demInfo->rows		= (this->demInfo->northMostSample - this->demInfo->southMostSample)
    / (int) this->demInfo->spatialResolution[1] + 1;
  
  /* create a short version of the quadrangle name by just taking */
  /* the first 40 characters - this lobs off the other 100 or so	*/
  /* bytes for things like process code, sectional indicator, etc */
  
  strncpy( this->demInfo->shortName, this->demInfo->mapLabel, 40 );
  this->demInfo->shortName[40] = '\0';
  
  for ( i = 39; i > 0; i-- ) {
    if ( isspace( this->demInfo->shortName[i] ) )
      this->demInfo->shortName[i] = '\0';
    else
      break;
  }
      
  array_size = this->demInfo->cols * this->demInfo->rows;
  
  if ((this->buffer = tsmMemCalloc(array_size, sizeof(short))) == NULL){
    tsmError(funcName, "Failed to allocate memory for buffer.");
    return FALSE;
  }

  this->imageWidth = this->demInfo->cols;
  this->imageHeight = this->demInfo->rows;

  return TRUE;
}

/* +------------------------------------------------------------------------+ *
 * | Name:tsmReadRowsDEM_printDEMInfo                                       | *
 * | Description:                                                           | *
 * |   Prints out textual information about a DEMInfo structure, as read    | *
 * |   from a DEM file.                                                     | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM *this   TsmReadRowsDEM instance.                      | *
 * | Return value:                                                          | *
 * |   None.                                                                | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static void tsmReadRowsDEM_printDEMInfo(TsmReadRowsDEM *this) {
  static const char *funcName = "tsmReadRowsDEM_printDEMInfo";

  CHECK_NOT_NULL0(this);
  CHECK_NOT_NULL0(this->demInfo);
  
  printf("Name: %s\n", this->demInfo->shortName );
  printf("Level: DEM-%d\n", this->demInfo->DEMlevel );
  printf("Elevation Pattern: " );
  switch( this->demInfo->elevationPattern ) {
  case 1:
    printf("regular\n" );
    break;
  case 2:
    printf("random (future use)\n" );
    break;
  default:
    printf("unknown\n" );
  }
  
  printf("Planimetric Reference System: " );
  switch( this->demInfo->groundSystem ) {
  case 0:
    printf("geographic\n" );
    break;
  case 1:
    printf("UTM\n" );
    break;
  case 2:
    printf("State plane\n" );
    break;
  default:
    printf("unknown\n" );
  }
  
  printf("Ground Zone: %d\n", this->demInfo->groundZone );
  printf("Planimetric Unit Of Measure: " );
  switch ( this->demInfo->planeUnitOfMeasure ) {
  case 0:
    printf("radians\n" );
    break;
  case 1:
    printf("feet\n" );
    break;
  case 2:
    printf("meters\n" );
    break;
  case 3:
    printf("arc-seconds\n" );
    break;
  default:
    printf("unknown\n" );
  }
  
  printf("Elevation Units code: " );
  switch ( this->demInfo->elevUnitOfMeasure ) {
  case 1:
    printf("feet\n" );
    break;
  case 2:
    printf("meters\n" );
    break;
  default:
    printf("unknown\n" );
  }
  
  printf("Polygon Sides: %d\n", this->demInfo->polygonSizes );
  printf("Corner Coordinates:\n");
  printf("   NW (%1.3lf, %1.3lf)  NE (%1.3lf, %1.3lf)\n",
    this->demInfo->groundCoords[1][0], this->demInfo->groundCoords[1][1],
    this->demInfo->groundCoords[2][0], this->demInfo->groundCoords[2][1] );
  printf("   SW (%1.3lf, %1.3lf)  SE (%1.3lf, %1.3lf)\n",
    this->demInfo->groundCoords[0][0], this->demInfo->groundCoords[0][1],
    this->demInfo->groundCoords[3][0], this->demInfo->groundCoords[3][1] );
  printf("Min: %1.1lf, Max: %1.1lf\n",
    this->demInfo->elevBounds[0], this->demInfo->elevBounds[1] );
  printf("Accuracy Code: %d\n", this->demInfo->accuracyCode );
  printf("Spatial Resolution: %1.1lf, %1.1lf, %1.1lf \n",
    this->demInfo->spatialResolution[0],
    this->demInfo->spatialResolution[1],
    this->demInfo->spatialResolution[2] );
  printf("Elevation read: min= %d, max= %d\n",
    this->demInfo->minElevation, this->demInfo->maxElevation);
  printf("Reported DEM size: %d x %d\n",
    this->demInfo->profileDimension[0],
    this->demInfo->profileDimension[1] );
  printf("Regular grid DEM size (rows x cols) = %d x %d\n",
    this->demInfo->rows, this->demInfo->cols );
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_readDEM75                                         | *
 * | Description:                                                           | *
 * |   This is the part that actually reads in the DEM data. 7.5-min DEM    | *
 * |   data is complicated by the fact that the corners of the DEM          | *
 * |   are registered against a geodetic system, but the values are         | *
 * |   specified in UTM. We therefore find that the DEM "grid" is not       | *
 * |   orthogonal to the northing/easting axes of the UTM system and also   | *
 * |   each column may have a different number of points in it. To solve    | *
 * |   this problem, we create a regular m x n grid that completely         | *
 * |   encloses the DEM data and then we sample the DEM data into this      | *
 * |   grid.                                                                | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM *this    TsmReadRowsDEM instance.                     | *
 * | Return value:                                                          | *
 * |   TRUE if successful, FALSE if there's any error.                      | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static int tsmReadRowsDEM_readDEM75(TsmReadRowsDEM *this) {
  static const char *funcName = "tsmReadRowsDEM_readDEM75";
  int 		c, r;
  int 		tempInt, lastProfile = 0;
  int 		firstRow, lastRow;
  int 		yspacing = (int) this->demInfo->spatialResolution[1];
  int 		profileID[2], profileSize[2];
  double	planCoords[2], localElevation, elevExtremea[2];
  
  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(this->buffer, FALSE);

  /* sample the DEM values into our regular grid */
  
  for ( c = 0; c < this->demInfo->cols; c++ ) {

    printf("Resampling 7.5-minute DEM file... %6.2f%%\r", c * 100 / (float) this->demInfo->cols);
    fflush(stdout);
    
    /* Read the Logical Record Type B at the start of the column */
    /* to find out about the rest of the data in this profile 	 */
    
    fscanf( this->fin, "%6d%6d%6d%6d",
      &profileID[0],				/* row identification number */
      &profileID[1],				/* column identification number */
      &profileSize[0],			/* number of elevations in row */
      &profileSize[1] );		/* number of elevations in col (always 1) */
    
    planCoords[0]   = tsmReadRowsDEM_readDouble(this->fin);	/* 1st ground coord */
    planCoords[1]   = tsmReadRowsDEM_readDouble(this->fin);	/* 1st ground coord */
    localElevation  = tsmReadRowsDEM_readDouble(this->fin);	/* local datum elev */
    elevExtremea[0] = tsmReadRowsDEM_readDouble(this->fin);	/* profile min elev */
    elevExtremea[1] = tsmReadRowsDEM_readDouble(this->fin);	/* profile max elev */
    
    /* check for end of processing */
    
    if ( profileID[1]-1 != lastProfile ) {
      printf("Resampling 7.5-minute DEM file... done.    \n");
      return TRUE;
    }
    else
      lastProfile = profileID[1];
    
    /* read in all the data for this column */
    
    firstRow = (int)(( planCoords[1] - this->demInfo->southMostSample ) / yspacing);
    lastRow  = firstRow + profileSize[0] - 1;
    
    for ( r = 0; r < this->demInfo->rows; r++ ) {
      
      if ( r < firstRow || r > lastRow ) {
        if ( this->demInfo->stitch_edges )
          this->buffer[(r*this->demInfo->cols)+c] = NODATA;
        continue;
      }
      
      fscanf( this->fin, "%6d", &tempInt );
      
      if ( tempInt != 0 && this->demInfo->spatialResolution[2] != 1.0) {
        tempInt = (int)(tempInt * this->demInfo->spatialResolution[2]);
      }
      this->demInfo->maxElevation = MAX(this->demInfo->maxElevation, tempInt);
      if (this->demInfo->minElevation == NODATA)
        this->demInfo->minElevation = tempInt;
      else
        this->demInfo->minElevation = MIN(this->demInfo->minElevation, tempInt);
      
      if ( this->demInfo->elevUnitOfMeasure == 1 )  /* convert ft to meters */
        tempInt = (int) ( tempInt * 0.3048 );
      
      if ( r < this->demInfo->rows ) 
        this->buffer[(r*this->demInfo->cols)+c] = (short) tempInt;
      
      planCoords[1] += yspacing;
    }
  }

  printf("Resampling 7.5-minute DEM file... done.  \n");

  return TRUE;
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_stitchDEMEdges                                    | *
 * | Description:                                                           | *
 * |   By sampling the DEM into a regular northing/easting aligned grid,    | *
 * |   there will be some edge values that are not defined. This would      | *
 * |   normally mean that we would drop to sea level around the edges.      | *
 * |   Instead, we simply fill out these undefined regions with the         | *
 * |   closest defined elevation values. This greatly reduces any           | *
 * |   edge artifacts that would otherwise occur. The user can suppress     | *
 * |   this operation by specifying the "-leave" command line option.       | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM *this    TsmReadRowsDEM instance.                     | *
 * | Return value:                                                          | *
 * |   None.                                                                | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static void tsmReadRowsDEM_stitchDEMEdges(TsmReadRowsDEM *this) {
  static const char *funcName = "tsmReadRowsDEM_stitchDEMEdges";
  const int thickness = 10;
  int r, c, index;

  CHECK_NOT_NULL0(this);
  CHECK_NOT_NULL0(this->buffer);

  /* first of all stitch the west-most and east-most edges */
  for ( r = 0; r < this->demInfo->rows; r++ ) {

    /* stitch the west edge */
    for ( c = thickness; c >= 0; c-- ) {
      index = ( r * this->demInfo->cols ) + c;
      
      if ( this->buffer[index] == NODATA )
        this->buffer[index] = this->buffer[index+1];
    }
    
    /* stitch the east edge */
    for ( c = this->demInfo->cols - thickness - 1; c < this->demInfo->cols; c++ ) {
      index = ( r * this->demInfo->cols ) + c;
      
      if ( this->buffer[index] == NODATA )
        this->buffer[index] = this->buffer[index-1];
    }
  }
  
  /* then stitch the north-most and south-most edges */
  for ( c = 0; c < this->demInfo->cols; c++ ) {
    
    /* stitch the north edge */
    for ( r = this->demInfo->rows - thickness - 1; r < this->demInfo->rows; r++ ) {
      index = ( r * this->demInfo->cols ) + c;
      
      if ( this->buffer[index] == NODATA )
        this->buffer[index] = this->buffer[index-this->demInfo->cols];
    }
    
    /* stitch the south edge */
    for ( r = thickness; r >= 0; r-- ) {
      index = ( r * this->demInfo->cols ) + c;
      
      if ( this->buffer[index] == NODATA )
        this->buffer[index] = this->buffer[index+this->demInfo->cols];
    }
  }

}

static int tsmReadRowsDEM_readDEM30(TsmReadRowsDEM *this) {
  /* static const char *funcName = "tsmReadRowsDEM_readDEM30";*/
  return TRUE;
}

static int tsmReadRowsDEM_readDEM1(TsmReadRowsDEM *this) {
  /* static const char *funcName = "tsmReadRowsDEM_readDEM1"; */

  return TRUE;
}

static FILE *tsmReadRowsDEM_createTempFile(TsmReadRowsDEM *this, char *filename) {
  static const char *funcName = "tsmReadRowsDEM_createTempFile";
  FILE *fout;

  if ((fout = fopen("demdownsample_tmp.raw", "wb")) == NULL) {
    tsmError(funcName, "Couldn't create temporary file.");
    return NULL;
  }

  return fout;
}

static int tsmReadRowsDEM_saveDEMAsRAW(TsmReadRowsDEM *this, char *filename) {
  static const char *funcName = "tsmReadRowsDEM_saveDEMAsRAW";
  FILE *fout;

  if ((fout = fopen(filename, "wb")) == NULL) {
    tsmError(funcName, "Couldn't create file named %s.", filename);
    return FALSE;
  }

  fwrite(this->buffer, sizeof(short), this->imageWidth * this->imageHeight, fout);

  fflush(fout);
  fclose(fout);
  return TRUE;

}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_saveDEMAsPBM                                      | *
 * | Description:                                                           | *
 * |   This function will write out the regular grid DEM as an image file.  | *
 * |   The data is written out as either a binary PPM (colour), a binary    | *
 * |   PGM (grayscale), or a binary PGB (thresholded black/white).          | *
 * | Parameters:                                                            | *
 * |   TsmReadRowsDEM *this   TsmReadRowsDEM instance.                      | *
 * |   FILE *pbm              File handle where the output goes.            | *
 * |   int type               Type of file. 1=PGB, 2=PGM, 3=PPM.            | *
 * | Return value:                                                          | *
 * |   TRUE if successful, FALSE if there's an error.                       | *
 * |                                                                        | *
 * | Original Author: Dr. Martin Reddy                                      | *
 * | Modified by: Yong Tze Chi                                              | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
static int tsmReadRowsDEM_saveDEMAsPBM(TsmReadRowsDEM *this, char *filename, int type) {
  static const char *funcName = "tsmReadRowsDEM_saveDEMAsPBM";
  int 	 i, j;
  double elev;
  FILE *pbm;

  CHECK_NOT_NULL(this, FALSE);

  if (type<1 || type>3) {
    tsmError(funcName, "Sorry, invalid file type.");
    return FALSE;
  }

  if ((pbm = fopen(filename, "wb")) == NULL) {
    tsmError(funcName, "Failed to create file %s.", filename);
    return FALSE;
  }
 
  fprintf( pbm, "P%d\n%d %d\n255\n", type==3 ? 6 : (type==2 ? 5 : 4),
    this->demInfo->cols, this->demInfo->rows );
  
  for ( j = this->demInfo->rows - 1; j >= 0; j-- ) {
    for ( i = 0; i < this->demInfo->cols; i++ ) {
      elev = (double) this->buffer[(j*this->demInfo->cols)+i];
      
      if ( type==3 ) {
        
        /* save image as a colour PPM */
        if ( elev < -1000 ) {
          fprintf( pbm, "%c%c%c", (char) 255, (char) 0, (char) 0 );
          continue;
        }
        
        /* lame attempt to try and make colour absolute based upon  */
        /* height (rather than taking a local range) - need to work */
        /* on a better formula for varying colour over all possible */
        /* elevation ranges (including negative too?)               */
        if ( elev <= 0.0 )
          fprintf( pbm, "%c%c%c", (char) 0, (char) 6, (char) 204 );
        else if ( elev < 150.0 ) {
          elev /= 150.0 / 0.5;
          fprintf( pbm, "%c%c%c", (char) (elev * 5.1),
            (char) (25.1 + ( elev * 204 )), (char) 0 );
        } else {
          elev /= 1000.0 / 0.5;
          elev += 0.5;
          fprintf( pbm, "%c%c%c", (char) (elev * 153),
            (char) (114.75 - (elev*12.75)),
            (char) (12.75 + (elev*38.25)) );
        }
        
      }
      else if (type==2) {
        
        /* save image as a greyscale PGM */
        elev -= this->demInfo->elevBounds[0];
        elev /= ( this->demInfo->elevBounds[1] / 255.0 );
        fprintf( pbm, "%c", (char) elev );
        
      }
      else {
        
        /* save image as a black and white thresholded PGM */
        fprintf( pbm, "%c", ( elev == 0.0 ) ? (char) 0 : (char) 255	);
        
      }
    }
  }

  return TRUE;
}

/* +------------------------------------------------------------------------+ *
 * | Name: tsmReadRowsDEM_read                                              | *
 * | Description:                                                           | *
 * |   Callback function to be hooked up to tsmDownSample_inmemory. This    | *
 * |   function does not read data from file. Since all the data we need    | *
 * |   has already been sampled into a regular m x n array in memory, this  | *
 * |   function simply copies the rows requested in to the output buffer.   | *
 * |                                                                        | *
 * |   The data in the cached array is assumed to be of short int type.     | *
 * |   This function will convert the data element to match the output type | *
 * |   desired. It checks this->bytesPerComp for the output format.         | *
 * | Parameters:                                                            | *
 * |   void *buffer         Output buffer.                                  | *
 * |   int startRow         Starting row number.                            | *
 * |   int numberOfRows     Number of rows to read.                         | *
 * |   int numberOfBytes    Number of bytes needed.                         | *
 * |   void *closure        TsmReadRowsDEM instance.                        | *
 * | Return value:                                                          | *
 * |   TRUE if successful, FALSE otherwise.                                 | *
 * |                                                                        | *
 * | Author: Yong Tze Chi                                                   | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
int tsmReadRowsDEM_read(void *buffer, int startRow, int numberOfRows, int numberOfBytes, void *closure) {
  static const char *funcName = "tsmReadRowsDEM_read";
  TsmReadRowsDEM *this = (TsmReadRowsDEM *) closure;
  int /*size, i, */ offset;

  CHECK_NOT_NULL(this, FALSE);
  CHECK_NOT_NULL(buffer, FALSE);

  offset = startRow * this->imageWidth * sizeof(short);
  if (ftell(this->fin) != offset) fseek(this->fin, offset, SEEK_SET);

  fread(buffer, sizeof(short), numberOfRows * this->imageWidth, this->fin);

  /*
  size = numberOfRows * this->imageWidth;

  if (this->bytesPerComp==sizeof(short)) {
    fread(buffer, sizeof(short), numberOfRows * this->imageWidth, this->fin);
  }
  else if (this->bytesPerComp==sizeof(long)) {
    long *outBuffer = (long *) buffer;
    short value;

    for (i=0; i<size; i++) {
      fread(&value, sizeof(short), 1, this->fin);
      outBuffer[i] = value;
    }
  }
  else if (this->bytesPerComp==sizeof(unsigned char)) {
    unsigned char *outBuffer = (unsigned char *) buffer;
    short value;

    for (i=0; i<size; i++) {
      fread(&value, sizeof(short), 1, this->fin);
      outBuffer[i] = (unsigned char) (value * UCHAR_MAX / (double) SHRT_MAX);
    }
  }
  */

  return TRUE;
}


/* +------------------------------------------------------------------------+ *
 * | Name: tsmParseCreateParams_DEM                                         | *
 * | Description:                                                           | *
 * |   Opens the DEM file and fill in information into the TsmTileParams    | *
 * |   and TsmTspecParams structures.                                       | *
 * | Parameters:                                                            | *
 * |   char *filename          Name of the DEM file.                        | *
 * |   TsmTileParams *tile     TsmTileParams structure.                     | *
 * |   TsmTspecParams *tspec   TsmTspecParams structure.                    | *
 * | Return value:                                                          | *
 * |   TRUE if ok, FALSE if error.                                          | *
 * |                                                                        | *
 * | Author: Yong Tze Chi                                                   | *
 * | Date: August 9, 2000                                                   | *
 * +------------------------------------------------------------------------+ */
int tsmParseCreateParams_DEM(char *filename, TsmTileParams *tile, TsmTspecParams *tspec) {
  static const char *funcName = "tsmReadRowsDEM_readDEMHeader";
  int 	  i, j;
  int     array_size;
  DEMInfo demInfo;
  FILE *fin;

  if ((fin = fopen(filename, "rb")) == NULL) {
    tsmError(funcName, "Failed to open file named %s.", filename);
    return FALSE;
  }
  
  
  fscanf( fin, "%144c%6d%6d%6d%6d",
    demInfo.mapLabel, 						 /* quadrangle name field */
    &demInfo.DEMlevel,						 /* DEM level code */
    &demInfo.elevationPattern,		 /* pattern code */
    &demInfo.groundSystem,				 /* planimetric code */
    &demInfo.groundZone );				 /* UTM zone */
  
  /* there is no magic number to identify DEM files, but we can  */
  /* at least check to see if we have valid values for the first */
  /* few fields to check that we have been given a USGS DEM file */
  
  if ( demInfo.DEMlevel <= 0 || demInfo.DEMlevel > 3 ) {
    fprintf( stderr, "input file is not a valid USGS DEM!\n" );
    exit( 1 );
  }
  
  /* read in the projection parameters. These can be ignored for	*/
  /* geographic, UTM, and state plane DEMs, i.e. most of the time */
  
  for ( i = 0; i < 15; i++ )
    demInfo.projectParams[i] = tsmReadRowsDEM_readDouble(fin);
  
  /* read the next three data elements from the DEM file */
  
  fscanf( fin,"%6d%6d%6d",
    &demInfo.planeUnitOfMeasure,	 /* units for ground coords */
    &demInfo.elevUnitOfMeasure, 	 /* units for elevation coords */
    &demInfo.polygonSizes );			 /* DEM polygon sides (4) */
  
  /* next we have the 4 x 2 array of ground coordinates for the corners */
  
  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 2 ; j++ )
      demInfo.groundCoords[i][j] = tsmReadRowsDEM_readDouble(fin);
    
  /* and then another three data elements: the min and max				*/
  /* elevation values and the degree of counterclockwise rotation */
  
  demInfo.elevBounds[0] = tsmReadRowsDEM_readDouble(fin);		/* min elev */
  demInfo.elevBounds[1] = tsmReadRowsDEM_readDouble(fin);		/* max elev */
  demInfo.localRotation = tsmReadRowsDEM_readDouble(fin);		/* ccw rot. */
  
  /* next we have the final data elements that we are concerned with */
  
  fscanf( fin,"%1d%12le%12le%12le%6d%6d",
    &demInfo.accuracyCode,					/* non or unknown accuracy */
    &demInfo.spatialResolution[0],	/* x spatial resolution */
    &demInfo.spatialResolution[1],	/* y spatial resolution */
    &demInfo.spatialResolution[2],	/* z spatial resolution */
    &demInfo.profileDimension[0], 	/* number of rows */
    &demInfo.profileDimension[1] ); /* number of columns */
  
  /* we are now done reading the DEM header, however, now we will */
  /* calculate some values from these for later convenience.			*/
  
  /* work out the extremities of the DEM and then use these to */
  /* calculate the number of rows and columns we have. N.B. 	 */
  /* DEMs have their corners rectified against lat/long, so 	 */
  /* this creates a rotated and skewed bounding box in UTM. 	 */
  
  demInfo.eastMost	= MAX( demInfo.groundCoords[NE][0],
				 demInfo.groundCoords[SE][0] );
  demInfo.westMost	= MIN( demInfo.groundCoords[NW][0],
				 demInfo.groundCoords[SW][0] );
  demInfo.northMost = MAX( demInfo.groundCoords[NE][1],
				 demInfo.groundCoords[NW][1] );
  demInfo.southMost = MIN( demInfo.groundCoords[SW][1],
				 demInfo.groundCoords[SE][1] );
  
  demInfo.eastMostSample =
    ( (int) ( demInfo.eastMost / demInfo.spatialResolution[0] ) )
    * (int) demInfo.spatialResolution[0];
  demInfo.westMostSample =
    ( (int) ( demInfo.westMost / demInfo.spatialResolution[0] ) )
    * (int) demInfo.spatialResolution[0] ;
  demInfo.northMostSample =
    ( (int) ( demInfo.northMost / demInfo.spatialResolution[1] ) )
    * (int) demInfo.spatialResolution[1] ;
  demInfo.southMostSample =
    ( (int) ( demInfo.southMost / demInfo.spatialResolution[1] ) )
    * (int) demInfo.spatialResolution[1] ;
  
  /* now that we now the furthest points, we can easily work out */
  /* how many samples we need for each row and column (N.B. DEMs */
  /* are not regular m x n grids of values due to the fact that  */
  /* the boundaries are aligned in lat/long but the data is 	 */
  /* specified in UTM coordinates.			         */
  
  demInfo.cols = (demInfo.eastMostSample - demInfo.westMostSample)
    / (int) demInfo.spatialResolution[0] + 1;
  demInfo.rows		= (demInfo.northMostSample - demInfo.southMostSample)
    / (int) demInfo.spatialResolution[1] + 1;

  tile->width = tspec->width = demInfo.cols;
  tile->height = tspec->height = demInfo.rows;
  tile->comps = tspec->comps = 1;
  tile->bytespercomp = sizeof(short);
  /* tile->out_bytespercomp = sizeof(short); */

  tspec->coord_sys = (demInfo.groundSystem==0)
    ? TSM_GEO_COORDSYS
    : ((demInfo.groundSystem==1) ? TSM_UTM_COORDSYS : TSM_NONE_COORDSYS);
  tspec->zone = demInfo.groundZone;
  tspec->ll_e = demInfo.westMost;
  tspec->ll_n = demInfo.southMost;

  array_size = strlen(demInfo.mapLabel);
  if (array_size>0) {
    if ((tspec->geoName = tsmMemMalloc(array_size)) != NULL) {
      strcpy(tspec->geoName, demInfo.mapLabel);
    }
  }
  tspec->start_res = demInfo.spatialResolution[0];

  fclose(fin);

  return TRUE;
}


/* EOF */
