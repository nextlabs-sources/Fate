#pragma once

#pragma warning(push)
#pragma warning(disable: 6386 6370)
#include <atlbase.h>
#include <atlwin.h>
#include <atlhost.h>
#pragma warning(pop)

#include <Commctrl.h>


class CNL_TabCtrl : public CWindowImpl<CNL_TabCtrl> 
{

public:
	DECLARE_WND_SUPERCLASS( _T("NextLabs_TabCtrl"), WC_TABCONTROL )
	CNL_TabCtrl();
	virtual ~CNL_TabCtrl();

	BEGIN_MSG_MAP(CNL_TabCtrl)
		// uses message reflection: WM_* comes back as OCM_*
		// 		MESSAGE_HANDLER( OCM_COMMAND, OnCommand )
		MESSAGE_HANDLER( OCM_DRAWITEM, OnDrawItem )
		MESSAGE_HANDLER( OCM_MEASUREITEM, OnMeasureItem )
		// 		MESSAGE_HANDLER( WM_DESTROY, OnDestroy ) // not a reflected message
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()


	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	int InsertItem(int nItem, LPCTSTR lpszItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		TCITEM tci = { 0 };
		tci.mask = TCIF_TEXT;
		tci.pszText = (LPTSTR) lpszItem;
		return (int)::SendMessage(m_hWnd, TCM_INSERTITEM, nItem, (LPARAM)&tci);
	}

	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_GETCURSEL, 0, 0L);
	}

	void SetPadding(SIZE size)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(size.cx, size.cy));
	}

	void SetItemSize(int cx, int cy)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(cx, cy));
	}


	int SetCurSel(int nItem)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, TCM_SETCURSEL, nItem, 0L);
	}

	void SetTextFont(HFONT hFont){m_hFont = hFont;}
	void SetBkColor(COLORREF color){m_clrBk = color;}
	void SetTabTextColor(COLORREF color){m_clrText = color;}
protected:
	COLORREF m_clrBk;
	COLORREF m_clrText;
	HFONT    m_hFont;
	HFONT	 m_hDefaultFont;
};