/*****************************************************************************
 * Name:
 *  TerraVisionCtl.cpp
 *
 * Description:
 *	MS-Windows specific routines
 *
 * Function List:
 *
 * Dependencies:
 *
 * Revision History:
 *     $Date: 2002/10/31 05:27:09 $
 *     $Author: reddy $
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
#include "TerraVisionCtl.h"
#include "TerraVisionPpg.h"

#include "tvViewer.h"

#include <oaa.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GLM_TIMER 125

IMPLEMENT_DYNCREATE(TerraVisionCtrl, COleControl)

static bool initialized;

// Message map
BEGIN_MESSAGE_MAP(TerraVisionCtrl, COleControl)
	//{{AFX_MSG_MAP(TerraVisionCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

// Dispatch map
BEGIN_DISPATCH_MAP(TerraVisionCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(TerraVisionCtrl)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showWireframe", m_showWireframe, OnShowWireframeChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showLighting", m_showLighting, OnShowLightingChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showHUD", m_showHUD, OnShowHUDChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showTextures", m_showTextures, OnShowTexturesChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showSky", m_showSky, OnShowSkyChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showGround", m_showGround, OnShowGroundChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showDatasets", m_showDatasets, OnShowDatasetsChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showFrustums", m_showFrustums, OnShowFrustumsChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showTiles", m_showTiles, OnShowTilesChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showPaths", m_showPaths, OnShowPathsChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showLogo", m_showLogo, OnShowLogoChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showCrosshair", m_showCrosshair, OnShowCrosshairChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showModels", m_showModels, OnShowModelsChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showFeaturesWireframe", m_showFeaturesWireframe, OnShowFeaturesWireframeChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showFeaturesTextured", m_showFeaturesTextured, OnShowFeaturesTexturedChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showFeaturesLighting", m_showFeaturesLighting, OnShowFeaturesLightingChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "featuresLoadTextures", m_featuresLoadTextures, OnFeaturesLoadTexturesChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "featuresBackfaceCull", m_featuresBackfaceCull, OnFeaturesBackfaceCullChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "useHorizonCull", m_useHorizonCull, OnUseHorizonCullChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "useZBufferHack", m_useZBufferHack, OnUseZBufferHackChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "freezeTiles", m_freezeTiles, OnFreezeTilesChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "showTileDebug", m_showTileDebug, OnShowTileDebugChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "scaleIPD", m_scaleIPD, OnScaleIPDChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "useMipMapTextures", m_useMipMapTextures, OnUseMipMapTexturesChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "useBackfaceCull", m_useBackfaceCull, OnUseBackfaceCullChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "useHeightBasedNavigation", m_useHeightBasedNavigation, OnUseHeightBasedNavigationChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "use2DConstrain", m_use2DConstrain, OnUse2DConstrainChanged, VT_BOOL)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "schema", m_schema, OnSchemaChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(TerraVisionCtrl, "viewpointURL", m_viewpointURL, OnViewpointURLChanged, VT_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "startRendering", startRendering, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "stopRendering", stopRendering, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "flyTo", flyTo, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "getView", getView, VT_EMPTY, VTS_PR8 VTS_PR8 VTS_PR8 VTS_PR8 VTS_PR8 VTS_PR8)
	DISP_FUNCTION(TerraVisionCtrl, "getViewGDC", getViewGDC, VT_EMPTY, VTS_PR8 VTS_PR8 VTS_PR8)
	DISP_FUNCTION(TerraVisionCtrl, "loadModel", loadModel, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "loadTerrain", loadTerrain, VT_EMPTY, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "message", message, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "move", move, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "pathLoad", pathLoad, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "pathReset", pathReset, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "pathPlay", pathPlay, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "pathStop", pathStop, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "pathPause", pathPause, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "refresh", refresh, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "resetView", resetView, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "rotate", rotate, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "set", set, VT_EMPTY, VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "setView", setView, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "setViewGDC", setViewGDC, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "stopMotion", stopMotion, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION(TerraVisionCtrl, "translate", translate, VT_EMPTY, VTS_R8 VTS_R8 VTS_R8)
	DISP_FUNCTION(TerraVisionCtrl, "unloadModel", unloadModel, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "unloadTerrain", unloadTerrain, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "host", host, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "port", port, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "hostPort", hostPort, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION(TerraVisionCtrl, "cmd", cmd, VT_EMPTY, VTS_BSTR)
	DISP_STOCKPROP_READYSTATE()
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(TerraVisionCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()

// Event map
BEGIN_EVENT_MAP(TerraVisionCtrl, COleControl)
	//{{AFX_EVENT_MAP(TerraVisionCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	EVENT_STOCK_READYSTATECHANGE()
	//}}AFX_EVENT_MAP
END_EVENT_MAP()

// Property pages
// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(TerraVisionCtrl, 1)
	PROPPAGEID(TerraVisionPropPage::guid)
END_PROPPAGEIDS(TerraVisionCtrl)

// Initialize class factory and guid
IMPLEMENT_OLECREATE_EX(TerraVisionCtrl, "TerraVisionCtrl",
	0xbeb6e5d6, 0x5455, 0x4bf2, 0xa7, 0xb, 0x8b, 0xca, 0x1b, 0x24, 0x1c, 0x2e)

// Type library ID and version
IMPLEMENT_OLETYPELIB(TerraVisionCtrl, _tlid, _wVerMajor, _wVerMinor)

// Interface IDs
const IID BASED_CODE IID_DTerraVision =
		{ 0x11705662, 0xa9b7, 0x47a1, { 0xa1, 0x81, 0xb2, 0x9, 0x2e, 0xc6, 0x33, 0xdc } };
const IID BASED_CODE IID_DTerraVisionEvents =
		{ 0xd385e174, 0x7562, 0x4936, { 0xba, 0x95, 0x6f, 0x1, 0x26, 0, 0xa4, 0x7d } };

// Control type information
static const DWORD BASED_CODE _dwTerraVisionOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(TerraVisionCtrl, IDS_TERRAVISION, _dwTerraVisionOleMisc)

// TerraVisionCtrl::TerraVisionCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for TerraVisionCtrl
BOOL TerraVisionCtrl::TerraVisionCtrlFactory::UpdateRegistry(BOOL bRegister) {
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_TERRAVISION,
			IDB_TERRAVISION,
			afxRegInsertable | afxRegApartmentThreading,
			_dwTerraVisionOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}

// Licensing strings
static const TCHAR BASED_CODE _szLicFileName[] = _T("TerraVision.lic");
static const WCHAR BASED_CODE _szLicString[] =	L"Copyright (c) 1999-2001 SRI International. All rights reserved.";

// TerraVisionCtrl::TerraVisionCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license
BOOL TerraVisionCtrl::TerraVisionCtrlFactory::VerifyUserLicense() {
	return AfxVerifyLicFile(AfxGetInstanceHandle(), _szLicFileName,	_szLicString);
}

// TerraVisionCtrl::TerraVisionCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key
BOOL TerraVisionCtrl::TerraVisionCtrlFactory::GetLicenseKey(DWORD dwReserved,	BSTR FAR* pbstrKey) {
	if (pbstrKey == NULL)
		return FALSE;

	*pbstrKey = SysAllocString(_szLicString);
	return (*pbstrKey != NULL);
}

// TerraVisionCtrl::TerraVisionCtrl - Constructor
TerraVisionCtrl::TerraVisionCtrl() {
	InitializeIIDs(&IID_DTerraVision, &IID_DTerraVisionEvents);

	m_lReadyState = READYSTATE_LOADING;
	// TODO: Call InternalSetReadyState when the readystate changes.

	viewer = NULL;
}

// TerraVisionCtrl::~TerraVisionCtrl - Destructor
TerraVisionCtrl::~TerraVisionCtrl() {
}

// TerraVisionCtrl::DoPropExchange - Persistence support
void TerraVisionCtrl::DoPropExchange(CPropExchange* pPX) {
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	PX_Bool(pPX, _T("featuresBackfaceCull"),			m_featuresBackfaceCull,			false);
	PX_Bool(pPX, _T("featuresLoadTextures"),			m_featuresLoadTextures,		  false);
	PX_Bool(pPX, _T("freezeTiles"),								m_freezeTiles,							false);
	PX_Bool(pPX, _T("scaleIPD"),									m_scaleIPD,									true);
	PX_Bool(pPX, _T("showCrosshair"),							m_showCrosshair,						true);
	PX_Bool(pPX, _T("showDatasets"),							m_showDatasets,							false);
	PX_Bool(pPX, _T("showFeaturesWireframe"),			m_showFeaturesWireframe,		false);
	PX_Bool(pPX, _T("showFeaturesTextured"),			m_showFeaturesTextured,			false);
	PX_Bool(pPX, _T("showFeaturesLighting"),			m_showFeaturesLighting,			true);
	PX_Bool(pPX, _T("showFrustums"),							m_showFrustums,							false);
	PX_Bool(pPX, _T("showGround"),								m_showGround,								true);
	PX_Bool(pPX, _T("showHUD"),										m_showHUD,									true);
	PX_Bool(pPX, _T("showLightinge"),							m_showLighting,							true);
	PX_Bool(pPX, _T("showLogo"),									m_showLogo,									false);
	PX_Bool(pPX, _T("showModels"),								m_showModels,								true);
	PX_Bool(pPX, _T("showPaths"),									m_showPaths,								false);
	PX_Bool(pPX, _T("showSky"),										m_showSky,									true);
	PX_Bool(pPX, _T("showTextures"),							m_showTextures,							true);
	PX_Bool(pPX, _T("showTileDebug"),							m_showTileDebug,						false);
	PX_Bool(pPX, _T("showTiles"),									m_showTiles,								false);
	PX_Bool(pPX, _T("showWireframe"),							m_showWireframe,						false);
	PX_Bool(pPX, _T("use2DConstrain"),						m_use2DConstrain,						false);
	PX_Bool(pPX, _T("useBackfaceCull"),						m_useBackfaceCull,					true);
	PX_Bool(pPX, _T("useHeightBasedNavigation"),	m_useHeightBasedNavigation,	true);
	PX_Bool(pPX, _T("useHorizonCull"),						m_useHorizonCull,						true);
	PX_Bool(pPX, _T("useMipMapTextures"),					m_useMipMapTextures,				true);
	PX_Bool(pPX, _T("useZBufferHack"),						m_useZBufferHack,						true);
	PX_String(pPX, _T("schema"),									m_schema,										"");
	PX_String(pPX, _T("viewpointURL"),						m_viewpointURL,							"");
}

// TerraVisionCtrl::GetControlFlags -
// Flags to customize MFC's implementation of ActiveX controls.
//
// For information on using these flags, please see MFC technical note
// #nnn, "Optimizing an ActiveX Control".
DWORD TerraVisionCtrl::GetControlFlags() {
	DWORD dwFlags = COleControl::GetControlFlags();

	// The control will not be redrawn when making the transition
	// between the active and inactivate state.
	dwFlags |= noFlickerActivate;

	// The control can optimize its OnDraw method, by not restoring
	// the original GDI objects in the device context.
	dwFlags |= canOptimizeDraw;
	return dwFlags & ~clipPaintDC;
}

// TerraVisionCtrl::OnResetState - Reset control to default state
void TerraVisionCtrl::OnResetState() {
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange
}

// TerraVisionCtrl::AboutBox - Display an "About" box to the user
void TerraVisionCtrl::AboutBox() {
	CDialog dlgAbout(IDD_ABOUTBOX_TERRAVISION);
	dlgAbout.DoModal();
}

void TerraVisionCtrl::Serialize(CArchive& ar) {
	COleControl::Serialize(ar);

  if (ar.IsStoring()) {
		ar << m_featuresBackfaceCull;
		ar << m_featuresLoadTextures;
		ar << m_freezeTiles;
		ar << m_scaleIPD;
		ar << m_showCrosshair;
		ar << m_showDatasets;
		ar << m_showFeaturesWireframe;
		ar << m_showFeaturesTextured;
		ar << m_showFeaturesLighting;
		ar << m_showFrustums;
		ar << m_showGround;
		ar << m_showHUD;
		ar << m_showLighting;
		ar << m_showLogo;
		ar << m_showModels;
		ar << m_showPaths;
		ar << m_showSky;
		ar << m_showTextures;
		ar << m_showTileDebug;
		ar << m_showTiles;
		ar << m_showWireframe;
		ar << m_use2DConstrain;
		ar << m_useBackfaceCull;
		ar << m_useHeightBasedNavigation;
		ar << m_useHorizonCull;
		ar << m_useMipMapTextures;
		ar << m_useZBufferHack;
		ar << m_schema;
		ar << m_viewpointURL;
	}
  else {
		ar >> m_featuresBackfaceCull;
		ar >> m_featuresLoadTextures;
		ar >> m_freezeTiles;
		ar >> m_scaleIPD;
		ar >> m_showCrosshair;
		ar >> m_showDatasets;
		ar >> m_showFeaturesWireframe;
		ar >> m_showFeaturesTextured;
		ar >> m_showFeaturesLighting;
		ar >> m_showFrustums;
		ar >> m_showGround;
		ar >> m_showHUD;
		ar >> m_showLighting;
		ar >> m_showLogo;
		ar >> m_showModels;
		ar >> m_showPaths;
		ar >> m_showSky;
		ar >> m_showTextures;
		ar >> m_showTileDebug;
		ar >> m_showTiles;
		ar >> m_showWireframe;
		ar >> m_use2DConstrain;
		ar >> m_useBackfaceCull;
		ar >> m_useHeightBasedNavigation;
		ar >> m_useHorizonCull;
		ar >> m_useMipMapTextures;
		ar >> m_useZBufferHack;
		ar >> m_schema;
		ar >> m_viewpointURL;
  }
}

// -----------------   TerraVisionCtrl message handlers

int TerraVisionCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	InternalSetReadyState(READYSTATE_UNINITIALIZED);

	viewer = new tvViewer;
	viewer->init(m_hWnd);

	InternalSetReadyState(READYSTATE_COMPLETE);

	initialized = false;

	return 0;
}

void TerraVisionCtrl::OnDestroy() {
	viewer->stopRendering();

	initialized = false;

	if (viewer != NULL) {
	  delete viewer;
	  viewer = NULL;
	}

  COleControl::OnDestroy();
}

BOOL TerraVisionCtrl::OnEraseBkgnd(CDC* pDC) {
	return false;
}

void TerraVisionCtrl::OnSize(UINT nType, int cx, int cy) {
	COleControl::OnSize(nType, cx, cy);

	tvEvent event;
	
	if (viewer == NULL)	return; 
	
	event.id = tvEvent::WINDOW_RESIZED;
	event.x = cx;
	event.y = cy;

	viewer->processEvent(&event);
}

void TerraVisionCtrl::OnTimer(UINT nIDEvent) {
	if (viewer == NULL) return;

	viewer->render();

	COleControl::OnTimer(nIDEvent);
}

void TerraVisionCtrl::OnLButtonDown(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
	event.id = tvEvent::MOUSE_DOWN;
	event.button = tvEvent::LEFT_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;

	viewer->processEvent(&event);

	COleControl::OnLButtonDown(nFlags, point);
}

void TerraVisionCtrl::OnLButtonUp(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
	event.id = tvEvent::MOUSE_UP;
	event.button = tvEvent::LEFT_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;

  viewer->processEvent(&event);

	COleControl::OnLButtonUp(nFlags, point);
}

void TerraVisionCtrl::OnMButtonUp(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
  event.id = tvEvent::MOUSE_UP;
	event.button = tvEvent::MIDDLE_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;
	
  viewer->processEvent(&event);

	COleControl::OnMButtonUp(nFlags, point);
}

void TerraVisionCtrl::OnMButtonDown(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
  event.id = tvEvent::MOUSE_DOWN;
	event.button = tvEvent::MIDDLE_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;
	
  viewer->processEvent(&event);

	COleControl::OnMButtonDown(nFlags, point);
}

void TerraVisionCtrl::OnMouseMove(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
	event.id = tvEvent::MOUSE_MOVE;
	event.x = point.x;
	event.y = point.y;

	viewer->processEvent(&event);

	COleControl::OnMouseMove(nFlags, point);
}

void TerraVisionCtrl::OnRButtonDown(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
  event.id = tvEvent::MOUSE_DOWN;
	event.button = tvEvent::RIGHT_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;
	
  viewer->processEvent(&event);

	COleControl::OnRButtonDown(nFlags, point);
}

void TerraVisionCtrl::OnRButtonUp(UINT nFlags, CPoint point) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
  event.id = tvEvent::MOUSE_UP;
	event.button = tvEvent::RIGHT_MOUSE_BUTTON;
	event.x = point.x;
	event.y = point.y;
	
  viewer->processEvent(&event);

	COleControl::OnRButtonUp(nFlags, point);
}

void TerraVisionCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
	event.key = 0;
	event.modifier = 0;
	switch (nChar) {
		case VK_SPACE:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_SPACE;
			break;

		case VK_LEFT:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_LEFT;
			break;
		
		case VK_RIGHT:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_RIGHT;
			break;
		
		case VK_UP:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_UP;
			break;

		case VK_DOWN:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_DOWN;
			break;

		case VK_NEXT:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_PAGE_DOWN;
			break;

		case VK_PRIOR:
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_PAGE_UP;
			break;

		case VK_SHIFT:
			event.id = tvEvent::KEY_ACTION;
			event.modifier = tvEvent::SHIFT_KEY_PRESSED;
			break;

		case VK_CONTROL:
			event.id = tvEvent::KEY_ACTION;
			event.modifier = tvEvent::CTRL_KEY_PRESSED;
			break;

		case VK_MENU:
			event.id = tvEvent::KEY_ACTION;
			event.modifier = tvEvent::ALT_KEY_PRESSED;
			break;

		case VK_BACK :  // Backspace
			event.id = tvEvent::KEY_PRESS;
			event.key = tvEvent::KEY_BACK_SPACE;
			break;

		default:
			event.id = tvEvent::KEY_PRESS;
			event.key = nChar;
			break;
	}

  viewer->processEvent(&event);

	COleControl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void TerraVisionCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (viewer == NULL)	return; 
	
	tvEvent event;
	event.key = 0;
	event.modifier = 0;

	switch(nChar) {
		case VK_SHIFT:
			event.id = tvEvent::KEY_ACTION_RELEASE;
			event.modifier = tvEvent::SHIFT_KEY_PRESSED;
			break;

		case VK_CONTROL:
			event.id = tvEvent::KEY_ACTION_RELEASE;
			event.modifier = tvEvent::CTRL_KEY_PRESSED;
			break;

		case VK_MENU:
			event.id = tvEvent::KEY_ACTION_RELEASE;
			event.modifier = tvEvent::ALT_KEY_PRESSED;
			break;
	}

  viewer->processEvent(&event);

	COleControl::OnKeyUp(nChar, nRepCnt, nFlags);
}
/*
void TerraVisionCtrl::setViewpointURL(LPCTSTR url) {
	if (viewer == NULL) return;

	viewer->setViewpointURL( url );
}
*/
void TerraVisionCtrl::startRendering() {
	if (viewer == NULL) return;

  // start the glm work proc 
	SetTimer(GLM_TIMER, 5, NULL);

	// Set control size, in case OnSize os not called
	tvEvent event;
	CRect rect;

	GetClientRect(rect);
	event.id = tvEvent::WINDOW_RESIZED;
	event.x = rect.right;
	event.y = rect.bottom;
	viewer->processEvent(&event);

  // update viewer properties and datasets
	updateViewer();

	// Start rendering thread
	viewer->startRendering();

	// Force redraw
	InvalidateControl();
}

