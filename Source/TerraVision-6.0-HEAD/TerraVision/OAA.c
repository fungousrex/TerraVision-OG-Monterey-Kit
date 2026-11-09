/* -*- tab-width: 2; -*-

  Name: OAA.c

  Description:
      Contains various routines that control TerraVision's interaction
      with SRI's OAA (Open Agent Architecture). This includes the features
      such as drawing on the screen and API hooks for the OAA to use.

  Revision History:
      January 19, 1999. Martin Reddy <reddy@ai.sri.com>
      $Id: OAA.c,v 2.63.2.14 2002/10/31 05:26:31 reddy Exp $

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

#include "TerraVisionInc.h"

#include "OAA.h"
#include "ThreeDCoreI.h"

#ifdef HAVE_X11
#define XWINDOWS
#endif

#include "agentlib.h"

#include <prsystem.h>
#include <prnetdb.h>

/* Variables used to control interaction with the OAA */

static char *oaa_host = NULL;          /* hostname for server */
static int oaa_port = -1;              /* port number for server */
static char *oaa_name = "TerraVision"; /* name for the OAA client */
static char *solvable = NULL;          /* list of solvables for facilitator */
static int is_oaa = FALSE;             /* are we connected to a facilitator? */
static PRThread *oaa_thread;           /* the OAA event loop thread */
static char *oaa_id = NULL;            /* TerraVision's base ID name */
static char oaa_connected[128];        /* host:port of connected OAA */

/* The set of macro definitions local to this module */

#define HIGH_WATER_MARK   32

/* The following variables are local to this module, the contain  */
/* various bits of state such as whether the mouse button is down */

static int time_out   = 1000;          /* ms timeout for end of text */
static int fullRefresh = 0;            /* Does RenderSentence refresh all? */

/* Now we define the prototypes for all the internal module functions */

static void SendSentence( tvBase *threeD );
static void AddPenCoord( tvBase *threeD, int x, int y );
static StrokeType *NewStroke( tvBase *threeD );
static void ProjectSentence( tvBase *threeD );
static void ClearDrawTimeOut( tvBase *threeD );
static void StartDrawTimeOut( tvBase *threeD );
static void AddSolvable( char *new_solvable );
static void tvAddLabel(tvBase *threeD, char *label, double lat, double lon, double elev);
static void ExtractNthArg( char const *arglist, int argno, char argument[] );
static int CountArgs( char const *arglist );
static void OAATimer(tvBase *threeD);
static void OAALoop(void *ignore);
static int OAAEvent( char *KS, char *func, char *args, char **answers );

/*********************** OAA INITIALISATION ROUTINES **********************/

/*
 * Synopsis:
 *   void AddSolvable( char *new_solvable )
 *
 * Description:
 *   This function will let you build up the string of solvables that
 *   you eventually pass to the facilitator. A solvable is basically
 *   the definition of some function that the facilitator can access,
 *   along with its number of parameters. This is specified in a prolog
 *   syntax such as "[func1(_,_),func2, func3(_)]". The new_solvable
 *   parameter would therefore be something like "func1(_,_)".
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 21 January 1999
 *
 */

static void
AddSolvable( char *new_solvable )
{
  int new_length;

  if ( new_solvable == NULL ) return;
  new_length = strlen( new_solvable );

  /* Is this the first solvable being added to the list? */

  if ( solvable == NULL ) {
    solvable = (char *) malloc( new_length + 3 );
    sprintf( solvable, "[%s]", new_solvable );
    return;
  }

  /* do we already have this solvable? */

  if ( strstr( solvable, new_solvable ) != NULL )
    return;

  /* Otherwise, append this solvable to the current list */

  solvable = (char *) realloc( solvable, strlen( solvable ) + new_length + 10 );

  solvable[strlen(solvable)-1] = ',';
  strcat( solvable, new_solvable );
  strcat( solvable, "]" );
}

/*
 * Synopsis:
 *   void AddTVSolvables( int add_solvables )
 *
 * Doc++ Description: *//**
 *   Lets you control whether the OAAConnect call will automatically
 *   add the terrain3d() solvable that TerraVision supports. This is
 *   useful when you are you building command line OAA agents on
 *   this code base but don't want them to conflict with a running
 *   TerraVision client.
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  11 April 2001
 *
 * @see      OAAConnect
 *
 */

static int add_terrain3d_solvable = TRUE;

void
AddTVSolvables( int add_solvables )
{
  add_terrain3d_solvable = add_solvables;
}

/*
 * Synopsis:
 *   int OAAConnect(tvBase *threeD)
 *
 * Doc++ Description: *//**
 *   If you have set the hostname and port number of a facilitator
 *   (using SetOAAHost and SetOAAPort), then this function will try
 *   to establish a connection to that facilitator. If you have not
 *   specified both of these, then no action is taken. 
 *
 * @return   TRUE if successfully connected to the prescribed facilitator
 *
 * @author   Martin Reddy
 *
 * @version  21 January 1999
 *
 * @see      SetOAAHost, SetOAAPort
 *
 */

int
OAAConnect( tvBase *threeD )
{
  const char *funcName = "OAAConnect";

  if ( oaa_host == NULL || oaa_port < 0 ) return FALSE;

  if ( is_oaa ) return TRUE;

  /* Define the list of solvables for the facilitator to know about */
  /* We just use one as the entrance point to all TV functionality */

  if ( add_terrain3d_solvable )
    AddSolvable( "terrain3d(Function,Params)" );

  TSMTRACE(( OAA_MASK, funcName, "Connecting to OAA facilitator..." ));
  TSMTRACE(( OAA_MASK, funcName, "Solvable = %s", solvable ));

  /* register the appropriate OAA callbacks for this library */

  oaa_register( OAA_DO_EVENT, OAAEvent );

  /* Set up the initial communication with the facilitator */

  is_oaa = setup_communication( oaa_name, oaa_host, oaa_port, solvable, 1 );
  if ( ! is_oaa ) {
    tsmError( funcName, "Couldn't connect to OAA facilitator on %s:%d",
	      oaa_host, oaa_port );
    return FALSE;
  }

  TSMTRACE(( OAA_MASK, funcName, "Completed communication with facilitator" ));

  /* let the server know that the agent is ready to begin... */

  oaa_ready( FALSE );

  TSMTRACE(( OAA_MASK, funcName, "Returned from oaa_ready" ));

  /* tsmStatus( "Connected to facilitator on %s:%d", oaa_host, oaa_port ); */

  if (pthr_create (&oaa_thread, "OAA", OAALoop, NULL)) {
    tsmError (funcName, "Unable to spawn the OAA thread");
    return FALSE;
  }

  TSMTRACE(( OAA_MASK, funcName, "Created OAA thread. Leaving function." ));

  sprintf( oaa_connected, "%s:%d", oaa_host, oaa_port );

  return TRUE;
}

/*
 * Synopsis:
 *   void OAADisconnect(tvBase *threeD)
 *
 * Doc++ Description: *//**
 *   Disconnects from the current OAA facilitator, as connected to
 *   using the OAAConnect() call. If not connected, this function 
 *   will do nothing.
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  20 March 2001
 *
 * @see      OAAConnect
 *
 */
void
OAADisconnect( tvBase *threeD )
{
  const char *funcName = "OAADisconnect";
  if ( ! is_oaa ) return;
  disconnect();
  is_oaa = FALSE;
  TSMTRACE(( OAA_MASK, funcName, "Disconnected from OAA facilitator" ));
}

/*
 * Synopsis:
 *   char *GetOAAConnected( void )
 *
 * Doc++ Description: *//**
 *   Returns a string describing the host and port number of the 
 *   OAA facilitator that we are currently connected to. A value
 *   of NULL implies that we are not currently connected to any 
 *   facilitator. You should not free this string.
 *
 * @return   A "host:port" string, or NULL if not connected
 *
 * @author   Martin Reddy
 *
 * @version  20 March 2001
 *
 * @see GetOAAHost, SetOAAPort, SetOAAHostPort, OAAConnect
 *
 */
char *
GetOAAConnected( void )
{
  if ( ! is_oaa ) return NULL;
  return oaa_connected;
}

/*
 * Synopsis:
 *   void SetOAAHost( char const *hostname )
 *
 * Doc++ Description: *//**
 *   Used to set the hostname for an OAA facilitator. This information is 
 *   used by OAAConnect to determine where to find the facilitator. 
 *   The hostname string is strdup'd inside the function.
 *
 * @param hostname  hostname for OAA facilitator.
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  21 January 1999
 *
 * @see GetOAAHost, SetOAAPort, SetOAAHostPort, OAAConnect
 *
 */
void
SetOAAHost( char const *hostname )
{
  if ( hostname != NULL )
    oaa_host = tsmMemStrdup( hostname );
}

/*
 * Synopsis:
 *   char *GetOAAHost( void )
 *
 * Doc++ Description: *//**
 *   Returns the current hostname for the OAA facilitator. This host is 
 *   used by OAAConnect to determine where to find the facilitator. 
 *   The returned string is strdup'd and should be freed by the caller
 *   when finished.
 *
 * @return   The OAA facilitator hostname, or NULL if none
 *
 * @author   Martin Reddy
 *
 * @version  20 March 2001
 *
 * @see SetOAAHost, SetOAAPort, SetOAAHostPort, OAAConnect
 *
 */
char *
GetOAAHost( void )
{
  return ( oaa_host ) ? tsmMemStrdup( oaa_host ) : NULL;
}

/*
 * Synopsis:
 *   void SetOAAPort( int port )
 *
 * Doc++ Description: *//**
 *   Used to set the port number for an OAA facilitator. This information is 
 *   used by OAAConnect to determine where to find the facilitator. 
 *
 * @param port   port number for the OAA facilitator.
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 21 January 1999
 *
 * @see GetOAAPort, SetOAAHost, SetOAAHostPort, OAAConnect
 *
 */
