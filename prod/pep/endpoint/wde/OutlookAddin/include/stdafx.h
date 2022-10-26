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
#pragma warning( disable : 6387 6011)
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning( pop )
using namespace ATL;
