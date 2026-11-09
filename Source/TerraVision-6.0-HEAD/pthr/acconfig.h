/* Describe whether or not to try inlining. */
#undef HAVE_INLINE

/* Can we do static initialisations with stdout/stderr? */
#undef HAVE_STATIC_INIT

/* We may need these to access thread-safe library functions. */
#undef _REENTRANT
#undef _THREAD_SAFE

/* Define to enable tracing calls. */
#undef TRACING

@BOTTOM@

/* Don't enable NSPR back-compatibility */
#define NO_NSPR_10_SUPPORT (1)

/* AIX requires this to be the first thing in the file.  */
#ifdef __GNUC__
# define alloca __builtin_alloca
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
