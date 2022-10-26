// edlpmanager.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "edlpmanager.h"

#include "SystrayMgr.h"
#include "EDPMgr.h"
#include "resource.h"
#include "EDPMgrUtilities.h"

#define WM_SHOWNOTIFICATIONTAB	WM_USER + 5

#ifdef _X86_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"") 
#endif

#define MAX_LOADSTRING 100
UINT WM_TASKBARCREATED = 0;

// Global Variables:
HINSTANCE g_hInstance = NULL;
HWND g_hMainWnd = NULL;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HANDLE  g_hMutextSingleInstance=NULL;  //make edpmanager a single instance in the session.

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void EnableOEPlugin();
BOOL                IsAlreadyAnInstanceInTheSession();
void DisableOutlook2016RecentAttachList();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	//DisableOutlook2016RecentAttachList();

	if(IsAlreadyAnInstanceInTheSession())
	{
		::OutputDebugStringW(L"An edpmanager already exist in this session.Exit.\n");
		return 1;
	}

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_EDLPMANAGER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		CloseHandle(g_hMutextSingleInstance);
		g_hMutextSingleInstance=NULL;
		return FALSE;
	}

	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDLPMANAGER));


	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
	//edpUtilities.SetNotifyDisplayLever(CEDPMUtilities::E_NONE);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//	try to get all dialog handles which need special process
		DWORD dwCount = 500;
		HWND hwndArray[500] = {NULL};
		if(edpUtilities.GetRegDlgHandleForKB(hwndArray, dwCount))
		{
			//	we have succeed get all dialog handles which need special process
			//	now, process the keyboard input for modaless dialogs
			BOOL bProcessed = FALSE;
			for (DWORD i = 0; i < dwCount; i++)
			{
				//	check if this hwnd is a valid hwnd and check if the msg is for it
				if((hwndArray[i] && IsDialogMessage(hwndArray[i], &msg)))
				{
					//	yes, the hwnd is valid and the msg is for it
					//	and, we have already process the msg in IsDialogMessage, 
					//	so we continue the message loop directly
					bProcessed = TRUE;
					break;
				}
			}	
			//	check if we have processed this msg
			if (bProcessed)
			{
				//	yes, have processed
				//	so we continue the message loop directly
				continue;
			}
		}
		else
		{
			//	return false means array size is not enough, 
			CEDPMgr& edpMgr = CEDPMgr::GetInstance();
			edpMgr.GetCELog().Log(CELOG_DEBUG, L"get modaless dialogs handles in main message loop, but memory is not enough\n");

			//	ok, we don't process the keyboard input for modaless dialogs
			//	do nothing.......
		}

		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CloseHandle(g_hMutextSingleInstance);
	g_hMutextSingleInstance=NULL;

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDLPMANAGER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_EDLPMANAGER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int /*nCmdShow*/)
{
   HWND hWnd;

   g_hInstance = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hMainWnd = hWnd;

   //	edlp manager hide main window
   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   CEDPMgr& mgr = CEDPMgr::GetInstance();
   if( !mgr.Init() )
   {
	   return FALSE;
   }
   
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;


	if ( DESTINY_SYSTEM_TRAY_MSG == message )
	{
		//	event to system tray icon

		switch (wParam) 
		{ 
			
			//	to exactly this tray icon

			case EDLP_MGR_TRAY_ICON_ID:

				switch(lParam)
				{
				case WM_LBUTTONDOWN:
					{
						break;
					}
				case WM_RBUTTONUP:

					//	right click tray icon
					
					{
                        //	get current pos
                        POINT CurPos;
                        GetCursorPos(&CurPos);

                        //	Bodge to get around a documented bug in Explorer // Keep comment... may be needed later for other windows versions
                        SetForegroundWindow(hWnd);

                        //	pop up action menu.
                        g_ActionMenu_R.Popup(CurPos.x, CurPos.y, hWnd);
                        g_pActionMenu_CurActived = &g_ActionMenu_R;
                        break;
					}


				case WM_MOUSEMOVE:
					//	mouse move to tray icon
					{
						//	get current pos
						static POINT lastPos = {0, 0};
						POINT CurPos ;
						GetCursorPos (&CurPos);

						if(lastPos.x != CurPos.x && lastPos.y != CurPos.y)
						{
						CEDPMgr& mgr = CEDPMgr::GetInstance();
						mgr.ShowEnforcerStatus();
						
							lastPos.x = CurPos.x;
							lastPos.y = CurPos.y;
						}
						
						break;
					}
				case WM_LBUTTONUP:
					{
						//	get current pos
						POINT CurPos ;
						GetCursorPos (&CurPos);
	 
 						//	Bodge to get around a documented bug in Explorer // Keep comment... may be needed later for other windows versions
 						SetForegroundWindow (hWnd);
 
 						//	pop up action menu.
                        g_ActionMenu_L.Popup(CurPos.x, CurPos.y, hWnd);
                        g_pActionMenu_CurActived = &g_ActionMenu_L;

						break;

					}
				case WM_LBUTTONDBLCLK:
					{
						HMODULE hMainDlg = GetModuleHandleW(L"edpmdlg.dll");
						if(hMainDlg)
						{
							typedef BOOL (*DoActionType) (DWORD dwIndex);
							DoActionType lfDoAction = (DoActionType)GetProcAddress(hMainDlg, "DoAction");
							if(lfDoAction)
							{
								lfDoAction(0);
								
							}
						}
					}
					break;
				case WM_MOUSELEAVE: 
				case WM_MOUSEHOVER:
				case WM_MOUSELAST:
					break;
				default:
					break;
				}
				break;

			default:
				break;
		}
	}

	switch (message)
	{
	case WM_CREATE:
		EnableOEPlugin();
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
        default:
            {
                //	let action menu handle this event,
                //	action menu can determine if this event should be handled by him.
                if (NULL != g_pActionMenu_CurActived)
                {
                    g_pActionMenu_CurActived->HandleClick(wmId);
                }
                else
                {
                    // Current active menu is null
                    CEDPMgr::GetInstance().GetCELog().Log(CELOG_DEBUG, L"g_pActionMenu_CurActived not our right menu events or unknown exception \n");
                }
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SHOWNOTIFICATIONTAB://This message will be triggered when user clicks on bubble.
		{
#ifdef _WIN64
			HMODULE hMainDlg = GetModuleHandleW(L"edpmdlg.dll");
#else 
			HMODULE hMainDlg = GetModuleHandleW(L"edpmdlg32.dll");
#endif
		 	if(hMainDlg)
		 	{
		 		typedef BOOL (*DoActionType) (DWORD dwIndex);
		 		DoActionType lfDoAction = (DoActionType)GetProcAddress(hMainDlg, "DoAction");
		 		if(lfDoAction)
		 		{
		 			lfDoAction(MAKELONG((WORD)wParam, 1000));
				}
		 	}
		}
		break;
	default:
		if(message == WM_TASKBARCREATED)
		{//Need to add the system tray again after explorer restarts.
			HICON hTrayIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDI_NL_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			CSystrayMgr& SystrayMgr = CSystrayMgr::GetInstance();
			SystrayMgr.AddSystray(EDLP_MGR_TRAY_ICON_ID, g_hMainWnd, DESTINY_SYSTEM_TRAY_MSG, hTrayIcon, NULL);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void EnableOEPlugin()
{
	//delete the register key:
	const wchar_t* szDisableKey[]  = {
		L"Software\\Microsoft\\Office\\14.0\\Outlook\\Resiliency\\DisabledItems", 
		L"Software\\Microsoft\\Office\\15.0\\Outlook\\Resiliency\\DisabledItems",
		L"Software\\Microsoft\\Office\\16.0\\Outlook\\Resiliency\\DisabledItems"}; 

		for(int iRegKey=0; iRegKey<sizeof(szDisableKey)/sizeof(szDisableKey[0]); iRegKey++)
		{
			HKEY hKeyDisable=NULL;
			LONG lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, szDisableKey[iRegKey], 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE | KEY_WRITE, &hKeyDisable);
			if((lStatus==ERROR_SUCCESS) && (hKeyDisable!=NULL))
			{
                int nValueIndex = 0;
				while(true)
				{
					DWORD nValueNameLen = 100;
					wchar_t szValueName[100]={0};
					DWORD nValueLen = 1024*2;
					wchar_t szValue[1024]={0};
					lStatus = RegEnumValue(hKeyDisable, nValueIndex, szValueName, &nValueNameLen, 0, NULL, (LPBYTE)szValue, &nValueLen);
					if(lStatus==ERROR_SUCCESS)
					{
						wchar_t* pPluginPath = szValue + 6;
						_wcslwr_s(pPluginPath, wcslen(pPluginPath)+ 1 /*must include NULL*/);
                         wchar_t* szOE = wcsstr(pPluginPath, L"outlook enforcer");
						 if(szOE)
						 {
							 RegDeleteValue(hKeyDisable, szValueName);
							 break;
						 }
					}
					else
					{
						break;
					}
					nValueIndex++;
				}
				RegCloseKey(hKeyDisable);
				//break;
			}
			else
			{
				continue;
			}
		}

		
		//delete "msoPEP.msoObj.1" on current_user
		const wchar_t* szKeyAddins = L"Software\\Microsoft\\Office\\Outlook\\Addins";
		HKEY hKeyAddins = NULL;
		LONG lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, szKeyAddins, 0, KEY_WRITE, &hKeyAddins);
		if((lStatus==ERROR_SUCCESS) && (hKeyAddins!=NULL))
		{
			RegDeleteKey(hKeyAddins, L"msoPEP.msoObj.1");
			RegCloseKey(hKeyAddins);
		}

}

BOOL IsAlreadyAnInstanceInTheSession()
{
	g_hMutextSingleInstance = CreateMutexW(NULL, FALSE, L"Local\\{E45ADECC-ABEC-4009-8A3F-2CB2F04DBE70}");
	DWORD dwLastError = GetLastError();
	if (g_hMutextSingleInstance==NULL)
	{
		return FALSE;
	}
	else
	{
		if(dwLastError==ERROR_ALREADY_EXISTS)
		{
			CloseHandle(g_hMutextSingleInstance);
			g_hMutextSingleInstance=NULL;
		}

		return (dwLastError==ERROR_ALREADY_EXISTS);
	}
}

void DisableOutlook2016RecentAttachList()
{
	const wchar_t* lpszKeyOutlook = L"Software\\Microsoft\\Office\\16.0\\Outlook";

	//open or create "outlook"
   HKEY hkeyOutlook = NULL;
   LONG lRes = RegCreateKeyExW(HKEY_CURRENT_USER, lpszKeyOutlook, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkeyOutlook, NULL);
   if (hkeyOutlook==NULL)
   {
	   return;
   }
   
   //open or create "Options"
   HKEY hkeyOptions = NULL;
   lRes = RegCreateKeyExW(hkeyOutlook, L"Options", 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkeyOptions, NULL);
   if (hkeyOptions==NULL)
   {
	   RegCloseKey(hkeyOutlook);
	   return;
   }

   //open or create L"Mail"
   HKEY hKeyMail = NULL;
   lRes = RegCreateKeyExW(hkeyOptions, L"Mail", 0, NULL, 0, KEY_SET_VALUE, NULL, &hKeyMail, NULL);
   if (hKeyMail==NULL)
   {
	   RegCloseKey(hkeyOutlook);
	   RegCloseKey(hkeyOptions);
	   return;
   }

   //set value
   if (hKeyMail)
   {
	   //set value
	   const DWORD dwValue = 0;
	   lRes = RegSetValueExW(hKeyMail, L"MaxAttachmentMenuItems", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue) );

	   RegCloseKey(hkeyOutlook);
	   RegCloseKey(hkeyOptions);
	   RegCloseKey(hKeyMail);
   }

}
