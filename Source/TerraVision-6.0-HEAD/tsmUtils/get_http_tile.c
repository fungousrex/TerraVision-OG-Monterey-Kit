/*
 * Name: get_http_tile.c
 *
 * Description:
 *     This is a CGI program that reads a single tile from a localdisk
 *     dataset and returns it as raw data or as a JFIF/JPEG file. This
 *     program should be installed in the cgi-bin directory of the http
 *     server to enable external clients to read datasets via http.
 *
 *     See the function InvalidArgs() for the CGI URL syntax.
 *
 *     This program requires the libcgi.a library.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 7 December 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: get_http_tile.c,v 1.5 2000/08/28 23:56:01 reddy Exp $
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
#include <cgi.h>
#include <tsm/tsm.h>

/* N.B. pyramid_dir may need to be customised for a local system */

static char *pyramid_dir = "/homedir/magic/TileSets/Pyramids";
/* static char *pyramid_dir = "/home/g2/users/reddy/public_html/TileSets/Pyramids"; */

static char *pyramid_name = NULL;
static char *tile_x = NULL;
static char *tile_y = NULL;
static char *tile_level = NULL;
static char *tile_test = NULL;
static char *tile_info = NULL;

static TsmConnectionParams params;

void
InvalidArgs( form_entry *p )
{
  print_mimeheader("text/html");
  
  puts( "<HTML><BODY><PRE>" );
  puts( "Program: get_http_tile" );
  printf( "Version: %s\n", tsmCvsToString( "$Revision: 1.5 $" ) );
  puts( "Purpose: reads a single tile from a TIFF file via http" );
  puts( "Usage: /cgi-bin/get_http_tile?" );
  puts( "          pyramid=[pyramid-name]    % name of pyramid, e.g. co.dem" );
  puts( "          x=[x]                     % x coordinate for tile" );
  puts( "          y=[y]                     % y coordinate for tile" );
  puts( "          level=[level]             % level number for tile" );
  puts( "          dir=[dir]                 % localdisk dir for Pyramids" );
  puts( "          lrbt=yes                  % use raw raster tiles" );
  puts( "          jpeg=yes                  % use jpeg compressed tiles" );
  puts( "          lzw=yes                   % use lzw compressed tiles" );
  puts( "          tiffs=yes                 % use TIFF format imagery" );
  puts( "          rawtiles=yes              % use raw .tile format imagery" );
  puts( "          test=yes                  % display tile in browser" );
  puts( "          info=yes                  % output tile info, no data" );
  printf( "Localdisk Pyramid dir: %s\n", pyramid_dir );
  puts( "Your args were:" );

  if ( p ) {
    for( ; p; p = p->next )
      printf( "  %s = %s\n", p->name, p->val );
  } else
    puts( "  None" );
  
  puts( "</PRE></BODY><HTML>" );
  exit( 0 );
}

void
GetTileError( char *message )
{
  print_mimeheader("text/html");
  
  printf( "<HTML><BODY><PRE>\n%s\n</PRE></BODY><HTML>\n", message );
  exit( 0 );
}

void
ParseArgs( void )
{
  int i;
  cgi_info ci;
  form_entry *parms, *p;

  /* Read the CGI parameters */
  
  get_cgi_info (&ci);
  if ( ( parms = get_form_entries(&ci) ) == NULL )
    InvalidArgs( parms );

  /* Parse out the params we require */

  for( p=parms; p; p = p->next ) {

    /* Required settings */

    if ( ! strcasecmp( p->name, "pyramid" ) )
      pyramid_name = p->val;
    else if ( ! strcasecmp( p->name, "x" ) )
      tile_x = p->val;
    else if ( ! strcasecmp( p->name, "y" ) )
      tile_y = p->val;
    else if ( ! strcasecmp( p->name, "level" ) )
      tile_level = p->val;

    /* Optional settings */

    else if ( ! strcasecmp( p->name, "test" ) )
      tile_test = p->val;
    else if ( ! strcasecmp( p->name, "info" ) )
      tile_info = p->val;
    else if ( ! strcasecmp( p->name, "dir" ) )
      pyramid_dir = p->val;
    else if ( ! strcasecmp( p->name, "lrbt" ) )
      tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LRBT_FORMAT );
    else if ( ! strcasecmp( p->name, "jpeg" ) )
      tsmSetParam( params, TSM_PARAM_FORMAT, TSM_JPEG_FORMAT );
    else if ( ! strcasecmp( p->name, "lzw" ) )
      tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LZW_FORMAT );
    else if ( ! strcasecmp( p->name, "tiffs" ) )
      tsmSetParam( params, TSM_PARAM_TIFF, TRUE );
    else if ( ! strcasecmp( p->name, "rawtiles" ) )
      tsmSetParam( params, TSM_PARAM_TIFF, FALSE );
  }

  /* Check that we have all required params */

  if ( pyramid_name == NULL || tile_x == NULL || tile_y == NULL ||
       tile_level == NULL )
    InvalidArgs( parms );
}

