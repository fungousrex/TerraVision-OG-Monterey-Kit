/*
  Filename: ADL.c

  Description: Contains routines for interfacing to the ADL gazetteer service
  over the Web. ADL is the Alexandria Digital Library at University of 
  California, Santa Barbara. This provides a service to convert points of
  interest into lat/long coordinates.

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
#include <ctype.h>
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

#define PLACE_TO_LATLONG                1
#define PLACE_TO_LATLONG_BOUNDS         2
#define PLACE_TO_LATLONG_PARTOF         3
#define PLACE_TO_LATLONG_PARTOF_BOUNDS  4

/* set ADL_DEBUG to 1 for debug messages */
#define ADL_DEBUG     1

static char *adl_server = "http://middleware.alexandria.ucsb.edu:8080/mw/";

char *
tvuStringCat( char *string, char *suffix )
{
  char *newstring;
  if ( suffix == NULL ) return string;
  if ( string == NULL ) return strdup( suffix );
  newstring = realloc( string, strlen( string ) + strlen( suffix ) + 1 );
  strcat( newstring, suffix );
  return newstring;
}

void
adl_debug( const char *msg, ... )
{
  va_list ap;

#ifndef ADL_DEBUG
  if ( tvu_mod != TVU_NO_GUI ) return;
#endif
  va_start( ap, msg );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
}

/* perform URL encoding on a string and return that newly allocated */
/* string (or NULL). This means converting spaces to %20 etc. */
static char *
url_encode( char *string )
{
  char *newstring, *returnstring;
  int  i, j, len;
  char hex[5];

  if ( string == NULL ) return NULL;
  if ( ( len = strlen( string ) ) == 0 ) return string;
  
  newstring = calloc( strlen(string)+1, 3 );
  for ( i = j = 0; i < len; i++ ) {
    if ( string[i] == ' ' || string[i] == ';' || string[i] == '/' ||
	 string[i] == '?' || string[i] == ':' || string[i] == '@' ||
	 string[i] == '`' || string[i] == '"' || string[i] == '<' ||
	 string[i] == '>' || string[i] == '#' || string[i] == '%' ||
	 string[i] == '{' || string[i] == '}' || string[i] == '|' ||
	 string[i] == '^' || string[i] == '~' || string[i] == '\\' || 
	 string[i] == '[' || string[i] == ']' ) {
      /* URL encode this character using the %XX format */
      sprintf( hex, "%%%02X", string[i] );
      strcat( newstring, hex );
      j += 3;
    } else {
      newstring[j++] = string[i];
    }
  }

  /* optimize the memory for this string and return it */
  returnstring = strdup( newstring );
  free( newstring );
  return returnstring;
}

static char *
post_adl_xml( char *servlet, char *post_body )
{
  char           *encoded_text = NULL, *result_text, url[512];
  char const     *content_type = "application/x-www-form-urlencoded";
  ghttp_request  *request = NULL;

  /* create the URL for the appropriate servlet */
  strcpy( url, adl_server );
  strcat( url, servlet );

  /* it doesn't seem to be necessary to URL encode the string, */
  /* but the ADL guys say that we should do this, so here goes */
  encoded_text = url_encode( post_body );

  /* Allocate a new empty request object */
  request = ghttp_request_new();

  /* Set the URI for the request object */
  ghttp_set_uri( request, url );

  /* the Content-Type must be set to application/x-www-form-urlencoded */
  ghttp_set_header( request, http_hdr_Content_Type, content_type );

  /* do a POST, not a GET */
  ghttp_set_type( request, ghttp_type_post );

  /* set the body of the POST request */
  if ( encoded_text )
    ghttp_set_body( request, encoded_text, strlen(encoded_text) );

  /* Prepare and process the connection */
  ghttp_prepare( request );
  if ( ghttp_process( request ) == ghttp_error ) {
    char const *error = ghttp_get_error( request );
    if ( error ) fprintf( stderr, "ERROR: %s\n", error );
    ghttp_request_destroy( request );
    return NULL;
  }

  /* get the result text */
  result_text = strdup( ghttp_get_body( request ) );

  if ( encoded_text ) free( encoded_text );

  ghttp_request_destroy( request );
  return result_text;  
}

