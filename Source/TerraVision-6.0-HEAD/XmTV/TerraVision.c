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

#define tvExtern

/* EXTERN INCLUDE FILES */
#include "TerraVisionInc.h"

#include "AboutDialog.h"
#include "BusyDialog.h"
#include "Database.h"
#include "Datasets.h"
#include "Dialogs.h"
#include "Features.h"
#include "Info.h"
#include "Link.h"
#include "NetLogger.h"
#include "NetLogCore.h"
#include "OAA.h"
#include "Prefs.h"
#include "ScannedMap.h"
#include "Texture.h"
#include "ThreeDWidgetI.h"
#include "TsGeoPyramid.h"
#include "TVLogger.h"
#include "Viewpoint.h"

#include <glt/glt.h>

#include <prinit.h>
#include <prthread.h>
#include <prcvar.h>

#include <pthr/list.h>

#include "X11Include.h"

#include <X11/cursorfont.h>

#include <Xm/CascadeB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>

#include <X11/keysym.h>
#include <Xm/MenuShell.h>

#include <assert.h>

#include <unistd.h>
#include <prio.h>

#ifdef AVS
#include <avs/avs.h>
#include <avs/field.h>
#include "vr_util.h"

int TerraVisionDesc (void);
static AVSfield *vr_input;
#endif

#include <sys/types.h>
#include <sys/resource.h>

#ifdef HAVE_INVENT_H
#include <invent.h>
#endif

static Display *tvDisplay;  
static Screen *tvScreen;
static XVisualInfo tvVisualInfo;

static PRThread *main_thread;

#include <tsm/tsm.h>
#include <tvutils/tvutils.h>

typedef struct TVDataSet {
  char const          *name;         /* the geoset name for this OI+DEM */
  char const          *oi_url;	     /* URL of command-line OI. */
  TsmConnectionParams oi_params;
  char const          *dem_url;	     /* URL of command-line DEM. */
  TsmConnectionParams dem_params;
} TVDataSet;

extern struct TsGeoPyramid *TVDataSetConnect(TVDataSet * tsm);

/* LOCAL FUNCTION PROTOTYPES */
static void SigCb(int);
static void SystemInit (void);
static void TerraVisionChooseVisuals (Display *disp, Screen *scr);
static XtAppContext TerraVisionInitialize(int *argcp, char *argv[]);
static XtAppContext TerraVisionInitGraphics(int *, char * []);
static int  TerraVisionXError (Display *disp, XErrorEvent *ev);
static int  TerraVisionXIOError (Display *disp);
static void TerraVisionGraphicsLoop(void *);
static int  TerraVisionGetURLPaths(void);
static void TerraVisionCreateStatusBar(Widget, Widget);
static void TerraVisionToggleChangedCB(Widget, XtPointer, XtPointer);
static void TerraVisionDebugLevelCB(Widget, XtPointer, XtPointer);
static void TerraVisionDebugMaskCB(Widget, XtPointer, XtPointer);
static void TerraVisionCreateMenuBar(Widget);
static void TerraVisionOverrideShellDefaults (XVisualInfo *v);
static int OpenInitialWindow( void );

void TerraVisionButtonCB(Widget, XtPointer, XtPointer);

static ThreeDWidget *TerraVisionDisplayTwoDWidget( void );
static ThreeDWidget *TerraVisionDisplayThreeDWidget( void );

static TVDataSet *TVDataSetCreate( char const *oi_url, char const *dem_url );
static void TVDataSetLoad( TVDataSet *tsm );
static int TVDataSetLoadNext( void );
static void TVDataSetLoadSuccess( void *closure );
static void TVDataSetLoadFailure( void *closure );
static void TVDataSetLoadThread( BusyDialog *busy );

static XtTimerCallbackProc TerraVisionTimerCallback(XtPointer, XtIntervalId *);
#ifdef AVS
static XtTimerCallbackProc AVSTimerCallback(XtPointer, XtIntervalId *);
#endif

/* LOCAL VARIABLES */
static XtIntervalId  widgetTimer;    /* Timer for misc widget functions */
#ifdef AVS
static XtIntervalId  avsTimer;       /* Timer for avs device */
#endif

static AboutDialogWidget *aboutDialog;	/* Widget to talk about TerraVision */

static int isLogging = FALSE;
static long systemMemory;		 /* Main memory of system (if >0) */
static int  currInitialSet;                 /* current DEM/OI to load */

static int exit_flag;
static int exit_return;
static PRLock *exit_lock;
static PRCondVar *exit_cond;

/*
 * LOCAL WIDGET MANIA!
 * WIDGETS, WIDGETS EVERYWHERE...
 *
 */

/* MENU WIDGETS */
static Widget menuBar;
static Widget menuOpenTVButton;
static Widget menuCloseTVButton;
static Widget menuQuitButton;
static Widget menuInfoButton;
static Widget menuAboutButton;
static Widget menuLicenseButton;

Widget menuMapButton;
Widget menuThreeDButton;
Widget menuTwoDButton;
Widget menuCaveButton;

/* LOGGING WIDGETS */
static Widget menuTVLoggingStartButton;
static Widget menuTVLoggingStopButton;
static Widget menuTVLoggingReaderButton;
static Widget menuTVLoggingRequestButton;
static Widget menuTVLoggingVisibleButton;
static Widget menuTVLoggingPredictButton;
static Widget menuTVLoggingRenderButton;
static Widget menuTVLoggingCacheButton;
static Widget menuTVLoggingTextureCacheButton;
static Widget menuTVLoggingDirectoryButton;
static Widget menuTVLoggingCleanButton;
static Widget menuTVLoggingGraphsButton;

/* DEBUGGING LEVEL WIDGETS */
static Widget menuTVDebugLevel0Button;
static Widget menuTVDebugLevel1Button;
static Widget menuTVDebugLevel2Button;
static Widget menuTVDebugLevel3Button;
static Widget menuTVDebugLevel4Button;

/* DEBUGGING MASK WIDGETS */
static Widget menuTVDebugDotGeoButton;
static Widget menuTVDebugTilesButton;
static Widget menuTVDebugTextureButton;
static Widget menuTVDebugInitButton;
static Widget menuTVDebug2DButton;
static Widget menuTVDebug3DButton;
static Widget menuTVDebugMapButton;
static Widget menuTVDebugOpenGLButton;
static Widget menuTVDebugVizButton;
static Widget menuTVDebugOAAButton;
static Widget menuTVDebugLogButton;
static Widget menuTVDebugCacheButton;

/* PREFS MENU WIDGETS */
static Widget menuDPSSPrefsButton;

/* HELP MENU WIDGETS */
static Widget menuHelpGuideButton;
static Widget menuHelpHomeButton;
static Widget menuHelpFAQButton;
static Widget menuHelpOverviewButton;

/* DIALOG WIDGETS */
Widget warningDialog;

/* STATUS BAR */
Widget gatsStatusFieldWidget;
Widget tsmStatusFieldWidget;

extern Widget applicationShell;

/* BUSY DIALOG BOX */
Widget busyLabelWidget;
Widget busyWidget;
Cursor busyCursor;

/* DIALOG BOXES */
static Widget quitDialog;

/* STRINGS TO DETERMINE PLATFORM */

/*
 *      Widget and non-widget resources if the application defaults
 *      file can't be found.
 *      [ Generated automatically from TerraVision.ad. ]
 */
static String fallbackResources[] = {
#include "TerraVision.ad.h"
        NULL
};

/*
 * Synopsis:
 *   int main( int argc, char *argv[] )
 *
 * Description:
 *   Main for TerraVision
 *
 * Externals:
 *
 * Returns:
 *    Upon successful completion returns TRUE otherwise a FALSE.
 *
 * Author:
 *    Stephen Lau
 *
 * Date: October 16, 1992
 *
 */

int
main( int argc, char *argv[] )
{
  static const char funcName[] = "main";

  XtAppContext appContext = 0;

#if 0
  {
    FILE *out = fopen ("tv.stdout", "w");
    if (out) {
      dup2 (fileno (out), 1);
      fclose (out);
    }
  }
#endif

#ifdef HAVE_SETLINEBUF
  setlinebuf (stdout);
  setlinebuf (stderr);
#endif

  /* initialise the tvutils library */

  tvuInit( TVU_DEFAULT );

  /* parse the config file and command line arguments */

  tvuParseArgArray( argc, argv );

  if ( tvuGetInt( TVU_ARGS_PARSED ) < tvuGetInt( TVU_TOTAL_ARGS ) ) {
    printf( "TerraVision: unrecognized option(s) specified\n"
	    "Try TerraVision -h for valid options.\n\n" );
    return FALSE;
  }

  /* Initialize various system-specific things. */

  SystemInit ();

  /* do any initialisation required for supporting VRML feature models */

  FeatureInit();

  /* initialise the state for event logging */

  CreateTsLog();

  /* initialise the animated busy dialogs */

  InitBusyDialog();

#ifdef AVS
  /* Define the avs network editor UI for the spaceball module. */
  AVScorout_init(argc,argv,TerraVisionDesc);
#endif

#ifdef DPSS
  {
    extern char *pname;
    pname = "TerraVision";
  }
#endif

  /*
   * INITIALIZE THREAD HANDLING.
   *
   */
  main_thread = PR_GetCurrentThread ();
  pthr_set_name ((void *)main_thread, "TerraVision");

  if (! tvuGetInt( TVU_CAVE ) ) {
    if (! (appContext = TerraVisionInitialize (&argc, argv))) {
      return FALSE;
    }
  }

  /* If this is the first time that this version of TerraVision */
  /* has been used, then pop up the license agreement.          */

  if ( tvuGetInt( TVU_SHOW_LICENSE ) || 
       strcmp( tvuGetString( TVU_LICENSE_ACCEPTED ), TERRAVISION_RELEASE ) )
    ConfirmLicense();

  /* Update the status fields of the main GUI */
	
  TerraVisionRefreshStatusFields( FALSE );

  /* Initiate logging on start up if -log command line option given */

  if ( tvuGetInt( TVU_LOG ) )
    TerraVisionStartLogging();

  /* Go into the main loop... */

#ifdef CAVE
  if ( tvuGetInt( TVU_CAVE ) ) {
    TerraVisionStartCAVE();
    return TRUE;
  }
#endif

  exit_lock = PR_NewLock ();
  exit_cond = PR_NewCondVar (exit_lock);

#if 0
  TerraVisionGraphicsLoop (appContext);
#else
  {
    PRThread *graphics = 
      PR_CreateThread (PR_USER_THREAD, 
		       TerraVisionGraphicsLoop, appContext,
		       PR_PRIORITY_HIGH, PR_GLOBAL_THREAD,
		       PR_JOINABLE_THREAD, 1<<16);
    pthr_set_name (graphics, "Renderer");

    PR_Lock (exit_lock); {
      while (! exit_flag)
	PR_WaitCondVar (exit_cond, PR_INTERVAL_NO_TIMEOUT);
    }; PR_Unlock (exit_lock);

    PR_JoinThread (graphics);
  }
#endif

  PR_DestroyCondVar (exit_cond);
  PR_DestroyLock (exit_lock);

  if (0 && PR_Cleanup () != PR_SUCCESS) {
    TSMTRACE ((INIT_MASK|1, funcName, "PR_Cleanup() failed"));
  }

  return exit_return;
}

void
TerraVisionExit (int ret)
{
  /*static char const funcName[] = "TerraVisionExit";*/
 
  PR_Lock (exit_lock); {
    exit_flag = 1;
    exit_return = ret;
    PR_NotifyAllCondVar (exit_cond);
  }; PR_Unlock (exit_lock);
}