#include <sys/time.h>
#include <time.h>

void
OutputTimestamp( char *comment )
{
#if 0
  struct timeval tp;
  time_t clock;
  struct tm *localtm;
  char   *timestr = "";

  clock = time(NULL);
  if ( ( localtm = localtime( &clock ) ) != NULL ) {
    timestr = asctime( localtm );
    timestr[24] = '\0';
  }

  fprintf( stderr, "get_http_tile: %s - %s\n", timestr, comment );
#else
  struct timeval tp;
  double this_clock;
  static double last_clock = 0, first_clock = 0;
  char   timestr[128];

  gettimeofday( &tp, NULL );
  this_clock = (double) tp.tv_sec + ( tp.tv_usec / 1e6 );

  if ( last_clock == 0 ) {
    strcpy( timestr, "T=0 sec" );
    first_clock = this_clock;
  } else
    sprintf( timestr, "T+%1.3lf: %1.3lf sec later", this_clock - first_clock,
	     this_clock - last_clock );

  last_clock = this_clock;

#endif
  
  fprintf( stderr, "get_http_tile: %s - %s\n", timestr, comment );
}

#define JFIF_MAGIC_SIZE  20  /* the byte size of the JFIF file header */

void
WriteTileAsJFIF( TsmConnection connect, uchar *tileBuffer )
{
  uchar *jpegBuffer = tsmAllocTileBuffer( connect );
  Pyramid *tspec = (Pyramid *) tsmGet( connect, TSM_DATASET_TSPEC );
  static const int  offset = JFIF_MAGIC_SIZE - 2; /* +header, -M_SOI marker */
  int length;
    
  /* store the JFIF header section in a static array for quick access */

  static unsigned char jfif_header[JFIF_MAGIC_SIZE] = {
    0xff, 0xD8,                  /* M_SOI marker */
    0xFF, 0xE0, 0x00, 0x10,      /* M_APP0 marker and length */
    'J', 'F', 'I', 'F', '\0',    /* JFIF identifier */
    0x01, 0x02,                  /* JFIF version = 1.02 */
    0x00,                        /* units = none */
    0x00, 0x01, 0x00, 0x01,      /* x and y densities = 1 */
    0x00, 0x00                   /* no thumbnail image */
  };

  /* Compress the tile into the jpeg buffer (leaving room for header) */

  length = tsmJPEGCompressTile( tileBuffer, jpegBuffer + offset,
				&tspec->tileTspec, 75 );

  if ( length == 0 ) {
    tsmFreeTileBuffer( jpegBuffer );
    tsmDisconnect( connect );
    GetTileError( "Couldn't JPEG compress the given tile" );
  }

  /* Write the JFIF APP header to the start of the jpeg buffer */

  memcpy( jpegBuffer, jfif_header, JFIF_MAGIC_SIZE );
    
  print_mimeheader("image/jpeg");
  fwrite( jpegBuffer, 1, length + offset, stdout );

  tsmFreeTileBuffer( jpegBuffer );
}

int
main( int argc, char **argv )
{
  TsmConnection       connect;
  char                filename[512], msg[512];
  uchar               *tileBuffer;
  int                 tx, ty, level;

  OutputTimestamp( "begin request" );

  /* Parse all of the CGI parameters */
  
  tsmInitParams( &params );
  ParseArgs();

  tx    = atoi( tile_x );
  ty    = atoi( tile_y );
  level = atoi( tile_level );

  /* Open a connection to the pyramid dataset */

  sprintf( filename, "%s/%s", pyramid_dir, pyramid_name );

  OutputTimestamp( "connect to dataset" );

  if ( ( connect = tsmConnect( filename, "r", params ) ) == NULL ) {
    tsmFreeParams( params );
    sprintf( msg, "Could not connect to pyramid: %s", filename );
    GetTileError( msg );
  }
  tsmFreeParams( params );

  /* Now get the requested tile */

  OutputTimestamp( "read the tile" );

  if ( tsmAllocReadTile( connect, tx, ty, level, &tileBuffer ) == FALSE ) {
    tsmDisconnect( connect );
    sprintf( msg, "Could not read tile: %d, %d, %d", tx, ty, level );
    GetTileError( msg );
  } 

  OutputTimestamp( "serve the tile" );

  /* Now output the tile as the result of this cgi request */

  if ( tile_test != NULL ) 
    WriteTileAsJFIF( connect, tileBuffer );
  else if ( tile_info != NULL ) {
    print_mimeheader("text/html");
    puts( "<HTML><BODY><PRE>" );
    tsmDescribeConnection( connect );
    puts( "</PRE></BODY><HTML>" );
  } else {
    print_mimeheader("text/html");
    fwrite( tileBuffer, 1, tsmGet( connect, TSM_TILE_SIZE ), stdout );
  }

  /* All done now */

  OutputTimestamp( "end request" );

  tsmFreeTileBuffer( tileBuffer );
  tsmDisconnect( connect );

  OutputTimestamp( "done" );

  return 0;
}
