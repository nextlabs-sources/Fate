#pragma once
#include "NXTLBS_Window.h"
#include "resource.h"

class CProductInfoDlg:  public CNXTLBS_Window<CProductInfoDlg>
{
public:
	CProductInfoDlg(void);
	~CProductInfoDlg(void);

	BEGIN_MSG_MAP(CProductInfoDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		CHAIN_MSG_MAP(CAxDialogImpl<CProductInfoDlg>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	enum { IDD = IDD_PRODUCTINFO};

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//it will call the CNXTLBS_Window::OnPaint if sub-class doesn't implement it.
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ShowWindow(SW_HIDE);
		return TRUE;
	}

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		ShowWindow(SW_HIDE);
		return TRUE;
	}
};
