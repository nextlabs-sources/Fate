#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <tchar.h>
#include <fltuser.h>
#include "eframework/platform/cesdk_loader.hpp"
#include "eframework/platform/cesdk_keymgmt_loader.hpp"
#include "celog.h"
#include "nlconfig.hpp"
#include "NLSESDKWrapper.h"

#define CELOG_CUR_MODULE L"NLSEPlugin"
#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENCRYPTION_USER_NLSESDKWRAPPER_CPP

#define CONNECT_TIMEOUT   1000 /* 1 second */
#define GETKEY_TIMEOUT_MS (5000)  /* 5 second */

static CEHandle ceHandle=NULL;
nextlabs::cesdk_loader cesdk;
nextlabs::cesdk_keymgmt_loader cesdkKeyMgmt;

/* Disable implicit cat warning of int to unsigned char */
#pragma warning( push )
#pragma warning( disable : 4245 )
static const unsigned char kmcPassword[] = {7, -117, 34, -79, -74, 85, -10, -63, -99, -120, 103, 15, -48, -46, -8, -88};
#pragma warning( pop )

#define GET_CESDK_FROM_POLICY_CONTROLLER

/** CESDKInit
 *
 *  Load the SDK libraries and functions for use.
 */
static BOOL LoadSdkFromPolicyController();
static BOOL LoadSdkFromCommonLibrary();

bool CESDKInit(void)
{
  CELOG_LOG(CELOG_DEBUG,L"CESDKInit: loading\n");

  WCHAR dll_path[MAX_PATH] = {0};
  

#ifdef GET_CESDK_FROM_POLICY_CONTROLLER
  if(!LoadSdkFromPolicyController())
  {
      if(!LoadSdkFromCommonLibrary())
      {
          CELOG_LOG(CELOG_CRITICAL,L"CESDKInit: load SDK failed\n");
          return false;
      }
  }
#else
  if(!LoadSdkFromCommonLibrary())
  {
      CELOG_LOG(CELOG_CRITICAL,L"CESDKInit: load SDK failed\n");
      return false;
  }
#endif

  if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\KeyManagementClient",dll_path,_countof(dll_path)) == true )
  {
    wcsncat_s(dll_path,_countof(dll_path),L"\\jservice\\KeyManagement\\bin",_TRUNCATE);
  }
  else
  {
    CELOG_LOG(CELOG_ERROR,L"CESDKInit: Key Management Client Component path failed.  Using default.\n");
    wcsncpy_s(dll_path,_countof(dll_path),L"C:\\Program Files\\NextLabs\\Policy Controller\\jservice\\KeyManagement\\bin",_TRUNCATE);
  }

  return cesdkKeyMgmt.load(dll_path);
}/* CESDKInit */

/** CESDKConnect
 *
 *  \brief Perform sdk connection to policy controller
 *
 *  \param resource_from (in) (Device) resource.
 *  \param user_name (in)     User name for evaluation.
 *  \param user_sid (in)      User SID for evaluation.
 *  \param deny (out)         Result of evaluation.
 *
 *  \return false on error.
 */
bool CESDKConnect()
{
  CEUser        user;
  CEApplication app;
  CEResult_t    res;

  if(ceHandle != NULL) {
    //has been connected; do nothing
      CELOG_LOG(CELOG_DEBUG, L"CESDKConnect: Already connected - do nothing\n");
    return true;
  }

  user.userID=NULL;
  user.userName=NULL;
  
  memset(&app,0x00,sizeof(app));
  app.appName = cesdk.fns.CEM_AllocateString(L"nlse plugin");  // Application
  app.appPath = cesdk.fns.CEM_AllocateString(L"");

  CELOG_LOG(CELOG_INFO,L"CESDKConnect: connecting\n");
  res = cesdk.fns.CECONN_DLL_Activate(app,
				      user,
				      NULL,
				      &ceHandle,
				      CONNECT_TIMEOUT);
  cesdk.fns.CEM_FreeString(app.appName);
  cesdk.fns.CEM_FreeString(app.appPath);
  if( res != CE_RESULT_SUCCESS ) {
    CELOG_LOG(CELOG_ERROR,L"CESDKConnect: CECONN_DLL_Activate failed err=%d\n", res);
    return false;
  }

  CELOG_LOG(CELOG_INFO,L"CESDKConnect: connection ok\n");
  return true;
}/* CESKDConnect */


