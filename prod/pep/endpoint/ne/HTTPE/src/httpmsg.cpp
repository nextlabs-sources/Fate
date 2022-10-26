#include "stdafx.h"
#include "httpmsg.h"
#include "gzipper.h"
#include "gziphelper.h"
#include "MapperMngr.h"
#include "SharedMemory.h"
#include "assert.h"

#pragma warning( push )
#pragma warning( disable : 6011 )
#  include <boost/algorithm/string.hpp>
#pragma warning( pop )

#define HTTP_PREFIX				"http://"
#define HTTPS_PREFIX				"https://"
#define HTTP_HEADER_SEPARATOR	":"

const string CHttpMsg::m_vermeerurlencoded = "application/x-vermeer-urlencoded";
const string CHttpMsg::m_vermeerrpc = "application/x-vermeer-rpc";
const string CHttpMsg::m_attachment = "attachment; ";
const string CHttpMsg::m_encoding = "gzip";
const string CHttpMsg::m_chunked = "chunked";
const string CHttpMsg::m_octet = "application/octet-stream";

#define HTTP_HEADER_HOST "HOST: "
#define HTTP_HEADER_CONTENT_TYPE "CONTENT-TYPE: "
#define HTTP_HEADER_CONTENT_LENGTH "CONTENT-LENGTH: "
#define HTTP_HEADER_CONTENT_ENCODING "CONTENT-ENCODING: "
#define HTTP_HEADER_CONTENT_DISPOSITION "CONTENT-DISPOSITION: "
#define HTTP_HEADER_REFERER "REFERER: "
#define HTTP_HEADER_TRANSFER_ENCODING "TRANSFER-ENCODING: "

#define HTTP_STARTLINE_GET "GET "
#define HTTP_STARTLINE_PUT "PUT "
#define HTTP_STARTLINE_POST "POST "

#define HTTP_HEADER_ENDER "\r\n"

#define HTTP_REP_STARTLINE_200 "HTTP/1.1 200 OK"

#define HTTP_MAX_BODY_LENGTH		6 * 1024
#define HTTP_MAX_CACHEDATA_LENGTH	20 * 1024 * 1024
//	modified in 2010,Jan,5
//	In most time, it is "200 OK", sometimes it is  "200 HTTP OK", and sometimes it is "200 " + unknown characters,
//	so, use the common part "200 ".
#define HTTP_REP_CODE_200 "200 "

#define HTTP_REP_CONTINUE_100 "HTTP/1.1 100 Continue"
#define HTTP_REP_CODE_206 "206 Partial Content"

#define HTTP_BOUNDARY "Content-Type: multipart/form-data; boundary="

const char* g_szHeaderItems[] = {"User-Agent", "Accept-Encoding"};//These items will be put to PC by HTTPE.

HttpMsgParserMap CHttpMsg::m_sParserMap;

HttpMsgParserMap::HttpMsgParserMap()
{
	//	for header
	m_headerParser[HTTP_HEADER_HOST] = &CHttpMsg::ParseHost;
	m_headerParser[HTTP_HEADER_CONTENT_TYPE] = &CHttpMsg::ParseContentType;
	m_headerParser[HTTP_HEADER_CONTENT_LENGTH] = &CHttpMsg::ParseContentLen;
	m_headerParser[HTTP_HEADER_CONTENT_ENCODING] = &CHttpMsg::ParseContentEncoding;
	m_headerParser[HTTP_HEADER_CONTENT_DISPOSITION] = &CHttpMsg::ParseContentDisposition;
	m_headerParser[HTTP_HEADER_REFERER] = &CHttpMsg::ParseReferer;
	m_headerParser[HTTP_HEADER_TRANSFER_ENCODING] = &CHttpMsg::ParseTransferEncoding;

	//	for request start line
	m_StartlineParser[HTTP_STARTLINE_GET] = &CHttpMsg::ParseGET;
	m_StartlineParser[HTTP_STARTLINE_PUT] = &CHttpMsg::ParsePUT;
	m_StartlineParser[HTTP_STARTLINE_POST] = &CHttpMsg::ParsePOST;
}

HttpMsgParserMap::~HttpMsgParserMap()
{
	m_headerParser.clear();
	m_StartlineParser.clear();
}

HttpParserType HttpMsgParserMap::GetHeaderParser(const string& headertype)
{
	string strTemp = headertype; 
	std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), towupper);

	map< string, HttpParserType >::iterator itor = m_headerParser.find(strTemp) ;
	if (itor != m_headerParser.end())
	{
		return itor->second;
	}

	return NULL;
}

HttpParserType HttpMsgParserMap::GetStartlineParser(const string& type)
{
	map< string, HttpParserType >::iterator it = m_StartlineParser.find(type);
	if( it != m_StartlineParser.end() )
	{
		return it->second;
	}

	return NULL;
}

CHttpMsg::CHttpMsg(BOOL request):
	m_bRequest(request),
	m_type(HTTP_UNSET),
	m_requestType(HTTP_UNSET),
	m_procResult(RESULT_UNSET),
	m_pos(string::npos),
	m_length(0),
	m_headerParsed(FALSE),
	m_dwContentlen(0),
	m_bRecvBody(FALSE),
	m_Sock(0),
	m_bIsHttps(false),
	m_dwLastBodyLength(0),
	m_dwRestChunkLen(0),
	m_bHandled(FALSE)
{
}

CHttpMsg::~CHttpMsg()
{
}

void CHttpMsg::AddTcpData(const string& tcpdata)
{
	if (!tcpdata.length() || m_httpdata.length() > HTTP_MAX_CACHEDATA_LENGTH)//Limit the max size of cache data
	{
		return;
	}

	if(IsProcess(L"firefox.exe") && tcpdata.find(HTTP_REP_STARTLINE_200) != string::npos && !m_httpdata_firefox.empty())
	{
		string strTemp = tcpdata;

		boost::replace_all(strTemp, "\n\n", "\r\n");
		
		if(strTemp.find(m_httpdata_firefox) == 0)
		{
			g_log.Log(CELOG_DEBUG, "HTTPE::This is not file data, Firefox.");

			return;
		}
	}

	if(IsProcess(L"firefox.exe") && tcpdata.find(HTTP_REP_STARTLINE_200) != string::npos)
	{
		m_httpdata_firefox = tcpdata;
	}

	/*******************************************************************************
	FIX BUG878
	For HTTPS of Firefox,
	it will loose "\r\n" before "Content-Type: multipart/form-data; boundary=".
	Don't know why.
	So, we added code to make sure there is "\r\n" before "Content-Type: multipart/form-data; boundary=".
	it will only impact HTTPS of Firefox.
	********************************************************************************/
	if(IsProcess(L"firefox.exe") && m_bIsHttps && !m_headerParsed && tcpdata.find(HTTP_BOUNDARY) == 0)
	{
		if(m_httpdata.length() > 2 && memcmp(m_httpdata.substr(m_httpdata.length() - 2, 2).c_str(),  "\r\n", 2) != 0)
		{
			m_httpdata += "\r\n";
		}
	}

	if ( m_pos == string::npos && m_length == 0 )
	{
		//	previous data all has been parsed
		m_pos = m_httpdata.length();
		m_length = tcpdata.length(); 
		m_httpdata.append(tcpdata.c_str(),tcpdata.length() ) ;
		return;
	}

	m_length += tcpdata.length();
	m_httpdata.append(tcpdata.c_str(),tcpdata.length() ) ;
	return;
}

