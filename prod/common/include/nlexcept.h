/**************************************************************************
 *
 * nlexcept.h
 *
 * Interface for exception information.
 *
 *************************************************************************/

#ifndef __NLEXCEPT_H__
#define __NLEXCEPT_H__

#if defined(_WIN32) || defined(_WIN64)
#include <imagehlp.h>
#include <cstdio>
#include <wchar.h>
#include <psapi.h>
#include <stdlib.h>
#include <nlconfig.hpp>
#include <Shlobj.h>

#pragma  comment(lib, "Shell32.lib")
/** NLEXCEPT_CBINFO
 *
 *  \brief Exception callback information.  Used to inform the nlexcept interface user of
 *         contextual information when an exception occurs.  Also permits override some
 *         defaults such as location to generate the dump file.
 */
typedef struct
{
  int version;                     /* CBINFO version */

  /************************************************
   * Static information/context of an exception
   ***********************************************/
  int code;                        /* Exception code */
  char source_file[MAX_PATH];      /* Source file */
  int source_line;                 /* Source file line number */

  /************************************************
   * User supplied information
   ***********************************************/
  int use_dump_root;               /* Use dump root set by callback */
  wchar_t dump_root[MAX_PATH];     /* Root path to generation minidump */
} NLEXCEPT_CBINFO;

/* Required libraries */
#pragma comment(lib,"advapi32.lib")

/* Prototype GetProcessImageFileNameW @ psapi.dll for dynamic load */
typedef DWORD (WINAPI *GetProcessImageFileNameW_fn_t)( HANDLE hProcess,
						       PWCHAR lpImageFileName,
						       DWORD nSize);

/* Prototype MiniDumpWriteDump @ dbghelp.dll for dynamic load */
typedef BOOL (WINAPI *MiniDumpWriteDump_fn_t)( HANDLE hProcess,
					       DWORD ProcessId,
					       HANDLE hFile,
					       MINIDUMP_TYPE DumpType,
					       PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
					       PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
					       PMINIDUMP_CALLBACK_INFORMATION CallbackParam );

/** nlexcept_filter
 *
 *  \brief Filter exceptions in __try/__except for MSVC.  Generate a minidump file
 *         with the name of the current process image name and the '.dmp' extension.
 *
 *         Example: myprog.exe -> myprog.exe.dmp
 *
 *  \param code (in)                Exception code.
 *  \param exception_pointers (in)  Exceptoin pointers.
 *  \param file_name (in-opt)       File name.
 *  \param file_line (in-opt)       File line.
 *  \param state (out-opt)          Inidcate that an exception has occured.  Set to
 *                                  non-zero exception code when nlexcept_filter_ex
 *                                  is called.
 *  \param callback (in-opt)        User supplied callback to be called when an exception occurs.
 *
 *  \return Exception filter policy.  See UnhandledExceptionFilter documentation
 *          at MSDN.
 */
