// TestActiveXDoc.cpp : implementation of the CTestActiveXDoc class
//

#include "stdafx.h"
#include "TestActiveX.h"

#include "TestActiveXDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestActiveXDoc

IMPLEMENT_DYNCREATE(CTestActiveXDoc, CDocument)

BEGIN_MESSAGE_MAP(CTestActiveXDoc, CDocument)
	//{{AFX_MSG_MAP(CTestActiveXDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestActiveXDoc construction/destruction

CTestActiveXDoc::CTestActiveXDoc()
{
	// TODO: add one-time construction code here

}

CTestActiveXDoc::~CTestActiveXDoc()
{
}

BOOL CTestActiveXDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTestActiveXDoc serialization

void CTestActiveXDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CTestActiveXDoc diagnostics

#ifdef _DEBUG
void CTestActiveXDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTestActiveXDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTestActiveXDoc commands
