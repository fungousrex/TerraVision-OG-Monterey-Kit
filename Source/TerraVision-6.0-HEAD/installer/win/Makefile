TCLDIR    = //c/Progra~1/Tcl
TCLMAJVER = 8
TCLMINVER = 4

ROOT = ../..
DEST = tcltk

TCLVER1 = $(TCLMAJVER)$(TCLMINVER)
TCLVER2 = $(TCLMAJVER).$(TCLMINVER)

RM = rm -f

all: makedirs tclscripts binfiles tcllibs

makedirs:
	mkdir -p $(DEST)/tcl
	mkdir -p $(DEST)/lib
	mkdir -p $(DEST)/lib/tcl$(TCLVER2)
	mkdir -p $(DEST)/lib/tk$(TCLVER2)

tclscripts:
	cp $(ROOT)/TclTV/*.tcl $(DEST)/tcl

binfiles:
	cp $(ROOT)/bin/TclTV.exe $(DEST)/bin
	cp $(ROOT)/nsprwin/lib/nspr4.dll $(DEST)/bin

tcllibs:
	cp $(TCLDIR)/bin/tcl$(TCLVER1).dll $(DEST)/bin
	cp $(TCLDIR)/bin/tclpip$(TCLVER1).dll $(DEST)/bin
	cp $(TCLDIR)/bin/tk$(TCLVER1).dll $(DEST)/bin
	cp $(TCLDIR)/lib/tk$(TCLVER2)/*.* $(DEST)/lib/tk$(TCLVER2)
	cp $(TCLDIR)/lib/tk$(TCLVER2)/tclIndex $(DEST)/lib/tk$(TCLVER2)
	cp $(TCLDIR)/lib/tcl$(TCLVER2)/*.tcl $(DEST)/lib/tcl$(TCLVER2)
	cp $(TCLDIR)/lib/tcl$(TCLVER2)/tclIndex $(DEST)/lib/tcl$(TCLVER2)

clean:
	$(RM) $(DEST)/bin/tcl*dll
	$(RM) $(DEST)/bin/tk*dll
	$(RM) $(DEST)/bin/nspr4.dll
	$(RM) $(DEST)/bin/TclTV.exe
	$(RM) -r $(DEST)/tcl
	$(RM) -r $(DEST)/lib
	$(RM) tvinstall.exe


