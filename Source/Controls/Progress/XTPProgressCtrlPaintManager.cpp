// XTPProgressCtrlPaintManager.cpp
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

#include "StdAfx.h"

#include "Common/XTPFramework.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPResourceImage.h"
#include "Common/XTPWinThemeWrapper.h"

#include "Controls/Util/XTPControlTheme.h"
#include "Controls/Progress/XTPProgressCtrlPaintManager.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlPaintManager
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlPaintManager::CXTPProgressCtrlPaintManager()
{
	m_bUseWinTheme = TRUE;
	m_nTheme = xtpControlThemeDefault;
}

CXTPProgressCtrlPaintManager::~CXTPProgressCtrlPaintManager()
{

}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlPaintManager, CXTPCmdTarget)

void CXTPProgressCtrlPaintManager::RefreshMetrics()
{
	m_winTheme.OpenThemeData(NULL, L"PROGRESS");

	m_clrBorder.SetStandardValue(::GetSysColor(COLOR_3DSHADOW));
	m_clrBar   .SetStandardValue(::GetSysColor(COLOR_HIGHLIGHT));
	m_clrBack  .SetStandardValue(::GetSysColor(COLOR_3DFACE));
	m_clrText  .SetStandardValue(::GetSysColor(COLOR_HIGHLIGHTTEXT));
}

void CXTPProgressCtrlPaintManager::DrawNcBorders(CDC* pDC, CRect rc)
{
	pDC->DrawEdge(rc, EDGE_SUNKEN, BF_RECT);
}

void CXTPProgressCtrlPaintManager::DeflateRect(XTPPROGRESSDRAWSTRUCT& ds)
{
	UNUSED_ALWAYS(ds.bSmooth);

	if (UsingWinThemes())
	{
		if (ds.bVertical)
			ds.rcClient.DeflateRect(3, 4);
		else
			ds.rcClient.DeflateRect(4, 3);
	}
	else if (!ds.bFlatStyle)
	{
		//  give 1 pixel around the bar
		ds.rcClient.DeflateRect(1, 1);
	}
}

void CXTPProgressCtrlPaintManager::Draw(XTPPROGRESSDRAWSTRUCT& ds)
{
	if (!UsingWinThemes() && !ds.bFlatStyle)
	{
		DrawNcBorders(ds.pDC, ds.rcClient);
		ds.rcClient.DeflateRect(2, 2);
	}

	ds.pDC->FillSolidRect(ds.rcClient, m_clrBack);

	if (UsingWinThemes())
	{
		m_winTheme.DrawThemeBackground(*ds.pDC, ds.bVertical ? PP_BARVERT : PP_BAR, 0, ds.rcClient, NULL);
	}

	int x, dxSpace, dxBlock, nBlocks;
	int iStart, iEnd;

	DeflateRect(ds);

	CRect rc = ds.rcClient;

	if (ds.bMarquee)
	{
		dxBlock = rc.Height() * 2 / 3;
		dxSpace = (ds.bVista || ds.bSmooth) ? 0 : 2;

		if (UsingWinThemes())
		{
			m_winTheme.GetThemeInt(0, 0, TMT_PROGRESSCHUNKSIZE, &dxBlock);
			m_winTheme.GetThemeInt(0, 0, TMT_PROGRESSSPACESIZE, &dxSpace);
		}

		rc.left = ds.nPos + ds.rcClient.left;

		for (int i = 0; i < 5; i++)
		{
			if (rc.left >= ds.rcClient.right)
				rc.left = ds.rcClient.left;

			rc.right = rc.left + dxBlock;

			if (rc.right > ds.rcClient.right)
				rc.right = ds.rcClient.right;

			if (UsingWinThemes())
			{
				m_winTheme.DrawThemeBackground(*ds.pDC, PP_CHUNK, 0, rc, NULL);
			}
			else
			{
				ds.pDC->FillSolidRect(rc, m_clrBar);
			}
			rc.left = rc.right + dxSpace;
		}
	}
	else
	{
		int iLow = ds.range.iLow;
		int iHigh = ds.range.iHigh;

		if (ds.bVertical)
		{
			iStart = rc.top;
			iEnd = rc.bottom;
			dxBlock = (rc.right - rc.left) * 2 / 3;
		}
		else
		{
			iStart = rc.left;
			iEnd = rc.right;
			dxBlock = (rc.bottom - rc.top) * 2 / 3;
		}

		x = ::MulDiv(iEnd - iStart, ds.nPos - iLow, iHigh - iLow);

		dxSpace = 2;

		if (UsingWinThemes())
		{
			m_winTheme.GetThemeInt(0, 0, TMT_PROGRESSCHUNKSIZE, &dxBlock);
			m_winTheme.GetThemeInt(0, 0, TMT_PROGRESSSPACESIZE, &dxSpace);
		}

		if (dxBlock == 0)
			dxBlock = 1;    // avoid div by zero

		if (!UsingWinThemes() && ds.bSmooth)
		{
			dxBlock = 1;
			dxSpace = 0;
		}

		nBlocks = (x + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace); // round up

		for (int i = 0; i < nBlocks; i++)
		{
			if (ds.bVertical)
			{

				rc.top = rc.bottom - dxBlock;

				// are we past the end?
				if (rc.bottom <= ds.rcClient.top)
					break;

				if (rc.top <= ds.rcClient.top)
					rc.top = ds.rcClient.top + 1;

			}
			else
			{
				rc.right = rc.left + dxBlock;

				// are we past the end?
				if (rc.left >= ds.rcClient.right)
					break;

				if (rc.right >= ds.rcClient.right)
					rc.right = ds.rcClient.right - 1;
			}

			if (UsingWinThemes())
			{
				m_winTheme.DrawThemeBackground(*ds.pDC, ds.bVertical ? PP_CHUNKVERT : PP_CHUNK, 0, rc, NULL);
			}
			else
			{
				ds.pDC->FillSolidRect(&rc, m_clrBar);
			}

			if (ds.bVertical)
			{
				rc.bottom = rc.top - dxSpace;
			}
			else
			{
				rc.left = rc.right + dxSpace;
			}
		}
	}

	if (!UsingWinThemes() && !ds.strText.IsEmpty())
	{
		CRect rcText(ds.rcClient);
		rcText.DeflateRect(2,0);
		
		ds.pDC->SetTextColor(m_clrText);
		ds.pDC->SetBkMode(TRANSPARENT);
		CXTPFontDC font(ds.pDC, ds.pFont);
		ds.pDC->DrawText(ds.strText, rcText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER);
	}
}

