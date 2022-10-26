
#include <cassert>
#include "eframework/platform/cesdk.hpp"
#include "fake_ceservice.h"

extern "C" void server_dispatch( char* , char** );

extern nextlabs::cesdk_loader cesdk;

/** FakeServiceInvoke
 *
 *  \brief Simulate ServiceInvoke@ceservice.dll  This will take only parameter "s" which
 *         is a packed set of parameters.
 *         See ceservice.h for details.
 */
extern "C"
CEResult_t FakeServiceInvoke(CEHandle h, CEString serviceName, CEString fmt, void **request, void ***response, CEint32 timeout)
{
  assert( serviceName != NULL );
  assert( request != NULL );
  assert( response != NULL );

  if( serviceName == NULL || request == NULL || response == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  CEResult_t rv = CE_RESULT_SUCCESS;
  CEString packed_string = (CEString)*(request + 0);
  wchar_t* string = (wchar_t*)cesdk.fns.CEM_GetString(packed_string);

  assert( string != NULL );
  if( string == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  char* temp_in  = (char*)malloc( wcslen(string) + 1 );
  if( temp_in == NULL )
  {
    return CE_RESULT_GENERAL_FAILED;
  }
  char* temp_out = NULL;

  _snprintf_s(temp_in,wcslen(string)+1, _TRUNCATE,"%ws",string);
  server_dispatch(temp_in,&temp_out);

  assert( temp_out != NULL );
  if( temp_out == NULL )
  {
    return CE_RESULT_GENERAL_FAILED;
  }

  size_t temp_outw_size = strlen(temp_out) * sizeof(wchar_t) + sizeof(wchar_t);
  wchar_t* temp_outw = (wchar_t*)malloc( temp_outw_size );
  if( temp_outw == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto FakeServiceInvoke_complete;
  }
  _snwprintf_s(temp_outw,temp_outw_size, _TRUNCATE,L"%hs",temp_out);

  CEString out_string = cesdk.fns.CEM_AllocateString(temp_outw);
  *response = (void**)malloc( sizeof(void*) * 2 );
  if( *response == NULL )
  {
    rv = CE_RESULT_GENERAL_FAILED;
    goto FakeServiceInvoke_complete;
  }

  *(*response + 0) = (void*)cesdk.fns.CEM_AllocateString(L"s");  /* param 0 - format */
  *(*response + 1) = (void*)out_string;                /* param 1 - payload */

 FakeServiceInvoke_complete:

  if( temp_in )
    free(temp_in);

  if( temp_out )
    free(temp_out);

  if( temp_outw )
    free(temp_outw);

  return rv;
}/* FakeServiceInvoke */

extern "C"
CEResult_t FakeServiceResponseFree(void **response)
{
  CEString out_param  = (CEString)response[0];
  CEString out_string = (CEString)response[1];

  assert( out_param != NULL );
  assert( out_string != NULL );

  cesdk.fns.CEM_FreeString(out_param);
  cesdk.fns.CEM_FreeString(out_string);

  free(response);
  return CE_RESULT_SUCCESS;
}/* FakeServiceResponseFree */
