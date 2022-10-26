#pragma once

#include "stdafx.h"

#include "Policy.h"
#include "Sddl.h"
#include "Winsock2.h"
#include "FileAttribute.hpp"
#include "..\FileManger.h"
//#pragma comment(lib, "cecem.lib")
//#pragma comment(lib, "ceconn.lib")
//#pragma comment(lib, "ceeval.lib")

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "ws2_32.lib")

#ifndef _MODULEFILENAME
#define _MODULEFILENAME L"EXCEL"
#endif 

PVOID CPolicyComm::m_pCECONN_Initialize = NULL ;
PVOID CPolicyComm::m_pCECONN_Close = NULL ;
PVOID CPolicyComm::m_pCEEVALUATE_CheckFile = NULL ;
CPolicyComm::CPolicyComm()
{
	ZeroMemory( m_szModuleName,MAX_PATH*sizeof(TCHAR ) ) ;
	memcpy( m_szModuleName,_MODULEFILENAME,wcslen( _MODULEFILENAME ) *sizeof(TCHAR) ) ;
	ZeroMemory( m_szModulePath,MAX_PATH*sizeof(TCHAR) ) ;
	GetModuleFileName( NULL,m_szModulePath,MAX_PATH ) ;
	ZeroMemory( m_szSid,MAX_USER_LEN*sizeof(TCHAR) ) ;
	ZeroMemory( m_szUserName,MAX_USER_LEN*sizeof(TCHAR) ) ;
	ZeroMemory( m_szHostName,MAX_USER_LEN*sizeof(TCHAR) ) ;
	m_connectHandle = NULL ;
	m_ulIp = 0 ;
	this->m_hConn = NULL ;
	this->m_hEval = NULL ;
	m_hConn = ::LoadLibrary( L"ceconn.dll" ) ; 
	m_hEval = ::LoadLibrary( L"ceeval.dll" ) ;
	if( m_hConn )
	{
		m_pCECONN_Initialize = (PVOID)::GetProcAddress( m_hConn, "CECONN_Initialize" ) ;
		m_pCECONN_Close = (PVOID)::GetProcAddress( m_hConn, "CECONN_Close" ) ;
	}
	if( m_hEval )
	{
		m_pCEEVALUATE_CheckFile = (PVOID)::GetProcAddress( m_hEval, "CEEVALUATE_CheckFile" ) ; 
	}
}

CPolicyComm::~CPolicyComm() 
{
	ZeroMemory( m_szModuleName,MAX_PATH*sizeof(TCHAR ) ) ;
	ZeroMemory( m_szModulePath,MAX_PATH*sizeof(TCHAR) ) ;
	ZeroMemory( m_szSid,MAX_USER_LEN*sizeof(TCHAR) ) ;
	ZeroMemory( m_szUserName,MAX_USER_LEN*sizeof(TCHAR) ) ;
	ZeroMemory( m_szHostName,MAX_USER_LEN*sizeof(TCHAR) ) ;
}

