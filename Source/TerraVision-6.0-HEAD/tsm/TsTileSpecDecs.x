/*
 * $Author: leei $
 * $Source: /homedir/magic/software/CVS/TerraVision/tsm/TsTileSpecDecs.x,v $
 * $RCSfile: TsTileSpecDecs.x,v $
 * $Revision: 1.23.10.2 $
 * $Locker:  $
 * $Date: 2001/08/17 18:54:40 $
 */
/*
 * TsTileSpecDecs.x
 *
 * by Yvan Leclerc, SRI; modified by Gary Hoo, LBL
 *
 * generate files using:
 *    rpcgen -h -o TsTileSpecDecs.h TsTileSpecDecs.x
 *    rpcgen -cP -o TsTileSpecDecs_xdr.c TsTileSpecDecs.x
 */

/* RCS identification */
#ifdef RPC_XDR
%static char rcsid[] = "$Id: TsTileSpecDecs.x,v 1.23.10.2 2001/08/17 18:54:40 leei Exp $";
#endif

#ifdef RPC_HDR
%#ifndef TSTILESPECDECS_H
%#define TSTILESPECDECS_H
%#include "tsmPurl.h"
#endif

%#include "tsmGenArray.h"

#ifdef RPC_HDR
%/*
% * Enums used by tspec I/O code.  Each enum below corresponds
% * to a "symbol" used in one of the tspec files.  File TsTileSpecUtils.c
% * contains the corresponding character strings for each symbol.  
% *
% * Note that each logical group of symbol begins with tsp...Start and 
% * ends with tsp...End.  That way, the tspec i/o routine can verify
% * that a tile belongs within a group by comparing the enum to those
% * values.  Because of this, the character string array contains
% * two blank strings between each group of symbols.
% *
% * DO NOT CHANGE this enum without changing the corresponding
% * tspSymbolStrings array of strings!
% */
#endif

/*
 * GH note: from what I can see, the blank lines in the following declaration
 * don't need to be saved for Yvan's scheme.  The character string array that
 * needs the precise formatting is in TsTileSpecUtils.c.  
 */

enum tspSymbol 
{
    tspSymbolError = -1,
    tspUnknownSymbol,

    tspTileSetTypeStart,
    tspSimpleTileSetType,
    tspPyramidTileSetType,
    tspRectangularORGATileSetType,
    tspMultiImageTileSetType,
    tspTerraFormRectangularAreaTileSetType,
    tspTerraVisionTileSetType,
    tspGeoPyramidTileSetType,
    tspTileSetTypeEnd,

    tspTileTypeStart,
    tspDemType,
    tspOiType,
    tspMapType,
    tspAerialImageType,
    tspTileTypeEnd,

    tspSubtypeStart,
    tspNoneSubtype,
    tspUsgsSubtype,
    tspMosaicSubtype,
    tspUsgsControlSubtype,
    tspSubtypeEnd,

    tspOperationStart,
    tspMerge,
    tspWindow,
    tspTile,
    tspRectify,
    tspOiTrim,
    tspUtmToLvcs,
    tspMosaic,
    tspGaussReduce,
    tspPointSample,
    tspTerraFormMosaic,
    tspScan,
    tspUsgsRectify,
    tspUsgsMosaic,
    tspGPyramid,
    tspOperationEnd,

    tspComponentNumberTypeStart,
    tspUnsignedByte,
    tspSignedByte,
    tspComponentNumberTypeEnd,

    tspStorageFormatStart,
    tspLrbtRaster,
    tspJPEGCompressed,
    tspStorageFormatEnd,

    tspCoordSystemTypeStart,
    tspSpheroid,
    tspCoordSystemTypeEnd,

    tspSpheroidNameStart,
    tspNad83,
    tspWGS84,
    tspSpheroidNameEnd,

    tspCoordSystemStart,
    tspUtmCoordSystem,
    tspLvcsCoordSystem,
    tspNone,
    tspCoordSystemEnd,

    tspCoordUnitStart,
    tspMeters,
    tspDegrees,
    tspCoordUnitEnd,

