#include "cbcontext.h"

#include <windows.h>
#include <Shlwapi.h>
#include <Shellapi.h>

#pragma warning(push)
#pragma warning(disable: 6387 6011) 
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning(pop)

#include <commonutils.hpp> 

#pragma warning(push)
#pragma warning(disable: 4819)  // ignored character code page issue  
#include <madCHook.h>
#pragma warning(pop)

#include "eventhander.h"
#include "eventparser.h"

namespace
{

#define BEFORECODEBLOCK_BOOL(rt) \
	{ \
	switch (rt) \
		{ \
	case kEventAllow: \
	break; \
	case kEventDeny: \
	return FALSE; \
	case kEventReturnDirectly: \
	return TRUE; \
	default: \
	break; \
		} \
	} 

}

namespace nextlabs
{

/************************************************************************/
/* CAdobeContext                                                */
/************************************************************************/
CCBContext::CCBContext():CShellExploerContext() 
{
    eventParser_ = boost::shared_ptr<CCBParser>(new CCBParser());
}

void CCBContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
{
	table[kHMCoCreateInstance] = true;
	table[kHMCOMNewItem] = true;
	table[kHMCOMRenameItem] = true;
	table[kHMCOMRenameItems] = true;
	table[kHMCOMCopyItems] = true;
	table[kHMCOMMoveItems] = true;
	table[kHMCOMPerformOperations] = true;
	table[kHMCOMDeleteItems] = true;

	//close hook SHFileOperationW API 
	table[kHMSHFileOperationW] = false;

	table[kHMDeleteFileW] = true;
	table[kHMCopyFileW] = true;
	table[kHMCopyFileExW] = true;
	table[kHMDeviceIoControl] = true;
	table[kHMSetFileAttributesW] = true;
	table[kHMAddUsersToEncryptedFile] = true;
	table[kHMSetNamedSecurityInfoW] = true;
	table[kHMEncryptFileW] = true;
	table[kHMDecryptFileW] = true;

	table[kHMMoveFileW] = true;
	table[kHMMoveFileExW] = true;
	table[kHMMoveFileWithProgressW] = true;
	table[kHMKernelBaseMoveFileExW] = true;
	table[kHMKernelBaseMoveFileWithProgressW] = true;

	table[kHMCOMThumbnailCache] = true;
}

BOOL CCBContext::MySetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes )
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

	if ((FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN) == dwFileAttributes || (0x4000000 | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_HIDDEN) == dwFileAttributes)
	{
		return Hooked_SetFileAttributesW_Next(lpFileName, dwFileAttributes);
	}

    if (eventParser_->IsSetFileAttribute(lpFileName))
    {
        BEFORECODEBLOCK_BOOL(OnBeforeSetFileAttributesW(lpFileName, dwFileAttributes));
    }

    return Hooked_SetFileAttributesW_Next(lpFileName, dwFileAttributes);
}

EventResult CCBContext::OnBeforeSetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
	return EventBeforeSetAttributeAction(lpFileName);
}

EventResult CCBContext::EventBeforeSetAttributeAction(const std::wstring& deviceName)
{
	boost::scoped_ptr<CEventHander> handle(new CEventHander());
	return handle->HandleSetFileAttributeAction(deviceName);  
}

}  // ns nextlabs
