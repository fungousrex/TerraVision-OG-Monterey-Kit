/* File	   : agentlib.h
   Author  : Adam Cheyer
   Purpose : Header file for C library for the Open Agent Architecture
   Updated : 1/16/95

   Copyright (C) 1995, SRI International.  All rights reserved.
*/

/****************************************************************************
 * Remarks:
 * If IS_UNIX is defined (using -DIS_UNIX in makefile), the
 *                      agent library will be made for UNIX machines.
 * If XWINDOWS is defined (using -DXWINDOWS in makefile), the
 *    agent library will define a get_event-interpret loop usable from
 *    XWindows programs (oaa_timer).
 *
 * If IS_WINDOWS is defined, the agent library will be made as an MSWindows DLL
 *    (agentlib.dll).
 *   In addition, if you make the library using the Borland Environment,
 *      please define the IS_BORLAND option. 
 *   If you make the library with the Microsoft Visual C++ environment, 
 *      please define the IS_MSCPP option.
 *
 * By default, if IS_WINDOWS is not defined, IS_UNIX is automatically
 * defined.
 ****************************************************************************/

#ifndef _AGENTLIB_H_INCLUDED
#define _AGENTLIB_H_INCLUDED

#define IS_CONSOLE

#ifdef WIN32
# define IS_WINDOWS
# include <windows.h>
#else
# define IS_UNIX
#endif

