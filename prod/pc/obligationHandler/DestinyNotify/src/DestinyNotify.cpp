// DestinyNotify.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <shellapi.h>
#include <commctrl.h>
#define SECURITY_WIN32
#include <security.h>
#include "DestinyNotify.h"
#include "iipcrequesthandler.h"
#include "globals.h"
#include "IPCStub.h" 
#include "IPCProxy.h" 
#include "NotificationInfo.h"
#include ".\destinynotifyrequesthandler.h"

#define MAX_LOADSTRING 255
#define MAX_FILENAME_LENGTH 32767+1  /* MAX Unicode path length */

// Constants
#define RESOURCE_LIB _T("ComplianceAgentNotifyResource.dll")
const TCHAR *  IPC_CM_REQUEST_HANDLER   = _T("com.bluejungle.destiny.agent.controlmanager.CMRequestHandler");
const TCHAR *  GET_AGENT_INFO = _T("getAgentInfo");

// Global Variables:
HINSTANCE hInst = NULL;								// current instance
HMODULE g_hResourceModule = NULL; //handle to resource dll.
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR* onlineHelpDir = NULL;

HWND g_hWnd = NULL;
HWND g_hNotificationDlg = NULL;
HMENU g_hMenu = NULL;
HANDLE g_hStubThread = NULL;
IPCStub* g_pStub = NULL;
DestinyNotifyRequestHandler* g_pRequestHandler = NULL;
NotificationVector g_notificationInfoArray;
NOTIFYICONDATA g_nid; 

