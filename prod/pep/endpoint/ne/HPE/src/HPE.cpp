// FTPE.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <list>
#include <string>

#pragma warning(push)
#pragma warning(disable: 6386)
#include <Ws2tcpip.h>
#pragma warning(pop)

using namespace std;
#include "MapperMgr.h"
#include "FilterRes.h"
#include "SPIInstaller.h"
#include "Eval.h"
#include "smart_ptr.h"
#include "eframework/auto_disable/auto_disable.hpp"
#include "eframework/platform/cesdk_loader.hpp"
extern nextlabs::cesdk_loader cesdkLoader;

#pragma comment(lib, "Ws2_32.lib")

#define EVAL_CACHE_TIME						10000

extern nextlabs::recursion_control hpe_hook_control;


typedef struct struEvalCache
{
	std::wstring strAction;
	std::wstring strSrc;
	std::wstring strDest;
	CEResponse_t nResult;
	DWORD		 dwTime;
}EVALCACHE, *LPEVALCACHE;

	
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

SOCKET WSPAPI WSPSocket(
				 int af,
				 int type,
				 int protocol,
				 LPWSAPROTOCOL_INFO lpProtocolInfo,
				 GROUP g,
				 DWORD dwFlags,
				 LPINT lpErrno
				 )
{
	SOCKET s = nextproctable.lpWSPSocket(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
	
	return s;
}

SOCKET WSPAPI WSPAccept(
				SOCKET s,
				struct sockaddr* addr,
				LPINT addrlen,
				LPCONDITIONPROC lpfnCondition,
				DWORD dwCallbackData,
				LPINT lpErrno
				)
{
	return nextproctable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);
}

int WSPAPI WSPCloseSocket(
				   SOCKET s,
				   LPINT lpErrno
				   )
{
	return nextproctable.lpWSPCloseSocket(s, lpErrno);
}

int WSPAPI my_WSPSend(
  SOCKET s,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesSent,
  DWORD dwFlags,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
  LPWSATHREADID lpThreadId,
  LPINT lpErrno
)
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPSend( s, lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags, lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno );
	}

	if(hpe_hook_control.is_disabled())
	{
		return nextproctable.lpWSPSend( s, lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags, lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno );
	}

	nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	struct sockaddr  name ;
	INT iLen = sizeof(sockaddr)  ;
	if(getpeername( s, &name, &iLen )  == 0 )
	{
		string sPeerIP = AddressToString(&name, sizeof(sockaddr), false);
		std::wstring strDestIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(strDestIP.c_str()))//added by kevin 2009-7-1
		{
			return nextproctable.lpWSPSend( s, lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags, lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno );
		}

		struct sockaddr_in* addr_v4 = (sockaddr_in*)&name;
		int nRemotePort = ntohs(addr_v4->sin_port);
		wchar_t wszPortBuf[20] = {0};
		_snwprintf_s(wszPortBuf, 20, _TRUNCATE, L"%d", nRemotePort);
		
		if( CheckNetworkAccess(s,strDestIP.c_str(),wszPortBuf) ==	 FALSE )
		{
			if( IsProcess(L"bpftpclient.exe") )
			{
				string sBuf;
				for(DWORD dwCnt = 0; dwCnt < dwBufferCount; ++dwCnt)
				{
					if(lpBuffers[dwCnt].buf != NULL && lpBuffers[dwCnt].len > 0)
					{
						sBuf.append(lpBuffers[dwCnt].buf, lpBuffers[dwCnt].len);
					}
				}
				if(lpNumberOfBytesSent != NULL)
				{
				  *lpNumberOfBytesSent = (DWORD)sBuf.length();
				}
				return 0 ;
			}
			if(IsProcess(L"FlashFXP.exe"))
			{
				closesocket(s);
			}

			*lpErrno = WSAESHUTDOWN;

			if(IsProcess(L"iexplore.exe"))
			{	
				*lpErrno = WSAETIMEDOUT;
			}

			return	SOCKET_ERROR ;
		}
	}
	return nextproctable.lpWSPSend( s, lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags, lpOverlapped,lpCompletionRoutine,lpThreadId,lpErrno );
}

