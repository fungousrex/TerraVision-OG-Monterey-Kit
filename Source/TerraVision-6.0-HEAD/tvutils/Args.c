/*
  Filename: Args.c

  Description: Contains platform-specific parsing of command line arguments
  for TerraVision applications.

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

#include <pthr/trace.h>
#include <pthr/timestamp.h>
#include <tsm/tsm.h>
#include <TerraVision/Trace.h>
#include <TerraVision/OAA.h>
#include <glt/glt.h>

#include "tvutils.h"
#include "Init.h"
#include "Args.h"
#include "Platform.h"

#include "RELEASE"


/* here is the internal state for the parameters */
/* that can be controlled from the command line. */

int   args_parsed = 0;                  /* number of arguments parsed */
int   args_total = 0;                   /* total number of arguments given */
float drop_to_wireframe = 2.0;          /* frame rate to reduce complexity */
FILE  *debug_file = NULL;               /* file handle for debug output file */
char  *debug_filename = NULL;           /* filename for debug output */
char  *schema_url = NULL;               /* url for .geo schema */
int   tv_trace_mask = 0;                /* trace mask set from command line */
char  *initialDem[MAX_URLS];            /* list of initial DEMs */
char  *initialOi[MAX_URLS];             /* list of initial OIs */
int   numInitialDems = 0;               /* size of initial DEM list */
int   numInitialOis = 0;                /* size of initial OI list */
int   initial_viewer = 0;               /* open 2d or 3d viewer (0=none)? */
int   stereoVisual = FALSE;             /* Request a stereo visual */
int   singleBuffer = FALSE;             /* Render in single buffer mode */
int   tileByTile = FALSE;               /* tile-by-tile debug mode. */
char  *cmd_list[MAX_CMDS];              /* list of commands from -cmd */
int   cmd_list_size = 0;                /* number of -cmd commands found */
char  *config_file = NULL;              /* pathname to the config file */
int   XErrorAbort = FALSE;              /* do an abort() on an X error */
int   showLicense = FALSE;              /* show the TerraVision license */
int   logOnStart  = FALSE;              /* start logging on startup? */
int   no_ipc = FALSE;                   /* allow commands typed on stdin */
int   ns_win_id = 0;                    /* Netscape plugin window ID */
int   width = 0;                        /* requested window width */
int   height = 0;                       /* requested window height */
int   cave = FALSE;                     /* is the CAVE on? */
int   no_config = FALSE;                /* ignore the configuration file? */
char  *path_url = NULL;                 /* url to a TerraVision path file */
char  *viewpoint_url = NULL;            /* url to a viewpoint file */
char  *database_url = NULL;             /* url to the database file */
char  *license_accepted = NULL;         /* accepted TV version license */
char  *profile_url = NULL;              /* .geo profile XML file to use */
char  *tcl_path = NULL;                 /* path to TerraVision.tcl */
int   ogl_visual_id = -1;               /* ID of desired OpenGL visual */
int   list_visuals = 0;                 /* list all OGL visuals and scores */

