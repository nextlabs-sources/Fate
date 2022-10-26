// EncryptionDlg.cpp : Implementation of CEncryptionDlg

#include "stdafx.h"

#include "PasswordDlg.h"
#include "UIInterface.h"
#include "ImageBase.h"
#include <shellapi.h>
#include <boost/algorithm/string.hpp>

using namespace pafUI;

extern HINSTANCE g_hInstance;

BOOL CheckPasswrodKeyValue( wchar_t *psd)
{
	BOOL bhasUpper = FALSE ;
	BOOL bHasData = FALSE ;
	int iLen =  (int) wcslen(psd) ;
	for( int i=0 ;i<iLen; ++i)
	{
		if( psd[i]>='A' && psd[i]<='Z')
		{
			bhasUpper =TRUE ;
		}
		if( psd[i]>='0' && psd[i]<='9')
		{
			bHasData =TRUE ;
		}
	}
	return bhasUpper&bHasData ;
}
// static void EllipsisPathText(const std::wstring &wstrOriginalPath, std::wstring &wstrEllipsisText,
// 							 DWORD dwRestrictedWidth, DWORD dwOriginalWidth);

// BOOL EA_GetPasswordFromUser(HWND hParentWnd, std::wstring &wstrPassword, BOOL bOptional)
// {
// 	CPasswordDlg dlg(bOptional);
// 
// 	if (hParentWnd)
// 	{
// 		dlg.DoModal(hParentWnd);
// 	}
// 	else
// 	{
// 		dlg.DoModal();
// 	}
// 
// 	wstrPassword = dlg.get_Password();
// 
// 	return dlg.get_Cancel();
// }

// CEncryptionDlg
VOID CPasswordDlg::InitHeaderStyle(VOID)
{
	HFONT hFont = ::CreateFontW( 0,0,0,11,FW_NORMAL,FALSE,FALSE,FALSE, ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,L"Segoe UI" ) ;
	HWND hHeader = ::GetDlgItem(m_hWnd, IDC_GROUPHEADER) ;
	::SendMessageW(hHeader, WM_SETFONT,(WPARAM)hFont,(LPARAM)0 ) ;

}
VOID CPasswordDlg::InitControlStyle(VOID) 
{
	

	CImageBase imgBase ;
	imgBase.InitImage( &m_imgBackground,g_hInstance, IDB_COMPANY_LOGO  ) ;
	for(int i = 0; i < m_imgBackground.GetWidth(); i++)
	{
		for(int j = 0; j < m_imgBackground.GetHeight(); j++)
		{
			unsigned char* pucColor = reinterpret_cast<unsigned char *>(m_imgBackground.GetPixelAddress(i , j));
			pucColor[0] = pucColor[0] * pucColor[3] / 255;
			pucColor[1] = pucColor[1] * pucColor[3] / 255;
			pucColor[2] = pucColor[2] * pucColor[3] / 255;
		}
	}

}

void CPasswordDlg::ShowEditScrollbar(DWORD dwID)
{
	HWND hWnd = ::GetDlgItem(m_hWnd, dwID);
	if(!hWnd)
		return;

	int nLines = (int)SendMessageW(hWnd, EM_GETLINECOUNT, 0, 0);

	if(nLines > 3)//show scroll bar
	{
		::SetWindowLongPtrW(hWnd, GWL_STYLE, ::GetWindowLongPtrW(hWnd, GWL_STYLE) | WS_VSCROLL);
	}
	else
	{
		::SetWindowLongPtrW(hWnd, GWL_STYLE, ::GetWindowLongPtrW(hWnd, GWL_STYLE) & ~WS_VSCROLL);
	}
}

LRESULT CPasswordDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CPasswordDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	// 	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	// 	this->SetIcon(hIcon,TRUE);
	
	InitHeaderStyle() ;
	InitControlStyle() ;
	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_SYM_DESC1), m_wstrDescription1.c_str());

	ShowEditScrollbar(IDC_SYM_DESC1);
