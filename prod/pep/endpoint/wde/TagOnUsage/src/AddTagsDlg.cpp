// AddTagsDlg.cpp : Implementation of CAddTagsDlg

#include "stdafx.h"
#include "AddTagsDlg.h"
#include "AddTagsMgr.h"
#include "Tagging.h"
#include <Wingdi.h>
#include <Windows.h>
#include <ShellAPI.h>

#define  MAX_CHARACTERS_LINE          40

using namespace std;
// CAddTagsDlg
LRESULT CAddTagsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CAddTagsDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	m_pMgr = CAddTagsMgr::GetInstance();

	if(m_pMgr)
	{
		HICON hIcon = LoadIcon(m_pMgr->GetApplicationInstance(), MAKEINTRESOURCE(IDI_NXT_LOGO));
		if(hIcon)
			SetIcon(hIcon);
	}
	
	::ShowWindow( GetDlgItem(IDC_OURLOGO), SW_HIDE );

	InitHeader();

	::SetWindowText(GetDlgItem(IDC_FILEPATH), L"");

	RECT rc;
	DWORD dwBkColor = GetSysColor( COLOR_BTNFACE);
	::GetWindowRect(GetDlgItem(IDC_TAGLIST), &rc);
	ScreenToClient(&rc);

	m_lvTags.SubclassWindow(GetDlgItem(IDC_TAGLIST));
	m_lvTags.ForceMeasureItemMessage();
	m_lvTags.InsertColumn(0, NL_LISTVIEW_TEXT, LVCFMT_LEFT, 145, 0);
	m_lvTags.InsertColumn(1, NL_LISTVIEW_COMBOBOX, LVCFMT_LEFT, 110, 0);

	m_lvTags.SetBkColor(dwBkColor);

	::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

	m_lvFile.SubclassWindow(GetDlgItem(IDC_FILELIST));
	m_lvFile.DeleteAllItems();
	m_lvFile.GetWindowRect(&rc);
	m_lvFile.SetBkColor(dwBkColor);
	m_lvFile.SetExtendedStyle(m_lvFile.GetExtendedStyle() 
		| LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	{
		LVCOLUMN lvColumn;

		ZeroMemory(&lvColumn, sizeof(lvColumn));
		lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_IMAGE | LVCFMT_CENTER;
		lvColumn.cx = (rc.right - rc.left - 16);
		m_lvFile.InsertColumn(0, &lvColumn);
	}

	m_hImglist = ImageList_Create(16,16,ILC_COLOR32,0,5); 

	// 	HIMAGELIST oldImgLst = ListView_SetImageList(m_hListView, m_hImglist, LVSIL_SMALL); 
	HIMAGELIST oldImgLst = m_lvFile.SetImageList(m_hImglist, LVSIL_SMALL);

	ImageList_Destroy(oldImgLst); 

	unsigned uTotalCount = 0;
	if(!UpdateItem(uTotalCount))
	{
		g_log.Log(CELOG_DEBUG, L"addTags::Show UI failed.");
		SendMessage( WM_COMMAND, IDCANCEL);
		return 1;
	}
	
	if(uTotalCount == 0)
	{
		g_log.Log(CELOG_DEBUG, L"addTags::InitDialog, no item needs to show.");
		SendMessage( WM_COMMAND, IDCANCEL);
		return 1;
	}
	
	UpdateCount(uTotalCount);

	m_hFontBold = CreateFontW(16,
		0,
		0,
		0,
		FW_BOLD,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Segoe UI"
		);

	m_hDefaultFontComboBox = CreateFontW(16,
		0,
		0,
		0,
		FW_NORMAL,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		L"Segoe UI"
		);

	m_lvTags.SetTextFont(m_hDefaultFontComboBox);
	m_lvTags.SetComboBoxFont(m_hDefaultFontComboBox);

	
	ShowWindow(SW_SHOWDEFAULT);
	SetForegroundWindow(m_hWnd);
//	CenterWindow();

	return 1;  // Let the system set the focus
}