/* TF/DG Fri Mar 14 1997 */
#if defined(__cplusplus)
  extern "C" {
#endif /* __cplusplus */

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef IS_DLL
#define EXTERN __declspec(dllexport)
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Communication layer
 ****************************************************************************/
#include <TerraVision/tcp.h>

/****************************************************************************
 * Registration primitives
 ****************************************************************************/
#define OAA_APP_DONE 1
#define OAA_APP_IDLE 2
#define OAA_DO_EVENT 3
#define OAA_READY 4
#define OAA_DISCONNECTED 5

/* EXTERN int oaa_register(int function_id,int (*user_function)()); */
EXTERN int oaa_register(int function_id,int (*user_function)(char*, char*, char*, char**));

/****************************************************************************
 * Open Agent Architecture primitives
 ****************************************************************************/
#if defined (IS_WINDOWS)
 EXTERN void TestDll();
#endif

#if defined (IS_WINDOWS) && !defined (IS_CONSOLE)

 EXTERN int setup_communication(HWND main_window,
                                char *KS, char *Host, int Port,
                                char *Solvable, int ServerQP32);

#endif

#if defined (IS_UNIX) || defined (IS_CONSOLE)  /* ------------------- */
 EXTERN void TestDll();
 EXTERN int setup_communication(char *KSName, char *Host, int Port,
		                char *Solvable, int ServerQP32);
#endif

EXTERN void get_agent_id(char **ask_ksid, char **ask_ksname);
EXTERN void disconnect();
EXTERN int is_connected();
EXTERN void read_env_vars(char **Host, int *Port, int *ServerType);
EXTERN void read_setup_file(char **Host, int *Port);

EXTERN int get_event(OAASOCKET inputStream,char *KS,char **event); /*SOCKET*/

#ifndef IS_WINDOWS
#ifndef IS_UNIX
#define IS_UNIX
#endif
#endif

EXTERN void set_timeout(double T, int TMode);
EXTERN void post_event(char *Event);
EXTERN void solve(char *Goal, char *Params, char **Answers);
EXTERN void send_data(char *S);
EXTERN void write_bb(char *Item, char *Data);
EXTERN void write_replace_bb(char *Item, char *Data);
EXTERN void write_once_bb(char *Item, char *Data);
EXTERN void retract_bb( char *Item, char *Data);
EXTERN void replace_bb(char *Item, char *OldData, char *NewData);
EXTERN void read_bb(char *Item, char *Data, char **answers);
EXTERN void can_solve(char *goal, char **answers);
EXTERN void interpret(char *KS, char *event, char **answers);
EXTERN void add_trigger(char *Kind, char *Type,char *Condition, char *Action);
EXTERN void add_trigger_to_ks(char *KS, char *Kind, char *Type, 
			      char *Condition, char *Action);
EXTERN void add_remote_trigger(char *Kind, char *Type, char *OpMask, 
		char *Template, char *MoreConditions, char *Action);
EXTERN int oaa_process_all_events();
EXTERN void oaa_loop(int print_ready);
EXTERN void oaa_ready(int print_ready);

/* ------------------------------------------------------- */
#ifdef IS_UNIX
#ifdef XWINDOWS	

#include "X11/Intrinsic.h"   /* XtAppContext type */

EXTERN void oaa_timer(XtAppContext app_ctxt);
EXTERN void handle_incoming_event(OAASOCKET inputStream);

#endif
#endif

#ifdef IS_WINDOWS
EXTERN void handle_incoming_event(LPARAM lParam);
#endif


/****************************************************************************
 * Special function for Colbert's Saphira
 ****************************************************************************/
#ifdef IS_SAPHIRA
EXTERN void sfLoadInit(void);
#endif

 /* ------------------------------------------------------- */

/****************************************************************************
 * Dynamic arrays and arrays of strings (strlist)
 ****************************************************************************/
/* Type: A dynamic array */
struct dyn_array {
   int count;
   int allocated;
   char **item;
};

EXTERN void init_dyn_array(struct dyn_array *da);
EXTERN void free_strlist(struct dyn_array *slist);
EXTERN void append_strlist(struct dyn_array *slist, char *str);
EXTERN int  member_strlist(char *elt, struct dyn_array strlist, 
			   int incr, int case_ins);

/****************************************************************************
 * Prolog utility predicates
 ****************************************************************************/
EXTERN void Functor(char *predicate, char **func, char **args);
EXTERN int Term(char *terms, char **aterm, char **restterms);
EXTERN void Argument(char *predicate, int n, char **argument);
EXTERN int  ArgumentAsInt(char *predicate, int n);
EXTERN float OaaFloat(char *arg);
EXTERN void NthElt(char *list, int n, char **elt);
EXTERN int  is_atom(char *term);
EXTERN int  is_list(char *term);
EXTERN int  is_var(char *t);
EXTERN int  is_int(char *t);
EXTERN int  list_len(char *list);
EXTERN void list_to_terms(char *s);
EXTERN void remove_quotes(char *s);
EXTERN const char *double_quotes(char *s);
EXTERN void undouble_quotes(char *s);
EXTERN void trim(char *s);
EXTERN int param_value(char const *aList, char *param, char **value);

EXTERN void rename_vars(char *term, char **new_term);
EXTERN int match_terms(char *t1, char *t2, struct dyn_array *vars);
EXTERN void deref_term(char *term, char **answer, struct dyn_array vars);
EXTERN int unify(char *term1, char *term2, char **answer);
EXTERN void group_to_terms(char *s);

/****************************************************************************
 * Delayed events
 ****************************************************************************/
EXTERN void free_delay_table_entry(int table_entry);
EXTERN void clean_delay_table();
EXTERN void print_delay_table();
EXTERN int delay_solution(char **answers);
EXTERN char *delayed_event(int table_entry);
EXTERN void return_delayed_solutions(int table_entry, char *answers);

EXTERN void oaa_GetCurrentContexts(char **ctx, int table_entry);
EXTERN int oaa_InContext(char *ctx, int table_entry);
EXTERN char *oaa_AddContextParams(char *contexts, char *params);
EXTERN char *oaa_AddDelayedContextParams(int table_entry, char *params);


/****************************************************************************
 * Miscellaneous predicates and variables
 ****************************************************************************/
EXTERN void append(char **str1, char *str2);
EXTERN void error_out(char *ErrMsg);
EXTERN void if_trace_on(char *msg, char *arg);
EXTERN char *expand_filename(unsigned char *Filename, unsigned char *startdir);

EXTERN int     TcpTraceOn;
EXTERN int     TraceOn;
EXTERN int     DebugOn;

#ifdef XWINDOWS	/* ------------------------------------------------------- */

EXTERN void processXevents();

#endif		/* ------------------------------------------------------- */

#ifdef IS_WINDOWS
EXTERN void processWindowsevents();
#endif


/****************************************************************************
 * To be defined by agent application!
 ****************************************************************************/
EXTERN int  do_event(char *KS, char *func, char *args, char **answers);
EXTERN void idle();
EXTERN void app_done();
EXTERN void app_init(); 


/****************************************************************************
* name:    OAA_FREE
* purpose: free a dynamic variable and set it to NULL afterwards
* inputs:
*   - int A: a variable to deallocate
* remarks:
*   OAA_FREE() is better than free() because it checks that you will
*     never attempt to free a nil variable, and you will never
*     try to free the same variable twice, which can lead to BIG problems!
*   OAA_FREE() is defined as a macro so that it is not necessary to pass
*     in the address of the variable you want to free, which is unnatural.
****************************************************************************/
#define OAA_FREE(A) if (A) { free(A); A = NULL; }


/**** Max size in characters of a KS id ****/
#define KS_SIZE 80

/* TF/DG Fri Mar 14 1997 */
#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* AGENTLIB_H_INCLUDED */


