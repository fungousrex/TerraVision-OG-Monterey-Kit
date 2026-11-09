/* File     : agentlib.c
   Author  : Adam Cheyer
   Purpose : Contains C version of library for the Open Agent Architecture
   Updated : 11/10/96

   Copyright (C) 1995,1996 SRI International.  All rights reserved.

   "Open Agent Architecture" and "OAA" are trademarks of SRI International.
*/

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                          %
% AGENTLIB.c                                    Adam Cheyer     1/16/95    %
%                                                                          %
% Common routines used by all C agents in the Open Agent Architecture.     %
%                                                                          %
%                                                                          %
% Agent specific declarations:                                             % 
%                                                                          %
%       ksname                  : unique identifier, provided during       %
%          call to setup_communication()                                   %
%       timeout                 : timeout value for tcp_select, specified  %
%          by a call to set_timeout().                                     %
%       solvable                : goals that the KS can solve. String      %
%          must be passed in as argument to                                %
%                                 setup_communication.                     %
%                                                                          %
%       oaa_do_event()          : defines actions for agent-specific events%
%                                                                          %
%       oaa_app_idle()                  : to be executed if a timeout occurs       %
%                                                                          %
% Event requests that all agents can respond to:                           %
%                                                                          %
%       trace_on                : turns on event tracing                   %
%       trace_off               : turns off event tracing                  %
%       tcp_trace_on            : turns on tcp tracing                     %
%       tcp_trace_off           : turns off tcp tracing                    %
%       debug_on                : turns on event debugging                 %
%       debug_off               : turns off event debugging                %
%       halt                    : quit to UNIX                             %
%       solve(ID,Goal)          : tries to solve the goal, then returns an %
%                                 event solved(ID,FromKS, Goal, Solutions) %
%                                 to the caller.  ID is used to match the  %
%                                 calling goal to the solution.            %
%                                                                          %
% Events not yet implemented:                                              %
%                                                                          %
%       set_timeout(Int, Mode)  : reinitializes the timeout for an agent   %
%       read_trigger(G,T,C,A)   : returns each operating trigger           %
%                                                                          %
% Library routines provided to C agents:                                   %
%                                                                          %
%    Agent routines:                                                       %
%  post_event(Event)  : sends event to Server                              %
%  write_bb(Item, Data)  : writes global Item/Data pair on Server          %
%  retract_bb(Item, Data)  : removes globa Item/Data pair from Server      %
%  oaa_timer()    : X-Windows callback to read events                      %
%                                                                          %
%    Prolog parsing (read/write) functions:                                %
%  Functor()    : splits term into functor and args                        %
%  Term()      : removes first term from list of terms                     %
%  NthElt()    : returns Nth of list of terms                              %
%  Argument()    : returns Nth argument of a term                          %
%  ArgumentAsInt()    : returns Nth argument of a term as an int           %
%  is_atom()    : returns TRUE if argument is an atom                      %
%  is_list()    : returns TRUE if argument is a list                       %
%  is_var()    : returns TRUE if argument is a variable                    %
%  is_int()    : returns TRUE if argument is an int                        %
%  double_quotes()    : double all ' marks in string                       %
%  remove_quotes()    : removes leading and trailing ' and "               %
%  list_len()    : returns number of terms in list                         %
%  list_to_terms()    : converts prolog list to term list                  %
%  param_value()    : extract value from a parameter list                  %
%                                                                          %
%    Primitives not yet implemented:                                       %
%       solve(Goal, Params, Ans): requests information from other agents   %
%                                                                          %
%                                                                          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include "agentlib.h"

#include <prio.h>
#include <prerror.h>

#ifdef IS_WINDOWS /* ---------------------------------------------------- */
 
#ifdef IS_BORLAND
#define EXPORT_BORLAND _export
#define EXPORT_MSCPP
#endif
 
#ifdef IS_MSWCPP
#define EXPORT_BORLAND 

#ifdef IS_DLL
#define EXPORT_MSCPP __declspec(dllexport)  
#else
#define EXPORT_MSCPP
#endif

#endif /* --- IS_MSWCPP -- */

/* need to define these to something! */
#if !defined(IS_BORLAND) && !defined(IS_MSWCPP)
#define EXPORT_MSCPP
#define EXPORT_BORLAND
#endif

/* #define EXPORT_MSCPP EXPORT */

#else /* --- IS_WINDOWS -- */

/* If not IS_WINDOWS, define IS_UNIX by default */
#ifndef IS_UNIX
#define IS_UNIX
#define EXPORT_BORLAND
#define EXPORT_MSCPP
#endif
 
#endif            /* ---------------------------------------------------- */
 

#include <stdlib.h>  /* used by expandFileName */
#include <stdio.h>
#include <string.h>

#ifdef IS_WINDOWS
#define strcasecmp stricmp  /* SG */
#endif

#ifdef IS_UNIX    /* ---------------------------------------------------- */
 
#include <pwd.h>        /* used by expandFileName */
#include <sys/param.h>  /* used by getlocalhostname */
#include <sys/uio.h>  /* used by read */
#include <netdb.h>  /* for MAXHOSTNAMELEN */
#include <ctype.h>  /* islower */
#include <errno.h>

#ifndef min
#define min(x,y) ((x)<(y) ? (x) : (y))
#endif

#define EXPORT_BORLAND
#define EXPORT_MSCPP
 
#endif            /* ---------------------------------------------------- */
 
#ifndef IS_DLL
# undef EXTERN
# define EXTERN
#endif

#ifdef XWINDOWS  /* ------------------------------------------------------- */

#include <X11/Intrinsic.h> 

#endif    /* ------------------------------------------------------- */


#ifndef lint
  static char *rcsid= "$Header: /homedir/magic/software/CVS/TerraVision/TerraVision/agentlib.c,v 2.16.2.2 2001/12/18 20:53:16 reddy Exp $";
#endif

#define FALSE 0
#define TRUE  1

#define SETUP_FILE_NAME "setup.pl"
#define TIMEOUT_EVENT "timeout"
#define DELAY_PRED "delayed_event"

/****************************************************************************
 * Registration variables
 ****************************************************************************/
int (*user_do_event)()=NULL;
void (*user_app_done)()=NULL;
void (*user_app_idle)()=NULL;
void (*user_app_ready)()=NULL;
void (*user_app_disconnected)()=NULL;

/****************************************************************************
 * Private variables
 ****************************************************************************/
static int     GoalCounter = 0;
int            TcpTraceOn;
int            TraceOn;
int            DebugOn;
int            IsConnected = FALSE;

/* Table of delayed events */
#define DELAY_TABLE_SIZE 64
char    *delay_table[DELAY_TABLE_SIZE];
char    *delayed_context_table[DELAY_TABLE_SIZE];

/* Stack for contexts */
#define CONTEXT_STACK_SIZE 64
char *context_stack[CONTEXT_STACK_SIZE];
int  context_stack_p = 0;

/* Name of agent, set during setup_communication */
EXTERN char    KSname[KS_SIZE] = "";
static char    *ksid=NULL;

/* If TimeOutMode = 0, ignores TimeOut value */
/* Otherwise uses timeout value */
static int     TimeOutMode = 0;  
static double  TimeOut = 0;   


#if defined (IS_UNIX) || defined (IS_CONSOLE)
#ifdef XWINDOWS  /* ------------------------------------------------------- */

static          XtAppContext app_context;

/* Globals used while waiting for an event */
static char    *polling_for_event = NULL;
static char    *poll_event = NULL;
static char    poll_ks[KS_SIZE];

static XtInputId oaa_timer_id = 0;  /* Installed input timer */


#endif
#endif    /* ------------------------------------------------------- */


#ifdef IS_WINDOWS  /* ---------------------------------------------------- */
 
/*****************************************************************************
* Windows specific functions & Variables
*****************************************************************************/
 
/* Globals used while waiting for an event */
 
static int     polling_for_event = FALSE;
static char    *poll_event = NULL;
static char    poll_ks[KS_SIZE];
 
char *global_solvable=NULL;
char *global_ks=NULL;
char *global_tmpstr=NULL;
 
int  global_server_type;
 
/* Pops up a text window, useful to debug */
char debugString[255];
void win_printf(char *string_to_display)
{
  MessageBox(NULL, string_to_display, "MP_Agentlib Dll", MB_OK);
}
 
/* Test function */

void TestDll()
{
#if defined (IS_CONSOLE)
  printf("Test Console (22 Dec 1998) MP_Agentlib Dll\n");
#else
  MessageBox(NULL, "Test Console (22 Dec 1998)", "MP_Agentlib Dll", MB_OK);
#endif
}
#endif /* ---------------------------------------------------------------- */
 

/* Stores file descriptor of TCP connection */
/* If -1 (UNIX) or INVALID_SOCKET (WINDOWS) , no connection established    */

EXTERN OAASOCKET Connection = INVALID_OAASOCKET;
 
/*****************************************************************************
* registration functions
*****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND oaa_register(int function_id, int (*user_function)())
{
  switch (function_id)
   {
    case OAA_DO_EVENT :
      user_do_event=user_function;
      break;
    case OAA_APP_DONE :
      user_app_done=(void (*)()) user_function;
      break;
    case OAA_APP_IDLE :
      user_app_idle=(void (*)()) user_function;
      break;
    case OAA_READY :
      user_app_ready=(void (*)()) user_function;
      break;
    case OAA_DISCONNECTED :
      user_app_disconnected=(void (*)()) user_function;
      break;
    default :
      return FALSE;
    }
  return TRUE;
}

int oaa_do_event(char *KS, char *func, char *args, char **answers)
{
  if (user_do_event)
    return(user_do_event(KS, func, args, answers));
  else
    return FALSE;
}

void oaa_app_idle()
{
  if (user_app_idle)
    user_app_idle();
}

void oaa_app_done()
{
  if (user_app_done)
    user_app_done();
}

/*****************************************************************************
* Initialization and connection functions
*****************************************************************************/

/****************************************************************************
 * name:    init_vars
 * purpose: Initialize global variables
 ****************************************************************************/
void
init_vars() {

  TcpTraceOn = FALSE;
  TraceOn = FALSE;
  DebugOn = FALSE;
  GoalCounter = 0;
  clean_delay_table();
}


/****************************************************************************
 * name:    set_timeout
 * purpose: allow an agent to dynamically change its timeout variables
 * inputs:
 *   - double TimeOut: Timeout value in seconds
 *   - int       Mode: If mode = 0, TimeOut is not used 
 *           Otherwise, TimeOut is used.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND set_timeout(double T,int TMode)
{
  TimeOut = T;
  TimeOutMode = TMode;
}


/****************************************************************************
 * name:    read_setup_file
 * purpose: Finds a setup file and reads the OAA host and port values
 * outputs:
 *   - char *Host: Host for the server process
 *   - int  *Port: Port for the server process
 * remarks:
 *   Host and/or Port are only set if they do not already contain a legal
 *   value.  This way, the values can be set first through command line args
 *   or environment variables (see read_env_vars).
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND read_setup_file(char **Host, int *Port)
{
  FILE *setup_file;
  char *PortStr = NULL;
  char *HostStr = NULL;
  char buf[500];
   
#ifdef IS_UNIX /* --------------------------------------------------------- */
  strcpy(buf, "./");
  strcat(buf, SETUP_FILE_NAME);
#endif /* ----------------------------------------------------------------- */
#ifdef IS_WINDOWS  /* ----------------------------------------------------- */
  strcpy(buf, SETUP_FILE_NAME);
#endif  /* ---------------------------------------------------------------- */
  if ((setup_file = fopen(buf, "r")) == NULL) {
#ifdef IS_WINDOWS  /* ----------------------------------------------------- */
    strcpy(buf,"C:\\");
    strcat(buf, SETUP_FILE_NAME);
#endif  /* ---------------------------------------------------------------- */
#ifdef IS_UNIX  /* -------------------------------------------------------- */
    strcpy(buf, "~/");
    strcat(buf, SETUP_FILE_NAME);
    strcpy(buf, expand_filename(buf, NULL));
#endif  /* ---------------------------------------------------------------- */
    setup_file = fopen(buf, "r");
  }

  if (setup_file != NULL) {
    while (fgets(buf, BUFSIZ, setup_file) != NULL) {
      if (strstr(buf, "rootdata") == buf) {
        Argument(buf, 1, &PortStr);
        Argument(buf, 2, &HostStr);
        remove_quotes(PortStr);
        if (*Port < 1) 
          *Port = atoi(PortStr);
        remove_quotes(HostStr);
        if (!*Host)
          *Host = strdup(HostStr);
        OAA_FREE(PortStr);
        OAA_FREE(HostStr);
      }
    }
    fclose(setup_file);
  }
}



/****************************************************************************
 * name:    read_env_vars
 * purpose: Initializes oaa variables from environment variables, if declared
 * outputs:
 *   - char *Host: Host for the server process
 *   - int  *Port: Port for the server process
 *   - int  *ServerType: 0 for qp3.1.4, 1 for qp3.2
 ****************************************************************************/
void read_env_vars(char **Host,int *Port, int *ServerType)
{
   char *val;

   val = getenv ("OAA_HOST");
   if (val && !*Host)
      *Host = strdup(val);

   val = getenv ("OAA_PORT");
   if (val && (*Port < 1))
      *Port = atoi(val);

   val = getenv ("OAA_SERVER_TYPE");
   if (val && (*ServerType < 0)) {
      if (strcmp(val, "qp3.1.4") == 0)
        *ServerType = 0;
      else
      if (strcmp(val, "qp3.2") == 0)
        *ServerType = 1;
      else printf("Error: unknown server type '%s'.\n", val);
   }
}


