/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsCoords.c,v 2.16.2.6 2002/10/31 05:26:38 reddy Exp $

  ----- BEGIN LICENSE BLOCK -----
  Version: MPL 1.1
 
  The contents of this file are subject to the Mozilla Public License Version
  1.1 (the "License"); you may not use this file except in compliance with
  the License. You may obtain a copy of the License at
  http://www.mozilla.org/MPL/
 
  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  for the specific language governing rights and limitations under the
  License.
 
  The Original Code is the TerraVision 3D Terrain Visualization System.
 
  The Initial Developer of the Original Code is SRI International.

  Portions created by the Initial Developer are Copyright (C) 1992-2002
  the Initial Developer. All Rights Reserved.
 
  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
  ----- END LICENSE BLOCK -----

*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Trace.h"

#include "TsCoords.h"

#ifndef HAVE_INLINE
# define exinline
# include "TsCoordsInline.h"
#endif

#include <prinit.h>  /* for PR_CallOnce() */

#include <math.h>
#include <float.h>
#include <vrml2/quat.h>
#include <vrml2/vec3.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

static void TsCacheSetOrientation (TsCache *cache,
				   float angle, float x, float y, float z);

static SE_CONVERT_COORD_SYSTEM_PAIR gcc_to_gdc;

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Geocentric coordinates to 
 *   the Geodetic coordinate system.
 *
 *   Geocentric coordinates (GCC) are <x, y, z> tuples that represent distances
 *   from the planet's center.  Geodetic coordinates (GDC) are tuples
 *   <longitude, latitude, elevation>.
 *
 * @param x    coordinate to be transformed.
 * @param y    
 * @param z    
 * @param c[3] returned transformed coordinate values.
 *
 * @return  1 on successful conversion;  0 on error condition.
 *
 */
int
TsGCCtoGDC (double x, double y, double z, 
	    double c[3])
{
  static char const funcName[] = "TsGCCtoGDC";
  SE_COORD gcc;
  SE_COORD gdc;

  gcc.is_2D = FALSE;
  gcc.u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;
  gcc.u.coord_3d.u.gc_coord.x = x;
  gcc.u.coord_3d.u.gc_coord.y = y;
  gcc.u.coord_3d.u.gc_coord.z = z;
  if (SE_ConvertCoordToGivenSystem (gcc_to_gdc, &gcc, &gdc)
      != SE_COORD_SUCCESS) {
    TSMTRACE ((CACHE_MASK|1, funcName, "FAIL: %.0f,%.0f,%.0f",
	       x, y, z));
    return 0;
  }
  c[0] = gdc.u.coord_3d.u.gd_coord.geodetic_longitude;
  c[1] = gdc.u.coord_3d.u.gd_coord.geodetic_latitude;
  c[2] = gdc.u.coord_3d.u.gd_coord.elevation;
  return 1;
}

#define QSPHERE (90.0F)
#define HSPHERE (180.0F)
#define SPHERE (360.0F)

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Normalize a geodetic coordinate value by ensuring
 *   that longitude is between -180 and 180 degrees and latitude is between
 *   -90 and 90 degrees. 
 *
 *   Geodetic coordinates (GDC) are tuples <longitude, latitude, elevation>.
 *
 * @param lng  coordinate to be normalized.
 * @param lat  
 *
 * @return  void
 *
 */
void
TsNormalizeGDC (double *lngp, double *latp)
{
  double lat = *latp, lng = *lngp;
  if (lat < -QSPHERE || lat > QSPHERE) {
    int lat_i;
    lat += QSPHERE;
    lat_i = floor (lat/HSPHERE);
    lat -= lat_i*HSPHERE;
    if (lat_i%2 == 0) {
      lat -= QSPHERE;
    } else {
      lat = QSPHERE - lat;
      lng += HSPHERE;
      *lngp = lng;
    }
    *latp = lat;
  }

  if (lng < -HSPHERE || lng > HSPHERE) {
    int lng_i;
    lng += HSPHERE;
    lng_i = floor (lng/SPHERE);
    lng -= lng_i*SPHERE;
    lng -= HSPHERE;
    *lngp = lng;
  }
}

static SE_CONVERT_COORD_SYSTEM_PAIR gdc_to_gcc;

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Geodetic coordinates to 
 *   the Geocentric coordinate system.
 *
 *   Geocentric coordinates (GCC) are <x, y, z> tuples that represent distances
 *   from the planet's center.  Geodetic coordinates (GDC) are tuples
 *   <longitude, latitude, elevation>.
 *
 * @param lng  coordinate to be transformed.
 * @param lat    
 * @param z    
 * @param c[3] transformed coordinate values.
 *
 * @return  1 on successful conversion;  0 on error condition.
 *
 */
int
TsGDCtoGCC (double lng, double lat, double z, double c[3])
{
  static char const funcName[] = "TsGDCtoGCC";
  SE_COORD gdc;
  SE_COORD gcc;
  
  TsNormalizeGDC (&lng, &lat);

  gdc.is_2D = FALSE;
  gdc.u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.coord_3d.u.gd_coord.geodetic_longitude = lng;
  gdc.u.coord_3d.u.gd_coord.geodetic_latitude  = lat;
  gdc.u.coord_3d.u.gd_coord.elevation = z;
  if (SE_ConvertCoordToGivenSystem (gdc_to_gcc, &gdc, &gcc)
      != SE_COORD_SUCCESS) {
    TSMTRACE ((CACHE_MASK|1, funcName, "FAIL: %.2f,%.2f,%.0f",
	       lng, lat, z));
    return 0;
  }
  c[0] = gcc.u.coord_3d.u.gc_coord.x;
  c[1] = gcc.u.coord_3d.u.gc_coord.y;
  c[2] = gcc.u.coord_3d.u.gc_coord.z;
  return 1;
}

