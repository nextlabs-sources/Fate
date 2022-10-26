#pragma once

#include "winsock2.h"
#include <string>
#include <map>
#include "httpmsg.h"


/*
dispatcher design:

store <sock, httpRequest> or <sock, httpResponse> pairs in dispacher;
within a sock, first, request will be sent out, then response will be received in, 
another request can not be sent out unitl the previous response has been recieved in,
this is assured by HTTP protocol!

so,
if we remove the request and response immediatly,
there will always be only one request or only one response with each sock.
*/

class CHttpCollector 
{
public:
	static CHttpCollector& CreateInstance();
	/*
	only process HTTP data, and only GET, POST, PUT, and their response will be processed;												
	unprocessed data will not be affected by HTTPE, which means they will be sent out or received without any monitor or deny.
	*/
	BOOL CollectHttpMsg(BOOL send, SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg);

	/*
	remove httpMsg by socket handle.
	*/
	void RemoveHttpMsg(BOOL send, SOCKET s);

	BOOL GetHttpMsgBySocket_SendList( SOCKET s, smartHttpMsg &spHttpMsg );
	BOOL GetHttpMsgBySocket_RecvList( SOCKET s, smartHttpMsg &spHttpMsg );
	BOOL GetHttpMsgByRmtPath_RecvList( const string & strRmtPath, smartHttpMsg &spHttpMsg );
private:
	BOOL Add_ReceiveData(SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg);
	BOOL Add_SendData(SOCKET s, const string& tcpdata, smartHttpMsg& httpMsg);


private:
	CHttpCollector(void);
	CHttpCollector(const CHttpCollector&);
	void operator = (const CHttpCollector&);
	~CHttpCollector(void);

private:
	map<SOCKET, smartHttpMsg> m_mapReceiveData;
	map<SOCKET, smartHttpMsg> m_mapSendData;
};