static void
tvuUsage( void )
{
  static char const usage[] = 
    "Usage: TerraVision [OPTIONS] ...\n\n"
    "-help or -h           - This printout\n"
    "-debug [MASK:]LEVEL   - Set debug level [1-4] and masks [GATtig23movO]\n"
    "-debug_file FILENAME  - Output debug information to this file\n"
    "-debug_ts LEVEL       - Output debug timestamp information\n"
    "-license              - Display the TerraVision license on startup\n"
#ifdef CAVE
    "-cave                 - CAVE version of TerraVision\n"
#endif
    "-oi OI_URL            - URL(s) of OI (ortho image pyramid).\n"
    "-dem DEM_URL          - URL(s) of DEM (elevation pyramid).\n"
    "-model VRML_URL       - URL(s) of GeoVRML 3D models to load.\n"
    "-schema URL           - URL of the .geo schema to load\n"
    "-profile URL          - URL of a .geo profile to use\n"
    "-dotgeo               - Connect to prototype .geo schema at SRI\n"
    "-proxy URL            - URL of your proxy server for http requests\n"
    "-view [2D|3D]         - Initial viewer to use\n"
    "-cmd OAA_CMD          - Execute this OAA command on startup\n"
    "-path NAME            - Initial path to load and play in 3-D viewer\n"
    "-viewpoints URL       - Specify URL for the Viewpoints file\n"
    "-droptowireframe HZ   - Specify frame rate to drop to wireframe\n"
    "-oaa HOST:PORT        - Set the host/port for the OAA facilitator\n"
    "-oaaid NAME           - Name of the OAA ID for the default viewer\n";

  static char const usage2[] =
    "-width N              - The width of the default canvas to create\n"
    "-height N             - The height of the default canvas to create\n"
#ifdef TVU_WINDOWS
    "-visualid N           - Pixel Format ID for desided visual\n" 
#endif
#ifdef TVU_UNIX
    "-listvisuals          - List all OpenGL visuals and their scores\n"
    "-visualid N           - Use the visual with ID list from -listvisuals\n"
#endif
    "-config filename      - Specify a different config.xml file\n"
    "-noconfig             - Do not load the config.xml file\n"
    "-version or -v        - Display version number and exit immediately\n";

  printf("%s", usage);

  if ( tvu_mode == TVU_MOTIF )
    printf(
      "-log                  - Begin logging immediately upon start up\n"
    );

  if ( tvu_mode == TVU_PLUGIN )
    printf(
      "-nswinid ID           - Window ID for Netscape plugin version\n"
    );

  if ( tvu_mode == TVU_TCLTK ) 
    printf(
      "-tcldir DIR           - Set the directory with TerraVision.tcl file\n"
    );

  printf("%s", usage2);

  exit( 1 );
}

static void
tvuPrintReleaseInfo( void )
{
  char const *version = TERRAVISION_RELEASE;
  char tv_date[100], tvu_date[100];
  int i, j;

  i = 0;
  while( TerraVisionReleaseDate[i] != ':' )
    i++;

  j = 0;
  i+= 2;
  while( TerraVisionReleaseDate[i] != '$')
    tv_date[j++] = TerraVisionReleaseDate[i++];
  tv_date[j-1] = 0;

  i = 0;
  while( tvuReleaseDate[i] != ':' )
    i++;

  j = 0;
  i+= 2;
  while( tvuReleaseDate[i] != '$')
    tvu_date[j++] = tvuReleaseDate[i++];
  tvu_date[j-1] = 0;

  printf( "TerraVision II: Release %s, %s.\n", version, tv_date );
  printf( "TVUtils Release %1.1f, %s.\n", TVU_RELEASE, tvu_date );
  tsmPrintReleaseInfo( stdout );
}


