EXTRA_DIST = Makefile.unx

lib_LTLIBRARIES = libtogl.la
#noinst_LIBRARIES = libtogl.a

include_HEADERS = togl.h
EXTRA_HEADERS = image.h toglinlines.h toglinternals.h \
	tkFont.h \
	tkInt4.0.h tkInt4.1.h tkInt4.2.h tkInt8.0.h tkInt8.0p2.h \
	tkInt8.2.2.h tkInt8.3.1.h \
	tkPort.h tkWin.h tkWinInt.h tkWinPort.h

libtogl_la_SOURCES = togl.c

## Version is 4.1.0  [x:y:z  (x - z).(z).(y) ]
libtogl_la_LDFLAGS = -version-info 1:3:0
