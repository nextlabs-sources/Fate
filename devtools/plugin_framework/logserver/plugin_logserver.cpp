/*****************************************************************************
 *
 * Log Server Plug-in for Policy Controller
 *
 * List on loopback:27915 for log clients.
 *
 * The log server accepts connection and associates clients with a log group.
 * The log group corresponds a set of clients and a single file.  For example,
 * a set of clients (group) can write to a single log file.  Outlook Enforcer
 * may require a single log file from two separate processes.  Both of these
 * process would connect to a group (i.e. "oe") and both clients will write
 * logs to a file "oe_log.txt" which is present within the Policy Controller.
 *
 * Types:
 *
 *   LogClient        Log client object.  Container for socket and group
 *                    membership.
 *
 *   LogGroup         Group object.  Represents a log group which one or more
 *                    clients may belong to.
 *
 * Association:
 *
 *   LogClient ----------------> LogGroup
 *
 *   LogClient::Log() ---------> LogGoup::Log()
 *
 *   Example:  Clients {C1,C2} belong to group G1 and client C3 belongs to
 *             group G2.
 *
 *             Client       Group
 *             -------------------------
 *              C1           G1
 *              C2           G1
 *              C3           G2
 *
 * NOTES:
 *
 *   When a connection is accepted the client must supply a 64-byte wide char
 *   based group name.
 *
 ****************************************************************************/

#include <winsock2.h>
#include <windows.h>
#include <cstdio>
#include <cassert>
#include <vector>
#include <map>
#include <string>

/** LogServerPluginContext
 *
 *  Context information for LogServer plug-in.
 */
typedef struct
{
  HANDLE cancel_event;  /* Event cancel support of device arrival */
  HANDLE th;            /* Thread handle */
  SOCKET s;             /* Server socket */
  HANDLE io_event;      /* Auto reset synchronization event for IO */
} LogServerPluginContext;

/** LogGroup
 *
 *  \brief Log client group.  Match group to group related data such as file.
 */
class LogGroup
{
  public:
    LogGroup( std::wstring in_name ) :
      fp(NULL), num_clients(0), name(in_name)
    {
      /* empty */
    }

    ~LogGroup(void)
    {
      if( fp != NULL )
      {
	fclose(fp);
      }
    }/* ~LogGroup */

    const std::wstring& Name(void)
    {
      return name;
    }/* Name */

    /** Log
     *
     *  \brief Log message to group file.
     *  \return Number of characters written to the log.
     */
    int Log( const wchar_t* string )
    {
      if( fp == NULL )
      {
	fp = _wfopen(name.c_str(),L"a+");
      }
      if( fp == NULL )
      {
	return -1;
      }
      return fwprintf(fp,string);
    }/* Log */

    void Acquire(void)  /* indicate use of group */
    {
      num_clients++;
    }/* Acquire */

    void Release(void)  /* indicate done with group */
    {
      num_clients--;
    }/* Release */

  private:
    std::wstring name;     /* group name */
    FILE* fp;              /* file for log output */
    int num_clients;       /* number of clients currently connect to the group */

};/* LogGroup */

/** LogClient
 *
 *  \brief Log client record.  Each client belongs a single log group.
 */
class LogClient
{
  public:
    LogClient( LogGroup& in_group ) :
      group(in_group)
    {
      group.Acquire();
    }/* LogClient */

    /** ~LogClient
     *
     *  \brief Release from group and close socket.
     */
    ~LogClient(void)
    {
      group.Release();
      closesocket(s);
    }/* ~LogClient */

    /** GroupName
     *
     *  \brief Name of the group this client belongs to.
     */
    const std::wstring& GroupName(void)
    {
      return group.Name();
    }/* GroupName */

    /** Log
     *
     *  \brief Log a message for a connected client.  Log message is sent to the
     *         group.
     *  \return Number of characters written to the log.
     */
    int Log( const wchar_t* string )
    {
      return group.Log(string);  /* log to group */
    }/* Log */

    SOCKET s;                  /* client socket */
  private:
    LogGroup& group;     /* group belong to */
};/* LogClient */

/********************************************************************************
 * Wait for network I/O events such as connect and read, or cancellaction signal
 * from PluginUnload.
 *******************************************************************************/
