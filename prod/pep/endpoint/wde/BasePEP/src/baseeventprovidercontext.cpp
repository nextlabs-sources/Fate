#include "BaseEventProviderContext.h"

#include <commonutils.hpp> 
#include "eventparser.h"

#include <boost/lexical_cast.hpp>

#pragma warning(push)
#pragma warning(disable: 6386 6031 6328 6258 6309 6387 6334 4267)  
#include <boost/asio.hpp>
#pragma warning(pop)

using namespace boost::asio;

#pragma warning( push )
#pragma warning( disable : 4996 6326 6246 6385 4328 )
#include <boost/xpressive/xpressive_dynamic.hpp>
#pragma warning(pop)

using namespace boost::xpressive;

#include "ScreenCaptureAuxiliary.h"

extern LPCTSTR SHARED_MUTEX_NAME;
extern HANDLE gSharedFileMutex;

namespace
{
    #define COMBEFORECODEBLOCK(rt) \
    { \
        switch (rt) \
        { \
        case kEventAllow: \
            break; \
        case kEventDeny: \
            return E_FAIL; \
        case kEventReturnDirectly: \
            return S_OK; \
        default: \
            break; \
        } \
    }
    #define COMAFTERCODEBLOCK(rt, hr) \
    { \
        switch (rt) \
        { \
        case kEventAllow: \
            break; \
        case kEventDeny: \
            return E_FAIL; \
        default: \
            break; \
        } \
        return hr; \
    }

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

    #define BEFORECODEBLOCK_NTSTATUS(rt) \
    { \
        switch (rt) \
        { \
        case kEventAllow: \
            break; \
        case kEventDeny: \
            return STATUS_CANCELLED; \
        case kEventReturnDirectly: \
            return STATUS_SUCCESS; \
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

	const WCHAR* DefaultText = L"NextLabs";
	const char*  charDisplay = "DISPLAY";
	const WCHAR* wcharDisplay = L"DISPLAY";

	const char* QueryFormat = "query:pid=";
	const char* QueryResultFormat = "^query=(\\w{0,})(;displaytext=(\\w{0,}))?";
	const int QueryLength = 64;

	const char* DenyFlag = "deny";
	const char* AllowFlag = "allow";
	const int QueryResultLength = 512;

	const char* SCEServerIP = "127.0.0.1";
	const USHORT SCEServerBasedPort = 20000;

}  // ns anonymous

namespace nextlabs
{

CBaseEventProviderContext::CBaseEventProviderContext() : CRuntimeContext()
{
    eventParser_ = boost::shared_ptr<CEventParser>(new CEventParser());
	networkEventParser_ = boost::shared_ptr<CNetworkEventParser>(new CNetworkEventParser());
	saveAsObligation = boost::shared_ptr<GenericSaveAsObligation>(GenericSaveAsObligation::GetInstance());

	DesktophWnd = GetDesktopWindow();
}

void CBaseEventProviderContext::PurgeDropTargets()
{
    eventParser_->RevokeAllDropTargets();
}

BOOL CBaseEventProviderContext::finishDrop(IDataObject* pDataObject,DNDWinClassAction winClassAction)
{
    std::wstring srcFilePath = L"";
    std::wstring dstFilePath = L"";
    std::list<std::wstring> files;

    if (eventParser_->isDropForPasteContent(pDataObject, winClassAction, srcFilePath, dstFilePath))
    {
        OutputDebugString(L"Drop content for Copy");
        EventResult rt = EventBeforeDropContent(srcFilePath, dstFilePath);
        if (rt == kEventAllow)
        {
            return TRUE;
        } else
        {
            return FALSE;
        }
    }
    
    if (eventParser_->isDropForOpenFiles(pDataObject, winClassAction, files))
    {
        OutputDebugString(L"Drop File For openning");
        //EventResult rt = eventBeforeOpenFile()
        return TRUE;
    }
    
    if (eventParser_->isDropForCopyFiles(pDataObject, winClassAction, files, dstFilePath))
    {
        OutputDebugString(L"Drop file for copy");
        for (std::list<std::wstring>::const_iterator itor = files.begin();  itor != files.end(); ++itor)
        {
            std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
            nextlabs::comhelper::FILEOP fileOp;
            OutputDebugString(itor->c_str());
            fileOp.strSrc = itor->c_str();
            fileOp.strDst = dstFilePath; 
            vecFileOP.push_back(fileOp); 
            EventResult rt = EventBeforeCopyFiles(vecFileOP);
            if (rt != kEventAllow)
            {
                return FALSE;
            }
        }
        return TRUE;
    }

    return TRUE;
}

HANDLE CBaseEventProviderContext::MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);

    if (!lpFileName || nextlabs::utils::CanIgnoreFile(lpFileName))
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

HANDLE CBaseEventProviderContext::MyCreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BEFORECODEBLOCK_HANDLE(OnBeforeGreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName));
    return Hooked_CreateFileMappingW_Next(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
}

BOOL CBaseEventProviderContext::MyCloseHandle(HANDLE hObject)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //
    // before
    //
    std::wstring filePath = eventParser_->GetFilePathByHandle(hObject);
    eventParser_->RemoveFilePathInfoByHandle(hObject);
    BOOL rt = eventParser_->IsHandleMapContantsFile(filePath);
    if (rt == FALSE)
    {
        eventParser_->RemoveOpenedFile(filePath);
    }
	eventParser_->RemoveFilePolicy(hObject);
    OnBeforeCloseHandle(hObject);
    return Hooked_CloseHandle_Next(hObject);
}

HANDLE CBaseEventProviderContext::MyFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BEFORECODEBLOCK_HANDLE(OnBeforeFindFirstFileExW(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags));
    return Hooked_FindFirstFileExW_Next(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
}

BOOL CBaseEventProviderContext::MyReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    return Hooked_ReadFile_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

BOOL CBaseEventProviderContext::MyReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_ReadFileEx_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
}

BOOL CBaseEventProviderContext::MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
	BEFORECODEBLOCK_BOOL(OnBeforeWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped));
    return Hooked_WriteFile_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL CBaseEventProviderContext::MySetEndOfFile(HANDLE hFile)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	BEFORECODEBLOCK_BOOL(OnBeforeSetEndOfFile(hFile));
	return Hooked_SetEndOfFile_Next(hFile);
}

BOOL CBaseEventProviderContext::MyKernelBaseSetEndOfFile(HANDLE hFile)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	BEFORECODEBLOCK_BOOL(OnBeforeSetEndOfFile(hFile));
	return Hooked_KernelBaseSetEndOfFile_Next(hFile);
}

BOOL CBaseEventProviderContext::MyWriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
	BEFORECODEBLOCK_BOOL(OnBeforeWriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine));
    return Hooked_WriteFileEx_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);

}

BOOL CBaseEventProviderContext::MyCreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_CreateHardLinkW_Next(lpFileName, lpExistingFileName, lpSecurityAttributes);
}

BOOL CBaseEventProviderContext::MyCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    BEFORECODEBLOCK_BOOL(OnBeforeCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation));
    return Hooked_CreateProcessW_Next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL CBaseEventProviderContext::MyCreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforeCreateDirectory(lpPathName, lpSecurityAttributes, &obligations));
    BOOL bRet = Hooked_CreateDirectoryW_Next(lpPathName, lpSecurityAttributes);
	if (bRet)
	{
		OnAfterCreateDirectory(lpPathName, lpSecurityAttributes, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyRemoveDirectoryW(LPCWSTR lpPathName)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	::OutputDebugString(__FUNCTIONW__);
	return Hooked_RemoveDirectoryW_Next(lpPathName);
}

BOOL CBaseEventProviderContext::MyDeleteFileW(LPCWSTR lpFileName)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);

	if (NULL == lpFileName)
	{
		return Hooked_DeleteFileW_Next(lpFileName);
	}

    BEFORECODEBLOCK_BOOL(OnBeforeDeleteFileW(lpFileName))

    return Hooked_DeleteFileW_Next(lpFileName);
}

