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
#include "Ws2spi.h"
#include "log.h"
#include "Sporder.h"
#include <Sddl.h>
#include "CEsdk.h"
#include "Error.h"
#include "Lock.h"
#include "baseImpl.h"
#include "Utilities.h"
#ifndef TIMEOUT_TIME
#define TIMEOUT_TIME                    30000
#endif




// TODO: reference additional headers your program requires here
