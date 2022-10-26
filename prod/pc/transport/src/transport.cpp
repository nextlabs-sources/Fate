#if defined (Linux) || defined (Darwin)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#endif

#if defined (WIN32) || defined (_WIN64)
#include <winsock2.h>
#endif

#include <queue>

//in case you want to re-define the default trace level
#define NLMODULE mytest_transport
#define NLTRACELEVEL 0

#include "nltypes.h"
#include "transport_private.h"
#include "transport.h"

using namespace std;
using namespace CETRANSPORT;

namespace {
queue<TRANSPORT_QUEUE_ITEM> dataQ;
static nlthread_cs_t  dataQ_cs;     /* dataQ critical section */
static nlthread_sem_t   semEnqueued;
void* serverthread(void* arg);
double totallocktime = 0;
struct sockaddr_in g_servaddr;

//Variables to stop transport
static nlthread_cs_t  stop_cs; //CS for notifying the stop of transport
bool bStopped;    //stop flag 
bool bInitialized = false;    //initialized flag 

static void enqueuePacket(nlsocket sockid, int len, char* buf)
{
  //Enqueue the data 
  //debug
  TRANSPORT_QUEUE_ITEM qitem;

  qitem.sock = sockid;
  qitem.buflen = len;
  qitem.buf = buf;

  double t1 = NL_GetCurrentTimeInMillisec();
  nlthread_cs_enter(&dataQ_cs);
  double lockspent = NL_GetCurrentTimeInMillisec() - t1;
  totallocktime += lockspent;
  
  dataQ.push(qitem);

  //debug
  
  //for(int j=0;j<len;j++)
  //printf("%c",buf[j]);
  
  nlthread_cs_leave(&dataQ_cs);
  nlthread_sem_post(&semEnqueued);
}



//thread that handles client connection and data transfer
void* serverworkthread(void* arg)
{
  nlthread_detach(nlthread_self());

  nlsocket clifd = *((nlsocket*)arg);
  free( arg );

  LPPER_HANDLE_DATA phd= (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA)); // PER_HANDLE_DATA has a pointer, but it is OK because phd does not go over network
  int ret,nLeft;

  if(phd == NULL) {
    //memory allocation failed; return
    nlthread_detach_end();
    return NULL;
  }

  //debug

  phd->Socket = clifd;
  phd->nExpectingBytes = 0;
  phd->packetLen = -1;
  phd->packetBuf = NULL;
  phd->PartialHeaderSize = 0; 

  char *recvbuffer, *recvbuf;
  recvbuffer = (char*)malloc(DATA_BUFSIZE);  //receiving buffer
  
  if(recvbuffer == NULL) {
    //memory allocation failed; return
    free(phd);
    nlthread_detach_end();
    return NULL;
  }

