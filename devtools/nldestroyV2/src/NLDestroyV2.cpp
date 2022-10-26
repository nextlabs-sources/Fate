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

void print_help(void)
{
  fprintf(stdout, "\nNextLabs NL Destroyer v2 (Built %s %s)\n", __DATE__, __TIME__);
  fprintf(stdout, "Each operation will create, copy, and delete a file in the specified folder\n");
  fprintf(stdout, "If the source option is specified, it will use that source file.\n");
  fprintf(stdout, "usage: nldestroyV2 --dir=<folder> [option] ...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Options:\n");
  fprintf(stdout, "   --dir=<folder>             Directory to do the operation on.\n");
  fprintf(stdout, "                              DO NOT include '\\' at the end !!\n");
  fprintf(stdout, "   --source=<file>            Source file to copy. Please provide full path.\n");
  fprintf(stdout, "                              Default is none, and the process will create\n");
  fprintf(stdout, "                              a text file with pid as the filename.\n");
  fprintf(stdout, "   --size=<size>              Size of file creation, used only if no source \n");
  fprintf(stdout, "                              specified. Multiple of 10 bytes.\n");
  fprintf(stdout, "   --count=<iterations>       Number of iterations to perform.\n");
  fprintf(stdout, "                              Count of 100 is default value, 0 is infinite.\n");
  fprintf(stdout, "   --verbose                  Verbose output.\n");
  fprintf(stdout, "   --delay=<delay>            Delay between each iteration. Default delay is 0.\n");
  fprintf(stdout, "   --thread=<thread count>    Number of threads to use. Default is 1.\n");
  fprintf(stdout, "   --nodelete                 Leave the created file on.\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Examples:\n");
  fprintf(stdout, "  nldestroyV2 --dir=\"c:\\temp\"\n");
  fprintf(stdout, "  nldestroyV2 --dir=\"c:\\temp folder\" --source=\"c:\\test.txt\"\n");
}/* print_help */

static unsigned __stdcall worker( void* arg );

/* File system operation */
static bool op_folderExist = false;

static wchar_t* op_prefix;
static std::wstring op_folder;
static std::size_t  op_count = 100;
static std::size_t  op_delay = 0;
static std::size_t  op_size = 10;
static std::wstring op_source;
static std::wstring filename;
static std::wstring extension;
static bool op_verbose       = false;
static bool op_delete		 = false;

int wmain( int argc , wchar_t** argv )
{
  srand(GetTickCount());
  std::size_t mt_count  = 1; /* single thread default */
  wchar_t current_dir[MAX_PATH+1] = {0};
  if( GetCurrentDirectoryW(_countof(current_dir),current_dir) == 0 )
  {
    fprintf(stderr, "nldestroyer: cannot read current directory\n");
    return 1;
  }

  op_prefix = current_dir;
  op_source = L"";

  /* Process options */
  for( int i = 1 ; i < argc ; ++i )
  {

	wchar_t* option = wcsstr(argv[i],L"=");
    if( option != NULL )
    {
      option++;
    }

	if( wcsncmp(argv[i],L"--dir=",wcslen(L"--dir=")) == 0 )
    {
      op_folderExist = true;
	  op_folder = option;
      continue;
    }
	else if( wcsncmp(argv[i],L"--count=",wcslen(L"--count=")) == 0 )
    {
      op_count = _wtoi(option);
      continue;
    }
	else if( wcsncmp(argv[i],L"--source=",wcslen(L"--source=")) == 0 )
    {
      op_source = option;
      continue;
    }
	else if( wcsncmp(argv[i],L"--size=",wcslen(L"--size=")) == 0 )
    {
      op_size = _wtoi(option);
      continue;
    }
    else if( _wcsicmp(argv[i],L"--verbose") == 0 )
    {
      op_verbose = true;
      continue;
    }
	else if( _wcsicmp(argv[i],L"--nodelete") == 0 )
    {
      op_delete = true;
      continue;
    }
    else if( wcsncmp(argv[i],L"--thread=",5) == 0 )
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

  /* There must be at least folder name option */

  if (op_folderExist == false)
  {
    print_help();
    return 0;
  }

  if (op_source != L"")
  {
	FILE* fp = NULL;
 
    //this will fail if more capabilities to read the 
    //contents of the file is required (e.g. \private\...)
    if (_wfopen_s(&fp, op_source.c_str(), L"r") != 0)
	{
		fprintf(stdout, "The specified file %ws is not found or unreadable !!.", op_source.c_str());
        return 0;
    }
	int found, found2;
	found = static_cast<int>(op_source.find_last_of(L"/\\"));
    found2 = static_cast<int>(op_source.find_last_of(L"."));
	filename = op_source.substr(found+1, found2-found-1);
	extension = op_source.substr(found2);
    
  }

  if( op_count >= (0x1 << 30) )
  {
    fprintf(stdout, "Do you really have that much time?\n");
    return 0;
  }

  fprintf(stdout, "Invoking %d worker thread(s) on '%ws' doing %d iterations with delay of %d each...\n\n", mt_count, op_folder.c_str(), op_count, op_delay);
  CreateDirectory(op_folder.c_str(),NULL);

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

  fprintf(stdout,"%d iterations in %d ms (%d sec)\n", op_count * mt_count, runtime, runtime / 1000);

  return 0;
}/* main */

/* Worker for file system ops */
static unsigned __stdcall worker( void* arg )
{
  double create_time = 0.0;
  DWORD dwBytesWritten;
  char DataBuffer[10]={'N','e','x','t','l','a','b','s','\0'};

  for( std::size_t i = 0 ; op_count == 0 || i < op_count ; i++ )
  {

	std::wstring source, dest;

	wchar_t tid[MAX_PATH];
	_ultow_s(_threadid, tid, MAX_PATH, 10);
	
	wchar_t w[36] = {0};
	_itow_s(static_cast<int>(i), w, 36, 10);
	
	//create file
    nextlabs::high_resolution_timer ht;

	if (op_source == L"")
	{
	  source = op_folder+L"\\file" + tid + L"-" + w;
	  HANDLE h = CreateFileW(source.c_str(),GENERIC_READ | GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
      create_time = ht.diff();
      if( h == INVALID_HANDLE_VALUE )
      {
		fprintf(stdout, "File Create Error. Exiting ... \n");
		_endthreadex(0);
		return 0;
	  } else {
		for (std::size_t idx=0; idx< op_size/10; idx++)
		{
			WriteFile(h, DataBuffer, 10, &dwBytesWritten, NULL);
		}
		CloseHandle(h);
	  }
	  dest = source + L"copy";
	} else
	{

	  source = op_source;
	  dest = op_folder + L"\\" + filename + L"-" + tid + L"-" + w + L"copy" + extension;
	}

	
	//copy file
    CopyFileW(source.c_str(),dest.c_str(),FALSE);

	//delete file
	if (op_delete == false)
	{
		DeleteFileW(source.c_str());
		DeleteFileW(dest.c_str());
	}

    if( op_verbose == true )
    {
      fprintf(stdout, "source: %ws\n", source.c_str());
      fprintf(stdout, "target: %ws\n", dest.c_str());
      fprintf(stdout, "CreateTime (TID: %ul): %f ms \n", _threadid, create_time);
      fprintf(stdout, "Sleeping for %d ms\n", op_delay);
    }

    if( op_delay >= 0 ) /* avoid yield */
    {
      Sleep(static_cast<DWORD>(op_delay));
    }
  }
  _endthreadex(0);
  return 0;
}/* worker */
