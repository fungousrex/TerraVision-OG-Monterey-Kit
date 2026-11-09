#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <TerraVision/Trace.h>
#include <tvutils/tvutils.h>

#include "tcl.h"
#include "tk.h"
#include "togl.h"
#include "tvBaseCmds.h"
#include "pthrCmds.h"
#include "tsGeoPyramidCmds.h"
#include "tsmCmds.h"

static int TclTVInit(Tcl_Interp*);

#ifdef HAVE_LIBGEO
# ifdef WIN32
#  ifdef _DEBUG
#   pragma comment( lib, "libGeo_Core_d.lib" )
#   pragma comment( lib, "libGeo_DiscClient_d.lib" )
#  else
#   pragma comment( lib, "libGeo_Core.lib" )
#   pragma comment( lib, "libGeo_DiscClient.lib" )
#  endif 
# endif
#endif

/*
 * m a i n ( )
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
#ifdef WIN32
   /* Set up dummy argc/argv variables under Windows */
   int argc = 1;
   char *argv[2] = { "TclTV.exe", "" };
#endif

   /* initialise the tvutils library */
   tvuInit( TVU_TCLTK );

   /* clear our communication list for talking to Tcl */
   tvMsgListClear();

   /* parse the config file and command line arguments */
#ifdef WIN32
  tvuParseArgString( cmdlin );
#else
  tvuParseArgArray( argc, argv );
#endif

   if ( tvuGetInt( TVU_ARGS_PARSED ) < tvuGetInt( TVU_TOTAL_ARGS ) ) {
     printf( "TerraVision: unrecognized option(s) specified\n"
	     "Try TerraVision -h for valid options.\n\n" );
     return 0;
   }

   /* enter the Tcl/Tk main loop */
   Tk_Main(argc, argv, TclTVInit);

   return 0;
}



/*
 * T c l T V I n i t ( )
 *
 * Called by TK_Main();  installs our new commands and otherwise sets up the
 * Tcl/Tk environment.
 *
 */
int 
TclTVInit(Tcl_Interp *interp)
{
   char *tcl_filename;

   /* 
    * Try to find the TerraVision.tcl root file. Fail if not found
    */

   if ( ( tcl_filename = tvuGetString( TVU_TCL_FILENAME ) ) == NULL ) {
     printf( "Could not find the TerraVision.tcl file (try -tcldir)\n" );
     return TCL_ERROR;
   }
  
   /*
    * Initialize Tcl, Tk, and the Togl widget module.
    */
   if (Tcl_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
   }

   if (Tk_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
   }

#ifdef WIN32
    /*
     * Set up a console window. Delete the following statement if you do not need that.
     */
   /*
    if (TkConsoleInit(interp) == TCL_ERROR) {
           return TCL_ERROR;
    }
    */
#endif /* WIN32 */

   if (Togl_Init(interp) == TCL_ERROR) {
      return TCL_ERROR;
   }

   /*
    * Specify the callback functions for widget creation, display,
    * and reshape.
    */
   Togl_CreateFunc(tvToglCreateCB);
   Togl_DisplayFunc(tvToglDisplayCB);
   Togl_ReshapeFunc(tvToglReshapeCB);
   Togl_DestroyFunc(tvToglDestroyCB);

   /* install sub-commands for all Togl instances... */
   Togl_CreateCommand("path", tvPathCommand);
   Togl_CreateCommand("view", tvViewCommand);
   Togl_CreateCommand("set",  tvSetCommand);
   Togl_CreateCommand("get",  tvGetCommand);
   Togl_CreateCommand("link", tvLinkCommand);
   Togl_CreateCommand("feature", tvFeatureCommand);
   Togl_CreateCommand("log",    tvLogCommand);
   Togl_CreateCommand("OAA",    tvOAACommand);
   Togl_CreateCommand("geoset", tvGeosetCommand);
   Togl_CreateCommand("destroy", tvDestroyCommand);
   Togl_CreateCommand("stroke",  tvStrokeCommand);
   Togl_CreateCommand("anchor",  tvAnchorCommand);

   /*
    * create some additional support commands...
    */
   Tcl_CreateObjCommand(interp, "tsGeoPyramid", tsGeoPyramidProc, NULL, NULL);
   Tcl_CreateObjCommand(interp, "tsmDatabase", tsmDatabaseCommand, NULL, NULL);
   Tcl_CreateObjCommand(interp, "tsmUtil", tsmUtilCommand, NULL, NULL);
   Tcl_CreateObjCommand(interp, "pthr", pthrProc, NULL, NULL);
   Tcl_CreateObjCommand(interp, "tvutils", tvutilsInterface, NULL, NULL);

   /*
    * miscellaneous initialization...
    */
   tvBaseCmdsInit();

   /*
    * Specify our startup script, which when mapped to the name 
    * "tcl_rcFileName", will be loaded automatically by Tcl.
    */

   /* Tcl_SetVar(interp, "tcl_rcFileName", tcl_filename, TCL_GLOBAL_ONLY); */

   if (Tcl_EvalFile(interp, tcl_filename) == TCL_ERROR) {
       return TCL_ERROR;
   }

   return TCL_OK;
}


