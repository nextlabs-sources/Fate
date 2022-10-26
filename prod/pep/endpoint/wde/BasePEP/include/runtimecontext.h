#ifndef __WDE_RUNTIMECONTEXT_H__
#define __WDE_RUNTIMECONTEXT_H__

#ifdef _MSC_VER
#pragma once
#else
#error "commonutils.hpp only supports windows-compile"
#endif // _MSC_VER

#include <map>

#pragma warning(push)
#pragma warning(disable: 4005 6387) 
#include <ntstatus.h>
#include <Shobjidl.h>
#pragma warning(pop)

#include "funcsignatures.hpp"
#include <nlexcept.h>
#include <AccCtrl.h>
// nextlabs pep common
#include <eframework/auto_disable/auto_disable.hpp>
// end nextlabs pep common
namespace nextlabs
{


class IHookFileOperation 
{
public:
	// create&close
	virtual HANDLE MyCreateFileW(LPCWSTR lpFileName,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,HANDLE hTemplateFile) =0;
    virtual HANDLE MyCreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName) = 0;
	virtual BOOL MyCloseHandle( HANDLE hObject) =0;
    virtual HANDLE MyFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags) = 0;
	//dir
	virtual BOOL MyCreateDirectoryW( LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) =0;
	virtual BOOL MyRemoveDirectoryW( LPCWSTR lpPathName) =0;
	//del
	virtual BOOL MyDeleteFileW( LPCWSTR lpFileName) =0;
	//read
	virtual BOOL MyReadFile( HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped) =0;
	virtual BOOL MyReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped,LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) =0;
	//write
	virtual BOOL MyWriteFile( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)=0;
	virtual BOOL MyWriteFileEx( HANDLE hFile,	 LPCVOID lpBuffer,	DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine	)=0;
	virtual BOOL MySetEndOfFile(HANDLE hFile) =0;
	virtual BOOL MyKernelBaseSetEndOfFile(HANDLE hFile) =0;

	//copy
	virtual BOOL MyCopyFileW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) =0;
	virtual BOOL MyCopyFileExW(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel, DWORD dwCopyFlags) =0;
    virtual BOOL MyPrivCopyFileExW(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel, DWORD dwCopyFlags) =0;
    //kernelbase copy
    virtual BOOL MyKernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine,LPVOID lpData,LPBOOL pbCancel, DWORD dwCopyFlags) =0;
	//move
	virtual BOOL MyMoveFileW( LPCWSTR lpExistingFileName,	 LPCWSTR lpNewFileName) =0;
	virtual BOOL MyMoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)=0;
	virtual BOOL MyMoveFileWithProgressW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,  DWORD dwFlags)=0;
	//kernelbase move
	virtual BOOL MyKernelBaseMoveFileExW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)=0;
	virtual BOOL MyKernelBaseMoveFileWithProgressW( LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName,LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData,  DWORD dwFlags)=0;
	//replace
	virtual BOOL MyReplaceFileW(LPCWSTR lpReplacedFileName, LPCWSTR lpReplacementFileName, LPCWSTR lpBackupFileName,DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID  lpReserved)=0;
	//hard link
	virtual BOOL MyCreateHardLinkW( LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)=0;
    // File Attribute
    virtual BOOL MyEncryptFileW(LPCWSTR lpFileName) = 0;
    virtual BOOL MyDecryptFileW(LPCWSTR lpFileName, DWORD dwReserved) = 0;
    virtual BOOL MySetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes ) = 0;
    virtual DWORD MyGetFileAttributesW(LPCWSTR lpFileName) = 0;
    virtual BOOL MySetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation, DWORD dwBufferSize) = 0;
    virtual DWORD MySetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl) = 0;
    virtual DWORD MyAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers) = 0;
    virtual NTSTATUS MyNtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor) = 0;
    //copycontent
    virtual HANDLE MySetClipboardData(UINT uFormat, HANDLE hMem) = 0;
	virtual HANDLE MyGetClipboardData(UINT uFormat) = 0;
	virtual HRESULT MyOleSetClipboard(LPDATAOBJECT pDataObj) = 0;
	virtual HRESULT MyOleGetClipboard(LPDATAOBJECT *ppDataObj) = 0;

    //drag/drop
    virtual HRESULT MyDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect) = 0;
	virtual HRESULT MyRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget) = 0;
	virtual HRESULT MyRevokeDragDrop(HWND hwnd) = 0;

	virtual int MySHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation) = 0;

    virtual NTSTATUS MyRtlDosPathNameToNtPathName_U_WithStatus(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName) = 0;
    virtual NTSTATUS MyNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength) = 0;
    virtual NTSTATUS MyNtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions) = 0; 
    virtual NTSTATUS MyNtClose(HANDLE handle) = 0;

	virtual BOOL MyGetSaveFileNameW(LPOPENFILENAMEW lpofn) = 0;
    virtual BOOL MyGetOpenFileNameW(LPOPENFILENAMEW lpofn) = 0;

    virtual HRESULT MySHSimulateDrop(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect) = 0;
};

