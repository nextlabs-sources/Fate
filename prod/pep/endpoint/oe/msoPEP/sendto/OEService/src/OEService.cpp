// OEService.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <process.h>
#include "SvrAgent.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif
typedef struct __StructParam
{
	// changed by Tonny for event driver
	//BOOL	bExit;
	HANDLE  hExit;
	HANDLE	hThread;
}StructParam;
//static void EnablePlugin(PVOID lParam);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(ul_reason_for_call);
    UNREFERENCED_PARAMETER(lpReserved);
    return TRUE;
}

static void ListenFunc(PVOID lParam)
{
	StructParam* pFlag =(StructParam*)lParam;
	CSvrSocket theSocket;
	theSocket.StartSocket();
	SOCKET hSocket = theSocket.GetSocket();
	CPacket thePacket;
	WSAEVENT hSocketEvent[2];
	hSocketEvent[0]=pFlag->hExit;
	hSocketEvent[1]=WSACreateEvent();
	if(WSAEventSelect(hSocket,hSocketEvent[1],FD_READ) == SOCKET_ERROR)
	{
		//OutputDebugStringW(L"Register Net event failed!\r\n");
		return ;
	}
	unsigned long lBufLen=BUF_LEN;
	char szBuf[BUF_LEN]="\0";

	DWORD dwIndex = 0;
	while(true)
	{
		//dwIndex = WSAWaitForMultipleEvents(2,hSocketEvent,FALSE,WSA_INFINITE,TRUE);
		dwIndex = WaitForMultipleObjects(2,hSocketEvent,FALSE,INFINITE);
		if((dwIndex != WAIT_FAILED) && (dwIndex != WAIT_TIMEOUT))
		{
			dwIndex -= WAIT_OBJECT_0;
			if(dwIndex == 0)	// stop signal 
			{
				break;
			}
			else	// net event come in
			{
				// reset event handle
				WSAResetEvent(hSocketEvent[dwIndex]);
				WSANETWORKEVENTS nevents;
				int              rc=0;

				// Enumerate the events
				rc = WSAEnumNetworkEvents(
					hSocket,
					hSocketEvent[dwIndex],
					&nevents
					);
				if(rc == SOCKET_ERROR)
				{
					fprintf(stderr, "HandleIo: WSAEnumNetworkEvents failed: %d\n", WSAGetLastError());
					continue;
				}
				if (nevents.lNetworkEvents & FD_READ)
				{
					// Check for read error
					if (nevents.iErrorCode[FD_READ_BIT] == 0)
					{
						// So far it's enough to use block model receiving data rather than WSARecvFrom. because WSARecvFrom is
						// used to pre-post a recv, here receive event has been signaled ,so we don't need pre-post recv at here.
						lBufLen = theSocket.RecvData(szBuf,BUF_LEN);
						if(lBufLen == SOCKET_ERROR)	continue;
						KeyWords theNode;
						unsigned short uMsg = thePacket.GetKeyWordFromBin(szBuf,lBufLen,theNode);
						if(uMsg == SETFILESMSG)
						{
							thePacket.InsertANode(theNode);
						}
						else if(uMsg == GETFILESMSG)
						{
							bool bTrue = thePacket.GetANode(theNode);
							if(bTrue)
							{
								bTrue = thePacket.GetBinFromKeyWords(GETFILESMSG,theNode,szBuf,lBufLen);
								if(bTrue)
									lBufLen = theSocket.SendData(szBuf,lBufLen);
							}
						}
					}
				}
			}
		}
	}
	WSACloseEvent(hSocketEvent[1]);
	_endthreadex(0);
	return ;
}
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
	StructParam* pFlag = new StructParam;
	if(pFlag == NULL)	return 0;

	pFlag->hExit = CreateEvent(NULL,TRUE,FALSE,NULL);
	if(pFlag->hExit == NULL)
	{
		delete pFlag;
		return 0;
	}

	pFlag->hThread = (HANDLE)_beginthread(ListenFunc,NULL,pFlag);
	if(pFlag->hThread == NULL)	
	{
		delete pFlag;
		return 0;
	}
//remove this code, becuase since office2010 2013 2016 we do this in ControlPanel ---bard 2019-4-22
//	pFlag->hEnableHandle=(HANDLE)_beginthread(EnablePlugin,NULL,pFlag);
	*in_context = pFlag;
	//OutputDebugStringW(L"OEService.DLL were started successfully!\r\n");
	return 1;
}
extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
	StructParam* pFlag = (StructParam*)in_context;
	if(pFlag != NULL)
	{
		SetEvent(pFlag->hExit);
		HANDLE hHandle[1];
		hHandle[0]=pFlag->hThread;
		DWORD dw = WaitForMultipleObjects(sizeof(hHandle)/sizeof(hHandle[0]),hHandle,TRUE,2000);
		if(dw != WAIT_OBJECT_0)
		{
			//OutputDebugStringW(L"exit failed!\r\n");
		}
		CloseHandle(pFlag->hExit);
		CloseHandle(pFlag->hThread);
		delete pFlag;
		//OutputDebugStringW(L"OEService.DLL were stopped successfully!\r\n");
		return 1;
	}
	return 0;
}

