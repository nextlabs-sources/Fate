

#ifndef __NUDF_SESSION_HPP__
#define __NUDF_SESSION_HPP__

#include <Wtsapi32.h>
#include <string>
#include <nudf\user.hpp>

namespace nudf
{
namespace win
{


class CSession
{
public:
    CSession();
    CSession(_In_ ULONG dwSessionId);
    virtual ~CSession();


    BOOL IsActive() throw();
    BOOL IsConnected() throw();
    BOOL IsDisconnected() throw();
    BOOL IsIdle() throw();
    BOOL IsConsole() throw();
    BOOL IsRdp() throw();
    BOOL GetInitialProgram(_Out_ std::wstring& wsInitProgram) throw();
    BOOL GetLogonUserName(_Out_ std::wstring& wsUserName, _Out_ std::wstring& wsDomainName) throw();
    BOOL GetWinStationName(_Out_ std::wstring& wsWinStationName) throw();
    BOOL GetClientName(_Out_ std::wstring& wsClientName) throw();
    BOOL GetClientAddress(_Out_ PWTS_CLIENT_ADDRESS wtsClientAddress) throw();
    BOOL GetConnState(_Out_ WTS_CONNECTSTATE_CLASS* wtsConnState) throw();
    BOOL GetClientProtocol(_Out_ USHORT* Proto) throw();

    BOOL GetUser(_Out_ CUser& u) throw();

    CSession& operator =(const CSession& s) throw();
    CSession& operator =(ULONG dwSessionId) throw();
    BOOL operator ==(const CSession& s) const throw();
    BOOL operator ==(ULONG dwSessionId) const throw();
    HANDLE GetToken() const throw();

    inline BOOL IsValid() const throw() {return (-1 != _SessionId);}
    inline DWORD GetSessionId() const throw() {return _SessionId;}
    inline VOID SetSessionId(_In_ DWORD dwSessionId) throw() {_SessionId = dwSessionId;}

private:
    ULONG   _SessionId;
};




}   // namespace nudf::win
}   // namespace nudf



#endif  // #ifndef __NUDF_SESSION_HPP__