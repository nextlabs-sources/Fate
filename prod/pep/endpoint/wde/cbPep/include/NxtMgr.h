#pragma once

#include <shobjidl.h>
#include <string>
#include <tlhelp32.h>

class CNxtMgr
{
public:
	static CNxtMgr* Instance();

	bool NeedBlock();
	bool PreHandleEncryption(IShellItem * src, IShellItem * dest);//return true or false, if false, that means the action was blocked.
	bool PreHandleEncryption(LPCWSTR src, LPCWSTR dest);
	void PostHandleEncryption(IShellItem * src, IShellItem * dest, LPCWSTR pszNewName);
	void PostHandleEncryption(LPCWSTR src, LPCWSTR dest);

	void PopupWarningBox(LPCWSTR title, LPCWSTR content);

	void Init();
	void Uninit();
	bool IsInitialized();

	std::wstring GetCurModuleName();
	std::string MyWideCharToMultipleByte(const std::wstring & strValue);
protected:
	bool IsFile(IShellItem* psiItem);
	bool IsLocalEncrypted(LPCWSTR pszFilePath);
	bool IsLocalEncrypted(IShellItem* item);
	bool IsLocalPath(LPCWSTR pszFileName);
	bool FileExists(LPCWSTR pszFileName);

	

	/*************************************************
	if we copy a file a.txt to another folder, though 
	a.txt exists in dest folder already, then windows 7 
	has a feature "copy keep both", the newly created 
	file will be renamed to a (2).txt, try again, it will
	be rename to a (3).txt
	FindFirstNumber:
	start: the index of (
	end: the index of )
	return value: true means there is a pair of () in file name
	which contains a number.

	GetPossiblePath:
	return the full file path for the new file copied by
	"copy, but keep both"

	Note: win XP doesn't have this feature.
	**************************************************/
	bool FindFirstNumber(const std::wstring& strFileName, std::wstring::size_type& start, std::wstring::size_type& end);
	std::wstring GetPossiblePath(LPCWSTR pszFilePath);

	/***************************************************
	copy c:\test\a.txt into c:\test,
	the new file will be named with "a - Copy.txt"
	both XP and windows 7 support this.
	*****************************************************/
	std::wstring GetPossiblePathWithSameFolder(LPCWSTR pszFilePath);
protected:
	CNxtMgr(void);
	~CNxtMgr(void);

	bool m_bInitialized;
};

class CNxtAeroPeek
{
private:
	CNxtAeroPeek();
	~CNxtAeroPeek();
	void DisableAeroPeek(bool bDisable=true);
	DWORD GetParentProcessID(DWORD dwProcessID,std::wstring& exePath);
public:
	static CNxtAeroPeek& GInstance();
	static unsigned __stdcall StartThread(PVOID lParam);
	void Load();
	void UnLoad();
public:
	HANDLE	m_hEvent ;
	HANDLE	m_hThread ;
	DWORD	m_dwProcessId;
	bool	m_bNeedCreateThread;
};