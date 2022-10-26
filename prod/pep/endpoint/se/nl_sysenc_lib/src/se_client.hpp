#ifndef __se_CLIENT_HPP__
#define __se_CLIENT_HPP__


#include <windows.h>
#include <tchar.h>
#include "CEsdk.h"
#include "eframework/platform/cesdk_loader.hpp"

/* Set the CE SDK for RPC client */
extern "C" void pcs_rpc_set_sdk( nextlabs::cesdk_loader* in_cesdk );


extern "C"
CEResult_t SE_DrmGetPaths( CEHandle h ,  wchar_t** out_paths  ,  int* out_paths_size  ,  BOOL in_fast_write , int timeout_ms );


extern "C"
CEResult_t SE_DrmAddPath( CEHandle h ,  const wchar_t* in_path  ,  BOOL in_fast_write , int timeout_ms );


extern "C"
CEResult_t SE_DrmRemovePath( CEHandle h ,  const wchar_t* in_path  ,  BOOL in_fast_write , int timeout_ms );

#endif __se_CLIENT_HPP__
