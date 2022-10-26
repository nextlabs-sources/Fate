// All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
// Redwood City CA, Ownership remains with Blue Jungle Inc, 
// All rights reserved worldwide. 

#ifndef _globals_h_
#define _globals_h_

#include <string.h>
#include <string>
#include <vector>
#include <list>
#include <map>

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

// Windows Header Files:
#include <windows.h>


#ifdef SHARED_EXPORTS
#    define DECLSPECIFIER __declspec(dllexport)
#    define EXPIMP_TEMPLATE
#else
#    define DECLSPECIFIER __declspec(dllimport)
#    define EXPIMP_TEMPLATE extern
#endif

//define tstring
#ifdef _UNICODE
    #define tstring std::wstring
#endif

typedef std::vector<TCHAR*> StringVector;
typedef std::vector<void*> PtrVector;
typedef std::list<void*> PtrList;
typedef std::list<TCHAR*> StringList;
typedef std::map<void*, void*> PtrToPtrMap;

#define CONTROLMGR_STUB_CLASS "com/bluejungle/destiny/agent/controlmanager/ControlManagerStub"
#define CM_STUB_GETINSTANCE_M "getInstance"

#endif
