#include "stdafx.h"
#include "dllmain.h"
#include "utils.h"
#include "NLOfficePEP_Comm.h"
#include "NLObMgr.h"
#include "NLHookApi.h"
#include "NLInsertAndDataAction.h"
#include "obligations.h"
#include "TalkWithSCE.h"
#include "dllmain.h"
#include "NLProcess.h"
////////////////////////////CELog2//////////////////////////////////////////////
// for CELog2 we should define: CELOG_CUR_FILE
#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLPROCESS)

extern BOOL g_bSavePressed;
//////////////////////////////for excel com interface////////////////////////////////////////////
// for excel COM interface, tlb/tlh/tli when we process excel insert picture we need use this
#ifndef MSO2K3
#pragma implementation_key(5773)

inline Excel::excelShapePtr Excel::excelShapes::AddPicture (
	_bstr_t FileName,
	enum MsoTriState LinkToFile,
	enum MsoTriState SaveWithDocument,
	float Left,
	float Top,
	float Width,
	float Height )
{
	struct excelShape * _result = 0;
	_com_dispatch_method(this, 0x6bb, DISPATCH_METHOD, VT_DISPATCH, (void*)&_result, 
		L"\x0008\x0003\x0003\x0004\x0004\x0004\x0004", (BSTR)FileName, LinkToFile, SaveWithDocument, Left, Top, Width, Height);
	return excelShapePtr(_result, false);
}

#pragma implementation_key(5684)
inline HRESULT Excel::excelShape::ScaleHeight ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x694, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#pragma implementation_key(5685)
inline HRESULT Excel::excelShape::ScaleWidth ( float Factor, enum Office::MsoTriState RelativeToOriginalSize, const _variant_t & Scale ) {
	return _com_dispatch_method(this, 0x698, DISPATCH_METHOD, VT_EMPTY, NULL, 
	L"\x0004\x0003\x080c", Factor, RelativeToOriginalSize, &Scale);
}

#endif

CNLProcess::CNLProcess( )
{
	InitializeCriticalSection(&m_csMapPassiveFlag);

	HKEY hKey = NULL;
	LONG lResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Palisade", 0, KEY_QUERY_VALUE, &hKey);
	if (ERROR_SUCCESS == lResult)
	{
		WCHAR szData[MAX_PATH] = { 0 };
		DWORD cbData = MAX_PATH * sizeof(WCHAR);
		lResult = RegQueryValueExW(hKey, L"Main Directory", NULL, NULL, (LPBYTE)szData, &cbData);
		if (ERROR_SUCCESS == lResult)
		{
			m_wstrPalisadePath = szData;
		}
		RegCloseKey(hKey);
	}
}

CNLProcess::~CNLProcess(void)
{
	DeleteCriticalSection(&m_csMapPassiveFlag);
}

void CNLProcess::OnWndActive( _In_ IDispatch* pDoc )
{
	// cache active file path and pDoc;
	wstring wstrCurActiveFilePath = NLUpdateCurActiveFile( pDoc );

	// check if it is after save as action
	if ( !wstrCurActiveFilePath.empty() )
	{
		CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

		// check if it is save as action, source file close
		wstring wstrSrcFilePath;
		if ( theObMgrIns.NLGetSaveAsFlag( wstrCurActiveFilePath, wstrSrcFilePath ) )
		{
			NLPRINT_DEBUGLOG( L" ######## Active Copy end: source file path:[%s], des:[%s] -- \n", wstrSrcFilePath.c_str(), wstrCurActiveFilePath.c_str() );
			theObMgrIns.NLSetSaveAsFlag( wstrCurActiveFilePath, false, wstrSrcFilePath );
			// About source SE file edit --> save as destination, all the SE tag will be synchronized when the DLL detach.
		}
	}
}

wstring CNLProcess::NLUpdateCurActiveFile( _In_ IDispatch* pDoc )
{
	CComPtr<IDispatch> pCurDoc = getActiveDoc();
	if ( NULL == pCurDoc )
	{
		pCurDoc = pDoc;
	}
	
	wstring wstrCurFilePath;

	if (!getDocumentPath(wstrCurFilePath, pCurDoc))
	{
		return std::wstring();
	}

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

	// judge if it is open in IE¡¢protect module ...
	if (isOpenInIEFlag(wstrCurFilePath))
	{
		wstrCurFilePath = theObMgrIns.NLGetIEFilePathCache(pCurDoc);
		if (wstrCurFilePath.empty())
		{
			// here means the active pDoc has changed, 
			// maybe we need to update the pDoc and the IE cache
			// "Note: for word/excel open in IE, 
			// here the active pDoc changed and maybe we need update the IE file cache here" 
			NLPRINT_DEBUGLOG(L"!!!!!!!Error: get current active file path failed, the current active pDoc changed, please check if need update the IE file path cache \n");
			return L"";
		}
	}
	else
	{
		theObMgrIns.NLSetCurActiveFilePath(wstrCurFilePath);
		theObMgrIns.NLSetFilePDocCache(wstrCurFilePath, pCurDoc);
	}

	// if no initialize, initialize it.
	if (!theObMgrIns.NLGetObMgrInitializeFlag(wstrCurFilePath))
	{
		theObMgrIns.NLInitializeObMgr(wstrCurFilePath, pCurDoc);
	}

	if (!NLGetPassiveFlag(wstrCurFilePath))
	{
		NLStartUpPassiveAction(wstrCurFilePath);
	}

	NLPRINT_DEBUGLOG(L"the current active file path is:[%s] \n", wstrCurFilePath.c_str());


	return wstrCurFilePath;
}

void CNLProcess::NLSetPassiveFlag( _In_ const wstring& wstrFilePath, _In_ const bool& bPassiveFlag )
{
	EnterCriticalSection(&m_csMapPassiveFlag);
	m_mapPassiveFlag[wstrFilePath] = bPassiveFlag;
	LeaveCriticalSection(&m_csMapPassiveFlag);
}

bool CNLProcess::NLGetPassiveFlag( _In_ const wstring& wstrFilePath )
{
	bool bPassiveFlag = false;
	EnterCriticalSection(&m_csMapPassiveFlag);
	map<wstring, bool>::iterator itr = m_mapPassiveFlag.find( wstrFilePath );
	if ( itr != m_mapPassiveFlag.end() )
	{
		bPassiveFlag = m_mapPassiveFlag[wstrFilePath];
	}
	LeaveCriticalSection(&m_csMapPassiveFlag);
	return bPassiveFlag;
}

void CNLProcess::NLProcessClassifyAction( _In_ const wstring& wstrFilePath )
{
	/* About classify action:
	*		1. deny or allow user change the custom tags, it is classify action, this is a passive actions
	*		2. for Raytheon rights protect, ribbon button click.
	*/
	// hook API "EnableWindow" to prevent the user to change the custom tags
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	theObMgrIns.NLSetActionStartUpFlag( wstrFilePath, kOA_CLASSIFY, true );
}

ProcessResult CNLProcess::NLProcessProtectedViewAction(_In_ IDispatch* pDoc, _In_ const wstring& wstrPath,_In_ IDispatch* pDispProtectedView)
{
	ProcessResult stuResult =NLOBMGRINSTANCE.NLProcessActionProtectedView(wstrPath,pDoc);
	if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
	{
		stuResult.vbCancel = VARIANT_TRUE;
		stuResult.kFuncStat = CloseProtectedView( pDispProtectedView) ? kFSSuccess : kFSFailed;
	}

	return stuResult;
}

ProcessResult CNLProcess::OnOpen( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{   
	ProcessResult rt = NLOBMGRINSTANCE.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_OPEN );
	if ( kFSSuccess != rt.kFuncStat || kPSDeny == rt.kPolicyStat )
	{
		rt.vbCancel = VARIANT_TRUE;
		rt.kFuncStat = NLCloseDocInOpenAction( pDoc, wstrFilePath ) ? kFSSuccess : kFSFailed;
	}
	return rt;
}

ProcessResult CNLProcess::OnEdit( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_EDIT, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );

	if ( isNewOfficeFile( wstrFilePath ) )
 	{
		// this is save as action
		stuResult = OnCopy( pDoc, wstrFilePath, kFileUnknown );
 	} 
 	else
 	{
		/*
			here we check if the document has been edited
			Sometime we change the file format and save it this is not EDIT action but the file changed.
			The office default save work flow doing.
			For SE file, we need encrypt before it save otherwise the SE file will be become non-encrypt.
		*/
		VARIANT_BOOL bSaved = VARIANT_FALSE;
		HRESULT hr = S_FALSE;
		switch ( pep::appType() )
		{
		case kAppWord:  
			{
				Word::_DocumentPtr ptrDoc(pDoc);
				hr = ptrDoc->get_Saved(&bSaved);
			}
			break;
		case kAppExcel:
			{
				// Excel: the saved status sometimes is wrong
			}
			break;
		case kAppPPT:
			{
				// PPT: the saved status sometimes is wrong
			}
			break;
		default:
			break;
		}
		
		CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
		NLPRINT_DEBUGLOG( L"be save:[%s] \n", VARIANT_FALSE == bSaved ? L"false" : L"true" );
		if ( SUCCEEDED(hr) && VARIANT_FALSE == bSaved )
		{
			stuResult = theObMgrIns.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_EDIT );
			if ( kFSFailed != stuResult.kFuncStat && kPSDeny != stuResult.kPolicyStat )
			{
				NLPRINT_DEBUGLOG( L"------------- success do edit save ------------- \n" );
				// edit save file success
				if ( theObMgrIns.NLGetClassifyCustomTagsFlag( wstrFilePath ) )
				{
					NLPRINT_DEBUGLOG( L"---- set cache initialize false ----\n" );
					theObMgrIns.NLSetObMgrInitializeFlag( wstrFilePath, false );
					theObMgrIns.NLSetClassifyCustomTagsFlag( wstrFilePath, false );
				}
			}
		}
 	}
	return stuResult;
}

