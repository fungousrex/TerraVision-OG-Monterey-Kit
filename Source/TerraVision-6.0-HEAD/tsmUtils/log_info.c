/*
 * Name: log_info.c
 *
 * Description:
 *     Reads a NetLogger file as output by the TerraVision system.
 *     It then performs a number of analyses on the data and outputs
 *     its results to the shell.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 9 June 1999.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: log_info.c,v 1.5.6.2 2001/05/11 23:41:31 reddy Exp $
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
#include <pthr/list.h>

#define APP_VERSION     tsmCvsToString( "$Revision: 1.5.6.2 $" )
#define APP_SENT_STR    "APP_SENT"
#define MASTER_IN_STR   "DPSS_MASTER_IN"
#define MASTER_OUT_STR  "DPSS_MASTER_OUT"
#define SERV_IN_STR     "DPSS_SERV_IN"
#define START_READ_STR  "DPSS_START_READ"
#define END_READ_STR    "DPSS_END_READ"
#define START_WRITE_STR "DPSS_START_WRITE"
#define APP_RECEIVE_STR "APP_RECEIVE"
#define TV_FRAME        "TV_FRAME"

typedef enum {
  UNKNOWN_EVENT,
  /* TerraVision NetLogger events */
  TILE_REQ, TILE_READ, TILE_WRITE, TILE_IN_CACHE, TILE_OUT_CACHE,
  TILE_IN_TEXCACHE, TILE_OUT_TEXCACHE, REQUEST_SENT, FRAME,
  /* DPSS NetLogger events */
  DPSS_APP_SENT, DPSS_MASTER_IN, DPSS_MASTER_OUT, DPSS_SERV_IN,
  DPSS_START_READ, DPSS_END_READ, DPSS_START_WRITE, DPSS_APP_RECEIVE
} LogType;

typedef struct {
  int     tx, ty, level;
  LogType type;
  double  stamp;
  char    *options;
  double  rate, quality;
} LogEvent;

list_t *log_list;
int    tile_req = 0, tile_read = 0, tile_write = 0;
int    tile_in_cache = 0, tile_out_cache = 0, request_sent = 0;
int    tile_in_texcache = 0, tile_out_texcache = 0, frame = 0;
int    app_sent = 0, master_in = 0, master_out = 0, serv_in = 0;
int    start_read = 0, end_read = 0, start_write = 0, app_receive = 0;
double log_time = 0.0, throughput = 0.0;
double avg_prev_read = 0.0;
int    tile_size = 128 * 128 * 3, total_bytes = 0;

double start_time = -1.0f, stop_time = -1.0f;

static double
elapsedTime (double start, double end)
{
  return end - start;
}

