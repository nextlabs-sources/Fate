#include "StdAfx.h"
#include "FileOperationHooking.h"
#include "NxtReceiver.h"
#include "eframework/auto_disable/auto_disable.hpp"
#include "celog.h"

#pragma warning(push)
#pragma warning(disable: 4244 4819)
#include <boost/thread.hpp>  
#include <boost/function.hpp> 
#include "madCHook_helper.h"
#pragma warning(pop)

typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 

boost::shared_mutex g_mutex; 
extern CELog cbPepLog;

std::map<LPVOID, LPVOID> CFileOperationHooking::m_mapHooks;

extern nextlabs::recursion_control hook_control;           // control recursion for hooks


CFileOperationHooking::CFileOperationHooking(void)
{
}

CFileOperationHooking::~CFileOperationHooking(void)
{
}

CFileOperationHooking* CFileOperationHooking::GetInstance()
{
	static CFileOperationHooking inst;
	return &inst;
}

bool CFileOperationHooking::Hook(IFileOperation *pObject)
{
	if (!pObject)
	{
		return false;
	}

	boost_unique_lock lockWriter(g_mutex);  

	LPVOID* pVTable = (*(LPVOID**)pObject);//the v table of the object
	

	//try to hook PerformOperations()
	LPVOID pPerformOperations = pVTable[21];
	if(m_mapHooks.find(pPerformOperations) == m_mapHooks.end())//
	{
		LPVOID next_PerformOperations = NULL;
		if(HookCode((LPVOID)pPerformOperations,(PVOID)CFileOperationHooking::MyPerformOperations,(LPVOID*)&next_PerformOperations) && next_PerformOperations != NULL)
		{
			m_mapHooks[pPerformOperations] = next_PerformOperations;
		}
	}

	return true;
}


HRESULT CFileOperationHooking::MyPerformOperations(IFileOperation * This)
{
	f_PerformOperations next_func = NULL;
	{
		boost_share_lock lockReader(g_mutex);

		LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
		LPVOID pPerformOperations = pVTable[21];

		std::map<LPVOID, LPVOID>::iterator iter = m_mapHooks.find(pPerformOperations);

		if(iter == m_mapHooks.end())
		{
			cbPepLog.Log(CELOG_DEBUG,L"exception in MyPerformOperations.");
			//OutputDebugStringW(L"exception in MyPerformOperations.");
			return S_FALSE;
		}

		next_func = (f_PerformOperations)(*iter).second;
	}

	if ( hook_control.is_disabled() == true)
	{
		return next_func(This);
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);


	CNxtReceiver* receiver = new CNxtReceiver();
	DWORD cookie = 0;
	This->Advise(receiver, &cookie);

	HRESULT hr = next_func(This);

	This->Unadvise(cookie);
	delete receiver;
	
	return hr;

}