DWORD WINAPI plugin_logserver_worker( LPVOID in_context )
{
  assert( in_context != NULL );
  LogServerPluginContext* context = (LogServerPluginContext*)in_context;

  std::vector<LogClient*> clients;                  /* set of clients */
  std::vector<LogClient*>::iterator i;              /* iterator for set of clients */
  std::map<std::wstring,LogGroup*> groups;    /* log groups */

  for( ;; )
  {
    /*************************************************************************
     * Wait for an network I/O event or to be cancelled.
     *************************************************************************/
    HANDLE wait_handles[] = { context->cancel_event , context->io_event };
    DWORD wait_result = WaitForMultipleObjects(2,wait_handles,FALSE,INFINITE);

    if( wait_result == WAIT_OBJECT_0 )           /* cancel event signaled */
    {
      fprintf(stderr, "plugin_logserver: cancel signal\n");
      break;
    }
    if( wait_result != (WAIT_OBJECT_0 + 1) )     /* I/O event signaled error */
    {
      fprintf(stderr, "plugin_logserver: fatal error with I/O signal\n");
      break;
    }

    /***********************************************************************
     * Set sockets to check state with select().  If no sockets are in a
     * ready state return to the above wait condition.  Read and error
     * conditions are checked for.
     **********************************************************************/
    fd_set rfds;
    fd_set efds;
    FD_ZERO(&rfds);
    FD_ZERO(&efds);
    FD_SET(context->s,&rfds);  /* listen socket */
    for( i = clients.begin() ; i != clients.end() ; ++i )
    {
      FD_SET((*i)->s,&rfds);
      FD_SET((*i)->s,&efds);
    }

    timeval tv = { 0 , 0 }; /* no wait - test only */
    if( select(0,&rfds,NULL,&efds,&tv) <= 0 )
    {
      continue; /* no sockets in ready state */
    }

    /***********************************************************************
     * There is at least one socket in a ready state.
     *   (1) Check listen socket for connection.
     *   (2) Iterate over connections and handle i/o.
     **********************************************************************/
    if( FD_ISSET(context->s,&rfds) )                   /* accept waiting? */
    {
      /**********************************************************************************
       * Accept connection.  Retreive socket for incomming connection.
       *********************************************************************************/
      SOCKET new_s;
      fprintf(stderr, "plugin_logserver: accepting connection\n");
      new_s = accept(context->s,NULL,NULL);             /* accept connection */

      if( new_s == INVALID_SOCKET )
      {
	fprintf(stderr, "plugin_logserver: accept() failed (WSALastError %d)\n", WSAGetLastError());
	continue;
      }

      WCHAR group_name[32];
      int bytes_read = recv(new_s,(char*)group_name,sizeof(group_name),0);
      if( bytes_read != sizeof(group_name) )
      {
	/* Client did not specify 32 character group name */
	fprintf(stderr, "plugin_logserver: client failed to send 64-byte group\n");
	closesocket(new_s);
	continue;
      }
      WSAEventSelect(new_s,context->io_event,FD_READ|FD_CLOSE);  /* associate with io_event */

      /**********************************************************************************
       * Retreive or create log group.
       *********************************************************************************/
      if( groups.find(group_name) == groups.end() )
      {
	groups[group_name] = new LogGroup(group_name);  /* create new group */
      }

      /**********************************************************************************
       * Create log client associated with log group.
       *********************************************************************************/
      LogClient* lc = new LogClient(*groups[group_name]);
      lc->s = new_s;
      lc->Log(L"Connected\n");
      clients.push_back(lc);       /* add client */

      fwprintf(stderr, L"plugin_logserver: [%s] Connected client %d\n", group_name, new_s);
      FD_CLR(context->s,&rfds);    /* clear socket from set */
    }

    /******************************************************************************
     * Iterate over connected clients to retrieve log messages wating to be read.
     *****************************************************************************/
    for( i = clients.begin() ; i != clients.end() ; ++i )
    {
      if( FD_ISSET((*i)->s,&efds) )  /* error read? */
      {
	fwprintf(stderr, L"plugin_logserver: [%s] Disconnected client %d (ERROR)\n", (*i)->GroupName().c_str(), (*i)->s);
	delete (*i);       /* disconnect client */
	clients.erase(i);  /* remove client */
	break;
      }
      if( FD_ISSET((*i)->s,&rfds) )  /* read ready? */
      {
	WCHAR buf[2048];      /* celog.h : CELOG_MAX_MESSAGE_SIZE_CHARS */

	int bytes_read = recv((*i)->s,(char*)buf,sizeof(buf),0);
	if( bytes_read <= 0 )   /* if failed recv disconnect client */
	{
	  fwprintf(stderr, L"plugin_logserver: [%s] Disconnected client %d\n", (*i)->GroupName().c_str(), (*i)->s);
	  delete (*i);       /* disconnect client */
	  clients.erase(i);  /* remove client */
	  break;
	}
	(*i)->Log(buf);         /* log message to client */
      }/* fd set */
    }/* for */
  }/* for */

  /* disconnect all clients */
  for( i = clients.begin() ; i != clients.end() ; ++i )
  {
    delete (*i);
  }
  return 0;
}/* plugin_logserver_worker */

