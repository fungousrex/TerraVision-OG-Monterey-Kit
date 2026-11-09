/*
 * Name: tvlaunch.c
 *
 * Description:
 *     A launcher program that is used to run the TerraVision plugin
 *     under the Windows platform. It will ensure that a specific
 *     binary and/or browser combination are used.
 *
 *     See Usage() for command line parameter details.
 *
 * Author:
 *     Martin Reddy, <reddy@ai.sri.com> - 28 November 2000.
 *
 * Function List:
 *     int  main( int, char** )
 *
 * Revision Information:
 *
 *     $Id: tvlaunch.c,v 1.6.2.1 2001/03/03 22:16:55 reddy Exp $ 
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

#ifndef WIN32
#include <stdio.h>
int
main( int argc, char **argv )
{
  fprintf( stderr, "%s: This is a Windows-only utility.\n", argv[0] );
  return 0;
}
#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

#define TRUE  1
#define FALSE 0

#define VERSION 1.0

/*
 * Synopsis: int Message( char *format, ... )
 *
 * Returns: Display a printf style message in a message box window
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

void
Message( char *format, ... )
{
  va_list args;
  char message[2048];
  
  va_start( args, format );
  vsprintf( message, format, args );
  va_end( args );

  MessageBox( 0, message, "TerraVision Launcher", MB_OK );
}

/*
 * Synopsis: int IsSubString( char *string, char *substring )
 *
 * Returns: Do a case-insensitive check whether a string is contained within another
 *
 * Author: Martin Reddy
 * 
 * Date: 29 November 2000
 *
 */

int
IsSubString( char *string, char *substring )
{
  char *s1, *s2, *result = NULL;
  int  i, len;

  if ( string == NULL || substring == NULL ) 
    return FALSE;

  s1 = strdup( string );
  s2 = strdup( substring );
  
  if ( s1 && s2 ) {
    len = strlen( s1 );
    for ( i = 0; i < len; i++ ) s1[i] = tolower( s1[i] );
    len = strlen( s2 );
    for ( i = 0; i < len; i++ ) s2[i] = tolower( s2[i] );

    result = strstr( s1, s2 );
  }

  if ( s1 ) free( s1 );
  if ( s2 ) free( s2 );

  return ( result != NULL );
}

/*
 * Synopsis: int GetArgValue( LPSTR cmdlin, char *option )
 *
 * Returns: Return whether the specified option appears in the command line
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

int
GetArgValue( LPSTR cmdlin, char *option )
{
  return IsSubString( cmdlin, option );
}

/*
 * Synopsis: char *GetArgString( LPSTR cmdlin, char *option )
 *
 * Returns: Return the string following the specified command line option
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

char *
GetArgString( LPSTR cmdlin, char *option )
{
  char *found = strstr( cmdlin, option ), *end, *next, *result;
  int  quoted = FALSE, done = FALSE, size;

  if ( ! found ) return NULL;

  found += strlen( option );
  while ( *found <= ' ' && *found != '\0' )
    found++;

  if ( *found == '\0' ) {
    Message( "No value string provided for the %s option", option );
    exit( 1 );
  } else if ( *found == '\"' ) {
    found++;
    quoted = TRUE;
    if ( *found == '\"' )
       return strdup( "" );
  }

  end = found;
  while (TRUE) {
    next = end + 1;
    if ( quoted ) {
      if ( *next == '\"' || *next == '\0' ) break;
    } else {
      if ( *next <= ' ' ) break;
    }
    end++;
  };
  
  size = end - found + 1;
  if ( ( result = calloc( size+1, 1 ) ) == NULL ) return NULL;
  memcpy( result, found, size );

  return result;
}

/*
 * Synopsis: void Usage( void )
 *
 * Returns: Display the program usage in a message box window
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

void
Usage( void )
{
  Message( "TerraVision Launcher V%1.1f\n\n"
	   "Copyright (c) 2000 - SRI International. All rights reserved\n\n"
	   "Used to run the TerraVision plugin under Windows and ensure\n"
	   "that a specific binary and/or browser combination are used.\n\n"
	   "Options:\n"
	   "  -target <url>  =  the HTML or PPT file to open\n"
	   "  -options <opts>  =  string to pass to browser application\n"
	   "  -installdir <dir>  =  set the TV install dir for the NS plugin\n"
	   "  -activexdir <dir>  =  directory with Active X files to install\n"
	   "  -netscape  =  use the Netscape browser for HTML targets\n"
	   "  -explorer  =  use the IE browser for HTML targets\n"
	   "  -regpaths  =  display application paths from the registry\n"
	   "  -firsttime =  display a message if this is first time use\n"
	   "  -debug  =  display debug output during launch\n"
	   "  -help  =  display this usage information\n",
	   VERSION );
  exit( 1 );
}

/*
 * Synopsis: int SetInstallDir( char *install_dir, int debug )
 *
 * Returns: Set the TerraVision install directory in the registry
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

int
SetInstallDir( char *install_dir, int debug )
{
  char const *subkey = "Software\\SRI International\\TerraVision Plugin";
  HKEY        key;
  LONG        ret;

  /* try to get the TerraVision Plugin registry key */

  ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_ALL_ACCESS, &key );
  if ( ret == ERROR_SUCCESS ) {

    /* set the install directory string */

    RegSetValueEx( key, "InstallDirectory", 0, REG_SZ,
		   install_dir, strlen(install_dir)+1 );
    
    /* close the registry key that we just opened */

    RegCloseKey( key );

    if ( debug ) Message( "Set TerraVision Install Dir To:\n%s", install_dir );

    return TRUE;
  }

  return FALSE;
}

