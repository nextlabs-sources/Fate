// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>   
#include <cassert>
#include <string>
#include "Ws2spi.h"
#include "Sporder.h"
#include <Sddl.h>
#include <WinSock2.h>
#include "oaidl.h"
#include "ocidl.h"
#include "log.h"
#include "Lock.h"
#include "Error.h"
#include "baseImpl.h"
#include "ModulVer.h"
#include "nlexcept.h"
#include "SPIInstaller.h"
#include "criticalMngr.h"
#include "CEsdk.h"
#include "eframework/platform/cesdk_loader.hpp"

#ifndef TIMEOUT_TIME
#define TIMEOUT_TIME                    30000
#endif

// TODO: reference additional headers your program requires here
