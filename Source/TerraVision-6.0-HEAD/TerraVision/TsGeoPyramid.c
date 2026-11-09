/* -*- tab-width: 8; -*-

  Revision History:
      $Id: TsGeoPyramid.c,v 2.113.2.16 2002/10/31 05:26:39 reddy Exp $

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

#include <limits.h>
#include <math.h>
#include <float.h>
#include <vrml2/quat.h>
#include <vrml2/vec3.h>

#include <pratom.h>

#include "TsGeoPyramid.h"
#include "TsGeoPyramidI.h"

#undef POINTCACHE
#undef INSTRUMENT_CACHE

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


/* Global list of TsGeoPyramids */
list_t *geoList;

typedef struct TsGeoPyramid_OIDEM {
  TsGeoPyramid geo;
  TsCache *dem;

  int oi_dem_offset;
  int num_polys;

  void (*convertOItoDEM) (struct TsGeoPyramid_OIDEM const *,
			  float u, float v, 
			  float *dem_u, float *dem_v);
  void (*convertDEMtoOI) (struct TsGeoPyramid_OIDEM const *,
			  float dem_u, float dem_v, 
			  float *u, float *v);
  float const *convertLinear;

  m4f lvcsOItoDEM;
  m4f lvcsDEMtoOI;

  TsGeoView *baseView;
} TsGeoPyramid_OIDEM;

#include <math.h>
#include <assert.h>

static void TsGeoOIDEM_Destroy (TsGeoPyramid *geo);
static int TsGeoOIDEM_Connections (TsGeoPyramid *geo, TsmConnection *cons);
static TsGeoView * TsGeoOIDEM_CreateView (TsGeoPyramid *geo);
static float TsGeoOIDEM_Elevation (TsGeoPyramid const *geo,
				   float u, float v);
static int TsGeoOIDEM_AddCallback (TsGeoPyramid *geo, TsCacheCB which,
				   TsCallbackF func, void *closure,
				   void (*destroyf)(void*));
static int TsGeoOIDEM_RemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
				      TsCallbackF func, void *closure);
static int TsGeoOIDEM_URLs (TsGeoPyramid const *geo, char const **urls);

static void TsCacheDEM_InitLoad (TsCache *cache, tsmRequestList *req);
static void TsCacheOI_InitLoad (TsCache *cache, tsmRequestList *req);

void
TsGeoCreate_Common (TsGeoPyramid *geo)
{
  /* Add dataset to global list. */
  if (geo) {
    if (! geoList) geoList = list_create (TRUE);
    list_push (geoList, geo);
  }
}

void
TsGeoDestroy_Common (TsGeoPyramid *geo)
{
  /* Remove dataset from global list. */
  if (geo) {
    list_remove (geoList, geo);
  }
}

TsTile *
TsGeoTop (TsGeoPyramid const *geo)
{
  /*static char const funcName[] = "TsGeoTop";*/
  TileSetTspec const *tspec = TsCacheTspec (geo->im);
  return TsCacheGetTile (geo->im, 0, 0, tspec->maxLevel);
}

#define INIT_ALT (100.0)

void
TsGeoViewpoint (TsGeoPyramid const *geo, float aspect, double eye[3])
{
  /* otherwise, just put us overhead the dataset */
 
  float        min_x, max_x, min_y, max_y, u, v, elev, extent;
  TsBBox const *bbox_uv = TsGeoBBoxUV( geo );
  TileSetTspec const *tspec = TsCacheTspec( geo->im );
  int          is_gdc;

  /* convenience vars for our UV bounding box */

  min_x = bbox_uv->min[0];
  max_x = bbox_uv->max[0];
  min_y = bbox_uv->min[1];
  max_y = bbox_uv->max[1] * aspect;

  /* is this a lat/long dataset? need to know this */

  is_gdc = ( tspec->projectionTspec.projectionType == tspLatLongProjection );

  /* work out the extent of the dataset we wish to view */

  extent = max( max_x - min_x, max_y - min_y );
  if ( is_gdc )
    extent = min( extent, 180.0 );

  /* start us out in the middle of the dataset... */
      
  u = min_x + ( max_x - min_x ) / 2; 
  v = min_y + ( max_y - min_y ) / 2; 

  /* if the units of min/max are degrees, we need to convert to meters */

  if ( is_gdc )
    extent *= 100000;

  /* ...and sufficiently high up to see the whole dataset */

  if ( ( elev = TsGeoElevationUV( geo, u, v ) ) == TS_BAD_ELEVATION )
    elev = 0;
  elev += extent * 2.0;

  TsGeoUVtoGCC( geo, u, v, elev+INIT_ALT, eye );
}

static inline int
TsGeoIXIYtoLVCS (TsGeoPyramid const *geo, 
		 float const *ixiytouv,
		 float ix, float iy, float z,
		 float l[3])
{
  double gcc[3];
  float u = TFM_U (ix,iy,ixiytouv);
  float v = TFM_V (ix,iy,ixiytouv);
  if (! TsGeoUVtoGCC (geo, u, v, z, gcc))
    return 0;
  
  TsGeoGCCtoLVCS (geo, gcc[0], gcc[1], gcc[2], l);
  return 1;
}

static inline void
TsGeoOIDEM_OItoDEM (TsGeoPyramid_OIDEM const *geov,
		    float u, float v,
		    float *dem_u, float *dem_v)
{
  (*geov->convertOItoDEM) (geov, u, v, dem_u, dem_v);
}

static inline void
TsGeoOIDEM_DEMtoOI (TsGeoPyramid_OIDEM const *geov,
		    float dem_u, float dem_v,
		    float *u, float *v)
{
  (*geov->convertDEMtoOI) (geov, dem_u, dem_v, u, v);
}

static void
TsGeoOIDEM_LinearOItoDEM (TsGeoPyramid_OIDEM const *geov,
			  float u, float v,
			  float *dem_u, float *dem_v)
{
  static char const funcName[] = "TsGeoOIDEM_LinearOItoDEM";
  
  float const *tfm = geov->convertLinear;
  *dem_u = tfm[0]*u + tfm[1]*v + tfm[2];
  *dem_v = tfm[3]*u + tfm[4]*v + tfm[5];
  TSMTRACE ((CACHE_MASK|3, funcName, "OI: %.1f,%.1f -> DEM: %.1f,%.1f",
	     u, v, *dem_u, *dem_v));
}

static void
TsGeoOIDEM_LinearDEMtoOI (TsGeoPyramid_OIDEM const *geov,
			  float dem_u, float dem_v,
			  float *u, float *v)
{
  static char const funcName[] = "TsGeoOIDEM_LinearDEMtoOI";
  
  float const *tfm = geov->convertLinear;
  *u = tfm[6]*dem_u + tfm[ 7]*dem_v + tfm[ 8];
  *v = tfm[9]*dem_u + tfm[10]*dem_v + tfm[11];
  TSMTRACE ((CACHE_MASK|3, funcName, "DEM: %.1f,%.1f -> OI: %.1f,%.1f",
	     dem_u, dem_v, *u, *v));
}

static void
TsGeoOIDEM_ComplexOItoDEM (TsGeoPyramid_OIDEM const *geov,
			   float u, float v,
			   float *dem_u, float *dem_v)
{
  static char const funcName[] = "TsGeoOIDEM_ComplexOItoDEM";
  TsGeoPyramid const *geo = (TsGeoPyramid const *) geov;
  double tmp[3];
  float dem[3];

  /* if can't do the conversion, then try to return something! */
  *dem_u = *dem_v = 0.0f;

  /* First we go from OI UV to the global coordinate system. */
  if ( TsCacheUVtoGDC (geo->im, u, v, 0, tmp)) {
    if ( TsCacheGDCtoUV (geov->dem, tmp[0], tmp[1], 0, dem)) {
      *dem_u = dem[0];
      *dem_v = dem[1];
    }
  }

  TSMTRACE ((CACHE_MASK|3, funcName, "OI: %.1f,%.1f -> DEM: %.1f,%.1f",
	     u, v, *dem_u, *dem_v));
}

static void
TsGeoOIDEM_ComplexDEMtoOI (TsGeoPyramid_OIDEM const *geov,
			   float dem_u, float dem_v,
			   float *u, float *v)
{
  static char const funcName[] = "TsGeoOIDEM_ComplexDEMtoOI";
  TsGeoPyramid const *geo = (TsGeoPyramid const *) geov;
  double tmp[3];
  float oi[3];

  /* First we go from OI UV to the global coordinate system. */
  if (!TsCacheUVtoGDC (geov->dem, dem_u, dem_v, 0, tmp))
    abort ();
  if (! TsCacheGDCtoUV (geo->im, tmp[0], tmp[1], 0, oi))
    abort();
  *u = oi[0];
  *v = oi[1];
  TSMTRACE ((CACHE_MASK|3, funcName, "DEM: %.1f,%.1f -> OI: %.1f,%.1f",
	     dem_u, dem_v, *u, *v));
}

static void
TsGeoSetupOIDEMConversion (TsGeoPyramid_OIDEM *geov)
{
  static char const funcName[] = "TsGeoSetupOIDEMConversion";

  TsGeoPyramid *geo = &geov->geo;
  TileSetTspec const *oi_tspec = TsCacheTspec (geo->im);
  TileSetTspec const *dem_tspec = TsCacheTspec (geov->dem);
  ProjectionTspec const *oi_proj = &oi_tspec->projectionTspec;
  ProjectionTspec const *dem_proj = &dem_tspec->projectionTspec;
  CoordTspec const *oi_coords = &oi_tspec->coordTspec;
  CoordTspec const *dem_coords = &dem_tspec->coordTspec;

  if (oi_proj->projectionType == dem_proj->projectionType
      && (oi_proj->projectionType == tspUtmProjection 
	  ? oi_proj->utmZoneNumber == dem_proj->utmZoneNumber
	  : 1)
      && oi_coords->spheroidName == dem_coords->spheroidName) {
    /* We can use a fast, linear conversion method. */
    double const *oiOrigin = TsCacheOrigin (geo->im);
    double const *demOrigin = TsCacheOrigin (geov->dem);
    float *tfm;

    geov->convertLinear = tfm = malloc (12 * sizeof (float));

    tfm[0] = 1; tfm[1] = 0; tfm[2] = oiOrigin[0] - demOrigin[0];
    tfm[3] = 0; tfm[4] = 1; tfm[5] = oiOrigin[1] - demOrigin[1];

    tfm[6] = 1; tfm[ 7] = 0; tfm[ 8] = -tfm[2];
    tfm[9] = 0; tfm[10] = 1; tfm[11] = -tfm[5];
    TSMTRACE((CACHE_MASK|1, funcName, "Linear: OI -> DEM %.2f,%.2f",
	      tfm[2], tfm[5]));
    geov->convertOItoDEM = TsGeoOIDEM_LinearOItoDEM;
    geov->convertDEMtoOI = TsGeoOIDEM_LinearDEMtoOI;
  } else {
    /* We have to go through the a slow system. */
    geov->convertOItoDEM = TsGeoOIDEM_ComplexOItoDEM;
    geov->convertDEMtoOI = TsGeoOIDEM_ComplexDEMtoOI;
  }

  /* Setup conversion from OI LVCS to DEM LVCS */
  {
    m3f const *R_oi = &geo->im->lvcs_rotate;
    double const *T_oi = geo->im->lvcs_origin;
    m3f const *R_dem = &geov->dem->lvcs_rotate;
    double const *T_dem = geov->dem->lvcs_origin;
    m4f *Md = &geov->lvcsOItoDEM;
    m4f *Mi = &geov->lvcsDEMtoOI;
    m4f Mt;
    int i;

    /* Calculate matrix for LVCS: OI -> DEM */
    identity_m4f (Md);
    mult_m3f_m4f (R_oi, Md, &Mt);
    for (i = 0; i < 3; ++i)
      M(Mt)[3][i] += T_oi[i] - T_dem[i];
    mult_m3fT_m4f (R_dem, &Mt, Md);
    TSMTRACE ((CACHE_MASK|1, funcName, "OI->DEM [%.2f, %.2f, %.2f, %.2f",
	       M(*Md)[0][0], M(*Md)[0][1], M(*Md)[0][2], M(*Md)[0][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f",
	       M(*Md)[1][0], M(*Md)[1][1], M(*Md)[1][2], M(*Md)[1][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f",
	       M(*Md)[2][0], M(*Md)[2][1], M(*Md)[2][2], M(*Md)[2][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f]",
	       M(*Md)[3][0], M(*Md)[3][1], M(*Md)[3][2], M(*Md)[3][3]));

    if (TSMTRACEP (CACHE_MASK|1)) {
      double gcc[3];
      float lo[4], ld[4];
      lo[0] = lo[1] = lo[2] = 100; 
      mult_m4f_v3f (Md, (v3f*)lo, (v4f*)ld);
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "OI_l %.1f,%.1f,%.1f => DEM_l %.1f,%.1f,%.1f",
		 lo[0], lo[1], lo[2], ld[0], ld[1], ld[2]));
      TsCacheLVCStoGCC (geo->im, lo[0], lo[1], lo[2], gcc);
      TsCacheGCCtoLVCS (geov->dem, gcc[0], gcc[1], gcc[2], ld);
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "== GCC %.0f,%.0f,%.0f => DEM_l %.1f,%.1f,%.1f",
		 gcc[0], gcc[1], gcc[2], ld[0], ld[1], ld[2]));
    }

    /* Calculate matrix for LVCS: DEM -> OI */
    identity_m4f (Mi);
    mult_m3f_m4f (R_dem, Mi, &Mt);
    for (i = 0; i < 3; ++i)
      M(Mt)[3][i] += T_dem[i] - T_oi[i];
    mult_m3fT_m4f (R_oi, &Mt, Mi);
    TSMTRACE ((CACHE_MASK|1, funcName, "DEM->OI [%.2f, %.2f, %.2f, %.2f",
	       M(*Mi)[0][0], M(*Mi)[0][1], M(*Mi)[0][2], M(*Mi)[0][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f",
	       M(*Mi)[1][0], M(*Mi)[1][1], M(*Mi)[1][2], M(*Mi)[1][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f",
	       M(*Mi)[2][0], M(*Mi)[2][1], M(*Mi)[2][2], M(*Mi)[2][3]));
    TSMTRACE ((CACHE_MASK|1, funcName, "         %.2f, %.2f, %.2f, %.2f]",
	       M(*Mi)[3][0], M(*Mi)[3][1], M(*Mi)[3][2], M(*Mi)[3][3]));

    if (TSMTRACEP (CACHE_MASK|1)) {
      double gcc[3];
      float lo[4], ld[4];
      ld[0] = ld[1] = ld[2] = 100; 
      mult_m4f_v3f (Mi, (v3f*)ld, (v4f*)lo);
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "DEM_l %.1f,%.1f,%.1f => OI_l %.1f,%.1f,%.1f",
		 ld[0], ld[1], ld[2], lo[0], lo[1], lo[2]));
      TsCacheLVCStoGCC (geov->dem, ld[0], ld[1], ld[2], gcc);
      TsCacheGCCtoLVCS (geo->im, gcc[0], gcc[1], gcc[2], lo);
      TSMTRACE ((CACHE_MASK|1, funcName,
		 "== GCC %.0f,%.0f,%.0f => OI_l %.1f,%.1f,%.1f",
		 gcc[0], gcc[1], gcc[2], lo[0], lo[1], lo[2]));
      
    }
  }
}

void
TsGeoCorrectName (TsGeoPyramid *geo)
{
  /* Work out the correct name for the GeoSet from the tspec file */
  if ( geo->im && geo->im->tspec && geo->im->tspec->geoName )
    geo->name = tsmMemStrdup( geo->im->tspec->geoName );
}

typedef struct TsTileRange {
  int min_x, max_x;
  int min_y, max_y;
  int res, ntiles;
  TsTile **tiles;
} TsTileRange;

typedef struct TsOITileAux {
  TsGeoTileAux geo;
  PRUint32 size;
  TsBBox uv_bbox;
  TsTileRange demRef;
  v3f *demCoords;
  struct TsOITileIteration *iter;
} TsOITileAux;

typedef struct TsDEMTileAux {
  PRUint32 size;
  int min_x, max_x, min_y, max_y;
  float *elevArray;
  float **elevs;
#ifndef POINTCACHE
  v3f *lvcs_pt0;
  v3f *next_pt;
  v3f ***lvcs_pts;
  v3f *lvcs_norm0;
  v3f *next_norm;
  v3f ***lvcs_norms;
#else /* POINTCACHE */
  v3f **lvcs_pts;
  v3f **lvcs_norms;
#endif
} TsDEMTileAux;

static int TsGeoOIDEM_InitLoad (TsGeoView *gv);

static void TsGeoOIDEM_SetBBoxPyramids (TsGeoPyramid_OIDEM *geov);

static void TsGeoOIDEM_TileExtent (TsCache const *cache,
				   void *arg, void *closure);
static void TsGeoOIDEM_freeOIAux (TsCache const *cache,
				  void *arg, void *closure);

static void TsGeoOIDEM_TileElevs (TsGeoPyramid_OIDEM const *geov,
				  TsTile *tile);
static void TsGeoOIDEM_TilePoints (TsGeoPyramid_OIDEM const *geov,
				   TsTile *tile);

static void TsCacheDEM_makeAux (TsCache const *cache, TsTile *tile);
static void TsCacheDEM_freeAux (TsCache const *cache,
				void *arg, void *closure);
static void TsCacheDEM_TileMap (TsCache const *cache,
				void *arg, void *closure);

static float TsCacheDEM_ElevRef (TsCache const *cache, TsTile const **tilep,
				 float ixf, float iyf, int res);
static float TsCacheDEM_Elevation (TsCache const *cache, TsTile const **tilep,
				   float ixf, float iyf, int res);

static int TsCacheDEM_PointNormRef (TsCache const *cache, TsTile const **tilep,
				    float ixf, float iyf, int res,
				    v3f *pt, v3f *norm);
static void TsCacheDEM_PointNormal (TsCache const *cache,
				    TsTile const **tilep, int res,
				    float u, float v, float const *zp,
				    float pt[3], float norm[3]);

/*
 * Synopsis:
 *   TsGeoPyramid * TsGeoCreate (TsGeoPyramidType type,
 *                               char const *im_url,
 *                               TsmConnectionParams im_params,
 *                               char const *geom_url,
 *                               TsmConnectionParams geom_params)<decl>
 *
 * Doc++ Description: *//** 
 *   Create a TsGeoPyramid, the handle for accessing an image pyramid with
 *   geometry. You specify the type of the pyramid as well as the URLs
 *   for the DEM and OI datasets. You can optionally specify connection
 *   parameters for the DEM and OI connections.
 *
 * @param    type	The type of pyramid, a TsGeoPyramidType.
 * @param    im_url     The URL of the image pyramid.
 * @param    im_params  The connection parameters for the image pyramid.
 * @param    dem_url    The URL of the DEM pyramid.
 * @param    dem_params The connection parameters for the DEM pyramid.
 *
 * @return   A handle for accessing all further TsGeo functions.
 *
 * @author   Lee Iverson
 * @version  December 1999
 *
 * @see      TsGeoPyramidType
 *
 */

