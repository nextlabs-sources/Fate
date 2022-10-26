#include "StdAfx.h"
#include "NL_ListView_combobox.h"

#define COMBOBOX_HEIGHT		200
#define TEXT_LENGTH			2000
#define COMBO_VALUE_LENGTH  5000

static bool IsSameRC(LPRECT rc1, LPRECT rc2)
{
	if(rc1 && rc2)
	{
		return rc1->left == rc2->left && rc1->top == rc2->top && rc1->right == rc2->right && rc1->bottom == rc2->bottom; 
	}
	return false;
}


CNL_ListView_ComboBox::CNL_ListView_ComboBox(void)
{
	m_hDefaultFont = CreateFontW(14,
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
								L"Arial"
								);

	m_hFont = m_hDefaultFont;

	m_hDefaultFontComboBox = 0;

	m_clrTextColor = RGB(0, 0, 0);
	
}

CNL_ListView_ComboBox::~CNL_ListView_ComboBox(void)
{
	if(m_hDefaultFont)
	{
		DeleteObject(m_hDefaultFont);
		m_hDefaultFont = NULL;
	}

	std::map<std::wstring, PARAM_COMBOBOX>::iterator iter;
	for( iter = m_mapComboBoxes.begin(); iter != m_mapComboBoxes.end(); iter++)
	{
		std::pair<std::wstring, PARAM_COMBOBOX> p = *iter;
		if(p.second.hWnd)
		{
			::DestroyWindow(p.second.hWnd);
		}
	}
}

void CNL_ListView_ComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if(!lpDrawItemStruct)
	{
		return;
	}


	HDC dc = lpDrawItemStruct->hDC;
	RECT item_rect = lpDrawItemStruct->rcItem;
	int item_id = lpDrawItemStruct->itemID;

	if(!dc || item_id < 0)
	{
		return;
	}

	//Redraw the background rect
	HBRUSH hBrush = CreateSolidBrush(GetBkColor());
	HBRUSH hOldBrush = (HBRUSH)SelectObject(dc, hBrush);

	FillRect(dc, &item_rect, hBrush);
	SelectObject(dc, hOldBrush);

	DeleteObject(hBrush);

	int nColCount = GetHeader().GetItemCount();

	HFONT hOldFont = (HFONT)SelectObject(dc, m_hFont);
	for(int i = 0; i < nColCount; i++)//Draw all the cell one by one (column)
	{
		wchar_t szText[TEXT_LENGTH] = {0};

		LVCOLUMN col;
		col.mask = LVCF_TEXT;
		col.pszText = szText;
		col.cchTextMax = TEXT_LENGTH;

		GetColumn(i, &col);//Get the column information

		RECT rc;
		GetCellRect(i,item_rect, rc);//get the current cell rect

		if(wcscmp(col.pszText, NL_LISTVIEW_TEXT) == 0)//draw text
		{
			int nCount = GetItemText(item_id, i, szText, TEXT_LENGTH);
			if(nCount > 0)
			{
				COLORREF clr = ::SetTextColor(dc, m_clrTextColor);
				DrawTextW(dc, szText, nCount, &rc, DT_LEFT | DT_END_ELLIPSIS | DT_TOP | DT_SINGLELINE);
				::SetTextColor(dc, clr);
			}

		}
		else if(wcscmp(col.pszText, NL_LISTVIEW_COMBOBOX) == 0)
		{
			wchar_t szKey[100] = {0};
			_snwprintf_s(szKey, 100, _TRUNCATE, L"%d%d", item_id, i);

			std::map<std::wstring, PARAM_COMBOBOX>::iterator iter = m_mapComboBoxes.find(szKey);
			if(iter != m_mapComboBoxes.end())
			{//User has inserted a combobox.
				PARAM_COMBOBOX& param = m_mapComboBoxes[szKey];

				if(param.bCreated && param.hWnd)//Move to the new position for the all existing comboboxes.
				{
					if(!IsSameRC(&param.rc, &rc))
					{
						::SetWindowPos(param.hWnd, NULL, rc.left, rc.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
						param.rc.left = rc.left;
						param.rc.top = rc.top;
						param.rc.right = rc.right;
						param.rc.bottom = rc.bottom;

					}

					if(!::IsWindowVisible(param.hWnd))
					{
						::ShowWindow(param.hWnd, SW_SHOW);
				}
					return;
				}
				else//create a combobox
				{
				HWND hWnd = ::CreateWindowW(L"ComboBoX", L"", WS_CHILD | WS_VSCROLL | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
											rc.left,
											rc.top,
											(rc.right - rc.left),
											COMBOBOX_HEIGHT,
											m_hWnd, NULL, NULL, NULL);

			//		g_log.Log(CELOG_DEBUG, L"Create combobox index: %s value: %s, HWND: %x", szKey, (*(param.vValues.begin())).c_str(), hWnd);

				if(hWnd)
				{
					param.hWnd = hWnd;
					param.bCreated = TRUE;
						param.rc.left = rc.left;
						param.rc.top = rc.top;
						param.rc.right = rc.right;
						param.rc.bottom = rc.bottom;


					std::vector<std::wstring>::iterator itr;
					int nDefault = -1;
					int nIndex = 0;
					for (itr = param.vValues.begin(); itr != param.vValues.end(); itr++)
					{
						if(wcscmp(param.strDefaultValue.c_str(), (*itr).c_str()) == 0)
							nDefault = nIndex;

						SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)(*itr).c_str());
						nIndex++;
					}
					
					//Set the text value of combobox
					if(nDefault >= 0)
					{
						SendMessage(hWnd, CB_SETCURSEL, (WPARAM)nDefault, 0);
					}
					else
					{
						if(param.vValues.size() > 0)
						{
							SendMessage(hWnd, CB_SETCURSEL, 0, 0);
						}
					}

					//set text font
					if(m_hDefaultFontComboBox != 0)
					{
						SendMessage(hWnd, WM_SETFONT, (WPARAM)m_hDefaultFontComboBox, (LPARAM)TRUE);
					}

					
				}
				}
				
					
			}
			
		}
		
	}

	SelectObject(dc, hOldFont);
	

	return;
}

