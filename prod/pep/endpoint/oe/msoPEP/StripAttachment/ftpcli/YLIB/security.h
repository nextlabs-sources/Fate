

#ifndef _YLIB_SECURITY_H_
#define _YLIB_SECURITY_H_
#include <vector>
#include <string>
#include "smart_ptr.h"
#if(_WIN32_WINNT < 0x0500)
#undef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#include <Sddl.h>
#include <Aclapi.h>
#else
#include <Sddl.h>
#include <Aclapi.h>
#endif
#pragma comment(lib, "Advapi32")

namespace YLIB
{
    class SecurityUtility
    {
    public:
        static std::wstring GetCurrentFullAccountName()
        {
            std::wstring strAccountName = L"";
            HANDLE hTokenHandle = 0;
            LPTSTR StringSid    = 0;
            DWORD cbInfoBuffer  = 512;
            UCHAR InfoBuffer[512]; memset(InfoBuffer, 0, sizeof(InfoBuffer));

            WCHAR   uname[64] = {0}; DWORD unamelen = 63;
            WCHAR   dname[64] = {0}; DWORD dnamelen = 63;
            SID_NAME_USE snu;

            // Prepare handle
            hTokenHandle = GetCurrentToken();
            if(0 == hTokenHandle)
                goto _exit;

            if(!GetTokenInformation(hTokenHandle, TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer))
                goto _exit;

            if(LookupAccountSid(NULL, ((PTOKEN_USER)InfoBuffer)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu))
            {
                strAccountName = dname;
                strAccountName += L"\\";
                strAccountName += uname;
            }

_exit:
            if(0!=hTokenHandle) CloseHandle(hTokenHandle);
            return strAccountName;
        }

        static PSID StringSidToSid(LPCWSTR pwzSid)
        {
            PSID    pSid = 0;
            if(ConvertStringSidToSidW(pwzSid/*L"S-1-5-21-1512634949-460182318-3030517407-1123"*/, &pSid))
            {
                DP((L"Convert SID OK (%d): \"%s\"\n", GetLastError(), pwzSid));
                return pSid;
            }
            else
            {
                DP((L"fail to convert SID (%d): \"%s\"\n", GetLastError(), pwzSid));
                return NULL;
            }
        }

        static std::wstring SidToStringSid(PSID pSid)
        {
            std::wstring strSid = L"";
            LPWSTR       pwzSid = 0;
            if (ConvertSidToStringSidW(pSid, &pwzSid) && pwzSid)
            {
                strSid = pwzSid;
                LocalFree(pwzSid);
            }
            return strSid;
        }

        static PSID GetSidByName(LPCWSTR pwzFullAccountName/*"DomainName\UserName"*/)
        {
            SID_NAME_USE   snuType;
            WCHAR          wzDomain[MAX_PATH+1];  memset(wzDomain, 0, sizeof(wzDomain));
            DWORD          cbDomain = 0;
            PSID           pSID     = NULL;
            DWORD          cbSID    = 0;
			std::wstring strSID;
            cbDomain = MAX_PATH;
            if(LookupAccountNameW(NULL, pwzFullAccountName, pSID, &cbSID, wzDomain, &cbDomain, &snuType))
            {
                goto _exit;
            }
            if (ERROR_INSUFFICIENT_BUFFER != GetLastError() || 0 == cbSID)
            {
                DP((L"LookupAccountNameW fail!\n"));
                goto _exit;
            }

            pSID = LocalAlloc(LPTR, cbSID); if(!pSID) goto _exit;
            memset(pSID, 0, cbSID);
            cbDomain = MAX_PATH;
            if(!LookupAccountNameW(NULL, pwzFullAccountName, pSID, &cbSID, wzDomain, &cbDomain, &snuType))
            {
                DP((L"LookupAccountNameW fail 2!\n"));
                LocalFree(pSID); pSID = 0;
                goto _exit;
            }

#ifdef _DEBUG
            strSID = SidToStringSid(pSID);
            DP((L"GetSidByName:: SID=%s\n", strSID.c_str()));
#endif

_exit:
            return pSID;
        }

        static PSID GetSidByName()
        {
            std::wstring   strCurrentAccountName = GetCurrentFullAccountName();
            if(0 == strCurrentAccountName.length())
                return NULL;
            else
                return GetSidByName(strCurrentAccountName.c_str());
        }

        static std::wstring GetStringSidByName(LPCWSTR pwzFullAccountName/*"DomainName\UserName"*/)
        {
            std::wstring    strSID = L"";
            PSID pSID = GetSidByName(pwzFullAccountName);
            LPWSTR pwzSID = 0;
            if(pSID && ConvertSidToStringSidW(pSID, &pwzSID))
                strSID = pwzSID;
            if(pSID) LocalFree((HLOCAL)pSID);
            if(pwzSID) LocalFree((HLOCAL)pwzSID);
            return strSID;
        }
        static std::wstring GetStringSidByName()
        {
            std::wstring   strCurrentAccountName = GetCurrentFullAccountName();
            if(0 == strCurrentAccountName.length())
                return L"";
            else
                return GetStringSidByName(strCurrentAccountName.c_str());
        }
        static void FreePSid(PSID pSid)
        {
            if(NULL != pSid) LocalFree((HLOCAL)pSid);
        }

