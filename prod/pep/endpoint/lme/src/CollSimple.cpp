#include "StdAfx.h"
#include "CollSimple.h"
#include "OfficeShare.h"
#include "PartDB.h"
#include "PCStream.h"

//ICollaborateSimple

INSTANCE_DEFINE( CHookedCollSimple );

void CHookedCollSimple::Hook( void* pSimple )
{
    //SubstituteOrgFuncWithNew( pSimple, 6, (void*)Hooked_Invoke );
    //SubstituteOrgFuncWithNew( pSimple, 12, (void*)Hooked_OnNotify );
    ////SubstituteOrgFuncWithNew( pSimple, 13, (void*)CollSimple_New_Init);
    ////SubstituteOrgFuncWithNew( pSimple, 14, (void*)New_CollSimple_get_Share);
    //SubstituteOrgFuncWithNew( pSimple, 15, (void*)New_CollSimple_put_Share); 
    //SubstituteOrgFuncWithNew( pSimple, 17, (void*)NewGet_Annotate );
    //SubstituteOrgFuncWithNew( pSimple, 18, (void*)NewPut_Annotate );
    //SubstituteOrgFuncWithNew( pSimple, 23, (void*)New_AnnotateClear);
    //SubstituteOrgFuncWithNew( pSimple, 27, (void*)NewPut_PrimaryWindow);
    //SubstituteOrgFuncWithNew( pSimple, 28, (void*)NewPut_PrimaryWindowTitle );
    //SubstituteOrgFuncWithNew( pSimple, 29, (void*)NewGet_PrimaryWindowTitle );
    /*SubstituteOrgFuncWithNew( pSimple, 30, (void*)NewGet_ProcessId);*/
    SubstituteOrgFuncWithNew( pSimple, 31, (void*)NewPut_ProcessId);
    //SubstituteOrgFuncWithNew( pSimple, 32, (void*)New_CollSimple_get_ControlRequestAllowed);
    //SubstituteOrgFuncWithNew( pSimple, 33, (void*)New_CollSimple_put_ControlRequestAllowed);
    //SubstituteOrgFuncWithNew( pSimple, 34, (void*)New_CollSimple_get_AutoAcceptControlRequest);
    //SubstituteOrgFuncWithNew( pSimple, 35, (void*)New_CollSimple_put_AutoAcceptControlRequest);
/*    SubstituteOrgFuncWithNew( pSimple, 36, (void*)Hooked_get_HotKey );
    SubstituteOrgFuncWithNew( pSimple, 37, (void*)Hooked_put_HotKey );
    SubstituteOrgFuncWithNew( pSimple, 38, (void*)NewPut_PrimaryWindowPosition );
    SubstituteOrgFuncWithNew( pSimple, 39, (void*)NewGet_PrimaryWindowPosition );
    SubstituteOrgFuncWithNew( pSimple, 43, (void*)New_OpenSnapshotTool);
    SubstituteOrgFuncWithNew( pSimple, 44, (void*)New_SendQosPacket);
    SubstituteOrgFuncWithNew( pSimple, 45, (void*)Hooked_InitPlayback);
    SubstituteOrgFuncWithNew( pSimple, 46, (void*)New_InitViewer);
    SubstituteOrgFuncWithNew( pSimple, 47, (void*)New_ShareViewer);
    SubstituteOrgFuncWithNew( pSimple, 48, (void*)New_CreateViewerPanel);    
    SubstituteOrgFuncWithNew( pSimple, 50, (void*)NewPut_Scale);    
    SubstituteOrgFuncWithNew( pSimple, 53, (void*)NewGetViewerSnapshotBuffer);  */  
    DoHook( pSimple );
}
//////////////////////////////////////////////////////////////////////////
// hook on these funcion
HRESULT __stdcall CHookedCollSimple::CollSimple_New_Init (ICollaborateSimple* pSimple,
									   IUnknown * pNPWRC_Stream )
{
	HRESULT hr = E_NOTIMPL;
    CollSimple_Old_Init pFunc = (CollSimple_Old_Init)(GetInstance()->GetOrgFunc( (void*)pSimple, CollSimple_New_Init ));
    if( !pFunc )
    {
        GetInstance()->Hook( pSimple );
        pFunc = (CollSimple_Old_Init)(GetInstance()->GetOrgFunc( (void*)pSimple, CollSimple_New_Init ));
    }

    if( pFunc )
    {
        hr = pFunc( pSimple, pNPWRC_Stream );
    }

	//if(SUCCEEDED(hr))
	//{
	//	INpwRC_Stream* pStream=NULL;
	//	hr = pNPWRC_Stream->QueryInterface(IID_INpwRC_Stream,(void**)pStream);
	//	if(SUCCEEDED(hr))
	//	{
	//		CHookedStream::GetInstance()->Hook( (PVOID)pStream );//HookStream(pStream);
	//	}
	//}

	return hr;
}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_get_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType * varShare ) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_get_Share pFunc = (Old_CollSimple_get_Share)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_get_Share)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_Share ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varShare );
//    }
//
//	char *strShare="NULL";
//	switch(*varShare)
//	{
//	case LMC_AppShare::SHARE_UNINIT:
//		{
//			strShare = "SHARE_UNINIT";
//		}
//		break;
//	case LMC_AppShare::SHARE_PAUSE:
//		{
//			strShare = "SHARE_PAUSE";
//		}
//		break;
//	case LMC_AppShare::SHARE_STOP:
//		{
//			strShare = "SHARE_STOP";
//		}
//		break;
//	case LMC_AppShare::SHARE_PLAY:
//		{
//			strShare = "SHARE_PLAY";
//		}
//		break;
//	}
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_put_Share (ICollaborateSimple* pSimple,LMC_Coll::ShareStateType varShare ) 
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_put_Share pFunc = (Old_CollSimple_put_Share)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_Share ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_put_Share)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_Share ));
//    }
//
//    VARIANT_BOOL bIsAvail = false;
//    pSimple->get_ViewerSnapshotAvailable( & bIsAvail );
//    if( bIsAvail )
//    {
//        ICollaborateViewerBuffer* pBuffer = 0;
//        HRESULT hr = pSimple->get_ViewerSnapshotBuffer( &pBuffer );
//        int i = 0;
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varShare );
//    }
//
//	char *strShare=NULL;
//	switch(varShare)
//	{
//	case LMC_AppShare::SHARE_UNINIT:
//		{
//			strShare = "SHARE_UNINIT";
//		}
//		break;
//	case LMC_AppShare::SHARE_PAUSE:
//		{
//			strShare = "SHARE_PAUSE";
//		}
//		break;
//	case LMC_AppShare::SHARE_STOP:
//		{
//			strShare = "SHARE_STOP";
//            //return E_FAIL;
//		}
//		break;
//	case LMC_AppShare::SHARE_PLAY:
//		{
//			strShare = "SHARE_PLAY";
//			//return S_OK;
//		}
//		break;
//	}
//
//	return hr;
//}


