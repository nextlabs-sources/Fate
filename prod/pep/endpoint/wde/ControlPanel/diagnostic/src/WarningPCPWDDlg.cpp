#include "StdAfx.h"
#include "WarningPCPWDDlg.h"

CWarningPCPWDDlg::CWarningPCPWDDlg(void)
{
	m_strText.clear();
}

CWarningPCPWDDlg::~CWarningPCPWDDlg(void)
{
}

LRESULT CWarningPCPWDDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	//	set title icon
	HICON hIcon = LoadIconW(NULL, MAKEINTRESOURCE(IDI_WARNING) );
	if(hIcon)
	{
		SetIcon(hIcon);
		DestroyIcon(hIcon);
	}

	//	center window
	CenterWindow();

	if(m_strText.length() > 0)
	{
		SetDlgItemTextW(IDC_WARNINGTEXT, m_strText.c_str());
	}

	return TRUE;
}

LRESULT CWarningPCPWDDlg::OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	EndDialog(1);

	return 0;
}

LRESULT CWarningPCPWDDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	EndDialog(1);

	return 0;
}
