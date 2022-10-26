// NxtOfficeSink.cpp : Implementation of CNxtOfficeSink
#include "stdafx.h"
#include "resource.h"
#include "dllmain.h"
#include "utils.h"
#include "NLAction.h"
#include "OfficeListener.h"
#include "NLObMgr.h"
#include "obligations.h"
#include "TalkWithSCE.h"
#include "NLOfficeSink.h"
#include "NLHookAPI.h"
#include "ScreenCaptureAuxiliary.h"

#define CELOG_CUR_FILE static_cast<celog_filepathint_t>(CELOG_FILEPATH_OFFICEPEP_MIN + EMNLFILEINTEGER_NLOFFICESINK)
extern BOOL g_bSavePressed;

CNxtOfficeSink::CNxtOfficeSink( ): m_pOfficeListener( NULL ), m_hCBHandle( NULL )
{

}

CNxtOfficeSink::~CNxtOfficeSink( )
{

}

BOOL CALLBACK CNxtOfficeSink::EnumChildProc(HWND hwnd, LPARAM lParam)
{
	wchar_t className[MAX_PATH] = { 0 };
	GetClassName(hwnd,className,MAX_PATH);

	if(wcscmp(className, L"EXCEL7") == 0)
	{
		CNLOvelayView& theOV = CNLOvelayView::GetInstance();
		theOV.AddView(hwnd);
		return false;
	}

	return true;
}

BOOL CALLBACK CNxtOfficeSink::EnumThreadWndProc(HWND hwnd, LPARAM lParam)
{
	wchar_t className[MAX_PATH] = { 0 };
	GetClassName(hwnd,className,MAX_PATH);

	if(wcscmp(className, L"XLMAIN") == 0)
	{
		return EnumChildWindows(hwnd, EnumChildProc, NULL);
	}

	return true;
}

// use this interface to insert our office add-in 
STDMETHODIMP CNxtOfficeSink::OnConnection( LPDISPATCH Application, ext_ConnectMode ConnectMode, LPDISPATCH AddInInst, SAFEARRAY * * custom )
{
	if (ConnectMode == ext_cm_AfterStartup)
	{
		return S_OK;
	}

	_AtlModule.OnAppConnect(Application);


	CNLAction& theActionIns = CNLAction::NLGetInstance();

	switch ( pep::appType() )
	{
	case kAppWord:
		{
			m_pOfficeListener = new CWordListener( );
			m_pOfficeListener->NLSinkEvent( Application );
		}
		break;
	case kAppExcel:
		{
			m_pOfficeListener = new CExcelListener( );

			/*
			*\brief: this code for open excel from IE(share point) directly if no excel application running
			*		 this is Microsoft's bug , no event sink for such scenarios.
			*	Excel open in share point no this open action
			*/
			HRESULT hr = E_FAIL;
			Excel::_excelApplicationPtr theApp(Application);
			if(theApp != NULL)
			{
				Excel::_WorkbookPtr pBook = NULL;
				hr = theApp->get_ActiveWorkbook(&pBook);
				if ( SUCCEEDED(hr) && pBook != NULL)
				{
					CComPtr<IDispatch> pDoc = NULL;
					hr = pBook->QueryInterface( IID_IDispatch, (void**)(&(pDoc.p)) );
					if( SUCCEEDED(hr) && NULL != pDoc )	
					{
						EnumThreadWindows(GetCurrentThreadId(), EnumThreadWndProc, NULL);

						NotifyEvent stuComNotifyEvent( emExcelOpen, pDoc, VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE );
						theActionIns.NLSetEventForCOMNotify( &stuComNotifyEvent );
					}
				}
				else
				{
					CComVariant varResult;
					hr = AutoWrap(DISPATCH_PROPERTYGET, &varResult, Application, L"ActiveProtectedViewWindow",0);
					if ( SUCCEEDED(hr) && varResult.pdispVal != NULL )
					{
						EnumThreadWindows(GetCurrentThreadId(), EnumThreadWndProc, NULL);

						NotifyEvent stuComNotifyEvent( emProtectedViewOpen, varResult.pdispVal, VARIANT_FALSE, VARIANT_FALSE, VARIANT_TRUE );
						theActionIns.NLSetEventForCOMNotify(&stuComNotifyEvent);
					}
				}
			}
			m_pOfficeListener->NLSinkEvent( Application );
		}
		break;
	case kAppPPT:
		{
			m_pOfficeListener = new CPPTListener( );
			m_pOfficeListener->NLSinkEvent( Application );
		}
		break;
	default:
		{
			NLPRINT_DEBUGLOG( L"!!!!!! get application type error \n" );
			return S_FALSE;
		}
		break;
	}

	// initialize hook event
	CNxtHookAPI& theHookIns = CNxtHookAPI::NLGetInstance();
	theHookIns.Init();
	
	WCHAR modulePath[MAX_PATH] = { 0 };
	GetModuleFileNameW(g_hInstance, modulePath, MAX_PATH);
	LoadLibraryW(modulePath);

	return S_OK;
}



