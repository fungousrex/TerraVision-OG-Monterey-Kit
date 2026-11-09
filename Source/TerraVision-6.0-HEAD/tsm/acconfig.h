#ifndef _config_h_
#define _config_h_

@TOP@

/* These are necessary for automake.  */
#undef PACKAGE
#undef VERSION

/* Define if we have libwww software available. */
#undef USE_HTTP

/* Define if we are using the DPSS (version 2.4 or greater). */
#undef DPSS

/* Define to enable tracing calls. */
#undef TRACING

/* Define to enable function inlining. */
#undef HAVE_INLINE

/* Define if DPSS libs have funcs to do continuation loads */
#undef DPSS_CONT_LOAD

/* We may need these to access thread-safe library functions. */
#undef _REENTRANT
#undef _THREAD_SAFE

@BOTTOM@

/* Don't enable NSPR back-compatibility */
#define NO_NSPR_10_SUPPORT (1)

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

#endif
