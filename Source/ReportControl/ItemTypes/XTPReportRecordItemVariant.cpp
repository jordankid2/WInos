// XTPReportRecordItemVariant.cpp : implementation of the XTPReportRecordItemVariant class.
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
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPPropExchange.h"
#include "Common/XTPCustomHeap.h"
#include "Common/XTPSmartPtrInternalT.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportControl.h"
#include "ReportControl/XTPReportRecordItemConstraint.h"
#include "ReportControl/XTPReportRow.h"
#include "ReportControl/XTPReportGroupRow.h"
#include "ReportControl/XTPReportRecord.h"
#include "ReportControl/XTPReportRecords.h"
#include "ReportControl/XTPReportColumn.h"
#include "ReportControl/ItemTypes/XTPReportRecordItemVariant.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int CXTPReportRecordItemVariant::m_nSortLocale = LOCALE_USER_DEFAULT;

//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemVariant

IMPLEMENT_SERIAL(CXTPReportRecordItemVariant, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPReportRecordItemVariant::CXTPReportRecordItemVariant(const VARIANT& lpValue)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
#endif

	m_oleValue = lpValue;
	m_oleValue.ChangeType((VARTYPE)(m_oleValue.vt & ~VT_BYREF));
}

void CXTPReportRecordItemVariant::SetValue(const COleVariant& var)
{
	m_oleValue = var;
}

void CXTPReportRecordItemVariant::SetValue(const VARIANT& var)
{
	m_oleValue = var;
}

void CXTPReportRecordItemVariant::SetValue(const VARIANT* var)
{
	m_oleValue = var;
}

COleVariant CXTPReportRecordItemVariant::GetValue()
{
	return m_oleValue;
}

CString CXTPReportRecordItemVariant::GetCaption(CXTPReportColumn* pColumn)
{
	if (IsChildOfMerge())
		return GetMergeItem()->GetCaption(pColumn);

	if (!m_strCaption.IsEmpty())
		return m_strCaption;

	COleVariant var(m_oleValue);

	CXTPReportRecordItemEditOptions* pEditOptions = NULL;
	if (m_pEditOptions)
		pEditOptions = m_pEditOptions;
	else if (pColumn)
		pEditOptions = pColumn->GetEditOptions();

	BOOL bConstraintEdit = FALSE;
	if (pEditOptions)
		bConstraintEdit = pEditOptions->m_bConstraintEdit;

	TRY
	{
		if (var.vt == VT_DATE && !bConstraintEdit && m_strFormatString != _T("%s"))
		{
			COleDateTime dt(var);
			return CXTPReportControlLocale::FormatDateTime(dt, m_strFormatString);
		}
		if (var.vt == VT_NULL)
		{
			var.vt = VT_I4;
			var.lVal = 0;
		}
		else
		{
			if (bConstraintEdit)
				CXTPReportControlLocale::VariantChangeTypeEx(var, VT_I4);
			else
				CXTPReportControlLocale::VariantChangeTypeEx(var, VT_BSTR);
		}
	}
	CATCH_ALL(e)
	{
	}
	END_CATCH_ALL

	if (bConstraintEdit)
	{
		CXTPReportRecordItemConstraint* pConstraint = pEditOptions ? pEditOptions->FindConstraint(var.lVal) : NULL;
		return pConstraint ? pConstraint->m_strConstraint : _T("");
	}

	if (var.vt != VT_BSTR) //this function expected VT_BSTR only?
		return _T("");

	CString strVariant(var.bstrVal);

	if (m_strFormatString == _T("%s"))
		return strVariant;

	CString strCaption;
	strCaption.Format(m_strFormatString, (LPCTSTR)strVariant);
	return strCaption;
}

COleVariant CXTPReportRecordItemVariant::GetGroupCaptionValue(CXTPReportColumn* pColumn)
{
	UNREFERENCED_PARAMETER(pColumn);
	return m_oleValue;
}

int CXTPReportRecordItemVariant::CompareGroupCaption(CXTPReportColumn* pColumn, CXTPReportGroupRow* pGroupRow)
{
	ASSERT_VALID(pGroupRow);
	UNREFERENCED_PARAMETER(pColumn);

	if (!m_strGroupCaption.IsEmpty())
		return m_pRecord->GetRecords()->Compare(m_strGroupCaption, pGroupRow->GetCaption());

	return CompareValues(m_oleValue, pGroupRow->GetCaptionValue(), m_pRecord->GetRecords()->IsCaseSensitive());
}

int CXTPReportRecordItemVariant::Compare(CXTPReportColumn* pColumn, CXTPReportRecordItem* pItem)
{
	ASSERT_VALID(pItem);
	UNREFERENCED_PARAMETER(pColumn);

	if (GetSortPriority() != -1 || pItem->GetSortPriority() != -1)
		return GetSortPriority() - pItem->GetSortPriority();

	CXTPReportRecordItemVariant* pItemVariant = DYNAMIC_DOWNCAST(CXTPReportRecordItemVariant, pItem);
	if (!pItemVariant)
		return 0;

	return CompareValues(m_oleValue, pItemVariant->m_oleValue, m_pRecord->GetRecords()->IsCaseSensitive());
}

