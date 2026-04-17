// XTPReportRecordItemConstraint.cpp : implementation of the CXTPReportRecordItemConstraint class.
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

#include "ReportControl/XTPReportRecordItemConstraint.h"
#include "ReportControl/XTPReportControlIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemConstraint

IMPLEMENT_DYNAMIC(CXTPReportRecordItemConstraint, CXTPCmdTarget);

CXTPReportRecordItemConstraint::CXTPReportRecordItemConstraint()
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif

	m_dwData = 0;
	m_nIndex = 0;
}

int CXTPReportRecordItemConstraint::GetIndex() const
{
	return m_nIndex;
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemConstraints

CXTPReportRecordItemConstraints::CXTPReportRecordItemConstraints()
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportRecordItemConstraints::~CXTPReportRecordItemConstraints()
{
	RemoveAll();
}


int CXTPReportRecordItemConstraints::GetCount() const
{
	return (int)m_arrConstraints.GetSize();
}

CXTPReportRecordItemConstraint* CXTPReportRecordItemConstraints::GetAt(int nIndex) const
{
	return m_arrConstraints.GetAt(nIndex);
}

void CXTPReportRecordItemConstraints::RemoveAll()
{
	for (int i = 0; i < GetCount(); i++)
	{
		if (m_arrConstraints[i] != NULL)
			m_arrConstraints[i]->InternalRelease();
	}
	m_arrConstraints.RemoveAll();
}

#ifdef _XTP_ACTIVEX

//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemConstraint

BEGIN_DISPATCH_MAP(CXTPReportRecordItemConstraint, CXTPCmdTarget)
	DISP_PROPERTY_ID(CXTPReportRecordItemConstraint, "Caption", 1, m_strConstraint, VT_BSTR)
	DISP_PROPERTY_ID(CXTPReportRecordItemConstraint, "Data", 2, m_dwData, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportRecordItemConstraint, "Index", 3, GetIndex, SetNotSupported, VT_I4)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRecordItemConstraint, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportRecordItemConstraint, XTPDIID_IReportRecordItemConstraint, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRecordItemConstraint, XTPDIID_IReportRecordItemConstraint)

//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemConstraints

BEGIN_DISPATCH_MAP(CXTPReportRecordItemConstraints, CXTPCmdTarget)
	DISP_FUNCTION_ID(CXTPReportRecordItemConstraints, "Count", dispidCount, OleGetItemCount, VT_I4, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRecordItemConstraints, "Constraint", DISPID_VALUE, OleGetItem, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemConstraints, "_NewEnum", DISPID_NEWENUM, OleNewEnum, VT_UNKNOWN, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRecordItemConstraints, "Add", dispidAdd, OleAdd, VT_EMPTY, VTS_BSTR VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRecordItemConstraints, "DeleteAll", 5, RemoveAll, VT_EMPTY, VTS_NONE)

END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRecordItemConstraints, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportRecordItemConstraints, XTPDIID_IReportRecordItemConstraints, Dispatch)
	//INTERFACE_PART(CXTPReportRecordItemConstraints, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRecordItemConstraints, XTPDIID_IReportRecordItemConstraints)

int CXTPReportRecordItemConstraints::OleGetItemCount()
{
	return GetCount();
}

LPDISPATCH CXTPReportRecordItemConstraints::OleGetItem(long nIndex)
{
	if (nIndex >= 0 && nIndex < GetCount())
	{
		return GetAt(nIndex)->GetIDispatch(TRUE);
	}
	AfxThrowOleException(E_INVALIDARG);
	return 0;
}

IMPLEMENT_ENUM_VARIANT(CXTPReportRecordItemConstraints)

void CXTPReportRecordItemConstraints::OleAdd(LPCTSTR lpszCaption, long dwData)
{

	CXTPReportRecordItemConstraint* pConstaint = new CXTPReportRecordItemConstraint();
	pConstaint->m_strConstraint = lpszCaption;
	pConstaint->m_dwData = dwData;
	pConstaint->m_nIndex = GetCount();

	m_arrConstraints.Add(pConstaint);
}

#endif
