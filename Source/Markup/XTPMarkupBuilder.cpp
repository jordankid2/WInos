// XTPMarkupBuilder.cpp: implementation of the CXTPMarkupBuilder class.
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

#include "Common/Base/Diagnostic/XTPDisableAdvancedWarnings.h"
#include <ActivScp.h>
#include "Common/Base/Diagnostic/XTPEnableAdvancedWarnings.h"

#include "GraphicLibrary/GdiPlus/XTPGdiPlus.h"

#include "Common/XTPFramework.h"
#include "Common/XTPVc80Helpers.h"
#include "Common/XTPSystemHelpers.h"
#include "Common/XTPSynchro.h"
#include "Common/XTPApplication.h"
#include "Common/XTPSingleton.h"
#include "Common/XTPGdiObjects.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPXMLHelpers.h"

#include "Markup/XTPMarkupTools.h"
#include "Markup/XTPMarkupBuilder.h"
#include "Markup/XTPMarkupParser.h"
#include "Markup/XTPMarkupObject.h"
#include "Markup/XTPMarkupStyle.h"
#include "Markup/XTPMarkupString.h"
#include "Markup/XTPMarkupInputElement.h"
#include "Markup/XTPMarkupUIElement.h"
#include "Markup/XTPMarkupFrameworkElement.h"
#include "Markup/Extensions/XTPMarkupExtension.h"

#include "Markup/XTPMarkupDrawingContext.h"
#include "Markup/XTPMarkupContext.h"
#include "Markup/Controls/XTPMarkupControl.h"
#include "Markup/Controls/XTPMarkupContentControl.h"
#include "Markup/Controls/XTPMarkupTextBlock.h"
#include "Markup/Controls/XTPMarkupScrollViewer.h"
#include "Markup/XTPMarkupFrameworkContentElement.h"
#include "Markup/Text/XTPMarkupTextElement.h"
#include "Markup/Text/XTPMarkupInline.h"
#include "Markup/Text/XTPMarkupInlineCollection.h"
#include "Markup/Text/XTPMarkupRun.h"
#include "Markup/XTPMarkupResourceDictionary.h"
#include "Markup/Shapes/XTPMarkupShape.h"
#include "Markup/Extensions/XTPMarkupStaticExtension.h"
#include "Markup/Extensions/XTPMarkupSystemColorsStaticExtension.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CXTPMarkupObjectProperty : public CXTPMarkupObject
{
	DECLARE_MARKUPCLASS(CXTPMarkupObjectProperty)
public:
	CXTPMarkupObjectProperty(CXTPMarkupDependencyProperty* pProperty = 0)
	{
		m_pProperty = pProperty;
		m_pContent = NULL;
		m_pCollection = NULL;
	}

	~CXTPMarkupObjectProperty()
	{
		MARKUP_RELEASE(m_pContent);
		MARKUP_RELEASE(m_pCollection);
	}

	virtual void SetContentObject(CXTPMarkupBuilder* pBuilder, CXTPMarkupObject* pContent)
	{
		CXTPMarkupType* pContentType = m_pProperty->GetPropetyType();

		if (pContent->IsKindOf(pContentType))
		{
			ASSERT(m_pContent == NULL);
			MARKUP_RELEASE(m_pContent);
			m_pContent = pContent;
			MARKUP_ADDREF(m_pContent);
		}
		else if (IsStringObject(pContent))
		{
			ASSERT(m_pContent == NULL);
			MARKUP_RELEASE(m_pContent);
			m_pContent = pContent;
			MARKUP_ADDREF(m_pContent);
		}
		else
		{
			if (m_pContent == NULL)
			{
				m_pCollection = pBuilder->GetMarkupContext()->CreateMarkupObject(m_pProperty->GetPropetyType());
				if (m_pCollection)
				{
					m_pCollection->SetAssociatedProperty(m_pProperty);
					m_pCollection->SetContentObject(pBuilder, pContent);
				}

				m_pContent = m_pCollection;
				m_pCollection = NULL;
			}
			else
			{
				m_pContent->SetContentObject(pBuilder, pContent);
			}
		}
	}

	CXTPMarkupObject* FindResource(const CXTPMarkupObject* pKey) const
	{
		if (m_pContent)
			return CXTPMarkupResourceDictionary::FindResource(m_pContent, pKey);

		return NULL;
	}

	CXTPMarkupObject* m_pContent;
	CXTPMarkupObject* m_pCollection;
	CXTPMarkupDependencyProperty* m_pProperty;
};

IMPLEMENT_MARKUPCLASS(NULL, CXTPMarkupObjectProperty, CXTPMarkupObject)

void CXTPMarkupObjectProperty::RegisterMarkupClass()
{

}


//////////////////////////////////////////////////////////////////////////
// CXTPMarkupBuilder::CStaticExtension

class CXTPMarkupBuilder::CStaticExtension
{
public:
	CXTPMarkupObject* ProvideValue(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue);
};

CXTPMarkupObject* CXTPMarkupBuilder::CStaticExtension::ProvideValue(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue)
{
	ASSERT(NULL != pBuilder);
	CXTPMarkupObject* pObject = pBuilder->GetMarkupContext()->GetExtensionRoot()->ProvideValue(lpszValue);

	if (NULL == pObject)
	{
		pBuilder->ThrowBuilderException(pBuilder->FormatString(_T("Cannot find the static extension '%ls'"), (LPCTSTR)lpszValue));
	}

	return pObject;
}



//////////////////////////////////////////////////////////////////////////
// CXTPMarkupBuilder::CStaticResourceExtension

class CXTPMarkupBuilder::CStaticResourceExtension
{
public:
	CStaticResourceExtension();
	~CStaticResourceExtension();
public:
	CXTPMarkupObject* ProvideValue(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue);
	CXTPMarkupObject* FindResources(CXTPMarkupObject* pElement, CXTPMarkupObject* pValue);
	CXTPMarkupObject* ResolveKey(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue);

};

CXTPMarkupBuilder::CStaticResourceExtension::CStaticResourceExtension()
{
}

CXTPMarkupBuilder::CStaticResourceExtension::~CStaticResourceExtension()
{

}

