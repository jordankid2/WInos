// XTPReportColumn.cpp : implementation of the CXTPReportColumn class.
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
#include "Common/XTPPropExchange.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPMarkupRender.h"
#include "Common/XTPCustomHeap.h"
#include "Common/XTPSmartPtrInternalT.h"
#include "Common/XTPColorManager.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportColumn.h"
#include "ReportControl/XTPReportColumns.h"
#include "ReportControl/XTPReportSubListControl.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportControl.h"
#include "ReportControl/XTPReportHeader.h"
#include "ReportControl/XTPReportPaintManager.h"
#include "ReportControl/XTPReportControlIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CXTPReportColumn, CXTPCmdTarget);

CXTPReportColumnDisplayOptions::CXTPReportColumnDisplayOptions()
{
	m_pColumn       = new CXTPReportColumnDisplaySettings();
	m_pFieldChooser = new CXTPReportColumnDisplaySettings();
	m_pGroupBy      = new CXTPReportColumnDisplaySettings();
}

CXTPReportColumnDisplayOptions::~CXTPReportColumnDisplayOptions()
{
	CMDTARGET_RELEASE(m_pColumn);
	CMDTARGET_RELEASE(m_pFieldChooser);
	CMDTARGET_RELEASE(m_pGroupBy);
}

CXTPReportColumnDisplaySettings* CXTPReportColumnDisplayOptions::Column() const
{
	return m_pColumn;
}

CXTPReportColumnDisplaySettings* CXTPReportColumnDisplayOptions::FieldChooser() const
{
	return m_pFieldChooser;
}

CXTPReportColumnDisplaySettings* CXTPReportColumnDisplayOptions::GroupBy() const
{
	return m_pGroupBy;
}


//////////////////////////////////////////////////////////////////////////
//
CXTPReportColumnDisplaySettings::CXTPReportColumnDisplaySettings()
	: m_bShowIcon(FALSE)
	, m_bShowText(FALSE)
{
}

BOOL CXTPReportColumnDisplaySettings::IsShowIcon() const
{
	return m_bShowIcon;
}

void CXTPReportColumnDisplaySettings::SetShowIcon(BOOL bShowIcon)
{
	m_bShowIcon = bShowIcon;
}

BOOL CXTPReportColumnDisplaySettings::IsShowText() const
{
	return m_bShowText;
}

void CXTPReportColumnDisplaySettings::SetShowText(BOOL bShowText)
{
	m_bShowText = bShowText;
}


//////////////////////////////////////////////////////////////////////////
// CXTPReportColumn

CXTPReportColumn::CXTPReportColumn(int nItemIndex, LPCTSTR strDisplayName, int nWidth,
		BOOL bAutoSize, int nIconID, BOOL bSortable, BOOL bVisible)
{
	_initData(nItemIndex, strDisplayName, _T(""), nWidth, bAutoSize,
		nIconID, bSortable, bVisible);
}

CXTPReportColumn::CXTPReportColumn(int nItemIndex, LPCTSTR strDisplayName, LPCTSTR strInternalName, int nWidth, BOOL bAutoSize, int nIconID, BOOL bSortable, BOOL bVisible)
{
	_initData(nItemIndex, strDisplayName, strInternalName, nWidth, bAutoSize,
		nIconID, bSortable, bVisible);
}

