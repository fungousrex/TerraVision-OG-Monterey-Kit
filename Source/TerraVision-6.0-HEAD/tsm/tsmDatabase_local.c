/*
 * Name: tsmDatabase_local.c
 *
 * Description:
 *   Contains routines for parsing and writing a dataset database file.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.ai.sri.com/~magic/tsmApi/dataset-db.html
 *
 * Function List:
 *    int tsmReadDatabase_local( TsmDatabase *database, char *filename )
 *    int tsmWriteDatabase_local( TsmDatabase *database, char *filename )
 *    int tsmNewDataSetInstance( TsmDataSet *dset )
 *    void tsmDeleteDataSetInstance( TsmDataSet *dataset, int index )
 *    int tsmNewDatabaseDataSet( TsmDatabase *dbase )
 *    void tsmDeleteDatabaseDataSet( TsmDatabase *database, int index )
 *    int tsmNewDatabaseServer( TsmDatabase *dbase )
 *    void tsmDeleteDatabaseServer( TsmDatabase *database, int index )
 *    int tsmNewDatabaseGeoSet( TsmDatabase *database );
 *    int tsmParseDatabaseLine( char *buffer, TsmDatabase *dbase, int * )
 *    NameUrlIds *tsmReadServer_local( Purl const *purl )
 *
 * Dependencies:
 *
 * Revision History:
 *    December 1997 - Initial version by Martin Reddy
 *
 *    $Id: tsmDatabase_local.c,v 1.15.6.3 2001/06/20 22:26:17 reddy Exp $
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

#include <dirent.h>
#include <sys/stat.h>

/* a macro to define a hashing of a string to speed parsing of keywords */

#define HASH_KEY(x,y,z) ((x)+((y)<<8)+((z)<<16))

/* define offsets into the tagstate array for each tag - see parse routine */

#define TAG_DATASET    0 
#define TAG_INSTANCE   1
#define TAG_SERVER     2
#define TAG_GEOSET     3
#define TAG_MODEL      4
#define TAG_MAP        5
#define TAG_ARRAY_SIZE 6      /* size of tagstate array = last tag no. + 1 */

int tsmReadOldDatabase_local( TsmDatabase *database, FILE *stream );


/*
 * Synopsis:
 *   int tsmNewDataSetInstance( TsmDataSet *dataset )
 *
 * Description:
 *   Grows the array for the instances in the dataset structure and
 *   allocates a new TsmDataSetInst structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 10 December 1997
 *
 */

