/*
 * Name: TerraVision.c
 *
 * Description: TerraVision: The code.
 *
 * Function List:
 *
 * Dependencies:
 *
 *  ----- BEGIN LICENSE BLOCK -----
 *  Version: MPL 1.1
 * 
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 * 
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 * 
 *  The Original Code is the TerraVision 3D Terrain Visualization System.
 * 
 *  The Initial Developer of the Original Code is SRI International.
 *
 *  Portions created by the Initial Developer are Copyright (C) 1992-2002
 *  the Initial Developer. All Rights Reserved.
 * 
 *  Contributor(s): Yvan Leclerc, Lee Iverson, Martin Reddy, Nat Bletter. 
 *  ----- END LICENSE BLOCK -----
 *
 */

/* EXTERN INCLUDE FILES */
#include "TerraVisionInc.h"

#ifdef WIN32
#include "Platform_win.h"
#else
#include "Platform_X.h"
#endif

#include "Features.h"
#include "OAA.h"
#include "Texture.h"
#include "ThreeDWidget.h"
#include "ThreeDWidgetI.h"
#include "TsGeoPyramid.h"

#include <tvutils/tvutils.h>

#include <glt/glt.h>

#include <prinit.h>
#include <prthread.h>
#include <prcvar.h>
#include <prproces.h>
#include <pratom.h>

#include <assert.h>
#include <sys/types.h>

/* GLOBAL VARIABLES */

ThreeDWidget *pluginWidget = NULL;
int          exit_flag = FALSE;

/* LOCAL VARIABLES */

static int       exit_return = 0;
static PRLock    *exit_lock = NULL;
static PRCondVar *exit_cond = NULL;

static PRThread  *main_thread = NULL;
static PRThread  *ipc_thread = NULL;

/* LOCAL FUNCTION PROTOTYPES */

static void TerraVisionInitialLoad( void );
static void TerraVisionIPCInput (void *arg);
static void TerraVisionGetPaths(void);

/*
 * Synopsis:
 *   int main( int argc, char *argv[] )
 *
 * Description:
 *   Main for the TerraVision Plugin
 *
 * Returns: TRUE for normal exit, otherwise a FALSE.
 *
 * Author: Martin Reddy
 *
 * Date: 5 January 2000
 *
 */

