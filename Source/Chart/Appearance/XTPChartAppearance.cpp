// XTPChartAppearance.cpp
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
#include "Common/XTPSynchro.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPXMLHelpers.h"
#include "Common/XTPPropExchange.h"
#include "Common/PropExchange/XTPPropExchangeSection.h"
#include "Common/PropExchange/XTPPropExchangeXMLNode.h"
#include "Common/XTPResourceManager.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/Base/Types/XTPPoint2.h"
#include "Common/Base/Types/XTPPoint3.h"
#include "Common/Base/Types/XTPSize.h"
#include "Common/Base/Types/XTPRect.h"

#include "Chart/Types/XTPChartTypes.h"
#include "Chart/XTPChartDefines.h"
#include "Chart/XTPChartElement.h"
#include "Chart/XTPChartContent.h"

#include "Chart/Appearance/XTPChartAppearance.h"
#include "Chart/Appearance/XTPChartPalette.h"
#include "Chart/Appearance/XTPChartFillStyle.h"

#include "Chart/Drawing/XTPChartDeviceContext.h"
#include "Chart/XTPChartIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////////
// CXTPChartElementAppearance

CXTPChartElementAppearance::CXTPChartElementAppearance(CXTPChartElement* pOwner)
{
	m_pOwner = pOwner;
}

CXTPChartElementAppearance::~CXTPChartElementAppearance()
{
}

void CXTPChartElementAppearance::ApplyToDeviceContext(CXTPChartDeviceContext* pDC)
{
	ASSERT_VALID(pDC);

	// Do nothing by default.
	UNREFERENCED_PARAMETER(pDC);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartAxisAppearance

CXTPChartAxisAppearance::CXTPChartAxisAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{
	InterlacedFillStyle = new CXTPChartFillStyle(this);
}

CXTPChartAxisAppearance::~CXTPChartAxisAppearance()
{
	SAFE_RELEASE(InterlacedFillStyle);
}

void CXTPChartAxisAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("Color"), Color);
	PX_Color(pPX, _T("InterlacedColor"), InterlacedColor);
	PX_Color(pPX, _T("InterlacedColor2"), InterlacedColor2);

	CXTPPropExchangeSection pxInterlacedFillStyle(pPX->GetSection(_T("InterlacedFillStyle")));
	InterlacedFillStyle->DoPropExchange(&pxInterlacedFillStyle);

	CXTPPropExchangeSection pxGridLines(pPX->GetSection(_T("GridLines")));
	PX_Color(&pxGridLines, _T("Color"), GridLinesColor);
	PX_Color(&pxGridLines, _T("MinorColor"), GridLinesMinorColor);

	CXTPPropExchangeSection pxAxisLabel(pPX->GetSection(_T("AxisLabel")));
	PX_Color(&pxAxisLabel, _T("TextColor"), AxisLabelTextColor);

	CXTPPropExchangeSection pxAxisTitle(pPX->GetSection(_T("AxisTitle")));
	PX_Color(&pxAxisTitle, _T("TextColor"), AxisTitleTextColor);

	CXTPPropExchangeSection pxAxisConstantLine(pPX->GetSection(_T("ConstantLine")));
	PX_Color(&pxAxisConstantLine, _T("Color"), ConstantLineColor);
	PX_Color(&pxAxisConstantLine, _T("TextColor"), ConstantLineTextColor);

	CXTPPropExchangeSection pxAxisStrip(pPX->GetSection(_T("Strip")));
	PX_Color(&pxAxisStrip, _T("Color"), StripColor);
	PX_Color(&pxAxisStrip, _T("Color2"), StripColor2);

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagram2DAppearance

CXTPChartDiagram2DAppearance::CXTPChartDiagram2DAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{
	m_pAxisAppearance = new CXTPChartAxisAppearance(this);

	BackgroundFillStyle = new CXTPChartFillStyle(this);
}

CXTPChartDiagram2DAppearance::~CXTPChartDiagram2DAppearance()
{
	SAFE_RELEASE(m_pAxisAppearance);
	SAFE_RELEASE(BackgroundFillStyle);
}

