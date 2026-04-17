// XTPSkinManager.cpp: implementation of the CXTPSkinManager class.
//
// (c)1998-2018 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Common/XTPFramework.h"
#define XTP_INTERNAL_UXTHEME_INCLUSION
#include "Common/Uxtheme.h"
#include "Common/Tmschema.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPColorManager.h"
#include "Common/ScrollBar/XTPScrollInfo.h"
#include "Common/ScrollBar/XTPScrollBase.h"
#include "Common/XTPSynchro.h"

#include "SkinFramework/XTPSkinManagerColorFilter.h"
#include "SkinFramework/XTPSkinManager.h"
#include "SkinFramework/XTPSkinImage.h"
#include "SkinFramework/XTPSkinManagerResource.h"
#include "SkinFramework/XTPSkinObject.h"
#include "SkinFramework/XTPSkinObjectFrame.h"
#include "SkinFramework/XTPSkinObjectMenu.h"
#include "SkinFramework/XTPSkinObjectComboBox.h"
#include "SkinFramework/XTPSkinManagerSchema.h"
#include "SkinFramework/XTPSkinManagerApiHook.h"
#include "SkinFramework/XTPSkinManagerModuleList.h"
#include "SkinFramework/XTPSkinDrawTools.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

typedef CXTPSingleton<CXTPSkinManager, CXTPSingletonDependencies<
	CXTPSingleton<CXTPSkinManagerModuleListSharedData>
> > XTPSkinManagerSingleton;

#define EVENT_OBJECT_REORDER 0x8004

///////////////////////////////////////////////////////////////////////////////
// CXTPSkinManagerMetrics::CHandleRegister

class CXTPSkinManagerMetrics::CHandleRegister : public CXTPSynchronized
{
	friend class CXTPSingleton<CXTPSkinManagerMetrics::CHandleRegister>;

	CHandleRegister()
	{
	}

	~CHandleRegister()
	{
		ASSERT("Not all handles are unregistered, potential leak detected." && 0 == m_Handles.GetCount());
	}

public:

	static CHandleRegister& Instance()
	{
		return CXTPSingleton<CXTPSkinManagerMetrics::CHandleRegister>::Instance();
	}

	void Register(HGDIOBJ hObject)
	{
		XTP_GUARD_EXCLUSIVE(CHandleRegister, this)
		{
			if (!IsRegisteredInternal(hObject))
			{
				m_Handles.SetAt(hObject, TRUE);
			}
		}
	}

	void Unregister(HGDIOBJ hObject)
	{
		XTP_GUARD_EXCLUSIVE(CHandleRegister, this)
		{
			m_Handles.RemoveKey(hObject);
		}
	}

	BOOL IsRegistered(HGDIOBJ hObject) const
	{
		XTP_GUARD_SHARED_CONST(CHandleRegister, this)
		{
			return IsRegisteredInternal(hObject);
		}
	}

private:

	BOOL IsRegisteredInternal(HGDIOBJ hObject) const
	{
		BOOL bDummy = FALSE;
		return m_Handles.Lookup(hObject, bDummy);
	}

	CMap<HGDIOBJ, HGDIOBJ, BOOL, BOOL> m_Handles;
};

///////////////////////////////////////////////////////////////////////////////
// CXTPSkinManagerMetrics

CXTPSkinManagerMetrics::CXTPSkinManagerMetrics(CXTPSkinManagerSchema* pSchema)
{
	m_pSchema = pSchema;

	ZeroMemory(m_brTheme, sizeof(m_brTheme));
	memset(m_clrTheme, -1, sizeof(m_clrTheme));

	m_cyCaption = m_cySmallCaption = m_cyOsCaption = m_cyOsSmallCaption = 0;

	m_bRefreshMetrics = FALSE;
}

CXTPSkinManagerMetrics::~CXTPSkinManagerMetrics()
{
	DestroyMetrics();
}

COLORREF CXTPSkinManagerMetrics::GetColor(int nIndex) const
{
	return nIndex >= 0 && nIndex < XTP_SKINMETRICS_COLORTABLESIZE ? m_clrTheme[nIndex] : 0;
}

BOOL CXTPSkinManagerMetrics::IsMetricObject(HGDIOBJ hObject) const
{
	if (hObject == 0)
		return FALSE;

	if (GetObjectType(hObject) != OBJ_BRUSH)
		return FALSE;

	if (hObject == m_xtpBrushWindow || hObject == m_xtpBrushDialog || hObject == m_xtpBrushTabControl)
		return TRUE;

	for (int i = 0; i < XTP_SKINMETRICS_COLORTABLESIZE; i++)
	{
		if (hObject == m_brTheme[i])
			return TRUE;
	}

	return FALSE;
}

BOOL AFX_CDECL CXTPSkinManagerMetrics::IsKnownMetricObject(HGDIOBJ hObject)
{
	return (NULL != hObject && OBJ_BRUSH != GetObjectType(hObject)
		? CHandleRegister::Instance().IsRegistered(hObject)
		: FALSE);
}

void CXTPSkinManagerMetrics::DeleteSysBrush(HBRUSH* pBrush)
{
	if (pBrush && *pBrush)
	{
		HBRUSH hBrush = *pBrush;
		*pBrush = NULL;
		DeleteObject(hBrush);
	}
}

void CXTPSkinManagerMetrics::DestroyMetrics()
{
	m_xtpFontCaption.DeleteObject();
	m_xtpFontSmCaption.DeleteObject();
	m_xtpFontMenu.DeleteObject();

	CHandleRegister& handleRegister = CHandleRegister::Instance();

	for (int iColorId = 0; iColorId < XTP_SKINMETRICS_COLORTABLESIZE; iColorId++)
	{
		if (NULL != m_brTheme[iColorId])
		{
			::DeleteObject(m_brTheme[iColorId]);
			m_brTheme[iColorId] = NULL;
		}
	}

	if (NULL != m_xtpBrushDialog.GetSafeHandle())
	{
		handleRegister.Unregister(m_xtpBrushDialog);
		m_xtpBrushDialog.DeleteObject();
	}

	if (NULL != m_xtpBrushTabControl.GetSafeHandle())
	{
		handleRegister.Unregister(m_xtpBrushTabControl);
		m_xtpBrushTabControl.DeleteObject();
	}

	if (NULL != m_xtpBrushWindow.GetSafeHandle())
	{
		handleRegister.Unregister(m_xtpBrushWindow);
		m_xtpBrushWindow.DeleteObject();
	}
}

BOOL CXTPSkinManagerMetrics::CreateTabControlBrush(CBitmap* pBitmap)
{
	ASSERT_VALID(pBitmap);

	BOOL bSuccess = FALSE;

	CHandleRegister& handleRegister = CHandleRegister::Instance();
	if (NULL != m_xtpBrushTabControl.GetSafeHandle())
	{
		handleRegister.Unregister(m_xtpBrushTabControl);
		m_xtpBrushTabControl.DeleteObject();
	}

	if (m_xtpBrushTabControl.CreatePatternBrush(pBitmap))
	{
		m_xtpBrushTabControl.SetHandleOwnershipNotGuaranteed();
		handleRegister.Register(m_xtpBrushTabControl);
		bSuccess = TRUE;
	}

	return bSuccess;
}

