#  TerraVision: A 3D terrain visualization system.
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.
#

lib_LIBRARIES = libtv2.a

libtv2_a_SOURCES = \
	Texture.c TextureInline.c \
	TsCache.c TsCacheInline.c \
	TsCoords.c TsCoordsInline.c \
	TsGeoPyramid_OI.c TsGeoPyramid.c TsGeoPyramidInline.c \
	TVGLX.c TsLogging.c NetLogCore.c DotGeo.c \
	VRML.c GeoVRML.c Features.c Visible.c Paths.c \
	ThreeDCore.c ThreeDNav.c ThreeDVis.c ThreeDDraw.c ThreeDIL.c \
	OAA.c agentlib.c tcp.c \
	DIS.c matrix.c


include_HEADERS = ThreeDCore.h Paths.h

EXTRA_HEADERS = \
	TsCache.h TsCacheInline.h TsCoords.h TsCoordsInline.h \
	TsGeoPyramid.h TsGeoPyramidI.h TsGeoPyramidInline.h \
	TerraVisionInc.h Texture.h TextureI.h TextureInline.h \
	ThreeDCoreI.h ThreeDIL.h \
	Features.h VRML.h GeoVRML.h \
	TVGLX.h TsLogging.h Visible.h \
	NetLogCore.h \
	OAA.h agentlib.h tcp.h \
	DIS.h matrix.h 


EXTRA_PROGRAMS = test_TsCache test_nav test_render test_oaa

TESTS = test_TsCache

test_TsCache_SOURCES = test_TsCache.c
test_TsCache_DEPENDENCIES = libtv2.a
test_TsCache_LDADD = -L. -ltv2

test_nav_SOURCES = test_nav.c
test_nav_LDADD = @GLUT_LIBS@

test_render_SOURCES = test_render.c
test_render_DEPENDENCIES = libtv2.a
test_render_LDADD = -L. -ltv2 @GLUT_LIBS@

test_oaa_SOURCES = test_oaa.c
test_oaa_DEPENDENCIES = libtv2.a
test_oaa_LDADD = -L. -ltv2
