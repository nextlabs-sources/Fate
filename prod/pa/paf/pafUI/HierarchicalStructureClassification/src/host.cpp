
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0600		// Change this to the appropriate value to target other versions of Windows.
#endif
#include <Windows.h>
#include <assert.h>

#include <nudf\host.hpp>


using namespace nudf::win;

CHost::CHost()
{
    WCHAR name[MAX_PATH+1] = { 0 };
    ULONG size = MAX_PATH;
    if (GetComputerNameExW(ComputerNameDnsFullyQualified, name, &size)) {
        name[size] = L'\0';
        _host = name;
    }
    name[0] = L'\0';
    GetSystemDefaultLocaleName(name, MAX_PATH);
    _locale = name;
}

CHost::~CHost()
{
}
  
const std::wstring& CHost::GetHostName() const throw()
{
    return _host;
}
  
const std::wstring& CHost::GetLocaleName() const throw()
{
    return _locale;
}

void CHost::Clear() throw()
{
    _host = L"";
    _locale = L"";
}

CHost& CHost::operator = (const CHost& host) throw()
{
    if(this != &host) {
        _host = host.GetHostName();
        _locale = host.GetLocaleName();
    }
    return *this;
}