  while(1) {
    //get data from socket
    ret = recv(clifd,recvbuffer,DATA_BUFSIZE,0);
    recvbuf = recvbuffer;  //recvbuf is pointing to the front-edge of the recvbuffer

    /* The connection has been dropped due one of the following conditions:
         (1) Peer closed connection (ret == 0)
	 (2) Socket error (ret < 0)
    */
    if(ret <= 0)
    {
#if defined (WIN32) || defined (_WIN64)
      int e=WSAGetLastError();
      if(e != 10054) {
	//10054: An existing connection was forcibly closed by the remote host.
	
      }
#endif
#if defined (Linux) || defined (Darwin)
  
#endif
      //peer shutdown
#if defined (Linux) || defined (Darwin)
      close(clifd);
#endif

#if defined (WIN32) || defined (_WIN64)
      closesocket(clifd);
#endif
      phd->Socket = (unsigned int)-1;
      break;
    }
      //debug
   
      
      nLeft = ret;
      
      while(nLeft>0) {
	if(phd->nExpectingBytes == 0)  //new arrival logical packet, just read and process the header
	    {
	      //debug
	  
	      
	      //get the length info of the logical packet
	      
	      //IF nLeft is smaller than the header!!!!!! put partial header into per handle data
	      if(nLeft < ( (int)sizeof(TRANSPORT_HEADER) - phd->PartialHeaderSize ) )
		{
		  memcpy(phd->PartialHeader + phd->PartialHeaderSize, recvbuf, nLeft);
		  phd->PartialHeaderSize += nLeft;
		  nLeft = 0;
		  continue;
		}
	      
	      TRANSPORT_HEADER  thdr;
	      if(phd->PartialHeaderSize>0)
		memcpy((char*)(&thdr),phd->PartialHeader,phd->PartialHeaderSize);
	      
	      int copylen = sizeof(TRANSPORT_HEADER) - phd->PartialHeaderSize;
	      memcpy(((char*)(&thdr))+phd->PartialHeaderSize,recvbuf,copylen);
	      recvbuf += copylen;
	      
	      //should get a complete header now, convert to host byte order
	      thdr.datalen = ntohl(thdr.datalen);


	      //memcpy(&(phd->packetLen),recvbuf,sizeof(phd->packetLen));
	      //recvbuf += sizeof(phd->packetLen);
	      
	      phd->packetBuf = (char*)malloc(thdr.datalen);
	      if(phd->packetBuf == NULL) {
				
		nLeft = 0;		
		phd->nExpectingBytes=0;
		phd->PartialHeaderSize=0;
		break;
	      }
	      memset(phd->packetBuf,0,thdr.datalen);
	      phd->packetLen = thdr.datalen;
	      phd->nExpectingBytes = thdr.datalen;
	      //nLeft -= sizeof(TRANSPORT_HEADER);
	      nLeft -= copylen;
	      phd->PartialHeaderSize = 0;  //reset
	      
	      
	      //phd->packetBuf = (char*) malloc(phd->packetLen);
	      //phd->nExpectingBytes = phd->packetLen;
	      //nLeft -= sizeof(TRANSPORT_HEADER);
	      
	      //debug
	   
	    }
	  else   //we got the header already, get the content
	    {
	      int doffset = phd->packetLen - phd->nExpectingBytes;
	      
	      //if the received data is less than the expecting bytes
	      if(nLeft < phd->nExpectingBytes)
		{
		  //debug
		
		  
		  memcpy(phd->packetBuf+doffset,recvbuf, nLeft);
		  recvbuf += nLeft;
		  
		  phd->nExpectingBytes -= nLeft;
		  nLeft = 0;		
		}
	      else  //received data is larger than or equal expecting bytes => a new packet is followed
		{
		  memcpy(phd->packetBuf+doffset,recvbuf, phd->nExpectingBytes);
		  recvbuf += phd->nExpectingBytes;
		  
		  nLeft -= phd->nExpectingBytes;
		  phd->nExpectingBytes = 0;
		  
		  //finishing receiving a logical packet
		  enqueuePacket(phd->Socket,phd->packetLen,phd->packetBuf);
		  //flush = true;
		}
	    }
      }
    }
  if(recvbuffer)
    free(recvbuffer);
  if(phd)
    free(phd);
  nlthread_detach_end();
  return NULL;
}

