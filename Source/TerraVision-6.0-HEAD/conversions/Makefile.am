lib_LIBRARIES = libse_conv.a

libse_conv_a_SOURCES = \
	se_conv.c \
	se_conv_default.c \
	se_conv_io.c \
	se_conv_print.c \
	se_conv_util.c \
	se_conv_valid.c \
	se_coord_conv.c \
	se_datums.c \
	se_stds.c

include_HEADERS = \
	se_conv.h \
	se_conv_default.h \
	se_conv_io.h \
	se_conv_print.h \
	se_conv_types.h \
	se_conv_util.h \
	se_conv_valid.h \
	se_datums.h \
	se_stds.h

noinst_PROGRAMS = make_geoid

make_geoid_SOURCES = make_geoid.c

ww15mgh16.c: make_geoid
	./make_geoid @srcdir@/WW15MGH.GRD > @srcdir@/ww15mgh16.c
