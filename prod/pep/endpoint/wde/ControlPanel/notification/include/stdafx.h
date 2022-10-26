// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define ISOLATION_AWARE_ENABLED 1

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>

#pragma warning(push)
#pragma warning(disable: 6387 6011 6386 6244)
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <shellapi.h>
#pragma warning(pop)

using namespace ATL;

// TODO: reference additional headers your program requires here
#include "resource.h"

extern HMODULE g_hInst;

//	celog
#include "celog.h"

extern CELog g_log;

extern const wchar_t* g_szFont;
