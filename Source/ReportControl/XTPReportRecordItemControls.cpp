// XTPReportRecordItemControls.cpp
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
#include "Common/Tmschema.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPPropExchange.h"
#include "Common/PropExchange/XTPPropExchangeEnumerator.h"
#include "Common/PropExchange/XTPPropExchangeEnumeratorPtr.h"
#include "Common/PropExchange/XTPPropExchangeSection.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPCustomHeap.h"
#include "Common/XTPSmartPtrInternalT.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportControl.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportRecordItemControls.h"
#include "ReportControl/XTPReportControlIIDs.h"

#pragma warning(disable: 4097) // TBase' used as synonym for class-name


#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemControl

IMPLEMENT_SERIAL(CXTPReportRecordItemControl, CXTPCmdTarget, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)
IMPLEMENT_SERIAL(CXTPReportRecordItemButton, CXTPReportRecordItemControl, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

IMPLEMENT_DYNAMIC(CXTPReportRecordItemControls, CXTPCmdTarget)
//////////////////////////////////////////////////////////////////////////

CXTPReportRecordItemControl::CXTPReportRecordItemControl(LPCTSTR szCaption)
	: m_strCaption(szCaption)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif

	m_nType = 0;
	m_nIndex = 0;


	LOGFONT lfIcon;
	VERIFY(CXTPDrawHelpers::GetIconLogFont(&lfIcon));


	m_clrCaption = GetSysColor(COLOR_BTNTEXT);
	m_sizeControl = CSize(0, 0);
	m_rcControl = CRect(0, 0, 0, 0);
	m_unFlags = 0;
	m_Alignment = xtpItemControlUnknown;
	m_bEnabled = TRUE;
	m_nState = 0;
	m_bThemed = FALSE;
}

CXTPReportRecordItemControl::~CXTPReportRecordItemControl()
{
	m_mapIcon.RemoveAll();
}

CFont* CXTPReportRecordItemControl::GetFont()
{
	return &m_xtpFontCaption;
}

void CXTPReportRecordItemControl::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Int(pPX, _T("Type"), m_nType);
	PX_String(pPX, _T("Caption"), m_strCaption);
	PX_ULong(pPX, _T("CaptionColor"), (ULONG&)m_clrCaption);
	PX_Size(pPX, _T("Size"), m_sizeControl, CSize(-1, -1));
	PX_ULong(pPX, _T("Flags"), (ULONG&)m_unFlags);
	PX_Int(pPX, _T("Alignment"), (int&)m_Alignment);
	PX_Bool(pPX, _T("Enable"), m_bEnabled);
	PX_Bool(pPX, _T("Themed"), m_bThemed);

	CXTPPropExchangeEnumeratorPtr pEnumItems(pPX->GetEnumerator(_T("StateIcons")));

	if (pPX->IsStoring())
	{
		DWORD dwCount = (DWORD)m_mapIcon.GetCount();
		POSITION posItem = pEnumItems->GetPosition(dwCount);

		POSITION posData = m_mapIcon.GetStartPosition();
		while (posData)
		{
			int nKey = 0, nValue = 0;
			m_mapIcon.GetNextAssoc(posData, nKey, nValue);

			CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
			PX_Int(&secItem, _T("State"), nKey);
			PX_Int(&secItem, _T("Icon"), nValue);
		}
	}
	else
	{
		m_mapIcon.RemoveAll();

		POSITION posItem = pEnumItems->GetPosition();

		while (posItem)
		{
			CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
			int nKey = 0, nValue = 0;
			PX_Int(&secItem, _T("State"), nKey, 0);
			PX_Int(&secItem, _T("Icon"), nValue, 0);

			m_mapIcon[nKey] = nValue;
		}
	}
}

void CXTPReportRecordItemControl::SetFont(CFont* pFont)
{
	ASSERT_VALID(pFont);

	LOGFONT lf;
	pFont->GetLogFont(&lf);

	m_xtpFontCaption.DeleteObject();
	m_xtpFontCaption.CreateFontIndirect(&lf);
}

