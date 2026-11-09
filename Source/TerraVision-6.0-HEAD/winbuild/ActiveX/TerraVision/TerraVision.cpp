/*****************************************************************************
* Name:
*    	TerraVision.cpp
*
* Description:
*	MS-Windows specific routines
*
* Function List:
*
* Dependencies:
*
*  ----- BEGIN LICENSE BLOCK -----
*  Version: MPL 1.1
* 
*  The contents of this file are subject to the Mozilla Public License Version
*  1.1 (the "License"); you may not use this file except in compliance with
*  the License. You may obtain a copy of the License at
*  http://www.mozilla.org/MPL/
* 
*  Software distributed under the License is distributed on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
*  for the specific language governing rights and limitations under the
*  License.
* 
*  The Original Code is the TerraVision 3D Terrain Visualization System.
* 
*  The Initial Developer of the Original Code is SRI International.
*
*  Portions created by the Initial Developer are Copyright (C) 1992-2002
*  the Initial Developer. All Rights Reserved.
* 
*  Contributor(s): Michael Eriksen
*  ----- END LICENSE BLOCK -----
*
*****************************************************************************/

#include "stdafx.h"
#include "TerraVision.h"

#include <objsafe.h>
#include <comcat.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTerraVisionApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
{0x10b518e7, 0x997e, 0x42a2, { 0xa7, 0x9a, 0x4f, 0xf5, 0x64, 0x65, 0x8b, 0xb3 } };

const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

const GUID CDECL BASED_CODE CLSID_SafeItem =
{0xbeb6e5d6, 0x5455, 0x4bf2, { 0xa7, 0xb, 0x8b, 0xca, 0x1b, 0x24, 0x1c, 0x2e} };

// Helper function to create a component category and associated
// description
HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription) {
	CATEGORYINFO catinfo;
	ICatRegister* pcr = NULL ;
	HRESULT hr = S_OK ;
	int len;
	
	hr = CoCreateInstance ((REFCLSID) CLSID_StdComponentCategoriesMgr, 
		(LPUNKNOWN) NULL, CLSCTX_INPROC_SERVER, 
		(REFIID) IID_ICatRegister, (LPVOID *)&pcr);
	if (FAILED(hr))
		return hr;
	
	// Make sure the HKCR\Component Categories\{..catid...}
	// key is registered
	catinfo.catid = catid;
	catinfo.lcid = 0x0409 ; // english
	
	// Make sure the provided description is not too long.
	// Only copy the first 127 characters if it is
	len = wcslen(catDescription);
	if (len>127)
		len = 127;
	wcsncpy(catinfo.szDescription, catDescription, len);
	// Make sure the description is null terminated
	catinfo.szDescription[len] = '\0';
	
	hr = pcr->RegisterCategories (1, &catinfo);
	pcr->Release();
	return hr;
}

// Helper function to register a CLSID as belonging to a component
// category
HRESULT RegisterCLSIDInCategory (REFCLSID clsid, CATID catid) {
	// Register your component categories information.
	ICatRegister* pcr = NULL ;
	HRESULT hr = S_OK ;
	hr = CoCreateInstance ((REFCLSID) CLSID_StdComponentCategoriesMgr, 
		(LPUNKNOWN) NULL, CLSCTX_INPROC_SERVER, 
		(REFIID) IID_ICatRegister, (LPVOID *)&pcr);
	
	if (SUCCEEDED(hr)) {
		// Register this category as being "implemented" by
		// the class.
		CATID rgcatid[1] ;
		rgcatid[0] = catid;
		hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
	}
	
	if (pcr != NULL)
		pcr->Release();
  
	return hr;
}

HRESULT UnRegisterCLSIDInCategory (REFCLSID clsid, CATID catid) {
	ICatRegister* pcr = NULL ;
	HRESULT hr = S_OK ;
	hr = CoCreateInstance ((REFCLSID) CLSID_StdComponentCategoriesMgr, 
		(LPUNKNOWN) NULL, CLSCTX_INPROC_SERVER, 
		(REFIID) IID_ICatRegister, (LPVOID *)&pcr);
	if (SUCCEEDED(hr)) {
		// Unregister this category as being "implemented" by
		// the class.
		CATID rgcatid[1] ;
		rgcatid[0] = catid;
		hr = pcr->UnRegisterClassImplCategories (clsid, 1, rgcatid);
	}
	
	if (pcr != NULL)
		pcr->Release();
  
	return hr;
}

////////////////////////////////////////////////////////////////////////////
// CTerraVisionApp::InitInstance - DLL initialization

BOOL CTerraVisionApp::InitInstance() {
	BOOL bInit = COleControlModule::InitInstance();
	
	if (bInit) {
		// TODO: Add your own module initialization code here.
	}
	
	return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CTerraVisionApp::ExitInstance - DLL termination

int CTerraVisionApp::ExitInstance() {
	// TODO: Add your own module termination code here.
	
	return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void) {
	HRESULT hr;
	
	AFX_MANAGE_STATE(_afxModuleAddrThis);
	
	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);
	
	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);
	
	// Mark the control as safe for initializing.
	hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
	hr = RegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForInitializing);

	// Mark the control as safe for scripting.
	hr = CreateComponentCategory(CATID_SafeForScripting,	L"Controls that are safely scriptable");
	hr = RegisterCLSIDInCategory(CLSID_SafeItem, CATID_SafeForScripting);
	
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void) {
	AFX_MANAGE_STATE(_afxModuleAddrThis);
	
	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);
	
	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);
	
	return NOERROR;
}
