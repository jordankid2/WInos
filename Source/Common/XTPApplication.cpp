// XTPApplication.cpp: implementation of the CXTPApplication class.
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

#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// This is to enforce CXTPApplication instance to be constructed 
// prior to any consumer object.
static const CXTPApplication* gpApplicationInstance = XTPGetApplication();

///////////////////////////////////////////////////////////////////////////////
// CXTPApplication

IMPLEMENT_DYNAMIC(CXTPApplication, CObject);

CXTPApplication::CXTPApplication()
	: m_pModuleState(AfxGetModuleState())
	, m_bShutdown(FALSE)
{
}

CXTPApplication::~CXTPApplication()
{
	// It's important to call XTPShutdown, not just Shutdown.
	XTPShutdown();
}

AFX_MODULE_STATE* CXTPApplication::GetModuleState()
{
	return m_pModuleState;
}

CXTPApplication& AFX_CDECL CXTPApplication::GetInstance()
{
	static CXTPApplication app;

	ASSERT("XTP application has been already shutdown" && !app.m_bShutdown);
	if (app.m_bShutdown)
	{
		AfxThrowOleException(E_FAIL);
	}

	return app;
}

void CXTPApplication::Shutdown()
{
	if (!m_bShutdown)
	{
		TRACE0("Codejock XTP application shutting down ...\n");
		NotifyReversed(&IXTPApplicationEvents::OnBeforeApplicationShutdown);
		NotifyReversed(&IXTPApplicationEvents::OnApplicationShutdown);
		TRACE0("Codejock XTP application shutting down complete.\n");

		m_bShutdown = TRUE;
	}
}

BOOL AFX_CDECL XTPInitialize()
{
	XTPGetApplication();
	return TRUE;
}

CXTPApplication* AFX_CDECL XTPGetApplication()
{
	return &CXTPApplication::GetInstance();
}

void AFX_CDECL XTPShutdown()
{
	// Perform application shutting down just once.
	static CXTPApplication* pApp = XTPGetApplication();
	if (NULL != pApp)
	{
		XTPGetApplication()->Shutdown();
		pApp = NULL;
		gpApplicationInstance = NULL;
	}
}