class IHookModuleProcessOperation
{
public:
	virtual BOOL MyCreateProcessW(LPCWSTR lpApplicationName,LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles,DWORD dwCreationFlags,LPVOID lpEnvironment,
		LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo,LPPROCESS_INFORMATION lpProcessInformation)=0;
	
	virtual VOID MyExitProcess( UINT uExitCode)=0;
	virtual BOOL MyTerminateProcess(HANDLE hProcess,UINT uExitCode)=0;

	virtual HMODULE MyLoadLibraryW( LPCWSTR lpLibFileName)=0;
	virtual HMODULE	MyLoadLibraryExW(	 LPCWSTR lpLibFileName,	 HANDLE hFile, DWORD dwFlags)=0;

	virtual BOOL MyDeviceIoControl(HANDLE hDevice,DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,
		LPVOID lpOutBuffer,DWORD nOutBufferSize, LPDWORD lpBytesReturned,LPOVERLAPPED lpOverlapped)=0;

	virtual HRESULT MyCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID  *ppv) =0;

};

class ICOMHookFileOperation
{
public:
    virtual HRESULT MyCOMCopyItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMCopyItems nextFunc) = 0;
    virtual HRESULT MyCOMMoveItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMMoveItems nextFunc) = 0;
    virtual HRESULT MyCOMPerformOperations(IFileOperation* This, PF_COMPerformOperations nextFunc) = 0;
	virtual HRESULT MyCOMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem, PF_COMNewItem nextfunc) = 0;
	virtual HRESULT MyCOMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem, PF_COMRenameItem nextfunc) = 0;
	virtual HRESULT MyCOMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName, PF_COMRenameItems nextfunc) = 0;
    virtual HRESULT MyCOMDeleteItems(IFileOperation* This, IUnknown *punkItems, PF_COMDeleteItems nextFunc) = 0;
    virtual HRESULT MyCOMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem, PF_COMDeleteItem nextFunc) = 0;

};

class ICOMHookThumbnailCacheOperation 
{
public:
    virtual HRESULT MyCOMGetThumbnail(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID, PF_COMGetThumbnail nextFunc) = 0;
};

class ICOMHookFileSaveDialog 
{
public:
	virtual HRESULT MyCOMShow(IFileSaveDialog* pThis, HWND hwndOwner, PF_COMShow nextFunc) = 0;
};

class ICOMHookDataObject
{
public:
	virtual HRESULT MyCOMSetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease, PF_COMSetData nextFunc) = 0;
};

class IHookNetworkOperation {
public:
	virtual HINTERNET MyInternetConnectA(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext) = 0;
	virtual HINTERNET MyInternetConnectW(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext) = 0;
	virtual BOOL      MyInternetCloseHandle(HINTERNET hInternet)=0;
	virtual HINTERNET MyHttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext) = 0;
	virtual HINTERNET MyHttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext) = 0;

};

