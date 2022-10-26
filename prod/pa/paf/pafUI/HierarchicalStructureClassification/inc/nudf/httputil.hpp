

#ifndef __NUDF_HTTP_UTIL_HPP__
#define __NUDF_HTTP_UTIL_HPP__



#include <Winhttp.h>
#include <string>
#include <vector>

namespace nudf {
namespace http {



class CHandle
{
public:
    CHandle();
    virtual ~CHandle();

    inline operator HINTERNET() const throw() {return _handle;}
    inline HINTERNET GetHandle() const throw() {return _handle;}

    void Attach(_In_ HINTERNET h) throw();
    HINTERNET Detach() throw();
    virtual void Close() throw();
    HRESULT SetOption(_In_ DWORD option, _In_ const void* value, _In_ DWORD length) throw();
    HRESULT QueryOption(_In_ DWORD option, _Out_ void* value, _Inout_ DWORD& length) const throw();

protected:
    inline bool IsHandleValid() const throw() {return (NULL != _handle);}

private:
    HINTERNET   _handle;
};

class CSession : public CHandle
{
public:
    CSession();
    CSession(_In_ const std::wstring& agent);
    virtual ~CSession();

    virtual HRESULT Open(_In_ bool async=false);
    virtual void Close() throw();

    inline const std::wstring& GetAgentName() const throw() {return _agent;}
    inline void SetAgentName(const std::wstring& agent) throw() {_agent=agent;}
    inline bool IsAsync() const throw() {return _async;}

private:
    std::wstring _agent;
    bool         _async;
};

class CConnection : public CHandle
{
public:
    CConnection();
    CConnection(_In_ const std::wstring& server, _In_ INTERNET_PORT port=INTERNET_DEFAULT_PORT);
    virtual ~CConnection();

    virtual HRESULT Connect(_In_ const CSession& session) throw();

    inline const std::wstring& GetServer() const throw() {return _server;}
    inline void SetServer(_In_ const std::wstring& server) throw() {_server=server;}
    inline INTERNET_PORT GetConnPort() const throw() {return _port;}
    inline void SetConnPort(_In_ INTERNET_PORT port) throw() {_port=port;}


private:
    std::wstring    _server;
    INTERNET_PORT   _port;
};

class CResponse
{
public:
    CResponse(){}
    virtual ~CResponse(){}

    HRESULT GetResponse(_In_ HINTERNET hRequest);
    void Clear();

    inline const std::vector<std::wstring>& GetHeaders() const throw() {return _headers;}
    inline const std::string& GetContent() const throw() {return _content;}

protected:
    HRESULT GetHeaders(_In_ HINTERNET hRequest);
    HRESULT GetContent(_In_ HINTERNET hRequest);

private:
    std::vector<std::wstring>   _headers;
    std::string                 _content;
};

class CRequest : public CHandle
{
public:
    CRequest();
    CRequest(_In_ const std::wstring& verb);
    CRequest(_In_ const std::wstring& verb, _In_ const std::wstring& path);
    virtual ~CRequest();
    
    inline const std::wstring& GetPath() const throw() {return _path;}
    inline const std::wstring& GetVerb() const throw() {return _verb;}
    inline void SetPath(_In_ const std::wstring& path) throw() {_path = path;}
    inline void SetSecure(_In_ bool secure) throw() {_secure = secure;}
    inline bool GetSecure() const throw() {return _secure;}

    virtual HRESULT Initialize(_In_ const CSession& session, _In_ const std::wstring& server, _In_ INTERNET_PORT port) throw();
    virtual HRESULT Reset();
    virtual void Close() throw();

    HRESULT AddHeaders(_In_ LPCWSTR headers) throw();
    HRESULT MergeHeaders(_In_ LPCWSTR headers) throw();
    HRESULT MergeHeadersByComma(_In_ LPCWSTR headers) throw();
    HRESULT MergeHeadersBySemicolon(_In_ LPCWSTR headers) throw();
    HRESULT RemoveHeader(_In_ LPCWSTR names) throw();
    HRESULT ReplaceHeaders(_In_ LPCWSTR headers) throw();

    virtual HRESULT Send(_In_ DWORD data_size = 0) throw();
    virtual HRESULT Send(_In_ PVOID data, _In_ DWORD data_size) throw();
    HRESULT GetResponse(_Out_ CResponse& response);

public:
    void OnCallback(_In_ DWORD code, _In_ void* info, _In_ DWORD length);

protected:
    //
    //  Virtual Callback Functions
    //
    virtual void OnClosingConnection() throw() {}
    virtual void OnConnectedToServer(_In_ LPCWSTR server_ip) throw() {UNREFERENCED_PARAMETER(server_ip);}
    virtual void OnConnectingToServer(_In_ LPCWSTR server_ip) throw() {UNREFERENCED_PARAMETER(server_ip);}
    virtual void OnConnectionClosed() throw() {}
    virtual void OnDataAvailable(_In_ DWORD size) throw() {UNREFERENCED_PARAMETER(size);}
    virtual void OnHandleCreated(_In_ HINTERNET h) throw() {UNREFERENCED_PARAMETER(h);}
    virtual void OnHandleClosing(_In_ HINTERNET h) throw() {UNREFERENCED_PARAMETER(h);}
    virtual void OnHeaderAvailable() throw() {}
    virtual void OnIntermediateResponse(_In_ DWORD status) throw() {UNREFERENCED_PARAMETER(status);}
    virtual void OnNameResolved(_In_ LPCWSTR name) throw() {UNREFERENCED_PARAMETER(name);}
    virtual void OnReadComplete(_In_ PUCHAR data, _In_ DWORD size) throw() {UNREFERENCED_PARAMETER(data);UNREFERENCED_PARAMETER(size);}
    virtual void OnReceivingResponse() throw() {}
    virtual void OnRedirect(_In_ LPCWSTR url) throw() {UNREFERENCED_PARAMETER(url);}
    virtual void OnRequestError(_In_ const WINHTTP_ASYNC_RESULT* result) throw() {UNREFERENCED_PARAMETER(result);}
    virtual void OnRequestSent(_In_ DWORD size) throw() {UNREFERENCED_PARAMETER(size);}
    virtual void OnResolvingName(_In_ LPCWSTR name) throw() {UNREFERENCED_PARAMETER(name);}
    virtual void OnResponseReceived(_In_ DWORD size) throw() {UNREFERENCED_PARAMETER(size);}
    virtual void OnSecureFailure(_In_ DWORD code) throw() {UNREFERENCED_PARAMETER(code);}
    virtual void OnSendingRequest() throw() {}
    virtual void OnSendRequestComplete() throw() {}
    virtual void OnWriteComplete(_In_ const void* info, _In_ DWORD size) throw() {UNREFERENCED_PARAMETER(info);UNREFERENCED_PARAMETER(size);}
    virtual void OnGetProxyForUrlComplete() throw() {}
    virtual void OnCloseComplete() throw() {}
    virtual void OnShutdownComplete() throw() {}
    virtual void OnUnknownStatus(_In_ DWORD status, _In_ const void* info, _In_ DWORD size) throw() {UNREFERENCED_PARAMETER(status);UNREFERENCED_PARAMETER(info);UNREFERENCED_PARAMETER(size);}

private:
    CConnection     _conn;
    std::wstring    _path;
    std::wstring    _verb;
    bool            _secure;
    bool            _async;
};




} // namespace nudf::http
} // namespace nudf


#endif  // __NUDF_HTTP_UTIL_HPP__