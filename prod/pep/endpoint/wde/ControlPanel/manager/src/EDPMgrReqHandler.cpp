#include "StdAfx.h"
#include <shellapi.h>
#define SECURITY_WIN32
#include <sddl.h>
#include <security.h>
#include "EDPMgrReqHandler.h"
#include "NotifyMgr.h"
#include "EDPMgrUtilities.h"
#include "Actions.h"
#include "EDPMgr.h"


#define SHOWNOTIFICATION _T("ShowNotification")
#define CUSTOMOBLIGATION _T("CustomObligation")
#define SUCCESS_RESPONSE _T("SUCCESS")
#define FAILURE_RESPONSE _T("FAILURE")

CEDPMgrReqHandler::CEDPMgrReqHandler(void)
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

	
	//	we need change handler name to "EDPManagerRequestHandler" before release, we should ask Nao/Scott' help to do it.
	//_tcscat (m_szName, _T("EDPManagerRequestHandler"));
	_tcsncat_s (m_szName, _countof(m_szName), _T("DestinyNotifyRequestHandler"), _TRUNCATE);
}

CEDPMgrReqHandler::~CEDPMgrReqHandler(void)
{
}

bool CEDPMgrReqHandler::Invoke(IPCREQUEST& request, IPCREQUEST* pResponse)
{

	CEDPMgr& edpMgr = CEDPMgr::GetInstance();
	CELog& log = edpMgr.GetCELog();

	//	check the type of this request
	//	check if it is a custom obligation
	if (_tcscmp (request.methodName, CUSTOMOBLIGATION) == 0)
	{
		//	yes, it is a custom obligation
		STARTUPINFO startupInfo;
		PROCESS_INFORMATION procInfo;
		BOOL ret;

		ZeroMemory(&startupInfo, sizeof startupInfo);
		startupInfo.cb = sizeof startupInfo;

		size_t size = _countof(request.params) * (_countof(request.params[0]) - 1) + 1;
		// Concatenate all params to a single command line.
		LPTSTR cmdline = new TCHAR[size];

		cmdline[0] = _T('\0');

		for (int i = 0; i < _countof(request.params); i++)
		{
			_tcsncat_s(cmdline, size, request.params[i], _TRUNCATE);
		}

		log.Log(CELOG_DEBUG, L"About to launch command: %s\n", cmdline);
		
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
			_tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), FAILURE_RESPONSE, _TRUNCATE);
			return (false);
		}

		_tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), SUCCESS_RESPONSE, _TRUNCATE);
		return true;
	}

	//	check if it is a show notification
	if ( _tcscmp (request.methodName, SHOWNOTIFICATION) == 0 )
	{
		//	yes, it is a show notification
		log.Log(CELOG_DEBUG, L"receive a notify from IPC\n");

		//	add request into history
		CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
		edpUtilities.AddNotifyToHistory((CEDPMUtilities::NOTIFY_INFO&) request);

	
		//	check if we need to show notification window to end user
		BOOL bNotify = NeedShowNotify();


		//	if we need to display, send message
		if (bNotify)
		{
			SendMessageW(g_hNotifyMsgLoop, NOTIFY_RECEIVE_MSG, edpUtilities.GetNotificationCount(), (LPARAM)&request);
		}


		//	always return succeed
		_tcsncpy_s (pResponse->params[0], _countof(pResponse->params[0]), SUCCESS_RESPONSE, _TRUNCATE);

	}
	
	return true;
}


TCHAR* CEDPMgrReqHandler::GetName ()
{
	return (m_szName);
}


BOOL CEDPMgrReqHandler::GetUserSID(TCHAR **pszUserName)
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


/*
check if we need to show the last notification through notification window to end user.
*/
BOOL CEDPMgrReqHandler::NeedShowNotify()
{
	//	remember current ticket
	DWORD dwCurTick = ::GetCurrentTime();


		//	check if need to send message to notification plugin to pop up dialog,
	CEDPMgr& edpMgr = CEDPMgr::GetInstance();
	CELog& log = edpMgr.GetCELog();

		//	first, check display level;
	CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
		CEDPMUtilities::NOTIFY_DIS_LEVEL eLevel;
		edpUtilities.GetNotifyDisplayLever(eLevel);

		//	check if the notify is block notify
		CEDPMUtilities::NotificationVector notifyArray;
		edpUtilities.GetNotifyHistory(notifyArray);
		BOOL bBlock = ( notifyArray.back().enforcement == DENY ); //	defined in actions.h
		BOOL bNotify = FALSE;

	//	determine if we need to show notification window according to current display level and the notify type(deny/allow).
		switch(eLevel)
		{
		case CEDPMUtilities::E_ALL:
			{
			log.Log(CELOG_DEBUG, L"we will show all notification\n");
				bNotify = TRUE;
			}
			break;
		case CEDPMUtilities::E_BLOCK_ONLY:
			{
				if (bBlock)
				{
				log.Log(CELOG_DEBUG, L"we will only show show block notification, this is\n");
					bNotify = TRUE;
				}
				else
				{
					log.Log(CELOG_DEBUG, L"we will only show show block notification, this isn't\n");
				bNotify = FALSE;
				}
			}
			break;
		case CEDPMUtilities::E_NONE:
			{
				bNotify = FALSE;
				log.Log(CELOG_DEBUG, L"we suppress all notifications\n");
			}
			break;
		default:
			{
				log.Log(CELOG_DEBUG, L"we meet unexpected error case in invoke of request handler\n");
			}
			break;
		}

	if(!bNotify)
		{
		//	we don't need to show this notify according to current display level and the notify type(deny/allow).
		goto FUN_EXIT;
		}

	//	we need to show this notify, but, we have to check if this is an duplicated event.
	//	for example, if OE query 100 times for one sent out email for the same action,
	//	we only show notify window once.

	//	check action first.
	if (m_sLastNotify.strAction == notifyArray.back().action)
	{
		//	yes, the action is the same as previous notify action
		//	we need to check the time ticket, if these two notify is very timely close, we don't show 
		//	this notify
		if (dwCurTick - m_sLastNotify.dwTick < c_dwDiffMax)
		{
			log.Log(CELOG_DEBUG, L"current notify action is %s, same as previous notify, their time difference is less than %d ms, we don't show it\n", \
				notifyArray.back().action, c_dwDiffMax);
			bNotify = FALSE;
			goto FUN_EXIT;
		}
	}
	
	//	the action is different, we must show or time difference is larger then c_dwDiffMax
	//	we need to show this notify
	log.Log(CELOG_DEBUG, L"current notify action is %s, previous is %s, their time diff is %d ms, we need to show this notify\n", \
		notifyArray.back().action, m_sLastNotify.strAction.c_str() ? m_sLastNotify.strAction.c_str() : L"", dwCurTick - m_sLastNotify.dwTick);

	goto FUN_EXIT;

FUN_EXIT:

	//	save current notify' time and action
	m_sLastNotify.dwTick = dwCurTick;
	m_sLastNotify.strAction = notifyArray.back().action;

	return bNotify;
}
