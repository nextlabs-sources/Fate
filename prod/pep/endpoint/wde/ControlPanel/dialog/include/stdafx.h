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

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#pragma warning(push)
#pragma warning(disable: 6387 6386 6011)
#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:6387)
#pragma warning(disable:6386)
#pragma warning(disable:4996)
#pragma warning(disable:6011 6386 6400 6211 6401)
#include <atlbase.h>
#include <atlwin.h>
#include <atlctl.h>
#include "atlapp.h"
#include "atlwinx.h"
#include "atlctrls.h"
#include "atlctrlx.h"
#pragma warning(pop)

#include "Utils.h"
// TODO: reference additional headers your program requires here

extern HINSTANCE g_hInst;

#include "utilities.h"
extern CELog g_log;

#include "Actions.h"


extern const wchar_t* g_szFont;
extern const int g_nFontSize;
extern const int g_nFontSizeBig;
extern const COLORREF  g_clrHighlight;

#define WM_NXTLBS_MYSUBMIT WM_USER + 250//This message will be sent to all child dialog when user clicks "OK" on main dialog
#define WM_SHOWNOTIFICATION WM_USER + 251

#define EVENT_STARTING_PC			L"NXTLBS_STARTING_PC"
#define EVENT_SETTING_LOG			L"NXTLBS_SETTING_LOG"
#define EVENT_STOPPING_PC			L"NXTLBS_STOPPING_PC"
