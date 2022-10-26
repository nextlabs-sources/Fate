#pragma once

#include "Resource.h"

class CWarningPCPWDDlg: public CAxDialogImpl<CWarningPCPWDDlg>
{
public:
	CWarningPCPWDDlg(void);
	~CWarningPCPWDDlg(void);


	enum { IDD = IDD_PCPWD };
	BEGIN_MSG_MAP(CWarningPCPWDDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CLOSE, BN_CLICKED, OnBnClickedClose)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	void SetText(LPCWSTR pwzText);
protected:
	wchar_t m_szText[1024];
};
