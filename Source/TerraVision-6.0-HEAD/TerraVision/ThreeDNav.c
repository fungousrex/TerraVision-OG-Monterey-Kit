/* -*- tab-width: 2; -*-

  Name: ThreeDNav.c

  Description: View-handling for a tvBase instance.

  Revision History:
      $Id: ThreeDNav.c,v 2.43.2.4 2002/10/31 05:26:36 reddy Exp $

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
#include "DIS.h"

#include <limits.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdarg.h>
#include <float.h>

#include <prerror.h>

#if !( defined(WIN32) && defined(MSVC) )
# include <values.h>
#endif

#ifdef NAVDEBUG
static FILE *navFile;
static void
navPrint (char const *fmt, ...)
{
  if (! navFile) {
    navFile = fopen ("tvnav.log", "w");
    setlinebuf (navFile);
  }

  va_list ap;
  va_start (ap, fmt);
  vfprintf (navFile, fmt, ap);
  va_end (ap, fmt);

}
#else
static void
navPrint (char const *fmt, ...)
{
}
#endif

#define INIT_ALT (100.0)

/* RCS INFO */
static char const ThreeDNav_rcsid[] = "$Id: ThreeDNav.c,v 2.43.2.4 2002/10/31 05:26:36 reddy Exp $";

/* STATIC LOCAL FUNCTIONS */
static void     tvBaseCalcView(tvBase *, tvViewpoint *);


/*****************************************************************************
 * Synopsis:
 *	void tvBaseGetView_nolock (tvBase *, tvViewpoint *, int)
 *
 * Description: Copy the current view into the view buffer.
 *
 * Externals: NONE
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: 10/98
 *
 ****************************************************************************/
void
tvBaseGetView_nolock (tvBase * threeD, tvViewpoint *view)
{
  *view = threeD->view;
}

/*****************************************************************************
 * Synopsis:
 *	void tvBaseGetView(tvBase *, tvViewpoint *)
 *
 * Doc++ Description: *//**
 *   Retreive the current view.
 *
 * Externals: NONE
 *
 * @param threeD    handle to rendering state.
 * @param view          view struct, into which current view is copied.
 *
 * @return  NONE
 *
 * @author  Lee Iverson
 *
 * @version 10/98
 *
 * @see     tvViewpoint
 *
 ****************************************************************************/
void
tvBaseGetView (tvBase * threeD, tvViewpoint *view)
{
  static char const funcName[] = "tvBaseGetView";
  tvBaseLock (threeD); {
    tvBaseGetView_nolock (threeD, view);
  }; tvBaseUnlock (threeD);
}

/*****************************************************************************
 * Synopsis:
 *	void tvBaseSetView_nolock (tvBase *, tvViewpoint const *)
 *	void tvBaseSetView (tvBase *, tvViewpoint const *)
 *
 * Description: Copy the view buffer into the threeD's current view.
 *
 * Externals: NONE
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: 10/98
 *
 ****************************************************************************/
void
tvBaseSetView_nolock (tvBase * threeD,
		      tvViewpoint const *view, PRBool jump)
{
  static const char funcName[] = "tvBaseSetView_nolock";
  float prev_vel = threeD->view.vel;

  navPrint ("%p set: %.0f,%.0f,%.0f YPR: %.1f,%.1f,%.1f\n",
	    threeD, view->eye[0], view->eye[1], view->eye[2],
	    view->yaw, view->pitch, view->roll);

  if (view != &threeD->view) {
    TSMTRACE ((1, funcName, "eye %.0f,%.0f,%.0f",
	       view->eye[0], view->eye[1], view->eye[2]));
    TSMTRACE ((THREED_MASK|VIS_MASK|1, funcName, "roll %g pitch %g yaw %g",
	       view->roll, view->pitch, view->yaw));

    threeD->view = *view;
  }

  tvBaseCalcView (threeD, &threeD->view);

  /* Send an EntityState PDU whenever we move. */
  if (threeD->started && threeD->disSending)
    tvBaseSetDISView (threeD, (prev_vel != view->vel),
		      threeD->disForceID, threeD->disEntityID);

  threeD->viewJumped |= jump;
  tvBaseRedoVisibility (threeD, TRUE);
  tvBasePostRedraw (threeD, FALSE);
}

void
tvBaseSetView (tvBase * threeD, tvViewpoint const *view, PRBool jump)
{
  static char const funcName[] = "tvBaseSetView";
  list_iter_t i;

  tvBaseLock (threeD); {
    tvBaseSetView_nolock (threeD, view, jump);
  }; tvBaseUnlock (threeD);

  /* also change the viewpoints of move linked widgets */
  for (i = list_first (threeD->moveLinks);
       ! list_done (i);
       i = list_next (i)) {
    tvBase *other = list_value (i);
    tvBaseSetView (other, view, jump);
  }

  tvBaseUpdate (threeD, TV_MOVEMENT);
}

/*
 * Doc++ Description: *//**
 *   Causes the current view to be recalculated and rendered.
 *
 * @param threeD  handle to rendering state
 * @param jump    TRUE if this is not an incremental viewpoint change
 *
 * return  void
 *             
 */        
void
tvBaseUpdateView (tvBase * threeD, PRBool jump)
{
  tvViewpoint *view = &threeD->view;

  navPrint ("%p upd: %.0f,%.0f,%.0f YPR: %.1f,%.1f,%.1f\n",
	    threeD, view->eye[0], view->eye[1], view->eye[2],
	    view->yaw, view->pitch, view->roll);
  tvBaseSetView (threeD, view, jump);
}

/*
 * Doc++ Description: *//**
 *   Installs the "frozen view", associated with a rendering state, as the
 *   view to be displayed.
 *
 * @param threeD  handle to rendering state
 *
 * @return void
 *
 */
void
tvBaseSetFrozenView (tvBase * threeD)
{
  if (threeD->freezeTiles)
    tvBaseSetView (threeD, &threeD->freezeView, FALSE);
}

