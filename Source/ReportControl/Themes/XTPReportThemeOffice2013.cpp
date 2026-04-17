// XTPReportThemeOffice2013.cpp : implementation of the CXTPReportPaintManager class.
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
#include "Common/XTPResourceManager.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPImageManager.h"
#include "Common/XTPResourceImage.h"
#include "Common/XTPColorManager.h"
#include "Common/XTPCustomHeap.h"
#include "Common/XTPSmartPtrInternalT.h"

#include "ReportControl/Resource.h"
#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportPaintManager.h"
#include "ReportControl/XTPReportRow.h"
#include "ReportControl/XTPReportGroupRow.h"
#include "ReportControl/XTPReportColumn.h"
#include "ReportControl/XTPReportHeader.h"
#include "ReportControl/XTPReportControl.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportHyperlink.h"

#include "ReportControl/Themes/XTPReportThemeOffice2013.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// class: CXTPReportThemeOffice2013
/////////////////////////////////////////////////////////////////////////////

CXTPReportThemeOffice2013::CXTPReportThemeOffice2013()
{
	m_bMakeUpper = TRUE;
	m_columnStyle = xtpReportColumnShaded;
	LoadGlyphs();
}

void CXTPReportThemeOffice2013::LoadGlyphs()
{
	CMDTARGET_RELEASE(m_pGlyphs);
	m_pGlyphs = new CXTPImageManager();

	int nBmpID = XTPDpiHelper()->SelectDpiSpecific(
		XTP_IDB_REPORT_GLYPHS_2013_12,
		XTP_IDB_REPORT_GLYPHS_2013_15,
		XTP_IDB_REPORT_GLYPHS_2013_18,
		XTP_IDB_REPORT_GLYPHS_2013_24);
	CBitmap bmp;
	CXTPResourceManager::AssertValid(XTPResourceManager()->LoadBitmap(&bmp, nBmpID));

	CXTPTempColorMask mask(m_pGlyphs, RGB(255, 0, 255));
	m_pGlyphs->SetIcons(bmp, 0, xtpReportGlyphCount, CSize(0, 0)); //11
}

