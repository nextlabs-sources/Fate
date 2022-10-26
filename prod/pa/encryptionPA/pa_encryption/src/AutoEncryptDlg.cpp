// EncryptionDlg.cpp : Implementation of CEncryptionDlg

#include "stdafx.h"

#include "AutoEncryptDlg.h"
#include "UIInterface.h"
#include "ImageBase.h"
#include <shellapi.h>
#pragma warning(push)
#pragma warning(disable: 6386)
#include <atlwin.h>
#pragma warning(pop)

using namespace pafUI;

extern HINSTANCE g_hInstance;

// static void EllipsisPathText(const std::wstring &wstrOriginalPath, std::wstring &wstrEllipsisText,
// 							 DWORD dwRestrictedWidth, DWORD dwOriginalWidth);

// BOOL EA_GetEncryptStatus(HWND hParentWnd, BOOL &bEncrypt, BOOL bOptional)
// {
// 	CAutoEncryptDlg dlg(bOptional);
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
// 	bEncrypt = dlg.get_Encrypt();
// 
// 	return dlg.get_Cancel();
// }
VOID CAutoEncryptDlg::InitHeaderStyle(VOID)
{
	HFONT hFont = ::CreateFontW( 0,0,0,11,FW_NORMAL,FALSE,FALSE,FALSE, ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,L"Segoe UI" ) ;
	HWND hHeader = ::GetDlgItem(m_hWnd, IDC_GROUPHEADER) ;
	::SendMessageW(hHeader, WM_SETFONT,(WPARAM)hFont,(LPARAM)0 ) ;
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
// CAutoEncryptDlg
LRESULT CAutoEncryptDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CAutoEncryptDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

// 	HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
// 	this->SetIcon(hIcon,TRUE);
	InitHeaderStyle() ;
	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_CERT_DESC1), m_wstrDescription1.c_str());
// 	::SetWindowTextW(::GetDlgItem(m_hWnd, IDC_CERT_DESC2), m_wstrDescription2.c_str());

	CheckDlgButton(IDC_ENCRYPT_CHECK, m_bEncrypt);

	if (!m_bOptional)
	{
 		::EnableWindow(::GetDlgItem(m_hWnd, IDC_ENCRYPT_CHECK), FALSE);
	}

	LONG lRet = ::GetWindowLong( m_hWnd, GWL_STYLE ) ;
	lRet = ::SetWindowLong( m_hWnd, GWL_STYLE, lRet|WS_TABSTOP ) ; 

	m_listView.SubclassWindow(GetDlgItem(IDC_CERT_FILE_LIST));

	UpdateListView();

	bHandled = TRUE;
	return 1;  // Let the system set the focus
}

// LRESULT CAutoEncryptDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
// {
// 	m_bEncrypt = IsDlgButtonChecked(IDC_ENCRYPT_CHECK);
// 
// 	m_bCancel = FALSE;
// 
// 	EndDialog(wID);
// 
// 	return 0;
// }
// 
// LRESULT CAutoEncryptDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
// {
// 	m_bCancel = TRUE;
// 
// 	EndDialog(wID);
// 	return 0;
// }
LRESULT CAutoEncryptDlg::OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
LRESULT CAutoEncryptDlg::OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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
	SetTextColor((HDC)wParam,   0x000000);   
	SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE));   
#ifdef _WIN64
	::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
	::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
}
#if 1
void CAutoEncryptDlg::UpdateListView( void )
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
		lvColumn.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		lvColumn.cx = (rc.right - rc.left - 30);
		m_listView.InsertColumn(0, &lvColumn);
	}

	HIMAGELIST hImglist = ImageList_Create(16,16,ILC_COLOR32,0,5); 

	HIMAGELIST oldImgLst = m_listView.SetImageList(hImglist, LVSIL_SMALL);  

	ImageList_Destroy(oldImgLst); 

	for (iterFile = m_listFiles.begin(); iterFile != m_listFiles.end(); iterFile++, nIndex++)
	{
		ZeroMemory(&lvItem,sizeof(LVITEM));

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
		lvItem.pszText = (LPWSTR)(*iterFile).second.c_str();
		lvItem.cchTextMax = (int)((*iterFile).second.size() + 1); // cast to int is necessary because on x64, size_t is 64 bit and int is 32 bit.  I assume this value does not exceed 32 bit max integer value.

		iRet = m_listView.InsertItem(&lvItem); 

		lvInfoTip.cbSize = sizeof(lvInfoTip);
		lvInfoTip.dwFlags = 0;
		lvInfoTip.pszText = (LPWSTR)(*iterFile).second.c_str();
		lvInfoTip.iItem = iRet;
		lvInfoTip.iSubItem = 0;
		m_listView.SetInfoTip(&lvInfoTip);
	}
}

static void EllipsisPathText(const std::wstring &wstrOriginalPath, std::wstring &wstrEllipsisText,
					  DWORD dwRestrictedWidth, DWORD dwOriginalWidth)
{
    size_t dwPathLen = wstrOriginalPath.size();
    size_t dwNeedCutLen = 0;
    size_t dwRestrictedLen = 0;
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
	        size_t dwPrefixLen = dwPathLen - dwNeedCutLen - wcslen(lpwzLastSlash);
		wstrEllipsisText = wstrOriginalPath.substr(0, dwPrefixLen);
		wstrEllipsisText += L"...";
		wstrEllipsisText += lpwzLastSlash;
	}

	return;
}
#endif