    tspPixelComponentNameStart,
    tspRed,
    tspGreen,
    tspBlue,
    tspElevation,
    tspAverage,
    tspPixelComponentNameEnd,

    tspScalingTypeStart,
    tspLinear,
    tspScalingTypeEnd,

    tspProjectionTypeStart,
    tspAffineProjection,
    tspUtmProjection,
    tspLatLongProjection,
    tspProjectionTypeEnd,

    tspTileMapStatusStart,
    tspTileMapEmpty,
    tspTileMapFull,
    tspTileMapStatusEnd,

    tspPlanetStart,
    tspMercury,
    tspVenus,
    tspEarth,
    tspMars,
    tspJupiter,
    tspSaturn,
    tspUranus,
    tspNeptune,
    tspPluto,
    tspPlanetEnd,

    tspCoordComponentNameStart,
    tspX,
    tspY,
    tspZ,
    tspCoordComponentNameEnd,

    tspFileStart,
    tspStdin,
    tspStdout,
    tspStderr,
    tspFileEnd,

    tspNumSymbols
};

/* Because rpcgen doesn't allow pointers to pointers to be declared directly:
 */
typedef string charPtr<>;

/* An enumerated type to specify the valid coord systems for a dataset */

enum TsmCoordSystem {
  TSM_NONE_COORDSYS = 0,    /* No coordinate system */
  TSM_LATLONG_COORDSYS,     /* Latitude/Longitude coordinate system */
  TSM_UTM_COORDSYS,         /* Universal Transverse Mercator system */
  TSM_GEO_COORDSYS,         /* Geocentric Coordinate System */
  TSM_LVCS_COORDSYS         /* Local Vertical Coordinate System */
};

/* An enumerated type to specify a planet in the solar system */

enum TsmPlanet {
  TSM_MERCURY,
  TSM_VENUS,
  TSM_EARTH,
  TSM_MARS,
  TSM_JUPITER,
  TSM_SATURN,
  TSM_URANUS,
  TSM_NEPTUNE,
  TSM_PLUTO
};

/*
 * Doc++ Description: *//** 
 *   Holds a lat/long coordinate.
 *
 *   A structure to hold a single latitude/longitude coordinate (deg)
 *
 * @param lat (double) = a latitude value in degrees
 * @param lon (double) = a longitude value in degrees
 *
 * @see GeoFootprint, UTMCoord, GeoCoord
 *
 */

struct LatLongCoord {
  double lat;
  double lon;
};

/*typedef struct LatLongCoord LatLongCoord;*/

/*
 * Doc++ Description: *//** 
 *   Holds four lat/long coordinates.
 *
 *   This structure holds exactly four lat/long coordinates. This is
 *   used to describe the four boundary corners of a dataset. The 
 *   corners should be given in counterclockwise order.
 *
 * @param c1 (LatLongCoord) = the 1st lat/long coordinate
 * @param c2 (LatLongCoord) = the 2nd lat/long coordinate
 * @param c3 (LatLongCoord) = the 3rd lat/long coordinate
 * @param c4 (LatLongCoord) = the 4th lat/long coordinate
 *
 * @see LatLongCoord, UTMFootprint, GeoFootprint
 *
 */

struct LatLongFootprint {
  LatLongCoord c1;
  LatLongCoord c2;
  LatLongCoord c3;
  LatLongCoord c4;
};

/*typedef struct LatLongFootprint LatLongFootprint;*/

/*
 * Doc++ Description: *//** 
 *   Holds a UTM coordinate.
 *
 *   A structure to hold a single UTM coordinate (meters/zone).
 *   UTM stands for Universal Transverse Mercator.
 *
 * @param e (double) = a easting value in meters
 * @param n (double) = a northing value in meters
 * @param zone (int) = the UTM zone number (1..60)
 *
 * @see UTMFootprint, LatLongCoord, GeoCoord
 *
 */

struct UTMCoord {
  double e;
  double n;
  int    zone;
};

/*typedef struct UTMCoord UTMCoord; */

