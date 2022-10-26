// pa_encryption.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "pa_encryption.h"

#include <shlobj.h>
#include <shlwapi.h>

#include <activeds.h>
#include <Adshlp.h>
#include <comutil.h>

#include "log.h"
// #include "PasswordDlg.h"
// #include "AutoEncryptDlg.h"
#include "EncryptionWrapper.h"

#include "MsgDlg.h"

#include "UIInterface.h"
#include "GSControls.h"

using namespace pafUI;

#define MAX_PASSWORD_LENGTH	128

#ifdef _WIN64
#define UI_DLL_NAME		L"pafUI.dll"
#else
#define UI_DLL_NAME		L"pafUI32.dll"
#endif

typedef UI_STATUS (WINAPI *funcGetParentWindow)(HWND &_hWnd ,HWND hParent) ;
typedef HWND  (WINAPI *funcCreateMainFrame)( const HWND _hChildWnd,
											const BOOL _bIsModel, 
											const wchar_t* _pszHelpURL,
											const wchar_t* _strBtName,
											const BTSTATUS _BTStatu,
											const HWND _hParent,
											const wchar_t* _pszTitle, 
											const wchar_t* _pszDescription );
typedef UI_STATUS (WINAPI *funcChange_PA_Panel)(
												const HWND hParent,
												const HWND _hNewPanelWnd,
												const wchar_t* _pszHelpURL,
												const wchar_t* _pszTitle, 
												const wchar_t* _pszDescription);
typedef UI_STATUS (WINAPI *funcReleaseMainFrame)(const HWND hParent);
typedef UI_STATUS (WINAPI *funcSetNEXT_OKCallBack)( ONCLICKNEXTBT pFunc ,PVOID _pData,const HWND hParent);
typedef UI_STATUS (WINAPI *funcSetCancelCallBack)( ONCLICKCANCELBT pFunc,PVOID _pData,const HWND hParent);

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance = NULL;
WCHAR g_wzDllPath[MAX_PATH];

static HINSTANCE s_hUIDLL = NULL;

static funcGetParentWindow s_fGetParentWindow = NULL;
static funcCreateMainFrame s_fCreateMainFrame = NULL;
static funcChange_PA_Panel s_fChange_PA_Panel = NULL;
static funcReleaseMainFrame s_fReleaseMainFrame = NULL;
static funcSetNEXT_OKCallBack s_fSetNEXT_OKCallBack = NULL;
static funcSetCancelCallBack s_fSetCancelCallBack = NULL;


static PA_STATUS ParseObligationList(OBJECTINFO &file, EA_Obligation &obligation);
static LPWSTR GetBaseFileName(LPWSTR lpwzFileName);
static void CALLBACK OnClickDoActionButton( PVOID lpContxt, LONG status, HWND _hParent );
static void CALLBACK OnClickCancelButton(PVOID lpContxt, HWND _hParent);
static DWORD WINAPI EncryptionThread( LPVOID lpParam );
static void LogEncryptionAssistant(LPEA_AssistantData lpData, int status);

