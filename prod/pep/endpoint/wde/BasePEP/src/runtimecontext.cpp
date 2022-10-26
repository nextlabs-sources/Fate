#include "runtimecontext.h"

#pragma warning(push)
#pragma warning(disable: 4819)  // ignored character code page issue  
#include <madCHook.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4244 4819 4512)
#include <boost/thread.hpp>  
#include <boost/function.hpp> 
#pragma warning(pop)

#include "commonutils.hpp"
#include "detours.h"

namespace
{
    typedef boost::shared_lock<boost::shared_mutex> boost_share_lock;  
    typedef boost::unique_lock<boost::shared_mutex> boost_unique_lock; 

    boost::shared_mutex gMutex; 

	const unsigned int kCOMRenameItemIndex = 12;
	const unsigned int kCOMRenameItemsIndex = 13;
    const unsigned int kCOMMoveItemsIndex = 15; 
    const unsigned int kCOMCopyItemsIndex = 17;
    const unsigned int kCOMDeleteItemIndex = 18;
    const unsigned int kCOMDeleteItemsIndex = 19;
	const unsigned int kCOMNewItemIndex = 20;
    const unsigned int kCOMPerformOperationsIndex = 21;

    const unsigned int kCOMGetThumbnail = 3;

	const unsigned int kCOMShow = 3;
}  // namespace anonymous 


// nextlabs pep common

//#include <celog.h>
// end nextlabs pep common


// global variables
nextlabs::CRuntimeContext* gContext_ = NULL;
// end global variables

nextlabs::recursion_control nextlabs::CRuntimeContext::hook_control;