ProcessResult CNLProcess::OnCopy( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath, _In_ const OfficeFile& emSaveAsType )
{
	// Copy action we need get the destination file path and judge the action: COPY or CONVERT action
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls, emSaveAsType=%d \n", pDoc,wstrFilePath.c_str(),emSaveAsType);
	ProcessResult result( kFSSuccess, kPSAllow, kOA_COPY, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
		
	// PPT save as action will triggered edit action
	if (pep::isPPtApp())
	{
		theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true );
	}
	
	// Hook API "ShowWindow" and get the destination file path
	NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d], variant cancel:[%d], function:[%s],line:[%d] \n", 
		result.kFuncStat, result.kPolicyStat,
		result.kAction, result.vbCancel, result.fname.c_str(), result.line);
	return result;
}

ProcessResult CNLProcess::OnPaste(  _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	return NLOBMGRINSTANCE.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_PASTE );
}

ProcessResult CNLProcess::OnPrint( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	return NLOBMGRINSTANCE.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_PRINT );
}

ProcessResult CNLProcess::OnSend( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	return NLOBMGRINSTANCE.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_SEND );
}

ProcessResult CNLProcess::OnInsert( _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, emInsertType=%d \n",wstrFilePath.c_str(),emInsertType);
	ProcessResult stuResult(  kFSUnknown, kPSUnknown,  kOA_INSERT, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	switch ( pep::appType() )
	{
	case kAppWord:  		
		// word insert we pop up our insert dialog to do this and need prevent the default insert dialog
		stuResult = OnWordInsert( emInsertType );
		//NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d],function:[%s],line:[%d] \n", stuResult.kFuncStat, stuResult.kPolicyStat,stuResult.kAction, stuResult.fname.c_str(), stuResult.line);
		break;
	case kAppExcel:		
		// excel we hook UI to get the file path
		stuResult = OnExcelInsert( wstrFilePath, emInsertType );		
		break;
	case kAppPPT:		
		//PPT we hook UI to get the file path
		stuResult = OnPPTInsert( wstrFilePath, emInsertType );		
		break;
	default:
		break;
	}
	return stuResult;
}

ProcessResult CNLProcess::OnConvert( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	return NLOBMGRINSTANCE.NLProcessActionCommon( pDoc, wstrFilePath.c_str(), NULL, kOA_CONVERT );
}

ProcessResult CNLProcess::OnClose( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	//ONLY_DEBUG
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, wstrFilePath=%ls \n", pDoc,wstrFilePath.c_str());
	ProcessResult stuResult(  kFSSuccess, kPSAllow,  kOA_CLOSE, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	bool bNeedClose = false;
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wchar_t wszMessage[1024] = { 0 };

	// check if there is save requirement and we should synchronize the tags
	switch ( pep::appType() )
	{
	case kAppWord:  
		{
			if (isNewOfficeFile(wstrFilePath))
			{
				return stuResult;
			}
			/*
			*	for word we get the close event notify before the save prompt dialog
			*/
			bNeedClose = NLCheckBeforeWordClose( pDoc, wstrFilePath, stuResult.vbCancel );
		}
		break;
	case kAppExcel:
		{
			bool bDeleteBuffer = true;

			if ( isNewOfficeFile( wstrFilePath ) )
			{
				return stuResult;
			}
			
			/* 
			* FixBug24714 judge template file, for template file, here we just can get the file name, without file path
			*/
			if ( isTemplateFile( wstrFilePath, false, false ) )
			{
				NLPRINT_DEBUGLOGEX( true, L"May be this is the template file:[%s]", wstrFilePath.c_str() );
				return stuResult;
			}

			VARIANT_BOOL bSaved = VARIANT_FALSE;

			Excel::_WorkbookPtr ptrDoc(pDoc);
			HRESULT hr = ptrDoc->get_Saved(0, &bSaved);

			LPWSTR wszCmd = GetCommandLine();
			bool bIgnoreSave = boost::algorithm::iends_with( wszCmd, L"/Automation -Embedding" );


			if (SUCCEEDED(hr) && !bSaved && !bIgnoreSave)
			{
				//fix bug 33117
				wchar_t wszFUNCRESPath[1024]={0};
				::GetModuleFileNameW( NULL, wszFUNCRESPath, 1024 );

				wchar_t* pLastBackslash = wcsrchr(wszFUNCRESPath, L'\\');
				if (pLastBackslash != NULL)
				{
					*pLastBackslash = L'\0';
				}

				wcscat_s(wszFUNCRESPath, L"\\Library\\Analysis\\FUNCRES.XLAM");

				bool bNeedSave = true;

				if (boost::algorithm::equals(wszFUNCRESPath, wstrFilePath))
				{
					bNeedSave = false;
				}
				else
				{
					if (!m_wstrPalisadePath.empty())
					{
						if (boost::algorithm::istarts_with(wstrFilePath, m_wstrPalisadePath) && (boost::algorithm::iends_with(wstrFilePath, L".xla") || boost::algorithm::iends_with(wstrFilePath, L".xlam")))
						{
							bNeedSave = false;
						}
					}
				}

				if (bNeedSave)
				{
					StringCchPrintf(wszMessage, 1023, L"Do you want to save the changes to %s?", GetFileName(wstrFilePath).c_str());	
					int nRet = MessageBox(NULL,  wszMessage, L"Microsoft Excel", MB_YESNOCANCEL | MB_ICONQUESTION);
					if(nRet == IDYES)
					{	
						stuResult = OnEdit( pDoc,wstrFilePath );
						if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
						{
							ptrDoc->put_Saved( 0, VARIANT_TRUE );
						}
						else
						{
							theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true );
							hr = ptrDoc->Save();
							bDeleteBuffer = SUCCEEDED(hr);
							theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, false );
						}
					}
					else if(nRet == IDNO)
					{
						ptrDoc->put_Saved(0, VARIANT_TRUE);
					}
					else
					{
						// cancel close and return;
						stuResult.vbCancel = VARIANT_TRUE;

						bDeleteBuffer = false;
					}
				}
			}
			
			if ( bDeleteBuffer )
			{
				TalkWithSCE::GetInstance().CacheOpenedFile(wstrFilePath, EF_SUB, TAG(0));
			}
		}
		break;
	case kAppPPT:
		{
			if ( isNewOfficeFile( wstrFilePath ) )
			{
				return stuResult;
			}
			
			bool bDeleteBuffer = true;

			PPT::_PresentationPtr ptrDoc(pDoc);
			MsoTriState saveStatus;
			HRESULT hr = ptrDoc->get_Saved(&saveStatus);
			if(SUCCEEDED(hr))
			{
				if(saveStatus == msoFalse)
				{	
					StringCchPrintf(wszMessage, 1023, L"Do you want to save the changes you made to\n%s?", GetFileName(wstrFilePath).c_str());	
					int nRet = MessageBox(NULL, wszMessage, L"Microsoft PowerPoint", MB_YESNOCANCEL | MB_ICONQUESTION);
					if( nRet == IDYES )
					{		
						stuResult = OnEdit( pDoc,wstrFilePath );
						if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
						{
							ptrDoc->put_Saved( msoTrue );
						}
						else
						{
							theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true );
							hr = ptrDoc->Save();
							bDeleteBuffer = SUCCEEDED(hr);
							theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, false );
						}
					}
					else if( nRet == IDNO )
					{
						ptrDoc->put_Saved(msoTrue);
					}
					else
					{
						// cancel close and return;
						stuResult.vbCancel = VARIANT_TRUE;

						// PPT need hook ShowWindown to deny dialog
						theObMgrIns.NLSetPPTUserCancelCloseFlag( wstrFilePath, true );
						
						bDeleteBuffer = false;
					}
				}
			}
			
			if ( bDeleteBuffer )
			{
				TalkWithSCE::GetInstance().CacheOpenedFile(wstrFilePath, EF_SUB, TAG(0));
			}
		}
		break;
	default:
		break;
	}
	if ( bNeedClose )
	{
		// clear cache
		theObMgrIns.NLClearFileCache( wstrFilePath );

		NLPRINT_DEBUGLOG( L"---- Close action after clear data cache \n" );
	}

	return stuResult;
}