void CHttpMsg::SetReqRmt(const string& reqRemote)
{
	m_requestRemote = reqRemote;
}

void CHttpMsg::SetReqType(HTTP_MSG_TYPE reqType)
{
	m_requestType = reqType;
}

HTTP_MSG_TYPE CHttpMsg::GetType()
{
	return m_type;
}

HTTP_MSG_TYPE CHttpMsg::GetReqType()
{
	return m_requestType;
}

void CHttpMsg::GetRmt(string& rmt, BOOL bWithPrefix)
{
	if(m_uri.length() > 0 && m_uri[0] != '/')
	{
		m_uri = '/' + m_uri;
	}
	if( m_host.empty() )
	{
		wstring strIP = GetPeerIP( m_Sock ) ;
		m_host = string( strIP.begin(),strIP.end() )  ;
	}
	if(bWithPrefix)
	{
		string strPrefix;
		strPrefix = HTTP_PREFIX;
		if(GetHttpsFlag())
		{
			strPrefix = HTTPS_PREFIX;
		}
		
		rmt = UrlDecode(strPrefix + m_host + m_uri);
		return;
	}
	else
	{
		rmt = UrlDecode(m_host + m_uri);
		return;
	}
}

void CHttpMsg::GetNavigationURL(string& url)
{
	//	use "host + url" as navigation url.
	//	do not use refer, refer is the url of the previous request.
		return GetRmt(url, TRUE);
	}

void CHttpMsg::GetReqRmt(string& ReqRmt, BOOL bWithPrefix)
{
	if(bWithPrefix)
	{
		if(GetHttpsFlag())
		{
			ReqRmt = HTTPS_PREFIX + m_requestRemote;
		}
		else
		{
		ReqRmt = HTTP_PREFIX + m_requestRemote;
		}
		return;
	}
	else
	{
		ReqRmt = m_requestRemote;
		return;
	}
}

HTTP_MSG_PROCESS_RESULT CHttpMsg::GetProcResult()
{
	return m_procResult;
}

void CHttpMsg::SetProcResult(HTTP_MSG_PROCESS_RESULT result)
{
	m_procResult = result;
}

BOOL CHttpMsg::ParseMsg()
{
	if(GetHandledStatus())
	{//Don't need to handle this MESSAGE again, it was handled in a previous PACKAGE.
		return FALSE;
	}

	//	parse start line
	if (m_type == HTTP_UNSET)
	{
		string startline;
		if (!GetHeader(startline))
		{
			//	impossible
			return FALSE;
		}
		//We need to continue to parse the header if it is "100 continue".
		if(startline.find(HTTP_REP_CONTINUE_100) == 0)
		{
			if (!GetHeader(startline))
			{
				return FALSE;
			}
		}

		if(startline.length() <= 2)
		{
			return FALSE;
		}

		if ( !ParseStartline(startline) )
		{
			return FALSE;
		}
	}

	//	parse header
	if (m_headerParsed == FALSE)
	{
		string header;
		while ( GetHeader(header) )
		{
			if (header == HTTP_HEADER_ENDER)//The header will be end with "\r\n\r\n". It means there aren't any more head lines if the "header" is "\r\n".
			{
				m_headerParsed = TRUE;

				if ( m_pos != string::npos && m_length != 0 )
				{
					//	header over, to parse body
					return ParseBody();
				}
				//	header over, no body yet
				ClearParsedData();
				return TRUE;
			}

			if ( !ParseHeader(header) )
			{
				g_log.Log(CELOG_DEBUG, "HTTPE::ParseHeader fail, Header:[%s]\r\n", header.c_str()); 
				return FALSE;
			}
		}

		//	no header yet
		ClearParsedData();
		return TRUE;
	}

	//	parse body
	if ( m_pos == string::npos || m_length == 0 )
	{
		return TRUE;	
	}
	return ParseBody();
}

BOOL CHttpMsg::GetHeader(string& header)
{
	if ( m_pos == string::npos || m_length == 0 || ( m_pos + m_length ) > m_httpdata.length() )
	{
		return FALSE;
	}

	string::size_type index = m_httpdata.find("\r\n", m_pos);

	if (index == string::npos)
	{
		return FALSE;
	}

	//	set header
	header = m_httpdata.substr(m_pos, index - m_pos + 2);

	//	update for next parse
	if ( m_httpdata.length() == (index + 2) )
	{
		//	\r\n is the last two characters in message
		m_pos = string::npos;
		m_length = 0;
	}
	else
	{
		m_pos = index + 2;
		m_length = m_httpdata.length() - m_pos;
	}

	//Save the header items to m_mapAttributes.
	std::string::size_type uIndex = header.find(HTTP_HEADER_SEPARATOR);
	if( uIndex != string::npos)
	{
		bool bRet = false;
		for(unsigned i = 0; i < _countof(g_szHeaderItems); i++)
		{
			if(header.length() > strlen(g_szHeaderItems[i]) && _memicmp(header.c_str(), g_szHeaderItems[i], strlen(g_szHeaderItems[i])) == 0)
			{
				bRet = true;
				break;
			}
		}

		if(bRet)
		{
		string subStr = header.substr(0, uIndex) ;
		wstring HeaderKey = wstring( subStr.begin(), subStr.end() );
		subStr = header.substr(uIndex + 2, header.length() - uIndex - 2) ;
		wstring HeaderValue = wstring( subStr.begin(), subStr.end() );

		m_mapAttributes[HeaderKey] = HeaderValue;
	}
	}
	return TRUE;
}

BOOL CHttpMsg::ParseHeader(const string& header)
{
	string::size_type index = header.find(": ");
	if (index == string::npos)
	{
		return FALSE;
	}

	HttpParserType pFunc = m_sParserMap.GetHeaderParser( header.substr(0, index + 2) );
	if (!pFunc)
	{
		//	it is possible, the header is not a desired one, ignore it
		return TRUE;
	}
	return (this->*pFunc)(header);
}