TsGeoPyramid *
TsGeoCreate_OIDEM ( char const *im_url, TsmConnectionParams im_params,
		    char const *dem_url, TsmConnectionParams dem_params)
{
  static char const funcName[] = "TsGeoCreate_OIDEM";

  static TsGeoCalls TsGeoCalls_OIDEM = {
    TsGeoOIDEM_Destroy,
    TsGeoOIDEM_Connections,
    TsGeoOIDEM_CreateView,
    TsGeoOIDEM_Elevation,
    TsGeoOIDEM_AddCallback,
    TsGeoOIDEM_RemoveCallback,
    TsGeoOIDEM_URLs
  };

  TsmConnectionParams tsmParams;
  int connected;

  TsGeoPyramid_OIDEM *geov;
  TsGeoPyramid *geo;
  TileSetTspec const *oiTspec;

  /* if no DEM supplied, then create a TsGeoPyramids_OI */
  if ( dem_url == NULL || strlen( dem_url ) == 0 )
    return TsGeoCreate_OI( im_url, im_params );

  /* XXX temporary hack */
  if ( strstr( im_url, "SDSS" ) != NULL ||
       strstr( im_url, "floating_us" ) != NULL )
    return TsGeoCreate_OI( im_url, im_params );

  geov = (TsGeoPyramid_OIDEM *) malloc (sizeof (TsGeoPyramid_OIDEM));
  geo = &geov->geo;

  TSMTRACE ((1, funcName, "%s", im_url));
  TSMTRINC ((CACHE_MASK|1, 1));

  geo->name = NULL;
  geo->type = TsGeo_OI_DEM;
  geo->logging = FALSE;

  /* Create the individual caches. */
  geo->im = TsCacheCreate (im_url, im_params);
  geo->calls = &TsGeoCalls_OIDEM;
  geov->dem = TsCacheCreate (dem_url, dem_params);

  /* Clear it to start with */
  geov->convertLinear = 0;
  geov->baseView = NULL;

  /* Now connect them. */
  tsmParams = TsCacheConnectionParams (geo->im);
  tsmSetParam (tsmParams, TSM_PARAM_BLOCKING_READ, TRUE);
  tsmSetParam (tsmParams, TSM_PARAM_REQUEST_CHECK, TRUE);
  /* tsmSetParam (tsmParams, TSM_PARAM_READ_MULTI, 1); */
  tsmParams = TsCacheConnectionParams (geov->dem);
  tsmSetParam (tsmParams, TSM_PARAM_BLOCKING_READ, TRUE);
  tsmSetParam (tsmParams, TSM_PARAM_REQUEST_CHECK, TRUE);
  /* tsmSetParam (tsmParams, TSM_PARAM_READ_MULTI, 1); */

  connected = (TsCacheConnect (geo->im)
	       && TsCacheConnect (geov->dem));

  /* Check for failure or interrupt */
  if (! connected || PR_Sleep (PR_INTERVAL_NO_WAIT) == PR_FAILURE) {
    TsGeoOIDEM_Destroy (geo);
    TSMTRINC ((CACHE_MASK|1, -1));
    TSMTRACE ((CACHE_MASK|1, funcName, "FAIL"));
    return NULL;
  }

  oiTspec = TsCacheTspec (geo->im);

  /* Update the geoset name from the OI's tspec geoname entry */
  TsGeoCorrectName (geo);

  /* Set up ease-of-use references to the image cache */
  geo->origin = geo->im->lvcs_origin;
  geo->rotate = &geo->im->lvcs_rotate;
  geo->orientation = geo->im->orientation;

  /* Set up for converting between OI and DEM systems. */
  TsGeoSetupOIDEMConversion (geov);

  /* The bbox of the image is in UV.  We need to convert this to the LVCS. */
  {
    int i, j, k;
    TsBBox const *uv_bbox = TsCacheBBox (geo->im);
    float const *uv_off[2];

    uv_off[0] = uv_bbox->min;
    uv_off[1] = uv_bbox->max;
    TsBBoxInit (&geo->lvcs_bbox);
    TsBBoxInit (&geo->gdc_bbox);
    for (i = 0; i < 2; ++i) {
      double u = uv_off[i][0];
      for (j = 0; j < 2; ++j) {
	double v = uv_off[j][1];
	for (k = 0; k < 2; ++k) {
	  double z = uv_off[k][2];
	  float l[3];
	  double gdc[3];
	  /* calculate the LVCS representation of the UV coordinate */
	  if (TsCacheUVtoLVCS (geo->im, u, v, z, l)) {
	    TsBBoxAddPoint (&geo->lvcs_bbox, l[0], l[1], l[2]);
	  } else {
	    TSMTRACE((1, funcName, "Corner %d,%d,%d: %g,%g,%g => LVCS FAILED",
		      i, j, k, u, v, z));
	  }
	  /* calculate the GDC representation of the UV coordinate */
	  if (TsCacheUVtoGDC (geo->im, u, v, z, gdc)) {
	    TsBBoxAddPoint (&geo->gdc_bbox, gdc[0], gdc[1], gdc[2]);
	  } else {
	    TSMTRACE((1, funcName, "Corner %d,%d,%d: %g,%g,%g => GDC FAILED",
		      i, j, k, u, v, z));
	  }
	}
      }
    }
    TsBBoxFinish (&geo->lvcs_bbox);
    TsBBoxFinish (&geo->gdc_bbox);
    geo->lvcs_bbox.coordType = TsCoordLVCS;
    geo->gdc_bbox.coordType = TsCoordGDC;
  }

  /* Initialize TsGeoInfo */
  geo->info.minLevel = oiTspec->minLevel;
  geo->info.maxLevel = oiTspec->maxLevel;
  geo->info.tileNumPixels[0] = oiTspec->tileTspec.numPixelsX;
  geo->info.tileNumPixels[1] = oiTspec->tileTspec.numPixelsY;
  geo->info.tileNumComps     = oiTspec->tileTspec.numComponentsPixel;
  geo->info.earthRadius[0]   = oiTspec->coordTspec.equatorialRadius;
  geo->info.earthRadius[1]   = oiTspec->coordTspec.polarRadius;

  geo->info.wholeEarth       = oiTspec->wrap_lon;
  geo->info.transparent      = FALSE;
  geo->info.oneSided         = TRUE;
  geo->info.viewAbove        = TRUE;
  geo->info.lightable        = TRUE;
  geo->info.skySurface       = FALSE;

  /* @@ Better think about this... @@ */
  {
    int numX = oiTspec->tileTspec.numPixelsX;
    int numY = oiTspec->tileTspec.numPixelsY;
    /* This gives us ~16 pixels per triangle. */
    /*geov->num_polys = MIN (numX, numY)/16;*/
    geov->num_polys = 4;
  }

  /* Choose an offset so that we get approx 1-1 DEM/OI */
  {
    int imLevel = oiTspec->minLevel;
    SimpleTileSet const * imTset = TsCacheTileSet (geo->im, imLevel);
    float const *im_ixiytouv = TsCacheTfm_ixiytouv (geo->im, imLevel);

    int demLevel = TsCacheTspec (geov->dem)->minLevel;
    SimpleTileSet const *demTset = TsCacheTileSet (geov->dem, demLevel);
    float const *dem_uvtoixiy = TsCacheTfm_uvtoixiy (geov->dem, demLevel);

    float im_w = imTset->tileTspec.numPixelsX;
    float im_post = im_w/geov->num_polys;
    float dem_u, dem_v, log_rat;
    float ix0, iy0, ix1, iy1, pxl_len;

    TsGeoOIDEM_OItoDEM (geov, 
			TFM_U (0, 0, im_ixiytouv),
			TFM_V (0, 0, im_ixiytouv),
			&dem_u, &dem_v);
    ix0 = TFM_IX (dem_u, dem_v, dem_uvtoixiy);
    iy0 = TFM_IY (dem_u, dem_v, dem_uvtoixiy);

    TsGeoOIDEM_OItoDEM (geov, 
			TFM_U (im_post, 0, im_ixiytouv),
			TFM_V (im_post, 0, im_ixiytouv),
			&dem_u, &dem_v);
    ix1 = TFM_IX (dem_u, dem_v, dem_uvtoixiy);
    iy1 = TFM_IY (dem_u, dem_v, dem_uvtoixiy);

    pxl_len = hypot (ix1-ix0, iy1-iy0);
    log_rat = log (pxl_len) / log (2.0);

    TSMTRACE ((CACHE_MASK|2, funcName,
	       "DEM[%d] = %.1f pxls/post OI[%d] (log2 %.2f)",
	       demLevel, pxl_len, imLevel, log_rat));

    geov->oi_dem_offset = floor (log_rat + demLevel - imLevel);
    TSMTRACE ((CACHE_MASK|1, funcName, "OI/DEM offset %d", 
	       geov->oi_dem_offset));
  }
    
  /* Callbacks specific to this GeoPyramid arrangement. */
  TsCacheAddCallback (geov->dem, TILE_CACHE,   TsCacheDEM_TileMap, geov, NULL);
  TsCacheAddCallback (geov->dem, TILE_DECACHE, TsCacheDEM_freeAux, NULL, NULL);

  TsCacheAddCallback (geo->im, TILE_INIT, TsGeoOIDEM_TileExtent, geov, NULL);
  TsCacheAddCallback (geo->im, TILE_DECACHE, TsGeoOIDEM_freeOIAux, NULL, NULL);

  TsCacheOI_SetupRemap (geo->im);

  /* Create default request lists */
  geov->baseView = TsGeoCreateView (geo);
  TsGeoOIDEM_InitLoad (geov->baseView);

  /* Now, initialize dataset bounding box. */
  TsGeoOIDEM_SetBBox (geo);

  TsGeoCreate_Common (geo);

  TSMTRINC ((CACHE_MASK|1, -1));
  TSMTRACE ((CACHE_MASK|1, funcName, "<= %p", geo));
  return (TsGeoPyramid *) geo;
}

static void
TsGeoOIDEM_Destroy (TsGeoPyramid *geo)
{
  static char const funcName[] = "TsGeoOIDEM_Destroy";
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;
  TsCache *im = geo->im;
  TsCache *dem = geov->dem;

  TSMTRACE ((CACHE_MASK|1, funcName, "(%p)", geo));
  TSMTRINC ((CACHE_MASK|1, 1));

  /* Call superclass destructor. */
  TsGeoDestroy_Common (geo);

  if ( geov->baseView ) TsGeoViewDestroy (geov->baseView);

  /* Ensure that im doesn't depend on dem anymore. */
  if (dem) TsCacheRemoveCallbacks (dem, TILE_CACHE);
  if (im)  TsCacheRemoveCallbacks (im, TILE_CACHE);

  /* First disconnect them... */
  if (im)  TsCacheDisconnect (im);
  if (dem) TsCacheDisconnect (dem);

  /* Now it is safe to destroy them */
  if (im)  TsCacheDestroy (im);
  if (dem) TsCacheDestroy (dem);

  if (geov->convertLinear) free ((void *) geov->convertLinear);

  if ( geo->name ) tsmMemFree ((void*) geo->name);

  free (geov);

  TSMTRINC ((CACHE_MASK|1, -1));
  TSMTRACE ((CACHE_MASK|1, funcName, "Done."));
}

static int
TsGeoOIDEM_Connections (TsGeoPyramid *geo, TsmConnection *cons)
{
  /*static char const funcName[] = "TsGeoOIDEM_Connections";*/
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;
  TsmConnection con;
  int n = 0;
  if ((con = TsCacheConnection (geo->im))) 
    cons[n++] = con;
  if ((con = TsCacheConnection (geov->dem))) 
    cons[n++] = con;
  return n;
}

typedef struct {
  TsGeoView super;

  tsmRequestList *oiReq;
  tsmRequestList *demReq;
} TsGeoView_OIDEM;

static int TsGeoOIDEM_PrepareTile (TsGeoView *gv,
				   TsTile *tile, int const *request_prio);
static void TsGeoOIDEM_SyncView (TsGeoView *geo);
static void TsGeoOIDEM_ClearView (TsGeoView *geo);
static void TsGeoOIDEM_DestroyView (TsGeoView *geo);

static TsGeoViewVTable TsGeoOIDEM_viewTable = {
  TsGeoOIDEM_PrepareTile,
  TsGeoOIDEM_SyncView,
  TsGeoOIDEM_ClearView,
  TsGeoOIDEM_DestroyView
};

static TsGeoView *
TsGeoOIDEM_CreateView (TsGeoPyramid *geo)
{
  static char const funcName[] = "TsGeoOIDEM_CreateView";
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;
  TsGeoView_OIDEM *v = malloc (sizeof (*v));
  TsGeoView *super = &v->super;

  super->geo = geo;
  super->vtable = &TsGeoOIDEM_viewTable;

  v->oiReq = TsCacheRequestList (geo->im);
  v->demReq = TsCacheRequestList (geov->dem);

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", v, geo));
  return &v->super;
}

static int
TsGeoOIDEM_InitLoad (TsGeoView *gv)
{
  static char const funcName[] = "TsGeoOIDEM_InitLoad";
  TsGeoView_OIDEM *view = (TsGeoView_OIDEM *) gv;
  TsGeoPyramid *geo = gv->geo;
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, geo));

  /* Load a small subset of the tiles for each. */
  TsCacheDEM_InitLoad (geov->dem, view->demReq);
  TsCacheOI_InitLoad (geo->im, view->oiReq);

  return TRUE;
}

static void
TsGeoOIDEM_SyncView (TsGeoView *gv)
{
  static char const funcName[] = "TsGeoOIDEM_SyncView";
  TsGeoView_OIDEM *view = (TsGeoView_OIDEM *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));

  tsmRequestSubmit (view->oiReq);
  tsmRequestSubmit (view->demReq);
}

static void
TsGeoOIDEM_ClearView (TsGeoView *gv)
{
  static char const funcName[] = "TsGeoOIDEM_ClearView";
  TsGeoView_OIDEM *view = (TsGeoView_OIDEM *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));

  tsmRequestSubmit (view->oiReq);  tsmRequestSubmit (view->oiReq);
  tsmRequestSubmit (view->demReq); tsmRequestSubmit (view->demReq);
}

static void
TsGeoOIDEM_DestroyView (TsGeoView *gv)
{
  static char const funcName[] = "TsGeoOIDEM_DestroyView";
  TsGeoView_OIDEM *view = (TsGeoView_OIDEM *) gv;

  TSMTRACE ((CACHE_MASK|1, funcName, "%p (geo %p)", gv, gv->geo));
  tsmRequestDestroy (view->oiReq);
  tsmRequestDestroy (view->demReq);
  free (view);
}

/*
 * Doc++ Description: *//**
 *   Turns on/off logging functions for capturing geoset statistics.
 *
 * @param geo      handle to geoset to be logged
 * @param logging  boolean value:  0 -  turn off logging,  1 - begin logging
 *
 * @return Returns the previous state of logging (0 or 1).
 *
 * @see    TsGeoPyramid
 *
 */
int
TsGeoLogging (TsGeoPyramid *geo, int logging)
{
  int prev = geo->logging;
  if (logging != prev) {
    int i;
    TsmConnection connect[2];
    int numConnects;

    if ( (numConnects = TsGeoConnections (geo, connect)) )
      for ( i = 0; i < numConnects; ++i )
	NetLogConnection( connect[i], logging );

    geo->logging = logging;
  }

  return prev;
}

static void TsCacheOI_InitElevationRange (TsCache const *cache, TsTile *tile);
static int TsGeoOIDEM_TilePriority (TsTile const *tile);

static TsTile const *
TsCacheDEM_PropagateElevationRange (TsCache const *cache,
				    TsTile const *tile,
				    int (*prio_f)(TsTile const *));

static float
clampf (float v, float v0, float v1)
{
  if (v < v0) return v0;
  else if (v > v1) return v1;
  else return v;
}

float
TsGeoPixelDiameter2 (TsGeoPyramid const *geo, int res)
{
  float const *ixiytoUV = TsCacheTfm_ixiytouv (geo->im, res);
  double du = TFM_DU (1, 0, ixiytoUV);
  double dv = TFM_DV (0, 1, ixiytoUV);
  double g0[3];
  double d = 0;
  int i, j;

  TsGeoUVtoGCC (geo,  0,  0, 0, g0);

  for (j = 0; j < 2; ++j) { 
    double g[3], d0 = 0;
    TsGeoUVtoGCC (geo, j?0:du, j?dv:0, 0, g);
    for (i = 0; i < 3; ++i) {
      double d1 = g[i] - g0[i];
      d0 += d1*d1;
    }
    if (d0 > d) d = d0;
  }

  return d;
}
float
TsGeoPixelDiameter (TsGeoPyramid const *geo, int res)
{
  return sqrt (TsGeoPixelDiameter2 (geo, res));
}

static void
TsCacheOI_InitElevationRange (TsCache const *cache, TsTile *tile)
{
  static char const funcName[] = "TsCacheOI_InitElevationRange";
  float min = FLT_MAX;
  float max = -FLT_MAX;

  TsTile *children[4];
  int nc = TsCacheTileChildren_const (tile, children);

  if (nc) {
    int i;
    TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]",
	       tile->id.x, tile->id.y, tile->id.res));
    TSMTRINC ((CACHE_MASK|2, 1));

    for (i = 0; i < nc; ++i) {
      TsTile const *child = children[i];
      if (TsCacheTilePrepared (child)) {
	float child_min = child->bbox.min[2];
	float child_max = child->bbox.max[2];
	TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] elev %g -> %g",
		   child->id.x, child->id.y, child->id.res,
		   child_min, child_max));
	if (child_min < min) min = child_min;
	if (child_max > max) max = child_max;
      }
    }

    TSMTRINC ((CACHE_MASK|2, -1));
    if (min != FLT_MAX)
      TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] elev %g -> %g",
		 tile->id.x, tile->id.y, tile->id.res, min, max));
  }

  tile->bbox.min[2] = min;
  tile->bbox.max[2] = max;
}

void
TsCacheOI_InitFinalBBox (TsCache const *cache, TsTile *tile)
{
  static char const funcName[] = "TsCacheOI_InitFinalBBox";

  TsTile *children[4];
  int nc = TsCacheTileChildren_const (tile, children);

  TsBBoxInit (&tile->bbox);

  if (nc) {
    int i;
    TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]",
	       tile->id.x, tile->id.y, tile->id.res));
    TSMTRINC ((CACHE_MASK|2, 1));

    for (i = 0; i < nc; ++i) {
      TsTile const *child = children[i];
      if (TsCacheTilePrepared (child)) {
	TsGeoTileAux const *aux = child->aux;
	if (aux->npolys)
	  TsBBoxAddBBox (&tile->bbox, &child->bbox);
      }
    }

    TSMTRINC ((CACHE_MASK|2, -1));
  }
}

static int
TsGeoOIDEM_TilePriority (TsTile const *tile)
{
  if (TsCacheTilePrepared (tile))
    return 1;
  else
    return 0;
}

/* Walk up tree and update bounding boxes. Returns top tile only if
   propagation reached that far and modified it. */

static TsTile const *
TsCacheDEM_PropagateElevationRange (TsCache const *cache, TsTile const *tile,
				    int (*prio_f)(TsTile const *))
{
  static char const funcName[] = "TsCacheDEM_PropagateElevationRange";
  float min = tile->bbox.min[2];
  float max = tile->bbox.max[2];
  int prio = (prio_f ? (*prio_f) (tile) : 0);
  TsCoordType coordType = tile->bbox.coordType;
  TsTile *curr = TsCacheTileParent_const (tile);
  TsTile *prev = 0;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] prio %d elev %g->%g",
	     tile->id.x, tile->id.y, tile->id.res, prio, min, max));
  TSMTRINC ((CACHE_MASK|2, 1));

  for ( ; curr; curr = TsCacheTileParent_const (curr)) {
    int curr_prio;
    prev = 0;
    /* Propagate into prepared tiles, but never propagate a non-resident tile
       into a resident tile */
    if (TsCacheTilePrepared (curr)
	&& (prio >= (curr_prio = (prio_f ? (*prio_f) (curr) : 0)))) {
      TsBBox *bbox = &curr->bbox;
      float curr_min = bbox->min[2];
      float curr_max = bbox->max[2];
      int changed = 0;

      /* Check for compatible bboxes. */
      assert (coordType == bbox->coordType);

      /* Check for expansion of z range. Change if needed. */
      if (min < curr_min) { bbox->min[2] = min; ++changed; }
      if (max > curr_max) { bbox->max[2] = max; ++changed; }

      if (changed) {
	TSMTRACE ((CACHE_MASK|2, funcName,
		   "[%d %d %d] prio %d elev %g->%g (was %g->%g)",
		   curr->id.x, curr->id.y, curr->id.res, curr_prio,
		   bbox->min[2], bbox->max[2],
		   curr_min, curr_max));
	TsBBoxFinish (bbox);
	prev = curr;
      }
    }
  }

  if (prev)
    TSMTRACE ((CACHE_MASK|2, funcName, "<= [%d %d %d]",
	       prev->id.x, prev->id.y, prev->id.res));

  TSMTRINC ((CACHE_MASK|2, -1));
  return prev;
}

#if 0
/* Walk down a tree, ensuring the descendant tiles are all properly nested. */

static void
TsCacheOI_PropagateBBoxDown (TsCache const *cache, 
			     TsTile *tile, int prop_prio)
{
  static char const funcName[] = "TsCacheOI_PropagateBBoxDown";
  TsBBox *prop_bbox = &tile->bbox;
  TsTile *children[4];
  int nchildren = TsCacheTileChildren_const (cache, tile, children);
  int i;

  for (i = 0; i < nchildren; ++i) {
    TsTile *curr = children[i];
    if (TsCacheTilePrepared (curr)) {
      TsBBox *bbox = &curr->bbox;
      int curr_prio = TsGeoOIDEM_TilePriority (curr);
      TSMTRACE ((CACHE_MASK|2, funcName,
		 "[%d %d %d] prio %d: %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
		 curr->id.x, curr->id.y, curr->id.res, curr_prio,
		 bbox->min[0], bbox->min[1], bbox->min[2],
		 bbox->max[0], bbox->max[1], bbox->max[2]));
      if (prop_prio > curr_prio 
	  || (prop_prio == curr_prio && curr_prio == 1)) {
	/* Shrink child bbox so that it fits. */
	int changed = 0, j;
	for (j = 0; j < 3; ++j) {
	  if (bbox->min[j] < prop_bbox->min[j]) {
	    bbox->min[j] = prop_bbox->min[j];
	    ++changed;
	  }
	  if (bbox->max[j] > prop_bbox->max[j]) {
	    bbox->max[j] = prop_bbox->max[j];
	    ++changed;
	  }
	}
	/* If we changed this box, then finish it and recurse down. */
	if (changed) {
	  TSMTRACE ((CACHE_MASK|2, funcName,
		     "[%d %d %d] prio %d: %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
		     curr->id.x, curr->id.y, curr->id.res, curr_prio,
		     bbox->min[0], bbox->min[1], bbox->min[2],
		     bbox->max[0], bbox->max[1], bbox->max[2]));
	  TsBBoxFinish (bbox);
	  TsCacheOI_PropagateBBoxDown (cache, curr, prop_prio);
	}
      } else if (TsBBoxAddBBox (prop_bbox, bbox)) {
	/* Since we just augmented tile's bbox, we may need to propagate that
	   up. */
	TSMTRACE ((CACHE_MASK|2, funcName,
		   "[%d %d %d] prio %d: %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
		   curr->id.x, curr->id.y, curr->id.res, curr_prio,
		   bbox->min[0], bbox->min[1], bbox->min[2],
		   bbox->max[0], bbox->max[1], bbox->max[2]));
	TsCacheOI_PropagateBBox (cache, tile);
      }
    }
  }
}
#endif

/* Walk up tree and update bounding boxes. Returns top tile only if
   propagation reached that far and */

TsTile const *
TsCacheOI_PropagateBBox (TsCache const *cache, TsTile *tile)
{
  static char const funcName[] = "TsCacheOI_PropagateBBox";
  int prop_prio = TsGeoOIDEM_TilePriority (tile);
  TsCoordType coordType = tile->bbox.coordType;
  TsTile *curr = TsCacheTileParent_const (tile);
  TsTile *prev = 0;
  TsBBox const *prop_bbox = &tile->bbox;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] prio %d",
	     tile->id.x, tile->id.y, tile->id.res, prop_prio));
  TSMTRINC ((CACHE_MASK|2, 1));
  TSMTRACE ((CACHE_MASK|2, funcName, "%.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
	     prop_bbox->min[0], prop_bbox->min[1], prop_bbox->min[2],
	     prop_bbox->max[0], prop_bbox->max[1], prop_bbox->max[2]));

  /* Ensure that all descendant tiles are properly nested. */
#if 0
  TsCacheOI_PropagateBBoxDown (cache, tile, prop_prio);
