// XTPListCtrlView.cpp : implementation of the CXTPListView and CXTPListCtrl class.
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

#include "Controls/Util/XTPControlTheme.h"
#include "Controls/Header/XTPHeaderCtrl.h"
#include "Controls/List/XTPListBase.h"
#include "Controls/List/XTPListCtrlView.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTPListView
/////////////////////////////////////////////////////////////////////////////
CXTPListView::CXTPListView()
{
	ImplAttach(&GetListCtrl());
}

CXTPListView::~CXTPListView()
{

}

IMPLEMENT_DYNCREATE(CXTPListView, CListView)

BEGIN_MESSAGE_MAP(CXTPListView, CListView)
	//{{AFX_MSG_MAP(CXTPListView)
	ON_LISTCTRL_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CXTPListCtrl
/////////////////////////////////////////////////////////////////////////////

CXTPListCtrl::CXTPListCtrl()
{
	ImplAttach(this);
}

CXTPListCtrl::~CXTPListCtrl()
{
}


IMPLEMENT_DYNAMIC(CXTPListCtrl, CListCtrl)

BEGIN_MESSAGE_MAP(CXTPListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CXTPListCtrl)
	ON_LISTCTRL_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
