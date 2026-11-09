#
#  tvutils: TerraVision utilities library
#
#  Copyright SRI International 2000
#
#  All rights reserved.
#

lib_LIBRARIES = libtvutils.a

libtvutils_a_SOURCES = \
	ADL.c \
	Args.c \
	Config.c \
	Gazetteer.c \
	Graphics.c \
	Init.c \
	Values.c \
	Viewpoint.c

include_HEADERS = tvutils.h

EXTRA_HEADERS = \
	Args.h \
	Config.h \
	Init.h \
	Platform.h \
	Viewpoint.h

EXTRA_PROGRAMS = test_tvu test_gaz test_adl

TEST_LIBDIRS = -L../TerraVision -L../libGeo_Core -L../libGeo_DiscClient -L.
TEST_LIBS    = -ltvutils -ltv2 -lGeoDisc -lGeo -lghttp

test_tvu_SOURCES = test_tvu.c
test_tvu_DEPENDENCIES = ../TerraVision/libtv2.a ./libtvutils.a @DEPS@
test_tvu_LDADD = $(TEST_LIBDIRS) $(TEST_LIBS)

test_gaz_SOURCES = test_gaz.c
test_gaz_DEPENDENCIES = ../TerraVision/libtv2.a ./libtvutils.a @DEPS@
test_gaz_LDADD = $(TEST_LIBDIRS) $(TEST_LIBS)

test_adl_SOURCES = test_adl.c
test_adl_DEPENDENCIES = ../TerraVision/libtv2.a ./libtvutils.a @DEPS@
test_adl_LDADD = $(TEST_LIBDIRS) $(TEST_LIBS)

CXXLINK = $(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@
LINK = $(CXXLINK)

