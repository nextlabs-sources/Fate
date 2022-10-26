// TagView.cpp : Implementation of DLL Exports.

//
// Note: COM+ 1.0 Information:
//      Please remember to run Microsoft Transaction Explorer to install the component(s).
//      Registration is not done by default. 

#include "stdafx.h"
#include "resource.h"
#include "TagView.h"
#include "compreg.h"
#include <list>

HINSTANCE g_hInstance = NULL;
std::wstring g_strFileTaggingDllPath = L"";

#define FILETAGGING_INSTALL_PATH_REGKEY			L"SOFTWARE\\Nextlabs\\FileTagging"
#define FILETAGGING_INSTALL_PATH				L"path"
#define FILETAGGING_NEXTLABS_REGKEY				L"SOFTWARE\\Nextlabs"
#define FILETAGGING_TAGVIEW_SUBKEY				L"TagView"
#define FILETAGGING_TAGVIEW_REGKEY				L"SOFTWARE\\Nextlabs\\TagView"	

#define FILETAGGING_TAGVIEW_COUNT				L"count"
class CTagViewModule : public CAtlDllModuleT< CTagViewModule >
{
public :
	DECLARE_LIBID(LIBID_TagViewLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TAGVIEW, "{533A91A0-62AC-4F5D-BD48-6F4201ECBCF3}")
};

CTagViewModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
	g_hInstance = hInstance;

	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			wchar_t szBuffer[MAX_PATH + 1] = {0};
			GetModuleFileNameW(hInstance, szBuffer, MAX_PATH);
			wchar_t* p = wcsrchr(szBuffer, '\\');
			if(p)
				*p = '\0';

			g_strFileTaggingDllPath = std::wstring(szBuffer);
			
		}
		break;
	}
		

    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