/*
 * Synopsis:
 *   LogEvent *ParseEvent( char const *buffer )
 *
 * Description:
 *   Takes a single NetLogger event string and parses it into a 
 *   LogEvent structure. We translate the event type string into
 *   an enumerated type for faster comparisons later. We extract
 *   the timestamp into a struct timeval structure, and we
 *   extract the tile (tx,ty,level) information if it is supplied.
 *
 * Returns: A valid LogEvent, or NULL if failed.
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

LogEvent *
ParseEvent( char const *buffer )
{
  char          *frag, *match;
  LogEvent      *event;
  static double first_stamp = -1.0;
  static int    warning = FALSE;
  static int    line = 0;

  /* create a new LogEvent that we will fill in */
  
  if ( ( event = calloc( sizeof(LogEvent), 1 ) ) == NULL )
    return NULL;

  line++;

  /* work out the event type */

  match = "NL.EVNT=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    frag += strlen( match );

    /* look for the TerraVision events */
    
    if ( ! strncmp( frag, TV_TILE_REQ, strlen(TV_TILE_REQ) ) )
      event->type = TILE_REQ;
    else if ( ! strncmp( frag, TV_TILE_READ, strlen(TV_TILE_READ) ) )
      event->type = TILE_READ;
    else if ( ! strncmp( frag, TV_TILE_WRITE, strlen(TV_TILE_WRITE) ) )
      event->type = TILE_WRITE;
    else if ( ! strncmp( frag, TV_TILE_IN_CACHE, strlen(TV_TILE_IN_CACHE) ) )
      event->type = TILE_IN_CACHE;
    else if ( ! strncmp( frag, TV_TILE_OUT_CACHE, strlen(TV_TILE_OUT_CACHE) ) )
      event->type = TILE_OUT_CACHE;
    else if (!strncmp( frag, TV_TILE_IN_TEXCACHE, strlen(TV_TILE_IN_TEXCACHE)))
      event->type = TILE_IN_TEXCACHE;
    else if (!strncmp(frag,TV_TILE_OUT_TEXCACHE, strlen(TV_TILE_OUT_TEXCACHE)))
      event->type = TILE_OUT_TEXCACHE;
    else if ( ! strncmp( frag, TV_REQUEST_SENT, strlen(TV_REQUEST_SENT) ) )
      event->type = REQUEST_SENT;
    else if ( ! strncmp( frag, TV_FRAME, strlen(TV_FRAME) ) )
      event->type = FRAME;

    /* look for the DPSS events */

    else if ( ! strncmp( frag, APP_SENT_STR, strlen(APP_SENT_STR) ) )
      event->type = DPSS_APP_SENT;
    else if ( ! strncmp( frag, MASTER_IN_STR, strlen(MASTER_IN_STR) ) )
      event->type = DPSS_MASTER_IN;
    else if ( ! strncmp( frag, MASTER_OUT_STR, strlen(MASTER_OUT_STR) ) )
      event->type = DPSS_MASTER_OUT;
    else if ( ! strncmp( frag, SERV_IN_STR, strlen(SERV_IN_STR) ) )
      event->type = DPSS_SERV_IN;
    else if ( ! strncmp( frag, START_READ_STR, strlen(START_READ_STR) ) )
      event->type = DPSS_START_READ;
    else if ( ! strncmp( frag, END_READ_STR, strlen(END_READ_STR) ) )
      event->type = DPSS_END_READ;
    else if ( ! strncmp( frag, START_WRITE_STR, strlen(START_WRITE_STR) ) )
      event->type = DPSS_START_WRITE;
    else if ( ! strncmp( frag, APP_RECEIVE_STR, strlen(APP_RECEIVE_STR) ) )
      event->type = DPSS_APP_RECEIVE;

    /* otherwise, warn the user that the file contains an unknown event */

    else if ( ! warning ) {
      fprintf( stderr, "Unknown event type found on line %d\n", line );
      warning = TRUE;
    }
  }

  /* work out the frame rate for a TV_FRAME event */

  match = "RATE=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    frag += strlen( match );
    sscanf( frag, "%lg", &event->rate );
  }

  /* work out the image quality for a TV_FRAME event */

  match = "QUALITY=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    frag += strlen( match );
    sscanf( frag, "%lg", &event->quality );
    if ( event->quality > 1.0 ) {
      fprintf( stderr, "Illegal image quality (%1.2f) on line %d, using 1.0\n",
	       event->quality, line );
      event->quality = 1.0;
    }
  }

  /* work out the tile ID, if defined */

  match = "DPSS.BID=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    frag += strlen( match );
    sscanf( frag, "%d.%d.%d", &event->tx, &event->ty, &event->level );
  }

  /* work out the event's timestamp */

  event->stamp = 0.0;

  match = "NL.USEC=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    double lo;
    frag += strlen( match );
    sscanf( frag, "%lf", &lo );
    event->stamp += lo / 1e6;
  }

  match = "NL.SEC=";
  if ( ( frag = strstr( buffer, match ) ) != NULL ) {
    double hi;
    frag += strlen( match );
    sscanf( frag, "%lf", &hi );
    event->stamp += hi;
  }

  /* make timestamps relative to 0.0 */

  if ( first_stamp == -1.0 ) first_stamp = event->stamp;
  event->stamp -= first_stamp;

  /* cull out any timestamps not in the selected range */

  if ( start_time >= 0.0 && event->stamp < start_time ) return NULL;
  if ( stop_time >= 0.0 && event->stamp > stop_time ) return NULL;

  return event;
}

