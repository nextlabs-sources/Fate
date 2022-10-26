#include "stdafx.h"

#include "NL_ListView.h"

CNL_HeadCtrl::CNL_HeadCtrl()
{
	m_hFont = CreateFontW(-14,
							0,
							0,
							0,
							FW_BOLD,
							0,
							0,
							0,
							DEFAULT_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							g_szFont
							);
}

CNL_HeadCtrl::~CNL_HeadCtrl()
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
}

LRESULT CNL_HeadCtrl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 

	//Draw background color
	RECT rcBk;
	::GetWindowRect(m_hWnd, &rcBk);
	ScreenToClient(&rcBk);
	
	HBRUSH hbrBkColor = CreateSolidBrush(RGB(180, 180, 180));
	FillRect(hdc, &rcBk, hbrBkColor);
	DeleteObject (hbrBkColor);


	int nCount = Header_GetItemCount(m_hWnd);

	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hFont);

	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	for(int i = 0; i < nCount; i++)
	{
		RECT rc;
		Header_GetItemRect(m_hWnd, i, &rc);

		HDITEM item;
		item.mask = HDI_TEXT;
		wchar_t buffer[1024] = {0};
		item.pszText = buffer;
		item.cchTextMax = 1024;

		Header_GetItem(m_hWnd, i, &item);

		TextOutW(hdc, rc.left, rc.top + 3, buffer, (int)(wcslen(buffer)));
	//	DrawTextExW(hdc, buffer, wcslen(buffer), &rc, DT_LEFT | DT_VCENTER, NULL);
	}

	SetBkMode(hdc, nOldMode);
	SelectObject(hdc, hOldFont);

	::EndPaint(m_hWnd, &ps); 

	return 0;
}

CNL_ListView::CNL_ListView()
{
	m_hFont = NULL; 
	m_clrBk = 0; 
	m_clrText = 0; 
	m_nLinesInItem = 1;

	m_hDefaultFont = CreateFontW(-12,
								0,
								0,
								0,
								550,
								0,
								0,
								0,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								CLEARTYPE_QUALITY,
								DEFAULT_PITCH | FF_DONTCARE,
								g_szFont
								);

	m_hFont = m_hDefaultFont;
}

CNL_ListView::~CNL_ListView()
{
	if(m_hDefaultFont)
	{
		DeleteObject(m_hDefaultFont);
		m_hDefaultFont = NULL;
	}
}

