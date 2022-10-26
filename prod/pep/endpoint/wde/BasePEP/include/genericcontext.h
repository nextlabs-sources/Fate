#ifndef __WDE_GENERICCONTEXT_H__
#define  __WDE_GENERICCONTEXT_H__

#include "baseeventprovidercontext.h"

namespace nextlabs
{

class CGenericContext : public CBaseEventProviderContext
{

private:
    virtual void OnResponseInitHookTable(bool(&table)[kHM_MaxItems]);
    virtual EventResult EventBeforeCreateProcess(const std::wstring& wstrAppPath);
    virtual EventResult EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData);
	virtual EventResult EventBeforeEditFile(const std::wstring& strPath, PVOID pUserData);
    virtual EventResult EventBeforeNewDirectory(const std::wstring& strPath, PVOID pUserData);
    virtual EventResult EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName);
    virtual EventResult EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles);
    virtual EventResult EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);
	virtual EventResult EventBeforeCopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists, nextlabs::Obligations& obligations);
    virtual EventResult EventBeforeCopyFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);

 //   virtual EventResult EventBeforeSetHiddenAttributeAction(const std::wstring& deviceName);
   // virtual EventResult EventBeforeSetReadOnlyAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetSecurityAttributeAction(const std::wstring& fileName);
    virtual EventResult EventBeforeSetEncryptAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetUnencryptedAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetCompressionAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeSetUncompressionAttributeAction(const std::wstring& deviceName);
    virtual EventResult EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData);
    virtual EventResult EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData);

    virtual EventResult EventBeforeCopyContent(const std::wstring& strFileSrc);
    virtual EventResult EventBeforePasteContent(const std::wstring& strFileSrc, std::wstring& strFileDst);
    virtual EventResult EventBeforeDropContent(const std::wstring& strFilesrc, const std::wstring& strFileDst);
	virtual EventResult EventBeforeWriteFile(const std::wstring& strFile);
	virtual EventResult EventBeforeSetEndOfFile(const std::wstring& strFile);

	virtual	EventResult EventAfterSaveAs(LPOPENFILENAMEW lpofn, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr);
	virtual	EventResult EventAfterSaveAs(IFileSaveDialog* pThis, HWND hwndOwner, const std::wstring& strSource, SaveAsInfo::SaveAsStruct* pComSaveAsStr);

	virtual void EventOpenHttpServer(HINTERNET hRequest, const std::wstring& serverUrl);

};

}  // ns nextlabs
#endif
