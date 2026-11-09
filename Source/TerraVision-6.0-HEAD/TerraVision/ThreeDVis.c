/* -*- tab-width: 2; -*-

  Name: ThreeDVis.c

  Description: Visibility handling for a tvBase instance.

  Revision History:
      $Id: ThreeDVis.c,v 2.26.2.9 2002/10/31 05:26:37 reddy Exp $

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

#include "TerraVisionInc.h"

#include "ThreeDCoreI.h"

#include "Visible.h"
#include "Texture.h"
#include "TsLogging.h"

#include <math.h>
#include <float.h>
#include <assert.h>

#include <prerror.h>

/* RCS INFO */
static char const ThreeDVis_rcsid[] = "$Id: ThreeDVis.c,v 2.26.2.9 2002/10/31 05:26:37 reddy Exp $";

/* STATIC LOCAL FUNCTIONS */
static void tvBaseRegenerateQuads (tvBase * threeD,
				   tvViewpoint const * view);

static int visible_cycle_min = 50;
static int visible_cycle_max = 100;

/* Doc++ Documentation: *//**
 *   ISSUE: Not clear what the role is here -- RLB.
 *
 * @param min 
 * @param max
 *
 */
void
tvBaseSetVisibleCycle (int min, int max)
{
  if (min)
    visible_cycle_min = min;
  if (max)
    visible_cycle_max = max;
}

void
tvBaseVisibleCycle (int *min, int *max)
{
  if (min)
    *min = visible_cycle_min;
  if (max)
    *max = visible_cycle_max;
}

void
tvBaseClearAllVisibleLists( tvBase *threeD, int vis_locked )
{
	if ( ! vis_locked )
		PR_Lock (threeD->visible_lock);

	ClearWorkList (threeD, threeD->workingList0);
	ClearWorkList (threeD, threeD->workingList1);

	if ( ! vis_locked )
		PR_Unlock (threeD->visible_lock);
}

static FILE *vis_out = 0;

#ifdef VIS_THREAD
void
tvBaseVisibleThread(void *generic)
{
  static char const funcName[] = "tvBaseVisibleThread";

  tvBase *threeD = (tvBase *) generic;
  tvViewpoint curr_view;
  int draw_count = 0;

  PRCondVar *vis_cond = threeD->vis_cond;

  /* Create a separate trace if VIS_OUT set */
#ifdef HAVE_GETENV
  {
    char *vis_name;
    if (vis_name = getenv ("VIS_OUT")) {
      vis_out = fopen (vis_name, "w");
      TSMTRACE ((VIS_MASK|1, funcName, "VIS_OUT => %s", vis_name));
    }
  }
#endif

  while (1) {
    int numTiles;
    PRIntervalTime startTime, endTime;

    if (tvBaseWaitRunnable (threeD) != PR_SUCCESS) {
      /* This is the signal to exit. */
      if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR)
	TSMTRACE ((VIS_MASK|1, funcName, "Interrupted."));
      TSMTRACE ((VIS_MASK|1, funcName, "Exiting thread"));
      return;
    }

    TSMTRACE ((VIS_MASK|1, funcName, "Wait for visibility cond."));
    tvBaseLock (threeD); {
      TSMTRACE ((VIS_MASK|1, funcName, "freeze %d Q %d V %d",
								 threeD->freezeTiles,
								 threeD->redo_quad, threeD->redo_vislist));
      while (! (! threeD->freezeTiles
								&& (threeD->redo_quad || threeD->redo_vislist))) {
				PR_WaitCondVar (vis_cond, PR_INTERVAL_NO_TIMEOUT);
				if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR) {
					TSMTRACE ((VIS_MASK|1, funcName, "Interrupted."));
					tvBaseUnlock (threeD);
					TSMTRACE ((VIS_MASK|1, funcName, "Exiting thread"));
					return;
				}
				TSMTRACE ((VIS_MASK|1, funcName, "freeze %d Q %d V %d",
									 threeD->freezeTiles,
									 threeD->redo_quad, threeD->redo_vislist));
      }
      tvBaseGetView_nolock (threeD, &curr_view);
    }; tvBaseUnlock (threeD);

    /* GET TIME FOR LOGGING */
    PR_Lock (threeD->visible_lock); {

			startTime = PR_IntervalNow();

			numTiles = tvBaseGenerateVisible (threeD, &curr_view, &draw_count);

			endTime = PR_IntervalNow();

    }; PR_Unlock (threeD->visible_lock);

    TSMTRACE ((VIS_MASK|1, funcName, "Visibility done."));

    if (numTiles != -1 && TsIsLogging(TV_VISIBLE_LOG))
      TsLogVisibleData(startTime, endTime, numTiles);

    /* Mandatory pause.  Timing dependent on cycle_min and cycle_max. */
    {
      double elapsed;
      unsigned long pause;
      int cycle_min = visible_cycle_min;
      int cycle_max = visible_cycle_max;

      if ((elapsed = tsmElapsedTime (startTime, endTime)) < cycle_max) {
	int cycle_left = cycle_max-(int)(1000*elapsed);
	if (cycle_min > cycle_left) cycle_left = cycle_min;
	pause = cycle_left;
      } else
	pause = cycle_min;
      TSMTRACE ((VIS_MASK|1, funcName, "Elapsed %g msec: Pause %d msec", 
		 1000*elapsed, pause));
      tsmPause_ms (pause);
      if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR) {
	/* This is the signal to exit. */
	TSMTRACE ((VIS_MASK|1, funcName, "Interrupted.  Exiting thread"));
	if (vis_out) fclose (vis_out);
	return;
      }
    }

  }
}
#endif

