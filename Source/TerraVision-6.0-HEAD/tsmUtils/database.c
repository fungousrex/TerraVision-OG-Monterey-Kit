/*
 * Name: database.c
 *
 * Description:
 *   This program provides various facilities for maintaining a dataset
 *   database. This includes being able to list all of the server names
 *   and dataset names in the database, add a new server to the database,
 *   remove all instances of a server from the database, and update the
 *   contents of the database with the datasets on all of the servers
 *   that it knows about.
 *
 *   You provide an input database file which will be overwritten with a
 *   new database containing all of the updated information.
 *
 *   For the -update option, if a server cannot be contacted, then the
 *   information that already exists in the database for that server
 *   will not be changed.
 *
 *   As an example, if you give this progam a file that contains only
 *   an entry for a single server, e.g.
 *
 *      <server>
 *        location: x-dsm://iss-1-atm.sri.magic.net/
 *      </server>
 *
 *   and call this program with the -update option, then this file will
 *   be updated with all of the information about all of the Pyramid
 *   datasets stored on that DPSS.
 *
 *   N.B. the format used for the dataset database file is described at:
 *   http://www.tsmApi.com/
 *
 *   This program was written for use with the tsmApi library from
 *   SRI International. For further information about this library,
 *   including downloads, documentation, and other examples, see:
 *
 *       http://www.ai.sri.com/~magic/tsmApi
 *
 * Author:
 *   Martin Reddy, <reddy@ai.sri.com> - 9 March 1998.
 *
 * Function List:
 *   int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: database.c,v 1.4 2000/11/27 23:57:30 reddy Exp $
 * 
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

#define APP_VERSION tsmCvsToString( "$Revision: 1.4 $" )

typedef enum {
  MODE_LIST,
  MODE_UPDATE,
  MODE_REMOVE,
  MODE_ADD
} ModeType;

int
main( int argc, char **argv )
{
  static char    *CVSRevision = "$Revision: 1.4 $";
  TsmDatabase    *database;
  TsmServer      server;
  ModeType       mode = MODE_LIST;
  int            i;
  char           *serverName;
  FILE           *filetest;

  /* do we have enough arguments? */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 ) {
    printf( "database - release %s\n", tsmCvsToString(CVSRevision) );
    puts( "\nusage: database <dbase_url> [...]" );
    puts( "\nA dataset database maintenance utility. This can list the" );
    puts( "contents of a database (default), update a database will all" );
    puts( "datasets on all servers it knows about, remove all instances" );
    puts( "from the database for a server, or add a new server entry.\n" );
    puts( " -update = updates database with datasets at all known servers" );
    puts( " -remove <server> = removes all dataset instances for the named" );
    puts( "                  server, e.g. x-dsm://iss-1-atm.sri.magic.net/" );
    puts( " -add <server> = adds a new server entry, if not already present" );
    puts( " -list = list all servers and datasets in database (default)" );
    tsmDescribeArgs( TSM_ARG_INIT );
    puts( "" );
    exit( 0 );
  }

  /* Display the SRI copyright message */

  printf( "Database %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* check command line for any supported options */

  if ( tsmGetArg( argc, argv, "-stdout" ) )
    fprintf( stderr, "Obsolete option: -stdout. Ignored\n" );

  if ( tsmGetArg( argc, argv, "-update" ) ) mode = MODE_UPDATE;
  if ( tsmGetArg( argc, argv, "-list" ) ) mode = MODE_LIST;

  if ( tsmGetArgString( argc, argv, "-remove", &serverName ) )
    mode = MODE_REMOVE;

  if ( tsmGetArgString( argc, argv, "-add", &serverName ) )
    mode = MODE_ADD;

  if ( tsmParsedAllOptions( FALSE ) == FALSE ) exit( 1 );

  /* if the database file does not exist, then create an empty file */

  if ( ( filetest = fopen( argv[1], "r" ) ) == NULL ) {
    if ( ( filetest = fopen( argv[1], "w" ) ) == NULL )
      printf( "Couldn't create database: %s\n", argv[1] );
    else {
      fclose( filetest );
      printf( "Created new database file: %s\n", argv[1] );
    }
  } else
    fclose( filetest );
  
  /* Read the current database */

  if ( ( database = tsmReadDatabase( argv[1] ) ) == NULL ) {
    fprintf( stderr, "Error reading dataset database: %s\n", argv[1] );
    exit( 1 );
  }

  /* now, do we want to remove instances, or add instances? */

  switch ( mode ) {
  case MODE_LIST:
    if ( database->numServers == 0 )
      printf( "No Servers\n" );
    else
      for ( i = 0; i < database->numServers; ++i )
	printf( "Server: %s\n", database->server[i].location->url );

    if ( database->numGeoSets == 0 )
      printf( "No Geosets\n" );
    else
      for ( i = 0; i < database->numGeoSets; ++i )
	printf( "Geoset: %s\n", database->geoset[i].name );

    if ( database->numDataSets == 0 )
      printf( "No Datasets\n" );
    else
      for ( i = 0; i < database->numDataSets; ++i )
	printf( "Dataset: %s (%d instances)\n", database->dataset[i].name,
		database->dataset[i].numInstances );

    break;

  case MODE_REMOVE:
    
    if ( ( server.location = tsmParseUrl( serverName ) ) == NULL ) {
      fprintf( stderr, "couldn't parse server name as URL\n" );
      exit( 1 );
    }
    
    tsmRemoveServerSetsFromDatabase( database, &server, NULL );
    tsmRemoveServerFromDatabase( database, server.location->url );

    fprintf( stderr, "%s removed from database\n", serverName );

    tsmFreePurl( server.location );

    break;

  case MODE_ADD:
    
    tsmAddServerToDatabase( database, serverName );
    fprintf( stderr, "%s added to database\n", serverName );

    break;

  case MODE_UPDATE:

    /* if there are no server definitions then we don't know where to look */

    if ( database->numServers == 0 ) {
      fprintf( stderr, "database contains no server definitions\n" );
      exit( 1 );
    }
    
    /* Otherwise, try to contact each server that has been specified */
    /* and update the database with all the datasets on that server  */

    for ( i = 0; i < database->numServers; ++i ) {
      fprintf( stderr, "%s\n", database->server[i].location->url );
      tsmUpdateDatabaseFromServer( database, &database->server[i], NULL );
    }

  }

  /* Write out the database to file if modified */

  if ( mode != MODE_LIST )
    tsmWriteDatabase( database, argv[1] );

  tsmFreeDatabase( database );

  return 0;
}

/*** EOF: database.c ***/
