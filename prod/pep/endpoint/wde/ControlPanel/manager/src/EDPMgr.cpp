#include "StdAfx.h"
#include "EDPMgr.h"

#include "actionMenu.h"
#include "StubMgr.h"
#include "SystrayMgr.h"
#include "NotifyMgr.h"
#include "SysUtils.h"

/*

Right and Left mouse click on EDP Manager Tray Icon.
this is changed in EDP Manager Sirius, Oct, 5, 2010.

g_ActionMenu_L --	action menu which will pop up when left click tray icon,
g_ActionMenu_R -- Action menu which will pop up when right click tray icon

*/
#define NL_EDPM_ACTION_MENU_L "menu_l.ini"
#define NL_EDPM_ACTION_MENU_R "menu_r.ini"

CActionMenu g_ActionMenu_L;   // left menu
CActionMenu g_ActionMenu_R; // right menu
CActionMenu* g_pActionMenu_CurActived = NULL;    // Specify the current active menu, right or left


HWND g_hNotifyMsgLoop;

typedef void (__stdcall * QueryStatusType)(char* pBuf, int nLen);


LRESULT CALLBACK WndProc_Notify(HWND hWnd,UINT wMsg,WPARAM wParam,LPARAM lParam) 
{ 
	switch (wMsg)
	{
	case NOTIFY_RECEIVE_MSG:
		{
			CEDPNotifyMgr& notifyMgr = CEDPNotifyMgr::GetInstance();
			notifyMgr.ShowNotify((PVOID)lParam, (int)wParam);
		}
		break;
	}
	return DefWindowProc(hWnd,wMsg,wParam,lParam); 
}

void CreateWnd(void) 
{    

	WNDCLASS wc = {0}; 
	wc.style         = 0; 
	wc.lpfnWndProc   = WndProc_Notify; 
	wc.cbClsExtra    = 0; 
	wc.cbWndExtra    = 0; 
	wc.hInstance     = g_hInstance; 
	wc.hIcon         = NULL; 
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW); 
	wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW); 
	wc.lpszMenuName = NULL; 
	wc.lpszClassName = TEXT("NL_NOTIFY_MSG_LOOP");

	RegisterClass(&wc);

	g_hNotifyMsgLoop = CreateWindowExW(0, 
										wc.lpszClassName, 
										wc.lpszClassName,
										WS_OVERLAPPEDWINDOW, 
										0, 
										0, 
										200, 
										200, 
										NULL, 
										NULL, 
										g_hInstance, 
										0); 

//	ShowWindow(g_hNotifyMsgLoop, SW_HIDE);
}


void PopupNotifyThread(void* /*pParam*/)
{

	CreateWnd();

	//The message loop     
	MSG msg; 
	while(GetMessage(&msg,NULL,0,0)) 
	{ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
}

CEDPMgr::CEDPMgr(void)
{
	//	flag for init celog.
	m_bLogInit = FALSE;


	//	initialize product information window.
	m_pInfoDlg = new CProductInfoDlg;
	if(m_pInfoDlg)
	{
		m_pInfoDlg->Create(NULL);
	}
}

CEDPMgr::~CEDPMgr(void)
{
	if(m_pInfoDlg)
	{
		if(m_pInfoDlg->m_hWnd)
		{
			DestroyWindow(m_pInfoDlg->m_hWnd);
		}
		delete m_pInfoDlg;
		m_pInfoDlg = NULL;
	}
}

CEDPMgr& CEDPMgr::GetInstance()
{
	static CEDPMgr mgr;
	return mgr;
}

BOOL CEDPMgr::Init()
{
	//	load icon handle from edlp manager module
	HICON hTrayIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDI_NL_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	//	add system tray icon
	CSystrayMgr& SystrayMgr = CSystrayMgr::GetInstance();
	SystrayMgr.AddSystray(EDLP_MGR_TRAY_ICON_ID, g_hMainWnd, DESTINY_SYSTEM_TRAY_MSG, hTrayIcon, NULL);


	//	initialize action menu
    if (!g_ActionMenu_L.Create(NL_EDPM_ACTION_MENU_L))
	{
		//	create menu failed.
		return FALSE;
	}

	if ( !g_ActionMenu_R.Create(NL_EDPM_ACTION_MENU_R) )
	{
		//	create menu failed.
		return FALSE;
	}

	//Create a thread to receive message for "notfiy_receive"
	_beginthread(PopupNotifyThread, 0, 0);

	CStubMgr& stubMgr = CStubMgr::GetInstance();
	stubMgr.StartStub();

	return TRUE;
}

void CEDPMgr::Quit()
{
	//Send Message to ask "msg loop thread" quit
	SendMessage(g_hNotifyMsgLoop, WM_QUIT, 0, 0);

	//Stop stub
	CStubMgr& stubMgr = CStubMgr::GetInstance();
	stubMgr.StopStub();

	//Destroy main window
	DestroyWindow(g_hMainWnd);
}

void CEDPMgr::ShowProductInfo(int x, int y)
{
	if(m_pInfoDlg && m_pInfoDlg->m_hWnd)
	{
		RECT rc;
		GetWindowRect( m_pInfoDlg->m_hWnd, &rc);

		int nWidth = GetSystemMetrics(SM_CXFULLSCREEN);

		if(x + (rc.right - rc.left) >= nWidth)
		{
			x = nWidth - (rc.right - rc.left);
		}
		y = y - (rc.bottom - rc.top);
		::SetWindowPos(m_pInfoDlg->m_hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
	//	m_pInfoDlg->ShowWindow(SW_SHOW);
	}
}

void CEDPMgr::ShowEnforcerStatus()
{
	char szPath[MAX_PATH + 1] = {0};
	if( !MyGetCurrentDirectory(szPath, MAX_PATH))
	{
		return ;
	}


	strncat_s(szPath, MAX_PATH, "status_plugin.ini", _TRUNCATE);

	CIniFile* pIni = new CIniFile(szPath);

	if (!pIni)
	{
		return ;
	}

	//read notify type
	char szCount[20] = {0};
	pIni->ReadString("Section Count", "Count", "0", szCount, 20);

	int nCount = atoi(szCount);

	string strToShow;
	for( int i = 0; i < nCount; i++)
	{
		char szPlugin[20] = {0};
		_snprintf_s(szPlugin, 20, _TRUNCATE, "plugin%d", i + 1);

		char szName[100] = {0};
		pIni->ReadString(szPlugin, "Name", "", szName, 100);

		char szDLL[MAX_PATH] = {0};
		pIni->ReadString(szPlugin, "DLLPath", "", szDLL, MAX_PATH);

		if(strlen(szName) > 0 && strlen(szDLL) > 0)
		{
			HMODULE hMod = LoadLibraryA(szDLL);
			if(hMod)
			{
				QueryStatusType lfQueryStatus = (QueryStatusType)GetProcAddress(hMod, "QueryStatus");
				if(lfQueryStatus)
				{
					char szStatus[1024] = {0};
					lfQueryStatus(szStatus, _countof(szStatus));

					if(strlen(szStatus) > 0)
					{
						if(strToShow.length() > 0)
						{
							strToShow += "\n";
						}

						strToShow += string(szName) + string("\n    ") + string(szStatus) + string("\n");
					}
				}
				FreeLibrary(hMod);
			}
		}

	}

	if(pIni)
	{
		delete pIni;
		pIni = NULL;
	}
	wchar_t szShow[1024] = {0};
	AnsiToUnicode(strToShow.c_str(), szShow, 1024);

	CSystrayMgr& SystrayMgr = CSystrayMgr::GetInstance();
	
	SystrayMgr.ModifySystray(szShow);

}
