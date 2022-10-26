// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EFTADHOC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EFTADHOC_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EFTADHOC_EXPORTS
#define EFTADHOC_API __declspec(dllexport)
#else
#define EFTADHOC_API __declspec(dllimport)
#endif

typedef enum {
        EFTAdhocProtocolFTP = 0,
        EFTAdhocProtocolFTPS_IMPLICIT = 1,
        EFTAdhocProtocolFTPS_EXPLICIT = 2,
        EFTAdhocProtocolSFTP2 = 3,
        EFTAdhocProtocolHTTP = 4,
        EFTAdhocProtocolHTTPS = 5,
        EFTAdhocProtocolSOCKS4 = 6,
        EFTAdhocProtocolSOCKS5 = 7,
        EFTAdhocProtocolFTPS_AUTH_TLS = 8
    } EFTAdhocProtocolEnum;

#if 0
EFTADHOC_API HRESULT EFTAdhoc_CreateNewRandomUser(std::wstring &strEFTServer, WORD wPort, 
												  std::wstring &strSiteName, std::wstring &strSiteSettingLevel,
											  std::wstring &strAdminUser, std::wstring &strAdminPass,
											  std::wstring &strNewUserName, std::wstring &strNewUserPass, 
											  WORD *pwPort, std::wstring &strErrorMsg,wchar_t* wszExpDate=NULL);


EFTADHOC_API HRESULT EFTAdhoc_UploadFile(std::wstring &strLocalFile, EFTAdhocProtocolEnum protocol,
										 std::wstring &strUser, std::wstring &strPassword,
										 std::wstring &strHost, WORD wPort, 
										 std::wstring &strRemotePath, std::wstring &strReturnFullPath,
										 std::wstring &strErrorMsg);
#endif
EFTADHOC_API HRESULT EFTAdhoc_CreateNewRandomUser(const wchar_t* strEFTServer, WORD wPort, 
												  const wchar_t* strSiteName, const wchar_t* strSiteSettingLevel,
												  const wchar_t* strAdminUser, const wchar_t* strAdminPass,
												  wchar_t* strNewUserName, wchar_t*strNewUserPass, 
												  WORD *pwPort, wchar_t*strErrorMsg,wchar_t* wszExpDate);
EFTADHOC_API HRESULT EFTAdhoc_UploadFile(const wchar_t* strLocalFile, EFTAdhocProtocolEnum protocol,
										 const wchar_t* strUser, const wchar_t* strPassword,
										 const wchar_t* strHost, const WORD wPort, 
										 const wchar_t* strRemotePath, wchar_t* strReturnFullPath,
										 wchar_t* strErrorMsg);
