#pragma once
#include <string>
using std::wstring;

typedef HRESULT (*FuncEFTAdhoc_CreateNewRandomUser)(const wchar_t* strEFTServer, WORD wPort, 
												  const wchar_t* strSiteName, const wchar_t* strSiteSettingLevel,
												  const wchar_t* strAdminUser, const wchar_t* strAdminPass,
												  wchar_t* strNewUserName, wchar_t*strNewUserPass, 
												  WORD *pwPort, wchar_t*strErrorMsg,wchar_t* wszExpDate);

typedef HRESULT (*FuncEFTAdhoc_UploadFile)(const wchar_t* strLocalFile, int protocol,
										 const wchar_t* strUser, const wchar_t* strPassword,
										 const wchar_t* strHost, const WORD wPort, 
										 const wchar_t* strRemotePath, wchar_t* strReturnFullPath,
										 wchar_t* strErrorMsg);

#if 0
class EFTAdhocObligation
{
public:
	EFTAdhocObligation(LPWSTR lpwzEFTServer, WORD wPort, LPWSTR lpwzAdminUser,
		LPWSTR lpwzAdminPass, LPWSTR lpwzSiteName, LPWSTR lpwzSettingTemplate);

	EFTAdhocObligation(EFTAdhocObligation *pObligation);

	~EFTAdhocObligation(void) {}

public:
	std::wstring m_strEFTServer;
	WORD m_wAdminPort;
	std::wstring m_strAdminUser;
	std::wstring m_strAdminPass;
	std::wstring m_strSiteName;
	std::wstring m_strSettingTemplate;
};
#endif
class EFTAdhoc
{
private:
	static HINSTANCE m_hEFTAdhoc;
	static FuncEFTAdhoc_CreateNewRandomUser m_fEFTAdhoc_CreateNewRandomUser;
	static FuncEFTAdhoc_UploadFile m_fEFTAdhoc_UploadFile;
	static CRITICAL_SECTION m_cs;

public:
	//EFTAdhoc(EFTAdhocObligation *pObligation);
	EFTAdhoc(const wchar_t* strsrv,const wchar_t* stradmin,const wchar_t* strpwd,WORD dwPort,const wchar_t* strsite,
		const wchar_t* strtemplate,const wchar_t* wszExpDate)
		:m_strEFTServer(strsrv),m_strAdminUser(stradmin),m_strAdminPass(strpwd),m_wAdminPort(dwPort),
		m_strSiteName(strsite),m_strSettingTemplate(strtemplate)
	{
		m_wcsExpDate[0]=L'\0';
		if(wszExpDate != NULL && wcslen(wszExpDate) > 0)
		{
			wcscpy_s(m_wcsExpDate,MAX_PATH,wszExpDate);
		}
	}
public:
	~EFTAdhoc(void);

	static BOOL LoadDLL(LPCWSTR wzInstallPath);
	static BOOL UnloadDLL(void);

	BOOL CreateUser(void);
	void SetLocalFile(LPWSTR lpwzLocalFile);
	void GetFullRemotePath(std::wstring &strFullPath);

	BOOL UploadFile(void);

private:
	//EFTAdhocObligation m_AdhocObligation;

	std::wstring m_strNewUser;
	std::wstring m_strNewPassword;
	WORD m_wFTPPort;

	std::wstring m_strLocalFile;
	std::wstring m_strRemotePath;

	std::wstring m_strEFTServer;
	WORD m_wAdminPort;
	std::wstring m_strAdminUser;
	std::wstring m_strAdminPass;
	std::wstring m_strSiteName;
	std::wstring m_strSettingTemplate;
	wchar_t m_wcsExpDate[64];
};

class RecycleParam
{
public:
	RecycleParam() { memset(m_wzTempFolder, 0, sizeof(m_wzTempFolder)); }
	~RecycleParam() {}

public:
	std::vector<EFTAdhoc *> m_pEFTAdhocs;
	std::vector<HANDLE> m_hThreads;
	WCHAR m_wzTempFolder[MAX_PATH + 2];
};

DWORD WINAPI UploadFileThread( LPVOID lpParam );

DWORD WINAPI RecycleTmpFilesThread( LPVOID lpParam );

//EFTAdhocObligation *EFTAdhocParseObligation(LPVOID lpObligation);
