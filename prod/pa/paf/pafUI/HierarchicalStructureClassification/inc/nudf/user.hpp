

#ifndef __NUDF_USER_HPP__
#define __NUDF_USER_HPP__

#include <nudf\sa.hpp>
#include <string>

namespace nudf
{
namespace win
{

#define MAX_USER_NAME       64
#define MAX_DOMAIN_NAME     64



class CUser
{
public:
    CUser();
    virtual ~CUser();

    VOID GetProcessUser(_In_ HANDLE hProcess);
    VOID GetInfoByToken(_In_ HANDLE hToken);
    VOID GetInfoBySid(_In_ PSID pSid);

    CUser& operator = (_In_ const CUser& u) throw();
    CUser& operator = (_In_ PSID pSid) throw();
    BOOL operator == (const CUser& u) const throw();

    BOOL IsValid() const throw();
    VOID Clear() throw();

    inline const CSid& GetSid() const throw() {return _Sid;}
    inline const std::wstring& GetName() const {return _Name;}
    inline const std::wstring& GetDomain() const {return _Domain;}
    inline const std::wstring& GetAccountName() const {return _AccountName;}
    inline const std::wstring& GetPrincipleName() const {return _PrincipleName;}

private:
    std::wstring    _Name;
    std::wstring    _Domain;
    std::wstring    _AccountName;
    std::wstring    _PrincipleName;
    CSid            _Sid;
};


class CImpersonate
{
public:
    CImpersonate();
    virtual ~CImpersonate();

    bool Impersonate(_In_ HANDLE hToken);
    void Revert();

private:
    bool    _impersonated;
};



}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_USER_HPP__