BOOL CBaseEventProviderContext::MyCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforeCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists, &obligations));  

    BOOL bRet = Hooked_CopyFileW_Next(lpExistingFileName, lpNewFileName, bFailIfExists);

	saveAsObligation->DoCopyFileW(bRet, lpExistingFileName, lpNewFileName);

	if (bRet)
	{
		if (kEventAllow != OnAfterCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists, &obligations))
		{
			return FALSE;
		}
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforeCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations));  

    BOOL bRet = Hooked_CopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	if (bRet)
	{
		if (kEventAllow != OnAfterCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations))
		{
			return FALSE;
		}
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyPrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforePrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations));  

    BOOL bRet = Hooked_PrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	if (bRet)
	{
		if (kEventAllow != OnAfterPrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations))
		{
			return FALSE;
		}
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyKernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
    nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforeKernelBasePrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations));  

    BOOL bRet = Hooked_KernelBasePrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    if (bRet)
    {
        if (kEventAllow != OnAfterKernelBasePrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags, &obligations))
        {
            return FALSE;
        }
    }
    return bRet;
}

BOOL CBaseEventProviderContext::MyMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_BOOL(OnBeforeMoveFile(lpExistingFileName, lpNewFileName, &obligations));
    BOOL bRet = Hooked_MoveFileW_Next(lpExistingFileName, lpNewFileName);

	saveAsObligation->DoMoveFileW(bRet, lpExistingFileName, lpNewFileName);

	if (bRet)
	{
		return OnAfterMoveFile(lpExistingFileName, lpNewFileName, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_BOOL(OnBeforeMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations));
    BOOL bRet = Hooked_MoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);

	if (bRet)
	{
		return OnAfterMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
    BEFORECODEBLOCK_BOOL(OnBeforeMoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags, &obligations));
    BOOL bRet = Hooked_MoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	if (bRet)
	{
		return OnAfterMoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyKernelBaseMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_BOOL(OnBeforeKernelBaseMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations));
	BOOL bRet = Hooked_KernelBaseMoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);
	if (bRet)
	{
		return OnAfterKernelBaseMoveFileEx(lpExistingFileName, lpNewFileName, dwFlags, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyKernelBaseMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	// to prevent the reentrant
	nextlabs::recursion_control_auto auto_disable(hook_control);
	::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_BOOL(OnBeforeKernelBaseMoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags, &obligations));
	BOOL bRet = Hooked_KernelBaseMoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	if (bRet)
	{
		return OnAfterKernelBaseMoveFileWithProgress(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags, &obligations);
	}
	return bRet;
}

BOOL CBaseEventProviderContext::MyReplaceFileW(LPCWSTR lpReplacedFileName, LPCWSTR lpReplacementFileName, LPCWSTR lpBackupFileName, DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved)
{
    // to prevent the reentrant
    nextlabs::recursion_control_auto auto_disable(hook_control);
    ::OutputDebugString(__FUNCTIONW__);
	nextlabs::Obligations obligations;
	BEFORECODEBLOCK_BOOL(OnBeforeMoveFile(lpReplacementFileName, lpReplacedFileName, &obligations));
    return Hooked_ReplaceFileW_Next(lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved);
    ///////////////////////////////
}

VOID CBaseEventProviderContext::MyExitProcess(UINT uExitCode)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    Hooked_ExitProcess_Next(uExitCode);

}

BOOL CBaseEventProviderContext::MyTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_TerminateProcess_Next(hProcess, uExitCode);
}

HMODULE CBaseEventProviderContext::MyLoadLibraryW(LPCWSTR lpLibFileName)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_LoadLibraryW_Next(lpLibFileName);
}

HMODULE CBaseEventProviderContext::MyLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_LoadLibraryExW_Next(lpLibFileName, hFile, dwFlags);
}

BOOL CBaseEventProviderContext::MyDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BEFORECODEBLOCK_BOOL(OnBeforeDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped));
    return Hooked_DeviceIoControl_Next(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

}

BOOL CBaseEventProviderContext::MySetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes )
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    if (eventParser_->IsSetFileAttribute(lpFileName))
    {
        BEFORECODEBLOCK_BOOL(OnBeforeSetFileAttributesW(lpFileName, dwFileAttributes));
    }
    /*if (eventParser_->IsSetAttributeHidden(dwFileAttributes))
    {
    EventResult ret = OnBeforeSetFileAttributeHidden(lpFileName);
    if (ret == kEventDeny)
    {
    dwFileAttributes = (dwFileAttributes & (~FILE_ATTRIBUTE_HIDDEN));
    }
    }

    if (eventParser_->IsSetAttributeReadOnly(dwFileAttributes))
    {
    EventResult ret = OnBeforeSetFileAttributesReadOnly(lpFileName);
    if (ret == kEventDeny)
    {
    dwFileAttributes = (dwFileAttributes & (~FILE_ATTRIBUTE_READONLY));
    }
    }*/
    return Hooked_SetFileAttributesW_Next(lpFileName, dwFileAttributes);
}

 DWORD CBaseEventProviderContext::MyGetFileAttributesW(LPCWSTR lpFileName)
 {
    nextlabs::recursion_control_auto auto_disable(hook_control);
    
    EventResult ret = OnBeforeGetFileAttributeW(lpFileName);
    if (ret == kEventAllow)
    {
        return Hooked_GetFileAttributesW_Next(lpFileName);
    }

    return INVALID_FILE_ATTRIBUTES;
    
 }

 DWORD CBaseEventProviderContext::MySetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl)
 {
     nextlabs::recursion_control_auto auto_disable(hook_control);
     EventResult ret = OnBeforeSetNamedSecurityInfoW(pObjectName, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
     if (ret == kEventAllow)
     {
         return Hooked_SetNamedSecurityInfoW_Next(pObjectName, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
     }
     return ERROR_ACCESS_DENIED;
 }

 DWORD CBaseEventProviderContext::MyAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUSers)
 {
     nextlabs::recursion_control_auto auto_disable(hook_control);
     EventResult ret = OnBeforeAddUsersToEncryptedFile(lpFileName, pUSers);
     if (ret == kEventAllow)
     {
         return Hooked_AddUsersToEncryptedFile_Next(lpFileName, pUSers);
     }
     return ERROR_ACCESS_DENIED;
 }

 NTSTATUS CBaseEventProviderContext::MyNtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor)
 {
     nextlabs::recursion_control_auto auto_disable(hook_control);
     EventResult ret = OnBeforeNtSetSecurityObject(handle, securityInformation, securityDescriptor);
     if (ret == kEventDeny)
     {
         return STATUS_ACCESS_DENIED;
     }
     
     return Hooked_NtSetSecurityObject_Next(handle, securityInformation, securityDescriptor);
 }

BOOL CBaseEventProviderContext::MySetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation, DWORD dwBufferSize)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);

    if (eventParser_->IsFileAttributeChange(FileInformationClass, lpFileInfromation))
    {
        std::wstring filePath = eventParser_->GetFilePathByHandle(hFile);
        BEFORECODEBLOCK_BOOL(OnBeforeSetFileAttributesW(filePath.c_str(), 0));
       
    }
    return Hooked_SetFileInformationByHandle_Next(hFile, FileInformationClass, lpFileInfromation, dwBufferSize);

}

BOOL CBaseEventProviderContext::MyEncryptFileW(LPCWSTR lpFileName)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    BEFORECODEBLOCK_BOOL(OnBeforeEncryptFileW(lpFileName));
    return Hooked_EncryptFileW_Next(lpFileName);
}

BOOL CBaseEventProviderContext::MyDecryptFileW(LPCWSTR lpFileName, DWORD dwReserved)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    BEFORECODEBLOCK_BOOL(OnBeforeDecryptFileW(lpFileName, dwReserved));
    return Hooked_DecryptFileW_Next(lpFileName, dwReserved);
}

HANDLE CBaseEventProviderContext::MySetClipboardData(UINT uFormat, HANDLE hMem)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    BEFORECODEBLOCK_HANDLE(OnBeforeSetClipboardData(uFormat, hMem));
    HANDLE hr = Hooked_SetClipboardData_Next(uFormat, hMem);
    return hr;
}

HANDLE CBaseEventProviderContext::MyGetClipboardData(UINT uFormat)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    BEFORECODEBLOCK_HANDLE(OnBeforeGetClipBoardData(uFormat));
    return Hooked_GetClipboardData_Next(uFormat);
}