/*
 * Doc++ Description: *//** 
 *   Holds four UTM coordinates.
 *
 *   This structure holds exactly four UTM coordinates. This is
 *   used to describe the four boundary corners of a dataset. The 
 *   corners should be given in counterclockwise order.
 *
 * @param c1 (UTMCoord) = the 1st UTM coordinate
 * @param c2 (UTMCoord) = the 2nd UTM coordinate
 * @param c3 (UTMCoord) = the 3rd UTM coordinate
 * @param c4 (UTMCoord) = the 4th UTM coordinate
 *
 * @see UTMCoord, LatLongFootprint, GeoFootprint
 */

struct UTMFootprint {
  UTMCoord c1;
  UTMCoord c2;
  UTMCoord c3;
  UTMCoord c4;
};

/*typedef struct UTMFootprint UTMFootprint;*/

/*
 * Doc++ Description: *//** 
 *   Holds an LVCS footprint.
 *
 *   A structure to hold a single LVCS footprint (origin + extent).
 *   LVCS stands for Local Vertical Coordinate System. It is a 
 *   flat plane tangent to the surface of the earth.
 *
 * @param lat (double) = the latitude of the LVCS origin (degrees)
 * @param lon (double) = the longitude of the LVCS origin (degrees)
 * @param width (double) = distance LVCS covers east-west (meters)
 * @param height (double) = distance LVCS covers north-south (meters)
 *
 * @see LVCSFootprint, LatLongCoord, GeoCoord, UTMCoord
 *
 */

struct LVCSFootprint {
  double lat;
  double lon;
  double xoffset;
  double yoffset;
  double width;
  double height;
};

/*typedef struct LVCSFootprint LVCSFootprint; */

/*
 * Doc++ Description: *//** 
 *   Holds a geocentric coordinate.
 *
 *   A structure to hold a single geocentric coordinate (meters).
 *
 * @param x (double) = the geocentric x-coordinate
 * @param y (double) = the geocentric y-coordinate
 * @param z (double) = the geocentric z-coordinate
 *
 * @see GeoFootprint, LatLongCoord, UTMCoord
 *
 */

struct GeoCoord {
  double x;
  double y;
  double z;
};

/*typedef struct GeoCoord GeoCoord;*/

/*
 * Doc++ Description: *//** 
 *   Holds four geocentric coordinates.
 *
 *   This structure holds exactly four geocentric coordinates. This is
 *   used to describe the four boundary corners of a dataset. The 
 *   corners should be given in counterclockwise order.
 *
 * @param c1 (GeoCoord) = the 1st geocentric coordinate
 * @param c2 (GeoCoord) = the 2nd geocentric coordinate
 * @param c3 (GeoCoord) = the 3rd geocentric coordinate
 * @param c4 (GeoCoord) = the 4th geocentric coordinate
 *
 * @see GeoCoord, UTMFootprint, LatLongFootprint
 *
 */

struct GeoFootprint {
  GeoCoord c1;
  GeoCoord c2;
  GeoCoord c3;
  GeoCoord c4;
};

/*typedef struct GeoFootprint GeoFootprint;*/

/*
 * Doc++ Description: *//** 
 *   Holds four coordinates in a number of possible systems.
 *
 *   This structure contains information about the four boundary coords
 *   for a dataset. These can be in any of a number of coordinate
 *   systems, including lat/long, UTM, or geocentric. There is a
 *   field (footprint) to indicate which coordinate system is
 *   being used.
 *
 * @param coordsys (TsmCoordSystem) = TSM_NONE_COORDSYS, TSM_LATLONG_COORDSYS,
 *                                    TSM_UTM_COORDSYS, or TSM_GEO_COORDSYS
 * @param ll (LatLongFootprint) = the lat/long footprint definition
 * @param utm (UTMFootprint) = the UTM footprint definition
 * @param geo (GeoFootprint) = the geocentric footprint definition
 *
 * @see LatLongFootprint, UTMFootprint, GeoFootprint
 *
 */