void CXTPReportThemeOffice2013::RefreshMetrics()
{
	// default colors located in OFFICE2013WORD_INI.

	CMDTARGET_RELEASE(m_pHyperlinkStyle);
	m_pHyperlinkStyle = new CXTPReportHyperlinkStyle(XTPIniColor(_T("ReportControl"), _T("HyperLink"), RGB(43, 87, 154)), xtpReportTextDecorationNone);

	CMDTARGET_RELEASE(m_pHyperlinkHoverStyle);
	m_pHyperlinkHoverStyle = new CXTPReportHyperlinkStyle(XTPIniColor(_T("ReportControl"), _T("HyperLinkHover"), RGB(163, 189, 227)), xtpReportTextDecorationNone);

	m_bInvertColumnOnClick = FALSE;

    m_clrBtnFace                .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("BtnFace"),                 RGB(225, 225, 225))); /* grid and separator color between items */
    m_clrBtnFacePressed         .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("BtnFacePressed"),          RGB(243, 243, 243))); /* header column pushed color */
    m_clrBtnText                .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("BtnText"),                 RGB( 68,  68, 68)));
    m_clrControlDark            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("ControlDark"),             RGB(171, 171, 171))); /* header border color */
    m_clrControlLightLight      .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("ControlLightLight"),       RGB(255, 255, 255)));
    m_clrHighlight              .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("Highlight"),               RGB(213, 225, 242))); /* row back selected color */
    m_clrHighlightText          .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("HighlightText"),           RGB( 68,  68,  68))); /* row text selected color */
    m_clrBoldText               .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("BoldText"),                RGB( 43,  87, 154))); /* row text bold color */
    m_clrBoldTextHighlight      .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("BoldTextHighlight"),       RGB( 43,  87, 154))); /* row text bold selected color */
    m_clrGroupText              .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupText"),               RGB( 68,  68,  68))); /* group text color */
    m_clrGroupTextHilite        .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupTextHilite"),         RGB( 68,  68,  68))); /* group text selected color */
    m_clrGroupBack              .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupBack"),               RGB(243, 243, 243))); /* group back color */
    m_clrGroupBackHilite        .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupBackHilite"),         RGB(213, 225, 242))); /* group back selected color */
    m_clrSelectedRow            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("SelectedRow"),             RGB(225, 225, 225)));
    m_clrSelectedRowText        .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("SelectedRowText"),         RGB( 68,  68,  68)));
    m_clrItemShade              .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("ItemShade"),               RGB(255, 255, 255))); /* sorted column back color */
    m_clrHotDivider             .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("HotDivider"),              RGB(255,   0,   0))); /* red arrow indicator when moving columns */
    m_clrFreezeColsDivider      .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("FreezeColsDivider"),       RGB( 43,  87, 154)));
    m_clrWindowText             .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("WindowText"),              RGB( 68,  68,  68))); /* row text color */
    m_clrControlBack            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("ControlBack"),             RGB(255, 255, 255))); /* row back color */
    m_clrConnector              .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("Connector"),               RGB(171, 171, 171))); /* group-by connector line color */
    m_clrIndentControl          .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("IndentControl"),           RGB(255, 255, 255)));
    m_clrCaptionText            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("CaptionText"),             RGB( 68,  68,  68))); /* header text color */
    m_clrCaptionTextPressed     .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("CaptionTextPressed"),      RGB( 68,  68,  68))); /* header text pushed color */
    m_clrHeaderControl          .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("HeaderControl"),           RGB(255, 255, 255))); /* header back color */
    m_clrGroupShadeBorder       .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupShadeBorder"),        RGB(171, 171, 171)));
    m_clrGroupShadeBack         .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupShadeBack"),          RGB(255, 255, 255)));
    m_clrGroupShadeText         .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupShadeText"),          RGB( 68,  68,  68)));
    m_clrGroupRowText           .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupRowText"),            RGB(171, 171, 171)));
    m_clrGroupBoxBack           .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GroupBoxBack"),            RGB(255, 255, 255))); /* group-by area back color */
    m_crlNoGroupByText          .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("NoGroupByText"),           RGB( 68,  68,  68))); /* group-by area text color */
    m_clrGradientColumnSeparator.SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GradientColumnSeparator"), RGB(171, 171, 171)));
    m_clrGlyph                  .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("Glyph"),                   RGB(171, 171, 171))); /* sort arrow color */
    m_clrGlyphHighlight         .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GlyphHighlight"),          RGB(171, 171, 171))); /* sort arrow highlight color */
    m_clrGlyphExpand            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GlyphExpand"),             m_clrControlBack));
    m_clrGlyphBorder            .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GlyphBorder"),             m_clrConnector));
    m_clrGlyphContract          .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("GlyphContract"),           m_clrBtnText));
	m_clrDisableBitmapLightest  .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("DisableBitmapLightest"),   RGB(250, 250, 250)));
	m_clrDisableBitmapDarkest   .SetStandardValue(XTPIniColor(_T("ReportControl"), _T("DisableBitmapDarkest"),    RGB(128, 128, 128)));

	SetGridColor(m_clrBtnFace);

	m_nGroupGridLineStyle = xtpReportGridSolid;

	m_xtpBrushVeriticalGrid.DeleteObject();
	m_xtpBrushHorizontalGrid.DeleteObject();
	m_xtpBrushTreeStructure.DeleteObject();
	m_xtpBrushGroupGridLineStyle.DeleteObject();

	LOGFONT lf;
	m_xtpFontCaption.GetLogFont(&lf);
	SetCaptionFont(lf);

	m_nTreeTextIndentRowParent = m_nTreeTextIndentRowChildren;
}