/*
 * Synopsis:
 *   void LoadLogFile( char *filename )
 *
 * Description:
 *   Loads the specified NetLogger file, parses all of the events into
 *   a list. The filename can also be a file://localhost/ or http:
 *   URL, so you can analyse logs that are sitting on an http server.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

void
LoadLogFile( char *filename )
{
  FILE     *handle;
  char     buffer[512];
  LogEvent *event;

  /* open the NetLogger file for input */
  
  if ( ( handle = tsmOpenStream( filename, "r" ) ) == NULL ) {
    fprintf( stderr, "Cannot open NetLogger file: %s\n", filename );
    exit( 1 );
  }

  fprintf( stderr, "Reading file... " ); fflush( stdout );

  /* create a new list to store all of the NetLogger events */

  log_list = list_create( FALSE );

  /* go through each event in the file, parse it, and add it to our list */

  while ( !feof( handle ) ) {

    buffer[0] = '\0';
    fgets( buffer, 512, handle );
    if ( feof( handle ) ) break;

    if ( ( event = ParseEvent( buffer ) ) == NULL ) 
      continue;

    list_push( log_list, event );

  }

  /* all done so close our log file - we have it all in memory now */

  tsmCloseStream( handle );

  /* check that we do have some information!! */

  fprintf( stderr, "done.\n" );

  if ( list_count( log_list ) == 0 ) {
    fprintf( stderr, "No NetLogger information in file!\n" );
    exit( 1 );
  }
}

/*
 * Synopsis:
 *   void CountEvents( void )
 *
 * Description:
 *   Counts all of the different NetLogger events that we support
 *   and outputs these counts to the terminal. We store the counts
 *   in global variables so that other routines have access to these
 *   values once they have been evaluated.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

void
CountEvents( void )
{
  list_iter_t i;

  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    LogEvent *event = list_value( i );

    switch ( event->type ) {
    case TILE_REQ:
      tile_req++;
      break;
    case TILE_READ:
      tile_read++;
      break;
    case TILE_WRITE:
      tile_write++;
      break;
    case TILE_IN_CACHE:
      tile_in_cache++;
      break;
    case TILE_OUT_CACHE:
      tile_out_cache++;
      break;
    case TILE_IN_TEXCACHE:
      tile_in_texcache++;
      break;
    case TILE_OUT_TEXCACHE:
      tile_out_texcache++;
      break;
    case REQUEST_SENT:
      request_sent++;
      break;
    case FRAME:
      frame++;
      break;
    case DPSS_APP_SENT:
      app_sent++;
      break;
    case DPSS_MASTER_IN:
      master_in++;
      break;
    case DPSS_MASTER_OUT:
      master_out++;
      break;
    case DPSS_SERV_IN:
      serv_in++;
      break;
    case DPSS_START_READ:
      start_read++;
      break;
    case DPSS_END_READ:
      end_read++;
      break;
    case DPSS_START_WRITE:
      start_write++;
      break;
    case DPSS_APP_RECEIVE:
      app_receive++;
      break;
    }

  }

  printf( "Count of events in log file:\n" );
  printf( "  TV_TILE_REQ          = %d\n", tile_req );
  printf( "  TV_TILE_READ         = %d\n", tile_read );
  printf( "  TV_TILE_WRITE        = %d\n", tile_write );
  printf( "  TV_TILE_IN_CACHE     = %d\n", tile_in_cache );
  printf( "  TV_TILE_OUT_CACHE    = %d\n", tile_out_cache );
  printf( "  TV_TILE_IN_TEXCACHE  = %d\n", tile_in_texcache );
  printf( "  TV_TILE_OUT_TEXCACHE = %d\n", tile_out_texcache );
  printf( "  TV_FRAME             = %d\n", frame );
  if ( app_sent > 0 ) {
    printf( "  APP_SENT             = %d\n", app_sent );
    printf( "  DPSS_MASTER_IN       = %d\n", master_in );
    printf( "  DPSS_MASTER_OUT      = %d\n", master_out );
    printf( "  DPSS_SERV_IN         = %d\n", serv_in );
    printf( "  DPSS_START_READ      = %d\n", start_read );
    printf( "  DPSS_END_READ        = %d\n", end_read );
    printf( "  DPSS_START_WRTE      = %d\n", start_write );
    printf( "  APP_RECEIVE          = %d\n", app_receive );
    printf( "  TV_REQUEST_SENT      = %d\n", request_sent );
  }
  printf( "  Total no. events     = %d\n", list_count( log_list ) );
  printf( "\n" );

}

/*
 * Synopsis:
 *   void TimeEvents( void )
 *
 * Description:
 *   Calculates a number of time-related stats, such as the total
 *   time for the logging (from the first event to the last event),
 *   the throughput into the application, the inter-request delay.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

void
TimeEvents( void )
{
  list_iter_t i;
  double      curr_t, var_t = 0.0, avg_t = 0.0, temp;
  int         numb = 0;
  LogEvent    *start, *end = NULL, *event, *prev = NULL;
  
  i = list_first(log_list);
  start = list_value( i );
  
  /* calculate the average inter-request time */
  for ( ; ! list_done(i); i = list_next(i) ) {
    end   = event;
    event = list_value( i );
    if ( event->type != TILE_REQ ) continue;

    if ( prev != NULL ) {
      curr_t = elapsedTime( prev->stamp, event->stamp );
      avg_t += curr_t;
      numb++;
    }
    prev = event;
  }

  if ( numb == 0 ) return;

  avg_t       = avg_t / numb;
  log_time    = elapsedTime( start->stamp, end->stamp );
  total_bytes = tile_read * tile_size;
  throughput  = ( log_time == 0.0 ) ? 0.0 : total_bytes * 8e-06 / log_time;

  /* calculate the inter-request time variable */
  prev = NULL;
  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    event = list_value( i );
    if ( event->type != TILE_REQ ) continue;
    /* variance = sqrt( sum across n of 1/n * (x_n - mean)**2 ) */
    if ( prev != NULL ) {
      curr_t = elapsedTime( prev->stamp, event->stamp );
      temp = avg_t - curr_t;
      var_t += ( temp * temp ) / numb;
    }
    prev = event;
  }
  var_t = sqrt( var_t );

  printf( "General Timing Information:\n" );
  printf( "  Total log time          = %1.2lf secs", log_time );
  if ( start_time >= 0.0 && stop_time >= 0.0 )
    printf( " (%1.1lf to %1.1lf)\n", start_time, stop_time );
  else if ( start_time >= 0.0 && stop_time < 0.0 )
    printf( " (%1.1lf to end)\n", start_time );
  else if ( start_time < 0.0 && stop_time >= 0.0 )
    printf( " (start to %1.1lf)\n", stop_time );
  else
    printf( "\n" );

  printf( "  Total KBytes read       = %d (%d x %d)\n",
	  total_bytes / 1024, tile_read, tile_size / 1024 );
  printf( "  Average throughput      = %1.2lf MBit/s (%d * 8e-6 / %1.1lf )\n",
	  throughput, total_bytes, log_time );
  printf( "  Avg inter-request time  = %1.2lf secs\n", avg_t );
  printf( "  Inter-req time variance = %1.2lf secs\n", var_t );
  printf( "\n" );

}

