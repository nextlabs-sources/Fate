#include "stdafx.h"
#include "FtpSocket.h"
#include "ParserResult.h"
#include "FtpDataConn.h"
#include "FtpCtrlConn.h"
#include "FtpConnMgr.h"
#include <algorithm>
#include <functional>
#include <cassert>
using namespace std;



CFtpConnMgr::CFtpConnMgr(void)
{
	
}

CFtpConnMgr::~CFtpConnMgr(void)
{

}

CFtpConnMgr& CFtpConnMgr::Instance()
{
	static CFtpConnMgr ins;
	
	return ins;
}

void CFtpConnMgr::AddConn(SOCKET fd, bool bWouldBlock, const struct sockaddr* peername)
{
	::EnterCriticalSection(&CcriticalMngr::s_csFtpConn);
	list<CFtpCtrlConn*>::iterator it;
	for(it = m_listCtrlConn.begin(); it != m_listCtrlConn.end(); ++it)
	{
		assert(fd != (*it)->GetSocket());
		if((*it)->GetDataConn() != NULL)
		{
			assert(fd != (*it)->GetDataConn()->GetSocket());
		}
	}
	
	CFtpSocket tmpSC(fd);
	DPA(("is %d a data connection\n", tmpSC.GetRemotePort()));
	for(it = m_listCtrlConn.begin(); it != m_listCtrlConn.end(); ++it)
	{
		// This is a pasv data connection
		if((*it)->GetConnMode() == FCM_PASV)
		{
			string sPeerIP = tmpSC.GetPeerIP();
			int nRemotePort = tmpSC.GetRemotePort();
			if( (sPeerIP == "" || nRemotePort == -1) && bWouldBlock && peername != NULL)
			{
				sPeerIP = CFtpSocket::AddressToString(peername, sizeof(sockaddr), false);
				struct sockaddr_in* addr_v4 = (sockaddr_in*)peername;
				nRemotePort = ntohs(addr_v4->sin_port);
			}
			if( (*it)->GetDataIP() == sPeerIP && (*it)->GetDataPort() == nRemotePort )
			{
				CFtpDataConn* pDataConn = new CFtpDataConn(fd);
				CFtpDataConn* pOldDataConn = (*it)->SetDataConn(pDataConn);
				DPA(("%d is a data connection\n", nRemotePort));
				// No need to check against NULL
				delete pOldDataConn;
				break;
			}
		}

		// This is a port data connection
		if((*it)->GetConnMode() == FCM_PORT && (*it)->GetDataIP() == tmpSC.GetLocalIP() && (*it)->GetDataPort() == tmpSC.GetLocalPort())
		{
			CFtpDataConn* pDataConn = new CFtpDataConn(fd);
			CFtpDataConn* pOldDataConn = (*it)->SetDataConn(pDataConn);
			// No need to check against NULL
			delete pOldDataConn;
			break;
		}
	}

	// Assume this is a control connection
	if(it == m_listCtrlConn.end())
	{
		CFtpCtrlConn* pCtrlConn = new CFtpCtrlConn(fd);
		m_listCtrlConn.push_back(pCtrlConn);
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
}

void CFtpConnMgr::DelConn(SOCKET fd)
{
	::EnterCriticalSection(&CcriticalMngr::s_csFtpConn);
	list<CFtpCtrlConn*>::iterator it;
	for(it = m_listCtrlConn.begin(); it != m_listCtrlConn.end(); ++it)
	{
		if(fd == (*it)->GetSocket())
		{
			CFtpDataConn* pOldDataConn = (*it)->SetDataConn(NULL);
			delete pOldDataConn;
			CFtpCtrlConn* pOldCtrlConn = (*it);
			m_listCtrlConn.erase(it);
			delete pOldCtrlConn;
			break;
		}
		if((*it)->GetDataConn() != NULL)
		{
			if(fd == (*it)->GetDataConn()->GetSocket())
			{
				CFtpDataConn* pOldDataConn = (*it)->SetDataConn(NULL);
				delete pOldDataConn;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
}

CFtpCtrlConn* CFtpConnMgr::FindCtrlConn(SOCKET fd) const
{
	list<CFtpCtrlConn*>::const_iterator it;
	bool bFound = false;
	::EnterCriticalSection(&CcriticalMngr::s_csFtpConn);
	if(m_listCtrlConn.empty())//fix bug9795. check if m_listCtrlConn is empty. this is very weird. bug9795 happens when FTPE tries to use " m_listCtrlConn.begin()" if this list is empty. kevin 2009-8-26
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
		return NULL;
	}
	for(it = m_listCtrlConn.begin(); it != m_listCtrlConn.end(); ++it)
	{
		if(fd == (*it)->GetSocket())
		{
			bFound = true;
			break;
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
	return bFound ? (*it) : NULL;
}

CFtpDataConn* CFtpConnMgr::FindDataConn(SOCKET fd) const
{
	list<CFtpCtrlConn*>::const_iterator it;
	bool bFound = false;
	::EnterCriticalSection(&CcriticalMngr::s_csFtpConn);
	if(m_listCtrlConn.empty())//fix bug9795. check if m_listCtrlConn is empty. this is very weird. bug9795 happens when FTPE tries to use " m_listCtrlConn.begin()" if this list is empty. kevin 2009-8-26
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
		return NULL;
	}
	for(it = m_listCtrlConn.begin(); it != m_listCtrlConn.end(); ++it)
	{
		if((*it)->GetDataConn() != NULL)
		{
			if(fd == (*it)->GetDataConn()->GetSocket())
			{
				bFound = true;
				break;
			}
		}
	}
	::LeaveCriticalSection(&CcriticalMngr::s_csFtpConn);
	return bFound ? (*it)->GetDataConn() : NULL;
}

ParserResult CFtpConnMgr::ParseSend(SOCKET fd, const string& sBuf)
{
	CFtpCtrlConn* pCtrlConn = FindCtrlConn(fd);
	if(pCtrlConn != NULL)
	{
		return pCtrlConn->ParseSend(sBuf);
	}
	CFtpDataConn* pDataConn = FindDataConn(fd);
	if(pDataConn != NULL)
	{
		return pDataConn->ParseSend(sBuf);
	}
	return PARSER_INPROCESS;
}

ParserResult CFtpConnMgr::ParseRecv(SOCKET fd, const string& sBuf)
{
	CFtpCtrlConn* pCtrlConn = FindCtrlConn(fd);
	if(pCtrlConn != NULL)
	{
		return pCtrlConn->ParseRecv(sBuf);
	}
	CFtpDataConn* pDataConn = FindDataConn(fd);
	if(pDataConn != NULL)
	{
		return pDataConn->ParseRecv(sBuf);
	}
	return PARSER_INPROCESS;
}

void CFtpConnMgr::SetFtpProtocolType(SOCKET fd, FtpProtocolType pt)
{
	CFtpCtrlConn* pCtrlConn = FindCtrlConn(fd);
	if(pCtrlConn != NULL)
	{
		pCtrlConn->SetFtpProtocolType(pt);
	}
}

FtpProtocolType CFtpConnMgr::GetFtpProtocolType(SOCKET fd) const
{
	CFtpCtrlConn* pCtrlConn = FindCtrlConn(fd);
	if(pCtrlConn != NULL)
	{
		return pCtrlConn->GetFtpProtocolType();
	}
	CFtpDataConn* pDataConn = FindDataConn(fd);
	if(pDataConn != NULL)
	{
		pCtrlConn = pDataConn->GetCtrlConn();
		assert(pCtrlConn != NULL);
		if(pCtrlConn)
		{
		return pCtrlConn->GetFtpProtocolType();
	}
	}
	return FPT_UNKNOWN;
}