void CXTPReportColumn::_initData(int nItemIndex, LPCTSTR strDisplayName, LPCTSTR strInternalName, int nWidth, BOOL bAutoSize, int nIconID, BOOL bSortable, BOOL bVisible)
{
	m_nItemIndex = nItemIndex;
	m_strInternalName = strInternalName;
	m_strName = strDisplayName;

	m_nIconID = nIconID;
	m_nMinWidth = XTP_DPI_X(10);
	m_nMaxWidth = 0; //unrestricted

	m_bVisible      = bVisible;
	m_bSortable     = bSortable;
	m_bGroupable    = TRUE;
	m_bFixed       = FALSE;
	m_bFrozen       = FALSE;
	m_bFilterable   = TRUE;
	m_bAllowRemove  = TRUE;
	m_bAllowDrag    = TRUE;
	m_bAutoSize     = bAutoSize;
	m_bAutoSortWhenGrouped = TRUE;

	m_nOldVisibleIndex = -1;

	m_bIsResizable = bAutoSize;
	m_rcColumn.SetRectEmpty();

	m_pColumns = NULL;
	m_bSortIncreasing = TRUE;


	m_nMaxItemWidth = 0;
	m_nAlignment = DT_LEFT;
	m_nHeaderAlignment = -1;
	m_nFooterAlignment = -1;

	m_bDrawFooterDivider = TRUE;
	m_bDrawHeaderDivider = TRUE;

	m_bAutoNumbering     = FALSE;
	m_nAutoNumberingBase = 1;


	m_nColumnStaticWidth = m_nColumnAutoWidth = XTP_DPI_X(nWidth);

	m_bShowInFieldChooser = TRUE;
	m_bEditable = TRUE;

	m_bDrawHeaderRowsVGrid = TRUE;
	m_bDrawFooterRowsVGrid = TRUE;

	m_nBestFitMode = xtpColumnBestFitModeVisibleData;

	m_pEditOptions = new CXTPReportRecordItemEditOptions();

	m_pDisplayOptions = new CXTPReportColumnDisplayOptions();
	m_pDisplayOptions->Column()->SetShowIcon(TRUE);
	m_pDisplayOptions->Column()->SetShowText(TRUE);
	m_pDisplayOptions->FieldChooser()->SetShowText(TRUE);
	m_pDisplayOptions->GroupBy()->SetShowText(TRUE);


	m_pMarkupUIElement = NULL;

	m_bPlusMinus = FALSE;
	m_bExpanded = FALSE;
	m_nNextVisualBlock = 0;

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportColumn::~CXTPReportColumn()
{
	CMDTARGET_RELEASE(m_pEditOptions);
	CMDTARGET_RELEASE(m_pDisplayOptions);

	XTPMarkupReleaseElement(m_pMarkupUIElement);
}

void CXTPReportColumn::SetCaption(LPCTSTR lpszCaption)
{
	CString strCaption(lpszCaption);
	if (m_strName == strCaption)
		return;

	m_strName = strCaption;
	if (m_pColumns)
		m_pColumns->GetReportHeader()->OnColumnsChanged(xtpReportColumnCaptionChanged, this);

	ResetMarkupUIElement();
}

void CXTPReportColumn::ResetMarkupUIElement()
{
	XTPMarkupReleaseElement(m_pMarkupUIElement);

	CXTPReportControl* pControl = m_pColumns ? m_pColumns->m_pControl : NULL;
	if (pControl && pControl->GetMarkupContext())
	{
		m_pMarkupUIElement = XTPMarkupParseText(pControl->GetMarkupContext(), m_strName);
	}
}

void CXTPReportColumn::SetFooterText(LPCTSTR strFooter)
{
	m_strFooterText = strFooter;
	if (m_pColumns)
	{
		m_pColumns->GetReportHeader()->GetControl()->AdjustLayout();
		m_pColumns->GetReportHeader()->GetControl()->AdjustScrollBars();
	}
}

void CXTPReportColumn::SetFooterFont(CFont *pFont)
{
	m_xtpFontFooter.DeleteObject(); // set default
	
	if (m_pColumns)
	{
		CFont* pFontDefault = &m_pColumns->GetReportHeader()->GetControl()->GetPaintManager()->m_xtpFontCaption;

		if (pFont && pFont->m_hObject && pFont->m_hObject != pFontDefault->m_hObject)
		{
			LOGFONT lfFooter;
			pFont->GetLogFont(&lfFooter);

			VERIFY(m_xtpFontFooter.CreateFontIndirect(&lfFooter));
		}

		m_pColumns->GetReportHeader()->GetControl()->AdjustLayout();
		m_pColumns->GetReportHeader()->GetControl()->AdjustScrollBars();
	}
}

void CXTPReportColumn::SetDrawFooterDivider(BOOL bSet)
{
	m_bDrawFooterDivider = bSet;
	
	if (m_pColumns)
		m_pColumns->GetReportHeader()->GetControl()->RedrawControl();
}

void CXTPReportColumn::SetDrawHeaderDivider(BOOL bSet)
{
	m_bDrawHeaderDivider = bSet;
	
	if (m_pColumns)
		m_pColumns->GetReportHeader()->GetControl()->RedrawControl();
}

CFont* CXTPReportColumn::GetFooterFont()
{
	if (m_xtpFontFooter.m_hObject != NULL)
		return &m_xtpFontFooter;

	if (m_pColumns)
		return &m_pColumns->GetReportHeader()->GetControl()->GetPaintManager()->m_xtpFontCaption;
	
	return NULL;
}

int CXTPReportColumn::GetNormAlignment(int nAlignment) const
{
	if (!GetColumns()->GetReportHeader()->GetPaintManager()->m_bRevertAlignment)
		return nAlignment;

	return nAlignment & DT_RIGHT ? (nAlignment - DT_RIGHT) : (nAlignment + DT_RIGHT);

}

int CXTPReportColumn::GetAlignment() const
{
	return GetNormAlignment(m_nAlignment);
}

int CXTPReportColumn::GetHeaderAlignment() const
{
	if (m_nHeaderAlignment != -1)
		return m_nHeaderAlignment;

	if (GetColumns()->GetReportHeader()->GetPaintManager()->m_bUseColumnTextAlignment)
		return GetAlignment();

	return GetNormAlignment(DT_LEFT);
}

int CXTPReportColumn::GetFooterAlignment() const
{
	if (m_nFooterAlignment != -1)
		return m_nFooterAlignment;

	if (GetColumns()->GetReportHeader()->GetPaintManager()->m_bUseColumnTextAlignment)
		return GetAlignment();

	return GetNormAlignment(DT_LEFT);
}

void CXTPReportColumn::SetFooterAlignment(int nAlignment)
{
	m_nFooterAlignment = nAlignment;
	
	if (m_pColumns)
	{
		m_pColumns->GetReportHeader()->GetControl()->AdjustLayout();
		m_pColumns->GetReportHeader()->GetControl()->AdjustScrollBars();
	}
}

int CXTPReportColumn::GetWidth() const
{
	return m_nColumnStaticWidth + (!m_bIsResizable ? GetIndent() : 0);
}

CRect CXTPReportColumn::GetRect() const
{
	return m_rcColumn;
}


int CXTPReportColumn::SetWidth(int nNewWidth)
{
	int nOldWidth = m_nColumnStaticWidth;
	m_nColumnStaticWidth = m_nColumnAutoWidth = XTP_DPI_X(nNewWidth);
	
	if (m_pColumns)
		m_pColumns->GetReportHeader()->OnColumnsChanged(xtpReportColumnWidthChanged, this);
	
	return nOldWidth;
}

BOOL CXTPReportColumn::IsSortedIncreasing() const
{
	return m_bSortIncreasing;
}
BOOL CXTPReportColumn::IsSortedDecreasing() const
{
	return !m_bSortIncreasing;
}

BOOL CXTPReportColumn::IsSorted() const
{
	if (m_pColumns)
		return (m_pColumns->GetSortOrder()->IndexOf(this) != -1);

	return FALSE;
}

void CXTPReportColumn::SetTreeColumn(BOOL bIsTreeColumn)
{
	if (m_pColumns == NULL)
		return;

	if (bIsTreeColumn)
	{
		m_pColumns->m_pTreeColumn = this;
	}
	else if (IsTreeColumn())
	{
		m_pColumns->m_pTreeColumn = NULL;
	}
}

BOOL CXTPReportColumn::IsTreeColumn() const
{
	if (m_pColumns)
		return m_pColumns->m_pTreeColumn == this;

	return FALSE;
}

BOOL CXTPReportColumn::IsVisible() const
{
	return m_bVisible;
}


void CXTPReportColumn::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Bool(pPX, _T("SortIncreasing"), m_bSortIncreasing, TRUE);
	PX_Bool(pPX, _T("Visible"), m_bVisible, TRUE);
	PX_Int(pPX, _T("Alignment"), m_nAlignment, DT_LEFT);
	PX_Int(pPX, _T("StaticWidth"), m_nColumnStaticWidth, 0);
	PX_Int(pPX, _T("AutoWidth"), m_nColumnAutoWidth, 0);

	if (pPX->GetSchema() > _XTP_SCHEMA_1041)
	{
		PX_Int(pPX, _T("HeaderAlignment"), m_nHeaderAlignment, -1);
		PX_Int(pPX, _T("FooterAlignment"), m_nFooterAlignment, -1);
	}
	if (pPX->GetSchema() >= _XTP_SCHEMA_1310)
	{
		PX_Bool(pPX, _T("PlusMinus"), m_bPlusMinus, FALSE);
		PX_Int(pPX, _T("NextVisualBlock"), m_nNextVisualBlock, 0);
		PX_Int(pPX, _T("OldVisibleIndex"), m_nOldVisibleIndex, -1);
		PX_Bool(pPX, _T("Expanded"), m_bExpanded, TRUE);
	}
}

