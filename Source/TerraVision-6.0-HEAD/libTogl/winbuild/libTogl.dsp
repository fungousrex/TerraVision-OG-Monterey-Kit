# Microsoft Developer Studio Project File - Name="libTogl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libTogl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtogl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtogl.mak" CFG="libTogl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libTogl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libTogl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libTogl - Win32 Release"

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
# ADD CPP /nologo /MDd /W3 /GX /O2 /I "$(pathtcl)\include" /I "..\..\libTogl" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtogl.lib"

!ELSEIF  "$(CFG)" == "libTogl - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(pathtcl)\include" /I "..\..\libTogl" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_WINDOWS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtogl_d.lib"

!ENDIF 

# Begin Target

# Name "libTogl - Win32 Release"
# Name "libTogl - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\double.c
# End Source File
# Begin Source File

SOURCE=..\gears.c
# End Source File
# Begin Source File

SOURCE=..\image.c
# End Source File
# Begin Source File

SOURCE=..\index.c
# End Source File
# Begin Source File

SOURCE=..\overlay.c
# End Source File
# Begin Source File

SOURCE=..\stereo.c
# End Source File
# Begin Source File

SOURCE=..\texture.c
# End Source File
# Begin Source File

SOURCE=..\tkConsole.c
# End Source File
# Begin Source File

SOURCE=..\togl.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\image.h
# End Source File
# Begin Source File

SOURCE=..\tkFont.h
# End Source File
# Begin Source File

SOURCE=..\tkInt4.0.h
# End Source File
# Begin Source File

SOURCE=..\tkInt4.1.h
# End Source File
# Begin Source File

SOURCE=..\tkInt4.2.h
# End Source File
# Begin Source File

SOURCE=..\tkInt8.0.h
# End Source File
# Begin Source File

SOURCE=..\tkInt8.0p2.h
# End Source File
# Begin Source File

SOURCE=..\tkInt8.2.2.h
# End Source File
# Begin Source File

SOURCE=..\tkInt8.3.1.h
# End Source File
# Begin Source File

SOURCE=..\tkIntDecls.h
# End Source File
# Begin Source File

SOURCE=..\tkPort.h
# End Source File
# Begin Source File

SOURCE=..\tkWin.h
# End Source File
# Begin Source File

SOURCE=..\tkWinInt.h
# End Source File
# Begin Source File

SOURCE=..\tkWinPort.h
# End Source File
# Begin Source File

SOURCE=..\togl.h
# End Source File
# Begin Source File

SOURCE=..\toglinlines.h
# End Source File
# Begin Source File

SOURCE=..\toglinternals.h
# End Source File
# End Group
# End Target
# End Project
