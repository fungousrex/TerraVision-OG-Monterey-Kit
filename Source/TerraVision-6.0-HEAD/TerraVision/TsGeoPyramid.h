/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsGeoPyramid.h,v 2.43.2.11 2002/10/31 05:26:40 reddy Exp $

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

#ifndef _TsGeoPyramid_h_
#define _TsGeoPyramid_h_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "TsCache.h"
#include "TsCoords.h"

#include <vrml2/vec3.h>

/*
 * Doc++ Description: *//** 
 *   Type of a TsGeoPyramid
 *
 *   An identifier for the various types of GeoPyramids we support, currently
 *   two. 
 *
 * @param TsGeo_OI_DEM   Matched DEM and OI caches.
 *
 * @see TsGeoPyramid
 *
 */

enum TsGeoPyramidType {
  TsGeo_OI_DEM=0
}; 
typedef enum TsGeoPyramidType TsGeoPyramidType;

/*
 * Doc++ Description: *//** 
 *   General information about a TsGeoPyramid.
 *
 *   A structure holding higher level information about the contents of a
 *   TsGeoPyramid.
 *
 * @param minLevel      Minimum level of the pyramid.
 * @param maxLevel      Maximum level of the pyramid.
 * @param tileNumPixels Number of pixels in [0]=X and [1]=Y directions
 * @param tileNumComps  Number of RGB components per pixel
 * @param wholeEarth    Does the dataset cover the whole earth?
 * @param transparent   Is the dataset semi-transparent?
 * @param oneSided      Is the dataset only viewable from one side?
 * @param viewAbove     Is the dataset viewable from above?
 * @param lightable     Is the dataset subject to lighting?
 * @param skySurface    Is the dataset to be used as the sky background?
 * @param earthRadius   The earth radii [0]=equatorial [1]=polar assumed
 *
 * @see TsGeoPyramid, TsGeoGetInfo
  *
 */

typedef struct TsGeoInfo {
  PRUint16 minLevel;
  PRUint16 maxLevel;

  PRUint16 tileNumPixels[2];
  PRUint8  tileNumComps;

  PRUintn  wholeEarth:1;
  PRUintn  transparent:1;
  PRUintn  oneSided:1;
  PRUintn  viewAbove:1;
  PRUintn  lightable:1;
  PRUintn  skySurface:1;

  double   earthRadius[2];
} TsGeoInfo;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TsGeoPyramid TsGeoPyramid;
typedef struct TsGeoView TsGeoView;

typedef struct TsCoord {
  float texCoord[2];
  float norm[3];
  float vertex[3];
} TsCoord;

typedef struct TsGeoTileAux {
  v3f normal;
  float normalThreshold;
  PRUint16 npolys;
  PRUint16 coords_w, coords_h;
  TsCoord *coords;
  PRUint16 neighIndex[9];
} TsGeoTileAux;

/* Global list of TsGeoPyramids */
extern list_t *geoList;

TsGeoPyramid *TsGeoCreate_OIDEM (char const *oiURL,
				 TsmConnectionParams oiParams, 
				 char const *demURL,
				 TsmConnectionParams demParams);

TsGeoPyramid *TsGeoCreate_OI (char const *oiURL,
			      TsmConnectionParams oiParams);

void TsGeoDestroy (TsGeoPyramid *geo);

char const * TsGeoName (TsGeoPyramid const *geo);
int TsGeoConnections (TsGeoPyramid *geo, TsmConnection *connects);

TsCache const * TsGeoCache (TsGeoPyramid const *geo);
TsTile * TsGeoTop (TsGeoPyramid const *geo);
int TsGeoChildren (TsTile const *tile, TsTile **children);
TsTile * TsGeoParent (TsTile const *tile);
TsTile * TsGeoNeighbour (TsTile const *tile, TsCacheNeighbour dir);
int TsGeoNeighbours (TsTile const *tile, TsTile **neighbours);

/* American aliases */
#define TsGeoNeighbor TsGeoNeighbour
#define TsGeoNeighbors TsGeoNeighbours

/* Image resolution in meters/pixel */
float TsGeoResolution (TsGeoPyramid const *geo);

/* Return the URLs associated with a TsGeoPyramid (up to 2). */
int TsGeoURLs (TsGeoPyramid const *geo, char const **urls);

/* Returns a position high above the center of the dataset */
void TsGeoViewpoint (TsGeoPyramid const *geo, float aspect, double *eye);

