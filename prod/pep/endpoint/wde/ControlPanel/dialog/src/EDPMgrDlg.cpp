#include "StdAfx.h"
#include "EDPMgrDlg.h"
#include "SysUtils.h"
#include "CEsdk.h"
#include "HelpDlg.h"
#include "EDPMgrUtilities.h"

#define EDLP_MGR_DLG_TAB_BASE 300

CEDLPMgrDlg::CEDLPMgrDlg(void)
{
	m_pTabCtrl = NULL;
}

CEDLPMgrDlg::~CEDLPMgrDlg(void)
{
	if (m_pTabCtrl)
	{
		delete m_pTabCtrl;
		m_pTabCtrl = NULL;
	}

	if(m_hHelp)
	{
		DestroyIcon(m_hHelp);
		m_hHelp = NULL;
	}
}

LRESULT CEDLPMgrDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD dwStart = GetTickCount();
	CAxDialogImpl<CEDLPMgrDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);
	bHandled = TRUE;

	HICON hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, FALSE);
	}

	hIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCE(IDI_LOGO), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	if(hIcon)
	{
		SetIcon(hIcon, TRUE);
	}

#if 0//Just put the title next to ICON.
	//Set Title As Middle
	{
		wchar_t szTitle[1024] = {0};
		::GetWindowText(m_hWnd, szTitle, 1024);
		RECT mainRC;
		::GetWindowRect(m_hWnd, &mainRC);
		HDC dc = ::GetDC(m_hWnd);
		SIZE len;
		GetTextExtentPoint32W(dc, szTitle, wcslen(szTitle), &len);
		SIZE spaceLen;
		GetTextExtentPoint32W(dc, L" ", 1, &spaceLen);
		int nSpaceCount = ( (mainRC.right - mainRC.left - len.cx) / spaceLen.cx ) / 2;
		if(nSpaceCount < 200)
		{
			wstring strFormatTitle;
			for(int i = 0; i < nSpaceCount; i++)
			{
				strFormatTitle += L" ";
			}

			strFormatTitle += szTitle;

			::SetWindowText(m_hWnd, strFormatTitle.c_str());
		}

		::ReleaseDC(m_hWnd, dc);
	}
#endif

	//	parse and get tab configuration 
	char pszFullFileName[MAX_PATH + 1] = {0};
	GetCurrentDirectory(pszFullFileName, MAX_PATH);
	strncat_s(pszFullFileName, MAX_PATH, "dialog.ini", _TRUNCATE);
	if ( !m_tabcfg.ParseConfig(pszFullFileName) )//Try to find related DLLs for each item
	{
		return FALSE;
	}
	m_tabcfg.GetConfig(m_tabinfo);


	//	get tab control handle from the dialog
	HWND hTab = GetDlgItem(IDC_FEATURES_TAB);


	//	instant a WTL tab ctrl object using the tab ctrl handle
	m_pTabCtrl = new CNL_TabCtrl();

	if (!m_pTabCtrl)
	{
		return FALSE;
	}

	m_pTabCtrl->SubclassWindow(hTab);
	m_pTabCtrl->SetBkColor(g_clrHighlight);

	//	show tab control using WTL tab ctrl
	DWORD dwTabCnt = (DWORD)m_tabinfo.size();

	for(DWORD i = 0; i < dwTabCnt; i++)
	{

		//	getting tab item name in unicode
		WCHAR wszName[256] = {0};
		AnsiToUnicode(m_tabinfo[i].szName, wszName, 256);

		//	insert tab item
		m_pTabCtrl->InsertItem(EDLP_MGR_DLG_TAB_BASE + i, wszName);


		//	maintain tab info plus ID and sub window handle
		TabsInfoPlus tabinfoPlus;

		tabinfoPlus.tabID = EDLP_MGR_DLG_TAB_BASE + i;
		tabinfoPlus.tabinfo = m_tabinfo[i];


		//	call function to create sub dialogs
		RECT rc;
		::GetWindowRect(GetDlgItem(IDC_FEATURES_TAB), &rc);
		ScreenToClient(&rc);
		rc.top += 25;
		rc.bottom -= 2;
		rc.left += 2;
		rc.right -= 2;
		tabinfoPlus.hWnd = m_tabinfo[i].pFun(m_tabinfo[i].dwIndex, m_hWnd, &rc);

		m_tabinfoPlus.push_back(tabinfoPlus);

	}	//	for(DWORD i = 0; i < dwTabCnt; i++)


	//	as we need these dialog and its child dialog can respond to keyboard input, 
	//	we need to register them
	HWND szWnds[1024] = {0};

	//	first, we put the child dialog first
	int nCount = 0;
	for (DWORD i = 0; i < m_tabinfoPlus.size(); i++)
	{
		if (m_tabinfoPlus[i].hWnd)
		{
			szWnds[i] = m_tabinfoPlus[i].hWnd;
			nCount++;
		}
	}

	//	then, we put the parent dialog
	szWnds[nCount] = m_hWnd;
	nCount++;

	//	finally, we call api to register them
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	edpUtilities.RegDlgHandleForKB(szWnds, nCount);

	g_log.Log(CELOG_DEBUG, L"RegDlgHandleForKB in edpdlg %d dialogs\n",nCount);

	//	show first tab sub window
	::ShowWindow(m_tabinfoPlus[0].hWnd, SW_SHOW);

	m_hHelp = LoadIconW(NULL, IDI_QUESTION);

