#pragma once
#include <string>
#include "resource.h"

using namespace std;

class CNoPermissionDlg: public CAxDialogImpl<CNoPermissionDlg>
{

private:
	wstring m_strString;

public:

	void MySetString(const wstring& pstr);

public:
	CNoPermissionDlg(void);
	~CNoPermissionDlg(void);

	enum { IDD = IDD_ERR_NO_PERMISSION };


	BEGIN_MSG_MAP(CNoPermissionDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedClose)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};