//////////////////////////////trigger the event by our pep////////////////////////////////////////////
// check if there is close requirement
bool CNLProcess::NLCheckBeforeWordClose( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath, _Out_ VARIANT_BOOL& bVariantCancel )
{
	bool bNeedClose = true;
	bVariantCancel = VARIANT_FALSE;
	
	// FixBug23988, for HDR
	LPWSTR wszCmd = GetCommandLine();
	bool bIgnoreSave = boost::algorithm::iends_with( wszCmd, L"/Automation -Embedding" );
	
	/* word open in IE, we get the close action after IE process.
	*  if the file edit and then close without save, IE will pop message box but we can get any process information in office pep
	*  after IE process the close we will get this close event and pop up the message box this is wrong.
	*  two message box will be pop up at this case when user choose "No" first on the message box which pop up by IE process.
	*/
//#pragma chMSG( "here will be a bug when word open in IE, edit and close without save, and user choose no first on IE message box " )

	// get save requirement flag
	VARIANT_BOOL bSaved = VARIANT_FALSE;
	Word::_DocumentPtr ptrDoc( pDoc );

	HRESULT hr = ptrDoc->get_Saved(&bSaved);
	
	if ( SUCCEEDED(hr) )
	{
		if ( bSaved == VARIANT_FALSE && !bIgnoreSave)
		{
			wchar_t wszLog[512] = { 0 };
			StringCchPrintf( wszLog, 511, L"Do you want to save the changes to %s?", GetFileName(wstrFilePath).c_str() );	
			int nRet = MessageBox( GetActiveWindow(),  wszLog, L"Microsoft Word", MB_YESNOCANCEL | MB_ICONQUESTION );

			if( IDYES == nRet )
			{
				ProcessResult stuResult = OnEdit( pDoc,wstrFilePath );
				if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
				{
					ptrDoc->put_Saved( VARIANT_TRUE );
				}
				else
				{
					// set save action
					CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
					theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, true );
					hr = ptrDoc->Save();
					bNeedClose = SUCCEEDED(hr);
					theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_EDIT, false );
				}
			}
			else if ( IDNO == nRet )
			{
				ptrDoc->put_Saved(VARIANT_TRUE);
			}
			else
			{
				// cancel close and return;
				bNeedClose = false;
				bVariantCancel = VARIANT_TRUE;
			}
		}
	}
	
	if ( bNeedClose )
	{
		TalkWithSCE::GetInstance().CacheOpenedFile( wstrFilePath, EF_SUB, TAG(0) );
	}
	return bNeedClose;
}

void CNLProcess::ProtectedViewWindowBeforeClose( _In_ IDispatch* pDisp, _In_ const wstring& wstrFilePath )
{
	if ( pDisp != NULL && !wstrFilePath.empty() )
	{
		TalkWithSCE::GetInstance().CacheOpenedFile(wstrFilePath, EF_SUB, TAG(0));
	}
}

void CNLProcess::NLStartUpPassiveAction( _In_ const wstring& wstrFilePath )
{
	NLProcessClassifyAction( wstrFilePath );
	NLSetPassiveFlag( wstrFilePath, true );
}

// this function only used to close the file when the open action is deny
bool CNLProcess::NLCloseDocInOpenAction( _In_ IDispatch* pDoc, _In_ const wstring& wstrFilePath )
{
	//ONLY_DEBUG
	/*
	*	here we should close this file but in the following case we can not close it directly.
	*		1. PPT open by another application
	*		2. Word open by hyper link
	*/
	
	bool bRet = true;
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	
	// check if it is PPT opened by other applications
	bool bClose = true;
	if ( pep::isPPtApp() && NLOpenedByOtherApp() )
	{
		NLPRINT_DEBUGLOGEX( true, L"this is ppt and openned by other application \n" );
		bRet = NLCloseDoc( pDoc, wstrFilePath );
		if ( !bRet )
		{
			NLPRINT_DEBUGLOGEX( true, L" Close failed, we set close flag \n" );
			theObMgrIns.NLSetCloseFlag( wstrFilePath, true );
		}
		bClose = false;
	}
	
	// check if it is word opened by hyper link, we hook
	wstring wstrLinkFile = theObMgrIns.NLGetLinkFilePath();
	if ( !wstrLinkFile.empty() && boost::algorithm::iends_with( wstrFilePath, wstrLinkFile ) )	
	{
		NLPRINT_DEBUGLOG( L"this is link file path \n" );
		if ( pep::isWordApp() )	// we set hook in PPT but check in word
		{
			theObMgrIns.NLSetCloseFlag( wstrFilePath, true );
		}
		bClose = false;
	}
	theObMgrIns.NLSetLinkFilePath( L"" );

	if ( bClose )	
	{
		bRet = NLCloseDoc( pDoc, wstrFilePath );
		if ( !bRet )
		{
			NLPRINT_DEBUGLOGEX( true, L" Close failed, we set close flag \n" );
			theObMgrIns.NLSetCloseFlag( wstrFilePath, true );
		}
	}

	NLPRINT_DEBUGLOGEX( true, L" Close file status:[%d, %s] \n", bRet, bRet ? L"true" : L"false" );
	return bRet;
}

bool CNLProcess::NLCloseDoc( _In_ IDispatch* pDispDoc, _In_ const wstring& wstrFilePath )
{
	//ONLY_DEBUG
	if ( NULL == pDispDoc )
	{
		NLPRINT_DEBUGLOG( L"we want to close the file by our pep but the pointer: pDispatch is NULL \n" );
		return false;
	}
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_CLOSE,  true );
	
	HRESULT hr = E_FAIL;
	
	switch ( pep::appType() )
	{
	case kAppWord:
		{
			Word::_DocumentPtr theDoc( pDispDoc );
			hr = theDoc->Close();
		}
		break;
	case kAppExcel:
		{
			Excel::_WorkbookPtr theDoc( pDispDoc );
			hr = theDoc->Close();
		}
		break;
	case kAppPPT:
		{
			PPT::_PresentationPtr theDoc( pDispDoc );
			hr = theDoc->Close();
		}
		break;
	default:
		break;
	}
	
	theObMgrIns.NLClearFileCache( wstrFilePath );
	theObMgrIns.NLSetNoNeedHandleFlag( wstrFilePath, kOA_CLOSE,  false );
	return SUCCEEDED( hr );
}

ProcessResult CNLProcess::OnWordInsert( _In_ const InsertType& emInsertType )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_INSERT, VARIANT_TRUE, __FUNCTIONW__, __LINE__ );
	Word::_wordApplicationPtr theApp( (IDispatch*)pep::getApp() );
	Word::DialogsPtr pDialogs = NULL;

	HRESULT hr = theApp->get_Dialogs( &pDialogs );
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	Word::DialogPtr pDlg =NULL;		
	switch ( emInsertType )
	{
	case kInsertObj:
		{
			hr = pDialogs->Item( wdDialogInsertObject, &pDlg );
			if ( FAILED(hr) )
			{
				stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
			}
		}
		break;
	case kInsertText_WordOnly:
		{
			hr = pDialogs->Item (wdDialogInsertFile,&pDlg);  
			if ( FAILED(hr) )
			{
				stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
			}
		}
		break;
	case kInsertPic:
		{
			hr = pDialogs->Item (wdDialogInsertPicture,&pDlg);
			if ( FAILED(hr) )
			{
				stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
			}
		}
		break;
	default:
		{
			stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		}
		break;
	}
	
	if ( kFSSuccess != stuResult.kFuncStat )
	{
		return stuResult;
	}
	
	long lRet = 0;
	CComVariant varTimeout(0);
	hr = pDlg->Display( &varTimeout, &lRet ); 
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	NLPRINT_DEBUGLOG( L"---display result [%d] --- \n", lRet );
	if ( lRet != -1 )
	{
		// user click cancel or insert an error type file( word insert pic can't convert ) will return 0
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	wstring wstrInsertPath;
	CComVariant varResult;
	
	if ( kInsertText_WordOnly == emInsertType )
	{
		Word::wordOptionsPtr opPtr;
		BSTR bstrFilePath = NULL;
		theApp->get_wordOptions( &opPtr );
		hr = opPtr->get_DefaultFilePath( wdDocumentsPath, &bstrFilePath );
		if ( SUCCEEDED(hr) && bstrFilePath != NULL )
		{
			wstrInsertPath = bstrFilePath;
			if ( *( wstrInsertPath.c_str() + wstrInsertPath.length()-1 ) != L'\\' )
			{
				wstrInsertPath += L"\\";
			}
			// free resource
			SysFreeString(bstrFilePath);
			bstrFilePath = NULL;
		}
	}
	
	if ( kInsertObj == emInsertType )
	{
		hr = AutoWrap( DISPATCH_PROPERTYGET, &varResult, pDlg, L"FileName", 0 );
	}
	else 
	{
		hr = AutoWrap( DISPATCH_PROPERTYGET, &varResult, pDlg, L"Name", 0 );
	}

	if ( SUCCEEDED(hr) && varResult.bstrVal != NULL )
	{
        std::wstring wstrResult(varResult.bstrVal); 
        if (boost::icontains(wstrResult.c_str(),L"\""))
        {
            boost::ireplace_all(wstrResult, L"\"", L"");
        }
        
        // insert from sharePoint
        if ( boost::istarts_with(wstrResult.c_str(), GetUserAppDataPath() + L"\\Local\\Microsoft\\Windows\\INetCache\\Content.MSO") ||  // for win10
             boost::istarts_with(wstrResult.c_str(), GetUserAppDataPath() + L"\\Local\\Microsoft\\Windows\\Temporary Internet Files\\Content.MSO" ))  // for win7
        {
            wstrInsertPath = g_cacheFilePath;
        } 
        else
        {
            wstrInsertPath += wstrResult; 
        }
       
	}
	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrInsertPath.c_str(), NULL, kOA_INSERT );
	stuResult.vbCancel = VARIANT_TRUE;
	
	if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )	
	{
		if ( kInsertObj == emInsertType )
		{
			HWND hObject = FindWindow( L"Object", L"bosa_sdm_msword" );
			if ( hObject == NULL )
			{
				FindWindow( L"Object", L"bosa_sdm_Microsoft Office Word" );
			}
			if ( hObject != NULL )
			{
				::PostMessage( hObject, WM_CLOSE,NULL, NULL );
			}
		}
	}
	else
	{
		// the insert file will be opened and insert into the target file
		hr = pDlg->Execute();
		if ( FAILED(hr) )
		{
			NLPRINT_DEBUGLOG( L"pDlg->Execute() failed \n" );
			stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		}
	}
	return stuResult;
}