struct Footprint {
  TsmCoordSystem   coordsys;    /* which structure is defined */
  LatLongFootprint latlong;     /* lat/long boundary for region */
  UTMFootprint     utm;         /* UTM boundary for region */
  GeoFootprint     geo;         /* geocentric boundary for region */
  LVCSFootprint    lvcs;        /* LVCS extent */
};

/*typedef struct Footprint Footprint;*/


/* ----------------------------------------------------------------
 * identity.tspec
 * ----------------------------------------------------------------
 */

#ifdef RPC_HDR
%typedef tsmArray(int) intArray;
%typedef tsmArray(tspSymbol) tspSymbolArray;
%typedef tsmArray(charPtr) charPtrArray;
%typedef tsmOffsetArray(int) tsmChecksumArray;

%extern declare_xdr_tsmArray(intArray)
%extern declare_xdr_tsmArray(tspSymbolArray)
%extern declare_xdr_tsmArray(charPtrArray)
%extern declare_xdr_tsmOffsetArray(tsmChecksumArray)
#endif

#ifdef RPC_XDR
%define_xdr_tsmArray(intArray,int)
%define_xdr_tsmArray(tspSymbolArray,tspSymbol)
%define_xdr_tsmArray(charPtrArray,charPtr)
%define_xdr_tsmOffsetArray(tsmChecksumArray,int)
#endif

/*
 * Doc++ Description: *//** 
 *      Holds identity tile set specifications.
 *
 *   The identity tspec holds information about a pyramid such as its name,
 *   the original sources, the operations performed on the sources,
 *   the checksums, etc.
 *
 * @param    name (char *) the name of the pyramid
 * @param    setId (int) obsolete
 * @param    checkSumLrbt (tsmChecksumArray) 
 *           an array of checksums for the raw lrbt-raster
 *           versions of the tile data (contains an entry for each level
 *           of the pyramid plus one for the entire pyramid)
 * @param    checkSumJpeg (tsmChecksumArray)
 *           an array of checksums for the JPEG compressed
 *           versions of the tile data (contains an entry for each level
 *           of the pyramid plus one for the entire pyramid)
 * @param    type (tspSymbol) tspDemType, tspOiType, tspMapType, or 
 *           tspAerialImageType
 * @param    source_type (charPtr) a string describing the source image
 *           type, e.g. "unknown", "spot", "cir-napp"
 * @param    subtype (tspSymbol) tspNoneSubtype, tspUsgsSubtype,
 *           tspMosaicSubtype, or tspUsgsControlSubtype
 * @param    sources (charPtrArray) array of strings describing data sources
 * @param    operations (tspSymbolArray) array of tspSymbols describing
 *           the operations performed in the data. One of: tspMerge,
 *           tspWindow, tspTile, tspRectify, tspOiTrim, tspUtmToLvcs,
 *           tspMosaic, tspGaussReduce, tspTerraFormMosaic, tspScan,
 *           tspUsgsRectify, tspUsgsMosaic, tspGPyramid
 * @param    operationDate (char *) the date of the pyramid creation
 * @param    operationDoneBy (char *) who performed the pyramid creation
 *
 * @see      Pyramid
 *
 */

struct IdentityTspec
{
    /*int test<>; */
    string name<>;
    int setId;
    tsmChecksumArray checkSumLrbt;
    tsmChecksumArray checkSumJpeg;
    tspSymbol type;
    tspSymbol subtype;
    charPtr source_type;
/*    int numSources; */
    charPtrArray sources;
/*    int numOperations; */
    tspSymbolArray operations;
    string operationDate<>;
    string operationDoneBy<>;
};

/* ----------------------------------------------------------------
 * tile.tspec
 * ----------------------------------------------------------------
 */

