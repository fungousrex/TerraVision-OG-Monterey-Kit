/*
  Filename: Config.c

  Description: Contains routines for managing the configuration of the
  TerraVision system, including locating and parsing the config file.

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
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tsm/tsm.h>
#include <tsm/tsmPurl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <TerraVision/Trace.h>
#include <TerraVision/OAA.h>

#include "Platform.h"
#include "tvutils.h"
#include "Args.h"
#include "Config.h"
#include "Viewpoint.h"
#include "Init.h"

#ifdef TVU_UNIX
#include <unistd.h>
#endif

#define MAX_DIR_LENGTH   512

typedef struct {
  xmlChar *option;
  xmlChar *value;
  xmlChar *value2;
} ConfigItem;

static ConfigItem **config = NULL;
static int        config_size = 0;

int
tvuStrCmp( char const *str1, char const *str2 )
{
  int i, len1, len2;
  if ( str1 == NULL || str2 == NULL ) return FALSE;
  len1 = strlen( str1 );
  len2 = strlen( str2 );
  if ( len1 != len2 ) return FALSE;
  for ( i = 0; i < len1; i++ ) 
    if ( toupper( str1[i] ) != toupper( str2[i] ) )
      return FALSE;
  return TRUE;
}

/* is str2 in str1? */

int
tvuSubStr( char const *str1, char const *str2 )
{
  int  i, len1, len2;
  char *s1, *s2, *result;

  if ( str1 == NULL || str2 == NULL ) return FALSE;

  s1 = strdup( str1 );
  s2 = strdup( str2 );
  len1 = strlen( s1 );
  len2 = strlen( s2 );

  for ( i = 0; i < len1; i++ ) s1[i] = tolower(s1[i]);
  for ( i = 0; i < len2; i++ ) s2[i] = tolower(s2[i]);

  result = strstr( s1, s2 );

  free( s1 );
  free( s2 );

  return ( result != NULL );
}

static int
tvuAddToConfig( char *opt, char *val, char *val2 )
{
  ConfigItem *item;
  int        success = FALSE, i;

  /* first of all check to see if the config entry already exists */

  for ( i = 0; i < config_size; i++ ) {
    item = config[i];
    if ( tvuStrCmp( (char *) item->option, opt ) ) {
      int overwrite = TRUE;

      /* we have found an item with the same option name */

      /* do we have a dataset definition that is already defined? */
      
      if ( tvuStrCmp( opt, "dataset" ) ) {
	if ( tvuStrCmp( val, (char *) item->value ) &&
	     tvuStrCmp( val2, (char *) item->value2 ) )
	  return TRUE;
	else
	  overwrite = FALSE;
      }

      /* do we have a command definition that is already defined? */
	
      if ( tvuStrCmp( opt, "command" ) ) {
	if ( tvuStrCmp( val, (char *) item->value ) )
	  return TRUE;
	else
	  overwrite = FALSE;
      }

      /* otherwise, let's override this same option with the new setting */

      if ( overwrite ) {
	if ( item->value ) free( item->value );
	if ( item->value2 ) free( item->value2 );
	item->value  = ( val ) ? (xmlChar *) strdup( val ) : NULL;
	item->value2 = ( val2 ) ? (xmlChar *) strdup( val2 ) : NULL;
	return TRUE;
      }
      
    }
  }

  /* otherwise, create a new entry */

  if ( ( item = calloc( sizeof(ConfigItem), 1 ) ) == NULL )
    return FALSE;

  item->option = ( opt ) ? (xmlChar *) strdup( opt ) : NULL;
  item->value  = ( val ) ? (xmlChar *) strdup( val ) : NULL;
  item->value2 = ( val2 ) ? (xmlChar *) strdup( val2 ) : NULL;

  /* add to the end of the list */

  if ( config == NULL ) {
    if ( ( config = malloc( sizeof(ConfigItem *) ) ) ) {
      *config = item;
      config_size = 1;
      success = TRUE;
    }
  } else {
    config = realloc( config, sizeof(ConfigItem *) * ( config_size + 1 ) );
    if ( config ) {
      config[config_size++] = item;
      success = TRUE;
    }
  }

  /* free the allocated structure if there was an error */

  if ( ! success ) {
    if ( item->option ) free( item->option );
    if ( item->value ) free( item->value );
    if ( item->value2 ) free( item->value2 );
    free( item );
  }

  return success;
}

