#ifndef __PA_MANAGER_H__
#define __PA_MANAGER_H__
/*
Created by Chellee 8-28-2008, 
for supporting the Encryption & File Tagging.
Encryption,Tagging...
Package the data for the Encryption and Tagging,
which is stored in the structure of OE...
*/

#include "PABase.h"

namespace PA_Mngr
{
/*
for the key of Obligation
*/
#define OBLIGATION_NAME_ID              L"CE_ATTR_OBLIGATION_NAME"
#define OBLIGATION_NUMVALUES_ID         L"CE_ATTR_OBLIGATION_NUMVALUES"
#define OBLIGATION_VALUE_ID             L"CE_ATTR_OBLIGATION_VALUE"

#define PA_INTERFACE_NAME		"DoPolicyAssistant"

	const	wchar_t PA_TEMP_FOLDER_NAME[] = L"PA\\"	 ;
	const	wchar_t PA_OBLIGATION_ENC[]	  =	L"File Encryption Assistant" ;
	const   wchar_t PA_OBLIGATION_ENC_CER[] = L"Certificate Encryption Assistant"  ;

	/*
	Typedefine the function type for the module of the PA.
	*/
	typedef enum _tagOBLIGATION
	{
		OB_HDR ,
		OB_ENC ,
		OB_TAG
	}OBTYPE;
	/*
	Typedefine the function type for the module of the PA.
	*/
	typedef PABase::PA_STATUS ( WINAPI *DoPolicyAssistantType )( PABase::PA_PARAM &_iParam,const HWND _hParentWnd ) ;
	typedef wchar_t* (WINAPIV* Get_CEStringType)( CEString strValue ) ;
	class CPAMngr
	{
	public:
		CPAMngr( const PVOID& fnGetCEString = NULL )
		{
			::ZeroMemory(	 m_szTempFoder, (MAX_PATH+1)*sizeof(wchar_t) ) ;
			m_bHasTagging = FALSE ;
			m_bHasEnc = FALSE ;
			m_bHasHDR = FALSE ;
			m_pGetCEString =  fnGetCEString ;
		} ;
		~CPAMngr()
		{
			/*if( !m_paParam.objList.empty() )
			{
				m_paParam.objList.clear() ;
			}
			if( !m_paParam.recipList.empty() )
			{
				m_paParam.recipList.clear() ;
			}
			if( ::lstrlenW(m_szTempFoder)>0 )
			{
				::ZeroMemory(	 m_szTempFoder, (MAX_PATH+1)*sizeof(wchar_t) ) ;
			}*/
		} ;
	public:
		/*
		In the OE, the source file should be the Temporary file path...
		strSrcFileName:
			[in]It is the original full file path. For WDE: 
	    TargetFileName:
			[in]It is the full file path, which is the object done the obligation by the PA .
	    obligation:
			[in]This data is the obligation returned from The Policy Controller (PDP).
		*/
		bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName,const CEAttributes *obligation) 
		{
			int i=0;
			PABase::OBJECTINFO objInfo ;
			objInfo.strSrcName = TargetFileName ;
			objInfo.strDisplayName =	strSrcFileName ;
			if(NULL== obligation || NULL==obligation->attrs||0==obligation->count) return false;

			// The first key must be the count of obligation
			if(NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[0].key) || NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[0].value)) return false;
			INT iCount = 0 ;

			for(i=1; i<obligation->count; i++)
			{
				PABase::OBLIGATION obInfo ;
				if(NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[i].key) || NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[i].value))
					continue;

				std::wstring name(((Get_CEStringType)m_pGetCEString)(obligation->attrs[i].key));
				std::wstring value(((Get_CEStringType)m_pGetCEString)(obligation->attrs[i].value));
				//DP((L"Obligations: [%s], <%s>\n", name.c_str(), value.c_str()));
				// create new Obligations
				if(0==wcsncmp(OBLIGATION_NAME_ID, name.c_str(), wcslen(OBLIGATION_NAME_ID)))
				{	
					obInfo.strOBName =  value.c_str();
					if( wcscmp( PA_Mngr::PA_OBLIGATION_ENC, obInfo.strOBName.c_str() ) == 0 || wcscmp(PA_Mngr::PA_OBLIGATION_ENC_CER,obInfo.strOBName.c_str() ) == 0  ) 
					{
						this->m_bHasEnc = TRUE ;
					}
					while( true )
					{

						int j = i +1 ;
						if( j >= obligation->count )
						{
							break ;
						}
						if(NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].key) )
						{
							i++ ;
							continue;
						}
						if(  NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].value) )
						{
							std::wstring name(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].key));
							if(0==wcsncmp(OBLIGATION_VALUE_ID, name.c_str(), wcslen(OBLIGATION_VALUE_ID)))
							{
								PABase::ATTRIBUTE attrInfo ;
								obInfo.attrList.push_back( attrInfo ) ;
								i++ ;
								continue ;
							}
						}
						std::wstring name(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].key));
						std::wstring value(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].value));
						//DP((L"Obligations: [%s], <%s>\n", name.c_str(), value.c_str()));
						if(0==wcsncmp(OBLIGATION_NAME_ID, name.c_str(), wcslen(OBLIGATION_NAME_ID)))
						{
							break ;
						}
						i++ ;
						if(0==wcsncmp(OBLIGATION_NUMVALUES_ID, name.c_str(), wcslen(OBLIGATION_NUMVALUES_ID)))
						{
							continue ;
						}
						if(0==wcsncmp(OBLIGATION_VALUE_ID, name.c_str(), wcslen(OBLIGATION_VALUE_ID)))
						{
							PABase::ATTRIBUTE attrInfo ;
							attrInfo.strValue = value.c_str() ;
							obInfo.attrList.push_back( attrInfo ) ;
							continue ;
						}
						else
						{
							continue ;
						}
					}
					objInfo.obList.push_back( obInfo ) ;
				}
			}
			wchar_t pszTempFolder[MAX_PATH+1];
			std::wstring temp_folder;
			if( GetTempFolder(temp_folder,NULL) == -1 )
			{
			  return false;
			}
			objInfo.strTempName = temp_folder;
			objInfo.lPARet = 0 ;
			objInfo.bFileNameChanged = FALSE ;
			m_paParam.objList.push_back( objInfo ) ;
			return true;
		};
		/*
		Updates the temp file name(here is the source file name)...
	   _pszOrigName:
			[in]This file name( Full Path) is the same to the TargetFileName which is set in the follow function
	   _pszNewName:
			[out] If the file path (full path)has been changed in the PA. The new will be written to this buffer, else this string is empty. 
		*/
		INT UpdateSrcFileName( const wchar_t *_pszOrigName, const wchar_t* _pszNewName )
		{
			INT iRet = 0 ;
			std::list< PABase::OBJECTINFO >::iterator itor = m_paParam.objList.begin() ;
			for( itor ; itor!= m_paParam.objList.end() ; itor ++ )
			{
				if( lstrcmpiW( (*itor).strSrcName.c_str(), _pszOrigName ) == 0  )
				{
					(*itor).strSrcName =   _pszNewName ;
					/*
					Modified by chellee on 14/10/08 ;6:32
					mark Code:(*itor).strRetName =   _pszNewName ;
					*/
					::wcsncpy_s( (*itor).strRetName, MAX_PATH, _pszNewName, _TRUNCATE )  ;
					//-----------------------------------------------------------------

					(*itor).bFileNameChanged =  TRUE ;
					break ;
				}
			}
			return iRet ;
		}  ;

		VOID SetTempFolderForPA( const wchar_t * i_tempFolder = NULL )
		{
			if( i_tempFolder  )
			{
				if( m_szTempFoder )
				{
					::ZeroMemory(	 m_szTempFoder, (MAX_PATH+1)*sizeof(wchar_t) ) ;
				}
				wcsncpy_s(m_szTempFoder,_countof(m_szTempFoder),i_tempFolder, _TRUNCATE);
			}
		}

		/*
		Get the temp folder
		*/
		INT GetTempFolder( std::wstring& temp_folder , const wchar_t *   i_pszFolderName= NULL)
		{	 
			INT iRet= 0 ;
			if( ::wcslen( m_szTempFoder ) == 0 )
			{
			        iRet =	 CreateTempFolderForPA(	 i_pszFolderName ) ; /* sets m_szTempFoder */
			}
			if( ::wcslen( m_szTempFoder ) != 0 )
			{
			  temp_folder = m_szTempFoder;
			}
			return iRet ;
		}

	public:

		static void DoLog(	PVOID lpLogCtx,
			PABase::tString &wstrlogIdentifier,
			PABase::tString &wstrAssistantName, 
			PABase::tString &wstrAssistantOptions, 
			PABase::tString &wstrAssistantDescription, 
			PABase::tString &wstrAssistantUserActions, 
			PABase::ATTRIBUTELIST &optAttributes)
		{
		  //DP((L"Log Identifier:%s\n",wstrlogIdentifier.c_str()));
		  //DP((L"Log Assistant Name :%s\n",wstrAssistantName.c_str()));
		  //DP((L"Log Assistant Options :%s\n",wstrAssistantOptions.c_str()));
		  //DP((L"Log Assistant Description :%s\n",wstrAssistantDescription.c_str()));
		  //DP((L"Log Assistant User Actions :%s\n",wstrAssistantUserActions.c_str()));
		  std::list<	PABase::ATTRIBUTE>::iterator itor =	  optAttributes.begin() ;
			for( itor ; itor!= optAttributes.end() ; itor ++ )
			{
			  //DP((L"Log Optional Attribute Key:%s,Value:%s\n",(*itor).strKey.c_str(),(*itor).strValue.c_str()));
			}
			if( lpLogCtx == NULL )
			{
				return ;
			}
			/*PolicyCommunicator* pPcyCmmtor =	(PolicyCommunicator*)lpLogCtx ;
			pPcyCmmtor->LogDecision( 
			CEString strKey = CEM_AllocateString (const TCHAR * str);
			CEAttributes **/
		};
		/*
		Do Encryption:
		1)	_lpLogCtx
			The File tagging and encryption will call the CALLBACK, this context will be used in the function of CALLBACK.
		2)	_iAction
			If current PA is the last one, the pep(WDE) should tell the PA. The PA will show the last button name in the end obligation in the PA.
			Such as:	¡°Send E-mail¡± in OE
			¡°OK¡± in WDE, if not ¡°Next¡± will be show in the last panel.
		3)	_bIsLastPA
			The PA should know that it is the last one in work flow in the list.
		4)	_szLastBtName
			This string will be show in the button when the obligations are implmented to the last one, otherwise this button shows ¡°Next¡±.
		5)	The Parent window of the PA¡¯s
			If the PEP want to set a parent window for the dialog of PA, it can passed by this parameter.
		*/
		LONG DoEcnryption( HMODULE hMod ,
				   const PVOID _lpLogCtx,
				   const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
				   const BOOL _bIsLastPA = TRUE,
				   const wchar_t* _szLastBtName = L"OK",
				   const HWND _hParentWnd = NULL ) 
		{
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _szLastBtName ;
			PVOID doPAEncryption = NULL ;
			doPAEncryption = GetProcAddress(hMod,PA_INTERFACE_NAME);
			if( _hParentWnd == NULL )
			{
				GetParentWindow(   const_cast<HWND&>(_hParentWnd ) );
			}
			if(	  doPAEncryption != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType)doPAEncryption)( m_paParam, _hParentWnd ) ;
			}
			return	 statu ;
		}
		/*
		Do Encryption:
		1)	_lpLogCtx
			The File tagging and encryption will call the CALLBACK, this context will be used in the function of CALLBACK.
		2)	_iAction
			If current PA is the last one, the pep(WDE) should tell the PA. The PA will show the last button name in the end obligation in the PA.
			Such as:	¡°Send E-mail¡± in OE
			¡°OK¡± in WDE, if not ¡°Next¡± will be show in the last panel.
		3)	_bIsLastPA
			The PA should know that it is the last one in work flow in the list.
		4)	_szLastBtName
			This string will be show in the button when the obligations are implmented to the last one, otherwise this button shows ¡°Next¡±.
		5)	The Parent window of the PA¡¯s
			If the PEP want to set a parent window for the dialog of PA, it can passed by this parameter.
		*/
		LONG DoFileTagging( HMODULE hMod ,
				    const PVOID _lpLogCtx,
				    const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
				    const BOOL _bIsLastPA = FALSE,
				    const wchar_t* _strLastBtName = _T("OK"),
				    const HWND _hParentWnd = NULL ) 
		{
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _strLastBtName ;
			PVOID doPATagging = NULL ;
			doPATagging = GetProcAddress(hMod,PA_INTERFACE_NAME);
			if( _hParentWnd == NULL )
			{
				GetParentWindow(   const_cast<HWND&>(_hParentWnd) ) ;
			}
			if(	  doPATagging != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType)doPATagging)(	 m_paParam ,_hParentWnd) ;
			}
			return	 statu ;
		}
		/*
		_pszOrigName:
		[in]This file name( Full Path) is the same to the TargetFileName which is set in the follow function
		_bNeedReAttach:
		[out] It is flag, this file has been modified and generated a new file.For OE, it need to be reattch, for WDE, the source file need to be changed.
		_pszNewName:
		[out] If the file path (full path)has been changed in the PA. The new will be written to this buffer, else this string is empty. 
		*/
		LONG QueryRetName_bySrc( const wchar_t * i_pszSrcName,BOOL &_bNeedReAttach, wchar_t *o_pszRetName )
		{
			LONG lRet = 0 ;
			std::list< PABase::OBJECTINFO >::iterator itor = m_paParam.objList.begin() ;
			_bNeedReAttach = FALSE ;
			for( itor ; itor!= m_paParam.objList.end() ; itor ++ )
			{
				/* NULL source name get's frist (default) source information */
				if( i_pszSrcName == NULL || lstrcmpiW( (*itor).strSrcName.c_str(), i_pszSrcName ) == 0  )
				{
					if( ( *itor).bFileNameChanged == TRUE )
					{
						_bNeedReAttach = TRUE ;
						if( o_pszRetName )
						{
							/*
							Modified by chellee on 14/10/08 ;6:32
							mark Code:::wcscpy_s( o_pszRetName, MAX_PATH, (*itor).strRetName.c_str() )  ;
							*/
							::wcsncpy_s( o_pszRetName, MAX_PATH, (*itor).strRetName, _TRUNCATE )  ;
							//-----------------------------------------------------------------

						}
						break ;
					}
				}
			}
			return lRet ;
		};

		/*
	   Check if current pa is the last one.
	   */
		BOOL CheckIsLastPA(PA_Mngr::OBTYPE _obType )
		{
			BOOL bRet = TRUE ;
			switch(_obType)
			{
			case PA_Mngr::OB_ENC:
				break ;
			case PA_Mngr::OB_HDR:
				break ;
			case PA_Mngr::OB_TAG:
				{
					if( m_bHasEnc == TRUE )
					{
						bRet = FALSE ;
					}
				}
				break ;
			} ;
			return bRet ;
		};
	protected:
		/*
		Manage the temporary file
		*/
		INT CreateTempFolderForPA( const wchar_t *   i_pszFolderName= NULL ) 
		{
			INT iRet = 0 ;
			wchar_t szTempPath[MAX_PATH+1] = {0} ;
			iRet = (INT) GetTempPath( MAX_PATH, szTempPath ) ;
			if( iRet == 0 )
			{
				//Get Temp file path failure!
				iRet = -1 ;
				return iRet ;
			}
			if(	 i_pszFolderName )
			{
				_snwprintf_s(m_szTempFoder,_countof(m_szTempFoder), _TRUNCATE,L"%s%s",szTempPath,i_pszFolderName);
			}
			else
			{
				_snwprintf_s(m_szTempFoder,_countof(m_szTempFoder), _TRUNCATE,L"%s%s",szTempPath,PA_Mngr::PA_TEMP_FOLDER_NAME);
			}
			BOOL bRet = TRUE ;
			bRet = CreateDirectory(	 m_szTempFoder, NULL ) ;

			if( (bRet == FALSE )&&(GetLastError()!=ERROR_ALREADY_EXISTS) )
			{	
				::ZeroMemory(	 m_szTempFoder, (MAX_PATH+1)*sizeof(wchar_t) ) ;
				iRet = -1 ;
			}
			return iRet ;
		};
		VOID SetLogComtex( PVOID lpLogCtx )
		{
			m_paParam.lpLogCtx =	 lpLogCtx ;
			m_paParam.fLog = (PABase::LogFunc)	DoLog ;
		} ;

		VOID GetParentWindow(HWND &_parentWnd)
		{
			_parentWnd = ::GetForegroundWindow() ;
		}
	protected:
		PABase::PA_PARAM  m_paParam ;
		wchar_t m_szTempFoder[MAX_PATH+1] ;
		BOOL m_bHasTagging ;
		BOOL m_bHasEnc ;
		BOOL m_bHasHDR ;
		PVOID m_pGetCEString ;
	};
};
#endif
