/*
  Filename: Gazetteer.c

  Description: Contains routines for interfacing to a gazetteer service
  over the Web and convert an address into a lat/long location. Currently,
  we use the whereonearth.com translation service. This only provides
  resolution down to city level. The interface is designed to be independent
  of the actual service however.

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

#include <stdio.h>
#include <stdlib.h>
#include <ghttp.h>
#include "tvutils.h"
#include <ghttp/http_trans.h>
#include <TerraVision/agentlib.h>

#ifndef TRUE
#define TRUE   1
#endif

#ifndef FALSE
#define FALSE  0
#endif

/*
 * int tvuIsUS()
 *
 * Returns whether a country string is designed for the United States.
 * Currently matches any case of "US" and "USA". The reason for doing
 * this is because if we have a US state abbreviation, then we need
 * to expand this to its full name for whereonearth.com. 
 *
 */

static int
tvuIsUS( char *country )
{
  if ( ! country )
    return TRUE;
  if ( strlen( country ) < 2 || strlen( country ) > 3 )
    return FALSE;
  if ( toupper( country[0] ) == 'U' && toupper( country[1] ) == 'S' )
    return TRUE;
  return FALSE;
}

/*
 * char *tvuStateName()
 *
 * Takes a two-letter US state abbreviation and returns the full name
 * of the state. Returns the input string if it is not recognised.
 *
 */

static char *
tvuStateName( char *abbrev )
{
  char state[3];

  if ( abbrev == NULL || strlen(abbrev) != 2 ) return abbrev;

  state[0] = toupper( abbrev[0] );
  state[1] = toupper( abbrev[1] );
  state[2] = '\0';

  if ( ! strcmp( state, "AL" ) ) return "Alabama";
  if ( ! strcmp( state, "AK" ) ) return "Alaska";
  if ( ! strcmp( state, "AZ" ) ) return "Arizona";
  if ( ! strcmp( state, "AR" ) ) return "Arkansas";
  if ( ! strcmp( state, "CA" ) ) return "California";
  if ( ! strcmp( state, "CO" ) ) return "Colorado";
  if ( ! strcmp( state, "CT" ) ) return "Connecticut";
  if ( ! strcmp( state, "DE" ) ) return "Delaware";
  if ( ! strcmp( state, "FL" ) ) return "Florida";
  if ( ! strcmp( state, "GA" ) ) return "Georgia";
  if ( ! strcmp( state, "HI" ) ) return "Hawaii";
  if ( ! strcmp( state, "ID" ) ) return "Idaho";
  if ( ! strcmp( state, "IL" ) ) return "Illinois";
  if ( ! strcmp( state, "IN" ) ) return "Indiana";
  if ( ! strcmp( state, "IA" ) ) return "Iowa";
  if ( ! strcmp( state, "KS" ) ) return "Kansas";
  if ( ! strcmp( state, "KY" ) ) return "Kentucky";
  if ( ! strcmp( state, "LA" ) ) return "Louisiana";
  if ( ! strcmp( state, "ME" ) ) return "Maine";
  if ( ! strcmp( state, "MD" ) ) return "Maryland";
  if ( ! strcmp( state, "MA" ) ) return "Massachusetts";
  if ( ! strcmp( state, "MI" ) ) return "Michigan";
  if ( ! strcmp( state, "MN" ) ) return "Minnesota";
  if ( ! strcmp( state, "MS" ) ) return "Mississippi";
  if ( ! strcmp( state, "MO" ) ) return "Missouri";
  if ( ! strcmp( state, "MT" ) ) return "Montana";
  if ( ! strcmp( state, "NE" ) ) return "Nebraska";
  if ( ! strcmp( state, "NV" ) ) return "Nevada";
  if ( ! strcmp( state, "NH" ) ) return "New Hampshire";
  if ( ! strcmp( state, "NJ" ) ) return "New Jersey";
  if ( ! strcmp( state, "NM" ) ) return "New Mexico";
  if ( ! strcmp( state, "NY" ) ) return "New York";
  if ( ! strcmp( state, "NC" ) ) return "North Carolina";
  if ( ! strcmp( state, "ND" ) ) return "North Dakota";
  if ( ! strcmp( state, "OH" ) ) return "Ohio";
  if ( ! strcmp( state, "OK" ) ) return "Oklahoma";
  if ( ! strcmp( state, "OR" ) ) return "Oregon";
  if ( ! strcmp( state, "PA" ) ) return "Pennsylvania";
  if ( ! strcmp( state, "RI" ) ) return "Rhode Island";
  if ( ! strcmp( state, "SC" ) ) return "South Carolina";
  if ( ! strcmp( state, "SD" ) ) return "South Dakota";
  if ( ! strcmp( state, "TN" ) ) return "Tennessee";
  if ( ! strcmp( state, "TX" ) ) return "Texas";
  if ( ! strcmp( state, "UT" ) ) return "Utah";
  if ( ! strcmp( state, "VT" ) ) return "Vermont";
  if ( ! strcmp( state, "VA" ) ) return "Virginia";
  if ( ! strcmp( state, "WA" ) ) return "Washington";
  if ( ! strcmp( state, "WV" ) ) return "West Virginia";
  if ( ! strcmp( state, "WI" ) ) return "Wisconsin";
  if ( ! strcmp( state, "WY" ) ) return "Wyoming";

  return abbrev;
}

