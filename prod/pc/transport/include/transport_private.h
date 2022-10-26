/***************************************************************************
Module:  transport_private.h
***************************************************************************/
#include "brain.h"
#include "nlthread.h"

namespace CETRANSPORT{

#define PDP_PORT 9233
#define PDP_SERVER_DEFAULT_ADDR   "127.0.0.1" //"10.17.11.41", local machine
#define DATA_BUFSIZE 4096

typedef struct
{
    nlint32 datalen; // this is explicitly 32 bit so that SDK and Policy Controller always agree on the header size
} TRANSPORT_HEADER, *LPTRANSPORT_HEADER;


#ifdef  WIN32

typedef struct
{
  OVERLAPPED Overlapped;
  WSABUF DataBuf;
  CHAR Buffer[DATA_BUFSIZE];
} PER_IO_OPERATION_DATA, * LPPER_IO_OPERATION_DATA;

#endif

typedef struct 
{
  nlsocket Socket;
  int    nExpectingBytes;		//number of bytes expected for the current logical packet
                                        //    if it is 0, it means it just complete receiving the last logical packet
  int    packetLen;           // length of current packet
  char*  packetBuf;           // buffer used to hold the whole logical packet
  char   PartialHeader[sizeof(TRANSPORT_HEADER)];
  int    PartialHeaderSize;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;
}
