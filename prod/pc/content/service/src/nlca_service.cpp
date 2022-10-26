#include <winsock2.h>
#include <windows.h>
#include <cassert>
#include <process.h>
#include <cassert>
#include <winbase.h>
#include <vector>
#include <string>
#include "nl_content_cracker_strings.hpp"
#include "nl_content_cracker_ifilter.hpp"
#include "nlca_service_efilter.hpp"
#include "celog.h"
#include "celog_policy_file.hpp"
#include "celog_policy_windbg.hpp"
#include "nl_content_analysis.hpp"
#include "nlca_rpc_server.hpp"
#include "nlca_framework.h"
#include "nlca_framework_network.hpp"
#include "nlconfig.hpp"

using namespace NLCA;

/***********************************************************************
 * Local global variables and functions scoped in this file
 **********************************************************************/
namespace {
#define NUM_NLCA_THREAD 10

CELog calog;
NLCA::ThreadPool *g_threadpool; //thread pool
SOCKET serviceListenSocket; //service listenning socket 
std::vector<NLCA::Client *> clients; /* set of clients */

//Shutdown service listenning socket  
bool Shutdown(void)
{
  closesocket(serviceListenSocket);
  WSACleanup();
  return true;
}/* Shutdown */

//Setup logging 
bool SetupLogging()
{
  LONG status;
  HKEY hKey = NULL; 

  status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
	  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,
			  KEY_QUERY_VALUE,
			  &hKey);
  if( status != ERROR_SUCCESS ) {
    return false;
  }

  char  pcRoot[MAX_PATH] = {0};                 /* PolicyControllerDir */
  DWORD pc_root_size = sizeof(pcRoot);

  status = RegQueryValueExA(hKey,                   /* handle to reg */      
			    "PolicyControllerDir", /* key to read */
			     NULL,               /* reserved */
			     NULL,               /* type */
			     (LPBYTE)pcRoot,     /* PolicyControllerDir */
			     &pc_root_size       /* size (in/out) */
			    );
  RegCloseKey(hKey);
  if( status != ERROR_SUCCESS ) {
    return false;
  }

  char logfile[MAX_PATH] = {0};
  _snprintf_s(logfile,_countof(logfile), _TRUNCATE,"%s//agentLog//caServiceCalog.txt",pcRoot);

  calog.SetPolicy( new CELogPolicy_File(logfile) ); 

  calog.Enable();
  calog.SetLevel(CELOG_WARNING);

  if (NLConfig::IsDebugMode()) {
      calog.SetLevel(CELOG_DEBUG);
      calog.SetPolicy( new CELogPolicy_WinDbg() );
  }

  //SetNLCAThreadPoolLog(&calog);
  //SetContentAnalysisRPCLog(&calog);
  return true;
}
 
bool ReadRequest(SOCKET &s, ContentAnalysisRPCTask **request)
{
  /***********************************************************************
   * Read the payload size
   **********************************************************************/
  int payload_size;
  int bytes_read = nl_recv_sync(s,(char*)&payload_size,sizeof(payload_size),0,1000); // 1000ms
  /* Handle: (1) error, (2) graceful disconnect, 
     (3) invalid payload size */
  if( bytes_read <= 0 ) {
    calog.Log(CELOG_INFO,
	      _T("nlca_service: Disconnected [socket=%d, bytes_read=%d]\n"), 
	      s, bytes_read);
    return false;
  }
  if( bytes_read < sizeof(payload_size) ) {
    calog.Log(CELOG_ERR,
	      _T("nlca_service: Failed to read payload size [socket=%d, bytes_read=%d]\n"), 
	      s, bytes_read);
    return false;
  }
  payload_size = ntohl(payload_size);
  //fprintf(stdout, "payload_size = %d\n", payload_size);
  assert( payload_size > 0 && payload_size < (32 * 1024) ); /* (0,32KB) */
  if( payload_size <= 0 || payload_size >= (32 * 1024) )
  {
    calog.Log(CELOG_ERR,
	      _T("ReadRequest[socket=%d]: payload(=%d) size out of range\n"),
	      s, payload_size);
    return false;
  }

  /***********************************************************************
   * Read payload
   **********************************************************************/
  char* buf = new (std::nothrow) char [payload_size+1];

  if( buf != NULL ) {
    bytes_read = nl_recv_sync(s,buf,payload_size,0,1000); // 1000ms timeout

    // bytes read must match payload size
    if(bytes_read != payload_size) {
      calog.Log(CELOG_ERR,
		_T("Bytes(%d) received is not what expected payload size (%d) from socket(%d). Still try to continue to analyze it.\n"),
		bytes_read, payload_size, s);
      return false;
    }

    /*********************************************************************
     * Deserialize payload
     ********************************************************************/
    bool bDeserial=NLCA_RPCServer_Deserialization(buf, payload_size,request);
    delete [] buf;
    if(!bDeserial) {
      calog.Log(CELOG_ERR,
		_T("Failed to deserialize payload(size=%d) from socket(%d)\n"),
		payload_size, s);
      return false; //failed to deserialize, discard this request
    }
  } else {
    calog.Log(CELOG_ERR,
       _T("Failed to allocate %d long buffer. Discard request (socket=%d)\n"),
	      payload_size+1, s); 
    return false;
  }
  return true;  
} 

