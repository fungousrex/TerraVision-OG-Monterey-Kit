# Microsoft Developer Studio Project File - Name="libnspr3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libnspr3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libnspr3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libnspr3.mak" CFG="libnspr3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libnspr3 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libnspr3 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libnspr3 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../WINBUILD" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /D "WINNT" /D "XP_PC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../bin/libnspr3.lib"

!ELSEIF  "$(CFG)" == "libnspr3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "../WINBUILD" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /D "WINNT" /D "XP_PC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../../bin/libnspr3_d.lib"

!ENDIF 

# Begin Target

# Name "libnspr3 - Win32 Release"
# Name "libnspr3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=..\pr\src\md\windows\w32poll.c
# End Source File
# Begin Source File

SOURCE=..\pr\src\md\windows\win32_errors.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