/****************************************************************************
 * name:    get_agent_id
 * purpose: Returns local agent's id and name in newly created strings
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND get_agent_id(char **ask_ksid, char **ask_ksname)
{
  if (ask_ksid && ksid)
      *ask_ksid = strdup(ksid);

   if (ask_ksname && KSname)
      *ask_ksname = strdup(KSname);
}

/****************************************************************************
 * name:    setup_communication
 * purpose: Establishes tcp connections to the Server
 * inputs:
 *   - char       *KS: Name for the KS
 *   - char     *host: host to connect to
 *   - int       port: port to connect to
 *   - char *Solvable: Prolog-formatted list of solvables provided by the KS
 *   - int ServerQP32: True if connecting to a server written in QP v3.2
 * return value: TRUE if connection made; FALSE otherwise
 * remarks: 
 *   If Host is null or empty, reads the host and port from the SETUP_FILE
 ****************************************************************************/

#ifndef IS_CONSOLE /* ------------------------------------------------------*/
#ifdef IS_WINDOWS /* ------------------------------------------------------ */

EXPORT_MSCPP
int EXPORT_BORLAND setup_communication(HWND main_window,
                         char *KS,
                         char *Host,
                         int Port,
                         char *Solvable,
                         int ServerQP32)
{

  /* DG
  char *tmpstr;
  char *kshost;
  */
  
  if ((Host==NULL) || (Port <= 0)) {
    error_out("setup_communication\nWrong facilitator address.\nCheck setup.pl, environment vars or command line.");
    return FALSE;
  }

  tcpStartup(main_window);
  /* Initialize OAA global variables */
  init_vars();
 
  /* If KSname not yet set by read_cmd_args() command line override */
  if (strcmp(KSname, "") == 0)
    strcpy(KSname, KS);
 
  global_solvable = malloc(strlen(Solvable)+strlen(KS)+35);
  global_tmpstr = malloc(strlen(Solvable)+strlen(KS)+35);
  global_ks = malloc(strlen(KS)+5);
  Connection = tcpConnect(Host, Port);
  if (Connection == INVALID_OAASOCKET) {
    error_out("Error by tcp_connect");
    return FALSE;
  }
 
  /* Initialize global values */
  strcpy(global_solvable,Solvable);
  strcpy(global_ks,KS);
  global_server_type=ServerQP32;
  return TRUE;
}
#endif /* ----------------------------------------------------------- */
#endif /* NOT IS_CONSOLE */
  
#ifndef IS_UNIX
#define MAXHOSTNAMELEN 255
#endif

char *getlocalhostname(void) {
  char buf[MAXHOSTNAMELEN + 1];
 
  if (gethostname(buf, MAXHOSTNAMELEN) == 0) { /* success! */
    return(strdup(buf));
  }
  else return(NULL); 
}

#if defined (IS_UNIX) || defined (IS_CONSOLE)  /* ------------------- */

int
setup_communication(char *KS,
                    char *Host,
                    int Port,
                    char *Solvable,
                    int ServerQP32)
{
    char *tmpstr;
    char *kshost;
    char *kshost2;

    if ((Host==NULL) || (Port <= 0)) {
      error_out("setup_communication\nWrong facilitator address.\nCheck setup.pl, environment vars or command line.");
      return FALSE;
    }

    /* Initialize OAA global variables */
    init_vars();

    /* If KSname not yet set by read_cmd_args() command line override */
    if (strcmp(KSname, "") == 0)
       strcpy(KSname, KS);

    tmpstr = malloc(strlen(Solvable)+strlen(KS)+35);

    Connection = tcpConnect(Host, Port);
    if (Connection == INVALID_OAASOCKET) {
      error_out("Error by tcp_connect");
      return FALSE;
    }
    IsConnected = TRUE;
    /* Send KS name, if using a server written in QP3.1.4 */
    if (!ServerQP32) {
       sprintf(tmpstr, "'%s'", KSname);
       send_data(tmpstr);
    }

    /* Records the hostname of the agent */
    kshost2 = getlocalhostname();
    if (kshost2) {
       kshost = (char *) malloc(strlen(kshost2) + 3);
       sprintf(kshost, "'%s'", kshost2);
       write_bb("kshost", kshost);
       OAA_FREE(kshost);
       OAA_FREE(kshost2);
    }

    /* Write language immediately so any communication from server
       will be in correct format */
    write_bb("language", "c");

    /* Version 2.0+ correctly receive sending KS address in events */
    write_bb("oaa_version", "2.0");

    /* Send list of solvable predicates */
    if (!Solvable || (strcmp(Solvable, "") == 0))
       sprintf(tmpstr, "register_solvable_goals([], '%s')", KS);
    else
       sprintf(tmpstr, "register_solvable_goals(%s,'%s')", Solvable, KS);
    post_event(tmpstr);

    OAA_FREE(tmpstr);
    return TRUE;
}

#endif


/****************************************************************************
 *    name: is_connected
 * purpose: Return the status of the facilitator's connection
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_connected() {
  return IsConnected;
}

 /****************************************************************************
 *    name: disconnect
 * purpose: Disconnects client from Facilitator
 ****************************************************************************/
#if defined (IS_WINDOWS) && !defined(IS_CONSOLE) /* ------------------------- */
 
EXPORT_MSCPP
void EXPORT_BORLAND disconnect()
{
  if (Connection != INVALID_OAASOCKET) {
   /* Close tcp connection */
   tcpShutdown(&Connection);
   Connection = INVALID_OAASOCKET;
   IsConnected = FALSE;
  }
}
#endif  /* ---------------------------------------------------------------- */

#if defined (IS_UNIX) || defined (IS_CONSOLE) /* -------------------------- */
void
disconnect()
{
  if (Connection != NULL) {
#ifdef XWINDOWS   
    /* If input timer still installed, remove it from X */
    if (oaa_timer_id)
        XtRemoveInput(oaa_timer_id);
    oaa_timer_id = 0;
#endif
    IsConnected = FALSE;
    /* Close tcp connection */
    tcpShutdown(&Connection);
    Connection = INVALID_OAASOCKET;

    if (user_app_disconnected)
      user_app_disconnected();
  }
}


#endif

/*****************************************************************************
* Functions for dynamic strings, stringlists, tables, etc.
*****************************************************************************/

/****************************************************************************
 * name:    append
 * purpose: Appends a string onto the end of a first one, adjusting the
 *     size of the resulting string if necessary.
 * inputs:
 *   - char **str1: Address of a string pointer, maybe containing data
 *   - char  *str2: A string to concatenate to str1
 * remarks:
 *   - The memory allocation of str1 will be readjusted to be able to contain
 *     text for both str1 and str2.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND append(char **str1, char *str2)
{
   char *p;

   if (*str1 == NULL) {
      *str1 = strdup(str2);
   }
   else {
      p = malloc(strlen(*str1) + strlen(str2)+1);
      strcpy(p, *str1);
      strcat(p, str2);
      OAA_FREE(*str1);
      *str1 = p;
   }
}


/****************************************************************************
 * name:    init_dyn_array
 * purpose: Initializes dynamic array
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND init_dyn_array(struct dyn_array *da)
{
   (*da).allocated = 0;
   (*da).count = 0;
   (*da).item = NULL;
}



/****************************************************************************
 * name:    member_strlist
 * purpose: Finds an element in a string list
 * inputs:
 *   - char *elt: The element to look for
 *   - char *slist[]: The list to look through
 *   - int  incr: Increment if not found
 *   - int  case_ins: if true, case insensitive comparison
 * returns:
 *   Position in list if found, -1 if not found
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND member_strlist(char *elt,
                         struct dyn_array strlist,
                   int incr,
                   int case_ins)
{
   int p = 0, r;

   for (p = 0; p < strlist.count; p += incr) {
      if (case_ins) 
   /* r = strcasecmp(elt,strlist.item[p]); */
    r = strcmp(elt,strlist.item[p]);
      else
         r = strcmp(elt,strlist.item[p]);
      if (r == 0)
         return p;  /* found! */
   }
   return -1;
}


/****************************************************************************
 * name:    append_strlist
 * purpose: Adds a string to the end of a strlist
 * inputs:
 *   - char *slist[]: the list
 *   - char *str: the string to add
 * remarks:
 *   Uses realloc to increase list size if necessary
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND append_strlist(struct dyn_array *slist,
                    char *str)
{
   /* Size by which to increment buffer if overflow */
   int INC_SIZE = 100;

   if ((*slist).count == (*slist).allocated) {
      if ((*slist).allocated == 0)
   (*slist).item = malloc(INC_SIZE * sizeof(str));
      else
         (*slist).item = realloc((*slist).item, 
      ((*slist).count + INC_SIZE) * sizeof(str));
      (*slist).allocated += INC_SIZE;
   }
   (*slist).item[(*slist).count] = strdup(str);
   (*slist).count++;
}



/****************************************************************************
 * name:    free_strlist
 * purpose: Frees all strings and space used by a string list
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND free_strlist(struct dyn_array *slist)
{
   int i;

   for (i = 0; i < (*slist).count; i++)
      free((*slist).item[i]);
   free((*slist).item);
   slist = NULL;
}


/*****************************************************************************
* Functions to manage contexts
*****************************************************************************/
 
/****************************************************************************
 * name:    push_context_stack
 * purpose: Add a list of contexts to context_stack
 ****************************************************************************/
void push_context_stack(char *ctx)
{
   if (context_stack_p < CONTEXT_STACK_SIZE) {
       context_stack[context_stack_p] = ctx;
       context_stack_p++;
   }
}

/****************************************************************************
 * name:    pop_context_stack
 * purpose: Pops the context stack
 ****************************************************************************/
void pop_context_stack()
{
   if (context_stack_p > 0) {
       context_stack_p--;
       OAA_FREE(context_stack[context_stack_p]);
   }
}

/****************************************************************************
 * name:    pop_context_stack
 * purpose: Pops the context stack
 ****************************************************************************/
void print_context_stack()
{
   int i;

   printf("CONTEXT_STACK: %d elements\n", context_stack_p);
   for (i = 0; i < context_stack_p; i++) 
       printf("%d] %s\n", i, context_stack[i]);
}


/****************************************************************************
 * name:    oaa_extract_contexts
 * purpose: extract any context parameters
 * returns: comma separated list of context parameters in new space
 ****************************************************************************/
char *oaa_extract_contexts(char *params)
{
   char *params2; 
   char *p;
   char *res = NULL;

   if (!params) return NULL;

   params2 = strdup(params);
   p = params2;

   list_to_terms(params2);
   while (p && *p) {
      char *elt;
      Term(p, &elt, &p);
      if (strstr(elt, "context(") == elt) {
   if (res)
      append(&res,",");
   append(&res, elt);
      }
      OAA_FREE(elt);
   }
   OAA_FREE(params2);

   return(res);
}


/*****************************************************************************
* Functions to manage delayed solve events
*****************************************************************************/
 
/****************************************************************************
 * name:    free_delay_table_entry
 * purpose: free's entry for given ID
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND free_delay_table_entry(int table_entry)
{
   if ((table_entry >= 0) && (table_entry < DELAY_TABLE_SIZE)) {
       OAA_FREE(delay_table[table_entry]);
       OAA_FREE(delayed_context_table[table_entry]);
   }
}


 
/****************************************************************************
 * name:    clean_delay_table
 * purpose: Cleans the delayed events table
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND clean_delay_table()
{
   int i;
   for (i = 0; i < DELAY_TABLE_SIZE; i++) {
       OAA_FREE(delay_table[i]);
       OAA_FREE(delayed_context_table[i]);
   }
}
 

 
/****************************************************************************
 * name:    find_free_delay_table_entry
 * purpose: Finds a free entry in the delayed events table.
 * returns:
 *   - int       Free entry found
 * remarks:
 *   If the table is full, returns -1
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND find_free_delay_table_entry()
{
   int i;
   for (i = 0; i < DELAY_TABLE_SIZE; i++)
     if (delay_table[i] == NULL)
        return i;
   return -1;          /* Means that the table is full */
}
 
 
/****************************************************************************
 * name:    add_delay_table_entry
 * purpose: Writes string s into the delay table at position n
 * returns:
 *   - int       -1 if there's a problem; n otherwise
 * remarks:
 *   Prior to calling this fcn, input argument n should be obtained by calling
 * find_free_delay_table_entry().
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND add_delay_table_entry(int n, char *s)
{
    if ((n >= 0) && !delay_table[n] && s && *s) {
      delay_table[n] = strdup(s);

      if ((context_stack_p > 0) && context_stack[context_stack_p-1])
         delayed_context_table[n] = strdup(context_stack[context_stack_p-1]);
      else delayed_context_table[n] = NULL;
      return n;
    }
    else {
      printf("Error adding delayed predicate:\n  %s\n", s);
      print_delay_table();
      return -1;
    }
}


/****************************************************************************
 * name:    print_delay_table
 * purpose: Prints the delayed events table.
 * remarks:
 *   Useful for debugging
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND print_delay_table()
{
  int item_id;

  printf("Delayed events waiting to be solved : \n");
  for (item_id = 0; item_id < DELAY_TABLE_SIZE; item_id++)
     printf("  %d. %s)  ctx:[%s]\n", item_id, delay_table[item_id], 
             delayed_context_table[item_id]);
}
 
 
/****************************************************************************
 *    name: delay_solution
 * purpose: Defer solving a do_event request until a later time.
 *      This procedure should be called in do_event() when a programmer
 *      wants to return from do_event() without returning an immediate
 *      solution.  "answers" will be given an appropriate variable, and
 *      an ID is returned by this function, which should be used to call
 *      return_delayed_solutions(Id,Solutions) when ready.
 * outputs: 
 *  char **answers: returns an appropriate result such that answers can
 *          be deferred until a later time.
 * returns:
 *   - int   An identifier which will be used to send the solutions when ready
 * implementation:
 *     answers will contain "DELAY_PRED(i)": since usually answers must
 *     contain a list, the Interpret function will know that this is a
 *     special case, and the event to be solved can be added to the delay_table
 *     at index i.
 * remarks:
 *   Returns -1 if it is not possible to delay the solution (table full).
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND delay_solution(char **answers)
{
   int i;

   i = find_free_delay_table_entry();

   if (i >= 0) {
      *answers = malloc(strlen(DELAY_PRED) + 10);
      sprintf(*answers, "%s(%d)", DELAY_PRED, i);
   }
   return i;
}


/****************************************************************************
 *    name: delayed_event
 * purpose: Retrieves the requested delayed event from the delay_table[]
 * inputs: 
 *   - int    table_entry: 
 *              Index into the delay table, indicating which event to retrieve
 * returns:
 *   - char * a copy of the delayed event stored in the delay_table
 * remarks:
 *   The event returned is allocated in a new space which should be OAA_FREE'd
 *   when finished using.
 ****************************************************************************/
