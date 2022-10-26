#include "StdAfx.h"
#include <list>
#include <string>
#include <algorithm>
using namespace std;
#include "FTPEEval.h"
#include "MapperMgr.h"
#include "Utilities.h"


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

void CMapperMgr::PushFileInfoToList(const string& path, const string& content)
{
	::EnterCriticalSection(&CcriticalMngr::s_csPathContentCache);
	PATH_CONTENT_LIST::iterator it = m_listPathContent.begin();
	DWORD dCurrentTime = GetTickCount() ; 
	int nCount = 0;
	for(; it != m_listPathContent.end(); )
	{
		if(TStringicmp(it->first.first,path))
		{
			//	for #10105
			if (IsProcess(L"explorer.exe"))
			{
				m_listPathContent.erase(it);
				m_listPathContent.push_back(make_pair(make_pair(path, content),GetTickCount()));
				::LeaveCriticalSection(&CcriticalMngr::s_csPathContentCache);
				return;
			}
			else
			{
			it->second =dCurrentTime ;
			break;
		}
		}
		if( (dCurrentTime - it->second) >MAX_TIME_OUT	 )
		{
			DPA(("Erase the file name:[%s]",it->first.first.c_str())) ;	
			m_listPathContent.erase(it);
			it = m_listPathContent.begin();

			for(int i = 0; i < nCount; i++)
			{
				it++;
			}
			continue ;
		}  
		nCount++;
		it++;
	}
	if(it != m_listPathContent.end())
	{
		it->first.second = content;
	}
	else
	{
		m_listPathContent.push_back(make_pair(make_pair(path, content),GetTickCount()));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csPathContentCache);
}

