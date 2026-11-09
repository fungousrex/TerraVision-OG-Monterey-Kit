/*
 * Name: utm_conv.c
 *
 * Description:
 *     A simple little program to illustrate the Lat/Long <-> UTM 
 *     conversion facilities of tsmApi.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 9 March 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: utm_conv.c,v 1.4.2.1 2001/09/11 23:25:10 reddy Exp $ 
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

#define APP_VERSION tsmCvsToString( "$Revision: 1.4.2.1 $" )

/*
 * Synopsis:
 *      void usage( void )
 *
 * Description:
 *      Displays the program usage information & description, then exits.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * Date: 20 November 1997
 *
 */

void usage( void )
{
  printf( "utm_conv - release %s\n", APP_VERSION );
  printf( "\nusage: utm_conv -latlong <lat> <long>\n" );
  printf( "   or: utm_conv -utm <easting> <northing> [<zone>]\n" );
  printf( "\nThis utility will either convert lat/long coordinates to the\n" );
  printf( "UTM coord system (-latlong), or UTM coords to lat/long (-utm).\n" );
  printf( "N.B. latlong coordinates in the range: -90..+90, -180..+180\n\n" );
  printf( " -latlong <lat> <long> = convert this geodetic coord\n" );
  printf( " -utm <e> <n> <zone> = convert this UTM coord\n" );
  printf( " -gcc <x> <y> <z> = convert this to geocentric coord\n\n" );
  tsmDescribeArgs( TSM_ARG_INIT );
  exit( 0 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 20 November 1997
 *
 */

int main( int argc, char **argv )
{
  double north, east, lat, lon, x, y, z, alt;
  int    zone;

  /* do we have enough arguments? */

  tsmParseInitArgs( argc, argv );

  if ( argc < 4 ) usage();

  /* Display the SRI copyright message */

  printf( "UtmConv %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* Now check which way we want to do the UTM/LatLong conversion */

  if ( strcmp( argv[1], "-latlong" ) == 0 ) {

    /* do the lat/long -> UTM & GCC conversion */

    lat = atof( argv[2] );
    lon = atof( argv[3] );

    if ( tsmLatLongToUTM( lat, lon, &zone, &east, &north ) == FALSE )
      return 1;

    if ( tsmLatLongToGeocentric( lat, lon, 0.0, &x, &y, &z ) == FALSE )
      return 1;

  } else if ( strcmp( argv[1], "-utm" ) == 0 ) {

    /* do the UTM -> lat/long & GCC conversion */

    east  = atof( argv[2] );
    north = atof( argv[3] );
    zone  = ( argc > 4 ) ? atoi( argv[4] ) : 0;

    if ( tsmUTMToLatLong( zone, east, north, &lat, &lon ) == FALSE )
      return 1;

    if ( tsmLatLongToGeocentric( lat, lon, 0.0, &x, &y, &z ) == FALSE )
      return 1;

  } else if ( strcmp( argv[1], "-gcc" ) == 0 ) {

    /* do the GCC -> lat/long and UTM conversion */

    x = atof( argv[2] );
    y = atof( argv[3] );
    z = atoi( argv[4] );

    if ( tsmGeocentricToLatLong( x, y, z, &lat, &lon, &alt ) == FALSE )
      return 1;

    if ( tsmLatLongToUTM( lat, lon, &zone, &east, &north ) == FALSE )
      return 1;

  } else {
    fprintf( stderr, "first arg must be -latlong or -utm\n" );
    return 1;
  }


  /* Display the result of the conversion to the user */

  printf( "Lat/Long = %1.6lf deg, %1.6lf deg\n", lat, lon );
  printf( "UTM      = %1.2lf easting, %1.2lf northing (zone %d)\n",
	  east, north, zone );
  printf( "GCC      = %1.2lf, %1.2lf, %1.2lf\n", x, y, z );

  return 0;
}

/*** EOF: utm_conv.c ***/
