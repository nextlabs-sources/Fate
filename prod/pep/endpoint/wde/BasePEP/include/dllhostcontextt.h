#ifndef __WDE_DLLHOSTCONTEXTCONTEXT_H__
#define  __WDE_DLLHOSTCONTEXTCONTEXT_H__

#include "genericcontext.h"

/***********************************************************************
// Special for dll host
***********************************************************************/
namespace nextlabs
{

class CDllHostContext : public CGenericContext
{

public:
    CDllHostContext();
private:
	virtual HANDLE MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) ;

private:
	virtual EventResult OnBeforeCreateFileW(LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
		LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

	virtual EventResult OnAfterCreateFileW(HANDLE& hFile, LPCWSTR lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode,
		LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, PVOID pUserData);

private:
	virtual EventResult EventBeforeCreateFile(LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }

	virtual EventResult EventAfterCreateFile(HANDLE hFile, LPCWSTR lpFileName, PVOID pUserData) { return kEventAllow; }

	virtual EventResult EventBeforeDeleteFiles(const std::vector<std::pair<std::wstring, nextlabs::Obligations>>& vecFiles);

	virtual EventResult EventBeforeNewFile(const std::wstring& strPath, PVOID pUserData);

	virtual EventResult EventBeforeFileOpen(const std::wstring& filePath, PVOID pUserData);

	virtual EventResult EventBeforeFolderOpen(const std::wstring& folderPath, PVOID pUserData);
};

}  // ns nextlabs

#endif
