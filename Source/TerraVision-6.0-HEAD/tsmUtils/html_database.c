/*
 * Filename: html_database.c
 *
 * Description:
 *     This program will read the list of datasets on a server and then
 *     generate an HTML file with a selection form to select between
 *     all of the available datasets. This is used in conjunction with
 *     the Netscape plugin version of TerraVision.
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
 *     Martin Reddy, <reddy@ai.sri.com> - 20 March 1998.
 *
 * Revision Information:
 *
 *     $Id: html_database.c,v 1.3 2000/08/28 23:56:02 reddy Exp $
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

#define APP_VERSION tsmCvsToString( "$Revision: 1.3 $" )

/*
 * Synopsis:
 *    void usage( void );
 *
 * Description:
 *    Display the program usage information to the terminal.
 *    This is called if insufficient parameters are given.
 *
 */

void
usage( void )
{
  printf( "html_database release %s\n", APP_VERSION );
  puts( "\nUsage: html_database <server>" );

  puts( "\nReads the list of datasets on a server and produces an HTML file" );
  puts( "with JavaScript logic to add/remove datasets from a TV plugin." );

  puts( "\nRequired Parameters:" );
  puts( " <server> = the URL for the dataset server." );
  exit( 1 );
}

/*
 * Synopsis:
 *      int main( int argc, char **argv )
 *
 * Description:
 *      The main part of the program: checks the command line arguments,
 *      creates the tspec, adds the pyramid to the database, generates
 *      the tiles for each level of the pyramid.
 *
 * Author: Martin Reddy
 * 
 * Date: 3 September 1997
 *
 */

int
main( int argc, char **argv )
{
  TsmDatabase  database = { 0 };
  TsmServer    server = { NULL };
  FILE         *fp = stdout;
  int          i;

  /* display the program usage if we don't have enough args */

  tsmParseInitArgs( argc, argv );

  if ( argc < 2 ) usage();

  /* Display the SRI copyright message */

  printf( "HttpDatabase %s, Copyright (c) 2000 SRI International. "
          "All rights reserved.\n", APP_VERSION );

  /* parse the URL that we have been given for the dataset server */

  if ( ( server.location = tsmParseUrl( argv[1] ) ) == NULL ) {
    fprintf( stderr, "Couldn't parse URL: %s\n", argv[1] );
    exit( 1 );
  }

  /* read the server */

  tsmUpdateDatabaseFromServer( &database, &server, NULL );  

  /* generate the HTML code */

  fprintf( fp, "<HTML>\n" );
  fprintf( fp, "<head>\n" );
  fprintf( fp, "<title>TerraVision Plugin Dataset Chooser</title>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<script language=\"JavaScript\">\n" );
  fprintf( fp, "<!--\n" );
  fprintf( fp, "var geo_name = \"\";\n" );
  fprintf( fp, "var oi_url = \"\";\n" );
  fprintf( fp, "var dem_url = \"\";\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "function GetGeoName() {\n" );
  fprintf( fp, "  var geo_selected = \"\", i;\n" );
  fprintf( fp, "  var geo_list = new Array(%d), oi_list = new Array(%d), "
	   "dem_list = new Array(%d);\n", database.numGeoSets, 
	   database.numGeoSets, database.numGeoSets );
  fprintf( fp, "\n" );

  for ( i = 0; i < database.numGeoSets; i++ ) {
    TsmGeoSet  *geoset = &database.geoset[i];
    TsmDataSet *oi = tsmGetDataSet( &database, geoset->oi_name );
    TsmDataSet *dem = tsmGetDataSet( &database, geoset->dem_name );

    fprintf( fp, "  geo_list[%d] = \"%s\";\n", i,
	     geoset->name );
    fprintf( fp, "  oi_list[%d]  = \"%s\";\n", i,
	     oi->instance->location[0].url );
    fprintf( fp, "  dem_list[%d] = \"%s\";\n", i,
	     dem->instance->location[0].url );
  }

  fprintf( fp, "\n" );
  fprintf( fp, "  geo_name = \"\"; oi_url = \"\"; dem_url = \"\";\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "  for ( i = 0; i < %d; i++ ) \n", database.numGeoSets );
  fprintf( fp, "    if ( document.form2.geosets.options[i].selected )\n" );
  fprintf( fp, "     geo_selected = document.form2.geosets.options[i].value;");
  fprintf( fp, "\n\n" );
  fprintf( fp, "  if ( geo_selected ) {\n" );
  fprintf( fp, "    for ( i = 0; i < geo_list.length; i++ )\n" );
  fprintf( fp, "      if ( geo_selected == geo_list[i] ) {\n" );
  fprintf( fp, "        geo_name = geo_list[i];\n" );
  fprintf( fp, "        oi_url   = oi_list[i];\n" );
  fprintf( fp, "        dem_url  = dem_list[i];\n" );
  fprintf( fp, "      }\n" );
  fprintf( fp, "  }\n" );
  fprintf( fp, "}\n" );
  fprintf( fp, "function AddGeo() {\n" );
  fprintf( fp, "  GetGeoName();\n" );
  fprintf( fp, "  var cmdstr = \"loadterrain('\" +oi_url + \"','\" + dem_url + \"','\" + geo_name + \"')\";\n" );
  fprintf( fp, "  parent.TV.document.simple1.cmd( cmdstr );\n" );
  fprintf( fp, "}\n" );
  fprintf( fp, "function RemoveGeo() {\n" );
  fprintf( fp, "  GetGeoName();\n" );
  fprintf( fp, "  var cmdstr = \"unloadterrain('\" + geo_name + \"')\";\n" );
  fprintf( fp, "  parent.TV.document.simple1.cmd( cmdstr );\n" );
  fprintf( fp, "}\n" );
  fprintf( fp, "//-->\n" );
  fprintf( fp, "</script>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "</head>\n" );
  fprintf( fp, "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<p><b>Datasets:</b><p>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<center><table bgcolor=\"#CCCCFF\" cellpadding=\"5\" border=\"1\" width=\"100%%\">\n" );
  fprintf( fp, "<tr><td >\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<!-- Setup some forms with JavaScript to control the plugin via LiveConnect -->\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<form name=\"form2\">\n" );
  fprintf( fp, "<center>\n" );
  fprintf( fp, "<select name=\"geosets\" size=\"%d\">\n",
	   database.numGeoSets );

  for ( i = 0; i < database.numGeoSets; i++ ) {
    TsmGeoSet  *geoset = &database.geoset[i];
    fprintf( fp, "<option value=\"%s\">%s</option>\n", geoset->name,
	     geoset->name );
  }  

  fprintf( fp, "</select>\n" );
  fprintf( fp, "<input type=\"button\" value=\" Add \" onclick=\"AddGeo()\"><br>\n" );
  fprintf( fp, "<input type=\"button\" value=\"Remove\" onclick=\"RemoveGeo()\">\n" );
  fprintf( fp, "</center>\n" );
  fprintf( fp, "<p>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "</form>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "</table></center>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "</td></tr></table>\n" );
  fprintf( fp, "\n" );
  fprintf( fp, "<p><b>Server:</b><p>\n" );
  fprintf( fp, "\n%s\n", argv[1] );
  fprintf( fp, "</body>\n" );
  fprintf( fp, "</HTML>\n" );
  
  return 0;
}

/* EOF: html_database.c */
