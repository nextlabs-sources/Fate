

/* This new Agent controller uses the new SDK interface                        */
/* Show case how to write a client program base on the NL SDK                  */

/* Note the SDK itself is cross platform, but this is an application           */
/* above the SDK, so it's up to the client user to write a cross platform code */
/* here.   Since it's more for showcase purpose. I will not use our internal   */
/* abstraction layer brain or nlstring here, otherwise a simple brain.h will   */
/* cover all.  This shows how useful our internal abstraction is.              */

#if defined (WIN32) || defined (_WIN64)
#include <tchar.h>
typedef TCHAR  myCommonChar;
#define API __stdcall
#endif

#if defined (Linux) || defined (Darwin)
typedef char   myCommonChar;
#define _T(x)  x
#define _tcscpy nlstrcpy
#define API
#endif

#include "CEsdk.h"
#include "CEPrivate.h"
#include "brain.h"


CEResult_t API stopAgentService (myCommonChar * lpszPassword)
{
  CEResult_t ret      = CE_RESULT_GENERAL_FAILED;
  CEHandle   handle   = NULL;
  CEString   appName  = NULL;
  CEString   password = NULL;
  
  appName = CEM_AllocateString (_T("StopEnforcer"));
  if (!appName) {
    return ret;
  }
  
  //"handle" is allocated by PDP
  CEApplication app;
  CEUser user;
  app.appName=appName;
  app.appPath=NULL;
  user.userName=NULL;
  user.userID=NULL;
  CEString type = CEM_AllocateString(_T("stopAgentService"));

  ret = CECONN_DLL_Activate(app, user, NULL, &handle, 10000);
  if (ret != CE_RESULT_SUCCESS) {
    if (appName)  CEM_FreeString (appName);
    if (type) CEM_FreeString (type);
    return ret;
  }

  password = CEM_AllocateString (lpszPassword);

  if (!password) {
    if (appName)  CEM_FreeString (appName);
    if (type) CEM_FreeString (type);
    ret = CE_RESULT_GENERAL_FAILED;
    CECONN_DLL_Deactivate(handle, 10000); 
    return ret;
  }

  //Stop PDP and "handle" will be freed by PDP also
  ret = CEP_StopPDP (handle, password, 30000); // 30 second timeout

  if (appName)  CEM_FreeString (appName);
  if (password) CEM_FreeString (password);
  if (type) CEM_FreeString (type);

#if defined(WIN32) || defined(_WIN64)
  // Making sure the PEP is shutting down as this DLL will be loaded
  // by the stupid installshield.  Shutdown call guarantee the internal
  // thread will be clean up.  
  //
  // This spits out a few errors on Linux, which are ugly, and we don't
  // have the installshield problem, so we'll just skip it here
  CECONN_DLL_Deactivate(handle, 200);
#endif
  return ret;

}

CEResult_t API stopAgentServiceWithoutPassword (myCommonChar * lpszPassword)
{
  CEResult_t ret      = CE_RESULT_GENERAL_FAILED;
  CEHandle   handle   = NULL;
  CEString   appName  = NULL;
  CEString   password = NULL;
  
  appName = CEM_AllocateString (_T("StopEnforcer"));
  if (!appName) {
    return ret;
  }
  
  CEString type = CEM_AllocateString (_T("StopEnforcer"));

  //"handle" is allocated by PDP
  CEApplication app;
  CEUser user;
  app.appName=appName;
  app.appPath=NULL;
  user.userName=NULL;
  user.userID=NULL;
  int maxTries=60;

  for(; maxTries >0; --maxTries) {
    //Retry for 5 minutes with a frequency of 5 seconds, timeout 30 seconds  
    ret = CECONN_DLL_Activate(app, user,NULL, &handle, 30000);
    if (ret == CE_RESULT_SUCCESS) 
      break;
    NL_sleep(5000);
  }

  if (ret != CE_RESULT_SUCCESS) {
    if (appName)  CEM_FreeString (appName);
    if (type) CEM_FreeString (type);
    return ret;
  }

  password = CEM_AllocateString (lpszPassword);

  if (!password) {
    if (appName)  CEM_FreeString (appName);
    if (type) CEM_FreeString (type);
    ret = CE_RESULT_GENERAL_FAILED;
    CECONN_DLL_Deactivate(handle, CE_INFINITE); 
    return ret;
  }

  //Stop PDP and "handle" will be freed by PDP also
  ret = CEP_StopPDPWithoutPassword (handle, password, 30000);  // 30 second timeout

  if (appName)  CEM_FreeString (appName);
  if (password) CEM_FreeString (password);
  if (type) CEM_FreeString (type);

#if defined(WIN32) || defined(_WIN64)
  // Making sure the PEP is shutting down as this DLL will be loaded
  // by the stupid installshield.  Shutdown call guarantee the internal
  // thread will be clean up.  
  //
  // This spits out a few errors on Linux, which are ugly, and we don't
  // have the installshield problem, so we'll just skip it here
  CECONN_DLL_Deactivate(handle, 200);
#endif
  return ret;

}


CEResult_t API stopAgentServiceWithChallenge (nlchar * challenge) 
{

  CEResult_t ret          = CE_RESULT_GENERAL_FAILED;
  CEHandle   handle       = NULL;
  CEString   appName      = NULL;
  CEString   ce_challenge = NULL; 
  CEString	 type		  = NULL;
  const myCommonChar * tmpstr = NULL;

  if (!challenge) {
    goto fail_and_cleanup;
  }

  appName = CEM_AllocateString (_T("StopEnforcer"));

  if (!appName) {
    goto fail_and_cleanup;
  }

  type = CEM_AllocateString (_T("StopEnforcer"));

  CEApplication app;
  CEUser user;
  app.appName=appName;
  app.appPath=NULL;
  user.userName=NULL;
  user.userID=NULL;
  ret = CECONN_DLL_Activate(app, user, NULL, &handle, CE_INFINITE);

  if (ret != CE_RESULT_SUCCESS) {
	goto fail_and_cleanup;
  }

  ret = CEP_GetChallenge (handle, ce_challenge, CE_INFINITE);

  if (ret != CE_RESULT_SUCCESS) {
    goto fail_and_cleanup;
  }

  tmpstr = CEM_GetString (ce_challenge);

  // This is bad since we don't know the size of the challenge storage
  // but have to keep it for the installscript..., which is (hopefully)
  // the only place that uses it.
  _tcscpy (challenge, tmpstr);

 fail_and_cleanup:
  if (handle)       CECONN_DLL_Deactivate(handle, 1000);
  if (appName)      CEM_FreeString (appName);
  if (ce_challenge) CEM_FreeString (ce_challenge);
  if (type) CEM_FreeString (type);

  return ret;
}

