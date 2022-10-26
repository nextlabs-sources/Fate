// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "NTXLBS_Bubble.h"
#include <list>
#include <map>
#include <string>
#include "celog.h"
#include "utilities.h"
#include "WndShadow.h"
CELog g_log;

CNotifyDlg* g_pDetailDlg = NULL; 



CRITICAL_SECTION g_CriSection;
std::list<BUBBLE> g_listBubbles;
HANDLE g_eventThread;
HANDLE g_hThread;

HMODULE g_hInst;

CRITICAL_SECTION g_csOfShowBubble;
std::map<std::wstring, DWORD> g_BubbleMap;
const DWORD g_DiffMax = 1500;

/********************************************************
This thread will check the status of all bubbles.
Close the "timeout" bubble ,and release memory.
********************************************************/
unsigned __stdcall CheckBubbleThread( void* pArguments )
{
	pArguments;

	while(1)
	{
		Sleep(20);
		if(WAIT_OBJECT_0 == WaitForSingleObject(g_eventThread, 0))
		{
			return 0;
		}

		std::list<BUBBLE>::iterator iter;
		EnterCriticalSection(&g_CriSection);
		for(iter = g_listBubbles.begin(); iter != g_listBubbles.end();)
		{//Check all the bubbles.
			BUBBLE bubble = (*iter);
			
			if(bubble.pBubble && bubble.nDuration > 0 && bubble.pBubble->m_hWnd && ::IsWindow(bubble.pBubble->m_hWnd) && ::IsWindowVisible(bubble.pBubble->m_hWnd))
			{
				if((int)GetTickCount() - bubble.nEntryTime < 0 || (int)GetTickCount() - bubble.nEntryTime > bubble.nDuration)
				{//Close the bubble if it exceeds duration.
					bubble.pBubble->CloseBubble();
				}
			}

			if(bubble.pBubble && bubble.pBubble->IsClosed())
			{//The bubble was closed by user, delete it.
				delete bubble.pBubble;
				iter = g_listBubbles.erase(iter);
				//iter = g_listBubbles.begin();
				continue;
			} 

			iter++;
		}
		LeaveCriticalSection(&g_CriSection);
	}
	return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID /*lpReserved*/
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			static bool bAttached = false;
			if(!bAttached)
			{
				g_hInst = hModule;
				InitializeCriticalSection(&g_CriSection);
                InitializeCriticalSection(&g_csOfShowBubble);

				g_eventThread = CreateEventW(NULL, TRUE, FALSE, NULL);
				unsigned threadID = 0;
				g_hThread = (HANDLE)_beginthreadex( NULL, 0, &CheckBubbleThread, NULL, 0, &threadID );

				//Create a global dialog, which will show the details of notification.
				g_pDetailDlg = new CNotifyDlg;

				if(g_pDetailDlg)
				{
					g_pDetailDlg->Create(NULL);
				}

				CWndShadow::Initialize(hModule);

				bAttached = true;

			}
			
			if (edp_manager::CCommonUtilities::InitLog(g_log, EDPM_MODULE_NOTIFY))
			{
				g_log.Log(CELOG_DEBUG, L"init log succeed in nl_notification\n");
			}

		}
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	case DLL_PROCESS_DETACH:
		{
			//Reset event to let thread exit first.
			bool bDetached = false;
			if(!bDetached)
			{
				SetEvent(g_eventThread);
				WaitForSingleObject(g_hThread, INFINITE);
                DeleteCriticalSection(&g_csOfShowBubble);
				DeleteCriticalSection(&g_CriSection);
				CloseHandle(g_eventThread);

				if(g_pDetailDlg)
				{
					if(g_pDetailDlg->m_hWnd)
					{
						DestroyWindow(g_pDetailDlg->m_hWnd);
					}

					delete g_pDetailDlg;
					g_pDetailDlg = NULL;
				}

				bDetached = true;
			}
			
		}
		break;
	}
	return TRUE;
}



void __stdcall notify(void* pInfo, int nDuration, HWND hCallingWnd, void* pReserved) 
{
// 	HINSTANCE hOldInst = _AtlBaseModule.GetResourceInstance();
// 	_AtlBaseModule.SetResourceInstance(g_hInst);

	g_log.Log(CELOG_DEBUG, L"notify() was called to show bubble, id: %d\n", (INT_PTR)pReserved);
	if(!pInfo)
	{
		return;
	}

	CNTXLBS_Bubble* pBubble;
	pBubble = new CNTXLBS_Bubble ;

	if(!pBubble)
	{
		return;
	}

	pBubble->SetCallingWnd(hCallingWnd);
	pBubble->SetDefaultBubbleSize(310, 80);

#pragma warning(push)
#pragma warning(disable:4311)
	pBubble->SetInfo(pInfo, (int)pReserved);
#pragma warning(pop)

 	pBubble->Create(NULL);
 	pBubble->ShowWindow(SW_SHOWNORMAL);

//	::SetWindowPos(pBubble->m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);

	BUBBLE bubble;
	bubble.pBubble = pBubble;
	bubble.nEntryTime = (int)GetTickCount();
	bubble.nDuration = nDuration;

	EnterCriticalSection(&g_CriSection);
	g_listBubbles.push_back(bubble);
	LeaveCriticalSection(&g_CriSection);

//	_AtlBaseModule.SetResourceInstance(hOldInst);


}

void __stdcall notify2(void* pInfo, int nDuration, HWND hCallingWnd, void* pReserved, const WCHAR* pHyperLink) 
{
    if(!pInfo || !pHyperLink)
    {
        return;
    }
    
    DWORD dwCurTick = GetCurrentTime();

	EnterCriticalSection(&g_csOfShowBubble);
    
    typedef struct  
    {
        ULONG ulSize;
        WCHAR methodName [64];
        WCHAR params [7][256];
    }NOTIFY_INFO;

    DWORD dwLastTick = g_BubbleMap[((NOTIFY_INFO*)pInfo)->params[1]];
    g_BubbleMap[((NOTIFY_INFO*)pInfo)->params[1]] = dwCurTick;
    
    LeaveCriticalSection(&g_csOfShowBubble);
    
    if (dwCurTick - dwLastTick < g_DiffMax)
    {
        return;
    }

    CNTXLBS_Bubble* pBubble;
    pBubble = new CNTXLBS_Bubble ;

    if(!pBubble)
    {
        return;
    }

    pBubble->SetCallingWnd(hCallingWnd);
    pBubble->SetDefaultBubbleSize(310, 80);

#pragma warning(push)
#pragma warning(disable:4311)
    pBubble->SetInfo(pInfo, (int)pReserved, pHyperLink);
#pragma warning(pop)

    pBubble->Create(NULL);
    pBubble->ShowWindow(SW_SHOWNORMAL);

    BUBBLE bubble;
    bubble.pBubble = pBubble;
    bubble.nEntryTime = (int)GetTickCount();
    bubble.nDuration = nDuration;

    EnterCriticalSection(&g_CriSection);
    g_listBubbles.push_back(bubble);
    LeaveCriticalSection(&g_CriSection);
}