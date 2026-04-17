// XTPReportHitTestInfo.cpp : implementation of the CXTPReportHitTestInfo class.
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
#include "Common/XTPCustomHeap.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportHitTestInfo.h"
#include "ReportControl/XTPReportRow.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportColumn.h"
#include "TrackControl/XTPTrackBlock.h"

#include "ReportControl/XTPReportControlIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CXTPReportHitTestInfo, CXTPCmdTarget)

CXTPReportHitTestInfo::CXTPReportHitTestInfo()
	: m_htCode (xtpReportHitTestUnknown)
	, m_pColumn(NULL)
	, m_pRow   (NULL)
	, m_pItem  (NULL)
	, m_pBlock (NULL)
	, m_iTrackPosition(0)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportHitTestInfo::~CXTPReportHitTestInfo()
{
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportHitTestInfo, CXTPCmdTarget)
	//{{AFX_DISPATCH_MAP(CReportHitTestInfo)
	//}}AFX_DISPATCH_MAP
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "Column",        1, OleGetColumn,        SetNotSupported, VT_DISPATCH)
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "Row",           2, OleGetRow,           SetNotSupported, VT_DISPATCH)
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "Item",          3, OleGetItem,          SetNotSupported, VT_DISPATCH)
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "ht",            4, OleGetHt,            SetNotSupported, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "Block",         5, OleGetBlock,         SetNotSupported, VT_DISPATCH)
	DISP_PROPERTY_EX_ID(CXTPReportHitTestInfo, "TrackPosition", 6, OleGetTrackPosition, SetNotSupported, VT_I4)
END_DISPATCH_MAP()

// Note: we add support for DIID_IReportHitTestInfo to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .ODL file.

BEGIN_INTERFACE_MAP(CXTPReportHitTestInfo, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportHitTestInfo, XTPDIID_IReportHitTestInfo, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportHitTestInfo, XTPDIID_IReportHitTestInfo)
/////////////////////////////////////////////////////////////////////////////
// CReportHitTestInfo message handlers

LPDISPATCH CXTPReportHitTestInfo::OleGetColumn()
{
	return m_pColumn? m_pColumn->GetIDispatch(TRUE): NULL;
}

LPDISPATCH CXTPReportHitTestInfo::OleGetRow()
{
	return m_pRow? m_pRow->GetIDispatch(TRUE): NULL;
}

LPDISPATCH CXTPReportHitTestInfo::OleGetItem()
{
	return m_pItem? m_pItem->GetIDispatch(TRUE): NULL;
}

LPDISPATCH CXTPReportHitTestInfo::OleGetBlock()
{
	return m_pBlock? m_pBlock->GetIDispatch(TRUE): NULL;
}

long CXTPReportHitTestInfo::OleGetTrackPosition()
{
	return m_iTrackPosition;
}

long CXTPReportHitTestInfo::OleGetHt()
{
	return m_htCode;
}

#endif