int WSPAPI WSPSend(
				   SOCKET s,
				   LPWSABUF lpBuffers,
				   DWORD dwBufferCount,
				   LPDWORD lpNumberOfBytesSent,
				   DWORD dwFlags,
				   LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				   LPWSATHREADID lpThreadId,
				   LPINT lpErrno
				   )
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
int WSPAPI WSPRecv(
  SOCKET s,
  LPWSABUF lpBuffers,
  DWORD dwBufferCount,
  LPDWORD lpNumberOfBytesRecvd,
  LPDWORD lpFlags,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
  LPWSATHREADID lpThreadId,
  LPINT lpErrno 
  )
{
	return nextproctable.lpWSPRecv( s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,	 lpFlags, lpOverlapped,   lpCompletionRoutine,lpThreadId,lpErrno ) ;
}

int WSPAPI WSPAddressToString(
					   LPSOCKADDR lpsaAddress,
					   DWORD dwAddressLength,
					   LPWSAPROTOCOL_INFO lpProtocolInfo,
					   LPWSTR lpszAddressString,
					   LPDWORD lpdwAddressStringLength,
					   LPINT lpErrno
					   )
{

	return nextproctable.lpWSPAddressToString(lpsaAddress, dwAddressLength,
		lpProtocolInfo, lpszAddressString, lpdwAddressStringLength, lpErrno);
}

int WSPAPI WSPAsyncSelect(
				   SOCKET s,
				   HWND hWnd,
				   unsigned int wMsg,
				   long lEvent,
				   LPINT lpErrno
				   )
{
	return nextproctable.lpWSPAsyncSelect(s, hWnd, wMsg, lEvent, lpErrno);
}

int WSPAPI WSPBind(
			SOCKET s,
			const struct sockaddr* name,
			int namelen,
			LPINT lpErrno
			)
{
	return nextproctable.lpWSPBind(s, name, namelen, lpErrno);
}

int WSPAPI WSPCancelBlockingCall(
						  LPINT lpErrno
						  )
{
	return nextproctable.lpWSPCancelBlockingCall(lpErrno);
}

int WSPAPI WSPCleanup(
			   LPINT lpErrno
			   )
{
	return nextproctable.lpWSPCleanup(lpErrno);
}



int WSPAPI my_WSPConnect(
			   SOCKET s,
			   const struct sockaddr* name,
			   int namelen,
			   LPWSABUF lpCallerData,
			   LPWSABUF lpCalleeData,
			   LPQOS lpSQOS,
			   LPQOS lpGQOS,
			   LPINT lpErrno
			   )
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	SOCKET sockBuf = s;//kevin 2009-9-3
	if(hpe_hook_control.is_disabled())
	{
		return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	string sPeerIP = AddressToString(name, sizeof(sockaddr), false);
	struct sockaddr_in* addr_v4 = (sockaddr_in*)name;
	int nRemotePort = ntohs(addr_v4->sin_port);
	if(sPeerIP != "" && nRemotePort > 0)
	{
		FTP_EVAL_INFO evalInfo;
		evalInfo.pszServerIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(evalInfo.pszServerIP.c_str()))//added by kevin 2009-7-1
		{
			return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
		}

		wchar_t wszPortBuf[20] = {0};
		_snwprintf_s(wszPortBuf, 20, _TRUNCATE, L"%d", nRemotePort);
		evalInfo.pszServerPort = wszPortBuf;
		DPW((L"HPE trying to connect to %s:%s\n", evalInfo.pszServerIP.c_str(), wszPortBuf));
		CPolicy *pPolicy = CPolicy::CreateInstance() ;
		CEEnforcement_t enforcement ;
		memset(&enforcement, 0, sizeof(CEEnforcement_t));
		evalInfo.pszSrcFileName = L"server://"+evalInfo.pszServerIP + L":" + evalInfo.pszServerPort ;
		FTPE_STATUS status = FTPE_SUCCESS;
		
			status = pPolicy->QuerySingleFilePolicy( CPolicy::m_hostopen, evalInfo , enforcement ) ;

		CEResponse_t response = enforcement.result;

		if(status == FTPE_SUCCESS && cesdkLoader.fns.CEEVALUATE_FreeEnforcement && enforcement.obligation)
		{
		cesdkLoader.fns.CEEVALUATE_FreeEnforcement(enforcement);
		}

		pPolicy->Release() ;
		if(status == FTPE_SUCCESS)
		{
			switch(response)
			{
			case CEAllow:
				break ;
			case CEDeny:
				{
					*lpErrno = WSAECONNREFUSED;
					return SOCKET_ERROR;
				}
				break ;
			default:
				break;
			}
		}
		/*
		Added by chellee on02/07/2009, 
		add for a filter for the cuteftp, check if it is the cute ftp, if it is ture, do the network access in the send	.
		*/
		if( !IsProcess( L"ftpte.exe" ) && !IsProcess( L"cuteftppro.exe" ) )
		{
			if(IsProcess(L"iexplore.exe") && 6 == GetIEVersionNum() )//fix bug9868
			{
				CMapperMgr& mapperIns = CMapperMgr::Instance();
				mapperIns.CheckExpiredItem();
			}
			if( CheckNetworkAccess(sockBuf,evalInfo.pszServerIP.c_str(),evalInfo.pszServerPort.c_str()) == FALSE )
					{
				//	Added by bsong on July 16, 2009, if it is opera, return 0, otherwise opera would crash after return SOCKET_ERROR
				if (IsProcess( L"opera.exe"))
				{
					return 0;
				}
				
						*lpErrno = WSAECONNREFUSED;

				if(IsProcess(L"iexplore.exe"))
				{	
					*lpErrno = WSAETIMEDOUT;
				}

						return SOCKET_ERROR;
					}
	}
	}
	return nextproctable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}

