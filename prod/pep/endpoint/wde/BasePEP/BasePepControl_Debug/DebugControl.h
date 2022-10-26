#ifndef __WDE_DEBUGCONTROL_H__
#define __WDE_DEBUGCONTROL_H__

#include "commonutils.hpp"

enum EMDHookType
{
    emCreateFile = 0,
    emGreateFileMapping,
    emCreateProcess,
    emIFileOperation,
    emIThumbnailCache,
    emFileSaveDialog,
    emFindFirstFileEx,
    emGetFileAttribute,
    emCoCreateInstance,
};

enum EMOpenTrigger
{
    byCreateFile = 0,
    byGreateFileMapping,
    byCreateProcess,
    byGetThumbnail,
    byFindFirstFileEx,
    byGetFileAttribute,
};

void InitDebugControl();
bool IsDisabledAction(nextlabs::policyengine::WdeAction kemAction);
bool IsDisabledHookType(EMDHookType kemHookType);
bool IsDisabledOpenEnforcerByTrigger(EMOpenTrigger kemOpenTrigger, const bool kbIsFolder);
bool IsNeedDoOpenEnforcerByPath(const wchar_t* kpwchPath, const bool kbIsFolder);

bool IsNeedAddComInit();
bool IsNeedAddComUninit();

#endif