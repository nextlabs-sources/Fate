#pragma once

#include "resource.h" 


class CWarningPCUnexpectDlg: public CAxDialogImpl<CWarningPCUnexpectDlg>
{
public:
	CWarningPCUnexpectDlg(void);
	~CWarningPCUnexpectDlg(void);


	enum { IDD = IDD_PCUNEXPECT };
	BEGIN_MSG_MAP(CWarningPCUnexpectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CLOSE, BN_CLICKED, OnBnClickedClose)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};
