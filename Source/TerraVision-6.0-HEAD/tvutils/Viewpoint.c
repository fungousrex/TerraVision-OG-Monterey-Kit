/*
  Filename: Viewpoint.c

  Description: Contains routines for managing the viewpoint bookmarks
  file for the TerraVision system, including locating and parsing the
  viewpoints list.

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
#include <string.h>
#include <math.h>
#include <tsm/tsm.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "tvutils.h"
#include "Args.h"
#include "Config.h"
#include "Platform.h"
#include "Viewpoint.h"

typedef struct {
  ViewpointItem *viewpoint;
  int           size;
} ViewpointList;


/* the database of viewpoints */

static ViewpointList view_dbase = { NULL, 0 };


/*
 * Synopsis:
 *   ViewpointItem *tvuFindViewpoint( char const *, char const * )
 *
 * Description:
 *   Searches through the current database of viewpoints looking for
 *   a viewpoint with the same name as that specified, and where the 
 *   viewpoint's dataset is the same as that specified.
 *
 * Returns: the found ViewpointItem or NULL if not found
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

ViewpointItem *
tvuFindViewpoint( char const *geoname, char const *search )
{
  int  i;

  if ( geoname == NULL || search == NULL || search[0] == '\0' )
    return NULL;

  /* Search for a view of the specified name in this dataset */

  for ( i = 0; i < view_dbase.size; i++ )
    if ( tvuStrCmp( search, view_dbase.viewpoint[i].name ) )
      if ( tvuStrCmp( geoname, view_dbase.viewpoint[i].dataset ) )
	return &view_dbase.viewpoint[i];

  return NULL;
}

/*
 * Synopsis:
 *   int tvuAddViewpoint( ViewpointItem *new_view )
 *
 * Description:
 *   Adds the specified ViewpointItem to the current viewpoint
 *   database. We do not add the item if it already exists, i.e. if
 *   the name/dataset strings match an already existing viewpoint.
 *
 * Returns: TRUE if added, FALSE if not added to database
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
tvuAddViewpoint( ViewpointItem *new_view )
{
  int i, new_size;
  ViewpointItem *new_viewpoint;

  /* If string already exists for this dataset, then don't add it */

  for ( i = 0; i < view_dbase.size; i++ )
    if ( tvuStrCmp( new_view->name, view_dbase.viewpoint[i].name ) )
      if ( tvuStrCmp( new_view->dataset, view_dbase.viewpoint[i].dataset ) )
	return FALSE;

  /* String is unique, so add an extra entry for the new string */

  new_size  = view_dbase.size + 1;
  new_viewpoint = (ViewpointItem *) realloc( view_dbase.viewpoint,
			     new_size * sizeof(ViewpointItem) );

  if ( new_viewpoint == NULL ) return FALSE;

  /* And then add the string to the database */

  new_viewpoint[new_size-1] = *new_view;

  view_dbase.viewpoint = new_viewpoint;
  view_dbase.size      = new_size;

  return TRUE;
}

/*
 * Synopsis:
 *   int tvuDeleteViewpoint( char const *, char const * )
 *
 * Description:
 *   Given a viewpoint name, we search for this viewpoint in the database
 *   and remove it if found. N.B. we only match a viewpoint if it's name
 *   is the same as that specified AND if the viewpoint's dataset is the
 *   same as that specified by the geoname string.
 *
 * Returns: TRUE if deleted, FALSE if not deleted
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
tvuDeleteViewpoint( char const *geoname, char const *del_string )
{
  int i, j;

  if ( del_string == NULL || del_string[0] == '\0' ) return FALSE;

  /* Search for the string to be deleted */

  for ( i = 0; i < view_dbase.size; i++ ) {
    if ( tvuStrCmp( del_string, view_dbase.viewpoint[i].name ) ) {
      if ( tvuStrCmp( geoname, view_dbase.viewpoint[i].dataset ) ) {

	/* Found it! So let's remove it and do that shuffle thang */

	if ( view_dbase.viewpoint[i].name != NULL )
	  free( (void *) view_dbase.viewpoint[i].name );
      
	if ( view_dbase.viewpoint[i].dataset != NULL )
	  free( (void *) view_dbase.viewpoint[i].dataset );

	for ( j = i; j < view_dbase.size-1; j++ )
	  view_dbase.viewpoint[j] = view_dbase.viewpoint[j+1];

	view_dbase.size--;

	return TRUE;
      }
    }
  }

  return FALSE;
}