static PRCallOnceType TsGlobalCoordsOnce; /* Implicit zero initialization */

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Initialize the global coordinate system
 *   conversion functions.
 *
 * @return  PR_SUCCESS on successful initialization;
 *          PR_FAILURE  on error condition.
 *
 */
static PRStatus PR_CALLBACK
TsGlobalCoordsInit (void)
{
  SE_SRF_PARAMETERS gcc;
  SE_SRF_PARAMETERS gdc;

  gcc.is_2D = FALSE;
  gcc.u.params_3d.spatial_reference_frame = SRM_GC_3D_SRF;

  gdc.is_2D = FALSE;
  gdc.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.params_3d.u.gd_parameters.horizontal_datum = SE_W84_HDATUM;
  gdc.u.params_3d.u.gd_parameters.vertical_datum   = SE_WGS84E_VDATUM;
  gdc.u.params_3d.u.gd_parameters.elevation_units  = SRM_METERS;

  if (SE_CreateCoordConversionConstants (&gcc, &gdc, &gcc_to_gdc)
      != SE_COORD_SUCCESS)
    return PR_FAILURE;
  if (SE_CreateCoordConversionConstants (&gdc, &gcc, &gdc_to_gcc)
      != SE_COORD_SUCCESS)
    return PR_FAILURE;
  return PR_SUCCESS;
}


/*
 * These conversions go from dataset-specific to global and back.
 */

static int
TsCacheUVtoGCC_GDC (TsCache const *cache,
		    float uf, float vf, float zf,
		    double c[3])
{
  static const char funcName[] = "TsCacheUVtoGCC_GDC";

  double const *origin = TsCacheOrigin (cache);
  SE_COORD gdc;
  SE_COORD gcc;

  double u = uf + origin[0];
  double v = vf + origin[1] ;
  double z = zf;

  TsNormalizeGDC (&u, &v);

  gdc.is_2D = FALSE;
  gdc.u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.coord_3d.u.gd_coord.geodetic_longitude = u;
  gdc.u.coord_3d.u.gd_coord.geodetic_latitude  = v;
  gdc.u.coord_3d.u.gd_coord.elevation = z;
  if (SE_ConvertCoordToGivenSystem (cache->UVtoGCC_cs[0], &gdc, &gcc)
      != SE_COORD_SUCCESS)
    return 0;
  c[0] = gcc.u.coord_3d.u.gc_coord.x;
  c[1] = gcc.u.coord_3d.u.gc_coord.y;
  c[2] = gcc.u.coord_3d.u.gc_coord.z;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "UV: GDC %.2f,%.2f,%.2f => GCC %.0f,%.0f,%.0f",
	     uf, vf, zf, c[0], c[1], c[2]));

  return 1;
}

static int
TsCacheGCCtoUV_GDC (TsCache const *cache,
		    double x, double y, double z, float uv[3])
{
  static const char funcName[] = "TsCacheGCCtoUV_GDC";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD gdc;
  SE_COORD gcc;
  double c[3];

  gcc.is_2D = FALSE;
  gcc.u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;
  gcc.u.coord_3d.u.gc_coord.x = x;
  gcc.u.coord_3d.u.gc_coord.y = y;
  gcc.u.coord_3d.u.gc_coord.z = z;
  if (SE_ConvertCoordToGivenSystem (cache->GCCtoUV_cs[0], &gcc, &gdc)
      != SE_COORD_SUCCESS)
    return 0;

  c[0] = gdc.u.coord_3d.u.gd_coord.geodetic_longitude;
  c[1] = gdc.u.coord_3d.u.gd_coord.geodetic_latitude;
  c[2] = gdc.u.coord_3d.u.gd_coord.elevation;

  uv[0] = c[0] - origin[0];
  uv[1] = c[1] - origin[1];
  uv[2] = c[2];

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "GCC %.0f,%.0f,%.0f => UV: GDC %.2f,%.2f,%.2f",
	     x, y, z, uv[0], uv[1], uv[2]));

  return 1;
}

static int
TsCacheUVtoGDC_GDC_fast (TsCache const *cache,
			 float uf, float vf, float zf,
			 double c[3])
{
  static const char funcName[] = "TsCacheUVtoGDC_GDC_fast";
  double const *origin = TsCacheOrigin (cache);

  double u = uf + origin[0];
  double v = vf + origin[1] ;
  double z = zf;

  TsNormalizeGDC (&u, &v);

  c[0] = u;
  c[1] = v;
  c[2] = z;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "UV: GDC %.2f,%.2f,%.2f => GDC %.2f,%.2f,%.0f",
	     uf, vf, zf, c[0], c[1], c[2]));

  return 1;
}