void CXTPSkinManagerMetrics::RefreshMetrics()
{
	static const COLORREF defColors[XTP_SKINMETRICS_COLORTABLESIZE] =
	{
		RGB(192, 192, 192),
		RGB( 58, 110, 165),
		RGB(  0,   0, 128),
		RGB(128, 128, 128),
		RGB(192, 192, 192),
		RGB(255, 255, 255),
		RGB(  0,   0,   0),
		RGB(  0,   0,   0),
		RGB(  0,   0,   0),
		RGB(255, 255, 255),
		RGB(192, 192, 192),
		RGB(192, 192, 192),
		RGB(128, 128, 128),
		RGB(  0,   0, 128),
		RGB(255, 255, 255),
		RGB(192, 192, 192),
		RGB(128, 128, 128),
		RGB(128, 128, 128),
		RGB(  0,   0,   0),
		RGB(192, 192, 192),
		RGB(255, 255, 255),
		RGB(  0,   0,   0),
		RGB(223, 223, 223),
		RGB(  0,   0,   0),
		RGB(255, 255, 225),
		RGB(180, 180, 180),
		RGB(  0,   0, 255),
		RGB( 16, 132, 208),
		RGB(181, 181, 181),
		RGB(  0,   0, 128),
		RGB(192, 192, 192)
	};

	CHandleRegister& handleRegister = CHandleRegister::Instance();

	XTP_GUARD_EXCLUSIVE(CXTPSkinManagerMetrics, this)
	{
		if (!m_pSchema)
		{
			for (int iColorId = 0; iColorId < XTP_SKINMETRICS_COLORTABLESIZE; iColorId++)
			{
				m_clrTheme[iColorId] = GetSysColor(iColorId);

				if (NULL != m_brTheme[iColorId])
				{
					::DeleteObject(m_brTheme[iColorId]);
					m_brTheme[iColorId] = NULL;
				}
			}
			return;
		}

		XTP_GUARD_SHARED(CXTPSkinManagerSchema, m_pSchema)
		{
			m_bRefreshMetrics = TRUE;

			UINT nSysMetrics = m_pSchema->GetClassCode(_T("SYSMETRICS"));

			NONCLIENTMETRICS ncm;
			ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
			ncm.cbSize = sizeof(NONCLIENTMETRICS);
			VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0));

			m_xtpFontCaption.DeleteObject();
			m_xtpFontCaption.CreateFontIndirect(&ncm.lfCaptionFont);

			m_xtpFontSmCaption.DeleteObject();
			m_xtpFontSmCaption.CreateFontIndirect(&ncm.lfSmCaptionFont);

			m_xtpFontMenu.DeleteObject();
			m_xtpFontMenu.CreateFontIndirect(&ncm.lfMenuFont);

			m_nBorderSize = 1;
			SystemParametersInfo(SPI_GETBORDER, 0, &m_nBorderSize, FALSE);

			m_cxBorder = GetSystemMetrics(SM_CXBORDER);
			m_cyBorder = GetSystemMetrics(SM_CYBORDER);
			m_cxEdge = GetSystemMetrics(SM_CXEDGE);
			m_cyEdge = GetSystemMetrics(SM_CYEDGE);

			m_cxHScroll = GetSystemMetrics(SM_CXHSCROLL);
			m_cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
			m_cxOsVScroll = m_cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
			m_cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

			XTP_GUARD_SHARED_(CXTPSkinManager, m_pSchema->GetSkinManager(), pSchemaSkinManager)
			{
				BOOL bApplyMetrics = pSchemaSkinManager->GetApplyOptions() & xtpSkinApplyMetrics;

				m_cyOsCaption = m_cyCaption = GetSystemMetrics(SM_CYCAPTION);
				m_cyOsSmallCaption = m_cySmallCaption = GetSystemMetrics(SM_CYSMCAPTION);

				if (bApplyMetrics)
				{
					m_pSchema->GetIntProperty(nSysMetrics, 0, 0, TMT_CAPTIONBARHEIGHT, m_cyCaption);
					m_cyCaption = XTP_DPI_Y(m_cyCaption + 1);

					m_pSchema->GetIntProperty(nSysMetrics, 0, 0, TMT_SMCAPTIONBARHEIGHT, m_cySmallCaption);
					m_cySmallCaption = XTP_DPI_Y(m_cySmallCaption + 1);

					int nScrollbarWidth = 0, nScrollbarHeight = 0;
					if (SUCCEEDED(m_pSchema->GetIntProperty(nSysMetrics, 0, 0, TMT_SCROLLBARWIDTH, nScrollbarWidth)) && nScrollbarWidth > 0)
					{
						m_cxHScroll = m_cxVScroll = XTP_DPI_X(nScrollbarWidth);
					}
					if (SUCCEEDED(m_pSchema->GetIntProperty(nSysMetrics, 0, 0, TMT_SCROLLBARHEIGHT, nScrollbarHeight)) && nScrollbarHeight > 0)
					{
						m_cyHScroll = m_cyVScroll = XTP_DPI_Y(nScrollbarHeight);
					}

					LOGFONT lfCaption, lfSmCaption, lfMenu;
					if (SUCCEEDED(m_pSchema->GetFontProperty(nSysMetrics, 0, 0, TMT_CAPTIONFONT, lfCaption)))
					{
						m_xtpFontCaption.DeleteObject();
						m_xtpFontCaption.CreateFontIndirect(&lfCaption);
					}

					if (SUCCEEDED(m_pSchema->GetFontProperty(nSysMetrics, 0, 0, TMT_SMALLCAPTIONFONT, lfSmCaption)))
					{
						m_xtpFontSmCaption.DeleteObject();
						m_xtpFontSmCaption.CreateFontIndirect(&lfSmCaption);
					}

					if (SUCCEEDED(m_pSchema->GetFontProperty(nSysMetrics, 0, 0, TMT_MENUFONT, lfMenu)))
					{
						m_xtpFontMenu.DeleteObject();
						m_xtpFontMenu.CreateFontIndirect(&lfMenu);
					}
				}

				for (int iColorId = 0; iColorId < XTP_SKINMETRICS_COLORTABLESIZE; iColorId++)
				{
					COLORREF clrOld = m_clrTheme[iColorId];
					m_clrTheme[iColorId] = (COLORREF)-1;

					COLORREF clrVal;

					if (FAILED(m_pSchema->GetColorProperty(nSysMetrics, 0, 0, iColorId + TMT_FIRSTCOLOR, clrVal)))
					{
						clrVal = defColors[iColorId];

						pSchemaSkinManager->ApplyColorFilter(clrVal);
					}

					m_clrTheme[iColorId] = clrVal;

					if ((clrOld != clrVal) || (m_brTheme[iColorId] == NULL))
					{
						if (NULL != m_brTheme[iColorId])
						{
							::DeleteObject(m_brTheme[iColorId]);
							m_brTheme[iColorId] = NULL;
						}

						m_brTheme[iColorId] = ::CreateSolidBrush(clrVal);
					}
				}
			}

			m_clrEdgeHighLight = m_clrTheme[COLOR_BTNHIGHLIGHT];
			m_clrEdgeShadow = m_clrTheme[COLOR_BTNSHADOW];
			m_clrEdgeLight = m_clrTheme[COLOR_3DLIGHT];
			m_clrEdgeDkShadow = m_clrTheme[COLOR_3DDKSHADOW];

			if (NULL != m_xtpBrushDialog.GetSafeHandle())
			{
				handleRegister.Unregister(m_xtpBrushDialog);
				m_xtpBrushDialog.DeleteObject();
			}

			if (NULL != m_xtpBrushTabControl.GetSafeHandle())
			{
				handleRegister.Unregister(m_xtpBrushTabControl);
				m_xtpBrushTabControl.DeleteObject();
			}

			if (NULL != m_xtpBrushWindow.GetSafeHandle())
			{
				handleRegister.Unregister(m_xtpBrushWindow);
				m_xtpBrushWindow.DeleteObject();
			}

			if (m_xtpBrushDialog.CreateSolidBrush(m_clrTheme[COLOR_3DFACE]))
			{
				m_xtpBrushDialog.SetHandleOwnershipNotGuaranteed();
				handleRegister.Register(m_xtpBrushDialog);
			}

			if (m_xtpBrushWindow.CreateSolidBrush(m_clrTheme[COLOR_WINDOW]))
			{
				m_xtpBrushWindow.SetHandleOwnershipNotGuaranteed();
				handleRegister.Register(m_xtpBrushWindow);
			}

			m_bRefreshMetrics = FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CXTPSkinManager

class XTP_SKINAMANGER_THREAD_STATE : public CNoTrackObject
{
public:
	XTP_SKINAMANGER_THREAD_STATE();
	virtual ~XTP_SKINAMANGER_THREAD_STATE();

public:
	HHOOK m_hHookOldCbtFilter;
};

XTP_SKINAMANGER_THREAD_STATE::XTP_SKINAMANGER_THREAD_STATE()
{
	m_hHookOldCbtFilter = 0;
}

XTP_SKINAMANGER_THREAD_STATE::~XTP_SKINAMANGER_THREAD_STATE()
{
	if (m_hHookOldCbtFilter)
	{
		UnhookWindowsHookEx(m_hHookOldCbtFilter);
	}
}



THREAD_LOCAL(XTP_SKINAMANGER_THREAD_STATE, _xtpSkinManagerThreadState)

CXTPSkinManager::CXTPSkinManager()
	: m_bAppVisualThemesDisabled(AreAppVisualThemesDisabled())
{
	::InitializeCriticalSection(&m_csObjects);

	s_pInstance = this;

	m_pModuleState = AfxGetModuleState();

	m_pSchema = NULL;
	m_bEnabled = FALSE;

	m_pResourceFile = new CXTPSkinManagerResourceFile(this);

	m_pApiHook = new CXTPSkinManagerApiHook();

	m_pClassMap = new CXTPSkinObjectClassMap();
	m_pClassMap->AddStandardClasses();

	m_mapObjects.InitHashTable(199, FALSE);

	m_bAutoApplyWindows = TRUE;
	m_bAutoApplyThreads = TRUE;
	m_dwApplyOptions = xtpSkinApplyFrame | xtpSkinApplyColors | xtpSkinApplyMetrics | xtpSkinApplyMenus;

	m_dwComCtrl = XTPSystemVersion()->GetComCtlVersion();

	m_bWin9x = XTPSystemVersion()->IsWin9x();

	m_hWinEventHook = NULL;
	m_pSetWinEventHook = NULL;
	m_pUnhookWinEvent = NULL;
	m_pWindowFilter = NULL;

	CXTPSkinManagerApiHook* pApiHook = CXTPSkinManagerApiHook::GetInstance();
	pApiHook;

	HMODULE hLib = GetModuleHandle(_T("USER32"));
	if (hLib)
	{
		m_pSetWinEventHook = (LPFNSETWINEVENTHOOK)GetProcAddress(hLib, "SetWinEventHook");
		m_pUnhookWinEvent = (LPFNUNHOOKWINEVENT)GetProcAddress(hLib, "UnhookWinEvent");

		if (m_pSetWinEventHook && m_pUnhookWinEvent)
		{
			m_hWinEventHook = m_pSetWinEventHook(EVENT_OBJECT_REORDER, EVENT_OBJECT_REORDER, NULL,
				&CXTPSkinManager::WinEventProc, GetCurrentProcessId(), 0, 0);
		}
	}
	EnableCurrentThread();

	XTPGetApplication()->Subscribe(this);
}

void CXTPSkinManager::EnableCurrentThread()
{
	DWORD dwThread = ::GetCurrentThreadId();

	XTP_SKINAMANGER_THREAD_STATE* pThreadState = _xtpSkinManagerThreadState.GetData();
	if (pThreadState)
	{
		if (pThreadState->m_hHookOldCbtFilter == 0)
		{
			pThreadState->m_hHookOldCbtFilter = ::SetWindowsHookEx(WH_CBT, CbtFilterHook, NULL, dwThread);
		}
	}
}

CXTPSkinManager::~CXTPSkinManager()
{
	XTPGetApplication()->Unsubscribe(this);

	if (m_bEnabled)
	{
		LoadSkin(NULL, NULL);
	}

	FreeSkinData();
	RemoveColorFilters();

	CMDTARGET_RELEASE(m_pSchema);
	CMDTARGET_RELEASE(m_pResourceFile);
	SAFE_DELETE(m_pClassMap);

	RemoveAll();

	m_pApiHook->FinalizeHookManagement();

	HMODULE hLib = GetModuleHandle(_T("USER32"));
	if (m_hWinEventHook && hLib && m_pUnhookWinEvent)
	{
		m_pUnhookWinEvent(m_hWinEventHook);
	}

	SAFE_DELETE(m_pApiHook);

	::DeleteCriticalSection(&m_csObjects);
}

BOOL CXTPSkinManager::IsColorFilterExists() const
{
	return m_arrFilters.GetSize() > 0;
}

void CXTPSkinManager::RemoveColorFilters()
{
	for (int i = 0; i < (int)m_arrFilters.GetSize(); i++)
	{
		delete m_arrFilters[i];
	}
	m_arrFilters.RemoveAll();
}

void CXTPSkinManager::AddColorFilter(CXTPSkinManagerColorFilter* pFilter)
{
	m_arrFilters.Add(pFilter);
}

void CXTPSkinManager::RedrawAllControls()
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		if (!m_bEnabled)
			return;

		if (!m_pSchema)
			return;

		XTPAccessExclusive(m_pSchema)->RefreshAllClasses();
		XTPColorManager()->RefreshColors();

		OnSkinChanged(TRUE, TRUE);
	}
}

