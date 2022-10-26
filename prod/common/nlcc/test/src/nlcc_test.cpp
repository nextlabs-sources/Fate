
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <conio.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <winioctl.h>

#include "nlcc.h"
#include "nlcc_ioctl.h"
#include "nlcc_ulib.h"

#include <eframework/timer/timer_high_resolution.hpp>

bool op_pdp     = false;
bool op_pep     = false;
bool op_verbose = false;
bool op_verify_attributes = false;

HANDLE cancel_event;

void verify_attributes(void)
{
  PNLCC_QUERY q = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );
  if( q == NULL )
  {
    fprintf(stderr, "verify_attributes: malloc failed\n");
    exit(1);
  }

  NLCC_UInitializeQuery(q);

  fprintf(stdout, "Testing NLCC_UAddAttribute bad parameters...");
  int result;
  result = NLCC_UAddAttribute(q,L"",L"value");
  assert( result != 0 );
  assert( GetLastError() == ERROR_INVALID_PARAMETER );

  result = NLCC_UAddAttribute(q,L"key",L"");
  assert( result != 0 );
  assert( GetLastError() == ERROR_INVALID_PARAMETER );

  fprintf(stdout, "ok.\n");

  fprintf(stdout, "Testing attributes packing...");
  for( size_t i = 0 ; i < 32 ; i++ )
  {
    wchar_t key[512] = {0};
    wchar_t value[512] = {0};

    _snwprintf_s(key,  _countof(key), _TRUNCATE,L"key%d",i);
    _snwprintf_s(value,_countof(key), _TRUNCATE,L"value%d",i);
    if( NLCC_UAddAttribute(q,key,value) < 0 )
    {
      fprintf(stdout, "out of space!\n");
      break;
    }
  }
  fprintf(stdout, "ok.\n");

  fprintf(stdout, "Verifying packed parameters...");
  for( size_t i = 0 ; ; i++ )
  {
    wchar_t temp[512] = {0};
    const wchar_t* key = NULL;
    const wchar_t* value = NULL;
    if( NLCC_UGetAttributeByIndex(q,i,&key,&value) != 0 )
    {
      break;
    }

    assert( key != NULL );
    assert( value != NULL );

    _snwprintf_s(temp,_countof(temp), _TRUNCATE,L"key%d",i);
    if( wcscmp(temp,key) != 0 )
    {
      fprintf(stderr, "failed.  Attribute %d.\n", i);
      exit(1);
    }

    _snwprintf_s(temp,_countof(temp), _TRUNCATE,L"value%d",i);
    if( wcscmp(temp,value) != 0 )
    {
      fprintf(stderr, "failed.  Attribute %d.\n", i);
      exit(1);
    }

    if( op_verbose )
      fprintf(stdout, "%02d: %ws = %ws\n", i, key, value);
  }/* for */
  fprintf(stdout, "ok.\n");

  fprintf(stdout, "Testing attributes overrrun...");
  NLCC_UInitializeQuery(q);
  for( size_t i = 0 ; ; i++ )
  {
    wchar_t key[512] = {0};
    wchar_t value[512] = {0};

    _snwprintf_s(key,  _countof(key), _TRUNCATE,L"key%d",i);
    _snwprintf_s(value,_countof(key), _TRUNCATE,L"value%d",i);
    if( NLCC_UAddAttribute(q,key,value) != 0 )
    {
      DWORD le = GetLastError();
      assert( le == ERROR_NOT_ENOUGH_MEMORY );
      if( le != ERROR_NOT_ENOUGH_MEMORY )
      {
	fprintf(stderr, "NLCC_UAddAttribute did not set last error to ERROR_NOT_ENOUGH_MEMORY.\n");
	exit(1);
      }
      fprintf(stdout, "ok.\n");
      break;
    }
  }/* for */

  free(q);

}/* verify_attributes */
#pragma warning(push)
#pragma warning(disable: 6262 6031)
void __cdecl client( void* arg )
{
  NLCC_HANDLE nlcc_handle;

  int rv = NLCC_UOpen(&nlcc_handle);
  if( rv != 0 )
  {
    fprintf(stderr, "client: NLCC_UOpen failed (le %d)\n", GetLastError());
    exit(1);
  }

  for( ; ; )
  {
    if( WaitForSingleObject(cancel_event,0) == WAIT_OBJECT_0 )
    {
      fprintf(stdout, "client: cancelled\n");
      break;
    }

    NLCC_QUERY request;
    NLCC_QUERY response;

    NLCC_UInitializeQuery(&request);
    NLCC_UInitializeQuery(&response);

    NLCC_UAddAttribute(&request,L"action",L"OPEN");
    NLCC_UAddAttribute(&request,L"source_name",L"C:\\ABCDEFG.TXT");

    NLCC_UAddAttribute(&request,L"source_type",L"fso");
    NLCC_UAddAttribute(&request,L"source_attr_foo_zbc"      ,L"fso---------------------01");
    NLCC_UAddAttribute(&request,L"source_attr_bar_xyz",     L"fso-----------------------02");
    NLCC_UAddAttribute(&request,L"source_attr_fooabcdefghij",L"fso-------------------------03");
    NLCC_UAddAttribute(&request,L"source_attr_bar0123456789",L"fso---------------------------04");

    NLCC_UAddAttribute(&request,L"application",L"c:\\1234567890.EXE");
    NLCC_UAddAttribute(&request,L"user_id",L"1-348583458");
    NLCC_UAddAttribute(&request,L"email",L"dave.mustaine@bnrmetal.com");
    NLCC_UAddAttribute(&request,L"host",L"malta");
    request.info.request.ip = 0;
    request.info.request.pid = 0;
    request.info.request.event_level = 0;

    nextlabs::high_resolution_timer ht;
    ht.start();
    rv = NLCC_UQuery(&nlcc_handle,&request,&response,0);
    ht.stop();
    if( rv < 0 )
    {
      fprintf(stderr, "NLCC_Query failed (le %d)\n", GetLastError());
      break;
    }

    fprintf(stdout, "NLCC_Query: response (%s) : %fms\n",
	    response.info.response.allow ? "allow" : "deny",
	    ht.diff());
  }

  _endthread();
}/* client */
#pragma warning(pop)
HANDLE h = NULL;