/***************************************
bool IsFolder(const wchar_t* pPath)
return true if the pPath is a folder
				Kevin Zhou 2009-1-20
*****************************************/
static bool IsFolder(const wchar_t* pPath )
{
	if(!pPath)
		return false;

	DWORD dw = ::GetFileAttributesW(pPath);

	if( dw & FILE_ATTRIBUTE_DIRECTORY )
		return true;
	else
		return false;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	UNUSED(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			WCHAR wzFullDLLPath[MAX_PATH];

			g_hInstance = (HINSTANCE)hModule;

			GetModuleFileName(hModule, g_wzDllPath, MAX_PATH);

			(_tcsrchr(g_wzDllPath,'\\'))[1] = 0;

			if (!s_hUIDLL)
			{
				_snwprintf_s(wzFullDLLPath, MAX_PATH, _TRUNCATE, L"%s\\%s", g_wzDllPath, UI_DLL_NAME);
				s_hUIDLL = LoadLibraryW(wzFullDLLPath);
				if (s_hUIDLL)
				{
					s_fGetParentWindow = (funcGetParentWindow)GetProcAddress(s_hUIDLL, "GetParentWindow");
					s_fCreateMainFrame = (funcCreateMainFrame)GetProcAddress(s_hUIDLL, "CreateMainFrame");
					s_fChange_PA_Panel = (funcChange_PA_Panel)GetProcAddress(s_hUIDLL, "Change_PA_Panel");
					s_fReleaseMainFrame = (funcReleaseMainFrame)GetProcAddress(s_hUIDLL, "ReleaseMainFrame");
					s_fSetNEXT_OKCallBack = (funcSetNEXT_OKCallBack)GetProcAddress(s_hUIDLL, "SetNEXT_OKCallBack");
					s_fSetCancelCallBack = (funcSetCancelCallBack)GetProcAddress(s_hUIDLL, "SetCancelCallBack");
				}
				else
				{
					DPW((L"Load library %s failed!(error=0x%x)\n", wzFullDLLPath, GetLastError()));
					return FALSE;
				}

				if (!s_fGetParentWindow || !s_fCreateMainFrame || !s_fChange_PA_Panel
					|| !s_fReleaseMainFrame || !s_fSetNEXT_OKCallBack || !s_fSetCancelCallBack)
				{
					DPW((L"Invalid library %s!(error=0x%x)\n", wzFullDLLPath, GetLastError()));
					FreeLibrary(s_hUIDLL);
					s_hUIDLL = NULL;
					return FALSE;
				}
			}

			LoadRegisteredAdapters();

			InitCommonControls();
			InitializeGSControls();
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			UninitializeGSControls();

			s_fGetParentWindow = NULL;
			s_fCreateMainFrame = NULL;
			s_fChange_PA_Panel = NULL;
			s_fReleaseMainFrame = NULL;
			s_fSetNEXT_OKCallBack = NULL;
			s_fSetCancelCallBack = NULL;

			if (s_hUIDLL)
			{
				FreeLibrary(s_hUIDLL);
				s_hUIDLL = NULL;
			}

			UnloadRegisteredAdapters();
			g_hInstance = NULL;
		}
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	DWORD 	dwRet	= S_OK;
	DWORD   dwDisposition	= 0;
	LONG    lResult			= 0;
	HKEY    hKeyNextlabs  = NULL;
	HKEY	hKeyEncryption	= NULL;
	WCHAR   wzKeyName[MAX_PATH+1];memset(wzKeyName, 0, sizeof(wzKeyName));
	char    szVal[MAX_PATH+1];    memset(szVal, 0, sizeof(szVal));

		_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs");
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, wzKeyName, 0, KEY_ALL_ACCESS, &hKeyNextlabs);
		if(ERROR_SUCCESS != lResult)     // get Nextlabs\Encryption key
		{
			DP((L"RegisterDll::Fail to open key: %s\n", wzKeyName));
			return E_UNEXPECTED;
		}

		lResult = RegCreateKeyEx( hKeyNextlabs, L"Encryption",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKeyEncryption,&dwDisposition);
		if(ERROR_SUCCESS != lResult)
		{
			DP((L"RegisterDll::Fail to open key: Encryption\n"));
		dwRet = (DWORD)E_UNEXPECTED;
	}

		if(NULL != hKeyEncryption) RegCloseKey(hKeyEncryption);
		if(NULL != hKeyNextlabs) RegCloseKey(hKeyNextlabs);

		hKeyNextlabs   = NULL;
		hKeyEncryption = NULL;

	return dwRet;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	WCHAR   wzKeyName[MAX_PATH];memset(wzKeyName, 0, sizeof(wzKeyName));

	_snwprintf_s(wzKeyName, MAX_PATH, _TRUNCATE, L"SOFTWARE\\Nextlabs\\Encryption");
	RegDeleteKey(HKEY_LOCAL_MACHINE, wzKeyName);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
PA_STATUS WINAPI DoPolicyAssistant( PA_PARAM &_iParam, const HWND _hParentWnd, bool forceObligation )
{
	PA_STATUS status = PA_SUCCESS;
	EA_AssistantData assistantData;

	DPW((L"DoPolicyAssistant for Encryption start...\n"));

	/* Parse all the parameters from PEP */
	assistantData.hPEPParantWnd = _hParentWnd;
	status = InitPAProps(_iParam, assistantData);
	if (status != PA_SUCCESS)
	{
		DPW((L"DoPolicyAssistant: InitPAProps failed!\n"));
		ReleasPA(assistantData);
		return status;
	}

	if (assistantData.symmAssistantData.listFiles.empty()
		&& assistantData.certAssistantData.listFiles.empty())
	{
		DPW((L"DoPolicyAssistant: No file need to be encrypted!\n"));
		ReleasPA(assistantData);
		return PA_SUCCESS;
	}

	/* Start to show dialogs */
	assistantData.dwNextPanelItem++;
	status = Begin_ShowPA(assistantData);
	if (status != PA_SUCCESS)
	{
		DPW((L"DoPolicyAssistant: Begin_ShowPA failed!\n"));
	}

	if (assistantData.bCanceled)
	{
		DPW((L"DoPolicyAssistant: User canceled!\n"));
		status = PA_ERROR;
	}

	LogEncryptionAssistant(&assistantData, status);

	ReleasPA(assistantData);

	DPW((L"DoPolicyAssistant for Encryption end(status=%d)...\n", status));

	return status;
}

