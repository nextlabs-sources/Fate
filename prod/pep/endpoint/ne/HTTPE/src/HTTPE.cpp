// HTTPE.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "HTTPE.h"
#include "SPIInstaller.h"
#include "HTTPMgr.h"
#include "timeout_list.hpp"

#pragma comment(lib, "Ws2_32.lib")

CTimeoutList g_listOverlappedBuf;

nextlabs::recursion_control hook_control_httpe;

int  WINAPI	 InstallSPI() 
{
	int iRet = 0 ;
	CProviderInstall provider ;
	
	iRet = (int)provider.InstallProvider() ;
	
	return iRet ;
}
int  WINAPI	 UninstallSPI()	 
{

	int iRet = 0 ;
	CProviderInstall provider ;
	iRet = (int)provider.UnInstallFilter() ;
	return iRet ;
}

extern GUID HTTPE_LAYER_GUID;
extern GUID FTPE_LAYER_GUID;
extern GUID HPE_LAYER_GUID;
extern GUID MSAFD_PROVIDER_GUID;

static WSPPROC_TABLE nextproctable;


int WSPAPI my_WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{	
	if(GetDetachFlag() || 0 == dwBufferCount)
	{
		return nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}

	/**************************************************************
	Below code is for Opera only.
	Opera is strange, the pointer of buffer will be invalid sometime.
	So, we need to check the pointer.
	Currently, we only found this problem on Opera.

												Kevin 2010-1-12
	***************************************************************/
	if(IsProcess(L"opera.exe") && dwBufferCount > 0)
	{
		if(IsBadReadPtr(lpBuffers[0].buf, lpBuffers[0].len))
		{
			g_log.Log(CELOG_DEBUG, "HTTPE::This pointer is not valid(Opera), %d, %d", (INT_PTR)lpBuffers[0].buf, lpBuffers[0].len);

			return SOCKET_ERROR;
		}
	}

	CHttpMgr& mgr = CHttpMgr::GetInstance();
	char* pTemp =   lpBuffers[0].buf ;		
	DWORD dBufLen = lpBuffers[0].len ;
    DWORD dwAddedData = 0 ;
	DWORD dwRet = mgr.ProcessHTTPData(s, lpBuffers, dwBufferCount, &dwAddedData, 0);

	//	check for upload ,
	if ( dwRet == HTTP_UPLOAD_DENIED )
	{
		//	the tcp packet contain uploaded data which is denied,
		//	so we do not send the tcp packet out.
		//	and shutdown the SD_SEND of this socket handle, 
		//	to make server quit receiving data.
		//	first, remember original buffer
		DWORD dwOriginToSent = lpBuffers[0].len ;
		char* pOriginBuf = lpBuffers[0].buf ;

		//	new a buffer
		const DWORD buflen = 1;
		char* pBuf = new char[ buflen ] ;
		if( pBuf != NULL )
		{
			//	set fake data to buffer
			const DWORD dwFakeData = 'a';
			memset(pBuf, dwFakeData, buflen) ;

			//	replace buffer
			lpBuffers[0].buf = pBuf ; 
			lpBuffers[0].len = buflen ;

			//	send
			dwRet =nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);  
			
			//	reset buffer
			if( *lpNumberOfBytesSent ==	buflen )
			{
				*lpNumberOfBytesSent = dwOriginToSent ;
			}
			lpBuffers[0].buf = pOriginBuf ;
			lpBuffers[0].len = dwOriginToSent;

			//	free new buffer
			delete [] pBuf ;
			pBuf = NULL ;

			//	shutdown socket with SD_SEND to make the http server quit receiving data
			shutdown(s, SD_SEND);

			return dwRet;
		}
	}

	if((dwRet != 0)&&(dwRet != HTTP_HEADER_INJECTION))//denied.
	{
		*lpErrno = WSAESHUTDOWN;
		if(IsProcess(L"firefox.exe"))
		{
			closesocket(s);
		}
		return SOCKET_ERROR;
	}
	DWORD dwSendRet = nextproctable.lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	if( dwRet == HTTP_HEADER_INJECTION )
	{  //Added for the HTTP HEADER INJECTION
		g_log.Log(CELOG_DEBUG,"\r\n%s\r\nSended Length[%d] Buffer Length[%d],Origin Buffer[%d]", lpBuffers[0].buf,*lpNumberOfBytesSent,lpBuffers[0].len,dBufLen);
		if( *lpNumberOfBytesSent >dBufLen ) 
		{
			*lpNumberOfBytesSent = *lpNumberOfBytesSent - dwAddedData ;
		}
		delete []lpBuffers[0].buf ;
		lpBuffers[0].buf = pTemp ;
		lpBuffers[0].len = dBufLen ;
	}

	return dwSendRet ;
}