int CXTPReportRecordItemControl::GetIconIndex(int nState) const
{
	int nIconIndex = XTP_REPORT_NOICON;
	if (m_mapIcon.Lookup(nState, nIconIndex))
		return nIconIndex;
	m_mapIcon.Lookup(0, nIconIndex);
	return nIconIndex;
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportRecordItemControl, CXTPCmdTarget)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Type", 1, GetType, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Index", 2, GetIndex, SetNotSupported, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Caption", 3, OleGetCaption, OleSetCaption, VT_BSTR)
	DISP_PROPERTY_ID(CXTPReportRecordItemControl, "CaptionColor", 4, m_clrCaption, VT_COLOR)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Font", 5, OleGetFont, OleSetFont, VT_FONT)
	DISP_PROPERTY_ID(CXTPReportRecordItemControl, "Alignment", 6, m_Alignment, VT_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemControl, "SetSize", 7, OleSetSize, VT_EMPTY, VTS_I4 VTS_I4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Width", 8, GetWidth, SetWidth, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Height", 9, GetHeight, SetHeight, VT_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemControl, "GetRect", 10, OleGetRect, VT_EMPTY, VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemControl, "Enable", 11, GetEnable, SetEnable, VT_BOOL)
	DISP_PROPERTY_ID(CXTPReportRecordItemControl, "Themed", 12, m_bThemed, VT_BOOL)
	DISP_FUNCTION_ID(CXTPReportRecordItemControl, "SetIconIndex", 13, SetIconIndex, VT_EMPTY, VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemControl, "GetIconIndex", 14, GetIconIndex, VT_I4, VTS_I4)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRecordItemControl, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportRecordItemControl, XTPDIID_IReportItemControl, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRecordItemControl, XTPDIID_IReportItemControl)

BSTR CXTPReportRecordItemControl::OleGetCaption()
{
	return m_strCaption.AllocSysString();
}

void CXTPReportRecordItemControl::OleSetCaption(LPCTSTR pcszCaption)
{
	m_strCaption = pcszCaption;
}

LPFONTDISP CXTPReportRecordItemControl::OleGetFont()
{
	return AxCreateOleFont(GetFont(), this, (LPFNFONTCHANGED)&CXTPReportRecordItemControl::OleSetFont);
}

void CXTPReportRecordItemControl::OleSetFont(LPFONTDISP pFontDisp)
{
	ASSERT((pFontDisp == NULL) || AfxIsValidAddress(pFontDisp, sizeof(IDispatch), FALSE));

	m_xtpFontCaption.DeleteObject();

	LOGFONT lf;
	if (AxGetLogFontFromDispatch(&lf, pFontDisp))
	{
		m_xtpFontCaption.CreateFontIndirect(&lf);
	}
}

void CXTPReportRecordItemControl::OleSetSize(long cx, long cy)
{
	m_sizeControl = CSize(cx, cy);
}

void CXTPReportRecordItemControl::OleGetRect(long* pnLeft, long* pnTop, long* pnRight, long* pnBottom)
{
	if (pnLeft)
		*pnLeft = m_rcControl.left;
	if (pnTop)
		*pnTop = m_rcControl.top;
	if (pnRight)
		*pnRight = m_rcControl.right;
	if (pnBottom)
		*pnBottom = m_rcControl.bottom;
}

#endif


//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemButton

CXTPReportRecordItemButton::CXTPReportRecordItemButton(LPCTSTR szCaption)
	: CXTPReportRecordItemControl(szCaption)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
#endif

	m_nState = m_nSavedState = PBS_NORMAL;
}

void CXTPReportRecordItemButton::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPReportRecordItemControl::DoPropExchange(pPX);
}

void CXTPReportRecordItemButton::OnLButtonDown(XTP_REPORTRECORDITEM_CLICKARGS* pClickArgs)
{
	UNREFERENCED_PARAMETER(pClickArgs);

	if (GetEnable())
		m_nState = PBS_PRESSED;
}

void CXTPReportRecordItemButton::OnLButtonUp(XTP_REPORTRECORDITEM_CLICKARGS* pClickArgs)
{
	//UNREFERENCED_PARAMETER(pClickArgs);
//>>attempt for 23588 issue case
	if (pClickArgs
		&& pClickArgs->pControl
		&& pClickArgs->pControl->IsEditMode())
		return;
//>>attempt for 23588 issue case

	m_nState = GetEnable() ? PBS_NORMAL : PBS_DISABLED;
	if (m_rcControl.PtInRect(pClickArgs->ptClient)
		&& pClickArgs->pControl
		&& m_rcControl.PtInRect(pClickArgs->pControl->m_mouseDownState.ptMouse))
	{
		XTP_NM_REPORTITEMCONTROL nm;
		::ZeroMemory(&nm, sizeof(nm));
		nm.pItem = pClickArgs->pItem;
		nm.pRow = pClickArgs->pRow;
		nm.pColumn = pClickArgs->pColumn;
		nm.pt = pClickArgs->ptClient;
		nm.pItemControl = this;
		pClickArgs->pControl->SendNotifyMessage(XTP_NM_REPORT_ITEMBUTTONCLICK, (NMHDR*)&nm);
	}
}