class IHookScreenCaptureOperation {
public:
	virtual BOOL MyBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) = 0;
	virtual BOOL MyMaskBlt(HDC hdcDest, int xDest, int yDest, int width, int height, HDC hdcSrc, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask, DWORD rop) = 0;
	virtual BOOL MyPlgBlt(HDC hdcDest, CONST POINT * lpPoint, HDC hdcSrc, int xSrc, int ySrc, int width, int height, HBITMAP hbmMask, int xMask, int yMask) = 0;
	virtual BOOL MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop) = 0;
	virtual BOOL MyPrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags) = 0;
	virtual HDC MyCreateDCA(LPCSTR pszDriver, LPCSTR pszDevice, LPCSTR pszPort, CONST DEVMODEA * pdm) = 0;
	virtual HDC MyCreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pwszPort, CONST DEVMODEW * pdm) = 0;
	virtual HDC MyGetDC(HWND hWnd) = 0;
	virtual HDC MyGetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags) = 0;
	virtual HDC MyGetWindowDC(HWND hWnd) = 0;
	virtual int MyReleaseDC(HWND hWnd, HDC hDC) = 0;
	virtual BOOL MyDeleteDC(HDC hdc) = 0;
};

class CRuntimeContext : public IHookFileOperation,
	public IHookModuleProcessOperation,
	public ICOMHookFileOperation,
	public ICOMHookThumbnailCacheOperation,
	public ICOMHookFileSaveDialog,
	public IHookNetworkOperation,
	public IHookScreenCaptureOperation,
	public ICOMHookDataObject {
public:
	typedef enum HookManifiest{
		//IHookFileOperation
		kHMCreateFileW =0,
        kHMCreateFileMappintW,
		kHMCloseHandle,
        kHMFindFirstFileExW,
		kHMCreateDirectoryW,
		kHMRemoveDirectoryW,
		kHMDeleteFileW,
		kHMReadFile,
		kHMReadFileEx,
		kHMWriteFile,
		kHMWriteFileEx,
		kHMSetEndOfFile,
		kHMKernelBaseSetEndOfFile,
		kHMCopyFileW,
		kHMCopyFileExW,
        kHMPrivCopyFileExW, 
        kHMKernelBasePrivCopyFileExW,
		kHMMoveFileW,
		kHMMoveFileExW,
		kHMMoveFileWithProgressW,
		kHMKernelBaseMoveFileExW,
		kHMKernelBaseMoveFileWithProgressW,
		kHMReplaceFileW,
		kHMCreateHardLinkW,
        //
        kHMSHSimulateDrop,
		//IHookModuleProcessOperation
		kHMCreateProcessW,
		kHMExitProcess,
		kHMTerminateProcess,
		kHMLoadLibraryW,
		kHMLoadLibraryExW,
		kHMDeviceIoControl,
        kHMSetFileInformationByHandle,
        kHMGetOpenFileName,
		//ICOMHookFileOperation
		kHMCoCreateInstance,
        kHMCOMPerformOperations,
		kHMCOMNewItem,
		kHMCOMRenameItem,
		kHMCOMRenameItems,
        kHMCOMCopyItems,
        kHMCOMMoveItems,
        kHMCOMDeleteItems,
        kHMCOMDeleteItem,
        kHMCOMThumbnailCache,
		kHMCOMShow,
        kHMSetFileAttributesW,
        kHMSetNamedSecurityInfoW,
        kHMGetFileAttributesW,
        kHMAddUsersToEncryptedFile,
        kHMEncryptFileW,
        kHMDecryptFileW,
        kHMSetClipboardData,
		kHMGetClipboardData,
		kHMOleSetClipboard,
		kHMOleGetClipboard,
        //
        kHMDoDragDrop,
		kHMRegisterDragDrop,
		kHMRevokeDragDrop,
        kHMSHFileOperationW,
        kHMNtCreateFile,
        kHMNtOpenFile,
        kHMNtClose,
        kHMRtlDosPathNameToNtPathName_U_WithStatus,
        kHMNtSetSecurityObject,
		//IHookNetworkOperation
		kHMInternetConnectA,
		kHMInternetConnectW,
		kHMInternetCloseHandle,
		kHMHttpOpenRequestA,
		kHMHttpOpenRequestW,
		////////////////////////////
		kHMGetSaveFileNameW,
		kHMBitBlt,
		kHMMaskBlt,
		kHMPlgBlt,
		kHMStretchBlt,
		kHMPrintWindow,
		kHMCreateDCA,
		kHMCreateDCW,
		kHMDeleteDC,
		kHMGetDC,
		kHMGetDCEx,
		kHMGetWindowDC,
		kHMReleaseDC,
		kHM_MaxItems
	};
protected:
	CRuntimeContext(){}
	CRuntimeContext(CRuntimeContext&);

public:
	BOOL Init();
	void Deinit();
	BOOL OnHookInit();
	void OnHookDeinit();
private:
	BOOL OnInitCESDK();
	BOOL OnHookInstall();
	void OnHookUninstall();
private: // designed for the derived to override
	//************************************
	// Method:    OnResponseInitHookTable
	// Description:		
	//		derived class can override this method to specify which function to be hooked
	// Example:
	//		table[kHMCreateFileW]=true   , to hook kernel32!CreateFileW
	//************************************
	virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]);	

	//************************************
	// Method:    OnResponseHookInstallOthers/OnResponseHookUninstallOthers
	// Description:
	//		give derived class a chance to set/unset its own hook intent
	// Example:
	//		Derived class may want to hook MFC_Internel32!aMfcDllFun which is not supported by RuntimeContext.
	//		Here is the good place to do that
	//************************************
	virtual BOOL OnResponseHookInstallOthers() { return TRUE; }
	virtual void OnResponseHookUninstallOthers() {}
	
    virtual void PurgeDropTargets(){}