void CXTPChartDiagram2DAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("BackgroundColor"), BackgroundColor);
	PX_Color(pPX, _T("BackgroundColor2"), BackgroundColor2);
	PX_Color(pPX, _T("BorderColor"), BorderColor);

	CXTPPropExchangeSection pxBackgroundFillStyle(pPX->GetSection(_T("BackgroundFillStyle")));
	BackgroundFillStyle->DoPropExchange(&pxBackgroundFillStyle);

	CXTPPropExchangeSection pxAxis(pPX->GetSection(_T("Axis")));
	m_pAxisAppearance->DoPropExchange(&pxAxis);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagram3DAppearance

const CXTPPoint3f CXTPChartDiagram3DAppearance::DefaultLightPosition(1.f, 1.f, 1.f);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultLightAmbientColor(0xff, 0, 0, 0);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultLightDiffuseColor(0xff, 0xff, 0xff, 0xff);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultLightSpecularColor(0xff, 0xff, 0xff, 0xff);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultLightModelAmbientColor(0xff, 0x66, 0x66, 0x66);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultMaterialEmissionColor(0xff, 0, 0, 0);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultMaterialDiffuseColor(0xff, 0x03, 0x0, 0x03);
const CXTPChartColor CXTPChartDiagram3DAppearance::DefaultMaterialSpecularColor(0xff, 0x80, 0x80, 0x80);
const float CXTPChartDiagram3DAppearance::DefaultMaterialShininess(1.f);


CXTPChartDiagram3DAppearance::CXTPChartDiagram3DAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
	, LightPosition(DefaultLightPosition)
	, LightAmbientColor(DefaultLightAmbientColor)
	, LightDiffuseColor(DefaultLightDiffuseColor)
	, LightSpecularColor(DefaultLightSpecularColor)
	, LightModelAmbientColor(DefaultLightModelAmbientColor)
	, MaterialEmissionColor(DefaultMaterialEmissionColor)
	, MaterialDiffuseColor(DefaultMaterialDiffuseColor)
	, MaterialSpecularColor(DefaultMaterialSpecularColor)
	, MaterialShininess(DefaultMaterialShininess)
{
}

CXTPChartDiagram3DAppearance::~CXTPChartDiagram3DAppearance()
{
}

void CXTPChartDiagram3DAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Point(pPX, _T("LightPosition"), LightPosition, DefaultLightPosition);
	PX_Color(pPX, _T("LightAmbientColor"), LightAmbientColor, DefaultLightAmbientColor);
	PX_Color(pPX, _T("LightDiffuseColor"), LightDiffuseColor, DefaultLightDiffuseColor);
	PX_Color(pPX, _T("LightSpecularColor"), LightSpecularColor, DefaultLightSpecularColor);
	PX_Color(pPX, _T("LightModelAmbientColor"), LightModelAmbientColor, DefaultLightModelAmbientColor);
	PX_Color(pPX, _T("MaterialEmissionColor"), MaterialEmissionColor, DefaultMaterialEmissionColor);
	PX_Color(pPX, _T("MaterialDiffuseColor"), MaterialDiffuseColor, DefaultMaterialDiffuseColor);
	PX_Color(pPX, _T("MaterialSpecularColor"), MaterialSpecularColor, DefaultMaterialSpecularColor);
	PX_Float(pPX, _T("MaterialShininess"), MaterialShininess, DefaultMaterialShininess);
}