/* This is a set of six linear inequalities designed to check view volume
   intersection.  Each of eqn[0] - eqn[5] is a plane descriptor [a,b,c,d] such
   that (ax+by+cz+d < 0) is true for the valid half-plane.  The intersection
   of all of these half-planes is the view volume.  Thus any point can be
   tested for inclusion in the view volume with a maximum of six vector inner
   product calculations (with half-plane complexity reductions for each eqn in
   sequence!). Left out far plane clip equation (0, 0, 1) for culling purposes.
   This essentially gives an infinite farplane.

   Given that the projection matrix M projects the vector p = [x,y,z,1] to
   p' = [x',y',z',w] by the equation p' = Mp, then the view volume is defined
   by the equations -w < x' < w, -w < y' < w, -w < z' < w.  Each of these is
   expressible by a linear inequality qp' < 0 (e.g. q = [-1,0,0,-1]
   corresponds to the inequality -w < x').  Since qMp < 0 can be expressed as
   (qM)p < 0, we can precalculate eqn[i] = q[i]M for each of the six
   inequalities.

   Now, if we wish to check the intersection of a sphere of radius r centered
   at p against our view volume, we will need to normalize each eqn[i] so that
   eqn[i].p is the signed distance from p to the plane repesented by 
   eqn[i].x = 0. Once this is done, then it is easy to see that eqn[i] +
   [0,0,0,r] represents a plane parallel to and a distance of r from eqn[i].
   We can then check for the intersection of each half-plane with the sphere
   (p,r) by simply adding the [0,0,0,r] vector to each eqn[i] before checking
   p's inclusion in the view volume! */

typedef struct {
  float eqn[6][4];
} ClipEquations;

static void
FillClipEquations (const_GLmatrix viewMatrix, ClipEquations *clip)
{
  static char const funcName[] = "FillClipEquations";
  static ClipEquations canon = {{
    { 1, 0, 0, -1 }, { -1, 0, 0, -1 },
    { 0, 1, 0, -1 }, { 0, -1, 0, -1 },
    { 0, 0, -1, -1 } }};
  int i, j;

  /* Transform the canonical equations by the view matrix. */
  for (i = 0; i < 5; ++i) {
    float *cq = canon.eqn[i];
    float *eqn = clip->eqn[i];
    float sum = 0;
    for (j = 0; j < 4; ++j) {
      eqn[j] = (cq[0] * viewMatrix[j][0]
		+ cq[1] * viewMatrix[j][1]
		+ cq[2] * viewMatrix[j][2]
		+ cq[3] * viewMatrix[j][3]);
      if (j < 3) sum += eqn[j]*eqn[j];
    }

    TSMTRACE ((VIS_MASK|4, funcName, "eqn[%d] = [%f,%f,%f,%f]",
	       i, eqn[0], eqn[1], eqn[2], eqn[3]));

    /* Normalize eqn so that inner products give signed distances. */
#ifdef __sgi
    sum = sqrtf (sum);
#else
    sum = sqrt (sum);
#endif
    for (j = 0; j < 4; ++j)
      eqn[j] /= sum;
    TSMTRACE ((VIS_MASK|4, funcName, "eqn[%d] = [%f,%f,%f,%f]",
	       i, eqn[0], eqn[1], eqn[2], eqn[3]));
  }
}

static int
ClipBoundingSphere (ClipEquations const *clip,
		    float const center[3], float r)
{
  static const char funcName[] = "ClipBoundingSphere";
  int i;
  float x = center[0], y = center[1], z = center[2];

  TSMTRACE ((VIS_MASK|3, funcName, "C=(%f,%f,%f) R=%f", x, y, z, r));
  for (i = 0; i < 5; ++i) {  /* don't do far plane clipping for culling */
    float const *eqn = clip->eqn[i];
    float inner = x*eqn[0] + y*eqn[1] + z*eqn[2] + (eqn[3] - r);
    if (inner > 0) {
      TSMTRACE ((VIS_MASK|3, funcName, "[%d] C . %f,%f,%f,%f = %f > 0",
		 i, eqn[0], eqn[1], eqn[2], (eqn[3]-r), inner));
      return FALSE;
    }
  }
  
  return TRUE;
}

typedef struct {
  tvBase *threeD;
  float nearPlane;
  float eye[3];
  GLmatrix viewMatrix;
  GLmatrix projMatrix;
  GLmatrix combMatrix;
  ClipEquations eqns;
  float horizon_d2;
  float d_scale;
  float *pixel_diam;
  int backfaceCull;
  int horizonCull;

  /* The following fields are used by ThreeDNearFar */
  float nearDist2;
  float nearPoint[3];
  TsTile const *nearTile;
  int inside;

  float farDist2;
  float farPoint[3];
  TsTile const *farTile;
} VisibleClosure;

