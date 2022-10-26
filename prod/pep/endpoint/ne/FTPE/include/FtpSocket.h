#pragma once

#include <winsock2.h>
#include <string>
using namespace std;

class CFtpSocket
{
public:
	explicit CFtpSocket(SOCKET fd);
	~CFtpSocket(void){}

	SOCKET GetSocket() const 
	{ return m_fd; }

	static string AddressToString(const struct sockaddr* addr, int addr_len, bool with_port);
	string GetLocalIP() const;
	string GetPeerIP() const;
	int GetLocalPort(/*int& error*/) const;
	int GetRemotePort(/*int& error*/) const;

private:
	SOCKET m_fd;
};
