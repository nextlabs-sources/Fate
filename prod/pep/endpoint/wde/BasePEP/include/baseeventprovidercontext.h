#ifndef __WDE_BASEEVENTPROVIDERCONTEXT_H__
#define  __WDE_BASEEVENTPROVIDERCONTEXT_H__

#include "runtimecontext.h"
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4512 4244 6387 6011) 
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#pragma warning(pop)

#include "commonutils.hpp"
#include "droptargetproxy.h"
#include "oledragndrop.h"
#include "GenericSaveAsObligation.h"


namespace nextlabs
{
class CEventParser;
class CNetworkEventParser;
class DropTargetProxy;
/***********************************************************************
// Provide default event parser to notify the derived specific event happens
// and notify the derived that a win32_api is being called,which is supported
// by the base class CRuntimeContext and had enabled in this class's
// override-method OnResponseInitHookTable
//
// a better base class for the derived to inherit
// in the derived class's context it is NOT RECOMMEND to override hooked-api
// method directly, use these event-notify methods instead
***********************************************************************/
class CBaseEventProviderContext : public CRuntimeContext
{
public:
    CBaseEventProviderContext();
public:
    //return value means allow drop or deny drop.
    BOOL finishDrop(IDataObject* pDataObject,DNDWinClassAction winClassAction);
private:
#pragma region ImplAPIs
    virtual HANDLE MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) ;