HRESULT CBaseEventProviderContext::MyOleSetClipboard(LPDATAOBJECT pDataObj)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    COMBEFORECODEBLOCK(OnBeforeOleSetClipboard(pDataObj));
    return Hooked_OleSetClipboard_Next(pDataObj);
}

HRESULT CBaseEventProviderContext::MyOleGetClipboard(LPDATAOBJECT *ppDataObj)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(__FUNCTIONW__);
    HRESULT hr = Hooked_OleGetClipboard_Next(ppDataObj);
    if (SUCCEEDED(hr))
    {
        EventResult result = OnAfterOleGetClipboard(ppDataObj);
        if (kEventAllow != result)
        {
            *ppDataObj = NULL;
            return E_FAIL;
        } 
        else
        {
            return hr;
        }
    }
    return hr;
}

HRESULT CBaseEventProviderContext::MyDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect)
{
	OutputDebugString(L"CBaseEventProviderContext::MyDoDragDrop");
    
    nextlabs::recursion_control_auto auto_disable(hook_control);
	COMBEFORECODEBLOCK(OnBeforeDoDragDrop(pDataObj, pDropSource, dwOkEffects, pdwEffect));
    return Hooked_DoDragDrop_Next(pDataObj, pDropSource, dwOkEffects, pdwEffect);
}

HRESULT CBaseEventProviderContext::MyRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
    OutputDebugString(L"CBaseEventProviderContext::MyRegisterDragDrop");
	
    nextlabs::recursion_control_auto auto_disable(hook_control);
    DropTargetProxy *proxy = NULL;
    COMBEFORECODEBLOCK(OnBeforeRegisterDragDrop(hwnd, pDropTarget, proxy));
    if (NULL == proxy)
    {
       return Hooked_RegisterDragDrop_Next(hwnd, pDropTarget);
    }
    HRESULT hr = Hooked_RegisterDragDrop_Next(hwnd, proxy);
    if (FAILED(hr))
    {
		delete proxy;
        return hr;
    }
    COMAFTERCODEBLOCK(OnAftereRegisterDragDrop(hwnd, proxy), hr);
}

HRESULT CBaseEventProviderContext::MyRevokeDragDrop(HWND hwnd)
{
    //OutputDebugStringW(L"CBaseEventProviderContext::MyRevokeDragDrop");
	
    nextlabs::recursion_control_auto auto_disable(hook_control);
    COMBEFORECODEBLOCK(OnBeforeRevokeDragDrop(hwnd));
    return Hooked_RevokeDragDrop_Next(hwnd);
}

BOOL CBaseEventProviderContext::MySHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation)
{
	//OutputDebugStringW(L"CBaseEventProviderContext::MySHFileOperationW---------------------");
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);
	BEFORECODEBLOCK_BOOL(OnBeforeSHFileOperationW(lpFileOperation));
	return Hooked_SHFileOperationW_Next(lpFileOperation);
}

NTSTATUS CBaseEventProviderContext::MyNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
                        ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BEFORECODEBLOCK_NTSTATUS(OnBeforeNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength));

    return Hooked_NtCreateFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize,
        FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
}

NTSTATUS CBaseEventProviderContext::MyNtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
   BEFORECODEBLOCK_NTSTATUS(OnBeforeNtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions));
    NTSTATUS ret =  Hooked_NtOpenFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    if (ret == STATUS_SUCCESS && *FileHandle && ObjectAttributes->ObjectName->Buffer != NULL)
    {
        std::wstring strPath = std::wstring(ObjectAttributes->ObjectName->Buffer);
        strPath = utils::FormatPath(strPath);
        eventParser_->StoreFileHandleWithPath(*FileHandle, strPath.c_str());
        eventParser_->AddOpenedFile(strPath.c_str());
    }
    
    return ret;
}

NTSTATUS CBaseEventProviderContext::MyNtClose(HANDLE handle)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    BEFORECODEBLOCK_NTSTATUS(OnBeforeNtClose(handle));
    std::wstring filePath = eventParser_->GetFilePathByHandle(handle);
    eventParser_->RemoveFilePathInfoByHandle(handle);
    if (!eventParser_->IsHandleMapContantsFile(filePath))
    {
        eventParser_->RemoveOpenedFile(filePath);
    }
    NTSTATUS ret = Hooked_NtClose_Next(handle);
    return ret;
}

NTSTATUS CBaseEventProviderContext::MyRtlDosPathNameToNtPathName_U_WithStatus(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    return Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next(DosFileName, NtFileName, FilePart, RelativeName);
}


HRESULT CBaseEventProviderContext::MyCOMCopyItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMCopyItems nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    COMBEFORECODEBLOCK(OnBeforeCOMCopyItems(This, punkItems, psiDestinationFolder));

    return nextFunc(This, punkItems, psiDestinationFolder);

} 

HRESULT CBaseEventProviderContext::MyCOMMoveItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMMoveItems nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(L"CBaseEventProviderContext::MyCOMMoveItems -------------");
    COMBEFORECODEBLOCK(OnBeforeCOMMoveItems(This, punkItems, psiDestinationFolder));

    return nextFunc(This, punkItems, psiDestinationFolder);

} 

HRESULT CBaseEventProviderContext::MyCOMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem, PF_COMNewItem nextFunc)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(L"CBaseEventProviderContext::MyCOMNewItem ---------------");
	COMBEFORECODEBLOCK(OnBeforeCOMNewItem(This, psiDestinationFolder, dwFileAttributes, pszName, pszTemplateName, pfopsItem))
	
	return nextFunc(This, psiDestinationFolder, dwFileAttributes, pszName, pszTemplateName, pfopsItem);
}

HRESULT CBaseEventProviderContext::MyCOMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem, PF_COMRenameItem nextfunc)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	COMBEFORECODEBLOCK(OnBeforeCOMRenameItem(This, psiDestinationFolder, pszNewName, pfopsItem));

	return nextfunc(This, psiDestinationFolder, pszNewName, pfopsItem);
}

HRESULT CBaseEventProviderContext::MyCOMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName, PF_COMRenameItems nextfunc)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	COMBEFORECODEBLOCK(OnBeforeCOMRenameItems(This, pUnkItems, pszNewName));

	return nextfunc(This, pUnkItems, pszNewName);
}

HRESULT CBaseEventProviderContext::MyCOMPerformOperations(IFileOperation* This, PF_COMPerformOperations nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);

    COMBEFORECODEBLOCK(OnBeforeCOMPerformOperations(This));

    return nextFunc(This);
}

HRESULT CBaseEventProviderContext::MyCOMDeleteItems(IFileOperation* This, IUnknown *punkItems, PF_COMDeleteItems nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(L"MyCOMDeleteItems");
	COMBEFORECODEBLOCK(OnBeforeCOMDeleteItems(This, punkItems))
	
	return nextFunc(This, punkItems);
	
}

HRESULT CBaseEventProviderContext::MyCOMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem, PF_COMDeleteItem nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    //OutputDebugStringW(L"MyCOMDeleteItem");
    COMBEFORECODEBLOCK(OnBeforeCOMDeleteItem(This, psiItem, pfopsItem))

    return nextFunc(This, psiItem, pfopsItem);
}

HRESULT CBaseEventProviderContext::MyCOMGetThumbnail(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID, PF_COMGetThumbnail nextFunc)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    EventResult ret = OnBeforeGetThumbnail(pShellItem);
    if (ret == kEventDeny)
    {
        //C:\\Program Files\\NextLabs\\Desktop Enforcer\\bin\\ce_deny.gif
        std::wstring imagefile = L"";
        
        if (!nextlabs::utils::GetDenyImageFilePath(imagefile))
        {
            return nextFunc(This, pShellItem, cxyRequestedThumbSize, flags, ppvThumb, pOutFlags, pThumbnailID);
        }
        IShellItem *ppShellItem;
        SHCreateItemFromParsingName(imagefile.c_str(), NULL, IID_IShellItem, (void**)&ppShellItem);
        nextFunc(This, ppShellItem, cxyRequestedThumbSize, WTS_EXTRACTDONOTCACHE | flags, ppvThumb, pOutFlags, pThumbnailID);
        ppShellItem->Release();
        return WTS_E_FAILEDEXTRACTION;
    }
    return nextFunc(This, pShellItem, cxyRequestedThumbSize, flags, ppvThumb, pOutFlags, pThumbnailID);
}

