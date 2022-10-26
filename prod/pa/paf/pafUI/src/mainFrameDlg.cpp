#include "stdafx.h"
#include "mainFrameDlg.h"
#include "UIInterface.h"
#include "shellapi.h"
CMainFrame::CMainFrame()
{
	m_dlgProgress = NULL ;
	m_hCommunicatorPanel = NULL ;
	m_dlgStatus  = MAINFRAME::DS_NEXT ;
	m_hChildPanel = NULL ;
	m_mainBtStrcture.pData = NULL ;
	m_mainBtStrcture.pMainBtFunc = NULL ;
	m_cancelBtStrcture.pMainBtFunc = NULL ;
	m_cancelBtStrcture.pData = NULL ;
	::ZeroMemory( m_BtName, MAX_PATH*sizeof(wchar_t ) ) ;
	m_iBtStatus = pafUI::BT_ENABLE ;
	m_hParent = NULL ;

	m_dwCreatedTime = 0;
} 
CMainFrame::~CMainFrame() 
{
	if( m_hCommunicatorPanel )
	{
		delete m_hCommunicatorPanel ;
		m_hCommunicatorPanel = NULL ;
	}
	if( m_dlgProgress )
	{
		delete m_dlgProgress ;
		m_dlgProgress = NULL ;
	}
	if( ::IsWindow(m_tmpDlg.m_hWnd) )
	{
		::DestroyWindow( m_tmpDlg.m_hWnd ) ;
	}
	m_strHelpURL.clear() ;
	//m_listNextBt.clear() ;
	//m_listCancel.clear() ;
	m_hChildPanel = NULL ;
	::ZeroMemory( m_BtName, MAX_PATH*sizeof(wchar_t ) ) ;
	m_hParent = NULL ;
}
/*
Initialize the image refernce.....
*/
HRESULT CMainFrame::InitImageRef() 
{
	HRESULT hr = S_OK ;

	/*
	Title icon
	*/

	HICON hIcon = (HICON)::LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAIN_TITLE_ICON), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAIN_TITLE_ICON), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	return hr ;
}
/*
The dialog initialize...
*/
LRESULT CMainFrame::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CMainFrame>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	
	m_dwCreatedTime = GetTickCount();

	if( !m_strHelpURL.empty() )
	{
		LONG lRet = GetWindowLong( GWL_EXSTYLE ) ;
		if( m_strHelpURL.empty() )
		{
			lRet = SetWindowLong( GWL_EXSTYLE, lRet|WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT ) ;
		}

	}
	else
	{
		LONG lRet = GetWindowLong( GWL_EXSTYLE ) ;
		lRet = SetWindowLong( GWL_EXSTYLE, lRet|WS_EX_CONTROLPARENT ) ;
	}
	bHandled = TRUE;
	if( m_hChildPanel )
	{
		//DoZBufLog(	this->m_hParent ) ;
		HWND hPre = ::SetParent( m_hChildPanel,this->m_hWnd ) ;
		//DoZBufLog(	this->m_hParent ) ;
		::SetFocus(	 m_hChildPanel ) ;

		//DoZBufLog(	this->m_hWnd ) ;
		if( hPre == NULL )
		{
			//MessageBox( L"Failure",0,0 ) ;
		}
		else
		{
			AutoJustWindow() ;
			::ShowWindow( m_hChildPanel, SW_SHOW ) ;
		}
	}
	//ShowProgressPanel( NULL );
	if( lstrlenW( this->m_BtName ) != 0 )
	{
		HWND hOkBt = GetDlgItem( ID_NEXT_FINISHED ) ;
		::SetWindowText( hOkBt,m_BtName ) ;
	}
	
	ProcessBtStatus( m_iBtStatus, ID_NEXT_FINISHED ) ;
	InitImageRef() ;
	SetMiddlePosition() ;
	DP((L"===============OnInitDialog======================"));
	
	return 1;  // Let the system set the focus
}
VOID CMainFrame::SetMiddlePosition() 
{
	INT iScreenWidth = GetSystemMetrics(   SM_CXFULLSCREEN ) ;
	INT iScreenHeight = GetSystemMetrics(SM_CYFULLSCREEN	 ) ;
	RECT rc ;
	this->GetWindowRect(  &rc ) ;
	INT iWidth = rc.right -rc.left ;
	INT iHeight  = rc.bottom -rc.top ;
	int left = (iScreenWidth - iWidth )/2 ;
	int top = ( iScreenHeight - iHeight ) /2	;
	SetWindowPos( HWND_TOP, left,  top,	  iWidth,iHeight, FALSE ) ; 
//	DoZBufLog(	this->m_hWnd ) ;
}
LRESULT CMainFrame::OnClickedNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bHandled;	//for warning C4100
	hWndCtl;	//for warning C4100
	wID;		//for warning C4100
	wNotifyCode;//for warning C4100

	::EnableWindow(hWndCtl, FALSE);
	if( m_mainBtStrcture.pMainBtFunc ) 
	{
		((MAINFRAME::OnClichNextType)(m_mainBtStrcture.pMainBtFunc))(m_mainBtStrcture.pData, m_dlgStatus, this->m_hWnd ) ;
	}
	::EnableWindow(hWndCtl, TRUE);

	return 0;
}

