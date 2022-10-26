
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <windows.h>

#include "eframework/timer/timer_high_resolution.hpp"

#define VERIFY_MIN_BLOCK_SIZE 1
#define VERIFY_MAX_BLOCK_SIZE (64 * 1024)

int  op_block_size = 4096;
int  op_count      = 100;
bool op_verbose    = false;
bool op_noncached  = false;
bool op_verify     = false;

int wmain( int argc , wchar_t** argv )
{
  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

    if( wcsncmp(argv[i],L"--count=",wcslen(L"--count=")) == 0 && option )
    {
      op_count = _wtoi(option);
      continue;
    }
    if( wcsncmp(argv[i],L"--block-size=",wcslen(L"--block-size=")) == 0 && option )
    {
      op_block_size = _wtoi(option);
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
    else if( _wcsicmp(argv[i],L"--non-cached") == 0 )
    {
      op_noncached = true;
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verify") == 0 )
    {
      op_verify = true;
      continue;
    }

  }/* for */

  HANDLE h;
  DWORD create_disp = FILE_ATTRIBUTE_NORMAL;

  if( op_noncached )
  {
    create_disp |= FILE_FLAG_NO_BUFFERING;
  }

  nextlabs::high_resolution_timer ht;
  h = CreateFileA("foo.bin",GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
		  create_disp,NULL);
  if( h == INVALID_HANDLE_VALUE )
  {
    fprintf(stderr, "CreateFile failed (%d)\n", GetLastError());
    return 1;
  }

  BOOL rv = FALSE;
  size_t total_bytes_written = 0;
  assert( op_block_size >= VERIFY_MIN_BLOCK_SIZE && op_block_size <= VERIFY_MAX_BLOCK_SIZE );
  unsigned char* buf = new unsigned char [op_block_size];
  memset(buf,'b',op_block_size);
  for( int i = 0 ; i < op_count ; i++ )
  {
    DWORD bytes_written = 0;
    rv = WriteFile(h,buf,op_block_size,&bytes_written,NULL);
    if( rv != TRUE )
    {
      fprintf(stderr, "WriteFile failed (%d)\n", GetLastError());
      break;
    }
    total_bytes_written += bytes_written;
  }
  CloseHandle(h);
  ht.stop();
  fprintf(stdout, "%d bytes in %.2fms\n",total_bytes_written,ht.diff());

  if( op_verify )
  {
    h = CreateFileA("foo.bin",GENERIC_READ,0,NULL,OPEN_ALWAYS,create_disp,NULL);
    if( h == INVALID_HANDLE_VALUE )
    {
      fprintf(stderr, "CreateFile failed (%d)\n", GetLastError());
      return 1;
    }
    for( ; ; )
    {
      DWORD bytes_read = 0;
      unsigned char ch = 0;
      rv = ReadFile(h,&ch,1,&bytes_read,NULL);
      if( rv != TRUE )
      {
	fprintf(stderr, "ReadFile failed (%d)\n", GetLastError());
	break;
      }
      if( bytes_read == 0 )
      {
	break;
      }
      if( ch != 'b' )
      {
	fprintf(stderr, "Verification error\n");
	break;
      }
    }
    CloseHandle(h);
  }

  return 0;
}/* main */
