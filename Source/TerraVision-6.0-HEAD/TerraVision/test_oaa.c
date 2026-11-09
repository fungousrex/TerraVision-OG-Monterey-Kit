/* -*- tab-width: 2; -*-

  Revision History:
      $Id: test_oaa.c,v 2.3.10.4 2002/10/31 05:26:44 reddy Exp $

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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <tsm/tsm.h>
#include "OAA.h"
#include "agentlib.h"

#define MAX_CMD_SIZE  2048
#define APP_VERSION tsmCvsToString( "$Revision: 2.3.10.4 $" )

/*
 * Synopsis:
 *    void usage( void );
 *
 * Description:
 *    Display the program usage information to the terminal.
 *    This is called if insufficient parameters are given.
 *
 * Author: Martin Reddy
 * 
 * Date: 3 February 2000
 *
 */

void
usage( void )
{
  printf( "test_oaa release %s\n", APP_VERSION );
  puts( "\nUsage: test_oaa [...]" );

  puts( "\nCommand line interface for controlling remote TerraVision" );
  puts( "sessions via an OAA facilitator" );

  puts( "\nOptional Parameters:" );
  puts( " -oaa <host>:<port> = specify the host:port of the facilitator" );
  puts( " -solve <expr> = send this single command and exit" );
  puts( " -help = display this help text " );

  PR_ProcessExit( 1 );
}

static int
OAAEvent( char *KS, char *func, char *args, char **answers )
{
  const char *funcName = "OAAEvent";
  char *function, *func_name, *func_args, *opt_args;
  int success = TRUE;

  /* Currently we only support one event: terrain3d */

  if ( func == NULL || args == NULL ) {
    printf( "OAA: NULL\n" );
    *answers = (char *) malloc( 3 );
    strcpy( *answers, "[]" );
    return FALSE;
  }

  printf( "OAA: %s(%s)\n", func, args );

  /* Setup a default answer: predicate succeeds with no variables returned */

  *answers = (char *) malloc( strlen(func) + strlen(args) + 10 );
  if ( *args )
    sprintf( *answers, "[%s(%s)]", func, args );
  else
    sprintf( *answers, "[%s]", func );

  return success;
}

/*
 * Synopsis:
 *   void OAALoop( void *ignore )
 *
 * Description:
 *   Our OAA event loop - this is run in a separate thread
 *
 * Author: Martin Reddy
 * 
 * Date: 3 February 2000
 *
 */

void
OAALoop(void *ignore)
{
  while (TRUE)   /* Listen in on the socket and dispatch when input ready */
    if (! oaa_process_all_events())
      break;
}

/*
 * Synopsis:
 *   void ConnectToFacilitator( char *oaa_host, int oaa_port )
 *
 * Description:
 *   Connects to the facilitator at the specified hostname and port.
 *
 * Author: Martin Reddy
 * 
 * Date: 3 February 2000
 *
 */

void
ConnectToFacilitator( char *oaa_host, int oaa_port )
{
  char *oaa_name = "TerraVision Console";
  char *solves = "[]";
  static PRThread *oaa_thread;               /* the OAA event loop thread */
  
  oaa_register( OAA_DO_EVENT, OAAEvent );

  if ( ! setup_communication( oaa_name, oaa_host, oaa_port, solves, 1 ) ) {
    fprintf( stderr, "Couldn't connect to OAA facilitator on %s:%d\n",
	      oaa_host, oaa_port );
    PR_ProcessExit( 1 );
  }

  fprintf( stderr, "Facilitator: %s:%d. ", oaa_host, oaa_port );
  fflush( stderr );

  oaa_ready( 1 );

  if ( pthr_create ( &oaa_thread, "OAA", OAALoop, NULL) ) {
    fprintf( stderr, "Unable to spawn the OAA thread\n" );
    PR_ProcessExit( 1 );
  }
}

void
GetAgents( void )
{
  char *answers = NULL;

  read_bb( "ksdata", "X", &answers );
  if ( answers ) printf( "ksdata = %s\n", answers );
}

/*
 * Synopsis:
 *   void SolveExpression( char *expression )
 *
 * Description:
 *   Sends the given expression to the OAA facilitator to be solved
 *   for any TerraVision clients that are out there (terrain3d()).
 *
 * Author: Martin Reddy
 * 
 * Date: 3 February 2000
 *
 */

void
SolveExpression( char *expression )
{
  char *content = NULL, *ptr, *answer = NULL;

  append( &content, "terrain3d(" );
  append( &content, expression );

  /* append empty optional argument if none specified */

  if ( ( ptr = strrchr( expression, ')' ) ) == NULL )
    ptr = expression;
  if ( strchr( ptr, ',' ) == NULL )
    append( &content, ",[]" );

  append( &content, ")" );
  
  printf( "SOLVE: %s\n", content );
  
  solve( content, "[]", &answer );

  printf( "RESULT: %s\n", answer );
  
  free( content );
  if ( answer ) free( answer );
}

/*
 * Synopsis:
 *   int main( int argc, char **argv )
 *
 * Description:
 *   The main part of the program: checks the command line arguments,
 *   reads commands from the console, and sends them to the OAA
 *   facilitator to control a remote TerraVision.
 *
 * Author: Martin Reddy
 * 
 * Date: 3 February 2000
 *
 */

int
main( int argc, char **argv )
{
  char command[MAX_CMD_SIZE];
  char *hostport = NULL, *sep;
  char *oaa_host = "lomond.ai.sri.com";
  char *expression  = NULL;
  int  oaa_port  = 3344;

  /* parse the command line arguments */

  if ( tsmGetArgString( argc, argv, "-oaa", &hostport ) ) {
    oaa_host = tsmMemStrdup( hostport );
    if ( ( sep = strchr( oaa_host, ':' ) ) != NULL ) {
      *sep = '\0';
      oaa_port = atoi( ++sep );
      if ( oaa_port == 0 ) oaa_port = -1;
    }
  }
  if ( tsmGetArg( argc, argv, "-help" ) ) usage();
  tsmGetArgString( argc, argv, "-solve", &expression );

  /* try to connect to the OAA facilitator */

  if ( ! expression )
    fprintf( stderr, "TerraVision OAA Console. Type \"quit\" to exit.\n" );

  ConnectToFacilitator( oaa_host, oaa_port );

  /* support the -solve command line option and exit */

  if ( expression ) {
    SolveExpression( expression );
    PR_ProcessExit( 0 );
  }

  /* This hangs right now - more funky event stuff!! */
  /* GetAgents(); */

  /* go into the main interpreter loop... */

  puts( "" );
  while ( !feof(stdin) ) {
    
    /* read the next command from the console */

    printf( "%% " ); fflush( stdout );
    command[0] = '\0';
    fgets( command, MAX_CMD_SIZE, stdin );
    if ( command[0] == '\0' || command[0] == '\n' ) continue;
    command[strlen(command)-1] = '\0';

    /* send the command to the OAA facilitator */

    if ( strcmp( command, "quit" ) == 0 || strcmp( command, "bye" ) == 0 ||
	 strcmp( command, "exit" ) == 0 )
      PR_ProcessExit( 0 );
    else
      SolveExpression( command );

  }

  PR_ProcessExit( 0 );
}