LRESULT CNL_ListView::OnDrawItem(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
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
			WCHAR wzText[MAX_TEXT_LENGTH + 1];

	//		lpDrawItem->rcItem.left += 2;
	//		lpDrawItem->rcItem.top += 2;

			COLORREF clrHighlight;
			int state = ListView_GetItemState(lpDrawItem->hwndItem, lpDrawItem->itemID, LVIS_FOCUSED | LVIS_SELECTED);
			if (state & LVIS_SELECTED)//Draw highlighted background
			{
				COLORREF color = GetSysColor(COLOR_HIGHLIGHT);
				HBRUSH hbrSEL = CreateSolidBrush(color);
				FillRect(lpDrawItem->hDC, &(lpDrawItem->rcItem), hbrSEL);
				DeleteObject (hbrSEL);
				clrHighlight = RGB(255, 255, 255);
			}
			else
			{//draw background
				COLORREF color;
				if(m_clrBk > 0 )
					color = m_clrBk;
				else
					color = GetSysColor(COLOR_BTNFACE);
				HBRUSH hbrBkColor = CreateSolidBrush(color);
				FillRect(lpDrawItem->hDC, &(lpDrawItem->rcItem), hbrBkColor);
				DeleteObject (hbrBkColor);
				clrHighlight = m_clrText;
			}


			//draw icon
			HIMAGELIST hImglist = ListView_GetImageList(lpDrawItem->hwndItem, LVSIL_SMALL);
			ImageList_Draw(hImglist, lpDrawItem->itemID, lpDrawItem->hDC, 
				lpDrawItem->rcItem.left, lpDrawItem->rcItem.top, ILD_NORMAL | ILD_TRANSPARENT);

			HWND hWnd = GetHeader();

			HFONT oldFont = (HFONT)SelectObject(lpDrawItem->hDC, m_hFont);
			COLORREF oldColor = SetTextColor(lpDrawItem->hDC, clrHighlight);

			if(hWnd)
			{
				int nColCount = Header_GetItemCount(hWnd);
		//		lpDrawItem->rcItem.left += 16;//icon
				SetBkMode(lpDrawItem->hDC, TRANSPARENT);
				for(int i = 0; i < nColCount; i++)
				{
					RECT rcCol;
					GetCellRect(i, lpDrawItem->rcItem, rcCol);
					//try to get the text of current selected item
					memset(wzText, 0, sizeof(wzText));
					ListView_GetItemText(lpDrawItem->hwndItem, lpDrawItem->itemID, i, wzText, MAX_TEXT_LENGTH);
					//draw text
					DrawTextW(lpDrawItem->hDC, wzText, (int)wcslen(wzText),
								&rcCol, DT_LEFT | DT_PATH_ELLIPSIS | DT_NOPREFIX | DT_VCENTER);
					
				}
			}

			
			SelectObject(lpDrawItem->hDC, oldFont);
 			SetTextColor(lpDrawItem->hDC, oldColor);

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
BOOL CNL_ListView::Create(DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID)
{
	m_hWnd = ::CreateWindowW(WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);

	return (m_hWnd == NULL) ? FALSE : TRUE;
}

// Generic creator allowing extended style bits
BOOL CNL_ListView::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
						   HWND hParentWnd, UINT nID)
{
	m_hWnd = ::CreateWindowExW(dwExStyle, WC_LISTVIEW, NULL, dwStyle | WS_CHILD, rect.left, rect.top,
		rect.right-rect.left, rect.bottom-rect.top, hParentWnd, (HMENU)(UINT_PTR)nID, NULL, NULL);

	return (m_hWnd == NULL) ? FALSE : TRUE;
}


// Attributes
// Retrieves the background color for the control.  
COLORREF CNL_ListView::GetBkColor() const
{
	return ListView_GetBkColor(m_hWnd);
}

// Sets background color for the control.
BOOL CNL_ListView::SetBkColor(COLORREF cr)
{
	m_clrBk = cr;
	return ListView_SetBkColor(m_hWnd, cr);
}

// Retrieves the image list associated with the control.
HIMAGELIST CNL_ListView::GetImageList(int nImageList) const
{
	return ListView_GetImageList(m_hWnd, nImageList);
}

// Sets the image list associated with this control.
HIMAGELIST CNL_ListView::SetImageList(HIMAGELIST pImageList, int nImageListType)
{
	return ListView_SetImageList(m_hWnd, pImageList, nImageListType);
}

// Retrieves the number of items in the control.
int CNL_ListView::GetItemCount() const
{
	return ListView_GetItemCount(m_hWnd);
}

// Retrieves a description of a particular item in the control.
BOOL CNL_ListView::GetItem(LVITEM* pItem) const
{
	return ListView_GetItem(m_hWnd, pItem);
}

// Sets the state of a particular item.
BOOL CNL_ListView::SetItemState(int nItem, UINT nState, UINT nMask)
{
	ListView_SetItemState(m_hWnd, nItem, nState, nMask);
	return TRUE;
}

// Retrieves the state of a particular item.
UINT CNL_ListView::GetItemState(int nItem, UINT nMask) const
{
	return ListView_GetItemState(m_hWnd, nItem, nMask);
}

// Retrieves the text associated with a particular item.
int CNL_ListView::GetItemText(int nItem, int nSubItem, LPWSTR lpwzText, int cchTextMax) const
{
	ListView_GetItemText(m_hWnd, nItem, nSubItem, lpwzText, cchTextMax);
	return 0;
}

