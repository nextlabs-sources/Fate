#include "StdAfx.h"
#include <list>
#include <string>
#include <algorithm>
using namespace std;
#include "MapperMgr.h"
#include "criticalMngr.h"




CMapperMgr::CMapperMgr(void)
{
}

CMapperMgr::~CMapperMgr(void)
{
}

CMapperMgr& CMapperMgr::Instance()
{
	static CMapperMgr ins;
	return ins;
}

/****************************************
function name: CheckExpiredItem
feature:
	This function will check the m_listHandleName,
Remove the item if it is expired.
****************************************/
void CMapperMgr::CheckExpiredItem()
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = m_listHandleName.begin();
	DWORD dCurTime = ::GetTickCount() ;
	for(; it != m_listHandleName.end(); )
	{
		DWORD dwTimeOut = MAX_TIME_OUT;
		if(IsProcess(L"iexplore.exe") && 6 == GetIEVersionNum() )
		{
			dwTimeOut = MAX_TIME_OUT_IE6;
		}
		if( (dCurTime - (*it)->dStartTime) > dwTimeOut	 )
		{
			DPA(("HPE::CheckCacheListTimeout, Erase the file name:[%s]\r\n",(*it)->strFile.c_str())) ;	
			it = m_listHandleName.erase(it);
			continue ;
		}  
		it++;
	}
	
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
}

void CMapperMgr::AddHandleName(HANDLE hFile, const string& sName)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = m_listHandleName.begin();
	DWORD dCurTime = ::GetTickCount() ;
	for(; it != m_listHandleName.end(); )
	{
		if((*it)->hFile == hFile)
		{
			(*it)->dStartTime = dCurTime ;
			(*it)->skCheckList.clear() ;
			break;
		}
		DWORD dwTimeOut = MAX_TIME_OUT;
		if(IsProcess(L"iexplore.exe") && 6 == GetIEVersionNum() )
		{
			dwTimeOut = MAX_TIME_OUT_IE6;
		}
			if(IsProcess(L"explorer.exe"))
			{
				dwTimeOut = MAX_TIME_OUT_EXPLORER;
			}
			//	check time out
		if( (dCurTime - (*it)->dStartTime) > dwTimeOut	 )
		{
			DPA(("Erase the file name:[%s]\r\n",(*it)->strFile.c_str())) ;	
			it = m_listHandleName.erase(it);
			continue ;
		}  
		it++;
	}
	if(it != m_listHandleName.end())
	{
		(*it)->strFile = sName;
	}
	else
	{
		YLIB::smart_ptr<HANDLE_NAME_TIME> hnt(new HANDLE_NAME_TIME);
		hnt->dStartTime	= GetTickCount() ;
		hnt->hFile =  hFile ;
		hnt->strFile = sName ; 
		m_listHandleName.push_back( hnt ) ;
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
}
string CMapperMgr::GetHandleName(HANDLE hFile)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	string sName;
	if(m_listHandleName.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
		return sName ;
	}
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if((*it)->hFile == hFile)
		{
			// When we find it, we remove it
			sName = (*it)->strFile;
			m_listHandleName.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	return sName;
}
void CMapperMgr::RemoveItemByFileName(const string &sName) 
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	if(m_listHandleName.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
		return ;
	}
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if((*it)->strFile == sName)
		{
			// When we find it, we remove it
			m_listHandleName.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
}
void CMapperMgr::RemoveHandleName(HANDLE hFile)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	if(m_listHandleName.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
		return  ;
	}

	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if((*it)->hFile == hFile)
		{
			// When we find it, we remove it
			m_listHandleName.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
}

BOOL CMapperMgr::GetAllOpenFiles(wstring& sk, std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>& listOpenFiles)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	if(m_listHandleName.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
		return TRUE  ;
	}
	std::list<YLIB::smart_ptr<HANDLE_NAME_TIME>>::const_iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if( CheckHasEvaluation(sk,  (*it)->skCheckList ) == TRUE )
		{
			//	modified in 2011/mar/21, removed && 6 == GetIEVersionNum() for bug 13831
			//	modified in 2011/mar/31, added "!IsProcess(L"rundll32.exe")" for bug 13844, rundll32 is used by chrome version 10, reason description:
			//	policy -- deny upload c:\kaka\d.txt
			//	rundll32.exe send some data that doesn't belong to d.txt, it is denied because d.txt is opened by rundll32.exe,
			//	rundll32.exe continue to send data, this time the data belongs to d.txt, so, we can't skip d.txt by below code, we must evaluate d.txt here
			//	end of modified in 2011/mar/31.
			if( !IsProcess(L"rundll32.exe") && !IsProcess(L"ftpte.exe") && !IsProcess(L"bpftpclient.exe") && !IsProcess(L"FlashFXP.exe") && !IsProcess(L"iexplore.exe") )//determine if the current process is IE6, fix bug9642, kevin 2009-8-10
			{
				//	skip this file
				continue;
			}
		}
		
			//	this file need to be evaluated
			(*it)->skCheckList.push_back( sk ) ;
			listOpenFiles.push_front(*it);
		
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	return TRUE;
}
BOOL CMapperMgr::CheckHasEvaluation( wstring& sk ,std::list<wstring>& skCheckList) const
{
	BOOL bRet = FALSE ;
	std::list<wstring>::iterator itor = skCheckList.begin() ;
	for(; itor != skCheckList.end(); ++itor)
	{
		if((*itor).compare(sk ) == 0 )
		{	 
			bRet = TRUE ;
			break;
		}
	}
	return bRet ;
}
