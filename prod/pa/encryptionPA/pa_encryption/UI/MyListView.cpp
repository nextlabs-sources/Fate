#include "stdafx.h"

#include "MyListView.h"

LRESULT CMyListView::OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;
	UNUSED(wParam);
	UNUSED(bHandled);

	if (uMsg != OCM_DRAWITEM)
	{
		return S_FALSE;
	}

	if (lpDrawItem->CtlType == ODT_LISTVIEW)
	{
		if (lpDrawItem->itemAction & ODA_DRAWENTIRE)
		{
			WCHAR wzFilePath[MAX_PATH+1];

			lpDrawItem->rcItem.left += 2;
			lpDrawItem->rcItem.top += 2;

			// fix bug7586 to forbid select item
// 			COLORREF clrHighlight;//fix bug7586 Kevin Zhou 2008-10-6
// 			int state = ListView_GetItemState(lpDrawItem->hwndItem, lpDrawItem->itemID, LVIS_FOCUSED | LVIS_SELECTED);
// 			if (state & LVIS_SELECTED)
// 			{
// 				COLORREF color = GetSysColor(COLOR_HIGHLIGHT);
// 				HBRUSH hbrSEL = CreateSolidBrush(color);
// 				FillRect(lpDrawItem->hDC, &(lpDrawItem->rcItem), hbrSEL);
// 				DeleteObject (hbrSEL);
// 				clrHighlight = RGB(255, 255, 255);//fix bug7586
// 			}
// 			else
// 			{
// 				COLORREF color = GetSysColor(COLOR_BTNFACE);
// 				HBRUSH hbrBkColor = CreateSolidBrush(color);
// 				FillRect(lpDrawItem->hDC, &(lpDrawItem->rcItem), hbrBkColor);
// 				DeleteObject (hbrBkColor);
// 				clrHighlight = RGB(0, 0, 0);//fix bug7586
// 			}

			memset(wzFilePath, 0, sizeof(wzFilePath));
			ListView_GetItemText(lpDrawItem->hwndItem, lpDrawItem->itemID, 0, wzFilePath, MAX_PATH);

			HIMAGELIST hImglist = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);
			ImageList_Draw(hImglist, lpDrawItem->itemID, lpDrawItem->hDC, 
				lpDrawItem->rcItem.left, lpDrawItem->rcItem.top, ILD_NORMAL | ILD_TRANSPARENT);

			lpDrawItem->rcItem.left += 16;
// 			COLORREF oldColor = SetTextColor(lpDrawItem->hDC, clrHighlight);//fix bug7586

			DrawTextW(lpDrawItem->hDC, wzFilePath, wcslen(wzFilePath),
				&(lpDrawItem->rcItem), DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX); // fix bug374

// 			SetTextColor(lpDrawItem->hDC, oldColor);//fix bug7586
		}
// 		if (lpDrawItem->itemAction & ODA_FOCUS)
// 		{
// 			int iState = lpDrawItem->itemState;
// 		}
// 		if (lpDrawItem->itemAction & ODA_SELECT)
// 		{
// 			int iState = lpDrawItem->itemState;
// 		}
	}

	return S_OK;
}

// Generic creator
BOOL CMyListView::Create(DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID)
{
// 	m_hWnd = ::CreateWindowW(WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
// 		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);

	CWindowImpl<CMyListView>::Create(hParentWnd, _U_RECT((LPRECT)&rect), WC_LISTVIEW, dwStyle, 0, _U_MENUorID((HMENU)(UINT_PTR)nID));

	return (m_hWnd == NULL) ? FALSE : TRUE;
}

// Generic creator allowing extended style bits
BOOL CMyListView::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
			  HWND hParentWnd, UINT nID)
{
// 	m_hWnd = ::CreateWindowExW(dwExStyle, WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
// 		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);
	CWindowImpl<CMyListView>::Create(hParentWnd, _U_RECT((LPRECT)&rect), WC_LISTVIEW, dwStyle, dwExStyle, _U_MENUorID((HMENU)(UINT_PTR)nID));

	return (m_hWnd == NULL) ? FALSE : TRUE;
}