static int
TsCacheUVtoGDC_GDC (TsCache const *cache,
		    float uf, float vf, float zf,
		    double c[3])
{
  static const char funcName[] = "TsCacheUVtoGDC_GDC";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD gdci;
  SE_COORD gcc;
  SE_COORD gdco;

  double u = uf + origin[0];
  double v = vf + origin[1] ;
  double z = zf;

  TsNormalizeGDC (&u, &v);

  gdci.is_2D = FALSE;
  gdci.u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdci.u.coord_3d.u.gd_coord.geodetic_longitude = u;
  gdci.u.coord_3d.u.gd_coord.geodetic_latitude  = v;
  gdci.u.coord_3d.u.gd_coord.elevation = z;

  if (SE_ConvertCoordToGivenSystem (cache->UVtoGCC_cs[0], &gdci, &gcc)
      != SE_COORD_SUCCESS)
    return 0;
  if (SE_ConvertCoordToGivenSystem (cache->UVtoGCC_cs[1], &gcc, &gdco)
      != SE_COORD_SUCCESS)
    return 0;

  c[0] = gdco.u.coord_3d.u.gd_coord.geodetic_longitude;
  c[1] = gdco.u.coord_3d.u.gd_coord.geodetic_latitude;
  c[2] = gdco.u.coord_3d.u.gd_coord.elevation;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "UV: GDC %.2f,%.2f,%.2f => GDC %.2f,%.2f,%.0f",
	     uf, vf, zf, c[0], c[1], c[2]));

  return 1;
}

static int
TsCacheGDCtoUV_GDC_fast (TsCache const *cache,
			 double lng, double lat, double z, float uv[3])
{
  static const char funcName[] = "TsCacheGDCtoUV_GDC_fast";
  double const *origin = TsCacheOrigin (cache);

  uv[0] = lng - origin[0];
  uv[1] = lat - origin[1];
  uv[2] = z;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "GDC %.2f,%.2f,%.0f => UV: GDC %.2f,%.2f,%.2f",
	     lng, lat, z, uv[0], uv[1], uv[2]));

  return 1;
}

static int
TsCacheGDCtoUV_GDC (TsCache const *cache,
		    double lng, double lat, double z, float uv[3])
{
  static const char funcName[] = "TsCacheGDCtoUV_GDC";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD gdc;
  SE_COORD gcc;

  gdc.is_2D = FALSE;
  gdc.u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.coord_3d.u.gd_coord.geodetic_longitude = lng;
  gdc.u.coord_3d.u.gd_coord.geodetic_latitude  = lat;
  gdc.u.coord_3d.u.gd_coord.elevation = z;

  if (SE_ConvertCoordToGivenSystem (cache->GDCtoUV_cs[0], &gdc, &gcc)
      != SE_COORD_SUCCESS)
    return 0;
  if (SE_ConvertCoordToGivenSystem (cache->GDCtoUV_cs[1], &gcc, &gdc)
      != SE_COORD_SUCCESS)
    return 0;

  uv[0] = gdc.u.coord_3d.u.gd_coord.geodetic_longitude - origin[0];
  uv[1] = gdc.u.coord_3d.u.gd_coord.geodetic_latitude  - origin[1];
  uv[2] = gdc.u.coord_3d.u.gd_coord.elevation;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "GDC %.2f,%.2f,%.0f => UV: GDC %.2f,%.2f,%.2f",
	     lng, lat, z, uv[0], uv[1], uv[2]));

  return 1;
}

static int
TsCacheUVtoGCC_UTM (TsCache const *cache,
		    float uf, float vf, float zf,
		    double c[3])
{
  static const char funcName[] = "TsCacheUVtoGCC_UTM";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD utm;
  SE_COORD gdc;
  SE_COORD gcc;
  SE_COORD_STATUS_CODE_ENUM ret;

  double u = uf + origin[0];
  double v = vf + origin[1] ;
  double z = zf;

  int north = !cache->southern_utm;
  if ( v < 0 ) v = -v;
  
  utm.is_2D = FALSE;
  utm.u.coord_3d.spatial_reference_frame = SRM_UTM_3D_SRF;
  utm.u.coord_3d.u.utm_coord.x = u;
  utm.u.coord_3d.u.utm_coord.y = v;
  utm.u.coord_3d.u.utm_coord.z = z;
  utm.u.coord_3d.u.utm_coord.hemisphere = 
    (north ? SE_NORTHERN_HEMISPHERE : SE_SOUTHERN_HEMISPHERE);

  ret = SE_ConvertCoordToGivenSystem (cache->UVtoGCC_cs[0], &utm, &gdc);
  if (ret != SE_COORD_SUCCESS
      && ret != SE_COORD_EXTENDED_SOURCE_COORD
      && ret != SE_COORD_EXTENDED_DEST_COORD
      && ret != SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD) {
    TSMTRACE ((CACHE_MASK|1, funcName, "%d:%g,%g,%g %c. FAIL",
	       cache->utm_zone, u, v, z, north ? 'N' : 'S'));
    return 0;
  }
  if (SE_ConvertCoordToGivenSystem (cache->UVtoGCC_cs[1], &gdc, &gcc)
      != SE_COORD_SUCCESS) {
    TSMTRACE ((CACHE_MASK|1, funcName, "%d:%g,%g,%g %c. FAIL",
	       cache->utm_zone, u, v, z, north ? 'N' : 'S'));
    return 0;
  }

  c[0] = gcc.u.coord_3d.u.gc_coord.x;
  c[1] = gcc.u.coord_3d.u.gc_coord.y;
  c[2] = gcc.u.coord_3d.u.gc_coord.z;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "UV: UTM%d %.2f,%.2f,%.2f => GCC %.2f,%.2f,%.0f",
	     cache->utm_zone, u, v, z, c[0], c[1], c[2]));
  return 1;
}