void __cdecl pdp_worker( void* arg )
{
  DWORD bytes_out = 0;
  BOOL rv;
  PNLCC_QUERY request = NULL;
  PNLCC_QUERY response = NULL;

  request = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );
  response = (PNLCC_QUERY)malloc( sizeof(NLCC_QUERY) );

  if( request == NULL || response == NULL )
  {
    fprintf(stderr, "pdp_worker: out of memory\n");
    exit(1);
  }

  HANDLE io_event = CreateEventW(NULL,TRUE,FALSE,NULL);
  if( io_event == NULL )
  {
    fprintf(stderr, "CreateEventW failed (le %d)\n", GetLastError());
    exit(1);
  }
  for( ; ; )
  {
    if( WaitForSingleObject(cancel_event,0) == WAIT_OBJECT_0 )
    {
      fprintf(stdout, "server: cancelled\n");
      break;
    }

    OVERLAPPED ol;

    /* Read request */
    NLCC_UInitializeQuery(request);

    memset(&ol,0x00,sizeof(ol));
    ResetEvent(io_event);
    ol.hEvent = io_event;
    rv = DeviceIoControl(h,IOCTL_POLICY_REQUEST,NULL,0,request,sizeof(*request),&bytes_out,&ol);
    if( rv == FALSE )
    {
      if( GetLastError() == ERROR_IO_PENDING )
      {
	rv = GetOverlappedResult(h,&ol,&bytes_out,TRUE);
      }
    }
    if( rv == FALSE )
    {
      fprintf(stderr, "DeviceIoControl failed (le %d)\n", GetLastError());
      continue;
    }

    if( bytes_out != sizeof(*request) )
    {
      fprintf(stderr, "IOCTL_POLICY_REQUEST failed read size (%d but should be %d)\n", bytes_out, sizeof(*request));
      continue;
    }

    if( op_verbose )
    {
      fprintf(stdout, "Read req: %d (bytes %d, TX ID %lld)\n",rv,bytes_out,request->tx_id);
      for( size_t i = 0 ; ; i++ )
      {
//	wchar_t temp[512] = {0};
	const wchar_t* key = NULL;
	const wchar_t* value = NULL;
	if( NLCC_UGetAttributeByIndex(request,i,&key,&value) != 0 )
	{
	  break;
	}
	fprintf(stdout, "%02d: %ws = %ws\n", i, key, value);
      }/* for */
    }/* if verbose */

    /* Process the query here */

    /* Respond */
    NLCC_UInitializeQuery(response);
    response->info.response.allow = 1;
    response->tx_id = request->tx_id;

    memset(&ol,0x00,sizeof(ol));
    ResetEvent(io_event);
    ol.hEvent = io_event;
    rv = DeviceIoControl(h,IOCTL_POLICY_RESPONSE,response,sizeof(*response),NULL,0,&bytes_out,&ol);
    if( rv == FALSE )
    {
      if( GetLastError() == ERROR_IO_PENDING )
      {
	rv = GetOverlappedResult(h,&ol,&bytes_out,TRUE);
      }
    }
    if( rv == FALSE )
    {
      fprintf(stderr, "DeviceIoControl failed (le %d)\n", GetLastError());
      continue;
    }

    if( op_verbose )
      fprintf(stdout, "IOCTL_POLICY_RESPONSE (bytes out %d)\n", bytes_out);
  }
  free(request);
  free(response);
  _endthread();
}/* pdp_worker */

