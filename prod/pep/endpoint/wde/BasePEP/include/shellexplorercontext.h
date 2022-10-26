#ifndef __WDE_SHELLEXPLORERCONTEXT_H__
#define  __WDE_SHELLEXPLORERCONTEXT_H__

#include "baseeventprovidercontext.h"

#include <map>
#include <vector>

/***********************************************************************
// Special for Windows Explorer
***********************************************************************/
namespace nextlabs
{

class CShellExploerContext : public CBaseEventProviderContext
{

public:
    CShellExploerContext();
private:
    virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]) ;

	virtual BOOL MyCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);

private:
    virtual EventResult EventBeforeCreateProcess(const std::wstring& wstrAppPath);
	virtual EventResult EventBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations);
    virtual EventResult EventBeforeCopyFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);
    virtual EventResult EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);
    virtual EventResult EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles);
	virtual EventResult EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData);
	virtual EventResult EventBeforeEditFile(const std::wstring& strPath, PVOID pUserData);
    virtual EventResult EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName);
    virtual EventResult EventBeforeSetCompressionAttributeAction(const std::wstring& filePath);
    virtual EventResult EventBeforeSetUncompressionAttributeAction(const std::wstring& deviceName);
   // virtual EventResult EventBeforeSetHiddenAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetEncryptAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetUnencryptedAttributeAction(const std::wstring& deviceName);
   // virtual EventResult EventBeforeSetReadOnlyAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetSecurityAttributeAction(const std::wstring& fileName);
	virtual EventResult EventBeforeWriteFile(const std::wstring& strFile);
	virtual EventResult EventBeforeSetEndOfFile(const std::wstring& strFile);
    virtual EventResult EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData);
    virtual EventResult EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData);
	virtual EventResult EventBeforeDropContent(const std::wstring& strFilesrc, const std::wstring& strFileDst);

	virtual void EventOpenHttpServer(HINTERNET hRequest, const std::wstring& serverUrl);


private:
	virtual EventResult OnBeforeCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, 
		LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
};

}  // ns nextlabs

#endif
