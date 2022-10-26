#include "shellexplorercontext.h"

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

}  // ns anonymous

namespace nextlabs
{

/************************************************************************/
/* CShellExploerContext                                                */
/************************************************************************/
CShellExploerContext::CShellExploerContext():CBaseEventProviderContext() 
{
    eventParser_ = boost::shared_ptr<CExplorerParser>(new CExplorerParser());
}
void CShellExploerContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
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

    table[kHMFindFirstFileExW] = true;
    table[kHMDeleteFileW] = true;
    table[kHMCopyFileW] = true;
    table[kHMCopyFileExW] = true;
    table[kHMDeviceIoControl] = true;
    table[kHMCreateFileW] = true;
    table[kHMCloseHandle] = true;
    table[kHMSetFileAttributesW] = true;
    table[kHMAddUsersToEncryptedFile] = true;
    table[kHMSetNamedSecurityInfoW] = true;
    table[kHMEncryptFileW] = true;
    table[kHMDecryptFileW] = true;

    table[kHMDoDragDrop] = true;
    table[kHMRegisterDragDrop] = true;
    table[kHMRevokeDragDrop] = true;
    table[kHMCreateProcessW] = true;

	table[kHMWriteFile] = true;
	table[kHMWriteFileEx] = true;
	table[kHMSetEndOfFile] = true;
	table[kHMKernelBaseSetEndOfFile] = true;

    table[kHMMoveFileW] = true;
    table[kHMMoveFileExW] = true;
    table[kHMMoveFileWithProgressW] = true;
    table[kHMKernelBaseMoveFileExW] = true;
    table[kHMKernelBaseMoveFileWithProgressW] = true;

    table[kHMCOMThumbnailCache] = true;

    table[kHMSHSimulateDrop] = true;

	// for internet 
	//table[kHMInternetConnectA] = true;
	table[kHMInternetConnectW] = true;
	table[kHMInternetCloseHandle] = true;
	//table[kHMHttpOpenRequestA] = true;
	table[kHMHttpOpenRequestW] = true;
}

BOOL CShellExploerContext::MyCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);
	BEFORECODEBLOCK_BOOL(OnBeforeCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation));

	if(lpCommandLine)
	{
		if (NULL != lpApplicationName && boost::algorithm::iends_with(lpApplicationName, L"\\Acrobat Elements\\Acrobat Elements.exe"))
		{
			int nArgs = 0;
			LPWSTR* szArglist = CommandLineToArgvW(lpCommandLine, &nArgs);

			if (NULL != szArglist)
			{
				BOOL bDeny = FALSE;
				if (nArgs > 1)
				{
					for (int i = 1; i < nArgs; i++)
					{
						if (nextlabs::policyengine::DoEvaluation(nextlabs::policyengine::WdeActionConvert, szArglist[i]) != nextlabs::policyengine::PolicyResultAllow)
						{  
							bDeny = TRUE; 
						}
					}
				}

				LocalFree(szArglist);

				if (bDeny)
				{
					return Hooked_CreateProcessW_Next(lpApplicationName, NULL, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
				}
			}
		}
	}

	return Hooked_CreateProcessW_Next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

EventResult CShellExploerContext::EventBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	EventResult rt = handler->HandleCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists, obligations);

	return rt; 
}

EventResult CShellExploerContext::EventBeforeCopyFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
{
    //OutputDebugStringW(L"----access: CShellExploerContext::EventBeforeCopyFiles");
    boost::scoped_ptr<CEventHander> handler(new CEventHander());
    EventResult rt = handler->HandleCopyAction(vecFileOp);

    return rt; 
}

EventResult CShellExploerContext::EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
{
    //OutputDebugStringW(L"----access: CShellExploerContext::EventBeforeMoveFiles");
    boost::scoped_ptr<CEventHander> handler(new CEventHander());
    EventResult rt = handler->HandleMoveAction(vecFileOp);

    return rt; 
}