private:  // implement MyCoCreateInstance, in MyCoCreateInstance, we need to handle COM hook.
	virtual HRESULT MyCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

    void HookIFileOperation();
	void HookIThumbnailCache();
	void HookFileSaveDialog();
protected:
	//
	// control recursion for hooks
	//
	static nextlabs::recursion_control hook_control;
	//
	// impl IFileOperation internally
	//
	static PF_CreateFileW Hooked_CreateFileW_Next;
    static PF_CreateFileMappingW Hooked_CreateFileMappingW_Next;
	static PF_CloseHandle Hooked_CloseHandle_Next;
    static PF_FindFirstFileExW Hooked_FindFirstFileExW_Next;
	static PF_CreateDirectoryW Hooked_CreateDirectoryW_Next;
	static PF_RemoveDirectoryW Hooked_RemoveDirectoryW_Next;
	static PF_DeleteFileW Hooked_DeleteFileW_Next;
	static PF_ReadFile Hooked_ReadFile_Next;
	static PF_ReadFileEx Hooked_ReadFileEx_Next;
	static PF_WriteFile Hooked_WriteFile_Next;
	static PF_WriteFileEx Hooked_WriteFileEx_Next;
	static PF_SetEndOfFile Hooked_SetEndOfFile_Next;
	static PF_SetEndOfFile Hooked_KernelBaseSetEndOfFile_Next;
	static PF_CopyFileW Hooked_CopyFileW_Next;
	static PF_CopyFileExW Hooked_CopyFileExW_Next;
    static PF_PrivCopyFileExW Hooked_PrivCopyFileExW_Next;
    static PF_PrivCopyFileExW Hooked_KernelBasePrivCopyFileExW_Next;
	static PF_MoveFileW Hooked_MoveFileW_Next;
	static PF_MoveFileExW Hooked_MoveFileExW_Next;
	static PF_MoveFileWithProgressW Hooked_MoveFileWithProgressW_Next;
	static PF_MoveFileExW Hooked_KernelBaseMoveFileExW_Next;
	static PF_MoveFileWithProgressW Hooked_KernelBaseMoveFileWithProgressW_Next;
	static PF_ReplaceFileW Hooked_ReplaceFileW_Next;
	static PF_CreateHardLinkW Hooked_CreateHardLinkW_Next;
    static PF_SetFileAttributesW Hooked_SetFileAttributesW_Next;
    static PF_SetNamedSecurityInfoW Hooked_SetNamedSecurityInfoW_Next;
    static PF_GetFileAttributesW Hooked_GetFileAttributesW_Next;
    static PF_AddUsersToEncryptedFile Hooked_AddUsersToEncryptedFile_Next;
    static PF_SetFileInformationByHandle Hooked_SetFileInformationByHandle_Next;
    static PF_EncryptFileW Hooked_EncryptFileW_Next;
    static PF_DecryptFileW Hooked_DecryptFileW_Next;
	static PF_SHFileOperationW Hooked_SHFileOperationW_Next;

    static PF_SetClipboardData Hooked_SetClipboardData_Next;
	static PF_GetClipboardData Hooked_GetClipboardData_Next;
	static PF_OleSetClipboard Hooked_OleSetClipboard_Next;
	static PF_OleGetClipboard Hooked_OleGetClipboard_Next;

    static PF_DoDragDrop Hooked_DoDragDrop_Next;
	static PF_RegisterDragDrop Hooked_RegisterDragDrop_Next;
	static PF_RevokeDragDrop Hooked_RevokeDragDrop_Next;

    // internal functions
    static PF_NtCreateFile Hooked_NtCreateFile_Next;
    static PF_NtOpenFile Hooked_NtOpenFile_Next;
    static PF_NtClose Hooked_NtClose_Next;
    static PF_RtlDosPathNameToNtPathName_U_WithStatus Hooked_RtlDosPathNameToNtPathName_U_WithStatus_Next;
    static PF_NtSetSecurityObject Hooked_NtSetSecurityObject_Next;
	//
	// impl IModuleProcessOperation internally
	//
	static PF_CreateProcessW Hooked_CreateProcessW_Next;
	static PF_ExitProcess Hooked_ExitProcess_Next;
	static PF_TerminateProcess Hooked_TerminateProcess_Next;
	static PF_LoadLibraryW Hooked_LoadLibraryW_Next;
	static PF_LoadLibraryExW Hooked_LoadLibraryExW_Next;
	static PF_DeviceIoControl Hooked_DeviceIoControl_Next;
	static PF_CoCreateInstance Hooked_CoCreateInstance_Next;
	
	//
	// impl IHookNetworkOperation internallly
	//
	static PF_InternetConnectA		Hooked_InternetConnectA_Next;
	static PF_InternetConnectW		Hooked_InternetConnectW_Next;
	static PF_InternetCloseHandle	Hooked_InternetCloseHandle_Next;
	static PF_HttpOpenRequestA		Hooked_HttpOpenRequestA_Next;
	static PF_HttpOpenRequestW		Hooked_HttpOpenRequestW_Next;

	static PF_GetSaveFileNameW		Hooked_GetSaveFileNameW_Next;
    static PF_GetOpenFileNameW      Hooked_GetOpenFileNameW_Next;

    static PF_SHSimulateDrop        Hooked_SHSimulateDrop_Next;

	static PF_BitBlt			Hooked_BitBlt_Next;
	static PF_MaskBlt			Hooked_MaskBlt_Next;
	static PF_PlgBlt			Hooked_PlgBlt_Next;
	static PF_StretchBlt		Hooked_StretchBlt_Next;
	static PF_PrintWindow		Hooked_PrintWindow_Next;
	static PF_CreateDCA			Hooked_CreateDCA_Next;
	static PF_CreateDCW			Hooked_CreateDCW_Next;
	static PF_DeleteDC			Hooked_DeleteDC_Next;
	static PF_GetDC				Hooked_GetDC_Next;
	static PF_GetDCEx			Hooked_GetDCEx_Next;
	static PF_GetWindowDC		Hooked_GetWindowDC_Next;
	static PF_ReleaseDC			Hooked_ReleaseDC_Next;

