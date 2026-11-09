/* -*- tab-width: 2; -*-

  Name: DotGeo.c

  Description: .geo discovery handling for a tvBase instance.

  Revision History:
      $Id: DotGeo.c,v 2.23.2.19 2002/10/31 05:26:29 reddy Exp $

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

#ifdef HAVE_LIBGEO
#include "TerraVisionInc.h"
#include "ThreeDCoreI.h"

#include <math.h>
#include <float.h>
#include <prerror.h>

#include <libGeo_DiscClient/DiscoverClientCAPI.h>
//#include <libGeo_DiscClient/PublicationClientCAPI.h>

static int tvDotGeoBBox(tvBase *threeD, TsBBox *gdc_bbox);
static int tvDotGeoBBoxQT(tvBase *threeD, TsBBox *gdc_bbox);
static int tvDotGeoBBox2D(tvBase *threeD, TsBBox *gdc_bbox);

static void tvLoadTerrainCB(void *generic);
static void tvLoadModelCB(void *generic);
static void tvDotGeoLoadTerrain(tvBase *threeD, char const *, char const *);
static void tvDotGeoLoadModel(tvBase *threeD, char const *url);
static void tvDotGeoLoadGeneric(tvBase *threeD, GeoDataElement const element);

static list_t *demList = NULL;

typedef struct 
{
  tvBase *threeD;        /* a handle to the 3D viewer's base struct */
  char   *url1;          /* the OI or VRML model URL */
  char   *url2;          /* the DEM URL */
} LoadClosure;

#ifdef MSVC
#pragma warning( disable : 4244 )	// automatic inline selected
#endif

/* macro to calculate whether bounding box b2 is completely within b1 */

#define BBOX_INSIDE(b1,b2) ( b2[0] >= b1[0] && b2[1] >= b1[1] && \
														 b2[2] <= b1[2] && b2[3] <= b1[3] )

/* macro to calculate the area of a bounding box in degrees^2 */

#define BBOX_AREA(element) ( ( element->north - element->south ) * ( element->east - element->west ) )

