#include "StdAfx.h"
#include "FileTagViewDlg.h"
#include "PromptDlg.h"
#include "Utils.h"


typedef struct struSortParam
{
	int					nSortType;
	int					nSortColumn;
	CFileTagViewDlg*	pDlg;
} SORTPARAM, *LPSORTPARAM;

static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{

	SORTPARAM* pSortParam = (SORTPARAM*)lParamSort;
	if(!pSortParam)
		return 0;

	int nIndex1 = -1, nIndex2 = -1;

	LVFINDINFO info; 

	info.flags = LVFI_PARAM;                        
	info.lParam = lParam1;                          
	nIndex1 = (int)ListView_FindItem(pSortParam->pDlg->GetDlgItem(IDC_TAGSLIST), -1, &info);

	info.lParam = lParam2;
	nIndex2 = (int)ListView_FindItem(pSortParam->pDlg->GetDlgItem(IDC_TAGSLIST), -1, &info);

	if(nIndex1 < 0 || nIndex2 < 0)
		return 0;

	wchar_t szBuf1[201] = {0};
	ListView_GetItemText(pSortParam->pDlg->GetDlgItem(IDC_TAGSLIST), nIndex1, pSortParam->nSortColumn, szBuf1, 200);
	wchar_t szBuf2[201] = {0};
	ListView_GetItemText(pSortParam->pDlg->GetDlgItem(IDC_TAGSLIST), nIndex2, pSortParam->nSortColumn, szBuf2, 200);

	if(!pSortParam || !szBuf1 || !szBuf2)
		return 0;

	int nCompRes = wcscmp(szBuf1, szBuf2);

	return pSortParam->nSortType == 0? nCompRes: -nCompRes;
}

CFileTagViewDlg::CFileTagViewDlg(void)
{
	m_pMgr = NULL;
	m_nSortType = 0;
	m_bSort = FALSE;
}

CFileTagViewDlg::~CFileTagViewDlg(void)
{
}

LRESULT CFileTagViewDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(!m_pMgr)
	{
		return 0;
	}
	::SendMessage(GetDlgItem(IDC_TAGSLIST).m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE,0, LVS_EX_FULLROWSELECT);

	LVCOLUMN col1;
	LVCOLUMN col2;

	ZeroMemory(&col1, sizeof(LV_COLUMN));
	ZeroMemory(&col2, sizeof(LV_COLUMN));


	wchar_t szTagName[101] = {0};
	LoadStringW(m_pMgr->GetCurrentInstance(), IDS_TAG_NAME, szTagName, 100);

	wchar_t szTagValue[101] = {0};
	LoadStringW(m_pMgr->GetCurrentInstance(), IDS_TAG_VALUE, szTagValue, 100);

	col1.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM; // 
	col1.cx = 160;
	col1.pszText = szTagName;

	col2.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM; // 
	col2.cx = 250;
	col2.pszText = szTagValue;

	::SendMessage(GetDlgItem(IDC_TAGSLIST).m_hWnd, LVM_INSERTCOLUMN,0, (LPARAM)&col1);
	::SendMessage(GetDlgItem(IDC_TAGSLIST).m_hWnd, LVM_INSERTCOLUMN,1, (LPARAM)&col2);

	if(!ShowTags(m_strFileName.c_str()))
	{
		::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
		return 0;
	}
	SetDlgItemText(IDC_FILENAME, m_strFileName.c_str());
	::SendMessage(GetDlgItem(IDC_FILENAME), EM_SETSEL, m_strFileName.length() - 1, -1);//fix bug263

	m_pMgr->GetFileTag()->SetParentWnd(m_hWnd);

	::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), FALSE);
//	OutputDebugStringW(L"Disable remove Button\r\n");
	HICON hIcon = LoadIcon(m_pMgr->GetCurrentInstance(), MAKEINTRESOURCE(IDI_LOGO));
	if(hIcon)
		SetIcon(hIcon);

	CenterWindow();
	return 0;
}