BOOL CHttpMsg::ParseStartline(const string& startline)
{
	if (m_bRequest)
	{
		string::size_type begin = startline.find(" ");
		if (begin == string::npos)
		{
			return FALSE;
		}
		HttpParserType pParser = m_sParserMap.GetStartlineParser( startline.substr(0, begin + 1) );
		if (!pParser)
		{
			//	impossible
			return FALSE;
		}
		return (this->*pParser)(startline);
	}

	//	start line of response, parse m_type
	string::size_type begin = startline.find(HTTP_REP_CODE_200);
	m_type = ( begin == string::npos ) ? HTTP_NO_200 : HTTP_200_OK;

	if(startline.find(HTTP_REP_CODE_206) != string::npos)
	{
		m_type = HTTP_200_OK;
	}
	/********************************************************************
	We found an interesting issue of HTTPS of Chrome.
	It will generate some "unknown" characters in the HTTP response.
	like:
	HTTP/1.1 200 ?~XXX
	Date: 2009/12-31
	....
	These "strange" characters will only appear after "200"
	We only saw this problem on mail.cn.nextlabs.com.
	Don't know why.
	Solution: ignore these characters, we only check "200" for Chrome.

	********************************************************************/
	if(IsProcess(L"chrome.exe") && startline.find("200") != string::npos)
	{
		m_type = HTTP_200_OK;
	}
	return TRUE;
}

//	start line parser
BOOL CHttpMsg::ParseGET(const string& startline)
{
	//	parse m_type
	m_type = HTTP_GET;

	//	parse m_uri
	string::size_type begin = startline.find(" ");
	if (begin == string::npos)
	{
		return FALSE;
	}
	string::size_type end = startline.find(" ", begin + 1);
	if (end == string::npos)
	{
		return FALSE;
	}
	m_uri = startline.substr(begin + 1, end - begin - 1);
	return TRUE;
}

BOOL CHttpMsg::ParsePUT(const string& startline)
{
	//	parse m_type
	m_type = HTTP_PUT;

	//	parse m_uri
	string::size_type begin = startline.find(" ");
	if (begin == string::npos)
	{
		return FALSE;
	}
	string::size_type end = startline.find(" ", begin + 1);
	if (end == string::npos)
	{
		return FALSE;
	}
	m_uri = startline.substr(begin + 1, end - begin - 1);
	return TRUE;
}

BOOL CHttpMsg::ParsePOST(const string& startline)
{
	//	parse m_type
	m_type = HTTP_POST;

	//	parse m_uri
	string::size_type begin = startline.find(" ");
	if (begin == string::npos)
	{
		return FALSE;
	}
	string::size_type end = startline.find(" ", begin + 1);
	if (end == string::npos)
	{
		return FALSE;
	}
	m_uri = startline.substr(begin + 1, end - begin - 1);
	return TRUE;
}

BOOL CHttpMsg::ParseHost(const string& header)
{
	string::size_type index = header.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = header.find("Host: ");
	if (start == string::npos)
	{
		return FALSE;
	}
	if (start >= index)
	{
		return FALSE;
	}
	m_host = header.substr(start + 6, index - start - 6);
	return TRUE;
}

BOOL CHttpMsg::ParseContentType(const string& header)
{
	const string tmp = "content-type: ";
	string tmpHeader = header;
	std::transform(tmpHeader.begin(), tmpHeader.end(), tmpHeader.begin(), tolower);
	string::size_type index = tmpHeader.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = tmpHeader.find(tmp);
	if (start == string::npos)
	{
		return FALSE;
	}
	//	get content type
	if (start >= index)
	{
		return FALSE;
	}
	m_contenttype = header.substr(start + tmp.length(), index - start - tmp.length());

	//	special case
	if (string::npos != m_contenttype.find("multipart/form-data"))
	{
		//	get boundary
		const string boundary = "boundary=";
		start = m_contenttype.find(boundary);
		if (start != string::npos)
		{
			m_boundary = "--" + m_contenttype.substr(start + boundary.length(), m_contenttype.length() - start + boundary.length());
		}
	}

	return TRUE;
}

BOOL CHttpMsg::ParseContentLen(const string& header)
{
	const string tmp = "Content-Length: ";
	string::size_type index = header.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = header.find(tmp);
	if (start == string::npos)
	{
		return FALSE;
	}
	if (start >= index)
	{
		return FALSE;
	}
	string len = header.substr(start + tmp.length(), index - start - tmp.length());
	m_dwContentlen = atoi(len.c_str());
	return TRUE;
}

BOOL CHttpMsg::ParseContentEncoding(const string& header)
{
	const string tmp = "Content-Encoding: ";
	string::size_type index = header.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = header.find(tmp);
	if (start == string::npos)
	{
		return FALSE;
	}
	if (start >= index)
	{
		return FALSE;
	}
	m_contentencoding = header.substr(start + tmp.length(), index - start - tmp.length());
	return TRUE;
}

BOOL CHttpMsg::ParseContentDisposition(const string& header)
{
	const string tmp = "Content-Disposition: ";
	const string tmp_lowercase = "Content-Disposition: ";

	string::size_type index = header.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = header.find(tmp);
	if (start == string::npos)
	{
		//	can not find "Content-Disposition: ",
		//	it is not compatible with rfc 2616.
		//	but in fact, I found sometimes there will be "Content-disposition: ",
		//	so, I also let HTTPE support it.
		//	try to see if there is "Content-disposition: ".
		start = header.find(tmp_lowercase);
		if (start == string::npos)
		{
		return FALSE;
	}
	}
	if (start >= index)
	{
		return FALSE;
	}
	m_contentdisposition = header.substr(start + tmp.length(), index - start - tmp.length());
	return TRUE;
}

BOOL CHttpMsg::ParseTransferEncoding(const string& header)
{
	const string tmp = "Transfer-Encoding: ";
	string::size_type index = header.find("\r\n");
	if (index == string::npos)
	{
		return FALSE;
	}
	string::size_type start = header.find(tmp);
	if (start == string::npos)
	{
		return FALSE;
	}
	if (start >= index)
	{
		return FALSE;
	}
	m_transferencoding = header.substr(start + tmp.length(), index - start - tmp.length());
	return TRUE;
}

void CHttpMsg::ClearParsedData()
{
	if ( m_pos == string::npos && m_length == 0 )
	{
		m_httpdata.clear();
	}
	else
	{
		m_httpdata = m_httpdata.substr(m_pos, m_length);
		m_pos = 0;
	}
}

