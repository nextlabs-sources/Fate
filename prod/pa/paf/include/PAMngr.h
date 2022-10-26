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
#include <string>
#include <vector>
#include <map>

struct   EnumParam   
{   
	HWND    hMainWnd;   
	DWORD   dwProcessID;   
	DWORD   dwThreadID;   
	int		nType;//0: compare thread id, 1: compare process id.
};   



typedef struct tagHCADDTAGINFO
{
	std::vector<std::pair<std::wstring,std::wstring>> vecTagInfo;
	std::wstring strSrcPath;
	std::wstring strDstPath;
	int   AddTagStatus;
	std::wstring strLogID;
}HCADDTAGINFO,*PHCADDTAGINFO;

 

namespace PA_Mngr
{
/*
for the key of Obligation
*/
#define OBLIGATION_NAME_ID              L"CE_ATTR_OBLIGATION_NAME"
#define OBLIGATION_NUMVALUES_ID         L"CE_ATTR_OBLIGATION_NUMVALUES"
#define OBLIGATION_VALUE_ID             L"CE_ATTR_OBLIGATION_VALUE"

#define PA_INTERFACE_NAME		"DoPolicyAssistant"

#define PROCESS_EXPLORER		L"explorer.exe"
#define MAIN_WINDOW_EXPLORER    L"ExploreWClass"
#define MAIN_WINDOW_EXPLORER2    L"CabinetWClass"
#define PA_CLASS_NAME           L"NXTLABS_PA_WINDOW_CLASS"

	const	wchar_t PA_TEMP_FOLDER_NAME[] = L"PA\\"	 ;
	const	wchar_t PA_OBLIGATION_ENC[]	  =	L"PASSWORD_BASED_ENCRYPTION" ;
	const   wchar_t PA_OBLIGATION_ENC_CER[] = L"IDENTITY_BASED_ENCRYPTION"  ;


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
	typedef PABase::PA_STATUS ( WINAPI *DoPolicyAssistantType)(_In_ PABase::PA_PARAM &_iParam, _In_opt_ const HWND _hParentWnd, _In_ bool forceObligation) ;
	typedef PABase::PA_STATUS ( WINAPI *DoPolicyAssistantType_Tagging )(_In_ PABase::PA_PARAM &_iParam, _In_opt_ const HWND _hParentWnd, _In_ bool forceObligation,
		_Out_ wchar_t** pSrcBuf,_Out_ DWORD& dwSrcLen,
		_Out_ wchar_t** pDstBuf,_Out_ DWORD& dwDstLen);

	typedef PABase::PA_STATUS ( WINAPI *DoPolicyAssistantType_OE)(_In_ PABase::PA_PARAM &_iParam, _In_opt_ const HWND _hParentWnd, _In_ bool forceObligation,_Out_ std::vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo) ;
	typedef wchar_t* (WINAPIV* Get_CEStringType)( CEString strValue ) ;
	typedef PABase::PA_STATUS (WINAPI *DeleteLastModifyTimeTag)(const wchar_t* szFile);
	typedef PABase::PA_STATUS (WINAPI *TaggingOnFile)(const wchar_t* wszFileName, const std::vector<std::pair<std::wstring,std::wstring>>* pVecTags);
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
			m_bOnlyAcceptXHeader = FALSE;
			//logd(L"CPAMngr-------------------");
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
		bool CheckDoOverLay(nextlabs::Obligations obs)
		{
			bool bRet = false;
			const std::list<nextlabs::Obligation>& theObs = obs.GetObligations();
			if(theObs.empty())
			{
				return false;
			}			
			std::list<nextlabs::Obligation>::const_iterator  iter;
			for(iter = theObs.begin();iter != theObs.end();iter++)
			{
				if(0 == _wcsicmp(L"VIEW_OVERLAY", ((*iter).name).c_str()))
				{
					bRet = true;
					break;
				}
			}
			return bRet;
		}

		void SetOnlyAcceptXHeader( BOOL b ){ m_bOnlyAcceptXHeader = b; }