STDMETHODIMP CNxtOfficeSink::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
{ONLY_DEBUG	
	CNLObMgr& theObMgrIns = CNLObMgr::NLGetInstance();
	if ( ext_dm_UserClosed == RemoveMode )
	{
		static bool bLoadItself = false;
		if (!bLoadItself)
		{
			wchar_t wszPath[MAX_PATH+1] = { 0 };
			GetModuleFileNameW( g_hInstance, wszPath, MAX_PATH );
			bLoadItself = ( NULL != LoadLibrary( wszPath ) );
			
			// FixBug24688
			if ( pep::isWordApp() )
			{
				wstring wstrFilePath;
				getDocumentPathEx( wstrFilePath, getActiveDoc() );
				NLPRINT_DEBUGLOGEX( true, L" the current file path is:[%s] \n", wstrFilePath.c_str() );
				
				theObMgrIns.NLSetFilePDocCache( wstrFilePath, NULL );
				theObMgrIns.NLSetRealsePDocFlag( wstrFilePath, false );
			}
		}
		return S_OK;
	}
	// set disconnect flag
	_AtlModule.connected(FALSE);
	
	// For SCE
	TalkWithSCE::GetInstance().UnregisterPEPCLient();

	// stop event notify
	if ( NULL != m_pOfficeListener )
	{
        SendMessage(g_hBubbleWnd, WM_DESTROY, NULL, NULL);
		m_pOfficeListener->NLUnSinkEvent();
		delete m_pOfficeListener;
		m_pOfficeListener = NULL;
	}
	
	// free action module
	NLACTIONINSTANCE.UnInitialize();
	
	// release hook event, for BUG24666, no need unhook
#if 0
	CNxtHookAPI& theHookIns = CNxtHookAPI::NLGetInstance();
	theHookIns.UnInit();
#endif	

	// Uninitialize asynchronous I/O, SE synchronize golden tags
	theObMgrIns.NLUnInitializeSynchronizeGoldenTags();

	// clear cache
	theObMgrIns.NLClearCache();

	return S_OK;
}