#endif

  /* Now walk up parent list. */
  for ( ; curr; curr = TsCacheTileParent_const (curr)) {
    int curr_prio;
    TsBBox *bbox = &curr->bbox;

    /* Propagate into prepared tiles, but never propagate a non-resident tile
       into a resident tile */
    if (! TsCacheTilePrepared (curr)
	|| (prop_prio < (curr_prio = TsGeoOIDEM_TilePriority (curr)))) {
      prev = 0;
      break;
    }

    /* Check for compatible bboxes. */
    assert (coordType == bbox->coordType);

    /* Check for expansion of z range. Change if needed. */
    if (TsBBoxAddBBox (bbox, prop_bbox)) {
      TSMTRACE ((CACHE_MASK|2, funcName,
		 "[%d %d %d] prio %d: %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
		 curr->id.x, curr->id.y, curr->id.res, curr_prio,
		 bbox->min[0], bbox->min[1], bbox->min[2],
		 bbox->max[0], bbox->max[1], bbox->max[2]));
      TsBBoxFinish (bbox);
      prev = curr;
    } else {
      /* We only need to go up until we don't expand the bbox. */
      prev = 0;
      break;
    }
  }

  if (prev)
    TSMTRACE ((CACHE_MASK|2, funcName, "<= [%d %d %d]",
	       prev->id.x, prev->id.y, prev->id.res));

  TSMTRINC ((CACHE_MASK|2, -1));
  return prev;
}

static int
TsGeoOIDEM_AddCallback (TsGeoPyramid *geo, TsCacheCB which,
			TsCallbackF func, void *closure,
			void (*destroyf)(void*))
{
  int ret;
  TsGeoPyramid_OIDEM const *geov = (TsGeoPyramid_OIDEM const *) geo;
  ret = TsCacheAddCallback (geo->im, which, func, closure, destroyf);
  ret &= TsCacheAddCallback (geov->dem, which, func, closure, 0);
  return ret;
}

static int
TsGeoOIDEM_RemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
			   TsCallbackF func, void *closure)
{
  int ret;
  TsGeoPyramid_OIDEM const *geov = (TsGeoPyramid_OIDEM const *) geo;
  ret = TsCacheRemoveCallback (geo->im, which, func, closure);
  ret &= TsCacheRemoveCallback (geov->dem, which, func, closure);
  return ret;
}

static int
TsGeoOIDEM_URLs (TsGeoPyramid const *geo, char const **urls)
{
  TsGeoPyramid_OIDEM const *geov = (TsGeoPyramid_OIDEM *) geo;
  urls[0] = geo->im->tspec->pathName;
  urls[1] = geov->dem->tspec->pathName;
  return 2;
}


static void
rangeTfmIXIY (float u0, float v0, float u1, float v1, float const *uvtoixiy,
	      float ixp[2], float iyp[2])
{
  float min_ix, min_iy, max_ix, max_iy, ix, iy;

  ix = TFM_IX (u0, v0, uvtoixiy);
  iy = TFM_IY (u0, v0, uvtoixiy);
  min_ix = max_ix = ix;
  min_iy = max_iy = iy;
  ix = TFM_IX (u1, v0, uvtoixiy);
  iy = TFM_IY (u1, v0, uvtoixiy);
  if (ix < min_ix) min_ix = ix; else if (ix > max_ix) max_ix = ix;
  if (iy < min_iy) min_iy = iy; else if (iy > max_iy) max_iy = iy;
  ix = TFM_IX (u1, v1, uvtoixiy);
  iy = TFM_IY (u1, v1, uvtoixiy);
  if (ix < min_ix) min_ix = ix; else if (ix > max_ix) max_ix = ix;
  if (iy < min_iy) min_iy = iy; else if (iy > max_iy) max_iy = iy;
  ix = TFM_IX (u0, v1, uvtoixiy);
  iy = TFM_IY (u0, v1, uvtoixiy);
  if (ix < min_ix) min_ix = ix; else if (ix > max_ix) max_ix = ix;
  if (iy < min_iy) min_iy = iy; else if (iy > max_iy) max_iy = iy;

  ixp[0] = min_ix;
  ixp[1] = max_ix;
  iyp[0] = min_iy;
  iyp[1] = max_iy;
}

static void
rangeTfmUV (float ix0, float iy0, float ix1, float iy1, float const *ixiytouv,
	    float up[2], float vp[2])
{
  float min_u, min_v, max_u, max_v, u, v;

  u = TFM_U (ix0, iy0, ixiytouv);
  v = TFM_V (ix0, iy0, ixiytouv);
  min_u = max_u = u;
  min_v = max_v = v;
  u = TFM_U (ix1, iy0, ixiytouv);
  v = TFM_V (ix1, iy0, ixiytouv);
  if (u < min_u) min_u = u; else if (u > max_u) max_u = u;
  if (v < min_v) min_v = v; else if (v > max_v) max_v = v;
  u = TFM_U (ix1, iy1, ixiytouv);
  v = TFM_V (ix1, iy1, ixiytouv);
  if (u < min_u) min_u = u; else if (u > max_u) max_u = u;
  if (v < min_v) min_v = v; else if (v > max_v) max_v = v;
  u = TFM_U (ix0, iy1, ixiytouv);
  v = TFM_V (ix0, iy1, ixiytouv);
  if (u < min_u) min_u = u; else if (u > max_u) max_u = u;
  if (v < min_v) min_v = v; else if (v > max_v) max_v = v;

  up[0] = min_u;
  up[1] = max_u;
  vp[0] = min_v;
  vp[1] = max_v;
}

/* Computer the set of DEM tiles that will be needed to calculate the OI tile
   elevations.  These elevations can only be calculated *after* all of these
   DEMs are loaded. The TsTileRange is therefore stored in the OI tile's aux
   data and referred to from there. */

static void
TsOIBBoxToDEMTileRange (TsGeoPyramid_OIDEM const *geov, int res, 
			TsBBox const *bbox, float expand,
			TsTileRange *range)
{
  static char const funcName[] = "TsOIBBoxToDEMTileRange";
  
  TsCache const *cache = geov->dem;
  float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
  SimpleTileSet const *tset = TsCacheTileSet (cache, res);
  int min_ix = TsCacheMinPixelX (cache, res);
  int max_ix = TsCacheMaxPixelX (cache, res);
  int min_iy = TsCacheMinPixelY (cache, res);
  int max_iy = TsCacheMaxPixelY (cache, res);
  int tile_w = tset->tileTspec.numPixelsX;
  int tile_h = tset->tileTspec.numPixelsY;

  float u0, v0, u1, v1, du, dv;
  float ix[2], iy[2];

  TsGeoOIDEM_OItoDEM (geov, bbox->min[0], bbox->min[1], &u0, &v0);
  TsGeoOIDEM_OItoDEM (geov, bbox->max[1], bbox->max[1], &u1, &v1);

  if (u1 < u0) { float t = u1; u1 = u0; u0 = t; }
  if (v1 < v0) { float t = v1; v1 = v0; v0 = t; }
  du = expand*(u1-u0);
  dv = expand*(v1-v0);

  assert (bbox->coordType == TsCoordUV);

  /* Expand bbox by fractional expansion requested. */
  u0 -= du; u1 += du;
  v0 -= dv; v1 += dv;

  TSMTRACE ((CACHE_MASK|2, funcName, "res %d: U,V: %.1f,%.1f -> %.1f,%.1f",
	     res, u0, v0, u1, v1));

  rangeTfmIXIY (u0, v0, u1, v1, uvtoixiy, ix, iy);

  TSMTRACE ((CACHE_MASK|2, funcName, "res %d: IX,IY: %g,%g -> %g,%g",
	     res, ix[0], iy[0], ix[1], iy[1]));

  if (ix[0] < min_ix) ix[0] = min_ix;
  if (ix[1] > max_ix) ix[1] = max_ix;
  if (iy[0] < min_iy) iy[0] = min_iy;
  if (iy[1] > max_iy) iy[1] = max_iy;

  TSMTRACE ((CACHE_MASK|2, funcName, "res %d: IX,IY: %g,%g -> %g,%g",
	     res, ix[0], iy[0], ix[1], iy[1]));

  range->res = res;
  range->min_x = floor (ix[0]/tile_w);
  range->max_x = floor (ix[1]/tile_w);
  range->min_y = floor (iy[0]/tile_h);
  range->max_y = floor (iy[1]/tile_h);
  range->ntiles  = (range->max_x-range->min_x+1);
  range->ntiles *= (range->max_y-range->min_y+1);

  TSMTRACE ((CACHE_MASK|2, funcName, "res %d: tile %d,%d -> %d,%d",
	     res, range->min_x, range->min_y, range->max_x, range->max_y));
}

/* Calculate the number of polygons on a side when we tesselate this tile.
   Eventually, this should be more sophisticated.  For now, we simply use the
   TsGeoPyramid's default value unless that would produce quads that are
   greater than 500Km on a side.  We make this limitiation to preserve the
   earth's curvature for very low resolution tiles.
*/

static int
TsGeoOIDEM_TileNumPolys (TsGeoPyramid_OIDEM const *geov, TsTile const *tile)
{
  static const char funcName[] = "TsGeoOIDEM_TileNumPolys";

  int num_polys = geov->num_polys;
  TsOITileAux *aux = tile->aux;
  TsBBox const *bbox = &tile->bbox;

  /* Let us correct for large curvatures */
  if (aux) {
    /* So bbox is in meters. */
    float radius = bbox->radius / num_polys;

    /* 5e5 meters = 500km */
    while (radius > 5e5 && num_polys < 16) {
      num_polys *= 2;
      radius /= 2;
    }
    TSMTRACE ((CACHE_MASK|1, funcName,
	       "%p[%d %d %d] %d polys %.1fm/poly",
	       tile, tile->id.x, tile->id.y, tile->id.res,
	       num_polys, radius));
  }

  return num_polys;
}

/* Create the whole set of auxiliary data for an OI tile.
   This includes calculating the range of DEM tiles needed for support, an
   initial estimate of the bounding box, and then the base elevations and a
   good estimate of the bounding box if the supporting DEM tiles have been
   prepared.
 */

static void
TsGeoOIDEM_MakeOIAux (TsGeoPyramid const *geo, TsTile *tile,
		      float min_z, float max_z)
{
  static char const funcName[] = "TsGeoOIDEM_MakeOIAux";
  TsGeoPyramid_OIDEM const *geov = (TsGeoPyramid_OIDEM *) geo;
  TileSetTspec const *demTspec = TsCacheTspec (geov->dem);
  int minLevel = demTspec->minLevel;
  int maxLevel = demTspec->maxLevel;
  PRUint32 aux_size;
  TsOITileAux *aux;
  TsTileRange range;
  int i, j, idx;
  float expand = 1.0/TsGeoOIDEM_TileNumPolys (geov, tile);
  int all_res = TRUE;

  int res = tile->id.res + geov->oi_dem_offset;
  res = min (res, maxLevel);
  res = max (res, minLevel);

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]",
	     tile->id.x, tile->id.y, tile->id.res));
  TSMTRINC ((CACHE_MASK|2, 1));

  /* We need the tile's extent in UV coords. Use the saved UV bbox.. */
  TsOIBBoxToDEMTileRange (geov, res, &tile->bbox, expand, &range);

  aux_size = sizeof (*aux) + range.ntiles * sizeof (TsTile *);
  aux = (TsOITileAux *) malloc (aux_size);
  aux->size = aux_size;
  aux->demRef = range;
  aux->demRef.tiles = (TsTile **) (aux+1);
  aux->uv_bbox = tile->bbox;
  aux->demCoords = 0;
  aux->iter = 0;
  aux->geo.coords = 0;
  aux->geo.npolys = 0;
  
  for (idx = 0, j = aux->demRef.min_y; j <= aux->demRef.max_y; ++j)
    for (i = aux->demRef.min_x; i <= aux->demRef.max_x; ++i, ++idx) {
      TsTile *demTile = TsCacheGetTile (geov->dem, i, j, res);
      TsCacheTileRef (demTile);
      aux->demRef.tiles[idx] = demTile;
      if (! TsCacheTilePrepared (demTile))
	all_res = FALSE;
    }

  tile->aux = aux;
  TsCacheTileAddMem (tile, aux->size);

  /* Initial estimate of UV bbox */
  TsGeoOIDEM_TileComputeBBox (geo, tile, min_z, max_z);

  /* If all DEM tiles are available, then improve the estimate of UV bbox */
  if (all_res)
    TsGeoOIDEM_TileElevs (geov, tile);

  TSMTRINC ((CACHE_MASK|2, -1));
}

static void
TsGeoOIDEM_freeOIAux (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "TsGeoOIDEM_freeOIAux";
  TsTile *tile = arg;
  if (tile->aux) {
    TsOITileAux *aux = tile->aux;
    int i, n = aux->demRef.ntiles;
    for (i = 0; i < n; ++i)
      TsCacheTileDeref (aux->demRef.tiles[i]);
    if (aux->demCoords) free (aux->demCoords);
    if (aux->iter) free (aux->iter);
    if (aux->geo.coords) free (aux->geo.coords);
    TsCacheTileAddMem (tile, -(int)aux->size);
    free (aux);
    tile->aux = 0;
  }
}

static inline float
Linearf (float d, float v0, float v1)
{
  return (1-d)*v0 + d*v1;
}

static inline float
BiLinearf (float dx, float dy, float v00, float v10, float v01, float v11)
{
  return Linearf (dy, Linearf (dx, v00, v10), Linearf (dx, v01, v11));
}

/*
 * Synopsis:
 *   int clampUV (TsCache const *cache, int res, float *u, float *v)
 *
 * Doc++ Description: *//** 
 *   Clamp a location (u,v) to the footprint of a tileset level.
 *
 *   Takes a level descriptor (cache, res) and clamps the location (u,v) so it
 *   falls within the footprint of the level.
 *
 * @param    cache The tileset cache against which (*u,*v) is clamped.
 * @param    res The level of the cache against which (*u,*v) is clamped.
 * @param    u A pointer to the u coordinate
 * @param    v A pointer to the v coordinate
 *
 * @return   True iff (*u,*v) was modified.
 *
 * @author   Lee Iverson
 * @version  12 June 1998
 *
 * @see      TFM_U, TFM_V, TFM_IX, TFM_IY, SimpleTileSet
 * */

static int
clampUV (TsCache const *cache, int res, float *u, float *v)
{
  SimpleTileSet const *tset = TsCacheTileSet (cache, res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, res);
  float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
  float min_ix = TsCacheMinPixelX (cache, res);
  float max_ix = TsCacheMaxPixelX (cache, res);
  float min_iy = TsCacheMinPixelY (cache, res);
  float max_iy = TsCacheMaxPixelY (cache, res);
  int clamp = 0;
  float ixf, iyf;

  /* Calculate (ix,iy) from (u,v) */
  ixf = TFM_IX (*u, *v, uvtoixiy);
  iyf = TFM_IY (*u, *v, uvtoixiy);

  /* Now clamp these values */
  if (ixf < min_ix) { ixf = min_ix; clamp = 1; }
  else if (ixf > max_ix) { ixf = max_ix; clamp = 1; }
  if (iyf < min_iy) { iyf = min_iy; clamp = 1; }
  else if (iyf > max_iy) { iyf = max_iy; clamp = 1; }

  /* If they were clamped then map (ix,iy) back to (u,v) */
  if (clamp) {
    *u = TFM_U (ixf, iyf, ixiytouv);
    *v = TFM_V (ixf, iyf, ixiytouv);
  }
  return clamp;
}

/* Compute the elevation of a particular (U,V) point in the OI's coordinate
   system by reference to the underlying DEM tiles. */

static float
TsGeoOIDEM_Elevation (TsGeoPyramid const *geo,
		      float u, float v)
{
  static char const funcName[] = "TsGeoOIDEM_Elevation";
  TsGeoPyramid_OIDEM const *geov = (TsGeoPyramid_OIDEM const *) geo;
  TsCache *dem = geov->dem;
  TileSetTspec const *tspec = TsCacheTspec (dem);
  TsBBox const *uv_bbox = TsCacheBBox (dem);
  float const slop = 1;
  static TsTile const *tile = 0;

  /* Convert to DEM coordinates. */
  TsGeoOIDEM_OItoDEM (geov, u, v, &u, &v);

  assert (uv_bbox->coordType == TsCoordUV);

  /* Remove slop and fail if outside bounds. */
  if (u    < uv_bbox->min[0]-slop || u > uv_bbox->max[0]+slop
      || v < uv_bbox->min[1]-slop || v > uv_bbox->max[1]+slop) {
    TSMTRACE ((CACHE_MASK|2, funcName, "%g,%g outside bbox [%g,%g -> %g,%g]",
	       u, v,
	       uv_bbox->min[0], uv_bbox->min[1],
	       uv_bbox->max[0], uv_bbox->max[1]));
    return TS_BAD_ELEVATION;
  }

  /* Bound by DEM limits. */
  u = clampf (u, uv_bbox->min[0], uv_bbox->max[0]);
  v = clampf (v, uv_bbox->min[1], uv_bbox->max[1]);

  return TsCacheDEM_Elevation (dem, &tile, u, v, tspec->minLevel);
}

typedef struct TsOITileIteration {
  int iy0, iy1, iy_spacing;
  float min_iyf, max_iyf;
  int ix0, ix1, ix_spacing;
  float min_ixf, max_ixf;
} TsOITileIteration;

/*
 * Synopsis:
 *   void TsGeoOIDEM_TileElevs (TsGeoPyramid_OIDEM const *geov, TsTile *tile)
 *
 * Doc++ Description: *//** 
 *   Determines the polygon iteration points for the given tile and calculates
 *   the DEM coordinates of those points (u, v, elevation) which are stored in
 *   the demCoords field of the tile's TsOITileAux aux field.  Also saves the
 *   polygon iteration parameters.  Both of these saved values are used by
 *   TsGeoOIDEM_TilePoints.
 *
 *   The primary purpose of this routine is actually to calculate the accurate
 *   UV and LVCS bounding boxes of the tile as early as possible.
 *
 * @param    geov	The OI/DEM TsGeoPyramid in which the tile resides.
 * @param    tile	A tile in geov.
 *
 * @return   void
 *
 * @author   Lee Iverson
 * @version  January 2001
 *
 */

