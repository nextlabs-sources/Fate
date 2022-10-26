#include "stdafx.h"
#include "httppreproc.h"
#include "criticalMngr.h"


static std::string g_szIgnoreIP[] = {"127.0.0.1", "localhost"};

CHttpPreProc::CHttpPreProc()
{
}

CHttpPreProc::~CHttpPreProc()
{
}


BOOL CHttpPreProc::PreProcessMsg(smartHttpMsg& httpMsg, BOOL& allow)
{
	//Check if the current URL is ignored.
	string rmt;
	for(unsigned i = 0; i < _countof(g_szIgnoreIP); i++)
	{
		httpMsg->GetRmt(rmt);
		if(rmt.find(g_szIgnoreIP[i]) == 0)
		{
			allow = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CHttpPreProc::PreCheck(SOCKET s)
{
	wstring strIP;
	strIP = GetPeerIP(s);
	if(strIP.length() > 0)
	{
		for(unsigned i = 0; i < _countof(g_szIgnoreIP); i++)
		{
			if( strIP.find( wstring( g_szIgnoreIP[i].begin(), g_szIgnoreIP[i].end() ) ) == 0 )
			{
		//		g_log.Log(CELOG_DEBUG, L"HTTPE::This IP was ignored. %s \r\n", strIP.c_str());
				return FALSE;
			}
		}
	}
	return TRUE;
}