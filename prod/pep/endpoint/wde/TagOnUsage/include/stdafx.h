// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include "resource.h"

#pragma warning( push )
#pragma warning(disable: 6387)
#pragma warning(disable: 6011)
#pragma warning( disable: 6386 )
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning( pop )

#include <list>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "Utils.h"
#include "CriSectionMgr.h"
#include "nlconfig.hpp"
#include "celog.h"

#define WM_UPDATE_ITEM_COUNT	WM_USER + 100
#define NAME_MUTEX_WRITE_SHAREDMEMORY		L"NXTLBS_WRITE_SHAREDMEMORY_1027"
#define EVENT_CMD_RECEIVED					L"NXTLBS_COMMAND_RECEIVED"

using namespace ATL;

extern CELog g_log;
