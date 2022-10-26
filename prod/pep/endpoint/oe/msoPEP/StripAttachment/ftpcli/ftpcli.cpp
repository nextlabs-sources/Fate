// ftpcli.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ftpcli.h"
#include <string>
#include "celog.h"
#include "celog_policy_windbg.hpp"
#include "celog_policy_file.hpp"
#include "eframework/auto_disable/auto_disable.hpp"
nextlabs::recursion_control mso_hook_control;

#include "EFTAdhoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//HINSTANCE g_hInstance;
//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//					 )
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//		{
//			g_hInstance=hModule;
//		}
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//    return TRUE;
//}

STDAPI FtpUpload(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile)
{
	
	/*std::wstring wstrFtpServer=pwzFtpServer;
	std::wstring wstrFtpUser=pwzFtpUser;
	std::wstring wstrFtpPasswd=pwzFtpPasswd;
	std::wstring wstrLocalFile=pwzLocalFile;
	std::wstring wstrRemoteFile=pwzRemoteFile;
	WCHAR wzFtpServer[256]=L"";
	WCHAR wzFtpUser[256]=L"";
	WCHAR wzFtpPasswd[256]=L"";
	WCHAR wzLocalFile[256]=L"";
	WCHAR wzRemoteFile[256]=L"";
	wcsncpy_s(wzFtpServer,_countof(wzFtpServer),pwzFtpServer,_countof(wzFtpServer));
	wcsncpy_s(wzFtpUser,_countof(wzFtpUser),pwzFtpUser,_countof(wzFtpUser));
	wcsncpy_s(wzFtpPasswd,_countof(wzFtpPasswd),pwzFtpPasswd,_countof(wzFtpPasswd));
	wcsncpy_s(wzLocalFile,_countof(wzLocalFile),pwzLocalFile,_countof(wzLocalFile));
	wcsncpy_s(wzRemoteFile,_countof(wzRemoteFile),pwzRemoteFile,_countof(wzRemoteFile));*/
	OutputDebugString(L"FtpUpload begin");
	//FTPClient ftpCli(wzFtpServer,wzFtpUser,wzFtpPasswd,iPort);
	FTPClient ftpCli(pwzFtpServer,pwzFtpUser,pwzFtpPasswd,iPort);
	if(ftpCli.Connect()==false)
	{
		//DP((L"FTP Connection failed!"));
		return S_FALSE;
	}
	//ftpCli.SetLocalFile(wstrLocalFile.c_str());
	//ftpCli.SetRemoteFile(wstrRemoteFile.c_str());
	ftpCli.SetLocalFile(pwzLocalFile);
	ftpCli.SetRemoteFile(pwzRemoteFile);
	bool bRet=ftpCli.Upload(pwzRemoteFile);
	if(bRet==false)
		return S_FALSE;
	return S_OK;
}

STDAPI FtpUploadEx(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile,
						  DWORD dwFlag)
{
	
	/*std::wstring wstrFtpServer=pwzFtpServer;
	std::wstring wstrFtpUser=pwzFtpUser;
	std::wstring wstrFtpPasswd=pwzFtpPasswd;
	std::wstring wstrLocalFile=pwzLocalFile;
	std::wstring wstrRemoteFile=pwzRemoteFile;
	WCHAR wzFtpServer[256]=L"";
	WCHAR wzFtpUser[256]=L"";
	WCHAR wzFtpPasswd[256]=L"";
	WCHAR wzLocalFile[256]=L"";
	WCHAR wzRemoteFile[256]=L"";
	wcsncpy_s(wzFtpServer,_countof(wzFtpServer),pwzFtpServer,_countof(wzFtpServer));
	wcsncpy_s(wzFtpUser,_countof(wzFtpUser),pwzFtpUser,_countof(wzFtpUser));
	wcsncpy_s(wzFtpPasswd,_countof(wzFtpPasswd),pwzFtpPasswd,_countof(wzFtpPasswd));
	wcsncpy_s(wzLocalFile,_countof(wzLocalFile),pwzLocalFile,_countof(wzLocalFile));
	wcsncpy_s(wzRemoteFile,_countof(wzRemoteFile),pwzRemoteFile,_countof(wzRemoteFile));*/
	OutputDebugString(L"FtpUpload begin");
	//FTPClient ftpCli(wzFtpServer,wzFtpUser,wzFtpPasswd,iPort);
	FTPClient ftpCli(pwzFtpServer,pwzFtpUser,pwzFtpPasswd,iPort);
	ftpCli.SetFlag(dwFlag);
	if(ftpCli.Connect()==false)
	{
		//DP((L"FTP Connection failed!"));
		return S_FALSE;
	}
	//ftpCli.SetLocalFile(wstrLocalFile.c_str());
	//ftpCli.SetRemoteFile(wstrRemoteFile.c_str());
	ftpCli.SetLocalFile(pwzLocalFile);
	ftpCli.SetRemoteFile(pwzRemoteFile);
	bool bRet=ftpCli.Upload(pwzRemoteFile);
	if(bRet==false) 
		return S_FALSE;
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////
//	For Tyco , transfer file to EFT Ftp Server (GlobalScope)

STDAPI EFTFtpUpload(const WCHAR*pwzFtpServer,
					const WCHAR*pwzFtpUser, 
					const WCHAR*pwzFtpPasswd, 
					int iPort,
					const WCHAR*pwzLocalFile, 
					WCHAR*pwzRemoteFile,
					DWORD dwFlag,
					const WCHAR* pwszExpDate,
					const WCHAR* pwszFtpSite,
					const WCHAR* pwszUserTemplate)
{
	HRESULT hr= E_FAIL;
	OutputDebugStringW(L"Enter to EFTFtpUpload module.....................\n");
	if(!EFTAdhoc::LoadDLL(NULL))	
	{
		return hr;
	}
	EFTAdhoc *pEFTAdhoc = new EFTAdhoc(pwzFtpServer,pwzFtpUser,pwzFtpPasswd,
		iPort,pwszFtpSite,pwszUserTemplate,pwszExpDate);
	if(pEFTAdhoc == NULL)	return hr;
	
	BOOL bRet = pEFTAdhoc->CreateUser();
	if (bRet)
	{
		pEFTAdhoc->SetLocalFile((LPWSTR)pwzLocalFile);
		wstring strFtpPath;
		pEFTAdhoc->GetFullRemotePath(strFtpPath);
		if(!strFtpPath.empty())
		{
			wcscpy_s(pwzRemoteFile,1024,strFtpPath.c_str());
		}
		//bRet = theEFTAdhoc.UploadFile();
		CreateThread(NULL, 0, UploadFileThread, (LPVOID)pEFTAdhoc, 0, NULL);
		hr = S_OK;
	}
	else
	{
		delete pEFTAdhoc;
	}
	return hr;
}