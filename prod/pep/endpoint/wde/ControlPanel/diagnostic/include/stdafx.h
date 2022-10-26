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



// TODO: reference additional headers your program requires here
#include <atlbase.h>

#pragma warning(push)
#pragma warning(disable:6386)
#pragma warning(disable:6387)
#include <atlwin.h>
#pragma warning(pop)

#include <assert.h>

extern HINSTANCE g_hInstance; 

#include "utilities.h"
extern CELog g_log;

#include "commctrl.h"

extern HFONT g_hFont;
extern HFONT g_hSmallFont;
extern const wchar_t* g_szFont;

#define EVENT_SETTING_LOG			L"NXTLBS_SETTING_LOG"
#define EVENT_STARTING_PC			L"NXTLBS_STARTING_PC"