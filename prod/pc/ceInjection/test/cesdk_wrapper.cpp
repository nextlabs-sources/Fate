/***********************************************************************
 *
 * Compliant Enterprise - Dynamic Injection Unit Test
 *
 **********************************************************************/

#include <cstdio>
#include <cstring>
#include <windows.h>

#include "cesdk_wrapper.h"

static CRITICAL_SECTION utest_cs;
static bool connected = false;
static CEHandle connectHandle;
static CEUser user;
static CEString pdpHostName = NULL;
static CEString type = NULL;

#define TIMEOUT_MS 5000

__declspec(dllexport) bool cesdk_wrapper::Init(void)
{
  InitializeCriticalSection(&utest_cs);

  type = CEM_AllocateString(L"cesdk_wrapper");    // type
  pdpHostName = CEM_AllocateString(L"localhost"); // hostname

  memset(&user,0x00,sizeof(user));
  user.userID   = CEM_AllocateString(L"test");    // user
  user.userName = CEM_AllocateString(L"test");

  connected = false;
  return true;
}/* cesdk_wrapper::Init */

__declspec(dllexport) bool cesdk_wrapper::Connect(void)
{
  EnterCriticalSection(&utest_cs);

  if( connected == false )
  {
    CEApplication app;
    memset(&app,0x00,sizeof(app));
    app.appName   = CEM_AllocateString(L"utest");  // Application
    app.appPath   = CEM_AllocateString(L"");

    CEResult_t res;

    res = CECONN_Initialize(app,user,pdpHostName,&connectHandle,TIMEOUT_MS); 

    if( res == CE_RESULT_SUCCESS )
    {
      connected = true;
    }

    CEM_FreeString(app.appName);
    CEM_FreeString(app.appPath);
  }

  LeaveCriticalSection(&utest_cs);

  return connected;
}/* cesdk_wrapper::Connect */

__declspec(dllexport) bool cesdk_wrapper::Disconnect(void)
{
  EnterCriticalSection(&utest_cs);
  if( connected == true )
  {
    CECONN_Close(connectHandle,CE_INFINITE);
    connected = false;
  }
  LeaveCriticalSection(&utest_cs);
  return true;
}/* cesdk_wrapper::Disconnect */

__declspec(dllexport) bool cesdk_wrapper::Eval( CEAction_t action , const TCHAR* resource_from , const TCHAR* resource_to )
{
  EnterCriticalSection(&utest_cs);

  if( cesdk_wrapper::Connect() == false )
  {
    /* Allow when connection fails */
    LeaveCriticalSection(&utest_cs);
    return true;
  }

  CEApplication   appRun;
  CEString        source = CEM_AllocateString(resource_from);
  CEString        target = CEM_AllocateString(resource_to);
  CEAttributes    sourceAttributes;
  CEAttributes    targetAttributes;

  memset(&sourceAttributes, 0,sizeof(sourceAttributes));
  memset(&targetAttributes, 0,sizeof(targetAttributes));
  memset(&appRun,0x00,sizeof(appRun));

  appRun.appName = CEM_AllocateString(L"URL");

  if( action == CE_ACTION_RUN )
  {
    appRun.appURL  = CEM_AllocateString(resource_from);
  }

  CEAttribute attr;
  sourceAttributes.count = 1;
  sourceAttributes.attrs = &attr;

  targetAttributes.count = 1;
  targetAttributes.attrs = &attr;

  sourceAttributes.attrs[0].key   = CEM_AllocateString(CE_ATTR_LASTWRITE_TIME);
  sourceAttributes.attrs[0].value = CEM_AllocateString(L"123456789");

  CEResult_t res;
  CEEnforcement_t enforcement;

  memset(&enforcement,0x00,sizeof(enforcement));

  res = CEEVALUATE_CheckFile(connectHandle,action,
			     source,&sourceAttributes,
			     target,&targetAttributes,
			     0,&user,&appRun,
			     CETrue,CE_NOISE_LEVEL_USER_ACTION,
			     &enforcement,TIMEOUT_MS);

  if( res != CE_RESULT_SUCCESS )
  {
    fprintf(stdout, "CEEVALUATE_CheckFile failed (result %d)\n", res);
  }

  bool result = true;

  if( res == CE_RESULT_SUCCESS && enforcement.result == CEDeny )
  {
    result = false;
  }

  CEM_FreeString(source);
  CEM_FreeString(target);

  CEM_FreeString(appRun.appName);
  if( appRun.appURL != NULL )
  {
    CEM_FreeString(appRun.appURL);
  }

  CEM_FreeString(sourceAttributes.attrs[0].key);
  CEM_FreeString(sourceAttributes.attrs[0].value);

  LeaveCriticalSection(&utest_cs);

  return result;
}/* cesdk_wrapper::Eval */
