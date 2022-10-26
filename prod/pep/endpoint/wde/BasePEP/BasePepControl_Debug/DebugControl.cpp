#include "DebugControl.h"
#include <set>
#include <map>
#include <string>
#include <windows.h>

using namespace nextlabs;
using namespace nextlabs::policyengine;

const wchar_t* g_kpchConfig_DisableHook = L"DisableHook";
const wchar_t* g_kpchConfig_CreateFile = L"CreateFile";
const wchar_t* g_kpchConfig_GreateFileMapping = L"GreateFileMapping";
const wchar_t* g_kpchConfig_CreateProcess = L"CreateProcess";
const wchar_t* g_kpchConfig_IFileOperation = L"IFileOperation";
const wchar_t* g_kpchConfig_IThumbnailCache = L"IThumbnailCache";
const wchar_t* g_kpchConfig_FileSaveDialog = L"FileSaveDialog";
const wchar_t* g_kpchConfig_FindFirstFileEx = L"FindFirstFileEx";
const wchar_t* g_kpchConfig_GetFileAttribute = L"GetFileAttribute";
const wchar_t* g_kpchConfig_CoCreateInstance = L"CoCreateInstance";

const wchar_t* g_kpchConfig_DisableEnforcer = L"DisableEnforcer";
const wchar_t* g_kpchConfig_OPEN = L"OPEN";
const wchar_t* g_kpchConfig_RUN = L"RUN";

const wchar_t* g_kpchConfig_Common = L"Common";
const wchar_t* g_kpchConfig_FileEnforcerDrive = L"FileEnforcerDrive";
const wchar_t* g_kpchConfig_FolderEnforcerDrive = L"FolderEnforcerDrive";
const wchar_t* g_kpchConfig_AddComInit = L"AddComInit";
const wchar_t* g_kpchConfig_AddComUninit = L"AddComUninit";


const wchar_t* g_kpchConfig_DisableOpenFileByTrigger = L"DisableOpenFileByTrigger";
//const wchar_t* g_kpchConfig_CreateFile = L"";
//const wchar_t* g_kpchConfig_GreateFileMapping = L"byGreateFileMapping";
//const wchar_t* g_kpchConfig_CreateProcess = L"";
const wchar_t* g_kpchConfig_GetThumbnail = L"GetThumbnail";

const wchar_t* g_kpchConfig_DisableOpenFolderByTrigger = L"DisableOpenFolderByTrigger";
//const wchar_t* g_kpchConfig_CreateFile = L"";
//const wchar_t* g_kpchConfig_CreateProcess = L"";
//const wchar_t* g_kpchConfig_GetThumbnail = L"";

static const wchar_t* s_kszHookType[] = {
    g_kpchConfig_CreateFile,
    g_kpchConfig_GreateFileMapping,
    g_kpchConfig_CreateProcess,
    g_kpchConfig_IFileOperation,
    g_kpchConfig_IThumbnailCache,
    g_kpchConfig_FileSaveDialog,
    g_kpchConfig_FindFirstFileEx,
    g_kpchConfig_GetFileAttribute,
    g_kpchConfig_CoCreateInstance
};
static const wchar_t* s_kszOpenTrigger[] = {
    g_kpchConfig_CreateFile,
    g_kpchConfig_GreateFileMapping,
    g_kpchConfig_CreateProcess,
    g_kpchConfig_GetThumbnail,
    g_kpchConfig_FindFirstFileEx,
    g_kpchConfig_GetFileAttribute,
};
const wchar_t* g_kpchConfigFile = L"C:\\NLDebug\\BasePepControl.ini";

static const WdeAction s_szWdeAction[] = {
    WdeActionRead, WdeActionRun,
};
std::set<WdeAction> g_setDisabledAction;

std::set<EMDHookType> g_setDisabledHookType;
std::set<EMOpenTrigger> g_setDisableOpenFileByTrigger;
std::set<EMOpenTrigger> g_setDisableOpenFolderByTrigger;
std::wstring g_wstrFileEnforcerDrive = L"*";
std::wstring g_wstrFolderEnforcerDrive = L"*";

