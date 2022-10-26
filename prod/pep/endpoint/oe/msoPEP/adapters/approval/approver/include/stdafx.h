// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

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

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#pragma warning(disable: 4278 4584)

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include "../YLIB/log.h"
#include "../YLIB/smart_ptr.h"
#include <mapi.h>


using namespace ATL;
#ifdef OUTLOOK_2007
#include "../import/2k7/mso.tlh"
using namespace Office;
#include "../import/2k7/msaddndr.tlh"
#include "../import/2k7/msoutl.tlh"
//using namespace Outlook;
#else
#include "../import/2k3/mso.tlh"
using namespace Office;
#include "../import/2k3/msaddndr.tlh"
#include "../import/2k3/msoutl.tlh"
//#import "./vbscript.tlb" rename_namespace("VBScriptRegEx")
//using namespace Outlook;
#endif

//#ifdef OUTLOOK_2007
//#import "../import/2k7/MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search
//#import "../import/2k7/mso.dll" rename_namespace("Office"), named_guids
//using namespace Office;
//#import "../import/2k7/msoutl.olb" rename_namespace("Outlook") raw_interfaces_only, named_guids 
////using namespace Outlook;
//#else
//#import "../import/2k3/MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids, auto_search
//#import "../import/2k3/mso.dll" rename_namespace("Office"), named_guids
//using namespace Office;
//#import "../import/2k3/msoutl.olb" rename_namespace("Outlook") raw_interfaces_only, named_guids 
//using namespace Outlook;
//#endif