#ifdef WIN32
int WINAPI
WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdlin, int iCmdShow )
#else
int
main( int argc, char *argv[] )
#endif
{
  static const char funcName[] = "main";
  PRThread *graphics;
  
#ifdef HAVE_SETLINEBUF
  setlinebuf (stdout);
  setlinebuf (stderr);
#endif

  /* initialise the tvutils library */

  tvuInit( TVU_PLUGIN );

  /* parse the config file and command line arguments */

#ifdef WIN32
  tvuParseArgString( cmdlin );
#else
  tvuParseArgArray( argc, argv );
#endif

  if ( tvuGetInt( TVU_ARGS_PARSED ) < tvuGetInt( TVU_TOTAL_ARGS ) ) {
    printf( "TerraVision: unrecognized option(s) specified\n"
	    "Try TerraVision -h for valid options.\n\n" );
    return FALSE;
  }

  /* output trace messages with the install and user directories */

  TSMTRACE(( INIT_MASK|1, funcName, "Install Dir = %s",
	     tvuGetString( TVU_INSTALL_DIR ) ));
  TSMTRACE(( INIT_MASK|1, funcName, "User Dir = %s",
	     tvuGetString( TVU_USER_DIR ) ));

  /* create our ThreeDWidget state structure */

  if ( ( pluginWidget = CreateThreeDWidget() ) == NULL ) {
    fprintf( stderr, "Could not create a ThreeDWidget!\n" );
    return FALSE;
  }

  /* Get the texture directory */

  TerraVisionGetPaths();

  /* get the window id to use for drawing */

  PlatformSetupGraphics( pluginWidget );

  /* MessageBox( 0, "Breakpoint reached", "TerraVision", MB_OK ); */

  /* load the user's viewpoint bookmarks file */

  tvuLoadViewpoints();
  
  /* do any initialisation required for supporting VRML feature models */

  FeatureInit();

  /* initialise the TsLog stuff in order to do net throughput logging */

  CreateTsLog();

  /* Initialize thread handling */

  main_thread = PR_GetCurrentThread ();
  pthr_set_name ((void *)main_thread, "TerraVision Plugin");

  /* we need to pop the window up here for X */

  if ( ! pluginWidget->is_plugin )
    PlatformShowWindow( pluginWidget );

  /* use a callback to load the datasets and set up the display */
  /* so that this all happens in a different thread.            */

  PlatformTimedFunc( pluginWidget, TerraVisionInitialLoad, 50 );
  
  /* create a dedicated thread to receive commonds on standard input */
  if ( ! tvuGetInt( TVU_NO_IPC ) ) {
    ipc_thread = 
      PR_CreateThread( PR_USER_THREAD, TerraVisionIPCInput, pluginWidget,
		       PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
		       PR_JOINABLE_THREAD, 1<<16 );
  }

  /* create the exit lock and condition */

  exit_lock = PR_NewLock ();
  exit_cond = PR_NewCondVar (exit_lock);

  /* Go into the main loop... */

#ifdef WIN32

  while ( ! exit_flag ) {
    MSG msg;

    /* PeekMessage must be in the same thread the window was created in */

    if ( PeekMessage( &msg, pluginWidget->platform.hWnd, 0, 0, PM_REMOVE ) ) {
      if ( msg.message == WM_QUIT ) {
	TerraVisionQuit ();
	break;
      } else {
	TranslateMessage( &msg );
	DispatchMessage( &msg );
      }
    }

    /* implement our timer callbacks */
    
    glmWRunWorkProcs();
 
    /* sleep for 1 ms so that we are not running busy */

    Sleep( 1 );
  }

#else

  /* create a dedicated thread to handle the graphics event loop */
  graphics =
    PR_CreateThread( PR_USER_THREAD, PlatformGraphicsLoop, pluginWidget,
		     PR_PRIORITY_HIGH, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD,
		     1<<16 );

  PR_Lock (exit_lock); {
    while (! exit_flag) {
      TSMTRACE(( INIT_MASK|1, funcName, "Wait for Renderer"));
      PR_WaitCondVar (exit_cond, PR_INTERVAL_NO_TIMEOUT);
    }
  }; PR_Unlock (exit_lock);

  TSMTRACE(( INIT_MASK|1, funcName, "Join Renderer"));
  PR_JoinThread (graphics);
#endif

  if (ipc_thread) {
    TSMTRACE(( INIT_MASK|1, funcName, "Join IPC"));
    PR_JoinThread (ipc_thread);
  }

  PR_DestroyCondVar (exit_cond);
  PR_DestroyLock (exit_lock);

  if (0 && PR_Cleanup () != PR_SUCCESS) {
    TSMTRACE(( INIT_MASK|1, funcName, "PR_Cleanup failed"));
  }

  return exit_return;
}

static void
LoadThread (void *closure)
{
  static char const funcName[] = "LoadThread";

  TsGeoPyramid *geo;
  int i, load_errors, datasets = tvuGetInt( TVU_NUM_DATASETS );
  
  pthr_set_name (PR_GetCurrentThread (), "Load");
  TSMTRACE ((INIT_MASK|1, funcName, "Starting"));

  /* load the requested datasets */

  for ( i = load_errors = 0; i < datasets; i++ ) {

    if ( ! (geo = TsGeoCreate_OIDEM (tvuGetStringN( TVU_OI_URL, i), NULL,
				     tvuGetStringN( TVU_DEM_URL, i), NULL))) {

      TSMTRACE(( THREED_MASK|1, "Could not load dataset, ",
		 tvuGetStringN( TVU_OI_URL, i ) ));
      load_errors++;
      
    } else if ( pluginWidget ) {
      ThreeDWidgetAddDataset (pluginWidget, geo);
    }
  }

  if ( datasets && load_errors == datasets ) {
    tvSetString (pluginWidget->threeDCore, TV_SPLASH_MESSAGE,
		 "Failed to load dataset");
    return;
  }

  /* load any path specified by the -path option, and start playing it */

  PathLoadInitial( pluginWidget, TRUE );

  /* perform any -cmd command line option commands */

  for ( i = 0; i < tvuGetInt( TVU_NUM_COMMANDS ); i++ )
    tvInterpret( pluginWidget->threeDCore, tvuGetStringN( TVU_COMMAND, i) );
}

