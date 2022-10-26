
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cassert> 

#include "CEsdk.h"

extern "C"
CEResult_t server_TEST_StringCopy( char** my_out_string  , char* my_in_string  , int* my_out_string_size )
{
  assert( my_out_string != NULL );
  assert( my_in_string != NULL );
  assert( my_out_string_size != NULL );

  if( my_out_string == NULL || my_in_string == NULL || my_out_string_size == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  size_t count = strlen(my_in_string) + 1;

  *my_out_string = (char*)malloc( count * sizeof(char) );
  if( *my_out_string == NULL )
  {
    return CE_RESULT_INSUFFICIENT_BUFFER;
  }

  strncpy_s(*my_out_string,count,my_in_string,_TRUNCATE);

  *my_out_string_size = (int)(count * sizeof(char));

  return CE_RESULT_SUCCESS;
}/* server_TEST_StringCopy */

extern "C"
CEResult_t server_TEST_StringCopyFail( char** my_out_string  , char* my_in_string  , int* my_out_string_size )
{
  return CE_RESULT_GENERAL_FAILED;
}/* server_TEST_StringCopyFail */

extern "C"
CEResult_t server_TEST_StringLength( char* my_in_string  , int* my_out_string_size )
{
  assert( my_in_string != NULL );

  if( my_in_string == NULL || my_out_string_size == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  *my_out_string_size = strlen(my_in_string);

  return CE_RESULT_SUCCESS;
}/* CEResult_t server_TEST_StringLength */

extern "C"
CEResult_t server_TEST_Add( int* x , int* y , int* result )
{
  assert( x != NULL );
  assert( y != NULL );
  assert( result != NULL );

  if( x == NULL || y == NULL || result == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  *result = *x + *y;

  return CE_RESULT_SUCCESS;
}/* CEResult_t server_TEST_Add */

extern "C"
CEResult_t server_TEST_ReturnError( CEResult_t* in_result )
{
  return *in_result;
}/* server_TEST_ReturnError */

extern "C"
CEResult_t server_TEST_Assert( int* x , int* y , int* result )
{
  assert( x != NULL );
  assert( y != NULL );

  if( x == NULL || y == NULL )
  {
    return CE_RESULT_INVALID_PARAMS;
  }

  if( *x == *y )
  {
    return CE_RESULT_SUCCESS;
  }
  return CE_RESULT_GENERAL_FAILED;
}/* CEResult_t server_TEST_StringLength */
