/*****************************************************************************
 * Name:
 *    	TerraVision.h
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

#if !defined(AFX_TERRAVISION_H__71E69989_D1C3_432A_9E40_ADBC4C9E5487__INCLUDED_)
#define AFX_TERRAVISION_H__71E69989_D1C3_432A_9E40_ADBC4C9E5487__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TerraVision.h : main header file for TERRAVISION.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTerraVisionApp : See TerraVision.cpp for implementation.

class CTerraVisionApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAVISION_H__71E69989_D1C3_432A_9E40_ADBC4C9E5487__INCLUDED)