void CXTPReportRecordItemButton::OnMouseEnter(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(point);
	if (m_nSavedState == PBS_PRESSED && (nFlags & MK_LBUTTON))
		m_nState = m_nSavedState;
}

void CXTPReportRecordItemButton::OnMouseLeave(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(point);
	m_nSavedState = m_nState;
	m_nState = GetEnable() ? PBS_NORMAL : PBS_DISABLED;
}

void CXTPReportRecordItemButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_rcControl.PtInRect(point) && m_nSavedState == PBS_PRESSED && (nFlags & MK_LBUTTON))
		m_nState = m_nSavedState;
}


//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemControls


CXTPReportRecordItemControls::CXTPReportRecordItemControls()
{
	m_pRecordItem = NULL;

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportRecordItemControls::CXTPReportRecordItemControls(CXTPReportRecordItem* pRecordItem)
: m_pRecordItem(pRecordItem)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportRecordItemControls::~CXTPReportRecordItemControls()
{
	RemoveAll();
}

void CXTPReportRecordItemControls::DoPropExchange(CXTPPropExchange* pPX)
{
	int nCount = (int)GetSize();

	CXTPPropExchangeEnumeratorPtr pEnumItems(pPX->GetEnumerator(_T("RecordItemControl")));

	if (pPX->IsStoring())
	{
		POSITION posItem = pEnumItems->GetPosition((DWORD)nCount);

		for (int i = 0; i < nCount; i++)
		{
			CXTPReportRecordItemControl* pItemControl = GetAt(i);
			ASSERT(pItemControl);
			if (!pItemControl)
				AfxThrowArchiveException(CArchiveException::badClass);

			CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
			PX_Object(&secItem, pItemControl, RUNTIME_CLASS(CXTPReportRecordItemControl));
		}
	}
	else
	{
		RemoveAll();

		POSITION posItem = pEnumItems->GetPosition();

		while (posItem)
		{
			CXTPReportRecordItemControl* pItemControl = NULL;

			CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
			PX_Object(&secItem, pItemControl, RUNTIME_CLASS(CXTPReportRecordItemControl));

			if (!pItemControl)
				AfxThrowArchiveException(CArchiveException::badClass);

			AddControl(pItemControl);
		}
	}
}

void CXTPReportRecordItemControls::RemoveAll()
{
	for (int nItem = (int)GetSize() - 1; nItem >= 0; nItem--)
	{
		CXTPReportRecordItemControl* pItem = GetAt(nItem);
		if (pItem)
			pItem->InternalRelease();
	}

	if (m_pRecordItem)
	{
		m_pRecordItem->m_pFocusedItemControl = 0;
	}

	TBase::RemoveAll();
}

void CXTPReportRecordItemControls::RemoveAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	CXTPReportRecordItemControl* pItem = GetAt(nIndex);

	if (m_pRecordItem && m_pRecordItem->m_pFocusedItemControl == pItem)
	{
		m_pRecordItem->m_pFocusedItemControl = 0;
	}

	if (pItem)
		pItem->InternalRelease();

	TBase::RemoveAt(nIndex);

	RefreshIndexes(nIndex);
}

void CXTPReportRecordItemControls::RefreshIndexes(int nIndexStart)
{
	for (int i=nIndexStart; i<GetSize(); i++)
	{
		CXTPReportRecordItemControl *pItem = GetAt(i);

		if (NULL != pItem)
		{
			pItem->m_nIndex = i;
		}
	}
}

CXTPReportRecordItemControl* CXTPReportRecordItemControls::AddControl(int nType, int nIndex)
{
	CXTPReportRecordItemControl* pControl = NULL;
	switch (nType)
	{
		case xtpItemControlTypeButton :
			pControl = (CXTPReportRecordItemControl*) new CXTPReportRecordItemButton;
			break;
		default:
			ASSERT(FALSE);
	}
	if (pControl)
		pControl->m_nType = nType;

	return AddControl(pControl, nIndex);
}