static char *
get_adl_xml( char *servlet, char *params )
{
  char           *encoded_text = NULL, *result_text, *url;
  ghttp_request  *request = NULL;

  encoded_text = url_encode( params );

  /* create the URL for the appropriate servlet */
  url = calloc( strlen( adl_server ) + strlen( encoded_text ) + 256, 1 );
  if ( ! url ) return NULL;

  strcpy( url, adl_server );
  strcat( url, servlet );
  if ( encoded_text ) {
    strcat( url, "?" );
    strcat( url, encoded_text );
  }

  /* Allocate a new empty request object */
  request = ghttp_request_new();

  /* Set the URI for the request object */
  ghttp_set_uri( request, url );

  /* do a GET, not a POST */
  ghttp_set_type( request, ghttp_type_get );

  /* Prepare and process the connection */
  ghttp_prepare( request );
  if ( ghttp_process( request ) == ghttp_error ) {
    char const *error = ghttp_get_error( request );
    if ( error ) fprintf( stderr, "ERROR: %s\n", error );
    ghttp_request_destroy( request );
    return NULL;
  }

  /* get the result text */
  result_text = strdup( ghttp_get_body( request ) );

  adl_debug( "\n%s", result_text );

  if ( encoded_text ) free( encoded_text );
  free( url );

  ghttp_request_destroy( request );
  return result_text;  
}

static int
adl_query( char *query, char *feature_type, double area[4] )
{
  char *xml = NULL, *result;
  int  id, expressions = 0;

  /* an empty string is the same as a NULL string - no data */

  if ( query != NULL && query[0] == '\0' )
    query = NULL;
  if ( feature_type != NULL && feature_type[0] == '\0' )
    feature_type = NULL;

  /* check how many expressions we need to build */

  if ( query ) expressions++;
  if ( feature_type ) expressions++;
  if ( area ) expressions++;

  xml = tvuStringCat( xml, "query=" );
  xml = tvuStringCat( xml, "<?xml version=\"1.0\"?>\n" );
  xml = tvuStringCat( xml, "<!DOCTYPE ADL-query SYSTEM \"http://www.alexandria.ucsb.edu/docs/metadata/ADL-query.dtd\">\n" );
  xml = tvuStringCat( xml, "<ADL-query>\n" );
  xml = tvuStringCat( xml, "<collections>\n" );
  xml = tvuStringCat( xml, "<collection>adl_gazetteer</collection>\n" );
  xml = tvuStringCat( xml, "</collections>\n" );

  if ( expressions > 1 )
    xml = tvuStringCat( xml, "<and>\n" );

  if ( feature_type != NULL ) {
    xml = tvuStringCat( xml, "<hierarchical-constraint>\n" );
    xml = tvuStringCat( xml, "<bucket>types</bucket>\n" );
    xml = tvuStringCat( xml, "<operator>is-a</operator>\n" );
    xml = tvuStringCat( xml, "<term vocabulary=\"ADL Feature Type Thesaurus\">" );
    xml = tvuStringCat( xml, feature_type );
    xml = tvuStringCat( xml, "</term>\n" );
    xml = tvuStringCat( xml, "</hierarchical-constraint>\n" );
  }

  if ( area != NULL ) {
    char buffer[128];
    xml = tvuStringCat( xml, "<spatial-constraint>\n" );
    xml = tvuStringCat( xml, "<bucket>geographic-locations</bucket>\n" );
    xml = tvuStringCat( xml, "<operator>is-contained-in</operator>\n" );
    xml = tvuStringCat( xml, "<box>\n" );
    sprintf( buffer, "<north>%1.10lg</north>\n", area[0] );
    xml = tvuStringCat( xml, buffer );
    sprintf( buffer, "<south>%1.10lg</south>\n", area[1] );
    xml = tvuStringCat( xml, buffer );
    sprintf( buffer, "<east>%1.10lg</east>\n", area[2] );
    xml = tvuStringCat( xml, buffer );
    sprintf( buffer, "<west>%1.10lg</west>\n", area[3] );
    xml = tvuStringCat( xml, buffer );
    xml = tvuStringCat( xml, "</box>\n" );
    xml = tvuStringCat( xml, "</spatial-constraint>\n" );
  }

  if ( query != NULL ) {
    xml = tvuStringCat( xml, "<textual-constraint>\n" );
    xml = tvuStringCat( xml, "<bucket>subject-related-text</bucket>\n" );

    /* contains-all-words, contains-any-words, contains-phrase */
    xml = tvuStringCat( xml, "<operator>contains-phrase</operator>\n" );

    xml = tvuStringCat( xml, "<text>" );
    xml = tvuStringCat( xml, ( query ) ? query : "" );
    xml = tvuStringCat( xml, "</text>\n" );
    xml = tvuStringCat( xml, "</textual-constraint>\n" );
  }

  if ( expressions > 1 )
    xml = tvuStringCat( xml, "</and>\n" );

  xml = tvuStringCat( xml, "</ADL-query>\n" );

  adl_debug( "\n\n%s\n", xml );

  if ( ( result = post_adl_xml( "query", xml ) ) == NULL ) {
    free( xml );
    return 0;
  }

  id = atoi( result );
  free( result );
  free( xml );
  return id;
}

