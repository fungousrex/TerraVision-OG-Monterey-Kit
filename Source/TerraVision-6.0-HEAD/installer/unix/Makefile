TCLDIR    = /usr/local/tcl
TCLMAJVER = 8
TCLMINVER = 4
ARCH      = linux-i386
TVVER     = 6.0



ROOT = ../../$(ARCH)
SRC  = ../..
DEST = tcltk

TCLVER1 = $(TCLMAJVER)$(TCLMINVER)
TCLVER2 = $(TCLMAJVER).$(TCLMINVER)

RM = rm -f

all: makedirs tclscripts binfiles tcllibs config archive

makedirs:
	mkdir -p $(DEST)/tcl
	mkdir -p $(DEST)/lib
	mkdir -p $(DEST)/lib/tcl$(TCLVER2)
	mkdir -p $(DEST)/lib/tk$(TCLVER2)

tclscripts:
	cp $(SRC)/TclTV/*.tcl $(DEST)/tcl

binfiles:
	cp $(ROOT)/TclTV/TclTV $(DEST)/bin
	cp $(SRC)/RELEASE $(DEST)

tcllibs:
	cp $(TCLDIR)/lib/tclConfig.sh $(DEST)/lib
	cp $(TCLDIR)/lib/tkConfig.sh $(DEST)/lib
	cp $(TCLDIR)/lib/libtcl$(TCLVER2).so $(DEST)/lib
	cp $(TCLDIR)/lib/libtk$(TCLVER2).so $(DEST)/lib
	cp $(TCLDIR)/lib/tk$(TCLVER2)/*.* $(DEST)/lib/tk$(TCLVER2)
	cp $(TCLDIR)/lib/tk$(TCLVER2)/tclIndex $(DEST)/lib/tk$(TCLVER2)
	cp $(TCLDIR)/lib/tcl$(TCLVER2)/*.tcl $(DEST)/lib/tcl$(TCLVER2)
	cp $(TCLDIR)/lib/tcl$(TCLVER2)/tclIndex $(DEST)/lib/tcl$(TCLVER2)

config:
	cp ../win/tcltk/config/*.xml $(DEST)/etc
	cp -pr ../win/tcltk/config/paths $(DEST)/etc

archive:
	cd tcltk ; tar -czf ../tvinstall-$(TVVER)-linux-i386.tar.gz *

clean:
	$(RM) $(DEST)/bin/TclTV
	$(RM) $(DEST)/RELEASE
	$(RM) $(DEST)/etc/*.xml
	$(RM) -r $(DEST)/tcl
	$(RM) -r $(DEST)/lib
	$(RM) -r $(DEST)/etc/paths
	$(RM) tvinstall*

