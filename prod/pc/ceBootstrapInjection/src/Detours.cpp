/*=========================Detours.cpp======================================*
 *                                                                          *
 * All sources, binaries and HTML pages (C) copyright 2007 by NextLabs,     *
 * San Mateo City CA, Ownership remains with NextLabs Inc,                  *
 * All rights reserved worldwide.                                           *
 *                                                                          *
 * Author : Heidi Zhou                                                      *
 * Date   : 10/18/2007                                                      *
 * Note   : Define data structures for managing detour functions.           *
 *==========================================================================*/
#include "Detours.h"

extern PVOID NextCoCreateInstance;
extern PVOID NextGetClipboardData;
extern PVOID NextSetClipboardData;

APIDetours::APIDetours(const WCHAR *dll, const WCHAR *func, PVOID real):dllName(dll), 
	funcName(func), realFunc(real)
{
  preDetourMutex=CreateMutex(NULL, false, NULL);
  postDetourMutex=CreateMutex(NULL, false, NULL);
}

APIDetours::~APIDetours()
{

	std::multimap<DetourPriority, DetourItem *>::iterator it;
	WaitForSingleObject(preDetourMutex, INFINITE);
	for(it=preDetourFuncs.begin(); it != preDetourFuncs.end(); it++)
		delete it->second;
	ReleaseMutex(preDetourMutex);

	WaitForSingleObject(postDetourMutex, INFINITE);
	for(it=postDetourFuncs.begin(); it != postDetourFuncs.end(); it++)
		delete it->second;
	ReleaseMutex(postDetourMutex);

	CloseHandle(preDetourMutex);
	CloseHandle(postDetourMutex);
}

bool APIDetours::AddOneDetour(const WCHAR *preDetourName, 
							PVOID preDetourPtr, 
							DetourPriority p,
							const WCHAR *postDetourName,
							PVOID postDetourPtr)
{
	if(preDetourName && preDetourPtr!=NULL) {
		DetourItem *pd=new DetourItem(preDetourName, preDetourPtr, p);
		WaitForSingleObject(preDetourMutex, INFINITE);
		preDetourFuncs.insert(std::pair<DetourPriority, DetourItem*>(p, pd));
		ReleaseMutex(preDetourMutex);
	}

	if(postDetourName && postDetourPtr!=NULL) {
		DetourItem *pd=new DetourItem(postDetourName, postDetourPtr, p);
		WaitForSingleObject(postDetourMutex, INFINITE);
		postDetourFuncs.insert(std::pair<DetourPriority, DetourItem*>(p, pd));
		ReleaseMutex(postDetourMutex);
	}
	return true;
}

