/***********************************************************************
 *
 * Compliant Enterprise - Dynamic Injection Unit Test
 *
 * This file serves as a simple demonstration of the CEAdapter injection
 * interface and to test basic hooking functionality.
 *
 *  Summary of hooked functions: 
 *
 *  Hook                        Target
 *  ---------------------------------------------------------------
 *  myMoveFileW                 {kernel32.dll,MoveFileW}
 *  myMoveFileExW               {kernel32.dll,MoveFileExW}
 *  myMoveFileWithProgressW     {kernel32.dll,MoveFileWithProgressW}
 *  myCopyFileW                 {kernel32.dll,CopyFileW}
 *  myCopyFileExW               {kernel32.dll,CopyFileExW}
 *  myDeleteFileW               {kernel32.dll,DeleteFileW}
 *
 **********************************************************************/

#include <cstdio>
#include <cassert>
#include <tchar.h>
#include <windows.h>

#include "CEAdapter.h"

#include "CEsdk.h"
#include "cesdk_wrapper.h"

/***********************************************************************
 * Hooks
 **********************************************************************/
static BOOL WINAPI myMoveFileW( LPCTSTR lpExistingFileName,
				LPCTSTR lpNewFileName )
{
  adapter::SetAdapterState("kernel32.dll","MoveFileW");

  _ftprintf(stdout, _T("MoveFileW: %s -> %s\n"), lpExistingFileName, lpNewFileName);

  if( cesdk_wrapper::Eval(CE_ACTION_MOVE,lpExistingFileName,lpNewFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpExistingFileName,lpNewFileName);
}/* myMoveFileW */

static BOOL WINAPI myMoveFileExW( LPCTSTR lpExistingFileName,
				  LPCTSTR lpNewFileName,
				  DWORD dwFlags )
{
  adapter::SetAdapterState("kernel32.dll","MoveFileExW");

  _ftprintf(stdout, _T("MoveFileExW: %s -> %s\n"), lpExistingFileName, lpNewFileName);

  if( cesdk_wrapper::Eval(CE_ACTION_MOVE,lpExistingFileName,lpNewFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpExistingFileName,lpNewFileName,dwFlags);
}/* myMoveFileEx */

static BOOL WINAPI myMoveFileWithProgressW( LPCTSTR lpExistingFileName,
					    LPCTSTR lpNewFileName,
					    LPPROGRESS_ROUTINE lpProgressRoutine,
					    LPVOID lpData,
					    DWORD dwFlags )
{
  adapter::SetAdapterState("kernel32.dll","MoveFileWithProgressW");

  _ftprintf(stdout, _T("MoveFileWithProgressW: %s -> %s\n"), lpExistingFileName, lpNewFileName);

  if( cesdk_wrapper::Eval(CE_ACTION_MOVE,lpExistingFileName,lpNewFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpExistingFileName,lpNewFileName,lpProgressRoutine,lpData,dwFlags);
}/* myMoveFileWithProgressW */

static BOOL WINAPI myCopyFileW( LPCTSTR lpFileName,
				LPCTSTR lpNewFileName,
				BOOL bFailIfExists )
{
  adapter::SetAdapterState("kernel32.dll","CopyFileW");

  _ftprintf(stdout, _T("CopyFileW: %s -> %s : replace %d\n"),
	    lpFileName, lpNewFileName, bFailIfExists);

  if( cesdk_wrapper::Eval(CE_ACTION_COPY,lpFileName,lpNewFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpFileName,lpNewFileName,bFailIfExists);
}/* myCopyFileW */

static BOOL WINAPI myCopyFileExW( LPCTSTR lpExistingFileName,
				  LPCTSTR lpNewFileName,
				  LPPROGRESS_ROUTINE lpProgressRoutine,
				  LPVOID lpData,
				  LPBOOL pbCancel,
				  DWORD dwCopyFlags )
{
  adapter::SetAdapterState("kernel32.dll","CopyFileExW");

  _ftprintf(stdout, _T("CopyFileExW: %s -> %s\n"), lpExistingFileName, lpNewFileName);

  if( cesdk_wrapper::Eval(CE_ACTION_COPY,lpExistingFileName,lpNewFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpExistingFileName,lpNewFileName,lpProgressRoutine,
			     lpData,pbCancel,dwCopyFlags);
}/* myCopyFileExW */

static BOOL WINAPI myDeleteFileW( LPCTSTR lpFileName )
{
  adapter::SetAdapterState("kernel32.dll","DeleteFileW");

  _ftprintf(stdout, _T("DeleteFileW: %s\n"), lpFileName);

  if( cesdk_wrapper::Eval(CE_ACTION_DELETE,lpFileName) == false )
  {
    adapter::SetDenyReturnValue<BOOL>(FALSE);
  }

  return adapter::Next<BOOL>(lpFileName);
}/* myDeleteFileW */

HANDLE WINAPI myCreateFileW( LPCTSTR lpFileName,
			     DWORD dwDesiredAccess,
			     DWORD dwShareMode,
			     LPSECURITY_ATTRIBUTES lpSecurityAttributes,
			     DWORD dwCreationDisposition,
			     DWORD dwFlagsAndAttributes,
			     HANDLE hTemplateFile )
{
  adapter::SetAdapterState("kernel32.dll","CreateFileW");

  //_ftprintf(stdout, _T("%s : access 0x%x\n"), lpFileName, dwDesiredAccess);
  /*
  if( lpFileName && cesdk_wrapper::Eval(CE_ACTION_READ,lpFileName) == false )
  {
    adapter::SetDenyReturnValue<HANDLE>(INVALID_HANDLE_VALUE);
  }
  */
  return adapter::Next<HANDLE>(lpFileName,
			       dwDesiredAccess,
			       dwShareMode,
			       lpSecurityAttributes,
			       dwCreationDisposition,
			       dwFlagsAndAttributes,
			       hTemplateFile);

}/* myCreateFileW */

/***********************************************************************
 * Adapter entry point.  Initialize state and add hooks.  This symbol
 * must be exported.
 **********************************************************************/
extern "C" int AdapterEntry(void)
{
  fprintf(stderr, "utest: AdapterEntry\n");

  bool bresult;

  bresult = cesdk_wrapper::Init();

  /* MoveFileXXX */
  bresult = adapter::AddHook("kernel32.dll","MoveFileW",myMoveFileW);
  bresult = adapter::AddHook("kernel32.dll","MoveFileExW",myMoveFileExW);
  bresult = adapter::AddHook("kernel32.dll","MoveFileWithProgressW",myMoveFileWithProgressW);

  /* CopyFileXXX */
  bresult = adapter::AddHook("kernel32.dll","CopyFileW",myCopyFileW);
  bresult = adapter::AddHook("kernel32.dll","CopyFileExW",myCopyFileExW);

  /* DeleteFile */
  bresult = adapter::AddHook("kernel32.dll","DeleteFileW",myDeleteFileW);

  //bresult = adapter::AddHook("kernel32.dll","CreateFileW",myCreateFileW);

  /* When initialization fails the return value will force the unloading
     of the hook library.
  */
  if( bresult == false )
  {
    fprintf(stderr, "utest: AdapterEntry failed\n");
    return -1;
  }

  return 0;
}/* AdapterEntry */