/*
 * void tvuGetNextValue()
 * void tvuGetNextTag()
 * void tvuGetNextText()
 *
 * Routines for parsing data out of a web page.
 *
 */

static void
tvuGetNextValue( char *input, char *output )
{
  *output = '\0';
  while( *input != '\"' )
    input++;
  input++;
  while ( *input != '\"' )
    *(output++) = *(input++);
  *output = '\0';
}

static void
tvuGetNextTag( char **input, char *output, int init_depth )
{
  char *ptr = *input;
  int  depth = init_depth;

  while ( isspace( *ptr ) ) ptr++;

  for ( ;; ptr++ ) {
    if ( *ptr == '<' ) depth++;
    if ( *ptr == '>' ) {
      depth--;
      if ( depth == 0 ) {
	ptr++;
	break;
      }
    }
    if ( depth == 0 ) break;
  }

  *output = '\0';
  while ( *ptr != '<' ) *(output++) = *(ptr++);
  *output = '\0';

  *input = ptr;

  /*
  *output = '\0';
  while ( *ptr != '>' || *(ptr+1) <= ' ' || *(ptr+1) == '<' ) ptr++;
  ptr++;
  while ( *ptr != '<' ) *(output++) = *(ptr++);
  *output = '\0';
  *input = ptr;
  */
}

static void
tvuGetNextText( char *input, char *output )
{
  *output = '\0';
  while( isspace(*input) )
    input++;
  while ( *input != '\"' && *input != '<' && *input > ' ' && *input != '&' )
    *(output++) = *(input++);
  *output = '\0';
}

/*
 * int tvuConvSpaces()
 *
 * Take a string and allocate a new string, replacing all space, tab,
 * newline, etc. symbols with the specified string. This is used to
 * convert spaces to escape sequences for shipping in URLs.
 *
 */

static char *
tvuConvSpaces( char *source, char *replacement )
{
  char buffer[1024] = "", *ptr = buffer;
  int i, replen = strlen( replacement );

  if ( source == NULL ) return NULL;

  for ( i = 0; i < strlen(source); i++ ) {
    if ( source[i] <= ' ' ) {
      strcat( buffer, replacement );
      ptr += replen;
    } else {
      *ptr++ = source[i];
      *ptr = '\0';
    }
  }      
   
  return strdup( buffer );
}

/*
 * int tvuGazIsFound()
 *
 * Returns TRUE if the returned HTML page contains a match for the
 * selected city address. Under whereonearth.com, we just search for
 * the string "No matches were found".
 * 
 */