BOOL CXTPReportRecordItemVariant::OnValueChanging(XTP_REPORTRECORDITEM_ARGS* pItemArgs, LPVARIANT lpNewValue)
{
#ifdef _XTP_ACTIVEX
	XTP_NM_REPORTVALUECHANGING nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	nmParams.pColumn = pItemArgs->pColumn;
	nmParams.pItem = this;
	nmParams.pRow = pItemArgs->pRow;
	nmParams.bCancel = FALSE;
	nmParams.lpNewValue = lpNewValue;

	pItemArgs->pControl->SendNotifyMessage(
		XTP_NM_REPORT_VALUECHANGING, (NMHDR*)&nmParams);

	return !nmParams.bCancel;
#else
	UNREFERENCED_PARAMETER(pItemArgs);
	UNREFERENCED_PARAMETER(lpNewValue);
	return TRUE;
#endif

}

BOOL CXTPReportRecordItemVariant::OnEditChanging(XTP_REPORTRECORDITEM_ARGS* pItemArgs, CString& rstrNewText)
{
#ifdef _XTP_ACTIVEX
	XTP_NM_REPORTVALUECHANGING nmParams;
	::ZeroMemory(&nmParams, sizeof(nmParams));

	COleVariant olevarText = rstrNewText;
	ASSERT(olevarText.vt == VT_BSTR);

	nmParams.pColumn = pItemArgs->pColumn;
	nmParams.pItem = this;
	nmParams.pRow = pItemArgs->pRow;
	nmParams.bCancel = FALSE;
	nmParams.lpNewValue = &olevarText;

	pItemArgs->pControl->SendNotifyMessage(XTP_NM_REPORT_EDIT_CHANGING, (NMHDR*)&nmParams);

	if (!nmParams.bCancel)
	{
		BOOL bConvert = CXTPReportControlLocale::VariantChangeTypeEx(olevarText, VT_BSTR, FALSE);
		if (bConvert && olevarText.vt == VT_BSTR)
		{
			rstrNewText = olevarText.bstrVal;
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	return !nmParams.bCancel;
#else
	UNREFERENCED_PARAMETER(pItemArgs);
	UNREFERENCED_PARAMETER(rstrNewText);
	return TRUE;
#endif
}

void CXTPReportRecordItemVariant::OnEditChanged(XTP_REPORTRECORDITEM_ARGS* pItemArgs,
												LPCTSTR szText)
{
	COleVariant var(szText);

	if (OnValueChanging(pItemArgs, &var))
	{
		TRY
		{
			CXTPReportControlLocale::VariantChangeTypeEx(var, m_oleValue.vt == VT_NULL ? (VARTYPE)VT_BSTR : m_oleValue.vt);
		}
		CATCH_ALL(e)
		{
			return;
		}
		END_CATCH_ALL

		m_oleValue = var;
	}
}

void CXTPReportRecordItemVariant::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPReportRecordItem::DoPropExchange(pPX);

	COleVariant varDefault(_T(""));
	PX_Variant(pPX, _T("Value"), m_oleValue, varDefault);
}

void CXTPReportRecordItemVariant::OnConstraintChanged(XTP_REPORTRECORDITEM_ARGS* pItemArgs, CXTPReportRecordItemConstraint* pConstraint)
{
	ASSERT_VALID(pConstraint);
	ASSERT(NULL != pItemArgs);

	BOOL bChooseOnly = GetEditOptions(pItemArgs->pColumn)->m_bConstraintEdit;
	if (bChooseOnly)
	{
		long index = (long) pConstraint->m_dwData;
		COleVariant var;
		var.vt = VT_I4;
		var.lVal = index;

		if (m_oleValue.vt == VT_NULL)
			m_oleValue.vt = VT_I4;

		BOOL bChanged = CXTPReportControlLocale::VariantChangeTypeEx(var, m_oleValue.vt, FALSE);

		if (bChanged && OnValueChanging(pItemArgs, &var))
		{
			m_oleValue = var;
		}
	}
	else
	{
		OnEditChanged(pItemArgs, pConstraint->m_strConstraint);
	}
}

DWORD CXTPReportRecordItemVariant::GetSelectedConstraintData(XTP_REPORTRECORDITEM_ARGS* pItemArgs)
{
	ASSERT(NULL != pItemArgs);

	if (GetEditOptions(pItemArgs->pColumn)->m_bConstraintEdit)
	{
		COleVariant var(m_oleValue);
		TRY
		{
			CXTPReportControlLocale::VariantChangeTypeEx(var, VT_I4);
		}
		CATCH_ALL(e)
		{
			return (DWORD)-1;
		}
		END_CATCH_ALL

		return var.lVal;

	}
	else
	{
		return (DWORD)-1;
	}
}

int CXTPReportRecordItemVariant::CompareValues(const COleVariant& vt1,
	const COleVariant& vt2, BOOL bCaseSensitive)
{
	ULONG dwFlags = (bCaseSensitive ? 0 : NORM_IGNORECASE);

	LCID lcidnSortLocale = m_nSortLocale;
	if (lcidnSortLocale == LOCALE_USER_DEFAULT)
	{
		lcidnSortLocale = CXTPReportControlLocale::GetActiveLCID();
	}

	return VarCmp(&const_cast<COleVariant&>(vt1), &const_cast<COleVariant&>(vt2),
		lcidnSortLocale, dwFlags) - VARCMP_EQ;
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportRecordItemVariant, CXTPReportRecordItem)
	DISP_PROPERTY_ID(CXTPReportRecordItemVariant, "Value", 3, m_oleValue, VT_VARIANT)
END_DISPATCH_MAP()

#endif
