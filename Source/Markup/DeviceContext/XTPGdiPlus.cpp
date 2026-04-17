// XTPGdiPlus.cpp: implementation of the CXTPGdiPlus class.
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

#include "GraphicLibrary/GdiPlus/XTPGdiPlus.h"

using namespace Gdiplus;
using namespace Gdiplus::DllExports;

#include "Markup/DeviceContext/XTPGdiPlus.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CXTPGdiPlus::CXTPGdiPlus()
	: m_nSessionTag(0)
	, m_hModule(NULL)
	, m_nGdiplusToken(NULL)
	, m_nCount(0)
{
}

CXTPGdiPlus::~CXTPGdiPlus()
{
}

void CXTPGdiPlus::Register(BOOL bInit)
{
	static GdiplusStartupOutput gdiplusStartupOutput;
	if (bInit)
	{
		++m_nSessionTag;
		++m_nCount;

		if (m_nCount > 1)
			return;

		ASSERT(m_nGdiplusToken == 0 && m_hModule == 0);

		m_hModule = LoadLibrary(_T("GdiPlus.dll"));

		if (m_hModule)
		{
			GdiplusStartupInput gdiplusStartupInput;
			gdiplusStartupInput.SuppressBackgroundThread = TRUE;
			GdiplusStartup(&m_nGdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
			gdiplusStartupOutput.NotificationHook(&m_nGdiplusToken);
		}
	}
	else
	{
		m_nCount--;

		if (m_nCount != 0)
			return;

		if (m_hModule)
		{
			// Termination of background thread, which is causing the shutdown problem.
			gdiplusStartupOutput.NotificationUnhook(m_nGdiplusToken);
			GdiplusShutdown(m_nGdiplusToken);
			FreeLibrary(m_hModule);
		}

		m_hModule = NULL;
		m_nGdiplusToken = 0;
	}
}