HANDLE pdp_th[16];
HANDLE pep_th[6];

void print_help(void)
{
  
}/* print_help */

int main( int argc , char** argv )
{
  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    char* option = strstr(argv[i],"=");
    if( option != NULL )
    {
      option++;
    }
    else if( strcmp(argv[i],"--pdp") == 0 )
    {
      op_pdp = true;
      continue;
    }
    else if( strcmp(argv[i],"--pep") == 0 )
    {
      op_pep = true;
      continue;
    }
    else if( strcmp(argv[i],"--verify-attributes") == 0 )
    {
      op_verify_attributes = true;
      continue;
    }
    else if( strcmp(argv[i],"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
  }/* for */

  if( argc < 2 )
  {
    print_help();
    return 1;
  }

  if( op_verify_attributes == true )
  {
    verify_attributes();
    return 0;
  }

  cancel_event = CreateEvent(NULL,TRUE,FALSE,NULL);
  if( cancel_event == NULL )
  {
    fprintf(stderr, "fatal: CreateEvent failed\n");
    return 1;
  }

  if( op_pdp )
  {
    fprintf(stderr, "pdp: opening PDP device\n");
    h = CreateFileW(NLCC_PDP_DEVICE,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,
		    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
		    NULL);
    if( h == INVALID_HANDLE_VALUE )
    {
      fprintf(stderr, "pdp: CreateFile failed (%d)\n", GetLastError());
      exit(1);
    }

    for( size_t i = 0 ; i < _countof(pdp_th) ; i++ )
    {
      uintptr_t ret = _beginthread(pdp_worker,0,NULL);
      if (ret == -1)
      {
	fprintf(stderr, "pdp: fatal: _beginthread failed\n");
	exit(1);
      }
      pdp_th[i] = (HANDLE)ret;
    }
  }/* op_pdp */

  if( op_pep )
  {
    fprintf(stdout, "PEP starting\n");
    for( size_t i = 0 ; i < _countof(pep_th) ; i++ )
    {
      uintptr_t ret = _beginthread(client,0,NULL);
      if (ret == -1)
      {
	fprintf(stderr, "pep: fatal: _beginthread failed\n");
	exit(1);
      }
      pep_th[i] = (HANDLE)ret;
    }
  }/* op_pep */

  /* wait for user input to cancel and complete */
  fprintf(stdout, "wait for cancel...  [press any key]\n");
#pragma warning(push)
#pragma warning(disable: 6031)
  _getch();
#pragma warning(pop) 
  SetEvent(cancel_event);

  fprintf(stdout, "wait for pdp to stop\n");
  for( size_t i = 0 ; i < _countof(pdp_th) ; i++ )
  {
    WaitForSingleObject(pdp_th[i],INFINITE);
    CloseHandle(pdp_th[i]);
  }

  fprintf(stdout, "wait for pep to stop\n");
  for( size_t i = 0 ; i < _countof(pep_th) ; i++ )
  {
    WaitForSingleObject(pep_th[i],INFINITE);
    CloseHandle(pep_th[i]);
  }

  CloseHandle(cancel_event);
  fprintf(stdout, "complete\n");

  return 0;
}/* main */
