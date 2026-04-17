// XTPReportRecordItemText.cpp : implementation of the CXTPReportRecordItemText class.
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
#include "Common/XTPPropExchange.h"
#include "Common/XTPCustomHeap.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/ItemTypes/XTPReportRecordItemText.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////////
// CXTPReportRecordItemText

IMPLEMENT_SERIAL(CXTPReportRecordItemText, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPReportRecordItemText::CXTPReportRecordItemText(LPCTSTR szText)
	: CXTPReportRecordItem()
{
	SetValue(szText);
}

void CXTPReportRecordItemText::SetValue(LPCTSTR szText)
{
	m_strText = szText;
	ParseBBCode(m_strText);
}

CString CXTPReportRecordItemText::GetCaption(CXTPReportColumn *pColumn)
{
	if (IsChildOfMerge())
		return GetMergeItem()->GetCaption(pColumn);

	if (!m_strCaption.IsEmpty())
		return m_strCaption;

	if (m_strFormatString == _T("%s"))
		return m_strText;

	CString strCaption;
	strCaption.Format(m_strFormatString, (LPCTSTR)m_strText);
	return strCaption;
}

void CXTPReportRecordItemText::OnEditChanged(XTP_REPORTRECORDITEM_ARGS* pItemArgs, LPCTSTR szText)
{
	SetValue(szText);

	CXTPReportRecordItem::OnEditChanged(pItemArgs, szText);
}

void CXTPReportRecordItemText::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPReportRecordItem::DoPropExchange(pPX);

	PX_String(pPX, _T("Text"), m_strText, _T(""));
}