void CXTPSkinManager::ApplyColorFilter(COLORREF& clr)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		for (int i = 0; i < (int)m_arrFilters.GetSize(); i++)
		{
			m_arrFilters[i]->ApplyColorFilter(clr);
		}
	}
}

CXTPSkinManagerSchema* CXTPSkinManager::CreateSchema(LPCTSTR lpszResourcePath, LPCTSTR lpszIniFileName)
{
	if (XTPColorManager()->IsLowResolution() || !CXTPSkinManagerModuleList::IsEnumeratorExists())
		return NULL;

	CXTPSkinManagerResourceFile* pResourceFile = new CXTPSkinManagerResourceFile(this);
	if (!pResourceFile->Open(lpszResourcePath, lpszIniFileName))
	{
		CMDTARGET_RELEASE(pResourceFile);
		return NULL;
	}

	CXTPSkinManagerSchema* pSchema = pResourceFile->CreateSchema();
	if (!pSchema)
	{
		CMDTARGET_RELEASE(pResourceFile);
		return NULL;

	}

	CMDTARGET_RELEASE(pResourceFile);

	if (FAILED(pSchema->ReadProperties()))
	{
		CMDTARGET_RELEASE(pSchema);
		return NULL;
	}

	pSchema->RefreshMetrcis();
	return pSchema;
}

BOOL CXTPSkinManager::LoadSkin(LPCTSTR lpszResourcePath, LPCTSTR lpszIniFileName)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		if (m_bAppVisualThemesDisabled)
			return FALSE;

		BOOL bEnabled = FALSE;
		XTP_GUARD_EXCLUSIVE_(CXTPSkinManagerApiHook, CXTPSkinManagerApiHook::GetInstance(), pApiHook)
		{
			pApiHook->FinalizeHookManagement();

			XTPSoundManager()->StopThread();

			CMDTARGET_RELEASE(m_pSchema);

			bEnabled = m_bEnabled;

			if (NULL != lpszResourcePath)
			{
				if (XTPColorManager()->IsLowResolution() ||
					!CXTPSkinManagerModuleList::IsEnumeratorExists() ||
					!ReadSkinData(lpszResourcePath, lpszIniFileName))
				{
					m_bEnabled = FALSE;
				}
				else
				{
					m_bEnabled = TRUE;

					pApiHook->InitializeHookManagement();

					m_pSchema->RefreshMetrcis();
				}
			}
			else
			{
				m_bEnabled = FALSE;
			}
		}

		XTPColorManager()->RefreshColors();

		OnSkinChanged(bEnabled, m_bEnabled);

		return m_bEnabled;
	}
}

CXTPSkinManagerMetrics* CXTPSkinManager::GetMetrics() const
{
	XTP_GUARD_SHARED_CONST(CXTPSkinManager, this)
	{
		return m_pSchema ? XTPAccessShared(m_pSchema)->GetMetrics() : NULL;
	}
}

CXTPSkinManagerClass* CXTPSkinManager::FromHandle(HTHEME hTheme)
{
	CXTPSkinManagerClass* pClass = NULL;

	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		if (NULL != m_pSchema)
		{
			pClass = XTPAccessExclusive(m_pSchema)->GetClassFromThemeHandle(hTheme);
		}
	}

	return pClass;
}