int WSPAPI WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	__try
	{
		return my_WSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped,
						lpCompletionRoutine, lpThreadId, lpErrno);
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
	if(lpFlags != NULL && *lpFlags == MSG_PEEK)
	{
		return nextproctable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	}


	int iRet = nextproctable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

	if(GetDetachFlag() || lpBuffers == NULL || dwBufferCount == 0 || NULL == lpNumberOfBytesRecvd)
	{
		return iRet;
	}

	DWORD dwReceived = *lpNumberOfBytesRecvd;
	if(lpErrno && *lpErrno == WSA_IO_PENDING && lpOverlapped)//Chrome uses "overlapped" mode of SOCKET, 
	{
		wchar_t szKey[100] = {0};
		_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%x", s, (INT_PTR)lpOverlapped);

		LPWSABUF lpTemBuf = new WSABUF[dwBufferCount];
		for( DWORD i = 0; i < dwBufferCount; i++)
		{
			lpTemBuf[i].buf = lpBuffers[i].buf;
			lpTemBuf[i].len = lpBuffers[i].len;
		}

		wchar_t szValue[100] = {0};
		_snwprintf_s(szValue, 100, _TRUNCATE, L"%x, %d, 0", (INT_PTR)lpTemBuf, dwBufferCount);

		g_listOverlappedBuf.AddItem(szKey, szValue);//Cache the buffer address and buffer count.

	}
	
	if( iRet == SOCKET_ERROR )
	{
		return iRet;
	}

	CHttpMgr& mgr = CHttpMgr::GetInstance();
	DWORD dRet = mgr.ProcessHTTPData(s, lpBuffers, dwBufferCount, &dwReceived, 1);
	
	if(	HTTP_REDIRECT == dRet || HTTP_NAVIGATION_DENIED == dRet || HTTP_UPLOAD_DENIED == dRet )
	{
		//	the data in \c lpBuffers is replaced,
		//	so, reset \c lpNumberOfBytesRecvd.
		*lpNumberOfBytesRecvd =dwReceived ;
	}

	return iRet ;
}