void
SetOAAPort( int port )
{
  oaa_port = port;
}

/*
 * Synopsis:
 *   void GetOAAPort( int port )
 *
 * Doc++ Description: *//**
 *   Returns the current port number for an OAA facilitator. This port is
 *   used by OAAConnect to determine where to find the facilitator. 
 *
 * Returns: the port number, or -1 if none set
 *
 * Author: Martin Reddy
 *
 * Date: 20 March 2001
 *
 * @see SetOAAPort, SetOAAHost, SetOAAHostPort, OAAConnect
 *
 */
int
GetOAAPort( void )
{
  return oaa_port;
}

/*
 * Synopsis:
 *   void SetOAAHostPort( char const *hostport )
 *
 * Doc++ Description: *//**
 *   Used to set the hostname and port of an OAA facilitator, from a string
 *   that is in the format host:port.  This information is used by OAAConnect
 *   to determine where to find the facilitator. 
 *
 * @param    hostport   string for the hostname and port number
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  21 January 1999
 *
 * @see SetOAAHost, SetOAAPort, OAAConnect
 *
 */
void
SetOAAHostPort( char const *hostport )
{
  char *sep;

  if ( hostport == NULL ) return;
  oaa_host = tsmMemStrdup( hostport );
  if ( ( sep = strchr( oaa_host, ':' ) ) == NULL ) return;
  *sep = '\0';
  oaa_port = atoi( ++sep );
  if ( oaa_port == 0 ) oaa_port = -1;
}

/*
 * Synopsis:
 *   char *GetNextOAAID( void )
 *
 * Description:
 *   Returns a default ID string to be used when creating a new
 *   tvBase instance. This will use the value set by SetOAADefaultID
 *   or an auto-generated ID string, appended with the viewer number
 *   (no viewer number is appended for viewer one). The first time
 *   the function is called an ID for viewer 1 is created, then next
 *   time an ID for viewer 2 is created.

 *   This function is not part of the public API. To query or change
 *   a tvBase's ID string, use tvGetString() and tvSetString(). This
 *   ID string can be used to send OAA commands to only certain
 *   instances and viewers.
 *
 * @return   the name of this viewer's default OAA ID string.
 *
 * @author   Martin Reddy
 *
 * @version  17 March 2001
 *
 * @see SetOAADefaultID, tvSetString, tvGetString
 *
 */

char *
GetNextOAAID( void )
{
  static int viewer = 0; 
  char this_id[128];
  
  viewer++;

  if ( ! oaa_id ) {
    char hname[64], base_id[128];
    int  pid = getpid();
    PR_GetSystemInfo( PR_SI_HOSTNAME, hname, 64 );
    sprintf( base_id, "%s-%d", hname, pid % 10000 );
    oaa_id = strdup( base_id );
  }

  if ( viewer == 1 )
    strcpy( this_id, oaa_id );
  else 
    sprintf( this_id, "%s-%d", oaa_id, viewer );

  return strdup( this_id );
}

/*
 * Synopsis:
 *   void SetOAADefaultID( char *id )
 *
 * Doc++ Description: *//**
 *   Sets the base ID for this instance of TerraVision. This ID
 *   is used to create the default ID string for each viewer of
 *   this instance. Once created, this default ID string for a
 *   tvBase can be queried or changed using the tvGetString()
 *   and tvSetString() functions. A tvBase's ID can be used to
 *   send OAA commands to a specific viewers.
 *
 * @param    id the new base ID string
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  17 March 2001
 *
 * @see tvSetString, tvGetString
 *
 */

void
SetOAADefaultID( char *id )
{
  if ( oaa_id ) free( oaa_id );
  oaa_id = ( id ) ? strdup( id ) : NULL;
}

/*
 * Synopsis:
 *   static int CountArgs( char *arglist )
 *
 * Description:
 *   Counts the number of comma-separated arguments that are specified
 *   in a string and returns this value. We can assume that if there
 *   is a comma, then some non-whitespace characters must have preceeded
 *   it - the OAA should enforce this for us, i.e. we should never get
 *   something like "1,,,,1".
 *
 * Returns: the number of comman-separated arguments
 *
 * Author: Martin Reddy
 *
 * Date: 25 January 1999
 *
 */

static int
CountArgs( char const *arglist )
{
  int args;

  if ( arglist == NULL || strlen(arglist) == 0  ) return 0;

  for ( args = 1; *arglist != '\0'; arglist++ )
    if ( *arglist == ',' ) args++;

  return args;
}

/*
 * Synopsis:
 *   void ExtractNthArg( char *arglist, int argno, char argument[] )
 *
 * Description:
 *   A little convenience function that will extract the nth argument
 *   from a list of arguments. The result is written into the already
 *   allocated memory block, argument. The first element in the list
 *   is numbered 1 (not 0).
 *
 *   N.B. currently, no checks are made to ensure that the argument
 *   string does not exceed the size of the string buffer.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 25 January 1999
 *
 */

static void
ExtractNthArg( char const *arglist, int argno, char argument[] )
{
  char *temp;

  if ( arglist == NULL || argno < 1 ) return;

  if ( strlen( arglist ) == 0 ) {
    strcpy( argument, "" );
    return;
  }

  NthElt( (char*) arglist, argno, &temp );
  remove_quotes( temp );
  strcpy( argument, temp );
  OAA_FREE( temp );
}

static double
ExtractNthFloat( char const *arglist, int argno )
{
  char buffer[1024];
  ExtractNthArg( arglist, argno, buffer );
  return atof( buffer );
}

static int
ExtractNthInt( char const *arglist, int argno )
{
  char buffer[1024];
  ExtractNthArg( arglist, argno, buffer );
  return atoi( buffer );
}

static int
ExtractNthBoolean( char const *arglist, int argno )
{
  char buffer[1024];
  ExtractNthArg( arglist, argno, buffer );
  return ! ( toupper(buffer[0]) == 'F' || buffer[0] == '0' );
}

/*
 * Synopsis:
 *   static int IsOption( char *arglist, char *func )
 *
 * Description:
 *   Checks for the presence of a function in a list of functions. 
 *   e.g. "smooth" in "[func1(x),smooth(x)]" will return TRUE.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 26 January 1999
 *
 */

static int
IsOption( char const *arglist, char *func )
{
  char *found;

  if ( arglist == NULL || func == NULL ) return FALSE;
  param_value( arglist, func, &found );
  if ( ! found ) return FALSE;
  free( found );
  return TRUE;
}

/*
 * Synopsis:
 *   char *OAASolve( char *expression, char *options, char *solveparams )
 *
 * Doc++ Description: *//**
 *   Sends a TerraVision command to the facilitator to be solved. 
 *   The expression should be something like "set('SHOW_WIREFRAME',1)".
 *   If specified, the options string will be used as the optional
 *   argument to the terrain3d() solvable. If we are not connected
 *   to an OAA facilitator, then no action is taken. The caller 
 *   should free the returned string if it is non-NULL.
 *
 *   For a list of possible values for the solveparams string, see
 *   http://www.ai.sri.com/~oaa/sri-private/doc/refmanual.html#oaa_Solve
 *
 * @param expression a valid terrain3d() function
 * @param options a valid terrain3d() optional argument, or NULL;
 *
 * Returns: The result of the command, or NULL if failed
 *
 * Author: Martin Reddy
 *
 * Date: 20 March 2001
 *
 * @see OAAConnect
 *
 */
char *
OAASolve( char *expression, char *options, char *solveparams )
{
  const char *funcName = "OAASolve";
  char *content = NULL, *ptr, *answer = NULL;

  if ( is_oaa == FALSE || expression == NULL ) return NULL;

  append( &content, "terrain3d(" );
  append( &content, expression );

  /* append empty/supplied optional argument if none specified */

  if ( ( ptr = strrchr( expression, ')' ) ) == NULL )
    ptr = expression;
  if ( strchr( ptr, ',' ) == NULL ) {
    if ( options == NULL )
      append( &content, ",[]" );
    else {
      append( &content, "," );
      if ( options[0] != '[' ) append( &content, "[" );
      append( &content, options );
      if ( options[strlen(options)-1] != ']' ) append( &content, "]" );
    }
  }

  append( &content, ")" );
  
  if ( ! solveparams || solveparams[0] == '\0' ) solveparams = "[]";
  TSMTRACE(( OAA_MASK, funcName, "solve(%s,%s)", content, solveparams ));
  
  solve( content, solveparams, &answer );

  TSMTRACE(( OAA_MASK, funcName, "result = %s", answer ? answer : "nil" ));
  
  free( content );

  return answer;
}

/************************** STANDARD OAA FUNCTIONS ************************/

