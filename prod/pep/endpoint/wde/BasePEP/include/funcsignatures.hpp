/***********************************************************************
 Contains all functions' signature of supported windows official APIS
***********************************************************************/

#ifndef __FUNC_SIGNATURES_H__
#define __FUNC_SIGNATURES_H__

#ifdef _MSC_VER
#pragma once
#else
#error "funcsignatures.hpp only supports windows-compile"
#endif // _MSC_VER

#include <windows.h>
#include <shellapi.h>
#include <Winternl.h>
#include <Wininet.h>  // for wininet apis
#include <AccCtrl.h>
#include <winefs.h>
#include <thumbcache.h>
#include <Commdlg.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma region Common

typedef BOOL (WINAPI *PF_CloseHandle)( _In_ HANDLE hObject );

#pragma endregion


#pragma region FileOperations

typedef HANDLE	(WINAPI	*PF_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

typedef HANDLE	(WINAPI	*PF_CreateFileW)( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

typedef HANDLE(WINAPI *PF_CreateFileMappingW)(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHeight, DWORD dwMaximumSizeLow, LPCTSTR lpName);

typedef HANDLE (WINAPI *PF_FindFirstFileExW)(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);

typedef BOOL (WINAPI *PF_CreateDirectoryW)( LPCWSTR lpPathName,	 LPSECURITY_ATTRIBUTES lpSecurityAttributes	);

typedef	BOOL (WINAPI *PF_RemoveDirectoryA)(	 LPCSTR lpPathName);

typedef	BOOL (WINAPI *PF_RemoveDirectoryW)(	 LPCWSTR lpPathName);

typedef BOOL (WINAPI *PF_DeleteFileA)( LPCSTR lpFileName	);

typedef BOOL (WINAPI *PF_DeleteFileW)( LPCWSTR lpFileName);


typedef BOOL (WINAPI *PF_ReadFile)( HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

typedef BOOL (WINAPI *PF_ReadFileEx)( HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPOVERLAPPED lpOverlapped,LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);



typedef BOOL (WINAPI *PF_WriteFile)(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

typedef BOOL (WINAPI *PF_WriteFileEx)( HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPOVERLAPPED lpOverlapped,LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

typedef BOOL (WINAPI *PF_SetEndOfFile)(HANDLE hFile);

typedef BOOL (WINAPI *PF_CopyFileA)(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,BOOL bFailIfExists);
typedef BOOL (WINAPI *PF_CopyFileW)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,BOOL bFailIfExists);

typedef BOOL (WINAPI *PF_CopyFileExA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel, DWORD dwCopyFlags);
typedef BOOL (WINAPI *PF_CopyFileExW)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData, LPBOOL pbCancel,	DWORD dwCopyFlags);

typedef BOOL (WINAPI *PF_PrivCopyFileExW)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData, LPBOOL pbCancel,	DWORD dwCopyFlags);

typedef	BOOL (WINAPI *PF_MoveFileA)( LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
typedef BOOL (WINAPI *PF_MoveFileW)( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
typedef BOOL (WINAPI *PF_MoveFileExA)(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,DWORD dwFlags);
typedef BOOL (WINAPI *PF_MoveFileExW)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,DWORD dwFlags);

typedef BOOL (WINAPI *PF_MoveFileWithProgressA)(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,DWORD dwFlags);
typedef BOOL (WINAPI *PF_MoveFileWithProgressW)(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,DWORD dwFlags);

typedef	BOOL (WINAPI *PF_ReplaceFileA)(LPCSTR lpReplacedFileName,LPCSTR lpReplacementFileName,LPCSTR lpBackupFileName,DWORD dwReplaceFlags,LPVOID lpExclude,LPVOID lpReserved);
typedef	BOOL (WINAPI *PF_ReplaceFileW)(LPCWSTR lpReplacedFileName,LPCWSTR lpReplacementFileName,LPCWSTR lpBackupFileName,DWORD dwReplaceFlags,LPVOID lpExclude,LPVOID  lpReserved);

typedef BOOL (WINAPI *PF_CreateHardLinkW)(LPCWSTR lpFileName,LPCWSTR lpExistingFileName,LPSECURITY_ATTRIBUTES lpSecurityAttributes);

typedef BOOL (WINAPI *PF_SetFileAttributesW)(LPCWSTR lpFileName, DWORD dwFileAttributes);

typedef DWORD (WINAPI *PF_SetNamedSecurityInfoW)(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl);

typedef DWORD (WINAPI *PF_AddUsersToEncryptedFile)(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers);
typedef DWORD (WINAPI *PF_GetFileAttributesW)(LPCWSTR lpFileName);

typedef BOOL (WINAPI *PF_EncryptFileW)(LPCWSTR lpFileName);
typedef BOOL (WINAPI *PF_DecryptFileW)(LPCWSTR lpFileName, DWORD dwReserved);

typedef BOOL (WINAPI *PF_SetFileInformationByHandle)(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInformation, DWORD dwBufferSize);

typedef int (WINAPI *PF_SHFileOperationW)(LPSHFILEOPSTRUCT lpFileOperation);

typedef HANDLE (WINAPI *PF_SetClipboardData)(UINT uFormat, HANDLE hMem);
typedef HANDLE (WINAPI *PF_GetClipboardData)(UINT uFormat);
typedef HRESULT (WINAPI *PF_OleSetClipboard)(LPDATAOBJECT pDataObj);
typedef HRESULT (WINAPI *PF_OleGetClipboard)(LPDATAOBJECT *ppDataObj);

typedef HRESULT (WINAPI *PF_DoDragDrop)(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect);
typedef HRESULT (WINAPI *PF_RegisterDragDrop)(HWND hwnd, LPDROPTARGET pDropTarget);
typedef HRESULT (WINAPI *PF_RevokeDragDrop)(HWND hwnd);

// internal functions
typedef NTSTATUS (NTAPI *PF_NtCreateFile)(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
                                          ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);

typedef NTSTATUS (NTAPI *PF_NtOpenFile) (OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess, IN ULONG OpenOptions);

typedef NTSTATUS (NTAPI *PF_NtClose)(HANDLE handle);

typedef NTSTATUS (NTAPI *PF_RtlDosPathNameToNtPathName_U_WithStatus)(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName);

typedef NTSTATUS (NTAPI *PF_NtSetSecurityObject)(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor);

typedef BOOL (WINAPI *PF_GetSaveFileNameW)(LPOPENFILENAMEW lpofn);

typedef BOOL (WINAPI *PF_GetOpenFileNameW)(LPOPENFILENAMEW lpofn);

typedef HRESULT (WINAPI *PF_SHSimulateDrop)(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect);


typedef  BOOL (WINAPI* PF_BitBlt) (_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int cx, _In_ int cy, _In_ HDC hdcSrc, _In_ int x1, _In_ int y1, _In_ DWORD rop);
typedef  BOOL (WINAPI* PF_MaskBlt) (_In_ HDC hdcDest, _In_ int xDest, _In_ int yDest, _In_ int width, _In_ int height, _In_ HDC hdcSrc,_In_ int xSrc, _In_ int ySrc, _In_ HBITMAP hbmMask, _In_ int xMask, _In_ int yMask, _In_ DWORD rop);
typedef  BOOL (WINAPI* PF_PlgBlt) (_In_ HDC hdcDest, _In_ CONST POINT * lpPoint, _In_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ int width, _In_ int height, _In_ HBITMAP hbmMask, _In_ int xMask, _In_ int yMask);
typedef  BOOL (WINAPI* PF_StretchBlt) (_In_ HDC hdcDest, _In_ int xDest, _In_ int yDest, _In_ int wDest, _In_ int hDest, _In_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ int wSrc, _In_ int hSrc, _In_ DWORD rop);
typedef  BOOL (WINAPI* PF_PrintWindow) (_In_ HWND hwnd, _In_ HDC hdcBlt, _In_ UINT nFlags);
typedef  HDC (WINAPI* PF_CreateDCA) (LPCSTR pszDriver, _In_ LPCSTR pszDevice, LPCSTR pszPort, _In_ CONST DEVMODEA * pdm);
typedef  HDC (WINAPI* PF_CreateDCW) (LPCWSTR pwszDriver, _In_ LPCWSTR pwszDevice, LPCWSTR pwszPort, _In_ CONST DEVMODEW * pdm);
typedef  HDC (WINAPI* PF_GetDC) (_In_ HWND hWnd);
typedef  HDC (WINAPI* PF_GetDCEx) (_In_ HWND hWnd, _In_ HRGN hrgnClip, _In_ DWORD flags);
typedef  HDC (WINAPI* PF_GetWindowDC) (_In_ HWND hWnd);
typedef  int (WINAPI* PF_ReleaseDC) (_In_ HWND hWnd, _In_ HDC hDC);
typedef  BOOL (WINAPI* PF_DeleteDC) (_In_ HDC hdc);

#pragma endregion


#pragma region ModuleProcessOperation
typedef BOOL (WINAPI *PF_CreateProcessW)(LPCWSTR lpApplicationName,LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,
	 LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,
	 LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo,LPPROCESS_INFORMATION lpProcessInformation);

typedef VOID (WINAPI *PF_ExitProcess)(UINT uExitCode);

typedef BOOL (WINAPI *PF_TerminateProcess)(HANDLE hProcess,UINT uExitCode);

typedef HMODULE	(WINAPI	*PF_LoadLibraryW)(LPCWSTR lpLibFileName);

typedef HMODULE (WINAPI *PF_LoadLibraryExW)(LPCWSTR lpLibFileName,HANDLE hFile,DWORD dwFlags);

typedef BOOL (WINAPI *PF_DeviceIoControl)(HANDLE hDevice,DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,
		LPVOID lpOutBuffer,DWORD nOutBufferSize,LPDWORD lpBytesReturned,LPOVERLAPPED lpOverlapped);

typedef HRESULT (WINAPI *PF_CoCreateInstance)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID  *ppv);
#pragma endregion

#pragma region COMFileOperation

typedef HRESULT(STDMETHODCALLTYPE *PF_COMPerformOperations)(IFileOperation * This);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMNewItem)(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMRenameItem)(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMRenameItems)(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMCopyItems)(IFileOperation * This, IUnknown *punkItems, IShellItem *psiDestinationFolder);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMMoveItems)(IFileOperation * This, IUnknown *punkItems, IShellItem *psiDestinationFolder);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMDeleteItems)(IFileOperation * This, IUnknown *punkItems);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMDeleteItem)(IFileOperation *This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMGetThumbnail)(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMShow)(IFileSaveDialog* pThis, HWND hwndOwner);
typedef HRESULT(STDMETHODCALLTYPE *PF_COMSetData)(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);

#pragma endregion

#pragma region NetworkOperation

typedef HINTERNET(WINAPI *PF_InternetConnectA)(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort,LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext);
typedef HINTERNET(WINAPI *PF_InternetConnectW)(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort,LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext);
typedef BOOL	 (WINAPI *PF_InternetCloseHandle)(HINTERNET hInternet);
typedef HINTERNET(WINAPI *PF_HttpOpenRequestA)(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);
typedef HINTERNET(WINAPI *PF_HttpOpenRequestW)(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);

#pragma endregion

#ifdef __cplusplus
}
#endif


#endif // __FUNC_SIGNATURES_H__



