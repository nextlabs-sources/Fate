// NL_ListView.h : Declaration of the CNL_ListView

#pragma once

#pragma warning(push)
#pragma warning(disable: 6386 6370)
#include <atlbase.h>
#include <atlwin.h>
#include <atlhost.h>
#pragma warning(pop)

#include <Commctrl.h>

class CNL_HeadCtrl: public CWindowImpl<CNL_HeadCtrl>
{
public:
	DECLARE_WND_SUPERCLASS( _T("NextLabs_HeadCtrl"), WC_LISTVIEW )
	CNL_HeadCtrl();
	virtual ~CNL_HeadCtrl();

	BEGIN_MSG_MAP(CNL_HeadCtrl)
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
protected:
	HFONT m_hFont;
};

#define MAX_TEXT_LENGTH 4096
#define HEIGHT_MARGIN	5
// CNL_ListView
class CNL_ListView : public CWindowImpl<CNL_ListView> 
{

public:
	DECLARE_WND_SUPERCLASS( _T("NextLabs_ListView"), WC_LISTVIEW )
	CNL_ListView(); 
	virtual ~CNL_ListView();

	BEGIN_MSG_MAP(CNL_ListView)
		// uses message reflection: WM_* comes back as OCM_*
		// 		MESSAGE_HANDLER( OCM_COMMAND, OnCommand )
		MESSAGE_HANDLER( OCM_DRAWITEM, OnDrawItem )
		MESSAGE_HANDLER( OCM_MEASUREITEM, OnMeasureItem )
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
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// Creator
public:
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


		//get head control
		HWND hHeader = GetHeader();
		if(hHeader)
		{
			m_header.SubclassWindow(hHeader);
		}
	}

	// Generic creator
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID);

	// Generic creator allowing extended style bits
	virtual BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
		HWND hParentWnd, UINT nID);

	// Attributes
	// Retrieves the background color for the control.  
	virtual COLORREF GetBkColor() const;

	// Sets background color for the control.
	virtual BOOL SetBkColor(COLORREF cr);

	// Retrieves the image list associated with the control.
	virtual HIMAGELIST GetImageList(int nImageList) const;

	// Sets the image list associated with this control.
	virtual HIMAGELIST SetImageList(HIMAGELIST pImageList, int nImageListType);

	// Retrieves the number of items in the control.
	virtual int GetItemCount() const;

	// Retrieves a description of a particular item in the control.
	virtual BOOL GetItem(LVITEM* pItem) const;

	// Sets the state of a particular item.
	virtual BOOL SetItemState(int nItem, UINT nState, UINT nMask);

	// Retrieves the state of a particular item.
	virtual UINT GetItemState(int nItem, UINT nMask) const;

	// Retrieves the text associated with a particular item.
	virtual int GetItemText(_In_ int nItem, _In_ int nSubItem, LPWSTR lpwzText, int cchTextMax) const;

	// Sets the text associated with a particular item.
	virtual BOOL SetItemText(int nItem, int nSubItem, LPWSTR lpszText);

	// Retrieves the control-specific extended style bits.
	virtual DWORD GetExtendedStyle();

	// Sets the control-specific extended style bits.
	virtual DWORD SetExtendedStyle(DWORD dwNewStyle);

	// Sets the delay (in milliseconds) for the mouse to hover
	// over an item before it is selected.
	virtual DWORD SetHoverTime(DWORD dwHoverTime = (DWORD)-1);

	// Retrieves the delay (in milliseconds) for the mouse to hover
	// over an item before it is selected.
	virtual DWORD GetHoverTime() const;

	// Operations

	// Adds an item to the control.
	virtual int InsertItem(const LVITEM* pItem);

	//Set item
	virtual BOOL SetItem(const LVITEM* pItem);

	// Removes a single item from the control.
	virtual BOOL DeleteItem(int nItem);

	// Removes all items from the control.
	virtual BOOL DeleteAllItems();

	// Finds an item in the control matching the specified criteria.  
	virtual int FindItem(LVFINDINFO* pFindInfo, int nStart = -1) const;

	// Inserts a column into a report-mode control.
	virtual int InsertColumn(int nCol, const LVCOLUMN* pColumn);

	// Deletes a column from a report-mode control.
	virtual BOOL DeleteColumn(int nCol);

	//Set the font for text
	virtual void SetFont(HFONT hFont);

	virtual HFONT GetFont(){return m_hFont;}

	virtual BOOL SetColWidth(int nCol, int nWidth);

	virtual int GetColWidth(int nCol);

	virtual HWND GetHeader();

#if (_WIN32_WINNT >= 0x0501)
	virtual  BOOL SetInfoTip(PLVSETINFOTIP plvInfoTip);
#endif

	void GetCellRect(int header_column, const RECT& item_rect, RECT& cell_rect)
	{
		HWND hWndHeader = GetHeader();
		RECT header_rect;
		Header_GetItemRect(hWndHeader, header_column, &header_rect);

		// If we don't do this, when we scroll to the right, we will be 
		// drawing as if we weren't and your cells won't line up with the
		// columns.
		int x_offset = -GetScrollPos(SB_HORZ);

		cell_rect.left = x_offset + header_rect.left;
		cell_rect.right = x_offset + header_rect.right;
		cell_rect.top = item_rect.top;
		cell_rect.bottom = item_rect.bottom;
	}

protected:
	HFONT		m_hFont;
	HFONT		m_hDefaultFont;
	COLORREF	m_clrText;
	COLORREF	m_clrBk;

	int m_nLinesInItem;

	CNL_HeadCtrl m_header;
};

