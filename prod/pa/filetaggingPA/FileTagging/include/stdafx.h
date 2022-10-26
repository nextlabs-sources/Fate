// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#endif 
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#pragma warning(push)
#pragma warning(disable:6387 )
#include <atlbase.h>
#include <atlstr.h>
#include <list>
#pragma warning(pop)

#include "PABase.h"
#pragma warning(push)
#pragma warning(disable:6386)
#include <atlwin.h>
#pragma warning(pop)

#include <string>
#include <vector>
using std::wstring;
using std::vector;
using std::pair;

#include "smart_ptr.h"
using namespace YLIB;

#include <gdiplus.h>
#include "atlimage.h"
#include "FileTagMgr.h"
extern UINT g_MainWindowMsgID;
extern wchar_t g_szSeparateTagValues[2];
extern wchar_t g_szSeparator_IndexTag[2];
extern HINSTANCE g_hInstance;


#define   NXTLBS_INDEX_TAG				L"NXTLBS_TAGS"

#ifdef _WIN64
	#define   FILETAGGING_DLL_NAME			L"pa_filetagging.dll"
#else
	#define   FILETAGGING_DLL_NAME			L"pa_filetagging32.dll"
#endif
// TODO: reference additional headers your program requires here