EXPORT_MSCPP
char *EXPORT_BORLAND delayed_event(int table_entry)
{
   if ((table_entry >= 0) && (table_entry < DELAY_TABLE_SIZE) &&
       delay_table[table_entry])
      return strdup(delay_table[table_entry]);
   else
      return NULL;
}



/****************************************************************************
 *    name: return_delayed_solutions
 * purpose: Sends the solutions to a delayed event to the Facilitator,
 *      and removes the event from the delay table.
 * inputs:
 *   - int    table_entry: 
 *              Index into the delay table, indicating which event is solved
 *   - char*  answer:
 *               The solution list for the delayed event
 * remarks:
 *   If answer=NULL, a failed event ([]) is sent to the Facilitator.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND return_delayed_solutions(int table_entry, char *answers)
{

  char *event = NULL;

  if ((table_entry >= 0) && (table_entry < DELAY_TABLE_SIZE) &&
       delay_table[table_entry])
    {
      /* delay table contains event to return, minus the answers */
      event = strdup(delay_table[table_entry]);

      /* add answers, or [] (failure) if none */
      if (answers)
         append(&event, answers);
      else append(&event, "[]");
      append(&event, ")");

      /* Send result to Facilitator */
      post_event(event);

      /* free storage */
      OAA_FREE(delay_table[table_entry]);
      OAA_FREE(event);
    }
  else 
     printf("\nError: delayed event #%d not found!\n", table_entry);
}


/****************************************************************************
 * name:    oaa_AddContextParams
 * purpose: Adds contexts to params, checking for duplicates.
 * remarks: DO NOT FREE the result: points to a static buffer.
 ****************************************************************************/
char *
oaa_AddContextParams(char *contexts, char *params)
{
   static char *out_str = 0;
   static int out_str_len = 0;
   int new_len;

/*   printf("add_context_params: %s >> %s\n", contexts, params); */

   if (params && *params && contexts && *contexts) {

       new_len = strlen(params) + 
     strlen(contexts) + 10;
 
       /* Reallocate space if necessary */
       if (out_str_len < new_len) {
         if (out_str) {
           OAA_FREE(out_str);
         }
         out_str = malloc(new_len);
         out_str_len = new_len;
       }

       if (*params == '[') {
          strcpy(out_str, params);
          out_str[strlen(out_str)-1] = 0;

    /* Add all elements in contexts not already in params */
    while (contexts && *contexts) {
       char *elt;

       Term(contexts, &elt, &contexts);
       if (strstr(out_str, elt) == NULL) {
    if (strcmp(out_str, "[") != 0)
       strcat(out_str, ",");
    strcat(out_str, elt);
    OAA_FREE(elt);
       }
    }

          strcat(out_str, "]");
       }

/*       printf("   results: %s\n", out_str); */

       return(out_str);
   }
   else return(params);
}


/****************************************************************************
 * name:    oaa_AddDelayedContextParams
 * purpose: Outgoing solves in the body of a delayed event should call this
 *          procedure to add any context params to the parameter list
 * remarks: DO NOT FREE the result: points to a static buffer.
 ****************************************************************************/
EXPORT_MSCPP
char * EXPORT_BORLAND 
oaa_AddDelayedContextParams(int table_entry, char *params)
{
   printf("AddDelayedContextParams: %d %s\n", table_entry, params);
   if ((table_entry >= 0) && (table_entry < DELAY_TABLE_SIZE) && 
  delayed_context_table[table_entry]) {

       return oaa_AddContextParams(delayed_context_table[table_entry], 
             params);
   }
   else return(params);
}


/****************************************************************************
 * name:    oaa_InCurrentContext
 * purpose: Returns true if a context string is in the given context.
 * inputs:  
 *    ctx : context string to check for
 *    table_entry: if >= 0, uses context stored for a delayed entry
 *          if -1, uses current context
 ****************************************************************************/
EXPORT_MSCPP
void  EXPORT_BORLAND 
oaa_GetCurrentContexts(char **ctx, int table_entry)
{
   *ctx = NULL;
   if ((table_entry >= 0) && (table_entry < DELAY_TABLE_SIZE)) {
      if (delayed_context_table[table_entry]) {
   *ctx = strdup(delayed_context_table[table_entry]);
      }
   }
   else
   if ((context_stack_p > 0) && (context_stack[context_stack_p-1]))
      *ctx = strdup(context_stack[context_stack_p-1]);
}


/****************************************************************************
 * name:    oaa_InCurrentContext
 * purpose: Returns true if a context string is in the given context.
 * inputs:  
 *    ctx : context string to check for
 *    table_entry: if >= 0, uses context stored for a delayed entry
 *          if -1, uses current context
 ****************************************************************************/
EXPORT_MSCPP
int  EXPORT_BORLAND 
oaa_InContext(char *ctx, int table_entry)
{
   int res = FALSE;
   char *p;
   char *p2 = NULL;

   oaa_GetCurrentContexts(&p2, table_entry);
   if (p2) {
      p = p2;
      while (p && *p && !res) {
   char *elt, *func, *args;
   Term(p, &elt, &p);
   Functor(elt, &func, &args);
   trim(args);
   res = (strcmp(args, ctx) == 0);
   OAA_FREE(elt);
   OAA_FREE(func);
   OAA_FREE(args);
      }
      OAA_FREE(p2);
   }

   return res;

}


/*****************************************************************************
* Prolog parsing predicates
*****************************************************************************/
 
/****************************************************************************
 * name:    Functor
 * purpose: Splits a string containing a prolog-style term into a functor 
 *          and arguments
 * inputs:
 *   - char *predicate: In form  "a(arg1, arg2, ...)"
 * outputs:
 *   - char *func: returns "a"
 *   - char *args: returns "arg1, arg2, ..."
 * remarks:
 *   - func and args will return new copies of the data, which should
 *     be free'd once they are no longer needed
 *   - If no arguments exist, the empty string is returned in args
 *     (1 byte allocated)
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND Functor(char *predicate,
             char **func,
             char **args)
{
   char *p;
   int i;

   /* Make sure predicate is not NULL! */
   if (!predicate)
      predicate = strdup("");

   /* If predicate is just a string, list or other compound term, */
   /* or an atom, just return the term, don't search for "("             */
   if (((*predicate) && (strchr("{[(\"", *predicate) != NULL))
       || is_atom(predicate)) {
      Term(predicate, func, args);
      *args = strdup("");
   }

   /* Otherwise, assume predicate in form aaa(bbb,ccc): pluck of */
   /* functor by searching for first "(" char.       */
   else {

      p = strstr(predicate, "(");
      if (!p) {        /* no arguments in term */
	*func = strdup(predicate);
	/* Chop off trailing '.' */
	i = strlen(*func) - 1;
	if ((i > 0) && (*func)[i] == '.')
	  (*func)[i] = '\0';
	*args = strdup("");
      }
      else {
	*func = malloc(p - predicate + 1);
	strncpy(*func, predicate, p - predicate);
	(*func)[p - predicate] = '\0';
	*args = strdup(p + 1);

	/* Chop off everything up to trailing ")"  */
	i = strlen(*args) - 1;
	while ((i >= 0) && ((*args)[i] != ')')) {
	  (*args)[i] = '\0';
	  i--;
	}
	if (i > 0)
	  (*args)[i] = '\0';
      }
   }
}


/****************************************************************************
 * name:    Term
 * purpose: takes a list of terms (comma separated) and returns the first (car)
 *          and rest (cdr) of the list.
 * inputs:
 *   - char *terms: a list of prolog terms
 * outputs:
 *   - char     *aterm: the first term in the list
 *   - char *restterms: the rest of the terms
 * remarks:
 *   - Terms may be nested arbitrarily deep (eg. a(b(c(d,1)), X).
 *   - aterm will return a new copy of the first argument, which should
 *     be free'd when finished using.  restterms will return a pointer
 *     into terms.
 *   - Term can correctly parse expression with embedded arguments using
 *     Japanese JIS7 format.  JIS7 is the Japanese encoding standard chosen
 *     for use with the OAA.  Three reasons for this choice:  1) it's the
 *     most popular standard on UNIX machines.  2) 7-bit encoding should
 *     be TCP/IP and modem friendly.  3) it's easy to parse: JIS7 embeds
 *     Japanese characters in a quotation-like wrapper, using ESC$B for the
 *     opening marker and ESC(B for the closing.
 * returns: 
 *     TRUE if a term was read, FALSE if a complete term could not be read.
 *     In the latter case, aterm and restterms returns "" as well.
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND Term(char *terms,
         char **aterm,
         char **restterms)
{
   int i;
   int n, len;
   int done = FALSE;
   int inquotes = FALSE;
   int inJIS7 = FALSE;
   int parens = 0;
   int seen_something = FALSE;

   *aterm = NULL;
   *restterms = NULL;

   len = strlen(terms);

   i = -1;
   while ((i < len-1) && !done) {
      i++;

      /* Check for JIS7 Japanese Encoding markers */
      if ((terms[i] == 27) && (terms[i+1] == '$') && (terms[i+2] == 'B'))
	inJIS7 = TRUE;
      else
	if ((terms[i] == 27) && (terms[i+1] == '(') && (terms[i+2] == 'B'))
	  inJIS7 = FALSE;
      
      /* If we are not looking at Japanese characters, try to parse the 
	 input. */
      if (!inJIS7)
	switch (terms[i]) {
	case '(':
	case '[':
	case '{':
	  if (!inquotes)
            parens = parens + 1;
	  break;
	case ')':
	case ']':
	case '}':
	  if (!inquotes) {
            parens = parens - 1;
            if (parens == 0)
	      done = TRUE;
	  }
	  break;
	case '\'':
	  inquotes = !inquotes;
	  break;
	case ',' :
	  if ((parens == 0) && (inquotes == 0))
            done = TRUE;
	  break;
	case ' ':
	case '\n':
	case '\t':
	  if ((parens == 0) && (inquotes == 0) && seen_something)
            done = TRUE;
	  break;
	default: seen_something = TRUE;
	}
   }
   if ((i >= 0) && !parens && !inquotes) {
     if (terms[i] == ',')
       n = i;
     else n = i+1;
     *aterm = malloc(n+1);
     strncpy(*aterm, terms, n);
     (*aterm)[n] = '\0';
     i++;
     while ((terms[i] != 0) && ((terms[i] == ' ') || (terms[i] == ',')))
       i++;
     *restterms = &terms[i];
      return TRUE;
   }
   else {
     *aterm = strdup("");
     *restterms = strdup("");
     return FALSE;
   } 
}


/****************************************************************************
 * name:    NthElt
 * purpose: Returns the Nth term in a list of terms
 * inputs:
 *   - char *list: a string containing a comma-separated list of terms
 *   - int      n: index into a list (nth term)
 * outputs:
 *   - char  *elt: the nth term in the list.
 * remarks:
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND NthElt(char *list,
            int n,
            char **elt)
{
   /* Local string variables */
   char *temp = NULL;

   while ( (strcmp(list, "") != 0) && (n > 0)) {
      n--;
      if (n == 0)
         Term(list, elt, &list);
      else {
	Term(list, &temp, &list);
	/* Deallocation local string variables */
	OAA_FREE(temp);
      }
   }
   if (*elt) trim(*elt);
}


/****************************************************************************
 * name:    Argument
 * purpose: Returns the nth argument in a term
 * inputs:
 *   - char *predicate: a prolog-style term:  eg   func(a1,a2,...)
 *   - int      n: index into a list (nth term)
 * outputs:
 *   - char  *argument: the nth argument in the term. eg (a2 if n = 2
 * remarks:
 *   argument will return a new copy of the argument, which should be
 *   be free'd when finished using.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND Argument(char *predicate,
              int n,
              char **argument)
{
   /* Local string vars */
   char *functor = NULL;
   char *args = NULL;

   Functor(predicate, &functor, &args);
   NthElt(args, n, argument);

   /* Free local string vars */
   OAA_FREE(functor);
   OAA_FREE(args);
}

/****************************************************************************
 * name:    ArgumentAsInt
 * purpose: Returns the nth argument in a term as integer
 * inputs:
 *   - char *predicate: a prolog-style term:  eg   func(a1,a2,...)
 *   - int      n: index into a list (nth term)
 * returns:
 *   - int   the nth argument in the term as an integer
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND ArgumentAsInt(char *predicate,
                  int n)
{
   char *elt = NULL;

   Argument(predicate, n, &elt);
   n = atoi(elt);
   OAA_FREE(elt);
   return n;
}


/****************************************************************************
 * name:    OaaFloat
 * purpose: Converts a prolog-style float in 3.25E+3 format to a float
 ****************************************************************************/
EXPORT_MSCPP
float EXPORT_BORLAND OaaFloat(char *arg)
{
   float f, pow, tens;
   int e;
   char *p;
 
   if ((p = strstr(arg, "E")) != NULL) {
      *p = '\0';
      if (*(p+1) == '+')
   tens = 10.0;
      else tens = (float)(0.1);
      e = atoi(p + 2);
   }
   else {
      e = 0;
      tens = 1.0;
   }
   sscanf(arg, "%f", &f);
   pow = 1.0;
   while (e > 0) {
     pow = pow * tens;
     e = e - 1;
   }
   f = f * pow;
   return f;
}