bool g_bAddComInit = true;
bool g_bAddComUninit = false;

void InitDebugControl()
{
    g_setDisabledAction.clear();
    g_setDisabledHookType.clear();
    g_setDisabledHookType.insert(emCoCreateInstance);
    g_setDisableOpenFileByTrigger.clear();
    g_setDisableOpenFolderByTrigger.clear();
    g_wstrFileEnforcerDrive = L"*";
    g_wstrFolderEnforcerDrive = L"*";
    g_bAddComInit = true;
    g_bAddComUninit = false;
	
    // Disabled action
    {
        const int knCount = sizeof(s_szWdeAction) / sizeof(WdeAction);
        for (int i = 0; i < knCount; ++i)
        {
		    WdeAction emAction = s_szWdeAction[i];
            int nRet = GetPrivateProfileInt(g_kpchConfig_DisableEnforcer, WdeActionMap(emAction), 0, g_kpchConfigFile);
            if (0 != nRet)
            {
                g_setDisabledAction.insert(emAction);
            }
        }
    }
    // Disabled Hook type
    {
        const int knCount = sizeof(s_kszHookType) / sizeof(wchar_t*);
        for (int i = 0; i < knCount; ++i)
        {
            int nRet = GetPrivateProfileInt(g_kpchConfig_DisableHook, s_kszHookType[i], 0, g_kpchConfigFile);
            if (0 != nRet)
            {
                g_setDisabledHookType.insert((EMDHookType)i);
            }
        }
    }
    // Disabled open file
    {
        const int knCount = sizeof(s_kszOpenTrigger) / sizeof(wchar_t*);
        for (int i = 0; i < knCount; ++i)
        {
            int nRet = GetPrivateProfileInt(g_kpchConfig_DisableOpenFileByTrigger, s_kszOpenTrigger[i], 0, g_kpchConfigFile);
            if (0 != nRet)
            {
                g_setDisableOpenFileByTrigger.insert((EMOpenTrigger)i);
            }
        }
    }
    // Disabled open folder
    {
        const int knCount = sizeof(s_kszOpenTrigger) / sizeof(wchar_t*);
        for (int i = 0; i < knCount; ++i)
        {
            int nRet = GetPrivateProfileInt(g_kpchConfig_DisableOpenFolderByTrigger, s_kszOpenTrigger[i], 0, g_kpchConfigFile);
            if (0 != nRet)
            {
                g_setDisableOpenFolderByTrigger.insert((EMOpenTrigger)i);
            }
        }
    }
    static const DWORD kdwSize = 1024;
    {
        wchar_t wszFileEnforcerDrive[kdwSize] = { 0 };
        GetPrivateProfileStringW(g_kpchConfig_Common, g_kpchConfig_FileEnforcerDrive, L"*", wszFileEnforcerDrive, kdwSize, g_kpchConfigFile);
        g_wstrFileEnforcerDrive = wszFileEnforcerDrive;
    }
    {
        wchar_t wszFolderEnforcerDrive[kdwSize] = { 0 };
        GetPrivateProfileStringW(g_kpchConfig_Common, g_kpchConfig_FolderEnforcerDrive, L"*", wszFolderEnforcerDrive, kdwSize, g_kpchConfigFile);
        g_wstrFolderEnforcerDrive = wszFolderEnforcerDrive;
    }
    {
        g_bAddComInit = (0 != GetPrivateProfileInt(g_kpchConfig_Common, g_kpchConfig_AddComInit, 1, g_kpchConfigFile));
        g_bAddComUninit = (0 != GetPrivateProfileInt(g_kpchConfig_Common, g_kpchConfig_AddComUninit, 0, g_kpchConfigFile));
    }

    // Log
    {
        {
            std::wstring wstrLog = L"WDE Action disable:";
            for (std::set<WdeAction>::iterator kitr = g_setDisabledAction.begin(); kitr != g_setDisabledAction.end(); ++kitr)
            {
                wstrLog.append(WdeActionMap(*kitr));
                wstrLog.push_back(',');
            }
            wstrLog.push_back('\n');
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
        {
            std::wstring wstrLog = L"HookType disable:";
            for (std::set<EMDHookType>::iterator kitr = g_setDisabledHookType.begin(); kitr != g_setDisabledHookType.end(); ++kitr)
            {
                wstrLog.push_back((int)*kitr + '0');
                wstrLog.push_back(',');
            }
            wstrLog.push_back('\n');
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
        {
            std::wstring wstrLog = L"OpenFile trigger disable:";
            for (std::set<EMOpenTrigger>::iterator kitr = g_setDisableOpenFileByTrigger.begin(); kitr != g_setDisableOpenFileByTrigger.end(); ++kitr)
            {
                wstrLog.push_back((int)*kitr + '0');
                wstrLog.push_back(',');
            }
            wstrLog.push_back('\n');
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
        {
            std::wstring wstrLog = L"OpenFolder trigger disable:";
            for (std::set<EMOpenTrigger>::iterator kitr = g_setDisableOpenFolderByTrigger.begin(); kitr != g_setDisableOpenFolderByTrigger.end(); ++kitr)
            {
                wstrLog.push_back((int)*kitr + '0');
                wstrLog.push_back(',');
            }
            wstrLog.push_back('\n');
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
        {
            std::wstring wstrLog = L"File enforcer drive:" + g_wstrFileEnforcerDrive + L"\n"; 
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
            wstrLog = L"Folder enforcer drive : " + g_wstrFolderEnforcerDrive + L"\n";
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
        {
            std::wstring wstrLog = L"AddComInit:";
            wstrLog.append(g_bAddComInit ? L"true\n" : L"false\n");
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
            wstrLog = L"AddComUninit:";
            wstrLog.append(g_bAddComUninit ? L"true\n" : L"false\n");
            NLPRINT_DEBUGLOGW(wstrLog.c_str());
        }
    }
}
bool IsDisabledAction(nextlabs::policyengine::WdeAction kemAction)
{
    bool bDisabled = g_setDisabledAction.end() != g_setDisabledAction.find(kemAction);
    wchar_t wszLog[1024] = { 0 };
    swprintf_s(wszLog, 1023, L"WDEAction:[%s] is disabled:[%s]\n", WdeActionMap(kemAction), HZBoolToStringW(bDisabled));
    NLPRINT_DEBUGLOGW(wszLog);
    return bDisabled;
}
bool IsDisabledHookType(EMDHookType kemHookType)
{
    bool bDisabled = g_setDisabledHookType.end() != g_setDisabledHookType.find(kemHookType);
    wchar_t wszLog[1024] = { 0 };
    swprintf_s(wszLog, 1023, L"Hook type:[%s] is disabled:[%s]\n", s_kszHookType[kemHookType], HZBoolToStringW(bDisabled));
    NLPRINT_DEBUGLOGW(wszLog);
    return bDisabled;
}
bool IsDisabledOpenEnforcerByTrigger(EMOpenTrigger kemOpenTrigger, const bool kbIsFolder)
{
    if (kbIsFolder)
    {
        return g_setDisableOpenFolderByTrigger.end() != g_setDisableOpenFolderByTrigger.find(kemOpenTrigger);
    }
    else
    {
        return g_setDisableOpenFileByTrigger.end() != g_setDisableOpenFileByTrigger.find(kemOpenTrigger);
    }
}
bool IsNeedDoOpenEnforcerByPath(const wchar_t* kpwchPath, const bool kbIsFolder)
{
    bool bRet = true;
    if (NULL != kpwchPath)
    {
        wchar_t wchUpper = toupper(kpwchPath[0]);
        if (kbIsFolder)
        {
            if (L"*" != g_wstrFolderEnforcerDrive)
            {
                bRet = std::wstring::npos != g_wstrFolderEnforcerDrive.find(wchUpper);
            }
        }
        else
        {
            if (L"*" != g_wstrFileEnforcerDrive)
            {
                bRet = std::wstring::npos != g_wstrFileEnforcerDrive.find(wchUpper);
            }
        }
    }
    return bRet;
}

bool IsNeedAddComInit()
{
    return g_bAddComInit;
}
bool IsNeedAddComUninit()
{
    return g_bAddComUninit;
}