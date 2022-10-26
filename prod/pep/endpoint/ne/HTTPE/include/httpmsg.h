#pragma once

#include "smart_ptr.h"
#include <string>
#include <list>
#include <map>
#include <vector>
using namespace std;

/********************************************************************************************************************************************************
This is a sample of HTTP protocol:


GET / HTTP/1.1
Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, application/xaml+xml, application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application,
Accept-Language: en-us
UA-CPU: x86
Accept-Encoding: gzip, deflate
User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.30; InfoPath.1; .NET CLR 3.0.04506.648; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; MS-RTC LM 8; .NET CLR 1.1.4322)
Host: www.google.cn
Connection: Keep-Alive
Cookie: PREF=ID=1ae0a785b7721395:U=da24ed0d9babb319:NW=1:TM=1254043437:LM=1254043449:S=MLNwKNWfXiEW4akJ; NID=28=Oz19n3flPwWlBasXIp7gh11MzonUKGm4oUNdfIWoFIUcniCOvebRxK2ok3faXFkszN7Ir0jFC1vsh8WFJwaafjIfTXnGnVTAmpLDTFetE4_bBf82T2WO8smYomXmWr8r



HTTP/1.0 200 OK
Date: Mon, 09 Nov 2009 05:32:39 GMT
Expires: -1
		 Cache-Control: private, max-age=0
		 Content-Type: text/html; charset=UTF-8
Server: gws
		X-XSS-Protection: 0
		Content-Encoding: gzip
		X-Cache: MISS from proxy.cn.nextlabs.com
Via: 1.0 proxy.cn.nextlabs.com:3128 (squid/2.6.STABLE21)
Connection: close
			..........tUmo.F..._AH....8o..J..w..r..R.(.-....%......w...[...2/..<3.,r(..".(Y.J.H..j.?..K....\...........0......q.....Z7.r..
			..HiV....Z......R%tcg.2lV.w..\U./.y~G..~.!....Usu..g...K......+!.........#)&.....O...kn}.Cm.....l^...Gy...
			%f<l
*********************************************************************************************************************************************************/


enum HTTP_MSG_TYPE
{
	HTTP_UNSET,
	HTTP_GET,
	HTTP_PUT,
	HTTP_POST,
	HTTP_200_OK,
	HTTP_NO_200
};

enum HTTP_MSG_PROCESS_RESULT
{
	RESULT_UNSET,
	RESULT_ALLOW,
	RESULT_DENY
};

typedef struct struHTTP_FILEDATA
{
	string filedata;
	string filename;
	HTTP_MSG_PROCESS_RESULT result;
	bool bHandled;
	struHTTP_FILEDATA()
	{
		result = RESULT_UNSET;
		bHandled = false;
	}
}HTTP_MULTI_FILEDATA;


class CHttpMsg;

typedef YLIB::smart_ptr<CHttpMsg> smartHttpMsg;

typedef BOOL (CHttpMsg::* HttpParserType)(const string &) ;

class CHttpMsg
{
public:
	CHttpMsg(BOOL request);
	~CHttpMsg();

	CHttpMsg & operator=( const CHttpMsg & ) {;};


	void AddTcpData(const string& tcpdata);

	void SetReqRmt(const string& reqRemote);
	void SetReqType(HTTP_MSG_TYPE reqType);
	
	HTTP_MSG_TYPE GetType();
	HTTP_MSG_TYPE GetReqType();
	
	void GetRmt(string& rmt, BOOL bWithPrefix = FALSE);
	void GetReqRmt(string& ReqRmt, BOOL bWithPrefix = FALSE);
	void GetNavigationURL(string& url);

	HTTP_MSG_PROCESS_RESULT GetProcResult();
	void SetProcResult(HTTP_MSG_PROCESS_RESULT result);

	/*
	return value:
	if return true, success;
	if return false, fatal error happened;
	*/
	BOOL ParseMsg();

