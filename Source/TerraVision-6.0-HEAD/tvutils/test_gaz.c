/*
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
/*

#include <stdio.h>
#include <string.h>
#include "tvutils.h"

int
main( int argc, char **argv )
{
  tvuAddress     address = { "", "", "", "", "US" };
  tvuGazResponse *response;
  int            i, autopick = 0, interactive = 0, quiet = 0;

  tvuInit( TVU_DEFAULT );

  /* output usage info if not enough command line arguments */

  if ( argc <= 2 ) {
    printf( "usage: %s [options]\n", argv[0] );
    printf( "where,\n" );
    printf( "  -street <s> = the street address and number\n" );
    printf( "  -city <s> = the city name, e.g. \"Menlo Park\"\n" );
    printf( "  -state <s> = the state or province name, e.g. CA " );
    printf( "(also -province)\n" );
    printf( "  -zip <s> = the postcode or zipcode, e.g. 94025 " );
    printf( "(also -postcode)\n" );
    printf( "  -country <s> = the country name, e.g. US (default)\n" );
    printf( "  -interactive = lets you manually select for multiple hits\n" );
    printf( "  -autopick = auto pick an address when multiple hits\n" );
    printf( "  -quiet = don't display matched address for -autopick\n\n" );
    exit( 0 );
  }

  /* parse the command line arguments */

  for ( i = 1; i < argc; i++ ) {
    if ( ! strcmp( argv[i], "-street" ) )   address.street = argv[++i];
    if ( ! strcmp( argv[i], "-city" ) )     address.city = argv[++i];
    if ( ! strcmp( argv[i], "-state" ) )    address.province = argv[++i];
    if ( ! strcmp( argv[i], "-province" ) ) address.province = argv[++i];
    if ( ! strcmp( argv[i], "-zip" ) )      address.postcode = argv[++i];
    if ( ! strcmp( argv[i], "-postcode" ) ) address.postcode = argv[++i];
    if ( ! strcmp( argv[i], "-country" ) )  address.country = argv[++i];
    if ( ! strcmp( argv[i], "-interactive" ) ) interactive = 1;
    if ( ! strcmp( argv[i], "-autopick" ) ) autopick = 1;
    if ( ! strcmp( argv[i], "-quiet" ) ) quiet = 1;
  }

  /* do the gazetteer call */

  response = tvuGazetteer( address.street, address.city, address.province,
                           address.postcode, address.country );

 display_result:

  if ( response == NULL ) {
    fprintf( stderr, "Could not geocode this address!\n" );
    exit( 1 );
  }

  /* did we get a selection of possible locations to choose from? */

  if ( response->type == TVU_GAZ_CHOICES ) {
    tvuAddress *addr;
    int        choice;

    /* no address and no options!! What's program to do? */

    if ( response->num_choices == 0 ) {
      fprintf( stderr, "Address does not resolve to a single result.\n" );
      fprintf( stderr, "However, a list of options is not available.\n" );
      exit( 1 );
    }

    /* user wants us to just pick the best one for us... */

    if ( autopick ) {
      choice = 0;

      if ( autopick++ > 1 ) {
	fprintf( stderr, "Aborting autopick...\n" );
	exit( 1 );
      }

      if ( ! quiet ) {
        addr = &response->choice[0];
	fprintf( stderr, "AutoPicking: " );
	if ( addr->street ) fprintf( stderr, "%s, ", addr->street );
	if ( addr->city ) fprintf( stderr, "%s, ", addr->city );
	if ( addr->province ) fprintf( stderr, "%s, ", addr->province );
	if ( addr->country ) fprintf( stderr, "%s.", addr->country );
	fprintf( stderr, "\n" );
      }

    } else {

      /* otherwise, show the user all of the possible locations */

      int i = 0;
      char user_input[128];

      fprintf( stderr, "Multiple possible locations found:\n" );
      while ( i < response->num_choices ) {
	addr = &response->choice[i];
	fprintf( stderr, "%d: ", i+1 );
	if ( addr->street ) fprintf( stderr, "%s, ", addr->street );
	if ( addr->city ) fprintf( stderr, "%s, ", addr->city );
	if ( addr->province ) fprintf( stderr, "%s, ", addr->province );
	if ( addr->country ) fprintf( stderr, "%s.", addr->country );
	fprintf( stderr, "\n" );
	i++;
      }

      if ( ! interactive ) exit( 1 );

      /* ask the user to select one of the above if -interactive */
	
      fprintf( stderr, "\nWhich location did you mean? " );
      fflush( stderr );
      fgets( user_input, 128, stdin );
      i = atoi( user_input );
      if ( i < 1 || i > response->num_choices ) {
	fprintf( stderr, "Invalid selection... exiting\n" );
	exit( 1 );
      }

      choice = i-1;
    }

    /* now, let's try again... */

    response = tvuRetryGazetteer( response, choice );

    goto display_result;
  }

  /* this should never happen! */

  if ( response->type != TVU_GAZ_LATLONG ) {
    fprintf( stderr, "Address does not resolve to a single result!\n" );
    exit( 1 );
  }

  /* got a result! output it to stdout */

  fprintf( stdout, "Latitude = %1.10lg Longitude = %1.10lg\n",
	   response->latitude, response->longitude );

  return 0;
}