int WSPAPI WSPRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped,
				   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	__try
	{
		return my_WSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped,
						lpCompletionRoutine, lpThreadId, lpErrno);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return 0;
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
		_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%x", s, (INT_PTR)lpOverlapped);
		wstring strValue;
		if(g_listOverlappedBuf.FindItem(szKey, strValue))//Try to get the "recv" buffer address with socket and overlapped structure
		{
			g_listOverlappedBuf.DeleteItem(szKey);
			std::wstring::size_type uIndex = strValue.find(L",");
			if(uIndex != wstring::npos)
			{
				wstring strBuf = strValue.substr(0, uIndex);
				UINT_PTR uBufAddr = 0;
				swscanf_s(strBuf.c_str(), L"%x"  ,&uBufAddr );//get the address of LPWSABUF which was posted in "WSPRecv"

				LPWSABUF lpBuffers = (LPWSABUF)uBufAddr;

				DWORD dwBufCount = 0;
				std::wstring::size_type uIndex2 = strValue.find(L",", uIndex + 1);
				if(uIndex2 != wstring::npos)
				{
					swscanf_s(strValue.substr(uIndex + 1, uIndex2 - uIndex - 1).c_str(), L"%d", &dwBufCount);//get buffer count

					DWORD dwTransfer = 0;
					swscanf_s(strValue.substr(uIndex2 + 1, strValue.length() - uIndex2 -1).c_str(), L"%d", &dwTransfer);//get the count of bytes which were received already
					dwTransfer += *lpcbTransfer;//add the count of new bytes which were transferred.

					CHttpMgr& mgr = CHttpMgr::GetInstance();
					DWORD dRet = mgr.ProcessHTTPData(s, lpBuffers, dwBufCount, &dwTransfer, 1);

					if(lpBuffers)
					{
						delete [] lpBuffers;
						lpBuffers = NULL;
					}
					if(	HTTP_REDIRECT == dRet || HTTP_NAVIGATION_DENIED == dRet || HTTP_UPLOAD_DENIED == dRet )
					{
						//	the data in \c lpBuffers is replaced,
						//	so, reset \c lpNumberOfBytesRecvd.
						*lpcbTransfer = dwTransfer ;
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
			_snwprintf_s(szKey, 100, _TRUNCATE, L"%d,%x", s, (INT_PTR)lpOverlapped);
			wstring strValue;
			if(g_listOverlappedBuf.FindItem(szKey, strValue))
			{
				std::wstring::size_type uIndex = strValue.rfind(L",");
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


int WSPAPI WSPStartup(WORD wVersionRequested,
					  LPWSPDATA lpWSPData,
					  LPWSAPROTOCOL_INFOW lpProtocolInfo,
					  WSPUPCALLTABLE UpcallTable,
					  LPWSPPROC_TABLE lpProcTable)
{
	if(hook_control_httpe.is_disabled())
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
					if( NULL != lpfnWSPStartup_WinSock)//Call the "WSPStartup" of mswsock.dll
					{
						return lpfnWSPStartup_WinSock(wVersionRequested, lpWSPData, &lpAllProtocolInfo[dwIndex], UpcallTable,lpProcTable);
					}
				}

				CProviderInstall::FreeFilter(lpAllProtocolInfo);
			}
		}
		return 0;
	}

	nextlabs::recursion_control_auto auto_disable(hook_control_httpe);

	HTTPE_Initialize();

	LPWSAPROTOCOL_INFOW lpAllProtocolInfo = NULL;
	DWORD               cbAllProtocolInfo = 0;
	DWORD               dwTotalProtocols = 0;
	DWORD               dwIndex = 0;

	if(memcmp(&(lpProtocolInfo->ProviderId), &HTTPE_LAYER_GUID, sizeof(GUID)) != 0)
	{

		return WSAEPROVIDERFAILEDINIT;
	}

	if(CProviderInstall::GetFilter(lpAllProtocolInfo, cbAllProtocolInfo, dwTotalProtocols) == FALSE)
	{
		return WSAEPROVIDERFAILEDINIT;
	}

	//Try to find the next provider
	for(dwIndex = 0; dwIndex < dwTotalProtocols; dwIndex++)
	{
		if(memcmp(&lpAllProtocolInfo[dwIndex].ProviderId, &HTTPE_LAYER_GUID, sizeof(GUID)) == 0)
		{
			break;//Found the index of httpe provider
			}
		}

	if(dwIndex >= dwTotalProtocols - 1)
		{
			CProviderInstall::FreeFilter(lpAllProtocolInfo);
			return WSAEPROVIDERFAILEDINIT;
		}

	dwIndex++;
	

	wchar_t szDllPath[MAX_PATH]={0};
	int nDllPathLen = MAX_PATH;
	int nErrno = 0;
	if(WSCGetProviderPath(&lpAllProtocolInfo[dwIndex].ProviderId, szDllPath, &nDllPathLen, &nErrno) == SOCKET_ERROR)//Try to get the related path for the next layer
	{ 
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	if(!ExpandEnvironmentStrings(szDllPath, szDllPath, MAX_PATH))
	{  
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	HMODULE hModule = LoadLibraryW(szDllPath) ;//Load the DLL of next layer under HTTPE
	if(hModule == NULL)
	{
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	LPWSPSTARTUP lpfnWSPStartup = NULL;
	if((lpfnWSPStartup = (LPWSPSTARTUP)GetProcAddress(hModule, "WSPStartup")) == NULL)//Call the "WSPStartup" of next layer, so that SPI can be passed to next layer.
	{ 
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return WSAEPROVIDERFAILEDINIT;
	}

	nErrno = lpfnWSPStartup(wVersionRequested, lpWSPData, &lpAllProtocolInfo[dwIndex], UpcallTable, lpProcTable);
	if(nErrno != 0)
	{
		CProviderInstall::FreeFilter(lpAllProtocolInfo);
		return nErrno;
	}


	if( IsSupportedProcess())
	{
		nextproctable = *lpProcTable;
	
		lpProcTable->lpWSPSend = WSPSend ;
		lpProcTable->lpWSPRecv = WSPRecv ;
		lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
	}
	CProviderInstall::FreeFilter(lpAllProtocolInfo);
	return 0;
}