// 	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_SYM_DESC2), m_wstrDescription2.c_str());

	CheckDlgButton(IDC_SYM_ENCRYPT, m_bEncrypt);
	
	::SendMessage(::GetDlgItem(m_hWnd, IDC_PASSWORD1), EM_SETLIMITTEXT, (WPARAM)127, 0);
	::SendMessage(::GetDlgItem(m_hWnd, IDC_PASSWORD2), EM_SETLIMITTEXT, (WPARAM)127, 0);

	if (!m_bOptional)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_SYM_ENCRYPT), FALSE);
	}

	if (m_bEncrypt)
	{
		UINT iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD1), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD2), FALSE);
	}

	LONG lRet = ::GetWindowLong( m_hWnd, GWL_STYLE ) ;
	lRet = ::SetWindowLong( m_hWnd, GWL_STYLE, lRet|WS_TABSTOP ) ; 

	m_listView.SubclassWindow(GetDlgItem(IDC_SYMM_FILE_LIST));

	UpdateListView();

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

// LRESULT CPasswordDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
// {
// 	WCHAR wzPassword1[128];
// 	WCHAR wzPassword2[128];
// 
// 	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD1), wzPassword1, 128);
// 	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD2), wzPassword2, 128);
// 
// 	if (wcscmp(wzPassword1, wzPassword2))
// 	{
// 		::MessageBox(m_hWnd, L"The Passwords you input don't match. Please try again!",
// 			L"Policy Assistant", MB_OK);
// 
// 		return 0;
// 	}
// 
// 	m_wstrPassword = wzPassword1;
// 
// 	m_bCancel = FALSE;
// 
// 	EndDialog(wID);
// 	return 0;
// }
// 
// LRESULT CPasswordDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
// {
// 	m_bCancel = TRUE;
// 
// 	EndDialog(wID);
// 	return 0;
// }

LRESULT CPasswordDlg::OnEnChangePassword1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CAxDialogImpl<CPasswordDlg>::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	WCHAR wzPassword1[128];
	WCHAR wzPassword2[128];

	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD1), wzPassword1, 128);
	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD2), wzPassword2, 128);

	UINT iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;

	if (!wzPassword1[0] && !wzPassword2[0])
	{
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else if(wcslen(wzPassword1)<6||wcslen(wzPassword2)<6)
	{
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else if(CheckPasswrodKeyValue(wzPassword1) == FALSE)
	{
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else if (wcscmp(wzPassword1, wzPassword2))
	{
// 		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else
	{
// 		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_ENABLE ,0 ) ;
	}

	return 0;
}

LRESULT CPasswordDlg::OnEnChangePassword2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CAxDialogImpl<CPasswordDlg>::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	WCHAR wzPassword1[128];
	WCHAR wzPassword2[128];

	UINT iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;

	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD1), wzPassword1, 128);
	::GetWindowText(::GetDlgItem(m_hWnd, IDC_PASSWORD2), wzPassword2, 128);
	if (!wzPassword1[0] && !wzPassword2[0])
	{
		if (!m_bOptional)
		{
// 			::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
			::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
		}
		else
		{
// 			::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);
			::PostMessage( ::GetParent(m_hWnd), iMsg, BT_ENABLE ,0 ) ;
		}
	}
	else if(wcslen(wzPassword1)<6||wcslen(wzPassword2)<6)
	{
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else if(CheckPasswrodKeyValue(wzPassword1) == FALSE)
	{
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else if (wcscmp(wzPassword1, wzPassword2))
	{
// 		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), FALSE);
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_DISABLE ,0 ) ;
	}
	else
	{
// 		::EnableWindow(::GetDlgItem(m_hWnd, IDOK), TRUE);
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_ENABLE ,0 ) ;
	}

	return 0;
}