/* Returns whether the GeoPyramid covers the entire planet */
int TsGeoWholeWorld (TsGeoPyramid const *geo);

/* Returns whether this GeoPyramids is for a sky dataset */
int TsGeoIsSky (TsGeoPyramid const *geo);

/* Returns 0 = front only, 1 = back only, 2 = both sides */
int TsGeoVisibleSides (TsGeoPyramid const *geo);

/* Returns the altitude specified for the DEM-less OI dataset */
float TsGeoOriginAltitude (TsGeoPyramid const *geo);

TsTile *TsGeoTile (TsGeoPyramid const *geo, int x, int y, int res);
int TsGeoTileValid (TsGeoPyramid const *geo, int x, int y, int res);

TsGeoView * TsGeoCreateView (TsGeoPyramid *geo);

TsGeoPyramid * TsGeoViewPyramid (TsGeoView *gv);

TsGeoPyramid *TsGeoFind ( char const *geoName );
TsGeoPyramid *TsGeoFindByURL ( char const *url );


/* Returns TRUE if the tile is prepared for display.
   If not, and request_prio is non-NULL, then generates a tile
   request */
int TsGeoViewTilePrepared (TsGeoView *gv,
			   TsTile *tile, int const *request_prio);

/* Actions on the request list associated with the TsGeoView */
void TsGeoViewSync (TsGeoView *gv);
void TsGeoViewClear (TsGeoView *gv);
void TsGeoViewDestroy (TsGeoView *gv);

v3f const *TsGeoTileNormal (TsTile const *tile);
float TsGeoTileNormalThreshold (TsTile const *tile);
TsGeoTileAux const *TsGeoTileGetAux (TsTile const *tile);

int TsGeoAddCallback (TsGeoPyramid *geo, TsCacheCB which,
		      TsCallbackF func, void *closure,
		      void (*destroyf) (void *closure));
int TsGeoRemoveCallback (TsGeoPyramid *geo, TsCacheCB which,
			 TsCallbackF func, void *closure);

void TsGeoGCCtoLVCS (TsGeoPyramid const *geo,
		     double g0, double g1, double g2, float l[3]);
void TsGeoLVCStoGCC (TsGeoPyramid const *geo, 
		     float l0, float l1, float l2, double g[3]);

int TsGeoUVtoGCC (TsGeoPyramid const *geo, 
		  float u, float v, float z,
		  double gcc[3]);
int TsGeoGCCtoUV (TsGeoPyramid const *geo, 
		  double x, double y, double z,
		  float uv[3]);
void TsGeoLVCStoUV (TsGeoPyramid const *geo, 
		    float x, float y, float z,
		    float uv[3]);

#define TS_BAD_ELEVATION (-1e6)
float TsGeoElevationUV (TsGeoPyramid const *geo, float u, float v);

float TsGeoPixelDiameter (TsGeoPyramid const *geo, int res);
float TsGeoPixelDiameter2 (TsGeoPyramid const *geo, int res);

float TsGeoElevationLVCS (TsGeoPyramid const *geo, float x, float y, float z);
float TsGeoElevation (TsGeoPyramid const *geo, double const v[3]);

float TsGeoAltitudeLVCS (TsGeoPyramid const *geo, float const l[3]);
float TsGeoAltitude (TsGeoPyramid const *geo, double const pos[3]);

TsBBox const *TsGeoBBox (TsGeoPyramid const *geo);
TsBBox const *TsGeoBBoxLVCS (TsGeoPyramid const *geo);
TsBBox const *TsGeoBBoxUV (TsGeoPyramid const *geo);
TsBBox const *TsGeoBBoxGDC (TsGeoPyramid const *geo);

double const *TsGeoOrigin (TsGeoPyramid const *geo);
float const *TsGeoOrientation (TsGeoPyramid const *geo);

TsGeoInfo const *TsGeoGetInfo (TsGeoPyramid const *geo);

int TsGeoLogging (TsGeoPyramid *geo, int logging);

int TsGeoIsLogging(TsGeoPyramid *geo);


#ifdef HAVE_INLINE
# include "NetLogCore.h"
# include <se_conv.h>

# ifndef NO_TsGeoPyramid_INLINE
#  ifdef __GNUC__
#   define exinline extern inline
#  else
#   define exinline inline
#  endif
#  include "TsGeoPyramidInline.h"
#  undef exinline
#  endif /* NO_TsGeoPyramid_INLINE */
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _TsGeoPyramid_h_ */
