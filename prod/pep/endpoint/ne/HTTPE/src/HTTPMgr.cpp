#include "stdafx.h"
#include "HTTPMgr.h"
#include "httpcollector.h"
#include "HttpProcessor.h"
#include "httppreproc.h"
#include "MapperMngr.h"
#include "APIHook.h"
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include "Security.h"

#define REDIRECT_HEADER "HTTP/1.1 302 Redirect\r\nContent-Type: text/html\r\nLocation: %s\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
//	this will be a fake response if we deny navigation or deny upload
#define HTTP_ERROR_CODE_RESPONSE "HTTP/1.0 401 Unauthorized\r\nContent-Length: 47\r\n\r\n<body><h2>HTTP/1.1 401 Unauthorized</h2></body>"

#define NON_HTTPS_DATA "NOTHTTPS"



extern std::map<DWORD, std::string> g_mapFireFox;
CHttpMgr::CHttpMgr()
{
	
}

CHttpMgr::~CHttpMgr()
{

}

CHttpMgr& CHttpMgr::GetInstance()
{
	static CHttpMgr mgr;
	return mgr;
}

DWORD CHttpMgr::ProcessHTTPData(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, unsigned uType /* = 0 */)//return value: 0 means "it is allowed", 1 means "it is denied". uType: 0 means "send", 1 means "recv".
{
	if(!lpBuffers)
		return 0;

	if(!CHttpPreProc::PreCheck(s))
	{
		return 0;
	}

	string sBuf;
	bool bHttps = false;
	if(uType == 0)//handle "send"
	{
		for(DWORD dwCnt = 0; dwCnt < dwBufferCount; ++dwCnt)
		{
			/************************************************************
			 There is a bug for Opera, 872. Opera will crash sometime.
			 From the dump file, I saw below information:
			 0012ec50 7c90378b 0012ed18 0012ffe0 0012ed34 ntdll!RtlConvertUlongToLargeInteger+0x7a
			 0012ed00 7c90eafa 00000000 0012ed34 0012ed18 ntdll!RtlConvertUlongToLargeInteger+0x46
			 0012f008 00d4d362 01e616b8 06f81008 0000002e ntdll!KiUserExceptionDispatcher+0xe
			 0012f024 00d291fd 01e616b8 0000002f 06f81008 Httpe!memcpy_s+0x4a [f:\dd\vctools\crt_bld\self_x86\crt\src\memcpy_s.c @ 67]
			 0012f048 00d39ab6 06f81008 0000002e 1d614225 Httpe!std::basic_string<char,std::char_traits<char>,std::allocator<char> >::append+0xed [x:\vc\include\xstring @ 985]
			 0012f0f8 00d224ed 000003fc 0012f188 00000001 Httpe!CHttpMgr::ProcessHTTPData+0xc6 [e:\checkout\nightly\enforcers_d_siena_networkenforcer2_cdc\platforms\win32\modules\httpe\src\httpmgr.cpp @ 56]

			 This is interesting, it crashs in "append".
			 I suspect the buf is NULL, or len is too long, so I added below
			 code.
			 
			 ************************************************************/
			CHAR FAR * pBuf = lpBuffers[dwCnt].buf;
			ULONG uLen = lpBuffers[dwCnt].len;

			if(pBuf != NULL && uLen > 0 && uLen < 0xCCCCCCCC)
			{
				sBuf.append(pBuf, uLen);

				if(pBuf != lpBuffers[dwCnt].buf || uLen != lpBuffers[dwCnt].len)
			{
					g_log.Log(CELOG_DEBUG, L"HTTPE::Exception, %x, %x, %d, %d", (INT_PTR)(lpBuffers[dwCnt].buf), (INT_PTR)pBuf, lpBuffers[dwCnt].len, uLen);
			}
		}

		}

		if(IsProcess(L"iexplore.exe") || IsProcess(L"chrome.exe") || IsProcess(L"firefox.exe") || IsProcess(L"explorer.exe"))
		{
			//for https.
			CMapperMngr& ins = CMapperMngr::Instance();
			std::string sDecrypt;
			string strPlain;
			while( (sDecrypt = ins.GetDecryptByEncryptData(sBuf)) != NON_HTTPS_DATA)
			{
				strPlain += sDecrypt;
			}

			if(!strPlain.empty())
			{
				sBuf = strPlain;
				bHttps = true;
				
				/***************************************************
				FIX BUG878
				For HTTPS of Firefox.
				There will be some characters before "POST".
				Don't know why.
				The pattern is: the begin 3 bytes are always 0x14,
				0x00, 0x00.
				So, we can remove these characters.

											Kevin 2010-1-8
				***************************************************/
				if(IsProcess(L"firefox.exe"))
				{
				        std::string::size_type uIndex = sBuf.find("POST");
					if( uIndex != 0 && uIndex != string::npos)
					{
						char szHead[] = {0x14, 0x00, 0x00};
						if(memcmp(sBuf.c_str(), szHead, 3) == 0)
						{
							sBuf = sBuf.substr(uIndex, sBuf.length() - uIndex - 1);
							g_log.Log(CELOG_DEBUG, "HTTPE::remove the special bytes before POST, \r\n,%s,\r\nsocket: %d\r\nlength; %d\r\n",  sBuf.c_str(), s, sBuf.length());
						}
					}
				}
				
				
			}
		}

	}
	else if(uType == 1)//handle "receive", we can only handle HTTP here. For HTTPS, we need to handle it in "decrypt" functions.
	{
		if(lpBuffers != NULL && lpNumberOfBytesRecvd != NULL)
		{
			int nBytesRecvd = *lpNumberOfBytesRecvd;
			
			DWORD dwCnt = 0;
			while(nBytesRecvd > 0 && dwCnt < dwBufferCount)
			{
				if(lpBuffers[dwCnt].buf != NULL && lpBuffers[dwCnt].len > 0)
				{
					sBuf.append(lpBuffers[dwCnt].buf, min(lpBuffers[dwCnt].len, (unsigned int)nBytesRecvd));
				}
				nBytesRecvd -= lpBuffers[dwCnt].len;
				++dwCnt;
			}
		}

		/***********************************************************
		Don't need to handle the data here if it is HTTPS.
		The received data of HTTPS will be handled in "decrypt" functions.
		***********************************************************/
		smartHttpMsg httpMsg;
		CHttpCollector& collector = CHttpCollector::CreateInstance();
		if(collector.GetHttpMsgBySocket_RecvList(s, httpMsg))
		{
			if(httpMsg->GetHttpsFlag())
			{
				//	the received data is not desired http message data
				//	but it may be https data.
				//	so, push the socket handle and the data into cache,
				//	the cache will be used in Decrypt function
				CMapperMngr& mapper = CMapperMngr::Instance();
				mapper.MapSocketEncryptedData(s, sBuf);
				return 0;
			}
		}
	}
	
	if(!sBuf.empty())
	{
		if(sBuf.find(HTTP_VERSION_1_0) == 0 || sBuf.find(HTTP_VERSION_1_1) == 0)
		{
			if( ProcessRedirctObligation( s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd ) )
			{
				/*
				redirect
				*/
				return HTTP_REDIRECT ;
			}
			//	try to see if the received response should be replaced by a fake 401 response.
			DWORD dwProcRlt = ProcessDeniedEval( s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd );
			if ( dwProcRlt )
			{
				//	the tcp data in \c lpBuffers is already replaced by a fake 401 response.
				return dwProcRlt;
			}
			else
			{
				//	dwProcRlt is 0, flow should go on,
				//	do nothing here.
			}
		}
		
		DWORD dwRet = ProcessHTTPData2(s, sBuf, bHttps, uType);
		if( (uType == 0) &&(dwRet==0))
		{  //added for the HTTP Header Injection Obligation
		   if(	ProcessHeaderInjectionObligation(   s,	lpBuffers,   lpNumberOfBytesRecvd) )
		   {
			   /*
			   Add the HTTP Header Injection
			   */
			   return HTTP_HEADER_INJECTION ;
		   }
		}
		if(dwRet == HTTP_HTTPS)
		{
			//	the received data is not desired http message data
			//	but it may be https data.
			//	so, push the socket handle and the data into cache,
			//	the cache will be used in Decrypt function
			CMapperMngr& mapper = CMapperMngr::Instance();
			mapper.MapSocketEncryptedData(s, sBuf);
		}

		return dwRet;
	}
	return 0;
}

