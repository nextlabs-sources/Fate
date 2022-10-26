/*=====================================policy.cpp===========================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2008 by NextLabs,     *
 * San Mateo, CA, Ownership remains with NextLabs Inc,                      * 
 * All rights reserved worldwide.                                           *
 *                                                                          * 
 * Author :                                                                 *
 * Date   : 03/06/2008                                                      *
 * Note   : Handling policy evaluation for tamper proof                     *
 *==========================================================================*/
#include <errno.h>
#include "tamper_private.h"

namespace {
/********************************************************
 * The following functions are scoped in this file only *
 ********************************************************/
//Get host ip address
bool GetHostIPAddr(struct in_addr &hostIPAddr)
{
//   char hostName[1024];
//   struct addrinfo hints, *res;
//   int err;
// 
// #if defined (WIN32) || defined (_WIN64)
//   WORD wVersionRequested;
//   WSADATA wsaData;
// 
//   wVersionRequested = MAKEWORD( 2, 2 );
//  
//   err = WSAStartup( wVersionRequested, &wsaData );
//   if ( err != 0 ) {
//     /* Tell the user that we could not find a usable */
//     /* WinSock DLL.                                  */
//     TRACE(0, _T("Failed to WSAStartup: error=%d\n"), WSAGetLastError());
//     hostIPAddr.s_addr=123;
//     return false;
//   }   
// #endif
// 
//   if(gethostname(hostName, 1024) != 0) {
// #if defined (WIN32) || defined (_WIN64)
//     TRACE(0, _T("Failed to get host name: error=%d\n"), WSAGetLastError());
// #else
//     TRACE(0, _T("Failed to get host name: error=%d\n"), errno);
// #endif
//     hostIPAddr.s_addr=123;
//     return false;
//   }
// 
//   memset(&hints, 0, sizeof(hints));
//   hints.ai_socktype = SOCK_STREAM;
//   hints.ai_family = AF_INET;
// 
//   if ((err = getaddrinfo(hostName, NULL, &hints, &res)) != 0) {
//     TRACE(0, _T("Failed to get host ip: error=%d\n"), err);
//     hostIPAddr.s_addr=123;
//     return false;
//   }
// 
//   hostIPAddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
// 
//   //printf("ip address : %s %lu\n", inet_ntoa(hostIPAddr), hostIPAddr.s_addr);
// 
//   freeaddrinfo(res);

  hostIPAddr.s_addr = (int)0x0100007f;
  return true;  
}

//retrieves the path of the executable file of the current process.
bool GetCurrentProcessFullName(nlchar* fullFileNameBuf, DWORD bufLen)
{
#if defined (WIN32) || defined (_WIN64)
  DWORD dwResult = 0;
  if (TRUE != ::IsBadReadPtr((PBYTE)fullFileNameBuf, sizeof(nlchar) * bufLen))
    dwResult = ::GetModuleFileNameW(NULL,              // handle to module
				    fullFileNameBuf,   // file name of module
				    bufLen             // size of buffer
				    );
  if(dwResult == 0) 
    TRACE(0, _T("Tamperproof: GetCurrentProcessFullName failed: err=%d\n"), 
	  GetLastError());
  return (dwResult != 0);
#else
  return false;
#endif
}

}
//Constructor
TamperPolicy::TamperPolicy():ceHandle(NULL), bInitFailed(false)
{
  GetHostIPAddr(hostIPAddr);
  nlthread_mutex_init(&connMutex);
}

//Constructor
TamperPolicy::~TamperPolicy()
{
  nlthread_mutex_destroy(&connMutex);
  //Disconnect to policy controller
  if(ceHandle) 
    CECONN_DLL_Deactivate(ceHandle, 10000);  // There's no particular reason this should be exactly 10 seconds; it should be enough. 
}

// Setup connection to policy controller.
bool TamperPolicy::SetupConnection()
{
  nlthread_mutex_lock(&connMutex);    
  TRACE(0, _T("Tamperproof: Setup Connection\n"));

  //The connection has been failed to setup; everything is allowed
  if(bInitFailed == true) {
    nlthread_mutex_unlock(&connMutex);
    return false;
  }
 
  //The connection has been setup
  if(ceHandle != NULL) {
    nlthread_mutex_unlock(&connMutex);
    return true;
  }

  if(bConnInProgress) {
    //On Windows platform, setting up socket connection will call CreateFile 
    //and it causes recursion of calling CECONN_Initialize. In order to avoid
    //this kind of recursion, bConnInProgress will be set to true when the
    //connection is setting up and CECONN_Initialize won't be called again. 
    nlthread_mutex_unlock(&connMutex);
    return false;
  }

  bConnInProgress=true;

  CEString type=CEM_AllocateString(_T("TamperProof"));
  CEString binaryPath=NULL;
  CEUser user;
  CEApplication app;
  nlchar processName[1024];

  user.userName=NULL;
  user.userID=NULL;
  if(!GetCurrentProcessFullName(processName, 1024))
    app.appPath=NULL;
  else {
    binaryPath=CEM_AllocateString(processName);
    app.appPath=binaryPath;
  }
  app.appName=NULL;

  CEResult_t res=CECONN_DLL_Activate(app,
				   user, 
				   NULL, //local policy controller
				   &ceHandle, 
				   5000);
  if(res != CE_RESULT_SUCCESS) {
    TRACE(0, _T("CECONN_Initialize failed: errorno=%d\n"), res);
    ceHandle=NULL;
    bInitFailed=true;
  }

  if(binaryPath)
    CEM_FreeString(binaryPath);
  CEM_FreeString(type);

  bConnInProgress=false;
  nlthread_mutex_unlock(&connMutex);

  return (res==CE_RESULT_SUCCESS)?true:false;
}

