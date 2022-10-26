
#include <windows.h>
#include <cassert>
#include <winioctl.h>

#include "nlcc.h"
#include "nlcc_ulib.h"
#include "nlcc_ioctl.h"

#define NLCC_IS_DOUBLE_TERM(p) ( *((p) + 0) == (wchar_t)NULL && *((p) + 1) == (wchar_t)NULL )

_Check_return_
int NLCC_UOpen( _Out_ PNLCC_HANDLE in_handle )
{
  assert( in_handle != NULL );
  in_handle->h = CreateFileW(NLCC_QUERY_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,
			     OPEN_EXISTING,
			     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			     NULL);
  if( in_handle->h == INVALID_HANDLE_VALUE )
  {
    return -1;
  }
  return 0;
}/* NLCC_UOpen */

int NLCC_UClose( _In_ PNLCC_HANDLE in_handle )
{
  assert( in_handle != NULL );
  CloseHandle(in_handle->h);
  return 0;
}/* NLCC_UClose */

int NLCC_UQuery( _In_ PNLCC_HANDLE in_handle ,
		 _In_ PNLCC_QUERY in_request ,
		 _Out_ PNLCC_QUERY out_response ,
		 _In_ size_t in_timeout )
{
  assert( in_handle != NULL );
  assert( in_request != NULL );
  assert( out_response != NULL );

  if( in_handle == NULL || in_request == NULL || out_response == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  DWORD bytes_out = 0;
  BOOL rv = FALSE;
  OVERLAPPED io_result;
  DWORD last_error = ERROR_SUCCESS;

  /* If a timeout was specified set the overlapped pointer to a concrete
   * object for use in DeviceIoControl and GetOverlappedResult.
   */
  memset(&io_result,0x00,sizeof(io_result));
  io_result.hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
  if( io_result.hEvent == NULL )
  {
    return -1;
  }

  rv = DeviceIoControl(in_handle->h,IOCTL_POLICY_QUERY,
		       in_request,sizeof(*in_request),      /* input */
		       out_response,sizeof(*out_response),  /* output */
		       &bytes_out,&io_result);
  DWORD le = GetLastError();
  if( rv == FALSE && le != ERROR_IO_PENDING )
  {
    goto NLCC_UQuery_complete;
  }

  /* Pending I/O will return false and use ERROR_IO_PENDING.  Retrieve the result.
   */
  if( le == ERROR_IO_PENDING )
  {
    /* Wait for the I/O to complete OR the timeout to expire */
    if( in_timeout > 0 )
    {
      DWORD wait_result;
      wait_result = WaitForSingleObject(io_result.hEvent,(DWORD)in_timeout);
      if( wait_result != WAIT_OBJECT_0 )
      {
	CancelIo(in_handle->h);
	if( wait_result == WAIT_TIMEOUT )
	{
	  last_error = WAIT_TIMEOUT;
	}
	rv = FALSE;
	goto NLCC_UQuery_complete;
      }
    }
    rv = GetOverlappedResult(in_handle->h,&io_result,&bytes_out,TRUE);
    if( rv == FALSE )
    {
      /* Propagate the error back to the caller  */
      last_error = (DWORD)io_result.Internal;
    }
  }/* if pending io */
  
 NLCC_UQuery_complete:

  if( io_result.hEvent != NULL )
  {
    CloseHandle(io_result.hEvent);
  }

  if( last_error != ERROR_SUCCESS )
  {
    SetLastError(last_error);
  }

  if( !rv )
  {
    return -1;
  }
  return 0;
}/* NLCC_UQuery */

void NLCC_UInitializeQuery( _Out_ PNLCC_QUERY in_query )
{
  assert( in_query != NULL );

  /* size of self */
  in_query->size = sizeof(NLCC_QUERY);

  /* transaction ID from 0 */
  in_query->tx_id = 0;

  /* double terminate attributes */
  in_query->attributes[0] = (wchar_t)NULL;
  in_query->attributes[1] = (wchar_t)NULL;

}/* NLCC_UInitializeQuery */

/* Return a pointer to the next NULL termination */
static const wchar_t* get_next_term( _In_ const wchar_t* in_string )
{
  const wchar_t* p = in_string;
  while( *p != (wchar_t)NULL )
  {
    p++;
  }
  return p;
}/* get_next_term */

_Check_return_
int NLCC_UAddAttribute( _In_ PNLCC_QUERY in_query ,
			_In_ const wchar_t* in_key ,
			_In_ const wchar_t* in_value )
{
  assert( in_query != NULL );
  assert( in_key != NULL );
  assert( in_value != NULL );

  if( in_query == NULL || in_key == NULL || in_value == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  /* Size (in bytes) of the input parameters excluding termination */
  size_t in_key_size = wcslen(in_key) * sizeof(wchar_t);
  size_t in_value_size = wcslen(in_value) * sizeof(wchar_t);

  if( in_key_size <= 0 || in_value_size <= 0 )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  wchar_t* p = in_query->attributes;
  /* Find last double term */
  for( ; ; )
  {
    if( NLCC_IS_DOUBLE_TERM(p) )
    {
      break;
    }
    p++;
  }

  /* If this is the first attribute is should be set a the start of the attributes
   * buffer, otherwise the double term has been detected and this is *not* the start
   * of that buffer.  Moving to the next location is required.
   */
  if( p != in_query->attributes )
  {
    p++;
  }

  /* Verify attribute will fit.  Determine bytes remaining and required. */
  size_t bytes_left, bytes_needed;
  bytes_left   = sizeof(in_query->attributes) - (p - in_query->attributes) * sizeof(wchar_t);
  bytes_needed =
    (in_key_size + sizeof(wchar_t)) +     /* key + termination (bytes) */
    (in_value_size + sizeof(wchar_t)) +   /* value + termination (bytes) */
    sizeof(wchar_t);                      /* secondary termination (double-term) (bytes) */

  /* Is there sufficient storage to pack the attribute? */
  if( bytes_left <= bytes_needed )
  {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return -1;
  }

  /* Copy key to to buffer and make use of the term included with the string */
  memcpy(p,in_key,in_key_size + sizeof(wchar_t));
  p += (in_key_size / sizeof(wchar_t)) + 1;

  /* Copy value to to buffer and make use of the term included with the string.  The memcpy will
   * handle the first term.  The second term for double-term of the attribute structure is added
   * afterward.
   */
  memcpy(p,in_value,in_value_size + sizeof(wchar_t));
  p += (in_value_size / sizeof(wchar_t)) + 1;
  *p = (wchar_t)NULL;

  return 0;
}/* NLCC_UAddAttribute */

_Check_return_
int NLCC_UGetAttributeByIndex( _In_ const PNLCC_QUERY in_query ,
			       _In_ size_t in_index ,
			       _Inout_ const wchar_t** out_key ,
			       _Inout_ const wchar_t** out_value )
{
  assert( in_query != NULL );
  assert( out_key != NULL );
  assert( out_value != NULL );

  if( in_query == NULL || out_key == NULL || out_value == NULL )
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return -1;
  }

  int rv = -1;
  const wchar_t* p = in_query->attributes;
  for( size_t curr_index = 0 ; ; curr_index++ )
  {
    /* The current position, p, must point to a non-single and non-double terminated
     * string.  The current position must always point to the start of a key in the
     * attributes array.
     */
    if( *p == NULL || NLCC_IS_DOUBLE_TERM(p) )
    {
      SetLastError(ERROR_NOT_FOUND);
      break;
    }

    const wchar_t* key = NULL;
    const wchar_t* value = NULL;

    key = p;                    /* By definition this is the key */
    value = get_next_term(p);   /* Value is after next term */
    value++;

    /* If this is the index we're looking for we're done */
    if( curr_index == in_index )
    {
      rv = 0;
      *out_key = key;      /* assign pointers to caller */
      *out_value = value;
      break;
    }

    p = value + wcslen(value) + 1;
  }/* for */

  return rv;
}/* NLCC_UGetAttributeByIndex */