ProcessResult CNLProcess::OnExcelInsert( _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, emInsertType=%d \n", wstrFilePath.c_str(),emInsertType);
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_INSERT, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	
	if ( wstrFilePath.empty() )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	ActionFlag flag( emInsertType );
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();	
	
	switch ( emInsertType )
	{
	case kInsertPic:
		{
			stuResult = NLDoExcelInsertPic( );
			//NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d],function:[%s],line:[%d] \n", stuResult.kFuncStat, stuResult.kPolicyStat,	stuResult.kAction, stuResult.fname.c_str(), stuResult.line);
		}
		break;
	case kInsertObj:
	case kInsertDataFromAccess_ExcelOnly:
	case kDataFromText_ExcelOnly:
		{
			CNLInsertAndDataAction& theInsetIns = CNLInsertAndDataAction::GetInstance();
			theInsetIns.initExcelInsertAndDataAction();
			theObMgrIns.NLSetActionStartUpFlag( wstrFilePath, kOA_INSERT, true, &flag );
		}
		break;
	default:
		break;
	}
	return stuResult;	
}

ProcessResult CNLProcess::NLDoExcelInsertPic( )
{
	// for excel insert picture we pop up the insert dialog by ourself and prevent the default insert dialog 
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_INSERT, VARIANT_TRUE, __FUNCTIONW__, __LINE__ );

	Excel::_excelApplicationPtr theApp( (IDispatch*)pep::getApp() );
	Office::FileDialogPtr pFileDlg = NULL;
	
	// 1. get the IFileDlg interface
	HRESULT hr = theApp->get_FileDialog( msoFileDialogFilePicker, &pFileDlg );		
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// 2. set the dialog properties
	hr = pFileDlg->put_AllowMultiSelect( VARIANT_TRUE );
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileDlg->put_Title( _bstr_t( L"Insert Picture" ) );
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileDlg->put_ButtonName( _bstr_t( L"Insert" ) );
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// 3. set filters
	Office::FileDialogFiltersPtr pFilters = NULL;
	hr = pFileDlg->get_Filters( &pFilters );
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Clear();
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	Office::FileDialogFilterPtr pFilter = NULL;
	hr = pFilters->Add(_bstr_t(L"All Files"),_bstr_t(L"*.*"),CComVariant(1),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"All Pictures"),_bstr_t(L"*.emf;*.wmf;*.jpg;*.jpeg;*.jfif;*.jpe;*.png;*.bmp;*.dib;*.rle;*.bmz;*.gif;*.gfa;*.emz"),CComVariant(2),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"Windows Enhanced Metafile"),_bstr_t(L"*.emf"),CComVariant(3),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"Windows Metafile"),_bstr_t(L"*.wmf"),CComVariant(4),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"JPEG File Interchange Format"),_bstr_t(L"*.jpg;*.jpeg;*.jfif;*.jpe"),CComVariant(5),&pFilter);
	if ( FAILED(hr) )
	{
		NLPRINT_DEBUGLOG( L" -0-- Hresul = [0x%x], [0x%x] \n", hr, GetLastError() );
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

 	hr = pFilters->Add(_bstr_t(L"Protable Network Graphics"),_bstr_t(L"*.png"),CComVariant(6),&pFilter);
 	if ( FAILED(hr) )
 	{
 		NLPRINT_DEBUGLOG( L" -0-- Hresult = [0x%x], [0x%x] \n", hr, GetLastError() );
 		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
 		return stuResult;
 	}
	
	// 4. the default insert picture type
	hr = pFileDlg->put_FilterIndex(2);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// 5. show the insert dialog
	int nVal = 0;
	hr = pFileDlg->Show(&nVal);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// 6. user click cancel button
	if ( 0 == nVal )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	FileDialogSelectedItemsPtr pFileItems = NULL;
	BSTR bstrInsertFileName = NULL;
	hr = pFileDlg->get_SelectedItems(&pFileItems);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileItems->Item(1,&bstrInsertFileName);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	wstring wstrInsertPath;
	
	if ( NULL != bstrInsertFileName )
	{
		wstrInsertPath = bstrInsertFileName;
		SysFreeString(bstrInsertFileName);
		bstrInsertFileName = NULL;
	}

	if ( !NLIsValidFileForInsertPicture(wstrInsertPath) ) 	
	{
		wstring wstrText = L"An error occurred while importing this file.";
		wstrText += wstrInsertPath;
		MessageBoxW(GetActiveWindow(),wstrText.c_str(),L"Microsoft Excel",MB_OK|MB_ICONWARNING);
	
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrInsertPath.c_str(), NULL, kOA_INSERT );
	stuResult.vbCancel = VARIANT_TRUE;

	if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
	{
		NLPRINT_DEBUGLOG( L"insert action failed or deny \n" );
		return stuResult;
	}

	IDispatch* pDispSheet = NULL;
	hr = theApp->get_ActiveSheet(&pDispSheet);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	Excel::_WorksheetPtr pSheet(pDispSheet);
	Excel::excelShapesPtr pShapes=NULL;
	hr = pSheet->get_excelShapes(&pShapes);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	Excel::RangePtr pRange=NULL;
	hr = theApp->get_ActiveCell(&pRange);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	CComVariant varLeft(0),varTop(0);
	hr = AutoWrap(DISPATCH_PROPERTYGET,&varLeft,pRange,L"Left",0);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = AutoWrap(DISPATCH_PROPERTYGET,&varTop,pRange,L"Top",0);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	//float dX= 10, dY= 10; 
	//if ( kVer2010 == pep::getVersion() )
	//{
	//	dX = -1;
	//	dY = -1;
	//}

    // fixed bug 35696, now commented out the original code above.
    float dX= -1, dY= -1;

	Excel::excelShapePtr pShape = pShapes->AddPicture(wstrInsertPath.c_str(),msoFalse,msoTrue,(float)varLeft.dblVal,(float)varTop.dblVal,dX,dY);
	if(pShape != NULL)
	{
		pShape->ScaleHeight(1,msoTrue,msoScaleFromTopLeft);
		pShape->ScaleWidth(1,msoTrue,msoScaleFromTopLeft);
	}
	else
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
	}
	return stuResult;
}