LRESULT CPasswordDlg::OnBnClickedSymEncrypt(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	m_bEncrypt = IsDlgButtonChecked(IDC_SYM_ENCRYPT);
	if (m_bEncrypt)
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD1), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD2), TRUE);

		BOOL bStub = FALSE;
		OnEnChangePassword1(0, 0, NULL, bStub);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD1), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PASSWORD2), FALSE);

		UINT iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;
		::PostMessage( ::GetParent(m_hWnd), iMsg, BT_ENABLE ,0 ) ;
	}
	return 0;
}
#define USER32_IDI_INFORMATION 104
VOID CPasswordDlg::RedrawInfoIcon(HDC hDC)
{
	HWND hBanCtrl = ::GetDlgItem(m_hWnd, IDC_BANNER_FLAG) ;
	if( m_hIcon == NULL )
	{
		HMODULE hMod = GetModuleHandle(L"user32.dll");
		m_hIcon = (HICON) ::LoadImage(hMod, MAKEINTRESOURCE(USER32_IDI_INFORMATION), IMAGE_ICON, 16, 16, LR_SHARED);
	}
	RECT rc ;
	::GetWindowRect( hBanCtrl, &rc ) ;
	this->ScreenToClient( &rc) ;
	::ShowWindow( hBanCtrl, SW_HIDE ) ;
	HBRUSH  br  = CreateSolidBrush(0x9BE8FB) ;

	FillRect( hDC,&rc,br ) ;
	DrawIconEx(hDC,rc.left,rc.top,m_hIcon,16,16, 0, NULL, DI_NORMAL | DI_COMPAT ); 

}
LRESULT CPasswordDlg::OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{


	HWND hCtrl = ::GetDlgItem(m_hWnd, IDC_GROUPHEADER) ;
	if( (HWND)lParam == hCtrl )
	{
		SetTextColor((HDC)wParam,   0x993300);   
		SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE)); 
#ifdef _WIN64
		::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
		::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif
		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 

	}
	hCtrl = ::GetDlgItem(m_hWnd, IDC_BANNER_MSG) ;
	if( (HWND)lParam == hCtrl )
	{
		if(m_brBanner == NULL )
		{
			m_brBanner = ::CreateSolidBrush(0x9BE8FB );
		}
		SetTextColor((HDC)wParam,   GetSysColor(COLOR_WINDOWTEXT));   
		SetBkColor((HDC)wParam,   0x9BE8FB);   
#ifdef _WIN64
		::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
		::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif  
		return(LRESULT) m_brBanner ;
	}
	hCtrl = ::GetDlgItem(m_hWnd, IDC_BANNER_FLAG) ;
	if( (HWND)lParam == hCtrl )
	{
		if(m_brBanner == NULL )
		{
			m_brBanner = ::CreateSolidBrush(0x9BE8FB );
		}
		SetTextColor((HDC)wParam,   GetSysColor(COLOR_WINDOWTEXT));   
		SetBkColor((HDC)wParam,   0x9BE8FB);   
#ifdef _WIN64
		::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
		::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif
		return(LRESULT) m_brBanner ;
	}
	
	SetTextColor((HDC)wParam,   0x404040);   
	SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE));   
#ifdef _WIN64
	::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
	::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
}

LRESULT CPasswordDlg::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	HDC hDC = GetDC();
	BeginPaint( &ps );
	RedrawInfoIcon(hDC) ;
	EndPaint( &ps );
	return 0 ;
}
LRESULT CPasswordDlg::OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	BOOL bRet = TRUE ;
	if( m_imgBackground.IsNull() )
	{
		return bRet ;
	}
	HWND hLogo = ::GetDlgItem( this->m_hWnd,IDC_LOGO_PICTURE) ;
	if( hLogo == NULL )
	{
		return bRet ;
	}
	RECT rc ;
	::GetWindowRect( hLogo, &rc ) ;
	this->ScreenToClient( &rc) ;
	::ShowWindow( hLogo, SW_HIDE ) ;