/*****************************************************************************
 * Synopsis:
 *	void tvBaseSetDISView (tvBase *threeD, int send_forced,
 *                             int forceID, int entityID)
 *
 * Description: Called when broadcasting EntityState PDUs from a viewer.
 *   Translates the current tvViewpoint into location, velocity and orientation
 *   parameters for tvBaseSendESPDU().
 *
 *   Ensures that unless send_forced is TRUE, we only send PDUs from this
 *   window every 1/2 second.
 *
 * Externals: NONE
 *
 * Returns: Nothing.
 *
 * Author: Lee Iverson
 *
 * Date: December 1999
 *
 ****************************************************************************/

void
tvBaseSetDISView (tvBase *threeD, int send_forced,
		  int forceID, int entityID)
{
  static char const funcName[] = "tvBaseSetDISView";
  PRTime now = PR_Now ();

  if (! send_forced) {
    static PRInt64 interval = LL_INIT (0, PR_USEC_PER_SEC/2);
    PRInt64 elapsed;

    LL_SUB (elapsed, now, threeD->disLastSend);
    send_forced = LL_CMP (elapsed, >, interval);
  }

  if (send_forced) {
    tvViewpoint const *view = &threeD->view;
    float speed = view->vel;
    v3f v;
    v3f o;

    /* Set velocity */
    if (speed == 0.0) {
      set_v3f (0, 0, 0, &v);
    } else {
      m4f const *viewMatrix = (m4f *) threeD->viewMatrix;
      tvHeadingForward (viewMatrix, &v, speed);
    }

    /* Set orientation: psi, theta, phi */
    set_v3f (view->yaw, view->pitch, view->roll, &o);

    TSMTRACE ((THREED_MASK|1, funcName, "SendESPDU"));
    tvBaseSendESPDU (threeD->disForceID, threeD->disEntityID,
			 view->eye, V(v), V(o));
    threeD->disLastSend = now;
  }
}

/*****************************************************************************
 * Synopsis:
 *	float tvBasePointElevation(tvBase *, double [3])
 *
 * Doc++ Description: *//**
 *   Determines the terrain elevation below the given point.
 *
 * Externals: NONE
 *
 * @param threeD  handle to rendering state
 * @param eye[3]      coordinates of reference point
 *
 * @return  The elevation, or 0 if cannot determine.
 *
 * @author  Martin Reddy
 *
 * @version May 2002
 *
 ****************************************************************************/

float
tvBasePointElevation( tvBase *threeD, double eye[3] )
{
  list_iter_t i;
  list_t *geoList = threeD->geoList;

  for (i = list_last (geoList); ! list_done (i); i = list_prev (i)) {
    GeoElement *elem = list_value (i);
    float uv[3], elev;

    if (TsGeoGCCtoUV (elem->geo, eye[0], eye[1], eye[2], uv)) {
      elev = TsGeoElevationUV (elem->geo, uv[0], uv[1]);
      if (elev != TS_BAD_ELEVATION)
        return elev;
    }
  }

  return 0.0f;
}

float
tvBasePointElevationGDC( tvBase *threeD, double gdc[3] )
{
  double gcc[3];
  if ( ! TsGDCtoGCC (gdc[0], gdc[1], gdc[2], gcc) )
    return 0.0f;
  return tvBasePointElevation( threeD, gcc );
}

/*****************************************************************************
 * Synopsis:
 *	float tvBasePointAltitude(tvBase *, double [3])
 *
 * Doc++ Description: *//**
 *   Determines the altitude above the terrain of the given point.
 *
 * Externals: NONE
 *
 * @param threeD  handle to rendering state
 * @param eye[3]      coordinates of reference point
 *
 * @return  The altitude.
 *
 * @author  Lee Iverson
 *
 * @version April 1999
 *
 ****************************************************************************/

float
tvBasePointAltitude (tvBase * threeD, double eye[3])
{
  static char const funcName[] = "tvBasePointAltitude";

  float altitude = FLT_MAX;
  list_iter_t i;
  list_t *geoList = threeD->geoList;

  for (i = list_last (geoList); ! list_done (i); i = list_prev (i)) {
    GeoElement *elem = list_value (i);
    float elev, uv[3];
    if (TsGeoGCCtoUV (elem->geo, eye[0], eye[1], eye[2], uv)) {
      elev = TsGeoElevationUV (elem->geo, uv[0], uv[1]);
      if (elev != TS_BAD_ELEVATION) {
	TSMTRACE ((THREED_MASK|2, funcName, "%s: UV %g,%g,%g ELEV %g",
		   TsGeoName (elem->geo), uv[0], uv[1], uv[2], elev));
	altitude = uv[2] - elev;
	break;
      }
    }
  }

  if (altitude == FLT_MAX) {
    double gdc[3];
    if (! TsGCCtoGDC (eye[0], eye[1], eye[2], gdc))
      abort ();
    TSMTRACE ((THREED_MASK|1, funcName, 
	       "  From %.0f,%.0f,%.0f [GDC %.3f,%.3f,%.0f]",
	       eye[0], eye[1], eye[2], gdc[0], gdc[1], gdc[2]));
    altitude = gdc[2];
  }

  TSMTRACE ((THREED_MASK|1, funcName, "  Altitude %.1f", altitude));

  return altitude;
}

/*****************************************************************************
 * Synopsis:
 *	void tvBaseConstrainView(tvBase *, tvViewpoint *)
 *
 * Description: We calculate the current view's altitude, ensuring that 
 *              we are not below ground.
 *
 * Externals: NONE
 *
 * Returns: NONE
 *
 * Author: Stephen Lau
 *
 * Date: 03/29/93
 *
 ****************************************************************************/

#define MIN_ALT (2.0)

