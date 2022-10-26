/*==========================test_conn.cpp===================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 1/22/2007                                                       *
 * Note   : Test SDK CECONN_XXX APIs.                                       *
 *==========================================================================*/
/*#include <time.h>
#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#include <ws2tcpip.h>
#endif
#if defined (Linux) || defined (Darwin)
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif*/
#include <errno.h>
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"

int main(int argc, char **argv)
{
  CEString appName=CEM_AllocateString(_T("ceconntest"));
  CEString binaryPath=CEM_AllocateString(_T("./ceconntest.exe"));
  CEString userName=CEM_AllocateString(_T("heidi"));
  CEString type = CEM_AllocateString(_T("test_conn"));
  CEHandle handle;  
  CEUser user;
  CEApplication app;
  CEResult_t result;
  CEint32 timedout=1000;

  user.userName=userName;
  user.userID=NULL;
  app.appPath=NULL;
  app.appName=NULL;

  //Try 50 times connection and disconnection
  for(int i=0; i<50; i++) {
    result=CECONN_Initialize(app, user, NULL, &handle, timedout);
    if(result == CE_RESULT_SUCCESS)
      result=CECONN_Close(handle, 1000);
    else {
      TRACE(0, _T("CECONN_Initialize failed: %d\n"), result);
      break;
    }
    timedout-=19;
  }

  //Make a deliberate invalid host connection
  CEString bogusHost=CEM_AllocateString(_T("bogus"));
  TRACE(0, _T("Make a deliberate invalid host connection"));
  result=CECONN_Initialize(app, user, bogusHost, &handle, 0);
  if(result == CE_RESULT_SUCCESS)
    result=CECONN_Close(handle, 1000);
  else
    TRACE(0, _T("CECONN_Close failed: %d\n"), result);
  CEM_FreeString(bogusHost);

  //Try 5 times connection and disconnection
  TRACE(0, _T("Try 5 times connection and disconnection"));
  timedout=1000;
  for(int i=0; i<5; i++) {
    result=CECONN_Initialize(app, user, NULL, &handle, timedout);
    if(result == CE_RESULT_SUCCESS)
      result=CECONN_Close(handle, 1000);
    else {
      TRACE(0, _T("CECONN_Initialize failed: %d\n"), result);
      break;
    }
    timedout-=19;
  }

  //Make a deliberate timeout connection call
  TRACE(0, _T("Make a deliberate 0 timeout connection call"));
  result=CECONN_Initialize(app, user, NULL, &handle, 0);
  if(result == CE_RESULT_SUCCESS)
    result=CECONN_Close(handle, 1000);
  else
    TRACE(0, _T("CECONN_Initialize failed: %d\n"), result);
  
  //After timeout, connect again
  result=CECONN_Initialize(app, user, NULL, &handle, 1000);
  if(result == CE_RESULT_SUCCESS) {
    result=CECONN_Close(handle, 1000);
    TRACE(0, _T("CECONN_Initialize succeed as expected\n"));
  } else
    TRACE(0, _T("CECONN_Initialize failed: %d\n"), result);
  CEM_FreeString(appName);
  CEM_FreeString(binaryPath);
  CEM_FreeString(userName);
  CEM_FreeString(type);
}