VOID CPolicyComm::GetUserInfo(TCHAR *szSid, INT size, TCHAR *szUserName, INT iNameLen)
{
	HANDLE hTokenHandle = NULL ;
	if(! OpenThreadToken( GetCurrentThread(),TOKEN_QUERY,TRUE, &hTokenHandle ) )
	{
		if( GetLastError() == ERROR_NO_TOKEN )
		{
			if(!OpenProcessToken( ::GetCurrentProcess(),TOKEN_QUERY,&hTokenHandle ) )
			{
				//Open Process Token failure...
				return ;
			}
		}
		else
		{
			//Other error happens
			return  ;
		}
	}

	BOOL bRet = FALSE ;
	UCHAR uszInfoBuf[MAX_PATH*2] = {0} ;
	DWORD dwInfoLen = MAX_PATH*2 ;
	LPTSTR pszSid = NULL;

	WCHAR   uname[64] = {0}; 
	DWORD unamelen = 63;
	WCHAR   dname[64] = {0};
	DWORD dnamelen = 63;
	WCHAR   szFqdnname[MAX_PATH+1] = {0} ; 
	SID_NAME_USE snu;
	if( !GetTokenInformation( hTokenHandle,TokenUser,uszInfoBuf,dwInfoLen,&dwInfoLen ) )
	{
		return ;
	}

	if( ConvertSidToStringSid( ((PTOKEN_USER)uszInfoBuf)->User.Sid ,&pszSid ) )
	{//_tcsncpy should instead of _tcsncpy_s
		_tcsncpy_s(szSid, size, pszSid, _TRUNCATE);
		if( szSid ) ::LocalFree( pszSid ) ;
	}

	if( ::LookupAccountSid(NULL, ((PTOKEN_USER)uszInfoBuf)->User.Sid, uname, &unamelen, dname, &dnamelen, &snu) )
	{
		wcsncpy_s( szUserName, iNameLen, uname, _TRUNCATE) ;

		char  szHostName[MAX_PATH+1] = {0} ;
		WCHAR wzHostName[MAX_PATH+1] = {0} ;
		INT iRet = gethostname( szHostName,MAX_PATH ) ;
		//DWORD derr = 0 ;
		//derr = ::GetLastError() ;
		if( iRet == 0 )
		{
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szHostName, -1, wzHostName, MAX_PATH);
			wcsncat_s(szUserName, iNameLen, L"@", _TRUNCATE); 
		/*	OLUtilities::GetFQDN(szHostName, szFqdnname, MAX_PATH);
			wcsncat(szUserName, szFqdnname, nLeftSize);*/
		}
	}

}

BOOL CPolicyComm::Connect2PolicyServer()
{
	BOOL bRet = FALSE ;
	CEApplication   app;
	CEUser          user;
	CEString        host;
	CEint32         timeout_in_millisec = TIMEOUT_TIME;
	if( CFileManager::m_fnAllocCEString == NULL )
	{
		return bRet ;
	}
	ZeroMemory( &app,sizeof(CEApplication ) ) ;
	ZeroMemory( &user,sizeof( CEUser ) ) ;
	app.appName		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szModuleName ) ; //CEM_AllocateString( this->m_szModuleName ) ;
	app.appPath		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szModulePath ) ;
	app.appURL		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( L"" ) ;
	user.userID		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szSid ) ;
	user.userName	= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szUserName ) ;
	host			= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( L"10.187.2.111" ) ;

	CEResult_t	ceResult = ((CECONN_InitializeType)m_pCECONN_Initialize)( app,user,NULL,&this->m_connectHandle, timeout_in_millisec ) ;
	if( ( ceResult == CE_RESULT_SUCCESS ) && ( this->m_connectHandle != NULL ) )
	{
		//success ...
		bRet = TRUE ;
	}
	else
	{
		//failue
	}
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( app.appName ) ; 
	//CEM_FreeString( app.appName ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( app.appURL ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( user.userID ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( user.userName ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( host ) ;

	return bRet ;
}
VOID CPolicyComm::InitReference()
{
	GetUserInfo( m_szSid,MAX_USER_LEN,m_szUserName,MAX_USER_LEN ) ;
	m_ulIp = this->GetIP() ;

}
VOID CPolicyComm::Disconnect2PolicyServer()
{
	if(NULL != m_connectHandle)
	{
		CEint32         timeout_in_millisec = TIMEOUT_TIME;
		((CECONN_CloseType)m_pCECONN_Close)(m_connectHandle, timeout_in_millisec);
		m_connectHandle = NULL;
	}
}

DWORD CPolicyComm::GetIP(VOID)
{
	CHAR szHostName[MAX_USER_LEN] = {0} ;
	HOSTENT	*pHostEnt ;
	in_addr inaddr ;

	int iRet = gethostname( szHostName, MAX_USER_LEN ) ;
	if( iRet == 0 )
	{
		pHostEnt = gethostbyname( szHostName ) ;
		if( pHostEnt != NULL )
		{
			memcpy(&inaddr.S_un,   pHostEnt->h_addr,   pHostEnt->h_length);
			return ntohl(inaddr.S_un.S_addr);
		}
	}
	return 0 ;
}