int
tvuAddConfig1( char const *option, char const *value )
{
  if ( option == NULL || value == NULL ) return FALSE;
  return tvuAddToConfig( (char *) option, (char *) value, NULL );
}

int
tvuAddConfig2( char const *option, char const *value1, char const *value2 )
{
  if ( option == NULL || value1 == NULL || value2 == NULL ) return FALSE;
  return tvuAddToConfig( (char *) option, (char *) value1, (char *) value2 );
}


void
tvuFreeConfig( void )
{
  int i;

  if ( config == NULL ) return;

  for ( i = 0; i < config_size; i++ ) {
    ConfigItem *item = config[i];
    if ( item->option ) free( item->option );
    if ( item->value ) free( item->value );
    if ( item->value2 ) free( item->value2 );
    free( item );
  }

  free( config );
  config = NULL;
}

static char *
tvuCatPath( char *string, char *path )
{
  int length = strlen( string );
  if ( length > 0 && ( string[length-1] != '/' && string[length-1] != '\\' ) )
    strcat( string, "/" );
  strcat( string, path );
  return string;
}

char *
tvuGetInstallDir( void )
{
#ifdef TVU_WINDOWS
  char        *subkey;
  HKEY        key;
  LONG        ret, key_type;
  static char install_dir[MAX_DIR_LENGTH];
  long        length = MAX_DIR_LENGTH;

  /* create a default install directory string hardwired to C: */
  /* and work out the Registry key to look up based on mode.   */

  if ( tvu_mode == TVU_PLUGIN ) {
    subkey = "Software\\SRI International\\TerraVision Plugin";
    strcpy( install_dir,
	    "C:/Program Files/SRI International/TerraVision Plugin");
  } else {
    subkey = "Software\\SRI International\\TerraVision";
    strcpy( install_dir,
	    "C:/Program Files/SRI International/TerraVision");
  }

  /* try to get the TerraVision Plugin registry key */

  ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_EXECUTE, &key );
  if ( ret == ERROR_SUCCESS ) {

    /* extract the install directory string if present */

    RegQueryValueEx( key, "InstallDirectory", NULL, &key_type,
			   install_dir, &length );
    
    /* close the registry key that we just opened */

    RegCloseKey( key );
  }

  return install_dir;
#else
  static char *install_dir;

  /* use the $TVHOME environment variable under unix */

  if ( ( install_dir = getenv("TVHOME") ) == NULL ) {

    /* use the old $MAGICHOME if set */

    if ( ( install_dir = getenv("MAGICHOME") ) == NULL ) {

      install_dir = "";
    }
  }
#endif

  return install_dir;
}


char *
tvuGetUserDir( void )
{
#ifdef TVU_WINDOWS
  static char homeDir[MAX_DIR_LENGTH];

  /* get the TerraVision install dir */

  strcpy( homeDir, tvuGetInstallDir() );

  /* add the "config" dir on to the end */

  tvuCatPath( homeDir, "config" );

  return homeDir;
#else
  static char homeDir[MAX_DIR_LENGTH], *env;

  homeDir[0] = '\0';
  if ( ( env = getenv("HOME") ) != NULL ) {
    strcpy( homeDir, env );
    tvuCatPath( homeDir, ".tv" );
  }

  return homeDir;
#endif
}

char *
tvuGetConfigFilename( void )
{
  static char buffer[MAX_DIR_LENGTH], *dir;

  /* use the -config command line value if given */

  if ( config_file ) {
    strcpy( buffer, config_file );
    return buffer;
  }

  /* otherwise, use the default location */
  
  buffer[0] = '\0';
  if ( ( dir = tvuGetUserDir() ) != NULL )
    strcpy( buffer, dir );
  tvuCatPath( buffer, "config.xml" );
  return buffer;
}

