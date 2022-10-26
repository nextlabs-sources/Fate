#ifndef __ENCRYPT_PROGRESS_DIALOG2_H__
#define __ENCRYPT_PROGRESS_DIALOG2_H__
#include "resource.h"       // main symbols

#include <atlhost.h>

class CEncryptProgressDlg2 : 
	public CAxDialogImpl<CEncryptProgressDlg2>
{
public:
	CEncryptProgressDlg2() {
		m_wstrTitle = L"";
		m_wstrDescription = L"";
	} ;

	~CEncryptProgressDlg2() {
	} ;

	enum { IDD = IDD_PROGRESS_DLG2 };

	BEGIN_MSG_MAP(CEncryptProgressDlg2)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAxDialogImpl<CEncryptProgressDlg2>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
	LRESULT EndPrograssDlg(VOID) ;

	void set_Titile(LPWSTR lpwzTitle) { m_wstrTitle = lpwzTitle; }
	void set_Description(LPWSTR lpwzDesc) 
	{
		m_wstrDescription = lpwzDesc;
		if (::IsWindow(m_hWnd))
		{
			::SetWindowText( GetDlgItem( IDC_INDICATION2), m_wstrDescription.c_str()) ;
		}
	}

private:
	std::wstring m_wstrTitle;
	std::wstring m_wstrDescription;

} ;
#endif