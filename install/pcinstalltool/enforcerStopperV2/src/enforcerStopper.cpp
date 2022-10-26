
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "CEsdk.h"
#include "enforcerStopper.h"
#include <io.h>
#include <Fcntl.h>
#include <stdio.h>

#define MAX_LOADSTRING   100
#define MAX_PASSWORD_LEN 1000

#define  TIMER_SET_FORCEGROUND_WND   1002

// Global variables.  A lazy way to pass around variables previously

HINSTANCE hInst;                // current instance
HWND      g_hWnd;
TCHAR     szTitle[MAX_LOADSTRING]; // The title bar text
TCHAR     szMsgAgentStopped[MAX_LOADSTRING];
TCHAR     szMsgAgentNotStopped[MAX_LOADSTRING];
TCHAR     szMsgAgentNotRunning[MAX_LOADSTRING];
TCHAR     szMsgIncorrectPassword[MAX_LOADSTRING];

bool g_bNeedQuench = false;

int  NLQuench();

/**
 * Returns TRUE if a password can be retreived from the command line.
 *
 */
BOOL GetPasswordFromCmdLine (LPCTSTR pszCmdLine, LPWSTR pszPassword)
{
  LPWSTR *szArglist;
  int nArgs;
  int i;
  BOOL bRet = FALSE;
  
  szArglist = CommandLineToArgvW(pszCmdLine, &nArgs);
  if( NULL != szArglist )
  {
    for(i = 0; i < nArgs - 1; i++) {
      if (_wcsicmp (L"-p", szArglist[i]) == 0 || 
          _wcsicmp (L"/p", szArglist[i]) == 0)
      {
        wcsncpy_s(pszPassword, MAX_PASSWORD_LEN, szArglist [i + 1], _TRUNCATE);
        bRet = TRUE;
        break;
      }
    }
  }

  // Free memory allocated for CommandLineToArgvW arguments.
  LocalFree(szArglist);
  return bRet;
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  g_hWnd = CreateDialog(hInstance, (LPCTSTR) IDD_PASSWORD, NULL, (DLGPROC) WndProc);

  if (!g_hWnd)
  {
	  OutputDebugString(_T("Create Password dialog failed.\n"));
    return FALSE;
  }

  ShowWindow(g_hWnd, nCmdShow );
  UpdateWindow(g_hWnd);
  SetWindowPos(g_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  SetTimer(g_hWnd, TIMER_SET_FORCEGROUND_WND, 500, NULL);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process dialog buttons
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId;
  CEResult_t err;

  switch (message) 
  {
	  case WM_TIMER:
  {
			if (wParam == TIMER_SET_FORCEGROUND_WND)
			{

				HWND hwnd = GetForegroundWindow();
				if (hwnd != g_hWnd)
				{
					OutputDebugString(_T("Set EnforceStopper window to foreground\n"));

					BYTE keyState[256] = { 0 };

					//to unlock SetForegroundWindow we need to imitate Alt pressing
					if (::GetKeyboardState((LPBYTE)&keyState))
					{
						if (!(keyState[VK_MENU] & 0x80))
						{
							::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
						}
					}

					::SetForegroundWindow(g_hWnd);

					if (::GetKeyboardState((LPBYTE)&keyState))
					{
						if (!(keyState[VK_MENU] & 0x80))
						{
							::keybd_event(VK_MENU, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
						}
					}
				}		
			}
			
  }
	  break;
  case WM_INITDIALOG:
    return true;
    break;
  case WM_COMMAND:
    wmId    = LOWORD(wParam); 
    switch (wmId)
    {
	case IDOK:
	{
				 TCHAR szPassword[MAX_PASSWORD_LEN];
				 UINT uPasswordCount = ::GetDlgItemText(g_hWnd, IDC_PASSWORD, szPassword, MAX_PASSWORD_LEN);

				 if (uPasswordCount)
				 {
					 err = stopAgentService(szPassword);
					 switch (err)
					 {
					 case CE_RESULT_SUCCESS:
						 // MessageBox (g_hWnd, szMsgAgentStopped,      szTitle, MB_OK | MB_ICONINFORMATION); 
						 EndDialog(g_hWnd, IDOK);
						 PostQuitMessage(0);
						 break;
					 case CE_RESULT_PERMISSION_DENIED:
						 MessageBox(g_hWnd, szMsgIncorrectPassword, szTitle, MB_OK | MB_ICONERROR);
						 break;
					 case CE_RESULT_CONN_FAILED:
						 EndDialog(g_hWnd, IDOK);
						 PostQuitMessage(0);
						 break;
					 default:
						 //MessageBox (g_hWnd, szMsgAgentNotRunning,   szTitle, MB_OK | MB_ICONERROR); 
						 g_bNeedQuench = true;
						 EndDialog(g_hWnd, IDOK);
						 PostQuitMessage(1603);
						 break;
						 //  default:
						 //  MessageBox (g_hWnd, szMsgAgentNotStopped,   szTitle, MB_OK | MB_ICONERROR); 
						 //   break;
					 }
				 }
				 else
				 {
					 // Password could not be read
					   MessageBox (g_hWnd, _T("Please enter password."), szTitle, MB_OK | MB_ICONERROR); 
				 }
	}
      break;
    case IDM_EXIT:
    case IDCANCEL:
      EndDialog(g_hWnd, IDCANCEL);
	  PostQuitMessage(1602);
      break;
    default:
      return 0;
    }
    break;
  }
  return 0;
}


/*int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)*/

int EnforcerStop(HINSTANCE hInstance, LPCTSTR lpCmdLine, int nCmdShow)
{

    if (stopAgentService(L"82q0958213434128503412uir0euwqr0urlksdajf") == CE_RESULT_CONN_FAILED)
    {
        return 0;
    }


  MSG msg;
  TCHAR szPassword [MAX_PASSWORD_LEN];

  LoadString (hInstance, IDS_APP_TITLE,              szTitle,                MAX_LOADSTRING);
  LoadString (hInstance, IDS_MSG_AGENT_STOPPED,      szMsgAgentStopped,      MAX_LOADSTRING);
  LoadString (hInstance, IDS_MSG_AGENT_NOT_STOPPED,  szMsgAgentNotStopped,   MAX_LOADSTRING);
  LoadString (hInstance, IDS_MSG_AGENT_NOT_RUNNING,  szMsgAgentNotRunning,   MAX_LOADSTRING);
  LoadString (hInstance, IDS_MSG_INCORRECT_PASSWORD, szMsgIncorrectPassword, MAX_LOADSTRING);

  OutputDebugString(lpCmdLine);
  if (GetPasswordFromCmdLine (lpCmdLine, szPassword))
  {
	  OutputDebugString(_T("get password from commandline\n"));
    CEResult_t nResult =  (stopAgentService (szPassword));
	if ((nResult == CE_RESULT_SUCCESS) || (nResult == CE_RESULT_PERMISSION_DENIED))
	{
		return ((nResult == CE_RESULT_SUCCESS) ? 0 : 1603);
	}
	else if (nResult == CE_RESULT_CONN_FAILED)
	{
		return 0;
	}
	else 
	{
		g_bNeedQuench = true;
	}
  }
  else
  {
	  OutputDebugString(_T("didn't get password from commandline\n"));

	  // Perform application initialization:
	  if (!InitInstance(hInstance, nCmdShow))
	  {
		  return FALSE;
	  }

	  // Main message loop:
	  while (GetMessage(&msg, NULL, 0, 0))
	  {
		  if (!::IsDialogMessage(g_hWnd, &msg))
		  {
			  TranslateMessage(&msg);
			  DispatchMessage(&msg);
		  }
	  }

	  if (!g_bNeedQuench)
	  {
		  return (int)msg.wParam;
	  }
  }

  //need call NLQuench
  if (g_bNeedQuench)
  {
	  return NLQuench()==0 ? 0 : 1603;
  }

}
