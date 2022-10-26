

#ifndef __NUDF_REGISTRY_HPP__
#define __NUDF_REGISTRY_HPP__

#include <string>
#include <vector>

namespace nudf
{
namespace win
{


class CRegKey
{
public:
    CRegKey();
    virtual ~CRegKey();

    virtual bool Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw();
    virtual void Close() throw();
    
    bool GetValue(_In_opt_ LPCWSTR name, _Out_ unsigned long* v) throw();
    bool GetValue(_In_opt_ LPCWSTR name, _Out_ unsigned __int64* v) throw();
    bool GetValue(_In_opt_ LPCWSTR name, _Out_ std::wstring& v, _Out_opt_ bool* expandable) throw();
    bool GetValue(_In_opt_ LPCWSTR name, _Out_ std::vector<unsigned char>& v) throw();
    bool GetValue(_In_opt_ LPCWSTR name, _Out_ std::vector<std::wstring>& v) throw(); 
    
    bool SetValue(_In_opt_ LPCWSTR name, _In_ unsigned long v) throw();
    bool SetValue(_In_opt_ LPCWSTR name, _In_ unsigned __int64 v) throw();
    bool SetValue(_In_opt_ LPCWSTR name, _In_ const std::wstring& v, _In_ bool expandable=false) throw();
    bool SetValue(_In_opt_ LPCWSTR name, _In_ const std::vector<unsigned char>& v) throw();
    bool SetValue(_In_opt_ LPCWSTR name, _In_ const std::vector<std::wstring>& v) throw();

    bool ValueExists(_In_opt_ LPCWSTR name, _In_opt_ ULONG* type);
    bool SubKeyExists(_In_ LPCWSTR name);
    bool DeleteValue(_In_opt_ LPCWSTR name) throw();
    bool DeleteSubKey(_In_ LPCWSTR name) throw();

    inline bool IsCreated() const throw() {return _created;}
    inline bool IsExisting() const throw() {return (!_created);}

    inline operator HKEY() throw() {return _key;}

protected:
    bool GetValueEx(_In_ LPCWSTR name, _Out_ ULONG* type, _Out_ std::vector<unsigned char>& v) throw();
    bool SetValueEx(_In_ LPCWSTR name, _In_ ULONG type, _In_ const BYTE* data, _In_ ULONG size) throw();


protected:
    HKEY            _key;
    bool            _created;
};

class CRegLocalMachine : public CRegKey
{
public:
    CRegLocalMachine();
    virtual ~CRegLocalMachine();

private:
    virtual bool Open(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool Create(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool CreateEx(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key=FALSE) throw();
    virtual void Close() throw();
};

class CRegCurrentUser : public CRegKey
{
public:
    CRegCurrentUser();
    virtual ~CRegCurrentUser();

private:
    virtual bool Open(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool Create(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool CreateEx(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key=FALSE) throw();
    virtual void Close() throw();
};

class CRegUsers : public CRegKey
{
public:
    CRegUsers();
    virtual ~CRegUsers();

private:
    virtual bool Open(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool Create(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool CreateEx(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key=FALSE) throw();
    virtual void Close() throw();
};

class CRegClassesRoot : public CRegKey
{
public:
    CRegClassesRoot();
    virtual ~CRegClassesRoot();

private:
    virtual bool Open(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool Create(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw();
    virtual bool CreateEx(_In_ HKEY hRoot, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key=FALSE) throw();
    virtual void Close() throw();
};




}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_REGISTRY_HPP__