/****************************************************************************
 * name:    OAAEvent
 * purpose: Allow agent to specify actions for events specific to the agent.
 * inputs:
 *   - char    *KS: KS sending the event to be executed
 *   - char  *func: functor of event to be executed
 *   - char  *args: arguments of event to be executed
 * outputs:
 *   answers: a list of answers.  Should be "[]" for failure, or a list
 *      containing the event with variables replaced with values.
 *      example:  a query "a(X)" could return "[a(1),a(2),a(3)]"
 * returns:
 *    True if OAAEvent know how to handle the event (it's answer will be
 *    taken into account), False otherwise.
 * solvables:
 *    terrain3d(refresh,[]) 
 *    terrain3d(stopmotion,[])
 *    terrain3d(resetview,[])
 *    terrain3d(message(Text),[])
 *    terrain3d(statusbar(Text),[])
 *    terrain3d(getview(X,Y,Z,Yaw,Pitch,Roll),[])
 *    terrain3d(setview(X,Y,Z,Yaw,Pitch,Roll),[])
 *    terrain3d(getview_gdc(Lat,Long,Elev),[])
 *    terrain3d(setview_gdc(Lat,Long,Elev,Yaw,Pitch,Roll),[])
 *    terrain3d(flyto(X,Y,Z,Yaw,Pitch,Roll),[])
 *    terrain3d(flyto_gdc(Lat,Long,Elev,Yaw,Pitch,Roll),[])
 *    terrain3d(DIS_ID(site,force,entity),[])
 *    terrain3d(translate(X,Y,Z),[])           : options = [noanim]  
 *    terrain3d(rotate(Yaw,Pitch,Roll),[])     : options = [noanim]
 *    terrain3d(move(Forward,Right,Up),[])     : options = [noanim]
 *    terrain3d(set(Option,Value),[])
 *    terrain3d(loadmodel(Url),[])
 *    terrain3d(unloadmodel(Url),[])
 *    terrain3d(loadterrain(Oiurl,Demurl),[])
 *    terrain3d(unloadterrain(Geoname),[])
 *    terrain3d(pathload(Url),[])
 *    terrain3d(pathreset,[])
 *    terrain3d(pathplay,[])
 *    terrain3d(pathstop,[])
 *    terrain3d(pathpause,[])
 *    terrain3d(getinfo(Id,Host,Os,Arch,Procs,Res),[])
 *    terrain3d(addlabel(String,lat,lon,elev),[])
 *    terrain3d(removelabel(String),[])
 *    terrain3d(getelev(Lat,Lon),[])
 *
 ****************************************************************************/

static void OAAFunc (tvBase *threeD, void *closure);

typedef struct {
  char const *name;
  char const *args;
  char const *opt_args;
  char **answers;
  int success;
} OAAClosure;

typedef struct {
  int         count;
  OAACallback cb[5];
  char        *func[5];
} OAACallbackList;

static OAACallbackList oaa_callback_list = { 0 };

void
OAAAddSolvable( char *solvable, OAACallback callback )
{
  char const *funcName = "OAAAddSolvable";
  char       *func, *ptr;

  /* can only add a certain number of OAA callbacks */

  if ( oaa_callback_list.count >= 5 ) {
    tsmError( funcName, "Couldn't add solvable - too many solvables" );
    return;
  }

  /* add the solvable function to the internal list */

  AddSolvable( solvable );

  /* extract the function part of the solvable */

  func = strdup( solvable );
  if ( ( ptr = strchr( func, '(' ) ) != NULL )
    *ptr = '\0';

  /* add all of this to our list of callbacks */

  oaa_callback_list.cb[oaa_callback_list.count] = callback;
  oaa_callback_list.func[oaa_callback_list.count] = func;
  oaa_callback_list.count++;
}

static int
OAAEvent( char *KS, char *func, char *args, char **answers )
{
  const char *funcName = "OAAEvent";
  char *function, *func_name, *func_args, *opt_args;
  int  success, i;

  /* do some sanity checks first */

  if ( func == NULL || args == NULL ) {
    *answers = (char *) malloc( 3 );
    strcpy( *answers, "[]" );
    return FALSE;
  }

  /* does this match a solvable that was added to our table */

  for ( i = 0; i < oaa_callback_list.count; i++ ) {
    if ( strcmp( func, oaa_callback_list.func[i] ) == 0 ) {
      if ( oaa_callback_list.cb[i] )
	return (oaa_callback_list.cb[i])( func, args, answers );
    }
  }

  /* Currently we only support one event interally: terrain3d */

  if ( func == NULL || args == NULL || strcmp( func, "terrain3d" ) != 0 ) {
    *answers = (char *) malloc( 3 );
    strcpy( *answers, "[]" );
    return FALSE;
  }

  /* Setup a default answer: predicate succeeds with no variables returned */

  *answers = (char *) malloc( strlen(func) + strlen(args) + 10 );
  if ( *args )
    sprintf( *answers, "[%s(%s)]", func, args );
  else
    sprintf( *answers, "[%s]", func );

  /* Get the name of the function to be performed and its arguments */

  NthElt( args, 1, &function );
  remove_quotes( function );
  Functor( function, &func_name, &func_args );
  OAA_FREE( function );

  NthElt( args, 2, &opt_args );
  remove_quotes( opt_args );
  list_to_terms( opt_args );

  TSMTRACE(( OAA_MASK, funcName, "OAA: %s(%s)", func_name, func_args ));

  {
    OAAClosure func;
    func.name = func_name;
    func.args = func_args;
    func.opt_args = opt_args;
    func.answers = answers;

    tvForallWindows (OAAFunc, &func);
    success = func.success;
  }

  OAA_FREE( func_name );
  OAA_FREE( func_args );
  OAA_FREE( opt_args );

  TSMTRACE(( OAA_MASK, funcName, "OAA: Returning answer %s", *answers ));
  return success;
}

/* Stub to make agentlib happy. */
void
oaa_idle ()
{
}

static int
OAAIsViewer( char const *args, char const *id )
{
  int  listlen, i;
  char *function, *func_name, *func_args;

  /* if there are no id() arguments, then this is for all viewers */

  if ( args == NULL || strlen( args ) == 0 ) return TRUE;
  if ( strstr( args, "id(" ) == NULL ) return TRUE;

  /* okay, we have one or more id() arguments - check them out */

  listlen = list_len( (char *) args );
  for ( i = 0; i < listlen; i++ ) {
    NthElt( (char *) args, i+1, &function );
    Functor( function, &func_name, &func_args );
    remove_quotes( func_args );

    /* do we have an id() argument matching this viewer's id? */
    if ( strcmp( func_name, "id" ) == 0 && strcmp( func_args, id ) == 0 ) 
      return TRUE;

    OAA_FREE( function );
    OAA_FREE( func_name );
    OAA_FREE( func_args );
  }

  /* if we got this far, then we didn't find a matching id - do nothing */
  return FALSE;
}