	public:
		bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName,nextlabs::Obligations obs) 
		{
			return SetObligations(strSrcFileName, TargetFileName, L"", obs);
		}

		bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName, const wchar_t* strDestFileName, nextlabs::Obligations obs) 
		{
			PABase::OBJECTINFO objInfo ;
			objInfo.strSrcName = TargetFileName ;
			objInfo.strDisplayName =strSrcFileName ;
			objInfo.strDestName =strDestFileName ;

			const std::list<nextlabs::Obligation>& theObs = obs.GetObligations();

			if(theObs.empty())
			{
				return false;
			}

			std::list<nextlabs::Obligation>::const_iterator  iter;
			for(iter = theObs.begin();iter != theObs.end();iter++)
			{
				PABase::OBLIGATION obInfo ;
				obInfo.strOBName = (*iter).name;
				if(!(*iter).options.empty())
				{
					std::list<nextlabs::ObligationOption>::const_iterator  iter_option;
					for(iter_option = (*iter).options.begin();iter_option != (*iter).options.end(); iter_option++)
					{
						PABase::ATTRIBUTE attrInfo ;
						attrInfo.strValue = (*iter_option).first;
						obInfo.attrList.push_back(attrInfo);
						attrInfo.strValue = (*iter_option).second;
						obInfo.attrList.push_back(attrInfo);
					}
				}
				objInfo.obList.push_back(obInfo);
			}
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
		}

		bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName, const CEAttributes *obligation) 
		{
			return SetObligations(strSrcFileName, TargetFileName, L"", obligation);
		}

		/*
		In the OE, the source file should be the Temporary file path...
		strSrcFileName:
			[in]It is the original full file path. For WDE: 
	    TargetFileName:
			[in]It is the full file path, which is the object done the obligation by the PA .
	    obligation:
			[in]This data is the obligation returned from The Policy Controller (PDP).
		*/
		bool SetObligations(const wchar_t *strSrcFileName, const wchar_t *TargetFileName, const wchar_t* strDestFileName, const CEAttributes *obligation) 
		{
			int i=0;
			PABase::OBJECTINFO objInfo ;
			objInfo.strSrcName = TargetFileName ;
			objInfo.strDisplayName =	strSrcFileName ;
			objInfo.strDestName =strDestFileName ;

			if(NULL== obligation || NULL==obligation->attrs||0==obligation->count) return false;

			// The first key must be the count of obligation
			if(NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[0].key) || NULL==((Get_CEStringType)m_pGetCEString)(obligation->attrs[0].value)) return false;

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
					if(!IsObligationAccepted(value))
					{
						//logd(L"[SetObligations]OnlyAcceptXHeader=%d, \"%s\" is unaccepted ", m_bOnlyAcceptXHeader, value.c_str());
						continue;
					}else 
					{
						//logd(L"[SetObligations]OnlyAcceptXHeader=%d, \"%s\" is accepted", m_bOnlyAcceptXHeader, value.c_str());
					}

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
							std::wstring name3(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].key));
							if(0==wcsncmp(OBLIGATION_VALUE_ID, name3.c_str(), wcslen(OBLIGATION_VALUE_ID)))
							{
								PABase::ATTRIBUTE attrInfo ;
								obInfo.attrList.push_back( attrInfo ) ;
								i++ ;
								continue ;
							}
						}
						std::wstring name2(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].key));
						std::wstring value2(((Get_CEStringType)m_pGetCEString)(obligation->attrs[j].value));
						//DP((L"Obligations: [%s], <%s>\n", name.c_str(), value.c_str()));
						if(0==wcsncmp(OBLIGATION_NAME_ID, name2.c_str(), wcslen(OBLIGATION_NAME_ID)))
						{
							break ;
						}
						i++ ;
						if(0==wcsncmp(OBLIGATION_NUMVALUES_ID, name2.c_str(), wcslen(OBLIGATION_NUMVALUES_ID)))
						{
							continue ;
						}
						if(0==wcsncmp(OBLIGATION_VALUE_ID, name2.c_str(), wcslen(OBLIGATION_VALUE_ID)))
						{
							PABase::ATTRIBUTE attrInfo ;
							attrInfo.strValue = value2.c_str() ;
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
		
		INT UpdateAttachFileName( const wchar_t *_pszOrigName, const wchar_t *pszCurrentTempFile,const wchar_t* _pszNewName )
		{
			INT iRet = 0 ;
			std::list< PABase::OBJECTINFO >::iterator itor = m_paParam.objList.begin() ;
			for( itor ; itor!= m_paParam.objList.end() ; itor ++ )
			{
				if ((*itor).bFileNameChanged)
				{
					if( lstrcmpiW( (*itor).strRetName, pszCurrentTempFile ) == 0  )
					{
						(*itor).strSrcName =   _pszNewName ;
						::wcsncpy_s( (*itor).strRetName, MAX_PATH, _pszNewName, _TRUNCATE )  ;
						break ;
					}
				}
				else
				{
					if( lstrcmpiW( (*itor).strSrcName.c_str(), _pszOrigName ) == 0  )
					{
						(*itor).strSrcName =   _pszNewName ;
						::wcsncpy_s( (*itor).strRetName, MAX_PATH, _pszNewName, _TRUNCATE )  ;
						(*itor).bFileNameChanged =  TRUE ;
						break ;
					}
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
                    wcsncpy_s(m_szTempFoder,_countof(m_szTempFoder),i_tempFolder, _TRUNCATE);
				}
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
            UNREFERENCED_PARAMETER(wstrAssistantUserActions);
            UNREFERENCED_PARAMETER(wstrAssistantDescription);
            UNREFERENCED_PARAMETER(wstrAssistantOptions);
            UNREFERENCED_PARAMETER(wstrAssistantName);
            UNREFERENCED_PARAMETER(wstrlogIdentifier);
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

		LONG  DeleteLastModifyTimeTag( HMODULE hMod, const wchar_t* wszFile) 
		{
			
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			
			PVOID doPADelLastModifyTimeTag = NULL ;
			doPADelLastModifyTimeTag = GetProcAddress(hMod, "DeleteLastModifyTimeTag");
			if(	  doPADelLastModifyTimeTag != NULL  )
			{
				statu = ((PA_Mngr::DeleteLastModifyTimeTag)doPADelLastModifyTimeTag)(wszFile) ;
			}
			return	 statu ;
		}

		PABase::PA_STATUS TaggingOnFile(HMODULE hMod,const wchar_t* wszFileName, const std::vector<std::pair<std::wstring,std::wstring>>* pVecTags)
		{
			PABase::PA_STATUS statu = PA_SUCCESS ; 

			PVOID doTaggingOnFile = NULL ;
			doTaggingOnFile = GetProcAddress(hMod, "TaggingOnFile");
			if(	doTaggingOnFile != NULL )
			{
				statu = ((PA_Mngr::TaggingOnFile)doTaggingOnFile)(wszFileName, pVecTags) ;
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
		LONG DoEcnryption( HMODULE hMod ,
				   const PVOID _lpLogCtx,
				   const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
				   const BOOL _bIsLastPA = TRUE,
				   const wchar_t* _szLastBtName = L"OK",
				   const HWND _hParentWnd = NULL,
				   bool forceObligation= false) 
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
				statu = ((PA_Mngr::DoPolicyAssistantType)doPAEncryption)( m_paParam, _hParentWnd, forceObligation ) ;
			}
			return	 statu ;
		}
#if 1
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
				    const HWND _hParentWnd = NULL,
					bool forceObligation = false) 
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
			if( _hParentWnd == NULL )
			{
				statu = -1;
			}
			if(	  doPATagging != NULL && _hParentWnd != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType)doPATagging)(	 m_paParam ,_hParentWnd, forceObligation) ;
			}
			return	 statu ;
		}


