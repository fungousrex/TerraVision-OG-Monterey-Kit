#  TerraVision: A 3D terrain visualization system.
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.

lib_LIBRARIES = libglt.a

if HAVE_X11
GLMX_C = glmXt.c glmX.c
endif

if HAVE_WINDOWS
GLMW_C = glmWin.c
endif

# if HAVE_TCL
GLMT_C = glmTcl.c
# endif

GLM_C = $(GLMX_C) $(GLMW_C) $(GLMT_C)

libglt_a_SOURCES = glt.c glt_inline.c glt_state.c glmil.c glm.c $(GLM_C)
pkginclude_HEADERS = glt.h glt_inline.h glm.h glmil.h glmimpl.h
