#include "stdafx.h"
#include "SystrayMgr.h"


CSystrayMgr& CSystrayMgr::GetInstance()
{
	static CSystrayMgr mgr;
	return mgr;
}

CSystrayMgr::CSystrayMgr()
{
//	m_pEnforcerStatusWindow = new CEnforcerStatusWindow;
}

CSystrayMgr::~CSystrayMgr()
{
// 	if(m_pEnforcerStatusWindow)
// 	{
// 		delete m_pEnforcerStatusWindow;
// 		m_pEnforcerStatusWindow = NULL;
// 	}
}

BOOL CSystrayMgr::AddSystray(const DWORD dwIconID, const HWND hIconWnd, const UINT dwMsgID, HICON hIcon, WCHAR* pszTip)
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA); 
	m_nid.hWnd = hIconWnd; 
	m_nid.uID = dwIconID;
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	m_nid.uCallbackMessage = dwMsgID;
	m_nid.hIcon = hIcon;

//	wcsncpy_s(m_nid.szInfo, 256, L"123", 3);

	if (pszTip)
	{
		//	user want to show product name when mouse over the tray icon
		wcsncpy_s (m_nid.szTip, 128, pszTip, _TRUNCATE); 
	}
	else
	{
		//	user don't want shell tray icon to process event when mouse over the tray icon
		//	ie, maybe user want to handle the mouse over event itself,
		//	so, do nothing here.
	}
	
	
	BOOL bRet = Shell_NotifyIconW(NIM_ADD, &m_nid);
	
	CEDPMgr& edpMgr = CEDPMgr::GetInstance();
	edpMgr.GetCELog().Log(CELOG_DEBUG,L"Shell_NotifyIconW, ret: %d, last error: %d, hex below-- hWnd: %x, uID: %x, hIcon: %x\r\n", bRet, GetLastError(), hIconWnd, dwIconID, hIcon);


	return TRUE;
}

/*****************************************************
This function was dropped.
We will create a window to show enforcer status
for "mouse move".
Used ShowEnforcerStatus instead.

******************************************************/
BOOL CSystrayMgr::ModifySystray(LPCWSTR pszEnforcerStatus)
{
	if(!pszEnforcerStatus)
	{
		return FALSE;
	}

	m_nid.uFlags &= ~NIF_INFO;
	int nSize = sizeof(m_nid.szTip) / sizeof(wchar_t);
	wcsncpy_s (m_nid.szTip, nSize, pszEnforcerStatus, _TRUNCATE); 

	return Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}
/*
void CSystrayMgr::ShowEnforcerStatus(int x, int y)
{
	if(!m_pEnforcerStatusWindow)
	{
		return;
	}

	if(!m_pEnforcerStatusWindow->m_hWnd)
	{
		m_pEnforcerStatusWindow->Create(NULL);
	}

	vector<ENFORCERSTATUS> vEnforcers;
	ENFORCERSTATUS status;
	status.strEnforcerName = L"Policy Controller";
	status.strEnforcerStatus = L"Running";
	vEnforcers.push_back(status);

	m_pEnforcerStatusWindow->SetEnforcerStatusInfo(vEnforcers);

	m_pEnforcerStatusWindow->ShowEnforcerStatus(x, y);
}*/


BOOL CSystrayMgr::ShowInfo()
{
	LoadStringW(g_hInstance, IDS_PRODUCTNAME, m_nid.szInfo, 200);
	LoadStringW(g_hInstance, IDS_NXTLBS, m_nid.szInfoTitle, sizeof(m_nid.szInfoTitle) / sizeof(wchar_t));

	m_nid.uFlags |= NIF_INFO;
	

	return Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}