/*
 * Synopsis:
 *   void FrameEvents( void )
 *
 * Description:
 *   Calculates a number of application-level per-frame stats
 *   such as frame rate and average image quality.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 9 April 2001
 *
 */

void
FrameEvents( void )
{
  list_iter_t i;
  double      rate = 0.0, quality = 0.0, calc_rate, frame_time;
  double      numb = 0.0, quality_variance = 0.0, temp;
  LogEvent    *start = NULL, *end = NULL;
  
  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    LogEvent *event = list_value( i );
    if ( event->type != FRAME ) continue;

    if ( start == NULL ) start = event;
    end = event;

    rate    += event->rate;
    quality += event->quality;
    numb    += 1.0;
  }

  if ( numb == 0 ) return;

  rate /= numb;
  quality /= numb;
  frame_time = elapsedTime( start->stamp, end->stamp );
  calc_rate = frame / frame_time;

  /* calculate the variance in image quality */
  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    LogEvent *event = list_value( i );
    if ( event->type != FRAME ) continue;
    /* variance = sqrt( sum across n of 1/n * (x_n - mean)**2 ) */
    temp = event->quality - quality;
    quality_variance += ( temp * temp ) / numb;
  }
  quality_variance = sqrt( quality_variance );

  printf( "Per-Frame Information:\n" );
  /* printf( "  Avg Reported Frame Rate = %1.2lf Hz\n", rate ); */
  printf( "  Avg Frame Rate          = %1.2lf Hz (%d frames in %1.2lf secs)\n",
	  calc_rate, frame, frame_time );
  printf( "  Avg Image Quality       = %1.0lf %%\n",
	  quality * 100.0);
  printf( "  Image Quality Variance  = %1.0lf %%\n",
	  quality_variance * 100.0 );
  printf( "\n" );

}

