#include "StdAfx.h"
#include <string>
#include "StopEnfDlg.h"
#include "EDPMgrUtilities.h"
#include "cesdk.h"
#include "PWDMgr.h"
#include "WarningPCPWDDlg.h"
#include "WarningPCUnexpectDlg.h"

HANDLE g_hStoppingPC = NULL;
wchar_t g_szPassword[256] = {0};
void __cdecl StopPCThread( void* pArguments )
{
	if(pArguments)
	{
		//	submit password to stop PC
		CEDPMUtilities& utilities = CEDPMUtilities::GetInstance();
		CEResult_t ret = utilities.StopPC(g_szPassword);

		::PostMessage((HWND)pArguments, WM_HANDLE_STOPPC, (WPARAM)ret, 0);
	}
}

CStopEnfDlg::CStopEnfDlg(void)
{
}

CStopEnfDlg::~CStopEnfDlg(void)
{
}

LRESULT CStopEnfDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// TODO: Add your message handler code here and/or call default
	CAxDialogImpl<CStopEnfDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	HICON hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, FALSE);
	}

	hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, TRUE);
	}

	m_bIsStoppingPC = FALSE;

	HWND hProgress = ::GetDlgItem(m_hWnd, IDC_STOPPINGPC_PROGRESS);
	::SendMessage(hProgress, PBM_SETMARQUEE, (WPARAM)TRUE, 50);

	return TRUE;
}

LRESULT CStopEnfDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CStopEnfDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bIsStoppingPC)
	{
		return 0;
	}

	DestroyWindow();
	return 0;
}

LRESULT CStopEnfDlg::OnBnClickedButtonCancelStopenf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
	return 0;
}

LRESULT CStopEnfDlg::OnBnClickedButtonSubmitStopenf(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	
	//	below code is stopping enforcer........................

	//	we need to get password user provided first,
	wchar_t* szPwd = NULL;
	if (GetDlgItemText(IDC_EDIT_STOP_ENF_PWD, szPwd) && szPwd)
	{
		WSADATA wsaData = {0};
		if( 0 == WSAStartup(MAKEWORD(2, 2), &wsaData) )//force to load NE before calling "connect PC". there is a bug related with installing both NE and WDE, 
		{
			SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock != INVALID_SOCKET)
			{
				closesocket(sock);
			}
			WSACleanup();
		}

		m_bIsStoppingPC = TRUE;

		g_hStoppingPC = CreateEventW(NULL, FALSE, TRUE,  EVENT_STOPPING_PC);

		SwitchUI(UI_STOPPING);
		memset(g_szPassword, 0, sizeof(g_szPassword));
		wcsncpy_s(g_szPassword, sizeof(g_szPassword)/sizeof(wchar_t), szPwd, _TRUNCATE);
		_beginthread(StopPCThread, 0, (void*)m_hWnd);

	}
	else
	{
		wchar_t szError[1024] = {0};
		LoadStringW(g_hInst, IDS_EMPTYPASSWORD, szError, sizeof(szError)/sizeof(wchar_t));
		CWarningPCPWDDlg pwdDlg;
		pwdDlg.SetText(szError);
		pwdDlg.DoModal();
	}

	return 0;
}

LRESULT CStopEnfDlg::OnHandleStopPC(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bIsStoppingPC = FALSE;

	if(g_hStoppingPC)
	{
		CloseHandle(g_hStoppingPC);
		g_hStoppingPC = NULL;
	}

	CEResult_t res = (CEResult_t)wParam;
	switch(res)
	{
	case CE_RESULT_SUCCESS:
		{
			//	pc is stopped, the password is correct, we save password as internal usage.
			CPWDMgr& pwdMgr = CPWDMgr::GetInstance();
			pwdMgr.setpwd(g_szPassword);

			//	password is saved, we can close current dialog
			::SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);

			return 0;
		}
		break;
	case CE_RESULT_INVALID_PARAMS:
	case CE_RESULT_PERMISSION_DENIED:
		{
			//	password invalid
			//	tell user invalid password
			CWarningPCPWDDlg pwdDlg;
			pwdDlg.DoModal();
		}
		break;
	default:
		{
			//	general error
			//	tell user meet general error to stop pc
			CWarningPCUnexpectDlg unexpectDlg;
			unexpectDlg.DoModal();
		}
		break;
	}

	SwitchUI(UI_BASIC);
	
	return 0;
}

void CStopEnfDlg::SwitchUI(UI_TYPE nType)//0: Type in password to stop PC, 1: show progress bar for stopping PC.
{
	switch (nType)
	{
	case UI_BASIC:
		{
			::ShowWindow(GetDlgItem(IDC_STOPPC_TITLE), SW_SHOW);
			::ShowWindow(GetDlgItem(IDC_STOPPC_PASSWORD), SW_SHOW);
			::ShowWindow(GetDlgItem(IDC_EDIT_STOP_ENF_PWD), SW_SHOW);

			::ShowWindow(GetDlgItem(IDC_STOPPINGPC_HINT), SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_STOPPINGPC_PROGRESS), SW_HIDE);

			::EnableWindow(GetDlgItem(IDC_BUTTON_SUBMIT_STOPENF), TRUE);
			::EnableWindow(GetDlgItem(IDC_BUTTON_CANCEL_STOPENF), TRUE);
		}
		break;
	case UI_STOPPING:
		{
			::ShowWindow(GetDlgItem(IDC_STOPPC_TITLE), SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_STOPPC_PASSWORD), SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_EDIT_STOP_ENF_PWD), SW_HIDE);

			::ShowWindow(GetDlgItem(IDC_STOPPINGPC_HINT), SW_SHOW);
			::ShowWindow(GetDlgItem(IDC_STOPPINGPC_PROGRESS), SW_SHOW);

			::EnableWindow(GetDlgItem(IDC_BUTTON_SUBMIT_STOPENF), FALSE);
			::EnableWindow(GetDlgItem(IDC_BUTTON_CANCEL_STOPENF), FALSE);
		}
		break;
	}
}
