// XTPReportThemeDefault.cpp : implementation of the CXTPReportPaintManager class.
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
#include "Common/XTPCustomHeap.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportPaintManager.h"
#include "ReportControl/XTPReportColumn.h"
#include "ReportControl/XTPReportHeader.h"

#include "ReportControl/Themes/XTPReportThemeDefault.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// class: CXTPReportThemeDefault
/////////////////////////////////////////////////////////////////////////////

CXTPReportThemeDefault::CXTPReportThemeDefault()
{
	m_columnStyle = xtpReportColumnShaded;
}

void CXTPReportThemeDefault::DrawColumnBackground(CDC* pDC, CXTPReportColumn* pColumn, CRect rcColumn, BOOL& /*bColumnPressed*/, BOOL& /*bDraggingFromHeader*/, CXTPReportHeader* pHeader)
{
	CXTPReportColumn* pPrevColumn = pHeader ? pHeader->GetNextVisibleColumn(pColumn->GetIndex(), -1) : NULL;

	pDC->DrawEdge(rcColumn, EDGE_RAISED, BF_RECT);

	if (!pColumn->GetDrawHeaderDivider())
	{
		DrawVerticalLine(pDC, rcColumn.right - XTP_DPI_X(1), rcColumn.top + XTP_DPI_Y(1), rcColumn.Height() - XTP_DPI_Y(2), m_clrHeaderControl);
		pDC->SetPixel(rcColumn.right - XTP_DPI_X(1), rcColumn.top, m_clrControlLightLight);
	}
	if (pPrevColumn && !pPrevColumn->GetDrawHeaderDivider())
		DrawVerticalLine(pDC, rcColumn.left, rcColumn.top + XTP_DPI_Y(1), rcColumn.Height() - XTP_DPI_Y(2), m_clrHeaderControl);
}
