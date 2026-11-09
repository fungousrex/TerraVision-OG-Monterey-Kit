# Microsoft Developer Studio Project File - Name="libtsm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtsm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtsm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtsm.mak" CFG="libtsm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtsm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtsm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtsm - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libtsm___Win32_Release"
# PROP BASE Intermediate_Dir "libtsm___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\.." /I "..\..\winbuild" /I "..\..\tsm" /I "..\..\tsm/winbuild" /I "..\..\tsm/winbuild/tsm" /I "../../nsprwin/include" /I "..\..\tiff/libtiff" /I "..\..\libxml" /I "..\..\libxml/winbuild" /I "..\..\libxml/winbuild/libxml" /I "..\..\jpeg" /I "..\..\jpeg/winbuild" /I "..\..\libungif/lib" /I "..\..\libpng" /I "..\..\zlib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_CONFIG_H" /D "TRACING" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtsm.lib"

!ELSEIF  "$(CFG)" == "libtsm - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\.." /I "..\..\winbuild" /I "..\..\tsm" /I "..\..\tsm/winbuild" /I "..\..\tsm/winbuild/tsm" /I "../../nsprwin/include" /I "..\..\tiff/libtiff" /I "..\..\libXML" /I "..\..\libXML/winbuild" /I "..\..\libxml/winbuild/libxml" /I "..\..\jpeg" /I "..\..\jpeg/winbuild" /I "..\..\libungif/lib" /I "..\..\libpng" /I "..\..\zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "HAVE_CONFIG_H" /D "TRACING" /YX /FD /GZ /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\libtsm_d.lib"

!ENDIF 

# Begin Target

# Name "libtsm - Win32 Release"
# Name "libtsm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\logging.c
# End Source File
# Begin Source File

SOURCE=..\orient.c
# End Source File
# Begin Source File

SOURCE=..\tsmAllocTspecs.c
# End Source File
# Begin Source File

SOURCE=..\tsmArg.c
# End Source File
# Begin Source File

SOURCE=..\tsmArray.c
# End Source File
# Begin Source File

SOURCE=..\tsmChecksum.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnect_wms.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnection.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnection_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnection_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnection_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnection_wms.c
# End Source File
# Begin Source File

SOURCE=..\tsmConnectionParams.c
# End Source File
# Begin Source File

SOURCE=..\tsmCreate.c
# End Source File
# Begin Source File

SOURCE=..\tsmCreate_RAW.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase_wms.c
# End Source File
# Begin Source File

SOURCE=..\tsmEmulate.c
# End Source File
# Begin Source File

SOURCE=..\tsmEmulate_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmEmulate_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmEmulate_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmGeoCoordBuffer.c
# End Source File
# Begin Source File

SOURCE=..\tsmGeoVRML.c
# End Source File
# Begin Source File

SOURCE=..\tsmHost.c
# End Source File
# Begin Source File

SOURCE=..\tsmHost_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmImage.c
# End Source File
# Begin Source File

SOURCE=..\tsmImageUtils.c
# End Source File
# Begin Source File

SOURCE=..\tsmJPEG.c
# End Source File
# Begin Source File

SOURCE=..\tsmLocalDbase.c
# End Source File
# Begin Source File

SOURCE=..\tsmLocalDbase_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmLocalDbase_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmLocalDbase_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmMem.c
# End Source File
# Begin Source File

SOURCE=..\tsmPurl.c
# End Source File
# Begin Source File

SOURCE=..\tsmPyramidInfo.c
# End Source File
# Begin Source File

SOURCE=..\tsmPyramidInfo_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmPyramidInfo_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmPyramidInfo_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDEM.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDOQ.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDTED.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsGIF.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsJPEG.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsPBM.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsRAW.c
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsTIFF.c
# End Source File
# Begin Source File

SOURCE=..\tsmRequest.c
# End Source File
# Begin Source File

SOURCE=..\tsmRequestSrv.c
# End Source File
# Begin Source File

SOURCE=..\tsmStringBuffer.c
# End Source File
# Begin Source File

SOURCE=..\tsmTile.c
# End Source File
# Begin Source File

SOURCE=..\tsmTile_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmTile_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmTile_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmTile_wms.c
# End Source File
# Begin Source File

SOURCE=..\tsmTileCoord.c
# End Source File
# Begin Source File