//Do policy evaluation; if allow, returns true.
bool TamperPolicy::evaluation(CEAction_t action, 
			      const nlchar *from, 
			      const nlchar *to)
{
  //connection setup failed, return true to allow
  if(!SetupConnection())
    return true;

  TRACE(0, _T("Tamperproof: Do Policy Evaluation\n"));

  CEString fromString=CEM_AllocateString(from);
  CEString toString=NULL;

  //Get source attribute
  //Get source's last modify date
  nlchar lastModifyDate[1024]; 
  memset(lastModifyDate, 0, sizeof(lastModifyDate));
  NLint64 t=NL_GetFileModifiedTime(from, NL_OpenProcessToken());
  nlsprintf(lastModifyDate, _countof(lastModifyDate), _T("%d"), t);
  if(0 == lastModifyDate[0]) 
    nlstrncpy_s(lastModifyDate, _countof(lastModifyDate), _T("123456789"), 9);
  CEString modAttrKey=CEM_AllocateString(_T("modified_date"));
  CEString modAttrVal=CEM_AllocateString(lastModifyDate);
  CEString purposeAttrKey=CEM_AllocateString(_T("CE::Purpose"));
  CEString purposeAttrVal=CEM_AllocateString(_T("tamper proofing"));
  CEAttribute fromAttr[2];
  CEAttributes fromAttrs;  
  fromAttr[0].key = modAttrKey;
  fromAttr[0].value = modAttrVal;
  fromAttr[1].key = purposeAttrKey;
  fromAttr[1].value = purposeAttrVal;
  fromAttrs.count=2;
  fromAttrs.attrs=fromAttr;

  //Get to resource
  if(to) 
    toString=CEM_AllocateString(to);

  //Do evaluation
  CEEnforcement_t enf;
  bool retried = false;
 retry:
  CEResult_t result=CEEVALUATE_CheckFile(ceHandle, 
					 action,
					 fromString,
					 &fromAttrs, //from attrs
					 toString,
					 NULL, //to attrs
					 (CEint32)((hostIPAddr.s_addr)),//ip 
					 NULL, //user
					 NULL, //application
					 CETrue, //performObligation 
					 CE_NOISE_LEVEL_USER_ACTION, 
					 &enf,
					 5000); //timeout in milliseconds
  if (result == CE_RESULT_CONN_FAILED && retried == false) {
      // TRACE(CELOG_INFO, _T("TamperProof: CEEVALUATE_CheckFile() failed because of connection problem.  Will re-establish connection.\n"));
      retried = true;
      
      // disconnect from Policy Controller
      CECONN_DLL_Deactivate(ceHandle, 10000); // There's no particular reason this should be exactly 10 seconds; it should be enough. 
      ceHandle = NULL;
      bInitFailed = false;
      
      // try to re-connect
      bool rv = SetupConnection();
      if (rv == true) {
	  // connection re-establishment succeeded.  Now retry CheckFile().
	  // TRACE(CELOG_INFO, _T("connection re-establishment succeeded.  Now retry CheckFile().\n"));
	  goto retry;
      } else {
	  // connection re-establishment failed.  Clean up and return.
	  // TRACE(CELOG_ERR, _T("TamperProof: Failed to re-establish connection.  Everything will be allowed.\n"));
      }
  }

  //Clean up
  CEM_FreeString(modAttrKey);
  CEM_FreeString(modAttrVal);
  CEM_FreeString(purposeAttrKey);
  CEM_FreeString(purposeAttrVal);
  CEM_FreeString(fromString);
  if(toString)
    CEM_FreeString(toString);

  if(result != CE_RESULT_SUCCESS) {
      // TRACE(CELOG_ERR, _T("CEEVALUATE_CheckFile() returned error %d\n"), result);
      return true; //evaluation failed: allow
  }

  return (enf.result==CEDeny)?false:true;
}