DWORD CHttpMgr::ProcessHTTPData2(SOCKET s, const std::string & strData, bool bHttps, unsigned int uType)//return value: 0 means "it is allowed", 1 means "it is denied", 2 means "it is an invalid data packet". uType: 0 means "send", 1 means "recv".
{
	if(!CHttpPreProc::PreCheck(s) || strData.empty())
	{
		return 0;
	}

	if(uType == 0)//handle "send"
	{
		CHttpCollector& collector = CHttpCollector::CreateInstance();

		smartHttpMsg httpMsg;
		BOOL bRet = collector.CollectHttpMsg(TRUE, s, strData, httpMsg);

		//do parser here
		if(bRet)//it means this package is a valid HTTP data.
		{
			httpMsg->SetHttpsFlag(bHttps);
			httpMsg->SetSock(s);
			//call pre_process()
			BOOL bAllow = FALSE;
			if(CHttpPreProc::PreProcessMsg(httpMsg, bAllow))
			{
				return bAllow? 0: 1;
			}

			//	check if the current http message belong to a POST message which is already denied, and is not HTTPS
			if ( httpMsg->GetProcResult() == RESULT_DENY && httpMsg->GetType() == HTTP_POST && !bHttps )
			{
				//	yes, the current http message belong to a POST message which is already denied, and is not HTTPS
				//	so, do not need further process.
				//	return directly
				return HTTP_UPLOAD_DENIED;
			}

			if(httpMsg->ParseMsg())//Parse the data package.
			{
				//Try to get the local/remote path, and do evaluations.
				int res = CHttpProcessor::ProcessMsg(httpMsg);

				if ( httpMsg->GetProcResult() == RESULT_DENY && httpMsg->GetType() == HTTP_POST && !bHttps )
				{
					//	the upload is denied, and it is http, not https, return HTTP_UPLOAD_DENIED
					return HTTP_UPLOAD_DENIED;
				}

				return res;
			}
		}
	}
	else if(uType == 1)//handle "receive"
	{
		CHttpCollector& collector = CHttpCollector::CreateInstance();

		smartHttpMsg httpMsg;
		BOOL bRet = collector.CollectHttpMsg(FALSE, s, strData, httpMsg);	
		if(bRet)//it means the current package is a valid HTTP response.
		{
			httpMsg->SetHttpsFlag(bHttps);
			httpMsg->SetSock(s);
			if(httpMsg->ParseMsg())
			{
				return CHttpProcessor::ProcessMsg(httpMsg);
			}
		}
		else
		{
			//	received data is not desired http message data
			//	May be it is https data.
			
			return HTTP_HTTPS;
		}
	}
	
	return 0;
}
BOOL CHttpMgr::ProcessRedirctObligation( SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd )
{
	/*
	Added for the navigation redirect
	*/
	BOOL bRet =  FALSE ;
	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spHttpMsg;
	if(  collector.GetHttpMsgBySocket_SendList( s, spHttpMsg ) )
	{
		string strURL;
		spHttpMsg->GetRedirectURL(strURL) ;
		if(!strURL.empty()&&(lpBuffers[0].len >= (strlen(REDIRECT_HEADER)+strURL.length())))
		{
			ZeroMemory( lpBuffers[0].buf,lpBuffers[0].len ) ; 
			::_snprintf_s(lpBuffers[0].buf, lpBuffers[0].len, _TRUNCATE,	REDIRECT_HEADER, strURL.c_str()) ;
			g_log.Log(CELOG_DEBUG,"HTTPE::Try to replace response with Redirect:\r\n%s, socket: %d", lpBuffers[0].buf, s);
			*lpNumberOfBytesRecvd =	 (DWORD)strlen( lpBuffers[0].buf ) ;
			lpBuffers[0].len =	(ULONG)strlen( lpBuffers[0].buf ) ;
			dwBufferCount = 1 ;
			bRet = TRUE ;

			/*********************************************************
			fix bug10810
			Root cause: 
				In this case, it won't go into "collect" logic, 
			so, the HTTPMsg in "send list" won't be removed.
			sometime, application will use same SOCKET to send/recv
			the new data. so that, it will still find the old HTTPMsg
			in "send list", it will be redirected again.

			Solution: remove the related HTTPMsg in "send list" after 
			"Redirect" was handled.

												kevin 2010-2-5
			**********************************************************/
			collector.RemoveHttpMsg(TRUE, s);
		}
	}
	return bRet ;
}

