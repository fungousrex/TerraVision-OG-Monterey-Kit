# Microsoft Developer Studio Project File - Name="libghttp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libghttp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libghttp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libghttp.mak" CFG="libghttp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libghttp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libghttp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libghttp - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\winbuild" /I "..\..\ghttp" /I "../../nsprwin/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libghttp.lib"

!ELSEIF  "$(CFG)" == "libghttp - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\..\winbuild" /I "..\..\ghttp" /I "../../nsprwin/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libghttp_d.lib"

!ENDIF 

# Begin Target

# Name "libghttp - Win32 Release"
# Name "libghttp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ghttp.c
# End Source File
# Begin Source File

SOURCE=..\http_base64.c
# End Source File
# Begin Source File

SOURCE=..\http_date.c
# End Source File
# Begin Source File

SOURCE=..\http_hdrs.c
# End Source File
# Begin Source File

SOURCE=..\http_req.c
# End Source File
# Begin Source File

SOURCE=..\http_resp.c
# End Source File
# Begin Source File

SOURCE=..\http_trans.c
# End Source File
# Begin Source File

SOURCE=..\http_uri.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ghttp.h
# End Source File
# Begin Source File

SOURCE=..\ghttp_constants.h
# End Source File
# Begin Source File

SOURCE=..\http_base64.h
# End Source File
# Begin Source File

SOURCE=..\http_date.h
# End Source File
# Begin Source File

SOURCE=..\http_global.h
# End Source File
# Begin Source File

SOURCE=..\http_hdrs.h
# End Source File
# Begin Source File

SOURCE=..\http_req.h
# End Source File
# Begin Source File

SOURCE=..\http_resp.h
# End Source File
# Begin Source File

SOURCE=..\http_trans.h
# End Source File
# Begin Source File

SOURCE=..\http_uri.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\ChangeLog

!IF  "$(CFG)" == "libghttp - Win32 Release"

!ELSEIF  "$(CFG)" == "libghttp - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
