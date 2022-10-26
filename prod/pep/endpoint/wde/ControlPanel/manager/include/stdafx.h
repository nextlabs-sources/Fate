// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>
#include <shellapi.h>



// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit




// TODO: reference additional headers your program requires here
#include "Resource.h"

#pragma warning(push)
#pragma warning(disable:6387 4005 6244 6386 4996 6011)

#include <atlbase.h>
#include <atlwin.h>
#include <atlctl.h>
#include "atlapp.h"
#include "atlwinx.h"
#include "atlctrls.h"
#pragma warning(pop)
using namespace ATL;

extern HINSTANCE g_hInstance;
extern HWND g_hMainWnd;
extern HWND g_hNotifyMsgLoop;

#define NOTIFY_RECEIVE_MSG			WM_USER + 101


#ifdef _USRDLL
#define DllExport __declspec( dllexport)
#else
#define DllExport __declspec( dllimport)
#endif // _AFXDLL


#include "utilities.h"
#include "EDPMgr.h"