        static BOOL SecureCopyW(LPCWSTR src, LPCWSTR dest, SECURITY_ATTRIBUTES* psa, BOOL bFailIfExists)
        {
            BOOL   bRet = FALSE;
            HANDLE hSrc=0, hDest=0;
            BYTE   cBuf[1024];
            DWORD  dwRead = 0, dwWritten=0;

            hSrc = CreateFileW(src, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            if(INVALID_HANDLE_VALUE == hSrc) goto _exit;

            hDest= CreateFileW(dest, GENERIC_WRITE, 0, psa, bFailIfExists?CREATE_NEW:CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
            if(INVALID_HANDLE_VALUE == hDest) goto _exit;

            while(ReadFile(hSrc, cBuf, 1024, &dwRead, NULL) && dwRead)
            {
                if( !WriteFile(hDest, cBuf, dwRead, &dwWritten, NULL) )
                {
                    CloseHandle(hDest); hDest = 0;
                    DeleteFileW(dest);
                    goto _exit;
                }
            }
            bRet = TRUE;

_exit:
            if(hSrc) CloseHandle(hSrc);
            if(hDest) CloseHandle(hDest);
            return bRet;
        }

    protected:
        static HANDLE GetCurrentToken()
        {
            HANDLE hTokenHandle = 0;
            if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hTokenHandle))
            {
                if(GetLastError() == ERROR_NO_TOKEN)
                {
                    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hTokenHandle ))
                        goto _exit;
                }
                else
                {
                    goto _exit;
                }
            }
_exit:
            return hTokenHandle;
        }
    };

    class AccessPermission
    {
    public:
        AccessPermission(DWORD dwPermission):m_dwPermission(dwPermission)
        {
            m_pSid = SecurityUtility::GetSidByName();
        }
        AccessPermission(LPCWSTR pwzDomain, LPCWSTR pwzUser, DWORD dwPermission):m_dwPermission(dwPermission)
        {
            std::wstring strFullName = pwzDomain;
            strFullName += L"\\";
            strFullName += pwzUser;
            m_pSid = SecurityUtility::GetSidByName(strFullName.c_str());
        }
        AccessPermission(LPCWSTR pwzSid, DWORD dwPermission):m_dwPermission(dwPermission)
        {
            m_pSid = SecurityUtility::StringSidToSid(pwzSid);
            if(!m_pSid)
            {
                DP((L"Fail to get sid from string sid\n"));
            }
        }
        virtual ~AccessPermission()
        {
            if(m_pSid) SecurityUtility::FreePSid(m_pSid);
            m_pSid = 0;
            m_dwPermission = 0;
        }
        inline PSID  get_SID(){return m_pSid;}
        inline DWORD get_Permission(){return m_dwPermission;}
    private:
        PSID    m_pSid;
        DWORD   m_dwPermission;
    };
    typedef std::vector<YLIB::COMMON::smart_ptr<AccessPermission>>    AccessPermissionList;

    class SecurityAttributesObject
    {
    public:
        SecurityAttributesObject()
        {
            memset(aclBuffer, 0, sizeof(aclBuffer));
            pAcl=(PACL)&aclBuffer;
            ::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
            ::InitializeAcl(pAcl, 1024, ACL_REVISION);
        }
        virtual ~SecurityAttributesObject()
        {
        }

        BOOL put_SecurityAttributes(AccessPermissionList& apl)
        {
            BOOL bRet = FALSE;
            ::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
            ::InitializeAcl(pAcl,   1024,   ACL_REVISION);
            for(AccessPermissionList::iterator it=apl.begin(); it!=apl.end(); ++it)
            {
                YLIB::COMMON::smart_ptr<AccessPermission> spAP = *it;
                bRet = ::AddAccessAllowedAce(pAcl, ACL_REVISION, spAP->get_Permission(), spAP->get_SID());
            }
            bRet = SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);   
            sa.nLength              =   sizeof(SECURITY_ATTRIBUTES);   
            sa.bInheritHandle       =   false;   
            sa.lpSecurityDescriptor =   &sd;
            return bRet;
        }

        SECURITY_ATTRIBUTES* get_SecurityAttributes(){return &sa;}

        BOOL SetFileDaclAttributes(LPCWSTR pwzFileName)
        {
            SECURITY_INFORMATION secInfo  = DACL_SECURITY_INFORMATION;
            //return SetFileSecurityW(pwzFileName, secInfo, &sd);
            if(SetFileSecurityW(pwzFileName, secInfo, &sd))
            {
                return TRUE;
            }
            else
            {
                DP((L"SetFileSecurityW Fail :: err=%d\n", GetLastError()));
                return FALSE;
            }
        }
    private:
        SECURITY_ATTRIBUTES   sa;       
        SECURITY_DESCRIPTOR   sd;     
        BYTE                  aclBuffer[1024];   
        PACL                  pAcl;
    };
}

#endif