static void
OAAFunc (tvBase *threeD, void *closure)
{
  static char const funcName[] = "";
  OAAClosure *func = closure;
  double x, y, z, yaw, pitch, roll;
  int anim;
  char buffer[1024];
  char const *func_name = func->name;
  char const *func_args = func->args;
  char const *opt_args = func->opt_args;
  char **answers = func->answers;

  func->success = TRUE;

  /* check to see if this message is to be interpretted by this viewer */

  if ( ! OAAIsViewer( opt_args, threeD->oaa_id ) )
    return;

  /* display any received OAA commands on the console */

  tvBaseConsole( threeD, "OAA: %s(%s) %s", func_name,
                 func_args, opt_args );

  if ( ! strcmp( func_name, "refresh" ) ) {

    /* Support the terrain3d(refresh) solvable */

    tvRefresh( threeD );

  } else if ( ! strcmp( func_name, "stopmotion" ) ) {

    /* Support the terrain3d(stopmotion) solvable */

    tvStopMotion( threeD );

  } else if ( ! strcmp( func_name, "resetview" ) ) {

    /* Support the terrain3d(resetview) solvable */

    tvResetView( threeD );

  } else if ( ! strcmp( func_name, "getinfo" ) ) {

    /* Support the terrain3d(getinfo) solvable */

    char hostname[128], sysname[128], release[128], arch[128];
    char netbuf[PR_NETDB_BUF_SIZE];
    int  procs = (int) PR_GetNumberOfProcessors();
    PRHostEnt hostent;

    if ( CountArgs( func_args ) < 6 ) return;

    PR_GetSystemInfo( PR_SI_HOSTNAME, hostname, 128 );
    PR_GetSystemInfo( PR_SI_SYSNAME, sysname, 128 );
    PR_GetSystemInfo( PR_SI_RELEASE, release, 128 );
    PR_GetSystemInfo( PR_SI_ARCHITECTURE, arch, 128 );

    if ( PR_GetHostByName( hostname, netbuf, PR_NETDB_BUF_SIZE, &hostent ) )
      strcpy( hostname, hostent.h_name );

    sprintf( buffer, "[terrain3d(%s('%s','%s','%s %s','%s',%d,''),[])]",
	     func_name, threeD->oaa_id, hostname, sysname,
	     release, arch, procs );

    free( *answers );
    *answers = (char*) malloc( strlen( buffer ) + 1);
    strcpy( *answers, buffer );

  } else if ( ! strcmp( func_name, "message" ) ) {

    /* Support the terrain3d(message) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, buffer );
    tvMessage( threeD, buffer );

  } else if ( ! strcmp( func_name, "statusbar" ) ) {

    /* Support the terrain3d(statusbar) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, buffer );
    if ( threeD )
      tvBaseMessage( threeD, TV_STATUS_MSG, buffer );

  } else if ( ! strcmp( func_name, "getelev" ) ) {

    /* Support the terrain3d(getelev) solvable */
    
    if ( CountArgs( func_args ) < 2 ) return;

    x     = ExtractNthFloat( func_args, 1 );
    y     = ExtractNthFloat( func_args, 2 );

    if ( threeD ) {
      double gdc[3];
      float  elev;
      gdc[0] = y; gdc[1] = x; gdc[2] = 10000.0f;
      elev = tvBasePointElevationGDC( threeD, gdc );
      sprintf( buffer, "[terrain3d(getelev('%f','%f','%f'),[])]",
               x, y, elev );

      free( *answers );
      *answers = (char*) malloc( strlen( buffer ) + 1);
      strcpy( *answers, buffer );
    }

  } else if ( ! strcmp( func_name, "getview" ) ) {

    /* Support the terrain3d(getview) solvable */
    
    if ( CountArgs( func_args ) < 6 ) return;

    tvGetView3d( threeD, &x, &y, &z, &yaw, &pitch, &roll );
    sprintf( buffer, "[terrain3d(%s('%f','%f','%f','%f','%f','%f'),[])]",
	     func_name, x, y, z, yaw, pitch, roll );

    free( *answers );
    *answers = (char*) malloc( strlen( buffer ) + 1);
    strcpy( *answers, buffer );

  } else if ( ! strcmp( func_name, "setview" ) ) {

    /* Support the terrain3d(setview) solvable */
    
    if ( CountArgs( func_args ) < 6 ) return;

    x     = ExtractNthFloat( func_args, 1 );
    y     = ExtractNthFloat( func_args, 2 );
    z     = ExtractNthFloat( func_args, 3 );
    yaw   = ExtractNthFloat( func_args, 4 );
    pitch = ExtractNthFloat( func_args, 5 );
    roll  = ExtractNthFloat( func_args, 6 );

    tvSetView3d( threeD, x, y, z, yaw, pitch, roll );

  } else if ( ! strcmp( func_name, "getview_gdc" ) ) {

    /* Support the terrain3d(getview_gdc) solvable */
    
    if ( CountArgs( func_args ) < 3 ) return;

    tvGetViewGDC3d( threeD, &x, &y, &z );
    sprintf( buffer, "[terrain3d(%s('%f','%f','%f'),[])]",
	     func_name, x, y, z );
    free( *answers );

		*answers = (char*) malloc( strlen( buffer ) + 1);
		strcpy( *answers, buffer );

  } else if ( ! strcmp( func_name, "setview_gdc" ) ) {

    /* Support the terrain3d(setview_gdc) solvable */
    
    if ( CountArgs( func_args ) < 6 ) return;

    x     = ExtractNthFloat( func_args, 1 );
    y     = ExtractNthFloat( func_args, 2 );
    z     = ExtractNthFloat( func_args, 3 );
    yaw   = ExtractNthFloat( func_args, 4 );
    pitch = ExtractNthFloat( func_args, 5 );
    roll  = ExtractNthFloat( func_args, 6 );

    tvSetViewGDC3d( threeD, x, y, z, yaw, pitch, roll );

  } else if ( ! strcmp( func_name, "flyto" ) ) {

    /* Support the terrain3d(flyto) solvable */

    if ( CountArgs( func_args ) < 6 ) return;

    x     = ExtractNthFloat( func_args, 1 );
    y     = ExtractNthFloat( func_args, 2 );
    z     = ExtractNthFloat( func_args, 3 );
    yaw   = ExtractNthFloat( func_args, 4 );
    pitch = ExtractNthFloat( func_args, 5 );
    roll  = ExtractNthFloat( func_args, 6 );

    tvFlyTo3d( threeD, x, y, z, yaw, pitch, roll );

  } else if ( ! strcmp( func_name, "flyto_gdc" ) ) {
    double gcc[3];

    /* Support the terrain3d(flyto) solvable */

    if ( CountArgs( func_args ) < 6 ) return;

    x     = ExtractNthFloat( func_args, 1 );
    y     = ExtractNthFloat( func_args, 2 );
    z     = ExtractNthFloat( func_args, 3 );
    yaw   = ExtractNthFloat( func_args, 4 );
    pitch = ExtractNthFloat( func_args, 5 );
    roll  = ExtractNthFloat( func_args, 6 );

    if ( TsGDCtoGCC( y, x, z, gcc ) )
      tvFlyTo3d( threeD, gcc[0], gcc[1], gcc[2],
                 yaw, pitch, roll );

  } else if ( ! strcmp( func_name, "DIS_ID" ) ) {
    int id[3];

    /* Support the terrain3d(DIS_ID) solvable */

    if ( CountArgs( func_args ) < 3 ) return;

    id[0] = ExtractNthInt( func_args, 1 );
    id[1] = ExtractNthInt( func_args, 2 );
    id[2] = ExtractNthInt( func_args, 3 );

    if (id[0] == 0 && id[1] == 0 || id[2] == 0) {
      tvSetBoolean (threeD, TV_DIS_SEND, FALSE);
    } else {
      tvSetIntegerv (threeD, TV_DIS_ID, id);
      tvSetBoolean (threeD, TV_DIS_SEND, TRUE);
    }

  } else if ( ! strcmp( func_name, "move" ) ) {

    /* Support the terrain3d(move) solvable */
    
    if ( CountArgs( func_args ) < 3 ) return;

    x = ExtractNthFloat( func_args, 1 );
    y = ExtractNthFloat( func_args, 2 );
    z = ExtractNthFloat( func_args, 3 );

    anim = ! IsOption( opt_args, "noanim" );

    tvMoveViewpoint( threeD, x, y, z, anim );

  } else if ( ! strcmp( func_name, "loadmodel" ) ) {

    char url[1024];

    /* Support the terrain3d(loadmodel) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, url );
    FeatureRefCountLoad( threeD, url );

  } else if ( ! strcmp( func_name, "unloadmodel" ) ) {

    char url[1024];

    /* Support the terrain3d(unloadmodel) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, url );
    FeatureUnload( threeD, url );

  } else if ( ! strcmp( func_name, "addlabel" ) ) {

    /* Support the terrain3d(addlabel) solvable */
    
    if ( CountArgs( func_args ) < 4 ) return;

    ExtractNthArg( func_args, 1, buffer );
    x = ExtractNthFloat( func_args, 2 ); /* lat */
    y = ExtractNthFloat( func_args, 3 ); /* lon */
    z = ExtractNthFloat( func_args, 4 ); /* elev (m) */

    tvAddLabel( threeD, buffer, x, y, z );

  } else if ( ! strcmp( func_name, "removelabel" ) ) {
    char url[512];

    /* Support the terrain3d(removelabel) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, buffer );
    sprintf( url, "Label: %s", buffer );

    FeatureUnload( threeD, url );

  } else if ( ! strcmp( func_name, "pathload" ) ) {

    char url[1024];

    /* Support the terrain3d(pathload) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, url );

    if ( url[0] != '\0' )
      PathLoadFile( threeD, url );
    else
      PathNew( threeD );

  } else if ( ! strcmp( func_name, "pathreset" ) ) {

    /* Support the terrain3d(pathreset) solvable */
    
    PathReset( threeD );

  } else if ( ! strcmp( func_name, "pathplay" ) ) {

    /* Support the terrain3d(pathplay) solvable */
    
    PathPlay( threeD );

  } else if ( ! strcmp( func_name, "pathstop" ) ) {

    /* Support the terrain3d(pathstop) solvable */
    
    PathStop( threeD );

  } else if ( ! strcmp( func_name, "pathpause" ) ) {

    /* Support the terrain3d(pathpause) solvable */
    
    PathPause( threeD );

  } else if ( ! strcmp( func_name, "loadterrain" ) ) {

    char         oi_url[1024], dem_url[1024];
    TsGeoPyramid *geo;

    /* Support the terrain3d(loadterrain) solvable */
    
    if ( CountArgs( func_args ) < 2 ) return;
    
    tvSetString( threeD, TV_SPLASH_MESSAGE, "Loading... Please wait." );
    tvBasePostRedraw( threeD, FALSE );

    ExtractNthArg( func_args, 1, oi_url );
    ExtractNthArg( func_args, 2, dem_url );

    tvBaseConsole( threeD, "Loading terrain (OAA) - %s", oi_url );
    if ( ( geo = TsGeoCreate_OIDEM ( oi_url, NULL, dem_url, NULL ) ) ) {
      tvAddDataset( threeD, geo );
      tvBaseConsole( threeD, "Loaded terrain succefully (OAA) - %s from %s",
		     TsGeoName(geo), oi_url );
    } else 
      tvBaseConsole( threeD, "Failed loading terrain (OAA) - %s", oi_url );


  } else if ( ! strcmp( func_name, "unloadterrain" ) ) {

    char         geo_name[512];
    TsGeoPyramid *geo;

    /* Support the terrain3d(unloadterrain) solvable */
    
    if ( CountArgs( func_args ) < 1 ) return;

    ExtractNthArg( func_args, 1, geo_name );

    if ( ( geo = tvGetDataset( threeD, geo_name ) ) ) {
      tvRemoveDataset( threeD, geo );
      tvBaseConsole( threeD, "Unloaded terrain (OAA) - %s", TsGeoName(geo) );
    }

  } else if ( ! strcmp( func_name, "set" ) ) {

    /* Support the terrain3d(set) solvable */
    
    if ( CountArgs( func_args ) < 2 ) return;

    ExtractNthArg( func_args, 1, buffer );

    if ( ! strcmp( buffer, "SHOW_LIGHTING" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_LIGHTING, value );

    } else if ( ! strcmp( buffer, "SHOW_HEADS_UP" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_HEADS_UP, value );

    } else if ( ! strcmp( buffer, "SHOW_HEADS_UP_DEBUG" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_HEADS_UP_DEBUG, value );

    } else if ( ! strcmp( buffer, "SHOW_WIREFRAME" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_WIREFRAME, value );

    } else if ( ! strcmp( buffer, "SHOW_TEXTURES" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_TEXTURES, value );

    } else if ( ! strcmp( buffer, "SHOW_SKY" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_SKY, value );

    } else if ( ! strcmp( buffer, "SHOW_GROUND" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_GROUND, value );

    } else if ( ! strcmp( buffer, "SHOW_DATASETS" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_DATASETS, value );

    } else if ( ! strcmp( buffer, "SHOW_FRUSTUMS" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_FRUSTUMS, value );

    } else if ( ! strcmp( buffer, "SHOW_TILES" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_TILES, value );

    } else if ( ! strcmp( buffer, "SHOW_PATHS" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_PATHS, value );

    } else if ( ! strcmp( buffer, "SHOW_LOGO" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_LOGO, value );

    } else if ( ! strcmp( buffer, "SHOW_CROSSHAIR" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_CROSSHAIR, value );

    } else if ( ! strcmp( buffer, "SHOW_MODELS" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_MODELS, value );

    } else if ( ! strcmp( buffer, "FEATURES_WIREFRAME" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FEATURES_WIREFRAME, value );

    } else if ( ! strcmp( buffer, "FEATURES_TEXTURED" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FEATURES_TEXTURED, value );

    } else if ( ! strcmp( buffer, "FEATURES_LIGHTING" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FEATURES_LIGHTING, value );

    } else if ( ! strcmp( buffer, "FEATURES_LOAD_TEXTURES" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FEATURES_LOAD_TEXTURES, value );

    } else if ( ! strcmp( buffer, "FEATURES_BACKFACE_CULL" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FEATURES_BACKFACE_CULL, value );

    } else if ( ! strcmp( buffer, "BACKFACE_CULL" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_BACKFACE_CULL, value );

    } else if ( ! strcmp( buffer, "HORIZON_CULL" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_HORIZON_CULL, value );

    } else if ( ! strcmp( buffer, "USE_ZBUFFER_HACK" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_USE_ZBUFFER_HACK, value );

    } else if ( ! strcmp( buffer, "FREEZE_TILES" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_FREEZE_TILES, value );

    } else if ( ! strcmp( buffer, "SHOW_TILE_DEBUG" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SHOW_TILE_DEBUG, value );

    } else if ( ! strcmp( buffer, "STEREO" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_STEREO, value );

    } else if ( ! strcmp( buffer, "SCALE_IPD" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_SCALE_IPD, value );

    } else if ( ! strcmp( buffer, "TWOD_CONSTRAIN" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_TWOD_CONSTRAIN, value );

    } else if ( ! strcmp( buffer, "MIPMAP_TEXTURES" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_MIPMAP_TEXTURES, value );

    } else if ( ! strcmp( buffer, "HEIGHT_BASED_NAVIGATION" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_HEIGHT_BASED_NAVIGATION, value );

    } else if ( ! strcmp( buffer, "DOODLE_ON_TERRAIN" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_DOODLE_ON_TERRAIN, value );

    } else if ( ! strcmp( buffer, "DIS_SEND" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_DIS_SEND, value );

    } else if ( ! strcmp( buffer, "DIS_MONITOR" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_DIS_MONITOR, value );

    } else if ( ! strcmp( buffer, "DIS_SITE_ID" ) ) {
      int value = ExtractNthInt( func_args, 2 );
      tvSetInteger( threeD, TV_DIS_SITE_ID, value );

    } else if ( ! strcmp( buffer, "DIS_FORCE_ID" ) ) {
      int value = ExtractNthInt( func_args, 2 );
      tvSetInteger( threeD, TV_DIS_FORCE_ID, value );

    } else if ( ! strcmp( buffer, "DIS_ENTITY_ID" ) ) {
      int value = ExtractNthInt( func_args, 2 );
      tvSetInteger( threeD, TV_DIS_ENTITY_ID, value );

    } else if ( ! strcmp( buffer, "DIS_ID" ) ) {
      int value[3];
      if ( CountArgs( func_args ) < 4 ) return;
      value[0] = ExtractNthInt( func_args, 2 );
      value[1] = ExtractNthInt( func_args, 3 );
      value[2] = ExtractNthInt( func_args, 4 );
      tvSetIntegerv( threeD, TV_DIS_ID, value );

    } else if ( ! strcmp( buffer, "CROSSHAIR" ) ) {
      int value[2];
      if ( CountArgs( func_args ) < 3 ) return;
      value[0] = ExtractNthInt( func_args, 2 );
      value[1] = ExtractNthInt( func_args, 3 );
      tvSetIntegerv( threeD, TV_CROSSHAIR, value );

    } else if ( ! strcmp( buffer, "VISIBLE_CYCLE" ) ) {
      int value[2];
      if ( CountArgs( func_args ) < 3 ) return;
      value[0] = ExtractNthInt( func_args, 2 );
      value[1] = ExtractNthInt( func_args, 3 );
      tvSetIntegerv( threeD, TV_VISIBLE_CYCLE, value );

    } else if ( ! strcmp( buffer, "TIME_OF_DAY" ) ) {
      float value = ExtractNthFloat( func_args, 2 );
      tvSetFloat( threeD, TV_TIME_OF_DAY, value );

    } else if ( ! strcmp( buffer, "RESOLUTION_ADJUST" ) ) {
      float value = ExtractNthFloat( func_args, 2 );
      tvSetFloat( threeD, TV_RESOLUTION_ADJUST, value );

    } else if ( ! strcmp( buffer, "ABSOLUTE_IPD" ) ) {
      float value = ExtractNthFloat( func_args, 2 );
      tvSetFloat( threeD, TV_ABSOLUTE_IPD, value );

    } else if ( ! strcmp( buffer, "ELEVATION_SCALE" ) ) {
      float value = ExtractNthFloat( func_args, 2 );
      tvSetFloat( threeD, TV_ELEVATION_SCALE, value );

    } else if ( ! strcmp( buffer, "MOTION_THRESHOLD" ) ) {
      float value = ExtractNthFloat( func_args, 2 );
      tvSetFloat( threeD, TV_MOTION_THRESHOLD, value );

    } else if ( ! strcmp( buffer, "DOTGEO_PAUSE" ) ) {
      int value = ExtractNthInt( func_args, 2 );
      tvSetInteger( threeD, TV_DOTGEO_PAUSE, value );

    } else if ( ! strcmp( buffer, "AUTO_POP_FRONT" ) ) {
      PRBool value = ExtractNthBoolean( func_args, 2 );
      tvSetBoolean( threeD, TV_AUTO_POP_FRONT, value );

    } else if ( ! strcmp( buffer, "SCHEMA_URL" ) ) {
      char value[1024];
      ExtractNthArg( func_args, 2, value );
      tvSetString( threeD, TV_SCHEMA_URL, value );

    } else if ( ! strcmp( buffer, "OAA_ID" ) ) {
      char value[1024];
      ExtractNthArg( func_args, 2, value );
      tvSetString( threeD, TV_OAA_ID, value );

    }

  } else if ( ! strcmp( func_name, "translate" ) ) {

    /* Support the terrain3d(translate) solvable */
    
    if ( CountArgs( func_args ) < 3 ) return;

    x = ExtractNthFloat( func_args, 1 );
    y = ExtractNthFloat( func_args, 2 );
    z = ExtractNthFloat( func_args, 3 );

    anim = ! IsOption( opt_args, "noanim" );

    tvTranslate( threeD, x, y, z, anim );

  } else if ( ! strcmp( func_name, "rotate" ) ) {

    /* Support the terrain3d(rotate) solvable */
    
    if ( CountArgs( func_args ) < 3 ) return;

    yaw   = ExtractNthFloat( func_args, 1 );
    pitch = ExtractNthFloat( func_args, 2 );
    roll  = ExtractNthFloat( func_args, 3 );

    anim = ! IsOption( opt_args, "noanim" );

    tvRotate( threeD, yaw, pitch, roll, anim );

  } else {

    /* Do not know this function - returned a failure event */

    TSMTRACE ((OAA_MASK, funcName, "OAA: unknown solvable %s", func_name));
    free( *answers );
    *answers = (char *) malloc( 3 );
    strcpy( *answers, "[]" );
    func->success = FALSE;
  }
}