BOOL CHttpMsg::ParseBody()
{
	if (m_pos == string::npos || !m_length)
	{
		return TRUE;
	}

	if (m_contentencoding == m_encoding)
	{
		//	save zip body for next time usage
		//	and try to unzip data
		m_zippedbody += m_httpdata.substr(m_pos, m_length);

		//	try to unzip data
		string unzippedData;
		if (m_chunked == m_transferencoding)
		{
			//	try compose chunked data
			string chunkdata;
			if ( FALSE == ComposeChunkedData(m_zippedbody, chunkdata) )
			{
				return FALSE;
			}
			//	unzip body
			UnzipBody(chunkdata, unzippedData);
		}
		else
		{
			UnzipBody(m_zippedbody, unzippedData);
		}

		if(m_dwLastBodyLength < 0 || m_dwLastBodyLength > unzippedData.length())//fix bug10856
		{
			g_log.Log(CELOG_DEBUG, L"HTTPE::The unzipped body was shorter than index: unzipped body length: %d, index: %d", unzippedData.length(), m_dwLastBodyLength);
			return FALSE;
		}

		//	sub the new unzipped data with the old one, and append the new part into m_body
		m_body += unzippedData.substr(m_dwLastBodyLength, unzippedData.length() - m_dwLastBodyLength);

		//	save the current unzipped data length before return.
		m_dwLastBodyLength = (DWORD)unzippedData.length();
	}
	else
	{
		if( m_httpdata.length() > m_pos )
		{
			string strTemp = m_httpdata.substr(m_pos, m_length) ;
			if (m_chunked == m_transferencoding)
			{
				//	try compose chunked data
				//	check m_dwRestChunkLen, only compose when it is 0
				if(!m_dwRestChunkLen)
				{
				string chunkdata;
				if ( FALSE == ComposeChunkedData(strTemp, chunkdata) )
				{
					return FALSE;
				}
				m_body += chunkdata ; 
			}
			}
			else
			{
			m_body.append( strTemp.c_str(), m_length ) ; 
		}
	}
	}

	//	body data has been got by m_body, m_httpdata is no need
	m_httpdata.clear();
	m_pos = string::npos;
	m_length = 0;

	return ParseUnzippedBody();
}

BOOL CHttpMsg::ParsePlainBody()
{
	if (!m_body.length())
	{
		return TRUE;
	}

	if(m_bRecvBody && m_filedatas.size() > 0)
	{
		if(IsProcess(L"Chrome.exe") && m_bIsHttps)//fix bug884
		{
			m_body.clear();

			return TRUE;
		}

		if( m_filedatas.back().filedata.length() < m_dwMaxFileCache)
		{
			m_filedatas.back().filedata += m_body;
		}
	
		m_body.clear();

		return TRUE;
	}

	if (!m_filedatas.size())
	{
		HTTP_MULTI_FILEDATA file;
		file.filedata = m_body.substr( 0, min( m_body.length(), m_dwMaxFileCache ) );
		string ReqRmt;
		GetReqRmt(ReqRmt, TRUE);
		string filename =  UrlDecode(ReqRmt);
		file.filename = filename ;
		m_filedatas.push_back( file );

		m_bRecvBody = TRUE;
	}
	else
	{
		//	already parsed necessary data
		//	do nothing
	}

	//	body parse task finished
	m_body.clear();
	return TRUE;
}

BOOL CHttpMsg::ParsePotentialVermeerRPCBody()
{
	const string cGetMethod = "get document";
	const string cFileSize = "vti_filesize";

	if (!m_body.length())
	{
		return TRUE;
	}

	if (!m_filedatas.size())
	{
		string::size_type nMethod = m_body.find(cGetMethod) ;
		if (string::npos == nMethod)
		{
			//	this body is not the desired one.
			return TRUE;
		}
	
		//	get file size
		string::size_type index = m_body.find(cFileSize);
		if (string::npos == index)
		{
			return TRUE;
		}
		const string token = "<li>IR|";
		string::size_type start = m_body.find(token, index + 1);
		if (string::npos == start)
		{
			return TRUE;
		}
		string::size_type end = m_body.find("\n", start + 1);
		if (string::npos == end || end <= ( start + token.length() ) )
		{
			return TRUE;
		}
		string len = m_body.substr( start + token.length(), end - start - token.length() );
		long dLen = ::atol( len.c_str() ) ;
		const string htmlEnder = "</html>";
		start = m_body.find(htmlEnder, start + 1);
		string strSub = m_body.substr( nMethod,start) ;
		//	get file data
		string strURL ;
		string strRmtFileName ;
		if( !ParseGetMethod( strSub, strURL, strRmtFileName ) )
		{
			return TRUE ;
		}
		if (string::npos == start)
		{
			return TRUE;
		}
		if( m_body.length()  == start + htmlEnder.length()+ strlen("\n") )
		{
			return TRUE ;
		}
		HTTP_MULTI_FILEDATA data;
		
		data .filedata = m_body.substr(  start + htmlEnder.length()+ strlen("\n"), min( dLen, m_dwMaxFileCache )) ;

		
		//Check the file header of office file, If the header data is not mapper, return, get the data continue...
		string strRmtFolder;
		GetReqRmt(strRmtFolder, TRUE);
		string::size_type pos = strRmtFolder.find("_vti_bin",0 ) ;
		if( pos != string::npos )
		{
			strRmtFolder = strRmtFolder.substr( 0,pos ) ;
			strRmtFileName = strRmtFolder + strRmtFileName ;
		}
		if(  CUtility::IsOfficeFile( strRmtFileName.c_str() ) )
		{
			unsigned char office2k3[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1, '\0'};
			unsigned char office2k7[] = {0x50, 0x4B, 0x03, 0x04, 0x14, 0x00, 0x06, 0x00};
			string strOffice2k7((char*)office2k7, 8);
			pos = data .filedata.find((char*)office2k3) ;

			if(pos != string::npos)
			{
				g_log.Log(CELOG_DEBUG, "This is office 2003 file");
				data .filedata = data .filedata.substr(  pos , min( data .filedata.length() -pos, m_dwMaxFileCache )) ;
			}
			else
			{
				pos = data.filedata.find(strOffice2k7);
				if(pos != string::npos)
				{
					g_log.Log(CELOG_DEBUG, "This is office 2007 file");
					data .filedata = data .filedata.substr(  pos , min( data .filedata.length() -pos, m_dwMaxFileCache )) ;
				}
			}

		}
		
		data.filename = UrlDecode(strRmtFileName) ;
		m_filedatas.push_back(data);

		/************************************************************************
		Download files with "embedded Explorer in IE"          
		This case is interesting.
		1. IE will try to download the data;
		2. Explorer will try to write the local file after "data transfer" finished.

		There are 2 processes involved.
		So we need to resolve the communication between 2 process.
		Here, we use SharedMemory to archive this aim.
										
		Format:
		URL						length of file data		 file data					
		1024 bytes				4 bytes					 4096

													Kevin 2009-11-23
		************************************************************************/
		if(IsProcess(L"iexplore.exe"))
		{
			static CSharedMemory sm_IE;
			static BOOL bSuccess = FALSE;
			if(!bSuccess)
			{
				bSuccess = sm_IE.CreateSharedMemory(SHARED_MEMORY_NAME_DOWNLOAD_EMBEDDED_EXPLORER, MAX_FILEDATA_SIZE + MAX_URL_SIZE + 4);
			}

			if(bSuccess)
			{
				DWORD dwLen = MAX_FILEDATA_SIZE + MAX_URL_SIZE + 4;
				char* pTemp = new char[dwLen];
				if(pTemp)
				{
					memset(pTemp, 0, dwLen);
					memcpy_s(pTemp, MAX_URL_SIZE, data.filename.c_str(), data.filename.length() > MAX_URL_SIZE? MAX_URL_SIZE: data.filename.length());
					std::string::size_type uDataLen = data.filedata.length();
					memcpy_s(pTemp + MAX_URL_SIZE, 4, &uDataLen, 4);
					memcpy_s(pTemp + MAX_URL_SIZE + 4, MAX_FILEDATA_SIZE, data.filedata.c_str(), data.filedata.length() > MAX_FILEDATA_SIZE? MAX_FILEDATA_SIZE: data.filedata.length());
					sm_IE.WriteSharedMemory(pTemp, dwLen);

					delete [] pTemp;
				}
			}
		}
	}
	else
	{
		//	doing nothing
	}
	
	//	body parse task finished
	m_body.clear();
	return TRUE;
}