BOOL CPolicyComm::VerifyFilePolicySingle( CEAction_t i_action,
										  CEString i_pszSrcFilePath,
										  CEAttributes *i_pSrcFileAttr,
										  CEString i_pszDesFilePath, 
										  CEAttributes *i_pDesFileAttr,CEEnforcement_t &enforcement  )
{
	BOOL bRet = TRUE ;
	CEApplication app ;
	CEUser user ;
	//CEEnforcement_t enforcement ;
	CEint32         timeout_in_millisec = TIMEOUT_TIME;
	
	::ZeroMemory( &app,sizeof( CEApplication ) ) ;
	::ZeroMemory( &user,sizeof( CEUser ) ) ;
	::ZeroMemory( &enforcement,sizeof( CEEnforcement_t ) ) ;

	app.appURL	= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( L"" ) ;
	app.appPath = ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szModulePath ) ;
	app.appName = ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szModuleName ) ;

	user.userID		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szSid ) ;
	user.userName	= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( this->m_szUserName ) ;
	if( m_connectHandle == NULL )
	{
		Connect2PolicyServer() ;
	}
	CEResult_t result ;
	try{
		if(m_pCEEVALUATE_CheckFile == NULL )
		{
			return FALSE ;
		}
		 result = /*CEEVALUATE_CheckFile*/((CEEVALUATE_CheckFileType)m_pCEEVALUATE_CheckFile)(	m_connectHandle, 
												i_action,
												i_pszSrcFilePath,
												i_pSrcFileAttr,
												i_pszDesFilePath,
												i_pDesFileAttr ,
												(CEint32) htonl(this->m_ulIp),
												&user,
												&app,
												CETrue,
												CE_NOISE_LEVEL_MIN,
												&enforcement,
												timeout_in_millisec ) ;
	}catch(...)
	{
		
	}
	if( result == CE_RESULT_SUCCESS ) 
	{
		bRet = (CEAllow == enforcement.result ) ;
	}
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( app.appName ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( app.appPath ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( app.appURL	)  ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( user.userID ) ;
	((CEM_FreeStringType)CFileManager::m_fnFreeCEString)( user.userName ) ;

	return bRet ;
}
BOOL CPolicyComm::VerifyFilePolicySingle(LONG i_action, wchar_t *i_pszFilePath,wchar_t* i_pszDestFilePath,CEEnforcement_t &enforcement )
{
	BOOL bRet = TRUE ;
	CEString pszSrcFilePath = ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( i_pszFilePath ) ;
	CEAttributes pFileAttr ;//= NULL ;
	CEString pszDesFilePath = ((CE_AllocStringType)CFileManager::m_fnAllocCEString)( i_pszDestFilePath ) ;
	ZeroMemory( &pFileAttr,sizeof( CEAttributes ) ) ;

	pFileAttr.attrs = new CEAttribute[2] ;
	pFileAttr.count = 2 ;

	TCHAR pszLastModifyTime[MAX_PATH] = {0} ;
	GetFileLastModifyTime( i_pszFilePath, pszLastModifyTime,MAX_PATH )	;
	if( wcslen( pszLastModifyTime ) == 0 )
	{
		::memcpy_s( pszLastModifyTime,MAX_PATH,L"2008/10/6",MAX_PATH ) ;
	}
	pFileAttr.attrs[0].key		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)(L"modified_date")	; 
	pFileAttr.attrs[0].value	= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)(pszLastModifyTime) ; 
	pFileAttr.attrs[1].key		= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)(L"resolved_name")	;
	pFileAttr.attrs[1].value	= ((CE_AllocStringType)CFileManager::m_fnAllocCEString)(i_pszFilePath)	;

	bRet = VerifyFilePolicySingle( (CEAction_t)i_action,
									pszSrcFilePath,
									&pFileAttr,
									pszDesFilePath,
									&pFileAttr ,enforcement) ;	

	return bRet ;
}