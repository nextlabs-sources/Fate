#include "dllhostcontextt.h"

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

#define BEFORECODEBLOCK_HANDLE(rt) \
	{ \
	switch (rt) \
	{ \
	case kEventAllow: \
	break; \
	case kEventDeny: \
	::SetLastError(ERROR_ACCESS_DENIED); \
	return INVALID_HANDLE_VALUE; \
	case kEventReturnDirectly: \
	return INVALID_HANDLE_VALUE; \
	default: \
	break; \
		} \
	}

}

namespace nextlabs
{

/************************************************************************/
/* CDllHostContext                                                */
/************************************************************************/
CDllHostContext::CDllHostContext():CGenericContext() 
{

}

HANDLE CDllHostContext::MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);

	if (!lpFileName || boost::algorithm::istarts_with(lpFileName, L"\\\\.\\PIPE\\") || boost::algorithm::istarts_with(lpFileName, L"\\\\.\\TAPE"))
	{
		return Hooked_CreateFileW_Next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	//
	// notify before
	//
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_HANDLE(OnBeforeCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, &obligations));
	// real
	//
	HANDLE hrt = Hooked_CreateFileW_Next(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	//
	// after
	//
	EventResult ret = OnAfterCreateFileW(hrt, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, &obligations);
	switch (ret)
	{
	case kEventAllow:
		{
			if (INVALID_HANDLE_VALUE != hrt)
			{
				eventParser_->StoreFileHandleWithPath(hrt, lpFileName);
				eventParser_->AddOpenedFile(lpFileName);
			}
		}
		break;
	case kEventDeny:
		{
			return INVALID_HANDLE_VALUE;
		}
		break;
	default:
		break;
	}

	saveAsObligation->DoCreateFileW ( hrt, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile );

	return hrt;
}

EventResult CDllHostContext::OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
	LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData)
{
	return this->EventAfterCreateFile(hFile, lpFileName, pUserData);
}

EventResult CDllHostContext::OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
	LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData)
{
	if (eventParser_->IsDeleteAction(dwFlagsAndAttributes))
	{
		std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
		vecFiles.push_back(std::make_pair(lpFileName, ob));
		return this->EventBeforeDeleteFiles(vecFiles);
	}
	else if (eventParser_->IsNewFileAction(lpFileName, dwCreationDisposition))
	{
		return this->EventBeforeNewFile(lpFileName, pUserData);
	}
	else if (eventParser_->IsFileOpen(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes))
	{
		BOOL bAllowed = TRUE;
		if (eventParser_->GetPolicyCacheResult(lpFileName, bAllowed))
		{
			if (bAllowed)
			{
				return kEventAllow;
			}
			else
			{
				return kEventDeny;
			}
		}

		EventResult er = this->EventBeforeFileOpen(lpFileName, pUserData);

		if (er == kEventAllow)
		{
			eventParser_->StorePolicyCacheResult(lpFileName, true);
		}
		else
		{
			eventParser_->StorePolicyCacheResult(lpFileName, false);
		}

		return er;
	}
	else if (eventParser_->IsDirectoryOpen(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes))
	{
		BOOL bAllowed = TRUE;
		if (eventParser_->GetPolicyCacheResult(lpFileName, bAllowed))
		{
			if (bAllowed)
			{
				return kEventAllow;
			}
			else
			{
				return kEventDeny;
			}
		}

		EventResult er = this->EventBeforeFolderOpen(lpFileName, pUserData);

		if (er == kEventAllow)
		{
			eventParser_->StorePolicyCacheResult(lpFileName, true);
		}
		else
		{
			eventParser_->StorePolicyCacheResult(lpFileName, false);
		}

		return er;
	}


	return this->EventBeforeCreateFile(lpFileName, pUserData);
}

EventResult CDllHostContext::EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());

	//return kEventDeny;
	EventResult rt = handler->HandleDeleteAction(vecFiles);

	return rt;
}

EventResult CDllHostContext::EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());

	EventResult rt = handler->HandleNewFileAction(strPath, pUserData);

	return rt;
}

EventResult CDllHostContext::EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	return handler->HandleOpenFileAction(filePath, false);

}

EventResult CDllHostContext::EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	return handler->HandleOpenFolderAction(folderPath);
}

}  // ns nextlabs
