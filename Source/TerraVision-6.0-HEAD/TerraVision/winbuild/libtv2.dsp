# Microsoft Developer Studio Project File - Name="libtv2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtv2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtv2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtv2.mak" CFG="libtv2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtv2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtv2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtv2 - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../.." /I "../../winbuild" /I "../../tsm" /I "../../tsm/winbuild" /I "../../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "../../conversions" /I "../../terravision" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtv2.lib"

!ELSEIF  "$(CFG)" == "libtv2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libtv2___Win32_Debug"
# PROP BASE Intermediate_Dir "libtv2___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /vmb /vd1 /GR- /GX /Z7 /Od /I "../.." /I "../../winbuild" /I "../../tsm" /I "../../tsm/winbuild" /I "../../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "../../conversions" /I "../../terravision" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /YX /FD /GZ /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtv2_d.lib"

!ENDIF 

# Begin Target

# Name "libtv2 - Win32 Release"
# Name "libtv2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\agentlib.c
# End Source File
# Begin Source File

SOURCE=..\DIS.c
# End Source File
# Begin Source File

SOURCE=..\DotGeo.c
# End Source File
# Begin Source File

SOURCE=..\Features.c
# End Source File
# Begin Source File

SOURCE=..\GeoVRML.c
# End Source File
# Begin Source File

SOURCE=..\matrix.c
# End Source File
# Begin Source File

SOURCE=..\NetLogCore.c
# End Source File
# Begin Source File

SOURCE=..\OAA.c
# End Source File
# Begin Source File

SOURCE=..\Paths.c
# End Source File
# Begin Source File

SOURCE=..\tcp.c
# End Source File
# Begin Source File

SOURCE=..\Texture.c
# End Source File
# Begin Source File

SOURCE=..\ThreeDCore.c
# End Source File
# Begin Source File

SOURCE=..\ThreeDDraw.c
# End Source File
# Begin Source File

SOURCE=..\ThreeDNav.c
# End Source File
# Begin Source File

SOURCE=..\ThreeDVis.c
# End Source File
# Begin Source File

SOURCE=..\TsCache.c
# End Source File
# Begin Source File

SOURCE=..\TsCoords.c
# End Source File
# Begin Source File

SOURCE=..\TsGeoPyramid.c
# End Source File
# Begin Source File

SOURCE=..\TsGeoPyramid_OI.c
# End Source File
# Begin Source File

SOURCE=..\TsLogging.c
# End Source File
# Begin Source File

SOURCE=..\TVGLX.c
# End Source File
# Begin Source File

SOURCE=..\Visible.c
# End Source File
# Begin Source File

SOURCE=..\VRML.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\agentlib.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\DIS.h
# End Source File
# Begin Source File

SOURCE=..\Features.h
# End Source File
# Begin Source File

SOURCE=..\GeoVRML.h
# End Source File
# Begin Source File

SOURCE=..\matrix.h
# End Source File
# Begin Source File

SOURCE=..\NetLogCore.h
# End Source File
# Begin Source File

SOURCE=..\OAA.h
# End Source File
# Begin Source File

SOURCE=..\Paths.h
# End Source File
# Begin Source File

SOURCE=..\tcp.h
# End Source File
# Begin Source File

SOURCE=..\Texture.h
# End Source File
# Begin Source File

SOURCE=..\TextureInline.h
# End Source File
# Begin Source File

SOURCE=..\ThreeDCore.h
# End Source File
# Begin Source File

SOURCE=..\ThreeDCoreI.h
# End Source File
# Begin Source File

SOURCE=..\Trace.h
# End Source File
# Begin Source File

SOURCE=..\TsCache.h
# End Source File
# Begin Source File

SOURCE=..\TsCoords.h
# End Source File
# Begin Source File

SOURCE=..\TsGeoPyramid.h
# End Source File
# Begin Source File

SOURCE=..\TsLogging.h
# End Source File
# Begin Source File

SOURCE=..\TVGLX.h
# End Source File
# Begin Source File

SOURCE=..\Visible.h
# End Source File
# Begin Source File

SOURCE=..\VRML.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# End Target
# End Project