static int
tvBaseConstrainView(tvBase * threeD, tvViewpoint * position)
{
  static char const funcName[] = "tvBaseConstrainView";
  double *eye = position->eye;

  int valid = FALSE;
  int constrained = FALSE;
  float altitude = FLT_MAX;
  list_iter_t i;

  /* If twoDConstrain is set, then keep the overhead view */    
  if (threeD->twoDConstrain) {
    threeD->view.pitch = -M_PI/2;
    threeD->view.yaw   = 0;
    threeD->view.roll  = 0;
    navPrint ("%p 2D: %.0f,%.0f,%.0f YPR: %.1f,%.1f,%.1f\n",
	      threeD, eye[0], eye[1], eye[2],
	      threeD->view.yaw, threeD->view.pitch, threeD->view.roll);
  }

  for (i = list_last (threeD->geoList);
       ! list_done (i);
       i = list_prev (i)) {
    GeoElement *elem = list_value (i);
    float elev, uv[3];
    if (! TsGeoGCCtoUV (elem->geo, eye[0], eye[1], eye[2], uv))
      continue;
    elev = TsGeoElevationUV (elem->geo, uv[0], uv[1]);
    if (elev == TS_BAD_ELEVATION)
      continue;

    TSMTRACE ((THREED_MASK|2, funcName, "%s: UV %g,%g,%g ELEV %g",
	       TsGeoName (elem->geo), uv[0], uv[1], uv[2], elev));
    if (uv[2] < elev + MIN_ALT) {
      TSMTRACE ((THREED_MASK|1, funcName, "Altitude %g < %g+%g",
		 uv[2], elev, MIN_ALT));
      TSMTRACE ((THREED_MASK|1, funcName, "  From %.0f,%.0f,%.0f",
		 eye[0], eye[1], eye[2]));
      if (TsGeoUVtoGCC (elem->geo, uv[0], uv[1], elev + MIN_ALT, eye)) {
	TSMTRACE ((THREED_MASK|1, funcName, "  To   %.0f,%.0f,%.0f",
		   eye[0], eye[1], eye[2]));
	constrained = TRUE;
	navPrint ("%p UV: %.0f,%.0f,%.0f (geo %p)\n",
		  threeD, eye[0], eye[1], eye[2], elem->geo);
      }
    }

    position->altitude = altitude = uv[2] - elev;
    ++valid;
    break;
  }

  /* If we're not over any data set, then we take elevation over MSL */
  if (! valid) {
    double gdc[3];
    if (! TsGCCtoGDC (eye[0], eye[1], eye[2], gdc))
      return FALSE;

    TSMTRACE ((THREED_MASK|2, funcName, "GDC %g,%g ELEV %g",
	       gdc[0], gdc[1], gdc[2]));
    position->altitude = altitude = gdc[2];
    if (altitude < MIN_ALT) {
      TSMTRACE ((THREED_MASK|1, funcName, "Altitude %g < %g",
		 altitude, MIN_ALT));
      position->altitude = altitude = MIN_ALT;
      TSMTRACE ((THREED_MASK|1, funcName,
		 "  From %.0f,%.0f,%.0f [GDC %.3f, %.3f, %.0f]",
		 eye[0], eye[1], eye[2], gdc[0], gdc[1], gdc[2]));
      if (TsGDCtoGCC (gdc[0], gdc[1], altitude, eye)) {
	TSMTRACE ((THREED_MASK|1, funcName,
		   "  To   %.0f,%.0f,%.0f", eye[0], eye[1], eye[2]));
	constrained = TRUE;
	navPrint ("%p UV: %.0f,%.0f,%.0f (ellipsoid)\n",
		  threeD, eye[0], eye[1], eye[2]);
      }
    }
  }

  /* limit the maximum altitude so that we are not able to continually */
  /* zoom out until we exceed the floating point precision and core. */

  if ( position->altitude > 3e8 ) {
    double gdc[3];
    position->altitude = 3e8;
    if ( TsGCCtoGDC( eye[0], eye[1], eye[2], gdc ) )
      TsGDCtoGCC( gdc[0], gdc[1], 3e8, eye );
  }

  TSMTRACE ((THREED_MASK|1, funcName, "  Altitude %.1f", position->altitude));

  return constrained;
}

/*****************************************************************************
 * Synopsis:
 *	void tvBaseCalcView(tvBase *, tvViewpoint *)
 *
 * Description: Based upon the viewer location, this functions calculates
 *    what point in 3 space the viewer is looking at.
 *    If we're following a Humvee, we look in that direction, without
 *    going below ground.
 *
 * Externals: NONE
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *	03/29/93
 *
 ****************************************************************************/

static void
tvBaseCalcView(tvBase * threeD, tvViewpoint * view)
{
  /*static char const funcName[] = "tvBaseCalcView";*/
  double const *eye = view->eye;

  tvBaseConstrainView (threeD, view);

  /* WRAP AROUND THE VIEW ANGLES IF NECESSARY */
  if (view->pitch <= -M_PI) {
    view->pitch += 2.0*M_PI;
  } else if (view->pitch > M_PI) {
    view->pitch -= 2.0*M_PI;
  }

  if (view->yaw <= -M_PI) {
    view->yaw += 2.0 * M_PI;
  } else if (view->yaw > M_PI) {
    view->yaw -= 2.0 * M_PI;
  }

  if (view->roll <= -M_PI) {
    view->roll += 2.0 * M_PI;
  } else if (view->roll > M_PI) {
    view->roll -= 2.0 * M_PI;
  }

  /* If we are too far from the reference point, then we reseat the reference
     origin and orientation */
  {
    int i;
    double *ref = view->reference;
    double d2;
    for (i = 0, d2 = 0; i < 3; ++i) {
      float d = eye[i] - ref[i];
      d2 += d*d;
    }

    /* following code suggested to be removed by Pete Tinker (Nov 28, 2000) */
    /* as it prevents straight-line movement beyond a tile. Removing this   */
    /* bad for us though - we need to find a cleaner way of doing this.     */

    if (d2 > 1e4) {
      memcpy (ref, eye, 3*sizeof(eye[0]));
      TsLVCSOrientation (ref, view->reference_plane);
      navPrint ("%p ref: %.0f,%.0f,%.0f [%.3f,%.3f,%.3f,%.3f]\n",
		threeD, ref[0], ref[1], ref[2],
		view->reference_plane[0],
		view->reference_plane[1],
		view->reference_plane[2],
		view->reference_plane[3]);
    }
  }
}