LRESULT CMainFrame::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bHandled;	//for warning C4100
	hWndCtl;	//for warning C4100
	wID;		//for warning C4100
	wNotifyCode;//for warning C4100
	DP((L"OnClickedCancel")) ;
	if( m_cancelBtStrcture.pMainBtFunc ) 
	{
		((MAINFRAME::ONCLICKCANCELBT)(m_cancelBtStrcture.pMainBtFunc))(m_cancelBtStrcture.pData , this->m_hWnd ) ;
	}
	ReleaseMainWnd(IDCANCEL) ;
//	m_dResult = S_FALSE	;
	return 0;
}
LRESULT CMainFrame::ShowProgressPanel( const wchar_t * _pszTitleName  ,const wchar_t * _pszDescrption )
{
	LRESULT hr = S_OK ;
	if( m_dlgProgress )
	{
		delete m_dlgProgress ;
		m_dlgProgress = NULL ;
	}
	::ShowWindow( m_hChildPanel, SW_HIDE ) ;
	RECT rc ;
	HWND hwnd = GetDlgItem( IDC_OBLIGATION_PANEL ) ;

	if( hwnd == NULL )
	{
		return hr ;
	}
	::GetWindowRect( hwnd, &rc ) ;
	this->ScreenToClient( &rc ) ;
	::ShowWindow( hwnd, SW_HIDE ) ;
	m_dlgProgress = new CProgressDlg ;
	m_dlgProgress->Create( m_hWnd, rc, NULL ) ;
	m_dlgProgress->MoveWindow( rc.left,rc.top,rc.right-rc.left ,rc.bottom-rc.top,TRUE ) ;
	//}
	m_dlgProgress->ShowDialog( _pszTitleName,_pszDescrption , 0 ) ;
	HWND hChild = GetDlgItem( ID_NEXT_FINISHED ) ;
	if( hChild ) 
	{
		::ShowWindow( hChild, SW_SHOW ) ;
	}
	hChild = GetDlgItem( IDCANCEL ) ;
	if( hChild ) 
	{
		::ShowWindow( hChild, SW_SHOW ) ;
	}
	return hr ;
}
LRESULT CMainFrame::ReleaseMainWnd(int nExitID)
{
	LRESULT hr = S_OK ;
	if( this->m_bDoModal == TRUE )
	{
		EndDialog(nExitID) ;
	}
	else
	{
		this->DestroyWindow() ;
	}
	this->m_hParent = NULL ;
	return hr ;
}
INT_PTR CMainFrame::CreateMainWindow(	const HWND _hChildWnd ,
									const BOOL _bIsModel ,
									const wchar_t* _pszHelpURL,
									const wchar_t* _strBtName,
									const INT _ibtStatus,
									const HWND _hParent,
									const wchar_t* _pszTitle, 
									const wchar_t* _pszDescription  ) 
{
	_pszDescription;	//for warning C4100
	_pszTitle;			//for warning C4100
	//HWND hwnd = NULL ;	//for warning C4189
	if( m_hParent == NULL )
	{
		m_hParent = _hParent ;
	}
	m_hChildPanel = _hChildWnd ;
	m_iBtStatus = _ibtStatus ;

	WNDCLASS wndcls;  
	::GetClassInfo( g_hInst, L"#32770", &wndcls );
	wndcls.lpszClassName = MAINFRAME::m_szClassName ; ; 
	RegisterClass( &wndcls ); 

	if( _pszHelpURL )
	{
		m_strHelpURL = _pszHelpURL ;
	}
	if( _strBtName )
	{
		::lstrcpynW(  m_BtName,_strBtName , MAX_PATH ) ;
	}
//	m_dlgStatus= (MAINFRAME::DLGSTATUS)_iDlgStatu ;
	if( _hParent != NULL )
	{

	}

	m_bDoModal = _bIsModel ;
	if( _bIsModel == TRUE )
	{
		if( !::IsWindow(m_hParent) )
		{
			m_hParent = GetForegroundWindow() ;
		}
		m_dResult = DoModal(m_hParent) ;			
		DWORD derr = ::GetLastError() ;
		DP((L"Result :[%d],Error[%d],Parent Window[%d]",m_dResult,derr,m_hParent)) ;
	}
	else
	{
		Create( m_hParent, NULL ) ;
		ShowWindow( SW_HIDE ) ;
	}
	return m_dResult ;
}
LRESULT CMainFrame::SetNext_OKCallBack( MAINFRAME::OnClichNextType pFunc, PVOID _pData )
{
	m_mainBtStrcture.pMainBtFunc = pFunc ;
	m_mainBtStrcture.pData = _pData ;
	return S_OK ;
}
LRESULT CMainFrame::SetCancelCallBack( MAINFRAME::ONCLICKCANCELBT pFunc, PVOID _pData ) 
{
	//m_listCancel.push_back( pFunc ) ;
	m_cancelBtStrcture.pMainBtFunc = pFunc ;
	m_cancelBtStrcture.pData = _pData ;
	return S_OK ;
}