#if 0//don't need this button in Sirius
	//Create "help" button
	RECT tabRC;
	HWND hTabWnd = GetDlgItem(IDC_FEATURES_TAB);
	::GetWindowRect(hTabWnd, &tabRC);
	ScreenToClient(&tabRC);

	m_rcHelp.left = tabRC.right - tabRC.left - 20;
	m_rcHelp.top = 4;
	m_rcHelp.right = m_rcHelp.left + 16;
	m_rcHelp.bottom = m_rcHelp.top + 16;


	HWND  hHelp = CreateWindowW(L"STATIC", L"H", WS_CHILD | WS_VISIBLE | SS_ICON, m_rcHelp.left, m_rcHelp.top, 16, 16, hTabWnd, 0, 0, 0);

	HICON hIcon = LoadIconW(g_hInst, MAKEINTRESOURCEW(IDI_EDPMHELP));
	::SendMessage(hHelp, STM_SETICON, (WPARAM)hIcon, 0L);

	DestroyIcon(hIcon);

	::SetWindowPos(hHelp, 0, 0, 0, 16, 16, SWP_NOMOVE | SWP_SHOWWINDOW);
#endif

	//bring this dialog to topmost
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	g_log.Log(CELOG_DEBUG, L"Initialize mail dialog elapsed: %d ms\n", GetTickCount() - dwStart);

	return TRUE;
}


LRESULT CEDLPMgrDlg::OnTcnSelchangeTab1(int /*idCtrl*/, LPNMHDR /*pNMHDR*/, BOOL& /*bHandled*/)
{
	//	show sub window according to tab sel


	if (!m_pTabCtrl)
	{
		return FALSE;
	}

	int curSel = m_pTabCtrl->GetCurSel();

	if (curSel < 0)
	{
		return FALSE;
	}

	DWORD dwTabCnt = (DWORD)m_tabinfoPlus.size();

	//	check which tab dialog should be show, and which should be hide
	for ( DWORD i = 0; i < dwTabCnt; i++ )
	{
		if ( m_tabinfoPlus[i].tabID - EDLP_MGR_DLG_TAB_BASE == (DWORD)curSel )
		{
			::ShowWindow(m_tabinfoPlus[i].hWnd, SW_SHOW);
			::SetFocus(m_tabinfoPlus[i].hWnd);
			::InvalidateRect(m_tabinfoPlus[i].hWnd, NULL, TRUE);
		}
		else
		{
			::ShowWindow(m_tabinfoPlus[i].hWnd, SW_HIDE);
		}
	}

	return TRUE;
}

void CEDLPMgrDlg::ShowNotification(int nID)
{
	if(m_pTabCtrl->GetCurSel() == 2)
	{
		::SendMessageW(m_tabinfoPlus[2].hWnd, WM_SHOWNOTIFICATION, NULL, (LPARAM)nID);
	}
}

LRESULT CEDLPMgrDlg::OnLButtonUp(UINT , WPARAM , LPARAM /*lParam*/, BOOL& )
{
/*	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	if(PtInRect(&m_rcHelp, pt))
	{
		CHelpDlg dlg;
		dlg.DoModal();
	}*/
	return 0;
}

LRESULT CEDLPMgrDlg::OnMouseMove(UINT , WPARAM , LPARAM /*lParam*/, BOOL& )
{
/*	POINT pt;
	pt.x=LOWORD(lParam);
	pt.y=HIWORD(lParam);

	if(PtInRect(&m_rcHelp, pt))
	{
		SetCursor(LoadCursorW(NULL, IDC_HAND));
	}
	else
	{
		SetCursor(LoadCursorW(NULL, IDC_ARROW) );
	}*/
	return 0;
}

LRESULT CEDLPMgrDlg::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	PAINTSTRUCT ps; 
	HDC hdc; 
	hdc = ::BeginPaint(m_hWnd, &ps); 


	::EndPaint(m_hWnd, &ps); 

	return 0;
}

void CEDLPMgrDlg::ShowTab(int nIndex)
{
	m_pTabCtrl->SetCurSel(nIndex);
	BOOL b;
	OnTcnSelchangeTab1(IDC_FEATURES_TAB, 0, b);
}
