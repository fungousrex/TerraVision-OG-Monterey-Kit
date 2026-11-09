#  TerraVision: A 3D terrain visualization system.
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.
#

bin_PROGRAMS = TerraVision

if HAVE_X11
PLATFORM = X
else
PLATFORM = win
endif

TerraVision_SOURCES = TerraVision.c PathsFE.c ThreeDWidget.c \
	glmTiming.c Viewpoint.c 

EXTRA_TerraVision_SOURCES = Platform_X.c Platform_win.c

EXTRA_HEADERS = PathsFE.h TerraVisionInc.h ThreeDWidget.h \
        ThreeDWidgetI.h Viewpoint.h Platform_$(PLATFORM).h

CXXLINK = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
LINK = $(CXXLINK)

TerraVision_DEPENDENCIES = Platform_$(PLATFORM).o \
	../TerraVision/libtv2.a ../tvutils/libtvutils.a \
        @DEPS@
TerraVision_LDADD = Platform_$(PLATFORM).o \
   -L../TerraVision -L../tvutils -ltvutils -ltv2

PURIFYOPTIONS = -follow-child-processes=yes -threads=yes -chain-length=12
TerraVision.pure: $(TerraVision_OBJECTS) $(TerraVision_DEPENDENCIES)
	@rm -f TerraVision.pure
	$(PURIFY) $(PURIFYOPTIONS) $(LINK) $(TerraVision_LDFLAGS) $(TerraVision_OBJECTS) $(TerraVision_LDADD) $(LIBS)

if HAVE_X11
EXTRA_PROGRAMS = testX

testX_SOURCES = testX.c glmTiming.c
testX_DEPENDENCIES = @DEPS@
testX_LDADD = -lglt -lpthrutil
endif