/* search for a tag, e.g. "<result-count>", in an XML formatted string */

static char *
get_tag( char *xml, char *tag )
{
  char *pos = strstr( xml, tag ), *pos_end, *string;
  int  bytes;

  /* locate the start of the tag in the XML string */

  if ( ! pos ) return NULL;

  pos++;
  while ( *(pos-1) != '>' ) pos++;
  pos_end = strchr( pos, '<' );
  bytes = ( pos_end ) ? pos_end - pos : xml + strlen( xml ) - pos;

  /* return the string */

  if ( ( string = calloc( bytes + 1, 1 ) ) == NULL )
    return NULL;

  memcpy( string, pos, bytes );
  return string;
}

void
tvuADLFreeResponse( tvuADLResponse *response )
{
  int i;
  if ( ! response ) return;

  for ( i = 0; i < response->matches; i++ ) {
    if ( response->title && response->title[i] ) free( response->title[i] );
    if ( response->type && response->type[i] ) free( response->type[i] );
    if ( response->id && response->id[i] ) free( response->id[i] );
  }

  if ( response->point_source ) free( response->point_source );
  if ( response->north ) free( response->north );
  if ( response->south ) free( response->south );
  if ( response->east ) free( response->east );
  if ( response->west ) free( response->west );
  if ( response->title ) free( response->title );
  if ( response->type ) free( response->type );
  if ( response->id ) free( response->id );

  free( response );
}

