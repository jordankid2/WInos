// XTPReportRows.cpp : implementation of the CXTPReportRows class.
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
#include "Common/XTPCustomHeap.h"
#include "Common/XTPSmartPtrInternalT.h"

#include "ReportControl/XTPReportDefines.h"
#include "ReportControl/XTPReportRow.h"
#include "ReportControl/XTPReportGroupRow.h"
#include "ReportControl/XTPReportRecord.h"
#include "ReportControl/XTPReportRecords.h"
#include "ReportControl/XTPReportRecordItem.h"
#include "ReportControl/XTPReportControl.h"
#include "ReportControl/XTPReportColumns.h"
#include "ReportControl/XTPReportColumn.h"
#include "ReportControl/XTPReportRows.h"
#include "ReportControl/ItemTypes/XTPReportRecordItemVariant.h"

#include "ReportControl/XTPReportControlIIDs.h"

#include "Common/Base/Diagnostic/XTPDisableNoisyWarnings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////
// CXTPReportRowsArray

void CXTPReportRowsArray::ReserveSize(INT_PTR nNewSize, INT_PTR nGrowBy /*= -1*/)
{
	if (GetSize() != 0)
	{
		ASSERT(FALSE);
		return;
	}
	
	SetSize(1, nNewSize);
	Add(NULL);
	SetSize(1, nGrowBy < 0 ? 0 : nGrowBy);
	
	m_nSize = 0;
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportScreenRows

CXTPReportScreenRows::CXTPReportScreenRows()
{
}

CXTPReportScreenRows::~CXTPReportScreenRows()
{
	Clear();
}

void CXTPReportScreenRows::Clear()
{
	// array cleanup
	for (int nRow = 0; nRow < GetSize(); nRow++)
	{
		CXTPReportRow* pRow = GetAt(nRow);
		pRow->InternalRelease();
	}

	RemoveAll();
}

CXTPReportRow* CXTPReportScreenRows::HitTest(CPoint pt) const
{
	for (int nRow=0; nRow<GetSize(); nRow++)
	{
		CXTPReportRow *pRow = GetAt(nRow);

		if (pRow->GetRect().PtInRect(pt))
		{
			return pRow;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportRows

CXTPReportRows::CXTPReportRows()
	: m_nFocusedRow(-1)
{
#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
	m_pVirtualRow = 0;
	m_nVirtualRowsCount = 0;
	m_pScreenRows = NULL;
}

CXTPReportRows::~CXTPReportRows()
{
	Clear();
	SAFE_DELETE(m_pScreenRows);
}

void CXTPReportRows::_swapIndexes(int& A, int& B)
{
	int C = B;
	B = A;
	A = C;
}

void CXTPReportRows::_clampIndexes(int& nOrig, const int& nMin, const int& nMax)
{
	nOrig = max(nOrig, nMin);
	nOrig = min(nOrig, nMax);
}

//////////////////////////////////////////////////////////////////////////
// CXTPReportRows member functions

CXTPReportScreenRows *CXTPReportRows::GetScreenRows()
{
	if (NULL == m_pScreenRows)
	{
		// Create on demand
		m_pScreenRows = new CXTPReportScreenRows();
	}
	return m_pScreenRows;
}

BOOL CXTPReportRows::Contains(const CXTPReportRow *pRow) const
{
	for (int nRow=0; nRow<m_arrRows.GetSize(); nRow++)
	{
		if (pRow == m_arrRows.GetAt(nRow))
		{
			return TRUE;
		}
	}

	return FALSE;
}

int CXTPReportRows::GetFocusedRowIndex() const
{
	return m_nFocusedRow;
}

BOOL CXTPReportRows::HasFocus() const
{
	return (-1 != m_nFocusedRow);
}

void CXTPReportRows::SetFocusedRowIndex(int nRow)
{
	m_nFocusedRow = nRow;
}


CXTPReportRow *CXTPReportRows::GetFocusedRow()
{
	CXTPReportRow *pFocusedRow = NULL;

	if (-1 != m_nFocusedRow)
	{
		pFocusedRow = GetAt(m_nFocusedRow);
	}

	return pFocusedRow;
}

void CXTPReportRows::Clear(BOOL bResetRow)
{
	// array cleanup
	for (int nRow = (int) m_arrRows.GetSize() - 1; nRow >= 0; nRow--)
	{
		CXTPReportRow* pRow = m_arrRows.GetAt(nRow);
		if (bResetRow)
		{
			pRow->SetVisible(FALSE);
			pRow->m_nIndex = -1;
		}
		pRow->InternalRelease();
	}

	m_arrRows.RemoveAll();

	if (m_pVirtualRow)
	{
		m_pVirtualRow->InternalRelease();
		m_pVirtualRow = NULL;
	}

	GetScreenRows()->Clear();

	m_nFocusedRow = -1;
}

CXTPReportRow* CXTPReportRows::GetNext(CXTPReportRow* pRow, BOOL bSkipGroupFocus)
{
	if (pRow)
	{
		int index = pRow->GetIndex();

		while (index < GetCount() - 1)
		{
			index++;
			pRow = GetAt(index);

			if (!bSkipGroupFocus || !pRow->IsGroupRow() || !pRow->IsExpanded())
				return pRow;
		}
	}
	else if (GetCount() > 0)
	{
		return GetAt(0);
	}
	return pRow;
}

CXTPReportRow* CXTPReportRows::GetPrev(CXTPReportRow* pRow, BOOL bSkipGroupFocus)
{
	if (pRow)
	{
		int index = pRow->GetIndex();

		while (index > 0)
		{
			index--;
			pRow = GetAt(index);

			if (!bSkipGroupFocus || !pRow->IsGroupRow() || !pRow->IsExpanded())
				return pRow;
		}
	}
	else if (GetCount() > 0)
	{
		return GetAt(0);
	}
	return pRow;
}

void CXTPReportRows::InsertAt(int nIndex, CXTPReportRow* pRow)
{
	m_arrRows.InsertAt(nIndex, pRow);
}

int CXTPReportRows::Add(CXTPReportRow* pRow)
{
	int nIndex = (int)m_arrRows.Add(pRow);

	pRow->m_nChildIndex = nIndex;
	pRow->m_pParentRows = this;

	return nIndex;
}

void CXTPReportRows::RemoveAt(int nIndex)
{
	m_arrRows.GetAt(nIndex)->InternalRelease();
	m_arrRows.RemoveAt(nIndex);
}

int CXTPReportRows::RemoveRow(CXTPReportRow* pRow)
{
	for (int i = 0; i < (int) m_arrRows.GetSize(); i++)
	{
		if (m_arrRows.GetAt(i) == pRow)
		{
			RemoveAt(i);
			return i;
		}
	}
	return -1;
}

void CXTPReportRows::SetVirtualMode(CXTPReportRow* pRow, int nCount)
{
	ASSERT(m_pVirtualRow == NULL);

	m_pVirtualRow = pRow;
	m_nVirtualRowsCount = nCount;
}

int AFX_CDECL CXTPReportRows::CompareRows(const CXTPReportRow** ppRow1, const CXTPReportRow** ppRow2)
{
	CXTPReportColumns* pColumns = (**ppRow1).GetControl()->GetColumns();

	CXTPReportRecord* pRecord1 = (**ppRow1).GetRecord();
	CXTPReportRecord* pRecord2 = (**ppRow2).GetRecord();

	ASSERT(pColumns);
	ASSERT(pRecord1 && pRecord2 || !pRecord1 && !pRecord2);

	if (pRecord1 == pRecord2 && pRecord1)
		return 0;

	if (!pRecord1 && !pRecord2)
	{
		// compare groupRows by its first not group child rows
		if ((*ppRow1)->HasChildren() && (*ppRow2)->HasChildren())
		{
			CXTPReportRow* pRow1a = (CXTPReportRow*)(*ppRow1);
			CXTPReportRow* pRow2a = (CXTPReportRow*)(*ppRow2);

			if (pRow1a->GetChilds() && pRow1a->GetChilds()->GetCount() &&
				pRow2a->GetChilds() && pRow2a->GetChilds()->GetCount())
			{
				pRow1a = pRow1a->GetChilds()->GetAt(0);
				pRow2a = pRow2a->GetChilds()->GetAt(0);

				return CompareRows((const CXTPReportRow**)&pRow1a, (const CXTPReportRow**)&pRow2a);
			}
		}
	}

	if (!pRecord1 || !pRecord2)
	{
		ASSERT(FALSE);
		return 0;
	}

	for (int nGroupOrder = 0; nGroupOrder < pColumns->GetGroupsOrder()->GetCount(); nGroupOrder++)
	{
		CXTPReportColumn* pColumn = pColumns->GetGroupsOrder()->GetAt(nGroupOrder);
		if (!pColumn->m_bAutoSortWhenGrouped)
			continue;

		BOOL bIncreasing = pColumn->m_bSortIncreasing;

		CXTPReportRecordItem* pItem1 = pRecord1->GetItem(pColumn);
		CXTPReportRecordItem* pItem2 = pRecord2->GetItem(pColumn);

		if (!pItem1 || !pItem2)
			continue;

		int nCompareResult = pItem1->CompareGroupCaption(pColumn, pItem2);

		if (nCompareResult != 0)
			return nCompareResult * (bIncreasing ? 1 : -1);
	}


	for (int nSortOrder = 0; nSortOrder < pColumns->GetSortOrder()->GetCount(); nSortOrder++)
	{
		CXTPReportColumn* pColumn = pColumns->GetSortOrder()->GetAt(nSortOrder);
		BOOL bIncreasing = pColumn->IsSortedIncreasing();

		CXTPReportRecordItem* pItem1 = pRecord1->GetItem(pColumn);
		CXTPReportRecordItem* pItem2 = pRecord2->GetItem(pColumn);

		if (!pItem1 || !pItem2)
			continue;

		int nCompareResult = pItem1->Compare(pColumn, pItem2);

		if (nCompareResult != 0)
			return nCompareResult * (bIncreasing ? 1 : -1);
	}

	return pRecord1->GetIndex() > pRecord2->GetIndex() ? 1 : -1;
}

int AFX_CDECL CXTPReportRows::CompareRows2(const CXTPReportRow** ppRow1, const CXTPReportRow** ppRow2)
{
	CXTPReportColumns* pColumns = (**ppRow1).GetControl()->GetColumns();

	CXTPReportRecord* pRecord1 = (**ppRow1).GetRecord();
	CXTPReportRecord* pRecord2 = (**ppRow2).GetRecord();

	ASSERT(pColumns);
	ASSERT(pRecord1 && pRecord2 || !pRecord1 && !pRecord2);

	if (pRecord1 == pRecord2 && pRecord1)
		return 0;

	if (!pRecord1 && !pRecord2)
	{
		// compare groupRows by its first not group child rows
		if ((*ppRow1)->HasChildren() && (*ppRow2)->HasChildren())
		{
			CXTPReportRow* pRow1a = (CXTPReportRow*)(*ppRow1);
			CXTPReportRow* pRow2a = (CXTPReportRow*)(*ppRow2);

			if (pRow1a->GetChilds() && pRow1a->GetChilds()->GetCount() &&
				pRow2a->GetChilds() && pRow2a->GetChilds()->GetCount())
			{
				pRow1a = pRow1a->GetChilds()->GetAt(0);
				pRow2a = pRow2a->GetChilds()->GetAt(0);

				return CompareRows((const CXTPReportRow**)&pRow1a, (const CXTPReportRow**)&pRow2a);
			}
		}
	}

	if (!pRecord1 || !pRecord2)
	{
		ASSERT(FALSE);
		return 0;
	}

	for (int nGroupOrder = 0; nGroupOrder < pColumns->GetGroupsOrder()->GetCount(); nGroupOrder++)
	{
		CXTPReportColumn* pColumn = pColumns->GetGroupsOrder()->GetAt(nGroupOrder);
		if (!pColumn->m_bAutoSortWhenGrouped)
			continue;

		BOOL bIncreasing = pColumn->m_bSortIncreasing;

		CXTPReportRecordItem* pItem1 = pRecord1->GetItem(pColumn);
		CXTPReportRecordItem* pItem2 = pRecord2->GetItem(pColumn);

		if (!pItem1 || !pItem2)
			continue;

		int nCompareResult = pItem1->CompareGroupCaption(pColumn, pItem2);

		if (nCompareResult != 0)
		{
			if (nGroupOrder > 0)
				nCompareResult = bIncreasing ? -1 : 1;
			return nCompareResult * (bIncreasing ? 1 : -1);
		}
	}

	for (int nSortOrder = 0; nSortOrder < pColumns->GetSortOrder()->GetCount(); nSortOrder++)
	{
		CXTPReportColumn* pColumn = pColumns->GetSortOrder()->GetAt(nSortOrder);
		BOOL bIncreasing = pColumn->IsSortedIncreasing();

		CXTPReportRecordItem* pItem1 = pRecord1->GetItem(pColumn);
		CXTPReportRecordItem* pItem2 = pRecord2->GetItem(pColumn);

		if (!pItem1 || !pItem2)
			continue;

		int nCompareResult = pItem1->Compare(pColumn, pItem2);

		if (nCompareResult != 0)
			return nCompareResult * (bIncreasing ? 1 : -1);
	}

	if (pRecord1->GetIndex() == pRecord2->GetIndex())
	{
		ASSERT(FALSE);
		return 0;
	}
	return pRecord1->GetIndex() < pRecord2->GetIndex() ? -1 : 1;
}

void CXTPReportRows::SortEx(XTPReportRowsCompareFunc pCompareFunc)
{
	typedef int (AFX_CDECL *GENERICCOMPAREFUNC)(const void *, const void*);

	qsort(m_arrRows.GetData(), (size_t) m_arrRows.GetSize(), sizeof(CXTPReportRow*), (GENERICCOMPAREFUNC)pCompareFunc);
}

void CXTPReportRows::Sort()
{
	SortEx(CompareRows);
}

int CXTPReportRows::GetCount() const
{
	int nCount = 0;

	if (NULL != m_pVirtualRow)
	{
		nCount = m_nVirtualRowsCount;
	}
	else
	{
		nCount = (int)m_arrRows.GetSize();
	}

	return nCount;
}

CXTPReportRow* CXTPReportRows::GetAt(int nIndex) const
{
	CXTPReportRow *pRow = NULL;

	if (NULL != m_pVirtualRow)
	{
		m_pVirtualRow->m_nIndex = nIndex;
		pRow = m_pVirtualRow;
	}
	else
	{
		const int nCount = GetCount();

		if ((nIndex >= 0) && (nIndex < nCount))
		{
			pRow = m_arrRows.GetAt(nIndex);
		}
	}

	return pRow;
}

CXTPReportRow* CXTPReportRows::Find(CXTPReportRecord *pRecord, BOOL bRecursive)
{
	int nSize = (int)m_arrRows.GetSize();

	for (int nRow=0; nRow<nSize; nRow++)
	{
		CXTPReportRow *pRow = m_arrRows.GetAt(nRow);

		if (pRow->GetRecord() == pRecord)
		{
			return pRow;
		}

		if (pRow->HasChildren() && bRecursive)
		{
			pRow = pRow->GetChilds()->Find(pRecord, TRUE);

			if (NULL != pRow)
			{
				return pRow;
			}
		}
	}

	return NULL;
}

CXTPReportRow* CXTPReportRows::Find(CXTPReportRecord *pRecord)
{
	return Find(pRecord, FALSE);
}

CXTPReportRow* CXTPReportRows::FindInTree(CXTPReportRecord *pRecord)
{
	return Find(pRecord, TRUE);
}

CXTPReportRow* CXTPReportRows::FindInsertionPos(CXTPReportRow* pRow, BOOL& bInsertAfter)
{
	CXTPReportRow* pInsertionRowPos = NULL;
	bInsertAfter = FALSE;

	INT_PTR nCount = m_arrRows.GetSize();
	for (INT_PTR i = 0; i < nCount; i++)
	{
		CXTPReportRow* pCurRow = m_arrRows.GetAt(i);
		if (pCurRow->IsGroupRow())
		{
			// Compare group caption
			CXTPReportRecord* pRecord = pRow->GetRecord();
			CXTPReportColumns* pColumns = pRow->GetControl()->GetColumns();
			if (!(pRecord && pColumns))
				continue;

			CXTPReportColumn* pColumn = pColumns->GetGroupsOrder()->GetAt(pCurRow->GetGroupLevel());
			CXTPReportRecordItem* pItem = pRecord->GetItem(pColumn);
			CXTPReportGroupRow* pGroupRow = DYNAMIC_DOWNCAST(CXTPReportGroupRow, pCurRow);
			if (!XTP_ASSERT_CHECK(NULL != pGroupRow))
				continue;

			int nCmpResult = pItem->CompareGroupCaption(pColumn, pGroupRow);

			if (pColumn->m_bSortIncreasing ? 0 < nCmpResult : nCmpResult < 0)
				continue;

			// Find in children
			if (!pCurRow->GetChilds())
				continue;

			if (0 == nCmpResult)
				pInsertionRowPos = pCurRow->GetChilds()->FindInsertionPos(pRow, bInsertAfter);

			if (!pInsertionRowPos)
			{
				if (0 != nCmpResult)
				{
					pInsertionRowPos = pCurRow;
				}
				else
				{
					pInsertionRowPos = pCurRow->GetChilds()->GetAt(pCurRow->GetChilds()->GetCount() - 1);
					bInsertAfter = TRUE;
				}
			}

			break;
		}

		if (CompareRows2((const CXTPReportRow**)&pRow, (const CXTPReportRow**)&pCurRow) <= 0)
		{
			pInsertionRowPos = pCurRow;
			break;
		}
	}

	return pInsertionRowPos;
}

void CXTPReportRows::RefreshChildIndices(BOOL bRunInChildren)
{
	for (int nIndex = 0; nIndex < GetCount(); nIndex++)
	{
		CXTPReportRow* pRow = GetAt(nIndex);
		pRow->m_nChildIndex = nIndex;
		ASSERT(pRow->m_pParentRows == this);
		if (bRunInChildren && pRow->HasChildren() && pRow->GetChilds())
			pRow->GetChilds()->RefreshChildIndices(bRunInChildren);
	}
}

void CXTPReportRows::ClearMergeHeight()
{
	if (m_pVirtualRow != NULL || 
		XTP_SAFE_GET2( GetAt(0), GetControl(), IsCellMergingAllowed(), TRUE ) == FALSE)
	{
		return;
	}

	int nCount = GetCount();
	for (int i = 0; i < nCount; i++)
	{
		CXTPReportRow* pRow = GetAt(i);
		if (pRow)
			pRow->SetMergeHeight(-1);
	}
}

void CXTPReportRows::SetSize(INT_PTR nNewSize, INT_PTR nGrowBy)
{
	int nSize = GetCount();
	if (nNewSize < nSize)
	{
		for (int i = (int)nNewSize; i < nSize; i++)
		{
			CXTPReportRow* pRow = GetAt(i);
			if (pRow)
				pRow->InternalRelease();
		}
	}

	m_arrRows.SetSize(nNewSize, nGrowBy);
}

void CXTPReportRows::ReserveSize(INT_PTR nNewSize, INT_PTR nGrowBy)
{
	m_arrRows.ReserveSize(nNewSize, nGrowBy);
}

void CXTPReportRows::SetAt(INT_PTR nIndex, CXTPReportRow* pRow)
{
	ASSERT(pRow);
	ASSERT(nIndex >= 0 && nIndex < GetCount());

	if (!pRow || nIndex < 0 || nIndex >= GetCount())
		return;

	CXTPReportRow* pRow_prev = GetAt((int)nIndex);
	if (pRow_prev)
		pRow_prev->InternalRelease();

	pRow->m_nChildIndex = (int)nIndex;
	pRow->m_pParentRows = this;

	m_arrRows.SetAt(nIndex, pRow);
}

CXTPReportRecordItem* CXTPReportRows::FindRecordItemByRows(int nStartIndex, int nEndIndex,
												int nStartColumn, int nEndColumn,
												int nRecordIndex, int nItem,
												LPCTSTR pcszText, int nFlags)
{
	if (GetCount() < 1 || GetAt(0) == NULL)
		return NULL;
	
// validate parameters
	BOOL bExactPhrase	= (nFlags & xtpReportTextSearchExactPhrase)	> 0;
	BOOL bMatchCase		= (nFlags & xtpReportTextSearchMatchCase)	> 0;
	BOOL bBackward		= (nFlags & xtpReportTextSearchBackward)	> 0;
	BOOL bExactStart	= (nFlags & xtpReportTextSearchExactStart)	> 0;
	
	CString sFind(pcszText);
	if (!bMatchCase)
		sFind.MakeUpper();

	CXTPReportRow* pRow = GetAt(0);
	CXTPReportColumns* pColumns = pRow->GetControl()->GetColumns();
	int nStep = bBackward ? -1 : 1;
	
	_clampIndexes(nStartIndex, 0, GetCount() - 1);
	_clampIndexes(nEndIndex,   0, GetCount() - 1);

	if (nEndIndex < nStartIndex)
		_swapIndexes(nStartIndex, nEndIndex);
	
	_clampIndexes(nStartColumn, 0, pColumns->GetCount() - 1);
	_clampIndexes(nEndColumn,   0, pColumns->GetCount() - 1);
	
	if (nEndColumn < nStartColumn)
		_swapIndexes(nStartColumn, nEndColumn);

	_clampIndexes(nRecordIndex, nStartIndex, nEndIndex);
	_clampIndexes(nItem, nStartColumn, nEndColumn);

	if (bBackward)
	{
		_swapIndexes(nStartIndex, nEndIndex);
		_swapIndexes(nStartColumn, nEndColumn);
	}
	
//searching
	for (int i = nRecordIndex; bBackward ? (i >= nEndIndex) : (i <= nEndIndex); i += nStep)
	{
		CXTPReportRow* pTryRow = GetAt(i);
		CXTPReportRecord* pTryRecord = pTryRow->GetRecord();
		if (pTryRecord == NULL)
			continue;
			
		for (int j = nStartColumn; bBackward ? (j >= nEndColumn) : (j <= nEndColumn); j += nStep)
		{
			if (i == nRecordIndex && (bBackward ? (j > nItem) : (j < nItem)))
				continue;

			CXTPReportRecordItem* pTryItem = pTryRecord->GetItem(j);
			if (pTryItem == NULL)
				continue;
				
			CString strCaption = pTryItem->GetCaption(NULL);
			if (strCaption.IsEmpty())
				continue;
			
			if (!bMatchCase)
				strCaption.MakeUpper();
			
			if (bExactPhrase)	//includes case bExactStart
			{
				if(strCaption == sFind)
					return pTryItem;
				else
					continue;
			}
			
			int k = strCaption.Find(sFind);
				
			if (bExactStart)
			{
				if (k == 0)
					return pTryItem;
				else
					continue;
			}
			
			if (k > -1)	// !bExactPhrase && !bExactStart
				return pTryItem;
		}
	}

	return NULL;
}



#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPReportRows, CXTPCmdTarget)
	DISP_FUNCTION_ID(CXTPReportRows, "Count", dispidCount, OleGetItemCount, VT_I4, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRows, "Row", DISPID_VALUE, OleGetItem, VT_DISPATCH, VTS_I4)
	DISP_FUNCTION_ID(CXTPReportRows, "_NewEnum", DISPID_NEWENUM, OleNewEnum, VT_UNKNOWN, VTS_NONE)
	DISP_FUNCTION_ID(CXTPReportRows, "FindRow", 2, OleFindRow, VT_DISPATCH, VTS_DISPATCH)
	DISP_FUNCTION_ID(CXTPReportRows, "FindRowInTree", 3, OleFindRowInTree, VT_DISPATCH, VTS_DISPATCH)
	DISP_FUNCTION_ID(CXTPReportRows, "FindRecordItemByRows", 4, OleFindRecordItemByRows, VT_DISPATCH, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_BSTR VTS_I4)
END_DISPATCH_MAP()

BEGIN_INTERFACE_MAP(CXTPReportRows, CXTPCmdTarget)
	INTERFACE_PART(CXTPReportRows, XTPDIID_IReportRows, Dispatch)
	//INTERFACE_PART(CXTPReportRows, IID_IEnumVARIANT, EnumVARIANT)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPReportRows, XTPDIID_IReportRows)
IMPLEMENT_ENUM_VARIANT(CXTPReportRows)

int CXTPReportRows::OleGetItemCount()
{
	return GetCount();
};

LPDISPATCH CXTPReportRows::OleGetItem(long nIndex)
{
	if (nIndex >= 0 && nIndex < GetCount())
	{
		return GetAt(nIndex)->GetIDispatch(TRUE);
	}
	AfxThrowOleException(E_INVALIDARG);
	return 0;
}

LPDISPATCH CXTPReportRows::OleFindRow(LPDISPATCH pRecordDisp)
{
	CXTPReportRecord* pRecord = DYNAMIC_DOWNCAST(CXTPReportRecord, 
		CXTPReportRecord::FromIDispatchSafe(pRecordDisp));
	if (!pRecord)
	{
		AfxThrowOleException(E_INVALIDARG);
	}

	CXTPReportRow* pRow = Find(pRecord);
	if (pRow)
	{
		return pRow->GetIDispatch(TRUE);
	}

	return NULL;
}

LPDISPATCH CXTPReportRows::OleFindRowInTree(LPDISPATCH pRecordDisp)
{
	CXTPReportRecord* pRecord = DYNAMIC_DOWNCAST(CXTPReportRecord, 
		CXTPReportRecord::FromIDispatchSafe(pRecordDisp));
	if (!pRecord)
		AfxThrowOleException(E_INVALIDARG);

	CXTPReportRow* pRow = FindInTree(pRecord);

	if (pRow)
		return pRow->GetIDispatch(TRUE);

	return NULL;
}

LPDISPATCH CXTPReportRows::OleFindRecordItemByRows(int nStartIndex, int nEndIndex,
												int nStartColumn, int nEndColumn,
												int nRecord, int nItem,
												LPCTSTR pcszText, int nFlags)
{
	CXTPReportRecordItem* pItem = FindRecordItemByRows(nStartIndex, nEndIndex, nStartColumn, nEndColumn, nRecord, nItem, pcszText, nFlags);
	if (pItem)
		return pItem->GetIDispatch(TRUE);
	return NULL;
}


#endif // _XTP_ACTIVEX
