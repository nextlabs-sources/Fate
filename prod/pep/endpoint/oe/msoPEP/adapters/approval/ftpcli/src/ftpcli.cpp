// ftpcli.cpp : Defines the entry point for the DLL application.
//

#pragma warning(push)
#pragma warning(disable: 6386)
#include "stdafx.h"
#pragma warning(pop)

#include "ftpcli.h"
#include <string>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ULONG CCommonLog::m_flag = 0;

 bool /*_stdcall*/ FtpUpload(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile)
{
	bool bRet=true;
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

	//FTPClient ftpCli(wzFtpServer,wzFtpUser,wzFtpPasswd,iPort);
	FTPClient ftpCli(pwzFtpServer,pwzFtpUser,pwzFtpPasswd,iPort);
	if(ftpCli.Connect()==false)
	{
		//DP((L"FTP Connection failed!"));
		return false;
	}
	//ftpCli.SetLocalFile(wstrLocalFile.c_str());
	//ftpCli.SetRemoteFile(wstrRemoteFile.c_str());
	ftpCli.SetLocalFile(pwzLocalFile);
	ftpCli.SetRemoteFile(pwzRemoteFile);
	bRet=ftpCli.Upload(pwzRemoteFile);
	if(bRet==true)
	{
		WCHAR* pCh=pwzRemoteFile;
		while(*pCh)
		{
			if(*pCh==L'\\')
				*pCh=L'/';
			pCh++;
		}
	}
	return bRet;
 }
// The one and only application object
//
//CWinApp theApp;
//
//using namespace std;
//
//int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
//{
//	int nRetCode = 0;
//
//	// initialize MFC and print and error on failure
//	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
//	{
//		// TODO: change error code to suit your needs
//		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
//		nRetCode = 1;
//	}
//	else
//	{
//		// TODO: code your application's behavior here.
//	}
//
//	return nRetCode;
//}