int WSPAPI WSPConnect(
				   SOCKET s,
				   const struct sockaddr* name,
				   int namelen,
				   LPWSABUF lpCallerData,
				   LPWSABUF lpCalleeData,
				   LPQOS lpSQOS,
				   LPQOS lpGQOS,
				   LPINT lpErrno
				   )
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

int WSPAPI WSPDuplicateSocket(
					   SOCKET s,
					   DWORD dwProcessId,
					   LPWSAPROTOCOL_INFO lpProtocolInfo,
					   LPINT lpErrno
					   )
{
	return nextproctable.lpWSPDuplicateSocket(s, dwProcessId, lpProtocolInfo, lpErrno);
}

int WSPAPI WSPEnumNetworkEvents(
						 SOCKET s,
						 WSAEVENT hEventObject,
						 LPWSANETWORKEVENTS lpNetworkEvents,
						 LPINT lpErrno
						 )
{
	return nextproctable.lpWSPEnumNetworkEvents(s, hEventObject, lpNetworkEvents, lpErrno);
}

int WSPAPI WSPEventSelect(
				   SOCKET s,
				   WSAEVENT hEventObject,
				   long lNetworkEvents,
				   LPINT lpErrno
				   )
{
	return nextproctable.lpWSPEventSelect(s, hEventObject, lNetworkEvents, lpErrno);
}

BOOL WSPAPI WSPGetOverlappedResult(
							SOCKET s,
							LPWSAOVERLAPPED lpOverlapped,
							LPDWORD lpcbTransfer,
							BOOL fWait,
							LPDWORD lpdwFlags,
							LPINT lpErrno
							)
{
	return nextproctable.lpWSPGetOverlappedResult(s, lpOverlapped, lpcbTransfer, 
		fWait, lpdwFlags, lpErrno);
}

int WSPAPI WSPGetPeerName(
				   SOCKET s,
					struct sockaddr* name,
					LPINT namelen,
					LPINT lpErrno
					)
{
	return nextproctable.lpWSPGetPeerName(s, name, namelen, lpErrno);
}

int WSPAPI WSPGetSockName(
				SOCKET s,
				struct sockaddr* name,
				LPINT namelen,
				LPINT lpErrno
				)
{
	return nextproctable.lpWSPGetSockName(s, name, namelen, lpErrno);
}

int WSPAPI WSPGetSockOpt(
				SOCKET s,
				int level,
				int optname,
				char* optval,
				LPINT optlen,
				LPINT lpErrno
				)
{
	return nextproctable.lpWSPGetSockOpt(s, level, optname, 
		optval, optlen, lpErrno);
}

BOOL WSPAPI WSPGetQOSByName(
					 SOCKET s,
					 LPWSABUF lpQOSName,
					 LPQOS lpQOS,
					 LPINT lpErrno
					 )
{
	return nextproctable.lpWSPGetQOSByName(s, lpQOSName, lpQOS, lpErrno);
}