BOOL CXTPProgressCtrlPaintManager::UsingWinThemes(BOOL bCheckReady /*=FALSE*/)
{
	if (bCheckReady)
	{
		return (m_bUseWinTheme && m_winTheme.IsAppThemed() && m_winTheme.IsAppThemeReady());
	}
	else
	{
		return (m_bUseWinTheme && m_winTheme.IsAppThemed());
	}
}

void CXTPProgressCtrlPaintManager::UseWinThemes(BOOL bUseWinThemes /*=TRUE*/)
{
	m_bUseWinTheme = bUseWinThemes;
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeFlat
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeFlat::CXTPProgressCtrlThemeFlat()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeFlat;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeFlat, CXTPProgressCtrlPaintManager)

void CXTPProgressCtrlThemeFlat::RefreshMetrics()
{
	CXTPProgressCtrlPaintManager::RefreshMetrics();

	m_clrBorder.SetStandardValue(::GetSysColor(COLOR_WINDOWFRAME));
}

void CXTPProgressCtrlThemeFlat::DrawNcBorders(CDC* pDC, CRect rc)
{
	pDC->Draw3dRect(rc, m_clrBorder, m_clrBorder);
	rc.DeflateRect(1,1);
	pDC->Draw3dRect(rc, m_clrBack, m_clrBack);
}