static int
tvuGazIsFound( char *buffer, int bytes )
{
  char const *errtag = "No matches were found";
  char *ptr, *endptr;

  if ( buffer == NULL || bytes <= 0 )
    return FALSE;

  ptr = buffer;
  endptr = buffer + bytes - 1;
  while ( ptr < endptr  ) {
    if ( *ptr == 'N' && strncmp( ptr, errtag, strlen(errtag) ) == 0 )
      return FALSE;
    ptr++;
  }

  return TRUE;
}

/*
 * tvuAddress *tvuGazGetChoices()
 *
 * Parses an HTML result from whereonearth.com and creates a list of
 * choices for a partial match.
 *
 */

static tvuAddress *
tvuGazGetChoices( char *buffer, int bytes, int *choices )
{
  char const *cgitag = "=\"http://www.whereonearth.com/cgi-bin/woe_geozip";
  char *ptr, *endptr, value[256];
  tvuAddress *address;
  int maxitems = 10, items = 0;

  if ( buffer == NULL || choices == NULL || bytes <= 0 )
    return NULL;

  if ( ! ( address = calloc( sizeof(tvuAddress), maxitems ) ) )
    return NULL;

  ptr = buffer;
  endptr = buffer + bytes - 1;
  while ( ptr < endptr && items < maxitems ) {
    if ( *ptr == '=' && strncmp( ptr, cgitag, strlen(cgitag) ) == 0 ) {
      tvuGetNextTag( &ptr, value, 1 );
      address[items].city = strdup( value );
      ptr = strstr( ptr, "<font" );
      tvuGetNextTag( &ptr, value, 0 );
      ptr = strstr( ptr, "<font" );
      tvuGetNextTag( &ptr, value, 0 );
      address[items].province = strdup( value );
      ptr = strstr( ptr, "<font" );
      tvuGetNextTag( &ptr, value, 0 );
      address[items].country = strdup( value );
      items++;
    }
    ptr++;
  }

  *choices = items;
  return address;
}

/*
 * int tvuGazGetLoc()
 *
 * Parses the HTML result from whereonearth.com and extracts the latitude
 * and longitude of the selected city address.
 *
 */

static int
tvuGazGetLoc( char *buffer, int bytes, double *latitude, double *longitude )
{
  char const *lattag = "=\"latitude\"";
  char const *longtag = "=\"longitude\"";
  char *ptr, *endptr, value[128];
  int  coords = 0;

  if ( buffer == NULL || bytes <= 0 )
    return FALSE;

  ptr = buffer;
  endptr = buffer + bytes - 1;
  while ( ptr < endptr && coords < 2 ) {
    if ( *ptr == '=' ) {
      if ( ! strncmp( ptr, lattag, strlen(lattag) ) ) {
	tvuGetNextValue( ptr + strlen(lattag) + 1, value );
	*latitude = atof( value );
	coords++;
      } else if ( ! strncmp( ptr, longtag, strlen(longtag) ) ) {
	tvuGetNextValue( ptr + strlen(longtag) + 1, value );
	*longitude = atof( value );
	coords++;
      }
    }
    ptr++;
  }

  return ( coords == 2 );
}

/*
 * tvuGazResponse *tvuGazetteer_mapblast()
 *
 * This function uses the MapBlast.com service to translate
 * street names into a lat/long coordinate. If there is not an
 * exact match then the function will return an error (no
 * partial match support just yet).
 *
 */