    virtual HANDLE MyCreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);

    virtual BOOL MyCloseHandle(HANDLE hObject) ;

    virtual HANDLE MyFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);

    virtual BOOL MyReadFile( HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped);

    virtual BOOL MyReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped,LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

    virtual BOOL MyWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    
    virtual BOOL MyWriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine	);

	virtual BOOL MySetEndOfFile(HANDLE hFile);

	virtual BOOL MyKernelBaseSetEndOfFile(HANDLE hFile);

    virtual BOOL MyCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

    virtual BOOL MyCreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) ;

	virtual BOOL MyRemoveDirectoryW(LPCWSTR lpPathName) ;

    virtual BOOL MyDeleteFileW(LPCWSTR lpFileName) ;

    virtual BOOL MyCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) ;

    virtual BOOL MyCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) ;

    virtual BOOL MyPrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) ;

    virtual BOOL MyKernelBasePrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) ;

    virtual BOOL MyMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) ;

    virtual BOOL MyMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) ;

    virtual BOOL MyMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags) ;

	virtual BOOL MyKernelBaseMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) ;

	virtual BOOL MyKernelBaseMoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags) ;

    virtual BOOL MyReplaceFileW(LPCWSTR lpReplacedFileName, LPCWSTR lpReplacementFileName, LPCWSTR lpBackupFileName, DWORD dwReplaceFlags, LPVOID lpExclude, LPVOID lpReserved) ;

    virtual BOOL MyCreateHardLinkW( LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

    virtual BOOL MySetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);

    virtual DWORD MyGetFileAttributesW(LPCWSTR lpFileName);

    virtual DWORD MySetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl);

    virtual DWORD MyAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUSers);

    virtual NTSTATUS MyNtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor);

    virtual BOOL MySetFileInformationByHandle(HANDLE hFile, FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation, DWORD dwBufferSize);

    virtual BOOL MyEncryptFileW(LPCWSTR lpFileName);

    virtual BOOL MyDecryptFileW(LPCWSTR lpFileName, DWORD dwReserved);

    virtual HANDLE MySetClipboardData(UINT uFormat, HANDLE hMem);
	
    virtual HANDLE MyGetClipboardData(UINT uFormat);
	
    virtual HRESULT MyOleSetClipboard(LPDATAOBJECT pDataObj);
	
    virtual HRESULT MyOleGetClipboard(LPDATAOBJECT *ppDataObj);

    virtual HRESULT MyDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect);
	
    virtual HRESULT MyRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget);
	
    virtual HRESULT MyRevokeDragDrop(HWND hwnd);

    virtual NTSTATUS MyNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);
    
    virtual NTSTATUS MyNtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions);

    virtual NTSTATUS MyNtClose(HANDLE handle);

    virtual NTSTATUS MyRtlDosPathNameToNtPathName_U_WithStatus(PWSTR DosFileName, PUNICODE_STRING NtFileName, PWSTR* FilePart, LPVOID RelativeName);

	virtual BOOL MySHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation);

    // implement IHookModuleProcessOperation
    virtual VOID MyExitProcess( UINT uExitCode);

    virtual BOOL MyTerminateProcess(HANDLE hProcess,UINT uExitCode);

    virtual HMODULE MyLoadLibraryW( LPCWSTR lpLibFileName);
    virtual HMODULE	MyLoadLibraryExW(LPCWSTR lpLibFileName,	 HANDLE hFile, DWORD dwFlags);

    virtual BOOL MyDeviceIoControl(HANDLE hDevice,DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,
        LPVOID lpOutBuffer,DWORD nOutBufferSize, LPDWORD lpBytesReturned,LPOVERLAPPED lpOverlapped);

    // Implement ICOMHookFileOperation virtual functions
	virtual HRESULT MyCOMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem, PF_COMNewItem nextfunc);

	virtual HRESULT MyCOMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem, PF_COMRenameItem nextfunc);

	virtual HRESULT MyCOMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName, PF_COMRenameItems nextfunc);

    virtual HRESULT MyCOMCopyItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMCopyItems nextFunc);

    virtual HRESULT MyCOMMoveItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder, PF_COMMoveItems nextFunc);

    virtual HRESULT MyCOMPerformOperations(IFileOperation* This, PF_COMPerformOperations nextFunc);

    virtual HRESULT MyCOMDeleteItems(IFileOperation* This, IUnknown *punkItems, PF_COMDeleteItems nextFunc);

    virtual HRESULT MyCOMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem, PF_COMDeleteItem nextFunc);

    virtual HRESULT MyCOMGetThumbnail(IThumbnailCache *This, IShellItem *pShellItem, UINT cxyRequestedThumbSize, WTS_FLAGS flags, ISharedBitmap **ppvThumb, WTS_CACHEFLAGS *pOutFlags, WTS_THUMBNAILID *pThumbnailID, PF_COMGetThumbnail nextFunc);
	
	virtual HRESULT MyCOMShow(IFileSaveDialog* pThis, HWND hwndOwner, PF_COMShow nextFunc);

	virtual HRESULT MyCOMSetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease, PF_COMSetData nextFunc);

    // Implement IHookNetworkOperation 
	virtual HINTERNET MyInternetConnectA(HINTERNET hInternet, LPCSTR lpszServerName, INTERNET_PORT nServerPort, LPCSTR lpszUserName, LPCSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext) ;

	virtual HINTERNET MyInternetConnectW(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext) ;

	virtual BOOL MyInternetCloseHandle(HINTERNET hInternet);

	virtual HINTERNET MyHttpOpenRequestA(HINTERNET hConnect, LPCSTR lpszVerb, LPCSTR lpszObjectName, LPCSTR lpszVersion, LPCSTR lpszReferrer, LPCSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext) ;

	virtual HINTERNET MyHttpOpenRequestW(HINTERNET hConnect, LPCWSTR lpszVerb, LPCWSTR lpszObjectName, LPCWSTR lpszVersion, LPCWSTR lpszReferrer, LPCWSTR* lplpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext) ;

	virtual BOOL MyGetSaveFileNameW(LPOPENFILENAMEW lpofn);

    virtual BOOL MyGetOpenFileNameW(LPOPENFILENAMEW lpofn);

    virtual HRESULT MySHSimulateDrop(IDropTarget *pDropTarget, IDataObject *pDataObj, DWORD grfKeyState, POINTL *pt, DWORD *pdwEffect);

	virtual BOOL MyBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);

	virtual BOOL MyMaskBlt(HDC hdcDest, int xDest, int yDest, int width, int height, HDC hdcSrc, int xSrc, int ySrc, HBITMAP hbmMask, int xMask, int yMask, DWORD rop);

	virtual BOOL MyPlgBlt(HDC hdcDest, CONST POINT * lpPoint, HDC hdcSrc, int xSrc, int ySrc, int width, int height, HBITMAP hbmMask, int xMask, int yMask);

	virtual BOOL MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);

	virtual BOOL MyPrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);

	virtual HDC MyCreateDCA(LPCSTR pszDriver, LPCSTR pszDevice, LPCSTR pszPort, CONST DEVMODEA * pdm);

	virtual HDC MyCreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pwszPort, CONST DEVMODEW * pdm);

	virtual HDC MyGetDC(HWND hWnd);

	virtual HDC MyGetDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags);

	virtual HDC MyGetWindowDC(HWND hWnd);

	virtual int MyReleaseDC(HWND hWnd, HDC hDC);

	virtual BOOL MyDeleteDC(HDC hdc);

