#ifndef _config_h_
#define _config_h_

@TOP@

/* These are necessary for automake.  */
#undef PACKAGE
#undef VERSION

/* Describe whether or not to try inlining. */
#undef HAVE_INLINE

/* Define if we have the GL Utility Library (glut). */
#undef HAVE_GLUT

/* Define if we have the OpenGL Library (gl). */
#undef HAVE_OPENGL

/* Enable debugging in parse. */
#undef YYDEBUG

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

#endif /* _config_h_ */