HRESULT CBaseEventProviderContext::MyCOMShow(IFileSaveDialog* pThis, HWND hwndOwner, PF_COMShow nextFunc)
{
	std::wstring strSource;
	eventParser_->GetCurrentDocumentPath(strSource);
	
	HRESULT hr = nextFunc(pThis, hwndOwner);

	if (SUCCEEDED(hr))
	{
		if (!OnAfterCOMShow(pThis, hwndOwner, strSource))
		{
			hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
			saveAsObligation->SetValid(FALSE);
		}
	}
	else
	{
		saveAsObligation->SetValid(FALSE);
	}

	return hr;
}

HRESULT CBaseEventProviderContext::MyCOMSetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease, PF_COMSetData nextFunc)
{
	return nextFunc(pThis, pformatetc, pmedium, fRelease);
}

HINTERNET CBaseEventProviderContext::MyInternetConnectA(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);
	return Hooked_InternetConnectA_Next(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
}


HINTERNET CBaseEventProviderContext::MyInternetConnectW(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);

	//OnBeforeInternetConnect(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	HINTERNET hConnect =Hooked_InternetConnectW_Next(hInternet, 
		lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);

	if (hConnect != NULL && lpszServerName != NULL && wcslen(lpszServerName) > 0){
		networkEventParser_->OnSuccessInternetConnect(hConnect, lpszServerName, nServerPort);
	}
	return hConnect;
}
	

BOOL CBaseEventProviderContext::MyInternetCloseHandle(HINTERNET hInternet)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);

	BOOL bResult = Hooked_InternetCloseHandle_Next(hInternet);
	if (bResult){
		networkEventParser_->OnSuccessInternetCloseHandle(hInternet);
	}
	return bResult;
}

HINTERNET CBaseEventProviderContext::MyHttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);
	return Hooked_HttpOpenRequestA_Next(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
}


HINTERNET CBaseEventProviderContext::MyHttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);
	//OutputDebugStringW(__FUNCTIONW__);
	HINTERNET hRequest = Hooked_HttpOpenRequestW_Next(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	if (hRequest != hConnect && lpszObjectName != NULL && wcslen(lpszObjectName) > 0){
		std::wstring serverUrl;
		bool hasServer = networkEventParser_->OnSuccessHttpOpenRequest(hConnect, lpszObjectName, serverUrl);
		if (hasServer){
			EventOpenHttpServer(hRequest, serverUrl);
		}
	}
	return hRequest;
}

BOOL CBaseEventProviderContext::MyGetSaveFileNameW(LPOPENFILENAMEW lpofn)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring strSource;
	eventParser_->GetCurrentDocumentPath(strSource);

	BOOL bRet = Hooked_GetSaveFileNameW_Next(lpofn);

	if (bRet)
	{
		bRet = OnAfterGetSaveFileNameW(lpofn, strSource);

		if (!bRet)
		{
			saveAsObligation->SetValid(FALSE);
		}
	}
	else
	{
		saveAsObligation->SetValid(FALSE);
	}

	return bRet;
}

BOOL CBaseEventProviderContext::MyGetOpenFileNameW(LPOPENFILENAMEW lpofn)
{
   
    memset(lpofn->lpstrFile, 0, lpofn->nMaxFile);
    if (!Hooked_GetOpenFileNameW_Next(lpofn))
    {
        return FALSE;
    }
    BOOL bRet = OnAfterGetOpenFileNameW(lpofn);
    return bRet;
}

HRESULT CBaseEventProviderContext::MySHSimulateDrop(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect)
{
    nextlabs::recursion_control_auto auto_disable(hook_control);
    if (pDataObj == NULL || pdwEffect == NULL)
    {
        return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
    }

    if (nextlabs::comhelper::DataObj_CanGoAsync(pDataObj))
    {
        return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
    }
    else
    {
        if (nextlabs::comhelper::DataObj_GoAsyncForCompat(pDataObj))
        {
            return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
        }
    }
    COMBEFORECODEBLOCK(OnBeforeSHSimulateDrop(pDataObj, pdwEffect));
    return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
}

BOOL CBaseEventProviderContext::MyBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring DisplayText;
	bool bAllow = Query(DisplayText, hdcSrc);
	if (bAllow)
	{
		return Hooked_BitBlt_Next(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	}

	HDC Newhdc = ScreenCaptureAuxiliary::GetInstance().GenerateHDC(DisplayText, cx, cy);
	BOOL bRet = Hooked_BitBlt_Next(hdc, x, y, cx, cy, Newhdc, x1, y1, rop);
	DeleteDC(Newhdc);

	return bRet;
}

BOOL CBaseEventProviderContext::MyMaskBlt(HDC hdcDest, int xDest, int yDest, int width, int height, HDC hdcSrc, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask, DWORD rop)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring DisplayText;
	bool bAllow = Query(DisplayText, hdcSrc);
	if (bAllow)
	{
		return Hooked_MaskBlt_Next(hdcDest, xDest, yDest, width, height, hdcSrc, xSrc, ySrc, hbmMask, xMask, yMask, rop);
	}

	HDC Newhdc = ScreenCaptureAuxiliary::GetInstance().GenerateHDC(DisplayText, width, height);
	BOOL bRet = Hooked_MaskBlt_Next(hdcDest, xDest, yDest, width, height, Newhdc, xSrc, ySrc, hbmMask, xMask, yMask, rop);
	DeleteDC(Newhdc);

	return bRet;
}

BOOL CBaseEventProviderContext::MyPlgBlt(HDC hdcDest, CONST POINT * lpPoint, HDC hdcSrc, int xSrc, int ySrc, int width, int height, HBITMAP hbmMask, int xMask, int yMask)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring DisplayText;
	bool bAllow = Query(DisplayText, hdcSrc);
	if (bAllow)
	{
		return Hooked_PlgBlt_Next(hdcDest, lpPoint, hdcSrc, xSrc, ySrc, width, height, hbmMask, xMask, yMask);
	}

	HDC Newhdc = ScreenCaptureAuxiliary::GetInstance().GenerateHDC(DisplayText, width, height);
	BOOL bRet = Hooked_PlgBlt_Next(hdcDest, lpPoint, Newhdc, xSrc, ySrc, width, height, hbmMask, xMask, yMask);
	DeleteDC(Newhdc);

	return bRet;
}

BOOL CBaseEventProviderContext::MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	std::wstring DisplayText;
	bool bAllow = Query(DisplayText, hdcSrc);
	if (bAllow)
	{
		return Hooked_StretchBlt_Next(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	}

	HDC Newhdc = ScreenCaptureAuxiliary::GetInstance().GenerateHDC(DisplayText, wDest, hDest);
	BOOL bRet = Hooked_StretchBlt_Next(hdcDest, xDest, yDest, wDest, hDest, Newhdc, xSrc, ySrc, wSrc, hSrc, rop);
	DeleteDC(Newhdc);

	return bRet;
}

BOOL CBaseEventProviderContext::MyPrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	BOOL bRet = Hooked_PrintWindow_Next(hwnd, hdcBlt, nFlags);

	if (bRet)
	{
		std::wstring DisplayText;
		bool bAllow = Query(DisplayText, hwnd);
		if (!bAllow)
		{
			ScreenCaptureAuxiliary::GetInstance().ReplaceHDC(DisplayText,hdcBlt)		;
		}		
	}

	return bRet;
}

HDC CBaseEventProviderContext::MyCreateDCA(LPCSTR pszDriver, LPCSTR pszDevice, LPCSTR pszPort, CONST DEVMODEA * pdm)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HDC hdc = Hooked_CreateDCA_Next(pszDriver, pszDevice, pszPort, pdm);
	if (NULL != hdc && NULL != pszDriver)
	{
		if (boost::algorithm::iequals(pszDriver, charDisplay) || NULL == pszDevice)
		{
			DWORD ProcessId = 0;
			AddHDC(hdc, ProcessId);
		}
	}

	return hdc;
}

