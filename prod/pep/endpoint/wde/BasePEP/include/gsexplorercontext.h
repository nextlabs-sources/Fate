#ifndef __WDE_GSEXPLORERCONTEXTCONTEXT_H__
#define  __WDE_GSEXPLORERCONTEXTCONTEXT_H__

#include "genericcontext.h"

/***********************************************************************
// Special for gsexplorer
***********************************************************************/
namespace nextlabs
{

class CGSExplorerContext : public CGenericContext
{

public:
    CGSExplorerContext();
private:
	virtual HANDLE MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) ;

	virtual HRESULT MyOleGetClipboard(LPDATAOBJECT *ppDataObj);

	virtual BOOL MyMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);

	virtual HRESULT MyCOMSetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease, PF_COMSetData nextFunc);

private:
	virtual EventResult OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
		LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

	virtual EventResult OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
		LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

	virtual EventResult OnAfterOleGetClipboard(LPDATAOBJECT *ppDataObj);

	virtual EventResult OnBeforeMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

	virtual EventResult OnAfterMoveFileEx(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags, PVOID pUserData);

private:
	virtual EventResult EventBeforeCreateFile(LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }

	virtual EventResult EventAfterCreateFile(HANDLE hFile, LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }

	virtual EventResult EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles);

	virtual EventResult EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData);

	virtual EventResult EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData);

	virtual EventResult EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData);

	virtual EventResult EventBeforePasteContent(const std::wstring& strFileSrc, std::wstring& strFileDst);

	virtual EventResult EventAfterMoveFiles(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, PVOID pUserData);

	virtual EventResult EventBeforeRename(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles, const std::wstring& strNewName);

	virtual EventResult EventBeforeMoveFiles(const std::vector<nextlabs::comhelper::FILEOP>& vecFileOp);

private:
	BOOL HandleEFSObligation(LPCWSTR lpFileName);
	BOOL HookIDataObjectSetData(LPDATAOBJECT pDataObj);

private:
	static HRESULT STDMETHODCALLTYPE Hooked_SetData(IDataObject* pThis, FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);

private:
	static std::map<LPVOID, LPVOID> mapCOMHooks_;

private:
	std::wstring wstrTempPath;
	std::wstring wstrTempPathPrefix;

	bool bBeginDeleteOnPaste;
	bool bMoveFileDenied;
};

}  // ns nextlabs

#endif