/****************************************************************************
 * name:    is_atom
 * purpose: Returns true if the term is an atom
 * inputs:
 *   - char *s: a string containing a prolog term
 * returns:
 *   True if the term is an atom
 * Note: this assumes no leading or trailing whitespace in s.
 * This is defined in accordance with QP 3.1 manual, section G.1-1-3.
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_atom(char *s)
{
   char *p;
   int len;
  
   if ((!s) || (!*s))
      return FALSE;

   /* 4 special cases */
   if (strcmp(s, "!") == 0 || strcmp(s, ";") == 0)
      return TRUE;
   if (strcmp(s, "[]") == 0 || strcmp(s, "{}") == 0)
      return TRUE;

   /* Any sequence of chars delimited by single quotes */
   len = strlen(s);
   if ((*s == '\'') && (s[len-1] == '\''))
      return TRUE;

   /* Any sequence of these chars: +-*^<>=`-:.?@#$&/\ */
   if (strchr("+-*/\\^<>=`-:.?@#$&", *s) != NULL) {
      for (p = s + 1 ; *p ; p++) {
         if (strchr("+-*/\\^<>=`-:.?@#$&", *p) == NULL)
            return FALSE;
      }
      return TRUE;
   }

   /* Any sequence of alphanumeric or '_' starting with a lowercase letter */
   if (islower(*s)) {
      for (p = s + 1 ; *p ; p++) {
         if ((isalnum(*p) == FALSE) && (*p != '_'))
            return FALSE;
      }
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************
 * name:    is_group
 * purpose: Returns true if the term is a group (list of terms delimited by
 *      [], {} or ().
 * inputs:
 *   - char *s: a string containing a prolog term
 * returns:
 *   True if the term is a group
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_group(char *s)
{
   int result;

   result =  (s && (
    ((*s == '[') && (s[strlen(s)-1] == ']')) ||
    ((*s == '{') && (s[strlen(s)-1] == '}')) ||
    ((*s == '(') && (s[strlen(s)-1] == ')'))));

   return result;
}


/****************************************************************************
 * name:    is_list
 * purpose: Returns true if the term is a list (in form [1,2,3])
 * inputs:
 *   - char *s: a string containing a prolog term
 * returns:
 *   True if the term is a list
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_list(char *s)
{
   int result;

   result =  (s && (*s == '[') && (s[strlen(s)-1] == ']'));

   return result;
}


/****************************************************************************
 * name:    is_var
 * purpose: Returns true if the term is a variable
 * inputs:
 *   - char *term: a string containing a prolog term (or variable)
 * returns:
 *   True if the term is a variable
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_var(char *t)
{
   return ((*t == '_') || ((*t >= 'A') && (*t <= 'Z')));
}


/****************************************************************************
 * name:    is_int
 * purpose: Returns true if the term is an integer
 * inputs:
 *   - char *term: a string containing an integer
 * returns:
 *   True if the term is a integer
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND is_int(char *t)
{
   char *p = t;
   while (t && *t) 
      if ((*t == ' ') ||
    ((*t >= '0') && (*t <= '9')))
        t++;
      else return FALSE;
   return ((p != t) && (p != NULL));
}


/****************************************************************************
 * name:    param_value
 * purpose: Extracts the value from a parameter list
 *  inputs:
 *   - char *aList: a parameter list, in the form "[attr1(Value),attr2(Value2)]"
 *   - char *param: parameter to extract (e.g. attr1)
 * outputs:
 *   - char **Value: value of parameter or NULL if not found
 *     If Value is NULL, no attempt to fill it will be made (Used to
 *     check if a param is present)
 *   - int : 1 if parameter found, 0 if not found.
 *           
 * remarks:
 *   - value will return a new copy of the argument, which should be
 *     be free'd when finished using.
 *   - most parameter lists should be in the form [p(Val), p2(Val2)]...
 *     If p has more than one argument, the list is returned
 *     If p has no arguments, "true" is returned as the value
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND param_value(char const *aList,
                  char *param,
                  char **value)
{
   char *p;
   char *tmp = NULL;
   char *func = NULL;
   char *args = NULL;
   char *elt = NULL;

   *value = NULL;
   tmp = strdup(aList);
   p = tmp;
   list_to_terms(tmp);

   while (*tmp && !(*value)) {
      Term(tmp, &elt, &tmp);
      Functor(elt,&func,&args);
      if (strcmp(func, param) == 0) {
        if (value) {
          if (args && *args)
            *value = strdup(args);
          else *value = strdup("true");
        }
        OAA_FREE(elt);
        OAA_FREE(args);
        OAA_FREE(func);
        OAA_FREE(p);
        return 1;
      }
      OAA_FREE(elt);
      OAA_FREE(args);
      OAA_FREE(func);
   }
   OAA_FREE(p);
   return 0;
}


/*****************************************************************************
* Special string functions
*****************************************************************************/

/****************************************************************************
 * name:    double_quotes
 * purpose: Doubles any "'" in the string, to prepare the string to be
 *      surrounded by "'".
 * inputs:
 *   - char *s: a string containing prolog terms
 * returns: the string with any "'" --> "''"
 * remarks: 
 *   - The pointer returned should NOT be deallocated or the contents changed
 *     in any way, just used as is.
 *   - The original parameter string is NOT changed.
 ****************************************************************************/
EXPORT_MSCPP
const char * EXPORT_BORLAND double_quotes(char *s)
{
  static char *out_str = 0;
  static int out_str_len = 0;
  char *in_ptr, *out_ptr;
  char c;
  int i = 0;
  int num = 0;
  int new_len;
 
  /* Count number of ' chars */
  while (i < (int)strlen(s)) {
    if (s[i] == '\'') {
      num++;
    }
    i++;
  }
 
  new_len = strlen(s) + num + 10;
 
  /* Reallocate space if necessary */
  if (out_str_len < new_len) {
    if (out_str) {
      OAA_FREE(out_str);
    }
    out_str = malloc(new_len);
    out_str_len = new_len;
  }
 
  /* Copy string to new buffer, doubling ' char */
  in_ptr = s;
  out_ptr = out_str;
  while ((c = *in_ptr++) != 0) {
    if (c == '\'') {
      *out_ptr++ = c;
      *out_ptr++ = c;
    } else {
      *out_ptr++ = c;
    }
  }
  *out_ptr++ = (char) 0;
 
  return out_str;

}


/****************************************************************************
 * name:    undouble_quotes
 * purpose: Converts any '' marks inside a string to just '
 *          Single quotes inside strings will be doubled when coming from Prolog
 * inputs:
 *   - char *s: a string containing prolog terms
 * outputs:
 *   - char *s: the string with all '' --> '
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND undouble_quotes(char *s)
{
  char c, cc, *in_ptr, *out_ptr;
 
  in_ptr = s;
  out_ptr = s;
 
  while ((c = *in_ptr++) != (char) NULL) {
    cc = *in_ptr;
    if (c == '\'' && cc == c) {
      *out_ptr++ = c;
      in_ptr++;
    } else {
      *out_ptr++ = c;
    }
  }
  *out_ptr++ = (char) 0;
}




/****************************************************************************
 * name:    remove_quotes
 * purpose: Removes leading and trailing ' and " marks
 * inputs:
 *   - char *s: a string containing prolog terms
 * outputs:
 *   - char *s: the string minus leading and trailing quotes
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND remove_quotes(char *s)
{
   if ((*s == '\'') || (*s == '"'))
      strcpy(s, &s[1]);
   if ((s[strlen(s)-1] == '\'') || (s[strlen(s)-1] == '"'))
      s[strlen(s)-1] = '\0';
}



/****************************************************************************
 * name:    trim
 * purpose: Removes leading and trailing spaces and other unprintables
 * inputs:
 *   - char *s: a string containing prolog terms
 * outputs:
 *   - char *s: the string minus leading and trailing spaces
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND trim(char *s)
{
   while (s && (*s) && (*s <= ' '))
      strcpy(s, &s[1]);
   while (s && (*s) && (s[strlen(s)-1] <= ' '))
      s[strlen(s)-1] = '\0';
}



/****************************************************************************
 * name:    list_len
 * purpose: Returns the length of a comma separated list of prolog terms
 * inputs:
 *   - char *list: list of prolog terms
 * returns:
 *   the number of elements in the list
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND list_len(char *list)
{
   int count = 0;
   char *ATerm = NULL;


   while (strcmp(list, "") != 0) {
      Term(list, &ATerm, &list);
      OAA_FREE(ATerm);
      count = count + 1;
   }
   return count;
}


/****************************************************************************
 * name:    group_to_terms
 * purpose: Converts an incoming group (a list delimited by either [],{} or
 *          () ) into a list of terms, on which we can use list_len, NthElt, 
 *      etc. i.e. a comma-separated list "elt1, elt2, etl3".
 *      Basically, just removes the brackets, parens, etc.
 * inputs/outputs:
 *   - char *s: a prolog group 
 * remarks: 
 *   group_to_terms overwrites old value with the new because the new value
 *   is guaranteed to be shorter.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND group_to_terms(char *s)
{
   int err = 0;

   if (s && (*s) && (strchr("{([", *s ) != NULL))
      strcpy(s, &s[1]);
   else err = 1;
   if (s && (*s) && (strchr("})]", s[strlen(s)-1]) != NULL))
      s[strlen(s)-1] = '\0';
   else err = 1;

   if (err) printf("Error in group_to_terms() -- invalid group:\n  '%s'\n",s);
}


/****************************************************************************
 * name:    list_to_terms
 * purpose: Converts an incoming prolog list [elt1, elt2, elt3] form) into 
 *      a list of terms, on which we can use list_len, NthElt, etc.
 *      i.e. a comma-separated list "elt1, elt2, etl3".
 *      Basically, just removes the brackets...
 * inputs/outputs:
 *   - char *s: a list in prolog canonical syntax
 * remarks: 
 *   list_to_terms overwrites old value with the new because the new value
 *   is guaranteed to be shorter.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND list_to_terms(char *s)
{
   int err = 0;

   if (s && (*s) && (*s == '['))
      strcpy(s, &s[1]);
   else err = 1;
   if (s && (*s) && (s[strlen(s)-1] == ']'))
      s[strlen(s)-1] = '\0';
   else err = 1;

   /*
   if (err) printf("Error in list_to_terms() -- invalid list:\n  '%s'\n",s);
   */
}


/*****************************************************************************
* Unification-related functions
*****************************************************************************/

/****************************************************************************
 * name:    bind_var
 * purpose: Stores a var/value pair in stringlist vars
 * inputs:
 *   - char *var: Variable name
 *   - char *value: Value
 *   - dyn_array vars: string list of var/value bindings
 * remarks: 
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND bind_var(char *var,
              char *value,
              struct dyn_array *vars)
{
   append_strlist(vars, var);
   append_strlist(vars, value);
}



/****************************************************************************
 * name:    deref
 * purpose: See if a variable can be resolved using existing variable bindings
 * inputs:
 *   - char *var: will return value if one can be found in var_bindings
 *   - char *var_bindings[]:
 *          A list stored with var/value in l[N]/l[N+1]
 * remarks: 
 *   quits program!
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND deref(char **var,
           struct dyn_array var_bindings)
{
   int done = 0;
   int pos;

   do {
      pos = member_strlist(*var, var_bindings, 2, 0);
      if (pos >= 0) {
         OAA_FREE(*var);
         *var = strdup(var_bindings.item[pos+1]);
      }
      else done = 1;
   } while (!done);
}



/****************************************************************************
 * name:    collect_vars
 * purpose: Collects all unique vars in a term into a strlist
 * remarks: 
 *    - Returns the stringlist of vars/bindings which must be deallocated
 *      when finished (free_strlist).
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND collect_vars(char *term,
                  struct dyn_array *vars)
{
   /* Dynamically allocated vars */
   char *fun1 = NULL;
   char *args1 = NULL;
   char *elt1 = NULL;

   /* These vars do not need to be deallocated, as they 
      are not dynamically allocated */
   char *p1;

   Functor(term, &fun1, &args1);
   trim(fun1);
   /* remove_quotes(fun1); */

   if (is_var(fun1)) {
      /* Anonymous variable "_" never gets bound */
      if (vars && (strcmp(fun1,"_") != 0) && 
     (member_strlist(fun1,*vars,2,0) == -1))   /* not already in list */
         bind_var(fun1, fun1, vars);           /* bind var to itself */
      goto done;
   }

   if (is_list(fun1) && (strcmp(fun1,"[]") != 0)) {
      args1 = strdup(fun1);
      list_to_terms(args1);
   }
   p1 = args1;
   if (*args1) {
      while ((strcmp(p1, "") != 0)) {
         Term(p1, &elt1, &p1);
         collect_vars(elt1,vars);
         OAA_FREE(elt1);
      }
   }

done:
   OAA_FREE(fun1);
   OAA_FREE(args1);
   OAA_FREE(elt1);
}





