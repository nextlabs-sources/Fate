#pragma once
#include <Windows.h>
#include <vector>
#include "CEsdk.h"

#define TIMEOUT_TIME                    50000

class PolicyCommunicator
{
public:
    virtual ~PolicyCommunicator();   

    PolicyCommunicator();

    BOOL EvaluateApplication(LPCWSTR srcApp);
    BOOL Connect2PolicyServer();

protected:
    void  InitData();
    void  GetUserInfo(LPWSTR wzSid, int nSize, LPWSTR UserName, int UserNameLen);
    DWORD GetIp();
    void Disconnect2PolicyServer();

private:
    WCHAR    m_wzSID[128];
    WCHAR    m_wzUserName[128];
    WCHAR    m_wzHostName[128];
    WCHAR    m_wzAppName[MAX_PATH];
    WCHAR    m_wzAppPath[MAX_PATH];
    CEHandle m_connectHandle;
    ULONG    m_ulIp;
};

bool EvaluateApp( LPCWSTR srcApp );
