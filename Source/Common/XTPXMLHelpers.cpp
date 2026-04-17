// XTPXMLHelpers.cpp
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

#include "Common/XTPCasting.h"
#include "Common/XTPFramework.h"
#include "Common/XTPVC80Helpers.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPResourceManager.h"
#include "Common/XTPXMLHelpers.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString AFX_CDECL XTPEscapeXmlString(LPCTSTR lpString, LPCTSTR lpDefaultValue /*= _T("")*/)
{
	ASSERT(NULL != lpString);

	CXTPComInitializer initCOM;

	CString strEscapedString;
	BOOL bSuccess = FALSE;

	XTPXML::IXMLDOMDocumentPtr xmlDocument;
	if (SUCCEEDED(xmlDocument.CreateInstance(CLSID_XTPDOMDocument)))
	{
		xmlDocument->put_async(VARIANT_FALSE);
		xmlDocument->put_preserveWhiteSpace(-1 /*VARIANT_TRUE*/); // produces warning in VC6

		if (VARIANT_FALSE != xmlDocument->loadXML(L"<X/>"))
		{
			xmlDocument->documentElement->text = lpString;
			_bstr_t xml = xmlDocument->documentElement->xml;
			LPCWSTR lpFrom = xml.operator LPCWSTR() + (sizeof("<X>") - 1);
			LPCWSTR lpTo = xml.operator LPCWSTR() + xml.length() - (sizeof("</X>") - 1);

			ASSERT(lpFrom <= lpTo);
			ASSERT(0 == wcsncmp(lpTo, L"</X>", sizeof("</X>") - 1));

			SIZE_T cch = lpTo - lpFrom;
			WCHAR* pWBuffer = new WCHAR[cch + 1];
			memcpy(pWBuffer, lpFrom, cch * sizeof(WCHAR));
			pWBuffer[cch] = L'\0';
			strEscapedString = XTP_CW2CT(pWBuffer);
			delete[] pWBuffer;

			bSuccess = TRUE;
		}
	}

	if (!bSuccess)
	{
		strEscapedString = (NULL != lpDefaultValue ? lpDefaultValue : _T(""));
	}

	return strEscapedString;
}