string CMapperMgr::PopFileInfoFromList(const string& content)
{
	::EnterCriticalSection(&CcriticalMngr::s_csPathContentCache);
	string sPath;

	if(IsProcess(L"explorer.exe"))
	{
		PATH_CONTENT_LIST::reverse_iterator ritor = m_listPathContent.rbegin() ;
		for(; ritor != m_listPathContent.rend(); ++ritor)
		{
			if(content.find(ritor->first.second) == 0 || ritor->first.second.find(content) == 0)
			{
				sPath = ritor->first.first;
				m_listPathContent.erase(--ritor.base());
				break;
			}
		}
	}
	else
	{
	PATH_CONTENT_LIST::iterator it = m_listPathContent.begin();
	for(; it != m_listPathContent.end(); ++it)
	{
		if(content.find(it->first.second) == 0 || it->first.second.find(content) == 0)
		{
			// When we find it, we remove it
			sPath = it->first.first;
			m_listPathContent.erase(it);
				break;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csPathContentCache);
	return sPath;
}

void CMapperMgr::PushHandleName(HANDLE hFile, const string& sName)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	HANDLE_NAME_LIST::iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if(it->first == hFile)
		{
			break;
		}
	}
	if(it != m_listHandleName.end())
	{
		it->second = sName;
	}
	else
	{
		m_listHandleName.push_back(make_pair(hFile, sName));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
}

string CMapperMgr::PopHandleName(HANDLE hFile)
{
	::EnterCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	string sName;
	HANDLE_NAME_LIST::iterator it = m_listHandleName.begin();
	for(; it != m_listHandleName.end(); ++it)
	{
		if(it->first == hFile)
		{
			sName = it->second;
			// When we find it, we remove it
			m_listHandleName.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csHandleNameCache);
	return sName;
}

void CMapperMgr::PushSocketBufEval(SOCKET s, const string& sBuf, const FTP_EVAL_INFO & evalInfo)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	SOCKET_BUF_EVAL_LIST::iterator it = m_listSocketBufEval.begin();
	for(; it != m_listSocketBufEval.end(); ++it)
	{
		if(it->first.first == s)
		{
			break;
		}
	}
	if(it != m_listSocketBufEval.end())
	{
		it->first.second = sBuf;
		it->second = evalInfo;
	}
	else
	{
		m_listSocketBufEval.push_back(make_pair(make_pair(s, sBuf), evalInfo));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
}

FTP_EVAL_INFO CMapperMgr::PopSocketBufEval4Explorer(const string& sBuf)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	FTP_EVAL_INFO evalInfo;
	SOCKET_BUF_EVAL_LIST::iterator it = m_listSocketBufEval.begin();
	for(; it != m_listSocketBufEval.end(); ++it)
	{
		if(it->first.second.find(sBuf) == 0 || sBuf.find(it->first.second) == 0)
		{
			evalInfo = it->second;
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	return evalInfo;
}
FTP_EVAL_INFO CMapperMgr::PopSocketBufEval4RecvEval(const SOCKET &s)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	FTP_EVAL_INFO evalInfo;
	
	if(m_listSocketBufEval.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
		return evalInfo;		
	}
	
	SOCKET_BUF_EVAL_LIST::iterator it = m_listSocketBufEval.begin();
	for(; it != m_listSocketBufEval.end(); ++it)
	{
		if(it->first.first == s)
		{
			evalInfo = it->second;
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	return evalInfo;
}
BOOL CMapperMgr::RemoveSocketBufEval4RecvEval(const SOCKET& s)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	BOOL bRet = FALSE ;

	if(m_listSocketBufEval.empty())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
		return bRet;		
	}

	SOCKET_BUF_EVAL_LIST::iterator it = m_listSocketBufEval.begin();
	for(; it != m_listSocketBufEval.end(); ++it)
	{
		if(it->first.first == s)
		{
			m_listSocketBufEval.erase(it);
			bRet = TRUE ;
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	return bRet ;

}
FTP_EVAL_INFO CMapperMgr::PopSocketBufEval(const string& sBuf)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	FTP_EVAL_INFO evalInfo;
	SOCKET_BUF_EVAL_LIST::iterator it = m_listSocketBufEval.begin();
	for(; it != m_listSocketBufEval.end(); ++it)
	{
		if(it->first.second.find(sBuf) == 0 || sBuf.find(it->first.second) == 0)
		{
			evalInfo = it->second;
			// When we find it, we remove it
			m_listSocketBufEval.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csSocketBufEvalCache);
	return evalInfo;
}

void CMapperMgr::PushThreadContent(DWORD dwThread, const string& sContent)
{
	::EnterCriticalSection(&CcriticalMngr::s_csThreadContent);
	THREAD_CONTENT_LIST::iterator it = m_listThreadContent.begin();
	for(; it != m_listThreadContent.end(); ++it)
	{
		if(it->first == dwThread)
		{
			break;
		}
	}
	if(it != m_listThreadContent.end())
	{
		it->second = sContent;
	}
	else
	{
		m_listThreadContent.push_back(make_pair(dwThread, sContent));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csThreadContent);
}

string CMapperMgr::PopThreadContent(DWORD dwThread)
{
	::EnterCriticalSection(&CcriticalMngr::s_csThreadContent);
	string sContent;
	THREAD_CONTENT_LIST::iterator it = m_listThreadContent.begin();
	for(; it != m_listThreadContent.end(); ++it)
	{
		if(it->first == dwThread)
		{
			sContent = it->second;
			// When we find it, we remove it
			m_listThreadContent.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csThreadContent);
	return sContent;
}

void CMapperMgr::PushThreadSocket(DWORD dwThread, SOCKET s)
{
	::EnterCriticalSection(&CcriticalMngr::s_csThreadSocket);
	THREAD_SOCKET_LIST::iterator it = m_listThreadSocket.begin();
	for(; it != m_listThreadSocket.end(); ++it)
	{
		if(it->first == dwThread)
		{
			break;
		}
	}
	if(it != m_listThreadSocket.end())
	{
		it->second = s;
	}
	else
	{
		m_listThreadSocket.push_back(make_pair(dwThread, s));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csThreadSocket);
}

SOCKET CMapperMgr::PopThreadSocket(DWORD dwThread)
{
	::EnterCriticalSection(&CcriticalMngr::s_csThreadSocket);
	SOCKET s = INVALID_SOCKET;
	THREAD_SOCKET_LIST::iterator it = m_listThreadSocket.begin();
	for(; it != m_listThreadSocket.end(); ++it)
	{
		if(it->first == dwThread)
		{
			s = it->second;
			// When we find it, we remove it
			m_listThreadSocket.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csThreadSocket);
	return s;
}

void CMapperMgr::AddConhandleEval(DWORD dwHandle, const FTP_EVAL_INFO & evalInfo)
{
	::EnterCriticalSection(&CcriticalMngr::s_csConhandleEval);
	CONHANDLE_EVAL_LIST::iterator it = m_listConhandleEval.begin();
	for(; it != m_listConhandleEval.end(); ++it)
	{
		if(it->first == dwHandle)
		{
			break;
		}
	}
	if(it != m_listConhandleEval.end())
	{
		it->second = evalInfo;
	}
	else
	{
		m_listConhandleEval.push_back(make_pair(dwHandle, evalInfo));
	}	
	::LeaveCriticalSection(&CcriticalMngr::s_csConhandleEval);
}

FTP_EVAL_INFO CMapperMgr::GetEvalByConhandle(DWORD dwHandle)
{
	::EnterCriticalSection(&CcriticalMngr::s_csConhandleEval);
	FTP_EVAL_INFO evalInfo;
	CONHANDLE_EVAL_LIST::iterator it = m_listConhandleEval.begin();
	for(; it != m_listConhandleEval.end(); ++it)
	{
		if(it->first == dwHandle)
		{
			evalInfo = it->second;
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csConhandleEval);
	return evalInfo;
}

void CMapperMgr::RemoveConhandleEval(DWORD dwHandle)
{
	::EnterCriticalSection(&CcriticalMngr::s_csConhandleEval);
	CONHANDLE_EVAL_LIST::iterator it = m_listConhandleEval.begin();
	for(; it != m_listConhandleEval.end(); ++it)
	{
		if(it->first == dwHandle)
		{
			// When we find it, we remove it
			m_listConhandleEval.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csConhandleEval);
}

void CMapperMgr::PushDelHandleName(HANDLE hFile, const string& sName)
{
	::EnterCriticalSection(&CcriticalMngr::s_csDelHandleNameCache);
	HANDLE_NAME_LIST::iterator it = m_listDelHandleName.begin();
	for(; it != m_listDelHandleName.end(); ++it)
	{
		if(it->first == hFile)
		{
			break;
		}
	}
	if(it != m_listDelHandleName.end())
	{
		it->second = sName;
	}
	else
	{
		m_listDelHandleName.push_back(make_pair(hFile, sName));
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csDelHandleNameCache);
}

string CMapperMgr::PopDelHandleName(HANDLE hFile)
{
	::EnterCriticalSection(&CcriticalMngr::s_csDelHandleNameCache);
	string sName;
	HANDLE_NAME_LIST::iterator it = m_listDelHandleName.begin();
	for(; it != m_listDelHandleName.end(); ++it)
	{
		if(it->first == hFile)
		{
			sName = it->second;
			// When we find it, we remove it
			m_listDelHandleName.erase(it);
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csDelHandleNameCache);
	return sName;
}