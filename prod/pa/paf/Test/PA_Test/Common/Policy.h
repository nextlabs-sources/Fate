#ifndef __TEST_POLICY_H___
#define __TEST_POLICY_H___

#include "CEsdk.h"

typedef CEResult_t (WINAPIV* CECONN_InitializeType)(CEApplication app, CEUser user, CEString pdpHostName, CEHandle * connectHandle,CEint32 timeout_in_millisec); 
typedef CEResult_t (WINAPIV* CECONN_CloseType ) (CEHandle handle, CEint32 timeout_in_millisec);
typedef CEResult_t (WINAPIV* CEEVALUATE_CheckFileType)(CEHandle handle,CEAction_t operation, CEString sourceFullFileName, CEAttributes * sourceAttributes,CEString targetFullFileName,CEAttributes * targetAttributes,CEint32 ipNumber,	CEUser  *user,CEApplication *app,CEBoolean performObligation,CENoiseLevel_t noiseLevel,CEEnforcement_t * enforcement,CEint32 timeout_in_millisec);
class CPolicyComm
{
public :
	CPolicyComm() ;
	virtual  ~CPolicyComm() ;
public :

	VOID InitReference(VOID)  ;
	BOOL Connect2PolicyServer(VOID) ;
	VOID Disconnect2PolicyServer(VOID) ;

	BOOL VerifyFilePolicySingle(	LONG i_action ,
									wchar_t* i_pszFilePath,wchar_t* i_pszDestFilePath ,CEEnforcement_t &enforcement ) ;


	BOOL VerifyFilePolicySingle(	CEAction_t i_action, 
									CEString i_pszFilePath,
									CEAttributes *i_pSourceFileAttr ,
									CEString i_pszDesFilePath, 
									CEAttributes *i_pDesFileAttr,CEEnforcement_t &enforcement ) ;

	DWORD GetIP(VOID)  ; 
	VOID GetUserInfo( TCHAR *szSid, INT size, TCHAR *szUserName,INT iNameLen ) ;
protected:
private:
#ifndef MAX_USER_LEN	
#define	MAX_USER_LEN	128
#endif
	TCHAR m_szModulePath[MAX_PATH] ;
	TCHAR m_szModuleName[MAX_PATH] ;
	TCHAR m_szSid[MAX_USER_LEN] ;
	TCHAR m_szUserName[MAX_USER_LEN] ;
	TCHAR m_szHostName[MAX_USER_LEN] ;
	/*static*/ CEHandle m_connectHandle;
	/*static*/ ULONG    m_ulIp;
	HMODULE m_hConn ;
	HMODULE m_hEval ;
public:
	static PVOID m_pCECONN_Initialize ;
	static PVOID m_pCECONN_Close ;
	static PVOID m_pCEEVALUATE_CheckFile ;
};
#endif