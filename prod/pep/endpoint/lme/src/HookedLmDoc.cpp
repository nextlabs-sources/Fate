#include "stdafx.h"
#include "HookedLmDoc.h"

//INSTANCE_DEFINE( CHookedLmDoc );
//
//void CHookedLmDoc::Hook( void* pLmDoc )
//{
//    SubstituteOrgFuncWithNew( pLmDoc, 7, (void*)Save );
//    SubstituteOrgFuncWithNew( pLmDoc, 8, (void*)Close );
//    SubstituteOrgFuncWithNew( pLmDoc, 9, (void*)SaveAs);
//    SubstituteOrgFuncWithNew( pLmDoc, 10, (void*)get_Images );
//    SubstituteOrgFuncWithNew( pLmDoc, 11, (void*)get_BuiltInDocumentProperties );
//    SubstituteOrgFuncWithNew( pLmDoc, 12, (void*)get_CustomDocumentProperties );
//    SubstituteOrgFuncWithNew( pLmDoc, 13, (void*)Create );
//    SubstituteOrgFuncWithNew( pLmDoc, 14, (void*)get_Dirty );
//    SubstituteOrgFuncWithNew( pLmDoc, 15, (void*)OCR);
//    SubstituteOrgFuncWithNew( pLmDoc, 16, (void*)PrintOut );
//    DoHook( pLmDoc );
//}
//
//HRESULT __stdcall CHookedLmDoc::Save ( IDocument* pDoc )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::Close ( IDocument* pDoc,
//    /*[in]*/ VARIANT_BOOL SaveChanges )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::SaveAs ( IDocument* pDoc,
//                                 /*[in]*/ BSTR FileName,
//                                 /*[in]*/ enum MiFILE_FORMAT FileFormat,
//                                 /*[in]*/ enum MiCOMP_LEVEL CompLevel )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::get_Images ( IDocument* pDoc,
///*[out,retval]*/ struct IImages * * pVal )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::get_BuiltInDocumentProperties ( IDocument* pDoc,
//    /*[out,retval]*/ IDispatch * * pVal )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::get_CustomDocumentProperties ( IDocument* pDoc,
//    /*[out,retval]*/ IDispatch * * pVal )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::Create ( IDocument* pDoc,
//    /*[in]*/ BSTR FileOpen )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::get_Dirty ( IDocument* pDoc,
//    /*[out,retval]*/ VARIANT_BOOL * pVal )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::OCR ( IDocument* pDoc,
//                              /*[in]*/ enum MiLANGUAGES LangId,
//                              /*[in]*/ VARIANT_BOOL OCROrientImage,
//                              /*[in]*/ VARIANT_BOOL OCRStraightenImage )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedLmDoc::PrintOut ( IDocument* pDoc,
//                                   /*[in]*/ long From,
//                                   /*[in]*/ long To,
//                                   /*[in]*/ long Copies,
//                                   /*[in]*/ BSTR PrinterName,
//                                   /*[in]*/ BSTR PrintToFileName,
//                                   /*[in]*/ VARIANT_BOOL PrintAnnotation,
//                                   /*[in]*/ enum MiPRINT_FITMODES FitMode )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}


INSTANCE_DEFINE( CHookedDocConverter );

void CHookedDocConverter::Hook( void* pLmDocConverter )
{
    SubstituteOrgFuncWithNew( pLmDocConverter, 7, (void*)Hooked_RequiresMODIConversion );
    SubstituteOrgFuncWithNew( pLmDocConverter, 8, (void*)Hooked_ConvertDocToLmp );
    //SubstituteOrgFuncWithNew( pLmDocConverter, 9, (void*)Hooked_GetErrorCode);
    //SubstituteOrgFuncWithNew( pLmDocConverter, 10, (void*)Hooked_GetMODIPrinterName );
    DoHook( pLmDocConverter );
}

