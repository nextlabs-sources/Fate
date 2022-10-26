#include "stdafx.h"
#include "FileManger.h"
#include "PALoad.h"
PVOID CFileManager::m_fnAllocCEString = NULL ;
PVOID CFileManager::m_fnGetCEString	= NULL ;
PVOID CFileManager::m_fnFreeCEString = NULL ;

CFileManager::CFileManager(const HINSTANCE hInst) {
	m_hEnc = NULL ;
	m_hTag = NULL ;
	PA_LOAD::LoadModuleByName(	PA_LOAD::PA_MODULE_NAME_ENC, PA_LOAD::PA_ENCRYPTION, m_hEnc, NULL ) ;
	PA_LOAD::LoadModuleByName(	PA_LOAD::PA_MODULE_NAME_TAG, PA_LOAD::PA_FILETAGGING, m_hTag, NULL ) ;
	m_hCEType = NULL ;
	m_hCEType = ::LoadLibrary( L"cecem.dll" ) ;
	m_fnGetCEString = NULL ;
	this->m_fnAllocCEString = NULL ;
	if( m_hCEType )
	{
		
		m_fnGetCEString = (PVOID)::GetProcAddress( m_hCEType,"CEM_GetString" ) ; 
		m_fnAllocCEString = (PVOID)::GetProcAddress( m_hCEType,"CEM_AllocateString" ) ; 
		m_fnFreeCEString = (PVOID)::GetProcAddress( m_hCEType,"CEM_FreeString" ) ; 
	}
	else
	{
		MessageBox( 0,L"Load Library [cecem.dll] Failure",0,0 ) ;
	}
	m_hOutWnd = NULL ;
} ;
CFileManager::~CFileManager(){
	if( m_hEnc )
	{
		::FreeLibrary( m_hEnc ) ;
	}
	//if( m_hTag ) 
	//{
	//	::FreeLibrary( m_hTag ) ;
	//}
} ;
BOOL CFileManager::DoAction_Copy( wchar_t *pszSrcFile, wchar_t * pszDestFile ) 
{
	try
	{
		CPolicyComm m_policyComm ;
		TCHAR szSid[MAX_PATH*2] = {0} ;
		TCHAR szUserName[MAX_PATH*2] = {0} ;
		INT iSize = 128 ;
		INT iUserNameLen = 128 ;
		m_policyComm.InitReference() ;
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Connect to the Policy " ) ;
		BOOL bRet = m_policyComm.Connect2PolicyServer() ;
		if( bRet == TRUE )
		{
			m_policyComm.GetUserInfo( szSid,iSize,szUserName,iUserNameLen ) ;
		}
		else
		{
			MessageBox( NULL, L"Please open and run the PDP first!", L"PDP Not Running", MB_OK|MB_ICONWARNING  ) ;
			return bRet ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Verify Policy for File.Source File:" ) ;
		DO_OUTPUT_STRING( m_hOutWnd, pszSrcFile ) ;
		BOOL bret = FALSE;									   
		CEEnforcement_t enforcement ;
		bRet = m_policyComm.VerifyFilePolicySingle(_CEAction_t::CE_ACTION_COPY, const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*>(  pszDestFile ),enforcement );

		PA_Mngr::CPAMngr paMgr(m_fnGetCEString) ;
		paMgr.SetObligations( const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*> (pszSrcFile), enforcement.obligation ) ;
		//	paMgr.DeleverRealAddr_API( L"CopyFileExW",s_CopyFileEx ) ;
		if(	 m_hTag == NULL ) 
		{
			MessageBox( NULL, L"Please Install the product of OE4.0 firstly, or Setting the modules of File Tagging!", L"Module of File Tagging not found", MB_OK|MB_ICONWARNING  ) ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Do File Tagging") ;
		LONG lRet = paMgr.DoFileTagging( m_hTag,NULL,PABase::AT_COPY,FALSE ) ;

		if(	 m_hEnc == NULL ) 
		{
			MessageBox( NULL, L"Please Install the product of OE4.0 firstly, or Setting the modules of File Encryption!", L"Module of File Encryption not found", MB_OK|MB_ICONWARNING  ) ;
		}
		if( lRet != 0 )
		{
			DP(( L"Do File Tagging Failure: Source[%s],Destination[%s]",pszSrcFile,pszDestFile ) ) ;
			DO_OUTPUT_STRING( m_hOutWnd, L"Do File Tagging Failure!") ;
			return bRet ;
		}
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Tagging Success!") ;
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Do File Encryption") ;
		lRet = paMgr.DoEcnryption( m_hEnc,NULL,PABase::AT_COPY,TRUE ) ;
		if( lRet != 0 )
		{
			DO_OUTPUT_STRING( m_hOutWnd, L"Do File Encryption Failure!") 
				DP(( L"Do File Encryption Failure: Source[%s],Destination[%s]",pszSrcFile,pszDestFile ) ) ;
			return bRet ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Do File Encryption Success!") ;
		wchar_t szRetFileName[MAX_PATH] = {0} ;
		BOOL bHasChanged = FALSE ;
		DP((L"Query the result file name:Source[%s]",pszSrcFile ) ) ;
		paMgr.QueryRetName_bySrc( pszSrcFile, bHasChanged, szRetFileName ) ;  
		wchar_t drive[_MAX_DRIVE] = {0} ;
		wchar_t dir[_MAX_DIR]= {0} ;
		wchar_t fname[_MAX_FNAME] = {0} ;
		wchar_t ext[_MAX_EXT] = {0} ;
		if(	 bHasChanged ==  TRUE )
		{
			DO_OUTPUT_STRING( m_hOutWnd,L"Source file has been modified in the PA! New File Name:") ;
			DO_OUTPUT_STRING( m_hOutWnd,szRetFileName) ;
			DP(( L"File name has been changed:New Source[%s],Destination[%s]",szRetFileName,pszDestFile ) ) ;
			::_wsplitpath( szRetFileName, drive,	 dir,	fname,ext ) ;
			::wcsncat_s(	 fname,	 _MAX_FNAME, ext, _TRUNCATE ) ;
			::wcsncat_s(	 pszDestFile,	 MAX_PATH, fname, _TRUNCATE ) ;
			CopyFileEx(	szRetFileName, pszDestFile, NULL,NULL,FALSE,COPY_FILE_ALLOW_DECRYPTED_DESTINATION ) ;
		}
		else
		{
			DP(( L"File name has not been changed:Origin Source[%s],Destination Path[%s]",pszSrcFile,pszDestFile ) ) ;
			DO_OUTPUT_STRING( m_hOutWnd,L"Source file has not been modified in the PA!") ;
			::_wsplitpath(	 pszSrcFile, drive,	 dir,	fname,ext ) ;
			::wcsncat_s(	 fname,	 _MAX_FNAME, ext, _TRUNCATE ) ;
			::wcsncat_s(	 pszDestFile,	 MAX_PATH, fname, _TRUNCATE ) ;
			CopyFileEx(	pszSrcFile, pszDestFile, NULL,NULL,FALSE,COPY_FILE_ALLOW_DECRYPTED_DESTINATION ) ;
		}
		::ZeroMemory( drive,  _MAX_DRIVE*sizeof(wchar_t)) ;
		::ZeroMemory( dir,  _MAX_DIR*sizeof(wchar_t)) ;
		::ZeroMemory( fname,  _MAX_FNAME*sizeof(wchar_t)) ;
		::ZeroMemory( ext,  _MAX_EXT*sizeof(wchar_t)) ;
		m_policyComm.Disconnect2PolicyServer() ;
	}
	catch(...)
	{
		return FALSE ;
	}
	return TRUE ;
}
BOOL CFileManager::DoAction_Move( wchar_t *pszSrcFile, wchar_t * pszDestFile )  
{
	try{
		CPolicyComm m_policyComm ;
		TCHAR szSid[MAX_PATH*2] = {0} ;
		TCHAR szUserName[MAX_PATH*2] = {0} ;
		INT iSize = 128 ;
		INT iUserNameLen = 128 ;
		m_policyComm.InitReference() ;
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Connect to the Policy " ) ;
		BOOL bRet = m_policyComm.Connect2PolicyServer() ;
		if( bRet == TRUE )
		{
			m_policyComm.GetUserInfo( szSid,iSize,szUserName,iUserNameLen ) ;
		}
		else
		{
			MessageBox( NULL, L"Please open and run the PDP first!", L"PDP Not Running", MB_OK|MB_ICONWARNING  ) ;
			return bRet ;
		}
		BOOL bret = FALSE;									   
		CEEnforcement_t enforcement ;

		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Verify Policy for File.Source File:" ) ;
		DO_OUTPUT_STRING( m_hOutWnd, pszSrcFile ) ;
		bRet = m_policyComm.VerifyFilePolicySingle(_CEAction_t::CE_ACTION_MOVE, const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*>(  pszDestFile ),enforcement );

		PA_Mngr::CPAMngr paMgr(m_fnGetCEString) ;
		paMgr.SetObligations( const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*> (pszSrcFile), enforcement.obligation ) ;
		//	paMgr.DeleverRealAddr_API( L"CopyFileExW",s_CopyFileEx ) ;
		if(	 m_hTag == NULL ) 
		{
			MessageBox( NULL, L"Please Install the product of OE4.0 firstly, or Setting the modules of File Tagging!", L"Module of File Tagging not found", MB_OK|MB_ICONWARNING  ) ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Do File Tagging") ;
		LONG lRet = paMgr.DoFileTagging( m_hTag,NULL,PABase::AT_MOVE,FALSE ) ;
		if( lRet != 0 )
		{
			DP(( L"Do File Tagging Failure: Source[%s],Destination[%s]",pszSrcFile,pszDestFile ) ) ;
			DO_OUTPUT_STRING( m_hOutWnd, L"Do File Tagging Failure!") ;
			return bRet ;
		}
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Tagging Success!") ;
		if(	 m_hEnc == NULL ) 
		{
			MessageBox( NULL, L"Please Install the product of OE4.0 firstly, or Setting the modules of File Encryption!", L"Module of File Encryption not found", MB_OK|MB_ICONWARNING  ) ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Begin Do File Encryption") ;
		lRet = paMgr.DoEcnryption( m_hEnc,NULL,PABase::AT_MOVE,TRUE ) ;
		if( lRet != 0 )
		{
			DO_OUTPUT_STRING( m_hOutWnd, L"Do File Encryption Failure!") ;
			DP(( L"Do File Encryption Failure: Source[%s],Destination[%s]",pszSrcFile,pszDestFile ) ) ;
			return bRet ;
		}
		DO_OUTPUT_STRING( m_hOutWnd, L"Do File Encryption Success!") ;
		wchar_t szRetFileName[MAX_PATH] = {0} ;
		BOOL bHasChanged = FALSE ;
		DP((L"Query the result file name:Source[%s]",pszSrcFile ) ) ;
		paMgr.QueryRetName_bySrc( pszSrcFile, bHasChanged, szRetFileName ) ;  
		wchar_t drive[_MAX_DRIVE] = {0} ;
		wchar_t dir[_MAX_DIR]= {0} ;
		wchar_t fname[_MAX_FNAME] = {0} ;
		wchar_t ext[_MAX_EXT] = {0} ;
		if(	 bHasChanged ==  TRUE )
		{
			DO_OUTPUT_STRING( m_hOutWnd,L"Source file has been modified in the PA! New File Name:") ;
			DO_OUTPUT_STRING( m_hOutWnd,szRetFileName) ;
			DP(( L"File name has been changed:New Source[%s],Destination[%s]",szRetFileName,pszDestFile ) ) ;
			::_wsplitpath(	 szRetFileName, drive,	 dir,	fname,ext ) ;
			::wcsncat_s(	 fname,	 _MAX_FNAME, ext, _TRUNCATE ) ;
			::wcsncat_s(	 pszDestFile,	 MAX_PATH, fname, _TRUNCATE ) ;
			if( MoveFileExW(	szRetFileName, pszDestFile, MOVEFILE_REPLACE_EXISTING ) )
			{
				if( DeleteFile(	  pszSrcFile ) == 0)
				{
					DWORD derr = ::GetLastError() ;
					DO_OUTPUT_STRING( m_hOutWnd,L"Delete File Failure! Error Code:") ;
					wchar_t szBuf[10]= {0} ;
					_snwprintf_s( szBuf, 10, _TRUNCATE, L"%d",derr ) ;
					DO_OUTPUT_STRING( m_hOutWnd,szBuf ) ;
					DP(( L"Delete File Failure! Source[%s]:Error Code[%d]",pszSrcFile,derr ) ) ;
					//MessageBox( 0,L"Delete File Failure!",szBuf,0 ) ;
				}
			}
		}
		else
		{
			DP(( L"File name has not been changed:Origin Source[%s],Destination Path[%s]",pszSrcFile,pszDestFile ) ) ;
			DO_OUTPUT_STRING( m_hOutWnd,L"Source file has not been modified in the PA!") ;
			::_wsplitpath(	 pszSrcFile, drive,	 dir,	fname,ext ) ;
			::wcsncat_s(	 fname,	 _MAX_FNAME, ext, _TRUNCATE ) ;
			MoveFileExW(	pszSrcFile, pszDestFile, MOVEFILE_REPLACE_EXISTING ) ;
		}
		::ZeroMemory( drive,  _MAX_DRIVE*sizeof(wchar_t)) ;
		::ZeroMemory( dir,  _MAX_DIR*sizeof(wchar_t)) ;
		::ZeroMemory( fname,  _MAX_FNAME*sizeof(wchar_t)) ;
		::ZeroMemory( ext,  _MAX_EXT*sizeof(wchar_t)) ;
		m_policyComm.Disconnect2PolicyServer() ;
	}
	catch(...)
	{
		return FALSE ;
	}
	return TRUE ;
}
VOID CFileManager::SetOutPutWnd( HWND hWnd )
{
	m_hOutWnd =	  hWnd ;
}

BOOL CFileManager::DoStub( wchar_t *pszSrcFile, wchar_t *pszDestFile ) 
{
	BOOL bRet = FALSE ;
	if( m_fnAllocCEString == NULL )
	{
		return bRet ;
	}
	DO_OUTPUT_STRING( m_hOutWnd,L"Do: SetObligations:Source File") ;
	DO_OUTPUT_STRING( m_hOutWnd,pszSrcFile) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Packaging the parameter of Obligation") ;
	CEAttributes  Obligations ;
	Obligations.attrs = new CEAttribute[12] ;
	CEString key = /*CEM_AllocateString*/((CE_AllocStringType)m_fnAllocCEString)( L"Count" ) ;

	CEString Value = ((CE_AllocStringType)m_fnAllocCEString)( L"12" ) ;

	Obligations.attrs[0].key = key ;
	Obligations.attrs[0].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_NAME:1" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_NAME:1" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"ManualFileTagging" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"ManualFileTagging" ) ;
	Obligations.attrs[1].key = key ;
	Obligations.attrs[1].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_POLICY:1" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_POLICY:1" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Storm/File Tagging/Manual" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Storm/File Tagging/Manual" ) ;
	Obligations.attrs[2].key = key ;
	Obligations.attrs[2].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:1" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:1" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Tag Name" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Tag Name" ) ;
	Obligations.attrs[3].key = key ;
	Obligations.attrs[3].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:2" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:2" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Document Class" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Document Class" ) ;
	Obligations.attrs[4].key = key ;
	Obligations.attrs[4].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:3" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:3" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Tag Values" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Tag Values" ) ;
	Obligations.attrs[5].key = key ;
	Obligations.attrs[5].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:4" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:4" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"EIRENAEI3E3AF123;kiiiJ;tDAJEIANFA" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"EIRENAEI3E3AF123;kiiiJ;tDAJEIANFA" ) ;
	Obligations.attrs[6].key = key ;
	Obligations.attrs[6].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:5" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:5" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Target" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Target" ) ;
	Obligations.attrs[7].key = key ;
	Obligations.attrs[7].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:6" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:6" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Source" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Source" ) ;
	Obligations.attrs[8].key = key ;
	Obligations.attrs[8].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:7" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:7" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Description" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Description" ) ;
	Obligations.attrs[9].key = key ;
	Obligations.attrs[9].value = Value ;
	//Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Description" ) ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:1:8" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:1:8" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"AAAAAAAAAAAAAAAAAAAAAA" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"AAAAAAAAAAAAAAAAAAAAAA" ) ;
	Obligations.attrs[10].key = key ;
	Obligations.attrs[10].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_NUMVALUES:1" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_NUMVALUES:1" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"8" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"8" ) ;
	Obligations.attrs[11].key = key ;
	Obligations.attrs[11].value = Value ;
	Obligations.count = 12 ;
	PA_Mngr::CPAMngr paMgr(m_fnGetCEString) ;
	//-----------------------------------------------------------------------------------------
	CEAttributes  ObEnc ;
	ObEnc.attrs = new CEAttribute[10] ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"Count" ) ;

	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"10" ) ;

	ObEnc.attrs[0].key = key ;
	ObEnc.attrs[0].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_NAME:2" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_NAME:2" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"File Encryption Assistant" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"File Encryption Assistant" ) ;
	ObEnc.attrs[1].key = key ;
	ObEnc.attrs[1].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_POLICY:2" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_POLICY:2" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Storm/File Tagging/Manual" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Storm/File Tagging/Multiple obligations" ) ;
	ObEnc.attrs[2].key = key ;
	ObEnc.attrs[2].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:1" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:1" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Encryption Adapter" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Encryption Adapter" ) ;
	ObEnc.attrs[3].key = key ;
	ObEnc.attrs[3].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:2" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:2" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"ZIP" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"ZIP" ) ;
	ObEnc.attrs[4].key = key ;
	ObEnc.attrs[4].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:3" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:3" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Description" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Description" ) ;
	ObEnc.attrs[5].key = key ;
	ObEnc.attrs[5].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:4" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:4" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"AAAAAAAAAAAAAAAAAAAAAA" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"AAAAAAAAAAAAAAAAAAAAAA" ) ;
	ObEnc.attrs[6].key = key ;
	ObEnc.attrs[6].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:5" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:5" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"Optional" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Optional" ) ;
	ObEnc.attrs[7].key = key ;
	ObEnc.attrs[7].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_VALUE:2:6" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_VALUE:2:6" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"true" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"true" ) ;
	ObEnc.attrs[8].key = key ;
	ObEnc.attrs[8].value = Value ;
	key = ((CE_AllocStringType)m_fnAllocCEString)( L"CE_ATTR_OBLIGATION_NUMVALUES:2" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"CE_ATTR_OBLIGATION_NUMVALUES:2" ) ;
	Value = ((CE_AllocStringType)m_fnAllocCEString)( L"6" ) ;
	DO_OUTPUT_STRING( m_hOutWnd,L"6" ) ;
	ObEnc.attrs[9].key = key ;
	ObEnc.attrs[9].value = Value ;
	ObEnc.count = 10 ;
	//	PA_Mngr::CPAMngr paMgr ;
	//-----------------------------------------------------------------------------------------


	paMgr.SetObligations( const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*> (pszSrcFile), &Obligations) ;
	delete[] Obligations.attrs ;
	paMgr.SetObligations( const_cast<wchar_t*> (pszSrcFile), const_cast<wchar_t*> (pszSrcFile), &ObEnc) ;
	delete[] ObEnc.attrs ;
	DO_OUTPUT_STRING( m_hOutWnd,L"SetObligations Finished!") ;
	DO_OUTPUT_STRING( m_hOutWnd,L"Do File Tagging Begin!") ;
	LONG lRet = paMgr.DoFileTagging( m_hTag,NULL,PABase::AT_MOVE,FALSE ) ;
	if( lRet!= 0 )
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Tagging Failure!") ;
	}
	else
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Tagging Success!") ;
	}
	DO_OUTPUT_STRING( m_hOutWnd,L"Do File Encryption Begin!") ;
	lRet =  lRet = paMgr.DoEcnryption( m_hEnc,NULL,PABase::AT_MOVE,TRUE ) ;
	if( lRet!= 0 )
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Encryption Failure!") ;
	}
	else
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Do File Encryption Success!") ;
	}
	wchar_t szRetFileName[MAX_PATH] = {0} ;
	BOOL bHasChanged = FALSE ;
	DO_OUTPUT_STRING( m_hOutWnd,L"QueryRetName_bySrc Begin!") ;
	paMgr.QueryRetName_bySrc( pszSrcFile, bHasChanged, szRetFileName ) ;  
	if( bHasChanged )
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Source file has been modified in the PA! New File Name:") ;
		DO_OUTPUT_STRING( m_hOutWnd,szRetFileName) ;
	}
	else
	{
		DO_OUTPUT_STRING( m_hOutWnd,L"Source file has not been modified in the PA!") ;
	}
	DO_OUTPUT_STRING( m_hOutWnd,L"Stub End!") ;
	return bRet ;
}