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

#ifdef HAVE_CONFIG_H
# ifdef MSVC
#  include "winbuild/config.h"
# else
#  include "config.h"
# endif
#endif

#include <stdio.h>
#include <string.h>
#include "tvutils.h"

void
listAllFeatures( void )
{
  printf( "ADL Feature Hierarchy:\n\n" );
  printf( "  administrative areas\n" );
  printf( "  . military areas\n" );
  printf( "  . parks\n" );
  printf( "  . . vistas\n" );
  printf( "  . political areas\n" );
  printf( "  . . countries\n" );
  printf( "  . . countries, 1st order divisions\n" );
  printf( "  . . countries, 2nd order divisions\n" );
  printf( "  . . countries, 3rd order divisions\n" );
  printf( "  . . countries, 4th order divisions\n" );
  printf( "  . populated places\n" );
  printf( "  . . cities\n" );
  printf( "  . . . capitals\n" );
  printf( "  . postal areas\n" );
  printf( "  . reference locations\n" );
  printf( "  . reserves\n" );
  printf( "  . school districts\n" );
  printf( "  . statistical areas\n" );
  printf( "  . . census areas\n" );
  printf( "  . . Metropolitan Statistical Areas\n" );
  printf( "  . territorial waters\n" );
  printf( "  . tribal areas\n" );
  printf( " \n" );
  printf( "  hydrographic features\n" );
  printf( "  . aquifers\n" );
  printf( "  . bays\n" );
  printf( "  . . fjords\n" );
  printf( "  . channels\n" );
  printf( "  . deltas\n" );
  printf( "  . drainage basins\n" );
  printf( "  . estuaries\n" );
  printf( "  . floodplains\n" );
  printf( "  . gulfs\n" );
  printf( "  . guts\n" );
  printf( "  . ice masses\n" );
  printf( "  . . glacier features\n" );
  printf( "  . lakes\n" );
  printf( "  . seas\n" );
  printf( "  . . oceans\n" );
  printf( "  . . . ocean currents\n" );
  printf( "  . . . ocean regions\n" );
  printf( "  . streams\n" );
  printf( "  . . rivers\n" );
  printf( "  . . . bends (river)\n" );
  printf( "  . . . rapids\n" );
  printf( "  . . . waterfalls\n" );
  printf( "  . . springs (hydrographic)\n" );
  printf( "  . thermal features\n" );
  printf( " \n" );
  printf( "  land parcels\n" );
  printf( " \n" );
  printf( "  manmade features\n" );
  printf( "  . agricultural sites\n" );
  printf( "  . buildings\n" );
  printf( "  . . capitol buildings\n" );
  printf( "  . . commercial sites\n" );
  printf( "  . . . industrial sites\n" );
  printf( "  . . . . power generation sites\n" );
  printf( "  . . court houses\n" );
  printf( "  . . institutional sites\n" );
  printf( "  . . . correctional facilities\n" );
  printf( "  . . . educational facilities\n" );
  printf( "  . . . medical facilities\n" );
  printf( "  . . . religious facilities\n" );
  printf( "  . . library buildings\n" );
  printf( "  . . museum buildings\n" );
  printf( "  . . post office buildings\n" );
  printf( "  . . research facilities\n" );
  printf( "  . . . data collection facilities\n" );
  printf( "  . . residential sites\n" );
  printf( "  . . . housing areas\n" );
  printf( "  . . . mobile home parks\n" );
  printf( "  . cemeteries\n" );
  printf( "  . disposal sites\n" );
  printf( "  . fisheries\n" );
  printf( "  . fortifications\n" );
  printf( "  . historical sites\n" );
  printf( "  . . archaeological sites\n" );
  printf( "  . hydrographic structures\n" );
  printf( "  . . breakwaters\n" );
  printf( "  . . canals\n" );
  printf( "  . . dam sites\n" );
  printf( "  . . gaging stations\n" );
  printf( "  . . harbors\n" );
  printf( "  . . . marinas\n" );
  printf( "  . . levees\n" );
  printf( "  . . offshore platforms\n" );
  printf( "  . . piers\n" );
  printf( "  . . reservoirs\n" );
  printf( "  . . waterworks\n" );
  printf( "  . landmarks\n" );
  printf( "  . launch facilities\n" );
  printf( "  . mine sites\n" );
  printf( "  . monuments\n" );
  printf( "  . oil fields\n" );
  printf( "  . recreational facilities\n" );
  printf( "  . . amusement parks\n" );
  printf( "  . . camps\n" );
  printf( "  . . performance sites\n" );
  printf( "  . . sports facilities\n" );
  printf( "  . . . pistol ranges\n" );
  printf( "  . storage structures\n" );
  printf( "  . telecommunication features\n" );
  printf( "  . towers\n" );
  printf( "  . transportation features\n" );
  printf( "  . . airport features\n" );
  printf( "  . . . heliports\n" );
  printf( "  . . . seaplane bases\n" );
  printf( "  . . aqueducts\n" );
  printf( "  . . bridges\n" );
  printf( "  . . cableways\n" );
  printf( "  . . locks\n" );
  printf( "  . . parking sites\n" );
  printf( "  . . pipelines\n" );
  printf( "  . . railroad features\n" );
  printf( "  . . roadways\n" );
  printf( "  . . trails\n" );
  printf( "  . . tunnels\n" );
  printf( "  . wells\n" );
  printf( "  . windmills\n" );
  printf( " \n" );
  printf( "  physiographic features\n" );
  printf( "  . alluvial fans\n" );
  printf( "  . arroyos\n" );
  printf( "  . badlands\n" );
  printf( "  . banks (hydrographic)\n" );
  printf( "  . bars (physiographic)\n" );
  printf( "  . basins\n" );
  printf( "  . . storage basins\n" );
  printf( "  . beaches\n" );
  printf( "  . bights\n" );
  printf( "  . capes\n" );
  printf( "  . caves\n" );
  printf( "  . cirques\n" );
  printf( "  . cliffs\n" );
  printf( "  . craters\n" );
  printf( "  . dunes\n" );
  printf( "  . flats\n" );
  printf( "  . gaps\n" );
  printf( "  . isthmuses\n" );
  printf( "  . karst areas\n" );
  printf( "  . ledges\n" );
  printf( "  . massifs\n" );
  printf( "  . mesas\n" );
  printf( "  . mineral deposit areas\n" );
  printf( "  . moraines\n" );
  printf( "  . mountains\n" );
  printf( "  . . continental divides\n" );
  printf( "  . . mountain ranges\n" );
  printf( "  . . mountain summits\n" );
  printf( "  . . ridges\n" );
  printf( "  . . . drumlins\n" );
  printf( "  . natural rock formations\n" );
  printf( "  . . arches (natural formation)\n" );
  printf( "  . plains\n" );
  printf( "  . plateaus\n" );
  printf( "  . playas\n" );
  printf( "  . reefs\n" );
  printf( "  . . coral reefs\n" );
  printf( "  . seafloor features\n" );
  printf( "  . . abyssal features\n" );
  printf( "  . . continental margins\n" );
  printf( "  . . hydrothermal vents\n" );
  printf( "  . . ocean trenches\n" );
  printf( "  . . seamounts\n" );
  printf( "  . . submarine canyons\n" );
  printf( "  . tectonic features\n" );
  printf( "  . . earthquake features\n" );
  printf( "  . . faults\n" );
  printf( "  . . . fault zones\n" );
  printf( "  . . . fracture zones\n" );
  printf( "  . . . rift zones\n" );
  printf( "  . . folds (geologic)\n" );
  printf( "  . . . anticlines\n" );
  printf( "  . . . synclines\n" );
  printf( "  . valleys\n" );
  printf( "  . . canyons\n" );
  printf( "  . volcanic features\n" );
  printf( "  . . lava fields\n" );
  printf( "  . . volcanoes\n" );
  printf( " \n" );
  printf( "  regions\n" );
  printf( "  . biogeographic regions\n" );
  printf( "  . . barren lands\n" );
  printf( "  . . deserts\n" );
  printf( "  . . forests\n" );
  printf( "  . . . petrified forests\n" );
  printf( "  . . . rain forests\n" );
  printf( "  . . . woods\n" );
  printf( "  . . grasslands\n" );
  printf( "  . . habitats\n" );
  printf( "  . . jungles\n" );
  printf( "  . . oases\n" );
  printf( "  . . shrublands\n" );
  printf( "  . . snow regions\n" );
  printf( "  . . tundras\n" );
  printf( "  . . wetlands\n" );
  printf( "  . cadastral areas\n" );
  printf( "  . climatic regions\n" );
  printf( "  . coastal zones\n" );
  printf( "  . economic regions\n" );
  printf( "  . firebreaks\n" );
  printf( "  . land regions\n" );
  printf( "  . . continents\n" );
  printf( "  . . islands\n" );
  printf( "  . . . archipelagos\n" );
  printf( "  . . subcontinents\n" );
  printf( "  . map regions\n" );
  printf( "  . . chart regions\n" );
  printf( "  . . map quadrangle regions\n" );
  printf( "  . . UTM zones\n" );
  printf( "  . research areas\n" );
  printf( "  . . ecological research sites\n" );
  printf( "  . . paleontological sites\n" );
  exit( 0 );
}