/*
*  we use IFileDialog for save as/insert ,others use Robbin button or event sink
*/
wstring CNxtOfficeSink::GetOfficeCommandXml()
{
	wstring  strCommandXml = L"";
	strCommandXml += L"<commands>";
	// generic action
	// save
	strCommandXml += L"<command idMso=\"FileSave\" onAction=\"FileSaveAction\"/>";
	// paste
	strCommandXml += L"<command idMso=\"Paste\" onAction=\"PasteAction\"/>";
	// copy
	strCommandXml += L"<command idMso=\"Copy\" onAction=\"CopyAction\"/>";
	// cut
	strCommandXml += L"<command idMso=\"Cut\" onAction=\"CutAction\"/>";
	// print
	strCommandXml += L"<command idMso=\"FilePrint\" onAction=\"FilePrintAction\"/>";
	// quick print
	strCommandXml += L"<command idMso=\"FilePrintQuick\" onAction=\"FilePrintQuickAction\"/>";
	// insert->object
	strCommandXml += L"<command idMso=\"OleObjectctInsert\" onAction=\"OleObjectctInsertAction\"/>";
	// send
	strCommandXml += L"<command idMso=\"FileSendAsAttachment\" onAction=\"FileSendAsAttachmentAction\"/>";
	strCommandXml += L"<command idMso=\"FileEmailAsPdfEmailAttachment\" onAction=\"FileEmailAsPdfEmailAttachmentAction\"/>";
	strCommandXml += L"<command idMso=\"FileInternetFax\" onAction=\"FileInternetFaxAction\"/>";
	strCommandXml += L"<command idMso=\"WindowNew\" onAction=\"WindowNewAction\"/>";
	// save as PDF or XPS
	strCommandXml += L"<command idMso=\"FileSaveAsPdfOrXps\" onAction=\"FileSaveAsPdfOrXpsAction\"/>";
	strCommandXml += L"<command idMso=\"FileEmailAsXpsEmailAttachment\" onAction=\"FileEmailAsXpsEmailAttachmentAction\"/>";
	// brief: for save as, 
	//	excel and word implemented under event, 
	//	so we only want to know the save as type via sink Ribbon button.
	strCommandXml += L"<command idMso=\"FileSaveAs\" onAction=\"FileSaveAsAction\"/>";

	OfficeVer emOfficeVer = pep::getVersion();
	if ( kVer2007 == emOfficeVer )
	{
		// 2007-> prepare->properties
		strCommandXml += L"<command idMso=\"FileProperties\" onAction=\"FilePropertiesAction\"/>";

		// 2007->publish->create document workspace
		strCommandXml += L"<command idMso=\"FileCreateDocumentWorkspace\" onAction=\"FileCreateDocumentWorkspaceAction\"/>";

		strCommandXml += L"<command idMso=\"FileSaveAsOtherFormats\" onAction=\"FileSaveAsOtherFormatsAction\"/>";
	} 
	else /*if ( kVer2010 == emOfficeVer )*/
	{
		// 2010,info->properties->advanced properties
		strCommandXml += L"<command idMso=\"AdvancedFileProperties\" onAction=\"AdvancedFilePropertiesAction\"/>";

		// 2001, insert->screen short
		strCommandXml += L"<command idMso=\"ScreenClipping\" onAction=\"ScreenClippingAction\"/>";
	}
	
	AppType emOfficeAppType = pep::appType( );
	switch ( emOfficeAppType )
	{
	case kAppWord:
		{
			// insert picture
			strCommandXml += L"<command idMso=\"PictureInsertFromFile\" onAction=\"PictureInsertFromFileAction\"/>";	// For Word/Excel insert picture

			strCommandXml += L"<command idMso=\"TextFromFileInsert\" onAction=\"TextFromFileInsertAction\"/>";
			strCommandXml += L"<command idMso=\"FileNewBlogPost\" onAction=\"FileNewBlogPostAction\"/>";
			strCommandXml += L"<command idMso=\"ZoomCurrent100\" onAction=\"ZoomCurrent100Action\"/>";
			strCommandXml += L"<command idMso=\"ZoomOnePage\" onAction=\"ZoomOnePageAction\"/>";
			strCommandXml += L"<command idMso=\"ZoomTwoPages\" onAction=\"ZoomTwoPagesAction\"/>";
			strCommandXml += L"<command idMso=\"ZoomPageWidth\" onAction=\"ZoomPageWidthAction\"/>";

			if ( kVer2007 == emOfficeVer )
			{
				strCommandXml += L"<command idMso=\"FileSaveToDocumentManagementServer\" onAction=\"FileSaveToDocumentManagementServerAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsWordDocx\" onAction=\"FileSaveAsWordDocxAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsWordDotx\" onAction=\"FileSaveAsWordDotxAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsWord97_2003\" onAction=\"FileSaveAsWord97_2003Action\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsWordOpenDocumentText\" onAction=\"FileSaveAsWordOpenDocumentTextAction\"/>";
			} 
			else /*if ( kVer2010 == emOfficeVer )*/
			{
				// for 2010
			}
		}
		break;
	case kAppExcel:
		{
			// insert picture
			strCommandXml += L"<command idMso=\"PictureInsertFromFile\" onAction=\"PictureInsertFromFileAction\"/>";	// For Word/Excel insert picture

			strCommandXml += L"<command idMso=\"GetExternalDataFromAccess\" onAction=\"GetExtDataFromAccess\"/>";
			strCommandXml += L"<command idMso=\"GetExternalDataFromWeb\" onAction=\"GetExtDataFromWeb\"/>";
			strCommandXml += L"<command idMso=\"GetExternalDataFromText\" onAction=\"GetExtDataFromText\"/>";
			strCommandXml += L"<command idMso=\"GetExternalDataExistingConnections\" onAction=\"GetExtDataFromExistConn\"/>";
			strCommandXml += L"<command idMso=\"CopyAsPicture\" onAction=\"CopyAsPictureAction\"/>";
			strCommandXml += L"<command idMso=\"SheetMoveOrCopy\" onAction=\"SheetMoveOrCopyAction\"/>";
			if ( kVer2007 == emOfficeVer )
			{
				strCommandXml += L"<command idMso=\"FileSaveToDocumentManagementServer\" onAction=\"FileSaveToDocumentManagementServerAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsExcelXlsx\" onAction=\"FileSaveAsExcelXlsxAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsExcelXlsxMacro\" onAction=\"FileSaveAsExcelXlsxMacroAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsExcel97_2003\" onAction=\"FileSaveAsExcel97_2003Action\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsExcelXlsb\" onAction=\"FileSaveAsExcelXlsbAction\"/>";
				strCommandXml += L"<command idMso=\"FilePublishExcelServices\" onAction=\"FilePublishExcelServicesAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsExcelOpenDocumentSpreadsheet\" onAction=\"FileSaveAsExcelOpenDocumentSpreadsheetAction\"/>";

				// print preview, for overlay on excel 2007 preview
				strCommandXml +=L"<command idMso=\"FilePrintPreview\" onAction=\"FilePrintPreviewAction\"/>";

				// print preview close
				strCommandXml +=L"<command idMso=\"PrintPreviewClose\" onAction=\"PrintPreviewCloseAction\"/>";
			} 
			else if ( kVer2010 == emOfficeVer )
			{
				// for 2010
			}
		}
		break;
	case kAppPPT:
		{
			// PPT insert picture, Note: we must add this only for PPT application, 
			// if we add this for excel we will can't get the ribbon event about data from access
			strCommandXml += L"<command idMso=\"PictureInsertFromFilePowerPoint\" onAction=\"PictureInsertFromFilePowerPointAction\"/>";

			// on 2007, publish-> package for cd, on 2010, save&send->package persentation for CD.
			strCommandXml += L"<command idMso=\"FilePackageForCD\" onAction=\"FilePackageForCDAction\"/>";

			// on 2007, publish->create handouts in word, on 2010, save&send->create handout
			strCommandXml += L"<command idMso=\"CreateHandoutsInWord\" onAction=\"CreateHandoutsInWordAction\"/>";

			// on 2007 ,publish->publish slides,on 2010,save&send->Publish slides
			strCommandXml += L"<command idMso=\"FilePublishSlides\" onAction=\"FilePublishSlidesAction\"/>";
			strCommandXml += L"<command idMso=\"DuplicateSelectedSlides\" onAction=\"DuplicateSelectedSlidesAction\"/>";

			if ( kVer2007 == emOfficeVer )
			{
				strCommandXml += L"<command idMso=\"FileSaveToDocumentManagementServer\" onAction=\"FileSaveToDocumentManagementServerAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsPowerPointPpsx\" onAction=\"FileSaveAsPowerPointPpsxAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsPowerPointPptx\" onAction=\"FileSaveAsPowerPointPptxAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsPowerPoint97_2003\" onAction=\"FileSaveAsPowerPoint97_2003Action\"/>";
				strCommandXml += L"<command idMso=\"FileCreateDocumentWorkspace\" onAction=\"FileCreateDocumentWorkspaceAction\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsPowerPointOpenDocumentPresentation\" onAction=\"FileSaveAsPowerPointOpenDocumentPresentationAction\"/>";
			} 
			else /*if ( kVer2010 == emOfficeVer )*/
			{
				// 2010,save&send->create a video
				strCommandXml += L"<command idMso=\"ExportToVideo\" onAction=\"ExportToVideoAction\"/>";

				// insert video
				strCommandXml += L"<command idMso=\"VideoInsert\" onAction=\"VideoInsert2PPT2010\"/>";

				// insert audio
				strCommandXml += L"<command idMso=\"AudioInsert\" onAction=\"AudioInsert2PPT2010\"/>";
				strCommandXml += L"<command idMso=\"FileSaveAsPowerPointOpenDocumentPresentation\" onAction=\"FileSaveAsPowerPointOpenDocumentPresentationAction\"/>";
				strCommandXml += L"<command idMso=\"BroadcastSlideShow\" onAction=\"BroadcastSlideShowAction\"/>";
			}
		}
		break;
	default:
		NLPRINT_DEBUGLOG( L"!!!!Seriours error ..........., please check! \n" );
		break;
	}
	strCommandXml += L"</commands>";
	return strCommandXml;
}

