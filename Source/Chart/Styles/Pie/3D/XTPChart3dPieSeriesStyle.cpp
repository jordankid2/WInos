// XTPChart3dPieSeriesStyle.cpp
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
#include "Common/XTPXMLHelpers.h"
#include "Common/XTPPropExchange.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/PropExchange/XTPPropExchangeEnumerator.h"
#include "Common/PropExchange/XTPPropExchangeEnumeratorPtr.h"
#include "Common/PropExchange/XTPPropExchangeSection.h"
#include "Common/PropExchange/XTPPropExchangeArchive.h"
#include "Common/PropExchange/XTPPropExchangeXMLNode.h"
#include "Common/PropExchange/XTPPropExchangeRegistry.h"
#include "Common/PropExchange/XTPPropExchangeIniFile.h"
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
#include "Chart/XTPChartSeriesPointView.h"
#include "Chart/XTPChartSeriesView.h"
#include "Chart/XTPChartSeriesLabel.h"
#include "Chart/XTPChartPanel.h"
#include "Chart/XTPChartDiagram.h"

#include "Chart/Appearance/XTPChartBorder.h"
#include "Chart/Appearance/XTPChartAppearance.h"

#include "Chart/Styles/Pie/Base/XTPChartPieSeriesStyleBase.h"
#include "Chart/Styles/Pie/Base/XTPChartPieDiagramBase.h"
#include "Chart/Styles/Pie/Base/XTPChartPieSeriesLabelBase.h"
#include "Chart/Styles/Pie/Base/XTPChartPieSeriesPointViewBase.h"
#include "Chart/Styles/Pie/Base/XTPChartPieSeriesViewBase.h"

#include "Chart/Styles/Pie/3D/XTPChart3dPieSeriesStyle.h"
#include "Chart/Styles/Pie/3D/XTPChart3dPieDiagram.h"
#include "Chart/Styles/Pie/3D/XTPChart3dPieSeriesLabel.h"
#include "Chart/Styles/Pie/3D/XTPChart3dPieSeriesView.h"
#include "Chart/XTPChartIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CXTPChart3dPieSeriesStyle

IMPLEMENT_SERIAL(CXTPChart3dPieSeriesStyle, CXTPChartPieSeriesStyleBase, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChart3dPieSeriesStyle::CXTPChart3dPieSeriesStyle()
	: m_nDepth(static_cast<int>(CXTPChart3dDiagram::m_fChartBoxSize / 10))
	, m_bTorus(FALSE)
	, m_3dRotation(0, 0, 90)
{
	Set3D();
	SetLabel(new CXTPChart3dPieSeriesLabel());
}

CXTPChartDiagram* CXTPChart3dPieSeriesStyle::CreateDiagram()
{
	return new CXTPChart3dPieDiagram();
}

CXTPChartSeriesView* CXTPChart3dPieSeriesStyle::CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
{
	return new CXTPChart3dPieSeriesView(pSeries, pDiagramView);
}

CXTPChart3dRotation CXTPChart3dPieSeriesStyle::Get3dRotation() const
{
	CXTPChart3dRotation rotation = m_3dRotation;

	if (!rotation.IsRotated())
	{
		rotation.m_dYaw = GetRotation();
	}

	return rotation;
}

void CXTPChart3dPieSeriesStyle::DoPropExchange(CXTPPropExchange* pPX)
{
	ASSERT_VALID(pPX);

	CXTPChartPieSeriesStyleBase::DoPropExchange(pPX);

	PX_Rotation(pPX, _T("Rotation3D"), m_3dRotation);
	PX_Int(pPX, _T("Depth"), m_nDepth);
	PX_Bool(pPX, _T("IsTorus"), m_bTorus);
}


#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChart3dPieSeriesStyle, CXTPChartPieSeriesStyleBase)
	DISP_PROPERTY_EX_ID(CXTPChart3dPieSeriesStyle, "Depth", 110, OleGetDepth, OleSetDepth, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPChart3dPieSeriesStyle, "IsTorus", 111, OleGetIsTorus, OleSetTorus, VT_BOOL)
	DISP_FUNCTION_ID(CXTPChart3dPieSeriesStyle, "Get3dRotation", 120, OleGet3dRotation, VT_DISPATCH, VTS_NONE)
	DISP_FUNCTION_ID(CXTPChart3dPieSeriesStyle, "Set3dRotation", 121, OleSet3dRotation, VT_EMPTY, VTS_DISPATCH)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPChart3dPieSeriesStyle, CXTPChartPieSeriesStyleBase)
	INTERFACE_PART(CXTPChart3dPieSeriesStyle, XTPDIID__DChartPieSeriesStyle, Dispatch)
	INTERFACE_PART(CXTPChart3dPieSeriesStyle, XTPDIID__DChart3dPieSeriesStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChart3dPieSeriesStyle, XTPDIID__DChart3dPieSeriesStyle)

IMPLEMENT_OLECREATE_EX2_CLSID(CXTPChart3dPieSeriesStyle, "Codejock.Chart3dPieSeriesStyle." _XTP_AXLIB_VERSION,
	XTPCLSID_Chart3dPieSeriesStyle);

int CXTPChart3dPieSeriesStyle::OleGetDepth()
{
	return GetDepth();
}

void CXTPChart3dPieSeriesStyle::OleSetDepth(int dDepth)
{
	SetDepth(dDepth);
}

LPDISPATCH CXTPChart3dPieSeriesStyle::OleGet3dRotation()
{
	return XTPGetDispatch(&m_3dRotation, TRUE);
}

void CXTPChart3dPieSeriesStyle::OleSet3dRotation(LPDISPATCH pRotationDisp)
{
	if (NULL != pRotationDisp)
	{
		CXTPChart3dRotation* pRotation = XTP_DYNAMIC_DOWNCAST_REMOTE(CXTPChart3dRotation,
			CXTPChart3dRotation::FromIDispatchSafe(pRotationDisp));
		if (NULL == pRotation)
		{
			AfxThrowOleException(E_INVALIDARG);
		}

		Set3dRotation(*pRotation);
	}
}

BOOL CXTPChart3dPieSeriesStyle::OleGetIsTorus()
{
	return IsTorus();
}

void CXTPChart3dPieSeriesStyle::OleSetTorus(BOOL bTorus)
{
	SetTorus(bTorus);
}

#endif