int
main( int argc, char **argv )
{
  tvuADLResponse *response;
  char           *place_name = NULL, *feature_type = NULL;
  int            i;

  tvuInit( TVU_NO_GUI );

  for ( i = 1; i < argc; i++ ) {
    if ( strcmp( argv[i], "-oaa" ) == 0 ) {
      SetOAAHostPort( argv[++i] );
      printf( "Connecting to OAA facilitator at %s\n", argv[i] );
      OAAConnect( NULL );
      while ( 1 )
	if (! oaa_process_all_events())
	  break;
      printf( "Disconnecting...\n" );
      return 0;

    } else if ( strncmp( argv[i], "-help", 2 ) == 0 ) {
      printf( "usage: %s [-oaa host> [-name name] [-type name]\n",
	      argv[0] );
      printf( "where,\n" );
      printf( "  -oaa <host:port> = run as an OAA agent\n" );
      printf( "\n" );
      printf( "  -name <name> = a subject name fragment, e.g. yosemite\n" );
      printf( "  -type <name> = the optional feature name, e.g. parks\n" );
      printf( "  -help or -h = display this help text\n" );
      printf( "  -list = displays complete ADL feature hierarchy\n" );
      printf( "\n" );
      /*
      printf( "  -street = the street number and name to resolve\n" );
      printf( "  -city = the city name to resolve\n" );
      printf( "  -province = the province to resolve (also -state)\n" );
      printf( "  -postcode = the postcode to resolve (also -zip)\n" );
      printf( "  -country = the country to resolve (default is \"US\")\n" );
      printf( "\n" );
      */
      exit( 0 );

    } else if ( strcmp( argv[i], "-name" ) == 0 ) {
      place_name = argv[++i];

    } else if ( strcmp( argv[i], "-type" ) == 0 ) {
      feature_type = argv[++i];

    } else if ( strcmp( argv[i], "-list" ) == 0 ) {
      listAllFeatures();

    }

  }

  /* no useful parameters? */

  if ( place_name == NULL ) {
    fprintf( stderr, "Need to specify one of -oaa, -name, or -help\n" );
    exit( 1 );
  }

  /* do the gazetteer call */

  response = tvuADLQuery( place_name, feature_type, NULL, -1 );

  if ( response == NULL ) {
    fprintf( stderr, "Could not find any matches for that criteria\n" );
    exit( 0 );
  }

  printf( "Found %d match", response->matches );
  if ( response->matches != 1 ) printf( "es" );
  if ( response->total_matches != response->matches )
    printf( " (out of %d)", response->total_matches );
  printf( ":\n" );

  for ( i = 0; i < response->matches; i++ ) {
    printf( "%s (%s): %1.10lg lon, %1.10lg lat", response->title[i],
	    response->type[i], response->west[i], response->south[i] );
    if ( ! response->point_source[i] )
      printf( " to %1.10lg lon, %1.10lg lat", response->east[i],
	      response->north[i] );
    printf( "\n" );
  }

  tvuADLFreeResponse( response );

  return 0;
}
