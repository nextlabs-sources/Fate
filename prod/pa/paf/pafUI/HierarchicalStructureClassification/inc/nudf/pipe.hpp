

#ifndef __NUDF_PIPE_HPP__
#define __NUDF_PIPE_HPP__


#include <string>
#include <map>

namespace nudf
{
namespace ipc
{
    


#define DEFAULT_PIPE_TIMEOUT    5000
#define DEFAULT_PIPE_BUFSIZE    4096

class CPipeServer
{
public:
    CPipeServer();
    CPipeServer(_In_ LPCWSTR wzName, _In_ DWORD dwBufSize, _In_ DWORD dwTimeout);
    virtual ~CPipeServer();

    VOID Start();
    VOID Shutdown();

    inline HANDLE GetConnectEvent() throw() {return m_hConnEvent;}
    inline ULONG GetTimeout() throw() {return m_dwTimeout;}
    inline ULONG GetBufSize() throw() {return m_dwBufSize;}
    inline ULONG IsStopping() throw() {return m_fStopping;}
    inline BOOL CanEveryoneAccess() throw() {return m_fEveryone;}
    inline void AllowEveryoneAccess() throw() {m_fEveryone=TRUE;}


    VOID SetName(_In_ LPCWSTR wzName) throw();
    VOID SetTimeout(_In_ ULONG dwTimeout) throw();
    VOID SetBufferSize(_In_ ULONG dwBufSize) throw();
    
    virtual VOID OnRequest(_In_ const UCHAR* pbRequest, _In_ ULONG cbRequest, _In_ UCHAR* pbReply, _In_ ULONG cbReply, _Out_ PULONG pcbValidReply, _Out_ PBOOL pfClose) throw();
    
    DWORD WorkerRoutine();

private:
    VOID CreatePipeInstance(PHANDLE phPipe, LPOVERLAPPED lpOverlap, PBOOL pfPending);
    VOID OnClientConnect(_In_ HANDLE hPipe, _In_ LPOVERLAPPED lpOverlap, _In_ BOOL fPending) throw();

private:
    WCHAR   m_wzName[MAX_PATH];
    ULONG   m_dwBufSize;
    ULONG   m_dwTimeout;
    HANDLE  m_hMainThread;
    ULONG   m_dwMainThreadId;
    HANDLE  m_hConnEvent;
    BOOL    m_fStopping;
    BOOL    m_fEveryone;
};


class CPipeClient
{
public:
    CPipeClient();
    CPipeClient(_In_ LPCWSTR wzName);
    virtual ~CPipeClient();

    VOID Connect(_In_ ULONG WaitTime);
    VOID Disconnect() throw();
    VOID Request(_In_ const UCHAR* pbReq, _In_ ULONG cbReq, _Out_opt_ PUCHAR pbReply, _In_ ULONG cbReply, _Out_opt_ PULONG cbReturned);

private:
    WCHAR   m_wzName[MAX_PATH];
    HANDLE  m_hPipe;
};

class CAnonymousPipe
{
public:
    CAnonymousPipe();
    virtual ~CAnonymousPipe();

    VOID Create(_In_opt_ LPSECURITY_ATTRIBUTES lpPipeAttributes);
    VOID Destroy();

    inline HANDLE GetReadPipe() throw() {return _hRd;}
    inline HANDLE GetWritePipe() throw() {return _hWr;}
    inline VOID SetSize(_In_ ULONG Size) throw() {_Size = Size;}

private:
    HANDLE  _hRd;
    HANDLE  _hWr;
    ULONG   _Size;
};


}   // namespace nudf::ipc
}   // namespace nudf



#endif  // #ifndef __NUDF_PIPE_HPP__