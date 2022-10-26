#ifndef __WDE_EVENTPARSER_H__
#define  __WDE_EVENTPARSER_H__

#include <windows.h>

#include "droptargetproxy.h"
#include "oledragndrop.h"

#include <shellapi.h>
#include <Winternl.h>
#include <WinInet.h>

#pragma warning(push)
#pragma warning(disable: 6387 6011) 

#include <ShObjIdl.h>
#pragma warning(pop)

#include <boost/noncopyable.hpp>
#include <set>
#include <hash_map>

namespace nextlabs
{
BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam);

class CEventParser : public boost::noncopyable
{
public:
    virtual ~CEventParser(){}

    virtual BOOL IsCreateProcess(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
    virtual BOOL IsCreateProcessToOpenFile(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, std::wstring& filePath);
	virtual BOOL IsNewFileAction(IFileOperation * This, IShellItem *psiDestinationFolder, DWORD dwFileAttributes, LPCWSTR pszName, LPCWSTR pszTemplateName, IFileOperationProgressSink *pfopsItem);
    virtual BOOL IsNewFileAction(const std::wstring& strPath, const DWORD& dwCreationDisposition);
    virtual BOOL IsNewFileAction(const std::wstring& strPath, const ULONG& createDisposition, const ULONG& createOptions);

	virtual BOOL IsEditFileAction(const std::wstring& strPath, const DWORD& dwDesiredAccess);

    virtual BOOL IsNewDirectory(const LPCWSTR lpPathName);
    virtual BOOL IsNewDirectory(const LPCWSTR lpPathName, const ULONG createDisposition, const ULONG createOptions);

	virtual BOOL IsRenameAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
	virtual BOOL IsRenameAction(IFileOperation * This, IShellItem *psiDestinationFolder, LPCWSTR pszNewName, IFileOperationProgressSink *pfopsItem);
	virtual BOOL IsRenameAction(IFileOperation * This, IUnknown *pUnkItems, LPCWSTR pszNewName);

    virtual BOOL IsDeleteAction(const IFileOperation* This, const IUnknown *punkItems);
    virtual BOOL IsDeleteAction(const IFileOperation* This, const IShellItem *psiItem, const IFileOperationProgressSink *pfopsItem);
    virtual BOOL IsDeleteAction(const LPCWSTR lpFileName);
    virtual BOOL IsDeleteAction(const DWORD& dwFlagsAndAttributes);
    virtual BOOL IsDeleteAction(ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, ULONG ShareAccess, ULONG OpenOptions);
    virtual BOOL IsDeleteAction(LPSHFILEOPSTRUCT lpFileOperation);

    virtual BOOL IsCopyAction(const IFileOperation* This, const IUnknown *punkItems, const IShellItem *psiDestinationFolder);
    virtual BOOL IsCopyAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists); 
    virtual BOOL IsCopyAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    virtual BOOL IsCopyAction(DWORD *pdwEffect);
    