/*********************************************************
This function can handle the case: upload files to sharepoint
via explorer. (Web client is off).
It supports multiple files upload.
The different file will be in different package.
*********************************************************/
BOOL CHttpMsg::ParsePotentialVermeerURLEncodedBody()
{ 
	const string cPutMethod = "method=put+document";
	if (!m_body.length())
	{
		return TRUE;
	}

	if(m_bRecvBody)
	{
		if(m_filedatas.size() > 0 && m_filedatas.back().filedata.length() < m_dwMaxFileCache)
		{
			m_filedatas.back().filedata += m_body;
		}
		m_body.clear();

		return TRUE;
	}

	std::string::size_type uMethod = m_body.find(cPutMethod);
	if ( string::npos == uMethod)
	{
		//	this body is not the desired one.
		return TRUE;
	}

	//	get file data
	const string token = "\n";
	string::size_type start = m_body.find(token, uMethod);
	if ( start == string::npos || (start + token.length()) >= m_body.length() )
	{
		return TRUE;
	}

	HTTP_MULTI_FILEDATA data;
	string strMethodLine = m_body.substr(uMethod, start - uMethod);
	string strRmtURI, strFileName;
	strMethodLine = UrlDecode(strMethodLine);
	if(ParseMethod(strMethodLine, strRmtURI, strFileName))
	{
		m_uri = strRmtURI;
		data.filename = strFileName;
	}

	data.filedata = m_body.substr(  start + token.length(), min( m_body.length() - start - token.length(), m_dwMaxFileCache ) );
	m_filedatas.push_back( data );
	m_bRecvBody = TRUE;
	//	body parse task finished
	m_body.clear();

	return TRUE;
}

/********************************************************************************************************************************************************************************
We saw "Content-Type: application/octet-stream" on "google document" with Chrome (Upload).
The format like below:

POST /upload/resumableupload/AEnB2Uqtno7SHP4IkR6eH3N2N3pU_fIUewSPNo7MNUll9w3MGxJ7Tfz5aK7RC0jqh4JAmyUepzy0A7eG_33o2UvELAyRDJJoVQ/0 HTTP/1.1
Host: docs.google.com
Connection: keep-alive
User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/532.0 (KHTML, like Gecko) Chrome/3.0.195.38 Safari/532.0
Content-Length: 125
Pragma: no-cache
Cache-Control: no-cache
X-HTTP-Method-Override: PUT
Content-Type: application/octet-stream
Accept-Encoding: gzip,deflate,sdch
Cookie: NID=16=C4hNrhmBMo3V-77dLL00lBWn6P9s1-USc3wphrM1PI9_qXDeslvJCrwwIlXBuwvUaFacbltS7eyxudf1vMGxnrOlNgyC7DP7nXgeHnXLawzuzuIEiOjbJak4nh83nCWi; PREF=ID=75396d6a14f2915a:U=a05cd49073e3bc37:TM=1253600084:LM=1259028433:GM=1:S=sWs3MI2HsqWH36gi; rememberme=true; HSID=AIpNz7C9cVZXl407p; SID=DQAAAIUAAACUpEBzd7QJKcxFideWNIekgBqeMfO9_-ozznmVQAi2iB2mdAI-yKg2BlHcobLTD-hgniqsQZuPE-5CfIeem0P84AWtY3YNA0578tGX0H5Q0Y-rPdCGR6jcPucg5X3sG3Y-AzzD1rtLK6sdNs96lSxAK-gj2TISZ16qlA_dIythOEMhs9PGOQZDHmFMoL9IXpE; WRITELY_SID=DQAAAIgAAAApkIpkfNlOYhdc5Ai-yixG_HWSouLH4GkH3lWEgX4UKdvAzMgJtun5Ju5QEuX5QgvLp5hkcUEVj0XVZ5sWa6ldfMp95UCBrozUAs_TSmwL6SCSVkmShNv9p3pwmxP2T829ZH3QJQLF3TCnnvXyHQiYFHL8oYDeCz4cM6LiAuAtTujKWjduhqFBV1adiH2dtcY
Accept-Language: zh-CN,zh;q=0.8
Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3

hello world
aaaaaaaaaaaaaaaaaaaaa12345
aaaaaaaaaaaaaaaaaaaaa12345
aaaaaaaaaaaaaaaaaaaaa12345
aaaaaaaaaaaaaaaaaaaaa12345
********************************************************************************************************************************************************************************/
BOOL CHttpMsg::ParseOctetBody()
{
	if (!m_body.length())
	{
		return TRUE;
	}

	if(m_bRecvBody)
	{
		if(m_filedatas.size() > 0 && m_filedatas.back().filedata.length() < m_dwMaxFileCache)
		{
			m_filedatas.back().filedata += m_body;
		}
		m_body.clear();

		return TRUE;
	}

	//	get file data
	HTTP_MULTI_FILEDATA data;
	
	if(m_body.length() > m_dwMaxFileCache)
	{
		data.filedata = m_body.substr( 0, m_dwMaxFileCache );
	}
	else
	{
		data.filedata = m_body;
	}
	m_filedatas.push_back( data );
	m_bRecvBody = TRUE;
	//	body parse task finished
	m_body.clear();

	return TRUE;
}