DWORD CHttpMgr::ProcessDeniedEval( SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd )
{
	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spHttpMsg;
	if(  collector.GetHttpMsgBySocket_SendList( s, spHttpMsg ) && !spHttpMsg->GetHttpsFlag() )
	{
		//	yes, we can find a http request by socket handle, and this http message is not an https message, we only handle HTTP.
		//	check if the request is a GET request
		if ( spHttpMsg->GetType() == HTTP_GET )
		{
			//	yes, the request is a GET request
			//	so, this tcp data belong to a response of a GET request
			//	try to check if the GET request is denied
			if ( spHttpMsg->GetProcResult() == RESULT_DENY )
			{
				//	the GET is denied
				//	this means the navigation is denied,
				//	we need to replace this tcp data with 404 fake response.
				if ( lpBuffers[0].len >= strlen(HTTP_ERROR_CODE_RESPONSE) )
				{
					//	replace with fake response
					ZeroMemory( lpBuffers[0].buf,lpBuffers[0].len ) ; 
					::_snprintf_s( lpBuffers[0].buf, lpBuffers[0].len, _TRUNCATE, HTTP_ERROR_CODE_RESPONSE ) ;
					lpBuffers[0].len =	(ULONG)strlen( lpBuffers[0].buf ) ;
					*lpNumberOfBytesRecvd =	 lpBuffers[0].len ;
					dwBufferCount = 1 ;

					//	erase the GET request, it is useless and should be erased
					collector.RemoveHttpMsg(TRUE, s);
					return HTTP_NAVIGATION_DENIED;
				}
			}
		}
		//	check if the request is a POST request
		else if ( spHttpMsg->GetType() == HTTP_POST )
		{	
			//	yes, the request is a POST request
			//	so, this tcp data belong to a response of a POST request
			//	try to check if the POST request is denied
			if ( spHttpMsg->GetProcResult() == RESULT_DENY )
			{

				//	the POST is denied
				//	this means the upload is denied,
				//	we need to replace this tcp data with 401 fake response.
				if ( lpBuffers[0].len >= strlen(HTTP_ERROR_CODE_RESPONSE) )
				{
					//	replace with fake response
					ZeroMemory( lpBuffers[0].buf,lpBuffers[0].len ) ; 
					::_snprintf_s( lpBuffers[0].buf, lpBuffers[0].len, _TRUNCATE, HTTP_ERROR_CODE_RESPONSE ) ;
					lpBuffers[0].len =	(ULONG)strlen( lpBuffers[0].buf ) ;
					*lpNumberOfBytesRecvd =	 lpBuffers[0].len ;
					dwBufferCount = 1 ;

					//	erase the POST request, it is useless and should be erased
					collector.RemoveHttpMsg(TRUE, s);
					return HTTP_UPLOAD_DENIED;
				}
			}
		}
	}
	return 0;
}