/****************************************************************************
 * Plug-in Entry Points
 ***************************************************************************/
extern "C" __declspec(dllexport) int PluginEntry( void** in_context )
{
  assert( in_context != NULL );
  if( in_context == NULL )
  {
    return 0;
  }
  LogServerPluginContext* context = new LogServerPluginContext;
  *in_context = (void*)context;
  memset(context,0x00,sizeof(LogServerPluginContext));

  int result = 0; /* fail by default */

  context->cancel_event = CreateEventA(NULL,TRUE,FALSE,NULL);
  context->io_event = CreateEventA(NULL,FALSE,FALSE,NULL);
  if( context->cancel_event == NULL || context->io_event == NULL )
  {
    fprintf(stderr, "plugin_logserver: CreateEvent failed\n");
    goto PluginEntry_complete;
  }

  WSADATA wsaData;
  WSAStartup(MAKEWORD(2,2),&wsaData);

  context->s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if( context->s == INVALID_SOCKET )
  {
    fprintf(stderr, "plugin_logserver: socket() failed\n");
    goto PluginEntry_complete;
  }

  /********************************************************************
   * Bind and listen on port
   *******************************************************************/
  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_addr.s_addr = inet_addr("127.0.0.1");
  service.sin_port = htons(27015);
  if( bind(context->s,(SOCKADDR*)&service,sizeof(service)) == SOCKET_ERROR )
  {
    fprintf(stderr, "plugin_logserver: bind() failed\n");
    goto PluginEntry_complete;
  }
  if( listen(context->s,32) != 0 )
  {
    fprintf(stderr, "plugin_logserver: listen() failed\n");
    goto PluginEntry_complete;
  }

  /* associate server socket with event for accept, read, and close */
  WSAEventSelect(context->s,context->io_event,FD_ACCEPT|FD_READ|FD_CLOSE);

  context->th = CreateThread(NULL,0,plugin_logserver_worker,*in_context,0,NULL);
  if( context->th == NULL )
  {
    fprintf(stderr, "plugin_logserver: CreateThread() failed\n");
    goto PluginEntry_complete;
  }
  result = 1;

 PluginEntry_complete:

  if( result == 0 )  /* cleanup failed initialization */
  {
    if( context->s != INVALID_SOCKET )
      closesocket(context->s);
    if( context->io_event != NULL )
      CloseHandle(context->io_event);
    if( context->cancel_event != NULL )
      CloseHandle(context->cancel_event);
    if( context->th != NULL )
      CloseHandle(context->th);
    delete context;
  }

  return result;
}/* PluginEntry */

extern "C" __declspec(dllexport) int PluginUnload( void* in_context )
{
  assert( in_context != NULL );

  if( in_context == NULL )
  {
    return 0;
  }

  LogServerPluginContext* context = (LogServerPluginContext*)in_context;

  /* Signal thread to complete */
  SetEvent(context->cancel_event);
  WaitForSingleObject(context->th,INFINITE);
  CloseHandle(context->th);
  CloseHandle(context->io_event);
  closesocket(context->s);
  WSACleanup();

  delete in_context;

  return 1;
}/* PluginUnload */