/*
 * Synopsis: char *PathToTerraVision( void )
 *
 * Returns: Return the install directory of TerraVision from the registry
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

char *
PathToTerraVision( void )
{
  char const *subkey = "Software\\SRI International\\TerraVision Plugin";
  HKEY        key;
  LONG        ret, key_type;
  static char install_dir[512] = "";
  long        length = 512;

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
}

/*
 * Synopsis: char *PathToNetscape( void )
 *
 * Returns: Return the command to run the default version of Netscape (4.x)
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

char *
PathToNetscape( void )
{
  char        subkey[512], ver[128];
  HKEY        key;
  LONG        ret, key_type;
  static char install_dir[512] = "";
  long        length = 512;

  /* try to get the Netscape 4.x registry key */

  strcpy( subkey, "Software\\Netscape\\Netscape Navigator" );
  ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_EXECUTE, &key );

  if ( ret != ERROR_SUCCESS ) {

    /* otherwise, try to get the Netscape 6.x registry key */
    
    strcpy( subkey, "Software\\Netscape\\Netscape 6" );
    ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_EXECUTE, &key );
  }

  if ( ret == ERROR_SUCCESS ) {

    /* extract the install directory string if present */

    RegQueryValueEx( key, "CurrentVersion", NULL, &key_type, ver, &length );

    strcat( subkey, "\\" );
    strcat( subkey, ver );
    strcat( subkey, "\\Main" );
    
    /* close the registry key that we just opened and open the next one */

    RegCloseKey( key );

    ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_EXECUTE, &key );
    if ( ret == ERROR_SUCCESS ) {

      /* extract the install directory string if present */

      length = 512;
      RegQueryValueEx( key, "Install Directory", NULL, &key_type, 
		       install_dir, &length );

      strcat( install_dir, "\\Program\\netscape.exe" );

      /* close the registry key that we just opened and open the next one */
      
      RegCloseKey( key );
    }

  }

  return install_dir;
}

/*
 * Synopsis: char *PathToPowerpoint( void )
 *
 * Returns: Return the command to run PowerPoint from the registry
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

char *
PathToPowerpoint( void )
{
  char const *subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\"
                       "App Paths\\PowerPnt.exe";
  HKEY        key;
  LONG        ret, key_type;
  static char install_dir[512] = "";
  long        length = 512;

  /* try to get the PowerPoint install directory registry key */

  ret = RegOpenKeyEx( HKEY_LOCAL_MACHINE, subkey, 0, KEY_EXECUTE, &key );
  if ( ret == ERROR_SUCCESS ) {

    /* extract the install directory string (default) */

    RegQueryValueEx( key, NULL, NULL, &key_type, install_dir, &length );
    
    /* close the registry key that we just opened */

    RegCloseKey( key );
  }
  
  return install_dir;
}