#define PA_INTERFACE_TAGGING_OE	"DoPolicyAssistant_OE"

		LONG DoFileTagging_OE( HMODULE hMod ,
			const PVOID _lpLogCtx,
			const PABase::ACTION _iAction,
			const BOOL _bIsLastPA,
			const wchar_t* _strLastBtName,
			const HWND _hParentWnd,
			bool forceObligation,
			std::vector<std::tr1::shared_ptr<HCADDTAGINFO>> *pVecHCInfo) 
		{
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _strLastBtName ;
			PVOID doPATagging = NULL ;
			doPATagging = GetProcAddress(hMod,PA_INTERFACE_TAGGING_OE);
			if( _hParentWnd == NULL )
			{
				GetParentWindow(   const_cast<HWND&>(_hParentWnd) ) ;
			}
			if( _hParentWnd == NULL )
			{
				statu = -1;
			}
			if(	  doPATagging != NULL && _hParentWnd != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType_OE)doPATagging)(	 m_paParam ,_hParentWnd, forceObligation,pVecHCInfo) ;
			}
			return	 statu ;
		}
		/*
		*\ change for get the tag value that user select for interactive tag obligation.
		*	didn't add tag at all.
		*/
		LONG DoFileTagging2( HMODULE hMod ,
				    const PVOID _lpLogCtx,
					_Out_ wchar_t** pSrcBuf,_Out_ DWORD& dwSrcLen,
					_Out_ wchar_t** pDstBuf,_Out_ DWORD& dwDstLen,
				    const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
				    const BOOL _bIsLastPA = FALSE,
				    const wchar_t* _strLastBtName = _T("OK"),
				    const HWND _hParentWnd = NULL,
					bool forceObligation = false)
		{
#define PA_INTERFACE_TAGGING_2	"DoPolicyAssistant2"
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _strLastBtName ;
			PVOID doPATagging = NULL ;
			doPATagging = GetProcAddress(hMod,PA_INTERFACE_TAGGING_2);
			if( _hParentWnd == NULL )
			{
				GetParentWindow(   const_cast<HWND&>(_hParentWnd) ) ;
			}
			if( _hParentWnd == NULL )
			{
				statu = -1;
			}
			if(	  doPATagging != NULL && _hParentWnd != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType_Tagging)doPATagging)(	 m_paParam ,_hParentWnd, forceObligation,
					pSrcBuf,dwSrcLen,pDstBuf,dwDstLen) ;
			}
			return	 statu ;
		}
			/*
		Do Visual Labeling:
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
		LONG DoVisualLabeling( HMODULE hMod ,
			const PVOID _lpLogCtx,
			const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
			const BOOL _bIsLastPA = FALSE,
			const wchar_t* _strLastBtName = _T("OK"),
			const HWND _hParentWnd = NULL, 
			bool forceObligation = false ) 
		{
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _strLastBtName ;
			PVOID doVisualLabeling = NULL ;
			doVisualLabeling = GetProcAddress(hMod,PA_INTERFACE_NAME);
			if( _hParentWnd == NULL )
			{
				GetParentWindow(   const_cast<HWND&>(_hParentWnd) ) ;
			}
			if( _hParentWnd == NULL )
			{
				statu = -1;
			}
			if(	  doVisualLabeling != NULL && _hParentWnd != NULL )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType)doVisualLabeling)(	 m_paParam ,_hParentWnd, forceObligation) ;
			}
			return	 statu ;
		}
#endif
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
		LONG DoPortableEncryption( HMODULE hMod ,
				    const PVOID _lpLogCtx,
				    const PABase::ACTION _iAction= PABase::AT_SENDMAIL ,
					const bool forceObligation = FALSE,
				    const BOOL _bIsLastPA = FALSE,
				    const wchar_t* _strLastBtName = _T("OK"),
				    const HWND _hParentWnd = NULL) 
		{
			(void)_hParentWnd;
			PABase::PA_STATUS statu = PA_SUCCESS ; 
			SetLogComtex( _lpLogCtx ) ;
			m_paParam._action  =	 _iAction ;
			m_paParam._bIsLastPA =	 _bIsLastPA;
			m_paParam._strLastButtonName =  _strLastBtName ;
			PVOID doPAPortableEncrypt = NULL ;
			doPAPortableEncrypt = GetProcAddress(hMod,PA_INTERFACE_NAME);
			if(	  doPAPortableEncrypt != NULL  )
			{
				statu = ((PA_Mngr::DoPolicyAssistantType)doPAPortableEncrypt)(	 m_paParam ,NULL, forceObligation) ;
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
							mark Code:::wcsncpy_s( o_pszRetName, MAX_PATH, (*itor).strRetName, _TRUNCATE )  ;
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

		static BOOL IsIndicatedProcess(LPCWSTR pszProcessName)
		{
			if(!pszProcessName)
				return FALSE;

			wchar_t szPath[MAX_PATH * 2] = {0};
			GetModuleFileNameW(NULL, szPath, MAX_PATH);
			wchar_t* p = wcsrchr(szPath, '\\');
			if(p && _wcsicmp(p + 1, pszProcessName) == 0)
				return TRUE;

			return !(_wcsicmp(szPath, pszProcessName));
			
		}
		static BOOL IsExplorerWindow(HWND hWnd)
		{
			wchar_t szClassName[101] = {0};
			::GetClassName(hWnd, szClassName, 100);
			if(_wcsicmp(szClassName, MAIN_WINDOW_EXPLORER) == 0 || _wcsicmp(szClassName, MAIN_WINDOW_EXPLORER2) == 0)
			{   
				HWND   hwndAfter   =   NULL;   
				while(NULL != (hwndAfter=::FindWindowEx(NULL,hwndAfter,PA_CLASS_NAME,NULL)))   
				{

					if(::GetParent(hwndAfter) == hWnd)
					{
						return FALSE;
					}

				}

				return   TRUE;   
			}
			
			return FALSE;
		}

		static BOOL   CALLBACK   EnumWinProc(HWND   hwnd,   LPARAM   lParam)   
		{   
			DWORD   dwProcessID;
			DWORD   dwThreadID;

			EnumParam*   pep   =   (EnumParam*)lParam;  
			if(!pep)
				return TRUE;

			wchar_t szClassName[MAX_PATH] = { 0 };
			::GetClassName(hwnd, szClassName, MAX_PATH);
			
			if (0 == wcscmp(szClassName, L"WaterMarkWindows") || 0 == wcscmp(szClassName, L"#32770"))
			{
				return TRUE;
			}

			dwThreadID = GetWindowThreadProcessId(hwnd,   &dwProcessID);   

			BOOL bRet = FALSE;
			if(pep->nType == 0)
			{
				bRet = (dwThreadID   ==   pep->dwThreadID && ::GetParent(hwnd) == NULL);
			}
			else if(pep->nType == 1)
			{
				bRet = (dwProcessID   ==   pep->dwProcessID && ::GetParent(hwnd) == NULL);
			}
            else
            {
                // Do nothing
                ;
            }

			if(bRet)
			{
				if(IsIndicatedProcess(PROCESS_EXPLORER))
				{
					BOOL bSucceed = IsExplorerWindow(hwnd);
					if(bSucceed)
					{
						pep->hMainWnd   =   hwnd;     
						return   FALSE;  
					}
				}
				else
				{
					pep->hMainWnd   =   hwnd;     
					return   FALSE; 
				}
			}
			
			return   TRUE;   
		} 

		VOID GetParentWindow(HWND &_parentWnd)
		{
// 			DWORD dwThreadID = GetCurrentThreadId();
// 
 			EnumParam   ep;   
 			ep.hMainWnd = NULL;
// 			ep.dwThreadID = dwThreadID;  
// 			ep.nType = 0;
// 
// 			EnumWindows((WNDENUMPROC)EnumWinProc,   
// 				(long)&ep);   
// 
// 			_parentWnd = ep.hMainWnd;
// 
// 			if(!_parentWnd)
// 			{
				ep.dwProcessID = GetCurrentProcessId();
				ep.nType = 1;
				EnumWindows((WNDENUMPROC)EnumWinProc,   
					(LPARAM)&ep);  
			
				_parentWnd = ep.hMainWnd;

				if(!_parentWnd)
				{
					_parentWnd = ::GetForegroundWindow() ;
					if(!_parentWnd)
					{
						_parentWnd = ::GetDesktopWindow();
					}

				}
// 			}
// 			else
// 				OutputDebugStringW(L"Succeed to get the current working window handle with current thread ID\r\n");
			
			wchar_t szClassName[51] = {0};
			::GetClassName(_parentWnd, szClassName, 50);
		}

		// if name is XHEADER_HIERARCHICAL_CLASSIFICATION, it will be changed to OE_HIERARCHICAL_CLASSIFICATION
		BOOL IsObligationAccepted(std::wstring& name)
		{
			int nRetOfHeaderHC = 1;
			if(0 == name.compare(L"AUTOMATIC_XHEADER_TAGGING") || 0 == (nRetOfHeaderHC = name.compare(L"XHEADER_HIERARCHICAL_CLASSIFICATION")))
			{
				if (m_bOnlyAcceptXHeader && 0 == nRetOfHeaderHC)
				{
					name = L"OE_HIERARCHICAL_CLASSIFICATION";
					return TRUE;
				}else
				{
					return FALSE;
				}
			}else if(m_bOnlyAcceptXHeader)
			{
				return FALSE;
			}
			return TRUE;
		}

	protected:
		PABase::PA_PARAM  m_paParam ;
		wchar_t m_szTempFoder[MAX_PATH+1] ;
		BOOL m_bHasTagging ;
		BOOL m_bHasEnc ;
		BOOL m_bHasHDR ;
		PVOID m_pGetCEString ;
		BOOL m_bOnlyAcceptXHeader;
	};
};
#endif