#pragma endregion

private: // notify to the derived that win32_api been calling, give a chance to modify something or detour 
#pragma region APINotify
    virtual EventResult OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
        LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

    virtual EventResult OnBeforeGreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);

    virtual EventResult OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
        LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

    virtual EventResult OnBeforeCloseHandle(HANDLE& hObject)
    { 
        return kEventAllow; 
    }

    virtual EventResult OnBeforeFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags);
    virtual EventResult OnBeforeCreateDirectory(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PVOID pUserData);

	virtual EventResult OnAfterCreateDirectory(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PVOID pUserData);

	virtual EventResult OnBeforeMoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData);

	virtual EventResult OnAfterMoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData);

	virtual EventResult OnBeforeMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnAfterMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnBeforeMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnAfterMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnBeforeKernelBaseMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnAfterKernelBaseMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnBeforeKernelBaseMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnAfterKernelBaseMoveFileWithProgress(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, DWORD dwFlags, PVOID pUserData);

    virtual EventResult OnBeforeDeleteFileW(LPCWSTR lpFileName);

    virtual EventResult OnBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, PVOID pUserData);

	virtual EventResult OnAfterCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, PVOID pUserData);

    virtual EventResult OnBeforeCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData); 

	virtual EventResult OnAfterCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData); 

	virtual EventResult OnBeforePrivCopyFileExW (LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData); 

    virtual EventResult OnBeforeKernelBasePrivCopyFileExW (LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData);  

	virtual EventResult OnAfterPrivCopyFileExW (LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
		LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData); 

    virtual EventResult OnAfterKernelBasePrivCopyFileExW (LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, 
        LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags, PVOID pUserData); 

    virtual EventResult OnBeforeWriteFile(HANDLE hFile, LPCVOID lpBuffer,
        DWORD& nNumberOfBytesToWrite, LPDWORD& lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

	virtual EventResult OnBeforeWriteFileEx(HANDLE hFile, LPCVOID lpBuffer,
		DWORD& nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

	virtual EventResult OnBeforeSetEndOfFile(HANDLE hFile);

    virtual EventResult OnBeforeCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, 
        LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

    virtual EventResult OnBeforeDeviceIoControl(HANDLE hDevice,DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,
        LPVOID lpOutBuffer,DWORD nOutBufferSize, LPDWORD lpBytesReturned,LPOVERLAPPED lpOverlapped);

    
    virtual EventResult OnBeforeSetFileAttributesReadOnly(LPCWSTR lpFileName);

    virtual EventResult OnBeforeSetFileAttributeHidden(LPCWSTR lpFileName);

    virtual EventResult OnBeforeSetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes);

    virtual EventResult OnBeforeGetFileAttributeW(LPCWSTR lpFileName);

    virtual EventResult OnBeforeSetNamedSecurityInfoW(LPWSTR pObjectName, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, PSID psidOwner, PSID psidGroup,PACL pDacl,PACL pSacl);

    virtual EventResult OnBeforeAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers);

    virtual EventResult OnBeforeNtSetSecurityObject(HANDLE handle, SECURITY_INFORMATION securityInformation, PSECURITY_DESCRIPTOR securityDescriptor);

    virtual EventResult OnBeforeEncryptFileW(LPCWSTR lpFileName);

    virtual EventResult OnBeforeDecryptFileW(LPCWSTR lpFileName, DWORD dwReserved);

	virtual EventResult OnBeforeSHFileOperationW(LPSHFILEOPSTRUCT lpFileOperation);

    virtual EventResult OnBeforeSetClipboardData(UINT uFormat, HANDLE hMem);
	
    virtual EventResult OnBeforeGetClipBoardData(UINT uFormat);
    
    virtual EventResult OnBeforeOleSetClipboard(LPDATAOBJECT pDataObj);
	
    virtual EventResult OnAfterOleGetClipboard(LPDATAOBJECT *ppDataObj);

    virtual EventResult OnBeforeDoDragDrop(LPDATAOBJECT pDataObj, LPDROPSOURCE pDropSource, DWORD dwOkEffects, LPDWORD pdwEffect);
    
    virtual EventResult OnBeforeRegisterDragDrop(HWND hwnd, LPDROPTARGET pDropTarget, __out DropTargetProxy *&proxy);
    virtual EventResult OnAftereRegisterDragDrop(const HWND hwnd, DropTargetProxy* &proxy);
	
    virtual EventResult OnBeforeRevokeDragDrop(HWND hwnd);

    virtual EventResult OnBeforeCOMRenameItem(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem);

	virtual EventResult OnBeforeCOMRenameItems(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName);

	virtual EventResult OnBeforeCOMNewItem(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem);

    virtual EventResult OnBeforeCOMCopyItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder);

    virtual EventResult OnBeforeCOMMoveItems(IFileOperation* This, IUnknown *punkItems, IShellItem *psiDestinationFolder);

    virtual EventResult OnBeforeCOMPerformOperations(IFileOperation* This)
    {
        return kEventAllow;
    }

    virtual EventResult OnBeforeCOMDeleteItems(IFileOperation* This, IUnknown *punkItems);


    virtual EventResult OnBeforeCOMDeleteItem(IFileOperation* This, IShellItem *psiItem, IFileOperationProgressSink *pfopsItem);

    virtual EventResult OnBeforeGetThumbnail(IShellItem *pShellItem);

    virtual EventResult OnBeforeNtCreateFile(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes, ULONG ShareAccess, ULONG CreateDisposition, ULONG CreateOptions, PVOID EaBuffer, ULONG EaLength);

    virtual EventResult OnBeforeNtOpenFile(OUT PHANDLE FileHandle, IN ACCESS_MASK DesiredAccess, IN POBJECT_ATTRIBUTES ObjectAttributes, OUT PIO_STATUS_BLOCK IoStatusBlock, IN ULONG ShareAccess, IN ULONG OpenOptions);

    virtual EventResult OnBeforeNtClose(HANDLE handle);

	virtual BOOL OnAfterGetSaveFileNameW(LPOPENFILENAMEW lpofn, const std::wstring& strSource);

    virtual BOOL OnAfterGetOpenFileNameW(LPOPENFILENAMEW lpofn);

	virtual BOOL OnAfterCOMShow(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource);

    virtual EventResult OnBeforeSHSimulateDrop(IDataObject *pDataObj, DWORD *pdwEffect);

	//virtual EventResult OnBeforeInternetConnect(HINTERNET hInternet, LPCWSTR lpszServerName, INTERNET_PORT nServerPort, LPCWSTR lpszUserName, LPCWSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext);
   
