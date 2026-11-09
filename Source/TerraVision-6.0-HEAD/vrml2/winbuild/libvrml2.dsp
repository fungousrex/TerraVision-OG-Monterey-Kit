# Microsoft Developer Studio Project File - Name="libvrml2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libvrml2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libvrml2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libvrml2.mak" CFG="libvrml2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libvrml2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libvrml2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libvrml2 - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\.." /I "..\../winbuild" /I "../../nsprwin/include" /I "..\../vrml2" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libvrml2.lib"

!ELSEIF  "$(CFG)" == "libvrml2 - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\.." /I "..\../winbuild" /I "../../nsprwin/include" /I "..\../vrml2" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libvrml2_d.lib"

!ENDIF 

# Begin Target

# Name "libvrml2 - Win32 Release"
# Name "libvrml2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\bbox.c
# End Source File
# Begin Source File

SOURCE=..\c_nodes.c
# End Source File
# Begin Source File

SOURCE=..\c_print.c
# End Source File
# Begin Source File

SOURCE=..\c_render.c
# End Source File
# Begin Source File

SOURCE=..\c_utils.c
# End Source File
# Begin Source File

SOURCE=..\decomp.c
# End Source File
# Begin Source File

SOURCE=..\geom3.c
# End Source File
# Begin Source File

SOURCE=..\invert.c
# End Source File
# Begin Source File

SOURCE=..\lu.c
# End Source File
# Begin Source File

SOURCE=..\newlex.c
# End Source File
# Begin Source File

SOURCE=..\newparse.c
# End Source File
# Begin Source File

SOURCE=..\plist.c
# End Source File
# Begin Source File

SOURCE=..\quat.c
# End Source File
# Begin Source File

SOURCE=..\symtab.c
# End Source File
# Begin Source File

SOURCE=..\vec3.c
# End Source File
# Begin Source File

SOURCE=..\vrml2defs.c
# End Source File
# Begin Source File

SOURCE=..\vrml2stubs.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\bbox.h
# End Source File
# Begin Source File

SOURCE=..\c_nodes.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\geom3.h
# End Source File
# Begin Source File

SOURCE=..\plist.h
# End Source File
# Begin Source File

SOURCE=..\quat.h
# End Source File
# Begin Source File

SOURCE=..\symtab.h
# End Source File
# Begin Source File

SOURCE=..\vec3.h
# End Source File
# Begin Source File

SOURCE=..\vrml2.h
# End Source File
# Begin Source File

SOURCE=..\vrml2il.h
# End Source File
# Begin Source File

SOURCE=..\vrml2int.h
# End Source File
# End Group
# End Target
# End Project