/**********************************************************
function name: ParseMultipartFormBody
feature:
	Handle the case: upload files to sharepoint with IE, upload file to gmail...
	It supports multiple files.
	Below is a sample:

	POST /sites/engineering/docs/_vti_bin/shtml.dll/Engineering%20Documents/Forms/Upload.aspx HTTP/1.1
	Content-Type: multipart/form-data; boundary=-----------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	Content-Length: 202246
	User-Agent: Mozilla/4.0 (compatible; MSIE 5.0; Windows NT 5.0) [Sharepoint Active-X Upload Control]
	Host: intranet.cn.nextlabs.com
	Connection: Keep-Alive
	Cache-Control: no-cache
	Cookie: MSOWebPartPage_AnonymousAccessCookie=80; WSS_KeepSessionAuthenticated=80
	Authorization: NTLM TlRMTVNTUAADAAAAGAAYAHAAAAAYABgAiAAAAAoACgBIAAAACgAKAFIAAAAUABQAXAAAAAAAAACgAAAABYKIogUBKAoAAAAPTgBYAFQAQwBOAGsAegBoAG8AdQBWADIAMAAwAC0ASwBaAEgATwBVANiUeV6470hIAAAAAAAAAAAAAAAAAAAAAG7PdOg2CnWsw0RNm1Ni5QNhqzoHlSj7Gg==

	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------

	Content-Disposition: form-data; name="destination"

	/sites/engineering/docs/Engineering Documents/HTTPE test

	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	...
	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------

	Content-Disposition: form-data; name="Attachment"; thicket="0"; filename="C:\Documents and Settings\kzhou\Desktop\1.GIF"

	Content-Type: image/gif

	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-file content
	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	...
	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	Content-Disposition: form-data; name="destination"

	/sites/engineering/docs/Engineering Documents/HTTPE test

	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	...
	-------------badda0d1-a2ca-4569-a85b-9d66342e3965-------------
	Content-Disposition: form-data; name="Attachment"; thicket="0"; filename="C:\Documents and Settings\kzhou\Desktop\2.GIF"
	Content-Type: image/gif

	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-file content
	-------------badda0d1-a2ca-4569-a85b-9d66342e3965---------------
	HTTP/1.1 302 Object Moved
**********************************************************/
BOOL CHttpMsg::ParseMultipartFormBody()
{ 
	if (!m_body.length())
	{
		return TRUE;
	}
	if (!m_boundary.length())
	{
		//	impossible
		return FALSE;
	}

	if(m_bRecvBody)
	{
		if(m_filedatas.back().filedata.length() < m_dwMaxFileCache)
		{
			m_filedatas.back().filedata += m_body;
		}

		if(m_boundary.length() > 0 && m_body.find(m_boundary) != string::npos)
		{//It means there are still some files, need to parse again.
			m_bRecvBody = FALSE;
		}
		else
		{
			m_body.clear();
			return TRUE;
		}
		
	}

	const string disp = "Content-Disposition:";
	const string token = "filename=";
	const string ender = "\r\n";
	const string destination = "name=\"destination\"";
	string filename;

	string::size_type disp_pos = m_body.find(disp);
	if (disp_pos == string::npos)
	{
		return TRUE;
	}

	for(;;disp_pos += disp.length())
	{
		string::size_type start = m_body.find(disp, disp_pos);
		if (string::npos == start)
		{
			//	all data in current TCP packet are useless, remove them
			m_body.clear();
			break;
		}

		disp_pos = start;

		string::size_type end = m_body.find(ender, start + 1);
		if (string::npos == end)
		{
			//	all data before "Content-Disposition:" are useless, remove them
			m_body = m_body.substr(disp_pos, m_body.length() - disp_pos);
			break;
		}

		//try to find destination
		string strDest;
		string::size_type nDest = m_body.find(destination + ender + ender, start);
		if(nDest != string::npos)
		{
			nDest = nDest + (destination + ender+ ender).length();
			end = m_body.find(ender, nDest);
			if(end != string::npos)
			{
				strDest = m_body.substr(nDest, end - nDest);	
				if(strDest.length() > 0)
				{
					m_uri = strDest;
				}
			}
			continue;
		}

		//try to find "filename"
		start = m_body.find(token, start + 1);
		if (start == string::npos || start > end)
		{
			//	can not find filedata, jump to next "Content-Disposition:"
			
			continue;
		}
		start = m_body.find("\"", start + 1);
		if (start == string::npos)
		{
			//	impossible
			return FALSE;
		}
		end = m_body.find("\"", start + 1);
		if ( end == string::npos || end <= start )
		{
			//	impossible
			return FALSE;
		}
		if ( (end - start) == 1 )
		{
			//	can not find filedata, jump to next "Content-Disposition:"
			continue;
		}
		filename = m_body.substr(start + 1, end - start - 1);
		start = m_body.find( ender + ender, end );
		if ( start == string::npos )
		{
			//	all data before "Content-Disposition:" are useless, remove them
			m_body = m_body.substr(disp_pos, m_body.length() - disp_pos);
			return TRUE;
		}
		end = m_body.find(m_boundary, start);
		if ( end == string::npos )
		{
			//	file data is not finished until the end of this TCP packet
		        std::string::size_type len = m_body.length() - ( start + (ender + ender).length() );
			HTTP_MULTI_FILEDATA file;
			file.filedata = m_body.substr( start + (ender + ender).length(), min(len, m_dwMaxFileCache) );
			file.filename = UrlDecode(filename);
			m_filedatas.push_back(file);

			m_bRecvBody = TRUE;
			m_body.clear();
			
			return TRUE;
		}

		disp_pos = end;
		//	file data is not finished until the boundary
		std::string::size_type len = end - start - (ender + ender).length() - ender.length();
		HTTP_MULTI_FILEDATA file;
		file.filedata = m_body.substr( start + (ender + ender).length(), min(len, m_dwMaxFileCache) );

		file.filename = filename;
		
		m_filedatas.push_back(file);

		//	jump to next "Content-Disposition:"
		continue;
	}

	return TRUE;
}