	void GetLocalPaths(std::vector<std::string>& localPaths);
	/*
	Add the interface for the Data mapping
	*/

	void GetFileList(list<HTTP_MULTI_FILEDATA>& fileList) ;

	BOOL IsRecvingBody(){return m_bRecvBody;};

	//Set the result to the file once it was evaluated. it can avoid the "multiple evaluation".
	void SetEvalResult(const string& filepath, HTTP_MSG_PROCESS_RESULT result);

	void SetSock(SOCKET s){m_Sock = s;};
	SOCKET GetSock(){return m_Sock;};

	void SetHttpsFlag(bool bHttps){m_bIsHttps = bHttps;};
	bool GetHttpsFlag() const {return m_bIsHttps;};

	void SetRedirectURL(const string & strRedirectURL){m_strRedirectURL = strRedirectURL;}
	void GetRedirectURL(string & redirectURL) const 
	{
		redirectURL = m_strRedirectURL;
	};
	void SetHeaderInjectionData( const vector<wstring>& vHeaderInjectionData ){m_vHeaderInjectionData = vHeaderInjectionData; } ;
	void GetHeaderInjectionData(vector<wstring>& vHeaderInjectionData) const 
	{
		vHeaderInjectionData = m_vHeaderInjectionData;
	};
	void SetHeaderItems( const map<wstring, wstring>& mapAttrs){m_mapAttributes = mapAttrs;};
	void GetHeaderItems(map<wstring, wstring> & mapAttrs) const
	{
		mapAttrs = m_mapAttributes;
	};

	void SetHandledStatus(BOOL b){m_bHandled = b;}
	BOOL GetHandledStatus()const {return m_bHandled;}
private:
	/*
	return value:
	if return true, refer to \c header;
	if return false, means there is no more complete header in httpmsg;
	*/
	BOOL GetHeader(string& header);

	/*
	return value:
	if return true, success;
	if return false, fatal error happened;
	*/
	BOOL ParseHeader(const string& header);
	BOOL ParseStartline(const string& startline);
	BOOL ParseBody();

private:
	//	start line parser
	BOOL ParseGET(const string& startline);
	BOOL ParsePUT(const string& startline);
	BOOL ParsePOST(const string& startline);

	//	header parser
	BOOL ParseHost(const string& header);
	BOOL ParseContentType(const string& header);
	BOOL ParseContentLen(const string& header);
	BOOL ParseContentEncoding(const string& header);	
	BOOL ParseContentDisposition(const string& header);	
	BOOL ParseTransferEncoding(const string& header);
	BOOL ParseReferer(const string& header);

	/*	
	body parser.

	return value:
	return true, parsed all desired information or there is no desired information in current HTTP message, so, return and wait the next TCP packet
	return false, fatal error.

	task:
	1, if there is already a file data parsed in previous parse, no need to parse this body any more. but the multipart/form data still need to process.
	2, get out file data in current body, the max cache memory is m_dwMaxFileCache byte,

	*/
	BOOL ParseUnzippedBody();
	BOOL ParsePlainBody();
	BOOL ParsePotentialVermeerRPCBody();
	BOOL ParsePotentialVermeerURLEncodedBody();
	BOOL ParseMultipartFormBody();
	BOOL ParseOctetBody();//We saw this format on "google document" with Chrome.
	/*
	unzip data

	return true, success
	return false, fatal error.
	*/
	BOOL UnzipBody(const string& input, string& output);

	BOOL ComposeChunkedData(const string& input, string& output);

	/************************************************************************
	If we try to upload files to sharepoint with explorer (Web client is OFF),
	We will get below information:
	method=put document:6.0.2.8164&service_name=/sites/engineering/docs&document=[document_name=Engineering Documents/HTTPE test/55555555555555.JPG;meta_info=[]]&put_option=edit,atomic,thicket&comment=&keep_checked_out=false
	"ParseMethod" is used to parse this string.
	************************************************************************/
	BOOL ParseMethod(const string & strSrc, string& strRmtURI, string& strFileName);
	BOOL ParseGetMethod( const string & strSrc, string& strRmtURI, string& strFileName);