void* serverthread(void* arg)
{
  nlthread_detach(nlthread_self());
  
#if defined (Linux) || defined (Darwin)
  socklen_t   clilen;
  struct sockaddr_in cliaddr;
#endif

  nlsocket  listenfd;
  nlsocket  connfd;
  struct sockaddr_in servaddr;
  fd_set  rset;

  listenfd = socket(AF_INET, SOCK_STREAM,0);
#if defined (WIN32) || defined (_WIN64)
  if (listenfd == INVALID_SOCKET) {
 
    WSACleanup();
    return NULL;
  }
#endif

#if defined (WIN32) || defined (_WIN64)
#else
  int optval;
  optval=1;
  if(setsockopt(listenfd, 
		SOL_SOCKET, 
		SO_REUSEADDR, 
		&optval, 
		sizeof(optval))!=0) {
    //set socket option failed
  }
#endif

  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons (PDP_PORT);
  
  int iResult=bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
#if defined (WIN32) || defined (_WIN64)
  if (iResult == SOCKET_ERROR) {
 
    closesocket(listenfd);
    WSACleanup();
    return NULL;
  }
#else
  if( iResult != 0) {
    //bind failed
    close(listenfd);
    return NULL;
  }
#endif

  //debug
 
 
  iResult=listen(listenfd, 20);
#if defined (WIN32) || defined (_WIN64)
  if (iResult == SOCKET_ERROR) {
 
    closesocket(listenfd);
    WSACleanup();
    return NULL;
  }
#endif

  FD_ZERO(&rset);
  FD_SET(listenfd,&rset);
  
  for(;;) {
    //wait for the epoll events

    //debug
    //double t1 = NL_GetCurrentTimeInMillisec();
    //TRACE(1,_T("before wait\n"));
      
      select((int)(listenfd+1), &rset, NULL,NULL,NULL); // cast to int necessary because SOCKET is 64 bit on x64, whereas int is 32 bit.  This should be safe as the number of socket descriptors shouldn't exceed 32 bit MAXINT.
            
    //debug
    //double lockspent = NL_GetCurrentTimeInMillisec() - t1;
    //TRACE(1,_T("Time spent to epoll: %f\n"), lockspent);
            
    if(FD_ISSET(listenfd,&rset)) {
#if defined (WIN32) || defined (_WIN64)
      connfd = accept(listenfd, NULL, NULL);
#else
      connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
#endif
      if(connfd<0) {
	//debug

	continue;
      }
#if defined (WIN32) || defined (_WIN64)
      if (connfd == INVALID_SOCKET) {
    
        closesocket(listenfd);
        WSACleanup();
        break;
      }
#endif

	  
      //debug
     

	  nlsocket *threadconnfd = (nlsocket *) malloc( sizeof(nlsocket) );
	  if( NULL == threadconnfd )
	  {
        //debug
       
		continue;
	  }
	  *threadconnfd = connfd;

      nlthread_t clithread;
      nlthread_detach_create(&clithread, 
			     (nlthread_detach_func)&serverworkthread,
			     threadconnfd); 
    }
  }
  nlthread_detach_end();
  return NULL;
}
}  //namespace

CEResult_t TRANSPORT_Cli_Initialize(nlsocket &sockfd, 
				    const char *serverName)
{
#if defined (WIN32) || defined (_WIN64)
  WSADATA              wsaData;

  if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
    return CE_RESULT_GENERAL_FAILED;
  }
#endif

  int ret;
  struct sockaddr_in servaddr;
  struct hostent *server = NULL;

  //Creat the socket
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0) {
 
    return CE_RESULT_CONN_FAILED;
  }

  //Get the host
  if(serverName) {
    server=gethostbyname(serverName);
    if(server == NULL) {
     
      return CE_RESULT_CONN_FAILED;
    }
  }

  //Initialize server address
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if(serverName)
    memcpy(&(servaddr.sin_addr.s_addr),server->h_addr,server->h_length);
  else
    servaddr.sin_addr.s_addr = inet_addr(PDP_SERVER_DEFAULT_ADDR);
  servaddr.sin_port = htons(PDP_PORT);

  //Connecting to the server
  ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if(ret<0) {
#if defined (WIN32) || defined (_WIN64)
    
#endif
#if defined (Linux) || defined (Darwin)
    
#endif
    return CE_RESULT_CONN_FAILED; 
  }
  return CE_RESULT_SUCCESS;
}

CEResult_t TRANSPORT_Serv_Initialize()
{
#if defined (WIN32) || defined (_WIN64)
  WSADATA              wsaData;

  if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
    return CE_RESULT_GENERAL_FAILED;
  }
#endif

  nlthread_t  tid;

  //initialize semaphore
  nlthread_cs_init(&dataQ_cs);
  //On Mac, we should call nlthread_sem_close to free the memory of 
  //g_semEqueued. However, to simplify semphore handling, sem_close
  //is not called since semEqueued will last through the whole
  //life of PDPMan.
  nlthread_sem_init(&semEnqueued,0);
  nlthread_cs_init(&stop_cs);
  bStopped=false;

  

  if(nlthread_detach_create(&tid,(nlthread_detach_func)(&serverthread),NULL))
  {
      bInitialized = true;
      return CE_RESULT_SUCCESS;
  }

  return CE_RESULT_GENERAL_FAILED;
}