//////////////////////////////////////////////////////////////////////////
PA_STATUS WINAPI InitPAProps ( PA_PARAM &param,
							EA_AssistantData &assistantData )
{
	PA_STATUS status = PA_SUCCESS;
	OBJECTINFOLIST::iterator iterObj;
	EA_Obligation obligation;

	assistantData.bLastPA = param._bIsLastPA;
	assistantData.wstrLastButtonName = param._strLastButtonName;
	assistantData.fLog = param.fLog;
	assistantData.lpLogCtx = param.lpLogCtx;

	if (param._action == AT_SENDMAIL)
	{
		assistantData.bMaintainFileNameAfterEncrypted = TRUE;
	}

	/* Parse PA Parameters */
	for (iterObj = param.objList.begin(); iterObj != param.objList.end(); iterObj++)
	{
		if ((*iterObj).lPARet)
		{
			continue;
		}

		status = ParseObligationList((*iterObj), obligation);
		if (status != PA_SUCCESS)
		{
			// Prompt error dialog
			WCHAR wzMessage[1024];
			WCHAR wzStringResource[512];

			LoadStringW(g_hInstance, IDS_CONFLICT_OBLIGATIONS, wzStringResource, 512);
			_snwprintf_s(wzMessage, 1024, _TRUNCATE, wzStringResource, (*iterObj).strDisplayName.c_str());
			EA_MessageBox(assistantData.hParantWnd, wzMessage);
			return status;
		}
		if( wcslen( (*iterObj).strTempName.c_str()) == 0 )
		{
			continue ;
		}
		EA_FileData data;
		data.wstrLogId = obligation.wstrLogId;
		data.wstrTmpDstFolder = (*iterObj).strTempName.c_str();

		CreateDirectoryW(data.wstrTmpDstFolder.c_str(), NULL);

		// Add Encryption sub-directory under the temp path
		if (!data.wstrTmpDstFolder.empty() && data.wstrTmpDstFolder[data.wstrTmpDstFolder.length()-1] != L'\\')
			data.wstrTmpDstFolder += L"\\";
		data.wstrTmpDstFolder += L"Encryption";
		CreateDirectoryW(data.wstrTmpDstFolder.c_str(), NULL);
		data.pOriginalObjectInfo = &(*iterObj);
		data.wstrFileDisplayName = (*iterObj).strDisplayName;
		data.wstrBaseFileName = GetBaseFileName((LPWSTR)(*iterObj).strDisplayName.c_str());

		if ((*iterObj).bFileNameChanged)
		{
			/*
				Modified by chellee on 14/10/08 ;6:32
				mark Code:data.wstrSrcFile = (*iterObj).strRetName.c_str();
			*/
			data.wstrSrcFile = (*iterObj).strRetName ;
			
		}
		else
		{
			data.wstrSrcFile = (*iterObj).strSrcName.c_str();
		}
		
		if(param._action == PABase::AT_MOVE && IsFolder(data.wstrSrcFile.c_str()))
		{
			DPW((L"This is a folder, ignore. %s\r\n", data.wstrSrcFile.c_str()));
			return PA_ERROR;
		}

		if (obligation.obType == EA_ObligationType_CertificateEncryptionAssistant)
		{
			if (assistantData.certAssistantData.listFiles.empty())
			{
				assistantData.certAssistantData.obligation.obType = obligation.obType;
				assistantData.certAssistantData.obligation.bOptional = obligation.bOptional;
				assistantData.certAssistantData.obligation.wstrDescription = obligation.wstrDescription;
				assistantData.certAssistantData.obligation.wstrEncryptAdapterName = obligation.wstrEncryptAdapterName;
				assistantData.certAssistantData.obligation.wstrLogId = obligation.wstrLogId;

				assistantData.certAssistantData.wstrSenderEmail = param.strSender.c_str();
				RECIPIENTLIST::iterator iterRecipient;
				for (iterRecipient = param.recipList.begin(); 
					iterRecipient != param.recipList.end(); iterRecipient++)
				{
					assistantData.certAssistantData.vecRecipients.push_back(*iterRecipient);
				}
				assistantData.certAssistantData.wstrPassword = L"";

				assistantData.dwTotalPanelItems++;
				assistantData.certAssistantData.dwItemIndex = assistantData.dwTotalPanelItems;
			}
			assistantData.certAssistantData.listFiles.push_back(data);
		}
		else if (obligation.obType == EA_ObligationType_SymmetricEncryptionAssistant)
		{
			if (assistantData.symmAssistantData.listFiles.empty())
			{
				assistantData.symmAssistantData.obligation.obType = obligation.obType;
				assistantData.symmAssistantData.obligation.bOptional = obligation.bOptional;
				assistantData.symmAssistantData.obligation.wstrDescription = obligation.wstrDescription;
				assistantData.symmAssistantData.obligation.wstrEncryptAdapterName = obligation.wstrEncryptAdapterName;
				assistantData.symmAssistantData.obligation.wstrLogId = obligation.wstrLogId;

				assistantData.dwTotalPanelItems++;
				assistantData.symmAssistantData.dwItemIndex = assistantData.dwTotalPanelItems;
			}
			assistantData.symmAssistantData.listFiles.push_back(data);
		}
		else
		{
			assistantData.listNot2DoFiles.push_back(data);
		}
	}
	

	return PA_SUCCESS;
}

