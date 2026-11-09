#  pthrutil: A library of pthread-safe basic functions
#
#  Copyright SRI International (1993-1998)
#
#  All rights reserved.

lib_LIBRARIES = libpthrutil.a

libpthrutil_a_SOURCES = \
	report.c trace_inline.c timestamp.c html_gen.c \
	list.c list_inline.c \
	aqueue.c aqueue_inline.c \
	eqhash.c eqhash_inline.c \
	eqlhash.c eqlhash_inline.c \
	heap.c \
	lru.c lru_inline.c \
	lru_hash.c lru_hash_inline.c \
	istack.c istack_inline.c

include_HEADERS = pthrutil.h
pkginclude_HEADERS = \
	report.h timestamp.h \
	trace.h trace_inline.h \
	list.h list_inline.h \
	aqueue.h aqueue_inline.h \
	eqhash.h eqhash_inline.h \
	eqlhash.h eqlhash_inline.h \
	heap.h \
	lru.h lru_inline.h \
	lru_hash.h lru_hash_inline.h \
	istack.h istack_inline.h

cfgincludedir = $(pkgincludedir)

EXTRA_PROGRAMS = test_list test_lru test_istack

LDADD = libpthrutil.a -lpthread

TESTS = test_list test_lru test_istack

test_list_SOURCES = test_list.c
test_lru_SOURCES = test_lru.c
test_istack_SOURCES = test_istack.c
