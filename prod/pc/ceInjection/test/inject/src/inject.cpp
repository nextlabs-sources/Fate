#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include "madCHook - static.h"

/** inject
 *
 *  \brief Inject a library into a specific process.
 *
 *  \param lib (in) Library
 *  \param pid (in) Process ID
 *
 *  \return True if the library was injected into the specified process.
 *          Otherwise false.
 */
static bool inject( const char* lib,
		    DWORD pid );

/** uninject
 *
 *  \brief Uninject a library into a specific process.
 *
 *  \param lib (in) Library
 *  \param pid (in) Process ID
 *
 *  \return True if the library was uninjected into the specified process.
 *          Otherwise false.
 */
static bool uninject( const char* lib,
		      DWORD pid );

int main( int argc , char** argv )
{
  if( argc < 3 )
  {
    fprintf(stdout, "usage: inject [lib] [pid]\n");
    return 1;
  }

  const char* lib = argv[1];  // library
  DWORD pid = atoi(argv[2]);  // pid

  InitializeMadCHook();

  bool result;
  result = inject(lib,pid);

  if( result != true )
  {
    fprintf(stderr, "inject: failed\n");
  }

  FinalizeMadCHook();

  int status = 0;
  if( result == false )
  {
    status = 1;
  }

  return 0;
}/* main */

static bool inject( const char* lib ,
		    DWORD pid )
{
  /* The process with 'pid' must be opened to its handle. */
  HANDLE phandle;
  phandle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);

  if( phandle == NULL )
  {
    fprintf(stdout, "inject: OpenProcess failed (%d)\n", GetLastError());
    return false;
  }

  /* Inject library into that process */
  BOOL result;
  result = InjectLibraryA(phandle,lib);

  CloseHandle(phandle);

  if( result == FALSE )
  {
    fprintf(stdout, "inject: InjectLibrary failed (le %d)\n", GetLastError());
    return false;
  }

  return true;
}/* inject */

static bool uninject( const char* lib ,
		      DWORD pid )
{
  HANDLE phandle;
  phandle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pid);

  if( phandle == NULL )
  {
    fprintf(stdout, "inject: OpenProcess failed (%d)\n", GetLastError());
    return false;
  }

  BOOL result;
  result = UninjectLibraryA(phandle,lib);

  CloseHandle(phandle);

  if( result == FALSE )
  {
    fprintf(stdout, "inject: UninjectLibrary failed\n");
    return false;
  }

  return true;
}/* uninject */
