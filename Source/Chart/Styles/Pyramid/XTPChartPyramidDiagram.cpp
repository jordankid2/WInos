// XTPChartPyramidDiagram.cpp
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
#include "Common/XTPPropExchange.h"
#include "Common/XTPFramework.h"
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Base/Types/XTPSize.h"
#include "Common/Base/Types/XTPRect.h"

#include "Chart/Types/XTPChartTypes.h"
#include "Chart/XTPChartDefines.h"
#include "Chart/XTPChartElement.h"
#include "Chart/XTPChartLegendItem.h"
#include "Chart/XTPChartElementView.h"
#include "Chart/XTPChartSeriesStyle.h"
#include "Chart/XTPChartPanel.h"
#include "Chart/XTPChartDiagram.h"
#include "Chart/XTPChartDiagramView.h"
#include "Chart/XTPChartSeriesPointView.h"
#include "Chart/XTPChartSeriesView.h"

#include "Chart/Styles/Pyramid/XTPChartPyramidDiagram.h"
#include "Chart/Styles/Pyramid/XTPChartPyramidSeriesStyle.h"
#include "Chart/XTPChartIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramidDiagram

IMPLEMENT_SERIAL(CXTPChartPyramidDiagram, CXTPChartDiagram, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPyramidDiagram::CXTPChartPyramidDiagram()
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
#endif
}

CXTPChartDiagramView* CXTPChartPyramidDiagram::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartPyramidDiagramView(this, pParent);
}

void CXTPChartPyramidDiagram::CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView)
{
	if (pView->GetCount() == 0)
		return;

	for (int i = 0; i < pView->GetSeriesView()->GetCount(); i++)
	{
		CXTPChartPyramidSeriesView* pSeriesView = (CXTPChartPyramidSeriesView*)pView->GetSeriesView()->GetAt(i);

		pSeriesView->CalculatePointLayout(pDC, pView->GetBounds());
		pSeriesView->CalculateLabelLayout(pDC);
	}
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramidDiagramView

CXTPChartPyramidDiagramView::CXTPChartPyramidDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent)
	: CXTPChartDiagramView(pDiagram, pParent)
{
	m_pSeriesView = new CXTPChartElementView(this);

	m_pLabelsView = new CXTPChartElementView(this);
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartPyramidDiagram, CXTPChartDiagram)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPChartPyramidDiagram, CXTPChartDiagram)
	INTERFACE_PART(CXTPChartPyramidDiagram, XTPDIID__DChartPyramidDiagram, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartPyramidDiagram, XTPDIID__DChartPyramidDiagram)

IMPLEMENT_OLECREATE_EX2_CLSID(CXTPChartPyramidDiagram, "Codejock.ChartPyramidDiagram." _XTP_AXLIB_VERSION,
	XTPCLSID_ChartPyramidDiagram);

#endif