#pragma endregion

private: // identified event response-method that this class parsed out, the derived can override them to accept each information

#pragma region ParsedEventNotify

    // notify Com-object CFileOperation will be return
 //   virtual void OnReturnComObject_FileOperation(IFileOperation* pObject){};

    virtual EventResult EventBeforeNormalFileCreate(LPCWSTR lpFileName, PVOID pUserData){ return kEventAllow; }
    virtual EventResult EventBeforeFileOpen(const std::wstring& fileName, PVOID pUserData){ return kEventAllow; }
    virtual EventResult EventBeforeFolderOpen(const std::wstring& folderName, PVOID pUserData){ return kEventAllow; }
    virtual EventResult EventBeforeCreateFile(LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }
    virtual EventResult EventAfterCreateFile(HANDLE hFile, LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }

    virtual EventResult EventBeforeCreateProcess(const std::wstring& wstrAppPath)
    {
        return kEventAllow;
    }

	virtual EventResult EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName)
	{
		return kEventAllow;
	}

	virtual EventResult EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData)
	{
		return kEventAllow;
	}

	virtual EventResult EventBeforeEditFile(const std::wstring& strPath, PVOID pUserData)
	{
		return kEventAllow;
	}

    virtual EventResult EventBeforeNewDirectory(const std::wstring& strPath, PVOID pUserData)
    {
        return kEventAllow;
    }

	virtual EventResult EventAfterNewDirectory(const std::wstring& strPath, PVOID pUserData);
    
    virtual EventResult EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetCompressionAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetUncompressionAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetSecurityAttributeAction(const std::wstring& fileName) { return kEventAllow; }
    virtual EventResult EventBeforeSetReadOnlyAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetHiddenAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow; 
    }

    virtual EventResult EventBeforeSetEncryptAttributeAction(const std::wstring& deviceName) 
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeSetUnencryptedAttributeAction(const std::wstring& deviceName)
    {
        return kEventAllow;
    }

    virtual EventResult EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
        return kEventAllow; 
    }

	virtual EventResult EventAfterMoveFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData);

	virtual EventResult EventBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations)
	{
		return kEventAllow;
	}

    virtual EventResult EventBeforeCopyFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp)
    {
        return kEventAllow;
    }

	virtual EventResult EventAfterCopyFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData);
	
    virtual EventResult EventBeforeCopyContent(const std::wstring& strFileSrc)
    {
        return kEventAllow;
    }
    
    virtual EventResult EventBeforePasteContent(const std::wstring& strFileSrc, std::wstring& strFileDst)
    {
        return kEventAllow;
    }
    
    virtual EventResult EventBeforeDropContent(const std::wstring& strFilesrc, const std::wstring& strFileDst)
    {
        return kEventAllow;
    }

	virtual EventResult EventBeforeWriteFile(const std::wstring& strFile)
	{
		return kEventAllow;
	}

	virtual EventResult EventBeforeSetEndOfFile(const std::wstring& strFile)
	{
		return kEventAllow;
	}

	virtual EventResult	EventAfterSaveAs(LPOPENFILENAMEW lpofn, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		return kEventAllow;
	}

	virtual EventResult	EventAfterSaveAs(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr)
	{
		return kEventAllow;
	}

    virtual EventResult EventAfterGetOpenFileName(LPOPENFILENAMEW pThis)
    {
        return kEventAllow;
    }

	virtual void		EventOpenHttpServer(HINTERNET hRequest, const std::wstring& serverUrl){ return; }