/*
 * Synopsis:
 *   int SameTile( LogEvent *event1, LogEvent *event2 )
 *
 * Description:
 *   Returns TRUE if both events are for the same (tx,ty,level) tile ID.
 *
 * Returns: TRUE or FALSE
 *
 * Author: Martin Reddy
 *
 * Date: 16 June 1999
 *
 */

int
SameTile( LogEvent *event1, LogEvent *event2 )
{
  if ( event1 == NULL || event2 == NULL ) return FALSE;
  return ( event1->tx == event2->tx &&
	   event1->ty == event2->ty &&
	   event1->level == event2->level );
}

/*
 * Synopsis:
 *   void list_remove_tile( list_t *list, LogEvent *event )
 *
 * Description:
 *   Scans through a list for any event with the same (tx,ty,level)
 *   as the specified event. If any such events are found in the
 *   list, they are deleted.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

void
list_remove_tile( list_t *list, LogEvent *event )
{
  list_iter_t i;

  for ( i = list_first(list); ! list_done(i); ) {
    LogEvent *curr = list_value( i );

    if ( SameTile( curr, event ) )
      i = list_delete( i );
    else
      i = list_next( i );

  }
}

/*
 * Synopsis:
 *   void StatEvents( void )
 *
 * Description:
 *   Does some slightly more complex stats in order to find out
 *   things like the percentage of tiles that were requested
 *   and then read in before the next request list.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 11 June 1999
 *
 */

void
StatEvents( void )
{
  list_iter_t i;
  list_t *thisblock = list_create( FALSE ), *lastblock = NULL;
  int    block_req = 0, last_block_req = 0, num = 0;
  float  perc_read, perc_disp;

  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    LogEvent *event = list_value( i );

    switch ( event->type ) {
    case REQUEST_SENT:

      if ( lastblock ) {
	int   reqs_left = list_count( lastblock );
	float reqs_perc = reqs_left * 100.0 / last_block_req;
	float read_perc = 100.0 - reqs_perc;

	avg_prev_read += read_perc;
	num++;

	list_destroy( lastblock );
	lastblock = thisblock;
	thisblock = list_create( FALSE );
      } else {
	lastblock = thisblock;
	thisblock = list_create( FALSE );
      }

      last_block_req = block_req;
      block_req      = 0;
      break;

    case TILE_REQ:
      list_push( thisblock, event );
      block_req++;
      break;

    case TILE_READ:
      if ( lastblock )
	list_remove_tile( lastblock, event );
      break;

    }

  }

  avg_prev_read = ( num == 0 ) ? 0 : avg_prev_read / num;
  perc_read = ( tile_req == 0 ) ? 0.0 : 100.0 * tile_read / tile_req;
  perc_disp = ( tile_req == 0 ) ? 0.0 : 100.0 * tile_in_texcache / tile_req;

  printf( "Tile Statistics:\n" );
  printf( "  %% of requested tiles read      = %1.1f%% (100 * %d / %d)\n",
	  perc_read, tile_read, tile_req );
  printf( "  %% of requested tiles displayed = %1.1f%% (100 * %d / %d)\n",
	  perc_disp, tile_in_texcache, tile_req );
  if ( avg_prev_read > 0.0 ) {
    printf( "  Avg %% of tiles read from prev\n" );
    printf( "  request list                   = %1.1f%%\n", avg_prev_read );
  }
  printf( "  No. tiles read & not displayed = %d (%d - %d)\n",
	  tile_read - tile_in_texcache, tile_read, tile_in_texcache );
  printf( "\n" );
}

/*
 * Synopsis:
 *   void DPSSEvents( void )
 *
 * Description:
 *   Does some analyses on the DPSS NetLogger events (if present).
 *   Currently we work out the avg/min/max triangular delay SRI-->(DPSS
 *   master)--> (DPSS server)-->(SRI host). We calculate this as:
 *     DPSS_MASTER_IN - APP_SENT +
 *     DPSS_SERV_IN - DPSS_MASTER_OUT +
 *     APP_RECEIVE - DPSS_START_WRITE
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 15 June 1999
 *
 */