BOOL CXTPReportColumn::HasSortTriangle() const
{
	if (m_pColumns == NULL)
		return FALSE;

	if (m_pColumns->GetSortOrder()->IndexOf(this) != -1)
		return TRUE;

	if (m_pColumns->GetGroupsOrder()->IndexOf(this) != -1 && m_bAutoSortWhenGrouped)
		return TRUE;

	return FALSE;
}

int CXTPReportColumn::GetCaptionWidth(CDC* pDC)
{
	int nTextWidth = pDC->GetTextExtent(m_strName).cx;
	if (m_pMarkupUIElement)
	{
		//CSize szText = XTPMarkupMeasureElement(m_pMarkupUIElement, m_rcColumn.Width(), INT_MAX);
		CSize szText = XTPMarkupMeasureElement(m_pMarkupUIElement, nTextWidth, INT_MAX);
		nTextWidth = szText.cx;
	}
	return nTextWidth;
}

BOOL CXTPReportColumn::IsDragging() const
{
	return m_pColumns && m_pColumns->GetReportHeader() && m_pColumns->GetReportHeader()->m_pDragColumn == this;
}

int CXTPReportColumn::GetIndent() const
{
	if (m_pColumns == NULL || !m_pColumns->GetReportHeader())
		return 0;

	if (m_pColumns->GetVisibleAt(0) == this)
		return GetControl()->GetHeaderIndent();

	return 0;
}

