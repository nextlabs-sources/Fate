#include "StdAfx.h"
#include "FileTagPanel.h"
#include <shellapi.h>
#include "UIInterface.h"
#include "Utils.h"
#include "PromptDlg.h"
#include "WorkThreads.h"
#include "ImageBase.h"
#include <boost/algorithm/string.hpp>

CFileTagPanel::CFileTagPanel(void)
{
	m_hParentWnd = NULL;
	m_hFontBold = NULL;
	m_pMgr = NULL;
	m_hImglist = NULL;

	m_hWnd = NULL;//added by kevin 2009-1-17
}

CFileTagPanel::~CFileTagPanel(void)
{
	if(m_hFontBold)
		DeleteObject(m_hFontBold);
}

void CALLBACK CFileTagPanel::OnClickOK(PVOID pData, LONG /*status*/, HWND hParent )
{
	CFileTagMgr* pMgr = (CFileTagMgr*)pData;
	if(!pMgr)
	{
		DP((L"The pointer of CFileTagMgr from OnClickOK is NULL.\r\n"));
		return;
	}
	if(pMgr->m_pFileTagPanel)//fix bug7609 kevin 2008-10-6
	{
		if(!pMgr->m_pFileTagPanel->CheckTagValues())
		{
			std::wstring strText = pMgr->GetResString(IDS_TAGVALUE_EMPTY);
			std::wstring strCaption = pMgr->GetResString(IDS_CAPTION);
			::MessageBoxW(hParent, strText.c_str(), strCaption.c_str(), MB_OK|MB_ICONERROR );
			return;
		}
	}

	//do current file tag
	BOOL bRet = pMgr->DoCurrentManualTagging();
	if (!bRet)
	{
		if (g_bLoadByOE)
		{
			smart_ptr<FILETAG_ITEM> spItem = smart_ptr<FILETAG_ITEM>(NULL);
			if (pMgr->GetCurrentItem(spItem) && _wcsicmp(spItem->strTagOnError.c_str(), ERROR_ACTION_BLOCK) == 0)
			{
				::EndDialog(hParent, IDCANCEL);
				hParent = NULL;
				return;
			}
			else
			{
				//spItem->strTagOnError = "continue"
			}
		}
		else
		{
			::EndDialog(hParent, PA_ERROR);
			hParent = NULL;
			return;
		}
	}
	

    //Get Next item
	CFileTagPanel* pPanel = NULL;	
	smart_ptr<FILETAG_ITEM> spNextItem;
	BOOL bNextItem= false;
	pPanel = NULL;
	pMgr->NextFileTaggingItem(spNextItem, bNextItem, pPanel);//Kevin 2008-10-7


	if(bNextItem)
	{
		DWORD dwRet = ::GetFileAttributesW(spNextItem->strFile.c_str());

		if(0xFFFFFFFF == dwRet)
		{
			PABase::ATTRIBUTELIST pa_listAttr;
			std::wstring szIdentifier(L"identifier");
			pMgr->AddLog(szIdentifier, pMgr->GetResString(IDS_FILETAG_DISPLAYNAME), pMgr->GetResString(IDS_FILE_NOT_EXIST), pMgr->GetResString(IDS_FILETAG_DESCRIPTION), pMgr->GetResString(IDS_FILETAG_TAG_FAIL), pa_listAttr);


		//	DP((L"File not exist\r\n"));
			PopupMessageBox(IDS_FILE_NOT_EXIST, spNextItem->strFile.c_str(), pPanel->m_hWnd);

			::EndDialog(hParent, PA_ERROR);//Cancel the file tagging panel if tagging failed. kevin zhou
			hParent = NULL;
		}
		else
		{
			FILETAG_ITEM item;
			item.strFile = spNextItem->strFile;
			item.strHint = spNextItem->strHint;
			item.listTags = spNextItem->listTags;
			item.strDisplayFile = spNextItem->strDisplayFile;
			if(pPanel)
			{
				if(!pPanel->PrepareItem(item))
				{
					::EndDialog(hParent, PA_ERROR);
					hParent = NULL;
				}
			}
		}
	}
	else
	{
	//	SendMessage(hParent, WM_CLOSE, 0, 0);
		::EndDialog(hParent, IDOK);
		hParent = NULL;
	}
	

	if(pPanel && hParent)
	{
		pPanel->m_hParentWnd = hParent;
		pPanel->UpdateButtons();
	}

	return;
}

