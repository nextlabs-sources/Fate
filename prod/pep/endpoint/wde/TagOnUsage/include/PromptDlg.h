#pragma once

#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlhost.h>
#pragma warning(pop)

#include "Resource.h"

class CPromptDlg:public CAxDialogImpl<CPromptDlg>
{
public:
	CPromptDlg(void);
	~CPromptDlg(void);

	enum { IDD = IDD_PROMPT };

	BEGIN_MSG_MAP(CPromptDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer);
		MESSAGE_HANDLER(WM_CLOSE, OnClose);
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		CHAIN_MSG_MAP(CAxDialogImpl<CPromptDlg>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClickedOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
//		EndDialog(wID);
		return 0;
	}

	LRESULT OnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
//		EndDialog(wID);
		return 0;
	}
public:
	void SetEndFlag(BOOL bFlag){m_bEnd = bFlag;}
	void SetPathInfo(LPCWSTR pszFilePath){m_strPathText = std::wstring(pszFilePath);}
protected:
	BOOL m_bEnd;
	std::wstring m_strPathText;
};