int CXTPReportColumn::GetMinWidth() const
{
	return ((m_bIsResizable || m_bAutoSize) ? m_nMinWidth : (m_nColumnStaticWidth) + GetIndent());
}

int CXTPReportColumn::GetMaxWidth() const
{
	return m_nMaxWidth;
}

void CXTPReportColumn::SetVisible(BOOL bVisible)
{
	int IconViewColumn(0);
	if (GetControl())
	{
		IconViewColumn = GetControl()->m_iIconViewColumn;
		if (IconViewColumn > 0 && IconViewColumn == GetIndex())
		{
			if (!bVisible)
				SetWidth(0);
		}
	}
	if (bVisible != m_bVisible)
	{
		if (m_bVisible)
			m_nOldVisibleIndex = GetVisibleIndex();

		m_bVisible = bVisible;
		if (m_pColumns)
			m_pColumns->GetReportHeader()->OnColumnsChanged(xtpReportColumnOrderChanged | (m_bVisible ? xtpReportColumnShown : xtpReportColumnHidden), this);
	}
}

int CXTPReportColumn::GetOldVisibleIndex() const
{
	return m_nOldVisibleIndex;
}

int CXTPReportColumn::GetItemIndex() const
{
	return m_nItemIndex;
}

void CXTPReportColumn::SetItemIndex(int nItemIndex)
{
	m_nItemIndex = nItemIndex;
}

int CXTPReportColumn::GetIndex() const
{
	if (m_pColumns == NULL)
		return -1;

	return m_pColumns->IndexOf(this);
}

int CXTPReportColumn::GetVisibleIndex() const
{
	if (m_pColumns)
	{
		int nIndex = -1;
		for (int i = 0; i < m_pColumns->GetCount(); i++)
		{
			if (m_pColumns->GetAt(i)->IsVisible())
			{
				nIndex++;
				if (m_pColumns->GetAt(i) == this)
					return nIndex;
			}
		}
	}

	return -1;
}

CXTPReportControl* CXTPReportColumn::GetControl() const
{
	if (m_pColumns)
		return m_pColumns->GetReportHeader()->GetControl();

	return NULL;
}

void CXTPReportColumn::SetShowInFieldChooser(BOOL bShow)
{
	if (bShow != m_bShowInFieldChooser)
	{
		m_bShowInFieldChooser = bShow;

		CXTPReportSubListControl* pSubList = m_pColumns ? m_pColumns->GetReportHeader()->GetSubListCtrl() : NULL;
		if (pSubList)
		{
			pSubList->UpdateList();
		}
	}
}

BOOL CXTPReportColumn::IsHotTracking() const
{
	if (m_pColumns)
		return m_pColumns->GetReportHeader()->GetHotTrackingColumn() == this;

	return FALSE;
}

