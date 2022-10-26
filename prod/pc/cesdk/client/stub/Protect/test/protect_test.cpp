/*==========================protect_test.cpp================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2006 by Blue Nextlabs,*
 * San Mateo, CA, Ownership remains with Blue Jungle Inc,                   * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author : Heidi Zhou                                                      *
 * Date   : 8/22/2007                                                       *
 * Note   : Test SDK CEPROTECT_XXX APIs.                                    *
 *==========================================================================*/
#include <time.h>
#if defined (WIN32) || defined (_WIN64)
#include <Winsock2.h>
#pragma warning(push)
#pragma warning(disable : 6386)
#include <ws2tcpip.h>
#pragma warning(pop)
#endif
#if defined (Linux) || defined (Darwin)
#include <iostream>
#include <fstream>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <errno.h>
#include "brain.h"
#include "CEsdk.h"
#include "CESDK_private.h"

int main(int argc, char **argv)
{
  CEString appName=CEM_AllocateString(_T("protect_test"));
  CEString binaryPath=CEM_AllocateString(_T("./ceprotect_test.exe"));
  CEString userName=CEM_AllocateString(_T("heidi"));
  CEHandle handle;
  CEUser user;
  CEApplication app;
  int c;

  user.userName=userName;
  user.userID=NULL;
  app.appPath=binaryPath;
  app.appName=appName;
  CEString type = CEM_AllocateString(_T("protect_test"));
  CEResult_t res=CECONN_Initialize(app, user, NULL, &handle, 1000);

  if(res == CE_RESULT_SUCCESS) {
    CEKeyRoot_t root=CE_KEYROOT_LOCAL_MACHINE;
    CEString key=CEM_AllocateString(_T("SOFTWARE\\Adobe\\Acrobat Distiller\\8.0\\InstallPath"));
    res=CEPROTECT_LockKey(handle, root, key);
    if(res == CE_RESULT_SUCCESS) {
      //At here, you can manually try to change the protected key value 
      //in order to test LockKey function. Then type any key to exit 
      //this program.
      c = getchar();
      res=CEPROTECT_UnlockKey(handle, root, key);
      if(res != CE_RESULT_SUCCESS)
	TRACE(0, _T("Failed to unlock key due to: %d\n"), res);
    } else
      TRACE(0, _T("Failed to lock key due to: %d\n"), res);
    CEM_FreeString(key);
  } else
    TRACE(0, _T("Failed to CECONN_Initialize due to: %d\n"), res);
  CEM_FreeString(appName);
  CEM_FreeString(binaryPath);
  CEM_FreeString(userName);
  CEM_FreeString(type);
  return 0;
}