// this function used to add a new ribbon button for office by our PEP: NextLabs->g_szButtonName ( Right Protect )
//wstring CNxtOfficeSink::NLAddNextLabsButton( )
//{
//	// Used for NextLabs define ribbon button
//	static const wchar_t* g_szButtonName = L"Right Protected";
//	static const wchar_t* g_szButtonID	 = L"NextLabsButton";
//
//	wstring strNextLabButtonXML;
//	// code back no need this button
//#if 0	
//	strNextLabButtonXML += L" <ribbon> <tabs> <tab id=\"NextLabsTab\" label=\"NextLabs\">";
//	strNextLabButtonXML += L"    <group id=\"NextLabsGroup\"  label=\"NextLabs Group\">";
//	strNextLabButtonXML += L"     <button id=\"";
//	strNextLabButtonXML += g_szButtonID;
//	strNextLabButtonXML += L"\"";
//	strNextLabButtonXML += L" getImage=\"GetImage\" size=\"large\" label=\"";
//	strNextLabButtonXML += g_szButtonName;
//	strNextLabButtonXML += L"\" onAction=\"OnClickButton\"/>";
//	strNextLabButtonXML += L"    </group>  </tab>  </tabs>";
//	strNextLabButtonXML += L" </ribbon>";
//#endif
//	return strNextLabButtonXML;
//}