BOOL CXTPReportColumn::IsShowInFieldChooser() const
{
	return m_bShowInFieldChooser;
}

int CXTPReportColumn::GetPrintWidth(int nTotalWidth) const
{
	if (m_pColumns == NULL)
		return 0;

	CXTPReportColumns* pColumns = m_pColumns;

	int nColumnsWidth = 0;
	CXTPReportColumn* pLastAutoColumn = NULL;

	for (int nColumn = 0; nColumn < pColumns->GetCount(); nColumn++)
	{
		CXTPReportColumn* pColumn = pColumns->GetAt(nColumn);
		if (!pColumn->IsVisible())
			continue;

		if (pColumn->IsAutoSize())
		{
			pLastAutoColumn = pColumn;
			nColumnsWidth += pColumn->GetWidth();
		}
		else
		{
			nTotalWidth -= pColumn->GetWidth();
		}
	}

	for (int i = 0; i < pColumns->GetCount(); i++)
	{
		CXTPReportColumn* pColumn = pColumns->GetAt(i);
		if (!pColumn->IsVisible())
			continue;

		int nWidth = pColumn->GetWidth();

		if (pColumn->IsAutoSize())
		{
			if (pColumn == pLastAutoColumn)
			{
				nWidth = max(nTotalWidth, pColumn->GetMinWidth());
			}
			else
			{
				nColumnsWidth = max(XTP_DPI_X(1), nColumnsWidth);

				nWidth =
					max(int(pColumn->GetWidth() * nTotalWidth / nColumnsWidth), pColumn->GetMinWidth());

				nTotalWidth -= nWidth;
				nColumnsWidth -= pColumn->GetWidth();
			}
		}

		if (pColumn == this)
			return nWidth;
	}

	return 0;
}


int CXTPReportColumn::GetBestFitWidth()
{
	CXTPReportControl* pControl = GetControl();
	CXTPReportPaintManager* pPaintManager = pControl->GetPaintManager();
	int nBestColumnWidth = 0;

	CClientDC dc(pControl);
	CXTPFontDC font(&dc, &pPaintManager->m_xtpFontCaption);
	nBestColumnWidth = XTP_DPI_X(6) + GetCaptionWidth(&dc);

	if (GetIconID() != XTP_REPORT_NOICON)
		nBestColumnWidth += pPaintManager->DrawBitmap(NULL, pControl, GetRect(), GetIconID()) + XTP_DPI_X(2);

	if (HasSortTriangle() && pPaintManager->m_DrawSortTriangleStyle != xtpReportDrawSortTriangleNever)
		nBestColumnWidth += XTP_DPI_X(27);

	int nDataWidth = pControl->OnGetColumnDataBestFitWidth(this);

	nBestColumnWidth = max(nBestColumnWidth, nDataWidth);
	nBestColumnWidth = max(nBestColumnWidth, m_nMaxItemWidth);

	return nBestColumnWidth;
}

int CXTPReportColumn::SetMinWidth(int nMinWidth)
{
	int OldMinWidth = m_nMinWidth;
	m_nMinWidth = XTP_DPI_X(nMinWidth);

	return OldMinWidth;
}

int CXTPReportColumn::SetMaxWidth(int nMaxWidth)
{
	int OldMaxWidth = m_nMaxWidth;
	m_nMaxWidth = XTP_DPI_X(nMaxWidth);

	return OldMaxWidth;
}

BOOL CXTPReportColumn::SetFilterable(BOOL bFiltrable)
{
	BOOL bOldFiltrable = m_bFilterable;
	m_bFilterable = bFiltrable;

	return bOldFiltrable;
}

void CXTPReportColumn::EnsureVisible()
{
	CXTPReportControl* pControl = GetControl();
	if (pControl)
		pControl->EnsureVisible(this);
}

int CXTPReportColumn::SetAlignment(int nAlignment)
{
	int nOld = m_nAlignment;
	m_nAlignment = nAlignment;

	return nOld;
}

void CXTPReportColumn::SetAllowDrag(BOOL bAllowDrag)
{
	//if (!m_bPlusMinus)
		m_bAllowDrag = bAllowDrag;
}


void CXTPReportColumn::SetPlusMinus(BOOL bPlusMinus)
{
	m_bPlusMinus = bPlusMinus;

	//if (m_bPlusMinus)
	//  m_bAllowDrag = FALSE;
}