static void
TsGeoOIDEM_TileElevs (TsGeoPyramid_OIDEM const *geov, TsTile *tile)
{
  static char const funcName[] = "TsGeoOIDEM_TileElevs";
  TsOITileAux *aux = tile->aux;

  if (! aux->geo.npolys) {
    TsCache const *imCache = geov->geo.im;
    SimpleTileSet const * const tset = TsCacheTileSet (imCache, tile->id.res);
    
    TsBBox const *uv_bbox = &imCache->bbox;
    float const *ixiytouv = TsCacheTfm_ixiytouv (imCache, tile->id.res);
    float const *uvtoixiy = TsCacheTfm_uvtoixiy (imCache, tile->id.res);

    int npolys = TsGeoOIDEM_TileNumPolys (geov, tile);
    int tile_w = tset->tileTspec.numPixelsX;
    int tile_h = tset->tileTspec.numPixelsY;
    int ix_spacing = tile_w / npolys;
    int iy_spacing = tile_h / npolys;
    int tile_ix = tile_w * tile->id.x;
    int tile_iy = tile_h * tile->id.y;
    int ix = tile_ix;
    int iy = tile_iy;
    int ixe = tile_ix + tile->pixels[0];
    int iye = tile_iy + tile->pixels[1];
    int ix0 = ix;
    int iy0 = iy;
    int ix1, iy1;
    int coords_w, coords_h, coords_len;

    /* The code below assumes that the TFM matrix is positive definite...
       Is that a problem? */
    float min_u = uv_bbox->min[0];
    float max_u = uv_bbox->max[0];
    float min_v = uv_bbox->min[1];
    float max_v = uv_bbox->max[1];

    float min_ixf = TFM_IX (min_u, min_v, uvtoixiy);
    float min_iyf = TFM_IY (min_u, min_v, uvtoixiy);
    float max_ixf = TFM_IX (max_u, max_v, uvtoixiy);
    float max_iyf = TFM_IY (max_u, max_v, uvtoixiy);

    if (min_ixf < tile_ix) min_ixf = tile_ix;
    if (max_ixf > ixe) max_ixf = ixe;
    if (min_iyf < tile_iy) min_iyf = tile_iy;
    if (max_iyf > iye) max_iyf = iye;

    TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d] %.1f-%.1f, %.1f-%.1f",
	       tile, tile->id.x, tile->id.y, tile->id.res,
	       min_ixf, max_ixf, min_iyf, max_iyf));

    /* Find bounds of ix */
    coords_w = 1;
    for ( ; ix < min_ixf; ix += ix_spacing)
      ix0 = ix;
    if (ix > min_ixf) ++coords_w;
    for ( ; ix < max_ixf; ix += ix_spacing, ++coords_w) {
      TSMTRACE ((CACHE_MASK|3, funcName, "ix %d [%d]", ix, coords_w));
    }
    ix1 = ix;

    coords_h = 1;
    for ( ; iy < min_iyf; iy += iy_spacing)
      iy0 = iy;
    if (iy > min_iyf) ++coords_h;
    for ( ; iy < max_iyf; iy += iy_spacing, ++coords_h) {
      TSMTRACE ((CACHE_MASK|3, funcName, "iy %d [%d]", iy, coords_h));
    }
    iy1 = iy;

    TSMTRACE ((CACHE_MASK|2, funcName,
	       "X %.1f-%.1f => W %d", min_ixf, max_ixf, coords_w));
    TSMTRACE ((CACHE_MASK|2, funcName,
	       "Y %.1f-%.1f => H %d", min_iyf, max_iyf, coords_h));

    coords_len = coords_w * coords_h;
    TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d] %dx%d = %d pts",
	       tile, tile->id.x, tile->id.y, tile->id.res,
	       coords_w, coords_h, coords_len));

    {
      int size = (coords_len * sizeof (aux->demCoords[0]));
      aux->demCoords = malloc (size);

      TSMTRACE ((CACHE_MASK|2, funcName, "malloc [%p - %p] %d %dx%d",
		 aux->demCoords, ((char*)aux->demCoords)+size,
		 size, coords_w, coords_h));

      aux->geo.coords_w = coords_w;
      aux->geo.coords_h = coords_h;
      aux->size += size;

      /* Save iteration parameters for TsGeoOIDEM_TilePoints */
      aux->iter = malloc (sizeof (*aux->iter));
      aux->iter->ix0 = ix0;
      aux->iter->ix1 = ix1;
      aux->iter->iy0 = iy0;
      aux->iter->iy1 = iy1;
      aux->iter->min_ixf = min_ixf;
      aux->iter->max_ixf = max_ixf;
      aux->iter->min_iyf = min_iyf;
      aux->iter->max_iyf = max_iyf;
      aux->size += sizeof (*aux->iter);
    }

    {
      m4f const *M = &geov->lvcsDEMtoOI;
      int tile_ix = tile_w * tile->id.x;
      int tile_iy = tile_h * tile->id.y;
      int bix = tile_ix;
      int biy = tile_iy;
      int demRes = aux->demRef.res;
      TsTile const *demTile = 0;
      TsBBox *bbox = (TsBBox *) &tile->bbox; /* Cast away const */
      float min_z =  FLT_MAX;
      float max_z = -FLT_MAX;

      v3f *demCoords = aux->demCoords;
      int cx, cy;

      int mid_x = npolys/2;
      int mid_y = npolys/2;

      /* Find starting index and midpoint */
      for ( ; biy+iy_spacing <= iy0; biy += iy_spacing, --mid_y);
      for ( ; bix+ix_spacing <= ix0; bix += ix_spacing, --mid_x);
      if (mid_x >= coords_w) mid_x = coords_w-1;
      if (mid_y >= coords_h) mid_y = coords_h-1;

      assert (bbox->coordType == TsCoordLVCS);
      TsCacheOI_InitFinalBBox (imCache, tile);

      for (iy = biy, cy = 0; iy <= iy1; iy += iy_spacing, ++cy) {
	float y = (iy < min_iyf ? min_iyf : (iy > max_iyf ? max_iyf : iy));
	for (ix = bix, cx = 0;
	     ix <= ix1;
	     ix += ix_spacing, ++demCoords, ++cx) {
	  float x = (ix < min_ixf ? min_ixf : (ix > max_ixf ? max_ixf : ix));
	  float u = TFM_U (x, y, ixiytouv);
	  float v = TFM_V (x, y, ixiytouv);
	  float dem_u, dem_v, elev;
	  v3f pt;
	  v4f v4;

	  if (u < min_u) u = min_u; else if (u > max_u) u = max_u;
	  if (v < min_v) v = min_v; else if (v > max_v) v = max_v;
	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "[%3d]@%p pxl %g %g => %.1f,%.1f",
		     demCoords-aux->demCoords, demCoords, x, y, u, v));

	  /* Convert OI to DEM uv and get elevation. */
	  TsGeoOIDEM_OItoDEM (geov, u, v, &dem_u, &dem_v);
	  elev = TsCacheDEM_Elevation (geov->dem, &demTile,
				       dem_u, dem_v, demRes);

	  
	  V(*demCoords)[0] = dem_u;
	  V(*demCoords)[1] = dem_v;
	  V(*demCoords)[2] = elev;

	  /* Update tile's LVCS bbox with transformed point */
	  TsCacheDEM_PointNormal (geov->dem, &demTile, demRes,
				  dem_u, dem_v, &elev,
				  V(pt), 0);
	  /* Transform point from DEM LVCS -> OI LVCS */
	  mult_m4f_v3f (M, &pt, &v4);

	  TsBBoxAddPoint (bbox, V(v4)[0], V(v4)[1], V(v4)[2]);

	  /* Also update elevation bounds for uv_bbox. */
	  if (elev < min_z) min_z = elev;
	  if (elev > max_z) max_z = elev;
	}
      }

      assert (demCoords == aux->demCoords + coords_len);

      /* Update the uv_bbox. */
      TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] range %.0f-%.0f",
		 tile, tile->id.x, tile->id.y, tile->id.res, 
		 min_z, max_z));
      aux->uv_bbox.min[2] = min_z;
      aux->uv_bbox.max[2] = max_z;
      TsBBoxFinish (&aux->uv_bbox);

      /* Redo the bbox. */
      TsBBoxFinish (bbox);
      /*TsCacheOI_PropagateBBox (imCache, tile);*/
    }

    aux->geo.npolys = npolys;
  }
}

/*
 * Synopsis:
 *   void TsGeoOIDEM_TilePoints (TsGeoPyramid_OIDEM const *geov, TsTile *tile)
 *
 * Doc++ Description: *//** 
 *   Fills in the polygon rendering description in the TsGeoTileAux component
 *   of the tile TsOITileAux aux value.
 *
 *   Depends on the setup information derived by TsGeoOIDEM_TileElevs.
 *
 * @param    geov	The OI/DEM TsGeoPyramid in which the tile resides.
 * @param    tile	A tile in geov.
 *
 * @return   void
 *
 * @author   Lee Iverson
 * @version  January 2001
 *
 * @see TsGeoOIDEM_TileElevs, TsCacheDEM_PointNormal
 */

static void
TsGeoOIDEM_TilePoints (TsGeoPyramid_OIDEM const *geov, TsTile *tile)
{
  static char const funcName[] = "TsGeoOIDEM_TilePoints";
  TsOITileAux *aux = tile->aux;

  /* Don't redo this... */
  if (aux->geo.coords)
    return;

  /* Make sure that we've grabbed the elevations first */
  if (! aux->demCoords)
    TsGeoOIDEM_TileElevs (geov, tile);

  {
    TsCache const *imCache = geov->geo.im;
    SimpleTileSet const * const tset = TsCacheTileSet (imCache, tile->id.res);
    
    TsBBox const *uv_bbox = &imCache->bbox;
    float const *ixiytouv = TsCacheTfm_ixiytouv (imCache, tile->id.res);
    float const *uvtoixiy = TsCacheTfm_uvtoixiy (imCache, tile->id.res);

    int npolys = aux->geo.npolys;
    int tile_w = tset->tileTspec.numPixelsX;
    int tile_h = tset->tileTspec.numPixelsY;
    int ix_spacing = tile_w / npolys;
    int iy_spacing = tile_h / npolys;
    int tile_ix = tile_w * tile->id.x;
    int tile_iy = tile_h * tile->id.y;
    int ix = tile_ix;
    int iy = tile_iy;
    int ix0, iy0, ix1, iy1;
    int coords_w, coords_h, coords_len;

    /* The code below assumes that the TFM matrix is positive definite...
       Is that a problem? */
    float min_u = uv_bbox->min[0];
    float max_u = uv_bbox->max[0];
    float min_v = uv_bbox->min[1];
    float max_v = uv_bbox->max[1];

    float min_ixf;
    float min_iyf;
    float max_ixf;
    float max_iyf;

    /* get iteration parameters from saved values */
    ix0 = aux->iter->ix0;
    ix1 = aux->iter->ix1;
    iy0 = aux->iter->iy0;
    iy1 = aux->iter->iy1;
    min_ixf = aux->iter->min_ixf;
    max_ixf = aux->iter->max_ixf;
    min_iyf = aux->iter->min_iyf;
    max_iyf = aux->iter->max_iyf;

    /* No longer need this information. */
    free (aux->iter);
    aux->size -= sizeof (*aux->iter);
    aux->iter = 0;

    coords_w = aux->geo.coords_w;
    coords_h = aux->geo.coords_h;
    coords_len = coords_w * coords_h;
    TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] %dx%d = %d pts",
	       tile->id.x, tile->id.y, tile->id.res,
	       coords_w, coords_h, coords_len));

    {
      int size = (coords_len * sizeof (aux->geo.coords[0]));
      aux->geo.coords = malloc (size);

      TSMTRACE ((CACHE_MASK|2, funcName, "malloc [%p - %p] %d %dx%d",
		 aux->geo.coords, ((char*)aux->geo.coords)+size,
		 size, coords_w, coords_h));

      aux->size += size;
    }

    {
      m4f const *M = &geov->lvcsDEMtoOI;
      int tile_ix = tile_w * tile->id.x;
      int tile_iy = tile_h * tile->id.y;
      int bix = tile_ix;
      int biy = tile_iy;
      int demRes = aux->demRef.res;
      TsTile const *demTile = 0;
      TsBBox *bbox = (TsBBox *) &tile->bbox; /* Cast away const */
      float min_z =  FLT_MAX;
      float max_z = -FLT_MAX;
      float min_tx, max_tx, min_ty, max_ty;

      v3f *demCoords = aux->demCoords;
      TsCoord *coords = aux->geo.coords;
      PRUint16 *neighIndex = aux->geo.neighIndex;
      int cx, cy;

      int mid_x = npolys/2;
      int mid_y = npolys/2;

      v4f norm;

      /* Find starting index and midpoint */
      for ( ; biy+iy_spacing <= iy0; biy += iy_spacing, --mid_y);
      for ( ; bix+ix_spacing <= ix0; bix += ix_spacing, --mid_x);
      if (mid_x >= coords_w) mid_x = coords_w-1;
      if (mid_y >= coords_h) mid_y = coords_h-1;

      assert (bbox->coordType == TsCoordLVCS);
      TsCacheOI_InitFinalBBox (imCache, tile);

      min_tx = 0.5/(float)tile_w;
      min_ty = 0.5/(float)tile_h;
      max_tx = (tile->pixels[0]-0.5)/(float)tile_w;
      max_ty = (tile->pixels[1]-0.5)/(float)tile_h;

      V(norm)[3] = 0;
      for (iy = biy, cy = 0; iy <= iy1; iy += iy_spacing, ++cy) {
	float y = (iy < min_iyf ? min_iyf : (iy > max_iyf ? max_iyf : iy));
	for (ix = bix, cx = 0;
	     ix <= ix1;
	     ix += ix_spacing, ++coords, ++demCoords, ++cx) {
	  float x = (ix < min_ixf ? min_ixf : (ix > max_ixf ? max_ixf : ix));
	  float u = TFM_U (x, y, ixiytouv);
	  float v = TFM_V (x, y, ixiytouv);
	  float dem_u, dem_v, elev;
	  float tx, ty;
	  v3f pt;
	  v4f v4, n4;

	  if (u < min_u) u = min_u; else if (u > max_u) u = max_u;
	  if (v < min_v) v = min_v; else if (v > max_v) v = max_v;
	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "[%3d]@%p pxl %g %g => %.1f,%.1f",
		     coords-aux->geo.coords, coords, x, y, u, v));

	  /* Convert OI to DEM uv and get elevation. */
	  dem_u = V(*demCoords)[0];
	  dem_v = V(*demCoords)[1];
	  TsCacheDEM_PointNormal (geov->dem, &demTile, demRes,
				  dem_u, dem_v, &V(*demCoords)[2],
				  V(pt), V(norm));

	  /* Transform point from DEM LVCS -> OI LVCS */
	  mult_m4f_v3f (M, &pt, &v4);
	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "DEM %.0f,%.0f,%.0f => OI %.0f,%.0f,%.0f",
		     V(pt)[0], V(pt)[1], V(pt)[2],
		     V(v4)[0], V(v4)[1], V(v4)[2]));
	  *((v3f *)coords->vertex) = *((v3f *) &v4);

	  if (TSMTRACEP (CACHE_MASK|3)) {
	    double dem_gcc[3], oi_gcc[3];
	    TsCacheLVCStoGCC (geov->geo.im,
			      V(v4)[0], V(v4)[1], V(v4)[2], oi_gcc);
	    TsCacheLVCStoGCC (geov->dem,
			      V(pt)[0], V(pt)[1], V(pt)[2], dem_gcc);
	    TSMTRACE ((CACHE_MASK|3, funcName,
		       "DEM: %.0f,%.0f,%.0f == OI: %.0f,%.0f,%.0f",
		       dem_gcc[0], dem_gcc[1], dem_gcc[2],
		       oi_gcc[0], oi_gcc[1], oi_gcc[2]));
	  }

	  /* Update tile's LVCS bbox with transformed point */
	  TsBBoxAddPoint (bbox, V(v4)[0], V(v4)[1], V(v4)[2]);

	  /* Transform normal from DEM LVCS -> OI LVCS */
	  mult_m4f_v4f (M, &norm, &n4);
	  *((v3f *)coords->norm) = *((v3f *) &n4);

	  /* These are wrt. the tile's image. */
	  tx = (x - tile_ix)/(float)tile_w;
	  if (tx < min_tx) tx = min_tx; else if (tx > max_tx) tx = max_tx;

	  ty = (y - tile_iy)/(float)tile_h;
	  if (ty < min_ty) ty = min_ty; else if (ty > max_ty) ty = max_ty;
	  
	  coords->texCoord[0] = tx;
	  coords->texCoord[1] = ty;

	  TSMTRACE ((CACHE_MASK|3, funcName,
		     "[%3d] %.0f,%.0f,%.0f N %.2f,%.2f,%.2f T %.3f,%.3f",
		     coords-aux->geo.coords,
		     coords->vertex[0], coords->vertex[1], coords->vertex[2],
		     coords->norm[0], coords->norm[1], coords->norm[2],
		     coords->texCoord[0], coords->texCoord[1]));

	  /* Also update elevation bounds for uv_bbox. */
	  elev = V(*demCoords)[2];
	  if (elev < min_z) min_z = elev;
	  if (elev > max_z) max_z = elev;
	}
      }

      assert (coords == aux->geo.coords + coords_len);

      /* Initialize the neighIndex pointers. */
      neighIndex[TsCache_SW] = 0;
      neighIndex[TsCache_S]  = mid_x;
      neighIndex[TsCache_SE] = (coords_w-1);
      neighIndex[TsCache_W]  = coords_w*mid_y;
      neighIndex[TsCache_C]  = coords_w*mid_y + mid_x;
      neighIndex[TsCache_E]  = coords_w*mid_y + (coords_w-1);
      neighIndex[TsCache_NW] = coords_w*(coords_h-1);
      neighIndex[TsCache_N]  = coords_w*(coords_h-1) + mid_x;
      neighIndex[TsCache_NE] = coords_w*(coords_h-1) + (coords_w-1);

      TSMTRACE ((CACHE_MASK|2, funcName, "SW: %3d %3d %3d",
		 neighIndex[TsCache_SW], neighIndex[TsCache_S],
		 neighIndex[TsCache_SE]));
      TSMTRACE ((CACHE_MASK|2, funcName, " W: %3d %3d %3d",
		 neighIndex[TsCache_W], neighIndex[TsCache_C],
		 neighIndex[TsCache_E]));
      TSMTRACE ((CACHE_MASK|2, funcName, "NW: %3d %3d %3d",
		 neighIndex[TsCache_NW], neighIndex[TsCache_N],
		 neighIndex[TsCache_NE]));

      if (TSMTRACEP(CACHE_MASK|1)) {
	int i;
	for (i = 0; i <= TsCache_SE; ++i)
	  assert (neighIndex[i] < coords_len);
      }

      /* Update the uv_bbox as well. */
      TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] range %.0f-%.0f",
		 tile->id.x, tile->id.y, tile->id.res, 
		 min_z, max_z));
      aux->uv_bbox.min[2] = min_z;
      aux->uv_bbox.max[2] = max_z;
      TsBBoxFinish (&aux->uv_bbox);

      /* Redo the bbox. */
      TsBBoxFinish (bbox);
      /*TsCacheOI_PropagateBBox (imCache, tile);*/
    }

    /* No longer need this one. */
    free (aux->demCoords);
    aux->demCoords = 0;
    aux->size -= coords_len * sizeof (aux->demCoords[0]);
  }
}

/*
 * Compute the range of normals over an OI tile.  This is done by simply
 * walking through the normals in the underlying DEM tiles, and accumulating
 * an average normal.  We can subsequently determine the minimal cosine of an
 * individual normal against the average.  From this, a backfacing threshold
 * is determined.
 */

static void
TsGeoOIDEM_TileNormalRange (TsTile *tile)
{
  static char const funcName[] = "TsGeoOIDEM_TileNormalRange";
  TsOITileAux *aux = tile->aux;
  v3f *avg = &aux->geo.normal;
  TsCoord const *coords = aux->geo.coords, *end;
  TsCoord const *coord;
  int w, h;
  float min_v;

  w = aux->geo.coords_w;
  h = aux->geo.coords_h;
  end = coords + w*h;

  TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d]",
	     tile, tile->id.x, tile->id.y, tile->id.res));

  /* Compute the average normal */
  V(*avg)[0] = V(*avg)[1] = V(*avg)[2] = 0;
  for (coord = coords; coord < end; ++coord) {
    add_v3f ((v3f *)coord->norm, avg, avg);
  }
  normalize_v3f (avg);

  /* And find the angle by finding the minimal inner product of avg with
     any of the normals. */
  for (coord = coords, min_v = 1; coord < end; ++coord) {
    float v = inner_v3f (avg, (v3f *)coord->norm);
    if (v < min_v) min_v = v;
  }

  /* normalThreshold is the threshold below which v.n is considered to be
     backfaced. */
  if (min_v < 0)
    aux->geo.normalThreshold = -1;
  else
    aux->geo.normalThreshold = cos (acos (min_v) + M_PI/2);
  TSMTRACE ((CACHE_MASK|2, funcName, "Angle: %.1f  Thresh: %.2f (%.1f)",
	     acos (min_v) * 180/M_PI, aux->geo.normalThreshold,
	     (acos (min_v) + M_PI/2) * 180/M_PI));
}

/* Prepare an OI tile for display. This is the basic routine that is used to
   generate tile requests from the tsmApi.  If we ask to prepare a tile that
   is not yet resident and the request_prio pointer is NON-NULL, then we
   submit a request for this tile.

   First, we check to see if all of the supporting DEM tiles are resident. If
   so, then we check whether this tile is resident.  All requests to fulfill
   this are made if it is not so (and request_prio is non-NULL).

   If the DEM tiles and this one are all resident, then we prepare the tile
   for display by calculating the points, normals, and texture coordinates.
*/

#define DEM_REQUEST_INCR (100)

static int
TsGeoOIDEM_PrepareTile (TsGeoView *gv, TsTile *tile,
			int const *request_prio)
{
  static char const funcName[] = "TsGeoOIDEM_PrepareTile";
  TsGeoView_OIDEM *view = (TsGeoView_OIDEM *) gv;
  TsGeoPyramid *geo = gv->geo;
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;
  int all_res = TRUE;

  TsOITileAux *oiAux = (TsOITileAux *) tile->aux;
  TsTileRange const *demRef = &oiAux->demRef;
  TsTile **demTiles = demRef->tiles;
  int ndems = demRef->ntiles;
  int i;

  TSMTRACE ((CACHE_MASK|2, funcName, "%p[%d %d %d]",
	     tile, tile->id.x, tile->id.y, tile->id.res)); 
  TSMTRINC ((CACHE_MASK|2, 1));

  for (i = 0; i < ndems; ++i) {
    if (! TsCacheTilePrepared (demTiles[i])) {
      TSMTRACE((CACHE_MASK|2, funcName, 
		"DEM %p[%d %d %d] not available (status %d)",
		demTiles[i],
		demTiles[i]->id.x, demTiles[i]->id.y, demTiles[i]->id.res,
		demTiles[i]->status)); 
      all_res = FALSE;
      if (request_prio)
	TsCacheRequestTile (funcName, view->demReq, demTiles[i],
			    *request_prio + DEM_REQUEST_INCR);
    }
  }

  if (TsCacheTileResident (tile)) {
    if (all_res) {
      /* Initially we just estimate the bbox from the bboxes of the 
	 associated DEM tiles. */
      TsTile const *top;

      TSMTRACE ((CACHE_MASK|1, funcName, "preparing %p[%d %d %d]",
		 tile, tile->id.x, tile->id.y, tile->id.res)); 

      /* Check that we can do this transition ourselves. */
      if (! TsCacheTileStartTransition (tile)) {
	TSMTRACE ((CACHE_MASK|1, funcName,
		   "%p[%d %d %d] can't start transition",
		   tile, tile->id.x, tile->id.y, tile->id.res));
	TSMTRINC ((CACHE_MASK|2, -1));
	return FALSE;
      }

      /* Calculate tile's points and range of normals */
      TsGeoOIDEM_TilePoints (geov, tile);
      TsGeoOIDEM_TileNormalRange (tile);

      /* Indicate that this tile is now in the PREPARED state. */
      TsCacheTileFinishTransition (funcName, tile, TS_PREPARED);

      /* Now ensure the consistency of the pyramid. */
      top = TsCacheOI_PropagateBBox (geo->im, tile);
      if (top) {
	/* Update OI bbox if necessary */
	TsBBox const *top_bbox = &top->bbox;
	TsBBox const *lvcs_bbox = TsGeoBBoxLVCS (geo);

	assert (top_bbox->coordType == lvcs_bbox->coordType);
	if (top_bbox->min[2] < lvcs_bbox->min[2]
	    || top_bbox->max[2] > lvcs_bbox->max[2]) {
	  TSMTRACE ((CACHE_MASK|1, funcName, 
		     "WARNING: %p[%d %d %d] elev %g->%g > %g->%g",
		     top, top->id.x, top->id.y, top->id.res,
		     top_bbox->min[2], top_bbox->max[2],
		     lvcs_bbox->min[2], lvcs_bbox->max[2]));
	}
      }
  
      TSMTRINC ((CACHE_MASK|2, -1));
      return TRUE;
    } else {
      TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d %d %d] missing DEMs",
		 tile, tile->id.x, tile->id.y, tile->id.res));
    }

  } else {
    /* Request the tile */
    if (request_prio)
      TsCacheRequestTile (funcName, view->oiReq, tile, *request_prio);

    /* Improve the estimate of UV bbox */
    if (all_res && ! oiAux->demCoords) {
      TSMTRACE ((CACHE_MASK|1, funcName, "improve %p[%d %d %d]",
		 tile, tile->id.x, tile->id.y, tile->id.res)); 
      TsGeoOIDEM_TileElevs (geov, tile);
    }
  }

  TSMTRINC ((CACHE_MASK|2, -1));
  return FALSE;
}

