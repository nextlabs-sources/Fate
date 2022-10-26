// addTags.cpp : Implementation of WinMain


#include "stdafx.h"
#include "resource.h"
#include "AddTagsMgr.h"
#include "CriSectionMgr.h"

#include "GSControls.h"

HANDLE g_hMutexWriteSharedMemory = NULL;

class CaddTagsModule : public CAtlExeModuleT< CaddTagsModule >
{
public :
//	DECLARE_LIBID(LIBID_addTagsLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ADDTAGS, "{4595B8E5-8293-4F7F-A88F-49A20EA9A529}")
};

CaddTagsModule _AtlModule;

static CCriSectionMgr g_critical;

static void WriteDataToSharedMemory(HANDLE hMutex, LPTSTR lpCmdLine)
{
	g_log.Log(CELOG_DEBUG, L"Wait to get mutex, cmd: %s\n", lpCmdLine);
	WaitForSingleObject(hMutex, INFINITE);
	g_log.Log(CELOG_DEBUG, L"Wait first instance to read the existing data from shared memory, cmd: %s\n", lpCmdLine);

	HANDLE hCmdReceived = OpenEvent(EVENT_ALL_ACCESS, NULL, EVENT_CMD_RECEIVED);
	if (hCmdReceived == NULL)
	{
		g_log.Log(CELOG_DEBUG, L"Error, Can't open event EVENT_CMD_RECEIVED, %d\n", GetLastError());

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		return;
	}

	do 
	{//it means the data on File Mapping still not read by first instance the EVENT_CMD_RECEIVED is signaled
		DWORD ret = WaitForSingleObject(hCmdReceived, 0);
		if(ret != WAIT_OBJECT_0)
			break;

		g_log.Log(CELOG_DEBUG, L"EVENT_CMD_RECEIVED is still signaled, that means the previous data on File Mapping was still not read by the first instance, cmdline: %s\n", lpCmdLine? lpCmdLine: L"empty");
		Sleep(10);
	} while (true);

	g_log.Log(CELOG_DEBUG, L"Try to write data into shared memory, cmd: %s\n", lpCmdLine);
	WriteSharedMemory(lpCmdLine, (DWORD)_tcslen(lpCmdLine) + 1);

	SetEvent(hCmdReceived);//Tell main instance that there is a new "command" in share memory.
	CloseHandle(hCmdReceived);

	ReleaseMutex(hMutex);

	CloseHandle(hMutex);
}

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR lpCmdLine, int nShowCmd)
{
	InitCommonControls();
	InitializeGSControls();

	CAddTagsMgr* pMgr = CAddTagsMgr::GetInstance();
	if(pMgr)
	{
		pMgr->SetApplicationInstance(hInstance);
	}

	InitLog();


	if(pMgr && lpCmdLine && pMgr->IsValid(lpCmdLine))
	{
		HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, NAME_MUTEX_WRITE_SHAREDMEMORY);
		if(hMutex)//There is an instance for addTags.exe already
		{
			WriteDataToSharedMemory(hMutex, lpCmdLine);
		}
		else
		{//The first instance of addTags.exe
			g_log.Log(CELOG_DEBUG, L"First instance of WdeAddtags.exe, cmd: %s\n", lpCmdLine);

			g_hMutexWriteSharedMemory = CreateMutex(NULL, TRUE, NAME_MUTEX_WRITE_SHAREDMEMORY);

			if(GetLastError() ==  ERROR_ALREADY_EXISTS)
			{
				WriteDataToSharedMemory(g_hMutexWriteSharedMemory, lpCmdLine);
				g_log.Log(CELOG_DEBUG, L"Mutex exists already, cmd: %s\n", lpCmdLine);
			}
			else
			{//first instance
				HANDLE hCmdReceived = CreateEventW(NULL, FALSE, FALSE, EVENT_CMD_RECEIVED);

				CreateSharedMemory();
				WriteSharedMemory(lpCmdLine, (DWORD)_tcslen(lpCmdLine) + 1);

				SetEvent(hCmdReceived);//Set event for "command received"

				if(g_hMutexWriteSharedMemory)
				{
					ReleaseMutex(g_hMutexWriteSharedMemory);
				}

				pMgr->StartWork();

				CloseSharedMemory();

				if(g_hMutexWriteSharedMemory)
				{
					CloseHandle(g_hMutexWriteSharedMemory);
				}
				
				if(hCmdReceived)
				{
					CloseHandle(hCmdReceived);
				}
			}
		}
	}

    return _AtlModule.WinMain(nShowCmd);
}

