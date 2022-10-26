#pragma warning( push )
#pragma warning( disable : 4005 )
#define _WIN32_WINNT _WIN32_WINNT_WINXP
#pragma warning( pop )
#define NTDDI_VERSION NTDDI_WINXPSP2

#include <windows.h>
#include <WinCrypt.h>
#pragma warning( push )
#pragma warning( disable : 4005 )
#include <fltuser.h>
#pragma warning( pop )

#include <string>
#include <boost/algorithm/string.hpp>

#include "eframework/platform/cesdk_abstraction.hpp"
#include "eframework/platform/cesdk_keymgmt_loader.hpp"
#include "eframework/timer/timer_high_resolution.hpp"
#include "celog.h"
#include "CEsdk.h"
#include "nlconfig.hpp"
#include "se_client.hpp"
#include "NLSESDKWrapper.h"

#include "nl_sysenc_lib.h"
#include "NLSELib.h"



#define CELOG_CUR_FILE \
  CELOG_FILEPATH_PROD_PEP_ENDPOINT_SE_NL_SYSENC_LIB_SRC_NLSESDKWRAPPER_CPP



#define DRM_PATH_OPS_TIMEOUT    5000   /* 5 seconds */

static nextlabs::cesdk_loader cesdk;
static nextlabs::cesdk_keymgmt_loader cesdkKeyMgmt;


typedef CEResult_t (*CESEC_MakeProcessTrusted_t)(CEHandle handle,CEString password);
static CESEC_MakeProcessTrusted_t CESEC_fnMakeProcessTrusted = NULL;

/* Disable implicit cat warning of int to unsigned char */
#pragma warning( push )
#pragma warning( disable : 4245 )
static const unsigned char kmcPassword[] = {7, -117, 34, -79, -74, 85, -10, -63, -99, -120, 103, 15, -48, -46, -8, -88};
#pragma warning( pop )

class CFirstTimeIndicator
{
public:
    CFirstTimeIndicator() : FirstTime_(TRUE)
    {
        InitializeCriticalSection(&cs_);
    }

    ~CFirstTimeIndicator()
    {
        DeleteCriticalSection(&cs_);
    }

    BOOL operator() ()
    {
        CELOG_ENTER;
        BOOL IsFirstTime = FALSE;

        ::EnterCriticalSection(&cs_);
        if(FirstTime_) {
            IsFirstTime = TRUE;
            FirstTime_  = FALSE;
        }
        ::LeaveCriticalSection(&cs_);

        CELOG_RETURN_VAL(IsFirstTime);
    }

private:
    CRITICAL_SECTION    cs_;
    BOOL                FirstTime_;
};

static CFirstTimeIndicator FirstTime;


std::wstring GetSdkDirectory()
{
    CELOG_ENTER;
    std::wstring strPath;
    WCHAR wzDllPath[MAX_PATH+1];

    if( NLConfig::GetComponentInstallPath(L"CommonLibraries", wzDllPath, MAX_PATH) )
    {
        strPath = wzDllPath;
        if(!boost::algorithm::iends_with(strPath, L"\\")) strPath += L"\\";
    }
    else
    {
        strPath = L"C:\\Program Files\\NextLabs\\common\\";
    }
    
    strPath += 
#ifdef _WIN64
        L"bin64";
#else	
        L"bin32";
#endif

    CELOG_RETURN_VAL(strPath);
}

std::wstring GetKmcDirectory()
{
    CELOG_ENTER;
    std::wstring strPath;
    WCHAR wzDllPath[MAX_PATH+1];

    if( NLConfig::GetComponentInstallPath(L"Enterprise DLP\\KeyManagementClient", wzDllPath, MAX_PATH) )
    {
        strPath = wzDllPath;
        if(!boost::algorithm::iends_with(strPath, L"\\")) strPath += L"\\";
    }
    else
    {
        strPath = L"C:\\Program Files\\NextLabs\\Policy Controller\\";
    }
    
    strPath += L"jservice\\KeyManagement\\bin";

    CELOG_RETURN_VAL(strPath);
}