// here we register our XML event
STDMETHODIMP CNxtOfficeSink::GetCustomUI( BSTR RibbonID, BSTR * RibbonXml )
{
	if ( !RibbonXml )
		return E_POINTER;

	// writing a XML in memory on RunTime
	wstring strXML = 	
		L"<customUI xmlns=\"http://schemas.microsoft.com/office/2006/01/customui\">" + 
			GetOfficeCommandXml() + 
			/*NLAddNextLabsButton() + */
		L"</customUI>";

	*RibbonXml = SysAllocString( strXML.c_str() );

	return *RibbonXml ? S_OK : E_OUTOFMEMORY ;
}

HRESULT CNxtOfficeSink::FinalConstruct()
{
	//LoadCBModule();
	return S_OK;
}

void CNxtOfficeSink::FinalRelease()
{
	/*
	* Fix Bug24624
	* We don't need free CBPEP module, because if we do this, sometime close word NXL file will hang, we test close NXL file about 100 times have 1 hang case.
	* For this bug, it hanged at CEPEP module when it unHook API "CoCreateInstance".
	* Office pep also hook this function maybe re-hook this function cause this bug. 
	*/
#if 0 
	// we need delete this code because it cause the bug24624
	UnloadCBModule();
#endif
}

//void CNxtOfficeSink::LoadCBModule()
//{
//	if ( NULL != m_hCBHandle )
//	{
//		return ;
//	}
//
//	typedef void (*DoInit)();
//	DoInit fnDoInit;
//
//	wchar_t szModulePath[1024]={0};
//	GetModuleFileNameW(g_hInstance,szModulePath,1023);
//	
//	// fix bug24666, here make sure our office pep always in process
//	NLPRINT_DEBUGLOG( L"the module path is:[%s] \n", szModulePath );
//	if ( NULL == LoadLibraryW( szModulePath ) )
//	{
//		NLPRINT_DEBUGLOG( L"the module path is:[%s], load ourself failed \n", szModulePath );
//	}
//	else
//	{
//		NLPRINT_DEBUGLOG( L"the module path is:[%s], load ourself success \n", szModulePath );
//	}
//
//	wchar_t* pPos = wcsrchr(szModulePath, L'\\');
//	
//	if ( NULL == pPos )
//	{
//		return ;
//	}
//	*++pPos = L'\0';
//	
//	std::wstring strCommonPath = szModulePath;
//#ifdef _WIN64
//	strCommonPath += L"cbPep.dll";
//#else
//	strCommonPath += L"cbPep32.dll";
//#endif
//	
//	m_hCBHandle = LoadLibraryW(strCommonPath.c_str());
//	if ( m_hCBHandle )
//	{
//		fnDoInit = (DoInit)GetProcAddress(m_hCBHandle, "DoInit");
//		fnDoInit();
//	}
//	else
//	{
//		NLPRINT_DEBUGLOG( L"Load CB module [%s], last error is [%d]..............\n", strCommonPath.c_str(), GetLastError() );
//	}
//}
//
//void CNxtOfficeSink::UnloadCBModule()
//{
//	if( NULL != m_hCBHandle )
//	{
//		FreeLibrary(m_hCBHandle);
//		m_hCBHandle = NULL;
//	}
//}