LRESULT CMainFrame::OnProcessUserMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
	LRESULT hr = S_OK ;
	if( SC_CONTEXTHELP == uMsg ) 
	{
		bHandled = TRUE ;
		return S_OK ;
	}
	if( uMsg == RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) )
	{
		/*
		Ok process ...
		*/
		pafUI::DLGSTATUS dlgStatu = (pafUI::DLGSTATUS) wParam ;
	
		ProcessDlgStatus( dlgStatu, ID_NEXT_FINISHED, lParam ) ;
	}
	if(  uMsg == RegisterWindowMessage( PAF_UI_BTTONSATUS ) )
	{
		pafUI::BTSTATUS status = (pafUI::BTSTATUS)wParam ;
		ProcessBtStatus( status,ID_NEXT_FINISHED ) ;
	}
	if(  uMsg == RegisterWindowMessage( PAF_UI_END_PROGRESSBAR ))
	{
		EndProgressPanel() ;
		HWND hChild = GetDlgItem( ID_NEXT_FINISHED ) ;
		if( hChild ) 
		{
			::ShowWindow( hChild, SW_SHOW ) ;
		}
		hChild = GetDlgItem( IDCANCEL ) ;
		if( hChild ) 
		{
			::ShowWindow( hChild, SW_SHOW ) ;
		}
	}
	return hr ;
}
LRESULT CMainFrame::OnNextProcess(WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled;	//for warning C4100
	lParam;	//for warning C4100
	wParam;	//for warning C4100
	LRESULT hr = S_OK ;
	HWND hCtrl = this->GetDlgItem( ID_NEXT_FINISHED ) ;
	if( hCtrl )
	{
		::SetWindowText( hCtrl,MAINFRAME::m_szNextBuffer ) ;
		m_dlgStatus = MAINFRAME::DS_NEXT ;
	}
	else
	{
		S_FALSE ;
	}
	return hr ;
}
LRESULT CMainFrame::OnOKProcess(WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled;	//for warning C4100
	lParam;	//for warning C4100
	wParam;	//for warning C4100
	LRESULT hr = S_OK ;
	HWND hCtrl = this->GetDlgItem( ID_NEXT_FINISHED ) ;
	if( hCtrl )
	{
		::SetWindowText( hCtrl,MAINFRAME::m_szOKBuffer ) ;
		m_dlgStatus = MAINFRAME::DS_OK ;
	}
	else
	{
		S_FALSE ;
	}
	return hr ;
}
VOID CMainFrame::ProcessDlgStatus( INT _status, INT IDCtrl, LPARAM lParam  )
{
	HWND hOkBt = GetDlgItem( IDCtrl ) ;
	if( hOkBt )
	{
		switch( _status )
		{
		case pafUI::DS_OK :
			{
				if( lParam == NULL )
				{
					::SetWindowText( hOkBt,MAINFRAME::m_szOKBuffer ) ;
				}
				else
				{
					wchar_t* str = (wchar_t*)lParam ;
					::SetWindowText( hOkBt,str ) ;
				}
				m_dlgStatus = MAINFRAME::DS_OK ;
				//hr = OnOKProcess( wParam, lParam, bHandled ) ;
			}
			break ;
		case pafUI::DS_NEXT:
			{
				/*
				Next Process...
				*/
				if( lParam == NULL )
				{
					::SetWindowText( hOkBt,MAINFRAME::m_szNextBuffer ) ;
				}
				else
				{
					wchar_t* str = (wchar_t*)lParam ;
					::SetWindowText( hOkBt,str ) ;
				}
				m_dlgStatus = MAINFRAME::DS_NEXT ;
				//hr = OnNextProcess( wParam, lParam, bHandled ) ;
			}
			break ;
		case pafUI::DS_FINISHED:
			{
			}
			break ;
		case pafUI::DS_SENDMAIL:
			{
				if( lParam == NULL )
				{
					::SetWindowText( hOkBt,MAINFRAME::m_szSendMail ) ;
				}
				else
				{
					wchar_t* str = (wchar_t*)lParam ;
					::SetWindowText( hOkBt,str ) ;
				}
				m_dlgStatus = MAINFRAME::DS_SENDMAIL ;
			}
			break ;
		case pafUI::DS_DEFAULT:
		default:
			break ;
		}
	}
}
LRESULT CMainFrame::OnHelp( BOOL& bHandled)
{
	bHandled;	//for warning C4100
	// TODO: Add your message handler code here and/or call default
	//MessageBox( L"PAF UI Help",0,0 ) ;
	if( !this->m_strHelpURL.empty() )
	{
		::ShellExecute(::GetDesktopWindow() ,L"Open",m_strHelpURL.c_str(),NULL, NULL, SW_SHOWNORMAL ) ;
	}
	return 0;
}
LRESULT CMainFrame::AutoJustWindow() 
{
	HRESULT hr = S_OK ;
	RECT rc ;
	HWND hwnd = GetDlgItem( IDC_OBLIGATION_PANEL ) ;

	if( hwnd == NULL )
	{
		return 1 ;
	}
	::GetWindowRect( hwnd, &rc ) ;
	this->ScreenToClient( &rc ) ;
	::ShowWindow( hwnd, SW_HIDE ) ;
	RECT rcChild ;
	::GetWindowRect( m_hChildPanel, &rcChild ) ;
	INT iGapWidth = rcChild.right -rcChild.left - rc.right +rc.left ;
	INT iGapHeight  = rcChild.bottom - rcChild.top - rc.bottom +rc.top ;
	RECT rcMainDlg ;
	this->GetWindowRect( &rcMainDlg) ;
	MoveWindow( rcMainDlg.left -iGapWidth/2 ,rcMainDlg.top-iGapHeight/2,rcMainDlg.right-rcMainDlg.left +iGapWidth,rcMainDlg.bottom-rcMainDlg.top + iGapHeight,TRUE ) ;
	::GetWindowRect( hwnd, &rc ) ;
	ScreenToClient( &rc ) ;
	::MoveWindow( hwnd,rc.left ,rc.top ,rc.right-rc.left +iGapWidth,rc.bottom-rc.top + iGapHeight,TRUE ) ;
	::MoveWindow( m_hChildPanel,rc.left,rc.top,rcChild.right -rcChild.left ,rcChild.bottom - rcChild.top ,TRUE ) ;
	AutoJustControl( ID_NEXT_FINISHED, iGapWidth,  iGapHeight ) ;
	AutoJustControl( IDCANCEL, iGapWidth,  iGapHeight ) ;
	AutoJustControl( IDC_COMUNICATOR_PANEL, iGapWidth,  iGapHeight ) ;
	AutoJustControl( IDC_GROUP_PA_PANEL, iGapWidth,  iGapHeight ) ;
	
	/*
		EnumChildWindows  
	*/
	return hr ;
}
VOID CMainFrame::AutoJustControl( UINT ID, INT iGapWidth, INT iGapHeight )
{
	HWND hOkBt = GetDlgItem( ID ) ;
	if( hOkBt )
	{
		RECT rcBt ;
		::GetWindowRect( hOkBt, &rcBt ) ;
		ScreenToClient( &rcBt ) ;
		if( IDC_COMUNICATOR_PANEL == ID )
		{
			::MoveWindow( hOkBt,rcBt.left ,rcBt.top ,rcBt.right-rcBt.left ,rcBt.bottom-rcBt.top +iGapHeight,TRUE ) ;
			
			if((m_hCommunicatorPanel!=NULL) &&(::IsWindow(m_hCommunicatorPanel->m_hWnd) ))
			{
			::MoveWindow( m_hCommunicatorPanel->m_hWnd,rcBt.left ,rcBt.top ,rcBt.right-rcBt.left ,rcBt.bottom-rcBt.top +iGapHeight,TRUE ) ;
			}
			
		}
		else
			if(  IDC_GROUP_PA_PANEL == ID ) 
			{
				::MoveWindow( hOkBt,rcBt.left ,rcBt.top ,rcBt.right-rcBt.left+ iGapWidth ,rcBt.bottom-rcBt.top+iGapHeight ,TRUE ) ;
			}
			else
		{
			::MoveWindow( hOkBt,rcBt.left+ iGapWidth/2 ,rcBt.top +iGapHeight,rcBt.right-rcBt.left ,rcBt.bottom-rcBt.top ,TRUE ) ;
		}

	}
}
VOID CMainFrame::ProcessBtStatus( INT _status, INT IDCtrl )
{
	HWND hOkBt = GetDlgItem( IDCtrl ) ;
	if( hOkBt )
	{
		switch( _status )
		{
		case pafUI::BT_ENABLE:
			{
				::EnableWindow( hOkBt, TRUE ) ;
			}
			break ;
		case pafUI::BT_DISABLE:
			{
				::EnableWindow( hOkBt, FALSE ) ;
			}
			break ;
		case pafUI::BT_HIDE:
			{
				::ShowWindow( hOkBt, SW_HIDE ) ;
			}
			break ;
		default:
			{
				::ShowWindow( hOkBt, SW_SHOW ) ;
			}
		}
	}
}
LRESULT CMainFrame::Change_PA_Panel(const HWND _hNewPanelWnd ,
									const wchar_t* _pszHelpURL,
									const wchar_t* _pszTitle ,
									const wchar_t* _pszDescription  ) 
{
	_pszDescription;	//for warning C4100
	_pszTitle;	//for warning C4100
	::ShowWindow( m_hChildPanel, SW_HIDE ) ;
	m_hChildPanel = _hNewPanelWnd ;
	if( _pszHelpURL )
	{
		this->m_strHelpURL = _pszHelpURL ;
		LONG lRet = GetWindowLong( GWL_EXSTYLE ) ;
		lRet = SetWindowLong( GWL_EXSTYLE, lRet|WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT ) ;
	}
	else
	{
		m_strHelpURL.clear() ;
		LONG lRet = GetWindowLong( GWL_EXSTYLE ) ;
		lRet = SetWindowLong( GWL_EXSTYLE, lRet^WS_EX_CONTEXTHELP|WS_EX_CONTROLPARENT ) ;
	}
	DP((L"Begin change the child panel"));
	if( m_hChildPanel )
	{
		DP((L"Child window is not NULL; Hanlde:[%d]",m_hChildPanel));
		HWND hPre = ::SetParent( m_hChildPanel,this->m_hWnd ) ;
		::SetFocus(	 m_hChildPanel ) ;
		if( hPre == NULL )
		{
		//	MessageBox( L"Failure",0,0 ) ;
			DP((L"Pre Parent window is NULL:"));
			AutoJustWindow() ;
			::ShowWindow( m_hChildPanel, SW_SHOW ) ;
		}
		else
		{
			DP((L"Pre Parent window is no NULL: Pre Parent Handle:[%d]",hPre));
			AutoJustWindow() ;
			::ShowWindow( m_hChildPanel, SW_SHOW ) ;
		}
	}
	return S_OK ;
}
HWND CMainFrame::GetWindowHandle(VOID)
{
	if( (this->m_hWnd != NULL)&&(::IsWindow( this->m_hWnd ) ))
	{
		return this->m_hWnd ;
	}
	if( (m_tmpDlg.m_hWnd!=NULL)&&(::IsWindow( m_tmpDlg.m_hWnd ) ))
	{
		return m_tmpDlg.m_hWnd ;
	}
	INT iRepCount = 0 ;
	while(iRepCount<10)
	{
		if( m_tmpDlg.m_hWnd != NULL )
		{
			m_tmpDlg.DestroyWindow() ;
		}
		m_tmpDlg.m_hWnd = NULL ;
		DP((L"Origin create window failure\n"));
		if( ::IsWindow(m_hParent) )
		{
			wchar_t szClassName[101] = {0};
			//for warning C6387
			if (m_hParent)
			::GetClassName(m_hParent, szClassName, 100);
			HWND hParent = m_hParent;
			//if(_wcsicmp(szClassName, L"ConsoleWindowClass") == 0)
			if(_wcsicmp(szClassName, L"ConsoleWindowClass") == 0)	//for warning C4996
				hParent = ::GetDesktopWindow();
			if(hParent && ::IsWindow(hParent))
				m_tmpDlg.Create( hParent ) ;
			
		}
		else
		{
			HWND hwnd= GetDesktopWindow() ;
			if( ::IsWindow(hwnd) )
			{
				m_tmpDlg.Create(hwnd) ;
			}
		}
		if( (m_tmpDlg.m_hWnd!=NULL)&&(::IsWindow( m_tmpDlg.m_hWnd ) ))
		{
			DP((L"Origin create window failure, Now create new:[%d]\n",m_tmpDlg.m_hWnd));
			return m_tmpDlg.m_hWnd ;
		}
		iRepCount ++ ;
	}
	DP((L"Get Parent window failure! Temp window is NULL!\n"));
	return NULL ;
}
LRESULT CMainFrame::EndProgressPanel( void ) 
{
	HRESULT hr = S_OK ;
	if( m_dlgProgress )
	{
		m_dlgProgress->EndPrograssDlg() ;
		delete m_dlgProgress ;
		m_dlgProgress = NULL ;
	}
	HWND hChild = GetDlgItem( ID_NEXT_FINISHED ) ;
	if( hChild ) 
	{
		::ShowWindow( hChild, SW_SHOW ) ;
	}
	hChild = GetDlgItem( IDCANCEL ) ;
	if( hChild ) 
	{
		::ShowWindow( hChild, SW_SHOW ) ;
	}
	return hr ;
}

