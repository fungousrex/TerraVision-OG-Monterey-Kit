/*
 * Name: report.c
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: report.c,v 1.14.2.2 2001/11/02 22:46:29 reddy Exp $
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

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if !( defined(WIN32) && defined(MSVC) )
#include <unistd.h>
#endif

#include <prthread.h>
#include <prlock.h>
#include <prinit.h>
#include <errno.h>

#include "report.h"
#include "trace.h"

#include "html_gen.h"

int ptrace_mask = 0;
int ptrace_level = 0;

static PRCallOnceType init;
static PRUintn trace_key;

static int trace_pid = 1;
static int trace_html = 0;

#ifdef HAVE_STATIC_INIT
static FILE *trace_stream = stderr;
#else
static FILE *trace_stream = NULL;
#endif

static char *html_start = "\n<pre>";
static char *html_end = "</pre>\n";

typedef struct {
  int indent;
} trace_params;

static PRLock *lock;

PRStatus PR_CALLBACK
report_init (void)
{
  PR_NewThreadPrivateIndex (&trace_key, NULL);

  /* Is this the appropriate place for this?  --RLB */
  lock = PR_NewLock();

  return PR_SUCCESS;
}

void
pthr_report_start (void)
{
    PR_Lock (lock);
}

void
pthr_report_end (void)
{
    PR_Unlock (lock);
}

static trace_params *
get_trace_params (void)
{
  trace_params *tr = (trace_params *) PR_GetThreadPrivate (trace_key);
  if (! tr) {
    tr = (trace_params *) calloc (1, sizeof (*tr));
    PR_SetThreadPrivate (trace_key, (void*) tr);
  }
  return tr;
}

void
pthr_print_report (FILE *fp, char const *buf, int newline)
{
  static int last_newline = 1;

  if ( ! fp ) fp = stderr;  /* initial stream can be NULL - see above */

  pthr_report_start (); {
    if (newline && !last_newline) putc ('\n', fp);
    fputs (buf, fp);
    if (newline) putc ('\n', fp);
    fflush (fp);
    last_newline = newline;
  }; pthr_report_end ();
}

static void 
vtrace (char const *prefix, char const *name, char const *fmt, va_list ap)
{
  if (strlen (fmt) == 1) {
    pthr_print_report (trace_stream, fmt, 0);
  } else {
    char buf[1024], *p = buf;
    trace_params *tr = (trace_params *) get_trace_params ();
    int indent = tr->indent;

    if (trace_html) {
      strcpy (p, html_start);
      p = strchr (p, '\0');
    }
    if (trace_pid) {
      if (name) {
	strcpy (p, name);
	p = strchr (p, '\0');
	*p++ = ' ';
      }
#ifdef _pthr_sproc_h_
      sprintf (p, "[%d]: ", (int) getpid ());
#else
      sprintf (p, "[%p]: ", (void*) PR_GetCurrentThread ());
#endif
      p = strchr (p, '\0');
    }
    for ( ; indent > 0; --indent) *p++ = ' ';
    if (prefix) {
      strcpy (p, prefix);
      p = strchr (p, '\0');
      *p++ = ' ';
    }
    vsprintf (p, fmt, ap);
    p = strchr (p, '\0');
    if (trace_html) {
      strcpy (p, html_end);
      p = strchr (p, '\0');
    }

    if (p >= buf+1024) abort ();

    pthr_print_report (trace_stream, buf, 1);
  }
}

static void 
trace (char const *prefix, char const *name, char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vtrace (prefix, name, fmt, ap);
  va_end (ap);
}

void
pthr_vtrace (char const *fmt, va_list ap)
{
  char const *name = 0;

  if (trace_pid)
    name = pthr_name ((void const *) PR_GetCurrentThread ());

  PR_CallOnce (&init, report_init);  

  vtrace (0, name, fmt, ap);
}

void
pthr_trace (char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  pthr_vtrace (fmt, ap);
  va_end (ap);
}