tvuADLResponse *
tvuADLQuery( char *query, char *feature_type, double area[4], int max_matches )
{
  char           body_text[512], lower_type[512], *result, *tag;
  int            id, i;
  tvuADLResponse *response;

  /* use zero value indicates a default max value, -1 means no limit */
  
  if ( max_matches == 0 ) max_matches = 7;

  /* create a lowercase version of the type string */

  strcpy( lower_type, ( feature_type ) ? feature_type : "" );
  for ( i = 0; i < strlen( lower_type ); i++ )
    lower_type[i] = tolower( lower_type[i] );

  /* check for Prolog/ICL variable name instead of a type string */
  /* or for a type string of "any". Set this to mean any type */

  if ( strcmp( lower_type, "any" ) == 0 )
    lower_type[0] = '\0';;

  /* do the XML query to the ADL middleware */

  if ( ( id = adl_query( query, lower_type, area ) ) == 0 )
    return NULL;

  /* do a WAIT, to wait under the server has completed the search */

  sprintf( body_text, "id=%d&op=wait", id );
  if ( ( result = get_adl_xml( "results", body_text ) ) == NULL )
    return NULL;
  free( result );

  /* do a STATUS, to find out how many hits we found */
  
  sprintf( body_text, "id=%d&op=status", id );
  if ( ( result = get_adl_xml( "results", body_text ) ) == NULL )
    return NULL;

  /* discover how many matches there are for this query */

  if ( ( tag = get_tag( result, "<result-count>" ) ) == NULL ) {
    free( result );
    return NULL;
  }
  free( result );

  /* create a response structure */

  if ( ! ( response = calloc( sizeof( tvuADLResponse ), 1 ) ) )
    return NULL;

  response->total_matches = response->matches = atoi( tag );
  if ( response->matches > max_matches && max_matches > 0 )
    response->matches = max_matches;

  free( tag );

  if ( response->matches > 0 ) {
    response->north        = calloc( sizeof( double ), response->matches );
    response->south        = calloc( sizeof( double ), response->matches );
    response->east         = calloc( sizeof( double ), response->matches );
    response->west         = calloc( sizeof( double ), response->matches );
    response->title        = calloc( sizeof( char * ), response->matches );
    response->id           = calloc( sizeof( char * ), response->matches );
    response->type         = calloc( sizeof( int ), response->matches );
    response->point_source = calloc( sizeof( int ), response->matches );
  } else {
    free( response );
    return NULL;
  }

  /* now get the details for each of these matches */

  for ( i = 0; i < response->matches; i++ ) {
    sprintf( body_text, "id=%d&op=get&index=%d", id, i );
    if ( ( result = get_adl_xml( "results", body_text ) ) ) {

      /* parse out the ADL metadata ID string */

      if ( ( tag = get_tag( result, "<identifier>" ) ) )
	response->id[i] = tag;

      if ( strstr( result, "<box>" ) != NULL ) {

	/* location is a bounding box */

	response->point_source[i] = FALSE;

	if ( ( tag = get_tag( result, "<north>" ) ) ) {
	  response->north[i] = atof( tag );
	  free( tag );
	}

	if ( ( tag = get_tag( result, "<south>" ) ) ) {
	  response->south[i] = atof( tag );
	  free( tag );
	}

	if ( ( tag = get_tag( result, "<east>" ) ) ) {
	  response->east[i] = atof( tag );
	  free( tag );
	}

	if ( ( tag = get_tag( result, "<west>" ) ) ) {
	  response->west[i] = atof( tag );
	  free( tag );
	}

      } else {

	/* location is a point source */

	response->point_source[i] = TRUE;

	if ( ( tag = get_tag( result, "<latitude>" ) ) ) {
	  response->north[i] = response->south[i] = atof( tag );
	  free( tag );
	}

	if ( ( tag = get_tag( result, "<longitude>" ) ) ) {
	  response->east[i] = response->west[i] = atof( tag );
	  free( tag );
	}

      }

      if ( ( tag = get_tag( result, "<title>" ) ) ) 
	response->title[i] = tag;

      if ( ( tag = get_tag( result, "<type>" ) ) ) 
	response->type[i] = tag;

      free( result );
    }
  }

  /* do a DELETE, to remove this ID from the server */
  
  sprintf( body_text, "id=%d&op=delete", id );
  if ( ( result = get_adl_xml( "results", body_text ) ) )
    free( result );

  return response;
}

static int
adl_get_parts( char *adl_id, char ***parts )
{
  char *result, *tag, params[512];
  int  count, parts_size = 10;

  sprintf( params, "view=full&id=%s", adl_id );

  if ( ( result = get_adl_xml( "metadata", params ) ) == NULL )
    return 0;

  *parts = (char **) calloc( sizeof(char *), parts_size );
  count = 0;
  
  tag = strstr( result, "IsPartOf" );
  while ( tag ) {
    char *end_group = strstr( tag, "</group" );
    char *field_name = strstr( tag, "name=\"Name\"" );

    if ( end_group && field_name && field_name < end_group ) {
      char *text = get_tag( field_name, "value" );
      if ( text ) {
	if ( count >= parts_size ) {
	  parts_size += 10;
	  *parts = (char **) realloc( *parts, parts_size * sizeof(char *) );
	}
	(*parts)[count++] = text;
      }
    }

    tag = strstr( tag+1, "IsPartOf" );
  }

  free( result );

  return count;
}

static void
adl_free_parts( char **parts, int count )
{
  if ( parts == NULL || *parts == NULL ) return;
  while ( count > 0 ) free( parts[--count] );
  free( parts );
}

