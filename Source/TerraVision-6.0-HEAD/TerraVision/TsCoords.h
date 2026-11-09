/* -*- tab-width: 2; -*-

  Revision History:
      $Id: TsCoords.h,v 2.5.2.2 2002/10/31 05:26:39 reddy Exp $

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

#ifndef _TsCoords_h_
#define _TsCoords_h_

#include "TsCache.h"

#ifdef __cplusplus
extern "C" {
#endif

int TsGCCtoGDC (double x, double y, double z, double gdc[3]);
int TsGDCtoGCC (double lng, double lat, double z, double gcc[3]);

int TsCacheUVtoGDC (TsCache const *cache,
		    float u, float v, float z, double gdc[3]);
int TsCacheGDCtoUV (TsCache const *cache,
		    double lat, double lng, double z, float uv[3]);
int TsCacheUVtoGCC (TsCache const *cache,
		    float u, float v, float z, double gcc[3]);
int TsCacheGCCtoUV (TsCache const *cache,
		    double x, double y, double z, float uv[3]);

int TsLVCSOrientation (double const origin[3], float orient[4]);

int TsCacheLVCStoGCC (TsCache const *cache,
		      float l0, float l1, float l2, double g[3]);
int TsCacheLVCStoGDC (TsCache const *cache,
		      float l0, float l1, float l2, double g[3]);
int TsCacheGCCtoLVCS (TsCache const *cache,
		      double g0, double g1, double g2, float l[3]);
int TsCacheGDCtoLVCS (TsCache const *cache,
		      double g0, double g1, double g2, float l[3]);

int TsCacheUVtoLVCS (TsCache const *cache, 
		     float u, float v, float z, float lvcs[3]);
  
int TsCacheLVCStoUV (TsCache const *cache, 
		     float x, float y, float z, float uv[3]);

/* To be called from TsCacheConnect only. */
int TsCacheCoordsInit (TsCache *cache);
void TsCacheCoordsDestroy (TsCache *cache);

#if defined(HAVE_INLINE) && !defined(NO_TsCoords_INLINE)
# ifdef __GNUC__
#  define exinline extern inline
# else
#  define exinline inline
# endif
# include "TsCoordsInline.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _TsCoords_h_ */