void CXTPChartDiagram3DAppearance::ApplyToDeviceContext(CXTPChartDeviceContext* pDC)
{
	ASSERT_VALID(pDC);

	CXTPChart3dDeviceContext* p3DDC = DYNAMIC_DOWNCAST(CXTPChart3dDeviceContext, pDC);
	ASSERT("Unsupported device context provided" && NULL != p3DDC);

	if (NULL != p3DDC)
	{
		p3DDC->SetLightPosition(LightPosition);
		p3DDC->SetLightAmbientColor(LightAmbientColor);
		p3DDC->SetLightDiffuseColor(LightDiffuseColor);
		p3DDC->SetLightSpecularColor(LightSpecularColor);
		p3DDC->SetLightModelAmbientColor(LightModelAmbientColor);
		p3DDC->SetMaterialEmissionColor(MaterialEmissionColor);
		p3DDC->SetMaterialDiffuseColor(MaterialDiffuseColor);
		p3DDC->SetMaterialSpecularColor(MaterialSpecularColor);
		p3DDC->SetMaterialShininess(MaterialShininess);
	}
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesLabelAppearance

CXTPChartSeriesLabelAppearance::CXTPChartSeriesLabelAppearance(CXTPChartElement* pOwner)
: CXTPChartElementAppearance(pOwner)
{
}

CXTPChartSeriesLabelAppearance::~CXTPChartSeriesLabelAppearance()
{
}
void CXTPChartSeriesLabelAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("BackgroundColor"), BackgroundColor);
	PX_Color(pPX, _T("TextColor"), TextColor);
	PX_Color(pPX, _T("BorderColor"), BorderColor);
	PX_Color(pPX, _T("ConnectorColor"), ConnectorColor);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesStyleAppearance


CXTPChartSeriesStyleAppearance::CXTPChartSeriesStyleAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{
	m_pLabelAppearance = new CXTPChartSeriesLabelAppearance(this);
}

CXTPChartSeriesStyleAppearance::~CXTPChartSeriesStyleAppearance()
{
	SAFE_RELEASE(m_pLabelAppearance);
}

void CXTPChartSeriesStyleAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPPropExchangeSection pxTitle(pPX->GetSection(_T("Label")));
	m_pLabelAppearance->DoPropExchange(&pxTitle);
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartFinanceStyleAppearance

CXTPChartFinanceStyleAppearance::CXTPChartFinanceStyleAppearance(CXTPChartElement* pOwner)
	: CXTPChartSeriesStyleAppearance(pOwner)
{
}

CXTPChartFinanceStyleAppearance::~CXTPChartFinanceStyleAppearance()
{
}

void CXTPChartFinanceStyleAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPChartSeriesStyleAppearance::DoPropExchange(pPX);

	PX_Color(pPX, _T("UpColor"), UpColor);
	PX_Color(pPX, _T("DownColor"), DownColor);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartPieStyleAppearance

CXTPChartPieStyleAppearance::CXTPChartPieStyleAppearance(CXTPChartElement* pOwner)
	: CXTPChartSeriesStyleAppearance(pOwner)
{
}

CXTPChartPieStyleAppearance::~CXTPChartPieStyleAppearance()
{
}

void CXTPChartPieStyleAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPChartSeriesStyleAppearance::DoPropExchange(pPX);

	PX_Color(pPX, _T("BorderColor"), BorderColor);
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartTitleAppearance

CXTPChartTitleAppearance::CXTPChartTitleAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{

}

void CXTPChartTitleAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("TextColor"), TextColor);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartLegendAppearance

CXTPChartLegendAppearance::CXTPChartLegendAppearance(CXTPChartElement* pOwner)
: CXTPChartElementAppearance(pOwner)
{

}

void CXTPChartLegendAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("TextColor"), TextColor);
	PX_Color(pPX, _T("BackgroundColor"), BackgroundColor);
	PX_Color(pPX, _T("BorderColor"), BorderColor);
}




//////////////////////////////////////////////////////////////////////////
// CXTPChartContentAppearance

CXTPChartContentAppearance::CXTPChartContentAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{
	m_pTitleAppearance = new CXTPChartTitleAppearance(this);
	m_pLegendAppearance = new CXTPChartLegendAppearance(this);
}

CXTPChartContentAppearance::~CXTPChartContentAppearance()
{
	SAFE_RELEASE(m_pLegendAppearance);
	SAFE_RELEASE(m_pTitleAppearance);
}

void CXTPChartContentAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Color(pPX, _T("BackgroundColor"), BackgroundColor);
	PX_Color(pPX, _T("BorderColor"), BorderColor);

	CXTPPropExchangeSection pxTitle(pPX->GetSection(_T("ChartTitle")));
	m_pTitleAppearance->DoPropExchange(&pxTitle);

	CXTPPropExchangeSection pxLegend(pPX->GetSection(_T("Legend")));
	m_pLegendAppearance->DoPropExchange(&pxLegend);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartAppearance

CXTPChartAppearance::CXTPChartAppearance(CXTPChartElement* pOwner)
	: CXTPChartElementAppearance(pOwner)
{
	m_pDiagram2DAppearance = new CXTPChartDiagram2DAppearance(this);
	m_pDiagram3DAppearance = new CXTPChartDiagram3DAppearance(this);
	m_pContentAppearance = new CXTPChartContentAppearance(this);

	m_pSeriesStyleAppearance = new CXTPChartSeriesStyleAppearance(this);
	m_pFinanceStyleAppearance = new CXTPChartFinanceStyleAppearance(this);
	m_pPieStyleAppearance = new CXTPChartPieStyleAppearance(this);

	m_pPalette = new CXTPChartPalette(this);

	// load default colors.
	VERIFY(LoadAppearance(_T("CHART_APPEARANCE_NATURE")));
	VERIFY(LoadPalette(_T("CHART_PALETTE_NATURE")));

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPChartAppearance::~CXTPChartAppearance()
{
	SAFE_RELEASE(m_pDiagram2DAppearance);
	SAFE_RELEASE(m_pDiagram3DAppearance);
	SAFE_RELEASE(m_pContentAppearance);

	SAFE_RELEASE(m_pSeriesStyleAppearance);
	SAFE_RELEASE(m_pFinanceStyleAppearance);
	SAFE_RELEASE(m_pPieStyleAppearance);

	SAFE_RELEASE(m_pPalette);
}

void CXTPChartAppearance::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPPropExchangeSection pxContent(pPX->GetSection(_T("Content")));
	m_pContentAppearance->DoPropExchange(&pxContent);

	CXTPPropExchangeSection pxDiagram2D(pPX->GetSection(_T("Diagram2D")));
	m_pDiagram2DAppearance->DoPropExchange(&pxDiagram2D);

	CXTPPropExchangeSection pxDiagram3D(pPX->GetSection(_T("Diagram3D")));
	m_pDiagram3DAppearance->DoPropExchange(&pxDiagram3D);

	CXTPPropExchangeSection pxSeriesStyleAppearance(pPX->GetSection(_T("SeriesStyle")));
	m_pSeriesStyleAppearance->DoPropExchange(&pxSeriesStyleAppearance);

	CXTPPropExchangeSection pxFinanceSeriesAppearance(pPX->GetSection(_T("FinanceSeriesStyle")));
	m_pFinanceStyleAppearance->DoPropExchange(&pxFinanceSeriesAppearance);

	CXTPPropExchangeSection pxPieSeriesAppearance(pPX->GetSection(_T("PieSeriesStyle")));
	m_pPieStyleAppearance->DoPropExchange(&pxPieSeriesAppearance);
}

BOOL CXTPChartAppearance::LoadAppearance(LPCTSTR lpszAppearance)
{
	CXTPPropExchangeXMLNode px(TRUE, NULL, _T("Appearance"));

	HMODULE hInstance = NULL;
	HRSRC hResource = XTPResourceManager()->FindResource(hInstance, lpszAppearance, RT_HTML);

	if (!hResource)
		return FALSE;

	if (!px.LoadFromResource(hInstance, lpszAppearance , RT_HTML))
		return FALSE;

	if (!px.OnBeforeExchange())
		return FALSE;

	px.SetCompactMode(TRUE);

	DoPropExchange(&px);

	OnChartChanged();

	return TRUE;
}

