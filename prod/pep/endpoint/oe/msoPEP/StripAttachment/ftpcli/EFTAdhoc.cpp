#include "StdAfx.h"

#include "EFTAdhoc.h"

#define OBLIGATION_EFTAdhocFileTransfer	L"EFTAdhocFileTransfer"

HINSTANCE EFTAdhoc::m_hEFTAdhoc = NULL;
FuncEFTAdhoc_CreateNewRandomUser EFTAdhoc::m_fEFTAdhoc_CreateNewRandomUser = NULL;
FuncEFTAdhoc_UploadFile EFTAdhoc::m_fEFTAdhoc_UploadFile = NULL;
CRITICAL_SECTION EFTAdhoc::m_cs;

#if 0
EFTAdhocObligation::EFTAdhocObligation(LPWSTR lpwzEFTServer, WORD wPort, LPWSTR lpwzAdminUser,
		LPWSTR lpwzAdminPass, LPWSTR lpwzSiteName, LPWSTR lpwzSettingTemplate)
		: m_strEFTServer(lpwzEFTServer), m_strAdminUser(lpwzAdminUser), m_strAdminPass(lpwzAdminPass),
		m_strSiteName(lpwzSiteName), m_strSettingTemplate(lpwzSettingTemplate)
{
	m_wAdminPort = wPort;
}

EFTAdhocObligation::EFTAdhocObligation(EFTAdhocObligation *pObligation)
{
	if (pObligation != NULL)
	{
		m_strEFTServer = pObligation->m_strEFTServer;
		m_wAdminPort = pObligation->m_wAdminPort;
		m_strAdminUser = pObligation->m_strAdminUser;
		m_strAdminPass = pObligation->m_strAdminPass;
		m_strSiteName = pObligation->m_strSiteName;
		m_strSettingTemplate = pObligation->m_strSettingTemplate;
	}
	else
	{
		m_wAdminPort = 21;
	}
}

EFTAdhoc::EFTAdhoc(EFTAdhocObligation *pObligation)
	: m_AdhocObligation(pObligation)
{
	m_wFTPPort = 21;
}
#endif

EFTAdhoc::~EFTAdhoc(void)
{
}
static bool GetOEInstallPath(wchar_t* in_value)
{
	HKEY hKey=NULL;
	LONG rstatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Nextlabs\\Compliant Enterprise\\Outlook Enforcer",0,KEY_QUERY_VALUE,&hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}
	DWORD result_size = MAX_PATH;
	rstatus = RegQueryValueExW(hKey,L"OutlookEnforcerDir",NULL,NULL,(LPBYTE)in_value,&result_size);
	RegCloseKey(hKey);
	if( rstatus != ERROR_SUCCESS )
	{
		return false;
	}
	return true;
}
BOOL EFTAdhoc::LoadDLL(LPCWSTR wzInstallPath)
{
	if(m_hEFTAdhoc != NULL)	return TRUE;
	WCHAR wzDllPath[MAX_PATH + 1];
	memset(wzDllPath, 0, sizeof(wzDllPath));

	if (wzInstallPath == NULL)
	{
		// get OE install path
		if(GetOEInstallPath(wzDllPath))
		{
			#ifndef _WIN64  
			wcscat_s(wzDllPath, MAX_PATH, L"bin\\EFTAdhoc32.dll");
			#else
			wcscat_s(wzDllPath, MAX_PATH, L"bin64\\EFTAdhoc.dll");
			#endif
		}
		else	return FALSE;
	} 
	else
	{
		wcscpy_s(wzDllPath, MAX_PATH, wzInstallPath);
		#ifndef _WIN64  
		wcscat_s(wzDllPath, MAX_PATH, L"bin\\EFTAdhoc32.dll");
		#else
		wcscat_s(wzDllPath, MAX_PATH, L"bin64\\EFTAdhoc.dll");
		#endif
	}

	DP((L"EFTAdhoc Load %s", wzDllPath));
	m_hEFTAdhoc = ::LoadLibraryW(wzDllPath);

	if (m_hEFTAdhoc != NULL)
	{
		m_fEFTAdhoc_CreateNewRandomUser = (FuncEFTAdhoc_CreateNewRandomUser)GetProcAddress(m_hEFTAdhoc, "EFTAdhoc_CreateNewRandomUser");
		m_fEFTAdhoc_UploadFile = (FuncEFTAdhoc_UploadFile)GetProcAddress(m_hEFTAdhoc, "EFTAdhoc_UploadFile");
	}

	InitializeCriticalSection(&m_cs);

	return (m_hEFTAdhoc != NULL);
}

BOOL EFTAdhoc::UnloadDLL(void)
{
	DeleteCriticalSection(&m_cs);

	if (m_hEFTAdhoc != NULL)
		FreeLibrary(m_hEFTAdhoc);

	m_hEFTAdhoc = NULL;
	m_fEFTAdhoc_CreateNewRandomUser = NULL;
	m_fEFTAdhoc_UploadFile = NULL;

	return TRUE;
}

