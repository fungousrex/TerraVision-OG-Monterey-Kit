/*****************************************************************************
 * Name:
 *    	TerraVisionPpg.h
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

#if !defined(AFX_TERRAVISIONPPG_H__9A505350_3D6E_462A_AB6F_9F5EEB8B5047__INCLUDED_)
#define AFX_TERRAVISIONPPG_H__9A505350_3D6E_462A_AB6F_9F5EEB8B5047__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TerraVisionPpg.h : Declaration of the TerraVisionPropPage property page class.

////////////////////////////////////////////////////////////////////////////
// TerraVisionPropPage : See TerraVisionPpg.cpp.cpp for implementation.

class TerraVisionPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(TerraVisionPropPage)
	DECLARE_OLECREATE_EX(TerraVisionPropPage)

// Constructor
public:
	TerraVisionPropPage();

// Dialog Data
	//{{AFX_DATA(TerraVisionPropPage)
	enum { IDD = IDD_PROPPAGE_TERRAVISION };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(TerraVisionPropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAVISIONPPG_H__9A505350_3D6E_462A_AB6F_9F5EEB8B5047__INCLUDED)
