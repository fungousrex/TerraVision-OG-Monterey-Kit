/*****************************************************************************
 * Name:
 *    	TerraVisionPpg.cpp
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
#include "TerraVisionPpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(TerraVisionPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(TerraVisionPropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(TerraVisionPropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(TerraVisionPropPage, "TerraVisionPropPage.1",
	0x708c5d6b, 0xb36d, 0x4208, 0xb8, 0x3b, 0x5e, 0xf, 0x7e, 0x29, 0xf8, 0xdd)


/////////////////////////////////////////////////////////////////////////////
// TerraVisionPropPage::TerraVisionPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for TerraVisionPropPage

BOOL TerraVisionPropPage::TerraVisionPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_TERRAVISION_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// TerraVisionPropPage::TerraVisionPropPage - Constructor

TerraVisionPropPage::TerraVisionPropPage() :
	COlePropertyPage(IDD, IDS_TERRAVISION_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(TerraVisionPropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// TerraVisionPropPage::DoDataExchange - Moves data between page and properties

void TerraVisionPropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(TerraVisionPropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// TerraVisionPropPage message handlers
