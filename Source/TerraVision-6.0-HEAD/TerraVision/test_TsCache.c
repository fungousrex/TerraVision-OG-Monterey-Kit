/* -*- tab-width: 2; -*-

  Revision History:
      $Id: test_TsCache.c,v 2.11.2.2 2002/10/31 05:26:44 reddy Exp $

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

#include "config.h"

#include <pthr/trace.h>

#include "TsCache.h"
#include "TsGeoPyramid.h"

static int pending = 0;

void
request_pyramid (TsCache *cache, int x, int y, int res, int minRes)
{
  TileSetTspec const *tspec = TsCacheTspec (cache);
  if (TsCacheTileValid (cache, x, y, res)) {
    TsCacheRequest (cache, x, y, res, TSM_NOW);
    ++pending;
    if (--res >= minRes) {
      x *= 2; y *= 2;
      request_pyramid (cache, x+0, y+0, res, minRes);
      request_pyramid (cache, x+1, y+0, res, minRes);
      request_pyramid (cache, x+0, y+1, res, minRes);
      request_pyramid (cache, x+1, y+1, res, minRes);
    }
  }
}

int
test_url (char const *url, int size, int levels)
{
  TsCache *cache =  TsCacheCreate (url, NULL);
  TileSetTspec const *tspec;
  int res;

  TsCacheConnect (cache);
  tspec = TsCacheTspec (cache);
  updated = TsCacheCond (cache, ts);

  request_pyramid (cache, 0, 0, tspec->maxLevel, tspec->maxLevel-levels);
  TsCacheSendRequests (cache);

  while (pending > 0) {
    printf ("%d pending requests.\n", pending);
    --pending;
  }

  TsCacheDisconnect (cache);

  TsCacheDestroy (cache);

  return TRUE;
}

int
geo_request_pyramid (TsGeoView *geoView, TsTile *tile, int n)
{
  TsGeoPyramid *geo = geoView->geo;
  TsTile *children[4];
  int priority = TSM_NOW;
  int unprepared = 1;

  if (TsGeoViewTilePrepared (geoView, tile, &priority)) {
    printf ("[%d %d %d] bbox %g,%g,%g -> %g,%g,%g\n",
	    tile->id.x, tile->id.y, tile->id.res,
	    tile->bbox.min[0], tile->bbox.min[1], tile->bbox.min[2],
	    tile->bbox.max[0], tile->bbox.max[1], tile->bbox.max[2]);
    TsGeometry (geo, tile);
    unprepared = 0;
  } else {
    printf ("[%d %d %d] not prepared\n", tile->id.x, tile->id.y, tile->id.res);
  }

  if (n--) {
    int nchildren = TsGeoChildren (geo, tile, children);
    int i;
    for (i = 0; i < nchildren; ++i)
      unprepared += geo_request_pyramid (geo, children[i], n);
  }

  return unprepared;
}

int
test_geopyramid (char const *oi_url, char const *dem_url, int depth)
{
  TsGeoPyramid *geo = TsGeoCreate (TsGeo_OI_DEM,
				   oi_url, NULL,
				   dem_url, NULL);
  TsGeoView *geoView = TsGeoCreateView (geo);

  int unprepared = 0;

  if (! geo) return FALSE;

  do {
    if (unprepared) sleep (2);
    unprepared = geo_request_pyramid (geoView, TsGeoTop (geo), depth);
    TsGeoViewSync (geoView);
  } while (unprepared);

  TsGeoViewDestroy (geoView);
  TsGeoDestroy (geo);

  return TRUE;
}

int
main ()
{
  static char const *url1 =
    "/home/scratch5/magic/TileSets/Pyramids/ntc-1-utm-2scene.128.oi";
  static char const *url2 =
    "/home/scratch5/magic/TileSets/Pyramids/ntc-1-utm-2scene.33.dem";
  static char const *url3 =
    "http://crazypete.ai.sri.com/TileSets/Pyramids/co.oi";
  static char const *url4 =
    "http://crazypete.ai.sri.com/TileSets/Pyramids/co.dem";

#ifdef DPSS
  static char const *url5 =
    "x-dpss://iss-1-atm.sri.magic.net/TileSets/Pyramids/68";
  static char const *url6 =
    "x-dpss://iss-1-atm.sri.magic.net/TileSets/Pyramids/67";
#endif

  pthr_trace_print_pid (TRUE);
  pthr_trace_mask (CACHE_MASK);
  pthr_trace_level (1);

#if 0 && defined(_pthr_sproc_h_)
  __pthread_debug (2);
#endif /* _pthr_sproc_h_ */

#if 0
  if (! test_url (url1, 256, 9))
    return 1;

  if (! test_url (url1, 128, 9))
    return 1;

  if (! test_url (url1, 512, 9))
    return 1;

  if (! test_url (url2, 512, 8))
    return 1;

#ifdef DPSS
  if (! test_url (url5, 512, 8))
    return 1;

  if (! test_url (url6, 512, 8))
    return 1;
#endif
#endif

  if (! test_geopyramid (url1, url2, 8))
    return 1;

  if (! test_geopyramid (url3, url4, 8))
    return 1;

#if DPSS
  if (! test_geopyramid (url5, url6, 8))
    return 1;
#endif

  return 0;
}