BOOL CHttpMsg::ParseUnzippedBody()
{
	//	for bug 13987
	//	obviously if the body type is "MultipartForm", we can't set handled status here.
	//	because in the body, there might be more than one uploaded file's data,
	//	we need to handle all the body content otherwise we may miss to process the latter uploaded file's data.
	//	How about other body types? the answer is I don't know, if you find other body types also 
	//	can contain multiple uploaded file's data or multiple downloaded file's data, 
	//	you must modify below condition expression.
	if ( !(m_type == HTTP_POST && m_boundary.length()) )
	{
		if(m_body.length() > HTTP_MAX_BODY_LENGTH)
		{
			//Don't need to parse the current MESSAGE again, since the "body" is enough for HTTP parser.
			SetHandledStatus(TRUE);
		}
	}
	//	end for bug 13987

	/********************************************************************************
	upload to share point, when web client service is on, and using windows explore.
	Actually, we can't resolve this case.
	In this case, IE will try to open the local file and read the file content, and 
	then IE will transfer this task to svchost.ext, svchost will send a "PUT" request
	to HTTP server and upload the file data.
															Kevin 2009-11-17
	********************************************************************************/
	if (m_type == HTTP_PUT && m_dwContentlen)
	{
		if (ParsePlainBody() == FALSE)
		{
			return FALSE;
		}
	}
	//	upload to yahoo mail, upload to sharepoint via IE
	else if (m_type == HTTP_POST && m_boundary.length())
	{
		if (ParseMultipartFormBody() == FALSE)
		{
			return FALSE;
		}
	}
	//	upload to share point, when web client service is off, and using windows explore
	else if (m_type == HTTP_POST && m_vermeerurlencoded == m_contenttype)
	{
		if (ParsePotentialVermeerURLEncodedBody() == FALSE)
		{
			return FALSE;
		}
	}
	//google document, upload (Chrome) 
	else if (m_type == HTTP_POST && m_octet == m_contenttype)
	{
		if(ParseOctetBody() ==FALSE)
		{
			return FALSE;
		}
	}
	//	download from share point, when web client service is off, and using windows explore
	else if (m_type == HTTP_200_OK && m_requestType == HTTP_POST && m_contenttype == m_vermeerrpc)
	{
		if (ParsePotentialVermeerRPCBody() == FALSE)
		{
			return FALSE;
		}
	}
	//Share point open with explorer
	else if (m_type == HTTP_200_OK && m_requestType == HTTP_POST )
	{
		if (ParsePotentialVermeerRPCBody() == FALSE)
		{
			return FALSE;
		}
	}
	//	download from yahoo mail
	else if ( m_type == HTTP_200_OK && m_requestType == HTTP_GET && string::npos != m_contentdisposition.find(m_attachment) )
	{
		if (ParsePlainBody() == FALSE)
		{
			return FALSE;
		}
	}
	//SharePoint download
	else if ( m_type == HTTP_200_OK && m_requestType == HTTP_GET  )
	{
		if (ParsePlainBody() == FALSE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CHttpMsg::UnzipBody(const string& input, string& output)
{
	CGZipper& zipper = CGZipper::Instance();
	zipper.UNZipData(input, output);

	return TRUE;
}

BOOL CHttpMsg::ComposeChunkedData(const string& input, string& output)
{
	/*
	Chunked-Body   =	*chunk
				last-chunk
				trailer
				CRLF

	chunk          =	chunk-size [ chunk-extension ] CRLF
				chunk-data CRLF
	chunk-size     =	1*HEX
	last-chunk     =	1*("0") [ chunk-extension ] CRLF

	chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
	chunk-ext-name = token
	chunk-ext-val  = token | quoted-string
	chunk-data     = chunk-size(OCTET)
	trailer        = *(entity-header CRLF)

	*/

	const string crlf = "\r\n";
	const string space = " ";
	DWORD chunklen = 0;
	string::size_type chunkbegin = 0;
	string::size_type index = 0;
	size_t totallen = 0;

	while (TRUE)
	{
		//	try to get chunk data len, the line is end with crlf
		index = input.find(crlf, chunkbegin);
		if ( index == string::npos || index <= chunkbegin || index >= input.length() )
		{
			return TRUE;
		}
		//	try to see if there is [ chunk-extension ]
		string::size_type tempindex = input.find(space, chunkbegin);
		if ( tempindex != string::npos && tempindex < index && tempindex > chunkbegin )
		{
			//	this means there is [ chunk-extension ]
			//	so should reset index, use the space as the end of length.
			index = tempindex;
		}
		
		//	the length is hex, need to translate it into Decimal
		chunklen = 0;
		sscanf_s(input.substr(chunkbegin, index - chunkbegin).c_str(), "%x"  ,&chunklen );
		if (!chunklen)
		{
			//	the chunk data len is 0,
			//	so it is the last-chunk
			return TRUE;
		}
		//	try to find the beginning the chunked data
		//	it is start after crlf
		index = input.find(crlf, chunkbegin);
		if ( index == string::npos )
		{
			//	case not easy to see.
			//	even it is true, we wait for next time to process, in next time, this case would not happen again.
			//	so directly return here.
			return TRUE;
		}
		if ( (index + crlf.length() + chunklen) > input.length() )
		{
			//	the chunk data is not finished until the end of the TCP packet
			//	this case may not be easy to see
			//	tell me when it happen
		        std::string::size_type len = input.length() - index - crlf.length();

			//	set remained data length
			m_dwRestChunkLen = (DWORD)(chunklen - len);

			len = min(m_dwMaxFileCache - totallen, len);
			output += input.substr(index + crlf.length(), len);
			totallen += len;
			return TRUE;
		}
		else
		{
			//	the chunk data is in this TCP packet
			size_t len = min(m_dwMaxFileCache - totallen, chunklen);
			output += input.substr(index + crlf.length(), len);
			totallen += len;
			if (totallen >= m_dwMaxFileCache)//changed by kevin 2010-1-4
			{
				//	output size is full
				g_log.Log(CELOG_DEBUG, "HTTPE::The buffer is full. size: %d", totallen);
				return TRUE;
			}

			//	all the chunked data is cached in output, try to double check if the
			//	end of this chunked data is crlf, it should be!
			//	otherwise there are some error.
			chunkbegin = input.find(crlf, index + crlf.length() + chunklen);

			if(chunkbegin == string::npos)
			{
				g_log.Log(CELOG_DEBUG, "HTTPE::Can't find the \"\r\n\".");
			}
			//	then set chunkbegin to the beginning of the next chunked data
			chunkbegin += crlf.length();
		}
	}
	return TRUE;
}

void CHttpMsg::GetLocalPaths(std::vector<std::string>& localPaths)
{
	localPaths.clear();
	list<HTTP_MULTI_FILEDATA>::iterator itr;
	for(itr = m_filedatas.begin(); itr != m_filedatas.end(); itr++)
	{
		if((*itr).result != RESULT_UNSET)
		{
			continue;
		}

		string strFilePath = (*itr).filename;
		strFilePath = UrlDecode(strFilePath);

		if(!(*itr).bHandled)
		{
		string strFileName_GB;
		CEncoding::UTF8ToGB2312(strFilePath, strFileName_GB);

		strFilePath = strFileName_GB;
		}

		if(!(strFilePath.length() > 2 && (strFilePath[1] == ':' || strFilePath.find("\\") == 0)))
		{
			//it is not an entire path, try to get the file path from data cache
			if((*itr).filedata.length() == 0)
			{
				const int cdwBufferLen = 1000;
				char buf[cdwBufferLen] = {0};
				_snprintf_s(buf, cdwBufferLen, _TRUNCATE, "HTTPE::There aren't any data in this package, maybe the file content is in next package. file name: %s, package-len: %d", 
						strFilePath.c_str(), m_dwContentlen);
				g_log.Log(CELOG_DEBUG, "%s", buf);
				continue;
			}
			CMapperMngr& mapperMgr = CMapperMngr::Instance();
			std::wstring strPath;
			if(strFilePath.length() > 0)
			{
				g_log.Log(CELOG_DEBUG, "HTTPE::Try to find local path with file name: [%s] and with data content, content len [%d]\n", strFilePath.c_str(), (*itr).filedata.length());
				strPath = mapperMgr.GetLocalPathByDataName((*itr).filedata, MyMultipleByteToWideChar(strFilePath));
			}
			if(strPath.empty())
			{
				g_log.Log(CELOG_DEBUG, "HTTPE::Try to find local path with data content only, content len [%d], file name in http data: [%s]\n", static_cast<int>((*itr).filedata.length()), strFilePath.c_str());
				strPath = mapperMgr.GetLocalPathByData((*itr).filedata);
			}
			
			if(strPath.length() > 0)
			{
				strFilePath = MyWideCharToMultipleByte(strPath);
				g_log.Log(CELOG_DEBUG, "HTTPE:: find the file path by matching file name from HTTP protocol: %s", strFilePath.c_str());
			}
			else
			{
				g_log.Log(CELOG_DEBUG, "HTTPE::Can't find the file path via content match, file name from HTTP protocol: %s", strFilePath.c_str());
				continue;
			}
		}
		localPaths.push_back(strFilePath);
		(*itr).filename = strFilePath;
		(*itr).bHandled = true;
	}
	return;
}

/*******************************************************************************
format:
method=put document:6.0.2.8164&service_name=/sites/engineering/docs&document=[document_name=Engineering Documents/HTTPE test/55555555555555.JPG;
meta_info=[]]&put_option=edit,atomic,thicket&comment=&keep_checked_out=false
*******************************************************************************/
BOOL CHttpMsg::ParseMethod(const string & strSrc, string& strRmtURI, string& strFileName)
{
	g_log.Log(CELOG_DEBUG, "HTTPE::Try to do \"ParserMethod\", %s", strSrc.c_str());

	string strSrcTemp = strSrc;
	std::transform(strSrcTemp.begin(), strSrcTemp.end(), strSrcTemp.begin(), tolower);

	const string strService = "service_name=";
	const string strDocument_name = "document_name=";
	std::string::size_type uService = strSrcTemp.find(strService);
	if(uService == string::npos)
		return FALSE;

	std::string::size_type uAnd = strSrcTemp.find("&", uService);
	if(uAnd == string::npos)
		return FALSE;

	std::string::size_type uDocument = strSrcTemp.find(strDocument_name, uAnd);
	if(uDocument == string::npos)
		return FALSE;

	std::string::size_type uSemicolon = strSrcTemp.find(";", uDocument);
	if(uSemicolon == string::npos)
		return FALSE;

	string strPart1 = strSrc.substr(uService + strService.length(), uAnd - uService - strService.length());
	if(strPart1.length() > 0 && strPart1[strPart1.length() - 1] != '/')
	{
		strPart1 += "/";
	}
	string strPart2 = strSrc.substr(uDocument + strDocument_name.length(), uSemicolon - uDocument - strDocument_name.length());

	strRmtURI = strPart1 + strPart2;

	const char* p = strrchr(strRmtURI.c_str(), '/');
	if(p)
	{
		strFileName = string(p + 1);
	}

	return TRUE;
}


void CHttpMsg::AddNewFile(HTTP_MULTI_FILEDATA file)
{
	list<HTTP_MULTI_FILEDATA>::iterator itr = m_filedatas.begin();
	for( ; itr != m_filedatas.end(); itr++)
	{
		if((*itr).filename == file.filename)
		{
			m_filedatas.erase(itr);
			break;
		}
	}

	m_filedatas.push_back(file);
}

void CHttpMsg::GetFileList( list<HTTP_MULTI_FILEDATA>& fileList )
{
	fileList = m_filedatas;
}

void CHttpMsg::SetEvalResult(const string & filepath, HTTP_MSG_PROCESS_RESULT result)
{
	list<HTTP_MULTI_FILEDATA>::iterator itr = m_filedatas.begin();
	for( ; itr != m_filedatas.end(); itr++)
	{
		if(_stricmp((*itr).filename.c_str(), filepath.c_str()) == 0 )
		{
			(*itr).result = result;
			break;
		}
	}
}
BOOL CHttpMsg::ParseGetMethod( const string& strSrc, string& strRmtURI, string& strFileName)
{
	BOOL bRet =  FALSE ;

	const string strDocument_name = "document_name=";
	std::string::size_type uDocument = strSrc.find(strDocument_name);
	if(uDocument == string::npos)
		return bRet;

	std::string::size_type uSemicolon = strSrc.find("\n", uDocument);
	if(uSemicolon == string::npos)
		return bRet;
	string fileName = strSrc.substr(uDocument + strDocument_name.length(), uSemicolon - uDocument - strDocument_name.length());
	strFileName = fileName ;
	strRmtURI =	   strFileName ;
	bRet =  TRUE ;
	return bRet;
}

BOOL CHttpMsg::ParseReferer(const string& header)
{
	const string cstrRefer = "Referer: ";
	string::size_type index = header.find("\r\n");
	if ( index == string::npos )
	{
		return FALSE;
	}
	string::size_type start = header.find(cstrRefer);
	if (start == string::npos)
	{
		return FALSE;
	}
	if ( start >= index || (index - start - cstrRefer.length()) <= 0 )
	{
		return FALSE;
	}
	m_referer = header.substr(start + cstrRefer.length(), index - start - cstrRefer.length());

	return TRUE;
}
