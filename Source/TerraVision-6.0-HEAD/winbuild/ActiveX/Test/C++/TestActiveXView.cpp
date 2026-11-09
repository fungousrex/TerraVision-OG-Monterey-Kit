#include "stdafx.h"
#include "TestActiveX.h"
#include "TestActiveXDoc.h"
#include "TestActiveXView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTestActiveXView, CView)

BEGIN_MESSAGE_MAP(CTestActiveXView, CView)
	//{{AFX_MSG_MAP(CTestActiveXView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
END_MESSAGE_MAP()

// CTestActiveXView construction/destruction
CTestActiveXView::CTestActiveXView() {
}

CTestActiveXView::~CTestActiveXView() {
}

BOOL CTestActiveXView::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CTestActiveXView drawing
void CTestActiveXView::OnDraw(CDC* pDC) {
	CTestActiveXDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

// CTestActiveXView diagnostics
#ifdef _DEBUG
void CTestActiveXView::AssertValid() const {
	CView::AssertValid();
}

void CTestActiveXView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CTestActiveXDoc* CTestActiveXView::GetDocument() {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTestActiveXDoc)));
	return (CTestActiveXDoc*)m_pDocument;
}
#endif //_DEBUG

// CTestActiveXView message handlers
int CTestActiveXView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	terravision = new CTerraVision;
	// TODO: Add your specialized creation code here
	terravision->Create(NULL, WS_VISIBLE, CRect(0, 0, 0, 0), this, 100); 

//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/gtopo30.oi",	"http://10.1.2.3/TileSets/Pyramids/gtopo30.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/sfbay.oi",		"http://10.1.2.3/TileSets/Pyramids/sfbay.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/paloalto.oi",	"http://10.1.2.3/TileSets/Pyramids/paloalto.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/benning.oi",	"http://10.1.2.3/TileSets/Pyramids/benning.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/monterey.oi",	"http://10.1.2.3/TileSets/Pyramids/monterey.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/pendleton.oi",	"http://10.1.2.3/TileSets/Pyramids/pendleton.dem");
//	terravision->loadTerrain("http://10.1.2.3/TileSets/Pyramids/yosemite.oi",	"http://10.1.2.3/TileSets/Pyramids/yosemite.dem");
//	terravision->SetViewpointURL("http://www.tvgeo.com/.TerraVisionViewpoints");

	terravision->loadTerrain("http://avila.ai.sri.com/TileSets/Pyramids/gtopo30.oi",	"http://avila.ai.sri.com/TileSets/Pyramids/gtopo30.dem");
//	terravision->SetSchema("http://www.tvgeo.com/TileSets/Schema/schema.xml");

	return 0;
}

void CTestActiveXView::OnSize(UINT nType, int cx, int cy) {
	CView::OnSize(nType, cx, cy);
	
	terravision->MoveWindow(0, 0, cx, cy);
}

BOOL CTestActiveXView::OnEraseBkgnd(CDC* pDC) {
	return false;
}


void CTestActiveXView::OnDestroy() {
	terravision->stopRendering();

	CView::OnDestroy();
}

void CTestActiveXView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class
	
	terravision->startRendering();

//	terravision.SetShowHUD(true);
//	terravision.SetShowLogo(true);

	terravision->Invalidate();
}