/* Determine the maximal deviation from a straight line due to curvature of
   the planet *between* two GCC points. */

static float
TsCurvatureAdjustment (TileSetTspec const *tspec,
		       v3f const *c0, v3f const *c1)
{
  static char const funcName[] = "TsCurvatureAdjustment";
  v3f n0 = *c0;
  v3f n1 = *c1;

  /* Avoid overflow by doing this now */
  normalize_v3f (&n0);
  normalize_v3f (&n1);

  /* Calc angular extent of tile */
  {
    float inner = inner_v3f (&n0, &n1), theta;
    if (inner < -1) inner = -1; else if (inner > 1) inner = 1;
    theta = acos (inner);
    TSMTRACE ((CACHE_MASK|2, funcName, 
	       "Corners [%.0f,%.0f,%.0f - %.0f,%.0f,%.0f] theta %.1f",
	       V(*c0)[0], V(*c0)[1], V(*c0)[2],
	       V(*c1)[0], V(*c1)[1], V(*c1)[2], 
	       theta*180.0/M_PI));

    /* Displacement from flat from planet's curvature */
    return tspec->coordTspec.equatorialRadius * (1 - cos(theta/2.0));
  }
}

#if 0
int
TsGeoTileCheckNesting (TsGeoPyramid const *geo, 
		       TsTile *tile, TsTile const *parent)
{
  static char const funcName[] = "TsGeoTileCheckNesting";

  int changed = FALSE;
  TsBBox *bbox = &tile->bbox;
  TsOITileAux const *aux = tile->aux;
  TsBBox const *parent_bbox;
  int lvcs = TsCacheTilePrepared (tile);
  int parent_prepared;
  int i;

  /* If this tile has geometry, then it *must* be OK. */
  if (aux->geo.geometry)
    return FALSE;

  if (! parent) {
    parent = TsCacheTileParent_const (tile->cache, tile);
    if (! parent) return FALSE;
  }

  parent_prepared = TsCacheTilePrepared (parent);

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d]%s parent [%d %d %d]%s",
	     tile->id.x, tile->id.y, tile->id.res,
	     lvcs ? " prepared" : "",
	     parent->id.x, parent->id.y, parent->id.res,
	     parent_prepared ? " prepared" : ""));

  if (lvcs) {
    if (! parent_prepared) return FALSE;
    parent_bbox = &parent->bbox;
  } else {
    if (parent_prepared) {
      TsOITileAux const *parent_aux = parent->aux;
      parent_bbox = &parent_aux->uv_bbox;
    } else {
      parent_bbox = &parent->bbox;
    }
  }

  TSMTRACE ((CACHE_MASK|2, funcName, "tile   %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
	     bbox->min[0], bbox->min[1], bbox->min[2],
	     bbox->max[0], bbox->max[1], bbox->max[2]));
  TSMTRACE ((CACHE_MASK|2, funcName, "parent %.0f,%.0f,%.0f - %.0f,%.0f,%.0f",
	     parent_bbox->min[0], parent_bbox->min[1], parent_bbox->min[2],
	     parent_bbox->max[0], parent_bbox->max[1], parent_bbox->max[2]));
  
  for (i = 0; i < 3; ++i) {
    if (bbox->min[i] < parent_bbox->min[i]) {
      bbox->min[i] = parent_bbox->min[i];
      ++changed;
    }
    if (bbox->max[i] > parent_bbox->max[i]) {
      bbox->max[i] = parent_bbox->max[i];
      ++changed;
    }
  }

  if (changed)
    TsBBoxFinish (bbox);

  return TRUE;
}
#endif

/* Convert the bbox of a tile from UVZ to LVCS. */

void
TsGeoOIDEM_TileComputeBBox (TsGeoPyramid const *geo,
			    TsTile *tile, 
			    float min_z, float max_z)
{
  static char const funcName[] = "TsGeoOIDEM_TileComputeBBox";
  TsOITileAux *aux = tile->aux;
  TsBBox *bbox = &tile->bbox;
  TsBBox *uv_bbox;
  TileSetTspec const *tspec = TsCacheTspec (geo->im);
  int i, j;
  float const *uv[2];
  v3f gcc_corner[2];

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] Z [%.1f-%.1f]",
	     tile->id.x, tile->id.y, tile->id.res,
	     min_z, max_z));
  TSMTRINC ((CACHE_MASK|2, 1));

  assert (tile->aux != 0);

  /* Update uv_bbox */
  uv_bbox = &aux->uv_bbox;
  uv_bbox->min[2] = min_z;
  uv_bbox->max[2] = max_z;

  /* Make min/max indexible */
  uv[0] = uv_bbox->min;
  uv[1] = uv_bbox->max;

  TsBBoxInit (bbox);
  for (i = 0; i < 2; ++i) {
    float u = uv[i][0];
    for (j = 0; j < 2; ++j) {
      float v = uv[j][1];
      double g[3];
      float l[3];
      if (TsCacheUVtoLVCS (geo->im, u, v, min_z, l))
	TsBBoxAddPoint (bbox, l[0], l[1], l[2]);

      if (TsCacheUVtoGCC (geo->im, u, v, max_z, g)
	  && TsCacheGCCtoLVCS (geo->im, g[0], g[1], g[2], l))
	TsBBoxAddPoint (bbox, l[0], l[1], l[2]);

      /* grab diagonal max corners for extent calculation */
      if (i == 0 && j == 0)
	set_v3f (g[0], g[1], g[2], &gcc_corner[0]);
      else if (i == 1 && j  == 1)
	set_v3f (g[0], g[1], g[2], &gcc_corner[1]);
    }
  }

  /* Optionally adjust bboxes by planetary curvature. */
  {
    float delta = 
      TsCurvatureAdjustment (tspec, &gcc_corner[0], &gcc_corner[1]);
    
    /* We have to expand the bounding box by delta (to be conservative) */
    if (delta > uv_bbox->max[2] - uv_bbox->min[2]) {
      int i;
    
      TSMTRACE ((CACHE_MASK|2, funcName, "Curvature adjustment %.0fm", delta));
      for (i = 0; i < 3; i++) {
	bbox->min[i] -= delta;
	bbox->max[i] += delta;
      }
    }
  }

#if 0
  {
    TsTile const *parent = TsCacheTileParent_const (geo->im, tile);

    /* Now, ensure that it is nested inside its parent */
    if (parent && TsCacheTilePrepared (parent)) {
      TsBBox const *pbbox = &parent->bbox;
      int j;
      for (j = 0; j < 3; ++j) {
	if (bbox->min[j] < pbbox->min[j]) bbox->min[j] = pbbox->min[j];
	if (bbox->max[j] > pbbox->max[j]) bbox->max[j] = pbbox->max[j];
      }
    }
  }
#endif

  TsBBoxFinish (bbox);

  /* Inform the bbox that it's been converted. */
  bbox->coordType = TsCoordLVCS;

  TSMTRINC ((CACHE_MASK|2, -1));
}

static TsBBox const *
TsGeoOIDEM_TileUVBBox (TsTile const *tile)
{
  TsOITileAux const *aux = tile->aux;
  return (aux ? &aux->uv_bbox : &tile->bbox);
}

/* Compute an OI tile's auxiliary information after the tile is loaded.  This
   function is registered as a TILE_INIT callback of the OI TsCache.
*/

static void
TsGeoOIDEM_TileExtent (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "TsGeoOIDEM_TileExtent";
  TsGeoPyramid *geo = closure;
  TsTile *tile = arg;

  /* If the parent is prepared, then we grab our estimated bounds from it. */
  TsTile const *parent = TsCacheTileParent_const (tile);
  TsBBox const *uv_bbox = ((parent && TsCacheTilePrepared (parent))
			   ? TsGeoOIDEM_TileUVBBox (parent)
			   : 0);

  /* Otherwise, use the dataset's bounds. */
  if (! uv_bbox) uv_bbox = TsCacheBBox (geo->im);

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] Z [%g-%g]",
	     tile->id.x, tile->id.y, tile->id.res, 
	     uv_bbox->min[2], uv_bbox->max[2]));

  TsGeoOIDEM_MakeOIAux (geo, tile, uv_bbox->min[2], uv_bbox->max[2]);
}

/* Convert a bounding box from LVCS coordinates to the global GCC */

static void
TsGeoBBoxLVCStoGCC (TsGeoPyramid const *geo,
		    TsBBox const *lvcs_bbox, TsBBox *bbox)
{
  static char const funcName[] = "TsGeoBBoxLVCStoGCC";
  float const *corner[2];
  int i, j, k;

  /* Check type of LVCS bbox. */
  assert (lvcs_bbox->coordType == TsCoordLVCS);

  corner[0] = lvcs_bbox->min;
  corner[1] = lvcs_bbox->max;

  /* Project all corners and accumulate minmax values. */
  TsBBoxInit (bbox);
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 2; ++j) {
      for (k = 0; k < 2; ++k) {
	double g[3];
	TsGeoLVCStoGCC (geo, corner[i][0], corner[j][1], corner[k][2], g);
	TsBBoxAddPoint (bbox, g[0], g[1], g[2]);
      }
    }
  }
  TsBBoxFinish (bbox);

  /* Label the GCC bbox */
  bbox->coordType = TsCoordGCC;

  TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     bbox->min[0], bbox->min[1], bbox->min[2],
	     bbox->max[0], bbox->max[1], bbox->max[2]));
}

void
TsGeoUpdateBBox (TsGeoPyramid *geo)
{
  static char const funcName[] = "TsGeoUpdateBBox";
  TsBBox const *uv_bbox = TsCacheBBox (geo->im);
  TsBBox *lvcs_bbox = &geo->lvcs_bbox;
  TsBBox *gcc_bbox = &geo->bbox;
  TsBBox *gdc_bbox = &geo->gdc_bbox;
  TileSetTspec const *tspec = TsCacheTspec (geo->im);
  float const *corner[2];
  v3f gcc_corner[2];
  int i, j, k;

  TSMTRACE ((CACHE_MASK|1, funcName, "UV  : %.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     uv_bbox->min[0], uv_bbox->min[1], uv_bbox->min[2],
	     uv_bbox->max[0], uv_bbox->max[1], uv_bbox->max[2]));

  corner[0] = uv_bbox->min;
  corner[1] = uv_bbox->max;

  /* Project all corners and accumulate minmax values. */
  TsBBoxInit (lvcs_bbox);
  TsBBoxInit (gcc_bbox);
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 2; ++j) {
      for (k = 0; k < 2; ++k) {
	double g[3];
	float l[3];
	TsGeoUVtoGCC (geo, corner[i][0], corner[j][1], corner[k][2], g);
	TsBBoxAddPoint (gcc_bbox, g[0], g[1], g[2]);

	/* grab diagonal max corners for extent calculation */
	if (i == 0 && j == 0 && k == 1)
	  set_v3f (g[0], g[1], g[2], &gcc_corner[0]);
	else if (i == 1 && j  == 1 && k == 1)
	  set_v3f (g[0], g[1], g[2], &gcc_corner[1]);

	TsGeoGCCtoLVCS (geo, g[0], g[1], g[2], l);
	TsBBoxAddPoint (lvcs_bbox, l[0], l[1], l[2]);
      }
    }
  }

  /* Optionally adjust bboxes by planetary curvature. */
  {
    float delta = 
      TsCurvatureAdjustment (tspec, &gcc_corner[0], &gcc_corner[1]);
    
    if (delta > uv_bbox->max[2] - uv_bbox->min[2]) {
      /* We have to outset both bounding boxes by delta (to be conservative) */
      int i;
    
      TSMTRACE ((CACHE_MASK|2, funcName, "Curvature adjustment %.0f", delta));
      for (i = 0; i < 3; i++) {
	lvcs_bbox->min[i] -= delta;
	lvcs_bbox->max[i] += delta;
	gcc_bbox->min[i] -= delta;
	gcc_bbox->max[i] += delta;
      }
    }
  }
    
  TsBBoxFinish (lvcs_bbox);
  TsBBoxFinish (gcc_bbox);

  /* now regenerate the GDC bounding box based upon the new GCC values */
  /* we do this here rather than integrated above so that we can       */
  /* accurately include the modifications for the earth's curvature.   */

  TsBBoxInit (gdc_bbox);

  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 2; ++j) {
      for (k = 0; k < 2; ++k) {
	double gdc[3];
	if ( TsCacheUVtoGDC (geo->im, corner[i][0], corner[j][1],
			     corner[k][2], gdc))
	  TsBBoxAddPoint (gdc_bbox, gdc[0], gdc[1], gdc[2]);
      }
    }
  }

  TsBBoxFinish (gdc_bbox);

  TSMTRACE ((CACHE_MASK|1, funcName, "LVCS: %.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     lvcs_bbox->min[0], lvcs_bbox->min[1], lvcs_bbox->min[2],
	     lvcs_bbox->max[0], lvcs_bbox->max[1], lvcs_bbox->max[2]));
  TSMTRACE ((CACHE_MASK|1, funcName, "GCC : %.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     gcc_bbox->min[0], gcc_bbox->min[1], gcc_bbox->min[2],
	     gcc_bbox->max[0], gcc_bbox->max[1], gcc_bbox->max[2]));
  TSMTRACE ((CACHE_MASK|1, funcName, "GDC : %.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	     gdc_bbox->min[0], gdc_bbox->min[1], gdc_bbox->min[2],
	     gdc_bbox->max[0], gdc_bbox->max[1], gdc_bbox->max[2]));
}

/* Set the initial value of the pyramid's bounding box.  This is only called
   at initialization. */

void
TsGeoOIDEM_SetBBox (TsGeoPyramid *geo)
{
  /*static char const funcName[] = "TsGeoOIDEM_SetBBox";*/
  TsGeoPyramid_OIDEM *geov = (TsGeoPyramid_OIDEM *) geo;
  TileSetTspec const *demTspec = TsCacheTspec (geov->dem);
  TsTile *demTop = TsCacheGetTile (geov->dem, 0, 0, demTspec->maxLevel);

  /* Fix pyramids to ensure footprint of DEM > OI */
  TsGeoOIDEM_SetBBoxPyramids (geov);

  /* In order to get elevation range, we need the DEM tile.
     This has already been requested by TsCacheDEM_InitLoad. */
  if (! TsCacheTilePrepared (demTop))
    TsCacheTileWait (geov->dem, demTop);

  {
    float const *min = demTop->bbox.min, *max = demTop->bbox.max;
    TsBBox *uv_bbox = (TsBBox *) TsCacheBBox (geo->im);

    /* Propagate these limits into the imagery bbox (no previous elevation) */
    uv_bbox->min[2] = min[2];
    uv_bbox->max[2] = max[2];
    TsBBoxFinish (uv_bbox);

    TsGeoUpdateBBox (geo);
  }
}

static void
organizeMinMax (float u[4])
{
  /* Place min in u[0] and max in u[1] */
  if (u[0] > u[1]) {
    float t = u[0]; u[0] = u[1]; u[1] = t;
  }
  if (u[2] < u[0]) u[0] = u[2];
  else if (u[2] > u[1]) u[1] = u[2];
  if (u[3] < u[0]) u[0] = u[3];
  else if (u[3] > u[1]) u[1] = u[3];
}

/* Ensure that we constrain the OI pyramid's extent so that it fits entirely
   inside the extent of the DEM pyramid.  In addition, ensures that the
   extents of all levels if the OI pyramid are properly nested.  This is to
   protect against certain bugs in the OI pyramid generation processs from
   propagating through the system.
*/

static void
TsGeoOIDEM_SetBBoxPyramids (TsGeoPyramid_OIDEM *geov)
{
  static char const funcName[] = "TsGeoOIDEM_SetBBoxPyramids";
  TsGeoPyramid *geo = &geov->geo;

  TsCache *imCache = geo->im;
  TileSetTspec const *imTspec = TsCacheTspec (imCache);
  int res = imTspec->minLevel;
  SimpleTileSet *imTset = (SimpleTileSet *) TsCacheTileSet (imCache, res);
  int min_ix = TsCacheMinPixelX (imCache, res);
  int max_ix = TsCacheMaxPixelX (imCache, res);
  int min_iy = TsCacheMinPixelY (imCache, res);
  int max_iy = TsCacheMaxPixelY (imCache, res);

  TsCache *demCache = geov->dem;
  TileSetTspec const *demTspec = TsCacheTspec (demCache);

  float u[4], v[4];
  float dem_u[4], dem_v[4];

  rangeTfmUV (min_ix, min_iy, max_ix+1, max_iy+1,
	      TsCacheTfm_ixiytouv (geo->im, imTspec->minLevel),
	      u, v);

  TsGeoOIDEM_OItoDEM (geov, u[0], v[0], &dem_u[0], &dem_v[0]);
  TsGeoOIDEM_OItoDEM (geov, u[0], v[1], &dem_u[1], &dem_v[1]);
  TsGeoOIDEM_OItoDEM (geov, u[1], v[0], &dem_u[2], &dem_v[2]);
  TsGeoOIDEM_OItoDEM (geov, u[1], v[1], &dem_u[3], &dem_v[3]);

  /* Find min/max of DEM u,v pairs */
  organizeMinMax (dem_u);
  organizeMinMax (dem_v);

  if (clampUV (demCache, demTspec->minLevel, &dem_u[0], &dem_v[0])  
      | clampUV (demCache, demTspec->minLevel, &dem_u[0], &dem_v[1])
      | clampUV (demCache, demTspec->minLevel, &dem_u[1], &dem_v[0])
      | clampUV (demCache, demTspec->minLevel, &dem_u[1], &dem_v[1])) {
    TSMTRACE ((CACHE_MASK|2, funcName, "%.0f,%.0f -> %.0f,%.0f",
	       dem_u[0], dem_v[0], dem_u[1], dem_v[1]));

    TsGeoOIDEM_DEMtoOI (geov, dem_u[0], dem_v[0], &u[0], &v[0]);
    TsGeoOIDEM_DEMtoOI (geov, dem_u[0], dem_v[1], &u[1], &v[1]);
    TsGeoOIDEM_DEMtoOI (geov, dem_u[1], dem_v[0], &u[2], &v[2]);
    TsGeoOIDEM_DEMtoOI (geov, dem_u[1], dem_v[1], &u[3], &v[3]);
    
    organizeMinMax (u);
    organizeMinMax (v);

    TsCacheSetExtent (imCache, u[0], v[0], u[1], v[1]);
  }

  /* Set bbox based as determined above. */ 
  {
    TsBBox *bbox = &geo->bbox;
    double const *origin = TsCacheOrigin (geo->im);
    double g[3];
    int i, j;
    TSMTRACE ((CACHE_MASK|1, funcName, "UV origin %.0f,%.0f [%g,%g -> %g,%g]",
	       origin[0], origin[1], u[0],v[0], u[1],v[1]));
    TSMTRINC ((CACHE_MASK|1, 1));
    TsBBoxInit (bbox);
    for (i = 0; i < 2; ++i) {
      for (j = 0; j < 2; ++j) {
	TsGeoUVtoGCC (geo, u[i], v[j], 0.0, g);
	TsBBoxAddPoint (bbox, g[0], g[1], g[2]);
      }
    }
    TSMTRACE ((CACHE_MASK|1, funcName, "%.0f,%.0f,%.0f -> %.0f,%.0f,%.0f",
	       bbox->min[0], bbox->min[1], bbox->min[2],
	       bbox->max[0], bbox->max[1], bbox->max[2]));
    TsBBoxFinish (bbox);
    TSMTRINC ((CACHE_MASK|1, -1));
  }
}

/* Load a small subset of the DEMs underlying a dataset in order to provide
   adequate estimate of bounding boxes. */
static void
TsCacheDEM_InitLoad (TsCache *cache, tsmRequestList *req)
{
  static char const funcName[] = "TsCacheDEM_InitLoad";
  TileSetTspec const *demTspec = TsCacheTspec (cache);
  int level = demTspec->maxLevel;
  int min_level = demTspec->minLevel;
  int ntiles = 0;

  /* We walk down until the number of tiles requested is >= 16.
     We do this now, because we're now enforced a rule whereby OI tiles
     front-load the DEM dependency.  It should be a considerable advantage to
     have a deeper set of initial DEM data available. */
  for ( ; level >= min_level && ntiles <= 16; --level) {
    SimpleTileSet const *tset = TsCacheTileSet (cache, level);
    int x, y;

    TSMTRACE ((CACHE_MASK|2, funcName, "Level %d: %d-%d,%d-%d",
	       level, tset->minTile[0], tset->maxTile[0],
	       tset->minTile[1], tset->maxTile[1]));
    for (x = tset->minTile[0]; x <= tset->maxTile[0]; ++x)
      for (y = tset->minTile[1]; y <= tset->maxTile[1]; ++y, ++ntiles)
	TsCacheRequest (cache, req, x, y, level, TSM_NOW);
  }
  tsmRequestSubmit (req);
}

/* Load a small subset of the OI tiles down to the level where >1 tile
   exists. */
static void
TsCacheOI_InitLoad (TsCache *cache, tsmRequestList *req)
{
  static char const funcName[] = "TsCacheOI_InitLoad";
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int level = tspec->maxLevel;
  int min_level = level;

  /* Find level with >1 tile */
  for ( ; ; --min_level) {
    SimpleTileSet const *tset = TsCacheTileSet (cache, min_level);
    if (tset->minTile[0] != tset->maxTile[0]
	|| tset->minTile[1] != tset->maxTile[1])
      break;
  }

  for ( ; level >= min_level; --level) {
    SimpleTileSet const *tset = TsCacheTileSet (cache, level);
    int x, y;

    TSMTRACE ((CACHE_MASK|2, funcName, "Level %d: %d-%d,%d-%d",
	       level, tset->minTile[0], tset->maxTile[0],
	       tset->minTile[1], tset->maxTile[1]));
    for (x = tset->minTile[0]; x <= tset->maxTile[0]; ++x)
      for (y = tset->minTile[1]; y <= tset->maxTile[1]; ++y)
	TsCacheRequest (cache, req, x, y, level, TSM_NOW);
  }
  tsmRequestSubmit (req);
}


