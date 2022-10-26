

#ifndef __NUDF_SECURE_ATTRIBUTES_HPP__
#define __NUDF_SECURE_ATTRIBUTES_HPP__

#include <AccCtrl.h>

#include <string>
#include <vector>
#include <map>

namespace nudf
{
namespace win
{
  

class CSid
{
public:
    CSid();
    CSid(_In_ PSID Sid) throw();
    CSid(_In_ LPCWSTR Sid) throw();
    ~CSid() throw();

    CSid& operator = (_In_ const CSid& Sid) throw();
    CSid& operator = (_In_opt_ PSID Sid) throw();
    CSid& operator = (_In_ LPCWSTR Sid) throw();

    BOOL operator == (_In_ const CSid& Sid) const throw();
    BOOL operator == (_In_ PSID Sid) const throw();
    BOOL operator == (_In_ LPCWSTR Sid) const throw();

    operator LPCWSTR() const throw();
    operator PSID() const throw();

    VOID Clear() throw();
    BOOL IsValid() const throw();

    inline PSID* GetSidPtr() throw() {return &_Sid;}
    inline PSID GetSid() throw() {return _Sid;}
    inline const PSID GetSid() const throw() {return _Sid;}
    inline const std::wstring& GetSidStr() const throw() {return _SidStr;}

private:
    PSID         _Sid;
    std::wstring _SidStr;
};

class CSidEveryone : public CSid
{
public:
    CSidEveryone();
    ~CSidEveryone();
};

class CSidAdmin : public CSid
{
public:
    CSidAdmin();
    ~CSidAdmin();
};

class CSecurityDescriptor
{
public:
    CSecurityDescriptor();
    virtual ~CSecurityDescriptor();

    inline operator const SECURITY_DESCRIPTOR* () const throw() {return (const SECURITY_DESCRIPTOR*)_pSD;}
    inline operator SECURITY_DESCRIPTOR* () throw() {return (SECURITY_DESCRIPTOR*)_pSD;}

private:
    PSECURITY_DESCRIPTOR _pSD;
};

class CExplicitAccess
{
public:
    CExplicitAccess();
    virtual ~CExplicitAccess();

    CExplicitAccess& operator =(const CExplicitAccess& ea);
    CExplicitAccess& operator =(const EXPLICIT_ACCESS& ea);

    inline operator const EXPLICIT_ACCESS& () const throw() {return _ea;};
    inline operator EXPLICIT_ACCESS& () throw() {return _ea;};

private:
    EXPLICIT_ACCESS _ea;
};

class CExplicitAccessList
{
public:
    CExplicitAccessList();
    virtual ~CExplicitAccessList();

    VOID Clear() throw() {_EAs.clear();}
    CExplicitAccessList& operator +(const EXPLICIT_ACCESS& ea);

private:
    std::vector<EXPLICIT_ACCESS>    _EAs;
};

class CSecurityAttributes
{
public:
    CSecurityAttributes();
    virtual ~CSecurityAttributes();

    inline const SECURITY_ATTRIBUTES& GetSa() const throw() {return (_sa);}
    inline SECURITY_ATTRIBUTES& GetSa() throw() {return (_sa);}

protected:
    SECURITY_ATTRIBUTES _sa;
};

class CEveryoneSA : public CSecurityAttributes
{
public:
    CEveryoneSA();
    virtual ~CEveryoneSA();

protected:
    void Init();
    
private:
    SECURITY_ATTRIBUTES _sa;
    PSECURITY_DESCRIPTOR _sd;
    EXPLICIT_ACCESS _ea;
    PACL _acl;
    PSID _sid;
};



}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_SECURE_ATTRIBUTES_HPP__