void CXTPReportColumn::SetNextVisualBlock(int nNextVisualBlock)
{
	m_nNextVisualBlock = nNextVisualBlock;
	
	//if (m_pColumns == NULL)
	//	return;
	//
	//int i = GetIndex();
	//int n = m_pColumns->GetCount();
	//int k = 0;
	//for (int j = i + 1; j < n; j++)
	//{
	//  CXTPReportColumn* pCol = m_pColumns->GetAt(j);
	//  BOOL b = pCol->IsVisible();
	//  int l = pCol->GetVisibleIndex();

	//  if (pCol != this && b && k < m_nNextVisualBlock)
	//  {
	//      CString s = pCol->GetCaption();
	//      pCol->SetAllowDrag(FALSE);
	//      k++;
	//  }
	//}
	//m_nNextVisualBlock = k;
}

CXTPReportColumns* CXTPReportColumn::GetColumns() const
{
	return m_pColumns;
}

void CXTPReportColumn::SetExpanded(BOOL bExpanded)
{
	if (m_pColumns == NULL)
		return;

	m_bExpanded = bExpanded;
	int nCnt = m_pColumns->GetCount();
	int iN = GetNextVisualBlock();
	int iVs = GetVisibleIndex();
	CXTPReportColumn* pCol = NULL;
	for (int iC = 0; iC < nCnt; iC++)
	{
		pCol = m_pColumns->GetAt(nCnt - iC - 1);
		if (pCol)
		{
			if (bExpanded)
			{
				int iVc = pCol->GetVisibleIndex();
				if (iVc == -1) continue;
				if (iN > 0 && iVc > iVs && iVc <= iVs + iN)
					pCol->SetVisible(FALSE);
				else if (iN < 0 && iVc < iVs && iVc >= iVs + iN)
					pCol->SetVisible(FALSE);
			}
			else
			{
				int iVc = pCol->GetOldVisibleIndex();
				if (iVc == -1) continue;
				if (iN > 0 && iVc > iVs && iVc <= iVs + iN)
					pCol->SetVisible(TRUE);
				//else if (iN < 0 && iVc <= iVs + 1 && iVc >= iVs + iN)
				else if (iN < 0 && iVc <= iVs + 2 && iVc >= iVs + iN)
					pCol->SetVisible(TRUE);
			}
		}
	}
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportColumn, CXTPCmdTarget)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "Caption", 1, OleGetCaption, SetCaption, VT_BSTR)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "Visible", 2, IsVisible, SetVisible, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "Alignment", 3, m_nAlignment, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "TreeColumn", 4, IsTreeColumn, SetTreeColumn, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "SortAscending", 5, m_bSortIncreasing, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "Sortable", 6, m_bSortable, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "AllowDrag", 7, m_bAllowDrag, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "Index", 8, GetIndex, SetNotSupported, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "ItemIndex", 9, m_nItemIndex, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "Resizable", 10, m_bIsResizable, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "AutoSize", 11, m_bAutoSize, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "Width", 12, GetWidth, SetWidth, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "Icon", 13, m_nIconID, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "Groupable", 14, m_bGroupable, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "ShowInFieldChooser", 15, IsShowInFieldChooser, SetShowInFieldChooser, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "Editable", 16, m_bEditable, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "EditOptions", 17, OleGetEditOptions, SetNotSupported, VT_DISPATCH)
	DISP_FUNCTION_ID(CXTPReportColumn, "Move", 18, OleMove, VT_EMPTY, VTS_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "AllowRemove", 19, m_bAllowRemove, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "Tag", 20, m_oleTag, VT_VARIANT)
	DISP_PROPERTY_ID(CXTPReportColumn, "Tooltip", 21, m_strTooltip, VT_BSTR)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "FooterText", 22, OleGetFooterText, SetFooterText, VT_BSTR)
	DISP_PROPERTY_ID(CXTPReportColumn, "HeaderAlignment", 23, m_nHeaderAlignment, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "FooterAlignment", 24, m_nFooterAlignment, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "AutoSortWhenGrouped", 25, m_bAutoSortWhenGrouped, VT_BOOL)
	DISP_FUNCTION_ID(CXTPReportColumn, "BestFit", 26, OleBestFit, VT_EMPTY, VTS_NONE)

	DISP_PROPERTY_EX_ID(CXTPReportColumn, "FooterFont", 27, OleGetFooterFont, OleSetFooterFont, VT_FONT)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "DrawFooterDivider", 28, GetDrawFooterDivider, SetDrawFooterDivider, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "DrawHeaderRowsVGrid", 29, GetDrawHeaderRowsVGrid, SetDrawHeaderRowsVGrid, VT_BOOL)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "DrawFooterRowsVGrid", 30, GetDrawFooterRowsVGrid, SetDrawFooterRowsVGrid, VT_BOOL)
	DISP_FUNCTION_ID(CXTPReportColumn, "EnsureVisible", 31, EnsureVisible, VT_EMPTY, VTS_NONE)
	DISP_PROPERTY_ID(CXTPReportColumn, "BestFitMode", 32, m_nBestFitMode, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "DrawHeaderDivider", 33, GetDrawHeaderDivider, SetDrawHeaderDivider, VT_BOOL)

	DISP_PROPERTY_ID(CXTPReportColumn, "Filterable", 34, m_bFilterable, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "MinimumWidth", 35, m_nMinWidth, VT_I4)

	DISP_PROPERTY_ID(CXTPReportColumn, "PlusMinus", 36, m_bPlusMinus, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "Expanded", 37, m_bExpanded, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportColumn, "NextVisualBlock", 38, m_nNextVisualBlock, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "VisibleIndex", 39, GetVisibleIndex, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportColumn, "OldVisibleIndex", 40, GetOldVisibleIndex, SetNotSupported, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "MaximumWidth", 41, m_nMaxWidth, VT_I4)
	DISP_PROPERTY_ID(CXTPReportColumn, "Key", 42, m_strInternalName, VT_BSTR)