static void remapBytes (TsCache const *cache, void *arg, void *closure);
static void remapShorts (TsCache const *cache, void *arg, void *closure);
static void remapInts (TsCache const *cache, void *arg, void *closure);

void
TsCacheOI_SetupRemap (TsCache *cache)
{
  static char const  funcName[] = "TsCacheOI_SetupRemap";
  TsmConnection      connect = TsCacheConnection( cache );
  TileSetTspec const *tspec = TsCacheTspec (cache);
  int                ncomps = tspec->tileTspec.numComponentsPixel;

  /* DETERMINE COMPONENT REORDER IF NECESSARY. */
  if (ncomps == 3) {
    tspSymbol const *comps = tspec->pixelTspec.componentNames.val;
    int *remap = alloca (ncomps * sizeof (int));
    int i, do_remap = 0;

    for (i = 0; i < ncomps; ++i) {
      remap[i] = comps[i] - tspRed;
      if (i != remap[i])
	do_remap = 1;
    }

    if (do_remap) {
      static TsCallbackF remapFuncs[] = {
	0, remapBytes, remapShorts, 0, remapInts
      };
      TsCallbackF remapFunc;
      void **remapClosure;

      int swap_len = 2 * ncomps * sizeof (int);
      int *swaps = alloca (swap_len);
      int nswaps = 0;

      /* Do shuffle by swaps. */
      for (i = 0; i < ncomps; ++i) {
	if (remap[i] != i) {
	  swaps[nswaps++] = i;
	  swaps[nswaps++] = remap[i];
	  remap[remap[i]] = remap[i];
	}
      }

      TSMTRACE ((CACHE_MASK|2, funcName, "Remap tiles: comps %dx%d swaps %d",
		 ncomps, tspec->tileTspec.numBytesPerComponent, nswaps));

      remapFunc = remapFuncs[tspec->tileTspec.numBytesPerComponent];
      remapClosure = malloc (4 * sizeof (void *) + swap_len);
      remapClosure[0] = &remapClosure[4];
      remapClosure[1] = (void*) nswaps;
      remapClosure[2] = (void*) ncomps;
      remapClosure[3] = (void*) tsmGet( connect, TSM_TILE_SIZE );
      memcpy (remapClosure[0], swaps, swap_len);

      TsCacheAddCallback (cache, TILE_CACHE, remapFunc, remapClosure, free);
    }
  }
}

static void
remapBytes (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "remapBytes";

  TsCacheTileCB *cb = arg;
  TsTile *tile = cb->tile;
  uchar *data = (uchar *) tile->data;

  void **args = closure;
  int *swaps = args[0];
  int nswaps = (int) args[1];
  int ncomps = (int) args[2];
  int len    = (int) args[3];

  uchar *data_end = data + len;
  int i;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] comps %d swaps %d",
	     tile->id.x, tile->id.y, tile->id.res, ncomps, nswaps));

  if (nswaps == 2) {
    int fr = swaps[0], to = swaps[1];
    for ( ; data < data_end; data += ncomps) {
      uchar v = data[fr];
      data[fr] = data[to];
      data[to] = v;
    }
  } else if (nswaps == 4) {
    int fr0 = swaps[0], to0 = swaps[1];
    int fr1 = swaps[2], to1 = swaps[3];
    for ( ; data < data_end; data += ncomps) {
      uchar v = data[fr0];
      data[fr0] = data[to0];
      data[to0] = v;
      v = data[fr1];
      data[fr1] = data[to1];
      data[to1] = v;
    }
  } else {
    for ( ; data < data_end; data += ncomps) {
      for (i = 0; i < nswaps; i += 2) {
	int from = swaps[i];
	int to   = swaps[i+1];
	uchar v = data[from];
	data[from] = data[to];
	data[to] = v;
      }
    }
  }
}

static void
remapShorts (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "remapShorts";

  TsCacheTileCB *cb = arg;
  TsTile *tile = cb->tile;
  short *data = (short *) tile->data;

  void **args = closure;
  int *swaps = args[0];
  int nswaps = (int) args[1];
  int ncomps = (int) args[2];
  int len    = (int) args[3];

  short *data_end = data + len;
  int i;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] comps %d swaps %d",
	     tile->id.x, tile->id.y, tile->id.res, ncomps, nswaps));

  if (nswaps == 2) {
    int fr = swaps[0], to = swaps[1];
    for ( ; data < data_end; data += ncomps) {
      short v = data[fr];
      data[fr] = data[to];
      data[to] = v;
    }
  } else if (nswaps == 4) {
    int fr0 = swaps[0], to0 = swaps[1];
    int fr1 = swaps[2], to1 = swaps[3];
    for ( ; data < data_end; data += ncomps) {
      short v = data[fr0];
      data[fr0] = data[to0];
      data[to0] = v;
      v = data[fr1];
      data[fr1] = data[to1];
      data[to1] = v;
    }
  } else {
    for ( ; data < data_end; data += ncomps) {
      for (i = 0; i < nswaps; i += 2) {
	int from = swaps[i];
	int to   = swaps[i+1];
	short v = data[from];
	data[from] = data[to];
	data[to] = v;
      }
    }
  }
}

static void
remapInts (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "remapInts";

  TsCacheTileCB *cb = arg;
  TsTile *tile = cb->tile;
  int *data  = (int *) tile->data;

  void **args = closure;
  int *swaps = args[0];
  int nswaps = (int) args[1];
  int ncomps = (int) args[2];
  int len    = (int) args[3];

  int *data_end = data + len;
  int i;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] comps %d swaps %d",
	     tile->id.x, tile->id.y, tile->id.res, ncomps, nswaps));

  if (nswaps == 2) {
    int fr = swaps[0], to = swaps[1];
    for ( ; data < data_end; data += ncomps) {
      int v = data[fr];
      data[fr] = data[to];
      data[to] = v;
    }
  } else if (nswaps == 4) {
    int fr0 = swaps[0], to0 = swaps[1];
    int fr1 = swaps[2], to1 = swaps[3];
    for ( ; data < data_end; data += ncomps) {
      int v = data[fr0];
      data[fr0] = data[to0];
      data[to0] = v;
      v = data[fr1];
      data[fr1] = data[to1];
      data[to1] = v;
    }
  } else {
    for ( ; data < data_end; data += ncomps) {
      for (i = 0; i < nswaps; i += 2) {
	int from = swaps[i];
	int to   = swaps[i+1];
	int v = data[from];
	data[from] = data[to];
	data[to] = v;
      }
    }
  }
}


typedef void (*demConvert_t) (float **v, void const *raw, int span,
			      int x0, int y0, int x1, int y1,
			      float offset, float scale);

#define DEMCONVERT(name,type)						  \
static void								  \
name (float **v0, void const *raw, int span,				  \
      int x0, int y0, int x1, int y1,					  \
      float offset, float scale)					  \
{									  \
  static char const funcName[] = #name;					  \
  type const *p = raw, *pend = p+span*(y1+1);				  \
  TSMTRACE ((CACHE_MASK|2, funcName, "[%d,%d - %d,%d]", x0, y0, x1, y1)); \
  for (p += y0*span, v0 += y0; p < pend; p += span, ++v0) {		  \
    type const *p0 = p+x0, *p0end = p+x1+1;				  \
    float *v = &v0[0][x0];						  \
    for ( ; p0 < p0end; ++p0, ++v) {					  \
      v[0] = p0[0] * scale + offset;					  \
      TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p elev %d => %.0f",	  \
		 p0-p, (p-(type const *)raw)/span, p0, p0[0], v[0]));	  \
    }									  \
  }									  \
}

DEMCONVERT(demConvert8,  PRInt8)
DEMCONVERT(demConvert16, PRInt16)
DEMCONVERT(demConvert32, PRInt32)
DEMCONVERT(demConvertu8,  PRUint8)
DEMCONVERT(demConvertu16, PRUint16)
DEMCONVERT(demConvertu32, PRUint32)

static int
demConvert (float **vals, 
	    SimpleTileSet const *tset, void const *raw, 
	    int x0, int y0, int x1, int y1)
{
  static char const funcName[] = "demConvert";
  float scale = tset->pixelTspec.scale;
  float offset = tset->pixelTspec.offset;
  int nbytes = tset->tileTspec.numBytesPerComponent;
  demConvert_t demFunc = 0;

  TSMTRACE ((CACHE_MASK|2, funcName, "ncomps %d signed %d nbytes %d",
	     tset->tileTspec.numComponentsPixel,
	     tset->tileTspec.componentNumberType == tspSignedByte,
	     nbytes));

  if (tset->tileTspec.numComponentsPixel != 1)
    return FALSE;
  
  if (tset->tileTspec.componentNumberType == tspSignedByte) {
    demFunc = (nbytes == 1 ? demConvert8 :
	       nbytes == 2 ? demConvert16 :
	       nbytes == 4 ? demConvert32 : 0);
  } else {
    demFunc = (nbytes == 1 ? demConvertu8 :
	       nbytes == 2 ? demConvertu16 :
	       nbytes == 4 ? demConvertu32 : 0);
  }
  
  if (! demFunc)
    return FALSE;

  demFunc (vals, raw, tset->tileTspec.totalPixels[0],
	   x0, y0, x1, y1,
	   offset, scale);

  return TRUE;
}

int
TsCacheDEM_boundsNeeded (TsCache const *cache, TsTile const *tile,
			 int *x0, int *x1, int *y0, int *y1)
{
  static char const funcName[] = "TsCacheDEM_boundsNeeded";
  TsBBox const *bbox = TsCacheBBox (cache);
  int tx = tile->id.x;
  int ty = tile->id.y;
  int res = tile->id.res;
  SimpleTileSet const *tset = TsCacheTileSet (cache, res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, res);
  float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
  int tile_ix = tset->tileTspec.numPixelsX * tx;
  int tile_iy = tset->tileTspec.numPixelsY * ty;
  int x0p = (tx == tset->minTile[0]);
  int x1p = (tx == tset->maxTile[0]);
  int y0p = (ty == tset->minTile[1]);
  int y1p = (ty == tset->maxTile[1]);
  float u[2], v[2];
  int ret = FALSE;
  int ix0, ix1, iy0, iy1;

  if (!x0p && !y0p && !x1p && !y1p)
    return FALSE;

  ix0 = *x0 + tile_ix; ix1 = *x1 + tile_ix;
  iy0 = *y0 + tile_iy; iy1 = *y1 + tile_iy;
  rangeTfmUV (ix0, iy0, ix1+1, iy1+1, ixiytouv, u, v);

  TSMTRACE ((CACHE_MASK|2, funcName, "Tile [%.0f,%.0f - %.0f,%.0f]",
	     u[0], v[0], u[1], v[1]));
  TSMTRACE ((CACHE_MASK|2, funcName, "-vs- [%.0f,%.0f - %.0f,%.0f]",
	     bbox->min[0], bbox->min[1], bbox->max[0], bbox->max[1]));
  
  if (x0p) 
    TSMTRACE ((CACHE_MASK|2, funcName, "x0: %.0f < %.0f?",
	       u[0], bbox->min[0]));
  if (x0p && u[0] > bbox->min[0]) {
    float nx0 = TFM_IX (bbox->min[0], v[0], uvtoixiy);
    float nx1 = TFM_IX (bbox->min[0], v[1], uvtoixiy);
    if (nx1 < nx0) nx0 = nx1;
    if (nx0 < ix0) {
      *x0 = floor (nx0) - tile_ix;
      TSMTRACE ((CACHE_MASK|1, funcName, "Expand x0 => %d", nx0));
    }
  }

  if (x1p) 
    TSMTRACE ((CACHE_MASK|2, funcName, "x1: %.0f > %.0f?",
	       u[1], bbox->max[0]));
  if (x1p && u[1] < bbox->max[0]) {
    float nx0 = TFM_IX (bbox->max[0], v[0], uvtoixiy);
    float nx1 = TFM_IX (bbox->max[0], v[1], uvtoixiy);
    if (nx1 > nx0) nx0 = nx1;
    if (nx0 > ix1) {
      *x1 = ceil (nx0) - tile_ix;
      TSMTRACE ((CACHE_MASK|1, funcName, "Expand x1 => %d", nx0));
    }
  }

  if (y0p) 
    TSMTRACE ((CACHE_MASK|2, funcName, "y0: %.0f < %.0f?",
	       v[0], bbox->min[1]));
  if (y0p && v[0] > bbox->min[1]) {
    float ny0 = TFM_IY (v[0], bbox->min[1], uvtoixiy);
    float ny1 = TFM_IY (v[1], bbox->min[1], uvtoixiy);
    if (ny1 < ny0) ny0 = ny1;
    if (ny0 < iy0) {
      *y0 = floor (ny0) - tile_iy;
      TSMTRACE ((CACHE_MASK|1, funcName, "Expand y0 => %d", ny0));
    }
  }

  if (y1p) 
    TSMTRACE ((CACHE_MASK|2, funcName, "y1: %.0f > %.0f?",
	       v[1], bbox->max[1]));
  if (y1p && v[1] < bbox->max[1]) {
    float ny0 = TFM_IY (v[0], bbox->max[1], uvtoixiy);
    float ny1 = TFM_IY (v[1], bbox->max[1], uvtoixiy);
    if (ny1 > ny0) ny0 = ny1;
    if (ny0 > iy1) {
      *y1 = ceil (ny0) - tile_iy;
      TSMTRACE ((CACHE_MASK|1, funcName, "Expand y1 => %d", ny0));
    }
  }

  return ret;
}

static void
TsCacheDEM_makeAux (TsCache const *cache, TsTile *tile)
{
  static const char funcName[] = "TsCacheDEM_makeAux";
  static TsDEMTileAux invalidAux = {
    sizeof (TsDEMTileAux),
    0, -1, 0, -1,
    0, 0 };

  SimpleTileSet const *tset = TsCacheTileSet (cache, tile->id.res);
  int span = tset->tileTspec.totalPixels[0];
  int x0 = 0;
  int x1 = span-1;
  int num_x;
  int y0 = 0;
  int y1 = tset->tileTspec.totalPixels[1]-1;
  int num_y;
  int tile_ix = tile->id.x * tset->tileTspec.numPixelsX;
  int tile_iy = tile->id.y * tset->tileTspec.numPixelsY;

  int min_ix = TsCacheMinPixelX (cache, tile->id.res) - tile_ix;
  int max_ix = TsCacheMaxPixelX (cache, tile->id.res) - tile_ix;
  int min_iy = TsCacheMinPixelY (cache, tile->id.res) - tile_iy;
  int max_iy = TsCacheMaxPixelY (cache, tile->id.res) - tile_iy;
  float *elevArray;

#ifdef POINTCACHE
  v3f *lvcs_pts;
  v3f *lvcs_norms;
#else
  v3f **lvcs_pts;
  v3f **lvcs_norms;
#endif
  PRUint32 aux_size;
  TsDEMTileAux *aux;
  int i;

  if (min_ix > x0) x0 = min_ix;
  if (max_ix < x1) x1 = max_ix;
  if (min_iy > y0) y0 = min_iy;
  if (max_iy < y1) y1 = max_iy;

  if (x1 < x0 || y1 < y0) {
    tile->aux = &invalidAux;
    return;
  }

  {
    int x0p = x0, x1p = x1, y0p = y0, y1p = y1;
    TsCacheDEM_boundsNeeded (cache, tile, &x0p, &x1p, &y0p, &y1p);
  }

  num_x = x1 - x0 + 1; if (num_x < 0) num_x = 0;
  num_y = y1 - y0 + 1; if (num_y < 0) num_y = 0;

  aux_size = 
    (sizeof (*aux)
     + num_x * num_y * sizeof (float)
     + num_y * sizeof (float *)
#ifdef POINTCACHE
     + 2 * (num_y * sizeof (v3f *) + (num_x*num_y) * sizeof (v3f))
#endif
      );

  aux = malloc (sizeof (*aux));

  aux->size  = aux_size;
  aux->min_x = x0;
  aux->max_x = x1;
  aux->min_y = y0;
  aux->max_y = y1;

  aux->elevArray = elevArray = malloc (num_x * num_y * sizeof (float));

  aux->elevs = malloc (num_y * sizeof (float *));

#ifdef POINTCACHE
  aux->lvcs_pts   = (v3f **) malloc (num_y * sizeof (v3f *)
				     + (num_x*num_y) * sizeof (v3f));
  lvcs_pts   = (v3f *) &aux->lvcs_pts[num_y];

  aux->lvcs_norms = (v3f **) malloc (num_y * sizeof (v3f *)
				     + (num_x*num_y) * sizeof (v3f));
  lvcs_norms = (v3f *) &aux->lvcs_norms[num_y];
#else
  {
    int size = (num_y * sizeof (v3f **)
		+ (num_x*num_y) * sizeof (v3f*)
		+ (num_x*num_y) * sizeof (v3f));

    aux->lvcs_pt0   = (v3f *) calloc (1, size);
    aux->lvcs_pts   = (v3f ***) (aux->lvcs_pt0 + num_x*num_y);
    lvcs_pts        = (v3f **) &aux->lvcs_pts[num_y];
    aux->next_pt    = aux->lvcs_pt0;

    aux->lvcs_norm0 = (v3f *) calloc (1, size);
    aux->lvcs_norms = (v3f ***) (aux->lvcs_norm0 + num_x*num_y);
    lvcs_norms      = (v3f **) &aux->lvcs_norms[num_y];
    aux->next_norm  = aux->lvcs_norm0;
    aux->size += size;
  }
#endif

  /* Adjust indices so that they offset against iy */
  aux->elevs      -= y0;
  aux->lvcs_pts   -= y0;
  aux->lvcs_norms -= y0;

  /* Initialize indexing arrays */
  for (i = y0; i <= y1; ++i) {
    aux->elevs     [i] = &elevArray [num_x*(i-y0)] - x0;
    aux->lvcs_pts  [i] = &lvcs_pts  [num_x*(i-y0)] - x0;
    aux->lvcs_norms[i] = &lvcs_norms[num_x*(i-y0)] - x0;
  }

  /* Create the float array from tile data */
  demConvert (aux->elevs, tset, tile->data, x0, y0, x1, y1);

  /* No longer need this */
  tsmFreeTileBuffer ((uchar *) tile->data);
  tile->data = 0;

  TSMTRACE ((CACHE_MASK|2, funcName, "[%d %d %d] pixels %d-%d,%d-%d",
	     tile->id.x, tile->id.y, tile->id.res,
	     x0, x1, y0, y1));

  tile->aux = aux;
  TsCacheTileAddMem (tile, aux->size);
  TSMTRACE ((CACHE_MASK|3, funcName, "[%d %d %d] aux %p[%d]",
	     tile->id.x, tile->id.y, tile->id.res,
	     tile->aux, aux_size));
}

static void
TsCacheDEM_freeAux (TsCache const *cache, void *arg, void *closure)
{
  /*static char const funcName[] = "TsCacheDEM_freeAux";*/
  TsTile *tile = arg;
  TsDEMTileAux *aux = tile->aux;
  if (aux && aux->elevs) {
    TsCacheTileAddMem (tile, -(int)aux->size);
    free (aux->elevArray);
    free (aux->elevs);
#ifdef POINTCACHE
    free (aux->lvcs_pts);
    free (aux->lvcs_norms);
#else
    free (aux->lvcs_pt0);
    free (aux->lvcs_norm0);
#endif
    free (aux);
    tile->aux = 0;
  }
}

/*
 * For DEM tilesets, we may need to extend the values in the maximal boundary
 * tiles so that the DEM actually covers the full range of UV.  This is caused
 * by inappropriate truncation of tile size when generating a Pyramid.
 *
 * We "solve" the problem by extrapolating the last two values to generate
 * another row of pixels.
 *
 * This implementation assumes that ixiytouv is a simple positive scale
 * transformation.  What we should really do is calculate the
 * byRes[res].max_pxl values beforehand and just extend by pixels heierin.
 */

static void
TsCacheDEM_EnsureCoverage (TsCache const *cache, TsTile *tile)
{
  static char const funcName[] = "TsCacheDEM_EnsureCoverage";
  SimpleTileSet *tset = (SimpleTileSet *)
    TsCacheTileSet (cache, tile->id.res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, tile->id.res);
  TsBBox const *uv_bbox = TsCacheBBox (cache);
  float max_u = uv_bbox->max[0];
  float max_v = uv_bbox->max[1];
  int tile_w = tset->tileTspec.numPixelsX;
  int tile_h = tset->tileTspec.numPixelsY;
  int span = tset->tileTspec.totalPixels[0];

  /* This should be a direct reference to tset here, since we are concerned
     with what data came in from the server vs. what we need. */
  int max_ix = tset->maxValidPixel[0];
  int max_iy = tset->maxValidPixel[1];
  int tile_ix = tile->id.x * tile_w;
  int tile_iy = tile->id.y * tile_h;

  /* This is only necessary for edge tiles */
  if (tile_ix <= max_ix && tile_ix + tile_w > max_ix) {
    int ix = max_ix;
    float u = TFM_U (ix, tile_iy, ixiytouv);
    float v = TFM_V (ix, tile_iy, ixiytouv);
    TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] %d,%d U %.0f < %.0f %s",
	       tile->id.x, tile->id.y, tile->id.res,
	       ix, tile_iy,
	       u, max_u, u < max_u ? "TRUE" : "FALSE"));
    if (u < max_u) {
      int x0 = max_ix - tile_ix - 1;
      if (x0 < 0) 
	TSMTRACE ((CACHE_MASK|1, funcName, "X %d max %d => x0 %d",
		   tile_ix, max_ix, x0));
      if (x0 >= 0) {
	int y, x2;
	int iy1 = tile->pixels[1];
	PRUint32 *data = (void *) tile->data;

	for (x2 = max_ix-tile_ix+1; x2 < tile_w; ++x2) {
	  PRUint32 const *x0p = &data[x0];
	  PRUint32 *x2p = &data[x2];
	  /* We need to do the extension */
	  for (y = 0; y < iy1; ++y, x0p += span, x2p += span) {
	    PRUint32 v0 = x0p[0];
	    PRUint32 v1 = x0p[1];
	    PRUint32 v2 = v0 + (v1-v0)*(x2-x0);
	    x2p[0] = v2;
	    TSMTRACE ((CACHE_MASK|3, funcName,
		       "data[%d,%d] = [%d]%d [%d]%d => [%d]%d",
		       x2, y, x0,v0, x0+1,v1, x2,v2));
	  }
	  u = TFM_U (tile_ix+x2, tile_iy, ixiytouv);
	  TSMTRACE ((CACHE_MASK|3, funcName, "%d,%d => %.0f (> %.0f %s)",
		     tile_ix+x2, tile_iy, u, max_u,
		     u > max_u ? "TRUE" : "FALSE"));
	  if (u > max_u)
	    break;
	}
	if (x2 == tile_w) --x2;

	cache->byRes[tile->id.res].max_pxl[0] = tile_ix+x2;
	TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d] extend to %dx%d",
		   cache, tile->id.res,
		   cache->byRes[tile->id.res].max_pxl[0],
		   cache->byRes[tile->id.res].max_pxl[1]));
	
	tile->pixels[0] = x2+1;
	TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] extend to %dx%d",
		   tile->id.x, tile->id.y, tile->id.res,
		   tile->pixels[0], tile->pixels[1]));
      }
    }
  }
  if (tile_iy <= max_iy && tile_iy + tile_h > max_iy) {
    int iy = max_iy;
    float u = TFM_U (tile_ix, iy, ixiytouv);
    float v = TFM_V (tile_ix, iy, ixiytouv);
    TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] %d,%d U %.0f < %.0f %s",
	       tile->id.x, tile->id.y, tile->id.res,
	       tile_ix, iy,
	       v, max_v, v < max_v ? "TRUE" : "FALSE"));
    if (v < max_v) {
      int y0 = max_iy - tile_iy - 1;
      if (y0 < 0) 
	TSMTRACE ((CACHE_MASK|1, funcName, "Y %d max %d => y0 %d",
		   tile_iy, max_iy, y0));
      if (y0 >= 0) {
	int x, y2;
	int ix1 = tile->pixels[0];
	PRUint32 *data = (void *) tile->data;

	for (y2 = max_iy-tile_iy+1; y2 < tile_h; ++y2) {
	  PRUint32 const *y0p = &data[y0*span];
	  PRUint32 const *y1p = y0p+span;
	  PRUint32 *y2p = &data[y2*span];
	  /* We need to do the extension */
	  for (x = 0; x < ix1; ++x, ++y0p, ++y1p, ++y2p) {
	    PRUint32 v0 = y0p[0];
	    PRUint32 v1 = y1p[0];
	    PRUint32 v2 = v0 + (v1-v0)*(y2-y0);
	    y2p[0] = v2;
	    TSMTRACE ((CACHE_MASK|3, funcName,
		       "data[%d,%d] = [%d]%d [%d]%d => [%d]%d",
		       x, y2, y0,v0, y0+1,v1, y2,v2));
	  }
	  v = TFM_V (tile_ix, tile_iy+y2, ixiytouv);
	  TSMTRACE ((CACHE_MASK|3, funcName, "%d,%d => %.0f (> %.0f %s)",
		     tile_ix, tile_iy+y2, v, max_v,
		     v > max_v ? "TRUE" : "FALSE"));
	  if (v > max_v)
	    break;
	}
	if (y2 == tile_h) --y2;

	cache->byRes[tile->id.res].max_pxl[1] = tile_iy+y2;
	TSMTRACE ((CACHE_MASK|1, funcName, "%p[%d] extend to %dx%d",
		   cache, tile->id.res,
		   cache->byRes[tile->id.res].max_pxl[0],
		   cache->byRes[tile->id.res].max_pxl[1]));

	tile->pixels[1] = y2+1;
	TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] extend to %dx%d pixels",
		   tile->id.x, tile->id.y, tile->id.res,
		   tile->pixels[0], tile->pixels[1]));
      }
    }
  }
}

