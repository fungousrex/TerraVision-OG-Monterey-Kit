/*
 * Filename: make_dem.c
 *
 * Description:
 *     This program will generate an entire DEM pyramid (using a UTM
 *     projection system). That is, this program will:
 *
 *        1. create the tiles.tspec file for a pyramid
 *        2. create the image tiles for the top level image provided
 *        3. recursively downsample this image and create all levels' tiles
 *        4. update the PyramidUrls database if necessary
 *        5. write to localdisk or DPSS.
 *        6. optionally produce only the tiles or the tspec
 *
 *     See the function usage() for the command line syntax.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 20 March 1998.
 *
 * Revision Information:
 *
 *     $Id: make_dem.c,v 1.14.2.4 2002/03/14 05:16:06 reddy Exp $
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tsm/tsm.h>

#define HELP "http://www.tsmapi.com/examples/Docs/make_dem.html"
#define APP_VERSION tsmCvsToString( "$Revision: 1.14.2.4 $" )

/*
 * Synopsis:
 *    void usage( void );
 *
 * Description:
 *    Display the program usage information to the terminal.
 *    This is called if insufficient parameters are given.
 *
 */

void
usage( void )
{
  printf( "make_dem release %s\n", APP_VERSION );
  puts( "\nUsage: make_dem <dataset> <img-name> [...]" );

  puts( "\nCreates a DEM pyramid (lrbt only) for use with TerraVision II." );
  puts( "Supported input formats = .dem, dt0, .raw .lrbt (see -help)" );

  puts( "\nRequired Parameters:" );
  puts( " <dataset> = filename/URL for new pyramid (no extensions)" );
  puts( " <img-name> = name of elevation file for 1st level of pyramid" );
  puts( " -width <n> = pixel width of image (samples)" );
  puts( " -height <n> = pixel height of image (lines)" );

  puts( "\nOptional Parameters:" );
  puts( " -bytespercomp <n> = bytes per source elevation value [default = 4]");
  puts( " -outbytespercomp <n> = bytes per output elevation value [default = 4]");
  puts( " -startlev <n> = start level for pyramid [default = 0]" );
  puts( " -startres <n> = resolution for start level [default = 32.0]" );
  puts( " -tilesize <n> = tile width/height [default = 128]" );
  puts( " -tileoverlap <n> = tile overlap pixels [default = 1]" );
  puts( " -geoname <s> = the geoset name shared between the DEM and OI" );
  puts( " -planet <s> = the name of the planet [default = \"earth\"]" );
  puts( " -type <s> = image source info, e.g. \"spot\" [default = \"\"]" );
  puts( " -descr <s> = a textual description of the pyramid image" );
  puts( " -sources <s> = string of sources for the image, e.g \"DOQ\"" );
  puts( " -ops <s> = string of operations performed on image" );
  puts( " -doneby <s> = string stating who performed the operation" );
  puts( " -scale <n> = scaling value to apply to DEM data [default = 0.001]");
  puts( " -offset <n> = offset to apply after scaling [default = 0.0]" );
  puts( " -sealevel <n> = height value representing sea areas [default = 0]" );

  puts( "\nGeo-reference Information:" );
  puts( " -ll_e <n> = UTM lower-left Easting (m) for data [default = 0.0]" );
  puts( " -ll_n <n> = UTM lower-left Northing (m) for data [default = 0.0]" );
  puts( " -zone <n> = UTM zone no. (1..60) at center of data [default = 0]" );
  puts( " -southern_hemi = UTM dataset is in southern hemisphere" );
  puts( " -ll_lat <n> = Lat/Long lower-left latitude (deg) [default = 0.0]" );
  puts( " -ll_long <n> = Lat/Long lower-left longitude (deg) [default = 0.0]");
  puts( " -lvcs_x = LVCS x offset (meters) from -ll_lat/long origin [default=0.0]" );
  puts( " -lvcs_y = LVCS y offset (meters) from -ll_lat/long origin [default=0.0]" );

  puts( "\nDataset Generation Options:" );
  tsmDescribeArgs( TSM_ARG_WRITE | TSM_ARG_CONNECT );

  puts( "\nOptional Output Control:" );
  puts( " -tilesonly = create tiles only, tspec already done" );
  puts( " -tspeconly = create the tspec only, no tiles" );
  puts( " -help = display the help web page for make_dem using netscape" );
  puts( " -info = display details about the new dataset once created" );
  tsmDescribeArgs( TSM_ARG_INIT );
  exit( 1 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main part of the program: checks the command line arguments,
 *      creates the tspec, adds the pyramid to the database, generates
 *      the tiles for each level of the pyramid.
 *
 * Author: Martin Reddy
 * 
 * Date: 3 September 1997
 *
 */

int
main( int argc, char **argv )
{
  TsmConnection       connect;
  TsmConnectionParams connect_params;
  TsmTileParams       tile_params;
  TsmTspecParams      tspec_params;
  char                path_url[512], image_name[512], buffer[512];
  char                *planet = NULL;
  int                 do_tiles, do_tspec, do_info;

  /* display the program usage if we don't have enough args */

  tsmParseInitArgs( argc, argv );

  if ( tsmGetArg( argc, argv, "-help" ) == TRUE ) {
    puts( "Attempting to display help page using netscape..." );
    tsmShowUrl( HELP );
    exit( 0 );
  }

  if ( argc < 3 ) usage();

  /* Display the SRI copyright message */

  printf( "MakeDEM %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Construct a full URL for the pyramid to be added to. We  will */
  /* implicitly add a ".dem" extension if not already present.     */

  strcpy( path_url, argv[1] );
  tsmAddFilenameExtension( path_url, ".dem" );

  strcpy( image_name, argv[2] );

  /* Set up a number of default values for all things optional */
  
  tsmInitTileParams( &tile_params, TSM_DEM_DATASET );
  tsmInitTspecParams( &tspec_params, TSM_DEM_DATASET );
  if ( ! tsmParseCreateParams( image_name, &tile_params, &tspec_params ) ) {
    fprintf( stderr, "Unsupported or incomplete file format\n" );
    exit( 1 );
  }

  do_tiles = TRUE;
  do_tspec = TRUE;
  do_info  = FALSE;
  
  /* Check for all of the required arguments */

  if ( tsmGetArgInt( argc, argv, "-width", &tile_params.width ) == TRUE )
    tspec_params.width  = tile_params.width;

  if ( tsmGetArgInt( argc, argv, "-height", &tile_params.height ) == TRUE )
    tspec_params.height = tile_params.height;

  /* Now we can check for any optional arguments */

  tsmGetArgInt( argc, argv, "-bytespercomp", &tile_params.bytespercomp );
  tsmGetArgInt( argc, argv, "-outbytespercomp", &tile_params.out_bytespercomp);

  tsmGetArgInt( argc, argv, "-startlev", &tspec_params.start_lev );
  tsmGetArgFloat( argc, argv, "-startres", &tspec_params.start_res );

  if ( tsmGetArgFloat( argc, argv, "-scale", &tspec_params.scale ) )
    tile_params.scale = tspec_params.scale;
  if ( tsmGetArgFloat( argc, argv, "-offset", &tspec_params.offset ) )
    tile_params.offset = tspec_params.offset;

  if ( tsmGetArgInt( argc, argv, "-tilesize", &tile_params.tile_width ) )
    tile_params.tile_height = tile_params.tile_width;
  if ( tsmGetArgInt( argc, argv, "-tileoverlap", &tile_params.tile_overlap_x ))
    tile_params.tile_overlap_y = tile_params.tile_overlap_x;

  tsmGetArgString( argc, argv, "-geoname", &tspec_params.geoName );
  tsmGetArgString( argc, argv, "-type", &tspec_params.source_type );
  tsmGetArgString( argc, argv, "-descr", &tspec_params.description );
  tsmGetArgString( argc, argv, "-sources", &tspec_params.sources );
  tsmGetArgString( argc, argv, "-ops", &tspec_params.ops );
  tsmGetArgString( argc, argv, "-doneby", &tspec_params.doneby );

  tsmGetArgString( argc, argv, "-planet", &planet );
  tspec_params.planet = tsmStringToPlanet( planet );

  tsmGetArgFloat( argc, argv, "-sealevel", &tile_params.sea_level );

  /* Geo-reference information */

  if ( tsmGetArgFloat( argc, argv, "-ll_e", &tspec_params.ll_e ) )
    tspec_params.coord_sys = TSM_UTM_COORDSYS;
  if ( tsmGetArgFloat( argc, argv, "-ll_n", &tspec_params.ll_n ) )
    tspec_params.coord_sys = TSM_UTM_COORDSYS;
  if ( tsmGetArgInt( argc, argv, "-zone", &tspec_params.zone ) )
    tspec_params.coord_sys = TSM_UTM_COORDSYS;

  /* we implement southern hemi UTM's with a negative northing value */

  if ( tsmGetArg( argc, argv, "-southern_hemi" ) )
    tspec_params.ll_n = -fabs(tspec_params.ll_n);

  if ( tsmGetArgFloat( argc, argv, "-ll_lat", &tspec_params.ll_lat ) )
    tspec_params.coord_sys = TSM_LATLONG_COORDSYS;
  if ( tsmGetArgFloat( argc, argv, "-ll_long", &tspec_params.ll_lon ) )
    tspec_params.coord_sys = TSM_LATLONG_COORDSYS;

  if ( tsmGetArgFloat( argc, argv, "-lvcs_x", &tspec_params.lvcs_x ) )
    tspec_params.coord_sys = TSM_LVCS_COORDSYS;
  if ( tsmGetArgFloat( argc, argv, "-lvcs_y", &tspec_params.lvcs_y ) )
    tspec_params.coord_sys = TSM_LVCS_COORDSYS;

  /* The optional output control parameters */

  connect_params = tsmParseWriteArgs( argc, argv );
  tsmSetParam( connect_params, TSM_PARAM_FALLBACK, FALSE );
  
  if ( tsmGetArg( argc, argv, "-tilesonly" ) == TRUE ) {
    do_tiles = TRUE;
    do_tspec = FALSE;
  }

  if ( tsmGetArg( argc, argv, "-tspeconly" ) == TRUE ) {
    do_tiles = FALSE;
    do_tspec = TRUE;
  }
   
  if ( tsmGetArg( argc, argv, "-info" ) == TRUE )
    do_info = TRUE;

  if ( tsmGetArg( argc, argv, "-pointsample" ) == TRUE )
    tile_params.filtering = TSM_POINT_SAMPLE; /* default anyway */

  tile_params.remove_temp_imgs = ! tsmGetArg( argc, argv, "-leaveimages" );

  /* Append an operations specifier for the type of filtering done */

  strcpy( buffer, tspec_params.ops );
  if ( tile_params.filtering == TSM_GAUSSIAN_FILTER ) {
    if ( buffer[0] != '\0' ) strcat( buffer, " " );
    strcat( buffer, "gauss-reduce" );
  } else if ( tile_params.filtering == TSM_POINT_SAMPLE ) {
    if ( buffer[0] != '\0' ) strcat( buffer, " " );
    strcat( buffer, "point-sample" );
  }
  tspec_params.ops = buffer;

  /* Check that we have enough information */

  if ( tile_params.width == 0 || tile_params.height == 0 ) {
    fprintf( stderr, "You must specify a -width and -height for the data!\n" );
    exit( 1 );
  }

  /* check that we have a valid (non-zero) scale factor */

  if ( tspec_params.scale == 0.0 ) {
    fprintf( stderr, "You must specify a non-zero -scale!\n" );
    exit( 1 );
  }

  /* check that we have some georeference information to play with */

  if ( tspec_params.coord_sys == TSM_NONE_COORDSYS ) {
    fprintf( stderr, "You must specify georeference details for the data!\n" );
    exit( 1 );
  }

  /***************** ARGUMENT PARSING DONE - NOW GENERATE ***************/

  /* Open  write connection for the new dataset */

  if ( ( connect = tsmConnect( path_url, "w", connect_params ) ) == NULL ) {
    fprintf( stderr, "couldn't create new dataset: %s\n", path_url );
    exit( 1 );
  }
  
  if ( tsmParsedAllOptions( FALSE ) == FALSE ) {
    tsmDisconnect( connect );
    exit( 1 );
  }

  tsmParseConnectArgs( argc, argv, connect );
  
  /* now create either/both of the tile data and tspec file */

  printf( "%s:\n", path_url );
  tsmDescribeImportFormat( tile_params );

  /* Create the tspec file. If the -tilesonly option was */
  /* specified then this is not saved to disk            */

  if ( ! tsmCreateTspec( connect, tspec_params, tile_params, do_tspec ) ) {
    fprintf( stderr, "Failed to create tspec for dataset\n" );
    exit( 1 );
  }

  printf( "Created tspec file for dataset\n" );

  /* now that we have made the tspec, we read it in so that we have */
  /* everything set that tsmGetPyramid() does.                      */

  if ( do_tiles == TRUE ) {
    
    if ( tsmCreateAllTiles( connect, image_name, tile_params ) == FALSE ) {
      fprintf( stderr, "Failed to create tile data for pyramid\n" );
      exit( 1 );
    } 

    printf( "Created tile data for dataset\n" );

    /* We write out the tspec again here (unless -tilesonly) because */
    /* the process of generating the tiles also calculates the       */
    /* min/max pixel values and the checksum array for the dataset.  */

    if ( do_tspec == TRUE ) {
      tsmWriteTspec( connect );

      printf( "Updated tspec with checksum and min/max pixel information\n" );
    }
    
  }

  /* if the -info option was given, tell us about the new dataset */

  if ( do_info == TRUE ) 
    tsmDescribeConnection( connect );

  /* All done, so close down... */

  tsmDisconnect( connect );
  
  return 0;
}

/* EOF: make_dem.c */
