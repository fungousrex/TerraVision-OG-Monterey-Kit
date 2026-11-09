/*
 * Name: report.h
 *
 * Description: 
 *   Part of the pthr library: a collection of thread-safe functions
 *   and datatypes.
 *
 * Revision History:
 *   $Id: report.h,v 1.5 2000/08/29 00:31:35 reddy Exp $
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

#ifndef _pthr_report_h_
#define _pthr_report_h_

#include <prthread.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void pthr_print_report (FILE *fp, char const *buf, int newline);

extern void pthr_report_start (void);
extern void pthr_report_end (void);

extern void pthr_report (char const *fmt, ...);
extern void pthr_report1 (char const *prefix, char const *fmt, ...);

extern void pthr_vreport (char const *fmt, va_list ap);
extern void pthr_vreport1 (char const *prefix, char const *fmt, va_list ap);

extern void pthr_error (char const *fmt, ...);
extern void pthr_error1 (char const *prefix, char const *fmt, ...);

extern void pthr_verror (char const *fmt, va_list ap);
extern void pthr_verror1 (char const *prefix, char const *fmt, va_list ap);

extern void         pthr_set_name (void const *ptr, char const *name);
extern char const * pthr_name (void const *ptr);

extern int pthr_create (PRThread **new_thread_id,
			char const *name, 
			void (*start_func) (void*),
			void *arg);

extern void pthr_cancelall (void);

#ifdef __cplusplus
}
#endif

#endif /* _pthr_report_h_ */