/*****************************************************************************
 * Synopsis:
 *	static void tvBaseInitView(tvBase *)
 *
 * Description: 
 *      Set the viewer viewpoint to an initial location.
 *      The initial location is based upon the dataset and always
 *      starts at "Xstart" "Ystart" of the data set.
 *
 * Externals: NONE
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *	03/29/93
 *
 *****************************************************************************/


/*
 * Doc++ Description: *//**
 *   Establishes the "initial view", which is retained as part of the state of
 *   a rendering thread.
 *
 * @param threeD   handle to rendering state
 * @param view     handle to a view specification
 *
 * @return  void
 *
 * @see     tvViewpoint
 *
 */
void
tvSetInitialView (tvBase *threeD, tvViewpoint const *view)
{
  static const char funcName[] = "tvSetInitialView";
  threeD->initView = view;
  TSMTRACE ((THREED_MASK|1, funcName, "%.0f,%.0f,%.0f\n",
	     view->eye[0], view->eye[1], view->eye[2]));
}

/*
 * Doc++ Description: *//**
 *   Initializes internal viewing parameters and draws an initial view. If a
 *   user-registered initial view has been set in the rendering state, that
 *   view is used.  Otherwise, a default view from the corner of the given
 *   geoset is used.  If the render-state is in 2D-constrain mode, an overhead
 *   view is constructed, positioned over the center of the geoset.
 *
 * @param threeD   handle to rendering state
 * @param geo          reference geoset
 *
 * @return void
 *
 * @see    TsGeoPyramid
 *
 */
void
tvBaseInitView(tvBase * threeD, TsGeoPyramid const *geo)
{
  static char const funcName[] = "tvBaseInitView";
  tvViewpoint pos, *position = &pos;

  if (geo == NULL) return;

  TSMTRACE ((THREED_MASK|VIS_MASK|1, funcName, ""));

  /* FOR THE ASPECT RATIO OF THE WINDOW... */

  tvBaseGetView (threeD, position);

  /* INITIALIZATION VALUES */
  position->fov = THREED_INIT_FOV;
  position->nearPlane = THREED_INIT_NEAR;
  position->farPlane = THREED_INIT_FAR;

  /* RESET THE VELOCITY AND ACCEL. */
  position->vel = 0.0;
  position->accel = 0.0;
  position->base_accel = 10;

  /* If an initial view has been set, then use it. */

  if ( threeD->initView && ! threeD->twoDConstrain ) {

    tvViewpoint const *init = threeD->initView;
    position->eye[0] = init->eye[0];
    position->eye[1] = init->eye[1];
    position->eye[2] = init->eye[2];
    position->yaw    = init->yaw;
    position->pitch  = init->pitch;
    position->roll   = init->roll;

    TSMTRACE ((THREED_MASK|1, funcName, "%.0f,%.0f,%.0f\n",
	       init->eye[0], init->eye[1], init->eye[2]));
  } else {

    /* otherwise, just put us overhead the dataset */

    TsGeoViewpoint( geo, threeD->width/(float)threeD->height, position->eye );
    TSMTRACE(( THREED_MASK|1, funcName, "Init overhead pos %.0f,%.0f,%.0f",
	       position->eye[0], position->eye[1], position->eye[2]));

    /* set the view to look straight down (for 3D viewer case) */

    position->pitch = -M_PI/2;
    position->yaw   = 0;
    position->roll  = 0;
  }

  /* stop any movement when we are reset to the initial viewpoint */

  tvStopMovement( threeD );

  /* FIGURE OUT WHAT POINT IN SPACE WE ARE LOOKING AT */
  threeD->computedNearFar = FALSE;
  threeD->viewValid = TRUE;
  tvBaseSetView (threeD, position, TRUE);

  tvBaseStatus (threeD, "Origin");
}


/*****************************************************************************
 * Synopsis:
 *	void tvBaseCentralView(tvBase *, TsGeoPyramid const* geo)
 *
 * Doc++ Description: *//**
 *      Puts the user at the midpoint of the data set facing down,
 *      above the data set.
 *
 * Externals: NONE
 *
 * @param threeD   handle to rendering state
 * @param geo          reference geoset
 *
 * @return   void
 *
 * @author   Nat Bletter
 *
 * @version  03/29/93
 *
 * @see      TsGeoPyramid
 *
 *****************************************************************************/
void
tvBaseCentralView(tvBase * threeD, TsGeoPyramid const *geo)
{
  static char const funcName[] = "tvBaseCentralView";

  tvViewpoint pos, *position = &pos;

  /* FOR THE ASPECT RATIO OF THE WINDOW... */
  tvBaseGetView (threeD, position);

  /* INITIALIZATION VALUES */
  position->fov   = 10;
  position->pitch = -M_PI/2;
  position->yaw   = 0;
  position->roll  = 0;

  /* RESET THE VELOCITY AND ACCEL. */
  position->vel = 0.0;
 
  /* PUT THE VIEWER AT THE CENTER OF THE DATA SET */
  {
    TsBBox const *bbox = TsGeoBBoxLVCS (geo);
    float u = bbox->center[0], v = bbox->center[1];
    float height = TsGeoElevationLVCS (geo, u, v, 0);
    TSMTRACE ((THREED_MASK|1, funcName, "Init pos %g,%g,%g (LVCS)",
	       u, v, height + 40000.0));
    TsGeoLVCStoGCC (geo, u, v, height + 40000.0, position->eye);
    TSMTRACE ((THREED_MASK|1, funcName, " = %.0f,%.0f,%.0f (GCC)",
	       position->eye[0], position->eye[1], position->eye[2]));
  }

  /* FIGURE OUT WHAT POINT IN SPACE WE ARE LOOKING AT */
  threeD->computedNearFar = FALSE;
  tvBaseSetView (threeD, position, TRUE);
}