CXTPMarkupObject* CXTPMarkupBuilder::CStaticResourceExtension::FindResources(CXTPMarkupObject* pElement, CXTPMarkupObject* pKey)
{
	if (pElement->IsKindOf(MARKUP_TYPE(CXTPMarkupFrameworkElement)))
	{
		CXTPMarkupObject* pValue = ((CXTPMarkupFrameworkElement*)pElement)->FindResource(pKey);
		if (pValue)
			return pValue;
	}

	if (pElement->IsKindOf(MARKUP_TYPE(CXTPMarkupFrameworkContentElement)))
	{
		CXTPMarkupObject* pValue = ((CXTPMarkupFrameworkContentElement*)pElement)->FindResource(pKey);
		if (pValue)
			return pValue;
	}

	if (pElement->IsKindOf(MARKUP_TYPE(CXTPMarkupStyle)))
	{
		CXTPMarkupObject* pValue = ((CXTPMarkupStyle*)pElement)->FindResource(pKey);
		if (pValue)
			return pValue;
	}

	if (pElement->IsKindOf(MARKUP_TYPE(CXTPMarkupObjectProperty)))
	{
		CXTPMarkupObject* pValue = ((CXTPMarkupObjectProperty*)pElement)->FindResource(pKey);
		if (pValue)
			return pValue;
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::CStaticResourceExtension::ResolveKey(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue)
{
	while (*lpszValue == ' ' || *lpszValue == '\n' || *lpszValue == '\r')
		lpszValue++;

	if (lpszValue[0] == '{')
	{
		if (lpszValue[1] == '}')
		{
			lpszValue = lpszValue + 2;
		}
		else
		{
			return pBuilder->ResolveMarkupExtension(lpszValue + 1);
		}
	}

	return CXTPMarkupString::CreateValue(lpszValue);
}

CXTPMarkupObject* CXTPMarkupBuilder::CStaticResourceExtension::ProvideValue(CXTPMarkupBuilder* pBuilder, LPCWSTR lpszValue)
{
	CXTPMarkupObject* pKey = ResolveKey(pBuilder, lpszValue);
	if (!pKey)
		return NULL;

	POSITION pos = pBuilder->m_arrObjects.GetTailPosition();
	while (pos)
	{
		CXTPMarkupObject* pElement = pBuilder->m_arrObjects.GetPrev(pos);

		CXTPMarkupObject* pValue = FindResources(pElement, pKey);

		if (pValue)
		{
			MARKUP_RELEASE(pKey);
			MARKUP_ADDREF(pValue);
			return pValue;
		}
	}

	if (pKey->IsKindOf(MARKUP_TYPE(CXTPMarkupColorKey)))
	{
		CXTPMarkupObject* pValue = CXTPMarkupSystemColorStaticExtension::GetSystemColor(((CXTPMarkupColorKey*)pKey)->m_nIndex);
		return pValue;
	}

	if (pKey->IsKindOf(MARKUP_TYPE(CXTPMarkupBrushKey)))
	{
		CXTPMarkupObject* pValue = CXTPMarkupSystemBrushStaticExtension::GetSystemBrush(((CXTPMarkupBrushKey*)pKey)->m_nIndex);
		return pValue;
	}

	MARKUP_RELEASE(pKey);

	pBuilder->ThrowBuilderException(pBuilder->FormatString(_T("Cannot find resource named '%ls'. Resource names are case sensitive"), (LPCTSTR)lpszValue));

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CXTPMarkupBuilder

CXTPMarkupBuilder::CStaticExtension* CXTPMarkupBuilder::GetStaticExtension()
{
	if (m_pStaticExtension == NULL)
	{
		m_pStaticExtension = new CStaticExtension;
	}

	return m_pStaticExtension;
}

CXTPMarkupBuilder::CStaticResourceExtension* CXTPMarkupBuilder::GetStaticResourceExtension()
{
	if (m_pStaticResourceExtension == NULL)
	{
		m_pStaticResourceExtension = new CStaticResourceExtension();
	}

	return m_pStaticResourceExtension ;
}


//////////////////////////////////////////////////////////////////////////
// CXTPMarkupBuilderException

IMPLEMENT_DYNAMIC(CXTPMarkupBuilderException, CException)

CXTPMarkupBuilderException::CXTPMarkupBuilderException(LPCTSTR lpszError)
{
	m_bInitialized = TRUE;
	m_bLoaded = TRUE;

	m_szMessage[_countof(m_szMessage) - 1] = 0;
	STRNCPY_S(m_szMessage, _countof(m_szMessage), lpszError, _countof(m_szMessage) - 1);
}

BOOL CXTPMarkupBuilderException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
	if (!lpszError)
		return FALSE;

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	// if we didn't load our string (eg, we're a console app)
	lstrcpyn(lpszError, m_szMessage, nMaxError);

	return TRUE;
}

BOOL CXTPMarkupBuilderException::GetErrorMessage(LPTSTR lpszError, 
	UINT nMaxError, PUINT pnHelpContext) const
{
	return const_cast<CXTPMarkupBuilderException*>(this)->GetErrorMessage(lpszError,
		nMaxError, pnHelpContext);
}

//////////////////////////////////////////////////////////////////////////
// CStringBuilder

class CXTPMarkupBuilder::CStringBuilder
{
public:
	CStringBuilder();
	~CStringBuilder();

public:
	void Add(LPCWSTR lpszWord);
	void Empty();

	int GetLength() const
	{
		return m_nLength;
	}

	LPCWSTR GetBuffer()
	{
		ASSERT(m_lpszData);
		ASSERT(m_nLength < m_nAlloc);
		m_lpszData[m_nLength] = 0;

		return m_lpszData;
	}

protected:
	int m_nAlloc;
	int m_nLength;
	LPWSTR m_lpszData;
};

CXTPMarkupBuilder::CStringBuilder::CStringBuilder()
{
	m_nAlloc = 0;
	m_nLength = 0;
	m_lpszData = NULL;
}

CXTPMarkupBuilder::CStringBuilder::~CStringBuilder()
{
	if (m_lpszData)
	{
		delete[] m_lpszData;
	}
}

void CXTPMarkupBuilder::CStringBuilder::Add(LPCWSTR lpszWord)
{
	int nWordLength = (int)wcslen(lpszWord);

	if (m_nAlloc < m_nLength + nWordLength + 1)
	{
		int nAlloc = m_nLength + max(256, nWordLength + 1);
		LPWSTR lpszData = new WCHAR[nAlloc];

		if (m_nLength > 0) memcpy(lpszData, m_lpszData, m_nLength * sizeof(WCHAR));
		if (m_lpszData) delete[] m_lpszData;

		m_lpszData = lpszData;
		m_nAlloc = nAlloc;
	}

	memcpy(m_lpszData + m_nLength, lpszWord, nWordLength * sizeof(WCHAR));
	m_nLength += nWordLength;
}

void CXTPMarkupBuilder::CStringBuilder::Empty()
{
	m_nLength = 0;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXTPMarkupBuilder::CXTPMarkupBuilder(CXTPMarkupContext* pContext)
	: m_pMarkupContext(pContext)
	, m_pStaticExtension(NULL)
	, m_pStaticResourceExtension(NULL)
	, m_pComInitializer(new CXTPComInitializer)
{
	ASSERT(m_pMarkupContext);
}

CXTPMarkupBuilder::~CXTPMarkupBuilder()
{
	SAFE_DELETE(m_pStaticExtension);
	SAFE_DELETE(m_pStaticResourceExtension);
	SAFE_DELETE(m_pComInitializer);
}

inline BOOL IsXmlWhitespace(WCHAR ch)
{
	return (ch < 0x80 && iswspace(ch));
}

inline BOOL IsXmlWhitespaceString(LPCWSTR lpszString)
{
	ASSERT(NULL != lpszString);

	BOOL bWhitespaceString = FALSE;
	if(L'\0' != *lpszString)
	{
		bWhitespaceString = TRUE;
		LPCWSTR lpCh = lpszString;
		while(L'\0' != *lpCh)
		{
			if(!IsXmlWhitespace(*lpCh))
			{
				bWhitespaceString = FALSE;
				break;
			}

			++lpCh;
		}
	}
	return bWhitespaceString;
}

inline LPCWSTR NormalizeXmlContentText(
	LPCWSTR lpszString,
	CArray<WCHAR, WCHAR>& strBuffer,
	int nTextLen,
	BOOL bForceSpaceOnLeft,
	BOOL bForceSpaceOnRight)
{
	ASSERT(NULL != lpszString);

	LPCWSTR lpLeft = lpszString;
	LPCWSTR lpRight = lpLeft + nTextLen;

	BOOL bLeftSpaceFound = FALSE;
	BOOL bRightSpaceFound = FALSE;

	while(lpLeft < lpRight && IsXmlWhitespace(*lpLeft))
	{
		++lpLeft;
		bLeftSpaceFound = TRUE;
	}

	if(lpLeft < lpRight)
	{
		--lpRight;

		while(lpLeft < lpRight && IsXmlWhitespace(*lpRight))
		{
			--lpRight;
			bRightSpaceFound = TRUE;
		}

		++lpRight;
	}

	SIZE_T nLen = lpRight - lpLeft;
	ASSERT(0 <= (lpRight - lpLeft));

	strBuffer.SetSize(nLen + 2 + 1);
	LPWSTR lpStr = strBuffer.GetData();
	if(bForceSpaceOnLeft && bLeftSpaceFound)
	{
		*lpStr++ = L' ';
	}

	BOOL bSpaceFound = FALSE;
	while(lpLeft < lpRight)
	{
		if(!IsXmlWhitespace(*lpLeft))
		{
			if(bSpaceFound)
			{
				*lpStr++ = L' ';
				bSpaceFound = FALSE;
			}

			*lpStr++ = *lpLeft++;
		}
		else
		{
			bSpaceFound = TRUE;
			++lpLeft;
		}
	}

	if(bForceSpaceOnRight && bRightSpaceFound)
	{
		*lpStr++ = L' ';
	}
	*lpStr = L'\0';

	return strBuffer.GetData();
}

inline LPCWSTR FindXmlNamespaceEnd(LPCWSTR lpszName)
{
	ASSERT(NULL != lpszName);

	LPCWSTR lpNsEnd = NULL;
	LPCWSTR lpCh = lpszName;
	do
	{
		if(L':' == *lpCh)
		{
			lpNsEnd = lpCh;
			break;
		}
	}
	while(L'\0' != *lpCh++);

	return lpNsEnd;
}

inline BOOL IsXmlNamespaceSupported(LPCWSTR lpszTagOrAttrName)
{
	ASSERT(NULL != lpszTagOrAttrName);

	BOOL bSupported = TRUE;
	LPCWSTR lpNsEnd = FindXmlNamespaceEnd(lpszTagOrAttrName);
	if(NULL != lpNsEnd)
	{
		UINT_PTR cch = lpNsEnd - lpszTagOrAttrName;

		// Note: For now it is hardcoded and always assumed that
		// all default namespace is 'http://schemas.microsoft.com/winfx/2006/xaml/presentation'
		// and x='http://schemas.microsoft.com/winfx/2006/xaml', but
		// that's not right and has to be changed in future so that:
		//  1. users would be able to choose whatever prefixes for XML
		//     namespaces they like, the only thing that matters has
		//     to remain XML namespace name itself, e.g:
		//       <Grid x:Name='A' xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' />
		//     has to be to equal to
		//       <xaml:Grid winfx:Name='A' xmlns:xaml='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:winfx='http://schemas.microsoft.com/winfx/2006/xaml' />
		//  2. user would be able to add their own namespaces with
		//     custom tags and attributes without conflicting with
		//     this implementation.
		bSupported =
			(0 == wcsncmp(lpszTagOrAttrName, L"x:", cch)) ||
			(0 == wcsncmp(lpszTagOrAttrName, L"xtp:", cch));
	}

	return bSupported;
}

CXTPMarkupObject* CXTPMarkupBuilder::CreateObject(
	LPCWSTR lpszTag,
	XTPXML::IXMLDOMElement* pElement,
	CXTPMarkupObject* pParent /*= NULL*/)
{
	ASSERT(NULL != lpszTag);

	CXTPMarkupObject* pMarkupObject = NULL;

	LPCWSTR lpszAttributeDot = wcschr(lpszTag, L'.');
	if(NULL != lpszAttributeDot)
	{
		if(NULL != pParent)
		{
			_bstr_t tagFullName = lpszTag;
			LPWSTR lpszTagPart = tagFullName;
			LPWSTR lpszAttributePart = lpszTagPart + (lpszAttributeDot - lpszTag + 1);
			lpszAttributePart[-1] = L'\0';
			CXTPMarkupDependencyProperty* pProperty = FindProperty(lpszTagPart, lpszAttributePart);

			if(NULL != pProperty)
			{
				BOOL bValidUse = FALSE;
				if(!pProperty->IsAttached())
				{
					// A dependency property can only be an immediate
					// child of tah parent tag.
					bValidUse = (0 == wcscmp(pParent->GetTagName(), lpszTagPart));
				}
				else
				{
					// An attached property requries a parent to be on
					// the tree up to the root element.
					do
					{
						bValidUse = (0 == wcscmp(pParent->GetTagName(), lpszTagPart));
						if(bValidUse)
						{
							break;
						}

						pParent = pParent->GetLogicalParent();
					}
					while(NULL != pParent);
				}

				if(!bValidUse)
				{
					ThrowBuilderException(FormatString(
						_T("Invalid use of dependency property %ls"),
						(LPCTSTR)lpszTag));
				}

				pMarkupObject = new CXTPMarkupObjectProperty(pProperty);
			}
		}
	}
	else
	{
		CXTPMarkupType* pType = CXTPMarkupType::LookupTag(lpszTag);
		if(NULL != pType)
		{
			pMarkupObject = m_pMarkupContext->CreateMarkupObject(pType);
		}
	}

	if(NULL != pMarkupObject)
	{
		pMarkupObject->SetTagName(lpszTag);
		pMarkupObject->SetSourceMarkupElement(pElement);
	}

	return pMarkupObject;
}

CString CXTPMarkupBuilder::FormatString(LPCTSTR lpszFormat, LPCTSTR lpszParameter)
{
	CString str;
	str.Format(lpszFormat, lpszParameter);
	return str;
}

CString CXTPMarkupBuilder::FormatString(LPCTSTR lpszFormat, LPCTSTR lpszParameter1, LPCTSTR lpszParameter2)
{
	CString str;
	str.Format(lpszFormat, lpszParameter1, lpszParameter2);
	return str;
}

CString CXTPMarkupBuilder::FormatString(LPCTSTR lpszFormat, LPCTSTR lpszParameter1, LPCTSTR lpszParameter2, LPCTSTR lpszParameter3)
{
	CString str;
	str.Format(lpszFormat, lpszParameter1, lpszParameter2, lpszParameter3);
	return str;
}

CXTPMarkupDependencyProperty* CXTPMarkupBuilder::FindProperty(LPCWSTR lpszTag, LPCWSTR lpszAttribute)
{
	CXTPMarkupType* pType = CXTPMarkupType::LookupTag(lpszTag);
	if (!pType)
		return NULL;

	return CXTPMarkupDependencyProperty::FindProperty(pType, lpszAttribute);
}

CXTPMarkupDependencyProperty* CXTPMarkupBuilder::FindProperty(CXTPMarkupType* pRuntimeClass, LPCWSTR lpszAttribute)
{
	LPCWSTR lpszAttributeDot = wcschr(lpszAttribute, L'.');

	if (lpszAttributeDot != NULL)
	{
		*(LPWSTR)lpszAttributeDot = 0;
		CXTPMarkupDependencyProperty* pProperty = FindProperty(lpszAttribute, lpszAttributeDot + 1);
		*(LPWSTR)lpszAttributeDot = '.';

		return pProperty;
	}

	if (!pRuntimeClass)
		return NULL;

	return CXTPMarkupDependencyProperty::FindProperty(pRuntimeClass, lpszAttribute);
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertValue(CXTPMarkupDependencyProperty* pProperty, CXTPMarkupObject* pValue)
{
	CXTPMarkupObject* pNewValue = NULL;

	if (pProperty->GetMetadata() && pProperty->GetMetadata()->m_pConverter)
	{
		pNewValue = (*pProperty->GetMetadata()->m_pConverter)(this, pValue, pProperty);
	}

	if (pNewValue == NULL)
	{
		CXTPMarkupAutoPtr pConverter(GetMarkupContext()->CreateMarkupObject(pProperty->GetPropetyType()));
		pConverter->SetAssociatedProperty(pProperty);
		pNewValue = pConverter->ConvertFrom(this, pValue);
	}

	if (!pNewValue)
	{
		ThrowBuilderException(CXTPMarkupBuilder::FormatString(_T("Cannot convert string in attribute '%ls' to object of type '%ls'"),
			(LPCTSTR)pProperty->GetName(), (LPCTSTR)pProperty->GetPropetyType()->m_lpszClassName));
	}
	return pNewValue;
}

CXTPMarkupObject* CXTPMarkupBuilder::CreateTypeKey(LPCWSTR lpszType)
{
	CXTPMarkupType* pType = CXTPMarkupType::LookupTag(lpszType);
	if (!pType)
	{
		return ThrowBuilderException(FormatString(_T("Cannot find type '%ls'"), (LPCTSTR)lpszType)), NULL;
	}

	pType->AddRef();
	return pType;
}

CXTPMarkupObject* CXTPMarkupBuilder::ResolveMarkupExtension(LPCWSTR lpszValue)
{
	int nLength = (int)wcslen(lpszValue);

	if (lpszValue[nLength - 1] != '}')
	{
		ThrowBuilderException(_T("Markup extension expressions must end with '}'"));
	}

	*((LPWSTR)lpszValue + nLength - 1) = '\0';

	if (wcsncmp(lpszValue, L"x:Static ", 9) == 0)
	{
		return GetStaticExtension()->ProvideValue(this, lpszValue + 9);
	}

	if (wcsncmp(lpszValue, L"StaticResource ", 15) == 0)
	{
		return GetStaticResourceExtension()->ProvideValue(this, lpszValue + 15);
	}

	if (wcsncmp(lpszValue, L"DynamicResource ", 16) == 0)
	{
		return GetStaticResourceExtension()->ProvideValue(this, lpszValue + 16);
	}

	if (wcsncmp(lpszValue, L"x:Type ", 7) == 0)
	{
		return CreateTypeKey(lpszValue + 7);
	}

	return ThrowBuilderException(FormatString(_T("The tag '%ls' does not exist in XML namespace"), (LPCTSTR)lpszValue)), NULL;
}

void CXTPMarkupBuilder::LoadMarkupObjectAttributes(
	CXTPMarkupObject* pObject,
	XTPXML::IXMLDOMElement* pElement)
{
	ASSERT(NULL != pObject);
	ASSERT(NULL != pElement);

	XTPXML::IXMLDOMNamedNodeMapPtr pAttributes = pElement->attributes;
	XTPXML::IXMLDOMNodePtr pAttrNode;
	while (pAttrNode = pAttributes->nextNode())
	{
		// Find attribute property object.
		_bstr_t attrName = pAttrNode->nodeName;
		LPCWSTR lpszAttrName = attrName;

		// Exclude 'xmlns' and all attibutes that prefixed with
		// namespace different from 'x:'.
		if(0 == wcscmp(lpszAttrName, L"xmlns")
			|| !IsXmlNamespaceSupported(lpszAttrName))
		{
			continue;
		}

		CXTPMarkupDependencyProperty* pProperty = FindProperty(pObject->GetType(), attrName);
		if(NULL == pProperty)
		{
			ThrowBuilderException(FormatString(_T("The property '%ls' does not exist in XML namespace"), (LPCTSTR)attrName));
		}

		// Load attribute content.
		_bstr_t attrValue = pAttrNode->text;

		CXTPMarkupAutoPtr pContentObject;
		LPCWSTR lpszAttrValue = NULL;
		if(0 < attrValue.length())
		{
			lpszAttrValue = attrValue;
			if(L'{' == lpszAttrValue[0])
			{
				if(L'}' == lpszAttrValue[1])
				{
					lpszAttrValue += 2;
				}
				else
				{
					pContentObject.Assign(ResolveMarkupExtension(lpszAttrValue + 1));
				}
			}
		}

		BOOL bNotifyOnLoad = FALSE;
		if(NULL == pContentObject.m_pObject)
		{
			pContentObject.Assign(CXTPMarkupString::CreateValue(lpszAttrValue));
			bNotifyOnLoad = TRUE;
		}

		pObject->SetPropertyObject(this, pProperty, pContentObject.m_pObject);

		if(bNotifyOnLoad)
		{
			pContentObject.m_pObject->OnLoaded(this);
		}
	}
}

BOOL CXTPMarkupBuilder::NeedPreserveXmlNodeWhitespaces(XTPXML::IXMLDOMNode* pNode) const
{
	ASSERT(NULL != pNode);

	BOOL bPreserve = FALSE;

	// Traversing gives better performance on avarage then getNamedItem
	XTPXML::IXMLDOMNamedNodeMapPtr pAttributes = pNode->attributes;
	XTPXML::IXMLDOMNodePtr pAttribute;
	while (pAttribute = pAttributes->nextNode())
	{
		if (0 == wcscmp(pAttribute->nodeName, L"xml:space"))
		{
			bPreserve = (0 == wcscmp(pAttribute->text, L"preserve"));
			break;
		}
	}

	return bPreserve;
}

void CXTPMarkupBuilder::LoadMarkupObjectTextContent(
	CXTPMarkupObject* pObject,
	XTPXML::IXMLDOMNode* pContent,
	BOOL bPreformatted)
{
	ASSERT(NULL != pObject);
	ASSERT(NULL != pContent);

	XTPXML::IXMLDOMTextPtr pText = pContent;
	_bstr_t nodeText = pText->data;
	if(0 < nodeText.length())
	{
		LPCWSTR lpszContent = nodeText;
		CArray<WCHAR, WCHAR> textBuffer;
		if(!bPreformatted)
		{
			BOOL bPreserveWhitespaces = NeedPreserveXmlNodeWhitespaces(pContent->parentNode);
			if(!bPreserveWhitespaces)
			{
				lpszContent = NormalizeXmlContentText(lpszContent, textBuffer,
					nodeText.length(), pContent->previousSibling, pContent->nextSibling);
			}
		}

		BOOL bAddContent = (L'\0' != *lpszContent);

		if(bAddContent && IsXmlWhitespaceString(lpszContent))
		{
			if(pObject->HasContentObject()
				&& pObject->AllowWhiteSpaceContent())
			{
				lpszContent = L" ";
			}
			else
			{
				bAddContent = FALSE;
			}
		}

		if(bAddContent)
		{
			CXTPMarkupAutoPtr pContentObject = CXTPMarkupString::CreateValue(lpszContent);
			if(NULL == pContentObject.m_pObject)
			{
				ThrowBuilderException(FormatString(_T("Failed to load '%ls' object content"),
					(LPCTSTR)pObject->GetTagName()));
			}

			pObject->SetContentObject(this, pContentObject.m_pObject);
			pContentObject.m_pObject->OnLoaded(this);
		}
	}
}

void CXTPMarkupBuilder::LoadMarkupObjectChild(
	CXTPMarkupObject* pObject,
	XTPXML::IXMLDOMNode* pChild)
{
	ASSERT(NULL != pObject);
	ASSERT(NULL != pChild);

	XTPXML::IXMLDOMElementPtr pChildElement = pChild;
	CXTPMarkupAutoPtr pContentObject = LoadXmlElement(pObject, pChildElement);
	if(NULL != pContentObject.m_pObject)
	{
		if(pContentObject->IsKindOf(MARKUP_TYPE(CXTPMarkupObjectProperty)))
		{
			CXTPMarkupObjectProperty* pObjectProperty = static_cast<CXTPMarkupObjectProperty*>(pContentObject.m_pObject);
			if(NULL == pObjectProperty->m_pContent)
			{
				ThrowBuilderException(FormatString(_T("'%ls' property does not have a value"),
					(LPCTSTR)pObjectProperty->m_pProperty->GetName()));
			}

			pObject->SetPropertyObject(this,
				pObjectProperty->m_pProperty,
				pObjectProperty->m_pContent);
		}
		else if(pContentObject->IsKindOf(MARKUP_TYPE(CXTPMarkupExtension)))
		{
			pObject->AddExtension(MARKUP_STATICCAST(CXTPMarkupExtension, pContentObject.m_pObject));
		}
		else
		{
			pObject->SetContentObject(this, pContentObject.m_pObject);
		}
	}
}

CXTPMarkupObject* CXTPMarkupBuilder::LoadXmlElement(
	CXTPMarkupObject* pParent,
	XTPXML::IXMLDOMElement* pElement)
{
	ASSERT(NULL != pElement);

	CXTPMarkupAutoPtr pObject = NULL;
	_bstr_t tagName = pElement->tagName;
	if(IsXmlNamespaceSupported(tagName))
	{
		pObject.Assign(CreateObject(tagName, pElement, pParent));
		if(NULL == pObject.m_pObject)
		{
			ThrowBuilderException(FormatString(_T("The tag '%ls' does not exist in XML namespace"), (LPCTSTR)tagName));
		}

		m_arrObjects.AddTail(pObject.m_pObject);

		try
		{
			pObject.m_pObject->SetBeingLoaded();

			LoadMarkupObjectAttributes(pObject.m_pObject, pElement);

			XTPXML::IXMLDOMNodeListPtr pChildNodes = pElement->childNodes;
			XTPXML::IXMLDOMNodePtr pChildNode;
			while (pChildNode = pChildNodes->nextNode())
			{
				XTPXML::DOMNodeType nodeType = pChildNode->nodeType;
				switch(nodeType)
				{
				case XTPXML::NODE_CDATA_SECTION:
					LoadMarkupObjectTextContent(pObject.m_pObject, pChildNode, TRUE);
					break;
				case XTPXML::NODE_TEXT:
					LoadMarkupObjectTextContent(pObject.m_pObject, pChildNode, FALSE);
					break;
				case XTPXML::NODE_ELEMENT:
					LoadMarkupObjectChild(pObject.m_pObject, pChildNode);
					break;
				}
			}

			pObject.m_pObject->SetBeingLoaded(FALSE);
			pObject.m_pObject->OnLoaded(this);

			m_arrObjects.RemoveTail();
		}
		catch(...)
		{
			pObject.m_pObject->SetBeingLoaded(FALSE);
			m_arrObjects.RemoveTail();
			throw;
		}

		pObject.AddRef();
	}
	else
	{
		TRACE1("Markup tag skipped: %ls\n", (LPCWSTR)tagName);
	}

	return pObject.m_pObject;
}

CXTPMarkupObject* CXTPMarkupBuilder::ParseObject(CXTPMarkupParser* pParser)
{
	ASSERT(NULL != pParser);

	CXTPMarkupObject *pRootObject = NULL;

	m_strLastError.Empty();

	if(pParser->GetXmlDocument())
	{
		try
		{
			TRY
			{			
				XTPXML::IXMLDOMElementPtr pDocumentElement = pParser->GetXmlDocument()->documentElement;
				pRootObject = LoadXmlElement(NULL, pDocumentElement);
			}
			CATCH(CXTPMarkupBuilderException, e)
			{
				TCHAR szErrorMessage[512];
				UINT nHelpContext;
				if(e->GetErrorMessage(szErrorMessage, 512, &nHelpContext))
				{
					m_strLastError = szErrorMessage;
					TRACE(m_strLastError + _T("\n"));
				}

				MARKUP_RELEASE(pRootObject);
			}
			END_CATCH
		}
		catch(const _com_error& ex)
		{
			m_strLastError.Format(_T("COM Error [0x%08X]: %s"), ex.Error(), ex.ErrorMessage());
			TRACE(m_strLastError + _T("\n"));
			MARKUP_RELEASE(pRootObject);
		}
		catch(...)
		{
			MARKUP_RELEASE(pRootObject);
			throw;
		}
	}
	else
	{
		m_strLastError = pParser->FormatErrorMessage();
		TRACE(m_strLastError + _T("\n"));
	}

	return pRootObject;
}


CXTPMarkupUIElement* CXTPMarkupBuilder::Parse(CXTPMarkupParser* pParser)
{
	ASSERT(NULL != pParser);

	CXTPMarkupUIElement* pRootUIElement = NULL;
	m_strLastError.Empty();

	if(pParser->GetXmlDocument())
	{
		try
		{
			TRY
			{
				m_pMarkupContext->ResetScriptEngine();

				XTPXML::IXMLDOMElementPtr pDocumentElement = pParser->GetXmlDocument()->documentElement;
				CXTPMarkupObject* pRootObject = LoadXmlElement(NULL, pDocumentElement);
				if(NULL == pRootObject
					|| !pRootObject->IsKindOf(MARKUP_TYPE(CXTPMarkupUIElement)))
				{
					MARKUP_RELEASE(pRootObject);
					ThrowBuilderException(_T("Root element have to be 'CXTPMarkupUIElement' type"));
				}

				pRootUIElement = static_cast<CXTPMarkupUIElement*>(pRootObject);
				pRootUIElement->Apply();

				m_pMarkupContext->RegisterScriptNamedObjects(this, pRootUIElement);
				m_pMarkupContext->RunScriptEngine(this);
			}
			CATCH(CXTPMarkupBuilderException, e)
			{
				TCHAR szErrorMessage[512];
				UINT nHelpContext;
				if(e->GetErrorMessage(szErrorMessage, 512, &nHelpContext))
				{
					m_strLastError = szErrorMessage;
					TRACE(m_strLastError + _T("\n"));
				}

				MARKUP_RELEASE(pRootUIElement);
			}
			END_CATCH
		}
		catch(const _com_error& ex)
		{
			m_strLastError.Format(_T("COM Error [0x%08X]: %s"), ex.Error(), ex.ErrorMessage());
			TRACE(m_strLastError + _T("\n"));
			MARKUP_RELEASE(pRootUIElement);
		}
		catch(...)
		{
			MARKUP_RELEASE(pRootUIElement);
			throw;
		}
	}
	else if(S_OK != pParser->GetErrorInfo().nCode)
	{
		m_strLastError = pParser->FormatErrorMessage();
		TRACE(m_strLastError + _T("\n"));
	}

	return pRootUIElement;
}


BOOL CXTPMarkupBuilder::ColorStringToKnownColor(LPCWSTR lpszValue, COLORREF& clr)
{
	int nLength = (int)wcslen(lpszValue);

	WCHAR ch = lpszValue[0];
	if (ch >= 'a' && ch <= 'z')
		ch = (WCHAR)(ch + 'A' - 'a');
	else if (ch < 'A' || ch > 'Z')
		return FALSE;

	switch (nLength)
	{
	case 3:
		if (ch == _T('R') && _wcsicmp(lpszValue, L"RED") == 0) return clr = 0xff0000ff;
		if (ch == _T('T') && _wcsicmp(lpszValue, L"TAN") == 0) return clr = 0xff8cb4d2;
		return FALSE;
	case 4:
		switch (ch)
		{
		case _T('A'): if (_wcsicmp(lpszValue, L"AQUA") == 0) return clr = 0xffffff00; return FALSE;
		case _T('B'): if (_wcsicmp(lpszValue, L"BLUE") == 0) return clr = 0xffff0000; return FALSE;
		case _T('C'): if (_wcsicmp(lpszValue, L"CYAN") == 0) return clr = 0xffffff00; return FALSE;
		case _T('G'):
			if (_wcsicmp(lpszValue, L"GOLD") == 0) return clr = 0xff00d7ff;
			if (_wcsicmp(lpszValue, L"GRAY") == 0) return clr = 0xff808080;
			return FALSE;
		case _T('L'): if (_wcsicmp(lpszValue, L"LIME") == 0) return clr = 0xff00ff00; return FALSE;
		case _T('N'): if (_wcsicmp(lpszValue, L"NAVY") == 0) return clr = 0xff800000; return FALSE;
		case _T('P'):
			if (_wcsicmp(lpszValue, L"PERU") == 0) return clr = 0xff3f85cd;
			if (_wcsicmp(lpszValue, L"PINK") == 0) return clr = 0xffcbc0ff;
			if (_wcsicmp(lpszValue, L"PLUM") == 0) return clr = 0xffdda0dd;
			return FALSE;
		case _T('S'): if (_wcsicmp(lpszValue, L"SNOW") == 0) return clr = 0xfffafaff; return FALSE;
		case _T('T'): if (_wcsicmp(lpszValue, L"TEAL") == 0) return clr = 0xff808000; return FALSE;
		}
		return FALSE;
	case 5:
		switch (ch)
		{
		case _T('A'): if (_wcsicmp(lpszValue, L"AZURE") == 0) return clr = 0xfffffff0; return FALSE;
		case _T('B'):
			if (_wcsicmp(lpszValue, L"BEIGE") == 0) return clr = 0xffdcf5f5;
			if (_wcsicmp(lpszValue, L"BLACK") == 0) return clr = 0xff000000;
			if (_wcsicmp(lpszValue, L"BROWN") == 0) return clr = 0xff2a2aa5;
			return FALSE;
		case _T('C'): if (_wcsicmp(lpszValue, L"CORAL") == 0) return clr = 0xff507fff; return FALSE;
		case _T('G'): if (_wcsicmp(lpszValue, L"GREEN") == 0) return clr = 0xff008000; return FALSE;
		case _T('I'): if (_wcsicmp(lpszValue, L"IVORY") == 0) return clr = 0xfff0ffff; return FALSE;
		case _T('K'): if (_wcsicmp(lpszValue, L"KHAKI") == 0) return clr = 0xff8ce6f0; return FALSE;
		case _T('L'): if (_wcsicmp(lpszValue, L"LINEN") == 0) return clr = 0xffe6f0fa; return FALSE;
		case _T('O'): if (_wcsicmp(lpszValue, L"OLIVE") == 0) return clr = 0xff008080; return FALSE;
		case _T('W'):
			if (_wcsicmp(lpszValue, L"WHEAT") == 0) return clr = 0xffb3def5;
			if (_wcsicmp(lpszValue, L"WHITE") == 0) return clr = 0xffffffff;
			return FALSE;
		}
		return FALSE;
	case 6:
		switch (ch)
		{
		case _T('M'): if (_wcsicmp(lpszValue, L"MAROON") == 0) return clr = 0xff000080; return FALSE;
		case _T('O'):
			if (_wcsicmp(lpszValue, L"ORANGE") == 0) return clr = 0xff00a5ff;
			if (_wcsicmp(lpszValue, L"ORCHID") == 0) return clr = 0xffd670da;
			return FALSE;
		case _T('P'): if (_wcsicmp(lpszValue, L"PURPLE") == 0) return clr = 0xff800080; return FALSE;
		case _T('S'):
			if (_wcsicmp(lpszValue, L"SALMON") == 0) return clr = 0xff7280fa;
			if (_wcsicmp(lpszValue, L"SIENNA") == 0) return clr = 0xff2d52a0;
			if (_wcsicmp(lpszValue, L"SILVER") == 0) return clr = 0xffc0c0c0;
			return FALSE;
		case _T('T'): if (_wcsicmp(lpszValue, L"TOMATO") == 0) return clr = 0xff4763ff; return FALSE;
		case _T('V'): if (_wcsicmp(lpszValue, L"VIOLET") == 0) return clr = 0xffee82ee; return FALSE;
		case _T('Y'): if (_wcsicmp(lpszValue, L"YELLOW") == 0) return clr = 0xff00ffff; return FALSE;
		case _T('I'): if (_wcsicmp(lpszValue, L"INDIGO") == 0) return clr = 0xff82004b; return FALSE;
		case _T('B'): if (_wcsicmp(lpszValue, L"BISQUE") == 0) return clr = 0xffc4e4ff; return FALSE;
		}
		return FALSE;
	case 7:
		switch (ch)
		{
		case _T('C'): if (_wcsicmp(lpszValue, L"CRIMSON") == 0) return clr = 0xff3c14dc; return FALSE;
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKRED") == 0) return clr = 0xff00008b;
			if (_wcsicmp(lpszValue, L"DIMGRAY") == 0) return clr = 0xff696969;
			return FALSE;
		case _T('F'): if (_wcsicmp(lpszValue, L"FUCHSIA") == 0) return clr = 0xffff00ff; return FALSE;
		case _T('H'): if (_wcsicmp(lpszValue, L"HOTPINK") == 0) return clr = 0xffb469ff; return FALSE;
		case _T('M'): if (_wcsicmp(lpszValue, L"MAGENTA") == 0) return clr = 0xffff00ff; return FALSE;
		case _T('O'): if (_wcsicmp(lpszValue, L"OLDLACE") == 0)return clr = 0xffe6f5fd; return FALSE;
		case _T('S'): if (_wcsicmp(lpszValue, L"SKYBLUE") == 0) return clr = 0xffebce87; return FALSE;
		case _T('T'): if (_wcsicmp(lpszValue, L"THISTLE") == 0) return clr = 0xffd8bfd8; return FALSE;
		}
		return FALSE;
	case 8:
		switch (ch)
		{
		case _T('C'): if (_wcsicmp(lpszValue, L"CORNSILK") == 0) return clr = 0xffdcf8ff; return FALSE;
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKBLUE") == 0) return clr = 0xff8b0000;
			if (_wcsicmp(lpszValue, L"DARKCYAN") == 0) return clr = 0xff8b8b00;
			if (_wcsicmp(lpszValue, L"DARKGRAY") == 0) return clr = 0xffa9a9a9;
			if (_wcsicmp(lpszValue, L"DEEPPINK") == 0) return clr = 0xff9314ff;
			return FALSE;
		case _T('H'): if (_wcsicmp(lpszValue, L"HONEYDEW") == 0) return clr = 0xfff0fff0; return FALSE;
		case _T('L'): if (_wcsicmp(lpszValue, L"LAVENDER") == 0) return clr = 0xfffae6e6; return FALSE;
		case _T('M'): if (_wcsicmp(lpszValue, L"MOCCASIN") == 0) return clr = 0xffb5e4ff; return FALSE;
		case _T('S'):
			if (_wcsicmp(lpszValue, L"SEAGREEN") == 0) return clr = 0xff578b2e;
			if (_wcsicmp(lpszValue, L"SEASHELL") == 0) return clr = 0xffeef5ff;
			return FALSE;
		}
		return FALSE;
	case 9:
		switch (ch)
		{
		case _T('A'): if (_wcsicmp(lpszValue, L"ALICEBLUE") == 0) return clr = 0xfffff8f0; return FALSE;
		case _T('B'): if (_wcsicmp(lpszValue, L"BURLYWOOD") == 0) return clr = 0xff87b8de; return FALSE;
		case _T('C'):
			if (_wcsicmp(lpszValue, L"CADETBLUE") == 0) return clr = 0xffa09e5f;
			if (_wcsicmp(lpszValue, L"CHOCOLATE") == 0) return clr = 0xff1e69d2;
			return FALSE;
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKGREEN") == 0) return clr = 0xff006400;
			if (_wcsicmp(lpszValue, L"DARKKHAKI") == 0) return clr = 0xff6bb7bd;
			return FALSE;
		case _T('F'): if (_wcsicmp(lpszValue, L"FIREBRICK") == 0) return clr = 0xff2222b2; return FALSE;
		case _T('G'):
			if (_wcsicmp(lpszValue, L"GAINSBORO") == 0) return clr = 0xffdcdcdc;
			if (_wcsicmp(lpszValue, L"GOLDENROD") == 0) return clr = 0xff20a5da;
			return FALSE;
		case _T('I'): if (_wcsicmp(lpszValue, L"INDIANRED") == 0)return clr = 0xff5c5ccd; return FALSE;
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LAWNGREEN") == 0)return clr = 0xff00fc7c;
			if (_wcsicmp(lpszValue, L"LIGHTBLUE") == 0) return clr = 0xffe6d8ad;
			if (_wcsicmp(lpszValue, L"LIGHTCYAN") == 0) return clr = 0xffffffe0;
			if (_wcsicmp(lpszValue, L"LIGHTGRAY") == 0) return clr = 0xffd3d3d3;
			if (_wcsicmp(lpszValue, L"LIGHTPINK") == 0) return clr = 0xffc1b6ff;
			if (_wcsicmp(lpszValue, L"LIMEGREEN") == 0) return clr = 0xff32cd32;
			return FALSE;
		case _T('M'):
			if (_wcsicmp(lpszValue, L"MINTCREAM") == 0) return clr = 0xfffafff5;
			if (_wcsicmp(lpszValue, L"MISTYROSE") == 0) return clr = 0xffe1e4ff;
			return FALSE;
		case _T('O'):
			if (_wcsicmp(lpszValue, L"OLIVEDRAB") == 0) return clr = 0xff238e6b;
			if (_wcsicmp(lpszValue, L"ORANGERED") == 0) return clr = 0xff0045ff;
			return FALSE;
		case _T('P'):
			if (_wcsicmp(lpszValue, L"PALEGREEN") == 0) return clr = 0xff98fb98;
			if (_wcsicmp(lpszValue, L"PEACHPUFF") == 0) return clr = 0xffb9daff;
			return FALSE;
		case _T('R'):
			if (_wcsicmp(lpszValue, L"ROSYBROWN") == 0) return clr = 0xff8f8fbc;
			if (_wcsicmp(lpszValue, L"ROYALBLUE") == 0) return clr = 0xffe16941;
			return FALSE;
		case _T('S'):
			if (_wcsicmp(lpszValue, L"SLATEBLUE") == 0) return clr = 0xffcd5a6a;
			if (_wcsicmp(lpszValue, L"SLATEGRAY") == 0)  return clr = 0xff908070;
			if (_wcsicmp(lpszValue, L"STEELBLUE") == 0) return clr = 0xffb48246;
			return FALSE;
		case _T('T'): if (_wcsicmp(lpszValue, L"TURQUOISE") == 0) return clr = 0xffd0e040; return FALSE;
		}
		return FALSE;
	case 10:
		switch (ch)
		{
		case _T('A'): if (_wcsicmp(lpszValue, L"AQUAMARINE") == 0) return clr = 0xffd4ff7f; return FALSE;
		case _T('B'): if (_wcsicmp(lpszValue, L"BLUEVIOLET") == 0) return clr = 0xffe22b8a; return FALSE;
		case _T('C'): if (_wcsicmp(lpszValue, L"CHARTREUSE") == 0) return clr = 0xff00ff7f; return FALSE;
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKORANGE") == 0) return clr = 0xff008cff;
			if (_wcsicmp(lpszValue, L"DARKORCHID") == 0) return clr = 0xffcc3299;
			if (_wcsicmp(lpszValue, L"DARKSALMON") == 0) return clr = 0xff7a96e9;
			if (_wcsicmp(lpszValue, L"DARKVIOLET") == 0) return clr = 0xffd30094;
			if (_wcsicmp(lpszValue, L"DODGERBLUE") == 0) return clr = 0xffff901e;
			return FALSE;
		case _T('G'): if (_wcsicmp(lpszValue, L"GHOSTWHITE") == 0) return clr = 0xfffff8f8; return FALSE;
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LIGHTCORAL") == 0)  return clr = 0xff8080f0;
			if (_wcsicmp(lpszValue, L"LIGHTGREEN") == 0) return clr = 0xff90ee90;
			return FALSE;
		case _T('M'):
			if (_wcsicmp(lpszValue, L"MEDIUMBLUE") == 0) return clr = 0xffcd0000; return FALSE;
		case _T('P'):
			if (_wcsicmp(lpszValue, L"PAPAYAWHIP") == 0) return clr = 0xffd5efff;
			if (_wcsicmp(lpszValue, L"POWDERBLUE") == 0) return clr = 0xffe6e0b0;
			return FALSE;
		case _T('S'): if (_wcsicmp(lpszValue, L"SANDYBROWN") == 0) return clr = 0xff60a4f4; return FALSE;
		case _T('W'): if (_wcsicmp(lpszValue, L"WHITESMOKE") == 0) return clr = 0xfff5f5f5; return FALSE;
		}
		return FALSE;
	case 11:
		switch (ch)
		{
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKMAGENTA") == 0) return clr = 0xff8b008b;
			if (_wcsicmp(lpszValue, L"DEEPSKYBLUE") == 0) return clr = 0xffffbf00;
			return FALSE;
		case _T('F'):
			if (_wcsicmp(lpszValue, L"FLORALWHITE") == 0)  return clr = 0xfff0faff;
			if (_wcsicmp(lpszValue, L"FORESTGREEN") == 0) return clr = 0xff228b22;
			return FALSE;
		case _T('G'): if (_wcsicmp(lpszValue, L"GREENYELLOW") == 0) return clr = 0xff2fffad; return FALSE;
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LIGHTSALMON") == 0)  return clr = 0xff7aa0ff;
			if (_wcsicmp(lpszValue, L"LIGHTYELLOW") == 0) return clr = 0xffe0ffff;
			return FALSE;
		case _T('N'):
			if (_wcsicmp(lpszValue, L"NAVAJOWHITE") == 0) return clr = 0xffaddeff;
			return FALSE;
		case _T('S'):
			if (_wcsicmp(lpszValue, L"SADDLEBROWN") == 0) return clr = 0xff13458b;
			if (_wcsicmp(lpszValue, L"SPRINGGREEN") == 0) return clr = 0xff7fff00;
			return FALSE;
		case _T('T'): if (_wcsicmp(lpszValue, L"TRANSPARENT") == 0) {clr = 0x00ffffff; return TRUE;} return FALSE;
		case _T('Y'): if (_wcsicmp(lpszValue, L"YELLOWGREEN") == 0) return clr = 0xff32cd9a; return FALSE;
		}
		return FALSE;
	case 12:
		switch (ch)
		{
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LIGHTSKYBLUE") == 0) return clr = 0xffface87;
			if (_wcsicmp(lpszValue, L"LEMONCHIFFON") == 0)   return clr = 0xffcdfaff;
			return FALSE;
		case _T('M'):
			if (_wcsicmp(lpszValue, L"MEDIUMORCHID") == 0) return clr = 0xffd355ba;
			if (_wcsicmp(lpszValue, L"MEDIUMPURPLE") == 0) return clr = 0xffdb7093;
			if (_wcsicmp(lpszValue, L"MIDNIGHTBLUE") == 0) return clr = 0xff701919;
			return FALSE;
		case _T('D'): if (_wcsicmp(lpszValue, L"DARKSEAGREEN") == 0) return clr = 0xff8fbc8f; return FALSE;
		case _T('A'): if (_wcsicmp(lpszValue, L"ANTIQUEWHITE") == 0) return clr = 0xffd7ebfa; return FALSE;
		}
		return FALSE;
	case 13:
		switch (ch)
		{
		case _T('D'):
			if (_wcsicmp(lpszValue, L"DARKSLATEBLUE") == 0) return clr = 0xff8b3d48;
			if (_wcsicmp(lpszValue, L"DARKSLATEGRAY") == 0) return clr = 0xff4f4f2f;
			if (_wcsicmp(lpszValue, L"DARKGOLDENROD") == 0) return clr = 0xff0b86b8;
			if (_wcsicmp(lpszValue, L"DARKTURQUOISE") == 0) return clr = 0xffd1ce00;
			return FALSE;
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LIGHTSEAGREEN") == 0) return clr = 0xffaab220;
			if (_wcsicmp(lpszValue, L"LAVENDERBLUSH") == 0) return clr = 0xfff5f0ff;
			return FALSE;
		case _T('P'):
			if (_wcsicmp(lpszValue, L"PALEGOLDENROD") == 0) return clr = 0xffaae8ee;
			if (_wcsicmp(lpszValue, L"PALETURQUOISE") == 0) return clr = 0xffeeeeaf;
			if (_wcsicmp(lpszValue, L"PALEVIOLETRED") == 0) return clr = 0xff9370db;
			return FALSE;
		}
		return FALSE;
	case 14:
		switch (ch)
		{
		case _T('B'): if (_wcsicmp(lpszValue, L"BLANCHEDALMOND") == 0) return clr = 0xffcdebff; return FALSE;
		case _T('C'): if (_wcsicmp(lpszValue, L"CORNFLOWERBLUE") == 0) return clr = 0xffed9564; return FALSE;
		case _T('D'): if (_wcsicmp(lpszValue, L"DARKOLIVEGREEN") == 0) return clr = 0xff2f6b55; return FALSE;
		case _T('L'):
			if (_wcsicmp(lpszValue, L"LIGHTSLATEGRAY") == 0) return clr = 0xff998877;
			if (_wcsicmp(lpszValue, L"LIGHTSTEELBLUE") == 0) return clr = 0xffdec4b0;
			return FALSE;
		case _T('M'): if (_wcsicmp(lpszValue, L"MEDIUMSEAGREEN") == 0) return clr = 0xff71b33c; return FALSE;
		}
		return FALSE;
	case 15:
		if (_wcsicmp(lpszValue, L"MEDIUMSLATEBLUE") == 0) return clr = 0xffee687b;
		if (_wcsicmp(lpszValue, L"MEDIUMTURQUOISE") == 0) return clr = 0xffccd148;
		if (_wcsicmp(lpszValue, L"MEDIUMVIOLETRED") == 0) return clr = 0xff8515c7;
		return FALSE;
	case 16:
		if (_wcsicmp(lpszValue, L"MEDIUMAQUAMARINE") == 0) return clr = 0xffaacd66;
		return FALSE;
	case 17:
		if (_wcsicmp(lpszValue, L"MEDIUMSPRINGGREEN") == 0) return clr = 0xff9afa00;
		return FALSE;
	case 20:
		if (_wcsicmp(lpszValue, L"LIGHTGOLDENRODYELLOW") == 0) return clr = 0xffd2fafa;
		return FALSE;
	}

	return FALSE;
}


CXTPMarkupObject* CXTPMarkupBuilder::ConvertTextDecorations(
	CXTPMarkupBuilder* /*pBuilder*/, 
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);

		if (_wcsicmp(lpszValue, L"Underline") == 0)
			return CXTPMarkupEnum::CreateValue(1);

		if (_wcsicmp(lpszValue, L"Strikethrough") == 0)
			return CXTPMarkupEnum::CreateValue(2);

		if (_wcsicmp(lpszValue, L"") == 0)
			return CXTPMarkupEnum::CreateValue(0);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertFontWeight(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		switch (nLength)
		{
		case 4:
			if (_wcsicmp(lpszValue, L"Bold") == 0) return CXTPMarkupEnum::CreateValue(FW_BOLD);
			if (_wcsicmp(lpszValue, L"Thin") == 0) return CXTPMarkupEnum::CreateValue(FW_THIN);
			return NULL;

		case 5:
			if (_wcsicmp(lpszValue, L"Light") == 0) return CXTPMarkupEnum::CreateValue(FW_LIGHT);
			if (_wcsicmp(lpszValue, L"Black") == 0) return CXTPMarkupEnum::CreateValue(FW_BLACK);
			if (_wcsicmp(lpszValue, L"Heavy") == 0) return CXTPMarkupEnum::CreateValue(FW_HEAVY);
			return NULL;

		case 6:
			if (_wcsicmp(lpszValue, L"Normal") == 0) return CXTPMarkupEnum::CreateValue(FW_NORMAL);
			if (_wcsicmp(lpszValue, L"Medium") == 0) return CXTPMarkupEnum::CreateValue(FW_MEDIUM);
			return NULL;

		case 7:
			if (_wcsicmp(lpszValue, L"Regular") == 0) return CXTPMarkupEnum::CreateValue(FW_REGULAR);
			return NULL;

		case 8:
			if (_wcsicmp(lpszValue, L"SemiBold") == 0) return CXTPMarkupEnum::CreateValue(FW_SEMIBOLD);
			if (_wcsicmp(lpszValue, L"DemiBold") == 0) return CXTPMarkupEnum::CreateValue(FW_DEMIBOLD);
			return NULL;

		case 9:
			if (_wcsicmp(lpszValue, L"ExtraBold") == 0) return CXTPMarkupEnum::CreateValue(FW_EXTRABOLD);
			if (_wcsicmp(lpszValue, L"UltraBold") == 0) return CXTPMarkupEnum::CreateValue(FW_ULTRABOLD);
			return NULL;

		case 10:
			if (_wcsicmp(lpszValue, L"ExtraLight") == 0) return CXTPMarkupEnum::CreateValue(FW_EXTRALIGHT);
			if (_wcsicmp(lpszValue, L"ExtraBlack") == 0) return CXTPMarkupEnum::CreateValue(950);
			return NULL;
		}

	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertFontQuality(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 9 && _wcsicmp(lpszValue, L"ClearType") == 0)
			return CXTPMarkupEnum::CreateValue(5);
	}
	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertLength(
	CXTPMarkupBuilder* pBuilder,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* pAssociatedProperty /*= NULL*/)
{
	ASSERT(NULL != pBuilder);
	ASSERT(NULL != pObject);

	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"Auto") == 0)
			return new CXTPMarkupInt(INT_MAX);

		int nValue = _wtoi(lpszValue);
		if (nValue == 0)
			return new CXTPMarkupInt(nValue);

		if ((nLength > 2) && _wcsicmp(lpszValue + nLength - 2, L"pt") == 0)
		{
			nValue = MulDiv(nValue, CXTPDpi::DefaultDpi, 72);
		}
		else if ((nLength > 2) && _wcsicmp(lpszValue + nLength - 2, L"in") == 0)
		{
			nValue *= CXTPDpi::DefaultDpi;
		}
		else if ((nLength > 2) && _wcsicmp(lpszValue + nLength - 2, L"cm") == 0)
		{
			nValue = int((double)nValue * 37.79528);
		}
		else if ((nLength > 2) && _wcsicmp(lpszValue + nLength - 2, L"mm") == 0)
		{
			nValue = int((double)nValue * 3.779528);
		}

		if (NULL != pAssociatedProperty)
		{
			if (0 != (pAssociatedProperty->GetFlags() & CXTPMarkupPropertyMetadata::flagHorzDpiSensible))
			{
				nValue = pBuilder->GetMarkupContext()->ScaleX(nValue);
			}
			else if (0 != (pAssociatedProperty->GetFlags() & CXTPMarkupPropertyMetadata::flagVertDpiSensible))
			{
				nValue = pBuilder->GetMarkupContext()->ScaleY(nValue);
			}
 		}

		return new CXTPMarkupInt(nValue);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertFontStyle(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 6 && _wcsicmp(lpszValue, L"Normal") == 0)
			return CXTPMarkupEnum::CreateValue(0);

		if (nLength == 6 && _wcsicmp(lpszValue, L"Italic") == 0)
			return CXTPMarkupEnum::CreateValue(1);
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////

CXTPMarkupObject* CXTPMarkupBuilder::ConvertVerticalAlignment(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 3 && _wcsicmp(lpszValue, L"Top") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupVerticalAlignmentTop);

		if (nLength == 6 && _wcsicmp(lpszValue, L"Center") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupVerticalAlignmentCenter);

		if (nLength == 6 && _wcsicmp(lpszValue, L"Bottom") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupVerticalAlignmentBottom);

		if (nLength == 7 && _wcsicmp(lpszValue, L"Stretch") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupVerticalAlignmentStretch);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertHorizontalAlignment(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"Left") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupHorizontalAlignmentLeft);

		if (nLength == 6 && _wcsicmp(lpszValue, L"Center") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupHorizontalAlignmentCenter);

		if (nLength == 5 && _wcsicmp(lpszValue, L"Right") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupHorizontalAlignmentRight);

		if (nLength == 7 && _wcsicmp(lpszValue, L"Stretch") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupHorizontalAlignmentStretch);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertTextWrapping(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"Wrap") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextWrap);

		if (nLength == 16 && _wcsicmp(lpszValue, L"WrapWithOverflow") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextWrapWithOverflow);

		if (nLength == 6 && _wcsicmp(lpszValue, L"NoWrap") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextNoWrap);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertTextTrimming(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"None") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextTrimmingNone);

		if (nLength == 17 && _wcsicmp(lpszValue, L"CharacterEllipsis") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextTrimmingCharacterEllipsis);

		if (nLength == 12 && _wcsicmp(lpszValue, L"WordEllipsis") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextTrimmingWordEllipsis);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertTextAlignment(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 6 && _wcsicmp(lpszValue, L"Center") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextAlignmentCenter);

		if (nLength == 4 && _wcsicmp(lpszValue, L"Left") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextAlignmentLeft);

		if (nLength == 5 &&_wcsicmp(lpszValue, L"Right") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextAlignmentRight);

		if (nLength == 7 &&_wcsicmp(lpszValue, L"Justify") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupTextAlignmentJustify);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertOrientation(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 10 && _wcsicmp(lpszValue, L"Horizontal") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupOrientationHorizontal);

		if (nLength == 8 && _wcsicmp(lpszValue, L"Vertical") == 0)
			return CXTPMarkupEnum::CreateValue(xtpMarkupOrientationVertical);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertBaselineAlignment(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 3 && _wcsicmp(lpszValue, L"Top") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineTop);
		if (nLength == 6 && _wcsicmp(lpszValue, L"Center") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineCenter);
		if (nLength == 6 && _wcsicmp(lpszValue, L"Bottom") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineBottom);
		if (nLength == 8 && _wcsicmp(lpszValue, L"Baseline") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaseline);
		if (nLength == 7 && _wcsicmp(lpszValue, L"TextTop") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineTextTop);
		if (nLength == 10 && _wcsicmp(lpszValue, L"TextBottom") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineTextBottom);
		if (nLength == 9 && _wcsicmp(lpszValue, L"Subscript") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineSubscript);
		if (nLength == 11 && _wcsicmp(lpszValue, L"Superscript") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupBaselineSuperscript);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertCursor(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"None") == 0) return new CXTPMarkupInt(0);
		if (nLength == 2 && _wcsicmp(lpszValue, L"No") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_NO);
		if (nLength == 5 && _wcsicmp(lpszValue, L"Arrow") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_ARROW);
		if (nLength == 11 && _wcsicmp(lpszValue, L"AppStarting") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_APPSTARTING);
		if (nLength == 5 && _wcsicmp(lpszValue, L"Cross") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_CROSS);
		if (nLength == 5 && _wcsicmp(lpszValue, L"IBeam") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_IBEAM);
		if (nLength == 6 && _wcsicmp(lpszValue, L"SizeAll") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_SIZEALL);
		if (nLength == 8 && _wcsicmp(lpszValue, L"SizeNESW") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_SIZENESW);
		if (nLength == 6 && _wcsicmp(lpszValue, L"SizeNS") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_SIZENS);
		if (nLength == 8 && _wcsicmp(lpszValue, L"SizeNWSE") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_SIZENWSE);
		if (nLength == 6 && _wcsicmp(lpszValue, L"SizeWE") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_SIZEWE);
		if (nLength == 7 && _wcsicmp(lpszValue, L"UpArrow") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_UPARROW);
		if (nLength == 4 && _wcsicmp(lpszValue, L"Wait") == 0) return new CXTPMarkupInt((DWORD)(DWORD_PTR)IDC_WAIT);
		if (nLength == 4 && _wcsicmp(lpszValue, L"Hand") == 0) return new CXTPMarkupInt((DWORD)32649);
	}

	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertVisibility(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 7 && _wcsicmp(lpszValue, L"Visible") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupVisibilityVisible);
		if (nLength == 6 && _wcsicmp(lpszValue, L"Hidden") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupVisibilityHidden);
		if (nLength == 9 && _wcsicmp(lpszValue, L"Collapsed") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupVisibilityCollapsed);
	}
	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertContent(
	CXTPMarkupBuilder* pBuilder,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		CXTPMarkupRun* pRun = MARKUP_CREATE(CXTPMarkupRun, pBuilder->GetMarkupContext());
		pRun->SetText((CXTPMarkupString*)pObject);
		MARKUP_ADDREF(pObject);

		CXTPMarkupTextBlock* pTextBlock = MARKUP_CREATE(CXTPMarkupTextBlock, pBuilder->GetMarkupContext());
		pTextBlock->GetInlines()->Add(pRun);

		return pTextBlock;
	}
	return pObject;

}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertScrollBarVisibility(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"Auto") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupScrollBarAuto);
		if (nLength == 7 && _wcsicmp(lpszValue, L"Visible") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupScrollBarVisible);
		if (nLength == 8 && _wcsicmp(lpszValue, L"Disabled") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupScrollBarDisabled);
	}
	return NULL;
}