void CXTPSkinManager::ExcludeModule(LPCTSTR lpszModule)
{
	XTP_GUARD_EXCLUSIVE_(CXTPSkinManagerApiHook, CXTPSkinManagerApiHook::GetInstance(), pApiHook)
	{
		pApiHook->ExcludeModule(lpszModule, FALSE);
	}
}

void CXTPSkinManager::SetApplyOptions(DWORD dwOptions)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		m_dwApplyOptions = dwOptions;

		if (!m_bEnabled)
			return;

		XTP_GUARD_EXCLUSIVE_(CXTPSkinManagerApiHook, CXTPSkinManagerApiHook::GetInstance(), pApiHook)
		{
			pApiHook->FinalizeHookManagement();
			pApiHook->InitializeHookManagement();
		}

		XTPAccessExclusive(m_pSchema)->RefreshMetrcis();
		XTPColorManager()->RefreshColors();

		OnSkinChanged(m_bEnabled, m_bEnabled);
	}
}


void CXTPSkinManager::SetResourceFile(CXTPSkinManagerResourceFile* pResourceFile)
{
	ASSERT(pResourceFile);
	if (!pResourceFile)
		return;

	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		CMDTARGET_RELEASE(m_pResourceFile);

		m_pResourceFile = pResourceFile;
		XTPAccessExclusive(m_pResourceFile)->m_pManager = this;
	}
}

void CXTPSkinManager::FreeSkinData()
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		XTPAccessExclusive(m_pResourceFile)->Close();
	}
}

BOOL CXTPSkinManager::ReadSkinData(LPCTSTR strResourcePath, LPCTSTR strIniFileName)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		FreeSkinData();

		XTP_GUARD_EXCLUSIVE(CXTPSkinManagerResourceFile, m_pResourceFile)
		{
			if (!m_pResourceFile->Open(strResourcePath, strIniFileName))
				return FALSE;

			m_pSchema = m_pResourceFile->CreateSchema();
			if (!m_pSchema)
				return FALSE;

			if (FAILED(XTPAccessExclusive(m_pSchema)->ReadProperties()))
				return FALSE;

			return TRUE;
		}
	}
}

LRESULT CALLBACK CXTPSkinManager::CbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
	XTP_SKINAMANGER_THREAD_STATE* pThreadState = _xtpSkinManagerThreadState.GetData();

	LRESULT lResult = CallNextHookEx(pThreadState->m_hHookOldCbtFilter, code,
			wParam, lParam);

	if (code == HCBT_CREATEWND && NULL != CXTPSkinManager::s_pInstance)
	{
		XTP_GUARD_SHARED_(CXTPSkinManager, XTPSkinManager(), pSkinManager)
		{
			if (pSkinManager->m_bAutoApplyWindows)
			{
				ASSERT(lParam != NULL);
				LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;
				ASSERT(lpcs != NULL);

				ASSERT(wParam != NULL); // should be non-NULL HWND
				HWND hWnd = (HWND)wParam;

				TCHAR szClassName[XTP_MAX_CLASSNAME + 1];
				if (0 != ::GetClassName(hWnd, szClassName, _countof(szClassName)))
				{
					if (pSkinManager->Lookup(hWnd) != NULL)
					{
						TRACE(_T("Warning: Window already attached\n"));
					}
					else
					{
						pSkinManager->SetHook(hWnd, szClassName, lpcs, TRUE);
					}
				}
			}
		}
	}

	return lResult;
}

_XTP_EXT_CLASS CXTPRWCriticalSection* AFX_CDECL XTPSkinGlobalAccess()
{
	static CXTPRWCriticalSection cs;
	return &cs;
}

CXTPSkinManager* CXTPSkinManager::s_pInstance = NULL;

_XTP_EXT_CLASS CXTPSkinManager* AFX_CDECL XTPSkinManager()
{
	XTP_SKINFRAMEWORK_GLOBALLOCK_SHARED_SCOPE();

	if (CXTPSkinManager::s_pInstance == NULL)
	{
		CXTPSkinManager::s_pInstance = &XTPSkinManagerSingleton::Instance();
	}

	return CXTPSkinManager::s_pInstance;
}

CXTPSkinManager* CXTPSkinManager::SetSkinManager(CXTPSkinManager* pSkinManager, BOOL bDelete /*=TRUE*/)
{
	ASSERT(pSkinManager != NULL);

	CXTPSkinManager* pPrevInstance = NULL;
	if (pSkinManager != NULL)
	{
		XTP_SKINFRAMEWORK_GLOBALLOCK_EXCLUSIVE_SCOPE();

		if (bDelete && &XTPSkinManagerSingleton::Instance() != s_pInstance)
		{
			SAFE_DELETE(s_pInstance);
		}
		else
		{
			pPrevInstance = s_pInstance;
		}

		s_pInstance = pSkinManager;
	}
	
	return pPrevInstance;
}

CXTPSkinManagerClass* CXTPSkinManager::GetSkinClass(const CXTPSkinObject* pObject, CString strClassList)
{
	CXTPSkinManagerClass* pClass = NULL;

	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		XTP_GUARD_SHARED_(CXTPSkinManagerSchema, (pObject && pObject->m_pSchema ? pObject->m_pSchema : m_pSchema), pSchema)
		{
			strClassList.MakeUpper();
			pClass = pSchema->GetClass(strClassList);
		}
	}

	return pClass;
}

void CXTPSkinManager::GetCurrentThemeName(CString& strThemeFileName, CString& strColorBuff)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		if (m_pResourceFile)
		{
			XTP_GUARD_SHARED(CXTPSkinManagerResourceFile, m_pResourceFile)
			{
				strThemeFileName = m_pResourceFile->GetResourcePath();
				strColorBuff = m_pResourceFile->GetIniFileName();
			}
		}
	}
}


int CXTPSkinManager::GetThemeSysSize(int iSizeId)
{
	XTP_GUARD_SHARED(CXTPSkinManagerSchema, m_pSchema)
	{
		int iVal = 0;
		if (!SUCCEEDED(m_pSchema->GetIntProperty(m_pSchema->GetClassCode(_T("SYSMETRICS")), 0, 0, iSizeId, iVal)))
			return 0;
		return iVal;
	}
}

BOOL CXTPSkinManager::GetThemeSysBool(int iBoolId)
{
	XTP_GUARD_SHARED(CXTPSkinManagerSchema, m_pSchema)
	{
		BOOL bVal = FALSE;
		if (!SUCCEEDED(m_pSchema->GetBoolProperty(m_pSchema->GetClassCode(_T("SYSMETRICS")), 0, 0, iBoolId, bVal)))
			return FALSE;
		return bVal;
	}
}


COLORREF CXTPSkinManager::GetThemeSysColor(int iColorId)
{
	if (!m_pSchema)
		return GetSysColor(iColorId - TMT_FIRSTCOLOR);

	return iColorId >= TMT_FIRSTCOLOR && iColorId <= TMT_LASTCOLOR ?
			XTPAccessShared(m_pSchema->GetMetrics())->m_clrTheme[iColorId - TMT_FIRSTCOLOR] : (COLORREF)-1;
}

HRESULT CXTPSkinManager::GetThemeSysFont(int iFontId, LOGFONT *plf)
{
	XTP_GUARD_SHARED(CXTPSkinManagerSchema, m_pSchema)
	{
		HRESULT hr = m_pSchema->GetFontProperty(m_pSchema->GetClassCode(_T("SYSMETRICS")), 0, 0, iFontId, *plf);
		return hr;
	}

}

HRESULT CXTPSkinManager::EnableThemeDialogTexture(HWND hWnd, DWORD dwFlags)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		CXTPSkinObjectFrame* pObject = (CXTPSkinObjectFrame*)Lookup(hWnd);

		if (!pObject)
			return E_INVALIDARG;

		XTP_GUARD_EXCLUSIVE(CXTPSkinObjectFrame, pObject)
		{
			pObject->m_dwDialogTexture = dwFlags;
#ifdef _XTP_ACTIVEX
			pObject->m_bActiveX = TRUE;
#endif

			if (pObject->m_dwDialogTexture == ETDT_ENABLETAB)
			{
				pObject->SetWindowProc();
			}
		}
	}

	return S_OK;
}

