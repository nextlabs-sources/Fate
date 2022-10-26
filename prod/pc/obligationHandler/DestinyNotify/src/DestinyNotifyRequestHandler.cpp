/*
 * DestinyNotifyRequestHandler.cpp
 * Author: Fuad Rashid
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */

#include "StdAfx.h"
#include <shellapi.h>
#define SECURITY_WIN32
#include <sddl.h>
#include <security.h>
#include "globals.h"
#include "brain.h"
#include "IPCStub.h" 
#include "DestinyNotify.h"
#include "iipcrequesthandler.h"
#include "NotificationInfo.h"
#include "dsipc.h"
#include "Actions.h"
#include ".\destinynotifyrequesthandler.h"

#define SHOWNOTIFICATION _T("ShowNotification")
#define CUSTOMOBLIGATION _T("CustomObligation")
#define SUCCESS_RESPONSE _T("SUCCESS")
#define FAILURE_RESPONSE _T("FAILURE")
#define MAX_LOADSTRING 100
#define MAX_NOTIFICATIONS 500


#ifdef _DEBUG

static void printLastError(LPCTSTR str)
{
    DWORD lastErr = GetLastError();
    TCHAR errStr[1024];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  NULL, lastErr, 0, errStr, (sizeof errStr / sizeof *errStr),
                  NULL);
    TRACE(1, _T("%s failed, error code %lu, \"%s\"\n"), str, lastErr, errStr);
}

#endif /* _DEBUG */

DestinyNotifyRequestHandler::DestinyNotifyRequestHandler(HWND hMainWnd, NotificationVector* pArray, NOTIFYICONDATA* pNID, HMODULE hResourceModule)
{
    TCHAR* pSID = NULL;
    if (GetUserSID (&pSID))
    {
        _tcsncpy_s (m_szName, _countof(m_szName), pSID, _TRUNCATE);
    }
    else
    {
        m_szName[0] = 0;
		pSID = NULL;
    }
	if( pSID )
	{
		free( pSID );
		pSID = NULL;
	}

    _tcsncat_s (m_szName, _countof(m_szName), _T("DestinyNotifyRequestHandler"), _TRUNCATE);
   
    m_bDisplayActivity = TRUE;
    m_bDisplayAllowActivity = TRUE;
    m_hWnd = hMainWnd;
    m_pNotificationArray = pArray;
    m_pNID = pNID;
    m_hResourceModule = hResourceModule;

    TCHAR szAction[MAX_LOADSTRING];
    TCHAR szText[MAX_LOADSTRING];
    ::LoadString (m_hResourceModule, IDS_DELETE_ACTION, szAction, MAX_LOADSTRING);
    ::LoadString (m_hResourceModule, IDS_DELETE_TITLE, szText, MAX_LOADSTRING);
    m_actionTitleMap[szAction]= szText;
    ::LoadString (m_hResourceModule, IDS_COPY_ACTION, szAction, MAX_LOADSTRING);
    ::LoadString (m_hResourceModule, IDS_COPY_TITLE, szText, MAX_LOADSTRING);
    m_actionTitleMap[szAction]= szText;
    ::LoadString (m_hResourceModule, IDS_PASTE_TITLE, szText, MAX_LOADSTRING);
    m_actionTitleMap[L"PASTE"]= szText;

    ::LoadString (m_hResourceModule, IDS_DETAILS, szText, MAX_LOADSTRING);
    m_singleMessageSuffix = _T("\n\n");
    m_singleMessageSuffix += szText;
    ::LoadString (m_hResourceModule, IDS_MULTIPLE_DENY, szText, MAX_LOADSTRING);
    m_multipleDenyMsg = szText;
    ::LoadString (m_hResourceModule, IDS_ALL_NOTIFICATIONS, szText, MAX_LOADSTRING);
    m_multipleDenyMsg += _T("\n\n");
    m_multipleDenyMsg += szText;
}

DestinyNotifyRequestHandler::~DestinyNotifyRequestHandler(void)
{
}