/*
 * Synopsis: void Usage( void )
 *
 * Returns: Return a command to run Internet Explorer
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

char *
PathToExplorer( void )
{
  return "explorer.exe";
}

/*
 * Synopsis: void DisplayPaths( void )
 *
 * Returns: Display the paths to various applications from the registry
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

void
DisplayPaths( void )
{
  char *tv = PathToTerraVision();
  char *ns = PathToNetscape();
  char *ie = PathToExplorer();
  char *pp = PathToPowerpoint();

  Message( "TerraVision Launcher V%1.1f\n\n"
	   "Copyright (c) 2000 - SRI International. All rights reserved\n\n"
	   "Found the following paths from your registry:\n\n"
	   "TerraVision = %s\n"
	   "Netscape = %s\n"
	   "Explorer = %s\n"
	   "Powerpoint = %s\n",
	   VERSION, tv, ns, ie, pp );
  exit( 1 );
}

/*
 * Synopsis: int Command( char *cmdstring, int debug )
 *
 * Returns: Spawn a command in a new process with optional debug output
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

int
Command( char *cmdstring, int debug )
{
  STARTUPINFO         startupInfo;
  PROCESS_INFORMATION procInfo;

  ZeroMemory(&startupInfo, sizeof(startupInfo));

  if ( debug ) Message( "Creating Process:\n%s", cmdstring );
  
  return CreateProcess( NULL, cmdstring, NULL, NULL, FALSE, 0, NULL, NULL, 
			&startupInfo, &procInfo );
}

/*
 * Synopsis: int WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
 *
 * Returns: program integer return code
 *
 * Author: Martin Reddy
 * 
 * Date: 28 November 2000
 *
 */

int WINAPI
WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdlin, int iCmdShow )
{
  int  use_netscape = TRUE, result = 0, debug = FALSE;
  char *target, *options, *instdir, *actxdir;
  char command[4096];

  /* parse the command line arguments */

  if ( GetArgValue( cmdlin, "-help" ) || ! GetArgValue( cmdlin, "-" ) )
    Usage();

  if ( GetArgValue( cmdlin, "-regpaths" ) )
    DisplayPaths();

  if ( GetArgValue( cmdlin, "-explorer" ) ) use_netscape = FALSE;
  if ( GetArgValue( cmdlin, "-netscape" ) ) use_netscape = TRUE;
  if ( GetArgValue( cmdlin, "-debug" ) ) debug = TRUE;
  if ( GetArgValue( cmdlin, "-firsttime" ) && use_netscape )
    Message( "About to run TerraVision inside of Netscape...\n\n"
	     "If you are already running Netscape, then you should\n"
	     "close the browser down by selecting the File/Exit\n"
	     "menu option. This will ensure that the TerraVision\n"
	     "plugin will be found by Netscape when it next runs.\n\n"
             "Click on the OK button below when you are ready." );

  target  = GetArgString( cmdlin, "-target" );
  options = GetArgString( cmdlin, "-options" );
  instdir = GetArgString( cmdlin, "-installdir" );
  actxdir = GetArgString( cmdlin, "-activexdir" );

  /* set the TerraVision install directory if asked to */
  
  if ( instdir )
    SetInstallDir( instdir, debug );

  if ( actxdir )
    Message( "ActiveX TerraVision installation not current supported" );

  /* fire up PowerPoint or Netscape appropriately */

  if ( target ) {

    if ( IsSubString( target, ".ppt" ) || IsSubString( target, ".pps" ) ) {

      /* Display a PowerPoint slideshow */

      sprintf( command, "%s \"%s\" %s", PathToPowerpoint(), target,
	       ( options ) ? options : "" );

    } else if ( IsSubString( target, ".htm" ) ) {

      /* Display an HTML page */

      sprintf( command, "%s \"%s\" %s",
	       ( use_netscape ) ? PathToNetscape() : PathToExplorer(),
	       target,
	       ( options ) ? options : "" );

    } else {

      /* Run the specified command with the desired options */

      sprintf( command, "\"%s\" %s", target, ( options ) ? options : "" );

    }

    result = Command( command, debug );
  }

  /* clean up */

  if ( target ) free( target );
  if ( options ) free( options );
  if ( instdir ) free( instdir );
  if ( actxdir ) free( actxdir );

  return result;
}
#endif

/*** EOF: tvlaunch.c ***/
