// XTPChartFunnelSeriesLabel.cpp
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

#include "Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h"
#include <math.h>
#include "Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h"

#include "Common/XTPFramework.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPPropExchange.h"
#include "Common/XTPTypeId.h"
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Base/Types/XTPSize.h"
#include "Common/Base/Types/XTPRect.h"

#include "Chart/Types/XTPChartTypes.h"
#include "Chart/XTPChartDefines.h"
#include "Chart/XTPChartElement.h"
#include "Chart/XTPChartLegendItem.h"
#include "Chart/XTPChartElementView.h"
#include "Chart/XTPChartSeriesLabel.h"
#include "Chart/XTPChartSeriesPointView.h"
#include "Chart/XTPChartSeriesView.h"

#include "Chart/Appearance/XTPChartFillStyle.h"
#include "Chart/Appearance/XTPChartBorder.h"

#include "Chart/Drawing/XTPChartDeviceContext.h"
#include "Chart/Drawing/XTPChartDeviceCommand.h"

#include "Chart/XTPChartObjectFactory.h"
#include "Chart/Drawing/XTPChartDrawingObjectFactory.h"

#include "Common/Math/XTPMathUtils.h"
#include "Chart/Utils/XTPChartTextPainter.h"

#include "Chart/Styles/Funnel/XTPChartFunnelSeriesLabel.h"
#include "Chart/Styles/Funnel/XTPChartFunnelSeriesView.h"
#include "Chart/XTPChartIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//////////////////////////////////////////////////////////////////////////
// CXTPChartFunnelSeriesLabel

IMPLEMENT_SERIAL(CXTPChartFunnelSeriesLabel, CXTPChartSeriesLabel, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartFunnelSeriesLabel::CXTPChartFunnelSeriesLabel()
{
	m_nPosition = xtpChartFunnelLabelRight;
	m_nLineLength = 20;
}

CXTPChartFunnelSeriesLabel::~CXTPChartFunnelSeriesLabel()
{
}

CXTPChartFunnelSeriesStyle* CXTPChartFunnelSeriesLabel::GetStyle() const
{
	return (CXTPChartFunnelSeriesStyle*)m_pOwner;
}

void CXTPChartFunnelSeriesLabel::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPChartSeriesLabel::DoPropExchange(pPX);

	PX_Enum(pPX, _T("Position"), m_nPosition, xtpChartFunnelLabelRight);
}

//////////////////////////////////////////////////////////////////////////
// CView;

class CXTPChartFunnelSeriesLabel::CView : public CXTPChartSeriesLabelView
{
public:
	CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView);

protected:
	virtual void CalculateLayout(CXTPChartDeviceContext* pDC);
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:

	XTPChartFunnelLabelPosition GetPosition() const;

};

CXTPChartElementView* CXTPChartFunnelSeriesLabel::CreateView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CView(this, pPointView, pParentView);
}


CXTPChartFunnelSeriesLabel::CView::CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
	: CXTPChartSeriesLabelView(pLabel, pPointView, pParentView)
{

}

