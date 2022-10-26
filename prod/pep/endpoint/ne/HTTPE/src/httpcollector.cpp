#include "stdafx.h"

#include "HttpCollector.h"
#include "criticalMngr.h"
#include "timeout_list.hpp"

CTimeoutList g_DownloadURIList(60000);



#define HTTP_RES_304 "304 NOT MODIFIED"
#define HTTP_REQ_GET "GET "
#define HTTP_REQ_PUT "PUT "
#define HTTP_REQ_POST "POST "

CHttpCollector::CHttpCollector()
{
}

CHttpCollector::~CHttpCollector()
{
	m_mapReceiveData.clear();
	m_mapSendData.clear();
}

CHttpCollector& CHttpCollector::CreateInstance()
{
	static	 CHttpCollector inst ;
	
	return inst;
}


BOOL CHttpCollector::CollectHttpMsg(BOOL send, SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg)
{	
	if (send == TRUE)
	{
		return Add_SendData(s, tcpdata, httpMsg);
	}
	else
	{
		return Add_ReceiveData(s, tcpdata, httpMsg);
	}
}

void CHttpCollector::RemoveHttpMsg(BOOL send, SOCKET s)
{
	if (send)
	{
		::EnterCriticalSection(&CcriticalMngr::s_csSendList);
		map<SOCKET, smartHttpMsg>::iterator it = m_mapSendData.find(s);
		if(it != m_mapSendData.end())
		{
			m_mapSendData.erase(it);
		}
		::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
		return;
	}
	else
	{
		::EnterCriticalSection(&CcriticalMngr::s_csRecvList);
		map<SOCKET, smartHttpMsg>::iterator it = m_mapReceiveData.find(s);
		if(it != m_mapReceiveData.end())
		{
			m_mapReceiveData.erase(it);
		}
		::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
		return;
	}
}

BOOL CHttpCollector::Add_ReceiveData(SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg)
{
	::EnterCriticalSection(&CcriticalMngr::s_csRecvList);

	map<SOCKET, smartHttpMsg>::iterator it = m_mapReceiveData.find(s);

	if(it == m_mapReceiveData.end())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);

		if ( tcpdata.find(HTTP_VERSION_1_0) == 0 || tcpdata.find(HTTP_VERSION_1_1) == 0 )
		{
			::EnterCriticalSection(&CcriticalMngr::s_csSendList);
			map<SOCKET, smartHttpMsg>::iterator it2 = m_mapSendData.find(s);
			if(it2 != m_mapSendData.end())
			{
				//	it is a new response of a previous request
				smartHttpMsg localhttpMsg( new CHttpMsg(FALSE) );
				localhttpMsg->AddTcpData(tcpdata);

				//	set its request information
				HTTP_MSG_TYPE type = it2->second->GetType();
				string reqRmt;
				it2->second->GetRmt(reqRmt);
				localhttpMsg->SetReqType(type);
				localhttpMsg->SetReqRmt(reqRmt);
				string strURL ;
				it2->second->GetRedirectURL(strURL)	 ;
				if( !strURL.empty() )
				{
					localhttpMsg->SetRedirectURL(strURL );
				}
				map<wstring, wstring> attrs;
				it2->second->GetHeaderItems(attrs);
				localhttpMsg->SetHeaderItems(attrs);

				//	remove the request, the request is useless
				m_mapSendData.erase(it2);

				::LeaveCriticalSection(&CcriticalMngr::s_csSendList);

				/**********************************************************************************************************************
				Format:
				GET /sites/engineering/docs/Engineering%20Documents/HTTPE%20test/Utilities.h HTTP/1.1
				Accept: ...
				UA-CPU: x86
				Accept-Encoding: gzip, deflate
				If-Modified-Since: Wed, 18 Nov 2009 05:10:58 GMT
				If-None-Match: "{3F83623F-268B-48BC-8A4B-BD60AA0D5BE6},6"
				User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; InfoPath.1; .NET CLR 3.0.04506.648; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; MS-RTC LM 8; .NET CLR 1.1.4322)
				Host: intranet.cn.nextlabs.com
				Connection: Keep-Alive
				Cookie: MSOWebPartPage_AnonymousAccessCookie=80; WSS_KeepSessionAuthenticated=80


				HTTP/1.1 304 NOT MODIFIED
				Date: Thu, 19 Nov 2009 06:25:28 GMT
				Server: Microsoft-IIS/6.0
				MicrosoftSharePointTeamServices: 12.0.0.4518
								 X-Powered-By: ASP.NET
				Exires: Wed, 04 Nov 2009 06:25:28 GMT
				Cache-Control: private,max-age=0
				Content-Length: 0
				Public-Extension: http://schemas.microsoft.com/repl-2
				Set-Cookie: WSS_KeepSessionAuthenticated=80; path=/
				Set-Cookie: MSOWebPartPage_AnonymousAccessCookie=80; expires=Thu, 19-Nov-2009 06:55:28 GMT; path=/

				in this case, IE won't download the data again, it just copy the file from "IT temp folder" to destination.

				for example:
				copy C:\Documents and Settings\Kevin\Local Settings\Temporary Internet Files\Content.IE5\6NOBZ96H\Utilities[1].h
				to c:\foo\utilities.h
				**************************************************************************************************************************/
				if(tcpdata.find(HTTP_RES_304) != string::npos)//for Sharepoint, IE will use the 
				{
					string ReqRmt;
					localhttpMsg->GetReqRmt(ReqRmt, TRUE);
					string strURI = UrlDecode(ReqRmt);
					std::string::size_type uIndex = strURI.rfind('/');
					if(uIndex == string::npos)
					{
						uIndex = strURI.rfind('\\');
					}

					if(uIndex != string::npos)
					{
						string strFileName = strURI.substr(uIndex + 1, strURI.length() - uIndex - 1);
						g_DownloadURIList.AddItem( wstring(strFileName.begin(), strFileName.end()), wstring(strURI.begin(), strURI.end()) );
					}
				}
				
				::EnterCriticalSection(&CcriticalMngr::s_csRecvList);
				m_mapReceiveData[s] = localhttpMsg;
				httpMsg = localhttpMsg;
				::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
				return TRUE;
			}
			else
			{
				::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
			}
		}

		//	not a part of desired response
		
		return FALSE;
	}

	//	continue part of a response
	it->second->AddTcpData(tcpdata);
	httpMsg = it->second;

	::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
	return TRUE;
}

