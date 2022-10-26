/*********************************************************************************************
 *
 * MadCodeHook include file helper.
 *
 * This file will include the appropriate MadCodeHook library and provide an error
 * condition if the platform is undefined.
 *
 ********************************************************************************************/

#ifndef __madCHook_helper_h__
#define __madCHook_helper_h__

#include <madCHook.h>

#if defined(_WIN64)
#pragma comment(lib,"madCHook64.lib")
#elif defined (_WIN32)
#pragma comment(lib,"madCHook32.lib")
#else
#error Unsupported platform.
#endif

#endif /* __madCHook_helper_h__ */