void CXTPSkinManager::SetWindowTheme(HWND hWnd, CXTPSkinManagerSchema* pSchema)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		CXTPSkinObject* pSkinObject = Lookup(hWnd);
		if (pSkinObject)
		{
			XTP_GUARD_EXCLUSIVE(CXTPSkinObject, pSkinObject)
			{
				CMDTARGET_RELEASE(pSkinObject->m_pSchema);
				pSkinObject->m_pSchema = pSchema;
				CMDTARGET_ADDREF(pSchema);

				pSkinObject->OnSkinChanged(m_bEnabled, pSchema ? TRUE : m_bEnabled);
			}
		}
	}

	hWnd = ::GetWindow(hWnd, GW_CHILD);
	while (hWnd)
	{
		SetWindowTheme(hWnd, pSchema);

		hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
	}
}

//////////////////////////////////////////////////////////////////////
// CXTPSkinManager

void CXTPSkinManager::ApplyWindow(HWND hWnd)
{
	ApplyWindow(hWnd, TRUE);
}

void CXTPSkinManager::ApplyWindow(HWND hWnd, BOOL bApplyChilds)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		if (m_bAppVisualThemesDisabled)
			return;

		if (Lookup(hWnd))
			return;

		TCHAR szClassName[XTP_MAX_CLASSNAME + 1];
		if (0 == ::GetClassName(hWnd, szClassName, _countof(szClassName)))
			return;

		CREATESTRUCT cs;
		ZeroMemory(&cs, sizeof(cs));

		cs.dwExStyle = (DWORD)GetWindowLongPtr(hWnd, GWL_EXSTYLE);
		cs.style = (LONG)GetWindowLongPtr(hWnd, GWL_STYLE);
		cs.hwndParent = ::GetParent(hWnd);
		cs.lpszClass = szClassName;

		CXTPSkinObject* pSkinObject = SetHook(hWnd, szClassName, &cs, FALSE);
		if (pSkinObject)
		{
			XTP_GUARD_SHARED(CXTPSkinObject, pSkinObject)
			{
				pSkinObject->OnSkinChanged(FALSE, m_bEnabled);
				if (0 == _tcsicmp(pSkinObject->m_strClassName, _T("COMBOBOX")))
				{
					HWND hWndList = CXTPSkinObjectComboBox::GetComboListBox(hWnd);
					if (hWndList)
					{
						ApplyWindow(hWndList);
					}
				}
			}
		}

		if (bApplyChilds)
		{
			hWnd = ::GetWindow(hWnd, GW_CHILD);
			while (hWnd)
			{
				ApplyWindow(hWnd);

				hWnd = ::GetWindow(hWnd, GW_HWNDNEXT);
			}
		}
	}
}

BOOL CALLBACK CXTPSkinManager::EnumWindowsProcNetBroadcast(HWND hWnd, LPARAM /*lParam*/)
{
	DWORD dwProcessId = 0;
	if (GetWindowThreadProcessId(hWnd, &dwProcessId) && dwProcessId == GetCurrentProcessId())
	{
		::PostMessage(hWnd, WM_SYSCOLORCHANGE, 0, 0);
	}

	return TRUE;
}

BOOL AFX_CDECL CXTPSkinManager::AreAppVisualThemesDisabled()
{
	BOOL bDisabled = FALSE;
	HKEY hkeyLayers = NULL;
	if(0 == RegOpenKey(
		HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers"),
		&hkeyLayers))
	{
		TCHAR szAppPath[MAX_PATH + 1];
		if(0 < GetModuleFileName(NULL, szAppPath, MAX_PATH))
		{
			DWORD dwType = 0;
			TCHAR szData[50] = { _T('\0') };
			DWORD cbData = sizeof(szData);
			if(0 == RegQueryValueEx(hkeyLayers, szAppPath, NULL, &dwType, 
				reinterpret_cast<PBYTE>(szData),
				&cbData))
			{
				static const TCHAR szDISABLETHEMES[] = _T("DISABLETHEMES");
				if(sizeof(szDISABLETHEMES) == cbData)
				{
					bDisabled = (0 == _tcscmp(szData, szDISABLETHEMES));
				}
			}
		}

		RegCloseKey(hkeyLayers);
	}

	return bDisabled;
}

void CXTPSkinManager::OnSkinChanged(BOOL bPrevState, BOOL bNewState)
{
	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		CWnd* pWnd = AfxGetMainWnd();
		if (pWnd) pWnd->SendMessage(WM_SYSCOLORCHANGE);

		DWORD dwThreadId = GetCurrentThreadId();
		HWND hWnd;
		POSITION pos = m_mapObjects.GetStartPosition();
		CXTPSkinObject* pSink;

		EnumWindows(EnumWindowsProcNetBroadcast, 0);

		while (pos)
		{
			m_mapObjects.GetNextAssoc(pos, hWnd, pSink);

			if (::IsWindow(hWnd))
			{
				XTP_GUARD_SHARED(CXTPSkinObject, pSink)
				{
					if (GetWindowThreadProcessId(hWnd, NULL) == dwThreadId)
					{
						pSink->OnSkinChanged(bPrevState, bNewState);
					}
					else
					{
						pSink->PostMessage(CXTPSkinObject::m_nMsgSkinChanged, bPrevState, bNewState);
					}
				}
			}
			else
			{
				Remove(hWnd);
			}
		}
	}
}

static BOOL IsModuleNameChecked(
	LPCSTR lpName,
	CArray<LPCSTR, LPCSTR>& arCheckedNames)
{
	ASSERT(NULL != lpName);

	BOOL bChecked = FALSE;

	for (INT_PTR i = 0; i < arCheckedNames.GetSize(); ++i)
	{
		if (_stricmp(lpName, arCheckedNames[i]))
		{
			bChecked = TRUE;
			break;
		}
	}

	return bChecked;
}

static BOOL IsModuleImported(
	HMODULE hSourceModule,
	HMODULE hModule,
	CArray<LPCSTR, LPCSTR>& arCheckedNames)
{
	BOOL bImported = FALSE;

	PBYTE pModuleStart = reinterpret_cast<PBYTE>(hSourceModule);

	PIMAGE_DOS_HEADER pDosHdr = reinterpret_cast<PIMAGE_DOS_HEADER>(pModuleStart);
	ASSERT(::AfxIsValidAddress(pDosHdr, sizeof(*pDosHdr), FALSE));

	if (pDosHdr->e_magic == IMAGE_DOS_SIGNATURE)
	{
		PIMAGE_NT_HEADERS pNtHdr = reinterpret_cast<PIMAGE_NT_HEADERS>(pModuleStart + pDosHdr->e_lfanew);
		ASSERT(::AfxIsValidAddress(pNtHdr, sizeof(*pNtHdr), FALSE));

		if (pNtHdr->Signature == IMAGE_NT_SIGNATURE)
		{
			PIMAGE_DATA_DIRECTORY pImportDataDir = &pNtHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
			if (0 != pImportDataDir->VirtualAddress)
			{
				PIMAGE_IMPORT_DESCRIPTOR pImportDir = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(
					pModuleStart + pImportDataDir->VirtualAddress);
				while (!bImported && 0 != pImportDir->OriginalFirstThunk)
				{
					if (0 != pImportDir->Name)
					{
						LPCSTR lpName = reinterpret_cast<LPCSTR>(pModuleStart + pImportDir->Name);
						if (!IsModuleNameChecked(lpName, arCheckedNames))
						{
							arCheckedNames.Add(lpName);

							HMODULE hImportedModule = ::GetModuleHandleA(lpName);
							if (hImportedModule == hModule)
							{
								bImported = TRUE;
							}
							else
							{
								bImported = IsModuleImported(hImportedModule, hModule, arCheckedNames);
							}
						}
					}

					++pImportDir;
				}
			}
		}
	}

	return bImported;
}

