#ifndef __OE_MANAGER_H__
#define __OE_MANAGER_H__
/*
Created by Chellee 8-28-2008, 
for supporting the Encryption & File Tagging.
Encryption,Tagging...
Package the data for the Encryption and Tagging,
which is stored in the structure of OE...
*/

#include "PABase.h"
#include "PAMngr.h"
#include "..\Outlook\outlookUtilities.h"
#include "..\Outlook\MailItemUtility.h"
#include "strsafe.h"

#define OB_AUTOMATIC_XHEADER_TAGGING    L"AUTOMATIC_XHEADER_TAGGING"
#define OB_XHEADER_HIERARCHICAL_CLASSIFICATION  L"XHEADER_HIERARCHICAL_CLASSIFICATION"

class COE_PAMngr: public PA_Mngr::CPAMngr
{
public:
	COE_PAMngr()
	{
#ifdef _WIN64
		HMODULE hmod = ::GetModuleHandle( L"cesdk.dll" ) ;
#else
		HMODULE hmod = ::GetModuleHandle( L"cesdk32.dll" ) ;
#endif
		if( hmod ) 
		{
			this->m_pGetCEString = (PVOID)::GetProcAddress(	 hmod, "CEM_GetString" ) ;
		}
	} ;
	~COE_PAMngr()
	{

	} ;
public:
	PABase::PA_PARAM* GetPaParam(){return &m_paParam;}
	/*
	In the OE, the source file should be the Temporary file path...
		OBJECTINFO.strDisplayName uses #strOrigFileName, but if it is an empty string then uses #srcFileName. See CFileTagMgr::ParseParameters 
	*/
	INT SetOE_Obligations(const wchar_t *strOrigFileName,const wchar_t *srcFileName,CEAttributes *obligation, const wchar_t* wszOEActionForHSC ) 
	{
		int ccObligation  = 0;
		if( (srcFileName == NULL )||( strOrigFileName == NULL ) || obligation == NULL)
		{
			return	ccObligation ;
		}
		
		logd(L"[SetOE_Obligations]\"%s\" exists? %d, \"%s\" exists? %d", strOrigFileName, PathFileExists(strOrigFileName), srcFileName, PathFileExists(srcFileName)); 

		//if( lstrcmpiW( strOrigFileName,L"C:\\No_attachment.ice" ) == 0 )
        if(0 == _wcsicmp(strOrigFileName, L"C:\\No_attachment.ice"))
		{
			return  ccObligation ;
		}
		if(  wcslen( srcFileName ) == 0  )
		{
			return ccObligation ;
		}
		ccObligation = SetObligations( strOrigFileName,	 srcFileName, obligation ) ;


		//added OE action for Hierarchical Structure of Classification.
		//e.g. force not to do this obligation or force to to this obligation
		PABase::OBJECTINFO* pObjInfo = &(*m_paParam.objList.rbegin());
		PABase::OBLIGATIONLIST* lstObligation = &pObjInfo->obList;
		PABase::OBLIGATIONLIST::iterator itObligation = lstObligation->begin();
		while (itObligation != lstObligation->end())
		{
			if (_wcsicmp(itObligation->strOBName.c_str(), L"OE_HIERARCHICAL_CLASSIFICATION")==0) 
			{
				PABase::ATTRIBUTE oeActionName={ L"OEACTIONFORHSC_NAME", OEACTIONFORHSC_NAME};
				PABase::ATTRIBUTE oeActionValue={ L"OEACTIONFORHSC_VALUE", wszOEActionForHSC};
				itObligation->attrList.push_back(oeActionName);
				itObligation->attrList.push_back(oeActionValue);
			}
			itObligation++;
		}
		

		return ccObligation;
	};

	void SetRecipients(STRINGLIST _listRecipients)
	{
		std::vector<std::wstring>::iterator itor =	 _listRecipients.begin() ;
		for(itor ; itor!=_listRecipients.end() ; itor ++ )
		{
			m_paParam.recipList.push_back( (*itor) ) ;
		}
	}

	void SetSender(wstring & strSender)
	{
		m_paParam.strSender = strSender;
	}

