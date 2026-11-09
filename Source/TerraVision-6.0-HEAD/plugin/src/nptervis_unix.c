/*
 * Filename: nptervis_unix.c
 *
 * Purpose:
 *   Contains the implementation for the UNIX specific parts of the
 *   TerraVision Netscape plugin.
 *
 * Author:
 *   Martin Reddy, SRI International.
 *
 * Revision:
 *   $Id: nptervis_unix.c,v 1.6 2000/08/07 22:11:11 reddy Exp $
 *
 * (c) Copyright 1993-1999. SRI International. All rights reserved.
 *
 */

#ifdef XP_UNIX

#include "nptervis_all.h"

#define PLUGIN_NAME          "TerraVision Plug-in"
#define PLUGIN_DESCRIPTION   "Plug-in for web-based terrain visualization"

/*
 * Synopsis:
 *   char *GetTerraVisionBinaryName( void )
 *
 * Description:
 *   Tries to find a version of the TerraVision plugin on a Unix
 *   machine that we can run.
 *
 *   The user should NOT free the returned string.
 *
 * Returns: string containing the full pathname of the TV binary
 *
 * Author: Martin Reddy
 *
 * Date: 7 August 2000
 *
 */

char *
GetTerraVisionBinaryName( void )
{
  int i;
  static char *binaries[] = {

    /* hard-wired Unix locations */

    "/homedir/magic/bin/TerraVisionPlugin",
    "/homedir/reddy/magic/TerraVision/irix6.3-o32-nodpss/PluginTV/TerraVision",

    NULL
  };

  /* scan through list of hardwired binary locations for a TerraVision exe */

  for ( i = 0; binaries[i]; i++ )
    if ( FileExists( binaries[i] ) )
      return binaries[i];

  return NULL;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_GetMIMEDescription:
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

char *
NPP_GetMIMEDescription( void )
{
  return( "application/x-terravision:tvs:TerraVision Plug-in" );
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * NPP_GetValue:
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

NPError
NPP_GetValue( void *future, NPPVariable variable, void *value )
{
  if ( variable == NPPVpluginNameString )
    *((char **)value) = PLUGIN_NAME;
  else if ( variable == NPPVpluginDescriptionString )
    *((char **)value) = PLUGIN_DESCRIPTION;
  else
    return NPERR_GENERIC_ERROR;
  
  return NPERR_NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * PlatformHandleEvent
 *
 * Handle platform-specific events.
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

int16
PlatformHandleEvent( PluginInstance *plugin, void *event )
{
  return 0;   /* UNIX Plugins do not use HandleEvent */
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * PlatformNew
 *
 * Initialize any Platform-Specific instance data.
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

void
PlatformNew( PluginInstance *plugin )
{
  DebugMessage( plugin, "PlatformNew" );
  
  plugin->pattr = NULL;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * PlatformDestroy
 *
 * Destroy any Platform-Specific instance data.
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

NPError
PlatformDestroy( PluginInstance *plugin )
{
  DebugMessage( plugin, "PlatformDestroy" );
  return NPERR_NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++
 * PlatformEventLoop
 *
 * Install the event loop to catch plugin messages
 +++++++++++++++++++++++++++++++++++++++++++++++++*/

void
PlatformEventLoop( PluginInstance *plugin, NPWindow *window )
{
  DebugMessage( plugin, "PlatformEventLoop" );
}

static char *
IntToStr( int value )
{
  char buffer[64];
  sprintf( buffer, "%d", value );
  return strdup( buffer );
}

int
TalkToTerraVision( PluginInstance *plugin, char const *message )
{
  static char const *newline = "\n";

  if ( plugin == NULL || plugin->pipe0_in == NULL ) return FALSE;

  DebugMessage( plugin, "Sending message: %s", message );

  if ( PR_Write( plugin->pipe0_in, message, strlen(message) ) >= 0 ) {
    PR_Write( plugin->pipe0_in, newline, 1 );
    return TRUE;
  } else
    return FALSE;
}

int
RunTerraVision( PluginInstance *plugin, const char *prog, int winid,
		int width, int height )
{
  char const  *argv[100];
  int         n = 0, i;
  PRProcess   *process;
  PRFileDesc  *pipe0_out, *pipe1_in;

  /* create the null-terminated argv list for the command */

  argv[n] = strdup( prog ); n++;
  argv[n] = strdup( "-nswinid" ); n++;
  argv[n] = IntToStr( winid ); n++;
  argv[n] = strdup( "-width" ); n++;
  argv[n] = IntToStr( width ); n++;
  argv[n] = strdup( "-height" ); n++;
  argv[n] = IntToStr( height ); n++;

  /* add the command line options from the EMBED tag */

  for ( i = 0; i < plugin->argc; i++ ) {
    argv[n] = strdup( plugin->argv[i] ); n++;
  }

  argv[n] = NULL;

  /* create a pipe for stdin (pipe0) and stdout (pipe1) */

  PR_CreatePipe( &pipe0_out, &plugin->pipe0_in );
  PR_CreatePipe( &plugin->pipe1_out, &pipe1_in );

  /* create a process attribute structure with our new pipes */

  if ( ( plugin->pattr = PR_NewProcessAttr() ) == NULL )
    return FALSE;

  PR_ProcessAttrSetStdioRedirect( plugin->pattr, 0, pipe0_out );
  PR_ProcessAttrSetStdioRedirect( plugin->pattr, 1, pipe1_in );

  /* execute the new process with our process atrributes */

  for ( i = 0; i < n; i++ )
    DebugMessage( plugin, "Arg[%d] = %s", i, argv[i] );

  process = PR_CreateProcess( prog, (char *const *) argv, NULL, 
			      plugin->pattr );

  PR_DetachProcess( process );

  PR_Close( pipe0_out );
  PR_Close( pipe1_in );

  TalkToTerraVision( plugin, "say hello terravision" );

  return ( process != NULL );
}

#endif /* XP_UNIX */