/****************************************************************************
 * name:    deref_term
 * purpose: replace all variables in a term by their bindings
 * inputs:
 *   - char *t, t2: terms to match (unify)
 *   - char ***vars: a string list where variable bindings will be stored
 *      during unification.  If vars is NULL, no var bindings are kept,
 *      acting as a simple match instead of a true unify.
 * remarks: 
 *    - Returns the stringlist of vars/bindings which must be deallocated
 *      when finished (free_strlist).
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND deref_term(char *term,
                char **answer,
                struct dyn_array vars)
{
   /* Dynamically allocated vars */
   char *fun1 = NULL;
   char *args1 = NULL;
   char *elt1 = NULL;

   /* These vars do not need to be deallocated, as they 
      are not dynamically allocated */
   char *p1;
   char openp[2], closep[2];

   Functor(term, &fun1, &args1);
   trim(fun1);
   /* remove_quotes(fun1); */

   deref(&fun1,vars);

   if (is_list(fun1) && (strcmp(fun1,"[]") != 0)) {
      args1 = strdup(fun1);
      list_to_terms(args1);
      strcpy(openp, "[");
      strcpy(closep, "]");
   }
   else {
      append(answer, fun1);
      strcpy(openp, "(");
      strcpy(closep, ")");
   }
   p1 = args1;
   if (*args1) {
      append(answer, openp);
      while ((strcmp(p1, "") != 0)) {
	Term(p1, &elt1, &p1);
	deref_term(elt1,answer,vars);
	if (*p1)
	  append(answer, ",");
	OAA_FREE(elt1);
      }
      append(answer, closep);
   }

   OAA_FREE(fun1);
   OAA_FREE(args1);
   OAA_FREE(elt1);
}



/****************************************************************************
 * name:    rename_vars
 * purpose: Renames all unique vars in a term to "_NUM" format.
 * remarks: Used by asserts to get around variable name collisions
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND rename_vars(char *term,
                 char **new_term)
{
   struct dyn_array vars;
   int i;

   init_dyn_array(&vars);

   collect_vars(term, &vars);

   /* Replace var values by "_NUM" format */
   for (i=1; i < vars.count; i = i + 2) {
      OAA_FREE(vars.item[i]);
      vars.item[i] = malloc(5);
      sprintf(vars.item[i], "_%d", i/2+1);
   }

   deref_term(term, new_term, vars);

   free_strlist(&vars);
}


/****************************************************************************
 * name:    match_terms
 * purpose: Returns true if Term1 matches Term2, saving variable bindings
 *          as it goes (if requested)
 * inputs:
 *   - char *t, t2: terms to match (unify)
 *   - dyn_array *vars: a string list where variable bindings will be stored
 *      during unification.  If vars is NULL, no var bindings are kept,
 *      acting as a simple match instead of a true unify.
 * remarks: 
 *    - Returns the stringlist of vars/bindings which must be deallocated
 *      when finished (free_strlist).
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND match_terms(char *t1,
                char *t2,
                struct dyn_array *vars)
{
   /* Dynamically allocated vars */
   char *fun1 = NULL;
   char *fun2 = NULL;
   char *args1 = NULL;
   char *args2 = NULL;
   char *elt1 = NULL;
   char *elt2 = NULL;

   /* These vars do not need to be deallocated, as they 
      are not dynamically allocated */
   char *p1, *p2;

   int result = TRUE;

   Functor(t1, &fun1, &args1);
   Functor(t2, &fun2, &args2);

   trim(fun1);
   trim(fun2);

   if (vars) {
      deref(&fun1,*vars);
      deref(&fun2,*vars);
   }

   if (is_var(fun1)) {
      /* Anonymous variable "_" never gets bound */
      if (vars && (strcmp(fun1,"_") != 0) && (strcmp(fun1, fun2) != 0))
         bind_var(fun1, fun2, vars);
      goto done;
   } else
   if (is_var(fun2)) {
      /* Anonymous variable "_" never gets bound */
      if (vars && (strcmp(fun2,"_") != 0) && (strcmp(fun1, fun2) != 0))
         bind_var(fun2, fun1, vars);
      goto done;
   } else {

      /* If both are [A,B,C] style lists, remove brackets
	 and start unifying args.  functors are set to "." */
     /* Works the same for (A,B,C) and {A,B,C}.  */
     if (is_group(fun1) && is_group(fun2)) {
       args1 = strdup(fun1);
       args2 = strdup(fun2);
       group_to_terms(args1);
       group_to_terms(args2);
       OAA_FREE(fun1);
       OAA_FREE(fun2);
       fun1 = strdup(".");
       fun2 = strdup(".");
     }
     
     remove_quotes(fun1);
     remove_quotes(fun2);
     if (strcmp(fun1, fun2) == 0) {   /* same functor */

       p1 = args1;
       p2 = args2;
       while ((strcmp(p1, "") != 0) && (strcmp(p2, "") != 0)) {
	 Term(p1, &elt1, &p1);
	 Term(p2, &elt2, &p2);
	 if (!match_terms(elt1, elt2, vars)) {
	   result = FALSE;
	   goto done;
	 }
         
         OAA_FREE(elt1);
	 OAA_FREE(elt2);
       }
       if ((*p1 != 0) || (*p2 != 0)) {
	 result = FALSE;
	 goto done;
       }
     }
     else {
       result = FALSE;
       goto done;
     }
   }

done:
   OAA_FREE(fun1);
   OAA_FREE(fun2);
   OAA_FREE(args1);
   OAA_FREE(args2);
   OAA_FREE(elt1);
   OAA_FREE(elt2);

   return result; 
   
}



/****************************************************************************
 * name:    unify
 * purpose: Perform true unification and return resulting term
 * returns: if the two terms unify, returns true and answer will contain 
 *      the unified term. Otherwise returns false (answer not changed)
 * remarks: 
 *   - If unification is successful, answer will be created in a new
 *     space which should be OAA_FREE'd when done using.
 *   - answer may be NULL, in which case the resulting term is not returned
 *     but unify will still return true or false
 *   - unify() only works for simple prolog structures.  The following do
 *     NOT work:
 *        o Expressions using operators,  eg: A = 2, A -> B | C.
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND unify(char *term1,
          char *term2,
          char **answer)
{
   struct dyn_array da;
   int result;

   init_dyn_array(&da);

   result = match_terms(term1, term2, &da);
   if (result && answer)
      deref_term(term1, answer, da);

   free_strlist(&da);
   return result;
}



/*****************************************************************************
* Trace and error messages
*****************************************************************************/




/****************************************************************************
 * name:    print_error
 * purpose: Print error message
 * inputs:
 *   - char *ErrMsg: Error message to be printed
 * remarks: 
 ****************************************************************************/
void
print_error(char *ErrMsg)
{
#if defined(IS_WINDOWS) && !defined(IS_CONSOLE) /* ------------------------ */
  char *temp;
  temp=(char*)malloc(sizeof(char)*255);
  sprintf(temp,"OAA Error: %s\n", ErrMsg);
  win_printf(temp);
  fflush(stdout);
  free(temp);  /* --------------------------------------------------------- */
#endif
 
#if defined IS_UNIX  || defined(IS_CONSOLE) /*---------------------------------- */
   printf("OAA Error: %s\n", ErrMsg);
   fflush(stdout);
#endif  /* ---------------------------------------------------------------- */
}

/****************************************************************************
 * name:    error_out
 * purpose: Print error message and exit program
 * inputs:
 *   - char *ErrMsg: Error message to be printed
 * remarks: 
 *   quits program!
 ****************************************************************************/
void
error_out(char *ErrMsg)
{
#if defined(IS_WINDOWS) && !defined(IS_CONSOLE) /* ------------------------ */
  char *temp;
  temp=(char*)malloc(sizeof(char)*255);
  sprintf(temp,"OAA Error: %s\n", ErrMsg);
  win_printf(temp);
  fflush(stdout);
  free(temp);  /* --------------------------------------------------------- */
#endif
 
#if defined IS_UNIX  || defined(IS_CONSOLE) /*---------------------------------- */
  printf("OAA Error: %s\n", ErrMsg);
  fflush(stdout);
#endif  /* ---------------------------------------------------------------- */
  /*oaa_app_done();*/
  /*disconnect();*/
  /* exit(1); */
}
 


/****************************************************************************
 * name:    if_trace_on
 * purpose: print a trace message if TraceOn is set
 * inputs:
 *   - char *msg
 *   - char *arg
 ****************************************************************************/
void
if_trace_on(char *msg,char *arg)
{
   if (TraceOn) {
      if (arg)
   printf(msg, arg);
      else
   printf(msg);
      fflush(stdout);
   }
}

/****************************************************************************
 * name:    if_tcp_trace_on
 * purpose: print a trace message if TcpTraceOn is set
 * inputs:
 *   - char *msg
 *   - char *arg1
 *   - char *arg2
 ****************************************************************************/
void
if_tcp_trace_on(char *msg, char *arg1, char *arg2)
{
   if (TcpTraceOn) {
      if (arg1 && arg2)
   printf(msg, arg1, arg2);
      else
      if (arg1)
   printf(msg, arg1);
      else
   printf(msg);
      fflush(stdout);
   }
}


/*****************************************************************************
* Sending, receiving data
*****************************************************************************/

/****************************************************************************
 * name:    basic_send, secure_send
 * purpose: Sends raw data (to the server)
 * inputs:
 *   - char *data: data to be sent
 *   - int size: size of the data
 * outputs:
 *   returns -1 if send (or write) fails; otherwise, returns the number of 
 *   bytes sent.
 * remarks:
 *  Messages bigger than 32K, need to be cut into smaller messages of
 *  32K, so write (send) behaves correctly.
 *    More work is needed to make this code robust.  We do nothing to handle
 *  errors from write or send.  Also, it might be good to check the socket
 *  (using poll) before writing. However, with this code, we have only
 *  seen errors in Lisp agents, and only when we purposely break the
 *  facilitator, so that it's not receiving the message promptly.
 *    We have found that in Lisp agents, write and send behave differently,
 *  write working better.
 ****************************************************************************/

#define MAX_SEND 10000

int basic_send(char* inData, int inSize) {
    int ret;

  if ((ret = PR_Send(Connection, inData, inSize,
		     0, PR_INTERVAL_NO_TIMEOUT)) == -1) {
    char error_message[255];
    sprintf(error_message,"Error in writing socket; inSize = %d, errno = %d\n",
            inSize, errno);
    print_error(error_message);
    return -1;
  }
  return ret;
}

int
secure_send(char *S, int size)
{ 
  int bytes_sent;

  while (size > 0) {
    bytes_sent = basic_send(S, min(size, MAX_SEND));
    if (bytes_sent == -1) 
        return -1;
    size-=bytes_sent;
    S+=bytes_sent;
  }
  return 1;
}

/****************************************************************************
 * name:    send_data
 * purpose: Sends raw data (not an event) to the server
 * inputs:
 *   - char *data: unformatted data to be sent
 * remarks:
 *   for internal use only...
 ****************************************************************************/
void
send_data(char *S)
{
  if (Connection != INVALID_OAASOCKET) {
    int n;
    char *tmpstr = NULL;

    tmpstr = malloc(strlen(S) + 5);
    sprintf(tmpstr, "%s.\n", S);
    n = strlen(tmpstr);
    if (secure_send( tmpstr, n) == -1)  {
      error_out("Error in write (send_data)\n");
      disconnect(); /* The connection to the facilitator was lost */
    }
    OAA_FREE(tmpstr);
  }
}


/****************************************************************************
 * name:    post_event
 * purpose: Sends an event to the server
 * inputs:
 *   - char *event: event to be posted
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND post_event(char *Event)
{
  if (Connection != INVALID_OAASOCKET) {
    int n;
    char *tmpstr = NULL;

    tmpstr = malloc(strlen(Event) + 10);
    sprintf(tmpstr, "term(%s).\n", Event);
    n = strlen(tmpstr);

    if (secure_send( tmpstr, n) == -1)  {
      error_out("Error in write (post_event)\n");
      disconnect();  /* The connection to the facilitator was lost */
    }
    OAA_FREE(tmpstr);
  }
}



/****************************************************************************
 * name:    extract_event
 * purpose: Reads one event from buffer stream and returns KS,Event
 * remarks:
 *   - KS should point to a buffer large enough to contain KS name
 *   - event will be in form "term(Result)." or "term(KS,Result)."
 ****************************************************************************/
void
extract_event(char *one_event,
              char *KS,
              char **result)
{
   char *func = NULL;
   char *args = NULL;

   Functor(one_event, &func, &args);
   if (strcmp(func, "term") == 0) {
      /* term(event(KS,Result)) */
      if (strstr(args, "event(") == args) {
   Argument(args, 2, result);
   OAA_FREE(func);
   Argument(args, 1, &func);
   strcpy(KS, func);
      }
      else *result = strdup(args);
   }
   else {
      printf("\nError: Event not wrapped in 'term().':\n %s\n", one_event);
      fflush(stdout);
   }
   OAA_FREE(func);
   OAA_FREE(args);
}



/****************************************************************************
 * name:    get_event
 * purpose: Reads an event from the TCP stream.
 * outputs:
 *   - char *KS: The KS who sent the event
 *   - char **event: The event read.
 * returns:
 *   TRUE if more events are immediately available in the queue
 * remarks:
 *   - KS should point to a buffer large enough to contain KS name
 *   - Returns a new pointer to the event, which should be free()'d
 *     when finished with the event.
 *   - Event returned is dynamically allocated and can have any length
 ****************************************************************************/
