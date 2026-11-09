# Microsoft Developer Studio Project File - Name="dllnspr3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dllnspr3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dllnspr3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dllnspr3.mak" CFG="dllnspr3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dllnspr3 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dllnspr3 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dllnspr3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dllnspr3___Win32_Release"
# PROP BASE Intermediate_Dir "dllnspr3___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "dllRelease"
# PROP Intermediate_Dir "dllRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLNSPR3_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_NSPR_BUILD_" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib mswsock.lib winmm.lib /nologo /dll /machine:I386 /out:"../../bin/dllnspr3.dll"

!ELSEIF  "$(CFG)" == "dllnspr3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dllnspr3___Win32_Debug"
# PROP BASE Intermediate_Dir "dllnspr3___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "dllDebug"
# PROP Intermediate_Dir "dllDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DLLNSPR3_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MDd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_NSPR_BUILD_" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib mswsock.lib winmm.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../bin/dllnspr3_d.dll" /pdbtype:sept
# Begin Custom Build
TargetPath=\TerraVision\bin\dllnspr3_d.dll
TargetName=dllnspr3_d
InputPath=\TerraVision\bin\dllnspr3_d.dll
SOURCE="$(InputPath)"

"f:\winnt\system32\$(TargetName)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) f:\winnt\system32

# End Custom Build

!ENDIF 

# Begin Target

# Name "dllnspr3 - Win32 Release"
# Name "dllnspr3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\pr\src\nspr.rc
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntdllmn.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntgc.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntinrval.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntio.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntmisc.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntsem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\ntthread.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\pralarm.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\pratom.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prcmon.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prcountr.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prcthr.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prdir.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prdtoa.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prdump.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prenv.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prerr.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prerror.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prerrortable.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prfdcach.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prfile.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prinit.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prinrval.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prio.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\priometh.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\pripc.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\pripcsem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prlayer.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\linking\prlink.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prlog.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prlog2.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prlong.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\malloc\prmalloc.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prmapopt.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\malloc\prmem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prmmap.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prmon.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prmwait.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prnetdb.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prolock.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\prosdep.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prpolevt.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prprf.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prrng.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prrwlock.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prscanf.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\memory\prseg.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prsem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\memory\prshm.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\memory\prshma.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prsocket.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\io\prstdio.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prsystem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prthinfo.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prtime.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\prtpd.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\misc\prtrace.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\combined\prucpu.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\combined\prucv.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\combined\prulock.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\combined\prustack.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\threads\combined\pruthr.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\prvrsion.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\pthreads\ptio.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\pthreads\ptmisc.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\pthreads\ptsynch.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\pthreads\ptthread.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\w32ipcsem.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\w32poll.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\w32rng.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\w32shm.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\win32_errors.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\pr\include\md\_pcos.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\md\_win32_errors.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\md\_winnt.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\nspr.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\private\pprio.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\private\pprmwait.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\private\pprthred.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\obsolete\pralarm.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\pratom.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prbit.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prclist.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prcmon.h
# End Source File
# Begin Source File

SOURCE=.\prcpucfg.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prcvar.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prdtoa.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prenv.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prerr.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prerror.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\private\primpl.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prinet.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prinit.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prinrval.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prio.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\pripcsem.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prlink.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prlock.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prlog.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prlong.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prmem.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prmon.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prmwait.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prnetdb.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\obsolete\probslet.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\md\prosdep.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prpdce.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prprf.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\private\prpriv.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prproces.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prrng.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prrwlock.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\obsolete\prsem.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prshm.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prshma.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prsystem.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prthread.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prtime.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prtypes.h
# End Source File
# Begin Source File

SOURCE=..\pr\include\prwin16.h
# End Source File
# Begin Source File

SOURCE=..\pr\src\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
