/********************************************************************************
 * PluginDriver
 *
 * Testing framework for plugins.
 *
 *******************************************************************************/
#include <cstdio>
#include <conio.h>
#include <windows.h>
#include <dbghelp.h>

typedef int (*plugin_entry_t)( void** );   /* Plugin entry */
typedef int (*plugin_unload_t)( void* );   /* Plugin unload */

/** efilter
 *
 *  Excecption filter.  When called a minidump in the Policy Controller
 *  root directory name 'cepdpman.dmp' is created.
 *
 *  \param exception_pointers (in) Exception pointers provided by
 *                                 OS for the exception that occurred.
 *
 */
static LONG WINAPI efilter( LPEXCEPTION_POINTERS exception_pointers )
{
    fprintf(stderr, "efilter: exception occurred.  writing dump file plugin_driver.exe.dmp\n");
    HANDLE ph = INVALID_HANDLE_VALUE;
    HANDLE fh = INVALID_HANDLE_VALUE;

    ph = OpenProcess(PROCESS_ALL_ACCESS,FALSE,GetCurrentProcessId());
    if( ph == NULL )
    {
        goto dump_complete;
    }

    WCHAR dumpfile[MAX_PATH] = {0};
    _snwprintf_s(dumpfile,sizeof(dumpfile)/sizeof(dumpfile[0]),L"plugin_driver.exe.dmp");

    fh = CreateFileW(dumpfile,
                     GENERIC_READ|GENERIC_WRITE,
                     0,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);

    if( fh == INVALID_HANDLE_VALUE )
    {
        goto dump_complete;
    }

    MINIDUMP_EXCEPTION_INFORMATION mn_exception_info;
    int type = (int)(MiniDumpWithDataSegs |
                     MiniDumpWithHandleData |
                     MiniDumpWithProcessThreadData|
		     MiniDumpWithFullMemory);

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
        MiniDumpWriteDump_fn(ph,GetCurrentProcessId(),fh,(MINIDUMP_TYPE)type,
                             &mn_exception_info,NULL,NULL);
    }
    FreeLibrary(hlib);

 dump_complete:

    if( ph != NULL )
    {
        CloseHandle(ph);
    }

    if( fh != INVALID_HANDLE_VALUE )
    {
        CloseHandle(fh);
    }

    /* This will supress the pop-up to the user.  The process is quietly
       terminated.
    */
    return EXCEPTION_EXECUTE_HANDLER;
}/* efilter */

int main( int argc , char** argv )
{
  if( argc != 2 )
  {
    fprintf(stdout, "plugin_driver: usage: plugin_driver [library_path]\n");
    return 0;
  }

  fprintf(stdout, "plugin_driver: installing exception filter\n");
  SetUnhandledExceptionFilter(efilter);

  fprintf(stdout, "plugin_driver: loading library %s\n", argv[1]);

  HMODULE hlib = LoadLibraryA(argv[1]);
  if( hlib == NULL )
  {
    fprintf(stdout, "plugin_driver: cannot load library (LastError %d)\n", GetLastError());
    return 1;
  }

  plugin_entry_t plugin_entry;
  plugin_unload_t plugin_unload;

  plugin_entry  = (plugin_entry_t)GetProcAddress(hlib,"PluginEntry");
  plugin_unload = (plugin_unload_t)GetProcAddress(hlib,"PluginUnload");

  if( plugin_entry == NULL || plugin_unload == NULL )
  {
    fprintf(stdout, "plugin_driver: no entry point(s): plugin_entry @ 0x%p plugin_unload @ 0x%p\n",
	    plugin_entry, plugin_unload);
    goto main_complete;
  }

  int result;
  void* context = NULL;

  fprintf(stdout, "plugin_driver: calling PluginEntry @ 0x%p\n", plugin_entry);
  try
  {
    result = plugin_entry(&context);
  }
  catch(...)
  {
    fprintf(stderr, "plugin_drive: PluginEntry caused an exception\n");
    result = 0;
  }
  if( result == 0 )
  {
    fprintf(stdout, "plugin_driver: entry point failed\n");
    goto main_complete;
  }

  fprintf(stdout, "plugin_driver: entry point context @ 0x%p\n",context);
  fprintf(stdout, "plugin_driver: Press any key to unload...\n");
  (void)_getch(); /* wait for console input */

  fprintf(stdout, "plugin_driver: calling PluginUnload @ 0x%p\n", plugin_unload);
  try
  {
    result = plugin_unload(context);
  }
  catch(...)
  {
    fprintf(stderr, "plugin_driver: PluginUnload caused an exception\n");
    result = 0;
  }

  if( result == 0 )
  {
    fprintf(stdout, "plugin_driver: PluginUnload failed\n");
  }

 main_complete:

  FreeLibrary(hlib);
  return 0;
}/* main */
