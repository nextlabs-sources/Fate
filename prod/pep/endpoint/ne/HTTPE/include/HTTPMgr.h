#pragma once


enum HTTP_ERROR
{
	HTTP_REDIRECT	= 1000,
	HTTP_HEADER_INJECTION = 1001,
	HTTP_NAVIGATION_DENIED = 1002,
	HTTP_UPLOAD_DENIED = 1003,
	HTTP_HTTPS = 1004
};

class CHttpMgr
{
protected:
	CHttpMgr();
	~CHttpMgr();
public:
	static CHttpMgr& GetInstance();

	DWORD ProcessHTTPData(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, unsigned uType = 0 );//return value: 0 means "it is allowed", 1 means "it is denied". uType: 0 means "send", 1 means "recv".
	DWORD ProcessHTTPData2(SOCKET s, const string & strData, bool bHttps, unsigned uType = 0);//return value: 0 means "it is allowed", 1 means "it is denied", 2 means "it is an invalid data packet". uType: 0 means "send", 1 means "recv".
	
	/*
	Change the receive data for the redirect obligation 
	*/
	BOOL ProcessRedirctObligation( SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd ) ;
	/*
	/*
	Change the receive data for the redirect obligation 
	*/
	BOOL ProcessRedirctObligation( const SOCKET s, PVOID pDecryptData, const DWORD& dBufferLen ) ;
	BOOL ProcessRedirctObligation( const SOCKET s, char* pDecryptData,  int * dBufferLen ) ;
	/*
	change the send data for the HTTP Header Injection
	*/
	BOOL ProcessHeaderInjectionObligation( SOCKET s, LPWSABUF lpBuffers, LPDWORD InjectedDataLen ) ;

private:
	/*
	function:
	check if the tcp data belong to a response of a GET or POST request,
	if it is, and if the GET or POST request is denied by HTTPE,
	we will replace the tcp data in \c lpBuffers with a fake 401 http response

	return value:
	0, means we did not do any thing.
	*/
	DWORD ProcessDeniedEval( SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd );
};