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
#include "stdafx.h"
//---------------------------------------------------------------------------
//
// Enumerate
// 
//---------------------------------------------------------------------------
enum DetourPriority {LOW, MEDIUM, HIGH};
//---------------------------------------------------------------------------
//
// Typedefs
// 
//---------------------------------------------------------------------------
typedef struct {
	WCHAR  *dllName; //name of Win DLL
	WCHAR  *funcName; //name of hooked Win API
	WCHAR  *preDetourFunc; //detour function that is called before real API
	WCHAR  *postDetourFunc;//detour function that is called after real API
	DetourPriority priority; //the priority of the detour
} HookDetour;

typedef struct _DetourItemTag{
	std::wstring name; //The name of function where the WinAPI detour to
	PVOID func; //the pointer to the detour function
	DetourPriority priority; //the priority of the detour

	_DetourItemTag(const WCHAR *n, PVOID f, DetourPriority p):
		name(n), func(f), priority(p){}
} DetourItem;

class APIDetours {
	std::wstring dllName; //name of Win DLL
	std::wstring funcName; //name of hooked Win API

	HANDLE preDetourMutex; //The mutex for preDetourFuncs
	std::multimap<DetourPriority, DetourItem *> preDetourFuncs; //The array of pre detour functions

	PVOID realFunc; //The original WinAPI

	HANDLE postDetourMutex; //The mutex for postDetourFuncs
	std::multimap<DetourPriority, DetourItem *> postDetourFuncs; //The array of post detour functions
	friend class CModuleScope;

public:

	APIDetours(const WCHAR *dll, const WCHAR *func, PVOID real);
	~APIDetours();

	bool AddOneDetour(const WCHAR *preDetourName, 
					PVOID preDetourPtr, 
					DetourPriority p,
					const WCHAR *postDetourName,
					PVOID postDetourPtr);
};

#endif // !defined(_DETOURS_H_)