/****************************************************************************
 * name:    idle
 * purpose: Called every time a timeout occurs.
 ****************************************************************************/

void
idle(void)
{
  const char *funcName = "idle";
  TSMTRACE(( OAA_MASK, funcName, "Entering" ));
}

/****************************************************************************
 * name:    app_done
 * purpose: Application specific deallocation before exiting
 ****************************************************************************/

void
app_done(void)
{
  const char *funcName = "app_done";
  TSMTRACE(( OAA_MASK, funcName, "Entering" ));
}

/************************ ON SCREEN DRAWING ROUTINES **********************/

void
InitSentence (tvBase *threeD)
{
  SentenceType *sentence = &threeD->sentence;

  /* Initialise the sentence to be empty */

  sentence->stroke    = NULL;
  sentence->highwater = 0;
  sentence->count     = 0;
  sentence->projected = FALSE;
}

/*
 * Synopsis:
 *   void ClearSentence( tvBase *threeD )
 *
 * Doc++ Description: *//**
 *   Called whenever we want to create a new sentence.
 *   This function will initialise the sentence structure so that
 *   it releases all previously allocated memory and resets all
 *   arrays to NULL. .
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 * @author  Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
ClearSentence( tvBase *threeD )
{
  int i;
  SentenceType *sentence = &threeD->sentence;

  /* Free a currently allocated sentence */

  if ( sentence->stroke != NULL ) {
    for ( i = 0; i < sentence->count; i++ ) {
      if ( sentence->stroke[i].coord != NULL )
	free( sentence->stroke[i].coord );
    }
    free( sentence->stroke );
  }

  InitSentence (threeD);
}