int get_event(OAASOCKET inputStream,
              char *KS,
              char **event)
{
 
  OAASOCKET FD;

#if defined (IS_UNIX) || defined (IS_CONSOLE)
  /* ------------------------------------------------------- */
  int r;
#endif /* ------------------------------------------------------- */
  int n;

  static char *E = NULL; /* Next event to be executed */
  char *one_event = NULL; 
  char *rest = NULL;
  char *result = NULL;
  char *end;
  char buffer[4096];    /* Read buffer size */
  int  finished_transmission = FALSE;

  /* Default KS if none sent in term(event(KS,Event)). wrapper */
  strcpy(KS, "Server");

  FD = inputStream;

  /* DG Jan 1999, before performing any action, checks the connection's status */
  if (Connection == INVALID_OAASOCKET) {
    *event = NULL;
    return FALSE;
  }

  /* ------------------------------------------------------------------- */
  /* Read events from the TCP socket. Events can be bigger than the read */
  /* buffer, so a loop is used until at least one entire event is read   */
  /* into E.                  */
  /* When reading data into buffer, more than one event can arrive at a  */
  /* time.  If more than one event arrives, the first event is removed   */
  /* from the buffer and returned.  The next event is stored in memory    */
  /* (in E), and will be immediately executed the next time get_event is */
  /* called, before reading from TCP socket.          */
  /* ------------------------------------------------------------------- */

  /* If E contains some text, use Term() to get first event to process */
  /* If Term() succeeds, a complete event was read, if it fails, the   */
  /* event was only partially read and we must read some more.    */
  /* Notes: one_event only needs to be OAA_FREE'd if Term succeeds, rest   */
  /*        never needs to be OAA_FREE'd.        */
  /* 3-11-97 HB: added a check that E has an open paren, since sometimes */
  /*             it held just the "t" of "term(", which caused Term() to */
  /*             succeed, but caused a failure further down, since really*/
  /*             there was more to read                                  */
  if (E && (*E) && strchr(E,'(') && Term(E, &one_event, &rest)) { 
    finished_transmission = TRUE;      /* TCP read not needed */

    /* Extract sending KS (if supplied) and event to process */
    extract_event(one_event, KS, &result);

    if (*rest == '.')        /* Skip over trailing '.' */
      end = strdup(rest+1);
    else 
      end = strdup(rest);
    OAA_FREE(E);            /* E should point to newly */
    E = end;                /*  allocated space contiaing */
    OAA_FREE(one_event);    /*  more events.     */
  }
  else 
    finished_transmission = FALSE;

  /* If no complete event found in memory, read TCP socket until event */
  while (!finished_transmission) {  /* If event is bigger than buffer, */
    /* loop until event is read */
  /* ------------------------------------------------------- */

  if (TimeOutMode > 0)
    r = tcpSelect(FD, TimeOut);
  else
    r = tcpSelect(FD, 0);

  if (Connection == INVALID_OAASOCKET) {
    *event = NULL;
    return FALSE;
  }
 
  switch (r) {
    case -1: /* Error */
      if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR)
	return FALSE;
      perror(NULL);
      *event = NULL;
      return FALSE;
      break;
    case 0: /* Timeout */
      strcpy(KS, KSname);
      *event = strdup(TIMEOUT_EVENT);
      return FALSE;
      break;
    default: /* Success, something to read */

      n = PR_Recv(FD, buffer, sizeof(buffer), 0, PR_INTERVAL_NO_TIMEOUT);
      /* 
        DEBUG Information
        sprintf(debugString, "After recv %d", n);
        win_printf(debugString);
      */ 
 
      switch (n) {
        case -1:      /* error while reading socket */
	  if (PR_GetError () == PR_PENDING_INTERRUPT_ERROR)
	    return FALSE;
          error_out("read");
          disconnect(); /* The connection to the facilitator was lost */
          *event = NULL;
          return FALSE;
        case 0:       /* end of file */
	  if ( IsConnected ) {
	    printf("~nEND_OF_FILE detected: server died?\n");
	    fflush(stdout);
	    disconnect(); /* The connection to the facilitator was lost */
	  }
          *event = NULL;
          return FALSE;
        default:   /* SUCCESS */
          buffer[n] = '\0';               /* terminate string */
          break;
        }
      }

      /* Add data read this time to E, buffer of events to process */
      append(&E, buffer);

      /* Use Term() to get first event to process                */
      /* If Term() succeeds, a complete event was read, if it fails, the   */
      /* event was only partially read and we must read some more.    */
      /* Notes: one_event only needs to be OAA_FREE'd if Term succeeds, rest   */
      /*        never needs to be OAA_FREE'd.          */
      if (E && (*E) && strchr(E,'(') && Term(E, &one_event, &rest)) { 
        finished_transmission = TRUE;     /* TCP read not needed */
        /* Extract sending KS (if supplied) and event to process */
        extract_event(one_event, KS, &result);
        if (*rest == '.')      /* Skip over trailing '.' */
          end = strdup(rest+1);
        else 
          end = strdup(rest);
        OAA_FREE(E);        /* E should point to newly */
        E = end;        /*  allocated space contiaing */
        OAA_FREE(one_event);      /*  more events.     */
      }
  }   /* While : go read more data if end of event not found. */ 
  *event = result;
  /*return (strcmp(E, "") != 0);*/  /* Return true if E not empty */
  return (result && result[0] != '\0');  /* XXX reddy - 7 Feb 2000 */
  /* (more events to read) */
}



/****************************************************************************
 * name:    get_event_from_connection
 * purpose: Reads an event from the TCP stream.
 * outputs:
 *   - char *KS: The KS who sent the event
 *   - char **event: The event read.
 * returns:
 *   TRUE if more events are immediately available in the queue
 * remarks:
 *   This is the same as get_event/3, except that it does not require
 *   the inputStream argument.  This is for the benefit of the Lisp
 *   agent library, which needs to call get_event, but doesn't have
 *   access to the global var. Connection.
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND get_event_from_connection(char *KS, char **event)
{
    return get_event(Connection, KS, event);
}



/*****************************************************************************
* Solve: requesting solutions from the Facilitator
*****************************************************************************/

#ifndef IS_CONSOLE /* ------------------------------------------------------*/
#ifdef IS_WINDOWS   /* ----------------------------------------------------- */
 /****************************************************************************
 * name:    processWindowsevents
 * purpose: Executes events in the Windows queue
 * remarks:
 *    Can be used in a loop while waiting for some
 *    global event to become true.
 ****************************************************************************/
void
processWindowsevents()
{
/* DG  int mask; */
  MSG msg;
 
   if (GetMessage (&msg, NULL, 0, 0))
    {
      TranslateMessage (&msg) ;
       DispatchMessage (&msg) ;
    }
}
#endif  /* -------------------------------------------------------------- */
#endif

#if defined (IS_UNIX) || defined (IS_CONSOLE)
#ifdef XWINDOWS

/****************************************************************************
 * name:    processXevents
 * purpose: Executes events in the X queue
 * remarks:
 *    Can be used in a loop while waiting for some
 *    global event to become true.
 ****************************************************************************/
void
processXevents()
{
    int mask;

    /*
     * Flush the client side output buffer.
     *
     * NOTE: The `False' indicates to *NOT* discard queued events.
    XSync(TheDisplay, False);
     */

    /*
     * Use application context information if given.
     */
    if( app_context != (XtAppContext)NULL ) {
      if( (mask = XtAppPending(app_context)) ) 
     XtAppProcessEvent(app_context, mask);
      }
      else {
  if( (mask = XtPending() ) )
     XtProcessEvent(mask);
      }
}
#endif
#endif


/****************************************************************************
 * name:    poll_until_event
 * purpose: Waits until a matching event is returned from server
 * inputs:
 *   - char *wait_event: event template we are waiting for
 *   - char **solved_event: the specific event returned
 * remarks:
 *   If XWindows is selected, a simulated XtMainLoop is run so that X
 *   can process its events.  Otherwise, get_event is simply called.
 ****************************************************************************/
void
poll_until_event(wait_event, solved_event)
char *wait_event;
char **solved_event;
{
   int event_returned = FALSE;
   char KS[KS_SIZE];
   char *event = NULL;
   char *answers = NULL;

   while (!event_returned) {

/*-------- Get an event ----------*/
#ifndef IS_CONSOLE /* ------------------------------------------------------*/
#ifdef IS_WINDOWS /* ------------------------------------------------------ */
 
    polling_for_event = TRUE;
    poll_event = NULL;
    /* Process Windows event stream until
       an event has been returned */
    while (poll_event == NULL) {
      processWindowsevents();
    }
    event_returned = TRUE; /* DG */
    *solved_event = poll_event;    /* DG */
    /* event = poll_event; */
    strcpy(KS, poll_ks);
    poll_event = NULL;
    polling_for_event = FALSE;
 
#endif  /* ---------------------------------------------------------------- */
#endif

/*-------- Get an event ----------*/
#if defined (IS_UNIX) || defined (IS_CONSOLE)
#ifdef XWINDOWS

      polling_for_event = wait_event;
      poll_event = NULL;

      /* Process X event stream until the right event has been returned */
      while (poll_event == NULL) {
        processXevents();
      }

      event_returned = TRUE;
      *solved_event = poll_event;
      strcpy(KS, poll_ks);
      poll_event = NULL;
      polling_for_event = NULL;

#else 
      get_event(Connection, KS, &event);

      /* Is event the desired one? */
      if (unify(event, wait_event,NULL)) {   /* Awaited event is here */
        event_returned = TRUE;
        *solved_event = strdup(event);
      }
      else {    /* An unexpected event: executed it anyway, then wait */  
        if (strcmp(event, TIMEOUT_EVENT) != 0) {
          if_tcp_trace_on("\n[TCP received from %s] :\n  %s.\n", KS, event);
            interpret(KS, event, &answers);
        } else 
          oaa_app_idle();
      }
#endif
#endif
/*--------------------------------*/
      OAA_FREE(event);
      OAA_FREE(answers);

   }  /* While */

}


/****************************************************************************
 * name:    solve
 * purpose: requests data or actions from distributed agents
 * inputs:
 *   - char   *goal: an ICL goal to be solve/performed
 *   - char *params: control parameters describing how the goal should be
 *    sent/resolved (see below).
 * outputs:
 *   - Answers: a list containing solutions to the requested goal.
 *        The answer value "[]" constitutes failure of the goal.
 * parameters:
 *    The params argument is a list which may contain:
 *
 *       cache           : cache all solutions locally              
 *       level_limit(N)  : highest number of levels to climb for    
 *                            solutions.                               
 *       address(KS)     : ask a specific KS to solve goal          
 *       and_parallel    : and-parallel solve of Goal list          
 *       or_parallel     : or-parallel solve of Goal list           
 *       test(Test)      : only solve goal on blackboards where Test
 *       succeeds locally.                        
 *       broadcast       : just disseminate message, do not request 
 *                            results.                                 
 *       asynchronous    : solve will not block until solution returns,
 *         the solutions will be returned asynchronously
 *         in solved(FromKs,Goal,Params,Solutions) messages
 *                         which should be handled by in oaa_do_event()
 *       solution_limit(N): limits the number of solutions found to 
 *                    N.  Currently only works for 1...        
 *       time_limit(N)   : Waits a maximum of N seconds before      
 *                            returning (failure if no solution).      
 *
 * remarks:
 *    If "broadcast" is sent in the parameter list, Ans may be sent as
 *    NULL.  Otherwise, solve will return anwers in a new space which
 *    should be deallocated using OAA_FREE() when no longer needed.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND solve(char *Goal,
           char *Params,
           char **Ans)
{
    char *event = NULL;
    char *solved_event = NULL;

    /* If connected, perform solve request */

    if (Connection != INVALID_OAASOCKET) {

       /* Add contexts to params (don't free new_params!) */
       char *new_params;
       
       if (context_stack_p > 0)
    new_params = oaa_AddContextParams(
        context_stack[context_stack_p-1], Params);
       else new_params = Params;

       event = malloc(strlen(Goal) + strlen(new_params) + 20);
       sprintf(event, "post_query(%s,%s)", Goal, new_params);
       post_event(event);

       OAA_FREE(event);

       /* Broadcast and asynchronous not requested: wait for return event */
       if ((strstr(Params, "broadcast") == 0) && 
     (strstr(Params,"asynchronous") == 0)) {
          event = malloc(strlen(Goal) + 50);
          sprintf(event, "solved(_FromKS, %s, _Params, _Solutions)", Goal);
          poll_until_event(event, &solved_event);
          if (solved_event)
             Argument(solved_event, 4, Ans);   /* Solutions */
       }
       /* Broadcast or Asynchronous requested:return immediately (no answers) */
       else {
          if (Ans)
             *Ans = strdup("[]");
       }
       OAA_FREE(event);
       OAA_FREE(solved_event);
   }
   /* if not connected, just return failure */
   else 
      if (Ans)
   *Ans = strdup("[]");
}



/*****************************************************************************
* Global data management
*****************************************************************************/

/****************************************************************************
 * name:    write_bb
 * purpose: Add information to global data stored on blackboard server
 * inputs:
 *   - char *item: type of data being written
 *   - char *data: value for the data type
 * remarks: 
 *   ex: write_bb(name, "Adam Cheyer")  adds name("Adam Cheyer") on blackboard
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND write_bb(char *Item,
              char *Data)
{
   char *s = NULL;

   s = malloc(strlen(Item) + strlen(Data) + 20);
   sprintf(s, "write_bb(%s, %s)", Item, Data);
   post_event(s);

   OAA_FREE(s);
}


/****************************************************************************
 * name:    write_replace_bb
 * purpose: Add information to global data stored on blackboard server,
 *          replacing old information with new information
 * inputs:
 *   - char *item: type of data being written
 *   - char *data: value for the data type
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND write_replace_bb(char *Item,
                                        char *Data)
{
   char *s = NULL;

   s = malloc(strlen(Item) + strlen(Data) + 30);
   sprintf(s, "write_replace_bb(%s, %s)", Item, Data);
   post_event(s);

   OAA_FREE(s);
}


/****************************************************************************
 * name:    write_once_bb
 * purpose: Add information to global data stored on blackboard server
 *          at most one times (if exists, doesn't write).
 * inputs:
 *   - char *item: type of data being written
 *   - char *data: value for the data type
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND write_once_bb(char *Item,
                   char *Data)
{
   char *s = NULL;

   s = malloc(strlen(Item) + strlen(Data) + 20);
   sprintf(s, "write_once_bb(%s, %s)", Item, Data);
   post_event(s);

   OAA_FREE(s);
}


/****************************************************************************
 * name:    retract_bb
 * purpose: Remove item from global data on blackboard
 * inputs:
 *   - char *item: type of data to remove
 *   - char *data: value of data to remove
 * remarks: 
 *   If Data points to a var, it will remove any value of type "item"
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND retract_bb(char *Item,
                char *Data)
{
   char *s = NULL;

   s = malloc(strlen(Item) + strlen(Data) + 20);
   sprintf(s, "retract_bb(%s, %s)", Item, Data);
   post_event(s);

   OAA_FREE(s);
}


/****************************************************************************
 * name:    replace_bb
 * purpose: Changes item value from Old to New
 * inputs:
 *   - char *item: type of data to remove
 *   - char *olddata: value of data to remove
 *   - char *newdata: new value of data
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND replace_bb(char *Item,
                char *OldData,
                char *NewData)
{
   char *s = NULL;

   s = malloc(strlen(Item) + strlen(OldData) + strlen(NewData) + 20);
   sprintf(s, "replace_bb(%s, %s, %s)", Item, OldData, NewData);
   post_event(s);

   OAA_FREE(s);
}



/****************************************************************************
 * name:    read_bb
 * purpose: Retrieves information from Facilitator's global data store
 * inputs:
 *   - char *item: type of data being written
 *   - char *data: value for the data type, variable if requesting values
 * outputs:
 *   - char **answers: list containing results 
 * remarks: 
 *    read_bb will return anwers in a new space which should be deallocated 
 *    using OAA_FREE() when no longer needed.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND read_bb(char *Item,
             char *Data,
             char **answers)
{
   char *s = NULL;

   /* Send request for information to Facilitator */
   s = malloc(strlen(Item) + strlen(Data) + 20);
   sprintf(s, "read_bb(%s, %s)", Item, Data);
   post_event(s);
   OAA_FREE(s);

   /* Wait for answers */
   poll_until_event("return_read_bb(Solutions)", &s);
   Argument(s, 1, answers);  /* Retrieve list of read_bbs */

   OAA_FREE(s);
}