void CNL_ListView_ComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if(!lpMeasureItemStruct)
	{
		return;
	}

	HDC dc = GetDC();
	TEXTMETRIC tm;
	HFONT hOldFont = (HFONT)SelectObject(dc, m_hFont);
	GetTextMetricsW(dc, &tm);
	SelectObject(dc, hOldFont);
	

	lpMeasureItemStruct->itemHeight = 
		(tm.tmHeight + tm.tmExternalLeading) * 2;

	return;
}

void CNL_ListView_ComboBox::InsertComboBox(int nItem, int nSubItem, std::vector<std::wstring>& vValues, std::wstring& strDefaultValue)
{
	wchar_t szKey[100] = {0};
	_snwprintf_s(szKey, 100, _TRUNCATE, L"%d%d", nItem, nSubItem);

	PARAM_COMBOBOX param;
	param.bCreated = FALSE;
	param.vValues = vValues;
	param.strDefaultValue = strDefaultValue;

	m_mapComboBoxes[szKey] = param;

}

BOOL CNL_ListView_ComboBox::GetItemTextEx(int nItem, int nSubItem, LPTSTR lpszText, int nLen)
{
	wchar_t szText[MAX_PATH] = {0};
	LVCOLUMN col;
	col.mask = LVCF_TEXT;
	col.pszText = szText;
	col.cchTextMax = MAX_PATH;

	GetColumn(nSubItem, &col);//Get the column information

	if(wcscmp(col.pszText, NL_LISTVIEW_TEXT) == 0)//Call the defatul function to get the text
	{
		return GetItemText(nItem, nSubItem, lpszText, nLen);
	}
	else if(wcscmp(col.pszText, NL_LISTVIEW_COMBOBOX) == 0)//Try to get the current selected text in combobox.
	{
		wchar_t szKey[100] = {0};
		_snwprintf_s(szKey, 100, _TRUNCATE, L"%d%d", nItem, nSubItem);

		std::map<std::wstring, PARAM_COMBOBOX>::iterator iter = m_mapComboBoxes.find(szKey);
		if(iter != m_mapComboBoxes.end())
		{//User has inserted a combobox.
			PARAM_COMBOBOX& param = m_mapComboBoxes[szKey];
			
			int nIndex = (int)SendMessage(param.hWnd, CB_GETCURSEL, 0, 0);
			if(nIndex >= 0)
			{
				SendMessage(param.hWnd, CB_GETLBTEXT, nIndex, (LPARAM)lpszText);
				return TRUE;
			}
		}

	}

	return FALSE;
}

BOOL CNL_ListView_ComboBox::DeleteAllItems()
{
	std::map<std::wstring, PARAM_COMBOBOX>::iterator iter;
	for( iter = m_mapComboBoxes.begin(); iter != m_mapComboBoxes.end(); iter++)
	{
		std::pair<std::wstring, PARAM_COMBOBOX> p = *iter;
		if(p.second.hWnd)
		{
			::DestroyWindow(p.second.hWnd);
		}
	}

	m_mapComboBoxes.clear();

	return CListViewCtrl::DeleteAllItems();
}

LRESULT CNL_ListView_ComboBox::OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	HideComboBoxes();

//	Invalidate(TRUE);

	bHandled = FALSE;
	
	return 0;
}

void CNL_ListView_ComboBox::HideComboBoxes()
{
	std::map<std::wstring, PARAM_COMBOBOX>::iterator iter;
	for( iter = m_mapComboBoxes.begin(); iter != m_mapComboBoxes.end(); iter++)
	{
		std::pair<std::wstring, PARAM_COMBOBOX> p = *iter;
		if(p.second.hWnd)
		{
			::ShowWindow(p.second.hWnd, SW_HIDE);
		}
	}
}

