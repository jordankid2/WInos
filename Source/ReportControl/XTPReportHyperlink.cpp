// XTPReportHyperlink.cpp : implementation of the CXTPReportHyperlink class.
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
#include "Common/XTPPropExchange.h"
#include "Common/PropExchange/XTPPropExchangeEnumerator.h"
#include "Common/PropExchange/XTPPropExchangeEnumeratorPtr.h"
#include "Common/PropExchange/XTPPropExchangeSection.h"
#include "Common/XTPSmartPtrInternalT.h"
#include "Common/XTPCustomHeap.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportHyperlink.h"
#include "ReportControl/XTPReportControlIIDs.h"

#pragma warning(disable: 4097) // TBase' used as synonym for class-name

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CXTPReportHyperlinkStyle::CXTPReportHyperlinkStyle(COLORREF color, XTPReportTextDecoration textDecoration)
	: m_color(color)
	, m_textDecoration(textDecoration)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

COLORREF CXTPReportHyperlinkStyle::GetColor() const
{
	return m_color;
}

void CXTPReportHyperlinkStyle::SetColor(COLORREF color)
{
	m_color = color;
}

XTPReportTextDecoration CXTPReportHyperlinkStyle::GetTextDecoration() const
{
	return m_textDecoration;
}

void CXTPReportHyperlinkStyle::SetTextDecoration(XTPReportTextDecoration textDecoration)
{
	m_textDecoration = textDecoration;
}


IMPLEMENT_DYNAMIC(CXTPReportHyperlink, CXTPCmdTarget)
IMPLEMENT_DYNAMIC(CXTPReportHyperlinks, CXTPCmdTarget)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXTPReportHyperlink::CXTPReportHyperlink(int nHyperTextBegin, int nHyperTextLen)
	: m_nHyperTextBegin(nHyperTextBegin),
	m_nHyperTextLen(nHyperTextLen),
	m_rcHyperSpot(0, 0, 0, 0)
{

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportHyperlink::~CXTPReportHyperlink()
{

}

void CXTPReportHyperlink::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Int(pPX, _T("HyperTextBegin"), m_nHyperTextBegin);
	PX_Int(pPX, _T("HyperTextLen"), m_nHyperTextLen);
}

CXTPReportHyperlinks::CXTPReportHyperlinks()
{

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportHyperlinks::~CXTPReportHyperlinks()
{
	RemoveAll();
}

void CXTPReportHyperlinks::RemoveAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	CXTPReportHyperlink* pItem = GetAt(nIndex);
	if (pItem)
		pItem->InternalRelease();

	TBase::RemoveAt(nIndex);
}

void CXTPReportHyperlinks::RemoveAll()
{
	for (int nItem = (int)GetSize() - 1; nItem >= 0; nItem--)
	{
		CXTPReportHyperlink* pItem = GetAt(nItem);
		if (pItem)
			pItem->InternalRelease();
	}
	TBase::RemoveAll();
}

void CXTPReportHyperlinks::DoPropExchange(CXTPPropExchange* pPX)
{
	int nCount = (int)GetSize();

	CXTPPropExchangeEnumeratorPtr pEnumItems(pPX->GetEnumerator(_T("Hyperlink")));

	if (pPX->IsStoring())
	{
		POSITION posItem = pEnumItems->GetPosition((DWORD)nCount);

		for (int i = 0; i < nCount; i++)
		{
			CXTPReportHyperlink* pItem = GetAt(i);
			ASSERT(pItem);
			if (pItem)
			{
				CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
				pItem->DoPropExchange(&secItem);
			}
		}
	}
	else
	{
		POSITION posItem = pEnumItems->GetPosition();
		while (posItem)
		{
			CXTPReportHyperlink* pItem = new CXTPReportHyperlink();
			if (!pItem)
			{
				return;
			}
			CXTPPropExchangeSection secItem(pEnumItems->GetNext(posItem));
			pItem->DoPropExchange(&secItem);

			Add(pItem);
		}
	}

}

void CXTPReportHyperlinks::CopyFrom(CXTPReportHyperlinks* pSrc)
{
	if (pSrc == this)
		return;

	RemoveAll();

	if (!pSrc)
		return;

	int nCount = pSrc->GetSize();
	for (int i = 0; i < nCount; i++)
	{
		CXTPReportHyperlink* pHlnk = pSrc->GetAt(i);
		if (pHlnk)
		{
			pHlnk->InternalAddRef();
			Add(pHlnk);
		}
	}
}

#ifdef _XTP_ACTIVEX

BEGIN_INTERFACE_MAP(CXTPReportHyperlink, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportHyperlink, XTPDIID_IReportHyperlink, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportHyperlink, XTPDIID_IReportHyperlink)

BEGIN_DISPATCH_MAP(CXTPReportHyperlink, CXTPCmdTarget)
	DISP_PROPERTY_ID(CXTPReportHyperlink, "HyperTextBegin", 1, m_nHyperTextBegin,   VT_I4)
	DISP_PROPERTY_ID(CXTPReportHyperlink, "HyperTextLen",   2, m_nHyperTextLen,     VT_I4)
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////

BEGIN_INTERFACE_MAP(CXTPReportHyperlinks, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportHyperlinks, XTPDIID_IReportHyperlinks, Dispatch)
	//INTERFACE_PART(CXTPReportHyperlinks, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportHyperlinks, XTPDIID_IReportHyperlinks)
IMPLEMENT_ENUM_VARIANT(CXTPReportHyperlinks)

BEGIN_DISPATCH_MAP(CXTPReportHyperlinks, CXTPCmdTarget)
	DISP_FUNCTION_ID(CXTPReportHyperlinks, "Count", 1, OleGetItemCount, VT_I4, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportHyperlinks, "Item", DISPID_VALUE, OleGetItem, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION_ID(CXTPReportHyperlinks, "_NewEnum", DISPID_NEWENUM, OleNewEnum, VT_UNKNOWN, VTS_NONE)

	DISP_FUNCTION_ID(CXTPReportHyperlinks, "AddHyperlink",  2, OleAddHyperlink, VT_DISPATCH, VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CXTPReportHyperlinks, "Remove",        3, OleRemove,       VT_EMPTY,    VTS_I4)
	DISP_FUNCTION_ID(CXTPReportHyperlinks, "RemoveAll",     4, OleRemoveAll,    VT_EMPTY,    VTS_NONE)
END_DISPATCH_MAP()

LPDISPATCH CXTPReportHyperlinks::OleGetItem(long nIndex)
{
	if (nIndex >= 0 && nIndex < GetSize())
	{
		CXTPReportHyperlink* pItem = GetAt(nIndex);
		return pItem ? pItem->GetIDispatch(TRUE) : NULL;
	}
	AfxThrowOleException(DISP_E_BADINDEX);
	return NULL;
}

LPDISPATCH CXTPReportHyperlinks::OleAddHyperlink(long nHyperTextBegin, long nHyperTextLen)
{
	CXTPReportHyperlink* pItem = new CXTPReportHyperlink(nHyperTextBegin, nHyperTextLen);
	if (!pItem)
		return NULL;

	Add(pItem);
	return pItem->GetIDispatch(TRUE);
}

#endif
