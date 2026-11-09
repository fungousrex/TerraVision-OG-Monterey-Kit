// TestActiveXDoc.h : interface of the CTestActiveXDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTACTIVEXDOC_H__B41E32B2_18DF_47E7_B89E_E62FDA1CD38F__INCLUDED_)
#define AFX_TESTACTIVEXDOC_H__B41E32B2_18DF_47E7_B89E_E62FDA1CD38F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTestActiveXDoc : public CDocument
{
protected: // create from serialization only
	CTestActiveXDoc();
	DECLARE_DYNCREATE(CTestActiveXDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestActiveXDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTestActiveXDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTestActiveXDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTACTIVEXDOC_H__B41E32B2_18DF_47E7_B89E_E62FDA1CD38F__INCLUDED_)
