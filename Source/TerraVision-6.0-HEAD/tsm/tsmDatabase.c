/*
 * Name: tsmDatabase.c
 *
 * Description:
 *   Contains high-level routines for manipulating dataset databases.
 *   This includes allocating/freeing database structures, reading
 *   database files, writing database files, updating database 
 *   structures with datasets on a server, etc.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Function List:
 *   TsmDataSetInst *tsmMakeDataSetInst( void )
 *   void tsmFreeDataSetInst( TsmDataSetInst *instance )
 *   TsmDataSet *tsmMakeDataSet( void )
 *   TsmGeoSet *tsmMakeGeoSet( void )
 *   void tsmFreeDataSetContents( TsmDataSet *dataset )
 *   void tsmFreeDataSet( TsmDataSet *dataset )
 *   void tsmFreeServer( TsmServer *server )
 *   void tsmFreeGeoSetContents( TsmGeoSet *geoset )
 *   void tsmFreeGeoSet( TsmGeoSet *geoset )
 *   void tsmAppendDataSet( xmlNodePtr root, TsmDataSet *dataset )
 *   void tsmAppendGeoSet( xmlNodePtr root, TsmGeoSet *geoset )
 *   void tsmAppendModel( xmlNodePtr root, TsmModel *model )
 *   void tsmAppendMap( xmlNodePtr root, TsmMap *map )
 *   void tsmAppendServer( xmlNodePtr root, TsmServer *server )
 *   void tsmAppendFootprint( xmNodePtr root, Footprint *foot )
 *   TsmDatabase *tsmMakeDatabase( void )
 *   void tsmFreeDatabase( TsmDatabase *database )
 *   int tsmPersistDatabase( TsmDatabase *database, char const *filename )
 *   int tsmAddDataSetToDatabase( TsmDatabase *database, TsmDataSet *dataset )
 *   TsmDatabase *tsmReadDatabase( char *location )
 *   int tsmWriteDatabase( TsmDatabase *database, char *location )
 *   int tsmRemoveServerSetsFromDatabase( TsmDatabase *, TsmServer *,
 *                                        NameUrlIds *)
 *   int tsmUpdateDatabaseFromServer( TsmDatabase *database, TsmServer *server)
 *   int tsmAddServerToDatabase( TsmDatabase *database, char *url ) 
 *   int tsmRemoveServerFromDatabase( TsmDatabase *database, char *url )
 *   TsmDataSet *tsmGetDataset( TsmDatabase *database, char *name )
 *
 * Dependencies:
 *
 * Revision History:
 *   December 1997 - Initial version by Martin Reddy
 *
 *   $Id: tsmDatabase.c,v 1.42.2.3 2002/03/14 05:42:10 reddy Exp $
 *
 * License:
 *   The contents of this file are subject to the Open Source Apache
 *   Software License Version 1.1 (the "License"); you may not use
 *   this file except in compliance with the License. You may obtain a
 *   copy of the License at http://www.tsmapi/license.shtml.
 *
 *   Software distributed under the License is distributed on an "AS IS"
 *   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *   License for the specific language governing rights and limitations
 *   under the License.
 *
 *   Portions are Copyright (c) SRI International, 1998-2000.
 * 
 */

#include "tsmInternal.h"

#include <libxml/tree.h>
#include <prprf.h>

typedef xmlChar const *xmlCharPtr;



static void tsmBuildGeosets( TsmDatabase *database );
static int tsmCompareGeoNames( char const *name1, char const *name2 );
static int tsmIsDatasetInDatabase( TsmDatabase const *, char const *,
				   char const *, int );
static int tsmRemoveMapFromDatabasePos( TsmDatabase *database, int i );
static int tsmRemoveModelFromDatabasePos ( TsmDatabase *database, int i );
void tsmRemoveServerSetsFromDatabase( TsmDatabase *, TsmServer *,
                                      NameUrlIds * );
int tsmIsFootprintInside( Footprint *inside, Footprint *outside );

/*
 * Synopsis:
 *   TsmDataSetInst *tsmMakeDataSetInst( void )
 *
 * Doc++ Description: *//** 
 *   Allocate a dataset instance structure.
 *      
 *   Allocate memory for a TsmDataSetInst structure, initialised with all 
 *   fields set to 0/NULL.
 *
 * @param    None
 *
 * @return   Pointer to allocated TsmDataSetInst structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmFreeDataSetInst, TsmDataSetInst
 *
 */

TsmDataSetInst *
tsmMakeDataSetInst( void )
{
  const char     *funcName = "tsmMakeDataSetInst";
  TsmDataSetInst *inst;

  inst = (TsmDataSetInst *) tsmMemCalloc( 1, sizeof(TsmDataSetInst) );

  if ( inst == NULL ) {
    tsmError( funcName, "Error allocating TsmDataSetInst" );
    return NULL;
  }
 
  return inst;
}

/*
 * Synopsis:
 *   void tsmFreeDataSetInst( TsmDataSetInst *instance )
 *
 * Doc++ Description: *//** 
 *   Free a data set instance structure.
 *
 *   Frees up any memory that has been allocated for the TsmDataSetInst
 *   structure. N.B. It does not deallocate the memory for the structure
 *   itself
 *
 * @param    instance the TsmDataSetInst structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmMakeDataSetInst, TsmDataSetInst
 *
 */

void
tsmFreeDataSetInst( TsmDataSetInst *instance )
{
  int i;

  if ( instance == NULL ) return;

  /* Free the location Purl */

  if ( instance->location != NULL )
    tsmFreePurl( instance->location );

  /* Free each server name string and the memory for the server array */

  for ( i = 0; i < instance->numServers; ++i )
    if ( instance->serverName[i] != NULL )
      tsmMemFree( instance->serverName[i] );

  if ( instance->serverName != NULL )
    tsmMemFree( instance->serverName );
}

/*
 * Synopsis:
 *   TsmDataSet *tsmMakeDataSet( void )
 *
 * Doc++ Description: *//** 
 *   Allocate a dataset structure.
 *      
 *   Allocate memory for a TsmDataSet structure, initialised with all 
 *   fields set to 0/NULL.
 *
 * @param    None
 *
 * @return   Pointer to allocated TsmDataSet structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmFreeDataSet, TsmDataSet
 *
 */

TsmDataSet *
tsmMakeDataSet( void )
{
  const char *funcName = "tsmMakeDataSet";
  TsmDataSet *dataset;

  dataset = (TsmDataSet *) tsmMemCalloc( 1, sizeof(TsmDataSet) );

  if ( dataset == NULL ) {
    tsmError( funcName, "Error allocating TsmDataSet" );
    return NULL;
  }
 
  return dataset;
}

/*
 * Synopsis:
 *   void tsmFreeDataSetContents( TsmDataSet *dataset )
 *
 * Description:
 *   Free a data set structure.
 *
 *   Frees up any memory that has been allocated for the TsmDataSet
 *   structure. N.B. it does not deallocate the memory for the TsmDataSet
 *   structure itself
 *
 * @param    dataset the TsmDataSet structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmMakeDataSet, TsmDataSet
 *
 */

void
tsmFreeDataSetContents( TsmDataSet *dataset )
{
  int i;

  if ( dataset == NULL ) return;

  /* Free the name, description, and source-type strings */

  if ( dataset->name != NULL )
    tsmMemFree( dataset->name );

  if ( dataset->description != NULL )
    tsmMemFree( dataset->description );

  if ( dataset->source_type != NULL )
    tsmMemFree( dataset->source_type );

  if ( dataset->geoname != NULL )
    tsmMemFree( dataset->geoname );

  /* Free the contents of each instance structure and then the array */

  for ( i = 0; i < dataset->numInstances; ++i ) 
    tsmFreeDataSetInst( &dataset->instance[i] );

  if ( dataset->instance != NULL )
    tsmMemFree( dataset->instance );
}

/*
 * Synopsis:
 *   void tsmFreeDataSet( TsmDataSet *dataset )
 *
 * Doc++ Description: *//** 
 *   Free a data set structure.
 *
 *   Frees up any memory that has been allocated for the TsmDataSet
 *   structure, and then deallocates the memory for the TsmDataSet
 *   structure itself.
 *
 * @param    dataset the TsmDataSet structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmMakeDataSet, TsmDataSet
 *
 */

void
tsmFreeDataSet( TsmDataSet *dataset )
{
  if ( dataset != NULL ) {
    tsmFreeDataSetContents( dataset );
    tsmMemFree( dataset );
  }
}

/*
 * Synopsis:
 *   TsmGeoSet *tsmMakeGeoSet( void )
 *
 * Doc++ Description: *//** 
 *   Allocate a geoset structure.
 *      
 *   Allocate memory for a TsmGeoSet structure, initialised with all 
 *   fields set to 0/NULL, and the planet field to TSM_EARTH.
 *
 * @param    None
 *
 * @return   Pointer to allocated TsmGeoSet structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  19 March 1998
 *
 * @see      tsmFreeGeoSet, TsmGeoSet
 *
 */

TsmGeoSet *
tsmMakeGeoSet( void )
{
  const char *funcName = "tsmMakeGeoSet";
  TsmGeoSet  *geoset;

  geoset = (TsmGeoSet *) tsmMemCalloc( 1, sizeof(TsmGeoSet) );

  if ( geoset == NULL ) {
    tsmError( funcName, "Error allocating TsmGeoSet" );
    return NULL;
  }

  geoset->planet = TSM_EARTH;
 
  return geoset;
}

/*
 * Synopsis:
 *   void tsmFreeGeoSetContents( TsmGeoSet *geoset )
 *
 * Description:
 *   Free a geoset structure.
 *
 *   Frees up any memory that has been allocated for the TsmGeoSet
 *   structure. N.B. it does not deallocate the memory for the TsmGeoSet
 *   structure itself
 *
 * @param    geoset the TsmGeoSet structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  19 March 1998
 *
 * @see      tsmMakeGeoSet, TsmGeoSet
 *
 */

void
tsmFreeGeoSetContents( TsmGeoSet *geoset )
{
  int n;

  if ( geoset == NULL ) return;

  /* Free the name strings */

  if ( geoset->name != NULL )
    tsmMemFree( geoset->name );

  if ( geoset->oi_name != NULL )
    tsmMemFree( geoset->oi_name );

  if ( geoset->dem_name != NULL )
    tsmMemFree( geoset->dem_name );

  if ( geoset->model_name != NULL ) {
    for ( n = 0; n < geoset->numModels; n++ )
      tsmMemFree( geoset->model_name[n] );
    tsmMemFree( geoset->model_name );
  }

  if ( geoset->map_name != NULL ) {
    for ( n = 0; n < geoset->numMaps; n++ )
      tsmMemFree( geoset->map_name[n] );
    tsmMemFree( geoset->map_name );
  }
}

/*
 * Synopsis:
 *   void tsmFreeGeoSet( TsmGeoSet *geoset )
 *
 * Doc++ Description: *//** 
 *   Free a geoset structure.
 *
 *   Frees up any memory that has been allocated for the TsmGeoSet
 *   structure, and then deallocates the memory for the TsmGeoSet
 *   structure itself.
 *
 * @param    dataset the TsmGeoSet structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  19 March 1998
 *
 * @see      tsmMakeGeoSet, TsmGeoSet
 *
 */

void
tsmFreeGeoSet( TsmGeoSet *geoset )
{
  if ( geoset != NULL ) {
    tsmFreeGeoSetContents( geoset );
    tsmMemFree( geoset );
  }
}

/*
 * Synopsis:
 *   void tsmFreeServer( TsmServer *server )
 *
 * Doc++ Description: *//** 
 *   Free a server structure.
 *
 *   Frees up any memory that has been allocated for the TsmServer
 *   structure. N.B. it does not deallocate the memory for the TsmServer
 *   structure itself
 *
 * @param    server the TsmServer structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  11 December 1997
 *
 * @see      tsmFreeDataSet, tsmFreeDatabase
 *
 */

void
tsmFreeServer( TsmServer *server )
{
  if ( server == NULL ) return;

  if ( server->location != NULL )
    tsmFreePurl( server->location );
}


/*
 * Synopsis:
 *   void tsmFreeMap( TsmMap *map )
 *
 * Doc++ Description: *//** 
 *   Free a map structure.
 *
 *   Frees up any memory that has been allocated for the TsmMap
 *   structure. N.B. it does not deallocate the memory for the TsmMap
 *   structure itself
 *
 * @param    map the TsmMap structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  5 March 1999
 *
 * @see      tsmFreeDatabase
 *
 */

void
tsmFreeMap( TsmMap *map )
{
  int n;

  if ( map == NULL ) return;

  if ( map->name != NULL )
    tsmMemFree( map->name );

  if ( map->geoname != NULL )
    tsmMemFree( map->geoname );

  if ( map->location != NULL ) {
    for ( n = 0; n < map->numLocations; n++ )
      tsmFreePurl( map->location[n] );
    tsmMemFree( map->location );
  }
}

/*
 * Synopsis:
 *   void tsmFreeModel( TsmModel *model )
 *
 * Doc++ Description: *//** 
 *   Free a model structure.
 *
 *   Frees up any memory that has been allocated for the TsmModel
 *   structure. N.B. it does not deallocate the memory for the TsmModel
 *   structure itself
 *
 * @param    model the TsmModel structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  5 March 1999
 *
 * @see      tsmFreeDatabase
 *
 */

void
tsmFreeModel( TsmModel *model )
{
  int n;

  if ( model == NULL ) return;

  if ( model->name != NULL )
    tsmMemFree( model->name );

  if ( model->geoname != NULL )
    tsmMemFree( model->geoname );

  if ( model->title != NULL )
    tsmMemFree( model->title );

  if ( model->description != NULL )
    tsmMemFree( model->description );

  if ( model->location != NULL ) {
    for ( n = 0; n < model->numLocations; n++ )
      tsmFreePurl( model->location[n] );
    tsmMemFree( model->location );
  }
}

/*
 * Synopsis:
 *   void tsmAppendFootprint( xmlNodePtr root, Footprint *foot )
 *
 * Doc++ Description: *//** 
 *   Appends a footprint to an XML tree.
 *
 *   This function will take a Footprint structure and will append a
 *   formatted version of this to the specified XML node.
 *   Only one footprint will be output, but which coordinate system
 *   this will be in is specified by the foot->coordsys field. The
 *   footprint tag will be a string of "latlongfootprint",
 *   "utmfootprint", or "geofootprint", depending upon coordsys.
 *
 * @param    root the XML node that we are appending to
 * @param    foot the Footprint structure to print out
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmAppendDataset
 *
 */