BOOL CXTPSkinManager::IsPersistentModule(HMODULE hModule)
{
	BOOL bPersistent = FALSE;

	AFX_MODULE_STATE* lpModuleState = ::AfxGetModuleState();
	ASSERT(NULL != lpModuleState);

	HMODULE hExeModule = ::GetModuleHandle(NULL);

	if (hModule == lpModuleState->m_hCurrentInstanceHandle || hModule == hExeModule)
	{
		// The current or executable module 
		bPersistent = TRUE;
	}
	else
	{
		// Check if the handle has already been checked. If once determined as non-persistent
		// the same handle value can never identify a persistent module.
		BOOL bChecked = FALSE;
		CXTPRWCriticalSection::CSharedLock sharedLock(m_csPersistentModuleCache);
		for (INT_PTR i = 0; i < m_arPersistentModuleCache.GetSize(); ++i)
		{
			const PERSISTENT_IMPORT_INFO& info = m_arPersistentModuleCache[i];
			if (hModule == info.hModule)
			{
				bPersistent = info.bPersistent;
				bChecked = TRUE;
				break;
			}
		}

		if (!bChecked)
		{
			// If not checked before, the module has to be check in IAT of both the current 
			// and executable modules and their dependency trees.
			CArray<LPCSTR, LPCSTR> arCheckedNames;
			bPersistent = IsModuleImported(lpModuleState->m_hCurrentInstanceHandle, hModule, arCheckedNames);
			if (!bPersistent && lpModuleState->m_hCurrentInstanceHandle != hExeModule)
			{
				arCheckedNames.RemoveAll();
				bPersistent = IsModuleImported(hExeModule, hModule, arCheckedNames);
			}

			CXTPRWCriticalSection::CSharedLock exclusiveLock(m_csPersistentModuleCache);
			PERSISTENT_IMPORT_INFO info = { hModule, bPersistent };
			m_arPersistentModuleCache.Add(info);
		}
	}

	return bPersistent;
}

CXTPSkinObject* CXTPSkinManager::Lookup(HWND hWnd)
{
	CXTPSkinObject* pSink;

	if (XTPAccessShared(this)->m_mapObjects.Lookup(hWnd, pSink))
		return pSink;

	return NULL;
}

#ifndef OBJID_WINDOW
#define OBJID_WINDOW 0x00000000
#endif

#ifndef EVENT_OBJECT_REORDER
#define EVENT_OBJECT_REORDER 0x8004
#endif

void CALLBACK CXTPSkinManager::WinEventProc(HWINEVENTHOOK /*hWinEventHook*/,
	DWORD event, HWND hWnd, LONG idObject, LONG /*idChild*/, DWORD /*dwEventThread*/, DWORD /*dwmsEventTime*/)
{
	if (event == EVENT_OBJECT_REORDER && idObject == OBJID_WINDOW)
	{
		XTP_SKINFRAMEWORK_MANAGE_STATE();

		XTP_GUARD_SHARED(CXTPSkinManager, XTPSkinManager())
		{
			if ((XTPSkinManager()->GetApplyOptions() & xtpSkinApplyFrame)
				&& XTPSkinManager()->IsEnabled())
			{
				CXTPSkinObjectFrame* pFrame = (CXTPSkinObjectFrame*)XTPSkinManager()->Lookup(hWnd);

				if (pFrame && ::IsWindow(hWnd))
				{
					XTP_GUARD_SHARED(CXTPSkinObjectFrame, pFrame)
					{
						DWORD dwStyle = pFrame->GetStyle();
						DWORD dwStyleRemove = XTP_REFRESHFRAME_AFFECTED_STYLES;
						if (dwStyle & dwStyleRemove)
						{
							pFrame->SetTimer(XTP_TID_REFRESHFRAME, XTP_REFRESHFRAME_DURATION,
								&CXTPSkinObjectFrame::OnTimerInternal);
						}
					}
				}
			}
		}
	}
}

#define PSH_WIZARD97IE4            0x00002000
#define PSH_WIZARD97IE5            0x01000000

#ifndef DWLP_USER
#define DWLP_USER 8
#endif

AFX_INLINE BOOL IsTabPage(LPCREATESTRUCT lpcs)
{
	if (!lpcs || !lpcs->hwndParent)
		return FALSE;

	if (!((lpcs->style & DS_CONTROL) && (lpcs->style & WS_CHILD)))
		return FALSE;

	CXTPSkinObjectFrame* pParent = (CXTPSkinObjectFrame*)XTPAccessShared(XTPSkinManager())->Lookup(lpcs->hwndParent);
	if (pParent && pParent->m_dwDialogTexture == ETDT_ENABLETAB)
		return TRUE;

	HWND hwndTabControl = (HWND)::SendMessage(lpcs->hwndParent, PSM_GETTABCONTROL, 0, 0);
	if (!hwndTabControl || !::IsWindow(hwndTabControl))
		return FALSE;

	HWND* ppd = (HWND*)GetWindowLongPtr(lpcs->hwndParent, DWLP_USER);
	if (!ppd || *ppd != lpcs->hwndParent)
		return FALSE;

	PROPSHEETHEADER* psh = (PROPSHEETHEADER*)(ppd + 1);
	if (!psh)
		return FALSE;

	if ((psh->dwFlags & (PSH_WIZARD | PSH_WIZARD97IE4 | PSH_WIZARD97IE5)) != 0)
		return FALSE;

	return TRUE;
}

BOOL CXTPSkinManager::IsWindowFilteredOut(HWND hWnd, LPCTSTR lpszClassName, LPCREATESTRUCT lpcs)
{
	// Filter out all windows that do not belong to the running process.
	DWORD dwProcessId = 0;
	if (GetWindowThreadProcessId(hWnd, &dwProcessId) && dwProcessId != GetCurrentProcessId())
		return TRUE;

	// Fitler out all windows that have neither styles nor sizes. It is assumed
	// all such windows are used as background windows for some internal communications.
	if(0 == lpcs->dwExStyle
		&& 0 == lpcs->style
		&& 0 == lpcs->x
		&& 0 == lpcs->y
		&& 0 == lpcs->cx
		&& 0 == lpcs->cy)
		return TRUE;

	// Filter out some predefined windows.
	if (_tcsicmp(lpszClassName, _T("tooltips_class32")) == 0)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("#43")) == 0) // MCI command handling window (winmm.dll)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("IME")) == 0)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("CicMarshalWndClass")) == 0)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("MSCTFIME UI")) == 0)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("WorkerW")) == 0)
		return TRUE;

	if (_tcsicmp(lpszClassName, _T("SHELLDLL_DefView")) == 0)
		return TRUE;

	return (NULL != m_pWindowFilter
		? m_pWindowFilter->FilterWindow(hWnd, lpszClassName, lpcs)
		: FALSE);
}

CXTPSkinObject* CXTPSkinManager::SetHook(HWND hWnd, LPCTSTR lpszClassName, LPCREATESTRUCT lpcs, BOOL bAuto)
{
	if (IsWindowFilteredOut(hWnd, lpszClassName, lpcs))
		return NULL;

	if (IsClassKnownIgnoredClass(lpszClassName))
		return NULL;

	XTP_GUARD_SHARED(CXTPSkinManager, this)
	{
		CXTPSkinObject* pSink = NULL;

		if (_tcsstr(lpszClassName, _T("WindowsForms")) != NULL)
		{
#ifdef _XTP_ACTIVEX
			if (bAuto)
			{
				return NULL;
			}
#endif

			CString strClass(lpszClassName);
			int nClassNameStart = strClass.Find(_T('.'));
			int nClassNameEnd = strClass.Find(_T('.'), nClassNameStart + 1);

			if (nClassNameStart != -1 && nClassNameEnd != -1 && nClassNameEnd != nClassNameStart)
			{
				strClass = strClass.Mid(nClassNameStart + 1, nClassNameEnd - nClassNameStart - 1);

				XTP_GUARD_SHARED(CXTPSkinObjectClassMap, m_pClassMap)
				{
					CXTPSkinObjectClassInfo* pInfo = m_pClassMap->Lookup(strClass);
					if (pInfo)
					{
						pSink = pInfo->CreateObject(lpszClassName, lpcs);
					}
				}
			}

			if (pSink == NULL)
			{
				pSink = new CXTPSkinObjectApplicationFrame();
			}

			pSink->m_bWindowsForms = TRUE;
		}

		if (NULL == pSink && NULL != m_pClassMap)
		{
			XTP_GUARD_SHARED(CXTPSkinObjectClassMap, m_pClassMap)
			{
				CXTPSkinObjectClassInfo* pInfo = m_pClassMap->Lookup(lpszClassName);

				if (pInfo)
				{
					pSink = pInfo->CreateObject(lpszClassName, lpcs);
				}
				else
				{
					pSink = new CXTPSkinObjectApplicationFrame();
				}
			}
		}

		if (pSink)
		{
			XTP_GUARD_EXCLUSIVE(CXTPSkinObject, pSink)
			{
				if (_tcscmp(lpszClassName, _T("#32770")) == 0)
				{
					((CXTPSkinObjectFrame*)pSink)->m_dwDialogTexture = IsTabPage(lpcs) ? ETDT_ENABLETAB : ETDT_ENABLE;
				}

				pSink->m_pManager = this;
				m_mapObjects[hWnd] = pSink;
				pSink->AttachHook(hWnd, lpcs, bAuto);
			}
		}

		return pSink;
	}
}

