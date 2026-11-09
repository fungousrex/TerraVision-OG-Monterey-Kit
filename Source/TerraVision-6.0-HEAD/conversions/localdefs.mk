include $(DEPTH)/templates/cleanup_macros.mk

DIR_PATH := libsrc/conversions
LIBNAME := SEconversions

C_SOURCES :=     \
		se_conv.c \
		se_conv_default.c \
		se_conv_print.c \
		se_conv_util.c \
		se_conv_valid.c \
		se_coord_conv.c \
		se_datums.c \
		se_stds.c


EXPORTED_HEADERS	:= \
		se_conv.h \
		se_conv_default.h \
		se_conv_print.h \
		se_conv_types.h \
		se_conv_util.h \
		se_conv_valid.h \
		se_datums.h \
		se_stds.h 

REQUIRED_LIBS		:= 

LOCAL_INCLUDES := 

include $(DEPTH)/templates/localtargets.mk