PA_STATUS WINAPI Begin_ShowPA ( EA_AssistantData &assistantData )
{
	// TODO: Show Windows
	if (assistantData.dwNextPanelItem > 0 
		&& assistantData.dwNextPanelItem <= assistantData.dwTotalPanelItems)
	{
		if (assistantData.symmAssistantData.dwItemIndex == assistantData.dwNextPanelItem)
		{
			// Show Symmetric Encryption Dialog
// 			WCHAR wzResourcString[512];
// 			WCHAR wzDescription[512];

			if (assistantData.pSymmDlg)
			{
				delete(assistantData.pSymmDlg);
				assistantData.pSymmDlg = NULL;
			}
			assistantData.pSymmDlg = new CPasswordDlg(assistantData.symmAssistantData.obligation.bOptional);

// 			::LoadString(g_hInstance, IDS_SYM_DESC1, wzResourcString, 512);
// 			swprintf_s(wzDescription, 512, wzResourcString, L"Next");
			
// 			assistantData.pSymmDlg->set_Description1(std::wstring(wzDescription));
// 			assistantData.pSymmDlg->set_Description2(assistantData.symmAssistantData.obligation.wstrDescription);

			assistantData.pSymmDlg->set_Description1(assistantData.symmAssistantData.obligation.wstrDescription);

			std::list<EA_FileData>::iterator iterFile;
			for (iterFile = assistantData.symmAssistantData.listFiles.begin();
				iterFile != assistantData.symmAssistantData.listFiles.end(); iterFile++)
			{
				assistantData.pSymmDlg->put_FileName((*iterFile).wstrSrcFile, (*iterFile).wstrFileDisplayName);
			}

			if (assistantData.hParantWnd)
			{
				assistantData.pSymmDlg->Create(assistantData.hParantWnd);
			}
			else
			{
				HWND hParentWnd = NULL;
				s_fGetParentWindow(hParentWnd, assistantData.hPEPParantWnd);
				if(!hParentWnd)//added by kevin 2008-11-19
				{
					DP((L"Failed to create temp parent window in PAF, try to use the window of current process.(encryption PA)\r\n"));
					hParentWnd = assistantData.hPEPParantWnd;
				}
				assistantData.pSymmDlg->Create(hParentWnd);
				if(!assistantData.pSymmDlg->m_hWnd)
				{
					DP((L"Failed to create password dialog.\r\n"));
				}
				assistantData.hParantWnd = hParentWnd;
			}

			assistantData.pSymmDlg->ShowWindow(SW_HIDE);

			s_fSetNEXT_OKCallBack( OnClickDoActionButton, &assistantData, assistantData.hPEPParantWnd ) ;
			s_fSetCancelCallBack( OnClickCancelButton, &assistantData, assistantData.hPEPParantWnd);

			assistantData.hCurrentItemWnd = assistantData.pSymmDlg->m_hWnd;
			assistantData.currObType = EA_ObligationType_SymmetricEncryptionAssistant;

			BTSTATUS btStatus = BT_ENABLE;
// 			if (!assistantData.symmAssistantData.obligation.bOptional)
			{
				btStatus = BT_DISABLE;
			}

			if (assistantData.dwNextPanelItem > 1)
			{
				// The second dialog show
				UINT iMsg = 0;
				if (assistantData.bLastPA && assistantData.dwNextPanelItem == assistantData.dwTotalPanelItems)
				{
// 					swprintf_s(wzDescription, 512, wzResourcString, assistantData.wstrLastButtonName.c_str());
// 					assistantData.pSymmDlg->set_Description1(std::wstring(wzDescription));

					iMsg = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;
					PostMessage(assistantData.hParantWnd, iMsg, DS_OK, (LPARAM)assistantData.wstrLastButtonName.c_str()) ;
				}

				iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;
				PostMessage(assistantData.hParantWnd, iMsg, btStatus, 0) ;

				s_fChange_PA_Panel((HWND)&assistantData/*assistantData.hPEPParantWnd*/,assistantData.pSymmDlg->m_hWnd, NULL, NULL, NULL);
			}
			else
			{
				// Show the encryption dialog first time. It call s_fCreateMainFrame. 
				// s_fCreateMainFrame will call dialog DoModal.
				if (assistantData.bLastPA && assistantData.dwNextPanelItem == assistantData.dwTotalPanelItems)
				{
// 					swprintf_s(wzDescription, 512, wzResourcString, assistantData.wstrLastButtonName.c_str());
// 					assistantData.pSymmDlg->set_Description1(std::wstring(wzDescription));

					s_fCreateMainFrame(assistantData.pSymmDlg->m_hWnd, TRUE, NULL, 
						assistantData.wstrLastButtonName.c_str(), btStatus, 
						(HWND)&assistantData/*assistantData.hPEPParantWnd*/, NULL, NULL);
				}
				else
				{
					s_fCreateMainFrame(assistantData.pSymmDlg->m_hWnd, TRUE, NULL, 
						L"Next", btStatus,(HWND)&assistantData /*assistantData.hPEPParantWnd*/, NULL, NULL);
				}

				// Encryption is done or canceled or error happened.
// 				s_fReleaseMainFrame(assistantData.hPEPParantWnd);
				assistantData.hPEPParantWnd = NULL;
			}
		}
		else if (assistantData.certAssistantData.dwItemIndex == assistantData.dwNextPanelItem)
		{
// 			WCHAR wzResourcString[512];
// 			WCHAR wzDescription[512];

			// Show Certificate Encryption Dialog
			if (assistantData.pCertDlg)
			{
				delete(assistantData.pCertDlg);
				assistantData.pCertDlg = NULL;
			}
			assistantData.pCertDlg = new CAutoEncryptDlg(assistantData.certAssistantData.obligation.bOptional);

// 			::LoadString(g_hInstance, IDS_CERT_DESC2, wzResourcString, 512);
// 			assistantData.pCertDlg->set_Description2(std::wstring(wzResourcString));
// 
// 			::LoadString(g_hInstance, IDS_CERT_DESC1, wzResourcString, 512);
// 			swprintf_s(wzDescription, 512, wzResourcString, L"Next");
// 			assistantData.pCertDlg->set_Description1(std::wstring(wzDescription));

			assistantData.pCertDlg->set_Description1(assistantData.certAssistantData.obligation.wstrDescription);
			std::list<EA_FileData>::iterator iterFile;
			for (iterFile = assistantData.certAssistantData.listFiles.begin();
				iterFile != assistantData.certAssistantData.listFiles.end(); iterFile++)
			{
				assistantData.pCertDlg->put_FileName((*iterFile).wstrSrcFile, (*iterFile).wstrFileDisplayName);
			}

			if (assistantData.hParantWnd)
			{
				assistantData.pCertDlg->Create(assistantData.hParantWnd);
			}
			else
			{
				HWND hParentWnd = NULL;
				s_fGetParentWindow(hParentWnd,assistantData.hPEPParantWnd);
				assistantData.pCertDlg->Create(hParentWnd);
				assistantData.hParantWnd = hParentWnd;
			}

			assistantData.pCertDlg->ShowWindow(SW_HIDE);

			s_fSetNEXT_OKCallBack( OnClickDoActionButton, &assistantData,assistantData.hPEPParantWnd ) ;
			s_fSetCancelCallBack(OnClickCancelButton, &assistantData,assistantData.hPEPParantWnd );

			assistantData.hCurrentItemWnd = assistantData.pCertDlg->m_hWnd;
			assistantData.currObType = EA_ObligationType_CertificateEncryptionAssistant;

			if (assistantData.dwNextPanelItem > 1)
			{
				UINT iMsg = 0;
				if (assistantData.bLastPA && assistantData.dwNextPanelItem == assistantData.dwTotalPanelItems)
				{
// 					swprintf_s(wzDescription, 512, wzResourcString, assistantData.wstrLastButtonName.c_str());
// 					assistantData.pCertDlg->set_Description1(std::wstring(wzDescription));

					iMsg = RegisterWindowMessage( PAF_UI_MAINWINDOW_STATUS ) ;
					PostMessage(assistantData.hParantWnd, iMsg, DS_OK, (LPARAM)assistantData.wstrLastButtonName.c_str()) ;
				}

				iMsg = RegisterWindowMessage( PAF_UI_BTTONSATUS ) ;
				PostMessage(assistantData.hParantWnd, iMsg, BT_ENABLE, 0) ;
				/*
				Modified by chellee for change the flag of the list framewindow...on 24/12/2008
				s_fChange_PA_Panel(assistantData.hPEPParantWnd,assistantData.pCertDlg->m_hWnd, NULL, NULL, NULL);
				*/
				s_fChange_PA_Panel((HWND)&assistantData/*assistantData.hPEPParantWnd*/,assistantData.pCertDlg->m_hWnd, NULL, NULL, NULL);
			}
			else
			{
				// Show the encryption dialog first time. It call s_fCreateMainFrame. 
				// s_fCreateMainFrame will call dialog DoModal.
				if (assistantData.bLastPA && assistantData.dwNextPanelItem == assistantData.dwTotalPanelItems)
				{
// 					swprintf_s(wzDescription, 512, wzResourcString, assistantData.wstrLastButtonName.c_str());
// 					assistantData.pCertDlg->set_Description1(std::wstring(wzDescription));
					/*
					Modified by chellee for change the flag of the list framewindow...on 24/12/2008
					s_fCreateMainFrame(assistantData.pCertDlg->m_hWnd, TRUE, NULL, 
						assistantData.wstrLastButtonName.c_str(), BT_ENABLE, 
						assistantData.hPEPParantWnd, NULL, NULL);
					*/
					s_fCreateMainFrame(assistantData.pCertDlg->m_hWnd, TRUE, NULL, 
						assistantData.wstrLastButtonName.c_str(), BT_ENABLE, 
						(HWND)&assistantData/*assistantData.hPEPParantWnd*/, NULL, NULL);
				}
				else
				{
					/*
					Modified by chellee for change the flag of the list framewindow...on 24/12/2008
					s_fCreateMainFrame(assistantData.pCertDlg->m_hWnd, TRUE, NULL, 
						L"Next", BT_ENABLE, assistantData.hPEPParantWnd, NULL, NULL);
					*/
					s_fCreateMainFrame(assistantData.pCertDlg->m_hWnd, TRUE, NULL, 
						L"Next", BT_ENABLE, (HWND)&assistantData/*assistantData.hPEPParantWnd*/, NULL, NULL);
				}

				// Encryption is done or canceled or error happened.
// 				s_fReleaseMainFrame(assistantData.hPEPParantWnd);
				assistantData.hPEPParantWnd = NULL;
			}
		}
	}

	return PA_SUCCESS;
}

