/* tsmTrace.h.  Generated automatically by configure.  */

/* ___________________Tile Set Manager (TSM)______________________
   
   <FILE-ID>
   :FILE-NAME "~magic/software/leclerc/tsmApi/tsmTrace.h"
   :TIME-STAMP "97/04/22 13:46:43 leclerc"
   :DISTRIBUTION-CATEGORY :PARTIAL-SOURCES-1
   :RCS-ID "$Id: tsmTrace.h,v 1.1.1.1 2002/10/31 05:43:36 martinreddy Exp $"
   :COMMENT "TSM Trace utility prototypes"
   </FILE-ID>
   ____________________________________________________ __    __
   Yvan Leclerc                                         \ \  / /
                                                         \ \/ /
   leclerc@ai.sri.com   http://www.ai.sri.com/~leclerc/      /
   Tel: (415) 859-6153              Fax: (415) 859-3735     /  ___
   ____________________________________________________    /____*/

#ifndef TSMTRACE_H
#define TSMTRACE_H

#include <stdio.h>

char *tsmApiReleaseOnly (void);
char *tsmApiReleaseDateOnly (void);
void tsmPrintReleaseInfo (FILE *stream);

#ifdef TRACING

# include <pthr/trace.h>

# define tsmError pthr_error1
# define tsmStatus pthr_report

# define TSMTRACE PTRACE
# define TSMTRINC PTRINC
# define TSMTRACEP PTRACEP
# define TSMMASK (PTRACE_MASK)

# define tsmSetTraceLevel pthr_trace_level
# define tsmSetHtmlFlag pthr_trace_html
# define tsmSetPrintPidFlag pthr_trace_print_pid

#else

# define TSMTRACEP(x) (0)
# define TSMTRACE(x) (0)
# define TSMTRINC(x) (0)
# define TSMMASK (0)

# define tsmSetTraceLevel(l) (l)
# define tsmSetHtmlFlag(h) (h)
# define tsmSetPrintPidFlag(p) (p)

#endif

#endif
