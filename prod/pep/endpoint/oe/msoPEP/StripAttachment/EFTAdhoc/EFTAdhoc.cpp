// EFTAdhoc.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "EFTAdhoc.h"
#include "rdPasswd.h"

#define DEFAULT_EFTAdhocSettingLevel	L"EFTAdhoc"

static void GetFileName(std::wstring &strFilePath, std::wstring &strFileName);

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			//CoInitialize(NULL);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			//CoUninitialize();
		}
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

static void GetExpTime(const WORD& nTime,WORD& wYear,WORD& wMonth,WORD& wDay)
{
	wYear = nTime / 365;
	wMonth = (nTime - wYear*365) / 30;
	wDay = nTime - wYear*365 - wMonth*30;
}
EFTADHOC_API HRESULT EFTAdhoc_CreateNewRandomUser(const wchar_t* strEFTServer, WORD wPort, 
												  const wchar_t* strSiteName, const wchar_t* strSiteSettingLevel,
												  const wchar_t* strAdminUser, const wchar_t* strAdminPass,
												  wchar_t* strNewUserName, wchar_t*strNewUserPass, 
												  WORD *pwPort, wchar_t*strErrorMsg,wchar_t* wszExpDate)
{
	HRESULT hr = S_OK;
	CComPtr<ICIServer> ptrEFTServer;
	BOOL bConnected = FALSE;

	hr = ptrEFTServer.CoCreateInstance(__uuidof(CIServer));
	if (SUCCEEDED(hr))
	{
		//fix a crash bug
		//use LoadLibrary to keep the SFTPCOMInterface.dll from being unloaded.
		//if SFTPCOMInterface.dll is unloaded, crash may happen. because after we finished upload, there is a thread remain running in SFTPCOMInterface.dll
		//so we can't unload SFTPCOMInterface.dll.
		static HMODULE hSFTPCOMInterfaceManualLoad = NULL;
		if (hSFTPCOMInterfaceManualLoad==NULL)
		{
			hSFTPCOMInterfaceManualLoad = LoadLibraryW(L"SFTPCOMInterface.dll");
			DP((L"LoadLibraryW SFTPCOMInterface.dll manually,hSFTPCOMInterface=0x%p.\n", hSFTPCOMInterfaceManualLoad));
		}
		
		_bstr_t bstrHost(strEFTServer);
		_bstr_t bstrLogin(strAdminUser);
		_bstr_t bstrPass(strAdminPass);

		DPW((L"EFTAdhoc_CreateNewRandomUser: Connect to EFTServer(%s:%d) with user %s\n", 
			strEFTServer, wPort, strAdminUser));

		try
		{
			hr = ptrEFTServer->Connect(bstrHost, wPort, bstrLogin, bstrPass);
			if (FAILED(hr))
			{
				DPW((L"EFTAdhoc_CreateNewRandomUser: Failed to connect to EFTServer!(hr=%d)\n", hr));
				wsprintfW(strErrorMsg,L"Failed to connect to EFT Server %s!",strEFTServer);
				return hr;
			}

			bConnected = TRUE;

			CComPtr<ICISites> ptrSites = ptrEFTServer->Sites();
			CComPtr<ICISite> ptrSite;
			if (ptrSites->Count() < 1)
			{
				strErrorMsg = L"No any site exists in EFT Server! Please configure the EFT Server first!";

				DPW((L"EFTAdhoc_CreateNewRandomUser: No site exists in EFT Server. Return!\n"));
				ptrEFTServer->Close();
				return E_FAIL;
			}

			if (wcslen(strSiteName) < 1)
			{
				ptrSite = ptrSites->Item(0);
			}
			else
			{
				for (int i = 0; i < ptrSites->Count(); i++)
				{
					if (_wcsicmp(strSiteName, (const wchar_t *)(ptrSites->Item(i)->Name)) == 0)
					{
						ptrSite = ptrSites->Item(i);
						break;
					}
				}
			}

			if (ptrSite == NULL)
			{
				wsprintfW(strErrorMsg,L"Site %s doesn't exist in EFT Server.",strSiteName);
				DPW((L"EFTAdhoc_CreateNewRandomUser: Site %s doesn't exist in EFT Server. Return!\n", strSiteName));
				ptrEFTServer->Close();
				return E_FAIL;
			}

			if (ptrSite->GetFTPAccess() == VARIANT_FALSE)
			{
				wsprintfW(strErrorMsg,L"Site %s doesn't support FTP access in EFT Server!",strSiteName);
				DPW((L"EFTAdhoc_CreateNewRandomUser: Site %s doesn't support FTP access in EFT Server. Return!\n", strSiteName));
				ptrEFTServer->Close();
				return E_FAIL;
			}

			std::wstring strSettingLevelName = strSiteSettingLevel;
			if (strSettingLevelName.empty())
			{
				strSettingLevelName = DEFAULT_EFTAdhocSettingLevel;
			}

			// Checking whether user setting level exists
			_variant_t varSettingLevels = ptrSite->GetSettingsLevels();

			LONG i = 0;
			for (i = varSettingLevels.parray->rgsabound[0].lLbound; i < (LONG)varSettingLevels.parray->rgsabound[0].cElements; i++)
			{
				VARIANT vLevel;
				SafeArrayGetElement(varSettingLevels.parray, (LONG *)&i, (void *)&vLevel);
				BSTR bstrLevel = vLevel.bstrVal;
				if (_wcsicmp(strSettingLevelName.c_str(), bstrLevel) == 0)
					break;
			}

			if (i == varSettingLevels.parray->cbElements)
			{
				wsprintfW(strErrorMsg,L"User setting level %s under site %s doesn't exist in EFT Server!",strSettingLevelName,strSiteName);
				DPW((L"EFTAdhoc_CreateNewRandomUser: User setting level %s under site %s doesn't exist in EFT Server. Return!\n", 
					strSettingLevelName, strSiteName));
				ptrEFTServer->Close();
				return E_FAIL;
			}

			CPasswdGenerator passGenerator;

			std::wstring szname = passGenerator.Generator(10);
			wcscpy_s(strNewUserName,32,szname.c_str());
			std::wstring szpwd = passGenerator.Generator(10);
			wcscpy_s(strNewUserPass,32,szpwd.c_str());

			_bstr_t bstrNewUser(szname.c_str());
			_bstr_t bstrNewPass(szpwd.c_str());
			_bstr_t bstrDesc(L"Temp user for uploading files in outlook Enforcer");
			_bstr_t bstrLevel(strSettingLevelName.c_str());

			hr = ptrSite->CreateUserEx(bstrNewUser, bstrNewPass, 0, bstrDesc, bstrNewUser, VARIANT_TRUE, VARIANT_TRUE, bstrLevel,abFalse);
			if (FAILED(hr))
			{
				DPW((L"EFTAdhoc_CreateNewRandomUser: Failed to create user %s with setting level %s under site %s. Return!\n", 
					strNewUserName, strSettingLevelName, strSiteName));
				ptrEFTServer->Close();
				return hr;
			}
			if(wszExpDate != NULL && wcslen(wszExpDate) > 0)
			{
				//CComVariant varTime(wszExpDate);
				SYSTEMTIME systime;
				GetSystemTime(&systime);
				WORD wYear=0,wMonth=0,wDay=0;
				WORD nTime = _wtoi(wszExpDate);
				GetExpTime(nTime,wYear,wMonth,wDay);
				systime.wYear += wYear;
				systime.wMonth += wMonth;

				systime.wDay += wDay;
				wMonth = systime.wDay / 30;
				systime.wDay = systime.wDay-wMonth*30;

				systime.wMonth += wMonth;
				wMonth = systime.wMonth / 12;
				systime.wMonth -= 12*wMonth;
				systime.wYear += wMonth;

				DPW((L"EFTAdhoc_CreateNewRandomUser: The expiration time is: %d/%d/%d.....\n", 
					systime.wMonth,systime.wDay,systime.wYear));

				DOUBLE vDouble=0;
				if(SystemTimeToVariantTime(&systime,&vDouble) !=0)
				{
					CComVariant theDate(vDouble,VT_DATE);
					if(SUCCEEDED(theDate.ChangeType(VT_DATE)))		
					{
						if(SUCCEEDED(ptrSite->GetUserSettings(bstrNewUser)->SetExpirationDate(&theDate,TRUE)))
						{
							DPW((L"EFTAdhoc_CreateNewRandomUser: Settting expiration time is: %d/%d/%d succeed.....\n", 
								systime.wMonth,systime.wDay,systime.wYear));
						}
					}
				}
			}

			if (pwPort != NULL)
				*pwPort = (WORD)ptrSite->GetPort();
		}
		catch(_com_error &e)
		{
			if (e.Description().length() != 0)
			{
				wsprintfW(strErrorMsg,L"%s",e.Description());
			}
			else
			{
				wsprintfW(strErrorMsg,L"Failed to create new user under site %s in the EFT Server!",strSiteName);
			}

			hr = e.Error();
			DPW((L"EFTAdhoc_CreateNewRandomUser: Failed Reason: %s\n", strErrorMsg));
		}

		if (bConnected)
			ptrEFTServer->Close();
	}
	else
	{
		wsprintfW(strErrorMsg,L"EFT Adhoc Module is not installed successfully!");
		DPW((L"EFTAdhoc_CreateNewRandomUser: CoCreateInstance ICIServer failed!(hr=%d)\n", hr));
	}

	return hr;
}