/******************************************************************************
 * Wait for network I/O events such as connect and read, or cancellaction 
 * signal from PluginUnload.
 *****************************************************************************/
bool nlca_service_worker(void)
{
  std::vector<Client*>::iterator i;   /* iterator for set of clients */
  fd_set rfds, efds;
  bool bStopService=false;

  calog.Log(CELOG_INFO,_T("Enter nlca_service_worker\n"));
  bool require_reset = false; // require reset on idle time
  for( ;; )
  {    
    /***********************************************************************
     * Set sockets to check state with select().  If no sockets are in a
     * ready state return to the above wait condition.  Read and error
     * conditions are checked for.
     **********************************************************************/
    FD_ZERO(&rfds);
    FD_ZERO(&efds);
    FD_SET(serviceListenSocket,&rfds);  /* listen socket */
    for( i = clients.begin() ; i != clients.end() ; ++i ) {
      SOCKET client_socket = (*i)->GetSocket();
      FD_SET(client_socket,&rfds);
      FD_SET(client_socket,&efds);
    }

    /****************************************************************************
     * Wait for a socket to become accept/error/read ready.
     *
     * After a set of file searches the service will restart.  This is done
     * to avoid the Adobe PDF IFilter from running away in its worker thread.
     *
     *
     * 30 seconds after the last search request without any clients the service
     * will restart.
     ***************************************************************************/
    timeval tv = { 30 , 0 }; // 30 second timeout
    int selectRet=select(0,&rfds,NULL,&efds,&tv);
    if( selectRet == 0 )
    {
      /* Is a reset required and nothing to do (idle) */
      if( require_reset == true && clients.size() == 0 )
      {
	break;  // reset process, teardown threadpool, etc.
      }
      continue; // nothing to do wait for another connection
    }
    if(selectRet == 0 || selectRet==SOCKET_ERROR) {
      calog.Log(CELOG_ERR,
		_T("nlca_service: select() failed (WSALastError %d)\n)"), 
		WSAGetLastError());
      //continue; /* no sockets in ready state */
      break; //abort
    } else {
      calog.Log(CELOG_DEBUG,_T("select return %d\n"), selectRet);      
    }

    require_reset = true; // a request will be serviced.

    /***********************************************************************
     * There is at least one socket in a ready state.
     *   (1) Check listen socket for connection.
     *   (2) Iterate over connections and handle i/o.
     **********************************************************************/
    if( FD_ISSET(serviceListenSocket,&rfds) ) {/* accept waiting? */
      /***********************************************************************
       * Accept connection.  Retreive socket for incomming connection.
       ***********************************************************************/
      calog.Log(CELOG_INFO,"nlca_service: accepting connection\n");
      SOCKET new_s = accept(serviceListenSocket,
			    NULL,NULL); /* accept connection */
      if( new_s == INVALID_SOCKET ) {
	calog.Log(CELOG_ERR,
	      _T("nlca_service: accept() failed (WSALastError %d)\n)"), 
		 WSAGetLastError());
	continue;
      }

      /************************************************************************
       * Create nlca client associated with the connected socket
       ***********************************************************************/
      Client* lc =NLCA_InitializeAClient();
      if(lc) {
	lc->SetSocket(new_s);	
	clients.push_back(lc);       /* add client */
      }
      calog.Log(CELOG_DEBUG, _T("nlca_service: Connected [%d]\n"), new_s);
      FD_CLR(serviceListenSocket,&rfds);    /* clear socket from set */
    }

    /**************************************************************************
     * Iterate over connected clients to retrieve messages waiting to be read.
     *************************************************************************/
    for( i = clients.begin() ; i != clients.end() ; ) {
      SOCKET curr_sock = (*i)->GetSocket();
      if( FD_ISSET(curr_sock,&efds) ) { /* error read? */
	calog.Log(CELOG_ERR, _T("nlca_service: Disconnected [%d] (ERROR)\n"), 
	      curr_sock);
	NLCA_ThreadPool_CancelTask(g_threadpool, (*i));
	NLCA_FreeAClient(*i);	  
	closesocket(curr_sock);
	i=clients.erase(i);  /*remove client */
	continue; //continue to handle next event
      }
      if( FD_ISSET(curr_sock,&rfds) ) {  /* read ready? */
	ContentAnalysisRPCTask *caRPCTask;
	if(ReadRequest(curr_sock, &caRPCTask)==false) {
	  //failed to read request. Discard this client
	  NLCA_ThreadPool_CancelTask(g_threadpool, (*i));
	  NLCA_FreeAClient(*i);
	  closesocket(curr_sock);
	  i=clients.erase(i); /* remove client */
	  continue; //continue to handle next event
	} else {
	  (*i)->SetTask(dynamic_cast<TASK *>(caRPCTask));
	  calog.Log(CELOG_DEBUG,_T("Read over socket\n"));	  
	}

	if(caRPCTask->GetRPC()->GetRPCType()==ContentAnalysisRPC::StopService) {
	  calog.Log(CELOG_INFO, _T("nlca_service: stop\n"));
	  bStopService=true;
	  closesocket(curr_sock);
	  NLCA_FreeAClient(*i);
	  clients.erase(i); /* remove client */
	  //ToDo: 1. send response? 
	  break;
	} else {
	  //Assign to threadpool: copy client to threadpool client
	  NLCA_ThreadPool_AddTask(g_threadpool,(*i));
	}
      }/* fd set: ready*/
      ++i;
    }/* for( i = clients.begin()  */

    if(bStopService)
      break;
  }/* for( ;; ) */

  /* Shutdown threadpool */
  NLCA_ThreadPool_Shutdown(g_threadpool);

  /* disconnect all clients */
  for( i = clients.begin() ; i != clients.end() ; ++i )
  {
    closesocket((*i)->GetSocket());
  }
  calog.Log(CELOG_INFO,_T("Leave nlca_service_worker\n"));
  return 0;
}/* nlca_service_worker */

