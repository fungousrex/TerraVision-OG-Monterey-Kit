#  TerraVision: A 3D terrain visualization system.
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.
#

bin_PROGRAMS = TerraVision

TerraVision_SOURCES = \
	AboutDialog.c Dialogs.c BusyDialog.c Database.c \
	DatabaseOpts.c DatabaseAdv.c Datasets.c \
        ScannedMap.c Link.c \
        TerraVision.c TerraVisionCAVE.c \
	NetLogger.c \
	FeaturesFE.c Prefs.c PathsFE.c \
	ThreeDControl.c ThreeDPanel.c ThreeDWidget.c \
	Viewpoint.c TVLogger.c Info.c \
	resource.c pixmaps.c

EXTRA_HEADERS = \
	AboutDialog.h Dialogs.h BusyDialog.h Database.h \
	PathsFE.h Datasets.h \
	ScannedMap.h TerraVision.ad.h TerraVisionGlobals.h \
	TerraVisionInc.h ThreeDControl.h ThreeDPanel.h \
	FeaturesFE.h Prefs.h \
	ThreeDWidget.h ThreeDWidgetI.h \
	X11FrameGrab.h X11Include.h \
	Viewpoint.h NetLogger.h TVLogger.h Info.h \
	pixmaps.h 

$(srcdir)/TerraVision.ad.h: $(srcdir)/TerraVision.ad
	cd $(srcdir); ./ad2c TerraVision.ad > TerraVision.ad.h

TerraVision_DEPENDENCIES = ../TerraVision/libtv2.a \
                           ../tvutils/libtvutils.a @DEPS@
TerraVision_LDADD = -L../TerraVision -L../tvutils -ltvutils -ltv2

INCLUDES = -I$(srcdir)/bitmaps

CXXLINK = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
LINK = $(CXXLINK)

PURIFYOPTIONS = -follow-child-processes=yes -threads=yes -chain-length=12 -max-threads=64
TerraVision.pure: $(TerraVision_OBJECTS) $(TerraVision_DEPENDENCIES)
	@rm -f TerraVision.pure
	$(PURIFY) $(PURIFYOPTIONS) $(LINK) $(TerraVision_LDFLAGS) $(TerraVision_OBJECTS) $(TerraVision_LDADD) $(LIBS)
