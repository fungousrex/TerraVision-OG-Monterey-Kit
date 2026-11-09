lib_LIBRARIES = libvrml2.a

libvrml2_a_SOURCES = \
	vrml2il.c vrml2stubs.c vrml2defs.c newparse.c newlex.c \
	plist.c symtab.c \
	c_nodes.c c_utils.c c_print.c c_render.c \
	vec3.c geom3.c bbox.c quat.c invert.c lu.c decomp.c

pkginclude_HEADERS = vrml2.h vrml2il.h syms.h \
	plist.h symtab.h \
	c_nodes.h nodes.def \
	vec3.h geom3.h quat.h bbox.h

EXTRA_HEADERS = vrml2int.h parse.h symlist.def c_nodesI.h

EXTRA_PROGRAMS = vrml2_parse vrml2_browse gensyms \
	test_syms test_plist test_proto test_vec3

vrml2_parse_SOURCES = vrml2_parse.c
vrml2_parse_LDADD = libvrml2.a

vrml2_browse_SOURCES = vrml2_browse.c
vrml2_browse_LDADD = libvrml2.a @GLUT_LIBS@ -lm

if GETOPT
GETOPT_C = getopt.c
endif

gensyms_SOURCES = gensyms.c $(GETOPT_C)
gensyms_DEPENDENCIES = symlist.def

$(srcdir)/syms.c $(srcdir)/syms.h: s-syms ; @true
s-syms : gensyms $(srcdir)/move-if-change
	./gensyms -o syms
	$(SHELL) $(srcdir)/move-if-change syms.h $(srcdir)/syms.h
	$(SHELL) $(srcdir)/move-if-change syms.c $(srcdir)/syms.c
	-rm -f syms.c syms.h
	touch s-syms

TESTS = test_syms test_plist test_proto test_vec3

test_syms_SOURCES = test_syms.c

test_plist_SOURCES = test_plist.c
test_plist_LDADD = plist.o symtab.o

test_proto_SOURCES = test_proto.c
test_proto_DEPENDENCIES = libvrml2.a
test_proto_LDADD = libvrml2.a

test_vec3_SOURCES = test_vec3.c
test_vec3_DEPENDENCIES = libvrml2.a
test_vec3_LDADD = libvrml2.a @GLUT_LIBS@ -lm

