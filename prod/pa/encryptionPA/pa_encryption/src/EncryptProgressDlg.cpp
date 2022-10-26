#include "stdafx.h"
#include "EncryptProgressDlg.h"

extern HINSTANCE g_hInstance;

// HWND CEncryptProgressDlg::m_hProgress = NULL ;
// INT CEncryptProgressDlg::m_iCount  = 0 ;

LRESULT CEncryptProgressDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	CAxDialogImpl<CEncryptProgressDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	this->SetIcon(hIcon,TRUE);

	::ShowWindow(GetDlgItem( IDCANCEL ), SW_HIDE);

	::SetWindowText(m_hWnd, m_wstrTitle.c_str());
	::SetWindowText( GetDlgItem( IDC_INDICATION), m_wstrDescription.c_str()) ;

	m_hProgress = GetDlgItem( IDC_PROGRESS_BAR );
	if( m_hProgress )
	{
		SendMessage( m_hProgress, PBM_SETRANGE,0, (LPARAM) MAKELPARAM (MIN_PROGRESS_RANGE, MAX_PROGRESS_RANGE) );
	}

	SetTimer( IDT_TIMER_EVENT, 1000, (TIMERPROC)TimerProc ) ;
	SendMessage( m_hProgress, PBM_SETPOS, (WPARAM)m_iCount, 0) ; 

	return 1;  // Let the system set the focus
} ;

// 
// LRESULT CEncryptProgressDlg::ShowDialog( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption, ULONG _lPercent ) 
// {
// 	LRESULT hr = S_OK ;
// 	if( _pszDescrption )
// 	{
// 		HWND hwnd = GetDlgItem( IDC_DESCRIPTION) ;
// 		if( hwnd )
// 		{
// 			::SetWindowText( hwnd, _pszDescrption ) ;
// 		}
// 	}
// 	m_iCount = 0 ;
// 	ShowWindow(  SW_SHOW ) ;
// 	//DWORD dID = 0 ;
// 	//m_hThread = CreateThread( 
// 	//           NULL,              // default security attributes
// 	//           0,                 // use default stack size  
// 	//           ProgressThreadProc,        // thread function 
// 	//           this,             // argument to thread function 
// 	//           0,                 // use default creation flags 
// 	//           &dID);   // returns the thread identifier 
// 	//	CProgressDlg *progress = (CProgressDlg*)lpParam ;
// 	SetTimer( IDT_TIMER_EVENT, 1000,(TIMERPROC)TimerProc ) ;
// 	if(!m_hProgress )
// 	{
// 		m_hProgress = FindWindowEx(m_hWnd, NULL,L"msctls_progress32", NULL ) ;
// 	}
// 	SendMessage( m_hProgress, PBM_SETPOS, (WPARAM)m_iCount, 0) ; 
// 	return hr ;
// }


VOID CALLBACK CEncryptProgressDlg::TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	UNUSED(dwTime);
	UNUSED(idTimer);
	UNUSED(message);

	HWND hProgress = ::GetDlgItem( hwnd, IDC_PROGRESS_BAR );
	INT iCount = (INT)SendMessage( hProgress, PBM_GETPOS, 0, 0) ;
	iCount = ( iCount+ STEP_TIP_DATA +1 )%MAX_PROGRESS_RANGE ;

	SendMessage( hProgress, PBM_SETPOS, (WPARAM)iCount, 0) ;
}

// LRESULT CEncryptProgressDlg::HideDialog()
// {
// 	LRESULT hr = S_OK ;
// 	KillTimer( IDT_TIMER_EVENT ) ;
// 	this->ShowWindow( SW_HIDE ) ;
// 	m_iCount= 0 ;
// 	return hr ;
// }

LRESULT CEncryptProgressDlg::EndPrograssDlg( ) 
{
	LRESULT hr = S_OK ;
	KillTimer( IDT_TIMER_EVENT ) ;
	//::DestroyWindow(m_hWnd) ;
// 	m_iCount = 0;
	//m_hProgress = NULL;
	//m_hWnd = NULL;
	
	EndDialog(0);

	return hr ;
}
//DWORD WINAPI CProgressDlg::ProgressThreadProc( LPVOID lpParam )  
//{
//	DWORD dRet = 0 ;
//	CProgressDlg *progress = (CProgressDlg*)lpParam ;
//	progress->SetTimer( IDT_TIMER_EVENT, 1000,(TIMERPROC)CProgressDlg::TimerProc ) ;
//	if(!progress->m_hProgress )
//	{
//		progress->m_hProgress = FindWindowEx( progress->m_hWnd, NULL,L"msctls_progress32", NULL ) ;
//	}
//	SendMessage( progress->m_hProgress, PBM_SETPOS, (WPARAM)progress->m_iCount, 0) ; 
//	return dRet ;
//
//}	