// Sets the text associated with a particular item.
BOOL CNL_ListView::SetItemText(int nItem, int nSubItem, LPWSTR lpszText)
{
	ListView_SetItemText(m_hWnd, nItem, nSubItem, lpszText);
	return TRUE;
}

// Retrieves the control-specific extended style bits.
DWORD CNL_ListView::GetExtendedStyle()
{
	return ListView_GetExtendedListViewStyle(m_hWnd);
}

// Sets the control-specific extended style bits.
DWORD CNL_ListView::SetExtendedStyle(DWORD dwNewStyle)
{
	return ListView_SetExtendedListViewStyle(m_hWnd, dwNewStyle);
}

// Sets the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CNL_ListView::SetHoverTime(DWORD dwHoverTime)
{
	return ListView_SetHoverTime(m_hWnd, dwHoverTime);
}

// Retrieves the delay (in milliseconds) for the mouse to hover
// over an item before it is selected.
DWORD CNL_ListView::GetHoverTime() const
{
	return ListView_GetHoverTime(m_hWnd);
}

// Operations

// Adds an item to the control.
int CNL_ListView::InsertItem(const LVITEM* pItem)
{
	return ListView_InsertItem(m_hWnd, pItem);
}

//Set item
BOOL CNL_ListView::SetItem(const LVITEM* pItem)
{
	return (BOOL)ListView_SetItem(m_hWnd, pItem);
}

// Removes a single item from the control.
BOOL CNL_ListView::DeleteItem(int nItem)
{
	return ListView_DeleteItem(m_hWnd, nItem);
}

// Removes all items from the control.
BOOL CNL_ListView::DeleteAllItems()
{
	return ListView_DeleteAllItems(m_hWnd);
}

// Finds an item in the control matching the specified criteria.  
int CNL_ListView::FindItem(LVFINDINFO* pFindInfo, int nStart) const
{
	return ListView_FindItem(m_hWnd, nStart, pFindInfo);
}

// Inserts a column into a report-mode control.
int CNL_ListView::InsertColumn(int nCol, const LVCOLUMN* pColumn)
{
	return ListView_InsertColumn(m_hWnd, nCol, pColumn);
}

//Get header
HWND CNL_ListView::GetHeader()
{
	return (HWND)ListView_GetHeader(m_hWnd);
}

//Set column width
BOOL CNL_ListView::SetColWidth(int nCol, int nWidth)
{
	return (BOOL)ListView_SetColumnWidth(m_hWnd, nCol, nWidth);
}

//Getcolumn width
int CNL_ListView::GetColWidth(int nCol)
{
	return ListView_GetColumnWidth(m_hWnd, nCol);
}

// Deletes a column from a report-mode control.
BOOL CNL_ListView::DeleteColumn(int nCol)
{
	return ListView_DeleteColumn(m_hWnd, nCol);
}

#if (_WIN32_WINNT >= 0x0501)
BOOL CNL_ListView::SetInfoTip(PLVSETINFOTIP plvInfoTip)
{
	return ListView_SetInfoTip(m_hWnd, plvInfoTip);
}
#endif

void CNL_ListView::SetFont(HFONT hFont)
{
	m_hFont = hFont;
}

LRESULT CNL_ListView::OnMeasureItem(UINT , WPARAM , LPARAM lParam, BOOL& )
{
	if(!lParam)
	{
		return 0;
	}

	LPMEASUREITEMSTRUCT lpMeasureItemStruct = (LPMEASUREITEMSTRUCT) lParam;

	HDC dc = GetDC();
	TEXTMETRIC tm;
	HFONT hOldFont = (HFONT)SelectObject(dc, m_hFont);
	GetTextMetricsW(dc, &tm);
	SelectObject(dc, hOldFont);


	lpMeasureItemStruct->itemHeight = 
		(tm.tmHeight + tm.tmExternalLeading + HEIGHT_MARGIN) * m_nLinesInItem;

	return 0;
}