// XTPFrameWnd.cpp : implementation for the CXTPFrameWnd and CXTPMDIFrameWnd classes.
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

#include "CommandBars/Resource.h"

#include "Common/XTPFramework.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPHookManager.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPDrawHelpers.h"

#include "CommandBars/XTPCommandBarsDefines.h"
#include "CommandBars/XTPPaintManager.h"
#include "CommandBars/XTPCommandBar.h"
#include "CommandBars/XTPToolBar.h"
#include "CommandBars/XTPCommandBars.h"
#include "CommandBars/XTPShortcutManager.h"
#include "CommandBars/XTPControl.h"
#include "CommandBars/XTPControlEdit.h"
#include "CommandBars/XTPFrameWnd.h"


#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




// CXTPFrameWnd

IMPLEMENT_DYNCREATE(CXTPFrameWnd, CFrameWnd)

CXTPFrameWnd::CXTPFrameWnd()
{
}

// CXTPMDIFrameWnd

IMPLEMENT_DYNCREATE(CXTPMDIFrameWnd, CMDIFrameWnd)

CXTPMDIFrameWnd::CXTPMDIFrameWnd()
{
}

extern const UINT XTP_WM_UPDATE_MDI_CHILD_THEME = RegisterWindowMessage(_T("XTP_WM_UPDATE_MDI_CHILD_THEME"));

void CXTPMDIFrameWnd::UpdateMDIChildrenTheme()
{
	if (NULL != GetSafeHwnd())
	{
		CWnd* pWnd = GetActiveFrame();
		if (pWnd != this)
		{
			while (NULL != pWnd)
			{
				pWnd->SendMessage(XTP_WM_UPDATE_MDI_CHILD_THEME);
				pWnd = pWnd->GetNextWindow();
			}
		}
	}
}

BOOL CXTPMDIFrameWnd::OnCmdMsg(
	UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (nID >= XTP_ID_WORKSPACE_MOVEPREVIOUS && nID <= XTP_ID_WORKSPACE_NEWVERTICAL)
	{
		CWnd* pWnd = CWnd::FromHandlePermanent(m_hWndMDIClient);
		if (pWnd && pWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;
	}

	// then pump through normal frame
	return CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