// response our ribbon button: NextLabs->g_szButtonName ( Right Protect )
//STDMETHODIMP CNxtOfficeSink::OnClickButton(IDispatch* pCtrl)
//{
//	static const wchar_t* g_szButtonName = L"Right Protected";
//	static const wchar_t* g_szButtonID = L"NextLabsButton";
//
//	CComQIPtr<Office::IRibbonControl> ptrCtrl( pCtrl );
//	ATLASSERT( ptrCtrl );
//	
//	BSTR bstr = NULL;
//	HRESULT hr = ptrCtrl->get_Id( &bstr );
//	if ( FAILED(hr) || bstr == NULL )	return E_FAIL;
//
//	if ( 0 == _wcsicmp( bstr, g_szButtonID ) )
//	{
//		STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( emOfficeRibbonNextLabsRibbonRightProtected, emOfficeUnknownType );
//		NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
//	}
//	::SysFreeString( bstr );
//	
//	return S_OK;
//}
//
//STDMETHODIMP CNxtOfficeSink::GetImage(IDispatch* pCtrl, IPictureDisp** pImage)
//{
//	HRESULT hr = S_OK;
//	HANDLE hBitMap = NULL;
//	PICTDESC d_pic;
//	d_pic.cbSizeofstruct = sizeof(PICTDESC);
//	d_pic.picType = PICTYPE_BITMAP;
//
//	hBitMap =(HBITMAP)::LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_NXTICON),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);
//	if(hBitMap==NULL)
//	{
//		return E_NOTIMPL;
//	}
//
//	d_pic.bmp.hbitmap = (HBITMAP)hBitMap;
//	hr = OleCreatePictureIndirect(&d_pic,IID_IPictureDisp,true,(void**)pImage);
//
//	return hr;
//}

//////////////////////////////////////////////////////////////////////////
//	Custom action event
//////////////////////////////////////////////////////////////////////////
// save[Edit]
STDMETHODIMP CNxtOfficeSink::FileSaveAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	g_bSavePressed = TRUE;
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSave, kFileUnknown );	// Save: 1. new document-> save as or convert; 2. Save
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/* ************************************* word 2007 saveas ****************************** */
// save as[copy or convert depend on the types of the src and dest] 2007/2010
STDMETHODIMP CNxtOfficeSink::FileSaveAsAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kFileUnknown );	// SaveAs: 1. save as or convert; 2. New document
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;
	
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
/////// 2007 word save as ////////////////////////////////////////////////
// save as->word document[copy or convert depend on the types of the src and dest] 2007
STDMETHODIMP CNxtOfficeSink::FileSaveAsWordDocxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kDocx );	// Only for office2007 SaveAs: 1. save as or convert; 2. New document
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// save as->word template[copy or convert depend on the types of the src and dest] 2007
STDMETHODIMP CNxtOfficeSink::FileSaveAsWordDotxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kDotx );	// Only for office2007 SaveAs: 1. save as or convert; 2. New document
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// save as->word 97-2003 document[copy or convert depend on the types of the src and dest] 2007
STDMETHODIMP CNxtOfficeSink::FileSaveAsWord97_2003Action(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kDoc );	// Only for office2007 SaveAs: 1. save as or convert; 2. New document
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// 2007 word. saveas->open document text
STDMETHODIMP CNxtOfficeSink::FileSaveAsWordOpenDocumentTextAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kOdt );	// Only for office2007 SaveAs: 1. save as or convert; 2. New document
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// save as XPS or PDF [convert] 
STDMETHODIMP CNxtOfficeSink::FileSaveAsPdfOrXpsAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonConvert, kPDF );	// office2007,save as XPS/PDF the default type is PDF.
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;
	
	return S_OK;
}

