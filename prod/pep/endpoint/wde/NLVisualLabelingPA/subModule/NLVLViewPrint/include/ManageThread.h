#ifndef _MANAGETHREAD_H_
#define _MANAGETHREAD_H_
#include "stdafx.h"
#include <list>
#include <map>
#include <vector>
#include "OvelayWndMgr.h"
#include "Auxiliary.h"
using namespace std;

class CManageThread
{
public:
	static CManageThread& GetInstance(void);
	void StartManageThread(HWND hView);
private:
	CManageThread();
	~CManageThread();
	void Init(void);
	void DetectAndAddToCache(void);
	bool DealWithMutilForeOverlap(HWND hfore);
	bool UpdateFirstChildWnd(list<pair<HWND,HRGN>>& lis);
	void ChangeRectCoordinate(RECT& rect);
	HRGN GetAboveRGN(HWND hPrev);
	HRGN UpdateRedrawRGN(HWND view);
	bool CheckIfNeedComputeRgn(HWND view,RECT currentRect);
	void WorkingFunc(void);
	static void CreateManageThreadFun(LPVOID lParam);
	bool CheckIfSkip(HWND hView);
	bool IsPPT2010(void);
	void ExistThread();
private:
	map<HWND,list<pair<HWND,HRGN>>> m_mapForeView;

	vector<HWND>		m_vecNeedAddView;
	CRITICAL_SECTION	m_secNeedAddView;
	HWND				m_FirstChildWnd;
	RECT				m_FirstChildWndRect;
	HWND				m_targetViewWnd;
	emProcessType		m_ProcessType;
	bool				m_bInit;
	CRITICAL_SECTION	m_secInit;
	HANDLE				m_hEvent;
	HANDLE				m_hThread;
	SHORT				m_TimeOut;
};
#endif
