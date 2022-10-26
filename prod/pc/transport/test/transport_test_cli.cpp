/***************************************************************************
Module:  transport_win32_test.cpp
***************************************************************************/

// Include the standard Windows and C-Runtime header files here.
//#include <windows.h>


// Include the exported data structures, symbols, functions, and variables.
//#include "winsock.h"

#if defined (Linux) || defined (Darwin)
#include "stdio.h"
#include "pthread.h"
#include <sys/time.h>
#endif

#if defined (WIN32) || defined (_WIN64)
#include <windows.h>
#include <stdio.h>
#include <process.h>
#endif

#include "transport.h"
#include "nltypes.h"

int TIMES;
int packsize;

#define BUFFER_SIZE  16384
nlsocket sockid;


////////////////////////////////////////////////////////////////////////////

int getIndex(int len,char* buf)
{
  char tmpbuf[100];
  for(int i=0;i<len;i++)
    {
      if(buf[i]!=';')
	tmpbuf[i] = buf[i];
      else
	{
	  tmpbuf[i] = 0;
	  return atoi(tmpbuf);
	}
    }
  return -1;
}


//start a thread to send reqeusts
#if defined (Linux) || defined (Darwin)
void* sendworker(void* arg)
#endif
#ifdef WIN32
unsigned __stdcall sendworker( void* arg )
#endif
{
  printf("sender thread started\n");
  int index = 0;
  char *tmpbuf = (char*)malloc(BUFFER_SIZE);
  memset(tmpbuf,'1',BUFFER_SIZE);
  do{

    _snprintf_s(tmpbuf,BUFFER_SIZE, _TRUNCATE, "%d",index++);
    int len = strlen(tmpbuf);
    _snprintf_s(tmpbuf,BUFFER_SIZE, _TRUNCATE, "%s;%s",tmpbuf,"01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
    //sprintf(tmpbuf,"%s;",tmpbuf);
    //printf("1\n");
    TRANSPORT_Sendn(sockid,len+2+packsize,tmpbuf);
    //printf("client sent\n");

    //usleep(1);
    //printf("2\n");
  }while(index<TIMES);
  free(tmpbuf);
  return NULL;
}

//start a thread to receive replies
#if defined (Linux) || defined (Darwin)
void* recvworker(void* arg)
#endif
#ifdef WIN32
unsigned __stdcall recvworker( void* arg )
#endif
{
  printf("recver thread started\n");
  int len;
  char* buf = (char*)malloc(BUFFER_SIZE);
  int cnt = 0;
  do{
    //printf("3\n");
    TRANSPORT_GetRecvLength(sockid,len);
    //printf("4\n");
    TRANSPORT_Recvn(sockid,len,buf);
    //printf("client received\n");
    //printf("5\n");
    cnt++;
    //printf("got reply : %s \n",buf);
    //printf("got reply #%d.\n",getIndex(1024,buf));
  }while(cnt<TIMES);
  free(buf);
  return NULL;
}



//int WINAPI WinMain(HINSTANCE hinstExe, HINSTANCE, LPTSTR pszCmdLine, int) {
int main(int argc, char** argv){

  if(argc<3)
    {
      printf("please specify # of requests and the size of one request (in bytes [1-16000])\n");
      printf("Usage:  transport_test_cli #_of_requests  size_of_each_request [<server name>]\n");
      exit(1);
    }

  TIMES = atoi(argv[1]);
  packsize  = atoi(argv[2]);
  char *serverName=NULL;

  if(argc == 4) 
    serverName=argv[3];

  //MessageBox(NULL, "world", TEXT("hello"), MB_OK);
#if defined (Linux) || defined (Darwin)
  struct timeval tv1,tv2;
  gettimeofday(&tv1,NULL);
#endif

#ifdef WIN32
  int t1,t2;
  t1 = GetTickCount();
#endif
  
  CEResult_t ret=TRANSPORT_Cli_Initialize(sockid, serverName);
  if(ret != CE_RESULT_SUCCESS) {
    TRACE(0, _T("Failed to connect to server: errorno=%d.\n"), ret);
    exit(1);
  }

#if defined (Linux) || defined (Darwin)
  pthread_t sender,recver;

  TRACE(1,_T("time before sender thread started: %f\n"),
	NL_GetCurrentTimeInMillisec() );
  pthread_create(&sender,NULL,&sendworker,NULL);
  pthread_create(&recver,NULL,&recvworker,NULL);
  
  pthread_join(sender,NULL);
  pthread_join(recver,NULL);

  gettimeofday(&tv2,NULL);
  double elapsed = ((double)(tv2.tv_sec) - (double)(tv1.tv_sec)) + ((double)(tv2.tv_usec) - (double)(tv1.tv_usec))/1000000;
#endif

#ifdef WIN32
  HANDLE hSender, hRecver;
  unsigned senderid, recverid;
  hSender = (HANDLE)_beginthreadex( NULL, 0, &sendworker, NULL, 0, &senderid);
  hRecver = (HANDLE)_beginthreadex( NULL, 0, &recvworker, NULL, 0, &recverid);

  WaitForSingleObject(hSender, INFINITE );
  WaitForSingleObject(hRecver, INFINITE );
  t2 = GetTickCount();
  double elapsed = ((double)t2 - (double)t1)/1000;
#endif

  printf("Total time elapsed: %f s\n",elapsed);
  
  printf("finished\n");
  TRANSPORT_Close(sockid);
  
  //TRANSPORT_Sendn(sockid, 1000,"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
  //TRANSPORT_Sendn(sockid,10,"0123456789");
  //TRANSPORT_Close(sockid);
}


////////////////////////////// End of File /////////////////////////////////


