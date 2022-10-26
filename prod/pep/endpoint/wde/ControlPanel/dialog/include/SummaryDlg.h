#pragma once
#include "stdafx.h"
#include "resource.h"       // main symbols
#include <string>
#include <map>
#include "NL_ListView.h"

#define  WM_SHOWLASTUPDATEDBUNDLE		WM_USER + 25

using namespace std;

class CSummaryDlg: public CAxDialogImpl<CSummaryDlg>
{
private:
	CNL_ListView m_compnentList;
	wstring m_lastUpdateTime;

	void FillData();

	BOOL GetLastUpdateBundle(wstring& lastUpdateTime);

	BOOL GetBundleLastModifiedTime(wstring& lastUpdateTime);
public:
	CSummaryDlg(void);
	~CSummaryDlg(void);

	enum { IDD = IDD_SUMMARY };


	BEGIN_MSG_MAP(CSummaryDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint) 
		MESSAGE_HANDLER(WM_SHOWLASTUPDATEDBUNDLE, OnShowLastUpdatedBundle) 
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	//	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorBtn)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnBnClickedOk)
		NOTIFY_HANDLER(IDC_LIST_INSTALLED_COMPONENT, LVN_COLUMNCLICK, OnLvnColumnclickListInstalledComponent)
		REFLECT_NOTIFICATIONS();
		
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorBtn(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowLastUpdatedBundle(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
	void ShowLastUpdatedBundleTime();

protected:
	HFONT m_hBold;
	HFONT m_hNormal;
	HANDLE m_hGetBundleTimeThread;

	map<wstring, wstring> m_mapMonth;
	map<int, int> m_mapSort;
public:
	LRESULT OnLvnColumnclickListInstalledComponent(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
};
