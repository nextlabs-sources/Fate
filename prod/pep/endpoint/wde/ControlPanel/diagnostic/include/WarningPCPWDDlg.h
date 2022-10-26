#pragma once

#include "Resource.h"

class CWarningPCPWDDlg: public CAxDialogImpl<CWarningPCPWDDlg>
{
public:
	CWarningPCPWDDlg(void);
	~CWarningPCPWDDlg(void);


	enum { IDD = IDD_WARNING_PC_PWD };
	BEGIN_MSG_MAP(CWarningPCPWDDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CLOSE, BN_CLICKED, OnBnClickedClose)
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
		SelectObject(dc, g_hFont);

		SetBkColor(dc,   GetSysColor(COLOR_BTNFACE)); 


		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
	}

	void SetText(LPCWSTR pszText){ if(pszText){m_strText = wstring(pszText);} }
protected:
	wstring m_strText;
};
