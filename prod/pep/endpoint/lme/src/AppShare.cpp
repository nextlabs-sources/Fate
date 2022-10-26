#include "stdafx.h"
#include "AppShare.h"
#include "OfficeShare.h"
#include "PartDB.h"
#include "PCStream.h"

//hook CollaborateAppShare
INSTANCE_DEFINE( CHookedCollAppShare );

void CHookedCollAppShare::Hook( void* pAppShare )
{
    //SubstituteOrgFuncWithNew( pAppShare, 6, (void*)NewAppShareInvoke);
    //SubstituteOrgFuncWithNew( pAppShare, 12, (void*)NewAppShareOnNotify);
    //SubstituteOrgFuncWithNew( pAppShare, 13, (void*)New_AppShare_Scraper);
    //SubstituteOrgFuncWithNew( pAppShare, 14, (void*)New_AppShare_ScraperFile);
    //SubstituteOrgFuncWithNew( pAppShare, 15, (void*)New_AppShare_Viewer);
    SubstituteOrgFuncWithNew( pAppShare, 16, (void*)New_AppShare_GetSnapshot);   
    
    DoHook( pAppShare );
}
//////////////////////////////////////////////////////////////////////////
//HRESULT __stdcall CHookedCollAppShare::New_AppShare_Scraper(ICollaborateAppShare* pAppShare,ICollaborateScraper** varScraper)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_AppShare_Scraper pFunc = (Old_AppShare_Scraper)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_Scraper ));
//    if( !pFunc )
//    {
//		DP((L"New_AppShare_Scraper")) ;
//        GetInstance()->Hook( pAppShare );
//        pFunc = (Old_AppShare_Scraper)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_Scraper ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pAppShare,varScraper);
//    }	
//
//	//////////////////////////////////////////////////////////////////////////
//	//if(SUCCEEDED(hr) && (*varScraper) != NULL)
//	//{
//	//	ICollaborateScraper* pScraper = (*varScraper);
//	//	BSTR bstrInfo=NULL;
//	//	HRESULT hr = pScraper->get_TypeOf(&bstrInfo);
//	//	
//	//	// here we need hook on Scraper
// //       CHookedCollScraper::GetInstance()->Hook( PVOID(pScraper) );
//	//}
//	//////////////////////////////////////////////////////////////////////////
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollAppShare::New_AppShare_ScraperFile(ICollaborateAppShare* pAppShare,ICollaborateScraperFile** varScraperFile)
//{
//	DP((L"New_AppShare_ScraperFile")) ;
//	HRESULT hr = E_NOTIMPL;
//    Old_AppShare_ScraperFile pFunc = (Old_AppShare_ScraperFile)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_ScraperFile ));
//    if( !pFunc )
//    {
//		DP((L"1111111111111111111111111111")) ;
//        GetInstance()->Hook( pAppShare );
//        pFunc = (Old_AppShare_ScraperFile)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_ScraperFile ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pAppShare,varScraperFile);
//    }	
//
//	//////////////////////////////////////////////////////////////////////////
//	//if(SUCCEEDED(hr) && (*varScraperFile) != NULL)
//	//{
//	//	BSTR strType = NULL;
//	//	ICollaborateScraperFile* pScraperFile = (*varScraperFile);
//	//	HRESULT hr = pScraperFile->get_TypeOf(&strType);
//	//	
//	//	// hook ScraperFile
//	//	DP((L"333333333333333333333333333")) ;
// //       CHookedCollScraperFile::GetInstance()->Hook( PVOID(pScraperFile) ); 
//	//}
//	//////////////////////////////////////////////////////////////////////////
//	return hr;
//}