HRESULT __stdcall CHookedDocConverter::Hooked_RequiresMODIConversion ( IDocConverter* pDocConverter,
                                                 /*[in]*/ BSTR bstrDocPath,
                                                 /*[out]*/ VARIANT_BOOL * pbRequiresConversion )
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
        Func_RequiresMODIConversion pFunc = (Func_RequiresMODIConversion)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_RequiresMODIConversion ));
		if( pFunc )
		{
			     hr =pFunc( pDocConverter, bstrDocPath, pbRequiresConversion );

		}

	}
	__try
	{
		return my_RequiresMODIConversion( pDocConverter, bstrDocPath, pbRequiresConversion );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedDocConverter::my_RequiresMODIConversion ( IDocConverter* pDocConverter,
                                                 /*[in]*/ BSTR bstrDocPath,
                                                 /*[out]*/ VARIANT_BOOL * pbRequiresConversion )
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
    Func_RequiresMODIConversion pFunc = (Func_RequiresMODIConversion)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_RequiresMODIConversion ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDocConverter );
        pFunc = (Func_RequiresMODIConversion)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_RequiresMODIConversion ));
    }

    HRESULT hr = E_NOTIMPL;

    /*extern CEvaluator gEva; 
    CCEResult aCEResult;
    STRINGLIST Attendee;
    CPartDB::GetInstance()->GetSIPList( Attendee );
    STRINGLIST Files;
    Files.push_back( bstrDocPath );
    gEva.EvaluateLMShareUploadFile( Attendee, Files,  aCEResult );  
    if( aCEResult.Items.result != Res_CEAllow )
    {
         return hr;
    }*/
    if( !DoEvaluate( bstrDocPath ,L"SHARE") )
    {
        OutputDebugStringA( "Upload file denied" );
        return hr;
    }

    OutputDebugStringA( "Upload file allowed" );

    if( pFunc )
    {
        hr = pFunc( pDocConverter, bstrDocPath, pbRequiresConversion );
    }
    return hr;
}

HRESULT __stdcall CHookedDocConverter::Hooked_ConvertDocToLmp ( IDocConverter* pDocConverter,
                                          /*[in]*/ short audVersion,
                                          /*[in]*/ BSTR bstrDocPath,
                                          /*[in]*/ BSTR bstrOriginalDocExtension,
                                          /*[out]*/ BSTR * pbstrLmpPath ) 
{
	HRESULT hr = E_NOTIMPL;
	if( LMEIsDisabled() == true )
	{
         Func_ConvertDocToLmp pFunc = (Func_ConvertDocToLmp)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_ConvertDocToLmp ));
		if( pFunc )
		{
			     hr =pFunc( pDocConverter, audVersion, bstrDocPath, bstrOriginalDocExtension, pbstrLmpPath );

		}

	}
	__try
	{
		return my_ConvertDocToLmp( pDocConverter, audVersion, bstrDocPath, bstrOriginalDocExtension, pbstrLmpPath );
	}
	__except(NLEXCEPT_FILTER_EX2(NULL,exception_cb))
	{
		;
	}
	return hr;
}
HRESULT __stdcall CHookedDocConverter::my_ConvertDocToLmp ( IDocConverter* pDocConverter,
                                          /*[in]*/ short audVersion,
                                          /*[in]*/ BSTR bstrDocPath,
                                          /*[in]*/ BSTR bstrOriginalDocExtension,
                                          /*[out]*/ BSTR * pbstrLmpPath ) 
{	
	//nextlabs::recursion_control_auto auto_disable(hook_control);
    Func_ConvertDocToLmp pFunc = (Func_ConvertDocToLmp)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_ConvertDocToLmp ));
    if( !pFunc )
    {
        GetInstance()->Hook( pDocConverter );
        pFunc = (Func_ConvertDocToLmp)(GetInstance()->GetOrgFunc( (void*)pDocConverter, Hooked_ConvertDocToLmp ));
    }

    HRESULT hr = E_NOTIMPL;

    if( !DoEvaluate( bstrDocPath,L"SHARE" ) )
    {
        OutputDebugStringA( "Upload file denied" );
        return hr;
    }

    OutputDebugStringA( "Upload file allowed" );

    if( pFunc )
    {
        hr = pFunc( pDocConverter, audVersion, bstrDocPath, bstrOriginalDocExtension, pbstrLmpPath );
    }
    return hr;
}

//HRESULT __stdcall CHookedDocConverter::Hooked_GetErrorCode ( IDocConverter* pDocConverter,
//                                       /*[in,out]*/ long * pErrorCode )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}
//
//HRESULT __stdcall CHookedDocConverter::Hooked_GetMODIPrinterName ( IDocConverter* pDocConverter,
//                                             /*[in,out]*/ BSTR * pbstrMODIPrinterName )
//{
//    HRESULT hr = E_NOTIMPL;
//    return hr;
//}