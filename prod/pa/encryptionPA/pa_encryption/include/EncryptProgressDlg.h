#ifndef __ENCRYPT_PROGRESS_DIALOG_H__
#define __ENCRYPT_PROGRESS_DIALOG_H__
#include "resource.h"       // main symbols

#include <atlhost.h>

#define  MAX_PROGRESS_RANGE 100 
#define  MIN_PROGRESS_RANGE	0 
#define  STEP_TIP_DATA 5
#define	 IDT_TIMER_EVENT	10

class CEncryptProgressDlg : 
	public CAxDialogImpl<CEncryptProgressDlg>
{
public:
	CEncryptProgressDlg() {
		m_hProgress = NULL;
		m_iCount = 0 ;

		m_wstrTitle = L"";
		m_wstrDescription = L"";
	} ;

	~CEncryptProgressDlg() {

#if 0
		if( this->m_hWnd )
		{
			KillTimer( IDT_TIMER_EVENT ) ;
			this->DestroyWindow() ;
		}
#endif
	} ;

	enum { IDD = IDD_PROGRESS_DLG };

	BEGIN_MSG_MAP(CEncryptProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CAxDialogImpl<CEncryptProgressDlg>)
	END_MSG_MAP()

	// Handler prototypes:
	//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) ;
// 	LRESULT ShowDialog( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption, ULONG _lPercent ) ;
// 	LRESULT HideDialog( VOID ) ;
	LRESULT EndPrograssDlg(VOID) ;
	static VOID CALLBACK TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime);

	void set_Titile(LPWSTR lpwzTitle) { m_wstrTitle = lpwzTitle; }
	void set_Description(LPWSTR lpwzDesc) 
	{
		m_wstrDescription = lpwzDesc;
		if (::IsWindow(m_hWnd))
		{
			::SetWindowText( GetDlgItem( IDC_INDICATION), m_wstrDescription.c_str()) ;
		}
	}

private:
	HWND m_hProgress ;
	INT m_iCount ;

	std::wstring m_wstrTitle;
	std::wstring m_wstrDescription;

	/*
	thread for progress bar...
	*//*
	HANDLE m_hThread ;
	static DWORD WINAPI ProgressThreadProc( LPVOID lpParam )  ;*/
} ;
#endif