CEResult_t TRANSPORT_Serv_GetNextRequest(TRANSPORT_QUEUE_ITEM *queueItem)
{
  
  //debug

  
  //If the stop flag is on, skip fetching requests and return
  nlthread_cs_enter(&stop_cs);
  if(bStopped) {
    nlthread_cs_leave(&stop_cs);

    return CE_RESULT_SUCCESS;
  }
  nlthread_cs_leave(&stop_cs);

  while(!nlthread_sem_wait(&semEnqueued));

  //If the stop flag is on, ignore the request and return
  nlthread_cs_enter(&stop_cs);
  if(bStopped) {
    nlthread_cs_leave(&stop_cs);
  
    return CE_RESULT_SUCCESS;
  }
  nlthread_cs_leave(&stop_cs);

  //Get request

  nlthread_cs_enter(&dataQ_cs);
  *queueItem = dataQ.front();
  dataQ.pop();
 
  nlthread_cs_leave(&dataQ_cs);
  


  return CE_RESULT_SUCCESS;
}


CEResult_t TRANSPORT_GetRecvLength(nlsocket sockid,  size_t &recvlen)
{
  TRANSPORT_HEADER hdr;
  int ret;

  ret = recv(sockid, (char *)(&hdr), sizeof(hdr),0);
  //printf("ret = %d\n",ret);
  if(ret<0) {
#if defined (WIN32) || defined (_WIN64)
   
#endif
#if defined (Linux) || defined (Darwin)
   
#endif
      return CE_RESULT_CONN_FAILED; 
  } else if (ret == 0) {
   
    return CE_RESULT_CONN_FAILED;
  }

  recvlen = ntohl(hdr.datalen);
  return CE_RESULT_SUCCESS;

}

CEResult_t TRANSPORT_Recvn(nlsocket sockid, size_t size, char* buf)
{
    size_t left = size;
  int recvd = 0;


  while(left>0)
    {
	recvd = recv(sockid, buf + (size - left),
		     left>INT_MAX ? INT_MAX : (int)left, // cast to int because recv() takes int.
		     0);
      if(recvd<0)
	{
#if defined (WIN32) || defined (_WIN64)
   
#endif
#if defined (Linux) || defined (Darwin)
   
#endif
	  return CE_RESULT_CONN_FAILED; 
	}
      left -= recvd;
    }
  return CE_RESULT_SUCCESS;
}

CEResult_t TRANSPORT_Sendn(nlsocket sockid, size_t size, char* buf)
{
  
  if(size > INT_MAX) {
    return CE_RESULT_INVALID_PARAMS;
  }

  int sent = 0;
  TRANSPORT_HEADER thdr;
  thdr.datalen = (nlint32)size; // cast to 32 bit.  this is explicitly 32 bit so that SDK and Policy Controller always agree on the header size.
  int datalen = thdr.datalen;
  int len = sizeof(TRANSPORT_HEADER)+datalen;
  int left = len;
  char* packetbuf = (char*)malloc(len);

  if(packetbuf == NULL) {
    //memory allocation failed; return
    return CE_RESULT_GENERAL_FAILED;
  }

  //network byte order
  thdr.datalen = htonl(thdr.datalen);
  memcpy(packetbuf,&(thdr),sizeof(TRANSPORT_HEADER));
  memcpy(packetbuf+sizeof(TRANSPORT_HEADER),buf,datalen);

  while(left>0) {
    //then the data
    sent = send(sockid, packetbuf + (len - left), left, 0);
    if(sent<0) {
#if defined (WIN32) || defined (_WIN64)
     
#endif
#if defined (Linux) || defined (Darwin)
     
#endif
      free(packetbuf);
      return CE_RESULT_CONN_FAILED; 
    }
    left -= sent;
  }
  free(packetbuf);
  
  return CE_RESULT_SUCCESS;
}
void TRANSPORT_Close(nlsocket sockid)
{
#if defined (Linux) || defined (Darwin)
  close(sockid);
#endif

#if defined (WIN32) || defined (_WIN64)
  closesocket(sockid);
  WSACleanup();
#endif
}

void TRANSPORT_MemoryFree(TRANSPORT_QUEUE_ITEM &qi)
{
  if(qi.buf!=NULL)
    free(qi.buf);
}

/**
 *  Shutdown all transport connections
 *  
 */
CEResult_t NL_TRANSPORT_Shutdown()
{
    if (bInitialized) {
        //Flag Transport to stop and wake up transport from waiting request
        nlthread_cs_enter(&stop_cs);
        bStopped=true;
        nlthread_cs_leave(&stop_cs);
        nlthread_sem_post(&semEnqueued);
    }
    
    return CE_RESULT_SUCCESS;
}
