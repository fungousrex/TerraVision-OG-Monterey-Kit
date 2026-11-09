configincludedir = $(includedir)

lib_LIBRARIES = @PTHREAD_LIBS@

libpthread_a_SOURCES = @PTHREAD_SRCS@
libpthread_a_LIBADD = @PTHREAD_OBJS@
libpthread_a_DEPENDENCIES = @PTHREAD_OBJS@

EXTRA_libpthread_a_SOURCES = pthr_sproc.c

EXTRA_LIBRARIES = libpthread.a
EXTRA_HEADERS = pthr_sproc.h
EXTRA_DATA = pthread.h

LDADD = @PTHREAD_LIBS@

configinclude_DATA = @PTHREAD_HDRS@

PTHR_DEPS = $(patsubst %.c, .deps/%.P, @PTHREAD_SRCS@)

DEP_FILES = $(PTHR_DEPS) \
 .deps/thr_ex1.P .deps/thr_ex2.P

EXTRA_PROGRAMS = thr_ex1 thr_ex2 thr_ex3

TESTS = thr_ex1 thr_ex2 @THR_EX3@

thr_ex1_SOURCES = thr_ex1.c
thr_ex1_DEPENDENCIES = $(LDADD)

thr_ex2_SOURCES = thr_ex2.c
thr_ex2_DEPENDENCIES = $(LDADD)

thr_ex3_SOURCES = thr_ex3.c
thr_ex3_DEPENDENCIES = $(LDADD)