/*
* Synopsis: 
*   void tvBaseDotGeoThread(void *generic)
*
* Description:
*   This is the main loop for the .geo discovery thread. It basically
*   fires up every client - selectable number of seconds, works out
*   the extent of the world being viewed, and sends a request to the
*   .geo hierarchy in order to find out all of the metadata for that
*   region.
*
* @param  generic a(void*) that should be cast to(tvBase *)
* 
* @return void
*
* @author Martin Reddy
*
* @date 18 Sep 2000
*
*/
void
tvBaseDotGeoThread(void *generic)
{
  static char const funcName[] = "tvBaseDotGeoThread";
  tvBase            *threeD = (tvBase *) generic;

#ifdef HAVE_LIBGEO
	if ( demList == NULL ) {
		demList = list_create( TRUE );
	}

	TSMTRACE ((DOTGEO_MASK|1, funcName, ".geo discover thread executing"));

  while (1) {
		int elementCount = 0;

    // Wait until the core is ready for us to do something 
    if (tvBaseWaitRunnable(threeD) != PR_SUCCESS)
		{
      if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR)
				TSMTRACE ((DOTGEO_MASK|1, funcName, "Interrupted."));
      TSMTRACE ((DOTGEO_MASK|1, funcName, "Exiting thread"));
      return;
    }
		
    // request metadata from geo, if we are connected 
    PR_Lock (threeD->dotgeo_lock); {
      if (threeD->geoDiscClientHandle != NULL) {
        PRIntervalTime querySentTime, queryReceivedTime;
        TsBBox         gdc_bbox;

        tvDotGeoBBox( threeD, &gdc_bbox );
/*        
        if (TSMTRACEP(DOTGEO_MASK|1)) {
          if (! geoGetBoolean(threeD->geo, GEO_LOGGING_ENABLED)) {
					  geoSetBoolean(threeD->geo, GEO_LOGGING_ENABLED, TRUE);
					  geoSetString(threeD->geo, GEO_LOGGING_FILE, "tvgeo.log");
          }
				  geoSetInteger(threeD->geo, GEO_LOGGING_LEVEL, GEO_NOTIFY_DEBUG);
        } else {
				  geoSetBoolean(threeD->geo, GEO_LOGGING_ENABLED, FALSE);
        }
*/			
			  querySentTime = PR_IntervalNow();

				TSMTRACE ((DOTGEO_MASK|1, funcName, "Send query"));

				// temp cache list contains new added geodata element 

				// clear temp cache list
				GeoClearFetchedMetadataList( threeD->geoFetchMetadataHandle );

				// query dotgeo.
				GeoGetGeoDataByBBox( threeD->geoDiscClientHandle,
																				 gdc_bbox.min[0], gdc_bbox.min[1],
                                         gdc_bbox.max[0], gdc_bbox.max[1] );

				// get temp cache list 
				elementCount = GeoGetFetchedMetadataSize( threeD->geoFetchMetadataHandle );
			  
				queryReceivedTime = PR_IntervalNow();

        TSMTRACE((DOTGEO_MASK|1, funcName, "Query took %1.3lf seconds\n",
				  				tsmElapsedTime( querySentTime, queryReceivedTime ) ));
      }
    }; PR_Unlock (threeD->dotgeo_lock);
  
    if ( elementCount != 0 ) {
			int i;
			GeoDataElement element = NULL;
			list_t *oiList = list_create( FALSE );

			TSMTRACE ((DOTGEO_MASK|2, funcName, "List received"));
			TSMTRINC ((DOTGEO_MASK|2, 1));

			for ( i=0; i<elementCount; i++ ) { 
				element = GeoGetGeoDataElement( threeD->geoFetchMetadataHandle, i );

        TSMTRACE ((DOTGEO_MASK|2, funcName, "Examine %s:%s",
									 element->description, element->type));
				TSMTRACE ((DOTGEO_MASK|2, funcName, "[%.4f..%.4f, %.4f..%.4f]", 
									 element->west, element->south, element->east, element->north));
	
				/* go through each entry and try to load it */
				if (strcasecmp(element->type, "vrml") == 0) {
					/* if we have a(Geo)VRML model, then load it */
					tvDotGeoLoadModel(threeD, element->url);
				} else if (strcasecmp(element->type, "oi") == 0) {
					list_push( oiList, element );
				} else if (strcasecmp(element->type, "dem") == 0) {
					list_push( demList, element );
        } else {
					/* anything else, we create a 3D hyperlink icon for it */
					tvDotGeoLoadGeneric(threeD, element);
        }
			}

			/* GeoFreeGeoDataElement( element ); */

			/* check oi and dem list for matches */
			if	( !list_empty( oiList ) ) {
				list_iter_t i;
				list_iter_t j;

				for (i = list_first( oiList ); !list_done( i ); ) {
					GeoDataElement oiElement = (GeoDataElement)list_value( i );

					for (j = list_first( demList ); !list_done( j ); ) {
						GeoDataElement demElement = (GeoDataElement)list_value( j );

						if ( strcmp( oiElement->description, demElement->description) == 0 ) {
							/* add dataset to terravision */
							tvDotGeoLoadTerrain( threeD, oiElement->url, demElement->url );
						}
						j = list_next( j );
					}	

					i = list_next( i );
				}
			}

			list_destroy( oiList );
//			list_destroy( demList );

			for ( i=0; i<elementCount; i++ ) { 
				element = GeoGetGeoDataElement( threeD->geoFetchMetadataHandle, i );
				GeoFreeGeoDataElement( element ); 
			}

/*
				//					geoCacheList *tempList1 = NULL;
					char         *dem_url = NULL;
					double       oi_area, dem_area;

					oi_area   = BBOX_AREA( element );
					dem_area  = 0.0;
//					tempList1 = cacheList;

					// if we have an OI, try to find the associated DEM 
					while ( tempList1 != NULL ) {
						if (strcmp(tempList1->element.name, tempList->element.name) == 0 &&
								strcasecmp(tempList1->element.type, "dem") == 0) {
							dem_url = tempList1->element.url;
						}
						
						tempList1 = tempList1->next;
					}
*/
					// if we didn't find a DEM based upon name, try looking at extent 
/*
					if ( ! dem_url ) {
						tempList1 = cacheList;

						while ( tempList1 != NULL ) {
							if ( strcasecmp( tempList1->element.type, "dem" ) == 0 ) {

								// check to see if this OI is contained within the DEM 

								if ( BBOX_INSIDE( tempList1->element.bbox,
																	tempList->element.bbox ) ) {
									double this_area = BBOX_AREA( tempList1->element.bbox );

									if ( dem_area == 0.0 ) {

										// first DEM we found with a suitable extent 
										
										dem_url  = tempList1->element.url;
										dem_area = this_area;

									} else if ( this_area - oi_area < dem_area - oi_area ) {

										// another suitable DEM - choose the one with 
										// an area that is closest to that of the OI. 

										dem_url  = tempList1->element.url;
										dem_area = this_area;

									}
								}
							}
							tempList1 = tempList1->next;
						}
					}

					// if we still have not found a DEM, check our already loaded sets 

					if ( ! dem_url ) {
						list_iter_t i = list_first( geoList );
						double      *oi_bbox = tempList->element.bbox;

						for ( ; ! list_done(i); i = list_next(i) ) {
							TsGeoPyramid *geo = (TsGeoPyramid *) list_value( i );
							const TsBBox *bbox = ( geo ) ? TsGeoBBoxGDC( geo ) : NULL;
							if ( bbox ) {
								double dem_bbox[4];

								// put DEM bbox into the format used by libGeo 
								
								dem_bbox[0] = bbox->min[0];
								dem_bbox[1] = bbox->min[1];
								dem_bbox[2] = bbox->max[0];
								dem_bbox[3] = bbox->max[1];

								// is the OI inside this already loaded DEM? 
								
                if ( BBOX_INSIDE( dem_bbox, oi_bbox ) ) {
									TsmConnection connect[2];
									int           num;
									double        this_area = 0.0;
									char          *this_url = NULL;

									// get the URL for this DEM dataset 

									if ( (num = TsGeoConnections( geo, connect ) ) ) {
										while ( --num >= 0 ) {
											int type = tsmGet( connect[num], TSM_DATASET_TYPE );
											if ( type == TSM_DEM_DATASET )
												this_url = (char *) tsmGet( connect[num],
																										TSM_DATASET_URL );
										}
									}

									if ( ! this_url ) continue;

									// calculate the area for this DEM dataset 

									this_area = BBOX_AREA( dem_bbox );

									if ( dem_area == 0.0 ) {

										// first DEM we found with a suitable extent 
										
										dem_url  = this_url;
										dem_area = this_area;

									} else if ( this_area - oi_area < dem_area - oi_area ) {

										// another suitable DEM - choose the one with 
										// an area that is closest to that of the OI. 

										dem_url  = this_url;
										dem_area = this_area;
										
									}

								}

							}
						}
					}

					// if we have found a DEM for this OI, then load them 
					if ( dem_url )
						tvDotGeoLoadTerrain( threeD, tempList->element.url, dem_url );

				}

				GeoFreeGeoDataElement( element );
			}
*/
			TSMTRINC ((DOTGEO_MASK|2, -1));
		}
		
    // Mandatory pause. 
    
    //TSMTRACE((DOTGEO_MASK|1, funcName, "Pause %d ms", threeD->dotgeo_pause));
    tsmPause_ms(threeD->dotgeo_pause);
    
    // Did we receive the signal to exit this thread. 
		
    if (PR_GetError() == PR_PENDING_INTERRUPT_ERROR) {
      TSMTRACE((DOTGEO_MASK|1, funcName, "Interrupted.  Exiting thread"));
      return;
    }
  }