static tvuGazResponse *
tvuGazetteer_mapblast( char *street, char *city, char *province,
		       char *postcode, char *country )
{
  char   url[1024], *doc, *ptr;
  char   *str, *zip, *cty, *sta;
  int    size, success;
  double lat, lon;

  http_trans_conn *http_conn = NULL;
  tvuGazResponse *response;

  if ( postcode == NULL && province == NULL && city == NULL ) return NULL;

  if ( tvuIsUS( country ) ) country = "USA";

  /* convert any spaces to "+" symbols for sending to server */

  str = tvuConvSpaces( street, "+" );
  cty = tvuConvSpaces( city, "+" );
  sta = tvuConvSpaces( province, "+" );
  zip = tvuConvSpaces( postcode, "+" );

  /* create the query URL in MapBlasts! format */

  if ( str == NULL ) str = strdup( "" );
  if ( zip != NULL && strlen(zip) > 0 ) {
    sprintf( url, "http://www.mapblast.com/myblast/map.mb?CMD=GEO&AD4=%s&"
	     "AD2_street=%s&AD2=%s&AD3=%s&req_action=crmap",
	     country, str, str, zip );
  } else {
    sprintf( url, "http://www.mapblast.com/myblast/map.mb?CMD=GEO&AD4=%s&"
	     "AD2_street=%s&AD2=%s&AD3=%s%%2C+%s&req_action=crmap",
	     country, str, str, cty, sta );
  }

  /* send the query */

  success = tsmUrlToMemory( &http_conn, url, &doc, &size );

  if ( str ) free( str );
  if ( cty ) free( cty );
  if ( sta ) free( sta );
  if ( zip ) free( zip );

  if ( ! success ) return NULL;

  /* extract the latitude value from the tag with name="chklat" */

  if ( ( ptr = strstr( doc, "Lat:" ) ) ) {
    char buffer[128];
    tvuGetNextText( ptr + 5, buffer );
    lat = atof( buffer );
  } else
    success = FALSE;

  /* extract the latitude value from the tag with name="chklon" */

  if ( ( ptr = strstr( doc, "Lon:" ) ) ) {
    char buffer[128];
    tvuGetNextText( ptr + 5, buffer );
    lon = atof( buffer );
  } else
    success = FALSE;

  /* all done! */

  free( doc );
  
  if ( ! success ) return NULL;

  /* allocate a response structure for the result and return it */

  if ( ! ( response = calloc( sizeof(tvuGazResponse), 1 ) ) )
    return NULL;

  response->type      = TVU_GAZ_LATLONG;
  response->latitude  = lat;
  response->longitude = lon;

  return response;
}

/*
 * tvuGazResponse *tvuGazetteer_whereone()
 *
 * This function uses the WhereOnEarth.com service to translate
 * world city names into a structure with either the lat/long
 * coordinates (in the case of an exact match), a list of
 * choices (in the case of a partial match), or an error (NULL)
 *
 */