static void
adl_test_unify( char *sol1, char *sol2, char **answers )
{
  char *unified = NULL;

  adl_debug( "Test unify: %s and %s", sol1, sol2 );

  if ( unify( sol1, sol2, &unified ) ) {

    adl_debug( "Unify succeeded!" );

    if ( unified ) {
      fprintf( stderr, "Adding solution: %s\n", unified );
      *answers = tvuStringCat( *answers, unified );
      *answers = tvuStringCat( *answers, "," );
      OAA_FREE( unified );
    }
  } else {
    adl_debug( "Unify failed!" );
  }
}

/* OAA callback for implementing the "place_to_latlong" solvable */
int
tvuADLCallback( char *func, char *args, char **answers )
{
  char const     *funcName = "tvuADLCallback";
  char           *place_name, *place_type, *orig_term;
  char           *north = NULL, *south = NULL, *east = NULL, *west = NULL;
  char           *partof = NULL;
  tvuADLResponse *response;
  int            i, length = list_len( args ), len, solvable, result = 0;
  double         box[4], *area = NULL;

  char           *orig_in_place = 0, *orig_in_type = 0, *orig_in_partof = 0;
  char           *orig_out_place = 0, *orig_out_type = 0, *orig_out_partof = 0;
  char           *orig_north = 0, *orig_east = 0, *orig_south=0, *orig_west=0;

  /* Setup a default answer: predicate succeeds with no variables returned */

  *answers = (char *) malloc( strlen(func) + strlen(args) + 10 );
  if ( *args )
    sprintf( *answers, "[%s(%s)]", func, args );
  else
    sprintf( *answers, "[%s]", func );

  /* extract the original term from this (no square brackets) */

  orig_term = strdup( (*answers) + 1 );
  if ( ! orig_term ) return 0;
  orig_term[strlen(orig_term)-1] = '\0';

  adl_debug( "Received query: %s", orig_term );

  /* work out which solvable we have */

  if ( strcmp( func, "place_to_latlong" ) == 0 )
    solvable = PLACE_TO_LATLONG;
  else if ( strcmp( func, "place_to_latlong_bounds" ) == 0 )
    solvable = PLACE_TO_LATLONG_BOUNDS;
  else if ( strcmp( func, "place_to_latlong_partof" ) == 0 )
    solvable = PLACE_TO_LATLONG_PARTOF;
  else if ( strcmp( func, "place_to_latlong_partof_bounds" ) == 0 )
    solvable = PLACE_TO_LATLONG_PARTOF_BOUNDS;
  else {
    TSMTRACE(( OAA_MASK, funcName, "Unknown functor: %s", func ));
    fprintf( stderr, "Unknown functor (%s). Returning %s\n", func, *answers );
    return 0;
  }

  /* check that we have the right number of arguments */

  if ( ( solvable == PLACE_TO_LATLONG && length != 8 ) ||
       ( solvable == PLACE_TO_LATLONG_BOUNDS && length != 12 ) ||
       ( solvable == PLACE_TO_LATLONG_PARTOF && length != 10 ) ||
       ( solvable == PLACE_TO_LATLONG_PARTOF_BOUNDS && length != 14 ) ) {
    TSMTRACE(( OAA_MASK, funcName, "Illegal number of args: %d", length ));
    fprintf( stderr, "Illegal args: returning %s\n", *answers );
    return 0;
  }

  /* Get the name of the function to be performed and its arguments */

  NthElt( args, 1, &place_name );
  NthElt( args, 2, &place_type );

  orig_in_place = strdup( place_name );
  orig_in_type  = strdup( place_type );

  /* check that we have a non-variable for the Place name */

  if ( is_var( place_name ) && is_var( place_type ) ) {
    TSMTRACE(( OAA_MASK, funcName, "Place name and type cannot both"
	       "be variables!" ));
    fprintf( stderr, "Need a place name or a type: returning %s\n", *answers );
    return 0;
  }

  /* convert a variable instance in the type field to mean "any" type */

  if ( is_var( place_name ) ) {
    OAA_FREE( place_name );
    place_name = strdup( "" );
  } else
    remove_quotes( place_name );

  if ( is_var( place_type ) ) {
    OAA_FREE( place_type );
    place_type = strdup( "" );
  } else
    remove_quotes( place_type );

  /* parse out the rest of the parameters */

  if ( solvable == PLACE_TO_LATLONG_BOUNDS ) {

    /* extract the north, south, east, and west coordinates */

    NthElt( args, 3, &north );
    orig_north = strdup( north );
    remove_quotes( north );
    box[0] = atof( north );

    NthElt( args, 4, &south );
    orig_south = strdup( south );
    remove_quotes( south );
    box[1] = atof( south );

    NthElt( args, 5, &east );
    orig_east = strdup( east );
    remove_quotes( east );
    box[2] = atof( east );

    NthElt( args, 6, &west );
    orig_west = strdup( west );
    remove_quotes( west );
    box[3] = atof( west );

    NthElt( args, 7, &orig_out_place );
    NthElt( args, 8, &orig_out_type );

    area = box;

    partof = NULL;

  } else if ( solvable == PLACE_TO_LATLONG_PARTOF_BOUNDS ) {

    /* extract the north, south, east, and west coordinates and partof */

    NthElt( args, 4, &north );
    orig_north = strdup( north );
    remove_quotes( north );
    box[0] = atof( north );

    NthElt( args, 5, &south );
    orig_south = strdup( south );
    remove_quotes( south );
    box[1] = atof( south );

    NthElt( args, 6, &east );
    orig_east = strdup( east );
    remove_quotes( east );
    box[2] = atof( east );

    NthElt( args, 7, &west );
    orig_west = strdup( west );
    remove_quotes( west );
    box[3] = atof( west );

    NthElt( args, 3, &partof );
    orig_in_partof = strdup( partof );
    if ( is_var( partof ) ) {
      OAA_FREE( partof );
      partof = strdup( "" );
    } else
      remove_quotes( partof );

    NthElt( args, 8, &orig_out_place );
    NthElt( args, 9, &orig_out_type );
    NthElt( args, 10, &orig_out_partof );

    area = box;

  } else if ( solvable == PLACE_TO_LATLONG_PARTOF ) {

    /* extract the partof information (could be a variable) */

    NthElt( args, 3, &partof );
    orig_in_partof = strdup( partof );
    if ( is_var( partof ) ) {
      OAA_FREE( partof );
      partof = strdup( "" );
    } else
      remove_quotes( partof );

    NthElt( args, 4, &orig_out_place );
    NthElt( args, 5, &orig_out_type );
    NthElt( args, 6, &orig_out_partof );

    north = south = east = west = NULL;

  } else if ( solvable == PLACE_TO_LATLONG ) {

    NthElt( args, 3, &orig_out_place );
    NthElt( args, 4, &orig_out_type );

    north = south = east = west = NULL;
  }


  TSMTRACE(( OAA_MASK, funcName, "OAA: %s, %s", place_name, place_type ));

  /* sent the gazetteer query to the ADL Middleware */

  response = tvuADLQuery( place_name, place_type, area, -1 );

  if ( response == NULL ) {
    TSMTRACE(( OAA_MASK, funcName, "Couldn't find a match" ));
    adl_debug( "No match found: returning %s", *answers );
    goto cleanup;
  }

  free( *answers );  /* discard the default answer */
  *answers = NULL;
  *answers = tvuStringCat( *answers, "[" );

  for ( i = 0; i < response->matches; i++ ) {
    char *in_place, *out_place, *in_type, *out_type;
    char buffer[1024], this_title[1024], this_type[1024];

    /* create single-quoted versions of the title and type */

    sprintf( this_title, "'%s'", response->title[i] );
    sprintf( this_type, "'%s'", response->type[i] );

    /* do any appropriate instantiating of place name or type variables */

    /* in_place  = is_var( orig_in_place ) ? this_title : orig_in_place; */
    in_place  = orig_in_place;
    out_place = this_title;

    /* in_type   = is_var( orig_in_type ) ? this_type : orig_in_type; */
    in_type   = orig_in_type;
    out_type  = this_type;

    if ( solvable == PLACE_TO_LATLONG_PARTOF ||
	 solvable == PLACE_TO_LATLONG_PARTOF_BOUNDS ) {
      char **part_list;
      int  j, parts = adl_get_parts( response->id[i], &part_list );

      /* create single-quoted versions of the title and type */


      if ( parts == 0 ) continue;

      for ( j = 0; j < parts; j++ ) {
	char *in_part, *out_part;
	char this_part[1024];

	if ( ! tvuSubStr( part_list[j], partof ) ) {
	  adl_debug( "Parts do not match: %s != %s", part_list[j],
		     orig_in_partof );
	  continue;
	}

	sprintf( this_part, "'%s'", part_list[j] );

	/* in_part  = is_var( orig_in_partof ) ? this_part : orig_in_partof; */
	in_part  = orig_in_partof;
	out_part = this_part;
	
	/* create the solution string */

	if ( solvable == PLACE_TO_LATLONG_PARTOF ) 
	  sprintf( buffer, "%s(%s,%s,%s,%s,%s,%s,%1.10lg,%1.10lg,%1.10lg,"
		   "%1.10lg)", func, in_place, in_type, in_part,
		   out_place, out_type, out_part,
		   response->north[i], response->south[i],
		   response->east[i], response->west[i] );
	else if ( solvable == PLACE_TO_LATLONG_PARTOF_BOUNDS ) 
	  sprintf( buffer, "%s(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,"
		   "%1.10lg,%1.10lg,%1.10lg,%1.10lg)", func, in_place, in_type,
		   in_part, orig_north, orig_south, orig_east, orig_west, 
		   out_place, out_type, out_part,
		   response->north[i], response->south[i],
		   response->east[i], response->west[i] );

	/* check that this solution unifies with the initial term */
	/* if it does unify, the solution is added to our answers */

	adl_test_unify( buffer, orig_term, answers );

      }

      adl_free_parts( part_list, parts );
      continue;
    }

    /* create the solution string */

    if ( solvable == PLACE_TO_LATLONG ) 
      sprintf( buffer, "%s(%s,%s,%s,%s,%1.10lg,%1.10lg,%1.10lg,"
	       "%1.10lg)", func, in_place, in_type, out_place, out_type,
	       response->north[i], response->south[i],
	       response->east[i], response->west[i] );
    else if ( solvable == PLACE_TO_LATLONG_BOUNDS ) 
      sprintf( buffer, "%s(%s,%s,%s,%s,%s,%s,%s,%s,"
	       "%1.10lg,%1.10lg,%1.10lg,%1.10lg)", func, in_place, in_type,
	       orig_north, orig_south, orig_east, orig_west, 
	       out_place, out_type,
	       response->north[i], response->south[i],
	       response->east[i], response->west[i] );

    /* check that this solution unifies with the initial term */
    /* if it does unify, the solution is added to our answers */

    adl_test_unify( buffer, orig_term, answers );

  }

  len = strlen( *answers );
  if ( len > 0 && (*answers)[len-1] == ',' )
    (*answers)[len-1] = '\0';

  *answers = tvuStringCat( *answers, "]" );

  tvuADLFreeResponse( response );
  result = 1;

 cleanup:

  TSMTRACE(( OAA_MASK, funcName, "OAA: Returning answer %s", *answers ));

  adl_debug( "Answer found: returning %s", *answers );

  OAA_FREE( orig_in_place );
  OAA_FREE( orig_in_type );
  OAA_FREE( orig_in_partof );
  OAA_FREE( orig_out_place );
  OAA_FREE( orig_out_type );
  OAA_FREE( orig_out_partof );
  OAA_FREE( orig_north );
  OAA_FREE( orig_south );
  OAA_FREE( orig_east );
  OAA_FREE( orig_west );
  OAA_FREE( orig_term );
  OAA_FREE( place_name );
  OAA_FREE( place_type );
  OAA_FREE( partof );
  OAA_FREE( north );
  OAA_FREE( south );
  OAA_FREE( east );
  OAA_FREE( west );
  return result;
}

