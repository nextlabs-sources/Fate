
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include "nlca_service_efilter.hpp"

/** efilter
 *
 *  Excecption filter.  When called a minidump in the Policy Controller
 *  root directory name 'cepdpman.dmp' is created.
 *
 *  \param exception_pointers (in) Exception pointers provided by
 *                                 OS for the exception that occurred.
 *
 *
 */
LONG WINAPI efilter( LPEXCEPTION_POINTERS exception_pointers )
{
  HANDLE ph = INVALID_HANDLE_VALUE;
  HANDLE fh = INVALID_HANDLE_VALUE;

  ph = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());
  if( ph == NULL )
  {
    goto dump_complete;
  }

  LONG rstatus;
  HKEY hKey = NULL; 

  rstatus = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
			  "SOFTWARE\\NextLabs\\Compliant Enterprise\\Policy Controller",
			  0,KEY_QUERY_VALUE,&hKey);
  if( rstatus != ERROR_SUCCESS )
  {
    goto dump_complete;
  }

  /* Policy Controller root directory (InstallDir) */
  WCHAR pc_root[MAX_PATH];
  DWORD pc_root_size = sizeof(pc_root);
  rstatus = RegQueryValueExW(hKey,L"InstallDir",NULL,NULL,(LPBYTE)pc_root,&pc_root_size);
  RegCloseKey(hKey);
  if( rstatus != ERROR_SUCCESS )
  {
    goto dump_complete;
  }

  WCHAR dumpfile[MAX_PATH];
  _snwprintf_s(dumpfile,_countof(dumpfile), _TRUNCATE,L"%s\\nlca_service.dmp",pc_root);

  fh = CreateFileW(dumpfile,GENERIC_READ|GENERIC_WRITE,0,NULL,
		   CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if( fh == INVALID_HANDLE_VALUE )
  {
    goto dump_complete;
  }

  MINIDUMP_EXCEPTION_INFORMATION mn_exception_info;
  MINIDUMP_TYPE type = (MINIDUMP_TYPE)(MiniDumpWithDataSegs |
				       MiniDumpWithHandleData |
				       MiniDumpWithProcessThreadData);

  mn_exception_info.ThreadId = GetCurrentThreadId();
  mn_exception_info.ExceptionPointers = exception_pointers;
  mn_exception_info.ClientPointers = FALSE;

  /* Win2K does not have MiniDumpWriteDump but this source must support platforms
     which have this function.  Look up the function and call it if it can be
     found.
  */
  typedef BOOL (WINAPI *MiniDumpWriteDump_fn_t)( HANDLE hProcess,
						 DWORD ProcessId,
						 HANDLE hFile,
						 MINIDUMP_TYPE DumpType,
						 PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
						 PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
						 PMINIDUMP_CALLBACK_INFORMATION CallbackParam );

  HMODULE hlib = LoadLibraryA("dbghelp.dll");
  if( hlib == NULL )
  {
    goto dump_complete;
  }

  MiniDumpWriteDump_fn_t MiniDumpWriteDump_fn = (MiniDumpWriteDump_fn_t)GetProcAddress(hlib,"MiniDumpWriteDump");
  if( MiniDumpWriteDump_fn != NULL )
  {
    MiniDumpWriteDump_fn(ph,GetCurrentProcessId(),fh,type,
			 &mn_exception_info,NULL,NULL);
  }
  FreeLibrary(hlib);

dump_complete:

  if( ph != NULL )
  {
    CloseHandle(ph);
  }

  if( fh != NULL )
  {
    CloseHandle(fh);
  }

  /* This will supress the pop-up to the user.  The process is quietly
     terminated.
  */
  return EXCEPTION_EXECUTE_HANDLER;
}/* efilter */
