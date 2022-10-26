#include "StdAfx.h"
#include <vector>
#include <string>
using namespace std;
#include "DelLogConfirmDlg.h"
#include "EDPMgrUtilities.h"
#include "utilities.h"

#define ENFORCER_DIAGS L"diags"

CDelLogConfirmDlg::CDelLogConfirmDlg(void)
{
}

CDelLogConfirmDlg::~CDelLogConfirmDlg(void)
{
}

LRESULT CDelLogConfirmDlg::OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	//	user proceed delete logs

	vector< pair<wstring, wstring> > v_NameDirPair;

	BOOL res = TRUE;

	//	get installed components' name and dir pair.
	CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
	if ( !utilities.GetComponentsInstallDir(v_NameDirPair) )
	{
		//	error
		res = FALSE;
		goto FUN_EXIT;
	}


	for (DWORD i = 0; i < v_NameDirPair.size(); i++)
	{
		if (v_NameDirPair[i].first == L"NextLabs Policy Controller")
		{
			//	this is policy controller
			//	delete agent log ............................

			//	learn where is agent log
			wchar_t src[1024] = {0};
			wcsncpy_s(src, 1024, v_NameDirPair[i].second.c_str(), _TRUNCATE);
			wcsncat_s(src, 1024, L"agentlog", _TRUNCATE);

			//	delete files under agent log
			edp_manager::CCommonUtilities::DeleteFiles(src);

			//	learn where is bundle.out and delete it
			_snwprintf_s(src, 1024, _TRUNCATE, L"%sbundle.out", v_NameDirPair[i].second.c_str());
			DeleteFile(src);
		}
		else
		{
			//	no, current component is not policy controller

			//	learn where is \diags.........
			wchar_t src[1024] = {0};
			wcsncpy_s(src, 1024, v_NameDirPair[i].second.c_str(), _TRUNCATE);
			wcsncat_s(src, 1024, ENFORCER_DIAGS, _TRUNCATE);
			
			//	delete \diags\logs
			wchar_t logs[1024] = {0};
			_snwprintf_s(logs, 1024, _TRUNCATE, L"%s\\logs", src);
			edp_manager::CCommonUtilities::DeleteFiles(logs);

			//	delete \diags\dumps
			_snwprintf_s(logs, 1024, _TRUNCATE, L"%s\\dumps", src);
			edp_manager::CCommonUtilities::DeleteFiles(logs);
		}
	}

FUN_EXIT:
	EndDialog(0);
	return 0;
}

LRESULT CDelLogConfirmDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	EndDialog(0);
	return 0;
}

LRESULT CDelLogConfirmDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	SetIcon(LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_LOGO)));


	CenterWindow();

	return TRUE;
}