/*
 * Synopsis:
 *   void tvuFreeViewpoints( void )
 *
 * Description:
 *   Empties the current viewpoint database, freeing any relevant 
 *   memory.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

void
tvuFreeViewpoints( void )
{
  int i;

  if ( view_dbase.viewpoint == NULL ) return;

  for ( i = 0; i < view_dbase.size; i++ ) {
    if ( view_dbase.viewpoint[i].name )
      free( (void *) view_dbase.viewpoint[i].name );
    if ( view_dbase.viewpoint[i].dataset )
      free( (void *) view_dbase.viewpoint[i].dataset );
  }

  free( view_dbase.viewpoint );
  view_dbase.viewpoint = NULL;
  view_dbase.size      = 0;
}

/*
 * Synopsis:
 *   int tvuViewpointCount( void )
 *
 * Description:
 *   Return the number of viewpoints in the currently loaded
 *   viewpoint list. 0 means no viewpoints loaded.
 *
 * Returns: number of viewpoints loaded.
 *
 * Author: Martin Reddy
 *
 * Date: 13 November 2000
 *
 */

int
tvuViewpointCount( void )
{
  return view_dbase.size;
}

/*
 * Synopsis:
 *   ViewpointItem *tvuViewpoint( int i )
 *
 * Description:
 *   Return the n'th viewpoint in th currently loaded viewpoint list.
 *   The valid range is zero to n-1, where n is the value returned
 *   by tvuViewpointCount().
 *
 * Returns: a valid ViewpointItem, or NULL if i is out of range
 *
 * Author: Martin Reddy
 *
 * Date: 13 November 2000
 *
 */

ViewpointItem *
tvuViewpoint( int i )
{
  if ( i < 0 || i >= view_dbase.size )
    return NULL;
  else
    return &view_dbase.viewpoint[i];
}

/*
 * Synopsis:
 *   void tvuSaveViewpoints( void )
 *
 * Description:
 *   Saves the current viewpoint database to the user's
 *   viewpoints.xml file.
 *
 * Returns: TRUE if file saved, FALSE if error occurred.
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
tvuSaveViewpoints( void )
{
  xmlDocPtr  doc;
  xmlNodePtr root, tree, subtree;
  char       *filename = tvuGetViewpointsFilename();
  char       *thedate = tsmDateString( (PRTime) 0 );
  int        i, bytes_written;

  /* Only save the viewpoints database if it has been updated */

  xmlIndentTreeOutput = 2;    /* indent nested XML tags by 2 spaces */

  /* create the XML-format header for the viewpoint file */

  doc = xmlNewDoc( "1.0" );
  xmlDocSetRootElement( doc, xmlNewDocNode(doc, NULL, "Viewpoints", NULL) );
  root = xmlDocGetRootElement( doc );
  xmlSetProp( root, "version", TERRAVISION_RELEASE );
  if ( thedate )
    xmlSetProp( root, "created", (xmlChar *) thedate );

  /* allocate an array of temp buffers to create the variable strings */

  /* Write out each viewpoint in an XML tag */

  for ( i = 0; i < view_dbase.size; ++i ) {
    ViewpointItem *item = &view_dbase.viewpoint[i];
    char          buffer[200];
    char          *ptr = buffer;

    tree = xmlNewChild( root, NULL, "viewpoint", NULL);
    xmlNewChild( tree, NULL, "name", (xmlChar *) item->name );
    xmlNewChild( tree, NULL, "dataset", (xmlChar *) item->dataset );

    sprintf( buffer, "%s", ( item->coordsys ) ? "gcc" : "lvcs" );
    xmlNewChild( tree, NULL, "coordsys", (xmlChar *) buffer );

    subtree = xmlNewChild( tree, NULL, "location", NULL );
    
    sprintf( ptr, "%1.10g", item->x );
    xmlSetProp( subtree, "x", (xmlChar *) ptr );
    ptr += 20;
    
    sprintf( ptr, "%1.10g", item->y );
    xmlSetProp( subtree, "y", (xmlChar *) ptr );
    ptr += 20;

    sprintf( ptr, "%1.10g", item->z );
    xmlSetProp( subtree, "z", (xmlChar *) ptr );
    ptr += 20;

    subtree = xmlNewChild( tree, NULL, "orientation", NULL );

    sprintf( ptr, "%1.6g", item->yaw );
    xmlSetProp( subtree, "yaw", (xmlChar *) ptr );
    ptr += 20;
    
    sprintf( ptr, "%1.6g", item->pitch );
    xmlSetProp( subtree, "pitch", (xmlChar *) ptr );
    ptr += 20;

    sprintf( ptr, "%1.6g", item->roll );
    xmlSetProp( subtree, "roll", (xmlChar *) ptr );
  }

  /* save this XML structure to the viewpoints file */

  tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );
  bytes_written = xmlSaveFile( filename, doc );

  xmlFreeDoc( doc );

  return ( bytes_written > 0 );
}

