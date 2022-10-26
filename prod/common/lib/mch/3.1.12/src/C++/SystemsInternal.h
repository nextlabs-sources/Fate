// ***************************************************************
//  SystemsInternal.h         version: 1.0.0  ·  date: 2010-01-10
//  -------------------------------------------------------------
//  defines shared types and functions internal to the lib
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2010 www.madshi.net, All Rights Reserved
// ***************************************************************

#ifndef _SYSTEMSINTERNAL_H
#define _SYSTEMSINTERNAL_H

#pragma warning(disable: 4127)  // conditional expression is constant

#include "Tracing.h"

#include "StringConstants.h"
#include "AtomicMove.h"

#include "FunctionTypes.h"
#include "Exception.h"

#include "ObjectTools.h"
#include "ModuleTools.h"
#include "ProcessTools.h"

#include "HookingTypes.h"
#include "DriverInject.h"
#include "Stubs.h"
#include "Patches.h"

#include "Inject.h"
#include "Hooking.h"

#include "CCodeHook.h"
#include "CCollectCache.h"

#include "IPC.h"

#include "DllMain.h"

#define SECTION L"Systems"

#endif