const UINT DESTINY_SYSTEM_TRAY_MSG = ::RegisterWindowMessage (_T("DESTINY_SYSTEM_TRAY_MSG"));

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance,HINSTANCE hResInstance);
BOOL				InitInstance(HINSTANCE, HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK        DlgHandler (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void ShowNotification (LPCTSTR cszTitle, LPCTSTR cszText);
void ShowNotificationDialog ();
void StartStub ();

/* Determine if notifications should be enabled.
 * Return true if notifications are enabled and false if they are not.
 *
 * Any error condition will lead to notifications being enabled.
 */
static bool IsNotifyUIEnabled(void)
{
  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,KEY_QUERY_VALUE,&hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    return false;
  }

  unsigned char enable = 0;
  DWORD enable_size = sizeof(enable);

  rstatus = RegQueryValueExA(hKey,                  /* handle to reg */      
			     "EnableNotifications", /* key to read */
			     NULL,                  /* reserved */
			     NULL,                  /* type */
			     (LPBYTE)&enable,       /* InstallDir */
			     &enable_size);         /* size (in/out) */

  RegCloseKey(hKey);

  if( rstatus != ERROR_SUCCESS )
  {
    return false;
  }

  /* Are notifications disabled? */
  if( enable != 0 )
  {
    return true;
  }

  return false;

}/* IsNotifyUIEnabled */

/* Load the online help URL
 *
 * Errors loading the URL will lead to the help URL being null.
 * See menu item behavior later in WndProc() function to determine
 * how a null URL is handled
 */
int LoadOnlineHelpURL()
{
  LONG result;
  HKEY hKey = NULL; 
  DWORD policyControllerDirSize = MAX_FILENAME_LENGTH;
  TCHAR* policyControllerDir = new TCHAR[MAX_FILENAME_LENGTH];

  result = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,KEY_QUERY_VALUE,&hKey);

  if( result != ERROR_SUCCESS )
  {
    return result;
  }

  result = RegQueryValueEx(hKey,                   
					       L"PolicyControllerDir", 
					       NULL,                  
						   NULL,                  
						   (LPBYTE)policyControllerDir,       
			               &policyControllerDirSize);         

  RegCloseKey(hKey);

  if( result == ERROR_SUCCESS )
  {
	  result = _tcsncat_s(policyControllerDir, MAX_FILENAME_LENGTH, L"\\help\\index.html", _TRUNCATE);
	  if( result == ERROR_SUCCESS )
	  {		    
		  size_t onlineHelpDirLen = _tcslen(policyControllerDir) + 1;
		  onlineHelpDir = new TCHAR[onlineHelpDirLen];
		  _tcsncpy_s(onlineHelpDir, onlineHelpDirLen, policyControllerDir, _TRUNCATE); 
	  }
  }
  
  if (policyControllerDir != NULL)
  {
	delete[] policyControllerDir;
	policyControllerDir = NULL;
  }

  return result;  

}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    g_hResourceModule = ::LoadLibrary (RESOURCE_LIB);

    // Initialize global strings
    LoadString(g_hResourceModule, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_DESTINYNOTIFY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance,g_hResourceModule);

    // Perform application initialization:
    if (!InitInstance (hInstance, g_hResourceModule, SW_HIDE)) 
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_DESTINYNOTIFY);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

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
ATOM MyRegisterClass(HINSTANCE hInstance,HINSTANCE hResInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX); 

    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= (WNDPROC)WndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hResInstance, (LPCTSTR)IDI_DESTINYNOTIFY);
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName	= (LPCTSTR)IDC_DESTINYNOTIFY;
    wcex.lpszClassName	= szWindowClass;
    wcex.hIconSm		= LoadIcon(hResInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, HINSTANCE hResInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    g_hWnd = CreateWindow(szWindowClass, _T(""), WS_OVERLAPPEDWINDOW,
        -1, -1, 100, 100, NULL, NULL, hInstance, NULL);

    if (!g_hWnd)
    {
        return FALSE;
    }

    ShowWindow(g_hWnd, nCmdShow);
    //UpdateWindow(g_hWnd);

    //Add icon to tray
    g_nid.cbSize = sizeof(NOTIFYICONDATA); 
    g_nid.hWnd = g_hWnd; 
    g_nid.uID = TRAY_ICON_01; // ID
    g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    g_nid.uCallbackMessage = DESTINY_SYSTEM_TRAY_MSG;
    g_nid.hIcon = LoadIcon(hResInstance, (LPCTSTR)IDI_SMALL);
    _tcscpy (g_nid.szTip, szTitle); 

    if( IsNotifyUIEnabled() == true )
    {
      Shell_NotifyIcon(NIM_ADD, &g_nid);
    }

	LoadOnlineHelpURL();

    //Create the menu
    g_hMenu = ::LoadMenu(g_hResourceModule, MAKEINTRESOURCE (IDR_MENU1));

    //// Need to create a submenu for use with TrackPopupMenu
    //HMENU hSubMenu = ::CreateMenu ();
    //::InsertMenu (g_hMenu, 0, MF_BYPOSITION | MF_POPUP, (UINT_PTR) hSubMenu, NULL);

    //// Strings should come from resource dll
    //::AppendMenu (hSubMenu, MF_STRING, ID_CONFIG, _T("&Config"));
    //::AppendMenu (hSubMenu, MF_STRING, ID_ABOUT, _T("&About"));
    //::AppendMenu (hSubMenu, MF_STRING, ID_EXIT, _T("E&xit"));

    StartStub ();

    /* Suspport disablement of notification */
    g_pRequestHandler->SetDisplayActivity( IsNotifyUIEnabled() );
    g_pRequestHandler->SetDisplayUI( IsNotifyUIEnabled() );

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
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
    BOOL bChecked = FALSE;

    if (message == DESTINY_SYSTEM_TRAY_MSG)
    {
        switch (wParam) 
        { 
        case TRAY_ICON_01:
            switch (lParam)
            {
            case WM_RBUTTONDOWN:
	        //OutputDebugStringA("WM_RBUTTONDOWN\n");
                POINT CurPos ;

                GetCursorPos (&CurPos);
                SetForegroundWindow (hWnd);// Bodge to get around a documented bug in Explorer // Keep comment... may be needed later for other windows versions

                TrackPopupMenu (::GetSubMenu(g_hMenu, 0),
                    TPM_LEFTBUTTON,
                    CurPos.x,
                    CurPos.y,
                    0,
                    hWnd,
                    NULL);

                //PostMessage (hWnd, WM_NULL, 0, 0); // Bodge to get around a documented bug in Explorer // Keep comment... may be needed later for other windows versions
                break ;
            case NIN_BALLOONUSERCLICK:
	        //OutputDebugStringA("MIN_BALLOONUSERCLICK\n");
                ShowNotificationDialog ();
                break;
            }
            break ;
        }
        return (0);
    }

    switch (message) 
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
        // Parse the menu selections:
        switch (wmId)
        {
        case ID_DISPLAY_ACTIVITY:
            // Toggle menu checked state
            bChecked = ::GetMenuState(g_hMenu, ID_DISPLAY_ACTIVITY, MF_BYCOMMAND) & MF_CHECKED;
            if (bChecked)
            {
                ::CheckMenuItem(g_hMenu, ID_DISPLAY_ACTIVITY, MF_UNCHECKED);
            }
            else
            {
                ::CheckMenuItem(g_hMenu, ID_DISPLAY_ACTIVITY, MF_CHECKED);
            }
            g_pRequestHandler->SetDisplayActivity (!bChecked);
            break;
        case ID_DISPLAY_ON_ALLOWED:
            // Toggle menu checked state
            bChecked = ::GetMenuState(g_hMenu, ID_DISPLAY_ON_ALLOWED, MF_BYCOMMAND) & MF_CHECKED;
            if (bChecked)
            {
                ::CheckMenuItem(g_hMenu, ID_DISPLAY_ON_ALLOWED, MF_UNCHECKED);
            }
            else
            {
                ::CheckMenuItem(g_hMenu, ID_DISPLAY_ON_ALLOWED, MF_CHECKED);
            }
            g_pRequestHandler->SetDisplayAllowActivity (!bChecked);
            break;
        case ID_VIEWNOTIFICATIONS:
            ShowNotificationDialog ();
            break;
        case IDM_ABOUT:
        case ID_AGENTINFO:
            DialogBox(g_hResourceModule, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            break;
        case ID_NOTIFY_HELP:
            {
				if (onlineHelpDir != NULL) 
				{
					ShellExecute(NULL, L"open", onlineHelpDir, NULL, NULL, SW_SHOWNORMAL);
				}
				else
				{
					// Use the old way of showing help
					WCHAR title [100];
					WCHAR helpText [1024];
					::LoadString(g_hResourceModule, IDS_HELP_TITLE, title, 100);
					::LoadString(g_hResourceModule, IDS_HELP_TEXT, helpText, 1024);
					MessageBox (NULL, helpText, title, MB_OK | MB_ICONINFORMATION);
				}
            }
            break;
        case IDM_EXIT:
        case ID_EXIT:
            //_tprintf (_T("Stopping Stub...\n"));
            g_pStub->Stop();
            ::WaitForSingleObject (g_hStubThread, 2000);
            //_tprintf (_T("Stub terminated successfully.\n"));

            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code here...
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        ::DestroyMenu (g_hMenu);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
/**
* Show notifications dialog.
*/
void ShowNotificationDialog ()
{
    DialogBox(g_hResourceModule, MAKEINTRESOURCE(IDD_NOTIFICATION_DIALOG), g_hWnd, (DLGPROC)DlgHandler);
}

void ShowNotification (LPCTSTR cszTitle, LPCTSTR cszText)
{
    NOTIFYICONDATA nid; 

    nid.cbSize = sizeof(NOTIFYICONDATA); 
    nid.hWnd = g_hWnd; 
    nid.uID = TRAY_ICON_01; //ID
    nid.uFlags = NIF_INFO; 
    nid.uTimeout = 1000;

    _tcsncpy_s (nid.szInfo, 256, cszText, _TRUNCATE); 
    _tcsncpy_s (nid.szInfoTitle, 64, cszTitle, _TRUNCATE); 

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}


DWORD WINAPI ThreadProc(LPVOID lpStub)
{
   ((IPCStub*) lpStub)->Run();

   ::ExitThread (0);
   //return (0);
}

void StartStub ()
{
    g_pStub = new IPCStub ();

    IIPCRequestHandler* ppHandlerArray [1];

    ppHandlerArray [0] = g_pRequestHandler = new DestinyNotifyRequestHandler (g_hWnd, &g_notificationInfoArray, &g_nid, g_hResourceModule);

    g_pStub->Init (ppHandlerArray, 1);

    g_hStubThread = ::CreateThread (NULL, 0, ThreadProc, (LPVOID) g_pStub, 0, NULL);       

    //TCHAR buf [100];
    //_tscanf (_T("%s"), buf);

    //_tprintf (_T("Stopping Stub...\n"));
    //pStub->Stop();
    //::WaitForSingleObject (hThread, INFINITE);
    //_tprintf (_T("Stub terminated successfully.\n"));
}


/**
* Initialize the Notification dialog by setting up and populating the 
* list
* 
* @param hDlg handle to the dialog being initialized
*/
BOOL InitNotificationDialog (HWND hDlg)
{
    HWND hList = ::GetDlgItem(hDlg, IDC_LIST1);
    if (hList == NULL)
    {
        return (FALSE);
    }

    TCHAR szName [256];
    ULONG size = 256;
    ::GetUserNameEx (NameSamCompatible, szName, &size);
    ::SetDlgItemText (hDlg, IDC_NAME, szName);

    TCHAR szHostName [MAX_COMPUTERNAME_LENGTH + 1];
    size = MAX_COMPUTERNAME_LENGTH + 1;

    GetComputerName (szHostName, &size);
    ::SetDlgItemText (hDlg, IDC_HOST, szHostName);

    ListView_SetExtendedListViewStyle (hList, LVS_EX_FULLROWSELECT);

    //Insert time column
    TCHAR* pszText = new TCHAR [MAX_LOADSTRING];
    LVCOLUMN lvColumn;
    lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;    
    LoadString(g_hResourceModule, IDS_DATE_TIME, pszText, MAX_LOADSTRING);
    lvColumn.pszText = pszText;
    lvColumn.cchTextMax = 0;
    lvColumn.cx = 150;
    ListView_InsertColumn (hList, 0, &lvColumn);

    lvColumn.cx = 100;
    LoadString(g_hResourceModule, IDS_EVENT, pszText, MAX_LOADSTRING);
    ListView_InsertColumn (hList, 1, &lvColumn);

    lvColumn.cx = 100;
    LoadString(g_hResourceModule, IDS_FILE, pszText, MAX_LOADSTRING);
    ListView_InsertColumn (hList, 2, &lvColumn);

    lvColumn.cx = 200;
    LoadString(g_hResourceModule, IDS_MESSAGE, pszText, MAX_LOADSTRING);
    ListView_InsertColumn (hList, 3, &lvColumn);

    LVITEM lvItem;
    memset (&lvItem, 0, sizeof (lvItem));
    lvItem.pszText = pszText;    

    for (unsigned int i = 0 ; i < g_notificationInfoArray.size (); i++)
    {
        NotificationInfo* pInfo = g_notificationInfoArray.at (i);
        _tcsncpy_s (pszText, MAX_LOADSTRING, pInfo->time.c_str(), _TRUNCATE);
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = i;
        ListView_InsertItem (hList, &lvItem);
		_tcsncpy_s (pszText, MAX_LOADSTRING, pInfo->event.c_str(), _TRUNCATE);
		pszText[MAX_LOADSTRING - 1] = L'\0';
        ListView_SetItemText (hList, i, 1, pszText);
		_tcsncpy_s (pszText, MAX_LOADSTRING, pInfo->file.c_str(), _TRUNCATE);
		pszText[MAX_LOADSTRING - 1] = L'\0';
        ListView_SetItemText (hList, i, 2, pszText);
        _tcsncpy_s (pszText, MAX_LOADSTRING, pInfo->message.c_str(), _TRUNCATE);
		pszText[MAX_LOADSTRING - 1] = L'\0';
        ListView_SetItemText (hList, i, 3, pszText);
    }

    delete [] pszText;

    g_hNotificationDlg = hDlg;

    return (TRUE);
}

// Message handler for notification dialog
LRESULT CALLBACK DlgHandler (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // If a dialog is open, close it before proceeding.
        if (g_hNotificationDlg != NULL)
        {
            ::PostMessage (g_hNotificationDlg, WM_CLOSE, 0, 0);
        }
        return InitNotificationDialog (hDlg);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/**
* Initialize the About dialog 
* 
* @param hDlg handle to the dialog being initialized
*/
BOOL InitAboutDialog (HWND hDlg)
{
    BOOL bAgentRunning = FALSE;
    WCHAR lastUpdateTime [256];
    IPCProxy* pIPCProxy = new IPCProxy ();
    if (pIPCProxy != NULL)
    {
        if (pIPCProxy->Init(IPC_CM_REQUEST_HANDLER))
        {
            IPCREQUEST request, response;

            memset (&request, 0, sizeof (IPCREQUEST));
            request.ulSize = sizeof (IPCREQUEST);

            memset (&response, 0, sizeof (IPCREQUEST));
            response.ulSize = sizeof (IPCREQUEST);

            wcsncpy_s (request.methodName, _countof(request.methodName), GET_AGENT_INFO, _TRUNCATE);
            if (pIPCProxy->Invoke (&request, &response))
            {
                bAgentRunning = TRUE;
                wcsncpy_s (lastUpdateTime, sizeof(lastUpdateTime)/sizeof(WCHAR),
			   response.params[0], _TRUNCATE);
            }

        }
        delete (pIPCProxy);
    }

    if (bAgentRunning)
    {
        TCHAR szRunning[MAX_LOADSTRING];			
        LoadString(g_hResourceModule, IDS_RUNNING, szRunning, MAX_LOADSTRING);
        SetDlgItemText (hDlg, IDC_STATUS, szRunning);
        SetDlgItemText (hDlg, IDC_DEPLOY_TIME, lastUpdateTime);
    }
    else
    {
        TCHAR szNotRunning[MAX_LOADSTRING];			
        LoadString(g_hResourceModule, IDS_NOT_RUNNING, szNotRunning, MAX_LOADSTRING);
        SetDlgItemText (hDlg, IDC_STATUS, szNotRunning);
        SetDlgItemText (hDlg, IDC_DEPLOY_TIME, _T(""));        
    }

    return (TRUE);
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return InitAboutDialog(hDlg);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}