BOOL CXTPChartAppearance::LoadPalette(LPCTSTR lpszPallete)
{
	CXTPPropExchangeXMLNode px(TRUE, NULL, _T("Palette"));

	HMODULE hInstance = NULL;
	HRSRC hResource = XTPResourceManager()->FindResource(hInstance, lpszPallete, RT_HTML);

	if (!hResource)
		return FALSE;

	if (!px.LoadFromResource(hInstance, lpszPallete, RT_HTML))
		return FALSE;

	if (!px.OnBeforeExchange())
		return FALSE;

	px.SetCompactMode(TRUE);

	m_pPalette->DoPropExchange(&px);

	OnChartChanged();

	return TRUE;
}

CXTPChartAppearance* CXTPChartAppearance::GetAppearance(const CXTPChartElement* pElement)
{
	return pElement->GetContent()->GetAppearance();
}


CXTPChartColor CXTPChartAppearance::GetLightColor(const CXTPChartColor& clr)
{
	COLORREF clrBackground = RGB(clr.GetR(), clr.GetG(), clr.GetB());

	DWORD dwHSLBackground = CXTPDrawHelpers::RGBtoHSL(clrBackground);
	DWORD dwL = GetBValue(dwHSLBackground);
	DWORD dwLight = (dwL + 240) / 2;

	CXTPChartColor res;
	res.SetFromCOLORREF(CXTPDrawHelpers::HSLtoRGB(RGB(GetRValue(dwHSLBackground), GetGValue(dwHSLBackground), dwLight)));

	return res;
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartAppearance, CXTPChartElementAppearance)
	DISP_FUNCTION_ID(CXTPChartAppearance, "SetAppearance", 1, OleSetAppearance, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CXTPChartAppearance, "SetPalette", 2, OleSetPalette, VT_EMPTY, VTS_BSTR)
	DISP_FUNCTION_ID(CXTPChartAppearance, "LoadAppearance", 3, OleLoadAppearance, VT_EMPTY, VTS_DISPATCH)
	DISP_FUNCTION_ID(CXTPChartAppearance, "LoadPalette", 4, OleLoadPalette, VT_EMPTY, VTS_DISPATCH)
	DISP_PROPERTY_EX_ID(CXTPChartAppearance, "Palette", 5, OleGetPalette, SetNotSupported, VT_DISPATCH)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPChartAppearance, CXTPChartElementAppearance)
	INTERFACE_PART(CXTPChartAppearance, XTPDIID_ChartAppearance, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartAppearance, XTPDIID_ChartAppearance)

void CXTPChartAppearance::OleSetAppearance(LPCTSTR lpszName)
{
	CString str(lpszName);
	str.MakeUpper();
	str.Replace(_T(" "), _T(""));

	LoadAppearance(_T("CHART_APPEARANCE_") + str);
}

void CXTPChartAppearance::OleSetPalette(LPCTSTR lpszName)
{
	CString str(lpszName);
	str.MakeUpper();
	str.Replace(_T(" "), _T(""));

	LoadPalette(_T("CHART_PALETTE_") + str);
}


void CXTPChartAppearance::OleLoadAppearance(LPDISPATCH lpPropExchage)
{
	CXTPPropExchangeSection px(PropExchangeFromControl(lpPropExchage));

	if ((CXTPPropExchange*)&px == NULL)
		return;

	if (px->OnBeforeExchange())
	{
		if (px->IsKindOf(RUNTIME_CLASS(CXTPPropExchangeXMLNode)))
			((CXTPPropExchangeXMLNode*)&px)->SetCompactMode(TRUE);

		DoPropExchange(&px);

		OnChartChanged();
	}
}

void CXTPChartAppearance::OleLoadPalette(LPDISPATCH lpPropExchage)
{
	CXTPPropExchangeSection px(PropExchangeFromControl(lpPropExchage));

	if ((CXTPPropExchange*)&px == NULL)
		return;

	if (px->OnBeforeExchange())
	{
		if (px->IsKindOf(RUNTIME_CLASS(CXTPPropExchangeXMLNode)))
			((CXTPPropExchangeXMLNode*)&px)->SetCompactMode(TRUE);

		m_pPalette->DoPropExchange(&px);

		OnChartChanged();
	}
}

LPDISPATCH CXTPChartAppearance::OleGetPalette()
{
	return XTPGetDispatch(GetPalette());
}

#endif