BOOL CAddTagsDlg::UpdateItem(unsigned& nTotalCount, BOOL bCheckTagCount/*FALSE*/)
{
	if(!m_pMgr)
	{
		return FALSE;
	}

	::EnterCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	
	wstring::size_type uCount = m_pMgr->m_listItems_Show.size();
	if(uCount > 0)
	{
		ITEMINFO info = m_pMgr->m_listItems_Show.front();//get the first item in the list
		::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);

		if(bCheckTagCount && info.vTags.size() == (unsigned)m_lvTags.GetItemCount())
		{//don't need to update UI.
			return TRUE;
		}

		//show prompt
		std::wstring strPromptLabel;
		if(info.strPrompt.length() > 0)
		{
			strPromptLabel = info.strPrompt;
		}
		else
		{
			wchar_t szPrompt[201] = {0};
			LoadStringW(m_pMgr->GetApplicationInstance(), IDS_PROMPT, szPrompt, 200);
			strPromptLabel = std::wstring(szPrompt);
		}

		SetDlgItemText(IDC_PROMPT, strPromptLabel.c_str());
		howEditScrollbar(IDC_PROMPT);

		//Show File path
		m_strFilePath = info.strFilePath;
		ShowFilePath(m_strFilePath.c_str());//show file path on the dialog. use "\r\n" if the file path is too long.

		m_lvTags.DeleteAllItems();

		std::vector<TAGINFO>::iterator itr;
		int nIndex = 0;

		std::map<std::wstring, std::wstring> mapTags;
		CTagging::ReadTags(m_strFilePath.c_str(), mapTags, NULL);

		for(itr = info.vTags.begin(); itr != info.vTags.end(); itr++)
		{
			std::wstring strValue = mapTags[(*itr).strTagName];

			m_lvTags.InsertItem(nIndex, (*itr).strTagName.c_str());
			m_lvTags.InsertComboBox(nIndex, 1, (*itr).vTagValues, strValue);
			
			nIndex++;
		}

		nTotalCount = static_cast<unsigned>(uCount);

		if(info.bOptional)
		{
			
			::ShowWindow(GetDlgItem(IDC_CANCEL), SW_SHOW);
		}
		else
		{
			
			::ShowWindow(GetDlgItem(IDC_CANCEL), SW_HIDE);
		}
	}
	else
	{
		::LeaveCriticalSection(&CCriSectionMgr::m_CriItemList_Shown);
	}
	
	return uCount > 0? TRUE: FALSE;
}

void CAddTagsDlg::ShowFilePath(LPCWSTR lpszFilePath)
{
	if(!lpszFilePath)
	{
		return;
	}

	m_lvFile.DeleteAllItems();
	
	SHFILEINFO   sfi;   

	SHGetFileInfo(lpszFilePath, 0, &sfi, sizeof(sfi),SHGFI_ICON);   
	if (!sfi.hIcon)
	{
		sfi.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	}

	HICON hIcon   =   sfi.hIcon;  
	

	std::wstring strFileName = lpszFilePath;
	
	LVITEM		lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	
	lvItem.mask = LVIF_IMAGE | LVIF_TEXT;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = (LPWSTR)strFileName.c_str();
	lvItem.cchTextMax = 300;

	int nRet = 0;
	ImageList_RemoveAll(m_hImglist);
	if(hIcon)
		nRet = ImageList_AddIcon(m_hImglist, hIcon);

	lvItem.iImage = 0;

	if(sfi.hIcon)
	{
		DestroyIcon(sfi.hIcon);
	}

	nRet = m_lvFile.InsertItem(&lvItem);
}

LRESULT CAddTagsDlg::OnBnClickedNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	std::map<std::wstring, std::wstring> mapTags;
	for(int i = 0; i < m_lvTags.GetItemCount(); i++)
	{
		wchar_t szTagName[2000] = {0};
		m_lvTags.GetItemTextEx(i, 0, szTagName, 1999);

		wchar_t szTagValue[5000] = {0};
		m_lvTags.GetItemTextEx(i, 1, szTagValue, 4999);

		mapTags[szTagName] = szTagValue;

	}
	g_log.Log(CELOG_DEBUG, L"Try to add tag\r\n");
	//Check if the current file can be tagged.
	DWORD dwErrorID = 0;
	if(!CanTag(m_strFilePath.c_str(), dwErrorID))
	{
		PopupErrorDlg(m_hWnd, m_pMgr->GetApplicationInstance(), dwErrorID);
	}
	else
	{
		//Add tag
		if(!CTagging::AddTag(m_strFilePath.c_str(), mapTags, m_hWnd))
		{
			PopupErrorDlg(m_hWnd, m_pMgr->GetApplicationInstance(), IDS_TAGGING_ERROR);
		}
	}
	

	//remove this item in list after tagging.
	if(m_pMgr)
	{
		m_pMgr->RemoveFirstShownItem();

		if(m_pMgr->NoMoreShownItems())
		{
			ShowWindow(SW_HIDE);
			
			UpdateCount(0);
			ShowWindow(SW_HIDE);
		}
		else
		{
			unsigned uTotalCount = 0;
			UpdateItem(uTotalCount);
			UpdateCount(uTotalCount);
		}
	}

	return 0;
}

