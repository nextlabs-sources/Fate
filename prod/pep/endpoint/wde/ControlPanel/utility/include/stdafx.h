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

//	celog instance used by edpmgrutilities
#include "utilities.h"
extern CELog g_log;

//	regedit path for edp manager
#define EDPM_REG_DIR L"Software\\Nextlabs\\EDP Manager"
#define EDPM_REG_ROOT_DIR L"Software\\Nextlabs"
#define EDPM_REG_SOFTWARE_ROOT_DIR L"Software"		//	may be used later
#define EDPM_REG_SUBROOT L"EDP Manager"
#define EDPM_REG_NXTLABS L"Nextlabs"
#define EDPM_REG_VERBOSE_LOG L"VerboseLogEnabled"
#define EDPM_REG_NOTIFY_LEVEL L"NotifyLevel"
#define EDPM_REG_NOTIFY_DURATION L"NotifyDuration"
#define EDPM_REG_NOTIFY_MAX L"MaxNotifyCache"
//	headers for atl
#include <atlbase.h>

#pragma warning(push)
#pragma warning(disable:6386)
#pragma warning(disable:6387)
#include <atlwin.h>
#pragma warning(pop)
