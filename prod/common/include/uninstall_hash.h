#include <stdlib.h>
#include <windows.h>
#pragma once

#ifdef UNINSTALL_HASH_EXPORTS
    #define UNINSTALL_HASH_EXPORT __declspec( dllexport)
#else
    #define UNINSTALL_HASH_EXPORT __declspec( dllimport)
#endif

#define HASH_BUFFER_TOO_SMALL -1
#define HASH_BUFFER_ERROR -2

UINT WINAPI  hashChallenge (LPCTSTR challenge, LPTSTR respBuf, size_t &respBufSize);