char *
tvuGetDatabaseFilename( void )
{
  static char buffer[MAX_DIR_LENGTH], *dir;

  /* use the -database command line value if given */

  if ( database_url ) {
    strcpy( buffer, database_url );
    return buffer;
  }

  /* otherwise, use the default location */
  
  buffer[0] = '\0';
  if ( ( dir = tvuGetUserDir() ) != NULL )
    strcpy( buffer, dir );
  tvuCatPath( buffer, "database.xml" );
  return buffer;
}

char *
tvuGetViewpointsFilename( void )
{
  static char buffer[MAX_DIR_LENGTH], *dir;

  /* use the -database command line value if given */

  if ( viewpoint_url ) {
    strcpy( buffer, viewpoint_url );
    return buffer;
  }

  /* otherwise, use the default location */
  
  buffer[0] = '\0';
  if ( ( dir = tvuGetUserDir() ) != NULL )
    strcpy( buffer, dir );
  tvuCatPath( buffer, "viewpoints.xml" );
  return buffer;
}

char *
tvuGetPathUrl( char *filename )
{
  static char buffer[MAX_DIR_LENGTH], *dir;

  if ( filename && ( filename[0] == '/' || filename[0] == '\\' ||
		     filename[1] == ':' ) ) {

    /* if filename is non-NULL and is a full path, then use it */
    
    strcpy( buffer, filename );

  } else {

    /* otherwise, use the default path dir and append filename if non-NULL */

    buffer[0] = '\0';
    if ( ( dir = tvuGetUserDir() ) != NULL )
      strcpy( buffer, dir );
    tvuCatPath( buffer, "paths" );
    if ( filename ) {
      tvuCatPath( buffer, filename );
      tsmAddFilenameExtension( buffer, ".path" );
    }
  
  }
  return buffer;
}

static char *fallback_tcldir = NULL;

/* set the fallback tcl directory for when it cannot be found */

static void
tvuSetTclFilename( char const *dir )
{
  if ( fallback_tcldir ) free( fallback_tcldir );
  fallback_tcldir = strdup( dir );
}

static void
tvuSaveTclPathname( char const *dir )
{
  const char *funcName = "tvuSaveTclPathname";
  char abs_path[MAX_DIR_LENGTH];

  if ( ! dir ) return;
  /* add the pathname to the config file for use next time round */

  if ( dir[0] == '.' && dir[1] == '.' ) {
    /* convert relative pathname to an absolute pathname */
    getcwd( abs_path, MAX_DIR_LENGTH );
    tvuCatPath( abs_path, (char *) dir );
    tvuAddConfig1( "tcl", abs_path );
  } else
    tvuAddConfig1( "tcl", dir );

  tvuSaveConfig();

  /* need to change to the tcl dir for Tcl/Tk to find its files */

  TSMTRACE(( INIT_MASK, funcName, "Using Tcl/Tk dir %s", dir ));

  if ( chdir( dir ) < 0 )
    tsmError( funcName, "Couldn't change to Tcl/Tk dir: %s", dir );
}

char *
tvuGetTclFilename( void )
{
  static char buffer[MAX_DIR_LENGTH], filename[MAX_DIR_LENGTH], *dir;
  static char *tcl_main = "TerraVision.tcl";
  FILE        *fp;

  /* use the -tcl command line value if given */

  if ( tcl_path ) {

    /* find the TerraVision.tcl file in that directory */

    strcpy( buffer, tcl_path );
    tvuCatPath( buffer, tcl_main );

    if ( ( fp = fopen( buffer, "r" ) ) ) {
      fclose( fp );
      tvuSaveTclPathname( tcl_path );
      return tcl_main;
    }
  }

  /* try finding TerraVision.tcl in current directory first */

  strcpy( buffer, tcl_main );

  if ( ( fp = fopen( buffer, "r" ) ) ) {
    fclose( fp );
    tvuSaveTclPathname( "." );
    return tcl_main;
  }

  /* otherwise, let's check our fallback name */

  if ( fallback_tcldir ) {
    tvuSaveTclPathname( fallback_tcldir );
    return tcl_main;
  }

  /* otherwise, try in the $TVHOME/tcl directory */
  
  buffer[0] = '\0';
  if ( ( dir = tvuGetInstallDir() ) != NULL )
    strcpy( buffer, dir );
  tvuCatPath( buffer, "tcl" );

  strcpy( filename, buffer );
  tvuCatPath( filename, tcl_main );

  if ( ( fp = fopen( filename, "r" ) ) ) {
    fclose( fp );
    tvuSaveTclPathname( buffer );
    return tcl_main;
  }

  /* otherwise, we couldn't find the root Tcl file */

  return NULL;
}