static int
projectBox (float const *min_in, float const *max_in,
	    const_GLmatrix M,
	    const_GLmatrix P,
	    float min[3], float max[3])
{
  static char const funcName[] = "projectBox";
  float x0, x1, y0, y1, z0, z1;
  float v0[3], v1[3];

  v0[0] = min_in[0]; v1[0] = max_in[0];
  v0[1] = min_in[1]; v1[1] = max_in[1];
  v0[2] = min_in[2]; v1[2] = max_in[2];
  TSMTRACE ((VIS_MASK|3, funcName,
	     "x [%.0f:%.0f], y [%.0f:%.0f], z [%.0f:%.0f]",
	     v0[0], v1[0], v0[1], v1[1], v0[2], v1[2]));

  /* Do the view transform in a completely brute force fashion. This code is
     only enabled in order to check the fast version. */
  if (0 && TSMTRACEP (VIS_MASK|3)) {
    int i, j, k;
    float *v[2];
    float t[3], o[4];
    x0 = y0 = z0 =  FLT_MAX;
    x1 = y1 = z1 = -FLT_MAX;
    v[0] = v0; v[1] = v1;
    for (i = 0; i < 2; ++i) {
      t[0] = v[i][0];
      for (j = 0; j < 2; ++j) {
	t[1] = v[j][1];
	for (k = 0; k < 2; ++k) {
	  t[2] = v[k][2];
	  MultMatrixVec (M, t, o);
	  if (o[0] < x0) x0 = o[0];
	  if (o[0] > x1) x1 = o[0];
	  if (o[1] < y0) y0 = o[1];
	  if (o[1] > y1) y1 = o[1];
	  if (o[2] < z0) z0 = o[2];
	  if (o[2] > z1) z1 = o[2];
	}
      }
    }
    TSMTRACE ((VIS_MASK|3, funcName,
	       "x [%.2f:%.2f], y [%.2f:%.2f], z [%.2f:%.2f]",
	       x0, x1, y0, y1, z0, z1));
  }

  /* Transform by the view matrix.
     Fast BBox transform taken from Graphics Gems: Transforming Axis-Aligned
     Bounding Boxes pp548-550. */
  {
    float Tx = M[3][0], Ty = M[3][1], Tz = M[3][2];
    int j;
    x0 = x1 = Tx;
    for (j = 0; j < 3; ++j) {
      float a = M[j][0] * v0[j];
      float b = M[j][0] * v1[j];
      x0 += min (a, b);
      x1 += max (a, b);
    }
    y0 = y1 = Ty;
    for (j = 0; j < 3; ++j) {
      float a = M[j][1] * v0[j];
      float b = M[j][1] * v1[j];
      y0 += min (a, b);
      y1 += max (a, b);
    }
    z0 = z1 = Tz;
    for (j = 0; j < 3; ++j) {
      float a = M[j][2] * v0[j];
      float b = M[j][2] * v1[j];
      z0 += min (a, b);
      z1 += max (a, b);
    }
  }

  TSMTRACE ((VIS_MASK|3, funcName,
	     "x [%.2f:%.2f], y [%.2f:%.2f], z [%.2f:%.2f]",
	     x0, x1, y0, y1, z0, z1));

  /* If entire bbox is *behind* view plane, then fail immediately. */
  if (z0 >= 0)
    return 0;
  /* Clip bbox to front of view plane. */
  else if (z1 >= 0)
    z1 = -1e-22;

  /* Project by the projection matrix.  The decomposition of the projection
     matrix is as follows:

     P = [ p0  0 p1  0 ]
         [  0 p2 p3  0 ]
	 [  0  0 n0 n1 ]
	 [  0  0 -1  0 ]

     Assuming p0 and p2 are positive, we can find the bbox minmaxes by:

     min_x = p0 * min (-x0/z0, -x0/z1, -x1/z0, -x1/z1) - p1
     max_x = p0 * max (-x0/z0, -x0/z1, -x1/z0, -x1/z1) - p1

     min_y = p2 * min (-y1/z0, -y0/z0, -y0/z1, -y1/z1) - p3
     max_y = p2 * max (-y1/z0, -y0/z0, -y0/z1, -y1/z1) - p3

     min_z = min (-n1/z0, -n1/z1) - n0
     max_z = max (-n1/z0, -n1/z1) - n0

     Since we can shortcut after failure of any of the tests, we do the z
     projection first since it is simplest.
  */

  /* Do the projection in a completely brute force fashion. This code is only
     enabled in order to check the fast version. */
  if (0 && TSMTRACEP (VIS_MASK|3)) {
    int i, j, k, l;
    float v[2][3];
    float t[3], o[4];
    float px0, px1, py0, py1, pz0, pz1;
    px0 = py0 = pz0 =  FLT_MAX;
    px1 = py1 = pz1 = -FLT_MAX;
    v[0][0] = x0; v[0][1] = y0; v[0][2] = z0;
    v[1][0] = x1; v[1][1] = y1; v[1][2] = z1;
    for (i = 0; i < 2; ++i) {
      t[0] = v[i][0];
      for (j = 0; j < 2; ++j) {
	t[1] = v[j][1];
	for (k = 0; k < 2; ++k) {
	  t[2] = v[k][2];
	  MultMatrixVec (P, t, o);
	  for (l = 0; l < 3; ++l)
	    o[l] /= o[3];
	  if (o[0] < px0) px0 = o[0];
	  if (o[0] > px1) px1 = o[0];
	  if (o[1] < py0) py0 = o[1];
	  if (o[1] > py1) py1 = o[1];
	  if (o[2] < pz0) pz0 = o[2];
	  if (o[2] > pz1) pz1 = o[2];
	}
      }
    }
    TSMTRACE ((VIS_MASK|3, funcName,
	       "u [%.2f:%.2f], v [%.2f:%.2f], d [%.2f:%.2f]",
	       px0, px1, py0, py1, pz0, pz1));
  }

  {
    float p0 = P[0][0], p1 = P[1][0], p2 = P[1][1], p3 = P[2][1];
    float n0 = P[2][2], n1 = P[3][2];
    float t0, t1, t2, t3;
    float mn, mx;
    float sz0 = 1.0F/z0, sz1 = 1.0F/z1;

    assert (p0>0 && p2>0);

    t0 = -n1*sz0; t1 = -n1*sz1;
    mn = min (t0, t1);
    mx = max (t0, t1);
    z0 = mn - n0; z1 = mx - n0;
    TSMTRACE ((VIS_MASK|2, funcName, "D [%g:%g] %s", z0, z1,
	       (z1 < -1.0F) ? "no" : "yes"));
    /* far plane left out for culling purposes, only use near plane */    
    if (z1 < -1.0F)
      return 0;

    t0 = -x0*sz0; t1 = -x0*sz1; t2 = -x1*sz0; t3 = -x1*sz1;
    mn = min (t0, t1); mn = min (mn, t2); mn = min (mn, t3);
    mx = max (t0, t1); mx = max (mx, t2); mx = max (mx, t3);
    x0 = p0*mn - p1; x1 = p0*mx - p1;
    TSMTRACE ((VIS_MASK|2, funcName, "U [%g:%g] %s", x0, x1,
	       (x0 > 1.0F || x1 < -1.0F) ? "no" : "yes"));
    if (x0 > 1.0F || x1 < -1.0F)
      return 0;

    t0 = -y0*sz0; t1 = -y0*sz1; t2 = -y1*sz0; t3 = -y1*sz1;
    mn = min (t0, t1); mn = min (mn, t2); mn = min (mn, t3);
    mx = max (t0, t1); mx = max (mx, t2); mx = max (mx, t3);
    y0 = p2*mn - p3; y1 = p2*mx - p3;
    TSMTRACE ((VIS_MASK|2, funcName, "V [%g:%g] %s", y0, y1,
	       (y0 > 1.0F || y1 < -1.0F) ? "no" : "yes"));
    if (y0 > 1.0F || y1 < -1.0F)
      return 0;
  }

  min[0] = x0; max[0] = x1;
  min[1] = y0; max[1] = y1;
  min[2] = z0; max[2] = z1;
  return 1;
}