void CXTPReportThemeOffice2013::DrawColumnText(CDC* pDC, CRect rcText, CString strCaption, int nHeaderAlignment, BOOL bIsHeader, BOOL bColumnPressed)
{
	UNREFERENCED_PARAMETER(bIsHeader);

	pDC->SetTextColor(bColumnPressed ? m_clrCaptionTextPressed : m_clrCaptionText);

	UINT uFlags = DT_END_ELLIPSIS | DT_NOPREFIX;
	if (nHeaderAlignment & DT_RIGHT)
		uFlags |= DT_RIGHT;
	else if (nHeaderAlignment & DT_CENTER)
		uFlags |= DT_CENTER;
	
	if ((nHeaderAlignment & DT_WORDBREAK) > 0)
	{
		uFlags |= DT_WORDBREAK | DT_WORD_ELLIPSIS;

		// try to center vertically because DT_VCENTER works only for DT_SINGLELINE;
		CRect rcTextReal = rcText;
		pDC->DrawText(strCaption, &rcTextReal, uFlags | DT_CALCRECT);

		int nHeightDiff = rcText.Height() - rcTextReal.Height();
		if (nHeightDiff > 1)
			rcText.top += nHeightDiff / 2;
		else if (nHeightDiff < 0)
		{
			rcText.top += XTP_DPI_Y(3);
		}
	}
	else
	{
		rcText.OffsetRect(XTP_DPI_X(2), 0);
		uFlags |= DT_SINGLELINE | DT_VCENTER;
	}

	pDC->DrawText(strCaption, &rcText, uFlags);
}

void CXTPReportThemeOffice2013::DrawPlainColumnBackground(CDC* pDC, CRect rcColumn)
{
	pDC->Draw3dRect(rcColumn, m_clrControlDark, m_clrControlDark);
}

void CXTPReportThemeOffice2013::DrawColumnBackground(CDC* pDC, CXTPReportColumn* pColumn, CRect rcColumn, BOOL& bColumnPressed, BOOL& bDraggingFromHeader, CXTPReportHeader* pHeader)
{
	CXTPReportColumn* pPrevColumn = pHeader ? pHeader->GetNextVisibleColumn(pColumn->GetIndex(), -1) : NULL;

	if (!bDraggingFromHeader)
	{
		if (bColumnPressed)
		{
			pDC->FillSolidRect(rcColumn, m_clrBtnFacePressed);
		}

		DrawHorizontalLine(pDC, rcColumn.left, rcColumn.bottom - XTP_DPI_Y(1), rcColumn.Width(), m_clrControlDark);

		if (pPrevColumn && pColumn->GetDrawHeaderDivider())
			DrawVerticalLine(pDC, rcColumn.left, rcColumn.top + XTP_DPI_Y(3), rcColumn.Height() - XTP_DPI_Y(6), m_clrControlDark);

		if (pColumn->GetControl()->IsGroupByVisible())
			DrawHorizontalLine(pDC, rcColumn.left, rcColumn.top, rcColumn.Width(), m_clrControlDark);
	}
}

void CXTPReportThemeOffice2013::DrawColumnInverted(CDC *pDC, CRect rcColumn, BOOL /*bColumnPressed*/, BOOL bDraggingFromHeader, int nShadowWidth)
{
	if (bDraggingFromHeader)
	{
		rcColumn.bottom += nShadowWidth;
		pDC->InvertRect(&rcColumn);
	}
}

void CXTPReportThemeOffice2013::DrawTriangle(CDC* pDC, CRect rcTriangle, BOOL bToDown, BOOL bDraggingFromHeader, int /*nShadowWidth*/, BOOL bColumnPressed)
{
	if (rcTriangle.Width() > XTP_DPI_X(10) && !bDraggingFromHeader)
	{
		CPoint pt(rcTriangle.CenterPoint());
		pt.Offset(XTP_DPI_X(5), 0);

		if (bToDown)
		{
			CXTPDrawHelpers::Triangle(pDC, CPoint(pt.x - XTP_DPI_X(4), pt.y - XTP_DPI_Y(2)),
				CPoint(pt.x, pt.y + XTP_DPI_Y(2)), CPoint(pt.x + XTP_DPI_X(4), pt.y - XTP_DPI_Y(2)), bColumnPressed ? m_clrGlyphHighlight : m_clrGlyph);
		}
		else
		{
			CXTPDrawHelpers::Triangle(pDC, CPoint(pt.x - XTP_DPI_X(4), pt.y + XTP_DPI_Y(2)),
				CPoint(pt.x, pt.y - XTP_DPI_Y(2)), CPoint(pt.x + XTP_DPI_X(4), pt.y + XTP_DPI_Y(2)), bColumnPressed ? m_clrGlyphHighlight : m_clrGlyph);
		}
	}
}