ProcessResult CNLProcess::NLDoPPTInsertPic( )
{
	// for PPT insert picture we pop up the insert dialog by ourself and prevent the default insert dialog
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_INSERT, VARIANT_TRUE, __FUNCTIONW__, __LINE__ );

	HRESULT hr = NULL;
	PPT::_pptApplicationPtr theApp( (IDispatch*)pep::getApp() );
	Office::FileDialogPtr   pFileDlg = NULL;

	hr = theApp->get_FileDialog( msoFileDialogFilePicker, &pFileDlg );		
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;	
		return stuResult;
	}

	hr = pFileDlg->put_AllowMultiSelect(VARIANT_TRUE);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileDlg->put_Title(_bstr_t(L"Insert Picture"));
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileDlg->put_InitialFileName(_bstr_t(L""));
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileDlg->put_ButtonName(_bstr_t(L"Insert"));
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	Office::FileDialogFiltersPtr pFilters=NULL;
	hr = pFileDlg->get_Filters(&pFilters);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Clear();
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	Office::FileDialogFilterPtr pFilter=NULL;
	hr = pFilters->Add(_bstr_t(L"All Files"),_bstr_t(L"*.*"),CComVariant(1),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	hr = pFilters->Add(_bstr_t(L"All Pictures"),_bstr_t(L"*.emf;*.wmf;*.jpg;*.jpeg;*.jfif;*.jpe;*.png;*.bmp;*.dib;*.rle;*.bmz;*.gif;*.gfa;*.emz"),CComVariant(2),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"Windows Enhanced Metafile"),_bstr_t(L"*.emf"),CComVariant(3),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"Windows Metafile"),_bstr_t(L"*.wmf"),CComVariant(4),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"JPEG File Interchange Format"),_bstr_t(L"*.jpg;*.jpeg;*.jfif;*.jpe"),CComVariant(5),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFilters->Add(_bstr_t(L"Protable Network Graphics"),_bstr_t(L"*.png"),CComVariant(6),&pFilter);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	// set the default picture type
	hr = pFileDlg->put_FilterIndex(2);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	int nVal = 0;
	hr = pFileDlg->Show(&nVal);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	// user choose cancel
	if ( 0 == nVal )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	FileDialogSelectedItemsPtr pFileItems=NULL;
	BSTR bstrName=NULL;
	hr = pFileDlg->get_SelectedItems(&pFileItems);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pFileItems->Item(1,&bstrName);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	wstring wstrInsertPath;
	if ( NULL != bstrName )
	{
		wstrInsertPath = bstrName;
		SysFreeString( bstrName );
		bstrName = NULL;
	}

    if ( !NLIsValidFileForInsertPicture(wstrInsertPath) ) 	
    {
        wstring wstrText = L"An error occurred while importing this file.";
        wstrText += wstrInsertPath;
        MessageBoxW(GetActiveWindow(),wstrText.c_str(),L"Microsoft PowerPoint",MB_OK|MB_ICONWARNING);

        stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
        return stuResult;
    }

	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	stuResult = theObMgrIns.NLProcessActionCommon( NULL, wstrInsertPath.c_str(), NULL, kOA_INSERT );
	stuResult.vbCancel = VARIANT_TRUE;

	if ( kFSSuccess != stuResult.kFuncStat || kPSDeny == stuResult.kPolicyStat )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	PPT::DocumentWindowPtr pWin=NULL;
	hr = theApp->get_ActiveWindow(&pWin);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	PPT::pptSelectionPtr pSelection;
	hr = pWin->get_pptSelection(&pSelection);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	PPT::SlideRangePtr pRange=NULL;
	hr = pSelection->get_SlideRange(&pRange);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	PPT::pptShapesPtr pShapes=NULL;
	hr = pRange->get_pptShapes(&pShapes);
   
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	PPT::pptShapePtr pPic=NULL;
	//float dX= 10, dY= 10; 
	//if ( kVer2010 == pep::getVersion() )
	//{
	//	dX = -1;
	//	dY = -1;
	//}

    // fixed bug 35698, now commented out the original code above.
    float dX= -1, dY= -1;

	BSTR bstrInsertPath = ::SysAllocString( wstrInsertPath.c_str() );	// here we need alloc a BSTR string
	hr = pShapes->AddPicture(bstrInsertPath,msoFalse,msoTrue,0,0,dX,dY,&pPic);
	SysFreeString(bstrInsertPath);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pPic->ScaleWidth(1,msoTrue,msoScaleFromTopLeft);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pPic->ScaleHeight(1,msoTrue,msoScaleFromTopLeft);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pPic->Select(msoTrue);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	PPT::pptShapeRangePtr pShapeRange=NULL;
	hr = pSelection->get_pptShapeRange(&pShapeRange);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pShapeRange->Align(msoAlignCenters,msoTrue);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}

	hr = pShapeRange->Align(msoAlignMiddles,msoTrue);
	if ( FAILED(hr) )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	return stuResult;
}

ProcessResult CNLProcess::OnPPTInsert( _In_ const wstring& wstrFilePath, _In_ const InsertType& emInsertType )
{
	NLPRINT_DEBUGLOG( L" The Parameters are: wstrFilePath=%ls, emInsertType=%d \n", wstrFilePath.c_str(),emInsertType);
	ProcessResult stuResult( kFSSuccess, kPSAllow,  kOA_INSERT, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );

	if ( wstrFilePath.empty() )
	{
		stuResult.kFuncStat = kFSFailed, stuResult.line = __LINE__;
		return stuResult;
	}
	
	ActionFlag stuOfficeActionFlag( emInsertType );
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();	

	switch ( emInsertType )
	{
	case kInsertPic:
		{
			stuResult = NLDoPPTInsertPic();
		}
		break;
	case kInsertObj:
		{
			CNLInsertAndDataAction& theInsetIns = CNLInsertAndDataAction::GetInstance();
			theInsetIns.initPPTInsertAndDataAction();
			theObMgrIns.NLSetActionStartUpFlag( wstrFilePath, kOA_INSERT, true, &stuOfficeActionFlag );
		}
		break;
	default:
		break;
	}
	return stuResult;
}

bool CNLProcess::NLDoCloseByCloseFlag()
{
	bool bClose = false;
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	wstring wstrActiveFilePath;
	CComPtr<IDispatch> pActiveDoc = getActiveDoc();
	getDocumentPathEx( wstrActiveFilePath, pActiveDoc );
	if ( theObMgrIns.NLGetCloseFlag( wstrActiveFilePath ) )
	{
		bClose = NLCloseDoc( pActiveDoc, wstrActiveFilePath );
	}
	return bClose;
}

void CNLProcess::NLDoExcelHyperLink()
{
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();

	CComPtr<IDispatch> pDoc = getActiveDoc();
	wstring wstrFilePath;
	if ( getDocumentPathEx( wstrFilePath, pDoc ) )
	{
		wstring wstrTemp = theObMgrIns.NLGetLinkFilePath( );
		theObMgrIns.NLSetLinkFilePath( L"" );

		if ( 0 == _wcsicmp( wstrTemp.c_str(), wstrFilePath.c_str() ) )
		{
			NLCloseDoc( pDoc, wstrFilePath );
		}
	}
}

bool CNLProcess::NLIsNoNeedHandle( _In_ IDispatch* pDoc, _In_ const OfficeAction& emAction )
{
	NLCELOG_ENTER
	CComPtr<IDispatch> pActDoc = getActiveDoc();	// Open in IE, word get active path is NULL
	CComPtr<IDispatch> pCurDoc = pDoc;
	if ( NULL == pCurDoc )
	{
		pCurDoc = pActDoc;
	}
	NLPRINT_DEBUGLOG( L" --- 1 --- current doc:[0x%x], active doc:[0x%x] \n", pCurDoc, pActDoc );
	
	{
		// debug
		wstring wstrTempCurFilePath;
		wstring wstrTempActFilePath;
		getDocumentPath( wstrTempCurFilePath, pCurDoc );
		getDocumentPath( wstrTempActFilePath, pActDoc );
		NLPRINT_DEBUGLOG( L"---- debug ---- current File path:[%s], active file path:[%s] \n", wstrTempCurFilePath.c_str(), wstrTempActFilePath.c_str() );
	}

	wstring wstrCurFilePath;
	if ( getDocumentPathEx( wstrCurFilePath, pCurDoc ) )
	{
		if (kAppWord == pep::appType() && wstrCurFilePath== L"Document in Unnamed")
		{
			return true;
		}
		NLPRINT_DEBUGLOG( L"the current action file path is:[%s], action:[%d] \n", wstrCurFilePath.c_str(), emAction );
		
		/*
		*	For excel open in IE(XP), except open action current pDoc and the active pDoc not the same but neither of them can get the file path 
		*/
		// judge open in IE
		CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
		if ( theObMgrIns.NLGetNoNeedHandleFlag( wstrCurFilePath, emAction ) )
		{
			NLPRINT_DEBUGLOG( L"no need handle this action" );
			theObMgrIns.NLSetNoNeedHandleFlag( wstrCurFilePath, emAction, false );
			NLCELOG_RETURN_VAL( true )
		}
		
		NLPRINT_DEBUGLOG( L"current doc:[0x%x], active doc:[0x%x] \n", pCurDoc, pActDoc );		
		if ( ( NULL != pActDoc ) && ( pActDoc != pCurDoc ) )
		{
			/*
			* excel open action, there are not the same
			*/
			NLPRINT_DEBUGLOG( L"!!!!!!!!!!!!! pDoc is not the same \n" );
			// For XP insert action
			// here we check if this action is triggered by insert action.
			// because excel, PPT insert action we can't no when the insert action end.
			// and not all insert action will triggered the open and close action
			if ( IsXp() )
			{
				wstring wstrActFilePath;
				getDocumentPathEx( wstrActFilePath, pActDoc );

				STUNLOFFICE_RIBBONEVENT stuActRibbonEvent;
				theObMgrIns.NLGetCurRibbonEvent( wstrActFilePath, stuActRibbonEvent );

				if ( kRibbonInsert == stuActRibbonEvent.rbEvent 
					|| kRibbonExcelDataTab   == stuActRibbonEvent.rbEvent )
				{
					NLPRINT_DEBUGLOG( L"insert event trigger this,src:[%s],insert:[%s] \n", wstrActFilePath.c_str(), wstrCurFilePath.c_str() );			
					NLCELOG_RETURN_VAL( true )
				}
			}
			
			// drag drop, insert, the current pDoc and the active pDoc is not the same.
			// this judgment conditions need to improve 
#pragma chMSG( "this judgment conditions need to improve" )
	//		bNoNeedHandle = true;	// here we do not know the accurate file.
		}

		// for word open in IE the active event is before the open event
		// we should initialize the cache before we process the open action
		if ( !theObMgrIns.NLGetObMgrInitializeFlag( wstrCurFilePath ) )
		{
			theObMgrIns.NLInitializeObMgr( wstrCurFilePath, pCurDoc );
			theObMgrIns.NLSetCurActiveFilePath( wstrCurFilePath );
			theObMgrIns.NLSetFilePDocCache( wstrCurFilePath, pCurDoc );
		}
	}
	else
	{
		NLPRINT_DEBUGLOG( L"!!!!!Get document file path failed \n" );
	}
	NLCELOG_RETURN_VAL( false )
}