VOID CMainFrame::DoZBufLog( HWND hCurrWnd ) 
{
	DP((L"---------------------------------Begin-----------------------------------------") ) ;
	if( hCurrWnd != NULL )
	{
		DP((L"-----------------------------Next Window---------------------------------------------") ) ;
		HWND hWnd = ::GetNextWindow( hCurrWnd,GW_HWNDNEXT )  ;
		wchar_t szWndName[MAX_PATH] = {0} ;
		while(true)
		{

			if( hWnd == NULL )
			{
				break ;
			}
			::GetWindowText(  hWnd , szWndName, MAX_PATH ) ;

			DP((L"Next Window Log, Title [%s], Handle:[%d]", szWndName,	hWnd ) ) ;
			hWnd = ::GetNextWindow( hWnd,GW_HWNDNEXT )  ;
			::ZeroMemory( szWndName, MAX_PATH*sizeof(wchar_t ) ) ;
		}

		DP((L"------------------------------Self Window--------------------------------------------") ) ;
		::ZeroMemory( szWndName, MAX_PATH*sizeof(wchar_t ) ) ;
		::GetWindowText(  hCurrWnd , szWndName, MAX_PATH ) ;
		DP((L"Self Window Log, Title [%s], Handle:[%d]", szWndName,	hCurrWnd ) ) ;
		DP((L"-------------------------------Previous Window-------------------------------------------") ) ;
		hWnd = ::GetNextWindow( hCurrWnd,GW_HWNDPREV )  ;
		while( true )
		{
			::ZeroMemory( szWndName, MAX_PATH*sizeof(wchar_t ) ) ;

			if( hWnd == NULL )
			{
				break ;
			}
			::GetWindowText(  hWnd , szWndName, MAX_PATH ) ;
			DP((L"Pre Window Log, Title [%s], Handle:[%d]", szWndName,	hWnd ) ) ;
			hWnd = ::GetNextWindow( hWnd,GW_HWNDPREV )  ;
		}
		DP((L"------------------------------End--------------------------------------------") ) ;

	}
}
BOOL IsEqualString( wchar_t *str)
{
	if(::wcscmp(str, L"Untitled Message")   == 0 )
	{
		return TRUE ;
	}
	if(::wcscmp(str, L"Inbox - Microsoft Outlook")   == 0 )
	{
		return TRUE ;
	}
	if( ::wcscmp(str, L"Policy Assistant")   == 0 )
	{
		return TRUE ;
	}
	return FALSE ;
}
VOID CMainFrame::DoZBufLog_byOrder( HWND hCurrWnd ) 
{
	hCurrWnd;	//for warning C4100
	DP((L"---------------------------------Begin-----------------------------------------") ) ;
	HWND hWnd = ::GetTopWindow(	NULL ) ;

	wchar_t szWndName[MAX_PATH] = {0} ;
	while(true)
	{
		if( hWnd == NULL )
		{
			break ;
		}
		::GetWindowText(  hWnd , szWndName, MAX_PATH ) ;
		if( IsEqualString(szWndName ) == TRUE )
		{
			DP((L"Next Window Log, Title [%s], Handle:[%d]", szWndName,	hWnd ) ) ;
		}
		hWnd = ::GetNextWindow( hWnd,GW_HWNDNEXT )  ;
		::ZeroMemory( szWndName, MAX_PATH*sizeof(wchar_t ) ) ;

	}
	DP((L"------------------------------End--------------------------------------------") ) ;
}
LRESULT CMainFrame::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	//DoZBufLog_byOrder(	this->m_hWnd ) ;
	::SetWindowPos( this->m_hParent, HWND_TOP, 0,  0, 0,0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_NOZORDER | SWP_FRAMECHANGED*/ ) ; 
	//DoZBufLog_byOrder(	this->m_hWnd ) ;
	return 0;
}
PVOID CMainFrame::get_PAObjectPtr(void)
{
	if( m_mainBtStrcture.pData == NULL )
	{
		return m_cancelBtStrcture.pData ;
	}
	return m_mainBtStrcture.pData ;
}
