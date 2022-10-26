// MsgDlg.cpp : Implementation of CEncryptionDlg

#include "stdafx.h"

#include "pa_encryption.h"
#include "MsgDlg.h"

extern HINSTANCE g_hInstance;

// CMsgDlg
LRESULT CMsgDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CMsgDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	this->SetIcon(hIcon,TRUE);

	{
		::SetWindowTextW(m_hWnd, L"Encryption Automation");
	}

	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_STATUS), m_wstrStatus.c_str());
	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_DESCRIPTION), m_wstrDescription.c_str());

// 	::SendMessage(::GetDlgItem(m_hWnd, IDC_STATUS), WM_SETFONT, )
	
	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

LRESULT CMsgDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UNUSED(wNotifyCode);
	UNUSED(hWndCtl);
	UNUSED(bHandled);

	EndDialog(wID);
	return 0;
}



void EA_MessageBox(HWND hOwnerWnd, LPCWSTR lpwzDesc)
{
	MessageBox(hOwnerWnd, lpwzDesc, L"Encryption Failed!", MB_OK | MB_ICONERROR);
}