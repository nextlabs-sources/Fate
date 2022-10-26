#include "StdAfx.h"
#include "WarningPCPWDDlg.h"

CWarningPCPWDDlg::CWarningPCPWDDlg(void)
{
	memset(m_szText, 0, sizeof(m_szText));
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

	if(wcslen(m_szText) > 0)
	{
		SetDlgItemTextW(IDC_ERROR_TEXT, m_szText);
	}
	//	center window
	CenterWindow();

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

void CWarningPCPWDDlg::SetText(LPCWSTR pwzText)
{
	if(pwzText)
	{
		wcsncpy_s(m_szText, sizeof(m_szText)/sizeof(wchar_t), pwzText, _TRUNCATE);
	}
}
