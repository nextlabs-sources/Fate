#pragma once

#include "resource.h"       // main symbols



class CStopPCDlg: public CAxDialogImpl<CStopPCDlg>
{
public:
	CStopPCDlg(void);
	~CStopPCDlg(void);

	enum { IDD = IDD_STOPPCDLG };

	BEGIN_MSG_MAP(CStopPCDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_PASSWORD, EN_CHANGE, OnEnChangePassword)			//	edit password for Enterprise DLP
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		CHAIN_MSG_MAP(CAxDialogImpl<CStopPCDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnEnChangePassword(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		HDC dc = (HDC)wParam;
		HWND hWnd = (HWND)lParam;

		hWnd;	
		//	SetTextColor(dc, RGB(0, 0, 255));
		SelectObject(dc, g_hFont);

		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 


		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}

	void SetEditFocus()
	{
		HWND hPwd = GetDlgItem(IDC_PASSWORD);
		if(hPwd)
		{
			::SetFocus(hPwd);
		}
	}

	void SetStatusText(LPCWSTR pwzText);
	void ShowWaitingInfo(BOOL bShow = TRUE);
};