int
tsmNewDataSetInstance( TsmDataSet *dset )
{
  const char     *funcName = "tsmNewDataSetInstance";
  TsmDataSetInst *new_inst;
  int            new_size;

  if ( dset == NULL ) {
    tsmError( funcName, "NULL pointer passed for dataset" );
    return FALSE;
  }

  /* try to realloc the instance array to hold one more entry */

  new_size = dset->numInstances + 1;
  new_inst = (TsmDataSetInst *)
    tsmMemManagerRealloc( dset->instance, new_size * sizeof(TsmDataSetInst) );

  if ( new_inst == NULL ) {
    tsmError( funcName, "couldn't grow the instance array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now update the */
  /* dataset fields & initialise a new DataSetInst at end of array */

  memset( new_inst + dset->numInstances, 0, sizeof(TsmDataSetInst) );

  dset->instance     = new_inst;
  dset->numInstances = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmDeleteDataSetInstance( TsmDataSet *dataset, int index )
 *
 * Description:
 *   Takes a TsmDataSet and an index value for an instance in that
 *   dataset, and then removes that instance from the dataset. This
 *   involves freeing the memory for the instance and also performing
 *   any necessary shuffling down of all instances after the one 
 *   deleted in the instance array.
 *
 * Externals: None
 *
 * Returns: Nothing
 *
 * Author: Martin Reddy
 *
 * Date: 13 December 1997
 *
 */

void
tsmDeleteDataSetInstance( TsmDataSet *dataset, int index )
{
  const char *funcName = "tsmDeleteDataSetInstance";
  int        i;

  /* Check that we have a valid dataset and it has enough instances */

  if ( dataset == NULL || dataset->numInstances <= index ) {
    tsmError( funcName, "No instance for dataset %p", dataset );
    return;
  }

  /* free the memory for the specified instance, and if this was the */
  /* last instance in the array, then remove the array completely.   */

  tsmFreeDataSetInst( &dataset->instance[index] );

  if ( --dataset->numInstances == 0 ) {
    tsmMemManagerFree( dataset->instance );
    dataset->instance = NULL;
    return;
  }

  /* shuffle the other instances down to fill in the space    */
  /* we have just created by removing the specified instance  */

  for ( i = index; i < dataset->numInstances; ++i )
    dataset->instance[i] = dataset->instance[i+1];
}

/*
 * Synopsis:
 *   int tsmNewDatabaseDataSet( TsmDatabase *database );
 *
 * Description:
 *   Grows the array for the datasets in the database structure and
 *   allocates a new TsmDataSet structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 9 December 1997
 *
 */

int
tsmNewDatabaseDataSet( TsmDatabase *dbase )
{
  const char *funcName = "tsmNewDatabaseDataSet";
  TsmDataSet *new_set;
  int        new_size;

  if ( dbase == NULL ) {
    tsmError( funcName, "NULL pointer passed for database" );
    return FALSE;
  }

  /* try to realloc the dataset array to hold one more entry */

  new_size = dbase->numDataSets + 1;
  new_set  = (TsmDataSet *)
    tsmMemManagerRealloc( dbase->dataset, new_size * sizeof(TsmDataSet) );

  if ( new_set == NULL ) {
    tsmError( funcName, "couldn't grow the dataset array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now just update */
  /* the database fields & initialise a new DataSet at end of array */

  memset( new_set + dbase->numDataSets, 0, sizeof(TsmDataSet) );

  dbase->dataset     = new_set;
  dbase->numDataSets = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmNewDatabaseMap( TsmDatabase *database );
 *
 * Description:
 *   Grows the array for the maps in the database structure and
 *   allocates a new TsmMap structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 6 March 1999
 *
 */

int
tsmNewDatabaseMap( TsmDatabase *dbase )
{
  const char *funcName = "tsmNewDatabaseMap";
  TsmMap     *new_map;
  int        new_size;

  if ( dbase == NULL ) {
    tsmError( funcName, "NULL pointer passed for database" );
    return FALSE;
  }

  /* try to realloc the map array to hold one more entry */

  new_size = dbase->numMaps + 1;
  new_map  = (TsmMap *) tsmMemRealloc( dbase->map, new_size * sizeof(TsmMap) );

  if ( new_map == NULL ) {
    tsmError( funcName, "couldn't grow the map array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now just update */
  /* the database fields & initialise a new TsmMap at end of array. */

  memset( new_map + dbase->numMaps, 0, sizeof(TsmMap) );

  dbase->map     = new_map;
  dbase->numMaps = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmNewDatabaseModel( TsmDatabase *database );
 *
 * Description:
 *   Grows the array for the models in the database structure and
 *   allocates a new TsmModel structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 6 March 1999
 *
 */

int
tsmNewDatabaseModel( TsmDatabase *dbase )
{
  const char *funcName = "tsmNewDatabaseModel";
  TsmModel   *new_model;
  int        new_size;

  if ( dbase == NULL ) {
    tsmError( funcName, "NULL pointer passed for database" );
    return FALSE;
  }

  /* try to realloc the model array to hold one more entry */

  new_size  = dbase->numModels + 1;
  new_model = (TsmModel *)
    tsmMemRealloc( dbase->model, new_size * sizeof(TsmModel) );

  if ( new_model == NULL ) {
    tsmError( funcName, "couldn't grow the model array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now just update  */
  /* the database fields & initialise a new TsmModel at end of array */

  memset( new_model + dbase->numModels, 0, sizeof(TsmModel) );

  dbase->model     = new_model;
  dbase->numModels = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   void tsmDeleteDatabaseDataSet( TsmDatabase *database, int index )
 *
 * Description:
 *   Takes a TsmDatabase and an index value for a dataset in that
 *   database, and then removes that dataset from the database. This
 *   involves freeing the memory for the dataset (and any instances)
 *   and also performing any necessary shuffling down of all datasets
 *   after the one deleted in the dataset array.
 *
 * Externals: None
 *
 * Returns: Nothing
 *
 * Author: Martin Reddy
 *
 * Date: 13 December 1997
 *
 */

void
tsmDeleteDatabaseDataSet( TsmDatabase *database, int index )
{
  const char *funcName = "tsmDeleteDatabaseDataSet";
  int        i;

  /* Check that we have a valid database and it has enough datasets */

  if ( database == NULL || database->numDataSets <= index ) {
    tsmError( funcName, "No dataset for database %p", database );
    return;
  }

  /* free the memory for the specified dataset, and if this was the */
  /* last dataset in the array, then remove the array completely.   */

  tsmFreeDataSetContents( &database->dataset[index] );

  if ( --database->numDataSets == 0 ) {
    tsmMemManagerFree( database->dataset );
    database->dataset = NULL;
    return;
  }

  /* shuffle the other datasets down to fill in the space   */
  /* we have just created by removing the specified dataset */

  for ( i = index; i < database->numDataSets; ++i )
    database->dataset[i] = database->dataset[i+1];
}

/*
 * Synopsis:
 *   int tsmNewDatabaseGeoSet( TsmDatabase *database );
 *
 * Description:
 *   Grows the array for the geosets in the database structure and
 *   allocates a new TsmGeoSet structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 19 March 1998
 *
 */

int
tsmNewDatabaseGeoSet( TsmDatabase *dbase )
{
  const char *funcName = "tsmNewDatabaseGeoSet";
  TsmGeoSet  *new_set;
  int        new_size;

  if ( dbase == NULL ) {
    tsmError( funcName, "NULL pointer passed for database" );
    return FALSE;
  }

  /* try to realloc the dataset array to hold one more entry */

  new_size = dbase->numGeoSets + 1;
  new_set  = (TsmGeoSet *)
    tsmMemRealloc( dbase->geoset, new_size * sizeof(TsmGeoSet) );

  if ( new_set == NULL ) {
    tsmError( funcName, "couldn't grow the geoset array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now just update */
  /* the database fields & initialise a new GeoSet at end of array  */

  memset( new_set + dbase->numGeoSets, 0, sizeof(TsmGeoSet) );

  dbase->geoset     = new_set;
  dbase->numGeoSets = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmNewDatabaseServer( TsmDatabase *database );
 *
 * Description:
 *   Grows the array for the servers in the database structure and
 *   allocates a new TsmServer structure at the end of the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 10 December 1997
 *
 */

int
tsmNewDatabaseServer( TsmDatabase *dbase )
{
  const char *funcName = "tsmNewDatabaseServer";
  TsmServer  *new_serv;
  int        new_size;

  if ( dbase == NULL ) {
    tsmError( funcName, "NULL pointer passed for database" );
    return FALSE;
  }

  /* try to realloc the dataset array to hold one more entry */

  new_size = dbase->numServers + 1;
  new_serv = (TsmServer *)
    tsmMemManagerRealloc( dbase->server, new_size * sizeof(TsmServer) );

  if ( new_serv == NULL ) {
    tsmError( funcName, "couldn't grow the server array to %d", new_size );
    return FALSE;
  }

  /* the reallocation worked if we get this far. So now just update */
  /* the database fields & initialise a new DataSet at end of array */

  memset( new_serv + dbase->numServers, 0, sizeof(TsmServer) );

  dbase->server     = new_serv;
  dbase->numServers = new_size;
  dbase->server[new_size-1].last_contacted = (time_t) -1;

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmAddGeoSetModel( TsmGeoset *geoset, char const *modelname )
 *
 * Description:
 *   Grows the array for the model names in the TsmGeoSet structure and
 *   inserts a new entry for this name into the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 8 March 1999
 *
 */

int
tsmAddGeoSetModel( TsmGeoSet *geoset, char const *modelname )
{
  const char *funcName = "tsmAddGeoSetModel";
  int        new_size;
  char       **new_array;

  if ( geoset == NULL || modelname == NULL ) return FALSE;

  /* try to realloc the name array to hold one more entry */

  new_size = geoset->numModels + 1;
  new_array = (char **) tsmMemRealloc( geoset->model_name, sizeof(char *)
				       * new_size );

  if ( new_array == NULL ) {
    tsmError( funcName, "Couldn't grow the name array in the geoset by one" );
    return FALSE;
  }

  /* now add the new name to the end of the array */

  geoset->numModels = new_size;
  geoset->model_name = new_array;
  geoset->model_name[geoset->numModels-1] = tsmMemStrdup( modelname );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmAddGeoSetMap( TsmGeoset *geoset, char const *mapname )
 *
 * Description:
 *   Grows the array for the map names in the TsmGeoSet structure and
 *   inserts a new entry for this name into the array.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 8 March 1999
 *
 */

int
tsmAddGeoSetMap( TsmGeoSet *geoset, char const *mapname )
{
  const char *funcName = "tsmAddGeoSetMap";
  int        new_size;
  char       **new_array;

  if ( geoset == NULL || mapname == NULL ) return FALSE;

  /* try to realloc the name array to hold one more entry */

  new_size = geoset->numMaps + 1;
  new_array = (char **) tsmMemRealloc( geoset->map_name, sizeof(char *)
				       * new_size );

  if ( new_array == NULL ) {
    tsmError( funcName, "Couldn't grow the name array in the geoset by one" );
    return FALSE;
  }

  /* now add the new name to the end of the array */

  geoset->numMaps = new_size;
  geoset->map_name = new_array;
  geoset->map_name[geoset->numMaps-1] = tsmMemStrdup( mapname );
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmNewModelLocation( TsmModel *model, char const *url )
 *
 * Description:
 *   Grows the array for the model locations in the TsmModel structure and
 *   inserts a new Purl entry for this url into the array. If the specified
 *   url already exists in the model structure, then this function will
 *   do nothing.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 7 March 1999
 *
 */

int
tsmNewModelLocation( TsmModel *model, char const *url )
{
  const char *funcName = "tsmNewModelLocation";
  int        new_size, n;
  Purl       *purl, **new_loc;

  if ( model == NULL || url == NULL ) return FALSE;

  /* Try to parse the URL first, in case it is not valid */
  
  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "Invalid URL for model location" );
    return FALSE;
  }

  /* does this URL already exist in the model structure? */

  for ( n = 0; n < model->numLocations; n++ ) {
    if ( strcmp( model->location[n]->url, purl->url ) == 0 ) {
      tsmFreePurl( purl );
      return FALSE;
    }
  }

  /* try to realloc the location array to hold one more entry */

  new_size = model->numLocations + 1;
  new_loc = (Purl **) tsmMemRealloc( model->location, sizeof(Purl *)*new_size);

  if ( new_loc == NULL ) {
    tsmError( funcName, "Couldn't grow the Purl array in TsmModel by one" );
    tsmFreePurl( purl );
    return FALSE;
  }

  /* now add the new Purl to the end of the array */

  model->numLocations = new_size;
  model->location = new_loc;
  model->location[model->numLocations-1] = purl;
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmNewMapLocation( TsmMap *map, char const *url )
 *
 * Description:
 *   Grows the array for the map locations in the TsmMap structure and
 *   inserts a new Purl entry for this url into the array. If the specified
 *   url already exists in the map structure, then this function will
 *   do nothing.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 7 March 1999
 *
 */

int
tsmNewMapLocation( TsmMap *map, char const *url )
{
  const char *funcName = "tsmNewMapLocation";
  int        new_size, n;
  Purl       *purl, **new_loc;

  if ( map == NULL || url == NULL ) return FALSE;

  /* Try to parse the URL first, in case it is not valid */
  
  if ( ( purl = tsmParseUrl( url ) ) == NULL ) {
    tsmError( funcName, "Invalid URL for map location" );
    return FALSE;
  }

  /* does this URL already exist in the map structure? */

  for ( n = 0; n < map->numLocations; n++ ) {
    if ( strcmp( map->location[n]->url, purl->url ) == 0 ) {
      tsmFreePurl( purl );
      return FALSE;
    }
  }

  /* try to realloc the location array to hold one more entry */

  new_size = map->numLocations + 1;
  new_loc = (Purl **) tsmMemRealloc( map->location, sizeof(Purl *)*new_size);

  if ( new_loc == NULL ) {
    tsmError( funcName, "Couldn't grow the Purl array in TsmMap by one" );
    tsmFreePurl( purl );
    return FALSE;
  }

  /* now add the new Purl to the end of the array */

  map->numLocations = new_size;
  map->location = new_loc;
  map->location[map->numLocations-1] = purl;
  return TRUE;
}

/*
 * Synopsis:
 *   void tsmDeleteDatabaseServer( TsmDatabase *database, int index )
 *
 * Description:
 *   Takes a TsmDatabase and an index value for a server in that
 *   database, and then removes that server from the database. This
 *   involves freeing the memory for the server and also performing
 *   any necessary shuffling down of all servers after the one deleted
 *
 * Externals: None
 *
 * Returns: Nothing
 *
 * Author: Martin Reddy
 *
 * Date: 13 December 1997
 *
 */

void
tsmDeleteDatabaseServer( TsmDatabase *database, int index )
{
  const char *funcName = "tsmDeleteDatabaseServer";
  int        i;

  /* Check that we have a valid database and it has enough servers */

  if ( database == NULL || database->numServers <= index ) {
    tsmError( funcName, "No server for database %p", database );
    return;
  }

  /* free the memory for the specified server, and if this was the */
  /* last server in the array, then remove the array completely.   */

  tsmFreeServer( &database->server[index] );

  if ( --database->numServers == 0 ) {
    tsmMemManagerFree( database->server );
    database->server = NULL;
    return;
  }

  /* shuffle the other servers down to fill in the space   */
  /* we have just created by removing the specified server */

  for ( i = index; i < database->numServers; ++i )
    database->server[i] = database->server[i+1];
}

/*
 * Synopsis:
 *   int tsmParseDatabaseLine( char *buffer, TsmDatabase *dbase )
 *
 * Description:
 *   This function will take a line of text read from a dataset database
 *   and will try to parse it into the supplied TsmDatabase structure.
 *   The routine will deal with
 *   allocating new structures were necessary for you.
 *
 *   The tagstate array is used to keep track of whether any of the 
 *   supported tags are currently opened. Currently we support three 
 *   tags, so you should pass in an array of 3 ints, all initialised
 *   to FALSE at first, e.g. "int tagstate[3] = {0,0,0};" Internally,
 *   the entries of this array are allocated as follows:
 *      tagstate[TAG_DATASET]  = TRUE while <dataset> tag is open
 *      tagstate[TAG_INSTANCE] = TRUE while <instance> tag is open
 *      tagstate[TAG_SERVER]   = TRUE while <server> tag is open
 *      tagstate[TAG_GEOSET]   = TRUE while <geoset> tag is open
 *      tagstate[TAG_MODEL]    = TRUE while <model> tag is open
 *      tagstate[TAG_MAP]      = TRUE while <map> tag is open
 *
 *   If a bounding box is specified in UTM coordinates, then the Lat/Long
 *   bounding box field is filled in using tsmApi's UTM<->Lat/Long conversion 
 *   functions; and vice-versa.
 *
 *   The parsing routine will check for the following errors in the
 *   database file and return an error on these:
 *     - </tag> before a <tag>
 *     - nesting of <tag> blocks
 *     - <instance> can only occur inside <dataset>
 *     - check that <tag> contains all required fields when </tag> found
 *     - keyword/value pairs outside of a <dataset> group
 *     - min/max-resolution values without a unit string
 *     - check that min-resolution <= max_resolution
 *     - invalid type value
 *     - unparsable URLs for location fields
 *
 * Externals: None
 *
 * Returns: TRUE if okay, FALSE if a tag structure error occurred.a
 *
 * Author: Martin Reddy
 *
 * Date: 16 October 1997
 *
 */

int
tsmParseDatabaseLine( char *buffer, TsmDatabase *dbase, int *tagstate )
{
  const char       *funcName = "tsmParseDatabaseLine";
  TsmDataSet       *curr_set = NULL;
  TsmDataSetInst   *curr_inst = NULL;
  TsmServer        *curr_server = NULL;
  TsmGeoSet        *curr_geoset = NULL;
  TsmModel         *curr_model = NULL;
  TsmMap           *curr_map = NULL;
  LatLongFootprint *ll_foot;
  UTMFootprint     *utm_foot;
  GeoFootprint     *geo_foot;
  LVCSFootprint    *lvcs_foot;
  char             *token, *data;
  char             temp_buffer[MAX_TSPEC_LINE_LENGTH];
  int              len, i;

  /* Extract the keyword and value pair from the line. We allow an */
  /* optional colon (:) after the keyword, so remove that if found */
  
  tspExtractTokenAndData( buffer, &token, &data );

  if ( token[0] == '\0' ) return TRUE;

  len = strlen( token );
  if ( len > 0 && token[len-1] == ':' )
    token[len-1] = '\0';

  /* Get a pointer to the current dataset, instance, and server being */
  /* updated in the dbase. If the relevant tag hasn't been opened yet */
  /* then these will remain NULL values.                              */
  
  if ( dbase->numDataSets > 0 ) {
    curr_set = &dbase->dataset[dbase->numDataSets-1];
    if ( curr_set->numInstances > 0 ) 
      curr_inst = &curr_set->instance[curr_set->numInstances-1];
  }

  if ( dbase->numServers > 0 )
    curr_server = &dbase->server[dbase->numServers-1];

  if ( dbase->numGeoSets > 0 )
    curr_geoset = &dbase->geoset[dbase->numGeoSets-1];

  if ( dbase->numModels > 0 )
    curr_model = &dbase->model[dbase->numModels-1];

  if ( dbase->numMaps > 0 )
    curr_map = &dbase->map[dbase->numMaps-1];

  /* Now try to parse the line. We are a bit more clever here than a  */
  /* list of if/elseif's by creating a hash key from the first three  */
  /* characters of the keyword and using this in a switch() statement */

  switch ( HASH_KEY(token[0],token[1],token[2]) ) {
    
    /* The <GEOSET> tag */

  case HASH_KEY('<','g','e'):
    if ( strcmp( token, "<geoset>" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == TRUE ) {
	tsmError( funcName, "illegal nesting of <geoset> tag" );
	return FALSE;
      }

      tsmNewDatabaseGeoSet( dbase );
      tagstate[TAG_GEOSET] = TRUE;
    }
    break;

    /* The </GEOSET> tag - do validity check on geoset just read */

  case HASH_KEY('<','/','g'):
    if ( strcmp( token, "</geoset>" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "</geoset> appears with no <geoset> tag" );
	return FALSE;
      }
      tagstate[TAG_GEOSET] = FALSE;

      if ( curr_geoset->name == NULL ) {
	tsmError( funcName, "Geoset contains no name data!" );
	return FALSE;
      }

      if ( curr_geoset->oi_name == NULL ) {
	tsmError( funcName, "Dataset contains no imagery name data!" );
	return FALSE;
      }

      if ( curr_geoset->dem_name == NULL ) {
	tsmError( funcName, "Dataset contains no DEM name data!" );
	return FALSE;
      }

    }
    break;

    /* The <DATASET> tag */

  case HASH_KEY('<','d','a'):
    if ( strcmp( token, "<dataset>" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == TRUE ) {
	tsmError( funcName, "illegal nesting of <dataset> tag" );
	return FALSE;
      } if ( tagstate[TAG_SERVER] == TRUE ) {
	tsmError( funcName, "<dataset> found before </server>" );
	return FALSE;
      }

      tsmNewDatabaseDataSet( dbase );
      tagstate[TAG_DATASET] = TRUE;
    }
    break;

    /* The </DATASET> tag - do validity check on dataset just read */

  case HASH_KEY('<','/','d'):
    if ( strcmp( token, "</dataset>" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "</dataset> appears with no <dataset> tag" );
	return FALSE;
      }
      tagstate[TAG_DATASET] = FALSE;

      if ( curr_set->name == NULL ) {
	tsmError( funcName, "Dataset contains no name data!" );
	return FALSE;
      }

      if ( curr_set->min_res == 0 ) {
	tsmError( funcName, "Dataset contains no min-resolution data!" );
	return FALSE;
      }

      if ( curr_set->max_res == 0 ) {
	tsmError( funcName, "Dataset contains no min-resolution data!" );
	return FALSE;
      }

      if ( curr_set->min_res > curr_set->max_res ) {
	tsmError( funcName, "min-resolution larger than max-resolution!" );
	return FALSE;
      }

      if ( curr_set->source_type == NULL )
	curr_set->source_type = tsmMemManagerStrdup( "unknown" );

    }
    break;

    /* The <INSTANCE> tag */

  case HASH_KEY('<','i','n'):
    if ( strcmp( token, "<instance>" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == TRUE ) {
	tsmError( funcName, "illegal nesting of <instance> tag" );
	return FALSE;
      } else if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "<instance> must appear inside <dataset>" );
	return FALSE;
      }

      tsmNewDataSetInstance( curr_set );
      tagstate[TAG_INSTANCE] = TRUE;
    }
    break;

    /* The </INSTANCE> tag - do validity check on instance just read */

  case HASH_KEY('<','/','i'):
    if ( strcmp( token, "</instance>" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == FALSE ) {
	tsmError( funcName, "</instance> appears with no <instance> tag" );
	return FALSE;
      }
      tagstate[TAG_INSTANCE] = FALSE;

      if ( curr_inst->location == NULL ) {
	tsmError( funcName, "Instance contains no location data!" );
	return FALSE;
      }

      if ( curr_inst->location->protocol == tsmISS ||
	   curr_inst->location->protocol == tsmDPSS ) {
	if ( curr_inst->serverName == NULL ) {
	  tsmError( funcName, "DPSS Instance contains no server data!" );
	  return FALSE;
	}
      }
    }
    break;

    /* The <SERVER> tag */

  case HASH_KEY('<','s','e'):
    if ( strcmp( token, "<server>" ) == 0 ) {
      if ( tagstate[TAG_SERVER] == TRUE ) {
	tsmError( funcName, "illegal nesting of <server> tag" );
	return FALSE;
      } if ( tagstate[TAG_DATASET] == TRUE ) {
	tsmError( funcName, "<server> found before </dataset>" );
	return FALSE;
      }

      tsmNewDatabaseServer( dbase );
      tagstate[TAG_SERVER] = TRUE;
    }
    break;

    /* The </SERVER> tag - do validity check on server just read */

  case HASH_KEY('<','/','s'):
    if ( strcmp( token, "</server>" ) == 0 ) {
      if ( tagstate[TAG_SERVER] == FALSE ) {
	tsmError( funcName, "</server> appears with no <server> tag" );
	return FALSE;
      }
      tagstate[TAG_SERVER] = FALSE;

      if ( curr_server->location == NULL ) {
	tsmError( funcName, "Server contains no location data!" );
	return FALSE;
      }
    }
    break;

    /* The <MODEL> tag */

  case HASH_KEY('<','m','o'):
    if ( strcmp( token, "<model>" ) == 0 ) {
      if ( tagstate[TAG_MODEL] == TRUE ) {
	tsmError( funcName, "illegal nesting of <model> tag" );
	return FALSE;
      }

      tsmNewDatabaseModel( dbase );
      tagstate[TAG_MODEL] = TRUE;
    }
    break;

    /* The </MODEL> and </MAP> tags - do validity check on server just read */

  case HASH_KEY('<','/','m'):
    if ( strcmp( token, "</model>" ) == 0 ) {
      if ( tagstate[TAG_MODEL] == FALSE ) {
	tsmError( funcName, "</model> appears with no <model> tag" );
	return FALSE;
      }
      tagstate[TAG_MODEL] = FALSE;

      if ( curr_model->name == NULL ) {
	tsmError( funcName, "Model contains no name field!" );
	return FALSE;
      }
      if ( curr_model->location == NULL ) {
	tsmError( funcName, "Model contains no location data!" );
	return FALSE;
      }
    } else if ( strcmp( token, "</map>" ) == 0 ) {
      if ( tagstate[TAG_MAP] == FALSE ) {
	tsmError( funcName, "</map> appears with no <map> tag" );
	return FALSE;
      }
      tagstate[TAG_MAP] = FALSE;

      if ( curr_map->name == NULL ) {
	tsmError( funcName, "Map contains no name field!" );
	return FALSE;
      }
      if ( curr_map->location == NULL ) {
	tsmError( funcName, "Map contains no location data!" );
	return FALSE;
      }
    }
    break;

    /* The <MODEL> tag */

  case HASH_KEY('<','m','a'):
    if ( strcmp( token, "<map>" ) == 0 ) {
      if ( tagstate[TAG_MAP] == TRUE ) {
	tsmError( funcName, "illegal nesting of <map> tag" );
	return FALSE;
      }

      tsmNewDatabaseMap( dbase );
      tagstate[TAG_MAP] = TRUE;
    }
    break;

    /* The "name" keyword (used in <dataset> and <geoset>) */

  case HASH_KEY('n','a','m'):
    if ( strcmp( token, "name" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE )
	curr_set->name = tspExtractAllStrdup( data );
      else if ( tagstate[TAG_GEOSET] == TRUE ) 
	curr_geoset->name = tspExtractAllStrdup( data );
      else if ( tagstate[TAG_MODEL] == TRUE )
	curr_model->name = tspExtractAllStrdup( data );
      else if ( tagstate[TAG_MAP] == TRUE )
	curr_map->name = tspExtractAllStrdup( data );
      else {
	tsmError( funcName, "keyword '%s' outside valid tag group", token );
	return FALSE;
      }
    }
    break;

    /* The "imagery" keyword */

  case HASH_KEY('i','m','a'):
    if ( strcmp( token, "imagery" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <geoset> group", token );
	return FALSE;
      }

      curr_geoset->oi_name = tspExtractNthStrdup( data, temp_buffer, 0 );
    }
    break;

    /* The "dem" keyword */

  case HASH_KEY('d','e','m'):
    if ( strcmp( token, "dem" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <geoset> group", token );
	return FALSE;
      }

      curr_geoset->dem_name = tspExtractNthStrdup( data, temp_buffer, 0 );
    }
    break;

    /* The "map" keyword */

  case HASH_KEY('m','a','p'):
    if ( strcmp( token, "map" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <geoset> group", token );
	return FALSE;
      }

      tsmAddGeoSetMap( curr_geoset, data );
    }
    break;

    /* The "model" keyword */

  case HASH_KEY('m','o','d'):
    if ( strcmp( token, "model" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <geoset> group", token );
	return FALSE;
      }

      tsmAddGeoSetModel( curr_geoset, data );
    }
    break;

    /* The "planet" keyword */

  case HASH_KEY('p','l','a'):
    if ( strcmp( token, "planet" ) == 0 ) {
      if ( tagstate[TAG_GEOSET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <geoset> group", token );
	return FALSE;
      }

      tspExtractNthString( data, temp_buffer, 0 );
      curr_geoset->planet = tsmStringToPlanet( temp_buffer );
    }
    break;

    /* The "type" keyword */

  case HASH_KEY('t','y','p'):
    if ( strcmp( token, "type" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <dataset> group", token );
	return FALSE;
      } 

      if ( strcmp( data, "image-pyramid" ) == 0 )
	curr_set->type = TSM_IMAGE_DATASET;
      else if ( strcmp( data, "dem-pyramid" ) == 0 )
	curr_set->type = TSM_DEM_DATASET;
      else if ( strcmp( data, "vrml-pyramid" ) == 0 )
	curr_set->type = TSM_VRML_DATASET;
      else {
	tsmError( funcName, "unknown value for type field (%s)", data );
	return FALSE;
      }
    }
    break;

    /* The "description" keyword */

  case HASH_KEY('d','e','s'):
    if ( strcmp( token, "description" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == TRUE )
	curr_set->description = tspExtractAllStrdup( data );
      else if ( tagstate[TAG_MODEL] == TRUE )
	curr_model->description = tspExtractAllStrdup( data );
      else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }
    }
    break;

    /* The "tilesize" keyword */

  case HASH_KEY('t','i','l'):
    if ( strcmp( token, "tilesize" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == TRUE ) {
	char *string = tspExtractAllStrdup( data );
	curr_set->tilesize = atoi( string );
	free( string );
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }
    }
    break;

    /* The "overlap" keyword */

  case HASH_KEY('o','v','e'):
    if ( strcmp( token, "overlap" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == TRUE ) {
	char *string = tspExtractAllStrdup( data );
	curr_set->overlap = atoi( string );
	free( string );
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }
    }
    break;

    /* The "title" keyword */

  case HASH_KEY('t','i','t'):
    if ( strcmp( token, "title" ) == 0 ) {
      if ( tagstate[TAG_MODEL] == TRUE ) {
	if ( curr_model->title ) tsmMemFree( curr_model->title );
	curr_model->title = tspExtractAllStrdup( data );
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }
    }
    break;

    /* The "source-type" keyword */

  case HASH_KEY('s','o','u'):
    if ( strcmp( token, "source-type" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <dataset> group", token );
	return FALSE;
      }

      curr_set->source_type = tspExtractNthStrdup( data, temp_buffer, 0 );
    }
    break;

    /* The "uv-corners" keyword */

  case HASH_KEY('u','v','-'):
    if ( strcmp( token, "uv-corners" ) == 0 ) {
      if ( tagstate[TAG_MAP] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <map> group", token );
	return FALSE;
      }

      curr_map->min_u = tspExtractNthFloat( data, temp_buffer, 0 );
      curr_map->min_v = tspExtractNthFloat( data, temp_buffer, 1 );
      curr_map->max_u = tspExtractNthFloat( data, temp_buffer, 2 );
      curr_map->max_v = tspExtractNthFloat( data, temp_buffer, 3 );
    }
    break;

    /* The "min resolution" keyword */

  case HASH_KEY('m','i','n'):
    if ( strcmp( token, "min-resolution" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <dataset> group", token );
	return FALSE;
      }

      tspExtractNthString( data, temp_buffer, 1 );
      if ( strcmp( temp_buffer, "m" ) != 0 &&
	   strcmp( temp_buffer, "deg" ) != 0 ) {
	tsmError( funcName, "Invalid min-resolution units: %s", temp_buffer );
	return FALSE;
      }
      curr_set->min_res = (float) tspExtractNthFloat( data, temp_buffer, 0 );
    }
    break;

    /* The "max resolution" keyword */

  case HASH_KEY('m','a','x'):
    if ( strcmp( token, "max-resolution" ) == 0 ) {
      if ( tagstate[TAG_DATASET] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <dataset> group", token );
	return FALSE;
      }

      tspExtractNthString( data, temp_buffer, 1 );
      if ( strcmp( temp_buffer, "m" ) != 0 &&
	   strcmp( temp_buffer, "deg" ) != 0 ) {
	tsmError( funcName, "Invalid max-resolution units: %s", temp_buffer );
	return FALSE;
      }
      curr_set->max_res = (float) tspExtractNthFloat( data, temp_buffer, 0 );
    }
    break;

    /* The "latlong-footprint" keyword */

  case HASH_KEY('l','a','t'):
    if ( strcmp( token, "latlong-footprint" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE ) {
	curr_set->footprint.coordsys = TSM_LATLONG_COORDSYS;
	ll_foot = &curr_set->footprint.latlong;
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }

      if ( tspExtractNthString( data, temp_buffer, 7 ) == NULL ) {
	tsmError( funcName, "Insufficient values for latlong-footprint" );
	return FALSE;
      }

      /* Read in the 4 lat/long coordinates for the footprint */

      ll_foot->c1.lat = tspExtractNthFloat( data, temp_buffer, 0 );
      ll_foot->c1.lon = tspExtractNthFloat( data, temp_buffer, 1 );
      ll_foot->c2.lat = tspExtractNthFloat( data, temp_buffer, 2 );
      ll_foot->c2.lon = tspExtractNthFloat( data, temp_buffer, 3 );
      ll_foot->c3.lat = tspExtractNthFloat( data, temp_buffer, 4 );
      ll_foot->c3.lon = tspExtractNthFloat( data, temp_buffer, 5 );
      ll_foot->c4.lat = tspExtractNthFloat( data, temp_buffer, 6 );
      ll_foot->c4.lon = tspExtractNthFloat( data, temp_buffer, 7 );

    }
    break;

    /* The "utm-footprint" keyword */

  case HASH_KEY('u','t','m'):
    if ( strcmp( token, "utm-footprint" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE ) {
	curr_set->footprint.coordsys = TSM_UTM_COORDSYS;
	utm_foot = &curr_set->footprint.utm;
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }

      if ( tspExtractNthString( data, temp_buffer, 11 ) == NULL ) {
	tsmError( funcName, "Insufficient values for utm-footprint" );
	return FALSE;
      }

      utm_foot->c1.e    = tspExtractNthFloat( data, temp_buffer, 0 );
      utm_foot->c1.n    = tspExtractNthFloat( data, temp_buffer, 1 );
      utm_foot->c1.zone = tspExtractNthInt( data, temp_buffer, 2 );
      utm_foot->c2.e    = tspExtractNthFloat( data, temp_buffer, 3 );
      utm_foot->c2.n    = tspExtractNthFloat( data, temp_buffer, 4 );
      utm_foot->c2.zone = tspExtractNthInt( data, temp_buffer, 5 );
      utm_foot->c3.e    = tspExtractNthFloat( data, temp_buffer, 6 );
      utm_foot->c3.n    = tspExtractNthFloat( data, temp_buffer, 7 );
      utm_foot->c3.zone = tspExtractNthInt( data, temp_buffer, 8 );
      utm_foot->c4.e    = tspExtractNthFloat( data, temp_buffer, 9 );
      utm_foot->c4.n    = tspExtractNthFloat( data, temp_buffer, 10 );
      utm_foot->c4.zone = tspExtractNthInt( data, temp_buffer, 11 );

    }
    break;

    /* The "lvcs-footprint" keyword */

  case HASH_KEY('l','v','c'):
    if ( strcmp( token, "lvcs-footprint" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE ) {
	curr_set->footprint.coordsys = TSM_LVCS_COORDSYS;
	lvcs_foot = &curr_set->footprint.lvcs;
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }

      if ( tspExtractNthString( data, temp_buffer, 5 ) == NULL ) {
	tsmError( funcName, "Insufficient values for lvcs-footprint" );
	return FALSE;
      }

      lvcs_foot->lat     = tspExtractNthFloat( data, temp_buffer, 0 );
      lvcs_foot->lon     = tspExtractNthFloat( data, temp_buffer, 1 );
      lvcs_foot->xoffset = tspExtractNthFloat( data, temp_buffer, 2 );
      lvcs_foot->yoffset = tspExtractNthFloat( data, temp_buffer, 3 );
      lvcs_foot->width   = tspExtractNthFloat( data, temp_buffer, 4 );
      lvcs_foot->height  = tspExtractNthFloat( data, temp_buffer, 5 );

    }
    break;

    /* The "geo-footprint" keyword */

  case HASH_KEY('g','e','o'):
    if ( strcmp( token, "geo-footprint" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE ) {
	curr_set->footprint.coordsys = TSM_GEO_COORDSYS;
	geo_foot = &curr_set->footprint.geo;
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }

      if ( tspExtractNthString( data, temp_buffer, 11 ) == NULL ) {
	tsmError( funcName, "Insufficient values for geo-footprint" );
	return FALSE;
      }

      geo_foot->c1.x = tspExtractNthFloat( data, temp_buffer, 0 );
      geo_foot->c1.y = tspExtractNthFloat( data, temp_buffer, 1 );
      geo_foot->c1.z = tspExtractNthFloat( data, temp_buffer, 2 );
      geo_foot->c2.x = tspExtractNthFloat( data, temp_buffer, 3 );
      geo_foot->c2.y = tspExtractNthFloat( data, temp_buffer, 4 );
      geo_foot->c2.z = tspExtractNthFloat( data, temp_buffer, 5 );
      geo_foot->c3.x = tspExtractNthFloat( data, temp_buffer, 6 );
      geo_foot->c3.y = tspExtractNthFloat( data, temp_buffer, 7 );
      geo_foot->c3.z = tspExtractNthFloat( data, temp_buffer, 8 );
      geo_foot->c4.x = tspExtractNthFloat( data, temp_buffer, 9 );
      geo_foot->c4.y = tspExtractNthFloat( data, temp_buffer, 10 );
      geo_foot->c4.z = tspExtractNthFloat( data, temp_buffer, 11 );

    } else if ( strcmp( token, "geo-name" ) == 0 ) {

      if ( tagstate[TAG_DATASET] == TRUE ) {
	curr_set->geoname = tspExtractAllStrdup( data );
      } else if ( tagstate[TAG_MODEL] == TRUE ) {
	curr_model->geoname = tspExtractAllStrdup( data );
      } else if ( tagstate[TAG_MAP] == TRUE ) {
	curr_map->geoname = tspExtractAllStrdup( data );
      } else {
	tsmError( funcName, "keyword '%s' outside valid group", token );
	return FALSE;
      }

    }

    break;

    /* The "location" keyword - can appear in <instance> or <server> */

  case HASH_KEY('l','o','c'):
    if ( strcmp( token, "location" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == TRUE ) {
	if ( ( curr_inst->location = tsmParseUrl( data ) ) == NULL ) {
	  tsmError( funcName, "Couldn't parse the instance URL: %s", data );
	  return FALSE;
	}
      } else if ( tagstate[TAG_SERVER] == TRUE ) {
	if ( ( curr_server->location = tsmParseUrl( data ) ) == NULL ) {
	  tsmError( funcName, "Couldn't parse the server URL: %s", data );
	  return FALSE;
	}
      } else if ( tagstate[TAG_MAP] == TRUE ) {
	if ( ! tsmNewMapLocation( curr_map, data ) ) {
	  tsmError( funcName, "Couldn't parse the map URL: %s", data );
	  return FALSE;
	}
      } else if ( tagstate[TAG_MODEL] == TRUE ) {
	if ( ! tsmNewModelLocation( curr_model, data ) ) {
	  tsmError( funcName, "Couldn't parse the model URL: %s", data );
	  return FALSE;
	}
      } else {
	tsmError( funcName, "'%s' not inside <instance> or <server>", token );
	return FALSE;
      }
    }
    break;

    /* The "last-contacted" keyword */

  case HASH_KEY('l','a','s'): {
    PRInt32 hi, lo;
    PRInt64 t;
    PRTime time;

    if ( strcmp( token, "last-contacted" ) == 0 ) {
      if ( tagstate[TAG_SERVER] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <server> group", token );
	return FALSE;
      }
      
      if (sscanf( data, "%8x%8x", &hi, &lo ) == 1)
	lo = 0;
      if ( hi == 0 && lo == 0 ) {
	tsmError( funcName, "Zero time field found for last-contacted" );
	return FALSE;
      }
      LL_UI2L (time, lo);
      LL_UI2L (t, hi); LL_SHL (t, t, 32);
      LL_OR (time, time, t);

      curr_server->last_contacted = time;
    }
    break;
  }

    /* The "formats" keyword */

  case HASH_KEY('f','o','r'):
    if ( strcmp( token, "formats" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <instance> group", token );
	return FALSE;
      }
      
      curr_inst->formats = TSM_NONE_FORMAT;
      for ( i = 0; tspExtractNthString( data, temp_buffer, i ) != NULL; ++i ) {
	if ( strcmp( temp_buffer, "jpeg" ) == 0 )
	  curr_inst->formats |= TSM_JPEG_FORMAT;
	else if ( strcmp( temp_buffer, "lrbt" ) == 0 )
	  curr_inst->formats |= TSM_LRBT_FORMAT;
	else if ( strcmp( temp_buffer, "lzw" ) == 0 )
	  curr_inst->formats |= TSM_LZW_FORMAT;
	else if ( strcmp( temp_buffer, "mip-map" ) == 0 )
	  curr_inst->formats |= TSM_MIPMAP_FORMAT;
	else {
	  tsmError( funcName, "unknown format value: %s", data );
	  return FALSE;
	}
      }
      
    }
    break;

    /* The "set-id" keyword */

  case HASH_KEY('s','e','t'):
    if ( strcmp( token, "set-id" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <instance> group", token );
	return FALSE;
      }
      
      curr_inst->set_id = tspExtractNthInt( data, temp_buffer, 0 );
    }
    break;

    /* The "set-id" keyword */

  case HASH_KEY('s','e','r'):
    if ( strcmp( token, "servers" ) == 0 ) {
      if ( tagstate[TAG_INSTANCE] == FALSE ) {
	tsmError( funcName, "keyword '%s' outside <instance> group", token );
	return FALSE;
      }
      
      /* count how many strings we have in the data field first of all */

      len = 0;
      while ( tspExtractNthString( data, temp_buffer, len ) != NULL )
	len++;

      if ( len == 0 ) {
	tsmError( funcName, "no values specified for servers field" );
	return FALSE;
      }

      /* now allocate an array for that many char *'s */

      curr_inst->serverName =
	(char **) tsmMemManagerMalloc( sizeof(char *) * len );

      if ( curr_inst->serverName == NULL ) {
	tsmError( funcName, "couldn't allocate the servers array" );
	return FALSE;
      }

      /* now fill the array with copies of the strings in the field */
      
      for ( i = 0; i < len; ++i ) 
	curr_inst->serverName[i] = tspExtractNthStrdup( data, temp_buffer, i );
      
      curr_inst->numServers = len;
    }
    break;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmReadDatabase_local( TsmDatabase *database, char *filename )
 *
 * Description:
 *   Parses a dataset database.
 *
 *   Takes the name of a file on local disk and attempts to parse this
 *   into a TsmDatabase structure. Lots of checks are done on the validity
 *   of the database file - see tsmParseDatabaseLine() above for the
 *   main list of errors that are checked for. In addition to these, this
 *   function will perform checks to ensure that all SGML tags were closed
 *   in the database.
 *
 *   The TsmDatabase structure that you pass in must have already been
 *   allocated, e.g. using tsmMakeDatabase().
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 8 December 1997
 *
 */

int
tsmReadDatabase_local( TsmDatabase *database, char *filename )
{
  const char *funcName = "tsmReadDatabase_local";
  FILE       *fp;

  /* Check that the caller has not passed a NULL database pointer */

  if ( database == NULL ) {
    tsmError( funcName, "you must pass an allocated database structure" );
    return FALSE;
  }

  /* Try to open the dataset database file for reading. */

  if ( ( fp = fopen( filename, "r" ) ) == NULL ) {
    tsmError( funcName, "Could not open database (%s)", filename );
    return FALSE;
  }

  return tsmReadOldDatabase_local( database, fp );
}

/*
 * Synopsis:
 *   int tsmReadOldDatabase_local( TsmDatabase *database, FILE *stream )
 *
 * Description:
 *   Parses an old (non-XML) dataset database.
 *
 *   Takes a file stream and attempts to parse this into a TsmDatabase
 *   structure. Lots of checks are done on the validity of the
 *   database file - see tsmParseDatabaseLine() above for the main
 *   list of errors that are checked for. In addition to these, this
 *   function will perform checks to ensure that all SGML tags were
 *   closed in the database.
 *
 *   The TsmDatabase structure that you pass in must have already been
 *   allocated, e.g. using tsmMakeDatabase().
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 8 December 1997
 *
 */

int
tsmReadOldDatabase_local( TsmDatabase *database, FILE *stream )
{
  const char *funcName = "tsmReadOldDatabase_local";
  char       buffer[MAX_TSPEC_LINE_LENGTH];
  int        tagstate[TAG_ARRAY_SIZE] = { FALSE };

  /* Check that the caller has not passed a NULL database pointer */

  if ( database == NULL ) {
    tsmError( funcName, "you must pass an allocated database structure" );
    return FALSE;
  }

  /* Now, read each valid line from the file and try to parse it */

  while ( tspReadLineFromTspec( stream, buffer ) == TRUE ) {
    if ( tsmParseDatabaseLine( buffer, database, tagstate ) == FALSE ) {
      tsmError( funcName, "database parse stopped on line: %s", buffer );
      fclose( stream );
      return FALSE;
    }
  }

  fclose( stream );

  /* Check that all tags were closed in the file */

  if ( tagstate[TAG_INSTANCE] == TRUE ) {
    tsmError( funcName, "<instance> tag was not closed in database" );
    return FALSE;
  } else if ( tagstate[TAG_SERVER] == TRUE ) {
    tsmError( funcName, "<server> tag was not closed in database" );
    return FALSE;
  } else if ( tagstate[TAG_DATASET] == TRUE ) {
    tsmError( funcName, "<dataset> tag was not closed in database" );
    return FALSE;
  }

  return TRUE;
}

/*
 * Synopsis:
 *   int tsmWriteDatabase_local( TsmDatabase *database, char *filename )
 *
 * Description:
 *   Save a database file to disk.
 *
 *   This function will take an already allocated TsmDatabase structure
 *   and writes it out to the specified filename on local disk.
 *
 * Externals: None
 *
 * Returns: TRUE if success, FALSE if fail
 *
 * Author: Martin Reddy
 *
 * Date: 11 December 1997
 *
 */

int
tsmWriteDatabase_local( TsmDatabase *database, char *filename )
{
  const char *funcName = "tsmWriteDatabase_local";
  FILE       *fp;

  /* Check that the caller has not passed a NULL database pointer */

  if ( database == NULL ) {
    tsmError( funcName, "you must pass an allocated database structure" );
    return FALSE;
  }

  /* Save out the current database to an XML file on disk */

  /* return tsmPersistDatabase( database, filename ); */

  /* Try to open the dataset database file for writing. */

  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    tsmError( funcName, "Could not write to database (%s)", filename );
    return FALSE;
  }

  /* Now use the print database routine with the file handle we just got */

  tsmPrintDatabase( database, fp );

  fclose( fp );
  return TRUE;

}

/*
 * Synopsis:
 *   int NameUrlIds *tsmReadServer_local( Purl const *url )
 *
 * Description:
 *   Read all TerraVision datasets in a localdisk directory.
 *
 *   This function accepts the name of a localdisk directory (can be a
 *   file: URL). It will then scan through that directory looking for
 *   all directories that have an extension of either ".oi", ".dem",
 *   ".model", or ".map". It will then return a NameUrlIds array with
 *   all such entries. N.B. this array could be zero length if no such
 *   entries exist in the directory.
 *
 * Returns: A list of available datasets, or NULL if couldn't read directory.
 *
 * Author: Martin Reddy
 *
 * Date: 3 November 1999
 *
 */

#ifdef MSVC

/* XXX reddy - opendir/readir is POSIX but not supported under MSVC!! */
/*             therefore, need to recode this bit with MSVC functions */

#include <windows.h>

NameUrlIds *
tsmReadServer_local( Purl const *purl )
{
  const char      *funcName = "tsmReadServer_local";
  HANDLE          handle;
  WIN32_FIND_DATA find_data;
  char            name[512], dirname[512], searchname[512];
  NameUrlIds      *nameUrlIds;
  int             i, count = 0;
  
  if ( purl == NULL )
    return NULL;

  /* take a copy of the pathname part of the URL */
  
  strcpy( dirname, purl->uri );
  strcpy( searchname, purl->uri );

  /* append a trailing slash if one is not given for the root name */

  if ( dirname[strlen(dirname)-1] != '/' )
    strcat( dirname, "/" );

  /* convert all '/' into '\' for PC compatibility in the search string */

  for ( i = strlen( searchname )-1; i >= 0; i-- )
    if ( searchname[i] == '/' )
      searchname[i] = '\\';
  
  /* append a trailing slash if one is not given, and *.* for FindFirstFile */

  if ( searchname[strlen(searchname)-1] != '\\' )
    strcat( searchname, "\\" );  
  strcat( searchname, "*.*" );

  /* allocate an empty database which we will read values into */
  
  if ( ( nameUrlIds = tsmMakeNameUrlIds( 0 ) ) == NULL ) {
    tsmError( funcName, "Unable to allocate server database array" );
    return NULL;
  }

  /* try to open the directory so we can scan through it */

  handle = FindFirstFile( searchname, &find_data );
  if ( handle == INVALID_HANDLE_VALUE ) {
    tsmError( funcName, "Cannot open directory: %s", dirname );
    return NULL;
  }

  /* now read each directory entry in turn */

  do {

    /* work out the full path name for this entry */

    strcpy( name, dirname );
    strcat( name, find_data.cFileName );

    /* does the entry have a valid filename extension? */

    if ( tsmCheckFilenameExtension( name, ".oi" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".dem" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".model" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".map" ) == FALSE )
      continue;
    
    /* we should have a valid directory now (or a symlink to one) */

    tsmAddNameUrlId( nameUrlIds, tsmMemStrdup( find_data.cFileName ),
		     tsmMemStrdup( name ), count++ );

    TSMTRACE(( TSMMASK|2, funcName, "Dataset: %s", name ));

  } while ( FindNextFile( handle, &find_data ) );

  FindClose( handle );

  return nameUrlIds;
}

#else

NameUrlIds *
tsmReadServer_local( Purl const *purl )
{
  const char    *funcName = "tsmReadServer_local";
  DIR           *handle;
  struct dirent *entry;
  char          name[512], dirname[512];
  NameUrlIds    *nameUrlIds;
  int           count = 0;
  
  if ( purl == NULL )
    return NULL;

  /* take a copy of the pathname part of the URL */
  
  strcpy( dirname, purl->uri );
  
  /* append a trailing slash if one is not given */

  if ( dirname[strlen(dirname)-1] != '/' )
    strcat( dirname, "/" );

  /* allocate an empty database which we will read values into */
  
  if ( ( nameUrlIds = tsmMakeNameUrlIds( 0 ) ) == NULL ) {
    tsmError( funcName, "Unable to allocate server database array" );
    return NULL;
  }

  /* try to open the directory so we can scan through it */

  if ( ( handle = opendir( dirname ) ) == NULL ) {
    tsmError( funcName, "Cannot open directory: %s", dirname );
    return NULL;
  }

  /* now read each directory entry in turn */

  while ( ( entry = readdir( handle ) ) != NULL ) {

    /* work out the full path name for this entry */

    strcpy( name, dirname );
    strcat( name, entry->d_name );

    /* does the entry have a valid filename extension? */

    if ( tsmCheckFilenameExtension( name, ".oi" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".dem" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".model" ) == FALSE &&
	 tsmCheckFilenameExtension( name, ".map" ) == FALSE )
      continue;
    
    /* we should have a valid directory now (or a symlink to one) */

    tsmAddNameUrlId( nameUrlIds, tsmMemStrdup( entry->d_name ),
		     tsmMemStrdup( name ), count++ );

    TSMTRACE(( TSMMASK|2, funcName, "Dataset: %s", name ));

  }

  closedir( handle );
  return nameUrlIds;
}
#endif

