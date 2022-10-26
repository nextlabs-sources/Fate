#ifndef __FTPCLI_H__
#define __FTPCLI_H__
//#ifdef __cplusplus
//extern "C"{
//#endif
#ifndef _WIN64  
#define FTPCLI_OUTPUT_NAME	L"ftpcli32.dll"
#else
#define FTPCLI_OUTPUT_NAME	L"ftpcli.dll"
#endif
STDAPI FtpUpload(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile);
typedef  HRESULT (WINAPI *FPFtpUpload)(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile);


#define FTPCLI_RENAME_APPENDNUMBER	0x01 //There should be no existing same name file on FTP server, if there is then rename it by appending a number or random string.
#define FTPCLI_RENAME_RANDOMSTRING	0x02
STDAPI FtpUploadEx(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile,
						  DWORD dwFlag);
typedef  HRESULT (WINAPI *FPFtpUploadEx)(const WCHAR*pwzFtpServer,
						  const WCHAR*pwzFtpUser, 
						  const WCHAR*pwzFtpPasswd, 
						  int iPort,
						  const WCHAR*pwzLocalFile, 
						  WCHAR*pwzRemoteFile,
						  DWORD dwFlag);

STDAPI EFTFtpUpload(const WCHAR*pwzFtpServer,
				   const WCHAR*pwzFtpUser, 
				   const WCHAR*pwzFtpPasswd, 
				   int iPort,
				   const WCHAR*pwzLocalFile, 
				   WCHAR*pwzRemoteFile,
				   DWORD dwFlag,
				   const WCHAR* pwszExpDate,
				   const WCHAR* pwszFtpSite,
				   const WCHAR* pwszUserTemplate);
typedef  HRESULT (WINAPI *FuncEFTFtpUpload)(const WCHAR*pwzFtpServer,
										 const WCHAR*pwzFtpUser, 
										 const WCHAR*pwzFtpPasswd, 
										 int iPort,
										 const WCHAR*pwzLocalFile, 
										 WCHAR*pwzRemoteFile,
										 DWORD dwFlag,
										 const WCHAR* pwszExpDate,
										 const WCHAR* pwszFtpSite,
										 const WCHAR* pwszUserTemplate);

//#ifdef __cplusplus
//}
//#endif


#endif //__FTPCLI_H__