// add for enable msopep.dll in case of mospep.dll crashed and user disable this plugin
// add by Tonny at 8-25
//////////////////////////////////////////////////////////////////////////
typedef std::pair<std::wstring, std::wstring>   FilePair;
// 0 : indicate look for failed 
inline int FindBinary(const PBYTE strSrc,const DWORD dwSrcLen,const PBYTE strContent,const DWORD dwContentLen)
{
	int nRet = 0;
	bool bFind = false;

	DWORD nSrcLen = dwSrcLen;
	PBYTE pstrSrc = strSrc;

	while(nSrcLen-- >= dwContentLen)
	{		
		nRet++;
		if(*pstrSrc++ != *strContent )	continue;

		for(int i=0;i<(int)dwContentLen-1;i++)
		{
			if(pstrSrc[i] != strContent[i+1]) break;
			if(i == (int)dwContentLen-2)	bFind = true;
		}

		if(1 == dwContentLen)	bFind = true;
		if(bFind)	break;
	}
	if (!bFind)	nRet = 0;
	return  nRet;
}

inline int DeleteItem(const wchar_t* strSubKey,wchar_t* pDeleteItem)
{
	CRegKey theKey;
	int i=0;
	wchar_t strName[MAX_PATH]=L"\0";
	DWORD dwLen = MAX_PATH;
	DWORD dwType=0;
	wchar_t pValue[512]={0};
	DWORD dwValueLen=1024;
	std::vector<FilePair> vecDelete;

	wchar_t strDeleteItem[512]=L"\0";
	wcsncpy_s(strDeleteItem,512,pDeleteItem, _TRUNCATE);
	_wcslwr_s(strDeleteItem,512);

	LONG lRet = theKey.Open(HKEY_USERS,strSubKey);
	if(lRet == ERROR_SUCCESS)
	{
		while(true)
		{
			lRet = RegEnumValueW(theKey.m_hKey,i++,strName,&dwLen,NULL,&dwType,(LPBYTE)pValue,&dwValueLen);
			if(lRet != ERROR_SUCCESS)	break;

			{
				// guess the disable value's format as below:
				wchar_t* strValue = pValue+6;
				_wcslwr_s(strValue,wcslen(strValue)+1);
				if(wcsstr(strValue,strDeleteItem) != NULL)
				{
					vecDelete.push_back(FilePair(strSubKey,std::wstring(strName)));
				}
				else
				{
					int nRet = FindBinary((PBYTE)pValue,dwValueLen,(PBYTE)strDeleteItem,(DWORD)wcslen(pDeleteItem)*sizeof(wchar_t));
					if(nRet != 0)	vecDelete.push_back(FilePair(strSubKey,std::wstring(strName)));
				}
			}
		}
		theKey.Close();

		size_t nsize = vecDelete.size();
		if(nsize<1)	return 0;
		lRet = theKey.Open(HKEY_USERS,vecDelete[0].first.c_str());
		if(lRet == ERROR_SUCCESS)
		{
			for(unsigned int ik=0;ik<nsize;ik++)
			{
				lRet = theKey.DeleteValue(vecDelete[ik].second.c_str());
			}
		}
	}
	return 0;
}

#if 0
static void EnablePlugin(PVOID lParam)
{
	StructParam* pFlag =(StructParam*)lParam;
	while(true)
	{
		DWORD dw = WaitForSingleObject(pFlag->hExit,500);
		if(dw == WAIT_OBJECT_0)
		{
			break;
		}
		else if(dw == WAIT_TIMEOUT)
		{
			CRegKey theKey;
			LONG lRet=theKey.Open(HKEY_USERS,NULL);
			if(lRet != ERROR_SUCCESS)	return ;
			int i=0;
			wchar_t strName[MAX_PATH]=L"\0";
			DWORD dwLen = MAX_PATH;

			while(true)
			{
				if(sizeof(void*)==8)
				{
					dwLen = MAX_PATH;
					lRet = theKey.EnumKey(i++,strName,&dwLen);
					if(lRet != ERROR_SUCCESS)	break;
					std::wstring strwname = strName;
					strwname += L"\\Software\\Microsoft\\Office\\11.0\\Outlook\\Resiliency\\DisabledItems";
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					DeleteItem(strwname.c_str(),L"mso2k3PEP.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\11.0\\Outlook\\Resiliency\\StartupItems";
					DeleteItem(strwname.c_str(),L"mso2k3PEP.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\12.0\\Outlook\\Resiliency\\DisabledItems";
					DeleteItem(strwname.c_str(),L"mso2k7PEP.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\12.0\\Outlook\\Resiliency\\StartupItems";
					DeleteItem(strwname.c_str(),L"mso2k7PEP.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
				}
				else
				{
					dwLen = MAX_PATH;
					lRet = theKey.EnumKey(i++,strName,&dwLen);
					if(lRet != ERROR_SUCCESS)	break;
					std::wstring strwname = strName;
					strwname += L"\\Software\\Microsoft\\Office\\11.0\\Outlook\\Resiliency\\DisabledItems";
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					DeleteItem(strwname.c_str(),L"mso2k3PEP32.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\11.0\\Outlook\\Resiliency\\StartupItems";
					DeleteItem(strwname.c_str(),L"mso2k3PEP32.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\12.0\\Outlook\\Resiliency\\DisabledItems";
					DeleteItem(strwname.c_str(),L"mso2k7PEP32.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
					strwname = strName;
					strwname +=L"\\Software\\Microsoft\\Office\\12.0\\Outlook\\Resiliency\\StartupItems";
					DeleteItem(strwname.c_str(),L"mso2k7PEP32.dll");
					//DeleteItem(strwname.c_str(),L"outlcrashpep.dll");
				}
			}
			theKey.Close();
		}
	}
	_endthreadex(0);
}
#endif 
//////////////////////////////////////////////////////////////////////////

#ifdef _MANAGED
#pragma managed(pop)
#endif

