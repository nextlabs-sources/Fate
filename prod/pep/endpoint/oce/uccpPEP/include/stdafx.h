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



#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#define _ATL_ALL_WARNINGS
#include <atlbase.h>
#include <atlcom.h>
#include <objbase.h>
using namespace ATL;

#pragma warning(disable : 4278 4584)

#include "nlexcept.h"

#import "..\uccapi.dll" rename_namespace("UCCP") \
	raw_interfaces_only, raw_native_types, named_guids, auto_search
using namespace UCCP;

#include "uccapi.tlh"	//	this file is generated from "..\uccapi.dll"

typedef unsigned (__stdcall *PTHREAD_START) (void*);

#define CREAT_THREAD(psa,cbStack,pfnStartAddr,	\
	pvParam, fdwCreate, pdwThreadID)			\
	((HANDLE) _beginthreadex(					\
	(void*)(psa),								\
	(unsigned) (cbStack),						\
	(PTHREAD_START) (pfnStartAddr),				\
	(void*)(pvParam),							\
	(unsigned)(fdwCreate),						\
	(unsigned*)(pdwThreadID)))

#include <map>
#include <set>
#include <string>
#include <list>

#include "nlconfig.hpp"