/* Convert DEM tile to floating point values and determine min/max values.
   This funtion is registered as a TILE_CACHE callback for the DEM cache.
*/

static void
TsCacheDEM_TileMap (TsCache const *cache, void *arg, void *closure)
{
  static char const funcName[] = "TsCacheDEM_TileMap";

  TsCacheTileCB *cb = arg;
  TsTile *tile = cb->tile;
  TsGeoPyramid_OIDEM *geov = closure;

  int x0, x1, ix;
  int y0, y1, iy;
  TsDEMTileAux *aux;

  if (! TsCacheTileStartTransition (tile)) {
    TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] already in transition",
	       tile->id.x, tile->id.y, tile->id.res));
    return;
  }

  /*TsCacheDEM_EnsureCoverage (cache, tile);*/
  TsCacheDEM_makeAux (cache, tile);
  aux = tile->aux;

  x0 = aux->min_x;
  x1 = aux->max_x;
  y0 = aux->min_y;
  y1 = aux->max_y;

  if (y1 >= y0 && x1 >= x0) {
    TileSetTspec const *tspec = TsCacheTspec (cache);
    SimpleTileSet const *tset = TsCacheTileSet (cache, tile->id.res);
    float const *ixiytouv = TsCacheTfm_ixiytouv (cache, tile->id.res);
#ifdef POINTCACHE
    int tile_ix = tile->id.x * tset->tileTspec.numPixelsX;
    int tile_iy = tile->id.y * tset->tileTspec.numPixelsY;
#endif
    float scale  = tspec->pixelTspec.scale;
    float offset = tspec->pixelTspec.offset;

    float **elevs = aux->elevs;
#ifdef POINTCACHE
    v3f **pts = aux->lvcs_pts;
    v3f **norms = aux->lvcs_norms;
#endif
    float z, min, max;
    TsTile const *demTop;
  
    TSMTRACE ((CACHE_MASK|2, funcName, 
	       "[%d %d %d] XY [%d,%d -> %d,%d] scale %g offset %g",
	       tile->id.x, tile->id.y, tile->id.res, x0,y0, x1,y1,
	       scale, offset));

    z = elevs[y0][x0];
    if (z == -9999 || z > 10000) z = 0;
    min = max = z;

    TsCacheOI_InitElevationRange (cache, tile);
    if (tile->bbox.min[2] < min) min = tile->bbox.min[2];
    if (tile->bbox.max[2] > max) max = tile->bbox.max[2];

    for (iy = y0; iy <= y1; ++iy) {
      float *fp = &elevs[iy][x0];
#ifdef POINTCACHE
      v3f *pt   = &pts[iy][x0];
#define INCR_PT (++pt)
#else
#define INCR_PT (0)
#endif
      for (ix = x0; ix <= x1; ++ix, ++fp, INCR_PT) {
	z = *fp;
	if (z == -9999 || z > 10000) z = 0;
#ifdef POINTCACHE
	if (! TsCacheUVtoLVCS (cache,
			       TFM_U (tile_ix+ix, tile_iy+iy, ixiytouv),
			       TFM_V (tile_ix+ix, tile_iy+iy, ixiytouv),
			       z, V(*pt)))
	  abort ();
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] %p LVCS %.0f,%.0f,%.0f",
		   ix, iy, pt, V(*pt)[0], V(*pt)[1], V(*pt)[2]));
#endif

	/* Track min/max elevation. */
	if (z < min) min = z;
	else if (z > max) max = z;
      }
    }

    tile->bbox.min[2] = min;
    tile->bbox.max[2] = max;
    TSMTRACE ((CACHE_MASK|2, funcName, "(%p) range [%g-%g]", tile, min, max));

    TsBBoxFinish (&tile->bbox);

#ifdef POINTCACHE
    /* Calculate norms for the DEM body */
    for (iy = y0; iy < y1; ++iy) {
      v3f *pt   = &pts[iy][x0];
      v3f *norm = &norms[iy][x0];
      for (ix = x0; ix < x1; ++ix, ++pt, ++norm) {
	v3f d0, d1;
	sub_v3f (&pt[1], pt, &d0);         normalize_v3f (&d0);
	sub_v3f (&pts[iy+1][ix], pt, &d1); normalize_v3f (&d1);
	TSMTRACE ((CACHE_MASK|4, funcName,
		   "[%d,%d] %.2f,%.2f,%.2f x %.2f,%.2f,%.2f",
		   ix, iy,
		   V(d0)[0], V(d0)[1], V(d0)[1],
		   V(d1)[0], V(d1)[1], V(d1)[1]));
	cross_v3f (&d0, &d1, (v3f *) norm);
	normalize_v3f ((v3f *) norm);
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] %p norm %.2f,%.2f,%.2f",
		   ix, iy, norm, V(*norm)[0], V(*norm)[1], V(*norm)[2]));
      }
      if (ix > 0)
	norm[0] = norm[-1];
      TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] %p norm %.2f,%.2f,%.2f",
		 ix, iy, norm, V(*norm)[0], V(*norm)[1], V(*norm)[2]));
    }
    /* Duplicate the last row. */
    if (iy > 0) {
      v3f *norm = &norms[iy][x0];
      for (ix = x0; ix <= x1; ++ix, ++norm) {
	norm[0] = norms[iy-1][ix];
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] %p norm %.2f,%.2f,%.2f",
		   ix, iy, norm, V(*norm)[0], V(*norm)[1], V(*norm)[2]));
      }
    }
#endif

    /* Now ensure the consistency of the pyramid. */
    if ((demTop = TsCacheDEM_PropagateElevationRange (cache, tile, 0))) {
      /* Update OI bbox if necessary */
      TsBBox const *dem_bbox = &demTop->bbox;
      TsCache *oi_cache = geov->geo.im;
      TsBBox *oi_bbox = (TsBBox *) TsCacheBBox (oi_cache);
      int changed = 0;

      if (dem_bbox->min[2] < oi_bbox->min[2]) {
	oi_bbox->min[2] = dem_bbox->min[2];
	++changed;
      }
      if (dem_bbox->max[2] > oi_bbox->max[2]) {
	oi_bbox->max[2] = dem_bbox->max[2];
	++changed;
      }
      if (changed) {
	TsBBoxFinish (oi_bbox);
	TsGeoUpdateBBox (&geov->geo);
      }
    }
  }

  if (! TsCacheTileFinishTransition (funcName, tile, TS_PREPARED))
    TSMTRACE ((CACHE_MASK|1, funcName, "[%d %d %d] transition interference",
	       tile->id.x, tile->id.y, tile->id.res));
}

/* Take an floating point (x,y) value in the coordinate system for a level of
   the DEM pyramid and convert to the coordinate system of a particular tile
   in that level.  This coordinate is returned in integer and fractional parts
   (ready for interpolation).

   What complicates this, is we may be querying a DEM tile that is not yet
   prepared, so...

   We optionally move to a different level of the DEM pyramid untiol we find
   one that is prepared.  This way we ensure that we will be able to return
   *some* answer when asked to interpolate.  In most cases, we already know
   that the tile is prepared, so we don't need to move between levels in the
   pyramid.
*/

static inline int
TsCacheDEM_ref (TsCache const *cache, TsTile const **tilep,
		float ixf, float iyf, int res,
		int *ixp, float *rxp, int *iyp, float *ryp)
{
  static char const funcName[] = "TsCacheDEM_ref";

  int res0 = res;
  TsTile const *tile;
  TsDEMTileAux *aux;
  int x, y, ix, iy;

  /* Determine ix,iy coordinates wrt. whole image at DEM resolution res. */
  SimpleTileSet const *tset = TsCacheTileSet (cache, res);
  int tile_w = tset->tileTspec.numPixelsX;
  int tile_h = tset->tileTspec.numPixelsY;

  int min_ix = TsCacheMinPixelX (cache, res);
  int max_ix = TsCacheMaxPixelX (cache, res);
  int min_iy = TsCacheMinPixelY (cache, res);
  int max_iy = TsCacheMaxPixelY (cache, res);

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "%.2f [%d-%d] %.2f [%d-%d] res %d",
	     ixf, min_ix, max_ix, iyf, min_iy, max_iy, res));

  /* Restrict to valid range */
  if (ixf < min_ix)
    ixf = min_ix;
  else if (ixf > max_ix)
    ixf = max_ix;
  if (iyf < min_iy)
    iyf = min_iy;
  else if (iyf > max_iy)
    iyf = max_iy;

  ix = floor (ixf); x = ix/tile_w;
  iy = floor (iyf); y = iy/tile_h;

  if ((tile = *tilep)
      && tile->id.x == x && tile->id.y == y && tile->id.res == res) {
    TSMTRACE ((CACHE_MASK|4, funcName, "-- reuse [%d %d %d]", 
	       tile->id.x, tile->id.y, tile->id.res));
    /* Reuse tile */
  } else {
    assert (TsCacheTileValid (cache, x, y, res));
    tile = TsCacheGetTile ((TsCache *) cache, x, y, res);
    while (! TsCacheTilePrepared (tile)) {
      ixf /= 2.0; iyf /= 2.0; ++res;
      tset = TsCacheTileSet (cache, res);

      TSMTRACE ((CACHE_MASK|4, funcName,
		 "try %.2f [%d-%d] %.2f [%d-%d] res %d",
		 ixf, min_ix, max_ix, iyf, min_iy, max_iy, res));

      /* Restrict to valid range */
      if (ixf < min_ix)
	ixf = min_ix;
      else if (ixf > max_ix)
	ixf = max_ix;
      if (iyf < min_iy)
	iyf = min_iy;
      else if (iyf > max_iy)
	iyf = max_iy;
      
      ix = floor (ixf); x = ix/tile_w;
      iy = floor (iyf); y = iy/tile_h;
      
      if ( ! TsCacheTileValid (cache, x, y, res) ) return FALSE;
      tile = TsCacheGetTile ((TsCache *) cache, x, y, res);
    }
    if (! tile)
      return FALSE;

    /* Save selected tile for next call. */
    *tilep = tile;

    TSMTRACE ((CACHE_MASK|4, funcName, "-- use [%d %d %d]", 
	       tile->id.x, tile->id.y, tile->id.res));

    if (res != res0) {
      ix = floor (ixf);
      iy = floor (iyf);
      assert (x == ix/tile_w && y == iy/tile_h);
    }
  }

  /* Recover fractional part. */
  ixf -= ix;
  iyf -= iy;
  TSMTRACE ((CACHE_MASK|4, funcName, "-- abs [%d %d] + [%.2f %.2f]", 
	     ix, iy, ixf, iyf));

  /* Make ix,iy relative to tile. */
  ix -= x * tile_w;
  iy -= y * tile_h;

  TSMTRACE ((CACHE_MASK|4, funcName, "-- rel [%d %d] + [%.2f %.2f]", 
	     ix, iy, ixf, iyf));

  /* Bound the values. */
  aux = tile->aux;
  if (ix < aux->min_x) { ix = aux->min_x; ixf = 0; }
  else if (ix >= aux->max_x) { ix = aux->max_x; ixf = 0; }
  if (iy < aux->min_y) { iy = aux->min_y; iyf = 0; }
  else if (iy >= aux->max_y) { iy = aux->max_y; iyf = 0; }

  /* Return values */
  *iyp = iy;  *ixp = ix;
  *ryp = iyf; *rxp = ixf;

  return TRUE;
}

/* Test for zero floating point value to avoid interpolating */
#define FZEROP(x) (fabs(x) < 0.01)

static float
TsCacheDEM_ElevRef (TsCache const *cache, TsTile const **tilep,
		    float ixf, float iyf, int res)
{
  static char const funcName[] = "TsCacheDEM_ElevRef";
  int ix, iy;
  TsDEMTileAux *aux;
  float elev;

  /* Find the coordinates wrt. a prepared tile */
  if (! TsCacheDEM_ref (cache, tilep, ixf, iyf, res,
			&ix, &ixf, &iy, &iyf))
    return TS_BAD_ELEVATION;

  aux = (*tilep)->aux;
  elev = FZEROP (iyf)
    ? (FZEROP (ixf)
       ? aux->elevs[iy][ix]
       : Linearf (ixf, aux->elevs[iy][ix], aux->elevs[iy][ix+1]))
    : (FZEROP (ixf)
       ? Linearf (iyf, aux->elevs[iy][ix], aux->elevs[iy+1][ix])
       : BiLinearf (ixf, iyf,
		    aux->elevs[iy  ][ix], aux->elevs[iy  ][ix+1],
		    aux->elevs[iy+1][ix], aux->elevs[iy+1][ix+1]));

  if (TSMTRACEP (CACHE_MASK|3)) {
    if (FZEROP (iyf)) {
      if (FZEROP (ixf))
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f",
		   ix,iy, &aux->elevs[iy][ix],
		   aux->elevs[iy][ix]));
      else
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f %.2f",
		   ix,iy, &aux->elevs[iy][ix], 
		   aux->elevs[iy][ix], aux->elevs[iy][ix+1]));
    } else {
      if (FZEROP (ixf)) {
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f",
		   ix,iy, &aux->elevs[iy][ix], 
		   aux->elevs[iy][ix]));
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f",
		   ix,iy+1, &aux->elevs[iy+1][ix], 
		   aux->elevs[iy+1][ix]));
      } else {
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f %.2f",
		   ix,iy, &aux->elevs[iy][ix], 
		   aux->elevs[iy][ix], aux->elevs[iy][ix+1]));
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] %p = %.2f %.2f",
		   ix,iy+1, &aux->elevs[iy+1][ix], 
		   aux->elevs[iy+1][ix], aux->elevs[iy+1][ix+1]));
      }
    }
  }

  TSMTRACE ((CACHE_MASK|3, funcName, "[%d %d %d] %.2f,%.2f: %.2f", 
	     (*tilep)->id.x, (*tilep)->id.y, (*tilep)->id.res,
	     ix+ixf, iy+iyf, elev));
  return elev;
}

static float
TsCacheDEM_Elevation (TsCache const *cache, TsTile const **tilep,
		      float u, float v, int res)
{
  static char const funcName[] = "TsCacheDEM_Elevation";
  
  /* Determine ix,iy coordinates wrt. whole image at DEM resolution res. */
  float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
  float ixf, iyf;

  ixf = TFM_IX (u, v, uvtoixiy);
  iyf = TFM_IY (u, v, uvtoixiy);

  TSMTRACE ((CACHE_MASK|3, funcName, "u,v %g,%g -> ix,iy %g,%g (res %d)",
	     u, v, ixf, iyf, res));

  return TsCacheDEM_ElevRef (cache, tilep, ixf, iyf, res);
}

#ifdef POINTCACHE
static int
TsCacheDEM_PointRef (TsCache const *cache, TsTile const **tilep,
		     float ixf, float iyf, int res,
		     v3f *pt)
{
  static char const funcName[] = "TsCacheDEM_PointRef";
  int ix, iy;

  TSMTRACE ((CACHE_MASK|3, funcName, "[%.2f,%.2f] res %d", ixf, iyf, res));

  /* Find the coordinates wrt. a prepared tile */
  if (! TsCacheDEM_ref (cache, tilep, ixf, iyf, res,
			&ix, &ixf, &iy, &iyf))
    return FALSE;

  {
    TsDEMTileAux *aux = (*tilep)->aux;
    v3f **pts = aux->lvcs_pts;
    v3f const *pt0 = &pts[iy][ix];
    int i;

    if (FZEROP (iyf)) {
      if (FZEROP (ixf)) {
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] P avg %p",
		   ix, iy, pt0));
	*pt = *pt0;
      } else {
	v3f const *pt1 = &pts[iy][ix+1];
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] P avg %p %p",
		   ix, iy, pt0, pt1));
	for (i = 0; i < 3; ++i) 
	  V(*pt)[i] = Linearf (ixf, V(*pt0)[i], V(*pt1)[i]);
      }
    } else {
      if (FZEROP (ixf)) {
	v3f const *pt1 = &pts[iy+1][ix];
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] P avg %p %p",
		   ix, iy, pt0, pt1));
	for (i = 0; i < 3; ++i) 
	  V(*pt)[i] = Linearf (iyf, V(*pt0)[i], V(*pt1)[i]);
      } else {
	v3f const *pt1 = &pts[iy  ][ix+1];
	v3f const *pt2 = &pts[iy+1][ix  ];
	v3f const *pt3 = &pts[iy+1][ix+1];
	TSMTRACE ((CACHE_MASK|3, funcName, "[%d,%d] P avg %p %p %p %p",
		   ix, iy, pt0, pt1, pt2, pt3));
	for (i = 0; i < 3; ++i) 
	  V(*pt)[i] = BiLinearf (ixf, iyf,
				 V(*pt0)[i], V(*pt1)[i],
				 V(*pt2)[i], V(*pt3)[i]);
      }
    }
  }

  TSMTRACE ((CACHE_MASK|3, funcName,
	     "[%d %d %d] %.2f,%.2f: P %.2f,%.2f,%.2f", 
	     (*tilep)->id.x, (*tilep)->id.y, (*tilep)->id.res,
	     ix+ixf, iy+iyf,
	     V(*pt)[0], V(*pt)[1], V(*pt)[2]));

  return TRUE;
}

static int
TsCacheDEM_NormRef (TsCache const *cache, TsTile const **tilep,
		    float ixf, float iyf, int res,
		    v3f *norm)
{
  static char const funcName[] = "TsCacheDEM_NormRef";
  int ix, iy;

  TSMTRACE ((CACHE_MASK|4, funcName, "[%.2f,%.2f] res %d", ixf, iyf, res));

  /* Find the coordinates wrt. a prepared tile */
  if (! TsCacheDEM_ref (cache, tilep, ixf-0.5, iyf-0.5, res,
			&ix, &ixf, &iy, &iyf))
    return FALSE;

  {
    TsDEMTileAux *aux = (*tilep)->aux;
    v3f **norms = aux->lvcs_norms;
    v3f const *norm0 = &norms[iy][ix];
    int i;

    if (FZEROP (iyf)) {
      if (FZEROP (ixf)) {
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p",
		   ix, iy, norm0));
	*norm = *norm0;
      } else {
	v3f const *norm1 = &norms[iy][ix+1];
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p",
		   ix, iy, norm0, norm1));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = Linearf (ixf, V(*norm0)[i], V(*norm1)[i]);
	normalize_v3f (norm);
      }
    } else {
      if (FZEROP (ixf)) {
	v3f const *norm1 = &norms[iy+1][ix];
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p",
		   ix, iy, norm0, norm1));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = Linearf (iyf, V(*norm0)[i], V(*norm1)[i]);
	normalize_v3f (norm);
      } else {
	v3f const *norm1 = &norms[iy  ][ix+1];
	v3f const *norm2 = &norms[iy+1][ix  ];
	v3f const *norm3 = &norms[iy+1][ix+1];
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p %p %p",
		   ix, iy, norm0, norm1, norm2, norm3));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = BiLinearf (ixf, iyf,
				   V(*norm0)[i], V(*norm1)[i],
				   V(*norm2)[i], V(*norm3)[i]);
	normalize_v3f (norm);
      }
    }
  }

  TSMTRACE ((CACHE_MASK|4, funcName,
	     "[%d %d %d] %.2f,%.2f: N %.2f,%.2f,%.2f", 
	     (*tilep)->id.x, (*tilep)->id.y, (*tilep)->id.res,
	     ix+ixf, iy+iyf,
	     V(*norm)[0], V(*norm)[1], V(*norm)[2]));

  return TRUE;
}
#else