CXTPMarkupObject* CXTPMarkupBuilder::ConvertStretch(
	CXTPMarkupBuilder* /*pBuilder*/,
	CXTPMarkupObject* pObject,
	CXTPMarkupDependencyProperty* /*pAssociatedProperty = NULL*/)
{
	if (IsStringObject(pObject))
	{
		LPCWSTR lpszValue = *((CXTPMarkupString*)pObject);
		int nLength = ((CXTPMarkupString*)pObject)->GetLength();

		if (nLength == 4 && _wcsicmp(lpszValue, L"None") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupStretchNone);
		if (nLength == 4 && _wcsicmp(lpszValue, L"Fill") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupStretchFill);
		if (nLength == 7 && _wcsicmp(lpszValue, L"Uniform") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupStretchUniform);
		if (nLength == 13 && _wcsicmp(lpszValue, L"UniformToFill") == 0) return CXTPMarkupEnum::CreateValue(xtpMarkupStretchUniformToFill);
	}
	return NULL;
}

BOOL AFX_CDECL CXTPMarkupBuilder::ConvertDouble(LPCWSTR& lpszValue, double& dValue, TCHAR cStop, BOOL bCheckTail)
{
	if (!lpszValue)
		return FALSE;

	while (*lpszValue == ' ') lpszValue++;

	dValue = 0;
	BOOL bSign = FALSE;
	if (*lpszValue == '-')
	{
		bSign = TRUE;
		lpszValue++;
	}
	else if (*lpszValue == '+')
	{
		lpszValue++;
	}

	LPCWSTR lpszNext = lpszValue;

	if (*lpszNext == 0)
		return FALSE;

	BOOL bDecimal = 0;
	double dDecimal = 0;

	while (*lpszNext != 0)
	{
		WCHAR c = *lpszNext;

		if (c == ' ' || c == cStop)
		{
			if (lpszNext == lpszValue)
				return FALSE;

			if (c == ' ')
			{
				while (*lpszNext == ' ') lpszNext++;
			}

			if (cStop != 0 && *lpszNext == cStop)
				lpszNext++;
			break;
		}

		if (c == '.')
		{
			if (bDecimal)
				return FALSE;
			bDecimal = TRUE;
			dDecimal = 1;

			if (lpszValue == lpszNext)
				lpszValue = ++lpszNext;
			else lpszNext++;

			continue;
		}

		if (c <'0' || c > '9')
			return FALSE;

		if (!bDecimal)
		{
			dValue = 10 * dValue + (c - '0');
		}
		else
		{
			dDecimal /= 10;
			dValue += dDecimal * double(c - '0');
		}

		lpszNext++;
	}

	if (lpszNext == lpszValue)
		return FALSE;

	lpszValue = lpszNext;
	while (*lpszValue == ' ') lpszValue++;

	if (bCheckTail && lpszValue[0] != 0)
		return FALSE;

	if (bSign) dValue = -dValue;

	return TRUE;
}

void CXTPMarkupBuilder::ThrowBuilderException(LPCTSTR lpszError)
{
	THROW(new CXTPMarkupBuilderException(lpszError));
}

void CXTPMarkupBuilder::ThrowBuilderException(HRESULT hr)
{
	ThrowBuilderException(_com_error(hr).ErrorMessage());
}
