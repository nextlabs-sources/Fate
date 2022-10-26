

#include <Windows.h>
#include <assert.h>
#include <Sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#define SECURITY_WIN32
#include <security.h>

#include <nudf\exception.hpp>
#include <nudf\sa.hpp>


using namespace nudf::win;




//
//  class CSid
//


CSid::CSid() : _Sid(NULL)
{
}

CSid::CSid(_In_ PSID Sid) throw() : _Sid(NULL)
{
    *this = Sid;
}

CSid::CSid(_In_ LPCWSTR Sid) throw() : _Sid(NULL)
{
    *this = Sid;
}

CSid::~CSid() throw()
{
    if(NULL != _Sid) {
        LocalFree(_Sid);
        _Sid = NULL;
    }
}

CSid& CSid::operator = (_In_ const CSid& Sid) throw()
{
    if(this != &Sid) {
        *this = Sid.GetSidStr().c_str();
    }
    return *this;
}

CSid& CSid::operator = (_In_opt_ PSID Sid) throw()
{
    Clear();

    if(NULL != Sid) {

        if(IsValidSid(Sid)) {

            LPWSTR pwzSid = NULL;
            DWORD dwLength = GetLengthSid(Sid);
            assert(0 != dwLength);

            if(ConvertSidToStringSidW(Sid, &pwzSid)) {
                _SidStr = pwzSid;
                LocalFree(pwzSid);
                pwzSid = NULL;
            }

            _Sid = LocalAlloc(0, dwLength);
            if(NULL != _Sid) {
                memset(_Sid, 0, dwLength);
                (VOID)CopySid(dwLength, _Sid, Sid);
            }
        }
    }

    return *this;
}

CSid& CSid::operator = (_In_ LPCWSTR Sid) throw()
{
    Clear();

    if(ConvertStringSidToSidW(Sid, &_Sid)) {
        _SidStr = Sid;
    }

    return *this;
}

BOOL CSid::operator == (_In_ const CSid& Sid) const throw()
{
    return (IsValid() && Sid.IsValid()) ? EqualSid(_Sid, Sid) : FALSE;
}

BOOL CSid::operator == (_In_ PSID Sid) const throw()
{
    return (Sid != NULL && IsValid()) ? EqualSid(_Sid, Sid) : FALSE;
}

BOOL CSid::operator == (_In_ LPCWSTR Sid) const throw()
{
    return (Sid != NULL && L'\0' != Sid[0] &&  IsValid()) ? (0 == _wcsicmp(Sid, _SidStr.c_str())) : FALSE;
}

CSid::operator LPCWSTR() const throw()
{
    return _SidStr.c_str();
}

CSid::operator PSID() const throw()
{
    return _Sid;
}

VOID CSid::Clear() throw()
{
    if(NULL != _Sid) {
        LocalFree(_Sid);
        _Sid = NULL;
    }
    _SidStr = L"";
}

BOOL CSid::IsValid() const throw()
{
    return (NULL != _Sid) ? IsValidSid(_Sid) : FALSE;
}

CSidEveryone::CSidEveryone()
{
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    if(!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, GetSidPtr())) {
        (*GetSidPtr()) = NULL;
    }
}

CSidEveryone::~CSidEveryone()
{
    if(NULL != GetSid()) {
        FreeSid(GetSid());
        *GetSidPtr() = NULL;
    }
}

CSidAdmin::CSidAdmin()
{
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    if(!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, GetSidPtr())) {
        (*GetSidPtr()) = NULL;
    }
}

CSidAdmin::~CSidAdmin()
{
    if(NULL != GetSid()) {
        FreeSid(GetSid());
        *GetSidPtr() = NULL;
    }
}


//
//  class CSecurityDescriptor
//

CSecurityDescriptor::CSecurityDescriptor() : _pSD(NULL)
{
}

CSecurityDescriptor::~CSecurityDescriptor()
{
}

//
//  class CSecurityAttributes
//

CSecurityAttributes::CSecurityAttributes()
{
    _sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    _sa.lpSecurityDescriptor = NULL;
    _sa.bInheritHandle = FALSE;
}

CSecurityAttributes::~CSecurityAttributes()
{
}


CEveryoneSA::CEveryoneSA() : CSecurityAttributes(), _sd(NULL), _sid(NULL), _acl(NULL)
{
    Init();
}

CEveryoneSA::~CEveryoneSA()
{
    if(NULL != _sid) {
        FreeSid(_sid);
        _sid = NULL;
    }
    if(NULL != _acl) {
        LocalFree(_acl);
        _acl = NULL;
    }
    if(NULL != _sd) {
        LocalFree(_sd);
        _sd = NULL;
    }
}

void CEveryoneSA::Init()
{
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    
    if(!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &_sid)) {
        return;
    }

    ZeroMemory(&_ea, sizeof(EXPLICIT_ACCESS));
    _ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    _ea.grfAccessMode        = SET_ACCESS;
    _ea.grfInheritance       = NO_INHERITANCE;
    _ea.Trustee.TrusteeForm  = TRUSTEE_IS_SID;
    _ea.Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
    _ea.Trustee.ptstrName    = (LPWSTR)_sid;
    
    if(0 != SetEntriesInAclW(1, &_ea, NULL, &_acl)) {
        FreeSid(_sid);
        _sid = NULL;
        return;
    }

    _sd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if(NULL == _sd) {
        FreeSid(_sid);
        _sid = NULL;
        LocalFree(_acl);
        _acl = NULL;
        return;
    }
    
    if(!InitializeSecurityDescriptor(_sd, SECURITY_DESCRIPTOR_REVISION)) {
        FreeSid(_sid);
        _sid = NULL;
        LocalFree(_acl);
        _acl = NULL;
        LocalFree(_sd);
        _sd = NULL;
        return;
    }
    if(!SetSecurityDescriptorDacl(_sd, TRUE, _acl, FALSE)) {
        FreeSid(_sid);
        _sid = NULL;
        LocalFree(_acl);
        _acl = NULL;
        LocalFree(_sd);
        _sd = NULL;
        return;
    }

    // Succeed
    _sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    _sa.lpSecurityDescriptor = _sd;
    _sa.bInheritHandle = FALSE;
}