//HRESULT __stdcall CHookedCollAppShare::New_AppShare_Viewer(ICollaborateAppShare* pAppShare,ICollaborateViewer** varViewer)
//{
//	HRESULT hr = E_NOTIMPL;
//	DP((L"New_AppShare_Viewer")) ;
//    Old_AppShare_Viewer pFunc = (Old_AppShare_Viewer)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_Viewer ));
//    if( !pFunc )
//    {
//		DP((L"222222222222222222222222222222")) ;
//        GetInstance()->Hook( pAppShare );
//        pFunc = (Old_AppShare_Viewer)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_Viewer ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pAppShare,varViewer);
//    }	
//	//////////////////////////////////////////////////////////////////////////
//	//if(SUCCEEDED(hr) && (*varViewer) != NULL)
//	//{
//	//	BSTR strType = NULL;
//	//	ICollaborateViewer* pViewer = (*varViewer);
//	//	HRESULT hr = pViewer->get_TypeOf(&strType);
//	//	
//	//	// hook Viewer
// //       CHookedCollViewer::GetInstance()->Hook( PVOID(pViewer) );
//	//}
//	//////////////////////////////////////////////////////////////////////////
//
//	return hr;
//}

HRESULT __stdcall CHookedCollAppShare::New_AppShare_GetSnapshot(ICollaborateAppShare* pAppShare,tagRECT* prcRegion, LMC_AppShare::wireHBITMAP* phbmpCaptured)
{	
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		 Old_AppShare_GetSnapshot pFunc = (Old_AppShare_GetSnapshot)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_GetSnapshot ));
		if( pFunc )
		{
			return pFunc(pAppShare,prcRegion, phbmpCaptured);
		}

	}
	__try
	{
		return my_AppShare_GetSnapshot(pAppShare,prcRegion, phbmpCaptured);
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollAppShare::my_AppShare_GetSnapshot(ICollaborateAppShare* pAppShare,tagRECT* prcRegion, LMC_AppShare::wireHBITMAP* phbmpCaptured)
{
	HRESULT hr = E_NOTIMPL;
    Old_AppShare_GetSnapshot pFunc = (Old_AppShare_GetSnapshot)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_GetSnapshot ));
    if( !pFunc )
    {
		DP((L"444444444444444444444444444")) ;
        GetInstance()->Hook( pAppShare );
        pFunc = (Old_AppShare_GetSnapshot)(GetInstance()->GetOrgFunc( (void*)pAppShare, New_AppShare_GetSnapshot ));
    }
    
	//CE_ACTION_WM_SHARE
	//if( DoEvaluate( TEXT( "[LM_ScreenSnapshot]" ), L"SHARE") )//MsgBoxAllowOrDeny( CPartDB::GetInstance()->GetPresenterAttendeeInfo().c_str(), L"Allow to share Snapshot?" ) ) 
	if(  DoAppEvaluate( LME_MAGIC_STRING ,L"SCREENSHOT",L"SHARE") ) 
    {
        if( pFunc )
        {
            hr = pFunc(pAppShare,prcRegion, phbmpCaptured);
        }	
    }
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//INSTANCE_DEFINE( CHookedCollScraperFile );
//
//void CHookedCollScraperFile::Hook( void* pScraperFile )
//{
//    SubstituteOrgFuncWithNew( pScraperFile, 6, (void*)NewScraperFileInvoke);
//    SubstituteOrgFuncWithNew( pScraperFile, 12, (void*)NewScraperFileOnNotify);    
//    SubstituteOrgFuncWithNew( pScraperFile, 14, (void*)New_ScraperFile_get_Share);
//    SubstituteOrgFuncWithNew( pScraperFile, 15, (void*)New_ScraperFile_put_Share);
//    SubstituteOrgFuncWithNew( pScraperFile, 18, (void*)New_ScraperFile_Init);
//    SubstituteOrgFuncWithNew( pScraperFile, 19, (void*)New_ScraperFile_Uninit);
//    SubstituteOrgFuncWithNew( pScraperFile, 20, (void*)New_ScraperFile_SendQosPacket);
//    DoHook( pScraperFile );
//}
//
////////////////////////////////////////////////////////////////////////////
//
//HRESULT __stdcall CHookedCollScraperFile::New_ScraperFile_get_Share(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType* varShare)
//{
//	HRESULT hr = E_NOTIMPL;
//	DP((L"New_ScraperFile_get_Share")) ;
//    Old_ScraperFile_get_Share pFunc = (Old_ScraperFile_get_Share)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_get_Share ));
//    if( !pFunc )
//    {
//		DP((L"555555555555555555555555555555")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (Old_ScraperFile_get_Share)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_get_Share ));
//    }
//
//    if( pFunc && IsExecute( L"ScraperFile_get_Share" ) )
//    {
//        hr = pFunc(pScraperFile,varShare);
//    }
//	return hr;
//}
//HRESULT __stdcall CHookedCollScraperFile::New_ScraperFile_put_Share(ICollaborateScraperFile*pScraperFile,LMC_AppShare::ShareStateType varShare)
//{
//	HRESULT hr = E_NOTIMPL;
//	DP((L"New_ScraperFile_put_Share")) ;
//    Old_ScraperFile_put_Share pFunc = (Old_ScraperFile_put_Share)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_put_Share ));
//    if( !pFunc )
//    {
//		DP((L"77777777777777777777777777777777")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (Old_ScraperFile_put_Share)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_put_Share ));
//    }
//
//    if( !IsExecute( L"ScraperFile_put_Share" ) )
//    {
//        pFunc = 0;
//    }
//
//	if(pFunc == NULL)
//	{
//		return hr;
//	}
//	hr = pFunc(pScraperFile,varShare);
//	return hr;
//}
//
//HRESULT  __stdcall CHookedCollScraperFile::New_ScraperFile_Init(ICollaborateScraperFile* pScraperFile,
//										IUnknown* pStream, 
//										BSTR strFile, 
//										double dblSpeed, 
//										unsigned long nTimeToPlay)
//{
//	HRESULT hr = E_NOTIMPL;
//	DP((L"New_ScraperFile_Init")) ;
//    Old_ScraperFile_Init pFunc = (Old_ScraperFile_Init)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_Init ));
//    if( !pFunc )
//    {
//		DP((L"777777777777ssss77777777777777777")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (Old_ScraperFile_Init)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_Init ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraperFile,pStream, strFile, dblSpeed, nTimeToPlay );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollScraperFile::New_ScraperFile_Uninit(ICollaborateScraperFile* pScraperFile,IErrorInfo* pErrorInfo)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_ScraperFile_Uninit pFunc = (Old_ScraperFile_Uninit)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_Uninit ));
//    if( !pFunc )
//    {
//		DP((L"9999999999999999999999999999999")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (Old_ScraperFile_Uninit)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_Uninit ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraperFile, pErrorInfo );
//    }
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollScraperFile::New_ScraperFile_SendQosPacket(ICollaborateScraperFile* pScraperFile,unsigned long PacketId)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_ScraperFile_SendQosPacket pFunc = (Old_ScraperFile_SendQosPacket)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_SendQosPacket ));
//    if( !pFunc )
//    {
//		DP((L"1212121212")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (Old_ScraperFile_SendQosPacket)(GetInstance()->GetOrgFunc( (void*)pScraperFile, New_ScraperFile_SendQosPacket ));
//    }
//
//    if( pFunc && IsExecute( L"ScraperFile_SendQosPacket" ) )
//    {
//        hr = pFunc(pScraperFile, PacketId );
//    }
//	return hr;
//}
//////////////////////////////////////////////////////////////////////////