HDC CBaseEventProviderContext::MyCreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pwszPort, CONST DEVMODEW * pdm)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HDC hdc = Hooked_CreateDCW_Next(pwszDriver, pwszDevice, pwszPort, pdm);
	if (NULL != hdc && NULL != pwszDriver)
	{
		if (boost::algorithm::iequals(pwszDriver, wcharDisplay) || NULL == pwszDevice)
		{
			DWORD ProcessId = 0;
			AddHDC(hdc, ProcessId);
		}
	}

	return hdc;
}

HDC CBaseEventProviderContext::MyGetDC(HWND hWnd)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HDC hdc = Hooked_GetDC_Next(hWnd);
	AddHDC(hdc, hWnd);
	return hdc;
}

HDC CBaseEventProviderContext::MyGetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HDC hdc = Hooked_GetDCEx_Next(hWnd, hrgnClip, flags);
	AddHDC(hdc, hWnd);
	return hdc;
}

HDC CBaseEventProviderContext::MyGetWindowDC(HWND hWnd)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	HDC hdc = Hooked_GetWindowDC_Next(hWnd);
	AddHDC(hdc, hWnd);
	return hdc;
}

int CBaseEventProviderContext::MyReleaseDC(HWND hWnd, HDC hDC)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	RemoveHDC(hDC);
	return Hooked_ReleaseDC_Next(hWnd, hDC);
}

BOOL CBaseEventProviderContext::MyDeleteDC(HDC hdc)
{
	nextlabs::recursion_control_auto auto_disable(hook_control);

	RemoveHDC(hdc);
	return Hooked_DeleteDC_Next(hdc);
}

#pragma region EventFilter

EventResult CBaseEventProviderContext::OnBeforeSHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation)
{
	if (eventParser_->IsDeleteAction(lpFileOperation))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::Obligations ob;
        std::wstring strFilePath = lpFileOperation->pFrom;
        vecFiles.push_back(std::make_pair(strFilePath, ob));
        return this->EventBeforeDeleteFiles(vecFiles);
	}
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnBeforeCOMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem)
{
    if (eventParser_->IsRenameAction(This, psiDestinationFolder, pszNewName, pfopsItem))
    {
        std::wstring strDestFolder;
        if (FAILED(nextlabs::comhelper::GetPathByShellItem(strDestFolder, psiDestinationFolder)))
        {
            return kEventAllow;
        }
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(strDestFolder, ob));
        return this->EventBeforeRename(vecFiles, pszNewName);
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeCOMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName)
{
	if (eventParser_->IsRenameAction(This, pUnkItems, pszNewName))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles = nextlabs::comhelper::GetFilePathsFromObject(pUnkItems);  
		return this->EventBeforeRename(vecFiles, pszNewName);
	}
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnBeforeCOMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem)
{
	if (eventParser_->IsNewFileAction(This, psiDestinationFolder, dwFileAttributes, pszName, pszTemplateName, pfopsItem))
	{
		//OutputDebugStringW(L"CBaseEventProviderContext::OnBeforeCOMNewItem IsNewFileAction true--------------");
        LPWSTR lpPath = NULL;
        HRESULT hr = psiDestinationFolder->GetDisplayName(SIGDN_FILESYSPATH, &lpPath);
        if(FAILED(hr) || lpPath == NULL)
        {
            return kEventAllow;
        }

        std::wstring strFilePath = lpPath;
        CoTaskMemFree(lpPath);
        strFilePath += L"\\";
        strFilePath += pszName;

        std::wstring strRealPath;
        nextlabs::comhelper::GetNewCreatedFileName(strFilePath, strRealPath);
		return this->EventBeforeNewFile(strRealPath, NULL);
	}
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnBeforeCOMDeleteItems(IFileOperation* This, IUnknown *punkItems)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(This, punkItems))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles = nextlabs::comhelper::GetFilePathsFromObject(punkItems);
        if (vecFiles.empty())
        {
            return kEventAllow;
        }

        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeCOMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(This, psiItem, pfopsItem))
    {
        std::wstring strDestFolder;
        if (FAILED(nextlabs::comhelper::GetPathByShellItem(strDestFolder, psiItem)))
        {
            return kEventAllow;
        }

        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(strDestFolder, ob));
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else
    {
         return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeGetThumbnail(IShellItem *pShellItem)
{
    std::wstring filepath = L"";

    if (eventParser_->IsPictureFileType(pShellItem, filepath))
    {
        return this->EventBeforeFileOpen(filepath, NULL);
    }
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
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

EventResult CBaseEventProviderContext::OnBeforeGreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName)
{
    std::wstring filePath = eventParser_->GetFilePathByHandle(hFile);
    return this->EventBeforeFileOpen(filePath, NULL);
}

EventResult CBaseEventProviderContext::OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
                               LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData)
{
    return this->EventAfterCreateFile(hFile, lpFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
    if (lpFileName)
    {
        if (PathFileExistsW(lpFileName) && PathIsDirectoryW(lpFileName))
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

            EventResult er = this->EventBeforeFolderOpen(lpFileName, NULL);

            if (er == kEventAllow)
            {
                eventParser_->StorePolicyCacheResult(lpFileName, true);
            }
            else
            {
                eventParser_->StorePolicyCacheResult(lpFileName, false);
            }

        }
    }
    
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeCreateDirectory(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsNewDirectory(lpPathName))
    {
        //OutputDebugStringW(L"eventParser_->IsNewDirectory(lpPathName)   TRUE----------------------");
        //OutputDebugStringW(lpPathName);
        return this->EventBeforeNewDirectory(lpPathName, pUserData);
    }
    else
    {
        //OutputDebugStringW(L"eventParser_->IsNewDirectory(lpPathName)   FALSE----------------------");
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnAfterCreateDirectory(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PVOID pUserData)
{
	return this->EventAfterNewDirectory(lpPathName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeMoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(lpNewFileName))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
        EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
    }
    else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
        nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
        EventResult er = this->EventBeforeMoveFiles(vecFileOP);  
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }    
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnAfterMoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(lpNewFileName))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
        EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
	}
    else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName, dwFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
        nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
        EventResult er = this->EventBeforeMoveFiles(vecFileOP);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    } 
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnAfterMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
										 LPVOID lpData, DWORD dwFlags, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(lpNewFileName))
    {        
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
        EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
	}
    else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP; 
        nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
        EventResult er = this->EventBeforeMoveFiles(vecFileOP); 
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }    
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnAfterMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
	LPVOID lpData, DWORD dwFlags, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeKernelBaseMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(lpNewFileName))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        //OutputDebugStringW(lpExistingFileName);
        nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
		EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
	}
    else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName, dwFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;  
        nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
        EventResult er = this->EventBeforeMoveFiles(vecFileOP);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }   
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnAfterKernelBaseMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeKernelBaseMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
	LPVOID lpData, DWORD dwFlags, PVOID pUserData)
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsDeleteAction(lpNewFileName))
    {
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::comhelper::ParseDeleteFolder(std::wstring(lpExistingFileName), vecFiles);
        return this->EventBeforeDeleteFiles(vecFiles);
    }
	else if (eventParser_->IsRenameAction(lpExistingFileName, lpNewFileName))
	{
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
		nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(lpExistingFileName, ob));
		EventResult er = this->EventBeforeRename(vecFiles, lpNewFileName);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFiles[0].second;
		}
		return er;
	}
    else if (eventParser_->IsMoveAction(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;  
        nextlabs::comhelper::WrapMoveFileOperation(std::wstring(lpExistingFileName), std::wstring(lpNewFileName), vecFileOP);
        EventResult er = this->EventBeforeMoveFiles(vecFileOP);
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }    
	else
	{
		return kEventAllow;
	}
}

EventResult CBaseEventProviderContext::OnAfterKernelBaseMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
	LPVOID lpData, DWORD dwFlags, PVOID pUserData)
{
	return this->EventAfterMoveFiles(lpExistingFileName, lpNewFileName, pUserData);
}

EventResult CBaseEventProviderContext::OnBeforeDeleteFileW(LPCWSTR lpFileName)
{
    //OutputDebugStringW(__FUNCTIONW__);
    std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
    nextlabs::Obligations ob;
    vecFiles.push_back(std::make_pair(lpFileName, ob));
    return this->EventBeforeDeleteFiles(vecFiles);
}