    virtual BOOL IsMoveAction(const IFileOperation* This, const IUnknown *punkItems, const IShellItem *psiDestinationFolder);
    virtual BOOL IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName); 
    virtual BOOL IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags); 
    virtual BOOL IsMoveAction(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags); 
    virtual BOOL IsMoveAction(DWORD *pdwEffect);

    // file Attributes
    virtual BOOL IsSetFileAttribute(LPCTSTR lpFileName);
    virtual BOOL IsSetAttributeReadOnly(DWORD dwAttributes);
    virtual BOOL IsSetAttributeHidden(DWORD dwAttributes);
    virtual BOOL IsSetAttributeReadOnly(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation);
    virtual BOOL IsSetAttributeHidden(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation);
    virtual BOOL IsFileAttributeChange(FILE_INFO_BY_HANDLE_CLASS FileInformationClass, LPVOID lpFileInfromation);

    virtual BOOL IsSetAttributeEncrypt(LPCWSTR lpFileName);
    virtual BOOL IsSetAttributeCompressed(HANDLE hDevice,DWORD dwIoControlCode, LPVOID lpInBuffer);
    virtual BOOL IsSetAttributeUncompressed(HANDLE hDevice,DWORD dwIoControlCode, LPVOID lpInBuffer);
    virtual BOOL IsSetAttributeUnencrypted(LPCWSTR lpFileName);
    virtual BOOL IsSetFileSecurityAttribute(LPCWSTR pObjectName, SE_OBJECT_TYPE ObjectType);
    virtual BOOL IsSetAddUsersToEncryptedFile(LPCWSTR lpFileName, PENCRYPTION_CERTIFICATE_LIST pUsers);

    //content copy/cut
    virtual BOOL isCopyContent(const UINT uFormat, __out std::wstring &srcFilePath);
    virtual BOOL isCopyContent(const LPDATAOBJECT pDataObject, __out std::wstring& srcFilePath);
    virtual BOOL IsDragDropContent(const LPDATAOBJECT pDataObj, const LPDROPSOURCE pDropSource, const DWORD dwOkEffects, const LPDWORD pdwEffect, __out std::wstring& srcFilePath);
    virtual BOOL isPasteContent(const UINT uFormat, __out std::wstring &dstFilePath, __out std::wstring& srcFilePath);
    virtual BOOL isPasteContent(const LPDATAOBJECT ppDataObject, __out std::wstring &dstFilePath, __out std::wstring& srcFilePath);
    
    virtual BOOL isDropForPasteContent(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::wstring& srcFilePath, __out std::wstring&dstFilePath);
    virtual BOOL isDropForOpenFiles(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::list<std::wstring>& files);
    virtual BOOL isDropForCopyFiles(IDataObject* pDataObject, const DNDWinClassAction& winClassAction, __out std::list<std::wstring>& srcfilePaths, __out std::wstring &dstFilePath);
    

    virtual BOOL IsFileOpen(LPCTSTR lpFileName,
        const DWORD dwDesiredAccess,
        const DWORD dwShareMode,
        const DWORD dwCreationDisposition,
        const DWORD dwFlagsAndAttributes);

    virtual BOOL IsDirectoryOpen(LPCTSTR lpFileName,
        const DWORD dwDesiredAccess,
        const DWORD dwShareMode,
        const DWORD dwCreationDisposition,
        const DWORD dwFlagsAndAttributes);

    virtual BOOL IsDirectoryOpen(LPCTSTR lpFileName);

    
    
    //those two using for drag/drop
    virtual BOOL IsRegisterDragDrop(HWND hWnd, LPDROPTARGET pDropTarget, __out DNDWinClassAction& winClassAction);
    //virtual BOOL IsRevokeDragDrop(HWND hWnd);

    virtual BOOL IsPictureFileType(IShellItem *pShellItem,  std::wstring& filepath);

public:
    void StoreFileHandleWithPath(HANDLE& hFileHandle, LPCWSTR filePath);
    std::wstring GetFilePathByHandle(HANDLE& hFileHandle);
    void RemoveFilePathInfoByHandle(HANDLE& hFileHandle);
    BOOL IsHandleMapContantsFile(const std::wstring& filePath);

    void AddOpenedFile(const std::wstring& filePath);
    void RemoveOpenedFile(const std::wstring& filePath);

	void StoreFilePolicy(HANDLE& hFileHandle, BOOL bWriteAllowed);
	void RemoveFilePolicy(HANDLE& hFileHandle);
	BOOL GetFilePolicy(HANDLE& hFileHandle, BOOL& bWriteAllowed);

	void StorePolicyCacheResult(const std::wstring& filePath, BOOL bAllowed);
	BOOL GetPolicyCacheResult(const std::wstring& filePath, BOOL& bAllowed);

    BOOL StoreClipboardSrcFile(const std::wstring& filePath);
    BOOL GetClipboardSrcFile(__out std::wstring& filePath);
    BOOL CleanClipboardSrcFile();

    BOOL StoreDragDropContentFile(const std::wstring& filePath);
    BOOL GetDragDropContentFile(__out std::wstring& filePath);
    
    void addDropTargetProxy(const HWND& hwnd, DropTargetProxy *pProxy);
    void removeDropTargetProxy(const HWND& hwnd);
    DropTargetProxy* GetDropTargetProxy(const HWND& hwnd);
    void RevokeAllDropTargets();