/**
*
* invokes a method. The method name is specified in the request object
* The implementation of this method must zero out the response object 
* and set the ulSize parameter of the response object
* 
* @param request
*            request object
*            Supported Methods:
*            ShowNotification (params: time, event, file, message1, [message2], [message3], [message4])
* @param response
*            pointer response object
* @return true if invocation is successful
* 
*/
bool DestinyNotifyRequestHandler::Invoke(IPCREQUEST& request, IPCREQUEST* pResponse)
{
    if ( _tcscmp (request.methodName, SHOWNOTIFICATION) == 0 &&
	 IsDisplayUIEnabled() == true )
    {
        bool bMultiple = false;

	Shell_NotifyIcon(NIM_ADD, m_pNID);

        NotificationInfo * pInfo = NULL;

        // Create a NotificationInfo object for each message
        for (int i = 3; i < 7; i++)
        {
            //Always add first message. For the rest, only add if present
            if (i == 3 || _tcslen (request.params [i]) != 0)
            {
                if (i == 4)
                {
                    bMultiple = true;
                }
                pInfo = new NotificationInfo;
                pInfo->time = request.params[0];
                pInfo->enforcement = DENY; // default to DENY
                if (request.params[1][0] == L'D')
                  pInfo->enforcement = DENY;
                else if (request.params[1][0] == L'A')
                  pInfo->enforcement = ALLOW;

                pInfo->event = &(request.params[1][1]);
                pInfo->file = request.params[2];
                pInfo->message = request.params[i];
				if(MAX_NOTIFICATIONS == m_pNotificationArray->size() )
				{
					NotificationInfo *last_pInfo = m_pNotificationArray->at(MAX_NOTIFICATIONS - 1 );
					m_pNotificationArray->pop_back();
					delete last_pInfo;
				}
                m_pNotificationArray->insert(m_pNotificationArray->begin(), pInfo);
            }
        }

        if (m_bDisplayActivity && request.params [3][0] != 0)
        {

          if (pInfo->enforcement == DENY || // DENY only governed by the display Activity flag
              (m_bDisplayAllowActivity && pInfo->enforcement == ALLOW)) 
          {
            // show notification
            NOTIFYICONDATA nid; 

            nid.cbSize = sizeof(NOTIFYICONDATA); 
            nid.hWnd = m_hWnd; 
            nid.uID = TRAY_ICON_01; //ID
            nid.uFlags = NIF_INFO; 
            nid.uTimeout = 1000;
            nid.dwInfoFlags = NIIF_INFO;

            // TODO: what if action is not found in map, replace with generic msg.
            if (m_actionTitleMap.find (request.params[1]) != m_actionTitleMap.end())
            {
                _tcsncpy_s (nid.szInfoTitle, 64, m_actionTitleMap.find(request.params[1])->second.c_str(), _TRUNCATE); 
            }
            else
            {
                _tcsncpy_s (nid.szInfoTitle, 64, _T("Policy message"), _TRUNCATE); 
            }

            _tcsncpy_s (nid.szInfo, 256, request.params [3], _TRUNCATE); 
            _tcsncat_s (nid.szInfo, 256, m_singleMessageSuffix.c_str(), _TRUNCATE);            
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            //else // multiple messages
            //{
            //    _tcsncpy (nid.szInfo, m_multipleDenyMsg.c_str(), 255); 
            //}
          }
        }
    }
    else if (_tcscmp (request.methodName, CUSTOMOBLIGATION) == 0)
    {
        STARTUPINFO startupInfo;
        PROCESS_INFORMATION procInfo;
        BOOL ret;

        ZeroMemory(&startupInfo, sizeof startupInfo);
        startupInfo.cb = sizeof startupInfo;

        // Concatenate all params to a single command line.
        size_t size = _countof(request.params) * (_countof(request.params[0]) - 1) + 1;
        LPTSTR cmdline = new TCHAR[size];

        cmdline[0] = _T('\0');

        for (int i = 0; i < _countof(request.params); i++)
        {
            _tcsncat_s(cmdline, size, request.params[i], _TRUNCATE);
        }

        TRACE(1, _T("About to launch command: %s\n"), cmdline);
        ret = CreateProcess(NULL,               // lpApplicationName
                            cmdline,            // lpCommandLine
                            NULL,               // lpProcessAttributes
                            NULL,               // lpThreadAttributes
                            FALSE,              // bInheritHandles
                            0x0,                // dwCreationFlags
                            NULL,               // lpEnvironment
                            NULL,               // lpCurrentDirectory
                            &startupInfo,       // lpStartupInfo
                            &procInfo           // lpProcessInformation
                            );
        delete []cmdline;

        if (ret)
        {
            TRACE(1, _T("Process launched, ID=%ld\n"), procInfo.dwProcessId);
            CloseHandle(procInfo.hThread);
            CloseHandle(procInfo.hProcess);
        }
        else
        {
#ifdef _DEBUG
            printLastError(_T("CreateProcess"));
#endif
            _tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), FAILURE_RESPONSE, _TRUNCATE);
            return (false);
        }
    }
    else 
    {
        return (false);
    }

    _tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), SUCCESS_RESPONSE, _TRUNCATE);
    return (true);

}

/**
* @return name of request handler to use for generating 
* unique names for OS objects
*/
TCHAR* DestinyNotifyRequestHandler::GetName ()
{
    return (m_szName);
}

BOOL DestinyNotifyRequestHandler::GetUserSID(TCHAR **pszUserName)
{
    HANDLE hToken;
    HANDLE hCurrentProcess = GetCurrentProcess();
    BOOL bSIDFound = FALSE;
    if(::OpenProcessToken(hCurrentProcess, TOKEN_READ, &hToken))
    {
        DWORD dwLen = NULL;
        //Pass NULL to get the right buffer size
        ::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLen);
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            PTOKEN_USER pTokenUser = (PTOKEN_USER) malloc (dwLen);
            if(::GetTokenInformation(hToken, TokenUser, pTokenUser, dwLen, &dwLen))
            {
                TCHAR* pszSID = NULL;
                ::ConvertSidToStringSid((PSID) pTokenUser->User.Sid, &pszSID);
                *pszUserName = (TCHAR*) malloc (sizeof(TCHAR) * (_tcslen (pszSID) + 1));
                _tcsncpy_s (*pszUserName, _tcslen (pszSID) + 1, pszSID, _TRUNCATE);
                ::LocalFree (pszSID);
                bSIDFound = TRUE;
            }
            free (pTokenUser);
        }
        ::CloseHandle(hToken);
    }
    ::CloseHandle(hCurrentProcess);
    return bSIDFound; 
}
