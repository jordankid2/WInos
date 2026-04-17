// XTPChartFunnelDiagram.cpp
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
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Base/Types/XTPSize.h"
#include "Common/Base/Types/XTPRect.h"

#include "Chart/Types/XTPChartTypes.h"
#include "Chart/XTPChartDefines.h"
#include "Chart/XTPChartElement.h"
#include "Chart/XTPChartLegendItem.h"
#include "Chart/XTPChartElementView.h"
#include "Chart/XTPChartPanel.h"
#include "Chart/XTPChartDiagram.h"
#include "Chart/XTPChartDiagramView.h"
#include "Chart/XTPChartSeriesPointView.h"
#include "Chart/XTPChartSeriesView.h"

#include "Chart/Styles/Funnel/XTPChartFunnelDiagram.h"
#include "Chart/Styles/Funnel/XTPChartFunnelSeriesView.h"
#include "Chart/XTPChartIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//////////////////////////////////////////////////////////////////////////
// CXTPChartFunnelDiagram

IMPLEMENT_SERIAL(CXTPChartFunnelDiagram, CXTPChartDiagram, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartFunnelDiagram::CXTPChartFunnelDiagram()
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
#endif
}

CXTPChartDiagramView* CXTPChartFunnelDiagram::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartFunnelDiagramView(this, pParent);
}

void CXTPChartFunnelDiagram::CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView)
{
	if (pView->GetCount() == 0)
		return;

	for (int i = 0; i < pView->GetSeriesView()->GetCount(); i++)
	{
		CXTPChartFunnelSeriesView* pSeriesView = (CXTPChartFunnelSeriesView*)pView->GetSeriesView()->GetAt(i);

		pSeriesView->CalculatePointLayout(pDC, pView->GetBounds());
		pSeriesView->CalculateLabelLayout(pDC);
	}
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartFunnelDiagramView

CXTPChartFunnelDiagramView::CXTPChartFunnelDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent)
	: CXTPChartDiagramView(pDiagram, pParent)
{
	m_pSeriesView = new CXTPChartElementView(this);

	m_pLabelsView = new CXTPChartElementView(this);
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartFunnelDiagram, CXTPChartDiagram)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPChartFunnelDiagram, CXTPChartDiagram)
	INTERFACE_PART(CXTPChartFunnelDiagram, XTPDIID__DChartFunnelDiagram, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartFunnelDiagram, XTPDIID__DChartFunnelDiagram)

IMPLEMENT_OLECREATE_EX2_CLSID(CXTPChartFunnelDiagram, "Codejock.ChartFunnelDiagram." _XTP_AXLIB_VERSION,
	XTPCLSID_ChartFunnelDiagram);

#endif