int IsFilePresent(std::wstring strPath, std::wstring strName)
{
    CELOG_ENTER;
    DWORD dwLastError = 0;

    if(!boost::algorithm::iends_with(strPath, L"\\")) strPath += L"\\";
    strPath += strName;

    if(INVALID_FILE_ATTRIBUTES != GetFileAttributesW(strPath.c_str()))
        CELOG_RETURN_VAL(0);

    switch(dwLastError)
    {
    case ERROR_ACCESS_DENIED:
        dwLastError = 0;
        break;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_NAME:
        dwLastError = 1;
        break;
    default:
        dwLastError = 2;
        break;
    }

    CELOG_RETURN_VAL(dwLastError);
}


/** CESDKInit
 *
 *  Load the SDK libraries and functions for use.
 */
bool CESDKInit(void)
{
    CELOG_ENTER;
    // EXE Name
    static std::wstring strAppName;
    static BOOL         bIsSeRecovery = FALSE;
    static std::wstring strSdkDirectory;
    static std::wstring strKmcDirectory;

#ifdef _WIN64
    static const std::wstring strSdkName(L"cesdk.dll");
    static const std::wstring strKmcName(L"KeyManagementConsumer.dll");
#else	
    static const std::wstring strSdkName(L"cesdk32.dll");
    static const std::wstring strKmcName(L"KeyManagementConsumer32.dll");
#endif

    //
    // Is this app NLSERecovery.exe?
    //
    if(strAppName.empty())
    {
        WCHAR wzAppName[MAX_PATH+1];
        GetModuleFileNameW(NULL, wzAppName, MAX_PATH);
        strAppName = wzAppName;
        if(!bIsSeRecovery && boost::algorithm::iends_with(strAppName, L"\\NLSERecovery.exe"))
            bIsSeRecovery = TRUE;
    }

    //
    // Get DLL path
    //
    if(strSdkDirectory.empty())
    {
        strSdkDirectory = bIsSeRecovery ? L"." : GetSdkDirectory();
    }
    if(strKmcDirectory.empty())
    {
        strKmcDirectory = bIsSeRecovery ? L"." : GetKmcDirectory();
    }

    //
    // Are all DLLs installed?
    //
    if(0!=IsFilePresent(strSdkDirectory, strSdkName) || 0!=IsFilePresent(strKmcDirectory, strKmcName))
        CELOG_RETURN_VAL(false);

    //
    // Load dlls if they are not loaded
    //
    if(!cesdk.is_loaded())
    {
        // we need to load sdk
        if(!cesdk.load(strSdkDirectory.c_str()))
            CELOG_RETURN_VAL(false);

        // set sdk
        pcs_rpc_set_sdk(&cesdk);
    }

    if(!cesdkKeyMgmt.is_loaded())
    {
        // we need to load key management dll
        if(!cesdkKeyMgmt.load(strKmcDirectory.c_str()))
            CELOG_RETURN_VAL(false);
    }

    CELOG_RETURN_VAL(true);
}/* CESDKInit */

_Check_return_ nextlabs::cesdk_loader* GetCeSdkInstance()
{
    CELOG_PTR_ENTER(nextlabs::cesdk_loader*);
    if(FirstTime()) {

        if(!CESDKInit()) {
            CELOG_PTR_RETURN_VAL(NULL);
        }
    }
    
    CELOG_PTR_RETURN_VAL((cesdk.is_loaded() && cesdkKeyMgmt.is_loaded()) ? (&cesdk) : NULL);
}

CEResult_t CESDKMakeProcessTrusted(const wchar_t *password)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res         = CE_RESULT_SUCCESS;
  CEString   passwordStr = NULL;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  if(NULL == CESEC_fnMakeProcessTrusted)
  {
      HMODULE hLib = sdk->get_mod();
      if(NULL == hLib)
          CELOG_ENUM_RETURN_VAL(CE_RESULT_GENERAL_FAILED);

      CESEC_fnMakeProcessTrusted = (CESEC_MakeProcessTrusted_t)GetProcAddress(hLib,"CESEC_MakeProcessTrusted");
      if(NULL == CESEC_fnMakeProcessTrusted)
          CELOG_ENUM_RETURN_VAL(CE_RESULT_GENERAL_FAILED);
  }
  
  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  passwordStr = sdk->fns.CEM_AllocateString(password);
  if (passwordStr == NULL) {
      sdkconn.disconnect();
      CELOG_ENUM_RETURN_VAL(CE_RESULT_GENERAL_FAILED);
  }

  res = CESEC_fnMakeProcessTrusted(sdkconn.get_connection_handle(), passwordStr);
  sdkconn.disconnect();
  sdk->fns.CEM_FreeString(passwordStr);
  CELOG_ENUM_RETURN_VAL(res);
}/* CESDKMakeProcessTrusted */


