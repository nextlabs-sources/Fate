

#include <windows.h>
#include <stdio.h>


static void PrintInfo();

BOOL WINAPI DllMain(
                    _In_  HINSTANCE hDll,
                    _In_  DWORD dwReason,
                    _In_  LPVOID lpReserved
                    )
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls((HMODULE)hDll);
        PrintInfo();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}

void PrintInfo()
{
    WCHAR szMod[MAX_PATH+1] = {0};
    WCHAR szInfo[2*MAX_PATH+1] = {0};
    GetModuleFileName(NULL, szMod, MAX_PATH);

#if defined(_WIN64)
    swprintf_s(szInfo, 2*MAX_PATH, L"MCHTestLib64 is loaded to: %s\n", szMod);
#else
    swprintf_s(szInfo, 2*MAX_PATH, L"MCHTestLib32 is loaded to: %s\n", szMod);
#endif
    OutputDebugString(szInfo);
}