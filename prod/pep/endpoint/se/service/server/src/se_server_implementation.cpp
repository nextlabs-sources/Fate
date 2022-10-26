
#define WINVER          _WIN32_WINNT_WINXP
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_WINNT    _WIN32_WINNT_WINXP
#else
#define _WIN32_WINNT    _WIN32_WINNT_WINXP
#endif
#define WINVER          _WIN32_WINNT_WINXP
#define NTDDI_VERSION   NTDDI_WINXPSP2

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <Fltuser.h>


#ifndef _NTDEF_
typedef __success(return >= 0) LONG NTSTATUS, *PNTSTATUS;
#endif

#include "CESDK.h"
#include "pcs_rpc.hpp"
#include "drmctrl.hpp"
#include "NLSECommon.h"


#define NLSE_DRM_PORT_NAME      L"\\NLSysEncryptionPort"
#define NLSE_FWDRM_PORT_NAME    L"\\NLSEFastWritePort"

static const WCHAR fNameW[] = L"C:\\Program Files\\NextLabs\\System Encryption\\config\\SystemEncryption.cfg";  ///hack

BOOL SendListToDriver(_In_ const std::list<std::wstring>& Paths, _In_ LPCWSTR DriverPortName);
BOOL ListToBlob(_In_ const std::list<std::wstring>& Paths,
                _Deref_post_bytecount_x_(*BlobSize) wchar_t **Blob,
                _Out_ int *BlobSize);

//
// Exported functions
//
extern "C"
__declspec( dllexport )
CEResult_t server_SE_DrmGetPaths(
    _Deref_post_bytecount_x_(*out_paths_size) wchar_t **out_paths,
    _Out_ int *out_paths_size,
    _In_ BOOL in_fast_write)
{
    CDRMCtrl drm;   

    if(!drm.Open(fNameW))
        return CE_RESULT_GENERAL_FAILED;
    
    const std::list<std::wstring>& list =
      in_fast_write ? drm.GetFwDrmList() : drm.GetDrmList();
    if(!ListToBlob(list, (wchar_t **) out_paths, out_paths_size))
        return CE_RESULT_GENERAL_FAILED;

    return CE_RESULT_SUCCESS;
}

extern "C"
__declspec( dllexport )
CEResult_t server_SE_DrmAddPath(_In_ const wchar_t *in_path,
    _In_ BOOL in_fast_write)

{
    CDRMCtrl drm;   

    if(NULL==in_path)
        return CE_RESULT_GENERAL_FAILED;

    if(!drm.Open(fNameW))
        return CE_RESULT_GENERAL_FAILED;
    
    if(!drm.AddPath(in_path, in_fast_write, TRUE))
        return CE_RESULT_GENERAL_FAILED;

    const std::list<std::wstring>& list =
      in_fast_write ? drm.GetFwDrmList() : drm.GetDrmList();
    if(!SendListToDriver(list,
        in_fast_write ? NLSE_FWDRM_PORT_NAME : NLSE_DRM_PORT_NAME))
        return CE_RESULT_GENERAL_FAILED;

    return CE_RESULT_SUCCESS;
}

extern "C"
__declspec( dllexport )
CEResult_t server_SE_DrmRemovePath(_In_ const wchar_t *in_path,
    _In_ BOOL in_fast_write)
{
    CDRMCtrl drm;
    
    if(NULL==in_path)
        return CE_RESULT_GENERAL_FAILED;

    if(!drm.Open(fNameW))
        return CE_RESULT_GENERAL_FAILED;
    
    if(!drm.RemovePath(in_path, in_fast_write, TRUE))
        return CE_RESULT_GENERAL_FAILED;

#if 0
    const std::list<std::wstring>& list =
      in_fast_write ? drm.GetFwDrmList() : drm.GetDrmList();
    if(!list.empty() && !SendListToDriver(list,
        in_fast_write ? NLSE_FWDRM_PORT_NAME : NLSE_DRM_PORT_NAME))
        return CE_RESULT_GENERAL_FAILED;
#endif

    return CE_RESULT_SUCCESS;
}

void Initialize(void)
{
    CDRMCtrl drm;
    if(drm.Open(fNameW))
    {
        // Send all list to normal driver
        const std::list<std::wstring>& list = drm.GetDrmList();
        SendListToDriver(list, NLSE_DRM_PORT_NAME);

        // Send all list to fast write driver
        const std::list<std::wstring>& listfw = drm.GetFwDrmList();
        SendListToDriver(listfw, NLSE_FWDRM_PORT_NAME);
    }
}


//
// Local
//
BOOL SendListToDriver(_In_ const std::list<std::wstring>& Paths, _In_ LPCWSTR DriverPortName)
{
    BOOL              bRet = FALSE;
    NLSE_USER_COMMAND cmd;
    HRESULT           hr;
    HANDLE            port;
    NLSE_PORT_CONTEXT portCtx;

    wchar_t* PathBlob = NULL;
    int      PathBlobSize = 0;

    if(!ListToBlob(Paths, &PathBlob, &PathBlobSize) || NULL==PathBlob)
        return FALSE;

    memset(&cmd, 0, sizeof(NLSE_USER_COMMAND));
    cmd.type  =NLSE_USER_COMMAND_SET_DRM_PATHS;
    cmd.msg.params.setDrmPaths.numPaths = static_cast<ULONG>(Paths.size());
    cmd.msg.params.setDrmPaths.paths    = PathBlob;

    portCtx.portTag = NLSE_PORT_TAG_DRM;

    hr = FilterConnectCommunicationPort(DriverPortName, 0, &portCtx, sizeof(portCtx), NULL, &port);  
    if( !IS_ERROR(hr) && port!=NULL)
    {
        DWORD result_size;
        hr = FilterSendMessage(port, &cmd, sizeof(cmd), NULL, 0, &result_size);
        CloseHandle(port);
        bRet =  IS_ERROR(hr)?FALSE:TRUE;
    }

    free(PathBlob);
    return bRet;
}

BOOL ListToBlob(_In_ const std::list<std::wstring>& Paths,
                _Deref_post_bytecount_x_(*BlobSize) wchar_t **Blob,
                _Out_ int *BlobSize)
{
    int Length;
    int MaxLength = 0;
    WCHAR* wzBuf;
    WCHAR* wzPath;
    std::list<std::wstring>::const_iterator it;
    
    // Count size
    for(it=Paths.begin(); it!=Paths.end(); ++it)
    {
        if((*it).length() <= 0)
            continue;
        MaxLength += static_cast<int>((*it).length());   // To hold this string
        MaxLength++;                   // For the NULL end of this string
    }
    if(MaxLength == 0) // No DRM path at all
        return FALSE;
    MaxLength ++;                      // Add one more NULL to end this Blob

    *BlobSize = MaxLength * sizeof(WCHAR);

    // Allocate Buffers
    wzBuf = (WCHAR*)malloc(*BlobSize);
    if(NULL == wzBuf)
        return FALSE;
    memset(wzBuf, 0, *BlobSize);
    *Blob = wzBuf;

    // Copy strings
    wzPath = wzBuf;
    for(it=Paths.begin(); it!=Paths.end(); ++it)
    {
        if((*it).length() <= 0)
            continue;
        wcsncpy_s(wzPath, (*it).length()+1, (*it).c_str(), _TRUNCATE);
        Length = static_cast<int> ((*it).length());    // To hold this string
        Length++;                   // For the NULL end of this string
        wzPath += Length;
    }
    return TRUE;
}