/****************************************************************************
 * name:    can_solve
 * purpose:  Asks the Facilitator for a list of agents who can solve a Goal
 * inputs:
 *   - char *goal: Solvable to search for
 * outputs:
 *   - char **answers: list containing agent ids
 * remarks:
 *    can_solve will return anwers in a new space which should be deallocated
 *    using OAA_FREE() when no longer needed.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND can_solve(char *goal,
             char **answers)
{
   char *s = NULL;
 
   /* Send request for information to Facilitator */
   s = malloc(strlen(goal) + 20);
   sprintf(s, "can_solve(%s)", goal);
   post_event(s);
   OAA_FREE(s);
 
   /* Wait for answers */
   poll_until_event("return_can_solve(Goal,Solutions)", &s);
   Argument(s, 2, answers);     /* Retrieve list of read_bbs */
 
   OAA_FREE(s);
}



/****************************************************************************
 * name:    add_trigger
 * purpose: Adds a trigger to the appropriate agent.
 * inputs:
 *   - char *Kind: may be 'test', 'data', or 'alarm'
 *   - char *Type: may be 'when', 'if' (single), 'whenever' (permanent)
 *   - char *Condition: test condition
 *   - char *Action: action to be performed if Condition succeeds
 * remarks: 
 *
 *   This function is OBSOLETE, replaced by add_remote_trigger/6.
 *
 *   add_trigger/4 does not require a KS to be specified, because the KS
 *   will be determined automatically:
 *      data: always installed on Server
 *      test: installed on appropriate agent by solvable
 *      alarm: installed on alarm agent
 *   For 'event' triggers, use add_trigger_on_ks()
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND add_trigger(char *Kind,
                 char *Type,
                 char *Condition,
                 char *Action)
{
   char *s = NULL;

   s = malloc(strlen(Type) + strlen(Condition) + strlen(Action) + 40);

   if (strcmp(Kind, "test") == 0) {
      sprintf(s, "post_trigger(test,%s,%s,%s)", Type, Condition, Action);
      post_event(s);
   }
   else
   if (strcmp(Kind, "data") == 0) {
      sprintf(s, "post_trigger(data,%s,%s,%s)", Type, Condition, Action);
      post_event(s);
   }
   else
   if (strcmp(Kind, "alarm") == 0) {
      sprintf(s, "add_alarm_trigger(alarm,%s,%s,%s)", Type, Condition, Action);
      solve(s, "[broadcast]", NULL);
   }

   OAA_FREE(s);
}


/****************************************************************************
 * name:    add_remote_trigger
 * purpose: Adds a trigger to the appropriate agent.
 * inputs:
 *   - char *Kind: may be 'test', 'data', or 'alarm'
 *   - char *Type: may be 'when', 'if' (single), 'whenever' (permanent)
 *   - char *OpMask: on_write, on_replace, on_retract, on_write_replace,
 *    or a list combining several of these
 *   - char *Template: template against which to match the trigger 
 *    (e.g. data, or test)
 *   - char *Action: action to be performed if Condition succeeds
 * remarks:
 *   add_remote_trigger/6 does not require a KS to be specified, because the 
 *    KS will be determined automatically:
 *      data: always installed on Server
 *      test: installed on appropriate agent by solvable
 *      alarm: installed on alarm agent
 *   For 'event' triggers, use add_trigger_on_ks()
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND add_remote_trigger(char *Kind,
                 char *Type, char *OpMask,
                 char *Template, char *MoreConditions,
                 char *Action)
{
   char *s;
 
   s = malloc(strlen(Type) + strlen(Template) + strlen(Action) + 
  strlen(OpMask) + strlen(MoreConditions) + 40);
 
   if ((strcmp(Kind, "test") == 0) || (strcmp(Kind, "data") == 0)) {
      sprintf(s, "post_trigger(%s,%s,%s,%s,%s,%s)", 
    Kind, Type, OpMask, Template, MoreConditions, Action);
      post_event(s);
   }
   else
   if (strcmp(Kind, "alarm") == 0) {
      sprintf(s, "add_alarm_trigger(alarm,%s,%s,task(%s))", Type, Template, Action);
      solve(s, "[broadcast]", NULL);
   }
 
   OAA_FREE(s);
}


/****************************************************************************
 * name:    add_trigger_to_ks
 * purpose: Adds a trigger to the specified agent.
 * inputs:
 *   - char *KS: KS on which the trigger should be installed
 *   - char *Kind: may be 'test', 'event', or 'alarm' (not 'data'!)
 *   - char *Type: may be 'when', 'if' (single), 'whenever' (permanent)
 *   - char *Condition: test condition
 *   - char *Action: action to be performed if Condition succeeds
 * remarks: 
 *   Cannot install a data trigger because data triggers always go on the
 *   Server agent which is not a valid KS.
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND add_trigger_to_ks(char *KS,
                       char *Kind,
                       char *Type,
                       char *Condition,
                       char *Action)
{
   char *s = NULL;

   s = malloc(strlen(Type) + strlen(Condition) + strlen(Action) + 40);

   sprintf(s, "post_trigger(%s,%s,%s,%s,%s)", KS,Kind,Type,Condition,Action);
   post_event(s);

   OAA_FREE(s);
}



/*****************************************************************************
* Loops: awaiting incoming events
*****************************************************************************/

#if defined (IS_UNIX) || defined (IS_CONSOLE)
      /*-------------------------------------------------------*/

/****************************************************************************
 * name:    oaa_process_all_events
 * purpose: Provides the get_event-interpret loop for a non-XWindows 
 *      application.
 * remarks: Loops over all events in queue, executing them.
 *          If timeout is not set (see set_timeout), get_event() blocks
 *         forever until a new event arrives, so this is an infinite
 *         loop
 *          If timeout is set to some value, if no event arrives in the given
 *             amount of time, TIMEOUT_EVENT is returned and 
 *         oaa_process_all_events returns.
 ****************************************************************************/
EXPORT_MSCPP
int EXPORT_BORLAND oaa_process_all_events()
{
  if (IsConnected) {
    char *event = NULL;
    char *answers = NULL;
    char KS[KS_SIZE];
    int done = FALSE;

    /* sfMessage("In process all"); */

    /* DG Jan 1999
       Since perror does not quit the application anymore,
       we added the IsConnected condition in the main loop.
    */
    while (!done && IsConnected) {
      /* Exit FALSE when get_event() fails. */
      if (! get_event(Connection, KS, &event))
	return FALSE;
      if (event) {
        if (strcmp(event, TIMEOUT_EVENT) != 0) {
          if_tcp_trace_on("\n[TCP received from %s] :\n  %s.\n", KS, event);
          interpret(KS, event, &answers);
        }
        else done = TRUE;
      }
      else
        error_out("get_event returned NULL event!\n");
      OAA_FREE(event);
      OAA_FREE(answers);
    }
    /* sfMessage("Out process all"); */
  }

  return IsConnected ? TRUE : FALSE;
}

/****************************************************************************
 * name:    oaa_loop
 * purpose: Provides the get_event-interpret loop for a non-XWindows 
 *      application.
 * remarks: If a timeout occurs, user-defined hook oaa_app_idle() is called.
 ****************************************************************************/
void
oaa_loop(print_ready)
int print_ready;
{
   oaa_ready(print_ready);

   while (TRUE) {
      oaa_process_all_events();
      oaa_app_idle();
   }
}

#endif     /*---------------------------------------------------------------*/




/****************************************************************************
 * name:    handle_incoming_event
 * purpose: Checks for an event from the server
 *          If an event arrives, calls interpret() to execute the event
 *          If a timeout occurs, execute oaa_app_idle()
 * remarks:
 *    only called by oaa_timer()
 ****************************************************************************/
#ifndef IS_CONSOLE /* ------------------------------------------------------*/
#ifdef IS_WINDOWS      /*---------------------------------------------------*/
 
EXPORT_MSCPP
void EXPORT_BORLAND handle_incoming_event(LPARAM lParam)
{
  char *event = NULL;
  char *answers = NULL;
  int  more_events = TRUE;
  char KS[50];
  char *kshost;
 
  switch (WSAGETSELECTEVENT(lParam)) {
    case FD_READ :
      /* Interpret all events in communications queue */
      while (more_events) {
        more_events = get_event(Connection, KS, &event);
        if (event) {
          if (strcmp(event, TIMEOUT_EVENT) != 0) {
            /* If waiting for specific event, don't execute, just return */
            if (polling_for_event) {
              poll_event = strdup(event);
              strcpy(poll_ks, KS);
              more_events = FALSE;
            } /*Otherwise, interpret received event*/
            else {
              if_trace_on("Event received: %s\n", event);
              interpret(KS, event, &answers);
            }
          }
        else 
          oaa_app_idle();
        OAA_FREE(event);
        OAA_FREE(answers);
        } else 
          error_out("get_event returned NULL event!!!\n");
      }
      break;
 
    case FD_WRITE :
      if (!IsConnected)
        {
          IsConnected = TRUE;
          /* Send KS name, if using a server written in QP3.1.4 */
          if (!global_server_type) {
            sprintf(global_tmpstr, "'%s'", KSname);
            send_data(global_tmpstr);
          }
 
          /* Records the hostname of the agent */
          kshost = getenv ("HOST");
          if (kshost)
            write_bb("kshost", kshost);
 
          /* Write language immediately so any communication from server
             will be in correct format */
          write_bb("language", "c");
 
          /* Version 2.0+ correctly receive sending KS address in events */
          write_bb("oaa_version", "2.0");
 
          /* Send list of solvable predicates */
          if (!global_solvable || (strcmp(global_solvable, "") == 0))
            sprintf(global_tmpstr, "register_solvable_goals([], '%s')",
                    global_ks);
          else
            sprintf(global_tmpstr, "register_solvable_goals(%s,'%s')",
                    global_solvable, global_ks);
          post_event(global_tmpstr);
 
          OAA_FREE(global_tmpstr);
          OAA_FREE(global_solvable);
          OAA_FREE(global_ks);
          oaa_ready(0);
        }
      break;
 
    case FD_CLOSE :
      break;
    }
}
#endif
#endif     /*-------------------------------------------------------------------
--------*/
 


#if defined (IS_UNIX) || defined (IS_CONSOLE)
#ifdef XWINDOWS  /* ------------------------------------------------------- */
void 
handle_incoming_event(inputStream)
int inputStream;
{
    char *event = NULL;
    char *answers = NULL;
    int  more_events = TRUE;
    char KS[50];


    /* Interpret all events in communications queue */
    while (more_events) {
       more_events = get_event(inputStream, KS, &event);
       if (event) {
          if (strcmp(event, TIMEOUT_EVENT) != 0) {

       /* If waiting for specific event, don't execute, just return */
       if (polling_for_event) {
      
          /* Is event the desired one? */
                if (unify(event,polling_for_event,NULL)) {   
       /* Awaited event is here */
        poll_event = strdup(event);
             strcpy(poll_ks, KS);
       polling_for_event = NULL;
          }
          else {
            /* Unexpected event, interpret it anyway */
            if_tcp_trace_on("\n[TCP received from %s] :\n  %s.\n", KS, event);
            interpret(KS, event, &answers);
          }
       }
       /* Otherwise, interpret received event */
       else {
          if_tcp_trace_on("\n[TCP received from %s] :\n  %s.\n", KS, event);
                interpret(KS, event, &answers);
       }
          }
       else oaa_app_idle();
          OAA_FREE(event);
          OAA_FREE(answers);
       } else error_out("get_event returned NULL event!\n"); 
    }
}


/****************************************************************************
 * name:    oaa_timer
 * purpose: If agent uses an X-Windows graphical user interface, oaa_timer
 *          can check for messages coming from the Server while still giving
 *          much control to X to move the mouse, refresh display, etc.
 * inputs:
 *   - XtAppContext app_ctxt: Application Context
 * remarks:
 *   - Calls handle_incoming_event: if an event arrives, executes intepret().
 ****************************************************************************/
void
oaa_timer(XtAppContext app_ctxt)
{

    app_context = app_ctxt;

    if (!oaa_timer_id)
       oaa_timer_id = XtAppAddInput(app_context, Connection, (XtPointer) XtInputReadMask,
          (XtInputCallbackProc) handle_incoming_event, 
          (XtPointer) Connection);
}

