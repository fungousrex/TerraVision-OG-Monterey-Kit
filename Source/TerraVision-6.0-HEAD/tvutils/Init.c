/*
  Filename: Init.c

  Description: Contains the library initialization and closedown routines.

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

#ifdef HAVE_CONFIG_H
# ifdef MSVC
#  include "winbuild/config.h"
# else
#  include "config.h"
# endif
#endif

#include <TerraVision/OAA.h>
#include <prinit.h>

#include "tvutils.h"
#include "Args.h"
#include "Config.h"

int tvu_mode = TVU_DEFAULT;

int tvuADLCallback( char *func, char *args, char **answers );
int tvuGazCallback( char *func, char *args, char **answers );

void
tvuInit( int mode )
{
  tvu_mode = mode;

  /* always have at least an empty string for the following */

  license_accepted = strdup( "" );

  /* Initialise the NSPR library */

  if ( PR_Initialized() == PR_FALSE )
    PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);

  /* set the default directory for flight path files */

  PathSetDefaultDir( tvuGetString( TVU_USER_DIR ) );

  /* set up our OAA callbacks to support the gazetteer functions */

  if ( mode == TVU_NO_GUI ) {

    OAAAddSolvable( "place_to_latlong(Place,Type,OutPlace,OutType,OutNorth,"
 		    "OutSouth,OutEast,OutWest)", tvuADLCallback );

    OAAAddSolvable( "place_to_latlong_bounds(Place,Type,North,South,East,West,"
                    "OutPlace,OutType,OutNorth,OutSouth,OutEast,OutWest)",
	  	    tvuADLCallback );

    OAAAddSolvable( "place_to_latlong_partof(Place,Type,Part,OutPlace,OutType,"
		    "OutPart,OutNorth,OutSouth,OutEast,OutWest)",
		    tvuADLCallback );

    OAAAddSolvable( "place_to_latlong_partof_bounds(Place,Type,Part,North,"
		    "South,East,West,OutPlace,OutType,OutPart,OutNorth,"
		    "OutSouth,OutEast,OutWest)", tvuADLCallback );

    OAAAddSolvable( "address_to_latlong(Street,City,Province,Postcode,Country,"
		    "Lat,Lon)", tvuGazCallback );

    AddTVSolvables( FALSE );
  }

}

void
tvuFinish( void )
{
  tvuArgFinish();
  tvuFreeConfig();
  tvuFreeViewpoints();
}

void
tvuInitCore( tvBase *tvbase )
{
  const char *query = "Querying .geo for initial datasets...";
  const char *loading = "Loading initial datasets...";
  const char *waiting = "Please select a terrain model...";
  char *schema_url, *profile_url;

  if ( tvbase == NULL ) return;

  /* specify the .geo schema (if defined) and update the tvBase message */

  schema_url = tvuGetString( TVU_SCHEMA_URL );
  profile_url = tvuGetString( TVU_PROFILE_URL );

  tvSetString( tvbase, TV_DOTGEO_PROFILE_URL, profile_url );

  if ( schema_url ) {

    tvSetString( tvbase, TV_SPLASH_MESSAGE, query );
    tvSetString( tvbase, TV_SCHEMA_URL, schema_url ); 

  } else if ( tvuGetInt( TVU_NUM_DATASETS ) > 0 ) {

    tvSetString( tvbase, TV_SPLASH_MESSAGE, loading );

  } else {

    tvSetString( tvbase, TV_SPLASH_MESSAGE, waiting );

  }
}

