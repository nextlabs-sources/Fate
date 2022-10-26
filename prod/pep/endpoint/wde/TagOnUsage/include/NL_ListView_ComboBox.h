#pragma once

#include <ShellAPI.h>

#pragma warning(push)
#pragma warning(disable: 4996)
#pragma warning(disable: 6400)
#pragma warning(disable: 6211)
#pragma warning(disable: 6401)
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlframe.h>
#pragma warning(pop)

#include <map>
#include <string>
#include <vector>

#define NL_LISTVIEW_TEXT		L"TEXT"
#define NL_LISTVIEW_COMBOBOX	L"COMBOBOX"

typedef struct struComboBox
{
	std::wstring strDefaultValue;
	std::vector<std::wstring> vValues;
	BOOL bCreated;
	HWND hWnd;
	RECT rc;
}PARAM_COMBOBOX, *LPPARAM_COMBOBOX;

class CNL_ListView_ComboBox: public CWindowImpl<CNL_ListView_ComboBox,CListViewCtrl>, public  COwnerDraw<CNL_ListView_ComboBox>
{
public:
	CNL_ListView_ComboBox(void);
	~CNL_ListView_ComboBox(void);

	BEGIN_MSG_MAP(CNL_ListView_ComboBox)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		CHAIN_MSG_MAP_ALT(COwnerDraw<CNL_ListView_ComboBox>, 1)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// We are required to have this empty DeleteItem to avoid ambiguity issues
	// with COwnderDraw::DeleteItem and CListViewCtrl::DeleteItem.
	// ---------------------------------------------------------------------
	void DeleteItem(LPDELETEITEMSTRUCT /*lpDeleteItemStruct*/) {};

	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	// If this control was .SubclassWindowed instead of .Create(Xe)d, then it 
	// will not receive a WM_MEASUREITEM since this message occurs before 
	// WM_INITDIALOG. By moving the window we can get windows to generate
	// another one for us.
	// ---------------------------------------------------------------------
	void ForceMeasureItemMessage()
	{
		// I just want to nudge it up a little.
		RECT rc;
		GetWindowRect(&rc);

		POINT pt;
		pt.x = rc.left;
		pt.y = rc.top;
		::ScreenToClient(GetParent(), &pt);
		LONG lWidth = rc.right - rc.left; 
		LONG lHeight = rc.bottom - rc.top;

		rc.left = pt.x;
		rc.right = rc.left + lWidth;
		rc.top = pt.y;

		MoveWindow(&rc);

		//move it back
		rc.bottom = rc.top + lHeight;
		MoveWindow(&rc);

	}

	void GetCellRect(int header_column, const RECT& item_rect, RECT& cell_rect)
	{
		CHeaderCtrl header = GetHeader();
		RECT header_rect;
		header.GetItemRect(header_column, &header_rect);

		// If we don't do this, when we scroll to the right, we will be 
		// drawing as if we weren't and your cells won't line up with the
		// columns.
		int x_offset = -GetScrollPos(SB_HORZ);

		cell_rect.left = x_offset + header_rect.left;
		cell_rect.right = x_offset + header_rect.right;
		cell_rect.top = item_rect.top;
		cell_rect.bottom = item_rect.bottom;
	}

public:
	void SetTextFont(HFONT hFont){m_hFont = hFont;}
	COLORREF SetTextColor(COLORREF clr){m_clrTextColor = clr; return m_clrTextColor;}
	void InsertComboBox(int nItem, int nSubItem, std::vector<std::wstring>& vValues, std::wstring& strDefaultValue);
	BOOL GetItemTextEx(int nItem, int nSubItem, LPTSTR lpszText, int nLen);//This is a wapper of GetItemText, it supports to get the text of combobox
	BOOL DeleteAllItems();
	void SetComboBoxFont(HFONT hFont){m_hDefaultFontComboBox = hFont;}

protected:
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void HideComboBoxes();
protected:
	HFONT m_hFont;
	HFONT m_hDefaultFont;
	HFONT m_hDefaultFontComboBox;
	COLORREF m_clrTextColor;

	//these 2 variables are maintained to support combobox.
	std::map<std::wstring, PARAM_COMBOBOX> m_mapComboBoxes;
};
