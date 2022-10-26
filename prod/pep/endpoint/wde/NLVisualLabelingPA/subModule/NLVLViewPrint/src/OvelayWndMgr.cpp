#include "stdafx.h"
#include "OvelayWndMgr.h"

COvelayWndMgr ::COvelayWndMgr ()
{
	InitializeCriticalSection(&m_GlobalSec);
	InitializeCriticalSection(&m_theWnd);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}
COvelayWndMgr ::~COvelayWndMgr ()
{
	DeleteAllOverLayInfo();
	DeleteCriticalSection(&m_theWnd);
	DeleteCriticalSection(&m_GlobalSec);

	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}
COvelayWndMgr& COvelayWndMgr ::GetInstance()
{	
	static COvelayWndMgr Ins;
	return Ins;
}

bool COvelayWndMgr::AddOverlayAndViewProc(__in const HWND& hWnd,__in const HWND& hOverlay,__in const WNDPROC & ViewProc)
{
	bool bRet = false;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.find(hWnd);
	if(it != m_mapWnd.end())
	{
		it->second->SethOverlayWnd(hOverlay);
		it->second->SetViewProc(ViewProc);
		bRet = true;
	}
	LeaveCriticalSection(&m_theWnd);
	return bRet;
}


COverlayWnd* COvelayWndMgr::GetOverLayInfoFromView(__in const HWND& hView)
{
	COverlayWnd* pRet = NULL;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.find(hView);
	if(it != m_mapWnd.end())	pRet = it->second;
	LeaveCriticalSection(&m_theWnd);
	return pRet;
}


COverlayWnd* COvelayWndMgr::GetOverLayInfoFromOverlayWnd(__in const HWND& hOverlay)
{
	COverlayWnd* pRet = NULL;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.begin();
	for(;it != m_mapWnd.end();it++) 
	{
		pRet = it->second;
		if(pRet != NULL)
		{
			if(pRet->GethOverlayWnd() == hOverlay)	break;
		}
	}
	LeaveCriticalSection(&m_theWnd);
	return pRet;
}

COverlayWnd* COvelayWndMgr::GetOverLayInfoFromPath(__in const wstring& strFilePath)
{
	COverlayWnd* pObject=NULL;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.begin();
	for(;it != m_mapWnd.end();it++) 
	{
		pObject = it->second;
		if(pObject != NULL)
		{
			if(pObject->GetstrFilePath() == strFilePath)	break;
		}
	}
	LeaveCriticalSection(&m_theWnd);
	return pObject;
}

//add a record
void COvelayWndMgr::AddOverLayInfoEx(const HWND& hWnd,COverlayWnd* pOverlay)
{
    if(pOverlay == NULL)	return ;
	EnterCriticalSection(&m_theWnd);
	m_mapWnd[hWnd] = pOverlay;
	LeaveCriticalSection(&m_theWnd);
}

void COvelayWndMgr::DeletOverLay(const HWND& hView)
{
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.find(hView);
	if(it != m_mapWnd.end())
	{
		if(it->second != NULL)	delete(it->second);
		m_mapWnd.erase(it);
	}
	LeaveCriticalSection(&m_theWnd);
}
//make sure release all overlay when exit
void COvelayWndMgr::DeleteAllOverLayInfo(void)
{
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.begin();
	for(;it != m_mapWnd.end();it++)
	{
		if(it->second != NULL)
		{
			delete(it->second);
		}
	}
	m_mapWnd.clear();
	LeaveCriticalSection(&m_theWnd);
}

bool COvelayWndMgr::ExistOverLayInfo(const HWND& hWnd)
{
	bool bFind = true;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.find(hWnd);
	if(it == m_mapWnd.end()) bFind = false;
	LeaveCriticalSection(&m_theWnd);
	return bFind;
}

void COvelayWndMgr::SetPrintView(HWND hPrintView,bool bFlag)
{
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.begin();
	for(;it != m_mapWnd.end();it++)
	{
		if(bFlag)
		{
			if((it->second)->GethForWnd() == GetParent(hPrintView))
				(it->second)->SethIfExistPrintView(hPrintView);
		}
		else
		{
			if((it->second)->GethForWnd() == GetParent(hPrintView))
				(it->second)->SethIfExistPrintView(NULL);
		}

	}
	LeaveCriticalSection(&m_theWnd);
}
bool COvelayWndMgr::CheckAdobeFrameWndHadVisibleView(HWND hFrameWnd)
{
	bool bIsHadOneViewVisible = false;
	EnterCriticalSection(&m_theWnd);
	map<HWND,COverlayWnd*>::iterator it = m_mapWnd.begin();
	for(;it != m_mapWnd.end();it++)
	{
		if(it->second->GethForWnd()== hFrameWnd)
		{
			BOOL BRet = IsWindowVisible(it->second->GethViewWnd());
			if (BRet)
			{
				bIsHadOneViewVisible = true;
				break;
			}
		}
	}
	LeaveCriticalSection(&m_theWnd);
	return bIsHadOneViewVisible;
}