private:
	//
	// impl IFileOperation internally
	//
	static HANDLE	WINAPI	Hooked_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    static HANDLE WINAPI Hooked_CreateFileMappingW( HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName );
	static BOOL WINAPI	Hooked_CloseHandle(HANDLE hObject);
    static HANDLE WINAPI Hooked_FindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
	static BOOL WINAPI	Hooked_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
	static BOOL WINAPI	Hooked_RemoveDirectoryW(LPCWSTR lpPathName);
	static BOOL WINAPI	Hooked_DeleteFileW(LPCWSTR lpFileName);
	static BOOL WINAPI	Hooked_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	static BOOL WINAPI	Hooked_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	static BOOL WINAPI	Hooked_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
	static BOOL WINAPI	Hooked_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	static BOOL WINAPI Hooked_SetEndOfFile(HANDLE hFile);
	static BOOL WINAPI Hooked_KernelBaseSetEndOfFile(HANDLE hFile);
	static BOOL WINAPI	Hooked_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
	static BOOL WINAPI	Hooked_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    static BOOL WINAPI	Hooked_PrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    static BOOL WINAPI	Hooked_KernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
	static BOOL WINAPI	Hooked_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
	static BOOL WINAPI	Hooked_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);
	static BOOL WINAPI	Hooked_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
	static BOOL WINAPI	Hooked_KernelBaseMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);
	static BOOL WINAPI	Hooked_KernelBaseMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
	static BOOL WINAPI	Hooked_ReplaceFileW(LPCWSTR lpReplacedFileName, LPCWSTR lpReplacementFileName, LPCWSTR lpBackupFileName, DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID  lpReserved);
	static BOOL WINAPI	Hooked_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    static BOOL WINAPI  Hooked_SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);
    static DWORD WINAPI Hooked_GetFileAttributesW(LPCWSTR lpFileName);
    static DWORD WINAPI Hooked_SetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl);
    static DWORD WINAPI Hooked_AddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers);
  
    static BOOL WINAPI  Hooked_SetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInfomationClass, LPVOID lpFileInformation, DWORD dwBufferSize);
    static BOOL WINAPI  Hooked_EncryptFileW(LPCWSTR lpFileName);
    static BOOL WINAPI  Hooked_DecryptFileW(LPCWSTR lpFileName, DWORD dwReserved);
	static int  WINAPI  Hooked_SHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation);
    
    static HANDLE WINAPI Hooked_SetClipboardData(UINT uFormat, HANDLE hMem);
	static HANDLE WINAPI Hooked_GetClipboardData(UINT uFormat);
	static HRESULT WINAPI Hooked_OleSetClipboard(LPDATAOBJECT pDataObj);
	static HRESULT WINAPI Hooked_OleGetClipboard(LPDATAOBJECT *ppDataObj);

    static HRESULT WINAPI Hooked_DoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect);
    static HRESULT WINAPI Hooked_RegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget);
	static HRESULT WINAPI Hooked_RevokeDragDrop(HWND hwnd);
    
    static HRESULT WINAPI Hooked_SHSimulateDrop(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect);
        

    // internal functions
    static NTSTATUS NTAPI Hooked_NtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
    static NTSTATUS NTAPI Hooked_NtOpenFile (OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions);
    static NTSTATUS NTAPI Hooked_NtClose(HANDLE handle);
    static NTSTATUS NTAPI Hooked_RtlDosPathNameToNtPathName_U_WithStatus(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName);
    static NTSTATUS NTAPI Hooked_NtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor);

    //
	// impl IModuleProcessOperation internally
	//
	static BOOL WINAPI Hooked_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
	static VOID WINAPI Hooked_ExitProcess(UINT uExitCode);
	static BOOL WINAPI Hooked_TerminateProcess(HANDLE hProcess, UINT uExitCode);
	static HMODULE	WINAPI	Hooked_LoadLibraryW(LPCWSTR lpLibFileName);
	static HMODULE WINAPI Hooked_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
	static BOOL WINAPI Hooked_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize,LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
	static HRESULT WINAPI Hooked_CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv);

    //
    // impl ICOMFileOperation
	//
    static HRESULT STDMETHODCALLTYPE Hooked_COMPerformOperations(IFileOperation * This);
	static HRESULT STDMETHODCALLTYPE Hooked_COMNewItem (IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem);
	static HRESULT STDMETHODCALLTYPE Hooked_COMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem);
	static HRESULT STDMETHODCALLTYPE Hooked_COMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName);
    static HRESULT STDMETHODCALLTYPE Hooked_COMCopyItems(IFileOperation* This,  IUnknown *punkItems,  IShellItem *psiDestinationFolder);
    static HRESULT STDMETHODCALLTYPE Hooked_COMMoveItems(IFileOperation* This,  IUnknown *punkItems,  IShellItem *psiDestinationFolder);
    static HRESULT STDMETHODCALLTYPE Hooked_COMDeleteItems(IFileOperation* This, IUnknown *punkItems);
    static HRESULT STDMETHODCALLTYPE Hooked_COMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem);
    static HRESULT STDMETHODCALLTYPE Hooked_COMGetThumbnail(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID);
	static HRESULT STDMETHODCALLTYPE Hooked_COMShow(IFileSaveDialog* pThis, HWND hwndOwner);

    //
	// impl IHookNetworkOperation
	//
	static HINTERNET WINAPI Hooked_InternetConnectA(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext);
	static HINTERNET WINAPI Hooked_InternetConnectW(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext);
	static BOOL		 WINAPI Hooked_InternetCloseHandle(HINTERNET hInternet);
	static HINTERNET WINAPI Hooked_HttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);
	static HINTERNET WINAPI Hooked_HttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext);

	static BOOL WINAPI Hooked_GetSaveFileNameW(LPOPENFILENAMEW lpofn);

    static BOOL WINAPI Hooked_GetOpenFileNameW(LPOPENFILENAMEW lpofn);

	static BOOL WINAPI Hooked_BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
	static BOOL WINAPI Hooked_MaskBlt(HDC hdcDest, int xDest, int yDest, int width, int height, HDC hdcSrc, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask, DWORD rop);
	static BOOL WINAPI Hooked_PlgBlt(HDC hdcDest, CONST POINT * lpPoint, HDC hdcSrc, int xSrc, int ySrc, int width, int height, HBITMAP hbmMask, int xMask, int yMask);
	static BOOL WINAPI Hooked_StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);
	static BOOL WINAPI Hooked_PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);
	static HDC WINAPI Hooked_CreateDCA(LPCSTR pszDriver, LPCSTR pszDevice, LPCSTR pszPort, CONST DEVMODEA * pdm);
	static HDC WINAPI Hooked_CreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pwszPort, CONST DEVMODEW * pdm);
	static HDC WINAPI Hooked_GetDC(HWND hWnd);
	static HDC WINAPI Hooked_GetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags);
	static HDC WINAPI Hooked_GetWindowDC(HWND hWnd);
	static int WINAPI Hooked_ReleaseDC(HWND hWnd, HDC hDC);
	static BOOL WINAPI Hooked_DeleteDC(HDC hdc);

private:
	//
	// in order to support Windows featured __try&__except and then to perform mini-dump 
	//
	static void exception_cb(NLEXCEPT_CBINFO* cbinfo);


private:
	// control whether a specified Win32_API to be hooked or not,
	//	- true , hooked
	//	- false, ignored
	bool hooktable_[kHM_MaxItems];
    std::map<LPVOID, LPVOID> mapCOMHooks_;  // Store hooks for COM object.
};
}  // ns nextlabs

#endif //__WDE_RUNTIMECONTEXT_H__