/*
 * Synopsis:
 *   int tvAdjustView (tvBase *threeD, tvViewpoint *view,
 *                     double forward, double right, double up )
 *
 * Doc++ Description: *//**
 *   Moves the specified viewpoint in a viewer-based direction. This
 *   means moving forward/backward along the path that the viewer is
 *   facing, sliding left/right/up/down in a direction perpendicular
 *   to the forward direction.
 *
 * @param threeD   handle to rendering state
 * @param view         reference viewpoint
 * @param forward      delta to be applied in the "forward" direction
 * @param right        delta to be applied in the "right" direction
 * @param up           delta to be applied in the "up" direction
 *
 * @return   True if actually moved.
 *
 * @author   Lee Iverson
 *
 * @version  April 12 1999
 *
 * @see      tvViewpoint
 *
 */

int
tvAdjustView (tvBase *threeD, 
	      tvViewpoint *view,
	      double forward, double right, double up )
{
  static const char funcName[] = "tvAdjustView";
  int moved = 0;
  m4f const *viewMatrix = (m4f *) threeD->viewMatrix;
  v3f h;
  float const *heading = V(h);

  TSMTRACE ((THREED_MASK|1, funcName, "forw %g right %g up %g",
	     forward, right, up ));

  /* move the viewer in the forward/backward direction */
  if (forward != 0.0) {
    tvHeadingForward (viewMatrix, &h, forward);
    view->eye[0] += heading[0];
    view->eye[1] += heading[1];
    view->eye[2] += heading[2];
    navPrint ("%p fwd: %.0f,%.0f,%.0f (+ %.0f,%.0f,%.0f) [%.0fm]\n",
	      threeD, view->eye[0], view->eye[1], view->eye[2],
	      heading[0], heading[1], heading[2], forward);
    moved = 1;
  }

  /* move the viewer left/right */
  if (right != 0.0) {
    tvHeadingRight (viewMatrix, &h, right);
    view->eye[0] += heading[0];
    view->eye[1] += heading[1];
    view->eye[2] += heading[2];
    navPrint ("%p rgt: %.0f,%.0f,%.0f (+ %.0f,%.0f,%.0f) [%.0fm]\n",
	      threeD, view->eye[0], view->eye[1], view->eye[2],
	      heading[0], heading[1], heading[2], right);
    moved = 1;
  }

  /* move the viewer up/down */
  if (up != 0.0) {
    tvHeadingUp (viewMatrix, &h, up);
    view->eye[0] += heading[0];
    view->eye[1] += heading[1];
    view->eye[2] += heading[2];
    navPrint ("%p  up: %.0f,%.0f,%.0f (+ %.0f,%.0f,%.0f) [%.0fm]\n",
	      threeD, view->eye[0], view->eye[1], view->eye[2],
	      heading[0], heading[1], heading[2], up);
    moved = 1;
  }

  /* if ( isnand( view->eye[0] ) ) abort(); */

  return moved;
}


/* Height at which acceleration is defined, for height-scaled navigation
   (in meters) */ 
#define ACCEL_BASE_HEIGHT  100

/*
 *
 * Synopsis:
 *	void tvCurrentView (tvBase *threeD, tvViewpoint *viewp)
 *
 * Doc++ Description: *//**
 *     Move viewpoint to "current" position, defined by elapsed time
 *     and current forward speed.  Fills in viewp pointer if
 *     non-NULL.
 *
 * Externals: NONE - Just Say No to Globals
 *
 * @param threeD  handle to rendering state
 * @param viewp   pointer to a tvViewpoint struct, which is populated with the current
 *                viewpoint if not NULL.
 *
 * @return  void
 *
 * @author  Lee Iverson
 * 
 * @version 10/27/99
 *
 * @see     tvViewpoint
 *
 */
