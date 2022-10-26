// MySock.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <stdlib.h>



int _tmain(int argc, _TCHAR* argv[])
{
	 WSADATA             wsd = {0};
    INT                 nErr = 0,
                        nStartup = 0,
                        nRet = 0;
    SOCKET              sock = INVALID_SOCKET;
    SOCKADDR_STORAGE    localaddr = {0},
                        destaddr = {0},
                        remoteaddr = {0};
    WSAOVERLAPPED       over = {0};
    WSABUF              wsabuf = {0};
    WSAMSG              wsamsg = {0};
    DWORD               dwBytes = 0,
                        dwFlags = 0;
    LPFN_WSARECVMSG     WSARecvMsg = NULL;
    LPFN_WSASENDMSG     WSASendMsg = NULL;

    nErr = WSAStartup(MAKEWORD(2, 2),&wsd);

	sock = socket(AF_INET,SOCK_STREAM,0);

	send(sock, "abcd", 4, 0);
	char buf[100] = {0};
	recv(sock, buf, 100, 0);
	return 0;
}

