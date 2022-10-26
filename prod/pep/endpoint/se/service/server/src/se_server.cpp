
#include <windows.h>
#include "CEsdk.h"
#include "pcs_rpc.hpp"
#include "pcs_rpc_packer.hpp"

extern "C"
CEResult_t server_SE_DrmGetPaths( wchar_t** out_paths  ,  int* out_paths_size  ,  BOOL in_fast_write );


extern "C"
CEResult_t server_SE_DrmAddPath( const wchar_t* in_path  ,  BOOL in_fast_write );


extern "C"
CEResult_t server_SE_DrmRemovePath( const wchar_t* in_path  ,  BOOL in_fast_write );




#include <cstdio>
#include <cstdlib>
#include <windows.h>

#include "CEsdk.h"
#include "ceservice.h"


extern "C" void dispatch_SE_DrmGetPaths( const char* in_string , char** out_string )
{
  wchar_t* param_out_paths;
  int param_out_paths_size;
  BOOL param_in_fast_write;

  CEResult_t rv;
  packer in_packer;
  packer out_packer;
  void* in_param_ptr = NULL;
  void* out_param_ptr = NULL;
  size_t in_param_size = 0;
  size_t out_param_size = 0;

  UNREFERENCED_PARAMETER(in_param_ptr);
  UNREFERENCED_PARAMETER(out_param_ptr);
  UNREFERENCED_PARAMETER(in_param_size);
  UNREFERENCED_PARAMETER(out_param_size);
  
  in_packer.assign_code_string(in_string);

  /* Unpack the input paramters and setup for call */

  /* Input parameter 0 is param_in_fast_write */
  in_param_ptr = in_packer.at(0,&in_param_size);
  assert( in_param_ptr != NULL );
  memcpy(&param_in_fast_write,in_param_ptr,in_param_size);


  /* Call server-side implementation */
  rv = server_SE_DrmGetPaths(&param_out_paths,&param_out_paths_size,param_in_fast_write);

  /* Free allocated input parameters */

  out_packer.set_return(rv);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }
  out_packer.pack_raw(param_out_paths,param_out_paths_size);
  free(param_out_paths);

  out_packer.pack(&param_out_paths_size);


  complete:
  size_t rpc_out_string_size = out_packer.get_coded_string().length() + 1;
  *out_string = (char*)malloc( rpc_out_string_size );
  if( *out_string == NULL )
  {
    return;
  }
  strncpy_s(*out_string,rpc_out_string_size,out_packer.get_coded_string().c_str(),_TRUNCATE);
} /* dispatch_SE_DrmGetPaths */


extern "C" void dispatch_SE_DrmAddPath( const char* in_string , char** out_string )
{
  wchar_t* param_in_path;
  BOOL param_in_fast_write;

  CEResult_t rv;
  packer in_packer;
  packer out_packer;
  void* in_param_ptr = NULL;
  void* out_param_ptr = NULL;
  size_t in_param_size = 0;
  size_t out_param_size = 0;

  UNREFERENCED_PARAMETER(in_param_ptr);
  UNREFERENCED_PARAMETER(out_param_ptr);
  UNREFERENCED_PARAMETER(in_param_size);
  UNREFERENCED_PARAMETER(out_param_size);
  
  in_packer.assign_code_string(in_string);

  /* Unpack the input paramters and setup for call */

  /* Input parameter 0 is param_in_path */
  in_param_ptr = in_packer.at(0,&in_param_size);
  assert( in_param_ptr != NULL );
  param_in_path = (wchar_t*)malloc( in_param_size );
  memcpy(param_in_path,in_param_ptr,in_param_size);

  /* Input parameter 1 is param_in_fast_write */
  in_param_ptr = in_packer.at(1,&in_param_size);
  assert( in_param_ptr != NULL );
  memcpy(&param_in_fast_write,in_param_ptr,in_param_size);


  /* Call server-side implementation */
  rv = server_SE_DrmAddPath(param_in_path,param_in_fast_write);

  /* Free allocated input parameters */
  /* Free input parameter param_in_path */
  free(param_in_path);

  out_packer.set_return(rv);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }


  complete:
  size_t rpc_out_string_size = out_packer.get_coded_string().length() + 1;
  *out_string = (char*)malloc( rpc_out_string_size );
  if( *out_string == NULL )
  {
    return;
  }
  strncpy_s(*out_string,rpc_out_string_size,out_packer.get_coded_string().c_str(),_TRUNCATE);
} /* dispatch_SE_DrmAddPath */


extern "C" void dispatch_SE_DrmRemovePath( const char* in_string , char** out_string )
{
  wchar_t* param_in_path;
  BOOL param_in_fast_write;

  CEResult_t rv;
  packer in_packer;
  packer out_packer;
  void* in_param_ptr = NULL;
  void* out_param_ptr = NULL;
  size_t in_param_size = 0;
  size_t out_param_size = 0;

  UNREFERENCED_PARAMETER(in_param_ptr);
  UNREFERENCED_PARAMETER(out_param_ptr);
  UNREFERENCED_PARAMETER(in_param_size);
  UNREFERENCED_PARAMETER(out_param_size);
  
  in_packer.assign_code_string(in_string);

  /* Unpack the input paramters and setup for call */

  /* Input parameter 0 is param_in_path */
  in_param_ptr = in_packer.at(0,&in_param_size);
  assert( in_param_ptr != NULL );
  param_in_path = (wchar_t*)malloc( in_param_size );
  memcpy(param_in_path,in_param_ptr,in_param_size);

  /* Input parameter 1 is param_in_fast_write */
  in_param_ptr = in_packer.at(1,&in_param_size);
  assert( in_param_ptr != NULL );
  memcpy(&param_in_fast_write,in_param_ptr,in_param_size);


  /* Call server-side implementation */
  rv = server_SE_DrmRemovePath(param_in_path,param_in_fast_write);

  /* Free allocated input parameters */
  /* Free input parameter param_in_path */
  free(param_in_path);

  out_packer.set_return(rv);
  if( rv != CE_RESULT_SUCCESS )
  {
    goto complete;
  }


  complete:
  size_t rpc_out_string_size = out_packer.get_coded_string().length() + 1;
  *out_string = (char*)malloc( rpc_out_string_size );
  if( *out_string == NULL )
  {
    return;
  }
  strncpy_s(*out_string,rpc_out_string_size,out_packer.get_coded_string().c_str(),_TRUNCATE);
} /* dispatch_SE_DrmRemovePath */



 /* Dispatch to the proper method based on the request */
extern "C" void server_dispatch( const char* in_string , char** out_string )
{

  char method[64] = {0};

  if( pcs_rpc_request::get_method(in_string,method,_countof(method)) == false )
    return;

  if( strcmp(method,"SE_DrmGetPaths") == 0 )
    dispatch_SE_DrmGetPaths(in_string,out_string);

  if( strcmp(method,"SE_DrmAddPath") == 0 )
    dispatch_SE_DrmAddPath(in_string,out_string);

  if( strcmp(method,"SE_DrmRemovePath") == 0 )
    dispatch_SE_DrmRemovePath(in_string,out_string);

  /* Invalid method */
}