/****************************************************************************
 * Setup network connection
 ***************************************************************************/
bool Initialize(void)
{
  if(SetupLogging()== false) {
    calog.Log(CELOG_ERR,L"Failed to setup logging\n");
    return false;
  }

  int result = 0; /* fail by default */

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0) {
    return false;
  }

  /* Initialize threadpool */
  g_threadpool=NLCA_ThreadPool_Initialize(NUM_NLCA_THREAD);
  if(g_threadpool==NULL)
    goto PluginEntry_complete;

  serviceListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if( serviceListenSocket == INVALID_SOCKET ) {
    calog.Log(CELOG_ERR,_T("nlca_service: socket() failed: %d\n"), 
	  WSAGetLastError());
    goto PluginEntry_complete;
  }

  /********************************************************************
   * Bind and listen on port (localhost only)
   *******************************************************************/
  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_addr.s_addr = inet_addr("127.0.0.1");
  service.sin_port = htons(NLCAService::SERVICE_PORT);
  if( bind(serviceListenSocket,(SOCKADDR*)&service,
	   sizeof(service)) == SOCKET_ERROR ) {
    calog.Log(CELOG_ERR,_T("nlca_service: bind() failed: %d\n"), 
	  WSAGetLastError());
    goto PluginEntry_complete;
  }
  if( listen(serviceListenSocket,32) != 0 ) {
    calog.Log(CELOG_ERR,_T("nlca_service: listen() failed: %d\n"), 
	  WSAGetLastError());
    goto PluginEntry_complete;
  }

  result = 1;

 PluginEntry_complete:

  if( result == 0 )  /* cleanup failed initialization */
  {
    Shutdown();
  }
  return result;
}/* Initialize */
}

/** is_already_running
 *
 *  \brief Determine if the given process is already running.
 *  \return true if the process is already running, otherwise false.
 */
static bool is_already_running(void)
{
  /* After this process terminates the ownership will be 'abandoned' so that it may be
   * acquired again by the next instance.
   */
  HANDLE h = CreateMutexA(NULL,TRUE,"NLCA_ServiceMutex");
  if( h == NULL )
  {
    return false;
  }

  /* Two cases:
   *
   *   (1) Mutex already exist.  Ownership request is ignored, so test for signaled
   *       state to take ownership.
   *   (2) Mutex does not exist.  Ownership is granted as a side-effect of mutex
   *       creation.
   */
  if( GetLastError() == ERROR_ALREADY_EXISTS )
  {
    if( WaitForSingleObject(h,0) == WAIT_TIMEOUT )
    {
      return true;
    }
  }

  /* at this point this process owns name mutext "NLCA_ServiceMutex" */

  return false;
}/* is_already_running */

int main(void)
{
  if( is_already_running() == true )
  {
    fprintf(stdout, "nlca_service already running\n");
    return 0;
  }

  SetUnhandledExceptionFilter(efilter);
  calog.Log(CELOG_INFO,_T("nlca_service: starting\n"));
  if( Initialize() != true ){
    calog.Log(CELOG_ERR,_T("nlca_service: failed\n"));
    return 1;
  }
  nlca_service_worker();  // block until stop command
  Shutdown();
  calog.Log(CELOG_INFO,_T("nlca_service: done\n"));
  return 0;
}/* main */
