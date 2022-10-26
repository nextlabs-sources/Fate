// FTPE.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "FTPE.h"
#include "FilterRes.h"

#include "FtpSocket.h"
#include "ParserResult.h"
#include "FtpDataConn.h"
#include "FtpCtrlConn.h"
#include "FtpConnMgr.h"
#include "Utilities.h"
#include "MapperMgr.h"
#include "FTPEEval.h"
#include "FtpSocket.h"
#include "eframework/auto_disable/auto_disable.hpp"
#include "timeout_list.hpp"

extern nextlabs::recursion_control hook_control;
extern nextlabs::cesdk_loader cesdkLoader;

CTimeoutList g_listOverlappedBuf;
static std::wstring g_szIgnoreIP[] = {L"127.0.0.1", L"localhost"};
bool IsIgnoredIP(LPCWSTR lpIP)
{
	if(!lpIP)
		return false;

	for(int i = 0; i < _countof(g_szIgnoreIP); i++)
	{
		if(_wcsicmp(lpIP, g_szIgnoreIP[i].c_str()) == 0)
		{
		//	DP((L"FTPE::IsIgnoredIP, This IP was ignored by FTPE. %s\r\n", lpIP));
			return true;
		}
	}
	return false;
}

/*******************************************************
function name: IsIgnoredDestAddress
feature: 
	check if the destination address associated with
the current SOCKET is ignored.          
********************************************************/
bool IsIgnoredDestAddress(SOCKET s)
{
	struct sockaddr  name ;
	INT iLen = sizeof(sockaddr)  ;
	if(getpeername( s, &name, &iLen )  == 0 )
	{
		string sPeerIP = CFtpSocket::AddressToString(&name, sizeof(sockaddr), false);
		std::wstring strDestIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(strDestIP.c_str()))
		{
			return true;
		}
	}
	return false;
}

bool IsIgnoredDestAddress(const struct sockaddr* name)
{
	if(!name)
	{
		return true;
	}

	string sPeerIP = CFtpSocket::AddressToString(name, sizeof(sockaddr), false);
	struct sockaddr_in* addr_v4 = (sockaddr_in*)name;
	int nRemotePort = ntohs(addr_v4->sin_port);
	if(sPeerIP != "" && nRemotePort > 0)
	{
		std::wstring strIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(strIP.c_str()))
		{
			return true;
		}
	}
	return false;
}

/*
These two interface is for the PPC plug-in
*/
//------------------------------------------------------------------------

int  WINAPI	 InstallSPI() 
{
	int iRet = 0 ;
	CProviderInstall provider ;
	wchar_t szFileName[MAX_PATH] = {0} ;
	if( CFilterRes::GetCurrentMudleFileName( szFileName ) !=0 )
	{
		iRet = (INT)provider.InstallProvider(szFileName,0) ;
	}
	return iRet ;
}
int  WINAPI	 UninstallSPI()	 
{
	
	int iRet = 0 ;
	CProviderInstall provider ;
	iRet = (INT)provider.UnInstallFilter() ;
	return iRet ;
}


static WSPPROC_TABLE nextproctable;

SOCKET WSPAPI my_WSPAccept(SOCKET s, struct sockaddr* addr, LPINT addrlen, LPCONDITIONPROC lpfnCondition, DWORD dwCallbackData, LPINT lpErrno)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
	}

	if(hook_control.is_disabled())
	{
		return nextproctable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	SOCKET sAccepted = nextproctable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
	if (sAccepted != INVALID_SOCKET)
	{
		CFtpConnMgr& ins = CFtpConnMgr::Instance();
		ins.AddConn(sAccepted, false, NULL);
	}
	return sAccepted;
}

