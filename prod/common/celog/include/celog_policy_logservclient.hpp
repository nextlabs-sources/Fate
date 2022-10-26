#ifndef __CELOG_POLICY_LOGSERVERCLIENT_HPP__
#define __CELOG_POLICY_LOGSERVERCLIENT_HPP__

#if !(defined(_WIN32) || defined(_WIN64))
  #error Not supported platform
#endif

#include <cassert>
#include <cstdio>
#include <winsock2.h>
#include <windows.h>
#include "celog.h"

#define CELOG_LOGSERVER_PORT 27015

/** CELogPolicy_LogServerClient
 *
 *  \brief Policy for logging to a file.  The file will be flushed with
 *         each write which occurs when the Log method is called.
 */
class CELogPolicy_LogServerClient : public CELogPolicy
{
  public:

    /** CELogPolicy_LogServerClient
     *
     *  \brief Constructor for log server client.
     *  \param in_group (in) Name of group to belong to.
     */
    CELogPolicy_LogServerClient( const WCHAR* in_group ) :
      s(INVALID_SOCKET)
    {
      wcsncpy_s(group,sizeof(group)/sizeof(WCHAR),in_group, _TRUNCATE);
    }/* CELogPolicy_LogServerClient */

    /** Connect
     *
     *  \brief Connect to a logserver.
     *
     *  \param in_group (in) Log group.
     *
     *  \notes true if connnection is successful, else false.
     */
    bool Connect(void) throw)();
    {
      WSADATA wsaData;
      if( WSAStartup(MAKEWORD(2,2),&wsaData) != NO_ERROR )
      {
	return false;
      }

      s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if( s == INVALID_SOCKET )
      {
	WSACleanup();
	return false;
      }

      /********************************************************************
       * Connect to log server on loopback
       *******************************************************************/
      sockaddr_in cserv; 
      cserv.sin_family = AF_INET;
      cserv.sin_addr.s_addr = inet_addr("127.0.0.1");
      cserv.sin_port = htons(CELOG_LOGSERVER_PORT);
      if( connect(s,(SOCKADDR*)&cserv,sizeof(cserv)) == SOCKET_ERROR )
      {
	closesocket(s);
	WSACleanup();
	s = INVALID_SOCKET;
	return false;
      }

      int bytes_sent;
      bytes_sent = send(s,(const char*)group,sizeof(group),0);
      return true;
    }/* Connect */

    virtual ~CELogPolicy_LogServerClient(void)
    {
      if( s != INVALID_SOCKET )
      {
	closesocket(s);
	WSACleanup();
      }
    }/* CELogPolicy_LogServerClient */

    /** Log
     *
     *  \brief Log message to a file and flush that file.
     *
     *  \param string (in) Log message
     */
    virtual int Log( const wchar_t* string )
    {
      WCHAR buf[CELOG_MAX_MESSAGE_SIZE_CHARS];
      wcsncpy_s(buf,sizeof(buf)/sizeof(WCHAR),string, _TRUNCATE);
      return send(s,(const char*)buf,sizeof(buf),0);
    }/* Log */

    virtual int Log( const char* string )
    {
      char buf[CELOG_MAX_MESSAGE_SIZE_CHARS];
      strncpy_s(buf,sizeof(buf)/sizeof(char),string, _TRUNCATE);
      return send(s,(const char*)buf,sizeof(buf),0);
    }/* Log */

  private:
    SOCKET s;  /* socket for LogServer connection */
    WCHAR group[32];

};/* CELogPolicy_LogServerClient */

#endif /* __CELOG_POLICY_LOGSERVERCLIENT_HPP__ */
