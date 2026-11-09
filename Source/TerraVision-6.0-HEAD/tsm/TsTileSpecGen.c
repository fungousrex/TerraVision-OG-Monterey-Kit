/*
 * Name: TsTileSpecGen.c
 *
 * Description:
 *      Generates Tspec (Tile Specification) data for either OI or DEM
 *      input images. A .prt file, associated with the image, is required 
 *      to hold the image resolution and min/max pixel values. An optional
 *      .descr file can be used to hold a description of the image.
 *
 * Function List:
 *
 * Revision History: 
 *      - Initial tspec generation code by Brian Davis, USGS.
 *      - Updated by Martin Reddy, Jul/Aug 1997, for new tspec interface.
 * 
 *   $Id: TsTileSpecGen.c,v 1.21.6.1 2001/08/17 18:54:45 leei Exp $
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
#include "RELEASE"

/*
 * Synopsis:
 *   Pyramid *tspCreatePyramid( char *pyramid_name, tspSymbol pyramid_type,
 *                              TsmTspecParams params )
 *
 * Description:
 *   Create a UTM coordinate system Pyramid tspec.
 *      
 *   Generates a Pyramid structure and fills in all of the fields with
 *   the information given. This structure can then be stored using the
 *   tsmPutPyramid function. The information we need to supply is provided
 *   via a TsmTspecParams structure.
 *
 * @param    pyramid_name the name (or pathname/URL) of the pyramid
 * @param    pyramid_type one of tspOiType or tspDemType
 * @param    params a valid TsmTspecParams containing all tspec values
 *
 * @return   allocated and filled pyramid structure, or NULL if failed
 *
 * @author   Martin Reddy, based upon code by Brian Davis
 * @version  21 August 1997
 *
 * @see      Pyramid, tsmPutPyramid, TsmTspecParams
 *
 */