BOOL CHttpCollector::Add_SendData(SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg)
{
	::EnterCriticalSection(&CcriticalMngr::s_csSendList);

	map<SOCKET, smartHttpMsg>::iterator it = m_mapSendData.find(s);

	if(it == m_mapSendData.end())
	{
		::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
		if ( tcpdata.find(HTTP_REQ_GET) == 0 || tcpdata.find(HTTP_REQ_PUT) == 0 || tcpdata.find(HTTP_REQ_POST) == 0 )
		{
			/****************************************************************************************
			Fix bug 878
			Firefox will send POST request like below when user tries to upload a file on gmail:

			POST /mail/channel/bind?OSID=76CD27B120048775&OAID=27&VER=6&it=62&at=xn3j38mpg677vls61v1y3e9bcmopxk&SID=DE9385CE83B00FB1&RID=57735&zx=st6dnmutyvyo&t=1 HTTP/1.1
			Host: mail.google.com
			User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.9.1.7) Gecko/20091221 Firefox/3.5.7
			Accept: text/html,application/xhtml+xml,application/xml;q=0.9,q=0.8
			Accept-Language: en-us,en;q=0.5
			Accept-Encoding: gzip,deflate
			Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
			Keep-Alive: 300
			Connection: keep-alive
			Content-Type: application/x-www-form-urlencoded; charset=UTF-8
			Referer: https://mail.google.com/mail/?ui=2&view=js&name=js&ver=2n73sWqpJa8.en.&am=!Mi2c2SsaQZO5A3Gi0fgGItaReiDqLpkZJ8MbtEaKlVYNpA
			Content-Length: 46
			Cookie: S=gmail=qzGiRrqbQ9jQ5RWrF3kVvg:gmproxy=Hi_StxPYimW4w5cLH8pWcw; GX=DQAAAIUAAADwx_ZkxO1qa1t87jV1NKx9a4Au0R0IzaoBSBx0o2B2BLdgFulhtqzYSrPgNkNPA2bYgM994SYfeaeqPph2gHPdkzqOrmvml7ctSyEWfgjTuqIE2ONZwzvL1xwkVSBI5WRrUM2iGNaKwpCxqBrKA9DRGRqMuqhJzWItD9v8VYznOIgstNttK8OMJ4qTe8ZeCEs; GMAIL_AT=xn3j38mpg677vls61v1y3e9bcmopxk; gmailchat=kevinzhou05@gmail.com/61513; PREF=ID=8ba5982deabc9fdf:U=a419d2c6f198100d:TM=1253256553:LM=1259215911:GM=1:S=85dTKLcDf9pJTw2h; NID=26=YLz5MN9isGpT9MDl6mjohuCu6FyAj9QDh-Z8OM_g7D0g2Ni2XxJQtazfNnIq8O_GPkFCPBth2JBjsNYG9py_-lIoXzhbNNj6QlY8pDUUUt_jiauZiTE9kO4QyLI00svc; TZ=-480; SID=DQAAAIUAAADMvvHqCRuq_yntjbTnFPWC_I3fQTtVwVnU4waGGEX_MuZxtZtwLL44Wd0VUGQ_XGWFPGeYy-CTEVoI-luRrx1oLmxhLCVQucVneljMdhf5zhGe3w4udF3-Ut0xi0wh5RZFawcQ7rVv-SdFPU8kvmqfEthGpGwkwYJP7hDKNS9xwzlCIGsuS-5-9qu0xfRvWoU; HSID=A9Ja6XqHoUSXYXXSg; SSID=AXh-5ffFpl2hPbe74
			Pragma: no-cache
			Cache-Control: no-cache

			count=1&req0_type=cf&req0_focuse


			and it will use the same socket to upload files too.
			So there will have problems, the next POST request of "file upload" will be looked as 
			body of the 1st request. and then, it will loose evaluations for the "file upload".

			solution:
			Ignore the POST request like above. it will only impact gmail of firefox.

			******************************************************************************************/
			if(IsProcess(L"firefox.exe") && tcpdata.find("POST ") == 0 && tcpdata.find("Host: mail.google.com") != string::npos 
				&& tcpdata.find("Content-Type: multipart/form-data; boundary=") == string::npos && tcpdata.find("\r\n\r\n") != string::npos)
			{
				return FALSE;
			}

			//	is a new request
			smartHttpMsg localhttpMsg( new CHttpMsg(TRUE) );
			localhttpMsg->AddTcpData(tcpdata);

			::EnterCriticalSection(&CcriticalMngr::s_csSendList);
			m_mapSendData[s] = localhttpMsg;
			httpMsg = localhttpMsg;
			::LeaveCriticalSection(&CcriticalMngr::s_csSendList);

			::EnterCriticalSection(&CcriticalMngr::s_csRecvList);
			map<SOCKET, smartHttpMsg>::iterator it2 = m_mapReceiveData.find(s);
			if(it2 != m_mapReceiveData.end())
			{
	//			g_log.Log(CELOG_DEBUG ,"==================================================");
				m_mapReceiveData.erase(it2);
			}

			::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
			return TRUE;
		}

		//	not a part of desired request
		
		return FALSE;
	}

	/*******************************************************************
	Fix bug873
	Sometime, the "POST" request will be in a "GET" request. 
	The probable reason is: there isn't response for the "GET" request.
	So, it always exists in our "cache", and then Firefox/Opera uses the 
	same socket to send files.
	********************************************************************/
	if( (IsProcess(L"firefox.exe") || IsProcess(L"opera.exe") ) && tcpdata.find("POST ") == 0 && 
		(tcpdata.find("Host: mail.google.com") != string::npos || tcpdata.find("mail.yahoo.com") != string::npos)
		&& tcpdata.find("Content-Type: multipart/form-data; boundary=") != string::npos && it->second->GetType() == HTTP_GET)
	{
		smartHttpMsg localhttpMsg( new CHttpMsg(TRUE) );
		localhttpMsg->AddTcpData(tcpdata);

		m_mapSendData[s] = localhttpMsg;
		httpMsg = localhttpMsg;
		
		::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
		g_log.Log(CELOG_DEBUG, L"HTTPE::This is an interesting case, the \"POST\" request is in a \"GET\" package.\r\n");
		return TRUE;
	}

	/************************************************************************/
	/* 
	Fix bug 10713, add by Benjamin, Jan 25 2010
	almost the same case as above.
	*/
	/************************************************************************/
	if( ( IsProcess(L"firefox.exe") || IsProcess(L"chrome.exe") ) && tcpdata.find("POST ") == 0 && 
		tcpdata.find("skydrive.live.com\r\n") != string::npos && 
		it->second->GetType() == HTTP_GET )
	{
		smartHttpMsg localhttpMsg( new CHttpMsg(TRUE) );
		localhttpMsg->AddTcpData(tcpdata);

		m_mapSendData[s] = localhttpMsg;
		httpMsg = localhttpMsg;

		::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
		g_log.Log(CELOG_DEBUG, L"HTTPE::This is an interesting case, the \"POST\" request is in a \"GET\" package.\r\n");
		return TRUE;
	}

	/*Added by chellee for the bug	889
	*/
	string HostUrl ;
	it->second->GetRmt( HostUrl ) ;
	if( IsProcess( L"iexplore.exe"   ) &&tcpdata.find("Host: docs.google.com") != string::npos	&&
		  tcpdata.find("Content-Type: multipart/form-data; boundary=") != string::npos &&
		 it->second->GetType() == HTTP_POST && HostUrl.find( "docs.google.com") != string::npos   )
	{
		  smartHttpMsg localhttpMsg( new CHttpMsg(TRUE) );
		localhttpMsg->AddTcpData(tcpdata);
	
		m_mapSendData[s] = localhttpMsg;
		httpMsg = localhttpMsg;
		
		::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
		g_log.Log(CELOG_DEBUG, L"HTTPE::This is an interesting case, the \"POST\" request is in a \"Post\" package.(IE)\r\n");
		return TRUE;
	}
	//	continue part of a request
	it->second->AddTcpData(tcpdata);
	httpMsg = it->second;

	::LeaveCriticalSection(&CcriticalMngr::s_csSendList);

	return TRUE;
}

