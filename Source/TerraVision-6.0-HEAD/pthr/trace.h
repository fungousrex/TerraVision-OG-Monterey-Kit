/*
 * Name: trace.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: trace.h,v 1.11.2.1 2001/07/10 06:54:26 leei Exp $
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

#ifndef _pthr_trace_h_
#define _pthr_trace_h_

#include "report.h"

#ifdef TRACING

#ifdef __GNUC__
#define FUNCNAME(x) static const char funcName[] = __PRETTY_FUNCTION__
#else
#define FUNCNAME(x) static const char funcName[] = x
#endif

#define PTRACEP(x) (pthr_tracep (x))
#define PTRACE(x) (pthr_trace_bylevel x, 1)
#define PTRINC(x) (pthr_trace_indent x)

#else /* !TRACING */

#define FUNCNAME(x) static int funcName

#define PTRACEP(x) (0)
#define PTRACE(x) (0)
#define PTRINC(x) (0)

#endif /* TRACING */

#define PTRACE_MASK (1<<3)

#ifdef __cplusplus
extern "C" {
#endif

extern void   pthr_trace (char const *fmt, ...);
#ifdef __GNUC__
#define pthr_trace_bylevel(l,args...) \
  (pthr_tracep (l) ? pthr_trace1(args) : 0)
#else
extern void   pthr_trace_bylevel (int level, char const *prefix, 
				  char const *fmt, ...);
#endif

extern void   pthr_vtrace (char const *fmt, va_list ap);
extern void   pthr_vtrace_bylevel (int level, char const *prefix, 
				   char const *fmt, va_list ap);

extern int    pthr_tracep (int level);
extern void   pthr_trace1 (char const *prefix, char const *fmt, ...);
extern int    pthr_trace_level (int level);
extern int    pthr_trace_mask (int mask);
extern int    pthr_trace_indent1 (int inc);
extern int    pthr_trace_indent (int level, int inc);
extern int    pthr_trace_print_pid (int pid);
extern int    pthr_trace_html (int html);
extern FILE * pthr_trace_stream (FILE *stream);

#if (defined(__cplusplus)||defined(HAVE_INLINE)) && !defined(NO_trace_INLINE)
# if defined(__GNUC__)
#  define exinline extern inline
# elif ( defined(WIN32) && defined(MSVC) )
#  define exinline __inline
# else
#  define exinline inline
# endif
# include "trace_inline.h"
# undef exinline
#endif /* HAVE_INLINE */

#ifdef __cplusplus
}
#endif

#endif /* _pthr_trace_h_ */