BOOL CHttpMgr::ProcessRedirctObligation( const SOCKET s, PVOID pDecryptData, const DWORD& dBufferLen )
{
  	/*
	Added for the navigation redirect
	*/
	BOOL bRet =  FALSE ;
	PSecBufferDesc lpBuffers = ( PSecBufferDesc)pDecryptData ;
	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spHttpMsg;
	if(  collector.GetHttpMsgBySocket_SendList( s, spHttpMsg ) )
	{
		string strURL;
		spHttpMsg->GetRedirectURL(strURL) ;
		size_t dAddedLen =   strlen(REDIRECT_HEADER)+strURL.length() ;
		if(!strURL.empty()&&(dBufferLen >= dAddedLen ))
		{
			ULONG i = 0 ;
			char * pBuf = new char[dAddedLen+1] ;
			ZeroMemory( pBuf,dAddedLen+1 ) ; 
			::_snprintf_s(pBuf, dAddedLen+1, _TRUNCATE,	REDIRECT_HEADER, strURL.c_str()) ;
			while(i<lpBuffers->cBuffers )
			{
				ZeroMemory( lpBuffers->pBuffers[i].pvBuffer,lpBuffers->pBuffers[i].cbBuffer ) ; 
				if( lpBuffers->pBuffers[i].cbBuffer >dAddedLen)
				{
					::memcpy_s( lpBuffers->pBuffers[i].pvBuffer,lpBuffers->pBuffers[i].cbBuffer-1, pBuf ,dAddedLen) ;
					break ;
				}
				++i ;	
			}
			delete []  pBuf ;
			pBuf = NULL ;
			lpBuffers->cBuffers = i+1 ;
			bRet = TRUE ;
		}
	}
	return bRet ;
}
BOOL CHttpMgr::ProcessRedirctObligation( const SOCKET s, char* pDecryptData,  int * dBufferLen ) 
{
	/*
	Added for the navigation redirect
	*/
	BOOL bRet =  FALSE ;
	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spHttpMsg;
	if(  collector.GetHttpMsgBySocket_SendList( s, spHttpMsg ) )
	{
		string strURL;
		spHttpMsg->GetRedirectURL(strURL) ;
		size_t dAddedLen =   strlen(REDIRECT_HEADER)+strURL.length() ;
		if(!strURL.empty()&&(*dBufferLen >= (int)dAddedLen ))
		{
			ZeroMemory( pDecryptData,dAddedLen+1 ) ; 
			::_snprintf_s(pDecryptData, dAddedLen+1, _TRUNCATE,	REDIRECT_HEADER, strURL.c_str()) ;
			*dBufferLen = (int)dAddedLen ;
			bRet = TRUE ;
		}
	}
	return bRet ;
}
BOOL CHttpMgr::ProcessHeaderInjectionObligation( SOCKET s, LPWSABUF lpBuffers, LPDWORD InjectedDataLen )
{
   	/*
	Added for the navigation redirect
	*/
	BOOL bRet =  FALSE ;
	CHttpCollector& collector = CHttpCollector::CreateInstance();
	smartHttpMsg spHttpMsg;
	*InjectedDataLen = 0;
	if(  collector.GetHttpMsgBySocket_SendList( s, spHttpMsg ) )
	{
		vector<wstring> vData;
		spHttpMsg->GetHeaderInjectionData(vData) ;

		vector<wstring>::iterator itr;
		string sBuf = string(lpBuffers[0].buf, lpBuffers[0].len) ;

		const static string strEnd = "\r\n\r\n" ;
		const static string strInner = "\r\n" ;
		for(itr = vData.begin(); itr != vData.end(); itr++)//Add all the "injection" items into HTTP header
		{
			std::string::size_type pos = sBuf.find( strEnd ) ;
		    
			if(	  pos != std::string.npos )
			{
				string strData = string((*itr).begin(), (*itr).end());
				strData = strData + strInner ;
				sBuf.insert( pos +strlen( strInner.c_str()),   strData ) ;
				*InjectedDataLen += (DWORD)strData.length() ;
			}
		}
		if(*InjectedDataLen > 0)
		{
			char* pBuf = NULL ;
			pBuf = new char[ sBuf.length()] ;
			memset(pBuf, 0,sBuf.length()) ;
				::memcpy_s( pBuf, sBuf.length() , sBuf.c_str() ,   sBuf.length() ) ;
				lpBuffers[0].buf =pBuf ; 
				lpBuffers[0].len = (ULONG)sBuf.length()  ;
				bRet = TRUE ;
			}
		}
	return bRet ;
}