void
tsmAppendFootprint( xmlNodePtr root, Footprint *foot )
{
  LatLongFootprint *ll_foot;
  UTMFootprint     *utm_foot;
  GeoFootprint     *geo_foot;
  LVCSFootprint    *lvcs_foot;
  char             buffer[512];
  xmlNodePtr       child;

  switch ( foot->coordsys ) {
  case TSM_LATLONG_COORDSYS:

    ll_foot = &foot->latlong;
    child = xmlNewChild( root, NULL, "latlongfootprint", NULL );

    sprintf( buffer, "%1.12lg %1.12lg", ll_foot->c1.lat, ll_foot->c1.lon );
    xmlNewChild( child, NULL, "latlongcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg", ll_foot->c2.lat, ll_foot->c2.lon );
    xmlNewChild( child, NULL, "latlongcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg", ll_foot->c3.lat, ll_foot->c3.lon );
    xmlNewChild( child, NULL, "latlongcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg", ll_foot->c4.lat, ll_foot->c4.lon );
    xmlNewChild( child, NULL, "latlongcoord", (xmlCharPtr) buffer );
    break;

  case TSM_UTM_COORDSYS:

    utm_foot = &foot->utm;
    child = xmlNewChild( root, NULL, "utmfootprint", NULL );

    sprintf( buffer, "%1.12lg %1.12lg %d", utm_foot->c1.e, utm_foot->c1.n,
	     utm_foot->c1.zone );
    xmlNewChild( child, NULL, "utmcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %d", utm_foot->c2.e, utm_foot->c2.n,
	     utm_foot->c2.zone );
    xmlNewChild( child, NULL, "utmcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %d", utm_foot->c3.e, utm_foot->c3.n,
	     utm_foot->c3.zone );
    xmlNewChild( child, NULL, "utmcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %d", utm_foot->c4.e, utm_foot->c4.n,
	     utm_foot->c4.zone );
    xmlNewChild( child, NULL, "utmcoord", (xmlCharPtr) buffer );
    break;

  case TSM_LVCS_COORDSYS:

    lvcs_foot = &foot->lvcs;
    child = xmlNewChild( root, NULL, "lvcsfootprint", NULL );

    sprintf( buffer, "%1.12lg %1.12lg", lvcs_foot->lat, lvcs_foot->lon );
    xmlNewChild( child, NULL, "latlongcoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg", lvcs_foot->xoffset,lvcs_foot->yoffset);
    xmlNewChild( child, NULL, "offset", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg", lvcs_foot->width, lvcs_foot->height );
    xmlNewChild( child, NULL, "dimensions", (xmlCharPtr) buffer );
    break;

  case TSM_GEO_COORDSYS:

    geo_foot = &foot->geo;
    child = xmlNewChild( root, NULL, "geofootprint", NULL );

    sprintf( buffer, "%1.12lg %1.12lg %1.12lg", geo_foot->c1.x,
	     geo_foot->c1.y, geo_foot->c1.z );
    xmlNewChild( child, NULL, "geocoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %1.12lg", geo_foot->c2.x,
	     geo_foot->c2.y, geo_foot->c2.z );
    xmlNewChild( child, NULL, "geocoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %1.12lg", geo_foot->c3.x,
	     geo_foot->c3.y, geo_foot->c3.z );
    xmlNewChild( child, NULL, "geocoord", (xmlCharPtr) buffer );

    sprintf( buffer, "%1.12lg %1.12lg %1.12lg", geo_foot->c4.x,
	     geo_foot->c4.y, geo_foot->c4.z );
    xmlNewChild( child, NULL, "geocoord", (xmlCharPtr) buffer );
    break;
  }

}

/*
 * Synopsis:
 *   void tsmAppendDataSet( xmlNodePtr root, TsmDataSet *dataset )
 *
 * Doc++ Description: *//** 
 *   Append a TsmDataSet structure to an XML tree.
 *
 *   This routine will take a TsmDataSet structure and will append it
 *   to the specified XML node.
 *
 * @param    root the XML node that we are appending to
 * @param    dataset the TsmDataSet structure to print
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmMakeDataSet, tsmPersistDatabase, TsmDataSet
 *
 */

void
tsmAppendDataSet( xmlNodePtr root, TsmDataSet *dataset )
{
  TsmDataSetInst *inst;
  int            i;
  char           *units = "m", *datatype;
  xmlNodePtr     base, child;
  char           minres[32], maxres[32], formats[64];

  if ( dataset == NULL ) return;

  /* convert non-string entries into text for passing to libxml */

  if ( dataset->type == TSM_IMAGE_DATASET )
    datatype = "image-pyramid";
  else if ( dataset->type == TSM_DEM_DATASET )
    datatype = "dem-pyramid";
  else if ( dataset->type == TSM_VRML_DATASET )
    datatype = "vrml-pyramid\n";
  else
    datatype = "unknown";

  if ( dataset->footprint.coordsys == TSM_LATLONG_COORDSYS )
    units = "deg";

  sprintf( minres, "%g %s", dataset->min_res, units );
  sprintf( maxres, "%g %s", dataset->max_res, units );

  /* output the new nodes to append to the XML tree */

  base  = xmlNewChild( root, NULL, "dataset", NULL );
  
  if ( dataset->name != NULL ) 
    xmlNewChild( base, NULL, "name", (xmlCharPtr) dataset->name );

  xmlNewChild( base, NULL, "type", (xmlCharPtr) datatype );

  if ( dataset->type == TSM_IMAGE_DATASET && dataset->source_type != NULL )
    xmlNewChild( base, NULL, "sourcetype", (xmlCharPtr) dataset->source_type );

  if ( dataset->description != NULL )
    xmlNewChild( base, NULL, "description", (xmlCharPtr) dataset->description);

  if ( dataset->geoname != NULL )
    xmlNewChild( base, NULL, "geoname", (xmlCharPtr) dataset->geoname );

  xmlNewChild( base, NULL, "minresolution", (xmlCharPtr) minres );

  xmlNewChild( base, NULL, "maxresolution", (xmlCharPtr) maxres );

  tsmAppendFootprint( base, &dataset->footprint );

  /* now output the tags for each instance of this dataset */

  if ( dataset->numInstances > 0 ) {
    for ( i = 0; i < dataset->numInstances; ++i ) {
      inst = &dataset->instance[i];
      
      child = xmlNewChild( base, NULL, "instance", NULL );

      xmlNewChild( child, NULL, "location", (xmlCharPtr) inst->location->url );

      if ( dataset->type == TSM_IMAGE_DATASET ) {
	strcpy( formats, "" );
	if ( ( inst->formats & TSM_LRBT_FORMAT ) != 0 ) {
	  strcat( formats, "lrbt" );
	}
	if ( ( inst->formats & TSM_JPEG_FORMAT ) != 0 ) {
	  if ( strlen( formats ) > 0 ) strcat( formats, " " );
	  strcat( formats, "jpeg" );
	}
	if ( ( inst->formats & TSM_LZW_FORMAT ) != 0 ) {
	  if ( strlen( formats ) > 0 ) strcat( formats, " " );
	  strcat( formats, "lzw" );
	}
	if ( ( inst->formats & TSM_MIPMAP_FORMAT ) != 0 ) {
	  if ( strlen( formats ) > 0 ) strcat( formats, " " );
	  strcat( formats, "mipmap" );
	}
	xmlNewChild( child, NULL, "formats", (xmlCharPtr) formats );
      }

      /* DPSS specific tags: servers and setid */

      if ( inst->location->protocol == tsmISS ||
	   inst->location->protocol == tsmDPSS ) {
	char servers[512] = "", setid[32];
	int  s;

	for ( s = 0; s < inst->numServers; ++s ) {
	  strcat( servers, inst->serverName[s] );
	  if ( s < inst->numServers-1 ) strcat( servers, " " );
	}

	sprintf( setid, "%d", inst->set_id );

	xmlNewChild( child, NULL, "servers", (xmlCharPtr) servers );

	if ( inst->set_id > 0 )
	  xmlNewChild( child, NULL, "setid", (xmlCharPtr) setid );

      }
    }
  }
}

/*
 * Synopsis:
 *   void tsmAppendGeoSet( xmlNodePtr root, TsmGeoSet *geoset )
 *
 * Doc++ Description: *//** 
 *   Appends a TsmGeoSet structure to an XML tree.
 *
 *   This routine will take a TsmGeoSet structure and will add it to
 *   the specified XML node.
 *
 * @param    root the XML node that we are appending to
 * @param    geoset the TsmGeoSet structure to print
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmAppendDataSet, tsmPersistDatabase, TsmGeoSet
 *
 */

void
tsmAppendGeoSet( xmlNodePtr root, TsmGeoSet *geoset )
{
  xmlNodePtr base;
  int        n;

  /* output the new nodes to append to the XML tree */

  base  = xmlNewChild( root, NULL, "geoset", NULL );
  
  if ( geoset->name != NULL ) 
    xmlNewChild( base, NULL, "name", (xmlCharPtr) geoset->name );

  if ( geoset->oi_name != NULL ) 
    xmlNewChild( base, NULL, "imagery", (xmlCharPtr) geoset->oi_name );

  if ( geoset->dem_name != NULL ) 
    xmlNewChild( base, NULL, "dem", (xmlCharPtr) geoset->dem_name );

  for ( n = 0; n < geoset->numModels; n++ )
    xmlNewChild( base, NULL, "model", (xmlCharPtr) geoset->model_name[n] );

  for ( n = 0; n < geoset->numMaps; n++ )
    xmlNewChild( base, NULL, "map", (xmlCharPtr) geoset->map_name[n] );

  xmlNewChild( base, NULL, "planet", (xmlCharPtr)
	       tsmPlanetToString(geoset->planet) );
}

/*
 * Synopsis:
 *   void tsmAppendMap( xmlNodePtr root, TsmMap *map )
 *
 * Doc++ Description: *//** 
 *   Append a TsmMap structure to an XML tree.
 *
 *   This routine will take a TsmMap structure and will add it to
 *   the specified XML tree.
 *
 * @param    root the XML node that we are appending to
 * @param    map the TsmMap structure to print
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmPersistDatabase
 *
 */

void
tsmAppendMap( xmlNodePtr root, TsmMap *map )
{
  xmlNodePtr base;
  char       buffer[128];
  int        n;

  /* convert non-string entries into text for passing to libxml */

  sprintf( buffer, "%20lg %20lg %20lg %20lg", map->min_u, map->min_v,
	   map->max_u, map->max_v ); 

  /* output the new nodes to append to the XML tree */

  base  = xmlNewChild( root, NULL, "map", NULL );
  
  if ( map->name != NULL ) 
    xmlNewChild( base, NULL, "name", (xmlCharPtr) map->name );

  if ( map->geoname != NULL ) 
    xmlNewChild( base, NULL, "geoname", (xmlCharPtr) map->geoname );

  for ( n = 0; n < map->numLocations; n++ ) 
    xmlNewChild( base, NULL, "location", (xmlCharPtr) map->location[n]->url );

  xmlNewChild( base, NULL, "uvcorners", (xmlCharPtr) buffer );

}

/*
 * Synopsis:
 *   void tsmAppendModel( xmlNodePtr root, TsmModel *model )
 *
 * Doc++ Description: *//** 
 *   Append a TsmModel structure to an XML tree.
 *
 *   This routine will take a TsmModel structure and will append it
 *   to the specified XML node.
 *
 * @param    root the XML node that we are appending to
 * @param    model the TsmModel structure to print
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmPersistDatabase
 *
 */

void
tsmAppendModel(  xmlNodePtr root, TsmModel *model )
{
  xmlNodePtr base;
  int        n;

  /* output the new nodes to append to the XML tree */

  base  = xmlNewChild( root, NULL, "model", NULL );
  
  if ( model->name != NULL ) 
    xmlNewChild( base, NULL, "name", (xmlCharPtr) model->name );

  if ( model->title != NULL ) 
    xmlNewChild( base, NULL, "title", (xmlCharPtr) model->title );

  if ( model->geoname != NULL )
    xmlNewChild( base, NULL, "geoname", (xmlCharPtr) model->geoname );

  if ( model->description != NULL )
    xmlNewChild( base, NULL, "description", (xmlCharPtr) model->description );

  for ( n = 0; n < model->numLocations; n++ ) 
    xmlNewChild( base, NULL, "location", (xmlCharPtr) model->location[n]->url);

  /*
  if ( model->name != NULL ) 
    fprintf( stream, "%sname: %s\n", leadString, model->name );
  if ( model->title != NULL ) 
    fprintf( stream, "%stitle: %s\n", leadString, model->title );
  if ( model->geoname != NULL ) 
    fprintf( stream, "%sgeoname: %s\n", leadString, model->geoname );
  if ( model->description != NULL )
    fprintf( stream, "%sdescription: %s\n", leadString, model->description );
  for ( n = 0; n < model->numLocations; n++ ) 
    fprintf( stream, "%slocation: %s\n", leadString, model->location[n]->url );
    */
}

/*
 * Synopsis:
 *   void tsmAppendServer( xmlNodePtr root, TsmServer *server )
 *
 * Doc++ Description: *//** 
 *   Append a TsmServer structure to the XML tree.
 *
 *   This routine will take a TsmServer structure and will add its
 *   contents as a child to the specified XML node. 
 *
 * @param    root the XML node that we are appending to
 * @param    server the TsmServer structure to print
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 November 1999
 *
 * @see      tsmAppendDataSet, tsmPersistDatabase, TsmServer
 *
 */

static void
rawTimeString (PRTime time, char *buffer, int len)
{
  PRInt64 lo, hi;
  PRInt64 t, lo_mask = LL_INIT (0, 0xffffffff);
  LL_AND (t, time, lo_mask); LL_L2UI (lo, t);
  LL_USHR (t, time, 32); LL_L2UI (hi, t);
  PR_snprintf( buffer, len, "%08x%08x", hi, lo);
}

void
tsmAppendServer( xmlNodePtr root, TsmServer *server )
{
  xmlNodePtr base, child;
  char       buffer[32], *datestring;

  /* convert non-string entries into text for passing to libxml */
  rawTimeString (server->last_contacted, buffer, 32);
  datestring = tsmDateString( server->last_contacted );

  /* output the new nodes to append to the XML tree */

  base  = xmlNewChild( root, NULL, "server", NULL );
  
  if ( server->location != NULL ) 
    xmlNewChild( base, NULL, "location", (xmlCharPtr) server->location->url );

  if ( server->last_contacted > 0 ) {
    child = xmlNewChild( base, NULL, "lastcontacted", NULL );
    xmlSetProp( child, "value", (xmlCharPtr) buffer );
    xmlSetProp( child, "text", (xmlCharPtr) datestring );
  }
}

/*
 * Synopsis:
 *   void tsmPrintFootprint( Footprint *foot, FILE *stream, char *leadString,
 *                           int colon )
 *
 * Doc++ Description: *//** 
 *   Prints a footprint to a stream.
 *
 *   This function will take a Footprint structure and will output a
 *   formatted version of this to the specified stream (e.g. stdout).
 *   Only one footprint will be output, but which coordinate system
 *   this will be in is specified by the foot->coordsys field. The
 *   footprint will be prefixed with a string of "latlong-footprint",
 *   "utm-footprint", or "geo-footprint", depending upon coordsys.
 *
 *   You can specify a leadString that is output at the start of each
 *   line, and you can specify whether you want a colon symbol to 
 *   appear after the prefix strings.
 *
 * @param    foot the Footprint structure to print out
 * @param    stream the stream to output the description to
 * @param    leadString a string to output at the start of each line
 * @param    colon TRUE (put a colon after prefix string), or FALSE
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  19 December 1997
 *
 * @see      tsmPrintDataset
 *
 */

void
tsmPrintFootprint( Footprint *foot, FILE *stream, char *leadString, int colon )
{
  LatLongFootprint *ll_foot;
  UTMFootprint     *utm_foot;
  GeoFootprint     *geo_foot;
  LVCSFootprint    *lvcs_foot;
  char             *sep;

  sep = ( colon ) ? ":" : "";

  switch ( foot->coordsys ) {
  case TSM_LATLONG_COORDSYS:
    ll_foot = &foot->latlong;
    fprintf( stream, "%slatlong-footprint%s", leadString, sep );
    fprintf( stream, " %1.12g %1.12g", ll_foot->c1.lat, ll_foot->c1.lon );
    fprintf( stream, "  %1.12g %1.12g", ll_foot->c2.lat, ll_foot->c2.lon );
    fprintf( stream, "  %1.12g %1.12g", ll_foot->c3.lat, ll_foot->c3.lon );
    fprintf( stream, "  %1.12g %1.12g", ll_foot->c4.lat, ll_foot->c4.lon );
    fprintf( stream, "\n" );
    break;
  case TSM_UTM_COORDSYS:
    utm_foot = &foot->utm;
    fprintf( stream, "%sutm-footprint%s", leadString, sep );
    fprintf( stream, " %1.12g %1.12g %d", utm_foot->c1.e,
	     utm_foot->c1.n, utm_foot->c1.zone );
    fprintf( stream, "  %1.12g %1.12g %d", utm_foot->c2.e,
	     utm_foot->c2.n, utm_foot->c2.zone );
    fprintf( stream, "  %1.12g %1.12g %d", utm_foot->c3.e,
	     utm_foot->c3.n, utm_foot->c3.zone );
    fprintf( stream, "  %1.12g %1.12g %d", utm_foot->c4.e,
	     utm_foot->c4.n, utm_foot->c4.zone );
    fprintf( stream, "\n" );
    break;
  case TSM_LVCS_COORDSYS:
    lvcs_foot = &foot->lvcs;
    fprintf( stream, "%slvcs-footprint%s", leadString, sep );
    fprintf( stream, " %1.12g %1.12g", lvcs_foot->lat, lvcs_foot->lon );
    fprintf( stream, " %1.12g %1.12g", lvcs_foot->xoffset, lvcs_foot->yoffset);
    fprintf( stream, " %1.12g %1.12g", lvcs_foot->width, lvcs_foot->height );
    fprintf( stream, "\n" );
    break;
  case TSM_GEO_COORDSYS:
    geo_foot = &foot->geo;
    fprintf( stream, "%sgeo-footprint%s", leadString, sep );
    fprintf( stream, " %1.12g %1.12g %1.12g", geo_foot->c1.x,
	     geo_foot->c1.y, geo_foot->c1.z );
    fprintf( stream, " %1.12g %1.12g %1.12g", geo_foot->c2.x,
	     geo_foot->c2.y, geo_foot->c2.z );
    fprintf( stream, " %1.12g %1.12g %1.12g", geo_foot->c3.x,
	     geo_foot->c3.y, geo_foot->c3.z );
    fprintf( stream, " %1.12g %1.12g %1.12g", geo_foot->c4.x,
	     geo_foot->c4.y, geo_foot->c4.z );
    fprintf( stream, "\n" );
    break;
  }

}

/*
 * Synopsis:
 *   void tsmPrintDataSet(TsmDataSet *dataset, FILE *stream, char *leadString)
 *
 * Doc++ Description: *//** 
 *   Print a TsmDataSet structure.
 *
 *   This routine will take a TsmDataSet structure and will print it out
 *   to the specified stream, i.e. you can direct this output to a file,
 *   or to the screen using stdout/stderr.
 *
 * @param    dataset the TsmDataSet structure to print
 * @param    stream the stream to output the structure to
 * @param    leadString a string to prefix to each line output (e.g. to
 *           supply a custom indentation level)
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmMakeDataSet, tsmPrintDatabase, TsmDataSet
 *
 */

void
tsmPrintDataSet( TsmDataSet *dataset, FILE *stream, char *leadString )
{
  TsmDataSetInst *inst;
  int            i, s;

  if ( dataset == NULL ) return;

  fprintf( stream, "%sname: %s\n", leadString, dataset->name );

  fprintf( stream, "%sgeo-name: %s\n", leadString, dataset->geoname );

  fprintf( stream, "%stype: ", leadString );
  switch ( dataset->type ) {
  case TSM_IMAGE_DATASET :
    fprintf( stream, "image-pyramid\n" );
    break;
  case TSM_DEM_DATASET :
    fprintf( stream, "dem-pyramid\n" );
    break;
  case TSM_VRML_DATASET :
    fprintf( stream, "vrml-pyramid\n" );
    break;
  default:
    fprintf( stream, "unknown\n" );
    break;
  }

  if ( dataset->type == TSM_IMAGE_DATASET && dataset->source_type != NULL )
    fprintf( stream, "%ssource-type: \"%s\"\n", leadString,
	     dataset->source_type );

  if ( dataset->description != NULL )
    fprintf( stream, "%sdescription: \"%s\"\n", leadString,
	     dataset->description );

  fprintf( stream, "%smin-resolution: %g m\n", leadString, dataset->min_res );
  fprintf( stream, "%smax-resolution: %g m\n", leadString, dataset->max_res );

  fprintf( stream, "%stilesize: %d\n", leadString, dataset->tilesize );
  fprintf( stream, "%soverlap: %d\n", leadString, dataset->overlap );

  tsmPrintFootprint( &dataset->footprint, stream, leadString, TRUE );

  if ( dataset->numInstances > 0 ) {
    for ( i = 0; i < dataset->numInstances; ++i ) {
      inst = &dataset->instance[i];
      
      fprintf( stream, "%s<instance>\n", leadString );
      fprintf( stream, "%s  location: %s\n", leadString, inst->location->url );

      if ( dataset->type == TSM_IMAGE_DATASET ) {
	fprintf( stream, "%s  formats:", leadString );
	if ( ( inst->formats & TSM_LRBT_FORMAT ) != 0 )
	  fprintf( stream, " lrbt" );
	if ( ( inst->formats & TSM_JPEG_FORMAT ) != 0 )
	  fprintf( stream, " jpeg" );
	if ( ( inst->formats & TSM_LZW_FORMAT ) != 0 )
	  fprintf( stream, " lzw" );
	if ( ( inst->formats & TSM_MIPMAP_FORMAT ) != 0 )
	  fprintf( stream, " mipmap" );
	fprintf( stream, "\n" );
      }

      if ( inst->location->protocol == tsmISS ||
	   inst->location->protocol == tsmDPSS ) {
	fprintf( stream, "%s  servers:", leadString );
	for ( s = 0; s < inst->numServers; ++s ) 
	  fprintf( stream, " %s", inst->serverName[s] );
	fprintf( stream, "\n" );
	if ( inst->set_id > 0 )
	  fprintf( stream, "%s  set-id: %d\n", leadString, inst->set_id );
      }

      fprintf( stream, "%s</instance>\n", leadString );
    }
  }
}

/*
 * Synopsis:
 *   void tsmPrintGeoSet( TsmGeoSet *geoset, FILE *stream, char *leadString )
 *
 * Doc++ Description: *//** 
 *   Print a TsmGeoSet structure.
 *
 *   This routine will take a TsmGeoSet structure and will print it out
 *   to the specified stream, i.e. you can direct this output to a file,
 *   or to the screen using stdout/stderr.
 *
 * @param    geoset the TsmGeoSet structure to print
 * @param    stream the stream to output the structure to
 * @param    leadString a string to prefix to each line output (e.g. to
 *           supply a custom indentation level)
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  19 March 1998
 *
 * @see      tsmPrintDataSet, tsmPrintDatabase, TsmGeoSet
 *
 */

void
tsmPrintGeoSet( TsmGeoSet *geoset, FILE *stream, char *leadString )
{
  int n;

  if ( geoset->name != NULL ) 
    fprintf( stream, "%sname: %s\n", leadString, geoset->name );
  if ( geoset->oi_name != NULL )
    fprintf( stream, "%simagery: %s\n", leadString, geoset->oi_name );
  if ( geoset->dem_name != NULL )
    fprintf( stream, "%sdem: %s\n", leadString, geoset->dem_name );
  for ( n = 0; n < geoset->numModels; n++ )
    fprintf( stream, "%smodel: %s\n", leadString, geoset->model_name[n] );
  for ( n = 0; n < geoset->numMaps; n++ )
    fprintf( stream, "%smap: %s\n", leadString, geoset->map_name[n] );
  fprintf( stream, "%splanet: %s\n", leadString,
	   tsmPlanetToString(geoset->planet) );
}

/*
 * Synopsis:
 *   void tsmPrintMap( TsmMap *map, FILE *stream, char *leadString )
 *
 * Doc++ Description: *//** 
 *   Print a TsmMap structure.
 *
 *   This routine will take a TsmMap structure and will print it out
 *   to the specified stream, i.e. you can direct this output to a file,
 *   or to the screen using stdout/stderr.
 *
 * @param    map the TsmMap structure to print
 * @param    stream the stream to output the structure to
 * @param    leadString a string to prefix to each line output (e.g. to
 *           supply a custom indentation level)
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  5 March 1999
 *
 * @see      tsmPrintDatabase
 *
 */

void
tsmPrintMap( TsmMap *map, FILE *stream, char *leadString )
{
  int n;

  if ( map->name != NULL ) 
    fprintf( stream, "%sname: %s\n", leadString, map->name );
  if ( map->geoname != NULL ) 
    fprintf( stream, "%sgeo-name: %s\n", leadString, map->geoname );
  for ( n = 0; n < map->numLocations; n++ ) 
    fprintf( stream, "%slocation: %s\n", leadString, map->location[n]->url );
  fprintf( stream, "%suv-corners: %g %g %g %g\n", leadString, 
	   map->min_u, map->min_v, map->max_u, map->max_v );
}

/*
 * Synopsis:
 *   void tsmPrintModel( TsmModel *model, FILE *stream, char *leadString )
 *
 * Doc++ Description: *//** 
 *   Print a TsmModel structure.
 *
 *   This routine will take a TsmModel structure and will print it out
 *   to the specified stream, i.e. you can direct this output to a file,
 *   or to the screen using stdout/stderr.
 *
 * @param    model the TsmModel structure to print
 * @param    stream the stream to output the structure to
 * @param    leadString a string to prefix to each line output (e.g. to
 *           supply a custom indentation level)
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  5 March 1999
 *
 * @see      tsmPrintDatabase
 *
 */

void
tsmPrintModel( TsmModel *model, FILE *stream, char *leadString )
{
  int n;

  if ( model->name != NULL ) 
    fprintf( stream, "%sname: %s\n", leadString, model->name );
  if ( model->title != NULL ) 
    fprintf( stream, "%stitle: %s\n", leadString, model->title );
  if ( model->geoname != NULL ) 
    fprintf( stream, "%sgeo-name: %s\n", leadString, model->geoname );
  if ( model->description != NULL )
    fprintf( stream, "%sdescription: %s\n", leadString, model->description );
  for ( n = 0; n < model->numLocations; n++ ) 
    fprintf( stream, "%slocation: %s\n", leadString, model->location[n]->url );
}

/*
 * Synopsis:
 *   void tsmPrintServer( TsmServer *server, FILE *stream, char *leadString )
 *
 * Doc++ Description: *//** 
 *   Print a TsmServer structure.
 *
 *   This routine will take a TsmServer structure and will print it out
 *   to the specified stream, i.e. you can direct this output to a file,
 *   or to the screen using stdout/stderr.
 *
 * @param    server the TsmServer structure to print
 * @param    stream the stream to output the structure to
 * @param    leadString a string to prefix to each line output (e.g. to
 *           supply a custom indentation level)
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmPrintDataSet, tsmPrintDatabase, TsmServer
 *
 */

void
tsmPrintServer( TsmServer *server, FILE *stream, char *leadString )
{
  if ( server->location != NULL ) 
    fprintf( stream, "%slocation: %s\n", leadString, server->location->url );
  if ( server->last_contacted > 0 ) {
    char buffer[32];
    rawTimeString (server->last_contacted, buffer, 32);
    fprintf( stream, "%slast-contacted: %s = %s\n", leadString,
	     buffer, tsmDateString( server->last_contacted ) );
  }
}

/*
 * Synopsis:
 *   void tsmPrintDatabase( TsmDatabase *database, FILE *stream )
 *
 * Doc++ Description: *//** 
 *   Print a database structure.
 *
 *   Prints the contents of a TsmDatabase structure to the specified
 *   stream, i.e. you can direct this output to a file, or to the screen
 *   using stdout/stderr.
 *
 * @param    database the TsmDatabase structure to print
 * @param    stream the stream to output the structure to
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmReadDatabase, tsmMakeDatabase, TsmDatabase
 *
 */

void
tsmPrintDatabase( TsmDatabase *database, FILE *stream )
{
  int i;

  if ( database == NULL ) return;

  fprintf( stream, "# Dataset Database - version 1.0\n" );
  fprintf( stream, "# Created by tsmApi %s", tsmApiReleaseOnly() );
  fprintf( stream, " on %s\n", tsmDateString(0) );
  fprintf( stream, "# For file format info, see " );
  fprintf( stream, "http://www.tsmApi.com/dataset-db.html\n" );

  for ( i = 0; i < database->numServers; ++i ) {
    fprintf( stream, "\n<server>\n" );
    tsmPrintServer( &database->server[i], stream, "  " );
    fprintf( stream, "</server>\n" );
  }

  for ( i = 0; i < database->numGeoSets; ++i ) {
    fprintf( stream, "\n<geoset>\n" );
    tsmPrintGeoSet( &database->geoset[i], stream, "  " );
    fprintf( stream, "</geoset>\n" );
  }

  for ( i = 0; i < database->numDataSets; ++i ) {
    fprintf( stream, "\n<dataset>\n" );
    tsmPrintDataSet( &database->dataset[i], stream, "  " );
    fprintf( stream, "</dataset>\n" );
  }

  for ( i = 0; i < database->numMaps; ++i ) {
    fprintf( stream, "\n<map>\n" );
    tsmPrintMap( &database->map[i], stream, "  " );
    fprintf( stream, "</map>\n" );
  }

  for ( i = 0; i < database->numModels; ++i ) {
    fprintf( stream, "\n<model>\n" );
    tsmPrintModel( &database->model[i], stream, "  " );
    fprintf( stream, "</model>\n" );
  }
}

/*
 * Synopsis:
 *   TsmDatabase *tsmMakeDatabase( void )
 *
 * Doc++ Description: *//** 
 *   Allocate a database structure.
 *      
 *   Allocate memory for a TsmDatabase structure, initialised with all 
 *   fields set to 0/NULL.
 *
 * @param    None
 *
 * @return   Pointer to allocated TsmDatabase structure, or NULL if failed.
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmFreeDatabase, TsmDatabase
 *
 */

TsmDatabase *
tsmMakeDatabase( void )
{
  const char  *funcName = "tsmMakeDatabase";
  TsmDatabase *database;

  database = (TsmDatabase *) tsmMemCalloc( 1, sizeof(TsmDatabase) );

  if ( database == NULL ) {
    tsmError( funcName, "Error allocating TsmDatabase" );
    return NULL;
  }
 
  return database;
}

/*
 * Synopsis:
 *   void tsmFreeDatabase( TsmDatabase *database )
 *
 * Doc++ Description: *//** 
 *   Free a database structure.
 *
 *   Frees up any memory that has been allocated for the TsmDatabase
 *   structure and then deallocates the memory for the structure itself
 *
 * @param    database the TsmDatabase structure to free
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmReadDatabase, tsmMakeDatabase, tsmDatabase
 *
 */

void
tsmFreeDatabase( TsmDatabase *database )
{
  int i;

  if ( database == NULL ) return;

  if ( database->location != NULL )
    tsmFreePurl( database->location );

  /* Free the contents of each TsmDataSet, then free the array */

  for ( i = 0; i < database->numDataSets; ++i ) 
    tsmFreeDataSetContents( &database->dataset[i] );

  if ( database->dataset != NULL )
    tsmMemFree( database->dataset );

  /* Free the contents of each TsmServer, then free the array */

  for ( i = 0; i < database->numServers; ++i )
    tsmFreeServer( &database->server[i] );

  if ( database->server != NULL )
    tsmMemFree( database->server );
  
  /* Free the contents of each TsmGeoSet, then free the array */

  for ( i = 0; i < database->numGeoSets; ++i )
    tsmFreeGeoSetContents( &database->geoset[i] );

  if ( database->geoset != NULL )
    tsmMemFree( database->geoset );
  
  /* Free the contents of each TsmMap, then free the array */

  for ( i = 0; i < database->numMaps; ++i )
    tsmFreeMap( &database->map[i] );

  if ( database->map != NULL )
    tsmMemFree( database->map );
  
  /* Free the contents of each TsmModel, then free the array */

  for ( i = 0; i < database->numModels; ++i )
    tsmFreeModel( &database->model[i] );

  if ( database->model != NULL )
    tsmMemFree( database->model );
  
  /* Free the TsmDatabase structure itself */

  tsmMemFree( database );
}

/*
 * Synopsis:
 *   void tsmPersistDatabase( TsmDatabase *database, char const *filename )
 *
 * Doc++ Description: *//** 
 *   Print a database structure.
 *
 *   Prints the contents of a TsmDatabase structure to the specified
 *   stream, i.e. you can direct this output to a file, or to the screen
 *   using stdout/stderr.
 *
 * @param    database the TsmDatabase structure to print
 * @param    stream the stream to output the structure to
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  8 December 1997
 *
 * @see      tsmReadDatabase, tsmMakeDatabase, TsmDatabase
 *
 */

int
tsmPersistDatabase( TsmDatabase *database, char const *filename )
{
  const char *funcName = "tsmPersistDatabase";
  xmlDocPtr  doc;
  xmlNodePtr top;
  int        i;

  if ( database == NULL ) return FALSE;

  /* output the root node of the XML tree */

  doc = xmlNewDoc( "1.0" );
  top = xmlNewDocNode( doc, NULL, "TerraVisionDatabase", NULL );
  xmlDocSetRootElement (doc, top);

  xmlSetProp( top, "version", "1.0" );
  xmlSetProp( top, "created", (xmlChar const *) tsmDateString(0) );

  /* and then output children for all servers, geosets, datasets, etc. */

  for ( i = 0; i < database->numServers; ++i )
    tsmAppendServer( top, &database->server[i] );

  for ( i = 0; i < database->numGeoSets; ++i )
    tsmAppendGeoSet( top, &database->geoset[i] );

  for ( i = 0; i < database->numDataSets; ++i )
    tsmAppendDataSet( top, &database->dataset[i] );

  for ( i = 0; i < database->numMaps; ++i )
    tsmAppendMap( top, &database->map[i] );

  for ( i = 0; i < database->numModels; ++i )
    tsmAppendModel( top, &database->model[i] );

  /* finally, save out the XML tree to the specified stream */

  if ( xmlSaveFile( filename, doc ) == FALSE ) {
    xmlFreeDoc( doc );
    tsmError( funcName, "Could not write to database (%s)", filename );
    return FALSE;
  }

  xmlFreeDoc( doc );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmAddDataSetToDatabase( TsmDatabase *database, TsmDataSet *dataset )
 *
 * Doc++ Description: *//** 
 *   Add a dataset to a database.
 *
 *   This function will add the contents of a TsmDataSet structure to
 *   the specified TsmDatabase. The dataset can contain multiple instances
 *   and all of these will be added as necessary. Only data that is not
 *   already in the dataset will be added.
 *
 *   This routine will first check to see if an entry already exists for
 *   the dataset in the database (i.e. if an entry with the same name field
 *   exists in the database). If the dataset does not already exist, then
 *   a new entry will be created and all of the fields copied over (strings
 *   are strdup'd). If the dataset does exist in the database already, then
 *   checks are made that all of the fields are the same (e.g. type, min/max
 *   resolution, etc.).
 *
 *   Once a dataset entry has been found or created, the routine will see
 *   if it needs to add any instances to this dataset. An instance is deemed
 *   to be unique by its URL and formats are the same, and if its server
 *   list is the same if the URL is for a DPSS set. If no instance already
 *   exists in the database, then one will be added.
 *
 *   If a DPSS instance of the dataset in the database is the same as the
 *   instance to be added, except that the set ID differs, then the set id
 *   will updated in the database with the new set ID.
 *
 * @param    database the TsmDatabase to be updated
 * @param    dataset  the TsmDataSet to add to the database
 *
 * @return   TRUE if everything worked okay, FALSE if an error occurred.
 *
 * @author   Martin Reddy
 * @version  11 December 1997
 *
 * @see      tsmReadDatabase, tsmWriteDatabase, TsmDatabase, TsmDataSet
 *
 */

int
tsmAddDataSetToDatabase( TsmDatabase *database, TsmDataSet *dataset )
{
  const char     *funcName = "tsmAddDataSetToDatabase";
  TsmDataSetInst *src_inst, *dest_inst, *new_inst;
  TsmDataSet     *new_set;
  int            i, j, m, n, found;

  /* Check that we have valid database and dataset pointers */

  if ( database == NULL || dataset == NULL || dataset->name == NULL ) {
    tsmError( funcName, "database, dataset & name pointers must be non-NULL" );
    return FALSE;
  }

  /* Check that the dataset has an instance - otherwise no point */

  if ( dataset->instance == NULL ) {
    tsmError( funcName, "dataset has no instance! What's the point?" );
    return FALSE;
  }

  /* Search to see if we already have an entry for the dataset */

  new_set = NULL;
  for ( i = 0; i < database->numDataSets && new_set == NULL; ++i )
    if ( strcmp( dataset->name, database->dataset[i].name ) == 0 )
      new_set = &database->dataset[i];

  /* If we didn't find the dataset in the database, then make a new entry */

  if ( new_set == NULL ) {
    if ( tsmNewDatabaseDataSet( database ) == FALSE ) {
      tsmError( funcName, "couldn't add a dataset to the database" );
      return FALSE;
    }
    new_set = &database->dataset[database->numDataSets - 1];

    /* Take copies of the dataset fields into the new dataset */

    new_set->name      = tsmMemStrdup( dataset->name );
    new_set->type      = dataset->type;
    new_set->footprint = dataset->footprint;
    new_set->min_res   = dataset->min_res;
    new_set->max_res   = dataset->max_res;
    new_set->tilesize  = dataset->tilesize;
    new_set->overlap   = dataset->overlap;

    if ( dataset->description != NULL )
      new_set->description = tsmMemStrdup( dataset->description );

    if ( dataset->geoname != NULL )
      new_set->geoname = tsmMemStrdup( dataset->geoname );

    if ( dataset->source_type != NULL )
      new_set->source_type = tsmMemStrdup( dataset->source_type );
    else
      new_set->source_type = tsmMemStrdup( "unknown" );

  } else {

    /* if the entry already exists, then check to see if all of the other */
    /* dataset fields tally. If they differ, then this is an error.       */

    if ( dataset->type != new_set->type ) {
      tsmError( funcName, "type field differs for dataset in database" );
      return FALSE;
    }

    /* Actually, if the resolutions differ, we can let that slide */

    if ( dataset->min_res != new_set->min_res || 
	 dataset->max_res != new_set->max_res ) {
      TSMTRACE(( TSMMASK|1, funcName, "different resolution in database "
		 "than for %s", dataset->name ));
    }
  }

  /* Okay, now we have updated the dataset entry of the database (if */
  /* necessary). So let's now update that dataset entry with the     */
  /* details for the particular instance, if not there already.      */

  /* copy all of the instances that are defined in the input dataset */

  for ( j = 0; j < dataset->numInstances; ++j ) {

    /* Search to see if we already have an entry for the instance */
    /* this involves testing the URL name, and also the server    */
    /* combination if the URL is for a DPSS set.                  */

    new_inst = NULL;
    src_inst = &dataset->instance[j];

    for ( i = 0; i < new_set->numInstances && new_inst == NULL; ++i ) {
      dest_inst = &new_set->instance[i];

      /* check the URL name */

      if ( src_inst->location == NULL || dest_inst->location == NULL ||
	   strcmp( src_inst->location->url, dest_inst->location->url ) != 0 )
	continue;

      /* do some extra tests if this is a DPSS dataset */

      if ( dest_inst->location->protocol == tsmISS ||
	   dest_inst->location->protocol == tsmDPSS ) {

	/* check to see if the setIds are different */

	if ( src_inst->set_id != dest_inst->set_id )
	  continue;

	/* check the server list (probably don't need to do */
	/* this as checking the set id should by enough).   */

	if ( src_inst->numServers != dest_inst->numServers ||
	     src_inst->serverName == NULL )
	  continue;

	found = TRUE;
	for ( m = 0; m < src_inst->numServers && found == TRUE; ++m ) {
	  found = FALSE;
	  for ( n = 0; n < dest_inst->numServers && found == FALSE; ++n ) {
	    if ( !strcmp( src_inst->serverName[m], dest_inst->serverName[n] ) )
	      found = TRUE;
	  }
	}

	if ( found == FALSE )
	  continue;
      }

      /* found a duplicate entry! Update any fields that might need it */

      new_inst = dest_inst;
      if ( src_inst->formats > 0 ) new_inst->formats = src_inst->formats;

      break;
    }

    /* We have checked all current instances for this dataset in the */
    /* database now. If we didn't find a duplicate instance, then we */
    /* want to create one now and fill in all of the fields.         */

    if ( new_inst == NULL && src_inst->location != NULL ) {
      if ( tsmNewDataSetInstance( new_set ) == FALSE ) {
	tsmError( funcName, "couldn't allocate a new instance for dataset" );
	return FALSE;
      }

      new_inst = &new_set->instance[new_set->numInstances - 1];
      
      new_inst->location   = tsmCopyPurl( src_inst->location );
      new_inst->formats    = src_inst->formats;
      new_inst->set_id     = src_inst->set_id;
      new_inst->numServers = src_inst->numServers;
      new_inst->serverName =
	(char **) tsmMemMalloc( sizeof(char *) * new_inst->numServers );

      for ( m = 0; m < new_inst->numServers; ++m )
	new_inst->serverName[m] = tsmMemStrdup( src_inst->serverName[m] );
    }

  }

  return TRUE;
}

/*
 * Synopsis:
 *   TsmDatabase *tsmReadDatabase( char *location )
 *
 * Doc++ Description: *//** 
 *   Reads a dataset database file.
 *
 *   This function takes a URL for a dataset database file and attempts
 *   to parse this into a TsmDatabase structure. The parsing routine
 *   will check for the following errors in the database file, causing
 *   a NULL return pointer:
 *
 *   \begin{itemize}
 *     \item </tag> before a <tag>
 *     \item nesting of <tag> blocks
 *     \item <tag> without a </tag>
 *     \item <instance> can only occur inside <dataset>
 *     \item check that <tag> contains all required fields when </tag> found
 *     \item keyword/value pairs only valid inside appropriate <tag> group
 *     \item min/max-resolution values without a unit string
 *     \item check that min-resolution <= max_resolution
 *     \item invalid type value
 *     \item unparsable URLs for location fields
 *   \end{itemize}   
 *
 *   The format used for the dataset database file is described at:
 *   \URL{http://www.ai.sri.com/~magic/tsmApi/dataset-db.html}
 *
 * @param    location the URL or localdisk filename for the database file
 *
 * @return   The allocated TsmDatabase, or NULL if an error occurred
 *
 * @author   Martin Reddy
 * @version  11 December 1997
 *
 * @see      tsmWriteDatabase, tsmFreeDatabase, TsmDatabase
 *
 */

TsmDatabase*
tsmReadDatabase( char *location )
{
  const char  *funcName = "tsmReadDatabase";
  TsmDatabase *database;
  Purl        *purl;
  int         success;

  /* Parse the URL for the database location */

  if ( location == NULL || ( purl = tsmParseUrl(location) ) == NULL ) {
    tsmError( funcName, "Invalid URL for dataset database file" );
    return NULL;
  }

  /* Make a TsmDatabase structure to hold the contents of the file */

  if ( ( database = tsmMakeDatabase() ) == NULL ) {
    tsmError( funcName, "Unable to allocate a TsmDatabase structure" );
    tsmFreePurl( purl );
    return NULL;
  }

  database->location = purl;

  /* Load the database file in a protocol dependent manner */

  switch ( purl->protocol ) {
  case tsmLocalFile:
    success = tsmReadDatabase_local( database, purl->uri );
    break;
  case tsmHTTP:
  case tsmISS:
  case tsmDPSS:
  case tsmWMT:
  default:
    tsmNoProtocolYet( purl->protocol, funcName );
    success = FALSE;
  }

  /* deallocate the database structure if something went wrong */

  if ( success == FALSE ) {
    tsmError( funcName, "Couldn't read the dataset database" );
    tsmFreeDatabase( database );
    return NULL;
  }

  /* return the allocated database structure */

  return database;
}

/*
 * Synopsis:
 *   int tsmWriteDatabase( TsmDatabase *database, char *location )
 *
 * Doc++ Description: *//** 
 *   Writes a dataset database file.
 *
 *   This function will take a TsmDatabase and write this out to a
 *   dataset database file. If a location URL is specified, then this
 *   is used to specify where the file is to be saved. N.B. this does
 *   not change the default URL (the location field) associated with
 *   the database.
 *
 *   If the location URL is NULL or "", then the database->location
 *   field will be used as the location of the database file. This
 *   field is automatically filled in by tsmReadDatabase(), so if you
 *   want to overwrite a database that you just read, then you can use
 *   location=NULL.
 *
 *   The format used for the dataset database file is described at:
 *   \URL{http://www.ai.sri.com/~magic/tsmApi/dataset-db.html}
 *
 * @param    database the TsmDatabase to write out
 * @param    location the URL/filename to write to (or NULL or "")
 *
 * @return   TRUE if successful, FALSE if the write failed
 *
 * @author   Martin Reddy
 * @version  11 December 1997
 *
 * @see      tsmReadDatabase, TsmDatabase
 * */

int
tsmWriteDatabase( TsmDatabase *database, char *location )
{
  const char  *funcName = "tsmWriteDatabase";
  Purl        *purl;
  int         success;
  
  /* treat a "" string as a NULL pointer */

  if ( location != NULL && location[0] == '\0' )
    location = NULL;

  /* Work out the URL to use for the database */

  if ( location != NULL ) {
    if ( ( purl = tsmParseUrl( location ) ) == NULL ) {
      tsmError( funcName, "Invalid URL for dataset database file" );
      return FALSE;
    }
  } else {
    if ( ( purl = database->location ) == NULL ) {
      tsmError( funcName, "No URL specified in TsmDatabase structure" );
      return FALSE;
    }
  }

  /* Save the database file in a protocol dependent manner */

  switch ( purl->protocol ) {
  case tsmLocalFile:
    success = tsmWriteDatabase_local( database, purl->uri );
    break;
  case tsmHTTP:
  case tsmDPSS:
  case tsmISS:
  case tsmWMT:
  default:
    tsmNoProtocolYet( purl->protocol, funcName );
    success = FALSE;
  }
  
  /* free the Purl if we allocated one in this routine */

  if ( location != NULL )
    tsmFreePurl( purl );

  /* give an error message if something went wrong - otherwise return okay */

  if ( success == FALSE ) {
    tsmError( funcName, "Couldn't save the dataset database" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmFootprintToLatLong( Footprint *in, Footprint *out )
 *
 * Description:
 *   Converts a geospatial footprint (provided in any of a number of
 *   coordinate systems) so that it is specified in terms of four
 *   lat/long coordinates. The original footprint is specified by
 *   the in pointer, and the modified footprint is written to
 *   the out structure.
 *
 *   Supported coordinate systems include: UTM, geocentric.
 *
 *   If the original footprint is already in lat/long, then no
 *   operation is performed and the function returns TRUE.
 *
 * Returns: TRUE if the footprint was converted to lat/long
 *
 * Author: Martin Reddy
 *
 * Date: 15 August 2000
 *
 */

int
tsmFootprintToLatLong( Footprint *in, Footprint *out )
{
  int    s1, s2, s3, s4;
  double h, w, x, y;
  double deg_per_m = 1.0 / 100000.0;

  /* check that we have a valid input and output pointer */

  if ( in == NULL || out == NULL )
    return FALSE;

  /* try to convert the footprint into lat/long coords */

  switch ( in->coordsys ) {
  case TSM_UTM_COORDSYS:
    /* convert UTM -> lat/long */
    out->coordsys = TSM_LATLONG_COORDSYS;
    s1 = tsmUTMToLatLong( in->utm.c1.zone, in->utm.c1.e, in->utm.c1.n,
			  &out->latlong.c1.lat, &out->latlong.c1.lon );
    s2 = tsmUTMToLatLong( in->utm.c2.zone, in->utm.c2.e, in->utm.c2.n,
			  &out->latlong.c2.lat, &out->latlong.c2.lon );
    s3 = tsmUTMToLatLong( in->utm.c3.zone, in->utm.c3.e, in->utm.c3.n,
			  &out->latlong.c3.lat, &out->latlong.c3.lon );
    s4 = tsmUTMToLatLong( in->utm.c4.zone, in->utm.c4.e, in->utm.c4.n,
			  &out->latlong.c4.lat, &out->latlong.c4.lon );
    return ( s1 && s2 && s3 && s4 );

  case TSM_GEO_COORDSYS:
    /* convert geocentric -> lat/long */
    out->coordsys = TSM_LATLONG_COORDSYS;
    s1 = tsmGeocentricToLatLong( in->geo.c1.x, in->geo.c1.y, in->geo.c1.z,
				 &out->latlong.c1.lat, &out->latlong.c1.lon,
				 &h );
    s2 = tsmGeocentricToLatLong( in->geo.c2.x, in->geo.c2.y, in->geo.c2.z,
				 &out->latlong.c2.lat, &out->latlong.c2.lon,
				 &h );
    s3 = tsmGeocentricToLatLong( in->geo.c3.x, in->geo.c3.y, in->geo.c3.z,
				 &out->latlong.c3.lat, &out->latlong.c3.lon,
				 &h );
    s4 = tsmGeocentricToLatLong( in->geo.c4.x, in->geo.c4.y, in->geo.c4.z,
				 &out->latlong.c4.lat, &out->latlong.c4.lon,
				 &h );
    return ( s1 && s2 && s3 && s4 );

  case TSM_LVCS_COORDSYS:
    /* convert local vertical coord system -> lat/long (approximate only) */
    out->coordsys = TSM_LATLONG_COORDSYS;

    x = in->lvcs.lon + ( in->lvcs.xoffset * deg_per_m );
    y = in->lvcs.lat + ( in->lvcs.yoffset * deg_per_m );
    w = ( in->lvcs.width * deg_per_m ) / 2.0;
    h = ( in->lvcs.height * deg_per_m ) / 2.0;

    out->latlong.c1.lon = x - w;
    out->latlong.c1.lat = y - h;
    out->latlong.c2.lon = x + w;
    out->latlong.c2.lat = y - h;
    out->latlong.c3.lon = x + w;
    out->latlong.c3.lat = y + h;
    out->latlong.c4.lon = x - w;
    out->latlong.c4.lat = y + h;

    return TRUE;

  case TSM_LATLONG_COORDSYS:
    /* already in lat/long - nothing to do except return the original */
    *out = *in;
    return TRUE;

  default:
    /* any other coordinate systems are not supported at this time */
    return FALSE;
  }
}

/*
 * Synopsis:
 *   int tsmIsFootprintInside( Footprint *inside, Footprint *outside )
 *
 * Description:
 *   Tests to see if one footprint (inside) is completely enclosed\
 *   by the other (outside).
 *
 *   Currently, if the footprints are in different coordinate systems
 *   then we always return FALSE.
 *
 * Returns: TRUE if the first footprint is enclosed by the second.
 *
 * Author: Martin Reddy
 *
 * Date: 5 November 1999
 *
 */

int
tsmIsFootprintInside( Footprint *inside, Footprint *outside )
{
  double     coord1, coord2;
  Footprint  ll1, ll2;

  if ( inside == NULL || outside == NULL ) 
    return FALSE;

  /* first, if both are LVCS datasets, then compare these now */

  if ( inside->coordsys == TSM_LVCS_COORDSYS &&
       outside->coordsys == TSM_LVCS_COORDSYS ) 

    /* for 2 LVCS coordinate systems to match, their origins must */
    /* be the same, and the extent of the inside footprint must   */
    /* be smaller than or equal to that of the outside footprint. */
    
    return ( inside->lvcs.lat == outside->lvcs.lat &&
	     inside->lvcs.lon == outside->lvcs.lon &&
	     inside->lvcs.width <= outside->lvcs.width &&
	     inside->lvcs.height <= outside->lvcs.height );

  /* otherwise, we try to convert both footprints into lat/long */
  /* and then do the comparison in lat/long space. This lets us */
  /* compare easily against footprints in different coord syses */

  if ( tsmFootprintToLatLong( inside, &ll1 ) == FALSE ||
       tsmFootprintToLatLong( outside, &ll2 ) == FALSE )
    return FALSE;

  /* now we do the comparison over the two lat/long footprints */
  /* for two lat/long footprints, the inside one must be       */
  /* contained completely within the outside footprint         */

  /* check the eastern most point */

  coord1 = max( max( max( ll1.latlong.c1.lon, ll1.latlong.c2.lon ),
		     ll1.latlong.c3.lon ), ll1.latlong.c4.lon );
  coord2 = max( max( max( ll2.latlong.c1.lon, ll2.latlong.c2.lon ),
		       ll2.latlong.c3.lon ), ll2.latlong.c4.lon );
  
  if ( coord1 > coord2 ) return FALSE;

  /* check the western most point */

  coord1 = min( min( min( ll1.latlong.c1.lon, ll1.latlong.c2.lon ),
		     ll1.latlong.c3.lon ), ll1.latlong.c4.lon );
  coord2 = min( min( min( ll2.latlong.c1.lon, ll2.latlong.c2.lon ),
		     ll2.latlong.c3.lon ), ll2.latlong.c4.lon );
  
  if ( coord1 < coord2 ) return FALSE;
  
  /* check the northern most point */
  
  coord1 = max( max( max( ll1.latlong.c1.lat, ll1.latlong.c2.lat ),
		     ll1.latlong.c3.lat ), ll1.latlong.c4.lat );
  coord2 = max( max( max( ll2.latlong.c1.lat, ll2.latlong.c2.lat ),
		     ll2.latlong.c3.lat ), ll2.latlong.c4.lat );
  
  if ( coord1 > coord2 ) return FALSE;
  
  /* check the southern most point */
  
  coord1 = min( min( min( ll1.latlong.c1.lat, ll1.latlong.c2.lat ),
		     ll1.latlong.c3.lat ), ll1.latlong.c4.lat );
  coord2 = min( min( min( ll2.latlong.c1.lat, ll2.latlong.c2.lat ),
		     ll2.latlong.c3.lat ), ll2.latlong.c4.lat );
  
  if ( coord1 < coord2 ) return FALSE;
  
  return TRUE;
}

/*
 * Synopsis:
 *   void tsmCalcFootprintFromXform( Pyramid *pyramid, TsmDataset *dataset )
 *
 * Description:
 *   This function takes a Pyramid structure and uses the informaton
 *   in this in order to fill out the footprint information for a
 *   database dataset, e.g. if the projection type for the pyramid is
 *   UTM, then the function will fill out the utm_footprint structure
 *   of the dataset by calculating the four bounding-box corners of
 *   the dataset.
 *
 * Externals: None
 *
 * Returns: Nothing
 *
 * Author: Martin Reddy
 *
 * Date: 15 December 1997
 *
 */

void
tsmCalcFootprintFromXform( Pyramid *pyramid, TsmDataSet *dataset )
{
  ImageTransformationTspec *xform;
  ProjectionTspec          *proj;
  CoordTspec               *coord;
  SimpleTileSet            *tileset;
  UTMFootprint             *utm_foot;
  LatLongFootprint         *ll_foot;
  LVCSFootprint            *lvcs_foot;
  GeoFootprint             *geo_foot;

  /* take some convenience pointers to make the source more readable */

  utm_foot  = &dataset->footprint.utm;
  geo_foot  = &dataset->footprint.geo;
  lvcs_foot = &dataset->footprint.lvcs;
  ll_foot   = &dataset->footprint.latlong;
  tileset   = &pyramid->tspecLevel.val[pyramid->minLevel];
  xform     = &tileset->imageTransformationTspec;
  coord     = &tileset->coordTspec;
  proj      = &tileset->projectionTspec;

  /* work out the utm, lat/long, or geocentric footprint from the pyramid */

  switch ( pyramid->projectionTspec.projectionType ) {

  case tspUtmProjection:

    /* we want to find the UTM coordinate for the lower-left corner */
    /* and upper right corner of the dataset. With these two values */
    /* we can complete the upper-left and lower-right, assuming a   */
    /* utm-coordinate-system-aligned bounding box organisation, i.e */
    /*                                                              */
    /*             | res  0  ll_e |        | 0 |       | x |        */
    /*  ixiytoUV = |  0  res ll_n | , ll = | 0 |, ur = | y |        */
    /*             |  0   0    1  |        | 1 |       | 1 |        */
    /*                                                              */
    /*  ixiytoUV * ll gives the point c1, ixiytoUV * ur gives c2    */

    /* c1 = lower-left */

    utm_foot->c1.e    = xform->ixiytoUV[2];
    utm_foot->c1.n    = xform->ixiytoUV[5];
    utm_foot->c1.zone = pyramid->projectionTspec.utmZoneNumber;

    /* c3 = upper-right */

    utm_foot->c3.e    = xform->ixiytoUV[0] * tileset->numValidPixel[0] +
                        xform->ixiytoUV[2];
    utm_foot->c3.n    = xform->ixiytoUV[4] * tileset->numValidPixel[1] +
                        xform->ixiytoUV[5];
    utm_foot->c3.zone = utm_foot->c1.zone;

    /* work out the values of c2 (upper-left) and c4 (lower-right) */

    utm_foot->c2.e    = utm_foot->c1.e;
    utm_foot->c2.n    = utm_foot->c3.n;
    utm_foot->c2.zone = utm_foot->c1.zone;

    utm_foot->c4.e    = utm_foot->c3.e;
    utm_foot->c4.n    = utm_foot->c1.n;
    utm_foot->c4.zone = utm_foot->c3.zone;

    dataset->footprint.coordsys = TSM_UTM_COORDSYS;
    break;

  case tspLatLongProjection:

    /* c1 = lower-left */

    ll_foot->c1.lat = coord->originLat;
    ll_foot->c1.lon = coord->originLong;

    /* c3 = upper-right */

    ll_foot->c3.lat = coord->originLat + tileset->numValidPixel[1] *
      pyramid->minResolution;
    ll_foot->c3.lon = coord->originLong + tileset->numValidPixel[0] *
      pyramid->minResolution;

    /* work out the values of c2 (upper-left) and c4 (lower-right) */

    ll_foot->c2.lat = ll_foot->c3.lat;
    ll_foot->c2.lon = ll_foot->c1.lon;

    ll_foot->c4.lat = ll_foot->c1.lat;
    ll_foot->c4.lon = ll_foot->c3.lon;

    dataset->footprint.coordsys = TSM_LATLONG_COORDSYS;
    break;

  case tspAffineProjection:

    lvcs_foot->lat     = coord->originLat;
    lvcs_foot->lon     = coord->originLong;
    lvcs_foot->xoffset = proj->lcsToUvProjectiveMatrix[2];
    lvcs_foot->yoffset = proj->lcsToUvProjectiveMatrix[6];
    lvcs_foot->width   = tileset->numValidPixel[0] * pyramid->minResolution;
    lvcs_foot->height  = tileset->numValidPixel[1] * pyramid->minResolution;

    dataset->footprint.coordsys = TSM_LVCS_COORDSYS;
    break;

  default:
    dataset->footprint.coordsys = TSM_NONE_COORDSYS;
    break;
  }

}

/*
 * Synopsis:
 *   int tsmRemoveServerSetsFromDatabase( TsmDatabase *database, TsmServer *,
 *                                        NameUrlIds * )
 *
 * Doc++ Description: *//** 
 *    Removes all sets for a server from database.
 *
 *    This function will take a TsmDatabase and a server name. It will then
 *    check every instance of every dataset in the database. Any instances
 *    that are found that describe datasets on the specifed server are
 *    removed from the database. If a dataset is reduced to having no 
 *    instances as a result of this, then that dataset is also removed
 *    from the database. The same is done to all of the map and model
 *    entries in the database.
 *
 *    The NameUrlIds parameter contains a list of (name, url, setId)
 *    tuples that define datasets to be kept in the database. If a database
 *    instance is found to match any entry in this list, then that
 *    instance is kept in the database and is not deleted. A NULL value
 *    can be passed here is you have no URLs that you want to keep for
 *    the specified server.
 *
 * @param    database the TsmDatabase structure to remove from
 * @param    server the server structure containing the location of the 
 *           server to check against each database instance.
 * @param    keep_sets a list of (name,url,setid) tuples defining entries
 *           that should not be deleted
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  13 December 1997
 *
 * @see      tsmUpdateDatabaseFromServer
 *
 */

void
tsmRemoveServerSetsFromDatabase( TsmDatabase *database, TsmServer *server,
				 NameUrlIds *keep_sets )
{
  const char     *funcName = "tsmRemoveServerSetsFromDatabase";
  TsmDataSet     *dataset;
  TsmDataSetInst *instance;
  int            i, j, k, found, serverLen, is_dpss;
  char           serverUrl[512];

  /* check that we have valid pointers to play with */
  
  if ( database == NULL || server == NULL || server->location == NULL ) {
    tsmError( funcName, "database and server pointers must be non-NULL" );
    return;
  }

  /* take a copy of the server URL and work out its length */

  strcpy( serverUrl, server->location->url );
  serverLen = strlen( serverUrl );

  /* If the url is an x-dsm:, then change this to x-iss: to match */
  /* against DPSS datasets that are recorded by that DSM.         */

  if ( strncmp( serverUrl, "x-dsm:", 6 ) == 0 )
    memcpy( serverUrl, "x-iss:", 6 );

  /* now go through every instance of every dataset and see if the */
  /* instance is on the specifed server. If it is, then remove it  */

  for ( i = 0; i < database->numDataSets; ++i ) {
    dataset = &database->dataset[i];

    for ( j = 0; j < dataset->numInstances; ++j ) {
      instance = &dataset->instance[j];

      /* Check to see if the start of the dataset is the */
      /* same as the server URL that we have been given. */

      if ( strncmp( instance->location->url, serverUrl, serverLen ) != 0 ) 
	continue;

      /* do we have a DPSS dataset? */

      is_dpss = ( instance->location->protocol == tsmDSM ||
		  instance->location->protocol == tsmISS ||
		  instance->location->protocol == tsmDPSS );

      /* check to see if this dataset exists in the keep list */
      /* definitely delete if we don't have enough info for   */
      /* this dataset, e.g. we now require a geoname field.   */

      if ( keep_sets && dataset->geoname ) {
	for ( k = found = 0; k < keep_sets->numItems && found == FALSE; k++ ) {

	  /* are the URLs have the same? */

	  if ( strcasecmp( keep_sets->names[k], dataset->name ) == 0 ) {

	    /* if we have a DPSS instance, we should check the setid too */

	    if ( is_dpss ) {
	      if ( instance->set_id == keep_sets->ids[k] )
		found = TRUE;
	    } else 
	      found = TRUE;
	  }
	}

	if ( found ) continue;
      }

      /* if we got this far, then we are to remove this instance */

      tsmDeleteDataSetInstance( dataset, j-- );
    }

    /* if we cleared out all of the instances in the dataset */
    /* then we can get rid of the dataset entry as well.     */

    if ( dataset->numInstances == 0 ) 
      tsmDeleteDatabaseDataSet( database, i-- );
  }

  /* now let's go through all of the maps for this server and remove */
  /* them (N.B. we don't bother checking the keep list for maps).    */

  for ( i = 0; i < database->numMaps; ++i ) {
    TsmMap *map = &database->map[i];

    for ( j = 0; j < map->numLocations; ) {
      Purl *location = map->location[j];

      /* Check to see if this location is on the server being removed */

      if ( strncmp( location->url, serverUrl, serverLen ) != 0 ) {
	j++;
	continue;
      }

      /* if we got this far, then we are to remove this instance */

      tsmFreePurl( map->location[j] );

      if ( --map->numLocations == 0 ) {
	if (tsmRemoveMapFromDatabasePos ( database, i )) {
	  --i;
	  break;
	}
      } else {
	for ( k = j; k < map->numLocations; ++k )
	  map->location[k] = map->location[k+1];
      }
    }
  }

  /* now let's go through all of the models for this server and remove */
  /* them (N.B. we don't bother checking the keep list for models).    */

  for ( i = 0; i < database->numModels; ++i ) {
    TsmModel *model = &database->model[i];

    for ( j = 0; j < model->numLocations; ) {
      Purl *location = model->location[j];

      /* Check to see if this location is on the server being removed */

      if ( strncmp( location->url, serverUrl, serverLen ) != 0 ) {
	j++;
	continue;
      }

      /* if we got this far, then we are to remove this instance */

      tsmFreePurl( model->location[j] );

      if ( --model->numLocations == 0 ) {
	if (tsmRemoveModelFromDatabasePos ( database, i )) {
	  --i;
	  break;
	}
      } else {
	for ( k = j; k < model->numLocations; ++k )
	  model->location[k] = model->location[k+1];
      }
    }
  }

  /* finally, we update the GeoSets array for the new set of datasets */

  tsmBuildGeosets( database );

}

/*
 * Synopsis:
 *   int tsmIsDatasetInDatabase( TsmDatabase const *database, char const *url,
 *                               char const *name, int set_id )
 *
 * Doc++ Description: *//** 
 *    Tests to see if a dataset exists in the database.
 *
 *    This function will take a TsmDatabase and search through it looking
 *    for a dataset with a name that matches the 'name' parameter, and
 *    an instance in that dataset that matches the 'url' string. Additionally
 *    the set ID must match the 'set_id' parameter. If such an entry is
 *    found in the database, then a result of TRUE is returned. Otherwise
 *    FALSE is returned. Because the set_id is part of the match, this
 *    function is normally used for matching DPSS dataset instances only.
 *
 * @param    database the TsmDatabase structure to remove from
 * @param    url the string with the full URL for the dataset being tested
 * @param    name the string with the name for the dataset being tested
 * @param    set_id the set ID number for the instance
 *
 * @return   TRUE if the dataset instance exists in the database
 *
 * @author   Martin Reddy
 * @version  7 July 1999
 *
 * @see      tsmUpdateDatabaseFromServer
 *
 */

static int
tsmIsDatasetInDatabase( TsmDatabase const *database, char const *url,
			char const *name, int set_id )
{
  const char           *funcName = "tsmIsDatasetInDatabase";
  TsmDataSet const     *dataset;
  TsmDataSetInst const *instance;
  int                   i, j, n;
  char                 *slash;

  /* check that we have valid pointers to play with */
  
  if ( database == NULL || url == NULL ) {
    tsmError( funcName, "database and url pointers must be non-NULL" );
    return FALSE;
  }

  /* find the length of the URL, up to the last '/' character */

  if ( ( slash = strrchr( url, '/' ) ) != NULL )
    n = slash - url;
  else
    n = strlen( url );

  /* now go through every instance of every dataset and see */
  /* if there is an instance with the same URL and set ID.  */

  for ( i = 0; i < database->numDataSets; ++i ) {
    dataset = &database->dataset[i];

    /* if the dataset name doesn't match then there's no chance */

    if ( strcmp( dataset->name, name ) != 0 )
      continue;

    for ( j = 0; j < dataset->numInstances; ++j ) {
      instance = &dataset->instance[j];

      /* compare the URL string (up to the dataset name) */

      if ( strncmp( instance->location->url, url, n ) == 0 &&
	   instance->set_id == set_id ) 
	return TRUE;

    }

  }

  return FALSE;
}

/*
 * Synopsis:
 *    int tsmAddMapToDatabase( TsmDatabase *database, TsmMap *map )
 *
 * Doc++ Description: *//** 
 *    Adds a new map entry to the database.
 *
 *    This function will take a TsmDatabase and a TsmMap structure. It will
 *    then attempt to add the contents of the TsmMap structure to the
 *    database. If a map entry already exists with this name, then a new
 *    entry is not created, but any unique location URLs are added to 
 *    the already existing database entry.
 *
 * @param    database the TsmDatabase structure to remove from
 * @param    map the TsmMap structure to add to the database
 *
 * @return   TRUE if the add was performed successfully
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      
 *
 */

int
tsmAddMapToDatabase( TsmDatabase *database, TsmMap *map )
{
  TsmMap    *new_map;
  int       i, n;

  if ( database == NULL || map == NULL || map->name == NULL ) return FALSE;

  /* check to see if we already have an entry for this map */

  for ( i = 0 ; i < database->numMaps; i++ ) {
    if ( strcmp( database->map[i].name, map->name ) == 0 ) {

      /* map already exists: just add the new location URL(s) to the */
      /* existing one. However, we also copy over the latest (u,v)   */
      /* corners in case they've changed since the last time.        */

      for ( n = 0; n < map->numLocations; n++ )
	tsmNewMapLocation( &database->map[i], map->location[n]->url );

      database->map[i].min_u = map->min_u;
      database->map[i].min_v = map->min_v;
      database->map[i].max_u = map->max_u;
      database->map[i].max_v = map->max_v;

      return TRUE;
    }
  }

  /* if we got this far, then this is a completely new map, */
  /* so we create a new entry and copy all of the data over */
  
  if ( tsmNewDatabaseMap( database ) == FALSE ) return FALSE;

  new_map = &database->map[database->numMaps-1];
  *new_map = *map;

  new_map->name = tsmMemStrdup( map->name );

  if ( map->geoname )
    new_map->geoname = tsmMemStrdup( map->geoname );

  new_map->location = (Purl **) tsmMemMalloc(map->numLocations*sizeof(Purl *));
  for ( i = 0; i < map->numLocations; i++ )
    new_map->location[i] = tsmCopyPurl( map->location[i] );
    
  return TRUE;
}

/*
 * Synopsis:
 *   TsmMap *tsmReadMap( char const *name, char const *url )
 *
 * Doc++ Description: *//** 
 *    Read the map.info file for a map.
 *
 *    This function will take a name and a URL for the directory containing
 *    a TerraVision map. It is assumed that this directory contains a file
 *    called map.info. This file specifies the (u,v) coordinates for the
 *    map and also specifies the name of the map image in that directory.
 *
 * @param    name (char *) the name of the map (map->name is set to this)
 * @param    url (char *) the url for the map files
 *
 * @return   An allocated TsmMap if successful, otherwise NULL
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmReadDatabase
 *
 */

TsmMap *
tsmReadMap( char const *name, char const *url )
{
  const char *funcName = "tsmReadMap";
  TsmMap     *map;
  char       newurl[512], buffer[512], tmpbuffer[512], *sep, *token, *data;
  FILE       *handle;

  if ( name == NULL || url == NULL ) return NULL;

  /* just check first of all that we have a valid url */

  if ( tsmIsUrlValid( url ) == FALSE ) {
    tsmError( funcName, "Invalid url: %s", url );
    return NULL;
  }

  /* create a new TsmMap structure */
  
  if ( ( map = (TsmMap *) tsmMemCalloc( 1, sizeof(TsmMap) ) ) == NULL ) {
    tsmError( funcName, "Could not allocate memory for a TsmMap" );
    return NULL;
  }

  map->name = tsmMemStrdup( name );

  /* try to parse the map.info file in the map directory */

  sep = ( url[strlen(url)-1] == '/' ) ? "" : "/";
  sprintf( newurl, "%s%s%s", url, sep, "map.info" );

  if ( ( handle = tsmOpenStream( newurl, "r" ) ) == NULL ) {
    tsmError( funcName, "Could not open the file: %s", newurl );
    tsmMemFree( map );
    return NULL;
  }

  /* Now, read each valid line from the file and try to parse it */

  while ( tspReadLineFromTspec( handle, buffer ) == TRUE ) {

    tspExtractTokenAndData( buffer, &token, &data );

    if ( strcmp( token, "map-name" ) == 0 ) {
      data = tspExtractNthStrdup( data, tmpbuffer, 0 );
      sprintf( newurl, "%s%s%s", url, sep, data );
      tsmNewMapLocation( map, newurl );
      tsmMemFree( data );
    } else if ( strcmp( token, "map-geoname" ) == 0 ) {
      map->geoname = tspExtractAllStrdup( data );
    } else if ( strcmp( token, "map-min-u" ) == 0 ) {
      map->min_u = atof( data );
    } else if ( strcmp( token, "map-min-v" ) == 0 ) {
      map->min_v = atof( data );
    } else if ( strcmp( token, "map-max-u" ) == 0 ) {
      map->max_u = atof( data );
    } else if ( strcmp( token, "map-max-v" ) == 0 ) {
      map->max_v = atof( data );
    } else
      tsmError( "unsupported map.info token: %s", token );
  }
  
  tsmCloseStream( handle );
  return map;
}

/*
 * Synopsis:
 *   TsmModel *tsmReadModel( char const *name, char const *url )
 *
 * Doc++ Description: *//** 
 *    Read the metadata for a VRML model.
 *
 *    This function will take a name and a URL for the directory containing
 *    a VRML site model and create a TsmModel structure. It is assumed that
 *    the directory contains a file called model.wrl, however currently 
 *    we do not check for the existence of this file or parse through it
 *    for a METADATA node.
 *
 * @param    name (char *) the name of the model (model->name is set to this)
 * @param    url (char *) the url for the model files
 *
 * @return   An allocated TsmModel if successful, otherwise NULL
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmReadDatabase
 *
 */

TsmModel *
tsmReadModel( char const *name, char const *url )
{
  const char *funcName = "tsmReadModel";
  TsmModel   *model;
  char       newurl[512], *sep;
  MFString   *mfstring;
  FILE       *handle;
  vsgWorld   *world;

  if ( name == NULL || url == NULL ) return NULL;

  /* just check first of all that we have a valid url */

  if ( tsmIsUrlValid( url ) == FALSE ) {
    tsmError( funcName, "Invalid url: %s", url );
    return NULL;
  }

  /* create a new TsmModel structure */
  
  if ( ( model = (TsmModel *) tsmMemCalloc( 1, sizeof(TsmModel) ) ) == NULL ) {
    tsmError( funcName, "Could not allocate memory for a TsmModel" );
    return NULL;
  }

  model->name = tsmMemStrdup( name );

  /* create the full model URL and add this to the TsmModel structure */

  sep = ( url[strlen(url)-1] == '/' ) ? "" : "/";
  sprintf( newurl, "%s%s%s", url, sep, "model.wrl" );

  tsmNewModelLocation( model, newurl );

  /* try to read the METADATA node for this model file */

  if ( ( handle = tsmOpenStream( newurl, "r" ) ) != NULL ) {

    world = vsgNewWorld( 0 );

    if ( vsgParseStream( world, handle, newurl ) == 0 ) {
      if ( ( mfstring = tsmGetMetadataNode( world ) ) != NULL ) {
	char buffer[512];

	/* parse the "name" field from the METADATA node */

	if ( tsmParseMetadataInfo( mfstring, "name", buffer ) )
	  model->title = tsmMemStrdup( buffer );

	/* parse the "geoname" field from the METADATA node */

	if ( tsmParseMetadataInfo( mfstring, "geoname", buffer ) )
	  model->geoname = tsmMemStrdup( buffer );
      }
      vsgDestroyWorld( world );
    }

    tsmCloseStream( handle );
  }

  return model;
}

/*
 * Synopsis:
 *    int tsmAddModelToDatabase( TsmDatabase *database, TsmModel *model )
 *
 * Doc++ Description: *//** 
 *    Adds a new model entry to the database.
 *
 *    This function will take a TsmDatabase and a TsmModel structure. It will
 *    then attempt to add the contents of the TsmModel structure to the
 *    database. If a model entry already exists with this name, then a new
 *    entry is not created, but any unique location URLs are added to 
 *    the already existing database entry.
 *
 * @param    database the TsmDatabase structure to remove from
 * @param    model the TsmModel structure to add to the database
 *
 * @return   TRUE if the add was performed successfully
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      
 *
 */

int
tsmAddModelToDatabase( TsmDatabase *database, TsmModel *model )
{
  TsmModel *new_model;
  int      i, n;

  if ( database == NULL || model == NULL || model->name == NULL ) return FALSE;

  /* check to see if we already have an entry for this model */

  for ( i = 0 ; i < database->numModels; i++ ) {
    if ( strcmp( database->model[i].name, model->name ) == 0 ) {

      /* model already exists: update its fields with the latest values */

      if ( database->model[i].title ) tsmMemFree( database->model[i].title );
      if ( model->title )
	database->model[i].title = tsmMemStrdup( model->title );
      else
	database->model[i].title = NULL;

      for ( n = 0; n < model->numLocations; n++ )
	tsmNewModelLocation( &database->model[i], model->location[n]->url );

      return TRUE;
    }
  }

  /* if we got this far, then this is a completely new model */
  /* so we create a new entry and copy all of the data over  */
  
  if ( tsmNewDatabaseModel( database ) == FALSE ) return FALSE;

  new_model = &database->model[database->numModels-1];
  *new_model = *model;

  new_model->name = tsmMemStrdup( model->name );
  if ( model->geoname ) 
    new_model->geoname = tsmMemStrdup( model->geoname );
  new_model->title = ( model->title ) ? tsmMemStrdup( model->title ) : NULL;
  new_model->location = (Purl **) tsmMemMalloc( model->numLocations * 
						sizeof(Purl *) );
  for ( i = 0; i < model->numLocations; i++ )
    new_model->location[i] = tsmCopyPurl( model->location[i] );
    
  return TRUE;
}

/*
 * Synopsis:
 *    void tsmBuildGeosets( TsmDatabase *database )
 *
 * Doc++ Description: *//** 
 *    Updates the list of GeoSets.
 *
 *    This function will go through the database looking for each OI
 *    dataset. It will then see if it can match these against any DEM in
 *    the database (match based upon same case-sensitive root name). If
 *    a DEM cannot be found based upon name, then we try to find the best
 *    DEM that completely covers the OI dataset. If we find a matchup
 *    between an OI and a DEM, then we also check to see if there are 
 *    any map or model entries that have the same root name as the OI and
 *    we add these to the geoset entry also.
 *
 *    We sort the list of geosets alphabetically.
 *
 * @param    database the TsmDatabase structure to be updated
 *
 * @return   void
 *
 * @author   Martin Reddy
 * @version  5 November 1999
 *
 * @see      
 *
 */

static void
tsmBuildGeosets( TsmDatabase *database )
{
  const char *funcName = "tsmBuildGeosets";
  int        i, j, found;
  TsmDataSet *curr_oi, *curr_dem;
  TsmMap     *curr_map;
  TsmModel   *curr_model;
  TsmGeoSet  *geoset;

  /* free the current geoset list - we're going to build from scratch */

  if ( database->numGeoSets > 0 ) {
    for ( i = 0; i < database->numGeoSets; i++ )
      tsmFreeGeoSetContents( &database->geoset[i] );
    if ( database->geoset != NULL )
      tsmMemFree( database->geoset );
  }

  /* reset the geoset list variables to signal an empty list */

  database->numGeoSets = 0;
  database->geoset     = NULL;

  /* go through every OI dataset and try to find a DEM to match with */

  for ( i = 0; i < database->numDataSets; i++ ) {

    curr_oi = &database->dataset[i];

    if ( curr_oi->type != TSM_IMAGE_DATASET ||
	 curr_oi->geoname == NULL )
      continue;

    /* okay, let's find a DEM with the same base name as this OI */

    for ( j = found = 0; j < database->numDataSets; j++ ) {

      curr_dem = &database->dataset[j];

      if ( curr_dem->type != TSM_DEM_DATASET ||
	   curr_dem->geoname == NULL )
	continue;

      /* we have an OI and a DEM, but do they match? */

      if ( tsmCompareGeoNames( curr_oi->geoname, curr_dem->geoname ) == 0 ) {
	found = TRUE;
	break;
      }

    }

    /* if couldn't match a DEM based upon geoname, then try spatial extent */

    if ( ! found ) {
      TsmDataSet *found_dem;

      for ( j = 0; j < database->numDataSets; j++ ) {

	curr_dem = &database->dataset[j];

	if ( curr_dem->type != TSM_DEM_DATASET ||
	     curr_dem->geoname == NULL )
	  continue;

	/* does the DEM completely cover the OI? */

	if ( tsmIsFootprintInside(&curr_oi->footprint, &curr_dem->footprint)){

	  /* save the highest resolution enclosing DEM as found_dem */
	  /* N.B. if the DEM and OI are in different coord systems  */
	  /* then we will need to convert to a standard resolution  */
	  /* representation (e.g. degrees -> meters).               */

	  if ( ! found ) {
	    found     = TRUE;
	    found_dem = curr_dem;
	  } else {

	    /* calculate resolutions in meters for comparisons */

	    float curr_min_res  = curr_dem->min_res;
	    float found_min_res = found_dem->min_res;

	    if ( curr_dem->footprint.coordsys == TSM_LATLONG_COORDSYS )
	      curr_min_res *= 110000.0; /* approx 1 deg in meters at equator */
	    if ( found_dem->footprint.coordsys == TSM_LATLONG_COORDSYS )
	      found_min_res *= 110000.0; /* approx 1 deg in meters at equtor */

	    if ( curr_min_res < found_min_res ) 
	      found_dem = curr_dem;
	  }
	  
	}
	
      }

      /* if we didn't find a matching DEM then that's all we can do */

      if ( found )
	curr_dem = found_dem;
      else
	continue;
      
      TSMTRACE(( TSMMASK|1, funcName, "Pairing DEM %s with OI %s",
		 curr_dem->name, curr_oi->name ));

    }

    /* otherwise, create a new GeoSet entry for the current OI + DEM */

    tsmNewDatabaseGeoSet( database );

    geoset = &database->geoset[database->numGeoSets-1];
    geoset->name     = tsmMemStrdup( curr_oi->geoname );
    geoset->oi_name  = tsmMemStrdup( curr_oi->name );
    geoset->dem_name = tsmMemStrdup( curr_dem->name );
    geoset->planet   = TSM_EARTH;

    /* see if there are any maps out there to associate with the geoset */

    for ( j = 0; j < database->numMaps; j++ ) {

      curr_map = &database->map[j];

      if ( tsmCompareGeoNames( curr_oi->geoname, curr_map->geoname ) == 0 ) {
	char **new_array;
	int  new_size;
	    
	/* add the current map name to this geoset */
	    
	new_size  = geoset->numMaps + 1;
	new_array = (char **) tsmMemRealloc( geoset->map_name, new_size *
					     sizeof(char *) );

	if ( new_array != NULL ) {
	  geoset->map_name = new_array;
	  geoset->numMaps  = new_size;
	  geoset->map_name[new_size-1] = tsmMemStrdup( curr_map->name );
	}
      }

    }
    
    /* see if there are any models out there to associate with the geoset */

    for ( j = 0; j < database->numModels; j++ ) {

      curr_model = &database->model[j];

      if ( tsmCompareGeoNames( curr_oi->geoname, curr_model->geoname ) == 0 ) {
	char **new_array;
	int  new_size;
	    
	/* add the current model name to this geoset */
	    
	new_size  = geoset->numModels + 1;
	new_array = (char **) tsmMemRealloc( geoset->model_name, new_size *
					     sizeof(char *) );

	if ( new_array != NULL ) {
	  geoset->model_name = new_array;
	  geoset->numModels  = new_size;
	  geoset->model_name[new_size-1] = tsmMemStrdup( curr_model->name );
	}
      }

    }

  }

  /* finally, we sort the list of geosets alphabetically */

  tsmSortGeoSets( database );
}

/*
 * Synopsis:
 *   int tsmUpdateDatabaseFromServerCB( TsmDatabase *, TsmServer *,
 *                                      void (*)(char *) )
 *
 * Doc++ Description: *//** 
 *   Updates a database with datasets from a server.
 *      
 *   This function will take an allocated database structure and a 
 *   server structure describing a TSM server. The function will then
 *   attempt to contact that server, query it for all of the datasets
 *   that it has available, and add instances for these to the database.
 *
 *   If the server is contactable, then the function will first delete
 *   all instances already in the database that describe datasets on that
 *   server. The new list is then built from scratch. This ensures, that
 *   any dataset removals will propagate from the server to the database.
 *
 *   Similarly, at the end we rebuild the list of GeoSets for the database
 *   based upon the new set of datasets, maps, and models. An OI and DEM
 *   are associated together based upon their root filename (i.e. without
 *   any extensions). If a DEM cannot be found for an OI based on this
 *   system, then we try to find the best enclosing DEM for the OI.
 *
 *   If a server cannot be contacted, then the function will return and
 *   the database will not be changed. This ensures that we don't delete
 *   information from the database just because a server is temporarily
 *   down.
 *
 *   If a dataset is named foo.jpg.oi, then this is taken as a jpeg
 *   instance of the dataset named foo.oi.
 *
 *   The callback parameter is a function that is called whenever this
 *   routine checks a new dataset. The callback function will be invoked
 *   with a string parameter that gives the name of the current set
 *   being analysed. This allows the calling application to provide som
 *   feedback on the progress of this process.
 *
 *   You can specify a TsmConnectionParams structure if you wish. 
 *   This structure will get passed to all tsmConnect calls that are made.
 *   Additionaly, the TSM_PARAM_FULL_DPSS_READ parameter is supported.
 *   If this is set to TRUE, then we perform a full read of
 *   all sets on a DPSS server, otherwise we only read the information
 *   for a dataset if it'd load date > last contact date for server.
 *
 * @param    database the database structure to be updated
 * @param    server the server structure containing the location of the 
 *           server to contact.
 * @param    param a connection parameter structure, or NULL
 * @param    callback a function to call whenever we start analysing a
 *           new dataset (or NULL)
 * @param    closure a closure pointer that is passed to the callback 
 *           function whenever it is invoked (can be NULL)
 *
 * @return   TRUE if successful, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  12 May 1999
 *
 * @see      tsmReadDatabase, tsmWriteDatabase, tsmRemoveServerSetsFromDatabase
 *
 */

int
tsmUpdateDatabaseFromServerCB( TsmDatabase *database, TsmServer *server,
			       TsmConnectionParams param,
			       TsmUpdateProc callback, void *closure )
{
  const char      *funcName = "tsmUpdateDatabaseFromServer";
  TsmConnection   connect;
  TsmDataSet      *dataset;
  TsmDataSetInst  *instance;
  TsmMap          *map;
  TsmModel        *model;
  TsmHandle       *tsmh = NULL;
  NameUrlIds      *set_list;
  Pyramid         *pyramid;
  PyramidInfo     *pinfo;
  ServerGroup     *sgroup;
  int             i, j, len, done, groupno, is_dpss;
  char            url[1024], *nameend;
#if DPSS
  PRTime          last_contact;
#endif

  /* check that we have valid pointers to play with */
  
  if ( database == NULL || server == NULL || server->location == NULL ) {
    tsmError( funcName, "database and server pointers must be non-NULL" );
    return FALSE;
  }

  /* read the set of all datasets available on the server */

  if ( ( set_list = tsmReadServer( server->location->url ) ) == NULL ) {
    tsmError( funcName, "Couldn't connect to %s", server->location->url );
    return FALSE;
  }

  /* If we got this far, then the server was contactable. In which */
  /* case we store the contact time. We also delete all of the     */
  /* entries that exist in the database for this server but which  */
  /* do not appear in the new list of Pyramids, e.g. datasets that */
  /* have been deleted since the last update.                      */

#if DPSS
  last_contact = server->last_contacted;
#endif
  server->last_contacted = PR_Now();

  tsmRemoveServerSetsFromDatabase( database, server, set_list );

  /* do we have a DPSS server? */

  is_dpss = ( strncmp( server->location->url, "x-iss:", 6 ) == 0 ||
	      strncmp( server->location->url, "x-dsm:", 6 ) == 0 ||
	      strncmp( server->location->url, "x-dpss:", 7 ) == 0 );

  if ( is_dpss ) {

    /* open the Tile Set Manager server - still need this for DPSS */

    if ( ( tsmh = tsmOpen( server->location->url ) ) == NULL ) {
      tsmError( funcName, "couldn't connect to %s", server->location->url );
      TSMTRINC(( TSMMASK|2, -1 ));
      return FALSE;
    }

    /* Load the pyramid database list */
    
    if ( tsmLoadPyramidDatabase( tsmh, "" ) == FALSE ) {
      tsmError( funcName, "couldn't open the pyramid database for server" );
      TSMTRINC(( TSMMASK|2, -1 ));
      return FALSE;
    }

    /* Open the connection to the dsm so that we can do dsmFetch calls */

    if ( tsmOpen_dsm( tsmh, tsmh->dbPurl ) == FALSE ) {
      tsmError( funcName, "Unable to open DSM." );
      TSMTRINC(( TSMMASK|2, -1 ));
      return FALSE;
    }

  }

  /* Go through each pyramid for this server */

  for ( i = 0; i < set_list->numItems; ++i ) {

    /* check to see if we have already dealt with a dataset of this */
    /* name. This may be the case for DPSS pyramids. The NameUrlId  */
    /* array has a separate entry for each DPSS pyramid instance,   */
    /* but we deal with all instances when we see the first name.   */

    done = FALSE;
    for ( j = 0; j < i && done == FALSE; ++j )
      if ( strcmp( set_list->names[i], set_list->names[j] ) == 0 )
	done = TRUE;

    if ( done ) continue;

    /* okay, we are about to deal with this new dataset, let's call */
    /* that callback function to tell our client where we're at.    */

    if ( callback ) {
      float percent = 100.0f * i / set_list->numItems;
      (*callback)( set_list->names[i], percent, closure );
    }

    /* Check to see if this is a map, model, or pyramid dataset */

    nameend = strrchr( set_list->names[i], '.' );

    /* Okay, do we have a map to play with here? */

    if ( nameend && strcmp( nameend, ".map" ) == 0 ) {
      TSMTRACE(( TSMMASK|2, funcName, "Checking map %s...",
		 set_list->names[i] ));

      /* parse the map.info file to fill out the TsmMap structure */

      if ( ! ( map = tsmReadMap( set_list->names[i], set_list->urls[i] ) ) ) {
	tsmError( funcName, "Unable to read map info %s", set_list->names[i] );
	continue;
      }

      /* try to add this possibly new entry to the database */

      tsmAddMapToDatabase( database, map );

      tsmFreeMap( map );
      tsmMemFree( map );
      continue;
    }

    /* No, then perhaps we have a VRML site model? */

    if ( nameend && strcmp( nameend, ".model" ) == 0 ) {
      TSMTRACE(( TSMMASK|2, funcName, "Checking model %s...",
		 set_list->names[i] ));

      /* build a new TsmMap structure and fill in its fields */

      if ( !( model = tsmReadModel( set_list->names[i], set_list->urls[i] ) )){
	tsmError( funcName, "Unable to set model info %s", set_list->names[i]);
	continue;
      }

      /* try to add this possibly new entry to the database */

      tsmAddModelToDatabase( database, model );

      tsmFreeModel( model );
      tsmMemFree( model );

      continue;
    }

#if DPSS
    {
      int fast_dpss;

      /* are we doing fast or full DPSS server reads? */

      if ( tsmIsParamSet( param, TSM_PARAM_FULL_DPSS_READ ) &&
	   param->full_dpss_read )
	fast_dpss = FALSE;
      else
	fast_dpss = TRUE;

      /* To speed up reading of a DPSS server, we only read the  */
      /* information for a DPSS set if it is new to the database */
      /* we don't do this if TSM_PARAMS_FULL_DPSS_READ is TRUE.  */

      if ( is_dpss && fast_dpss ) {
	dsmSetInfo dsi = dsmFetch( tsmh->dsmHandle, set_list->ids[i] );

	if ( tsmCompareDPSSDates( dsiDateLoaded(dsi), last_contact ) < 0 ) {
	  if ( tsmIsDatasetInDatabase( database, set_list->urls[i], 
				       set_list->names[i], set_list->ids[i] ) ) {

	    TSMTRACE(( TSMMASK|2, funcName, "Already know about %s...",
		       set_list->names[i] ));
	    continue;
	  }	  
	}
      }
    }
#endif

    /* Okay, we can process this entry as a dataset then... */

    TSMTRACE(( TSMMASK|2, funcName, "Checking dataset %s...\n",
	       set_list->names[i] ));

    if ( ( dataset = tsmMakeDataSet() ) == NULL ) {
      tsmError( funcName, "Unable to make a new dataset structure" );
      return FALSE;
    }

    /* Fill in the information that we get from the PyramidUrls */
    /* file. This is just the dataset name and it's URL.        */
    /* N.B. if the URL is x-dsm: then we change this to x-iss:  */
    /* because it is describing a dataset on a DPSS. Also, if   */
    /* the name ends in .jpg.oi, then we convert this to .oi    */
    /* because this is a jpg instance of a dataset with the     */
    /* same root name                                           */

    dataset->name = tsmMemStrdup( set_list->names[i] );

    if ( ( len = strlen( dataset->name ) ) > 7 ) {
      nameend = dataset->name + len - 7;
      if ( strcmp( nameend, ".jpg.oi" ) == 0 )
	strcpy( nameend, ".oi" );
    }

    strcpy( url, set_list->urls[i] );
    if ( strncmp( url, "x-dsm:", 6 ) == 0 )
      memcpy( url, "x-iss:", 6 );

    /* In order to get more information, we're going to have */
    /* to open up a connection to the particular dataset.    */

    if ( ( connect = tsmConnect( url, "r", param ) ) == NULL ) {
      tsmError( funcName, "Couldn't open dataset: %s", url );
      tsmFreeDataSet( dataset );
      continue;
    }

    pyramid = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );

    /* Well, we got the tspec, so now let's fill in the */
    /* dataset entry from the tspec file information.   */

    /* First of all, fill in all of the general dataset information */

    dataset->min_res  = pyramid->minResolution;
    dataset->max_res  = pyramid->maxResolution;
    dataset->tilesize = pyramid->tileTspec.numPixelsX;
    dataset->overlap  = pyramid->tileTspec.overLap[0];

    if ( pyramid->description != NULL )
      dataset->description = tsmMemStrdup( pyramid->description );

    if ( pyramid->geoName != NULL )
      dataset->geoname = tsmMemStrdup( pyramid->geoName );

    if ( pyramid->identityTspec.source_type != NULL )
      dataset->source_type = tsmMemStrdup( pyramid->identityTspec.source_type);
    else
      dataset->source_type = tsmMemStrdup( "unknown" );

    dataset->type = TSM_IMAGE_DATASET;
    if ( pyramid->identityTspec.type == tspDemType )
      dataset->type = TSM_DEM_DATASET;
    
    /* If we have a footprint defined in the pyramid tspec, then */
    /* take a copy of this, otherwise we have to make one from   */
    /* the various projection matrices that we have available    */

    if ( pyramid->footprint.coordsys == TSM_NONE_COORDSYS )
      tsmCalcFootprintFromXform( pyramid, dataset );
    else
      dataset->footprint = pyramid->footprint;

    /* Now fill in the particular instance information */

    if ( is_dpss ) {

      /* If we've got a DPSS dataset, then find out all of the  */
      /* different server combinations and set IDs that exist   */
      /* for the dataset name, creating a new instance for each */

      if ( ( pinfo = tsmGetPyramidInfo(tsmh, set_list->names[i]) ) == NULL ) {
	tsmError( funcName, "Couldn't get DPSS dataset information" );
	tsmFreePyramid( pyramid );
	tsmFreeDataSet( dataset );
	continue;
      }

      for ( groupno = 0; groupno < pinfo->numServerGroups; ++groupno ) {

	/* make the URL use the pyramid name rather than the set-id */

	if ( ( nameend = strrchr( url, '/' ) ) != NULL ) {
	  nameend++;
	  *nameend = '\0';
	  strcat( url, pinfo->name );
	}

	/* Create a new instance for each server group */

	tsmNewDataSetInstance( dataset );
	instance = &dataset->instance[dataset->numInstances-1];
	if ( ( instance->location = tsmParseUrl( url ) ) == NULL ) {
	  tsmError( funcName, "Couldn't parse URL: %s", url );
	  tsmFreeDataSet( dataset );
	  tsmFreePyramid( pyramid );
	  /* tsmFreePyramidInfo( pinfo ); */
	  continue;
	}

	sgroup = &pinfo->serverGroup[groupno];

	instance->set_id     = sgroup->setId;
	instance->numServers = sgroup->numServers;
	instance->serverName = (char **)
	  tsmMemMalloc( sizeof(char *) * sgroup->numServers );

	for ( j = 0; j < instance->numServers; ++j )
	  instance->serverName[j] = tsmMemStrdup( sgroup->serverHostNames[j] );

	if ( sgroup->imageFormat == tspJPEGCompressed )
	  instance->formats = TSM_JPEG_FORMAT;
	else 
	  instance->formats = TSM_LRBT_FORMAT;

      }

      /* tsmGetPyramidInfo allocation needs cleaning up before we can */
      /* use the free function - everything should be strdup'd into   */
      /* the structure, not just pointer-copied. Something to do later*/

      /* tsmFreePyramidInfo( pinfo ); */
      
    } else {

      /* If we have a file or http dataset, then all we need */
      /* to fill in is the location and formats fields.      */

      tsmNewDataSetInstance( dataset );
      instance = &dataset->instance[dataset->numInstances-1];
      if ( ( instance->location = tsmParseUrl( url ) ) == NULL ) {
	tsmError( funcName, "Couldn't parse URL: %s", url );
	tsmFreeDataSet( dataset );
	tsmFreePyramid( pyramid );
	continue;
      }

      /* Check for the lrbt / jpeg / lzw tiff files */

      if ( tsmCheckTIFFFormat( connect, TSM_LRBT_FORMAT ) )
	instance->formats |= TSM_LRBT_FORMAT;
      if ( tsmCheckTIFFFormat( connect, TSM_JPEG_FORMAT ) )
	instance->formats |= TSM_JPEG_FORMAT;
      if ( tsmCheckTIFFFormat( connect, TSM_LZW_FORMAT ) )
	instance->formats |= TSM_LZW_FORMAT;

      /* if no TIFF files exist, then we assume old lrbt-only format */
	
      if ( instance->formats == TSM_NONE_FORMAT )
	instance->formats = TSM_LRBT_FORMAT;
    }

    /* Now that we have created the new dataset entry, */
    /* add this to the database (if not there already) */

    tsmAddDataSetToDatabase( database, dataset );
    
    tsmDisconnect( connect );
    tsmFreeDataSet( dataset );
  }

  /* close the connection and then update the geoset list */

  if ( is_dpss )
    tsmClose( tsmh );        /* only ever opened for updating DPSS servers */

  tsmFreeNameUrlIds( set_list );

  tsmBuildGeosets( database );

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmUpdateDatabaseFromServer( TsmDatabase *database, TsmServer *server)
 *
 * Doc++ Description: *//** 
 *   Updates a database with datasets from a server.
 *      
 *   This function is basically just a wrapper to
 *   tsmUpdateDatabaseFromServerCB with the callback parameters set to NULL.
 *   It is provided for backwards compatibility with previous versions
 *   of the tsmApi.
 *
 * @param    database the database structure to be updated
 * @param    server the server structure containing the location of the 
 *           server to contact.
 *
 * @return   TRUE if successful, FALSE if failed.
 *
 * @author   Martin Reddy
 * @version  12 December 1997
 *
 * @see      tsmUpdateDataseFromServerCB
 *
 */

int
tsmUpdateDatabaseFromServer( TsmDatabase *database, TsmServer *server,
			     TsmConnectionParams param )
{
  return tsmUpdateDatabaseFromServerCB( database, server, param, NULL, NULL );
}

int
tsmAreServersTheSame( char *server1, char *server2 )
{
  int l1 = strlen( server1 ), l2 = strlen( server2 );
  return ( strncmp( server1, server2, ( l1 < l2 ) ? l1 : l2 ) == 0 );
}

/*
 * Synopsis:
 *   int tsmAddServerToDatabase( TsmDatabase *database, char *url ) 
 *
 * Doc++ Description: *//** 
 *   Adds a server to a database.
 *
 *   This function will take a URL for a TSM server and will add
 *   an entry for this in the specified database. If an entry already
 *   exists in the database for the server, then a new one will not
 *   be created.
 *
 * @param    database the TsmDatabase structure to be modified
 * @param    url the serverName to be added to the database, e.g.
 *           x-dsm://iss-1-atm.sri.magic.net
 *
 * @return   TRUE if returns with server in database, FALSE if failed
 *
 * @author   Martin Reddy
 * @version  14 December 1997
 *
 * @see      tsmRemoveServerFromDatabase
 *
 */

int
tsmAddServerToDatabase( TsmDatabase *database, char *url ) 
{
  const char *funcName = "tsmAddServerToDatabase";
  Purl       *purl;
  int        i;

  /* check that we have valid pointers to play with */
  
  if ( database == NULL || url == NULL ) {
    tsmError( funcName, "database and url pointers must be non-NULL" );
    return FALSE;
  }

  /* parse the url and used the parsed url for the subsequent check */

  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "couldn't parse URL: %s", url );
    return FALSE;
  }
    
  /* do we already have an entry for this server? */

  for ( i = 0; i < database->numServers; ++i )
    if ( tsmAreServersTheSame( database->server[i].location->url, purl->url ) )
      return TRUE;

  /* It's a new one, so add it... */

  tsmNewDatabaseServer( database );
  database->server[database->numServers-1].location = purl;
  
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmRemoveServerFromDatabase( TsmDatabase *database, char *url )
 *
 * Doc++ Description: *//** 
 *   deletes a server from a database.
 *
 *   This function will take a TsmDatabase structure and a TSM server
 *   URL. It will then remove any server entries in the database with
 *   the same URL.
 *
 * @param    database the TsmDatabase structure to be modified
 * @param    url the serverName to be removed from the database, e.g.
 *           x-dsm://iss-1-atm.sri.magic.net
 *
 * @return   TRUE if returns with server in database, FALSE if failed
 *
 * @author   Martin Reddy
 * @version  14 December 1997
 *
 * @see      tsmAddServerToDatabase
 *
 */

int
tsmRemoveServerFromDatabase( TsmDatabase *database, char *url )
{
  const char *funcName = "tsmRemoveServerFromDatabase";
  int        i;

  /* check that we have valid pointers to play with */
  
  if ( database == NULL || url == NULL ) {
    tsmError( funcName, "database and url pointers must be non-NULL" );
    return FALSE;
  }

  /* search for any server entries with this server name */

  for ( i = 0; i < database->numServers; ++i )
    if ( tsmAreServersTheSame( database->server[i].location->url, url ) )
      tsmDeleteDatabaseServer( database, i-- );

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmCompareGeoNames( char const *name1, char const *name2 )
 *
 * Doc++ Description: *//** 
 *   Compares the geoname of two dataset.
 *
 *   Given the geoname for two datasets, this function returns whether
 *   the two geonames match. The geonames could come from a pyramid
 *   a map, or a model dataset. The comparison is case sensitive.
 *
 * @param    name1 the first geoname to compare
 * @param    name1 the second geoname to compare
 *
 * @return   0 if the two are the same, != 0 if not.
 *
 * @author   Martin Reddy
 * @version  5 November 1999
 *
 * @see      tsmBuildGeoSets
 *
 */

static int
tsmCompareGeoNames( char const *name1, char const *name2 )
{
  if ( name1 == NULL || name2 == NULL )
    return 1;
  else
    return strcmp( name1, name2 );
}

/*
 * Synopsis:
 *   void tsmSortGeoSets( TsmDatabase *database )
 *
 * Description:
 *   Takes a database and sorts all of the geosets in that database
 *   so that they appear in alphabetical order. Also, this function
 *   will remove any geosets whose OI and DEM do not exist in the
 *   database.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 22 August 1998
 *
 */

void
tsmSortGeoSets( TsmDatabase *database )
{
  int       n, i, last;
  TsmGeoSet temp, *array;

  if ( database == NULL ) return;

  /* Do a selection sort on the geoset array. This works by finding */
  /* the lexicographically last geoset name in the array and making */
  /* this the last entry in the array. Then we find the next last   */
  /* geoset name and make this the next to last entry, etc.         */

  array = database->geoset;

  for ( n = database->numGeoSets - 1; n > 0; --n ) {

    /* Find the lexicographically last geoset name between 0..n */

    for ( i = 1, last = 0; i <= n; ++i )
      if ( strcasecmp( array[i].name, array[last].name ) > 0 ) 
	last = i;

    /* If it is not already the last entry, then swap it to the end */

    if ( last < n ) {
      temp = array[last];
      array[last] = array[n];
      array[n] = temp;
    }
  }
}

/*
 * Synopsis:
 *    TsmDataSet *tsmGetDataSet( TsmDatabase *database, char const *name )
 *
 * Doc++ Description: *//** 
 *    Searches a database for a particular dataset.
 *
 *    This function will search through all of the datasets in a 
 *    database and return a pointer to a dataset that has the
 *    specified name (case sensitive). If such a dataset does not exists
 *    in the database, or an error occurred, then NULL is returned.
 *
 * @param    database the database to use for the dataset lookup
 * @param    name the name field of the dataset to be matched against
 *
 * @return   The TsmDataSet structure that was found, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  19 March 1998
 *
 * @see      tsmReadDatabase, TsmDataSet
 *
 */

TsmDataSet *
tsmGetDataSet( TsmDatabase *database, char const *name )
{
  int i;
  
  if ( database == NULL || name == NULL || name[0] == '\0' )
    return NULL;

  for ( i = 0; i < database->numDataSets; ++i )
    if ( strcmp( database->dataset[i].name, name ) == 0 )
      return &database->dataset[i];

  return NULL;
}

/*
 * Synopsis:
 *    TsmMap *tsmGetMap( TsmDatabase *database, char const *name )
 *
 * Doc++ Description: *//** 
 *    Searches a database for a particular map.
 *
 *    This function will search through all of the maps in a 
 *    database and return a pointer to a map that has the
 *    specified name (case sensitive). If such a map does not exists in
 *    the database, or an error occurred, then NULL is returned.
 *
 * @param    database the database to use for the dataset lookup
 * @param    name the name field of the map to be matched against
 *
 * @return   The TsmMap structure that was found, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmReadDatabase, TsmMap
 *
 */

TsmMap *
tsmGetMap( TsmDatabase *database, char const *name )
{
  int i;
  
  if ( database == NULL || name == NULL || name[0] == '\0' )
    return NULL;

  for ( i = 0; i < database->numMaps; ++i )
    if ( strcmp( database->map[i].name, name ) == 0 )
      return &database->map[i];

  return NULL;
}

/*
 * Synopsis:
 *    TsmModel *tsmGetModel( TsmDatabase *database, char const *name )
 *
 * Doc++ Description: *//** 
 *    Searches a database for a particular model.
 *
 *    This function will search through all of the models in a 
 *    database and return a pointer to a model that has the
 *    specified name (case sensitive). If such a model does not exists in
 *    the database, or an error occurred, then NULL is returned.
 *
 * @param    database the database to use for the dataset lookup
 * @param    name the name field of the model to be matched against
 *
 * @return   The TsmModel structure that was found, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmReadDatabase, TsmModel
 *
 */

TsmModel *
tsmGetModel( TsmDatabase *database, char const *name )
{
  int i;
  
  if ( database == NULL || name == NULL || name[0] == '\0' )
    return NULL;

  for ( i = 0; i < database->numModels; ++i )
    if ( strcmp( database->model[i].name, name ) == 0 )
      return &database->model[i];

  return NULL;
}

/*
 * Synopsis:
 *    TsmGeoSet *tsmGetGeoSet( TsmDatabase *database, char const *name )
 *
 * Doc++ Description: *//** 
 *    Searches a database for a particular geoset.
 *
 *    This function will search through all of the geosets in a 
 *    database and return a pointer to a geoset that has the
 *    specified name (case sensitive). If such a geoset does not exists in
 *    the database, or an error occurred, then NULL is returned.
 *
 * @param    database the database to use for the dataset lookup
 * @param    name the name field of the geoset to be matched against
 *
 * @return   The TsmGeoSet structure that was found, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  22 March 1999
 *
 * @see      tsmReadDatabase, TsmGeoSet
 *
 */

TsmGeoSet *
tsmGetGeoSet( TsmDatabase *database, char const *name )
{
  int i;
  
  if ( database == NULL || name == NULL || name[0] == '\0' )
    return NULL;

  for ( i = 0; i < database->numGeoSets; ++i )
    if ( strcmp( database->geoset[i].name, name ) == 0 )
      return &database->geoset[i];

  return NULL;
}

/*
 * Synopsis:
 *   int tsmRemoveMapFromDatabasePos( TsmDatabase *database, int idx )
 *
 * Description:
 *   Removes the map at the given index from the database and free its
 *   memory.
 *
 * @param    database the database structure to search/update
 * @param    idx the position of the map in the map list
 *
 * @return   TRUE if the map was removed successfully
 *
 * @author   Lee Iverson
 * @version  13 November 2000
 *
 * @see      tsmRemoveMapFromDatabase
 *
 */

static int
tsmRemoveMapFromDatabasePos( TsmDatabase *database, int i )
{
  int n;
  
  if ( database == NULL || i < 0 || i >= database->numMaps)
    return FALSE;

  /* remove the map entry from the list of maps */

  database->numMaps--;
  tsmFreeMap( &database->map[i] );

  for ( n = i; n < database->numMaps; n++ )
    database->map[n] = database->map[n+1];

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmRemoveMapFromDatabase( TsmDatabase *database, char *mapname )
 *
 * Description:
 *   Removes the named map from the database
 *
 *   This function will check all of the map entries in the database
 *   for any that have the specified name. If a match is found, then
 *   that map entry is removed from the database and its memory freed.
 *
 * @param    database the database structure to search/update
 * @param    mapname the name of the map to remove (as in it's name field)
 *
 * @return   TRUE if the map was found and removed successfully
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmRemoveModelFromDatabase
 *
 */

int
tsmRemoveMapFromDatabase( TsmDatabase *database, char *mapname )
{
  int i, removed = FALSE;
  
  if ( database == NULL || mapname == NULL || mapname[0] == '\0' )
    return FALSE;

  /* remove the named map entry from the list of maps */

  for ( i = 0; i < database->numMaps; ) {
    if ( strcmp( database->map[i].name, mapname ) == 0 ) {
      removed = tsmRemoveMapFromDatabasePos (database, i);
    } else
      i++;
  }

  return removed;
}

/*
 * Synopsis:
 *   int tsmRemoveModelFromDatabasePos( TsmDatabase *database, int idx )
 *
 * Description:
 *   Removes the model at the given index from the database and free its
 *   memory.
 *
 * @param    database the database structure to search/update
 * @param    idx the position of the model in the model list
 *
 * @return   TRUE if the model was removed successfully
 *
 * @author   Lee Iverson
 * @version  13 November 2000
 *
 * @see      tsmRemoveModelFromDatabase
 *
 */

int
tsmRemoveModelFromDatabasePos ( TsmDatabase *database, int i )
{
  int n;
  
  if ( database == NULL || i < 0 || i >= database->numModels )
    return FALSE;

  /* remove the named model entry from the list of models */

  database->numModels--;
  tsmFreeModel( &database->model[i] );

  for ( n = i; n < database->numModels; n++ )
    database->model[n] = database->model[n+1];

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmRemoveModelFromDatabase( TsmDatabase *database, char *modelname )
 *
 * Description:
 *   Removes the named model from the database
 *
 *   This function will check all of the model entries in the database
 *   for any that have the specified name. If a match is found, then
 *   that model entry is removed from the database and its memory freed.
 *
 * @param    database the database structure to search/update
 * @param    modelname the name of the model to remove (as in it's name field)
 *
 * @return   TRUE if the model was found and removed successfully
 *
 * @author   Martin Reddy
 * @version  8 March 1999
 *
 * @see      tsmRemoveMapFromDatabase
 *
 */

int
tsmRemoveModelFromDatabase( TsmDatabase *database, char *modelname )
{
  int i, removed = FALSE;
  
  if ( database == NULL || modelname == NULL || modelname[0] == '\0' )
    return FALSE;

  /* remove the named model entry from the list of models */

  for ( i = 0; i < database->numModels; ) {
    if ( strcmp( database->model[i].name, modelname ) == 0 ) {
      removed = tsmRemoveModelFromDatabasePos (database, i);
    } else
      i++;
  }

  return removed;
}

/*
 * Synopsis:
 *   char *tsmGetGeoSetFromURL( TsmDatabase *database, char const *url )
 *
 * Description:
 *   Locate the GeoSet name given a dataset URL.
 *
 *   This function will search through all of the dataset instances
 *   in a database looking for one that matches the specified url.
 *   If found, then we find the name of the GeoSet that has that
 *   pyramid linked in and return the name of that GeoSet.
 *
 *   Do not attempt to do a free on the string returned - it is a copy
 *   of the pointer from the database and has not been strdup'd.
 *
 * @param    database the database structure to update
 * @param    url the url to locate in the database
 *
 * @return   a copy of the name of the GeoSet, or NULL if not found
 *
 * @author   Martin Reddy
 * @version  16 July 1999
 *
 */

char *
tsmGetGeoSetFromURL( TsmDatabase *database, char const *url )
{
  const char *funcName = "tsmGetGeoSetFromURL";
  Purl       *purl;
  int        i, j;
  char       *geoname = NULL;

  /* check the validity of the parameters that we were given */

  if ( database == NULL || url == NULL )
    return NULL;

  /* convert URL to a canonical form */

  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "Invalid URL: %s", url );
    return NULL;
  }

  /* do the search through each dataset instance */

  for ( i = 0; i < database->numDataSets && geoname == NULL; i++ ) {
    TsmDataSet *dataset = &database->dataset[i];
    for ( j = 0; j < dataset->numInstances && geoname == NULL; j++ ) {
      TsmDataSetInst *inst = &dataset->instance[j];

      if ( strcasecmp( inst->location->url, purl->url ) == 0 )
	geoname = dataset->name;
    }
  }

  tsmFreePurl( purl );

  if ( geoname == NULL ) return NULL;

  /* we have found a dataset with the URL in it - now find it's geoset */

  for ( i = 0; i < database->numGeoSets; i++ ) {
    TsmGeoSet *geoset = &database->geoset[i];
    if ( strcmp( geoset->oi_name, geoname ) == 0 ||
	 strcmp( geoset->dem_name, geoname ) == 0 )
      return geoset->name;
  }

  return NULL;
}

/*
 * Synopsis:
 *   int NameUrlIds *tsmReadServer( char const *url )
 *
 * Description:
 *   Read all TerraVision datasets in a localdisk directory.
 *
 *   This function accepts the name of a TerraVision dataset server.
 *   This can be a localdisk pathname, a file: URL, or an http: URL.
 *   It will then scan through all the directories on that server looking for
 *   directories that have an extension of either ".oi", ".dem",
 *   ".model", or ".map". It will then return a NameUrlIds array with
 *   all such entries. N.B. this array could be zero length if no such
 *   entries exist on the server.
 *
 * Returns: A list of available datasets, or NULL if couldn't read server.
 *
 * Author: Martin Reddy
 *
 * Date: 3 November 1999
 *
 */

NameUrlIds *
tsmReadServer( char const *url )
{
  const char *funcName = "tsmReadServer";
  Purl       *purl;
  NameUrlIds *nameUrlIds = NULL;
  char       newurl[512];

  if ( url == NULL || strlen( url ) == 0 )
    return NULL;

  /* If we have been given a url that ends in TileSets, then we */
  /* automatically append a Pyramids directory name. Previously */
  /* this was implied, but now this means that you can also     */
  /* enter a url that explicitly ends in a Pyramids directory.  */

  strcpy( newurl, url );

  if ( strstr( newurl + strlen( newurl ) - 10, "TileSets" ) != NULL ) {
    if ( newurl[strlen(newurl)-1] != '/' )
      strcat( newurl, "/" );
    strcat( newurl, "Pyramids" );
  }
  
  /* if we have been given a file: URL, then get the dir name */
  
  if ( ( purl = tsmParseUrl( newurl ) ) == NULL ) {
    tsmError( funcName, "Cannot parse the URL/path %s", newurl );
    return NULL;
  }

  /* call the protocol-specific method for reading the server */

  switch ( purl->protocol ) {
  case tsmLocalFile:
    nameUrlIds = tsmReadServer_local( purl );
    break;
  case tsmHTTP:
    nameUrlIds = tsmReadServer_web( purl );
    break;
  case tsmWMT:
    nameUrlIds = tsmReadServer_wms( purl );
    break;
#if DPSS
  case tsmISS:
  case tsmDPSS:
    nameUrlIds = tsmReadServer_dpss( purl );
    break;
#endif
  default:
    tsmNoProtocolYet( purl->protocol, funcName );
  }

  /* clean up and return the result */

  tsmFreePurl( purl );

  return nameUrlIds;
}

