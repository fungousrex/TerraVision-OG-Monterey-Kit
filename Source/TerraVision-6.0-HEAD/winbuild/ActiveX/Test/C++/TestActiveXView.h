#if !defined(AFX_TESTACTIVEXVIEW_H__FBD2BDE6_7361_4866_AE73_72EA5FD1AA6D__INCLUDED_)
#define AFX_TESTACTIVEXVIEW_H__FBD2BDE6_7361_4866_AE73_72EA5FD1AA6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TerraVision.h"

class CTestActiveXView : public CView {
protected: // create from serialization only
	CTestActiveXView();
	DECLARE_DYNCREATE(CTestActiveXView)

// Attributes
public:
	CTestActiveXDoc* GetDocument();

	CTerraVision *terravision;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestActiveXView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestActiveXView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestActiveXView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TestActiveXView.cpp
inline CTestActiveXDoc* CTestActiveXView::GetDocument()
   { return (CTestActiveXDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTACTIVEXVIEW_H__FBD2BDE6_7361_4866_AE73_72EA5FD1AA6D__INCLUDED_)
