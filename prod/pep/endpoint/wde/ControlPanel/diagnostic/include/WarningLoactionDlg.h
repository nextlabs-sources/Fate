#pragma once


#include "resource.h" 


class CWarningLoactionDlg: public CAxDialogImpl<CWarningLoactionDlg>
{
public:
	CWarningLoactionDlg(void);
	~CWarningLoactionDlg(void);


	enum { IDD = IDD_WARNING_LOCATION };
	BEGIN_MSG_MAP(CWarningLoactionDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(ID_CLOSE, BN_CLICKED, OnBnClickedClose)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		HDC dc = (HDC)wParam;
		HWND hWnd = (HWND)lParam;

		hWnd;	
		//	SetTextColor(dc, RGB(0, 0, 255));
		SelectObject(dc, g_hSmallFont);

		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 


		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}
};