/*
 * Doc++ Description: *//** 
 *   Holds tile tile-set specifications.
 *
 *   This structure specifies the storage format used to represent the
 *   pixels in a tile, as well as the pixel dimensions of the tile.
 *
 * @param    numPixelsX (int) the width of the tile in pixels
 * @param    numPixelsY (int) the height of the tile in pixels
 * @param    overLap (int[2]) the number of pixels of overlap in each
 *           tile in the x and y directions.
 * @param    totalPixels (int[2]) the sum of numPixelsX/Y and overlap[0]/[1]
 * @param    numComponentsPixel (int) the number of components per
 *           pixel, e.g RGB is 3 components
 * @param    numBytesPerComponent (int) the number of bytes per
 *           component, e.g. 1, 2, or 4.
 * @param    componentNumberType (tspSymbol) tspSignedByte or tspUnsignedByte
 * @param    storageFormat (tspSymbol) tspLrbtRaster or tspJPEGCompressed
 *
 * @see      PixelTspec, Pyramid
 *
 */

struct TileTspec
{
    int numPixelsX;
    int numPixelsY;
    int overLap[2];
    int totalPixels[2];
    int numComponentsPixel;
    int numBytesPerComponent;
    tspSymbol componentNumberType;
    tspSymbol storageFormat;
};

/* ----------------------------------------------------------------
 * 3d-coordinate-system.tspec
 * ----------------------------------------------------------------
 */

/*
 * Doc++ Description: *//** 
 *   Holds the 3d-coordinate tile-set specifications.
 *
 *  This structure holds information about the 3D coordinate system
 *  that is used to represent the tile data.
 *
 * @param    systemType (tspSymbol) currently only tspSpheroid
 * @param    spheroidName (tspSymbol) currently only tspNad83 or tspWGS84
 * @param    equatorialRadius (double) the equatorial radius of the
 *           spheroid (meters). This is usually called 'a' in USGS literature.
 * @param    polarRadius (double) the polar radius of the spheroid (meters).
 *           This is usually called 'b' in the USGS literature.
 * @param    geotoLVCS (float[16]) a 4x4 matrix to transform the geocentric
 *           coordinate system to a local coordinate system, or lcs. I.e.
 *           a point (x,y,z,1) in the geocentric coordinate system is
 *           transformed to the lcs by pre-multiplying with this matrix.
 * @param    lvcstoGeo (float[16]) the 4x4 inverse matrix of geotoLVCS.
 * @param    lcsType (tspSymbol) tspUtmCoordSystem, tspLvcsCoordSystem, tspNone
 * @param    originLat (float) the latitude of the origin (degrees).
 * @param    originLong (float) the longitude of the origin (degrees).
 * @param    originAltitude (float) the altitude of the origin (meters).
 * @param    localUnitsPerMeter (float) currently only 1.0
 * @param    componentNames (tspSymbolArray) currently only {tspX,tspY,tspZ}
 * @param    componentUnits (tspSymbol) currently only tspMeters or tspDegrees
 *
 * @see      Pyramid
 *
 */

struct CoordTspec
{
    tspSymbol systemType;
    tspSymbol spheroidName;
    double equatorialRadius;
    double polarRadius;
    float lvcstoGeo[16];
    float geotoLVCS[16];
    tspSymbol lcsType;
    float originLat;
    float originLong;
    float originAltitude;
    float localUnitsPerMeter;
    tspSymbolArray componentNames;
    tspSymbol componentUnits;
};

/* ----------------------------------------------------------------
 * pixel.tspec
 * ----------------------------------------------------------------
 */

/*
 * Doc++ Description: *//** 
 *   Holds the pixel tile-set specifications.
 *
 *   This structure provides information to read and examine the
 *   numerical data within a tile; although it is not sufficient for
 *   its interpretation and display.
 *
 * @param    componentNames (tspSymbolArray) the names of the components
 *           in the order they are stored for each pixel. Currently must
 *           be one of the set (tspRed, tspGreen, tspBlue,
 *           tspElevation, tspAverage).
 * @param    minVal (float) the smallest scaled pixel value within the
 *           entire tile-set (for all components). 
 * @param    maxVal (float) the largest scaled pixel value within the
 *           entire tile-set.
 * @param    scalingType (tspSymbol) currently only tspLinear.
 * @param    scale (float) before using pixel value, scale by this
 *           value then add the offset (below)
 * @param    offset (float) the offset to apply to the pixel before using.
 *
 * @see      TileTspec, Pyramid
 *
 */