void CXTPProgressCtrlThemeFlat::DeflateRect(XTPPROGRESSDRAWSTRUCT& ds)
{
	if (UsingWinThemes())
	{
		if (ds.bVertical)
			ds.rcClient.DeflateRect(3, 4);
		else
			ds.rcClient.DeflateRect(4, 3);
	}
	else if (!ds.bFlatStyle && ds.bSmooth)
	{
		//  give 1 pixel around the bar
		ds.rcClient.InflateRect(1, 1);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeOffice2000
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeOffice2000::CXTPProgressCtrlThemeOffice2000()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeOffice2000;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeOffice2000, CXTPProgressCtrlPaintManager)

void CXTPProgressCtrlThemeOffice2000::DrawNcBorders(CDC* pDC, CRect rc)
{
	pDC->Draw3dRect(rc, m_clrBorder, GetXtremeColor(COLOR_BTNHIGHLIGHT));
	rc.DeflateRect(1,1);
	pDC->Draw3dRect(rc, m_clrBack, m_clrBack);
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeOffice2003
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeOffice2003::CXTPProgressCtrlThemeOffice2003()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeOffice2003;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeOffice2003, CXTPProgressCtrlThemeOffice2000)

void CXTPProgressCtrlThemeOffice2003::RefreshMetrics()
{
	CXTPProgressCtrlPaintManager::RefreshMetrics();
	
	XTPCurrentSystemTheme systemTheme = XTPColorManager()->GetCurrentSystemTheme();
	switch (systemTheme)
	{
	case xtpSystemThemeBlue:
	case xtpSystemThemeRoyale:
	case xtpSystemThemeAero:
		m_clrBorder.SetStandardValue(RGB(127, 157, 185));
		break;
		
	case xtpSystemThemeOlive:
		m_clrBorder.SetStandardValue(RGB(164, 185, 127));
		break;
		
	case xtpSystemThemeSilver:
		m_clrBorder.SetStandardValue(RGB(165, 172, 178));
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeResource
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeResource::CXTPProgressCtrlThemeResource()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeResource;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeResource, CXTPProgressCtrlThemeOffice2000)

void CXTPProgressCtrlThemeResource::Draw(XTPPROGRESSDRAWSTRUCT& ds)
{
	CXTPResourceImage* pImageTrack = XTPLoadImage(_T("PROGRESSTRACK"));
	CXTPResourceImage* pImageChunk = XTPLoadImage(_T("PROGRESSCHUNK"));
	
	if (!pImageTrack || !pImageChunk)
	{
		ASSERT(FALSE);
		CXTPProgressCtrlThemeOffice2000::Draw(ds);
		return;
	}

	ds.pDC->FillSolidRect(ds.rcClient, m_clrBack);
	pImageTrack->DrawImage(ds.pDC, ds.rcClient, pImageTrack->GetSource(), CRect(2,2,2,2), 0xff00ff);

	ds.rcClient.DeflateRect(2,2);

	int x = ::MulDiv(ds.rcClient.Width(), ds.nPos - ds.range.iLow, ds.range.iHigh - ds.range.iLow);

	CRect rcSrc(pImageChunk->GetSource());
	rcSrc.right -= 4;

	CRect rcDest(ds.rcClient.left, ds.rcClient.top, ds.rcClient.left + x, ds.rcClient.bottom);

	if (rcDest.Width() < rcSrc.Width())
		rcSrc.left = rcSrc.right - rcDest.Width();

	pImageChunk->DrawImage(ds.pDC, rcDest, rcSrc, CRect(2,2,2,2), 0xff00ff);

	if (ds.rcClient.left + x < ds.rcClient.right - 1)
	{
		int nShadow = min(4, ds.rcClient.right - ds.rcClient.left - x);
		rcSrc = CRect(rcSrc.right, rcSrc.top, rcSrc.right + nShadow, rcSrc.bottom);

		rcDest = CRect(rcDest.right, rcDest.top, rcDest.right + nShadow, rcDest.bottom);

		pImageChunk->DrawImage(ds.pDC, rcDest, rcSrc, CRect(0, 2, 0, 2), 0xFF00FF);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeOffice2013
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeOffice2013::CXTPProgressCtrlThemeOffice2013()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeOffice2013;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeOffice2013, CXTPProgressCtrlThemeOffice2000)

void CXTPProgressCtrlThemeOffice2013::DrawNcBorders(CDC* pDC, CRect rc)
{
	pDC->Draw3dRect(rc, m_clrBorder, m_clrBorder);
	rc.DeflateRect(1,1);
	pDC->Draw3dRect(rc, m_clrBack, m_clrBack);
}

void CXTPProgressCtrlThemeOffice2013::RefreshMetrics()
{
	m_clrBorder.SetStandardValue(XTPIniColor(_T("Controls.ProgressBar"), _T("Border"), RGB(188, 188, 188)));
	m_clrBar   .SetStandardValue(XTPIniColor(_T("Controls.ProgressBar"), _T("Bar"),    RGB(  6, 176,  37)));
	m_clrBack  .SetStandardValue(XTPIniColor(_T("Controls.ProgressBar"), _T("Back"),   RGB(230, 230, 230)));
	m_clrText  .SetStandardValue(XTPIniColor(_T("Controls.ProgressBar"), _T("Text"),   RGB(	68,  68,  68)));
}

void CXTPProgressCtrlThemeOffice2013::Draw(XTPPROGRESSDRAWSTRUCT& ds)
{
	CXTPProgressCtrlThemeOffice2000::Draw(ds);
}

/////////////////////////////////////////////////////////////////////////////
// CXTPProgressCtrlThemeVisualStudio2015
/////////////////////////////////////////////////////////////////////////////

CXTPProgressCtrlThemeVisualStudio2015::CXTPProgressCtrlThemeVisualStudio2015()
{
	m_bUseWinTheme = FALSE;
	m_nTheme = xtpControlThemeVisualStudio2015;
}

IMPLEMENT_DYNAMIC(CXTPProgressCtrlThemeVisualStudio2015, CXTPProgressCtrlThemeOffice2013)