SOURCE=..\tsmTrace.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspec.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspec_dpss.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspec_local.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspec_web.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspec_wms.c
# End Source File
# Begin Source File

SOURCE=..\tsmTspecUtils.c
# End Source File
# Begin Source File

SOURCE=..\tsmUrlToFile.c
# End Source File
# Begin Source File

SOURCE=..\tsmUtils.c
# End Source File
# Begin Source File

SOURCE=..\tsmUTM.c
# End Source File
# Begin Source File

SOURCE=..\tsmVRML.c
# End Source File
# Begin Source File

SOURCE=..\tsmVRMLUtils.c
# End Source File
# Begin Source File

SOURCE=..\tsmWMS.c
# End Source File
# Begin Source File

SOURCE=..\tsmWMS.h
# End Source File
# Begin Source File

SOURCE=..\tsmWriteRows.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecGen.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecIn.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecOut.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecParse.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecUrls.c
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecUtils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\logging.h
# End Source File
# Begin Source File

SOURCE=..\orient.h
# End Source File
# Begin Source File

SOURCE=..\rpc_types.h
# End Source File
# Begin Source File

SOURCE=..\sih.h
# End Source File
# Begin Source File

SOURCE=..\sihDecs.h
# End Source File
# Begin Source File

SOURCE=..\sihProts.h
# End Source File
# Begin Source File

SOURCE=..\tsm.h
# End Source File
# Begin Source File

SOURCE=..\tsmArg.h
# End Source File
# Begin Source File

SOURCE=..\tsmArray.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnect_dpss.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnect_local.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnect_web.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnection.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnectionInt.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnectionParams.h
# End Source File
# Begin Source File

SOURCE=..\tsmConnectionParamsInt.h
# End Source File
# Begin Source File

SOURCE=..\tsmCreate.h
# End Source File
# Begin Source File

SOURCE=..\tsmDatabase.h
# End Source File
# Begin Source File

SOURCE=..\tsmEmulate.h
# End Source File
# Begin Source File

SOURCE=..\tsmEmulateInt.h
# End Source File
# Begin Source File

SOURCE=..\tsmExtProts.h
# End Source File
# Begin Source File

SOURCE=..\tsmGenArray.h
# End Source File
# Begin Source File

SOURCE=..\tsmGeoCoordBuffer.h
# End Source File
# Begin Source File

SOURCE=..\tsmGeoVRML.h
# End Source File
# Begin Source File

SOURCE=..\tsmImage.h
# End Source File
# Begin Source File

SOURCE=..\tsmInternal.h
# End Source File
# Begin Source File

SOURCE=..\tsmIntProts.h
# End Source File
# Begin Source File

SOURCE=..\tsmLAS_IO.h
# End Source File
# Begin Source File

SOURCE=..\tsmLocalDbase.h
# End Source File
# Begin Source File

SOURCE=..\tsmPackTspecs.h
# End Source File
# Begin Source File

SOURCE=..\tsmPurl.h
# End Source File
# Begin Source File

SOURCE=..\tsmPurlDefns.h
# End Source File
# Begin Source File

SOURCE=..\tsmPyramidInfo.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDEM.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDOQ.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsDTED.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsGIF.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsJPEG.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsPBM.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsRAW.h
# End Source File
# Begin Source File

SOURCE=..\tsmReadRowsTIFF.h
# End Source File
# Begin Source File

SOURCE=..\tsmRequest.h
# End Source File
# Begin Source File

SOURCE=..\tsmRequestSrv.h
# End Source File
# Begin Source File

SOURCE=..\tsmStringBuffer.h
# End Source File
# Begin Source File

SOURCE=..\tsmTile.h
# End Source File
# Begin Source File

SOURCE=..\tsmTileCoord.h
# End Source File
# Begin Source File

SOURCE=..\tsmTileInt.h
# End Source File
# Begin Source File

SOURCE=..\tsmTrace.h
# End Source File
# Begin Source File

SOURCE=..\tsmTspecUtils.h
# End Source File
# Begin Source File

SOURCE=..\tsmVRMLUtils.h
# End Source File
# Begin Source File

SOURCE=..\tsmWriteRows.h
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecDecs.h
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecExtProts.h
# End Source File
# Begin Source File

SOURCE=..\TsTileSpecIntProts.h
# End Source File
# End Group
# End Target
# End Project
