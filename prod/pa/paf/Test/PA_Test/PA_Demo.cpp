// PA_Demo.cpp : Implementation of CPA_Demo

#include "stdafx.h"
#include "PA_Demo.h"
#include "shlobj.h"

// CPA_Demo
LRESULT CPA_Demo::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CPA_Demo>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	HICON hIcon = (HICON)::LoadImage(m_hInst, MAKEINTRESOURCE(IDI_ICON_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(m_hInst, MAKEINTRESOURCE(IDI_ICON_MAIN), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);
	SetMiddlePosition() ;
	HWND hWnd = this->GetDlgItem( IDC_LIST_OUTPUT ) ;
	m_FileMngr.SetOutPutWnd( hWnd ) ;
	m_hOutPutWnd =	hWnd ;
	return 1;  // Let the system set the focus
}



LRESULT CPA_Demo::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
VOID CPA_Demo::SetMiddlePosition() 
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
}

BOOL CPA_Demo::OpenFileGetPath( wchar_t *pszFileName )
{
	BOOL bRet = FALSE ;
	if( pszFileName ==  NULL )
	{
		return bRet ;
	}
	OPENFILENAME ofn; 
	wchar_t szFile[MAX_PATH+1] = {0} ;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	bRet = GetOpenFileName(&ofn) ;
	if( bRet == TRUE )
	{
		::memcpy_s( pszFileName, MAX_PATH*sizeof(wchar_t), szFile, MAX_PATH*sizeof(wchar_t) ) ;
	}

	return bRet ;
}
BOOL CPA_Demo::GetSaveFilePath( wchar_t *pszFilePath )
{
	BOOL bRet = FALSE ;
	if( pszFilePath ==  NULL )
	{
		return bRet ;
	}
	//OPENFILENAME ofn; 
	wchar_t szFile[MAX_PATH+1] = {0} ;

	BROWSEINFO bi;  
	bi.hwndOwner= this->m_hWnd;  
	bi.pidlRoot=NULL;  
	bi.pszDisplayName=szFile;  
	bi.lpszTitle=L"Get Target Path";  
	bi.ulFlags= BIF_NEWDIALOGSTYLE|BIF_USENEWUI|BIF_SHAREABLE;  
	bi.lpfn=NULL;  
	bi.lParam=NULL;  
	bi.iImage=NULL;  

	if( SHGetPathFromIDList(SHBrowseForFolder(&bi),szFile))
	{
		bRet = TRUE ;
		::memcpy_s( pszFilePath, MAX_PATH*sizeof(wchar_t), szFile, MAX_PATH*sizeof(wchar_t) ) ;
	}
	return bRet ;

}
LRESULT CPA_Demo::OnBnClickedButtonGetSource(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code her
	::ZeroMemory( m_szSrcFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
	if( OpenFileGetPath(   m_szSrcFileName ) == TRUE )
	{
		HWND hwnd = this->GetDlgItem(	IDC_EDIT_SOURCE_FILE_NAME )  ;
		if( hwnd )
		{
			::SetWindowText(hwnd,  m_szSrcFileName ) ;
			/*	if( wcslen( m_szDestFileName ) != 0 )
			{
			wchar_t drive[_MAX_DRIVE] = {0} ;
			wchar_t dir[_MAX_DIR]= {0} ;
			wchar_t fname[_MAX_FNAME] = {0} ;
			wchar_t ext[_MAX_EXT] = {0} ;
			::_wsplitpath(	 m_szSrcFileName, drive,	 dir,	fname,ext ) ;
			::wcscat_s(	 fname,	 _MAX_FNAME, ext ) ;
			::wcscat_s(	 m_szDestFileName,	 MAX_PATH, L"\\" ) ;
			::wcscat_s(	 m_szDestFileName,	 MAX_PATH, fname ) ;
			HWND hwnd = this->GetDlgItem(	IDC_EDIT_DEST_FILE_NAME )  ;
			if( hwnd )
			{
			::SetWindowText( hwnd, m_szDestFileName ) ;
			}
			::ZeroMemory( drive,  _MAX_DRIVE*sizeof(wchar_t)) ;
			::ZeroMemory( dir,  _MAX_DIR*sizeof(wchar_t)) ;
			::ZeroMemory( fname,  _MAX_FNAME*sizeof(wchar_t)) ;
			::ZeroMemory( ext,  _MAX_EXT*sizeof(wchar_t)) ;
			}*/
		}
	}
	return 0;
}

LRESULT CPA_Demo::OnBnClickedButtonGetDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	::ZeroMemory( m_szDestFileName, (MAX_PATH+1)*sizeof( wchar_t ) ) ;
	if( GetSaveFilePath(   m_szDestFileName ) == TRUE )
	{
		/*if( wcslen( m_szSrcFileName ) != 0 )
		{
		wchar_t drive[_MAX_DRIVE] = {0} ;
		wchar_t dir[_MAX_DIR]= {0} ;
		wchar_t fname[_MAX_FNAME] = {0} ;
		wchar_t ext[_MAX_EXT] = {0} ;
		::_wsplitpath(	 m_szSrcFileName, drive,	 dir,	fname,ext ) ;
		::wcscat_s(	 fname,	 _MAX_FNAME, ext ) ;
		::wcscat_s(	 m_szDestFileName,	 MAX_PATH, L"\\" ) ;
		::wcscat_s(	 m_szDestFileName,	 MAX_PATH, fname ) ;
		::ZeroMemory( drive,  _MAX_DRIVE*sizeof(wchar_t)) ;
		::ZeroMemory( dir,  _MAX_DIR*sizeof(wchar_t)) ;
		::ZeroMemory( fname,  _MAX_FNAME*sizeof(wchar_t)) ;
		::ZeroMemory( ext,  _MAX_EXT*sizeof(wchar_t)) ;
		}*/
		::wcsncat_s(	 m_szDestFileName,	 MAX_PATH, L"\\", _TRUNCATE ) ;
		HWND hwnd = this->GetDlgItem(	IDC_EDIT_DEST_FILE_NAME )  ;
		if( hwnd )
		{
			::SetWindowText( hwnd, m_szDestFileName ) ;
		}
	}
	return 0;
}

