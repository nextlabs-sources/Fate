#pragma once

#include "resource.h"       // main symbols
#include "NL_ListView.h"
#include "EDPMgrUtilities.h"


class CNotifyDlg: public CAxDialogImpl<CNotifyDlg>
{
public:
	CNotifyDlg(void);
	~CNotifyDlg(void);

	enum { IDD = IDD_NOTIFICATIONS };

	BEGIN_MSG_MAP(CNotifyDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_PAINT, OnPaint) 
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		COMMAND_HANDLER(IDC_REFRESH, BN_CLICKED, OnBnClickedRefresh)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_SHOWNOTIFICATION, OnShowNotification)
		NOTIFY_HANDLER(IDC_LIST_NOTIFICATIONS, LVN_ITEMCHANGED, OnLvnItemchangedList)
		NOTIFY_HANDLER(IDC_LIST_NOTIFICATIONS, LVN_GETINFOTIP, OnLvnGetInfoTipList)
		CHAIN_MSG_MAP(CAxDialogImpl<CNotifyDlg>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowNotification(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLvnItemchangedList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnLvnGetInfoTipList(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
public:
	void ShowNotifications();
	void FillData();
	void FillData_test();//for test
	void ShowDetails(int nIndex);
protected:
	CNL_ListView			m_listNotifications;
	HANDLE					m_hThread;
	HFONT					m_hFont;
	HFONT					m_hFontBig;
	CEDPMUtilities::NotificationVector m_NotifyArray;
public:
	LRESULT OnBnClickedRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