BOOL CHttpCollector::GetHttpMsgBySocket_SendList( SOCKET s, smartHttpMsg &spHttpMsg )
{
	BOOL bRet = FALSE ;
	::EnterCriticalSection(&CcriticalMngr::s_csSendList);
	map<SOCKET, smartHttpMsg>::iterator it = m_mapSendData.find(s);
	if(	it!= m_mapSendData.end() )
	{
		spHttpMsg = (*it).second;
		bRet = TRUE ;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csSendList);
	return bRet ;
}

BOOL CHttpCollector::GetHttpMsgBySocket_RecvList( SOCKET s, smartHttpMsg &spHttpMsg )
{
	BOOL bRet = FALSE ;
	::EnterCriticalSection(&CcriticalMngr::s_csRecvList);
	map<SOCKET, smartHttpMsg>::iterator it = m_mapReceiveData.find(s);
	if(	it!= m_mapReceiveData.end() )
	{
		spHttpMsg = (*it).second;
		bRet = TRUE ;
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
	return bRet ;
}

BOOL CHttpCollector::GetHttpMsgByRmtPath_RecvList( const string & strRmtPath, smartHttpMsg &spHttpMsg )
{
	BOOL bRet = FALSE ;
	::EnterCriticalSection(&CcriticalMngr::s_csRecvList);
	map<SOCKET, smartHttpMsg>::iterator it = m_mapReceiveData.begin();
	for( ; it != m_mapReceiveData.end(); it++)
	{
		smartHttpMsg smMsg = it->second;
		list<HTTP_MULTI_FILEDATA> listFile;
		smMsg->GetFileList(listFile);

		list<HTTP_MULTI_FILEDATA>::iterator itr = listFile.begin();
		for( ; itr != listFile.end(); itr++)
		{
			if(_stricmp((*itr).filename.c_str(), strRmtPath.c_str()) == 0)
			{
				bRet = TRUE;
				spHttpMsg = smMsg;
				break;
			}
		}
		if(bRet)
		{
			break;
		}
	}

	::LeaveCriticalSection(&CcriticalMngr::s_csRecvList);
	return bRet ;
}
