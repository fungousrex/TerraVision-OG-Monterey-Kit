/*
 * Name: tsmArg.c
 *
 * Description:
 *   Contains general routines to parse command line arguments, as well
 *   as routines that parse the arguments looking for tsmApi specific
 *   options and acting upon these in someway. These are designed to make
 *   life simpler for people developing command line tools built on the
 *   tsmApi.
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:  March 1998 - Initial version by Martin Reddy
 * 
 *   $Id: tsmArg.c,v 1.30.2.5 2001/07/24 00:48:02 reddy Exp $
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
 *
 */

#include "tsmInternal.h"
#include "tsmConnectionInt.h"
#include "tsmConnectionParamsInt.h"

static int  tsm_argc = 0;
static char **tsm_argv = NULL;
static int  *tsm_argb = NULL;

/*
 * Synopsis:
 *   void tsmGrabArgs( int argc, char **argv )
 *   void tsmFreeArgs( void )
 *   void tsmSetArg( int pos )
 *   int tsmIsArgSet( int pos )
 *
 * Description:
 *   These functions are used to keep track of which command line
 *   arguments have been matched by tsmApi.  tsmGrabArgs grabs an
 *   internal copy of command line and makes an array of booleans to
 *   record which options have been matched.  tsmFreeArgs frees the
 *   boolean array. tsmSetArg records an option as being
 *   matched. tsmIsArgSet asks if an option has been matched
 *
 * Author: Martin Reddy
 *
 * Date: 3 June 1998
 *
 */

static
void tsmGrabArgs( int argc, char **argv )
{
  if ( tsm_argv != NULL ) return;

  tsm_argc = argc;
  tsm_argv = argv;
  tsm_argb = (int *) tsmMemCalloc( argc, sizeof(int) );
}

static
void tsmFreeArgs( void )
{
  if ( tsm_argb == NULL ) return;
  tsmMemFree( tsm_argb );
  tsm_argc = 0;
  tsm_argv = NULL;
  tsm_argb = NULL;
}

static
void tsmSetArg( int pos )
{
  if ( tsm_argb == NULL ) return;
  if ( pos < 0 || pos >= tsm_argc ) return;
  tsm_argb[pos] = TRUE;
}

static
int tsmIsArgSet( int pos )
{
  if ( tsm_argb == NULL ) return FALSE;
  if ( pos < 0 || pos >= tsm_argc ) return FALSE;
  return tsm_argb[pos];
}

/*
 * Synopsis:
 *   int tsmGetArg( int argc, char **argv, char *arg )
 *
 * Doc++ Description: *//** 
 *   Check if an option was supplied on the command line.
 *
 *   This function will take the argc and argv variables that were
 *   passed to an application's main() function and will then test
 *   to see if a specific option string was supplied in the command
 *   line. The string comparison is case sensitive.
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 * @param    arg  an argument to test for in the argv array
 *
 * @return   TRUE if arg exists in argv, FALSE if not.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmGetArgInt, tsmGetArgFloat, tsmGetArgString,
 *           tsmParseInitArgs, tsmParseReadArgs
 *
 */

int
tsmGetArg( int argc, char **argv, char *arg )
{
  int i;

  tsmGrabArgs( argc, argv );

  for ( i = 1; i < argc; ++i ) {
    if ( strcmp( argv[i], arg ) == 0 ) {
      tsmSetArg( i );
      return TRUE;
    }
  }
  return FALSE;
}

/*
 * Synopsis:
 *    int tsmGetArgPointer( int argc, char **argv, char *arg, char **value )
 *
 * Description:
 *    This is the generic function that is used be tsmGetArgInt, 
 *    tsmGetArgFloat, and tsmGetArgString in order to return the
 *    value associated with an option on the command line.
 *
 *    N.B. the function can still return TRUE and return NULL in
 *    *value. This means that the option string was found, but no
 *    value was associated with that option. 
 *
 * Returns: TRUE if the option was found on the command line, FALSE if not
 *
 * Author: Martin Reddy
 *
 * Date: 10 March 1998
 *
 */

static int
tsmGetArgPointer( int argc, char **argv, char *arg, char **value )
{
  int i;

  tsmGrabArgs( argc, argv );

  *value = NULL;
  for ( i = 1; i < argc; ++i ) {
    if ( strcmp( argv[i], arg ) == 0 && tsmIsArgSet(i) == FALSE ) {
      tsmSetArg( i );
      if ( i >= argc-1 ) {
	return TRUE;
      } else {
	*value = argv[++i];
	tsmSetArg( i );
	return TRUE;
      }
    }
  }
  return FALSE;
}