BOOL AFX_CDECL CXTPSkinManager::IsClassKnownIgnoredClass(LPCTSTR lpszClassName)
{
	ASSERT(NULL != lpszClassName);

	BOOL bKnown = FALSE;

	// Check if the window has to be ignored.
	static const LPCTSTR lpszClassNamesToIgnore[] =
	{
		_T("Internet Explorer"),
		_T("OleMainThread")
	};

	for(int i = 0; i < _countof(lpszClassNamesToIgnore); ++i)
	{
		if (NULL != _tcsstr(lpszClassName, lpszClassNamesToIgnore[i]))
		{
			bKnown = TRUE;
			break;
		}
	}

	return bKnown;
}

void CXTPSkinManager::Remove(HWND hWnd)
{
	Remove(hWnd, FALSE);
}

void CXTPSkinManager::Remove(HWND hWnd, BOOL bAuto)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		CXTPSkinObject* pSink = Lookup(hWnd);
		if (pSink)
		{
			XTP_GUARD_EXCLUSIVE(CXTPSkinObject, pSink)
			{
				pSink->UnattachHook(bAuto);

#ifdef _AFXDLL
				pSink->m_pModuleState = AfxGetModuleState();
#endif
			}

			m_mapObjects.RemoveKey(hWnd);
			pSink->InternalRelease();
		}
	}
}

void CXTPSkinManager::RemoveAll(BOOL bUnattach)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManager, this)
	{
		HWND hWnd;
		POSITION pos = m_mapObjects.GetStartPosition();
		CXTPSkinObject* pSink;

		while (pos)
		{
			m_mapObjects.GetNextAssoc(pos, hWnd, pSink);

			XTP_GUARD_EXCLUSIVE(CXTPSkinObject, pSink)
			{
				if (bUnattach) pSink->UnattachHook(FALSE);

#ifdef _AFXDLL
				pSink->m_pModuleState = AfxGetModuleState();
#endif
			}

			pSink->InternalRelease();
		}

		m_mapObjects.RemoveAll();
	}
}

LRESULT CALLBACK CXTPSkinManager::DoCallWindowProc(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return CXTPSkinManagerApiHook::CallWindowProcOrig(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK CXTPSkinManager::HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	XTP_SKINFRAMEWORK_MANAGE_STATE();

	MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
	MSG  oldMsg = curMsg;
	BOOL bCallWindowProc = TRUE;
	LRESULT lResult = 0;
	WNDPROC wndProc = NULL;
	CXTPSkinObject* pSink = NULL;
	BOOL bSkinManagerEnabled = FALSE;

	XTP_GUARD_SHARED_(CXTPSkinManager, XTPSkinManager(), pSkinManager)
	{
		// Get hook object for this window. Get from hook map
		pSink = pSkinManager->Lookup(hWnd);
		if (NULL == pSink)
			return 0;

		wndProc = XTPAccessShared(pSink)->m_pOldWndProc;
		bSkinManagerEnabled = pSkinManager->m_bEnabled;
	}

	if (!bSkinManagerEnabled)
	{
		if (message == WM_NCDESTROY)
		{
			XTPSkinManager()->Remove(hWnd, TRUE);
		}

		return ::CallWindowProc(wndProc, hWnd, message, wParam, lParam);
	}

	curMsg.hwnd = hWnd;
	curMsg.message = message;
	curMsg.wParam = wParam;
	curMsg.lParam = lParam;

	// pass to message hook
	if (WM_NCDESTROY != message)
	{
		BOOL bEndHook = FALSE;
		XTP_GUARD_SHARED(CXTPSkinObject, pSink)
		{
			if (!pSink->m_bCustomDraw || message == CXTPSkinObject::m_nMsgUpdateSkinState)
			{
				pSink->OnBeginHook(message, xtpSkinDefaultHookMessage, 0, 0);

				if (pSink->OnHookMessage(message, wParam, lParam, lResult))
				{
					bCallWindowProc = FALSE;
				}

				bEndHook = TRUE;
			}
		}

		if (bEndHook)
		{
			pSink->OnEndHook();
		}
	}

	// Window is being destroyed: unhook all hooks (for this window)
	// and pass message to orginal window proc
	if (message == WM_NCDESTROY)
	{
		XTPSkinManager()->Remove(hWnd, TRUE);
	}

	if (bCallWindowProc)
	{
		lResult = DoCallWindowProc(wndProc, hWnd, message, wParam, lParam);
	}

	curMsg = oldMsg;
	return lResult;
}

void CXTPSkinManager::OnBeforeApplicationShutdown(CXTPApplication* pApplication)
{
	UNREFERENCED_PARAMETER(pApplication);

	if (m_bEnabled)
	{
		LoadSkin(NULL, NULL);
	}
}

//////////////////////////////////////////////////////////////////////////

CXTPSkinManagerClass::CXTPSkinManagerClass(
	CXTPSkinManagerSchema* pSchema,
	CString pszClassList,
	int nClassCode)
	: m_strClass(pszClassList)
	, m_nClassCode(nClassCode)
	, m_pSchema(pSchema)
	, m_pImages(new CXTPSkinImages())
	, m_hSystemTheme(NULL)
{
	XTP_GUARD_SHARED_(CXTPSkinManagerApiHook, CXTPSkinManagerApiHook::GetInstance(), pApiHook)
	{
		CXTPSkinManagerApiFunction* pOpenThemeDataFunction = pApiHook->GetHookedFunction(xtpSkinApiOpenThemeData);
		if (NULL != pOpenThemeDataFunction)
		{
			PROC pfnOriginal = pOpenThemeDataFunction->GetOriginal();
			if (NULL != pfnOriginal)
			{
				USES_CONVERSION;
				typedef HTHEME(STDAPICALLTYPE* PFNOpenThemeData)(HWND, LPCWSTR);
				m_hSystemTheme = reinterpret_cast<PFNOpenThemeData>(pfnOriginal)(NULL, T2CW(m_strClass));
			}
		}
	}

	m_mapCachedProperties.InitHashTable(199);
}

CXTPSkinManagerClass::~CXTPSkinManagerClass()
{
	if (NULL != m_hSystemTheme)
	{
		XTP_GUARD_SHARED_(CXTPSkinManagerApiHook, CXTPSkinManagerApiHook::GetInstance(), pApiHook)
		{
			CXTPSkinManagerApiFunction* pOpenThemeDataFunction = pApiHook->GetHookedFunction(xtpSkinApiCloseThemeData);
			if (NULL != pOpenThemeDataFunction)
			{
				PROC pfnOriginal = pOpenThemeDataFunction->GetOriginal();
				if (NULL != pfnOriginal)
				{
					typedef HRESULT(STDAPICALLTYPE* PFNCloseThemeData)(HTHEME);
					reinterpret_cast<PFNCloseThemeData>(pfnOriginal)(m_hSystemTheme);
				}
			}
		}
	}

	delete m_pImages;
}

BOOL CXTPSkinManagerClass::DrawThemeText(CDC* pDC, int iPartId, int iStateId, const CString& strText, DWORD dwFormat, const RECT *pRect)
{
	pDC->SetBkMode(TRANSPARENT);

	CRect rcCaptionMargins = XTP_DPI(GetThemeRect(iPartId, iStateId, TMT_CAPTIONMARGINS));
	rcCaptionMargins.top = rcCaptionMargins.bottom = 0;

	CRect rc(pRect);
	rc.DeflateRect(rcCaptionMargins);

	if (GetThemeEnumValue(iPartId, iStateId, TMT_CONTENTALIGNMENT) == CA_CENTER)
	{
		dwFormat |=  DT_CENTER;
	}

	pDC->SetTextColor(GetThemeColor(iPartId, iStateId, TMT_TEXTCOLOR, pDC->GetTextColor()));

	if (GetThemeEnumValue(iPartId, iStateId, TMT_TEXTSHADOWTYPE) == TST_SINGLE)
	{
		CSize sz = XTP_DPI(GetThemeSize(iPartId, iStateId, TMT_TEXTSHADOWOFFSET));
		if (sz != CSize(0, 0))
		{
			COLORREF clrShadow = GetThemeColor(iPartId, iStateId, TMT_TEXTSHADOWCOLOR);
			CRect rcShadow(rc);
			rcShadow.OffsetRect(sz);

			COLORREF clr = pDC->SetTextColor(clrShadow);
			XTPSkinFrameworkDrawText(*pDC, strText, rcShadow, dwFormat);
			pDC->SetTextColor(clr);
		}
	}

	XTPSkinFrameworkDrawText(*pDC, strText, rc, dwFormat);

	return TRUE;
}

CXTPSkinManagerSchemaProperty* CXTPSkinManagerClass::GetProperty(XTPSkinManagerProperty propType, int iPartId, int iStateId, int iPropId)
{
	XTP_GUARD_EXCLUSIVE(CXTPSkinManagerClass, this)
	{
		UINT nCachedProp = iPropId + ((iPartId + (iStateId << 6)) << 14);
		CXTPSkinManagerSchemaProperty* pProperty = NULL;

		if (m_mapCachedProperties.Lookup(nCachedProp, pProperty))
			return pProperty;

		pProperty = XTPAccessShared(m_pSchema)->GetProperty(m_nClassCode, iPartId, iStateId, iPropId);
		if (!pProperty)
		{
			m_mapCachedProperties.SetAt(nCachedProp, NULL);
			return 0;
		}

		XTP_GUARD_SHARED(CXTPSkinManagerSchemaProperty, pProperty)
		{
			if (pProperty->propType != propType)
				return 0;

			m_mapCachedProperties.SetAt(nCachedProp, pProperty);

			return pProperty;
		}
	}
}

CString CXTPSkinManagerClass::GetThemeString(int iPartId, int iStateId, int iPropId, LPCTSTR lpszDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_STRING, iPartId, iStateId, iPropId);

	if (!pProperty)
		return lpszDefault;

	return XTPAccessShared(pProperty)->lpszVal;
}

