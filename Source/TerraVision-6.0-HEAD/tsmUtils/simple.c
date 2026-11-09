/*
 * Name: simple.c
 *
 * Description:
 *     This file contains a simple main program to read in one tile and print
 *     its checksum using the Tile Set Manager (TSM) API.  Its primary
 *     purpose is to demonstrate the API with as little "clutter" as
 *     possible.
 *
 *     Note that no error checking of any kind is done here so as to make
 *     the structure of the calling sequence easy to understand. When 
 *     developing actual programs you should of course always test for
 *     return codes to see if any operation has failed. See other example
 *     programs for examples on doing this, e.g. gettile.c
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
 *     int  main( int argc, char **argv )
 *
 * Revision Information:
 *
 *     $Id: simple.c,v 1.3 2000/08/28 23:56:07 reddy Exp $
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
#include <tsm/tsm.h>

int
main( int argc, char **argv )
{
  TsmConnection connect;
  uchar         *tileBuffer;
  int           tx, ty, level, checksum;

  /* Open a connection to a local tileset (this may not be on your system) */

  connect = tsmConnect( "~magic/TileSets/Pyramids/yosemite.oi", "r", NULL );

  /* Let's choose some tile to read by defining its coords and res. level */

  tx = 5; ty = 3; level = 1;

  /* Now read this tile from the dataset that we have opened */

  tsmAllocReadTile( connect, tx, ty, level, &tileBuffer );

  /* Calculate and print out the checksum for the tile */

  checksum = tsmCalcTileChecksum( connect, tileBuffer );
  printf( "Checksum for tile (%d,%d: %d) = %d\n", tx, ty, level, checksum );

  /* Free the tile buffer for the tile we read in and disconnect */

  tsmFreeTileBuffer( tileBuffer );
  tsmDisconnect( connect );

  return 0;
}

/*** EOF: simple.c ***/
