#include "StdAfx.h"
#include "ProgressDlg.h"

HWND CProgressDlg::m_hProgress = NULL ;
INT CProgressDlg::m_iCount  = 0 ;

LRESULT CProgressDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	CAxDialogImpl<CProgressDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;
	m_hProgress = FindWindowEx( this->m_hWnd, NULL,L"msctls_progress32", NULL ) ;
	//m_hProgress = this->GetDlgItem( IDD_PROGRESS_DIALOG ) ;
	if( m_hProgress )
	{
		SendMessage( m_hProgress, PBM_SETRANGE,0, (LPARAM) MAKELPARAM (MIN_RANGE, MAX_RANGE) );
	}
	return 1;  // Let the system set the focus
} ;
LRESULT CProgressDlg::ShowDialog( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption, ULONG _lPercent ) 
{
	_lPercent;		//for warning C4100
	_pszTitleName;	//for warning C4100
	LRESULT hr = S_OK ;
	if( _pszDescrption )
	{
		HWND hwnd = GetDlgItem( IDC_DESCRIPTION) ;
		if( hwnd )
		{
			::SetWindowText( hwnd, _pszDescrption ) ;
		}
	}
	m_iCount = 0 ;
	ShowWindow(  SW_SHOW ) ;
	//DWORD dID = 0 ;
	//m_hThread = CreateThread( 
 //           NULL,              // default security attributes
 //           0,                 // use default stack size  
 //           ProgressThreadProc,        // thread function 
 //           this,             // argument to thread function 
 //           0,                 // use default creation flags 
 //           &dID);   // returns the thread identifier 
//	CProgressDlg *progress = (CProgressDlg*)lpParam ;
	SetTimer( IDT_TIMER_EVENT, 1000,(TIMERPROC)TimerProc ) ;
	if(!m_hProgress )
	{
		m_hProgress = FindWindowEx(m_hWnd, NULL,L"msctls_progress32", NULL ) ;
	}
	SendMessage( m_hProgress, PBM_SETPOS, (WPARAM)m_iCount, 0) ; 
	return hr ;
}
VOID CALLBACK CProgressDlg::TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{
	dwTime;		//for warning C4100
	idTimer;	//for warning C4100
	message;	//for warning C4100
	hwnd;		//for warning C4100

	m_iCount = ( m_iCount+ STEP_TIP_DATA +1 )%MAX_RANGE ;
	if( m_hProgress )
	{
		SendMessage( m_hProgress, PBM_SETPOS, (WPARAM)m_iCount, 0) ;
	}
}
LRESULT CProgressDlg::HideDialog()
{
	LRESULT hr = S_OK ;
	KillTimer( IDT_TIMER_EVENT ) ;
	this->ShowWindow( SW_HIDE ) ;
	m_iCount= 0 ;
	return hr ;
}
LRESULT CProgressDlg::EndPrograssDlg( ) 
{
	LRESULT hr = S_OK ;
	KillTimer( IDT_TIMER_EVENT ) ;
	::DestroyWindow(m_hWnd) ;
	//this->ShowWindow( SW_HIDE ) ;
	m_iCount= 0 ;
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