#endif /* HAVE_LIBGEO */
}

/*
* Synopsis: 
*   int tvDotGeoBBoxQT(tvBase *threeD, TsBBox *gdc_bbox)
*
* Description:
*   This function has the task of return a lat/long bounding box
*   for the visible area in the specified 3D viewer. This information
*   is calculated for us in GCC coordinates by the Visibility thread,
*   so we just need to take that latest value and convert to a GDC
*   bounding box(taking into consideration the earth's curvature!)
*
*   This function calculates the bounding box for the potentially
*   visible region by using the bounding box of all displayed tiles.
*   In essence, the currently loaded dataset(s) define an area of
*   interest that we restrict our calculation too. If we want to
*   search over the entire earth, then we would need to have a
*   global dataset loaded.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  gdc_bbox The returned bounding box of the visible extent in
*                  GDC(lat/long) coordinates
* 
* @return Always TRUE at the moment
*
* @author Martin Reddy
*
* @date 18 Sep 2000
*
*/
static int
tvDotGeoBBoxQT(tvBase *threeD, TsBBox *gdc_bbox)
{
  static char const funcName[] = "tvDotGeoBBox";
  double            gdc[3], gcc_min[3], gcc_max[3];
	
  /* get the GCC bounding box for the currently visible set of tiles */
	
  tvBaseLock(threeD); 
	{
    TsBBox *b = &threeD->workingList0->bbox;
    gcc_min[0] = (double) b->min[0];
    gcc_min[1] = (double) b->min[1];
    gcc_min[2] = (double) b->min[2];
    gcc_max[0] = (double) b->max[0];
    gcc_max[1] = (double) b->max[1];
    gcc_max[2] = (double) b->max[2];
  };
	tvBaseUnlock(threeD);
	
  /* now convert the GCC bounding box into GDC(lat/long) */
  /* to do this we need to project each corner into GDC   */
	
  TsBBoxInit(gdc_bbox);
	
  TsGCCtoGDC(gcc_min[0], gcc_min[1], gcc_min[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_min[0], gcc_min[1], gcc_max[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_min[0], gcc_max[1], gcc_min[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_min[0], gcc_max[1], gcc_max[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_max[0], gcc_min[1], gcc_min[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_max[0], gcc_min[1], gcc_max[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_max[0], gcc_max[1], gcc_min[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  TsGCCtoGDC(gcc_max[0], gcc_max[1], gcc_max[2], gdc);
  TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
	
  /* if we have a highly curved part of the earth, then */
  /* we need to same more heavily the GCC values. We do */
  /* this by sampling the mid points as well as min/max */
	
  if (gdc_bbox->max[0] - gdc_bbox->min[0] > 40.0)
	{
    double gcc_mid[3];
    
    gcc_mid[0] = gcc_min[0] +(gcc_max[0] - gcc_min[0]) / 2.0;
    gcc_mid[1] = gcc_min[1] +(gcc_max[1] - gcc_min[1]) / 2.0;
    gcc_mid[2] = gcc_min[2] +(gcc_max[2] - gcc_min[2]) / 2.0;
		
    TsGCCtoGDC(gcc_mid[0], gcc_min[1], gcc_min[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_min[1], gcc_mid[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_min[1], gcc_max[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_mid[1], gcc_min[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_mid[1], gcc_max[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_max[1], gcc_min[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_max[1], gcc_mid[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_mid[0], gcc_max[1], gcc_max[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_min[0], gcc_mid[1], gcc_min[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_min[0], gcc_mid[1], gcc_mid[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_min[0], gcc_mid[1], gcc_max[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_max[0], gcc_mid[1], gcc_min[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_max[0], gcc_mid[1], gcc_mid[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
		
    TsGCCtoGDC(gcc_max[0], gcc_mid[1], gcc_max[2], gdc);
    TsBBoxAddPoint(gdc_bbox, gdc[0], gdc[1], gdc[2]);
  }
	
  /* all done! Print a trace of the visible area in lat/long */
	
  TsBBoxFinish(gdc_bbox);  /* only need for center/radius - can ignore */
	
  TSMTRACE ((DOTGEO_MASK|1, funcName, "  Lat: %g..%g, Long: %g..%g (QT)",
						 gdc_bbox->min[1], gdc_bbox->max[1], gdc_bbox->min[0],
						 gdc_bbox->max[0]));
	
  return TRUE;
}

/*
* Synopsis: 
*   int tvDotGeoBBox2D(tvBase *threeD, TsBBox *gdc_bbox)
*
* Description:
*   This function has the task of return a lat/long bounding box
*   for the visible area in the specified 2D - constrained viewer.
*   That is, we assume that the camera is always facing straight
*   down to the surface of the planet. This makes that calculation
*   much simpler and so we can use this special case calculation
*   when the viewer is so constrained.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  gdc_bbox The returned bounding box of the visible extent in
*                  GDC(lat/long) coordinates
* 
* @return Always TRUE at the moment
*
* @author Martin Reddy
*
* @date 19 Sep 2000
*
*/
static int
tvDotGeoBBox2D(tvBase *threeD, TsBBox *gdc_bbox)
{
  static char const funcName[] = "tvDotGeoBBox2D";
  tvViewpoint       view;
  double            gdc[3], min_gdc[2], max_gdc[2];
  float             extent;
	
  /* take a copy of the current viewpoint for this viewer */
  
  tvBaseLock(threeD); 
	{
    view = threeD->view;
  };
	tvBaseUnlock(threeD);
	
  /* Convert the current viewpoint into GDC to get the */
  /* lat/long of the camera as it looks straight down  */
	
  TsGCCtoGDC(view.eye[0], view.eye[1], view.eye[2], gdc);
	
  /* start building the bounding box */
	
  TsBBoxInit(gdc_bbox);
	
  /* work out the half - extent of the view from the camera piercing point */
  /* we take into consideration the aspect ratio and field of view */
	
  extent = max(min(0.00000592 * gdc[2], 180.0), 0.00001);
  extent = (extent / 20.0) * view.fov * 0.5 * 0.75;
	
  /* calculate the min and max lat/long coordinates */
	
  min_gdc[0] = gdc[0] -(extent * view.aspect);
  max_gdc[0] = gdc[0] +(extent * view.aspect);
  min_gdc[1] = max(gdc[1] - extent, -90.0);
  max_gdc[1] = min(gdc[1] + extent, 90.0);
	
  /* add these to the bounding box structure */
	
  TsBBoxAddPoint(gdc_bbox, min_gdc[0], min_gdc[1], 0.0);
  TsBBoxAddPoint(gdc_bbox, max_gdc[0], max_gdc[1], 0.0);
	
  TsBBoxFinish(gdc_bbox);  /* only need for center/radius - can ignore */
	
  TSMTRACE ((DOTGEO_MASK|1, funcName, "%g..%g, %g..%g (2D)",
						 gdc_bbox->min[1], gdc_bbox->max[1], gdc_bbox->min[0],
						 gdc_bbox->max[0]));
	
  return TRUE;
}

/*
* Synopsis: 
*   int tvDotGeoBBox(tvBase *threeD, TsBBox *gdc_bbox)
*
* Description:
*   This function has the task of return a lat/long bounding box
*   for the visible area in the specified 3D viewer. Currently,
*   we use a specialized function for the 2D and 3D viewers. The
*   2D function uses a mathematical method assuming that the viewer
*   is always looking straight down. The 3D method uses the bounding
*   box of the currently visible terrain quadtree. The latter is
*   less robust because it requires some tiles to be loaded initially
*   and is constrained by the tile size of the coarsest dataset.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  gdc_bbox The returned bounding box of the visible extent in
*                  GDC(lat/long) coordinates
* 
* @return Always TRUE at the moment
*
* @author Martin Reddy
*
* @date 20 Sep 2000
*
*/
static int
tvDotGeoBBox(tvBase *threeD, TsBBox *gdc_bbox)
{
  /* If we have no sets loaded, then fake a whole - world view */
  
  if (list_empty(threeD->geoList))
	{
		TsBBoxInit(gdc_bbox);
		TsBBoxAddPoint(gdc_bbox, -180, -90, 0.0);
		TsBBoxAddPoint(gdc_bbox, 180, 90, 0.0);
		TsBBoxFinish(gdc_bbox);
		return TRUE;
  }
  
  /* otherwise, calculate bounding box from the viewpoint */
	
  if (threeD->twoDConstrain)
    return tvDotGeoBBox2D(threeD, gdc_bbox);
  else
    return tvDotGeoBBoxQT(threeD, gdc_bbox);
}

/*
* Synopsis: 
*   void tvLoadTerrainCB(void *generic)
*
* Description:
*   This is the callback which is used by tvDotGeoLoadTerrain() in
*   order to perform the actual loading of a TerraVision terrain model.
*
* @param  generic  The callback closure with everything we need to know
* 
* @return void
*
* @author Martin Reddy
*
* @date 20 Sep 2000
*
*/
static void
tvLoadTerrainCB(void *generic)
{
  LoadClosure  *c = (LoadClosure *) generic;
  TsGeoPyramid *geo;
	
  /* try to load the terrain dataset */
	
  tvBaseConsole( c->threeD, "Loading terrain (GeoWeb) - %s",
								 c->url1 ? c->url1 : "n/a" );

  if ((geo = TsGeoCreate_OIDEM(c->url1, NULL, c->url2, NULL))) {
    tvAddDataset(c->threeD, geo);
    /* output a message on the dataset console window */
    tvBaseConsole( c->threeD,"Loaded terrain succefully (GeoWeb) - %s from %s",
									 TsGeoName(geo), c->url1 );
    /* tell the GUI layer that we have loaded a dataset for it */
    tvBaseMessage( c->threeD, TV_TERRAIN_ADD_MSG, TsGeoName(geo) );
	} else
		tvBaseConsole( c->threeD, "Failed loading terrain (GeoWeb) - %s",
									 c->url1 ? c->url1 : "n/a" );
	
  /* free the closure structure when done */
	
  if (c->url1)
		tsmMemFree(c->url1);
  if (c->url2)
		tsmMemFree(c->url2);
  tsmMemFree(c);
}

/*
* Synopsis: 
*   void tvDotGeoLoadTerrain(tvBase *threeD, char const *, char const *)
*
* Description:
*   This function is called by the .geo discovery thread in order to
*   load a TerraVision terrain model. We need an OI and a DEM URL in
*   order to perform this. The actual dataset loading happens in a
*   newly - spawned thread, i.e. this function will return immediately.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  oi       A URL for the OI dataset
* @param  dem      A URL for the associated DEM dataset
* 
* @return void
*
* @author Martin Reddy
*
* @date 20 Sep 2000
*
*/
static void
tvDotGeoLoadTerrain(tvBase *threeD, char const *oi, char const *dem)
{
  static char const funcName[] = "tvDotGeoLoadTerrain";
  PRThread          *load_thread;
  LoadClosure       *closure;

  if ( oi == NULL || strlen(oi) == 0 )
    return;

  /* allocate a closure to tell our callback everything it needs */
	
  if ((closure = tsmMemMalloc(sizeof(LoadClosure))) == NULL)
	{
    tsmError(funcName, "Unable to create a LoadClosure for load process");
    return;
  }
	
  /* fill in the closure fields */
	
  closure->threeD = threeD;
  closure->url1   = tsmMemStrdup(oi);
  closure->url2   = tsmMemStrdup(dem);
	
  /* spawn a new thread to perform the load operation */
	
  if (pthr_create(&load_thread, "Load Terrain", tvLoadTerrainCB, closure))
	{
    tsmError(funcName, "Unable to spawn the terrain load process");
    tsmMemFree(closure);
    return;
  }
	
  /* create some debug output */
	
  TSMTRACE((DOTGEO_MASK|1, funcName, "Loading terrain from .geo discovery"));
  TSMTRACE((DOTGEO_MASK|1, funcName, "Imagery  : %s", oi));
  TSMTRACE((DOTGEO_MASK|1, funcName, "Elevation: %s", dem));
}

/*
* Synopsis: 
*   void tvLoadModelCB(void *generic)
*
* Description:
*   This is the callback which is used by tvDotGeoLoadModel() in
*   order to perform the actual loading of a GeoVRML model.
*
* @param  generic  The callback closure with everything we need to know
* 
* @return void
*
* @author Martin Reddy
*
* @date 20 Sep 2000
*
*/
static void
tvLoadModelCB(void *generic)
{
  LoadClosure *c = (LoadClosure *) generic;

  /* try to load the GeoVRML model */
	
  if ( FeatureRefCountLoad(c->threeD, c->url1) != NULL ) {

    /* tell the GUI layer that we have loaded a model for it */
    tvBaseMessage( c->threeD, TV_MODEL_ADD_MSG, c->url1 );

  } else {

    /* display a failure message on the dataset console.  */
    /* N.B. a success message is handled within Feature.c */

    tvBaseConsole( c->threeD, "Failed to load model (GeoWeb) - %s",
                   c->url1 );

  }

  /* free the closure structure when done */
	
  if (c->url1)
		tsmMemFree(c->url1);
  if (c->url2)
		tsmMemFree(c->url2);
  tsmMemFree(c);
}

/*
* Synopsis: 
*   void tvDotGeoLoadModel(tvBase *threeD, char const *url)
*
* Description:
*   This function is called by the .geo discovery thread in order to
*   load a GeoVRML model. We just need the URL to the GeoVRML file in
*   order to perform this. The actual scene loading happens in a
*   newly - spawned thread, i.e. this function will return immediately.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  url      The URL for the GeoVRML model
* 
* @return void
*
* @author Martin Reddy
*
* @date 20 Sep 2000
*
*/
static void
tvDotGeoLoadModel(tvBase *threeD, char const *url)
{
  static char const funcName[] = "tvDotGeoLoadModel";
  PRThread          *load_thread;
  LoadClosure       *closure;

  if ( url == NULL || strlen( url ) == 0 )
    return;

  /* allocate a closure to tell our callback everything it needs */
	
  if ((closure = tsmMemMalloc(sizeof(LoadClosure))) == NULL) {
    tsmError(funcName, "Unable to create a LoadClosure for load process");
    return;
  }
	
  /* fill in the closure fields */
	
  closure->threeD = threeD;
  closure->url1   = tsmMemStrdup(url);
  closure->url2   = NULL;
	
  /* spawn a new thread to perform the load operation */
	
  if (pthr_create(&load_thread, "Load Model", tvLoadModelCB, closure))
	{
    tsmError(funcName, "Unable to spawn the GeoVRML load process");
    tsmMemFree(closure);
    return;
  }
	
  /* create some debug output */
	
  TSMTRACE((DOTGEO_MASK|1, funcName, "Loading GeoVRML from .geo discovery"));
  TSMTRACE((DOTGEO_MASK|1, funcName, "URL : %s", url));
}

static const char *VrmlWrapper = "\
#VRML V2.0 utf8\n\
EXTERNPROTO GeoLocation [\n\
  field SFNode   geoOrigin\n\
  field MFString geoSystem\n\
  field SFString geoCoords\n\
  field MFNode   children\n\
] \"http://www.ai.sri.com/geovrml/1.0/protos/GeoLocation.wrl\"\n\
EXTERNPROTO GeoOrigin [\n\
  exposedField  MFString  geoSystem\n\
  exposedField  SFString  geoCoords\n\
] \"http://www.ai.sri.com/geovrml/1.0/protos/GeoOrigin.wrl\"\n\
GeoLocation {\n\
  geoSystem [ \"GDC\" ]\n\
";

static const char *VrmlDefault = "\
Shape {\n\
  appearance Appearance { material Material {\n\
    diffuseColor 1 0 0 } }\n\
  geometry Box { size 5000 5000 5000 }\n\
}\n\
";

/*
* Synopsis: 
*   void tvDotGeoLoadWebcam(tvBase *threeD, char const *url)
*
* Description:
*   This function is called by the .geo discovery thread in order to
*   load a Webcam (or potentially any other) URL. We do this by
*   creating a wrapper GeoVRML model with an Anchor node that points
*   to the actual URL. We then rely on a web browser to display the
*   media appropriately.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  url      The URL for the resource
* 
* @return void
*
* @author Martin Reddy
*
* @date 20 December 2001
*
*/
static void
tvDotGeoLoadGeneric(tvBase *threeD, GeoDataElement const element)
{
  static char const funcName[] = "tvDotGeoLoadWebcam";
	char *vrml = NULL, buffer[512];
  double  lat, lon;

  if ( element == NULL || element->url == NULL );

  tvBaseConsole( threeD, "Creating icon (GeoWeb) - %s", element->url );

	/* create the header part with all the EXTERNPROTOs and GeoLocation */
	vrml = tsmStringCat( vrml, VrmlWrapper );

	/* insert the coorect coordinates into the GeoLocation node */
	lat = element->south + ( element->north - element->south ) / 2.0;
	lon = element->west + ( element->east - element->west ) / 2.0;

  sprintf( buffer, "geoCoords \"%1.10lg %1.10lg +0\"\n", lat, lon );
	vrml = tsmStringCat( vrml, buffer );
	vrml = tsmStringCat( vrml, "geoOrigin DEF GEOORG GeoOrigin {\n" );
	vrml = tsmStringCat( vrml, "  geoSystem [ \"GDC\" ]\n  " );
	vrml = tsmStringCat( vrml, buffer );
	vrml = tsmStringCat( vrml, "}\n" );

	/* output the Anchor node to wrap the real geometry up in */
	vrml = tsmStringCat( vrml, "children Anchor {\n" );
  vrml = tsmStringCat( vrml, "  url \"" );
	vrml = tsmStringCat( vrml, element->url );
	vrml = tsmStringCat( vrml, "\"\n" );

  vrml = tsmStringCat( vrml, "  description \"" );
	if ( element->description )
		vrml = tsmStringCat( vrml, element->description );
	else if ( element->type )
		vrml = tsmStringCat( vrml, element->type );
	vrml = tsmStringCat( vrml, "\"\n" );

	vrml = tsmStringCat( vrml, "  children [\n" );

	/* output the actual vrml geometry here */

	vrml = tsmStringCat( vrml, VrmlDefault );

	/* output the trailing VRML to finish off the file */
	vrml = tsmStringCat( vrml, "  ]\n }\n}\n" );

  /* tvBaseConsole( threeD, "GeoVRML Icon = \n%s\n", vrml ); */

	/* okay, now we've build a GeoVRML wrapper for the URL. */
	/* let's parse it and all it to the scene */

  if ( FeatureRefCountParse( threeD, vrml, element->url ) ) {
    tvBaseMessage( threeD, TV_MODEL_ADD_MSG, element->url );
  } else {
    tvBaseConsole( threeD, "Could not create icon for %s", element->url );
  }

}

#endif