static int
TsCacheGCCtoUV_UTM (TsCache const *cache,
		    double x, double y, double z,
		    float uv[3])
{
  static const char funcName[] = "TsCacheGCCtoUV_UTM";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD utm;
  SE_COORD gdc;
  SE_COORD gcc;
  SE_COORD_STATUS_CODE_ENUM ret;

  gcc.is_2D = FALSE;
  gcc.u.coord_3d.spatial_reference_frame = SRM_GC_3D_SRF;
  gcc.u.coord_3d.u.gc_coord.x = x;
  gcc.u.coord_3d.u.gc_coord.y = y;
  gcc.u.coord_3d.u.gc_coord.z = z;

  if (SE_ConvertCoordToGivenSystem (cache->GCCtoUV_cs[0], &gcc, &gdc)
      != SE_COORD_SUCCESS)
    return 0;
  ret = SE_ConvertCoordToGivenSystem (cache->GCCtoUV_cs[1], &gdc, &utm);
  if (ret != SE_COORD_SUCCESS && ret != SE_COORD_EXTENDED_DEST_COORD)
    return 0;

  uv[0] = utm.u.coord_3d.u.utm_coord.x - origin[0];
  uv[2] = utm.u.coord_3d.u.utm_coord.z;

  if ( cache->southern_utm == TRUE && 
    gdc.u.coord_3d.u.gd_coord.geodetic_latitude > 0.0 ) {
    /* in northern hemi for a southern hemi dataset */
    uv[1] = 10000000 - origin[1] + utm.u.coord_3d.u.utm_coord.y;
  } else if ( cache->southern_utm == FALSE && 
    gdc.u.coord_3d.u.gd_coord.geodetic_latitude < 0.0 ) {
    /* in southern hemi for a northern hemi dataset */
    uv[1] = -origin[1] - (10000000 - utm.u.coord_3d.u.utm_coord.y);
  } else {
    /* no special case with crossing the equator funniness */
    uv[1] = utm.u.coord_3d.u.utm_coord.y - origin[1];
  }

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "GCC %.2f,%.2f,%.0f => UV: GDC %.2f,%.2f,%.2f",
	     x, y, z, uv[0], uv[1], uv[2]));

  return 1;
}

static int
TsCacheUVtoGDC_UTM (TsCache const *cache,
		    float uf, float vf, float zf,
		    double c[3])
{
  static const char funcName[] = "TsCacheUVtoGDC_UTM";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD utm;
  SE_COORD gdc;
  SE_COORD_STATUS_CODE_ENUM ret;

  double u = uf + origin[0];
  double v = vf + origin[1] ;
  double z = zf;

  int north = !cache->southern_utm;
  if ( v < 0 ) v = -v;

  utm.is_2D = FALSE;
  utm.u.coord_3d.spatial_reference_frame = SRM_UTM_3D_SRF;
  utm.u.coord_3d.u.utm_coord.x = u;
  utm.u.coord_3d.u.utm_coord.y = v;
  utm.u.coord_3d.u.utm_coord.z = z;
  utm.u.coord_3d.u.utm_coord.hemisphere = 
    (north ? SE_NORTHERN_HEMISPHERE : SE_SOUTHERN_HEMISPHERE);

  ret = SE_ConvertCoordToGivenSystem (cache->UVtoGDC_cs[0], &utm, &gdc);
  if (ret != SE_COORD_SUCCESS
      && ret != SE_COORD_EXTENDED_SOURCE_COORD
      && ret != SE_COORD_EXTENDED_DEST_COORD
      && ret != SE_COORD_EXTENDED_SOURCE_AND_DEST_COORD) {
    TSMTRACE ((CACHE_MASK|1, funcName, "%d:%g,%g,%g FAIL",
	       cache->utm_zone, u, v, z));
    return 0;
  }

  c[0] = gdc.u.coord_3d.u.gd_coord.geodetic_longitude;
  c[1] = gdc.u.coord_3d.u.gd_coord.geodetic_latitude;
  c[2] = gdc.u.coord_3d.u.gd_coord.elevation;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "UV: UTM%d %.2f,%.2f,%.2f => GDC %.2f,%.2f,%.0f",
	     cache->utm_zone, uf, vf, zf, c[0], c[1], c[2]));

  return 1;
}

static int
TsCacheGDCtoUV_UTM (TsCache const *cache,
		    double lng, double lat, double z,
		    float uv[3])
{
  static const char funcName[] = "TsCacheGDCtoUV_UTM";
  double const *origin = TsCacheOrigin (cache);
  SE_COORD utm;
  SE_COORD gdc;
  SE_COORD_STATUS_CODE_ENUM ret;

  gdc.is_2D = FALSE;
  gdc.u.coord_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.coord_3d.u.gd_coord.geodetic_longitude = lng;
  gdc.u.coord_3d.u.gd_coord.geodetic_latitude = lat;
  gdc.u.coord_3d.u.gd_coord.elevation = z;

  ret = SE_ConvertCoordToGivenSystem (cache->GDCtoUV_cs[0], &gdc, &utm);
  if (ret != SE_COORD_SUCCESS && ret != SE_COORD_EXTENDED_DEST_COORD)
    return 0;

  uv[0] = utm.u.coord_3d.u.utm_coord.x - origin[0];
  if ( utm.u.coord_3d.u.utm_coord.y > 0.0 )
    uv[1] = utm.u.coord_3d.u.utm_coord.y - origin[1];
  else
    uv[1] = -utm.u.coord_3d.u.utm_coord.y - origin[1];
  uv[2] = utm.u.coord_3d.u.utm_coord.z;

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "GDC %.2f,%.2f,%.0f => UV: UTM%d %.2f,%.2f,%.2f",
	     lng, lat, z, cache->utm_zone, uv[0], uv[1], uv[2]));

  return 1;
}