void CAddTagsDlg::UpdateCount(unsigned uCount)
{
	if(m_hWnd)
	{
		SetDlgItemInt(IDC_FILECOUNT, uCount);

		if(uCount > 1)
		{
			wchar_t szNext[101] = {0};
			LoadStringW(m_pMgr->GetApplicationInstance(), IDS_NEXT, szNext, 100);

			::SetWindowText( GetDlgItem(IDC_NEXT), szNext);
		}
		else
		{
			wchar_t szOK[101] = {0};
			LoadString(m_pMgr->GetApplicationInstance(), IDS_OK, szOK, 100);

			::SetWindowText( GetDlgItem(IDC_NEXT), szOK);
		}
	}
}

LRESULT CAddTagsDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default

	return 0;
}

LRESULT CAddTagsDlg::OnUpdateItemCount(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(m_pMgr)
	{
		unsigned uCount = m_pMgr->GetShownItem();
		if ( /*(GetDlgItemInt(IDC_FILECOUNT) != uCount ) &&*/ uCount > 0)
		{
			if(m_hWnd)
			{
				if(!::IsWindowVisible(m_hWnd))
				{
					ShowWindow(SW_SHOW);
					unsigned uTotalCount = 0;
					UpdateItem(uTotalCount);
				}

				unsigned uTotalCount = 0;
				UpdateItem(uTotalCount, TRUE);
				UpdateCount(uCount);
			}

		}
	}
	return 0;
}
LRESULT CAddTagsDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// TODO: Add your message handler code here and/or call default
	if(m_hImglist)
	{
		ImageList_Destroy(m_hImglist);
		m_hImglist = NULL;
	}

	m_lvFile.UnsubclassWindow(TRUE);
	return 0;
}

LRESULT CAddTagsDlg::OnBnClickedCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	//remove this item in list after tagging.
	if(m_pMgr)
	{
		m_pMgr->RemoveFirstShownItem();

		if(m_pMgr->NoMoreShownItems())
		{
			ShowWindow(SW_HIDE);

			UpdateCount(0);
			ShowWindow(SW_HIDE);
		}
		else
		{
			unsigned uTotalCount = 0;
			UpdateItem(uTotalCount);
			UpdateCount(uTotalCount);
		}
	}
	return 0;
}

void CAddTagsDlg::howEditScrollbar( DWORD dwID)
{
	HWND hWnd = ::GetDlgItem(m_hWnd, dwID);
	if(!hWnd)
		return;

	int nLines = (int)SendMessageW(hWnd, EM_GETLINECOUNT, 0, 0);

	if(nLines > 6)//show scroll bar
	{
		::SetWindowLongPtrW(hWnd, GWL_STYLE, ::GetWindowLongPtrW(hWnd, GWL_STYLE) | WS_VSCROLL);
		::SendMessageW(hWnd, WM_VSCROLL, SB_TOP, NULL);
	}
	else
	{
		::SetWindowLongPtrW(hWnd, GWL_STYLE, ::GetWindowLongPtrW(hWnd, GWL_STYLE) & ~WS_VSCROLL);
	}
}

void CAddTagsDlg::InitHeader()
{
	HFONT hFont = ::CreateFontW( 0,0,0,11,FW_NORMAL,FALSE,FALSE,FALSE, ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,L"Segoe UI" ) ;
	HWND hHeader = ::GetDlgItem(m_hWnd, IDC_TITLE) ;
	::SendMessageW(hHeader, WM_SETFONT,(WPARAM)hFont,(LPARAM)0 ) ;


	CImageBase imgBase ;
	imgBase.InitImage( &m_imgBackground,m_pMgr->GetApplicationInstance(), IDB_LOGO  ) ;
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

LRESULT CAddTagsDlg::OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hCtrl = ::GetDlgItem(m_hWnd, IDC_TITLE) ;
	if( (HWND)lParam == hCtrl )
	{
		SetTextColor((HDC)wParam,   0x993300);   
		SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE));   

		::SetWindowLongPtr(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   

		return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 

	}
	SetTextColor((HDC)wParam,   0x404040);   
	SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE));   

	::SetWindowLongPtr(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   

	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
}

LRESULT CAddTagsDlg::OnPaintDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint( &ps);

	if( !m_imgBackground.IsNull())
	{
		HWND hLogo = ::GetDlgItem(this->m_hWnd, IDC_OURLOGO);
		if (hLogo)
		{
			RECT rc ;
			::GetWindowRect( hLogo, &rc ) ;
			this->ScreenToClient( &rc) ;

			m_imgBackground.Draw(hdc,rc.left,rc.top ) ;
		}
		
	}

	EndPaint(&ps);

	return 0;
}