static void
SystemInit (void)
{
  static const char funcName[] = "SystemInit";

  /* Determine how much system memory is available. */
  systemMemory = -1;

#ifdef HAVE_INVENT_H
  {
    inventory_t *inv;
    setinvent ();
    for (inv = getinvent (); inv; inv = getinvent ()) {
      if (inv->inv_class == INV_MEMORY && inv->inv_type == INV_MAIN) {
	systemMemory = inv->inv_state;
	TSMTRACE ((1, funcName, "Main Memory: %dMB", systemMemory>>20));
	if (systemMemory/2 > TsCacheMemReserved) {
	  TsCacheMemReserved = systemMemory/2;
	  TSMTRACE ((CACHE_MASK|1, funcName, 
		     "Tiles: %dMB", TsCacheMemReserved>>20));
	}
	break;
      }
    }
    endinvent ();
  }
#endif

#ifdef __sgi
  /* SET THE MAXIMUM STACK SIZE. */
  prctl (PR_SETSTACKSIZE, 16*(1<<20)); /* 16MB */
#endif

#ifdef DEBUG
  {
    char * config_val;
    static unsigned long clockResolution;

    /* Get the resolution of the clock from the config file */
    if (! tvGetResourceAsInt ("CLOCK_RESOLUTION_USEC", &clockResolution)) {
      TSMTRINC ((INIT_MASK|1, -1));
      return (FALSE);
    }

    /* Clock for the system so everyone remains in sync */
    if (pthr_create (&clock_thread, "Clock", Timer, &clockResolution))
    {
      tsmError (funcName, "Unable to spawn the timer process");
      TSMTRINC ((INIT_MASK|1, -1));
      return FALSE;
    }
  }
#endif
}

/*
 * Synopsis:
 *    void TerraVisionRefreshMenus( void )
 *
 * Description:
 *    Refreshes the sensitivity of the Views menu entries. These
 *    menu options are ghosted out if there are no datasets loaded.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 6 April 1999
 *
 */

void
TerraVisionRefreshMenus( void )
{
  /*
  int show_menu = ( geoList != NULL && list_count(geoList) > 0 );

  XtSetSensitive( menuThreeDButton, show_menu );
  XtSetSensitive( menuTwoDButton, show_menu );
  XtSetSensitive( menuMapButton, show_menu );
#ifdef CAVE
  XtSetSensitive( menuCaveButton, show_menu );
#endif
*/
}

/*
 * Synopsis: int TerraVisionRefreshStatusFields( int initialise )
 *
 * Description: Updates the two status fields that appear at the
 *            bottom of the main TerraVision GUI. Currently we 
 *            display the OI and DEM urls, if loaded.
 *
 *            If the initialise parameter is TRUE, then "Not Loaded"
 *            will be displayed for each URL, even if a URL is known.
 *            This is done so that the initial setting of these fields
 *            don't grow the main GUI window by an unaesthetic degree.
 *            Normally, you will pass FALSE to this function.
 *
 * Externals: None
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 25 November 1998
 *
 */

void
TerraVisionRefreshStatusFields( int initialise )
{
  const char  *notavail = "Not Specified.";
  char        buffer[512];
  char const  *cfg_file = NULL;
  char const  *db_file = NULL;
  XmString    string;

  if ( tvuGetInt( TVU_CAVE ) ) return;

  /* we flagged as the initial call, we don't fill in the strings */

  if ( initialise == FALSE ) {
    cfg_file = tvuGetString( TVU_CONFIG_URL );
    db_file  = tvuGetString( TVU_DATABASE_URL );
  }

  /* display the full filename for the .TerraVision config file */

  sprintf( buffer, "Config File: %s", ( cfg_file ) ? cfg_file : notavail );
  string = tvXmString( buffer );
  XtVaSetValues( gatsStatusFieldWidget, XmNlabelString, string, NULL );
  XmStringFree( string );

  /* display the full filename for the local dataset database file */

  sprintf(buffer, "Database File: %s", ( db_file ) ? db_file : notavail );
  string = tvXmString( buffer );
  XtVaSetValues( tsmStatusFieldWidget, XmNlabelString, string, NULL );
  XmStringFree( string );
}


/*
 * Synopsis:
 *   TVDataSet *TVDataSetCreate(char *initialOi, char *initialDem)
 *
 * Description:
 *   Creates a new TVDataSet which is used in communication with the TSM
 *
 * Externals: NONE
 *
 * Returns: pointer to new TSMConncetion on success, NULL on failure
 *
 * Author: Stephen Lau
 * 
 * Date: Feb 1995
 *
 */

TVDataSet *
TVDataSetCreate( char const *initialOi, char const *initialDem )
{
  static char *funcName = "TVDataSetCreate";
  TVDataSet   *tsm;

  TSMTRINC(( TSMMASK|2, 1 ));
  TSMTRACE(( TSMMASK|2, funcName, "Entering." ));

  /* create a TVDataSet structure to describe this GeoSet */

  if ( ( tsm = (TVDataSet *) malloc(sizeof(TVDataSet)) ) == NULL ) {
    tsmError (funcName, "Error unable to allocate space for TSM.");
    TSMTRINC ((TSMMASK|2, -1));
    return (NULL);
  }

  /* fill in the structure with the OI and DEM URLs */

  tsm->oi_url     = initialOi;
  tsm->dem_url    = initialDem;
  tsm->oi_params  = NULL;
  tsm->dem_params = NULL;

  /* lookup the GeoSet name in the database based upon the OI url */
  tsm->name = tsmGetGeoSetFromURL( tv_database.database, tsm->oi_url );
  if ( tsm->name != NULL )
    tsm->name = tsmMemStrdup (tsm->name);

  /* if not in the database, then make one up from the OI URL string */

  if ( tsm->name == NULL ) 
    tsm->name = tsmNameFromPathname( (char *) tsm->oi_url );

  /* all done! */

  TSMTRACE(( TSMMASK|2, funcName, "Exiting." ));
  TSMTRINC(( TSMMASK|2, -1 ));

  return tsm;
}

/*
 * Synopsis:
 *    int OpenInitialWindow( void )
 *
 * Description:
 *    This function will open a 2D or 3D (or no) initial window,
 *    depending upon the setting the user specified on the command
 *    line for the -view option. If no setting was specified, then
 *    this function never does anything. Once the specified window
 *    is opened for the first time, subsequent calls to this function
 *    will also do nothing.
 *
 * Returns: TRUE if window opened on this call
 *
 * Author: Martin Reddy
 * 
 * Date: 20 November 2000
 *
 */

static int
OpenInitialWindow( void )
{
  static int first_time = TRUE;

  if ( ! first_time ) return FALSE;

  if ( tvuGetInt( TVU_INITIAL_VIEWER ) == 2 ) {

    /* Display the 2D viewer by default */

    TerraVisionDisplayTwoDWidget();
    return TRUE;
      
  } else if ( tvuGetInt( TVU_INITIAL_VIEWER ) == 3 ) {
      
    /* Display the 3D viewer by default */
      
    TerraVisionDisplayThreeDWidget();
    return TRUE;

  }

  first_time = FALSE;
  return FALSE;
}      

/*
 * Synopsis:
 *    void TVDataSetLoadSuccess( void *closure )
 *
 * Description:
 *    This function is called upon a successful connection to a 
 *    new dataset. It is implemented as a followup callback of
 *    the BusyDialog in TVDataSetLoadThread.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 6 April 1999
 *
 */

typedef struct {
  TVDataSet *tsm;
  TsGeoPyramid *geo;
} TVDataSetLoadSuccessClosure;

static void
addDatasetCB (tvBase *threeD, void *closure)
{
  TsGeoPyramid *geo = closure;
  tvAddDataset (threeD, geo);
}

static void
TVDataSetFree (TVDataSet * tsm)
{
  /* Clean up TVDataSet */
  tsmMemFree (tsm->name);
  free (tsm);
}

static void
TVDataSetLoadSuccess( void *closure )
{
  /*static char const funcName[] = "TVDataSetLoadSuccess";*/
  TVDataSetLoadSuccessClosure *cl = closure;

  /* refresh the Dataset Info window in case it's open */

  InfoRefreshDatasetList();

  /* Refresh the Viewpoint Bookmarks list */

  ViewpointRefresh();
 
  /* Update the status fields of the main GUI */
	
  TerraVisionRefreshStatusFields( FALSE );

  /* Do logging for this dataset too if logging is enabled */

  TerraVisionRestartLogging();

  /* Refresh any currently open Dataset Selector dialogs */

  RefreshDatasetSelectors();

  /* Refresh any map viewers */

  ScannedMapRefreshLists();

  /* Now tell the user that the dataset is ready. */

  TerraVisionRefreshMenus();
  
  /* Add this dataset to all current windows */

  if ( cl && cl->geo )
    tvForallWindows (addDatasetCB, cl->geo);

  TVDataSetFree (cl->tsm);
  free (cl);

  /* load the next dataset that was specified on the command line */
  /* only continue on passed here if we have loaded all datasets. */

  if ( TVDataSetLoadNext() )
    return;

  /* open the initial viewer specified for the first time round */

  OpenInitialWindow();

}

/*
 * Synopsis:
 *    void TVDataSetLoadFailure( void *closure )
 *
 * Description:
 *    This function is called when we failed to open a new dataset.
 *    It is implemented as a followup callback of the BusyDialog
 *    in TVDataSetLoadThread. All it does is display an error message.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 6 April 1999
 *
 */

static void
TVDataSetLoadFailure( void *closure )
{
  /*static char const funcName[] = "TVDataSetLoadFailure";*/
  TVDataSet         *tsm = (TVDataSet *) closure;
  char              buffer[512];
  XmString          string;

  /*  Display an error message to the user to tell of failure */

  sprintf( buffer, "ERROR: Could not open dataset %s\n", tsm->name );
  strcat( buffer, "Please check the Imagery and DEM servers" );

  string = tvXmString( buffer );
  XtVaSetValues( warningDialog, XmNmessageString, string, NULL );
  XmStringFree( string );

  XtManageChild( warningDialog );

  TVDataSetFree (tsm);

  /* load the next dataset that was specified on the command line */

  TVDataSetLoadNext();
}

/*
 * Synopsis:
 *   void TVDataSetLoadThread( BusyDialog *busy )
 *
 * Description:
 *   This is the work thread for loading a new dataset.
 *   It is called by the BusyDialog that is created in
 *   TVDataSetLoad(). It tries to open the specified
 *   dataset and vectors off to the appropriate followup
 *   function depending upon its success.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 6 April, 1999
 *
 */

static void
TVDataSetLoadThread( BusyDialog *busy )
{
  static char * funcName = "TVDataSetLoadThread";
  TVDataSet * tsm = (TVDataSet *) busy->closure;
  TsGeoPyramid *geo;

  TSMTRINC ((TSMMASK|2, 1));
  TSMTRACE ((TSMMASK|2, funcName, "Preparing data sets"));

  if ( ! tsm->oi_params ) tsmInitParams( &tsm->oi_params );
  if ( ! tsm->dem_params ) tsmInitParams( &tsm->dem_params );

  /* initialize the new data set */

  if ( ! (geo = TVDataSetConnect( tsm )) )
    SetBusyFollowup( busy, TVDataSetLoadFailure, tsm );
  else {
    TVDataSetLoadSuccessClosure *closure = malloc (sizeof (*closure));
    closure->tsm = tsm;
    closure->geo = geo;
    SetBusyFollowup( busy, TVDataSetLoadSuccess, closure );
  }

  TSMTRACE ((TSMMASK|2, funcName, "done."));
  TSMTRINC ((TSMMASK|2, -1));

  CloseBusyDialog( busy );
}

/*
 * Synopsis:
 *   void TVDataSetLoad( TVDataSet *tsm )
 *
 * Description:
 *   When the user selects a data set, we put up a busy dialog box
 *   and spawn off a work thread to change the data set and load the
 *   initial subset of DEM tiles. This uses the BusyDialog routines.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 6 April, 1999
 *
 */