static void
TerraVisionInitialLoad( void )
{
  int init_mode_3d = tvuGetInt( TVU_INITIAL_VIEWER ) != 2;

  /* Create the 3-D window structure */

  if ( ! InitThreeDWidget( pluginWidget, init_mode_3d ) ) {
    fprintf( stderr, "Unable to initialise the 3D widget!" );
    return;
  }

  /* specify the .geo schema URL before opening the viewer */

  tvuInitCore( pluginWidget->threeDCore );

  /* load in the initial set of specified datasets */

  PR_CreateThread( PR_USER_THREAD, LoadThread, pluginWidget,
		   PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_UNJOINABLE_THREAD,
		   1<<16 );

  /* start rendering */

  if (list_count (geoList) > 0)
    ThreeDWidgetStartRendering (pluginWidget);
}


void
TerraVisionExit (int ret)
{
  static char const funcName[] = "TerraVisionExit";

  PR_Lock (exit_lock); {
    exit_flag = 1;
    exit_return = ret;
    TSMTRACE ((INIT_MASK|1, funcName, "Exit flagged return %d", ret));
    PR_NotifyAllCondVar (exit_cond);
  }; PR_Unlock (exit_lock);

  if (ipc_thread) {
    TSMTRACE ((INIT_MASK|1, funcName, "Interrupt IPC"));
    PR_Interrupt (ipc_thread);
  }
}

/*
 * Synopsis: char * PR_ReadLine (PRFileDesc *fp, char *buf, int len)
 *
 * Description:
 *   Read a line of text from an NSPR file descriptor, e.g. PR_STDIN.
 *   We do a PR_Poll before going into the PR_Read so that we do not
 *   block uninterruptably inside PR_Read. This function will strip
 *   any end-of-line characters from the string.
 *
 * Returns: pointer to string or NULL if failed
 *
 * Author: Martin Reddy
 * 
 * Date: 8 November, 2000.
 *
 */

char *
PR_ReadLine (PRFileDesc *fp, char *buf, int len)
{
  static char const funcName[] = "PRFGets";
  int               curlen = 0;
  char              *dst = buf, val;
  PRPollDesc        pds;

  *dst = '\0';
  do {

    /* first of all we want to poll for a read character because */
    /* the PR_Read call is blocking and non-interruptable.       */

    pds.fd = fp;
    pds.in_flags = PR_POLL_READ;
    TSMTRACE ((INIT_MASK|1, funcName, "Poll(%d)", fp));
    for (;;) {
      int n = PR_Poll (&pds, 1, PR_MillisecondsToInterval (100));

      if (n > 0)
	break;
      else if (n < 0)
	return NULL;
      else if (exit_flag)
	return NULL;
    }

    /* okay, we definitely have a character to read now */

    TSMTRACE ((INIT_MASK|1, funcName, "Read(%d)", fp));
    if ( PR_Read( fp, dst, 1 ) <= 0 )
      return NULL;

    val = *(dst++);
    curlen++;
  } while ( val != '\n' && curlen < len-1  );

  /* null terminate and strip end-of-line marker(s) */

  *dst = '\0';
  while ( dst > buf && ( *(dst-1) == '\n' || *(dst-1) == '\r' ) )
    *(--dst) = '\0';

  return buf;
}

/*
 * Synopsis: static void TerraVisionIPCInput(ThreeDWidget)
 *
 * Description: Get commands from stdin and process them
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 15 December, 1999
 *
 */