PA_STATUS WINAPI NextPAItem ( EA_AssistantData &assistantData, BOOL bOKClicked )
{
	BOOL bSkiped = FALSE;
	PA_STATUS status = 0;
	WCHAR wzStringResource[512];

	if (!bOKClicked)
	{
		// Cancel encrypt files
		return PA_ERROR;
	}

	/* Get Dialog Panel Content */
	if (assistantData.currObType == EA_ObligationType_SymmetricEncryptionAssistant)
	{
		WCHAR wzPassword1[MAX_PASSWORD_LENGTH*2];
		::GetWindowTextW(::GetDlgItem(assistantData.hCurrentItemWnd, IDC_PASSWORD1), wzPassword1, MAX_PASSWORD_LENGTH*2);
		assistantData.symmAssistantData.wstrPassword = wzPassword1;
		if (assistantData.symmAssistantData.wstrPassword.size() > MAX_PASSWORD_LENGTH-1)
		{
			LoadStringW(g_hInstance, IDS_PASSWD2LONG, wzStringResource, 512);
			EA_MessageBox(assistantData.hParantWnd, wzStringResource);
			return PA_ERROR;
		}
		bSkiped = !(::IsDlgButtonChecked(assistantData.hCurrentItemWnd, IDC_SYM_ENCRYPT));
		assistantData.symmAssistantData.bSkiped = bSkiped;
	}
	else if (assistantData.currObType == EA_ObligationType_CertificateEncryptionAssistant)
	{
		BOOL bEncrypt = ::IsDlgButtonChecked(assistantData.hCurrentItemWnd, IDC_ENCRYPT_CHECK);
		bSkiped = !bEncrypt;
		assistantData.certAssistantData.bSkiped = bSkiped;
	}
	else
	{
		bSkiped = TRUE;
	}

	if (bSkiped)
	{
		// TODO: Add Log
	}
	else
	{
		HANDLE hEncryptionThread = NULL;
		DWORD dwThreadId = 0;

		CEncryptProgressDlg2 dlg;

		LoadStringW(g_hInstance, IDS_PROGRESS_TITLE, wzStringResource, 512);
		dlg.set_Titile(wzStringResource);
		LoadStringW(g_hInstance, IDS_PROGRESS_DESCRIPTION, wzStringResource, 512);
		dlg.set_Description(wzStringResource);

		assistantData.pProgressDlg = &dlg;

		// Call EncryptionWrapper according to the current dialog type
		hEncryptionThread = CreateThread( 
				NULL,              // default security attributes
				0,                 // use default stack size  
				EncryptionThread,        // thread function 
				&(assistantData),             // argument to thread function 
				0,                 // use default creation flags 
				&dwThreadId);   // returns the thread identifier 
		if (!hEncryptionThread)
		{
			return PA_ERROR;
		}

		dlg.DoModal(assistantData.hParantWnd);

		WaitForSingleObject(hEncryptionThread, INFINITE);
		GetExitCodeThread(hEncryptionThread, (LPDWORD)&status);

		if (status != PA_SUCCESS)
		{
			// FAILED to do encryption
			assistantData.bCanceled = TRUE;
			assistantData.dwNextPanelItem = assistantData.dwTotalPanelItems;
			
			if (assistantData.wstrErrorMessage.empty())
			{
				EA_MessageBox(assistantData.hParantWnd, L"Application failed to encrypt the file");
			}
			else
			{
				EA_MessageBox(assistantData.hParantWnd, assistantData.wstrErrorMessage.c_str());
			}
		}
	}

	/* Re-initialize */
	assistantData.dwNextPanelItem++;
	assistantData.currObType = EA_ObligationType_None;
	if (assistantData.hCurrentItemWnd)
	{
		EndDialog(assistantData.hCurrentItemWnd, 0);
		assistantData.hCurrentItemWnd = NULL;
	}

	if (assistantData.dwNextPanelItem > assistantData.dwTotalPanelItems)
	{
		/* All dialog panels are handled */
		// fix bug 353
// 		ShowWindow(assistantData.hParantWnd, SW_HIDE);
// 		UpdateWindow(assistantData.hParantWnd);
		EndDialog(assistantData.hParantWnd, 0);
		assistantData.hParantWnd = NULL;
	}
	else
	{
		/* Show Next Dialog Panel */
		Begin_ShowPA(assistantData);
	}

	return PA_SUCCESS;
}