void CXTPReportThemeOffice2013::DrawWindowFrame(CDC* pDC, CRect rcWindow)
{
	if (::IsWindow(m_pControl->GetSafeHwnd()))
	{
		if (m_pControl->GetExStyle() & WS_EX_STATICEDGE)
		{
			pDC->Draw3dRect(rcWindow, m_clrBtnFace, m_clrBtnFace);
			
			//update scrollbars
			SCROLLINFO  si = { 0 };
			si.cbSize = sizeof(SCROLLINFO);
			m_pControl->GetScrollInfo(SB_VERT, &si, SIF_ALL);
			m_pControl->SetScrollInfo(SB_VERT, &si); // Note: Causes an OnSize message
			m_pControl->GetScrollInfo(SB_HORZ, &si, SIF_ALL);
			m_pControl->SetScrollInfo(SB_HORZ, &si); // Note: Causes an OnSize message
		}
		else
		{
			CXTPReportPaintManager::DrawWindowFrame(pDC, rcWindow);
		}
	}
}

void CXTPReportThemeOffice2013::FillGroupRowMetrics(CXTPReportGroupRow* pRow,
												 XTP_REPORTRECORDITEM_METRICS* pMetrics, BOOL bPrinting)
{
	ASSERT(pRow && pMetrics);
	if (!pRow || !pRow->GetControl() || !pMetrics)
		return;

	BOOL bControlFocused = pRow->GetControl()->HasFocus();

	pMetrics->clrForeground = m_clrGroupRowText;
	pMetrics->clrBackground = XTP_REPORT_COLOR_DEFAULT;

	if (pRow->IsSelected() && bControlFocused && !bPrinting)
	{
		pMetrics->clrForeground = m_clrHighlightText;
		pMetrics->clrBackground = m_clrHighlight;
	}
	else if (m_bShadeGroupHeadings)
	{
		pMetrics->clrForeground = m_clrGroupShadeText;
		pMetrics->clrBackground = m_clrGroupShadeBack;
	}

	pMetrics->pFont = m_bGroupRowTextBold ? &m_xtpFontBoldText : &m_xtpFontText;
}

void CXTPReportThemeOffice2013::DrawGroupRowBack(CDC* pDC, CRect rcRow, XTP_REPORTRECORDITEM_METRICS* pMetrics)
{
	if (pMetrics->clrBackground != XTP_REPORT_COLOR_DEFAULT)
	{
		pDC->FillSolidRect(rcRow, pMetrics->clrBackground);
		DrawHorizontalLine(pDC, rcRow.left, rcRow.top - XTP_DPI_Y(1), rcRow.Width(), m_clrBtnFace);
		DrawHorizontalLine(pDC, rcRow.left, rcRow.top, rcRow.Width(), m_clrBtnFace);
	}
}

void CXTPReportThemeOffice2013::DrawGroupRow(CDC* pDC, CXTPReportGroupRow* pRow, CRect rcRow,
										  XTP_REPORTRECORDITEM_METRICS* pMetrics)
{
	CXTPReportControl *pControl = pRow->GetControl();
	BOOL bControlFocused = pControl->HasFocus();

	BOOL bCustomForegroundMetrics = (pMetrics->clrForeground != m_clrGroupRowText) && (pMetrics->clrForeground != m_clrHighlightText) && (pMetrics->clrForeground != m_clrGroupShadeText);
	BOOL bCustomBackgroundMetrics = (pMetrics->clrBackground != XTP_REPORT_COLOR_DEFAULT) && (pMetrics->clrBackground != m_clrHighlight) && (pMetrics->clrBackground != m_clrGroupShadeBack);

	if (pRow->IsFocused() && bControlFocused && !pDC->IsPrinting() && pControl->IsRowFocusVisible())
	{
		m_clrGroupShadeBorder = m_clrGroupBackHilite;
		pMetrics->clrForeground = m_clrGroupTextHilite;
		//pMetrics->clrForeground = bCustomForegroundMetrics ? pMetrics->clrForeground : m_clrGroupTextHilite;
	}
	else
	{
		m_clrGroupShadeBorder = m_clrGroupBack;
		pMetrics->clrForeground = bCustomForegroundMetrics ? pMetrics->clrForeground : m_clrGroupText;
	}

	pMetrics->clrBackground = bCustomBackgroundMetrics ? pMetrics->clrBackground : m_clrGroupShadeBorder;
/*
	if (m_bMakeUpper)
	{
		pMetrics->strText.MakeUpper();
	}
*/
	CXTPReportPaintManager::DrawGroupRow(pDC, pRow, rcRow, pMetrics);
}


