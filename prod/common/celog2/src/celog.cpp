/*************************************************************************
 *
 * Compliant Enterprise Logging
 *
 * Implementation file for logging interface described in celog.h
 *
 ************************************************************************/

#include <Windows.h>
#include "celog.h"
#include "celog_mgr.h"

extern CELogMgr mgr;

BOOL WINAPI DllMain(_In_  HINSTANCE hinstDLL, _In_  DWORD fdwReason,
                    _In_  LPVOID lpvReserved)
{
  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    return mgr.Init();

  case DLL_PROCESS_DETACH:
    mgr.Destroy();
    return TRUE;

  default:
    return TRUE;
  }
}

_Check_return_
bool CELog_IsPerfLogMode(void)
{
  return mgr.IsPerfLogMode();
}

int CELog_LogA(_In_     celog_filepathint_t fileInt,
               _In_     int line,
               _In_z_   const wchar_t *mod,
               _In_     celog_level_t level,
               _In_z_ _Printf_format_string_ const char *fmt,
               _In_     ...)
{
  va_list argPtr;
  int ret;

  va_start(argPtr, fmt);
  ret = mgr.Log(fileInt, line, mod, level, fmt, argPtr);
  va_end(argPtr);

  return ret;
}

int CELog_LogW(_In_     celog_filepathint_t fileInt,
               _In_     int line,
               _In_z_   const wchar_t *mod,
               _In_     celog_level_t level,
               _In_z_ _Printf_format_string_ const wchar_t *fmt,
               _In_     ...)
{
  va_list argPtr;
  int ret;

  va_start(argPtr, fmt);
  ret = mgr.Log(fileInt, line, mod, level, fmt, argPtr);
  va_end(argPtr);

  return ret;
}
