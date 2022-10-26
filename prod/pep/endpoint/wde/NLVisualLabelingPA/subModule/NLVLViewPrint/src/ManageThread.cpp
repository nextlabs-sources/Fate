#include "ManageThread.h"
#include <process.h>
#define WM_SETFORGROUND WM_USER+7

CManageThread::CManageThread():m_bInit(false),m_hEvent(NULL)
{
	m_FirstChildWnd = NULL;
	m_targetViewWnd = NULL;
	m_TimeOut = 0;
	SetRect(&m_FirstChildWndRect,0,0,0,0);
	InitializeCriticalSection(&m_secNeedAddView);
	InitializeCriticalSection(&m_secInit);

}
CManageThread::~CManageThread()
{
	ExistThread();
	DeleteCriticalSection(&m_secInit);
	DeleteCriticalSection(&m_secNeedAddView);
}

CManageThread& CManageThread::GetInstance(void)
{
	static CManageThread theIns;
	return theIns;
}
void CManageThread::ExistThread()
{
	if(m_hEvent != NULL)
	{
		SetEvent(m_hEvent);
		if(m_hThread != INVALID_HANDLE_VALUE)
			::WaitForSingleObject(m_hThread,1000);
		CloseHandle(m_hEvent);
		CloseHandle(m_hThread);
		m_hEvent = NULL;
	}
}
bool CManageThread::CheckIfNeedComputeRgn(HWND view,RECT currentRect)
{
	RECT viewRect,DstRect;
	GetWindowRect(view,&viewRect);
	if(IntersectRect(&DstRect,&viewRect,&m_FirstChildWndRect) || IntersectRect(&DstRect,&viewRect,&currentRect) )
		return true;
	return false;
}
void CManageThread::ChangeRectCoordinate(RECT& rect)
{
	POINT lefttop = {rect.left,rect.top};
	POINT rightbottom = {rect.right,rect.bottom};
	ScreenToClient(m_targetViewWnd,&lefttop);
	ScreenToClient(m_targetViewWnd,&rightbottom);
	if(m_ProcessType == WORD_TYPE)
	{
		lefttop.y -= 3;
		rightbottom.y -= 3;
	}
	else
	{
		lefttop.y += 25;
		rightbottom.y += 25;
	}
	SetRect(&rect,lefttop.x,lefttop.y,rightbottom.x,rightbottom.y);
}

HRGN CManageThread::GetAboveRGN(HWND hPrev)
{
	HWND hPrevPrev = GetWindow(hPrev,GW_HWNDPREV);
	RECT hPrevRect;
	GetWindowRect(hPrev,&hPrevRect);
	ChangeRectCoordinate(hPrevRect);
	HRGN thisWnd = CreateRectRgnIndirect(&hPrevRect);
	if(hPrevPrev)
	{
		CombineRgn(thisWnd,thisWnd,GetAboveRGN(hPrevPrev),RGN_OR);
	}
	return thisWnd;
}
HRGN CManageThread::UpdateRedrawRGN(HWND view)
{
	//for word _wwg
	wchar_t className[MAX_PATH + 1] = {0};
	GetClassName(view,className,MAX_PATH);
	if(_wcsicmp(className,L"_WwG")==0)	view = GetParent(view);
	m_targetViewWnd = view;

	HWND hPrev = GetWindow(view,GW_HWNDPREV);
	RECT viewRect;
	GetWindowRect(view,&viewRect);
	ChangeRectCoordinate(viewRect);
	HRGN thisWnd = CreateRectRgnIndirect(&viewRect);;
	if(hPrev)	CombineRgn(thisWnd,thisWnd,GetAboveRGN(hPrev),RGN_DIFF);
	return thisWnd;
}


