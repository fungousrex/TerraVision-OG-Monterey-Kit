# Microsoft Developer Studio Project File - Name="libglt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libglt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libglt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libglt.mak" CFG="libglt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libglt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libglt - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libglt - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(pathtcl)/include" /I "..\.." /I "..\..\winbuild" /I "../../nsprwin/include" /I "../../tsm/winbuild" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libglt.lib"

!ELSEIF  "$(CFG)" == "libglt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libglt___Win32_Debug"
# PROP BASE Intermediate_Dir "libglt___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "$(pathtcl)/include" /I "..\.." /I "..\..\winbuild" /I "../../nsprwin/include" /I "../../tsm/winbuild" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libglt_d.lib"

!ENDIF 

# Begin Target

# Name "libglt - Win32 Release"
# Name "libglt - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\glm.c
# End Source File
# Begin Source File

SOURCE=..\glmil.c
# End Source File
# Begin Source File

SOURCE=..\glmTcl.c
# End Source File
# Begin Source File

SOURCE=..\glmWin.c
# End Source File
# Begin Source File

SOURCE=..\glt.c
# End Source File
# Begin Source File

SOURCE=..\glt_inline.c
# End Source File
# Begin Source File

SOURCE=..\glt_state.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\glm.h
# End Source File
# Begin Source File

SOURCE=..\glmil.h
# End Source File
# Begin Source File

SOURCE=..\glmimpl.h
# End Source File
# Begin Source File

SOURCE=..\glmTcl.h
# End Source File
# Begin Source File

SOURCE=..\glt.h
# End Source File
# Begin Source File

SOURCE=..\glt_inline.h
# End Source File
# Begin Source File

SOURCE=..\gltI.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# PROP Exclude_From_Build 1
# End Source File
# End Target
# End Project