LRESULT CPA_Demo::OnBnClickedMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if( CheckSrc_DestFile() == FALSE )
	{
		return 0 ;
	}
	try{
		DO_OUTPUT_STRING( m_hOutPutWnd, L"Begin simulate the action of Move "  ) ;
		DP((L"Begin simulate the action of Move:Source[%s];Destination[%s]",m_szSrcFileName, m_szDestFileName ) ) ;
		m_FileMngr.DoAction_Move( this->m_szSrcFileName,this->m_szDestFileName ) ;
		DP((L"End simulate the action of Move:Source[%s];Destination[%s]",m_szSrcFileName, m_szDestFileName ) ) ;
		DO_OUTPUT_STRING( m_hOutPutWnd, L"End simulate the action of Move "  ) ;
	}
	catch(...)
	{
		return 0 ;
	}
	return 0;
}

LRESULT CPA_Demo::OnClickedTestCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if( CheckSrc_DestFile() == FALSE )
	{
		return 0 ;
	}
	try{
		DO_OUTPUT_STRING( m_hOutPutWnd, L"Begin simulate the action of Copy "  ) ;
		DP((L"Begin simulate the action of Copy:Source[%s];Destination[%s]",m_szSrcFileName, m_szDestFileName ) ) ;
		m_FileMngr.DoAction_Copy( this->m_szSrcFileName,this->m_szDestFileName ) ;
		DO_OUTPUT_STRING( m_hOutPutWnd, L"End simulate the action of Copy "  ) ;
		DP((L"End simulate the action of Copy:Source[%s];Destination[%s]",m_szSrcFileName, m_szDestFileName ) ) ;
	}
	catch(...)
	{
		return 0 ;
	}
	return 0;
}
BOOL CPA_Demo::CheckSrc_DestFile() 
{
	BOOL bRet = FALSE ;

	::ZeroMemory(   m_szSrcFileName, MAX_PATH*sizeof(wchar_t) ) ;

	HWND hwnd = this->GetDlgItem(	IDC_EDIT_SOURCE_FILE_NAME )  ;
	if( hwnd )
	{
		::GetWindowText( hwnd, m_szSrcFileName,MAX_PATH ) ;
	}
	if( wcslen( m_szSrcFileName ) == 0) 
	{
		MessageBox( L"Please click the button of source to add the source file name!", L"Source File is Empty",	MB_OK|MB_ICONWARNING ) ;

		return bRet ;
	}
	::ZeroMemory(   m_szDestFileName, MAX_PATH*sizeof(wchar_t) ) ;

	hwnd = this->GetDlgItem(	IDC_EDIT_DEST_FILE_NAME )  ;
	if( hwnd )
	{
		::GetWindowText( hwnd, m_szDestFileName,MAX_PATH ) ;
	}
	if( wcslen( m_szDestFileName ) == 0) 
	{
		MessageBox( L"Please click the button of destination to add the destination file path!", L"Destination File is Empty",	MB_OK|MB_ICONWARNING ) ;
		return bRet ;
	}
	bRet = TRUE ;
	return bRet ;
}
LRESULT CPA_Demo::OnBnClickedButtonStub(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	if( CheckSrc_DestFile() == FALSE )
	{
		return 0 ;
	}

	m_FileMngr.DoStub( this->m_szSrcFileName,this->m_szDestFileName ) ;
	return 0;
}