ProcessResult CNLProcess::HandleFacet( 
						_In_ const OfficeAction& action, 
						_In_ const EventType& event, 
						_Inout_ STUNLOFFICE_EVENTSTATUS& stuEventStatus )
{
	NLCELOG_ENTER
	//NLPRINT_DEBUGLOG( L" The Parameters are: emAction=%d, emEventTriggerPoint=%d, stuEventStatus=%p \n", action,event, &stuEventStatus);
	ProcessResult result( kFSSuccess, kPSAllow, action, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	TalkWithSCE& theTalkWithSCE = TalkWithSCE::GetInstance();

	// - Get current file path
	CComPtr<IDispatch> pDoc = NULL;
	wstring wstrCurActivePath;
		// On doc opened in office protected mode
	if (stuEventStatus.stuCurComNotify.bProtectedViewOpen == VARIANT_TRUE)
	{
		wstrCurActivePath = getProctedViewPath( stuEventStatus.stuCurComNotify.pDoc, pDoc );
		if ( wstrCurActivePath.empty() || NULL == pDoc )
		{
			// non-protect view
			NLPRINT_DEBUGLOG( L"Get protect module file path failed, maybe this is not opened by protect module \n" );
			NLCELOG_RETURN_VAL_NOPRINT( result )
		}
	}
	else
	{		
		pDoc = getActiveDoc(); 
		if ( NULL == pDoc )
		{
			//NLPRINT_DEBUGLOG( L" !!!!Error, please care here, we get active doc failed, please check it \n" );
			wstrCurActivePath = theObMgrIns.NLGetCurActiveFilePath();
			pDoc = theObMgrIns.NLGetFilePDocCache( wstrCurActivePath );	// if get active doc failed we will use the cache pDoc.
		}
		else
		{
			getDocumentPathEx( wstrCurActivePath, pDoc ); 
		}
		//NLPRINT_DEBUGLOG( L"----- current active path:[%s], pDoc=[0x%x], action:[%d] ---- \n", wstrCurActivePath.c_str(), pDoc, emAction );
	}
		// Sanity check
	if ( wstrCurActivePath.empty() || NULL == pDoc )
	{
		NLPRINT_DEBUGLOG( L"~~~~~~ the file information is wrong when we check it before we begin to process the action:[%d], FilePath:[%s], pDoc:[%p] \n", 
			action, wstrCurActivePath.c_str(), pDoc );
		result.kFuncStat = kFSFailed, result.line = __LINE__;
		NLCELOG_RETURN_VAL_NOPRINT( result )
	}
	
	// - Check the no need handle flag
	if ( theObMgrIns.NLGetNoNeedHandleFlag( wstrCurActivePath, action ) )
	{
		theObMgrIns.NLSetNoNeedHandleFlag( wstrCurActivePath, action, false ); // here we should recover this flag 
		NLPRINT_DEBUGLOG( L"trigger by:[%d], action:[%d], and this is may be triggered by our code and no need handle it \n", event, action );
		result.kFuncStat = kFSSuccess, result.line = __LINE__;
		NLCELOG_RETURN_VAL_NOPRINT( result )
	}
	
	// - Save the current runtime status
	if ( kFireByComNotify == event )
	{
		// FixBug24688, cause by pDoc release, word new document no need cache the pDoc
		NotifyEvent stuComNotify = stuEventStatus.stuCurComNotify;
		if ( pep::isWordApp( ) )
		{
			NLPRINT_DEBUGLOGEX( true, L"this is new word document \n" );
			stuComNotify.pDoc = NULL;
		}
		
		// in fact this status now we just cache, now no used this anymore.
		theObMgrIns.NLSetCurComNotify( wstrCurActivePath, stuComNotify );
	} 
	else if ( kFireByRibbonUI == event )
	{
		theObMgrIns.NLSetCurRibbonEvent( wstrCurActivePath, stuEventStatus.stuCurRibbonEvent );
	}
	
	// - Hooked Open action ,for some special intension	 
	//		- opened in IE, word and excel need cache file path
	if ( kOA_OPEN == action )
	{
		theObMgrIns.NLClearFileCache( wstrCurActivePath );	// we have no final close event to clear this cache
		theObMgrIns.NLInitializeObMgr( wstrCurActivePath, pDoc );
		
		CComPtr<IDispatch> pActDoc = getActiveDoc();
		if ( NULL != pActDoc )
		{
			/*
			*	cache the file path for office file open in IE
			*	Excel open in IE we can get file path for current pDoc or active pDoc, but we can not get right file path at edit action
			*/
			NLPRINT_DEBUGLOG( L"---------- begin to cache the file path. the active pDoc as the key£º[0x%x],[%s] \n", pActDoc, wstrCurActivePath.c_str() );
			theObMgrIns.NLSetIEFilePathCache( pActDoc, wstrCurActivePath );
		}
		else
		{
			/* word open by IE, at open action we cannot get the active pDoc
			*	 Here Open action, Word application and no active pDoc, we think it is word opened by IE
			*	 We set this flag and will cache the file path at document change event
			*/			
			theObMgrIns.NLSetIEOpenFlag( wstrCurActivePath, true );
		}
	}
	else
	{
		if ( !theObMgrIns.NLGetObMgrInitializeFlag( wstrCurActivePath ) )
		{
			theObMgrIns.NLInitializeObMgr( wstrCurActivePath, pDoc );
		}
	}
	
	// update cache
	theObMgrIns.NLSetCurActiveFilePath( wstrCurActivePath );
	theObMgrIns.NLSetFilePDocCache( wstrCurActivePath, pDoc );

	// debug
	{
		wstring wstrActionTemp = NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum( action );
		NLPRINT_DEBUGLOG( L"------------------- action:[%s], trigger point:[%d], file path:[%s] \n", wstrActionTemp.c_str(), event, wstrCurActivePath.c_str() );
		wchar_t wszTempLog[1024] = { 0 };
		swprintf_s( wszTempLog, 1023, L"Begin to process office action, trigger point:[%d], file path:[%s]", event, wstrCurActivePath.c_str() );
		NLMessageBox( wszTempLog, wstrActionTemp.c_str(), MB_OK );
	}

	switch ( action )
	{
	case	kOA_OPEN:
		{
			if ( kFireByComNotify == event )
			{
				if (VARIANT_TRUE == stuEventStatus.stuCurComNotify.bProtectedViewOpen )
				{
					// protect view
					result = NLProcessProtectedViewAction(pDoc,wstrCurActivePath, stuEventStatus.stuCurComNotify.pDoc);
				}
				else
				{
					// normal or Open inIE
					result = OnOpen( pDoc, wstrCurActivePath ); 							
				}

				/*
					1. For protect module open action we add PrintScreen cache after open success for word/excel/PPT.
					2. For normal module open action we add this cache for word and excel after open success.
					3. For PPT normal module open we add it at before open action and clear it after we deny open action.
				*/
				if ( kFSSuccess != result.kFuncStat || kPSDeny == result.kPolicyStat )
				{
					// deny
					if ( ( VARIANT_TRUE != stuEventStatus.stuCurComNotify.bProtectedViewOpen ) && 
						pep::isPPtApp() )
					{
						// PPT normal module open.
						theTalkWithSCE.CacheOpenedFile( wstrCurActivePath, EF_SUB, TAG(0) );
					}
				}
				else
				{
					// allow
					theTalkWithSCE.CacheOpenedFile( wstrCurActivePath, EF_ADD, TAG(0) );
				}

				/*
				*	Word/Excel open in IE we can not get right file path and set this flag at active event
				*	Protect view we start the passive action at here.
				*/
				if ( kFSSuccess == result.kFuncStat &&
					kPSAllow == result.kPolicyStat &&
					VARIANT_FALSE == result.vbCancel
					)
				{
					NLStartUpPassiveAction( wstrCurActivePath );
				}
			}
		}// end NLOFFICEACTION_OPEN
		break;
	case	kOA_EDIT:
		{
// save
			if ( kFireByComNotify == event )
			{
				// for bug24370 word need check this flag
				if ( pep::isWordApp() && ( !theObMgrIns.NLGetUserSaveFlag( wstrCurActivePath ) ) )
				{
					// maybe this EDIT com notify is trigger by word auto save
					NLPRINT_DEBUGLOG( L"Maybe word auto save happened \n" );
				}
				else
				{
					result = OnEdit( pDoc, wstrCurActivePath );
					theObMgrIns.NLSetUserSaveFlag( wstrCurActivePath, false );
				}							
			}
			else if ( kFireByRibbonUI == event )
			{
				theObMgrIns.NLSetUserSaveFlag( wstrCurActivePath, true );
				switch ( pep::appType() )
				{
				case kAppWord:  
					{
						// for word here nothing need to do, all save action has COM event notify and new document has copy event notify
						/*
						*	Word: ribbon edit notify
						*	All save action has COM event notify and new document has copy event notify
						*	Normal: ctrl+s, save button, COM event notify save event. 
						*	Open in IE: Edit action: ctrl+s, save button, COM event notify save as event.
						*							Copy action: no office file save as button, use IE menu bar to do save as.
						*	Protect module: 
						*/
						NLPRINT_DEBUGLOG( L"WORD: ribbon edit action happened \n" );
 						if ( isOpenInIE() )
 						{
							NLPRINT_DEBUGLOG( L"this word docment is opening in IE" );

							// process EDIT action
							result = OnEdit( pDoc, wstrCurActivePath );

 							// set no need handle flag, no need handle Save as
							// the function is success and no deny, we need set this flag, because after ribbon notify, COM save as event will be triggered.
 							if ( kFSSuccess == result.kFuncStat &&
								   kPSAllow == result.kPolicyStat &&
									 VARIANT_FALSE == result.vbCancel
								 )
 							{
								theObMgrIns.NLSetNoNeedHandleFlag( wstrCurActivePath, kOA_COPY, true );
 							}
						}
					}
					break;
				case kAppExcel:
					{
						/*
						*	Excel open in IE
						*	XP no save action
						* Win7:
						*		Ctrl+s --> save as
						*		Save button --> choose --> save or save as
						*/
						NLPRINT_DEBUGLOG( L"Excel: ribbon edit action happened \n" );
					}
					break;
				case kAppPPT:
					{
						NLPRINT_DEBUGLOG( L"PPT: ribbon edit action happened \n" );
					}
					break;
				default:
					break;
				}
			}
		}
		break;
	case	kOA_COPY:
		{
// save as -> convert
			if ( kFireByRibbonUI == event )
			{	
				if ( pep::isPPtApp() )
				{
					NLPRINT_DEBUGLOG( L"process PPT copy action pDoc:[0x%x], active doc:[0x%x] \n", pDoc, getActiveDoc() );
					
					// PPT, copy action will triggered save action
					result = OnCopy( pDoc, wstrCurActivePath, GetFileSaveAsType() );
				}
				else
				{
					SetFileSaveAsType( stuEventStatus.stuCurRibbonEvent.unOfficeEventExtraType.emExtraDesFileType );
				}
			}
			else if ( kFireByComNotify == event )
			{
				//NLPRINT_DEBUGLOG( L"process copy action \n" );
				// for save
				if ( !pep::isPPtApp() )
				{
					// oye: ignore XP epoch
					// XP excel open by IE, save as action, has some problem: 
					// I will process this case later					
					//if ( IsXp() && pep::isExcelApp() && isOpenInIE() )
					//{
					//	// we can not process this case
					//	NLPRINT_DEBUGLOG( L"this is XP excel open in IE save as action, now I don't known hao to process it, old PEE will hang at this case \n" );
					//	result.vbCancel = VARIANT_TRUE;
					//}
					//else
					//{
					//	
					//}

					result = OnCopy(pDoc, wstrCurActivePath, GetFileSaveAsType());
				}
			}
		}
		break;
	case	kOA_PASTE:
		{
			result = OnPaste( pDoc, wstrCurActivePath );
		}
		break;
	case	kOA_PRINT:
		{
			result = OnPrint( pDoc, wstrCurActivePath );
		}
		break;
	case	kOA_SEND:
		{
// don't care destination
			result = OnSend( pDoc, wstrCurActivePath );
		}
		break;
	case kOA_INSERT:	// insert event is convert action, but we need precess it independent
		{
// insert, data
			if ( kFireByRibbonUI == event )
			{
				result = OnInsert( wstrCurActivePath, stuEventStatus.stuCurRibbonEvent.unOfficeEventExtraType.emExtraInsertType );
			}
		}
		break;	
	case	kOA_CONVERT:
		{
// don't care destination, save as other format
			if ( kFireByRibbonUI == event )	// office 2007 save as XPS or PDF
			{
				result = OnConvert( pDoc, wstrCurActivePath );
			}
		}
		break;
	case kOA_CLOSE:
		{
			if ( VARIANT_TRUE == stuEventStatus.stuCurComNotify.bProtectedViewClose )
			{
				// process protect view close
				ProtectedViewWindowBeforeClose( pDoc, wstrCurActivePath );
			}
			else
			{
				result = OnClose( pDoc, wstrCurActivePath );
			}
		}
		break;
	default:
		NLPRINT_DEBUGLOG( L"unknown action may be some error happened, please check \n" );
		break;
	}
	
	NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d], variant cancel:[%d], function:[%s],line:[%d] \n", result.kFuncStat, result.kPolicyStat,
		result.kAction, result.vbCancel, result.fname.c_str(), result.line);

	NLCELOG_RETURN_VAL_NOPRINT( result )
} //end OnOfficeAction