static tvuGazResponse *
tvuGazetteer_whereone( char *street, char *city, char *province,
		       char *postcode, char *country, int choice )
{
  char           body_text[2048], *result_text;
  double         lat, lon;
  int            result_len;
  ghttp_request  *request = NULL;
  ghttp_status   status;
  tvuGazResponse *response;

  if ( choice >= 0 )
    sprintf( body_text, "mode=3&num=%d&password=&", choice );
  else
    strcpy( body_text, "mode=2&password=&" );

  if ( city && strlen(city) > 0 ) {
    strcat( body_text, "town=" );
    strcat( body_text, city );
    strcat( body_text, "&" );
  }
  if ( province && strlen(province) > 0  ) {
    strcat( body_text, "province=" );
    if ( strlen( province ) == 2 && tvuIsUS( country ) )
      strcat( body_text, tvuStateName(province) );
    else
      strcat( body_text, province );
    strcat( body_text, "&" );
  }
  if ( postcode && strlen(postcode) > 0  ) {
    strcat( body_text, "zipcode=" );
    strcat( body_text, postcode );
    strcat( body_text, "&" );
  }
  if ( country && strlen(country) > 0  ) {
    strcat( body_text, "country=" );
    strcat( body_text, country );
    strcat( body_text, "&" );
  }
  body_text[strlen(body_text)-1] = '\0';

  /* Allocate a new empty request object */
  request = ghttp_request_new();
  /* Set the URI for the request object */
  ghttp_set_uri(request, "http://www.whereonearth.com/cgi-bin/woe_geozip");
  /* Close the connection after you are done. */
  ghttp_set_header(request, http_hdr_Connection, "close");
  /* do a POST, not a GET */
  ghttp_set_type(request, ghttp_type_post);
  /* set the body of the POST request */
  ghttp_set_body(request, body_text, strlen(body_text) );
  /* Prepare the connection */
  ghttp_prepare(request);
  /* Process the request */
  status = ghttp_process(request);

  /* check for an error in the request */
  if ( status == ghttp_error ) {
    ghttp_request_destroy(request);
    return NULL;
  }

  /* get the result text */
  result_text = ghttp_get_body(request);
  result_len  = ghttp_get_body_len(request);

  /* allocate a response structure */

  if ( ! ( response = calloc( sizeof(tvuGazResponse), 1 ) ) ) {
    ghttp_request_destroy(request);
    return NULL;
  }

  /* did we find a location? */
  if ( ! tvuGazIsFound( result_text, result_len ) ) {
    ghttp_request_destroy(request);
    return NULL;
  }

  /* look for the latitude longitude result in the text */
  if ( tvuGazGetLoc( result_text, result_len, &lat, &lon ) ) {
    response->type = TVU_GAZ_LATLONG;
    response->latitude = lat;
    response->longitude = lon;
    ghttp_request_destroy(request);
    return response;
  }

  /* look for a list of alternatives */
  response->choice = tvuGazGetChoices( result_text, result_len,
					&response->num_choices );
  if ( response->choice ) {
    response->type = TVU_GAZ_CHOICES;
    ghttp_request_destroy(request);
    return response;
  }

  /* otherwise, no go! */
  ghttp_request_destroy(request);
  return NULL;
}

/*
 * tvuGazResponse *tvuGazetteer()
 *
 * This exported function is the one that does all the work. You give
 * it a street address and it returns a structure with either the
 * lat/long coordinates (in the case of an exact match), a list of
 * choices (in the case of a partial match), or an error (NULL).
 *
 * The function will use the MapBlast! web service to translate
 * street-level addresses, otherwise it will use the WhereOnEarth
 * service to resolve city-level requests anywhere in the world.
 * If MapBlast! fails, then we try WhereOnEarth
 *
 */

tvuGazResponse *
tvuGazetteer( char *street, char *city, char *province, char *postcode,
	      char *country )
{
  tvuGazResponse *res;

  if ( ( street != NULL && strlen( street ) > 0 ) ) { 
    res = tvuGazetteer_mapblast( street, city, province, postcode, country );
    if ( res ) return res;
  }

  return tvuGazetteer_whereone( street, city, province, postcode, country, -1);
}

/*
 * tvuGazResponse *tvuRetryGazetteer( tvuGazResponse *response, int choice )
 *
 * Takes a tvuGazResponse, as returned from a call to tvuGazetteer,
 * and then tries to look up the specified choice in the choice
 * array of possible matches. If there is no such choice entry,
 * then this will return NULL.
 *
 * This call will free the tvuGazResponse structure passed in.
 *
 */

tvuGazResponse *
tvuRetryGazetteer( tvuGazResponse *response, int choice )
{
  tvuAddress     *addr;
  tvuGazResponse *newresponse;

  if ( response == NULL ) return NULL;
  if ( choice < 0 || choice >= response->num_choices ) {
    tvuFreeGazResponse( response );
    return NULL;
  }

  addr = &response->choice[choice];

  if ( ( addr->street != NULL && strlen( addr->street ) > 0 ) )
    newresponse = 
      tvuGazetteer_mapblast( addr->street, addr->city, addr->province, 
			     addr->postcode, addr->country );
  else
    newresponse = 
      tvuGazetteer_whereone( addr->street, addr->city, addr->province,
			     addr->postcode, addr->country, choice );

  tvuFreeGazResponse( response );
  return newresponse;

}

/*
 * void tvuFreeGazResponse()
 *
 * Frees the memory for the structure returned by tvuGazetteer()
 *
 */