PA_STATUS WINAPI ReleasPA ( EA_AssistantData &assistantData )
{
	if (assistantData.hCurrentItemWnd)
	{
		EndDialog(assistantData.hCurrentItemWnd, 0);
		assistantData.hCurrentItemWnd = NULL;
	}

	if (assistantData.hParantWnd)
	{
		EndDialog(assistantData.hParantWnd, 0);
		assistantData.hParantWnd = NULL;
	}

	if (assistantData.pSymmDlg)
	{
		delete assistantData.pSymmDlg;
		assistantData.pSymmDlg = NULL;
	}

	if (assistantData.pCertDlg)
	{
		delete assistantData.pCertDlg;
		assistantData.pCertDlg = NULL;
	}

	assistantData.listNot2DoFiles.clear();
	assistantData.symmAssistantData.vecRecipients.clear();
	assistantData.symmAssistantData.listFiles.clear();

	assistantData.certAssistantData.vecRecipients.clear();
	assistantData.certAssistantData.listFiles.clear();
	
	return PA_SUCCESS;
}

static PA_STATUS ParseObligationList(OBJECTINFO &file, EA_Obligation &obligation)
{
	OBLIGATIONLIST::iterator iterObligation;
	POBLIGATION lpSymmEncryption = NULL;
	POBLIGATION lpCertificateEncryption = NULL;
	POBLIGATION lpOligation = NULL;

	for (iterObligation = file.obList.begin(); iterObligation != file.obList.end(); iterObligation++)
	{
		if (!_wcsicmp((*iterObligation).strOBName.c_str(), L"PASSWORD_BASED_ENCRYPTION"))
		{
			lpSymmEncryption = (POBLIGATION)&(*iterObligation);
		}

		if (!_wcsicmp((*iterObligation).strOBName.c_str(), L"IDENTITY_BASED_ENCRYPTION"))
		{
			lpCertificateEncryption = (POBLIGATION)&(*iterObligation);
		}
	}

	obligation.obType = EA_ObligationType_None;

	if (lpSymmEncryption && lpCertificateEncryption)
	{
		// TODO: Prompt error dialog
		DPW((L"Both have Symmetric & Certificate Encryption Assistant obligation for one file!\n"));
		return PA_ERROR;
	}

	if (lpSymmEncryption)
	{
		obligation.obType = EA_ObligationType_SymmetricEncryptionAssistant;
		lpOligation = lpSymmEncryption;
	}

	if (lpCertificateEncryption)
	{
		obligation.obType = EA_ObligationType_CertificateEncryptionAssistant;
		lpOligation = lpCertificateEncryption;
	}

	if (lpOligation)
	{
		ATTRIBUTELIST::iterator iterAttribute;

		DPW((L"Encryption Assistant: obligation=%s\n", lpOligation->strOBName.c_str()));

		for (iterAttribute = lpOligation->attrList.begin();
			iterAttribute != lpOligation->attrList.end(); iterAttribute++)
		{
			DPW((L"Encryption Assistant: Attribute=%s\n", (*iterAttribute).strValue.c_str()));

			if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"Encryption Adapter"))
			{
				iterAttribute++;
				DPW((L"Encryption Assistant: Attribute=%s\n", (*iterAttribute).strValue.c_str()));
				obligation.wstrEncryptAdapterName = (*iterAttribute).strValue.c_str();
			}

			if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"Description"))
			{
				iterAttribute++;
				DPW((L"Encryption Assistant: Attribute=%s\n", (*iterAttribute).strValue.c_str()));
				obligation.wstrDescription = (*iterAttribute).strValue.c_str();
			}

			if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"Optional"))
			{
				iterAttribute++;
				DPW((L"Encryption Assistant: Attribute=%s\n", (*iterAttribute).strValue.c_str()));
				if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"true"))
				{
					obligation.bOptional = TRUE;
				}
				else
				{
					obligation.bOptional = FALSE;
				}
			}

			if (!_wcsicmp((*iterAttribute).strValue.c_str(), L"LogId"))
			{
				iterAttribute++;
				DPW((L"Encryption Assistant: Attribute=%s\n", (*iterAttribute).strValue.c_str()));
				obligation.wstrLogId = (*iterAttribute).strValue.c_str();
			}
		}
	}

	return PA_SUCCESS;
}