void
tvCurrentView (tvBase *threeD, tvViewpoint *viewp)
{
  static char const funcName[] = "tvCurrentView";

  PRIntervalTime prevTime = threeD->moveTime;

  tvBaseLock (threeD); {
    tvViewpoint *view = &threeD->view;
    double height_old = view->altitude;
    float speed = view->vel, accel = view->accel;
    int moved = FALSE;

    threeD->moveTime = PR_IntervalNow();

    /* Integrate speed and acceleration over time. */
    if (speed != 0.0 || accel != 0.0) {
      float dt = tsmElapsedTime (prevTime, threeD->moveTime);
      float dist = dt * (speed + (accel * dt)/2);
      float d_vel = accel * dt;

      navPrint ("%p tms: now %d prev %d\n",
		threeD, threeD->moveTime, prevTime);
      navPrint ("%p vel: %.0fm/s * %.2fs = %.0fm\n",
		threeD, speed, dt, dist);

      /* Protect against excessive intervals (> 10 seconds).
	 N.B. This can possibly happen with rollover of the interval time. */
      if (dt > 10) {
	TSMTRACE ((THREED_MASK|1, funcName, "Timer rollover: %d < %d",
		   threeD->moveTime, prevTime));
	dist = 0;
      }

      if (dist != 0.0) {
	if (tvAdjustView (threeD, view, dist, 0.0, 0.0)) {
	  moved = TRUE;
	  view->vel += d_vel;
	  TSMTRACE ((THREED_MASK|1, funcName, "speed + %.1f = %.1f", 
		     d_vel, view->vel));
	}
      }
    }

    /* Determine adjustment from deltas */
    if (threeD->delta_x != 0.0 || threeD->delta_y != 0.0) {
      float dx = threeD->delta_x;
      float dy = threeD->delta_y;
      switch (threeD->delta_motion) {
      case TVC_ATTITUDE:
        TSMTRACE ((THREED_MASK|1, funcName, "attitude by %.2f,%.2f", dx, dy));
        view->yaw   += (M_PI / 180.0) * -dx;
        view->pitch += (M_PI / 180.0) * -dy;
        break;
      case TVC_SLIDE:
        TSMTRACE ((THREED_MASK|1, funcName, "slide by %.2f,%.2f", dx, dy));
        if (threeD->heightBasedNav) {
          float scale = height_old / ACCEL_BASE_HEIGHT;
          if (threeD->twoDConstrain) 
	    tvAdjustView (threeD, view, 0,
			  dx * 3.0 * scale, 
			  dy * 3.0 * scale);
          else
	    tvAdjustView (threeD, view, 0,
			  dx * 5.0 * scale, 
			  dy * 5.0 * scale);
        } else {
          tvAdjustView (threeD, view, 0, dx * 10.0, dy * 10.0);
        }
	break;
      }
      moved = TRUE;
    }

    /* Now, if we moved, establish this as new viewpoint. */
    if (moved)
      tvBaseSetView_nolock (threeD, view, FALSE);
      
    /* Adjust speed for height */
    if (moved && threeD->heightBasedNav) {
      double height = view->altitude;
      if ( height_old != 0.0 ) {
	view->vel *= height / height_old;
	TSMTRACE ((THREED_MASK|1, funcName, "speed * %.3f = %.1f (was %g)", 
		   height / height_old, view->vel, height_old));
      }
    }

    /* Set return value if needed. */
    if (viewp)
      *viewp = *view;

  }; tvBaseUnlock (threeD);
}

/*
 *
 * Synopsis:
 *	void tvDeltaView (tvBase *threeD,
 *                        double forw, double right, double up,
 *                        tvViewpoint *viewp)
 *
 * Doc++ Description: *//**
 *   Adjust current viewpoint based on viewpoint-dependent axes.
 *
 * Externals: NONE - Just Say No to Globals
 *
 * ISSUE:  This is not presently called anywhere, but its recent version 
 *   number indicates it may be intended for the public interface.  Essentially
 *   similiar to "tvAdjustView()"  (upon which this is layered), but at a 
 *   higher level of service.
 *
 * @param threeD       handle to rendering state
 * @param forw         delta to be applied in the "forward" direction
 * @param right        delta to be applied in the "right" direction
 * @param up           delta to be applied in the "up" direction
 * @param viewp        reference viewpoint
 *
 * @return   void
 *
 * @author   Lee Iverson
 * 
 * @version  10/27/99
 *
 * @see      tvViewpoint
 *
 */
void
tvDeltaView (tvBase *threeD, double forw, double right, double up,
	     tvViewpoint *viewp)
{
  static char const funcName[] = "tvDeltaView";

  tvBaseLock (threeD); {
    tvViewpoint *view = &threeD->view;

    if (tvAdjustView (threeD, view, forw, right, up)) {
      threeD->moveTime = PR_IntervalNow();
      tvBaseSetView_nolock (threeD, view, FALSE);
    }

    /* Set return value if needed. */
    if (viewp)
      *viewp = *view;

  }; tvBaseUnlock (threeD);
}

/*
 *
 * Synopsis:
 *	void tvSetView (tvBase *threeD, tvViewpoint const *view)
 *
 * Doc++ Description:  *//**
 *   Sets the current view to that specified.
 *
 * Externals: NONE - Just Say No to Globals
 *
 * @param threeD   handle to rendering state
 * @param view     new viewpoint specification
 *
 * @return   void
 *
 * @author   Lee Iverson
 * 
 * @version  10/27/99
 *
 * @see      tvViewpoint
 *
 */
void
tvSetView (tvBase *threeD, tvViewpoint const *view, PRBool jump)
{
  static char const funcName[] = "tvSetView";

  tvBaseLock (threeD); {
    threeD->moveTime = PR_IntervalNow();
    tvBaseSetView_nolock (threeD, view, jump);
  }; tvBaseUnlock (threeD);
}

/*
 * Synopsis:
 *   void tvSetView3d (tvBase *threeD,
 *                     double x, double y, double z,
 *                     double yaw, double pitch, double roll)
 *
 * Doc++ Description: *//**
 *   This function will set the current viewpoint for the 3-D viewer.
 *   You pass in values for the (x,y,z) coordinate (where Y is up and 
 *   the units are meters), and the (yaw,pitch,roll) orientation (in
 *   units of radians).
 *
 * @param threeD   handle to rendering state
 * @param x        coordinate of eye point
 * @param y        "
 * @param z        "
 * @param yaw      orientation parameters for view
 * @param pitch    "
 * @param roll     "
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  20 January 1999
 *
 */

void
tvSetView3d( tvBase *threeD,
	     double x, double y, double z,
	     double yaw, double pitch, double roll )
{
  static char const funcName[] = "tvSetView3d";

  if ( threeD == NULL ) return;

  tvBaseLock (threeD); {
    tvViewpoint *view = &threeD->view;

    navPrint ("%p 3D: %.0f,%.0f,%.0f YPR %.1f,%.1f,%.1f\n",
	      threeD, x, y, z, yaw, pitch, roll);
    view->eye[0] = x;
    view->eye[1] = y;
    view->eye[2] = z;
    view->yaw    = yaw;
    view->pitch  = pitch;
    view->roll   = roll;
    view->vel    = 0.0;
    view->accel  = 0.0;

    threeD->moveTime = PR_IntervalNow();
    tvBaseSetView_nolock ( threeD, view, TRUE );
  }; tvBaseUnlock (threeD);
}