BOOL EFTAdhoc::CreateUser(void)
{
	HRESULT hr = E_FAIL;
	std::wstring strErrorMsg = L"";

	if (m_fEFTAdhoc_CreateNewRandomUser != NULL)
	{
		wchar_t szUser[32]={0},szPwd[32]={0};
		wchar_t szError[512]={0};
		hr = m_fEFTAdhoc_CreateNewRandomUser(m_strEFTServer.c_str(), m_wAdminPort,
			m_strSiteName.c_str(), m_strSettingTemplate.c_str(), 
			m_strAdminUser.c_str(), m_strAdminPass.c_str(),
			szUser, szPwd, &m_wFTPPort,szError,m_wcsExpDate);
		m_strNewUser = szUser;
		m_strNewPassword = szPwd;
		strErrorMsg = szError;
	}
	else
	{
		DPW((L"m_fEFTAdhoc_CreateNewRandomUser == NULL\n"));
		strErrorMsg = L"EFT Adhoc Module is not installed successfully!";
	}

//	if (!strErrorMsg.empty())
//		MessageBoxW(NULL, strErrorMsg.c_str(), L"EFT Adhoc File Transfer", MB_ICONWARNING);

	return SUCCEEDED(hr);
}

void EFTAdhoc::SetLocalFile(LPWSTR lpwzLocalFile)
{
	if (lpwzLocalFile != NULL)
	{
		m_strLocalFile = lpwzLocalFile;

		// here we need to copy a temp file otherwise,outlook will remove the temp file after the email was send out.
		WCHAR tmp_path[MAX_PATH] = {0} ;

		DWORD dwRet = GetTempPath(_countof(tmp_path), tmp_path);

		WCHAR tmp_file[MAX_PATH]={0};                 
		GetTempFileName(tmp_path, L"OE_", 0, tmp_file);
		CopyFile(lpwzLocalFile,tmp_file,FALSE);
		DPW((L"Copy File from [%s] to [%s] , ret = [%d].............\n",lpwzLocalFile,tmp_file,GetLastError()));
		
		std::wstring::size_type lastPos = m_strLocalFile.find_last_of(L"\\");
		std::wstring strFileName = L"";
		if (lastPos >= 0)
		{
			strFileName = m_strLocalFile.substr(lastPos+1);
		}
		else
		{
			strFileName = m_strLocalFile;
		}

		m_strRemotePath = L"/";
		m_strRemotePath += strFileName;

		// save the tmp file
		m_strLocalFile = tmp_file;
	}
}

void EFTAdhoc::GetFullRemotePath(std::wstring &strFullPath)
{
	if (!(m_strNewUser.empty() || m_strNewPassword.empty() || m_strRemotePath.empty()))
	{
		strFullPath = L"ftp://";
		strFullPath += m_strNewUser;
		strFullPath += L":";
		strFullPath += m_strNewPassword;
		strFullPath += L"@";
		strFullPath += m_strEFTServer;
		strFullPath += L":";
		wchar_t wzPort[36]; memset(wzPort, 0, sizeof(wzPort));
		_itow_s(m_wFTPPort, wzPort, 35, 10);
		strFullPath += wzPort;
		strFullPath += m_strRemotePath;
	}
}

BOOL EFTAdhoc::UploadFile(void)
{
	HRESULT hr = E_FAIL;
	std::wstring strErrorMsg = L"";

	if (m_fEFTAdhoc_UploadFile != NULL)
	{
		wchar_t szFullPath[1024]={0};
		wchar_t szErrorMsg[512]={0};

		EnterCriticalSection(&m_cs);
		hr = m_fEFTAdhoc_UploadFile(m_strLocalFile.c_str(), 0, m_strNewUser.c_str(), m_strNewPassword.c_str(), 
			m_strEFTServer.c_str(), m_wFTPPort, m_strRemotePath.c_str(), szFullPath, szErrorMsg);
		strErrorMsg = szErrorMsg;
		LeaveCriticalSection(&m_cs);
	}
	else
	{
		DPW((L"m_fEFTAdhoc_UploadFile == NULL\n"));
		strErrorMsg = L"EFT Adhoc Module is not installed successfully!";
	}

//	if (!strErrorMsg.empty())
//		MessageBoxW(NULL, strErrorMsg.c_str(), L"EFT Adhoc File Transfer", MB_ICONWARNING);

	DeleteFile(m_strLocalFile.c_str());
	return SUCCEEDED(hr);
}