/** CESDKDisconnect
 *
 *  \brief Perform sdk disconnection
 *
 *  \return false on error.
 */
bool CESDKDisconnect()
{
  CELOG_LOG(CELOG_INFO,L"CESDKDisconnect: disconnecting\n");
  cesdk.fns.CECONN_DLL_Deactivate(ceHandle,CONNECT_TIMEOUT);
  return true;
}/* CESDKDisconnect */


CEResult_t CESDKKeyManagementAPI(NLSE_MESSAGE *inMsg,
				 NLSE_MESSAGE *outMsg)
{
  CEKey key;
  CEKeyID keyID;
  CEString keyRingName = NULL;
  WCHAR wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN + 1] = {0};
  
  //set key ring name
  MultiByteToWideChar(CP_UTF8, 0, 
		      inMsg->keyRingName, NLE_KEY_RING_NAME_MAX_LEN,
		      wKeyRingName, NLE_KEY_RING_NAME_MAX_LEN);
  wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN] = L'\0';
  keyRingName=cesdk.fns.CEM_AllocateString(wKeyRingName);
  
  //set key ID
  memcpy(&keyID, &inMsg->keyID, sizeof(keyID));
#if 0
  printf("Get Key:\nID:");
  for(int i=0; i<KM_HASH_LEN; i++) {
    printf("%x", keyID.hash[i]);
  }
  printf("\nTime:%d\n", keyID.timestamp);
#endif

  //Initialize key
  key.struct_version = 1;

  //calling SDK to get key
  CELOG_LOG(CELOG_DEBUG,
	    L"CESDKKeyManagementAPI: GetKey: key ring = '%s', pid = %d\n",
            wKeyRingName, inMsg->pid);
  nextlabs::high_resolution_timer ht;
  CEResult_t res = cesdkKeyMgmt.fns.CEKey_GetKey(ceHandle,
						 kmcPassword,
						 _countof(kmcPassword),
						 keyRingName,
						 keyID,
						 &key,
						 inMsg->pid,
						 GETKEY_TIMEOUT_MS);//1 sec
  ht.stop();
  CELOG_LOG( res == CE_RESULT_SUCCESS ? CELOG_DEBUG : CELOG_CRITICAL,
	    L"CESDKKeyManagementAPI: CEKey_GetKey %fms (res %d)\n",
	     ht.diff(),res);
  if( CE_RESULT_SUCCESS == res ) {
    memcpy(outMsg->key, key.key, sizeof(outMsg->key));
    memcpy(&(outMsg->keyID), &(key.id), sizeof(outMsg->keyID));
#if 0
    printf("Get Key succeed:\nID:");
    for(int i=0; i<NLSE_KEY_ID_LENGTH_IN_BYTES; i++) {
      printf("%x", outMsg->keyID.hash[i]);
    }
    printf("\nTime:%d\n", outMsg->keyID.timestamp);
#endif
  }

  cesdk.fns.CEM_FreeString(keyRingName);
  return res;
}

/** CESDKCheckFileSimple
 *
 *  \brief Perform a simple check-file SDK evaluation without all the functionality in the full-blown CEEVALUATE_CheckFile().
 *
 *  \return CE_RESULT_SUCCESS on success.
 */