static void
TerraVisionIPCInput (void *arg)
{
  char const   funcName[] = "TerraVisionIPCInput";
  ThreeDWidget *pluginWidget = (ThreeDWidget *) arg;
  char         buffer[2048];

  pthr_set_name (PR_GetCurrentThread (), "IPC");
  TSMTRACE ((INIT_MASK|1, funcName, "Starting"));

  /* LOOP AWAY */

  while (! exit_flag) {
    char *cmd = PR_ReadLine (PR_STDIN, buffer, 2048);

    if (! cmd) break;
    if ( strlen( cmd ) == 0 ) continue;

    TSMTRACE ((INIT_MASK|1, funcName, "IPC: \"%s\"\n", cmd));

    /* interpret the command that we have just received */

    if ( ! strcmp( cmd, "quit" ) ) {
      /* Doing a PR_ProcessExit seems to be more robust than calling */
      /* TerraVisionQuit() for when the plugin is inside a web page. */
      PR_ProcessExit( 0 );
      /* TerraVisionQuit(); */
    } else if ( pluginWidget && pluginWidget->threeDCore ) {
      tvInterpret( pluginWidget->threeDCore, cmd );
    }
  }

  TSMTRACE ((INIT_MASK|1, funcName, "Exiting"));
}

/*
 * Synopsis: int TerraVisionGetPaths()
 *
 * Description: Get the URL's for the buttons
 *     out of the configuration file
 *
 * Returns: TRUE on success
 *          FALSE on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 */

void
TerraVisionGetPaths( void )
{
  char texture_path[1024];
  char *magicHome = tvuGetString( TVU_INSTALL_DIR );

  /* FIND WHERE THE TEXTURES ARE */

  if ( magicHome ) {
    sprintf( texture_path, "%s/etc/textures", magicHome );
    TextureSetPath( texture_path );
  }
}

/*
 * Synopsis: TerraVisionRemoveGeoPyramid(void)
 *
 * Description: This function is called when the 
 *    user selects a different geo pyramid.
 *
 * Externals: tsm - the connection to the tsm
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: December 20, 1994
 *
 */

static void
TerraVisionRemoveGeoPyramid(TsGeoPyramid *geo)
{
  static char * funcName = "TerraVisionRemoveGeoPyramid";

  TSMTRINC ((INIT_MASK|2, 1));
  TSMTRACE ((INIT_MASK|2, funcName, "Entering."));

  if (! geo) return;

  /* POP DOWN THE VIEWS */
  
  tvRemoveDataset( pluginWidget->threeDCore, geo );
    
  TSMTRACE ((INIT_MASK|2, funcName, "Done dismissing windows."));

  /* TOGGLE SOME WIDGETS */
  TSMTRACE ((INIT_MASK|2, funcName, "Resetting buttons."));
  /* TELL THE TILE TABLES OF THE CHANGE */
  TSMTRACE ((INIT_MASK|2, funcName, "Resetting data sets."));
  TsGeoDestroy (geo);

  TSMTRACE ((INIT_MASK|2, funcName, "Done."));
  TSMTRINC ((INIT_MASK|2, -1));
}

/*
 * Synopsis: TerraVisionQuit(void)
 *
 * Description: This function should be 
 *     called when TerraVision exits normally
 *     Things are closed off and killed.
 *
 * Returns: Nothing - program exits
 *
 * Author: Stephen Lau
 * 
 * Date: April 1995
 *
 */

void
TerraVisionQuit(void)
{
  static char *funcName = "TerraVisionQuit";

  ThreeDWidget *w = pluginWidget;

  TSMTRINC ((INIT_MASK|1, 1));
  TSMTRACE ((INIT_MASK|1, funcName, "Entering."));

  /* destroy our ThreeDWidget structure */

  assert (sizeof (ThreeDWidget *) == sizeof (PRInt32));
  if (! (w = (ThreeDWidget *) PR_AtomicSet ((PRInt32 *) &pluginWidget, 0)))
    return;

  DestroyThreeDWidget (w);
  
  /* N.B. cleaning up the geoList here causes problems */
  /* it looks like the reader thread is still running? */
  /* doing a tsmCleanupHttp() here has the same probs. */

  TSMTRACE ((INIT_MASK|1, funcName, "Done."));
  if (geoList) {
    while (! list_empty (geoList)) {
      TsGeoPyramid *geo = list_pop (geoList);
      TsGeoDestroy (geo);
    }
  }
  TSMTRINC ((INIT_MASK|1, -1));

  /* clean up all of the tvutils state (config, viewpoints, etc.) */

  tvuFinish();

#ifdef WIN32
  /* And exit the application... */
  PR_ProcessExit( 0 );
#else
  TerraVisionExit (0);
#endif
}

