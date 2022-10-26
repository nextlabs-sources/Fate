#pragma once
#ifndef _CITEMMANAGER_H__
#define _CITEMMANAGER_H__
#include <string>
#include <list>
#include <shlwapi.h>

using std::wstring;
using std::list;

typedef struct _WORKITEM
{
	wstring		strDocPath;
	HWND		hKeyWnd;
}WORKITEM, *PWORKITEM;

extern BOOL g_bDebugMode_L1;
extern wchar_t g_szLog[1024];
extern wchar_t g_strWinCaption[1025];
extern wchar_t g_strWinClass[MAX_PATH];
class CItemManager
{
private:
	CItemManager()
	{
		InitializeCriticalSection(&m_theSec);
	}
public:
	~CItemManager()
	{
		DeleteCriticalSection(&m_theSec);
	}
	// add a new task to item list
	void AddItem(const WORKITEM& Item)
	{
		if (g_bDebugMode_L1)
		{
			swprintf_s(g_szLog,L"[NLVLOfficeEnforcer:] try to add the blackbox.aspx win handle [%p] at here ........\n",Item.hKeyWnd);
			OutputDebugStringW(g_szLog);
		}
		EnterCriticalSection(&m_theSec);
		m_listItem.push_back(Item);
		LeaveCriticalSection(&m_theSec);
	}
	/*
	*\ check if current item has been handled
	*/
	bool CheckIfExistItem(const HWND& hKeyWnd)
	{
		bool bTrue = false;
		list<WORKITEM>::iterator iter ;
		EnterCriticalSection(&m_theSec);
		for(iter = m_listItem.begin() ;iter != m_listItem.end();++iter)
		{
			WORKITEM &theItem = (*iter);
			if(theItem.hKeyWnd == hKeyWnd)	
			{
				if(::GetClassName(theItem.hKeyWnd,g_strWinCaption,1024) == 0)		
					break;
				if (g_bDebugMode_L1)
				{
					swprintf_s(g_szLog,L"[NLVLOfficeEnforcer:] here we have handle key win handle of [%p] already at here ........\n",hKeyWnd);
					OutputDebugStringW(g_szLog);
				}

				bTrue= true;
				break;
			}
		}
		LeaveCriticalSection(&m_theSec);
		return bTrue;
	}
	/*
	* check if current wnd has invalid and remove it from list if it is invalid.
	*/
	void MaintainItems()
	{
		list<WORKITEM>::iterator iter;
		EnterCriticalSection(&m_theSec);
		for(iter = m_listItem.begin();iter != m_listItem.end();)
		{
			WORKITEM &theItem = (*iter);
			if(::GetClassName(theItem.hKeyWnd,g_strWinCaption,1024) == 0)
			{
				if(GetLastError() == ERROR_INVALID_WINDOW_HANDLE) 
				{
					// erase it and continue
					iter = m_listItem.erase(iter);
					continue;
				}
			}
			iter++;
		}
		LeaveCriticalSection(&m_theSec);
	}
	static CItemManager* GetInstance()
	{
		if(m_pInstance == NULL)
		{
			m_pInstance = new CItemManager;
		}
		return m_pInstance;
	}
	/*
	* For IE refresh window ,overlay will disappear, so we need to check
	*	if there is one win handle invalid and redo-overlay base on it.
	*/
	vector<wstring> CheckInvalidFile()
	{
		vector<wstring> vecFile;
		list<WORKITEM>::iterator iter;
		EnterCriticalSection(&m_theSec);
		for(iter = m_listItem.begin();iter != m_listItem.end();)
		{
			WORKITEM &theItem = (*iter);
			if(::GetClassName(theItem.hKeyWnd,g_strWinCaption,1024) == 0)
			{
				if(GetLastError() == ERROR_INVALID_WINDOW_HANDLE) 
				{
					// erase it and continue
					vecFile.push_back(theItem.strDocPath);
					iter = m_listItem.erase(iter);
					continue;
				}
			}
			iter++;
		}
		LeaveCriticalSection(&m_theSec);
		return vecFile;
	}
private:
	std::list<WORKITEM> m_listItem;
	CRITICAL_SECTION m_theSec;
	static CItemManager* m_pInstance;
};

CItemManager* CItemManager::m_pInstance = NULL;
#endif