#include "pcs_rpc_packer.hpp"
#include <tchar.h>
#include "CEsdk.h"
#include "eframework/platform/cesdk_loader.hpp"


static nextlabs::cesdk_loader* cesdk = NULL;


extern "C" void pcs_rpc_set_sdk( nextlabs::cesdk_loader* in_cesdk )
{
  assert( in_cesdk != NULL );
  cesdk = in_cesdk;
}


#include <cstdio>
#include <cstdlib>
#include <windows.h>

#include "CEsdk.h"
#include "ceservice.h"


extern "C"
CEResult_t SE_DrmGetPaths( CEHandle h ,  wchar_t** out_paths  ,  int* out_paths_size  ,  BOOL in_fast_write  , int in_timeout )
{
  CEResult_t rv = CE_RESULT_GENERAL_FAILED;

  CEString packed_string = NULL;
  CEString service_name = NULL;
  CEString fmt = NULL;

  /* Packing input parameters */
  packer in_pack;

  /* Set RPC parameters */
  in_pack.set_service("NL_SE_CLIENT");
  in_pack.set_method("SE_DrmGetPaths");

  in_pack.pack(&in_fast_write);
  
  packer out_packer;
  /* Allocate packed string */
  size_t temp_size = in_pack.size_coded() * sizeof(wchar_t);
  wchar_t* temp = (wchar_t*)malloc( temp_size );
  if( temp == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto complete;
  }

  /* Write out coded string for invokation */
  swprintf_s(temp,temp_size/sizeof(wchar_t),L"%hs",in_pack.get_coded_string().c_str());
  packed_string = cesdk->fns.CEM_AllocateString(temp);
  if( packed_string == NULL )
    goto complete;
  
  void** response = NULL;

  service_name = cesdk->fns.CEM_AllocateString(L"NL_SE_CLIENT");
  if( service_name == NULL )
    goto complete;

  fmt = cesdk->fns.CEM_AllocateString(L"s");
  if( fmt == NULL )
    goto complete;

  /* Allocate request object */
  void* request[2] = {0};
  *(request + 0) = (void*)packed_string;
  *(request + 1) = NULL;

  /* Call into service as a transport */
  rv = cesdk->fns.ServiceInvoke(h,service_name,fmt,request,&response,in_timeout);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }
  
  cesdk->fns.CEM_FreeString(packed_string);
  
  /* response[0] holds the format */
  CEString response_string = (CEString)response[1];
  assert( response_string != NULL );
  
  /* Unpack and decode response */
  size_t out_param_size = 0;
  const void* out_param_ptr = NULL;
  UNREFERENCED_PARAMETER(out_param_size);
  UNREFERENCED_PARAMETER(out_param_ptr);

  out_packer.assign_code_string(cesdk->fns.CEM_GetString(response_string));

  cesdk->fns.ServiceResponseFree(response);

  /* Unpack 'out_paths' parameter 0 */
  out_param_ptr = out_packer.at(0,&out_param_size);
  assert( out_param_ptr != NULL );

  *out_paths = (wchar_t*)malloc(out_param_size);
  if( *out_paths == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto complete;
  }
  memcpy(*out_paths,out_param_ptr,out_param_size);

  /* Unpack 'out_paths_size' parameter 1 */
  out_param_ptr = out_packer.at(1,&out_param_size);
  assert( out_param_ptr != NULL );

  memcpy(out_paths_size,out_param_ptr,out_param_size);

  rv = out_packer.get_return();

  complete:

  if( service_name != NULL )
    cesdk->fns.CEM_FreeString(service_name);

  if( fmt != NULL )
    cesdk->fns.CEM_FreeString(fmt);

  if( temp != NULL )
    free(temp);

  return rv;
} /* SE_DrmGetPaths */