#endif     /* ------------------------------------------------------- */
#endif


/****************************************************************************
 * name:    oaa_ready
 * purpose: Lets the Server know that the XApp is done any personal
 *          initializations.  Should be called by the agent just before
 *      entering the MainLoop().
 ****************************************************************************/
EXPORT_MSCPP
void EXPORT_BORLAND oaa_ready(int print_ready)
{
  char *tmp1 = NULL;
  char *tmp2 = NULL;
  char *answers = NULL;
  char *readbb = NULL;
  char *lastReadBB = NULL;

  tmp1 = malloc(strlen(KSname) + 40);
  tmp2 = malloc(strlen(KSname) + 40);
  sprintf(tmp1, "[InternalName,  open, Solvables, '%s']", KSname);
  sprintf(tmp2, "[InternalName, ready, Solvables, '%s']", KSname);
  replace_bb("ksdata", tmp1, tmp2);

  read_bb("ksdata", tmp2, &answers);
     
  if (answers && (strcmp(answers, "[]") != 0)) {
      char tmp[2048]; /* debug */
      sprintf(tmp,"ready : answers %s",answers);
      /* win_printf(tmp); */

     list_to_terms(answers);

     NthElt(answers, list_len(answers), &lastReadBB);

     sprintf(tmp,"ready : last bb %s",lastReadBB);
      /* win_printf(tmp); */
     
     Argument(lastReadBB, 2, &readbb);

     list_to_terms(readbb);
     NthElt(readbb, 1, &ksid);
  }

  OAA_FREE(answers);
  OAA_FREE(readbb);
  OAA_FREE(tmp1);
  OAA_FREE(tmp2);
  OAA_FREE(lastReadBB);

  if (user_app_ready)
      user_app_ready();
  if (print_ready) {
#if defined IS_WINDOWS
 win_printf("Ready.\n"); 
#endif

#ifdef IS_UNIX
    printf("Ready.\n");
#endif
 
    fflush(stdout);
  }
}



/*****************************************************************************
* Interpret: handling incoming events
*****************************************************************************/

/****************************************************************************
 * name:    interpret
 * purpose: Executes standard events for all agents, or calls agent-specific
 *          oaa_do_event procedure.
 * inputs:
 *   - char    *KS: KS sending the event to be executed
 *   - char *event: event to be executed
 * outputs:
 *   answers: a list of answers.  Should be "[]" for failure, or a list
 *      containing the event with variables replaced with values.
 *      example:  a query "a(X) could return "[a(1),a(2),a(3)]"
 ****************************************************************************/
void
interpret(char *KS,
          char *event,
          char **answers)
{
   /* Local string variables */
   char *func = NULL;
   char *params = NULL;
   char *args = NULL;
   char *goal = NULL;
   char *s = NULL;
   char *goalanswers = NULL;
   int arity = 0;
   int n;

   append(answers, "[");

   Functor(event, &func, &args);
   arity = list_len(args);

   if ((strcmp(func, "debug_on") == 0) && (arity == 0)) {
      printf("\nDebug on.\n");
      fflush(stdout);
      DebugOn = TRUE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "debug_off") == 0) && (arity == 0)) {
      printf("\nDebug off.\n");
      fflush(stdout);
      DebugOn = FALSE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "trace_on") == 0) && (arity == 0)) {
      printf("\nTrace on.\n");
      fflush(stdout);
      TraceOn = TRUE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "trace_off") == 0) && (arity == 0)) {
      printf("\nTrace off.\n");
      fflush(stdout);
      TraceOn = FALSE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "tcp_trace_on") == 0) && (arity == 0)) {
      printf("\nTCP trace on.\n");
      fflush(stdout);
      TcpTraceOn = TRUE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "tcp_trace_off") == 0) && (arity == 0)) {
      printf("\nTCP trace off.\n");
      fflush(stdout);
      TcpTraceOn = FALSE;
      append(answers, event);   /* Success */
   } else
   if ((strcmp(func, "halt") == 0) && (arity == 0)) {
      printf("\nReceived halt command: exiting...\n");
      fflush(stdout);
      oaa_app_done();
      disconnect();
      exit(0);
   } else
   if ((strcmp(func, "add_trigger") == 0) && (arity == 4)) {
      char *kind = NULL;
      char *type = NULL;
      char *cond = NULL;
      char *action = NULL;
      char *fcond = NULL;
      char *acond = NULL;
      char *ans = NULL;

      NthElt(args, 1, &kind);
      NthElt(args, 2, &type);
      remove_quotes(kind);
      remove_quotes(type);
      if ((strcmp(kind, "test") == 0) && (strcmp(type, "if") == 0)) {
   char *buf = NULL;
   NthElt(args, 3, &cond);
   NthElt(args, 4, &action);
   Functor(cond, &fcond, &acond);

   /* Tell UI: Trigger added */
   buf = malloc(strlen(cond)+strlen(action)+100);
   sprintf(buf, "inform_ui(trigger,trigger_added(%s,%s,%s))",
    type, cond, action);
   solve(buf, "[broadcast]", NULL);
   OAA_FREE(buf);

   /* Execute conditions */
   oaa_do_event(KS, fcond, acond, &ans);
   if (strstr(ans, fcond) != NULL) {

      /* Fire trigger */
      printf("Trigger firing: %s: %s\n", cond, action);
      solve(action, "[broadcast]", NULL);

      /* Tell UI: Trigger firing */
      buf = malloc(strlen(cond)+strlen(action)+100);
      sprintf(buf, "inform_ui(trigger,trigger_fired(%s,%s,%s))",
    type, cond, action);
      solve(buf, "[broadcast]", NULL);
      OAA_FREE(buf);
   }
      }
      else 
  fprintf(stderr,
    "Only test/if triggers currently handled in C agentlib\n");

      OAA_FREE(kind);
      OAA_FREE(type);
      OAA_FREE(action);
      OAA_FREE(cond);
      OAA_FREE(fcond);
      OAA_FREE(acond);
      OAA_FREE(ans);
   } else
   if ((strcmp(func, "solve") == 0) && (arity == 3)) {
      NthElt(args, 2, &goal);    /* Goal */
      NthElt(args, 3, &params);    /* Params */

      push_context_stack(oaa_extract_contexts(params));

      if_trace_on("\n\nAttempting to solve: %s...\n", goal);
      interpret(KS, goal, &goalanswers);

      if (strstr(params, "broadcast") == 0) {
        if (TraceOn) {
          printf("\nSolutions found for %s:\n   %s\n", goal, goalanswers);
          fflush(stdout);
        }
        Argument(event, 1, &func);  /* ID */

   /* In case answers were not returned */
   if (!goalanswers)
      goalanswers = strdup("[]");

   s = malloc(strlen(func)+strlen(KSname)+strlen(goal)+strlen(params)+
        strlen(goalanswers)+20);

   /* Answers should be a list of answers  */
   /* If so, return answers to Facilitator */
   if (is_list(goalanswers)) {
      sprintf(s, "solved(%s,%s,%s,%s,%s)", 
         func, KSname, goal, params, goalanswers);
      post_event(s);
   }
   else
   /* Request to delay the solve until later.  Add to delay_table */
   if (strstr(goalanswers, DELAY_PRED) == goalanswers) {
      /* delay_table will contain solved predicate minus the solutions */
      /* which will be added in return_delayed_solutions() when ready  */
      sprintf(s, "solved(%s,%s,%s,%s,", 
         func, KSname, goal, params);
      n = ArgumentAsInt(goalanswers, 1);
      add_delay_table_entry(n, s);
   }
      }
      else {   /* broadcast */
   if (TraceOn) {
      printf("\nGoal completed (broadcast):\n   %s\n", goal);
      fflush(stdout);
    }
      }
      pop_context_stack();
   } else
   /* If oaa_do_event succeeds, returns new answers */
   if (oaa_do_event(KS, func, args, &s)) {
      OAA_FREE(*answers);
      *answers = strdup(s);  /* Return answers or failure */
      goto free_local_vars;
   }

   append(answers, "]");

free_local_vars:    /* Free local variables */
   OAA_FREE(func);
   OAA_FREE(params);
   OAA_FREE(args);
   OAA_FREE(goal);
   OAA_FREE(goalanswers);
   OAA_FREE(s);

}




#ifdef IS_UNIX /* ---------------------------------------------------------*/
/****************************************************************************
 *  Utility: Expand filenames
 ****************************************************************************/

/****************************************************************************
 * name:    filename_as_directory
 * purpose: concatenate a slash onto a filename if necessary
 * inputs:
 *    - char  *in: file to work on
 *    - char *out: result
 * remarks:
 *    Internal static command 
 ****************************************************************************/
static char *
filename_as_directory (out, in)
     char *out, *in;
{
  int size = strlen (in) - 1;

  strcpy (out, in);

  /* For Unix syntax, Append a slash if necessary */
  if (out[size] != '/')
    strcat (out, "/");
  return out;
}


/****************************************************************************
 * name:    expand_filename
 * purpose: Convert a filename to absolute, and canonicalize it.
 * inputs:
 *   - char *filename: filename to expand
 *   - char *startdir: directory to start with if filename is relative.
 *        If startdir is nil or missing, the current working directory
 *    is used.
 * returns:
 *   The expanded filename in a structure created by malloc.
 * remarks:
 *   Accepts "~", "~username", and all relative paths
 *   Filenames containing . or .. as components are simplified
 ****************************************************************************/
char *
expand_filename(nm, startDir)
   unsigned char *nm;
   unsigned char *startDir;
{
  
  register unsigned char *newdir, *p, *o;
  int tlen;
  unsigned char *target;
  struct passwd *pw;
  int lose;
  
  /* If nm is absolute, flush ...// and detect /./ and /../.
     If no /./ or /../ we can return right away. */
  if (
      nm[0] == '/'
      )
    {
      p = nm;
      lose = 0;
      while (*p)
  {
    if (p[0] == '/' && p[1] == '/')
      nm = p + 1;
    if (p[0] == '/' && p[1] == '~')
      nm = p + 1, lose = 1;
    if (p[0] == '/' && p[1] == '.'
        && (p[2] == '/' || p[2] == 0
      || (p[2] == '.' && (p[3] == '/' || p[3] == 0))))
      lose = 1;
    p++;
  }
      if (!lose)
  {
    return strdup (nm);
  }
    }

  /* Now determine directory to start with and put it in NEWDIR.  */

  newdir = 0;

  if (nm[0] == '~')
    {
      if (nm[1] == '/' || nm[1] == 0)
  {
    /* Handle ~ on its own.  */
    newdir = (unsigned char *) getenv ("HOME");
  }
      else
  {
    /* Handle ~ followed by user name.  */
    unsigned char *user = nm + 1;

    /* Find end of name.  */
    unsigned char *ptr = (unsigned char *) strchr (user, '/');
    int len = ptr ? ptr - user : strlen (user);

    /* Copy the user name into temp storage.  */
    o = (unsigned char *) strdup(user);
    o[len] = 0;

    /* Look up the user name.  */
    pw = (struct passwd *) getpwnam (o);
    if (!pw) {
      fprintf(stderr, "User \"%s\" is not known", o);
      fflush(stderr);
    }
    newdir = (unsigned char *) pw->pw_dir;

    /* Discard the user name from NM.  */
    nm += len;
  }

      /* Discard the ~ from NM.  */
      nm++;

      if (newdir == 0)
  newdir = (unsigned char *) "";
    }

  if (nm[0] != '/' && !newdir)
    {
      if (startDir[0] == 0)
         newdir = (unsigned char *)getenv("PWD");
      else newdir = startDir;
    }

  if (newdir != 0)
    {
      /* Get rid of any slash at the end of newdir.  */
      int length = strlen (newdir);
      if (length > 1 && newdir[length - 1] == '/')
  {
    unsigned char *temp = (unsigned char *) strdup(newdir);
    temp[length - 1] = 0;
    newdir = temp;
  }
      tlen = length + 1;
    }
  else
    tlen = 0;

  /* Now concatenate the directory and name to new space in the stack frame */

  tlen += strlen (nm) + 1;
  target = (unsigned char *) malloc (tlen);
  *target = 0;

  if (newdir)
    {
      if (nm[0] == 0 || nm[0] == '/')
  strcpy (target, newdir);
      else
      filename_as_directory (target, newdir);
    }

  strcat (target, nm);

  /* Now canonicalize by removing /. and /foo/.. if they appear */
  /* This could be done by using realpath(), but realpath only  */
  /* works if the file actually exists.        */

  p = target;
  o = target;

  while (*p)
    {
      if (*p != '/')
   {
    *o++ = *p++;
  }
      else if (!strncmp (p, "//", 2))
  {
    o = target;
    p++;
  }
      else if (p[0] == '/' && p[1] == '.' &&
         (p[2] == '/' || p[2] == 0))
        {
          if (o == target && p[2] == '\0')
            *o++ = *p;
          p += 2;
        }
      else if (!strncmp (p, "/..", 3)
         && o != target
         && (p[3] == '/' || p[3] == 0))
  {
    while (o != target && *--o != '/')
      ;
    if (o == target && *o == '/')
      ++o;
    p += 3;
  }
      else
   {
    *o++ = *p++;
  }
    }

  target[o-target] = 0;

  return (char *)target;
}

#endif  /* --------------------------------------------------------------*/

/****************************************************************************
 * Misc
 ****************************************************************************/

EXPORT_MSCPP void EXPORT_BORLAND special_free(void* ptr) {
 free(ptr);
}

/****************************************************************************
 * Special function for Colbert's Saphira
 ****************************************************************************/
#ifdef IS_SAPHIRA
EXPORT_MSCPP
void EXPORT_BORLAND sfLoadInit(void)
{
#ifdef IS_WINDOWS  /* ---------------------------------------------------- */
  win_printf("Welcome to Saphira");
#endif
}
#endif