/*
 * Synopsis:
 *   void tvSetViewGDC3d( tvBase *threeD,
 *                        double lat, double long, double elev,
 *                        double yaw, double pitch, double roll)
 *
 * Description:
 *   This function will set the current viewpoint for the 3-D viewer.
 *   using GDC (latitude/longitude) coordinates. You pass in values
 *   for the (lat,long,elev) coordinate (where lat and long are in
 *   degrees and elev is elevation above the ellipsoid in meters),
 *   and the (yaw,pitch,roll) orientation (in units of radians).
 *
 *   The (yaw,pitch,roll) orientation is relative to the local 
 *   reference plane at the specified location, i.e. a value of
 *   (0,0,0) will always orient you so that you are upright and
 *   facing due north; and a value of (0,-PI/2,0) will always
 *   orient you so that you are facing straight down.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 10 June 1999
 *
 */

void
tvSetViewGDC3d( tvBase *threeD, 
		double lat, double lon,	double elev,
		double yaw, double pitch, double roll )
{
  static const char funcName[] = "tvSetViewGDC3d";
  double     gcc[3];

  if ( threeD == NULL ) return;

  /* convert the GDC coordinate into GCC */

  if ( TsGDCtoGCC( lon, lat, elev, gcc ) == FALSE ) {
    tsmError( funcName, "Error converting GDC->GCC (%f, %f, %f)",
	      lat, lon, elev );
    return;
  }

  tvBaseLock (threeD); {
    tvViewpoint *view = &threeD->view;
    
    /* set the new viewpoint position and orientation */

    navPrint ("%p GDC: %g,%g,%.0f\n", threeD, lat, lon, elev);
    navPrint ("%p GCC: %.0f,%.0f,%.0f YPR %.1f,%.1f,%.1f\n",
	      threeD, gcc[0], gcc[1], gcc[2], yaw, pitch, roll);
    view->eye[0] = gcc[0];
    view->eye[1] = gcc[1];
    view->eye[2] = gcc[2];
    view->vel    = 0.0;
    view->accel  = 0.0;
    view->yaw    = yaw;
    view->pitch  = pitch;
    view->roll   = roll;
      
    threeD->moveTime = PR_IntervalNow();

    /* work out the reference plane for this GCC point */

    if ( TsLVCSOrientation( gcc, view->reference_plane ) ) {
      
      view->reference[0] = gcc[0];
      view->reference[1] = gcc[1];
      view->reference[2] = gcc[2];
    }

    tvBaseSetView_nolock ( threeD, view, TRUE );

  }; tvBaseUnlock (threeD);
}

/*
 * Synopsis:
 *   void tvGetView3d( double*, double*, double*, double*, double*, double* )
 *
 * Description:
 *   This function will retrieve the current viewpoint for the 3-D viewer.
 *   The double values that are returned are in the same format as for
 *   tvSetView3d().
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

void
tvGetView3d( tvBase *threeD,
	     double *x, double *y, double *z,
	     double *yaw, double *pitch, double *roll )
{
  tvViewpoint view;

  if ( threeD == NULL ) return;

  tvBaseGetView( threeD, &view );

  *x     = view.eye[0];
  *y     = view.eye[1];
  *z     = view.eye[2];  
  *yaw   = view.yaw;
  *pitch = view.pitch;
  *roll  = view.roll;
}

/*
 * Synopsis:
 *   void tvGetViewGDC3d( tvBase *, double*, double*, double* )
 *
 * Description:
 *   This function will retrieve the current viewpoint for the 3-D viewer 
 *   in GDC (latitude/longitude) coordinates. Latitude and longitude are
 *   returned in units of degrees, and elevation is returned as elevation
 *   above the ellipsoid in units of meters.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 10 June 1999
 *
 */

void
tvGetViewGDC3d( tvBase *threeD, double *lat, double *lon, double *elev )
{
  static const char funcName[] = "tvGetViewGDC3d";
  tvViewpoint view;
  double gdc[3];

  if ( threeD == NULL ) return;

  tvBaseGetView( threeD, &view );

  /* convert the current GCC eye coordinate to GDC */

  if ( TsGCCtoGDC( view.eye[0], view.eye[1], view.eye[2], gdc ) == FALSE ) {
    tsmError( funcName, "Error converting GCC->GDC (%f, %f, %f)",
	      view.eye[0], view.eye[1], view.eye[2] );
    return;
  }

  /* return the GDC coordinate to the user */

  *lat  = gdc[0];
  *lon  = gdc[1];
  *elev = gdc[2];
}

/*
 * Synopsis: static void tvBaseAccelerate(void *)
 *
 * Description:  Timer function that accelerates the view's velocity and
 *               moves the current viewpoint.
 *               Since the timer expires when this function is called,
 *               the timer must be reset so this function will be called again.
 *
 * Externals: NONE
 *
 * Returns: NULL
 *
 * Author: Stephen Lau
 *
 * Date: April 8, 1993
 *
 */
static void
tvBaseAccelerate (void *closure)
{
  /*static char * funcName = "tvBaseAccelerate";*/
  tvBase * threeD = closure;

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  /* INCREASE THE VELOCITY BASED ON ACCELERATION */
  if (threeD->heightBasedNav)
    threeD->view.vel += (threeD->view.base_accel
			 * (threeD->view.altitude / ACCEL_BASE_HEIGHT));
  else
    threeD->view.vel += threeD->view.base_accel;

  /* RESET THE TIMER FOR ANOTHER ROUND */

  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseAccelerate, threeD);
}

/*
 * Doc++ Description: *//**
 *   Begins a "fly through" the view space, with the velocity increasing
 *   steadily over time.
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 */
void
tvStartAccelerate (tvBase *threeD)
{
  static const char funcName[] = "tvStartAccelerate"; 
  if ( threeD == NULL ) return;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  /* CLEAR OUT ANY OLD TIMERS */
  if (threeD->moveTimer)
    tvBaseDelTimer (threeD, &threeD->moveTimer);

  /* SET THE TIMER TO CALL THE FUNC "tvBaseAccelerate" */
  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseAccelerate, threeD);
}