int WSPAPI WSPIoctl(
			 SOCKET s,
			 DWORD dwIoControlCode,
			 LPVOID lpvInBuffer,
			 DWORD cbInBuffer,
			 LPVOID lpvOutBuffer,
			 DWORD cbOutBuffer,
			 LPDWORD lpcbBytesReturned,
			 LPWSAOVERLAPPED lpOverlapped,
			 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
			 LPWSATHREADID lpThreadId,
			 LPINT lpErrno
			 )
{
	return nextproctable.lpWSPIoctl(s, dwIoControlCode, lpvInBuffer, cbInBuffer,
		lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine, 
		lpThreadId, lpErrno);
}

SOCKET WSPAPI WSPJoinLeaf(
				   SOCKET s,
				   const struct sockaddr* name,
				   int namelen,
				   LPWSABUF lpCallerData,
				   LPWSABUF lpCalleeData,
				   LPQOS lpSQOS,
				   LPQOS lpGQOS,
				   DWORD dwFlags,
				   LPINT lpErrno
				   )
{
	return nextproctable.lpWSPJoinLeaf(s, name, namelen, lpCallerData,
		lpCalleeData, lpSQOS, lpGQOS, dwFlags, lpErrno);
}

int WSPAPI WSPListen(
			  SOCKET s,
			  int backlog,
			  LPINT lpErrno
			  )
{
	return nextproctable.lpWSPListen(s, backlog, lpErrno);
}

int WSPAPI WSPRecvDisconnect(
					  SOCKET s,
					  LPWSABUF lpInboundDisconnectData,
					  LPINT lpErrno
					  )
{
	return nextproctable.lpWSPRecvDisconnect(s, lpInboundDisconnectData, lpErrno);
}