BOOL CFileTagViewDlg::ShowTags(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return FALSE;

	SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_DELETEALLITEMS,0, 0);

	std::list<smart_ptr<FILETAG_PAIR>> listTags;
	listTags.clear();
	BOOL bRet = GetAllTagsEx(pszFileName, m_pMgr, &listTags);

	if(!bRet)
	{
		return FALSE;
	}

	std::list<std::wstring> listAllOurTags;
	std::list<std::wstring>::iterator n_itr;
	listAllOurTags.clear();
	std::wstring strValue;
	BOOL bIndexTag = GetIndexTag(&listTags, pszFileName, strValue);
	if(bIndexTag)
		Split_NXT_Tags(strValue, listAllOurTags);
	if(listAllOurTags.size() <= 0)
	{
		DP((L"The content of \"Index tag\" is empty!\r\n"));
		bIndexTag = FALSE;
	}

	std::list<smart_ptr<FILETAG_PAIR>>::iterator itr;
	SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_DELETEALLITEMS,0, 0);
	int i = 0;
	for(itr = listTags.begin(); itr != listTags.end(); itr++)
	{
		smart_ptr<FILETAG_PAIR> spTag = *itr;
		
		std::wstring strTagName, strTagValue;
		strTagName = spTag->strTagName;
		strTagValue = spTag->strTagValue;

		//check whether the tag is our tag. Kevin 2008-9-18
	
		if(!TagExists(strTagName.c_str(), listAllOurTags, n_itr))
			continue;
	

		LVITEM item;
		item.mask = LVIF_TEXT|LVIF_PARAM;       // 
		item.cchTextMax = MAX_PATH;       // 
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = (LPWSTR)strTagName.c_str();
		item.lParam = i;

		SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_INSERTITEM,0, (LPARAM)&item);

		item.mask = LVIF_TEXT;
		item.iSubItem = 1;
		item.pszText = (LPWSTR)strTagValue.c_str();
		SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_SETITEM,0, (LPARAM)&item);


		i++;
	}
	
	if(m_bSort)//kevin Zhou 2008-10-8
	{
		SORTPARAM param;
		
		param.nSortColumn = m_nSortColumn;
		param.nSortType = m_nSortType?0: 1;
		param.pDlg = this;

		ListView_SortItems(GetDlgItem(IDC_TAGSLIST), (PFNLVCOMPARE)ListCompare, (LPARAM)&param);//sort
	}


	::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), FALSE);

	return TRUE;
}

void CFileTagViewDlg::SetFileName(LPCWSTR pszFileName)
{
	if(!pszFileName)
		return;

	m_strFileName = std::wstring(pszFileName);
}

LRESULT CFileTagViewDlg::OnBnClickedRemove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here
	int nCount = 0;
	nCount = (int)SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_GETSELECTEDCOUNT,0, 0);

	if(nCount <= 0)
	{
		wchar_t szWarning[501] = {0};
		LoadStringW(m_pMgr->GetCurrentInstance(), IDS_REMOVE_WARNING, szWarning, 500);

		MessageBox(szWarning, L"Warnning", MB_OK|MB_ICONWARNING);
		return 0;
	}

	wchar_t szHint[501] = {0};
	LoadStringW(m_pMgr->GetCurrentInstance(), IDS_REMOVE_HINT, szHint, 500);

	wchar_t szCaption[501] = {0};
	LoadStringW(m_pMgr->GetCurrentInstance(), IDS_CAPTION, szCaption, 500);

	if(IDCANCEL == MessageBox(szHint, szCaption, MB_OKCANCEL|MB_ICONQUESTION))//fix bug287
	{
		::SetFocus(GetDlgItem(IDC_TAGSLIST).m_hWnd);
		return 0;
	}


	int nSelected;
	int nBegin = -1;
	std::list<int> listSelected;
	for(int i = 0; i < nCount; i++)
	{
		nSelected = (int)SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_GETNEXTITEM, nBegin, LVNI_SELECTED);
		if(nSelected >= 0)
		{
			listSelected.push_back(nSelected);
			nBegin = nSelected;
		}
	}

	std::list<int>::iterator itr;
	std::list<std::wstring> listSelectedTagNames;
	wchar_t buffer[1001] = {0};
	LVITEM item;
	ZeroMemory(&item, sizeof(LVITEM));

	item.mask = LVIF_TEXT;       // 
	item.cchTextMax = 1000;       // 

	item.pszText = buffer;


	//get all tag names which has been selected by user.
	std::list<std::wstring> listTagsNeedRemove;
	listTagsNeedRemove.clear();
	std::list<smart_ptr<FILETAG_PAIR>> listPairTagsNeedRemove;
	listPairTagsNeedRemove.clear();
	for(itr = listSelected.begin(); itr != listSelected.end(); itr++)
	{
		item.iItem = *itr;
		item.iSubItem = 0;

		SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_GETITEM,0, (LPARAM)&item);
		listTagsNeedRemove.push_back(item.pszText);

		smart_ptr<FILETAG_PAIR> spPair(new FILETAG_PAIR);
		spPair->strTagName = std::wstring(item.pszText);
		spPair->strTagValue = L"";
		listPairTagsNeedRemove.push_back(spPair);
	}	


	RemoveTagEx(&listTagsNeedRemove, m_strFileName.c_str(), m_pMgr);

	ShowTags(m_strFileName.c_str());
	return 0;
}

