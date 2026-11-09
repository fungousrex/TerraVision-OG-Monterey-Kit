/*
 * Name: calc_bbox.c
 *
 * Description:
 *     Reads the tspec for a TerraVision dataset and outputs information
 *     about the bounding box of that dataset in geographic coordinates.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 19 October 1999.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *     void print_gdc( double, double, char * )
 *
 * Revision Information:
 *
 *     $Id: calc_bbox.c,v 1.4 2000/11/27 23:57:30 reddy Exp $
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
#include <math.h>
#include <tsm/tsm.h>

#define APP_VERSION tsmCvsToString( "$Revision: 1.4 $" )

/*
 * Synopsis:
 *   void usage( void )
 *
 * Description:
 *   Displays the program usage information & description, then exits.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 October 1999
 *
 */

void usage( void )
{
  printf( "calc_bbox - release %s\n", APP_VERSION );
  printf( "\nusage: calc_bbox <pyramid-url>\n" );
  printf( "\nThis utility reads a TerraVision dataset and then outputs\n" );
  printf( "information about that dataset's bounding box.\n" );
  printf( "\nParameters:\n" );
  printf( " <pyramid-url>   = URL/filename of the pyramid to analyze\n" );
  printf( "\n" );
  exit( 0 );
}

/*
 * Synopsis:
 *    void print_gdc( double, double, char * )
 *
 * Description:
 *   Output a GDC (lat/long) coordinate with an option message appended
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 19 October 1999
 *
 */

void print_gdc( double lon, double lat, char *message )
{
  printf( "            (%1.4lf%c, %1.4lf%c) degrees %s\n",
	  fabs(lon), ( lon < 0 ) ? 'W' : 'E',
	  fabs(lat), ( lat < 0 ) ? 'S' : 'N',
	  message );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty. This checks the command line arguments,
 *      opens the desired pyramid, and then outputs the bounding box
 *      and other information for the dataset.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 *
 * Date: 19 October 1999
 *
 */

int main( int argc, char **argv )
{
  TsmConnection   connect;
  Pyramid         *tspec;
  ProjectionTspec *proj;
  SimpleTileSet   *tset;
  CoordTspec      *coord;
  float           *xform;
  double          x, y;
  int             *dimen;

  /* Check the command line arguments - display usage if invalid */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 ) usage();

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* Display the SRI copyright message */

  printf( "CalcBBox %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Open a connection to the specified dataset */

  if ( ( connect = tsmConnect( argv[1], "r", NULL ) ) == NULL ) {
    fprintf( stderr, "ERROR: couldn't connect to %s\n", argv[1] );
    exit( 1 );
  }

  /* get the tspec structure for this dataset */

  if ( ( tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC ) ) == NULL ) {
    fprintf( stderr, "ERROR: no valid tspec data for %s\n", argv[1] );
    exit( 1 );
  }

  /* tell us a bit about the dataset and the tile about to be read */

  tsmDescribeConnection( connect );

  /* describe the bounding box in the dataset's coordinate system */

  proj  = &tspec->projectionTspec;
  tset  = &tspec->tspecLevel.val[tspec->minLevel];
  xform = tset->imageTransformationTspec.ixiytoUV;
  dimen = tset->numValidPixel;
  coord = &tspec->coordTspec;

  printf( "  Bnd Box = " );

  switch ( proj->projectionType ) {
  case tspUtmProjection:
    printf( "UTM Zone %d\n", proj->utmZoneNumber );
    printf( "            (%1.1fe, %1.1fn) meters lower-left\n",
	    xform[2], xform[5] );
    printf( "            (%1.1fe, %1.1fn) meters upper-right\n",
	    xform[0] * dimen[0] + xform[2],
	    xform[4] * dimen[1] + xform[5] );
    break;
  case tspLatLongProjection:
    printf( "GDC (Lat/Long)\n" );
    print_gdc( xform[2], xform[5], "lower-left" );
    print_gdc( xform[0] * dimen[0] + xform[2],
	       xform[4] * dimen[1] + xform[5], "upper-right" );
    break;
  case tspAffineProjection:
    printf( "LVCS (Local Vertical Coord System)\n" );
    printf( "            (%1.1f, %1.1f) meters lower-left\n",
	    xform[2], xform[5] );
    printf( "            (%1.1f, %1.1f) meters upper-right\n",
	    xform[0] * dimen[0] + xform[2],
	    xform[4] * dimen[1] + xform[5] );
    break;
  }

  /* describe the GDC footprint of the database where appropriate */

  switch ( proj->projectionType ) {
  case tspUtmProjection:
    printf( "  GDC Box = GDC (lat/long) footprint\n" );

    tsmUTMToLatLong( proj->utmZoneNumber,
		     xform[2],
		     xform[5],
		     &y, &x );
    print_gdc( x, y, "lower-left" );

    tsmUTMToLatLong( proj->utmZoneNumber,
		     xform[2],
		     xform[4] * dimen[1] + xform[5],
		     &y, &x );
    print_gdc( x, y, "upper-left" );

    tsmUTMToLatLong( proj->utmZoneNumber,
		     xform[0] * dimen[0] + xform[2],
		     xform[5],
		     &y, &x );
    print_gdc( x, y, "lower-right" );

    tsmUTMToLatLong( proj->utmZoneNumber,
		     xform[0] * dimen[0] + xform[2],
		     xform[4] * dimen[1] + xform[5],
		     &y, &x );
    print_gdc( x, y, "upper-right" );

    break;
  case tspAffineProjection:
    printf( "  Origin  = GDC (lat/long)\n" );

    x = coord->originLong;
    y = coord->originLat;
    print_gdc( x, y, "lower-left" );

    break;
  }

  /* clean up */

  tsmDisconnect( connect );

  return 0;
}

/*** EOF: calc_bbox.c ***/
