/*
 * Name: tv2geo.c
 *
 * Description:
 *   This program will load a TerraVision dataset database file and
 *   produce a file that can be used as input to the .geo publishing
 *   utilities. That is, you can take a TerraVision list of terrain
 *   and GeoVRML datasets and enable these to be published in a .geo
 *   infrastructure.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.tsmApi.com/
 *
 *   This program was written for use with the tsmApi library from
 *   SRI International. For further information about this library,
 *   including downloads, documentation, and other examples, see:
 *
 *       http://www.tsmapi.com/
 *
 * Author:
 *   Martin Reddy, <reddy@ai.sri.com> - 27 November 2000.
 *
 * Function List:
 *   int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: tv2geo.c,v 1.2 2000/12/01 05:50:12 reddy Exp $
 * 
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tsm/tsm.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.2 $" )

void
output_footprint( Footprint *footprint, FILE *fp )
{
  double           n, s, e, w;
  Footprint        ll_footprint;
  LatLongFootprint *gdc;

  if ( ! tsmFootprintToLatLong( footprint, &ll_footprint ) )
    return;

  gdc = &ll_footprint.latlong;

  n = max( max ( max( gdc->c1.lat, gdc->c2.lat ), gdc->c3.lat ), gdc->c4.lat );
  s = min( min ( min( gdc->c1.lat, gdc->c2.lat ), gdc->c3.lat ), gdc->c4.lat );
  e = max( max ( max( gdc->c1.lon, gdc->c2.lon ), gdc->c3.lon ), gdc->c4.lon );
  w = min( min ( min( gdc->c1.lon, gdc->c2.lon ), gdc->c3.lon ), gdc->c4.lon );

  fprintf( fp, "  <gd:geoBndBox>\n" );
  fprintf( fp, "    <gd:north>%lg</gd:north>\n", n );
  fprintf( fp, "    <gd:south>%lg</gd:south>\n", s );
  fprintf( fp, "    <gd:east>%lg</gd:east>\n", e );
  fprintf( fp, "    <gd:west>%lg</gd:west>\n", w );
  fprintf( fp, "  </gd:geoBndBox>\n" );
}

void
convert_dataset( TsmDataSet *dataset, FILE *fp )
{
  int i;

  if ( dataset->numInstances == 0 ) return;

  fprintf( stderr, "Converting %s...                   \r", dataset->name );

  fprintf( fp, "<gd:geodata>\n" );

  fprintf( fp, "  <gd:name>%s</gd:name>\n",
	   ( dataset->geoname ) ? dataset->geoname : dataset->name );

  if ( dataset->description && dataset->description[0] != '\0' )
    fprintf( fp, "  <gd:description>%s</gd:description>\n",
	     dataset->description );

  fprintf( fp, "  <gd:type>%s</gd:type>\n",
	   ( dataset->type == TSM_DEM_DATASET ) ? "Dem" : "Oi" );

  for ( i = 0; i < dataset->numInstances; i++ ) {
    TsmDataSetInst *inst = &dataset->instance[i];

    if ( inst->location->protocol == tsmHTTP )
      fprintf( fp, "  <gd:url>%s</gd:url>\n", inst->location->url );
  }

  output_footprint( &dataset->footprint, fp );

  fprintf( fp, "</gd:geodata>\n\n" );
}

void
convert_model( TsmDatabase *database, TsmModel *model, FILE *fp )
{
  int        i;
  TsmGeoSet  *geoset;
  TsmDataSet *dataset;
  char       *name = model->title;

  /* don't output an XML entry for this model if it is not associated */
  /* with a dataset. Eventually, we should calculate the bounding box */
  /* of a VRML file and use that directly.                            */

  if ( ! ( geoset = tsmGetGeoSet( database, model->geoname ) ) )
    return;

  if ( ! ( dataset = tsmGetDataSet( database, geoset->oi_name ) ) )
    return;

  /* find a good name for this model */

  if ( ! name ) name = model->geoname;
  if ( ! name ) name = model->name;

  fprintf( stderr, "Converting %s...                   \r", model->name );

  /* output the XML format for the model information */

  fprintf( fp, "<gd:geodata>\n" );
  fprintf( fp, "  <gd:name>%s</gd:name>\n", name );

  if ( model->description && model->description[0] != '\0' )
    fprintf( fp, "  <gd:description>%s</gd:description>\n",
	     model->description );

  fprintf( fp, "  <gd:type>Vrml</gd:type>\n" );

  for ( i = 0; i < model->numLocations; i++ )
    if ( model->location[i]->protocol == tsmHTTP )
      fprintf( fp, "  <gd:url>%s</gd:url>\n", model->location[i]->url );

  output_footprint( &dataset->footprint, fp );

  fprintf( fp, "</gd:geodata>\n\n" );
}

int
main( int argc, char **argv )
{
  TsmDatabase *database;
  FILE        *fp = stdout;
  int         i, http_yes = FALSE;

  /* do we have enough arguments? */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 || argc > 3 ) {
    printf( "tv2geo - release %s\n", APP_VERSION );
    puts( "\nusage: tv2geo <dbase_url> [<dotgeo_list>]" );
    puts( "\nA utility to convert a TerraVision dataset database into a" );
    puts( "form that can be easily added to a .geo infrastructure.\n" );
    puts( "  <dbase_url> = the filename of a TerraVision database" );
    puts( "  <dotgeo_list> = the output xml filename (or stdout if omitted" );
    puts( "" );
    exit( 0 );
  }

  /* Display the SRI copyright message */

  fprintf( stderr, "TV2Geo %s, Copyright (c) 2000 SRI International. "
	   "All rights reserved.\n", APP_VERSION );

  /* check command line for any supported options */

  if ( argc == 3 && ( fp = fopen( argv[2], "w" ) ) == NULL ) {
    fprintf( stderr, "Could not write to output file: %s\n", argv[2] );
    exit( 1 );
  }

  /* Read the current database */

  if ( ( database = tsmReadDatabase( argv[1] ) ) == NULL ) {
    fprintf( stderr, "Error reading dataset database: %s\n", argv[1] );
    exit( 1 );
  }

  /* output the XML header - just use fprintf's for the time being */

  fprintf(fp,"<?xml version=\"1.0\"?>\n");
  fprintf(fp,"<geo:geoschemans xmlns:geo=\"http://www.dgeo.org/hschema/\">\n");
  fprintf(fp,"<gd:geodatans xmlns:gd=\"http://www.dgeo.org/geodata/\">\n\n");

  /* print some warnings if appropriate */

  if ( database->numDataSets == 0 )
    fprintf( stderr, "Database contains no datasets!\n" );

  for ( i = 0; i < database->numServers; ++i )
    if ( database->server[i].location->protocol == tsmHTTP )
      http_yes = TRUE;

  if ( ! http_yes )
    fprintf( stderr, "Database contains no http datasets - ignoring urls\n" );

  /* convert to the .geo input format */

  for ( i = 0; i < database->numDataSets; ++i )
    convert_dataset( &database->dataset[i], fp );

  for ( i = 0; i < database->numModels; ++i )
    convert_model( database, &database->model[i], fp );

  /* clean up */

  fprintf( stderr, "Done.                              \n" );

  fprintf( fp, "</gd:geodatans>\n" );
  fprintf( fp, "</geo:geoschemans>\n" );

  if ( fp != stdout )
    fclose( fp );

  tsmFreeDatabase( database );

  return 0;
}

/*** EOF: tv2geo.c ***/
