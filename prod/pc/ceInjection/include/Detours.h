/*=========================Detours.h========================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo City CA, Ownership remains with NextLabs Inc,                  *
 * All rights reserved worldwide.                                           *
 *                                                                          *
 * Author : Heidi Zhou                                                      *
 * Date   : 10/18/2007                                                      *
 * Note   : Declare some data structures for managing detour functions.     *
 *==========================================================================*/
#if !defined(_DETOURS_H_)
#define _DETOURS_H_

#include <windows.h>
#include <tchar.h>
#include <map>

//---------------------------------------------------------------------------
// Enumerate
//---------------------------------------------------------------------------
enum DetourPriority {LOW, MEDIUM, HIGH};

//---------------------------------------------------------------------------
// Typedefs
//---------------------------------------------------------------------------
typedef struct {
  int level;              // Level to install at
  WCHAR hook_libname[MAX_PATH];    // Hook library name (e.g. mykernel32.dll)
  WCHAR target_libname[MAX_PATH];  // Target library name (e.g. kernel32.dll)
  char  *funcName;        // Target function name (e.g. CreateFileW)
  char  *preDetourFunc;   // detour function that is called before real API
  char  *postDetourFunc;  // detour function that is called after real API
} HookDetour;

typedef struct _DetourItemTag{
	std::string name; //The name of function where the WinAPI detour to
	PVOID func; //the pointer to the detour function

	_DetourItemTag(const char *n, PVOID f):
		name(n), func(f){}
} DetourItem;

class APIDetours {
	std::multimap<DetourPriority, DetourItem *> preDetourFuncs; //The array of pre detour functions

	std::multimap<DetourPriority, DetourItem *> postDetourFuncs; //The array of post detour functions
	friend class CModuleScope;

public:

	~APIDetours();

	bool AddOneDetour(const char *preDetourName, 
			  PVOID preDetourPtr, 
			  const char *postDetourName,
			  PVOID postDetourPtr);
};

#endif // !defined(_DETOURS_H_)
