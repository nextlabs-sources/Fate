

#pragma once

#ifndef _INCLUDE_BJOLHOOK
#define _INCLUDE_BJOLHOOK

#define ACT_HOOK    true
#define ACT_UNHOOK  false
#define HOOK_UNHOOK_FUNC(act, name)\
    if(act)\
    DetourFunctionWithTrampoline((PBYTE)real_##name, (PBYTE)hooked_##name);\
    else\
    DetourRemove((PBYTE)real_##name, (PBYTE)hooked_##name);

#define NLCALLNEXTCREATEFILEFUNCTION CallNextCreateFileFunction

void StartStopHook(bool bStart);

HANDLE CallNextCreateFileFunction(LPCTSTR lpFileName,DWORD dwDesiredAccess,
								  DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile);


#endif