int WSPAPI WSPRecvFrom(
				SOCKET s,
				LPWSABUF lpBuffers,
				DWORD dwBufferCount,
				LPDWORD lpNumberOfBytesRecvd,
				LPDWORD lpFlags,
				struct sockaddr* lpFrom,
				LPINT lpFromlen,
				LPWSAOVERLAPPED lpOverlapped,
				LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
				LPWSATHREADID lpThreadId,
				LPINT lpErrno
				)
{
	return nextproctable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
		lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSelect(
			int nfds,
			fd_set* readfds,
			fd_set* writefds,
			fd_set* exceptfds,
			const struct timeval* timeout,
			LPINT lpErrno
			)
{
	return nextproctable.lpWSPSelect(nfds, readfds, writefds, exceptfds, timeout, lpErrno);
}

int WSPAPI WSPSendDisconnect(
					  SOCKET s,
					  LPWSABUF lpOutboundDisconnectData,
					  LPINT lpErrno
					  )
{
	return nextproctable.lpWSPSendDisconnect(s, lpOutboundDisconnectData, lpErrno);
}

int WSPAPI my_WSPSendTo(
			  SOCKET s,
			  LPWSABUF lpBuffers,
			  DWORD dwBufferCount,
			  LPDWORD lpNumberOfBytesSent,
			  DWORD dwFlags,
			  const struct sockaddr* lpTo,
			  int iTolen,
			  LPWSAOVERLAPPED lpOverlapped,
			  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
			  LPWSATHREADID lpThreadId,
			  LPINT lpErrno
			  )
{
	if(GetDetachFlag())
	{
		return nextproctable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
			dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	if(hpe_hook_control.is_disabled())
	{
		return nextproctable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
			dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	if( lpTo)
	{
		string sPeerIP = AddressToString(lpTo, sizeof(sockaddr), false);
		std::wstring strDestIP = StringT1toT2<char, wchar_t>(sPeerIP);
		if(IsIgnoredIP(strDestIP.c_str()))
		{
			return nextproctable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
				dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
		}

		struct sockaddr_in* addr_v4 = (sockaddr_in*)lpTo;
		int nRemotePort = ntohs(addr_v4->sin_port);
		wchar_t wszPortBuf[20] = {0};
		_snwprintf_s(wszPortBuf, 20, _TRUNCATE, L"%d", nRemotePort);
		if( CheckNetworkAccess(s,strDestIP.c_str(),wszPortBuf) ==	 FALSE )
		{
			*lpErrno = WSAESHUTDOWN;
			return	SOCKET_ERROR ;
		}
	}
	return nextproctable.lpWSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
		dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
}

int WSPAPI WSPSendTo(
					 SOCKET s,
					 LPWSABUF lpBuffers,
					 DWORD dwBufferCount,
					 LPDWORD lpNumberOfBytesSent,
					 DWORD dwFlags,
					 const struct sockaddr* lpTo,
					 int iTolen,
					 LPWSAOVERLAPPED lpOverlapped,
					 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
					 LPWSATHREADID lpThreadId,
					 LPINT lpErrno
					 )
{
	__try
	{
		return my_WSPSendTo(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
}
int WSPAPI WSPSetSockOpt(
				  SOCKET s,
				  int level,
				  int optname,
				  const char* optval,
				  int optlen,
				  LPINT lpErrno
				  )
{
	return nextproctable.lpWSPSetSockOpt(s, level, optname, optval, optlen, lpErrno);
}

int WSPAPI WSPShutdown(
				SOCKET s,
				int how,
				LPINT lpErrno
				)
{
	return nextproctable.lpWSPShutdown(s, how, lpErrno);
}

int WSPAPI WSPStringToAddress(
					   LPWSTR AddressString,
					   INT AddressFamily,
					   LPWSAPROTOCOL_INFO lpProtocolInfo,
					   LPSOCKADDR lpAddress,
					   LPINT lpAddressLength,
					   LPINT lpErrno
					   )
{
	return nextproctable.lpWSPStringToAddress(AddressString, AddressFamily, lpProtocolInfo,
		lpAddress, lpAddressLength, lpErrno);
}

extern GUID HPE_LAYER_GUID;
extern GUID MSAFD_PROVIDER_GUID;

int WSPAPI WSPStartup(WORD wVersionRequested,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFOW lpProtocolInfo,
					  WSPUPCALLTABLE UpcallTable,
					  LPWSPPROC_TABLE lpProcTable)
{
	if(hpe_hook_control.is_disabled())
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

	nextlabs::recursion_control_auto auto_disable(hpe_hook_control);

	HPE_Init();

	LPWSAPROTOCOL_INFOW lpAllProtocolInfo = NULL;
	DWORD               cbAllProtocolInfo = 0;
	DWORD               dwTotalProtocols = 0;
	DWORD               dwIndex = 0;

	if(memcmp(&(lpProtocolInfo->ProviderId), &HPE_LAYER_GUID, sizeof(GUID)) != 0)
	{
		DP((L"Not HPE_LAYER_GUID\n"));  
		return WSAEPROVIDERFAILEDINIT;
	}

	if(CProviderInstall::GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols) == FALSE)
	{
		return WSAEPROVIDERFAILEDINIT;
	}

	//Try to find the next provider
	for(dwIndex = 0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			break;//Found the index of HPE provider
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
		DP((L"WSCGetProviderPath Error!\n"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	if(!ExpandEnvironmentStrings(szDllPath, szDllPath, MAX_PATH))
	{
		DP((L"ExpandEnvironmentStrings Error!\n"));    
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	HMODULE hModule = LoadLibraryW(szDllPath) ;
	if(hModule == NULL)
	{
		DP((L"LoadLibrary Error!\n")); 
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	LPWSPSTARTUP lpfnWSPStartup = NULL;
	if((lpfnWSPStartup = (LPWSPSTARTUP)GetProcAddress(hModule, "WSPStartup")) == NULL)
	{
		DP((L"GetProcessAddress Error!\n"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	nErrno = lpfnWSPStartup(wVersionRequested, lpWSPData, &lpAllProtocolInfo[dwIndex], UpcallTable, lpProcTable);
	if(nErrno != 0)
	{
		DP((L"wspstartupfunc Error!\n"));  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return nErrno;
	}

	if( CFilterRes::IsSupportedProcess() == TRUE )
	{
		nextproctable = *lpProcTable;
		lpProcTable->lpWSPSend = WSPSend ;
 		lpProcTable->lpWSPConnect = WSPConnect;
 		lpProcTable->lpWSPSendTo = WSPSendTo;
	}
	CProviderInstall::FreeFilter(lpAllProtocolInfo);

	
	DP((L"HPE WSPStartup Success\n")) ;

	return 0;
}