void
DPSSEvents( void )
{
  list_iter_t i, j;
  LogEvent *evnt_master_in = NULL, *evnt_app_sent = NULL;
  LogEvent *evnt_serv_in = NULL, *evnt_master_out = NULL;
  LogEvent *evnt_app_receive = NULL, *evnt_start_write = NULL;
  double   min_val, max_val, avg_val = 0, val;
  int      first = TRUE, numb = 0;

  if ( app_sent == 0 ) return;

  for ( i = list_first(log_list); ! list_done(i); i = list_next(i) ) {
    LogEvent *event = list_value( i );

    if ( event->type != DPSS_APP_SENT ) continue;

    evnt_app_sent = event;
    evnt_app_receive = NULL;

    /* search for all of the other 5 DPSS events for this tile */

    for ( j = i; ! list_done( j ); j = list_next(j) ) {
      LogEvent *next_event = list_value( j );
      switch ( next_event->type ) {
      case DPSS_MASTER_IN:
	if ( SameTile( evnt_app_sent, next_event ) )
	  evnt_master_in = next_event;
	break;
      case DPSS_MASTER_OUT:
	if ( SameTile( evnt_app_sent, next_event ) )
	  evnt_master_out = next_event;
	break;
      case DPSS_SERV_IN:
	if ( SameTile( evnt_app_sent, next_event ) )
	  evnt_serv_in = next_event;
	break;
      case DPSS_START_WRITE:
	if ( SameTile( evnt_app_sent, next_event ) )
	  evnt_start_write = next_event;
	break;
      case DPSS_APP_RECEIVE:
	if ( SameTile( evnt_app_sent, next_event ) )
	  evnt_app_receive = next_event;
	break;
      }
      
      if ( evnt_app_receive != NULL ) break;
      
    }

    /* now we should have all of the info to make the calculation */
    
    if ( evnt_app_receive == NULL ) continue;
    
    val =
      elapsedTime( evnt_app_sent->stamp, evnt_master_in->stamp ) +
      elapsedTime( evnt_master_out->stamp, evnt_serv_in->stamp ) +
      elapsedTime( evnt_start_write->stamp, evnt_app_receive->stamp );

    /* update the average, min, and max values with the current value */

    avg_val += val;
    if ( first ) {
      min_val = max_val = val;
      first = FALSE;
    } else {
      if ( val < min_val ) min_val = val;
      if ( val > max_val ) max_val = val;
    }
    numb++;
      
  }

  avg_val = ( numb == 0 ) ? 0 : avg_val / numb;

  /* convert seconds into milliseconds */
  avg_val *= 1000.0;
  min_val *= 1000.0;
  max_val *= 1000.0;

  printf( "DPSS Event Statistics:\n" );
  printf( "  Avg DPSS triangular delay  = %1.0f ms\n", avg_val );
  printf( "  Min DPSS triangular delay  = %1.0f ms\n", min_val );
  printf( "  Max DPSS triangular delay  = %1.0f ms\n", max_val );

}

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
 * Date: 11 June 1999
 *
 */

void
usage( void )
{
  printf( "log_info - release %s\n", APP_VERSION );
  printf( "\nusage: log_info <logfile> [...]\n" );
  printf( "\nThis utility reads a NetLogger file as output by the\n" );
  printf( "TerraVision system and performs various analyses on it.\n" );
  printf( "\nParameters:\n" );
  printf( " <logfile> = the filename of the NetLogger file to read\n" );
  printf( " -start <n> = number of seconds into log to begin analyses\n" );
  printf( " -stop <n> = number of seconds into log to stop analyses\n" );
  printf( " -desc <msg> = textual description of log to output in results\n" );
  printf( " -tilesize <n> = byte size of one tile, e.g. 128*128*3=49152\n" );
  printf( "\n" );
  exit( 0 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty. This checks the command line arguments,
 *      opens the desired NetLogger file and then does its stuff....
 *
 * Externals: None
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 11 June 1999
 *
 */

int main( int argc, char **argv )
{
  char *descr = NULL;

  if ( argc < 2 ) usage();

  tsmGetArgFloat( argc, argv, "-start", &start_time );
  tsmGetArgFloat( argc, argv, "-stop", &stop_time );
  tsmGetArgString( argc, argv, "-desc", &descr );
  tsmGetArgInt( argc, argv, "-tilesize", &tile_size );

  LoadLogFile( argv[1] );

  if ( descr )
    printf( "Session Description\n  %s\n\n", descr );

  CountEvents();

  TimeEvents();

  FrameEvents();

  StatEvents(); 

  DPSSEvents();

  return 0;
}

/*** EOF: log_info.c ***/
