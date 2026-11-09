# Microsoft Developer Studio Project File - Name="tvutils" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=tvutils - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tvutils.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tvutils.mak" CFG="tvutils - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tvutils - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "tvutils - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tvutils - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\.." /I "..\../winbuild" /I "..\../tsm" /I "..\../tsm/winbuild" /I "..\../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "..\../tiff/libtiff" /I "..\../libXML" /I "..\../libXML/winbuild" /I "..\../libXML/winbuild/libxml" /I "..\../jpeg" /I "..\../jpeg/winbuild" /I "..\../libungif/lib" /I "../../ghttp" /I "../../conversions" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /D "MSVC" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\tvutils.lib"

!ELSEIF  "$(CFG)" == "tvutils - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\.." /I "..\../winbuild" /I "..\../tsm" /I "..\../tsm/winbuild" /I "..\../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "..\../tiff/libtiff" /I "..\../libXML" /I "..\../libXML/winbuild" /I "..\../libXML/winbuild/libxml" /I "..\../jpeg" /I "..\../jpeg/winbuild" /I "..\../libungif/lib" /I "../../ghttp" /I "../../conversions" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "MSVC" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\tvutils_d.lib"

!ENDIF 

# Begin Target

# Name "tvutils - Win32 Release"
# Name "tvutils - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ADL.c
# End Source File
# Begin Source File

SOURCE=..\Args.c
# End Source File
# Begin Source File

SOURCE=..\Config.c
# End Source File
# Begin Source File

SOURCE=..\Gazetteer.c
# End Source File
# Begin Source File

SOURCE=..\Graphics.c
# End Source File
# Begin Source File

SOURCE=..\Init.c
# End Source File
# Begin Source File

SOURCE=..\Values.c
# End Source File
# Begin Source File

SOURCE=..\Viewpoint.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\acconfig.h
# End Source File
# Begin Source File

SOURCE=..\Args.h
# End Source File
# Begin Source File

SOURCE=..\Config.h
# End Source File
# Begin Source File

SOURCE=..\Init.h
# End Source File
# Begin Source File

SOURCE=..\Platform.h
# End Source File
# Begin Source File

SOURCE=..\tvutils.h
# End Source File
# Begin Source File

SOURCE=..\Viewpoint.h
# End Source File
# End Group
# End Target
# End Project