// clear annotate
//HRESULT __stdcall CHookedCollSimple::New_AnnotateClear(ICollaborateSimple* pSimple)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_AnnotateClear pFunc = (Old_AnnotateClear)(GetInstance()->GetOrgFunc( (void*)pSimple, New_AnnotateClear ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_AnnotateClear)(GetInstance()->GetOrgFunc( (void*)pSimple, New_AnnotateClear ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_get_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL* varControlRequestAllowed)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_get_ControlRequestAllowed pFunc = (Old_CollSimple_get_ControlRequestAllowed)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_ControlRequestAllowed ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_get_ControlRequestAllowed)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_ControlRequestAllowed ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varControlRequestAllowed );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_put_ControlRequestAllowed(ICollaborateSimple* pSimple,VARIANT_BOOL varControlRequestAllowed)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_put_ControlRequestAllowed pFunc = (Old_CollSimple_put_ControlRequestAllowed)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_ControlRequestAllowed ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_put_ControlRequestAllowed)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_ControlRequestAllowed ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varControlRequestAllowed );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_get_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL* varAutoAcceptControlRequest)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_get_AutoAcceptControlRequest pFunc = (Old_CollSimple_get_AutoAcceptControlRequest)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_AutoAcceptControlRequest ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_get_AutoAcceptControlRequest)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_get_AutoAcceptControlRequest ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varAutoAcceptControlRequest );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CollSimple_put_AutoAcceptControlRequest(ICollaborateSimple* pSimple,VARIANT_BOOL varAutoAcceptControlRequest)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CollSimple_put_AutoAcceptControlRequest pFunc = (Old_CollSimple_put_AutoAcceptControlRequest)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_AutoAcceptControlRequest ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CollSimple_put_AutoAcceptControlRequest)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CollSimple_put_AutoAcceptControlRequest ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varAutoAcceptControlRequest );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_InitViewer(ICollaborateSimple* pSimple,IUnknown* pNPWRC_Stream)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_InitViewer pFunc = (Old_InitViewer)(GetInstance()->GetOrgFunc( (void*)pSimple, New_InitViewer ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_InitViewer)(GetInstance()->GetOrgFunc( (void*)pSimple, New_InitViewer ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, pNPWRC_Stream );
//    }
//	//if(SUCCEEDED(hr))
//	//{
//	//	INpwRC_Stream* pRCStream=NULL;
//	//	hr = pNPWRC_Stream->QueryInterface(IID_INpwRC_Stream,(void**)&pRCStream);
//	//	if(SUCCEEDED(hr) && pRCStream != NULL)
//	//	{
//	//		CHookedStream::GetInstance()->Hook( (PVOID)pRCStream );//HookStream(pRCStream);
//	//	}
//	//}
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_ShareViewer(ICollaborateSimple* pSimple,LMC_Coll::ShareStateType rhs)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_ShareViewer pFunc = (Old_ShareViewer)(GetInstance()->GetOrgFunc( (void*)pSimple, New_ShareViewer ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_ShareViewer)(GetInstance()->GetOrgFunc( (void*)pSimple, New_ShareViewer ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, rhs );
//    }
//
//	char *strShare="NULL";
//	switch(rhs)
//	{
//	case LMC_AppShare::SHARE_UNINIT:
//		{
//			strShare = "SHARE_UNINIT";
//		}
//		break;
//	case LMC_AppShare::SHARE_PAUSE:
//		{
//			strShare = "SHARE_PAUSE";
//		}
//		break;
//	case LMC_AppShare::SHARE_STOP:
//		{
//			strShare = "SHARE_STOP";
//		}
//		break;
//	case LMC_AppShare::SHARE_PLAY:
//		{
//			strShare = "SHARE_PLAY";
//			return hr;
//		}
//		break;
//	}
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_CreateViewerPanel(
//										ICollaborateSimple* pSimple,
//										long hWndSlide, 
//										long* phWnd)
//
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_CreateViewerPanel pFunc = (Old_CreateViewerPanel)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CreateViewerPanel ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_CreateViewerPanel)(GetInstance()->GetOrgFunc( (void*)pSimple, New_CreateViewerPanel ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, hWndSlide, phWnd );
//    }
//
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_OpenSnapshotTool(ICollaborateSimple* pSimple)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_OpenSnapshotTool pFunc = (Old_OpenSnapshotTool)(GetInstance()->GetOrgFunc( (void*)pSimple, New_OpenSnapshotTool ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_OpenSnapshotTool)(GetInstance()->GetOrgFunc( (void*)pSimple, New_OpenSnapshotTool ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple );
//    }
//	return hr;
//}

//HRESULT __stdcall CHookedCollSimple::New_SendQosPacket(ICollaborateSimple* pSimple,unsigned long nPacketId)
//{
//	HRESULT hr = E_NOTIMPL;
//    Old_SendQosPacket pFunc = (Old_SendQosPacket)(GetInstance()->GetOrgFunc( (void*)pSimple, New_SendQosPacket ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Old_SendQosPacket)(GetInstance()->GetOrgFunc( (void*)pSimple, New_SendQosPacket ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, nPacketId );
//    }
//	return hr;
//}

HRESULT __stdcall CHookedCollSimple::NewPut_ProcessId (
                                    ICollaborateSimple* pSimple,
                                    /*[in]*/ unsigned long varProcessId )
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
        Put_ProcessIdFunc pFunc = (Put_ProcessIdFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_ProcessId ));
		if( pFunc )
		{
			     hr =  pFunc( pSimple, varProcessId );
		}

	}
	__try
	{
		return  myPut_ProcessId( pSimple, varProcessId );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedCollSimple::myPut_ProcessId (
                                    ICollaborateSimple* pSimple,
                                    /*[in]*/ unsigned long varProcessId )
{	
	// nextlabs::recursion_control_auto auto_disable(hook_control);
    HRESULT hr = E_NOTIMPL;
    Put_ProcessIdFunc pFunc = (Put_ProcessIdFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_ProcessId ));
    if( !pFunc )
    {
        GetInstance()->Hook( pSimple );
        pFunc = (Put_ProcessIdFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_ProcessId ));
    }
	const TCHAR* pName  = GetProcessName( varProcessId );
		DWORD nStrLen = (DWORD)wcslen(pName);;
	bool bAllow = false;

	std::wstring strInfo( pName );
	//strInfo += TEXT("\n\n");
	//strInfo += CPartDB::GetInstance()->GetPresenterAttendeeInfo();

	if( nStrLen )
	{
		if( _wcsicmp( GetNameWithoutPath( strInfo.c_str(), (DWORD)strInfo.length()), L"PWConsole.exe" ) == 0 )
		{
			//CE_ACTION_WM_SHARE
			bAllow = DoAppEvaluate( LME_MAGIC_STRING,TEXT( "DESKTOP" ),L"SHARE" );
		}
		else
		{
			bAllow = DoAppEvaluate( strInfo.c_str() ,L"APPLICATION",L"SHARE");//MsgBoxAllowOrDeny( strInfo.c_str(), L"Allow to share following process ?" );
		}
	}
	else
	{
		bAllow = DoAppEvaluate( LME_MAGIC_STRING,TEXT( "DESKTOP" ),L"SHARE" );//"[LM_LocalDeskTop]" ) );//MsgBoxAllowOrDeny( strInfo.c_str(), L"Allow to share Desktop ?" );
	}
	if( !bAllow)
	{
		return hr ;
	}
    if( pFunc )
    {
        hr = pFunc( pSimple, varProcessId );
    }

    if( varProcessId )
    {
        CProcessDB::GetInstance()->AddName( GetProcessName( varProcessId ) );
        return hr;
    }
    return hr;
}


//HRESULT __stdcall CHookedCollSimple::NewPut_PrimaryWindow(
//                                       ICollaborateSimple* pSimple,
//                                       /*[in]*/ long varPrimaryWindow )
//
//{
//    HRESULT hr = E_NOTIMPL;
//    Put_PrimaryWindowFunc pFunc = (Put_PrimaryWindowFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindow ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Put_PrimaryWindowFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindow ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varPrimaryWindow );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedCollSimple::NewPut_Scale (
//                                ICollaborateSimple* pSimple,
//                                /*[in]*/ VARIANT_BOOL varScale ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Put_ScaleFunc pFunc = (Put_ScaleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_Scale ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Put_ScaleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_Scale ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varScale );
//    }
//    return hr;
//}

////////////////////////////////////////////////////////////////////////


//HRESULT _stdcall CHookedCollSimple::NewGetViewerSnapshotBuffer (
//    ICollaborateSimple* pSimple,
///*[out,retval]*/ struct ICollaborateViewerBuffer * * varViewerSnapshotBuffer )
//{
//    HRESULT hr = E_NOTIMPL;
//    GetViewerSnapshotBufferFunc pFunc = (GetViewerSnapshotBufferFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGetViewerSnapshotBuffer ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (GetViewerSnapshotBufferFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGetViewerSnapshotBuffer ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varViewerSnapshotBuffer );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedCollSimple::NewPut_Annotate ( ICollaborateSimple* pSimple,
//                                   /*[in]*/ VARIANT_BOOL varAnnotate )
//{
//    HRESULT hr = E_NOTIMPL;
//    Put_AnnotateFunc pFunc = (Put_AnnotateFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_Annotate ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Put_AnnotateFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_Annotate ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varAnnotate );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedCollSimple::NewGet_Annotate ( ICollaborateSimple* pSimple,
//                                   /*[in]*/ VARIANT_BOOL varAnnotate )
//{
//    HRESULT hr = E_NOTIMPL;
//    Get_AnnotateFunc pFunc = (Get_AnnotateFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_Annotate ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Get_AnnotateFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_Annotate ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varAnnotate );
//    }
//    return hr;
//}


//HRESULT __stdcall CHookedCollSimple::NewPut_PrimaryWindowPosition (
//    ICollaborateSimple* pSimple,
//    /*[in]*/ long nLeftX,
//    /*[in]*/ long nTopY,
//    /*[in]*/ long nRightX,
//    /*[in]*/ long nBottomY ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Put_PrimaryWindowPositionFunc pFunc = (Put_PrimaryWindowPositionFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindowPosition ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Put_PrimaryWindowPositionFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindowPosition ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, nLeftX, nTopY, nRightX, nBottomY );
//    }
//    return hr;
//}

//HRESULT __stdcall CHookedCollSimple::NewGet_PrimaryWindowPosition (
//    ICollaborateSimple* pSimple,
//    /*[in]*/ long * pnLeftX,
//    /*[out]*/ long * pnTopY,
//    /*[out]*/ long * pnRightX,
//    /*[out]*/ long * pnBottomY )
//{
//    HRESULT hr = E_NOTIMPL;
//    Get_PrimaryWindowPositionFunc pFunc = (Get_PrimaryWindowPositionFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_PrimaryWindowPosition ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Get_PrimaryWindowPositionFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_PrimaryWindowPosition ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, pnLeftX, pnTopY, pnRightX, pnBottomY );
//    }
//    
//    return hr;
//}
//

//HRESULT __stdcall CHookedCollSimple::NewGet_ProcessId (
//                                    ICollaborateSimple* pSimple,
//                                    /*[out,retval]*/ unsigned long * varProcessId )
//{
//    HRESULT hr = E_NOTIMPL;
//    Get_ProcessIdFunc pFunc = (Get_ProcessIdFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_ProcessId ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Get_ProcessIdFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_ProcessId ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varProcessId );
//    }
//    return hr;  
//}



//HRESULT __stdcall CHookedCollSimple::NewGet_PrimaryWindowTitle (
//    ICollaborateSimple* pSimple,
//    /*[out,retval]*/ BSTR * varPrimaryWindowTitle )
//{
//    HRESULT hr = E_NOTIMPL;
//    Get_PrimaryWindowTitleFunc pFunc = (Get_PrimaryWindowTitleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_PrimaryWindowTitle ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Get_PrimaryWindowTitleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewGet_PrimaryWindowTitle ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varPrimaryWindowTitle );
//    }
//    return hr;  
//}

//HRESULT __stdcall CHookedCollSimple::NewPut_PrimaryWindowTitle (
//    ICollaborateSimple* pSimple,
//    /*[in]*/ BSTR varPrimaryWindowTitle ) 
//{
//    HRESULT hr = E_NOTIMPL;
//    Put_PrimaryWindowTitleFunc pFunc = (Put_PrimaryWindowTitleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindowTitle ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Put_PrimaryWindowTitleFunc)(GetInstance()->GetOrgFunc( (void*)pSimple, NewPut_PrimaryWindowTitle ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varPrimaryWindowTitle );
//    }
//    return hr;  
//}

//HRESULT __stdcall CHookedCollSimple::Hooked_get_HotKey ( ICollaborateSimple* pSimple,
//                                     /*[out,retval]*/ unsigned int * varHotKey )
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_get_HotKey pFunc = (Func_get_HotKey)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_get_HotKey ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Func_get_HotKey)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_get_HotKey ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varHotKey );
//    }
//    return hr;  
//}