static int
TsCacheUVtoGDC_twostage (TsCache const *cache,
			 float uf, float vf, float zf,
			 double g[3])
{
  static const char funcName[] = "TsCacheUVtoGDC_twostage";
  double c[3];
  int ret = FALSE;

  if (TsCacheUVtoGCC (cache, uf, vf, zf, c)) {
    ret = TsGCCtoGDC (c[0], c[1], c[2], g);
    if (! ret) {
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "GCC: %.0f,%.0f,%.0f => GDC FAIL", c[0], c[1], c[2]));
    }
  } else {
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "UV: %.2f,%.2f,%.2f => GCC FAIL", uf, vf, zf));
  }

  if (ret)
    TSMTRACE ((CACHE_MASK|4, funcName,
	       "UV: %.2f,%.2f,%.2f => GDC %.2f,%.2f,%.0f",
	       uf, vf, zf, g[0], g[1], g[2]));

  return ret;
}

static int
TsCacheGDCtoUV_twostage (TsCache const *cache,
			 double lng, double lat, double z,
			 float uv[3])
{
  static const char funcName[] = "TsCacheGDCtoUV_twostage";
  double c[3];
  int ret = FALSE;

  if (TsGDCtoGCC (lng, lat, z, c)) {
    ret = TsCacheGCCtoUV (cache, c[0], c[1], c[2], uv);
    if (! ret)
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "GCC: %.0f,%.0f,%.0f => UV FAIL", c[0], c[1], c[2]));
  } else {
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "GDC: %.2f,%.2f,%.2f => GCC FAIL", lng, lat, z));
  }

  if (ret)
    TSMTRACE ((CACHE_MASK|4, funcName,
	       "GDC %.2f,%.2f,%.0f => UV: .2f,%.2f,%.2f",
	       lng, lat, z, uv[0], uv[1], uv[2]));

  return ret;
}

static int
TsCacheUVtoGCC_twostage (TsCache const *cache,
			 float uf, float vf, float zf,
			 double g[3])
{
  static const char funcName[] = "TsCacheUVtoGCC_twostage";
  double c[3];
  int ret = FALSE;

  if (TsCacheUVtoGDC (cache, uf, vf, zf, c)) {
    ret = TsGDCtoGCC (c[0], c[1], c[2], g);
    if (! ret)
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "GDC: %.2f,%.2f,%.0f => GCC FAIL", c[0], c[1], c[2]));
  } else {
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "UV: %.2f,%.2f,%.2f => GDC FAIL", uf, vf, zf));
  }

  if (ret)
    TSMTRACE ((CACHE_MASK|4, funcName,
	       "UV: %.2f,%.2f,%.2f => GCC %.2f,%.2f,%.0f",
	       uf, vf, zf, g[0], g[1], g[2]));

  return ret;
}

static int
TsCacheGCCtoUV_twostage (TsCache const *cache,
			 double x, double y, double z,
			 float uv[3])
{
  static const char funcName[] = "TsCacheGCCtoUV_twostage";
  double c[3];
  int ret = FALSE;

  if (TsGCCtoGDC (x, y, z, c)) {
    ret = TsCacheGDCtoUV (cache, c[0], c[1], c[2], uv);
    if (! ret)
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "GDC: %.2f,%.2f,%.0f => UV FAIL", c[0], c[1], c[2]));
  } else {
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "GCC: %.2f,%.2f,%.2f => GDC FAIL", x, y, z));
  }

  if (ret)
    TSMTRACE ((CACHE_MASK|4, funcName,
	       "GCC %.2f,%.2f,%.0f => UV: .2f,%.2f,%.2f",
	       x, y, z, uv[0], uv[1], uv[2]));

  return ret;
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Determine the angle and axis form of the rotation
 *   needed to describe an LVCS coordinate frame in GCC coordinate space.  The
 *   orientation of the LVCS coordinate frame is determined based entirely on
 *   the origin of the LVCS space.
 *
 *   The Local Vertical coordinate system (LVCS) is a localized, arbitrary 
 *   coordinate space;  points are represented by the tuple (x,y,z), with 
 *   the Z axis oriented in the UP direction.  Geocentric coordinates (GCC) 
 *   are <x, y, z> tuples that represent distances from the planet's center.
 *
 * @param origin[3]  origin of the LVCS space defined in GCC coordinates.
 * @param orient[4]  the angle and axis of the rotation required to go from
 *                   the geocentricframe to LVCS.
 *
 * @return int
 *
 */