/*
 * Synopsis:
 *   void SendSentence( tvBase *threeD )
 *
 * Description:
 *   This function will send the current sentence structure to
 *   the facilitor to be interpreted and acted upon. This is
 *   called whenever a sentence has been completed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

static void
SendSentence( tvBase *threeD )
{
  /*static const char funcName[] = "SendSentence";*/
  char       *content = NULL, buffer[100];
  SentenceType *sentence = &threeD->sentence;
  StrokeType *stroke;
  int        i, j;

  if ( sentence->stroke == NULL ) return;

  /* Build a string containing all of the points */

  append( &content, "inform_ui(pen,points([" );
  
  for ( i = 0; i < sentence->count; i++ ) {

    stroke = &sentence->stroke[i];
    append( &content, "[" );
      
    for ( j = 0; j < stroke->count; j++ ) {
      sprintf( buffer, "%d,%d%s",
	       (int) stroke->coord[j].x, (int) stroke->coord[j].y,
	       ( j < stroke->count - 1 ) ? "," : "" );
      append( &content, buffer );
    }

    append( &content, "]" );
    if ( i < sentence->count - 1 ) append( &content, "," );
  }

  append( &content, "]))" );
  
  /* Send the pen strokes to the OAA to solve */

  if ( TSMTRACEP( OAA_MASK) ) tsmStatus( "%s", content );

  if ( is_oaa )
    solve( content, "[broadcast]", NULL );
}

/*
 * Synopsis:
 *   void ProjectSentence( tvBase *threeD )
 *
 * Description:
 *   This function will project the sentence onto the 3D terrain and store it
 *   in the coords for latter rendering.
 *
 * Returns: void
 *
 * Author: Nat Bletter
 *
 * Date: 4/9/99
 *
 */

static void
ProjectSentence( tvBase *threeD )
{
  /*static const char funcName[] = "ProjectSentence";*/
  SentenceType        *sentence = &threeD->sentence;
  StrokeType          *stroke;
  PenCoordType        *coord;
  int                 i, j;

  /* Do we have anything to actually project yet? */

  if ( sentence->stroke == NULL ) return;

  for ( i = 0; i < sentence->count; i++ ) {
    stroke = &sentence->stroke[i];
    for ( j = 0; j < stroke->count; j++ ) {
      coord = &stroke->coord[j];
      tvBaseDrawProcessPick(threeD, coord->x, coord->y, coord->proj);
    }
  }

  sentence->projected = TRUE;
}


/*
 * Synopsis:
 *   void AddPenCoord( tvBase *threeD, int x, int y )
 *
 * Description:
 *   Adds a new (x,y) coordinate to the current stroke. This
 *   function will initialise the sentence structure if it has
 *   not been initialsed yet, it will also dynamically grow the
 *   array of points for this stroke as necessary.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

static void
AddPenCoord( tvBase *threeD, int x, int y )
{
  SentenceType *sentence = &threeD->sentence;
  StrokeType *stroke;
  int i;
  
  /* Do we need to allocate an array for the strokes? */

  if ( sentence->stroke == NULL )
    stroke = NewStroke (threeD);
  else
    stroke = &sentence->stroke[sentence->count-1];

  /* Do we need to allocate an array for this stroke? */

  if ( stroke->coord == NULL ) {
    stroke->coord = (PenCoordType *) calloc( stroke->highwater, 
					     sizeof(PenCoordType) );
    if ( stroke->coord == NULL ) return;
    stroke->count = 0;
  }

  /* Do we need to allocate more memory for this stroke? */
  i = stroke->count++;
  if ( stroke->count >= stroke->highwater ) {
    stroke->highwater += HIGH_WATER_MARK;
    if (! stroke->coord)
      stroke->coord = (PenCoordType *) malloc( stroke->highwater
					       * sizeof(PenCoordType) );
    else
      stroke->coord = (PenCoordType *) realloc( stroke->coord,
						stroke->highwater
						* sizeof(PenCoordType) );
    if ( stroke->coord == NULL ) return;
  }

  /* All ready to add a new coordinate at long last! */

  stroke->coord[i].x = (unsigned int) x;
  stroke->coord[i].y = (unsigned int) y;
}

/*
 * Synopsis:
 *   StrokeType * NewStroke( tvBase *threeD )
 *
 * Description:
 *   This functions is called in order to start a new stroke.
 *   If it is called when no current strok has been allocated,
 *   then a new stroke is allocated and the function returns.
 *   This function is called whenever the user starts a new
 *   stroke.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

static StrokeType *
NewStroke( tvBase *threeD )
{
  int byte_size, i;
  SentenceType *sentence = &threeD->sentence;
  StrokeType *stroke;

  /* Do we need to allocate more memory for the stroke array? */
  i = sentence->count;
  ++sentence->count;
  
  if ( sentence->count >= sentence->highwater ) {
    sentence->highwater += HIGH_WATER_MARK;
    byte_size = sentence->highwater * sizeof(StrokeType);

    if (! sentence->stroke)
      sentence->stroke = (StrokeType *) malloc (byte_size);
    else
      sentence->stroke = (StrokeType *) realloc (sentence->stroke, byte_size);
    if ( sentence->stroke == NULL ) return NULL;
  }

  /* Okay, let's initiate a new stroke then */
  stroke = &sentence->stroke[i];
  stroke->coord = NULL;
  stroke->highwater = 0;
  stroke->count = 0;

  return stroke;
}

/*
 * Synopsis:
 *   void RenderSentence( tvBase *, int is_locked )
 *
 * Description:
 *   This function will create the OpenGL primitives for rendering
 *   the current sentence. If you pass TRUE for the parameter is_locked
 *   then we assume that the calling function has already locked the
 *   OpenGL process (e.g. the full screen refresh). If you pass FALSE,
 *   then we assume that we are just doing a partial screen refresh ontop
 *   of the last full render - we therefore lock the OpenGL process and
 *   perform a glFlush at the end to display the incremental update.
 *
 *   By enabling incremental updates to the frame buffer we can make the
 *   drawing on screen more interactive as we don't need to render the
 *   texture-mapped terrain in the background at each new event.
 *
 *   Another feature that we have here is the ability to force any 
 *   number of successive full refreshes instead of incremental ones.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

void
RenderSentence( tvBase *threeD, int is_locked )
{
  const char          *funcName = "RenderSentence";
  SentenceType        *sentence = &threeD->sentence;
  StrokeType          *stroke;
  PenCoordType        *coord;
  int                 i, j;

  /* Do we wish to trigger a full refresh instead of only an incremental? */

  if ( fullRefresh > 0 ) {
    if ( is_locked )
      fullRefresh--;   /* receive refresh request inside render thread */
    else {
      /* refresh request in drawing thread */
      tvRefresh( threeD );
      return;
    }
  }

  /* Do we have anything to actually draw yet? */

  if ( sentence->stroke == NULL ) return;

  if ( ! is_locked ) glmMakeCurrentWindow (funcName, threeD->glmw );

  /* Generate the OpenGL constructs for the current sentence */
  glPushAttrib( GL_CURRENT_BIT|GL_ENABLE_BIT ); {
    if (sentence->projected && threeD->projectToTerrain) {
      glPushMatrix(); {

	glDisable( GL_TEXTURE_2D );
	glColor4ub( 0xff, 0xff, 0x00, 0xff );
	
	glLineWidth( (GLfloat) 2 );
	glPointSize( (GLfloat) 2 );

	glTranslatef(- threeD->view.eye[0],
		     - threeD->view.eye[1],
		     - threeD->view.eye[2]);
      
	for ( i = 0; i < sentence->count; i++ ) {
	  stroke = &sentence->stroke[i];
	  glBegin( (stroke->count==1) ? GL_POINTS : GL_LINE_STRIP ); {
	    for ( j = 0; j < stroke->count; j++ ) {
	      coord = &stroke->coord[j];
	      glVertex3d(coord->proj[0], coord->proj[1]+5, coord->proj[2]);
	    }
	  }; glEnd();
	}

      }; glPopMatrix();
    }
    else if (!sentence->projected) {

      glMatrixMode( GL_PROJECTION );
      glPushMatrix(); {
	glLoadIdentity();
	glOrtho (-0.5, threeD->width-0.5, -0.5, threeD->height-0.5, -1.0, 1.0);
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix(); {
	  glLoadIdentity();
	  glDisable( GL_TEXTURE_2D );
	  glDisable( GL_DEPTH_TEST );
	  glColor4ub( 0xff, 0xff, 0x00, 0xff );
	
	  glLineWidth( (GLfloat) 2 );
	  glPointSize( (GLfloat) 2 );

	  for ( i = 0; i < sentence->count; i++ ) {
	    stroke = &sentence->stroke[i];
	    glBegin( (stroke->count==1) ? GL_POINTS : GL_LINE_STRIP ); {
	      for ( j = 0; j < stroke->count; j++ ) {
		coord = &stroke->coord[j];
		glVertex2i ((int) coord->x,
			    threeD->height - (int) coord->y);
	      }
	    }; glEnd();
	  }

	}; glPopMatrix();
      }; glPopMatrix();
    }
  }; glPopAttrib();

  if ( ! is_locked ) {
    glFlush ();
    glmReleaseWindow( funcName, threeD->glmw );
  }
}

/*
 * Synopsis:
 *   void DrawTimeOut( void *closure )
 *   void ClearDrawTimeOut( tvBase * )
 *   void StartDrawTimeOut( tvBase * )
 *
 * Description:
 *   These three functions manage the ability of TerraVision to
 *   detect when the user has finised a sentence. We assume that
 *   the user has finished a sentence if they have not entered
 *   a new stroke within some short time interval (e.g. 1 sec).
 *   We implement this by setting up a timer callback which,
 *   when called, will complete the sentence. Whenever the user
 *   starts a new stroke, this callback is removed.
 *
 * Returns: void
 *
 * Author: Martin Reddy
 *
 * Date: 20 January 1999
 *
 */