//HRESULT __stdcall CHookedCollSimple::Hooked_put_HotKey ( ICollaborateSimple* pSimple,
//                                            /*[in]*/ unsigned int varHotKey )
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_put_HotKey pFunc = (Func_put_HotKey)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_put_HotKey ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Func_put_HotKey)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_put_HotKey ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, varHotKey );
//    }
//    return hr;  
//}

//HRESULT __stdcall CHookedCollSimple::Hooked_InitPlayback ( ICollaborateSimple* pSimple,
//                                       /*[in]*/ IUnknown * pNPWRC_Stream,
//                                       /*[in]*/ BSTR strFile,
//                                       /*[in]*/ double dblSpeed,
//                                       /*[in]*/ unsigned long nTimeToPlay )
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_InitPlayback pFunc = (Func_InitPlayback)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_InitPlayback ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Func_InitPlayback)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_InitPlayback ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, pNPWRC_Stream, strFile, dblSpeed, nTimeToPlay );
//    }
//    return hr;  
//}

//HRESULT STDMETHODCALLTYPE CHookedCollSimple::Hooked_Invoke( ICollaborateSimple* pSimple,
//                                        /* [in] */ DISPID dispIdMember,
//                                        /* [in] */ REFIID riid,
//                                        /* [in] */ LCID lcid,
//                                        /* [in] */ WORD wFlags,
//                                        /* [out][in] */ DISPPARAMS *pDispParams,
//                                        /* [out] */ VARIANT *pVarResult,
//                                        /* [out] */ EXCEPINFO *pExcepInfo,
//                                        /* [out] */ UINT *puArgErr)
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_Invoke pFunc = (Func_Invoke)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_Invoke ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Func_Invoke)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_Invoke ));
//    }
//
//    switch( dispIdMember )
//    {
//    case 0x6003000a: 
//       // ShowInfo( TEXT("AnnotateClear() called." ) );
//        break;
//
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, dispIdMember,  riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr );
//    }
//    return hr;  
//}

//HRESULT __stdcall CHookedCollSimple::Hooked_OnNotify ( ICollaborateSimple* pSimple,
///*[in]*/ struct IBaseEvent * pEvent )
//{
//    HRESULT hr = E_NOTIMPL;
//    Func_OnNotify pFunc = (Func_OnNotify)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_OnNotify ));
//    if( !pFunc )
//    {
//        GetInstance()->Hook( pSimple );
//        pFunc = (Func_OnNotify)(GetInstance()->GetOrgFunc( (void*)pSimple, Hooked_OnNotify ));
//    }
//
//    if( pFunc )
//    {
//        hr = pFunc( pSimple, pEvent );
//    }
//    return hr;  
//}