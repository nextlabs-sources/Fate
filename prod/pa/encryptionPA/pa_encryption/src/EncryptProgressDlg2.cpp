#include "stdafx.h"
#include "EncryptProgressDlg2.h"

extern HINSTANCE g_hInstance;


LRESULT CEncryptProgressDlg2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	CAxDialogImpl<CEncryptProgressDlg2>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	this->SetIcon(hIcon,TRUE);

	::ShowWindow(GetDlgItem( IDCANCEL ), SW_HIDE);

	::SetWindowText(m_hWnd, m_wstrTitle.c_str());
	::SetWindowText( GetDlgItem( IDC_INDICATION2), m_wstrDescription.c_str()) ;

	return 1;  // Let the system set the focus
} ;

LRESULT CEncryptProgressDlg2::EndPrograssDlg( ) 
{
	LRESULT hr = S_OK ;

	EndDialog(0);

	return hr ;
}