void TerraVisionCtrl::stopRendering() {
	if (viewer == NULL) return;

	KillTimer(GLM_TIMER);

	viewer->stopRendering();

	// Force redraw
	//InvalidateControl();
}

void TerraVisionCtrl::updateViewer() {
	if (viewer == NULL) return;

	tvBase *core;

	if ((core = viewer->getTVBase()) == NULL) return;

	tvSetBoolean( core, TV_FEATURES_BACKFACE_CULL, m_featuresBackfaceCull );
	tvSetBoolean( core, TV_FEATURES_LOAD_TEXTURES, m_featuresLoadTextures );
	tvSetBoolean( core, TV_FREEZE_TILES, m_freezeTiles );
	tvSetBoolean( core, TV_SCALE_IPD, m_scaleIPD );
	tvSetBoolean( core, TV_SHOW_CROSSHAIR, m_showCrosshair );
	tvSetBoolean( core, TV_SHOW_DATASETS, m_showDatasets );
	tvSetBoolean( core, TV_FEATURES_WIREFRAME, m_showFeaturesWireframe );
	tvSetBoolean( core, TV_FEATURES_TEXTURED, m_showFeaturesTextured );
	tvSetBoolean( core, TV_FEATURES_LIGHTING, m_showFeaturesLighting );
	tvSetBoolean( core, TV_SHOW_FRUSTUMS, m_showFrustums );
	tvSetBoolean( core, TV_SHOW_GROUND, m_showGround );
	tvSetBoolean( core, TV_SHOW_HEADS_UP, m_showHUD );
	tvSetBoolean( core, TV_SHOW_LIGHTING, m_showLighting );
	tvSetBoolean( core, TV_SHOW_LOGO, m_showLogo );
	tvSetBoolean( core, TV_MIPMAP_TEXTURES, m_useMipMapTextures );
	tvSetBoolean( core, TV_SHOW_MODELS, m_showModels );
	tvSetBoolean( core, TV_SHOW_PATHS, m_showPaths );
	tvSetBoolean( core, TV_SHOW_SKY, m_showSky );
	tvSetBoolean( core, TV_SHOW_TEXTURES, m_showTextures );
	tvSetBoolean( core, TV_SHOW_TILE_DEBUG, m_showTileDebug );
	tvSetBoolean( core, TV_SHOW_TILES, m_showTiles );
	tvSetBoolean( core, TV_SHOW_WIREFRAME, m_showWireframe );
	tvSetBoolean( core, TV_TWOD_CONSTRAIN, m_use2DConstrain );
	tvSetBoolean( core, TV_BACKFACE_CULL, m_useBackfaceCull );
	tvSetBoolean( core, TV_HEIGHT_BASED_NAVIGATION, m_useHeightBasedNavigation );
	tvSetBoolean( core, TV_HORIZON_CULL, m_useHorizonCull );
	tvSetBoolean( core, TV_USE_ZBUFFER_HACK, m_useZBufferHack );
	viewer->setViewpointURL( m_viewpointURL );
}

