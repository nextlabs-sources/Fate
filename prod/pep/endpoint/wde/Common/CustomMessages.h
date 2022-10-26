/*
 * CustomMessages.h 
 * Author: Helen Friedland
 * All sources, binaries and HTML pages (C) copyright 2004 by Blue Jungle Inc., 
 * Redwood City CA, Ownership remains with Blue Jungle Inc, 
 * All rights reserved worldwide. 
 */


#ifndef _CUSTOMMESSAGES_H_
#define _CUSTOMMESSAGES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UWM_HOOKTOOL_DLL_LOADED_MSG \
	"UWM_HOOKTOOL_DLL_LOADED - {68D9B79A-09E0-4e20-9273-767C8813CA1F}"
#define UWM_HOOKTOOL_DLL_UNLOADED_MSG \
	"UWM_HOOKTOOL_DLL_UNLOADED - {68D9B79A-09E0-4e20-9273-767C8813CA1F}"

const UINT UWM_HOOKTOOL_DLL_LOADED = 
	::RegisterWindowMessageA(UWM_HOOKTOOL_DLL_LOADED_MSG);
const UINT UWM_HOOKTOOL_DLL_UNLOADED = 
	::RegisterWindowMessageA(UWM_HOOKTOOL_DLL_UNLOADED_MSG);


#endif //_CUSTOMMESSAGES_H_

//--------------------- End of the file -------------------------------------