ProcessResult CNLProcess::NLProcessComNotifyWord( _In_ IDispatch* pDoc, 
	_In_ const WordEvent& emWordComEvent, 
	_In_ const VARIANT_BOOL&  bVariantSaveAsUI )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	
	NotifyEvent stuComNotify( emWordComEvent, pDoc, bVariantSaveAsUI );
	STUNLOFFICE_EVENTSTATUS stuEventStatus( stuComNotify );
	
	NLPRINT_DEBUGLOG( L" ***************** The word COM notify: word event:[0x%x], save as UI:[%s] \n", emWordComEvent, bVariantSaveAsUI ? L"true": L"false" );
	switch ( emWordComEvent )
	{
	case kWdStartup:			// ( emWordStartup = 0x01 )
		{
			// we can do some simple initialize things here
			NLPRINT_DEBUGLOG( L"word start up \n" );      
		}
		break;
	case kWdDocChange:	// ( emWordDocChange = 0x03 )
		{
//#pragma chMSG( "Word doc change, here we should and set pDoc NULL and check protect view open and do evaluation" )
			//	CComPtr<IDispatch> pDoc = NULL,EvaProtectedViewOpen(pDoc);
			// here we should update the current active file path, protect module
			
			// for Word open in IE
			// after word open in IE no active event, only this event
			CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
			if ( isOpenInIE() )
			{
				wstring wstrFilePath = theObMgrIns.NLGetCurActiveFilePath();
				if ( theObMgrIns.NLGetIEOpenFlag( wstrFilePath ) )
				{
					CComPtr<IDispatch> pActDoc = getActiveDoc();
					NLPRINT_DEBUGLOG( L"======== emWordDocChange: the active pDoc is: CComPtr=[0x%x], IDispatch=[0x%x] \n", pActDoc, pActDoc.p );

					// this is word open in IE, after open action we save the file path
					theObMgrIns.NLSetIEFilePathCache( pActDoc, wstrFilePath );
					theObMgrIns.NLSetIEOpenFlag( wstrFilePath, false );
				}
			}
			
	//		stuEventStatus.stuCurComNotify.bProtectedViewOpen = VARIANT_TRUE;
	//		stuResult = NLProcessOfficeAction( NLOFFICEACTION_OPEN, emNLOfficeEventTriggerByCOMNotify, stuEventStatus );
			break;
		}
		break;
	case kWdDocOpen:				// ( emWordOpen	= 0x04 )
		{
			NLPRINT_DEBUGLOG( L"open action \n" );
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case kWdDocBeforeClose:	// ( emWordBeforeClose	=  0x06 )
		{
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_CLOSE, kFireByComNotify, stuEventStatus );	
		}
		break;
	case kWdProtectedViewWindowOpen:
		{
			stuEventStatus.stuCurComNotify.bProtectedViewOpen = VARIANT_TRUE;
			HandleFacet( kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case kWdProtectedViewWindowBeforeClose:
		{
			stuEventStatus.stuCurComNotify.bProtectedViewClose = VARIANT_TRUE;
			HandleFacet( kOA_CLOSE, kFireByComNotify, stuEventStatus );
		}
		break;
	case kWdDocBeforeSave:	// ( emWordBeforeSave = 0x08 )
		{
			// Only ribbon: save/save as/ctrl+s --> ribbon event; other save event --> Com before save notify		
			// save or save as
			if ( bVariantSaveAsUI )
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_COPY, kFireByComNotify, stuEventStatus );
			}
			else
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_EDIT, kFireByComNotify, stuEventStatus );
			}
		}
		break;
	case kWdNewDoc:		// ( emWordNewDoc = 0x09 )
		{
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case kWdWndActivate:			// ( emWordActivate = 0x0A )
		{
			//"Word active, here we should check if we need do overlau and do evaluation for open action"
			OnWndActive( pDoc );	// this function update some cache that will used in "NLDoCloseByCloseFlag"
			
			if ( !NLDoCloseByCloseFlag() )
			{
				NLVIEWOVERLAYINSTANCE.TriggerDoViewOverlay();
			}
		}
		break;
	default:
		break;
	}
	
	NLPRINT_DEBUGLOG( L"result:function statue:[%d],policy statue:[%d],action:[%d], variant cancel:[%d], function:[%s],line:[%d] \n", stuResult.kFuncStat, stuResult.kPolicyStat,
		stuResult.kAction, stuResult.vbCancel, stuResult.fname.c_str(), stuResult.line);
	return stuResult;
}

