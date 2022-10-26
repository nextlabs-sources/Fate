/****************************************************************************************
 *
 * NL Destroyer
 *
 * Try and destroy the Desktop Enforcer and Policy Controller.
 *
 ***************************************************************************************/

#include <windows.h>
#include <process.h>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <string>
#include <list>

#include "htimer.hpp"

/** CreateRandomPath
 *
 *  \brief Generate a random path with a given prefix.
 *
 *  \param path (out)    Randomly generated path.
 *  \param in_prefix(in) Prefix to use for randomly generated path.
 */
bool CreateRandomPath( std::wstring& path , const std::wstring& in_prefix )
{
  std::size_t len = rand() % MAX_PATH;

  path = in_prefix;
  path += L"\\";
  wchar_t tid[MAX_PATH];
  _ultow_s(_threadid, tid, MAX_PATH, 10);
  path += tid;
  for( std::size_t i = 0 ; i < len ; i++ )
  {
    wchar_t ch;
    ch = 'a' + (wchar_t)(rand() % 26);
    path += ch;

    if( rand() % 16 == 0 )
    {
      path += L"\\";
    }
  }
  return true;
}/* CreateRandomPath */

void print_help(void)
{
  fprintf(stdout, "NextLabs NL Destroyer (Built %s %s)\n", __DATE__, __TIME__);
  fprintf(stdout, "usage: nldestroyer [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "   --prefix=[path prefix]              Path to use as a prefix.\n");
  fprintf(stdout, "   --count=[iterations]                Number of iterations to perform.\n");
  fprintf(stdout, "                                       Count of 100 is default value.\n");
  fprintf(stdout, "                                       Count of 0 is infinite.\n");
  fprintf(stdout, "   --createfile                        (CreateFile)\n");
  fprintf(stdout, "   --copyfile                          (CopyFile)\n");
  fprintf(stdout, "   --copyfileex                        (CopyFileEx)\n");
  fprintf(stdout, "   --deletefile                        (DeleteFile)\n");
  fprintf(stdout, "   --createdirectory                   (CreateDirectory)\n");
  fprintf(stdout, "   --time                              Total elapsed time to run.  Path\n");
  fprintf(stdout, "                                       generation time is not included.\n");
  fprintf(stdout, "   --verbose                           Verbose output.\n");
  fprintf(stdout, "   --seed=[number]                     Seed for random path generation.  If no\n");
  fprintf(stdout, "                                       specified, then current tick count,\n");
  fprintf(stdout, "                                       GetTickCount(), is used.\n");
  fprintf(stdout, "   --delay                             Delay between operations.\n");
  fprintf(stdout, "   --mt=[thread count]                 Number of threads to use.\n");
  fprintf(stdout, "                                       Default thread count is 1.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "If '--prefix' option is not used, then the current directory is the prefix.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "  nldestroyer --prefix=c:\\foo --count=1000 --createfile\n");
  fprintf(stdout, "  nldestroyer --count=5000 --copyfile --copyfileex\n");
}/* print_help */

static unsigned __stdcall worker( void* arg );

/* File system operation */
static bool op_CreateFileW        = false;
static bool op_CopyFileW          = false;
static bool op_CopyFileExW        = false;
static bool op_DeleteFileW        = false;
static bool op_CreateDirectoryW   = false;

static std::wstring op_prefix;
static std::size_t  op_count = 100;
static std::size_t  op_delay = 0;
static bool op_verbose       = false;

int wmain( int argc , wchar_t** argv )
{
  fprintf(stdout, "................\n");
  srand(GetTickCount());
  std::size_t mt_count  = 1; /* single thread default */
  wchar_t current_dir[MAX_PATH+1] = {0};
  if( GetCurrentDirectoryW(_countof(current_dir),current_dir) == 0 )
  {
    fprintf(stderr, "nldestroyer: cannot read current directory\n");
    return 1;
  }

  op_prefix = current_dir;

  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {
    wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

    if( wcsncmp(argv[i],L"--prefix=",wcslen(L"--prefix=")) == 0 )
    {
      op_prefix = option;
      continue;
    }
    if( wcsncmp(argv[i],L"--count=",wcslen(L"--count=")) == 0 )
    {
      op_count = _wtoi(option);
      continue;
    }
    if( _wcsicmp(argv[i],L"--time") == 0 )
    {
      op_CreateFileW = true;
      continue;
    }
    if( _wcsicmp(argv[i],L"--CreateFile") == 0 )
    {
      op_CreateFileW = true;
      continue;
    }
    if( _wcsicmp(argv[i],L"--CopyFile") == 0 )
    {
      op_CopyFileW = true;
      continue;
    }
    if( _wcsicmp(argv[i],L"--CopyFileEx") == 0 )
    {
      op_CopyFileExW = true;
      continue;
    }
    if( _wcsicmp(argv[i],L"--DeleteFile") == 0 )
    {
      op_DeleteFileW = true;
      continue;
    }
    if( _wcsicmp(argv[i],L"--CreateDirectory") == 0 )
    {
      op_CreateDirectoryW = true;
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
    else if( wcsncmp(argv[i],L"--seed=",7) == 0 )
    {
      int x = _wtoi(option);
      srand(x);
      continue;
    }
    else if( wcsncmp(argv[i],L"--mt=",5) == 0 )
    {
      mt_count = _wtoi(option);
      continue;
    }
	else if( wcsncmp(argv[i],L"--delay=",8) == 0 )
    {
      op_delay = _wtoi(option);
      continue;
    }
  }/* for */

  /* There must be at least one option */
  bool any_options = op_CreateFileW || op_CopyFileW || op_CopyFileExW ||
      op_DeleteFileW || op_CreateDirectoryW;
  if( argc <= 1 || any_options == false )
  {
    print_help();
    return 0;
  }

  if( op_count >= (0x1 << 30) )
  {
    fprintf(stdout, "Do you really have that much time?\n");
    return 0;
  }

  /***********************************************************************************
   *
   * Create threads in a suspended state.  Resume them and join after each completes
   *
   **********************************************************************************/

  std::list<HANDLE> tlist;
  for( std::size_t i = 0 ; i < mt_count ; i++ ) // create threads in a suspended state
  {
    HANDLE th;
    th = (HANDLE)_beginthreadex(NULL,0,worker,NULL,CREATE_SUSPENDED,NULL);
    if( th == NULL )
    {
      fprintf(stderr, "nldestroyer: fatal error: _beginthreadex failed (errno %d)\n", errno);
      exit(1);
    }
    tlist.push_back(th);
  }
  
  std::list<HANDLE>::iterator it;
  DWORD start_time = GetTickCount(); /* start time */
  for( it = tlist.begin() ; it != tlist.end() ; it++ )   // resume suspended threads
  {
    HANDLE th = *it;
    ResumeThread(th);
  }
  for( it = tlist.begin() ; it != tlist.end() ; it++ )   // join threads
  {
    HANDLE th = *it;
    WaitForSingleObject(th,INFINITE);
    CloseHandle(th);
  }
  DWORD runtime = (GetTickCount() - start_time);

  fprintf(stdout,"%d iterations in %d ms (%d sec)\n", op_count, runtime, runtime / 1000);

  return 0;
}/* main */

/* Worker for file system ops */
static unsigned __stdcall worker( void* arg )
{
  double create_time = 0.0;
  for( std::size_t i = 0 ; op_count == 0 || i < op_count ; i++ )
  {
    std::wstring source, dest;

    CreateRandomPath(source,op_prefix);
    CreateRandomPath(dest,op_prefix);

    if( op_CreateFileW == true )
    {
      nextlabs::high_resolution_timer ht;
      HANDLE h = CreateFileW(source.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
      create_time = ht.diff();
      if( h != NULL )
      {
	CloseHandle(h);
      }
    }
    if( op_CopyFileW == true )
    {
      CopyFileW(source.c_str(),dest.c_str(),FALSE);
    }
    if( op_CopyFileExW == true )
    {
      CopyFileExW(source.c_str(),dest.c_str(),NULL,NULL,NULL,0);
    }
    if( op_DeleteFileW == true )
    {
      DeleteFileW(source.c_str());
    }
    if( op_CreateDirectoryW == true )
    {
      CreateDirectoryW(source.c_str(),NULL);
    }
	
    if( op_verbose == true )
    {
      fprintf(stdout, "source: %ws\n", source.c_str());
      fprintf(stdout, "target: %ws\n", dest.c_str());
      fprintf(stdout, "CreateTime (TID: %ul): %f ms \n", _threadid, create_time);
      fprintf(stdout, "Sleeping for %d ms\n", op_delay);
    }

    if( op_delay >= 0 ) /* avoid yeild */
    {
      Sleep(static_cast<DWORD>(op_delay));
    }
  }
  _endthreadex(0);
  return 0;
}/* worker */