static void
DrawTimeOut( void *closure )
{
  tvBase *threeD = closure;

  /* Flush out the last stroke and ship the whole sentence off */
  NewStroke (threeD); 
  SendSentence (threeD);
  if (threeD->projectToTerrain)
    ProjectSentence (threeD);
  else
    ClearSentence (threeD);
  tvRefresh (threeD);
  fullRefresh = 2; /* we're double buffered, so redraw twice */

  /* Make sure we know that this has been cleared. */
  threeD->drawTimer = 0;
}

static void
ClearDrawTimeOut( tvBase *threeD )
{
  if ( threeD->drawTimer )
    tvBaseDelTimer (threeD, &threeD->drawTimer);
}

static void
StartDrawTimeOut( tvBase *threeD )
{
  if ( time_out <= 0 ) return;  /* timeout of 0 means never send doodle */

  ClearDrawTimeOut (threeD);
  tvBaseAddTimer (threeD, &threeD->drawTimer, time_out, 
		  DrawTimeOut, threeD );
}

/*
 * Synopsis:
 *   void SetDrawTimeout( float secs )
 *
 * Doc++ Description: *//**
 *   Used to set the timeout delay that is used before a doodle is sent to the
 *   OAA facilitator (if connected) and the doodle erased.
 *
 * @param  secs   number of seconds to delay
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  7 February 1999
 *
 */
void
SetDrawTimeout (float secs)
{
  static const char *funcName = "SetDrawTimeout";
  TSMTRACE(( OAA_MASK, funcName, "Setting time_out to %1.2f sec", secs ));
  time_out = (int) ( secs * 1000.0 );
}

/*
 * Synopsis:
 *   float GetDrawTimeout( void )
 *
 * Doc++ Description: *//**
 *   Used to query the timeout delay that is used before a doodle is sent to 
 *   the OAA facilitator (if connected) and the doodle erased.
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  7 February 1999
 *
 * @see      SetDrawTimeout
 *
 */
float
GetDrawTimeout (void)
{
  return (float) time_out / 1000.0;
}

/*
 * Synopsis:void tvBeginStroke (tvBase *threeD, int x, int y)
 *
 * Doc++ Description: *//**
 *   Start a doodle stroke at given coordinates.  Should be called on
 *   mouse down.
 *
 * @param threeD   handle to rendering state
 * @param x        coordinate of stroke start-point
 * @param y        "
 *
 * @return   void
 *
 * @author   Lee Iverson
 *
 * @version  December 1999
 *
 * @see      tvContinueStroke,  tvEndStroke
 *
 */
void
tvBeginStroke (tvBase *threeD, int x, int y)
{
  tvStopMotion (threeD);
  if (threeD->sentence.projected)
    ClearSentence (threeD);
  NewStroke (threeD);
  AddPenCoord (threeD, x, y);
  ClearDrawTimeOut (threeD);
}

/*
 * Synopsis:void tvContinueStroke (tvBase *threeD, int x, int y)
 *
 * Doc++ Description: *//**
 *   Continue a stroke started by tvBeginStroke().  Should be called on
 *   mouse motion between button down and up events.
 *
 * @param threeD   handle to rendering state
 * @param x        coordinate of stroke point
 * @param y        "
 *
 * @return   void
 *
 * @author   Lee Iverson
 *
 * @version  December 1999
 *
 * @see      tvBeginStroke, tvEndStroke
 *
 */
void
tvContinueStroke (tvBase *threeD, int x, int y)
{
  AddPenCoord( threeD, x, y );
  RenderSentence( threeD, FALSE );
}

/*
 * Synopsis:void tvEndStroke (tvBase *threeD)
 *
 * Doc++ Description: *//**
 *   Complete a doodle stroke.  Should be called on mouse up.
 *
 * @param threeD  handle to rendering state
 *
 * @return   void
 *
 * @author   Lee Iverson
 *
 * @version  December 1999
 *
 * @see      tvBeginStroke, tvContinueStroke
 *
 */
void
tvEndStroke (tvBase *threeD)
{
  RenderSentence( threeD, FALSE );
  StartDrawTimeOut( threeD );
}

/*********************** OAA API HOOKS TO CONTROL TV **********************/

/*
 * Synopsis:
 *   void tvStopMotion( void )
 *
 * Doc++ Description: *//**
 *   This function will stop any current camera animation that is currently 
 *   playing in the 3-D viewer.  It provides a hook for the OAA to control 
 *   TerraVision. The user can use LMB/RMB to induce forward and backward 
 *   momentum - this call nulls that out and stops the viewpoint.
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 * @author  Martin Reddy
 *
 * @version 20 January 1999
 *
 */
void
tvStopMotion( tvBase *threeD )
{
  /* If the animation timer for the 3-D viewer is set, then remove it */
  tvStopMovement (threeD);
}

/*
 * Synopsis:
 *   void tvRefresh( void )
 *
 * Doc++ Description: *//**
 *   This function will simply perform a refresh of the current viewpoint.
 *
 * @return void
 *
 * @author Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
tvRefresh( tvBase *threeD )
{
  if ( threeD == NULL ) return;
  tvBasePostRedraw( threeD, FALSE );
}

/*
 * Synopsis:
 *   void tvFlyTo3d( double, double, double, double, double, double )
 *
 * Doc++ Description: *//**
 *   This does the same job as tvSetView3d, except that it performs
 *   a funky cool animation from the current viewpoint to the new
 *   location. As before (x,y,z) are meters, and (yaw,pitch,roll) are
 *   radians.
 *
 * @return void
 *
 * @author Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
tvFlyTo3d( tvBase *threeD, double x, double y, double z,
	   double yaw, double pitch, double roll )
{
  if ( threeD == NULL ) return;

  tvFlyTo(threeD, x, y, z, yaw, pitch, roll );
}

/*
 * Synopsis:
 *   void tvTranslate( double dx, double dy, double dz )
 *
 * Doc++ Description: *//**
 *   This function will move the viewpoint in the 2/3 viewer. This
 *   is a relative move, i.e. you move (dx,dy,dy) from the current
 *   viewpoint position.
 *
 * @param   dx  relative offsets for new viewpoint
 * @param   dy  ditto
 * @param   dz  ditto
 *
 * @return  void
 *
 * @author  Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
tvTranslate( tvBase *threeD, 
	       double dx, double dy, double dz,
	       int anim )
{
  tvViewpoint view;

  if ( threeD == NULL ) return;

  tvCurrentView( threeD, &view );

  view.eye[0] += dx;
  view.eye[1] += dy;
  view.eye[2] += dz;

  if ( anim )
    tvFlyTo3d( threeD, view.eye[0], view.eye[1], view.eye[2],
	       view.yaw, view.pitch, view.roll );
  else
    tvSetView( threeD, &view, FALSE );
}

/*
 * Synopsis:
 *   void tvRotate( tvBase *threeD, double dyaw, double dpitch, 
 *                   double droll, int anim)
 *
 * Doc++ Description: *//**
 *   This function will rotate the viewpoint in the 3-D viewer. This
 *   is a relative rotation, i.e. you rotate (dyaw,dpitch,droll) 
 *   relative to the viewer's current orientation. All angles are
 *   given in units of radians.
 *
 * @param threeD   handle to rendering state
 * @param dyaw     yaw  (ISSUE:  units?  degs vs. rads?)
 * @param dpitch   pitch
 * @param droll    roll
 * @param anim     boolean value indicating whether animation is in progress
 *                 at the point this function is called.  1 = animation in 
 *                 progress,  0 = static.
 *
 * @return   void
 *
 * @author   Martin Reddy
 *
 * @version  20 January 1999
 *
 */

void
tvRotate( tvBase *threeD, 
	  double dyaw, double dpitch, double droll,
	  int anim )
{
  tvViewpoint view;

  if ( threeD == NULL ) return;

  tvCurrentView (threeD, &view);

  view.yaw   += dyaw;
  view.pitch += dpitch;
  view.roll  += droll;

  if ( anim )
    tvFlyTo3d( threeD, view.eye[0], view.eye[1], view.eye[2],
	       view.yaw, view.pitch, view.roll );
  else
    tvSetView( threeD, &view, FALSE );
}

/*
 * Synopsis:
 *   int tvMoveViewpoint( tvBase *threeD, 
 *                          double forward, double right, double up, int anim)
 *
 * Doc++ Description: *//**
 *   Moves the current viewpoint in a viewer-based direction. This
 *   means moving forward/backward along the path that the viewer is
 *   facing, sliding left/right/up/down in a direction perpendicular
 *   to the forward direction.
 *
 *   If the widget's view was moved, then the updated value is passed back to
 *   the widget via tvSetView().
 *
 * @param threeD   handle to rendering state
 * @param forward  forward delta
 * @param right    right delta
 * @param up       upwards delta
 * @param anim     boolean value indicating whether animation is in progress
 *                 at the point this function is called.  1 = animation in 
 *                 progress,  0 = static.
 *
 * @return  True if actually moved.
 *
 * @author  Martin Reddy
 *
 * @version 20 January 1999
 *
 */

int
tvMoveViewpoint( tvBase *threeD,
		   double forward, double right, double up,
		   int anim )
{
  /*static const char funcName[] = "tvMoveViewpoint";*/
  int moved = 0;
  tvViewpoint view;

  if ( threeD == NULL ) return 0;

  /* grab the current view. */
  tvCurrentView( threeD, &view );

  moved = tvAdjustView (threeD, &view, forward, right, up);

  /* update the display */
  if (moved) {
    if ( anim )
      tvFlyTo3d( threeD, view.eye[0], view.eye[1], view.eye[2],
		 view.yaw, view.pitch, view.roll );
    else
      tvSetView( threeD, &view, FALSE );
  }

  return moved;
}