#if 0
EFTAdhocObligation *EFTAdhocParseObligation(LPVOID lpObligation)
{
	CObligations *pObligation = (CObligations *)lpObligation;
	std::wstring strEFTServer = L"";
	WORD wAdminPort = 21;
	std::wstring strAdminUser = L"";
	std::wstring strAdminPass = L"";
	std::wstring strSiteName = L"";
	std::wstring strSettingTemplate = L"";
	BOOL bFound = FALSE;
	EFTAdhocObligation *pNewObligation = NULL;

	for(int j=0; j<pObligation->m_Obligations.size(); j++)
	{
		std::wstring wstrKey;
		std::wstring wstrValue;
		if(0 == _wcsnicmp(pObligation->m_Obligations[j]->name.c_str(), OBLIGATION_EFTAdhocFileTransfer, wcslen(OBLIGATION_EFTAdhocFileTransfer)))
		{
			for(int k=0; k<(int)pObligation->m_Obligations[j]->values.size()-1; k++)
			{
				wstrKey = pObligation->m_Obligations[j]->values[k++];
				wstrValue = pObligation->m_Obligations[j]->values[k];
				if (_wcsicmp(wstrKey.c_str(), L"EFT Server") == 0)
					strEFTServer = wstrValue;
				else if (_wcsicmp(wstrKey.c_str(), L"EFT Admin Port") == 0)
					wAdminPort = _wtoi(wstrValue.c_str());
				else if (_wcsicmp(wstrKey.c_str(), L"Admin User") == 0)
					strAdminUser = wstrValue;
				else if (_wcsicmp(wstrKey.c_str(), L"Admin Password") == 0)
					strAdminPass = wstrValue;
				else if (_wcsicmp(wstrKey.c_str(), L"EFT Site Name") == 0)
					strSiteName = wstrValue;
				else if (_wcsicmp(wstrKey.c_str(), L"User Settings Template") == 0)
					strSettingTemplate = wstrValue;
			}
			bFound = TRUE;
			break;
		}
	}

	if (bFound)
	{
		DPW((L"Get %s obligaton: EFTServer=%s AdminPort=%d, AdminUser=%s, SiteName=%s, SettingTemplate=%s",
			OBLIGATION_EFTAdhocFileTransfer, 
			strEFTServer.c_str(), wAdminPort, strAdminUser.c_str(), 
			strSiteName.c_str(), strSettingTemplate.c_str()));
		pNewObligation = new EFTAdhocObligation((LPWSTR)strEFTServer.c_str(), wAdminPort, 
			(LPWSTR)strAdminUser.c_str(), (LPWSTR)strAdminPass.c_str(), 
			(LPWSTR)strSiteName.c_str(), (LPWSTR)strSettingTemplate.c_str());
	}

	return pNewObligation;
}
#endif
DWORD WINAPI UploadFileThread( LPVOID lpParam )
{
	//NotesInitThread();

	EFTAdhoc *pEFTAdhoc = (EFTAdhoc *)lpParam;
	
	DPA(("Upload File Thread Start...\n"));

	CoInitialize(NULL);

	//DPA(("After CoInitialize...\n"));

	BOOL bRet = FALSE;
	if (pEFTAdhoc != NULL)
	{
		bRet = pEFTAdhoc->UploadFile();
		delete pEFTAdhoc;
	}

	DPA(("Upload File Thread Return...\n"));

	CoUninitialize();

	//NotesTermThread();

	return 0;
}

DWORD WINAPI RecycleTmpFilesThread( LPVOID lpParam )
{
	//NotesInitThread();

	RecycleParam *pParam = (RecycleParam *)lpParam;

	DPA(("RecycleTmpFilesThread Start...\n"));

	CoInitialize(NULL);

	if (pParam != NULL)
	{
		size_t iCount = pParam->m_hThreads.size();
		if (iCount > 0)
		{
			HANDLE *hThreads = new HANDLE[iCount];
			std::vector<HANDLE>::iterator iter;
			size_t i = 0;
			for (iter = pParam->m_hThreads.begin(); iter != pParam->m_hThreads.end() && i < iCount; i++, iter++)
			{
				hThreads[i] = (*iter);
			}

			// Wait for all upload file thread to complete.
			WaitForMultipleObjects((DWORD)iCount, hThreads, TRUE, INFINITE);
			for (i = 0; i < iCount; i++)
			{
				CloseHandle(hThreads[i]);
			}

			delete [] hThreads;
		}

		pParam->m_wzTempFolder[wcslen(pParam->m_wzTempFolder)+1] = '\0';
		SHFILEOPSTRUCTW opShFile;
		opShFile.hwnd = NULL;
		opShFile.wFunc = FO_DELETE;
		opShFile.pFrom = pParam->m_wzTempFolder;
		opShFile.pTo = NULL;
		opShFile.fFlags = (FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR);
		opShFile.fAnyOperationsAborted = 0;
		opShFile.hNameMappings = NULL;
		opShFile.lpszProgressTitle = NULL;

		int iRet = SHFileOperationW(&opShFile);
		if (iRet != 0)
		{
			DWORD dwErr = GetLastError();
			DPW((L"Delete folder %s failed: %x", pParam->m_wzTempFolder, dwErr));
		}

		delete pParam;
	}

	DPA(("RecycleTmpFilesThread End...\n"));

	CoUninitialize();

	//NotesTermThread();

	return 0;
}