CEResult_t CESDKKeyManagementAPI(NLSE_MESSAGE *inMsg,
				 NLSE_MESSAGE *outMsg)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEKey key;
  CEKeyID keyID;
  CEString keyRingName = NULL;
  WCHAR wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN + 1] = {0};

  
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }
  
  //set key ring name
  MultiByteToWideChar(CP_UTF8, 0, 
		      inMsg->keyRingName, NLE_KEY_RING_NAME_MAX_LEN,
		      wKeyRingName, NLE_KEY_RING_NAME_MAX_LEN);
  wKeyRingName[NLE_KEY_RING_NAME_MAX_LEN] = L'\0';
  keyRingName=sdk->fns.CEM_AllocateString(wKeyRingName);
  
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
  
  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  nextlabs::high_resolution_timer ht;
  CEResult_t res = cesdkKeyMgmt.fns.CEKey_GetKey( sdkconn.get_connection_handle(), 
						  kmcPassword,
						  _countof(kmcPassword),
						  keyRingName,
						  keyID,
						  &key,
						  inMsg->pid,
						  DEFAULT_KM_TIMEOUT/5);//1 sec
  sdkconn.disconnect();
  //L"CESDKKeyManagementAPI: CEKey_GetKey %fms (res %d)\n",ht.diff(),res);
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

  sdk->fns.CEM_FreeString(keyRingName);
  CELOG_ENUM_RETURN_VAL(res);
}

/** CESDKDrmGetPaths
 *
 *  \brief Get the list of DRM paths from PC Service.
 *
 *  \param out_paths (out)    blob of DRM paths concatenated together
 *  \param out_path_size (out) # of bytes in out_paths
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmGetPaths(wchar_t **out_paths, int *out_paths_size)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmGetPaths(sdkconn.get_connection_handle(), out_paths, out_paths_size,
                        FALSE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmGetPaths */

CEResult_t CESDKDrmGetFwPaths(wchar_t **out_paths, int *out_paths_size)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmGetPaths(sdkconn.get_connection_handle(), out_paths, out_paths_size,
                        TRUE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmGetFwPaths */

/** CESDKDrmAddPath
 *
 *  \brief Tell the PC Service to add the path to the DRM path list.
 *
 *  \param in_paths (in)      path to add
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmAddPath(const wchar_t *in_path)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmAddPath(sdkconn.get_connection_handle(), in_path, FALSE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmAddPath */

CEResult_t CESDKDrmAddFwPath(const wchar_t *in_path)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmAddPath(sdkconn.get_connection_handle(), in_path, TRUE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmAddFwPath */

/** CESDKDrmRemovePath
 *
 *  \brief Tell the PC Service to remove the path from the DRM path list.
 *
 *  \param in_paths (in)      path to remove
 *
 *  \return CE_RESULT_SUCCESS if okay
 */
CEResult_t CESDKDrmRemovePath(const wchar_t *in_path)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmRemovePath(sdkconn.get_connection_handle(), in_path, FALSE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmRemovePath */

CEResult_t CESDKDrmRemoveFwPath(const wchar_t *in_path)
{
  CELOG_ENUM_ENTER(CEResult_t);
  CEResult_t res;
  nextlabs::cesdk_loader* sdk;

  sdk = GetCeSdkInstance();
  if(NULL == sdk) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_INIT_CESDK);
  }

  nextlabs::cesdk_connection sdkconn;
  sdkconn.set_sdk(sdk);

  if (!sdkconn.connect()) {
      CELOG_ENUM_RETURN_VAL((CEResult_t)NLSE_ENC_ERROR_CANT_CONNECT_PC);
  }

  res = SE_DrmRemovePath(sdkconn.get_connection_handle(), in_path, TRUE, DRM_PATH_OPS_TIMEOUT);
  sdkconn.disconnect();

  CELOG_ENUM_RETURN_VAL(res);
} /* CESDKDrmRemoveFwPath */