char *
tvuGetSchemaUrl( void )
{
  static char buffer[MAX_DIR_LENGTH];
  char        *dir, *result = NULL;
  Purl        *purl;

  /* if no -schema command line value given, then nothing to return */

  if ( ! schema_url ) return NULL;

  /* do we have an absolute path/URL? */

  if ( strchr( schema_url, ':' ) != NULL ||
       schema_url[0] == '/' ||
       schema_url[0] == '\\' )
    result = schema_url;

  /* otherwise, relative filenames are relative to the user's config dir */
  
  if ( ! result ) {
    buffer[0] = '\0';
    if ( ( dir = tvuGetUserDir() ) != NULL )
      strcat( buffer, dir );
    tvuCatPath( buffer, schema_url );
    result = buffer;
  }

  /* libGeo requires local filenames to be in a file://localhost/ form. */

  if ( ! ( purl = tsmParseUrl( result ) ) ) return NULL;

  strcpy( buffer, purl->url );
  tsmFreePurl( purl );

  return buffer;
}

char *
tvuGetProfileUrl( void )
{
  static char buffer[MAX_DIR_LENGTH];
  char        *dir, *result = NULL;
  Purl        *purl;

  /* if no -profile command line value given, then use default */

  if ( ! profile_url ) {
    strcpy( buffer, "file://localhost/" );
    if ( ( dir = tvuGetUserDir() ) != NULL )
      tvuCatPath( buffer, dir );
    tvuCatPath( buffer, "profile.xml" );
    return buffer;
  }

  /* do we have an absolute path/URL? */

  if ( strchr( profile_url, ':' ) != NULL ||
       profile_url[0] == '/' ||
       profile_url[0] == '\\' )
    result = profile_url;

  /* otherwise, relative filenames are relative to the user's config dir */
  
  if ( ! result ) {
    buffer[0] = '\0';
    if ( ( dir = tvuGetUserDir() ) != NULL )
      strcpy( buffer, dir );
    tvuCatPath( buffer, profile_url );
    result = buffer;
  }

  /* libGeo requires local filenames to be in a file://localhost/ form. */

  if ( ! ( purl = tsmParseUrl( result ) ) ) return NULL;

  strcpy( buffer, purl->url );
  tsmFreePurl( purl );

  return buffer;
}

