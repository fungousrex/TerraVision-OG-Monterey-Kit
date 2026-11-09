# Microsoft Developer Studio Project File - Name="TclTV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=TclTV - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TclTV.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TclTV.mak" CFG="TclTV - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TclTV - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TclTV - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TclTV - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "$(pathtcl)/include" /I "../.." /I "../../winbuild" /I "../../tsm" /I "../../tsm/winbuild" /I "../../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "../../conversions" /I "../../terravision" /I "../../libTogl" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_CONFIG_H" /D "TRACING" /D "_MSVC" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib ws2_32.lib mswsock.lib winmm.lib nspr4.lib tk84.lib tcl84.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrtd" /out:"..\..\bin\TclTV.exe" /libpath:"$(pathtcl)/lib" /libpath:"../../nsprwin/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TclTV - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "$(pathtcl)/include" /I "../.." /I "../../winbuild" /I "../../tsm" /I "../../tsm/winbuild" /I "../../tsm/winbuild/tsm" /I "../../nsprwin/include" /I "../../conversions" /I "../../terravision" /I "../../libTogl" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_CONFIG_H" /D "TRACING" /D "_MSVC" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib ws2_32.lib mswsock.lib winmm.lib nspr4.lib tk84.lib tcl84.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"..\..\bin\TclTV_d.exe" /pdbtype:sept /libpath:"$(pathtcl)/lib" /libpath:"../../nsprwin/lib"

!ENDIF 

# Begin Target

# Name "TclTV - Win32 Release"
# Name "TclTV - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\main.c
# End Source File
# Begin Source File

SOURCE=..\pthrCmds.c
# End Source File
# Begin Source File

SOURCE=..\tsGeoPyramidCmds.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabaseCmds.c
# End Source File
# Begin Source File

SOURCE=..\tsmUtilCmds.c
# End Source File
# Begin Source File

SOURCE=..\tvBaseCmds.c
# End Source File
# Begin Source File

SOURCE=..\tvLinkCmds.c
# End Source File
# Begin Source File

SOURCE=..\tvPathCmds.c
# End Source File
# Begin Source File

SOURCE=..\tvViewCmds.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\pthrCmds.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\tsGeoPyramidCmds.h
# End Source File
# Begin Source File

SOURCE=..\tsmCmds.h
# End Source File
# Begin Source File

SOURCE=..\tvBaseCmds.h
# End Source File
# Begin Source File

SOURCE=..\tvBaseCmdsP.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TclTV.rc
# End Source File
# Begin Source File

SOURCE=.\tv16.ico
# End Source File
# Begin Source File

SOURCE=.\tv32.ico
# End Source File
# End Group
# End Target
# End Project