	void AddNewFile(HTTP_MULTI_FILEDATA file);
private:
	//	to save memory space
	void ClearParsedData();

private:
	map<wstring, wstring> m_mapAttributes;//This variable is used to store header items. for example, key: Host, value: www.google.com

	string m_strRedirectURL;//This variable is used to store the new URL of redirect.
	vector<wstring> m_vHeaderInjectionData;//This variable is used to store the new data of HTTP Header Injection.

	bool m_bIsHttps;

	SOCKET m_Sock;
	//	type information
	BOOL m_bRequest;
	HTTP_MSG_TYPE m_type;

	//	only for response
	HTTP_MSG_TYPE m_requestType;	
	string m_requestRemote;

	//	origin data
	string m_httpdata;

	/************************************************************************
	I found an interesting issue in firefox:
	There will have another "HTTP/1.1 200 OK" before file data
	sometime when we try to download files with Firefox.
	For example:
	1. Try to download a file with FireFox;
	2. Get the response "HTTP/1.1 200 OK..."
	3. Normally, we should get the file data after this response, but
	   actually, we will have one more "HTTP/1.1 200 OK". The 2nd response
	   is similar to #2. replace "\n\n" with "\r\n" in the 2nd response, we 
	   can see the 2nd response contains the 1st one.

													kevin 2009-11-23
	************************************************************************/
	string m_httpdata_firefox;

	//	the zipped body data
	//	we need to save all of them to unzip data
	string m_zippedbody;

	//	used in zip body case
	//	after unzip the zipped body, we need to sub the data with the old unzipped data,
	//	and then append the new part into m_body
	DWORD m_dwLastBodyLength;

	//	origin data to be parse
	string::size_type m_pos;
	string::size_type m_length;

	//	body unzipped to be parse
	string m_body;

	//	parsed information
	string m_referer;
	string m_host;
	string m_uri;
	string m_contenttype;
	string m_boundary;
	string m_contentdisposition;
	string m_transferencoding;
	string m_contentencoding;
	DWORD m_dwContentlen;
	list<HTTP_MULTI_FILEDATA> m_filedatas;

	static const DWORD m_dwMaxFileCache = 4096;

	BOOL m_bRecvBody;//TRUE means it is in the progress of "receive file content".

	//	process status
	BOOL m_headerParsed;
	HTTP_MSG_PROCESS_RESULT m_procResult;

	//	parser for headers
	friend class HttpMsgParserMap;
	static HttpMsgParserMap m_sParserMap;

	//	const string in HTTP content type
	static const string m_vermeerurlencoded;
	static const string m_octet;//It was used in "google document" with Chrome (Upload), kevin 2010-1-18
	static const string m_vermeerrpc;

	//	const string in HTTP content disposition
	static const string m_attachment;

	//	const string in HTTP content encoding
	static const string m_encoding;

	//	const string in HTTP transfer encoding
	static const string m_chunked;


	//	remained length of sub chunked data, 
	//	it is used in the case -- the sub chunked data is not finished until the end of this TCP packet!!!
	DWORD m_dwRestChunkLen;

	BOOL m_bHandled;//TRUE: means this MESSAGE was handled by HTTPE, we don't need to handle again.
};

//	this class is not designed as an interface,
//	it should only be instanced once in CHttpMsg as a static member data.
class HttpMsgParserMap
{
public:
	HttpMsgParserMap();
	~HttpMsgParserMap();

	/*
	return value:
	if return NULL, can not find a parser to parse the header;
	if return non-NULL, it is a parser function;
	*/
	HttpParserType GetHeaderParser(const string& headertype);
	HttpParserType GetStartlineParser(const string& type);

private:
	map< string, HttpParserType > m_headerParser;
	map< string, HttpParserType > m_StartlineParser;
};