struct PixelTspec
{
    tspSymbolArray componentNames;
    float minVal;
    float maxVal;
    tspSymbol scalingType;
    float scale;
    float offset;
};

/* ----------------------------------------------------------------
 * image-transformation.tspec
 * ----------------------------------------------------------------
 */

/* Changed name from TileSetImageTrans to ImageTransformationTspec */

/*
 * Doc++ Description: *//** 
 *   Holds the image transformation tile-set specifications.
 *
 *   This structure holds two 3x3 matrices that can be used to perform
 *   the transformation from the image's intrinsic 2-D coordinate system
 *   to its pixel coordinates, where (0,0) corresponds to the pixel in
 *   the lower left-hand corner.
 *
 *   N.B. the last row of both matrices is not used - i.e. should be 
 *   (0,0,1) - but it may be used in the future to allow for a final
 *   projective transformation.
 *
 * @param    uvtoIXIY (float[9]) a 3x3 matrix that pre-multiplies the
 *           vector (u,v,1) to yield (ix,iy)
 * @param    ixiytoUV (float[9]) a 3x3 matrix that pre-multiplies the
 *           vector (ix,iy,1) to yield (u,v)
 *
 * @see      Pyramid, ProjectionTspec
 *
 */

struct ImageTransformationTspec
{
    float uvtoIXIY[9];
    float ixiytoUV[9];
};

/* ----------------------------------------------------------------
 * projection.tspec
 * ----------------------------------------------------------------
 */

/*
 * Doc++ Description: *//** 
 *   Holds the projection tile-set specifications.
 *
 *   For convenience, the projective transformation (from Cartesian local
 *   coordinate system to the image's pixel coordinates) is specified as
 *   the product of two projections:
 *   <OL>
 *   <LI> a projection from the local 3-D coordinate to the
 *        image's intrinsic 2-D coordinates system (u,v) 
 *   <LI> a projection from (u,v) to the image's pixel coordinate
 *        system (ix,iy).
 *   </OL>
 *   This structure hold information about the first of these projections.
 *   (The ImageTransformationTspec holds information about the second.)
 *
 * @param    projectionType (tspSymbol) one of tspUtmProjection,
 *           tspAffineProjection, or tspLatLongProjection
 * @param    lcsToUvProjectiveMatrix (float[16]) the 4x4 affine
 *           projection matrix
 * @param    uvaToLcsMatrix (float[16]) the 4x4 inverse matrix of
 *           lcsToUvProjectiveMatrix
 * @param    inverseFocalLength (float) the inverse of the focal
 *           length of the camera. When this is zero, the projective
 *           transformation is orthographic and the piercing point is
 *           undefined.
 * @param    piercingPoint (float[2]) the (u,v) coordinate of the
 *           piercing point of the camera when inverse focal length is
 *           non-zero. Currently must be {0,0}.
 * @param    numCoeffs (int) the number of coefficents in the coeffs array
 * @param    coeffs (float[6]) the array of coefficents
 * @param    utmZoneNumber (int) the UTM zone number of the data
 * @param    centralMeridian (float) the central meridian of the data 
 * @param    k0 (float) the value of k0 (the ellipsoid flattening) for
 *           the planet
 * @param    geoidSeparation (float) distance between average sea
 *           level and the actual point on the planet's surface
 *
 * @see      ImageTransformationTspec, Pyramid
 *
 */

struct ProjectionTspec
{
    tspSymbol projectionType;
    float lcsToUvProjectiveMatrix[16];
    float uvaToLcsMatrix[16];
    float inverseFocalLength;
    float piercingPoint[2];
    int numCoeffs;
    float coeffs[6];
    int utmZoneNumber;
    float centralMeridian;
    float k0;
    float geoidSeparation;
};

/* ----------------------------------------------------------------
 * simple-tile-set
 * ----------------------------------------------------------------
 */

