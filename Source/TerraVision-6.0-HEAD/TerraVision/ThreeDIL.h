/* -*- tab-width: 2; -*-

  Revision History:
      $Id: ThreeDIL.h,v 2.8.4.2 2002/10/31 05:26:36 reddy Exp $

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

#include "Trace.h"
#include <prtypes.h>

#include "TsGeoPyramidI.h"

exinline TsGeoPyramid *
tvBaseTop (tvBase *threeD)
{
  GeoElement *geo_elem = 0;
  if (! list_empty (threeD->geoList))
    geo_elem = (struct GeoElement *)list_value (list_first (threeD->geoList));

  return (geo_elem ? geo_elem->geo : 0);
}

/*** Deadcode?  --RLB 10/18/99 */
exinline float
tvBaseAltitude (tvBase * threeD)
{
  return threeD->view.altitude;
}

/*
 * Synopsis:
 *   int tvHeading (m4f const *view,
 *                  float x, float y, float z,
 *                  v3f *h)
 *
 * Description:
 *   Transforms a unit vector, representing an eye-centered heading and
 *   returns a heading in the global coordinate system.
 *
 *   These are wrt. the canonical view matrix, therefore:
 *     forward = -Z
 *     right   = +X
 *     up      = +Y
 *
 * Returns: The unit vector in heading[3].
 *
 * Author: Lee Iverson
 *
 * Date: 9 April 1999.
 *
 */

exinline void
tvHeading (m4f const *view,
	   float x, float y, float z,
	   v3f *h)
{
  v3f u;
  V(u)[0] = x; V(u)[1] = y; V(u)[2] = z;
  mult_v3f_m4f (&u, view, h);
}