static void
tvuUpgradeUnixConfig( void )
{
#ifdef TVU_UNIX
  char        *configname = tvuGetConfigFilename();
  char        *viewname = tvuGetViewpointsFilename();
  char        *pathdir = tvuGetPathUrl( NULL );
  char        buffer[1024], *ptr, *licend;
  struct stat stat_struct;
  static char path[MAX_DIR_LENGTH], *homedir;
  FILE        *fp;

  /* does the $HOME/.tv/config.xml file exist? If so, then exit */

  if ( stat( configname, &stat_struct ) == 0 )
    return;

  /* get the $HOME path - where the old config files were stored */

  if ( ( homedir = getenv("HOME") ) == NULL )
    return;

  /* make the full pathname to the old .TerraVision config file */

  strcpy( path, homedir );
  tvuCatPath( path, ".TerraVision" );

  /* parse the .TerraVision file and extract the license_accepted */
  /* and database_url settings. Then save these in the new format */
  /* If .TerraVision doesn't exist, then no config files to copy. */

  if ( ( fp = fopen( path, "r" ) ) == NULL )
    return;

  while ( !feof( fp ) ) {
    fgets( buffer, 512, fp );
    if ( feof( fp ) ) break;
    ptr = buffer;
    if ( *(ptr++) != '*' ) continue;
    
    if ( strncmp( ptr, "license_accepted:", 17 ) == 0 ) {
      ptr += 17;
      while ( *ptr <= ' ' && *ptr != '\n' ) ptr++;
      if ( license_accepted ) free( license_accepted );
      license_accepted = strdup( ptr );
      for ( licend = license_accepted; *licend > ' '; licend++ );
      *licend = '\0';
      tvuAddConfig1( "license", license_accepted );
      
    } else if ( strncmp( ptr, "database_url:", 13 ) == 0 ) {
      char *start_ptr = ptr + 13, *end_ptr = buffer + strlen(buffer) - 1;
      while ( *start_ptr <= ' ' && *start_ptr != '\n' )
	start_ptr++;
      while ( *end_ptr <= ' ' )
	*(end_ptr--) = '\0';
      if ( database_url == NULL ) free( database_url );
      database_url = strdup( start_ptr );
      tvuAddConfig1( "database", start_ptr );
    }
    
  }
  fclose( fp );

  tvuSaveConfig();

  /* now copy the viewpoints file to the .tv directory */

  tvuLoadViewpoints_old( NULL );
  tvuSaveViewpoints();

  /* also copy any path files to the .tv/paths directory */

  sprintf( buffer, "cp -pr %s/.TerraVisionPaths %s", homedir, pathdir );
  system( buffer );
  
#endif
}

