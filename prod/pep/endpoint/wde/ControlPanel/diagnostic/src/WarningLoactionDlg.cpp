#include "StdAfx.h"
#include "WarningLoactionDlg.h"

CWarningLoactionDlg::CWarningLoactionDlg(void)
{
}

CWarningLoactionDlg::~CWarningLoactionDlg(void)
{
}

LRESULT CWarningLoactionDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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

LRESULT CWarningLoactionDlg::OnBnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	EndDialog(1);

	return 0;
}

LRESULT CWarningLoactionDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	EndDialog(1);

	return 0;
}