EFTADHOC_API HRESULT EFTAdhoc_UploadFile(const wchar_t* strLocalFile, EFTAdhocProtocolEnum protocol,
										 const wchar_t* strUser, const wchar_t* strPassword,
										 const wchar_t* strHost, const WORD wPort, 
										 const wchar_t* strRemotePath, wchar_t* strReturnFullPath,
										 wchar_t* strErrorMsg)
{
	HRESULT hr = S_OK;
	CComPtr<IClientFTPEngine> ptrClientFTPEngine;

	hr = ptrClientFTPEngine.CoCreateInstance(__uuidof(ClientFTPEngine));
	if (SUCCEEDED(hr))
	{
		_bstr_t bstrLocalPath(strLocalFile);
		_bstr_t bstrUser(strUser);
		_bstr_t bstrPassword(strPassword);
		_bstr_t bstrHost(strHost);
		_bstr_t bstrRemotePath(strRemotePath);

		DPW((L"EFTAdhoc_UploadFile: Upload file %s with user %s to host %s:%d/%s with protocol=%d \n", 
			strLocalFile, strUser, strHost, wPort, strRemotePath, protocol));

		try{
			hr = ptrClientFTPEngine->Upload(bstrLocalPath, (ProtocolEnum)protocol, 
				bstrUser, bstrPassword, bstrHost, wPort, bstrRemotePath);
		}
		catch (_com_error &e)
		{
			if (e.Description().length() != 0)
			{
				wcscpy_s(strErrorMsg,512,e.Description());
			}
			else
			{
				wsprintfW(strErrorMsg,L"Failed to upload file %s to FTP server %s with user %s ",strLocalFile,strHost,strUser);
			}
			hr = e.Error();
		}

		if (FAILED(hr))
		{
			DPW((L"EFTAdhoc_UploadFile: Failed to upload file %s!(hr=%d)(%s)\n", strLocalFile, hr, strErrorMsg));
		}
		else
		{
			if (protocol == EFTAdhocProtocolFTP)
			{
				wchar_t wzPort[36]; memset(wzPort, 0, sizeof(wzPort));
				_itow_s(wPort, wzPort, 35, 10);
				wsprintfW(strReturnFullPath,L"ftp://%s:%s@%s:%s%s",strUser,strPassword,strHost,wzPort,strRemotePath);
			}
		}
	}
	else
	{
		//strErrorMsg = L"FTP Module is not installed successfully!";
		wcscpy_s(strErrorMsg,512,L"FTP Module is not installed successfully!");
		DPW((L"EFTAdhoc_UploadFile: CoCreateInstance IClientFTPEngine failed!(hr=%d)\n", hr));
	}

	return hr;
}


static void GetFileName(std::wstring &strFilePath, std::wstring &strFileName)
{
	std::wstring::size_type lastPos = strFilePath.find_last_of(L"\\");
	if (lastPos >= 0)
	{
		strFileName = strFilePath.substr(lastPos+1);
	}
	else
	{
		strFileName = strFilePath;
	}
}