void
TVDataSetLoad( TVDataSet *tsm )
{
  /*static const char funcName[] = "TVDataSetLoad";*/
  char         buffer[512];
  Purl         *dem_purl;

  sprintf( buffer, "Please wait...\nLoading Terrain for %s\nFrom ", tsm->name);

  /* Tell us where we are getting the DEM from */
  
  dem_purl = tsmParseUrl( tsm->dem_url );

  if ( dem_purl->protocol == tsmLocalFile )
    strcat( buffer, "local disk" );
  else {
    strcat( buffer, dem_purl->protocolString );
    strcat( buffer, "://" );
    strcat( buffer, dem_purl->hostName );
  }
  
  tsmFreePurl( dem_purl );

  /* Open the BusyDialog window and vector off to the work thread */

  OpenBusyDialog( NULL, buffer, TVDataSetLoadThread, tsm );
}

/*
 * Synopsis:
 *   int TVDataSetLoadNext( void )
 *
 * Description:
 *   This function is used in order to load all of the datasets that
 *   the user specified on the command line using -oi/-dem pairs.
 *
 *   Each time you call this function it will take the next pair of
 *   OI and DEM urls and pass these to TVDataSetLoad(), above. 
 *   
 *   N.B. you must initialise currInitialSet to 0 before calling this
 *   function for the first time. We do this in TerraVisionParseArgs.
 *
 * Returns: TRUE if a new dataset load was initiated, FALSE if not.
 *
 * Author: Martin Reddy
 * 
 * Date: 18 July, 1999
 *
 */

static int
TVDataSetLoadNext( void )
{
  const char *funcName = "TVDataSetLoadNext";
  TVDataSet  *tsm;
  char       *oi_url, *dem_url;

  /* do we have another dataset to load? */

  if ( currInitialSet >= tvuGetInt( TVU_NUM_DATASETS ) )
    return FALSE;

  /* Create TSM structure */

  oi_url  = tvuGetStringN( TVU_OI_URL, currInitialSet );
  dem_url = tvuGetStringN( TVU_DEM_URL, currInitialSet );

  if ( ( tsm = TVDataSetCreate( oi_url, dem_url ) ) == NULL ) {
    tsmError( funcName, "Unable to specify data set!" );
    return FALSE;
  }

  /* now load the dataset with this DEM and OI */

  TVDataSetLoad( tsm );

  /* increment count so that we load the next dataset next time round */
  
  currInitialSet++;
  return TRUE;
}

/*
 * Synopsis:
 *   void TerraVisionLoadDataset( char *geo_name, char *oi_url, char *dem_url,
 *                                TsmConnectionParams, TsmConnectionParams )
 *
 * Description:
 *   This function is a temporary wrapper that will fake an old style
 *   TVDataSet structure so that we can use the new global database
 *   functions to interface with the existing code to load datasets.
 *   In time, we should clear out the old TVDataSet structure and
 *   exercise all remnants of the original dataset selection. In the 
 *   meantime, let's make it work.
 *
 *   The geo_name string is just used for information dialogs. If it
 *   is NULL, then an appropriate name will be created from the OI
 *   url.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 24 August 1998
 *
 */

void
TerraVisionLoadDataset( char const *geo_name,
			char const *oi_url,
			char const *dem_url,
			TsmConnectionParams oi_params,
			TsmConnectionParams dem_params )
{
  /*static const char funcName[] = "TerraVisionLoadDataset";*/
  TVDataSet *tsm = TVDataSetCreate (oi_url, dem_url);
			
  if ( geo_name )
    tsm->name = tsmMemStrdup( geo_name );
  else
    tsm->name = tsmNameFromPathname( (char *) oi_url );

  tsm->oi_params = tsmCopyParams( oi_params );
  tsm->dem_params = tsmCopyParams( dem_params );

  TVDataSetLoad( tsm );
}

/*
 * Synopsis: int TerraVisionInitialize(void)
 *	
 *
 * Description: Checks to see what machine we are on
 *      Initializes shared memory, semaphores
 *      Creates structures needed later on
 *
 * Externals:
 *
 *
 * Returns: TRUE = on success
 *          FALSE - on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1994
 *
 */
static XtAppContext
TerraVisionInitialize(int *argcp, char *argv[])
{
  static char * funcName = "TerraVisionInitialize";
  XtAppContext appContext = 0;

  TSMTRINC ((INIT_MASK|1, 1));
  TSMTRACE ((INIT_MASK|1, funcName, "Entering."));

  /*
   * REGISTER CALLBACKS FOR UNTIMELY DEATHS.
   * We need to do this so we don't have remnants of shared
   * memory floating around. 
   *
   */
  
  if (0) {
    /* Ensure that all signals are enabled. */
    sigset_t set;
    sigemptyset(&set);
    sigprocmask (SIG_SETMASK, &set, NULL);
  }

  if (0) {
    struct sigaction sa;

    sa.sa_handler = SigCb;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction (SIGTERM, &sa, NULL);
    sigaction (SIGBUS, &sa, NULL);
    sigaction (SIGSEGV, &sa, NULL);
    sigaction (SIGINT, &sa, NULL);
    sigaction (SIGTRAP, &sa, NULL);
  }

  /* INITIALIZE GRAPHICS AND WIDGETS */
  appContext = TerraVisionInitGraphics (argcp, argv);

  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Done."));
  TSMTRINC ((INIT_MASK|OGL_MASK|1, -1));

  if (! appContext)
    tsmError(funcName, "Unable to intialize graphics!");

  return appContext;
}

/*
 * Synopsis: TerraVisionChooseVisuals(Display *d, Screen *scr)
 *
 * Description:  Chooses an appropriate visual.
 *
 * Externals: tvVisualInfo
 *
 * Returns: NONE
 *
 * Author:
 *		Lee Iverson
 *
 * Date:
 *	December 11, 1996
 */
static void
TerraVisionChooseVisuals (Display *disp, Screen *scr)
{
  static char * funcName = "TerraVisionChooseVisuals";

  /* INITIALIZE OPENGL CONTEXT */
  int screen_num = XScreenNumberOfScreen (scr);

  /* CHECK IF WE'VE GOT A 24 BIT TRUECOLOR VISUAL. */
  if (XMatchVisualInfo (disp, screen_num,
			24, TrueColor, &tvVisualInfo)) {

  /* CHECK IF WE'VE GOT A 16 BIT TRUECOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num,
			       16, TrueColor, &tvVisualInfo)) {

  /* CHECK IF WE'VE GOT A 16 BIT PSEUDOCOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num,
			       16, PseudoColor, &tvVisualInfo)) {
    
  /* CHECK IF WE'VE GOT AN 8 BIT PSEUDOCOLOR VISUAL. */
  } else if (XMatchVisualInfo (disp, screen_num,
			       8, PseudoColor, &tvVisualInfo)) {
  } else {
    tsmError (funcName, "Unable to find usable basic visual.");
    exit (1);
  }

  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, 
	     "tvVisualInfo {vis=%p, vid=%x, scrn=%d, depth=%d, cls=%d}",
	     tvVisualInfo.visual, tvVisualInfo.visualid, 
	     tvVisualInfo.screen, 
	     tvVisualInfo.depth, tvVisualInfo.class));
}

/*
 * Synopsis: TerraVisionDefineColormap(Widget shell, XVisualInfo *vinfo)
 *
 * Description:  Chooses an appropriate colormap based on vinfo and
 *		 attaches it to shell.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: December 11, 1996
 */

static void
TerraVisionDefineColormap (Widget shell, XVisualInfo *vinfo)
{
  /* When we use a the default visual, nothing interesting is required. */
  if (DefaultVisual (tvDisplay, vinfo->screen) == vinfo->visual)
    return;

  /* Otherwise, create an appropriate colormap. */
  {
    Colormap colormap;
    XColor color, exact;
    unsigned long foreground, background;
    unsigned long top_shadow, bot_shadow, select;

    colormap = XCreateColormap (tvDisplay, 
				RootWindow (tvDisplay, vinfo->screen),
				vinfo->visual, AllocNone);

    /* Allocate the background and foreground. */
    if (! XAllocNamedColor (tvDisplay, colormap, "gray", &color, &exact))
      return;
    background = color.pixel;

    /* Ask Motif to allocate appropriate coverage colors. */
    XmGetColors (tvScreen, colormap, background, &foreground,
		 &top_shadow, &bot_shadow, &select);

    /* And initialize the applicationShell for inheritance. */
    XtVaSetValues (shell,
		   XmNdepth, vinfo->depth,
		   XmNcolormap, colormap,
		   XmNvisual, vinfo->visual,
		   XmNbackground, background,
		   XmNforeground, foreground,
		   XmNarmColor, select,
		   XmNtopShadowColor, top_shadow,
		   XmNbottomShadowColor, bot_shadow,
		   NULL);

    TerraVisionOverrideShellDefaults (vinfo);
  }
}

/*
 * Synopsis: TerraVisionInitGraphics()
 *
 *
 * Description:  Initializes the graphics, X11 and OpenGL, and runs the
 *    		 event loop.
 *
 * Externals:
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 *
 * Date:
 *	November 7, 1993
 */
XtAppContext
TerraVisionInitGraphics(int *argcp, char * argv[])
{
  static char *funcName = "TerraVisionInitGraphics";
  char *       displayName = NULL, wintitle[64];
  Widget       mainForm;
  Dimension    shadowThickness;
  Dimension    appHeight, appWidth;
  XtAppContext appContext;
  char const * appName;
  char const * appClass = "TerraVision";
  XmString     title;

  TSMTRINC ((INIT_MASK|OGL_MASK|1, 1));
  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Entering."));

  widgetTimer = 0;
#ifdef AVS
  avsTimer = 0;
#endif
  
  /*
   * IF NO DISPLAY VARIABLE IS SET, WE MAY BE USING XCPS
   * OR THE USER COULD BE BRAIN DEAD. DEFAULT TO LOCAL DISPLAY
   *
   */
  displayName = getenv("DISPLAY");
  if(!displayName) {
    TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName,
	       "No DISPLAY environment variable set.\n" \
	       "Defaulting GL to localhost:0.0"));
    displayName = "localhost:0.0";
  }

#ifdef CAVE
#endif

  /* INITIALIZE XWINDOWS & CREATE APP SHELL */
  applicationShell = XtVaOpenApplication(&appContext, appClass,
					 NULL, 0,
					 argcp, argv,
					 fallbackResources,
					 applicationShellWidgetClass,
					 NULL, 0,
					 NULL);
  if (applicationShell == NULL)
    {
      tsmError (funcName, "Unable to initialize X11.");
      TSMTRINC ((INIT_MASK|OGL_MASK|1, -1));
      return (NULL);
    }

  /* The display is derived from the application shell. */
  tvDisplay = XtDisplay (applicationShell);
  tvScreen = XtScreen (applicationShell);


  appName = XtName (applicationShell);
  tvInitDatabase (tvDisplay, appName, appClass, argcp, argv);

  /* Check for various OpenGL features. */
  TerraVisionChooseVisuals (tvDisplay, tvScreen);

  TerraVisionDefineColormap (applicationShell, &tvVisualInfo);

  /* Register error handler for X11 so that we can debug errors. */
  XSetErrorHandler (TerraVisionXError);
  XSetIOErrorHandler (TerraVisionXIOError);

#if defined(DEBUG) && defined(SPACEBALL_H)
  /* INITIALIZE THE SPACEBALL */
  InitializeSpaceBall(tvDisplay);