BOOL CFileTagPanel::PrepareItem(FILETAG_ITEM& item)
{
	m_lvFile.DeleteAllItems();
	m_lvTags.DeleteAllItems();

	SHFILEINFO   sfi;   

	SHGetFileInfo(item.strFile.c_str(),0,&sfi,sizeof(sfi),SHGFI_ICON);   
	HICON hIcon   =   sfi.hIcon;   

//	if(!hIcon)
//		return FALSE;

	//get file name
	std::wstring strFileName = item.strDisplayFile;
	if (g_bLoadByOE)
	{
		size_t nPos = item.strDisplayFile.find_last_of(L'\\');
		if (nPos != std::wstring::npos)
		{
			strFileName = item.strDisplayFile.substr(nPos + 1);
		}
	}
	else if (item.strDisplayFile.length() > 130)
	{
		strFileName = item.strDisplayFile.substr(0, 60);
		strFileName = strFileName + L"...";
		strFileName = strFileName + item.strDisplayFile.substr(item.strDisplayFile.length() - 60, 60);
	}


	LVITEM		lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));
	LVSETINFOTIP lvInfoTip;
	lvItem.mask = LVIF_IMAGE | LVIF_TEXT;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = (LPWSTR)strFileName.c_str();
	lvItem.cchTextMax = (int)(strFileName.length()+1);

	int nRet = 0;
	ImageList_RemoveAll(m_hImglist);//fix bug317
	if(hIcon)
		nRet = ImageList_AddIcon(m_hImglist, hIcon);

	lvItem.iImage = 0;
	

	nRet = m_lvFile.InsertItem(&lvItem);

	lvInfoTip.cbSize = sizeof(lvInfoTip);
	lvInfoTip.dwFlags = 0;
	lvInfoTip.pszText = L"tool tip";
	lvInfoTip.iItem = nRet;
	lvInfoTip.iSubItem = 0;
	m_lvFile.SetInfoTip(&lvInfoTip);
//	m_lvFile.SetFont(m_hNormalFont);


	//show the tag values

	std::list<smart_ptr<FILETAG>>::iterator itr;
	int nIndex = 0;
	for(itr = item.listTags.begin(); itr != item.listTags.end(); itr++)
	{
		smart_ptr<FILETAG> spTag = *itr;

	//	m_lvTags.AddItem(spTag->strTagName.c_str(), m_hFontBold, RGB(0, 0, 0), NULL, 0, TRUE, 200, 30 );

		std::list<std::wstring>::iterator v_itr;
		std::vector<std::wstring> vTags;
		for(v_itr = spTag->listValues.begin(); v_itr != spTag->listValues.end(); v_itr++)
		{
			std::wstring strValue = *v_itr;
			vTags.push_back(strValue);
		}

		m_lvTags.InsertItem(nIndex, spTag->strTagName.c_str());
		m_lvTags.InsertComboBox(nIndex, 1, vTags);

		nIndex++;
	}


	LPGET_TAGVALUE_BYNAME_THREAD_PARAM pParam = new GET_TAGVALUE_BYNAME_THREAD_PARAM;
	BOOL bRet = TRUE;
	if(pParam)
	{
		CPromptDlg dlg;
		dlg.SetEndFlag(FALSE);
		dlg.SetPathInfo(item.strFile.c_str());
		pParam->pItem = &item;
		pParam->pMgr = m_pMgr;

		if(NeedPopupPromptDlg(item.strFile.c_str()))
		{
			pParam->pDlg = &dlg;
			unsigned dwThreadID = 0;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &GetTagValueByNameThread, pParam, 0, &dwThreadID );
			if(!hThread)
			{
				DP((L"Failed to begin thread for GetTagValueByNameThread()\r\n"));
				if(pParam)
					delete pParam;
				return FALSE;
			}
			dlg.DoModal();
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}
		else
		{
			pParam->pDlg = NULL;
			GetTagValueByNameThread(pParam);
		}
		
		bRet = pParam->bSuccess;
		bRet = TRUE;//fix bug439 KEVIN 2008-11-16 
		if(!bRet)
		{
			HWND hWnd = m_hWnd && ::IsWindow(m_hWnd)?m_hWnd:m_pMgr->GetMailWindow();

			PopupMessageBox(pParam->dwErrorID, item.strFile.c_str(), hWnd);
		}

		delete pParam;
	}
	
	if(hIcon)
		DeleteObject(hIcon);

	SetDlgItemText(IDC_HINT, item.strHint.c_str());

	ShowEditScrollbar(IDC_HINT);

	return bRet;
}