/* Called for every tile added to QuadTree.  Determines whether to update the
   estimates of the near/far planes. */

static void
ThreeDNearFar (TsTile const *tile,
	       float eye[3],
	       float *nearDist2, float nearPoint[3], TsTile const **nearTile,
	       float *farDist2, float farPoint[3], TsTile const **farTile,
	       int *inside)
{
  static char const funcName[] = "ThreeDNearFar";
  float const *min = tile->bbox.min, *max = tile->bbox.max;
  float const *corner[2];
  double near2 = FLT_MAX, far2 = -FLT_MAX;
  int near_i, far_i;

  TSMTRACE ((VIS_MASK|3, funcName, 
	     "[%d %d %d] %.1f,%.1f,%.1f - %.1f,%.1f,%.1f",
	     tile->id.x, tile->id.y, tile->id.res,
	     min[0], min[1], min[2], max[0], max[1], max[2]));
  TSMTRACE ((VIS_MASK|3, funcName, "eye %.1f,%.1f,%.1f",
	     eye[0], eye[1], eye[2]));

  /* Determine near and far points of this tile's bbox */
  {
    int i, j, k, idx;
    corner[0] = min; corner[1] = max;
    for (k = 0, idx = 0; k < 2; ++k) {
      float z = corner[k][2] - eye[2];
      for (j = 0; j < 2; ++j) {
	float y = corner[j][1] - eye[1];
	for (i = 0; i < 2; ++i, ++idx) {
	  float x = corner[i][0] - eye[0];
	  double d2 = x*x + y*y + z*z;
	  if (d2 < near2) near2 = d2, near_i = idx;
	  if (d2 >  far2)  far2 = d2,  far_i = idx;
	}
      }
    }
  }

  TSMTRACE ((VIS_MASK|3, funcName, "near %.0f,%.0f,%.0f dist %.0f",
	     ((1 & (near_i>>0)) ? max : min)[0],
	     ((1 & (near_i>>1)) ? max : min)[1],
	     ((1 & (near_i>>2)) ? max : min)[2],
	     sqrt (near2)));
  TSMTRACE ((VIS_MASK|3, funcName, "far  %.0f,%.0f,%.0f dist %.0f",
	     ((1 & (far_i>>0)) ? max : min)[0],
	     ((1 & (far_i>>1)) ? max : min)[1],
	     ((1 & (far_i>>2)) ? max : min)[2],
	     sqrt (far2)));

  /* Far point is simply upper bound of bbox distance */
  if (far2 > *farDist2) {
    *farDist2 = far2;
    *farTile = tile;
    farPoint[0] = ((1 & (far_i>>0)) ? max : min)[0];
    farPoint[1] = ((1 & (far_i>>1)) ? max : min)[1];
    farPoint[2] = ((1 & (far_i>>2)) ? max : min)[2];
    TSMTRACE ((VIS_MASK|2, funcName, "** update far  %.1f,%.1f,%.1f dist %.0f",
	       farPoint[0], farPoint[1], farPoint[2], sqrt (far2)));
  }

  /* The near calculation is somewhat more complicated.  First, we check if
     the eye point is inside or outside of the bbox.

     Outside: We use distance to box, which is either distance to corner or
     to a side, whichever is closer.  To determine corner or side, we
     need to determine whether the perpendicular projection of the eye
     point actually falls onto the side or not.

     Inside: At this point, we just punt and use the MIN_NEAR_PLANE distance
     and flag that we are inside a tile by setting insideTile.  In the
     future, we might optionally check the actual tile's data for
     proximity.
  */
  {
    /* Since we've determined that near_i is the index of the closest
       corner, we need only check the three sides attached to that corner.
       The locally axis-aligned bounding box alows us to make an
       inside/outside decision easily.
    */
    int idx_x = (1 & (near_i>>0));  /* 0 if min, 1 if max */
    float d_x = idx_x ? eye[0] - max[0] : min[0] - eye[0];
    int idx_y = (1 & (near_i>>1));  /* 0 if min, 1 if max */
    float d_y = idx_y ? eye[1] - max[1] : min[1] - eye[1];
    int idx_z = (1 & (near_i>>2));  /* 0 if min, 1 if max */
    float d_z = idx_z ? eye[2] - max[2] : min[2] - eye[2];
    float lnear[3];
    lnear[0] = idx_x ? max[0] : min[0];
    lnear[1] = idx_y ? max[1] : min[1];
    lnear[2] = idx_z ? max[2] : min[2];

    TSMTRACE ((VIS_MASK|3, funcName, "%d%d%d d = %.1f,%.1f,%.1f: %c%c%c",
	       idx_x, idx_y, idx_z, d_x, d_y, d_z,
	       d_x >= 0 ? 'o' : 'i', 
	       d_y >= 0 ? 'o' : 'i', 
	       d_z >= 0 ? 'o' : 'i'));
    if (d_x >= 0) {
      /* Outside of x == {min,max}[0] side. */
      if (d_y >= 0) {
	/* Outside of y == {min,max}[1] side. */
	if (d_z >= 0) {
	  /* ooo: */
	  /* Outside of z == {min,max}[2] side. */
	} else {
	  /* ooi: */
	  /* Inside of z == {min,max}[2] side. */
	  /* Distance is distance to XY edge. */
	  near2 = d_x*d_x + d_y*d_y;
	  lnear[2] = eye[2];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project XY near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], sqrt (near2)));
	}
      } else {
	/* Inside of y == {min,max}[1] side. */
	if (d_z >= 0) {
	  /* oio: */
	  /* Outside of z == {min,max}[2] side. */
	  /* Distance is distance to XZ edge. */
	  near2 = d_x*d_x + d_z*d_z;
	  lnear[1] = eye[1];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project XZ near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], sqrt (near2)));
	} else {
	  /* oii: */
	  /* Inside of z == {min,max}[2] side. */
	  /* Distance is distance to X side. */
	  near2 = d_x*d_x;
	  lnear[1] = eye[1];
	  lnear[2] = eye[2];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project X near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], fabs (d_x)));
	}
      }
    } else {
      /* Inside of x == {min,max}[0] side. */
      if (d_y >= 0) {
	/* Outside of y == {min,max}[1] side. */
	if (d_z >= 0) {
	  /* ioo: */
	  /* Outside of z == {min,max}[2] side. */
	  /* Distance is distance to YZ edge. */
	  near2 = d_y*d_y + d_z*d_z;
	  lnear[0] = eye[0];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project YZ near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], sqrt (near2)));
	} else {
	  /* ioi: */
	  /* Inside of z == {min,max}[2] side. */
	  /* Distance is distance to Y side. */
	  near2 = d_y*d_y;
	  lnear[0] = eye[0];
	  lnear[2] = eye[2];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project Y near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], fabs (d_y)));
	}
      } else {
	/* Inside of y == {min,max}[1] side. */
	if (d_z >= 0) {
	  /* iio: */
	  /* Outside of z == {min,max}[2] side. */
	  /* Distance is distance to Z side. */
	  near2 = d_z*d_z;
	  lnear[0] = eye[0];
	  lnear[1] = eye[1];
	  TSMTRACE ((VIS_MASK|3, funcName, 
		     "project Z near %.0f,%.0f,%.0f dist %.0f",
		     lnear[0], lnear[1], lnear[2], fabs (d_z)));
	} else {
	  /* iii: */
	  /* Inside of z == {min,max}[2] side. */
	  /* We are completely inside this one... */
	  near2 = MIN_NEAR_PLANE * MIN_NEAR_PLANE;
	  lnear[0] = eye[0];
	  lnear[1] = eye[1];
	  lnear[2] = eye[2];
	  *inside = TRUE;
	  TSMTRACE ((VIS_MASK|2, funcName, "<INSIDE>"));
	}
      }
    }

    /* We now know near2 and nearPoint. */
    /* Far point is simply upper bound of bbox distance */
    if (near2 < *nearDist2) {
      *nearDist2 = near2;
      *nearTile = tile;
      nearPoint[0] = lnear[0];
      nearPoint[1] = lnear[1];
      nearPoint[2] = lnear[2];
      TSMTRACE ((VIS_MASK|2, funcName,
		 "** update near %.0f,%.0f,%.0f dist %.0f",
		 lnear[0], lnear[1], lnear[2], sqrt (near2)));
    }
  }
}

