/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsCoordsInline.h,v 2.5.4.2 2002/10/31 05:26:39 reddy Exp $

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

#include <se_conv.h>
#include <vrml2/vec3.h>

#include "TsCacheI.h"

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in the tileset's native (u,v)
 *   coordinate system to Geodetic coordinates. 
 *   .
 *   Geodetic coordinates (GDC) are tuples <longitude, latitude, elevation>.
 *
 * @param cache  reference tileset cache
 * @param u      input coordinate in LVCS space.
 * @param v      "
 * @param z      "
 * @param gdc[3] returned transformed coordinate in Geodetic space.
 *
 * @return success
 *
 */
exinline int
TsCacheUVtoGDC (TsCache const *cache, float u, float v, float z, double gdc[3])
{
  return (*cache->UVtoGDC) (cache, u, v, z, gdc);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Geodetic coordinates 
 *   to the tileset's native (u,v) coordinate system.
 *
 *   Geodetic coordinates (GDC) are tuples <longitude, latitude, elevation>.
 *
 * @param cache  reference tileset cache
 * @param lng    input coordinate in Geodetic space.
 * @param lat    "
 * @param z      "
 * @param uv[3]  returned transformed coordinate in UV space.
 *
 * @return int
 *
 */
exinline int
TsCacheGDCtoUV (TsCache const *cache, double lng, double lat, double z, float uv[3])
{
  return (*cache->GDCtoUV) (cache, lng, lat, z, uv);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in the tileset's native (u,v)
 *   coordinate system to Geocentric coordinates. 
 *   .
 *   Geocentric coordinates (GCC) are <x, y, z> tuples that represent
 *   distances from the planet's center.
 *
 * @param cache  reference tileset cache
 * @param u      input coordinate in UV space.
 * @param v      "
 * @param z      "
 * @param gcc[3] returned transformed coordinate in Geocentric space.
 *
 * @return success
 *
 */
exinline int
TsCacheUVtoGCC (TsCache const *cache, float u, float v, float z, double gcc[3])
{
  return (*cache->UVtoGCC) (cache, u, v, z, gcc);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Geocentric coordinates 
 *   to the tileset's native (u,v) coordinate system.
 *
 *   Geocentric coordinates (GCC) are <x, y, z> tuples that represent
 *   distances from the planet's center.
 *
 * @param cache  reference tileset cache
 * @param x      input coordinate in Geocentric space.
 * @param y      "
 * @param z      "
 * @param uv[3]  returned transformed coordinate in UV space.
 *
 * @return int
 *
 */
exinline int
TsCacheGCCtoUV (TsCache const *cache, double x, double y, double z, float uv[3])
{
  return (*cache->GCCtoUV) (cache, x, y, z, uv);
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in the tileset's native (u,v)
 *   coordinate system to Local Vertical coordinates
 *
 *   The Local Vertical coordinate system (LVCS) is a localized, arbitrary 
 *   coordinate space;  points are represented by the tuple (x,y,z), with 
 *   the Z axis oriented in the UP direction.
 *
 * @param cache   reference tileset cache
 * @param u       input coordinate in UV space.
 * @param v       "
 * @param z       "
 * @param lvcs[3] returned transformed coordinate in UV space.
 *
 * @return int
 *
 */
exinline int
TsCacheUVtoLVCS (TsCache const *cache, 
		 float u, float v, float z,
		 float lvcs[3])
{
  static char const funcName[] = "TsCacheUVtoLVCS";
  if (cache->lvcs) {
    lvcs[0] = u;
    lvcs[1] = v;
    lvcs[2] = z;
  } else {
    double gcc[3];
    if (! TsCacheUVtoGCC (cache, u, v, z, gcc)) {
      TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> GCC: FAIL",
		 u, v, z));
      return FALSE;
    }
    TSMTRACE ((CACHE_MASK|4, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	       u, v, z, gcc[0], gcc[1], gcc[2]));
    if (! TsCacheGCCtoLVCS (cache, gcc[0], gcc[1], gcc[2], lvcs)) {
      TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> LVCS: FAIL",
		 gcc[0], gcc[1], gcc[2]));
      return FALSE;
    }
  }
  return TRUE;
}

/*
 * Doc++ Description: *//**
 *   Coordinate conversion:  Transform a point in Local Vertical coordinates 
 *   to the tileset's native (u,v) coordinate system.
 *
 *   The Local Vertical coordinate system (LVCS) is a localized, arbitrary 
 *   coordinate space;  points are represented by the tuple (x,y,z), with 
 *   the Z axis oriented in the UP direction.
 *
 * @param cache  reference tileset cache
 * @param x      input coordinate in LVCS space.
 * @param y      "
 * @param z      "
 * @param uv[3]  returned transformed coordinate in UV space.
 *
 * @return int
 *
 */
exinline int
TsCacheLVCStoUV (TsCache const *cache, 
		 float x, float y, float z,
		 float uv[3])
{
  static char const funcName[] = "TsCacheLVCStoUV";
  if (cache->lvcs) {
    uv[0] = x;
    uv[1] = y;
    uv[2] = z;
  } else {
    double gcc[3];
    if (! TsCacheLVCStoGCC (cache, x, y, z, gcc)) {
      TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> GCC: FAIL",
		 x, y, z));
      return FALSE;
    }
    TSMTRACE ((CACHE_MASK|4, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	       x, y, z, gcc[0], gcc[1], gcc[2]));
    if (! TsCacheGCCtoUV (cache, gcc[0], gcc[1], gcc[2], uv)) {
      TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> UV: FAIL",
		 gcc[0], gcc[1], gcc[2]));
      return FALSE;
    }
  }
  return TRUE;
}

