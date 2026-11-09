// TestActiveX.h : main header file for the TESTACTIVEX application
//

#if !defined(AFX_TESTACTIVEX_H__3FBC08B0_BBD2_43CE_A9D5_F03B244B245D__INCLUDED_)
#define AFX_TESTACTIVEX_H__3FBC08B0_BBD2_43CE_A9D5_F03B244B245D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestActiveXApp:
// See TestActiveX.cpp for the implementation of this class
//

class CTestActiveXApp : public CWinApp
{
public:
	CTestActiveXApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestActiveXApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTestActiveXApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTACTIVEX_H__3FBC08B0_BBD2_43CE_A9D5_F03B244B245D__INCLUDED_)