static LPWSTR GetBaseFileName(LPWSTR lpwzFileName)
{
	LPWSTR lpwzBaseFileName = NULL;

	lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '\\');
	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)wcsrchr(lpwzFileName, '/');
	}

	if (!lpwzBaseFileName)
	{
		lpwzBaseFileName = (LPWSTR)lpwzFileName;
	}
	else
	{
		lpwzBaseFileName++;
	}

	return lpwzBaseFileName;
}

static void CALLBACK OnClickDoActionButton( PVOID lpContxt, LONG status, HWND _hParent )
{
	LPEA_AssistantData lpAssistant = (LPEA_AssistantData)lpContxt;
	UNUSED(status);

	lpAssistant->hParantWnd = _hParent;

	NextPAItem(*lpAssistant, TRUE);
}

static void CALLBACK OnClickCancelButton(PVOID lpContxt, HWND _hParent)
{
	LPEA_AssistantData lpAssistant = (LPEA_AssistantData)lpContxt;

	/* End all dialog handles */
	if (lpAssistant->hCurrentItemWnd)
	{
		lpAssistant->currObType = EA_ObligationType_None;
		EndDialog(lpAssistant->hCurrentItemWnd, 0);
		lpAssistant->hCurrentItemWnd = NULL;
		lpAssistant->dwNextPanelItem = 0;
	}

	lpAssistant->bCanceled = TRUE;

	lpAssistant->hParantWnd = NULL;
	EndDialog(_hParent, 0);
}