EventResult CShellExploerContext::EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData)
{
	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	
	if (boost::algorithm::iends_with(strPath, L":bjDocumentLabelXML"))
	{
		std::wstring newPath = boost::algorithm::erase_tail_copy(strPath, 19);
		boost::scoped_ptr<CEventHander> handle(new CEventHander());
		return handle->HandleWriteFileAction(newPath);
	}
	else
	{
		return handler->HandleNewFileAction(strPath, pUserData);
	}
}

EventResult CShellExploerContext::EventBeforeEditFile(const std::wstring& strPath, PVOID pUserData)
{

	boost::scoped_ptr<CEventHander> handler(new CEventHander());
	EventResult rt = handler->HandleEditFileAction(strPath, pUserData);

	return rt;
}

EventResult CShellExploerContext::EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName)
{
    boost::scoped_ptr<CEventHander> handler(new CEventHander());
    EventResult rt = handler->HandleRenameAction(vecFiles, strNewName);
    return rt;
}

EventResult CShellExploerContext::EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
{
    boost::scoped_ptr<CEventHander> handler(new CEventHander());
    EventResult rt = handler->HandleDeleteAction(vecFiles);
    return rt;
}

EventResult CShellExploerContext::EventBeforeSetCompressionAttributeAction(const std::wstring& deviceName)
{
    //OutputDebugStringW(L"CShellExploerContext::EventBeforeSetCompressionAttributeAction");
    //OutputDebugStringW(deviceName.c_str());
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileAttributeAction(deviceName);       
}

EventResult CShellExploerContext::EventBeforeSetUncompressionAttributeAction(const std::wstring& deviceName)
{
    //OutputDebugStringW(L"CShellExploerContext::EventBeforeSetUncompressionAttributeAction");
    //OutputDebugStringW(deviceName.c_str());
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileAttributeAction(deviceName);       
}

//EventResult CShellExploerContext::EventBeforeSetHiddenAttributeAction(const std::wstring& deviceName)
//{
//    OutputDebugStringW(L"CShellExploerContext::EventBeforeSetHiddenAttributeAction");
//    OutputDebugStringW(deviceName.c_str());
//    boost::scoped_ptr<CEventHander> handle(new CEventHander());
//    return handle->HandleSetFileAttributeAction(deviceName);       
//}

EventResult CShellExploerContext::EventBeforeSetEncryptAttributeAction(const std::wstring& deviceName)
{
    //OutputDebugStringW(L"CShellExploerContext::EventBeforeSetEncryptAttributeAction");
    //OutputDebugStringW(deviceName.c_str());
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileAttributeAction(deviceName);
}

EventResult CShellExploerContext::EventBeforeSetSecurityAttributeAction(const std::wstring& fileName)
{
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileSecurityAttributeAction(fileName);
}

EventResult CShellExploerContext::EventBeforeSetUnencryptedAttributeAction(const std::wstring& deviceName)
{
    //OutputDebugStringW(L"CShellExploerContext::EventBeforeSetUnencryptedAttributeAction");
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileAttributeAction(deviceName);
}

//EventResult CShellExploerContext::EventBeforeSetReadOnlyAttributeAction(const std::wstring& deviceName)
//{
//    OutputDebugStringW(L"CShellExploerContext::EventBeforeSetReadOnlyAttributeAction");
//    boost::scoped_ptr<CEventHander> handle(new CEventHander());
//    return handle->HandleSetFileAttributeAction(deviceName);       
//}

EventResult CShellExploerContext::EventBeforeSetAttributeAction(const std::wstring& deviceName)
{
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleSetFileAttributeAction(deviceName);  
}