int
tvuLoadConfig( void )
{
  xmlDocPtr  doc;
  xmlNodePtr root, tree;
  char       *filename = tvuGetConfigFilename();

  if ( no_config ) return FALSE;

  /* open and parse the config.xml file */

  if ( ( doc = xmlParseFile( filename ) ) == NULL ) {
    tvuUpgradeUnixConfig();
    return FALSE;
  }

  /* get the root node and check that it is "TerraVision" */

  if ( ( root = xmlDocGetRootElement( doc ) ) == NULL )
    return FALSE;

  if ( tvuStrCmp( (char *) root->name, "TerraVision" ) == FALSE ||
       ( tree = root->xmlChildrenNode ) == NULL )
    return FALSE;

  /* walk through each entry and parse it */

  tree = tree->next;
  while ( tree ) {
    char *name = (char *) tree->name;
    
    if ( tvuStrCmp( name, "dataset" ) ) {
      char *oi = (char *) xmlGetProp( tree, "oi" );
      char *dem = (char *) xmlGetProp( tree, "dem" );
      if ( oi && dem ) {
	tvuAddConfig2( name, oi, dem );
	if ( numInitialOis < MAX_URLS )
	  initialOi[numInitialOis++] = oi;  /* already strdup'd */
	if ( numInitialDems < MAX_URLS )
	  initialDem[numInitialDems++] = dem;  /* already strdup'd */
      }
      
    } else if ( tvuStrCmp( name, "option" ) ) {
      char *optname = (char *) xmlGetProp( tree, "name" );
      char *optval  = (char *) xmlGetProp( tree, "value" );
      if ( optname && optval ) {
	char buffer[512];
	
	tvuAddConfig2( name, optname, optval );
	sprintf( buffer, "set('%s','%s')", optname, optval );
	
	if ( cmd_list_size < MAX_CMDS )
	  cmd_list[cmd_list_size++] = strdup( buffer );

	free( optname );
	free( optval );
      }
      
    } else if ( tvuStrCmp( name, "geovrml" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      if ( url ) {
	tvuAddConfig1( name, url );
      }

    } else if ( tvuStrCmp( name, "schema" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      if ( url ) {
	tvuAddConfig1( name, url );
	if ( schema_url ) free( schema_url );
	schema_url = url; /* strdup'd in xmlGetProp */
      }

    } else if ( tvuStrCmp( name, "database" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      if ( url ) {
	tvuAddConfig1( name, url );
	if ( database_url ) free( database_url );
	database_url = url; /* strdup'd in xmlGetProp */
      }

    } else if ( tvuStrCmp( name, "proxy" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      tvuAddConfig1( name, url );
      tsmSetHttpProxy( url );
      free( url );

    } else if ( tvuStrCmp( name, "viewpoint" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      if ( url ) {
	tvuAddConfig1( name, url );
	if ( viewpoint_url ) free( viewpoint_url );
	viewpoint_url = url; /* strdup'd in xmlGetProp */
      }

    } else if ( tvuStrCmp( name, "path" ) ) {
      char *url = (char *) xmlGetProp( tree, "url" );
      if ( url ) {
	tvuAddConfig1( name, url );
	if ( path_url ) free( path_url );
	path_url = url; /* strdup'd in xmlGetProp */
      }

    } else if ( tvuStrCmp( name, "license" ) ) {
      char *version = (char *) xmlGetProp( tree, "accepted" );
      if ( version ) {
	tvuAddConfig1( name, version );
	if ( license_accepted ) free( license_accepted );
	license_accepted = strdup( version );
	free( version );
      }

    } else if ( tvuStrCmp( name, "visual" ) ) {
      char *id = (char *) xmlGetProp( tree, "id" );
      if ( id ) {
	tvuAddConfig1( name, id );
	ogl_visual_id = atoi( id );
	free( id );
      }

    } else if ( tvuStrCmp( name, "oaa" ) ) {
      char *host = (char *) xmlGetProp( tree, "hostname" );
      if ( host ) {
	tvuAddConfig1( name, host );
	SetOAAHostPort( host );
	free( host );
      }

    } else if ( tvuStrCmp( name, "tcl" ) ) {
      char *tcldir = (char *) xmlGetProp( tree, "dir" );
      if ( tcldir ) {
	tvuSetTclFilename( tcldir );
	free( tcldir );
      }

    } else if ( tvuStrCmp( name, "command" ) ) {
      char *value = (char *) xmlGetProp( tree, "value" );
      if ( value ) {
	tvuAddConfig1( name, value );
	if ( cmd_list_size < MAX_CMDS )
	  cmd_list[cmd_list_size++] = value; /* strdup'd in xmlGetProp */
      }

    }

    tree = tree->next->next;
  }

  /* all done! */
  
  xmlFreeDoc( doc );

  return TRUE;
}

int
tvuSaveConfig( void )
{
  xmlDocPtr  doc;
  xmlNodePtr root, tree;
  char       *filename = tvuGetConfigFilename();
  char       *thedate = tsmDateString( (PRTime) 0 );
  int        i, bytes_written;

  if ( no_config ) return FALSE;

  xmlIndentTreeOutput = 2;    /* indent nested XML tags by 2 spaces */

  doc = xmlNewDoc( "1.0" );
  xmlDocSetRootElement( doc, xmlNewDocNode(doc, NULL, "TerraVision", NULL) );
  root = xmlDocGetRootElement( doc );
  xmlSetProp( root, "version", TERRAVISION_RELEASE );
  if ( thedate )
    xmlSetProp( root, "created", (xmlChar *) thedate );

  for ( i = 0; i < config_size; i++ ) {
    ConfigItem *item = config[i];

    if ( tvuStrCmp( (char *) item->option, "option" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "name", item->value );
      xmlSetProp( tree, "value", item->value2 );

    } else if ( tvuStrCmp( (char *) item->option, "dataset" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "oi", item->value );
      xmlSetProp( tree, "dem", item->value2 );

    } else if ( tvuStrCmp( (char *) item->option, "geovrml" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "schema" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "database" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "proxy" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "oaa" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "hostname", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "tcl" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "dir", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "command" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "value", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "viewpoint" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "path" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "url", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "license" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "accepted", item->value );

    } else if ( tvuStrCmp( (char *) item->option, "visual" ) ) {
      tree = xmlNewChild( root, NULL, item->option, NULL);
      xmlSetProp( tree, "id", item->value );

    }

  }

  tsmMakeDirIfNotExist( filename, TSM_PATH_AND_FILE );
  bytes_written = xmlSaveFile( filename, doc );

  xmlFreeDoc( doc );

  return ( bytes_written > 0 );
}
