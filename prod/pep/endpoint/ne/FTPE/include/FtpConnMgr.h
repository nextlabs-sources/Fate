#pragma once
#include <list>
using namespace std;

class CFtpSocket;
class CFtpCtrlConn;
class CFtpDataConn;

class CFtpConnMgr
{
public:

	static CFtpConnMgr& Instance();
	void AddConn(SOCKET fd, bool bWouldBlock, const struct sockaddr* peername);
	void DelConn(SOCKET fd);

	void SetFtpProtocolType(SOCKET fd, FtpProtocolType pt);
	FtpProtocolType GetFtpProtocolType(SOCKET fd) const;

	ParserResult ParseSend(SOCKET fd, const string& sBuf);
	ParserResult ParseRecv(SOCKET fd, const string& sBuf);

// helpers
private:
	CFtpCtrlConn* FindCtrlConn(SOCKET fd) const;
	CFtpDataConn* FindDataConn(SOCKET fd) const;

private:
	CFtpConnMgr(void);
	CFtpConnMgr(const CFtpConnMgr&);
	void operator = (const CFtpConnMgr&);
	~CFtpConnMgr(void);

private:
	list<CFtpCtrlConn*> m_listCtrlConn;
};