/* Changed name from TspecLevel to SimpleTileSet */
/* changed imageTransform to imageTransformationTspec */
/* Added isLocalFileTIFF to SimpleTileSet - reddy, 7 July 1997 */

/*
 * Doc++ Description: *//** 
 *    Holds all of the tile-set specifications for a Simple Tile Set.
 *
 *  A Simple Tile Set contains all of the information for the data in 
 *  a single level of a pyramid. 
 *
 * @param    minTile (int[2]) the minimum (tx,ty) tile coordinate, e.g. (0,0)
 * @param    maxTile (int[2]) the maximum (tx,ty) tile coordinate
 * @param    numTile (int[2]) maxTile[n] - minTile[n] + 1
 * @param    tileMapStatus (tspSymbol) tspTileMapEmpty or tspTileMapFull
 *           [obsolete]
 * @param    numPixels (int[2]) the total number of pixels in the
 *           mosaic in the ix and iy directions
 * @param    minValidPixel (int[2]) the minimum (ix,iy) image
 *           coordinate of the data in the mosaic
 * @param    maxValidPixel (int[2]) the maximum (ix,iy) image
 *           coodinate of the data in the mosaic (e.g. data could be
 *           padded with zeros to fill to tile boundary)
 * @param    numValidPixel (int[2]) maxValidPixel[n] - minValidPixel[n] + 1
 * @param    imageFileName (char *) the filename of the source mosaic image
 * @param    description (char *) textual description of the data
 * @param    imageTransformationTspec (ImageTransformationTspec) see link below
 * @param    identityTspec (IdentityTspec) see link below
 * @param    tileTspec (TileTspec) see link below
 * @param    pixelTspec (PixelTspec) see link below
 * @param    projectionTspec (ProjectionTspec) see link below
 * @param    coordTspec (CoordTspec) see link below
 *
 * @see      ImageTransformationTspec, IdentityTspec, TileTspec, PixelTspec,  
 *           ProjectionTspec, CoordTspec, Pyramid
 *
 */

struct SimpleTileSet
{
    int minTile[2];
    int maxTile[2];
    int numTile[2];
    tspSymbol tileMapStatus;
    int numPixels[2];
    int minValidPixel[2];
    int maxValidPixel[2];
    int numValidPixel[2];
    string imageFileName<>;
    string description<>;
    ImageTransformationTspec imageTransformationTspec;
    IdentityTspec identityTspec;
    TileTspec tileTspec;
    PixelTspec pixelTspec;
    ProjectionTspec projectionTspec;
    CoordTspec coordTspec;
};

/* ----------------------------------------------------------------
 * pyramid-tile-set
 * ----------------------------------------------------------------
 */

/* Changed name from Tspec to Pyramid--returned in dsmGetTspec() */
/* TOP -hmh */

#ifdef RPC_HDR
%typedef tsmOffsetArray(SimpleTileSet) SimpleTileSetArray;
%extern declare_xdr_tsmOffsetArray(SimpleTileSetArray)
#endif

#ifdef RPC_XDR
%define_xdr_tsmOffsetArray(SimpleTileSetArray,SimpleTileSet)
#endif