//	INT iWidth = rc.right -rc.left ;
//	INT iHeight = rc.bottom- rc.top ;
	m_imgBackground.Draw((HDC)wParam,rc.left,rc.top ) ;
	bRet = TRUE ;
	return FALSE ;

}
#if 1
void CPasswordDlg::UpdateListView( void )
{
	SHFILEINFO  sfi;
	LVITEM		lvItem;
	std::list<std::pair<std::wstring, std::wstring>>::iterator iterFile;
	UINT32		nIndex = 0;
	int			iRet = 0;
	RECT		rc;
	COLORREF	color;
	LVSETINFOTIP lvInfoTip;

	color = GetSysColor( COLOR_BTNFACE);

	m_listView.DeleteAllItems();
	m_listView.GetWindowRect(&rc);
	m_listView.SetBkColor(color);
	m_listView.SetExtendedStyle(m_listView.GetExtendedStyle() 
		| LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	{
		LVCOLUMN lvColumn;

		ZeroMemory(&lvColumn, sizeof(lvColumn));
		lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_IMAGE | LVCFMT_CENTER;
		lvColumn.cx = (rc.right - rc.left - 30);
		m_listView.InsertColumn(0, &lvColumn);
	}

	HIMAGELIST hImglist = ImageList_Create(16,16,ILC_COLOR32,0,5); 

	HIMAGELIST oldImgLst = m_listView.SetImageList(hImglist, LVSIL_SMALL); 

	ImageList_Destroy(oldImgLst); 

	//is load by outlook
	wchar_t wszExeFileName[1024 + 1] = { 0 };
	GetModuleFileNameW(NULL, wszExeFileName, 1024);
	BOOL bLoadByOutlook = boost::algorithm::iends_with(wszExeFileName, L"outlook.exe");

	for (iterFile = m_listFiles.begin(); iterFile != m_listFiles.end(); iterFile++, nIndex++)
	{
		ZeroMemory(&lvItem,sizeof(LVITEM));

		//get file name(not contains the path)
		LPCWSTR lpwszFileName = iterFile->second.c_str();
		if (bLoadByOutlook)
		{
			size_t nPos = iterFile->second.find_last_of(L'\\');
			if (nPos != std::wstring::npos)
			{
				lpwszFileName = iterFile->second.c_str() + nPos + 1;
			}
		}
		int ncchMaxFileName = wcslen(lpwszFileName) + 1;

		SHGetFileInfo((*iterFile).first.c_str(),0,&sfi,sizeof(sfi),SHGFI_ICON);
		if (!sfi.hIcon)
		{
			sfi.hIcon = LoadIconW(NULL, IDI_APPLICATION);
// 			continue;
		}
		iRet = ImageList_AddIcon(hImglist, sfi.hIcon);
		DestroyIcon(sfi.hIcon);

		lvItem.iImage = iRet; 

		lvItem.mask = LVIF_IMAGE | LVIF_TEXT;
		lvItem.iItem = nIndex;
		lvItem.iSubItem = 0;
		lvItem.pszText = (LPWSTR)lpwszFileName;
		lvItem.cchTextMax = ncchMaxFileName;

		iRet = m_listView.InsertItem(&lvItem); 

		lvInfoTip.cbSize = sizeof(lvInfoTip);
		lvInfoTip.dwFlags = 0;
		lvInfoTip.pszText = (LPWSTR)lpwszFileName;
		lvInfoTip.iItem = iRet;
		lvInfoTip.iSubItem = 0;
		m_listView.SetInfoTip(&lvInfoTip);
		
	}
}

static void EllipsisPathText(const std::wstring &wstrOriginalPath, std::wstring &wstrEllipsisText,
							 DWORD dwRestrictedWidth, DWORD dwOriginalWidth)
{
	DWORD dwPathLen =(DWORD) wstrOriginalPath.size();
	DWORD dwNeedCutLen = 0;
	DWORD dwRestrictedLen = 0;
	LPWSTR lpwzLastSlash = NULL;

	if (dwOriginalWidth <= dwRestrictedWidth)
	{
		wstrEllipsisText = wstrOriginalPath;
		return;
	}

	dwNeedCutLen = (dwOriginalWidth - dwRestrictedWidth) * dwPathLen / dwOriginalWidth + 9;
	dwRestrictedLen = dwPathLen - dwNeedCutLen;

	lpwzLastSlash = (LPWSTR)wcsrchr(wstrOriginalPath.c_str(), L'\\');
	if (wcslen(lpwzLastSlash)+3 > dwRestrictedLen)
	{
		wstrEllipsisText = wstrOriginalPath.substr(0, 3);
		wstrEllipsisText += L"...";
		wstrEllipsisText += (lpwzLastSlash + wcslen(lpwzLastSlash)+3 - dwRestrictedLen);
	}
	else
	{
		DWORD dwPrefixLen = dwPathLen - dwNeedCutLen - ((DWORD) wcslen(lpwzLastSlash));
		wstrEllipsisText = wstrOriginalPath.substr(0, dwPrefixLen);
		wstrEllipsisText += L"...";
		wstrEllipsisText += lpwzLastSlash;
	}

	return;
}
#endif