static DWORD WINAPI EncryptionThread( LPVOID lpParam )
{
	LPEA_AssistantData lpData = (LPEA_AssistantData)lpParam;
	PA_STATUS status = PA_SUCCESS;

	status = EncryptionWrapper(lpData);

	while (!::IsWindow(lpData->pProgressDlg->m_hWnd))
	{
		Sleep(30);
	}

	lpData->pProgressDlg->EndPrograssDlg();
	lpData->pProgressDlg = NULL;

	return status;
}

static void LogEncryptionAssistant(LPEA_AssistantData lpData, int status)
{
	std::wstring wstrAssistant = L"";
	std::wstring wstrAssistantDescription = L"";
	std::wstring wstrUserActions = L"";
	ATTRIBUTELIST listAttr;

	if (!lpData->fLog)
	{
		return;
	}

	if (!lpData->certAssistantData.vecRecipients.empty())
	{
		ATTRIBUTE attr;
		attr.strKey = L"Recipients";

		StringVector::iterator iterRec;
		for (iterRec = lpData->certAssistantData.vecRecipients.begin();
			iterRec != lpData->certAssistantData.vecRecipients.end(); iterRec++)
		{
			attr.strValue += (*iterRec).c_str();
			attr.strValue += L";";
		}

		listAttr.push_back(attr);
	}

	if (lpData->bCanceled)
	{
		wstrUserActions = L"User canceled encryption";
	}
	else if (status != PA_SUCCESS)
	{
		wstrUserActions = L"Encryption failed";
	}
	else
	{
		wstrUserActions = L"User encrypted file";
	}

	std::list<EA_FileData>::iterator iterFile;
	for (iterFile = lpData->symmAssistantData.listFiles.begin();
		iterFile != lpData->symmAssistantData.listFiles.end(); iterFile++)
	{
		wstrAssistantDescription = L"This obligation assistant performs symmetric encryption and compression of your valuable files";
		wstrAssistant = L"File Encryption Assistant";

		lpData->fLog(lpData->lpLogCtx, (*iterFile).wstrLogId, wstrAssistant,
			lpData->symmAssistantData.obligation.wstrEncryptAdapterName,
			wstrAssistantDescription, wstrUserActions, listAttr);
	}

	for (iterFile = lpData->certAssistantData.listFiles.begin();
		iterFile != lpData->certAssistantData.listFiles.end(); iterFile++)
	{
		wstrAssistantDescription = L"This obligation assistant performs asymmetric encryption of your valuable files";
		wstrAssistant = L"Certificate Encryption Assistant";

		lpData->fLog(lpData->lpLogCtx, (*iterFile).wstrLogId, wstrAssistant,
			lpData->certAssistantData.obligation.wstrEncryptAdapterName,
			wstrAssistantDescription, wstrUserActions, listAttr);
	}

}