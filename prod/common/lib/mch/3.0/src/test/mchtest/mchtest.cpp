
#pragma warning(disable: 4819)
#include <windows.h>
#include <stdio.h>
#include "madCHook_helper.h"



#define INJECT_DRIVER_NAME_A        "NLInjection"
#define INJECT_DRIVER_NAME_W        L"NLInjection"
#define INJECT_DRIVER_DESC          L"NextLabs code injection driver."
#define INJECT_DRIVER_FILENAME64    L"nlinjection64.sys"
#define INJECT_DRIVER_FILENAME32    L"nlinjection32.sys"
#define INJECT_DLL_FILENAME64       "mchtestlib64.dll"
#define INJECT_DLL_FILENAME32       "mchtestlib32.dll"

BOOL init_path();
void help(_In_ const char* AppName);

static WCHAR g_apppath[MAX_PATH+1]   = {0};
static WCHAR g_drvpath64[MAX_PATH+1] = {0};
static WCHAR g_drvpath32[MAX_PATH+1] = {0};
static char  g_dllpath64[MAX_PATH+1] = {0};
static char  g_dllpath32[MAX_PATH+1] = {0};

int main(int argc, char** argv)
{
    BOOL bRet = FALSE;

    if(argc != 2)
    {
        help(argv[0]);
        return 0;
    }

#if defined(_WIN64)
    if(!Is64bitOS())
    {
        printf("This program cannot run on 32bit OS!\n");
        return 0;
    }
#else
    if(Is64bitOS())
    {
        printf("This is cannot run on 64bit OS!\n");
        return 0;
    }
#endif

    if(!init_path())
    {
        printf("Fail to initialize path!\n");
        return 0;
    }

    if(0 == _stricmp(argv[1], "--install"))
    {
        bRet = InstallInjectionDriver(INJECT_DRIVER_NAME_W, g_drvpath32, g_drvpath64, INJECT_DRIVER_DESC);
        if(!bRet)
        {
            printf("Fail to install driver!\n");
            return 0;
        }
        printf("Driver installed!\n");
    }
    else if(0 == _stricmp(argv[1], "--uninstall"))
    {
        bRet = UninstallInjectionDriver(INJECT_DRIVER_NAME_W);
        printf("Driver uninstalled!\n");
    }
    else if(0 == _stricmp(argv[1], "--start"))
    {
        bRet = StartInjectionDriver(INJECT_DRIVER_NAME_W);
        if(bRet)
            printf("Driver started!\n");
        else
            printf("Fail to start NLInjection!\n");
    }
    else if(0 == _stricmp(argv[1], "--stop"))
    {
        bRet = StopInjectionDriver(INJECT_DRIVER_NAME_W);
        if(bRet)
            printf("Driver stopped!\n");
        else
            printf("Fail to stop NLInjection!\n");
    }
    else if(0 == _stricmp(argv[1], "-i"))
    {
        bRet = InjectLibraryA(INJECT_DRIVER_NAME_A, g_dllpath32, CURRENT_SESSION, FALSE);
        if(!bRet)
        {
            printf("Fail to inject 32 bit dll!\n");
            return 0;
        }
        printf("32 bit dll is injected!\n");
#if defined(_WIN64)
        bRet = InjectLibraryA(INJECT_DRIVER_NAME_A, g_dllpath64, CURRENT_SESSION, FALSE);
        if(!bRet)
        {
            printf("Fail to inject 64 bit dll!\n");
            return 0;
        }
        printf("64 bit dll is injected!\n");
#endif
    }
    else if(0 == _stricmp(argv[1], "-u"))
    {
    }
    else
    {
        help(argv[0]);
    }

    return 0;
}

void help(_In_ const char* AppName)
{
    printf("Usage:\n");
    printf("    %s --install: install the injection driver\n", AppName);
    printf("    %s --uninstall: uninstall the injection driver\n", AppName);
    printf("    %s --start: start the injection driver\n", AppName);
    printf("    %s --stop: stop the injection driver\n", AppName);
    printf("    %s -i: inject dll\n", AppName);
    printf("    %s -u: unload dll\n", AppName);
}

BOOL init_path()
{
    WCHAR* pos = NULL;
    GetModuleFileNameW(NULL, g_apppath, MAX_PATH);
    pos = wcsrchr(g_apppath, L'\\');
    if(NULL == pos) return FALSE;

    *pos = L'\0';
    swprintf_s(g_drvpath64, MAX_PATH, L"\\??\\%s\\%s", g_apppath, INJECT_DRIVER_FILENAME64);
    swprintf_s(g_drvpath32, MAX_PATH, L"\\??\\%s\\%s", g_apppath, INJECT_DRIVER_FILENAME32);
    sprintf_s(g_dllpath64, MAX_PATH, "%S\\%s", g_apppath, INJECT_DLL_FILENAME64);
    sprintf_s(g_dllpath32, MAX_PATH, "%S\\%s", g_apppath, INJECT_DLL_FILENAME32);

    printf("  64 bit driver: %S\n", g_drvpath64);
    printf("  32 bit driver: %S\n", g_drvpath32);
    printf("  64 bit DLL:    %s\n", g_dllpath64);
    printf("  32 bit DLL:    %s\n", g_dllpath32);
    return TRUE;
}