int
tvuLoadViewpoints_new( void )
{
  xmlDocPtr     doc;
  xmlNodePtr    root, tree, subtree;
  char          *filename = tvuGetViewpointsFilename();
  ViewpointItem item;

  /* open and parse the config.xml file */

  if ( ( doc = xmlParseFile( filename ) ) == NULL )
    return FALSE;

  /* Clear any currently loaded database */

  tvuFreeViewpoints();

  /* get the root node and check that it is "TerraVision" */

  if ( ( root = xmlDocGetRootElement( doc ) ) == NULL )
    return FALSE;

  if ( tvuStrCmp( (char *) root->name, "Viewpoints" ) == FALSE ||
       ( tree = root->xmlChildrenNode ) == NULL )
    return FALSE;

  /* walk through each entry and parse it */

  tree = tree->next;
  while ( tree ) {
    char *name = (char *) tree->name;
    
    if ( ! tvuStrCmp( name, "viewpoint" ) ) continue;
    if ( ( subtree = tree->xmlChildrenNode->next ) == NULL ) continue;

    /* initialise and read this viewpoint definition */

    memset( &item, 0, sizeof(ViewpointItem) );

    /* read the tags and attributes for this viewpoint tag */

    while ( subtree ) {
      name = (char *) subtree->name;

      if ( tvuStrCmp( name, "name" ) ) {
	item.name = (char *) xmlNodeGetContent( subtree );

      } else if ( tvuStrCmp( name, "dataset" ) ) {
	item.dataset = (char *) xmlNodeGetContent( subtree );
 
      } else if ( tvuStrCmp( name, "coordsys" ) ) {
	char *coordstr = (char *) xmlNodeGetContent( subtree );
	if ( coordstr ) {
	  item.coordsys = tvuStrCmp( coordstr, "gcc" );
	  free( coordstr );
	}

      } else if ( tvuStrCmp( name, "location" ) ) {
	char *x = (char *) xmlGetProp( subtree, "x" );
	char *y = (char *) xmlGetProp( subtree, "y" );
	char *z = (char *) xmlGetProp( subtree, "z" );
	item.x = ( x ) ? atof( x ) : 0.0;
	item.y = ( y ) ? atof( y ) : 0.0;
	item.z = ( z ) ? atof( z ) : 0.0;
	if ( x ) free( x );
	if ( y ) free( y );
	if ( z ) free( z );

      } else if ( tvuStrCmp( name, "orientation" ) ) {
	char *yaw = (char *) xmlGetProp( subtree, "yaw" );
	char *pitch = (char *) xmlGetProp( subtree, "pitch" );
	char *roll = (char *) xmlGetProp( subtree, "roll" );
	item.yaw   = ( yaw ) ? atof( yaw ) : 0.0;
	item.pitch = ( pitch ) ? atof( pitch ) : 0.0;
	item.roll  = ( roll ) ? atof( roll) : 0.0 ;
	if ( yaw ) free( yaw );
	if ( pitch ) free( pitch );
	if ( roll ) free( roll );
	
      }

      subtree = subtree->next->next;
    }

    tvuAddViewpoint( &item );

    tree = tree->next->next;
  }

  /* all done! */
  
  xmlFreeDoc( doc );

  return TRUE;

}

/*
 * Synopsis:
 *   void ViewpointUpgradeItem ( int version, ViewpointItem *view )
 *
 * Description:
 *   Take a viewpoint from a previous file version and upgrade to the current
 *   format.
 *
 *   Version 0: Locations were of the form (x,z,y) in dataset-local
 *              coordinates.
 *
 * Returns: void
 *
 * Author: Lee Iverson
 *
 * Date: April 13, 1999
 *
 */

static void
tvuViewpointUpgradeItem (ViewpointItem *item, int from_version)
{
  /* convert from an older version. (N.B. No break statements.  That way,
     these can cascade from older to newer). */
  switch (from_version) {
  case 0: {
    double tmp = item->y;
    item->y = item->z;
    item->z = tmp;
    item->pitch = -item->pitch + TVU_PI/2;
    item->yaw   =  item->yaw   - TVU_PI/2;
  }
  }

  /* canonicalize pitch and yaw */
  if (item->yaw <= -TVU_PI)
    item->yaw += 2*TVU_PI;  
  else if (item->yaw > TVU_PI)
    item->yaw -= 2*TVU_PI;  
  if (item->pitch <= -TVU_PI)
    item->pitch += 2*TVU_PI;  
  else if (item->pitch > TVU_PI)
    item->pitch -= 2*TVU_PI;  
}