public:
    virtual BOOL GetCurrentDocumentPath(__out std::wstring& filePath, BOOL bFullPath = TRUE, BOOL *pPathReliable = NULL, const std::wstring& titleName = L"");
    virtual HWND GetCurrentDocumentWindow();
    BOOL GetOLEDropFiles(IDataObject *pDataObject, std::list<std::wstring>& files);

private:
	struct strPolicyCacheResult
	{
		DWORD dwTime;
		BOOL bAllow;
	};

protected:
    std::map<HANDLE, std::wstring> h2fileName_;
    std::set<std::wstring> openedFiles_;
	std::map<HANDLE, BOOL> h2filePolicy_;
	std::map<std::wstring, strPolicyCacheResult> h2PolicyCacheResult_;
    std::map<HWND, DropTargetProxy*> droptargets_;
};


class CExplorerParser: public CEventParser
{
public:
    virtual ~CExplorerParser() {}
    
    virtual BOOL IsSetFileAttribute(LPCTSTR lpFileName);
  
    virtual BOOL IsFileOpen(LPCTSTR lpFileName,
        const DWORD dwDesiredAccess,
        const DWORD dwShareMode,
        const DWORD dwCreationDisposition,
        const DWORD dwFlagsAndAttributes);

    virtual BOOL IsCreateProcessToOpenFile(LPCWSTR lpApplicationName,
        LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment, 
        LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo, 
        LPPROCESS_INFORMATION lpProcessInformation,
        std::wstring& filePath);
};

class CCBParser: public CEventParser
{
public:
	virtual ~CCBParser() {}
};

class CNetworkEventParser : public boost::noncopyable
{
// 	class InetServer
// 	{
// 	public:
// 		std::wstring name;       // Server Name
// 		std::wstring object;     // Current Object Name (i.e. HTTP object)
// 		DWORD last_access;       // Last access time in tick count (ms)
// 		std::wstring uploadpath; // Save the upload path here
// 	////////////////////
// 		InetServer() :name(),object(),uploadpath(),last_access(0){}/* InetServer */
// 	};
	class COpenedInternetHandle
	{
	public:
		COpenedInternetHandle() :inetconn(NULL), server(L""), port(0), refcount(0){}
		COpenedInternetHandle(HINTERNET conn, LPCWSTR wzServer, WORD wPort) :inetconn(conn), server(wzServer), port(wPort), refcount(0){};
		virtual ~COpenedInternetHandle(){};

		const HINTERNET get_inetconn(){ return inetconn; }

		std::wstring get_server(){ return server; }
		void set_server(LPCWSTR pwzServer){ server = pwzServer; }

		const WORD get_port(){ return port; }
		void set_port(WORD wPort){ port = wPort; }

		DWORD inc_ref(){ return ++refcount; }
		DWORD dec_ref(){ if (refcount > 0) --refcount; return refcount; }

	private:
		HINTERNET       inetconn;
		std::wstring    server;
		WORD            port;
		DWORD           refcount;
	};
	typedef stdext::hash_map<HINTERNET, boost::shared_ptr<COpenedInternetHandle>>    INETCONNMAP;
public:
	void OnSuccessInternetConnect(HINTERNET hConnect, LPCWSTR lpszServerName, INTERNET_PORT nServerPort);
	void OnSuccessInternetCloseHandle(HINTERNET hConnect);

	//************************************
	// if return true, outServerPath is valid, caller can get serverpath and used later
	//************************************
	bool OnSuccessHttpOpenRequest(HINTERNET hConnect, LPCWSTR lpszObjectName, std::wstring& outServerPath);
private:
	inline INETCONNMAP::iterator FindIterByKeySafe(HINTERNET key){
		boost::shared_lock<boost::shared_mutex> lock;
		return inetConnMap_.find(key);
	}
private:
	INETCONNMAP inetConnMap_;
	boost::shared_mutex mapMutex_;
};

}  // ns nextlabs

#endif 
