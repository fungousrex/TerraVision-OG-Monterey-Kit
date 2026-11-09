/* Define to enable function inlining. */
#undef HAVE_INLINE

/* Define to indicate that X11 is available. */
#undef HAVE_X11

/* Define to enable tracing calls. */
#undef TRACING

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
