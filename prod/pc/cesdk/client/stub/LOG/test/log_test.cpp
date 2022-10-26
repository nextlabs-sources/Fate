/*===========================log_test.cpp===================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by Nextlabs,     *
 * San Mateo CA, Ownership remains with NextLabs Inc,                       * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 3/25/2008                                                       *
 * Note   : Test SDK CELOG_XXX APIs.                                        *
 *==========================================================================*/
#include <errno.h>
#include "brain.h"
#include "cetype.h"
#include "CEsdk.h"

int main(int argc, char **argv)
{
  CEString appName=CEM_AllocateString(_T("celogtest"));
  CEString binaryPath=CEM_AllocateString(_T("./celogtest.exe"));
  CEString userName=CEM_AllocateString(_T("heidi"));
  CEString type = CEM_AllocateString(_T("test_log"));
  CEString cookie = CEM_AllocateString(_T("cookie123"));
  CEString logId = CEM_AllocateString(_T("1111123"));
  CEString aName = CEM_AllocateString(_T("assistant name"));
  CEHandle handle;  
  CEUser user;
  CEApplication app;
  CEResult_t result;
  CEint32 timedout=1000;

  user.userName=userName;
  user.userID=NULL;
  app.appPath=NULL;
  app.appName=NULL;

  CEString okey=CEM_AllocateString(_T("oAttr key"));
  CEString oval=CEM_AllocateString(_T("oAttr value"));
  CEAttribute oattr;
  CEAttributes oattrs;
  oattr.key = okey;
  oattr.value = oval;
  oattrs.count=1;
  oattrs.attrs=&oattr;

  result=CECONN_Initialize(app, user, NULL, &handle, timedout);
  if(result == CE_RESULT_SUCCESS) { 
    //result=CELOGGING_LogDecision(handle,cookie, CEAllow, &oattrs); 
    //result=CELOGGING_LogDecision(handle,cookie, CEAllow, NULL); 
    result=CELOGGING_LogObligationData(handle, logId, aName, NULL);
    result=CELOGGING_LogObligationData(handle, logId, aName, &oattrs);
    if(result != CE_RESULT_SUCCESS) 
      TRACE(0, _T("CELOG_LogDecision failed: %d\n"), result);
    result=CECONN_Close(handle, 1000);
  } else {
    TRACE(0, _T("CECONN_Initialize failed: %d\n"), result);
  }

  CEM_FreeString(appName);
  CEM_FreeString(binaryPath);
  CEM_FreeString(userName);
  CEM_FreeString(type);
  CEM_FreeString(cookie);
}
