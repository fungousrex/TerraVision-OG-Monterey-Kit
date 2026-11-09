# Microsoft Developer Studio Project File - Name="ActiveX_TV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ActiveX_TV - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TerraVision.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TerraVision.mak" CFG="ActiveX_TV - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ActiveX_TV - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ActiveX_TV - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ActiveX_TV - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ActiveX_TV - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../.." /I "../../../winbuild" /I "../../../tsm" /I "../../../tsm/winbuild" /I "../../../tsm/winbuild/tsm" /I "../../../nsprwin/include" /I "../../../conversions" /I "../../../terravision" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "HAVE_CONFIG_H" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib glu32.lib ws2_32.lib mswsock.lib winmm.lib nspr4.lib libGeo_DiscClient.lib libGeo_Core.lib /nologo /subsystem:windows /dll /machine:I386 /def:".\TerraVision.def" /out:"..\..\..\bin\TerraVision.ocx" /libpath:"$(pathtcl)/lib" /libpath:"../../../nsprwin/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Release
TargetPath=\src\TerraVision\bin\TerraVision.ocx
InputPath=\src\TerraVision\bin\TerraVision.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Ext "ocx"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "../../.." /I "../../../winbuild" /I "../../../tsm" /I "../../../tsm/winbuild" /I "../../../tsm/winbuild/tsm" /I "../../../nsprwin/include" /I "../../../conversions" /I "../../../terravision" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "HAVE_CONFIG_H" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib ws2_32.lib mswsock.lib winmm.lib nspr4.lib libGeo_DiscClient.lib libGeo_Core.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /def:".\TerraVision.def" /out:"..\..\..\bin\TerraVision_d.ocx" /pdbtype:sept /libpath:"$(pathtcl)/lib" /libpath:"../../../nsprwin/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\Debug
TargetPath=\src\TerraVision\bin\TerraVision_d.ocx
InputPath=\src\TerraVision\bin\TerraVision_d.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugU"
# PROP BASE Intermediate_Dir "DebugU"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugU"
# PROP Intermediate_Dir "DebugU"
# PROP Target_Ext "ocx"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\DebugU
TargetPath=.\DebugU\TerraVision.ocx
InputPath=.\DebugU\TerraVision.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseU"
# PROP BASE Intermediate_Dir "ReleaseU"
# PROP BASE Target_Ext "ocx"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseU"
# PROP Intermediate_Dir "ReleaseU"
# PROP Target_Ext "ocx"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_USRDLL" /D "_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386
# Begin Custom Build - Registering ActiveX Control...
OutDir=.\ReleaseU
TargetPath=.\ReleaseU\TerraVision.ocx
InputPath=.\ReleaseU\TerraVision.ocx
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "ActiveX_TV - Win32 Release"
# Name "ActiveX_TV - Win32 Debug"
# Name "ActiveX_TV - Win32 Unicode Debug"
# Name "ActiveX_TV - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TerraVision.cpp
# End Source File
# Begin Source File

SOURCE=.\TerraVision.def

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision.odl
# End Source File
# Begin Source File

SOURCE=.\TerraVision.rc
# End Source File
# Begin Source File

SOURCE=.\TerraVisionCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\TerraVisionPpg.cpp
# End Source File
# Begin Source File

SOURCE=.\tvEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\tvViewer.cpp
# End Source File
# Begin Source File

SOURCE=.\Viewpoint.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TerraVision.h
# End Source File
# Begin Source File

SOURCE=.\TerraVisionCtl.h
# End Source File
# Begin Source File

SOURCE=.\TerraVisionPpg.h
# End Source File
# Begin Source File

SOURCE=.\tvEvent.h
# End Source File
# Begin Source File

SOURCE=.\tvViewer.h
# End Source File
# Begin Source File

SOURCE=.\tvWinDef.h
# End Source File
# Begin Source File

SOURCE=.\Viewpoint.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TerraVision.ico
# End Source File
# Begin Source File

SOURCE=.\TerraVisionCtl.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\cabsign.bat

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ChangeLog

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision.ddf

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision.inf

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision.lic

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying license file...
OutDir=.\Release
ProjDir=.
TargetName=TerraVision
InputPath=.\TerraVision.lic

"$(OutDir)\$(TargetName).lic" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(ProjDir)\$(TargetName).lic" "$(OutDir)"

# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying license file...
OutDir=.\Debug
ProjDir=.
TargetName=TerraVision_d
InputPath=.\TerraVision.lic

"$(OutDir)\$(TargetName).lic" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(ProjDir)\$(TargetName).lic" "$(OutDir)"

# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying license file...
OutDir=.\DebugU
ProjDir=.
TargetName=TerraVision
InputPath=.\TerraVision.lic

"$(OutDir)\$(TargetName).lic" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(ProjDir)\$(TargetName).lic" "$(OutDir)"

# End Custom Build

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying license file...
OutDir=.\ReleaseU
ProjDir=.
TargetName=TerraVision
InputPath=.\TerraVision.lic

"$(OutDir)\$(TargetName).lic" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "$(ProjDir)\$(TargetName).lic" "$(OutDir)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision_d.ddf

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TerraVision_d.inf

!IF  "$(CFG)" == "ActiveX_TV - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "ActiveX_TV - Win32 Unicode Release"

!ENDIF 

# End Source File
# End Target
# End Project
# Section ActiveX_TV : {BEB6E5D6-5455-4BF2-A70B-8BCA1B241C2E}
# 	2:21:DefaultSinkHeaderFile:terravision1.h
# 	2:16:DefaultSinkClass:CTerraVision
# End Section
# Section ActiveX_TV : {11705662-A9B7-47A1-A181-B2092EC633DC}
# 	2:5:Class:CTerraVision
# 	2:10:HeaderFile:terravision1.h
# 	2:8:ImplFile:terravision1.cpp
# End Section