void TerraVisionCtrl::OnShowWireframeChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowLightingChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowHUDChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowTexturesChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowSkyChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowGroundChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowDatasetsChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowFrustumsChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowTilesChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowPathsChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowLogoChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowCrosshairChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowModelsChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowFeaturesWireframeChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowFeaturesTexturedChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowFeaturesLightingChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnFeaturesLoadTexturesChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnFeaturesBackfaceCullChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUseHorizonCullChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUseZBufferHackChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnFreezeTilesChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnShowTileDebugChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnScaleIPDChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUseMipMapTexturesChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUseBackfaceCullChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUseHeightBasedNavigationChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnUse2DConstrainChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::SetModifiedFlag(BOOL bModified) {
  // update viewer properties and datasets
	updateViewer();
	
	COleControl::SetModifiedFlag(bModified);
}

BOOL TerraVisionCtrl::PreCreateWindow(CREATESTRUCT& cs) {
	cs.style |= ( WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CS_OWNDC );
	
	return COleControl::PreCreateWindow(cs);
}

void TerraVisionCtrl::flyTo(double x, double y, double z, double yaw, double pitch, double roll) {
	char command[256];

	sprintf(command, "flyto(%f,%f,%f,%f,%f,%f)", x, y, z, yaw, pitch, roll);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::getView(double FAR* x, double FAR* y, double FAR* z, double FAR* yaw, double FAR* pitch, double FAR* roll) {
	char command[256];

	sprintf(command, "getview(%f,%f,%f,%f,%f,%f)", x, y, z, yaw, pitch, roll);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::getViewGDC(double FAR* latitude, double FAR* longitude, double FAR* elev) {
	char command[256];

	sprintf(command, "getview_gdc(%f,%f,%f)", latitude, longitude, elev);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::loadModel(LPCTSTR url) {
	char command[256];

	sprintf(command, "loadmodel(\"%s\")", url);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::loadTerrain(LPCTSTR oi, LPCTSTR dem) {
	char *command = (char*)malloc( sizeof(char) * 1024 );

	sprintf(command, "loadterrain(\"%s\",\"%s\")", oi, dem);
	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::message(LPCTSTR text) {
	char command[256];

	sprintf(command, "message(\"%s\")", text);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::move(double forward, double right, double up) {
	char command[256];

	sprintf(command, "move(%f,%f,%f)", forward, right, up);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::pathLoad(LPCTSTR text) {
	char command[256];

	sprintf(command, "pathload(\"%s\")", text);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::pathReset() {
	char command[256];

	sprintf(command, "pathreset");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::pathPlay() {
	char command[256];

	sprintf(command, "pathplay");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::pathStop() {
	char command[256];

	sprintf(command, "pathstop");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::pathPause() {
	char command[256];

	sprintf(command, "pathpause");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::refresh() {
	char command[256];

	sprintf(command, "refresh");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::resetView() {
	char command[256];

	sprintf(command, "resetview");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::rotate(double yaw, double pitch, double roll) {
	char command[256];

	sprintf(command, "rotate(%f,%f,%f)", yaw, pitch, roll);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::set(LPCTSTR option, LPCTSTR text) {
	char command[256];

	sprintf(command, "set(\"%s\",\"%s\")", option, text);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::setView(double x, double y, double z, double yaw, double pitch, double roll) {
	char command[256];

	sprintf(command, "setview(%f,%f,%f,%f,%f,%f)", x, y, z, yaw, pitch, roll);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::setViewGDC(double latitude, double longitude, double elevation, double yaw, double pitch, double roll) {
	char command[256];

	sprintf(command, "setview_gdc(%f,%f,%f,%f,%f,%f)", latitude, longitude, elevation, yaw, pitch, roll);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::stopMotion() {
	char command[256];

	sprintf(command, "stopmotion");

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::translate(double x, double y, double z) {
	char command[256];

	sprintf(command, "translate(%f,%f,%f)", x, y, z);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::unloadModel(LPCTSTR url) {
	char command[256];

	sprintf(command, "unloadmodel(\"%s\")", url);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::unloadTerrain(LPCTSTR geoname) {
	char command[256];

	sprintf(command, "unloadterrain(\"%s\")", geoname);

	tvInterpret(viewer->getTVBase(), command);
}

void TerraVisionCtrl::host(LPCTSTR host) {
	SetOAAHost(host);
}

void TerraVisionCtrl::port(LPCTSTR number) {
	SetOAAPort( atoi(number) );
}

void TerraVisionCtrl::hostPort(LPCTSTR hostport) {
	SetOAAHostPort( hostport );
}

void TerraVisionCtrl::cmd(LPCTSTR cmd) {
	tvInterpret(viewer->getTVBase(), cmd);
}

void TerraVisionCtrl::OnSchemaChanged() {
	if (viewer == NULL) return;

	tvBase *core;

	if ((core = viewer->getTVBase()) == NULL) return;

	tvSetString( core, TV_SCHEMA_URL, m_schema );
  tvSetString( core, TV_SPLASH_MESSAGE, "Querying .geo for initial datasets...");
}

void TerraVisionCtrl::OnViewpointURLChanged() {
	SetModifiedFlag();
}

void TerraVisionCtrl::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	COleControl::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}

void TerraVisionCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (initialized == false) {
		startRendering();
		initialized = true;
	}
}