bool CManageThread::UpdateFirstChildWnd(list<pair<HWND,HRGN>>& lis)
{
	bool bRet = false;
	wchar_t className[MAX_PATH] = {0};
	list<pair<HWND,HRGN>>::iterator it = lis.begin();
	for( ; it != lis.end() ; ++it)
	{	
		HWND brother = it->first;
		if(IsWindow(brother))
		{			
			GetClassName(brother,className,MAX_PATH);
			if(_wcsicmp(className,L"_WwG")==0)
			{
				brother = GetParent(brother);
				if(!brother) continue;
			}
			brother = GetWindow(brother,GW_HWNDFIRST);
			GetClassName(brother,className,MAX_PATH);
			if(_wcsicmp(className,L"EXCEL6")==0)
				brother = GetWindow(brother,GW_HWNDNEXT);
			if(brother) 
			{
				if(	m_FirstChildWnd != brother)
				{
					bRet = true;
					m_FirstChildWnd = brother;
				}
				return bRet;
			}
		}
	}
	return bRet;
}
bool CManageThread::DealWithMutilForeOverlap(HWND hfore)
{
	bool bRet = false;
	bool flag = false;
	COvelayWndMgr& theMgr = COvelayWndMgr::GetInstance();
	COverlayWnd* pOver = NULL;
	map<HWND,list<pair<HWND,HRGN>>>::iterator itt = m_mapForeView.begin();
	for( ; itt != m_mapForeView.end() ; ++itt)
	{
		if(itt->first == hfore)	flag = true;
		else flag = false;
		if(!IsWindow(itt->first))
		{
			m_mapForeView.erase(itt);
			break;
		}
		list<pair<HWND,HRGN>>::iterator itlist = itt->second.begin();
		for( ; itlist != itt->second.end(); ++itlist)
		{
			if(!flag)
			{
				pOver = theMgr.GetOverLayInfoFromView(itlist->first);
				if(pOver)
				{
					DWORD dwStyleEx = static_cast<DWORD>(GetWindowLongPtrW(pOver->GethOverlayWnd(),GWL_EXSTYLE));
					DWORD dwResult = dwStyleEx & WS_EX_TOPMOST;
					if(dwResult)
					{
						SetWindowPos(pOver->GethOverlayWnd(),HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
						bRet = true;
					}
				}
			}
		}
	}
	return bRet;
}
bool CManageThread::IsPPT2010(void)
{
	bool bRet = false;
	if(m_ProcessType == PPT_TYPE)
	{
		CAuxiliary theAu;
		if(theAu.GetAppVer()==2010)
			bRet =  true;
	}
	return bRet;
}

void CManageThread::DetectAndAddToCache(void)
{
	vector<HWND> vechRView;
	EnterCriticalSection(&m_secNeedAddView);
	vechRView = m_vecNeedAddView;
	m_vecNeedAddView.clear();
	LeaveCriticalSection(&m_secNeedAddView);

	if(!vechRView.empty())
	{
		vector<HWND>::iterator itVec = vechRView.begin();
		m_FirstChildWnd = vechRView[vechRView.size()-1];
		for( ; itVec != vechRView.end() ; ++ itVec)
		{
			HWND hForeGroundWnd = GetAncestor(*itVec,GA_ROOT);
			HRGN lin = CreateRectRgn(0,0,1,1);
			pair<HWND,HRGN> oneMember(*itVec,lin);
			map<HWND,list<pair<HWND,HRGN>>>::iterator it = m_mapForeView.find(hForeGroundWnd);
			if(it == m_mapForeView.end())
			{
				list<pair<HWND,HRGN>>  viewlist;
				viewlist.push_front(oneMember);
				m_mapForeView[hForeGroundWnd] = viewlist;
			}
			else
			{
				it->second.push_front(oneMember);
			}
		}
	}
}
void CManageThread::WorkingFunc(void)
{
	COvelayWndMgr& theMgr = COvelayWndMgr::GetInstance();
	COverlayWnd* pOver = NULL;
	HRGN redrawRgn = CreateRectRgn(0,0,1,1);
	wchar_t className[MAX_PATH+1] = {0};
	do 
	{
		DetectAndAddToCache();

		HWND hForeGroundWnd = GetForegroundWindow();
		
 		GetClassName(hForeGroundWnd,className,MAX_PATH);
 		if(_wcsicmp(className,L"WaterMarkWindows")==0)
 		{
			if(m_TimeOut++ > 0)
			{
				m_TimeOut = 0;
 				pOver = theMgr.GetOverLayInfoFromOverlayWnd(hForeGroundWnd);
 				if(pOver)
					PostMessage(hForeGroundWnd,WM_SETFORGROUND,NULL,NULL);

			}
			Sleep(200);
 			continue;
 		}

		map<HWND,list<pair<HWND,HRGN>>>::iterator it = m_mapForeView.find(hForeGroundWnd);
		if(IsPPT2010())
		{
			if(_wcsicmp(className,L"PPTFrameClass")==0)
			{
				if(DealWithMutilForeOverlap(hForeGroundWnd))
				{
					SetWindowPos(hForeGroundWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				}
			}
		}
		else if(m_ProcessType == ADOBE_TYPE)
		{
			if(_wcsicmp(className,L"AcrobatSDIWindow")==0)
			{
				if(DealWithMutilForeOverlap(hForeGroundWnd))
				{
					SetWindowPos(hForeGroundWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				}
			}
		}
		else if(it != m_mapForeView.end())
		{
			RECT currentFirstChildWndRect;
			CopyRect(&currentFirstChildWndRect,&m_FirstChildWndRect);
			//here update static member m_FirstChildWnd
			if(IsWindow(m_FirstChildWnd))
			{
				GetWindowRect(m_FirstChildWnd,&currentFirstChildWndRect);
			}
			else
			{
				list<pair<HWND,HRGN>>::iterator itlist = it->second.begin();
				for(; itlist != it->second.end() ; ++itlist)
				{
					if(itlist->first == m_FirstChildWnd)
					{
						DeleteObject(itlist->second);
						it->second.erase(itlist);
						break;
					}
				}
			}

			bool bUpdated = UpdateFirstChildWnd(it->second);
			if( !EqualRect(&m_FirstChildWndRect,&currentFirstChildWndRect) || bUpdated )
			{
				//invalidate region computer
				list<pair<HWND,HRGN>>::iterator listit = it->second.begin();
				for( ; listit != it->second.end() ; ++listit )
				{
					HWND hViewWnd = listit->first;
					//if(move first child affect this view)	bool IsNeedUpadteRgn(HWND view,RECT old,RECT now)
					if(CheckIfNeedComputeRgn(hViewWnd,currentFirstChildWndRect))
					{
						redrawRgn = listit->second;
						listit->second = UpdateRedrawRGN(hViewWnd);
						//here can optimize setwindowRgn and invalidRgn
						pOver = theMgr.GetOverLayInfoFromView(hViewWnd);
						if(pOver)
						{
							if(hViewWnd == m_FirstChildWnd || IsChild(m_FirstChildWnd,hViewWnd))
							{
								SetWindowRgn(pOver->GethOverlayWnd(),listit->second,true);
							}
							else
							{
								SetWindowRgn(pOver->GethOverlayWnd(),listit->second,false);
								CombineRgn(redrawRgn,redrawRgn,listit->second,RGN_XOR);
								InvalidateRgn(pOver->GethOverlayWnd(),redrawRgn,true);
							}
						}
					}
				}
				CopyRect(&m_FirstChildWndRect,&currentFirstChildWndRect);
			}
		}
		DWORD dwTime = WaitForSingleObject(m_hEvent,400);
		if(dwTime == WAIT_OBJECT_0 || dwTime == WAIT_FAILED)	break;
	} while (true);
	DeleteObject(redrawRgn);
	_endthread();
}
void CManageThread::CreateManageThreadFun(LPVOID lParam)
{
	CManageThread* pMana = static_cast<CManageThread*>(lParam);
	pMana->WorkingFunc();
}
bool CManageThread::CheckIfSkip(HWND hView)
{
	bool bRet = false;
	CAuxiliary theAu;
	if(theAu.GetOpenType(GetForegroundWindow())==IE_OPNE_TYPE)
	{
		bRet = true;
	}
	else
	{
		wchar_t className[MAX_PATH+1] = {0};
		GetClassName(hView,className,MAX_PATH);
		if(_wcsicmp(className,L"screenClass")==0)
		{
			bRet = true;
		}
	}
	return bRet;
}
void CManageThread::Init(void)
{
	EnterCriticalSection(&m_secInit);
	if(!m_bInit)
	{
		m_hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);		
		m_bInit = true;		
		m_ProcessType = CAuxiliary().GetProgressType();
		m_hThread =	(HANDLE)_beginthread(CreateManageThreadFun,NULL,this);
	}
	LeaveCriticalSection(&m_secInit);
}
void CManageThread::StartManageThread(HWND hView)
{
	Init();

	if(CheckIfSkip(hView))	return;

	EnterCriticalSection(&m_secNeedAddView);
	m_vecNeedAddView.push_back(hView);
	LeaveCriticalSection(&m_secNeedAddView);
}