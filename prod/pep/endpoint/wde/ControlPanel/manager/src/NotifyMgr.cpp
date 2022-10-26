#include "StdAfx.h"
#include "NotifyMgr.h"
#include "IniFile.h"
#include "SysUtils.h"
#include "EDPMgrUtilities.h"


#define NOTIFY_SECTION "notify style"


typedef void (__stdcall *NotifyType)(void* pInfo, int nDuration, HWND hCallingWnd, void* pReserved);

CEDPNotifyMgr::CEDPNotifyMgr(void)
{
}

CEDPNotifyMgr::~CEDPNotifyMgr(void)
{
}

CEDPNotifyMgr& CEDPNotifyMgr::GetInstance()
{
	static CEDPNotifyMgr NotifyMgr;
	return NotifyMgr;
}

bool CEDPNotifyMgr::ShowNotify(PVOID notifyInfo, int nIDNotification)
{
	char szType[20] = {0};
	char szPath[MAX_PATH] = {0};

	int nType = 0;
	if(ReadConfig(szType, 20, szPath, MAX_PATH))
	{
		nType = ::atoi(szType);
	}

	char szFullPath[MAX_PATH] = {0};
	ParseNotifyConfig(nType, szPath, szFullPath, MAX_PATH);

	if(strlen(szFullPath) > 0)
	{
		HMODULE hNotifyDLL = LoadLibraryA(szFullPath);
		if(!hNotifyDLL)//Load notify dll failed.
		{
			CEDPMgr& edpMgr = CEDPMgr::GetInstance();
			edpMgr.GetCELog().Log(CELOG_DEBUG, "Failed to load notify dll, path %s\n", szFullPath);
			return false;
		}

		NotifyType lfNotify = NULL;
		lfNotify = (NotifyType)GetProcAddress(hNotifyDLL, "notify");
		if(!lfNotify)
		{
			CEDPMgr& edpMgr = CEDPMgr::GetInstance();
			edpMgr.GetCELog().Log(CELOG_DEBUG, "Failed to get function address \"notify\"\n");
			return false;
		}

		CEDPMUtilities& edpUtilities = CEDPMUtilities::GetInstance();
		CEDPMUtilities::NOTIFY_DIS_DURATION eDuration = CEDPMUtilities::E_USER_CLOSE;
		edpUtilities.GetNotifyDuration(eDuration);
		DWORD dwDuration = 0;
		switch(eDuration)
		{
		case CEDPMUtilities::E_USER_CLOSE:
			dwDuration = 0;
			break;
		case CEDPMUtilities::E_45_SECONDS:
			dwDuration = 45000;
			break;
		case CEDPMUtilities::E_30_SECONDS:
			dwDuration = 30000;
			break;
		case CEDPMUtilities::E_15_SECONDS:
			dwDuration = 15000;
			break;
		default:
			break;
		}

#pragma warning(push)
#pragma warning(disable:4312)
		lfNotify(notifyInfo, dwDuration, g_hMainWnd, (void*)nIDNotification);
#pragma warning(pop)

		return true;
	}

	return false;
}

bool CEDPNotifyMgr::ReadConfig(char* pNotifyType, int nTypeLen, char* pNotifyPath, int nPathLen)
{
	if(!pNotifyPath || !pNotifyType)
	{
		return false;
	}

	char szPath[MAX_PATH + 1] = {0};
	if( !MyGetCurrentDirectory(szPath, MAX_PATH))
	{
		return false;
	}

	
	strncat_s(szPath, MAX_PATH, "notify.ini", _TRUNCATE);

	CIniFile* pIni = new CIniFile(szPath);

	if (!pIni)
	{
		return false;
	}

	//read notify type
	char szType[20] = {0};
	pIni->ReadString(NOTIFY_SECTION, "type", "0", szType, 20);

	if(strlen(szType) <= 0)
	{
		return false;
	}

	strncpy_s(pNotifyType, nTypeLen, szType, _TRUNCATE);

	//Read notify dll path
	char szNotifyDLLPath[MAX_PATH] = {0};
	pIni->ReadString(NOTIFY_SECTION, "path", "", szNotifyDLLPath, MAX_PATH);

	strncpy_s(pNotifyPath, nPathLen, szNotifyDLLPath, _TRUNCATE);

	if(pIni)
	{
		delete pIni;
		pIni = NULL;
	}

	return true;
}

void CEDPNotifyMgr::ParseNotifyConfig(const int nType, const char* pNotifyPath, IN OUT char* pFullPath, int nLen)
{
	if(!pNotifyPath || !pFullPath)
	{
		return;
	}

	int nTypeTmp = nType;
	if(nTypeTmp == 2 && (pNotifyPath == NULL || strlen(pNotifyPath) == 0 ))
	{//User defines "custom notification", but he/she didn't type in the path of "custom notification DLL". if so, we use "default" notification.
		nTypeTmp = 0;
	}

	if(nTypeTmp > 2 || nTypeTmp < 0)
	{
		nTypeTmp = 0;
	}

	switch (nTypeTmp)
	{
	case 0: //default notification
		{
			char szPath[MAX_PATH + 1] = {0};
			MyGetCurrentDirectory(szPath, MAX_PATH);

#ifdef _WIN64
			strncat_s(szPath, MAX_PATH, "notification.dll", _TRUNCATE);
#else
			strncat_s(szPath, MAX_PATH, "notification32.dll", _TRUNCATE);
#endif
	
			strncpy_s(pFullPath, nLen, szPath, _TRUNCATE);

		}
		break;
	case 1:// build in custom notification
		break;
	case 2:// custom notification
		{
			if(pNotifyPath)
			{
				strncpy_s(pFullPath, nLen, pNotifyPath, _TRUNCATE);
			}
		}
		break;
	default:
		break;
	}

}