/*
 * Doc++ Description: *//** 
 *   Holds the tspec information for a Pyramid.
 *
 *   This structure holds all of the information to descibe the data
 *   in an entire pyramid. The tspec file tiles.tspec gets mapped into
 *   this data structure using the tsmReadTspec function, which is
 *   called implicitly for you when you do a tsmConnect.
 *
 * @param    pathName (char *) the pathname of the pyramid
 * @param    tspecBasePurl (Purl *) the parsed URL of the tspec file(s)
 * @param    tileBasePurl (Purl *) the parsed URL of the tile data
 * @param    invResScaleFactor (float) currently 2.0
 * @param    minLevel (int) the starting level of the pyramid (highest res)
 * @param    maxLevel (int) the final level of the pyramid (lowest res)
 * @param    minResolution (float) the resolution of minLevel
 * @param    maxResolution (float) the resolution of maxLevel
 * @param    imageHierarchyMinValidLevel (int)
 * @param    imageHierarchyMaxValidLevel (int)
 * @param    imageHierarchyFileName (char *) the filename of the
 *           original mosaic image
 * @param    description (char *) textual description of the pyramid data set
 * @param    tspecLevel (SimpleTileSetArray) an array of SimpleTileSet
 *           structures, one for each pyramid level - see link below
 * @param    identityTspec (IdentityTspec) see link below
 * @param    tileTspec (TileTspec) see link below
 * @param    pixelTspec (PixelTspec) see link below
 * @param    projectionTspec (ProjectionTspec) see link below
 * @param    coordTspec (CoordTspec) see link below
 * @param    rectification (TsmCoordSystem) one of TSM_NONE_COORDSYS,
 *           TSM_LATLONG_COORDSYS, TSM_UTM_COORDSYS, or TSM_GEO_COORDSYS
 * @param    rectificationGrid (Purl *) if rectifcation is TSM_NONE_COORDSYS,
 *           then this gives the URL for a rectification grid
 * @param    footprint (Footprint) the footprint of the pyramid (the 4
 *           boundary corners)
 * @param    geoName (char *) a string containing the name of the
 *           GeoSet for this dataset. The geoName for
 *           complementary OI and DEM datasets should be the same.
 * @param    wrap_lat (int) set to TRUE if the dataset wraps around
 *           the world vertically, i.e. extends from -90 to +90 deg lat.
 * @param    wrap_lon (int) set to TRUE if the dataset wraps around
 *           the world horizontally, i.e. extends from -180 to +180
 *           deg long.
 * @param    planet (TsmPlanet) defines the planet that this dataset
 *           is on, e.g. TSM_EARTH, TSM_MARS, TSM_VENUS, etc.
 * @param    sky is TRUE only if this tileset is intended to be rendered
 *           as a sky background image..
 * @param    twoSided is TRUE only if this tileset should be visible both
 *           from above and below.
 * @param    fromBelow is TRUE if this tileset should be visible from below.
 *           This is ignored if twoSided is TRUE.
 *
 * @see      ImageTransformationTspec, IdentityTspec, TileTspec, PixelTspec,  
 *           ProjectionTspec, CoordTspec, SimpleTileSet, Footprint,
 *           tsmAllocTspec, tsmFreeTspec, tsmCopyPyramid
 *
 */

struct Pyramid     /* Test this first!! */
{
    int type;
    string pathName<>;
    Purl *tspecBasePurl;
    Purl *tileBasePurl;
    float invResScaleFactor;
    int minLevel;
    float minResolution;
    int maxLevel;
    float maxResolution;
    int imageHierarchyMinValidLevel;
    int imageHierarchyMaxValidLevel;
    string imageHierarchyFileName<>;
    string description<>;
    SimpleTileSetArray tspecLevel;
    IdentityTspec identityTspec;
    TileTspec tileTspec;
    PixelTspec pixelTspec;
    ProjectionTspec projectionTspec;
    CoordTspec coordTspec;
    int tileSize;
    TsmCoordSystem rectification;
    Purl *rectificationGrid;
    Footprint footprint;
    int wrap_lat;
    int wrap_lon;
    int sky;
    int twoSided;
    int fromBelow;
    string geoName<>;
    TsmPlanet planet;
    string mapName<>;  /* obsolete */
    double mapMinU;    /* obsolete */
    double mapMaxU;    /* obsolete */
    double mapMinV;    /* obsolete */
    double mapMaxV;    /* obsolete */
};

typedef Pyramid TileSetTspec;

#ifdef RPC_HDR
%typedef tsmArray(Pyramid) PyramidArray;
%extern declare_xdr_tsmArray(PyramidArray)
#endif

#ifdef RPC_XDR
%define_xdr_tsmArray(PyramidArray,Pyramid)
#endif

/* ----------------------------------------------------------------
 * geo-pyramid-tile-set tile-set.tspec
 * ----------------------------------------------------------------
 */

struct NameUrlPairs
{
    charPtrArray names;
    charPtrArray urls;
};

#ifdef RPC_HDR
%#endif
#endif

