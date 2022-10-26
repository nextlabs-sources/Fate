#include <windows.h>
#include <new>
#include <shlwapi.h>
#include <stdio.h>

#define SZ_NXLFILTER_CLSID L"{38522F37-C617-4438-BA5E-77BA8B655237}"
#define SZ_NXLFILTER_HANDLER L"{EFD15EFF-FED6-45CC-83C4-4DAAA4CAAC9C}"

#ifdef _X86_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='x86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#else ifdef _AMD64_
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='amd64' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"") 
#endif


HRESULT CNXLFilter_CreateInstance(REFIID riid, void **ppv);

// Handle to the DLL's module
HINSTANCE g_hInst = NULL;

// Module Ref count
long c_cRefModule = 0;
				
void DllAddRef()
{
    InterlockedIncrement(&c_cRefModule);
}

void DllRelease()
{
    InterlockedDecrement(&c_cRefModule);
}

//Similar to any COM object, filter DLLs need a class factory to create instances of the filter. 
class CClassFactory : public IClassFactory
{
public:
    CClassFactory(REFCLSID clsid) : m_cRef(1), m_clsid(clsid)
    {
        DllAddRef();
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CClassFactory, IClassFactory),
            { 0 }
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IClassFactory
    IFACEMETHODIMP CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
    {
        *ppv = NULL;
        HRESULT hr;
        if (punkOuter)
        {
            hr = CLASS_E_NOAGGREGATION;
        }
        else
        {
            CLSID clsid;
            if (SUCCEEDED(CLSIDFromString(SZ_NXLFILTER_CLSID, &clsid)) && IsEqualCLSID(m_clsid, clsid))
            {
                hr = CNXLFilter_CreateInstance(riid, ppv);
            }
            else
            {
                hr = CLASS_E_CLASSNOTAVAILABLE;
            }
        }
        return hr;
    }

    IFACEMETHODIMP LockServer(BOOL bLock)
    {
        if (bLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }
        return S_OK;
    }

private:
    ~CClassFactory()
    {
        DllRelease();
    }

    long m_cRef;
    CLSID m_clsid;
};

// Standard DLL functions
STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    return (c_cRefModule == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    *ppv = NULL;
    CClassFactory *pClassFactory = new (std::nothrow) CClassFactory(clsid);
    HRESULT hr = pClassFactory ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
        pClassFactory->Release();
    }
    return hr;
}

// A struct to hold the information required for a registry entry
struct REGISTRY_ENTRY
{
    HKEY    hkeyRoot;
    PCWSTR pszKeyName;
    PCWSTR pszValueName;
    PCWSTR pszData;
};