ProcessResult CNLProcess::NLProcessComNotifyExcel( _In_ IDispatch* pDoc, _In_ const ExcelEvent& emExcelComEvent,
																											 _In_ const VARIANT_BOOL&  bVariantSaveAsUI )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	STUNLOFFICE_EVENTSTATUS stuEventStatus;
	stuEventStatus.stuCurComNotify.eventflag = emExcelComEvent;

	NLPRINT_DEBUGLOG( L"The excel COM notify: excel event:[%d], save as UI:[%s] \n", emExcelComEvent, bVariantSaveAsUI ? L"true": L"false" );
	switch ( emExcelComEvent )
	{
	case emExcelWindowsDeactivate:			// ( emExcelWindowsDeactivate	= 0x615 )
		{
			// no thing to do at this event
		}
		break;
	case emExcelOpen:									// ( emExcelOpen	= 0x61F )
		{
			NLPRINT_DEBUGLOG( L"excel open action \n" );
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case emExcelSave:									// ( emExcelSave = 0x623 )
		{
			if ( bVariantSaveAsUI )// for excel .xltx file save operator(ctrl+s ...) we get COPY action directly
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_COPY, kFireByComNotify, stuEventStatus );
			}
			else
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_EDIT, kFireByComNotify, stuEventStatus );
			}
		}
		break;
	case emExcelClose:									// ( emExcelClose	= 0x622 )
		{
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_CLOSE, kFireByComNotify, stuEventStatus );
		}
		break;
	case emExcelPrint:									// ( emExcelPrint	= 0x624 )
		{
			// print overlay
		}
		break;
	case emExelWBActive:							// ( emExelWBActive = 0x620 )
		{
			OnWndActive( pDoc );
			NLVIEWOVERLAYINSTANCE.TriggerDoViewOverlay();
		}
		break;
	case emProtectedViewOpen:					// ( emProtectedViewOpen = 0xB57 )
		{
			// protect module open the excel
			stuEventStatus.stuCurComNotify.bProtectedViewOpen = VARIANT_TRUE;
			stuEventStatus.stuCurComNotify.pDoc = pDoc;
			stuResult = HandleFacet( kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case emExcelProtectedViewWindowBeforeClose:
		{
			stuEventStatus.stuCurComNotify.bProtectedViewClose = VARIANT_TRUE;
			stuResult = HandleFacet( kOA_CLOSE, kFireByComNotify, stuEventStatus );
		}
		break;
	case emExcelSelectionChange:					// ( emExcelSelectionChange = 0x616 )
		{
			// nothing to do here
		}
		break;
	case emProtectedViewWindowActivate:	// ( emProtectedViewWindowActivate = 0xB5D )
		{
			// nothing to do here
		}
		break;
	case emExcelHyperLink:								// ( emExcelHyperLink = 0x73E )
		{
			NLDoExcelHyperLink();
		}
		break;
	case emWindowsActive:								// ( emWindowsActive = 0x614 )
        /*
         * can't input here when open excel from sharepoint to view overlay, bug 35164 by allen.
        */
		{
			// do overlay here
			if ( !NLDoCloseByCloseFlag() )
			{
				NLVIEWOVERLAYINSTANCE.TriggerDoViewOverlay();
			}
		}
		break;
	default:
		break;
	}
	return stuResult;
}

ProcessResult CNLProcess::NLProcessComNotifyPpt( _In_ IDispatch* pDoc, _In_ const PPTEvent& emPptComEvent, _In_ const VARIANT_BOOL&  bVariantSaveAsUI )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	STUNLOFFICE_EVENTSTATUS stuEventStatus;
	stuEventStatus.stuCurComNotify.eventflag = emPptComEvent;

	NLPRINT_DEBUGLOG( L"The ppt COM notify: ppt event:[%d], save as UI:[%s] \n", emPptComEvent, bVariantSaveAsUI ? L"true": L"false" );
	switch ( emPptComEvent )
	{ 
	case emPPTAfterNewPresentation:	// ( emPPTAfterNewPresentation = 0x7E4 )
		{
			NLPRINT_DEBUGLOG( L"new PPT file open action \n" );
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case emPPTAfterPresentationOpen:		// ( emPPTAfterPersentationOpen = 0x7E5, after open )
		{
			/* 
			   for office add-in: "Camtasia", if we install this office add-in, it will prevent the open event
			   our office pep can not get PPT after open event.
			   old office pep process open action at here and deny(close) the PPT at emPPTWindowActivate or emPPTWinSelectionChange
			*/
			NLPRINT_DEBUGLOG( L"PPT open action \n" );
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );			
		}
		break;
	case emPPTPresentationOpen:			// ( emPPTPresentationOpen = 0x7D6, before open )
		{
			wstring wstrPPTFilePath;
			getDocumentPath( wstrPPTFilePath, pDoc );
			NLOBMGRINSTANCE.NLSetPPTPrintActiveFilePath( wstrPPTFilePath );

			TalkWithSCE::GetInstance().CacheOpenedFile( wstrPPTFilePath, EF_ADD, TAG(0) );
		}
		break;
	case emPPTWinSelectionChange:	// ( emPPTWinSelectionChange = 0x7D1 )
		{
			NLDoCloseByCloseFlag();
		}
		break;
	case emPPTBeforeClose:				// ( emPPTBeforeClose = 0x7D4 )
		{
			stuResult = NLProcessOfficeActionFace( pDoc, kOA_CLOSE, kFireByComNotify, stuEventStatus );
		}
		break;
	case emPPTSlideShow:					// ( emPPTSlideShow = 0x7DB )
		{
			wstring wstrCurFilePath;
			getDocumentPathEx( wstrCurFilePath, getActiveDoc() );
			NLPRINT_DEBUGLOG( L" the current file path is:[%s] \n", wstrCurFilePath.c_str() );
			
			if ( !NLOBMGRINSTANCE.NLGetObMgrInitializeFlag( wstrCurFilePath ) )
			{
				// FixBug24680 this used to process right menu "show"
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_OPEN, kFireByComNotify, stuEventStatus );
				NLPRINT_DEBUGLOGEX( true, L" --- right menu slide show:[%s] \n", wstrCurFilePath.c_str() );
			}
			else
			{
				NLVIEWOVERLAYINSTANCE.TriggerDoViewOverlay();
			}
		}
		break;
	case emPPTSave:							// ( emPPTSave = 0x7E2 )
		{
			if (g_bSavePressed)
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_EDIT, kFireByComNotify, stuEventStatus );
			}
			else
			{
				stuResult = NLProcessOfficeActionFace( pDoc, kOA_COPY, kFireByComNotify, stuEventStatus );
			}
			g_bSavePressed =FALSE;
		}
		break;
	case emPPTAferSaveAs:					// ( emPPTAferSaveAs = 0x7D5 )
		{
			// this is not save as action
			NLPRINT_DEBUGLOG( L"=========== PPT after save as COM notity \n" );
		}
		break;
	case emPPTWindowActivate:		// ( emPPTWindowActivate = 0x7D9 )
		{
			OnWndActive( pDoc );
			NLVIEWOVERLAYINSTANCE.TriggerDoViewOverlay();
		}
		break;
	case emPPTPrint:							// ( emPPTPrint = 0x7DF )
		{
			// we do print overlay by hook "StartDoc"
		}
		break;
	case emPPTProtectedViewWindowOpen: // ( emPPTProtectedViewWindowOpen	= 0x7EA )
		{
			// open by protect module
			stuEventStatus.stuCurComNotify.bProtectedViewOpen = VARIANT_TRUE;
			stuEventStatus.stuCurComNotify.pDoc = pDoc;
			stuResult = HandleFacet( kOA_OPEN, kFireByComNotify, stuEventStatus );
		}
		break;
	case emPPTProtectedViewWindowBeforeClose:
		{
			stuEventStatus.stuCurComNotify.bProtectedViewClose = VARIANT_TRUE;
			stuResult = HandleFacet( kOA_CLOSE, kFireByComNotify, stuEventStatus );
		}
		break;
	case emPPTSlideShowNextClick:		// ( emPPTSlideShowNextClick = 0x7E3 )
		{
			NLDoCloseByCloseFlag();
		}
		break;
	default:
		break;
	}
	return stuResult;
}

ProcessResult CNLProcess::NLProcessHookEvent( _Inout_ HookEvent& stuOfficeHookEvent )
{
	ProcessResult stuResult( kFSSuccess, kPSAllow, kOA_Unknown, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	STUNLOFFICE_EVENTSTATUS stuEventStatus;
	stuEventStatus.stuCurHookEvent = stuOfficeHookEvent;

	switch ( stuOfficeHookEvent.emOfficeHookEvent )
	{
	case emOfficeHookKeyBoardCtrlC:	// copy/cut/paste are all paste action.
	case emOfficeHookKeyBoardCtrlX:
		{
			stuResult = HandleFacet( kOA_PASTE, kFireByHookAPI, stuEventStatus );		
		}
		break;
	default:
		break;
	}
	return stuResult;
}

ProcessResult CNLProcess::NLProcessOfficeActionFace( _In_ IDispatch* pDoc, _In_ const OfficeAction&	emAction, _In_ const EventType& emEventTriggerPoint, _Inout_ STUNLOFFICE_EVENTSTATUS& stuEventStatus )
{
	//ONLY_DEBUG
	NLPRINT_DEBUGLOG( L" The Parameters are: pDoc=%p, emAction=%d, emEventTriggerPoint=%d, stuEventStatus=%p \n", pDoc,emAction,emEventTriggerPoint,&stuEventStatus);
	ProcessResult stuResult( kFSSuccess, kPSAllow, emAction, VARIANT_FALSE, __FUNCTIONW__, __LINE__ );
	
	if ( !NLIsNoNeedHandle( pDoc, emAction ) )
	{
		stuResult = HandleFacet( emAction, kFireByComNotify, stuEventStatus ); // can input the pDoc parameter.
	}
	else
	{
		// debug
		{
			wstring wstrActionTemp = NLACTIONTYPEINSTANCE.NLGetStringActionTypeByEnum( emAction );
			NLPRINT_DEBUGLOG( L"-------------------no need handle: action:[%s], trigger point:[%d] \n", wstrActionTemp.c_str(), emEventTriggerPoint );
		}
	}
	return stuResult;
}