/*
 * Synopsis:
 *   void tvResetView( void )
 *
 * Doc++ Description: *//**
 *   Returns the user to the inital viewpoint.
 *
 * @param threeD  handle to rendering state
 *
 * @return  void
 *
 * @author  Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
tvResetView( tvBase *threeD )
{
  if ( threeD == NULL ) return;
  tvBaseInitView(threeD, tvBaseTop (threeD));
}

/*
 * Synopsis:
 *   void tvMessage( char const *message )
 *
 * Doc++ Description: *//**
 *   This function will display a textual message inside a dialog
 *   window. This dialog window is a child of the 2D/3D viewer, i.e.
 *   it is iconfied when the 2D/3D viewer is iconified, etc. You can
 *   include escape sequences such as \n in the message string to
 *   format your text.
 *
 * @param  threeD  handle to rendering state
 * @param  message textual message
 *
 * @return void
 *
 * @author Martin Reddy
 *
 * @version 20 January 1999
 *
 */

void
tvMessage( tvBase *threeD, char const *message )
{
  if (! threeD) return;
  tvBaseMessage (threeD, TV_MESSAGE_MSG, message);
}

/*
* Synopsis: 
*   void tvAddLabel(tvBase *threeD, char *label, double lat, double lon, double elev)
*
* Description:
*   This function will create a text label at the specified lat/long
*   location and elevation. This is done by creating a GeoVRML scene
*   on the fly and then just adding it as any other 3D model. The
*   label is then treated like other 3D models and can be unloaded
*   through TerraVision's Close 3D Models interface.
*
* @param  threeD   The handle for the viewer we want to know the visible
*                  extent off.
* @param  label    The text string to display
* @param  lat      The geodetic latitude for the label
* @param  lon      The geodetic longitude for the label
* @param  elev     The elevation for the label above the terrain in m
* 
* @return void
*
* @author Martin Reddy
*
* @date 3 April 2001
*
*/

void
tvAddLabel(tvBase *threeD, char *label, double lat, double lon, double elev)
{
  static char const funcName[] = "tvAddLabel";
	char *vrml = NULL, buffer[512];

  if ( threeD == NULL || label == NULL ) return;

  tvBaseConsole( threeD, "Creating label - %s", label );

	/* create the header part with all the EXTERNPROTOs and GeoLocation */
  vrml = tsmStringCat( vrml, "#VRML V2.0 utf8\n" );
  vrml = tsmStringCat( vrml, "EXTERNPROTO GeoLocation [\n" );
  vrml = tsmStringCat( vrml, "  field SFNode   geoOrigin\n" );
  vrml = tsmStringCat( vrml, "  field MFString geoSystem\n" );
  vrml = tsmStringCat( vrml, "  field SFString geoCoords\n" );
  vrml = tsmStringCat( vrml, "  field MFNode   children\n" );
  vrml = tsmStringCat( vrml, "] \"http://www.ai.sri.com/geovrml/1.0/protos/GeoLocation.wrl\"\n" );
  vrml = tsmStringCat( vrml, "EXTERNPROTO GeoOrigin [\n" );
  vrml = tsmStringCat( vrml, "  exposedField  MFString  geoSystem\n" );
  vrml = tsmStringCat( vrml, "  exposedField  SFString  geoCoords\n" );
  vrml = tsmStringCat( vrml, "] \"http://www.ai.sri.com/geovrml/1.0/protos/GeoOrigin.wrl\"\n" );
  vrml = tsmStringCat( vrml, "GeoLocation {\n" );
  vrml = tsmStringCat( vrml, "  geoSystem [ \"GDC\" ]\n" );

	/* insert the coorect coordinates into the GeoLocation node */
  sprintf( buffer, "geoCoords \"%1.10lg %1.10lg +%1.10lg\"\n", 
           lat, lon, elev );
	vrml = tsmStringCat( vrml, buffer );
	vrml = tsmStringCat( vrml, "geoOrigin DEF GEOORG GeoOrigin {\n" );
	vrml = tsmStringCat( vrml, "  geoSystem [ \"GDC\" ]\n  " );
	vrml = tsmStringCat( vrml, buffer );
	vrml = tsmStringCat( vrml, "}\n" );

	/* output the Shape node to wrap the real geometry up in */
	vrml = tsmStringCat( vrml, "children Shape {\n" );

  vrml = tsmStringCat( vrml, "appearance Appearance {" );
  vrml = tsmStringCat( vrml, "  material Material { diffuseColor 1 1 1 } " );
  vrml = tsmStringCat( vrml, "}" );
  vrml = tsmStringCat( vrml, "geometry Text {" );
  vrml = tsmStringCat( vrml, "  string [ \"" );
  vrml = tsmStringCat( vrml, label );
  vrml = tsmStringCat( vrml, "\" ]" );
  vrml = tsmStringCat( vrml, "  fontStyle FontStyle {" );
  vrml = tsmStringCat( vrml, "    family \"SANSERIF\"" );
  vrml = tsmStringCat( vrml, "    style  \"PLAIN\"" );
  vrml = tsmStringCat( vrml, "    size   10000" );
  vrml = tsmStringCat( vrml, "  }" );
  vrml = tsmStringCat( vrml, "}" );

	/* output the trailing VRML to finish off the file */
	vrml = tsmStringCat( vrml, "  }\n }\n" );

  /* tvBaseConsole( threeD, "GeoVRML Label = \n%s\n", vrml ); */

	/* okay, now we've build a GeoVRML wrapper for the Label. */
	/* let's parse it and all it to the scene */

	sprintf( buffer, "Label: %s", label );
  if ( FeatureRefCountParse( threeD, vrml, buffer ) ) {
    tvBaseMessage( threeD, TV_MODEL_ADD_MSG, buffer );
  } else {
    tvBaseConsole( threeD, "Could not create label  %s", label );
  }

}

/*
 * Synopsis:
 *   int tvInterpret( tvBase *threeD, char const *command )
 *
 *   Interprets a command to the TerraVision core. This command is
 *   in the form used by the OAA interface (see OAAEvent above), e.g.
 *   valid command strings include "resetview", "message(hello)", and
 *   "setview_gdc(0,0,0,0,0,0)". In addition, you may specify an
 *   optional argument as in the OAA interface, by supplying this in
 *   square parenthesis after a comma, for example,
 *   "translate(X,Y,Z),[noanim]".
 *
 * Doc++ Description: *//**
 *   Interprets a command to the TerraVision core. This command is
 *   in the form used by the OAA interface; e.g.
 *   valid command strings include "resetview", "message(hello)", and
 *   "setview_gdc(0,0,0,0,0,0)". In addition, you may specify an
 *   optional argument as in the OAA interface, by supplying this in
 *   square parenthesis after a comma, for example,
 *   "translate(X,Y,Z),[noanim]".
 *
 * @return   TRUE if command was executed successfully, FALSE otherwise.
 *
 * @author   Martin Reddy
 *
 * @version  20 December 1999
 *
 */

int
tvInterpret( tvBase *threeD, char const *command )
{
  const char *funcName = "tvInterpret";
  char       *answer = NULL, *cmd, *ptr;
  int        result;

  if ( threeD == NULL || command == NULL ) return FALSE;

  /* allocate space for a new command string */

  if ( ( cmd = tsmMemMalloc( strlen(command) + 10 ) ) == NULL )
    return FALSE;

  /* build the OAA solvable string */

  sprintf( cmd, "%s", command );

  /* add an empty optional argument if no optional argument supplied */

  if ( ( ptr = strrchr( command, ')' ) ) == NULL )
    ptr = (char *) command;
  if ( strchr( ptr, ',' ) == NULL )
    strcat( cmd, ",[]" );

  /* send the command to the OAA interpreter - goes to OAAEvent() above */

  TSMTRACE(( OAA_MASK|1, funcName, "Interpret: %s", cmd ));

  OAAEvent( (char *) funcName, "terrain3d", cmd, &answer );

  /* a result of "[]" means failure. */

  result = ( answer != NULL && strcmp( answer, "[]" ) );

  /* clean up our temporary strings */

  if ( answer ) tsmMemFree( answer );
  tsmMemFree( cmd );

  return result;
}

#if 0
/*
 * Event management.  We'll take this out of agentlib since we have a more
 * portable way of doing it, but it relys on the timer handling provided by
 * the tvBase.
 */

/****************************************************************************
 * name:    OAATimer
 * purpose: If agent uses an X-Windows graphical user interface, oaa_timer
 *          can check for messages coming from the Server while still giving
 *          much control to X to move the mouse, refresh display, etc.
 * inputs:
 *   - tvBase *threeD: A context in which to create the timer.
 * remarks:
 *   - Calls handle_incoming_event: if an event arrives, executes intepret().
 ****************************************************************************/

static void *oaa_timer_id = 0;

static void
handle_incoming_event (void *closure)
{
  tvBase *threeD = closure;
  PRPollDesc pds[1];
  extern OAASOCKET Connection;

  oaa_timer_id = 0;

  /* Don't do anything if no connection. */
  if (Connection == INVALID_OAASOCKET)
    return;

  /* Check if any input is available */
  pds[0].fd = Connection;
  pds[0].in_flags = PR_POLL_READ;
  if (PR_Poll (pds, 1, PR_INTERVAL_NO_WAIT) > 0)
    oaa_handle_incoming_event(Connection);

  OAATimer (threeD);
}

static void
OAATimer(tvBase *threeD)
{
  if (! oaa_timer_id)
    oaa_timer_id = tvBaseAddTimer (threeD, 20,
				   handle_incoming_event, threeD );
}
#endif

static void
OAALoop(void *ignore)
{
  /* Listen in on the socket and dispatch when input ready */

  while (TRUE)
    if (! oaa_process_all_events())
      break;
}

