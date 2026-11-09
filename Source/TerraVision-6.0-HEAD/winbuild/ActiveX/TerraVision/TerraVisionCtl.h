/*****************************************************************************
 * Name:
 *  TerraVisionCtl.h
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

#if !defined(AFX_TERRAVISIONCTL_H__E0A3EF22_8A3C_4842_A1E5_50A9BFC87EAA__INCLUDED_)
#define AFX_TERRAVISIONCTL_H__E0A3EF22_8A3C_4842_A1E5_50A9BFC87EAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tvWinDef.h"
#include "tvViewer.h"

// TerraVisionCtl.h : Declaration of the TerraVisionCtrl ActiveX Control class.

/////////////////////////////////////////////////////////////////////////////
// TerraVisionCtrl : See TerraVisionCtl.cpp for implementation.

class TerraVisionCtrl : public COleControl
{
	DECLARE_DYNCREATE(TerraVisionCtrl)

// Constructor
public:
	TerraVisionCtrl();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TerraVisionCtrl)
	public:
//	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	virtual DWORD GetControlFlags();
	virtual void Serialize(CArchive& ar);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	~TerraVisionCtrl();

	BEGIN_OLEFACTORY(TerraVisionCtrl)        // Class factory and guid
		virtual BOOL VerifyUserLicense();
		virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
	END_OLEFACTORY(TerraVisionCtrl)

	DECLARE_OLETYPELIB(TerraVisionCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(TerraVisionCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(TerraVisionCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(TerraVisionCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(TerraVisionCtrl)
	BOOL m_showWireframe;
	afx_msg void OnShowWireframeChanged();
	BOOL m_showLighting;
	afx_msg void OnShowLightingChanged();
	BOOL m_showHUD;
	afx_msg void OnShowHUDChanged();
	BOOL m_showTextures;
	afx_msg void OnShowTexturesChanged();
	BOOL m_showSky;
	afx_msg void OnShowSkyChanged();
	BOOL m_showGround;
	afx_msg void OnShowGroundChanged();
	BOOL m_showDatasets;
	afx_msg void OnShowDatasetsChanged();
	BOOL m_showFrustums;
	afx_msg void OnShowFrustumsChanged();
	BOOL m_showTiles;
	afx_msg void OnShowTilesChanged();
	BOOL m_showPaths;
	afx_msg void OnShowPathsChanged();
	BOOL m_showLogo;
	afx_msg void OnShowLogoChanged();
	BOOL m_showCrosshair;
	afx_msg void OnShowCrosshairChanged();
	BOOL m_showModels;
	afx_msg void OnShowModelsChanged();
	BOOL m_showFeaturesWireframe;
	afx_msg void OnShowFeaturesWireframeChanged();
	BOOL m_showFeaturesTextured;
	afx_msg void OnShowFeaturesTexturedChanged();
	BOOL m_showFeaturesLighting;
	afx_msg void OnShowFeaturesLightingChanged();
	BOOL m_featuresLoadTextures;
	afx_msg void OnFeaturesLoadTexturesChanged();
	BOOL m_featuresBackfaceCull;
	afx_msg void OnFeaturesBackfaceCullChanged();
	BOOL m_useHorizonCull;
	afx_msg void OnUseHorizonCullChanged();
	BOOL m_useZBufferHack;
	afx_msg void OnUseZBufferHackChanged();
	BOOL m_freezeTiles;
	afx_msg void OnFreezeTilesChanged();
	BOOL m_showTileDebug;
	afx_msg void OnShowTileDebugChanged();
	BOOL m_scaleIPD;
	afx_msg void OnScaleIPDChanged();
	BOOL m_useMipMapTextures;
	afx_msg void OnUseMipMapTexturesChanged();
	BOOL m_useBackfaceCull;
	afx_msg void OnUseBackfaceCullChanged();
	BOOL m_useHeightBasedNavigation;
	afx_msg void OnUseHeightBasedNavigationChanged();
	BOOL m_use2DConstrain;
	afx_msg void OnUse2DConstrainChanged();
	CString m_schema;
	afx_msg void OnSchemaChanged();
	CString m_viewpointURL;
	afx_msg void OnViewpointURLChanged();
	afx_msg void startRendering();
	afx_msg void stopRendering();
	afx_msg void flyTo(double x, double y, double z, double yaw, double pitch, double roll);
	afx_msg void getView(double FAR* x, double FAR* y, double FAR* z, double FAR* yaw, double FAR* pitch, double FAR* roll);
	afx_msg void getViewGDC(double FAR* latitude, double FAR* longitude, double FAR* elev);
	afx_msg void loadModel(LPCTSTR url);
	afx_msg void loadTerrain(LPCTSTR oi, LPCTSTR dem);
	afx_msg void message(LPCTSTR text);
	afx_msg void move(double forward, double right, double up);
	afx_msg void pathLoad(LPCTSTR text);
	afx_msg void pathReset();
	afx_msg void pathPlay();
	afx_msg void pathStop();
	afx_msg void pathPause();
	afx_msg void refresh();
	afx_msg void resetView();
	afx_msg void rotate(double yaw, double pitch, double roll);
	afx_msg void set(LPCTSTR option, LPCTSTR text);
	afx_msg void setView(double x, double y, double z, double yaw, double pitch, double roll);
	afx_msg void setViewGDC(double latitude, double longitude, double elevation, double yaw, double pitch, double roll);
	afx_msg void stopMotion();
	afx_msg void translate(double x, double y, double z);
	afx_msg void unloadModel(LPCTSTR url);
	afx_msg void unloadTerrain(LPCTSTR geoname);
	afx_msg void host(LPCTSTR host);
	afx_msg void port(LPCTSTR number);
	afx_msg void hostPort(LPCTSTR hostport);
	afx_msg void cmd(LPCTSTR cmd);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(TerraVisionCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(TerraVisionCtrl)
	dispidShowWireframe = 1L,
	dispidShowLighting = 2L,
	dispidShowHUD = 3L,
	dispidShowTextures = 4L,
	dispidShowSky = 5L,
	dispidShowGround = 6L,
	dispidShowDatasets = 7L,
	dispidShowFrustums = 8L,
	dispidShowTiles = 9L,
	dispidShowPaths = 10L,
	dispidShowLogo = 11L,
	dispidShowCrosshair = 12L,
	dispidShowModels = 13L,
	dispidShowFeaturesWireframe = 14L,
	dispidShowFeaturesTextured = 15L,
	dispidShowFeaturesLighting = 16L,
	dispidFeaturesLoadTextures = 17L,
	dispidFeaturesBackfaceCull = 18L,
	dispidUseHorizonCull = 19L,
	dispidUseZBufferHack = 20L,
	dispidFreezeTiles = 21L,
	dispidShowTileDebug = 22L,
	dispidScaleIPD = 23L,
	dispidUseMipMapTextures = 24L,
	dispidUseBackfaceCull = 25L,
	dispidUseHeightBasedNavigation = 26L,
	dispidUse2DConstrain = 27L,
	dispidSchema = 28L,
	dispidViewpointURL = 29L,
	dispidStartRendering = 30L,
	dispidStopRendering = 31L,
	dispidFlyTo = 32L,
	dispidGetView = 33L,
	dispidGetViewGDC = 34L,
	dispidLoadModel = 35L,
	dispidLoadTerrain = 36L,
	dispidMessage = 37L,
	dispidMove = 38L,
	dispidPathLoad = 39L,
	dispidPathReset = 40L,
	dispidPathPlay = 41L,
	dispidPathStop = 42L,
	dispidPathPause = 43L,
	dispidRefresh = 44L,
	dispidResetView = 45L,
	dispidRotate = 46L,
	dispidSet = 47L,
	dispidSetView = 48L,
	dispidSetViewGDC = 49L,
	dispidStopMotion = 50L,
	dispidTranslate = 51L,
	dispidUnloadModel = 52L,
	dispidUnloadTerrain = 53L,
	dispidHost = 54L,
	dispidPort = 55L,
	dispidHostPort = 56L,
	dispidCmd = 57L,
	//}}AFX_DISP_ID
	};

private:
	void				init();
	void				updateViewer();
	void				SetModifiedFlag(BOOL bModified = TRUE);

private:
	tvViewer		*viewer;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAVISIONCTL_H__E0A3EF22_8A3C_4842_A1E5_50A9BFC87EAA__INCLUDED)