#ifdef INSTRUMENT_CACHE
#define INSTRUMENT_COUNT(x) PR_AtomicIncrement (&x)
PRInt32 PointRef_points;
PRInt32 PointRef_points_cached;
#else
#define INSTRUMENT_COUNT(x)
#endif

static v3f const *
TsCacheDEM_PointRef (TsCache const *cache, TsTile const **tilep,
		     float u, float v, float const *zp,
		     int ix0, int iy0, int res)
{
  static char const funcName[] = "TsCacheDEM_PointRef";
  int ix, iy;
  float ixf, iyf;

  /* Find the coordinates wrt. a prepared tile */
  if (! TsCacheDEM_ref (cache, tilep, ix0, iy0, res,
			&ix, &ixf, &iy, &iyf))
    return 0;

  TSMTRACE ((CACHE_MASK|2, funcName,
	     "[%d,%d] res %d => [%d,%d] wrt [%d %d %d]",
	     ix0, iy0, res, ix, iy,
	     (*tilep)->id.x, (*tilep)->id.y, (*tilep)->id.res));

  {
    TsDEMTileAux *aux = (*tilep)->aux;
    v3f ***pts = aux->lvcs_pts;
    v3f **pt0 = &pts[iy][ix];

    INSTRUMENT_COUNT (PointRef_points);
    if (! *pt0) {
      float z;

      z = (zp ? *zp : TsCacheDEM_ElevRef (cache, tilep, ix0, iy0, res));

      /* Allocate space for a new point */
      *pt0 = aux->next_pt++;
      TsCacheUVtoLVCS (cache, u, v, z, V(**pt0));
      TSMTRACE ((CACHE_MASK|2, funcName, "[%.0f,%.0f,%.0f]",
		 V(**pt0)[0], V(**pt0)[1], V(**pt0)[2]));
    } else {
      INSTRUMENT_COUNT (PointRef_points_cached);
      TSMTRACE ((CACHE_MASK|2, funcName, "[%.0f,%.0f,%.0f] reused",
		 V(**pt0)[0], V(**pt0)[1], V(**pt0)[2]));
    }

    return *pt0;
  }
}

static int
TsCacheDEM_CalcNorm (TsCache const *cache, TsTile const **tilep,
		     int ix0, int iy0, int res, v3f *norm)
{
  static char const funcName[] = "TsCacheDEM_CalcNorm";
  SimpleTileSet const *tset = TsCacheTileSet (cache, res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, res);
  int max_ix = TsCacheMaxPixelX (cache, res);
  int max_iy = TsCacheMaxPixelY (cache, res);
  int neg = FALSE;
  v3f const *p0, *px, *py;
  int ix, iy;

  ix = ix0; iy = iy0;
  p0 = TsCacheDEM_PointRef (cache, tilep,
			    TFM_U (ix, iy, ixiytouv),
			    TFM_V (ix, iy, ixiytouv),
			    NULL,
			    ix, iy, res);

  ix = ix0+1; iy = iy0;
  if (ix > max_ix) {
    neg = !neg;
    ix = ix0 - 1;
  }
  px = TsCacheDEM_PointRef (cache, tilep,
			    TFM_U (ix, iy, ixiytouv),
			    TFM_V (ix, iy, ixiytouv),
			    NULL,
			    ix, iy, res);

  ix = ix0; iy = iy0+1;
  if (iy > max_iy) {
    neg = !neg;
    iy = iy0 - 1;
  }
  py = TsCacheDEM_PointRef (cache, tilep,
			    TFM_U (ix, iy, ixiytouv),
			    TFM_V (ix, iy, ixiytouv),
			    NULL,
			    ix, iy, res);

  {
    v3f d0, d1;
    sub_v3f (px, p0, &d0);
    sub_v3f (py, p0, &d1);
    TSMTRACE ((CACHE_MASK|4, funcName,
	       "[%d,%d] %.2f,%.2f,%.2f x %.2f,%.2f,%.2f",
	       ix0, iy0,
	       V(d0)[0], V(d0)[1], V(d0)[1],
	       V(d1)[0], V(d1)[1], V(d1)[1]));
    if (! neg)
      cross_v3f (&d0, &d1, norm);
    else
      cross_v3f (&d1, &d0, norm);
    normalize_v3f (norm);
  }

  return TRUE;
}

PRInt32 NormRef_norms;
PRInt32 NormRef_norms_cached;

static int
TsCacheDEM_NormRef (TsCache const *cache, TsTile const **tilep,
		    float ixf, float iyf, int res,
		    v3f *norm)
{
  static char const funcName[] = "TsCacheDEM_NormRef";
  int ix, iy;

  TSMTRACE ((CACHE_MASK|4, funcName, "[%.2f,%.2f] res %d", ixf, iyf, res));

  /* Find the coordinates wrt. a prepared tile */
  if (! TsCacheDEM_ref (cache, tilep, ixf-0.5, iyf-0.5, res,
			&ix, &ixf, &iy, &iyf))
    return FALSE;

  {
    SimpleTileSet const *tset = TsCacheTileSet (cache, res);
    int tile_w = tset->tileTspec.numPixelsX;
    int tile_h = tset->tileTspec.numPixelsY;
    TsDEMTileAux *aux = (*tilep)->aux;
    v3f ***norms = aux->lvcs_norms;
    v3f **norm0 = &norms[iy][ix];
    int i;
    int ox = (*tilep)->id.x * tile_w;
    int oy = (*tilep)->id.y * tile_h;

    INSTRUMENT_COUNT (NormRef_norms);
    if (! *norm0) {
      *norm0 = aux->next_norm++;
      TsCacheDEM_CalcNorm (cache, tilep, ox+ix, oy+iy, res, *norm0);
    } else {
      INSTRUMENT_COUNT (NormRef_norms_cached);
    }

    if (FZEROP (iyf)) {
      if (FZEROP (ixf)) {
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p",
		   ix, iy, norm0));
	*norm = **norm0;
      } else {
	v3f **norm1 = &norms[iy][ix+1];
	INSTRUMENT_COUNT (NormRef_norms);
	if (! *norm1) {
	  *norm1 = aux->next_norm++;
	  TsCacheDEM_CalcNorm (cache, tilep, ox+ix+1, oy+iy, res, *norm1);
	} else {
	  INSTRUMENT_COUNT (NormRef_norms_cached);
	}
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p",
		   ix, iy, *norm0, *norm1));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = Linearf (ixf, V(**norm0)[i], V(**norm1)[i]);
	normalize_v3f (norm);
      }
    } else {
      if (FZEROP (ixf)) {
	v3f **norm1 = &norms[iy+1][ix];
	INSTRUMENT_COUNT (NormRef_norms);
	if (! *norm1) {
	  *norm1 = aux->next_norm++;
	  TsCacheDEM_CalcNorm (cache, tilep, ox+ix, oy+iy+1, res, *norm1);
	} else {
	  INSTRUMENT_COUNT (NormRef_norms_cached);
	}
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p",
		   ix, iy, *norm0, *norm1));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = Linearf (iyf, V(**norm0)[i], V(**norm1)[i]);
	normalize_v3f (norm);
      } else {
	v3f **norm1 = &norms[iy  ][ix+1];
	v3f **norm2 = &norms[iy+1][ix  ];
	v3f **norm3 = &norms[iy+1][ix+1];
	INSTRUMENT_COUNT (NormRef_norms);
	if (! *norm1) {
	  *norm1 = aux->next_norm++;
	  TsCacheDEM_CalcNorm (cache, tilep, ox+ix+1, oy+iy, res, *norm1);
	} else {
	  INSTRUMENT_COUNT (NormRef_norms_cached);
	}
	INSTRUMENT_COUNT (NormRef_norms);
	if (! *norm2) {
	  *norm2 = aux->next_norm++;
	  TsCacheDEM_CalcNorm (cache, tilep, ox+ix, oy+iy+1, res, *norm2);
	} else {
	  INSTRUMENT_COUNT (NormRef_norms_cached);
	}
	INSTRUMENT_COUNT (NormRef_norms);
	if (! *norm3) {
	  *norm3 = aux->next_norm++;
	  TsCacheDEM_CalcNorm (cache, tilep, ox+ix+1, oy+iy+1, res, *norm3);
	} else {
	  INSTRUMENT_COUNT (NormRef_norms_cached);
	}
	TSMTRACE ((CACHE_MASK|4, funcName, "[%d,%d] N avg %p %p %p %p",
		   ix, iy, norm0, norm1, norm2, norm3));
	for (i = 0; i < 3; ++i) 
	  V(*norm)[i] = BiLinearf (ixf, iyf,
				   V(**norm0)[i], V(**norm1)[i],
				   V(**norm2)[i], V(**norm3)[i]);
	normalize_v3f (norm);
      }
    }
  }

  TSMTRACE ((CACHE_MASK|3, funcName,
	     "[%d %d %d] %.2f,%.2f: N %.2f,%.2f,%.2f", 
	     (*tilep)->id.x, (*tilep)->id.y, (*tilep)->id.res,
	     ix+ixf, iy+iyf,
	     V(*norm)[0], V(*norm)[1], V(*norm)[2]));

  return TRUE;
}
#endif

/*
 * Synopsis:
 *    void TsCacheDEM_PointNormal (TsCache const *cache,
 *                                 TsTile const **tilep, int res,
 *                                 float u, float v, float const *zp,
 *                                 float point[3], float norm[3])
 *
 * Doc++ Description: *//** 
 *   Calculates the LVCS point and normal given U,V.  First computes the
 *   elevation at that point (given a resolution in the DEM cache), unless the
 *   caller passes in a non-NULL zp value, which points to the correct
 *   elevation.
 *
 * @param    cache	The DEM cache to use.
 * @param    tilep	An array of DEM tiles to index into for elevation.
 * @param    res        The level of the DEM cache to use
 * @param    u          The U coordinate (in DEM UV)
 * @param    v          The V coordinate (in DEM UV)
 * @param    zp         A pointer to the elevation (optionally NULL)
 * @param    point      Return location of the DEM LVCS point
 * @param    norm       Return location of the DEM LVCS normal
 *
 * @return   void
 *
 * @author   Lee Iverson
 * @version  January 2001
 *
 */

PRInt32 PointNormal_points;
PRInt32 PointNormal_points_cached;

static void
TsCacheDEM_PointNormal (TsCache const *cache,
			TsTile const **tilep, int res,
			float u, float v, float const *zp,
			float point[3], float norm[3])
{
  static char const funcName[] = "TsCacheDEM_PointNormal";
  
  /* Determine ix,iy coordinates wrt. whole image at DEM resolution res. */
  float const *uvtoixiy = TsCacheTfm_uvtoixiy (cache, res);
  float const *ixiytouv = TsCacheTfm_ixiytouv (cache, res);
  float ixf, iyf;
  float z;

  ixf = TFM_IX (u, v, uvtoixiy);
  iyf = TFM_IY (u, v, uvtoixiy);

  TSMTRACE ((CACHE_MASK|4, funcName, "u,v %g,%g -> ix,iy %g,%g (res %d)",
	     u, v, ixf, iyf, res));

#ifndef POINTCACHE
  if (point) {
    int ix = floor(ixf), iy = floor(iyf);
    int done = FALSE;

    /* Check to see if ixf and ixy are integers and if so, use the integral,
       cached primitive to obtain LVCS coords. */
    INSTRUMENT_COUNT (PointNormal_points);    
    if (FZEROP (ixf-ix) && FZEROP (iyf-iy)) {
      v3f const *p = TsCacheDEM_PointRef (cache, tilep,
					  u, v, zp,
					  ix, iy, res);
      if (p) {
	*(v3f*)point = *p;
	INSTRUMENT_COUNT (PointNormal_points_cached);
	done = TRUE;
      }
    }

    /* If we weren't able to use the integral interpolator, then do manual
       conversion (u,v,z) -> LVCS */
    if (! done) {
      z = (zp ? *zp : TsCacheDEM_ElevRef (cache, tilep, ixf, iyf, res));
      TsCacheUVtoLVCS (cache, u, v, z, point);
    }
  }

#if 1
  if (norm)
    TsCacheDEM_NormRef (cache, tilep, ixf, iyf, res, (v3f *) norm);
#else
  if (norm) {
    int max_ix = TsCacheMaxPixelX (cache, res);
    int max_iy = TsCacheMaxPixelY (cache, res);
    int neg = FALSE;
    v3f p0, px, py;
    float ix, iy;

    ixf = floor (ixf);
    iyf = floor (iyf);
    z = TsCacheDEM_ElevRef (cache, tilep, ixf, iyf, res);
    TsCacheUVtoLVCS (cache,
		     TFM_U (ixf, iyf, ixiytouv),
		     TFM_V (ixf, iyf, ixiytouv),
		     z, V(p0));

    ix = ixf+1; iy = iyf;
    if (ix > max_ix) {
      neg = !neg;
      ix = ixf -1;
    }
    z = TsCacheDEM_ElevRef (cache, tilep, ix, iy, res);
    TsCacheUVtoLVCS (cache,
		     TFM_U (ix, iy, ixiytouv),
		     TFM_V (ix, iy, ixiytouv),
		     z, V(px));
    
    ix = ixf; iy = iyf+1;
    if (iy > max_iy) {
      neg = !neg;
      iy = iyf-1;
    }
    z = TsCacheDEM_ElevRef (cache, tilep, ix, iy, res);
    TsCacheUVtoLVCS (cache,
		     TFM_U (ix, iy, ixiytouv),
		     TFM_V (ix, iy, ixiytouv),
		     z, V(py));

    {
      v3f d0, d1;
      sub_v3f (&px, &p0, &d0);
      sub_v3f (&py, &p0, &d1);
      TSMTRACE ((CACHE_MASK|4, funcName,
		 "[%g,%g] %.2f,%.2f,%.2f x %.2f,%.2f,%.2f",
		 ixf, iyf,
		 V(d0)[0], V(d0)[1], V(d0)[1],
		 V(d1)[0], V(d1)[1], V(d1)[1]));
      if (! neg)
	cross_v3f (&d0, &d1, (v3f *) norm);
      else
	cross_v3f (&d1, &d0, (v3f *) norm);
      normalize_v3f ((v3f *) norm);
    }
  }
#endif

#ifdef INSTRUMENT_CACHE
  if (PointNormal_points%100 == 0) {
    fprintf (stderr, "PointNormal: %d (%.1f%% cached)\n", 
	     PointNormal_points, 
	     (100.0*PointNormal_points_cached)/PointNormal_points);
    fprintf (stderr, "PointRef   : %d (%.1f%% cached)\n", 
	     PointRef_points, (100.0*PointRef_points_cached)/PointRef_points);
    fprintf (stderr, "NormRef    : %d (%.1f%% cached)\n", 
	     NormRef_norms, (100.0*NormRef_norms_cached)/NormRef_norms);
  }
#endif
#else
  if (point)
    TsCacheDEM_PointRef (cache, tilep, ixf, iyf, res, (v3f *) norm);
  if (norm)
    TsCacheDEM_NormRef (cache, tilep, ixf, iyf, res, (v3f *) norm);
#endif
}

/*
 * Synopsis:
 *   int TsGeoWholeWorld (TsGeoPyramid const *geo)
 *
 * Doc++ Description: *//** 
 *   Takes a TsGeoPyramid and returns a boolean result describing whether
 *   the terrain dataset covers the entire planet. That is, it covers
 *   360 degrees of longitude and 180 degrees of latitude.
 *
 * @param    geo	The TsGeoPyramid to be tested
 *
 * @return   TRUE if the dataset covers the entire planet, otherwise FALSE.
 *
 * @author   Martin Reddy
 * @version  December 2000
 *
 */

int
TsGeoWholeWorld (TsGeoPyramid const *geo)
{
  const TileSetTspec *tspec;

  if ( ! geo ) return FALSE;

  if ( ! ( tspec = TsCacheTspec( geo->im ) ) ) return FALSE;

  return ( tspec->wrap_lat && tspec->wrap_lon );
}

/*
 * Synopsis:
 *   int TsGeoIsSky (TsGeoPyramid const *geo)
 *
 * Doc++ Description: *//** 
 *   Returns whether this GeoPyramids is for a sky dataset
 *
 * @param    geo	The TsGeoPyramid to be tested
 *
 * @return   TRUE if the dataset is for a sky image
 *
 * @author   Martin Reddy
 * @version  November 2001
 *
 */

int
TsGeoIsSky (TsGeoPyramid const *geo)
{
  const TileSetTspec *tspec;

  if ( ! geo ) return FALSE;

  if ( ! ( tspec = TsCacheTspec( geo->im ) ) ) return FALSE;

  return ( tspec->sky );

}

/*
 * Synopsis:
 *   int TsGeoVisibleSides (TsGeoPyramid const *geo)
 *
 * Doc++ Description: *//** 
 *   Find out whether the dataset is to be rendered double-sided
 *   or with only either the front or back visible.
 *
 * @param    geo	The TsGeoPyramid to be tested
 *
 * @return   0 = front only, 1 = back only, 2 = both sides
 *
 * @author   Martin Reddy
 * @version  November 2001
 *
 */

int
TsGeoVisibleSides (TsGeoPyramid const *geo)
{
  const TileSetTspec *tspec;

  if ( ! geo ) return 0;

  if ( ! ( tspec = TsCacheTspec( geo->im ) ) ) return 0;

  if ( tspec->twoSided )
    return 2;
  if ( tspec->fromBelow )
    return 1;

  return 0;
}

/*
 * Synopsis:
 *  float TsGeoOriginAltitude (TsGeoPyramid const *geo)
 *
 * Doc++ Description: *//** 
 *   Returns the altitude specified for the DEM-less OI dataset.
 *   Returns 0.0 in the case of anything else.
 *
 * @param    geo	The TsGeoPyramid to be tested
 *
 * @return   altitude value in meters
 *
 * @author   Martin Reddy
 * @version  November 2001
 *
 */

float
TsGeoOriginAltitude (TsGeoPyramid const *geo)
{
  const TileSetTspec *tspec;

  if ( ! geo ) return 0.0;

  if ( ! ( tspec = TsCacheTspec( geo->im ) ) ) return 0.0;

  return ( tspec->coordTspec.originAltitude );
}

/*
 * Synopsis:
 *  TsGeoPyramid *TsGeoFind( char const *geoName )
 *
 * Doc++ Description: *//** 
 *   Returns the currently-loaded geopyramid with the
 *   specified geoname (as returned by TsGeoName)
 *
 * @param    geoName	The TsGeoPyramid name to be matched against
 *
 * @return   The TsGeoPyramid structure, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  December 2001
 *
 */

TsGeoPyramid *
TsGeoFind ( char const *geoName )
{
	list_iter_t i;
	
	if ( geoList == NULL || geoName == NULL ) return NULL;

	for ( i = list_first(geoList); ! list_done(i); i = list_next(i) ) {
		TsGeoPyramid *geo = list_value(i);
		if ( geo && strcmp( geoName, TsGeoName(geo) ) == 0 )
			return geo;
	}

	return NULL;
}

/*
 * Synopsis:
 *  TsGeoPyramid *TsGeoFindByURL( char const *url )
 *
 * Doc++ Description: *//** 
 *   Returns the currently-loaded geopyramid with an
 *   OI or DEM that matches exactly the specified URL
 *
 * @param    url	the http:// or file:// URL to search for
 *
 * @return   The TsGeoPyramid structure, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  January 2002
 *
 */

TsGeoPyramid *
TsGeoFindByURL ( char const *url )
{
	list_iter_t i;
	
	if ( geoList == NULL || url == NULL ) return NULL;

	for ( i = list_first(geoList); ! list_done(i); i = list_next(i) ) {
    char const *urls[5];
    int  count, j;
    TsGeoPyramid *geo = list_value(i);

    count = TsGeoOIDEM_URLs ( geo, urls );
    while ( --count >= 0 ) {
      Purl *purl1, *purl2;
      int  match;
      if ( ! urls[count] ) continue;
      purl1 = tsmParseUrl( urls[count] );
      purl2 = tsmParseUrl( url );
      match = ( purl1 && purl2 && strcmp( purl1->url, purl2->url ) == 0 );
      if ( purl1 ) tsmFreePurl( purl1 );
      if ( purl2 ) tsmFreePurl( purl2 );
      if ( match ) return geo;
    }
	}

	return NULL;
}

/*
 * Synopsis:
 *   int TsGeoIsLogging(void)
 *
 * Doc++ Description: *//** 
 *   Returns TRUE or FALSE indicating whether logging is currently
 *   activated. That is, logging of NetLogger events for the specified
 *   connection.
 *
 * @param geo   handle to geoset to be tested for logging status
 *
 * @return   TRUE if NetLogger logging is enabled, otherwise FALSE.
 *
 * @author   Martin Reddy
 * @version  9 April 2001
 *
 * @see    TsGeoLogging, TsGeoPyramid
 */

int
TsGeoIsLogging(TsGeoPyramid *geo)
{
  return geo ? geo->logging : FALSE;
}
