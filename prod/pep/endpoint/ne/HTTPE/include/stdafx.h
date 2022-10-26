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
#include <list>
#include "ModulVer.h"

#pragma warning(push)
#pragma warning(disable: 4505)
#include "nlexcept.h"
#pragma warning(pop)

#include <WinSock2.h>
#include "Ws2spi.h"
#include "Sporder.h"
#include <string>
#include "nlconfig.hpp"
#include "celog.h"
#include "Utilities.h"
#include "ShlObj.h"

using namespace std;

extern CELog g_log;
extern HMODULE g_hMod;

#define		SHARED_MEMORY_NAME_DOWNLOAD_EMBEDDED_EXPLORER		L"download_with_embedded_explorer_nxtlabs"
#define		MAX_FILEDATA_SIZE		4096
#define		MAX_URL_SIZE			1024
#define		HTTP_OPEN				L"OPEN"
#define		HTTP_UPLOAD				L"UPLOAD"

#define HTTP_VERSION_1_0 "HTTP/1.0 "
#define HTTP_VERSION_1_1 "HTTP/1.1 "
// TODO: reference additional headers your program requires here
