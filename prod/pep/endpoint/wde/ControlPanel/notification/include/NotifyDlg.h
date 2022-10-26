#pragma once

#include "resource.h"       // main symbols
#include "NL_NotificationsList.h"

/*

parsed info

*/
struct NotificationInfo
{
	wstring time;
	wstring action;
	wstring file;
	wstring message;
	int     enforcement;        //	store DENY/ALLOW
};

class CNotifyDlg: public CAxDialogImpl<CNotifyDlg>
{
public:
	CNotifyDlg(void);
	~CNotifyDlg(void);

	enum { IDD = IDD_NOTIFICATION };

	BEGIN_MSG_MAP(CNotifyDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_HANDLER(IDC_CLOSE, BN_CLICKED, OnBnClickedClose)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		CHAIN_MSG_MAP(CAxDialogImpl<CNotifyDlg>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
public:
	
	void FillData();


	BOOL SetNotifyInfo(const NotificationInfo info)
	{
		m_info = info;

		return TRUE;
	}

	void ShowDetails(int nIndex);
	
protected:
	CNL_NotificationsList m_listNotifications;
	HFONT m_hBold;
	NotificationInfo m_info;
public:
	LRESULT OnBnClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};