CRect CXTPSkinManagerClass::GetThemeRect(int iPartId, int iStateId, int iPropId, CRect rcDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_RECT, iPartId, iStateId, iPropId);

	if (!pProperty)
		return rcDefault;

	return XTPAccessShared(pProperty)->rcVal;
}

int CXTPSkinManagerClass::GetThemeInt(int iPartId, int iStateId, int iPropId, int nDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_INT, iPartId, iStateId, iPropId);

	if (!pProperty)
		return nDefault;

	return XTPAccessShared(pProperty)->iVal;
}
BOOL CXTPSkinManagerClass::GetThemeBool(int iPartId, int iStateId, int iPropId, BOOL bDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_BOOL, iPartId, iStateId, iPropId);

	if (!pProperty)
		return bDefault;

	return XTPAccessShared(pProperty)->bVal;
}
COLORREF CXTPSkinManagerClass::GetThemeColor(int iPartId, int iStateId, int iPropId, COLORREF clrDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_COLOR, iPartId, iStateId, iPropId);

	if (!pProperty)
		return clrDefault;

	COLORREF clrVal = XTPAccessShared(pProperty)->clrVal;
	m_pSchema->GetSkinManager()->ApplyColorFilter(clrVal);

	return clrVal;
}

int CXTPSkinManagerClass::GetThemeEnumValue(int iPartId, int iStateId, int iPropId, int nDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_ENUM, iPartId, iStateId, iPropId);

	if (!pProperty)
		return nDefault;

	return XTPAccessShared(pProperty)->iVal;
}

CSize CXTPSkinManagerClass::GetThemeSize(int iPartId, int iStateId, int iPropId, CSize szDefault)
{
	CXTPSkinManagerSchemaProperty* pProperty = GetProperty(XTP_SKINPROPERTY_POSITION, iPartId, iStateId, iPropId);

	if (!pProperty)
		return szDefault;

	return XTPAccessShared(pProperty)->szVal;
}


BOOL CXTPSkinManagerClass::DrawThemeBackground(CDC* pDC, int iPartId, int iStateId, const RECT *pRect)
{
	return m_pSchema->DrawThemeBackground(pDC, this, iPartId, iStateId, pRect);
}

BOOL CXTPSkinManagerClass::GetThemePartSize(int iPartId, int iStateId, RECT *pRect, int eSize, SIZE* pSize)
{
	if (GetThemeEnumValue(iPartId, iStateId, TMT_BGTYPE) != BT_IMAGEFILE)
		return FALSE;

	int nImageFile = (GetThemeEnumValue(iPartId, iStateId, TMT_IMAGESELECTTYPE) != IST_NONE) &&
		(GetThemeEnumValue(iPartId, iStateId, TMT_GLYPHTYPE, GT_NONE) == GT_NONE) ?
		TMT_IMAGEFILE1: TMT_IMAGEFILE;

	CString strImageFile = GetThemeString(iPartId, iStateId, nImageFile);
	if (strImageFile.IsEmpty())
	{
		if (nImageFile != TMT_IMAGEFILE1)
			return FALSE;

		strImageFile = GetThemeString(iPartId, iStateId, TMT_IMAGEFILE);
		if (strImageFile.IsEmpty())
			return FALSE;

	}


	CXTPSkinImage* pImage = XTPAccessExclusive(GetImages())->LoadFile(
		XTPAccessShared(m_pSchema)->GetResourceFile(), strImageFile);
	if (!pImage)
	{
		return FALSE;
	}

	XTP_GUARD_SHARED(CXTPSkinImage, pImage)
	{
		int nImageCount = GetThemeInt(iPartId, iStateId, TMT_IMAGECOUNT, 1);
		if (nImageCount < 1)
			nImageCount = 1;

		BOOL bHorizontalImageLayout = GetThemeEnumValue(iPartId, iStateId, TMT_IMAGELAYOUT, IL_HORIZONTAL) == IL_HORIZONTAL;

		CSize sz(pImage->GetWidth(), pImage->GetHeight());
		if (bHorizontalImageLayout) sz.cx /= nImageCount; else sz.cy /= nImageCount;

		if (eSize == TS_TRUE)
		{
			*pSize = sz;
		}
		if (eSize == TS_DRAW)
		{
			if (GetThemeEnumValue(iPartId, iStateId, TMT_SIZINGTYPE, ST_STRETCH) == ST_TRUESIZE)
			{
				*pSize = sz;
				return TRUE;
			}

			if (!pRect)
			{
				pSize->cy = 0;
				pSize->cx = 0;
			}
			else
			{
				pSize->cy = pRect->bottom - pRect->top;
				pSize->cx = pRect->right - pRect->left;
			}

			if (GetThemeBool(iPartId, iStateId, TMT_UNIFORMSIZING, FALSE))
			{
				pSize->cx = MulDiv(pSize->cy, sz.cx, sz.cy);
			}
		}
	}

	return TRUE;
}
