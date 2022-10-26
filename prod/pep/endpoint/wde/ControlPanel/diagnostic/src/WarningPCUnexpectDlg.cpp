#include "StdAfx.h"
#include "WarningPCUnexpectDlg.h"

CWarningPCUnexpectDlg::CWarningPCUnexpectDlg(void)
{
}

CWarningPCUnexpectDlg::~CWarningPCUnexpectDlg(void)
{
}

LRESULT CWarningPCUnexpectDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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

	return TRUE;
}

LRESULT CWarningPCUnexpectDlg::OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	EndDialog(1);

	return 0;
}

LRESULT CWarningPCUnexpectDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	EndDialog(1);

	return 0;
}