#pragma endregion

private:
    virtual void PurgeDropTargets();

protected:
     boost::shared_ptr<CEventParser> eventParser_;

	 boost::shared_ptr<GenericSaveAsObligation> saveAsObligation;

private:
	EventResult OnBeforeCommonWriteFile(HANDLE hFile);
	BOOL HandleEFSObligation(LPCWSTR lpFileName);

private:
	boost::shared_ptr<CNetworkEventParser> networkEventParser_;

private:
	void AddHDC(_In_ HDC hdc, _In_ DWORD ProcessId);
	void AddHDC(_In_ HDC hdc, _In_ HWND hWnd);
	void RemoveHDC(_In_ HDC hdc);
	bool GetHDCInfo(_Out_ DWORD& ProcessId, _In_ HDC hdc);

	bool Query(_Out_ std::wstring& DisplayText, _In_ HDC hdc);
	bool Query(_Out_ std::wstring& DisplayText, _In_ HWND hWnd);
	bool Query(_Out_ std::wstring& DisplayText, _In_ DWORD ProcessID);

	bool Parse(_Out_ std::wstring& DisplayText, _In_ const std::string& str);

	boost::mutex	ms_Mutex;
	std::map<HDC, DWORD> ms_hdcInfo;
	HWND DesktophWnd;
};
}  // ns nextlabs

#endif