EventResult CBaseEventProviderContext::OnBeforeNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
                                 ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
{
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(DesiredAccess);
    UNREFERENCED_PARAMETER(IoStatusBlock);
    UNREFERENCED_PARAMETER(AllocationSize);
    UNREFERENCED_PARAMETER(FileAttributes);
    UNREFERENCED_PARAMETER(ShareAccess);
    UNREFERENCED_PARAMETER(EaBuffer);
    UNREFERENCED_PARAMETER(EaLength);
    //OutputDebugStringW(__FUNCTIONW__);

    std::wstring strFilePath = std::wstring(ObjectAttributes->ObjectName->Buffer);
    if (!boost::algorithm::istarts_with(strFilePath, L"\\??\\"))
    {
        DWORD dwPathSize =  GetCurrentDirectory(0, NULL);
        if (dwPathSize > 0)
        {
            WCHAR* buf = new WCHAR[dwPathSize + 1];
            memset(buf, 0, sizeof(WCHAR) * (dwPathSize + 1));
            GetCurrentDirectory(dwPathSize, buf);
            strFilePath = std::wstring(buf) + L"\\" + strFilePath;
            delete [] buf;
            buf = NULL;
        }
    }
    else
    {
        strFilePath = strFilePath.substr(4);
    }
    if (eventParser_->IsNewDirectory(strFilePath.c_str(), CreateDisposition, CreateOptions))
    {
        return this->EventBeforeNewDirectory(strFilePath.c_str(), NULL);
    }
    else if (eventParser_->IsNewFileAction(strFilePath, CreateDisposition, CreateOptions))
    {
         return this->EventBeforeNewFile(strFilePath.c_str(), NULL);
    }

    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeNtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, 
                                                          OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions)
{
    UNREFERENCED_PARAMETER(FileHandle);
    UNREFERENCED_PARAMETER(IoStatusBlock);
    UNREFERENCED_PARAMETER(ShareAccess);
    UNREFERENCED_PARAMETER(OpenOptions);

    if (eventParser_->IsDeleteAction(DesiredAccess, ObjectAttributes, ShareAccess, OpenOptions))
    {
        std::wstring strFilePath = std::wstring(ObjectAttributes->ObjectName->Buffer);
        if (!boost::algorithm::istarts_with(strFilePath, L"\\??\\"))
        {

            DWORD dwPathSize =  GetCurrentDirectory(0, NULL);
            if (dwPathSize > 0)
            {
                WCHAR* buf = new WCHAR[dwPathSize + 1];
                memset(buf, 0, sizeof(WCHAR) * (dwPathSize + 1));
                GetCurrentDirectory(dwPathSize, buf);
                strFilePath = std::wstring(buf) + L"\\" + strFilePath;
                delete [] buf;
                buf = NULL;
            }

        }

        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles;
        nextlabs::Obligations ob;
        vecFiles.push_back(std::make_pair(strFilePath, ob));
        return this->EventBeforeDeleteFiles(vecFiles);
    }
    
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeNtClose(HANDLE handle)
{
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeSetFileAttributeHidden(LPCWSTR lpFileName)
{
    return EventBeforeSetHiddenAttributeAction(lpFileName);
}

EventResult CBaseEventProviderContext::OnBeforeSetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
    return EventBeforeSetAttributeAction(lpFileName);
}

EventResult CBaseEventProviderContext::OnBeforeGetFileAttributeW(LPCWSTR lpFileName)
{
    if (eventParser_->IsDirectoryOpen(lpFileName))
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

        EventResult er = EventBeforeFolderOpen(lpFileName, NULL);

        if (er == kEventAllow)
        {
            eventParser_->StorePolicyCacheResult(lpFileName, true);
        }
        else
        {
            eventParser_->StorePolicyCacheResult(lpFileName, false);
        }
    }
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeSetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl)
{
    if (eventParser_->IsSetFileSecurityAttribute(pObjectName, ObjectType))
    {
        return EventBeforeSetSecurityAttributeAction(pObjectName);
    }
    
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers)
{
    if (eventParser_->IsSetAddUsersToEncryptedFile(lpFileName, pUsers))
    {
        return EventBeforeSetAttributeAction(lpFileName);
    }

    return kEventAllow;
    
}

EventResult CBaseEventProviderContext::OnBeforeNtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor)
{
    std::wstring fileName = eventParser_->GetFilePathByHandle(handle);
    if (fileName.c_str())
    {
        return EventBeforeSetSecurityAttributeAction(fileName);
    }
    return kEventAllow;
    
}


EventResult CBaseEventProviderContext::OnBeforeSetFileAttributesReadOnly(LPCWSTR lpFileName)
{
    return EventBeforeSetReadOnlyAttributeAction(lpFileName);
}