// Attributes
// Retrieves the background color for the control.  
COLORREF CMyListView::GetBkColor() const
{
	return ListView_GetBkColor(m_hWnd);
}

// Sets background color for the control.
BOOL CMyListView::SetBkColor(COLORREF cr)
{
	return ListView_SetBkColor(m_hWnd, cr);
}

// Retrieves the image list associated with the control.
HIMAGELIST CMyListView::GetImageList(int nImageList) const
{
	return ListView_GetImageList(m_hWnd, nImageList);
}

// Sets the image list associated with this control.
HIMAGELIST CMyListView::SetImageList(HIMAGELIST pImageList, int nImageListType)
{
	return ListView_SetImageList(m_hWnd, pImageList, nImageListType);
}

// Retrieves the number of items in the control.
int CMyListView::GetItemCount() const
{
	return ListView_GetItemCount(m_hWnd);
}

// Retrieves a description of a particular item in the control.
BOOL CMyListView::GetItem(LVITEM* pItem) const
{
	return ListView_GetItem(m_hWnd, pItem);
}

// Sets the state of a particular item.
BOOL CMyListView::SetItemState(int nItem, UINT nState, UINT nMask)
{
	ListView_SetItemState(m_hWnd, nItem, nState, nMask);
	return TRUE;
}

// Retrieves the state of a particular item.
UINT CMyListView::GetItemState(int nItem, UINT nMask) const
{
	return ListView_GetItemState(m_hWnd, nItem, nMask);
}

// Retrieves the text associated with a particular item.
int CMyListView::GetItemText(int nItem, int nSubItem, LPWSTR lpwzText, int cchTextMax) const
{
	ListView_GetItemText(m_hWnd, nItem, nSubItem, lpwzText, cchTextMax);
	return 0;
}

// Sets the text associated with a particular item.
BOOL CMyListView::SetItemText(int nItem, int nSubItem, LPWSTR lpszText)
{
	ListView_SetItemText(m_hWnd, nItem, nSubItem, lpszText);
	return TRUE;
}

// Retrieves the control-specific extended style bits.
DWORD CMyListView::GetExtendedStyle()
{
	return ListView_GetExtendedListViewStyle(m_hWnd);
}

// Sets the control-specific extended style bits.
DWORD CMyListView::SetExtendedStyle(DWORD dwNewStyle)
{
	return ListView_SetExtendedListViewStyle(m_hWnd, dwNewStyle);
}

// Sets the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CMyListView::SetHoverTime(DWORD dwHoverTime)
{
	return ListView_SetHoverTime(m_hWnd, dwHoverTime);
}

// Retrieves the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CMyListView::GetHoverTime() const
{
	return ListView_GetHoverTime(m_hWnd);
}

// Operations

// Adds an item to the control.
int CMyListView::InsertItem(const LVITEM* pItem)
{
	return ListView_InsertItem(m_hWnd, pItem);
}

// Removes a single item from the control.
BOOL CMyListView::DeleteItem(int nItem)
{
	return ListView_DeleteItem(m_hWnd, nItem);
}

// Removes all items from the control.
BOOL CMyListView::DeleteAllItems()
{
	return ListView_DeleteAllItems(m_hWnd);
}

// Finds an item in the control matching the specified criteria.  
int CMyListView::FindItem(LVFINDINFO* pFindInfo, int nStart) const
{
	return ListView_FindItem(m_hWnd, nStart, pFindInfo);
}

// Inserts a column into a report-mode control.
int CMyListView::InsertColumn(int nCol, const LVCOLUMN* pColumn)
{
	return ListView_InsertColumn(m_hWnd, nCol, pColumn);
}

// Deletes a column from a report-mode control.
BOOL CMyListView::DeleteColumn(int nCol)
{
	return ListView_DeleteColumn(m_hWnd, nCol);
}

#if (_WIN32_WINNT >= 0x0501)
BOOL CMyListView::SetInfoTip(PLVSETINFOTIP plvInfoTip)
{
	return ListView_SetInfoTip(m_hWnd, plvInfoTip);
}
#endif