static void
ThreeDNearFarCB (TsTile const *tile, TsGeoPyramid const *geo, int leaf, 
		 void *closure)
{
  VisibleClosure *vis = closure;
  if (leaf)
    ThreeDNearFar (tile, vis->eye,
		   &vis->nearDist2, vis->nearPoint, &vis->nearTile,
		   &vis->farDist2, vis->farPoint, &vis->farTile,
		   &vis->inside);
}

static int
tvBaseVisible (TsTile const *tile, TsGeoPyramid const *geo,
	       int check_vis, void *closure)
{
  static const char funcName[] = "tvBaseVisible";
  VisibleClosure *vis = closure;
  float min[3], max[3];
  float d, d_thresh;
  int prepared, is_sky;

  TSMTRACE ((VIS_MASK|3, funcName, "(%p [%d %d %d]) %dx%d",
	     tile, tile->id.x, tile->id.y, tile->id.res,
	     tile->pixels[0], tile->pixels[1]));

  is_sky = TsGeoIsSky( geo );

  if (check_vis
      && ! ClipBoundingSphere (&vis->eqns,
			       tile->bbox.center,
			       tile->bbox.radius))
    return VIS_FAIL;

  /* Prepare tile if available */
  prepared = TsCacheTilePrepared (tile);

  /* This is a new method based entirely on pixel size and distance. */
  {
#if 0
    /* This version attempts to adjust the pixel diameter based on elevation
       change per pixel.  However, if the elevation estimates are way too
       conservative then we end up with much too high a resolution being
       requested and displayed. Hmmm... */
    int px = tile->pixels[0];
    int py = tile->pixels[1];
    float e = tile->bbox.max[2] - tile->bbox.min[2];
    float e2 = (e*e)/(px*px + py*py);
    
    d_thresh = vis->d_scale * (vis->pixel_diam[tile->id.res] + sqrt (e2));

    TSMTRACE ((VIS_MASK|3, funcName, 
	       "e=%.0f p^2=%d e^2=%.1f diam=%.2f",
	       e, px*px + py*py, e2, vis->pixel_diam[tile->id.res],
	       vis->d_scale));
#else
    
    
    d_thresh = vis->d_scale * vis->pixel_diam[tile->id.res];
#endif

    /* This calculates the distance from the BBox center to the near plane of
       the current view frustum. */
    {
      float const *eqn = vis->eqns.eqn[4];
      float const *pt = tile->bbox.center;

      d = -(pt[0]*eqn[0] + pt[1]*eqn[1] + pt[2]*eqn[2] + eqn[3]);
    }

    TSMTRACE ((VIS_MASK|3, funcName, "d=%.0f d_scale=%.0f thresh=%.0f",
	       d, vis->d_scale, d_thresh));
    if (d > d_thresh ) {
      VISTR ((" %.*s%c %p[%d %d %d] dist %g%s > %g\n",
	      tile->id.res, "           ", 'O', 
	      tile->cache, tile->id.x, tile->id.y, tile->id.res,
	      d, prepared ? "" : "e", d_thresh));
      TSMTRACE ((VIS_MASK|2, funcName, "dist %g > %g? <= VIS_OVERRES",
		 d, d_thresh));
      return VIS_OVERRES;
    }
  }

  /* Check for over the horizon - no point doing this for sky datasets */
  if (vis->horizonCull && !is_sky) {
    float d2 = TsBBoxDist2 (&tile->bbox, vis->eye);
    if (d2 > vis->horizon_d2) {
      TSMTRACE ((VIS_MASK|2, funcName,
		 "%p [%d %d %d] horizon %.0f > %.0f <= VIS_FAIL",
		 tile, tile->id.x, tile->id.y, tile->id.res,
		 d2, vis->horizon_d2));
      return VIS_FAIL;
    }
  }

  /* Check for backfacing */

  /* XXX backface culling is currently broken for TsGeoPyramid_OI */
  /* datasets (those without a DEM) - let's just disable backface */
  /* culling for sky datasets as a result in the meantime. */

  if (vis->backfaceCull
      && prepared && is_sky == 0
      && TileBackfacing (tile, (v3f *) vis->eye)) {
    TSMTRACE ((VIS_MASK|2, funcName, "(%p [%d %d %d]) backfacing <= VIS_FAIL",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return VIS_FAIL;
  }

  /* Just cut out if we're not checking */
  if (! check_vis) {
    VISTR ((" %.*s%c %p[%d %d %d] dist %g%s > %g\n",
	    tile->id.res, "           ", 'V', 
	    tile->cache, tile->id.x, tile->id.y, tile->id.res,
	    d, prepared ? "" : "e", d_thresh));
    return VIS_VISIBLE | VIS_ALLIN;
  }

  /* If bounding box doesn't intersect viewing frustrum at all,
     don't keep subdividing. */
  if ( ! projectBox (tile->bbox.min, tile->bbox.max,
		    (const_GLmatrix) vis->viewMatrix,
		    (const_GLmatrix) vis->projMatrix,
		    min, max)) {
    TSMTRACE ((VIS_MASK|3, funcName, "(%p [%d %d %d]) outside <= VIS_FAIL",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    return VIS_FAIL;
  }

  if (min[0] >= -1.0 && max[0] <= 1.0
      && min[1] >= -1.0 && max[1] <= 1.0
      && min[2] >= -1.0 && max[2] <= 1.0) {
    TSMTRACE ((VIS_MASK|2, funcName, "(%p [%d %d %d]) <= VIS_ALLIN",
	       tile, tile->id.x, tile->id.y, tile->id.res));
    VISTR ((" %.*s%c %p[%d %d %d] dist %g%s > %g\n",
	    tile->id.res, "           ", 'V', 
	    tile->cache, tile->id.x, tile->id.y, tile->id.res,
	    d, prepared ? "" : "e", d_thresh));
    return VIS_VISIBLE | VIS_ALLIN;
  }

  TSMTRACE ((VIS_MASK|2, funcName, "(%p [%d %d %d]) <= VIS_VISIBLE",
	     tile, tile->id.x, tile->id.y, tile->id.res));
  VISTR ((" %.*s%c %p[%d %d %d] dist %g%s > %g\n",
	  tile->id.res, "           ", 'V', 
	  tile->cache, tile->id.x, tile->id.y, tile->id.res,
	  d, prepared ? "" : "e", d_thresh));
  return VIS_VISIBLE;
}


/*****************************************************************************
 * Synopsis:
 *	
 *
 * Description:
 *
 *
 * Externals:
 *
 *
 * Returns:
 *
 * Author:
 *		Nat Bletter
 * 
 * Date:
 *
 *****************************************************************************/
int
tvBaseGenerateVisible(tvBase * threeD, tvViewpoint const *view, int *count)
{
  static char const funcName[] = "tvBaseGenerateVisible";

  VisibleList *currList = threeD->workingList0;
  VisibleList *prevList = threeD->workingList0;
  list_iter_t gi;
  int max_tex;
  int redoQuad = threeD->redo_quad;
  int viewJumped = threeD->viewJumped;

  if (! threeD->mapped) {
    return 0;
  }

  if (redoQuad) {
    threeD->redo_quad = 0;
		TSMTRACE ((VIS_MASK|1, funcName, "Redoing quad"));
    tvBaseRegenerateQuads (threeD, view);

    if (! viewJumped)
      currList = threeD->workingList1;

    TSMTRACE ((TEX_MASK|VIS_MASK|2, funcName, "Clear list %p", currList));
    threeD->redo_vislist = 0;
		TSMTRACE ((VIS_MASK|1, funcName, "Redoing vislist"));
    ClearWorkList (threeD, currList);

    TSMTRACE ((VIS_MASK|2, funcName, "Begin parsing list %p", currList));
    currList->poppingStrategy = threeD->poppingStrategy;

    max_tex = (viewJumped ? 128 : 4);

    for (gi = list_first (threeD->geoList);
	 ! list_done (gi);
	 gi = list_next (gi)) {
      GeoElement *elem = list_value (gi);
      TsGeoPyramid *geo = elem->geo;
      TsGeoView *geoView = elem->geoView;
      QuadTile const *quad = elem->quad;

      if (quad) {
	v3f eye_s, *eye = 0;
	TSMTRACE ((VIS_MASK|1, funcName, "Add tiles for %s", TsGeoName (geo)));

	/* Initializing eye is a signal to do backface culling */
	if (threeD->backfaceCull) {

	  if ( TsGeoIsSky( geo ) )
	    TsGeoGCCtoLVCS (geo, 0.0, 0.0, 0.0, V(eye_s));
	  else
	    TsGeoGCCtoLVCS (geo, view->eye[0], view->eye[1], view->eye[2],
			    V(eye_s));
	  eye = &eye_s;
	}

	/* Get the relevant texture image if possible */
	if (use_subtexture && ! elem->texImage && threeD->tvGLX)
	  elem->texImage = GetTextureImage (threeD->tvGLX, geo);
	
	AddVisibleGeoList (currList, geoView, eye, quad, elem->texImage,
			   &max_tex);

	/* Accumulate the leaf count in all quads. */
	currList->maxTiles += quad->leaves;

	if (vis_out) {
	  fprintf (vis_out, "-- %s:\n", TsGeoName (geo));
	  PrintQuadTree (vis_out, quad);
	  fprintf (vis_out, "\n");
	}
      }
    }

  } else {
    /* Rerequest tiles if necessary */
    if (threeD->rerequest) {
      threeD->rerequest = FALSE;
      for (gi = list_first (threeD->geoList);
	   ! list_done (gi);
	   gi = list_next (gi)) {
	GeoElement *elem = list_value (gi);
	TsGeoView *geoView = elem->geoView;
	QuadTile const *quad = elem->quad;
    
	TSMTRACE ((VIS_MASK|1, funcName, "Rerequest tiles. QUAD %p", quad));
	threeD->rerequest |= 
	  (RequestVisible (geoView, (QuadTile *) quad, TSM_NOW) < 0);
	TsGeoViewSync (geoView);
      }
    }

    max_tex = 4;
  }

  if (currList != prevList) {
    /* Incrementally update currList given new quads and old view list */
    if (! viewJumped) {
      /* FIXME: Temporarily increase budget to paper over problems. */
      max_tex += 4;
      ApplyVisibleList (currList, threeD->treehash0, prevList, &max_tex);
    }

    /* Now swap so that workingList0 is always current */
    threeD->workingList0 = currList;
    threeD->workingList1 = prevList;

    ClearWorkList (threeD, prevList);
  }

  /* Do an incremental adjustment of visible list */
  threeD->redo_vislist = 0;
  AdjustVisibleList (currList, &max_tex);

  TSMTRACE ((VIS_MASK|1, funcName, "Using list %p:%d [%d]",
						 currList, currList->id, currList->ntiles));
  if (vis_out)
    PrintVisibleList (vis_out, currList);

  /* Create the next drawList from the workList. */
  {
    int currWorkList = threeD->currWorkList;
    VisibleDrawList * workList = &threeD->drawList[currWorkList];

    workList->id = ++*count;
    CreateDrawList (threeD, currList, workList, view);
    TSMTRACE ((VIS_MASK|1, funcName, "Set drawList %p:%d",
							 workList, workList->id));
    tvBaseNextDrawList (threeD);

    if (vis_out) {
      fprintf (vis_out, "VisibleDrawList %p:%d\n", workList, workList->id);
      fprintf (vis_out, "-----\n\n");
      fflush (vis_out);
    }
  }

  /* Redo this until max_tex >= 0 */
  if (max_tex <= 0
      || (currList->poppingStrategy == VisiblePopping_None && max_tex < 4))
    tvBaseRedoVisibleList (threeD, FALSE);

  return currList->n;
}

static void
ThreeDFreeVisParams (VisibilityParams *vis)
{
  VisibleClosure *closure = vis->closure;
  free (closure->pixel_diam);
  free (closure);
  free (vis);
}

static void
tvBaseRegenerateQuads (tvBase * threeD, tvViewpoint const * view)
{
  static char const funcName[] = "tvBaseRegenerateQuads";
  VisibleClosure *closure;
  VisibilityParams *visParams;
  istack *ist = threeD->is0;
  eqhash_t in_tree = threeD->treehash1;
  eqhash_t prev_tree = threeD->treehash0;
  float nearDist2 = FLT_MAX;
  double nearPoint[3];
  TsTile const *nearTile = 0;
  float farDist2 = -FLT_MAX;
  double farPoint[3];
  TsTile const *farTile = 0;
  int inside = FALSE;
  int ngeos = list_count (threeD->geoList);
  QuadTile **prev_quads = alloca (ngeos * sizeof (prev_quads[0]));
  int i;
  int numTiles = 0;
  list_iter_t gi;

  /* Remember old quads */
  for (gi = list_first (threeD->geoList), i = 0;
       ! list_done (gi);
       gi = list_next (gi), ++i) {
    GeoElement *elem = list_value (gi);
    prev_quads[i] = elem->quad;
  }
  istack_clear (ist);

  threeD->rerequest = FALSE;

  /* Forget previous tree */
  eqhash_clear (in_tree);

  for (gi = list_first (threeD->geoList);
       ! list_done (gi);
       gi = list_next (gi)) {
    GeoElement *elem = list_value (gi);
    TsGeoPyramid *geo = elem->geo;
    TsGeoView *geoView = elem->geoView;
    TsGeoInfo const *info = TsGeoGetInfo (geo);
    float tex_scale = pow (2.0, threeD->resAdjust);
    QuadTile *quad;

    closure = malloc (sizeof (*closure));
    closure->threeD = threeD;
    closure->nearPlane = view->nearPlane;
    closure->nearDist2 =  FLT_MAX;
    closure->farDist2  = -FLT_MAX;
    closure->inside = FALSE;
    closure->backfaceCull = threeD->backfaceCull;
    closure->horizonCull = threeD->horizonCull;

    TSMTRACE ((VIS_MASK|1, funcName, "\"%s\"", TsGeoName (geo)));

    if ( TsGeoIsSky( geo ) )
      TsGeoGCCtoLVCS (geo, 0.0, 0.0, 0.0, closure->eye);
    else
      TsGeoGCCtoLVCS (geo, view->eye[0], view->eye[1], view->eye[2],
		      closure->eye);

    TSMTRACE ((VIS_MASK|1, funcName, "fov %d aspect %g near %g far %g",
	       view->fov, view->aspect,
	       view->nearPlane, view->farPlane));
    TSMTRACE ((VIS_MASK|1, funcName, "eye %.0f,%.0f,%.0f",
	       view->eye[0], view->eye[1], view->eye[2]));
    TSMTRACE ((VIS_MASK|1, funcName, "roll %g pitch %g yaw %g",
	       view->roll, view->pitch, view->yaw));
    TSMTRACE ((VIS_MASK|2, funcName, "local eye %.1f,%.1f,%.1f",
	       closure->eye[0], closure->eye[1], closure->eye[2]));

    tvBaseGeoViewMatrix (threeD, geo,
			 closure->viewMatrix, 
			 closure->projMatrix, 
			 closure->combMatrix);
      
    TRACE_MATRIX (VIS_MASK|2, funcName, "viewMatrix", closure->viewMatrix);
    TRACE_MATRIX (VIS_MASK|2, funcName, "projMatrix", closure->projMatrix);
    TRACE_MATRIX (VIS_MASK|2, funcName, "combMatrix", closure->combMatrix);

    closure->pixel_diam = (float *)
      malloc (sizeof (float) * (info->maxLevel+1));
      
    /* Compute distance thresholds which can be used to quickly check for
       OVERRES tiles when max_px == max (tile->pixels[i]) */
    {
      int level;
      int scr_size = threeD->width;
      float tan_fov = tan (view->fov/2 * M_PI/180.0);
      float d_scale = scr_size/(2 * tex_scale * tan_fov);

      closure->d_scale = d_scale;
      for (level = info->minLevel; level <= info->maxLevel; ++level) {
	float pd = sqrt(TsGeoPixelDiameter2 (geo, level));
	closure->pixel_diam[level] = pd;
	TSMTRACE ((VIS_MASK|2, funcName, "dist[%d] = %.1f",
		   level, d_scale * pd));
      }
    }

    /* The horizon distance is simply calculated by noticing that the eye
     * point, center of the earth and a point on the horizon form a right
     * triangle. Thus, given the distance from the center of the earth to the
     * eye point (pc) and the polar radius of the earth (r), we can calculate
     * the distance to the horizon as d^2 = pc^2 - r^2.
     */

    /* no point doing horizon culling for sky datasets! because */
    /* we are on the inside of a sphere looking out. */

    if (threeD->horizonCull && ! TsGeoIsSky( geo ) ) {
      float polarRadius = info->earthRadius[1];
      float horizon_d2 = 0;
      int i;
      for (i = 0; i < 3; ++i)
	horizon_d2 += view->eye[i] * view->eye[i];
      horizon_d2 -= polarRadius*polarRadius;
      closure->horizon_d2 = horizon_d2;
    }

    FillClipEquations ((const_GLmatrix) closure->combMatrix, &closure->eqns);

    visParams = malloc (sizeof (*visParams));
    visParams->ist       = ist;
    visParams->in_tree   = in_tree;
    visParams->prev_tree = prev_tree;
    visParams->vis_f     = tvBaseVisible;
    visParams->add_f     = ThreeDNearFarCB;
    visParams->free_f    = ThreeDFreeVisParams;
    visParams->closure   = closure;

    elem->quad = quad = CalcVisibility (TsGeoTop (geo), geoView, visParams);
    numTiles = (quad ? quad->tree_size : 0);

    /* Update nearDist if necessary. */
    if (closure->nearDist2 < nearDist2) {
      float const *pt = closure->nearPoint;
      nearDist2 = closure->nearDist2;
      nearTile  = closure->nearTile;
      TsGeoLVCStoGCC (geo, pt[0], pt[1], pt[2], nearPoint);
      TSMTRACE ((VIS_MASK|1, funcName, "Near: %.0fm [%.0f,%.0f,%.0f]",
		 sqrt (nearDist2), 
		 nearPoint[0], nearPoint[1], nearPoint[2]));
      /* Update inside if necessary. */
      inside = closure->inside;
      if (inside)
	TSMTRACE ((VIS_MASK|1, funcName, "Inside tile [%d %d %d]",
		   nearTile->id.x, nearTile->id.y, nearTile->id.res)); 
    }

    /* Update farDist if necessary. */
    if (closure->farDist2 > farDist2) {
      float const *pt = closure->farPoint;
      farDist2 = closure->farDist2;
      farTile = closure->farTile;
      TsGeoLVCStoGCC (geo, pt[0], pt[1], pt[2], farPoint);
      TSMTRACE ((VIS_MASK|1, funcName, "Far : %.0fm [%.0f,%.0f,%.0f]",
		 sqrt (farDist2), 
		 farPoint[0], farPoint[1], farPoint[2]));
    }

    if (quad) {
      threeD->rerequest |= (RequestVisible (geoView, quad, TSM_NOW) < 0);
      TsGeoViewSync (geoView);
    }

    TSMTRACE ((VIS_MASK|1, funcName, "Done with visibility. QUAD %p[%d]",
	       quad, numTiles));
  }

  /* Clear the previous quad trees.  This is deferred so that we're not
     dereferencing tiles to zero and then immediately rereferencing them */
  for (i = 0; i < ngeos; ++i)
    if (prev_quads[i])
      QuadTreeDeref (prev_quads[i]);
    
  /* Now exchange stacks and trees. */
  {
    istack *is0 = threeD->is0;
    threeD->is0 = threeD->is1;
    threeD->is1 = is0;
  }
  {
    eqhash_t treehash0 = threeD->treehash0;
    threeD->treehash0 = threeD->treehash1;
    threeD->treehash1 = treehash0;
  }

  /* We now have nearDist/farDist etc. Update threeD.
   * This should be done locked so we can change all of these together.
   */
  tvBaseLock (threeD); {
    threeD->computedNearFar = TRUE;

    memcpy (threeD->nearPoint, nearPoint, 3 * sizeof (nearPoint[0]));
    threeD->nearTile = nearTile;
    threeD->inside = inside;

    memcpy (threeD->farPoint, farPoint, 3 * sizeof (farPoint[0]));
    threeD->farTile = farTile;
  }; tvBaseUnlock (threeD);
}
