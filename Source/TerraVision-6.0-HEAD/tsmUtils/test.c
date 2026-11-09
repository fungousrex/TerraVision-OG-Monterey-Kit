/*
 * Name: test.c
 *
 * Description:
 *     See the function usage() for the command line syntax.
 *
 *     This program was written for use with the tsmApi library from
 *     SRI International. For further information about this library,
 *     including downloads, documentation, and other examples, see:
 *
 *         http://www.tsmApi.com/
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 30 June 1998.
 *
 * Function List:
 *     void usage( void )
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: test.c,v 1.3 2000/08/28 23:56:07 reddy Exp $ 
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

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main program bitty.
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 30 June 1998
 *
 */

int main( int argc, char **argv )
{
  char *url = "iss-1.sri.magic.net";
  char *set_name;
  TsmHandle *tsmh;
  

  printf( "open %s...\n", url );

  if ( ( tsmh = tsmOpen( url ) ) == NULL ) {
    puts( "error doing tsmOpen" );
    exit( 1 );
  }

#ifdef DO_GEOS
  set_name = strdup(geoPyramid->tspec.demNames.val[0]);
#else
  set_name = calloc(1, sizeof (set_name) + 10);
  sprintf( set_name, "%s.dem", "2" );
#endif       
             
  /*
  load_tile_set_to_iss(ih_p, dsm, tsmh, 
		       &ds_info, pl_method, replicate_mode, no_write);
		       */
  printf( "close %s...\n", url );

  tsmClose(tsmh);
}

/*** EOF: test.c ***/