LRESULT CFileTagViewDlg::OnKillFocus(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& /*bHandled*/)
{
	::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), FALSE);
	return 0;
}

LRESULT CFileTagViewDlg::OnSetFocus(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& /*bHandled*/)
{
	int nCount = 0;
	nCount = (int)SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_GETSELECTEDCOUNT,0, 0);

	if(nCount > 0)
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), TRUE);
	else
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), FALSE);

	return 0;
}

LRESULT CFileTagViewDlg::OnLvnItemchangedListtags(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	pNMLV;
	// TODO: Add your control notification handler code here

	int nCount = 0;
	nCount = (int)SendMessage(GetDlgItem(IDC_TAGSLIST), LVM_GETSELECTEDCOUNT,0, 0);

	if(nCount > 0)
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), TRUE);
	else
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_REMOVE), FALSE);


	return 0;
}

LRESULT CFileTagViewDlg::OnColumnClick(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	m_bSort = TRUE;
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	SORTPARAM param;
	m_nSortColumn = pNMListView->iSubItem;
	param.nSortColumn = pNMListView->iSubItem;
	param.nSortType = m_nSortType;
	param.pDlg = this;

	ListView_SortItems(GetDlgItem(IDC_TAGSLIST), (PFNLVCOMPARE)ListCompare, (LPARAM)&param);//sort

	//Update the header of listview
	HWND hHeader = ListView_GetHeader(GetDlgItem(IDC_TAGSLIST));

	HDITEM header;
	header.mask = HDI_TEXT ;
	wchar_t buffer[201] = {0};
	header.pszText = buffer;
	header.cchTextMax = 200;
	SendMessage(hHeader, HDM_GETITEM, param.nSortColumn, (LPARAM)&header);

	HBITMAP hBmp = NULL;
	if(m_nSortType == 0)
	{
		m_nSortType = 1;
		hBmp = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ARROW_UP));
	}
	else if(m_nSortType == 1)
	{
		m_nSortType = 0;
		hBmp = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ARROW_DOWN));
	}

	header.mask|= HDI_BITMAP|HDI_FORMAT;
	header.hbm = hBmp;
	header.fmt = HDF_STRING|HDF_LEFT|HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
	
	SendMessage(hHeader, HDM_SETITEM, param.nSortColumn, (LPARAM)&header);

	//remove all up/down arrows for other headers
	int nHeaderCount = (int)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0);
	for(int i = 0; i < nHeaderCount; i++)
	{
		if(i != param.nSortColumn)
		{
			header.mask = HDI_TEXT ;
			wchar_t szHeader[201] = {0};
			header.pszText = szHeader;
			header.cchTextMax = 200;
			SendMessage(hHeader, HDM_GETITEM, i, (LPARAM)&header);

			header.mask|= HDI_FORMAT;
			
			header.fmt = HDF_STRING|HDF_LEFT;

			SendMessage(hHeader, HDM_SETITEM, i, (LPARAM)&header);
		}
	}

	

	return 0;
}