int
tvuParseArgArray( int argc, char **argv )
{
  int i;

  /* record the number of args supplied (ignore the command name) */

  args_total = argc - 1;
  
  /* initialise the debug generation routines */

  pthr_trace_stream (stderr);
  pthr_trace_html (FALSE);
  pthr_trace_print_pid (TRUE);

  for ( i = 0; i < MAX_URLS; i++ )
    initialOi[i] = initialDem[i] = NULL;

  /* first of all search for any config file information, because */
  /* we would like to load the config file before parsing the     */
  /* command line arguments (so that command line overrides).     */

  for (i = 1; i < argc; i++) {

    if (! strcmp (argv[i], "-noconfig")) {
      no_config = TRUE;
      args_parsed++;

    } else if (! strcmp (argv[i], "-config")) {
      if ( ++i < argc ) {
	config_file = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no filename specified for -config option\n" );
    }
  }

  /* Now, load the appropriate configuration file */

  if ( no_config == FALSE )
    tvuLoadConfig();

  /* Handle the rest of the command line arguments */

  for (i = 1; i < argc; i++) {

    if (! strcmp (argv[i], "-debug")) {
      int trace_level;
      if ( ++i >= argc ) {
	fprintf( stderr, "no level specified for -debug option\n" );
	continue;
      }
      if (strchr (argv[i], ':')) {
	char *p = argv[i];
	tv_trace_mask = 0;
	for ( ; *p != ':'; ++p) {
	  switch (*p) {
	  case 'A':
	    tv_trace_mask = ~0;
	    break;
	  case 'T':
	    tv_trace_mask |= TSMMASK;
	    break;
	  case 't':
	    tv_trace_mask |= TEX_MASK;
	    break;
	  case 'i':
	    tv_trace_mask |= INIT_MASK;
	    break;
	  case 'g':
	    tv_trace_mask |= GLT_MASK;
	    break;
	  case 'G':
	    tv_trace_mask |= DOTGEO_MASK;
	    break;
	  case '2':
	    tv_trace_mask |= TWOD_MASK;
	    break;
	  case '3':
	    tv_trace_mask |= THREED_MASK;
	    break;
	  case 'm':
	    tv_trace_mask |= MAP_MASK;
	    break;
	  case 'o':
	    tv_trace_mask |= OGL_MASK | GLT_MASK;
	    break;
	  case 'O':
	    tv_trace_mask |= OAA_MASK;
	    break;
	  case 'v':
	    tv_trace_mask |= VIS_MASK;
	    break;
	  case 'l':
	    tv_trace_mask |= LOG_MASK;
	    break;
	  case 'c':
	    tv_trace_mask |= CACHE_MASK;
	    break;
	  default:
	    fprintf (stderr, "Unrecognized debug type '%c'\n", *p);
	    break;
	  }
	}
	trace_level = atoi (&p[1]);
      } else {
	tv_trace_mask = ~0;
	trace_level = atoi (argv[i]);
      }
      
      pthr_trace_level (trace_level);
      pthr_trace_mask (tv_trace_mask);
      args_parsed += 2;

    } else if (!strcmp(argv[i], "-debug_file")) {
      if ( ++i < argc ) {
	FILE *old_debug_file = debug_file;

	if ( ( debug_file = fopen( argv[i], "w" ) ) == NULL ) 
	  fprintf( stderr, "couldn't create log file: %s\n", argv[i] );
	else {
	  pthr_trace_stream( debug_file );
	  if ( old_debug_file )
	    fclose( old_debug_file );
	  if ( debug_filename ) free( debug_filename );
	  debug_filename = strdup( argv[i] );
	  args_parsed += 2;
	}
      } else
	fprintf( stderr, "no filename specified for -debug_file option\n" );
    }

#if defined(_pthr_sproc_h_) && DEBUG_PTHREAD
    else if (! strcmp (argv[i], "-debug_pthread")) {
      if ( ++i < argc ) {
	__pthread_debug (atoi (argv[i]));
	args_parsed++;
      } else
	fprintf( stderr, "no level specified for -debug_pthread option\n");
    }
#endif /* _pthr_sproc_h_ */

    else if (! strcmp (argv[i], "-debug_ts")) {
      if ( ++i < argc ) {
	__timestamp_debug (atoi (argv[i]));
	args_parsed += 2;
      } else
	fprintf( stderr, "no level specified for -debug_ts option\n" );

    } else if (! strcmp(argv[i], "-dem")) {
      if ( numInitialDems >= MAX_URLS )
	fprintf( stderr, "too many -dem options specified!\n" );
      else if ( ++i < argc ) {
	initialDem[numInitialDems++] = strdup (argv[i]);
	args_parsed += 2;
      } else
	fprintf( stderr, "no URL specified for -dem option\n" );

    } else if (! strcmp(argv[i], "-oi")) {
      if ( numInitialOis >= MAX_URLS )
	fprintf( stderr, "too many -oi options specified!\n" );
      if ( ++i < argc ) {
	initialOi[numInitialOis++] = strdup (argv[i]);
	args_parsed += 2;
      } else
	fprintf( stderr, "no URL specified for -oi option\n" );

    } else if (! strcmp(argv[i], "-geo")) {
      ++i; /* deprecated - ignore -geo's parameter too */
      args_parsed += 2;

    } else if (!strcmp(argv[i], "-view")) {
      if ( ++i < argc ) {
	initial_viewer = ( argv[i][0] != '2' ) ? 3 : 2;
	args_parsed += 2;
      } else
	fprintf( stderr, "no mode specified for -view option\n" );

    } else if (!strcmp(argv[i], "-tcldir")) {
      if ( ++i < argc ) {
	if ( tcl_path ) free( tcl_path );
	tcl_path = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no directory specified for -tcldir option\n" );

    } else if (!strcmp(argv[i], "-droptowireframe")) {
      if ( ++i < argc ) {
	drop_to_wireframe = (float) max( 0.0, atof( argv[i] ) );
	args_parsed += 2;
      } else
	fprintf( stderr, "no value specified for -droptowireframe option\n" );

    } else if (! strcmp (argv[i], "-singleBuffer")) {
      singleBuffer = TRUE;
      args_parsed++;

    } else if (! strcmp (argv[i], "-tileByTile")) {
      singleBuffer = TRUE;
      tileByTile = TRUE;
      args_parsed++;

    } else if (! strcmp (argv[i], "-proxy")) {
      if ( ++i < argc ) {
	tsmSetHttpProxy( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no url specified for -proxy option\n" );

    } else if (! strcmp (argv[i], "-stereo")) {
      stereoVisual = TRUE;
      args_parsed++;
    }

#ifdef TVU_UNIX
    else if (!strcmp(argv[i], "-xabort")) {
      XErrorAbort = TRUE;
      args_parsed++;
    }
#endif

#ifdef CAVE
    else if (!strcmp(argv[i], "-cave")) {
      cave = TRUE;
      args_parsed++;
    }
#endif /* CAVE */

    else if (!strcmp(argv[i], "-display")) {
      i++;
      args_parsed++;
      continue;

    } else if (!strcmp(argv[i], "-license")) {
      showLicense = TRUE;
      args_parsed++;

    } else if (!strcmp(argv[i], "-log")) {
      logOnStart = TRUE;
      args_parsed++;

    } else if (!strcmp(argv[i], "-nswinid")) {
      if ( ++i < argc ) {
	ns_win_id = atoi( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no window ID specified for -nswinid option\n" );

    } else if (!strcmp(argv[i], "-width")) {
      if ( ++i < argc ) {
	width = atoi( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no width specified for -width option\n" );

    } else if (!strcmp(argv[i], "-height")) {
      if ( ++i < argc ) {
	height = atoi( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no height specified for -height option\n" );

    } else if (! strcmp (argv[i], "-listvisuals")) {
      list_visuals = TRUE;
      args_parsed++;

    } else if (!strcmp(argv[i], "-visualid")) {
      if ( ++i < argc ) {
	ogl_visual_id = atoi( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no ID specified for -visualid option\n" );

    } else if (!strcmp(argv[i], "-path")) {
      if ( ++i < argc ) {
	if ( path_url ) free( path_url );
	path_url = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no name specified for -path option\n" );

    } else if (!strcmp(argv[i], "-viewpoints")) {
      if ( ++i < argc ) {
	if ( viewpoint_url ) free( viewpoint_url );
	viewpoint_url = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no url specified for -viewpoints option\n" );

    } else if (!strcmp(argv[i], "-schema")) {
      if ( ++i < argc ) {
	if ( schema_url ) free( schema_url );
	schema_url = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no url specified for -schema option\n" );

    } else if (!strcmp(argv[i], "-profile")) {
      if ( ++i < argc ) {
	if ( profile_url ) free( profile_url );
	profile_url = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no url specified for -profile option\n" );

    } else if (!strcmp(argv[i], "-dotgeo")) {
      if ( schema_url ) free( schema_url );
      schema_url = strdup( "http://avila.ai.sri.com/TileSets/Schema/schema.xml" );
      args_parsed++;

    } else if (!strcmp(argv[i], "-no_ipc")) {
      no_ipc = TRUE;
      args_parsed++;

    } else if (!strcmp(argv[i], "-oaa")) {
      if ( ++i < argc ) {
	SetOAAHostPort( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no host/port specified for -oaa option\n" );

    } else if (!strcmp(argv[i], "-oaaid")) {
      if ( ++i < argc ) {
	SetOAADefaultID( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no ID string specified for -oaaid option\n" );

    } else if (!strcmp(argv[i], "-cmd")) {
      if ( ++i < argc && cmd_list_size < MAX_CMDS ) {
	cmd_list[cmd_list_size++] = strdup( argv[i] );
	args_parsed += 2;
      } else
	fprintf( stderr, "no command specified for -cmd option\n" );

    } else if (!strcmp(argv[i], "-model")) {
      if ( ++i < argc && cmd_list_size < MAX_CMDS ) {
        char cmd[512], *ptr;
        sprintf( cmd, "loadmodel('%s')", argv[i] );
	/* convert PC slashes to unix ones */
	while ( ( ptr = strchr( cmd, '\\' ) ) != NULL ) *ptr = '/';
	cmd_list[cmd_list_size++] = strdup( cmd );
	args_parsed += 2;
      } else
	fprintf( stderr, "no url specified for -model option\n" );

    } else if ( !strcmp(argv[i], "-help") ||
		!strcmp(argv[i], "-h") ||
		!strcmp(argv[i], "?") ) {
      args_parsed++;
      tvuUsage();

    } else if ( !strcmp(argv[i], "-v" ) ||
		!strcmp(argv[i], "-version" ) ) {
      tvuPrintReleaseInfo();
      args_parsed++;
      exit( 1 );
    }
  }

  /* issue an error if number of -oi URLs not the same as the -dem URLs */

  /*
  if ( numInitialDems != numInitialOis ) {
    fprintf( stderr, "Error: mismatched number of -oi and -dem options\n" );
    exit( 0 );
  }
  */

  /* for a plugin version, setup a default dataset if none specified */

  if ( tvu_mode == TVU_PLUGIN ) {

#if 1
#define GEO_HOME "http://avila.ai.sri.com"
#else
#define GEO_HOME "http://www.tvgeo.com"
#endif

    if ( numInitialDems == 0 && schema_url == NULL ) {
      numInitialDems = 2;
      numInitialOis  = 2;
      initialDem[0]  = strdup (GEO_HOME "/TileSets/Pyramids/gtopo30.dem");
      initialOi[0]   = strdup (GEO_HOME "/TileSets/Pyramids/gtopo30.oi");
      initialDem[1]  = strdup (GEO_HOME "/TileSets/Pyramids/sfbay.dem");
      initialOi[1]   = strdup (GEO_HOME "/TileSets/Pyramids/sfbay.oi");
    }
  }
  
  return args_parsed;
}

int
tvuParseArgString( char const *args )
{
  int   argc = 0, parsed;
  char  *argv[100] = { NULL }, *arg_copy;
  char  *p, *q;

  if ( args == NULL ) return 0;

  /* set the first argument to the .exe name */

  argv[0] = "TerraVision.exe";

  /* take a copy of the argument string as we will modify it */

  if ( ( arg_copy = strdup( args ) ) == NULL ) return 0;

  /* Parse command line handling quotes. */

  p = (char *) arg_copy;
  while (*p) {
    /* for each argument */
    while ((*p) && (*p == ' '))
      p++;    /* skip over leading spaces */
    if (*p == '\042') {
      p++;             /* skip " */
      q = p;
      /* scan to end of argument - doesn't handle embedded quotes */
      while ((*p) && (*p != '\042'))
        p++;
      argv[++argc] = q;
      if (*p)
        *p++ = '\0';
    }
    else if (*p) {
      /* delimited by spaces */
      q = p;
      while ((*p) && (*p != ' '))
        p++;
      argv[++argc] = q;
      if (*p)
        *p++ = '\0';
    }
  }
  argv[++argc] = NULL;

  /* now we have created an argc/argv array, use tvuParseArgArray */

  parsed = tvuParseArgArray( argc, argv );

  /* clean up */

  free( arg_copy );

  return parsed;
}

void
tvuArgFinish( void )
{
  int i;

  /* close the debug output file */

  if ( debug_file ) {
    pthr_trace_stream( stderr );
    fclose( debug_file );
    debug_file = NULL;
  }

  /* free any arrays of strings we allocated */

  for ( i = 0; i < numInitialOis; i++ )
    if ( initialOi[i] )
      free( initialOi[i] );
  numInitialOis = 0;

  for ( i = 0; i < numInitialDems; i++ )
    if ( initialDem[i] )
      free( initialDem[i] );
  numInitialDems = 0;

  for ( i = 0; i < cmd_list_size; i++ )
    if ( cmd_list[i] )
      free( cmd_list[i] );
  cmd_list_size = 0;

  /* free any strings we allocated */

  if ( schema_url ) {
    free( schema_url );
    schema_url = NULL;
  }

  if ( debug_filename ) {
    free( debug_filename );
    debug_filename = NULL;
  }

  if ( config_file ) {
    free( config_file );
    config_file = NULL;
  }

  if ( path_url ) {
    free( path_url );
    path_url = NULL;
  }

  if ( viewpoint_url ) {
    free( viewpoint_url );
    viewpoint_url = NULL;
  }
}
