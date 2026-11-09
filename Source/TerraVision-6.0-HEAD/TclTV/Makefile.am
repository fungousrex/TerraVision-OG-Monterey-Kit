#  TclTV: A Tcl/Tk front-end to the TerraVision 3D 
#         terrain visualization system.
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.
#

bin_PROGRAMS = TclTV

TclTV_SOURCES = main.c tsGeoPyramidCmds.c tvBaseCmds.c tvViewCmds.c \
	tvPathCmds.c tsmDatabaseCmds.c tvLinkCmds.c pthrCmds.c \
	tsmUtilCmds.c

EXTRA_HEADERS = tsGeoPyramidCmds.h tvBaseCmds.h tsmCmds.h \
	tvLinkCmds.h pthrCmds.h

CXXLINK = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
LINK = $(CXXLINK)

TclTV_DEPENDENCIES = ../TerraVision/libtv2.a ../tvutils/libtvutils.a @DEPS@
TclTV_LDADD = -L../TerraVision -L../tvutils -ltvutils -ltv2 

PURIFYOPTIONS = -follow-child-processes=yes -threads=yes -chain-length=12
TclTV.pure: $(TclTV_OBJECTS) $(TclTV_DEPENDENCIES)
	@rm -f TclTV.pure
	$(PURIFY) $(PURIFYOPTIONS) $(LINK) $(TclTV_LDFLAGS) $(TclTV_OBJECTS) $(TclTV_LDADD) $(LIBS)