#endif

  /* THE WINDOW CONTAINS ONE  FORM AS PARENT */
  sprintf( wintitle, "TerraVision II" );

  title = tvXmString( wintitle );
  mainForm = XtVaCreateManagedWidget("mainForm", xmFormWidgetClass,
				     applicationShell,
				     XmNdialogTitle, title,
				     NULL);
  XmStringFree (title);

  /* GET THE SHADOW THICKNESSES TO LATER SET MIN SIZES */
  XtVaGetValues(mainForm, XmNshadowThickness, &shadowThickness, NULL);
  XtVaSetValues(mainForm, XmNhorizontalSpacing, shadowThickness,
		XmNverticalSpacing, shadowThickness, NULL);

  /* CREATE POPUP DIALOG BOXES */
  quitDialog = CreateQuitDialog(applicationShell);
  warningDialog = CreateWarningDialog(applicationShell);
  NetLogInit(applicationShell);
  DBaseInit(applicationShell);
  
  /* CREATE THE MENU BAR */
  TerraVisionCreateMenuBar(mainForm);
  
  /* CREATE STATUS BAR */
  TerraVisionCreateStatusBar(mainForm, menuBar);


  /* GET THE URL's FOR THE 2D/3D BUTTONS */
  if (! TerraVisionGetURLPaths ()) {
    tsmError (funcName, "Unable to get the button URL's.");
    TSMTRINC ((INIT_MASK|OGL_MASK|1, -1));
    return (FALSE);
  }

  XtRealizeWidget(applicationShell);

  /* GET THE CURRENT SIZE OF THE APPLICATION BASED ON RESOURCE FILE   */
  XtVaGetValues(applicationShell, 
		XmNwidth, &appWidth,
		XmNheight, &appHeight,
		NULL);

  /* ADD THE SHADOW SIZE AND MAKE THIS THE MIN SIZE OF THE APPLICATION */
  appWidth += shadowThickness;
  appHeight += shadowThickness;
  
  XtVaSetValues(applicationShell, 
		XmNminWidth, appWidth,
		XmNminHeight, appHeight,
		NULL);

  /* CREATE THE BUSY CURSOR */
  busyCursor = XCreateFontCursor(tvDisplay, XC_watch);

#ifdef AVS
  /* TIMER FOR VARIOUS WIDGET FUNCTIONS */
  if(!avsTimer) {
    avsTimer = XtAppAddTimeOut(appContext,
			       100,
			       (XtTimerCallbackProc)
			       AVSTimerCallback,
			       appContext);
  }
#endif

  /* Create a pixmap for the application's icon */

  /*
    {
    Pixmap iconified = MakeIconifiedPixmap( mainForm );
    XtVaSetValues( applicationShell, XmNiconPixmap, iconified, NULL );
    }
  */
  XtVaSetValues( applicationShell, XmNiconName, "TerraVision", NULL );


  /* TIMER FOR VARIOUS WIDGET FUNCTIONS */
  if(!widgetTimer) {
    widgetTimer = XtAppAddTimeOut(appContext,
				  100, 
				  (XtTimerCallbackProc)
				  TerraVisionTimerCallback,
				  appContext);
  }

  TSMTRACE ((INIT_MASK|OGL_MASK|1, funcName, "Done"));
  TSMTRINC ((INIT_MASK|OGL_MASK|1, -1));
  return (appContext);
}

/*
 * Synopsis: TerraVisionXError (Display *disp, XErrorEvent *ev)
 *
 * Description:  Process an X11 error.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: January 17, 1996
 */
/*static*/ int
TerraVisionXError (Display *disp, XErrorEvent *ev)
{
  char buf[512];
  XGetErrorText (disp, ev->error_code, buf, 512);
  pthr_trace_print_pid (TRUE);
  pthr_report ("** X Error %s\n\t Serial #%d, error %d, req %d, minor %d",
	       buf, ev->serial,
	       ev->error_code, ev->request_code, ev->minor_code);
  if ( tvuGetInt( TVU_X_ERROR_ABORT ) ) abort ();
  return 0;
}

/*
 * Synopsis: TerraVisionXIOError (Display *disp, XErrorEvent *ev)
 *
 * Description:  Process an X11 I/O error.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: January 17, 1996
 */

static int
TerraVisionXIOError (Display *disp)
{
  fprintf (stderr, "PID %d: X I/O Error\n", (int) getpid ());
  abort ();
  return 0;
}


/*
 * Synopsis: TerraVisionOverrideShellDefaults(XVisualInfo *v)
 *
 * Description:  Overrides the visual used by all shells created by the
 *               application.  This is necessary because Motif properly
 *               inherits depth and colormap from the applicationShell but not
 *               the visual.
 *
 * Returns: NONE
 *
 * Author: Lee Iverson
 *
 * Date: December 11, 1996
 */
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>

static struct {
  void (*Initialize) ();
  Visual *visual;
} shellValues;

static void
TerraVisionShellInitialize (Widget treq, Widget tnew,
			    ArgList args, Cardinal *num_args)
{
  ShellWidget shnew = (ShellWidget) tnew;

  shnew->shell.visual = shellValues.visual;

  if (shellValues.Initialize)
    (*shellValues.Initialize) (treq, tnew, args, num_args);
}

static void
TerraVisionOverrideShellDefaults (XVisualInfo *v)
{
  shellValues.visual = v->visual;

  shellValues.Initialize = shellWidgetClass->core_class.initialize;
  shellWidgetClass->core_class.initialize = TerraVisionShellInitialize;
}

/*
 * Synopsis: static void TerraVisionGraphicsLoop(XtAppContext)
 *	
 *
 * Description: Loop to handle X Events and SpaceBall events
 *
 *
 * Externals: Widget threeD - for spaceball events
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1993
 *
 */

#ifdef TRACE_OGL
static char const *XEventName[] = {
  "Zero",
  "One",
  "KeyPress",
  "KeyRelease",
  "ButtonPress",
  "ButtonRelease",
  "MotionNotify",
  "EnterNotify",
  "LeaveNotify",
  "FocusIn",
  "FocusOut",
  "KeymapNotify",
  "Expose",
  "GraphicsExpose",
  "NoExpose",
  "VisibilityNotify",
  "CreateNotify",
  "DestroyNotify",
  "UnmapNotify",
  "MapNotify",
  "MapRequest",
  "ReparentNotify",
  "ConfigureNotify",
  "ConfigureRequest",
  "GravityNotify",
  "ResizeRequest",
  "CirculateNotify",
  "CirculateRequest",
  "PropertyNotify",
  "SelectionClear",
  "SelectionRequest",
  "SelectionNotify",
  "ColormapNotify",
  "ClientMessage",
  "MappingNotify"
};
#endif

static void
TerraVisionGraphicsLoop (void *arg)
{
  /*static char const funcName[] = "TerraVisionGraphicsLoop";*/
  XtAppContext appContext = arg;
  PRThread *thr = PR_GetCurrentThread ();

  /* If requested, connect to the tsm after we pop up the GUI. */
  /* If not loading any datasets, then check to see if we are  */
  /* to open an initial viewer (needs to be done in callback). */

  if ( tvuGetInt( TVU_NUM_DATASETS ) || tvuGetInt( TVU_CAVE ) ) {
    TVDataSetLoadNext ();
  } else if ( tvuGetInt( TVU_INITIAL_VIEWER ) > 0 ) {
    OpenInitialWindow ();
  }

  /* LOOP AWAY */
  while (! exit_flag) {
    XEvent event;
    XtAppNextEvent (appContext, &event);
    XtDispatchEvent (&event);
    /* XUnlockDisplay( XtDisplay(applicationShell) ); */
  }
}


/*
 * Synopsis: void TerraVisionCreateMenuBar(Widget)
 *   
 *
 * Description:
 *    The menu bar is created here. A callback is added to each selection so
 * we can register a respone to each selection.
 *
 * Externals: menuBar, menuAboutButton ...
 *
 *
 * Returns:
 *    void
 *
 * Author:
 *      Stephen Lau
 * 
 * Date: October 16, 1992
 *
 */
