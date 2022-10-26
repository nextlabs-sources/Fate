// MyProvider.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <ws2spi.h>
#include <mswsock.h>
#include <sporder.h>
#include <strsafe.h>

#include <stdio.h>
#include <stdlib.h>




// {C9BFDE32-37A4-4048-BBB4-5DEC9F70B33D}
static const GUID MyguID = 
{ 0xc9bfde32, 0x37a4, 0x4048, { 0xbb, 0xb4, 0x5d, 0xec, 0x9f, 0x70, 0xb3, 0x39 } };

extern wchar_t g_szDLLPath[MAX_PATH + 1];
HMODULE g_hSPIMod = NULL;



SOCKET_CONTEXT * GetSocketContext(SOCKET s)
{
	SOCKET_CONTEXT *sockContext = NULL;

   
	sockContext = (SOCKET_CONTEXT *)FindSocketContextPtr(s);
		

	return sockContext;
}

// 
// Function: WSPSend
//
// Description:
//      This function implements the WSPSend function for the IFS LSP. This routine
//      simply parses the send buffer for an HTTP GET request. If one is found it
//      is simply displayed to the debugger. This illustrates how to parse data
//      buffers. Note that this samply only intercepts the WSPSend routine since
//      we're interested in only HTTP TCP traffic. If we wanted to parse datagram
//      oriented protocols we should then also intercept WSPSendTo, WSASendMsg,
//      and TransmitPackets. Note that we don't intercept TransmitFile or 
//      TransmitPackets for this HTTP parsing since the client HTTP sides do not
//      use those APIs for sending requests (i.e. these extension functions are
//      typically used in the server response).
//
int WSPAPI
WSPSend(
        SOCKET          s,
        LPWSABUF        lpBuffers,
        DWORD           dwBufferCount,
        LPDWORD         lpNumberOfBytesSent,
        DWORD           dwFlags,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
        LPWSATHREADID   lpThreadId,
        LPINT           lpErrno
       )
{
    SOCKET_CONTEXT *sockContext = GetSocketContext(s);

	int             rc = SOCKET_ERROR;

	if(sockContext == NULL)
	{
		goto cleanup;
	}
  
    ASSERT( sockContext->Provider->NextProcTable.lpWSPSend );

    // Just pass the request along to the lower provider. NOTE: If we choose to
    //    modify the data things get a bit trickier if we substitute our own send
    //    buffer since we would need to be in the data notification path in order
    //    to know when we are able to free that memory (i.e. the lower layer has
    //    processed it and is done). In this case a non-IFS LSP is more appropriate
    //    since it intercepts all IO completion notifications.

    rc = sockContext->Provider->NextProcTable.lpWSPSend(
            s,
            lpBuffers,
            dwBufferCount, 
            lpNumberOfBytesSent,
            dwFlags,
            lpOverlapped,
            lpCompletionRoutine,
            lpThreadId,
            lpErrno
            );

	OutputDebugStringW(L"MyProvider2, WSPSend()");  

cleanup:

    return rc;
}

int WSPAPI WSPRecv(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno
    )
{
	SOCKET_CONTEXT *sockContext = GetSocketContext(s);

	int             rc = SOCKET_ERROR;

	if(sockContext == NULL)
	{
		goto cleanup;
	}s
  
    ASSERT( sockContext->Provider->NextProcTable.lpWSPRecv );

	OutputDebugStringW(L"MyProvider2, WSPRecv()");   

	rc = sockContext->Provider->NextProcTable.lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, 
									lpCompletionRoutine, lpThreadId, lpErrno);
cleanup:

    return rc;
}
//
// Function: WSPSocket
//
// Description:
//    This routine creates a socket. For an IFS LSP the lower provider's socket
//    handle is returned to the uppler layer. When a socket is created, a socket
//    context structure is created for the socket returned from the lower provider.
//    This context is used if the socket is later connected to a proxied address.
//
SOCKET WSPAPI 
WSPSocket(
        int                 af,
        int                 type,
        int                 protocol,
        LPWSAPROTOCOL_INFOW lpProtocolInfo,
        GROUP               g,
        DWORD               dwFlags,
        LPINT               lpErrno
        )
{
	
	return WSPSocketInit(af, type, protocol, lpProtocolInfo, g, dwFlags, lpErrno);
	
}

int WSPAPI 
WSPStartup(
		   WORD                wVersion,
		   LPWSPDATA           lpWSPData,
		   LPWSAPROTOCOL_INFOW lpProtocolInfo,
		   WSPUPCALLTABLE      UpCallTable,
		   LPWSPPROC_TABLE     lpProcTable
		   )
{
	OutputDebugStringW(L"MyProvider2: WSPStartup");

	WSPStartupInit(wVersion, lpWSPData, lpProtocolInfo, UpCallTable, lpProcTable, MyguID);

	lpProcTable->lpWSPSend = WSPSend;
	lpProcTable->lpWSPSocket = WSPSocket;
	lpProcTable->lpWSPRecv = WSPRecv;
		

	
	return 0;
}