extern "C"
CEResult_t SE_DrmAddPath( CEHandle h ,  const wchar_t* in_path  ,  BOOL in_fast_write  , int in_timeout )
{
  CEResult_t rv = CE_RESULT_GENERAL_FAILED;

  CEString packed_string = NULL;
  CEString service_name = NULL;
  CEString fmt = NULL;

  /* Packing input parameters */
  packer in_pack;

  /* Set RPC parameters */
  in_pack.set_service("NL_SE_CLIENT");
  in_pack.set_method("SE_DrmAddPath");

  in_pack.pack(in_path);
  in_pack.pack(&in_fast_write);
  
  packer out_packer;
  /* Allocate packed string */
  size_t temp_size = in_pack.size_coded() * sizeof(wchar_t);
  wchar_t* temp = (wchar_t*)malloc( temp_size );
  if( temp == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto complete;
  }

  /* Write out coded string for invokation */
  swprintf_s(temp,temp_size/sizeof(wchar_t),L"%hs",in_pack.get_coded_string().c_str());
  packed_string = cesdk->fns.CEM_AllocateString(temp);
  if( packed_string == NULL )
    goto complete;
  
  void** response = NULL;

  service_name = cesdk->fns.CEM_AllocateString(L"NL_SE_CLIENT");
  if( service_name == NULL )
    goto complete;

  fmt = cesdk->fns.CEM_AllocateString(L"s");
  if( fmt == NULL )
    goto complete;

  /* Allocate request object */
  void* request[2] = {0};
  *(request + 0) = (void*)packed_string;
  *(request + 1) = NULL;

  /* Call into service as a transport */
  rv = cesdk->fns.ServiceInvoke(h,service_name,fmt,request,&response,in_timeout);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }
  
  cesdk->fns.CEM_FreeString(packed_string);
  
  /* response[0] holds the format */
  CEString response_string = (CEString)response[1];
  assert( response_string != NULL );
  
  /* Unpack and decode response */
  size_t out_param_size = 0;
  const void* out_param_ptr = NULL;
  UNREFERENCED_PARAMETER(out_param_size);
  UNREFERENCED_PARAMETER(out_param_ptr);

  out_packer.assign_code_string(cesdk->fns.CEM_GetString(response_string));

  cesdk->fns.ServiceResponseFree(response);

  rv = out_packer.get_return();

  complete:

  if( service_name != NULL )
    cesdk->fns.CEM_FreeString(service_name);

  if( fmt != NULL )
    cesdk->fns.CEM_FreeString(fmt);

  if( temp != NULL )
    free(temp);

  return rv;
} /* SE_DrmAddPath */


extern "C"
CEResult_t SE_DrmRemovePath( CEHandle h ,  const wchar_t* in_path  ,  BOOL in_fast_write  , int in_timeout )
{
  CEResult_t rv = CE_RESULT_GENERAL_FAILED;

  CEString packed_string = NULL;
  CEString service_name = NULL;
  CEString fmt = NULL;

  /* Packing input parameters */
  packer in_pack;

  /* Set RPC parameters */
  in_pack.set_service("NL_SE_CLIENT");
  in_pack.set_method("SE_DrmRemovePath");

  in_pack.pack(in_path);
  in_pack.pack(&in_fast_write);
  
  packer out_packer;
  /* Allocate packed string */
  size_t temp_size = in_pack.size_coded() * sizeof(wchar_t);
  wchar_t* temp = (wchar_t*)malloc( temp_size );
  if( temp == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto complete;
  }

  /* Write out coded string for invokation */
  swprintf_s(temp,temp_size/sizeof(wchar_t),L"%hs",in_pack.get_coded_string().c_str());
  packed_string = cesdk->fns.CEM_AllocateString(temp);
  if( packed_string == NULL )
    goto complete;
  
  void** response = NULL;

  service_name = cesdk->fns.CEM_AllocateString(L"NL_SE_CLIENT");
  if( service_name == NULL )
    goto complete;

  fmt = cesdk->fns.CEM_AllocateString(L"s");
  if( fmt == NULL )
    goto complete;

  /* Allocate request object */
  void* request[2] = {0};
  *(request + 0) = (void*)packed_string;
  *(request + 1) = NULL;

  /* Call into service as a transport */
  rv = cesdk->fns.ServiceInvoke(h,service_name,fmt,request,&response,in_timeout);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }
  
  cesdk->fns.CEM_FreeString(packed_string);
  
  /* response[0] holds the format */
  CEString response_string = (CEString)response[1];
  assert( response_string != NULL );
  
  /* Unpack and decode response */
  size_t out_param_size = 0;
  const void* out_param_ptr = NULL;
  UNREFERENCED_PARAMETER(out_param_size);
  UNREFERENCED_PARAMETER(out_param_ptr);

  out_packer.assign_code_string(cesdk->fns.CEM_GetString(response_string));

  cesdk->fns.ServiceResponseFree(response);

  rv = out_packer.get_return();

  complete:

  if( service_name != NULL )
    cesdk->fns.CEM_FreeString(service_name);

  if( fmt != NULL )
    cesdk->fns.CEM_FreeString(fmt);

  if( temp != NULL )
    free(temp);

  return rv;
} /* SE_DrmRemovePath */