static int nlexcept_filter_ex( unsigned int code ,
			       _In_opt_ LPEXCEPTION_POINTERS exception_pointers ,
			       _In_opt_ const char* file_name ,
			       _In_ int file_line ,
			       _In_opt_ int* state ,
			       _In_opt_ void (*callback)( NLEXCEPT_CBINFO* cbinfo ) )
{
  HANDLE ph = NULL;                 /* handle to process for minidump write */
  HANDLE fh = INVALID_HANDLE_VALUE; /* handle to file for minidump write */
  HMODULE hlib_psapi = NULL;        /* handle to psapi.dll */
  HMODULE hlib_dbghelp = NULL;      /* handle to dbghelp.dll */
  NLEXCEPT_CBINFO cbinfo;           /* exception callback context */

  /* Set the state to a non-zero value to indicate that an exception has occured. */
  if( state != NULL )
  {
    *state = code;
  }

  /* If there is a callback function defined setup callback information and call it. */
  memset(&cbinfo,0x00,sizeof(cbinfo));
  cbinfo.code = code;          /* exception code */
  cbinfo.use_dump_root = 0;    /* do not use callback defined dump root */
  if( callback != NULL )
  {
    if( file_name != NULL )
    {
      strncpy_s(cbinfo.source_file,_countof(cbinfo.source_file),file_name,_TRUNCATE);
      cbinfo.source_line = file_line;
    }
    callback(&cbinfo); /* call user supplied callback to inform of exception */
  }

  char exception_msg[MAX_PATH * 2] = {0};
  if( file_name != NULL )
  {
      _snprintf_s(exception_msg,_countof(exception_msg), _TRUNCATE,
          "nlexcept_filter_ex: exception 0x%x : line %d of %s\n",
          code, file_line, file_name);
  }
  else
  {
      _snprintf_s(exception_msg,_countof(exception_msg), _TRUNCATE,
	      "nlexcept_filter_ex: exception 0x%x\n",code);
  }

  ph = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());
  if( ph == NULL )
  {
    goto nlexcept_filter_complete;
  }

  /* Policy Controller root directory (InstallDir) */
  WCHAR comp_root[MAX_PATH] = {0};
  wcsncpy_s(comp_root,_countof(comp_root),cbinfo.dump_root,_TRUNCATE); /* default to callback location */
  if( cbinfo.use_dump_root == 0 )
  {
    if( NLConfig::GetComponentInstallPath(NULL,comp_root,_countof(comp_root)) == false )
    {
      goto nlexcept_filter_complete;
    }
  }

  /* To support Win2K GetProcessImageFileName() and MiniDumpWriteDump() must
     be dynamically located.  Otherwise there is an unresolved link dependency
     at runtime which prevents a binary from being loaded.
   */
  hlib_psapi   = LoadLibraryA("psapi.dll");
  hlib_dbghelp = LoadLibraryA("dbghelp.dll");

  if( hlib_psapi == NULL || hlib_dbghelp == NULL )
  {
    /* Fail and drop down to clean up */
    goto nlexcept_filter_complete;
  }

  GetProcessImageFileNameW_fn_t GetProcessImageFileNameW_fn = NULL;
  MiniDumpWriteDump_fn_t MiniDumpWriteDump_fn = NULL;

  /* GetProcessImageFileName @ psapi.dll */
  GetProcessImageFileNameW_fn = (GetProcessImageFileNameW_fn_t)GetProcAddress(hlib_psapi,
									      "GetProcessImageFileNameW");

  /* MiniDumpWriteDump @ dbghelp.dll */
  MiniDumpWriteDump_fn = (MiniDumpWriteDump_fn_t)GetProcAddress(hlib_dbghelp,
								"MiniDumpWriteDump");

  if( GetProcessImageFileNameW_fn == NULL || MiniDumpWriteDump_fn == NULL )
  {
    /* Fail and drop down to clean up */
    goto nlexcept_filter_complete;
  }

  WCHAR image_path[MAX_PATH] = {0};
  if( GetProcessImageFileNameW_fn(ph,image_path,_countof(image_path)) == 0 )
  {
    goto nlexcept_filter_complete;
  }

  WCHAR* image_name = wcsrchr(image_path,L'\\');  /* reverse char search */
  if( image_name == NULL )
  {
    goto nlexcept_filter_complete;
  }

  /* The dump file will be generated in Program Files\NextLabs\ instead
     of the Policy Controller directroy.  This permits write access.
   */
  WCHAR dumpfile[MAX_PATH] = {0};
  _snwprintf_s(dumpfile,_countof(dumpfile), _TRUNCATE,L"%s\\%s.dmp",comp_root,image_name);

  fh = CreateFileW(dumpfile,GENERIC_READ|GENERIC_WRITE,0,NULL,
		   CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

  if( fh == INVALID_HANDLE_VALUE )
  {
      WCHAR szTempPath[MAX_PATH] = { 0 };
      dumpfile[0] = L'\0';
      if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szTempPath)))
      {
          _snwprintf_s(dumpfile, _countof(dumpfile), _TRUNCATE, L"%s\\Temp\\%s.dmp", szTempPath, image_name);
          fh = CreateFileW(dumpfile, GENERIC_READ | GENERIC_WRITE, 0, NULL,
              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

      }
      if (fh == INVALID_HANDLE_VALUE)    goto nlexcept_filter_complete;
  }

  int type = (int)(MiniDumpWithDataSegs |
		   MiniDumpWithHandleData |
		   MiniDumpWithHandleData |
		   MiniDumpWithProcessThreadData);

  /********************************************************************************************
   * Debug build will generate a complete process dump.
   *******************************************************************************************/
  if( NLConfig::IsDebugMode() == true )
  {
    type |= MiniDumpWithFullMemory;
  }

  MINIDUMP_EXCEPTION_INFORMATION mn_exception_info;
  mn_exception_info.ThreadId = GetCurrentThreadId();
  mn_exception_info.ExceptionPointers = exception_pointers;
  mn_exception_info.ClientPointers = FALSE;

  MiniDumpWriteDump_fn(ph,GetCurrentProcessId(),fh,(MINIDUMP_TYPE)type,
		       &mn_exception_info,NULL,NULL);

 nlexcept_filter_complete:

  if( ph != NULL )                  /* close process handle */
  {
    CloseHandle(ph);
  }

  if( fh != INVALID_HANDLE_VALUE )  /* close file handle */
  {
    CloseHandle(fh);
  }

  /* Unload libs used for dynamic function loading for Win2K support */
  if( hlib_psapi != NULL )
  {
    FreeLibrary(hlib_psapi);
  }

  if( hlib_dbghelp != NULL )
  {
    FreeLibrary(hlib_dbghelp);
  }

  return EXCEPTION_CONTINUE_SEARCH;
}/* nlexcept_filter */

/** NLEXCEPT_FILTER
 *
 *  \brief  Macro for deafult exception filter.
 *  \return EXCEPTION_CONTINUE_SEARCH
 */
#define NLEXCEPT_FILTER() nlexcept_filter_ex(GetExceptionCode(),GetExceptionInformation(),__FILE__,__LINE__,NULL,NULL)

/** NLEXCEPT_FILTER_EX
 *
 *  \brief  Macro for deafult exception filter.
 *  \param  state
 *  \return EXCEPTION_CONTINUE_SEARCH
 */
#define NLEXCEPT_FILTER_EX(state) nlexcept_filter_ex(GetExceptionCode(),GetExceptionInformation(),__FILE__,__LINE__,state,NULL)

/** NLEXCEPT_FILTER_EX2
 *
 *  \brief  Macro for deafult exception filter.
 *  \param  state (opt-out)   Exception code of the exception.
 *  \param  callback (opt-in) Callback to be issued during exception handling.
 *  \return EXCEPTION_CONTINUE_SEARCH
 */
#define NLEXCEPT_FILTER_EX2(state,callback) nlexcept_filter_ex(GetExceptionCode(),GetExceptionInformation(),__FILE__,__LINE__,state,callback)

#endif /* defined(_WIN32) || defined(_WIN64) */

#endif /* __NLEXCEPT_H__ */
