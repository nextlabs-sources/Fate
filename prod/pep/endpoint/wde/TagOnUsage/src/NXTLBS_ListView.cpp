#include "stdafx.h"

#include "NXTLBS_ListView.h"

LRESULT CNXTLBS_ListView::OnDrawItem(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) lParam;

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

// 			COLORREF clrHighlight;//fix bug7586
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
// 				COLORREF color;
// 				if(m_clrBk > 0 )
// 					color = m_clrBk;
// 				else
// 					color = GetSysColor(COLOR_BTNFACE);
// 				HBRUSH hbrBkColor = CreateSolidBrush(color);
// 				FillRect(lpDrawItem->hDC, &(lpDrawItem->rcItem), hbrBkColor);
// 				DeleteObject (hbrBkColor);
// 				clrHighlight = m_clrText;//fix bug7586
// 			}

			memset(wzFilePath, 0, sizeof(wzFilePath));
			ListView_GetItemText(lpDrawItem->hwndItem, lpDrawItem->itemID, 0, wzFilePath, MAX_PATH);

			HIMAGELIST hImglist = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);
			ImageList_Draw(hImglist, lpDrawItem->itemID, lpDrawItem->hDC, 
				lpDrawItem->rcItem.left, lpDrawItem->rcItem.top, ILD_NORMAL | ILD_TRANSPARENT);

			lpDrawItem->rcItem.left += 16;
			SetBkMode(lpDrawItem->hDC, TRANSPARENT);
			HFONT oldFont = (HFONT)SelectObject(lpDrawItem->hDC, m_hFont);

// 			COLORREF oldColor = SetTextColor(lpDrawItem->hDC, clrHighlight);

			DrawTextW(lpDrawItem->hDC, wzFilePath, (int)wcslen(wzFilePath),
				&(lpDrawItem->rcItem), DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX); // fix bug374
			SelectObject(lpDrawItem->hDC, oldFont);

// 			SetTextColor(lpDrawItem->hDC, oldColor);

		}
		if (lpDrawItem->itemAction & ODA_FOCUS)
		{
//			int iState = lpDrawItem->itemState;
		}
		if (lpDrawItem->itemAction & ODA_SELECT)
		{
//			int iState = lpDrawItem->itemState;
		}
	}

	return S_OK;
}

// Generic creator
BOOL CNXTLBS_ListView::Create(DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID)
{
	m_hWnd = ::CreateWindowW(WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);

	return (m_hWnd == NULL) ? FALSE : TRUE;
}

// Generic creator allowing extended style bits
BOOL CNXTLBS_ListView::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
						   HWND hParentWnd, UINT nID)
{
	m_hWnd = ::CreateWindowExW(dwExStyle, WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);

	return (m_hWnd == NULL) ? FALSE : TRUE;
}


// Attributes
// Retrieves the background color for the control.  
COLORREF CNXTLBS_ListView::GetBkColor() const
{
	return ListView_GetBkColor(m_hWnd);
}

// Sets background color for the control.
BOOL CNXTLBS_ListView::SetBkColor(COLORREF cr)
{
	m_clrBk = cr;
	return ListView_SetBkColor(m_hWnd, cr);
}

// Retrieves the image list associated with the control.
HIMAGELIST CNXTLBS_ListView::GetImageList(int nImageList) const
{
	return ListView_GetImageList(m_hWnd, nImageList);
}

// Sets the image list associated with this control.
HIMAGELIST CNXTLBS_ListView::SetImageList(HIMAGELIST pImageList, int nImageListType)
{
	return ListView_SetImageList(m_hWnd, pImageList, nImageListType);
}

// Retrieves the number of items in the control.
int CNXTLBS_ListView::GetItemCount() const
{
	return ListView_GetItemCount(m_hWnd);
}

// Retrieves a description of a particular item in the control.
BOOL CNXTLBS_ListView::GetItem(LVITEM* pItem) const
{
	return ListView_GetItem(m_hWnd, pItem);
}

// Sets the state of a particular item.
BOOL CNXTLBS_ListView::SetItemState(int nItem, UINT nState, UINT nMask)
{
	ListView_SetItemState(m_hWnd, nItem, nState, nMask);
	return TRUE;
}

// Retrieves the state of a particular item.
UINT CNXTLBS_ListView::GetItemState(int nItem, UINT nMask) const
{
	return ListView_GetItemState(m_hWnd, nItem, nMask);
}

// Retrieves the text associated with a particular item.
int CNXTLBS_ListView::GetItemText(int nItem, int nSubItem, LPWSTR lpwzText, int cchTextMax) const
{
	ListView_GetItemText(m_hWnd, nItem, nSubItem, lpwzText, cchTextMax);
	return 0;
}

// Sets the text associated with a particular item.
BOOL CNXTLBS_ListView::SetItemText(int nItem, int nSubItem, LPWSTR lpszText)
{
	ListView_SetItemText(m_hWnd, nItem, nSubItem, lpszText);
	return TRUE;
}

// Retrieves the control-specific extended style bits.
DWORD CNXTLBS_ListView::GetExtendedStyle()
{
	return ListView_GetExtendedListViewStyle(m_hWnd);
}

// Sets the control-specific extended style bits.
DWORD CNXTLBS_ListView::SetExtendedStyle(DWORD dwNewStyle)
{
	return ListView_SetExtendedListViewStyle(m_hWnd, dwNewStyle);
}

// Sets the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CNXTLBS_ListView::SetHoverTime(DWORD dwHoverTime)
{
	return ListView_SetHoverTime(m_hWnd, dwHoverTime);
}

// Retrieves the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CNXTLBS_ListView::GetHoverTime() const
{
	return ListView_GetHoverTime(m_hWnd);
}

// Operations

// Adds an item to the control.
int CNXTLBS_ListView::InsertItem(const LVITEM* pItem)
{
	return ListView_InsertItem(m_hWnd, pItem);
}

// Removes a single item from the control.
BOOL CNXTLBS_ListView::DeleteItem(int nItem)
{
	return ListView_DeleteItem(m_hWnd, nItem);
}

// Removes all items from the control.
BOOL CNXTLBS_ListView::DeleteAllItems()
{
	return ListView_DeleteAllItems(m_hWnd);
}

// Finds an item in the control matching the specified criteria.  
int CNXTLBS_ListView::FindItem(LVFINDINFO* pFindInfo, int nStart) const
{
	return ListView_FindItem(m_hWnd, nStart, pFindInfo);
}

// Inserts a column into a report-mode control.
int CNXTLBS_ListView::InsertColumn(int nCol, const LVCOLUMN* pColumn)
{
	return ListView_InsertColumn(m_hWnd, nCol, pColumn);
}

// Deletes a column from a report-mode control.
BOOL CNXTLBS_ListView::DeleteColumn(int nCol)
{
	return ListView_DeleteColumn(m_hWnd, nCol);
}

#if (_WIN32_WINNT >= 0x0501)
BOOL CNXTLBS_ListView::SetInfoTip(PLVSETINFOTIP plvInfoTip)
{
	return ListView_SetInfoTip(m_hWnd, plvInfoTip);
}
#endif

void CNXTLBS_ListView::SetFont(HFONT hFont)
{
	m_hFont = hFont;
}