CEResult_t CESDKCheckFileSimple(const WCHAR *sourceFullFileName,
                                const WCHAR *targetFullFileName,
                                CEAction_t action,
                                const WCHAR *appName,
                                CEResponse_t *pResponse)
{
  CEString srcStr = NULL, targetStr = NULL;
  CEAttributes srcAttrs = {NULL, 0}, targetAttrs = {NULL, 0};
  const CEint32 ip = 0;                 // localhost
  CEUser user = {NULL, NULL};
  CEApplication app = {NULL, NULL, NULL};
  CEEnforcement_t enf = {CEAllow, NULL};
  const CEint32 timeout = 3 * 1000;
  CEResult_t ret;

  srcStr = cesdk.fns.CEM_AllocateString(sourceFullFileName);
  targetStr = cesdk.fns.CEM_AllocateString(targetFullFileName);

  srcAttrs.count = 0;
  srcAttrs.attrs = new CEAttribute;
  if (srcAttrs.attrs != NULL)
  {
    srcAttrs.attrs[0].key =
      cesdk.fns.CEM_AllocateString(CE_ATTR_LASTWRITE_TIME);
    srcAttrs.attrs[0].value =
      cesdk.fns.CEM_AllocateString(L"0123456789");  /* magic number! */
    srcAttrs.count = 1;
  }

  user.userName = cesdk.fns.CEM_AllocateString(L"");
  user.userID = cesdk.fns.CEM_AllocateString(L"0");
  app.appName = cesdk.fns.CEM_AllocateString(appName);
  app.appPath = cesdk.fns.CEM_AllocateString(L"");
  app.appURL = NULL;

  if ((srcStr == NULL || targetStr == NULL) ||
      (srcAttrs.attrs == NULL || srcAttrs.attrs[0].key == NULL ||
       srcAttrs.attrs[0].value == NULL) ||
      (user.userName == NULL || user.userID == NULL) ||
      (app.appName == NULL || app.appPath == NULL))
  {
    ret = CE_RESULT_GENERAL_FAILED;
    goto cleanup;
  }

  ret = cesdk.fns.CEEVALUATE_CheckFile(ceHandle, action, srcStr, &srcAttrs,
                                       targetStr, &targetAttrs, ip, &user,
                                       &app, CETrue,
                                       CE_NOISE_LEVEL_USER_ACTION,
                                       &enf, timeout);

  *pResponse = enf.result;
  cesdk.fns.CEEVALUATE_FreeEnforcement(enf);

cleanup:
  cesdk.fns.CEM_FreeString(app.appPath);
  cesdk.fns.CEM_FreeString(app.appName);
  cesdk.fns.CEM_FreeString(user.userID);
  cesdk.fns.CEM_FreeString(user.userName);

  if (srcAttrs.attrs != NULL)
  {
    cesdk.fns.CEM_FreeString(srcAttrs.attrs[0].value);
    cesdk.fns.CEM_FreeString(srcAttrs.attrs[0].key);
    delete srcAttrs.attrs;
  }

  cesdk.fns.CEM_FreeString(targetStr);
  cesdk.fns.CEM_FreeString(srcStr);

  return ret;
}


BOOL LoadSdkFromPolicyController()
{
  CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromPolicyController: loading\n");

  WCHAR dll_path[MAX_PATH] = {0};
  WCHAR componentName[] = L"Compliant Enterprise\\Policy Controller";

  if( NLConfig::GetComponentInstallPath(componentName,dll_path,_countof(dll_path)) == true )
  {
    wcsncat_s(dll_path,_countof(dll_path),L"\\bin",_TRUNCATE);
  }
  else
  {
    CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromPolicyController: Cannot get policy Controller install path.  Using default.\n");
    wcsncpy_s(dll_path,_countof(dll_path),L"C:\\Program Files\\NextLabs\\Policy Controller\\bin",_TRUNCATE);
  }

  if(!cesdk.load(dll_path))
  {
    CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromPolicyController: load SDK failed\n");
    return FALSE;
  }

  CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromPolicyController: load SDK succeed\n");
  return TRUE;
}

BOOL LoadSdkFromCommonLibrary()
{
  CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromCommonLibrary: loading\n");

  WCHAR dll_path[MAX_PATH] = {0};
  WCHAR componentName[] = L"CommonLibraries";

  if( NLConfig::GetComponentInstallPath(componentName,dll_path,_countof(dll_path)) == true )
  {
#ifdef _WIN64
    wcsncat_s(dll_path,_countof(dll_path),L"\\bin64",_TRUNCATE);
#else
    wcsncat_s(dll_path,_countof(dll_path),L"\\bin32",_TRUNCATE);
#endif
  }
  else
  {
    CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromCommonLibrary: Fail to find CommonLibraries Component path.  Using default.\n");
#ifdef _WIN64
    wcsncpy_s(dll_path,_countof(dll_path),L"C:\\Program Files\\NextLabs\\common\\bin64",_TRUNCATE);
#else
    wcsncpy_s(dll_path,_countof(dll_path),L"C:\\Program Files\\NextLabs\\common\\bin32",_TRUNCATE);
#endif
  }

  if(!cesdk.load(dll_path))
  {
    CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromCommonLibrary: load SDK failed\n");
    return FALSE;
  }

  CELOG_LOG(CELOG_DEBUG,L"LoadSdkFromCommonLibrary: load SDK succeed\n");
  return TRUE;
}