// save as->other formats[convert] 2007
STDMETHODIMP CNxtOfficeSink::FileSaveAsOtherFormatsAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kFileUnknown );	// office2007,save as other format but the default type is current file type.
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/**********************************  word 2007  publish ************************* */
// publish->Document management server[send] (word/excel/ppt)
STDMETHODIMP CNxtOfficeSink::FileSaveToDocumentManagementServerAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );	// publish is send action and send action we don't case the destination path.
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// word:publish->blog[send]
STDMETHODIMP CNxtOfficeSink::FileNewBlogPostAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );	// publish is send action and send action we don't case the destination path.
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// publish->create document workspace[send] (word/excel/ppt)
STDMETHODIMP CNxtOfficeSink::FileCreateDocumentWorkspaceAction(IDispatch* RibbonControl, VARIANT_BOOL *pvarfPressed, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );	// publish is send action and send action we don't case the destination path.
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/* ************************************* send ************************************************* */
// send->E-mail[send]
STDMETHODIMP CNxtOfficeSink::FileSendAsAttachmentAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );	// send action
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// send->E-mail as pdf attachment[send]
STDMETHODIMP CNxtOfficeSink::FileEmailAsPdfEmailAttachmentAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );	// send action
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// send->E-mail as xps attachment[send]
STDMETHODIMP CNxtOfficeSink::FileEmailAsXpsEmailAttachmentAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );	// send action
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// send->Internet Fax[send]
STDMETHODIMP CNxtOfficeSink::FileInternetFaxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );	// send action
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/* *****************************************¡¡insert ************************************* */
// insert->Object->Text From file[convert] only for word
STDMETHODIMP CNxtOfficeSink::TextFromFileInsertAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonInsert, kInsertText_WordOnly );	// only word has this ribbon event
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// insert->object->create from file
STDMETHODIMP CNxtOfficeSink::OleObjectctInsertAction( IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonInsert, kInsertObj );	
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// insert->picture[convert] ppt
STDMETHODIMP CNxtOfficeSink::PictureInsertFromFilePowerPointAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonInsert, kInsertPic );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// insert->picture[convert], word excel
STDMETHODIMP CNxtOfficeSink::PictureInsertFromFileAction(IDispatch* RidbbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonInsert, kInsertPic );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/****************************************** copy content **************************** */
//cut[copy content]
STDMETHODIMP CNxtOfficeSink::CutAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPasteContent, emCutContent );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//copy[copy content]
STDMETHODIMP CNxtOfficeSink::CopyAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPasteContent, emCopyContent );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// Excel: Home->Copy->Copy as a picture
STDMETHODIMP CNxtOfficeSink::CopyAsPictureAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPasteContent, emCopyAsAPicture );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

STDMETHODIMP CNxtOfficeSink::SheetMoveOrCopyAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPasteContent, emMoveOrCopy);
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// only for PPT 
STDMETHODIMP CNxtOfficeSink::DuplicateSelectedSlidesAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPasteContent, emCopySelectedSlides );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/* *********************************** excel 2007 saveas ************************************ */
//Excel 2007: save as->Excel workbook[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsExcelXlsxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kXlsx );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//Excel 2007: save as->Excel Macro-enabled workbook[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsExcelXlsxMacroAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kXlsm );	// XLSX macro: file type XLSM 
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;
	return S_OK;
}

//Excel 2007: save as->Excel 97-2003 workbook[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsExcel97_2003Action(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kXls );	// 97-2003 excel file type: xls
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//save as->excel binary workbook[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsExcelXlsbAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kXlsb );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//Excel.....it does not exist for some version of office[copy or convert depend on the types of the src and destination and convert action doesn't care the destination path
STDMETHODIMP CNxtOfficeSink::FileSaveAsExcelOpenDocumentSpreadsheetAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kOds );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/**************************************** excel 2007 publish *******************************/
//Excel 2007: publish->Excel services[send]
STDMETHODIMP CNxtOfficeSink::FilePublishExcelServicesAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}
/* ******************************** excel data *************************************** */
//excel data->get external data->From Access[convert]
STDMETHODIMP CNxtOfficeSink::GetExtDataFromAccess(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonExcelDataTab, kInsertDataFromAccess_ExcelOnly );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//data->get external data->From text[convert]
STDMETHODIMP CNxtOfficeSink::GetExtDataFromText(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonExcelDataTab, kDataFromText_ExcelOnly );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/*************************** ppt2007 saveas  *******************************************/
//save as->powerpoint show[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsPowerPointPpsxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kPpsx );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//save as->powerpoint persentation[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsPowerPointPptxAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, lPptx );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//....it does not exist in some version of office[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsPowerPointOpenDocumentPresentationAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kOdp );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//saveas->powerpoint 97-2003 persentation[copy or convert depend on the types of the src and dest]
STDMETHODIMP CNxtOfficeSink::FileSaveAsPowerPoint97_2003Action(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSaveAs, kPpt ); // 97-2003 PPT file type is: .ppt
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/* ********************************* ppt2007 publish ************************ */ 
//publish->package for cd[send]
STDMETHODIMP CNxtOfficeSink::FilePackageForCDAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//publish->publish slides[send]
STDMETHODIMP CNxtOfficeSink::FilePublishSlidesAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