END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportColumn, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportColumn, XTPDIID_IReportColumn, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportColumn, XTPDIID_IReportColumn)

CXTPReportColumn* AFX_CDECL CXTPReportColumn::FromDispatch(LPDISPATCH pDisp)
{
	CXTPReportColumn* pInst = NULL;
	if (NULL != pDisp)
	{
		pInst = XTP_DYNAMIC_DOWNCAST_REMOTE_(CXTPReportColumn, CXTPReportColumn::FromIDispatchSafe(pDisp));
		if (NULL == pInst)
		{
			AfxThrowOleException(E_INVALIDARG);
		}
	}

	return pInst;
}

BSTR CXTPReportColumn::OleGetCaption()
{
	return m_strName.AllocSysString();
}

LPDISPATCH CXTPReportColumn::OleGetEditOptions()
{
	return m_pEditOptions->GetIDispatch(TRUE);
}

void CXTPReportColumn::OleMove(long nIndex)
{
	if (m_pColumns == NULL)
		return;

	m_pColumns->ChangeColumnOrder(nIndex, m_pColumns->IndexOf(this));
	m_pColumns->GetReportHeader()->OnColumnsChanged(xtpReportColumnOrderChanged | xtpReportColumnMoved, this);
}

BSTR CXTPReportColumn::OleGetFooterText()
{
	return GetFooterText().AllocSysString();
}

void CXTPReportColumn::OleBestFit()
{
	if (m_pColumns && m_pColumns->m_pControl &&
		m_pColumns->m_pControl->GetReportHeader())
	{
		m_pColumns->m_pControl->GetReportHeader()->BestFit(this);
	}
	else
	{
		ASSERT(FALSE);
	}
}

LPFONTDISP CXTPReportColumn::OleGetFooterFont()
{
	return AxCreateOleFont(GetFooterFont(), this, (LPFNFONTCHANGED)&CXTPReportColumn::OleSetFooterFont);
}

void CXTPReportColumn::OleSetFooterFont(LPFONTDISP pFontDisp)
{
	LOGFONT lfNew, lfDef;
	if (!AxGetLogFontFromDispatch(&lfNew, pFontDisp))
		return;

	m_xtpFontFooter.DeleteObject(); // use default
	
	if (m_pColumns)
	{
		CFont* pFontDefault = &m_pColumns->GetReportHeader()->GetControl()->GetPaintManager()->m_xtpFontCaption;
		pFontDefault->GetLogFont(&lfDef);

		if (memcmp(&lfNew, &lfDef, sizeof(LOGFONT)) != 0)
		{
			VERIFY(m_xtpFontFooter.CreateFontIndirect(&lfNew));
		}

		m_pColumns->GetReportHeader()->GetControl()->AdjustLayout();
		m_pColumns->GetReportHeader()->GetControl()->AdjustScrollBars();
	}
}

#endif
