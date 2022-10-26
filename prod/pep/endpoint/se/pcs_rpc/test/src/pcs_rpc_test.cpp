
#include <windows.h>
#include <cstdio>
#include <cstdlib>

#include "eframework/platform/cesdk.hpp"

#include "CEsdk.h"
#include "fake_ceservice.h"

#include "test_client.hpp"

nextlabs::cesdk_loader cesdk;
nextlabs::cesdk_connection cesdk_conn;

static int perform_tests(void)
{
  CEResult_t rv = CE_RESULT_SUCCESS;
  char* in_string = "Hello world!";
  char* out_string = NULL;
  int out_string_size = 0;
  rv = TEST_StringCopy(cesdk_conn.get_connection_handle(),&out_string,in_string,&out_string_size,5000);
  if( rv == CE_RESULT_SUCCESS )
  {
    fprintf(stdout, "result = %d : input(%s) : output(%s)\n", rv, in_string, out_string);
    free(out_string);
  }

  /* This must fail! */
  rv = TEST_StringCopyFail(cesdk_conn.get_connection_handle(),&out_string,in_string,&out_string_size,5000);

  const char* string_table[] = 
    {
      "This is a message",
      "This is a longer message",
      "This is a longer message than the longer message",
      "a",
      "ab",
      "abc",
      "abcd",
      "1234567890"
    };

  for( size_t i = 0 ; i < _countof(string_table) ; i++ )
  {
    rv = TEST_StringLength(cesdk_conn.get_connection_handle(),(char*)string_table[i],&out_string_size,5000);
    if( rv != CE_RESULT_SUCCESS )
    {
      fprintf(stdout, "TEST_StringLength failed for %s (%d)\n", string_table[i], rv);
      continue;
    }
    fprintf(stdout, "%d ?= %d : %s\n", out_string_size, strlen(string_table[i]), string_table[i]);
    if( out_string_size != (int)strlen(string_table[i]) )
    {
      fprintf(stdout, "TEST_StringLength failed: %d != %d\n", out_string_size, strlen(string_table[i]));
      return 1;
    }
  }

  const size_t max_count = 100;
  for( size_t i = 0 ; i < max_count ; i++ )
  {
    int result = 0;
    int x = i;
    int y = max_count - i;

    rv = TEST_Add(cesdk_conn.get_connection_handle(),&x,&y,&result,5000);
    if( rv != CE_RESULT_SUCCESS )
    {
      fprintf(stdout, "TEST_Add failed for %d + %d (%d)\n", x, y, rv);
      continue;
    }

    //assert( x + y == result );
    if( x + y != result )
    {
      fprintf(stdout, "TEST_Add failed: %d + %d != %d\n", x, y, result);
      return 1;
    }
  }

  /* Verify return values for all CEResult_t values */
  for( int i = (int)CE_RESULT_SUCCESS ; i != (int)CE_RESULT_APPLICATION_AUTH_FAILED ; i-- )
  {
    CEResult_t result = (CEResult_t)i;
    rv = TEST_ReturnError(cesdk_conn.get_connection_handle(),&result,5000);
    if( rv != result )
    {
      fprintf(stdout, "TEST_ReturnError failed: %d != %d\n", rv, result);
      return 1;
    }
  }

  int x = 0, y = 1;

  x = 0; y = 1;
  rv = TEST_Assert(cesdk_conn.get_connection_handle(),&x,&y,5000);
  fprintf(stdout, "TEST_Assert( %d == %d ) (%d)\n", x, y, rv);

  x = 2; y = 2;
  rv = TEST_Assert(cesdk_conn.get_connection_handle(),&x,&y,5000);
  fprintf(stdout, "TEST_Assert( %d == %d ) (%d)\n", x, y, rv);

  return 0;
}/* perform_tests */

int main(void)
{
  bool status;

  status = cesdk.load(L"C:\\Program Files\\Nextlabs\\Policy Controller\\bin");
  if( status == false )
  {
    fprintf(stderr, "pcs_idl_test: failed to load CE SDK\n");
    return 1;
  }
  cesdk_conn.set_sdk(&cesdk);
  status = cesdk_conn.connect();
  if( status == false )
  {
    fprintf(stderr, "pcs_idl_test: failed to connect\n");
    //return 1;
  }

  pcs_rpc_set_sdk(&cesdk);

  int rv = perform_tests();

  return rv;
}/* main */