void
pthr_vtrace_bylevel (int level, char const *prefix,
		     char const *fmt, va_list ap)
{
  if (pthr_tracep (level)) {
    char const *name = 0;

    if (trace_pid)
      name = pthr_name ((void const *) PR_GetCurrentThread ());

    PR_CallOnce (&init, report_init);
  
    vtrace (prefix, name, fmt, ap);
  }
}

void
pthr_vtrace1 (char const *prefix, char const *fmt, va_list ap)
{
  char const *name = 0;

  if (trace_pid)
    name = pthr_name ((void const *) PR_GetCurrentThread ());

  PR_CallOnce (&init, report_init);

  vtrace (prefix, name, fmt, ap);
}

void
pthr_trace1 (char const *prefix, char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  pthr_vtrace1 (prefix, fmt, ap);
  va_end (ap);
}

#undef pthr_trace_bylevel
void
pthr_trace_bylevel (int level, char const *prefix, char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  pthr_vtrace_bylevel (level, prefix, fmt, ap);
  va_end (ap);
}

void
pthr_vreport1 (char const *prefix, char const *fmt, va_list ap)
{
  char buf[1024], *p = buf;

  PR_CallOnce (&init, report_init);
  
  if (prefix) {
    strcpy (p, prefix); p = strchr (p, '\0');
    *p++ = ' ';
  }
  vsprintf (p, fmt, ap);
  p = strchr (p, '\0');

  if (p >= buf+1024) abort ();

  pthr_print_report (stdout, buf, 1);
}

void
pthr_report1 (char const *prefix, char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  pthr_vreport1 (prefix, fmt, ap);
  va_end (ap);
}

void
pthr_vreport (char const *fmt, va_list ap)
{
  pthr_vreport1 (0, fmt, ap);
}

void
pthr_report (char const *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  pthr_vreport (fmt, ap);
  va_end (ap);
}

void
pthr_verror (char const *fmt, va_list ap)
{
  char buf[1024], *p;

  PR_CallOnce (&init, report_init);
  
  strcpy (buf, "ERROR: ");
  p = strchr (buf, '\0');
  vsprintf (p, fmt, ap);

  if (p >= buf+1024) abort ();

  pthr_print_report (stderr, buf, 1);
}

void
pthr_error (char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  pthr_verror (fmt, ap);
  va_end (ap);
}

void
pthr_verror1 (char const *prefix, char const *fmt, va_list ap)
{
  char buf[1024], *p;

  PR_CallOnce (&init, report_init);
  
  strcpy (buf, "ERROR: ");
  p = strchr (buf, '\0');
  strcpy (p, prefix);
  p = strchr (p, '\0');
  *p++ = ' ';

  vsprintf (p, fmt, ap);

  if (p >= buf+1024) abort ();

  pthr_print_report (stderr, buf, 1);
}

void
pthr_error1 (char const *prefix, char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  pthr_verror1 (prefix, fmt, ap);
  va_end (ap);
}

int
pthr_trace_indent1 (int inc)
{
  int old_indent;
  trace_params *tr;

  PR_CallOnce (&init, report_init);
  
  tr = (trace_params *) get_trace_params ();
  old_indent = tr->indent;
  tr->indent += inc;

  return old_indent;
}

int
pthr_trace_print_pid (int pid)
{
  int old_pid = trace_pid;
  trace_pid = pid;
  return old_pid;
}

int
pthr_trace_html (int html)
{
  int old_html = trace_html;
  trace_html = html;
  return old_html;
}

FILE *
pthr_trace_stream (FILE *stream)
{
  FILE *old_stream = trace_stream;
  if (old_stream) fflush (old_stream);
  if (stream) {
    trace_stream = stream;
#ifdef HAVE_SETLINEBUF
    setlinebuf (trace_stream);
#endif
  }
  if (trace_stream != old_stream && trace_html)
    html__set_html_stream (stream);
  return old_stream;
}

typedef struct {
  void const *ptr;
  char const *name;
} name_assoc;

static int names_len, names_n;
static name_assoc *names_table;

static PRLock *names_lock;
static PRCallOnceType names_init;

