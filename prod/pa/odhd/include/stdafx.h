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
#define _WIN32_WINNT 0x0502	// Change this to the appropriate value to target other versions of Windows.
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



#pragma warning(disable : 4996 4584 4311 4312) //4278 
#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <vector>
#include <Sddl.h>
#include <WinSock2.h>
#pragma warning(push)
#pragma warning(disable: 6386)
#include <Ws2tcpip.h>
#pragma warning(pop)
#include <shlobj.h>

/*
REMOVEFAIL: Remove Hidden data failed
NOHD:	No Hidden data found
HAVEHD:	Found hidden data
REMOVEOK:Hidden data be removed succeeded
INSPECTFAIL:Detect hidden data failed
This definition should be consistent with enum ODHDSTATUS in odhd.h
*/
enum {REMOVEFAIL=-1,NOHD=0, HAVEHD=1, REMOVEOK=2, INSPECTFAIL=3 };

using namespace ATL;

#define OCP_REALPATH					L"CE_REALPATH"
#define OCP_TEMPPATH					L"CE_TEMPPATH"

/************************************************************************/
/* Add Office Import FOR OFFICE 2007                                    */
/************************************************************************/
#ifdef WSO2K7
#include "2k7/mso.tlh"
using namespace Office;

#include "2k7/msaddndr.tlh"

#include "2k7/vbe6ext.tlh"
using namespace VBE6;

#include "2k7/msword.tlh"
using namespace Word;

#include "2k7/excel.tlh"
using namespace Excel;

#include "2k7/msppt.tlh"
using namespace PPT;

#include "2k7/msoutl.tlh"
using namespace Outlook;
#endif

/************************************************************************/
/* Add Office Import FOR OFFICE 2003                                    */
/************************************************************************/
#ifdef WSO2K3
#include "2k3/mso.tlh"
using namespace Office;

#include "2k3/msaddndr.tlh"
#include "2k3/vbe6ext.tlh"
using namespace VBE6;

#include "2k3/msword.tlh"
using namespace Word;

#include "2k3/excel.tlh"
using namespace Excel;

#include "2k3/msppt.tlh"
using namespace PPT;

#include "2k3/msoutl.tlh"
using namespace Outlook;
#endif

/************************************************************************/
/* Add Office Import FOR OFFICE 2010                                    */
/************************************************************************/
#if (defined WSO2010) || (defined WSO2013)
#include "2010/mso.tlh"
using namespace Office;

#include "2010/msaddndr.tlh"
#include "2010/vbe6ext.tlh"
using namespace VBIDE;

#include "2010/msword.tlh"
using namespace Word;

#include "2010/excel.tlh"
using namespace Excel;

#include "2010/msppt.tlh"
using namespace PPT;

#include "2010/msoutl.tlh"
using namespace Outlook;
#endif