Pyramid *
tspCreatePyramid( char *pyramid_name, tspSymbol pyramid_type,
		  TsmTspecParams params, TsmTileParams tile_params )
{
  const  char *funcName = "tspCreatePyramid";

  int    end_lev, lev;                    /* min/max level for the pyramid */
  float  end_res, res;                   /* max resolution for the pyramid */

  int    x_tiles, y_tiles;     /* number of rows & cols of tiles in mosaic */
  int    tl_sz, img_size, min_size,i; /* the pixel length of a single tile */

  double inv_res;                    /* for ixiytoUV and uvtoIXIY matrices */
  double utm_m_e, utm_m_n;        /* center UTM coord for the bounding box */
  double geo_m_x, geo_m_y;   /* center Lat/Long coord for the bounding box */

  double origin_x, origin_y;         /* origin in whatever coord sys units */

  PRExplodedTime tm;
  char   today[512];                     /* string to contain today's date */
  char   *parsed_name;                     /* start of parsed pyramid name */
  char   tsmName[50];               /* default name if no idOpDoneBy given */

  char   *listBuffer;        /* temp buffer used by tspec parsing routines */

  Pyramid         *pyramid;
  IdentityTspec   *ident_tspec;
  TileTspec       *tile_tspec;
  CoordTspec      *coord_tspec;
  PixelTspec      *pixel_tspec;
  ProjectionTspec *proj_tspec;
  SimpleTileSet   *tileset;
  ImageTransformationTspec *xform_tspec;

  /* First of all we want to work out how may levels we will be creating */
  /* for this OI pyramid. We calculate this by seeing how many times we  */
  /* have to downsample the input image in order to get an image that is */
  /* completely contained within one tile.                               */

  end_lev = params.start_lev;
  end_res = params.start_res;

  img_size = max( params.height, params.width );
  min_size = min( tile_params.tile_width, tile_params.tile_height );
  while ( img_size > min_size ) {
    end_lev++;
    end_res  *= 2.0;
    img_size = (img_size+1)/2;
  }

  /* Allocate a pyramid structure with the required no. of SimpleTileSets */
  /* we don't give it a pathname as that's not our business here - we are */
  /* not concerned with storing the pyramid here - just with creating it  */

  if ( ! ( pyramid = tsmAllocatePyramid( "", params.start_lev, end_lev ) ) ) {
    tsmError( funcName, "couldn't allocate pyramid structure" );
    return NULL;
  }

  /* Do some sanity checks on in the input parameters, just in case */

  if ( params.description == NULL ) params.description = "";
  if ( params.sources == NULL ) params.sources = "";
  if ( params.source_type == NULL || params.source_type[0] == '\0')
    params.source_type = "unknown";
  if ( params.ops == NULL ) params.ops = "";
  if ( params.doneby == NULL || params.doneby[0] == '\0' ) {
    strcpy( tsmName, "tsmApi " );
    strcat( tsmName, &tsmApiRelease[1] );
    tsmName[strlen(tsmName)-2] = '\0';
    params.doneby = tsmName;
  }

  /* Work out the tile size: default OI tiles are 128x128, DEMs are 32x32 */
  /* These defaults are set up in tsmInitTileParams, but can be modified. */

  tl_sz = tile_params.tile_width;

  /* if in the northern hemisphere, the lower corner is less north than */
  /* the top corner. We subrtact the amount of the data resolution      */
  /* because the origin according to yvan is lower left.  The location  */
  /* identified with a LAS pixel is the upper left corner of the        */
  /* rectangular representation of the pixel.  According to TV, the     */
  /* location values should correspond to the lower left of the         */
  /* rectangular representation of the pixel.  The x position is the    */
  /* same for both. If the oi and dem are co-registered - they should   */
  /* be - both need to have their y corner values adjusted by the number*/
  /* of meters in a pixel (resolution).  We just need to adjust the     */
  /* value used to create the tpsecs, however, because the difference   */
  /* between the top and bottom of a pixel is the value of its          */
  /* resolution.  The same new y value will result in either case.      */

  /* First of all, write out the base pyramid information that was */
  /* usually stored in the tile-set.tspec file. We output this     */
  /* each time because the maxLevel/Resolution values change.      */

  pyramid->type = tspPyramidTileSetType;
  pyramid->invResScaleFactor = 2.0;
  pyramid->minLevel = params.start_lev;
  pyramid->maxLevel = end_lev;
  pyramid->minResolution = params.start_res;
  pyramid->maxResolution = end_res;
  pyramid->description = tsmMemManagerStrdup(params.description);
  pyramid->rectification = TSM_UTM_COORDSYS;

  /* rendering parameters */
  pyramid->sky = params.sky;
  pyramid->twoSided = params.two_sided;
  pyramid->fromBelow = params.view_from_below;

  /* allocate the memory for the componentNames array */
  /* of the pyramid's pixel.tspec structure.          */

  pixel_tspec = &pyramid->pixelTspec;
  if (! tsmArrayMalloc (tspSymbol, pixel_tspec->componentNames,params.comps)) {
    tsmError( funcName, "couldn't allocate pixel.tspec structs" );
    return NULL;
  }

  /* create the base pyramid pixel.tspec */

  if ( pyramid_type == tspOiType ) {
    if ( params.comps == 3 ) {
      pixel_tspec->componentNames.val[0] = tspRed;
      pixel_tspec->componentNames.val[1] = tspGreen;
      pixel_tspec->componentNames.val[2] = tspBlue;
    } else
      pixel_tspec->componentNames.val[0] = tspAverage;
  } else
    pixel_tspec->componentNames.val[0] = tspElevation;

  pixel_tspec->minVal = -1.0;   /* invalid: not calculated yet */
  pixel_tspec->maxVal = -1.0;
  pixel_tspec->scalingType = tspLinear;
  pixel_tspec->scale = params.scale;
  pixel_tspec->offset = params.offset;

  /* create the base pyramid tile.tspec */

  tile_tspec = &pyramid->tileTspec;
  tile_tspec->numPixelsX = tile_params.tile_width;
  tile_tspec->numPixelsY = tile_params.tile_height;
  tile_tspec->overLap[0] = tile_params.tile_overlap_x;
  tile_tspec->overLap[1] = tile_params.tile_overlap_y;
  tile_tspec->totalPixels[0] = tile_params.tile_width +
    tile_params.tile_overlap_x;
  tile_tspec->totalPixels[1] = tile_params.tile_height +
    tile_params.tile_overlap_y;
  tile_tspec->storageFormat = tspLrbtRaster;

  if ( pyramid_type == tspOiType ) {
    tile_tspec->componentNumberType  = tspUnsignedByte;
    tile_tspec->numComponentsPixel   = params.comps;
    tile_tspec->numBytesPerComponent = 1;
  } else {
    tile_tspec->componentNumberType = tspSignedByte;
    tile_tspec->numComponentsPixel   = 1;
    tile_tspec->numBytesPerComponent = tile_params.out_bytespercomp;
  }

  /* create the base pyramid identity.tspec */

  ident_tspec = &pyramid->identityTspec;

  /* work out the name of the pyramid. This should be passed in anyway,  */
  /* but in case we were given a filename/URL, we make sure that we only */
  /* have the filename component of any URL.                             */

  if ( ( parsed_name = strrchr( pyramid_name, '/' ) ) != NULL )
    parsed_name++;
  else
    parsed_name = pyramid_name;

  /* work out the time in mm/dd/yyyy format */

  PR_ExplodeTime(PR_Now(), PR_LocalTimeParameters, &tm);
  sprintf(today, "%d/%d/%d", tm.tm_month+1, tm.tm_mday, tm.tm_year);

  /* calculate how many sources and operations we have to allocate for */

  listBuffer = (char *) tsmMemMalloc( max( max( strlen(params.sources),
						strlen(params.ops) ), 80 ) );

  ident_tspec->name = tsmMemManagerStrdup(parsed_name);
  ident_tspec->type = pyramid_type;
  ident_tspec->subtype = tspNoneSubtype;
  ident_tspec->operationDate = tsmMemManagerStrdup( today );
  ident_tspec->operationDoneBy = tsmMemManagerStrdup( params.doneby );
  ident_tspec->source_type = tsmMemManagerStrdup( params.source_type );

  tspParseCharArray ((char *) params.sources, listBuffer, 
		     &pyramid->identityTspec.sources.len,
		     &pyramid->identityTspec.sources.val);
  tspParseSymbolArray ((char *) params.ops, listBuffer,
		       &pyramid->identityTspec.operations.len,
		       &pyramid->identityTspec.operations.val);

  tsmMemFree( listBuffer );

  /* allocate the checkSum arrays, filled with -1 to show not calc'd yet */

  if (! tsmMakeChecksum (&ident_tspec->checkSumLrbt,params.start_lev,end_lev)){
    tsmError( funcName, "couldn't allocate checksum-lrbt array!" );
    ident_tspec->checkSumLrbt.len = 0;
  }

  if (! tsmMakeChecksum (&ident_tspec->checkSumJpeg,params.start_lev,end_lev)){
    tsmError( funcName, "couldn't allocate checksum-jpeg array!" );
    ident_tspec->checkSumJpeg.len = 0;
  }

  /* If a geoName was not specified, then try to make one   */
  /* up. We do this by taking the dataset name and chopping */
  /* off any extensions and capitalising the first letter,  */
  /* e.g. "irwin.128.oi" -> "Irwin"                         */

  if ( params.geoName != NULL ) 
    pyramid->geoName = tsmMemStrdup( params.geoName );
  else {
    pyramid->geoName = tsmMemStrdup( ident_tspec->name );
    pyramid->geoName[0] = toupper( pyramid->geoName[0] );

    if ( ( listBuffer = strchr( pyramid->geoName, '.' ) ) != NULL )
      *listBuffer = '\0';
  }
    
  pyramid->planet = params.planet;
  
  /* create the base pyramid 3d-coordinate-system.tspec */

  coord_tspec = &pyramid->coordTspec;
  coord_tspec->systemType = tspSpheroid;
  coord_tspec->spheroidName = tspNad83;
  coord_tspec->equatorialRadius = 6378137.0;
  coord_tspec->polarRadius = 6356752.3;
  coord_tspec->originAltitude = params.altitude;

  proj_tspec = &pyramid->projectionTspec;

  for ( i = 0; i < 16; i++ ) {
    /* (1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0) */
    if ( i == 0 || i == 5 || i == 10 || i == 15 )
      coord_tspec->lvcstoGeo[i] = 1.0;
    else
      coord_tspec->lvcstoGeo[i] = 0.0;
    coord_tspec->geotoLVCS[i] = coord_tspec->lvcstoGeo[i];

    proj_tspec->lcsToUvProjectiveMatrix[i] = coord_tspec->lvcstoGeo[i];
    proj_tspec->uvaToLcsMatrix[i] = coord_tspec->lvcstoGeo[i];
  }
  coord_tspec->lcsType = tspNone;

  /* create the base pyramid projection.tspec */

  switch ( params.coord_sys ) {
  case TSM_LVCS_COORDSYS:

    /* create a projection.tspec for an LVCS dataset */

    proj_tspec->projectionType = tspAffineProjection;

    /* update the coord.tspec for the WGS84 datum */

    coord_tspec->spheroidName       = tspWGS84;
    coord_tspec->equatorialRadius   = 6378137.0;
    coord_tspec->polarRadius        = 6356752.3142;
    coord_tspec->originLat          = params.ll_lat;
    coord_tspec->originLong         = params.ll_lon;
    coord_tspec->componentUnits     = tspMeters;
    coord_tspec->localUnitsPerMeter = 1.0;
    coord_tspec->lcsType            = tspLvcsCoordSystem;

    /* set the component names to be "x y z", in that order */

    if ( ! tsmArrayMalloc( tspSymbol, coord_tspec->componentNames, 3 ) ) {
      tsmError( funcName, "couldn't allocate coord.tspec structs" );
      return NULL;
    }

    coord_tspec->componentNames.val[0] = tspX;
    coord_tspec->componentNames.val[1] = tspY;
    coord_tspec->componentNames.val[2] = tspZ;

    /* set a lat/long rectifiction grid by default */
    
    pyramid->rectification = TSM_LATLONG_COORDSYS;

    /* save the dataset dimensions for the image transform matrix */

    origin_x = params.lvcs_x;
    origin_y = params.lvcs_y;
    break;

  case TSM_UTM_COORDSYS:

    /* create a projection.tspec for a UTM dataset */
    
    utm_m_e = params.ll_e + ( params.start_res * ( params.width / 2.0 ) );
    utm_m_n = params.ll_n + ( params.start_res * ( params.height / 2.0 ) );

    if ( ! tsmUTMToLatLong(params.zone, utm_m_e, utm_m_n, &geo_m_x, &geo_m_y)){
      tsmError( funcName, "Couldn't convert UTM mid point to lat/long" );
      return NULL;
    }

    proj_tspec->projectionType = tspUtmProjection;
    proj_tspec->utmZoneNumber = params.zone;
    proj_tspec->centralMeridian = tspCalcCentralMeridian( params.zone );
    proj_tspec->k0 = tspCalcK0();
    proj_tspec->geoidSeparation = tspCalcGeoidSeparation( geo_m_x, geo_m_y );

    /* save the northing/easting origin for the image transform matrix */

    origin_x = params.ll_e;
    origin_y = params.ll_n;
    break;

  case TSM_LATLONG_COORDSYS:

    /* create a projection.tspec for a LatLong dataset */

    proj_tspec->projectionType = tspLatLongProjection;

    /* update the coord.tspec for the WGS84 datum */

    coord_tspec->spheroidName     = tspWGS84;
    coord_tspec->equatorialRadius = 6378137.0;
    coord_tspec->polarRadius      = 6356752.3142;
    coord_tspec->originLat        = params.ll_lat;
    coord_tspec->originLong       = params.ll_lon;
    coord_tspec->componentUnits   = tspDegrees;

    /* set a lat/long rectifiction grid by default */
    
    pyramid->rectification = TSM_LATLONG_COORDSYS;

    /* save the lat/long origin for the image transform matrix */

    origin_x = params.ll_lon;
    origin_y = params.ll_lat;
    break;

  default:
    tsmError( funcName, "Unknown coordinate system" );
  }

  /* Now we will do the individual levels of the OI or DEM pyramid. */
  /* Here we have to generate tile-set.tspec info for every level   */

  for ( lev = params.start_lev, res = params.start_res;
	lev <= end_lev;
	lev++, res *= 2.0 ) {

    /* work out the number of rows/cols of tiles in this level */

    y_tiles = ( params.height / tl_sz ) + ( params.height % tl_sz != 0 );
    x_tiles = ( params.width  / tl_sz ) + ( params.width % tl_sz != 0 );

    /* fill in the tile-set.tspec fields */

    tileset = &pyramid->tspecLevel.val[lev];

    tileset->minTile[0] = 0;
    tileset->minTile[1] = 0;
    tileset->maxTile[0] = x_tiles - 1;
    tileset->maxTile[1] = y_tiles - 1;
    TSMTRACE ((TSMMASK|2, funcName, "level %d tiles X=[%d,%d] Y=[%d,%d]",
	       tileset->minTile[0], tileset->maxTile[0],
	       tileset->minTile[1], tileset->maxTile[1]));
    tileset->numTile[0] = x_tiles;
    tileset->numTile[1] = y_tiles;
    tileset->numPixels[0] = x_tiles * tl_sz;
    tileset->numPixels[1] = y_tiles * tl_sz;
    tileset->minValidPixel[0] = 0;
    tileset->minValidPixel[1] = 0;
    tileset->maxValidPixel[0] = params.width - 1;
    tileset->maxValidPixel[1] = params.height - 1;
    tileset->numValidPixel[0] = params.width;
    tileset->numValidPixel[1] = params.height;
    tileset->tileMapStatus = tspTileMapFull;
    tileset->description = NULL;

    /* simulate the downsampling of the image for the next level */

    params.width  = (params.width+1)/2;
    params.height = (params.height+1)/2;

    /* create the image-transformation.tspec for this level */

    inv_res = 1 / res;

    xform_tspec = &tileset->imageTransformationTspec;

    xform_tspec->uvtoIXIY[0] = inv_res;
    xform_tspec->uvtoIXIY[1] = 0.0;
    xform_tspec->uvtoIXIY[2] = origin_x * inv_res * -1.0;
    xform_tspec->uvtoIXIY[3] = 0.0;
    xform_tspec->uvtoIXIY[4] = inv_res;
    xform_tspec->uvtoIXIY[5] = origin_y * inv_res * -1.0;
    xform_tspec->uvtoIXIY[6] = 0.0;
    xform_tspec->uvtoIXIY[7] = 0.0;
    xform_tspec->uvtoIXIY[8] = 1.0;

    /* the ixiy->uv matrix is simply the inverse of the uv->ixiy matrix */

    tsmInverse3x3( xform_tspec->uvtoIXIY, xform_tspec->ixiytoUV );

    /*
      xform_tspec->ixiytoUV[0] = res;      xform_tspec->ixiytoUV[1] = 0.0;
      xform_tspec->ixiytoUV[2] = box_ll_x; xform_tspec->ixiytoUV[3] = 0.0;
      xform_tspec->ixiytoUV[4] = res;      xform_tspec->ixiytoUV[5] = box_ll_y;
      xform_tspec->ixiytoUV[6] = 0.0;      xform_tspec->ixiytoUV[7] = 0.0;
      xform_tspec->ixiytoUV[8] = 1.0;
    */

  }

  /* Fill all of the duplicate SimpleTileSet fields from the Pyramid fields */

  tsmFillSimpleTileSets( pyramid );

  return pyramid;
}