//publish->create handouts for word[send]
STDMETHODIMP CNxtOfficeSink::CreateHandoutsInWordAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonPublish, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// PPT: Slide show -> broadcast slide show, we do this as send action
STDMETHODIMP CNxtOfficeSink::BroadcastSlideShowAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}
//////////////////////////////////////////////////////////////////////////

// Office: word/excel/ppt >: view->new window, this is only used for overlay 
STDMETHODIMP CNxtOfficeSink::WindowNewAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonViewNewWindow, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}
STDMETHODIMP CNxtOfficeSink::FilePrintPreviewAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	//CNLGlobalInfo& theGlobalIns = CNLGlobalInfo::NLGetInstance();
	if(pep::appType()==kAppExcel && pep::getVersion() == kVer2007)
	{
		NLVIEWOVERLAYINSTANCE.SendPrintPreview(true);
	}
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}
STDMETHODIMP CNxtOfficeSink::PrintPreviewCloseAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	//CNLGlobalInfo& theGlobalIns = CNLGlobalInfo::NLGetInstance();
	if (pep::appType() == kAppExcel && pep::getVersion() == kVer2007)
	{
		NLVIEWOVERLAYINSTANCE.SendPrintPreview(false);
	}
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

// view->zoom: word: one page, two page, page width, text width, whole page, Magnification 75%, 100%, 200%, ... and so on.
STDMETHODIMP CNxtOfficeSink::ZoomCurrent100Action(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonViewZoom, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

STDMETHODIMP CNxtOfficeSink::ZoomOnePageAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonViewZoom, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

STDMETHODIMP CNxtOfficeSink::ZoomTwoPagesAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonViewZoom, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

STDMETHODIMP CNxtOfficeSink::ZoomPageWidthAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonViewZoom, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

// PPT 2010: save&send->Create a video->create a video[send]
STDMETHODIMP CNxtOfficeSink::ExportToVideoAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	STUNLOFFICE_RIBBONEVENT stuOfficeXMLRibbon( kRibbonSend, kFileUnknown );
	NLACTIONINSTANCE.NLSetEventForXMLRibbon( &stuOfficeXMLRibbon );
	NULL == cancel ? NULL : cancel->boolVal = stuOfficeXMLRibbon.bVariantCancel;

	return S_OK;
}

/******************************* no using any more begin, we don't care the following action ************************************/
//data->get external data->From existing connections[convert],doesn't care.
STDMETHODIMP CNxtOfficeSink::GetExtDataFromExistConn(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//data->get external data->From Web[convert], doesn't care
STDMETHODIMP CNxtOfficeSink::GetExtDataFromWeb(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//Insert->media->Video[convert]
STDMETHODIMP CNxtOfficeSink::VideoInsert2PPT2010(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//Insert->media->audio[convert]
STDMETHODIMP CNxtOfficeSink::AudioInsert2PPT2010(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//office2010: info->property->advanced properties
STDMETHODIMP CNxtOfficeSink::AdvancedFilePropertiesAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

// prepare->properties
STDMETHODIMP CNxtOfficeSink::FilePropertiesAction(IDispatch* RibbonControl, VARIANT_BOOL *pvarfPressed, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//office2010: insert->illustrations->screenshot->screen clipping[convert]
STDMETHODIMP CNxtOfficeSink::ScreenClippingAction(IDispatch* RibbonControl, VARIANT* cancel)
{
    std::wstring DisplayText;
    DWORD processID = 0;
    bool bAllow = CNxtHookAPI::Query(DisplayText, processID);
    if (!bAllow)
    {
        cancel->boolVal = VARIANT_TRUE;
    }
    else
    {
        cancel->boolVal = VARIANT_FALSE;
    }
	
	return S_OK;
}

//paste
STDMETHODIMP CNxtOfficeSink::PasteAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

// print[print], we handle it by hook API instead of here for print overlay
STDMETHODIMP CNxtOfficeSink::FilePrintAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}

//quick print[print]
STDMETHODIMP CNxtOfficeSink::FilePrintQuickAction(IDispatch* RibbonControl, VARIANT* cancel)
{
	cancel->boolVal = VARIANT_FALSE;
	return S_OK;
}
/******************************* no using any more end ************************************/