PRStatus PR_CALLBACK
name_init (void)
{
  /* Is here the appropriate place for initializing the lock? --RLB */
  names_lock = PR_NewLock();

  names_len = 47;
  names_table = (name_assoc *) calloc (names_len, sizeof (name_assoc));

  return PR_SUCCESS;
}

static int
ptr_hash (void const *ptr)
{
  unsigned long val = (unsigned long) ptr;
  val >>= 2;
  return 0x7fffffff & (val + ((val << 5) ^ 0x0f0f0f0f));
}

static name_assoc *
name_slot (void const *ptr)
{
  name_assoc *table_end;
  name_assoc *slot;

  /* Expand table if necessary. */
  if (names_n > names_len/2) {
    int new_len = names_len * 2 + 1;
    name_assoc *new_table = (name_assoc *)
      calloc (new_len, sizeof (name_assoc));
    name_assoc *old_table = names_table, *old_end = names_table + names_len;

    names_table = new_table;
    names_len = new_len;

    for (slot = old_table; slot < old_end; ++slot) {
      if (slot->name) {
	name_assoc *new_slot = name_slot (slot->ptr);
	new_slot->ptr = slot->ptr;
	new_slot->name = slot->name;
      }
    }
    free (old_table);
  } 

  table_end = names_table + names_len;
  slot = names_table + (ptr_hash (ptr) % names_len);
  while (slot->ptr != ptr && slot->name) {
    if (++slot >= table_end) slot = names_table;
  }

  return slot;
}

void
pthr_set_name (void const *ptr, char const *name)
{
#ifdef _pthr_sproc_h_
  if (pthread_is_pthread (ptr)) {
    pthread_setname ((pthread_t) ptr, name);
    return;
  }
#endif

  {
    int len = strlen (name) + 1;
    char *ptr_name = (char *) malloc (len);

    memcpy (ptr_name, name, len);

    if (! names_table) PR_CallOnce (&names_init, name_init);

    PR_Lock (names_lock); {
      name_assoc *slot = name_slot (ptr);
      if (slot->name) {
	free ( (void *) slot->name);
      }
      slot->name = ptr_name;
      if (name) {
	slot->ptr = ptr;
	++names_n;
      } else {
	name_assoc *table_end = names_table + names_len;
	/* May need to reposition entries after slot. */
	while (1) {
	  name_assoc *new_slot;
	  if (++slot >= table_end) slot = names_table;
	  if (! slot->name) break;
	  new_slot = names_table + (ptr_hash (slot->ptr) % names_len);
	  if (new_slot != slot) {
	    while (new_slot->name)
	      if (++new_slot >= table_end) new_slot = names_table;
	    if (new_slot != slot) {
	      new_slot->ptr = slot->ptr;
	      new_slot->name = slot->name;
	      slot->ptr = 0;
	      slot->name = 0;
	    }
	  }
	}
	--names_n;
      }
    }; PR_Unlock (names_lock);
  }
}

static char names[16][12];
static int name_idx = 0;

char const *
pthr_name (void const *ptr)
{
#ifdef _pthr_sproc_h_
  if (ptr && pthread_is_pthread (ptr))
    return pthread_name ((pthread_t) ptr);
#endif

  {
    char const *name;
    int old_level = pthr_trace_level (0);

    if (! names_table) PR_CallOnce (&names_init, name_init);

    PR_Lock (names_lock); {
      name_assoc *slot = name_slot (ptr);
      name = slot->name;
      if (! name) {
	sprintf (names[name_idx], "%p", ptr);
	name = names[name_idx];
	name_idx = (name_idx+1)%16;
      }
    }; PR_Unlock (names_lock);

    pthr_trace_level (old_level);

    return name;
  }
}

#include <errno.h>

int
pthr_create (PRThread **new_thread_id, 
             char const *name,
             void (*start_func) (void*),
             void *arg)
{
  *new_thread_id =
    PR_CreateThread(PR_USER_THREAD, start_func, arg,
		    PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD,
		    PR_JOINABLE_THREAD, 1<<16);

  if (! *new_thread_id) {
    return EAGAIN;
  }

  pthr_set_name ((void const *) *new_thread_id, name);

  return 0;
}