// Creates a registry key (if needed) and sets the default value of the key
HRESULT CreateRegKeyAndSetValue(const REGISTRY_ENTRY *pRegistryEntry,DWORD dwRegType = REG_SZ)
{
	HRESULT hr;
	HKEY hKey;

	LONG lRet = RegCreateKeyExW(pRegistryEntry->hkeyRoot, pRegistryEntry->pszKeyName,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (lRet != ERROR_SUCCESS)
	{
		hr = HRESULT_FROM_WIN32(lRet);
	}
	else
	{
		BYTE* pValue = (BYTE*)pRegistryEntry->pszData;
		if (dwRegType == REG_DWORD)
		{
			DWORD dwValue = _wtoi(pRegistryEntry->pszData);
			pValue = (BYTE*)&dwValue;
		}
		lRet = RegSetValueExW(hKey, pRegistryEntry->pszValueName, 0, dwRegType,
			(LPBYTE)pValue,
			((DWORD)wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR));
		hr = HRESULT_FROM_WIN32(lRet);
		RegCloseKey(hKey);
	}
	return hr;
}

// Registers this COM server
STDAPI DllRegisterServer()
{
    HRESULT hr;
    WCHAR szModuleName[MAX_PATH];
    if (!GetModuleFileNameW(g_hInst, szModuleName, ARRAYSIZE(szModuleName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        // List of registry entries we want to create
        const REGISTRY_ENTRY rgRegistryEntries[] =
        {
            // RootKey             KeyName                                                                                  ValueName           Data
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_CLSID,                                     NULL,               L"NXL Filter"},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_CLSID L"\\InProcServer32",                 NULL,               szModuleName},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_CLSID L"\\InProcServer32",                 L"ThreadingModel",  L"Both"},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_HANDLER,                                   NULL,               L"NXL Filter Persistent Handler"},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_HANDLER L"\\PersistentAddinsRegistered",   NULL,               L""},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_HANDLER L"\\PersistentAddinsRegistered\\{89BCB740-6119-101A-BCB7-00DD010655AF}", NULL, SZ_NXLFILTER_CLSID},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\.nxl",															NULL,				 L"NXL File Format"},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\.nxl",															L"Content Type",	 L"application/nxl"},
            {HKEY_LOCAL_MACHINE,   L"Software\\Classes\\.nxl\\PersistentHandler",										NULL,				 SZ_NXLFILTER_HANDLER}
        };

        hr = S_OK;
        for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
        {
            hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
        }
    }
	if (SUCCEEDED(hr))
	{
		wchar_t* szPath = NULL;
		wchar_t* szFilterKey = NULL;
		HKEY hKey = NULL;
		LSTATUS rRet = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Office Server\\15.0", &hKey);
		if (rRet == ERROR_SUCCESS)
		{
			szPath = L"SOFTWARE\\Microsoft\\Office Server\\15.0\\Search\\Setup\\ContentIndexCommon\\Filters\\Extension\\.nxl";
			szFilterKey = L"SOFTWARE\\Microsoft\\Office Server\\15.0\\Search\\Setup\\Filters\\.nxl";
		}
		else
		{
			rRet = RegOpenKeyW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Office Server\\14.0",  &hKey);
			if (rRet == ERROR_SUCCESS)
			{
				szPath = L"SOFTWARE\\Microsoft\\Office Server\\14.0\\Search\\Setup\\ContentIndexCommon\\Filters\\Extension\\.nxl";
				szFilterKey = L"SOFTWARE\\Microsoft\\Office Server\\14.0\\Search\\Setup\\Filters\\.nxl";
			}
		}
		if (szPath != NULL)
		{
			RegCloseKey(hKey);
			const REGISTRY_ENTRY rgRegistryEntries[] =
			{
				{ HKEY_LOCAL_MACHINE, szPath, NULL, SZ_NXLFILTER_CLSID },
				{ HKEY_LOCAL_MACHINE, szFilterKey, L"Extension", L"nxl" },
				{ HKEY_LOCAL_MACHINE, szFilterKey, L"FileTypeBucket", L"3" },
				{ HKEY_LOCAL_MACHINE, szFilterKey, L"MimeTypes", L"application/nxl" }
			};
			DWORD dwType = REG_SZ;
			for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
			{
				if (i == 2)	dwType = REG_DWORD;
				else dwType = REG_SZ;
				hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i],dwType);
			}		
		}
	}
    return hr;
}

// Unregisters this COM server
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;
    const PCWSTR rgpszKeys[] =
    {
        L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_CLSID,
        L"Software\\Classes\\CLSID\\" SZ_NXLFILTER_HANDLER,
        L"Software\\Classes\\.nxl",
		L"SOFTWARE\\Microsoft\\Office Server\\15.0\\Search\\Setup\\ContentIndexCommon\\Filters\\Extension\\.nxl",
		L"SOFTWARE\\Microsoft\\Office Server\\15.0\\Search\\Setup\\Filters\\.nxl",
		L"SOFTWARE\\Microsoft\\Office Server\\14.0\\Search\\Setup\\ContentIndexCommon\\Filters\\Extension\\.nxl",
		L"SOFTWARE\\Microsoft\\Office Server\\14.0\\Search\\Setup\\Filters\\.nxl"
    };

    // Delete the registry entries
    for (int i = 0; i < ARRAYSIZE(rgpszKeys) && SUCCEEDED(hr); i++)
    {
        DWORD dwError = SHDeleteKey(HKEY_LOCAL_MACHINE, rgpszKeys[i]);
        if (ERROR_FILE_NOT_FOUND == dwError)
        {
            // If the registry entry has already been deleted, say S_OK.
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    return hr;
}
