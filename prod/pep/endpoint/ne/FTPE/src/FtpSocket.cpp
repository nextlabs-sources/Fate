#include "stdafx.h"
#include "FtpSocket.h"

#pragma warning(push)
#pragma warning(disable: 6386)
#include <Ws2tcpip.h>
#pragma warning(pop)

#include <errno.h>
#include <cassert>
using namespace std;

CFtpSocket::CFtpSocket(SOCKET fd)
{
	assert(fd != INVALID_SOCKET);
	m_fd = fd;
}

string CFtpSocket::AddressToString(const struct sockaddr* addr, int addr_len, bool with_port)
{
	addr_len;
	char portbuf[NI_MAXSERV];

#if 1
	{
		// Win2K fallback
		if (addr->sa_family != AF_INET)
			return "";
		char* s = inet_ntoa(((struct sockaddr_in*)addr)->sin_addr);
		if (!s)
			return "";

		string host = string(s);
		if (!with_port)
			return host;

		_snprintf_s(portbuf, NI_MAXSERV, _TRUNCATE, ":%d", (int)ntohs(((struct sockaddr_in*)addr)->sin_port));
		return host + portbuf;
	}

#else
	int res = getnameinfo(addr, addr_len, hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
	if (res) // Should never fail
		return "";

	string host = string(hostbuf);
	string port = string(portbuf);

	// IPv6 uses colons as separator, need to enclose address
	// to avoid ambiguity if also showing port
	if (with_port && addr->sa_family == AF_INET6)
		host = "[" + host + "]";

	if (with_port)
		return host + ":" + port;
	else
		return host;
#endif
}

string CFtpSocket::GetLocalIP() const
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	int res = getsockname(m_fd, (sockaddr*)&addr, &addr_len);
	if (res)
		return "";

	return AddressToString((sockaddr *)&addr, addr_len, false);
}

string CFtpSocket::GetPeerIP() const
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	int res = getpeername(m_fd, (sockaddr*)&addr, &addr_len);
	if (res)
		return "";

	return AddressToString((sockaddr *)&addr, addr_len, false);
}

int CFtpSocket::GetLocalPort() const
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	int error = getsockname(m_fd, (sockaddr*)&addr, &addr_len);
	if (error)
	{
#ifdef __WXMSW__
		error = ConvertMSWErrorCode(error);
#endif
		return -1;
	}

	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in* addr_v4 = (sockaddr_in*)&addr;
		return ntohs(addr_v4->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6* addr_v6 = (sockaddr_in6*)&addr;
		return ntohs(addr_v6->sin6_port);
	}

	error = EINVAL;
	return -1;
}

int CFtpSocket::GetRemotePort() const
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	int error = getpeername(m_fd, (sockaddr*)&addr, &addr_len);
	if (error)
	{
#ifdef __WXMSW__
		error = ConvertMSWErrorCode(error);
#endif
		return -1;
	}

	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in* addr_v4 = (sockaddr_in*)&addr;
		return ntohs(addr_v4->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6* addr_v6 = (sockaddr_in6*)&addr;
		return ntohs(addr_v6->sin6_port);
	}

	error = EINVAL;
	return -1;
}