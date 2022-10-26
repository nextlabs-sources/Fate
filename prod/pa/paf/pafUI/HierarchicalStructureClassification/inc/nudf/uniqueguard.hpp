

#ifndef __NUDF_UNIQUEGUARD_HPP__
#define __NUDF_UNIQUEGUARD_HPP__

#include <Windows.h>

namespace nudf
{
namespace util
{


#define NUDF_UNIQUEGUARD_NAME   L"__nudf::util::uniqueguard__"

class CUniqueGuard
{
public:
    CUniqueGuard() : m_hMutex(NULL), m_dwErr(0)
    {
        m_hMutex = ::CreateMutexW(NULL, FALSE, NUDF_UNIQUEGUARD_NAME);
        m_dwErr = GetLastError();
    }

    CUniqueGuard(_In_ LPCWSTR Name) : m_hMutex(NULL), m_dwErr(0)
    {
        m_hMutex = ::CreateMutexW(NULL, FALSE, Name?Name:NUDF_UNIQUEGUARD_NAME);
        m_dwErr = GetLastError();
    }

    virtual ~CUniqueGuard()
    {
        if(NULL != m_hMutex) {
            CloseHandle(m_hMutex);
            m_hMutex = NULL;
        }
    }

    inline BOOL IsValid() const throw() {return (NULL != m_hMutex);}
    inline BOOL IsUnique() const throw() {return (ERROR_ALREADY_EXISTS != m_dwErr);}
    inline DWORD GetError() const throw() {return m_dwErr;}

private:
    HANDLE  m_hMutex;
    DWORD   m_dwErr;
};


}   // namespace nudf::util
}   // namespace nudf


#endif  // #ifndef __NUDF_UNIQUEGUARD_HPP__