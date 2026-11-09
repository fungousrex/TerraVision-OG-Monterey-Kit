#ifndef _config_h_
#define _config_h_

@TOP@

/* These are necessary for automake.  */
#undef PACKAGE
#undef VERSION

/* Define if we are using the DPSS */
#undef DPSS

/* Define to enable function inlining. */
#undef HAVE_INLINE

/* Define to enable tracing calls. */
#undef TRACING

@BOTTOM@

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# ifndef alloca
#  define alloca __builtin_alloca
# endif
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#endif