LRESULT CFileTagPanel::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CFileTagPanel>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	m_hFontBold = CreateFontW(16,
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

	InitHeaderStyle() ;
	RECT rc;
/*		::GetWindowRect(GetDlgItem(IDC_FILELIST), &rc);
		ScreenToClient(&rc);
		m_lvFile.Create(m_hWnd, rc);
	m_lvFile.SetBackGroundColor(dwBkColor);*/

	DWORD dwBkColor = GetSysColor( COLOR_BTNFACE);

	m_lvTags.SubclassWindow(GetDlgItem(IDC_TAGLIST));
	m_lvTags.ForceMeasureItemMessage();
	m_lvTags.InsertColumn(0, NL_LISTVIEW_TEXT, LVCFMT_LEFT, 145, 0);
	m_lvTags.InsertColumn(1, NL_LISTVIEW_COMBOBOX, LVCFMT_LEFT, 110, 0);
	m_lvTags.SetBkColor(dwBkColor);
	m_lvTags.SetTextFont(m_hFontBold);
	m_lvTags.SetComboBoxFont(m_hFontBold);


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
		lvColumn.cx = (rc.right - rc.left - 10);
		m_lvFile.InsertColumn(0, &lvColumn);
	}

	m_hImglist = ImageList_Create(16,16,ILC_COLOR32,0,5); 

	// 	HIMAGELIST oldImgLst = ListView_SetImageList(m_hListView, m_hImglist, LVSIL_SMALL); 
	HIMAGELIST oldImgLst = m_lvFile.SetImageList(m_hImglist, LVSIL_SMALL);

	ImageList_Destroy(oldImgLst); 

	return 1;  // Let the system set the focus
}

LRESULT CFileTagPanel::OnPaintDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps;
	HDC hdc;
	
	hdc = BeginPaint( &ps);

	EndPaint(&ps);

	return 0;
}

void CFileTagPanel::DrawLine(int nLeft, int nTop, int nWidth)
{
	HDC hDC = ::GetDC(m_hWnd);

	HPEN   hPen   =   CreatePen(PS_SOLID, 1, RGB(0, 0, 0));  

	HPEN   old_hpen   =   (HPEN)SelectObject(hDC,hPen);//

	POINT pt;
	BOOL bRet = MoveToEx(hDC, nLeft, nTop, &pt);  
	
	bRet = LineTo(hDC, nLeft + nWidth, nTop );  

	
	SelectObject(hDC,old_hpen);  
	DeleteObject(hPen);  

	::ReleaseDC(m_hWnd, hDC);   

	/*Gdiplus::Graphics graphics(hDC);

	// Create a Pen object.
	Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0), 3);

	// Create two Point objects that define the line.
	Gdiplus::Point point1(10, 50);
	Gdiplus::Point point2(1000, 50);

	// Draw the line.
	graphics.DrawLine(&blackPen, point1, point2);

*/
}

BOOL CFileTagPanel::GetTagValues(std::list<smart_ptr<FILETAG_PAIR>>* pList)
{
	if(!pList)
		return FALSE;

	for(int i = 0; i < m_lvTags.GetItemCount(); i++)
	{
		wchar_t szTagName[2000] = {0};
		m_lvTags.GetItemTextEx(i, 0, szTagName, 1999);

		wchar_t szTagValue[5000] = {0};
		m_lvTags.GetItemTextEx(i, 1, szTagValue, 4999);

		smart_ptr<FILETAG_PAIR> pair(new FILETAG_PAIR);
		pair->strTagName = szTagName;
		pair->strTagValue = szTagValue;

		pList->push_back(pair);
	}
	return TRUE;
}

BOOL CFileTagPanel::CheckTagValues()
{
	for(int i = 0; i < m_lvTags.GetItemCount(); i++)
	{
		wchar_t szTagValue[MAX_PATH + 1] = {0};
		m_lvTags.GetItemTextEx(i, 1, szTagValue, MAX_PATH);

		if(wcslen(szTagValue) == 0)
			return FALSE;
	}
	return TRUE;
}

void CFileTagPanel::UpdateButtons()
{
	if(m_pMgr)
	{
		std::wstring strText = m_pMgr->GetOKButtonText();

		::SendMessageW( m_hParentWnd, g_MainWindowMsgID, (WPARAM)pafUI::DS_NEXT, (LPARAM) (strText.c_str())) ;
	}
	
}

LRESULT CFileTagPanel::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(m_hImglist)
	{
		ImageList_Destroy(m_hImglist);
		m_hImglist = NULL;
	}
	m_lvFile.UnsubclassWindow(TRUE);
	m_lvTags.UnsubclassWindow(TRUE);
	return 0;
}
VOID CFileTagPanel::InitHeaderStyle(VOID) 
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
LRESULT CFileTagPanel::OnEraseBackGround(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled;lParam;uMsg;	// just for warning of c4100
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
	
	m_imgBackground.Draw((HDC)wParam,rc.left,rc.top ) ;
	bRet = TRUE ;
	return FALSE ;

}
LRESULT CFileTagPanel::OnCtrlColorsStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled;uMsg;
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
	SetTextColor((HDC)wParam,   0x404040);   
	SetBkColor((HDC)wParam,   GetSysColor(COLOR_BTNFACE));   
#ifdef _WIN64
	::SetWindowLong(m_hWnd,   DWLP_MSGRESULT,   (LONG)TRUE);   
#else
	::SetWindowLong(m_hWnd,   DWL_MSGRESULT,   (LONG)TRUE);   
#endif
	return   (LRESULT)GetSysColorBrush(COLOR_BTNFACE); 
}

void CFileTagPanel::ShowEditScrollbar( DWORD dwID)
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