int
TsLVCSOrientation (double const origin[3], float orient[4])
{
  static char const funcName[] = "TsLVCSOrientation";
  v3f axis;
  quat qx, qz, qr;
  float angle;
  double gdc[3];

  if ( ! TsGCCtoGDC (origin[0], origin[1], origin[2], gdc) )
    return FALSE;

  TSMTRACE ((CACHE_MASK|1, funcName, "   => GDC %.3f,%.3f,%.0f",
	     gdc[0], gdc[1], gdc[2]));

  rotation_quat (0, 0, 1, (90.0 + gdc[0]), &qz);
  rotation_quat (1, 0, 0, (90.0 - gdc[1]), &qx);
  add_quat (&qx, &qz, &qr);
  angle = quat_angle (&qr, &axis);

  TSMTRACE ((CACHE_MASK|1, funcName, "   => theta %.0f: %.3f,%.3f,%.3f",
	     angle, V(axis)[0], V(axis)[1], V(axis)[2]));
  orient[0] = angle;
  orient[1] = V(axis)[0];
  orient[2] = V(axis)[1];
  orient[3] = V(axis)[2];

  return TRUE;
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Local Vertical coordinates 
 *   to the Geocentric coordinate system.
 *
 *   The Local Vertical coordinate system (LVCS) is a localized, arbitrary 
 *   coordinate space;  points are represented by the tuple (x,y,z), with 
 *   the Z axis oriented in the UP direction.  Geocentric coordinates (GCC) 
 *   are <x, y, z> tuples that represent distances from the planet's center.
 *
 * @param cache   reference tileset cache
 * @param l0      input coordinate in LVCS space.
 * @param l1      "
 * @param l2      "
 * @param g[3]    returned transformed coordinate in GCC space.
 *
 * @return int
 *
 */
int
TsCacheLVCStoGCC (TsCache const *cache,
		  float l0, float l1, float l2, double g[3])
{
  static char const funcName[] = "TsGeoLVCStoGCC";
  v3f v, vg;
  int i;
  V(v)[0] = l0; V(v)[1] = l1; V(v)[2] = l2;
  mult_m3f_v3f (&cache->lvcs_rotate, &v, &vg);
  for (i = 0; i < 3; ++i)
    g[i] = V(vg)[i] + cache->lvcs_origin[i];
  TSMTRACE ((CACHE_MASK|4, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     l0, l1, l2, g[0], g[1], g[2]));
  return 1;
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in the TsCache's Local
 *   Vertical coordinate system to Geocentric coordinates.
 *   .
 *
 *   The Local Vertical coordinate system (LVCS) is a localized, arbitrary
 *   coordinate space; points are represented by the tuple (x,y,z), with the Z
 *   axis oriented in the UP direction. Geodetic coordinates (GDC) are tuples
 *   <longitude, latitude, elevation>.
 *
 *
 * @param cache   reference tileset cache
 * @param l0      input coordinate in LVCS space.
 * @param l1      "
 * @param l2      "
 * @param g[3]    returned transformed coordinate in Geodetic space.
 *
 * @return success
 * */
int
TsCacheLVCStoGDC (TsCache const *cache,
		  float l0, float l1, float l2, double g[3])
{
  /*static char const funcName[] = "TsCacheLVCStoGDC";*/
  double tmp[3];
  TsCacheLVCStoGCC (cache, l0, l1, l2, tmp);
  return TsGCCtoGDC (tmp[0], tmp[1], tmp[2], g);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Geocentric coordinates to 
 *   the TsCache's Local Vertical coordinate system.
 *
 *   Geocentric coordinates (GCC) are <x, y, z> tuples that represent 
 *   distances from the planet's center. The Local Vertical coordinate system 
 *   (LVCS) is a localized, arbitrary coordinate space;  points are represented
 *    by the tuple (x,y,z), with the Z axis oriented in the UP direction.  
 *
 *
 * @param cache   reference tileset cache
 * @param g0      input coordinate in GCC space.
 * @param g1      "
 * @param g2      "
 * @param l[3]    returned transformed coordinate in LVCS space.
 *
 * @return success
 *
 */
int
TsCacheGCCtoLVCS (TsCache const *cache,
		  double g0, double g1, double g2, float l[3])
{
  static char const funcName[] = "TsCacheGCCtoLVCS";
  v3f v;
  V(v)[0] = g0 - cache->lvcs_origin[0];
  V(v)[1] = g1 - cache->lvcs_origin[1];
  V(v)[2] = g2 - cache->lvcs_origin[2];

  mult_v3f_m3f (&v, &cache->lvcs_rotate, (v3f *)l);
  TSMTRACE ((CACHE_MASK|4, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     g0, g1, g2, l[0], l[1], l[2]));

  return 1;
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Local Vertical coordinates 
 *   to the Geodetic coordinate system.
 *
 *   Geodetic coordinates (GDC) are tuples <longitude, latitude,
 *   elevation>. The Local Vertical coordinate system (LVCS) is a localized,
 *   arbitrary coordinate space; points are represented by the tuple (x,y,z),
 *   with the Z axis oriented in the UP direction.
 *
 * @param cache   reference tileset cache
 * @param g0      input coordinate in Geodetic coordinates.
 * @param g1      "
 * @param g2      "
 * @param l[3]    returned transformed coordinate in LVCS space.
 *
 * @return int
 * */
int
TsCacheGDCtoLVCS (TsCache const *cache,
		  double g0, double g1, double g2, float l[3])
{
  /*static char const funcName[] = "TsCacheGDCtoLVCS";*/
  double tmp[3];

  if (! TsGDCtoGCC (g0, g1, g2, tmp))
    return 0;

  return TsCacheGCCtoLVCS (cache, tmp[0], tmp[1], tmp[2], l);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Initialize the tileset-specific coordinate system
 *   conversion functions.
 *
 * @return  1 on successful initialization; 0  on error condition.
 *
 */
int
TsCacheCoordsInit (TsCache *cache)
{
  static char const funcName[] = "TsCacheCoordsInit";
  TileSetTspec const *tspec = TsCacheTspec (cache);
  ProjectionTspec const *proj = &tspec->projectionTspec;
  CoordTspec const *coords = &tspec->coordTspec;
  int fast = 1;
  int success = 1;

  SE_SRF_PARAMETERS uv;
  SE_SRF_PARAMETERS gdc;
  SE_SRF_PARAMETERS gcc;

  SE_HORIZONTAL_DATUM_ENUM hdatum = SE_W84_HDATUM;
  SE_VERTICAL_DATUM_ENUM vdatum = SE_WGS84E_VDATUM;
  SRM_ELEVATION_UNITS_ENUM z_units = SRM_METERS;

  /* Set up for repeated conversions from GCC to GDC and back. */
  PR_CallOnce (&TsGlobalCoordsOnce, TsGlobalCoordsInit);

  /* Set up GDC parameters */
  gdc.is_2D = FALSE;
  gdc.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
  gdc.u.params_3d.u.gd_parameters.horizontal_datum = SE_W84_HDATUM;
  gdc.u.params_3d.u.gd_parameters.vertical_datum   = SE_WGS84E_VDATUM;
  gdc.u.params_3d.u.gd_parameters.elevation_units  = SRM_METERS;

  /* Set up GCC parameters */
  gcc.is_2D = FALSE;
  gcc.u.params_3d.spatial_reference_frame = SRM_GC_3D_SRF;

  /* Now set up local coords. */
  if (coords->spheroidName == tspNad83) {
    hdatum = SE_NAR_C_HDATUM;
    fast = 0;
  }

  /* Initialize empty */
  cache->UVtoGCC_cs = 0;
  cache->GCCtoUV_cs = 0;
  cache->UVtoGDC_cs = 0;
  cache->GDCtoUV_cs = 0;

  switch (proj->projectionType) {
  case tspUtmProjection:
    uv.is_2D = FALSE;
    uv.u.params_3d.spatial_reference_frame = SRM_UTM_3D_SRF;
    uv.u.params_3d.u.utm_parameters.horizontal_datum = hdatum;
    uv.u.params_3d.u.utm_parameters.vertical_datum   = vdatum;
    uv.u.params_3d.u.utm_parameters.x_offset = 0;
    uv.u.params_3d.u.utm_parameters.y_offset = 0;
    uv.u.params_3d.u.utm_parameters.z_units  = z_units;
    uv.u.params_3d.u.utm_parameters.zone = proj->utmZoneNumber;
    cache->utm_zone = proj->utmZoneNumber;

    /* We need to do this because the SEDRIS conversions library advertises
       the ability to do UTM -> GCC conversion, but doesn't actually do
       it... So we need to do UTM <-> GDC <-> GCC */
    {
      SE_SRF_PARAMETERS gdc1;
      
      /* Set up GDC parameters */
      gdc1.is_2D = FALSE;
      gdc1.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
      gdc1.u.params_3d.u.gd_parameters.horizontal_datum = hdatum;
      gdc1.u.params_3d.u.gd_parameters.vertical_datum   = vdatum;
      gdc1.u.params_3d.u.gd_parameters.elevation_units  = SRM_METERS;

      cache->UVtoGCC = TsCacheUVtoGCC_UTM;
      cache->UVtoGCC_cs = calloc (2, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
      if (SE_CreateCoordConversionConstants (&uv, &gdc1, 
					     &cache->UVtoGCC_cs[0])
	  != SE_COORD_SUCCESS) {
	fprintf (stderr, "Failed UTM->GDC1 init\n");
	success = 0;
      } else if (SE_CreateCoordConversionConstants (&gdc1, &gcc, 
						    &cache->UVtoGCC_cs[1])
		 != SE_COORD_SUCCESS) {
	fprintf (stderr, "Failed GDC1->GCC init\n");
	success = 0;
      }

      cache->GCCtoUV = TsCacheGCCtoUV_UTM;
      cache->GCCtoUV_cs = calloc (2, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
      if (SE_CreateCoordConversionConstants (&gcc, &gdc1,
					     &cache->GCCtoUV_cs[0])
	  != SE_COORD_SUCCESS) {
	fprintf (stderr, "Failed GCC->GDC1 init\n");
	success = 0;
      } else if (SE_CreateCoordConversionConstants (&gdc1, &uv, 
						    &cache->GCCtoUV_cs[1])
		 != SE_COORD_SUCCESS) {
	fprintf (stderr, "Failed GDC1->UTM init\n");
	success = 0;
      }
    }

    cache->UVtoGDC = TsCacheUVtoGDC_UTM;
    cache->UVtoGDC_cs = calloc (1, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
    if (SE_CreateCoordConversionConstants (&uv, &gdc, &cache->UVtoGDC_cs[0])
	!= SE_COORD_SUCCESS) {
      if (success)
	cache->UVtoGDC = TsCacheUVtoGDC_twostage;
    }

    cache->GDCtoUV = TsCacheGDCtoUV_UTM;
    cache->GDCtoUV_cs = calloc (1, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
    if (SE_CreateCoordConversionConstants (&gdc, &uv, &cache->GDCtoUV_cs[0])
	!= SE_COORD_SUCCESS) {
      if (success)
	cache->GDCtoUV = TsCacheGDCtoUV_twostage;
    }

    break;

  case tspAffineProjection:
    if (coords->lcsType != tspLvcsCoordSystem) {
      fprintf (stderr, "Yow.  Non-LVCS affine dataset!");
      success = 0;
    }

    TsGDCtoGCC (coords->originLong, coords->originLat, 0.0,
		cache->lvcs_origin);
    TSMTRACE ((CACHE_MASK|1, funcName, "UV %.0f,%.0f,0 => GCC %.0f,%.0f,%.0f",
	       coords->originLong, coords->originLat,
	       cache->lvcs_origin[0],
	       cache->lvcs_origin[1],
	       cache->lvcs_origin[2]));

    cache->lvcs = TRUE;
    cache->UVtoGCC = TsCacheLVCStoGCC;
    cache->GCCtoUV = TsCacheGCCtoLVCS;
    cache->UVtoGDC = TsCacheLVCStoGDC;
    cache->GDCtoUV = TsCacheGDCtoLVCS;
    break;

  case tspLatLongProjection:
    uv.is_2D = FALSE;
    uv.u.params_3d.spatial_reference_frame = SRM_GD_3D_SRF;
    uv.u.params_3d.u.gd_parameters.horizontal_datum = hdatum;
    uv.u.params_3d.u.gd_parameters.vertical_datum   = vdatum;
    uv.u.params_3d.u.gd_parameters.elevation_units  = z_units;

    cache->UVtoGCC = TsCacheUVtoGCC_GDC;
    cache->UVtoGCC_cs = calloc (1, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
    if (SE_CreateCoordConversionConstants (&uv, &gcc, cache->UVtoGCC_cs)
	!= SE_COORD_SUCCESS)
      success = 0;

    cache->GCCtoUV = TsCacheGCCtoUV_GDC;
    cache->GCCtoUV_cs = calloc (1, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
    if (SE_CreateCoordConversionConstants (&gcc, &uv, cache->GCCtoUV_cs)
	!= SE_COORD_SUCCESS)
      success = 0;

    cache->UVtoGDC = fast ? TsCacheUVtoGDC_GDC_fast : TsCacheUVtoGDC_GDC;
    if (! fast) {
      cache->UVtoGDC_cs = calloc (2, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
      if (SE_CreateCoordConversionConstants (&uv, &gcc,
					     &cache->UVtoGDC_cs[0])
	  != SE_COORD_SUCCESS)
	success = 0;
      else if (SE_CreateCoordConversionConstants (&gcc, &gdc,
						  &cache->UVtoGDC_cs[1])
	       != SE_COORD_SUCCESS)
	success = 0;
    }

    cache->GDCtoUV = fast ? TsCacheGDCtoUV_GDC_fast : TsCacheGDCtoUV_GDC;
    if (! fast) {
      cache->GDCtoUV_cs = calloc (2, sizeof (SE_CONVERT_COORD_SYSTEM_PAIR));
      if (SE_CreateCoordConversionConstants (&gdc, &gcc,
					     &cache->GDCtoUV_cs[0])
	  != SE_COORD_SUCCESS)
	success = 0;
      else if (SE_CreateCoordConversionConstants (&gcc, &uv,
						  &cache->GDCtoUV_cs[1])
	  != SE_COORD_SUCCESS)
	success = 0;
    }

    TSMTRACE ((CACHE_MASK|1, funcName, "Data in Geodetic coords"));
    break;

  default:
    tsmError (funcName, "Unrecognized projection %d",
	      tspec->projectionTspec.projectionType);
    success = 0;
    break;
  }

  /* Set up LVCS rotation */
  if (success) {
    float orient[4];

    if (proj->projectionType != tspAffineProjection) {
      TsCacheUVtoGCC (cache, 0, 0, 0, cache->lvcs_origin);
      TSMTRACE ((CACHE_MASK|1, funcName, "UV 0,0,0 => GCC %.0f,%.0f,%.0f",
		 cache->lvcs_origin[0],
		 cache->lvcs_origin[1],
		 cache->lvcs_origin[2]));
    }

    if ( ! TsLVCSOrientation (cache->lvcs_origin, orient) )
      return 0;

    TsCacheSetOrientation (cache, orient[0], orient[1], orient[2], orient[3]);

    TSMTRACE ((CACHE_MASK|1, funcName, "orientation %.2f: %.2f,%.2f,%.2f",
	       cache->orientation[0], cache->orientation[1],
	       cache->orientation[2], cache->orientation[3]));

    if (TSMTRACEP(CACHE_MASK|1)) {
      /* Check transforms */
      double gcc[3];
      float lvcs[3];
      TsCacheLVCStoGCC (cache, 100, 100, 100, gcc);
      TsCacheGCCtoLVCS (cache, gcc[0], gcc[1], gcc[2], lvcs);
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "LVCS 100,100,100 => GCC %.0f,%.0f,%.0f",
		 gcc[0], gcc[1], gcc[2]));
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "GCC %.0f,%.0f,%.0f => GCC %.1f,%.1f,%.1f",
		 gcc[0], gcc[1], gcc[2], lvcs[0], lvcs[1], lvcs[2]));
    }
  }

  return success;
}

void
TsCacheCoordsDestroy (TsCache *cache)
{
  /*static char const funcName[] = "TsCacheCoordsDestroy";*/
  TileSetTspec const *tspec = TsCacheTspec (cache);
  ProjectionTspec const *proj = &tspec->projectionTspec;

  switch (proj->projectionType) {
  case tspUtmProjection:
    if (cache->UVtoGCC_cs)
      SE_FreeCoordConversionConstants (cache->UVtoGCC_cs[1]);
    if (cache->GCCtoUV_cs)
      SE_FreeCoordConversionConstants (cache->GCCtoUV_cs[1]);
  case tspLatLongProjection:
    if (cache->UVtoGCC_cs) {
      SE_FreeCoordConversionConstants (cache->UVtoGCC_cs[0]);
      free (cache->UVtoGCC_cs);
    }
    if (cache->GCCtoUV_cs) {
      SE_FreeCoordConversionConstants (cache->GCCtoUV_cs[0]);
      free (cache->GCCtoUV_cs);
    }
    if (cache->UVtoGDC_cs) {
      SE_FreeCoordConversionConstants (cache->UVtoGDC_cs[0]);
      free (cache->UVtoGDC_cs);
    }
    if (cache->GDCtoUV_cs) {
      SE_FreeCoordConversionConstants (cache->GDCtoUV_cs[0]);
      free (cache->GDCtoUV_cs);
    }
    
    break;
  default:
    break;
  }
}

static void
TsCacheSetOrientation (TsCache *cache,
		       float angle, float x, float y, float z)
{
  cache->orientation[0] = angle;
  cache->orientation[1] = x;
  cache->orientation[2] = y;
  cache->orientation[3] = z;
  glRotation_m3f (angle, x, y, z, &cache->lvcs_rotate);
}