SOCKET WSPAPI WSPAccept(SOCKET s, struct sockaddr* addr, LPINT addrlen, LPCONDITIONPROC lpfnCondition, DWORD dwCallbackData, LPINT lpErrno)
{
	__try
	{
		return my_WSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
}
int WSPAPI WSPCloseSocket(SOCKET s, LPINT lpErrno)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPCloseSocket(s, lpErrno);
	}

	if(hook_control.is_disabled())
	{
		return nextproctable.lpWSPCloseSocket(s, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	int iRet = nextproctable.lpWSPCloseSocket(s, lpErrno);
	if(iRet != SOCKET_ERROR)
	{
		CFtpConnMgr& ins = CFtpConnMgr::Instance();
		ins.DelConn(s);
	}
	return iRet;
}

int WSPAPI my_WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	if(hook_control.is_disabled())
	{
		return nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(IsIgnoredDestAddress(s))
	{
		return nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	if(lpBuffers != NULL)
	{
		CFtpConnMgr& ftpMgrIns = CFtpConnMgr::Instance();
		if(ftpMgrIns.GetFtpProtocolType(s) == FPT_REGULAR)
		{
			string sBuf;
			for(DWORD dwCnt = 0; dwCnt < dwBufferCount; ++dwCnt)
			{
				if(lpBuffers[dwCnt].buf != NULL && lpBuffers[dwCnt].len > 0)
				{
					sBuf.append(lpBuffers[dwCnt].buf, lpBuffers[dwCnt].len);
				}
			}
			if(sBuf.length() > 0)
			{
				ParserResult pr = ftpMgrIns.ParseSend(s, sBuf);
				if(pr == PARSER_DENY)
				{
					if(IsProcess(L"bpftpclient.exe")||IsProcess(L"FlashFXP.exe"))
					{
						if(lpNumberOfBytesSent != NULL)
						{
						  *lpNumberOfBytesSent = (DWORD)sBuf.length();
						}
						return 0;
					}
					return SOCKET_ERROR;
				}
			}
		}
		else if(ftpMgrIns.GetFtpProtocolType(s) == FTP_FTPS_IMPLICIT)
		{
			CMapperMgr& mapperIns = CMapperMgr::Instance();
			DWORD dwThreadID = ::GetCurrentThreadId();
			string sBuf = mapperIns.PopThreadContent(dwThreadID);
			if(sBuf.length() > 0)
			{
				ParserResult pr = ftpMgrIns.ParseSend(s, sBuf);
				if(pr == PARSER_DENY)
				{
					if(IsProcess(L"bpftpclient.exe"))
					{
						if(lpNumberOfBytesSent != NULL)
						{
						  *lpNumberOfBytesSent = (DWORD)sBuf.length();
						}
						return 0;
					}
					return SOCKET_ERROR;
				}
			}
		}
	}
	return nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	__try
	{
		return my_WSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
}
int WSPAPI my_WSPRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}
	
	if(lpFlags != NULL && *lpFlags == MSG_PEEK)
	{
		return nextproctable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	int iRet = nextproctable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

	if(hook_control.is_disabled())
	{
		return iRet;
	}
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(IsIgnoredDestAddress(s))
	{
		return iRet;
	}

	if(lpErrno && *lpErrno == WSA_IO_PENDING && lpOverlapped)//Chrome uses "overlapped" mode of SOCKET, 
	{
		wchar_t szKey[100] = {0};
		_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%p", s, lpOverlapped);

		LPWSABUF lpTemBuf = new WSABUF[dwBufferCount];
		for( DWORD i = 0; i < dwBufferCount; i++)
		{
			lpTemBuf[i].buf = lpBuffers[i].buf;
			lpTemBuf[i].len = lpBuffers[i].len;
		}

		wchar_t szValue[100] = {0};
		_snwprintf_s(szValue, 100, _TRUNCATE, L"%p, %d, 0", lpTemBuf, dwBufferCount);

		g_listOverlappedBuf.AddItem(szKey, szValue);//Cache the buffer address and buffer count.

	}
	
	if(iRet == SOCKET_ERROR)
	{
		return iRet;
	}

	CFtpConnMgr& ftpMgrIns = CFtpConnMgr::Instance();
	if(ftpMgrIns.GetFtpProtocolType(s) == FPT_REGULAR)
	{
		if(lpBuffers != NULL && lpNumberOfBytesRecvd != NULL)
		{
			int nBytesRecvd = *lpNumberOfBytesRecvd;
			string sBuf;
			DWORD dwCnt = 0;
			while(nBytesRecvd > 0 && dwCnt < dwBufferCount)
			{
				if(lpBuffers[dwCnt].buf != NULL && lpBuffers[dwCnt].len > 0)
				{
					sBuf.append(lpBuffers[dwCnt].buf, min(lpBuffers[dwCnt].len, (unsigned int)nBytesRecvd));
				}
				nBytesRecvd -= lpBuffers[dwCnt].len;
				++dwCnt;
			}

			if(sBuf.length() > 0)
			{
				string sParsingBuf(sBuf);
				ParserResult pr = ftpMgrIns.ParseRecv(s, sParsingBuf);
				if(pr == PARSER_DENY)
				{
					return SOCKET_ERROR;
				}
				else if(pr == PARSER_BUF_MODIFIED)
				{
					string::size_type idxStr = 0;
					for(DWORD nBuf = 0; nBuf < dwBufferCount; ++nBuf)
					{
						if(lpBuffers[nBuf].buf != NULL && lpBuffers[nBuf].len > 0)
						{
							for(DWORD idxBuf = 0; idxBuf < lpBuffers[nBuf].len && idxStr < sParsingBuf.length(); ++idxBuf)
							{
								if(sParsingBuf[idxStr] != lpBuffers[nBuf].buf[idxBuf])
								{
									lpBuffers[nBuf].buf[idxBuf] = sParsingBuf[idxStr];
								}
								++idxStr;
							}
						}
					}
				}
			}
		}
	}

	CMapperMgr& mapperIns = CMapperMgr::Instance();
	DWORD dwThreadID = ::GetCurrentThreadId();
	mapperIns.PushThreadSocket(dwThreadID, s);
	
	/*
		Added for the IE/Opera/Chrome/Firefox. When the user click the file directly, it could not be denied in the write file,
		so we try to have the ability to deny user click the file directly here.
		For some small file, like *.txt, Browser won't save it to local file. it will read the content to memory and show it 
		directly. so we need to do query here.
	*/
	if (IsProcess(L"opera.exe") || IsProcess(L"iexplore.exe") || IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))
	{
		FTP_EVAL_INFO evalInfo ;
		evalInfo = mapperIns.PopSocketBufEval4RecvEval(s);
		evalInfo.pszSrcFileName = evalInfo.pszDestFileName ;
		if(evalInfo.IsValid() && evalInfo.iProtocolType == FPT_REGULAR )
		{
			evalInfo.pszDestFileName = DUMMY_DESTINATION ;

			//Check the evaluation cache first
			BOOL bAllow;
			FTPE_STATUS status = FTPE_SUCCESS ;
			CEResponse_t response;
			if(GetEvalCache(CPolicy::m_ftp, evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bAllow))
			{
				response = bAllow? CEAllow: CEDeny;
			}
			else
			{
			CPolicy *pPolicy = CPolicy::CreateInstance() ;
			CEEnforcement_t enforcement ;
			memset(&enforcement, 0, sizeof(CEEnforcement_t));

			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;

				response = enforcement.result;
			cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

			pPolicy->Release() ;

				BOOL bEvalResult = (response == CEAllow);
				PushEvalCache(CPolicy::m_ftp, evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bEvalResult );
			}

			
			if(status == FTPE_SUCCESS)
			{
				switch(response)
				{
				case CEAllow:
					break ;
				case CEDeny:
					{
						mapperIns.RemoveSocketBufEval4RecvEval(s) ;
						return SOCKET_ERROR;
					}
				default:
					break;
				}
			}
		}
	}

   	return iRet ;
}

int WSPAPI WSPRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	__try
	{
		return my_WSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
}

BOOL HandleReceivedData(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd)//FALSE means denied
{
	CFtpConnMgr& ftpMgrIns = CFtpConnMgr::Instance();
	if(ftpMgrIns.GetFtpProtocolType(s) == FPT_REGULAR)
	{
		if(lpBuffers != NULL && lpNumberOfBytesRecvd != NULL)
		{
			int nBytesRecvd = *lpNumberOfBytesRecvd;
			string sBuf;
			DWORD dwCnt = 0;
			while(nBytesRecvd > 0 && dwCnt < dwBufferCount)
			{
				if(lpBuffers[dwCnt].buf != NULL && lpBuffers[dwCnt].len > 0)
				{
					sBuf.append(lpBuffers[dwCnt].buf, min(lpBuffers[dwCnt].len, (unsigned int)nBytesRecvd));
				}
				nBytesRecvd -= lpBuffers[dwCnt].len;
				++dwCnt;
			}

			if(sBuf.length() > 0)
			{
				string sParsingBuf(sBuf);
				ParserResult pr = ftpMgrIns.ParseRecv(s, sParsingBuf);
				if(pr == PARSER_DENY)
				{
					return FALSE;
				}

				else if(pr == PARSER_BUF_MODIFIED)
				{
					string::size_type idxStr = 0;
					for(DWORD nBuf = 0; nBuf < dwBufferCount; ++nBuf)
					{
						if(lpBuffers[nBuf].buf != NULL && lpBuffers[nBuf].len > 0)
						{
							for(DWORD idxBuf = 0; idxBuf < lpBuffers[nBuf].len && idxStr < sParsingBuf.length(); ++idxBuf)
							{
								if(sParsingBuf[idxStr] != lpBuffers[nBuf].buf[idxBuf])
								{
									lpBuffers[nBuf].buf[idxBuf] = sParsingBuf[idxStr];
								}
								++idxStr;
							}
						}
					}
				}
			}
		}
	}

	CMapperMgr& mapperIns = CMapperMgr::Instance();
	if (IsProcess(L"opera.exe") || IsProcess(L"iexplore.exe") || IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe"))
	{
		FTP_EVAL_INFO evalInfo ;
		evalInfo = mapperIns.PopSocketBufEval4RecvEval(s);
		evalInfo.pszSrcFileName = evalInfo.pszDestFileName ;
		if(evalInfo.IsValid() && evalInfo.iProtocolType == FPT_REGULAR )
		{
			evalInfo.pszDestFileName = DUMMY_DESTINATION ;

			//Check the evaluation cache first
			BOOL bAllow;
			FTPE_STATUS status = FTPE_SUCCESS ;
			CEResponse_t response;
			if(GetEvalCache(CPolicy::m_ftp, evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bAllow))
			{
				response = bAllow? CEAllow: CEDeny;
			}
			else
			{
				CPolicy *pPolicy = CPolicy::CreateInstance() ;
				CEEnforcement_t enforcement ;
				memset(&enforcement, 0, sizeof(CEEnforcement_t));

				status = pPolicy->QuerySingleFilePolicy( CPolicy::m_ftp, evalInfo , enforcement ) ;

				response = enforcement.result;
				cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);

				pPolicy->Release() ;

				BOOL bEvalResult = (response == CEAllow);
				PushEvalCache(CPolicy::m_ftp, evalInfo.pszSrcFileName.c_str(), evalInfo.pszDestFileName.c_str(), bEvalResult );
			}


			if(status == FTPE_SUCCESS)
			{
				switch(response)
				{
				case CEAllow:
					break ;
				case CEDeny:
					{
						mapperIns.RemoveSocketBufEval4RecvEval(s) ;
						return FALSE;
					}
				default:
					break;
				}
			}
		}
	}

	return TRUE;
}

BOOL WINAPI
my_WSPGetOverlappedResult(
						  IN SOCKET  s,
						  IN LPWSAOVERLAPPED  lpOverlapped,
						  OUT LPDWORD  lpcbTransfer,
						  IN BOOL  fWait,
						  OUT LPDWORD  lpdwFlags,
						  OUT LPINT  lpErrno
					   )
{
	BOOL bRet = nextproctable.lpWSPGetOverlappedResult(s, lpOverlapped, lpcbTransfer, fWait, lpdwFlags, lpErrno);
	if(bRet)
	{//it means the "overlapped" is finished, so we can try to get the recv buffer here.
		wchar_t szKey[100] = {0};
		_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%p", s, lpOverlapped);
		wstring strValue;
		if(g_listOverlappedBuf.FindItem(szKey, strValue))//Try to get the "recv" buffer address with socket and overlapped structure
		{
			g_listOverlappedBuf.DeleteItem(szKey);
			size_t uIndex = strValue.find(L",");
			if(uIndex != wstring::npos)
			{
				wstring strBuf = strValue.substr(0, uIndex);
				LPWSABUF uBufAddr = 0;
				swscanf_s(strBuf.c_str(), L"%p"  ,&uBufAddr );//get the address of LPWSABUF which was posted in "WSPRecv"

				LPWSABUF lpBuffers = (LPWSABUF)uBufAddr;

				DWORD dwBufCount = 0;
				size_t uIndex2 = strValue.find(L",", uIndex + 1);
				if(uIndex2 != wstring::npos)
				{
					swscanf_s(strValue.substr(uIndex + 1, uIndex2 - uIndex - 1).c_str(), L"%d", &dwBufCount);//get buffer count

					DWORD dwTransfer = 0;
					swscanf_s(strValue.substr(uIndex2 + 1, strValue.length() - uIndex2 -1).c_str(), L"%d", &dwTransfer);//get the count of bytes which were received already
					dwTransfer += *lpcbTransfer;//add the count of new bytes which were transferred.

					if(!HandleReceivedData(s, lpBuffers, dwBufCount, &dwTransfer))
					{//denied
						*lpcbTransfer = 0;
					}

					if(lpBuffers)
					{
						delete [] lpBuffers;
						lpBuffers = NULL;
					}
					
				}

			}
		}
	}
	else
	{
		if(WSAGetLastError() == WSA_IO_INCOMPLETE)
		{//Update the count of transferred the bytes.
			wchar_t szKey[100] = {0};
			_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%p", s, lpOverlapped);
			wstring strValue;
			if(g_listOverlappedBuf.FindItem(szKey, strValue))
			{
				size_t uIndex = strValue.rfind(L",");
				if(uIndex != wstring::npos)
				{
					wstring strTransfer = strValue.substr(uIndex + 1, strValue.length() - uIndex - 1);
					int nTransfer = 0;
					swscanf_s(strTransfer.c_str(), L"%d", &nTransfer);//Get the old count of transferred bytes.
					nTransfer += *lpcbTransfer;//add the count of new transferred bytes
					strValue = strValue.substr(0, uIndex + 1);
					wchar_t szTransfer[50] = {0};
					_snwprintf_s(szTransfer, 50, _TRUNCATE, L"%d", nTransfer);
					strValue += wstring(szTransfer);
					g_listOverlappedBuf.DeleteItem(szKey);//update the existing item
					g_listOverlappedBuf.AddItem(szKey, strValue);
				}
			}
		}
	}
	return bRet;
}

BOOL WINAPI
WSPGetOverlappedResult(
					   IN SOCKET  s,
					   IN LPWSAOVERLAPPED  lpOverlapped,
					   OUT LPDWORD  lpcbTransfer,
					   IN BOOL  fWait,
					   OUT LPDWORD  lpdwFlags,
					   OUT LPINT  lpErrno
					   )
{
	__try
	{
		return my_WSPGetOverlappedResult(
			s,
			lpOverlapped,
			lpcbTransfer,
			fWait,
			lpdwFlags,
			lpErrno
			);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return TRUE;
}

int WSPAPI my_WSPConnect(SOCKET s, const struct sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	if(hook_control.is_disabled())
	{
		return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);

	if(IsIgnoredDestAddress(name))
	{
		return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	int iRet = nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);

	if(iRet != SOCKET_ERROR || *lpErrno == WSAEWOULDBLOCK)
	{
		CFtpConnMgr& ins = CFtpConnMgr::Instance();
		ins.AddConn(s, *lpErrno == WSAEWOULDBLOCK, name);
	}

	return iRet;
}

int WSPAPI WSPConnect(SOCKET s, const struct sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno)
{
	__try
	{
		return my_WSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
}

extern GUID FTPE_LAYER_GUID;
extern GUID HPE_LAYER_GUID;
extern GUID MSAFD_PROVIDER_GUID;

int WSPAPI WSPStartup(WORD wVersionRequested,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFOW lpProtocolInfo,
					  WSPUPCALLTABLE UpcallTable,
					  LPWSPPROC_TABLE lpProcTable)
{
	if(hook_control.is_disabled())
	{
		HMODULE hModule = GetModuleHandleW(L"mswsock.dll") ;//Try to get the handle of mswsock
		if(!hModule)
		{
			hModule = LoadLibraryW(L"mswsock.dll");
		}
		if(hModule)
		{
			LPWSAPROTOCOL_INFOW lpAllProtocolInfo = NULL;
			DWORD               cbAllProtocolInfo = 0;
			DWORD               dwTotalProtocols = 0;
			DWORD               dwIndex = 0;
			if(CProviderInstall::GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols))
			{	
				for(dwIndex = 0; dwIndex < dwTotalProtocols; dwIndex++)//Try to get the SPI of TCP/IP, mswsock
				{
					if(lpAllProtocolInfo[dwIndex].iAddressFamily == AF_INET &&
						lpAllProtocolInfo[dwIndex].iProtocol == IPPROTO_TCP  &&
						memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &MSAFD_PROVIDER_GUID, sizeof(GUID)) == 0 )
					{
						break;
					}
				}

				if(dwIndex < dwTotalProtocols)
				{
					LPWSPSTARTUP lpfnWSPStartup_WinSock = (LPWSPSTARTUP)GetProcAddress(hModule, "WSPStartup");
					if(NULL != lpfnWSPStartup_WinSock)//Call the "WSPStartup" of mswsock.dll
					{
						return lpfnWSPStartup_WinSock(wVersionRequested, lpWSPData, &lpAllProtocolInfo[dwIndex], UpcallTable,lpProcTable);
					}
				}

				CProviderInstall::FreeFilter(lpAllProtocolInfo);
			}
		}
		return 0;
	}

	nextlabs::recursion_control_auto auto_disable(hook_control);


	FTPE_Init();

	LPWSAPROTOCOL_INFOW lpAllProtocolInfo = NULL;
	DWORD               cbAllProtocolInfo = 0;
	DWORD               dwTotalProtocols = 0;
	DWORD               dwIndex = 0;

	if(memcmp(&(lpProtocolInfo->ProviderId), &FTPE_LAYER_GUID, sizeof(GUID)) != 0)
	{
		DP((L"Not FTPE_LAYER_GUID"));  
		return WSAEPROVIDERFAILEDINIT;
	}

	if(CProviderInstall::GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols) == FALSE)
	{
		return WSAEPROVIDERFAILEDINIT;
	}

	//Try to find the next provider
	for(dwIndex = 0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &FTPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			break;
		}
	}

	if(dwIndex >= dwTotalProtocols - 1)
	{
		DP((L"No avalible underlying providers"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	dwIndex++;

	wchar_t szDllPath[MAX_PATH] = {0};
	int nDllPathLen = MAX_PATH;
	int nErrno = 0;
	if(WSCGetProviderPath(&lpAllProtocolInfo[dwIndex].ProviderId, szDllPath, &nDllPathLen, &nErrno) == SOCKET_ERROR)
	{
		DP((L"WSCGetProviderPath Error!"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	if(!ExpandEnvironmentStrings(szDllPath, szDllPath, MAX_PATH))
	{
		DP((L"ExpandEnvironmentStrings Error!"));    
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	HMODULE hModule = LoadLibraryW(szDllPath) ;
	if(hModule == NULL)
	{
		DP((L"LoadLibrary Error!")); 
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	LPWSPSTARTUP lpfnWSPStartup = NULL;
	if((lpfnWSPStartup = (LPWSPSTARTUP)GetProcAddress(hModule, "WSPStartup")) == NULL)
	{
		DP((L"GetProcessAddress Error!"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	nErrno = lpfnWSPStartup(wVersionRequested, lpWSPData, &lpAllProtocolInfo[dwIndex], UpcallTable, lpProcTable);
	if(nErrno != 0)
	{
		DP((L"wspstartupfunc Error!"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return nErrno;
	}

	if( CFilterRes::IsSupportedProcess() == TRUE )
	{
		nextproctable = *lpProcTable;
		lpProcTable->lpWSPAccept = (LPWSPACCEPT)WSPAccept;
		lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
		lpProcTable->lpWSPSend = WSPSend ;
		lpProcTable->lpWSPRecv = WSPRecv ;
 		lpProcTable->lpWSPConnect = WSPConnect;
 		lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
	}
	CProviderInstall::FreeFilter(lpAllProtocolInfo);

	
	DP((L"FTPE WSPStartup Success\r\n")) ;

	return 0;
}
