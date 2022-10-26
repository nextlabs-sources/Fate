

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x501

#include <windows.h>
#include <Lmshare.h>
#include <Lm.h>
#include <WtsApi32.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

typedef
DWORD
(WINAPI * Kernel32_WTSGetActiveConsoleSessionId)();

#define EXPORT
EXPORT DWORD GetRdpSessionAddress();

/************************************************************************/
/* LOCAL ROUTINES                                                       */
/************************************************************************/
static BOOL  GetConsolSessionId(PDWORD pdwSessionId);
static BOOL  GetSessionProtoType(DWORD dwSessionId, USHORT* pusProtoType);
static BOOL  GetSessionRemoteIp(DWORD dwSessionId, PDWORD pdwIp);

/************************************************************************/
/*                                                                      */
/************************************************************************/
DWORD GetRdpSessionAddress()
{
    DWORD   dwSessionId = static_cast<DWORD>(-1);
    USHORT  usProtoType = 0;
    DWORD   dwIp        = 0;

    static DWORD dwIpStart = ((192<<24)&0xFF000000) | ((168<<16)&0xFF0000) | ((187<<8)&0xFF00) | 80;
    static DWORD dwIpEnd   = ((192<<24)&0xFF000000) | ((168<<16)&0xFF0000) | ((187<<8)&0xFF00) | 85;

    if(!GetConsolSessionId(&dwSessionId)) {
        return 0;
    }
    if(!GetSessionProtoType(dwSessionId, &usProtoType)) {
        return 0;
    }
    if(WTS_PROTOCOL_TYPE_RDP != usProtoType) {
        return 0;
    }
    if(!GetSessionRemoteIp(dwSessionId, &dwIp)) {
        return 0;
    }
    if(dwIp>=dwIpStart && dwIp<=dwIpEnd) {
        return 0;
    }

    return dwIp;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
BOOL GetConsolSessionId(PDWORD pdwSessionId)
{
    static HMODULE hKernel32 = LoadLibraryA("Kernel32.dll");
    if (hKernel32 == NULL )
		return FALSE;
    static Kernel32_WTSGetActiveConsoleSessionId _WTSGetActiveConsoleSessionId = (Kernel32_WTSGetActiveConsoleSessionId)GetProcAddress( hKernel32,"WTSGetActiveConsoleSessionId");


    if(ProcessIdToSessionId( GetCurrentProcessId(), pdwSessionId))
    {
        if(_WTSGetActiveConsoleSessionId)   // WinXPp/win2k3
        {
            if(*pdwSessionId == _WTSGetActiveConsoleSessionId()) return FALSE;
        }
        else                                // Win2k
        {
            if(*pdwSessionId == 0) return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL GetSessionProtoType(DWORD dwSessionId, USHORT* pusProtoType)
{
    LPTSTR pBuffer = NULL;
    DWORD  BytesReturned;
    BOOL   Ret;
    Ret = WTSQuerySessionInformation(
                                     WTS_CURRENT_SERVER_HANDLE,
                                     dwSessionId,
                                     WTSClientProtocolType,
                                     &pBuffer,
                                     &BytesReturned
                                    );
    if(Ret && pBuffer)
    {
        *pusProtoType = *((USHORT*)(pBuffer));
        WTSFreeMemory(pBuffer);
        return TRUE;
    }

    return FALSE;
}

BOOL GetSessionRemoteIp(DWORD dwSessionId, PDWORD pdwIp)
{
    LPTSTR pBuffer = NULL;
    DWORD  BytesReturned;
    BOOL   Ret;

    Ret = WTSQuerySessionInformation(
                                      WTS_CURRENT_SERVER_HANDLE,
                                      dwSessionId,
                                      WTSClientAddress,
                                      &pBuffer,
                                      &BytesReturned
                                    );
    if(Ret && pBuffer)
    {
        PWTS_CLIENT_ADDRESS Address = (PWTS_CLIENT_ADDRESS)pBuffer;
        *pdwIp   = ((Address->Address[2]<<24)&0xFF000000) | ((Address->Address[3]<<16)&0xFF0000) | ((Address->Address[4]<<8)&0xFF00) | Address->Address[5];
        WTSFreeMemory(pBuffer); pBuffer = NULL;
        return TRUE;
    }

    return FALSE;
}