/*
 * Synopsis: static void tvBaseDeccelerate(void *)
 *
 * Description: Timer function that deccelerates the view's velocity and
 *              moves the current viewpoint.
 *              Since the timer expires when this function is called,
 *              the timer must be reset so this function will be called again.
 *
 * Externals: NONE
 *
 * Returns: NULL
 *
 * Author: Stephen Lau
 * 
 * Date: April 8, 1993
 *
 */
static void
tvBaseDeccelerate(void *closure)
{
  /*static char * funcName = "tvBaseDeccelerate";*/
  tvBase * threeD = closure;

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  /* DECCELERATE BASED UPON THE ACCELERATION VALUE */
  if (threeD->heightBasedNav)
    threeD->view.vel -= (threeD->view.base_accel 
			 * (threeD->view.altitude / ACCEL_BASE_HEIGHT));
  else
    threeD->view.vel -= threeD->view.base_accel;

  /* RESET THE TIMER FOR ANOTHER ROUND */
  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseDeccelerate, threeD);
}

/*
 * Doc++ Description: *//**
 *   Begins to decelerate a current "fly through" the view space, thus 
 *   steadily decreasing the velocity.
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 */
void
tvStartDeccelerate (tvBase *threeD)
{
  static const char funcName[] = "tvStartDeccelerate"; 
  if ( threeD == NULL ) return;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  /* CLEAR OUT ANY OLD TIMERS */
  if (threeD->moveTimer)
    tvBaseDelTimer (threeD, &threeD->moveTimer);

  /* SET THE TIMER TO CALL THE FUNC "tvBaseAccelerate" */
  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseDeccelerate, threeD);
}

/*
 * Synopsis:
 * static void tvBaseMove(void *)
 *
 * Description:  Timer function that moves the current viewpoint based
 * upon the viewpoint velocity.
 * Since the timer expires when this function is called,
 * the timer must be reset so this function will be called again.
 *
 * Externals: NONE
 *
 * Returns: NULL
 *
 * Author: Stephen Lau
 * 
 * Date: April 9, 1993
 *
 */
static void
tvBaseMove(void *closure)
{
  /*static char const funcName[] = "tvBaseMove";*/
  tvBase * threeD = closure;

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  /* RESET THE TIMER FOR ANOTHER ROUND */
  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseMove, threeD);
}

/*
 * Doc++ Description: *//**
 *   Begins a "fly through" the view space, setting the motion to a constant
 *   velocity (ISSUE:  how is the velocity established?).
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 */
void
tvStartMovement (tvBase *threeD)
{
  static const char funcName[] = "tvStartMovement"; 
  if ( threeD == NULL ) return;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  /* MOVE THE VIEWPOINT */
  tvCurrentView (threeD, NULL);

  threeD->delta_x = 0;
  threeD->delta_y = 0;

  /* CLEAR OUT ANY OLD TIMERS */
  if (threeD->moveTimer)
    tvBaseDelTimer (threeD, &threeD->moveTimer);

  /* SET THE TIMER TO CALL THE FUNC "tvBaseAccelerate" */
  tvBaseAddTimer (threeD, &threeD->moveTimer, threeD->renderTime,
		  tvBaseMove, threeD);
}

/*
 * Synopsis:
 * void tvStopMovement( tvBase *threeD )
 *
 * Doc++ Description: *//**
 *   This function will stop any current camera animation that is currently
 *   playing in the 3-D viewer.
 *
 * @param threeD  handle to rendering state
 *
 * @return   void
 *
 * @author: Martin Reddy
 *
 * @version  20 January 19
 *
 */
void
tvStopMovement ( tvBase *threeD )
{
  static const char funcName[] = "tvStopMovement"; 
  if ( threeD == NULL ) return;

  TSMTRACE ((THREED_MASK|1, funcName, ""));

  /* If the animation timer for the 3-D viewer is set, then remove it */
  if ( threeD->moveTimer )
    tvBaseDelTimer (threeD, &threeD->moveTimer);
  
  threeD->delta_x = 0;
  threeD->delta_y = 0;
}

/*
 * Doc++ Description: *//**
 *   Sets the "slide" parameters for animated fly-throughs.  The slide 
 *   parameters are used to introduce motion in a direction that is not 
 *   parallel to the line of sight.  For example, one could simulate the
 *   view of looking out to the side during a fly through the geo space.
 *
 * @param threeD  handle to rendering state
 * @param dx      delta to be applied to the "right" direction.
 * @param dy      delta to be applied to the "up" direction
 *
 * @return void
 *
 */
void
tvBaseSetSlideDelta (tvBase *threeD, float dx, float dy)
{
  static const char funcName[] = "tvBaseSetSlideDelta";
  TSMTRACE ((THREED_MASK|1, funcName, "%.2f,%.2f", dx, dy));

  threeD->delta_motion = TVC_SLIDE;
  threeD->delta_x = dx;
  threeD->delta_y = dy;
}

/*
 * Doc++ Description: *//**
 *   Sets the "attitude" parameters for animated fly-throughs.  
 *   (ISSUE:  what is the precise effect of this? )
 *
 * @param threeD  handle to rendering state
 * @param dx      delta to be applied "yaw" of the view
 * @param dy      delta to be applied to the "pitch" of the view
 *
 * @return void
 *
 */
void
tvBaseSetAttitudeDelta (tvBase *threeD, float dx, float dy)
{
  static const char funcName[] = "tvBaseSetAttitudeDelta"; 
  TSMTRACE ((THREED_MASK|1, funcName, "%.2f,%.2f", dx, dy));
  
  threeD->delta_motion = TVC_ATTITUDE;
  threeD->delta_x = dx;
  threeD->delta_y = dy;
}