/*
 * Synopsis:
 *   int tsmGetArgString( int argc, char **argv, char *arg, char **value )
 *
 * Doc++ Description: *//** 
 *   Get the string after an option on the command line.
 *
 *   This function will take the argc and argv variables that were
 *   passed to an application's main() function and will then test
 *   to see if a specific option string was supplied in the command
 *   line. If it is found, then the string directly after that option
 *   is returned. N.B. the string is not copied - it is a pointer to
 *   the actual argv string, so don't free it. You should take a copy
 *   of the string with strdup() if you want to alter it.
 *
 *   The string comparison is case sensitive.
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 * @param    arg  an argument to test for in the argv array
 * @param    value a pointer to an string to be filled with the option's text
 *
 * @return   TRUE if the option was found and it's value placed in the
 *           value variable, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmGetArg, tsmGetArgInt, tsmGetArgFloat
 *
 */

int
tsmGetArgString( int argc, char **argv, char *arg, char **value )
{
  static const char *funcName = "tsmGetArgString";
  char              *string;

  if ( tsmGetArgPointer( argc, argv, arg, &string ) == FALSE )
    return FALSE;

  if ( string == NULL ) {
    tsmError( funcName, "no string value supplied for %s option", arg );
    return FALSE;
  }

  *value = string;
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmGetArgInt( int argc, char **argv, char *arg, int *value )
 *
 * Doc++ Description: *//** 
 *   Get the integer after an option on the command line.
 *
 *   This function will take the argc and argv variables that were
 *   passed to an application's main() function and will then test
 *   to see if a specific option string was supplied in the command
 *   line. If it is found, then the string directly after that option
 *   is converted to an integer value and returned.
 *
 *   The string comparison is case sensitive.
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 * @param    arg  an argument to test for in the argv array
 * @param    value a pointer to an integer to be filled with the option's value
 *
 * @return   TRUE if the option was found and it's value placed in the
 *           value variable, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmGetArg, tsmGetArgFloat, tsmGetArgString
 *
 */

int
tsmGetArgInt( int argc, char **argv, char *arg, int *value )
{
  static const char *funcName = "tsmGetArgInt";
  char              *string;

  if ( tsmGetArgPointer( argc, argv, arg, &string ) == FALSE )
    return FALSE;

  if ( string == NULL ) {
    tsmError( funcName, "no integer value supplied for %s option", arg );
    return FALSE;
  }

  *value = atoi(string);
  return TRUE;
}

/*
 * Synopsis:
 *   int tsmGetArgFloat( int argc, char **argv, char *arg, double *value )
 *
 * Doc++ Description: *//** 
 *   Get the double after an option on the command line.
 *
 *   This function will take the argc and argv variables that were
 *   passed to an application's main() function and will then test
 *   to see if a specific option string was supplied in the command
 *   line. If it is found, then the string directly after that option
 *   is converted to a double floating-point value and returned.
 *
 *   The string comparison is case sensitive.
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 * @param    arg  an argument to test for in the argv array
 * @param    value a pointer to an integer to be filled with the option's value
 *
 * @return   TRUE if the option was found and it's value placed in the
 *           value variable, FALSE otherwise.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmGetArg, tsmGetArgInt, tsmGetArgString
 *
 */

int
tsmGetArgFloat( int argc, char **argv, char *arg, double *value )
{
  static const char *funcName = "tsmGetArgFloat";
  char              *string;

  if ( tsmGetArgPointer( argc, argv, arg, &string ) == FALSE )
    return FALSE;

  if ( string == NULL ) {
    tsmError( funcName, "no float value supplied for %s option", arg );
    return FALSE;
  }

  *value = atof(string);
  return TRUE;
}

/*
 * Synopsis:
 *    int tsmParsedAllOptions( void )
 *
 * Doc++ Description: *//** 
 *    Checks if all options were parsed.
 *
 *    Once you have used the various command line parsing routines
 *    of tsmApi to check for any options, you can use this function
 *    to find out if every option was parsed. The function returns
 *    FALSE if there were any options on the command line that were
 *    not parsed by tsmApi. N.B. an option is taken as any string on
 *    the command line that begins with a '-' character.
 *
 *    You can optionally have tsmApi provide an error report for
 *    you detailing the specific options that were not parsed.
 *
 * @param    silent FALSE if you want a report on stderr, TRUE if no report
 *
 * @return   TRUE if all options were parsed, FALSE if some options were not
 *           parsed
 *
 * @author   Martin Reddy
 * @version  3 June 1998
 *
 * @see      tsmGetArg, tsmGetArgInt, tsmGetArgString, tsmGetArgFloat
 *
 */

int
tsmParsedAllOptions( int silent )
{
  int i, result = TRUE, first = TRUE;

  if ( tsm_argb == NULL ) return TRUE;

  for ( i = 1; i < tsm_argc; ++i ) {
    if ( tsm_argb[i] == FALSE && tsm_argv[i][0] == '-' ) {
      if ( silent == FALSE ) {
	if ( first == TRUE ) {
	  fprintf( stderr, "WARNING - unsupported option(s):" );
	  first = FALSE;
	}
	fprintf( stderr, " %s", tsm_argv[i] );
      }
      result = FALSE;
    }
  }

  if ( silent == FALSE && first == FALSE ) fprintf( stderr, "\n" );
  tsmFreeArgs();
  return result;
}

/*
 * Synopsis:
 *   int tsmParseInitArgs( int argc, char **argv )
 *
 * Doc++ Description: *//** 
 *   Implement any standard tsmApi command line options.
 *
 *   This function can be used to allow your programs to support 
 *   any standard command line arguments for setting initial
 *   tsmApi state. If used, it should be the first tsmApi call that 
 *   you make.
 *
 *   Currently the supported options are:
 *   \begin{itemize}
 *     \item "-debug <level>": If found, the value after the -debug
 *     option will be used to set the tsmApi debug trace level. This
 *     should be a value in the range 0-5 (a warning will be output if
 *     an invalid value is given and the value will be clamped).
 *     \item "-version": will display the tsmApi release date and
 *     version number to stdout. Also, if DPSS support is compiled in,
 *     then the DPSS library version information is displayed too.
 *   \end{itemize}
 *
 *   N.B. The string comparison for option matching is case sensitive.
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmParseReadArgs, tsmParseWriteArgs, tsmParseConnectArgs,
 *           tsmDescribeArgs, tsmGetArg
 * */

void
tsmParseInitArgs( int argc, char **argv )
{
  static const char *funcName = "tsmParseInitArgs";
  int               debug_level;

  /* check for the -debug <level> command line option */

  if ( tsmGetArgInt( argc, argv, "-debug", &debug_level ) == TRUE ) {
    if ( debug_level < 0 || debug_level > 5 ) {
      debug_level = max( min( debug_level, 5 ), 0 );
      tsmError( funcName, "Invalid debug level. Clamping to %d", debug_level );
    }

    pthr_trace_stream (stderr);
    pthr_trace_html (FALSE);
    pthr_trace_print_pid (TRUE);
    pthr_trace_mask (TSMMASK);
    tsmSetTraceLevel( debug_level );
  }

  /* Check for the -version command line options */

  if ( tsmGetArg( argc, argv, "-version" ) == TRUE ) 
      tsmPrintReleaseInfo( stdout );

}

/*
 * Synopsis:
 *   TsmConnectionParams tsmParseReadArgs( int argc, char **argv );
 *
 * Doc++ Description: *//** 
 *   Create a set of read connection hints from command line.
 *
 *   This function can be used to allow your programs to support 
 *   a number of standard command line arguments for specifying
 *   preferences for opening a read connection.
 *
 *   The function will scan the provided command line looking for any
 *   supported arguments.  It will build a TsmConnectionParams
 *   structure based upon the options that it finds and return this to
 *   you the client. This TsmConnectionParams structure can then be
 *   passed directly to tsmConnect when opening a connection.
 *
 *   You should free the memory used by the TsmConnectionParams structure
 *   when you are finished with it by calling tsmFreeParams().
 *   
 *   For example, if this function finds an option called "-readjpeg"
 *   on the command line, it will insert an entry into the TsmConnectionParams
 *   structure to ask for a JPEG dataset. So when you use tsmConnect with
 *   this set of hints, it will try to open a JPEG version of the dataset.
 *
 *   The currently supported options that this function recognises are:
 *
 *   \begin{itemize}
 *   \item "-readlrbt" = try to read a raw lrbt-raster version of the dataset
 *   \item "-readjpeg" = try to read a jpeg-compressed version of the dataset
 *   \item "-readtiffs" = try to read local tile data in TIFF format
 *   \item "-readrawtiles" = try to read local tile data in old .tile format
 *   \item "-readsetid" = try to read the DPSS set with this Set ID.
 *   \item "-readexact" = if cannot find exact dataset, then fail. Otherwise
 *                        tsmApi will try to find another version of dataset.
 *   \item "-dontvalidatetspec" = by default we check the fields of a tspec
 *                        file when loaded to see if valid. This option will 
 *                        disable those tests.
 *   \end{itemize}
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 *
 * @return   An allocated TsmConnectionParams structure, or NULL if no
 *           supported options were found on the command line.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmParseWriteArgs, tsmParseConnectArgs, tsmParseInitArgs,
 *           tsmDescribeArgs, tsmGetArg, tsmFreeParams
 *
 */

TsmConnectionParams
tsmParseReadArgs( int argc, char **argv )
{
  static const char   *funcName = "tsmParseReadArgs";
  TsmConnectionParams params;
  int                 set_id, n;
  char                server_opt[32], *server_name;
  int                 read_multi;

  /* initialise the param structure and check for supported arguments */

  tsmInitParams( &params );
  
  if ( tsmGetArg( argc, argv, "-readlrbt" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LRBT_FORMAT );

  if ( tsmGetArg( argc, argv, "-readjpeg" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_JPEG_FORMAT );

  if ( tsmGetArg( argc, argv, "-readlzw" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LZW_FORMAT );

  if ( tsmGetArg( argc, argv, "-readtiffs" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TIFF, TRUE );

  if ( tsmGetArg( argc, argv, "-readrawtiles" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TIFF, FALSE );

  if ( tsmGetArgInt( argc, argv, "-readsetid", &set_id ) == TRUE )
    tsmSetParam( params, TSM_PARAM_SETID, set_id );

  if ( tsmGetArg( argc, argv, "-dontvalidatetspec" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_VALIDATE_TSPEC, FALSE );

  if ( tsmGetArg( argc, argv, "-readexact" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FALLBACK, FALSE );

  if ( tsmGetArg( argc, argv, "-readmetadata" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_DPSS_TSPEC, FALSE );

  if ( tsmGetArg( argc, argv, "-readoldtspec" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_NEW_TSPEC, FALSE );

  if ( tsmGetArg( argc, argv, "-readnewtspec" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_NEW_TSPEC, TRUE );

  if ( tsmGetArgInt( argc, argv, "-read_multi", &read_multi ) == TRUE )
    tsmSetParam( params, TSM_PARAM_READ_MULTI, read_multi );

  /* check for any number of -readserver<n> options */

  if ( tsmGetArg( argc, argv, "-readserver0" ) == TRUE ) 
    tsmError( funcName, "-readserver0 invalid, start at -readserver1" );

  for ( n = 1; ; ++n ) {
    sprintf( server_opt, "-readserver%d", n );
    if ( tsmGetArgString( argc, argv, server_opt, &server_name ) == FALSE )
      break;
    tsmSetParam( params, TSM_PARAM_SERVERS, server_name );
  }

  /* If no read connection params were found, then just return NULL */

  if ( params->flag == 0 ) {
    tsmFreeParams( params );
    return NULL;
  }

  return params;
}

/*
 * Synopsis:
 *   TsmConnectionParams tsmParseWriteArgs( int argc, char **argv );
 *
 * Doc++ Description: *//** 
 *   Create a set of write connection hints from command line.
 *
 *   This function can be used to allow your programs to support 
 *   a number of standard command line arguments for specifying
 *   preferences for opening a write connection.
 *
 *   The function will scan the provided command line looking for any
 *   supported arguments.  It will build a TsmConnectionParams
 *   structure based upon the options that it finds and return this to
 *   you the client. This TsmConnectionParams structure can then be
 *   passed directly to tsmConnect when opening a connection.
 *
 *   You should free the memory used by the TsmConnectionParams structure
 *   when you are finished with it by calling tsmFreeParams().
 *   
 *   For example, if this function finds an option called "-writejpeg"
 *   on the command line, it will insert an entry into the
 *   TsmConnectionParams structure to say that a JPEG dataset should
 *   be generated. So when you use tsmConnect with this set of hints,
 *   it will try to create a JPEG version of the dataset.
 *
 *   The currently supported options that this function recognises are:
 *
 *   \begin{itemize}
 *   \item "-writelrbt" = write a raw lrbt-raster version of the dataset
 *   \item "-writejpeg" = write a jpeg-compressed version of the dataset
 *   \item "-writetiffs" = write local tile data in TIFF format 
 *   \item "-writerawtiles" = write local tile data in old .tile format
 *   \item "-writeexact" = fail if an illegal dataset combination is
 *                         specified (e.g. JPEG DEMs), otherwise a legal
 *                         alternative will be selected (e.g. LRBT DEMs).
 *   \end{itemize}
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 *
 * @return   An allocated TsmConnectionParams structure, or NULL if no
 *           supported options were found on the command line.
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmParseReadArgs, tsmParseConnectArgs, tsmParseInitArgs,
 *           tsmDescribeArgs, tsmGetArg, tsmFreeParams
 *
 */

TsmConnectionParams
tsmParseWriteArgs( int argc, char **argv )
{
  static const char   *funcName = "tsmParseWriteArgs";
  TsmConnectionParams params;
  int                 n;
  char                server_opt[32], *server_name;

  /* initialise the param structure and check for supported arguments */

  tsmInitParams( &params );
  
  if ( tsmGetArg( argc, argv, "-writelrbt" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LRBT_FORMAT );

  if ( tsmGetArg( argc, argv, "-writejpeg" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_JPEG_FORMAT );

  if ( tsmGetArg( argc, argv, "-writelzw" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FORMAT, TSM_LZW_FORMAT );

  if ( tsmGetArg( argc, argv, "-writetiffs" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TIFF, TRUE );

  if ( tsmGetArg( argc, argv, "-writerawtiles" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TIFF, FALSE );

  if ( tsmGetArg( argc, argv, "-writeexact" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_FALLBACK, FALSE );

  /* check for any number of -writeserver<n> options */

  if ( tsmGetArg( argc, argv, "-writeserver0" ) == TRUE ) 
    tsmError( funcName, "-writeserver0 invalid, start at -writeserver1" );

  for ( n = 1; ; ++n ) {
    sprintf( server_opt, "-writeserver%d", n );
    if ( tsmGetArgString( argc, argv, server_opt, &server_name ) == FALSE )
      break;
    tsmSetParam( params, TSM_PARAM_SERVERS, server_name );
  }

  /* If no read connection params were found, then just return NULL */

  if ( params->flag == 0 ) {
    tsmFreeParams( params );
    return NULL;
  }

  return params;
}


/*
 * Synopsis:
 *   TsmConnectionParams tsmParseVRMLArgs( int argc, char **argv );
 *
 * Doc++ Description: *//** 
 *   Create a set of VRML writing connection hints from command line.
 *
 *   This function can be used to allow your programs to support 
 *   a number of standard command line arguments for specifying
 *   preferences for writing TerraVRML file.
 *
 *   The function will scan the provided command line looking for any
 *   supported arguments.  It will build a TsmConnectionParams
 *   structure based upon the options that it finds and return this to
 *   you the client. This TsmConnectionParams structure can then be
 *   passed directly to tsmConnect when opening a connection.
 *
 *   You should free the memory used by the TsmConnectionParams structure
 *   when you are finished with it by calling tsmFreeParams().
 *   
 *   For example, if this function finds an option called "-gaussian"
 *   on the command line, it will insert an entry into the
 *   TsmConnectionParams structure to say that TerraVRML files should
 *   be written out with gaussian interpolation of elevation. 
 *
 *   The currently supported options that this function recognises are:
 *
 *   \begin{itemize}
 *   \item "-vrmldir <dir>" = directory where this dataset will be written to
 *   \item "-numpolys <n>" = polygons per side on a vtile
 *   \item "-lods <n>" = number of lods in one tree file
 *   \item "-treeskip <n>" = levels to skip between tree files
 *   \item "-gaussian" = use gaussian elevation interpolation instead of linear
 *   \item "-collision" = turn on collision detection for VRML geometry
 *   \item "-testvtiles" = create flat, randomly-colored vtiles
 *   \item "-start <n>" = TV start pyramid level (def=lowest-res)
 *   \item "-end <n>" = TV end pyramid level (def=highest-res)
 *   \item "-pixpertex <ratio>" = scale for LOD range calc (def=1.0)
 *   \item "-elevscale" = sets the elevation scaling relative to lat/long 
 *		     dimensions (default = 1.0)
 *   \item "-nonormals" = do not generate normals for each vtile file
 *   \item "-normals" = generate normals for each vtile file
 *   \item "-absurl" or "-vrmlurl" = set the absolute root URL 
 *   \end{itemize}
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 *
 * @return   An allocated TsmConnectionParams structure, or NULL if no
 *           supported options were found on the command line.
 *
 * @author   Nat Bletter
 * @version  29 April 1998
 *
 * @see      tsmParseWriteArgs, tsmParseReadArgs, tsmParseConnectArgs,
 *           tsmParseInitArgs, tsmDescribeArgs, tsmGetArg, tsmFreeParams
 *
 */

TsmConnectionParams
tsmParseVRMLArgs( int argc, char **argv )
{
  /*static const char funcName[] = "tsmParseVRMLArgs";*/
  TsmConnectionParams params;
  int                 n;
  char                *buffer;
  double              lcs;

  /* initialise the param structure and check for supported arguments */

  tsmInitParams( &params );
  
  if ( tsmGetArgString( argc, argv, "-vrmldir", &buffer ) == TRUE )
    tsmSetParam( params, TSM_PARAM_VRML_DIR, buffer );

  if ( tsmGetArgInt( argc, argv, "-numpolys", &n ) == TRUE )
    tsmSetParam( params, TSM_PARAM_NUM_POLYS, n );

  if ( tsmGetArg( argc, argv, "-collision" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_COLLISION, TRUE );

  if ( tsmGetArg( argc, argv, "-testvtiles" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TEST_VTILES, TRUE );

  if ( tsmGetArgInt( argc, argv, "-start", &n ) == TRUE )
    tsmSetParam( params, TSM_PARAM_START_LEVEL, n );

  if ( tsmGetArgInt( argc, argv, "-end", &n ) == TRUE )
    tsmSetParam( params, TSM_PARAM_END_LEVEL, n );

  if ( tsmGetArgFloat( argc, argv, "-pixpertex", &lcs ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TEXEL_SCALE, lcs );

  if ( tsmGetArgFloat( argc, argv, "-elevscale", &lcs ) == TRUE )
    tsmSetParam( params, TSM_PARAM_ELEV_SCALE, lcs);

  if ( tsmGetArg( argc, argv, "-nonormals" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_NORMALS, FALSE );

  if ( tsmGetArg( argc, argv, "-normals" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_NORMALS, TRUE );

  if ( tsmGetArg( argc, argv, "-nomodels" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_MODELS, FALSE );

  if ( tsmGetArg( argc, argv, "-onesided" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_SOLID, TRUE );

  if ( tsmGetArg( argc, argv, "-notextures" ) == TRUE )
    tsmSetParam( params, TSM_PARAM_TEXTURES, FALSE );

  if ( tsmGetArgString( argc, argv, "-absurl", &buffer ) == TRUE ||
       tsmGetArgString( argc, argv, "-vrmlurl", &buffer ) == TRUE )
    tsmSetParam( params, TSM_PARAM_VRML_URL, buffer );

  if ( tsmGetArgInt( argc, argv, "-rootlevel", &n ) == TRUE )
    tsmSetParam( params, TSM_PARAM_ROOT_LEVEL, n );

  /* If no read connection params were found, then just return NULL */

  if ( params->flag == 0 ) {
    tsmFreeParams( params );
    return NULL;
  }

  return params;
}


/*
 * Synopsis:
 *   void tsmParseConnectArgs( int argc, char **argv, TsmConnection con );
 *
 * Doc++ Description: *//** 
 *   Updates a connection's settings from command line options.
 *
 *   This function can be used to allow your programs to support 
 *   a number of standard command line arguments for specifying
 *   the settings of a connection once it has been opened.
 *
 *   The function will scan the provided command line looking for any
 *   supported arguments.  It will build update the passed TsmConnection
 *   structure based upon the options that it finds, effectively by doing
 *   a number of tsmSet() calls based upon the available parameters.
 *   
 *   For example, if this function finds an option called "-jpegquality 50"
 *   on the command line, it will do a tsmSet(connect, TSM_JPEG_QUALITY, 50)
 *   for the passed connection, thus setting the output JPEG quality for
 *   any tiles.
 *
 *   The currently supported options that this function recognises are:
 *
 *   \begin{itemize}
 *   \item "-dontreadtiles = don't do any disk or network access during
 *         any of the read tile routines [READ ONLY]
 *   \item "-jpegquality <n>" = the jpeg quality to write tiles with (0-100%)
 *         [WRITE ONLY]
 *   \item "-metadata<n>" = sets one of n metadata URLs for a new DPSS set
 *         [WRITE ONLY]
 *   \end{itemize}
 *
 * @param    argc the number of arguments passed to main()
 * @param    argv the array of command line strings passed to main()
 *
 * @author   Martin Reddy
 * @version  13 March 1998
 *
 * @see      tsmParseReadArgs, tsmParseWriteArgs, tsmParseInitArgs,
 *           tsmDescribeArgs, tsmGetArg
 *
 */

void
tsmParseConnectArgs( int argc, char **argv, TsmConnection connect )
{
  static const char   *funcName = "tsmParseConnectArgs";
  int                 n;
  char                metadata_opt[32], *metadata;

  /* Check for any options for a read connection if that is what we are */

  if ( connect->mode == TSM_READ_ONLY || connect->mode == TSM_READ_WRITE ) {

    if ( tsmGetArg( argc, argv, "-dontreadtiles" ) == TRUE )
      tsmSet( connect, TSM_DONT_READ_TILES, TRUE );

  }
  
  /* Check for any options for a write connection if that is what we are */

  if ( connect->mode == TSM_WRITE_ONLY || connect->mode == TSM_READ_WRITE ) {

    if ( tsmGetArgInt( argc, argv, "-jpegquality", &n ) == TRUE )
      tsmSet( connect, TSM_JPEG_QUALITY, n );

    /* check for any number of -metadata<n> options */

    if ( tsmGetArg( argc, argv, "-metadata0" ) == TRUE ) 
      tsmError( funcName, "-metadata0 invalid, start at -metadata1" );

    for ( n = 1; ; ++n ) {
      sprintf( metadata_opt, "-metadata%d", n );
      if ( tsmGetArgString( argc, argv, metadata_opt, &metadata ) == FALSE )
	break;
      tsmSet( connect, TSM_METADATA_URL, metadata );
    }

  }
}

/*
 * Synopsis:
 *    void tsmDescribeArgs( int which_args )
 *
 * Doc++ Description: *//** 
 *    Output descriptions of selected tsmApi arguments.
 *
 *    Through the use of functons such as tsmParseInitArgs, you can let
 *    the tsmApi parse your application's command line arguments for
 *    you and perform certain actions on these. In the future, more
 *    tsmApi arguments may be supported so we need an extensible way
 *    to describe all of the functions that an application currently
 *    supports, for example when displaying the program usage to the
 *    terminal.
 *
 *    This function will print out all of the arguments (with a 
 *    description of their usage) that tsmApi currently supports in
 *    it's various automatic parsing routines. Output is directed
 *    to stdout.
 *
 *    The parameter that you pass to this function details which of
 *    tsmApi's parse functions you have used, and hence which sets of
 *    arguments your application supports, e.g. this can be any
 *    combination of the following constants:
 *
 *    {\bf TSM_ARG_INIT}: if you used the tsmParseInitArgs function to 
 *    parse any general state tsmApi arguments.
 *
 *    {\bf TSM_ARG_READ}: if you used the tsmParseReadArgs function to 
 *    parse any read connection preferences.
 *
 *    {\bf TSM_ARG_WRITE}: if you used the tsmParseWriteArgs function to 
 *    parse any write connection preferences.
 *
 *    {\bf TSM_ARG_VRML}: if you used the tsmParseVRMLArgs function to 
 *    parse any write connection preferences.
 *
 *    {\bf TSM_ARG_CONNECT}: if you used the tsmParseConnectArgs function to 
 *    parse any write connection preferences. Note that this will output
 *    options for both a read or a write connection. To try and be a bit more
 *    intelligent, the read connection settings will be output if the 
 *    TSM_ARG_READ value is being passed in also (or if neither the 
 *    TSM_ARG_READ or TSM_ARG_WRITE values are being passed in). And 
 *    similarly for the write connection settings.
 *
 * @param    which_args any combination (bitwise-or) of TSM_ARG_INIT,
 *                      TSM_ARG_READ, TSM_ARG_WRITE, TSM_ARG_VRML, 
 *                      and TSM_ARG_CONNECT.
 *
 * @return   Nothing (void)
 *
 * @author   Martin Reddy
 * @version  10 March 1998
 *
 * @see      tsmParseInitArgs, tsmParseReadArgs, tsmParseWriteArgs,
 *           tsmParseConnectArgs, tsmParseVRMLArgs
 *
 */

void
tsmDescribeArgs( int which_args )
{
  int arg_init, arg_read, arg_write, arg_connect, arg_vrml;

  arg_init    = ( which_args & TSM_ARG_INIT ) != 0;
  arg_read    = ( which_args & TSM_ARG_READ ) != 0;
  arg_write   = ( which_args & TSM_ARG_WRITE ) != 0;
  arg_connect = ( which_args & TSM_ARG_CONNECT ) != 0;
  arg_vrml    = ( which_args & TSM_ARG_VRML ) != 0;

  if ( arg_read ) {
    puts( " -readlrbt = read a raw lrbt-raster version of dataset (default)" );
    puts( " -readjpeg = read a jpeg-compressed version of the dataset" );
    puts( " -readlzw = read an LZW compressed version of the dataset" );
    puts( " -readtiffs = read local tile data in TIFF format (default)" );
    puts( " -readrawtiles = read local tile data in old .tile format" );
    puts( " -readsetid <id> = read the DPSS set with this Set ID" );
    puts( " -readserver<n> = one of n servers for the DPSS set to read, n>=1");
    puts( " -readexact = if cannot find exact dataset, then fail" );
    puts( " -readmetadata = for DPSS sets, try metadata URLs for tspec" );
    puts( " -readnewtspec = for file/http sets, read only new style tspec" );
    puts( " -readoldtspec = for file/http sets, read only old style tspecs" );
    puts( " -dontvalidatetspec = don't do validity checks on tspec files" );
  }

  if ( arg_connect && ( arg_read || ( (!arg_read) && (!arg_write) ) ) ) {
    puts( " -dontreadtiles = don't do any disk/network access");
  }

  if ( arg_write ) {
    puts( " -writelrbt = write a raw lrbt-raster version of dataset (default)" );
    puts( " -writejpeg = write a jpeg-compressed version of the dataset" );
    puts( " -writelzw = write an lzw compressed version of dataset (lrbt)" );
    puts( " -writetiffs = write local tile data in TIFF format (default)" );
    puts( " -writerawtiles = write local tile data in old .tile format" );
    puts( " -writeserver<n> = one of n servers for creating a DPSS set, n>=1");
    puts( " -writeexact = fail if illegal dataset combo, e.g. JPEG DEMs" );
  }

  if ( arg_connect && ( arg_write || ( (!arg_read) && (!arg_write) ) ) ) {
    puts( " -jpegquality <n> = set output jpeg quality, n=5..100 (default=75)" );
    puts( " -metadata<n> = set one of n tspec URLs when writing a DPSS dataset, n>=1");
  }

  if ( arg_init ) {
    puts( " -debug <n> = set the tsmApi debug level, n=0..5 (default=0)" );
    puts( " -version = display tsmApi release data and version number" );
  }

  if (arg_vrml) {
    puts(" -vrmldir <dir> = directory where this dataset will be written to");
    puts(" -numpolys <n> = polygons per side in a tile");
    puts(" -collision = turn on collision detection for VRML geometry");
    puts(" -start <n> = TV start pyramid level (def=lowest-res)");
    puts(" -end <n> = TV end pyramid level (def=highest-res)");
    puts(" -pixpertex <ratio> = scale for LOD range calc (def=1.0)");
    puts(" -elevscale = sets the elevation scaling relative to lat/long dimensions (default 1.0)");
    puts(" -nonormals = doesn't put normals in tile files");
    puts(" -normals = put normals in tile files");
    puts(" -nomodels = doesn't generate VRML files");
    puts(" -notextures = doesn't generate texture image files");
    puts(" -onesided = generate one-sided polygons for the terrain");
    puts(" -vrmlurl = sets the root URL in all VRML files (can also use -absurl)");
  }    

}
