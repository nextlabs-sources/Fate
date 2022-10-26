/***************************************************************************
Module:  transport.h
***************************************************************************/


//#ifdef __TRANSPORT_API

// MYLIBAPI should be defined in all of the DLL's source
// code modules before this header file is included. 

// All functions/variables are being exported.
//#define TRANSPORT_API extern "C" __declspec(dllexport)
//#else

// This header file is included by an EXE source code module.
// Indicate that all functions/variables are being imported.
//#define TRANSPORT_API extern "C" __declspec(dllimport)

//#endif

#ifndef  __CE_TRANSPORT
#define  __CE_TRANSPORT

#include "nlstrings.h"
#include "CEsdk.h"
#include "brain.h"
#include "nltypes.h"

typedef struct
{
  nlsocket sock;
  int buflen;
  char*	buf;
} TRANSPORT_QUEUE_ITEM;

////////////////////////////////////////////////////////////////////////////

// Define exported function prototypes here.
CEResult_t TRANSPORT_Cli_Initialize(nlsocket &sockid,
				    const char *serverName);
CEResult_t TRANSPORT_Serv_Initialize();
CEResult_t TRANSPORT_Serv_GetNextRequest(TRANSPORT_QUEUE_ITEM *queueItem);
CEResult_t TRANSPORT_GetRecvLength(nlsocket sockid,  size_t &recvlen);
CEResult_t TRANSPORT_Recvn(nlsocket sockid, size_t size, char* buf);
CEResult_t TRANSPORT_Sendn(nlsocket sockid, size_t size, char* buf);
void TRANSPORT_Close(nlsocket sockid);
void TRANSPORT_MemoryFree(TRANSPORT_QUEUE_ITEM &qi);
CEResult_t NL_TRANSPORT_Shutdown();


////////////////////////////// End of File /////////////////////////////////
#endif