void
TerraVisionCreateMenuBar (Widget parent)
{ 
 /*static char const funcName[] = "TerraVisionCreateMenuBar";*/
 Widget       cascade, menu_pane;
 Arg          args[12];
 XmString     name, accel;
 unsigned int n;
 static XtCallbackRec callbacks[] = {{NULL, NULL}, {NULL, NULL}};
 int trace_level = pthr_trace_level (-1);
 int tv_trace_mask = tvuGetInt( TVU_TRACE_MASK );

    n = 0;
    XtSetArg(args[n], XmNspacing, 15); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    
    menuBar = XmCreateMenuBar(parent, "menuBar", args, n);
    XtManageChild (menuBar);

    /* CREATE THE "File" MENU */
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menu_pane = XmCreatePulldownMenu (menuBar, "menu_pane", args, n);

    callbacks[0].callback = TerraVisionButtonCB;

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuOpenTVButton = XtVaCreateManagedWidget("Open Dataset", 
					       xmPushButtonGadgetClass,
					       menu_pane,
					       XmNactivateCallback,
					       callbacks,
					       XmNmnemonic, 'O',
					       NULL);

    menuCloseTVButton = XtVaCreateManagedWidget("Close Dataset", 
						xmPushButtonGadgetClass,
						menu_pane,
						XmNactivateCallback,
						callbacks,
						XmNmnemonic, 'C',
						NULL);
 
    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );


    /* Create the Debug Level submenu. */
    {
      Widget submenu;

      n = 0;
      XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
      XtSetArg(args[n], XmNradioBehavior, True); n++;
      XtSetArg(args[n], XmNradioAlwaysOne, True); n++;
      submenu = XmCreatePulldownMenu (menu_pane, "submenu", args, n);

      name = tvXmString ("None");
      menuTVDebugLevel0Button = XtVaCreateManagedWidget (
                                        "debug_0", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, '0',
					XmNvisibleWhenOff, False,
					XmNset, 
					trace_level==0 ? True : False,
					NULL);
      XmStringFree (name);
      XtManageChild (menuTVDebugLevel0Button);
      XtAddCallback (menuTVDebugLevel0Button, XmNvalueChangedCallback,
		     TerraVisionDebugLevelCB, (XtPointer) 0);

      name = tvXmString ("Level 1");
      menuTVDebugLevel1Button = XtVaCreateManagedWidget (
                                        "debug_1", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, '1',
					XmNvisibleWhenOff, False,
					XmNset,
					trace_level==1 ? True : False,
					NULL);
      XmStringFree (name);
      XtManageChild (menuTVDebugLevel1Button);
      XtAddCallback (menuTVDebugLevel1Button, XmNvalueChangedCallback,
		     TerraVisionDebugLevelCB, (XtPointer) 1);

      name = tvXmString ("Level 2");
      menuTVDebugLevel2Button = XtVaCreateManagedWidget (
                                        "debug_2", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, '2',
					XmNvisibleWhenOff, False,
					XmNset,
					trace_level==2 ? True : False,
					NULL);
      XmStringFree (name);
      XtManageChild (menuTVDebugLevel2Button);
      XtAddCallback (menuTVDebugLevel2Button, XmNvalueChangedCallback,
		     TerraVisionDebugLevelCB, (XtPointer) 2);

      name = tvXmString ("Level 3");
      menuTVDebugLevel3Button = XtVaCreateManagedWidget (
                                        "debug_3", xmToggleButtonGadgetClass,
					submenu, 
					XmNlabelString, name,
					XmNmnemonic, '3',
					XmNvisibleWhenOff, False,
					XmNset,
					trace_level==3 ? True : False,
					NULL);
      XmStringFree (name);
      XtManageChild (menuTVDebugLevel3Button);
      XtAddCallback (menuTVDebugLevel3Button, XmNvalueChangedCallback,
		     TerraVisionDebugLevelCB, (XtPointer) 3);

      name = tvXmString ("Level 4");
      menuTVDebugLevel4Button = XtVaCreateManagedWidget (
                                        "debug_4", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, '4',
					XmNvisibleWhenOff, False,
					XmNset,
					trace_level==4 ? True : False,
					NULL);
      XmStringFree (name);
      XtManageChild (menuTVDebugLevel4Button);
      XtAddCallback (menuTVDebugLevel4Button, XmNvalueChangedCallback,
		     TerraVisionDebugLevelCB, (XtPointer) 4);

      n = 0;
      XtSetArg (args[n], XmNsubMenuId, submenu); n++;
      XtSetArg (args[n], XmNmnemonic, 'D'); n++;
      cascade = XmCreateCascadeButton (menu_pane, "Debug Level", args, n);
      XtManageChild (cascade);
    }

    /* Create the Debug Masks submenu. */
    {
      Widget submenu, button;

      n = 0;
      XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
      submenu = XmCreatePulldownMenu (menu_pane, "submenu", args, n);

      /*
      button = XtVaCreateManagedWidget( "mask_0", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, tvXmString ("All"),
					XmNmnemonic, 'A',
					XmNindicatorOn, True,
					XmNset, False,
					NULL );
      XtManageChild( button );
      XtAddCallback( button, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 0 );
      */

      name = tvXmString ("Tiles");
      menuTVDebugTilesButton = XtVaCreateManagedWidget( "mask_1",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 't',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&TSMMASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugTilesButton );
      XtAddCallback( menuTVDebugTilesButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 1 );

      name = tvXmString ("Texture");
      menuTVDebugTextureButton = XtVaCreateManagedWidget( "mask_2",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'x',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&TEX_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugTextureButton );
      XtAddCallback( menuTVDebugTextureButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 2 );

      name = tvXmString ("Init");
      menuTVDebugInitButton = XtVaCreateManagedWidget( "mask_3",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'I',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&INIT_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugInitButton );
      XtAddCallback( menuTVDebugInitButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 3 );

      name = tvXmString ("2-D Viewer");
      menuTVDebug2DButton = XtVaCreateManagedWidget( "mask_4",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, '2',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
				(tv_trace_mask&TWOD_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebug2DButton );
      XtAddCallback( menuTVDebug2DButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 4 );

      name = tvXmString ("3-D Viewer");
      menuTVDebug3DButton = XtVaCreateManagedWidget( "mask_5",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, '3',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
				(tv_trace_mask&THREED_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebug3DButton );
      XtAddCallback( menuTVDebug3DButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 5 );

      name = tvXmString ("Map Viewer");
      menuTVDebugMapButton = XtVaCreateManagedWidget( "mask_6",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'M',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
				(tv_trace_mask&MAP_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugMapButton );
      XtAddCallback( menuTVDebugMapButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 6 );

      name = tvXmString ("OpenGL");
      menuTVDebugOpenGLButton = XtVaCreateManagedWidget( "mask_7",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'O',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
				(tv_trace_mask&OGL_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugOpenGLButton );
      XtAddCallback( menuTVDebugOpenGLButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 7 );

      name = tvXmString ("Visibility");
      menuTVDebugVizButton = XtVaCreateManagedWidget( "mask_8",
				        xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'V',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&VIS_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugVizButton );
      XtAddCallback( menuTVDebugVizButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 8 );

      name = tvXmString ("OAA");
      menuTVDebugOAAButton = XtVaCreateManagedWidget( "mask_9",
				        xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'A',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&OAA_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugOAAButton );
      XtAddCallback( menuTVDebugOAAButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 9 );

      name = tvXmString ("Logging");
      menuTVDebugLogButton = XtVaCreateManagedWidget( "mask_10",
				        xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'L',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&LOG_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugLogButton );
      XtAddCallback( menuTVDebugLogButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 10 );

      name = tvXmString ("Cache");
      menuTVDebugCacheButton = XtVaCreateManagedWidget( "mask_11",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'C',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&CACHE_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugCacheButton );
      XtAddCallback( menuTVDebugCacheButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 11 );

      name = tvXmString (".geo");
      menuTVDebugDotGeoButton = XtVaCreateManagedWidget( "mask_12",
					xmToggleButtonGadgetClass, submenu,
					XmNlabelString, name,
					XmNmnemonic, 'G',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset,
					(tv_trace_mask&DOTGEO_MASK)?True:False,
					NULL);
      XmStringFree (name);
      XtManageChild( menuTVDebugDotGeoButton );
      XtAddCallback( menuTVDebugDotGeoButton, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 12 );

      XtVaCreateManagedWidget( "separator", 
			       xmSeparatorGadgetClass, submenu, NULL );

      name = tvXmString ("DPSS");
      button = XtVaCreateManagedWidget ("mask_100", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, 'D',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset, False,
					NULL);
      XmStringFree (name);
      XtManageChild( button );
      XtAddCallback( button, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 100 );

      name = tvXmString ("Timestamp");
      button = XtVaCreateManagedWidget ("mask_101", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, 'T',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset, False,
					NULL);
      XmStringFree (name);
      XtManageChild( button );
      XtAddCallback( button, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 101 );

#if defined(_pthr_sproc_h_) && DEBUG_PTHREAD
      name = tvXmString ("Threads");
      button = XtVaCreateManagedWidget ("mask_102", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, 'h',
					XmNindicatorOn, True,
					XmNvisibleWhenOff, True,
					XmNset, False,
					NULL);
      XmStringFree (name);
      XtManageChild( button );
      XtAddCallback( button, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 102 );
#endif

      /*
      name = tvXmString ("Timestamps");
      button = XtVaCreateManagedWidget ("mask_10", xmToggleButtonGadgetClass,
					submenu,
					XmNlabelString, name,
					XmNmnemonic, 'm',
					XmNindicatorOn, True,
					XmNset, False,
					NULL);
      XmStringFree (name);
      XtManageChild( button );
      XtAddCallback( button, XmNvalueChangedCallback,
		     TerraVisionDebugMaskCB, (XtPointer) 10 );
      */

      n = 0;
      XtSetArg (args[n], XmNsubMenuId, submenu); n++;
      XtSetArg (args[n], XmNmnemonic, 'M'); n++;
      cascade = XmCreateCascadeButton (menu_pane, "Debug Masks", args, n);
      XtManageChild (cascade);
    }

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuInfoButton = XtVaCreateManagedWidget("Dataset Info", 
					     xmPushButtonGadgetClass,
					     menu_pane,
					     XmNactivateCallback,
					     callbacks,
					     XmNmnemonic, 'I',
					     NULL);
 
    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    accel = XmStringCreateLocalized( "Ctrl+Q" );
    menuQuitButton = XtVaCreateManagedWidget("Exit",
					     xmPushButtonGadgetClass,
					     menu_pane,
					     XmNactivateCallback, callbacks,
					     XmNmnemonic, 'x',
					     XmNaccelerator, "Ctrl<Key>Q",
					     XmNacceleratorText, accel,
					     NULL);
    XmStringFree( accel );

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane); n++;
    XtSetArg (args[n], XmNmnemonic, 'F'); n++;
    cascade = XmCreateCascadeButton (menuBar, "File", args, n);
    XtManageChild (cascade);

    /* CREATE THE "Views" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menu_pane = XmCreatePulldownMenu (menuBar, "menu_pane", args, n);

    menuThreeDButton = XtVaCreateManagedWidget("New 3-D Viewer", 
					       xmPushButtonGadgetClass,
					       menu_pane,
					       XmNactivateCallback,
					       callbacks,
					       XmNmnemonic, '3',
					       NULL);
 
    menuTwoDButton = XtVaCreateManagedWidget("New 2-D Viewer",
					     xmPushButtonGadgetClass,
					     menu_pane,
					     XmNactivateCallback, callbacks,
					     XmNmnemonic, '2',
					     NULL);

    menuMapButton = XtVaCreateManagedWidget("New Map Viewer",
					    xmPushButtonGadgetClass,
					    menu_pane,
					    XmNactivateCallback, callbacks,
					    XmNmnemonic, 'M',
					    NULL);

#ifdef CAVE
    menuCaveButton = XtVaCreateManagedWidget("CAVE 3-D Viewer",
					    xmPushButtonGadgetClass,
					    menu_pane,
					    XmNactivateCallback, callbacks,
					    XmNmnemonic, 'C',
					    NULL);
    XtSetSensitive(menuCaveButton, False);
#else
    menuCaveButton = NULL;
#endif

    TerraVisionRefreshMenus();
    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane); n++;
    XtSetArg (args[n], XmNmnemonic, 'V'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Views", args, n);
    XtManageChild (cascade);

    /* CREATE THE "Logging" MENU */
    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menu_pane = XmCreatePulldownMenu (menuBar, "menu_pane", args, n);

    menuTVLoggingStartButton =
      XtVaCreateManagedWidget("Start Logging",
			      xmPushButtonGadgetClass, menu_pane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'S',
			      NULL );

    menuTVLoggingStopButton =
      XtVaCreateManagedWidget("Stop Logging",
			      xmPushButtonGadgetClass, menu_pane,
			      XmNactivateCallback, callbacks,
			      XmNmnemonic, 'p',
			      NULL );
    XtSetSensitive( menuTVLoggingStopButton, False );

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuTVLoggingGraphsButton =
       XtVaCreateManagedWidget("View Logging Graphs",
			       xmPushButtonGadgetClass, menu_pane,
			       XmNactivateCallback, callbacks,
			       XmNmnemonic, 'V',
			       NULL);

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuTVLoggingDirectoryButton =
       XtVaCreateManagedWidget("Set Logging Options",
			       xmPushButtonGadgetClass, menu_pane,
			       XmNactivateCallback, callbacks,
			       XmNmnemonic, 'O',
			       NULL);

    /* Create the Logging Event submenu. */
    {
      Widget submenu;

      n = 0;
      XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
      submenu = XmCreatePulldownMenu( menu_pane, "submenu", args, n );

      menuTVLoggingPredictButton =
	XtVaCreateManagedWidget( "Log Prediction QT",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNmnemonic, 'P',
				 XmNset, True,
				 NULL );
 
      menuTVLoggingRequestButton =
	XtVaCreateManagedWidget( "Log Tile Requests",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'e',
				 NULL );

      XtAddCallback( menuTVLoggingRequestButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );

      menuTVLoggingReaderButton =
	XtVaCreateManagedWidget( "Log Tile Reads",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'R',
				 NULL );

      XtAddCallback( menuTVLoggingReaderButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );

      menuTVLoggingCacheButton =
	XtVaCreateManagedWidget( "Log Tile Cache",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'C',
				 NULL );

      XtAddCallback( menuTVLoggingCacheButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );

      menuTVLoggingVisibleButton =
	XtVaCreateManagedWidget( "Log Visible QuadTrees",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'Q',
				 NULL );
 
      XtAddCallback( menuTVLoggingVisibleButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );

      menuTVLoggingRenderButton =
	XtVaCreateManagedWidget( "Log Rendering Times",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'T',
				 NULL );

      XtAddCallback( menuTVLoggingRenderButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );

      menuTVLoggingTextureCacheButton =
	XtVaCreateManagedWidget( "Log Texture Cache",
				 xmToggleButtonGadgetClass, submenu,
				 XmNindicatorOn, True,
				 XmNvisibleWhenOff, True,
				 XmNset, True,
				 XmNmnemonic, 'x',
				 NULL );

      XtAddCallback( menuTVLoggingTextureCacheButton, XmNvalueChangedCallback,
		     TerraVisionToggleChangedCB, (XtPointer) &tvLogger );
 
      n = 0;
      XtSetArg( args[n], XmNsubMenuId, submenu ); n++;
      XtSetArg( args[n], XmNmnemonic, 'E' ); n++;
      cascade = XmCreateCascadeButton( menu_pane, "Set Log Events", args, n );
      XtManageChild( cascade );
    }

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuTVLoggingCleanButton =
       XtVaCreateManagedWidget("Clean Log Directory",
			       xmPushButtonGadgetClass, menu_pane,
			       XmNactivateCallback, callbacks,
			       XmNmnemonic, 'l',
			       NULL);

    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane); n++;
    XtSetArg (args[n], XmNmnemonic, 'L'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Logging", args, n);
    XtManageChild (cascade);

    /* CREATE THE "PREFS" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menu_pane = XmCreatePulldownMenu (menuBar, "menu_pane", args, n);

    menuDPSSPrefsButton =
      XtVaCreateManagedWidget( "DPSS Preferences",
			       xmPushButtonGadgetClass, menu_pane,
			       XmNactivateCallback, callbacks,
			       XmNmnemonic, 'D',
			       NULL );
 
    n = 0;
    XtSetArg (args[n], XmNsubMenuId, menu_pane); n++;
    XtSetArg (args[n], XmNmnemonic, 'P'); n++;
    cascade = XmCreateCascadeButton (menuBar, "Prefs", args, n);
    XtManageChild (cascade);

    /* CREATE THE "HELP" MENU */

    n = 0;
    XtSetArg(args[n], XmNtearOffModel, XmTEAR_OFF_ENABLED); n++;
    menu_pane = XmCreatePulldownMenu (menuBar, "menu_pane", args, n);

    menuAboutButton = XtVaCreateManagedWidget("About", 
					      xmPushButtonGadgetClass,
					      menu_pane,
					      XmNactivateCallback, callbacks,
					      XmNmnemonic, 'A',
					      NULL);

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuLicenseButton = XtVaCreateManagedWidget("License", 
						xmPushButtonGadgetClass,
						menu_pane,
						XmNactivateCallback, callbacks,
						XmNmnemonic, 'L',
						NULL);

    XtVaCreateManagedWidget( "separator", 
			     xmSeparatorGadgetClass, menu_pane, NULL );

    menuHelpOverviewButton = XtVaCreateManagedWidget("Overview", 
						     xmPushButtonGadgetClass,
						     menu_pane,
						     XmNactivateCallback,
						     callbacks,
						     XmNmnemonic, 'O',
						     NULL);

    menuHelpGuideButton = XtVaCreateManagedWidget("User Guide",
						xmPushButtonGadgetClass,
						menu_pane,
						XmNactivateCallback,
						callbacks,
						XmNmnemonic, 'U',
						NULL);

    menuHelpFAQButton = XtVaCreateManagedWidget("TV FAQ", 
						xmPushButtonGadgetClass,
						menu_pane,
						XmNactivateCallback,
						callbacks,
						XmNmnemonic, 'F',
						NULL);

    menuHelpHomeButton = XtVaCreateManagedWidget("TV Home Page", 
						      xmPushButtonGadgetClass,
						      menu_pane,
						      XmNactivateCallback,
						      callbacks,
						      XmNmnemonic, 'H',
						      NULL);

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, menu_pane ); n++;
    XtSetArg( args[n], XmNmnemonic, 'H' ); n++;
    cascade = XmCreateCascadeButton( menuBar, "Help", args, n );
    XtManageChild (cascade);

    /* Tell Motif that this last menu was the Help menu - this */
    /* will make Motif display it on the right side of the bar */

    XtVaSetValues( menuBar, XmNmenuHelpWidget, cascade , NULL );
}

ThreeDWidget *
TerraVisionInitThreeDWidget (Boolean cave, Boolean mode3d)
{
  static const char *funcName = "TerraVisionInitThreeDWidget";
  ThreeDWidget      *threeD;
  
  /* CREATE THREE D WINDOW */
  threeD = CreateThreeDWidget(applicationShell, cave, mode3d);
  if (! threeD) {
    tsmError (funcName, "Unable to create the 3D widget!");
    return NULL;
  }

  if ( threeD ) {
    int i;

    /* specify the .geo hierarchy/schema URL before starting the viewer */

    tvuInitCore( threeD->threeD );

    /* start the rendering process for the viewer */

    ThreeDWidgetStartRendering (threeD);

    /* perform any -cmd command line option commands */

    for ( i = 0; i < tvuGetInt( TVU_NUM_COMMANDS ); i++ )
      ThreeDWidgetInterpret(threeD, tvuGetStringN( TVU_COMMAND, i ) );
  }


  return threeD;

}

static void
TerraVisionInitAboutDialog (void)
{
  static const char *funcName = "TerraVisionInitAboutDialog";
  if (! aboutDialog) {
    /* CREATE ABOUT DIALOG BOX */
    aboutDialog = CreateAboutDialog(applicationShell,
				    TERRAVISION_RELEASE,
				    TerraVisionReleaseDate);
    if(!aboutDialog) {
      tsmError (funcName, "Unable to create the about dialog structure!");
      return;
    }
  }
}

static void
TerraVisionInitTVLogger (void)
{
  static const char *funcName = "TerraVisionInitTVLogger";
  if (! tvLogger) {
    /* CREATE THE LOGGER FOR CAPTURING TIMING INFORMATION */
    tvLogger = CreateTVLogger (applicationShell);
    if (! tvLogger) {
      tsmError (funcName, "Unable to create Logging struct.");
      return;
    }
  }
}

static ThreeDWidget *
TerraVisionDisplayTwoDWidget( void )
{
  return TerraVisionInitThreeDWidget( False, False );
}

static ThreeDWidget *
TerraVisionDisplayThreeDWidget( void )
{
  return TerraVisionInitThreeDWidget( False, True );
}

/*
 * Synopsis:
 *   void TerraVisionHelp( char *helpfile )
 *
 * Description:
 *   Displays a TerraVision User Guide web page using the default
 *   web browser. The specfied help file will first be looked for
 *   in the $TVHOME/etc/help directory. If this file does not
 *   exist, then we default to the user guide on the TerraVision
 *   web page at http://www.tvgeo.com/user_guide
 *
 *   If helpfile is NULL, then the root help page is displayed
 *   (i.e. index.html in the top most help directory).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April, 1999.
 *
 */

void
TerraVisionHelp( char *helpfile )
{
  char        url[1024];
  PRFileInfo  filetest;

  if ( helpfile == NULL ) helpfile = "welcome.shtml";

  /* do we have the help file in $TVHOME/etc/help */

  sprintf( url, "%s/etc/help/%s", tvuGetString( TVU_INSTALL_DIR ), helpfile );
  
  if ( PR_GetFileInfo ( url, &filetest ) == PR_SUCCESS ) {
    Purl *purl = tsmParseUrl( url );

    if ( purl ) {

      /* display the locally-hosted help page using the web browser */

      tsmShowUrl( purl->url );
      tsmFreePurl( purl );
      return;
    }
  }

  /* if not, then default to the one on the TerraVision web page */

  sprintf( url, "http://www.tvgeo.com/user_guide/%s", helpfile );
  tsmShowUrl( url );
}

/*
 * Synopsis:
 *   void TerraVisionHelpCB( Widget, XtPointer, XtPointer )
 *
 * Description:
 *   This function is a callback wrapper to the TerraVisionHelp routine.
 *   It can be used for X/Motif callback functions that want to display
 *   a help page on activation. The client_data pointer should point
 *   to a string specifying the help file to display from the
 *   TerraVision User Guide (NULL = main User Guide page).
 *
 * Returns: void
 *
 * Author: Martin Reddy
 * 
 * Date: 19 April, 1999.
 *
 */

void
TerraVisionHelpCB( Widget w, XtPointer client_data, XtPointer call_data )
{
  /*static char const funcName[] = "ThreeDWidgetHelpMenuCB";*/
  TerraVisionHelp( (char *) client_data );
}

/*
 * Synopsis: void TerraVisionButtonCB(Widget, XtPointer, XtPointer)
 *   
 *
 * Description:
 *     Most Of The Widget Pushbuttons Have This As Their Activate Callback
 *
 * Externals: Many widgets
 *
 *
 * Returns:  NONE
 *
 * Author:
 *      Stephen Lau
 * 
 * Date: 1992
 *
 */
void
TerraVisionButtonCB(Widget    w,   
		    XtPointer client_data,
		    XtPointer call_data)
{
  /*static const char funcName[] = "TerraVisionButtonCB";*/
  static Widget confirm_clean;

  if (w == menuMapButton) {
    ScannedMapShow();
  }

  else if (w == menuTVLoggingStartButton) {
    TerraVisionStartLogging();
  }

  else if (w == menuTVLoggingStopButton) {
    TerraVisionStopLogging();
  }

  else if (w == menuTVLoggingDirectoryButton) {
    NetLogShow();
  }

  else if (w == menuTVLoggingGraphsButton) {
    TerraVisionInitTVLogger ();
    if (tvLogger)
      TVLoggerShowGraphs (tvLogger);
  }

  else if (w == menuTVLoggingCleanButton) {
    GenericConfirmDialog( applicationShell, &confirm_clean, 
			  "Please confirm that you wish to remove all of\n"
			  "TerraVision's NetLogger log files from the\n"
			  "Log Directory (see Set Logging Options).",
			  (XtCallbackProc) NetLogClean, NULL );
  }

  else if (w == menuDPSSPrefsButton) {
    PrefsShowDPSS( applicationShell );
  }

  else if (w == menuHelpOverviewButton) {
    tsmShowUrl( "http://www.tvgeo.com/pubs/overview/overview.html" );
  }

  else if (w == menuHelpGuideButton) {
    TerraVisionHelp( NULL );
  }

  else if (w == menuHelpHomeButton) {
    tsmShowUrl( "http://www.tvgeo.com/" );
  }

  else if (w == menuHelpFAQButton) {
    tsmShowUrl( "http://www.tvgeo.com/faq.shtml" );
  }

#ifdef DEBUG
  else if (w == menuDEMButton) {
    XtManageChild (Demstatus->Mainform);
  }

  else if (w == menuOIButton) {
    XtManageChild (Oistatus->Mainform);
  }
#endif

  /* DISPLAY THE TWO D OVERHEAD VIEW OF THE TERRAIN */
  else if (w == menuTwoDButton) {
    TerraVisionDisplayTwoDWidget();
  }

  /* DISPLAY THE THREE D WIDGET */
  else if (w == menuThreeDButton) {
    TerraVisionDisplayThreeDWidget();
  }

#ifdef CAVE
  /* DISPLAY THE THREE D WIDGET */
  else if (w == menuCaveButton) {
    TerraVisionStartCAVE();
  }
#endif /* CAVE */

  /* DISPLAY THE QUIT DIALOG BOX TO CONFIRM QUIT */
  else if (w == menuQuitButton) {
    XtManageChild(quitDialog);
  }

  /* DISPLAY THE DATASET INFORMATION WINDOW */

  else if ( w == menuInfoButton ) {
    InfoShow( applicationShell );
  }

  /* DISPLAY THE OPEN TV SELECTION WINDOW */

  else if ( w == menuOpenTVButton ) {
    DBaseShowOpenTV();
  }

  /* DISPLAY THE CLOSE DATASET SELECTION WINDOW */

  else if ( w == menuCloseTVButton ) {
    RemoveDataset();
  }

  /* DISPLAY ABOUT WINDOW */
  else if (w == menuAboutButton) {
    TerraVisionInitAboutDialog ();
    if (aboutDialog) {
      AboutDialogManage (aboutDialog);
    }
  }

  /* DISPLAY THE LICENSE AGREEMENT */
  else if (w == menuLicenseButton) {
    ShowLicense();
  }

  /* QUIT DIALOG BOX */
  else if (w == quitDialog) {
    XtUnmanageChild(w);

    /* OKAY, THEY REALLY WANT TO QUIT
     * THIS IS THE ONLY WAY THE USER CAN EXIT FROM 
     * THE SYSTEM.
     */
    if (((XmAnyCallbackStruct *) call_data)->reason == XmCR_OK) {
      TerraVisionQuit();
    }
  }

  /* WARNING DIALOG BOX */
  else if (w == warningDialog) {
    XtUnmanageChild(warningDialog);
  }
}

/*
 * Synopsis: void TerraVisionDebugLevelCB(Widget, XtPointer, XtPointer)
 *   
 *
 * Description: This is the radio button callback for our DebugLevel changer.
 *
 * Externals: None
 *
 * Returns:  NONE
 *
 * Author: Lee Iverson
 * 
 * Date: December 23, 1996.
 *
 */

static void
TerraVisionDebugLevelCB(Widget    w,   
			XtPointer client_data,
			XtPointer call_data)
{
 static char const funcName[] = "TerraVisionDebugLevelCB";
 int value = (int) client_data;
 
 if (value > 0) {

   /* if no masks are currently set, then set them all so that we */
   /* do get some debugging information generated by default.     */

   if ( tvuGetInt( TVU_TRACE_MASK ) == 0 ) {
     tvuSetInt( TVU_TRACE_MASK, ~0 );
     XtVaSetValues( menuTVDebugTilesButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugTextureButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugInitButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebug2DButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebug3DButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugMapButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugOpenGLButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugVizButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugOAAButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugLogButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugCacheButton, XmNset, True, NULL );
     XtVaSetValues( menuTVDebugDotGeoButton, XmNset, True, NULL );
   }

 }
 pthr_trace_level (value);
 TSMTRACE ((1, funcName, "== Debug level %d", value));
}

/*
 * Synopsis: void TerraVisionDebugMaskCB(Widget, XtPointer, XtPointer)
 *
 * Description: This is the radio button callback for our Debug Mask changer.
 *
 * Externals: None
 *
 * Returns:  NONE
 *
 * Author: Martin Reddy
 * 
 * Date: 3 June, 1998.
 *
 */

static void
TerraVisionDebugMaskCB( Widget    w,   
			XtPointer client_data,
			XtPointer call_data )
{
  static char const funcName[] = "TerraVisionDebugMaskCB";
  int          value = (int) client_data;
  Boolean      status;
  int          trace_level = pthr_trace_level (-1);
  int          tv_trace_mask = tvuGetInt( TVU_TRACE_MASK );
  int          mask = 0;

  XtVaGetValues( w, XmNset, &status, NULL );
 
  switch ( value ) {
  case 0:
    mask = ~0;
    break;

  case 1:
    mask = TSMMASK;
    break;

  case 2:
    mask = TEX_MASK;
    break;

  case 3:
    mask = INIT_MASK;
    break;

  case 4:
    mask = TWOD_MASK | GLT_MASK;
    break;

  case 5:
    mask = THREED_MASK | GLT_MASK;
    break;

  case 6:
    mask = MAP_MASK | GLT_MASK;
    break;

  case 7:
    mask = OGL_MASK | GLT_MASK;
    break;

  case 8:
    mask = VIS_MASK;
    break;

  case 9:
    mask = OAA_MASK;
    break;

  case 10:
    mask = LOG_MASK;
    break;

  case 11:
    mask = CACHE_MASK;
    break;

  case 12:
    mask = DOTGEO_MASK;
    break;

  case 100:
#ifdef DPSS
    {
      extern int Debug;
      Debug = !!status;
    }
#endif
    break;

  case 101:
    __timestamp_debug (status ? 2 : 0);
    break;

#if defined(_pthr_sproc_h_) && DEBUG_PTHREAD
  case 102:
    __pthread_debug (status ? 2 : 0);
    break;
#endif /* _pthr_sproc_h_ */

  default:
    puts( "Not implemented Yet" );
    break;
  }

  /* Turn bits on or off. */
  if ( status )
    tv_trace_mask |= mask;
  else
    tv_trace_mask &= ~mask;
 
  tvuSetInt( TVU_TRACE_MASK, tv_trace_mask );
  TSMTRACE ((1, funcName, "== Debug mask %#x", tv_trace_mask));

  /* if we have any masks set, but the debug level is 0, then */
  /* set the debug level to 1 so that user sees something.    */

  if ( trace_level == 0 && tv_trace_mask != 0 ) {
    pthr_trace_level( 1 );
    XtVaSetValues( menuTVDebugLevel0Button, XmNset, False, NULL );
    XtVaSetValues( menuTVDebugLevel1Button, XmNset, True, NULL );
    XtVaSetValues( menuTVDebugLevel2Button, XmNset, False, NULL );
    XtVaSetValues( menuTVDebugLevel3Button, XmNset, False, NULL );
    XtVaSetValues( menuTVDebugLevel4Button, XmNset, False, NULL );
  }
}

/*
 * Synopsis: void SigCb(int)
 *   
 *
 * Description: Signal handler to capture nasty and not so
 *              nasty signals so we can exit gracefully.
 *
 * Externals:
 *
 * Returns: NONE
 *
 * Author:
 *      Stephen Lau
 * 
 * Date: 1992
 *
 */
void
SigCb(int sd)
{
 /*static char const funcName[] = "SigCb"; */
 PRThread *self;
 pid_t pid;
 char const *name;
 int pass_through = 0;
 int exit_val = sd;

    /* Get the process id and name. */
    pid = getpid ();
    self = PR_GetCurrentThread ();
    name = pthr_name ((void *)self);
    if (! name) name = "<Unregistered>";

    switch(sd)
    {
	case SIGALRM:
	    fprintf(stderr,"PID %d \"%s\": Ring! Ring!\n", (int)pid, name);
	    return;

      	case SIGBUS:
            fprintf(stderr,"PID %d \"%s\": Bus Error.\n", (int)pid, name);
	    pass_through = TRUE;
            break;

      	case SIGSEGV:
            fprintf(stderr,"PID %d \"%s\": Segmentation Fault.\n", (int)pid, name);
	    pass_through = TRUE;
            break;

      	case SIGTERM:
            break;

        case SIGINT:
	    if (self != main_thread)
	      return;

	    /* pthr_describe_threads (); @@ */
	    {
	      int pid;
	      printf ("Send SEGV to a thread? ");
	      fflush (stdout);
	      if (scanf ("%d", &pid) == 1) {
		kill (pid, SIGSEGV);
		break;
	      }
	    } 
	    exit_val = 0;
            break;

      	case SIGTRAP:
            fprintf(stderr,"PID %d \"%s\": Trace Trap.\n", (int)pid, name);
            break;

	default:
	    fprintf(stderr,"PID %d \"%s\": Unknown signal %d.\n", (int)pid, name, sd);
	    break;
    }     

    /* Check if requested to continue with pass_through action. */
    if (pass_through) {
      struct sigaction sa;
      sigset_t set;

      /* Reset signal handler to default. */
      sa.sa_handler = SIG_DFL;
      sigemptyset (&sa.sa_mask);
      sa.sa_flags = SA_RESTART;
      sigaction (sd, &sa, NULL);

      /* Unblock this signal. */
      sigemptyset (&set);
      sigaddset (&set, sd);
      sigprocmask (SIG_UNBLOCK, &set, NULL);

      /* And pass it on. */
      kill (pid, sd);
    } else {
#if 0
      usptr_t *tmp;
      if ((tmp = (usptr_t *) test_and_set ((unsigned long *)&terraArena, 0)))
      {
	TSMTRACE ((INIT_MASK|1, funcName, "Detach arena %p", tmp));
	usdetach (tmp);
      }
#endif
      exit (exit_val);
    }
}

/*
 * Synopsis: TerraVisionTimerCallback()
 *	
 *
 * Description:
 *    Timer for handling miscellanous widget functions like the clock etc
 *
 *
 * Externals:
 *
 *
 * Returns: NULL 
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1993
 *
 */
static XtTimerCallbackProc
TerraVisionTimerCallback(XtPointer client_data,
		  	 XtIntervalId * id)
{
  /*static char const funcName[] = "TerraVisionTimerCallback";*/
  XtAppContext appContext = (XtAppContext) client_data;
  static int tvLoggerCount = 0;
  static int viewpointCount = 0;
  static int scannedMapCount = 0;
  ThreeDPanel *threeDPanel = 0;
  list_iter_t i;  

  /* REDRAW THE SCANNED MAP ONCE IN A WHILE */
  if (scannedMap
      && ScannedMapIsManaged (scannedMap)
      && scannedMapCount >= 10) {
    scannedMapCount = 0;
    ScannedMapRedraw();
  } else {
    scannedMapCount++;
  }

  /* UPDATE OUR CURRENT GL_POSITION DISPLAY */

  if (widgetList) {
    for (i = list_first (widgetList); ! list_done (i); i = list_next (i)) {
      ThreeDWidget *threeD = list_value(i);
      if ((threeDPanel = ThreeDWidgetPanel (threeD))
	  && threeDPanel->managed[POSITION_PANEL]
	  && viewpointCount >= 10) {
	ThreeDPanelUpdatePosition (threeDPanel);
	viewpointCount = 0;
      } else {
	viewpointCount++;
      }
    }
  }

  /* EVERY 50th TIME THROUGH HERE WE UPDATE THE LOGGING WINDOW */
  if (tvLogger
      && TVLoggerIsManaged (tvLogger)
      && tvLoggerCount >= 50) {
    tvLoggerCount = 0;
    TVLoggerUpdateWidgets(tvLogger);
  } else {
    tvLoggerCount++;
  }

  /* RESET THE TIMER */
  widgetTimer = XtAppAddTimeOut(appContext,
				100, 
				(XtTimerCallbackProc)
				TerraVisionTimerCallback,
				appContext);
  return NULL;
}

/*
 * Synopsis: TsGeoPyramid * TVDataSetConnect(TVDataSet *)
 *	
 *
 * Description: Initialize the tile tables,
 *          spawns of the reader threads,
 *          reads in the DEM's
 *
 * Externals:
 *
 *
 * Returns: TRUE - on success
 *          FALSE - on failure
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: 1995
 *
 *
 */
TsGeoPyramid *
TVDataSetConnect(TVDataSet * tsm)
{
  static char *funcName = "TVDataSetConnect";
  TsGeoPyramid *geo;

  TSMTRINC ((TSMMASK|1, 1));
  TSMTRACE ((TSMMASK|1, funcName, "Entering."));

  /* Create a new TsGeoPyramid to be used. */
  geo = TsGeoCreate_OIDEM (tsm->oi_url, tsm->oi_params,
			   tsm->dem_url, tsm->dem_params);
  if (! geo) {
    TSMTRACE ((TSMMASK|1, funcName, "Failed."));
    TSMTRINC ((TSMMASK|1, -1));
    return geo;
  }

  /* Start logging if enabled. */
  TsGeoLogging (geo, isLogging);

  TSMTRACE ((TSMMASK|1, funcName, "Done."));
  TSMTRINC ((TSMMASK|1, -1));
  return geo;
}

/*
 * Synopsis: int TerraVisionGetURLPaths()
 *	
 *
 * Description: Get the URL's for the buttons
 *     out of the configuration file
 *
 *
 * Externals: urlPaths - an array to keep the URL's
 *
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
int
TerraVisionGetURLPaths(void)
{
  /*static char const funcName[] = "TerraVisionGetURLPaths";*/

  /* FIND WHERE THE TEXTURES ARE */
  {
    char const *texture_path;
    if (tvGetResourceAsString ("TEXTURE_PATHNAME", &texture_path))
      TextureSetPath (texture_path);
  }

  if (! tvGetResourceAsString ("TERRAVISION_URL", &urlPaths[TV_PIXMAP])) {
    return FALSE;
  }

  if (! tvGetResourceAsString ("ARPA_URL", &urlPaths[DARPA_PIXMAP])) {
    return FALSE;
  }

  if (! tvGetResourceAsString ("SRI_URL", &urlPaths[SRI_PIXMAP])) {
    return FALSE;
  }

  if (! tvGetResourceAsString ("MAGIC_URL", &urlPaths[MAGIC_PIXMAP])) {
    return FALSE;
  }

  return TRUE;
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
removeDatasetCB (tvBase *threeD, void *closure)
{
  TsGeoPyramid *geo = closure;
  tvRemoveDataset (threeD, geo);
}

static void
TerraVisionRemoveGeoPyramid(TsGeoPyramid *geo)
{
  static char * funcName = "TerraVisionRemoveGeoPyramid";

  TSMTRINC ((INIT_MASK|2, 1));
  TSMTRACE ((INIT_MASK|2, funcName, "Entering."));

  if (! geo) return;

  /* POP DOWN THE VIEWS */
  tvForallWindows (removeDatasetCB, geo);
      
  if (scannedMap && ScannedMapIsManaged (scannedMap)) {
    TSMTRACE ((INIT_MASK|2, funcName, "Dismissing Map."));
    XtUnmanageChild (scannedMap->mainForm);
  }

  TSMTRACE ((INIT_MASK|2, funcName, "Done dismissing windows."));

  /* TOGGLE SOME WIDGETS */
  TSMTRACE ((INIT_MASK|2, funcName, "Resetting buttons."));
  /*
  XtSetSensitive(menuThreeDButton, False);
  XtSetSensitive(menuTwoDButton, False);
  XtSetSensitive(menuMapButton, False);
  if (menuCaveButton)
    XtSetSensitive(menuCaveButton, False);
  */

  /* TELL THE TILE TABLES OF THE CHANGE */
  TSMTRACE ((INIT_MASK|2, funcName, "Resetting data sets."));
  TsGeoDestroy (geo);

  TSMTRACE ((INIT_MASK|2, funcName, "Done."));
  TSMTRINC ((INIT_MASK|2, -1));
}

/*
 * Synopsis: TerraVisionCreateStatusBar(Widget, Widget)
 *	
 *
 * Description: This function creates the status bar
 *      at the bottom of the main window telling
 *      which URL, GATS server we are connected to.
 *
 * Externals: The two status widgets
 *
 *
 * Returns: NONE
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: Jam 12, 1995
 *
 */
static void
TerraVisionCreateStatusBar(Widget mainForm,
			   Widget topWidget)
{
 /*static char const funcName[] = "TerraVisionCreateStatusBar";*/
 Widget formWidget, separator;

    separator = XtVaCreateManagedWidget("separator", 
					xmSeparatorGadgetClass, 
					mainForm,
					XmNwidth, 4,
					XmNshadowThickness, 4,
					XmNtopAttachment, XmATTACH_WIDGET,
					XmNtopWidget, topWidget,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    formWidget = XtVaCreateManagedWidget("form1", 
					 xmFormWidgetClass, 
					 mainForm,
					 XmNtopAttachment, XmATTACH_WIDGET,
					 XmNtopWidget, separator,
					 XmNrightAttachment, XmATTACH_FORM,
					 XmNleftAttachment, XmATTACH_FORM,
					 XmNbottomAttachment, XmATTACH_FORM,
					 NULL);

    separator = XtVaCreateManagedWidget("separator", 
					xmSeparatorGadgetClass, 
					formWidget,
					XmNwidth, 4,
					XmNshadowThickness, 4,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);

    gatsStatusFieldWidget =
       XtVaCreateManagedWidget("gatsStatusLabelWidget",
			       xmLabelGadgetClass,
			       formWidget,
			       XmNleftAttachment, XmATTACH_FORM,
			       NULL);
    XtVaSetValues(separator,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, gatsStatusFieldWidget,
		  NULL);

    separator = XtVaCreateManagedWidget("separator", 
					xmSeparatorGadgetClass, 
					formWidget,
					XmNwidth, 4,
					XmNshadowThickness, 4,
					XmNleftAttachment, XmATTACH_FORM,
					XmNrightAttachment, XmATTACH_FORM,
					NULL);
    XtVaSetValues(gatsStatusFieldWidget,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, separator,
		  NULL);

    tsmStatusFieldWidget = XtVaCreateManagedWidget("tsmStatusLabelWidget",
						   xmLabelGadgetClass,
						   formWidget,
						   XmNbottomAttachment,
						   XmATTACH_FORM,
						   XmNleftAttachment,
						   XmATTACH_FORM,
						   NULL);
    XtVaSetValues(separator,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, tsmStatusFieldWidget,
		  NULL);

    /* Update the status fields of the main GUI */
	
    TerraVisionRefreshStatusFields( TRUE );
}

/*
 * Synopsis: TerraVisionQuit(void)
 *	
 *
 * Description: This function should be 
 *     called when TerraVision exits normally
 *     Things are closed off and killed.
 *
 *
 * Externals: cave - flag for cave usage
 *
 * Returns: Nothing - program exits
 *
 * Author:
 *		Stephen Lau
 * 
 * Date: April 1995
 *
 */
static void
destroyThreeDWidgetCB (ThreeDWidget *threeD, void *closure)
{
  DestroyThreeDWidget (threeD);
}

void
TerraVisionQuit(void)
{
  static char *funcName = "TerraVisionQuit";

  TSMTRINC ((INIT_MASK|1, 1));
  TSMTRACE ((INIT_MASK|1, funcName, "Entering."));

  TerraVisionStopLogging();

  ThreeDWidgetForall (destroyThreeDWidgetCB, NULL);

  /* DESTROY OFF TEREVISION AND THE MAP */
  if (scannedMap)
    DestroyScannedMap (&scannedMap);

  /* Close the tsm. */
  TSMTRACE ((INIT_MASK|TSMMASK|1, funcName, "Close TSM"));
  TSMTRINC ((INIT_MASK|TSMMASK|1, 1));
  while (! list_empty (geoList)) {
    TsGeoPyramid *geo = list_pop (geoList);
    TsGeoDestroy (geo);
  }
  tsmCleanupHttp();
  TSMTRACE ((INIT_MASK|TSMMASK|1, funcName, "TSM Closed."));
  TSMTRINC ((INIT_MASK|TSMMASK|1, -1));

  /* SPIT OUT A MESSAGE TO THE USER */
  srandom( (int) PR_Now() );
  if((random() % 100) < 70) {
    fprintf(stderr,"Thank you for using TerraVision.\n");
  }
  else {
    if((random() % 1000) == 500) {
      fprintf(stderr, "Tax deductible donations accepted.\n");
      fprintf(stderr, "Please direct flames to /dev/null\n");
    }
    else {
      fprintf(stderr,"Thank you for stopping by. "
	      "Please visit us again.\n");
    }
  }

  /* clean up all of the tvutils state (config, viewpoints, etc.) */

  tvuFinish();

  /* THE CAVE SHOULDN'T EXIT */
  TSMTRACE ((INIT_MASK|1, funcName, "Done."));
  TSMTRINC ((INIT_MASK|1, -1));
  if( ! tvuGetInt( TVU_CAVE ) ) {
    TerraVisionExit (0);
  }
}

static int
TerraVisionToggleTVLogger (TsLogFiles log, int flag)
{
  static const char *funcName = "TerraVisionToggleTVLogger";

  if ( flag ) {
    if ( ! TsLogStartLogging( log ) ) {
      tsmError( funcName, "Unable to start tv logging %d", log );
      return FALSE;
    }
  } else {
    if ( ! TsLogEndLogging( log ) ) {
      tsmError( funcName, "Unable to end tv logging %d", log );
      return FALSE;
    }
  }
  return TRUE;
}

void
TerraVisionStopLogging( void )
{
  if (isLogging) {
    isLogging = FALSE;

    TerraVisionToggleTVLogger( TV_REQUEST_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_VISIBLE_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_READER_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_RENDER_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_CACHE_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_TEXTURE_CACHE_LOG, FALSE );
    TerraVisionToggleTVLogger( TV_PREDICT_LOG, FALSE );

    {
      list_iter_t i;
      for (i = list_first (geoList); ! list_done (i); i = list_next (i))
	TsGeoLogging ((TsGeoPyramid *) list_value (i), FALSE);
    }

    XtSetSensitive( menuTVLoggingStartButton, True );
    XtSetSensitive( menuTVLoggingStopButton, False );
  }
}

void
TerraVisionStartLogging( void )
{
  int flag;

  if (! isLogging) {
    isLogging = TRUE;

    TerraVisionInitTVLogger ();

    flag = XmToggleButtonGadgetGetState( menuTVLoggingRequestButton );
    TerraVisionToggleTVLogger( TV_REQUEST_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingVisibleButton );
    TerraVisionToggleTVLogger( TV_VISIBLE_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingReaderButton );
    TerraVisionToggleTVLogger( TV_READER_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingRenderButton );
    TerraVisionToggleTVLogger( TV_RENDER_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingCacheButton );
    TerraVisionToggleTVLogger( TV_CACHE_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingTextureCacheButton );
    TerraVisionToggleTVLogger( TV_TEXTURE_CACHE_LOG, flag );

    flag = XmToggleButtonGadgetGetState( menuTVLoggingPredictButton );
    TerraVisionToggleTVLogger( TV_PREDICT_LOG, flag );

    XtSetSensitive( menuTVLoggingStartButton, False );
    XtSetSensitive( menuTVLoggingStopButton, True );

    {
      list_iter_t i;
      for (i = list_first (geoList); ! list_done (i); i = list_next (i))
	TsGeoLogging ((TsGeoPyramid *) list_value (i), TRUE);
    }
  }
}

void
TerraVisionRestartLogging( void )
{
  /* set logging for all open connections if we are logging */
  /* used to refresh logging state when we load new datasets */
  if ( isLogging ) {
    isLogging = FALSE;
    TerraVisionStartLogging();
  }
}

/*
 * Synopsis: void TerraVisionToggleChangedCB(Widget, XtPointer, XtPointer)
 *
 * Description: When a toggle button is
 *	pressed, tis callback is called. The flag in TerraVision is changed
 *	to reflect the changed state.
 *
 * Externals:
 *
 * Returns: NULL
 *
 * Author:
 *		Stephen Lau
 *
 * Date: 1993
 *
 */
static void
TerraVisionToggleChangedCB(Widget    w,
			   XtPointer client_data,
			   XtPointer call_data)
{
  /*static char const funcName[] = "TerraVisionToggleChangedCB";*/

  /* GET THE STATE OF THE BUTTON AND CHANGE THE APPROPRIATE FLAG */
  int flag = XmToggleButtonGadgetGetState(w);

  /* TOGGLE LOGGING FOR PERFORMANCE MEASUREMENTS */
  if ( isLogging ) {

    /* TOGGLE TILE REQUEST LOGGING */
    if (w == menuTVLoggingRequestButton) {
      TerraVisionToggleTVLogger (TV_REQUEST_LOG, flag);
    }

    /* TOGGLE VISIBILITY CALC LOGGING */
    else if (w == menuTVLoggingVisibleButton) {
      TerraVisionToggleTVLogger (TV_VISIBLE_LOG, flag);
    }

    /* TOGGLE TILE READER LOGGING */
    else if (w == menuTVLoggingReaderButton) {
      TerraVisionToggleTVLogger (TV_READER_LOG, flag);
    }

    /* TOGGLE RENDER TIME LOGGING */
    else if (w == menuTVLoggingRenderButton) {
      TerraVisionToggleTVLogger (TV_RENDER_LOG, flag);
    }

    /* TOGGLE CACHE LOGGING */
    else if (w == menuTVLoggingCacheButton) { 
      TerraVisionToggleTVLogger (TV_CACHE_LOG, flag);
    }

    /* TOGGLE TEXTURE CACHE LOGGING */
    else if (w == menuTVLoggingTextureCacheButton) {
      TerraVisionToggleTVLogger (TV_TEXTURE_CACHE_LOG, flag);
    }

    /* TOGGLE PREDICTION LOGGING */
    else if (w == menuTVLoggingPredictButton) {
      TerraVisionToggleTVLogger (TV_PREDICT_LOG, flag);
    }
  }
}