namespace nextlabs
{

void CRuntimeContext::exception_cb(NLEXCEPT_CBINFO* cbinfo)
{
	hook_control.process_disable(); /* prevent recursion when handling an exception */

	if (cbinfo != NULL)
	{
		wchar_t comp_root[MAX_PATH] = { 0 }; // component root for WDE
		if (NLConfig::GetComponentInstallPath(L"Compliant Enterprise\\Desktop Enforcer", comp_root, _countof(comp_root)) == true)
		{
			wcsncat_s(comp_root, _countof(comp_root), L"\\diags\\dumps", _TRUNCATE);
			wcsncpy_s(cbinfo->dump_root, _countof(cbinfo->dump_root), comp_root, _TRUNCATE);
			cbinfo->use_dump_root = 1;
		}
		//  	 		CELOG_LOG(CELOG_CRITICAL,L"EXCEPTION 0x%X : PID %d TID %d : %hs [%d] \n",
		//  	 			cbinfo->code,GetCurrentProcessId(), GetCurrentThreadId(),
		//  	 			cbinfo->source_file,cbinfo->source_line);
	}
}/* exception_cb */


#pragma region FileOperations

PF_CreateFileW				CRuntimeContext::Hooked_CreateFileW_Next = NULL;
PF_CreateFileMappingW       CRuntimeContext::Hooked_CreateFileMappingW_Next = NULL;
PF_CloseHandle				CRuntimeContext::Hooked_CloseHandle_Next = NULL;
PF_FindFirstFileExW         CRuntimeContext::Hooked_FindFirstFileExW_Next = NULL;
PF_CreateDirectoryW			CRuntimeContext::Hooked_CreateDirectoryW_Next = NULL;
PF_RemoveDirectoryW			CRuntimeContext::Hooked_RemoveDirectoryW_Next = NULL;
PF_DeleteFileW				CRuntimeContext::Hooked_DeleteFileW_Next = NULL;
PF_ReadFile					CRuntimeContext::Hooked_ReadFile_Next = NULL;
PF_ReadFileEx				CRuntimeContext::Hooked_ReadFileEx_Next = NULL;
PF_WriteFile				CRuntimeContext::Hooked_WriteFile_Next = NULL;
PF_WriteFileEx				CRuntimeContext::Hooked_WriteFileEx_Next = NULL;
PF_SetEndOfFile				CRuntimeContext::Hooked_SetEndOfFile_Next = NULL;
PF_SetEndOfFile				CRuntimeContext::Hooked_KernelBaseSetEndOfFile_Next = NULL;
PF_CopyFileW				CRuntimeContext::Hooked_CopyFileW_Next = NULL;
PF_CopyFileExW				CRuntimeContext::Hooked_CopyFileExW_Next = NULL;
PF_PrivCopyFileExW	        CRuntimeContext::Hooked_PrivCopyFileExW_Next = NULL;
PF_PrivCopyFileExW	        CRuntimeContext::Hooked_KernelBasePrivCopyFileExW_Next = NULL;
PF_MoveFileW				CRuntimeContext::Hooked_MoveFileW_Next = NULL;
PF_MoveFileExW				CRuntimeContext::Hooked_MoveFileExW_Next = NULL;
PF_MoveFileWithProgressW	CRuntimeContext::Hooked_MoveFileWithProgressW_Next = NULL;
PF_MoveFileExW				CRuntimeContext::Hooked_KernelBaseMoveFileExW_Next = NULL;
PF_MoveFileWithProgressW	CRuntimeContext::Hooked_KernelBaseMoveFileWithProgressW_Next = NULL;
PF_ReplaceFileW				CRuntimeContext::Hooked_ReplaceFileW_Next = NULL;
PF_CreateHardLinkW			CRuntimeContext::Hooked_CreateHardLinkW_Next = NULL;
PF_SetFileAttributesW       CRuntimeContext::Hooked_SetFileAttributesW_Next = NULL;
PF_SetNamedSecurityInfoW     CRuntimeContext::Hooked_SetNamedSecurityInfoW_Next = NULL;
PF_GetFileAttributesW       CRuntimeContext::Hooked_GetFileAttributesW_Next = NULL;
PF_AddUsersToEncryptedFile  CRuntimeContext::Hooked_AddUsersToEncryptedFile_Next = NULL;
PF_SetFileInformationByHandle CRuntimeContext::Hooked_SetFileInformationByHandle_Next = NULL;
PF_EncryptFileW             CRuntimeContext::Hooked_EncryptFileW_Next = NULL;
PF_DecryptFileW             CRuntimeContext::Hooked_DecryptFileW_Next = NULL;
PF_SHFileOperationW			CRuntimeContext::Hooked_SHFileOperationW_Next = NULL;

PF_SetClipboardData			CRuntimeContext::Hooked_SetClipboardData_Next = NULL;
PF_GetClipboardData			CRuntimeContext::Hooked_GetClipboardData_Next = NULL;
PF_OleSetClipboard			CRuntimeContext::Hooked_OleSetClipboard_Next = NULL;
PF_OleGetClipboard			CRuntimeContext::Hooked_OleGetClipboard_Next = NULL;

PF_DoDragDrop				CRuntimeContext::Hooked_DoDragDrop_Next = NULL;
PF_RegisterDragDrop			CRuntimeContext::Hooked_RegisterDragDrop_Next = NULL;
PF_RevokeDragDrop			CRuntimeContext::Hooked_RevokeDragDrop_Next = NULL;

PF_NtCreateFile             CRuntimeContext::Hooked_NtCreateFile_Next = NULL;
PF_NtOpenFile               CRuntimeContext::Hooked_NtOpenFile_Next = NULL;
PF_NtClose                  CRuntimeContext::Hooked_NtClose_Next = NULL;
PF_RtlDosPathNameToNtPathName_U_WithStatus  CRuntimeContext::Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next = NULL;
PF_NtSetSecurityObject CRuntimeContext::Hooked_NtSetSecurityObject_Next = NULL;

PF_GetSaveFileNameW			CRuntimeContext::Hooked_GetSaveFileNameW_Next = NULL;
PF_GetOpenFileNameW         CRuntimeContext::Hooked_GetOpenFileNameW_Next = NULL;

PF_SHSimulateDrop           CRuntimeContext::Hooked_SHSimulateDrop_Next = NULL;

PF_BitBlt				CRuntimeContext::Hooked_BitBlt_Next = NULL;
PF_MaskBlt				CRuntimeContext::Hooked_MaskBlt_Next = NULL;
PF_PlgBlt				CRuntimeContext::Hooked_PlgBlt_Next = NULL;
PF_StretchBlt			CRuntimeContext::Hooked_StretchBlt_Next = NULL;
PF_PrintWindow			CRuntimeContext::Hooked_PrintWindow_Next = NULL;
PF_CreateDCA			CRuntimeContext::Hooked_CreateDCA_Next = NULL;
PF_CreateDCW			CRuntimeContext::Hooked_CreateDCW_Next = NULL;
PF_DeleteDC				CRuntimeContext::Hooked_DeleteDC_Next = NULL;
PF_GetDC				CRuntimeContext::Hooked_GetDC_Next = NULL;
PF_GetDCEx				CRuntimeContext::Hooked_GetDCEx_Next = NULL;
PF_GetWindowDC			CRuntimeContext::Hooked_GetWindowDC_Next = NULL;
PF_ReleaseDC			CRuntimeContext::Hooked_ReleaseDC_Next = NULL;

HANDLE	WINAPI	CRuntimeContext::Hooked_CreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile)
{
	if (hook_control.is_disabled()){
		return Hooked_CreateFileW_Next(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,	dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);

	}
	if (!gContext_){
		return Hooked_CreateFileW_Next(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	// detour to runtime context
	__try
	{
		return gContext_->MyCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI CRuntimeContext::Hooked_CreateFileMappingW( HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName )
{
    if (hook_control.is_disabled()){
        return Hooked_CreateFileMappingW_Next(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);

    }
    if (!gContext_){
        return Hooked_CreateFileMappingW_Next(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }

    // detour to runtime context
    __try
    {
        return gContext_->MyCreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }

    return INVALID_HANDLE_VALUE;
}

BOOL WINAPI	CRuntimeContext::Hooked_CloseHandle(HANDLE hObject)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CloseHandle_Next(hObject);
	}
	if (!gContext_)
	{
		return Hooked_CloseHandle_Next(hObject);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCloseHandle(hObject);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

HANDLE WINAPI CRuntimeContext::Hooked_FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
    if (hook_control.is_disabled())
    {
        return Hooked_FindFirstFileExW_Next(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
    }
    if (!gContext_)
    {
        return Hooked_FindFirstFileExW_Next(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyFindFirstFileExW(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }

    return INVALID_HANDLE_VALUE;
}
BOOL WINAPI	CRuntimeContext::Hooked_CreateDirectoryW(LPCWSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CreateDirectoryW_Next(lpPathName, lpSecurityAttributes);
	}
	if (!gContext_)
	{
		return Hooked_CreateDirectoryW_Next(lpPathName, lpSecurityAttributes);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCreateDirectoryW(lpPathName, lpSecurityAttributes);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_RemoveDirectoryW(LPCWSTR lpPathName)
{
	if (hook_control.is_disabled())
	{
		return Hooked_RemoveDirectoryW_Next(lpPathName);
	}
	if (!gContext_)
	{
		return Hooked_RemoveDirectoryW_Next(lpPathName);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyRemoveDirectoryW(lpPathName);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_DeleteFileW(LPCWSTR lpFileName)
{
	if (hook_control.is_disabled())
	{
		return Hooked_DeleteFileW_Next(lpFileName);
	}
	if (!gContext_)
	{
		return Hooked_DeleteFileW_Next(lpFileName);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyDeleteFileW(lpFileName);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	if (hook_control.is_disabled())
	{
		return Hooked_ReadFile_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
	if (!gContext_)
	{
		return Hooked_ReadFile_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	if (hook_control.is_disabled())
	{
		return Hooked_ReadFileEx_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}
	if (!gContext_)
	{
		return Hooked_ReadFileEx_Next(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	if (hook_control.is_disabled())
	{
		return Hooked_WriteFile_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	if (!gContext_)
	{
		return Hooked_WriteFile_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	}
	// detour to runtime context
	__try
	{
		return gContext_->MyWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	if (hook_control.is_disabled())
	{
		return Hooked_WriteFileEx_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
	}
	if (!gContext_)
	{
		return Hooked_WriteFileEx_Next(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyWriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_SetEndOfFile(HANDLE hFile)
{
	if (hook_control.is_disabled())
	{
		return Hooked_SetEndOfFile_Next(hFile);
	}
	if (!gContext_)
	{
		return Hooked_SetEndOfFile_Next(hFile);
	}

	__try
	{
		return gContext_->MySetEndOfFile(hFile);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_KernelBaseSetEndOfFile(HANDLE hFile)
{
	if (hook_control.is_disabled())
	{
		return Hooked_KernelBaseSetEndOfFile_Next(hFile);
	}
	if (!gContext_)
	{
		return Hooked_KernelBaseSetEndOfFile_Next(hFile);
	}

	__try
	{
		return gContext_->MyKernelBaseSetEndOfFile(hFile);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CopyFileW_Next(lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	if (!gContext_)
	{
		return Hooked_CopyFileW_Next(lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	if (!gContext_)
	{
		return Hooked_CopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_PrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    if (hook_control.is_disabled())
    {
        return Hooked_PrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    if (!gContext_)
    {
        return Hooked_PrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyPrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }

    return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_KernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    if (hook_control.is_disabled())
    {
        return Hooked_KernelBasePrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    if (!gContext_)
    {
        return Hooked_KernelBasePrivCopyFileExW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyKernelBasePrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }

    return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	if (hook_control.is_disabled())
	{
		return Hooked_MoveFileW_Next(lpExistingFileName, lpNewFileName);
	}
	if (!gContext_)
	{
		return Hooked_MoveFileW_Next(lpExistingFileName, lpNewFileName);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyMoveFileW(lpExistingFileName, lpNewFileName);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD  dwFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_MoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);
	}
	if (!gContext_)
	{
		return Hooked_MoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_MoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	if (!gContext_)
	{
		return Hooked_MoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyMoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_KernelBaseMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD  dwFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_KernelBaseMoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);
	}
	if (!gContext_)
	{
		return Hooked_KernelBaseMoveFileExW_Next(lpExistingFileName, lpNewFileName, dwFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyKernelBaseMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_KernelBaseMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_KernelBaseMoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	if (!gContext_)
	{
		return Hooked_KernelBaseMoveFileWithProgressW_Next(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyKernelBaseMoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_ReplaceFileW(LPCWSTR lpReplacedFileName, LPCWSTR lpReplacementFileName, LPCWSTR lpBackupFileName, DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID  lpReserved)
{
	if (hook_control.is_disabled())
	{
		return Hooked_ReplaceFileW_Next(lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved);
	}
	if (!gContext_)
	{
		return Hooked_ReplaceFileW_Next(lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved);

	}
	// detour to runtime context
	__try
	{
		return gContext_->MyReplaceFileW(lpReplacedFileName, lpReplacementFileName, lpBackupFileName, dwReplaceFlags, lpExclude, lpReserved);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI	CRuntimeContext::Hooked_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CreateHardLinkW_Next(lpFileName, lpExistingFileName, lpSecurityAttributes);
	}
	if (!gContext_)
	{
		return Hooked_CreateHardLinkW_Next(lpFileName, lpExistingFileName, lpSecurityAttributes);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCreateHardLinkW(lpFileName, lpExistingFileName, lpSecurityAttributes);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes)
{
    if (hook_control.is_disabled())
    {
        return Hooked_SetFileAttributesW_Next(lpFileName, dwFileAttributes);
    }
    if (!gContext_)
    {
        return Hooked_SetFileAttributesW_Next(lpFileName, dwFileAttributes);
    }
    // detour to runtime context
    __try
    {
        //OutputDebugStringW(L"CRuntimeContext::Hooked_SetFileAttributesW");
        return gContext_->MySetFileAttributesW(lpFileName, dwFileAttributes);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return FALSE;
}

DWORD WINAPI CRuntimeContext::Hooked_GetFileAttributesW(LPCWSTR lpFileName)
{
    if (hook_control.is_disabled())
    {
        return Hooked_GetFileAttributesW_Next(lpFileName);
    }
    if (!gContext_)
    {
        return Hooked_GetFileAttributesW_Next(lpFileName);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyGetFileAttributesW(lpFileName);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return INVALID_FILE_ATTRIBUTES;
}

DWORD WINAPI CRuntimeContext::Hooked_SetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl)
{
    if (hook_control.is_disabled())
    {
        return Hooked_SetNamedSecurityInfoW_Next(pObjectName, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
    }
    if (!gContext_)
    {
        return Hooked_SetNamedSecurityInfoW_Next(pObjectName, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MySetNamedSecurityInfoW(pObjectName, ObjectType, SecurityInfo, psidOwner, psidGroup, pDacl, pSacl);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return ERROR_ACCESS_DENIED;
}

DWORD WINAPI CRuntimeContext::Hooked_AddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers)
{
    if (hook_control.is_disabled())
    {
        return Hooked_AddUsersToEncryptedFile(lpFileName, pUsers);
    }
    if (!gContext_)
    {
        return Hooked_AddUsersToEncryptedFile(lpFileName, pUsers);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyAddUsersToEncryptedFile(lpFileName, pUsers);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
     return ERROR_ACCESS_DENIED;
}

BOOL WINAPI CRuntimeContext::Hooked_SetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInfomationClass, LPVOID lpFileInformation, DWORD dwBufferSize)
{
    if (hook_control.is_disabled())
    {
        return Hooked_SetFileInformationByHandle_Next(hFile, FileInfomationClass, lpFileInformation, dwBufferSize);
    }
    if (!gContext_)
    {
        return Hooked_SetFileInformationByHandle_Next(hFile, FileInfomationClass, lpFileInformation, dwBufferSize);
    }
    // detour to runtime context
    __try
    {
        //OutputDebugStringW(L"CRuntimeContext::Hooked_SetFileInformationByHandle");
        return gContext_->MySetFileInformationByHandle(hFile, FileInfomationClass, lpFileInformation, dwBufferSize);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_EncryptFileW(LPCWSTR lpFileName)
{
    if (hook_control.is_disabled())
    {
        return Hooked_EncryptFileW_Next(lpFileName);
    }
    if (!gContext_)
    {
        return Hooked_EncryptFileW_Next(lpFileName);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyEncryptFileW(lpFileName);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_DecryptFileW(LPCWSTR lpFileName, DWORD dwReserved)
{
    if (hook_control.is_disabled())
    {
        return Hooked_DecryptFileW_Next(lpFileName, dwReserved);
    }
    if (!gContext_)
    {
        return Hooked_DecryptFileW_Next(lpFileName, dwReserved);
    }
    // detour to runtime context
    __try
    {
        return gContext_->MyDecryptFileW(lpFileName, dwReserved);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return FALSE;
}

int WINAPI CRuntimeContext::Hooked_SHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation)
{
	//OutputDebugStringW(L"Hooked_SHFileOperation-----------------");
	if (hook_control.is_disabled()) // todo need to judge the OS version.
	{
		//OutputDebugStringW(L"Hooked_SHFileOperation   hook_control.is_disabled()-----------------");
		return Hooked_SHFileOperationW_Next(lpFileOperation);
	}
	if (!gContext_)
	{
		return Hooked_SHFileOperationW_Next(lpFileOperation);
	}
	// detour to runtime context
	__try
	{
		//OutputDebugStringW(L"Hooked_SHFileOperation  enter MySHFileOperationW-----------------");
		return gContext_->MySHFileOperationW(lpFileOperation);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	return FALSE;
}

HANDLE CRuntimeContext::Hooked_SetClipboardData(UINT uFormat, HANDLE hMem)
{
	if (hook_control.is_disabled())
	{
		return Hooked_SetClipboardData_Next(uFormat, hMem);

	}
	if (!gContext_)
	{
		return Hooked_SetClipboardData_Next(uFormat, hMem);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MySetClipboardData(uFormat, hMem);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;

}

HANDLE CRuntimeContext::Hooked_GetClipboardData(UINT uFormat)
{
	if (hook_control.is_disabled())
	{
		return Hooked_GetClipboardData_Next(uFormat);
	}
	if (!gContext_)
	{
		return Hooked_GetClipboardData_Next(uFormat);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyGetClipboardData(uFormat);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return INVALID_HANDLE_VALUE;
}

HRESULT CRuntimeContext::Hooked_OleSetClipboard(LPDATAOBJECT pDataObj)
{
	if (hook_control.is_disabled())
	{
		return Hooked_OleSetClipboard_Next(pDataObj);
	}
	if (!gContext_)
	{
		return Hooked_OleSetClipboard_Next(pDataObj);
	}

	__try
	{
		return gContext_->MyOleSetClipboard(pDataObj);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;
	}
	return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_OleGetClipboard(LPDATAOBJECT *pDataObj)
{
	if (hook_control.is_disabled())
	{
		return Hooked_OleGetClipboard_Next(pDataObj);
	}
	if (!gContext_)
	{
		return Hooked_OleGetClipboard_Next(pDataObj);
	}

	__try
	{
		return gContext_->MyOleGetClipboard(pDataObj);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;
	}

    return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_DoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect)
{
	if (hook_control.is_disabled())
	{
		return Hooked_DoDragDrop_Next(pDataObj, pDropSource, dwOkEffects, pdwEffect);
	}
	if (!gContext_)
	{
		return Hooked_DoDragDrop_Next(pDataObj, pDropSource, dwOkEffects, pdwEffect);
	}

	__try
	{
		return gContext_-> MyDoDragDrop(pDataObj, pDropSource, dwOkEffects, pdwEffect);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;
	}
    
    return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_RegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget)
{
	if (hook_control.is_disabled())
	{
		return Hooked_RegisterDragDrop_Next(hwnd, pDropTarget);
	}
	if (!gContext_)
	{
		return Hooked_RegisterDragDrop_Next(hwnd, pDropTarget);
	}

	__try
	{
		return gContext_-> MyRegisterDragDrop(hwnd, pDropTarget);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;
	}

    return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_RevokeDragDrop(HWND hwnd)
{
	if (hook_control.is_disabled())
	{
		return Hooked_RevokeDragDrop_Next(hwnd);
	}
	if (!gContext_)
	{
		return Hooked_RevokeDragDrop_Next(hwnd);
	}

	__try
	{
		return gContext_->MyRevokeDragDrop(hwnd);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;
	}

    return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_SHSimulateDrop(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect)
{
    if (hook_control.is_disabled())
    {
        return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
    }
    if (!gContext_)
    {
        return Hooked_SHSimulateDrop_Next(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
    }

    __try
    {
        return gContext_->MySHSimulateDrop(pDropTarget, pDataObj, grfKeyState, pt, pdwEffect);
    }
    __except(NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;
    }

    return E_FAIL;
}

NTSTATUS NTAPI CRuntimeContext::Hooked_NtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength)
{
    if (hook_control.is_disabled())
    {
        return Hooked_NtCreateFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
    }

    if (!gContext_)
    {
        return Hooked_NtCreateFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
    }

    __try
    {
        return gContext_->MyNtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return STATUS_CANCELLED;
}

NTSTATUS NTAPI CRuntimeContext::Hooked_NtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions)
{
    if (hook_control.is_disabled())
    {
        return Hooked_NtOpenFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    }

    if (!gContext_)
    {
        return Hooked_NtOpenFile_Next(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    }

    __try
    {
        return gContext_->MyNtOpenFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, ShareAccess, OpenOptions);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return STATUS_CANCELLED;
}

NTSTATUS NTAPI CRuntimeContext::Hooked_NtClose(HANDLE handle)
{
    if (hook_control.is_disabled())
    {
        return Hooked_NtClose_Next(handle);
    }

    if (!gContext_)
    {
        return Hooked_NtClose_Next(handle);
    }

    __try
    {
        return gContext_->MyNtClose(handle);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return STATUS_CANCELLED;
}

NTSTATUS NTAPI CRuntimeContext::Hooked_RtlDosPathNameToNtPathName_U_WithStatus(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName)
{
    if (hook_control.is_disabled())
    {
        return Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next(DosFileName, NtFileName, FilePart, RelativeName);
    }

    if (!gContext_)
    {
        return Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next(DosFileName, NtFileName, FilePart, RelativeName);
    }

    __try
    {
        return gContext_->MyRtlDosPathNameToNtPathName_U_WithStatus(DosFileName, NtFileName, FilePart, RelativeName);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return 0xC0000001L;
}

NTSTATUS NTAPI CRuntimeContext::Hooked_NtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor)
{
    if (hook_control.is_disabled())
    {
        return Hooked_NtSetSecurityObject_Next(handle, securityInformation, securityDescriptor);
    }

    if (!gContext_)
    {
        return Hooked_NtSetSecurityObject_Next(handle, securityInformation, securityDescriptor);
    }

    __try
    {
        return gContext_->MyNtSetSecurityObject(handle, securityInformation, securityDescriptor);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;/* empty */
    }
    return STATUS_ACCESS_DENIED;
}

#pragma endregion

#pragma region ModuleProcessOperation

PF_CreateProcessW		CRuntimeContext::Hooked_CreateProcessW_Next = NULL;
PF_ExitProcess			CRuntimeContext::Hooked_ExitProcess_Next = NULL;
PF_TerminateProcess		CRuntimeContext::Hooked_TerminateProcess_Next = NULL;
PF_LoadLibraryW			CRuntimeContext::Hooked_LoadLibraryW_Next = NULL;
PF_LoadLibraryExW		CRuntimeContext::Hooked_LoadLibraryExW_Next = NULL;
PF_DeviceIoControl		CRuntimeContext::Hooked_DeviceIoControl_Next = NULL;
PF_CoCreateInstance		CRuntimeContext::Hooked_CoCreateInstance_Next = NULL;

BOOL WINAPI CRuntimeContext::Hooked_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CreateProcessW_Next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	if (!gContext_)
	{
		return Hooked_CreateProcessW_Next(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
			dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

VOID WINAPI CRuntimeContext::Hooked_ExitProcess(UINT uExitCode)
{
	if (hook_control.is_disabled())
	{
		Hooked_ExitProcess_Next(uExitCode);
		return;
	}
	if (!gContext_)
	{
		Hooked_ExitProcess_Next(uExitCode);

		return;
	}
	// detour to runtime context
	__try
	{
		gContext_->MyExitProcess(uExitCode);
		return;
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

}

BOOL WINAPI CRuntimeContext::Hooked_TerminateProcess(HANDLE hProcess, UINT uExitCode)
{
	if (hook_control.is_disabled())
	{
		return Hooked_TerminateProcess_Next(hProcess, uExitCode);
	}
	if (!gContext_)
	{
		return Hooked_TerminateProcess_Next(hProcess, uExitCode);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyTerminateProcess(hProcess, uExitCode);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

HMODULE	WINAPI CRuntimeContext::Hooked_LoadLibraryW(LPCWSTR lpLibFileName)
{
	if (hook_control.is_disabled())
	{
		return Hooked_LoadLibraryW_Next(lpLibFileName);
	}
	if (!gContext_)
	{
		return Hooked_LoadLibraryW_Next(lpLibFileName);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyLoadLibraryW(lpLibFileName);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return NULL;
}

HMODULE WINAPI CRuntimeContext::Hooked_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_LoadLibraryExW_Next(lpLibFileName, hFile, dwFlags);
	}
	if (!gContext_)
	{
		return Hooked_LoadLibraryExW_Next(lpLibFileName, hFile, dwFlags);

	}
	// detour to runtime context
	__try
	{
		return gContext_->MyLoadLibraryExW(lpLibFileName, hFile, dwFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return NULL;
}

BOOL WINAPI CRuntimeContext::Hooked_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize,
	LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
	if (hook_control.is_disabled())
	{
		return Hooked_DeviceIoControl_Next(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
	}
	if (!gContext_)
	{
		return Hooked_DeviceIoControl_Next(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

HRESULT WINAPI CRuntimeContext::Hooked_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CoCreateInstance_Next(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
	if (!gContext_)
	{
		return Hooked_CoCreateInstance_Next(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return E_FAIL;
}

#pragma endregion

#pragma region COMFileOperation

HRESULT CRuntimeContext::Hooked_COMPerformOperations(IFileOperation * This)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMPerformOperations next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pPerformOperations = pVTable[kCOMPerformOperationsIndex];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pPerformOperations);

        if(iter == gContext_->mapCOMHooks_.end())
        {
            //OutputDebugStringW(L"exception in Hooked_COMPerformOperations.");
            return S_FALSE;
        }

        next_func = (PF_COMPerformOperations)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This);
    }
    
    return gContext_->MyCOMPerformOperations(This, next_func);
 /*   __try
    {
        return gContext_->MyCOMPerformOperations(This, next_func);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;
    }*/

    //return E_FAIL;
}

HRESULT CRuntimeContext::Hooked_COMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem)
{
	if(!gContext_)
	{
		return E_FAIL;
	}
	PF_COMNewItem next_func = NULL;
	{
		LPVOID* pVTable = (*(LPVOID**)This); // the v table of the object
		LPVOID pNewItem = pVTable[kCOMNewItemIndex];

		boost_share_lock lockReader(gMutex);
		std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pNewItem);
		if (iter == gContext_->mapCOMHooks_.end())
		{
			//OutputDebugStringW(L"exception in Hooked_COMNewItem.");
			return S_FALSE;
		}
		next_func = (PF_COMNewItem)(*iter).second;
	}
	if (hook_control.is_disabled())
	{
		return next_func(This, psiDestinationFolder, dwFileAttributes, pszName, pszTemplateName, pfopsItem);
	}
	return gContext_->MyCOMNewItem(This, psiDestinationFolder, dwFileAttributes, pszName, pszTemplateName, pfopsItem, next_func);
}

HRESULT CRuntimeContext::Hooked_COMCopyItems(IFileOperation* This, __RPC__in_opt IUnknown *punkItems, __RPC__in_opt IShellItem *psiDestinationFolder)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMCopyItems next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pCopyItems = pVTable[kCOMCopyItemsIndex];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pCopyItems);

        if(iter == gContext_->mapCOMHooks_.end())
        {
            //OutputDebugStringW(L"exception in Hooked_COMCopyItems.");
            return S_FALSE;
        }

        next_func = (PF_COMCopyItems)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This, punkItems, psiDestinationFolder);
    }

    return gContext_->MyCOMCopyItems(This, punkItems, psiDestinationFolder, next_func);

/*    __try
    {
        return gContext_->MyCOMCopyItems(This, punkItems, psiDestinationFolder, next_func);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;
    }

    return E_FAIL;*/
}

HRESULT CRuntimeContext::Hooked_COMMoveItems(IFileOperation* This, __RPC__in_opt IUnknown *punkItems, __RPC__in_opt IShellItem *psiDestinationFolder)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMMoveItems next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pMoveItems = pVTable[kCOMMoveItemsIndex];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pMoveItems);

        if(iter == gContext_->mapCOMHooks_.end())
        {
            //OutputDebugStringW(L"exception in Hooked_COMMoveItems.");
            return S_FALSE;
        }

        next_func = (PF_COMMoveItems)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This, punkItems, psiDestinationFolder);
    }

    return gContext_->MyCOMMoveItems(This, punkItems, psiDestinationFolder, next_func);
}

HRESULT CRuntimeContext::Hooked_COMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem)
{
	if (!gContext_)
	{
		return E_FAIL;
	}

	PF_COMRenameItem next_func = NULL;
	{
		LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
		LPVOID pRenameItem = pVTable[kCOMRenameItemIndex];

		boost_share_lock lockReader(gMutex);
		std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pRenameItem);

		if(iter == gContext_->mapCOMHooks_.end())
		{
			//OutputDebugStringW(L"exception in Hooked_COMRenameItem.  -------------- Hooked_COMRenameItem");
			return S_FALSE;
		}

		next_func = (PF_COMRenameItem)(*iter).second;
	}

	if (hook_control.is_disabled())
	{
		return next_func(This, psiDestinationFolder, pszNewName, pfopsItem);
	}

	return gContext_->MyCOMRenameItem(This, psiDestinationFolder, pszNewName, pfopsItem, next_func);
}

HRESULT CRuntimeContext::Hooked_COMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName)
{
	if (!gContext_)
	{
		return E_FAIL;
	}

	PF_COMRenameItems next_func = NULL;
	{
		LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
		LPVOID pRenameItems = pVTable[kCOMRenameItemsIndex];

		boost_share_lock lockReader(gMutex);
		std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pRenameItems);

		if(iter == gContext_->mapCOMHooks_.end())
		{
			//OutputDebugStringW(L"exception in Hooked_COMRenameItem.  -------------- Hooked_COMRenameItems");
			return S_FALSE;
		}

		next_func = (PF_COMRenameItems)(*iter).second;
	}

	if (hook_control.is_disabled())
	{
		return next_func(This, pUnkItems, pszNewName);
	}

	return gContext_->MyCOMRenameItems(This, pUnkItems, pszNewName, next_func);
}

HRESULT CRuntimeContext::Hooked_COMDeleteItems(IFileOperation* This, IUnknown *punkItems)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMDeleteItems next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pDeleteItems = pVTable[kCOMDeleteItemsIndex];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pDeleteItems);

        if(iter == gContext_->mapCOMHooks_.end())
        {
            //OutputDebugStringW(L"exception in Hooked_COMDeleteItems.");
            return S_FALSE;
        }

        next_func = (PF_COMDeleteItems)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This, punkItems);
    }

    return gContext_->MyCOMDeleteItems(This, punkItems, next_func);

/*    __try
    {
        return gContext_->MyCOMDeleteItems(This, punkItems, next_func);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;
    }

    return E_FAIL;*/
}

HRESULT CRuntimeContext::Hooked_COMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMDeleteItem next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pDeleteItem = pVTable[kCOMDeleteItemIndex];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pDeleteItem);

        if(iter == gContext_->mapCOMHooks_.end())
        {
            //OutputDebugStringW(L"exception in MyCOMDeleteItem.");
            return S_FALSE;
        }

        next_func = (PF_COMDeleteItem)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This, psiItem, pfopsItem);
    }

    return gContext_->MyCOMDeleteItem(This, psiItem, pfopsItem, next_func);

/*    __try
    {
        return gContext_->MyCOMDeleteItem(This, psiItem, pfopsItem, next_func);
    }
    __except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
    {
        ;
    }

    return E_FAIL;*/
}

HRESULT CRuntimeContext::Hooked_COMGetThumbnail(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID)
{
    if (!gContext_)
    {
        return E_FAIL;
    }

    PF_COMGetThumbnail next_func = NULL;
    {
        LPVOID* pVTable = (*(LPVOID**)This);//the v table of the object
        LPVOID pGetThumbnail = pVTable[kCOMGetThumbnail];

        boost_share_lock lockReader(gMutex);
        std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pGetThumbnail);

        if(iter == gContext_->mapCOMHooks_.end())
        {
			next_func = (PF_COMGetThumbnail)gContext_->mapCOMHooks_.begin()->second;
            return next_func(This, pShellItem, cxyRequestedThumbSize, flags, ppvThumb, pOutFlags, pThumbnailID);
        }

        next_func = (PF_COMGetThumbnail)(*iter).second;
    }

    if (hook_control.is_disabled())
    {
        return next_func(This, pShellItem, cxyRequestedThumbSize, flags, ppvThumb, pOutFlags, pThumbnailID);
    }

    return gContext_->MyCOMGetThumbnail(This, pShellItem, cxyRequestedThumbSize, flags, ppvThumb, pOutFlags, pThumbnailID, next_func);
}

HRESULT CRuntimeContext::Hooked_COMShow(IFileSaveDialog* pThis, HWND hwndOwner)
{
	if (!gContext_)
	{
		return E_FAIL;
	}

	PF_COMShow next_func = NULL;
	{
		LPVOID* pVTable = (*(LPVOID**)pThis);//the v table of the object
		LPVOID pShow = pVTable[kCOMShow];

		boost_share_lock lockReader(gMutex);
		std::map<LPVOID, LPVOID>::iterator iter = gContext_->mapCOMHooks_.find(pShow);

		if(iter == gContext_->mapCOMHooks_.end())
		{
			return S_FALSE;
		}

		next_func = (PF_COMShow)(*iter).second;
	}

	return gContext_->MyCOMShow(pThis, hwndOwner, next_func);
}

#pragma endregion

#pragma region NetworkOperation

PF_InternetConnectA CRuntimeContext::Hooked_InternetConnectA_Next = NULL;
PF_InternetConnectW CRuntimeContext::Hooked_InternetConnectW_Next = NULL;
PF_InternetCloseHandle CRuntimeContext::Hooked_InternetCloseHandle_Next = NULL;
PF_HttpOpenRequestA CRuntimeContext::Hooked_HttpOpenRequestA_Next = NULL;
PF_HttpOpenRequestW CRuntimeContext::Hooked_HttpOpenRequestW_Next = NULL;

HINTERNET WINAPI CRuntimeContext::Hooked_InternetConnectA(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext)
{
	if (hook_control.is_disabled())
	{
		return Hooked_InternetConnectA_Next(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	if (!gContext_)
	{
		return Hooked_InternetConnectA_Next(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyInternetConnectA(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	::SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}


HINTERNET WINAPI CRuntimeContext::Hooked_InternetConnectW(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext)
{
	if (hook_control.is_disabled())
	{
		return Hooked_InternetConnectW_Next(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	if (!gContext_)
	{
		return Hooked_InternetConnectW_Next(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyInternetConnectW(hInternet, lpszServerName, nServerPort, lpszUserName, lpszPassword, dwService, dwFlags, dwContext);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	::SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}

BOOL CRuntimeContext::Hooked_InternetCloseHandle(HINTERNET hInternet)
{
	if (hook_control.is_disabled())
	{
		return Hooked_InternetCloseHandle_Next(hInternet);
	}
	if (!gContext_)
	{
		return Hooked_InternetCloseHandle_Next(hInternet);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyInternetCloseHandle(hInternet);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	return FALSE;
}




HINTERNET WINAPI CRuntimeContext::Hooked_HttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{
	if (hook_control.is_disabled())
	{
		return Hooked_HttpOpenRequestA_Next(hConnect,lpszVerb,lpszObjectName,lpszVersion,lpszReferrer,lplpszAcceptTypes,dwFlags,dwContext);
	}
	if (!gContext_)
	{
		return Hooked_HttpOpenRequestA_Next(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyHttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	::SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}


HINTERNET WINAPI CRuntimeContext::Hooked_HttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
{
	if (hook_control.is_disabled())
	{
		return Hooked_HttpOpenRequestW_Next(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	}
	if (!gContext_)
	{
		return Hooked_HttpOpenRequestW_Next(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyHttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferrer, lplpszAcceptTypes, dwFlags, dwContext);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	::SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}

BOOL WINAPI CRuntimeContext::Hooked_GetSaveFileNameW(LPOPENFILENAMEW lpofn)
{
	if (hook_control.is_disabled())
	{
		return Hooked_GetSaveFileNameW_Next(lpofn);
	}
	if (!gContext_)
	{
		return Hooked_GetSaveFileNameW_Next(lpofn);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyGetSaveFileNameW(lpofn);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}
	::SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}

BOOL WINAPI CRuntimeContext::Hooked_GetOpenFileNameW(LPOPENFILENAMEW lpofn)
{
    if (hook_control.is_disabled())
    {
        return Hooked_GetOpenFileNameW_Next(lpofn);
    }
    if (!gContext_)
    {
        return Hooked_GetOpenFileNameW_Next(lpofn);
    }

    return gContext_->MyGetOpenFileNameW(lpofn);
}

BOOL WINAPI CRuntimeContext::Hooked_BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop)
{
	if (hook_control.is_disabled())
	{
		return Hooked_BitBlt_Next(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	}
	if (!gContext_)
	{
		return Hooked_BitBlt_Next(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_MaskBlt(HDC hdcDest, int xDest, int yDest, int width, int height, HDC hdcSrc, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask, DWORD rop)
{
	if (hook_control.is_disabled())
	{
		return Hooked_MaskBlt_Next(hdcDest, xDest, yDest, width, height, hdcSrc, xSrc, ySrc, hbmMask, xMask, yMask, rop);
	}
	if (!gContext_)
	{
		return Hooked_MaskBlt_Next(hdcDest, xDest, yDest, width, height, hdcSrc, xSrc, ySrc, hbmMask, xMask, yMask, rop);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyMaskBlt(hdcDest, xDest, yDest, width, height, hdcSrc, xSrc, ySrc, hbmMask, xMask, yMask, rop);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_PlgBlt(HDC hdcDest, CONST POINT * lpPoint, HDC hdcSrc, int xSrc, int ySrc, int width, int height, HBITMAP hbmMask, int xMask, int yMask)
{
	if (hook_control.is_disabled())
	{
		return Hooked_PlgBlt_Next(hdcDest, lpPoint, hdcSrc, xSrc, ySrc, width, height, hbmMask, xMask, yMask);
	}
	if (!gContext_)
	{
		return Hooked_PlgBlt_Next(hdcDest, lpPoint, hdcSrc, xSrc, ySrc, width, height, hbmMask, xMask, yMask);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyPlgBlt(hdcDest, lpPoint, hdcSrc, xSrc, ySrc, width, height, hbmMask, xMask, yMask);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop)
{
	if (hook_control.is_disabled())
	{
		return Hooked_StretchBlt_Next(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	}
	if (!gContext_)
	{
		return Hooked_StretchBlt_Next(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyStretchBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

BOOL WINAPI CRuntimeContext::Hooked_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_PrintWindow_Next(hwnd, hdcBlt, nFlags);
	}
	if (!gContext_)
	{
		return Hooked_PrintWindow_Next(hwnd, hdcBlt, nFlags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyPrintWindow(hwnd, hdcBlt, nFlags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

HDC WINAPI CRuntimeContext::Hooked_CreateDCA(LPCSTR pszDriver, LPCSTR pszDevice, LPCSTR pszPort, CONST DEVMODEA * pdm)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CreateDCA_Next(pszDriver, pszDevice, pszPort, pdm);
	}
	if (!gContext_)
	{
		return Hooked_CreateDCA_Next(pszDriver, pszDevice, pszPort, pdm);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCreateDCA(pszDriver, pszDevice, pszPort, pdm);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

HDC WINAPI CRuntimeContext::Hooked_CreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pwszPort, CONST DEVMODEW * pdm)
{
	if (hook_control.is_disabled())
	{
		return Hooked_CreateDCW_Next(pwszDriver, pwszDevice, pwszPort, pdm);
	}
	if (!gContext_)
	{
		return Hooked_CreateDCW_Next(pwszDriver, pwszDevice, pwszPort, pdm);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyCreateDCW(pwszDriver, pwszDevice, pwszPort, pdm);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

HDC WINAPI CRuntimeContext::Hooked_GetDC(HWND hWnd)
{
	if (hook_control.is_disabled())
	{
		return Hooked_GetDC_Next(hWnd);
	}
	if (!gContext_)
	{
		return Hooked_GetDC_Next(hWnd);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyGetDC(hWnd);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

HDC WINAPI CRuntimeContext::Hooked_GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
{
	if (hook_control.is_disabled())
	{
		return Hooked_GetDCEx_Next(hWnd, hrgnClip, flags);
	}
	if (!gContext_)
	{
		return Hooked_GetDCEx_Next(hWnd, hrgnClip, flags);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyGetDCEx(hWnd, hrgnClip, flags);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

HDC WINAPI CRuntimeContext::Hooked_GetWindowDC(HWND hWnd)
{
	if (hook_control.is_disabled())
	{
		return Hooked_GetWindowDC_Next(hWnd);
	}
	if (!gContext_)
	{
		return Hooked_GetWindowDC_Next(hWnd);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyGetWindowDC(hWnd);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

int WINAPI CRuntimeContext::Hooked_ReleaseDC(HWND hWnd, HDC hDC)
{
	if (hook_control.is_disabled())
	{
		return Hooked_ReleaseDC_Next(hWnd, hDC);
	}
	if (!gContext_)
	{
		return Hooked_ReleaseDC_Next(hWnd, hDC);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyReleaseDC(hWnd, hDC);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return 0;
}

BOOL WINAPI CRuntimeContext::Hooked_DeleteDC(HDC hdc)
{
	if (hook_control.is_disabled())
	{
		return Hooked_DeleteDC_Next(hdc);
	}
	if (!gContext_)
	{
		return Hooked_DeleteDC_Next(hdc);
	}
	// detour to runtime context
	__try
	{
		return gContext_->MyDeleteDC(hdc);
	}
	__except (NLEXCEPT_FILTER_EX2(NULL, exception_cb))
	{
		;/* empty */
	}

	return FALSE;
}

#pragma endregion

HRESULT CRuntimeContext::MyCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
//	nextlabs::recursion_control_auto auto_disable(hook_control);  // bug 35466, this bug only happens on Win10 64bit, (VM nli-win106402). so far we don't know the root cause, just disable this line code. Defact: below code can't call CoCreateInstance, otherwise dead-loop crash.
/*#ifdef _DEBUG 
	wprintf(L"CoCreateInstance\n");
#endif // DEBUG
	return Hooked_CoCreateInstance_Next(rclsid, pUnkOuter, dwClsContext, riid, ppv);*/

    HRESULT hr = Hooked_CoCreateInstance_Next(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    

    //
    // successfully return CLSID_FileOperation object with interface IID_IFileOperation
    //
    if (SUCCEEDED(hr) && 
        (*ppv) != NULL &&
        ::IsEqualCLSID(rclsid, CLSID_FileOperation) &&
        ::IsEqualIID(riid, IID_IFileOperation))
    {
        //::OutputDebugStringW(L"some one get the class-FileOperation and use interface-IFileOperation\n");

        IFileOperation*pObject = static_cast<IFileOperation*>(*ppv);
        LPVOID* pVTable = (*(LPVOID**)pObject);  // the v table of the object

        boost_unique_lock lockWriter(gMutex);  // lock for write access of mapCOMHooks_

        if (hooktable_[kHMCOMPerformOperations])
        {
            // try to hook PerformOperations()
            LPVOID pPerformOperations = pVTable[kCOMPerformOperationsIndex];
            if(mapCOMHooks_.find(pPerformOperations) == mapCOMHooks_.end())  // haven't hooked this function, then hook.
            {
				LPVOID* pnext_PerformOperations = new LPVOID();
                if(HookCode((LPVOID)pPerformOperations,(PVOID)CRuntimeContext::Hooked_COMPerformOperations,(LPVOID*)pnext_PerformOperations) && *pnext_PerformOperations)
                {
                    mapCOMHooks_[pPerformOperations] = *pnext_PerformOperations;
                }
            }
            else
            {
                //OutputDebugStringW(L"this function has been hooked already, don't need to hook again");
            }
        }

		if (hooktable_[kHMCOMNewItem])
		{
			LPVOID pNewItem = pVTable[kCOMNewItemIndex];
			if (mapCOMHooks_.find(pNewItem) == mapCOMHooks_.end())
			{
				LPVOID* pnext_NewItem = new LPVOID();
				if(HookCode((LPVOID)pNewItem, (PVOID)CRuntimeContext::Hooked_COMNewItem, (LPVOID*)pnext_NewItem) && *pnext_NewItem)
				{
					mapCOMHooks_[pNewItem] = *pnext_NewItem;
				}
			}
		}

		if (hooktable_[kHMCOMRenameItem])
		{
			LPVOID pRenameItem = pVTable[kCOMRenameItemIndex];
			if (mapCOMHooks_.find(pRenameItem) == mapCOMHooks_.end())
			{
				LPVOID* pnext_RenameItem = new LPVOID();
				if(HookCode((LPVOID)pRenameItem, (PVOID)CRuntimeContext::Hooked_COMRenameItem, (LPVOID*)pnext_RenameItem) && *pnext_RenameItem)
				{
					mapCOMHooks_[pRenameItem] = *pnext_RenameItem;
					//OutputDebugStringW(L"hook kHMCOMRenameItem success---------------------");
				}else{
					//OutputDebugStringW(L"hook kHMCOMRenameItem failed---------------------");
				}
			}
		}

		if (hooktable_[kHMCOMRenameItems])
		{
			LPVOID pRenameItems = pVTable[kCOMRenameItemsIndex];
			if (mapCOMHooks_.find(pRenameItems) == mapCOMHooks_.end())
			{
				LPVOID* pnext_RenameItems = new LPVOID();
				if(HookCode((LPVOID)pRenameItems, (PVOID)CRuntimeContext::Hooked_COMRenameItems, (LPVOID*)pnext_RenameItems) && *pnext_RenameItems)
				{
					mapCOMHooks_[pRenameItems] = *pnext_RenameItems;
				}
			}
		}
        
        if (hooktable_[kHMCOMCopyItems])
        {
            // try to hook CopyItems
            LPVOID pCopyItems = pVTable[kCOMCopyItemsIndex];
            if (mapCOMHooks_.find(pCopyItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_CopyItems = new LPVOID();
                if(HookCode((LPVOID)pCopyItems,(PVOID)CRuntimeContext::Hooked_COMCopyItems,(LPVOID*)pnext_CopyItems) && *pnext_CopyItems)
                {
                    mapCOMHooks_[pCopyItems] = *pnext_CopyItems;
                }
            }
        }

        if (hooktable_[kHMCOMMoveItems])
        {
            // try to hook MoveItems
            LPVOID pMoveItems = pVTable[kCOMMoveItemsIndex];
            if (mapCOMHooks_.find(pMoveItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_MoveItems = new LPVOID();
                if(HookCode((LPVOID)pMoveItems,(PVOID)CRuntimeContext::Hooked_COMMoveItems,(LPVOID*)pnext_MoveItems) && *pnext_MoveItems)
                {
                    mapCOMHooks_[pMoveItems] = *pnext_MoveItems;
                }
            }
        }
        
        if (hooktable_[kHMCOMDeleteItems])
        {
            // try to hook DeleteItems
            LPVOID pDeleteItems = pVTable[kCOMDeleteItemsIndex];
            if (mapCOMHooks_.find(pDeleteItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_DeleteItems = new LPVOID();
                if (HookCode((LPVOID)pDeleteItems, (PVOID)CRuntimeContext::Hooked_COMDeleteItems, (LPVOID*)pnext_DeleteItems) && *pnext_DeleteItems)
                {
                    mapCOMHooks_[pDeleteItems] = *pnext_DeleteItems;
                }
                
            }
        }

        if (hooktable_[kHMCOMDeleteItem])
        {
            LPVOID pDeleteItem = pVTable[kCOMDeleteItemIndex];
            if (mapCOMHooks_.find(pDeleteItem) == mapCOMHooks_.end())
            {
				LPVOID* pnext_DeleteItem = new LPVOID();
                if (HookCode((LPVOID)pDeleteItem, (PVOID)CRuntimeContext::Hooked_COMDeleteItem, (LPVOID*)pnext_DeleteItem) && *pnext_DeleteItem)
                {
                    mapCOMHooks_[pDeleteItem] = *pnext_DeleteItem;
                }

            }
        }
        
    }

	if (SUCCEEDED(hr) &&
		(*ppv) != NULL &&
		::IsEqualCLSID(rclsid, CLSID_LocalThumbnailCache) &&
		::IsEqualIID(riid, IID_IThumbnailCache))
	{
		IThumbnailCache *pObject = static_cast<IThumbnailCache *>(*ppv);
		LPVOID *pVTable = (*(LPVOID **)pObject); // the v table of the object

		boost_unique_lock lockWriter(gMutex); // lock for write access of mapCOMHooks_

		if (hooktable_[kHMCOMThumbnailCache])
		{
			LPVOID pGetThumbnail = pVTable[kCOMGetThumbnail];

			MEMORY_BASIC_INFORMATION memoryInfo = {0};
			VirtualQuery(pGetThumbnail, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION));

			wchar_t currentModule[MAX_PATH] = {0};
			GetModuleFileNameW((HMODULE)memoryInfo.AllocationBase, currentModule, MAX_PATH);

			if (boost::algorithm::iends_with(currentModule, L"\\thumbcache.dll"))
			{
				if (mapCOMHooks_.find(pGetThumbnail) == mapCOMHooks_.end())
				{
					LPVOID *pnext_GetThumbnail = new LPVOID();
					if (HookCode((LPVOID)pGetThumbnail, (PVOID)CRuntimeContext::Hooked_COMGetThumbnail, (LPVOID *)pnext_GetThumbnail) && *pnext_GetThumbnail)
					{
						mapCOMHooks_[pGetThumbnail] = *pnext_GetThumbnail;
					}
				}
			}
		}
	}

	if (SUCCEEDED(hr) && 
		(*ppv) != NULL &&
		::IsEqualCLSID(rclsid, CLSID_FileSaveDialog))
	{
		IFileSaveDialog*pObject = static_cast<IFileSaveDialog*>(*ppv);
		LPVOID* pVTable = (*(LPVOID**)pObject);  // the v table of the object

		boost_unique_lock lockWriter(gMutex);  // lock for write access of mapCOMHooks_

		if (hooktable_[kHMCOMShow]){
			LPVOID pShow = pVTable[kCOMShow];
			if (mapCOMHooks_.find(pShow) == mapCOMHooks_.end())
			{
				LPVOID* pnext_Show = new LPVOID();
				if (HookCode((LPVOID)pShow, (PVOID)CRuntimeContext::Hooked_COMShow, (LPVOID*)pnext_Show) && *pnext_Show)
				{
					mapCOMHooks_[pShow] = *pnext_Show;
				}
			}
		}
	}

    return hr;
}


void CRuntimeContext::HookIFileOperation()
{
	IUnknown* pIFileOperation = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_INPROC, IID_IFileOperation, (LPVOID*)&pIFileOperation);
	if((S_OK == hr) && (NULL != pIFileOperation))
    {
        utils::DebugViewLog("[BasePep] %s Get FileOperation instance success\n", __FUNCTION__);

        LONG lerr = 0;
        LPVOID* pVTable = (*(LPVOID**)pIFileOperation);  // the v table of the object
        boost_unique_lock lockWriter(gMutex);  // lock for write access of mapCOMHooks_

        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

		 if (hooktable_[kHMCOMPerformOperations])
        {
            // try to hook PerformOperations()
            LPVOID pPerformOperations = pVTable[kCOMPerformOperationsIndex];
            if(mapCOMHooks_.find(pPerformOperations) == mapCOMHooks_.end())  // haven't hooked this function, then hook.
            {
				LPVOID* pnext_PerformOperations = new LPVOID();
				*pnext_PerformOperations = pPerformOperations;
                lerr = DetourAttach(pnext_PerformOperations, (PVOID)CRuntimeContext::Hooked_COMPerformOperations);
				if(ERROR_SUCCESS == lerr)
                {
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::PerformOperations success\n", __FUNCTION__);
                    mapCOMHooks_[pPerformOperations] = pnext_PerformOperations;
                }
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::PerformOperations failed, err: %d\n", __FUNCTION__, lerr);
				}
            }
        }

		if (hooktable_[kHMCOMNewItem])
		{
			LPVOID pNewItem = pVTable[kCOMNewItemIndex];
			if (mapCOMHooks_.find(pNewItem) == mapCOMHooks_.end())
			{
				LPVOID* pnext_NewItem = new LPVOID();
				*pnext_NewItem = pNewItem;
                lerr = DetourAttach(pnext_NewItem, (PVOID)CRuntimeContext::Hooked_COMNewItem);
				if(ERROR_SUCCESS == lerr)
				{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::NewItem success\n", __FUNCTION__);
					mapCOMHooks_[pNewItem] = pnext_NewItem;
				}
				else{
					utils::DebugViewLog("[BasePep] %s Hook IFileOperation::NewItem failed, err: %d\n", __FUNCTION__, lerr);
				}
			}
		}

		if (hooktable_[kHMCOMRenameItem])
		{
			LPVOID pRenameItem = pVTable[kCOMRenameItemIndex];
			if (mapCOMHooks_.find(pRenameItem) == mapCOMHooks_.end())
			{
				LPVOID* pnext_RenameItem = new LPVOID();
				*pnext_RenameItem = pRenameItem;
                lerr = DetourAttach(pnext_RenameItem, (PVOID)CRuntimeContext::Hooked_COMRenameItem);
				if(ERROR_SUCCESS == lerr)
				{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::RenameItem success\n", __FUNCTION__);
					mapCOMHooks_[pRenameItem] = pnext_RenameItem;
				}else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::RenameItem failed, err: %d\n", __FUNCTION__, lerr);
				}
			}
		}

		if (hooktable_[kHMCOMRenameItems])
		{
			LPVOID pRenameItems = pVTable[kCOMRenameItemsIndex];
			if (mapCOMHooks_.find(pRenameItems) == mapCOMHooks_.end())
			{
				LPVOID* pnext_RenameItems = new LPVOID();
				*pnext_RenameItems = pRenameItems;
                lerr = DetourAttach(pnext_RenameItems, (PVOID)CRuntimeContext::Hooked_COMRenameItems);
				if(ERROR_SUCCESS == lerr)
				{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::RenameItems success\n", __FUNCTION__);
					mapCOMHooks_[pRenameItems] = pnext_RenameItems;
				}
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::RenameItems failed, err: %d\n", __FUNCTION__, lerr);
				}
			}
		}
        
        if (hooktable_[kHMCOMCopyItems])
        {
            // try to hook CopyItems
            LPVOID pCopyItems = pVTable[kCOMCopyItemsIndex];
            if (mapCOMHooks_.find(pCopyItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_CopyItems = new LPVOID();
				*pnext_CopyItems = pCopyItems;
                lerr = DetourAttach(pnext_CopyItems, (PVOID)CRuntimeContext::Hooked_COMCopyItems);
				if(ERROR_SUCCESS == lerr)                
                {
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::CopyItems success\n", __FUNCTION__);
                    mapCOMHooks_[pCopyItems] = pnext_CopyItems;
                }
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::CopyItems failed, err: %d\n", __FUNCTION__, lerr);
				}
            }
        }

        if (hooktable_[kHMCOMMoveItems])
        {
            // try to hook MoveItems
            LPVOID pMoveItems = pVTable[kCOMMoveItemsIndex];
            if (mapCOMHooks_.find(pMoveItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_MoveItems = new LPVOID();
				*pnext_MoveItems = pMoveItems;
                lerr = DetourAttach(pnext_MoveItems, (PVOID)CRuntimeContext::Hooked_COMMoveItems);
				if(ERROR_SUCCESS == lerr)
                {
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::MoveItems success\n", __FUNCTION__);
                    mapCOMHooks_[pMoveItems] = pnext_MoveItems;
                }
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::MoveItems failed, err: %d\n", __FUNCTION__, lerr);
				}
            }
        }
        
        if (hooktable_[kHMCOMDeleteItems])
        {
            // try to hook DeleteItems
            LPVOID pDeleteItems = pVTable[kCOMDeleteItemsIndex];
            if (mapCOMHooks_.find(pDeleteItems) == mapCOMHooks_.end())
            {
				LPVOID* pnext_DeleteItems = new LPVOID();
				*pnext_DeleteItems = pDeleteItems;
                lerr = DetourAttach(pnext_DeleteItems, (PVOID)CRuntimeContext::Hooked_COMDeleteItems);
				if(ERROR_SUCCESS == lerr)
                {
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::DeleteItems success\n", __FUNCTION__);
                    mapCOMHooks_[pDeleteItems] = pnext_DeleteItems; //we can't save *pnext_DeleteItems, because it only changed after DetourTransactionCommit
                }
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::DeleteItems failed, err: %d\n", __FUNCTION__, lerr);
				}
                
            }
        }

        if (hooktable_[kHMCOMDeleteItem])
        {
            LPVOID pDeleteItem = pVTable[kCOMDeleteItemIndex];
            if (mapCOMHooks_.find(pDeleteItem) == mapCOMHooks_.end())
            {
				LPVOID* pnext_DeleteItem = new LPVOID();
				*pnext_DeleteItem = pDeleteItem;
                lerr = DetourAttach(pnext_DeleteItem, (PVOID)CRuntimeContext::Hooked_COMDeleteItem);
                if(ERROR_SUCCESS == lerr)
                {
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::DeleteItem success\n", __FUNCTION__);
                    mapCOMHooks_[pDeleteItem] = pnext_DeleteItem;
                }
				else{
                    utils::DebugViewLog("[BasePep] %s Hook IFileOperation::DeleteItem failed, err: %d\n", __FUNCTION__, lerr);
				}

            }
        }

        lerr = DetourTransactionCommit();

		//replace function cache
		if(ERROR_SUCCESS == lerr){

		   if (hooktable_[kHMCOMPerformOperations]) {
				LPVOID pOriginal = pVTable[kCOMPerformOperationsIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}

			if (hooktable_[kHMCOMNewItem]) {
				LPVOID pOriginal = pVTable[kCOMNewItemIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}

			if (hooktable_[kHMCOMRenameItem]) {
				LPVOID pOriginal = pVTable[kCOMRenameItemIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}


			if (hooktable_[kHMCOMRenameItems]) {
				LPVOID pOriginal = pVTable[kCOMRenameItemsIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}

			if (hooktable_[kHMCOMCopyItems]) {
				LPVOID pOriginal = pVTable[kCOMCopyItemsIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}

			
			if (hooktable_[kHMCOMMoveItems]) {
				LPVOID pOriginal = pVTable[kCOMMoveItemsIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}

			if (hooktable_[kHMCOMDeleteItems]) {
				LPVOID pOriginal = pVTable[kCOMDeleteItemsIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}


			if (hooktable_[kHMCOMDeleteItem]) {
				LPVOID pOriginal = pVTable[kCOMDeleteItemIndex];
				if (mapCOMHooks_.find(pOriginal) != mapCOMHooks_.end()) {
						mapCOMHooks_[pOriginal] = *((LPVOID*)mapCOMHooks_[pOriginal]);
				}
			}
		
		}
		else{
            utils::DebugViewLog("[BasePep] %s HookCommit failed, err: %d\n", __FUNCTION__, lerr);
		}

		pIFileOperation->Release();
		pIFileOperation = NULL;
	}
    else
    {
        utils::DebugViewLog("[BasePep] %s Create IFileOperation instance failed, err: 0x%08x\n", __FUNCTION__, hr);
    }
}

void CRuntimeContext::HookIThumbnailCache()
{
    IUnknown* pIThumbnailCache = NULL;
    HRESULT hr = CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC, IID_IThumbnailCache, (LPVOID*)&pIThumbnailCache);
    if (S_OK == hr && pIThumbnailCache != NULL)
    {
        LPVOID* pVTable = *(LPVOID**)pIThumbnailCache;
        boost_unique_lock lockWriter(gMutex); // lock for write access of mapCOMHooks_

        if (hooktable_[kHMCOMThumbnailCache])
        {
            LPVOID pGetThumbnail = pVTable[kCOMGetThumbnail];

            MEMORY_BASIC_INFORMATION memoryInfo = { 0 };
            VirtualQuery(pGetThumbnail, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION));

            wchar_t currentModule[MAX_PATH] = { 0 };
            GetModuleFileNameW((HMODULE)memoryInfo.AllocationBase, currentModule, MAX_PATH);

            if (boost::algorithm::iends_with(currentModule, L"\\thumbcache.dll"))
            {
                if (mapCOMHooks_.find(pGetThumbnail) == mapCOMHooks_.end())
                {
                    DetourRestoreAfterWith();
                    DetourTransactionBegin();
                    DetourUpdateThread(GetCurrentThread());

                    LPVOID *pnext_GetThumbnail = new LPVOID();
                    *pnext_GetThumbnail = pVTable[kCOMGetThumbnail];
                    LONG lerr = DetourAttach(pnext_GetThumbnail, (PVOID)CRuntimeContext::Hooked_COMGetThumbnail);
                    if (ERROR_SUCCESS == lerr)
                    {
                        utils::DebugViewLog("[BasePep] %s hook IThumbnailCache::GetThumbnail success\n", __FUNCTION__);
                        mapCOMHooks_[pGetThumbnail] = pnext_GetThumbnail;
                    }
                    else
                    {
                        utils::DebugViewLog("[BasePep] %s hook IThumbnailCache::GetThumbnail failed, err: %d\n", __FUNCTION__, lerr);
                    }

                    lerr = DetourTransactionCommit();
                    if (ERROR_SUCCESS == lerr)
                    {
                        if (mapCOMHooks_.find(pGetThumbnail) != mapCOMHooks_.end())
                        {
                            mapCOMHooks_[pGetThumbnail] = *((LPVOID*)mapCOMHooks_[pGetThumbnail]);
                        }
                    }
                    else
                    {
                        utils::DebugViewLog("[BasePep] %s HookCommit IThumbnailCache::GetThumbnail failed, err: %d\n", __FUNCTION__, lerr);
                    }
                }
            }
        }

        pIThumbnailCache->Release();
        pIThumbnailCache = NULL;
    }
    else
    {
        utils::DebugViewLog("[BasePep] %s Create IThumbnailCache failed, err: 0x%08x\n", __FUNCTION__, hr);
    }
}

void CRuntimeContext::HookFileSaveDialog()
{
    IUnknown* IFileSaveDialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC, IID_IFileSaveDialog, (LPVOID*)&IFileSaveDialog);
    if (S_OK == hr && IFileSaveDialog != NULL)
    {
        LPVOID* pVTable = *(LPVOID**)IFileSaveDialog;
        boost_unique_lock lockWriter(gMutex);  // lock for write access of mapCOMHooks_

        if (hooktable_[kHMCOMShow]) {
            LPVOID pShow = pVTable[kCOMShow];
            if (mapCOMHooks_.find(pShow) == mapCOMHooks_.end())
            {
                DetourRestoreAfterWith();
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                LPVOID* pnext_Show = new LPVOID();
                *pnext_Show = pVTable[kCOMShow];
                LONG lerr = DetourAttach(pnext_Show, (PVOID)CRuntimeContext::Hooked_COMShow);
                if (lerr == ERROR_SUCCESS)
                {
                    utils::DebugViewLog("[BasePep] %s hook IFileSaveDialog::Show success\n", __FUNCTION__);
                    mapCOMHooks_[pShow] = pnext_Show;
                }
                else
                {
                    utils::DebugViewLog("[BasePep] %s hook IFileSaveDialog::Show failed, err: %d\n", __FUNCTION__, lerr);
                }

                lerr = DetourTransactionCommit();
                if (lerr == ERROR_SUCCESS)
                {
                    if (mapCOMHooks_.find(pShow) != mapCOMHooks_.end())
                    {
                        mapCOMHooks_[pShow] = *((LPVOID*)mapCOMHooks_[pShow]);
                    }
                }
                else
                {
                    utils::DebugViewLog("[BasePep] %s HookCommit IFileSaveDialog::Show failed, err: %d\n", __FUNCTION__, lerr);
                }
            }
        }
    }
    else
    {
        utils::DebugViewLog("[BasePep] %s Create IFileSaveDialog failed, err: 0x%08x\n", __FUNCTION__, hr);
    }
}


BOOL CRuntimeContext::Init()
{
	//	Init class params
	//	--Hooktable set 0 ,all false by default
	for (int i = 0; i < kHM_MaxItems; i++){
		hooktable_[i] = false;
	}
	//  --COM Interface Hook
	mapCOMHooks_.clear();

//	OnInitCESDK();

	// Init Hook
	OnHookInit();

	//call CoCreateInstance with CLSID_FileOperation to make the interface of IFileOperation been hooked.
	CoInitialize(NULL);
	{
		// prepare Detours
		HookIFileOperation();
        HookIThumbnailCache();
        HookFileSaveDialog();
	}
	CoUninitialize();
	
	return TRUE;
}

void CRuntimeContext::Deinit()
{
	// Deinit CESDK?
	// write cesdk code here

	// Deinit Hook
	OnHookDeinit();
    
    //this function must after unhook, if not, it will cause dead lock.
    PurgeDropTargets();

	// make null to close detour routine
	gContext_ = NULL;

}

BOOL CRuntimeContext::OnHookInit()
{
	// derived class can override this method to specify it's own hook intent
	OnResponseInitHookTable(hooktable_);

	InitializeMadCHook();

	hook_control.process_disable(); // disable hooking during hook setup
	{
		// MadCHook provided CollectHooks/FlushHooks ,to speed up batch-hooks 
		CollectHooks();
		{
			OnHookInstall();
			// give derived object a chance to setup it's own hook procedure
			OnResponseHookInstallOthers();

		}
		FlushHooks();
	}
	hook_control.process_enable();// restore hook_control just after OnHookInit()

	return TRUE;

}

void CRuntimeContext::OnHookDeinit()
{
	hook_control.process_disable(); // make whole hook framework dormant
	{
		OnHookUninstall();
		// give derived object a chance to setup it's own hook procedure
		OnResponseHookUninstallOthers();
	}
	hook_control.process_enable();
	FinalizeMadCHook();
}

void CRuntimeContext::OnResponseInitHookTable(bool(&table)[kHM_MaxItems])
{
	// all false by default
	for (int i = 0; i < kHM_MaxItems; i++){
		table[i] = false;
	}
}

BOOL CRuntimeContext::OnInitCESDK()
{
	nextlabs::comm_helper::Init_Cesdk(&nextlabs::policyengine::gCesdkContext);
	return TRUE;
}

BOOL CRuntimeContext::OnHookInstall()
{
	// impl IFileOperation
	if (hooktable_[kHMCreateFileW])
		HookAPI("kernelbase", "CreateFileW", (void*)Hooked_CreateFileW, (void**)&Hooked_CreateFileW_Next);
    if(hooktable_[kHMCreateFileMappintW])
        HookAPI("kernelbase", "CreateFileMappingW", (void*)Hooked_CreateFileMappingW, (void**)&Hooked_CreateFileMappingW_Next);
	if (hooktable_[kHMCloseHandle])
		HookAPI("kernel32", "CloseHandle", (void*)Hooked_CloseHandle, (void**)&Hooked_CloseHandle_Next);
    if (hooktable_[kHMFindFirstFileExW])
        HookAPI("kernelbase", "FindFirstFileExW", (void *)Hooked_FindFirstFileExW, (void **)&Hooked_FindFirstFileExW_Next);
	if (hooktable_[kHMCreateDirectoryW])
		HookAPI("kernel32", "CreateDirectoryW", (void*)Hooked_CreateDirectoryW, (void**)&Hooked_CreateDirectoryW_Next);
	if (hooktable_[kHMRemoveDirectoryW])
		HookAPI("kernel32", "RemoveDirectoryW", (void*)Hooked_RemoveDirectoryW, (void**)&Hooked_RemoveDirectoryW_Next);
	if (hooktable_[kHMDeleteFileW])
		HookAPI("kernel32", "DeleteFileW", (void*)Hooked_DeleteFileW, (void**)&Hooked_DeleteFileW_Next);
	if (hooktable_[kHMReadFile])
		HookAPI("kernel32", "ReadFile", (void*)Hooked_ReadFile, (void**)&Hooked_ReadFile_Next);
	if (hooktable_[kHMReadFileEx])
		HookAPI("kernel32", "ReadFileEx", (void*)Hooked_ReadFileEx, (void**)&Hooked_ReadFileEx_Next);
	if (hooktable_[kHMWriteFile])
		HookAPI("kernel32", "WriteFile", (void*)Hooked_WriteFile, (void**)&Hooked_WriteFile_Next);
	if (hooktable_[kHMWriteFileEx])
		HookAPI("kernel32", "WriteFileEx", (void*)Hooked_WriteFileEx, (void**)&Hooked_WriteFileEx_Next);
	if (hooktable_[kHMSetEndOfFile])
		HookAPI("kernel32", "SetEndOfFile", (void*)Hooked_SetEndOfFile, (void**)&Hooked_SetEndOfFile_Next);
	if (hooktable_[kHMKernelBaseSetEndOfFile])
		HookAPI("KernelBase", "SetEndOfFile", (void*)Hooked_KernelBaseSetEndOfFile, (void**)&Hooked_KernelBaseSetEndOfFile_Next);
	if (hooktable_[kHMCopyFileW])
		HookAPI("kernel32", "CopyFileW", (void*)Hooked_CopyFileW, (void**)&Hooked_CopyFileW_Next);
	if (hooktable_[kHMCopyFileExW])
		HookAPI("kernel32", "CopyFileExW", (void*)Hooked_CopyFileExW, (void**)&Hooked_CopyFileExW_Next);
    if (hooktable_[kHMPrivCopyFileExW])   
        HookAPI("kernel32", "PrivCopyFileExW", (void*)Hooked_PrivCopyFileExW, (void**)&Hooked_PrivCopyFileExW_Next); 
    if (hooktable_[kHMKernelBasePrivCopyFileExW])   
        HookAPI("KernelBase", "PrivCopyFileExW", (void*)Hooked_KernelBasePrivCopyFileExW, (void**)&Hooked_KernelBasePrivCopyFileExW_Next); 
	if (hooktable_[kHMMoveFileW])
		HookAPI("kernel32", "MoveFileW", (void*)Hooked_MoveFileW, (void**)&Hooked_MoveFileW_Next);
	if (hooktable_[kHMMoveFileExW])
        HookAPI("kernel32", "MoveFileExW", (void*)Hooked_MoveFileExW, (void**)&Hooked_MoveFileExW_Next);
	if (hooktable_[kHMMoveFileWithProgressW])
		HookAPI("kernel32", "MoveFileWithProgressW", (void*)Hooked_MoveFileWithProgressW, (void**)&Hooked_MoveFileWithProgressW_Next);
	if (hooktable_[kHMKernelBaseMoveFileExW])
		HookAPI("KernelBase", "MoveFileExW", (void*)Hooked_KernelBaseMoveFileExW, (void**)&Hooked_KernelBaseMoveFileExW_Next);
	if (hooktable_[kHMKernelBaseMoveFileWithProgressW])
		HookAPI("KernelBase", "MoveFileWithProgressW", (void*)Hooked_KernelBaseMoveFileWithProgressW, (void**)&Hooked_KernelBaseMoveFileWithProgressW_Next);
	if (hooktable_[kHMReplaceFileW])
		HookAPI("kernel32", "ReplaceFileW", (void*)Hooked_ReplaceFileW, (void**)&Hooked_ReplaceFileW_Next);
	if (hooktable_[kHMCreateHardLinkW])
		HookAPI("kernel32", "CreateHardLinkW", (void*)Hooked_CreateHardLinkW, (void**)&Hooked_CreateHardLinkW_Next);
    if (hooktable_[kHMSetFileAttributesW])
        HookAPI("kernelbase", "SetFileAttributesW", (void*)Hooked_SetFileAttributesW, (void**)&Hooked_SetFileAttributesW_Next);
    if (hooktable_[kHMGetFileAttributesW])
        HookAPI("kernelbase", "GetFileAttributesW", (void *)Hooked_GetFileAttributesW, (void**)&Hooked_GetFileAttributesW_Next);
    if (hooktable_[kHMSetNamedSecurityInfoW])
        HookAPI("advapi32", "SetNamedSecurityInfoW", (void *)Hooked_SetNamedSecurityInfoW, (void**)&Hooked_SetNamedSecurityInfoW_Next);
    if (hooktable_[kHMAddUsersToEncryptedFile])
        HookAPI("advapi32", "AddUsersToEncryptedFile", (void *)Hooked_AddUsersToEncryptedFile, (void**)&Hooked_AddUsersToEncryptedFile_Next);
    if (hooktable_[kHMGetOpenFileName])
        HookAPI("Comdlg32", "GetOpenFileNameW", (void*)Hooked_GetOpenFileNameW, (void**)&Hooked_GetOpenFileNameW_Next);

    if (hooktable_[kHMSHSimulateDrop])
    {
        HMODULE hModuel = GetModuleHandle(L"shlwapi.dll");
        if (hModuel != NULL)
        {
            FARPROC funcAddress = GetProcAddress(hModuel, (char*)186);
            HookCode((LPVOID)funcAddress,(PVOID)Hooked_SHSimulateDrop,(LPVOID*)&Hooked_SHSimulateDrop_Next);
        }
    }
   
    if (hooktable_[kHMSetFileInformationByHandle])
        HookAPI("kernelbase", "SetFileInformationByHandle", (void*)Hooked_SetFileInformationByHandle, (void**)&Hooked_SetFileInformationByHandle_Next);

    if (hooktable_[kHMEncryptFileW])
        HookAPI("Advapi32", "EncryptFileW", (void*)Hooked_EncryptFileW, (void**)&Hooked_EncryptFileW_Next);
    if (hooktable_[kHMDecryptFileW])
        HookAPI("Advapi32", "DecryptFileW", (void*)Hooked_DecryptFileW, (void**)&Hooked_DecryptFileW_Next);
	if (hooktable_[kHMSHFileOperationW])
		HookAPI("shell32", "SHFileOperationW", (void*)Hooked_SHFileOperationW, (void**)&Hooked_SHFileOperationW_Next);
   
    if (hooktable_[kHMSetClipboardData])
		HookAPI("user32", "SetClipboardData", (void*)Hooked_SetClipboardData, (void**)&Hooked_SetClipboardData_Next);		
	if (hooktable_[kHMGetClipboardData])
		HookAPI("user32", "GetClipboardData", (void*)Hooked_GetClipboardData, (void**)&Hooked_GetClipboardData_Next);
	if (hooktable_[kHMOleSetClipboard])
		HookAPI("ole32", "OleSetClipboard", (void*)Hooked_OleSetClipboard, (void**)&Hooked_OleSetClipboard_Next);
	if (hooktable_[kHMOleGetClipboard])
		HookAPI("ole32", "OleGetClipboard", (void *)Hooked_OleGetClipboard, (void**)&Hooked_OleGetClipboard_Next);
    
    if (hooktable_[kHMDoDragDrop])
		HookAPI("ole32", "DoDragDrop", (void*)Hooked_DoDragDrop, (void**)&Hooked_DoDragDrop_Next);
    if (hooktable_[kHMRegisterDragDrop])
		HookAPI("ole32", "RegisterDragDrop", (void*)Hooked_RegisterDragDrop, (void**)&Hooked_RegisterDragDrop_Next);
	if (hooktable_[kHMRevokeDragDrop])
		HookAPI("ole32", "RevokeDragDrop", (void*)Hooked_RevokeDragDrop, (void**)&Hooked_RevokeDragDrop_Next);
	
    // internal functions
    if (hooktable_[kHMNtCreateFile])
        HookAPI("Ntdll", "NtCreateFile", (void*)Hooked_NtCreateFile, (void**)&Hooked_NtCreateFile_Next);
    if (hooktable_[kHMNtOpenFile])
        HookAPI("Ntdll", "NtOpenFile", (void*)Hooked_NtOpenFile, (void**)&Hooked_NtOpenFile_Next);
    if (hooktable_[kHMNtClose])
        HookAPI("Ntdll", "NtClose", (void*)Hooked_NtClose, (void**)&Hooked_NtClose_Next);
    
    
    if (hooktable_[kHMRtlDosPathNameToNtPathName_U_WithStatus])
        HookAPI("Ntdll", "RtlDosPathNameToNtPathName_U_WithStatus", (void*)Hooked_RtlDosPathNameToNtPathName_U_WithStatus, (void**)&Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next);

    if (hooktable_[kHMNtSetSecurityObject])
        HookAPI("Ntdll", "NtSetSecurityObject", (void*)Hooked_NtSetSecurityObject, (void**)&Hooked_NtSetSecurityObject_Next);
    
	// impl IModuleProcessOperation
	if (hooktable_[kHMCreateProcessW])
		HookAPI("kernel32", "CreateProcessW", (void*)Hooked_CreateProcessW, (void**)&Hooked_CreateProcessW_Next);
	if (hooktable_[kHMExitProcess])
		HookAPI("kernel32", "ExitProcess", (void*)Hooked_ExitProcess, (void**)&Hooked_ExitProcess_Next);
	if (hooktable_[kHMTerminateProcess])
		HookAPI("kernel32", "TerminateProcess", (void*)Hooked_TerminateProcess, (void**)&Hooked_TerminateProcess_Next);
	if (hooktable_[kHMLoadLibraryW])
		HookAPI("kernel32", "LoadLibraryW", (void*)Hooked_LoadLibraryW, (void**)&Hooked_LoadLibraryW_Next);
	if (hooktable_[kHMLoadLibraryExW])
		HookAPI("kernel32", "LoadLibraryExW", (void*)Hooked_LoadLibraryExW, (void**)&Hooked_LoadLibraryExW_Next);
	if (hooktable_[kHMDeviceIoControl])
		HookAPI("kernel32", "DeviceIoControl", (void*)Hooked_DeviceIoControl, (void**)&Hooked_DeviceIoControl_Next);
    //didn't hook CoCreateInstance, we all CoCreateInstance manually, get object, and hook it.
	//if (hooktable_[kHMCoCreateInstance])
	//	HookAPI("ole32", "CoCreateInstance", (void*)Hooked_CoCreateInstance, (void**)&Hooked_CoCreateInstance_Next);

	// impl IHookNetworkOperation
	if (hooktable_[kHMInternetConnectA])
		HookAPI("Wininet", "InternetConnectA", (void*)Hooked_InternetConnectA, (void**)&Hooked_InternetConnectA_Next);
	if (hooktable_[kHMInternetConnectW])
		HookAPI("Wininet", "InternetConnectW", (void*)Hooked_InternetConnectW, (void**)&Hooked_InternetConnectW_Next);
	if (hooktable_[kHMInternetCloseHandle])
		HookAPI("Wininet","InternetCloseHandle",(void*)Hooked_InternetCloseHandle,(void**)&Hooked_InternetCloseHandle_Next);
	if (hooktable_[kHMHttpOpenRequestA])
		HookAPI("Wininet","HttpOpenRequestA",(void*)Hooked_HttpOpenRequestA,(void**)&Hooked_HttpOpenRequestA_Next);
	if (hooktable_[kHMHttpOpenRequestW])
		HookAPI("Wininet", "HttpOpenRequestW", (void*)Hooked_HttpOpenRequestW, (void**)&Hooked_HttpOpenRequestW_Next);
	
	if (hooktable_[kHMGetSaveFileNameW])
		HookAPI("Comdlg32", "GetSaveFileNameW", (void*)Hooked_GetSaveFileNameW, (void**)&Hooked_GetSaveFileNameW_Next);

	if (hooktable_[kHMBitBlt])
		HookAPI("Gdi32", "BitBlt", (void*)Hooked_BitBlt, (void**)&Hooked_BitBlt_Next);
	if (hooktable_[kHMMaskBlt])
		HookAPI("Gdi32", "MaskBlt", (void*)Hooked_MaskBlt, (void**)&Hooked_MaskBlt_Next);
	if (hooktable_[kHMPlgBlt])
		HookAPI("Gdi32", "PlgBlt", (void*)Hooked_PlgBlt, (void**)&Hooked_PlgBlt_Next);
	if (hooktable_[kHMStretchBlt])
		HookAPI("Gdi32", "StretchBlt", (void*)Hooked_StretchBlt, (void**)&Hooked_StretchBlt_Next);
	if (hooktable_[kHMPrintWindow])
		HookAPI("user32", "PrintWindow", (void*)Hooked_PrintWindow, (void**)&Hooked_PrintWindow_Next);
	if (hooktable_[kHMCreateDCA])
		HookAPI("Gdi32", "CreateDCA", (void*)Hooked_CreateDCA, (void**)&Hooked_CreateDCA_Next);
	if (hooktable_[kHMCreateDCW])
		HookAPI("Gdi32", "CreateDCW", (void*)Hooked_CreateDCW, (void**)&Hooked_CreateDCW_Next);
	if (hooktable_[kHMDeleteDC])
		HookAPI("Gdi32", "DeleteDC", (void*)Hooked_DeleteDC, (void**)&Hooked_DeleteDC_Next);
	if (hooktable_[kHMGetDC])
		HookAPI("user32", "GetDC", (void*)Hooked_GetDC, (void**)&Hooked_GetDC_Next);
	if (hooktable_[kHMGetDCEx])
		HookAPI("user32", "GetDCEx", (void*)Hooked_GetDCEx, (void**)&Hooked_GetDCEx_Next);
	if (hooktable_[kHMGetWindowDC])
		HookAPI("user32", "GetWindowDC", (void*)Hooked_GetWindowDC, (void**)&Hooked_GetWindowDC_Next);
	if (hooktable_[kHMReleaseDC])
		HookAPI("user32", "ReleaseDC", (void*)Hooked_ReleaseDC, (void**)&Hooked_ReleaseDC_Next);

	return TRUE;
}

void CRuntimeContext::OnHookUninstall()
{
	// for robust and safety sake , do not unhook
}

}  // ns nextlabs
