/*==========================test_private.cpp================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * Redwood City CA, Ownership remains with Blue Jungle Inc,                 * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 2/28/2007                                                       *
 * Note   : Test SDK CEP_StopPDP, etc APIs.                                 *
 *==========================================================================*/
#include <time.h>
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"
#include "transport.h"
#include "nlthread.h"
#include "marshal.h"
#include "CEPrivate.h"

int main(int argc, char** argv)
{
  CEResult_t ret      = CE_RESULT_GENERAL_FAILED;
  CEHandle   handle   = NULL;
  CEString   appName  = NULL;
  CEString   password = NULL;
  
  if(argc<2) {
    TRACE(0, _T("Usage: test_private <option>\n"));
    return 0;
  }

  if(strcmp(argv[1],"stopAgent")) 
    return 0;

  appName = CEM_AllocateString (_T("test_private"));
  if (!appName) {
    return 0;
  }
  
  //"handle" is allocated by PDP
  CEApplication app;
  CEUser user;
  app.appName=appName;
  app.appPath=NULL;
  user.userName=NULL;
  user.userID=NULL;

  ret = CECONN_Initialize (app, user, NULL, &handle, CE_INFINITE);
  if (ret != CE_RESULT_SUCCESS) {
    TRACE(0, _T("CECONN_Initialize failed: errorno=%d\n"),ret);
    if (appName)  CEM_FreeString (appName);
    return 0;
  }

  password = CEM_AllocateString (_T("password"));
  if (!password) {
    if (appName)  CEM_FreeString (appName);
    ret = CE_RESULT_GENERAL_FAILED;
    CECONN_Close(handle, CE_INFINITE); 
    return 0;
  }

  //Stop PDP and "handle" will be freed by PDP also
  TRACE(0, _T("Call StopPDP.\n"));
  ret = CEP_StopPDP (handle, password, 50000);
  TRACE(0, _T("StopPDP return: %d\n"), ret);

  CECONN_Close(handle, CE_INFINITE);

  if (appName)  CEM_FreeString (appName);
  if (password) CEM_FreeString (password);
  return 0;
}