EventResult CBaseEventProviderContext::OnBeforeEncryptFileW(LPCWSTR lpFileName)
{
    // TBD USE EventParser
    if (eventParser_->IsSetAttributeEncrypt(lpFileName))
    {
        return this->EventBeforeSetEncryptAttributeAction(lpFileName);
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeDecryptFileW(LPCWSTR lpFileName, DWORD dwReserved)
{
    if (eventParser_->IsSetAttributeUnencrypted(lpFileName))
    {
        return this->EventBeforeSetUnencryptedAttributeAction(lpFileName);
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeSetClipboardData(UINT uFormat, HANDLE hMem)
{
    std::wstring srcFilePath = L"";
    if (eventParser_->isCopyContent(uFormat, srcFilePath))
    {
        return this->EventBeforeCopyContent(srcFilePath);
    }
    else 
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeGetClipBoardData(UINT uFormat)
{
    std::wstring srcFilePath = L"";
    std::wstring dstFilePath = L"";
    if (eventParser_->isPasteContent(uFormat, dstFilePath, srcFilePath))
    {
        return this->EventBeforePasteContent(srcFilePath, dstFilePath);
    }

    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeOleSetClipboard(LPDATAOBJECT pDataObj)
{
    std::wstring srcFilePath = L"";
    if (eventParser_->isCopyContent(pDataObj, srcFilePath))
    {
        return this->EventBeforeCopyContent(srcFilePath);
    }
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnAfterOleGetClipboard(LPDATAOBJECT *ppDataObj)
{
    std::wstring srcFilePath = L"";
    std::wstring dstFilePath = L"";
    if (eventParser_->isPasteContent(*ppDataObj, dstFilePath, srcFilePath))
    {
        return this->EventBeforePasteContent(srcFilePath, dstFilePath);
    }

    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect)
{
    std::wstring srcFilePath = L"";
    if (eventParser_->IsDragDropContent(pDataObj, pDropSource, dwOkEffects, pdwEffect, srcFilePath))
    {
        return kEventAllow;
    }
    //TBD
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget, __out DropTargetProxy *&proxy)
{
    DNDWinClassAction winclassAction;
    if (eventParser_->IsRegisterDragDrop(hwnd, pDropTarget, winclassAction))
    {
        OutputDebugString(L"RegisterDrapDrop success");
        proxy = new DropTargetProxy(hwnd, pDropTarget, this, winclassAction);
        if (proxy == NULL)
        {   // registered proxy target successfully Save proxy target in m_DropTargets
            OutputDebugString(L"create proxy failed");
        }
    }
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnAftereRegisterDragDrop(const HWND hwnd, DropTargetProxy* &proxy)
{
    proxy->AddRef();
    eventParser_->addDropTargetProxy(hwnd, proxy);
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeRevokeDragDrop(HWND hwnd)
{
    DropTargetProxy *proxy = eventParser_->GetDropTargetProxy(hwnd);
    if (proxy)
    {
        proxy->Release();
    }
    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeCOMCopyItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder) 
{
    if(eventParser_->IsCopyAction(This, punkItems, psiDestinationFolder)) 
    {
        //OutputDebugStringW(__FUNCTIONW__);

        std::wstring strSrcRoot = L"";
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles = nextlabs::comhelper::GetFilePathsFromObject(punkItems, strSrcRoot);  

		std::vector<std::wstring> vecCopyFiles;
		for (std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator ci = vecFiles.begin(); ci != vecFiles.end(); ci++)
		{
			vecCopyFiles.push_back(ci->first);
		}

        if (!vecFiles.empty())
        {
            std::vector<nextlabs::comhelper::FILEOP> vecFileOP; 
            std::wstring strDestFolder = L"";  //  C:\allen\nds_test
            if (SUCCEEDED(nextlabs::comhelper::GetPathByShellItem(strDestFolder, psiDestinationFolder)) && !strDestFolder.empty())
            {
               nextlabs::comhelper::GenerateDestPath(vecCopyFiles, strDestFolder, strSrcRoot, vecFileOP, true);
         
               return this->EventBeforeCopyFiles(vecFileOP); 
            }

        }
        return kEventAllow;
    }
    else
    {
        return kEventAllow;
    }

}

EventResult CBaseEventProviderContext::OnBeforeCOMMoveItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder) 
{
    //OutputDebugStringW(__FUNCTIONW__);
    if(eventParser_->IsMoveAction(This, punkItems, psiDestinationFolder)) 
    {
        //OutputDebugStringW(__FUNCTIONW__);

        std::wstring strSrcRoot = L"";
        std::vector<std::pair<std::wstring, nextlabs::Obligations>> vecFiles = nextlabs::comhelper::GetFilePathsFromObject(punkItems, strSrcRoot);  

		std::vector<std::wstring> vecMoveFiles;
		for (std::vector<std::pair<std::wstring, nextlabs::Obligations>>::const_iterator ci = vecFiles.begin(); ci != vecFiles.end(); ci++)
		{
			vecMoveFiles.push_back(ci->first);
		}

        if (!vecFiles.empty())
        {
            std::vector<nextlabs::comhelper::FILEOP> vecFileOP; 
            std::wstring strDestFolder = L"";  //  C:\allen\nds_test
            if (SUCCEEDED(nextlabs::comhelper::GetPathByShellItem(strDestFolder, psiDestinationFolder)) && !strDestFolder.empty())
            {
                nextlabs::comhelper::GenerateDestPath(vecMoveFiles, strDestFolder, strSrcRoot, vecFileOP, false);

                return this->EventBeforeMoveFiles(vecFileOP); 
            }

        }
        return kEventAllow;  

    }
    else
    {
        return kEventAllow;
    }

}

EventResult CBaseEventProviderContext::OnBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, PVOID pUserData)  // nds
{
    //OutputDebugStringW(L" -------CBaseEventProviderContext::OnBeforeCopyFileW-------------");
    if (eventParser_->IsCopyAction(lpExistingFileName, lpNewFileName, bFailIfExists))
    {
		nextlabs::Obligations obligations;
        EventResult er = this->EventBeforeCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists, obligations); 
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = obligations;
		}

		return er;
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnAfterCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, PVOID pUserData)  // nds
{
	return EventAfterCopyFiles(lpExistingFileName, lpNewFileName, pUserData); 
}

EventResult CBaseEventProviderContext::OnBeforeCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
                                                           LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  // nds2
{
    //OutputDebugStringW(__FUNCTIONW__);
    
    if (eventParser_->IsCopyAction(lpExistingFileName, lpNewFileName, lpProgressRoutine, 
        lpData, pbCancel, dwCopyFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
        nextlabs::comhelper::FILEOP fileOp;

        fileOp.strSrc = std::wstring(lpExistingFileName);
        fileOp.strDst = std::wstring(lpNewFileName);

        //OutputDebugStringW(fileOp.strSrc.c_str());
        //OutputDebugStringW(fileOp.strDst.c_str());

        vecFileOP.push_back(fileOp);
        EventResult er = this->EventBeforeCopyFiles(vecFileOP);  
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnAfterCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
	LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  // nds2
{
	return EventAfterCopyFiles(lpExistingFileName, lpNewFileName, pUserData); 
}

EventResult CBaseEventProviderContext::OnBeforePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
                                                           LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsCopyAction(lpExistingFileName, lpNewFileName, lpProgressRoutine, 
        lpData, pbCancel, dwCopyFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
        nextlabs::comhelper::FILEOP fileOp;

        fileOp.strSrc = std::wstring(lpExistingFileName);
        fileOp.strDst = std::wstring(lpNewFileName);     

        //OutputDebugStringW(fileOp.strSrc.c_str());
        //OutputDebugStringW(fileOp.strDst.c_str());

        vecFileOP.push_back(fileOp);
        EventResult er = this->EventBeforeCopyFiles(vecFileOP);  
		if(er == kEventAllow)
		{
			*((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
		}
		return er;
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnBeforeKernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
                                                               LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  
{
    //OutputDebugStringW(__FUNCTIONW__);
    if (eventParser_->IsCopyAction(lpExistingFileName, lpNewFileName, lpProgressRoutine, 
        lpData, pbCancel, dwCopyFlags))
    {
        std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
        nextlabs::comhelper::FILEOP fileOp;

        fileOp.strSrc = std::wstring(lpExistingFileName);
        fileOp.strDst = std::wstring(lpNewFileName);     

        //OutputDebugStringW(fileOp.strSrc.c_str());
        //OutputDebugStringW(fileOp.strDst.c_str());

        vecFileOP.push_back(fileOp);
        EventResult er = this->EventBeforeCopyFiles(vecFileOP);  
        if(er == kEventAllow)
        {
            *((nextlabs::Obligations*)pUserData) = vecFileOP[0].obligations;
        }
        return er;
    }
    else
    {
        return kEventAllow;
    }
}

EventResult CBaseEventProviderContext::OnAfterPrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
	LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  
{
	return EventAfterCopyFiles(lpExistingFileName, lpNewFileName, pUserData); 
}

EventResult CBaseEventProviderContext::OnAfterKernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
                                                              LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData)  
{
    return EventAfterCopyFiles(lpExistingFileName, lpNewFileName, pUserData); 
}

EventResult CBaseEventProviderContext::OnBeforeWriteFile(HANDLE hFile, LPCVOID lpBuffer,
										DWORD& nNumberOfBytesToWrite, LPDWORD& lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	return OnBeforeCommonWriteFile(hFile);
}

EventResult CBaseEventProviderContext::OnBeforeWriteFileEx(HANDLE hFile, LPCVOID lpBuffer,
									DWORD& nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	return OnBeforeCommonWriteFile(hFile);
}

EventResult CBaseEventProviderContext::OnBeforeSetEndOfFile(HANDLE hFile)
{
	BOOL bWriteAllowed = FALSE;
	if (eventParser_->GetFilePolicy(hFile, bWriteAllowed))
	{
		if (bWriteAllowed)
		{
			return kEventAllow;
		}
		else
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return kEventDeny;
		}
	}

	std::wstring filePath = eventParser_->GetFilePathByHandle(hFile);
	EventResult er = EventBeforeSetEndOfFile(filePath);

	if (er == kEventAllow)
	{
		eventParser_->StoreFilePolicy(hFile, true);
	}
	else
	{
		eventParser_->StoreFilePolicy(hFile, false);
		SetLastError(ERROR_ACCESS_DENIED);
	}

	return er;
}

EventResult CBaseEventProviderContext::OnBeforeDeviceIoControl(HANDLE hDevice,DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,
                                            LPVOID lpOutBuffer,DWORD nOutBufferSize, LPDWORD lpBytesReturned,LPOVERLAPPED lpOverlapped)
{
    if (eventParser_->IsSetAttributeCompressed(hDevice, dwIoControlCode, lpInBuffer))
    {
        std::wstring filePath = eventParser_->GetFilePathByHandle(hDevice);
        return EventBeforeSetCompressionAttributeAction(filePath);
    }

    if (eventParser_->IsSetAttributeUncompressed(hDevice, dwIoControlCode, lpInBuffer))
    {
        std::wstring filePath = eventParser_->GetFilePathByHandle(hDevice);
        return EventBeforeSetUncompressionAttributeAction(filePath);
    }

    return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeCommonWriteFile(HANDLE hFile)
{
	BOOL bWriteAllowed = FALSE;
	if (eventParser_->GetFilePolicy(hFile, bWriteAllowed))
	{
		if (bWriteAllowed)
		{
			return kEventAllow;
		}
		else
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return kEventDeny;
		}
	}

	std::wstring filePath = eventParser_->GetFilePathByHandle(hFile);
	EventResult er = EventBeforeWriteFile(filePath);

	if (er == kEventAllow)
	{
		eventParser_->StoreFilePolicy(hFile, true);
	}
	else
	{
		eventParser_->StoreFilePolicy(hFile, false);
		SetLastError(ERROR_ACCESS_DENIED);
	}

	return er;
}

BOOL CBaseEventProviderContext::OnAfterGetSaveFileNameW(LPOPENFILENAMEW lpofn, const std::wstring& strSource)
{	
	SaveAsInfo::SaveAsStruct ComSaveAsStr;
	ComSaveAsStr.bPolicyAllow = FALSE;

	EventResult er = EventAfterSaveAs(lpofn, strSource, &ComSaveAsStr);

	if (ComSaveAsStr.bPolicyAllow)
	{
		if (!strSource.empty())
		{
			saveAsObligation->Prepare(strSource, ComSaveAsStr.strDestinationPath, ComSaveAsStr.obs, lpofn);
		}
	}

	if (er == kEventAllow)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CBaseEventProviderContext::OnAfterGetOpenFileNameW(LPOPENFILENAMEW lpofn)
{   
    BEFORECODEBLOCK_BOOL(this->EventAfterGetOpenFileName(lpofn));
}

BOOL CBaseEventProviderContext::OnAfterCOMShow(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource)
{
	SaveAsInfo::SaveAsStruct ComSaveAsStr;
	ComSaveAsStr.bPolicyAllow = FALSE;

	EventResult er = EventAfterSaveAs(pThis, hwndOwner, strSource, &ComSaveAsStr);

	if (ComSaveAsStr.bPolicyAllow)
	{
		if (!strSource.empty())
		{
			saveAsObligation->Prepare(strSource, ComSaveAsStr.strDestinationPath, ComSaveAsStr.obs, pThis);
		}
	}

	if (er == kEventAllow)
	{
		return TRUE;
	}
	return FALSE;
}

EventResult CBaseEventProviderContext::OnBeforeSHSimulateDrop(IDataObject *pDataObj, DWORD *pdwEffect)
{
	std::list<std::wstring> srcList;
	if (!eventParser_->GetOLEDropFiles(pDataObj, srcList) || srcList.empty())
	{
		return kEventAllow;
	}

    std::wstring destFilePath;
    if (!nextlabs::comhelper::GetExplorerPath(destFilePath))
    {
        return kEventAllow;
    }
   
    if (destFilePath.empty())
    {
        WCHAR tempPath[MAX_PATH] = {0};
        SHGetSpecialFolderPath(0, tempPath, CSIDL_DESKTOPDIRECTORY, 0);
        destFilePath = tempPath;
    }

	destFilePath += L"\\";

	std::vector<nextlabs::comhelper::FILEOP> vecFileOP;   
	for (std::list<std::wstring>::const_iterator itor = srcList.begin(); itor != srcList.end(); ++itor)
	{
		nextlabs::comhelper::FILEOP fileOp;
		fileOp.strSrc = itor->c_str();
		fileOp.strDst = destFilePath; 
		vecFileOP.push_back(fileOp); 
	}

    if (eventParser_->IsCopyAction(pdwEffect))
    {
        return EventBeforeCopyFiles(vecFileOP);
    }
    else if (eventParser_->IsMoveAction(pdwEffect))
    {
        return EventBeforeMoveFiles(vecFileOP);
    }
	
    return kEventAllow;
}

bool CBaseEventProviderContext::Query(std::wstring& DisplayText, HDC hdc)
{
	DWORD ProcessId = 0;

	if (!GetHDCInfo(ProcessId, hdc))
	{
		return true;
	}

	return Query(DisplayText, ProcessId);
}

bool CBaseEventProviderContext::Query(std::wstring& DisplayText, HWND hWnd)
{
	DWORD ProcessID = 0;

	if (NULL != hWnd && hWnd != DesktophWnd)
	{
		GetWindowThreadProcessId(hWnd, &ProcessID);	
	}

	return Query(DisplayText, ProcessID);
}

bool CBaseEventProviderContext::Query(std::wstring& DisplayText, DWORD ProcessID)
{
	std::string strQuery = QueryFormat + boost::lexical_cast<std::string>(ProcessID);
	strQuery.resize(QueryLength);

	DWORD SessionID = 0;
	ProcessIdToSessionId(GetCurrentProcessId(), &SessionID);

	try
	{
		io_service ios;

		ip::tcp::socket sock(ios);
		ip::tcp::endpoint ep(ip::address::from_string(SCEServerIP), SCEServerBasedPort + static_cast<USHORT>(SessionID));

		sock.connect(ep);

		write(sock, buffer(strQuery));

		std::vector<char> str(QueryResultLength);

		read(sock, buffer(str));

		return Parse(DisplayText, &str[0]);
	}
	catch (...)
	{
		return true;			
	}
}

bool CBaseEventProviderContext::GetHDCInfo(DWORD& ProcessId, HDC hdc)
{
	boost::mutex::scoped_lock lock(ms_Mutex);

	std::map<HDC, DWORD>::const_iterator cit = ms_hdcInfo.find(hdc);

	if (cit == ms_hdcInfo.end())
	{
		return false;
	}

	ProcessId = cit->second;

	return true;
}

bool CBaseEventProviderContext::Parse(std::wstring& DisplayText, const std::string& str)
{
	cregex reg = cregex::compile(QueryResultFormat);

	cmatch what;

	try
	{
		regex_match(str.c_str(), what, reg);

		if (what.size() < 4)
		{
			return false;
		}

		const std::string QueryFlag = what[1];

		if (boost::algorithm::iequals(QueryFlag, AllowFlag))
		{
			return true;
		}
		else if (boost::algorithm::iequals(QueryFlag, DenyFlag))
		{
			DisplayText = ScreenCaptureAuxiliary::GetInstance().stringTowsting(what[3]);

			if (DisplayText.empty())
			{
				DisplayText = DefaultText;
			}
		}

		return false;
	}
	catch (...)
	{
		return false;			
	}
}

void CBaseEventProviderContext::AddHDC(HDC hdc, DWORD ProcessId)
{
	boost::mutex::scoped_lock lock(ms_Mutex);

	ms_hdcInfo[hdc] = ProcessId;
}

void CBaseEventProviderContext::AddHDC(HDC hdc, HWND hWnd)
{
	if (NULL == hdc)
	{
		return;
	}

	DWORD ProcessId = 0;

	if (hWnd == DesktophWnd)
	{
		AddHDC(hdc, ProcessId);
	}
	else
	{
		GetWindowThreadProcessId(hWnd, &ProcessId);

		AddHDC(hdc, ProcessId);
	}
}

void CBaseEventProviderContext::RemoveHDC(HDC hdc)
{
	boost::mutex::scoped_lock lock(ms_Mutex);

	ms_hdcInfo.erase(hdc);
}

BOOL CBaseEventProviderContext::HandleEFSObligation(LPCWSTR lpFileName)
{
	//OutputDebugStringW(__FUNCTIONW__);

	BOOL bRet = TRUE;

	DWORD le = GetLastError();
	if (Hooked_EncryptFileW_Next == NULL)
	{
		bRet = EncryptFileW(lpFileName);
	}
	else
	{
		bRet = Hooked_EncryptFileW_Next(lpFileName);
	}
	SetLastError(le);

	return bRet;
}

EventResult CBaseEventProviderContext::EventAfterNewDirectory(const std::wstring& strPath, PVOID pUserData)
{
	return kEventAllow;
}

EventResult CBaseEventProviderContext::EventAfterCopyFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData)
{
	return kEventAllow;
}

EventResult CBaseEventProviderContext::EventAfterMoveFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData)
{
	if (pUserData != NULL)
	{
		nextlabs::Obligations* pob = (nextlabs::Obligations*)pUserData;
		if (pob->IsObligationSet(WDE_OBLIGATION_ENCRYPT_NAME))
		{
			HandleEFSObligation(lpNewFileName);
		}
	}

	return kEventAllow;
}

EventResult CBaseEventProviderContext::OnBeforeCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, 
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
 
#pragma endregion

}  // ns nextlabs




