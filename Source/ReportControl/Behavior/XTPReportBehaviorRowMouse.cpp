// XTPReportBehaviorRowMouse.cpp : implementation of the CXTPReportBehaviorRowMouse class.
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
#include "Common/XTPCustomHeap.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/Behavior/XTPReportBehavior.h"
#include "ReportControl/XTPReportControlIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


CXTPReportBehaviorRowMouse::CXTPReportBehaviorRowMouse(
	XTPReportMouseEvent event)

	: Left (NULL)
	, Right(NULL)
{
	Left  = new CXTPReportBehaviorRowMouseButton(xtpReportMouseButtonLeft,  event);
	Right = new CXTPReportBehaviorRowMouseButton(xtpReportMouseButtonRight, event);

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPReportBehaviorRowMouse::~CXTPReportBehaviorRowMouse()
{
	SAFE_DELETE(Left);
	SAFE_DELETE(Right);
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportBehaviorRowMouse, CXTPCmdTarget)
	DISP_FUNCTION_ID(CXTPReportBehaviorRowMouse, "LeftButton",  1, OleGetLeft,  VT_DISPATCH, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportBehaviorRowMouse, "RightButton", 2, OleGetRight, VT_DISPATCH, VTS_NONE)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportBehaviorRowMouse, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportBehaviorRowMouse, XTPDIID_IReportBehaviorRowMouse, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportBehaviorRowMouse, XTPDIID_IReportBehaviorRowMouse)


LPDISPATCH CXTPReportBehaviorRowMouse::OleGetLeft()
{
	return XTPGetDispatch(Left);
}

LPDISPATCH CXTPReportBehaviorRowMouse::OleGetRight()
{
	return XTPGetDispatch(Right);
}

#endif // _XTP_ACTIVEX
