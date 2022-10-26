// MyListView.h : Declaration of the CNXTLBS_ListView

#pragma once

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlbase.h>
#include <atlwin.h>
#include <atlhost.h>
#pragma warning(pop)

#include <Commctrl.h>

// CNXTLBS_ListView
class CNXTLBS_ListView : public CWindowImpl<CNXTLBS_ListView> 
{

public:
	DECLARE_WND_SUPERCLASS( _T("MyListView"), WC_LISTVIEW )
	CNXTLBS_ListView() {m_hFont = NULL; m_clrBk = 0; m_clrText = 0;}

	BEGIN_MSG_MAP(CNXTLBS_ListView)
		// uses message reflection: WM_* comes back as OCM_*
		// 		MESSAGE_HANDLER( OCM_COMMAND, OnCommand )
		MESSAGE_HANDLER( OCM_DRAWITEM, OnDrawItem )
		// 		MESSAGE_HANDLER( WM_DESTROY, OnDestroy ) // not a reflected message
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// 	LRESULT OnDestroy( UINT, WPARAM, LPARAM, BOOL& ) {
	// ¡¡¡¡	 return S_OK;
	// 	}

	// 	LRESULT OnCommand( UINT, WPARAM wParam, LPARAM, BOOL& ) {
	// 		return 0;
	// 	}

	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// Creator
public:
	// Generic creator
	BOOL Create(DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID);

	// Generic creator allowing extended style bits
	BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
		HWND hParentWnd, UINT nID);

	// Attributes
	// Retrieves the background color for the control.  
	COLORREF GetBkColor() const;

	// Sets background color for the control.
	BOOL SetBkColor(COLORREF cr);

	// Retrieves the image list associated with the control.
	HIMAGELIST GetImageList(int nImageList) const;

	// Sets the image list associated with this control.
	HIMAGELIST SetImageList(HIMAGELIST pImageList, int nImageListType);

	// Retrieves the number of items in the control.
	int GetItemCount() const;

	// Retrieves a description of a particular item in the control.
	BOOL GetItem(LVITEM* pItem) const;

	// Sets the state of a particular item.
	BOOL SetItemState(int nItem, UINT nState, UINT nMask);

	// Retrieves the state of a particular item.
	UINT GetItemState(int nItem, UINT nMask) const;

	// Retrieves the text associated with a particular item.
	int GetItemText(_In_ int nItem, _In_ int nSubItem, LPWSTR lpwzText, int cchTextMax) const;

	// Sets the text associated with a particular item.
	BOOL SetItemText(int nItem, int nSubItem, LPWSTR lpszText);

	// Retrieves the control-specific extended style bits.
	DWORD GetExtendedStyle();

	// Sets the control-specific extended style bits.
	DWORD SetExtendedStyle(DWORD dwNewStyle);

	// Sets the delay (in milliseconds) for the mouse to hover
	// over an item before it is selected.
	DWORD SetHoverTime(DWORD dwHoverTime = (DWORD)-1);

	// Retrieves the delay (in milliseconds) for the mouse to hover
	// over an item before it is selected.
	DWORD GetHoverTime() const;

	// Operations

	// Adds an item to the control.
	int InsertItem(const LVITEM* pItem);

	// Removes a single item from the control.
	BOOL DeleteItem(int nItem);

	// Removes all items from the control.
	BOOL DeleteAllItems();

	// Finds an item in the control matching the specified criteria.  
	int FindItem(LVFINDINFO* pFindInfo, int nStart = -1) const;

	// Inserts a column into a report-mode control.
	int InsertColumn(int nCol, const LVCOLUMN* pColumn);

	// Deletes a column from a report-mode control.
	BOOL DeleteColumn(int nCol);

	//Set the font for text
	void SetFont(HFONT hFont);

	HFONT GetFont(){return m_hFont;}

#if (_WIN32_WINNT >= 0x0501)
	BOOL SetInfoTip(PLVSETINFOTIP plvInfoTip);
#endif

protected:
	HFONT		m_hFont;
	COLORREF	m_clrText;
	COLORREF	m_clrBk;
};

