

#include <Windows.h>
#include <assert.h>
#include <Sddl.h>
#define SECURITY_WIN32
#include <security.h>

#include <nudf\exception.hpp>
#include <nudf\host.hpp>
#include <nudf\user.hpp>


using namespace nudf::win;




//
//  class CUser
//

CUser::CUser()
{
}

CUser::~CUser()
{
}

VOID CUser::GetProcessUser(_In_ HANDLE hProcess)
{
    HANDLE hToken = NULL;
    if(!OpenProcessToken(hProcess, TOKEN_READ, &hToken)) {
        throw WIN32ERROR();
    }

    try {
        GetInfoByToken(hToken);
        CloseHandle(hToken);
        hToken = NULL;
    }
    catch(const nudf::CException& e) {
        CloseHandle(hToken);
        hToken = NULL;
        throw e;
    }
}

VOID CUser::GetInfoByToken(_In_ HANDLE hToken)
{
    std::vector<unsigned char> buf;
    PTOKEN_USER UserInfo = NULL;
    DWORD       ReturnLength = 0;

    Clear();
    
    GetTokenInformation(hToken, TokenUser, NULL, 0, &ReturnLength);
    if(0 == ReturnLength) {
        throw WIN32ERROR();
    }

    ReturnLength += sizeof(TOKEN_USER);
    buf.resize(ReturnLength, 0);
    //UserInfo = (PTOKEN_USER)(buf.data());
	UserInfo = (PTOKEN_USER)(&(*buf.begin()));

    try {

        if(!GetTokenInformation(hToken, TokenUser, UserInfo, ReturnLength, &ReturnLength)) {
            throw WIN32ERROR();
        }
        GetInfoBySid(UserInfo->User.Sid);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

VOID CUser::GetInfoBySid(_In_ PSID pSid)
{
    WCHAR   wzUserName[MAX_USER_NAME] = {0};
    WCHAR   wzDomainName[MAX_USER_NAME] = {0};
    DWORD   dwUserName = MAX_USER_NAME;
    DWORD   dwDomainName = MAX_USER_NAME;
    SID_NAME_USE sidNameUse;

    Clear();

    _Sid = pSid;
    if(!_Sid.IsValid()) {
        SetLastError(ERROR_INVALID_SID);
        throw WIN32ERROR();
    }

    if (!LookupAccountSidW(NULL, pSid, wzUserName, &dwUserName, wzDomainName, &dwDomainName, &sidNameUse)) {
        _Sid.Clear();
        throw WIN32ERROR();
    }

    nudf::win::CHost host;

    _Name = wzUserName;
    _Domain = wzDomainName;
    if(0 == _Domain.length()) {
        // This is not in domain
        _AccountName = host.GetHostName();
        _AccountName += L"\\";
        _AccountName += _Name;
        _PrincipleName = _Name;
    }
    else {

        if(0 == _wcsicmp(_Domain.c_str(), L"BUILTIN")) {
            // This is not in domain
            _AccountName = L"BUILTIN\\";
            _AccountName += _Name;
            _PrincipleName = _Name;
        }
        else if(0 == _wcsicmp(_Domain.c_str(), host.GetHostName().c_str())) {
            // This is not in domain
            _AccountName = host.GetHostName();
            _AccountName += L"\\";
            _AccountName += _Name;
            _PrincipleName = _Name;
        }
        else {
            // This is in domain
            WCHAR wzPrincipleName[MAX_PATH] = {0};
            DWORD dwPrincipleName = MAX_PATH;
            _AccountName = _Domain;
            _AccountName += L"\\";
            _AccountName += _Name;
            if (::TranslateNameW(_AccountName.c_str(), NameSamCompatible, NameUserPrincipal, wzPrincipleName, &dwPrincipleName)) {
                _PrincipleName = wzPrincipleName;
            }
        }
    }
}

CUser& CUser::operator = (_In_ const CUser& u) throw()
{
    if(this != &u) {
        _Name = u.GetName();
        _Domain = u.GetDomain();
        _AccountName = u.GetAccountName();
        _PrincipleName = u.GetPrincipleName();
        _Sid = u.GetSid();
    }
    return *this;
}

CUser& CUser::operator = (_In_ PSID pSid) throw()
{
    try {
        GetInfoBySid(pSid);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
    return *this;
}

BOOL CUser::operator == (const CUser& u) const throw()
{
    return (_Sid == u.GetSid()) ? TRUE : FALSE;
}

BOOL CUser::IsValid() const throw()
{
    return _Sid.IsValid();
}

VOID CUser::Clear() throw()
{
    _Name.clear();
    _Domain.clear();
    _AccountName.clear();
    _PrincipleName.clear();
    _Sid.Clear();
}


//
//
//

CImpersonate::CImpersonate() : _impersonated(false)
{
}

CImpersonate::~CImpersonate()
{
    Revert();
}

bool CImpersonate::Impersonate(_In_ HANDLE hToken)
{
    if(_impersonated) {
        return false;
    }
    if(NULL == hToken) {
        return false;
    }
    
    if(ImpersonateLoggedOnUser(hToken)) {
        _impersonated = true;
    }

    return _impersonated;
}

void CImpersonate::Revert()
{
    if(_impersonated) {
        RevertToSelf();
        _impersonated = false;
    }
}
