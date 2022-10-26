#include "StdAfx.h"
#include "NoPermissionDlg.h"

CNoPermissionDlg::CNoPermissionDlg(void)
{
}

CNoPermissionDlg::~CNoPermissionDlg(void)
{
}

LRESULT CNoPermissionDlg::OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	EndDialog(1);

	return 0;
}

LRESULT CNoPermissionDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	EndDialog(1);

	return 0;
}

LRESULT CNoPermissionDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	
	HICON hIcon = LoadIconW(NULL, MAKEINTRESOURCE(IDI_WARNING));
	if(hIcon)
	{
		SetIcon(hIcon);
		DestroyIcon(hIcon);
	}
	else
	{
		g_log.Log(CELOG_DEBUG, L"LoadIconW IDI_WARNING failed in CNoPermissionDlg\n");
	}

	SetDlgItemText(IDC_STATIC_TXT, m_strString.c_str());
	
	
	CenterWindow();

	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return TRUE;
}

void CNoPermissionDlg::MySetString(const wstring& str)
{
	m_strString = str;
}