CSize CXTPReportThemeOffice2013::DrawCollapsedBitmap(CDC* pDC, const CXTPReportRow* pRow, CRect& rcBitmap)
{
	CSize szGlyphPlace(XTP_DPI_X(15), XTP_DPI_Y(15));

	if (pDC)
	{
		CSize szGlyph(XTP_DPI_X(5), XTP_DPI_Y(8));
		rcBitmap.right = rcBitmap.left + szGlyphPlace.cx;

		CPoint ptGlyph = rcBitmap.CenterPoint();
		ptGlyph.x -= szGlyph.cx / 2;
		ptGlyph.y -= szGlyph.cy / 2;

		if (pRow->IsExpanded())
		{
			XTPDrawHelpers()->DrawExpandTriangle(pDC, ptGlyph, TRUE,
				m_clrGlyphContract);
		}
		else
		{
			XTPDrawHelpers()->DrawExpandTriangle(pDC, ptGlyph, FALSE,
				m_clrGlyphBorder, m_clrGlyphExpand);
		}
	}

	return szGlyphPlace;
}

void CXTPReportThemeOffice2013::DrawFocusedRow(CDC* pDC, CRect rcRow)
{
	if (m_pControl && ((CXTPReportControl* ) m_pControl)->IsIconView())
		rcRow.DeflateRect(XTP_DPI_X(1), XTP_DPI_Y(1));

	pDC->Draw3dRect(rcRow, m_clrBtnFace, m_clrBtnFace);
	DrawHorizontalLine(pDC, rcRow.left, rcRow.bottom, rcRow.Width(), m_clrBtnFace);

	COLORREF clrTextColor = pDC->SetTextColor(RGB(0,0,0));
	COLORREF clrBkColor = pDC->SetBkColor(m_clrBtnFace);

	pDC->DrawFocusRect(rcRow);
	pDC->SetTextColor(clrTextColor);
	pDC->SetBkColor(clrBkColor);
}

int CXTPReportThemeOffice2013::GetRowHeight(CDC* pDC, CXTPReportRow* pRow)
{
	if (m_pControl->m_bThemeMetrics)
	{
		if (!pRow->IsGroupRow())
			return m_nRowHeight + (IsGridVisible(FALSE) ? XTP_DPI_Y(1) : 0);

		return m_nRowHeight + XTP_DPI_Y(6);
	}
	else
	{
		return CXTPReportPaintManager::GetRowHeight(pDC, pRow);
	}
}

CRect CXTPReportThemeOffice2013::GetGroupRowTextSize(CRect rcBitmap, CRect rcRow, int nBitmapWidth, int nNoIconWidth, int nTextOffset)
{
	if (m_pControl->m_bThemeMetrics)
	{
		CRect rcText(rcBitmap.left, rcRow.top, rcRow.right, rcRow.bottom);
		rcText.left += nBitmapWidth;
		rcText.left += nNoIconWidth;
		rcText.left += nTextOffset;
		rcText.left += m_nTreeTextIndentGroupRow;
		return rcText;
	}
	else
	{
		return CXTPReportPaintManager::GetGroupRowTextSize(
			rcBitmap, rcRow, nBitmapWidth, nNoIconWidth, nTextOffset);
	}
}

CRect CXTPReportThemeOffice2013::GetGroupRowBmpSize(CRect rcRow, int nBitmapOffset)
{
	if (m_pControl->m_bThemeMetrics)
	{
		CRect rcBitmap(rcRow);
		rcBitmap.OffsetRect(nBitmapOffset,0);
		return rcBitmap;
	}
	else
	{
		return CXTPReportPaintManager::GetGroupRowBmpSize(rcRow, nBitmapOffset);
	}
}