void CXTPChartFunnelSeriesLabel::CView::CalculateLayout(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

BOOL CXTPChartFunnelSeriesLabel::IsInside() const
{
	XTPChartFunnelLabelPosition position = GetPosition();
	return position == xtpChartFunnelLabelCenter;

}

XTPChartFunnelLabelPosition CXTPChartFunnelSeriesLabel::CView::GetPosition() const
{
	return ((CXTPChartFunnelSeriesLabel*)m_pLabel)->GetPosition();
}

CXTPChartDeviceCommand* CXTPChartFunnelSeriesLabel::CView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartFunnelSeriesLabel* pLabel = STATIC_DOWNCAST(CXTPChartFunnelSeriesLabel, m_pLabel);

	double lineAngle = 0;

	int borderThickness = m_pLabel->GetBorder()->GetThickness();

	CXTPChartPointF anchorPoint;
	bool bCenter = FALSE;

	CXTPChartFunnelSeriesPointView* pPointView = (CXTPChartFunnelSeriesPointView*)m_pPointView;

	if (pLabel->GetPosition() == xtpChartFunnelLabelRight)
	{
		lineAngle = CXTPMathUtils::Degree2Radian(0);
		anchorPoint.X = pPointView->m_rc.GetLeft() + int((pPointView->m_rc.Width + pPointView->m_rc.Width * (pPointView-> m_dFrom + pPointView->m_dTo) / 2) / 2);
		anchorPoint.Y = (float)(int)pPointView->m_rc.GetCenter().Y;
	}
	else if (pLabel->GetPosition() == xtpChartFunnelLabelLeft)
	{
		lineAngle = CXTPMathUtils::Degree2Radian(180);
		anchorPoint.X = pPointView->m_rc.GetLeft() + int((pPointView->m_rc.Width - pPointView->m_rc.Width * (pPointView-> m_dFrom + pPointView->m_dTo) / 2) / 2);
		anchorPoint.Y = (float)(int)pPointView->m_rc.GetCenter().Y;
	}
	else
	{
		anchorPoint = pPointView->m_rc.GetCenter();
		bCenter = TRUE;
	}


	CXTPChartString text(m_pLabel->GetPointLabel(m_pPointView->GetPoint()));
	if (text.IsEmpty() || text == _T(" "))
		return NULL;

	CXTPChartDeviceCommand* pCommand = CXTPChartDrawingObjectFactory::Create2dHitTestElementCommand(m_pLabel);

	CXTPChartTextPainter painter(pDC, text, m_pLabel);
	CXTPChartSizeF size = painter.GetSize();

	CXTPChartColor clrTextColor = GetActualTextColor();

	CXTPChartPointF startPoint(CXTPMathUtils::Round(anchorPoint.X), CXTPMathUtils::Round(anchorPoint.Y));
	CXTPChartPointF finishPoint(CXTPMathUtils::Round(anchorPoint.X + (cos(lineAngle) * m_pLabel->GetLineLength())),
		CXTPMathUtils::Round(anchorPoint.Y - (sin(lineAngle) * m_pLabel->GetLineLength())));

	CXTPChartRectF innerBounds;
	CXTPChartRectF bounds;

	if (bCenter)
	{
		bounds = CXTPChartRectF(0, 0, size.Width + 2 * borderThickness, size.Height + 2 * borderThickness);
		bounds.Offset(anchorPoint.X - bounds.Width / 2, anchorPoint.Y - bounds.Height / 2);
		bounds.Round();

		innerBounds = bounds;
		innerBounds.Inflate((float)-borderThickness, (float)-borderThickness);
	}
	else
	{
		CXTPChartPointF start(CXTPMathUtils::Round(anchorPoint.X), CXTPMathUtils::Round(anchorPoint.Y));
		CXTPChartPointF finish(CXTPMathUtils::Round(anchorPoint.X + (cos(lineAngle) * m_pLabel->GetLineLength())),
			CXTPMathUtils::Round(anchorPoint.Y - (sin(lineAngle) * m_pLabel->GetLineLength())));

		bounds = CXTPChartSeriesLabelConnectorPainterBase::CalcBorderBoundsForTangentDrawing(finish, lineAngle, size, borderThickness, innerBounds);
	}

	CXTPChartPointF labelPoint = innerBounds.GetLocation();

	CXTPChartColor clrBackColor = m_pLabel->GetActualBackColor();
	pCommand->AddChildCommand(m_pLabel->GetFillStyle()->CreateDeviceCommand(bounds, clrBackColor, clrBackColor));

	painter.SetLocation(labelPoint);

	pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, clrTextColor));


	if (m_pLabel->GetShowLines() && !bCenter)
	{
		CXTPChartColor clrConnectorColor = GetActualConnectorColor();

		CXTPChartSeriesLabelLineConnectorPainter linePainter(startPoint, finishPoint, bounds);
		pCommand->AddChildCommand(linePainter.CreateDeviceCommand(pDC, clrConnectorColor));
	}


	if (borderThickness)
	{
		CXTPChartColor clrBorderColor = GetActualBorderColor();

		pCommand->AddChildCommand(m_pLabel->GetBorder()->CreateInnerBorderDeviceCommand(bounds, clrBorderColor));
	}

	return pCommand;
}


#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartFunnelSeriesLabel, CXTPChartSeriesLabel)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartFunnelSeriesLabel, "Position", 106, m_nPosition, OleChartChanged, VT_I4)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPChartFunnelSeriesLabel, CXTPChartSeriesLabel)
INTERFACE_PART(CXTPChartFunnelSeriesLabel, XTPDIID_ChartFunnelSeriesLabel, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartFunnelSeriesLabel, XTPDIID_ChartFunnelSeriesLabel)

#endif