void
tvuFreeGazResponse( tvuGazResponse *response )
{
  if ( response ) {
    if ( response->choice )
      free( response->choice );
    free( response );
  }
}

/*
 * int example_main()
 *
 * an example main() function that shows how to use the tvuGazetteer call.
 *
 */

static int
example_main( int argc, char **argv ) 
{
  tvuGazResponse *response;

  if ( argc < 4 ) exit( 0 );

  if ( ( response = tvuGazetteer( NULL, argv[1], argv[2], argv[3], "US" ) ) ){
    if ( response->type == TVU_GAZ_LATLONG )
      printf( "Location: %1.3lf, %1.3lf\n", response->latitude,
	      response->longitude );
    else {
      int i;
      for ( i = 0; i < response->num_choices; i++ ) 
	printf( "Choice %d: %s, %s, %s\n", i, response->choice[i].city,
		response->choice[i].province, response->choice[i].country );
    }

    tvuFreeGazResponse( response );
  } else
    printf( "Could not locate!\n" );

  return 0;
}

/* OAA callback for implementing the "address_to_latlong" solvable */
int
tvuGazCallback( char *func, char *args, char **answers )
{
  char const     *funcName = "tvuGazCallback";
  char           *street, *city, *province, *postcode, *country;
  tvuGazResponse *response;
  int            length = list_len( args );
  char           buffer[1024];

  /* Setup a default answer: predicate succeeds with no variables returned */

  *answers = (char *) malloc( strlen(func) + strlen(args) + 10 );
  if ( *args )
    sprintf( *answers, "[%s(%s)]", func, args );
  else
    sprintf( *answers, "[%s]", func );

  /* check that we have the right number of arguments */

  if ( length != 7 ) {
    TSMTRACE(( OAA_MASK, funcName, "Illegal number of args: %d", length ));
    return 0;
  }

  /* Get the name of the function to be performed and its arguments */

  NthElt( args, 1, &street );
  remove_quotes( street );

  NthElt( args, 2, &city );
  remove_quotes( city );

  NthElt( args, 3, &province );
  remove_quotes( province );

  NthElt( args, 4, &postcode );
  remove_quotes( postcode );

  NthElt( args, 5, &country );
  remove_quotes( country );

  if ( strlen( country ) == 0 )
    country = strdup( "US" );

  TSMTRACE(( OAA_MASK, funcName, "OAA: %s, %s, %s, %s, %s", street,
	     city, province, postcode,
	     country ));

  /* sent the query to the address gazetteer(s) */

  response = tvuGazetteer( street, city, province, postcode, country );

  if ( response == NULL ) {
    TSMTRACE(( OAA_MASK, funcName, "Couldn't find a match" ));
    OAA_FREE( street );
    OAA_FREE( city );
    OAA_FREE( province );
    OAA_FREE( postcode );
    OAA_FREE( country );
    return 0;
  }

  /* if we get multiple choices, then auto pick the first one */

  if ( response->type == TVU_GAZ_CHOICES )
    response = tvuRetryGazetteer( response, 0 );

  /* if we still couldn't resolve the address, then we're SOL */

  if ( response->type != TVU_GAZ_LATLONG )
    return 0;

  /* otherwise, we have a single answer to return */

  free( *answers );  /* discard the default answer */
  *answers = NULL;
  *answers = tsmStringCat( *answers, "[" );

  sprintf( buffer, "%s('%s','%s','%s','%s','%s',%1.10lg,%1.10lg)",
	   func, street, city, province, postcode, country,
	   response->latitude, response->longitude );

  *answers = tsmStringCat( *answers, buffer );
  *answers = tsmStringCat( *answers, "]" );

  tvuFreeGazResponse( response );

  TSMTRACE(( OAA_MASK, funcName, "OAA: Returning answer %s", *answers ));
  OAA_FREE( street );
  OAA_FREE( city );
  OAA_FREE( province );
  OAA_FREE( postcode );
  OAA_FREE( country );
  return 1;
}