EventResult CShellExploerContext::EventBeforeWriteFile(const std::wstring& strFile)
{
	boost::scoped_ptr<CEventHander> handle(new CEventHander());

	if (boost::algorithm::iends_with(strFile, L":bjDocumentLabelXML"))
	{
		std::wstring newFile = boost::algorithm::erase_tail_copy(strFile, 19);
		return handle->HandleWriteFileAction(newFile);
	}
	else
	{
		return handle->HandleWriteFileAction(strFile);
	}
}

EventResult CShellExploerContext::EventBeforeSetEndOfFile(const std::wstring& strFile)
{
	boost::scoped_ptr<CEventHander> handle(new CEventHander());

	if (boost::algorithm::iends_with(strFile, L":bjDocumentLabelXML"))
	{
		std::wstring newFile = boost::algorithm::erase_tail_copy(strFile, 19);	
		return handle->HandleWriteFileAction(newFile);
	}
	else
	{
		return handle->HandleWriteFileAction(strFile);
	}
}

EventResult CShellExploerContext::EventBeforeFileOpen(const std::wstring& fileName, PVOID pUserData)
{
 
  //  In explorer, did not care createfilew file open, it should be dealt  in concrete APP.
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleOpenFileAction(fileName);
}
EventResult CShellExploerContext::EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData)
{
    boost::scoped_ptr<CEventHander> handle(new CEventHander());
    return handle->HandleOpenFolderAction(folderPath);
}

EventResult CShellExploerContext::EventBeforeDropContent(const std::wstring& strFilesrc, const std::wstring& strFileDst)
{
	//OutputDebugString(L"CShellExploerContext::EventBeforeDropContent");
	//OutputDebugString(strFilesrc.c_str());
	//OutputDebugString(strFileDst.c_str());
	boost::scoped_ptr<CEventHander> handle(new CEventHander());
	EventResult rt = handle->HandleDropContentAction(strFilesrc, strFileDst);
	return rt;
}

void CShellExploerContext::EventOpenHttpServer(HINTERNET hRequest, const std::wstring& serverUrl)
{
	//::OutputDebugStringW(__FUNCTIONW__);
	//::OutputDebugStringW(serverUrl.c_str());
	boost::scoped_ptr<CEventHander> handle(new CEventHander());
	handle->HandleHttpOpen(hRequest, serverUrl);
	return;
}

EventResult CShellExploerContext::EventBeforeCreateProcess(const std::wstring& wstrAppPath)
{
    //OutputDebugStringW(__FUNCTIONW__);
    boost::scoped_ptr<CEventHander> handler(new CEventHander());
    EventResult rt = handler->HandleCreateProcess(wstrAppPath);

    return rt;
}

EventResult CShellExploerContext::OnBeforeCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, 
	LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	//OutputDebugStringW(__FUNCTIONW__);
	if (eventParser_->IsCreateProcess(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation))
	{

		std::wstring wstrAppPath = L"";
		if (lpApplicationName != NULL)
		{
			wstrAppPath = std::wstring(lpApplicationName);
		}
		else
		{
			if (lpCommandLine != NULL)
			{
				LPWSTR *szArglist = NULL;
				int     nNumArgs = 0;
				szArglist = CommandLineToArgvW(lpCommandLine, &nNumArgs);
				if (szArglist != NULL)
				{
					wstrAppPath = ( NULL != szArglist[0])? szArglist[0]:L""; 
					LocalFree(szArglist);
					szArglist = NULL;
				}  
			}           
		}

		if (wstrAppPath.empty())
		{
			return kEventAllow;
		}
		else
		{
			//OutputDebugStringW(wstrAppPath.c_str());
			EventResult ret = this->EventBeforeCreateProcess(wstrAppPath); 
			if (ret == kEventAllow)
			{
				std::wstring fileToOpenPath;
				if (eventParser_->IsCreateProcessToOpenFile(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, fileToOpenPath))
				{
					return this->EventBeforeFileOpen(fileToOpenPath, NULL);
				}
			}
			return ret;
		}
	}
	else
	{
		return kEventAllow;
	}
}

}  // ns nextlabs