CXTPReportRecordItemControl* CXTPReportRecordItemControls::AddControl(CXTPReportRecordItemControl* pControl, int nIndex)
{
	if (!pControl)
		return NULL;

	if (nIndex < 0 || nIndex >= GetSize())
		nIndex = Add(pControl);
	else
		InsertAt(nIndex, pControl);

	pControl->m_nIndex = nIndex;

	RefreshIndexes(nIndex + 1);

	return pControl;
}

void CXTPReportRecordItemControls::CopyFrom(CXTPReportRecordItemControls* pSrc)
{
	if (pSrc == this)
		return;

	RemoveAll();

	if (!pSrc)
		return;

	int nCount = pSrc->GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CXTPReportRecordItemControl* pItem = pSrc->GetAt(i);
		if (pItem)
		{
			pItem->InternalAddRef();
			Add(pItem);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CXTPReportRecordItemControlHookWnd, CWnd)
//{{AFX_MSG_MAP(CXTPReportRecordItemControlHookWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CXTPReportRecordItemControlHookWnd::CXTPReportRecordItemControlHookWnd(XTP_REPORTRECORDITEM_CLICKARGS* pClickArgs)
{
	m_ClickArgs = *pClickArgs;

	m_ClickArgs.AddRef();
}

CXTPReportRecordItemControlHookWnd::~CXTPReportRecordItemControlHookWnd()
{
	m_ClickArgs.Release();
}

void CXTPReportRecordItemControlHookWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);
	m_ClickArgs.ptClient = point;
	ClientToScreen(&m_ClickArgs.ptClient);
	m_ClickArgs.pItem->OnLButtonDown(&m_ClickArgs);
}

void CXTPReportRecordItemControlHookWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	UNREFERENCED_PARAMETER(nFlags);
	m_ClickArgs.ptClient = point;
	ClientToScreen(&m_ClickArgs.ptClient);
	m_ClickArgs.pItem->OnLButtonUp(&m_ClickArgs);
}

void CXTPReportRecordItemControlHookWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	m_ClickArgs.pItem->OnMouseMove(nFlags, point, m_ClickArgs.pControl);
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportRecordItemButton, CXTPReportRecordItemControl)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRecordItemButton, CXTPReportRecordItemControl)
	INTERFACE_PART(CXTPReportRecordItemButton, XTPDIID_IReportItemButton, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRecordItemButton, XTPDIID_IReportItemButton)

//////////////////////////////////////////////////////////////////////////
BEGIN_DISPATCH_MAP(CXTPReportRecordItemControls, CXTPCmdTarget)
	DISP_FUNCTION_ID(CXTPReportRecordItemControls, "Count", 1, OleGetItemCount, VT_I4, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRecordItemControls, "RemoveAll", 2, RemoveAll, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRecordItemControls, "RemoveAt", 3, RemoveAt, VT_EMPTY, VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemControls, "AddButton", 4, OleAddButton, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemControls, "Item", DISPID_VALUE, OleGetItem, VT_DISPATCH, VTS_I4)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRecordItemControls, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportRecordItemControls, XTPDIID_IReportItemControls, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRecordItemControls, XTPDIID_IReportItemControls)
IMPLEMENT_ENUM_VARIANT(CXTPReportRecordItemControls)


LPDISPATCH CXTPReportRecordItemControls::OleAddButton(int nIndex)
{
	CXTPReportRecordItemControl* pItemControl = AddControl(xtpItemControlTypeButton, nIndex);
	if (!pItemControl)
		return NULL;
	return pItemControl->GetIDispatch(TRUE);

	//CXTPReportRecordItemButton* pItemButton = DYNAMIC_DOWNCAST(CXTPReportRecordItemButton, pItemControl);
	//return pItemButton ? pItemButton->GetIDispatch(TRUE) : NULL;
}

LPDISPATCH CXTPReportRecordItemControls::OleGetItem(long nIndex)
{
	CXTPReportRecordItemControl* pItemControl = GetAt(nIndex);

	if (pItemControl)
		return pItemControl->GetIDispatch(TRUE);

//  if (!pItemControl || pItemControl->GetType() != xtpItemControlTypeButton)
//      return NULL;

	//CXTPReportRecordItemButton* pItemButton = DYNAMIC_DOWNCAST(CXTPReportRecordItemButton, pItemControl);
	//return pItemButton ? pItemButton->GetIDispatch(TRUE) : NULL;

	return NULL;
}

#endif