//INSTANCE_DEFINE( CHookedCollScraper );
//
//void CHookedCollScraper::Hook( void* pScraper )
//{
//    SubstituteOrgFuncWithNew( pScraper, 6, (void*)NewScraperInvoke);
//    SubstituteOrgFuncWithNew( pScraper, 14, (void*)New_Scraper_get_Share);
//    SubstituteOrgFuncWithNew( pScraper, 15, (void*)New_Scraper_put_Share);
//    SubstituteOrgFuncWithNew( pScraper, 18, (void*)New_CollScraper_Init);
//    SubstituteOrgFuncWithNew( pScraper, 19, (void*)NewScraperGet_Annotate);
//    SubstituteOrgFuncWithNew( pScraper, 20, (void*)NewScraperPut_Annotate);
//
//    DoHook( pScraper );
//}
////////////////////////////////////////////////////////////////////////////
//
//HRESULT __stdcall CHookedCollScraper::New_Scraper_put_Share(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType varShare)
//{
//
//	HRESULT hr = E_NOTIMPL;
//    Old_Scraper_put_Share pFunc = (Old_Scraper_put_Share)(GetInstance()->GetOrgFunc( (void*)pScraper, New_Scraper_put_Share ));
//    if( !pFunc )
//    {
//		DP((L"1313131313131")) ;
//        GetInstance()->Hook( pScraper );
//        pFunc = (Old_Scraper_put_Share)(GetInstance()->GetOrgFunc( (void*)pScraper, New_Scraper_put_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraper, varShare );
//    }
//
//	switch(varShare)
//	{
//	case LMC_AppShare::SHARE_UNINIT:
//		{
//			
//		}
//		break;
//	case LMC_AppShare::SHARE_PAUSE:
//		{
//			
//		}
//		break;
//	case LMC_AppShare::SHARE_STOP:
//		{
//			
//		}
//		break;
//	case LMC_AppShare::SHARE_PLAY:
//		{
//			//sprintf_s(strInfo,sizeof(strInfo),"SHARE_PLAY !\r\n");
//			//OutPutLog(strInfo,0);
//			////varShare = LMC_AppShare::SHARE_UNINIT;
//		}
//		break;
//	}
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollScraper::New_Scraper_get_Share(ICollaborateScraper*pScraper,LMC_AppShare::ShareStateType* varShare)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_Scraper_get_Share pFunc = (Old_Scraper_get_Share)(GetInstance()->GetOrgFunc( (void*)pScraper, New_Scraper_get_Share ));
//    if( !pFunc )
//    {
//		DP((L"645645454545454545")) ;
//        GetInstance()->Hook( pScraper );
//        pFunc = (Old_Scraper_get_Share)(GetInstance()->GetOrgFunc( (void*)pScraper, New_Scraper_get_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraper, varShare );
//    }
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollScraper::New_CollScraper_Init(ICollaborateScraper*pScraper, IUnknown* pStream)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollScraper_Init pFunc = (Old_CollScraper_Init)(GetInstance()->GetOrgFunc( (void*)pScraper, New_CollScraper_Init ));
//    if( !pFunc )
//    {
//		DP((L"6576778989")) ;
//        GetInstance()->Hook( pScraper );
//        pFunc = (Old_CollScraper_Init)(GetInstance()->GetOrgFunc( (void*)pScraper, New_CollScraper_Init ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraper, pStream );
//    }
//	//if(SUCCEEDED(hr))
//	//{
//	//	INpwRC_Stream* pRCStream=NULL;
//	//	hr = pStream->QueryInterface(IID_INpwRC_Stream,(void**)&pRCStream);
//	//	if(SUCCEEDED(hr) && pRCStream != NULL)
//	//	{
//	//		DP((L"856756757576456")) ;
// //           CHookedStream::GetInstance()->Hook( (PVOID)pRCStream );// HookStream(pRCStream);
//	//	}
//	//}
//
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollScraper::NewScraperPut_Annotate ( ICollaborateScraper*pScraper,
//                                          /*[in]*/ VARIANT_BOOL varAnnotate )
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_Put_Annotate pFunc = (Old_Put_Annotate)(GetInstance()->GetOrgFunc( (void*)pScraper, NewScraperPut_Annotate ));
//    if( !pFunc )
//    {
//		DP((L"11111111112322222222222222222")) ;
//        GetInstance()->Hook( pScraper );
//        pFunc = (Old_Put_Annotate)(GetInstance()->GetOrgFunc( (void*)pScraper, NewScraperPut_Annotate ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraper, varAnnotate );
//    }
//
//    return hr;
//}
//
//HRESULT __stdcall CHookedCollScraper::NewScraperGet_Annotate ( ICollaborateScraper*pScraper,
//                                          /*[in]*/ VARIANT_BOOL* varAnnotate )
//{
//    HRESULT hr = E_NOTIMPL;
//    Old_Get_Annotate pFunc = (Old_Get_Annotate)(GetInstance()->GetOrgFunc( (void*)pScraper, NewScraperGet_Annotate ));
//    if( !pFunc )
//    {
//		DP((L"34444444444444444442222222222222222")) ;
//        GetInstance()->Hook( pScraper );
//        pFunc = (Old_Get_Annotate)(GetInstance()->GetOrgFunc( (void*)pScraper, NewScraperGet_Annotate ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraper, varAnnotate );
//    }
//
//    return hr;
//}

//////////////////////////////////////////////////////////////////////////
//INSTANCE_DEFINE( CHookedCollViewer );
//
//void CHookedCollViewer::Hook( void* pViewer )
//{
//    SubstituteOrgFuncWithNew( pViewer, 13, (void*)New_CollViewer_Init);
//    SubstituteOrgFuncWithNew( pViewer, 15, (void*)New_CollViewer_Get_ViewerPanel);
//    SubstituteOrgFuncWithNew( pViewer, 6, (void*)NewViewerInvoke);
//    DoHook( pViewer );
//}
////////////////////////////////////////////////////////////////////////////
//#include "MsgHook.h"
//HRESULT __stdcall CHookedCollViewer::New_CollViewer_Init(ICollaborateViewer*pViewer, IUnknown* pStream)
//{
//    //CMsgHook::GetInstance()->SetMsgHook( GetWindowThreadProcessId(GetWindowHwd(), NULL) );
//
//	HRESULT hr = E_NOTIMPL;
//    Old_CollViewer_Init pFunc = (Old_CollViewer_Init)(GetInstance()->GetOrgFunc( (void*)pViewer, New_CollViewer_Init ));
//    if( !pFunc )
//    {
//		DP((L"999999999999999999997777777777777777")) ;
//        GetInstance()->Hook( pViewer );
//        pFunc = (Old_CollViewer_Init)(GetInstance()->GetOrgFunc( (void*)pViewer, New_CollViewer_Init ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewer, pStream );
//    }
//
//    //if(SUCCEEDED(hr))
//    //{
//    //    INpwRC_Stream* pRCStream=NULL;
//    //    hr = pStream->QueryInterface(IID_INpwRC_Stream,(void**)&pRCStream);
//    //    if(SUCCEEDED(hr) && pRCStream != NULL)
//    //    {
//    //        CHookedStream::GetInstance()->Hook( (PVOID)pRCStream );// HookStream(pRCStream);
//    //    }
//    //}
//	return hr;
//}
//
//HRESULT __stdcall CHookedCollViewer::New_CollViewer_Get_ViewerPanel (
//    ICollaborateViewer*pViewer,
///*[out,retval]*/ struct ICollaborateViewerPanel * * varViewerPanel )
//{
//	DP((L"5223678r23423")) ;
//    HRESULT hr = E_NOTIMPL;
//    CollViewer_Get_ViewerPanelFunc pFunc = (CollViewer_Get_ViewerPanelFunc)(GetInstance()->GetOrgFunc( (void*)pViewer, New_CollViewer_Get_ViewerPanel ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pViewer );
//        pFunc = (CollViewer_Get_ViewerPanelFunc)(GetInstance()->GetOrgFunc( (void*)pViewer, New_CollViewer_Get_ViewerPanel ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewer, varViewerPanel );
//      /*  if( S_OK == hr )
//        {
//            CHookedViewerPanel::GetInstance()->Hook( PVOID( *varViewerPanel ) );
//        }*/
//    }
//    return hr;
//}
////////////////////////////////////////////////////////////////////////////
//
//
//HRESULT __stdcall CHookedCollScraperFile::NewScraperFileOnNotify (
//                               ICollaborateScraperFile* pScraperFile,
///*[in]*/ struct IBaseEvent * pEvent )
//{
//    HRESULT hr = E_NOTIMPL;
//    OnNotifyFunc pFunc = (OnNotifyFunc)(GetInstance()->GetOrgFunc( (void*)pScraperFile, NewScraperFileOnNotify ));
//    if( !pFunc )
//    {
//		DP((L"sfddddddddddddddd")) ;
//        GetInstance()->Hook( pScraperFile );
//        pFunc = (OnNotifyFunc)(GetInstance()->GetOrgFunc( (void*)pScraperFile, NewScraperFileOnNotify ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pScraperFile, pEvent );
//    }
//    return hr;
//}

HRESULT __stdcall CHookedCollAppShare::NewAppShareOnNotify (
                                       ICollaborateAppShare* pThis,
/*[in]*/ struct IBaseEvent * pEvent )
{	
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
		   AppShareOnNotifyFunc pFunc = (AppShareOnNotifyFunc)(GetInstance()->GetOrgFunc( (void*)pThis, NewAppShareOnNotify ));
		if( pFunc )
		{
			return   pFunc(pThis, pEvent );
		}

	}
	__try
	{
		return  myAppShareOnNotify(pThis, pEvent );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollAppShare::myAppShareOnNotify (
                                       ICollaborateAppShare* pThis,
/*[in]*/ struct IBaseEvent * pEvent )
{
    HRESULT hr = E_NOTIMPL;

    AppShareOnNotifyFunc pFunc = (AppShareOnNotifyFunc)(GetInstance()->GetOrgFunc( (void*)pThis, NewAppShareOnNotify ));
    if( !pFunc )
    {
		DP((L"sddddddddddddddddddddawaaaaaaaaaaaaaaaaaa")) ;
        GetInstance()->Hook( pThis );
        pFunc = (AppShareOnNotifyFunc)(GetInstance()->GetOrgFunc( (void*)pThis, NewAppShareOnNotify ));
    }

    if( pFunc )
    {
        hr = pFunc(pThis, pEvent );
    }
    return hr;
}


HRESULT __stdcall CHookedCollAppShare::NewAppShareInvoke( ICollaborateAppShare* pThis,
                                    /* [in] */ DISPID dispIdMember,
                                    /* [in] */ REFIID riid,
                                    /* [in] */ LCID lcid,
                                    /* [in] */ WORD wFlags,
                                    /* [out][in] */ DISPPARAMS *pDispParams,
                                    /* [out] */ VARIANT *pVarResult,
                                    /* [out] */ EXCEPINFO *pExcepInfo,
                                    /* [out] */ UINT *puArgErr ) 
{
	pThis ;
	dispIdMember ;
	riid;
	lcid;
	wFlags;
		pDispParams;
	pVarResult;
	pExcepInfo;
	puArgErr ; 
	return S_OK;
}
//
//HRESULT __stdcall CHookedCollScraperFile::NewScraperFileInvoke( ICollaborateScraperFile* pThis,
//                                       /* [in] */ DISPID dispIdMember,
//                                       /* [in] */ REFIID riid,
//                                       /* [in] */ LCID lcid,
//                                       /* [in] */ WORD wFlags,
//                                       /* [out][in] */ DISPPARAMS *pDispParams,
//                                       /* [out] */ VARIANT *pVarResult,
//                                       /* [out] */ EXCEPINFO *pExcepInfo,
//                                       /* [out] */ UINT *puArgErr ) 
//{
//    return S_OK;
//}
//
//HRESULT __stdcall CHookedCollScraper::NewScraperInvoke( ICollaborateScraper* pThis,
//                                   /* [in] */ DISPID dispIdMember,
//                                   /* [in] */ REFIID riid,
//                                   /* [in] */ LCID lcid,
//                                   /* [in] */ WORD wFlags,
//                                   /* [out][in] */ DISPPARAMS *pDispParams,
//                                   /* [out] */ VARIANT *pVarResult,
//                                   /* [out] */ EXCEPINFO *pExcepInfo,
//                                   /* [out] */ UINT *puArgErr ) 
//{
//    return S_OK;
//}


//
//HRESULT __stdcall CHookedCollViewer::NewViewerInvoke( ICollaborateViewer* pThis,
//                             /* [in] */ DISPID dispIdMember,
//                             /* [in] */ REFIID riid,
//                             /* [in] */ LCID lcid,
//                             /* [in] */ WORD wFlags,
//                             /* [out][in] */ DISPPARAMS *pDispParams,
//                             /* [out] */ VARIANT *pVarResult,
//                             /* [out] */ EXCEPINFO *pExcepInfo,
//                             /* [out] */ UINT *puArgErr ) 
//{
//    return S_OK;
//}



//INSTANCE_DEFINE( CHookedViewerPanel );
//
//void CHookedViewerPanel::Hook( void* pViewerPanel )
//{
//    SubstituteOrgFuncWithNew( pViewerPanel, 13, (void*)Hooked_Init );
//    SubstituteOrgFuncWithNew( pViewerPanel, 14, (void*)Hooked_get_State );
//    SubstituteOrgFuncWithNew( pViewerPanel, 15, (void*)Hooked_put_State );
//    SubstituteOrgFuncWithNew( pViewerPanel, 16, (void*)Hooked_get_Handle );
//    SubstituteOrgFuncWithNew( pViewerPanel, 17, (void*) Hooked_get_AutoPan );
//    SubstituteOrgFuncWithNew( pViewerPanel, 18, (void*)Hooked_put_AutoPan );
//    SubstituteOrgFuncWithNew( pViewerPanel, 19, (void*)Hooked_get_Control );
//    SubstituteOrgFuncWithNew( pViewerPanel, 20, (void*)Hooked_put_Control );
//    SubstituteOrgFuncWithNew( pViewerPanel, 21, (void*)Hooked_get_Scale );
//    SubstituteOrgFuncWithNew( pViewerPanel, 22, (void*)Hooked_put_Scale );
//    SubstituteOrgFuncWithNew( pViewerPanel, 23, (void*)Hooked_get_AspectRatio );
//    SubstituteOrgFuncWithNew( pViewerPanel, 24, (void*)Hooked_get_ViewerSnapshotBuffer );
//    DoHook( pViewerPanel );
//}
//HRESULT __stdcall CHookedViewerPanel::Hooked_Init ( ICollaborateViewerPanel* pViewerPanel,
//                                                   /*[in]*/ wireHWND hWndParent )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_Init pFunc = (Func_Init)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_Init ));
//    if( !pFunc )
//    {
//		DP((L"uuuuuuuuuutttttttttttttttttt")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_Init)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_Init ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, hWndParent );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_State ( ICollaborateViewerPanel* pViewerPanel,
//                                                        /*[out,retval]*/ enum LMC_AppShare::ShareStateType * varState )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_State pFunc = (Func_get_State)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_State ));
//    if( !pFunc )
//    {
//		DP((L"ooooooooooooooooooooooooooooooooo")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_State)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_State ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varState );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_put_State ( ICollaborateViewerPanel* pViewerPanel,
//                                                        /*[in]*/ enum LMC_AppShare::ShareStateType varState )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_put_State pFunc = (Func_put_State)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_State ));
//    if( !pFunc )
//    {
//		DP((L"eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_put_State)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_State ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varState );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_Handle ( ICollaborateViewerPanel* pViewerPanel,
//                                                         /*[out,retval]*/ wireHWND * varHandle )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_Handle pFunc = (Func_get_Handle)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Handle ));
//    if( !pFunc )
//    {
//		DP((L"tttttttttttttttteeeeeeeeeeeeeee")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_Handle)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Handle ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varHandle );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_AutoPan ( ICollaborateViewerPanel* pViewerPanel,
//                                                          /*[out,retval]*/ VARIANT_BOOL * varAutoPan )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_AutoPan pFunc = (Func_get_AutoPan)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_AutoPan ));
//    if( !pFunc )
//    {
//		DP((L"wwwwwwwwwwwwwwwwwwwwwdddddddddddddd")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_AutoPan)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_AutoPan ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varAutoPan );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_put_AutoPan ( ICollaborateViewerPanel* pViewerPanel,
//                                                          /*[in]*/ VARIANT_BOOL varAutoPan )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_put_AutoPan pFunc = (Func_put_AutoPan)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_AutoPan ));
//    if( !pFunc )
//    {
//		DP((L"rrrrrrrrrrrrrrrrrggggggggggggggggggg")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_put_AutoPan)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_AutoPan ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varAutoPan );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_Control ( ICollaborateViewerPanel* pViewerPanel,
//                                                          /*[out,retval]*/ VARIANT_BOOL * varControl )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_Control pFunc = (Func_get_Control)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Control ));
//    if( !pFunc )
//    {
//		DP((L"sssssssssssssssddddddddddd")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_Control)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Control ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varControl );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_put_Control ( ICollaborateViewerPanel* pViewerPanel,
//                                                          /*[in]*/ VARIANT_BOOL varControl )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_put_Control pFunc = (Func_put_Control)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_Control ));
//    if( !pFunc )
//    {
//		DP((L"23dasfgsdt34r345t34r5")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_put_Control)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_Control ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varControl );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_Scale ( ICollaborateViewerPanel* pViewerPanel,
//                                                        /*[out,retval]*/ VARIANT_BOOL * varScale )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_Scale pFunc = (Func_get_Scale)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Scale ));
//    if( !pFunc )
//    {
//		DP((L"90se344fs")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_Scale)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_Scale ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varScale );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_put_Scale ( ICollaborateViewerPanel* pViewerPanel,
//                                                        /*[in]*/ VARIANT_BOOL varScale )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_put_Scale pFunc = (Func_put_Scale)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_Scale ));
//    if( !pFunc )
//    {
//		DP((L"sdfcvggggggggggggggg")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_put_Scale)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_put_Scale ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varScale );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_AspectRatio ( ICollaborateViewerPanel* pViewerPanel,
//                                                              /*[out,retval]*/ double * varAspectRatio )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_AspectRatio pFunc = (Func_get_AspectRatio)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_AspectRatio ));
//    if( !pFunc )
//    {
//		DP((L"sdfwefffffffffffffffffffffffffffff")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_AspectRatio)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_AspectRatio ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varAspectRatio );
//    }
//    return hr;
//}
//
//HRESULT __stdcall CHookedViewerPanel::Hooked_get_ViewerSnapshotBuffer ( ICollaborateViewerPanel* pViewerPanel,
///*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer )
//{
//    HRESULT hr = E_NOTIMPL;
//
//    Func_get_ViewerSnapshotBuffer pFunc = (Func_get_ViewerSnapshotBuffer)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_ViewerSnapshotBuffer ));
//    if( !pFunc )
//    {
//		DP((L"vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv")) ;
//        GetInstance()->Hook( pViewerPanel );
//        pFunc = (Func_get_ViewerSnapshotBuffer)(GetInstance()->GetOrgFunc( (void*)pViewerPanel, Hooked_get_ViewerSnapshotBuffer ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc(pViewerPanel, varViewerSnapshotBuffer );
//    }
//    return hr;
//}
