#  DIS: Read DIS PDUs from a network stream
#

lib_LIBRARIES = libdis_client.a

libdis_client_a_SOURCES = \
	client_lib.c print.c

pkginclude_HEADERS = \
	pdu.h disdefs.h \
	appearance.h appearance2.h articulat.h \
	category.h country.h datum.h radar.h repair.h sites.h

EXTRA_PROGRAMS = test_it

test_it_DEPS = libdis_client.a
test_it_LDADD = libdis_client.a
