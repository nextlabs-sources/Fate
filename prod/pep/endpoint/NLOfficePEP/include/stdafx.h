// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif



#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#define MINI_OFFICEPEP	0
#include "resource.h"

// C
#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <process.h>
// C++
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <fstream>
using namespace std;

// Windows
// Note: WinSock2.h must be included before windows.h
#include <WinSock2.h> 
#include <Windows.h>

// Boost
#pragma warning( push )
#pragma warning( disable: 4244 4267 4512 4996 6011 6031 6258 6386 6385 6328 6309 6387 6334  )
#include "boost/algorithm/string.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>	// debug version this is warning: 6011
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/noncopyable.hpp>
#pragma warning( pop )


// ATL&COM&SHELL
#pragma warning( push )
#pragma warning( disable: 6387 6011 )
#include <Shellapi.h> 
#include <OAIdl.h>
#include <shlobj.h>
#include <ShObjIdl.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning( pop )

// Nextlabs common
#include "nlconfig.hpp"

// Office 2k7
#include "../import/2k7/mso.tlh"
using namespace Office;

#include "../import/2k7/vbe6ext.tlh"
using namespace VBE6;

#include "../import/2k7/msword.tlh"
using namespace Word;

#include "../import/2k7/excel.tlh"
using namespace Excel;

#include "../import/2k7/msppt.tlh"
using namespace PPT;

using namespace ATL;
#include "../import/2k7/msaddndr.tlh"

// Safe string operator
// Note: strsafe.h should be include after tchar.h, windows.h, atlbase.h ...
#include <strsafe.h>