	INT SetRecipients(  CComPtr<IDispatch> _pMailItem, STRINGLIST _listRecipients )
	{

		INT iCount = 0 ;
		GetSender(_pMailItem) ;
		//	m_paParam.strSender =  _szSender ;
		std::vector<std::wstring>::iterator itor =	 _listRecipients.begin() ;
		for(itor ; itor!=_listRecipients.end() ; itor ++ )
		{
			m_paParam.recipList.push_back( (*itor) ) ;
		}
		return	iCount ;
	}
	HRESULT GetSender(  CComPtr<IDispatch> _pMailItem )
	{
		HRESULT hr =S_OK ;
		wchar_t strlog[MAX_PATH] = {0};
		DWORD dwStartTime = GetTickCount();
		/*
		added by chellee on 02-09-2008 22-3
		*/
		//--------------------------------------------------------------------
		CComPtr<Outlook::_NameSpace>  namePtr = NULL ;
		/*BSTR bNameSpace = ::SysAllocString( L"MAPI" ) ;
		hr = m_spApp->GetNamespace( bNameSpace, &namePtr) ;
		if(FAILED( hr ) )
		{
		DP((L"Get Current User Failure!\n"));
		}*/
		
		CEventLog::WriteEventLog(L"Enter get_Session GetSender @@",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
		hr = MailItemUtility::get_Session(_pMailItem, &namePtr) ;
		if(FAILED( hr ) )
		{
			return S_FALSE ;
			DP((L"Get session User Failure!\n"));
		}
		CComPtr<Outlook::Recipient> current = NULL ;

		CEventLog::WriteEventLog(L"Enter get_CurrentUser GetSender @@",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);

		hr = namePtr->get_CurrentUser( &current) ;
		if(FAILED( hr ) )
		{
			DP((L"Get Current User Failure!\n"));
			return S_FALSE ;
		}
		CComPtr<Outlook::AddressEntry> addrEntry ;

		CEventLog::WriteEventLog(L"Enter get_AddressEntry GetSender @@\n",EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);

		hr = current->get_AddressEntry( &addrEntry ) ;
		if(FAILED( hr ) )
		{
			DP((L"Get get_AddressEntry  Failure!\n"));
			return S_FALSE ;
		}
		STRINGLIST		strRecipient;
		OLUtilities::ExpandAddressEntry(  addrEntry, strRecipient ) ;
		if( !strRecipient.empty() )
		{
			m_paParam.strSender = strRecipient[0] ;
		}

		StringCchPrintf(strlog,MAX_PATH,L"GetSender spend time is [%d] @@@",GetTickCount() - dwStartTime);
		CEventLog::WriteEventLog(strlog,EVENTLOG_INFORMATION_TYPE,EVENTLOG_INFO_ID);
		//--------------------------------------------------------------------
		return hr ;
	} ;
	INT Check_VaildPosition( CComPtr<Outlook::Attachments> pAtts, LONG& iCurrent )
	{
		LONG iPosition = iCurrent ;
		try{
			CComPtr<Outlook::Attachment>   spAttachment;
			VARIANT vi; vi.vt = VT_INT; 
			HRESULT hr = FALSE ;
			while(true)
			{
				vi.intVal = iPosition;
				DP((L"Get Item start!\n"));
				hr = pAtts->Item(vi, &spAttachment);
				DP((L"Get Item End!\n"));
				if(	( FAILED( hr ) ) )
				{
					break ;
				}
				DP((L"Increase!\n"));
				iPosition = iPosition+1  ;
			}
		}
		catch(...)
		{}
		DP((L"Finished!\n"));
		iCurrent =	 iPosition ;
		return 0 ;
	}
	static INT Reset_DisplayName(  CComPtr<Outlook::Attachment> pAttachment, const wchar_t *pszFullName )
	{
		INT iRet = 0 ;
		if( pAttachment )
		{
			if(IsMsgFile(pszFullName))
			{
				LPCWSTR pszFileName = wcsrchr(pszFullName, L'\\');
				if(NULL != pszFileName)
				{	
					DP((L"Display name: %s\n", pszFileName+1));
					BSTR DisplayName = ::SysAllocString( pszFileName+1 ) ;
					pAttachment->put_DisplayName( DisplayName ) ;
					::SysFreeString(  DisplayName ) ;
				}
			}
		}
		return iRet ;
	}
public:

	BOOL CheckAndCreateRandTemp( std::wstring strFilePath, std::wstring &strTempFolder ) 
	{
		BOOL bRet = FALSE ;
		struct _stat stat_buffer;
		if (_wstat(strFilePath.c_str(), &stat_buffer) == 0)
		{
			/* If the encrypted file exists, create a random temp folder to hold the new one. */

			WCHAR wzFolderName[MAX_PATH];
			DWORD dwCurrentTick = 0;

			dwCurrentTick = GetTickCount();
			_snwprintf_s(wzFolderName, MAX_PATH, _TRUNCATE, L"%8x", dwCurrentTick);

			//
			strTempFolder += wzFolderName;
			strTempFolder += L"\\";
			CreateDirectoryW(strTempFolder.c_str(), NULL);
			bRet = TRUE ;
		}
		return bRet ;
	}
 
	static BOOL IsMsgFile(LPCWSTR pwzFile)
	{
		if(	 pwzFile == NULL )
		{
			return FALSE ;
		}
		LPCWSTR pSuffix = wcsrchr(pwzFile, L'.');
		if(NULL == pSuffix) return FALSE;

		if(0 == _wcsicmp(pSuffix, L".msg") || 0 == _wcsicmp(pSuffix, L".oft"))
			return TRUE;

		return FALSE;
	}
	static BOOL IsPubFile( const wchar_t* pszFile )
	{
		if( pszFile == NULL )
		{
			return FALSE ;
		}
	   LPCWSTR pSuffix = wcsrchr(pszFile, L'.');
		if(NULL == pSuffix) return FALSE;

		if(0 == _wcsicmp(pSuffix, L".pub"))
			return TRUE;

		return FALSE;
	}
	INT ClearObjectList (VOID )
	{
		INT iRet = 0 ;
		m_paParam.objList.clear() ;
		return iRet ;
	}
private:

	VOID GetParentWindow(HWND &_parentWnd)
	{
		_parentWnd = ::FindWindow( L"OpusApp", NULL ) ;
		if(	  _parentWnd == NULL )
		{
			_parentWnd = ::FindWindow( L"rctrl_renwnd32", NULL ) ;
			if( _parentWnd == NULL )
			{
				_parentWnd = ::GetForegroundWindow() ;
			}
		}
	}


};

#endif