/*
 * Synopsis:
 *   void tvuLoadViewpoints_old( void )
 *
 * Description:
 *   Attempts to load the user's .TerraVisionViewpoints file into
 *   our viewpoint database. If this file does not exist, then nothing
 *   will happen. The .TerraVisionViewpoints file is the old style
 *   that we used for storing viewpoint files. We now use a true XML
 *   file format and libxml.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 29 May 1998
 *
 */

int
tvuLoadViewpoints_old( char *filename )
{
  ViewpointItem item;
  char          *sep;
  char          buffer[512], token[512];
  FILE          *handle;
  int           size;
  int           header_seen = 0;
  int           version;

  /* Open the .TerraVisionViewpoints file for reading */

  if ( filename == NULL ) {
#ifdef TVU_UNIX
    /* expand the $HOME/.TerraVisionViewpoints filename under unix */
  
    char *homedir = getenv( "HOME" );
    if ( homedir == NULL ) return FALSE;
    sprintf( buffer, "%s/.TerraVisionViewpoints", homedir );
    filename = buffer;
#else
    return FALSE;
#endif
  }

  if ( ( handle = tsmOpenStream( filename, "r" ) ) == NULL )
    return FALSE;

  /* Clear any currently loaded database */

  tvuFreeViewpoints();

  memset( &item, 0, sizeof(ViewpointItem) );

  /* Parse each line from the data file into the new database */

  while ( ! feof( handle ) ) {
    char *b;

    /* Get the next line from the file, and strip any */
    /* line feeds, carriage returns, ctrl chars etc.  */ 

    buffer[0] = '\0';
    fgets( buffer, 512, handle );
    while ( ( size = strlen(buffer) ) > 0 && buffer[size-1] < ' ' )
      buffer[size-1] = '\0';

    /* skip white space and line if empty */
    for (b = buffer; *b && isspace (*b); ++b);
    if (*b == '\0') continue;

    if (! header_seen) {
      if (strncmp( b, "<!DOCTYPE ", 10 ) == 0 ) {
	sscanf( b, "<!DOCTYPE VIEWPOINT %d>", &version );
	header_seen = 1;
	continue;
      }
    }

    /* If it's an open tag, then clear the current item record ready for */
    /* new data. If it is a close tag, then add the record we just read. */
    if (! header_seen) {
      /* Version 0 file. */
      version = 0;
      header_seen = 1;
    }

    if ( tvuStrCmp( b, "<VIEWPOINT>" ) ) {
      memset( &item, 0, sizeof(ViewpointItem) );
    }

    else if ( tvuStrCmp( b, "</VIEWPOINT>" ) ) {
      tvuViewpointUpgradeItem ( &item, version );
      tvuAddViewpoint( &item );
    }

    /* Otherwise check to see if we have a data tag */

    else {
      sscanf( b, "%s", token );
      if ( ( sep = strchr( b, ':' ) ) != NULL ) {
	sep++; sep++; /* skip ':' and one whitespace */

	if ( tvuStrCmp( token, "name:" ) ) {
	  item.name = strdup( sep );
	} else if ( tvuStrCmp( token, "dataset:" ) ) {
	  item.dataset = strdup( sep );
	} else if ( tvuStrCmp( token, "location:" ) ) {
	  sscanf( sep, "%lf%lf%lf\n", &item.x, &item.y, &item.z );
	} else if ( tvuStrCmp( token, "gcc:" ) ) {
	  item.coordsys = ( (int) atof( sep ) ) == 1;
	} else if ( tvuStrCmp( token, "position:" ) ) {
	  /* obsolete token where y and z where flipped */
	  sscanf( sep, "%lf%lf%lf\n", &item.x, &item.z, &item.y );
	} else if ( tvuStrCmp( token, "orientation:" ) ) {
	  sscanf( sep, "%f%f%f\n", &item.yaw, &item.pitch, &item.roll );
	}
      }
    }

  }

  tsmCloseStream( handle );
  return TRUE;
}

int
tvuLoadViewpoints( void )
{
  char *filename = tvuGetViewpointsFilename();
  char buffer[128] = "";
  FILE *handle;

  /* open the viewpoints file and read the first few bytes */

  if ( ( handle = tsmOpenStream( filename, "r" ) ) == NULL )
    return FALSE;

  fgets( buffer, 128, handle );
  tsmCloseStream( handle );

  /* if true XML, then use the new routines, else support */
  /* the old psuedo-XML format used by olders versions.   */

  if ( strncmp( buffer, "<?xml", 5 ) == 0 )
    return tvuLoadViewpoints_new();
  else
    return tvuLoadViewpoints_old( NULL );
}
